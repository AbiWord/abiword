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
#include "fp_Page.h"
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


#define REDRAW_UPDATE_MSECS	500

FL_DocLayout::FL_DocLayout(PD_Document* doc, GR_Graphics* pG)
:	m_hashFontCache(19),
	m_pageSize(_getDefaultPageSize())
{
	m_pDoc = doc;
	m_pG = pG;
	m_pView = NULL;
	m_pSpellCheckTimer = NULL;
	m_pPendingBlock = NULL;
	m_pPendingWord = NULL;
	m_pFirstSection = NULL;
	m_pLastSection = NULL;
	m_bAutoSpellCheck = UT_FALSE;
	m_bSpellCheckCaps = UT_TRUE;
	m_bSpellCheckNumbers = UT_TRUE;
	m_bSpellCheckInternet = UT_TRUE;
	m_pPrefs = NULL;

	m_pRedrawUpdateTimer = UT_Timer::static_constructor(_redrawUpdate, this, m_pG);
	if (m_pRedrawUpdateTimer)
	{
		m_pRedrawUpdateTimer->set(REDRAW_UPDATE_MSECS);
		m_pRedrawUpdateTimer->start();
	}

	// TODO the following (both the new() and the addListener() cause
	// TODO malloc's to occur.  we are currently inside a constructor
	// TODO and are not allowed to report failure.
	
	m_pDocListener = new fl_DocListener(doc, this);
	doc->addListener(static_cast<PL_Listener *>(m_pDocListener),&m_lid);
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

	if (m_pSpellCheckTimer)
	{
		m_pSpellCheckTimer->stop();
	}
	
	DELETEP(m_pSpellCheckTimer);
	DELETEP(m_pPendingWord);

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
	GR_Font* pFont;

	const char* pszFamily	= PP_evalProperty("font-family",pSpanAP,pBlockAP,pSectionAP, m_pDoc, UT_TRUE);
	const char* pszStyle	= PP_evalProperty("font-style",pSpanAP,pBlockAP,pSectionAP, m_pDoc, UT_TRUE);
	const char* pszVariant	= PP_evalProperty("font-variant",pSpanAP,pBlockAP,pSectionAP, m_pDoc, UT_TRUE);
	const char* pszWeight	= PP_evalProperty("font-weight",pSpanAP,pBlockAP,pSectionAP, m_pDoc, UT_TRUE);
	const char* pszStretch	= PP_evalProperty("font-stretch",pSpanAP,pBlockAP,pSectionAP, m_pDoc, UT_TRUE);
	const char* pszSize		= PP_evalProperty("font-size",pSpanAP,pBlockAP,pSectionAP, m_pDoc, UT_TRUE);
	const char* pszPosition = PP_evalProperty("text-position",pSpanAP,pBlockAP,pSectionAP, m_pDoc, UT_TRUE);

	// for superscripts and subscripts, we'll automatically shrink the font size
	if ((0 == UT_stricmp(pszPosition, "superscript")) ||
		(0 == UT_stricmp(pszPosition, "subscript")))
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
			m_pG->setLayoutResolutionMode(UT_TRUE);
			}
		pFont = m_pG->findFont(pszFamily, pszStyle, pszVariant, pszWeight, pszStretch, pszSize);
		m_pG->setLayoutResolutionMode(UT_FALSE);
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
									m_pageSize,
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

fl_BlockLayout* FL_DocLayout::findBlockAtPosition(PT_DocPosition pos)
{
	fl_BlockLayout* pBL = NULL;
	PL_StruxFmtHandle sfh;

	if (m_pDoc->getStruxOfTypeFromPosition(m_lid, pos, PTX_Block, &sfh))
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		switch (pL->getType())
		{
		case PTX_Block:
			pBL = static_cast<fl_BlockLayout *>(pL);
			break;
				
		case PTX_Section:
		default:
			UT_ASSERT((0));
		}
	}
	else
	{
		UT_ASSERT((0));
	}


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

#ifdef FMT_TEST
void FL_DocLayout::__dump(FILE * fp) const
{
	int count = m_vecPages.getItemCount();

	fprintf(fp,"FL_DocLayout::dump(0x%p) contains %d pages.\n", this, m_vecPages.getItemCount());

	for (int i=0; i<count; i++)
	{
		fp_Page* p = (fp_Page*) m_vecPages.getNthItem(i);
		p->__dump(fp);
	}

	fprintf(fp,"FL_DocLayout::dump(0x%p) sections:\n",this);
	for (fl_SectionLayout * psl=getFirstSection(); (psl); psl=psl->getNext())
	{
		psl->__dump(fp);
	}
}
#endif

#define SPELL_CHECK_MSECS 100

void FL_DocLayout::_toggleAutoSpell(UT_Bool bSpell)
{
	m_bAutoSpellCheck = bSpell;

	UT_DEBUGMSG(("FL_DocLayout::_toggleAutoSpell (%s)\n", bSpell ? "UT_TRUE" : "UT_FALSE" ));

	if (bSpell)
	{
		// make sure the timer is started
		if (!m_pSpellCheckTimer)
		{
			m_pSpellCheckTimer = UT_Timer::static_constructor(_spellCheck, this, m_pG);
			if (m_pSpellCheckTimer)
				m_pSpellCheckTimer->set(SPELL_CHECK_MSECS);
		}
		else
		{
			m_pSpellCheckTimer->start();
		}

		// recheck the whole doc
		fl_DocSectionLayout * pSL = getFirstSection();
		while (pSL)
		{
			fl_BlockLayout* b = pSL->getFirstBlock();
			while (b)
			{
				// TODO: just check and remove matching squiggles
				// for now, destructively recheck the whole thing
				queueBlockForSpell(b, UT_FALSE);
				b = b->getNext();
			}
			pSL = (fl_DocSectionLayout *) pSL->getNext();
		}
	}
	else
	{
		// make sure the timer is stopped
		if (m_pSpellCheckTimer)
			m_pSpellCheckTimer->stop();	
	
		// remove the squiggles, too
		fl_DocSectionLayout * pSL = getFirstSection();
		while (pSL)
		{
			fl_BlockLayout* b = pSL->getFirstBlock();
			while (b)
			{
				b->_purgeSquiggles();
				b = b->getNext();
			}
			pSL = (fl_DocSectionLayout *) pSL->getNext();
		}
	}
}

void FL_DocLayout::_spellCheck(UT_Timer * pTimer)
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
}

void FL_DocLayout::queueBlockForSpell(fl_BlockLayout *pBlock, UT_Bool bHead)
{
	/*
		This routine queues up blocks for timer-driven spell checking.  
		By default, this is a FIFO queue, but it can be explicitly 
		reprioritized by setting bHead == UT_TRUE.  
	*/

	if (m_bAutoSpellCheck)
	{
		if (!m_pSpellCheckTimer)
		{
			m_pSpellCheckTimer = UT_Timer::static_constructor(_spellCheck, this, m_pG);
			if (m_pSpellCheckTimer)
				m_pSpellCheckTimer->set(SPELL_CHECK_MSECS);
		}
		else
		{
			m_pSpellCheckTimer->start();
		}
	}

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

void FL_DocLayout::dequeueBlock(fl_BlockLayout *pBlock)
{
	UT_sint32 i = m_vecUncheckedBlocks.findItem(pBlock);

	if (i>=0)
	{
		m_vecUncheckedBlocks.deleteNthItem(i);
	}

	// when queue is empty, kill timer
	if (m_vecUncheckedBlocks.getItemCount() == 0)
	{
		m_pSpellCheckTimer->stop();
	}
}

void FL_DocLayout::setPendingWord(fl_BlockLayout *pBlock, fl_PartOfBlock* pWord)
{
	if ((pBlock == m_pPendingBlock) && 
		(pWord == m_pPendingWord))
		return;

	UT_ASSERT(!m_pPendingBlock || !pBlock);

	if (pBlock && m_pPendingBlock && m_pPendingWord)
	{
		UT_ASSERT(pWord);
	}

	// when clobbering prior POB, make sure we don't leak it
	FREEP(m_pPendingWord);

	m_pPendingBlock = pBlock;
	m_pPendingWord = pWord;
}

UT_Bool FL_DocLayout::checkPendingWord(void)
{
	UT_Bool bUpdate = UT_FALSE;

	if (!m_pPendingBlock)
		return bUpdate;

	// check pending word
	UT_ASSERT(m_pPendingWord);
	bUpdate = m_pPendingBlock->checkWord(m_pPendingWord);

	m_pPendingWord = NULL;	// NB: already freed by checkWord

	// not pending any more
	setPendingWord(NULL, NULL);

	return bUpdate;
}

UT_Bool FL_DocLayout::isPendingWord(void) const
{
	return (m_pPendingBlock ? UT_TRUE : UT_FALSE);
}

UT_Bool	FL_DocLayout::touchesPendingWord(fl_BlockLayout *pBlock, 
										 UT_uint32 iOffset, 
										 UT_sint32 chg) const
{
	UT_uint32 len = (chg < 0) ? -chg : 0;

	if (!m_pPendingBlock)
		return UT_FALSE;

	UT_ASSERT(pBlock);

	// are we in the same block?
	if (m_pPendingBlock != pBlock)
		return UT_FALSE;

	UT_ASSERT(m_pPendingWord);

	return m_pPendingWord->doesTouch(iOffset, len);
}

void FL_DocLayout::addSection(fl_DocSectionLayout* pSL)
{
	if (m_pLastSection)
	{
		UT_ASSERT(m_pFirstSection);
		UT_ASSERT(m_pLastSection->getNext() == NULL);

		pSL->setNext(NULL);
		m_pLastSection->setNext(pSL);
		pSL->setPrev(m_pLastSection);
		m_pLastSection = pSL;
	}
	else
	{
		pSL->setPrev(NULL);
		pSL->setNext(NULL);
		m_pFirstSection = pSL;
		m_pLastSection = m_pFirstSection;
	}
}

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
	UT_Bool b;
	FL_DocLayout *pDocLayout = (FL_DocLayout *)data;

	// UT_DEBUGMSG(("spell_prefsListener\n"));		
	UT_ASSERT( pApp && pPrefs && data );

	// caps/number/internet
	UT_Bool changed = UT_FALSE;	
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

fp_PageSize FL_DocLayout::_getDefaultPageSize()
{
	// TODO The following page dimensions should NOT be hard-coded.
	// TODO return PageSize initialized by prefs.

	return fp_PageSize(fp_PageSize::Letter);
}

