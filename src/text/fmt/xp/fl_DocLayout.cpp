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
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_timer.h"

// TODO why do we define these multiply, in different files? --EWS
#define DELETEP(p)	do { if (p) delete p; } while (0)
#define FREEP(p)	do { if (p) free(p); } while (0)

FL_DocLayout::FL_DocLayout(PD_Document* doc, GR_Graphics* pG) : m_hashFontCache(19)
{
	m_pDoc = doc;
	m_pG = pG;
	m_pView = NULL;
	m_pSpellCheckTimer = NULL;
	m_pPendingWord = NULL;
	
	// TODO the following (both the new() and the addListener() cause
	// TODO malloc's to occur.  we are currently inside a constructor
	// TODO and cannot report failure.
	
	m_pDocListener = new fl_DocListener(doc, this);
	doc->addListener(static_cast<PL_Listener *>(m_pDocListener),&m_lid);
}

FL_DocLayout::~FL_DocLayout()
{
	if (m_pDoc)
		m_pDoc->removeListener(m_lid);

	DELETEP(m_pDocListener);

	DELETEP(m_pSpellCheckTimer);
	DELETEP(m_pPendingWord);

	UT_VECTOR_PURGEALL(fp_Page *, m_vecPages);
	UT_VECTOR_PURGEALL(fl_SectionLayout *, m_vecSectionLayouts);

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
}

FV_View * FL_DocLayout::getView(void) const
{
	return m_pView;
}

PD_Document* FL_DocLayout::getDocument() const
{
	return m_pDoc;
}

GR_Graphics* FL_DocLayout::getGraphics()
{
	return m_pG;
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
		iHeight += fl_PAGEVIEW_MARGIN_X * 2;
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
		iWidth += fl_PAGEVIEW_MARGIN_Y * 2;
	}

	return iWidth;
}

GR_Font* FL_DocLayout::findFont(const PP_AttrProp * pSpanAP,
								const PP_AttrProp * pBlockAP,
								const PP_AttrProp * pSectionAP)
{
	GR_Font* pFont;

	const char* pszFamily	= PP_evalProperty("font-family",pSpanAP,pBlockAP,pSectionAP);
	const char* pszStyle	= PP_evalProperty("font-style",pSpanAP,pBlockAP,pSectionAP);
	const char* pszVariant	= PP_evalProperty("font-variant",pSpanAP,pBlockAP,pSectionAP);
	const char* pszWeight	= PP_evalProperty("font-weight",pSpanAP,pBlockAP,pSectionAP);
	const char* pszStretch	= PP_evalProperty("font-stretch",pSpanAP,pBlockAP,pSectionAP);
	const char* pszSize		= PP_evalProperty("font-size",pSpanAP,pBlockAP,pSectionAP);
	
	// NOTE: we currently favor a readable hash key to make debugging easier
	// TODO: speed things up with a smaller key (the three AP pointers?) 
	char key[500];
	sprintf(key,"%s;%s;%s;%s;%s;%s",pszFamily, pszStyle, pszVariant, pszWeight, pszStretch, pszSize);
	
	UT_HashTable::UT_HashEntry* pEntry = m_hashFontCache.findEntry(key);
	if (!pEntry)
	{
		// TODO -- note that we currently assume font-family to be a single name,
		// TODO -- not a list.  This is broken.

		pFont = m_pG->findFont(pszFamily, pszStyle, pszVariant, pszWeight, pszStretch, pszSize);
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
	UT_ASSERT(m_vecPages.getItemCount() > 0);

	return (fp_Page*) m_vecPages.getNthItem(0);
}

fp_Page* FL_DocLayout::getLastPage()
{
	UT_ASSERT(m_vecPages.getItemCount() > 0);

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

fp_Page* FL_DocLayout::addNewPage()
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
	
	fp_Page*		pPage = new fp_Page(this, m_pView, 850, 1100);
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

fl_SectionLayout* FL_DocLayout::getPrevSection(fl_SectionLayout* pSL) const
{
	fl_SectionLayout* pPrev = NULL;

	UT_sint32 ndx = m_vecSectionLayouts.findItem(pSL);
	UT_ASSERT(ndx >= 0);

	if (ndx > 0)
	{
		pPrev = (fl_SectionLayout*) m_vecSectionLayouts.getNthItem(ndx-1);
		UT_ASSERT(pPrev);
	}

	return pPrev;
}

fl_SectionLayout* FL_DocLayout::getNextSection(fl_SectionLayout* pSL) const
{
	fl_SectionLayout* pNext = NULL;

	UT_sint32 ndx = m_vecSectionLayouts.findItem(pSL);
	UT_ASSERT(ndx >= 0);

	if (m_vecSectionLayouts.getItemCount() > (UT_uint32)(ndx+1))
	{
		pNext = (fl_SectionLayout*) m_vecSectionLayouts.getNthItem(ndx+1);
		UT_ASSERT(pNext);
	}

	return pNext;
}

void FL_DocLayout::deleteEmptyColumnsAndPages(void)
{
	int i;
	
	int countSections = m_vecSectionLayouts.getItemCount();
	for (i=0; i<countSections; i++)
	{
		fl_SectionLayout* pSL = (fl_SectionLayout*) m_vecSectionLayouts.getNthItem(i);

		pSL->deleteEmptyColumns();
	}

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

int FL_DocLayout::formatAll()
{
	UT_ASSERT(m_pDoc);
	UT_DEBUGMSG(("BEGIN Formatting document: 0x%x\n", this));

	UT_Bool bStillGoing = UT_TRUE;
	int countSections = m_vecSectionLayouts.getItemCount();

	while (bStillGoing)
	{
		bStillGoing = UT_FALSE;
		
		for (int i=0; i<countSections; i++)
		{
			fl_SectionLayout* pSL = (fl_SectionLayout*) m_vecSectionLayouts.getNthItem(i);

			bStillGoing = pSL->format() || bStillGoing;
		}
	}

	UT_DEBUGMSG(("END Formatting document: 0x%x\n", this));

	return 0;
}

#ifdef FMT_TEST
void FL_DocLayout::__dump(FILE * fp) const
{
	int count = m_vecPages.getItemCount();

	fprintf(fp,"FL_DocLayout::dump(%p) contains %ld pages.\n", this, m_vecPages.getItemCount());

	for (int i=0; i<count; i++)
	{
		fp_Page* p = (fp_Page*) m_vecPages.getNthItem(i);

		p->__dump(fp);
	}

	// TODO dump the section layouts
}
#endif

void FL_DocLayout::_spellCheck(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);

	// this is a static callback method and does not have a 'this' pointer.

	FL_DocLayout * pDocLayout = (FL_DocLayout *) pTimer->getInstanceData();
	UT_ASSERT(pDocLayout);

	UT_Vector* vecToCheck = &pDocLayout->m_vecUncheckedBlocks;
	UT_ASSERT(vecToCheck);

	UT_uint32 i = vecToCheck->getItemCount();

	if (i > 0)
	{
		fl_BlockLayout *pB = (fl_BlockLayout *) vecToCheck->getFirstItem();

		if (pB != NULL)
		{
			pB->checkSpelling();
			vecToCheck->deleteNthItem(0);
			i--;
		}
	}

	// TODO: might be safer to return BOOL and let timer kill itself
	// ALT: just call pDocLayout->dequeueBlock(pB)
#if 0
	if (i == 0)
	{
		// timer not needed any more, so clear it
		DELETEP(pDocLayout->m_pSpellCheckTimer);
		pDocLayout->m_pSpellCheckTimer = NULL;
	}
#endif
}

#define SPELL_CHECK_MSECS 100

void FL_DocLayout::queueBlockForSpell(fl_BlockLayout *pBlock, UT_Bool bHead)
{
	/*
		This routine queues up blocks for timer-driven spell checking.  
		By default, this is a FIFO queue, but it can be explicitly 
		reprioritized by setting bHead == UT_TRUE.  
	*/

	if (!m_pSpellCheckTimer)
	{
		m_pSpellCheckTimer = UT_Timer::static_constructor(_spellCheck, this);

		if (m_pSpellCheckTimer)
			m_pSpellCheckTimer->set(SPELL_CHECK_MSECS);
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
		m_vecUncheckedBlocks.deleteNthItem(i);
	else
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	// when queue is empty, kill timer
	if (m_vecUncheckedBlocks.getItemCount() == 0)
	{
		DELETEP(m_pSpellCheckTimer);
		m_pSpellCheckTimer = NULL;
	}
}
