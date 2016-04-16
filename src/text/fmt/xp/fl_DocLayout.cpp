/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_sleep.h"
#include "fl_DocListener.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_FootnoteLayout.h"
#include "fl_FrameLayout.h"
#include "fl_BlockLayout.h"
#include "fl_TOCLayout.h"
#include "fl_ContainerLayout.h"
#ifdef ENABLE_SPELL
#include "fl_Squiggles.h"
#endif
#include "fl_AutoNum.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_TextRun.h"
#include "fp_Run.h"
#include "fp_FrameContainer.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "pp_Property.h"
#include "gr_Graphics.h"
#include "xav_Listener.h"
#include "xap_App.h"
#include "ap_Prefs.h"
#include "fp_ContainerObject.h"
#include "fp_FootnoteContainer.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_timer.h"
#include "ut_string.h"
#include "ut_mbtowc.h"
#include "xap_Frame.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "pf_Frag_Strux.h"
#include "ie_imp_RTF.h"
#include "ie_exp_RTF.h"
#include "ap_StatusBar.h"
#include "ap_FrameData.h"

#ifdef ENABLE_SPELL
#include "spell_manager.h"
#endif

#include "gr_EmbedManager.h"

#include "xap_EncodingManager.h"

#include <set>

#define REDRAW_UPDATE_MSECS	500

const FootnoteTypeDesc s_FootnoteTypeDesc[] = {
	{ FOOTNOTE_TYPE_NUMERIC, "1, 2, 3 ...", "numeric" },
	{ FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS, "[1], [2], [3] ...", "numeric-square-brackets" },
	{ FOOTNOTE_TYPE_NUMERIC_PAREN, "(1), (2), (3) ...", "numeric-paren" },
	{ FOOTNOTE_TYPE_NUMERIC_OPEN_PAREN,"1), 2), 3) ...", "numeric-open-paren" },
	{ FOOTNOTE_TYPE_LOWER, "a, b, c ...", "lower" },
	{ FOOTNOTE_TYPE_LOWER_PAREN, "(a), (b), (c) ...", "lower-paren" },
	{ FOOTNOTE_TYPE_LOWER_OPEN_PAREN, "a), b), c) ...", "lower-paren-open" },
	{ FOOTNOTE_TYPE_UPPER, "A, B, C ...", "upper" },
	{ FOOTNOTE_TYPE_UPPER_PAREN, "(A), (B), (C) ...", "upper-paren" },
	{ FOOTNOTE_TYPE_UPPER_OPEN_PAREN, "A), B), C) ...", "upper-paren-open" },
	{ FOOTNOTE_TYPE_LOWER_ROMAN, "i, ii, iii ...", "lower-roman" },
	{ FOOTNOTE_TYPE_LOWER_ROMAN_PAREN, "(i), (ii), (iii) ...", "lower-roman-paren" },
	{ FOOTNOTE_TYPE_UPPER_ROMAN, "I, II, III ...", "upper-roman" },
	{ FOOTNOTE_TYPE_UPPER_ROMAN_PAREN, "(I), (II), (III) ...", "upper-roman-paren" },
	{ _FOOTNOTE_TYPE_INVALID, NULL, NULL }
};

FL_DocLayout::FL_DocLayout(PD_Document* doc, GR_Graphics* pG)
  : m_docViewPageSize("A4"),
    m_pG(pG),
    m_pDoc(doc),
    m_pView(NULL),
    m_lid((PL_ListenerId)-1),
    m_pFirstSection(NULL),
    m_pLastSection(NULL),
	m_toSpellCheckHead(NULL),
	m_toSpellCheckTail(NULL),
    m_pPendingBlockForSpell(NULL),
    m_pPendingWordForSpell(NULL),
    m_bSpellCheckCaps(true),
    m_bSpellCheckNumbers(true),
    m_bSpellCheckInternet(true),
    m_bAutoSpellCheck(true),
    m_uDocBackgroundCheckReasons(0),
    m_bStopSpellChecking(false),
    m_bImSpellCheckingNow(false),
    m_pPendingBlockForSmartQuote(NULL),
    m_uOffsetForSmartQuote(0),
    m_pBackgroundCheckTimer(NULL),
    m_pPrefs(NULL),
    m_pRedrawUpdateTimer(NULL),
    m_iSkipUpdates(0),
    m_bDeletingLayout(false),
    m_bisLayoutFilling(false),
    m_iRedrawCount(0),
    m_iPageWidth(0),
    m_iPageHeight(0),
    m_FootnoteType(FOOTNOTE_TYPE_NUMERIC),
    m_iFootnoteVal(1),
    m_bRestartFootSection(false),
    m_bRestartFootPage(false),
    m_iEndnoteVal(1),
    m_EndnoteType(FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS),
    m_bRestartEndSection(false),
    m_bPlaceAtDocEnd(false),
    m_bPlaceAtSecEnd(true),
    m_iGraphicTick(0), 
    m_iDocSize(0),
    m_iFilled(0),
    m_bSpellCheckInProgress(false),
    m_bAutoGrammarCheck(false),
    m_PendingBlockForGrammar(NULL),
    m_iGrammarCount(0),
    m_bFinishedInitialCheck(false),
    m_iPrevPos(0),
    m_pQuickPrintGraphics(NULL),
    m_bIsQuickPrint(false),
    m_bDisplayAnnotations(false),
    m_bDisplayRDFAnchors(false),
    m_pSavedContainer(NULL),
    m_pRebuiltBlockLayout(NULL)
{
#ifdef FMT_TEST
        m_pDocLayout = this;
#endif
        setLayoutIsFilling(false),
	m_pRedrawUpdateTimer = UT_Timer::static_constructor(_redrawUpdate, this);
	if (m_pRedrawUpdateTimer && !pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
		m_pRedrawUpdateTimer->set(REDRAW_UPDATE_MSECS);
		m_pRedrawUpdateTimer->start();
	}

	_setDocPageDimensions();
	// TODO the following (both the new() and the addListener() cause
	// TODO g_try_malloc's to occur.  we are currently inside a constructor
	// TODO and are not allowed to report failure.

	// Turn off list updating until document is formatted

	m_pDoc->disableListUpdates();

	strncpy(m_szCurrentTransparentColor,
			static_cast<const char *>(XAP_PREF_DEFAULT_ColorForTransparent), 9);
	m_vecFootnotes.clear();
	m_vecAnnotations.clear();
	m_vecEndnotes.clear();

}

FL_DocLayout::~FL_DocLayout()
{
        UT_DEBUGMSG(("Deleting DocLayout %p DocListener %p lid %d\n",this,m_pDocListener,m_lid));

	m_bDeletingLayout = true;
	if (m_pPrefs)
	{
		m_pPrefs->removeListener ( _prefsListener, this );
	}

	if (m_pDoc)
	{
		m_pDoc->removeListener(m_lid);
	}

	DELETEP(m_pDocListener);

	if (m_pBackgroundCheckTimer)
	{
		m_bStopSpellChecking = true;
		m_pBackgroundCheckTimer->stop();
	}

	DELETEP(m_pBackgroundCheckTimer);
	DELETEP(m_pPendingWordForSpell);

	if (m_pRedrawUpdateTimer)
	{
		m_pRedrawUpdateTimer->stop();
	}

	DELETEP(m_pRedrawUpdateTimer);

	UT_sint32 count = m_vecPages.getItemCount() -1;
	while(count >= 0)
	{
		fp_Page * pPage = static_cast<fp_Page *>(m_vecPages.getNthItem(count));
		if(pPage->getPrev())
		{
			pPage->getPrev()->setNext(NULL);
		}
		m_vecPages.deleteNthItem(count);
		delete pPage;
		count--;
	}

	while (m_pFirstSection)
	{
		fl_DocSectionLayout* pNext = m_pFirstSection->getNextDocSection();
		delete m_pFirstSection;
		m_pFirstSection = pNext;
	}
	std::set<GR_EmbedManager *> garbage;
	std::map<std::string, GR_EmbedManager *>::iterator i, iend;
	iend = m_mapEmbedManager.end();
	for (i = m_mapEmbedManager.begin(); i != iend; i++)
		if ((*i).first == (*i).second->getObjectType())
			garbage.insert((*i).second);
	m_mapEmbedManager.clear();
	iend = m_mapQuickPrintEmbedManager.end();
	for (i = m_mapQuickPrintEmbedManager.begin(); i != iend; i++)
		if ((*i).first == (*i).second->getObjectType())
			garbage.insert((*i).second);
	m_mapQuickPrintEmbedManager.clear();
	std::set<GR_EmbedManager *>::iterator j, jend = garbage.end();
	for (j = garbage.begin(); j != jend; j++)
		delete *j;
	garbage.clear();
}

/*!
 * Set the variables needed for a QuickPrint
 */
void  FL_DocLayout::setQuickPrint(GR_Graphics * pGraphics)
{
	std::set<GR_EmbedManager *> garbage;
	std::map<std::string, GR_EmbedManager *>::iterator i, iend;
	iend = m_mapQuickPrintEmbedManager.end();
	for (i = m_mapQuickPrintEmbedManager.begin(); i != iend; i++)
		if ((*i).first == (*i).second->getObjectType())
			garbage.insert((*i).second);
	m_mapQuickPrintEmbedManager.clear();
	std::set<GR_EmbedManager *>::iterator j, jend = garbage.end();
	for (j = garbage.begin(); j != jend; j++)
		delete *j;
	garbage.clear();
	if(pGraphics != NULL)
	{
	    m_bIsQuickPrint = true;
	    m_pQuickPrintGraphics = pGraphics;
	}
	else
	{
	    m_bIsQuickPrint = false;
	    m_pQuickPrintGraphics = NULL;
	    fl_BlockLayout * pBL = getFirstSection()->getFirstBlock();
	    //
	    // Clear out any hanging pointers
	    //
	    while(pBL)
	    {
		pBL->clearPrint();
		pBL = pBL->getNextBlockInDocument();
	    }
	    //
	    // Ensure all fonts are owned by the original graphics class
	    //
	    refreshRunProperties();
	}
}

GR_Graphics * FL_DocLayout::getQuickPrintGraphics(void) const
{
  return  m_pQuickPrintGraphics;
}

/*!
 * Get an embedManager of the requested Type.for a quickPrint
 */
GR_EmbedManager * FL_DocLayout::getQuickPrintEmbedManager(const char * szEmbedType)
{
  // Look in the current collection first.
   GR_EmbedManager * pEmbed = NULL;
  std::map<std::string, GR_EmbedManager *>::iterator i;
  if ((i = m_mapQuickPrintEmbedManager.find(szEmbedType)) != m_mapQuickPrintEmbedManager.end())
    return (*i).second;
  pEmbed = XAP_App::getApp()->getEmbeddableManager(m_pQuickPrintGraphics,szEmbedType);
  if((strcmp(pEmbed->getObjectType(),"default") == 0) &&
     ((i = m_mapQuickPrintEmbedManager.find("default")) != m_mapQuickPrintEmbedManager.end()))
    {
      delete pEmbed;
      return (*i).second;
    }
  UT_DEBUGMSG(("Got manager of type %s \n",pEmbed->getObjectType()));
  if (strcmp(pEmbed->getObjectType(), szEmbedType) != 0)
	{
      if ((i = m_mapQuickPrintEmbedManager.find(pEmbed->getObjectType())) != m_mapQuickPrintEmbedManager.end())
        {
          m_mapQuickPrintEmbedManager[szEmbedType] = (*i).second;
          delete pEmbed;
          return (*i).second;
	    }
      m_mapQuickPrintEmbedManager[pEmbed->getObjectType()] = pEmbed;
    }
  m_mapQuickPrintEmbedManager[szEmbedType] = pEmbed;
  pEmbed->initialize();
  
  return pEmbed;
}

/*!
 * Get an embedManager of the requested Type.
 */
GR_EmbedManager * FL_DocLayout::getEmbedManager(const char * szEmbedType)
{
  // Look in the current collection first.
  GR_EmbedManager * pEmbed = NULL;
  std::map<std::string, GR_EmbedManager *>::iterator i;
  if ((i = m_mapEmbedManager.find(szEmbedType)) != m_mapEmbedManager.end())
    return (*i).second;
  pEmbed = XAP_App::getApp()->getEmbeddableManager(m_pG,szEmbedType);
  if((strcmp(pEmbed->getObjectType(),"default") == 0) &&
     ((i = m_mapEmbedManager.find("default")) != m_mapEmbedManager.end()))
    {
      delete pEmbed;
      return (*i).second;
    }
  UT_DEBUGMSG(("Got manager of type %s \n",pEmbed->getObjectType()));
  if (strcmp(pEmbed->getObjectType(), szEmbedType) != 0)
	{
      if ((i = m_mapEmbedManager.find(pEmbed->getObjectType())) != m_mapEmbedManager.end())
        {
          m_mapEmbedManager[szEmbedType] = (*i).second;
          delete pEmbed;
          return (*i).second;
	    }
      m_mapEmbedManager[pEmbed->getObjectType()] = pEmbed;
    }
  m_mapEmbedManager[szEmbedType] = pEmbed;
  pEmbed->initialize();
  
  return pEmbed;
}

/*! 
 * little helper method for lookup properties
 */
FootnoteType FL_DocLayout::FootnoteTypeFromString(const gchar * pszFootnoteType)
{
	FootnoteType iFootnoteType;
	if (pszFootnoteType == NULL)
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC;
	}
	else if(pszFootnoteType[0] == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC;
	}
	else if(strcmp(pszFootnoteType,"numeric") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC;
	}
	else if(strcmp(pszFootnoteType,"numeric-square-brackets") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS;
	}
	else if(strcmp(pszFootnoteType,"numeric-paren") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC_PAREN;
	}
	else if(strcmp(pszFootnoteType,"numeric-open-paren") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC_OPEN_PAREN;
	}
	else if(strcmp(pszFootnoteType,"upper") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_UPPER;
	}
	else if(strcmp(pszFootnoteType,"upper-paren") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_UPPER_PAREN;
	}
	else if(strcmp(pszFootnoteType,"upper-paren-open") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_UPPER_OPEN_PAREN;
	}
	else if(strcmp(pszFootnoteType,"lower") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_LOWER;
	}
	else if(strcmp(pszFootnoteType,"lower-paren") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_LOWER_PAREN;
	}
	else if(strcmp(pszFootnoteType,"lower-paren-open") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_LOWER_OPEN_PAREN;
	}
	else if(strcmp(pszFootnoteType,"lower-roman") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_LOWER_ROMAN;
	}
	else if(strcmp(pszFootnoteType,"lower-roman-paren") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_LOWER_ROMAN_PAREN;
	}
	else if(strcmp(pszFootnoteType,"upper-roman") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_UPPER_ROMAN;
	}
	else if(strcmp(pszFootnoteType,"upper-roman-paren") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_UPPER_ROMAN_PAREN;
	}
	else
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS;
	}
	return iFootnoteType;
}

/*!
 * This method reads the document properties and saves local versions of them
 * here.
 */
void FL_DocLayout::_lookupProperties(void)
{
	const gchar * pszFootnoteType = NULL;
	const PP_AttrProp* pDocAP = getDocument()->getAttrProp();
	UT_return_if_fail(pDocAP);
	pDocAP->getProperty("document-footnote-type", (const gchar *&)pszFootnoteType);
	m_FootnoteType = FootnoteTypeFromString(pszFootnoteType);

	const gchar * pszEndnoteType = NULL;
	pDocAP->getProperty("document-endnote-type", (const gchar *&)pszEndnoteType);
	m_EndnoteType = FootnoteTypeFromString(pszEndnoteType);

	const gchar * pszTmp = NULL;
	pDocAP->getProperty("document-footnote-initial", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		m_iFootnoteVal =  atoi(pszTmp);
	}
	else
	{
		m_iFootnoteVal = 1;
	}

	pDocAP->getProperty("document-footnote-restart-section", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(strcmp(pszTmp,"1") == 0)
		{
			m_bRestartFootSection = true;
		}
		else
		{
			m_bRestartFootSection = false;
		}
	}
	else
	{
		m_bRestartFootSection = false;
	}

	pDocAP->getProperty("document-footnote-restart-page", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(strcmp(pszTmp,"1") == 0)
		{
			m_bRestartFootPage = true;
		}
		else
		{
			m_bRestartFootPage = false;
		}
	}
	else
	{
		m_bRestartFootPage = false;
	}

	pDocAP->getProperty("document-endnote-initial", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		m_iEndnoteVal =  atoi(pszTmp);
	}
	else
	{
		m_iEndnoteVal = 1;
	}

	pDocAP->getProperty("document-endnote-restart-section", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(strcmp(pszTmp,"1") == 0)
		{
			m_bRestartEndSection = true;
		}
		else
		{
			m_bRestartEndSection = false;
		}
	}
	else
	{
		m_bRestartEndSection = false;
	}

	pDocAP->getProperty("document-endnote-place-endsection", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(strcmp(pszTmp,"1") == 0)
		{
			m_bPlaceAtDocEnd = false;
		}
		else
		{
			m_bPlaceAtDocEnd = true;
		}
	}
	else
	{
		m_bPlaceAtDocEnd = false;
	}

	pDocAP->getProperty("document-endnote-place-enddoc", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(strcmp(pszTmp,"1") == 0)
		{
			m_bPlaceAtSecEnd = false;
		}
		else
		{
			m_bPlaceAtSecEnd = true;
		}
	}
	else
	{
		m_bPlaceAtSecEnd = true;
	}

}

void FL_DocLayout::updatePropsNoRebuild(void)
{
	_lookupProperties();
}

/*!
 * Change the graphics pointer for this layout.Needed for quick zoom.
 */
void FL_DocLayout::setGraphics(GR_Graphics * pG)
{
	m_pG = pG;
	m_iGraphicTick++;

	// we need to at least re-acquire a new copy of the fonts
	// since the last ones' lives are attached to the old
	// graphics class
	updatePropsRebuild();
}

void FL_DocLayout::updatePropsRebuild(void)
{
	_lookupProperties();
	rebuildFromHere( m_pFirstSection);
}

FootnoteType FL_DocLayout::getFootnoteType(void) const
{
	return m_FootnoteType;
}


void FL_DocLayout::clearAllCountWraps(void)
{
  UT_sint32 i = 0;
  for(i=0; i<countPages();i++)
  {
      getNthPage(i)->clearCountWrapNumber();
  }
}
/*!
 * This Method fills the layout structures from the PieceTable.
 */
void FL_DocLayout::fillLayouts(void)
{
	_lookupProperties();
	setLayoutIsFilling(true);
	AP_StatusBar * pStatusBar = NULL;
	m_docViewPageSize = getDocument()->m_docPageSize;
	_setDocPageDimensions();
	if(m_pView)
	{
		m_pView->setPoint(0);
		m_pView->setLayoutIsFilling(true);
		if(m_pView->getParentData())
		{
            if(AP_FrameData * pData =  static_cast<AP_FrameData *>(static_cast<XAP_Frame *>(m_pView->getParentData())->getFrameData()))
            {
				pStatusBar = static_cast<AP_StatusBar *>(pData->m_pStatusBar);
				if(pStatusBar)
				{
					pStatusBar->setStatusProgressType(0,100,PROGRESS_STARTBAR);
					pStatusBar->showProgressBar();
				}
            }
            
		}
	}
	m_pDoc->getBounds(true,m_iDocSize);
//
// Make a document listner to get info pumped into the layouts.
//
	m_pDocListener = new fl_DocListener(m_pDoc, this);
	UT_return_if_fail(m_pDocListener);
//
// The act of adding the listner to the document also causes the
// the document to pump it's content into the layout classes.
//
	m_pDoc->setDontImmediatelyLayout(true);
	m_pDocListener->setHoldTableLayout(false);
	m_pDoc->addListener(static_cast<PL_Listener *>(m_pDocListener),&m_lid);
	m_pDoc->setDontImmediatelyLayout(false);
	UT_ASSERT(m_lid != (PL_ListenerId)-1);
	GR_Graphics * pG = getGraphics();
	formatAll(); // Do we keep this or not?
	m_bFinishedInitialCheck = false;
	m_iPrevPos = 0;
	m_iGrammarCount = 0;
	if(m_pView)
	{
		m_pView->setLayoutIsFilling(false);
		setLayoutIsFilling(false);
		m_pView->moveInsPtTo(FV_DOCPOS_BOD);
		m_pView->clearCursorWait();
		m_pView->updateLayout();
		if(!pG->queryProperties(GR_Graphics::DGP_PAPER))
		{
			m_pView->updateScreen(false);
			XAP_Frame * pFrame = static_cast<XAP_Frame *>(m_pView->getParentData());
			if(pFrame)
			{
				pFrame->setYScrollRange();
			}
		}
	}
	setLayoutIsFilling(false);
	if(!m_pView)
	{
		updateLayout();
	}

	// Layout of any TOC that is built only from a restricted document range is tentative, because
	// the information required by the TOC might not have been available during the incremental
	// load.  In that case the TOCs made made certain assumptions about the presence of a given
	// bookmark in the doc during the fill. These assumptions now need to be verified and, if
	// required, fixed
	
	fl_TOCLayout* pBadTOC = NULL;
	
	//
	// Maybe one day we can fill TOC's directly from the 
	// PT before doing the layout of the rest of the document.
	//
	for (UT_sint32 i = 0; i < getNumTOCs(); ++i)
	{
		fl_TOCLayout * pTOC = getNthTOC(i);
		if(!pTOC)
		{
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			continue;
		}
		if (pTOC->isTOCEmpty())
		{
			pTOC->fillTOC();
			m_pView->updateLayout();
		}

		// because the incremental load is sequential, the TOCs are in the order they have in the
		// document, so we just need to remember the first one.
		if(!pBadTOC && pTOC->verifyBookmarkAssumptions())
		{
			pBadTOC = pTOC;
		}
	}

	if(pBadTOC)
	{
		// hard luck -- we need to redo the layout, since the TOC probably changed size
		fl_SectionLayout * pSL = pBadTOC->getSectionLayout();
		fl_DocSectionLayout * pDSL = NULL;
		
		if(pSL->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			pDSL = static_cast<fl_DocSectionLayout*>(pSL);
		}
		else
		{
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		}

		if(!pDSL)
		{
			formatAll();
		}
		else
		{
			while (pSL)
			{
				pSL->format();
				if(pSL->getContainerType() == FL_CONTAINER_DOCSECTION)
				{
					static_cast<fl_DocSectionLayout *>(pSL)->completeBreakSection();
					static_cast<fl_DocSectionLayout *>(pSL)->checkAndRemovePages();
				}
				pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
			}
		}
		
		if(m_pView)
		{
			m_pView->updateLayout();
			if(!pG->queryProperties(GR_Graphics::DGP_PAPER))
			{
			  //				m_pView->updateScreen(false);
				XAP_Frame * pFrame = static_cast<XAP_Frame *>(m_pView->getParentData());
				if(pFrame)
				{
					pFrame->setYScrollRange();
				}
			}
		}
	}

	// Frame related tasks

	if (m_vecFramesToBeInserted.getItemCount() > 0)
	{
		// There is a mismatch between the new layout and the saved file.
		// The requested page for some frames does not exists.
		// Insert all remaining frames on the last page.
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		fp_FrameContainer * pFrame = NULL;
		UT_sint32 k = 0;
		UT_sint32 kmax = m_vecFramesToBeInserted.getItemCount();
		fp_Page * pPage = getLastPage();
		for (k = 0; k < kmax; k++)
		{
			pFrame = m_vecFramesToBeInserted.getNthItem(0);
			m_vecFramesToBeInserted.deleteNthItem(0);
			pPage->insertFrameContainer(pFrame);
		}		
	}

	setFramePageNumbers(0);
	loadPendingObjects();
	//
	// One more time!
	//
	setFramePageNumbers(0);
	//
	// Fix all Lists
	//
	getDocument()->enableListUpdates();
	for(UT_uint32 j =0; j<getDocument()->getListsCount();j++)
	{
	    fl_AutoNum * pAuto = getDocument()->getNthList(j);
	    pAuto->markAsDirty();
	}
	getDocument()->updateDirtyLists();
	if(pStatusBar)
	{
	  pStatusBar->setStatusProgressType(0,100,PROGRESS_STOPBAR);
	  pStatusBar->hideProgressBar();
	}
}

/*!
 * This method loads all the pending objects that could not be loaded
 * until the rest of the text is layed out.
 */
bool FL_DocLayout::loadPendingObjects(void)
{
        FV_View * pView = getView();
	if(!pView)
	        return false;
	PD_Document * pDoc = getDocument();
        ImagePage * pImagePage = NULL;
	UT_sint32 i = 0;
	pImagePage = pDoc->getNthImagePage(i);
	UT_UTF8String sVal,sProp;
	bool bOK = false;
	PT_DocPosition pos = 0;
	fp_Page * pPage = NULL;
	UT_UTF8String allProps;
	fl_DocSectionLayout * pDSL = NULL;
	for(i=0;pImagePage;pImagePage = pDoc->getNthImagePage(++i))
        {
		UT_UTF8String sID = *pImagePage->getImageId();
		allProps = *pImagePage-> getProps();
		bOK = AnchoredObjectHelper(pImagePage->getXInch(),
					   pImagePage->getYInch(),
					   pImagePage->getPageNo(),
					   allProps,
					   pos,
					   pPage);
		if(!bOK)
		    continue;

		// Props neeed for the image
		
		sProp="frame-type";
		sVal="image";
		UT_UTF8String_setProperty(allProps,sProp,sVal);

	  //
	  // Now define the Frame attributes strux
	  //
		PP_PropertyVector attributes = {
			PT_STRUX_IMAGE_DATAID, sID.utf8_str(),
			"props", allProps.utf8_str()
		};
		pf_Frag_Strux * pfFrame = NULL;
		pDoc->insertStrux(pos, PTX_SectionFrame, attributes, PP_NOPROPS, &pfFrame);
		PT_DocPosition posFrame = pfFrame->getPos();
		pDoc->insertStrux(posFrame+1,PTX_EndFrame);
		pView->insertParaBreakIfNeededAtPos(posFrame+2);
		//
		// Now rebreak from this page forward.
		pDSL = pPage->getOwningSection();
		pDSL->setNeedsSectionBreak(true,pPage);
		while(pDSL)
		{
		    pDSL->format();
		    pDSL = pDSL->getNextDocSection();
		}
		//
		// Get Next ImagePage
		//

	}
	i = 0;
	TextboxPage * pTBPage = pDoc->getNthTextboxPage(i);
	for(i=0;pTBPage;pTBPage = pDoc->getNthTextboxPage(++i))
	{
	    allProps = *pTBPage->getProps();
	    bOK = AnchoredObjectHelper(pTBPage->getXInch(),
				       pTBPage->getYInch(),
				       pTBPage->getPageNo(),
				       allProps,
				       pos,
				       pPage);
	    if(!bOK)
	        continue;

	    // Props neeed for the Text box

	    sProp="frame-type";
	    sVal="textbox";
	    UT_UTF8String_setProperty(allProps,sProp,sVal);

	  //
	  // Now define the Frame attributes strux
	  //
	    PP_PropertyVector attributes = {
			"props", allProps.utf8_str()
		};
	    pf_Frag_Strux * pfFrame = NULL;
	    pDoc->insertStrux(pos, PTX_SectionFrame, attributes, PP_NOPROPS, &pfFrame);
	    PT_DocPosition posFrame = pfFrame->getPos();
	    pDoc->insertStrux(posFrame+1,PTX_EndFrame);
	    pDoc->insertStrux(posFrame+1,PTX_Block);
	    pView->insertParaBreakIfNeededAtPos(posFrame+3);

	    //
	    // Now insert the content
	    //

	    const UT_ByteBuf * pBuf = pTBPage->getContent();
	    PD_DocumentRange docRange(pDoc, posFrame+1,posFrame+1);
	    IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(pDoc);
	    const unsigned char * pData = static_cast<const unsigned char *>(pBuf->getPointer(0));
	    UT_uint32 iLen = pBuf->getLength();
	    pImpRTF->pasteFromBuffer(&docRange,pData,iLen);
	    delete pImpRTF;
	    //
	    // Now rebreak from this page forward.
	    pDSL = pPage->getOwningSection();
	    pDSL->setNeedsSectionBreak(true,pPage);
	    while(pDSL)
	    {
		pDSL->format();
		pDSL = pDSL->getNextDocSection();
	    }
	}
	//
	// Remove all pending objects. They've now been loaded.
	//
	pDoc->clearAllPendingObjects();
        return true;
}

/*!
 * Returns true if it founds a valid pos to insert the postioned object
 */
bool FL_DocLayout::AnchoredObjectHelper(double x, double y, UT_sint32 iPage, UT_UTF8String & allProps, PT_DocPosition & pos, fp_Page *& pPage)
{
	UT_UTF8String sVal,sProp;
	iPage = iPage -1; // Start from page 1
	if(iPage>=m_vecPages.getItemCount())
	    iPage = m_vecPages.getItemCount()-1;
	pPage = m_vecPages.getNthItem(iPage);
	UT_sint32 xPos = UT_LAYOUT_RESOLUTION*x;
	UT_sint32 yPos = UT_LAYOUT_RESOLUTION*y;
	bool bBOL,bEOL,isTOC;
	pPage->mapXYToPosition(xPos,yPos, pos, bBOL,bEOL,isTOC);
	//
	// Set the image position in the frame properties as well
	// as the properties that define this as a positioned image
	// positioned relative to a page.
	//
	sVal = UT_formatDimensionedValue(x,"in", NULL);
	sProp="frame-page-xpos";
	UT_UTF8String_setProperty(allProps,sProp,sVal);
	sVal = UT_formatDimensionedValue(y,"in", NULL);
	sProp="frame-page-ypos";
	UT_UTF8String_setProperty(allProps,sProp,sVal);
	sProp="position-to";
	sVal="page-above-text";
	UT_UTF8String_setProperty(allProps,sProp,sVal);

	//
	// Position the object immediately after the closest block
	//
	fl_BlockLayout * pBL = findBlockAtPosition(pos);
	if(pBL == NULL)
	{
	    return false;
	}
	//
	// This should place the the frame strux immediately after the 
	// block containing position posXY.
	// It returns the Frag_Strux of the new frame.
	//

	fl_BlockLayout * pPrevBL = pBL;
	while(pBL && ((pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_ENDNOTE) || (pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_FOOTNOTE) || (pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_TOC)|| (pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_FRAME)))
	{
	    UT_DEBUGMSG(("Skipping Block %p \n",pBL));
	    pPrevBL = pBL;
	    pBL = pBL->getPrevBlockInDocument();
	}
	if(pBL == NULL)
	{
	    pBL = pPrevBL;
	}
	UT_ASSERT((pBL->myContainingLayout()->getContainerType() != FL_CONTAINER_HDRFTR) 
		  && (pBL->myContainingLayout()->getContainerType() != FL_CONTAINER_SHADOW));
	pos = pBL->getPosition();
	return true;
}

/*!
 * Code to deal with Dangling Pointers in fb_ColumnBreaker. Unfortunately there is no way to avoid the
 * problem that some containers will get deleted during a page wrap. Even more unfortunately we need to
 * hold a pointer to a container in fb_ColumnBreaker that coukd get deleted. This code allows us to detect
 * and repair the damage when this occurs.
 */
/*!
 * The save the pointer that might be left dangling. This methos is called from fb_ColumnBreak.
 * We set a boolean inside this class that tells us we need
 * signal fl_Doclayout that it has been deleted. The pointer m_pRebuiltBlock is the block that contains the
 * Container. The pointer to it will be set within BlockLayout when the container gets deleted
 */
void FL_DocLayout::setSaveContainerPointer( fp_Container * pContainer)
{
        m_pSavedContainer = pContainer;
	pContainer->setAllowDelete(false);
	m_pRebuiltBlockLayout = NULL;
}

void FL_DocLayout::setRebuiltBlock(fl_BlockLayout *pBlock)
{
  m_pRebuiltBlockLayout = pBlock; 
}

fl_BlockLayout * FL_DocLayout::getRebuiltBlock(void) const
{
  return m_pRebuiltBlockLayout;
}

fp_Container * FL_DocLayout::getSavedContainerPointer(void) const
{
  return m_pSavedContainer;
}

#if 0
// Don't think we need this code after moving this functionality to
// fp_ColumnBreaker::breakSection()! FIXME remove if we're sure we dont!
/*!
 * This method returns true of the document is not completely layed out. This
 * happens in documents with for example a large TOC and a bunch of footnotes
 * (the RTF 1.7 spec is a good example).
 *
 * This is a hack; BreakSection should automatically detect this. For now
 * it works though :) - MARCM
 */
bool FL_DocLayout::needsRebreak(void)
{
    bool bRebreak = false;
    fl_DocSectionLayout * pLastSec = getLastSection();
    if(pLastSec)
    {
        fl_ContainerLayout * pCL = pLastSec->getLastLayout();
	fl_BlockLayout * pBL = NULL;
	if(pCL && (pCL->getContainerType() == FL_CONTAINER_BLOCK))
        {
	    pBL = static_cast<fl_BlockLayout *>(pCL);
	}
	else if(pCL)
	{
	    pBL = pCL->getPrevBlockInDocument();
	}
	else
	{
	    UT_ASSERT_HARMLESS(pCL);
	}
	if(pBL)
	{
	    fp_Line * pLine = static_cast<fp_Line *>(pBL->getLastContainer());
	    if(pLine == NULL)
	    {
	        return true;
	    }
	    fp_Page * pPage = pLine->getPage();
	    if(pPage == NULL)
	    {
	        return true;
	    }
	    else if(pLine->getY() > pPage->getHeight())
	    {
		fl_DocSectionLayout * pDSL= pPage->getOwningSection();
		pDSL->setNeedsSectionBreak(true,pPage);
		pDSL->format();
		return true;
	    }
	    else
	    {
	        UT_sint32 iPage = 1;
		pPage = getFirstPage();
		while(pPage && pPage != pLine->getPage())
		{
		    pPage = pPage->getNext();
		    iPage++;
		}
		iPage--;
		if(iPage != countPages())
		{
		    return true;
		}
		if(pLine->getPage() != pPage)
		{
		    return true;
		}
	    }
	}
	
    }
    return bRebreak;
}

void FL_DocLayout::Rebreak(void)
{
    fl_DocSectionLayout * pDSL = getFirstSection();
    while(pDSL)
    {
	pDSL->completeBreakSection();
	pDSL = pDSL->getNextDocSection();
    }

    //
    // Finally set all page numbers in frames
    //
    setFramePageNumbers(0);
}
#endif


/*!
 *  This method is used to reset the colorization such as what occurs
 * when showAuthors state is changed.
 */ 
void FL_DocLayout::refreshRunProperties(void)
{
    fl_DocSectionLayout * pDSL = getFirstSection();
    fl_BlockLayout * pBL = pDSL->getFirstBlock();
    while(pBL)
    {
        pBL->refreshRunProperties();
	pBL = pBL->getNextBlockInDocument();
    }
}

/*!
 * Starting from page iStartPage, set the page numbers of the frames in the
 * document.
 */
void FL_DocLayout::setFramePageNumbers(UT_sint32 iStartPage)
{
      UT_sint32 iPage = 0;
      fp_Page * pPage = NULL;
      for(iPage=iStartPage; iPage<countPages();iPage++)
      {
	  pPage = getNthPage(iPage);
	  pPage->setPageNumberInFrames();
      }
}

/*!
 * relocate the frame given to a new block. This involves changing the piece table as the 
 * frame strux is placed immediately after its parent block strux.
 * The function returns the pointer to the new frame layout. 
 */
fl_FrameLayout * FL_DocLayout:: relocateFrame(fl_FrameLayout * pFL, fl_BlockLayout * newBlock, 
											  const PP_PropertyVector & attributes, const PP_PropertyVector & properties)
{
	if(m_pDoc->isDoingTheDo())
	{
		return(pFL);
	}
	m_pDoc->beginUserAtomicGlob();
	const PP_AttrProp* pFrameAP = NULL;
	PP_AttrProp * pUpdatedFrameAP = NULL;
	pFL->getAP(pFrameAP);
	pUpdatedFrameAP = pFrameAP->cloneWithReplacements(attributes, properties, false);

	// Copy the frame content to clipboard
	bool isTextBox = (pFL->getFrameType() < FL_FRAME_WRAPPER_IMAGE);
	PT_DocPosition posStart = pFL->getPosition(true);
	PT_DocPosition posEnd = posStart + pFL->getLength();
	UT_ByteBuf * pLocalBuf = new UT_ByteBuf;
	if(isTextBox)
	{
		PD_DocumentRange dr_oldFrame;
		dr_oldFrame.set(m_pDoc,posStart+1,posEnd-1);
		IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(m_pDoc);
		PD_DocumentRange docRange(m_pDoc, posStart+1,posEnd-1);
		pExpRtf->copyToBuffer(&docRange,pLocalBuf);
		delete pExpRtf;
	}

	// Delete Frame
	pf_Frag_Strux* sdhStart =  pFL->getStruxDocHandle();
	pf_Frag_Strux* sdhEnd = NULL;
	posStart = m_pDoc->getStruxPosition(sdhStart);
	m_pDoc->getNextStruxOfType(sdhStart, PTX_EndFrame, &sdhEnd);
	if(sdhEnd == NULL)
	{
		posEnd = posStart+1;
	}
	else
	{
		posEnd = m_pDoc->getStruxPosition(sdhEnd)+1;
	}
	UT_uint32 iRealDeleteCount;
	PP_AttrProp * p_AttrProp_Before = NULL;
	m_pDoc->deleteSpan(posStart, posEnd, p_AttrProp_Before, iRealDeleteCount,true);
	pFL = NULL;
	// Insert the new frame struxes
	pf_Frag_Strux * pfFrame = NULL;
	m_pDoc->insertStrux(newBlock->getPosition(),PTX_SectionFrame,pUpdatedFrameAP->getAttributes(),
						pUpdatedFrameAP->getProperties(),&pfFrame);
	PT_DocPosition posFrame = pfFrame->getPos();
	m_pDoc->insertStrux(posFrame+1,PTX_EndFrame);
	m_pView->insertParaBreakIfNeededAtPos(posFrame+2);
	// paste in the content of the frame.
	if(isTextBox)
	{
		PD_DocumentRange docRange(m_pDoc,posFrame+1,posFrame+1);
		IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(m_pDoc);
		const unsigned char * pData = static_cast<const unsigned char *>(pLocalBuf->getPointer(0));
		UT_uint32 iLen = pLocalBuf->getLength();		
		pImpRTF->pasteFromBuffer(&docRange,pData,iLen);
		delete pImpRTF;
	}
	delete pLocalBuf;
	m_pDoc->endUserAtomicGlob();

	fl_ContainerLayout * pNewFL = pfFrame->getFmtHandle(m_lid);
	if (pNewFL && (pNewFL->getContainerType() == FL_CONTAINER_FRAME))
	{
		return (static_cast <fl_FrameLayout *>(pNewFL));
	}
	else
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return NULL;
	}
}

/*! 
  add a frame to the list of frames that need to be inserted on a page later in the document than its 
  parent block. A frame can be placed up to 3 pages after its parent block. This list is needed during 
  the initial layout stage.
 */

bool FL_DocLayout::addFramesToBeInserted(fp_FrameContainer * pFrame)
{
	m_vecFramesToBeInserted.addItem(pFrame);
	return true;
}

/*! 
  remove a frame from the list of frames that need to be inserted on a page later in the document.
 */

bool FL_DocLayout::removeFramesToBeInserted(fp_FrameContainer * pFrame)
{
	UT_sint32 i = m_vecFramesToBeInserted.findItem(pFrame);
	if(i < 0)
	{
		return false;
	}
	m_vecFramesToBeInserted.deleteNthItem(i);
	return true;
}

/*! 
  find a frame that needs to be inserted on page pPage. Only frames that are inserted on a page later
  in the document than their parent block are placed in this list. This list is needed during 
  the initial layout stage.
 */

fp_FrameContainer * FL_DocLayout::findFramesToBeInserted(fp_Page * pPage)
{
	UT_sint32 count = m_vecFramesToBeInserted.getItemCount();
	if (count == 0)
	{
		return NULL;
	}

	UT_sint32 iPage = pPage->getPageNumber();
	UT_sint32 k = 0;
	fp_FrameContainer * pFrame = NULL;
	for (k = 0;k < count;k++)
	{
		pFrame = m_vecFramesToBeInserted.getNthItem(k);
		if (pFrame->getPreferedPageNo() == iPage)
		{
			return pFrame;
		}
	}
	return NULL;
}


void FL_DocLayout::setView(FV_View* pView)
{
	m_pView = pView;

	fp_Page* pPage = getFirstPage();

	while (pPage)
	{
		pPage->setView(pView);

		pPage = pPage->getNext();
	}

	if (m_pView && !m_pPrefs )
	{
		XAP_Prefs *pPrefs= XAP_App::getApp()->getPrefs();
		UT_ASSERT_HARMLESS(pPrefs);

		if (pPrefs)
		{
			// remember this so we can remove the listener later
			m_pPrefs = pPrefs;

			// initialize the vars here
			_prefsListener( pPrefs, NULL, this );

			// keep updating itself
			pPrefs->addListener ( _prefsListener, this );
			bool b;
			if (m_pPrefs->getPrefsValueBool(static_cast<const gchar *>("DebugFlash"),&b)  &&  b == true)
			{
				addBackgroundCheckReason(bgcrDebugFlash);
			}
			m_pPrefs->getPrefsValueBool(static_cast<const gchar *>("AutoGrammarCheck"),&b);
			if (b)
			{
				addBackgroundCheckReason(bgcrGrammar);
				m_bAutoGrammarCheck = true;
				m_iGrammarCount = 0;
				m_iPrevPos = 0;
			}
		}
	}
}

/*!
 * This method fills the referenced string with the character representation
 * of the decimal footnote value based on the FootnoteType passed as a 
 * parameter
 */
void FL_DocLayout::getStringFromFootnoteVal(UT_String & sVal, UT_sint32 iVal, FootnoteType iFootType) const
{
        fl_AutoNum autoCalc(0,0,NUMBERED_LIST,0,NULL,NULL,getDocument(),getView());
	switch (iFootType)
	{
	case FOOTNOTE_TYPE_NUMERIC:
		UT_String_sprintf (sVal,"%d", iVal);
		break;
	case FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS:
		UT_String_sprintf (sVal,"[%d]", iVal);
		break;
	case FOOTNOTE_TYPE_NUMERIC_PAREN:
		UT_String_sprintf (sVal,"(%d)", iVal);
		break;
	case FOOTNOTE_TYPE_NUMERIC_OPEN_PAREN:
		UT_String_sprintf (sVal,"%d)", iVal);
		break;
	case FOOTNOTE_TYPE_LOWER:
	{
		char * val = autoCalc.dec2ascii(iVal,96);
		UT_String_sprintf (sVal,"%s",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_LOWER_PAREN:
	{
		char * val = autoCalc.dec2ascii(iVal,96);
		UT_String_sprintf (sVal,"(%s)",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_LOWER_OPEN_PAREN:
	{
		char * val = autoCalc.dec2ascii(iVal,96);
		UT_String_sprintf (sVal,"%s)",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_UPPER:
	{
		char * val = autoCalc.dec2ascii(iVal,64);
		UT_String_sprintf (sVal,"%s",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_UPPER_PAREN:
	{
		char * val = autoCalc.dec2ascii(iVal,64);
		UT_String_sprintf (sVal,"(%s)",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_UPPER_OPEN_PAREN:
	{
		char * val = autoCalc.dec2ascii(iVal,64);
		UT_String_sprintf (sVal,"%s)",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_LOWER_ROMAN:
	{
		char * val = autoCalc.dec2roman(iVal,true);
		UT_String_sprintf (sVal,"%s",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_LOWER_ROMAN_PAREN:
	{
		char * val = autoCalc.dec2roman(iVal,true);
		UT_String_sprintf (sVal,"(%s)",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_UPPER_ROMAN:
	{
		char * val = autoCalc.dec2roman(iVal,false);
		UT_String_sprintf (sVal,"%s",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_UPPER_ROMAN_PAREN:
	{
		char * val = autoCalc.dec2roman(iVal,false);
		UT_String_sprintf (sVal,"(%s)",val);
		FREEP(val);
		break;
	}
	default:
		UT_String_sprintf (sVal,"%d", iVal);
	}
}


/*!
 * This simply returns the number of footnotes in the document.
 */
UT_uint32 FL_DocLayout::countFootnotes(void) const
{
	return m_vecFootnotes.getItemCount();
}
/*!
 * Add a footnote layout to the vector remembering them.
 */
void FL_DocLayout::addFootnote(fl_FootnoteLayout * pFL)
{
	m_vecFootnotes.addItem(pFL);
}

/*!
 * get a pointer to the Nth footnote layout in the vector remembering them.
 */
fl_FootnoteLayout * FL_DocLayout::getNthFootnote(UT_sint32 i) const
{
	UT_ASSERT(i>=0);
	if(i >= m_vecFootnotes.getItemCount())
	{
		return NULL;
	}
	else
	{
		return m_vecFootnotes.getNthItem(i);
	}
}

/*!
 * Remove a foonote layout from the Vector.
 */
void FL_DocLayout::removeFootnote(fl_FootnoteLayout * pFL)
{
	UT_sint32 i = m_vecFootnotes.findItem(pFL);
	if(i< 0)
	{
		return;
	}
	m_vecFootnotes.deleteNthItem(i);
}

/*!
 * This method returns the footnote layout associated with the input PID
 */
fl_FootnoteLayout * FL_DocLayout::findFootnoteLayout(UT_uint32 footpid) const
{
	UT_sint32 i = 0;
	fl_FootnoteLayout * pTarget = NULL;
 	fl_FootnoteLayout * pFL = NULL;
	for(i=0; i<m_vecFootnotes.getItemCount(); i++)
	{
		pFL = getNthFootnote(i);
		if(pFL->getFootnotePID() == footpid)
		{
			pTarget = pFL;
			break;
		}
	}
	return pTarget;
}
/*!
 * This returns the position of the footnote in the document. This is useful
 * for calculating the footnote's value and positioning it in a footnote 
 * section
 */
UT_sint32 FL_DocLayout::getFootnoteVal(UT_uint32 footpid) const
{
	UT_sint32 i =0;
	UT_sint32 pos = m_iFootnoteVal;
	fl_FootnoteLayout * pTarget = findFootnoteLayout(footpid);
 	fl_FootnoteLayout * pFL = NULL;
	if(pTarget== NULL)
	{
		return 0;
	}
	PT_DocPosition posTarget = pTarget->getDocPosition();
	fl_DocSectionLayout * pDocSecTarget = pTarget->getDocSectionLayout();
	fp_Container * pCon = pTarget->getFirstContainer();
	fp_Page * pPageTarget = NULL;
	if(pCon)
	{
		pPageTarget = pCon->getPage();
	}
	for(i=0; i<m_vecFootnotes.getItemCount(); i++)
	{
		pFL = getNthFootnote(i);
		if(!m_bRestartFootSection && !m_bRestartFootPage)
		{
			if(pFL->getDocPosition() < posTarget)
			{
				pos++;
			}
		}
		else if(m_bRestartFootSection)
		{
			if((pDocSecTarget == pFL->getDocSectionLayout()) && (pFL->getDocPosition() < posTarget))
			{
				pos++;
			}
		}
		else if(m_bRestartFootPage)
		{
			pCon = pFL->getFirstContainer();
			fp_Page * pPage = NULL;
			if(pCon)
			{
				pPage = pCon->getPage();
			}
			if((pPage == pPageTarget) && (pFL->getDocPosition() < posTarget))
			{
				pos++;
			} 
		}
	}
	return pos;
}

// Annotation methods


static UT_sint32 compareLayouts(const void * ppCL1, const void * ppCL2)
{
  void * v1 = const_cast<void *>(ppCL1);
  void * v2 = const_cast<void *>(ppCL2);
  fl_ContainerLayout ** pCL1 = reinterpret_cast<fl_ContainerLayout **>(v1);
  fl_ContainerLayout ** pCL2 = reinterpret_cast<fl_ContainerLayout **>(v2);
  return static_cast<UT_sint32>((*pCL1)->getPosition(true)) - static_cast<UT_sint32>((*pCL2)->getPosition(true));
}

/*!
 * This simply returns the number of annotations in the document.
 */
UT_uint32 FL_DocLayout::countAnnotations(void) const
{
	return m_vecAnnotations.getItemCount();
}

/*!
 * Collapse all the blocks containing Annotations. This is useful for
 * when we toggle displaying/hiding annotations.
 */
bool  FL_DocLayout::collapseAnnotations(void)
{
  fl_AnnotationLayout * pFL = NULL;
  fl_BlockLayout * pBL = NULL;
  UT_uint32 i = 0;
  for(i= 0; i<countAnnotations(); i++)
  {
      pFL = getNthAnnotation(i);
      if(pFL)
      {
	  pBL = pFL->getContainingBlock();
	  if(pBL)
	  {
	      pBL->collapse();
	  }
	  pBL = static_cast<fl_BlockLayout *>(pFL->getFirstLayout());
	  if(pBL)
	      pBL->collapse();

	  pFL->collapse();
      }
  }
  return true;
}

/*!
 * Add a annotation layout to the vector remembering them.
 */
void FL_DocLayout::addAnnotation(fl_AnnotationLayout * pFL)
{
	m_vecAnnotations.addItem(pFL);
	m_vecAnnotations.qsort(compareLayouts);
	UT_uint32 i = 0;
	for(i=0; i<countAnnotations();i++)
	{
	    fl_AnnotationLayout * pAL = getNthAnnotation(i);
	    fp_AnnotationRun * pARun = pAL->getAnnotationRun();
	    if(pARun)
	    {
		pARun->recalcValue();
	    }
	}
}

/*!
 * get a pointer to the Nth annotation layout in the vector remembering them.
 */
fl_AnnotationLayout * FL_DocLayout::getNthAnnotation(UT_sint32 i) const
{
	UT_ASSERT(i>=0);
	if(i >= m_vecAnnotations.getItemCount())
	{
		return NULL;
	}
	else
	{
		return m_vecAnnotations.getNthItem(i);
	}
}

/*!
 * Remove an annotation layout from the Vector.
 */
void FL_DocLayout::removeAnnotation(fl_AnnotationLayout * pFL)
{
	UT_sint32 i = m_vecAnnotations.findItem(pFL);
	if(i< 0)
	{
		return;
	}
	m_vecAnnotations.deleteNthItem(i);
	if(isLayoutDeleting())
	  return;
	m_vecAnnotations.qsort(compareLayouts);
	for(i=0; i<static_cast<UT_sint32>(countAnnotations());i++)
	{
	    fl_AnnotationLayout * pAL = getNthAnnotation(i);
	    fp_AnnotationRun * pARun = pAL->getAnnotationRun();
	    if(pARun)
	    {
		pARun->recalcValue();
	    }
	}
}

/*!
 * This method returns the annotation layout associated with the input PID
 */
fl_AnnotationLayout * FL_DocLayout::findAnnotationLayout(UT_uint32 annpid) const
{
	UT_sint32 i = 0;
	fl_AnnotationLayout * pTarget = NULL;
 	fl_AnnotationLayout * pFL = NULL;
	for(i=0; i<m_vecAnnotations.getItemCount(); i++)
	{
		pFL = getNthAnnotation(i);
		if(pFL->getAnnotationPID() == annpid)
		{
			pTarget = pFL;
			break;
		}
	}
	return pTarget;
}
/*!
 * This returns the position of the Annotation in the vector of annotations. This is useful
 * for calculating the annotionation positioning it in a annotation 
 * section
 */
UT_sint32 FL_DocLayout::getAnnotationVal(UT_uint32 annpid) const
{
	UT_sint32 i =0;
	UT_sint32 pos = 0;
 	fl_AnnotationLayout * pAL = NULL;
	for(i=0; i<m_vecAnnotations.getItemCount(); i++)
	{
		pAL = getNthAnnotation(i);
		if(pAL->getAnnotationPID() == annpid)
		{
		        pos = i;
			break;
		}
	}
	if(pos != i)
	  pos = -1;
	return pos;
}


/*!
 * The method returns the doc section layout before which the endnotes are 
 * inserted.
 */
fl_DocSectionLayout * FL_DocLayout::getDocSecForEndnote(fp_EndnoteContainer * pECon) const
{
	fl_DocSectionLayout *pDSL = NULL;
	if(getPlaceEndAtSecEnd())
	{
		fl_EndnoteLayout * pEL = static_cast<fl_EndnoteLayout *>(pECon->getSectionLayout());
		pDSL = pEL->getDocSectionLayout();
		return pDSL;
	}
	pDSL = getLastSection();
	return pDSL;
}

/*!
 * This method checks to too if the endnote container to be removed is the 
 * first or last of the section. If it is the first/last pointers are updated.
 */
void FL_DocLayout::removeEndnoteContainer(fp_EndnoteContainer * pECon)
{
	xxx_UT_DEBUGMSG(("Remove endnote container %x \n",pECon));
	fl_DocSectionLayout * pDSL = getDocSecForEndnote(pECon);
	if(pDSL->getFirstEndnoteContainer() == static_cast<fp_Container *>(pECon))
	{
		pDSL->setFirstEndnoteContainer(static_cast<fp_EndnoteContainer *>(pECon->getNext()));
	}
	if(pDSL->getLastEndnoteContainer() == static_cast<fp_Container *>(pECon))
	{
		pDSL->setLastEndnoteContainer(static_cast<fp_EndnoteContainer *>(pECon->getPrev()));
	}
//
// Remove from list
//
	if(pECon->getPrev())
	{
		pECon->getPrev()->setNext(pECon->getNext());
	}
	if(pECon->getNext())
	{
		pECon->getNext()->setPrev(pECon->getPrev());
	}
//	fl_EndnoteLayout * pEL = static_cast<fl_EndnoteLayout *>(pECon->getSectionLayout());
//	pDSL = static_cast<fl_DocSectionLayout *>(pEL->myContainingLayout());
//	if(!pDSL->isCollapsing())
	{
		fp_Column * pCol = static_cast<fp_Column *>(pECon->getContainer());

		if(pCol)
		{
			pCol->removeContainer(pECon);
		}
	}
}

/*!
 * This method inserts the endnote container into the list of containers held
 * held by the appropriate DocSection.
 */
void FL_DocLayout::insertEndnoteContainer(fp_EndnoteContainer * pECon)
{
	fl_DocSectionLayout * pDSL = getDocSecForEndnote(pECon);
	fp_Container * pCon = pDSL->getFirstEndnoteContainer();
	if(pCon == NULL)
	{
		pDSL->setFirstEndnoteContainer(pECon);
		pDSL->setLastEndnoteContainer(pECon);
		pECon->setNext(NULL);
		pECon->setPrev(NULL);
		fp_Column * pCol2 =  static_cast<fp_Column *>(pDSL->getLastContainer());
		if(pCol2)
		{
			pCol2->addContainer(pECon);
//
// No height defined yet. Can't layout
//
//			pCol->layout();
		}
		else
		{
			fp_Column * pCol = static_cast<fp_Column *>(pDSL->getNewContainer(NULL));
			pCol->addContainer(pECon);
//
// No height defined yet. Can't layout
//
//			pCol->layout();
		}
		return;
	}
	fp_EndnoteContainer * pETmp = static_cast<fp_EndnoteContainer *>(pCon);
	fl_EndnoteLayout * pEL = static_cast<fl_EndnoteLayout *>(pECon->getSectionLayout());
	fl_EndnoteLayout * pETmpL = static_cast<fl_EndnoteLayout *>(pETmp->getSectionLayout());
	bool bBefore = (pEL->getPosition() < pETmpL->getPosition());
	while(!bBefore && pETmp)
	{
		pETmp = static_cast<fp_EndnoteContainer *>(pETmp->getNext());
		if(pETmp)
		{
			pETmpL = static_cast<fl_EndnoteLayout *>(pETmp->getSectionLayout());
			UT_return_if_fail(pETmpL);
			bBefore = (pEL->getPosition() < pETmpL->getPosition());
		}
	}
	if(bBefore)
	{
		fp_EndnoteContainer * pOldPrev = static_cast<fp_EndnoteContainer *>(pETmp->getPrev());
		pETmp->setPrev(pECon);
		if(pDSL->getFirstEndnoteContainer() == pETmp)
		{
			pDSL->setFirstEndnoteContainer(pECon);
		}
		else
		{
			pOldPrev->setNext(pECon);
	
		}
		fp_Column * pCol = static_cast<fp_Column *>(pETmp->getContainer());
		pECon->setNext(pETmp);
		pECon->setPrev(pOldPrev);
		if(pOldPrev)
		{
			pCol->insertContainerAfter(pECon, pOldPrev);
		}
		else
		{
			pCol->insertContainer(pECon);
		}
		pCol->layout();
	}
	else
	{
		pETmp = static_cast<fp_EndnoteContainer *>(pDSL->getLastEndnoteContainer());
		pETmp->setNext(pECon);
		pECon->setPrev(pETmp);
		pECon->setNext(NULL);
		pDSL->setLastEndnoteContainer(pECon);
		fp_Column * pCol = static_cast<fp_Column *>(pETmp->getContainer());
		if(!pCol)
		{
			pCol = static_cast<fp_Column *>(pDSL->getLastContainer());
			if(pCol == NULL)
			{
				pCol = static_cast<fp_Column *>(pDSL->getNewContainer());
			}
		}
		pCol->addContainer(pECon);
		pCol->layout();
	}
}

/*!
 * This simply returns the number of footnotes in the document.
 */
UT_uint32 FL_DocLayout::countEndnotes(void) const
{
	return m_vecEndnotes.getItemCount();
}
/*!
 * Add a footnote layout to the vector remembering them.
 */
void FL_DocLayout::addEndnote(fl_EndnoteLayout * pFL)
{
	m_vecEndnotes.addItem(pFL);
}

/*!
 * get a pointer to the Nth footnote layout in the vector remembering them.
 */
fl_EndnoteLayout * FL_DocLayout::getNthEndnote(UT_sint32 i) const
{
	UT_ASSERT(i>=0);
	if(i >= m_vecEndnotes.getItemCount())
	{
		return NULL;
	}
	else
	{
		return m_vecEndnotes.getNthItem(i);
	}
}

/*!
 * Remove a foonote layout from the Vector.
 */
void FL_DocLayout::removeEndnote(fl_EndnoteLayout * pFL)
{
	UT_sint32 i = m_vecEndnotes.findItem(pFL);
	if(i< 0)
	{
		return;
	}
	m_vecEndnotes.deleteNthItem(i);
}

/*!
 * This method returns the footnote layout associated with the input PID
 */
fl_EndnoteLayout * FL_DocLayout::findEndnoteLayout(UT_uint32 footpid) const
{
	UT_sint32 i = 0;
	fl_EndnoteLayout * pTarget = NULL;
 	fl_EndnoteLayout * pFL = NULL;
	for(i=0; i<m_vecEndnotes.getItemCount(); i++)
	{
		pFL = getNthEndnote(i);
		if(pFL->getEndnotePID() == footpid)
		{
			pTarget = pFL;
			break;
		}
	}
	return pTarget;
}
/*!
 * This returns the position of the Endnote in the document. This is useful
 * for calculating the Endnote's value and positioning it in a footnote 
 * section
 */
UT_sint32 FL_DocLayout::getEndnoteVal(UT_uint32 footpid) const
{
	UT_sint32 i =0;
	UT_sint32 pos = m_iEndnoteVal;
	fl_EndnoteLayout * pTarget = findEndnoteLayout(footpid);
 	fl_EndnoteLayout * pFL = NULL;
	if(pTarget== NULL)
	{
		return 0;
	}
	PT_DocPosition posTarget = pTarget->getDocPosition();
	fl_DocSectionLayout * pDocSecTarget = pTarget->getDocSectionLayout();
	for(i=0; i<m_vecEndnotes.getItemCount(); i++)
	{
		pFL = getNthEndnote(i);
		if(!m_bRestartEndSection)
		{
			if(pFL->getDocPosition() < posTarget)
			{
				pos++;
			}
		}
		else if(m_bRestartEndSection)
		{
			if((pDocSecTarget == pFL->getDocSectionLayout()) && (pFL->getDocPosition() < posTarget))
			{
				pos++;
			}
		}
	}
	return pos;
}


//
//--------------------------------------------------------------------
// Table of content Functions.
//--------------------------------------------------------------------
//

UT_sint32 FL_DocLayout::getNumTOCs(void) const
{
	return m_vecTOC.getItemCount();
}

fl_TOCLayout * FL_DocLayout::getNthTOC(UT_sint32 i) const
{
	if( i >= getNumTOCs())
	{
		return NULL;
	}
	return m_vecTOC.getNthItem(i);
}

void FL_DocLayout::recalculateTOCFields(void)
{
	UT_sint32 num = getNumTOCs();
	UT_sint32 i =0;
	for (i=0; i<num; i++)
	{
		fl_TOCLayout * pTOCL = getNthTOC(i);
		pTOCL->recalculateFields(i);
	}
}

/*!
 * This method scans all the TOC in the document and adds or removes the
 * supplied block if it needs to be either added or removed from a TOC.
 * This method returns true if pBlock is in at least one TOC
 */
bool FL_DocLayout::addOrRemoveBlockFromTOC(fl_BlockLayout * pBlock)
{
	UT_sint32 count = getNumTOCs();
	if(count == 0)
	{
		return false;
	}
	UT_UTF8String sStyle;
	pBlock->getStyle(sStyle);
	UT_sint32 i = 0;
	UT_sint32 inTOC = count;
	UT_sint32 _addTOC = 0;

	for(i=0; i<count; i++)
	{
		fl_TOCLayout * pTOC = getNthTOC(i);
		if(pTOC->isBlockInTOC(pBlock))
		{
			if(!pTOC->isStyleInTOC(sStyle))
			{
				pTOC->removeBlock(pBlock);
				inTOC--;
			}
			else
			{
//
// Style changed so delete the old shadow of the block and make a new shadow.
//
				pTOC->removeBlock(pBlock);
				pTOC->addBlock(pBlock);
			}
		}
		else
		{
			if(pTOC->isStyleInTOC(sStyle))
			{
				pTOC->addBlock(pBlock);
				_addTOC++;
			}
		}
	}
	if((inTOC <= 0) && (_addTOC == 0))
	{
		return false;
	}
	return true;
}

/*!
 * Remove pBlock from all the TOC's it's in.
 * Return false if there are no TOC's in the Doc.
 * return true otherwise.
 */
bool FL_DocLayout::removeBlockFromTOC(fl_BlockLayout *pBlock)
{
	UT_sint32 count = getNumTOCs();
	if(count == 0)
	{
		return false;
	}
	UT_sint32 i = 0;
	for(i=0; i<count; i++)
	{
		fl_TOCLayout * pTOC = getNthTOC(i);
		if(pTOC->isBlockInTOC(pBlock))
		{
			pTOC->removeBlock(pBlock);
		}
	}
	return true;
}

/*!
 * returns true if the block is in at least one TOC.
 */
bool FL_DocLayout::isBlockInTOC(fl_BlockLayout * pBlock) const
{
	UT_sint32 count = getNumTOCs();
	if(count == 0)
	{
		return false;
	}
	UT_sint32 i = 0;
	for(i=0; i<count; i++)
	{
		fl_TOCLayout * pTOC = getNthTOC(i);
		if(pTOC->isBlockInTOC(pBlock))
		{
			return true;
		}
	}
	return false;
}

/*!
 * Fill the supplied vector with pointers to the blocks matching the supplied
 * Block.
 * Return false if no matching block were found.
 * true otherwise
 */
bool FL_DocLayout::getMatchingBlocksFromTOCs(fl_BlockLayout * pBlock, UT_GenericVector<fl_BlockLayout*>* pVecBlocks) const
{
	UT_sint32 count = getNumTOCs();
	if(count == 0)
	{
		return false;
	}
	UT_sint32 i = 0;
	for(i=0; i<count; i++)
	{
		fl_TOCLayout * pTOC = getNthTOC(i);
		if(pTOC->isBlockInTOC(pBlock))
		{
			fl_BlockLayout * pMatch = pTOC->getMatchingBlock(pBlock);
			pVecBlocks->addItem(pMatch);
		}
	}
	return (pVecBlocks->getItemCount() > 0);
}
	
bool FL_DocLayout::addTOC(fl_TOCLayout * pTOC)
{
	m_vecTOC.addItem(pTOC);
	return true;
}

bool FL_DocLayout::removeTOC(fl_TOCLayout * pTOC)
{
	UT_sint32 count = getNumTOCs();
	if(count == 0)
	{
		return false;
	}
	UT_sint32 i = m_vecTOC.findItem(pTOC);
	if(i < 0)
	{
		return false;
	}
	m_vecTOC.deleteNthItem(i);
	return true;
}

/*
   updates affected TOCs in response to bookmark operation
   returns true if operation resulted in change, false otherwise
*/
bool FL_DocLayout::updateTOCsOnBookmarkChange(const gchar * pBookmark)
{
	UT_return_val_if_fail( pBookmark && !isLayoutFilling(), false );
	bool bChange = false;
	
	for(UT_sint32 i = 0; i < getNumTOCs(); ++i)
	{
		fl_TOCLayout * pTOC = getNthTOC(i);
		UT_return_val_if_fail( pTOC, false );

		if(pTOC->getRangeBookmarkName().size() && !strcmp(pTOC->getRangeBookmarkName().utf8_str(), pBookmark))
		{
			// this TOC depends on the given bookmark, update ...
			pTOC->fillTOC();
			bChange = true;
		}
	}

	return bChange;
}


/**
 * Calculates the total height of the layout. Includes the 
 * vertical page margins when not printing.
 */
UT_sint32 FL_DocLayout::getHeight() const
{
	UT_sint32 iHeight = 0;
	FV_View * pView = getView(); // add page view dimensions
	UT_uint32 count = m_vecPages.getItemCount();
	UT_uint32 numRows = count / pView->getNumHorizPages();
	if (count > (pView->getNumHorizPages() * numRows))
	{
		numRows++;
	}

	for (unsigned int i = 0; i<numRows; i++)
	{
		UT_uint32 iRow = i / pView->getNumHorizPages();			
		iHeight += pView->getMaxHeight(iRow);
	}
	
	if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		if(pView)
		{
			iHeight += pView->getPageViewSep() * count; // Not count - 1, since we want a nice gray border at the very bottom of the document as well
			iHeight += pView->getPageViewTopMargin();
		}
		else
		{
			iHeight += fl_PAGEVIEW_PAGE_SEP * count; // Not count - 1, since we want a nice gray border at the very bottom of the document as well
			iHeight += fl_PAGEVIEW_MARGIN_Y;
		}
	}
	if(iHeight < 0)
	{
		iHeight = 0;
	}
	xxx_UT_DEBUGMSG(("FL_DocLayout::getHeight() - returned height %d \n",iHeight));
	return iHeight;
}

/**
 * Calculates the maximum width a page has in the layout. Includes the 
 * left page margin when not printing.
 */
UT_sint32 FL_DocLayout::getWidth() const
{
	UT_sint32 iWidth = 0;
	int count = m_vecPages.getItemCount();

	for (int i=0; i<count; i++)
	{
		fp_Page* p = m_vecPages.getNthItem(i);

		// we layout pages vertically, so this is max, not sum
		if (iWidth < p->getWidth())
			iWidth = p->getWidth();
	}

	if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		// add page view dimensions
		if(getView())
			iWidth += getView()->getPageViewLeftMargin() * 2;
		else
			iWidth += fl_PAGEVIEW_MARGIN_X * 2;
	}
	return iWidth;
}

const GR_Font* FL_DocLayout::findFont(const PP_AttrProp * pSpanAP,
									  const PP_AttrProp * pBlockAP,
				      const PP_AttrProp * pSectionAP,
				      GR_Graphics * pG,
				      bool isField) const
{
	const char* pszFamily	= PP_evalProperty("font-family",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszField	= PP_evalProperty("field-font",NULL,pBlockAP,NULL, m_pDoc, true);
	const char* pszStyle	= PP_evalProperty("font-style",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszVariant	= PP_evalProperty("font-variant",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszWeight	= PP_evalProperty("font-weight",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszStretch	= PP_evalProperty("font-stretch",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszSize		= PP_evalProperty("font-size",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszPosition = PP_evalProperty("text-position",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszLang     = PP_evalProperty("lang",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);

	xxx_UT_DEBUGMSG(("findFont::field-font is %s isField %d \n",pszField,isField));
	if ((pszField != NULL) && isField && (strcmp(pszField, "NULL") != 0))
		pszFamily = pszField;

	xxx_UT_DEBUGMSG(("findFont::pszFamily is %s \n",pszFamily));
	// for superscripts and subscripts, we'll automatically shrink the font size
	if ((0 == strcmp(pszPosition, "superscript")) ||
		(0 == strcmp(pszPosition, "subscript")))
	{
		double newSize = UT_convertToPoints(pszSize) * 2.0 / 3.0;
		pszSize = UT_formatDimensionedValue(newSize,"pt",".0");
	}
	if(pG==NULL)
	{
	    return m_pG->findFont(pszFamily, pszStyle,
						  pszVariant, pszWeight,
						  pszStretch, pszSize,
						  pszLang);
	}
	else
	{
	    
	    return pG->findFont(pszFamily, pszStyle,
			      pszVariant, pszWeight,
			      pszStretch, pszSize,
			      pszLang);
	}
}

/*!
 * Set the Document View page Size to properties provided. Rebuild 
 * the document afterwards.
 */
bool FL_DocLayout::setDocViewPageSize(const PP_AttrProp * pAP)
{
       PP_PropertyVector pProps = pAP->getProperties();
       FV_View * pView = getView();
       XAP_Frame * pFrame = NULL;
       UT_sint32 iZoom = 100;
       if(pView)
	    pFrame = static_cast<XAP_Frame *>(pView->getParentData());
       if(pFrame)
       {
	    iZoom = pFrame->getZoomPercentage();
	    XAP_Frame::tZoomType zt = pFrame->getZoomType();
	    if((zt == XAP_Frame::z_PAGEWIDTH) || (zt == XAP_Frame::z_WHOLEPAGE))
	    {
	         if(pView->isHdrFtrEdit())
		 {
		       pView->clearHdrFtrEdit();
		       pView->warpInsPtToXY(0,0,false);
		 } 
		 if(zt == XAP_Frame::z_PAGEWIDTH)
		 {
		       iZoom = pView->calculateZoomPercentForPageWidth();
		 }
		 if(zt == XAP_Frame::z_WHOLEPAGE)
		 {
		       iZoom = pView->calculateZoomPercentForWholePage();
		 }
	    }
       }
       bool b = m_docViewPageSize.Set(pProps);
       _setDocPageDimensions();
       if(pView && (pView->getViewMode() != VIEW_WEB))
       {
	    rebuildFromHere(m_pFirstSection);
       }
       if(pFrame)
	    pFrame->quickZoom(iZoom);
       return b;
}

const GR_Font* FL_DocLayout::findFont(const PP_AttrProp * pSpanAP,
									  const PP_AttrProp * pBlockAP,
				      const PP_AttrProp * pSectionAP,
				      bool isField) const
{
	const char* pszFamily	= PP_evalProperty("font-family",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszField	= PP_evalProperty("field-font",NULL,pBlockAP,NULL, m_pDoc, true);
	const char* pszStyle	= PP_evalProperty("font-style",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszVariant	= PP_evalProperty("font-variant",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszWeight	= PP_evalProperty("font-weight",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszStretch	= PP_evalProperty("font-stretch",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszSize		= PP_evalProperty("font-size",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszPosition = PP_evalProperty("text-position",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszLang     = PP_evalProperty("lang",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);

	if (pszField != NULL && isField && strcmp(pszField, "NULL"))
		pszFamily = pszField;

	// for superscripts and subscripts, we'll automatically shrink the font size
	if ((0 == strcmp(pszPosition, "superscript")) ||
		(0 == strcmp(pszPosition, "subscript")))
	{
		double newSize = UT_convertToPoints(pszSize) * 2.0 / 3.0;
		pszSize = UT_formatDimensionedValue(newSize,"pt",".0");
	}
	return m_pG->findFont(pszFamily, pszStyle,
			      pszVariant, pszWeight,
			      pszStretch, pszSize,
			      pszLang);
}

void FL_DocLayout::changeDocSections(const PX_ChangeRecord_StruxChange * pcrx, fl_DocSectionLayout * pDSL)
{
	fl_DocSectionLayout * pCur = pDSL;
	pDSL->doclistener_changeStrux(pcrx);
	while(pCur != NULL)
	{
		if(m_pDoc->isMarginChangeOnly())
		{
			pCur->doMarginChangeOnly();
		}
		else
		{
			pCur->collapse();
		}
		pCur = pCur->getNextDocSection();
	}
	if(m_pDoc->isMarginChangeOnly())
	{
		return;
	}
	pCur = pDSL;
	while(pCur != NULL)
	{
		pCur->updateDocSection();
		pCur = pCur->getNextDocSection();
	}
}


UT_sint32 FL_DocLayout::countPages() const
{
	return m_vecPages.getItemCount();
}

UT_sint32 FL_DocLayout::findPage(fp_Page * pPage) const
{
	UT_sint32 count = m_vecPages.getItemCount();
	if(count < 1)
	{
		return -1;
	}
	return m_vecPages.findItem(pPage);
}

fp_Page* FL_DocLayout::getNthPage(int n) const
{
	UT_ASSERT(m_vecPages.getItemCount() > 0);
	if(n >= m_vecPages.getItemCount())
	  return NULL;
	return m_vecPages.getNthItem(n);
}

fp_Page* FL_DocLayout::getFirstPage() const
{
	if (m_vecPages.getItemCount() == 0)
	{
		return NULL;
	}

	return m_vecPages.getNthItem(0);
}

fp_Page* FL_DocLayout::getLastPage() const
{
	if (m_vecPages.getItemCount() == 0)
	{
		return NULL;
	}

	return m_vecPages.getNthItem(m_vecPages.getItemCount()-1);
}

void FL_DocLayout::deletePage(fp_Page* pPage, bool bDontNotify /* default false */)
{
	UT_sint32 ndx = m_vecPages.findItem(pPage);
	UT_ASSERT(ndx >= 0);

	if (pPage->getPrev())
	{
		pPage->getPrev()->setNext(pPage->getNext());
	}

	if (pPage->getNext())
	{
		pPage->getNext()->setPrev(pPage->getPrev());
	}
	pPage->setPrev(NULL);
	pPage->setNext(NULL);
	m_vecPages.deleteNthItem(ndx);
	delete pPage;
	if(ndx < countPages())
	{
	    setFramePageNumbers(ndx);
	}
	// let the view know that we deleted a page,
	// so that it can update the scroll bar ranges
	// and whatever else it needs to do.
    //
    // Check for point > 0 to allow multi-threaded loads
    //
	if (m_pView && !bDontNotify && (m_pView->getPoint() > 0) && !m_pDoc->isPieceTableChanging())
	{
		m_pView->notifyListeners(AV_CHG_PAGECOUNT);
	}
}

fp_Page* FL_DocLayout::addNewPage(fl_DocSectionLayout* pOwner, bool bNoUpdate)
{
	fp_Page*		pLastPage;

	if (countPages() > 0)
	{
		pLastPage = getLastPage();
	}
	else
	{
		pLastPage = NULL;
	}

	fp_Page* pPage = new fp_Page(	this,
									m_pView,
									m_docViewPageSize,
									pOwner);
	if (pLastPage)
	{
		UT_ASSERT(pLastPage->getNext() == NULL);

		pLastPage->setNext(pPage);
	}
	pPage->setPrev(pLastPage);
	m_vecPages.addItem(pPage);
	pOwner->addOwnedPage(pPage);

	// let the view know that we created a new page,
	// so that it can update the scroll bar ranges
	// and whatever else it needs to do.

	if (m_pView && m_pView->shouldScreenUpdateOnGeneralUpdate() && m_pView->getPoint() > 0 && !bNoUpdate) // skip this if rebuilding or if we're loading a document
	{
		m_pView->notifyListeners(AV_CHG_PAGECOUNT);
	}

	return pPage;
}

/*!
  Find block at document position
  \param pos Document position
  \return Block at specified position.
  If bLookOnlyBefore = true, it returns NULL if no block can be found
  If bLookOnlyBefore = false, it returns the first block to the right of
  that position (it may still return NULL).
*/
fl_BlockLayout* FL_DocLayout::findBlockAtPosition(PT_DocPosition pos, bool bLookOnlyBefore) const
{
	fl_BlockLayout* pBL = NULL;
	fl_ContainerLayout* sfh = 0;

	PT_DocPosition posEOD;
	bool bRes;
	xxx_UT_DEBUGMSG(("Pos at entry %d \n",pos));
	bRes = m_pDoc->getBounds(true, posEOD);
	UT_ASSERT(bRes);
	if(m_pDoc->isEndFootnoteAtPos(pos))
	{
		xxx_UT_DEBUGMSG(("End footnote found at %d \n",pos));
		pos--;
	}
	if(m_pDoc->isFootnoteAtPos(pos))
	{
		xxx_UT_DEBUGMSG(("Start footnote found at %d \n",pos));
		pos+=2;
	}
	if(m_pDoc->isFootnoteAtPos(pos-1))
	{
		xxx_UT_DEBUGMSG(("Start footnote found at %d \n",pos));
		pos+=1;
	}

	bRes = m_pDoc->getStruxOfTypeFromPosition(m_lid, pos, PTX_Block, &sfh);
	// If block wasn't found at position, try finding it to the right,
	// limited only by the EOD.
	while(!bRes && !bLookOnlyBefore && (pos < posEOD) )
	{
		pos++;
		bRes = m_pDoc->getStruxOfTypeFromPosition(m_lid, pos, PTX_Block, &sfh);
	}

	if (bRes)
	{
		fl_Layout * pL = static_cast<fl_Layout *>(sfh);
		if(!pL)
			return NULL;

		switch (pL->getType())
		{
		case PTX_Block:
			pBL = static_cast<fl_BlockLayout *>(pL);
			while(pBL && !pBL->canContainPoint())
			{
				pBL = pBL->getPrevBlockInDocument();
			}
				  
			break;

		case PTX_Section:
		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			// We asked for a block, and we got a section.  Bad
			return NULL;
		}
	}
	else
	{
		return NULL;
	}

	if(pBL== NULL)
	{
	  //
	  // Give up!
	  //
	     return NULL;
	}
	
	fl_ContainerLayout * pMyC = pBL->myContainingLayout();
	while(pMyC && (pMyC->getContainerType() != FL_CONTAINER_DOCSECTION)
	      && (pMyC->getContainerType() != FL_CONTAINER_HDRFTR)
	      && (pMyC->getContainerType() != FL_CONTAINER_SHADOW))
	{
	  pMyC = pMyC->myContainingLayout();
	}
	if((pMyC->getContainerType() == FL_CONTAINER_HDRFTR)
	      || (pMyC->getContainerType() == FL_CONTAINER_SHADOW))
	{
		fl_HdrFtrShadow * pShadow = NULL;
		FV_View * pView = getView();
		if(pView && pView->isHdrFtrEdit())
		{
			pShadow = pView->getEditShadow();
//
// We might actually be in the other HdrFtr is the point got here from an undo!
// Check for this.
//
			if(!pShadow->getHdrFtrSectionLayout()->isPointInHere(pos))
			{
			        fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pBL->getSectionLayout());
				while(pCL && pCL->getContainerType() != FL_CONTAINER_HDRFTR && pCL->getContainerType() != FL_CONTAINER_DOCSECTION)
				{
				  if(pCL == pCL->myContainingLayout())
				  {
				    break;
				  }
				  pCL = pCL->myContainingLayout();
				}
				fl_HdrFtrSectionLayout * pHF = NULL;
				if(pCL && pCL->getContainerType() == FL_CONTAINER_HDRFTR)
				{ 
				     pHF = static_cast<fl_HdrFtrSectionLayout *>(pCL);
				}
				if(pHF && pHF->isPointInHere(pos))
				{
					pShadow = pHF->getFirstShadow();
					if(pShadow)
					{
						pView->clearHdrFtrEdit();
						pView->setHdrFtrEdit(pShadow);
						pBL = static_cast<fl_BlockLayout *>(pShadow->findBlockAtPosition(pos));
						return pBL;
					}
					else
					{
						return NULL;
					}
				}
				// Ok, we're really confused now, point is nowhere to be found.
				// It might be OK if pos-1 is in here, though...
				if (pShadow && !pShadow->getHdrFtrSectionLayout()->isPointInHere(pos-1))
				{
					//			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				}
			}

		}
		else if(pMyC->getContainerType() == FL_CONTAINER_SHADOW)
		{
		        pShadow = static_cast<fl_HdrFtrShadow *>(pMyC);
		}
		else
		{
			pShadow = static_cast<fl_HdrFtrSectionLayout *>(pMyC)->getFirstShadow();
		}
		fl_BlockLayout * ppBL = NULL;
		if(pShadow != NULL)
		{
			ppBL = static_cast<fl_BlockLayout *>(pShadow->findMatchingContainer(pBL));
		}
		else
		{
			UT_DEBUGMSG(("No Shadow! But there should be ! \n"));
			//	UT_ASSERT(0);
		}
//
// FIXME: Header/Footers
// some failsafe code should not trigger. Header/footer still not perfect.
//
		if(!ppBL)
		{
			if(!isLayoutFilling())
			{
				//			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		}
		else
		{
			pBL = ppBL;
		}
	}
	UT_ASSERT(pBL);
	return pBL;
}

fl_BlockLayout* FL_DocLayout::findBlockAtPositionReverse(PT_DocPosition pos) const
{
	fl_BlockLayout* pBL = NULL;
	fl_ContainerLayout* sfh = 0;

	PT_DocPosition posBOD;
	bool bRes;

	bRes = m_pDoc->getBounds(false, posBOD);
	UT_ASSERT(bRes);
	if(m_pDoc->isEndFootnoteAtPos(pos))
	{
		xxx_UT_DEBUGMSG(("End footnote found at %d \n",pos));
		pos--;
	}
	if(m_pDoc->isFootnoteAtPos(pos))
	{
		xxx_UT_DEBUGMSG(("Start footnote found at %d \n",pos));
		pos+=2;
	}
	bRes = m_pDoc->getStruxOfTypeFromPosition(m_lid, pos, PTX_Block, &sfh);
	// If block wasn't found at position, try finding it to the right,
	// limited only by the EOD.
	while(!bRes && (pos > posBOD))
	{
		pos--;
		bRes = m_pDoc->getStruxOfTypeFromPosition(m_lid, pos, PTX_Block, &sfh);
	}

	if (bRes)
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		if(!pL)
			return NULL;

		switch (pL->getType())
		{
		case PTX_Block:
			pBL = static_cast<fl_BlockLayout *>(pL);
			break;

		case PTX_Section:
		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			// We asked for a block, and we got a section.  Bad
			return NULL;
		}
	}
	else
	{
		UT_ASSERT_HARMLESS(0);
		return NULL;
	}

	if(pBL->getSectionLayout()->getType() == FL_SECTION_HDRFTR)
	{
		fl_HdrFtrShadow * pShadow = NULL;
		FV_View * pView = getView();
		if(pView && pView->isHdrFtrEdit())
		{
			pShadow = pView->getEditShadow();
//
// We might actually be in the other HdrFtr is the point got here from an undo!
// Check for this.
//
			if(!pShadow->getHdrFtrSectionLayout()->isPointInHere(pos))
			{
				fl_HdrFtrSectionLayout * pHF = (fl_HdrFtrSectionLayout *) pBL->getSectionLayout();
				if(pHF->isPointInHere(pos))
				{
					pShadow = pHF->getFirstShadow();
					pView->clearHdrFtrEdit();
					pView->setHdrFtrEdit(pShadow);
					pBL = (fl_BlockLayout *) pShadow->findBlockAtPosition(pos);
					return pBL;
				}
				// Ok, we're really confused now, point is nowhere to be found.
				// It might be OK if pos-1 is in here, though...
				if (!pShadow->getHdrFtrSectionLayout()->isPointInHere(pos-1))
				{
					UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				}
			}

		}
		else
		{
			pShadow = ((fl_HdrFtrSectionLayout *) pBL->getSectionLayout())->getFirstShadow();
		}
		fl_BlockLayout * ppBL = NULL;
		if(pShadow != NULL)
			ppBL = (fl_BlockLayout *) pShadow->findMatchingContainer(pBL);
		else
		{
			if(!isLayoutFilling())
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}
		}
		
		if(ppBL) {
			pBL = ppBL;
			}
	}
	UT_ASSERT(pBL);
	return pBL;
}

void FL_DocLayout::deleteEmptyColumnsAndPages(void)
{
	fl_DocSectionLayout* pSL = m_pFirstSection;
	while (pSL)
	{
		pSL->deleteEmptyColumns();
		pSL = pSL->getNextDocSection();
	}

	deleteEmptyPages();
}

void FL_DocLayout::deleteEmptyPages( bool bDontNotify /* default false */)
{
	int i;

	int iCountPages = m_vecPages.getItemCount();
	for (i=iCountPages - 1; i>=0; i--)
	{
		fp_Page* p = m_vecPages.getNthItem(i);
		UT_ASSERT_HARMLESS(p);
		if (p && p->isEmpty())
		{
			deletePage(p, bDontNotify);
		}
	}
}

void FL_DocLayout::updateOnViewModeChange()
{
	UT_DEBUGMSG(("updateOnViewModeChange \n"));
	// force margin properties lookup
	fl_SectionLayout* pSL = m_pFirstSection;
	m_docViewPageSize = getDocument()->m_docPageSize;
	UT_DebugOnly<UT_Dimension> orig_ut = DIM_IN;
	orig_ut = m_docViewPageSize.getDims();
	UT_DEBUGMSG(("updateOnViewModeChange - docViewPageSize width %f \n",m_docViewPageSize.Width(orig_ut)));
  	while (pSL)
  	{
		pSL->lookupMarginProperties();
		pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
  	}
	
	// rebuild
	formatAll();
}


void FL_DocLayout::formatAll()
{
	UT_return_if_fail(m_pDoc);
	m_pDoc->enableListUpdates();
	fl_SectionLayout* pSL = m_pFirstSection;
	clearAllCountWraps();
	while (pSL)
	{
		if(pSL->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pSL);
			pDSL->recalculateFields(0);
			if (!pDSL->isFirstPageValid())
			{
				pDSL->collapse();
			}
			pDSL->format();
			pDSL->checkAndRemovePages();			
		}
		else
		{
			pSL->recalculateFields(0);
			pSL->format();
		}

		pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
	}
}


void FL_DocLayout::rebuildFromHere( fl_DocSectionLayout * pFirstDSL)
{
  UT_DEBUGMSG(("Rebuilding DocLAyout %p doc %p \n",this,m_pDoc));
	UT_ASSERT(m_pDoc);
	if(isLayoutFilling())
	{
//		UT_ASSERT(0);
		return;
	}
	if(m_pDoc->isMarginChangeOnly())
	{
		return;
	}
//
	fl_DocSectionLayout * pStart = pFirstDSL;
//	fl_DocSectionLayout * pStart = pFirstDSL->getPrevDocSection();
//	if(pStart == NULL)
//	{
//		pStart = pFirstDSL;
//	}
	fl_DocSectionLayout * pDSL = pStart;
	// add page view dimensions
#if 1
	UT_DEBUGMSG(("SEVIOR: Rebuild from section %p \n",pFirstDSL));
	for(UT_sint32 k=0; k< m_vecPages.getItemCount(); k++)
	{
		fp_Page * pPage = m_vecPages.getNthItem(k);
		if(pPage->getOwningSection() == pFirstDSL)
		{
			UT_DEBUGMSG(("SEVIOR: Rebuilding from page %d \n",k));
			break;
		}
	}
#endif
	while (pDSL)
	{
		pDSL->collapse();
		pDSL = pDSL->getNextDocSection();
	}
	deleteEmptyColumnsAndPages();
	clearAllCountWraps();
//
// Clear out rebuild marks from this collapse
//
	pDSL = static_cast<fl_DocSectionLayout *>(m_pFirstSection);
	while(pDSL)
	{
		pDSL->clearRebuild();
		pDSL = pDSL->getNextDocSection();
	}

	deleteEmptyColumnsAndPages();
	pDSL= pStart;
	while (pDSL)
	{
		UT_DEBUGMSG(("SEVIOR: Building section %p \n",pDSL));
		pDSL->updateDocSection();
		pDSL->clearRebuild();
		pDSL = pDSL->getNextDocSection();
	}
//
// Clear out rebuild marks from the rebuild
//
	pDSL = static_cast<fl_DocSectionLayout *>(m_pFirstSection);
	while(pDSL)
	{
		pDSL->clearRebuild();
		pDSL = pDSL->getNextDocSection();
	}
}

void FL_DocLayout::updateLayout()
{
	/*
	  TODO the following routine checks every paragraph in the
	  document to see if it needs a reformat.  How is this going
	  to perform on a 50-page document?


	  Very good point. We need a isOnScreen() method!!!
	*/
	UT_ASSERT(m_pDoc);

	fl_SectionLayout* pSL = m_pFirstSection;
	while (pSL)
	{
		if(!isLayoutFilling())
		{
		        pSL->updateLayout(false);
		}
		if(pSL->getType() == FL_SECTION_DOC)
		{
			if(static_cast<fl_DocSectionLayout *>(pSL)->needsRebuild())
			{
				break;
			}
		}
		pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
	}
	if(pSL == NULL)
	{
		deleteEmptyColumnsAndPages();
		return;
	}
	if(m_pDoc->isPieceTableChanging())
	{
		static_cast<fl_DocSectionLayout *>(pSL)->clearRebuild();
		return;
	}
	rebuildFromHere(static_cast<fl_DocSectionLayout *>(pSL));
}


void FL_DocLayout::updateColor()
{
	UT_ASSERT(m_pDoc);
	FV_View * pView = getView();
	if(pView)
	{
		XAP_App * pApp = pView->getApp();
		XAP_Prefs * pPrefs = pApp->getPrefs();
		const gchar * pszTransparentColor = NULL;
		pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForTransparent),&pszTransparentColor);
//
// Save the new preference color
//
		strncpy(m_szCurrentTransparentColor,pszTransparentColor,9);
	}
//
// Now loop through the document and update the Background color
//
	fl_DocSectionLayout* pDSL = static_cast<fl_DocSectionLayout *>(m_pFirstSection);
	while (pDSL)
	{
		pDSL->setPaperColor();
		pDSL = pDSL->getNextDocSection();
	}
	fp_Page * pPage = NULL;
	UT_sint32 i =0;
	for(i=0; i<m_vecPages.getItemCount(); i++)
	{
		pPage = m_vecPages.getNthItem(i);
		pPage->getFillType().setTransColor(m_szCurrentTransparentColor);
		pPage->getFillType().markTransparentForPrint();
	}

//
// Redraw the view associated with this document.
//
	if(pView)
	{
		pView->updateScreen(false);
	}

}

#define BACKGROUND_CHECK_MSECS 100

#ifdef ENABLE_SPELL
/*!
 Toggle auto spell-checking state
 \param bSpell True if spell-checking should be enabled, false otherwise
 When disabling spelling, all squiggles are deleted.
 When enabling spelling, force a full check of the document.
*/
void
FL_DocLayout::_toggleAutoSpell(bool bSpell)
{
	bool bOldAutoSpell = getAutoSpellCheck();
	UT_DEBUGMSG(("_toggleAutoSpell %d \n",bSpell));
	// Add reason to background checker
	if (bSpell)
	{
		UT_DEBUGMSG(("Adding Auto SpellCheck  \n"));
		addBackgroundCheckReason(bgcrSpelling);
	}
	else
	{
		UT_DEBUGMSG(("Removing Auto SpellCheck  \n"));
		removeBackgroundCheckReason(bgcrSpelling);
	}

	xxx_UT_DEBUGMSG(("FL_DocLayout::_toggleAutoSpell (%s)\n",
					 bSpell ? "true" : "false" ));

	if (bSpell)
	{
		xxx_UT_DEBUGMSG(("Rechecking spelling in blocks \n"));
		queueAll(bgcrSpelling);
	}
	else
	{
		// Disabling, so remove the squiggles too
		fl_DocSectionLayout * pSL = getFirstSection();
		if(pSL)
		{
			fl_ContainerLayout* b = pSL->getFirstLayout();
			while (b)
			{
				if(b->getContainerType() == FL_CONTAINER_BLOCK)
				{
					static_cast<fl_BlockLayout *>(b)->removeBackgroundCheckReason(bgcrSpelling);
					static_cast<fl_BlockLayout *>(b)->getSpellSquiggles()->deleteAll();
					b = static_cast<fl_BlockLayout *>(b)->getNextBlockInDocument();
				}
				else
				{
					b = b->getNext();
				}
			}
		}
		if (bOldAutoSpell)
		{
			// If we're here, it was set to TRUE before but now it is
			// being set to FALSE. This means that it is the user
			// setting it. That's good.
			m_pView->draw(NULL);
			// A pending word would be bad. Not sure why it's not
			// ignored once autospell is off, but for now it should
			// definitely be annulled.
			setPendingWordForSpell(NULL, NULL);
		}
	}
}


/*!
 Toggle auto spell-checking state
 \param bGrammar True if grammar-checking should be enabled, false otherwise
 When disabling grammar checking, all squiggles are deleted.
 When enabling grammar, force a full check of the document.
*/
void
FL_DocLayout::_toggleAutoGrammar(bool bGrammar)
{
	bool bOldAutoGrammar = getAutoGrammarCheck();
	UT_DEBUGMSG(("_toggleAutoGrammar %d \n",bGrammar));
	// Add reason to background checker
	if (bGrammar)
	{
		UT_DEBUGMSG(("Adding Auto GrammarCheck  \n"));
		addBackgroundCheckReason(bgcrGrammar);
		m_bAutoGrammarCheck = true;
	}
	else
	{
		UT_DEBUGMSG(("Removing Auto Grammar  \n"));
		removeBackgroundCheckReason(bgcrGrammar);
		m_bAutoGrammarCheck = false;
	}

	xxx_UT_DEBUGMSG(("FL_DocLayout::_toggleAutoGrammar (%s)\n",
					 bGrammar ? "true" : "false" ));

	if (bGrammar)
	{
		xxx_UT_DEBUGMSG(("Rechecking Grammar in blocks \n"));
		queueAll(bgcrGrammar);
	}
	else
	{
		// Disabling, so remove the squiggles too
		fl_DocSectionLayout * pSL = getFirstSection();
		if(pSL)
		{
			fl_ContainerLayout* b = pSL->getFirstLayout();
			while (b)
			{
				if(b->getContainerType() == FL_CONTAINER_BLOCK)
				{
					static_cast<fl_BlockLayout *>(b)->removeBackgroundCheckReason(bgcrGrammar);
					static_cast<fl_BlockLayout *>(b)->getGrammarSquiggles()->deleteAll();
					b = static_cast<fl_BlockLayout *>(b)->getNextBlockInDocument();
				}
				else
				{
					b = b->getNext();
				}
			}
		}
		if (bOldAutoGrammar)
		{
			// If we're here, it was set to TRUE before but now it is
			// being set to FALSE. This means that it is the user
			// setting it. That's good.
			m_pView->draw(NULL);
		}
	}
}
#endif

void FL_DocLayout::_toggleAutoSmartQuotes(bool bSQ)
{
	setPendingSmartQuote(NULL, 0);  // avoid surprises
	if (bSQ)
	{
		addBackgroundCheckReason(bgcrSmartQuotes);
	}
	else
	{
		removeBackgroundCheckReason(bgcrSmartQuotes);
	}

	UT_DEBUGMSG(("FL_DocLayout::_toggleAutoSmartQuotes(%s)\n", bSQ ? "true" : "false" ));
}

/*!
Calculate the page height and width in layout unit
*/

void FL_DocLayout::_setDocPageDimensions(void)
{
	m_iPageWidth = UT_convertSizeToLayoutUnits(m_docViewPageSize.Width(DIM_IN),DIM_IN);
	m_iPageHeight = UT_convertSizeToLayoutUnits(m_docViewPageSize.Height(DIM_IN),DIM_IN);
}

void FL_DocLayout::setDisplayAnnotations(bool bDisplayAnnotations)
{
  m_bDisplayAnnotations = bDisplayAnnotations;
}

bool FL_DocLayout::displayAnnotations(void) const
{
  return m_bDisplayAnnotations;
}

bool FL_DocLayout::displayRDFAnchors(void) const
{
    return m_bDisplayRDFAnchors;
}

void FL_DocLayout::setDisplayRDFAnchors(bool v)
{
    m_bDisplayRDFAnchors = v;
}



#ifdef ENABLE_SPELL
/*!
 Do background spell-check
 \param pWorker Worker object
 \note This is a static callback method and does not have a 'this' pointer.
*/
void
FL_DocLayout::_backgroundCheck(UT_Worker * pWorker)
{
	UT_return_if_fail(pWorker);

	// Get the doclayout
	FL_DocLayout * pDocLayout = static_cast<FL_DocLayout *>(pWorker->getInstanceData());
	UT_return_if_fail(pDocLayout);

	// Win32 timers can fire prematurely on asserts (the dialog's
	// message pump releases the timers)
	if (!pDocLayout->m_pView)
	{
		return;
	}

//
// Don't redraw on selections.
//
//	if (!pDocLayout->m_pView->isSelectionEmpty())
//	{
//		return;
//	}
	xxx_UT_DEBUGMSG(("BAckground check called. \n"));
	// Don't spell check while printing!
	if(pDocLayout->m_pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
		return;
	}

	// Don't spell check if disabled, or already happening
	if(pDocLayout->m_bStopSpellChecking || pDocLayout->m_bImSpellCheckingNow || pDocLayout->isLayoutFilling())
	{
	  xxx_UT_DEBUGMSG(("Already spellchecking!!!!!!!! \n"));
		return;
	}

	// Code added to hold spell checks during block insertions
	if(pDocLayout->m_pDoc->isPieceTableChanging())
	{
		return;
	}

	// Don't spell check while a redrawupdate is happening either...
	PD_Document * pDoc = pDocLayout->getDocument();
	if(pDoc->isRedrawHappenning())
	{
		return;
	}

	// Flag that spell checking is in action.
	// Note: this is not a good way to do mutual exclusion!
	pDocLayout->m_bImSpellCheckingNow = true;

	fl_BlockLayout *pB = pDocLayout->spellQueueHead();
	xxx_UT_DEBUGMSG(("Spellchecking block %x \n",pB));
	if (pB != NULL)
	{
		// This looping seems like a lot of wasted effort when we
		// don't define meaning for most of the bits, but it's
		// small effort compared to all that squiggle stuff that
		// goes on for the spelling stuff.
		if(pB->getContainerType() == FL_CONTAINER_BLOCK)
		{
			for (UT_uint32 bitdex = 0;
				 bitdex < 8*sizeof(pB->m_uBackgroundCheckReasons);
				 bitdex++)
			{
				UT_uint32 mask;
				mask = (1 << bitdex);
				if (pB->hasBackgroundCheckReason(mask))
				{
					if(!pDocLayout->m_bFinishedInitialCheck 
					   && pDocLayout->m_iPrevPos > pB->getPosition())
					{
						pDocLayout->m_bFinishedInitialCheck = true;
					}
					pDocLayout->m_iPrevPos = pB->getPosition();
					
					// Note that we remove this reason from queue
					// before checking it (otherwise asserts could
					// trigger redundant recursive calls)
					switch (mask)
					{
					case bgcrNone:
						pB->removeBackgroundCheckReason(mask);
						break;
					case bgcrDebugFlash:
						pB->debugFlashing();
						pB->removeBackgroundCheckReason(mask);
						break;
					case bgcrSpelling:
					{
						xxx_UT_DEBUGMSG(("Spelling checking block %x directly \n",pB));
						bool b = pB->checkSpelling();
						if(b)
						{
							pB->removeBackgroundCheckReason(mask);
						}
						break;
					}
					case bgcrGrammar:
					{
						if(!pDocLayout->m_bFinishedInitialCheck)
						{
							if(pDocLayout->m_iGrammarCount < 4)
							{
								pDocLayout->m_iGrammarCount++;
								pDocLayout->m_bImSpellCheckingNow = false;
								return;
							}
							pDocLayout->m_iGrammarCount = 0;
						}
						
						xxx_UT_DEBUGMSG(("Grammar checking block %x directly \n",pB));
						XAP_App * pApp = pDocLayout->getView()->getApp();
						//
						// If a grammar checker plugin is loaded it will check the block now.
						//
						pApp->notifyListeners(pDocLayout->getView(),
											  AV_CHG_BLOCKCHECK,reinterpret_cast<void *>(pB));
						pB->removeBackgroundCheckReason(mask);
						pB->drawGrammarSquiggles();
						break;
					}
					case bgcrSmartQuotes:
					default:
						pB->removeBackgroundCheckReason(mask);
						break;
					}
				}
			}
		}
		// Delete block from queue if there are no more reasons
		// for checking it.
		if((pB->getContainerType() != FL_CONTAINER_BLOCK) 
		   || (!pB->m_uBackgroundCheckReasons))
		{
			pB->dequeueFromSpellCheck();
		}
	}
	else
	{
		// No blocks to spellcheck so stop the idle/timer. Otherwise
		// we consume 100% CPU.
		pDocLayout->m_pBackgroundCheckTimer->stop();
	}

	pDocLayout->m_bImSpellCheckingNow = false;
}

/*!
 Enqueue block for background spell-checking
 \param iReason Reason for checking the block FIXME - enum?
 \param pBlock Block to enqueue
 \param bHead When true, insert block at head of queue

 This routine queues up blocks for timer-driven spell checking, etc.
 By default, this is a FIFO queue, but it can be explicitly
 reprioritized by setting bHead to true.
*/
void
FL_DocLayout::queueBlockForBackgroundCheck(UT_uint32 iReason,
										   fl_BlockLayout *pBlock,
										   bool bHead)
{
	// If there's no timer running, start one
	if (!m_pBackgroundCheckTimer)
	{
	    int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	    if(getView() && getView()->isGrammarLoaded() && m_bAutoGrammarCheck)
	    {
	         inMode = UT_WorkerFactory::TIMER;
	    }
	    UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	    
	    m_pBackgroundCheckTimer = UT_WorkerFactory::static_constructor (_backgroundCheck, this, inMode, outMode);

	    UT_ASSERT(m_pBackgroundCheckTimer);
	    UT_ASSERT(outMode != UT_WorkerFactory::NONE);

		// If the worker is working on a timer instead of in the idle
		// time, set the frequency of the checks.
	    if ( UT_WorkerFactory::TIMER == outMode )
		{
			// this is really a timer, so it's safe to static_cast it
			static_cast<UT_Timer*>(m_pBackgroundCheckTimer)->set(BACKGROUND_CHECK_MSECS);
		}
#if 1
	    m_bStopSpellChecking = false;
	    m_pBackgroundCheckTimer->start();
#endif

	}
#if 1 // We need this to restart the idle handler.
	else
	{
		//		m_pBackgroundCheckTimer->stop();
		m_bStopSpellChecking = false;
		m_pBackgroundCheckTimer->start();
	}
#endif

	// Set debug flash reason on block if it is set
	if (hasBackgroundCheckReason(bgcrDebugFlash))
	{
		pBlock->addBackgroundCheckReason(bgcrDebugFlash);
	}
	pBlock->addBackgroundCheckReason(iReason);

	if (!pBlock->isQueued())
	{
		// Add block if it's not already in the queue. Add it either
		// at the head, or at the tail.
		if (bHead)
			pBlock->enqueueToSpellCheckAfter(NULL);
		else
			pBlock->enqueueToSpellCheckAfter(m_toSpellCheckTail);
	}
	else if (bHead)
	{
		// Block is already in the queue, bubble it to the start
		pBlock->dequeueFromSpellCheck();
		pBlock->enqueueToSpellCheckAfter(NULL);
	}
}

void FL_DocLayout::dequeueAll(void)
{
	fl_BlockLayout *pB = spellQueueHead();
	while (pB != NULL)
	{
		fl_BlockLayout *pNext = pB->nextToSpell();
		pB->clearQueueing();
		pB = pNext;
	}
	setSpellQueueHead(NULL);
	setSpellQueueTail(NULL);
	UT_DEBUGMSG(("Dequeue all \n"));

	m_PendingBlockForGrammar = NULL;
	m_bStopSpellChecking = true;
	if(m_pBackgroundCheckTimer)
	{
		m_pBackgroundCheckTimer->stop();
		// Wait for checking to complete before returning.
		while(m_bImSpellCheckingNow == true)
		{
			// TODO shouldn't we have a little sleep here?
		}
	}
}

void FL_DocLayout::queueAll(UT_uint32 iReason)
{
	fl_DocSectionLayout * pSL = getFirstSection();
	if(pSL)
	{
		// We will place the block that contains the ins point and its immediate neigbours
		// at the top of the queue, this will make the check look faster to the user
		FV_View * pView = getView();
		UT_GenericVector<fl_BlockLayout*> vBL;
		const UT_sint32 iLimit = 5;
		
		fl_BlockLayout * pCurBL = pView->getBlockAtPosition(pView->getPoint());

		if(pCurBL)
		{
			fl_BlockLayout * pBL = pCurBL;

			UT_sint32 i = 0;
			for(i = 0; i < iLimit/2 + iLimit%2 && pBL; ++i, pBL = pBL->getPrevBlockInDocument())
			{
				vBL.addItem(pBL);
			}

			pBL = pCurBL->getNextBlockInDocument();
			for(i = iLimit/2 + iLimit%2; i < iLimit && pBL; ++i, pBL = pBL->getNextBlockInDocument())
			{
				vBL.addItem(pBL);
			}
		}
		
		fl_ContainerLayout* b = pSL->getFirstLayout();
		while (b)
		{
			// TODO: just check and remove matching squiggles
			// for now, destructively recheck the whole thing
			if(b->getContainerType() == FL_CONTAINER_BLOCK)
			{
				bool bHead = (vBL.findItem(static_cast<fl_BlockLayout *>(b)) >= 0);
				queueBlockForBackgroundCheck(iReason, static_cast<fl_BlockLayout *>(b), bHead);
				b = static_cast<fl_BlockLayout *>(b)->getNextBlockInDocument();
			}
			else
			{
				b = b->getNext();
			}
		}
	}
}



/*!
 * Set the next block to be grammar checked. It won't actually get checked
 * until the insertPoint leaves this block.
 */
void FL_DocLayout::setPendingBlockForGrammar(fl_BlockLayout * pBL)
{
  xxx_UT_DEBUGMSG(("Pending called with block %x pending %x \n",pBL,m_PendingBlockForGrammar));
  if(!m_bAutoGrammarCheck)
    return;
  if((m_PendingBlockForGrammar != NULL) && (m_PendingBlockForGrammar != pBL))
    {
      xxx_UT_DEBUGMSG(("Block %x queued \n",m_PendingBlockForGrammar));
      queueBlockForBackgroundCheck(bgcrGrammar,m_PendingBlockForGrammar,true);
    }
  m_PendingBlockForGrammar = pBL;
}


/*!
 * This is called from fv_View::_fixPointCoords to actually queue a grammar 
 * check a pending block.
 */
void FL_DocLayout::triggerPendingBlock(fl_BlockLayout * pBL)
{
  xxx_UT_DEBUGMSG(("Trigger called with block %x pending %x \n",pBL,m_PendingBlockForGrammar));
  if(!m_bAutoGrammarCheck)
    return;
  if((m_PendingBlockForGrammar != NULL) && (m_PendingBlockForGrammar != pBL))
    {
      queueBlockForBackgroundCheck(bgcrGrammar,m_PendingBlockForGrammar,true);
      m_PendingBlockForGrammar = NULL;
     }
}

/*!
 Remove block from background checking queue
 \param pBlock Block to remove

 When the last block is removed from the queue, the background timer
 is stopped. The function does not return before the background
 spell-checking timer has stopped.
*/
bool
FL_DocLayout::dequeueBlockForBackgroundCheck(fl_BlockLayout *pBlock)
{
	bool bRes = false;

	// Remove block from queue if it's found there
	bRes = pBlock->isQueued();
	if (bRes) {
		pBlock->dequeueFromSpellCheck();
	}
	if(pBlock == m_PendingBlockForGrammar)
	  {
	    xxx_UT_DEBUGMSG(("Dequeue block %x in dequeue \n",pBlock));
	    m_PendingBlockForGrammar = NULL;
	  }
	// When queue is empty, kill timer
	if (spellQueueHead() == NULL)
	{
		m_bStopSpellChecking = true;
		if(m_pBackgroundCheckTimer)
		{
			m_pBackgroundCheckTimer->stop();
			// Wait for checking to complete before returning.
			while(m_bImSpellCheckingNow == true)
			{
				// TODO shouldn't we have a little sleep here?
			}
		}
	}

	return bRes;
}

/*!
  Mark a region of a block to be spell checked
  \param pBlock Block
  \param pWord  Region

  If called with NULL arguments, any prior marked region will be
  freed. Callers must reuse pWord (by calling getPendingWordForSpell)
  when set.
*/
void
FL_DocLayout::setPendingWordForSpell(const fl_BlockLayout *pBlock,
									 fl_PartOfBlock* pWord)
{
	// Return if matching the existing marked region
	if ((pBlock == m_pPendingBlockForSpell) &&
		(pWord == m_pPendingWordForSpell))
		return;

	// Assert an existing pWord allocation is reused
	UT_ASSERT(!m_pPendingBlockForSpell || !pBlock
			  || m_pPendingWordForSpell == pWord);

	// Check for valid arguments
	if (pBlock && m_pPendingBlockForSpell && m_pPendingWordForSpell)
	{
		UT_ASSERT(pWord);
	}

	if (m_pPendingWordForSpell && (m_pPendingWordForSpell != pWord))
	{
		// When clobbering prior POB, make sure we don't leak it
		DELETEP(m_pPendingWordForSpell);
	}

	m_pPendingBlockForSpell = pBlock;
	m_pPendingWordForSpell = pWord;
}

/*!
 Spell-check pending word
 \result True if word checked, false otherwise
 If a word is pending, spell-check it.

 \note This function used to exit if PT was changing - but that
       prevents proper squiggle behavior during undo, so the check has
       been removed. This means that the pending word POB must be
       updated to reflect the PT changes before the IP is moved.
*/
bool
FL_DocLayout::checkPendingWordForSpell(void)
{
	// do not attempt to check a word if check is already in progress (see 7197)
	if(m_bSpellCheckInProgress)
		return false;

	bool bUpdate = false;

	xxx_UT_DEBUGMSG(("FL_DocLayout::checkPendingWordForSpell\n"));

	if (!m_pPendingBlockForSpell)
		return bUpdate;

	m_bSpellCheckInProgress = true;
	
	// Check pending word
	UT_ASSERT(m_pPendingWordForSpell);
	bUpdate = m_pPendingBlockForSpell->checkWord(m_pPendingWordForSpell);

	m_pPendingWordForSpell = NULL;	// NB: already freed by checkWord

	// Not pending any more
	setPendingWordForSpell(NULL, NULL);

	m_bSpellCheckInProgress = false;
	
	return bUpdate;
}

/*!
 Is a word pending for spelling
 \return True if a word is pending, false otherwise
*/
bool
FL_DocLayout::isPendingWordForSpell(void) const
{
	return (m_pPendingBlockForSpell ? true : false);
}

/*!
 Determine if position touches the pending word for spelling
 \param pBLock Block of position
 \param iOffset Offset in block
 \param chg  FIXME
 \return True if position touches pending word, false otherwise

 FIXME: why this function/chg? Caller uses result for what?
*/
bool
FL_DocLayout::touchesPendingWordForSpell(fl_BlockLayout *pBlock,
										 UT_sint32 iOffset,
										 UT_sint32 chg) const
{
	UT_uint32 len = (chg < 0) ? -chg : 0;

	UT_ASSERT(pBlock);

	if (!m_pPendingBlockForSpell)
		return false;

	// Are we in the same block?
	if (m_pPendingBlockForSpell != pBlock)
		return false;

	UT_return_val_if_fail(m_pPendingWordForSpell,false);

	return m_pPendingWordForSpell->doesTouch(iOffset, len);
}
#endif // ENABLE_SPELL

/*!
 * This method appends a DocSectionLayout onto the linked list of SectionLayout's
 * and updates the m_pFirstSection and m_pLastSection member variables
 * accordingly.
 * The structure of this linked list is as follows.
 *    pDSL->pDSL->....pDSL->pEndnoteSL->pHdrFtrSL->pHdrFtrSL->NULL
 *     ^                ^
 *m_pFirstSection   m_pLastSection
 *ie we have all the DocSections in a linked list followed by all the Header/
 * Footer sections. This reflects the locations in the piece table where
 * the header/footer sections are located at the end of the document.
\param  fl_DocSectionLayout * pSL the DocSectionLayout to be appended.
\param  fl_DocSectionLayout* pAfter the DocSectionLayout after which our new
        DocSectionLayout is inserted.
*/
void FL_DocLayout::addSection(fl_DocSectionLayout* pSL)
{
	if (m_pLastSection)
	{
		UT_ASSERT(m_pFirstSection);
		insertSectionAfter(m_pLastSection,pSL);
	}
	else
	{
		pSL->setPrev(NULL);
		pSL->setNext(NULL);
		m_pFirstSection = pSL;
		m_pLastSection = m_pFirstSection;
	}
}

/*!
 * This method inserts a DocSectionLayout into the linked list of SectionLayout's
 * and updates the m_pFirstSection and m_pLastSection member variables
 * accordingly
\param  fl_DocSectionLayout * pNewSL the DocSectionLayout to be inserted.
\param  fl_DocSectionLayout* pAfter the DocSectionLayout after which our new
        DocSectionLayout is inserted.
*/
void FL_DocLayout::insertSectionAfter(fl_DocSectionLayout* pAfter, fl_DocSectionLayout* pNewSL)
{
        UT_return_if_fail(pAfter);
	pNewSL->setNext(pAfter->getNext());
	pNewSL->setPrev(pAfter);
	if (pAfter->getNext())
	{
		pAfter->getNext()->setPrev(pNewSL);
	}
	pAfter->setNext(pNewSL);

	if (m_pLastSection == pAfter)
	{
		m_pLastSection = pNewSL;
	}
}

/*!
 * This method removes a DocSectionLayout from the linked list of SectionLayout's
 * and updates the m_pFirstSection and m_pLastSection member variables
 * accordingly
\param  fl_DocSectionLayout * pSL the DocSectionLayout to be removed.
*/

void FL_DocLayout::removeSection(fl_DocSectionLayout * pSL)
{
	UT_return_if_fail(pSL);
	UT_ASSERT(m_pFirstSection);

	if (pSL->getPrev())
	{
		pSL->getPrev()->setNext(pSL->getNext());
	}

	if (pSL->getNext())
	{
		pSL->getNext()->setPrev(pSL->getPrev());
	}

	if (pSL == m_pFirstSection)
	{
		m_pFirstSection = m_pFirstSection->getNextDocSection();
		if (!m_pFirstSection)
		{
			m_pLastSection = NULL;
		}
	}

	if (pSL == m_pLastSection)
	{
		m_pLastSection = m_pLastSection->getPrevDocSection();
		if (!m_pLastSection)
		{
			m_pFirstSection = NULL;
		}
	}

	pSL->setNext(NULL);
	pSL->setPrev(NULL);
}

/*!
 * Include the header/footer section layouts AFTER the last DocSection in the
 * the getNext, getPrev list. This will ensure that the headers/footers will be
 * formatted and updated correctly.
 \param fl_SectionLayout * pHdrFtrSL the header/footer layout to be inserted
        into the sectionlayout linked list.
 * The structure of this linked list is as follows.
 *    pDSL->pDSL->pDSL....pDSL->pEndnoteSL->pHdrFtrSL->pHdrFtrSL->NULL
 *     ^                   ^
 *m_pFirstSection   m_pLastSection
 *
 *ie we have all the DocSections in a linked list followed by all the Header/
 * Footer sections. This reflects the locations in the piece table where
 * the header/footer sections are located at the end of the document.
*/
void FL_DocLayout::addHdrFtrSection(fl_SectionLayout* pHdrFtrSL)
{
	UT_ASSERT(m_pLastSection);

	fl_SectionLayout * pLSL = static_cast<fl_SectionLayout *>(m_pLastSection);
	fl_SectionLayout * pnext = static_cast<fl_SectionLayout *>(pLSL->getNext());

	while (pnext && pnext->getType() == FL_SECTION_ENDNOTE)
		pnext = static_cast<fl_SectionLayout *>(pnext->getNext());

	if(pnext)
	{
		pnext->setPrev(pHdrFtrSL);
		pLSL->setNext(pHdrFtrSL);
		pHdrFtrSL->setPrev(pLSL);
		pHdrFtrSL->setNext(pnext);
	}
	else
	{
		pLSL->setNext(pHdrFtrSL);
		pHdrFtrSL->setPrev(pLSL);
		pHdrFtrSL->setNext(pnext);
	}
}

/*!
 *  This method removes a header/footer layout from the section linked list.
 \param fl_SectionLayout * pHdrFtrSL is the header/footer section to be removed
*/
void FL_DocLayout::removeHdrFtrSection(fl_SectionLayout * pHdrFtrSL)
{
	UT_return_if_fail(pHdrFtrSL);

	if(pHdrFtrSL->getPrev())
	{
		pHdrFtrSL->getPrev()->setNext(pHdrFtrSL->getNext());
	}
	if (pHdrFtrSL->getNext())
	{
		pHdrFtrSL->getNext()->setPrev(pHdrFtrSL->getPrev());
	}
	pHdrFtrSL->setNext(NULL);
	pHdrFtrSL->setPrev(NULL);
}

fl_DocSectionLayout* FL_DocLayout::findSectionForHdrFtr(const char* pszHdrFtrID) const
{
	if(!pszHdrFtrID)
		return NULL;

	const char* pszAtt = NULL;

	fl_DocSectionLayout* pDocSL = m_pFirstSection;
	while (pDocSL)
	{
		pszAtt = pDocSL->getAttribute("header");
		if ( pszAtt	&& (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}

		pszAtt = pDocSL->getAttribute("footer");
		if (pszAtt && (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}
		pszAtt = pDocSL->getAttribute("header-even");
		if ( pszAtt	&& (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}

		pszAtt = pDocSL->getAttribute("footer-even");
		if (pszAtt && (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}
		pszAtt = pDocSL->getAttribute("header-last");
		if ( pszAtt	&& (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}

		pszAtt = pDocSL->getAttribute("footer-last");
		if (pszAtt && (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}
		pszAtt = pDocSL->getAttribute("header-first");
		if ( pszAtt	&& (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}

		pszAtt = pDocSL->getAttribute("footer-first");
		if (pszAtt && (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}

		pDocSL = pDocSL->getNextDocSection();
	}

	return NULL;
}

/*static*/ void FL_DocLayout::_prefsListener (
	XAP_Prefs			*pPrefs,
	UT_StringPtrMap	* /*phChanges*/,  // not used
	void				*data
	)
{
	bool b;
	FL_DocLayout *pDocLayout = static_cast<FL_DocLayout *>(data);

	xxx_UT_DEBUGMSG(("spell_prefsListener %p\n", pDocLayout));
	UT_ASSERT( pPrefs && data );

	// caps/number/internet

    // Note that these options are now "ignore..." in the prefs pane,
    // so the opton settings are reverted for use in the doclayout
    // (b = !b)
	bool changed = false;
#ifdef ENABLE_SPELL
	pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_SpellCheckCaps), &b );
    b = !b;
	changed = changed || (b != pDocLayout->getSpellCheckCaps());
	pDocLayout->m_bSpellCheckCaps = b;

	pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_SpellCheckNumbers), &b );
    b = !b;
	changed = changed || (b != pDocLayout->getSpellCheckNumbers());
	pDocLayout->m_bSpellCheckNumbers = b;

	pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_SpellCheckInternet), &b );
    b = !b;
	changed = changed || (b != pDocLayout->getSpellCheckInternet());
	pDocLayout->m_bSpellCheckInternet = b;

	// auto spell
	pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_AutoSpellCheck), &b );
	changed = changed || (b != pDocLayout->m_bAutoSpellCheck);
	if(b != pDocLayout->m_bAutoSpellCheck || (pDocLayout->m_iGraphicTick < 2))
	{
		pDocLayout->m_bAutoSpellCheck = b;
		pDocLayout->_toggleAutoSpell( b );
	}

	// grammar check
	pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_AutoGrammarCheck), &b );
	changed = changed || (b != pDocLayout->m_bAutoSpellCheck);
	if(b != pDocLayout->m_bAutoGrammarCheck || (pDocLayout->m_iGraphicTick < 2))
	{
		pDocLayout->m_bAutoGrammarCheck = b;
		pDocLayout->_toggleAutoGrammar( b );
	}
#endif
// autosave

	UT_String stTmp;
	FV_View * pView = pDocLayout->getView();
	if(pView)
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
		if(pFrame)
		{
			pPrefs->getPrefsValueBool(static_cast<const gchar *>(XAP_PREF_KEY_AutoSaveFile), &b );
			changed = (b != pFrame->isBackupRunning());
			if(changed)
			{
				pFrame->setAutoSaveFile(b);
			}

// autosave period

			pPrefs->getPrefsValue(XAP_PREF_KEY_AutoSaveFilePeriod, stTmp);
			UT_sint32 iPeriod = atoi(stTmp.c_str());
			changed = (iPeriod != pFrame->getAutoSavePeriod());
			if(changed)
			{
				pFrame->setAutoSaveFilePeriod(iPeriod);
				if(pFrame->isBackupRunning())
				{
					pFrame->setAutoSaveFile(false);
					pFrame->setAutoSaveFile(true);
				}
			}
		}
	}


	if ( changed )
	{
		// TODO: recheck document
		;
	}

	pPrefs->getPrefsValueBool( static_cast<const gchar *>(XAP_PREF_KEY_SmartQuotesEnable), &b );
	
	
	pDocLayout->_toggleAutoSmartQuotes( b );
	
	const gchar * pszTransparentColor = NULL;
	pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForTransparent),&pszTransparentColor);
	if(strcmp(pszTransparentColor,pDocLayout->m_szCurrentTransparentColor) != 0)
	{
		if(pDocLayout->getView() && (pDocLayout->getView()->getPoint() > 0))
		{
			pDocLayout->updateColor();
		}
	}

	// Display Annotations

	pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_DisplayAnnotations), &b );
	changed = changed || (b != pDocLayout->m_bDisplayAnnotations);
	if(b != pDocLayout->m_bDisplayAnnotations || (pDocLayout->m_iGraphicTick < 2))
	{
		pDocLayout->m_bDisplayAnnotations = b;
		pDocLayout->collapseAnnotations();
		pDocLayout->formatAll();
		if(pDocLayout->getView())
		{
		    pDocLayout->getView()->updateScreen(false);
		}
	}

	// Display RDF Anchors

	pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_DisplayRDFAnchors), &b );
	changed = changed || (b != pDocLayout->m_bDisplayRDFAnchors);
	if(b != pDocLayout->m_bDisplayRDFAnchors || (pDocLayout->m_iGraphicTick < 2))
	{
		pDocLayout->m_bDisplayRDFAnchors = b;
		pDocLayout->formatAll();
		if(pDocLayout->getView())
		{
		    pDocLayout->getView()->updateScreen(false);
		}
	}

}

#ifdef ENABLE_SPELL
void FL_DocLayout::recheckIgnoredWords()
{
	// recheck the whole doc
	fl_DocSectionLayout * pSL = getFirstSection();
	if(pSL)
	{
		fl_ContainerLayout* b = pSL->getFirstLayout();
		while (b)
		{
			if(b->getContainerType() == FL_CONTAINER_BLOCK)
			{
				static_cast<fl_BlockLayout *>(b)->recheckIgnoredWords();
				b = static_cast<fl_BlockLayout *>(b)->getNextBlockInDocument();
			}
			else
			{
				b = b->getNext();
			}
		}
	}
}
#endif

/*!
 * Mark the whole document for a redraw
 */
void FL_DocLayout::setNeedsRedraw(void)
{
     if(!m_pFirstSection)
         return;
     setSkipUpdates(0);
     fl_BlockLayout * pBL = m_pFirstSection->getFirstBlock();
     while(pBL)
     {
	 pBL->setNeedsRedraw();
	 pBL = pBL->getNextBlockInDocument();
     }
}

void FL_DocLayout::_redrawUpdate(UT_Worker * pWorker)
{
	UT_return_if_fail(pWorker);

	// this is a static callback method and does not have a 'this' pointer.

	FL_DocLayout * pDocLayout = static_cast<FL_DocLayout *>(pWorker->getInstanceData());
	UT_return_if_fail(pDocLayout);

	if (!pDocLayout->m_pView || pDocLayout->isLayoutFilling())
	{
		// Win32 timers can fire prematurely on asserts
		// (the dialog's message pump releases the timers)
		return;
	}
	xxx_UT_DEBUGMSG(("SEVIOR: _redraw update \n"));
//
// Check if we're in the middle of a PieceTable change. If so don't redraw!
//
	PD_Document * pDoc = pDocLayout->getDocument();
	if(pDoc->isPieceTableChanging())
	{
		UT_DEBUGMSG(("PieceTable changing don't redraw \n"));
		return;
	}
	if(pDocLayout->isQuickPrint())
	{
	        UT_DEBUGMSG(("Doing a quickPrint don't redraw \n"));
		return;
	}
//
// Don't redraw while the selection is active
//
//	if(!pDocLayout->m_pView->isSelectionEmpty())
//	{
//		return;
//	}
//
// Lock out PieceTable changes till we finished.
//
	pDoc->setRedrawHappenning(true);
//
// Check if we've been asked to wait for a while..
//
	UT_uint32 skip = pDocLayout->getSkipUpdates();
	if(skip > 0)
	{
		skip--;
		pDocLayout->setSkipUpdates(skip);
		pDoc->setRedrawHappenning(false);
		return;
	}
//
// Check if we're printing. If so Bail out
//
	if(pDocLayout->m_pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
		pDoc->setRedrawHappenning(false);
		return;
	}
	bool bStopOnRebuild = false;
	fl_SectionLayout* pSL = pDocLayout->m_pFirstSection;
//
// This bit is to make sure the insertionPoint is always on screen.
//
	FV_View * pView = pDocLayout->getView();
	bool bEnd,bDir;
	bEnd = false;
	fl_BlockLayout * pBlock = NULL;
	fp_Run *pRun = NULL;
	UT_sint32 x1,x2,y1,y2;
	UT_uint32 height;
	UT_sint32 origY;
	pView->_findPositionCoords(pView->getPoint(),bEnd,x1,y1,x2,y2,height,bDir,&pBlock,&pRun);
	origY = y1;
	while (pSL && !bStopOnRebuild)
	{
		if(pDoc->isPieceTableChanging())
		{
			pDoc->setRedrawHappenning(false);
			return;
		}
		pSL->redrawUpdate();
//
// Might need some code here to check if we need a rebuild. In principle we should not need it.
//
		if(pSL->getType() == FL_SECTION_DOC)
		{
			if(static_cast<fl_DocSectionLayout *>(pSL)->needsRebuild())
			{
				bStopOnRebuild = true;
			}
		}
		if(!bStopOnRebuild)
		{
			pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
		}
	}
	pDocLayout->deleteEmptyColumnsAndPages();
	if(bStopOnRebuild)
	{
		UT_DEBUGMSG(("SEVIOR: Rebuilding from docLayout \n"));
		pDocLayout->rebuildFromHere(static_cast<fl_DocSectionLayout *>(pSL));
	}
	pView->_findPositionCoords(pView->getPoint(),bEnd,x1,y1,x2,y2,height,bDir,&pBlock,&pRun);
//
// If Y location has changed make sure it's still on screen
//
	if(y1 != origY)
	{
		UT_DEBUGMSG(("Line pos changed \n"));
//		UT_ASSERT(0);
		pView->_ensureInsertionPointOnScreen();
	}

//
// we've finished
//
	pDoc->setRedrawHappenning(false);
	pDocLayout->m_iRedrawCount++;
}

void FL_DocLayout::setPendingSmartQuote(fl_BlockLayout *bl, UT_uint32 of)
{
	xxx_UT_DEBUGMSG(("FL_DocLayout::setPendingSmartQuote(%x, %d)\n", bl, of));
	m_pPendingBlockForSmartQuote = bl;
	m_uOffsetForSmartQuote = of;
}

/* wjc sez....

This algorithm is based on my observation of how people actually use
quotation marks, sometimes in contravention of generally accepted
principles of punctuation.  It is certainly also true that my
observations are overwhelmingly of American English text, with a
smattering of various other languages observed from time to time.  I
don't believe that any algorithm for this can ever be perfect.  There
are too many infrequently-occurring but legitimate cases where a user
might want something else.  FWIW, I haven't tested out the specifics
of the smart quote algorithm in ThatOtherWordProcessor.

Some terms for the purpose of this discussion (I'm open to plenty of
advice on what specific items should fit in each of these classes):

sqBREAK  A structural break in a document.  For example, a paragraph
  break, a column break, a page break, the beginning or end of a
  document, etc.  Does not include font, size, bold/italic/underline
  changes (which are completely ignored for the purposes of this
  algorithm).

sqFOLLOWPUNCT A subset of layman's "punctuation".  I include only
  things that can normally occur after a quote mark with no intervening
  white space.  Includes period, exclamation point, question mark,
  semi-colon, colon, comma (but not parentheses, square and curly
  brackets, which are treated specially below).  There may be a few
  others that aren't on the kinds of keyboards I use, and there are
  certainly Latin1 and other locale-specific variants, but the point
  is that there are lots of random non-alphanumerics which aren't
  included in *PUNCT for this algorithm.

sqOPENPUNCT  The opening half of pairwise, non-quote punctuation.  Open
  parenthesis, open square bracket, open curly brace.

sqCLOSEPUNCT  The closing half of pairwise, non-quote punctuation.  Close
  parenthesis, close square bracket, close curly brace.

[[The idea about open and close punctuation was found in a mid-1980s
note by Dave Dunham, brought to my attention by Leonard Rosenthol
<leonardr@lazerware.com>.]]

sqOTHERPUNCT  Punctuation which is not sqFOLLOWPUNCT, sqOPENPUNCT, or
  sqCLOSEPUNCT.

sqALPHA  Alphabetic characters in the C isalpha() sense, but there are
  certainly some non-ASCII letter characters which belong in this
  bucket, too.

sqWHITE  White speace haracters in the C isspace() sense.

QUOTE  Any of ASCII double quote, ASCII quote (which many people call
  the ASCII single quote or the ASCII apostrophe), or ASCII backquote.
  I take it as given that a significant minority of people randomly or
  systematically interchange their use of ASCII quote and ASCII
  backquote, so I treat them the same in the algorithm.  The majority
  of people use ASCII quote for both opening and closing single quote.

PARITY  Whether a quote is single or double.  For ease of description,
  I'll say that the parity of single and double quotes are opposites
  of each other.  When QUOTEs are converted to curly form, the parity
  never changes.

================================================================

Given a QUOTE character, these conditions/rules are logically tested in
order:

0.  OK, first an easy exception case: If ASCII (single) quote (but not
ASCII backquote) appears between two sqALPHAs, it may be treated as an
apostrophe and converted to its curly form.  Otherwise, it is treated
like all other QUOTEs and follows the normal algorithm.

1.  If a QUOTE is immediately preceded by a curly quote of opposite
parity, it is converted to a curly quote in the same direction.

2.  If a QUOTE is immediately preceded by a curly quote of the same
parity, it is converted to a curly quote of opposite direction.

3.  If a QUOTE is immediately followed by a curly quote of opposite
parity, it is converted to a curly quote in the same direction.

4.  If a QUOTE is immediately followed by a curly quote of the same
parity, it is converted to a curly quote of opposite direction.

[[The above cases are intended to handle normal nested quotes or cases
where quotes enclose empty strings.  Different cultures use different
parities as start points for nested quotes, but the algorithm doesn't
care.]]

5.  If a QUOTE is immediately preceded by an sqOPENPUNCT, it is
converted to a curly quote in the open direction.

6.  If a QUOTE is immediately followed by a sqCLOSEPUNCT, it is
converted to a curly quote in the close direction.

7.  If a QUOTE is in isolation, it is converted to a curly quote
in the open direction.  It is in isolation if it is immediately
preceded and followed by either a sqBREAK or sqWHITE.  The things
before and after it don't have to be of the same type.

Rule 7 was originally that it didin't convert.  This does not make
sense because the most common use of smart quotes is as you're typing
where there is often a space before and nothing after (since it hasn't
been written yet!) -- Bobby Weinmann Feb 4, 2008

8.  If a QUOTE is immediately preceded by a sqBREAK or sqWHITE and
is immediately followed by anything other than a sqBREAK or sqWHITE,
it is converted to the opening form of curly quote.

9.  If a QUOTE is immediately followed by a sqBREAK, sqWHITE, or
sqFOLLOWPUNCT and is immediately preceded by anything other than sqBREAK
or sqWHITE, it is converted to the closing form of curly quote.

10.  Any other QUOTE is not converted.

================================================================

The algorithm doesn't make a special case of using ASCII double quote
as an inches indicator (there are other uses, like lat/long minutes;
ditto for the ASCII quote) because it is tough to tell if some numbers
with an ASCII double quote after them are intended to be one of those
"other things" or is just the end of a very long quote.  So, the
algorithm will be wrong sometimes in those cases.

It is otherwise sort of conservative, preferring to not convert things
it doesn't feel confident about.  The reason for that is that there is
a contemplated on-the-fly conversion to smart quotes, but there is no
contemplated on-the-fly conversion to ASCII QUOTEs.  So, if the
algorithm makes a mistake by not converting, the user can correct it
by directly entering the appropriate smart quote character or by
heuristically tricking AbiWord into converting it for him/her and then
fixing things up.  (That heuristic step shouldn't be necessary, you
know, but I think we all use software for which we have become
accustomed to such things.)

What about the occasions when this algorithm (or any alternative
algorithm) makes a mistake and converts a QUOTE to the curly form when
it really isn't wanted, in a particular case, by the user?  Although
the user can change it back, some contemplated implementation details
might run around behind the barn and re-convert it when the user isn't
looking.  I think we need a mechanism for dealing with that, but I
want to save proposals for that to be separate from the basic
algorithm.
*/

// The following are descriptions of the thing before or after a
// character being considered for smart quote promotion.  The thing
// is either a structural break in a document, or it is a literal
// character that is part of some class (in some cases the class is
// so small it has only one possible member).  The classes should
// look familar from the algorithm above.  There is a special class
// used only for the coding of rule:  sqDONTCARE in a rule means it
// doesn't matter what occurs in that position.
enum sqThingAt
{
	sqDONTCARE     = 1,
	sqQUOTEls      = 2,
	sqQUOTErs      = 3,
	sqQUOTEld      = 4,
	sqQUOTErd      = 5,// the smart quotes, left/right single/double
	sqBREAK        = 6,
	sqFOLLOWPUNCT  = 7,
	sqOPENPUNCT    = 8,
	sqCLOSEPUNCT   = 9,
	sqOTHERPUNCT   =10,
	sqALPHA        =11,
	sqWHITE        =12
};

// TODO:  This function probably needs tuning for non-Anglo locales.
static enum sqThingAt whatKindOfChar(UT_UCSChar thing)
{
	xxx_UT_DEBUGMSG(("what sort of character is %d 0x%x |%c|\n", thing, thing, thing));
	switch (thing)
	{
	case UCS_LQUOTE:     return sqQUOTEls;
	case UCS_RQUOTE:     return sqQUOTErs;
	case UCS_LDBLQUOTE:  return sqQUOTEld;
	case UCS_RDBLQUOTE:  return sqQUOTErd;

	case '(': case '{': case '[':  return sqOPENPUNCT;
	case ')': case '}': case ']':  return sqCLOSEPUNCT;

	case '.': case ',': case ';': case ':': case '!': case '?':  return sqFOLLOWPUNCT;

		// see similar control characters in fl_BlockLayout.cpp
	case UCS_FF:	// form feed, forced page break
	case UCS_VTAB:	// vertical tab, forced column break
	case UCS_LF:	// newline
	case UCS_TAB:	// tab
		return sqBREAK;
	}
	if (UT_UCS4_isalpha(thing)) return sqALPHA;
	if (UT_UCS4_ispunct(thing)) return sqOTHERPUNCT;
	if (UT_UCS4_isspace(thing)) return sqWHITE;

	return sqBREAK;  // supposed to be a character, but...!
}

struct sqTable
{
	enum sqThingAt before;
	UT_UCSChar thing;
	enum sqThingAt after;
	UT_UCSChar replacement;
};
// The idea of the table is to drive the algorithm without lots of
// cluttery code.  Something using this table pre-computes what the
// things are before and after the character in question, and then
// dances through this table looking for a match on all three.
// The final item in each row is the character to use to replace
// the candidate character.
//
// (Yeah, this table is big, but it is only used when a quote character
// shows up in typing or in a paste, and it goes pretty fast.)
//
// sqDONTCARE is like a wild card for the thing before or after, and
// UCS_UNKPUNK in the replacement position means don't do a replacement.
static struct sqTable sqTable_en[] =
{
	{sqALPHA,     '\'', sqALPHA,      UCS_RQUOTE},          // rule 0
	{sqALPHA,     '`',  sqALPHA,      UCS_RQUOTE},          // rule 0

	{sqQUOTEld,   '\'', sqDONTCARE,   UCS_LQUOTE},          // rule 1
	{sqQUOTErd,   '\'', sqDONTCARE,   UCS_RQUOTE},          // rule 1

	{sqQUOTEld,   '`',  sqDONTCARE,   UCS_LQUOTE},          // rule 1
	{sqQUOTErd,   '`',  sqDONTCARE,   UCS_RQUOTE},          // rule 1

	{sqQUOTEls,   '"',  sqDONTCARE,   UCS_LDBLQUOTE},       // rule 1
	{sqQUOTErs,   '"',  sqDONTCARE,   UCS_RDBLQUOTE},       // rule 1

	{sqQUOTEls,   '\'', sqDONTCARE,   UCS_RQUOTE},          // rule 2
	{sqQUOTErs,   '\'', sqDONTCARE,   UCS_LQUOTE},          // rule 2

	{sqQUOTEls,   '`',  sqDONTCARE,   UCS_RQUOTE},          // rule 2
	{sqQUOTErs,   '`',  sqDONTCARE,   UCS_LQUOTE},          // rule 2

	{sqQUOTEld,   '"',  sqDONTCARE,   UCS_RDBLQUOTE},       // rule 2
	{sqQUOTErd,   '"',  sqDONTCARE,   UCS_LDBLQUOTE},       // rule 2

	{sqDONTCARE,   '\'', sqQUOTEld,   UCS_LQUOTE},          // rule 3
	{sqDONTCARE,   '\'', sqQUOTErd,   UCS_RQUOTE},          // rule 3

	{sqDONTCARE,   '`',  sqQUOTEld,   UCS_LQUOTE},          // rule 3
	{sqDONTCARE,   '`',  sqQUOTErd,   UCS_RQUOTE},          // rule 3

	{sqDONTCARE,   '"',  sqQUOTEls,   UCS_LDBLQUOTE},       // rule 3
	{sqDONTCARE,   '"',  sqQUOTErs,   UCS_RDBLQUOTE},       // rule 3

	{sqDONTCARE,   '\'', sqQUOTEls,   UCS_RQUOTE},          // rule 4
	{sqDONTCARE,   '\'', sqQUOTErs,   UCS_LQUOTE},          // rule 4

	{sqDONTCARE,   '`',  sqQUOTEls,   UCS_RQUOTE},          // rule 4
	{sqDONTCARE,   '`',  sqQUOTErs,   UCS_LQUOTE},          // rule 4

	{sqDONTCARE,   '"',  sqQUOTEld,   UCS_RDBLQUOTE},       // rule 4
	{sqDONTCARE,   '"',  sqQUOTErd,   UCS_LDBLQUOTE},       // rule 4

	{sqOPENPUNCT,  '\'', sqDONTCARE,  UCS_LQUOTE},          // rule 5
	{sqOPENPUNCT,  '`',  sqDONTCARE,  UCS_LQUOTE},          // rule 5
	{sqOPENPUNCT,  '"',  sqDONTCARE,  UCS_LDBLQUOTE},       // rule 5

	{sqDONTCARE, '\'', sqCLOSEPUNCT,  UCS_RQUOTE},          // rule 6
	{sqDONTCARE, '`',  sqCLOSEPUNCT,  UCS_RQUOTE},          // rule 6
	{sqDONTCARE, '"',  sqCLOSEPUNCT,  UCS_RDBLQUOTE},       // rule 6

	{sqBREAK,      '\'', sqBREAK,     UCS_LQUOTE},         // rule 7
	{sqWHITE,      '\'', sqBREAK,     UCS_LQUOTE},         // rule 7
	{sqBREAK,      '\'', sqWHITE,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '\'', sqWHITE,     UCS_UNKPUNK},         // rule 7

	{sqBREAK,      '`',  sqBREAK,     UCS_LQUOTE},         // rule 7
	{sqWHITE,      '`',  sqBREAK,     UCS_LQUOTE},         // rule 7
	{sqBREAK,      '`',  sqWHITE,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '`',  sqWHITE,     UCS_UNKPUNK},         // rule 7

	{sqBREAK,      '"',  sqBREAK,     UCS_LDBLQUOTE},         // rule 7
	{sqWHITE,      '"',  sqBREAK,     UCS_LDBLQUOTE},         // rule 7
	{sqBREAK,      '"',  sqWHITE,     UCS_LDBLQUOTE},         // rule 7
	{sqWHITE,      '"',  sqWHITE,     UCS_LDBLQUOTE},         // rule 7

	{sqBREAK,      '\'', sqDONTCARE,  UCS_LQUOTE},          // rule 8
	{sqWHITE,      '\'', sqDONTCARE,  UCS_LQUOTE},          // rule 8

	{sqBREAK,      '`',  sqDONTCARE,  UCS_LQUOTE},          // rule 8
	{sqWHITE,      '`',  sqDONTCARE,  UCS_LQUOTE},          // rule 8

	{sqBREAK,      '"',  sqDONTCARE,  UCS_LDBLQUOTE},       // rule 8
	{sqWHITE,      '"',  sqDONTCARE,  UCS_LDBLQUOTE},       // rule 8

	{sqDONTCARE,   '\'', sqBREAK,     UCS_RQUOTE},          // rule 9
	{sqDONTCARE,   '\'', sqWHITE,     UCS_RQUOTE},          // rule 9
	{sqDONTCARE,   '\'', sqFOLLOWPUNCT, UCS_RQUOTE},          // rule 9

	{sqDONTCARE,   '`',  sqBREAK,     UCS_RQUOTE},          // rule 9
	{sqDONTCARE,   '`',  sqWHITE,     UCS_RQUOTE},          // rule 9
	{sqDONTCARE,   '`',  sqFOLLOWPUNCT, UCS_RQUOTE},          // rule 9

	{sqDONTCARE,   '"',  sqBREAK,     UCS_RDBLQUOTE},       // rule 9
	{sqDONTCARE,   '"',  sqWHITE,     UCS_RDBLQUOTE},       // rule 9
	{sqDONTCARE,   '"',  sqFOLLOWPUNCT, UCS_RDBLQUOTE},       // rule 9

	// following rules are the same as falling off the end of the list...

	//{sqDONTCARE,   '\'', sqDONTCARE,   UCS_UNKPUNK},        // rule 10
	//{sqDONTCARE,   '`',  sqDONTCARE,   UCS_UNKPUNK},        // rule 10
	//{sqDONTCARE,   '"',  sqDONTCARE,   UCS_UNKPUNK},        // rule 10

	{sqDONTCARE, 0, sqDONTCARE, UCS_UNKPUNK}  // signals end of table
};

void FL_DocLayout::considerSmartQuoteCandidateAt(fl_BlockLayout *block, UT_uint32 offset)
{
	if (!block)
		return;
	if (m_pView->isHdrFtrEdit())
		return;
	if (!getSmartQuotes())
		return;
	if (!m_pView->m_bAllowSmartQuoteReplacement)
		return;

	setPendingSmartQuote(NULL, 0);  // avoid recursion
	UT_GrowBuf pgb(1024);
	block->getBlockBuf(&pgb);
	// this is for the benefit of the UT_DEBUGMSG and should be changed to
	// something other than '?' if '?' ever shows up as UT_isSmartQuotableCharacter()
	UT_UCSChar c = '?';
	if (pgb.getLength() > offset) c = *pgb.getPointer(offset);
	UT_DEBUGMSG(("FL_DocLayout::considerSmartQuoteCandidateAt(%p, %d)  |%c|\n", block, offset, c));

	//  there are some operations that leave a dangling pending
	//  smart quote, so just double check before plunging onward
	if (UT_isSmartQuotableCharacter(c))
	{
		enum sqThingAt before = sqBREAK, after = sqBREAK;
		if (offset > 0)
		{
			// TODO: is there a need to see if this is on a run boundary?
			// TODO: Within a block, are there runs that are significant
			// TODO: breaks or whatever?
			before = whatKindOfChar(*pgb.getPointer(offset - 1));
		}
		else
		{
			// candidate was the first character in the block, so
			// see what was at the end of the previous block, if any
			fl_BlockLayout *ob = static_cast<fl_BlockLayout *>(block->getPrev());
			if (ob)
			{
				fp_Run *last, *r = ob->getFirstRun();
				do
				{
					last = r;
				} while ((r = r->getNextRun()));  // assignment
				if (last  &&  (FPRUN_TEXT == last->getType())  &&  last->getLength() > 0)
				{
					fp_Line *blocks_line, *lasts_line;
					blocks_line = block->getFirstRun()->getLine();
					lasts_line = last->getLine();
					if (blocks_line == lasts_line)
					{
						// last run of previous block was a text run on the same line
						// so find out what the final character was
						UT_GrowBuf pgb_b(1024);
						ob->getBlockBuf(&pgb_b);
						if (pgb_b.getLength())
						{
							before = whatKindOfChar(*pgb_b.getPointer(pgb_b.getLength()-1));
						}
					}
				}
			}
		}

		if (offset+1 < pgb.getLength())
		{
			// TODO: is there a need to see if this is on a run boundary?
			// TODO: Within a block, are there runs that are significant
			// TODO: breaks or whatever?
			after = whatKindOfChar(*pgb.getPointer(offset + 1));
		}
		else
		{
			// candidate was the last character in a block, so see
			// what's at the beginning of the next block, if any
			fl_BlockLayout *ob = static_cast<fl_BlockLayout *>(block->getNext());
			if (ob)
			{
				fp_Run *r = ob->getFirstRun();
				if (r  &&  (FPRUN_TEXT == r->getType()))
				{
					// first run of next block is a text run, so
					// see what the first character was
					UT_GrowBuf pgb_a(1024);
					ob->getBlockBuf(&pgb_a);
					if (pgb_a.getLength())
					{
						after = whatKindOfChar(*pgb_a.getPointer(0));
					}
				}
			}
		}

		// we now know what the before and after things are, so
		// spin through the table.
		UT_UCSChar replacement = UCS_UNKPUNK;  // means don't replace
		// TODO:  select a table based on default locale or on the locale
		// TODO:  of the fragment of text we're working in (locale tagging
		// TODO:  of text doesn't exist in Abi as of this writing)
		struct sqTable *table = sqTable_en;
		for (unsigned int tdex=0; table[tdex].thing; ++tdex)
		{
			if (c != table[tdex].thing) continue;
			if (table[tdex].before == sqDONTCARE  ||  table[tdex].before == before)
			{
				if (table[tdex].after == sqDONTCARE  ||  table[tdex].after == after)
				{
					replacement = table[tdex].replacement;
					break;
				}
			}
		}
		UT_DEBUGMSG(("before %d, after %d, replace %x\n", before, after, replacement));
		if (replacement != UCS_UNKPUNK)
		{
            //UT_sint32 s1,s2,s3,s4,s5;
            //bool b1;
            //fp_Run * pThisRun = block->findPointCoords(block->getPosition() + offset,false,s1,s2,s3,s4,s5,b1);

            xxx_UT_DEBUGMSG(("pThisRun [0x%x], vis dir. %d\n", pThisRun, pThisRun->getVisDirection()));

			gint nOuterQuoteStyleIndex = 0; // Default to English
			gint nInnerQuoteStyleIndex = 1; // Default to English
			bool bUseCustomQuotes = false;
			bool bOK2 = true;
			
			// 1st - See if we should use custom quotes
			if (m_pPrefs)
			{
				bOK2 = m_pPrefs->getPrefsValueBool( static_cast<const gchar *>(XAP_PREF_KEY_CustomSmartQuotes), &bUseCustomQuotes );
				if (bOK2 && bUseCustomQuotes)
				{
					bool bOK1 = m_pPrefs->getPrefsValueInt( static_cast<const gchar *>(XAP_PREF_KEY_OuterQuoteStyle), nOuterQuoteStyleIndex );
					if (!bOK1)
					{
						nOuterQuoteStyleIndex = 0; // English if bad
					}
					else
					{
						bool bOK = m_pPrefs->getPrefsValueInt( static_cast<const gchar *>(XAP_PREF_KEY_InnerQuoteStyle), nInnerQuoteStyleIndex );
						if (!bOK)
						{
							nInnerQuoteStyleIndex = 1; // English if bad
						}
					}
				}
			}

			// 2nd - Not using custom quotes, look up doc lang
			if (!bOK2 || !bUseCustomQuotes)
			{
				std::string lang;
				PP_PropertyVector props_in;
				if (m_pView->getCharFormat(props_in))
				{
					lang = PP_getAttribute("lang", props_in);
				}

				if (!lang.empty())
				{
					const XAP_LangInfo* found = XAP_EncodingManager::findLangInfoByLocale(lang.c_str());
					if (found)
					{
						nOuterQuoteStyleIndex = found->outerQuoteIdx;
						nInnerQuoteStyleIndex = found->innerQuoteIdx;
					}
				}
			}

			// 3rd - bad thing happened, go with English
			if (nOuterQuoteStyleIndex < 0 || nInnerQuoteStyleIndex < 0)
			{
				nOuterQuoteStyleIndex = 0;
				nInnerQuoteStyleIndex = 1;
			}
			bool bNoChange = false;
			switch (replacement)
			{
			case UCS_LQUOTE:
				replacement = XAP_EncodingManager::smartQuoteStyles[nInnerQuoteStyleIndex].leftQuote;
				bNoChange = (replacement == c);
				break;
			case UCS_RQUOTE:
				replacement = XAP_EncodingManager::smartQuoteStyles[nInnerQuoteStyleIndex].rightQuote;
				bNoChange = (replacement == c);
				break;
			case UCS_LDBLQUOTE:
				replacement = XAP_EncodingManager::smartQuoteStyles[nOuterQuoteStyleIndex].leftQuote;
				bNoChange = (replacement == c);
				break;
			case UCS_RDBLQUOTE:
				replacement = XAP_EncodingManager::smartQuoteStyles[nOuterQuoteStyleIndex].rightQuote;
				bNoChange = (replacement == c);
				break;
			}
			if(bNoChange)
			{
			      UT_DEBUGMSG(("No change detected \n"));
			      return ;
			}
			// your basic emacs (save-excursion...)  :-)
			PT_DocPosition saved_pos, quotable_at;
			saved_pos = m_pView->getPoint();
			quotable_at = block->getPosition(false) + offset;

			m_pView->moveInsPtTo(quotable_at);
			// delete/insert create change records for UNDO
			m_pView->cmdSelectNoNotify(quotable_at, quotable_at + 1);
			m_pView->cmdCharInsert(&replacement, 1);
			m_pView->moveInsPtTo(saved_pos);
		}
	}
}

void FL_DocLayout::notifyBlockIsBeingDeleted(fl_BlockLayout *pBlock)
{
	if(pBlock == m_pPendingBlockForSpell)
	{
		m_pPendingBlockForSpell = NULL;
	}

	if(pBlock == m_pPendingBlockForSmartQuote)
	{
		m_pPendingBlockForSmartQuote = NULL;
	}
#ifdef ENABLE_SPELL
	pBlock->dequeueFromSpellCheck();
#endif
}

inline fl_AutoNum * FL_DocLayout::getListByID(UT_uint32 id) const
{
	return m_pDoc->getListByID(id);
}

inline fl_AutoNum * FL_DocLayout::getNthList(UT_uint32 i) const
{
	return m_pDoc->getNthList(i);
}

inline UT_uint32 FL_DocLayout::getListsCount(void) const
{
	return m_pDoc->getListsCount();
}

inline void FL_DocLayout::addList(fl_AutoNum * pAutoNum)
{
	m_pDoc->addList(pAutoNum);
}

void FL_DocLayout::notifyListeners(AV_ChangeMask mask)
{
	if (m_pView)
		m_pView->notifyListeners(mask);
}
