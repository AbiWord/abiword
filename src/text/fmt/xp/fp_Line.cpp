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
#include "ap_Prefs.h"

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

	
	
	UT_sint32 ndx = m_vecRuns.findItem(pRun);
	UT_ASSERT(ndx >= 0);
	m_vecRuns.deleteNthItem(ndx);

#ifdef BIDI_ENABLED
	removeDirectionUsed(pRun->getDirection());
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
	
	UT_sint32 my_xoff = 0, my_yoff = 0;
	
	m_pContainer->getScreenOffsets(this, my_xoff, my_yoff);
	xxx_UT_DEBUGMSG(("SEVIOR: Drawing line in line pG, my_yoff=%d \n",my_yoff));
	if(((my_yoff < -32000) || (my_yoff > 32000)) && pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
//
// offscreen don't bother.
//
		return;
	}
	dg_DrawArgs da;
	
	da.yoff = my_yoff + m_iAscent;
	da.xoff = my_xoff;
	da.pG = pG;

	int count = m_vecRuns.getItemCount();
	
	for (int i=0; i < count; i++)
	{
		// NB !!! In the BiDi build drawing has to be done in the logical
		// order, otherwise overstriking characters cannot be seen
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
		// NB !!! In the BiDi build drawing has to be done in the logical
		// order, otherwise overstriking characters cannot be seen
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

//this is a helper function for getRunWith; it works out working direction and
//which tabs to use from the alignment
//it is public, because it is only used when getRunWidth is called from
//outside of the class

void fp_Line::getWorkingDirectionAndTabstops(FL_WORKING_DIRECTION &eWorkingDirection, FL_WHICH_TABSTOP &eUseTabStop) const
{
	fb_Alignment* pAlignment = m_pBlock->getAlignment();
	UT_ASSERT(pAlignment);
    FB_AlignmentType eAlignment 	  = pAlignment->getType();

#ifdef BIDI_ENABLED
	// find out the direction of the paragraph
	FriBidiCharType iDomDirection = m_pBlock->getDominantDirection();
#endif
        	
	eWorkingDirection = WORK_FORWARD;
	eUseTabStop = USE_NEXT_TABSTOP;
	
    // now from the current alignment work out which way we need to process the line
    // and the corresponding starting positions

    switch (eAlignment)
    {
        case FB_ALIGNMENT_LEFT:
#ifdef BIDI_ENABLED
			if(iDomDirection == FRIBIDI_TYPE_RTL)
	  			eUseTabStop = USE_PREV_TABSTOP;
	  		else
#endif
				eUseTabStop = USE_NEXT_TABSTOP;
			
            eWorkingDirection = WORK_FORWARD;
            break;

        case FB_ALIGNMENT_RIGHT:
#ifdef BIDI_ENABLED
			if(iDomDirection == FRIBIDI_TYPE_RTL)
	  			eUseTabStop = USE_NEXT_TABSTOP;
	  		else
#endif
				eUseTabStop = USE_PREV_TABSTOP;
	  			
            eWorkingDirection = WORK_BACKWARD;
            break;

        case FB_ALIGNMENT_CENTER:
            eWorkingDirection = WORK_FORWARD;
            eUseTabStop = USE_FIXED_TABWIDTH;
            break;

        case FB_ALIGNMENT_JUSTIFY:
#ifdef BIDI_ENABLED
            if(iDomDirection == FRIBIDI_TYPE_RTL)
                eWorkingDirection = WORK_BACKWARD;
            else
#endif
                eWorkingDirection = WORK_FORWARD;
            eUseTabStop = USE_NEXT_TABSTOP;
            break;

        default:
            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    }
	return;
}

// this function will calculate correct width for all runs, including
// tab runs, from a visual processing index, it return point to the run
// at the index for the callers convenience.

//NB. in bidi more iXLayoutUnits is a logical coordinance
//however, in _calculateWidthOfRun iX and iXLayoutUnits are visual,
//so we need to do a conversion here ...

fp_Run* fp_Line::calculateWidthOfRun(UT_sint32 &iWidthLayoutUnits, UT_uint32 iIndxVisual, FL_WORKING_DIRECTION eWorkingDirection, FL_WHICH_TABSTOP eUseTabStop)
{
	const UT_uint32 iCountRuns		  = m_vecRuns.getItemCount();
	UT_sint32 iXLreal, iXreal;
	static UT_sint32 Screen_resolution = m_pBlock->getDocLayout()->getGraphics()->getResolution();
	iXreal = iWidthLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
	
	//work out the real index based on working direction
	UT_uint32 iIndx;
   	iIndx = eWorkingDirection == WORK_FORWARD ? iIndxVisual : iCountRuns - iIndxVisual - 1;
    		
#ifdef BIDI_ENABLED
	// of course, the loop is running in visual order, but the vector is
	// in logical order
	fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iIndx));
#else
	fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(iIndx);
#endif

#ifdef BIDI_ENABLED
	// find out the direction of the paragraph
	FriBidiCharType iDomDirection = m_pBlock->getDominantDirection();
	
	if(iDomDirection == FRIBIDI_TYPE_RTL)
	{
		iXLreal = m_iMaxWidthLayoutUnits - iWidthLayoutUnits;
		iXreal 	= m_iMaxWidth - iXreal;
	}
	else
#endif
	{
		iXLreal = iWidthLayoutUnits;
		//iXreal = iXreal;
	}

	xxx_UT_DEBUGMSG(("fp_Line::calculateWidthOfRun: \n"
				 "       iXreal %d, iXLreal %d, iIndxVisual %d, iCountRuns %d,\n"
				 "       eWorkingDirection %d, eUseTabStop %d,\n"
				         ,iXreal, iXLreal, iIndxVisual, iCountRuns,
						 eWorkingDirection, eUseTabStop
				));
				
	_calculateWidthOfRun(iXreal,
						 iXLreal,
						 pRun,
						 iIndxVisual,
						 iCountRuns,
						 eWorkingDirection,
						 eUseTabStop
#ifdef BIDI_ENABLED
						 ,iDomDirection
#endif						
						 );
						
	xxx_UT_DEBUGMSG(("new iXreal %d, iXLreal %d\n", iXreal, iXLreal));
#ifdef BIDI_ENABLED
	if(iDomDirection == FRIBIDI_TYPE_RTL)
	{
		iWidthLayoutUnits = m_iMaxWidthLayoutUnits - iXLreal;
	}
	else
#endif
	{
		iWidthLayoutUnits = iXLreal;
	}
	
	return pRun;
}

// private version of the above, which expect both the index and run prointer
// to be passed to it.
inline void fp_Line::_calculateWidthOfRun(	UT_sint32 &iX,
									UT_sint32 &iXLayoutUnits,
									fp_Run * pRun,
									UT_uint32 iIndx,
									UT_uint32 iCountRuns,
									FL_WORKING_DIRECTION eWorkingDirection,
									FL_WHICH_TABSTOP eUseTabStop
#ifdef BIDI_ENABLED
									,FriBidiCharType iDomDirection
#endif
									)
{
	if(!pRun)
		return;

	switch(pRun->getType())
	{
		case FPRUN_TAB:
		{
			// a fixed width fraction of the font ascent which we will use for centered alignment
			// i.e, width = font_ascent * iFixedWidthMlt / iFixedWidthDiv
			const UT_uint32 iFixedWidthMlt = 2;
			const UT_uint32 iFixedWidthDiv = 1;
			static UT_sint32 Screen_resolution = m_pBlock->getDocLayout()->getGraphics()->getResolution();
			UT_uint32 iWidth = 0;
			fp_TabRun* pTabRun = static_cast<fp_TabRun*>(pRun);
	
			// now we handle any Tab runs that are not meant to use a fixed width
			if (eUseTabStop != USE_FIXED_TABWIDTH)
			{
				//if we are using the tabstops, we go through the hoops,
				UT_sint32	iPosLayoutUnits;
				eTabType	iTabType;
				eTabLeader	iTabLeader;
				bool bRes;
				
			// now find the tabstop for this tab, depending on whether we
			// are to use next or previous tabstop				
			if(eUseTabStop == USE_NEXT_TABSTOP)
			{
#ifdef BIDI_ENABLED
				if(iDomDirection == FRIBIDI_TYPE_RTL)
				{
					bRes = findNextTabStopInLayoutUnits(m_iMaxWidthLayoutUnits - iXLayoutUnits, iPosLayoutUnits, iTabType, iTabLeader);
					iPosLayoutUnits = m_iMaxWidthLayoutUnits - iPosLayoutUnits;
				}
				else
#endif
					bRes = findNextTabStopInLayoutUnits(iXLayoutUnits, iPosLayoutUnits, iTabType, iTabLeader);
			}
			else
#ifdef BIDI_ENABLED
			if(iDomDirection == FRIBIDI_TYPE_RTL)
			{
				bRes = findPrevTabStopInLayoutUnits(m_iMaxWidthLayoutUnits - iXLayoutUnits, iPosLayoutUnits, iTabType, iTabLeader);
				iPosLayoutUnits = m_iMaxWidthLayoutUnits - iPosLayoutUnits;
			}
			else
#endif
				bRes = findPrevTabStopInLayoutUnits(iXLayoutUnits, iPosLayoutUnits, iTabType, iTabLeader);
					
				
			UT_ASSERT(bRes);

			fp_Run *pScanRun = NULL;
			UT_sint32 iScanWidth = 0;
			UT_sint32 iScanWidthLayoutUnits = 0;

			pTabRun->setLeader(iTabLeader);
			pTabRun->setTabType(iTabType);

			// we need to remember what the iX was				
			UT_sint32 iXprev;
			iXprev = iX;		
				
			xxx_UT_DEBUGMSG(("pf_Line::_calculateWidthOfRun(): tab: iX %d, iXLayout %d, iPosLayout %d\n",iX,iXLayoutUnits,iPosLayoutUnits));
			
#ifdef BIDI_ENABLED				
			FriBidiCharType iVisDirection = pTabRun->getVisDirection();
#endif
			switch ( iTabType )
			{
				case FL_TAB_LEFT:
#ifdef BIDI_ENABLED
					if(iVisDirection == FRIBIDI_TYPE_LTR && iDomDirection == FRIBIDI_TYPE_LTR)
#endif						
					{
						iXLayoutUnits = iPosLayoutUnits;
						iX = iXLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
						iWidth = abs(iX - iXprev);
						xxx_UT_DEBUGMSG(("left tab (ltr para), iX %d, iWidth %d\n", iX,iWidth));
					}
#ifdef BIDI_ENABLED
					else
					{
		    	    	iScanWidth = 0;
			    	    iScanWidthLayoutUnits = 0;
						for ( UT_uint32 j = iIndx+1; j < iCountRuns; j++ )
						{
							UT_uint32 iJ;
							iJ = eWorkingDirection == WORK_FORWARD ? j : iCountRuns - j - 1;
						
							pScanRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iJ));
							if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
								break;
					
							iScanWidth += pScanRun->getWidth();
							iScanWidthLayoutUnits += pScanRun->getWidthInLayoutUnits();
						}
			
						if ( iScanWidthLayoutUnits > abs(iPosLayoutUnits - iXLayoutUnits))
						{
							iWidth = 0;
						}
						else
						{
							iXLayoutUnits += iPosLayoutUnits - iXLayoutUnits - (UT_sint32)eWorkingDirection * iScanWidthLayoutUnits;
							iX += iPosLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS - iX - (UT_sint32)eWorkingDirection * iScanWidth;
							iWidth = abs(iX - iXprev);
						}
					}
#endif
					break;

					case FL_TAB_CENTER:
					{
		    		    iScanWidth = 0;
		    	    	iScanWidthLayoutUnits = 0;
						for ( UT_uint32 j = iIndx+1; j < iCountRuns; j++ )
						{
							UT_uint32 iJ;
							iJ = eWorkingDirection == WORK_FORWARD ? j : iCountRuns - j - 1;
#ifdef BIDI_ENABLED
							pScanRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iJ));
#else							
							pScanRun = (fp_Run*) m_vecRuns.getNthItem(iJ);
#endif
							if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
								break;
							iScanWidth += pScanRun->getWidth();
							iScanWidthLayoutUnits += pScanRun->getWidthInLayoutUnits();
						}
	
						if ( iScanWidthLayoutUnits / 2 > abs(iPosLayoutUnits - iXLayoutUnits))
							iWidth = 0;
						else
						{
							iXLayoutUnits += iPosLayoutUnits - iXLayoutUnits - (UT_sint32)eWorkingDirection * iScanWidthLayoutUnits / 2;
							iX += iPosLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS - iX - (UT_sint32)eWorkingDirection * iScanWidth / 2;
							iWidth = abs(iX - iXprev);
						}
						break;
					}
			
					case FL_TAB_RIGHT:
#ifdef BIDI_ENABLED
						if(iVisDirection == FRIBIDI_TYPE_RTL && iDomDirection == FRIBIDI_TYPE_RTL)
						{
							iXLayoutUnits = iPosLayoutUnits;
							iX = iXLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
							iWidth = abs(iX - iXprev);
							xxx_UT_DEBUGMSG(("right tab (rtl para), iX %d, width %d\n", iX,pTabRun->getWidth()));
						}
						else
#endif
						{
							xxx_UT_DEBUGMSG(("right tab (ltr para), ii %d\n",ii));
				    	    iScanWidth = 0;
			    		    iScanWidthLayoutUnits = 0;
							for ( UT_uint32 j = iIndx+1; j < iCountRuns; j++ )
							{
								UT_uint32 iJ;
								iJ = eWorkingDirection == WORK_FORWARD ? j : iCountRuns - j - 1;
								xxx_UT_DEBUGMSG(("iJ %d\n", iJ));
#ifdef BIDI_ENABLED
								pScanRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iJ));
#else
								pScanRun = (fp_Run*) m_vecRuns.getNthItem(iJ);
#endif								
								if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
									break;
								iScanWidth += pScanRun->getWidth();
								iScanWidthLayoutUnits += pScanRun->getWidthInLayoutUnits();
							}
							
							xxx_UT_DEBUGMSG(("iScanWidthLayoutUnits %d, iPosLayoutUnits %d, iXLayoutUnits %d\n",iScanWidthLayoutUnits,iPosLayoutUnits,iXLayoutUnits));
		
							if ( iScanWidthLayoutUnits > abs(iPosLayoutUnits - iXLayoutUnits))
							{
								iWidth = 0;
							}
							else
							{
								iXLayoutUnits += iPosLayoutUnits - iXLayoutUnits - (UT_sint32)eWorkingDirection * iScanWidthLayoutUnits;
								iX += iPosLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS - iX - (UT_sint32)eWorkingDirection * iScanWidth;
								iWidth = abs(iX - iXprev);
							}
				
						}
						break;

					case FL_TAB_DECIMAL:
					{
						UT_UCSChar *pDecimalStr;
						UT_uint32	runLen = 0;

						// the string to search for decimals
						if (UT_UCS_cloneString_char(&pDecimalStr, ".") != true)
						{
							// Out of memory. Now what?
						}
            	
		    		    iScanWidth = 0;
		    	    	iScanWidthLayoutUnits = 0;
						for ( UT_uint32 j = iIndx+1; j < iCountRuns; j++ )
						{
							UT_uint32 iJ;
							iJ = eWorkingDirection == WORK_FORWARD ? j : iCountRuns - j - 1;
#ifdef BIDI_ENABLED						
							pScanRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iJ));
#else
							pScanRun = (fp_Run*) m_vecRuns.getNthItem(iJ);
#endif
							if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
								break;
				
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

							xxx_UT_DEBUGMSG(("%s(%d): foundDecimal=%d len=%d iScanWidth=%d \n",
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
			            	
						iXLayoutUnits = iPosLayoutUnits - (UT_sint32)eWorkingDirection * iScanWidthLayoutUnits;
						iX = iPosLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS - (UT_sint32)eWorkingDirection * iScanWidth;
						iWidth = abs(iX - iXprev);
						FREEP(pDecimalStr);	
						break;
					}
		
					case FL_TAB_BAR:
					{
						iXLayoutUnits = iPosLayoutUnits;
						iX = iXLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
						iWidth = abs(iX - iXprev);
					}
					break;

					default:
						UT_ASSERT(UT_NOT_IMPLEMENTED);
				}; //switch

				// if working backwards, set the new X coordinance
				// and decide if line needs erasing
			}
			else //this is not a Tab run, or we are using fixed width tabs
			{
				iWidth = pRun->getAscent()*iFixedWidthMlt / iFixedWidthDiv;
	
			
				xxx_UT_DEBUGMSG(("run[%d] (type %d) width=%d\n", i,pRun->getType(),iWidth));
			}
		
			pTabRun->setWidth(iWidth);
			return;
		}
#ifdef BIDI_ENABLED
		case FPRUN_TEXT:
		{
      		if(static_cast<fp_TextRun*>(pRun)->getUseContextGlyphs())
				pRun->recalcWidth();
			//and fall through to default
		}
#endif

		default:
		{
			if(eWorkingDirection == WORK_FORWARD)
			{
				iXLayoutUnits += pRun->getWidthInLayoutUnits();
				iX += pRun->getWidth();
			}
			else
			{
				iXLayoutUnits -= pRun->getWidthInLayoutUnits();
				iX -= pRun->getWidth();
			}
					xxx_UT_DEBUGMSG(("fp_Line::calculateWidthOfRun (non-tab [0x%x, type %d, dir %d]): width %d\n",
							pRun,pRun->getType(),pRun->getDirection(),pRun->getWidth()));
			return;
		}
		
	}//switch run type
		
}

#if 1
void fp_Line::layout(void)
{
	/*
		This would be very simple if it was not for the Tabs :-); this is what
		we will do:
		
		(1) determine alignment for this line; if the alignment is right
			we will have to work out the width of the runs in reverse
			visual order, because we know only the physical position of the
			end of the line;
			
		(2) in bidi mode we determine the direction of paragraph, since it
			dictates what the alignment of the last line of a justified
			praragraph will be.
			
		(3) if the direction of our paragraph corresponds to the alignment
			(i.e., left to ltr) we will process the tab stops the normal way,
			i.e, we will look for the next tabstop for each tab run. If the
			alignment is contrary to the direction of paragraph we will lookup
			the previous tabstop instead, since we cannot shav the text in
			the normal direction.
	*/

	// first of all, work out the height
   	recalcHeight();
   	
    // get current alignment; note that we cannot initialize the alignment
    // at this stage, (and chances are we will not need to anyway), because
    // we have to first calculate the widths of our tabs
	fb_Alignment* pAlignment = m_pBlock->getAlignment();
	UT_ASSERT(pAlignment);
    FB_AlignmentType eAlignment 	  = pAlignment->getType();

	const UT_uint32 iCountRuns		  = m_vecRuns.getItemCount();
	UT_sint32 iStartX 				  = 0;
	UT_sint32 iStartXLayoutUnits 	  = 0;

#ifdef BIDI_ENABLED
	// find out the direction of the paragraph
	FriBidiCharType iDomDirection = m_pBlock->getDominantDirection();
#endif
        	
	// a variable to keep the processing direction
	// NB: it is important that WORK_FORWARD is set to 1 and
	// WORK_BACKWARD to -1; this gives us a simple way to convert
	// addition to substraction by mulplying by the direction
	FL_WORKING_DIRECTION eWorkingDirection = WORK_FORWARD;

    // a variable that will tell us how to interpret the tabstops
	FL_WHICH_TABSTOP eUseTabStop = USE_NEXT_TABSTOP;
	
    // now from the current alignment work out which way we need to process the line
    // and the corresponding starting positions
        
    switch (eAlignment)
    {
        case FB_ALIGNMENT_LEFT:
#ifdef BIDI_ENABLED
			if(iDomDirection == FRIBIDI_TYPE_RTL)
	  			eUseTabStop = USE_PREV_TABSTOP;
	  		else
#endif
				eUseTabStop = USE_NEXT_TABSTOP;
			
            eWorkingDirection = WORK_FORWARD;
            break;
            
        case FB_ALIGNMENT_RIGHT:
#ifdef BIDI_ENABLED
			if(iDomDirection == FRIBIDI_TYPE_RTL)
	  			eUseTabStop = USE_NEXT_TABSTOP;
	  		else
#endif
				eUseTabStop = USE_PREV_TABSTOP;
	  			
            eWorkingDirection = WORK_BACKWARD;
			iStartX = m_iMaxWidth;
		    iStartXLayoutUnits = m_iMaxWidthLayoutUnits;
            break;

        case FB_ALIGNMENT_CENTER:
            eWorkingDirection = WORK_FORWARD;
            eUseTabStop = USE_FIXED_TABWIDTH;
            // we will pretend the line starts at pos 0, work out the width
            // and then shift it by the necessary amount to the right
            iStartX = 0;
            iStartXLayoutUnits = 0;
            break;
            
        case FB_ALIGNMENT_JUSTIFY:
#ifdef BIDI_ENABLED
            if(iDomDirection == FRIBIDI_TYPE_RTL)
            {
                eWorkingDirection = WORK_BACKWARD;
				iStartX = m_iMaxWidth;
			    iStartXLayoutUnits = m_iMaxWidthLayoutUnits;
            }
            else
#endif
            {
                eWorkingDirection = WORK_FORWARD;
            }
            eUseTabStop = USE_NEXT_TABSTOP;
            break;

        default:
            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    }

    //now variables to keep track of our progress along the line
	UT_sint32 iX			= iStartX;
	UT_sint32 iXLayoutUnits	= iStartXLayoutUnits;
	
	//variables to keep information about how to erase the line once we are
	//in position to do so
	bool bLineErased		= false;
	UT_uint32 iIndxToEraseFrom = 0;

#if 0 //def DEBUG

	//some extra but lengthy degug stuff
	char *al;
	char left[] = "left";
	char right[]= "right";
	char cent[] = "center";
	char just[] = "justified";

    switch (eAlignment)
    {
        case FB_ALIGNMENT_LEFT:
        	al = left;
            break;

        case FB_ALIGNMENT_RIGHT:
        	al = right;
            break;

        case FB_ALIGNMENT_CENTER:
        	al = cent;
            break;

        case FB_ALIGNMENT_JUSTIFY:
        	al = just;
            break;

        default:
            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    }

    char *d;
    char fwd[] = "forward";
    char bck[] = "backward";

    if(eWorkingDirection == WORK_FORWARD)
    	d = fwd;
    else
    	d = bck;
    	
    char *t;
    char next[] = "next";
    char prev[] = "prev";
    char fxd[] = "fixed width";

    switch (eUseTabStop)
    {
    	case USE_NEXT_TABSTOP:
    		t = next;
    		break;
    	case USE_PREV_TABSTOP:
    		t = prev;
    		break;
    	case USE_FIXED_TABWIDTH:
    		t = fxd;
    		break;
    	default:
    		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    }
    	
	xxx_UT_DEBUGMSG(("fp_Line::layout(), this = 0x%x\n"
				 "       alignment [%s], working direction [%s], using tabstops [%s]\n"
				 "       fixed width multiplier %d/%d\n"
				 "       iStartX    = %d, iStartXLayout = %d \n"
				 "       iCountRuns = %d\n",
				 this, al, d, t, iFixedWidthMlt, iFixedWidthDiv,
				 iStartX, iStartXLayoutUnits, iCountRuns
	));
		
#endif //end of the debug stuff


	// now we work our way through the runs on this line
	xxx_UT_DEBUGMSG(("fp_Line::layout ------------------- \n"));
 	for (UT_uint32 ii=0; ii<iCountRuns; ++ii)
	{
		//work out the real index based on working direction
  		UT_uint32 iIndx;
    	iIndx = eWorkingDirection == WORK_FORWARD ? ii : iCountRuns - ii - 1;
    		
#ifdef BIDI_ENABLED
		// of course, the loop is running in visual order, but the vector is
		// in logical order
  		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iIndx));
#else
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(iIndx);
#endif

		// if we are working from the left, we want to set the
		// X coordinance now; if from the right we will do it
		// after we have got to the width worked out
		// also, decide whether erasure is needed
		if(eWorkingDirection == WORK_FORWARD)
		{
			if(!bLineErased && pRun->getX()!= iX)
			{
				bLineErased = true;				
				iIndxToEraseFrom = iIndx;
			}
			pRun->setX(iX);
		}
		xxx_UT_DEBUGMSG(("fp_Line::layout: iX %d, iXL %d, ii %d, iCountRuns %d\n"
					 "       run type %d\n",
					iX, iXLayoutUnits, ii, iCountRuns, pRun->getType()));
		_calculateWidthOfRun(iX,
							 iXLayoutUnits,
							 pRun,
							 ii,
							 iCountRuns,
							 eWorkingDirection,
							 eUseTabStop
#ifdef BIDI_ENABLED
							,iDomDirection
#endif		
							);
			
		// if working backwards, set the new X coordinance
		// and decide if line needs erasing
		if(eWorkingDirection == WORK_BACKWARD)
		{
			if(!bLineErased && pRun->getX()!= iX)
			{
				bLineErased = true;				
				iIndxToEraseFrom = iIndx;
			}
			pRun->setX(iX);
		}
	} //for
	
	//now we are ready to deal with the alignment
	//we only need to do this in case of centered lines and justified lines,
	// so we will move it below
	//pAlignment->initialize(this);

	if(eAlignment == FB_ALIGNMENT_JUSTIFY)
	{
		pAlignment->initialize(this);
		
		// now we need to shift the x-coordinances to reflect the new widths
		// of the spaces
		iStartX = pAlignment->getStartPosition();
		for (UT_uint32 k = 0; k < iCountRuns; k++)
		{
#ifdef BIDI_ENABLED
			UT_uint32 iK = (eWorkingDirection == WORK_FORWARD) ? k : iCountRuns - k - 1;
	  		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iK));
#else
			fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(k);
#endif
			UT_ASSERT(pRun);

#ifdef BIDI_ENABLED
			if(eWorkingDirection == WORK_BACKWARD)
			{
				iStartX -= pRun->getWidth();
				pRun->setX(iStartX);
			}
			else
#endif
			{
				pRun->setX(iStartX);
				iStartX += pRun->getWidth();
			}
		}
	}
	else
	//if the line is centered we will have to shift the iX of each run
	//since we worked on the assumption that the line starts at 0
	//only now are we in the position to enquire of the alignment what
	//the real starting position should be
	if(eAlignment == FB_ALIGNMENT_CENTER)
	{
		pAlignment->initialize(this);
		iStartX = pAlignment->getStartPosition();
		for (UT_uint32 k = 0; k < iCountRuns; k++)
		{
#ifdef BIDI_ENABLED
	  		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(k));
#else
			fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(k);
#endif
			UT_ASSERT(pRun);
			pRun->setX(pRun->getX() + iStartX);
		}
	}
	else
	if(eAlignment == FB_ALIGNMENT_RIGHT)
	{   // we have not dealt with trailing spaces ...
		pAlignment->initialize(this);
		iStartX = pAlignment->getStartPosition();
#ifdef BIDI_ENABLED
  		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(0));
#else
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(0);
#endif
		if(pRun->getX() != iStartX)
			for (UT_uint32 k = 0; k < iCountRuns; k++)
			{
#ifdef BIDI_ENABLED
	  			pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(k));
#else
				pRun = (fp_Run*) m_vecRuns.getNthItem(k);
#endif
				UT_ASSERT(pRun);
				pRun->setX(iStartX);
				iStartX += pRun->getWidth();
			}	
	}
	//not entirely sure about this, by now all the coordinances have
	//changed, but it seems to work :-)
	//pAlignment->eraseLineFromRun(this, iIndxToEraseFrom);
	clearScreenFromRunToEnd(iIndxToEraseFrom);
}
#else
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
			
			/*
				Bit more complicated in the bidi case by the fact that we
				have to deal with the runs in their visual order
			*/
						
#ifdef BIDI_ENABLED
			UT_DEBUGMSG(("pf_Line::layout(): tab: iXLayout %d, iStartx %d, iPosLayout %d\n",iXLayoutUnits,iStartXLayoutUnits,iPosLayoutUnits));
			FriBidiCharType iVisDirection = pTabRun->getVisDirection();
			FriBidiCharType iDomDirection = m_pBlock->getDominantDirection();
			switch ( iTabType )
			{
			case FL_TAB_LEFT:
				if(iVisDirection == FRIBIDI_TYPE_LTR && iDomDirection == FRIBIDI_TYPE_LTR)
				{
					iXLayoutUnits = (iPosLayoutUnits + iStartXLayoutUnits);
					iX = iXLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
					pTabRun->setWidth(iX - pTabRun->getX());
					UT_DEBUGMSG(("left tab (ltr para), iX %d, width %d\n", iX,pTabRun->getWidth()));
				}
				else
				{
				
					for ( UT_uint32 j = i+1; j < iCountRuns; j++ )
					{
						pScanRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(j));
						if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
							break;
					
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
				}
			break;

			case FL_TAB_CENTER:
			{
				for ( UT_uint32 j = i+1; j < iCountRuns; j++ )
				{
					pScanRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(j));
					if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
						break;
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
			}
			
			case FL_TAB_RIGHT:
				if(iVisDirection == FRIBIDI_TYPE_RTL && iDomDirection == FRIBIDI_TYPE_RTL)
				{
					iXLayoutUnits = (iPosLayoutUnits + iStartXLayoutUnits);
					iX = iXLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
					pTabRun->setWidth(iX - pTabRun->getX());
					UT_DEBUGMSG(("right tab (rtl para), iX %d, width %d\n", iX,pTabRun->getWidth()));
				}
				else
				{
					for ( UT_uint32 j = i+1; j < iCountRuns; j++ )
					{
						pScanRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(j));
						if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
							break;
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
			
				}
				break;

			case FL_TAB_DECIMAL:
			{
				UT_UCSChar *pDecimalStr;
				UT_uint32	runLen = 0;

				// the string to search for decimals
				if (UT_UCS_cloneString_char(&pDecimalStr, ".") != true)
				{
					// Out of memory. Now what?
				}

				for ( UT_uint32 j = i+1; j < iCountRuns; j++ )
				{
					pScanRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(j));
					if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
						break;
					
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
#else			
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
				{
					iXLayoutUnits = (iPosLayoutUnits + iStartXLayoutUnits);
					iX = iXLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
					pTabRun->setWidth(iX - pTabRun->getX());
				}
			break;

			default:
				UT_ASSERT(UT_NOT_IMPLEMENTED);
			};
#endif

		}
		else
		{
			iXLayoutUnits += pRun->getWidthInLayoutUnits();
			iX += pRun->getWidth();
			
			xxx_UT_DEBUGMSG(("run[%d] (type %d) width=%d\n", i,pRun->getType(),pRun->getWidth()));
		}
	}
}
#endif

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

	//has to be <=
	if (iTabStopPosition <= m_iMaxWidth)
	{
		iPosition = iTabStopPosition;
		iType = iTabStopType;
		iLeader = iTabStopLeader;

		return true;
	}
	else
	{
		UT_DEBUGMSG(("fp_Line::findNextTabStop: iStartX %d, m_iMaxWidth %d\n"
					 "          iPosition %d, iTabStopPosition %d, iType %d, iLeader %d\n",
					 iStartX, m_iMaxWidth,iPosition, iTabStopPosition,(UT_sint32)iType, (UT_sint32)iLeader));
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

	if (iTabStopPosition <= m_iMaxWidthLayoutUnits)
	{
		iPosition = iTabStopPosition;
		iType = iTabStopType;
		iLeader = iTabStopLeader;

		return true;
	}
	else
	{
		UT_DEBUGMSG(("fp_Line::findNextTabStopLayout: iStartX %d, m_iMaxWidthLayoutUnits %d\n"
					 "           iPosition %d, iTabStopPosition %d,iType %d, iLeader %d\n",
					 iStartX, m_iMaxWidthLayoutUnits,iPosition, iTabStopPosition,(UT_sint32)iType, (UT_sint32)iLeader));
		return false;
	}
}

bool	fp_Line::findPrevTabStop(UT_sint32 iStartX, UT_sint32& iPosition, eTabType & iType, eTabLeader & iLeader )
{
	UT_sint32	iTabStopPosition = 0;
	eTabType	iTabStopType = FL_TAB_NONE;
	eTabLeader	iTabStopLeader = FL_LEADER_NONE;

	bool bRes = m_pBlock->findPrevTabStop(iStartX + getX(), getX() + getMaxWidth(), iTabStopPosition, iTabStopType, iTabStopLeader);
	UT_ASSERT(bRes);

	iTabStopPosition -= getX();

	if (iTabStopPosition <= m_iMaxWidth)
	{
		iPosition = iTabStopPosition;
		iType = iTabStopType;
		iLeader = iTabStopLeader;

		return true;
	}
	else
	{
		UT_DEBUGMSG(("fp_Line::findPrevTabStop: iStartX %d, m_iMaxWidth %d\n"
					 "          iPosition %d, iTabStopPosition %d, iType %d, iLeader %d\n",
					 iStartX, m_iMaxWidth,iPosition, iTabStopPosition, (UT_sint32)iType, (UT_sint32)iLeader));
		return false;
	}
}

bool	fp_Line::findPrevTabStopInLayoutUnits(UT_sint32 iStartX, UT_sint32& iPosition, eTabType& iType, eTabLeader& iLeader )
{
	UT_sint32	iTabStopPosition = 0;
	eTabType	iTabStopType = FL_TAB_NONE;
	eTabLeader	iTabStopLeader = FL_LEADER_NONE;

	bool bRes = m_pBlock->findPrevTabStopInLayoutUnits(iStartX + getXInLayoutUnits(),
														  getXInLayoutUnits() + getMaxWidthInLayoutUnits(),
														  iTabStopPosition, iTabStopType, iTabStopLeader);
	UT_ASSERT(bRes);

	iTabStopPosition -= getXInLayoutUnits();

	if (iTabStopPosition <= m_iMaxWidthLayoutUnits)
	{
		iPosition = iTabStopPosition;
		iType = iTabStopType;
		iLeader = iTabStopLeader;

		return true;
	}
	else
	{
		UT_DEBUGMSG(("fp_Line::findPrevTabStopLayout: iStartX %d, m_iMaxWidthLayoutUnits %d\n"
					 "         iPosition %d, iTabStopPosition %d, iType %d, iLeader %d\n",
					 iStartX, m_iMaxWidthLayoutUnits,iPosition, iTabStopPosition,(UT_sint32)iType, (UT_sint32)iLeader));
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

#if 0
// since the tab width's have already been calculated by a call to layout
// this is not necessary (I gather this is an ancient piece of code
// put here before the layout function was implemented)

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
#endif
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

#if 0		
// since the tab width's have already been calculated by a call to layout
// this is not necessary (I gather this is an ancient piece of code
// put here before the layout function was implemented)

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
#endif
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
			UT_DEBUGMSG(("_createMapOfRuns: rtl line only (line 0x%x)\n", this));			
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
			UT_DEBUGMSG(("_createMapOfRuns: bidi line (%d ltr runs, %d rtl runs, line 0x%x)\n", m_iRunsLTRcount, m_iRunsRTLcount, this));			

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
					case FRIBIDI_TYPE_AL  : s_pPseudoString[i] = (FriBidiChar) 0x062D; break;
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
			
			fribidi_log2vis(/* input */
		     s_pPseudoString,
		     count,
		     &iBlockDir,
		     /* output */
		     /*FriBidiChar *visual_str*/ NULL,
		     s_pMapOfRunsL2V,
		     s_pMapOfRunsV2L,
		     s_pEmbeddingLevels
		     );

		     //the only other thing that remains is to pass the visual
		     //directions down to the runs.		
		     for (i=0; i<count;i++)
		     {
				((fp_Run*) m_vecRuns.getNthItem(i))->setVisDirection(s_pEmbeddingLevels[i]%2 ? FRIBIDI_TYPE_RTL : FRIBIDI_TYPE_LTR);
				xxx_UT_DEBUGMSG(("L2V %d, V2L %d, emb. %d [run 0x%x]\n", s_pMapOfRunsL2V[i],s_pMapOfRunsV2L[i],s_pEmbeddingLevels[i],m_vecRuns.getNthItem(i)));
		     }
		}//if/else only rtl
	}

	return(UT_OK);
}

/* the following two functions convert the position of a run from logical to visual
   and vice versa */

UT_uint32 fp_Line::_getRunLogIndx(UT_uint32 indx)
{
#ifdef DEBUG
	UT_uint32 iCount = m_vecRuns.getItemCount();
	if(iCount <= indx)
		UT_DEBUGMSG(("fp_Line::_getRunLogIndx: indx %d, iCount %d\n", indx,iCount));
#endif	
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
		case FRIBIDI_TYPE_AL:
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
		case FRIBIDI_TYPE_AL:
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
		case FRIBIDI_TYPE_AL:
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
		case FRIBIDI_TYPE_AL:
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
