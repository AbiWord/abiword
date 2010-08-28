#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ap_Features.h"

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

#include "ap_Dialog_Grammar.h"

GrammarChecker *
AP_Dialog_Grammar::_getChecker (void)
{
	if (m_pView)
		return m_pView->getGrammarCheckerForSelection();
	
	return NULL;

}


// TODO GPB: cleanup
#if 0
bool
AP_Dialog_Grammar::_grammarCheckSentence (const UT_UCSChar * sentence, size_t len)
{
	GrammarChecker * checker = _getChecker();
	
	if (!checker)
		return true;
	
	if (checker->checkSentence(sentence, len) == GrammarChecker::CHECK_SUCCEEDED)
		{
		g_debug("check succeeded");

		return true;
		}
	
	g_debug("check fail");

	return false;
}
#endif

AP_Dialog_Grammar::AP_Dialog_Grammar(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialoggrammar")
{
   m_bIsSelection = false;

   m_iSentOffset = 0;
   m_iSentLength = -1;
   m_pSent = NULL;
   m_bSkipMistake = false;

   m_pView = NULL;
	
   m_pStartSection = NULL;
   m_pStartBlock = NULL;
   m_iStartIndex = -1;
   m_pEndSection = NULL;
   m_pEndBlock = NULL;
   m_iEndLength = 0;
   m_pCurrSection = NULL;
   m_pCurrBlock = NULL;
   m_pPreserver = NULL;

   m_bCancelled = false;
   
   nCurMistake = 0;
   m_mistakes = NULL;
   m_curMistake = NULL;
}

AP_Dialog_Grammar::~AP_Dialog_Grammar(void)
{
	UT_DEBUGMSG(("DEBUG: AP_Dialog_Grammar destructor called\n"));
	// Why clear the selection?  MS Word doesn't either
	if (m_pView) 
	{
	  if (!m_bIsSelection && m_pView->isSelectionEmpty())
	    m_pView->cmdUnselectSelection();

	  m_pView->moveInsPtTo( m_iOrigInsPoint );
	}

	DELETEP(m_pSentIterator);
	DELETEP(m_pPreserver);
	_purgeMistakes();
}

void AP_Dialog_Grammar::_purgeMistakes(void)
{
	if (!m_mistakes) return;

	// FIXME avoid asking for the checker again !!!
	GrammarChecker * checker = _getChecker();
	if (!checker)
		return;
	checker->purgeMistakes();	

	DELETEP(m_mistakes);
}

void AP_Dialog_Grammar::runModal(XAP_Frame * pFrame)
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

	m_pSentIterator = new fl_BlockSentenceIterator(m_pCurrBlock, 0);
   
	m_bSkipMistake = false;
}


/*!
 * Scan through document until we find a misspelled word or the document ends
 */
bool AP_Dialog_Grammar::nextWrongSentence(void)
{
	UT_return_val_if_fail (m_pSentIterator && m_pView && m_pView->getLayout(), false);

	// Makes this honor spelling prefs
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail (pPrefs, false);
   
	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	UT_return_val_if_fail (pPrefsScheme, false);		  
   
	bool b = false;
	pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_AutoGrammarCheck), &b);


	// Yes, I know. This is a bit anal. But it works, and I'm too tired
	// to rethink the iterator behaviour to match the requirement right
	// now. I'll fix it sometime in the future... FIXME:jskov
	//if (!m_bSkipMistake)
	//m_pSentIterator->revertToPreviousSent();
	//m_bSkipSent = false;

	UT_sint32 iPTLength;
	// loop until a sentence with mistake end of document is hit
	for (;;) 
	{
		while (m_pSentIterator->nextSentForGrammarChecking(m_pSent, m_iSentLength, m_iSentOffset, iPTLength))
		{
		   
		   // We have found a word, but if there was a selection, make
		   // sure the word lies inside the selection (this check is
		   // for the start of the selection only).
		   if (m_iStartIndex >= 0)
		   {
			   if (m_iStartIndex >= (m_iSentOffset + m_iSentLength))
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
		   
		   if (m_pCurrBlock == m_pEndBlock
			   && m_iSentOffset >= m_iEndLength)
		   {
			   // Trigger next-block handling code (below)
			   break;
		   }
		   // unknown word... update dialog

		   //makeMistakeVisible(); // display the sentence 
	   
		   GrammarChecker * checker = _getChecker();
		   if (!checker)
			   return false;

			//_purgeMistakes();


			// get mistakes from grammar checker
			if (checker->checkSentence(m_pSent, m_iSentLength) == GrammarChecker::CHECK_MISTAKES)
			{
				m_mistakes = new UT_GenericVector<GrammarMistake*>();
				UT_return_val_if_fail (m_mistakes, false);

				for (UT_uint32 i = 0; i < checker->getNMistakes(); i++)
				{
					GrammarMistake *mstk = checker->getMistake(i);

					if (mstk)
						m_mistakes->addItem(mstk);
				}

			nCurMistake = 0;

			// return to caller
			return true;
			}

		   // update sentence boundaries (so we can display
		   // the misspelled word in proper context)
		   //m_pSentIterator->updateSentenceBoundaries();

		   // we have all the important state information in
		   // class variables, so the next call to this
		   // function will pick up at the same place.  this
		   // also means we'll check whatever changes they
		   // make to this word.
	   }

	   // iterator is of no more use to us
	   DELETEP(m_pSentIterator);

	   // since we're done with this current block, put it
	   // in the block spell queue so squiggles will be updated
	   FL_DocLayout * docLayout = m_pCurrSection->getDocLayout();

	   // causes SEGV if a table is in the document!!!
	   if (b)
		   docLayout->queueBlockForBackgroundCheck(FL_DocLayout::bgcrGrammar, m_pCurrBlock);

	   // was that the last block in the selection?
	   if (m_pCurrBlock == m_pEndBlock)
		   return false;

	   // no, so move on to the next block
	   m_pCurrBlock = m_pCurrBlock->getNextBlockInDocument();
	   
	   if (m_pCurrBlock == NULL) 
	   {
		   // end of document.
		   return false;
	   }
	 
	   // update the iterator with our new block
	   m_pSentIterator = new fl_BlockSentenceIterator(m_pCurrBlock, 0);
	   UT_return_val_if_fail (m_pSentIterator, false);
   }
}

bool AP_Dialog_Grammar::nextSentenceMistake()
{
	if (nCurMistake < m_mistakes->getItemCount())
	{
		m_curMistake = m_mistakes->getNthItem(nCurMistake);
		nCurMistake++;
		return true;
	}
	return false;
}

bool AP_Dialog_Grammar::makeMistakeVisible()
{
   // Always clear selection before making a new one
   m_pView->cmdUnselectSelection();

   m_pView->moveInsPtTo( (PT_DocPosition) (m_pCurrBlock->getPosition() + m_iSentOffset + m_curMistake->iStart) );
   m_pView->extSelHorizontal(true, static_cast<UT_uint32>(m_curMistake->iEnd - m_curMistake->iStart));
   m_pView->updateScreen();
   
   return true;
}

void AP_Dialog_Grammar::ignoreMistake(void)
{
	//m_bSkipSentence = true;
}

bool AP_Dialog_Grammar::changeMistakeWith(const UT_UCSChar * newword)
{
	bool result = true;


	// very small hack to fix bug #597 - seems
	// that the focus gets shifted to the textbox instead
	// of the document, so isSelectionEmpty() returns true
	makeMistakeVisible();
	UT_sint32 iNewLength = UT_UCS4_strlen(newword); 
	UT_sint32 iOldLength = m_curMistake->iEnd - m_curMistake->iStart; //consider setting a variable to hold current mistake size
	
	// hack to update next mistakes positions in the sentence
	if (iOldLength != iNewLength)
	{
		_updateSentenceMistakesPos(iNewLength - iOldLength); 
		m_pSentIterator->updateBlock(iNewLength - iOldLength);
	}

	result = m_pPreserver->cmdCharInsert(newword, iNewLength);
	m_pView->updateScreen();
   
	// If this is the last block, adjust the end length accordingly
	// (seeing as the change must have occured before the end of the	
	// selection).
	if (m_bIsSelection && m_pEndBlock == m_pCurrBlock)
		m_iEndLength += (iNewLength - (m_curMistake->iEnd - m_curMistake->iStart));

                 
	return result;
}

void AP_Dialog_Grammar::_updateSentenceMistakesPos(UT_sint32 iOffset)
{
	// check if sentence has other detected mistakes 
	if (nCurMistake < m_mistakes->getItemCount())
	{
		for (UT_uint32 i = nCurMistake; i < m_mistakes->getItemCount(); i++)
		{
			UT_DEBUGMSG(("update Sentence mistake postion: %d %d\n"));
			GrammarMistake *old_mstk = m_mistakes->getNthItem(i);
			GrammarMistake *new_mstk = old_mstk;
			new_mstk->iStart += iOffset;
			new_mstk->iEnd += iOffset;
			m_mistakes->setNthItem(i, new_mstk, &old_mstk);
		}
		
	}
}
