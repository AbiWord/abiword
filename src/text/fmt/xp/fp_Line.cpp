/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fb_Alignment.h"
#include "fp_Column.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"
#include "gr_DrawArgs.h"
#include "gr_Graphics.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"

fp_Line::fp_Line() 
{
	m_iAscent = 0;
	m_iDescent = 0;
	m_iMaxWidth = 0;
	m_iWidth = 0;
	m_iHeight = 0;
	m_iX = 0;
	m_iY = 0;
	m_pContainer = NULL;
	m_pBlock = NULL;
}

fp_Line::~fp_Line()
{
}

void fp_Line::setMaxWidth(UT_sint32 iMaxWidth)
{
	m_iMaxWidth = iMaxWidth;
}

void fp_Line::setContainer(fp_Container* pContainer)
{
	if (pContainer == m_pContainer)
	{
		return;
	}

	if (m_pContainer)
	{
		clearScreen();
	}
	
	m_pContainer = pContainer;
}

UT_Bool fp_Line::removeRun(fp_Run* pRun, UT_Bool bTellTheRunAboutIt)
{
	UT_sint32 ndx = m_vecRuns.findItem(pRun);
	UT_ASSERT(ndx >= 0);
	m_vecRuns.deleteNthItem(ndx);

	if (bTellTheRunAboutIt)
	{
		pRun->setLine(NULL);
	}

	return UT_TRUE;
}

void fp_Line::insertRunBefore(fp_Run* pNewRun, fp_Run* pBefore)
{
	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	UT_ASSERT(pNewRun);
	UT_ASSERT(pBefore);

	pNewRun->setLine(this);
	
	UT_sint32 ndx = m_vecRuns.findItem(pBefore);
	UT_ASSERT(ndx >= 0);

	m_vecRuns.insertItemAt(pNewRun, ndx);
}

void fp_Line::insertRun(fp_Run* pNewRun)
{
	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	pNewRun->setLine(this);

	m_vecRuns.insertItemAt(pNewRun, 0);
}

void fp_Line::addRun(fp_Run* pNewRun)
{
	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	pNewRun->setLine(this);

	m_vecRuns.addItem(pNewRun);
}

void fp_Line::insertRunAfter(fp_Run* pNewRun, fp_Run* pAfter)
{
	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	UT_ASSERT(pNewRun);
	UT_ASSERT(pAfter);
	
	pNewRun->setLine(this);
	
	UT_sint32 ndx = m_vecRuns.findItem(pAfter);
	UT_ASSERT(ndx >= 0);
	
	m_vecRuns.insertItemAt(pNewRun, ndx+1);
}

void fp_Line::remove(void)
{
	if (m_pNext)
	{
		m_pNext->setPrev(m_pPrev);
	}

	if (m_pPrev)
	{
		m_pPrev->setNext(m_pNext);
	}

	m_pContainer->removeLine(this);
}

void fp_Line::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	int count = m_vecRuns.getItemCount();
	UT_ASSERT(count > 0);

	fp_Run* pFirstRun = (fp_Run*) m_vecRuns.getNthItem(0);
	UT_ASSERT(pFirstRun);

	bBOL = UT_FALSE;
	if (x < pFirstRun->getX())
	{
		bBOL = UT_TRUE;

		UT_sint32 y2 = y - pFirstRun->getY() - m_iAscent + pFirstRun->getAscent();
		pFirstRun->mapXYToPosition(0, y2, pos, bBOL, bEOL);

		return;
	}

	// check all of the runs.
	
	fp_Run* pClosestRun = NULL;
	UT_sint32 iClosestDistance = 0;

	for (int i=0; i<count; i++)
	{
		fp_Run* pRun2 = (fp_Run*) m_vecRuns.getNthItem(i);

		if (pRun2->canContainPoint())
		{
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
#if 0
					// this only happens in an empty line, right?
					/*
					  NOPE.  Runs with no width can actually happen now, due to
					  a variety of changes, including the introduction of forced
					  breaks.
					*/
					
					UT_ASSERT(m_iWidth==0);
					UT_ASSERT(i==0);
					UT_ASSERT(count==1);
#endif

					pRun2->mapXYToPosition(x - pRun2->getX(), y2, pos, bBOL, bEOL);

					return;
				}
			}

			if (!pClosestRun)
			{
				pClosestRun = pRun2;
				if (x < pRun2->getX())
				{
					iClosestDistance = pRun2->getX() - x;
				}
				else if (x >= pRun2->getX() + pRun2->getWidth())
				{
					iClosestDistance = x - (pRun2->getX() + pRun2->getWidth());
				}
			}
			else
			{
				if (x < pRun2->getX())
				{
					if ((pRun2->getX() - x) < iClosestDistance)
					{
						iClosestDistance = pRun2->getX() - x;
						pClosestRun = pRun2;
					}
				}
				else if (x >= (pRun2->getX() + pRun2->getWidth()))
				{
					if (x - ((pRun2->getX() + pRun2->getWidth())) < iClosestDistance)
					{
						iClosestDistance = x - (pRun2->getX() + pRun2->getWidth());
						pClosestRun = pRun2;
					}
				}
			}
		}
	}

	UT_ASSERT(pClosestRun);
	
	UT_sint32 y2 = y - pClosestRun->getY() - m_iAscent + pClosestRun->getAscent();
	pClosestRun->mapXYToPosition(x - pClosestRun->getX(), y2, pos, bBOL, bEOL);
}

void fp_Line::getOffsets(fp_Run* pRun, UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pContainer->getOffsets(this, my_xoff, my_yoff);
	
	xoff = my_xoff + pRun->getX();
	yoff = my_yoff + pRun->getY() + m_iAscent - pRun->getAscent();
}

void fp_Line::getScreenOffsets(fp_Run* pRun,
							   UT_sint32& xoff,
							   UT_sint32& yoff)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	/*
	  This method returns the screen offsets of the given
	  run, referring to the UPPER-LEFT corner of the run.
	*/
	
	m_pContainer->getScreenOffsets(this, my_xoff, my_yoff);
	
	xoff = my_xoff + pRun->getX();
	yoff = my_yoff + pRun->getY();
}

void fp_Line::recalcHeight()
{
	UT_sint32 count = m_vecRuns.getItemCount();
	UT_sint32 i;

	UT_sint32 iMaxAscent = 0;
	UT_sint32 iMaxDescent = 0;

	for (i=0; i<count; i++)	// TODO merge these two loops into one....
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

    UT_sint32 iOldHeight = m_iHeight;
	UT_sint32 iOldAscent = m_iAscent;
	UT_sint32 iOldDescent = m_iDescent;
	
	UT_sint32 iNewHeight = iMaxAscent + iMaxDescent;
	UT_sint32 iNewAscent = iMaxAscent;
	UT_sint32 iNewDescent = iMaxDescent;

	{
		// adjust line height to include leading
		double dLineSpace;
		UT_Bool bExact;
		m_pBlock->getLineSpacing(dLineSpace, bExact);

		if (bExact)
			iNewHeight += (UT_sint32) dLineSpace;
		else
			iNewHeight = (UT_sint32) (iNewHeight * dLineSpace);
	}

	if (
		(iOldHeight != iNewHeight)
		|| (iOldAscent != iNewAscent)
		|| (iOldDescent != iNewDescent)
		)
	{
		clearScreen();

		m_iHeight = iNewHeight;
		m_iAscent = iNewAscent;
		m_iDescent = iNewDescent;
	}
}

void fp_Line::clearScreen(void)
{
	int count = m_vecRuns.getItemCount();

	for (int i=0; i < count; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		pRun->clearScreen();
	}
}

void fp_Line::draw(GR_Graphics* pG)
{
	UT_ASSERT(m_iWidth <= m_iMaxWidth);
	
	UT_sint32 my_xoff = 0, my_yoff = 0;
	
	m_pContainer->getScreenOffsets(this, my_xoff, my_yoff);

	dg_DrawArgs da;
	
	da.yoff = my_yoff + m_iAscent;
	da.xoff = my_xoff;
	da.pG = pG;

	int count = m_vecRuns.getItemCount();
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
	int count = m_vecRuns.getItemCount();

	pDA->yoff += m_iAscent;

	for (int i=0; i<count; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		dg_DrawArgs da = *pDA;
		da.xoff += pRun->getX();
		da.yoff += pRun->getY();
		pRun->draw(&da);
	}
}

void fp_Line::layout(void)
{
	recalcHeight();
	
	fb_Alignment *pAlignment = getBlock()->getAlignment();
	pAlignment->initialize(this);

/*
	m_iWidth = calculateWidthOfLine();

	UT_sint32 iExtraWidth = getMaxWidth() - m_iWidth;

	UT_uint32 iAlignCmd = getBlock()->getAlignment();

	UT_sint32 iMoveOver = 0;
	
	switch (iAlignCmd)
	{
	case FL_ALIGN_BLOCK_LEFT:
		iMoveOver = 0;
		break;
	case FL_ALIGN_BLOCK_RIGHT:
		iMoveOver = iExtraWidth;
		break;
	case FL_ALIGN_BLOCK_CENTER:
		iMoveOver = iExtraWidth / 2;
		break;
	case FL_ALIGN_BLOCK_JUSTIFY:
		// TODO
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
*/

	UT_uint32 iCountRuns = m_vecRuns.getItemCount();
	UT_sint32 iX = 0;
	UT_uint32 i;

	iX = pAlignment->getStartPosition();
	
	for (i=0; i<iCountRuns; i++)		// TODO do we need to do this if iMoveOver is zero ??
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);
		pRun->setX(iX);
		
		if (pRun->getType() == FPRUN_TAB)
		{
			UT_sint32 iPos;
			unsigned char iTabType;

			UT_Bool bRes = findNextTabStop(iX, iPos, iTabType);
			UT_ASSERT(bRes);
			UT_ASSERT(iTabType == FL_TAB_LEFT);

			fp_TabRun* pTabRun = static_cast<fp_TabRun*>(pRun);
			pTabRun->setWidth(iPos - iX);
			
			iX = iPos;
		}
		else
		{
			iX += pAlignment->getMove(pRun);
		}
	}
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

UT_sint32 fp_Line::getMarginBefore(void) const
{
	if (isFirstLineInBlock() && getBlock()->getPrev())
	{
		fp_Line* pPrevLine = getBlock()->getPrev()->getLastLine();
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
	if (isLastLineInBlock() && getBlock()->getNext())
	{
		fp_Line* pNextLine = getBlock()->getNext()->getFirstLine();
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

UT_Bool	fp_Line::findNextTabStop(UT_sint32 iStartX, UT_sint32& iPosition, unsigned char& iType)
{
	UT_sint32		iTabStopPosition = 0;
	unsigned char	iTabStopType;

	UT_Bool bRes = m_pBlock->findNextTabStop(iStartX + getX(), getX() + getMaxWidth(), iTabStopPosition, iTabStopType);
	UT_ASSERT(bRes);

	iTabStopPosition -= getX();

	if (iTabStopPosition < m_iMaxWidth)
	{
		iPosition = iTabStopPosition;
		iType = iTabStopType;

		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

void fp_Line::recalcMaxWidth()
{
	UT_sint32 iX = m_pBlock->getLeftMargin();

	if (isFirstLineInBlock())
	{
		iX += m_pBlock->getTextIndent();
	}

	setX(iX);

	UT_sint32 iMaxWidth = m_pContainer->getWidth();
	iMaxWidth -= m_pBlock->getRightMargin();
	iMaxWidth -= m_pBlock->getLeftMargin();
	if (isFirstLineInBlock())
	{
		iMaxWidth -= m_pBlock->getTextIndent();
	}
	
	setMaxWidth(iMaxWidth);
}

fp_Line*	fp_Line::getNextLineInSection(void) const
{
	if (m_pNext)
	{
		return m_pNext;
	}

	fl_BlockLayout* pNextBlock = m_pBlock->getNext();
	if (pNextBlock)
	{
		return pNextBlock->getFirstLine();
	}

	return NULL;
}

fp_Line*	fp_Line::getPrevLineInSection(void) const
{
	if (m_pPrev)
	{
		return m_pPrev;
	}

	fl_BlockLayout* pPrevBlock = m_pBlock->getPrev();
	if (pPrevBlock)
	{
		return pPrevBlock->getLastLine();
	}

	return NULL;
}

UT_Bool	fp_Line::containsForcedColumnBreak(void) const
{
	fp_Run* pRun = getLastRun();
	if (pRun->getType() == FPRUN_FORCEDCOLUMNBREAK)
	{
		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

UT_Bool fp_Line::containsForcedPageBreak(void) const
{
	fp_Run* pRun = getLastRun();
	if (pRun->getType() == FPRUN_FORCEDPAGEBREAK)
	{
		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

void fp_Line::coalesceRuns(void)
{
	UT_uint32 count = m_vecRuns.getItemCount();
	for (UT_uint32 i=0; i<(count-1); i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		if (pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);
			if (pTR->canMergeWithNext())
			{
				pTR->mergeWithNext();
				count--;
			}
		}
	}
}

UT_sint32 fp_Line::calculateWidthOfLine(void)
{
	UT_uint32 iCountRuns = m_vecRuns.getItemCount();
	UT_sint32 iX = 0;
	UT_uint32 i;

	// first calc the width of the line
	for (i=0; i<iCountRuns; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);
		
		if (pRun->getType() == FPRUN_TAB)
		{
			UT_sint32 iPos;
			unsigned char iTabType;

			UT_Bool bRes = findNextTabStop(iX, iPos, iTabType);
			UT_ASSERT(bRes);
			UT_ASSERT(iTabType == FL_TAB_LEFT);

			fp_TabRun* pTabRun = static_cast<fp_TabRun*>(pRun);
			pTabRun->setWidth(iPos - iX);
			
			iX = iPos;
		}
		else if (pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pRun);

			if(i == 0)
			{
				iX += pRun->getWidth();
			}
			else if(i == iCountRuns - 1)
			{
				iX += pRun->getWidth();
			}
			else
			{
				iX += pRun->getWidth();
			}
		}
		else
		{
			iX += pRun->getWidth();
		}
	}

	m_iWidth = iX;

	return iX;
}

UT_uint32 fp_Line::countSpaces(void) const
{
	UT_uint32 iCountRuns = m_vecRuns.getItemCount();
	UT_uint32 i;
	int iSpaceCount = 0;

	// first calc the width of the line
	for (i=0; i<iCountRuns; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);
		
		if (pRun->getType() == FPRUN_TAB)
		{
			UT_ASSERT(UT_FALSE);
			
			// TODO: decide if a tab is a space.

		}
		else
		{
			if(pRun->canBreakBefore())
				{
				iSpaceCount++;
				}
		}

	}

	return iSpaceCount;
}

void fp_Line::splitRunsAtSpaces(void)
{
	UT_uint32 count = m_vecRuns.getItemCount();
	for (UT_uint32 i=0; i<count; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		if (pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);
			UT_sint32 iSpacePosition;
			do
			{

				iSpacePosition = pTR->findCharacter(1, UCS_SPACE);

				if (iSpacePosition > 0)
				{
					pTR->split(iSpacePosition);
					count++;
				}
			}
			while(iSpacePosition > 0);
		}
	}
	
	count = m_vecRuns.getItemCount();

	fp_Run* pRun = getLastRun();

	if (pRun->getType() == FPRUN_TEXT)
	{
		fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);
		UT_sint32 iSpacePosition = pTR->findCharacter(1, UCS_SPACE);

		if(iSpacePosition > 0)
			pTR->split(iSpacePosition);
	}
}

UT_Bool fp_Line::isLastCharacter(UT_UCSChar Character) const
{
	fp_Run *pRun = getLastRun();

	if (pRun->getType() == FPRUN_TEXT)
	{
		fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);

		return pTR->isLastCharacter(Character);
	}

	return UT_FALSE;
}


