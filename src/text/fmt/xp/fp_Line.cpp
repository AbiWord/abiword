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
#include <math.h>
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
	UT_uint32	fp_Line::s_iClassInstanceCounter = 0;
	UT_uint32  * fp_Line::s_pPseudoString = 0;
	UT_uint16  * fp_Line::s_pMapOfRunsL2V = 0;
	UT_uint16  * fp_Line::s_pMapOfRunsV2L = 0;
	UT_Byte    * fp_Line::s_pEmbeddingLevels = 0;
	UT_uint32	fp_Line::s_iMapOfRunsSize = 0;
	fp_Line	* fp_Line::s_pMapOwner = 0;
	#else
	//make sure that any references to the static members are renamed to their non-static versions
	#define s_iMapOfRunsSize m_iMapOfRunsSize
	#define s_pMapOfRuns m_pMapOfRuns
	#endif
#endif

fp_Line::fp_Line()
{
	m_pBlock = NULL;
	m_pContainer = NULL;
	m_iWidth = 0;
	m_iWidthLayoutUnits = 0;
	m_iMaxWidth = 0;
	m_iMaxWidthLayoutUnits = 0;
	m_iClearToPos = 0;
	m_iClearLeftOffset = 0;
	m_iHeight = 0;

	m_iScreenHeight = -1;
	m_iHeightLayoutUnits = 0;
	m_iAscent = 0;
	m_iDescent = 0;
	m_iX = 0;
	m_iXLayoutUnits = 0;
	m_iY = -2000000; // So setY(0) triggers a clearscreen and redraw!
	m_iYLayoutUnits = 0;
	m_pNext = NULL;
	m_pPrev = NULL;
	m_bNeedsRedraw = false;
	
#ifdef BIDI_ENABLED
	m_iRunsRTLcount = 0;
	m_iRunsLTRcount = 0;
	m_bMapDirty = true;	//map that has not been initialized is dirty by deafault

	#ifdef USE_STATIC_MAP
	if(!s_pMapOfRunsL2V)
	{
		s_pMapOfRunsL2V = new UT_uint16[RUNS_MAP_SIZE];
		s_pMapOfRunsV2L = new UT_uint16[RUNS_MAP_SIZE];
		s_pPseudoString    = new UT_uint32[RUNS_MAP_SIZE];
		s_pEmbeddingLevels =  new UT_Byte[RUNS_MAP_SIZE];
		s_iMapOfRunsSize = RUNS_MAP_SIZE;
	}
	++s_iClassInstanceCounter; // this tells us how many instances of Line are out there
				               //we use this to decide whether the above should be
				               //deleted by the destructor
	#else
	m_pMapOfRunsL2V = new UT_uint16[RUNS_MAP_SIZE];
	m_pMapOfRunsV2L = new UT_uint16[RUNS_MAP_SIZE];
	m_pPseudoString    = new UT_uint32[RUNS_MAP_SIZE];
	m_pEmbeddingLevels =  new UT_Byte[RUNS_MAP_SIZE];
	m_iMapOfRunsSize = RUNS_MAP_SIZE;
	#endif

   	UT_ASSERT(s_pMapOfRunsL2V && s_pMapOfRunsV2L && s_pPseudoString && s_pEmbeddingLevels);
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
		delete[] s_pMapOfRunsL2V;
		s_pMapOfRunsL2V = 0;
		
		delete[] s_pMapOfRunsV2L;
		s_pMapOfRunsV2L = 0;
		
		delete[] s_pPseudoString;
		s_pPseudoString = 0;
		
		delete[] s_pEmbeddingLevels;
		s_pEmbeddingLevels = 0;
	}
	#else
	delete[] m_pMapOfRunsL2V;
	m_pMapOfRunsL2V = 0;
	delete[] m_pMapOfRunsV2L;
	m_pMapOfRunsV2L = 0;
	delete[] m_pPseudoString;
	m_pPseudoString = 0;
	delete[] s_pEmbeddingLevels;
	m_pEmbeddingLevels = 0;
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
	updateBackgroundColor();
}

void fp_Line::updateBackgroundColor()
{
	UT_uint32 count = m_vecRuns.getItemCount();
	UT_uint32 i = 0;
	for(i=0;i<count;i++)
		static_cast<fp_Run *>(m_vecRuns.getNthItem(i))->updateBackgroundColor();
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
	removeDirectionUsed(pRun->getDirection());
#endif
	
	
	UT_sint32 ndx = m_vecRuns.findItem(pRun);
	UT_ASSERT(ndx >= 0);
	m_vecRuns.deleteNthItem(ndx);
#ifdef BIDI_ENABLED
	//#ifndef USE_STATIC_MAP
	//_createMapOfRuns();
	//#else
	//m_bMapDirty = true;
	//#endif
#endif

	return true;
}

void fp_Line::insertRunBefore(fp_Run* pNewRun, fp_Run* pBefore)
{
	//UT_DEBUGMSG(("insertRunBefore (line 0x%x, run 0x%x, type %d, dir %d)\n", this, pNewRun, pNewRun->getType(), pNewRun->getDirection()));
	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	UT_ASSERT(pNewRun);
	UT_ASSERT(pBefore);

	pNewRun->setLine(this);
	
	UT_sint32 ndx = m_vecRuns.findItem(pBefore);
	UT_ASSERT(ndx >= 0);

	m_vecRuns.insertItemAt(pNewRun, ndx);
#ifdef BIDI_ENABLED
	addDirectionUsed(pNewRun->getDirection());
	//#ifndef USE_STATIC_MAP		
	//_createMapOfRuns(); //#TF update the map
	//#else
	//m_bMapDirty = true;
	//#endif	
#endif
}

void fp_Line::insertRun(fp_Run* pNewRun)
{
	//UT_DEBUGMSG(("insertRun (line 0x%x, run 0x%x, type %d)\n", this, pNewRun, pNewRun->getType()));
	
	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	pNewRun->setLine(this);

	m_vecRuns.insertItemAt(pNewRun, 0);
#ifdef BIDI_ENABLED
	addDirectionUsed(pNewRun->getDirection());
	//#ifndef USE_STATIC_MAP		
	//_createMapOfRuns(); //#TF update the map
	//#else
	//m_bMapDirty = true;
	//#endif	
#endif
}

void fp_Line::addRun(fp_Run* pNewRun)
{
	//UT_DEBUGMSG(("addRun (line 0x%x, run 0x%x, type %d)\n", this, pNewRun, pNewRun->getType()));

	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	pNewRun->setLine(this);

	m_vecRuns.addItem(pNewRun);
#ifdef BIDI_ENABLED
	addDirectionUsed(pNewRun->getDirection());
	//# ifndef USE_STATIC_MAP		
	//_createMapOfRuns();			//#TF update the map
	//# else
	//m_bMapDirty = true;
	//# endif	
#endif
	setNeedsRedraw();
}

void fp_Line::insertRunAfter(fp_Run* pNewRun, fp_Run* pAfter)
{
	//UT_DEBUGMSG(("insertRunAfter (line 0x%x, run 0x%x, type %d)\n", this, pNewRun, pNewRun->getType()));
	
	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	UT_ASSERT(pNewRun);
	UT_ASSERT(pAfter);
	
	pNewRun->setLine(this);
	
	UT_sint32 ndx = m_vecRuns.findItem(pAfter);
	UT_ASSERT(ndx >= 0);
	
	m_vecRuns.insertItemAt(pNewRun, ndx+1);
#ifdef BIDI_ENABLED
	addDirectionUsed(pNewRun->getDirection());
	//#ifndef USE_STATIC_MAP		
	//_createMapOfRuns(); //#TF update the map
	//#else
	//m_bMapDirty = true;
	//#endif	
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
				// #TF this can also happen legitimately with overstriking text runs
				//UT_ASSERT(FPRUN_FMTMARK == pRun2->getType());

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

/*!
  Set height assigned to line on screen
  \param iHeight Height in screen units

  While recalcHeight computes the height of the line as it will render
  on the screen, the fp_Column does the actual line layout and does so
  with greater accuracy. In particular, the line may be assigned a
  different height on the screen than what it asked for.

  This function allows the line representation to reflect the actual
  screen layout size, which improves the precision of XY/position
  conversion functions.

  \note This function is quite intentionally <b>not</b> called
		setHeight. It should <b>only</b> be called from
		fp_Column::layout.

  \see fp_Column::layout
  Note bye Sevior: This method is causing pixel dirt by making lines smaller
  than their calculated heights!
*/
void fp_Line::setAssignedScreenHeight(UT_sint32 iHeight)
{
	m_iScreenHeight = iHeight;
}

/*!
  Compute the height of the line

  Note that while the line is asked to provide height/width and
  computes this based on its content Runs, it may later be assigned
  additional screen estate by having its height changed. That does not
  affect or override layout details, but increases precision of
  XY/position conversions.

  \fixme I originally put in an assertion that checked that the line
		 was only ever asked to grow in size. But that fired a lot, so
		 it had to be removed. This suggests that we actually try to
		 render stuff to closely on screen - the fp_Line::recalcHeight
		 function should probably be fixed to round height and widths
		 up always. But it gets its data from Runs, so this is not
		 where the problem should be fixed.

  \see fp_Column::layout, fp_Line::setAssignedScreenHeight
*/
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

	// adjust line height to include leading
	double dLineSpace, dLineSpaceLayout;
	fl_BlockLayout::eSpacingPolicy eSpacing;
	m_pBlock->getLineSpacing(dLineSpace, dLineSpaceLayout, eSpacing);
	if(fabs(dLineSpace) < 0.0001)
	{
		xxx_UT_DEBUGMSG(("fp_Line: Set Linespace to 1.0 \n"));
		dLineSpace = 1.0;
	}
	if (eSpacing == fl_BlockLayout::spacing_EXACT)
	{
		xxx_UT_DEBUGMSG(("recalcHeight exact \n"));
		iNewHeight = (UT_sint32) dLineSpace;
		iNewHeightLayoutUnits = (UT_sint32) dLineSpaceLayout;
	}
	else if (eSpacing == fl_BlockLayout::spacing_ATLEAST)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: recalcHeight at least \n"));
		iNewHeight = UT_MAX(iNewHeight, (UT_sint32) dLineSpace);
		iNewHeightLayoutUnits = UT_MAX(iNewHeightLayoutUnits, (UT_sint32) dLineSpaceLayout);
	}
	else
	{
		// multiple
		iNewHeight = (UT_sint32) (iNewHeight * dLineSpace +0.5);
		iNewHeightLayoutUnits = (UT_sint32) (iNewHeightLayoutUnits * dLineSpaceLayout +0.5);
		xxx_UT_DEBUGMSG(("recalcHeight neither dLineSpace = %f newheight =%d m_iScreenHeight =%d m_iHeight= %d\n",dLineSpace,iNewHeight,m_iScreenHeight,m_iHeight));
	}

	if (
		(iOldHeight != iNewHeight)
		|| (iOldAscent != iNewAscent)
		|| (iOldDescent != iNewDescent)
//		|| (iNewHeight > m_iScreenHeight)
		)
	{
		clearScreen();

#if 0
		// FIXME:jskov We now get lines with height 0. Why is that a
		// problem (i.e., why the assert?)
		UT_ASSERT(iNewHeightLayoutUnits);
#endif
		m_iHeight = iNewHeight;
		m_iScreenHeight = -1;	// undefine screen height
		m_iHeightLayoutUnits = iNewHeightLayoutUnits;
		m_iAscent = iNewAscent;
		m_iDescent = iNewDescent;
	}
}

/*!
 * Return a pointer to the run given by runIndex in the  line
\param runIndex the nth run in the line
\returns fp_Run * pRun the pointer to the nth run
*/

fp_Run * fp_Line::getRunFromIndex(UT_uint32 runIndex)
{
	UT_uint32 count = m_vecRuns.getItemCount();
	fp_Run * pRun = NULL;
	if(count > 0 && runIndex < count)
	{
		pRun = (fp_Run *) m_vecRuns.getNthItem(runIndex);
	}
	return pRun;
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
			UT_RGBColor * pClr = pRun->getPageColor();
			// Note: we use getHeight here instead of m_iScreenHeight
			// in case the line is asked to render before it's been
			// assigned a height. Call it robustness, if you want.

			xxx_UT_DEBUGMSG(("ClearToEnd pRun cleartopos = %d yoff = %d height =%d \n",m_iClearToPos,yoffLine,getHeight()));
			pRun->getGraphics()->fillRect(*pClr,xoffLine - m_iClearLeftOffset, yoffLine, m_iClearToPos + m_iClearLeftOffset, getHeight());
//
// Sevior: I added this for robustness.
//
			m_pBlock->setNeedsRedraw();
			setNeedsRedraw();

		}
	}
	
}

/*!
 * This method clears a line from the run given to the end of the line.
\param fp_Run * pRun
*/
void fp_Line::clearScreenFromRunToEnd(fp_Run * ppRun)
{
	fp_Run * pRun = NULL;
	UT_uint32 count =  m_vecRuns.getItemCount();
	if(count > 0)
	{
		UT_sint32 k = m_vecRuns.findItem((void *) ppRun);
		if(k>=0)
		{
			UT_uint32 runIndex = (UT_uint32) k;
			UT_sint32 xoff, yoff;

#ifdef BIDI_ENABLED
			pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(runIndex));
#else
			pRun = (fp_Run*) m_vecRuns.getNthItem(runIndex);
#endif
			//
			// Handle case where character extend behind the left side
			// like italic Times New Roman f. Clear a litle bit before if
			// there is clear screen there
			//
			UT_sint32 j = runIndex - 1;
			fp_Run * pPrev = NULL;
			if(j>=0)
			{
				pPrev = (fp_Run *) m_vecRuns.getNthItem(j);
			}
			UT_sint32 leftClear = 0;
			while(j >= 0 && pPrev != NULL && pPrev->getLength() == 0)
			{
				pPrev = (fp_Run *) m_vecRuns.getNthItem(j);
				j--;
			}
			leftClear = pRun->getDescent();
			if(j>=0 && pPrev != NULL && pPrev->getType() == FPRUN_TEXT)
				leftClear = 0;
			if(j>= 0 && pPrev != NULL && pPrev->getType() == FPRUN_FIELD)
				leftClear = 0;
			if(j>=0 && pPrev != NULL && pPrev->getType() == FPRUN_IMAGE)
				leftClear = 0;
			getScreenOffsets(pRun, xoff, yoff);
			UT_sint32 xoffLine, yoffLine;
		
			m_pContainer->getScreenOffsets(this, xoffLine, yoffLine);
			if(xoff == xoffLine)
				leftClear = pRun->getDescent();

			UT_RGBColor * pClr = pRun->getPageColor();

			pRun->getGraphics()->fillRect(*pClr,xoff - leftClear, yoff, m_iClearToPos + leftClear - (xoff - xoffLine) , getHeight());
//
// Sevior: I added this for robustness.
//
			getBlock()->setNeedsRedraw();
			setNeedsRedraw();
			UT_uint32 i;
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
}


/*!
 * This method clears a line from the first non-dirty run at the given index
 * to the end of the line.
\param UT_uint32 runIndex
*/

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

		//
		// Handle case where character extend behind the left side
		// like italic Times New Roman f. Clear a litle bit before if
		// there is clear screen there
		//
		UT_sint32 j = runIndex - 1;
		fp_Run * pPrev = pRun->getPrev();
		UT_sint32 leftClear = 0;
		while(j >= 0 && pPrev != NULL && pPrev->getLength() == 0)
		{
			pPrev = (fp_Run *) m_vecRuns.getNthItem(j);
			j--;
		}
		leftClear = pRun->getDescent();
		if(j>=0 && pPrev != NULL && pPrev->getType() == FPRUN_TEXT)
			leftClear = 0;
		if(j>=0 && pPrev != NULL && pPrev->getType() == FPRUN_FIELD)
			leftClear = 0;
		if(j>=0 && pPrev != NULL && pPrev->getType() == FPRUN_IMAGE)
			leftClear = 0;

		getScreenOffsets(pRun, xoff, yoff);
		UT_sint32 xoffLine, yoffLine;
		UT_sint32 oldheight = getHeight();
		recalcHeight();
		UT_ASSERT(oldheight == getHeight());
		m_pContainer->getScreenOffsets(this, xoffLine, yoffLine);
		fp_Line * pPrevLine = getPrevLineInSection();
		if(pPrevLine != NULL)
		{
			UT_sint32 xPrev=0;
			UT_sint32 yPrev=0;
			fp_Run * pLastRun = pPrevLine->getLastRun();
			if(pLastRun != NULL)
			{
				pPrevLine->getScreenOffsets(pLastRun,xPrev,yPrev);
				if((leftClear >0) && (yPrev > 0) && (yPrev == yoffLine))
				{
					leftClear = 0;
				}
			}
		}
		if(xoff == xoffLine)
				leftClear = pRun->getDescent();
		UT_RGBColor * pClr = pRun->getPageColor();
		pRun->getGraphics()->fillRect(*pClr,xoff - leftClear, yoff, m_iClearToPos  + leftClear - (xoff - xoffLine) , getHeight());
//
// Sevior: I added this for robustness.
//
		getBlock()->setNeedsRedraw();
		setNeedsRedraw();
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


void fp_Line::setNeedsRedraw(void)
{
	m_bNeedsRedraw = true;
	m_pBlock->setNeedsRedraw();
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
	//line can be wider than the max width due to trailing spaces
	//UT_ASSERT(m_iWidth <= m_iMaxWidth);
	
	xxx_UT_DEBUGMSG(("SEVIOR: Drawing line in line pG \n"));
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

		FP_RUN_TYPE rType = pRun->getType();

		// for these two types of runs, we want to draw for the
		// entire line-width on the next line. see bug 1301
		if (rType == FPRUN_FORCEDCOLUMNBREAK ||
			rType == FPRUN_FORCEDPAGEBREAK)
		{
			// there's no need to reset anything - a page or column
			// break is logically always the last thing on a line or
			// a page
			da.xoff = my_xoff;
		}
		else
		{
			da.xoff += pRun->getX();
		}

		da.yoff += pRun->getY();
		pRun->draw(&da);

		da.xoff -= pRun->getX();
		da.yoff -= pRun->getY();
	}
}

void fp_Line::draw(dg_DrawArgs* pDA)
{
	int count = m_vecRuns.getItemCount();
	
	xxx_UT_DEBUGMSG(("SEVIOR: Drawing line in line pDA \n"));

	pDA->yoff += m_iAscent;

	for (int i=0; i<count; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);
		FP_RUN_TYPE rType = pRun->getType();

		dg_DrawArgs da = *pDA;

		// for these two types of runs, we want to draw for the
		// entire line-width on the next line. see bug 1301
		if (rType == FPRUN_FORCEDCOLUMNBREAK ||
			rType == FPRUN_FORCEDPAGEBREAK)
		{
			UT_sint32 my_xoff = 0, my_yoff = 0;
			m_pContainer->getScreenOffsets(this, my_xoff, my_yoff);			
			da.xoff = my_xoff;
		}
		else
		{
			da.xoff += pRun->getX();
		}
		da.yoff += pRun->getY();
		pRun->draw(&da);
	}
}

void fp_Line::layout(void)
{
	xxx_UT_DEBUGMSG(("fp_Line::layout called\n"));
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
		//UT_UCSChar c;  ((fp_TextRun *)pRun)->getCharacter(0, c);
		//char cc = (char) c;
		xxx_UT_DEBUGMSG(("i: %d, starts %c ", i, cc));
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
			xxx_UT_DEBUGMSG(("erased line from vis. run %d\n", i));
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

			pTabRun->setLeader(iTabLeader);
			pTabRun->setTabType(iTabType);
			// for everybody except the left tab, we need to know how much text is to follow
			switch ( iTabType )
			{
			case FL_TAB_LEFT:
				{
					iXLayoutUnits = (iPosLayoutUnits + iStartXLayoutUnits);
					iX = iXLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
#ifdef BIDI_ENABLED
//the else branch is probably wrong, have a look at the right tab
					if(pTabRun->getVisDirection() == FRIBIDI_TYPE_RTL)
						pTabRun->setWidth(m_iWidth - (iX - pTabRun->getX()));
					else
						pTabRun->setWidth(iX - pTabRun->getX());
#else
					pTabRun->setWidth(iX - pTabRun->getX());
#endif
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
				{
					iXLayoutUnits = (iPosLayoutUnits + iStartXLayoutUnits);
					iX = iXLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
					pTabRun->setWidth(iX - pTabRun->getX());
				}
			break;

			default:
				UT_ASSERT(UT_NOT_IMPLEMENTED);
			};

		}
		else
		{
			iXLayoutUnits += pRun->getWidthInLayoutUnits();
			iX += pRun->getWidth();
			
			xxx_UT_DEBUGMSG(("run[%d] (type %d) width=%d\n", i,pRun->getType(),pRun->getWidth()));
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

fp_Run* fp_Line::getLastTextRun(void) const
{
	const UT_sint32 i = m_vecRuns.getItemCount();
	fp_Run * pRun = NULL;
	if(i <= 0)
	{
		pRun = getBlock()->getFirstRun();
		return pRun;
	}
	else
	{
		pRun = (fp_Run*) m_vecRuns.getLastItem();
		while(pRun != NULL && pRun->getType() != FPRUN_TEXT)
		{
			pRun = pRun->getPrev();
		}
		if(pRun == NULL)
		{
			pRun = getBlock()->getFirstRun();
		}
		return pRun;
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
	UT_ASSERT(iMaxWidth > 0);
	fl_DocSectionLayout * pSL =  getBlock()->getDocSectionLayout();
	UT_ASSERT(pSL);
	if(pSL->getNumColumns() > 1)
	{
		m_iClearToPos = iMaxWidth + pSL->getColumnGap();
		m_iClearLeftOffset = pSL->getColumnGap() -1;
	}
	else
	{
		m_iClearToPos = iMaxWidth + pSL->getRightMargin() - 2;
		m_iClearLeftOffset = pSL->getLeftMargin() -1;
	}

	iMaxWidth -= m_pBlock->getRightMargin();
	iMaxWidth -= m_pBlock->getLeftMargin();
	m_iClearToPos -= m_pBlock->getLeftMargin();
	if (isFirstLineInBlock())
	{
		iMaxWidth -= m_pBlock->getTextIndent();
	}

	// Check that there's actually room for content
	UT_ASSERT(iMaxWidth > 0);

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
	
	// Check that there's actually room for content
	UT_ASSERT(iMaxWidth > 0);

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
	//UT_DEBUGMSG(("coalesceRuns (line 0x%x)\n", this));
	UT_uint32 count = m_vecRuns.getItemCount();
	for (UT_sint32 i=0; i < (UT_sint32)(count-1); i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem((UT_uint32)i);

		if (pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);
			if (pTR->canMergeWithNext())
			{
				pTR->mergeWithNext();
				count--;
				i--; //test the newly merged run with the next
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
		//UT_DEBUGMSG(("calculateWidthOfLine: run[%d] (type %d) width=%d total=%d\n", i, pRun->getType(), pRun->getWidth(),iX));
	}
    // this is a wrong assert, since line can include trailing spaces
    // that are out of the margins.
	//UT_ASSERT(iX <= m_iMaxWidth);

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
	//UT_DEBUGMSG(("splitRunsAtSpaces (line 0x%x)\n", this));

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
#ifdef BIDI_ENABLED
				addDirectionUsed(pRun->getDirection(),false);
#endif				
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
#ifdef BIDI_ENABLED
			addDirectionUsed(pRun->getDirection(),false);
#endif				
			pTR->split(iSpacePosition + 1);
		}
	}

#ifdef BIDI_ENABLED
	if(count != countOrig)
	{
		m_bMapDirty = true;
		_createMapOfRuns();
	}
#endif
}

#ifdef BIDI_ENABLED
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// BIDI specific functions
//
/*!
	Creates a map for conversion from visual to logical position of runs on the line.
	\param void
	
	\note This function is BIDI-exclusive.
*/
UT_sint32 fp_Line::_createMapOfRuns()
{
	UT_uint32 i=0;

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
#endif
		UT_uint32 count = m_vecRuns.getItemCount();
		if(!count)
			return UT_OK;  // do not even try to map a line with no runs

		if(count == 1)   //if there is just one run, then make sure that it maps on itself and return
		{
			s_pMapOfRunsL2V[0] = 0;
			s_pMapOfRunsV2L[0] = 0;
			return UT_OK;
		}

		if (count >= s_iMapOfRunsSize) //the MapOfRuns member is too small, reallocate
		{
			delete[] s_pMapOfRunsL2V;
			delete[] s_pMapOfRunsV2L;
			delete[] s_pPseudoString;
			delete[] s_pEmbeddingLevels;
			
			s_iMapOfRunsSize = count + 20; //allow for 20 extra runs, so that we do not have to
			                               //do this immediately again
			s_pMapOfRunsL2V = new UT_uint16[s_iMapOfRunsSize];
			s_pMapOfRunsV2L = new UT_uint16[s_iMapOfRunsSize];
			s_pPseudoString    = new UT_uint32[RUNS_MAP_SIZE];
			s_pEmbeddingLevels =  new UT_Byte[RUNS_MAP_SIZE];
			
			
			UT_ASSERT(s_pMapOfRunsL2V && s_pMapOfRunsV2L && s_pPseudoString && s_pEmbeddingLevels);
		}

		//make sure that the map is not exessively long;
		if ((count < RUNS_MAP_SIZE) && (s_iMapOfRunsSize > 2* RUNS_MAP_SIZE))
		{
		 	delete[] s_pMapOfRunsL2V;
		 	delete[] s_pMapOfRunsV2L;
			delete[] s_pPseudoString;
			delete[] s_pEmbeddingLevels;
		 	
			s_iMapOfRunsSize = RUNS_MAP_SIZE;
			
			s_pMapOfRunsL2V = new UT_uint16[s_iMapOfRunsSize];
			s_pMapOfRunsV2L = new UT_uint16[s_iMapOfRunsSize];
			s_pPseudoString    = new UT_uint32[RUNS_MAP_SIZE];
			s_pEmbeddingLevels =  new UT_Byte[RUNS_MAP_SIZE];

			
			UT_ASSERT(s_pMapOfRunsL2V && s_pMapOfRunsV2L && s_pPseudoString && s_pEmbeddingLevels);
		}
		
		if(!m_iRunsRTLcount)
		{
			xxx_UT_DEBUGMSG(("_createMapOfRuns: ltr line only (line 0x%x)\n", this));
			for (i = 0; i < count; i++)
			{
				//the map is actually never used, we only need to set the
				//the visual directions for all our runs to 0
				//s_pMapOfRunsL2V[i] = i;
				//s_pMapOfRunsV2L[i] = i;
				((fp_Run*) m_vecRuns.getNthItem(i))->setVisDirection(FRIBIDI_TYPE_LTR);
			}
			return UT_OK;
		}
		else

		//if this is unidirectional rtl text, we just fill the map sequentially
		//from back to start
		if(!m_iRunsLTRcount)
		{
			xxx_UT_DEBUGMSG(("_createMapOfRuns: rtl line only (line 0x%x)\n", this));			
			for(i = 0; i < count/2; i++)
			{
				s_pMapOfRunsL2V[i]= count - i - 1;
				s_pMapOfRunsV2L[i]= count - i - 1;
				s_pMapOfRunsL2V[count - i - 1] = i;
				s_pMapOfRunsV2L[count - i - 1] = i;
				((fp_Run*) m_vecRuns.getNthItem(i))->setVisDirection(FRIBIDI_TYPE_RTL);
			}
			
			if(count % 2)   //the run in the middle
			{
				s_pMapOfRunsL2V[count/2] = count/2;
				s_pMapOfRunsV2L[count/2] = count/2;
				((fp_Run*) m_vecRuns.getNthItem(count/2))->setVisDirection(FRIBIDI_TYPE_RTL);

			}
		
		}
		else
		{
			/*
				This is a genuine bidi line, so we have to go the full way.
			*/
			xxx_UT_DEBUGMSG(("_createMapOfRuns: bidi line (%d ltr runs, %d rtl runs, line 0x%x)\n", m_iRunsLTRcount, m_iRunsRTLcount, this));			

			// create a pseudo line string
			/*
				The fribidi library takes as its input a Unicode string, which
				it then analyses. Rather than trying to construct a string for
				the entire line, we create a short one in which each run
				is represented by a single character of a same direction as
				that of the entire run.
			*/
			UT_sint32 iRunDirection;
						
			for(i = 0; i < count; i++)
			{
				iRunDirection = ((fp_Run*) m_vecRuns.getNthItem(i))->getDirection();
				switch(iRunDirection)
				{
					case FRIBIDI_TYPE_LTR : s_pPseudoString[i] = (FriBidiChar) 'a'; break;
					case FRIBIDI_TYPE_RTL : s_pPseudoString[i] = (FriBidiChar) 0x05d0; break;
					//case FRIBIDI_TYPE_WL
					//case FRIBIDI_TYPE_WR
					case FRIBIDI_TYPE_EN  : s_pPseudoString[i] = (FriBidiChar) '0'; break;
					case FRIBIDI_TYPE_ES  : s_pPseudoString[i] = (FriBidiChar) '/'; break;
					case FRIBIDI_TYPE_ET  : s_pPseudoString[i] = (FriBidiChar) '#'; break;
					case FRIBIDI_TYPE_AN  : s_pPseudoString[i] = (FriBidiChar) 0x0660; break;
					case FRIBIDI_TYPE_CS  : s_pPseudoString[i] = (FriBidiChar) ','; break;
					case FRIBIDI_TYPE_BS  : s_pPseudoString[i] = (FriBidiChar) 0x000A; break;
					case FRIBIDI_TYPE_SS  : s_pPseudoString[i] = (FriBidiChar) 0x000B; break;
					case FRIBIDI_TYPE_WS  : s_pPseudoString[i] = (FriBidiChar) ' '; break;
					case FRIBIDI_TYPE_AL  : s_pPseudoString[i] = (FriBidiChar) 0x061B; break;
					case FRIBIDI_TYPE_NSM : s_pPseudoString[i] = (FriBidiChar) 0x0300; break;
					case FRIBIDI_TYPE_LRE : s_pPseudoString[i] = (FriBidiChar) 0x202A; break;
					case FRIBIDI_TYPE_RLE : s_pPseudoString[i] = (FriBidiChar) 0x202B; break;
					case FRIBIDI_TYPE_LRO : s_pPseudoString[i] = (FriBidiChar) 0x202D; break;
					case FRIBIDI_TYPE_RLO : s_pPseudoString[i] = (FriBidiChar) 0x202E; break;
					case FRIBIDI_TYPE_PDF : s_pPseudoString[i] = (FriBidiChar) 0x202C; break;
					case FRIBIDI_TYPE_ON  : s_pPseudoString[i] = (FriBidiChar) '!'; break;

				}
				xxx_UT_DEBUGMSG(("fp_Line::_createMapOfRuns: pseudo char 0x%x\n",s_pPseudoString[i]));
			}

			FriBidiCharType iBlockDir = m_pBlock->getDominantDirection();
			// NB !!! the current version of fribidi confuses
			// the L2V and V2L arrays !!! (or we do, does it matter?)
			
			fribidi_log2vis(/* input */
		     s_pPseudoString,
		     count,
		     &iBlockDir,
		     /* output */
		     /*FriBidiChar *visual_str*/ NULL,
		     s_pMapOfRunsV2L,
		     s_pMapOfRunsL2V,
		     s_pEmbeddingLevels
		     );

		     //the only other thing that remains is to pass the visual
		     //directions down to the runs.		
		     for (i=0; i<count;i++)
		     {
				((fp_Run*) m_vecRuns.getNthItem(i))->setVisDirection(s_pEmbeddingLevels[i]%2 ? FRIBIDI_TYPE_RTL : FRIBIDI_TYPE_LTR);
				xxx_UT_DEBUGMSG(("L2V %d, V2L %d, emb. %d\n", s_pMapOfRunsL2V[i],s_pMapOfRunsV2L[i],s_pEmbeddingLevels[i]));
		     }
		}//if/else only rtl
	}

	return(UT_OK);
}

/* the following two functions convert the position of a run from logical to visual
   and vice versa */

UT_uint32 fp_Line::_getRunLogIndx(UT_uint32 indx)
{
	UT_ASSERT((m_vecRuns.getItemCount() > indx));

	if(!m_iRunsRTLcount)
		return(indx);

	_createMapOfRuns();
	return(s_pMapOfRunsV2L[indx]);
}


UT_uint32 fp_Line::_getRunVisIndx(UT_uint32 indx)
{
	UT_ASSERT(m_vecRuns.getItemCount() > indx);

	if(!m_iRunsRTLcount)
		return(indx);

	_createMapOfRuns();
	return(s_pMapOfRunsL2V[indx]);
}

fp_Run * fp_Line::getLastVisRun()
{
	if(!m_iRunsRTLcount)
		return(getLastRun());

	_createMapOfRuns();
	UT_uint32 count = m_vecRuns.getItemCount();
	UT_ASSERT(count > 0);
	return((fp_Run *) m_vecRuns.getNthItem(s_pMapOfRunsV2L[count - 1]));
}

fp_Run * fp_Line::getFirstVisRun()
{
	if(!m_iRunsRTLcount)
		return(0);

	_createMapOfRuns();
	return((fp_Run *) m_vecRuns.getNthItem(s_pMapOfRunsV2L[0]));
}

////////////////////////////////////////////////////////////////////
//
// the following three functions are used to keep track of rtl and
// ltr runs on the line; this allows us to avoid the fullblown
// bidi algorithm for ltr-only and rtl-only lines
//
// the parameter bRefreshMap specifies whether the map of runs should
// be recalculated; if you call any of these functions in a loop
// and do not need the refreshed map inside of that loop, set it to
// false and then after the loop set m_bMapDirty true and run
// _createMapOfRuns (when outside of fp_Line, make sure that only
// the last call gets true)

void fp_Line::addDirectionUsed(FriBidiCharType dir, bool bRefreshMap)
{
	switch(dir)
	{
		case FRIBIDI_TYPE_LTR:
		case FRIBIDI_TYPE_EN:
			m_iRunsLTRcount++;
			//UT_DEBUGMSG(("increased LTR run count [%d, this=0x%x]\n", m_iRunsLTRcount, this));
			break;
			
		case FRIBIDI_TYPE_RTL:
			m_iRunsRTLcount++;
			//UT_DEBUGMSG(("increased RTL run count [%d, this=0x%x]\n", m_iRunsRTLcount, this));
			break;
		default:;
	}
	if(bRefreshMap && dir != FRIBIDI_TYPE_UNSET)
	{
		m_bMapDirty = true;
		_createMapOfRuns();
	}
}

void fp_Line::removeDirectionUsed(FriBidiCharType dir, bool bRefreshMap)
{
	switch(dir)
	{
		case FRIBIDI_TYPE_LTR:
		case FRIBIDI_TYPE_EN:
			m_iRunsLTRcount--;
			//UT_DEBUGMSG(("decreased LTR run count (fp_Line::removeDirectionUsed) [%d, this=0x%x]\n", m_iRunsLTRcount, this));
			
			if(m_iRunsLTRcount < 0)
				m_iRunsLTRcount = 0;
			break;
			
		case FRIBIDI_TYPE_RTL:
			m_iRunsRTLcount--;
			//UT_DEBUGMSG(("decreased RTL run count (fp_Line::removeDirectionUsed) [%d, this=0x%x]\n", m_iRunsRTLcount, this));
			
			if(m_iRunsRTLcount < 0)
				m_iRunsRTLcount = 0;
			break;
		default:;
	}
	if(bRefreshMap && dir != FRIBIDI_TYPE_UNSET)
	{
		m_bMapDirty = true;
		_createMapOfRuns();
	}
}

void fp_Line::changeDirectionUsed(FriBidiCharType oldDir, FriBidiCharType newDir, bool bRefreshMap)
{
	if(oldDir == newDir)
		return;
		
	switch(newDir)
	{
		case FRIBIDI_TYPE_LTR:
		case FRIBIDI_TYPE_EN:
			m_iRunsLTRcount++;
			//UT_DEBUGMSG(("increased LTR run count [%d, this=0x%x]\n", m_iRunsLTRcount, this));
			break;
			
		case FRIBIDI_TYPE_RTL:
			m_iRunsRTLcount++;
			//UT_DEBUGMSG(("increased RTL run count [%d, this=0x%x]\n", m_iRunsRTLcount, this));
			break;
		default:;
	}

	switch(oldDir)
	{
		case FRIBIDI_TYPE_LTR:
		case FRIBIDI_TYPE_EN:
			m_iRunsLTRcount--;
			//UT_DEBUGMSG(("decreased LTR run count (fp_Line::removeDirectionUsed) [%d, this=0x%x]\n", m_iRunsLTRcount, this));
			
			if(m_iRunsLTRcount < 0)
				m_iRunsLTRcount = 0;
			break;
			
		case FRIBIDI_TYPE_RTL:
			m_iRunsRTLcount--;
			//UT_DEBUGMSG(("decreased RTL run count (fp_Line::removeDirectionUsed) [%d, this=0x%x]\n", m_iRunsRTLcount, this));
			
			if(m_iRunsRTLcount < 0)
				m_iRunsRTLcount = 0;
			break;
		default:;
	}
		
	if(bRefreshMap && newDir != FRIBIDI_TYPE_UNSET)
	{
		m_bMapDirty = true;
		_createMapOfRuns();
	}
}

#endif //BIDI_ENABLED
