/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include "ut_types.h"	// for FREEP

#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fb_Alignment.h"
#include "fp_Column.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"
#include "fp_Page.h"
#include "fl_SectionLayout.h"
#include "gr_DrawArgs.h"
#include "gr_Graphics.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"

fp_Line::fp_Line() 
{
	m_iAscent = 0;
	m_iDescent = 0;
	m_iMaxWidth = 0;
	m_iMaxWidthLayoutUnits = 0;
	m_iWidth = 0;
	m_iHeight = 0;
	m_iX = 0;
	m_iY = 0;
	m_pContainer = NULL;
	m_pBlock = NULL;

	m_bNeedsRedraw = UT_FALSE;
}

fp_Line::~fp_Line()
{
}

void fp_Line::setMaxWidth(UT_sint32 iMaxWidth)
{
	m_iMaxWidth = iMaxWidth;
}

void fp_Line::setMaxWidthInLayoutUnits(UT_sint32 iMaxWidth)
{
	m_iMaxWidthLayoutUnits = iMaxWidth;
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
	if (bTellTheRunAboutIt)
	{
		pRun->setLine(NULL);
	}

	UT_sint32 ndx = m_vecRuns.findItem(pRun);
	UT_ASSERT(ndx >= 0);
	m_vecRuns.deleteNthItem(ndx);


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

	setNeedsRedraw();
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
	const int count = m_vecRuns.getItemCount();
	UT_ASSERT(count > 0);

	fp_Run* pFirstRun = (fp_Run*) m_vecRuns.getNthItem(0);
	UT_ASSERT(pFirstRun);

	bBOL = UT_FALSE;
	if (x < pFirstRun->getX())
	{
		bBOL = UT_TRUE;

		UT_sint32 y2 = y - pFirstRun->getY() - m_iAscent + pFirstRun->getAscent();
		pFirstRun->mapXYToPosition(0, y2, pos, bBOL, bEOL);

		UT_ASSERT(bEOL == UT_TRUE || bEOL == UT_FALSE);
		UT_ASSERT(bBOL == UT_TRUE || bBOL == UT_FALSE);

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

					UT_ASSERT(bEOL == UT_TRUE || bEOL == UT_FALSE);
					UT_ASSERT(bBOL == UT_TRUE || bBOL == UT_FALSE);

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

					UT_ASSERT(bEOL == UT_TRUE || bEOL == UT_FALSE);
					UT_ASSERT(bBOL == UT_TRUE || bBOL == UT_FALSE);

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

	UT_ASSERT(bEOL == UT_TRUE || bEOL == UT_FALSE);
	UT_ASSERT(bBOL == UT_TRUE || bBOL == UT_FALSE);

}

void fp_Line::getOffsets(fp_Run* pRun, UT_sint32& xoff, UT_sint32& yoff)
{
  // This returns the baseline of run. ie the bottom of the line of text
  //
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
	UT_sint32 iMaxAscentLayoutUnits = 0;
	UT_sint32 iMaxDescentLayoutUnits = 0;

	for (i=0; i<count; i++)
	{
		UT_sint32 iAscent;
		UT_sint32 iDescent;
		UT_sint32 iAscentLayoutUnits;
		UT_sint32 iDescentLayoutUnits;

		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		iAscent = pRun->getAscent();
		iDescent = pRun->getDescent();
		iAscentLayoutUnits = pRun->getAscentInLayoutUnits();
		UT_ASSERT(!iAscent || iAscentLayoutUnits);
		iDescentLayoutUnits = pRun->getDescentInLayoutUnits();
	
	
		if (pRun->isSuperscript() || pRun->isSubscript())
		{
			iAscent += iAscent * 1/2;
			iDescent += iDescent;
			iAscentLayoutUnits += iAscentLayoutUnits * 1/2;
			iDescentLayoutUnits += iDescentLayoutUnits;
		}

		iMaxAscent = UT_MAX(iMaxAscent, iAscent);
		iMaxDescent = UT_MAX(iMaxDescent, iDescent);
		iMaxAscentLayoutUnits = UT_MAX(iMaxAscentLayoutUnits, iAscentLayoutUnits);
		iMaxDescentLayoutUnits = UT_MAX(iMaxDescentLayoutUnits, iDescentLayoutUnits);
	}

    UT_sint32 iOldHeight = m_iHeight;
	UT_sint32 iOldAscent = m_iAscent;
	UT_sint32 iOldDescent = m_iDescent;
	
	UT_sint32 iNewHeight = iMaxAscent + iMaxDescent;
	UT_sint32 iNewHeightLayoutUnits = iMaxAscentLayoutUnits + iMaxDescentLayoutUnits;
	UT_sint32 iNewAscent = iMaxAscent;
	UT_sint32 iNewDescent = iMaxDescent;

	{
		// adjust line height to include leading
		double dLineSpace, dLineSpaceLayout;
		fl_BlockLayout::eSpacingPolicy eSpacing;
		m_pBlock->getLineSpacing(dLineSpace, dLineSpaceLayout, eSpacing);

		if (eSpacing == fl_BlockLayout::spacing_EXACT)
			{
			iNewHeight = (UT_sint32) dLineSpace;
			
			iNewHeightLayoutUnits = (UT_sint32) dLineSpaceLayout;

			}
		else if (eSpacing == fl_BlockLayout::spacing_ATLEAST)
			{
			iNewHeight = UT_MAX(iNewHeight, (UT_sint32) dLineSpace);

			iNewHeightLayoutUnits = UT_MAX(iNewHeightLayoutUnits, (UT_sint32) dLineSpaceLayout);
			}
		else
			{
			// multiple
			iNewHeight = (UT_sint32) (iNewHeight * dLineSpace);
			iNewHeightLayoutUnits = (UT_sint32) (iNewHeightLayoutUnits * dLineSpaceLayout);
			}
	}

	if (
		(iOldHeight != iNewHeight)
		|| (iOldAscent != iNewAscent)
		|| (iOldDescent != iNewDescent)
		)
	{
		clearScreen();

		m_iHeight = iNewHeight;
		m_iHeightLayoutUnits = iNewHeightLayoutUnits;
		UT_ASSERT(m_iHeightLayoutUnits);
		m_iAscent = iNewAscent;
		m_iDescent = iNewDescent;
	}
}

void fp_Line::clearScreen(void)
{
	UT_uint32 count = m_vecRuns.getItemCount();
	if(count)
	{
		fp_Run* pRun;
		UT_Bool bNeedsClearing = UT_FALSE;

		UT_uint32 i;
		for (i = 0; i < count; i++)
		{
			pRun = (fp_Run*) m_vecRuns.getNthItem(i);

			if(!pRun->isDirty())
			{
				bNeedsClearing = UT_TRUE;

				pRun->markAsDirty();
			}
		}
		
		if(bNeedsClearing)
		{
			pRun = (fp_Run*) m_vecRuns.getNthItem(0);
			
			UT_sint32 xoffLine, yoffLine;

			m_pContainer->getScreenOffsets(this, xoffLine, yoffLine);

			pRun->getGraphics()->clearArea(xoffLine, yoffLine, m_iMaxWidth, m_iHeight);
		}
	}
	
}

void fp_Line::clearScreenFromRunToEnd(UT_uint32 runIndex)
{
	fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(runIndex);
	UT_uint32 count = m_vecRuns.getItemCount();

	// Find the first none dirty run.

	UT_uint32 i;
	for(i = runIndex; i < count; i++)
	{
		pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		if(pRun->isDirty())
		{
			runIndex++;
		}
		else
		{
			break;
		}
	}

	if(runIndex < count)
	{
		UT_sint32 xoff, yoff;

		pRun = (fp_Run*) m_vecRuns.getNthItem(runIndex);

		getScreenOffsets(pRun, xoff, yoff);
		UT_sint32 xoffLine, yoffLine;

		m_pContainer->getScreenOffsets(this, xoffLine, yoffLine);
		
		pRun->getGraphics()->clearArea(xoff, yoff, m_iMaxWidth - (xoff - xoffLine), m_iHeight);
		
		for (i = runIndex; i < count; i++)
		{
			pRun = (fp_Run*) m_vecRuns.getNthItem(i);

			pRun->markAsDirty();
		}
	}
}


void fp_Line::redrawUpdate(void)
{
	UT_uint32 count = m_vecRuns.getItemCount();
	if(count)
	{
		draw(((fp_Run*) m_vecRuns.getNthItem(0))->getGraphics());
	}

	m_bNeedsRedraw = UT_FALSE;
	
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


	UT_uint32 iCountRuns = m_vecRuns.getItemCount();
	UT_sint32 iX = 0;
	UT_sint32 iStartX;
	UT_uint32 i;
	UT_Bool bLineErased = UT_FALSE;

	iStartX = iX = pAlignment->getStartPosition();
	
	for (i=0; i<iCountRuns; i++)		// TODO do we need to do this if iMoveOver is zero ??
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);
		if(!bLineErased && iX != pRun->getX())
			{

			// Need to erase some or all of the line depending of Alignment mode.

			pAlignment->eraseLineFromRun(this, i);
			bLineErased = UT_TRUE;
			}
			
		pRun->setX(iX);

		
		if (pRun->getType() == FPRUN_TAB)
		{
			UT_sint32 iPos;
			unsigned char iTabType;

			UT_Bool bRes = findNextTabStop(iX - iStartX, iPos, iTabType);
			UT_ASSERT(bRes);

			fp_TabRun* pTabRun = static_cast<fp_TabRun*>(pRun);
			fp_Run *pScanRun;
			int iScanWidth = 0;

			// for everybody except the left tab, we need to know how much text is to follow
			switch ( iTabType )
			{
			case FL_TAB_LEFT:
				pTabRun->setWidth(iPos - (iX - iStartX));
				iX = iPos + iStartX;
				break;

			case FL_TAB_CENTER:
				for ( pScanRun = pRun->getNext();	
					  pScanRun && pScanRun->getType() != FPRUN_TAB; 
					  pScanRun = pScanRun->getNext() )
				{
					iScanWidth += pScanRun->getWidth();
				}
	
				if ( iScanWidth / 2 > iPos - (iX - iStartX) )
					pTabRun->setWidth(0);
				else
				{
					int tabWidth = iPos - (iX - iStartX) - iScanWidth / 2 ;
					pTabRun->setWidth( tabWidth );
					iX += tabWidth;
				}
		
				break;

			case FL_TAB_RIGHT:
				for ( pScanRun = pRun->getNext();	
					  pScanRun && pScanRun->getType() != FPRUN_TAB; 
					  pScanRun = pScanRun->getNext() )
				{
					iScanWidth += pScanRun->getWidth();
				}
		
				if ( iScanWidth > iPos - (iX - iStartX) )
					pTabRun->setWidth(0);
				else
				{
					int tabWidth = iPos - (iX - iStartX) - iScanWidth ;
					pTabRun->setWidth( tabWidth );
					iX += tabWidth;
				}
				break;

			case FL_TAB_DECIMAL:
			{
				UT_UCSChar const *pSpanPtr, *pCh;
				UT_UCSChar *pDecimalStr;
				UT_uint32   len, runLen;
				iScanWidth=0;

				// the string to search for decimals
				UT_UCS_cloneString_char(&pDecimalStr, ".");

				for ( pScanRun = pRun->getNext();	
					  pScanRun && pScanRun->getType() != FPRUN_TAB; 
					  pScanRun = pScanRun->getNext() )
				{
					UT_Bool foundDecimal = UT_FALSE;
					if ( getBlock()->getSpanPtr( pScanRun->getBlockOffset(), &pSpanPtr, &len ) )
					{
						runLen = pScanRun->getLength();
						UT_ASSERT(len >= runLen);
						pCh = pSpanPtr;

						#if defined(UT_DEBUG) && 1
							char szbuffer[0x400];
							UT_UCS_strcpy_to_char(szbuffer, pSpanPtr);
							szbuffer[runLen] = 0;
							UT_DEBUGMSG(("%s:%d  searching span [%s] (%d chars) (iScanWidth=%d)\n", 
										  __FILE__, __LINE__, szbuffer, runLen, iScanWidth));
						#endif

						while ( runLen )
						{
							if ( *pCh++ == *pDecimalStr )		// TODO - FIXME
							{
								foundDecimal = UT_TRUE;	
								break;
							}
							runLen--;
						}
					}
					UT_DEBUGMSG(("%s:%d  foundDecimal=%d len=%d iScanWidth=%d \n", 
								__FILE__, __LINE__, foundDecimal, pScanRun->getLength()-runLen, iScanWidth));
					if ( foundDecimal )
					{
						iScanWidth += getBlock()->getDocLayout()->getGraphics()->
							measureString( pSpanPtr, 0, pScanRun->getLength() - runLen, NULL );
						break; // we found our decimal, don't search any further
					}
					else	
						iScanWidth += pScanRun->getWidth();
				}
			
				UT_DEBUGMSG((" tabrun iX=%d iPos=%d iScanWidth=%d tabwidth=%d newX=%d\n", 
									  iX,	iPos,   iScanWidth,   iPos-(iX-iStartX)-iScanWidth,iPos-iScanWidth));	
				pTabRun->setWidth(iPos - (iX - iStartX) - iScanWidth);
				iX = iPos - iScanWidth + iStartX;
			
				FREEP(pDecimalStr);	
				break;
			}
		
			case FL_TAB_BAR:

			default:
				UT_ASSERT(UT_NOT_IMPLEMENTED);
			};

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

void fp_Line::setXInLayoutUnits(UT_sint32 iX)
{
	m_iXLayoutUnits = iX;
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

void fp_Line::setYInLayoutUnits(UT_sint32 iY)
{
	m_iYLayoutUnits = iY;
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
//		UT_ASSERT(pNextLine);
		if (!pNextLine)
			return 0;

		UT_ASSERT(pNextLine->isFirstLineInBlock());
					
		UT_sint32 iBottomMargin = getBlock()->getBottomMargin();
		
		UT_sint32 iNextTopMargin = pNextLine->getBlock()->getTopMargin();
		
		UT_sint32 iMargin = UT_MAX(iBottomMargin, iNextTopMargin);

		return iMargin;
	}

	return 0;
}

UT_sint32 fp_Line::getMarginAfterInLayoutUnits(void) const
{
	if (isLastLineInBlock() && getBlock()->getNext())
	{
		fp_Line* pNextLine = getBlock()->getNext()->getFirstLine();
//		UT_ASSERT(pNextLine);
		if (!pNextLine)
			return 0;

		UT_ASSERT(pNextLine->isFirstLineInBlock());
					
		UT_sint32 iBottomMargin = getBlock()->getBottomMarginInLayoutUnits();
		
		UT_sint32 iNextTopMargin = pNextLine->getBlock()->getTopMarginInLayoutUnits();
		
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

UT_Bool	fp_Line::findNextTabStopInLayoutUnits(UT_sint32 iStartX, UT_sint32& iPosition, unsigned char& iType)
{
	UT_sint32		iTabStopPosition = 0;
	unsigned char	iTabStopType;

	UT_Bool bRes = m_pBlock->findNextTabStopInLayoutUnits(iStartX + getXInLayoutUnits(), getXInLayoutUnits() + getMaxWidthInLayoutUnits(), iTabStopPosition, iTabStopType);
	UT_ASSERT(bRes);

	iTabStopPosition -= getXInLayoutUnits();

	if (iTabStopPosition < m_iMaxWidthLayoutUnits)
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

	// Do same calculation but in layout units.

	iX = m_pBlock->getLeftMarginInLayoutUnits();

	if (isFirstLineInBlock())
	{
		iX += m_pBlock->getTextIndentInLayoutUnits();
	}

	setXInLayoutUnits(iX);

	iMaxWidth = m_pContainer->getWidthInLayoutUnits();
	iMaxWidth -= m_pBlock->getRightMarginInLayoutUnits();
	iMaxWidth -= m_pBlock->getLeftMarginInLayoutUnits();
	if (isFirstLineInBlock())
	{
		iMaxWidth -= m_pBlock->getTextIndentInLayoutUnits();
	}
	
	setMaxWidthInLayoutUnits(iMaxWidth);

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
	if(!isEmpty())
	{
		fp_Run* pRun = getLastRun();
		if (pRun->getType() == FPRUN_FORCEDCOLUMNBREAK)
		{			
			return UT_TRUE;
		}
	}

	return UT_FALSE;
}

UT_Bool fp_Line::containsForcedPageBreak(void) const
{
	if (!isEmpty())
	{
		fp_Run* pRun = getLastRun();
		if (pRun->getType() == FPRUN_FORCEDPAGEBREAK)
		{
			return UT_TRUE;
		}
	}

	return UT_FALSE;
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
	const UT_uint32 iCountRuns = m_vecRuns.getItemCount();
	UT_sint32 iX = 0;

	// first calc the width of the line
	for (UT_uint32 i = 0; i < iCountRuns; ++i)
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
		else
		{
			iX += pRun->getWidth();
		}
	}

	UT_ASSERT(iX <= m_iMaxWidth);

	m_iWidth = iX;

	return iX;
}

UT_sint32 fp_Line::calculateWidthOfLineInLayoutUnits(void)
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

			UT_Bool bRes = findNextTabStopInLayoutUnits(iX, iPos, iTabType);
			UT_ASSERT(bRes);
			UT_ASSERT(iTabType == FL_TAB_LEFT);

			fp_TabRun* pTabRun = static_cast<fp_TabRun*>(pRun);
			pTabRun->setWidth(iPos - iX);
			
			iX = iPos;
		}
		else
		{
			iX += pRun->getWidthInLayoutUnits();
		}
	}

	m_iWidthLayoutUnits = iX;

	return iX;
}

UT_sint32 fp_Line::calculateWidthOfTrailingSpaces(void)
{
	// need to move back until we find the first non blank character and
	// return the distance back to this character.

	UT_ASSERT(!isEmpty());

	UT_sint32 iTrailingBlank = 0;

	fp_Run *pCurrentRun = getLastRun();

	do
	{
		if(!pCurrentRun->doesContainNonBlankData())
		{
			iTrailingBlank += pCurrentRun->getWidth();
		}
		else
		{
			iTrailingBlank += pCurrentRun->findTrailingSpaceDistance();
			break;
		}
		
		if(pCurrentRun == getFirstRun())
			break;

		pCurrentRun = pCurrentRun->getPrev();
	}
	while(pCurrentRun);


	return iTrailingBlank;
}

UT_sint32 fp_Line::calculateWidthOfTrailingSpacesInLayoutUnits(void)
{
	// need to move back until we find the first non blank character and
	// return the distance back to this character.

	UT_ASSERT(!isEmpty());

	UT_sint32 iTrailingBlank = 0;

	fp_Run *pCurrentRun = getLastRun();

	do
	{
		if(!pCurrentRun->doesContainNonBlankData())
		{
			iTrailingBlank += pCurrentRun->getWidthInLayoutUnits();
		}
		else
		{
			iTrailingBlank += pCurrentRun->findTrailingSpaceDistanceInLayoutUnits();
			break;
		}
		
		if(pCurrentRun == getFirstRun())
			break;

		pCurrentRun = pCurrentRun->getPrev();
	}
	while(pCurrentRun);


	return iTrailingBlank;
}

UT_uint32 fp_Line::countJustificationPoints(void) const
{
	UT_uint32 iCountRuns = m_vecRuns.getItemCount();
	UT_sint32 i;
	UT_uint32 iSpaceCount = 0;
	UT_Bool bStartFound = UT_FALSE;

	// first calc the width of the line
	for (i=iCountRuns -1 ; i >= 0; i--)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);
		
		if (pRun->getType() == FPRUN_TAB)
		{
		  //			UT_ASSERT(UT_FALSE);
		  UT_DEBUGMSG(("TODO - decide if tab is a space \n"));
			// TODO: decide if a tab is a space.

		}
		else if (pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);
			if(bStartFound)
			{
				iSpaceCount += pTR->countJustificationPoints();
			}
			else
			{
				if(pTR->doesContainNonBlankData())
				{
					iSpaceCount += pTR->countJustificationPoints();
					iSpaceCount -= pTR->countTrailingSpaces();
					bStartFound = UT_TRUE;
				}

			}
		}
		else
		{
			bStartFound = UT_TRUE;
		}
	}

	return iSpaceCount;
}


UT_Bool fp_Line::isLastCharacter(UT_UCSChar Character) const
{
	UT_ASSERT(!isEmpty());

	fp_Run *pRun = getLastRun();

	if (pRun->getType() == FPRUN_TEXT)
	{
		fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);

		return pTR->isLastCharacter(Character);
	}

	return UT_FALSE;
}

void fp_Line::resetJustification()
{
	UT_uint32 count = m_vecRuns.getItemCount();
	for (UT_uint32 i=0; i<count; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		if (pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);

			pTR->resetJustification();
		}
	}
}


void fp_Line::distributeJustificationAmongstSpaces(UT_sint32 iAmount)
{
	if(iAmount)
	{
		UT_uint32 iSpaceCount = countJustificationPoints();

		if(iSpaceCount)
		{
			// Need to distribute Extra width amongst spaces.

			splitRunsAtSpaces();
			
			UT_uint32 count = m_vecRuns.getItemCount();
			for (UT_uint32 i=0; i<count; i++)
			{
				fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

				if (pRun->getType() == FPRUN_TEXT)
				{
					fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);

					UT_uint32 iSpacesInText = pTR->countJustificationPoints();
					if(iSpacesInText > iSpaceCount)
						iSpacesInText = iSpaceCount;	// Takes care of trailing spaces.

					if(iSpacesInText)
					{
						UT_sint32 iJustifyAmountForRun = (int)((double)iAmount / iSpaceCount * iSpacesInText);

						pTR->distributeJustificationAmongstSpaces(iJustifyAmountForRun, iSpacesInText);

						iAmount -= iJustifyAmountForRun;
						iSpaceCount -= iSpacesInText;
					}
				}
			}
		}		
	}
}

void fp_Line::splitRunsAtSpaces(void)
{
	UT_uint32 count = m_vecRuns.getItemCount();
	for (UT_uint32 i=0; i<count; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		if (pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun* pTR = (fp_TextRun *)pRun;
			UT_sint32 iSpacePosition;

			iSpacePosition = pTR->findCharacter(0, UCS_SPACE);

			if ((iSpacePosition > 0) && 
				((UT_uint32) iSpacePosition < pTR->getBlockOffset() + pTR->getLength() - 1))
			{
				pTR->split(iSpacePosition + 1);
				count++;
			}
		}
	}
	
	count = m_vecRuns.getItemCount();

	fp_Run* pRun = getLastRun();

	if (pRun->getType() == FPRUN_TEXT)
	{
		fp_TextRun* pTR = (fp_TextRun *)pRun;
		UT_sint32 iSpacePosition = pTR->findCharacter(0, UCS_SPACE);

		if ((iSpacePosition > 0) && 
			((UT_uint32) iSpacePosition < pTR->getBlockOffset() + pTR->getLength() - 1))
		{
			pTR->split(iSpacePosition + 1);
		}
	}


}


