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



#include "fp_Line.h"
#include "fp_Column.h"
#include "fl_BlockLayout.h"
#include "fp_Run.h"
#include "gr_DrawArgs.h"
#include "fl_DocLayout.h"
#include "gr_Graphics.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"

fp_Line::fp_Line() 
{
	m_iMaxWidth = 0;
	m_iWidth = 0;
	m_iHeight = 0;
	m_iX = 0;
	m_iY = 0;
	m_iBaseX = 0;
	m_pColumn = NULL;
	m_pBlock = NULL;
}

fp_Line::~fp_Line()
{
}

void fp_Line::setMaxWidth(UT_sint32 iMaxWidth)
{
	m_iMaxWidth = iMaxWidth;
}

UT_Bool fp_Line::isEmpty(void) const
{
	return ((m_vecRuns.getItemCount()) == 0);
}

void fp_Line::setColumn(fp_Column* pColumn)
{
	m_pColumn = pColumn;
}

void fp_Line::setBlock(fl_BlockLayout* pBlock)
{
	m_pBlock = pBlock;
}

UT_sint32 fp_Line::getWidth() const
{
	return m_iWidth;
}

UT_sint32 fp_Line::getMaxWidth() const
{
	return m_iMaxWidth;
}

void fp_Line::setNext(fp_Line* p)
{
	m_pNext = p;
}

void fp_Line::setPrev(fp_Line* p)
{
	m_pPrev = p;
}

int fp_Line::countRuns() const
{
	return m_vecRuns.getItemCount();
}

fp_Run* fp_Line::getFirstRun() const
{
	fp_Run* pRun = (fp_Run*) m_vecRuns.getFirstItem();

	return pRun;
}

fp_Run* fp_Line::getLastRun() const
{
	fp_Run* pRun = (fp_Run*) m_vecRuns.getLastItem();

	return pRun;
}

UT_Bool fp_Line::removeRun(fp_Run* pRun)
{
	int numRuns = m_vecRuns.getItemCount();
	UT_Bool bAdjust = UT_FALSE;
	UT_sint32 iAdjust = 0;
	int i;

	for (i = 0; i < numRuns; i++)
	{
		fp_Run* pRun2 = (fp_Run*) m_vecRuns.getNthItem(i);

		if (bAdjust)
		{
			pRun2->setX(pRun2->getX() - iAdjust);
		}
		else if (pRun2 == pRun)
		{
			iAdjust = pRun->getWidth();
			bAdjust = UT_TRUE;
		}
	}

	for (i = 0; i < numRuns; i++)
	{
		fp_Run* pRun2 = (fp_Run*) m_vecRuns.getNthItem(i);

		if (pRun2 == pRun)
		{
			m_vecRuns.deleteNthItem(i);
			
			UT_ASSERT(((signed)m_iWidth) >= iAdjust);
			m_iWidth -= iAdjust;

			pRun->setLine(NULL);

			recalcHeight();
			
			return UT_TRUE;
		}
	}

	return UT_FALSE;
}

void fp_Line::insertRunBefore(fp_Run* pNewRun, fp_Run* pBefore)
{
	UT_ASSERT(pNewRun);
	UT_ASSERT(pBefore);

	pNewRun->setLine(this);
	pNewRun->setX(pBefore->getX());
	
	UT_sint32 iNewRunWidth = pNewRun->getWidth();

	UT_sint32 ndx = m_vecRuns.findItem(pBefore);
	UT_ASSERT(ndx >= 0);

	m_vecRuns.insertItemAt(pNewRun, ndx);

	UT_sint32 iCountRuns = m_vecRuns.getItemCount();
	for (UT_sint32 i=ndx+1; i<iCountRuns; i++)
	{
		fp_Run* pRun3 = (fp_Run*) m_vecRuns.getNthItem(i);
		UT_ASSERT(pRun3);
		UT_ASSERT((i > (ndx+1)) || (pRun3 == pBefore));

		pRun3->setX(pRun3->getX() + iNewRunWidth);
	}

	m_iWidth += iNewRunWidth;
	
	recalcHeight();
}

void fp_Line::insertRun(fp_Run* pRun)
{
	pRun->setLine(this);

	m_vecRuns.insertItemAt(pRun, 0);

	UT_sint32 count = m_vecRuns.getItemCount();
	UT_sint32 width = pRun->getWidth();

	m_iWidth += width;
	
	for (UT_sint32 i = 0; i < count; i++)
	{
		fp_Run *pRun2 = (fp_Run*) m_vecRuns.getNthItem(i);

		if (i == 0)
		{
			pRun2->setX(0);
		}
		else
		{
			pRun2->setX(pRun2->getX() + width);
		}
	}
	
	recalcHeight();
}

void fp_Line::addRun(fp_Run* pRun)
{
	pRun->setLine(this);
	pRun->setX(m_iWidth);

	m_vecRuns.addItem(pRun);
	
	m_iWidth += pRun->getWidth();

	recalcHeight();
}

void fp_Line::splitRunInLine(fp_Run* pRun1, fp_Run* pRun2)
{
	// insert run2 after run1 in the current line.
	
	pRun2->setLine(this);
	pRun2->setX(pRun1->getX() + pRun1->getWidth());

	UT_sint32 count = m_vecRuns.getItemCount();
	UT_sint32 k;
	for (k=0; k<count; k++)
	{
		fp_Run* pRun3 = (fp_Run*) m_vecRuns.getNthItem(k);
		UT_ASSERT(pRun3);
		if (pRun3 == pRun1)
		{
			m_vecRuns.insertItemAt(pRun2, k+1);
			return;
		}
	}

	// we don't update m_iWidth or recalcHeight() since we
	// assume the space in run2 came from run1.
}

UT_uint32 fp_Line::getNumChars() const
{
	UT_uint32 iCountChars = 0;
	
	int iCountRuns = m_vecRuns.getItemCount();
	for (int i=0; i<iCountRuns; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		iCountChars += pRun->getLength();
	}

	return iCountChars;
}

void fp_Line::runSizeChanged(fp_Run* pRun, UT_sint32 oldWidth, UT_sint32 newWidth)
{
	UT_ASSERT(pRun);
	
	UT_sint32 dx = newWidth - oldWidth;

	if (dx != 0)
	{
		UT_sint32 count = m_vecRuns.getItemCount();

		UT_Bool bIncr = UT_FALSE;

		/*
		  TODO is the following approach correct for a line which
		  is centered?
		*/
		
		/*
		  search thru the list of runs.  when we find the current run,
		  we need to increment all the runs that follow us
		*/
		
		for (UT_sint32 i = 0; i < count; i++)
		{
			fp_Run* pRun2 = (fp_Run*) m_vecRuns.getNthItem(i);

			if (pRun == pRun2)
			{
				bIncr = UT_TRUE;
				continue;
			}

			if (bIncr)
			{
				pRun2->setX(pRun2->getX() + dx);
			}
		}

		m_iWidth += dx;
		UT_ASSERT(((signed)m_iWidth) >= 0);
	}

	recalcHeight();

	m_pBlock->alignOneLine(this);
}

void fp_Line::remove()
{
	if (m_pNext)
	{
		m_pNext->setPrev(m_pPrev);
	}

	if (m_pPrev)
	{
		m_pPrev->setNext(m_pNext);
	}

	m_pColumn->removeLine(this);
}

void fp_Line::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	bBOL = UT_FALSE;

	if (x < 0)
	{
		x = 0;
		bBOL = UT_TRUE;
	}
	else if (x >= (UT_sint32)m_iWidth)
	{
		x = ((m_iWidth > 0) ? m_iWidth - 1 : 0);
	}

	// check all of the runs.
	int count = m_vecRuns.getItemCount();

	for (int i=0; i<count; i++)
	{
		fp_Run* pRun2 = (fp_Run*) m_vecRuns.getNthItem(i);

		UT_sint32 y2 = y - pRun2->getY() - m_iAscent + pRun2->getAscent();
		if ((x >= (UT_sint32) pRun2->getX()) && (x < (UT_sint32) (pRun2->getX() + pRun2->getWidth())))
		{
			// when hit testing runs within a line, we ignore the Y coord
//			if (((y2) >= 0) && ((y2) < (pRun2->getHeight())))
			{
				pRun2->mapXYToPosition(x - pRun2->getX(), y2, pos, bBOL, bEOL);

				return;
			}
		}
		else if (((x - pRun2->getX()) == 0) && (pRun2->getWidth() == 0))
		{
			// zero-length run
			{
				// this only happens in an empty line, right?
				UT_ASSERT(m_iWidth==0);
				UT_ASSERT(i==0);
				UT_ASSERT(count==1);

				pRun2->mapXYToPosition(x - pRun2->getX(), y2, pos, bBOL, bEOL);

				return;
			}
		}
	}

	// TODO pick the closest run
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void fp_Line::getOffsets(fp_Run* pRun, UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pColumn->getOffsets(this, my_xoff, my_yoff);
	
	xoff = my_xoff + pRun->getX();
	yoff = my_yoff + pRun->getY() + m_iAscent - pRun->getAscent();
}

void fp_Line::getScreenOffsets(fp_Run* pRun,
							   UT_sint32& xoff,
							   UT_sint32& yoff,
							   UT_sint32& width,
							   UT_sint32& height,
							   UT_Bool bLineHeight)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pColumn->getScreenOffsets(this, my_xoff, my_yoff, width, height);
	
	xoff = my_xoff + pRun->getX();

	if (bLineHeight)
	{
		yoff = my_yoff;
	}
	else
	{
		yoff = my_yoff + pRun->getY() + m_iAscent - pRun->getAscent();
	}

	width = m_iWidth;
	height = m_iHeight;
}

void fp_Line::recalcHeight()
{
	UT_sint32 count = m_vecRuns.getItemCount();
	UT_sint32 i;

	UT_sint32 iMaxAscent = 0;
	UT_sint32 iMaxDescent = 0;

    UT_sint32 iOldHeight = m_iHeight;

	for (i=0; i<count; i++)
	{
		UT_sint32 iAscent;

		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		iAscent = pRun->getAscent();
		iMaxAscent = UT_MAX(iMaxAscent, iAscent);
	}

	for (i=0; i<count; i++)
	{
		UT_sint32 iDescent;

		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		iDescent = pRun->getDescent();
		iMaxDescent = UT_MAX(iMaxDescent, iDescent);
	}

	m_iAscent = iMaxAscent;
	m_iHeight = iMaxAscent + iMaxDescent;

	{
		// adjust line height to include leading
		double dLineSpace;
		UT_Bool bExact;
		m_pBlock->getLineSpacing(dLineSpace, bExact);

		if (bExact)
			m_iHeight += (UT_sint32) dLineSpace;
		else
			m_iHeight = (UT_sint32) (m_iHeight * dLineSpace);
	}

	if ((iOldHeight != m_iHeight) && m_pColumn)
	{
		// We need to let our column know that we changed height.

		m_pColumn->lineHeightChanged(this, iOldHeight, m_iHeight);
	}
}

UT_sint32 fp_Line::getAscent(void) const
{
	return m_iAscent;
}

void fp_Line::shrink(UT_sint32 width)
{
	UT_ASSERT(((signed)m_iWidth) > width);
	m_iWidth -= width;
}

void fp_Line::clearScreen()
{
	int count = m_vecRuns.getItemCount();

	for (int i=0; i < count; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		pRun->clearScreen();
	}
}

void fp_Line::draw(DG_Graphics* pG)
{
	UT_ASSERT(m_iWidth <= m_iMaxWidth);
	
	UT_sint32 my_xoff = 0, my_yoff = 0;
	UT_sint32 width, height;
	
	m_pColumn->getScreenOffsets(this, my_xoff, my_yoff, width, height);

	dg_DrawArgs da;
	
	da.yoff = my_yoff + m_iAscent;
	da.xoff = my_xoff;
	da.x = 0;
	da.y = 0;
	da.pG = pG;
	da.width = m_iWidth;
	da.height = m_iHeight;

	// TODO: The following line means that no selection will be drawn.  Is this right?
	da.iSelPos1 = da.iSelPos2 = 0;
	
	int count = m_vecRuns.getItemCount();

	my_yoff += m_iAscent;

	for (int i=0; i < count; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		da.xoff += pRun->getX();
		da.yoff += pRun->getY();
		pRun->draw(&da);
		da.xoff -= pRun->getX();
		da.yoff -= pRun->getY();
	}
}

void fp_Line::draw(dg_DrawArgs* pDA)
{
	const UT_sint32 height = pDA->height;
	const UT_sint32 y = pDA->y;
	
	int count = m_vecRuns.getItemCount();

	UT_sint32 yOff = pDA->yoff;
	
	pDA->yoff += m_iAscent;

	for (int i=0; i<count; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);
		UT_sint32 rHeight = pRun->getHeight();

		// TODO adjust this clipping to draw lines which are partially visible
		// TODO - X clipping - for now, just clip against y
		if (((((UT_sint32)pRun->getY() + yOff) >= y) &&
		     (((UT_sint32)pRun->getY() + yOff) <= (y + height))) ||
		    ((((UT_sint32)pRun->getY() + yOff) <= y) &&
		     (((UT_sint32)pRun->getY() + yOff + rHeight) > y)))
		{
			dg_DrawArgs da = *pDA;
			da.xoff += pRun->getX();
			da.yoff += pRun->getY();
		    pRun->draw(&da);
		}
	}
}

void fp_Line::align()
{
	UT_ASSERT(m_pColumn);
	
	m_pBlock->alignOneLine(this);
}

UT_sint32 fp_Line::getBaseX(void) const
{
	return m_iBaseX;
}

void fp_Line::setX(UT_sint32 iX)
{
	if (m_iX == iX)
	{
		return;
	}

	clearScreen();
	
	m_iX = iX;
}

void fp_Line::setY(UT_sint32 iY)
{
	if (m_iY == iY)
	{
		return;
	}
	
	clearScreen();
	
	m_iY = iY;
}

void fp_Line::setBaseX(UT_sint32 iBaseX)
{
	if (m_iBaseX == iBaseX)
	{
		return;
	}
	
	m_iBaseX = iBaseX;
}

UT_Bool fp_Line::isFirstLineInBlock(void) const
{
	return m_pBlock->getFirstLine() == this;
}

UT_Bool fp_Line::isLastLineInBlock(void) const
{
	return m_pBlock->getLastLine() == this;
}

UT_sint32 fp_Line::getMarginBefore(void) const
{
	if (isFirstLineInBlock() && getBlock()->getPrev(UT_FALSE))
	{
		fp_Line* pPrevLine = getBlock()->getPrev(UT_FALSE)->getLastLine();
		UT_ASSERT(pPrevLine);
		UT_ASSERT(pPrevLine->isLastLineInBlock());
					
		UT_sint32 iBottomMargin = pPrevLine->getBlock()->getBottomMargin();
		
		UT_sint32 iNextTopMargin = getBlock()->getTopMargin();
		
		UT_sint32 iMargin = UT_MAX(iBottomMargin, iNextTopMargin);

		return iMargin;
	}

	return 0;
}

UT_sint32 fp_Line::getMarginAfter(void) const
{
	if (isLastLineInBlock() && getBlock()->getNext(UT_FALSE))
	{
		fp_Line* pNextLine = getBlock()->getNext(UT_FALSE)->getFirstLine();
		UT_ASSERT(pNextLine);
		UT_ASSERT(pNextLine->isFirstLineInBlock());
					
		UT_sint32 iBottomMargin = getBlock()->getBottomMargin();
		
		UT_sint32 iNextTopMargin = pNextLine->getBlock()->getTopMargin();
		
		UT_sint32 iMargin = UT_MAX(iBottomMargin, iNextTopMargin);

		return iMargin;
	}

	return 0;
}

UT_Bool fp_Line::recalculateFields(void)
{
	UT_Bool bResult = UT_FALSE;
	
	UT_uint32 iNumRuns = m_vecRuns.getItemCount();
	for (UT_uint32 i = 0; i < iNumRuns; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		if (pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun* pFieldRun = (fp_FieldRun*) pRun;
			UT_Bool bSizeChanged = pFieldRun->calculateValue();

			bResult = bResult || bSizeChanged;
		}
	}

	return bResult;
}

