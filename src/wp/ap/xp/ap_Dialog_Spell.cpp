/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_hash.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_App.h"

#include "ap_FrameData.h"
#include "fl_DocLayout.h"
#include "xap_Prefs.h"
#include "ap_Prefs.h"

#include "ap_Dialog_Spell.h"

#include "xap_EncodingManager.h"

#if 1
// todo: remove this requirement on INPUTWORDLEN for pspell builds
#include "ispell_def.h"
#endif

// important that you call makeWordVisible before you call this
SpellChecker *
AP_Dialog_Spell::_getDict (void)
{
	SpellChecker * checker = NULL;
	const char * szLang = NULL;

	const XML_Char ** props_in = NULL;
	if (m_pView && m_pView->getCharFormat(&props_in))
	{
		szLang = UT_getAttribute("lang", props_in);
		FREEP(props_in);
	}

	if (szLang)
	{
		// we get smart and request the proper dictionary
		checker = SpellManager::instance().requestDictionary(szLang);
	}
	else
	{
		// we just (dumbly) default to the last dictionary
		checker = SpellManager::instance().lastDictionary();
	}

	return checker;
}

bool
AP_Dialog_Spell::_spellCheckWord (const UT_UCSChar * word, UT_uint32 len)
{
	SpellChecker * checker = _getDict();

	if (!checker)
	{
		// no checker, don't mark as wrong
		return true;
	}

	if (checker->checkWord (word, len) == SpellChecker::LOOKUP_SUCCEEDED)
		return true;
	return false;
}

AP_Dialog_Spell::AP_Dialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogspelling.html"), m_Suggestions(0)
{
   m_iWordOffset = 0;
   m_iWordLength = -1;

   m_iSentenceStart = 0;
   m_iSentenceEnd = 0;
   
   m_pBlockBuf = NULL;
   m_pView = NULL;
   m_pSection = NULL;
   m_pBlock = NULL;
   m_pChangeAll = NULL;
   m_pIgnoreAll = NULL;

   m_bCancelled = false;
}

AP_Dialog_Spell::~AP_Dialog_Spell(void)
{
	if (m_pView) 
	{
		if (!m_pView->isSelectionEmpty())
			m_pView->cmdUnselectSelection();

		m_pView->moveInsPtTo( m_iOrigInsPoint );
	}

	DELETEP(m_pBlockBuf);
	UT_HASH_PURGEDATA(UT_UCSChar*,m_pChangeAll, free);
	DELETEP(m_pChangeAll);
	DELETEP(m_pIgnoreAll);

	_purgeSuggestions();
}

void AP_Dialog_Spell::_purgeSuggestions(void)
{
	if (!m_Suggestions) return;

	for (UT_uint32 i = 0; i < m_Suggestions->getItemCount(); i++)
	{
		UT_UCSChar * sug = (UT_UCSChar *)m_Suggestions->getNthItem(i);
		if (sug)
			free(sug);
	}

	DELETEP(m_Suggestions);
}

void AP_Dialog_Spell::runModal(XAP_Frame * pFrame)
{
   UT_ASSERT(pFrame);
   m_pFrame = pFrame;
			  
   AP_FrameData * frameData = (AP_FrameData*) m_pFrame->getFrameData();
   m_pDoc = frameData->m_pDocLayout->getDocument();
   m_pView = frameData->m_pDocLayout->getView();
   m_iOrigInsPoint = m_pView->getPoint();
   m_pSection = frameData->m_pDocLayout->getFirstSection();
   m_pBlock = (fl_BlockLayout *) m_pSection->getFirstLayout();
			   
   m_pBlockBuf = new UT_GrowBuf(1024);
   bool bRes = m_pBlock->getBlockBuf(m_pBlockBuf);
   UT_ASSERT(bRes);
   
   m_pChangeAll = new UT_StringPtrMap(7); // is 7 buckets adequate? too much?
   m_pIgnoreAll = new UT_StringPtrMap(7);

   UT_DEBUGMSG(("modal spell dialog: xp init complete\n"));
}

bool AP_Dialog_Spell::nextMisspelledWord(void)
{
   UT_ASSERT(m_pBlockBuf);

   UT_UCSChar* pBlockText = (UT_UCS4Char*)m_pBlockBuf->getPointer(0);
   UT_uint32 iBlockLength = m_pBlockBuf->getLength();

   UT_ASSERT(m_pView && m_pView->getLayout() );	
   bool checkCaps = m_pView->getLayout()->getSpellCheckCaps();
   bool checkNumeric = m_pView->getLayout()->getSpellCheckNumbers();

   // loop until a misspelled word or end of document is hit
   for (;;) 
   {

	   // do we need to move to the next block?
	   if (m_iWordOffset >= iBlockLength) 
	   {

		   // since we're done with this current block, put it
		   // in the block spell queue so squiggles will be updated
	
		   FL_DocLayout * docLayout = m_pSection->getDocLayout();
		   
		   // Makes this honor spelling prefs
		   XAP_App * pApp = m_pFrame->getApp();
		   UT_ASSERT(pApp);
		   XAP_Prefs * pPrefs = pApp->getPrefs();
		   UT_ASSERT(pPrefs);
	 
		   XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
		   UT_ASSERT(pPrefsScheme);
		   
		   bool b = false;
		   pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_AutoSpellCheck, &b);
		   
		   if (b)
		   {
			   docLayout->queueBlockForBackgroundCheck(FL_DocLayout::bgcrSpelling, m_pBlock);
		   }
		   m_pBlock = (fl_BlockLayout *) m_pBlock->getNext();
		   
		   // next section, too?
		   if (m_pBlock == NULL) 
		   {
	 
			   m_pSection = (fl_DocSectionLayout*) m_pSection->getNext();
			   if (m_pSection == NULL)
				   return false; // end of document
			   m_pBlock = (fl_BlockLayout *) m_pSection->getFirstLayout();
		   }
	 
		   // update the buffer with our new block
		   m_pBlockBuf->truncate(0);
		   bool bRes = m_pBlock->getBlockBuf(m_pBlockBuf);
		   UT_ASSERT(bRes);
	     
		   m_iWordOffset = 0;
		   m_iSentenceStart = 0;
		   m_iSentenceEnd = 0;
		   iBlockLength = m_pBlockBuf->getLength();
		   pBlockText = (UT_UCS4Char*)m_pBlockBuf->getPointer(0);

	   }
	
	   // scan block for misspelled words

	   // now start checking
	   bool bFound;
	   bool bAllUpperCase;
	   bool bHasNumeric;
	
	   while (m_iWordOffset < iBlockLength) 
	   {

		   // skip delimiters...
		   while (m_iWordOffset < iBlockLength)
		   {
			   UT_UCSChar followChar, prevChar;
			   followChar = ((m_iWordOffset + 1) < iBlockLength)  ?  pBlockText[m_iWordOffset+1]  :  UCS_UNKPUNK;
			   prevChar = m_iWordOffset > 0 ? pBlockText[m_iWordOffset-1] : UCS_UNKPUNK;
		 
			   if (!UT_isWordDelimiter( pBlockText[m_iWordOffset], followChar, prevChar))
			   {
				   break;
			   }
			   m_iWordOffset++;
		   }
	     
		   // ignore initial quote
		   if (pBlockText[m_iWordOffset] == '\'')
		   {
			   m_iWordOffset++;
		   }
		   if (m_iWordOffset < iBlockLength) 
		   {
	    
			   // we're at the start of a word. find end of word...
			   bAllUpperCase = true;
			   bHasNumeric = false;
			   bFound = false;
			   m_iWordLength = 0;
			   while ((!bFound) && (m_iWordOffset + m_iWordLength) < iBlockLength) 
			   {

				   UT_UCSChar followChar, prevChar;
				   followChar = ((m_iWordOffset + m_iWordLength + 1) < iBlockLength)  ?  pBlockText[m_iWordOffset + m_iWordLength + 1]  :  UCS_UNKPUNK;
				   prevChar = m_iWordOffset + m_iWordLength > 0 ? pBlockText[m_iWordOffset + m_iWordLength - 1] : UCS_UNKPUNK;
			
				   if ( true == UT_isWordDelimiter( pBlockText[m_iWordOffset + m_iWordLength], followChar, prevChar)) 
				   {
					   bFound = true;
				   } 
				   else 
				   {
					   bAllUpperCase &= UT_UCS4_isupper(pBlockText[m_iWordOffset + m_iWordLength]);
					   bHasNumeric |= UT_UCS4_isdigit(pBlockText[m_iWordOffset + m_iWordLength]);

					   m_iWordLength++;
				   }
			   }
		  
			   // ignore terminal quote
			   if (pBlockText[m_iWordOffset + m_iWordLength - 1] == '\'')
			   {
				   m_iWordLength--;
			   }
		  
			   // for some reason, the spell checker fails on all 1-char words & really big ones
			   // -this is a limitation in the underlying default checker ispell --JB
			   if ((m_iWordLength > 1) &&
				   XAP_EncodingManager::get_instance()->noncjk_letters(pBlockText+m_iWordOffset, m_iWordLength) && 
				   (!checkCaps || !bAllUpperCase) &&
				   (!UT_UCS4_isdigit(pBlockText[m_iWordOffset]) &&
					(m_iWordLength < INPUTWORDLEN))) 
			   {
	    
				   // try testing our current change all lists
				   if (!inChangeAll()) 
				   {
					   // try ignore all list and user dictionaries here, too
					   XAP_App * pApp = m_pFrame->getApp();

					   UT_UCSChar theWord[INPUTWORDLEN + 1];
					   UT_uint32 iNewLength = 0;
					   for (UT_uint32 i=0; i < (UT_uint32)m_iWordLength; i++)
					   {
						   UT_UCSChar currentChar;
						   currentChar = pBlockText[m_iWordOffset + i];
			
						   // Remove UCS_ABI_OBJECT from the word
						   if (currentChar == UCS_ABI_OBJECT) continue;
			
						   // Convert smart quote apostrophe to ASCII
						   // single quote to be compatible with
						   // ispell
						   if (currentChar == UCS_RQUOTE) currentChar = '\'';

						   theWord[iNewLength++] = currentChar;
					   }
					   theWord[iNewLength+1] = 0;

					   // Configurably ignore upper-case words
					   if (bAllUpperCase && 
						   m_pView->getLayout()->getSpellCheckCaps())
					   {
						   m_iWordOffset += (m_iWordLength + 1);
						   continue;
					   }

					   // Configurably ignore words containing digits
					   if (bHasNumeric && 
						   m_pView->getLayout()->getSpellCheckNumbers())
					   {
						   m_iWordOffset += (m_iWordLength + 1);
						   continue;
					   }

//					   makeWordVisible();

					   if (!m_pDoc->isIgnore(theWord, iNewLength) &&
						   !pApp->isWordInDict(theWord, iNewLength) &&
						   !_spellCheckWord(theWord, iNewLength)) 
					   {
		  
						   // unknown word...
						   // prepare list of possibilities
						   SpellChecker * checker = _getDict();
						   if (!checker)
						   {
							   UT_DEBUGMSG(("No checker returned\n"));
							   return false;
						   }
						   makeWordVisible(); // display the word now.

						   m_Suggestions = checker->suggestWord(theWord, 
																iNewLength);
						   if(m_Suggestions)
						   {
							   pApp->suggestWord(m_Suggestions,theWord,  
												 iNewLength);
						   }
						   if (!m_Suggestions)
						   {
							   UT_DEBUGMSG(("DOM: no suggestions returned from main dictionary \n"));
							   m_Suggestions = new UT_Vector();
							   pApp->suggestWord(m_Suggestions,theWord,
												 iNewLength);
							   if(m_Suggestions->getItemCount() == 0)
							   {
								   DELETEP(m_Suggestions);
								   m_Suggestions = NULL;
								   return false;
							   }		
						   }
						   
						   // update sentence boundaries (so we can display
						   // the misspelled word in proper context)
						   if (m_iWordOffset + m_iWordLength > m_iSentenceEnd ||
							   m_iWordOffset < m_iSentenceStart) 
						   {
							   _updateSentenceBoundaries();
						   }

						   UT_DEBUGMSG(("misspelled word found\n"));
						   
						   // return to caller
						   return true;
						   // we have all the important state information in class variables,
						   // so the next call to this function will pick up at the same place.
						   // this also means we'll check whatever changes they make to this word.
					   }
				   } 
				   else 
				   {
					   // we changed the word, and the block buffer has
					   // been updated, so reload the pointer and length
					   pBlockText = (UT_UCS4Char*) m_pBlockBuf->getPointer(0);
					   iBlockLength = m_pBlockBuf->getLength();
					   // the offset shouldn't change
				   }
			   }
		
			   // correctly spelled, so continue on
			   m_iWordOffset += (m_iWordLength + 1);
		   }
	   }
   }
}

bool AP_Dialog_Spell::makeWordVisible(void)
{
   UT_DEBUGMSG(("making misspelled word visible in main window\n"));
   
   if (!m_pView->isSelectionEmpty())
     m_pView->cmdUnselectSelection();
   m_pView->moveInsPtTo( (PT_DocPosition) (m_pBlock->getPosition() + m_iWordOffset) );
   m_pView->extSelHorizontal(true, (UT_uint32) m_iWordLength);
   m_pView->updateScreen();
   
   return true;
}

bool AP_Dialog_Spell::addIgnoreAll(void)
{
	UT_UCSChar * pBuf = (UT_UCSChar *) m_pBlockBuf->getPointer(m_iWordOffset);
	return m_pDoc->appendIgnore(pBuf,m_iWordLength);
}

void AP_Dialog_Spell::ignoreWord(void)
{
   // skip past the current word
   m_iWordOffset += (m_iWordLength + 1);
}

// changing words

bool AP_Dialog_Spell::inChangeAll(void)
{
   UT_UCSChar * bufferUnicode = _getCurrentWord();
   UT_ASSERT(bufferUnicode);
   char * bufferNormal = (char *) UT_calloc(UT_UCS4_strlen(bufferUnicode) + 1, sizeof(char));
   UT_UCS4_strcpy_to_char(bufferNormal, bufferUnicode);
   FREEP(bufferUnicode);
   const void * ent = m_pChangeAll->pick(bufferNormal);
   FREEP(bufferNormal);

   if (ent == NULL) return false;
   else {
      makeWordVisible();
      bool bRes = changeWordWith( (UT_UCSChar*) (ent) ); 
      return bRes;
   }
}

bool AP_Dialog_Spell::addChangeAll(UT_UCSChar * newword)
{
   UT_UCSChar * bufferUnicode = _getCurrentWord();
   char * bufferNormal = (char *) UT_calloc(UT_UCS4_strlen(bufferUnicode) + 1, sizeof(char));
   UT_UCS4_strcpy_to_char(bufferNormal, bufferUnicode);
   FREEP(bufferUnicode);

   // make a copy of the word for storage
   UT_UCSChar * newword2 = (UT_UCSChar*) UT_calloc(UT_UCS4_strlen(newword) + 1, sizeof(UT_UCSChar));
   UT_UCS4_strcpy(newword2, newword);
   
   m_pChangeAll->insert(bufferNormal, 
			(void *) newword2);

   FREEP(bufferNormal);
   
   return true;
}

bool AP_Dialog_Spell::changeWordWith(UT_UCSChar * newword)
{
   bool result = true;

   UT_DEBUGMSG(("changing word\n"));
   UT_DEBUGMSG(("SAM: gp: %d\n", m_pView->getPoint()));

   // very small hack to fix bug #597 - seems
   // that the focus gets shifted to the textbox instead
   // of the document, so isSelectionEmpty() returns true
   makeWordVisible ();
   m_iWordLength = UT_UCS4_strlen(newword);

#ifdef DEBUG   
   UT_UCSChar * p;
   p = newword;
   UT_DEBUGMSG(("SAM : The new word is \n"));
   for(int i=0;i<m_iWordLength;i++)
   {
	   UT_DEBUGMSG(("%c\n", (char)p[i]));
   }
#endif

   result = m_pView->cmdCharInsert(newword, m_iWordLength);
   m_pView->updateScreen();
   
   // reload block into buffer, as we just changed it
   m_pBlockBuf->truncate(0);
   bool bRes = m_pBlock->getBlockBuf(m_pBlockBuf);
   UT_ASSERT(bRes);
                 
   return result;
}

bool AP_Dialog_Spell::addToDict(void)
{
	UT_UCSChar * pBuf = (UT_UCSChar *) m_pBlockBuf->getPointer(m_iWordOffset);
	XAP_App * pApp = m_pFrame->getApp();

	// add word to the current custom dictionary
	return pApp->addWordToDict(pBuf,m_iWordLength);
}

UT_UCSChar * AP_Dialog_Spell::_getCurrentWord(void)
{
   UT_UCSChar * word = (UT_UCSChar*) UT_calloc(m_iWordLength + 1, sizeof(UT_UCSChar));
   if (word == NULL) return NULL;

   UT_UCS4Char * pBuf = (UT_UCS4Char*) m_pBlockBuf->getPointer(m_iWordOffset);
   
   for (UT_sint32 i = 0; i < m_iWordLength; i++) word[i] = (UT_UCSChar) pBuf[i];

   return word;
}

UT_UCSChar * AP_Dialog_Spell::_getPreWord(void)
{
   UT_sint32 len = m_iWordOffset - m_iSentenceStart;
   UT_UCSChar * preword = (UT_UCSChar*) UT_calloc(len + 1, sizeof(UT_UCSChar));
   if (preword == NULL) return NULL;
   
   if (len) {
      UT_UCS4Char * pBuf = (UT_UCS4Char*) m_pBlockBuf->getPointer(m_iSentenceStart);
   
      for (UT_sint32 i = 0; i < len; i++) preword[i] = (UT_UCSChar) pBuf[i];
   }
   
   return preword;
}

UT_UCSChar * AP_Dialog_Spell::_getPostWord(void)
{
   UT_sint32 len = m_iSentenceEnd - (m_iWordOffset + m_iWordLength) + 1;
   UT_UCSChar * postword = (UT_UCSChar*) UT_calloc(len + 1, sizeof(UT_UCSChar));
   if (postword == NULL) return NULL;
   
   if (len) {
      UT_UCS4Char * pBuf = (UT_UCS4Char*) m_pBlockBuf->getPointer(m_iWordOffset+m_iWordLength);
   
      for (UT_sint32 i = 0; i < len; i++) postword[i] = (UT_UCSChar) pBuf[i];
   }
   
   return postword;
}


// TODO  This function finds the beginning and end of a sentence enclosing
// TODO  the current misspelled word. Right now, it starts from the word
// TODO  and works forward/backward until finding [.!?] or EOB
// TODO  This needs to be improved badly. However, I can't think of a 
// TODO  algorithm to do so -- especially not one which could work with
// TODO  other languages very well...
// TODO  Anyone have something better?

void AP_Dialog_Spell::_updateSentenceBoundaries(void)
{
   
   UT_UCSChar* pBlockText = (UT_UCS4Char*)m_pBlockBuf->getPointer(0);
   UT_uint32 iBlockLength = m_pBlockBuf->getLength();
   
   // go back until a period is found
   m_iSentenceStart = m_iWordOffset;
   while (m_iSentenceStart > 0) {
      if (pBlockText[m_iSentenceStart] == '.' ||
	  pBlockText[m_iSentenceStart] == '?' ||
	  pBlockText[m_iSentenceStart] == '!') {
	 m_iSentenceStart++;
	 break;
      }
      m_iSentenceStart--;
   }
   
   // skip back past any whitespace
   while (pBlockText[m_iSentenceStart] == ' ' ||
	  pBlockText[m_iSentenceStart] == UCS_TAB ||
	  pBlockText[m_iSentenceStart] == UCS_LF ||
	  pBlockText[m_iSentenceStart] == UCS_VTAB ||
	  pBlockText[m_iSentenceStart] == UCS_FF) m_iSentenceStart++;
   
   // go forward until a period is found
   m_iSentenceEnd = m_iWordOffset + m_iWordLength;
   while (m_iSentenceEnd < iBlockLength) {
      if (pBlockText[m_iSentenceEnd] == '.' ||
	  pBlockText[m_iSentenceEnd] == '?' ||
	  pBlockText[m_iSentenceEnd] == '!') break;
      m_iSentenceEnd++;
   }
   if (m_iSentenceEnd == iBlockLength) m_iSentenceEnd--;

}
