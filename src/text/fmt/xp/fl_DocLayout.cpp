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
#include "fl_ColumnSetLayout.h"
#include "fl_ColumnLayout.h"
#include "fl_BlockLayout.h"
#include "fp_Page.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "pp_Property.h"
#include "gr_Graphics.h"
#include "xav_Listener.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"

FL_DocLayout::FL_DocLayout(PD_Document* doc, DG_Graphics* pG) : m_hashFontCache(19)
{
	m_pDoc = doc;
	m_pG = pG;
	m_pView = NULL;

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

	if (m_pDocListener)
		delete m_pDocListener;

	UT_VECTOR_PURGEALL(fp_Page *, m_vecPages);
	UT_VECTOR_PURGEALL(fl_SectionLayout *, m_vecSectionLayouts);

	UT_HASH_PURGEDATA(DG_Font *, m_hashFontCache);
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

PD_Document* FL_DocLayout::getDocument() const
{
	return m_pDoc;
}

DG_Graphics* FL_DocLayout::getGraphics()
{
	return m_pG;
}

UT_uint32 FL_DocLayout::getHeight()
{
	UT_uint32 iHeight = 0;
	int count = m_vecPages.getItemCount();

	for (int i=0; i<count; i++)
	{
		fp_Page* p = (fp_Page*) m_vecPages.getNthItem(i);

		iHeight += p->getHeight();
	}

	return iHeight;
}

UT_uint32 FL_DocLayout::getWidth()
{
	UT_uint32 iWidth = 0;
	int count = m_vecPages.getItemCount();

	for (int i=0; i<count; i++)
	{
		fp_Page* p = (fp_Page*) m_vecPages.getNthItem(i);

		iWidth += p->getWidth();
	}

	return iWidth;
}

DG_Font* FL_DocLayout::findFont(const PP_AttrProp * pSpanAP,
								const PP_AttrProp * pBlockAP,
								const PP_AttrProp * pSectionAP)
{
	DG_Font* pFont;

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
		pFont = (DG_Font*) pEntry->pData;
	}

	return pFont;
}

int FL_DocLayout::countPages()
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
	
	// TODO pass the margins.  which ones?
	// TODO get these constants from the document.
	fp_Page*		pPage = new fp_Page(this, m_pView, 850, 1100, 100, 100, 100, 100);
	if (pLastPage)
	{
		UT_ASSERT(pLastPage->getNext() == NULL);

		pLastPage->setNext(pPage);
	}
	m_vecPages.addItem(pPage);

	// let the view know that we created a new page,
	// so that it can update the scroll bar ranges
	// and whatever else it needs to do.

	if (m_pView)
		m_pView->notifyListeners(AV_CHG_PAGECOUNT);
	
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
		case PTX_ColumnSet:
		case PTX_Column:
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

int FL_DocLayout::reformat()
{
	UT_Bool bStillGoing = UT_TRUE;
	int countSections = m_vecSectionLayouts.getItemCount();

	while (bStillGoing)
	{
		bStillGoing = UT_FALSE;
		
		for (int i=0; i<countSections; i++)
		{
			fl_SectionLayout* pSL = (fl_SectionLayout*) m_vecSectionLayouts.getNthItem(i);

			bStillGoing = pSL->reformat() || bStillGoing;
		}
	}

	return 0;
}

#ifdef FMT_TEST
void FL_DocLayout::__dump(FILE * fp) const
{
	int count = m_vecPages.getItemCount();

	fprintf(fp,"FL_DocLayout::dump(0x%08lx) contains %ld pages.\n", (UT_uint32)this, m_vecPages.getItemCount());

	for (int i=0; i<count; i++)
	{
		fp_Page* p = (fp_Page*) m_vecPages.getNthItem(i);

		p->__dump(fp);
	}

	// TODO dump the section layouts
}
#endif
