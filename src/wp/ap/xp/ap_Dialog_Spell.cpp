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

#include "ap_Dialog_Spell.h"

#include "xap_EncodingManager.h"

AP_Dialog_Spell::AP_Dialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id)
{
   m_iWordOffset = 0;
   m_iWordLength = -1;

   m_iSentenceStart = 0;
   m_iSentenceEnd = 0;
   
   m_Suggestions.count = 0;
   m_Suggestions.score = NULL;
   m_Suggestions.word = NULL;

   m_pBlockBuf = NULL;
   m_pView = NULL;
   m_pSection = NULL;
   m_pBlock = NULL;
   m_pChangeAll = NULL;
   m_pIgnoreAll = NULL;

   m_bCancelled = UT_FALSE;
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
	UT_HASH_PURGEDATA(UT_UCSChar*,(*m_pChangeAll));
	DELETEP(m_pChangeAll);
	DELETEP(m_pIgnoreAll);

	_purgeSuggestions();
}

void AP_Dialog_Spell::_purgeSuggestions(void)
{
	for (int i = 0; i < m_Suggestions.count; i++)
		FREEP(m_Suggestions.word[i]);

	FREEP(m_Suggestions.word);
	FREEP(m_Suggestions.score);
	m_Suggestions.count = 0;
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
   m_pBlock = m_pSection->getFirstBlock();
			   
   m_pBlockBuf = new UT_GrowBuf(1024);
   UT_Bool bRes = m_pBlock->getBlockBuf(m_pBlockBuf);
   UT_ASSERT(bRes);
   
   m_pChangeAll = new UT_HashTable(7); // is 7 buckets adequate? too much?
   m_pIgnoreAll = new UT_HashTable(7);

   UT_DEBUGMSG(("modal spell dialog: xp init complete\n"));
}

UT_Bool AP_Dialog_Spell::nextMisspelledWord(void)
{
   UT_ASSERT(m_pBlockBuf);

   UT_UCSChar* pBlockText = m_pBlockBuf->getPointer(0);
   UT_uint32 iBlockLength = m_pBlockBuf->getLength();

   UT_ASSERT(m_pView && m_pView->getLayout() );	
   UT_Bool checkCaps = m_pView->getLayout()->getSpellCheckCaps();

   // loop until a misspelled word or end of document is hit
   for (;;) {

      // do we need to move to the next block?
      if (m_iWordOffset >= iBlockLength) {

	 // since we're done with this current block, put it
	 // in the block spell queue so squiggles will be updated
	 FL_DocLayout * docLayout = m_pSection->getDocLayout();
	 docLayout->queueBlockForBackgroundCheck(FL_DocLayout::bgcrSpelling, m_pBlock);
	 
	 m_pBlock = m_pBlock->getNext();

	 // next section, too?
	 if (m_pBlock == NULL) {
	 
	    m_pSection = (fl_DocSectionLayout*) m_pSection->getNext();
	    if (m_pSection == NULL)
	      return UT_FALSE; // end of document
	    m_pBlock = m_pSection->getFirstBlock();
	 }
	 
	 // update the buffer with our new block
	 m_pBlockBuf->truncate(0);
	 UT_Bool bRes = m_pBlock->getBlockBuf(m_pBlockBuf);
	 UT_ASSERT(bRes);
	     
	 m_iWordOffset = 0;
	 m_iSentenceStart = 0;
	 m_iSentenceEnd = 0;
	 iBlockLength = m_pBlockBuf->getLength();
	 pBlockText = m_pBlockBuf->getPointer(0);

      }
	
      // scan block for misspelled words

      // now start checking
      UT_Bool bFound;
      UT_Bool bAllUpperCase;
	
      while (m_iWordOffset < iBlockLength) {

	 // skip delimiters...
	 while (m_iWordOffset < iBlockLength)
	 {
		 UT_UCSChar followChar;
		 followChar = ((m_iWordOffset + 1) < iBlockLength)  ?  pBlockText[m_iWordOffset+1]  :  UCS_UNKPUNK;
		 if (!UT_isWordDelimiter( pBlockText[m_iWordOffset], followChar)) break;
		 m_iWordOffset++;
	 }
	     
	 // ignore initial quote
	 if (pBlockText[m_iWordOffset] == '\'')
	   m_iWordOffset++;
	     
	 if (m_iWordOffset < iBlockLength) {
	    
	    // we're at the start of a word. find end of word...
	    bAllUpperCase = UT_TRUE;
	    bFound = UT_FALSE;
	    m_iWordLength = 0;
	    while ((!bFound) && (m_iWordOffset + m_iWordLength) < iBlockLength) {

			UT_UCSChar followChar;
			followChar = ((m_iWordOffset + m_iWordLength + 1) < iBlockLength)  ?  pBlockText[m_iWordOffset + m_iWordLength + 1]  :  UCS_UNKPUNK;
	       if ( UT_TRUE == UT_isWordDelimiter( pBlockText[m_iWordOffset + m_iWordLength], followChar)) {

		  bFound = UT_TRUE;

	       } else {

		  if (bAllUpperCase)
		    bAllUpperCase = UT_UCS_isupper(pBlockText[m_iWordOffset + m_iWordLength]);
			    
		  m_iWordLength++;

	       }
	    }
		  
	    // ignore terminal quote
	    if (pBlockText[m_iWordOffset + m_iWordLength - 1] == '\'')
	      m_iWordLength--;
		  
	    // for some reason, the spell checker fails on all 1-char words & really big ones
	    // -this is a limitation in the underlying default checker ispell --JB
	    if ((m_iWordLength > 1) &&
		XAP_EncodingManager::instance->noncjk_letters(pBlockText+m_iWordOffset, m_iWordLength) && 
		(!checkCaps || !bAllUpperCase) &&             // TODO: iff relevant Option is set
		(!UT_UCS_isdigit(pBlockText[m_iWordOffset]) &&
		 (m_iWordLength < 100))) {
	    
	       // try testing our current change all lists
		  if (!inChangeAll()) {
		    
		     // try ignore all list and user dictionaries here, too
		     XAP_App * pApp = m_pFrame->getApp();

			 UT_UCSChar theWord[101];
			 //UT_DEBUGMSG(("char: %s", pBlockText[m_iWordOffset]));
			 // convert smart quote apostrophe to ASCII single quote to be compatible with ispell
			 for (int ldex=0; ldex<m_iWordLength; ++ldex)
			 {
				 UT_UCSChar currentChar;
				 currentChar = pBlockText[m_iWordOffset + ldex];
				 if (currentChar == UCS_RQUOTE) currentChar = '\'';
				 theWord[ldex] = currentChar;
			 }
			 UT_DEBUGMSG(("word: %s\n", theWord));
			 if (!m_pDoc->isIgnore(theWord, m_iWordLength) &&
			     !pApp->isWordInDict(theWord, m_iWordLength) &&
			     !SpellCheckNWord16(theWord, m_iWordLength)) {
		  
			// unknown word...
			// prepare list of possibilities
			SpellCheckSuggestNWord16(theWord, m_iWordLength,
						 &m_Suggestions);
			    
			// update sentence boundaries (so we can display
			// the misspelled word in proper context)
			if (m_iWordOffset + m_iWordLength > m_iSentenceEnd ||
			    m_iWordOffset < m_iSentenceStart) {
			   _updateSentenceBoundaries();
			}

			UT_DEBUGMSG(("misspelled word found\n"));
		     
			// return to caller
			return UT_TRUE;
			// we have all the important state information in class variables,
			// so the next call to this function will pick up at the same place.
			// this also means we'll check whatever changes they make to this word.
		     }
		  } else {
		     // we changed the word, and the block buffer has
		     // been updated, so reload the pointer and length
		     pBlockText = m_pBlockBuf->getPointer(0);
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

UT_Bool AP_Dialog_Spell::makeWordVisible(void)
{
   UT_DEBUGMSG(("making misspelled word visible in main window\n"));
   
   if (!m_pView->isSelectionEmpty())
     m_pView->cmdUnselectSelection();
   m_pView->moveInsPtTo( (PT_DocPosition) (m_pBlock->getPosition() + m_iWordOffset) );
   m_pView->extSelHorizontal(UT_TRUE, (UT_uint32) m_iWordLength);
   m_pView->updateScreen();
   
   return UT_TRUE;
}

UT_Bool AP_Dialog_Spell::addIgnoreAll(void)
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

UT_Bool AP_Dialog_Spell::inChangeAll(void)
{
   UT_UCSChar * bufferUnicode = _getCurrentWord();
   UT_ASSERT(bufferUnicode);
   char * bufferNormal = (char *) calloc(UT_UCS_strlen(bufferUnicode) + 1, sizeof(char));
   UT_UCS_strcpy_to_char(bufferNormal, bufferUnicode);
   FREEP(bufferUnicode);
   UT_HashEntry * ent = m_pChangeAll->findEntry(bufferNormal);
   FREEP(bufferNormal);

   if (ent == NULL) return UT_FALSE;
   else {
      makeWordVisible();
      UT_Bool bRes = changeWordWith( (UT_UCSChar*) (ent->pData) ); 
      return bRes;
   }
}

UT_Bool AP_Dialog_Spell::addChangeAll(UT_UCSChar * newword)
{
   UT_UCSChar * bufferUnicode = _getCurrentWord();
   char * bufferNormal = (char *) calloc(UT_UCS_strlen(bufferUnicode) + 1, sizeof(char));
   UT_UCS_strcpy_to_char(bufferNormal, bufferUnicode);
   FREEP(bufferUnicode);

   // make a copy of the word for storage
   UT_UCSChar * newword2 = (UT_UCSChar*) calloc(UT_UCS_strlen(newword) + 1, sizeof(UT_UCSChar));
   UT_UCS_strcpy(newword2, newword);
   
   UT_sint32 iRes = m_pChangeAll->addEntry(bufferNormal, NULL, (void*) newword2);

   FREEP(bufferNormal);
   
   if (iRes < 0) return UT_FALSE;
   else return UT_TRUE;
}

UT_Bool AP_Dialog_Spell::changeWordWith(UT_UCSChar * newword)
{
   UT_Bool result = UT_TRUE;

   UT_DEBUGMSG(("changing word\n"));
   UT_DEBUGMSG(("SAM: gp: %d\n", m_pView->getPoint()));

   m_iWordLength = UT_UCS_strlen(newword);

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
   UT_Bool bRes = m_pBlock->getBlockBuf(m_pBlockBuf);
   UT_ASSERT(bRes);
                 
   return result;
}

UT_Bool AP_Dialog_Spell::addToDict(void)
{
	UT_UCSChar * pBuf = (UT_UCSChar *) m_pBlockBuf->getPointer(m_iWordOffset);
	XAP_App * pApp = m_pFrame->getApp();

	// add word to the current custom dictionary
	return pApp->addWordToDict(pBuf,m_iWordLength);
}

UT_UCSChar * AP_Dialog_Spell::_getCurrentWord(void)
{
   UT_UCSChar * word = (UT_UCSChar*) calloc(m_iWordLength + 1, sizeof(UT_UCSChar));
   if (word == NULL) return NULL;

   UT_uint16 * pBuf = m_pBlockBuf->getPointer(m_iWordOffset);
   
   for (UT_sint32 i = 0; i < m_iWordLength; i++) word[i] = (UT_UCSChar) pBuf[i];

   return word;
}

UT_UCSChar * AP_Dialog_Spell::_getPreWord(void)
{
   UT_sint32 len = m_iWordOffset - m_iSentenceStart;
   UT_UCSChar * preword = (UT_UCSChar*) calloc(len + 1, sizeof(UT_UCSChar));
   if (preword == NULL) return NULL;
   
   if (len) {
      UT_uint16 * pBuf = m_pBlockBuf->getPointer(m_iSentenceStart);
   
      for (UT_sint32 i = 0; i < len; i++) preword[i] = (UT_UCSChar) pBuf[i];
   }
   
   return preword;
}

UT_UCSChar * AP_Dialog_Spell::_getPostWord(void)
{
   UT_sint32 len = m_iSentenceEnd - (m_iWordOffset + m_iWordLength) + 1;
   UT_UCSChar * postword = (UT_UCSChar*) calloc(len + 1, sizeof(UT_UCSChar));
   if (postword == NULL) return NULL;
   
   if (len) {
      UT_uint16 * pBuf = m_pBlockBuf->getPointer(m_iWordOffset+m_iWordLength);
   
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
   
   UT_UCSChar* pBlockText = m_pBlockBuf->getPointer(0);
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
