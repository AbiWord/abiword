 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/


#include "ut_types.h"

#include "fp_SectionSlice.h"
#include "fp_Column.h"
#include "fp_Page.h"
#include "dg_DrawArgs.h"

#include "ut_assert.h"

FP_SectionSlice::FP_SectionSlice(UT_sint32 iWidth, UT_sint32 iHeight)
{
	m_pPage = NULL;
	m_pPageData = NULL;
	m_iWidth = iWidth;
	m_iHeight = iHeight;
}

FP_SectionSlice::~FP_SectionSlice()
{
	UT_VECTOR_PURGEALL(fp_ColumnInfo, m_vecColumnInfos);
}

void FP_SectionSlice::setPage(FP_Page* pPage, void* p)
{
	m_pPage = pPage;
	m_pPageData = p;
}

FP_Page* FP_SectionSlice::getPage() const
{
	return m_pPage;
}

UT_sint32 FP_SectionSlice::getWidth()
{
	return m_iWidth;
}

UT_sint32 FP_SectionSlice::getHeight()
{
	return m_iHeight;
}

FP_Column* FP_SectionSlice::getFirstColumn()
{
	fp_ColumnInfo* pCI = (fp_ColumnInfo*) m_vecColumnInfos.getNthItem(0);
	return pCI->pColumn;
}

void FP_SectionSlice::addColumn(FP_Column* pCol, UT_sint32 xoff, UT_sint32 yoff)
{
	fp_ColumnInfo* pCI = new fp_ColumnInfo(pCol, xoff, yoff);
	pCol->setSectionSlice(this, pCI);
	m_vecColumnInfos.addItem(pCI);
}

void FP_SectionSlice::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, UT_Bool& bRight)
{
	int count = m_vecColumnInfos.getItemCount();
	UT_uint32 iMinDist = 0xffffffff;
	fp_ColumnInfo* pMinDist = NULL;
	for (int i=0; i<count; i++)
	{
		fp_ColumnInfo* pCI = (fp_ColumnInfo*) m_vecColumnInfos.getNthItem(i);

		if (pCI->pColumn->containsPoint(x - pCI->xoff, y - pCI->yoff))
		{
			pCI->pColumn->mapXYToPosition(x - pCI->xoff, y - pCI->yoff, pos, bRight);
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

	pMinDist->pColumn->mapXYToPosition(x - pMinDist->xoff, y - pMinDist->yoff, pos, bRight);
}

void FP_SectionSlice::getOffsets(FP_Column* pCol, void* p, UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pPage->getOffsets(this, m_pPageData, my_xoff, my_yoff);
	
	fp_ColumnInfo* pCI = (fp_ColumnInfo*) p;
	UT_ASSERT(pCI->pColumn == pCol);

	xoff = my_xoff + pCI->xoff;
	yoff = my_yoff + pCI->yoff;
}

void FP_SectionSlice::getScreenOffsets(FP_Column* pCol, void* p,
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

void FP_SectionSlice::draw(dg_DrawArgs* pDA)
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

void FP_SectionSlice::dump()
{
}
