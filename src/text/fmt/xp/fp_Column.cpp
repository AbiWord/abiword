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
#include <math.h>
#include <string.h>

#include "fp_Column.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_BlockLayout.h"
#include "pp_AttrProp.h"
#include "gr_Graphics.h"
#include "gr_DrawArgs.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"

fp_Column::fp_Column(fl_SectionLayout* pSectionLayout)
{
	m_pNext = NULL;
	m_pPrev = NULL;

	m_pLeader = NULL;
	m_pNextFollower = NULL;
	
	m_pSectionLayout = pSectionLayout;
	m_pG = m_pSectionLayout->getDocLayout()->getGraphics();

	m_iWidth = 0;
	m_iHeight = 0;

	m_pPage = NULL;
	m_iMaxHeight = 0;
	m_iX = 0;
	m_iY = 0;
}

fp_Column::~fp_Column()
{
	/*
	  Note that we do not delete the lines here.  They are owned by
	  the block, not the column.
	*/
}

void fp_Column::setPage(fp_Page* pPage)
{
	m_pPage = pPage;
}

void fp_Column::setWidth(UT_sint32 iWidth)
{
	if (iWidth == m_iWidth)
	{
		return;
	}
	
	m_iWidth = iWidth;

	// TODO we really need to force a re-line-break operation on every block herein

//	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void fp_Column::setHeight(UT_sint32 iHeight)
{
	if (iHeight == m_iHeight)
	{
		return;
	}
	
	m_iHeight = iHeight;

	// what do we do here?  does it ever happen?
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void fp_Column::setMaxHeight(UT_sint32 iMaxHeight)
{
	UT_ASSERT(iMaxHeight > 0);

	if (iMaxHeight == m_iMaxHeight)
	{
		return;
	}
	
	m_iMaxHeight = iMaxHeight;
}

void fp_Column::setLeader(fp_Column* p)
{
	m_pLeader = p;
}

void fp_Column::setFollower(fp_Column* p)
{
	m_pNextFollower = p;
}

void fp_Column::setNext(fp_Column*p)
{
	m_pNext = p;
}

void fp_Column::setPrev(fp_Column*p)
{
	m_pPrev = p;
}

void fp_Column::getOffsets(fp_Line* pLine, UT_sint32& xoff, UT_sint32& yoff)
{
	xoff = getX() + pLine->getX();
	yoff = getY() + pLine->getY();
}

void fp_Column::getScreenOffsets(fp_Line* pLine,
									 UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pPage->getScreenOffsets(this, my_xoff, my_yoff);
	
	xoff = my_xoff + pLine->getX();
	yoff = my_yoff + pLine->getY();
}

void fp_Column::removeLine(fp_Line* pLine)
{
	UT_sint32 ndx = m_vecLines.findItem(pLine);
	UT_ASSERT(ndx >= 0);

	m_vecLines.deleteNthItem(ndx);

	// don't delete the line here, it's deleted elsewhere.
}

UT_Bool fp_Column::insertLine(fp_Line* pNewLine)
{
	m_vecLines.insertItemAt(pNewLine, 0);
		
	pNewLine->setColumn(this);

	pNewLine->recalcMaxWidth();

	return UT_TRUE;
}

UT_Bool fp_Column::addLine(fp_Line* pNewLine)
{
	m_vecLines.addItem(pNewLine);
		
	pNewLine->setColumn(this);

	pNewLine->recalcMaxWidth();

	return UT_TRUE;
}

void fp_Column::layout(void)
{
	UT_sint32 iY = 0;

	UT_uint32 iCountLines = m_vecLines.getItemCount();
	for (UT_uint32 i=0; i < iCountLines; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);
		
		UT_sint32 iLineHeight = pLine->getHeight();
//		UT_sint32 iLineMarginBefore = (i != 0) ? pLine->getMarginBefore() : 0;
		UT_sint32 iLineMarginAfter = pLine->getMarginAfter();

//		iY += iLineMarginBefore;
		pLine->setY(iY);
		iY += iLineHeight;
		iY += iLineMarginAfter;
	}

	UT_uint32 iNewHeight = iY;
	if (m_iHeight == iNewHeight)
	{
		return;
	}

	m_iHeight = iY;
	
	m_pPage->columnHeightChanged(this);
}

UT_Bool fp_Column::insertLineAfter(fp_Line*	pNewLine, fp_Line*	pAfterLine)
{
	UT_uint32 ndx = m_vecLines.findItem(pAfterLine);
	UT_ASSERT(ndx >= 0);

	if (ndx == (m_vecLines.getItemCount() - 1))
	{
		m_vecLines.addItem(pNewLine);
	}
	else
	{
		m_vecLines.insertItemAt(pNewLine, ndx+1);
	}

	pNewLine->setColumn(this);

	pNewLine->recalcMaxWidth();

	return UT_TRUE;
}

UT_Bool fp_Column::isEmpty(void) const
{
	return (m_vecLines.getItemCount() == 0);
}

void fp_Column::clearScreen(void)
{
	int count = m_vecLines.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

		pLine->clearScreen();
	}
}

void fp_Column::draw(dg_DrawArgs* pDA)
{
	int count = m_vecLines.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

		dg_DrawArgs da = *pDA;
		da.xoff += pLine->getX();
		da.yoff += pLine->getY();
		pLine->draw(&da);
	}

#if 0
	m_pG->drawLine(pDA->xoff, pDA->yoff, pDA->xoff + m_iWidth, pDA->yoff);
	m_pG->drawLine(pDA->xoff + m_iWidth, pDA->yoff, pDA->xoff + m_iWidth, pDA->yoff + m_iMaxHeight);
	m_pG->drawLine(pDA->xoff + m_iWidth, pDA->yoff + m_iMaxHeight, pDA->xoff, pDA->yoff + m_iMaxHeight);
	m_pG->drawLine(pDA->xoff, pDA->yoff + m_iMaxHeight, pDA->xoff, pDA->yoff);
#endif	
}

void fp_Column::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	int count = m_vecLines.getItemCount();

	UT_ASSERT(count > 0);
	
	for (int i = 0; i<count; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

		// when hit testing lines within a column, we ignore the X coord.
//		if (((x - pLine->getX()) >= 0) && ((x - pLine->getX()) < (pLine->getWidth())))
		{
			if (((y - pLine->getY()) >= 0) && ((y - pLine->getY()) < (UT_sint32)(pLine->getHeight())))
			{
				pLine->mapXYToPosition(x - pLine->getX(), y - pLine->getY(), pos, bBOL, bEOL);
				return;
			}
		}

	    /*
		  Be careful with the comparisons of signed vs. unsigned
		  below.  (bug 172 resulted from that kind of problem.)
		*/
		
		if ((i + 1) < count)
		{
			if (y >= (pLine->getY() + (UT_sint32) pLine->getHeight()))
			{
				fp_Line* pLine2 = (fp_Line*) m_vecLines.getNthItem(i+1);
				if (y < (pLine2->getY()))
				{
					/*
					  The point is between these two lines.  Pick one.
					*/

					if ((pLine2->getY() - y) < (y - (pLine->getY() + (UT_sint32) pLine->getHeight())))
					{
						pLine2->mapXYToPosition(x - pLine2->getX(), y - pLine2->getY(), pos, bBOL, bEOL);
						return;
					}
					else
					{
						pLine->mapXYToPosition(x - pLine->getX(), y - pLine->getY(), pos, bBOL, bEOL);
						return;
					}
				}
			}
		}

		// TODO it might be better to move these special cases outside the loop
		if ((i == 0) && (y < pLine->getY()))
		{
			pLine->mapXYToPosition(x - pLine->getX(), y - pLine->getY(), pos, bBOL, bEOL);
			return;
		}
		
		if ((i == (count-1)) && (y >= (pLine->getY() + (UT_sint32)pLine->getHeight())))
		{
			pLine->mapXYToPosition(x - pLine->getX(), y - pLine->getY(), pos, bBOL, bEOL);
			return;
		}
	}

	// TODO pick the closest line
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

UT_uint32	fp_Column::distanceFromPoint(UT_sint32 x, UT_sint32 y)
{
	UT_sint32 dx;
	UT_sint32 dy;
	
	if (x < m_iX)
	{
		dx = m_iX - x;
	}
	else if (x > (m_iX + m_iWidth - 1))
	{
		dx = x - (m_iX + m_iWidth - 1);
	}
	else
	{
		dx = 0;
	}

	if (y < m_iY)
	{
		dy = y - m_iY;
	}
	else if (y > (m_iY + m_iHeight - 1))
	{
		dy = y - (m_iY + m_iHeight - 1);
	}
	else
	{
		dy = 0;
	}

	if (dx == 0)
	{
		return dy;
	}

	if (dy == 0)
	{
		return dx;
	}

	UT_uint32 dist = (UT_uint32) (sqrt((dx * dx) + (dy * dy)));

	return dist;
}

void fp_Column::setX(UT_sint32 iX)
{
	if (iX == m_iX)
	{
		return;
	}
	
	int count = m_vecLines.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

		pLine->clearScreen();
	}
	
	m_iX = iX;
}

void fp_Column::setY(UT_sint32 iY)
{
	if (iY == m_iY)
	{
		return;
	}

	int count = m_vecLines.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

		pLine->clearScreen();
	}
	
	m_iY = iY;
}

fp_Line* fp_Column::getFirstLine(void) const
{
	if (m_vecLines.getItemCount() > 0)
	{
		return (fp_Line*) m_vecLines.getNthItem(0);
	}
	else
	{
		return NULL;
	}
}

fp_Line* fp_Column::getLastLine(void) const
{
	UT_uint32 iCount = m_vecLines.getItemCount();
	
	if (iCount > 0)
	{
		return (fp_Line*) m_vecLines.getNthItem(iCount - 1);
	}
	else
	{
		return NULL;
	}
}

