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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_hash.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_App.h"

#include "ap_FrameData.h"
#include "fl_DocLayout.h"
#include "xap_Prefs.h"
#include "ap_Prefs.h"

#include "ap_Dialog_Spell.h"

// important that you call makeWordVisible before you call this
SpellChecker *
AP_Dialog_Spell::_getDict (void)
{
	if (m_pView)
	  return m_pView->getDictForSelection ();
	return NULL;
}

bool
AP_Dialog_Spell::_spellCheckWord (const UT_UCSChar * word, UT_uint32 len)
{
	SpellChecker * checker = _getDict();

	// no checker, don't mark as wrong
	if (!checker)
		return true;

	if (checker->checkWord (word, len) == SpellChecker::LOOKUP_SUCCEEDED)
		return true;
	return false;
}

AP_Dialog_Spell::AP_Dialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogspelling"), m_Suggestions(nullptr)
{
   m_bIsSelection = false;

   m_iWordOffset = 0;
   m_iWordLength = -1;
   m_pWord = NULL;
   m_bSkipWord = false;
  
   m_pView = NULL;
   m_pStartSection = NULL;
   m_pStartBlock = NULL;
   m_iStartIndex = -1;
   m_pEndSection = NULL;
   m_pEndBlock = NULL;
   m_iEndLength = 0;
   m_pCurrSection = NULL;
   m_pCurrBlock = NULL;
   m_pChangeAll = NULL;
   m_pIgnoreAll = NULL;
   m_pPreserver = NULL;

   m_bCancelled = false;
}

AP_Dialog_Spell::~AP_Dialog_Spell(void)
{
	// Why clear the selection?  MS Word doesn't either
	if (m_pView) 
	{
	  if (!m_bIsSelection && m_pView->isSelectionEmpty())
	    m_pView->cmdUnselectSelection();

	  m_pView->moveInsPtTo( m_iOrigInsPoint );
	}

	DELETEP(m_pPreserver);

	m_pChangeAll->freeData();
//	UT_HASH_PURGEDATA(UT_UCSChar*,m_pChangeAll, g_free);
	DELETEP(m_pChangeAll);
	DELETEP(m_pIgnoreAll);
	DELETEP(m_pWordIterator);

	_purgeSuggestions();
}

void AP_Dialog_Spell::_purgeSuggestions(void)
{
	if (!m_Suggestions) return;

	for (UT_sint32 i = 0; i < m_Suggestions->getItemCount(); i++)
	{
		UT_UCS4Char * sug = m_Suggestions->getNthItem(i);
		if (sug)
			g_free(sug);
	}

	DELETEP(m_Suggestions);
}

void AP_Dialog_Spell::runModal(XAP_Frame * pFrame)
{
   UT_return_if_fail (pFrame);
   m_pFrame = pFrame;
			  
   AP_FrameData * frameData = static_cast<AP_FrameData*>(m_pFrame->getFrameData());
   m_pDoc = frameData->m_pDocLayout->getDocument();
   m_pView = frameData->m_pDocLayout->getView();
   m_iOrigInsPoint = m_pView->getPoint();
   m_pPreserver = new FL_SelectionPreserver (m_pView);

   if (m_pView->isSelectionEmpty())
   {
	   m_pCurrSection = frameData->m_pDocLayout->getFirstSection();
	   m_pCurrBlock = static_cast<fl_BlockLayout *>(m_pCurrSection->getFirstLayout());
   }
   else
   {
	   PD_DocumentRange range;

	   // If some text is selected we want to check just that (first)
	   m_pView->getDocumentRangeOfCurrentSelection(&range);
   
	   m_pStartBlock = m_pView->getBlockAtPosition(range.m_pos1);
	   m_pStartSection = m_pStartBlock->getDocSectionLayout();
	   m_iStartIndex = range.m_pos1 - m_pStartBlock->getPosition();

	   m_pEndBlock = m_pView->getBlockAtPosition(range.m_pos2);
	   m_pEndSection = m_pEndBlock->getDocSectionLayout();
	   m_iEndLength = range.m_pos2 - m_pEndBlock->getPosition();

	   m_pCurrBlock = m_pStartBlock;
	   m_pCurrSection = m_pStartSection;
	   m_bIsSelection = true;
   }

   m_pWordIterator = new fl_BlockSpellIterator(m_pCurrBlock, 0);
   
   m_pChangeAll = new UT_GenericStringMap<UT_UCSChar*>(7); // is 7 buckets adequate? too much?
   m_pIgnoreAll = new UT_GenericStringMap<UT_UCSChar*>(7);

   m_bSkipWord = false;
}

/*!
 * Scan through document until we find a misspelled word or the document ends
 */
bool AP_Dialog_Spell::nextMisspelledWord(void)
{
   UT_return_val_if_fail (m_pWordIterator && m_pView && m_pView->getLayout(), false);

   // Makes this honor spelling prefs
   XAP_App * pApp = XAP_App::getApp();
   UT_return_val_if_fail (pApp, false);
   XAP_Prefs * pPrefs = pApp->getPrefs();
   UT_return_val_if_fail (pPrefs, false);

   XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
   UT_return_val_if_fail (pPrefsScheme, false);

   bool b = false;
   pPrefs->getPrefsValueBool(AP_PREF_KEY_AutoSpellCheck, b);

   // Yes, I know. This is a bit anal. But it works, and I'm too tired
   // to rethink the iterator behaviour to match the requirement right
   // now. I'll fix it sometime in the future... FIXME:jskov
   if (!m_bSkipWord)
	   m_pWordIterator->revertToPreviousWord();
   m_bSkipWord = false;

   UT_sint32 iPTLength;
   // loop until a misspelled word or end of document is hit
   for (;;) 
   {
	   while (m_pWordIterator->nextWordForSpellChecking(m_pWord, m_iWordLength, m_iWordOffset, iPTLength))
	   {
		   // We have found a word, but if there was a selection, make
		   // sure the word lies inside the selection (this check is
		   // for the start of the selection only).
		   if (m_iStartIndex >= 0)
		   {
			   if (m_iStartIndex >= (m_iWordOffset + m_iWordLength))
			   {
				   // Word is not inside the selection - skip to
				   // next one
				   continue;
			   }
			   // OK, it's inside the selection - we don't have to
			   // check for this anymore
			   m_iStartIndex = -1;
		   }

		   // If there was a selection, we may have to stop here if
		   // we're past the end length (compare the m_iWordOffset and
		   // not the end of the word, thus including a partially
		   // selected word).
		   if (m_bIsSelection && m_pCurrBlock == m_pEndBlock
			   && m_iWordOffset >= m_iEndLength)
		   {
			   // Trigger next-block handling code (below)
			   break;
		   }

		   // try testing our current change all lists
		   if (!inChangeAll()) 
		   {
			   // try ignore all list and user dictionaries here, too
			   if (!_spellCheckWord(m_pWord, m_iWordLength)) 
			   {
				   // unknown word... update dialog

				   makeWordVisible(); // display the word now.

				   SpellChecker * checker = _getDict();
				   if (!checker)
					   return false;

					_purgeSuggestions();

					// create an empty vector
					UT_ASSERT_HARMLESS(!m_Suggestions);

					m_Suggestions = new UT_GenericVector<UT_UCSChar*>();
					UT_return_val_if_fail (m_Suggestions, false);

					// get suggestions from spelling engine

					if (checker->checkWord(m_pWord, m_iWordLength) == SpellChecker::LOOKUP_FAILED)
					{
						std::unique_ptr<std::vector<UT_UCSChar*>> cpvEngineSuggestions;
						cpvEngineSuggestions = checker->suggestWord(m_pWord, m_iWordLength);

						for (UT_uint32 i = 0; i < cpvEngineSuggestions->size(); ++i)
						{
							UT_UCS4Char *sug = cpvEngineSuggestions->at(i);
							UT_return_val_if_fail (sug, false);
							m_Suggestions->addItem(sug);
						}
					}
				   // add suggestions from user's AbiWord file
				   pApp->suggestWord(m_Suggestions, m_pWord, m_iWordLength);

				   // update sentence boundaries (so we can display
				   // the misspelled word in proper context)
				   m_pWordIterator->updateSentenceBoundaries();

				   // return to caller
				   return true;
				   // we have all the important state information in
				   // class variables, so the next call to this
				   // function will pick up at the same place.  this
				   // also means we'll check whatever changes they
				   // make to this word.
			   }
		   } 
		   else 
		   {
			   UT_uint32 iOldLength = m_pWordIterator->getBlockLength();

			   // we changed the word, and the block buffer has been
			   // updated, so reload the pointer and length
			   m_pWordIterator->updateBlock();

			   // If this is the last block, adjust the end length
			   // accordingly (seeing as the change must have occurred
			   // before the end of the selection).
			   if (m_bIsSelection && m_pEndBlock == m_pCurrBlock)
				   m_iEndLength += (m_pWordIterator->getBlockLength() - iOldLength);
		   }
	   }

	   // iterator is of no more use to us
	   DELETEP(m_pWordIterator);

	   // since we're done with this current block, put it
	   // in the block spell queue so squiggles will be updated
	   FL_DocLayout * docLayout = m_pCurrSection->getDocLayout();

	   // causes SEGV if a table is in the document!!!
	   if (b)
		   docLayout->queueBlockForBackgroundCheck(FL_DocLayout::bgcrSpelling, m_pCurrBlock);

	   // was that the last block in the selection?
	   if (m_bIsSelection && m_pCurrBlock == m_pEndBlock)
		   return false;

	   // no, so move on to the next block
	   m_pCurrBlock = m_pCurrBlock->getNextBlockInDocument();
	   
	   if (m_pCurrBlock == NULL) 
	   {
		   // end of document.
		   return false;
	   }
	 
	   // update the iterator with our new block
	   m_pWordIterator = new fl_BlockSpellIterator(m_pCurrBlock, 0);
	   UT_return_val_if_fail (m_pWordIterator, false);
   }
}

bool AP_Dialog_Spell::makeWordVisible(void)
{
   // Always clear selection before making a new one
   m_pView->cmdUnselectSelection();

   m_pView->moveInsPtTo( (PT_DocPosition) (m_pCurrBlock->getPosition() + m_iWordOffset) );
   m_pView->extSelHorizontal(true, static_cast<UT_uint32>(m_iWordLength));
   m_pView->updateScreen(true);
   
   return true;
}

bool AP_Dialog_Spell::addIgnoreAll(void)
{
	_getDict ()->ignoreWord(m_pWord,m_iWordLength);
	return true;
}

void AP_Dialog_Spell::ignoreWord(void)
{
	m_bSkipWord = true;
}

// changing words

bool AP_Dialog_Spell::inChangeAll(void)
{
	UT_sint32 iLength;
	const UT_UCSChar * bufferUnicode = m_pWordIterator->getCurrentWord(iLength);
	UT_return_val_if_fail (bufferUnicode, false);
	char * bufferNormal = static_cast<char *>(UT_calloc(iLength + 1, sizeof(char)));
	UT_UCS4_strncpy_to_char(bufferNormal, bufferUnicode, iLength);
	const UT_UCSChar * ent = m_pChangeAll->pick(bufferNormal);
	FREEP(bufferNormal);

	if (ent == NULL) 
		return false;
	else {
		makeWordVisible();
		bool bRes = changeWordWith( (UT_UCSChar*) (ent) ); 
		return bRes;
	}
}

bool AP_Dialog_Spell::addChangeAll(const UT_UCSChar * newword)
{
	UT_sint32 iLength;
	const UT_UCSChar * bufferUnicode = m_pWordIterator->getCurrentWord(iLength);
	UT_return_val_if_fail (bufferUnicode, false);
	char * bufferNormal = static_cast<char *>(UT_calloc(iLength + 1, sizeof(char)));
	UT_UCS4_strncpy_to_char(bufferNormal, bufferUnicode, iLength);

   // make a copy of the word for storage
   UT_UCSChar * newword2 = static_cast<UT_UCSChar*>(UT_calloc(UT_UCS4_strlen(newword) + 1, sizeof(UT_UCSChar)));
   UT_UCS4_strcpy(newword2, newword);
   
   m_pChangeAll->insert(bufferNormal, newword2);

   FREEP(bufferNormal);
   
   return true;
}

bool AP_Dialog_Spell::changeWordWith(const UT_UCSChar * newword)
{
   bool result = true;

   // very small hack to fix bug #597 - seems
   // that the focus gets shifted to the textbox instead
   // of the document, so isSelectionEmpty() returns true
   makeWordVisible ();
   UT_sint32 iNewLength = UT_UCS4_strlen(newword);
   UT_sint32 iOldLength;

   // have the spell checker learn about our misspellings
   const UT_UCSChar * oldWord = m_pWordIterator->getCurrentWord(iOldLength);
   _getDict()->correctWord (oldWord, iOldLength, newword, iNewLength);

   result = m_pPreserver->cmdCharInsert(newword, iNewLength);
   m_pView->updateScreen(true);
   
   // If this is the last block, adjust the end length accordingly
   // (seeing as the change must have occurred before the end of the
   // selection).
   if (m_bIsSelection && m_pEndBlock == m_pCurrBlock)
	   m_iEndLength += (iNewLength - m_iWordLength);

   m_pWordIterator->updateBlock();
                 
   return result;
}

bool AP_Dialog_Spell::addToDict(void)
{
  return _getDict()->addToCustomDict(m_pWord, m_iWordLength);
}
