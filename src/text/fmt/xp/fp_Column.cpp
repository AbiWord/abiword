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
#include "fl_SectionLayout.h"
#include "gr_DrawArgs.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"

fp_Container::fp_Container(UT_uint32 iType, fl_SectionLayout* pSectionLayout)
:	m_iMaxHeight(0),
	m_iMaxHeightLayoutUnits(0),
	m_iHeight(0),
	m_iHeightLayoutUnits(0)
{
	m_iType = iType;
	m_pSectionLayout = pSectionLayout;
	
	m_pG = m_pSectionLayout->getDocLayout()->getGraphics();

	m_iWidth = 0;
	m_iWidthLayoutUnits = 0;

	m_pPage = NULL;
	m_iMaxHeight = 0;
	m_iX = 0;
	m_iY = 0;
}

fp_Container::~fp_Container()
{
	/*
	  Note that we do not delete the lines here.  They are owned by
	  the block, not the column.
	*/
}

void fp_Container::setPage(fp_Page* pPage)
{
	m_pPage = pPage;
}

void fp_Container::setWidth(UT_sint32 iWidth)
{
	if (iWidth == m_iWidth)
	{
		return;
	}
	
	m_iWidth = iWidth;

	// TODO we really need to force a re-line-break operation on every block herein

//	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void fp_Container::setWidthInLayoutUnits(UT_sint32 iWidth)
{
	m_iWidthLayoutUnits = iWidth;

}

void fp_Container::setHeight(UT_sint32 iHeight)
{
	if (iHeight == m_iHeight)
	{
		return;
	}
	
	m_iHeight = iHeight;

	// what do we do here?  does it ever happen?
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void fp_Container::setMaxHeight(UT_sint32 iMaxHeight)
{
	UT_ASSERT(iMaxHeight > 0);

	if (iMaxHeight == m_iMaxHeight)
	{
		return;
	}
	
	m_iMaxHeight = iMaxHeight;
}

void fp_Container::setMaxHeightInLayoutUnits(UT_sint32 iMaxHeight)
{
	UT_ASSERT(iMaxHeight > 0);

	if (iMaxHeight == m_iMaxHeightLayoutUnits)
	{
		return;
	}
	
	m_iMaxHeightLayoutUnits = iMaxHeight;
}

void fp_Container::getOffsets(fp_Line* pLine, UT_sint32& xoff, UT_sint32& yoff)
{
	xoff = getX() + pLine->getX();
	yoff = getY() + pLine->getY();
}

void fp_Container::getScreenOffsets(fp_Line* pLine,
									 UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pPage->getScreenOffsets(this, my_xoff, my_yoff);
	
	xoff = my_xoff + pLine->getX();
	yoff = my_yoff + pLine->getY();
}

void fp_Container::removeLine(fp_Line* pLine)
{
	UT_sint32 ndx = m_vecLines.findItem(pLine);
	UT_ASSERT(ndx >= 0);

	m_vecLines.deleteNthItem(ndx);

	// don't delete the line here, it's deleted elsewhere.
}

UT_Bool fp_Container::insertLine(fp_Line* pNewLine)
{
	m_vecLines.insertItemAt(pNewLine, 0);
	pNewLine->setContainer(this);
	pNewLine->recalcMaxWidth();

	return UT_TRUE;
}

UT_Bool fp_Container::addLine(fp_Line* pNewLine)
{
	m_vecLines.addItem(pNewLine);
	pNewLine->setContainer(this);
	pNewLine->recalcMaxWidth();

	return UT_TRUE;
}

UT_Bool fp_Container::insertLineAfter(fp_Line*	pNewLine, fp_Line*	pAfterLine)
{
	UT_ASSERT(pAfterLine);
	UT_ASSERT(pNewLine);
	
	UT_sint32 count = m_vecLines.getItemCount();
	UT_sint32 ndx = m_vecLines.findItem(pAfterLine);
	UT_ASSERT( (count > 0) || (ndx == -1) );
	
	/*
	  TODO this routine should not be allowing pAfterLine to be NULL.
	  Right now, we've fixed the symptom, but we really should fix
	  the problem.
	*/
	UT_ASSERT(ndx >= 0);

	if ( (ndx+1) == count )				// append after last line in vector
		m_vecLines.addItem(pNewLine);
	else if (ndx >= 0)					// append after this item within the vector
		m_vecLines.insertItemAt(pNewLine, ndx+1);
	else
	{
		// TODO remove this....
		m_vecLines.insertItemAt(pNewLine, 0);
	}

	pNewLine->setContainer(this);
	pNewLine->recalcMaxWidth();

	return UT_TRUE;
}

UT_Bool fp_Container::isEmpty(void) const
{
	return (m_vecLines.getItemCount() == 0);
}

void fp_Container::clearScreen(void)
{
	int count = m_vecLines.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

		pLine->clearScreen();
	}
}

void fp_Container::draw(dg_DrawArgs* pDA)
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

void fp_Container::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
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

				UT_ASSERT(bEOL == UT_TRUE || bEOL == UT_FALSE);
				UT_ASSERT(bBOL == UT_TRUE || bBOL == UT_FALSE);
				
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
					}
					else
					{
						pLine->mapXYToPosition(x - pLine->getX(), y - pLine->getY(), pos, bBOL, bEOL);
					}

					UT_ASSERT(bEOL == UT_TRUE || bEOL == UT_FALSE);
					UT_ASSERT(bBOL == UT_TRUE || bBOL == UT_FALSE);

					return;
				}
			}
		}

		// TODO it might be better to move these special cases outside the loop
		if ((i == 0) && (y < pLine->getY()))
		{
			pLine->mapXYToPosition(x - pLine->getX(), y - pLine->getY(), pos, bBOL, bEOL);

			UT_ASSERT(bEOL == UT_TRUE || bEOL == UT_FALSE);
			UT_ASSERT(bBOL == UT_TRUE || bBOL == UT_FALSE);
			
			return;
		}
		
		if ((i == (count-1)) && (y >= (pLine->getY() + (UT_sint32)pLine->getHeight())))
		{
			pLine->mapXYToPosition(x - pLine->getX(), y - pLine->getY(), pos, bBOL, bEOL);

			UT_ASSERT(bEOL == UT_TRUE || bEOL == UT_FALSE);
			UT_ASSERT(bBOL == UT_TRUE || bBOL == UT_FALSE);
			
			return;
		}
	}

	// TODO pick the closest line
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

UT_uint32	fp_Container::distanceFromPoint(UT_sint32 x, UT_sint32 y)
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
		dy = m_iY - y;
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

	UT_ASSERT(dist > 0);
	
	return dist;
}

void fp_Container::setX(UT_sint32 iX)
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

void fp_Container::setY(UT_sint32 iY)
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

fp_Line* fp_Container::getFirstLine(void) const
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

fp_Line* fp_Container::getLastLine(void) const
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

fp_Column::fp_Column(fl_SectionLayout* pSectionLayout) : fp_Container(FP_CONTAINER_COLUMN, pSectionLayout)
{
	m_pNext = NULL;
	m_pPrev = NULL;

	m_pLeader = NULL;
	m_pNextFollower = NULL;
}

fp_Column::~fp_Column()
{

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

void fp_Column::layout(void)
{
	UT_sint32 iY = 0;
	UT_sint32 iYLayoutUnits = 0;

	UT_uint32 iCountLines = m_vecLines.getItemCount();
	for (UT_uint32 i=0; i < iCountLines; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);
		
		UT_sint32 iLineHeight = pLine->getHeight();
		UT_sint32 iLineHeightLayoutUnits = pLine->getHeightInLayoutUnits();
//		UT_sint32 iLineMarginBefore = (i != 0) ? pLine->getMarginBefore() : 0;
		UT_sint32 iLineMarginAfter = pLine->getMarginAfter();
		UT_sint32 iLineMarginAfterLayoutUnits = pLine->getMarginAfterInLayoutUnits();

//		iY += iLineMarginBefore;
		pLine->setY(iY);
		pLine->setYInLayoutUnits(iYLayoutUnits);
		iY += iLineHeight;
		iY += iLineMarginAfter;
		iYLayoutUnits += iLineHeightLayoutUnits;
		iYLayoutUnits += iLineMarginAfterLayoutUnits;
	}

	UT_sint32 iNewHeight = iY;
	if (m_iHeight == iNewHeight)
	{
		return;
	}

	m_iHeight = iY;
	m_iHeightLayoutUnits = iYLayoutUnits;
	
	m_pPage->columnHeightChanged(this);
}

void fp_Column::bumpLines(fp_Line* pLastLineToKeep)
{
	UT_sint32 ndx = m_vecLines.findItem(pLastLineToKeep);
	UT_ASSERT(ndx >= 0);
	UT_sint32 iCount = m_vecLines.getItemCount();
	UT_sint32 i;

	fp_Column* pNextColumn = getNext();
	UT_ASSERT(pNextColumn);

	if (pNextColumn->isEmpty())
	{
		for (i=ndx + 1; i<iCount; i++)
		{
			fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

			pNextColumn->addLine(pLine);
		}
	}
	else
	{
		for (i=iCount - 1; i>=(ndx+1); i--)
		{
			fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);
		
			pNextColumn->insertLine(pLine);
		}
	}
	
	for (i=iCount - 1; i > ndx; i--)
	{
		m_vecLines.deleteNthItem(i);
	}
}

fl_DocSectionLayout* fp_Column::getDocSectionLayout(void) const
{
	UT_ASSERT(m_pSectionLayout->getType() == FL_SECTION_DOC);

	return (fl_DocSectionLayout*) m_pSectionLayout;
}

fp_HdrFtrContainer::fp_HdrFtrContainer(UT_sint32 iX,
									   UT_sint32 iY,
									   UT_sint32 iWidth,
									   UT_sint32 iHeight,
									   UT_sint32 iWidthLayout,
									   UT_sint32 iHeightLayout,
									   fl_SectionLayout* pSectionLayout) : fp_Container(FP_CONTAINER_HDRFTR, pSectionLayout)
{
	m_iX = iX;
	m_iY = iY;
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iWidthLayoutUnits = iWidthLayout;
	m_iHeightLayoutUnits = iHeightLayout;
}

fp_HdrFtrContainer::~fp_HdrFtrContainer()
{

}

void fp_HdrFtrContainer::layout(void)
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

	// note that the height of a HdrFtr container never changes.
	// TODO deal with overflow of this container.  clip.
}

fl_HdrFtrSectionLayout* fp_HdrFtrContainer::getHdrFtrSectionLayout(void) const
{
	UT_ASSERT(m_pSectionLayout->getType() == FL_SECTION_HDRFTR);

	return (fl_HdrFtrSectionLayout*) m_pSectionLayout;
}

