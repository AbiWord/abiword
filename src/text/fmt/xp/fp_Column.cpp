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

	m_bNeedsLayout = UT_FALSE;
}

fp_Column::~fp_Column()
{
	/*
	  Note that we do not delete the lines here.  They are owned by
	  the block, not the column.
	*/
}

fl_SectionLayout* fp_Column::getSectionLayout(void) const
{
	return m_pSectionLayout;
}

void fp_Column::setPage(fp_Page* pPage)
{
	m_pPage = pPage;
}

UT_uint32 fp_Column::getHeight(void) const
{
	return m_iHeight;
}

void fp_Column::setWidth(UT_uint32 iWidth)
{
	if (iWidth == m_iWidth)
	{
		return;
	}
	
	m_iWidth = iWidth;

	// TODO we really need to force a re-line-break operation on every block herein

//	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void fp_Column::setHeight(UT_uint32 iHeight)
{
	if (iHeight == m_iHeight)
	{
		return;
	}
	
	m_iHeight = iHeight;

	// what do we do here?  does it ever happen?
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void fp_Column::setMaxHeight(UT_uint32 iMaxHeight)
{
	UT_ASSERT(iMaxHeight > 0);

	if (iMaxHeight == m_iMaxHeight)
	{
		return;
	}
	
	m_iMaxHeight = iMaxHeight;
	m_bNeedsLayout = UT_TRUE;
	updateLayout();
}

fp_Page* fp_Column::getPage(void) const
{
	return m_pPage;
}

void fp_Column::setLeader(fp_Column* p)
{
	m_pLeader = p;
}

void fp_Column::setFollower(fp_Column* p)
{
	m_pNextFollower = p;
}

fp_Column* fp_Column::getLeader(void) const
{
	return m_pLeader;
}

fp_Column* fp_Column::getFollower(void) const
{
	return m_pNextFollower;
}

void fp_Column::setNext(fp_Column*p)
{
	m_pNext = p;
}

void fp_Column::setPrev(fp_Column*p)
{
	m_pPrev = p;
}

fp_Column* fp_Column::getNext(void) const
{
	return m_pNext;
}

fp_Column* fp_Column::getPrev(void) const
{
	return m_pPrev;
}

void fp_Column::getOffsets(fp_Line* pLine, UT_sint32& xoff, UT_sint32& yoff)
{
	xoff = getX() + pLine->getX();
	yoff = getY() + pLine->getY();
}

void fp_Column::getScreenOffsets(fp_Line* pLine,
									 UT_sint32& xoff, UT_sint32& yoff,
									 UT_sint32& width, UT_sint32& height)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pPage->getScreenOffsets(this, my_xoff, my_yoff, width, height);
	
	xoff = my_xoff + pLine->getX();
	yoff = my_yoff + pLine->getY();
}

void fp_Column::removeLine(fp_Line* pLine)
{
	UT_sint32 ndx = m_vecLines.findItem(pLine);
	UT_ASSERT(ndx >= 0);

	m_vecLines.deleteNthItem(ndx);

	// don't delete the line here, it's deleted elsewhere.

	m_bNeedsLayout = UT_TRUE;
}

UT_Bool fp_Column::insertLineAfter(fp_Line*	pNewLine, fp_Line*	pAfterLine)
{
	if (pAfterLine)
	{
		UT_ASSERT(m_iMaxHeight > 0);
		
		if ((pAfterLine->getY() + pAfterLine->getHeight() + pNewLine->getHeight()) > m_iMaxHeight)
		{
			return UT_FALSE;
		}
		
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

		pNewLine->setY(pAfterLine->getY() + pAfterLine->getHeight());
	}
	else
	{
		m_vecLines.insertItemAt(pNewLine, 0);
		pNewLine->setY(0);
	}

	pNewLine->setMaxWidth(getWidth());
	pNewLine->setColumn(this);
	
	m_bNeedsLayout = UT_TRUE;

	return UT_TRUE;
}

UT_sint32 fp_Column::_getMarginBeforeLine(fp_Line* pLine)
{
	UT_uint32 i = m_vecLines.findItem(pLine);
	UT_ASSERT(i >= 0);

	if (pLine->isFirstLineInBlock() && pLine->getBlock()->getPrev(UT_FALSE))
	{
		fp_Line* pPrevLine = pLine->getBlock()->getPrev(UT_FALSE)->getLastLine();
		UT_ASSERT(pPrevLine);
		UT_ASSERT(pPrevLine->isLastLineInBlock());
		if (i > 0)
		{
			UT_ASSERT(pPrevLine == ((fp_Line*) m_vecLines.getNthItem(i - 1)));
		}
					
		UT_sint32 iBottomMargin =
			m_pG->convertDimension(pPrevLine->getBlock()->getProperty("margin-bottom"));
		
		UT_sint32 iNextTopMargin =
			m_pG->convertDimension(pLine->getBlock()->getProperty("margin-top"));
		
		UT_sint32 iMargin = UT_MAX(iBottomMargin, iNextTopMargin);

		return iMargin;
	}

	return 0;
}

UT_Bool fp_Column::isEmpty(void) const
{
	return (m_vecLines.getItemCount() == 0);
}

void fp_Column::updateLayout(void)
{
	if (m_bNeedsLayout)
	{
		UT_Bool bBumpedSomething = UT_FALSE;
		UT_uint32 iCurY = 0;
		int count = m_vecLines.getItemCount();

		UT_ASSERT(m_iMaxHeight > 0);
		
		for (int i = 0; i<count; i++)
		{
			fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

			if (i > 0)
			{
				UT_sint32 iMargin = _getMarginBeforeLine(pLine);
				iCurY += iMargin;
			}
			
			if ((iCurY + pLine->getHeight()) > m_iMaxHeight)
			{
				/*
				  We're out of room.  This line, as well as any others after
				  it, must move to the next column.
				*/

				int iFirstToBump = i;

				fp_Column* pNextColumn = getNext();
				if (!pNextColumn)
				{
					pNextColumn = m_pSectionLayout->getNewColumn();
				}

				int iBump = count-1;
				while (iBump >= iFirstToBump)
				{
					fp_Line* pBumpLine = (fp_Line*) m_vecLines.getNthItem(iBump);
					m_vecLines.deleteNthItem(iBump);

					pNextColumn->insertLineAfter(pBumpLine, NULL);
					
					iBump--;
				}

				bBumpedSomething = UT_TRUE;
				pNextColumn->updateLayout();
				
				break;
			}
		
			pLine->setY(iCurY);
			iCurY += pLine->getHeight();
		}

		if (!bBumpedSomething && (iCurY < m_iMaxHeight))
		{
			fp_Column* pNextColumn = getNext();
				
			if (pNextColumn)
			{
				UT_ASSERT(pNextColumn->getWidth() == getWidth());
				
				UT_Bool bDone = UT_FALSE;
				UT_Bool bSlurpedSomething = UT_FALSE;

				while (!bDone)
				{
					fp_Line* pLine = pNextColumn->getFirstLine();
					if (pLine && ((iCurY + pLine->getHeight()) < m_iMaxHeight))
					{
						pNextColumn->removeLine(pLine);
						m_vecLines.addItem(pLine);

						UT_sint32 iMargin = _getMarginBeforeLine(pLine);
						iCurY += iMargin;
						
						pLine->setY(iCurY);
						pLine->setMaxWidth(getWidth());
						pLine->setColumn(this);
						iCurY += pLine->getHeight();

						bSlurpedSomething = UT_TRUE;
					}
					else
					{
						bDone = UT_TRUE;
					}
				}

				if (bSlurpedSomething)
				{
					pNextColumn->updateLayout();
				}
			}
		}
		
		if (iCurY != m_iHeight)
		{
			m_iHeight = iCurY;

			UT_ASSERT(m_pPage);
			
			m_pPage->columnHeightChanged(this);
		}

		m_bNeedsLayout = UT_FALSE;
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
}

UT_Bool fp_Column::containsPoint(UT_sint32 x, UT_sint32 y)
{
	if ((x < 0) || (x >= (UT_sint32)m_iWidth))
	{
		return UT_FALSE;
	}
	if ((y < 0) || (y >= (UT_sint32)m_iHeight))
	{
		return UT_FALSE;
	}

	return UT_TRUE;
}

void fp_Column::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	int count = m_vecLines.getItemCount();
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
	
	if (x < 0)
	{
		dx = -x;
	}
	else if (x > (UT_sint32)m_iWidth)
	{
		dx = x - m_iWidth;
	}
	else
	{
		dx = 0;
	}

	if (y < 0)
	{
		dy = -y;
	}
	else if (y > (UT_sint32)m_iHeight)
	{
		dy = y - m_iHeight;
	}
	else
	{
		dy = 0;
	}

	UT_uint32 dist = (UT_uint32) (sqrt((dx * dx) + (dy * dy)));

	return dist;
}

void fp_Column::lineHeightChanged(fp_Line* pLine, DG_Graphics* pG, UT_sint32 iOldHeight, UT_sint32 iNewHeight)
{
	m_bNeedsLayout = UT_TRUE;
}

UT_uint32 fp_Column::getWidth(void) const
{
	return m_iWidth;
}

UT_sint32 fp_Column::getX(void) const
{
	return m_iX;
}

UT_sint32 fp_Column::getY(void) const
{
	return m_iY;
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
