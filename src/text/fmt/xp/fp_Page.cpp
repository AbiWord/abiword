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
#include "fl_SectionLayout.h"
#include "fp_Column.h"
#include "gr_Graphics.h"
#include "gr_DrawArgs.h"
#include "fv_View.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"

fp_Page::fp_Page(FL_DocLayout* pLayout, FV_View* pView,
				 UT_uint32 iWidth, UT_uint32 iHeight)
{
	UT_ASSERT(pLayout);

	m_pLayout = pLayout;
	m_pView = pView;
	
	DG_Graphics * pG = pLayout->getGraphics();
	UT_ASSERT(pG);

	m_iWidth = UT_docUnitsFromPaperUnits(pG,iWidth);
	m_iHeight = UT_docUnitsFromPaperUnits(pG,iHeight);

	m_pNext = NULL;
	m_pPrev = NULL;

	m_bNeedsRedraw = UT_TRUE;
}

fp_Page::~fp_Page()
{
}

UT_Bool fp_Page::isEmpty(void) const
{
	return m_vecColumnLeaders.getItemCount() == 0;
}

UT_sint32 fp_Page::getWidth(void) const
{
	return m_iWidth;
}

UT_sint32 fp_Page::getHeight(void) const
{
	return m_iHeight;
}

void fp_Page::getScreenOffsets(fp_Column* pCol, UT_sint32& xoff, UT_sint32& yoff, UT_sint32& width, UT_sint32& height)
{
	UT_ASSERT(m_pView);
	
	m_pView->getPageScreenOffsets(this, xoff, yoff, width, height);

	xoff += pCol->getX();
	yoff += pCol->getY();
}

fp_Page* fp_Page::getNext(void) const
{
	return m_pNext;
}

fp_Page* fp_Page::getPrev(void) const
{
	return m_pPrev;
}

void fp_Page::setNext(fp_Page* p)
{
	m_pNext = p;
}

void fp_Page::setPrev(fp_Page* p)
{
	m_pPrev = p;
}

FL_DocLayout* fp_Page::getDocLayout()
{
	return m_pLayout;
}

void fp_Page::draw(dg_DrawArgs* pDA)
{
	// draw each column on the page
	int count = m_vecColumnLeaders.getItemCount();

	for (int i=0; i<count; i++)
	{
		fp_Column* pCol = (fp_Column*) m_vecColumnLeaders.getNthItem(i);

		fp_Column* pTmpCol = pCol;
		while (pTmpCol)
		{
			dg_DrawArgs da = *pDA;
			da.xoff += pTmpCol->getX();
			da.yoff += pTmpCol->getY();
			pTmpCol->draw(&da);

			pTmpCol = pTmpCol->getFollower();
		}
	}

	m_bNeedsRedraw = UT_FALSE;
}

UT_Bool fp_Page::needsRedraw(void) const
{
	return m_bNeedsRedraw;
}

#ifdef FMT_TEST
void fp_Page::__dump(FILE * fp) const
{
}
#endif

UT_uint32 fp_Page::countColumnLeaders(void) const
{
	return m_vecColumnLeaders.getItemCount();
}

fp_Column* fp_Page::getNthColumnLeader(UT_sint32 n) const
{
	return (fp_Column*) m_vecColumnLeaders.getNthItem(n);
}

void fp_Page::_reformat(void)
{
	int count = countColumnLeaders();
	if (count <= 0)
	{
		return;
	}

	fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
	fl_SectionLayout* pFirstSectionLayout = pFirstColumnLeader->getSectionLayout();
	UT_sint32 iLeftMargin = pFirstSectionLayout->getLeftMargin();
	UT_sint32 iRightMargin = pFirstSectionLayout->getRightMargin();
	UT_sint32 iTopMargin = pFirstSectionLayout->getTopMargin();
	UT_sint32 iBottomMargin = pFirstSectionLayout->getBottomMargin();
	
	UT_uint32 iY = iTopMargin;
	
	int i;
	for (i=0; i<count; i++)
	{
		fp_Column* pLeader = getNthColumnLeader(i);
		fl_SectionLayout* pSL = pLeader->getSectionLayout();
		UT_uint32 iNumColumns = pSL->getNumColumns();
		UT_uint32 iColumnGap = pSL->getColumnGap();

		UT_uint32 iSpace = m_iWidth - iLeftMargin - iRightMargin;
		UT_uint32 iColWidth = (iSpace - ((iNumColumns - 1) * iColumnGap)) / iNumColumns;
		
		UT_uint32 iX = iLeftMargin;
		
		fp_Column* pTmpCol = pLeader;
		UT_sint32 iMostHeight = 0;
		while (pTmpCol)
		{
			pTmpCol->setX(iX);
			pTmpCol->setY(iY);
			pTmpCol->setMaxHeight(m_iHeight - iBottomMargin - iY);
			pTmpCol->setWidth(iColWidth);
			iX += (iColWidth + iColumnGap);

			iMostHeight = UT_MAX(iMostHeight, pTmpCol->getHeight());

			pTmpCol = pTmpCol->getFollower();
		}

		iY += iMostHeight;

		iY += m_pLayout->getGraphics()->convertDimension("0.25in");	// TODO

		if (iY >= (m_iHeight - iBottomMargin))
		{
			break;
		}
	}

	while (i < count)
	{
		fp_Column* pLeader = getNthColumnLeader(i);
		fl_SectionLayout* pSL = pLeader->getSectionLayout();

		// TODO move this column to the next page.
		i++;
	}
}

#if 0
UT_Bool fp_Page::addColumnLeader(fp_Column* pLeader)
{
	m_vecColumnLeaders.addItem(pLeader);

	_reformat();

	// TODO this shouldn't ALWAYS return true, right?
	
	return UT_TRUE;
}
#endif

void fp_Page::removeColumnLeader(fp_Column* pLeader)
{
	UT_sint32 ndx = m_vecColumnLeaders.findItem(pLeader);
	UT_ASSERT(ndx >= 0);

	m_vecColumnLeaders.deleteNthItem(ndx);
		
	fp_Column* pTmpCol = pLeader;
	while (pTmpCol)
	{
		pTmpCol->setPage(NULL);
		
		pTmpCol = pTmpCol->getFollower();
	}

	_reformat();
}

UT_Bool fp_Page::insertColumnLeader(fp_Column* pLeader, fp_Column* pAfter)
{
	/*
	  TODO we should check to see if there is ANY hope of this
	  column leader actually fitting on the page.
	*/
	
	if (pAfter)
	{
		UT_sint32 ndx = m_vecColumnLeaders.findItem(pAfter);
		UT_ASSERT(ndx >= 0);

		m_vecColumnLeaders.insertItemAt(pLeader, ndx+1);
	}
	else
	{
		m_vecColumnLeaders.insertItemAt(pLeader, 0);
	}

	fp_Column* pTmpCol = pLeader;
	while (pTmpCol)
	{
		pTmpCol->setPage(this);
		
		pTmpCol = pTmpCol->getFollower();
	}

	_reformat();

	return UT_TRUE;
}

void fp_Page::columnHeightChanged(fp_Column* pCol)
{
	fp_Column* pLeader = pCol->getLeader();
	
	UT_sint32 ndx = m_vecColumnLeaders.findItem(pLeader);
	UT_ASSERT(ndx >= 0);

	_reformat();
}

void fp_Page::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	int count = m_vecColumnLeaders.getItemCount();
	UT_uint32 iMinDist = 0xffffffff;
	fp_Column* pMinDist = NULL;
	for (int i=0; i<count; i++)
	{
		fp_Column* pLeader = (fp_Column*) m_vecColumnLeaders.getNthItem(i);

		fp_Column* pColumn = pLeader;
		while (pColumn)
		{
			if (pColumn->containsPoint(x - pColumn->getX(), y - pColumn->getY()))
			{
				pColumn->mapXYToPosition(x - pColumn->getX(), y - pColumn->getY(), pos, bBOL, bEOL);
				return;
			}
			UT_uint32 iDist = pColumn->distanceFromPoint(x - pColumn->getX(), y - pColumn->getY());
			if (iDist < iMinDist)
			{
				iMinDist = iDist;
				pMinDist = pColumn;
			}
			pColumn = pColumn->getFollower();
		}
	}

	UT_ASSERT(pMinDist);

	pMinDist->mapXYToPosition(x - pMinDist->getX(), y - pMinDist->getY(), pos, bBOL, bEOL);
}

void fp_Page::setView(FV_View* pView)
{
	m_pView = pView;
}


