/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 



#include "ut_types.h"

#include "fp_SectionSlice.h"
#include "fp_Column.h"
#include "fp_Page.h"
#include "dg_DrawArgs.h"

#include "ut_assert.h"

fp_SectionSlice::fp_SectionSlice(UT_sint32 iWidth, UT_sint32 iHeight)
{
	m_pPage = NULL;
	m_pPageData = NULL;
	m_iWidth = iWidth;
	m_iHeight = iHeight;
}

fp_SectionSlice::~fp_SectionSlice()
{
	UT_VECTOR_PURGEALL(fp_ColumnInfo, m_vecColumnInfos);
}

void fp_SectionSlice::setPage(fp_Page* pPage, void* p)
{
	m_pPage = pPage;
	m_pPageData = p;
}

fp_Page* fp_SectionSlice::getPage() const
{
	return m_pPage;
}

UT_sint32 fp_SectionSlice::getWidth()
{
	return m_iWidth;
}

UT_sint32 fp_SectionSlice::getHeight()
{
	return m_iHeight;
}

fp_Column* fp_SectionSlice::getFirstColumn()
{
	fp_ColumnInfo* pCI = (fp_ColumnInfo*) m_vecColumnInfos.getNthItem(0);
	return pCI->pColumn;
}

void fp_SectionSlice::addColumn(fp_Column* pCol, UT_sint32 xoff, UT_sint32 yoff)
{
	fp_ColumnInfo* pCI = new fp_ColumnInfo(pCol, xoff, yoff);
	pCol->setSectionSlice(this, pCI);
	m_vecColumnInfos.addItem(pCI);
}

void fp_SectionSlice::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, UT_Bool& bEOL)
{
	int count = m_vecColumnInfos.getItemCount();
	UT_uint32 iMinDist = 0xffffffff;
	fp_ColumnInfo* pMinDist = NULL;
	for (int i=0; i<count; i++)
	{
		fp_ColumnInfo* pCI = (fp_ColumnInfo*) m_vecColumnInfos.getNthItem(i);

		if (pCI->pColumn->containsPoint(x - pCI->xoff, y - pCI->yoff))
		{
			pCI->pColumn->mapXYToPosition(x - pCI->xoff, y - pCI->yoff, pos, bEOL);
			return;
		}
		UT_uint32 iDist = pCI->pColumn->distanceFromPoint(x - pCI->xoff, y - pCI->yoff);
		if (iDist < iMinDist)
		{
			iMinDist = iDist;
			pMinDist = pCI;
		}
	}

	UT_ASSERT(pMinDist);

	pMinDist->pColumn->mapXYToPosition(x - pMinDist->xoff, y - pMinDist->yoff, pos, bEOL);
}

void fp_SectionSlice::getOffsets(fp_Column* pCol, void* p, UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pPage->getOffsets(this, m_pPageData, my_xoff, my_yoff);
	
	fp_ColumnInfo* pCI = (fp_ColumnInfo*) p;
	UT_ASSERT(pCI->pColumn == pCol);

	xoff = my_xoff + pCI->xoff;
	yoff = my_yoff + pCI->yoff;
}

void fp_SectionSlice::getScreenOffsets(fp_Column* pCol, void* p,
									   UT_sint32& xoff, UT_sint32& yoff,
									   UT_sint32& width, UT_sint32& height)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pPage->getScreenOffsets(this, m_pPageData, my_xoff, my_yoff,
							  width, height);
	
	fp_ColumnInfo* pCI = (fp_ColumnInfo*) p;
	UT_ASSERT(pCI->pColumn == pCol);

	xoff = my_xoff + pCI->xoff;
	yoff = my_yoff + pCI->yoff;
}

void fp_SectionSlice::draw(dg_DrawArgs* pDA)
{
	int count = m_vecColumnInfos.getItemCount();
	for (int i=0; i<count; i++)
	{
		fp_ColumnInfo* pCI = (fp_ColumnInfo*) m_vecColumnInfos.getNthItem(i);
		dg_DrawArgs da = *pDA;
		da.xoff += pCI->xoff;
		da.yoff += pCI->yoff;
		
		pCI->pColumn->draw(&da);
	}
}

void fp_SectionSlice::dump()
{
}
