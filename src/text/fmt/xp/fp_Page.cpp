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



#include <stdlib.h>
#include <string.h>

#include "ut_types.h"
#include "ut_misc.h"

#include "fp_Page.h"
#include "fl_DocLayout.h"
#include "fp_SectionSlice.h"
#include "gr_DrawArgs.h"
#include "fv_View.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"

fp_Page::fp_Page(FL_DocLayout* pLayout, FV_View* pView,
				 UT_uint32 iWidth, UT_uint32 iHeight,
				 UT_uint32 iLeft,
				 UT_uint32 iTop, 
				 UT_uint32 iRight,
				 UT_uint32 iBottom)
{
	UT_ASSERT(pLayout);

	m_pLayout = pLayout;
	m_pView = pView;
	
	DG_Graphics * pG = pLayout->getGraphics();
	UT_ASSERT(pG);

	m_iWidth = UT_docUnitsFromPaperUnits(pG,iWidth);
	m_iHeight = UT_docUnitsFromPaperUnits(pG,iHeight);

	m_iLeft = UT_docUnitsFromPaperUnits(pG,iLeft);
	m_iTop = UT_docUnitsFromPaperUnits(pG,iTop);
	m_iRight = UT_docUnitsFromPaperUnits(pG,iRight);
	m_iBottom = UT_docUnitsFromPaperUnits(pG,iBottom);

	m_pNext = NULL;
}

fp_SectionSliceInfo::fp_SectionSliceInfo(fp_SectionSlice* p, UT_uint32 x, UT_uint32 y)
{
	pSlice = p;
	xoff = x;
	yoff = y;
}

fp_Page::~fp_Page()
{
	UT_VECTOR_PURGEALL(fp_SectionSliceInfo, m_vecSliceInfos);
}


int fp_Page::getWidth()
{
	return m_iWidth;
}

int fp_Page::getHeight()
{
	return m_iHeight;
}

void fp_Page::getOffsets(fp_SectionSlice* pSS, void* pData, UT_sint32& xoff, UT_sint32& yoff)
{
	fp_SectionSliceInfo* pSSI = (fp_SectionSliceInfo*) pData;
	UT_ASSERT(pSS == pSSI->pSlice);

	xoff = pSSI->xoff;
	yoff = pSSI->yoff;
}

void fp_Page::getScreenOffsets(fp_SectionSlice* pSS, void* pData, UT_sint32& xoff, UT_sint32& yoff, UT_sint32& width, UT_sint32& height)
{
	fp_SectionSliceInfo* pSSI = (fp_SectionSliceInfo*) pData;
	UT_ASSERT(pSS == pSSI->pSlice);

	UT_ASSERT(m_pView);
	
	m_pView->getPageScreenOffsets(this, xoff, yoff, width, height);

	xoff += pSSI->xoff;
	yoff += pSSI->yoff;
}

fp_Page* fp_Page::getNext()
{
	return m_pNext;
}

void fp_Page::setNext(fp_Page* p)
{
	m_pNext = p;
}

FL_DocLayout* fp_Page::getLayout()
{
	return m_pLayout;
}

void fp_Page::draw(dg_DrawArgs* pDA)
{
	/*
		draw each slice on the page
	*/
	int count = m_vecSliceInfos.getItemCount();
	fp_SectionSlice* p;

	for (int i=0; i<count; i++)
	{
		fp_SectionSliceInfo* pci = (fp_SectionSliceInfo*) m_vecSliceInfos.getNthItem(i);
		p = pci->pSlice;

		dg_DrawArgs da = *pDA;
		da.xoff += pci->xoff;
		da.yoff += pci->yoff;
		p->draw(&da);
	}
}

void fp_Page::dump()
{
	int count = m_vecSliceInfos.getItemCount();
	fp_SectionSlice* p;

	for (int i=0; i<count; i++)
	{
		fp_SectionSliceInfo* pci = (fp_SectionSliceInfo*) m_vecSliceInfos.getNthItem(i);
		p = pci->pSlice;

		UT_DEBUGMSG(("fp_Page::dump(0x%x) - fp_SectionSlice 0x%x\n", this, p));
		p->dump();
	}
}

UT_Bool fp_Page::requestSpace(fl_SectionLayout*, fp_SectionSlice** ppsi)
{
	UT_sint32 iHeight = 0;
	int count = m_vecSliceInfos.getItemCount();
	for (int i=0; i<count; i++)
	{
		fp_SectionSliceInfo* pSSI = (fp_SectionSliceInfo*) m_vecSliceInfos.getNthItem(i);
		iHeight += pSSI->pSlice->getHeight();
	}

	UT_sint32 yBottom = m_iTop + iHeight;
	UT_sint32 iAvailable = m_iHeight - m_iBottom - yBottom;

	if (iAvailable > 0)
	{
		fp_SectionSlice* pSS = new fp_SectionSlice(m_iWidth - (m_iRight + m_iLeft), iAvailable);
		fp_SectionSliceInfo* pSSI = new fp_SectionSliceInfo(pSS, m_iLeft, yBottom);
		pSS->setPage(this, pSSI);
		m_vecSliceInfos.addItem(pSSI);
		*ppsi = pSS;

		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

void fp_Page::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	int count = m_vecSliceInfos.getItemCount();
	fp_SectionSlice* p;
	UT_uint32 iMinDist = 0xffffffff;
	fp_SectionSliceInfo* pMinDist = NULL;
	
	for (int i=0; i<count; i++)
	{
		fp_SectionSliceInfo* pci = (fp_SectionSliceInfo*) m_vecSliceInfos.getNthItem(i);
		p = pci->pSlice;

		// when hit testing for SectionSlices on a page, we ignore X coords
//		if (((x - pci->xoff) > 0) && ((x - pci->xoff) < p->getWidth()))
		{
			if (((y - (UT_sint32)pci->yoff) > 0) && ((y - (UT_sint32)pci->yoff) < (UT_sint32)p->getHeight()))
			{
				p->mapXYToPosition(x - pci->xoff, y - pci->yoff, pos, bBOL, bEOL);
				return;
			}
		}

		UT_uint32 iDistTop;
		UT_uint32 iDistBottom;
		UT_uint32 iDist;

		iDistTop = UT_ABS((y - (UT_sint32) pci->yoff));
		iDistBottom = UT_ABS((y - (UT_sint32) (pci->yoff + p->getHeight())));
			
		iDist = UT_MIN(iDistTop, iDistBottom);
		if (iDist < iMinDist)
		{
			pMinDist = pci;
			iMinDist = iDist;
		}
	}

	UT_ASSERT(pMinDist);
	pMinDist->pSlice->mapXYToPosition(x - pMinDist->xoff, y - pMinDist->yoff, pos, bBOL, bEOL);
}

void fp_Page::setView(FV_View* pView)
{
	m_pView = pView;
}


