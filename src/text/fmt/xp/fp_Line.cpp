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

#ifdef BIDI_ENABLED
	#ifdef USE_STATIC_MAP
	//initialize the static members of the class
	UT_uint32    fp_Line::s_iClassInstanceCounter = 0;
	UT_sint32  * fp_Line::s_pMapOfRuns = 0;
	UT_uint32    fp_Line::s_iMapOfRunsSize = 0;
	fp_Line    * fp_Line::s_pMapOwner = 0;
	#else
	//make sure that any references to the static members are renamed to their non-static versions
	#define s_iMapOfRunsSize m_iMapOfRunsSize
	#define s_pMapOfRuns m_pMapOfRuns
	#endif
#endif

fp_Line::fp_Line() 
{
	m_iAscent = 0;
	m_iDescent = 0;
	m_iMaxWidth = 0;
	m_iMaxWidthLayoutUnits = 0;
	m_iWidth = 0;
	m_iWidthLayoutUnits = 0;
	m_iHeight = 0;
	m_iHeightLayoutUnits = 0;
	m_iX = 0;
	m_iXLayoutUnits = 0;
	m_iY = 0;
	m_iYLayoutUnits = 0;
	m_pContainer = NULL;
	m_pBlock = NULL;

	m_bNeedsRedraw = false;
#ifdef BIDI_ENABLED
	m_iRunsRTLcount = 0;
	m_iRunsLTRcount = 0;
	m_bMapDirty = true;    //map that has not been initialized is dirty by deafault

	#ifdef USE_STATIC_MAP
	if(!s_pMapOfRuns)
	{
		s_pMapOfRuns = new UT_sint32[RUNS_MAP_SIZE];
		s_iMapOfRunsSize = RUNS_MAP_SIZE;
	}
	++s_iClassInstanceCounter; // this tells us how many instances of Line are out there
	                           //we use this to decide whether the above should be
	                           //deleted by the destructor
	#else
	m_pMapOfRuns = new UT_sint32[RUNS_MAP_SIZE];
	m_iMapOfRunsSize = RUNS_MAP_SIZE;
	#endif

   	UT_ASSERT(s_pMapOfRuns);
#endif
	m_bNeedsRedraw = false;
}

fp_Line::~fp_Line()
{
#ifdef BIDI_ENABLED
	#ifdef USE_STATIC_MAP
	--s_iClassInstanceCounter;
	if(!s_iClassInstanceCounter) //this is the last/only instance of the class Line
	{
		delete[] s_pMapOfRuns;
		s_pMapOfRuns = 0;
	}
	#else
	delete[] m_pMapOfRuns;
	m_pMapOfRuns = 0;
	#endif
#endif
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

bool fp_Line::removeRun(fp_Run* pRun, bool bTellTheRunAboutIt)
{
#ifdef BIDI_ENABLED
	// need to tell the previous run to redraw, in case this run contained
	// overstriking characters
	fp_Run* pPrevRun  = pRun->getPrev();
	if(pPrevRun)
		pPrevRun->clearScreen();
#endif	
	if (bTellTheRunAboutIt)
	{
		pRun->setLine(NULL);
	}

#ifdef BIDI_ENABLED
    switch(pRun->getDirection())
    {
		case 0:
			m_iRunsLTRcount--;
			UT_ASSERT((m_iRunsLTRcount >= 0));
			break;
			
		case 1:
			m_iRunsRTLcount--;
			UT_ASSERT((m_iRunsRTLcount >= 0));
			break;
		default:;
    }
#endif
	
	
	UT_sint32 ndx = m_vecRuns.findItem(pRun);
	UT_ASSERT(ndx >= 0);
	m_vecRuns.deleteNthItem(ndx);
#ifdef BIDI_ENABLED
	#ifndef USE_STATIC_MAP
	_createMapOfRuns();
	#else
	m_bMapDirty = true;
	#endif
#endif

	return true;
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
#ifdef BIDI_ENABLED
	switch(pNewRun->getDirection())
	{
		case 0:
			m_iRunsLTRcount++;
			break;
					
		case 1:
			m_iRunsRTLcount++;
			break;
					
		default:; 	//either -1 for whitespace, or 2 for 'not set'
					//the latter only happens in Unicode mode and is
					//rectified by subsequent call to setDirection
	}
	#ifndef USE_STATIC_MAP		
	_createMapOfRuns(); //#TF update the map
	#else
	m_bMapDirty = true;
	#endif	
#endif
}

void fp_Line::insertRun(fp_Run* pNewRun)
{
	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	pNewRun->setLine(this);

	m_vecRuns.insertItemAt(pNewRun, 0);
#ifdef BIDI_ENABLED
	switch(pNewRun->getDirection())
	{
		case 0:
			m_iRunsLTRcount++;
			break;
					
		case 1:
			m_iRunsRTLcount++;
			break;
					
		default:; 	//either -1 for whitespace, or 2 for 'not set'
					//the latter only happens in Unicode mode and is
					//rectified by subsequent call to setDirection
	}
	#ifndef USE_STATIC_MAP		
	_createMapOfRuns(); //#TF update the map
	#else
	m_bMapDirty = true;
	#endif	
#endif
}

void fp_Line::addRun(fp_Run* pNewRun)
{
	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	pNewRun->setLine(this);

	m_vecRuns.addItem(pNewRun);
#ifdef BIDI_ENABLED
	switch(pNewRun->getDirection())
	{
	case 0:
		m_iRunsLTRcount++;
		break;
					
	case 1:
		m_iRunsRTLcount++;
		break;
					
	default:
		//either -1 for whitespace, or 2 for 'not set'
		//the latter only happens in Unicode mode and is
		//rectified by subsequent call to setDirection
		break;
	}
# ifndef USE_STATIC_MAP		
	_createMapOfRuns();			//#TF update the map
# else
	m_bMapDirty = true;
# endif	
#endif
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
#ifdef BIDI_ENABLED
	switch(pNewRun->getDirection())
	{
		case 0:
			m_iRunsLTRcount++;
			break;
					
		case 1:
			m_iRunsRTLcount++;
			break;
					
		default:; 	//either -1 for whitespace, or 2 for 'not set'
					//the latter only happens in Unicode mode and is
					//rectified by subsequent call to setDirection
	}
	#ifndef USE_STATIC_MAP		
	_createMapOfRuns(); //#TF update the map
	#else
	m_bMapDirty = true;
	#endif	
#endif
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

void fp_Line::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos,
							  bool& bBOL, bool& bEOL)
{
	const int count = m_vecRuns.getItemCount();
	UT_ASSERT(count > 0);
#ifdef BIDI_ENABLED
	fp_Run* pFirstRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(0)); //#TF retrieve first visual run
#else
	fp_Run* pFirstRun = (fp_Run*) m_vecRuns.getNthItem(0);
#endif
	UT_ASSERT(pFirstRun);

	bBOL = false;
	if (x < pFirstRun->getX())
	{
		bBOL = true;

		UT_sint32 y2 = y - pFirstRun->getY() - m_iAscent + pFirstRun->getAscent();
		pFirstRun->mapXYToPosition(0, y2, pos, bBOL, bEOL);
		return;
	}

	// check all of the runs.
	
	fp_Run* pClosestRun = NULL;
	UT_sint32 iClosestDistance = 0;

	for (int i=0; i<count; i++)
	{
#ifdef BIDI_ENABLED
		fp_Run* pRun2 = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(i));  //#TF get i-th visual run
#else
		fp_Run* pRun2 = (fp_Run*) m_vecRuns.getNthItem(i);
#endif
		if (pRun2->canContainPoint() || pRun2->isField())
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
				// Zero-length run. This should only happen with
				// FmtMrk Runs.
				
				UT_ASSERT(FPRUN_FMTMARK == pRun2->getType());

				pRun2->mapXYToPosition(x - pRun2->getX(), y2, pos, bBOL, bEOL);

				return;
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
	if(pClosestRun->isField())
	{
		UT_uint32 width = pClosestRun->getWidth() + 1;
		pClosestRun->mapXYToPosition(width , y2, pos, bBOL, bEOL);
	}
	else
	{
		pClosestRun->mapXYToPosition(x - pClosestRun->getX(), y2, pos, bBOL, bEOL);
	}
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

#if 0 
		// FIXME:jskov We now get lines with height 0. Why is that a
		// problem (i.e., why the assert?)
		UT_ASSERT(iNewHeightLayoutUnits);
#endif

		m_iHeight = iNewHeight;
		m_iHeightLayoutUnits = iNewHeightLayoutUnits;
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
		bool bNeedsClearing = false;

		UT_uint32 i;
		for (i = 0; i < count; i++)
		{
			pRun = (fp_Run*) m_vecRuns.getNthItem(i);

			if(!pRun->isDirty())
			{
				bNeedsClearing = true;

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
	//fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(runIndex);
	fp_Run* pRun; //#TF initialization not needed
	UT_uint32 count = m_vecRuns.getItemCount();

	// Find the first none dirty run.

	UT_uint32 i;
	for(i = runIndex; i < count; i++)
	{
#ifdef BIDI_ENABLED
		pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(i));
#else
		pRun = (fp_Run*) m_vecRuns.getNthItem(i);
#endif

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

#ifdef BIDI_ENABLED
		pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(runIndex));
#else
		pRun = (fp_Run*) m_vecRuns.getNthItem(runIndex);
#endif

		getScreenOffsets(pRun, xoff, yoff);
		UT_sint32 xoffLine, yoffLine;

		m_pContainer->getScreenOffsets(this, xoffLine, yoffLine);

		pRun->getGraphics()->clearArea(xoff, yoff, m_iMaxWidth - (xoff - xoffLine), m_iHeight);
		
		for (i = runIndex; i < count; i++)
		{
#ifdef BIDI_ENABLED
			pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(i));
#else
			pRun = (fp_Run*) m_vecRuns.getNthItem(i);
#endif

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

	m_bNeedsRedraw = false;
	
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
	
	fb_Alignment* pAlignment = getBlock()->getAlignment();
	UT_ASSERT(pAlignment);
	pAlignment->initialize(this);

	const UT_uint32 iCountRuns			= m_vecRuns.getItemCount();
	const UT_sint32 iStartX				= pAlignment->getStartPosition();
	const UT_sint32 iStartXLayoutUnits	= pAlignment->getStartPositionInLayoutUnits();
	const UT_sint32 Screen_resolution =
		getBlock()->getDocLayout()->getGraphics()->getResolution();

	UT_sint32 iX			= iStartX;
	UT_sint32 iXLayoutUnits	= iStartXLayoutUnits;
	bool bLineErased		= false;

	// TODO do we need to do this if iMoveOver is zero ??
	for (UT_uint32 i=0; i<iCountRuns; ++i)
	{
#ifdef BIDI_ENABLED
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(i));
#else
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);
#endif
		if(!bLineErased && iX != pRun->getX())
		{

			// Need to erase some or all of the line depending of Alignment mode.
#ifdef BIDI_ENABLED
            		pRun->setX(iX); //#TF have to set this before the call to erase, since otherwise
                            		//we will be erasing from invalid coordinances.
#endif
			pAlignment->eraseLineFromRun(this, i);
			bLineErased = true;
		}
#ifdef BIDI_ENABLED
		else	
#endif			
		pRun->setX(iX);
		
		if (pRun->getType() == FPRUN_TAB)
		{
			UT_sint32	iPosLayoutUnits;
			eTabType	iTabType;
			eTabLeader	iTabLeader;

			bool bRes = findNextTabStopInLayoutUnits(iXLayoutUnits - iStartXLayoutUnits, iPosLayoutUnits, iTabType, iTabLeader);
			UT_ASSERT(bRes);

			fp_TabRun* pTabRun = static_cast<fp_TabRun*>(pRun);
			fp_Run *pScanRun = NULL;
			int iScanWidth = 0;
			int iScanWidthLayoutUnits = 0;

			// for everybody except the left tab, we need to know how much text is to follow
			switch ( iTabType )
			{
			case FL_TAB_LEFT:
				{
				iXLayoutUnits = (iPosLayoutUnits + iStartXLayoutUnits);
				iX = iXLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
				pTabRun->setWidth(iX - pTabRun->getX());
				}
				break;

			case FL_TAB_CENTER:
				for ( pScanRun = pRun->getNext();
					  pScanRun && pScanRun->getType() != FPRUN_TAB;
					  pScanRun = pScanRun->getNext() )
				{
					iScanWidth += pScanRun->getWidth();
					iScanWidthLayoutUnits += pScanRun->getWidthInLayoutUnits();
				}
	
				if ( iScanWidthLayoutUnits / 2 > iPosLayoutUnits - (iXLayoutUnits - iStartXLayoutUnits) )
					pTabRun->setWidth(0);
				else
				{
					iXLayoutUnits += iPosLayoutUnits - (iXLayoutUnits - iStartXLayoutUnits) - iScanWidthLayoutUnits / 2;
					iX += iPosLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS - (iX - iStartX) - iScanWidth / 2;
					pTabRun->setWidth(iX - pTabRun->getX());
				}
		
				break;

			case FL_TAB_RIGHT:
			{
				for ( pScanRun = pRun->getNext();
					  pScanRun && pScanRun->getType() != FPRUN_TAB;
					  pScanRun = pScanRun->getNext() )
				{
					iScanWidth += pScanRun->getWidth();
					iScanWidthLayoutUnits += pScanRun->getWidthInLayoutUnits();
				}
		
				if ( iScanWidthLayoutUnits > iPosLayoutUnits - (iXLayoutUnits - iStartXLayoutUnits) )
				{
					pTabRun->setWidth(0);
				}
				else
				{
					iXLayoutUnits += iPosLayoutUnits - (iXLayoutUnits - iStartXLayoutUnits) - iScanWidthLayoutUnits;
					iX += iPosLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS - (iX - iStartX) - iScanWidth;
					pTabRun->setWidth(iX - pTabRun->getX());
				}
				break;
			}

			case FL_TAB_DECIMAL:
			{
				UT_UCSChar *pDecimalStr;
				UT_uint32	runLen = 0;

				// the string to search for decimals
				if (UT_UCS_cloneString_char(&pDecimalStr, ".") != true)
				{
					// Out of memory. Now what?
				}

				for ( pScanRun = pRun->getNext();
					  pScanRun && pScanRun->getType() != FPRUN_TAB;
					  pScanRun = pScanRun->getNext() )
				{
					bool foundDecimal = false;

					if(pScanRun->getType() == FPRUN_TEXT)
					{
						UT_sint32 decimalBlockOffset = ((fp_TextRun *)pScanRun)->findCharacter(0, pDecimalStr[0]);

						if(decimalBlockOffset != -1)
						{
							foundDecimal = true;

							runLen = pScanRun->getBlockOffset() - decimalBlockOffset;
						}
					}

					UT_DEBUGMSG(("%s(%d): foundDecimal=%d len=%d iScanWidth=%d \n",
								__FILE__, __LINE__, foundDecimal, pScanRun->getLength()-runLen, iScanWidth));
					if ( foundDecimal )
					{
						if(pScanRun->getType() == FPRUN_TEXT)
						{
							iScanWidth += ((fp_TextRun *)pScanRun)->simpleRecalcWidth(fp_TextRun::Width_type_display, runLen);
							iScanWidthLayoutUnits += ((fp_TextRun *)pScanRun)->simpleRecalcWidth(fp_TextRun::Width_type_layout_units, runLen);
						}
						break; // we found our decimal, don't search any further
					}
					else
					{
						iScanWidth += pScanRun->getWidth();
						iScanWidthLayoutUnits += pScanRun->getWidthInLayoutUnits();
					}
				}
			
				iXLayoutUnits = iPosLayoutUnits - iScanWidthLayoutUnits + iStartXLayoutUnits;
				iX = iPosLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS - iScanWidth + iStartX;
				pTabRun->setWidth(iX - pTabRun->getX());

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
			iXLayoutUnits += pRun->getWidthInLayoutUnits();
			iX += pRun->getWidth();
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

bool fp_Line::recalculateFields(void)
{
	bool bResult = false;
	
	UT_uint32 iNumRuns = m_vecRuns.getItemCount();
	for (UT_uint32 i = 0; i < iNumRuns; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		if (pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun* pFieldRun = (fp_FieldRun*) pRun;
			bool bSizeChanged = pFieldRun->calculateValue();

			bResult = bResult || bSizeChanged;
		}
	}

	return bResult;
}

fp_Run* fp_Line::getLastRun(void) const
{
	const UT_sint32 i = m_vecRuns.getItemCount();
	if(i <= 0)
	{
		fp_Run* pRun = getBlock()->getFirstRun();
		return pRun;
	}
	else
	{
		return ((fp_Run*) m_vecRuns.getLastItem());
	}
}

bool	fp_Line::findNextTabStop(UT_sint32 iStartX, UT_sint32& iPosition, eTabType & iType, eTabLeader & iLeader )
{
	UT_sint32	iTabStopPosition = 0;
	eTabType	iTabStopType = FL_TAB_NONE;
	eTabLeader	iTabStopLeader = FL_LEADER_NONE;

	bool bRes = m_pBlock->findNextTabStop(iStartX + getX(), getX() + getMaxWidth(), iTabStopPosition, iTabStopType, iTabStopLeader);
	UT_ASSERT(bRes);

	iTabStopPosition -= getX();

	if (iTabStopPosition < m_iMaxWidth)
	{
		iPosition = iTabStopPosition;
		iType = iTabStopType;
		iLeader = iTabStopLeader;

		return true;
	}
	else
	{
		return false;
	}
}

bool	fp_Line::findNextTabStopInLayoutUnits(UT_sint32 iStartX, UT_sint32& iPosition, eTabType& iType, eTabLeader& iLeader )
{
	UT_sint32	iTabStopPosition = 0;
	eTabType	iTabStopType = FL_TAB_NONE;
	eTabLeader	iTabStopLeader = FL_LEADER_NONE;

	bool bRes = m_pBlock->findNextTabStopInLayoutUnits(iStartX + getXInLayoutUnits(),
														  getXInLayoutUnits() + getMaxWidthInLayoutUnits(),
														  iTabStopPosition, iTabStopType, iTabStopLeader);
	UT_ASSERT(bRes);

	iTabStopPosition -= getXInLayoutUnits();

	if (iTabStopPosition < m_iMaxWidthLayoutUnits)
	{
		iPosition = iTabStopPosition;
		iType = iTabStopType;
		iLeader = iTabStopLeader;

		return true;
	}
	else
	{
		return false;
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

bool	fp_Line::containsForcedColumnBreak(void) const
{
	if(!isEmpty())
	{
		fp_Run* pRun = getLastRun();
		if (pRun->getType() == FPRUN_FORCEDCOLUMNBREAK)
		{			
			return true;
		}
	}

	return false;
}

bool fp_Line::containsForcedPageBreak(void) const
{
	if (!isEmpty())
	{
		fp_Run* pRun = getLastRun();
		if (pRun->getType() == FPRUN_FORCEDPAGEBREAK)
		{
			return true;
		}
	}
	return false;
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
			UT_sint32	iPos;
			eTabType	iTabType;
			eTabLeader	iTabLeader;

			bool bRes = findNextTabStop(iX, iPos, iTabType, iTabLeader);
			UT_ASSERT(bRes);
			UT_ASSERT(iTabType == FL_TAB_LEFT);

			// TODO -- support all the tabs  shack@uiuc.edu

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
			eTabType iTabType;
			eTabLeader iTabLeader;

			bool bRes = findNextTabStopInLayoutUnits(iX, iPos, iTabType, iTabLeader);
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
	bool bStartFound = false;

	// first calc the width of the line
	for (i=iCountRuns -1 ; i >= 0; i--)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);
		
		if (pRun->getType() == FPRUN_TAB)
		{
			//			UT_ASSERT(false);
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
					bStartFound = true;
				}

			}
		}
		else
		{
			bStartFound = true;
		}
	}

	return iSpaceCount;
}


bool fp_Line::isLastCharacter(UT_UCSChar Character) const
{
	UT_ASSERT(!isEmpty());

	fp_Run *pRun = getLastRun();

	if (pRun->getType() == FPRUN_TEXT)
	{
		fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);

		return pTR->isLastCharacter(Character);
	}

	return false;
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
#ifdef BIDI_ENABLED
	UT_uint32 countOrig = count;
#endif
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

#ifdef BIDI_ENABLED
	if(count != countOrig)
		m_bMapDirty = true;
#endif
}

#ifdef BIDI_ENABLED
//BIDI specific functions

/* creates a map for conversion from visual to logical position */

UT_sint32 fp_Line::_createMapOfRuns()
{
	UT_uint32 i=0;

	if(!m_iRunsRTLcount)
	{
		//UT_DEBUGMSG(("_createMapOfRuns: ltr line only\n"));
		return UT_OK;
	}
    //UT_DEBUGMSG(("createMapOfRuns: this=%x, Owner=%x, Dirty=%x\n", this, s_pMapOwner, m_bMapDirty));
#ifdef USE_STATIC_MAP
    if((s_pMapOwner != this) || (m_bMapDirty))
    {
        //claim the ownership of the map and mark it not dirty
        s_pMapOwner = this;
        m_bMapDirty = false;

#else //if using non-static map, we only check for dirtiness
    if(m_bMapDirty)
    {
        m_bMapDirty = false;
	//just rename the static members to their non-static equivalents, otherwise the code below
	//is fine
#endif
		UT_uint32 count = m_vecRuns.getItemCount();
		if(!count) 
			return UT_OK;  // do not even try to map a line with no runs

        if(count == 1)   //if there is just one run, then make sure that it maps on itself and return
        {
            s_pMapOfRuns[0] = 0;
            return UT_OK;
        }

        if (count >= s_iMapOfRunsSize) //the MapOfRuns member is too small, reallocate
        {
			delete[] s_pMapOfRuns;
			s_iMapOfRunsSize = count + 20; //allow for 20 extra runs, so that we do not have to
                                           //do this immediately again
			s_pMapOfRuns = new UT_sint32[s_iMapOfRunsSize];
			UT_ASSERT(s_pMapOfRuns);
        }

		//make sure that the map is not exessively long;
		if ((count < RUNS_MAP_SIZE) && (s_iMapOfRunsSize > 2* RUNS_MAP_SIZE))
		{
         	delete[] s_pMapOfRuns;
			s_iMapOfRunsSize = RUNS_MAP_SIZE;
			s_pMapOfRuns = new UT_sint32[s_iMapOfRunsSize];
			UT_ASSERT(s_pMapOfRuns);
		}

		//if this is unidirectional rtl text, we can skip the next step
		//we only need to test absence of ltr runs
		if(!m_iRunsLTRcount)
		{
			//UT_DEBUGMSG(("_createMapOfRuns: rtl line only\n"));			
			for(i = 0; i < count/2; i++)
			{
				s_pMapOfRuns[i]= count - i - 1;
				s_pMapOfRuns[count - i - 1] = i;
			}
			if(count % 2)
				s_pMapOfRuns[count/2] = count/2;
		
		}
		else
		{
			//UT_DEBUGMSG(("_createMapOfRuns: bidi line (%d ltr runs, %d rtl runs)\n",m_iRunsLTRcount, m_iRunsRTLcount));        	
			// get the dominant direction of the block and set the MapOfRuns so that all runs
			// that have different direction than the dominant will have value 1,
			// neutral runs -1 and the rest 0
			UT_sint32 RTLdominant = m_pBlock->getDominantDirection();
			UT_sint32 iRunDirection = ((fp_Run*) m_vecRuns.getNthItem(0))->getDirection();
			// run 0 is a special case, we will treat it here, to speed up the loop below
			if(iRunDirection == -1)
				//if this is the very first run, then set it to the paragraph direction,
				//this will make things easier in the next step				
				s_pMapOfRuns[0] = RTLdominant;
			else
				s_pMapOfRuns[0] = (iRunDirection) ? !RTLdominant : RTLdominant;
				
			for (i=1; i < count; i++)
			{
				fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);
				iRunDirection = pRun->getDirection();
				if(iRunDirection == -1)
				{
					s_pMapOfRuns[i] = -1;
				}
				else
				{
					s_pMapOfRuns[i] = (iRunDirection) ? !RTLdominant : RTLdominant;
				}
			}

        	//UT_DEBUGMSG(("fp_Line::_createMapOfRuns(), s_pMapOfRuns{pre}(domDir %d) %d %d %d %d\n", RTLdominant, s_pMapOfRuns[0], s_pMapOfRuns[1], s_pMapOfRuns[2], s_pMapOfRuns[3]));

        	// all sequences of runs that have direction different than the block direction have
        	// to be put into mirror order but first of all we have to convert all directionally
        	// neutral runs into correct direction depending on their context

        	//UT_DEBUGMSG(("pre-map0 %d, %d, %d, %d, %d, %d\n", s_pMapOfRuns[0], s_pMapOfRuns[1], s_pMapOfRuns[2], s_pMapOfRuns[3], s_pMapOfRuns[4], s_pMapOfRuns[5]));

        	UT_uint32 j;
        	for (i=1; i < count; i++)
        	{
            	if(s_pMapOfRuns[i] == -1) //directionally neutral, i.e., whitespace
				{
            		//if we follow a run that is consistent with the direciton of the para, then so will be this
            		//(dont have to worry about the i-1 since this will never happen on position 0
            		//because we have already set the value for any neutral run at pos 0 to that of the
            		//dominant direction
                	if(s_pMapOfRuns[i-1] == 0)
					{
                    	s_pMapOfRuns[i] = 0;
					}
                	//if the preceeding run is foreign, we will have the direction of the following run
                	//except where the following run is end of paragraph marker.
                	//but we have to skip any following whitespace runs
                	else
                	{
                    	j = i + 1;
                    	while ((s_pMapOfRuns[j] == -1) && (j < count))
                        	j++;
                    	/*	last run on the line and the last run before
                    		the formating marker require special treatment.
                    		if the last run on the line is whitespace, it will have the
                    		direction of the preceding run; if it is a formating marker
                    		preceded by a white space, the white space will also get
                    		the direction of the preceding run.
                    	*/
                    	if(j == count || ((j == count -1) && (((fp_Run *) m_vecRuns.getNthItem(j))->getType() == FPRUN_ENDOFPARAGRAPH)))
                			s_pMapOfRuns[i] = s_pMapOfRuns[i-1]; //last run on the line, will have the direction of the preceding run
                    	else
							s_pMapOfRuns[i] = s_pMapOfRuns[j]; //otherwise the direction of the first non-white run we found
                	}
            	}
        	}

        	//UT_DEBUGMSG(("pre-map1 %d, %d, %d, %d, %d, %d\n", s_pMapOfRuns[0], s_pMapOfRuns[1], s_pMapOfRuns[2], s_pMapOfRuns[3], s_pMapOfRuns[4], s_pMapOfRuns[5]));
        	//now we can do the reorganisation
        	for (i=0; i < count; i++)
        	{
				if(s_pMapOfRuns[i] != 0) //foreign direction of text
				{
					j = i;
					while(s_pMapOfRuns[i] && (i < count))
						++i;
					--i;
					for (UT_uint32 n = 0; n <= i - j; n++)
					{
						UT_ASSERT( ((i-n) < count) && ((n+j) < count));
						s_pMapOfRuns[i - n] = n + j;
					}
				}
				else //direction consistent with dominant direction
				{
					s_pMapOfRuns[i] = i;
				}
			}

		
        // if the dominant direction is rtl, the final order of the runs
        // has to be a mirror of the present order

        UT_uint32 temp;

        if(RTLdominant) //we have to switch all the runs around the centre.
		{
            for (i = 0; i < count/2; i++)
			{
                UT_ASSERT((count - i - 1) < count);
                temp = s_pMapOfRuns[i];
                s_pMapOfRuns[i] = s_pMapOfRuns[count - i - 1];
                s_pMapOfRuns[count - i - 1] = temp;
			}
		}
		
	}//if/else only rtl
		
    //UT_DEBUGMSG(("fp_Line::_createMapOfRuns(), s_pMapOfRuns{post} %d %d %d %d\n", s_pMapOfRuns[0], s_pMapOfRuns[1], s_pMapOfRuns[2], s_pMapOfRuns[3]));
    //UT_DEBUGMSG(("count=%d: %d, %d, %d, %d, %d, %d, %d\n", count, s_pMapOfRuns[0], s_pMapOfRuns[1],s_pMapOfRuns[2],s_pMapOfRuns[3],s_pMapOfRuns[4],s_pMapOfRuns[5],s_pMapOfRuns[6]));

    }

    return(UT_OK);
}

/* the following two functions convert the position of a run from logical to visual
   and vice versa */

UT_uint32 fp_Line::_getRunLogIndx(UT_uint32 indx)
{
    //UT_DEBUGMSG(("indx=%d, ItemCount=%d, s_pMapOfRuns[indx]=%d\n", indx, m_vecRuns.getItemCount(),s_pMapOfRuns[indx]));
    UT_ASSERT((m_vecRuns.getItemCount() > indx));

    if(!m_iRunsRTLcount)
    	return(indx);

    _createMapOfRuns();
    return(s_pMapOfRuns[indx]);
}


UT_uint32 fp_Line::_getRunVisIndx(UT_uint32 indx)
{
    UT_ASSERT(m_vecRuns.getItemCount() > indx);

    if(!m_iRunsRTLcount)
    	return(indx);

    UT_uint32 i = 0;
    _createMapOfRuns();
    //UT_DEBUGMSG(("getRunLogIndex: indx=%d, map=%d, %d, %d, %d, %d, %d\n", indx, s_pMapOfRuns[0], s_pMapOfRuns[1],s_pMapOfRuns[2],s_pMapOfRuns[3],s_pMapOfRuns[4],s_pMapOfRuns[5]));
    for(;;)
    {
        UT_ASSERT(m_vecRuns.getItemCount() > i);
        if(s_pMapOfRuns[i] == (UT_sint32)indx)
			return(i);
        ++i;
    }
}

fp_Run * fp_Line::getLastVisRun()
{
    if(!m_iRunsRTLcount)
    	return(getLastRun());

    _createMapOfRuns();
    UT_uint32 count = m_vecRuns.getItemCount();
    UT_ASSERT(count > 0);
    return((fp_Run *) m_vecRuns.getNthItem(s_pMapOfRuns[count - 1]));
}

fp_Run * fp_Line::getFirstVisRun()
{
    if(!m_iRunsRTLcount)
    	return(0);

    _createMapOfRuns();
    return((fp_Run *) m_vecRuns.getNthItem(s_pMapOfRuns[0]));
}

void fp_Line::addDirectionUsed(UT_uint32 dir)
{
	switch(dir)
	{
		case 0:
			m_iRunsLTRcount++;
			break;
			
		case 1:
			m_iRunsRTLcount++;
			break;
		default:;
	}
}

void fp_Line::removeDirectionUsed(UT_uint32 dir)
{
	switch(dir)
	{
		case 0:
			m_iRunsLTRcount--;
			if(m_iRunsLTRcount < 0)
				m_iRunsLTRcount = 0;
			break;
			
		case 1:
			m_iRunsRTLcount--;
			if(m_iRunsRTLcount < 0)
				m_iRunsRTLcount = 0;
			break;
		default:;
	}
}
#endif //BIDI_ENABLED
