/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <stdio.h>
#include <stdlib.h>

#include "ut_types.h"
#include "fl_DocListener.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_BlockLayout.h"
#include "fl_AutoNum.h"
#include "fp_Page.h"
#include "fp_Run.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "pp_Property.h"
#include "gr_Graphics.h"
#include "xav_Listener.h"
#include "xap_App.h"
#include "ap_Prefs.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_timer.h"
#include "ut_string.h"

#define REDRAW_UPDATE_MSECS	500

FL_DocLayout::FL_DocLayout(PD_Document* doc, GR_Graphics* pG)
	:	m_hashFontCache(19)
{
	m_pDoc = doc;
	m_pG = pG;
	m_pView = NULL;

	m_pBackgroundCheckTimer = NULL;
	m_pPendingBlockForSpell = NULL;
	m_pPendingWordForSpell = NULL;
	m_pPendingBlockForSmartQuote = NULL;
	m_uOffsetForSmartQuote = 0;
	m_pFirstSection = NULL;
	m_pLastSection = NULL;
	m_bSpellCheckCaps = true;
	m_bSpellCheckNumbers = true;
	m_bSpellCheckInternet = true;
	m_pPrefs = NULL;
	m_bStopSpellChecking = false;
	m_bImSpellCheckingNow = false;
	m_uDocBackgroundCheckReasons = 0;
	m_pRedrawUpdateTimer = UT_Timer::static_constructor(_redrawUpdate, this, m_pG);
	if (m_pRedrawUpdateTimer)
	{
		m_pRedrawUpdateTimer->set(REDRAW_UPDATE_MSECS);
		m_pRedrawUpdateTimer->start();
	}

	// TODO the following (both the new() and the addListener() cause
	// TODO malloc's to occur.  we are currently inside a constructor
	// TODO and are not allowed to report failure.

	// Turn off list updating until document is formatted

	m_pDoc->disableListUpdates();
	m_pDocListener = new fl_DocListener(doc, this);
	doc->addListener(static_cast<PL_Listener *>(m_pDocListener),&m_lid);
//
// Put the default View color to white
//
	strncpy(m_szCurrentTransparentColor,(const char *) XAP_PREF_DEFAULT_ColorForTransparent,9);

#ifdef FMT_TEST
	m_pDocLayout = this;
#endif

}

FL_DocLayout::~FL_DocLayout()
{
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
		while(m_bImSpellCheckingNow == true)
		{
		}
	}
	
	DELETEP(m_pBackgroundCheckTimer);
	DELETEP(m_pPendingWordForSpell);

	if (m_pRedrawUpdateTimer)
	{
		m_pRedrawUpdateTimer->stop();
	}
	
	DELETEP(m_pRedrawUpdateTimer);

	UT_VECTOR_PURGEALL(fp_Page *, m_vecPages);
	
	while (m_pFirstSection)
	{
		fl_DocSectionLayout* pNext = m_pFirstSection->getNextDocSection();
		delete m_pFirstSection;
		m_pFirstSection = pNext;
	}

	UT_HASH_PURGEDATA(GR_Font *, m_hashFontCache);
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
		XAP_App * pApp = m_pView->getApp();
		UT_ASSERT(pApp);
		XAP_Prefs *pPrefs= pApp->getPrefs();
		UT_ASSERT(pPrefs);

		if (pPrefs)
		{
			// remember this so we can remove the listener later
			m_pPrefs = pPrefs;

			// initialize the vars here
			_prefsListener( pApp, pPrefs, NULL, this );

			// keep updating itself	
			pPrefs->addListener ( _prefsListener, this ); 
			bool b;
			if (m_pPrefs->getPrefsValueBool((const XML_Char *)"DebugFlash",&b)  &&  b == true)
			{
				addBackgroundCheckReason(bgcrDebugFlash);
			}
		}
	}
}

UT_sint32 FL_DocLayout::getHeight()
{
	UT_sint32 iHeight = 0;
	int count = m_vecPages.getItemCount();

	for (int i=0; i<count; i++)
	{
		fp_Page* p = (fp_Page*) m_vecPages.getNthItem(i);

		iHeight += p->getHeight();
	}

	if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		// add page view dimensions 
		iHeight += fl_PAGEVIEW_PAGE_SEP * (count - 1);
		iHeight += fl_PAGEVIEW_MARGIN_Y * 2;
	}

	return iHeight;
}

UT_sint32 FL_DocLayout::getWidth()
{
	UT_sint32 iWidth = 0;
	int count = m_vecPages.getItemCount();

	for (int i=0; i<count; i++)
	{
		fp_Page* p = (fp_Page*) m_vecPages.getNthItem(i);

		// we layout pages vertically, so this is max, not sum
		if ((UT_sint32) iWidth < p->getWidth())
			iWidth = p->getWidth();
	}

	if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		// add page view dimensions 
		iWidth += fl_PAGEVIEW_MARGIN_X * 2;
	}

	return iWidth;
}

GR_Font* FL_DocLayout::findFont(const PP_AttrProp * pSpanAP,
								const PP_AttrProp * pBlockAP,
								const PP_AttrProp * pSectionAP,
								UT_sint32 iUseLayoutResolution)
{
	GR_Font* pFont = NULL;

	const char* pszFamily	= PP_evalProperty("font-family",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszStyle	= PP_evalProperty("font-style",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszVariant	= PP_evalProperty("font-variant",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszWeight	= PP_evalProperty("font-weight",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszStretch	= PP_evalProperty("font-stretch",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszSize		= PP_evalProperty("font-size",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszPosition = PP_evalProperty("text-position",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);

	// for superscripts and subscripts, we'll automatically shrink the font size
	if ((0 == UT_strcmp(pszPosition, "superscript")) ||
		(0 == UT_strcmp(pszPosition, "subscript")))
	{
		double newSize = UT_convertToPoints(pszSize) * 2.0 / 3.0;
		pszSize = UT_formatDimensionedValue(newSize,"pt",".0");
	}
	// NOTE: we currently favor a readable hash key to make debugging easier
	// TODO: speed things up with a smaller key (the three AP pointers?) 
	char key[500];
	sprintf(key,"%s;%s;%s;%s;%s;%s,%i",pszFamily, pszStyle, pszVariant, pszWeight, pszStretch, pszSize, iUseLayoutResolution);
	
	UT_HashEntry* pEntry = m_hashFontCache.findEntry(key);
	if (!pEntry)
	{
		// TODO -- note that we currently assume font-family to be a single name,
		// TODO -- not a list.  This is broken.

		if(iUseLayoutResolution)
		{
			m_pG->setLayoutResolutionMode(true);
		}
		pFont = m_pG->findFont(pszFamily, pszStyle, pszVariant, pszWeight, pszStretch, pszSize);
		m_pG->setLayoutResolutionMode(false);
		UT_ASSERT(pFont);

		// add it to the cache
		UT_sint32 res = m_hashFontCache.addEntry(key, NULL, pFont);
		UT_ASSERT(res==0);
	}
	else
	{
		pFont = (GR_Font*) pEntry->pData;
	}

	return pFont;
}


GR_Font* FL_DocLayout::findFont(const PP_AttrProp * pSpanAP,
								const PP_AttrProp * pBlockAP,
								const PP_AttrProp * pSectionAP,
								UT_sint32 iUseLayoutResolution, bool isField)
{
	GR_Font* pFont = NULL;

	const char* pszFamily	= PP_evalProperty("font-family",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszField	= PP_evalProperty("field-font",NULL,pBlockAP,NULL, m_pDoc, true);
	const char* pszStyle	= PP_evalProperty("font-style",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszVariant	= PP_evalProperty("font-variant",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszWeight	= PP_evalProperty("font-weight",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszStretch	= PP_evalProperty("font-stretch",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszSize		= PP_evalProperty("font-size",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszPosition = PP_evalProperty("text-position",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);

	// for superscripts and subscripts, we'll automatically shrink the font size
	if ((0 == UT_strcmp(pszPosition, "superscript")) ||
		(0 == UT_strcmp(pszPosition, "subscript")))
	{
		double newSize = UT_convertToPoints(pszSize) * 2.0 / 3.0;
		pszSize = UT_formatDimensionedValue(newSize,"pt",".0");
	}

	// NOTE: we currently favor a readable hash key to make debugging easier
	// TODO: speed things up with a smaller key (the three AP pointers?) 
	char key[500];
	if(pszField!="NULL" && pszField != NULL && isField==true)
	{
		pszFamily = pszField;
	}
	sprintf(key,"%s;%s;%s;%s;%s;%s,%i",pszFamily, pszStyle, pszVariant, pszWeight, pszStretch, pszSize, iUseLayoutResolution);
	UT_HashEntry* pEntry = m_hashFontCache.findEntry(key);
	if (!pEntry)
	{
		// TODO -- note that we currently assume font-family to be a single name,
		// TODO -- not a list.  This is broken.

		if(iUseLayoutResolution)
		{
			m_pG->setLayoutResolutionMode(true);
		}
		pFont = m_pG->findFont(pszFamily, pszStyle, pszVariant, pszWeight, pszStretch, pszSize);
		m_pG->setLayoutResolutionMode(false);
		UT_ASSERT(pFont);

		// add it to the cache
		UT_sint32 res = m_hashFontCache.addEntry(key, NULL, pFont);
		UT_ASSERT(res==0);
	}
	else
	{
		pFont = (GR_Font*) pEntry->pData;
	}

	return pFont;
}

UT_uint32 FL_DocLayout::countPages()
{
	return m_vecPages.getItemCount();
}

fp_Page* FL_DocLayout::getNthPage(int n)
{
	UT_ASSERT(m_vecPages.getItemCount() > 0);

	return (fp_Page*) m_vecPages.getNthItem(n);
}

fp_Page* FL_DocLayout::getFirstPage()
{
	if (m_vecPages.getItemCount() == 0)
	{
		return NULL;
	}
	
	return (fp_Page*) m_vecPages.getNthItem(0);
}

fp_Page* FL_DocLayout::getLastPage()
{
	if (m_vecPages.getItemCount() == 0)
	{
		return NULL;
	}
	
	return (fp_Page*) m_vecPages.getNthItem(m_vecPages.getItemCount()-1);
}

void FL_DocLayout::deletePage(fp_Page* pPage)
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
		
	m_vecPages.deleteNthItem(ndx);
	delete pPage;
		
	// let the view know that we created a new page,
	// so that it can update the scroll bar ranges
	// and whatever else it needs to do.

	if (m_pView)
	{
		m_pView->notifyListeners(AV_CHG_PAGECOUNT);
	}
}

fp_Page* FL_DocLayout::addNewPage(fl_DocSectionLayout* pOwner)
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
									m_pDoc->m_docPageSize,
									pOwner);
	if (pLastPage)
	{
		UT_ASSERT(pLastPage->getNext() == NULL);

		pLastPage->setNext(pPage);
	}
	pPage->setPrev(pLastPage);
	m_vecPages.addItem(pPage);

	// let the view know that we created a new page,
	// so that it can update the scroll bar ranges
	// and whatever else it needs to do.

	if (m_pView)
	{
		m_pView->notifyListeners(AV_CHG_PAGECOUNT);
	}
	
	return pPage;
}

/*!
  Find block at document position
  \param pos Document position
  \return Block at specified posistion, or the first block to the
          rigth of that position. May return NULL.

*/
fl_BlockLayout* FL_DocLayout::findBlockAtPosition(PT_DocPosition pos)
{
	fl_BlockLayout* pBL = NULL;
	PL_StruxFmtHandle sfh;

	PT_DocPosition posEOD;
	bool bRes;

	bRes = m_pDoc->getBounds(true, posEOD);
	UT_ASSERT(bRes);

	bRes = m_pDoc->getStruxOfTypeFromPosition(m_lid, pos, PTX_Block, &sfh);
	// If block wasn't found at position, try finding it to the right,
	// limited only by the EOD.
	while(!bRes && (pos < posEOD))
	{
		pos++;
		bRes = m_pDoc->getStruxOfTypeFromPosition(m_lid, pos, PTX_Block, &sfh);
	}

	if (bRes)
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		switch (pL->getType())
		{
		case PTX_Block:
			pBL = static_cast<fl_BlockLayout *>(pL);
			break;
				
		case PTX_Section:
		default:
			UT_ASSERT((UT_SHOULD_NOT_HAPPEN)); 
			// We asked for a block, and we got a section.  Bad
		}
	}
	else
	{
		UT_ASSERT((0));
		return NULL;
	}

	if(pBL->getSectionLayout()->getType() == FL_SECTION_HDRFTR)
	{
		fl_HdrFtrShadow * pShadow = NULL;
		FV_View * pView = getView();
		if(pView && pView->isHdrFtrEdit())
		{
			pShadow = pView->getEditShadow();
		}
		else
		{
			pShadow = ((fl_HdrFtrSectionLayout *) pBL->getSectionLayout())->getFirstShadow();
		}
		if(pShadow != NULL)
			pBL = pShadow->findMatchingBlock(pBL);
		else
		{
			UT_DEBUGMSG(("SEVIOR: No Shadow! But there should be ! \n"));
			//	UT_ASSERT(0);
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

void FL_DocLayout::deleteEmptyPages(void)
{
	int i;

	int iCountPages = m_vecPages.getItemCount();
	for (i=iCountPages - 1; i>=0; i--)
	{
		fp_Page* p = (fp_Page*) m_vecPages.getNthItem(i);

		if (p->isEmpty())
		{
			deletePage(p);
		}
	}
}

void FL_DocLayout::formatAll()
{
	UT_ASSERT(m_pDoc);
	m_pDoc->enableListUpdates();
	fl_SectionLayout* pSL = m_pFirstSection;
	while (pSL)
	{
		pSL->format();
		
		pSL = pSL->getNext();
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
		pSL->updateLayout();
		
		pSL = pSL->getNext();
	}
	
	deleteEmptyColumnsAndPages();
}


void FL_DocLayout::updateColor()
{
	UT_ASSERT(m_pDoc);
	FV_View * pView = getView();
	if(pView)
	{
		XAP_App * pApp = pView->getApp();
		XAP_Prefs * pPrefs = pApp->getPrefs();
		const XML_Char * pszTransparentColor = NULL;
		pPrefs->getPrefsValue((const XML_Char *) XAP_PREF_KEY_ColorForTransparent,&pszTransparentColor);
//
// Save the new preference color
//
		strncpy(m_szCurrentTransparentColor,pszTransparentColor,9);
	}  
//
// Now loop through the document and update the Background color
//
	fl_DocSectionLayout* pDSL = (fl_DocSectionLayout *) m_pFirstSection;
	while (pDSL)
	{
		pDSL->setPaperColor();
		pDSL->updateBackgroundColor();
		pDSL = pDSL->getNextDocSection();
	}
//
// Redraw the view associated with this document.
//
	if(pView)
	{
		// remember the state of the cursor.
		bool bCursorErased = false;
		if (pView->isCursorOn() == true)
		{
			pView->eraseInsertionPoint();
			bCursorErased = true;
		}
		pView->updateScreen(false);
//
// Redraw the cursor if needed
//
		if (bCursorErased == true)
		{
			pView->drawInsertionPoint();
		}
	}

}

#define BACKGROUND_CHECK_MSECS 100

void FL_DocLayout::_toggleAutoSpell(bool bSpell)
{
	bool bOldAutoSpell = getAutoSpellCheck();
	if (bSpell)
	{
		addBackgroundCheckReason(bgcrSpelling);
	}
	else
	{
		removeBackgroundCheckReason(bgcrSpelling);
	}

	UT_DEBUGMSG(("FL_DocLayout::_toggleAutoSpell (%s)\n", bSpell ? "true" : "false" ));

	if (bSpell)
	{
		// recheck the whole doc
		fl_DocSectionLayout * pSL = getFirstSection();
		while (pSL)
		{
			fl_BlockLayout* b = pSL->getFirstBlock();
			while (b)
			{
				// TODO: just check and remove matching squiggles
				// for now, destructively recheck the whole thing
				queueBlockForBackgroundCheck(bgcrSpelling, b);
				b = b->getNext();
			}
			pSL = (fl_DocSectionLayout *) pSL->getNext();
		}
	}
	else
	{
		// remove the squiggles, too
		fl_DocSectionLayout * pSL = getFirstSection();
		while (pSL)
		{
			fl_BlockLayout* b = pSL->getFirstBlock();
			while (b)
			{
				b->removeBackgroundCheckReason(bgcrSpelling);
				b->_purgeSquiggles();
				b = b->getNext();
			}
			pSL = (fl_DocSectionLayout *) pSL->getNext();
		}
		if (bOldAutoSpell)
		{
			// If we're here, it was set to TRUE before but now it is being set
			// to FALSE. This means that it is the user setting it. That's good.
			m_pView->draw(NULL);
			// A pending word would be bad. Not sure why it's not ignored once autospell is off, but for now it should definattely be annulled.
			setPendingWordForSpell(NULL, NULL);
		}
	}
}

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

void FL_DocLayout::_backgroundCheck(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);

	// this is a static callback method and does not have a 'this' pointer.

	FL_DocLayout * pDocLayout = (FL_DocLayout *) pTimer->getInstanceData();
	UT_ASSERT(pDocLayout);

	if (!pDocLayout->m_pView)
	{
		// Win32 timers can fire prematurely on asserts
		// (the dialog's message pump releases the timers)
		return;
	}

	if(pDocLayout->m_bStopSpellChecking == true || pDocLayout->m_bImSpellCheckingNow == true)
	{
		return;
	}
	// Code added to hold spell checks during block insertions

	if(pDocLayout->m_pDoc->isPieceTableChanging() == true)
	{
		return;
	}
	pDocLayout->m_bImSpellCheckingNow = true;

	// prevent getting a new timer hit before we've finished this one by
	// temporarily disabling the timer
	//pDocLayout->m_pBackgroundCheckTimer->stop();

	UT_Vector* vecToCheck = &pDocLayout->m_vecUncheckedBlocks;
	UT_ASSERT(vecToCheck);

	UT_uint32 i = vecToCheck->getItemCount();

	if (i > 0)
	{
		fl_BlockLayout *pB = (fl_BlockLayout *) vecToCheck->getFirstItem();

		if (pB != NULL)
		{
			for (unsigned int bitdex=0; bitdex<8*sizeof(pB->m_uBackgroundCheckReasons); ++bitdex)
			{
				// This looping seems like a lot of wasted effort when we
				// don't define meaning for most of the bits, but it's small
				// effort compared to all that squiggle stuff that goes on
				// for the spelling stuff.
				UT_uint32 mask;
				mask = (1 << bitdex);
				if (pB->hasBackgroundCheckReason(mask))
				{
					//	note that we remove this reason from queue before checking it
					//	(otherwise asserts could trigger redundant recursive calls)
					pB->removeBackgroundCheckReason(mask);
					switch (mask)
					{
					case bgcrNone:
						break;
					case bgcrDebugFlash:
						pB->debugFlashing();
						break;
					case bgcrSpelling:
						pB->checkSpelling();
						break;
					case bgcrSmartQuotes:
					default:
						break;
					}
				}
			}
			if (!pB->m_uBackgroundCheckReasons)
			{
				vecToCheck->deleteNthItem(0);
				i--;
			}
		}
	}

	if (i != 0 && pDocLayout->m_bStopSpellChecking == false)
	{
		// restart timer unless it's not needed any more
		//pDocLayout->m_pBackgroundCheckTimer->start();
	}
	pDocLayout->m_bImSpellCheckingNow = false;
}

void FL_DocLayout::queueBlockForBackgroundCheck(UT_uint32 reason, fl_BlockLayout *pBlock, bool bHead)
{
	/*
	  This routine queues up blocks for timer-driven spell checking, etc.  
	  By default, this is a FIFO queue, but it can be explicitly 
	  reprioritized by setting bHead == true.  
	*/
	if (!m_pBackgroundCheckTimer)
	{
		m_pBackgroundCheckTimer = UT_Timer::static_constructor(_backgroundCheck, this, m_pG);
		if (m_pBackgroundCheckTimer)
			m_pBackgroundCheckTimer->set(BACKGROUND_CHECK_MSECS);
	}
	else
	{
		//		m_pBackgroundCheckTimer->stop();
		m_bStopSpellChecking = false;
		m_pBackgroundCheckTimer->start();
	}

	if (hasBackgroundCheckReason(bgcrDebugFlash))
	{
		pBlock->addBackgroundCheckReason(bgcrDebugFlash);
	}
	pBlock->addBackgroundCheckReason(reason);

	UT_sint32 i = m_vecUncheckedBlocks.findItem(pBlock);

	if (i < 0)
	{
		// add it
		if (bHead)
			m_vecUncheckedBlocks.insertItemAt(pBlock, 0);
		else
			m_vecUncheckedBlocks.addItem(pBlock);
	}
	else if (bHead)
	{
		// bubble it to the start
		m_vecUncheckedBlocks.deleteNthItem(i);
		m_vecUncheckedBlocks.insertItemAt(pBlock, 0);
	}
}

void FL_DocLayout::dequeueBlockForBackgroundCheck(fl_BlockLayout *pBlock)
{
	UT_sint32 i = m_vecUncheckedBlocks.findItem(pBlock);

	if (i>=0)
	{
		m_vecUncheckedBlocks.deleteNthItem(i);
	}

	// when queue is empty, kill timer
	if (m_vecUncheckedBlocks.getItemCount() == 0)
	{
		m_bStopSpellChecking = true;
		if(m_pBackgroundCheckTimer)
		{
			m_pBackgroundCheckTimer->stop();
			while(m_bImSpellCheckingNow == true)
			{
			}
		}
	}
}

void FL_DocLayout::setPendingWordForSpell(fl_BlockLayout *pBlock, fl_PartOfBlock* pWord)
{
	if ((pBlock == m_pPendingBlockForSpell) && 
		(pWord == m_pPendingWordForSpell))
		return;

	UT_ASSERT(!m_pPendingBlockForSpell || !pBlock);

	if (pBlock && m_pPendingBlockForSpell && m_pPendingWordForSpell)
	{
		UT_ASSERT(pWord);
	}

	// when clobbering prior POB, make sure we don't leak it
	DELETEP(m_pPendingWordForSpell);

	m_pPendingBlockForSpell = pBlock;
	m_pPendingWordForSpell = pWord;
}

bool FL_DocLayout::checkPendingWordForSpell(void)
{
	bool bUpdate = false;

	if (!m_pPendingBlockForSpell)
		return bUpdate;

	if(m_pDoc->isPieceTableChanging() == true)
	{
		return bUpdate;
	}

	// check pending word
	UT_ASSERT(m_pPendingWordForSpell);
	bUpdate = m_pPendingBlockForSpell->checkWord(m_pPendingWordForSpell);

	m_pPendingWordForSpell = NULL;	// NB: already freed by checkWord

	// not pending any more
	setPendingWordForSpell(NULL, NULL);

	return bUpdate;
}

bool FL_DocLayout::isPendingWordForSpell(void) const
{
	return (m_pPendingBlockForSpell ? true : false);
}

bool	FL_DocLayout::touchesPendingWordForSpell(fl_BlockLayout *pBlock, 
												 UT_uint32 iOffset, 
												 UT_sint32 chg) const
{
	UT_uint32 len = (chg < 0) ? -chg : 0;

	if (!m_pPendingBlockForSpell)
		return false;

	UT_ASSERT(pBlock);

	// are we in the same block?
	if (m_pPendingBlockForSpell != pBlock)
		return false;

	UT_ASSERT(m_pPendingWordForSpell);

	return m_pPendingWordForSpell->doesTouch(iOffset, len);
}
/*!
 * This method appends a DocSectionLayout onto the linked list of SectionLayout's
 * and updates the m_pFirstSection and m_pLastSection member variables 
 * accordingly.
 * The structure of this linked list is as follows.
 *    pDSL->pDSL->pDSL....pSDL->pHdrFtrSL->pHdrFtrSl->pHdrFtrSL->NULL
 *     ^                   ^
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
	UT_ASSERT(pSL);
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
 *    pDSL->pDSL->pDSL....pSDL->pHdrFtrSL->pHdrFtrSl->pHdrFtrSL->NULL
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

	fl_SectionLayout * pLSL = (fl_SectionLayout *) m_pLastSection;
	fl_SectionLayout * pnext = pLSL->getNext();
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
	UT_ASSERT(pHdrFtrSL);

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
	const char* pszAtt = NULL;

	fl_DocSectionLayout* pDocSL = m_pFirstSection;
	while (pDocSL)
	{
		pszAtt = pDocSL->getAttribute("header");
		if (
			pszAtt
			&& (0 == UT_stricmp(pszAtt, pszHdrFtrID))
			)
		{
			return pDocSL;
		}
		
		pszAtt = pDocSL->getAttribute("footer");
		if (
			pszAtt
			&& (0 == UT_stricmp(pszAtt, pszHdrFtrID))
			)
		{
			return pDocSL;
		}
		
		pDocSL = pDocSL->getNextDocSection();
	}

	return NULL;
}

/*static*/ void FL_DocLayout::_prefsListener (
	XAP_App				*pApp,
	XAP_Prefs			*pPrefs,
	UT_AlphaHashTable	* /*phChanges*/,  // not used
	void				*data
	) 
{
	bool b;
	FL_DocLayout *pDocLayout = (FL_DocLayout *)data;

	// UT_DEBUGMSG(("spell_prefsListener\n"));		
	UT_ASSERT( pApp && pPrefs && data );

	// caps/number/internet
	bool changed = false;	
	pPrefs->getPrefsValueBool( (XML_Char *)AP_PREF_KEY_SpellCheckCaps, &b );
	changed = changed || (b != pDocLayout->getSpellCheckCaps());
	pDocLayout->m_bSpellCheckCaps = b;

	pPrefs->getPrefsValueBool( (XML_Char *)AP_PREF_KEY_SpellCheckNumbers, &b );
	changed = changed || (b != pDocLayout->getSpellCheckNumbers());
	pDocLayout->m_bSpellCheckNumbers = b;

	pPrefs->getPrefsValueBool( (XML_Char *)AP_PREF_KEY_SpellCheckInternet, &b );
	changed = changed || (b != pDocLayout->getSpellCheckInternet());
	pDocLayout->m_bSpellCheckInternet = b;
	
	// auto spell
	pPrefs->getPrefsValueBool( (XML_Char *)AP_PREF_KEY_AutoSpellCheck, &b );
	pDocLayout->_toggleAutoSpell( b );
	// do this because it's recheck to document - TODO

	if ( changed )
	{
		// TODO: recheck document
		;
	}

	pPrefs->getPrefsValueBool( (XML_Char *)XAP_PREF_KEY_SmartQuotesEnable, &b );
	pDocLayout->_toggleAutoSmartQuotes( b );

	const XML_Char * pszTransparentColor = NULL;
	pPrefs->getPrefsValue((const XML_Char *)XAP_PREF_KEY_ColorForTransparent,&pszTransparentColor);
	if(UT_strcmp(pszTransparentColor,pDocLayout->m_szCurrentTransparentColor) != 0)
		pDocLayout->updateColor();
}

void FL_DocLayout::recheckIgnoredWords()
{
	// recheck the whole doc
	fl_DocSectionLayout * pSL = getFirstSection();
	while (pSL)
	{
		fl_BlockLayout* b = pSL->getFirstBlock();
		while (b)
		{
			b->recheckIgnoredWords();
			b = b->getNext();
		}
		pSL = (fl_DocSectionLayout *) pSL->getNext();
	}
}

void FL_DocLayout::_redrawUpdate(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);

	// this is a static callback method and does not have a 'this' pointer.

	FL_DocLayout * pDocLayout = (FL_DocLayout *) pTimer->getInstanceData();
	UT_ASSERT(pDocLayout);

	if (!pDocLayout->m_pView)
	{
		// Win32 timers can fire prematurely on asserts
		// (the dialog's message pump releases the timers)
		return;
	}

	fl_SectionLayout* pSL = pDocLayout->m_pFirstSection;
	while (pSL)
	{
		pSL->redrawUpdate();
		
		pSL = pSL->getNext();
	}

/*
  UT_Vector* vecToCheck = &pDocLayout->m_vecUncheckedBlocks;
  UT_ASSERT(vecToCheck);

  UT_uint32 i = vecToCheck->getItemCount();

  if (i > 0)
  {
  fl_BlockLayout *pB = (fl_BlockLayout *) vecToCheck->getFirstItem();

  if (pB != NULL)
  {
  vecToCheck->deleteNthItem(0);
  i--;

  //	note that we remove this block from queue before checking it
  //	(otherwise asserts could trigger redundant recursive calls)
  pB->checkSpelling();
  }
  }

  if (i == 0)
  {
  // timer not needed any more, so suspend it
  pDocLayout->m_pSpellCheckTimer->stop();
  }
*/
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
principals of punctuation.  It is certainly also true that my
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

7.  If a QUOTE is in isolation, it is not converted.  It is in
isolation if it is immediately preceded and followed by either a sqBREAK
or sqWHITE.  The things before and after it don't have to be of
the same type.

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
	if (UT_UCS_isalpha(thing)) return sqALPHA;
	if (UT_UCS_ispunct(thing)) return sqOTHERPUNCT;
	if (UT_UCS_isspace(thing)) return sqWHITE;

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

	{sqBREAK,      '\'', sqBREAK,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '\'', sqBREAK,     UCS_UNKPUNK},         // rule 7
	{sqBREAK,      '\'', sqWHITE,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '\'', sqWHITE,     UCS_UNKPUNK},         // rule 7

	{sqBREAK,      '`',  sqBREAK,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '`',  sqBREAK,     UCS_UNKPUNK},         // rule 7
	{sqBREAK,      '`',  sqWHITE,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '`',  sqWHITE,     UCS_UNKPUNK},         // rule 7

	{sqBREAK,      '"',  sqBREAK,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '"',  sqBREAK,     UCS_UNKPUNK},         // rule 7
	{sqBREAK,      '"',  sqWHITE,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '"',  sqWHITE,     UCS_UNKPUNK},         // rule 7

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

	setPendingSmartQuote(NULL, 0);  // avoid recursion
	UT_GrowBuf pgb(1024);
	block->getBlockBuf(&pgb);
	// this is for the benefit of the UT_DEBUGMSG and should be changed to
	// something other than '?' if '?' ever shows up as UT_isSmartQuotableCharacter()
	UT_UCSChar c = '?';
	if (pgb.getLength() > offset) c = *pgb.getPointer(offset);
	UT_DEBUGMSG(("FL_DocLayout::considerSmartQuoteCandidateAt(%x, %d)  |%c|\n", block, offset, c));

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
			fl_BlockLayout *ob = block->getPrev();
			if (ob)
			{
				fp_Run *last, *r = ob->getFirstRun();
				do
				{
					last = r;
				} while ((r = r->getNext()));  // assignment
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
			fl_BlockLayout *ob = block->getNext();
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
			// your basic emacs (save-excursion...)  :-)
			PT_DocPosition saved_pos, quotable_at;
			saved_pos = m_pView->getPoint();
			quotable_at = block->getPosition(false) + offset;

			m_pView->moveInsPtTo(quotable_at);
			// delete/insert create change records for UNDO
			m_pView->cmdCharDelete(true, 1);
			m_pView->cmdCharInsert(&replacement, 1);
			m_pView->moveInsPtTo(saved_pos);

			// Alas, Abi undo moves the insertion point, so you can't
			// just UNDO right after a smart quote pops up to force
			// an ASCII quote.  For an open quote, you could type
			// " backspace to get it (in other words, quote, space,
			// backspace.  The space will prevent the smart quote
			// promotion (no magic ... just following the rules).
			// For a close quote, type "/backspace (quote, slash, backspace)
			// for similar reasons.
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
