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

UT_sint32 fp_Column::getHeight(void) const
{
	return m_iHeight;
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
									 UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pPage->getScreenOffsets(this, my_xoff, my_yoff);
	
	xoff = my_xoff + pLine->getX();
	yoff = my_yoff + pLine->getY();
}

void fp_Column::setNeedsLayoutUpdate(void)
{
	m_bNeedsLayout = UT_TRUE;
}

void fp_Column::removeLine(fp_Line* pLine)
{
	UT_sint32 ndx = m_vecLines.findItem(pLine);
	UT_ASSERT(ndx >= 0);

	m_vecLines.deleteNthItem(ndx);

	// don't delete the line here, it's deleted elsewhere.

	m_bNeedsLayout = UT_TRUE;
}

UT_sint32 fp_Column::getSpaceAtBottom(void) const
{
	fp_Line* pLine = getLastLine();
	if (pLine)
	{
		return m_iMaxHeight - (pLine->getY() + pLine->getHeight());
	}
	else
	{
		return m_iMaxHeight;
	}
}

UT_Bool fp_Column::insertLineAfter(fp_Line*	pNewLine, fp_Line*	pAfterLine, UT_sint32 iHeight)
{
	if (pNewLine->getHeight() > 0)
	{
		iHeight = pNewLine->getHeight();
	}

	fl_BlockLayout* pBL = pNewLine->getBlock();
	
	if (pAfterLine)
	{
		UT_ASSERT(m_iMaxHeight > 0);
		UT_ASSERT(pAfterLine->isLastLineInBlock() || (pBL == pAfterLine->getBlock()));

		UT_sint32 iMargin = pNewLine->getMarginBefore();
		
		if ((pAfterLine->getY() + pAfterLine->getHeight() + iMargin + iHeight) > m_iMaxHeight)
		{
			pNewLine->setColumn(NULL);

			if (pAfterLine != getLastLine())
			{
				fp_Line* pLastLine = getLastLine();

				UT_ASSERT((pLastLine->getHeight() + pLastLine->getMarginBefore()) < (iHeight + iMargin));
			}
			
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
			m_bNeedsLayout = UT_TRUE;
		}

		pNewLine->setY(pAfterLine->getY() + pAfterLine->getHeight() + iMargin);
	}
	else
	{
		UT_ASSERT(pNewLine->isFirstLineInBlock()
				  || (getPrev()
					  && getPrev()->getLastLine()
					  && (getPrev()->getLastLine()->getBlock() == pBL)
					  )
			);
		
		m_vecLines.insertItemAt(pNewLine, 0);
		pNewLine->setY(0);

		if (m_vecLines.getItemCount() > 1)
		{
			m_bNeedsLayout = UT_TRUE;
		}
	}

	UT_sint32 iX = pBL->getLeftMargin();

	if (pNewLine->isFirstLineInBlock())
	{
		iX += pBL->getTextIndent();
	}

	pNewLine->setX(iX);

	UT_sint32 iMaxWidth = getWidth();
	iMaxWidth -= pBL->getRightMargin();
	iMaxWidth -= pBL->getLeftMargin();
	if (pNewLine->isFirstLineInBlock())
	{
		iMaxWidth -= pBL->getTextIndent();
	}
	
	pNewLine->setMaxWidth(iMaxWidth);
	pNewLine->setColumn(this);

	return UT_TRUE;
}

UT_Bool fp_Column::isEmpty(void) const
{
	return (m_vecLines.getItemCount() == 0);
}

void fp_Column::moveLineFromNextColumn(fp_Line* pLine)
{
#if 0
	fp_Column* pNextCol = getNext();
	UT_ASSERT(pNextCol);
	UT_ASSERT(pLine->getColumn() == pNextCol);
	UT_ASSERT(pLine->getColumn()->getFirstLine() == pLine);

	pNextCol->removeLine(pLine);
	insertLineAfter(pLine, getLastLine(), pLine->getHeight());
	UT_ASSERT(pLine->getColumn() == this);
	UT_ASSERT(getLastLine() == pLine);
#else
	fp_Column* pOtherCol = pLine->getColumn();
	UT_ASSERT(pOtherCol);
	UT_ASSERT(pLine->getColumn()->getFirstLine() == pLine);

	pOtherCol->removeLine(pLine);
	insertLineAfter(pLine, getLastLine(), pLine->getHeight());
	UT_ASSERT(pLine->getColumn() == this);
	UT_ASSERT(getLastLine() == pLine);
#endif
}

void fp_Column::moveLineToNextColumn(fp_Line* pLine)
{
	UT_sint32 ndx = m_vecLines.findItem(pLine);
	UT_ASSERT(ndx >= 0);

	moveLineToNextColumn(ndx);
}

void fp_Column::moveLineToNextColumn(UT_uint32 iBump)
{
	UT_ASSERT(m_vecLines.getItemCount() > 0);
	UT_ASSERT(iBump == (m_vecLines.getItemCount() - 1));
	
	fp_Column* pNextColumn = getNext();
	if (!pNextColumn)
	{
		pNextColumn = m_pSectionLayout->getNewColumn();
	}
					
	fp_Line* pBumpLine = (fp_Line*) m_vecLines.getNthItem(iBump);

	m_vecLines.deleteNthItem(iBump);

	pNextColumn->insertLineAfter(pBumpLine, NULL, pBumpLine->getHeight());
	UT_ASSERT(pBumpLine->getColumn() == pNextColumn);
	UT_ASSERT(pNextColumn->getFirstLine() == pBumpLine);
}

void fp_Column::bumpLinesToNextColumn(UT_uint32 iFirstToBump)
{
	UT_uint32 iCountLines = m_vecLines.getItemCount();
	UT_uint32 i;

	for (i=iCountLines-1; i >= iFirstToBump; i--)
	{
		moveLineToNextColumn(i);
	}
}

void fp_Column::checkForWidowsAndOrphans(void)
{
	fp_Line* pLine;
	
	pLine = getFirstLine();
	if (pLine)
	{
		fl_BlockLayout* pBlock = pLine->getBlock();
		UT_ASSERT(pBlock);
		pBlock->checkForWidowsAndOrphans();
	}
	
	pLine = getLastLine();
	if (pLine)
	{
		fl_BlockLayout* pBlock = pLine->getBlock();
		UT_ASSERT(pBlock);
		pBlock->checkForWidowsAndOrphans();
	}
}

void fp_Column::updateLayout(void)
{
	if (m_bNeedsLayout)
	{
		m_bNeedsLayout = UT_FALSE;
		
		UT_Bool bBumpedSomething = UT_FALSE;
		UT_Bool bDoNotSlurp = UT_FALSE;
		UT_sint32 iCurY = 0;
		UT_uint32 count = m_vecLines.getItemCount();
		UT_uint32 i;

		UT_ASSERT(m_iMaxHeight > 0);
		
		for (i = 0; i<count; i++)
		{
			fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

			UT_sint32 iMargin = 0;
			if (i > 0)
			{
				iMargin = pLine->getMarginBefore();
				iCurY += iMargin;
			}
			
			if ((iCurY + pLine->getHeight()) > m_iMaxHeight)
			{
				/*
				  We're out of room.  This line, as well as any others after
				  it, must move to the next column.
				*/

				int iFirstToBump = i;

				int iBump = count-1;
				while (iBump >= iFirstToBump)
				{
					moveLineToNextColumn(iBump);
					
					bBumpedSomething = UT_TRUE;
					
					iBump--;
				}

				if (bBumpedSomething)
				{
					fp_Column* pNextColumn = getNext();
					UT_ASSERT(pNextColumn);
					pNextColumn->updateLayout();

					pNextColumn->getFirstLine()->getBlock()->checkForWidowsAndOrphans();
				}
				
				break;
			}

			pLine->setY(iCurY);
			iCurY += pLine->getHeight();
		}

#if 0
		count = m_vecLines.getItemCount();
		if (count > 0)
		{
			for (i = 0; i<count; i++)
			{
				fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);
				fp_Run* pLastRunOnLine = pLine->getLastRun();

				if (
					(pLastRunOnLine->getType() == FPRUN_FORCEDCOLUMNBREAK)
					)
				{
					if ((i+1) < count)
					{
						bumpLinesToNextColumn(i+1);
						bBumpedSomething = UT_TRUE;
					}
					bDoNotSlurp = UT_TRUE;
					break;
				}
			}
		}
#endif
		
		if (bBumpedSomething)
		{
			bDoNotSlurp = UT_TRUE;
		}
		
		if (
			!bDoNotSlurp
			&& (iCurY < m_iMaxHeight)
			)
		{
			fp_Column* pNextColumn = getNext();
				
			if (pNextColumn)
			{
				UT_ASSERT(pNextColumn->getWidth() == getWidth());
				
				UT_Bool bDone = UT_FALSE;

				while (!bDone)
				{
					fp_Line* pFirstLineInNextCol = NULL;
					while (pNextColumn && !pFirstLineInNextCol)
					{
						pFirstLineInNextCol = pNextColumn->getFirstLine();
						if (!pFirstLineInNextCol)
						{
							pNextColumn = pNextColumn->getNext();
						}
					}
					
					UT_Bool bSlurpedSomething = UT_FALSE;
					UT_sint32 iMargin = 0;
					
					if (pFirstLineInNextCol)
					{
						fl_BlockLayout* pBlock = pFirstLineInNextCol->getBlock();
						
						iMargin = pFirstLineInNextCol->getMarginBefore();

						fp_Line* pTmpLine = pFirstLineInNextCol;
						UT_uint32 iNumBlockLines = 0;
						while (pTmpLine && (pTmpLine->getColumn() == pNextColumn))
						{
							iNumBlockLines++;
							
							pTmpLine = pTmpLine->getNext();
						}
						
						pTmpLine = pFirstLineInNextCol;
						UT_uint32 iSlurpHeight = 0;
						UT_uint32 iNumLinesThatFit = 0;
						UT_uint32 iSpace = getSpaceAtBottom();
						while (iNumLinesThatFit < iNumBlockLines)
						{
							UT_sint32 iMarg2 = pTmpLine->getMarginBefore();
								
							if ((iSlurpHeight + iMarg2 + pTmpLine->getHeight()) <= iSpace)
							{
								iSlurpHeight += (iMarg2 + pTmpLine->getHeight());
								iNumLinesThatFit++;
							}
							else
							{
								break;
							}

							pTmpLine = pTmpLine->getNext();
						}

						if (iNumLinesThatFit > 0)
						{
							if (iNumLinesThatFit < iNumBlockLines)
							{
								while (
									((iNumBlockLines - iNumLinesThatFit) < pBlock->getWidowsProperty())
									&& (iNumLinesThatFit > 0)
									)
								{
									iNumLinesThatFit--;
								}
							}

							if (iNumLinesThatFit > 0)
							{
								UT_uint32 iNumLinesAlreadyHere = 0;
								pTmpLine = getLastLine();
								if (pTmpLine && (pTmpLine->getBlock() == pFirstLineInNextCol->getBlock()))
								{
									while (pTmpLine && (pTmpLine->getColumn() == this))
									{
										iNumLinesAlreadyHere++;
										
										pTmpLine = pTmpLine->getPrev();
									}
								}

								if (
									(iNumLinesThatFit == iNumBlockLines)
									|| ((iNumLinesThatFit + iNumLinesAlreadyHere) >= pBlock->getOrphansProperty())
									)
								{
									/*
									  OK.  We can slurp
									*/

									fp_Line* pMoveLine = pFirstLineInNextCol;
									for (i=0; i<iNumLinesThatFit; i++)
									{
										moveLineFromNextColumn(pMoveLine);
										pMoveLine = pMoveLine->getNext();
										
										bSlurpedSomething = UT_TRUE;
									}
								}
								else
								{
									bDone = UT_TRUE;
								}
							}
							else
							{
								bDone = UT_TRUE;
							}
						}
						else
						{
							bDone = UT_TRUE;
						}
					}
					else
					{
						bDone = UT_TRUE;
					}

					if (bSlurpedSomething)
					{
						pNextColumn->updateLayout();
					}
				}
			}
		}

		{
			fp_Line* pLastLine = getLastLine();

			if (pLastLine)
			{
				if ((pLastLine->getY() + pLastLine->getHeight()) != m_iHeight)
				{
					m_iHeight = (pLastLine->getY() + pLastLine->getHeight());
					UT_ASSERT(m_pPage);
			
					m_pPage->columnHeightChanged(this);
				}
			}
		}
	}
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

UT_Bool fp_Column::containsPoint(UT_sint32 x, UT_sint32 y)
{
	if ((x < 0) || (x >= m_iWidth))
	{
		return UT_FALSE;
	}
	if ((y < 0) || (y >= m_iHeight))
	{
		return UT_FALSE;
	}

	return UT_TRUE;
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
	
	if (x < 0)
	{
		dx = -x;
	}
	else if (x > m_iWidth)
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
	else if (y > m_iHeight)
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

void fp_Column::lineHeightChanged(fp_Line* pLine, UT_sint32 iOldHeight, UT_sint32 iNewHeight)
{
	m_bNeedsLayout = UT_TRUE;
}

UT_sint32 fp_Column::getWidth(void) const
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

