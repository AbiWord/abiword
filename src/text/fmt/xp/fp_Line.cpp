/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
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
#include <locale.h> 			// localeconv()
#include "ut_types.h"	// for FREEP

#include "fl_DocLayout.h"
#include "fl_FootnoteLayout.h"
#include "fl_BlockLayout.h"
#include "fb_Alignment.h"
#include "fp_Column.h"
#include "fp_TableContainer.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"
#include "fp_Page.h"
#include "fv_View.h"
#include "fl_SectionLayout.h"
#include "fl_TableLayout.h"
#include "gr_DrawArgs.h"
#include "gr_Graphics.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ap_Prefs.h"

#ifdef USE_STATIC_MAP
//initialize the static members of the class
FriBidiChar     * fp_Line::s_pPseudoString = 0;
FriBidiStrIndex * fp_Line::s_pMapOfRunsL2V = 0;
FriBidiStrIndex * fp_Line::s_pMapOfRunsV2L = 0;
FriBidiLevel    * fp_Line::s_pEmbeddingLevels = 0;
UT_uint32 	      fp_Line::s_iMapOfRunsSize = 0;
fp_Line         * fp_Line::s_pMapOwner = 0;
#else
//make sure that any references to the static members are renamed to their non-static versions
#define s_iMapOfRunsSize m_iMapOfRunsSize
#define s_pMapOfRuns m_pMapOfRuns
#endif

#define STATIC_BUFFER_INITIAL 150
#define SCALE_TO_SCREEN (double)getGraphics()->getResolution() / UT_LAYOUT_UNITS

UT_sint32 * fp_Line::s_pOldXs = NULL;
UT_uint32   fp_Line::s_iOldXsSize = 0;
UT_uint32	fp_Line::s_iClassInstanceCounter = 0;

fp_Line::fp_Line(fl_SectionLayout * pSectionLayout) : 
	fp_Container(FP_CONTAINER_LINE, pSectionLayout),
	m_pBlock(NULL),
	m_iWidth(0),
	m_iMaxWidth(0),
	m_iClearToPos(0),
	m_iClearLeftOffset(0),
	m_iHeight(0),
	m_iScreenHeight(-1),
	m_iAscent(0),
	m_iDescent(0),
	m_iX(0),
	m_iY(-2000000), // So setY(0) triggers a clearscreen and redraw!
		            // I do not like this at all; we have no business
		            // of clearing at fictional coordinances

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	m_iWidthLayoutUnits(0),
	m_iMaxWidthLayoutUnits(0),
	m_iHeightLayoutUnits(0),
	m_iXLayoutUnits(0),
		m_iYLayoutUnits(-20000000),
#endif
	//m_bRedoLayout(true),
	m_bNeedsRedraw(false),
	m_bMapDirty(true), //map that has not been initialized is dirty by deafault
	m_iRunsRTLcount(0),
	m_iRunsLTRcount(0),
	m_bIsCleared(true),
	m_bContainsFootnoteRef(false)
{
	if(!s_iClassInstanceCounter)
	{
		s_pOldXs = new UT_sint32[STATIC_BUFFER_INITIAL];
		UT_ASSERT(s_pOldXs);
		s_iOldXsSize = STATIC_BUFFER_INITIAL;
	}

	#ifdef USE_STATIC_MAP
	if(!s_pMapOfRunsL2V)
	{
		s_pMapOfRunsL2V = new FriBidiStrIndex[RUNS_MAP_SIZE];
		s_pMapOfRunsV2L = new FriBidiStrIndex[RUNS_MAP_SIZE];
		s_pPseudoString    = new FriBidiChar[RUNS_MAP_SIZE];
		s_pEmbeddingLevels =  new FriBidiLevel[RUNS_MAP_SIZE];
		s_iMapOfRunsSize = RUNS_MAP_SIZE;
	}
    #else
	m_pMapOfRunsL2V = new FriBidiStrIndex[RUNS_MAP_SIZE];
	m_pMapOfRunsV2L = new FriBidiStrIndex[RUNS_MAP_SIZE];
	m_pPseudoString    = new FriBidiChar[RUNS_MAP_SIZE];
	m_pEmbeddingLevels =  new FriBidiLevel[RUNS_MAP_SIZE];
	m_iMapOfRunsSize = RUNS_MAP_SIZE;
    #endif

	UT_ASSERT(s_pMapOfRunsL2V && s_pMapOfRunsV2L && s_pPseudoString && s_pEmbeddingLevels);

	++s_iClassInstanceCounter; // this tells us how many instances of Line are out there
							   //we use this to decide whether the above should be
							   //deleted by the destructor
}

fp_Line::~fp_Line()
{
	--s_iClassInstanceCounter;
	if(!s_iClassInstanceCounter)
	{
		delete [] s_pOldXs;
		s_pOldXs = NULL;
		s_iOldXsSize = 0;
	}

#ifdef USE_STATIC_MAP
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
	setScreenCleared(true);
}


#ifndef NDEBUG
bool fp_Line::assertLineListIntegrity(void)
{
	UT_sint32 k =0;
	fp_Run * pRunBlock = getFirstRun();
	fp_Run * pRunLine = NULL;
	for(k=0;k<getNumRunsInLine();k++)
	{
		pRunLine = getRunFromIndex(k);
		if(pRunLine != pRunBlock)
		{
			UT_DEBUGMSG(("Whoops! bug in Line at run %d %x offset %d Type %d \n",k,pRunLine,pRunLine->getBlockOffset(),pRunLine->getType()));
			pRunLine->printText();
			UT_sint32 i =0;
			for(i=0;i<getNumRunsInLine();i++)
			{
				fp_Run *pRun = getRunFromIndex(i);
				pRun->printText();
			}
			UT_ASSERT(pRunLine == pRunBlock);
		}
		pRunBlock = pRunBlock->getNext();
	}
	return true;
}
#else
bool fp_Line::assertLineListIntegrity(void)
{
	return true;
}
#endif
/*!
 * Return the gap between columns.
 */
UT_sint32  fp_Line::getColumnGap(void)
{
	return ((fp_Column *)getColumn())->getColumnGap();
}

/*!
 * Returns the column containing this line. This takes account of broken tables.
 */
fp_Container * fp_Line::getColumn(void)
{
	fp_Container * pCon = getContainer();
	if(pCon == NULL)
	{
		return NULL;
	}
	else if(pCon->getContainerType() != FP_CONTAINER_CELL)
	{
		return pCon->getColumn();
	}

	fp_CellContainer * pCell = (fp_CellContainer *) pCon;
	fp_TableContainer * pTab = (fp_TableContainer *) pCell->getContainer();
	if(pTab == NULL)
	{
	  return NULL;
	}
	UT_ASSERT(pTab->getContainerType() == FP_CONTAINER_TABLE);
	fp_TableContainer * pBroke = pTab->getFirstBrokenTable();
	if(pBroke == NULL)
	{
		return pCon->getColumn();
	}
	bool bFound = false;

	while(pBroke && !bFound)
	{
		if(pBroke->isInBrokenTable(pCell,this))
		{
			bFound = true;
			break;
		}
		pBroke = (fp_TableContainer *) pBroke->getNext();
	}
	if(bFound)
	{
		return pBroke->getColumn();
	}
	return pCon->getColumn();
}

/*!
 * Returns the page containing this line. Takes account of broken tables.
 */
fp_Page * fp_Line::getPage(void)
{
	return getColumn()->getPage();
}


/*!
 * Returns true if this is the first line in the block.
 */
bool fp_Line::isFirstLineInBlock(void) const
{
	return (m_pBlock->getFirstContainer() == (fp_Container *) this);
}

/*!
 * Returns true if this is the last line in the block.
 */
bool fp_Line::isLastLineInBlock(void) const
{
	return (m_pBlock->getLastContainer() == (fp_Container *) this);
}

void fp_Line::setMaxWidth(UT_sint32 iMaxWidth)
{
	m_iMaxWidth = iMaxWidth;
}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
void fp_Line::setMaxWidthInLayoutUnits(UT_sint32 iMaxWidth)
{
	m_iMaxWidthLayoutUnits = iMaxWidth;
}
#endif

void fp_Line::setContainer(fp_Container* pContainer)
{
	if (pContainer == getContainer())
	{
		return;
	}

	if (getContainer())
	{
		clearScreen();
	}

	fp_Container::setContainer(pContainer);
	setMaxWidth(pContainer->getWidth());
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	setMaxWidthInLayoutUnits(pContainer->getWidthInLayoutUnits());
#endif
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
	// need to tell the previous run to redraw, in case this run contained
	// overstriking characters
//	fp_Run* pPrevRun  = pRun->getPrev();
//	if(pPrevRun)
//		pPrevRun->clearScreen();

	if (bTellTheRunAboutIt)
	{
		pRun->setLine(NULL);
	}

	UT_sint32 ndx = m_vecRuns.findItem(pRun);
	UT_ASSERT(ndx >= 0);
	m_vecRuns.deleteNthItem(ndx);

	removeDirectionUsed(pRun->getDirection());

	return true;
}

void fp_Line::insertRunBefore(fp_Run* pNewRun, fp_Run* pBefore)
{
	//UT_DEBUGMSG(("insertRunBefore (line 0x%x, run 0x%x, type %d, dir %d)\n", this, pNewRun, pNewRun->getType(), pNewRun->getDirection()));
	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	UT_ASSERT(pNewRun);
	UT_ASSERT(pBefore);

	if (pNewRun->getType() == FPRUN_FIELD)
	{
		fp_FieldRun * fr = (fp_FieldRun*) pNewRun;
		if (fr->getFieldType() == FPFIELD_endnote_ref)
			m_bContainsFootnoteRef = true;
	}

	pNewRun->setLine(this);

	UT_sint32 ndx = m_vecRuns.findItem(pBefore);
	UT_ASSERT(ndx >= 0);

	m_vecRuns.insertItemAt(pNewRun, ndx);

	addDirectionUsed(pNewRun->getDirection());
}

void fp_Line::insertRun(fp_Run* pNewRun)
{
	//UT_DEBUGMSG(("insertRun (line 0x%x, run 0x%x, type %d)\n", this, pNewRun, pNewRun->getType()));

	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	pNewRun->setLine(this);

	m_vecRuns.insertItemAt(pNewRun, 0);

	addDirectionUsed(pNewRun->getDirection());
}

void fp_Line::addRun(fp_Run* pNewRun)
{
	//UT_DEBUGMSG(("addRun (line 0x%x, run 0x%x, type %d)\n", this, pNewRun, pNewRun->getType()));
	if (pNewRun->getType() == FPRUN_FIELD)
	{
		fp_FieldRun * fr = (fp_FieldRun*) pNewRun;
		if (fr->getFieldType() == FPFIELD_endnote_ref)
			m_bContainsFootnoteRef = true;
	}

	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	pNewRun->setLine(this);

	m_vecRuns.addItem(pNewRun);

	addDirectionUsed(pNewRun->getDirection());
	//setNeedsRedraw();
}

void fp_Line::insertRunAfter(fp_Run* pNewRun, fp_Run* pAfter)
{
	//UT_DEBUGMSG(("insertRunAfter (line 0x%x, run 0x%x, type %d)\n", this, pNewRun, pNewRun->getType()));
	if (pNewRun->getType() == FPRUN_FIELD)
	{
		fp_FieldRun * fr = (fp_FieldRun*) pNewRun;
		if (fr->getFieldType() == FPFIELD_endnote_ref)
			m_bContainsFootnoteRef = true;
	}

	UT_ASSERT(m_vecRuns.findItem(pNewRun) < 0);
	UT_ASSERT(pNewRun);
	UT_ASSERT(pAfter);

	pNewRun->setLine(this);

	UT_sint32 ndx = m_vecRuns.findItem(pAfter);
	UT_ASSERT(ndx >= 0);

	m_vecRuns.insertItemAt(pNewRun, ndx+1);

	addDirectionUsed(pNewRun->getDirection());
}

void fp_Line::remove(void)
{
	// getNext()/getPrev() appear hight in the performance statistics
	// called from this function so we will cache them
	fp_ContainerObject * pPrev = getPrev();
	fp_ContainerObject * pNext = getNext();
	
	if (pNext)
	{
		pNext->setPrev(pPrev);
	}

	if (pPrev)
	{
		pPrev->setNext(pNext);
	}

	((fp_VerticalContainer *)getContainer())->removeContainer(this);
}

void fp_Line::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos,
							  bool& bBOL, bool& bEOL)
{
	UT_uint32 count = m_vecRuns.getItemCount();

	FV_View* pView = getBlock()->getDocLayout()->getView();
	bool bShowHidden = pView->getShowPara();
	UT_uint32 i = 0;
	fp_Run* pFirstRun;
	bool bHidden;
	FPVisibility eHidden;

	do {

		pFirstRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(i++)); //#TF retrieve first visual run
		UT_ASSERT(pFirstRun);

		// if this tab is to be hidden, we must treated as if its
		// width was 0
		eHidden  = pFirstRun->isHidden();
		bHidden = ((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
		              || eHidden == FP_HIDDEN_REVISION
		              || eHidden == FP_HIDDEN_REVISION_AND_TEXT);
	}while(bHidden && (i < count));

	if(bHidden && (i < count))
	{
		//all runs on this line are hidden, at the moment just assert
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
	}



	bBOL = false;
	if (x <= pFirstRun->getX())
	{
		xxx_UT_DEBUGMSG(("fp_Line::mapXYToPosition [0x%x]: x=%d, first run x=%d\n", this, x, pFirstRun->getX()));
		bBOL = true;
		bool bBBOL = true;
		UT_sint32 y2 = y - pFirstRun->getY() - m_iAscent + pFirstRun->getAscent();
		pFirstRun->mapXYToPosition(0, y2, pos, bBBOL, bEOL);
		return;
	}

	// check all of the runs.

	fp_Run* pClosestRun = NULL;
	UT_sint32 iClosestDistance = 0;

	for (i=0; i<count; i++)
	{
		fp_Run* pRun2 = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(i));	//#TF get i-th visual run

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

	// if we do not have a closest run by now, then all the content of
	// this line is hidden; the only circumstance under which this
	// should be legal is if this is a last line in a paragraph and
	// this is the only paragraph in the document, in which case we
	// will use the EndOfParagraph run
	// However, for now we will allow this for all last lines in a
	// paragraph, whether it is the only one in the doc or not, since
	// hidden paragraphs need to be handled elsewhere
	
	if(!pClosestRun)
	{
		pClosestRun = getLastVisRun();

		if(pClosestRun && pClosestRun->getType() == FPRUN_ENDOFPARAGRAPH)
		{
			UT_sint32 y2 = y - pClosestRun->getY() - m_iAscent + pClosestRun->getAscent();
			pClosestRun->mapXYToPosition(x - pClosestRun->getX(), y2, pos, bBOL, bEOL);
			return;
		}
		
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		pos = 2; // start of document; this is just to avoid crashing
		return;
	}
	

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
	UT_sint32 my_xoff = -31999;
	UT_sint32 my_yoff = -31999;
	fp_VerticalContainer * pVCon= ((fp_VerticalContainer *)getContainer());
	pVCon->getOffsets(this, my_xoff, my_yoff);
	xoff = my_xoff + pRun->getX();
	yoff = my_yoff + pRun->getY() + m_iAscent - pRun->getAscent();
}

void fp_Line::getScreenOffsets(fp_Run* pRun,
							   UT_sint32& xoff,
							   UT_sint32& yoff)
{
	UT_sint32 my_xoff = -31999;
	UT_sint32 my_yoff = -31999;

	/*
		This method returns the screen offsets of the given
		run, referring to the UPPER-LEFT corner of the run.
	*/
	fp_VerticalContainer * pVCon= ((fp_VerticalContainer *)getContainer());
	pVCon->getScreenOffsets(this, my_xoff, my_yoff);

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
  Note by Sevior: This method is causing pixel dirt by making lines smaller
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

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	UT_sint32 iMaxAscentLayoutUnits = 0;
	UT_sint32 iMaxDescentLayoutUnits = 0;
#endif

	UT_sint32 iMaxImage =0;
	UT_sint32 iMaxText = 0;
	bool bSetByImage = false;
	for (i=0; i<count; i++)
	{
		UT_sint32 iAscent;
		UT_sint32 iDescent;

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		UT_sint32 iAscentLayoutUnits;
		UT_sint32 iDescentLayoutUnits;
#endif

		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		iAscent = pRun->getAscent();
		iDescent = pRun->getDescent();
		
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		iAscentLayoutUnits = pRun->getAscentInLayoutUnits();
		UT_ASSERT(!iAscent || iAscentLayoutUnits);
		iDescentLayoutUnits = pRun->getDescentInLayoutUnits();
#endif

		if (pRun->isSuperscript() || pRun->isSubscript())
		{
			iAscent += iAscent * 1/2;
			iDescent += iDescent;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
			iAscentLayoutUnits += iAscentLayoutUnits * 1/2;
			iDescentLayoutUnits += iDescentLayoutUnits;
#endif
		}
		if(pRun->getType() == FPRUN_IMAGE)
		{
			iMaxImage = UT_MAX(iAscent,iMaxImage);
		}
		else
		{
			iMaxText = UT_MAX(iAscent,iMaxText);
		}
		iMaxAscent = UT_MAX(iMaxAscent, iAscent);
		iMaxDescent = UT_MAX(iMaxDescent, iDescent);

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		iMaxAscentLayoutUnits = UT_MAX(iMaxAscentLayoutUnits, iAscentLayoutUnits);
		iMaxDescentLayoutUnits = UT_MAX(iMaxDescentLayoutUnits, iDescentLayoutUnits);
#endif
	}
	UT_sint32 iOldHeight = m_iHeight;
	UT_sint32 iOldAscent = m_iAscent;
	UT_sint32 iOldDescent = m_iDescent;

	UT_sint32 iNewHeight = iMaxAscent + iMaxDescent;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	UT_sint32 iNewHeightLayoutUnits = iMaxAscentLayoutUnits + iMaxDescentLayoutUnits;
#endif
	UT_sint32 iNewAscent = iMaxAscent;
	UT_sint32 iNewDescent = iMaxDescent;

	// adjust line height to include leading
	double dLineSpace;

	fl_BlockLayout::eSpacingPolicy eSpacing;

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	double dLineSpaceLayout;
	m_pBlock->getLineSpacing(dLineSpace, dLineSpaceLayout, eSpacing);
#else
	m_pBlock->getLineSpacing(dLineSpace, eSpacing);
#endif
	
	if(fabs(dLineSpace) < 0.0001)
	{
		xxx_UT_DEBUGMSG(("fp_Line: Set Linespace to 1.0 \n"));
		dLineSpace = 1.0;
	}
	if(iMaxImage > 0 && (iMaxImage > iMaxText * dLineSpace))
	{
		bSetByImage = true;
	}
	if (eSpacing == fl_BlockLayout::spacing_EXACT)
	{
		xxx_UT_DEBUGMSG(("recalcHeight exact \n"));
		iNewHeight = (UT_sint32) dLineSpace;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		iNewHeightLayoutUnits = (UT_sint32) dLineSpaceLayout;
#endif
	}
	else if (eSpacing == fl_BlockLayout::spacing_ATLEAST)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: recalcHeight at least \n"));
		iNewHeight = UT_MAX(iNewHeight, (UT_sint32) dLineSpace);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		iNewHeightLayoutUnits = UT_MAX(iNewHeightLayoutUnits, (UT_sint32) dLineSpaceLayout);
#endif
	}
	else
	{
		// multiple
		if(!bSetByImage)
		{
			iNewHeight = (UT_sint32) (iNewHeight * dLineSpace +0.5);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
			iNewHeightLayoutUnits = (UT_sint32) (iNewHeightLayoutUnits * dLineSpaceLayout +0.5);
#endif
			xxx_UT_DEBUGMSG(("recalcHeight neither dLineSpace = %f newheight =%d m_iScreenHeight =%d m_iHeight= %d\n",dLineSpace,iNewHeight,m_iScreenHeight,m_iHeight));
		}
		else
		{
			iNewHeight = UT_MAX(iMaxAscent+(UT_sint32) (iMaxDescent*dLineSpace + 0.5), (UT_sint32) dLineSpace);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
			iNewHeightLayoutUnits = UT_MAX(iMaxAscentLayoutUnits+(UT_sint32 )(iMaxDescentLayoutUnits*dLineSpace +0.5), (UT_sint32) dLineSpaceLayout);
#endif
		}
	}

	if (
		(iOldHeight != iNewHeight)
		|| (iOldAscent != iNewAscent)
		|| (iOldDescent != iNewDescent)
//		|| (iNewHeight > m_iScreenHeight)
		)
	{
		clearScreen();

#if 0 && !defined(WITH_PANGO)
		// FIXME:jskov We now get lines with height 0. Why is that a
		// problem (i.e., why the assert?)
		UT_ASSERT(iNewHeightLayoutUnits);
#endif
		m_iHeight = iNewHeight;
		m_iScreenHeight = -1;	// undefine screen height
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		m_iHeightLayoutUnits = iNewHeightLayoutUnits;
#endif
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
	UT_sint32 count = m_vecRuns.getItemCount();
	fp_Run * pRun = NULL;
	if(count > 0 && (UT_sint32)runIndex < count)
	{
		pRun = (fp_Run *) m_vecRuns.getNthItem(runIndex);
	}
	return pRun;
}

void fp_Line::clearScreen(void)
{
	if(getBlock()->isHdrFtr() || isScreenCleared())
	{
		return;
	}
	UT_sint32 count = m_vecRuns.getItemCount();
	if(count)
	{
		fp_Run* pRun;
		bool bNeedsClearing = true;

		UT_sint32 i;

		pRun = (fp_Run*) m_vecRuns.getNthItem(0);
		if(!pRun->getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
			return;

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
			fp_VerticalContainer * pVCon= ((fp_VerticalContainer *)getContainer());
			pVCon->getScreenOffsets(this, xoffLine, yoffLine);

			// Note: we use getHeight here instead of m_iScreenHeight
			// in case the line is asked to render before it's been
			// assigned a height. Call it robustness, if you want.

			UT_sint32 height = getHeight();
			// I have added the +1 to clear dirt after squiggles and
			// revision underlines

			pRun->getGraphics()->fillRect(pRun->getPageColor(),xoffLine - m_iClearLeftOffset, yoffLine, m_iClearToPos + m_iClearLeftOffset, height);
//
// Sevior: I added this for robustness.
//
			setScreenCleared(true);
			m_pBlock->setNeedsRedraw();
			setNeedsRedraw();
			UT_uint32 i;
			for(i=0; i < m_vecRuns.getItemCount();i++)
			{
				pRun = (fp_Run*) m_vecRuns.getNthItem(i);
				pRun->markAsDirty();
			}
		}
	}

}

/*!
 * This method clears a line from the run given to the end of the line.
\param fp_Run * pRun
*/
void fp_Line::clearScreenFromRunToEnd(fp_Run * ppRun)
{
	if(getBlock()->isHdrFtr())
	{
		return;
	}
	fp_Run * pRun = NULL;
	UT_sint32 count =  m_vecRuns.getItemCount();
	if(count > 0)
	{
		pRun = (fp_Run*) m_vecRuns.getNthItem(0);
		if(!pRun->getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
			return;

		UT_sint32 k = m_vecRuns.findItem((void *) ppRun);
		if(k>=0)
		{
			UT_uint32 runIndex = (UT_uint32) k;
			UT_sint32 xoff, yoff;

			pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(runIndex));

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
			fp_VerticalContainer * pVCon= ((fp_VerticalContainer *)getContainer());
			pVCon->getScreenOffsets(this, xoffLine, yoffLine);
			if(xoff == xoffLine)
				leftClear = pRun->getDescent();
			xxx_UT_DEBUGMSG(("SEVIOR: Doing clear from run to end xoff %d yoff %d \n",xoff,yoff));
			UT_ASSERT(yoff == yoffLine);
			pRun->getGraphics()->fillRect(pRun->getPageColor(),
										  xoff - leftClear,
										  yoff,
										  m_iClearToPos + leftClear
        										  - (xoff - xoffLine),
										  getHeight());
//
// Sevior: I added this for robustness.
//
			getBlock()->setNeedsRedraw();
			setNeedsRedraw();
			UT_sint32 i;
			for (i = runIndex; i < count; i++)
			{
				pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(i));
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
	if(getBlock()->isHdrFtr())
	{
		return;
	}
	//fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(runIndex);
	fp_Run* pRun; //#TF initialization not needed
	UT_sint32 count = m_vecRuns.getItemCount();
	pRun = (fp_Run*) m_vecRuns.getNthItem(0);

	fp_Run * pFRun = pRun;
	bool bUseFirst = false;
	if(runIndex == 1)
	{
		bUseFirst = true;
	}
	if(count > 0 && !pRun->getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
		return;

	// Find the first non dirty run.

	UT_sint32 i;
	for(i = runIndex; i < count; i++)
	{
		pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(i));

		if(pRun->isDirty())
		{
			runIndex++;
		}
		else
		{
			break;
		}
	}

	if((UT_sint32)runIndex < count)
	{
		UT_sint32 xoff, yoff;

		pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(runIndex));

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
		if(j>0 && pPrev != NULL && pPrev->getType() == FPRUN_TEXT)
		{
			leftClear = 0;
		}
		if(j>=0 && pPrev != NULL && pPrev->getType() == FPRUN_FIELD)
			leftClear = 0;
		if(j>=0 && pPrev != NULL && pPrev->getType() == FPRUN_IMAGE)
			leftClear = 0;

		if(bUseFirst)
		{
			getScreenOffsets(pFRun, xoff, yoff);
		}
		else
		{
			getScreenOffsets(pRun, xoff, yoff);
		}
		UT_sint32 xoffLine, yoffLine;
		UT_sint32 oldheight = getHeight();
		recalcHeight();
		UT_ASSERT(oldheight == getHeight());
		fp_VerticalContainer * pVCon= ((fp_VerticalContainer *)getContainer());
		pVCon->getScreenOffsets(this, xoffLine, yoffLine);

		UT_ASSERT(yoff == yoffLine);

		fp_Line * pPrevLine = (fp_Line *) getPrevContainerInSection();
		if(pPrevLine != NULL && (pPrevLine->getContainerType() == FP_CONTAINER_LINE))
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

		pRun->getGraphics()->fillRect(pRun->getPageColor(),
									  xoff - leftClear,
									  yoff,
									  m_iClearToPos + leftClear
									         - (xoff - xoffLine),
									  getHeight());
//
// Sevior: I added this for robustness.
//
		getBlock()->setNeedsRedraw();
		setNeedsRedraw();
		if(bUseFirst)
		{
		    runIndex = 0;
		}
		for (i = runIndex; i < count; i++)
		{
			pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(i));
			pRun->markAsDirty();
		}
	}
}


void fp_Line::setNeedsRedraw(void)
{
	m_bNeedsRedraw = true;
	m_pBlock->setNeedsRedraw();
}

/*! returns true if the line is on screen, false otherwise
    the caller can use this to test whether further processing is
    necessary, for instance inside a loop if the return value changes
    from true to false then no more visible lines are forthcoming and
    the loop can be terminated
 */
bool fp_Line::redrawUpdate(void)
{
	if(!isOnScreen())
		return false;
	
	UT_sint32 count = m_vecRuns.getItemCount();
	if(count)
	{
		draw(((fp_Run*) m_vecRuns.getNthItem(0))->getGraphics());
	}

	m_bNeedsRedraw = false;
	return true;
}


void fp_Line::draw(GR_Graphics* pG)
{
	//line can be wider than the max width due to trailing spaces
	//UT_ASSERT(m_iWidth <= m_iMaxWidth);

	UT_sint32 count = m_vecRuns.getItemCount();
	if(count <= 0)
		return;

	UT_sint32 my_xoff = 0, my_yoff = 0;
	fp_VerticalContainer * pVCon= ((fp_VerticalContainer *)getContainer());
	pVCon->getScreenOffsets(this, my_xoff, my_yoff);
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
	da.bDirtyRunsOnly = true; //magic line to give a factor 2 speed up!
	const UT_Rect* pRect = pG->getClipRect();

	FV_View* pView = getBlock()->getDocLayout()->getView();
	bool bShowHidden = pView->getShowPara();

	for (int i=0; i < count; i++)
	{
		// NB !!! In the BiDi build drawing has to be done in the logical
		// order, otherwise overstriking characters cannot be seen
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		FPVisibility eHidden  = pRun->isHidden();
		if((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
		   || eHidden == FP_HIDDEN_REVISION
		   || eHidden == FP_HIDDEN_REVISION_AND_TEXT)
			continue;

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

		// shortcircuit drawing if we're not included in the dirty region
		UT_Rect runRect(da.xoff, da.yoff, pRun->getWidth(), pRun->getHeight());
#ifdef DEBUG
		flash(pG, runRect, UT_RGBColor(255, 255, 0));

		if (pRect)
			flash(pG, *pRect, UT_RGBColor(0, 255, 255));
		else
			xxx_UT_DEBUGMSG(("pRect NULL\n"));
#endif /* DEBUG */
		if (pRect == NULL || pRect->intersectsRect(&runRect))
			pRun->draw(&da);

		da.xoff -= pRun->getX();
		da.yoff -= pRun->getY();
	}
//
// Check if this is in a cell, if so redraw the lines around it.
//
#if 0
	fp_Container * pCon = getContainer();
	if(pCon->getContainerType() == FP_CONTAINER_CELL)
	{
		fp_CellContainer * pCell = (fp_CellContainer *) pCon;
		pCell->drawLinesAdjacent();
	}
#endif

}

void fp_Line::draw(dg_DrawArgs* pDA)
{
	UT_sint32 count = m_vecRuns.getItemCount();
	if(count <= 0)
		return;

	//xxx_UT_DEBUGMSG(("SEVIOR: Drawing line in line pDA \n"));
	FV_View* pView = getBlock()->getDocLayout()->getView();
	bool bShowHidden = pView->getShowPara();

	pDA->yoff += m_iAscent;
	const UT_Rect* pRect = pDA->pG->getClipRect();

	for (int i=0; i<count; i++)
	{
		// NB !!! In the BiDi build drawing has to be done in the logical
		// order, otherwise overstriking characters cannot be seen
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		FPVisibility eHidden  = pRun->isHidden();
		if((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
		   || eHidden == FP_HIDDEN_REVISION
		   || eHidden == FP_HIDDEN_REVISION_AND_TEXT)
			continue;

		FP_RUN_TYPE rType = pRun->getType();

		dg_DrawArgs da = *pDA;

		// for these two types of runs, we want to draw for the
		// entire line-width on the next line. see bug 1301
		if (rType == FPRUN_FORCEDCOLUMNBREAK ||
			rType == FPRUN_FORCEDPAGEBREAK)
		{
			UT_sint32 my_xoff = 0, my_yoff = 0;
			fp_VerticalContainer * pVCon= ((fp_VerticalContainer *)getContainer());
			pVCon->getScreenOffsets(this, my_xoff, my_yoff);
			da.xoff = my_xoff;
		}
		else
		{
			da.xoff += pRun->getX();
		}

		da.yoff += pRun->getY();
		UT_Rect runRect(da.xoff, da.yoff - m_iAscent, pRun->getWidth(), pRun->getHeight());
		// flash(pDA->pG, runRect, UT_RGBColor(0, 255, 0));

		if (pRect == NULL || pRect->intersectsRect(&runRect))
			pRun->draw(&da);

		da.yoff -= pRun->getY();
	}
//
// Check if this is in a cell, if so redraw the lines around it.
//
#if 0
	fp_Container * pCon = getContainer();
	if(pCon->getContainerType() == FP_CONTAINER_CELL)
	{
		fp_CellContainer * pCell = (fp_CellContainer *) pCon;
		pCell->drawLinesAdjacent();
	}
#endif
}

//this is a helper function for getRunWith; it works out working direction and
//which tabs to use from the alignment
//it is public, because it is only used when getRunWidth is called from
//outside of the class

void fp_Line::getWorkingDirectionAndTabstops(FL_WORKING_DIRECTION &eWorkingDirection, FL_WHICH_TABSTOP &eUseTabStop) const
{
	fb_Alignment* pAlignment = m_pBlock->getAlignment();
	UT_ASSERT(pAlignment);
	FB_AlignmentType eAlignment = pAlignment->getType();

	// find out the direction of the paragraph
	FriBidiCharType iDomDirection = m_pBlock->getDominantDirection();

	eWorkingDirection = WORK_FORWARD;
	eUseTabStop = USE_NEXT_TABSTOP;

	// now from the current alignment work out which way we need to process the line
	// and the corresponding starting positions

	switch (eAlignment)
	{
		case FB_ALIGNMENT_LEFT:
			if(iDomDirection == FRIBIDI_TYPE_RTL)
				eUseTabStop = USE_PREV_TABSTOP;
			else
				eUseTabStop = USE_NEXT_TABSTOP;

			eWorkingDirection = WORK_FORWARD;
			break;

		case FB_ALIGNMENT_RIGHT:
			if(iDomDirection == FRIBIDI_TYPE_RTL)
				eUseTabStop = USE_NEXT_TABSTOP;
			else
				eUseTabStop = USE_PREV_TABSTOP;

			eWorkingDirection = WORK_BACKWARD;
			break;

		case FB_ALIGNMENT_CENTER:
			eWorkingDirection = WORK_FORWARD;
			eUseTabStop = USE_FIXED_TABWIDTH;
			break;

		case FB_ALIGNMENT_JUSTIFY:
			if(iDomDirection == FRIBIDI_TYPE_RTL)
				eWorkingDirection = WORK_BACKWARD;
			else
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

//NB. in bidi mode iXLayoutUnits is a logical coordinance
//however, in _calculateWidthOfRun iX and iXLayoutUnits are visual,
//so we need to do a conversion here ...

fp_Run* fp_Line::calculateWidthOfRun(UT_sint32 &iWidthLayoutUnits, UT_uint32 iIndxVisual, FL_WORKING_DIRECTION eWorkingDirection, FL_WHICH_TABSTOP eUseTabStop)
{
	const UT_sint32 iCountRuns		  = m_vecRuns.getItemCount();
	UT_ASSERT(iCountRuns > (UT_sint32)iIndxVisual);

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	UT_sint32 iXLreal;
	const UT_sint32 Screen_resolution =
		m_pBlock->getDocLayout()->getGraphics()->getResolution();
#endif
	UT_sint32 iXreal;

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	iXreal = iWidthLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
#else
	iXreal = iWidthLayoutUnits; // no layout units realy
#endif

	//work out the real index based on working direction
	UT_uint32 iIndx;
	iIndx = eWorkingDirection == WORK_FORWARD ? iIndxVisual : iCountRuns - iIndxVisual - 1;

	// of course, the loop is running in visual order, but the vector is
	// in logical order
	fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iIndx));

	// find out the direction of the paragraph
	FriBidiCharType iDomDirection = m_pBlock->getDominantDirection();

	if(iDomDirection == FRIBIDI_TYPE_RTL)
	{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		iXLreal = m_iMaxWidthLayoutUnits - iWidthLayoutUnits;
#endif
		iXreal	= m_iMaxWidth - iXreal;
	}
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	else
	{
		iXLreal = iWidthLayoutUnits;
		//iXreal = iXreal;
	}
#endif
	xxx_UT_DEBUGMSG(("fp_Line::calculateWidthOfRun (0x%x L 0x%x): \n"
				 "		 iXreal %d, iXLreal %d, iIndxVisual %d, iCountRuns %d,\n"
				 "		 eWorkingDirection %d, eUseTabStop %d,\n"
						 ,pRun,this,iXreal, iXLreal, iIndxVisual, iCountRuns,
						 eWorkingDirection, eUseTabStop
				));

	_calculateWidthOfRun(iXreal,
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
						 iXLreal,
#endif
						 pRun,
						 iIndxVisual,
						 iCountRuns,
						 eWorkingDirection,
						 eUseTabStop,
						 iDomDirection
						 );

//xxx_UT_DEBUGMSG(("new iXreal %d, iXLreal %d\n", iXreal, iXLreal));

	if(iDomDirection == FRIBIDI_TYPE_RTL)
	{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		iWidthLayoutUnits = m_iMaxWidthLayoutUnits - iXLreal;
#else
		iWidthLayoutUnits = m_iMaxWidth - iXreal;
#endif
	}
	else
	{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		iWidthLayoutUnits = iXLreal;
#else
		iWidthLayoutUnits = iXreal;
#endif
	}

	return pRun;
}

// private version of the above, which expect both the index and run prointer
// to be passed to it.
inline void fp_Line::_calculateWidthOfRun(	UT_sint32 &iX,
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
									UT_sint32 &iXLayoutUnits,
#endif
									fp_Run * pRun,
									UT_uint32 iIndx,
									UT_uint32 iCountRuns,
									FL_WORKING_DIRECTION eWorkingDirection,
									FL_WHICH_TABSTOP eUseTabStop,
									FriBidiCharType iDomDirection
									)
{
	if(!pRun)
		return;

	FV_View* pView = getBlock()->getDocLayout()->getView();
	bool bShowHidden = pView->getShowPara();
	FPVisibility eHidden  = pRun->isHidden();

	bool bHidden = ((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
					|| eHidden == FP_HIDDEN_REVISION
					|| eHidden == FP_HIDDEN_REVISION_AND_TEXT);

	// If the run is to be hidden just return, since the positions
	// should remain as they were, but it would be preferable if this
	// situation was trapped higher up (thus the assert)
	if(bHidden)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}


#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	const UT_sint32 Screen_resolution = m_pBlock->getDocLayout()->getGraphics()->getResolution();
#endif

	switch(pRun->getType())
	{
		case FPRUN_TAB:
		{
			// a fixed width fraction of the font ascent which we will use for centered alignment
			// i.e, width = font_ascent * iFixedWidthMlt / iFixedWidthDiv
			const UT_uint32 iFixedWidthMlt = 2;
			const UT_uint32 iFixedWidthDiv = 1;
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
				if(iDomDirection == FRIBIDI_TYPE_RTL)
				{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
					
					UT_sint32 iStartPos = getContainer()->getWidthInLayoutUnits() - iXLayoutUnits;
					bRes = findNextTabStopInLayoutUnits(iStartPos, iPosLayoutUnits, iTabType, iTabLeader);
					iPosLayoutUnits = getContainer()->getWidthInLayoutUnits() - iPosLayoutUnits;
#else
					UT_sint32 iStartPos = getContainer()->getWidth() - iX;
					bRes = findNextTabStop(iStartPos, iPosLayoutUnits, iTabType, iTabLeader);
					iPosLayoutUnits = getContainer()->getWidth() - iPosLayoutUnits;
#endif
				}
				else
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
					bRes = findNextTabStopInLayoutUnits(iXLayoutUnits, iPosLayoutUnits, iTabType, iTabLeader);
#else
					bRes = findNextTabStop(iX, iPosLayoutUnits, iTabType, iTabLeader);
#endif
			}
			else

			if(iDomDirection == FRIBIDI_TYPE_RTL)
			{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
				UT_sint32 iStartPos = getContainer()->getWidthInLayoutUnits() - iXLayoutUnits;

				bRes = findPrevTabStopInLayoutUnits(iStartPos, iPosLayoutUnits, iTabType, iTabLeader);
				iPosLayoutUnits = getContainer()->getWidthInLayoutUnits() - iPosLayoutUnits;
#else
				UT_sint32 iStartPos = getContainer()->getWidth() - iX;

				bRes = findPrevTabStop(iStartPos, iPosLayoutUnits, iTabType, iTabLeader);
				iPosLayoutUnits = getContainer()->getWidth() - iPosLayoutUnits;
#endif
			}
			else
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
				bRes = findPrevTabStopInLayoutUnits(iXLayoutUnits, iPosLayoutUnits, iTabType, iTabLeader);
#else
				bRes = findPrevTabStop(iX, iPosLayoutUnits, iTabType, iTabLeader);
#endif


			UT_ASSERT(bRes);

			fp_Run *pScanRun = NULL;
			UT_sint32 iScanWidth = 0;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
			UT_sint32 iScanWidthLayoutUnits = 0;
#endif
			pTabRun->setLeader(iTabLeader);
			pTabRun->setTabType(iTabType);

			// we need to remember what the iX was
			UT_sint32 iXprev;
			iXprev = iX;

//xxx_UT_DEBUGMSG(("pf_Line::_calculateWidthOfRun(): tab: iX %d, iXLayout %d, iPosLayout %d, resolution %d\n",iX,iXLayoutUnits,iPosLayoutUnits,Screen_resolution));

			FriBidiCharType iVisDirection = pTabRun->getVisDirection();

			switch ( iTabType )
			{
				case FL_TAB_LEFT:
					if(iVisDirection == FRIBIDI_TYPE_LTR && iDomDirection == FRIBIDI_TYPE_LTR)
					{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
						iXLayoutUnits = iPosLayoutUnits;
						iX = iXLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
#else
						iX = iPosLayoutUnits;
#endif

						iWidth = abs(iX - iXprev);
						xxx_UT_DEBUGMSG(("left tab (ltr para), iX %d, iWidth %d\n", iX,iWidth));
					}
					else
					{
						iScanWidth = 0;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
						iScanWidthLayoutUnits = 0;
#endif
						for ( UT_uint32 j = iIndx+1; j < iCountRuns; j++ )
						{
							UT_uint32 iJ;
							iJ = eWorkingDirection == WORK_FORWARD ? j : iCountRuns - j - 1;

							pScanRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iJ));
							if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
								break;

							iScanWidth += pScanRun->getWidth();
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
							iScanWidthLayoutUnits += pScanRun->getWidthInLayoutUnits();
#endif
						}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
						if ( iScanWidthLayoutUnits > abs(iPosLayoutUnits - iXLayoutUnits))
#else
						if ( iScanWidth > abs(iPosLayoutUnits - iX))
#endif
						{
							iWidth = 0;
						}
						else
						{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
							iXLayoutUnits += iPosLayoutUnits - iXLayoutUnits - (UT_sint32)eWorkingDirection * iScanWidthLayoutUnits;
							iX += iPosLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS - iX - (UT_sint32)eWorkingDirection * iScanWidth;
#else
							iX += iPosLayoutUnits - iX - (UT_sint32)eWorkingDirection * iScanWidth;
#endif
							iWidth = abs(iX - iXprev);
						}
					}

					break;

					case FL_TAB_CENTER:
					{
						iScanWidth = 0;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
						iScanWidthLayoutUnits = 0;
#endif
						for ( UT_uint32 j = iIndx+1; j < iCountRuns; j++ )
						{
							UT_uint32 iJ;
							iJ = eWorkingDirection == WORK_FORWARD ? j : iCountRuns - j - 1;

							pScanRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iJ));

							if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
								break;
							iScanWidth += pScanRun->getWidth();
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
							iScanWidthLayoutUnits += pScanRun->getWidthInLayoutUnits();
#endif
						}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
						if ( iScanWidthLayoutUnits / 2 > abs(iPosLayoutUnits - iXLayoutUnits))
#else
						if ( iScanWidth / 2 > abs(iPosLayoutUnits - iX))
#endif
							iWidth = 0;
						else
						{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
							iXLayoutUnits += iPosLayoutUnits - iXLayoutUnits - (UT_sint32)eWorkingDirection * iScanWidthLayoutUnits / 2;
							iX += iPosLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS - iX - (UT_sint32)eWorkingDirection * iScanWidth / 2;
#else
							iX += iPosLayoutUnits - iX - (UT_sint32)eWorkingDirection * iScanWidth / 2;
#endif
							iWidth = abs(iX - iXprev);
						}
						break;
					}

					case FL_TAB_RIGHT:
						if(iVisDirection == FRIBIDI_TYPE_RTL && iDomDirection == FRIBIDI_TYPE_RTL)
						{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
							iXLayoutUnits = iPosLayoutUnits;
							iX = iXLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
#else
							iX = iPosLayoutUnits;
#endif
							iWidth = abs(iX - iXprev);
							xxx_UT_DEBUGMSG(("right tab (rtl para), iX %d, width %d\n", iX,pTabRun->getWidth()));
						}
						else
						{
							xxx_UT_DEBUGMSG(("right tab (ltr para), ii %d\n",ii));
							iScanWidth = 0;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
							iScanWidthLayoutUnits = 0;
#endif
							for ( UT_uint32 j = iIndx+1; j < iCountRuns; j++ )
							{
								UT_uint32 iJ;
								iJ = eWorkingDirection == WORK_FORWARD ? j : iCountRuns - j - 1;
								xxx_UT_DEBUGMSG(("iJ %d\n", iJ));

								pScanRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iJ));

								if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
									break;
								iScanWidth += pScanRun->getWidth();
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
								iScanWidthLayoutUnits += pScanRun->getWidthInLayoutUnits();
#endif
							}

							xxx_UT_DEBUGMSG(("iScanWidthLayoutUnits %d, iPosLayoutUnits %d, iXLayoutUnits %d\n",iScanWidthLayoutUnits,iPosLayoutUnits,iXLayoutUnits));

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
							if ( iScanWidthLayoutUnits > abs(iPosLayoutUnits - iXLayoutUnits))
#else
							if ( iScanWidth > abs(iPosLayoutUnits - iX))
#endif
							{
								iWidth = 0;
							}
							else
							{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
								iXLayoutUnits += iPosLayoutUnits - iXLayoutUnits - (UT_sint32)eWorkingDirection * iScanWidthLayoutUnits;
								iX += iPosLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS - iX - (UT_sint32)eWorkingDirection * iScanWidth;
#else
								iX += iPosLayoutUnits - iX - (UT_sint32)eWorkingDirection * iScanWidth;
#endif
								iWidth = abs(iX - iXprev);
							}

						}
						break;

					case FL_TAB_DECIMAL:
					{
						UT_UCSChar *pDecimalStr;
						UT_uint32	runLen = 0;

#if 1
// localeconv is ANSI C and C++, but in case we might run into trouble
// this will make it easire to undo temporarily (we need to do this though)
						// find what char represents a decimal point
						lconv *loc = localeconv();
						if ( ! UT_UCS4_cloneString_char(&pDecimalStr, loc->decimal_point) )
						{
							// Out of memory. Now what?
						}
#else
						if ( ! UT_UCS_cloneString_char(&pDecimalStr, '.') )
						{
							// Out of memory. Now what?
						}
#endif
						iScanWidth = 0;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
						iScanWidthLayoutUnits = 0;
#endif
						for ( UT_uint32 j = iIndx+1; j < iCountRuns; j++ )
						{
							UT_uint32 iJ;
							iJ = eWorkingDirection == WORK_FORWARD ? j : iCountRuns - j - 1;

							pScanRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iJ));

							if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
								break;

							bool foundDecimal = false;
							if(pScanRun->getType() == FPRUN_TEXT)
							{
								UT_sint32 decimalBlockOffset = ((fp_TextRun *)pScanRun)->findCharacter(0, pDecimalStr[0]);
								if(decimalBlockOffset != -1)
								{
									foundDecimal = true;
									UT_uint32 u_decimalBlockOffset = static_cast<UT_uint32>(decimalBlockOffset);
									UT_ASSERT(pScanRun->getBlockOffset() <= u_decimalBlockOffset); // runLen is unsigned
									runLen = u_decimalBlockOffset - pScanRun->getBlockOffset();
								}
							}

							xxx_UT_DEBUGMSG(("%s(%d): foundDecimal=%d len=%d iScanWidth=%d \n",
								__FILE__, __LINE__, foundDecimal, pScanRun->getLength()-runLen, iScanWidth));
							if ( foundDecimal )
							{
								UT_ASSERT(pScanRun->getType() == FPRUN_TEXT);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
								iScanWidth += ((fp_TextRun *)pScanRun)->simpleRecalcWidth(fp_TextRun::Width_type_display, runLen);
								iScanWidthLayoutUnits += ((fp_TextRun *)pScanRun)->simpleRecalcWidth(fp_TextRun::Width_type_layout_units, runLen);
#else
								iScanWidth += ((fp_TextRun *)pScanRun)->simpleRecalcWidth(runLen);
#endif
								break; // we found our decimal, don't search any further
							}
							else
							{
								iScanWidth += pScanRun->getWidth();
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
								iScanWidthLayoutUnits += pScanRun->getWidthInLayoutUnits();
#endif
							}
						}
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
						if ( iScanWidthLayoutUnits > abs(iPosLayoutUnits - iXLayoutUnits))
#else
						if ( iScanWidth > abs(iPosLayoutUnits - iX))
#endif
						{

							// out of space before the decimal point;
							// tab collapses to nothing
							iWidth = 0;
						}
						else {
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
							iXLayoutUnits = iPosLayoutUnits - (UT_sint32)eWorkingDirection * iScanWidthLayoutUnits;
							iX = iPosLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS - (UT_sint32)eWorkingDirection * iScanWidth;
#else
							iX = iPosLayoutUnits - (UT_sint32)eWorkingDirection * iScanWidth;
#endif
							iWidth = abs(iX - iXprev);
						}
						FREEP(pDecimalStr);
						break;
					}

					case FL_TAB_BAR:
					{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
						iXLayoutUnits = iPosLayoutUnits;
						iX = iXLayoutUnits * Screen_resolution / UT_LAYOUT_UNITS;
#else
						iX = iPosLayoutUnits;
#endif
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

			pTabRun->setTabWidth(iWidth);
			return;
		}

		case FPRUN_TEXT:
		{
			if(static_cast<fp_TextRun*>(pRun)->getUseContextGlyphs())
				pRun->recalcWidth();
			//and fall through to default
		}

		default:
		{
			if(eWorkingDirection == WORK_FORWARD)
			{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
				iXLayoutUnits += pRun->getWidthInLayoutUnits();
#endif
				iX += pRun->getWidth();
			}
			else
			{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
				iXLayoutUnits -= pRun->getWidthInLayoutUnits();
#endif
				iX -= pRun->getWidth();
			}
					xxx_UT_DEBUGMSG(("fp_Line::calculateWidthOfRun (non-tab [0x%x, type %d, dir %d]): width %d\n",
							pRun,pRun->getType(),pRun->getDirection(),pRun->getWidth()));
			return;
		}

	}//switch run type

}

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

	xxx_UT_DEBUGMSG(("fp_Line::layout (0x%x)\n",this));

	// first of all, work out the height
	recalcHeight();

	UT_sint32 iCountRuns		  = m_vecRuns.getItemCount();
	// I think we cannot return before the call to recalcHeight above, since we
	// could be called in response to all runs being removed, and that potentially
	// changes the line height; anything from here down has to do with runs though
	// so if we have none, we can return
	if(iCountRuns <= 0)
		return;

	// get current alignment; note that we cannot initialize the alignment
	// at this stage, (and chances are we will not need to anyway), because
	// we have to first calculate the widths of our tabs
	fb_Alignment* pAlignment = m_pBlock->getAlignment();
	UT_ASSERT(pAlignment);
	FB_AlignmentType eAlignment 	  = pAlignment->getType();

	//we have to remember the old X coordinances of our runs
	//to be able to decide latter whether and where from to erase
	//(this is a real nuisance, but since it takes two passes to do the layout
	//I do not see a way to avoid this)
	//we will use a static buffer for this initialised to a decent size and
	//reallocated as needed
#ifdef DEBUG
	const UT_uint32 iDefinesLine = __LINE__;
#endif

#ifdef DEBUG
	UT_uint32 iRealocCount = 0;
#endif
	while((UT_sint32)s_iOldXsSize < iCountRuns + 1)
	{
		// always make sure there is one space available past the last run
		// we will set that to 0 and it will help us to handle the justified
		// alignment spliting runs while we are working on them (see notes after
		// the main loop)

		// UT_DEBUGMSG(("fp_Line::layout: static buffer pOldXs too small\n"
					 // "		(original size %d, new size %d)\n"
					 // "		IF THIS MESSAGE APPEARS TOO OFTEN, INCREASE \"STATIC_BUFFER_INITIAL\"\n"
					 // "		(line %d in %s)\n",
					 // iOldXsSize, iOldXsSize+STATIC_BUFFER_INCREMENT, iDefinesLine + 2, __FILE__));

		delete[] s_pOldXs;
		s_iOldXsSize *= 2;
		s_pOldXs = new UT_sint32[s_iOldXsSize];
#ifdef DEBUG
		iRealocCount++;
		if(iRealocCount > 1)
			UT_DEBUGMSG(("fp_Line::layout: static buffer required repeated reallocation\n"
						 "		 IF THIS MESSAGE APPEARS INCREASE \"STATIC_BUFFER_INCREMENT\"\n"
						 "		 (line %d in %s)\n", iDefinesLine+1, __FILE__));

#endif
	}

	UT_ASSERT(s_pOldXs);

	UT_sint32 iStartX				  = 0;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	UT_sint32 iStartXLayoutUnits	  = 0;
#endif

	// find out the direction of the paragraph
	FriBidiCharType iDomDirection = m_pBlock->getDominantDirection();

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
			if(iDomDirection == FRIBIDI_TYPE_RTL)
				eUseTabStop = USE_PREV_TABSTOP;
			else
				eUseTabStop = USE_NEXT_TABSTOP;

			eWorkingDirection = WORK_FORWARD;
			break;

		case FB_ALIGNMENT_RIGHT:
			if(iDomDirection == FRIBIDI_TYPE_RTL)
				eUseTabStop = USE_NEXT_TABSTOP;
			else
				eUseTabStop = USE_PREV_TABSTOP;

			eWorkingDirection = WORK_BACKWARD;
			iStartX = m_iMaxWidth;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
			iStartXLayoutUnits = m_iMaxWidthLayoutUnits;
#endif
			break;

		case FB_ALIGNMENT_CENTER:
			eWorkingDirection = WORK_FORWARD;
			eUseTabStop = USE_FIXED_TABWIDTH;
			// we will pretend the line starts at pos 0, work out the width
			// and then shift it by the necessary amount to the right
			iStartX = 0;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
			iStartXLayoutUnits = 0;
#endif
			break;

		case FB_ALIGNMENT_JUSTIFY:
			if(iDomDirection == FRIBIDI_TYPE_RTL)
			{
				eWorkingDirection = WORK_BACKWARD;
				iStartX = m_iMaxWidth;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
				iStartXLayoutUnits = m_iMaxWidthLayoutUnits;
#endif
			}
			else
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
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	UT_sint32 iXLayoutUnits = iStartXLayoutUnits;
#endif
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
				 "		 alignment [%s], working direction [%s], using tabstops [%s]\n"
				 "		 fixed width multiplier %d/%d\n"
				 "		 iStartX	= %d, iStartXLayout = %d \n"
				 "		 iCountRuns = %d\n",
				 this, al, d, t, iFixedWidthMlt, iFixedWidthDiv,
				 iStartX, iStartXLayoutUnits, iCountRuns
	));

#endif //end of the debug stuff


	// now we work our way through the runs on this line
	xxx_UT_DEBUGMSG(("fp_Line::layout ------------------- \n"));

	FV_View* pView = getBlock()->getDocLayout()->getView();
	bool bShowHidden = pView ? pView->getShowPara() : false;

	UT_sint32 ii = 0;
	for (; ii<iCountRuns; ++ii)
	{
		//work out the real index based on working direction
		UT_uint32 iIndx;
		iIndx = eWorkingDirection == WORK_FORWARD ? ii : iCountRuns - ii - 1;

		// of course, the loop is running in visual order, but the vector is
		// in logical order
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iIndx));


		// if this tab is to be hidden, we must treat it as if its
		// width was 0
		FPVisibility eHidden  = pRun->isHidden();
		bool bHidden = ((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
		              || eHidden == FP_HIDDEN_REVISION
		              || eHidden == FP_HIDDEN_REVISION_AND_TEXT);

		if(bHidden)
			continue;

		// if we are working from the left, we want to set the
		// X coordinance now; if from the right we will do it
		// after we have got to the width worked out
		// also, decide whether erasure is needed
		if(eWorkingDirection == WORK_FORWARD)
		{
			s_pOldXs[iIndx] = pRun->getX();
				pRun->setX(iX,FP_CLEARSCREEN_NEVER);
		}
		xxx_UT_DEBUGMSG(("fp_Line::layout: iX %d, iXL %d, ii %d, iCountRuns %d\n"
					 "		 run type %d\n",
					iX, iXLayoutUnits, ii, iCountRuns, pRun->getType()));
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		_calculateWidthOfRun(iX,
							 iXLayoutUnits,
							 pRun,
							 ii,
							 iCountRuns,
							 eWorkingDirection,
							 eUseTabStop,
							 iDomDirection
							);
#else
		_calculateWidthOfRun(iX,
							 pRun,
							 ii,
							 iCountRuns,
							 eWorkingDirection,
							 eUseTabStop,
							 iDomDirection
							);
#endif

		// if working backwards, set the new X coordinance
		// and decide if line needs erasing
		if(eWorkingDirection == WORK_BACKWARD)
		{
			s_pOldXs[iIndx] = pRun->getX();
			pRun->setX(iX,FP_CLEARSCREEN_NEVER);
		}
	} //for

	// this is to simplify handling justified alignment -- see below
	s_pOldXs[ii] = 0;


	///////////////////////////////////////////////////////////////////
	//	now we are ready to deal with the alignment
	//
	pAlignment->initialize(this);
	iStartX = pAlignment->getStartPosition();

	// now we have to get the iCountRuns value afresh, because if the alignment
	// is justified then it is possible that the call to pAlignment->initialize()
	// will split the previous set of runs into more ... (took me many frustrated
	// hours to work this out) -- this happens on loading a document where each
	// line is initially just a single run in the non-bidi build
	//
	// This also means that the pOldX array may be of no
	// use, but then we will only need to worry about pOldXs just after the last
	// run, since as long as the first new run kicks in, the rest will follow

	iCountRuns		  = m_vecRuns.getItemCount();

	xxx_UT_DEBUGMSG(("fp_Line::layout(): original run count %d, new count %d\n",
				ii, iCountRuns));
	switch(eAlignment)
	{
		case FB_ALIGNMENT_LEFT:
		case FB_ALIGNMENT_RIGHT:
			{
				for (UT_sint32 k = 0; k < iCountRuns; k++)
				{
					fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(k));
					UT_ASSERT(pRun);

					// if this tab is to be hidden, we must treated as if its
					// width was 0
					FPVisibility eHidden  = pRun->isHidden();
					bool bHidden = ((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
								  || eHidden == FP_HIDDEN_REVISION
								  || eHidden == FP_HIDDEN_REVISION_AND_TEXT);

					if(bHidden)
						continue;

					//eClearScreen = iStartX == pOldXs[k] ? FP_CLEARSCREEN_NEVER : FP_CLEARSCREEN_FORCE;
					if(!bLineErased && iStartX != s_pOldXs[k])
					{
						bLineErased = true;
						iIndxToEraseFrom = k;
					}

					pRun->setX(iStartX,FP_CLEARSCREEN_NEVER);
					iStartX += pRun->getWidth();
				}

			}
		break;
		case FB_ALIGNMENT_JUSTIFY:
			{
				// now we need to shift the x-coordinances to reflect the new widths
				// of the spaces
				for (UT_sint32 k = 0; k < iCountRuns; k++)
				{
					UT_uint32 iK = (eWorkingDirection == WORK_FORWARD) ? k : iCountRuns - k - 1;
					fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(iK));
					UT_ASSERT(pRun);

					// if this tab is to be hidden, we must treated as if its
					// width was 0
					FPVisibility eHidden  = pRun->isHidden();
					bool bHidden = ((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
								  || eHidden == FP_HIDDEN_REVISION
								  || eHidden == FP_HIDDEN_REVISION_AND_TEXT);

					if(bHidden)
						continue;

					if(eWorkingDirection == WORK_BACKWARD)
					{
						iStartX -= pRun->getWidth();
						//eClearScreen = iStartX == pOldXs[iK] ? FP_CLEARSCREEN_NEVER : FP_CLEARSCREEN_FORCE;
						if(!bLineErased && iStartX != s_pOldXs[iK])
						{
							bLineErased = true;
							iIndxToEraseFrom = iK;
						}

						pRun->setX(iStartX, FP_CLEARSCREEN_NEVER);
					}
					else
					{
						//eClearScreen = iStartX == pOldXs[iK] ? FP_CLEARSCREEN_NEVER : FP_CLEARSCREEN_FORCE;
						if(!bLineErased && iStartX != s_pOldXs[iK])
						{
							bLineErased = true;
							iIndxToEraseFrom = iK;
						}
						pRun->setX(iStartX, FP_CLEARSCREEN_NEVER);
						iStartX += pRun->getWidth();
					}
				}
		}
		break;
		case FB_ALIGNMENT_CENTER:
			{
				//if the line is centered we will have to shift the iX of each run
				//since we worked on the assumption that the line starts at 0
				//only now are we in the position to enquire of the alignment what
				//the real starting position should be

				for (UT_sint32 k = 0; k < iCountRuns; k++)
				{
					fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(k));
					UT_ASSERT(pRun);

					// if this tab is to be hidden, we must treated as if its
					// width was 0
					FPVisibility eHidden  = pRun->isHidden();
					bool bHidden = ((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
								  || eHidden == FP_HIDDEN_REVISION
								  || eHidden == FP_HIDDEN_REVISION_AND_TEXT);

					if(bHidden)
						continue;

					UT_sint32 iCurX = pRun->getX();
					//eClearScreen = iCurX + iStartX == pOldXs[k] ? FP_CLEARSCREEN_NEVER : FP_CLEARSCREEN_FORCE;
					if(!bLineErased && iCurX + iStartX != s_pOldXs[k])
					{
						bLineErased = true;
						iIndxToEraseFrom = k;
					}

					pRun->setX(iCurX + iStartX, FP_CLEARSCREEN_NEVER);
				}
			}
		break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	} //switch eAlignment

	if(bLineErased)
	{
		xxx_UT_DEBUGMSG(("fp_Line::layout (0x%x): clearling line from indx %d\n", this, iIndxToEraseFrom));
		clearScreenFromRunToEnd(iIndxToEraseFrom);
	}
	else
		xxx_UT_DEBUGMSG(("fp_Line::layout (0x%x): nothing to clear\n", this));

}

bool fp_Line::containsFootnoteReference(void)
{
	fp_Run * pRun = NULL;
	UT_sint32 i =0;
	bool bFound = false;
	for(i=0; (i< countRuns()) && !bFound; i++)
	{
		pRun = getRunFromIndex((UT_uint32)i);
		if(pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun * pFRun = (fp_FieldRun *) pRun;
			if(pFRun->getFieldType() == FPFIELD_footnote_ref)
			{
				bFound = true;
				break;
			}
		}
	}
	return bFound;
}

bool fp_Line::getFootnoteContainers(UT_Vector * pvecFoots)
{
	fp_Run * pRun = NULL;
	UT_uint32 i =0;
	bool bFound = false;
	fp_FootnoteContainer * pFC = NULL;
	PT_DocPosition posStart = getBlock()->getPosition();
	PT_DocPosition posEnd = posStart + getLastRun()->getBlockOffset() + getLastRun()->getLength();
	posStart += getFirstRun()->getBlockOffset();
	for(i=0; (i< (UT_uint32) countRuns()); i++)
	{
		pRun = getRunFromIndex(i);
		if(pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun * pFRun = (fp_FieldRun *) pRun;
			if(pFRun->getFieldType() == FPFIELD_footnote_ref)
			{
				fp_FieldFootnoteRefRun * pFNRun = (fp_FieldFootnoteRefRun *) pFRun;
				fl_FootnoteLayout * pFL = getBlock()->getDocLayout()->findFootnoteLayout(pFNRun->getPID());
				
				UT_ASSERT(pFL);
				xxx_UT_DEBUGMSG(("Pos of footnote %d start of run %d end of run %d \n",pFL->getDocPosition(),posStart,posEnd));
				if(pFL && pFL->getDocPosition()>= posStart && pFL->getDocPosition() <= posEnd)
				{
					pFC = (fp_FootnoteContainer *) pFL->getFirstContainer();
					bFound = true;
					pvecFoots->addItem((void *) pFC);
				}
			}
		}
	}
	if(bFound)
	{
		UT_DEBUGMSG(("Found %d footnotes on this line %x \n",pvecFoots->getItemCount(),this));
	}
	return bFound;
}

void fp_Line::setX(UT_sint32 iX, bool bDontClearIfNeeded)
{
	if (m_iX == iX)
	{
		return;
	}
	if(!bDontClearIfNeeded)
	{
		clearScreen();
	}
	m_iX = iX;
}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
void fp_Line::setXInLayoutUnits(UT_sint32 iX)
{
	m_iXLayoutUnits = iX;
}
#endif

void fp_Line::setY(UT_sint32 iY)
{
	if (m_iY == iY)
	{
		return;
	}

	clearScreen();
	m_iY = iY;
}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
void fp_Line::setYInLayoutUnits(UT_sint32 iY)
{
	m_iYLayoutUnits = iY;
}
#endif

UT_sint32 fp_Line::getMarginBefore(void) const
{
	if (isFirstLineInBlock() && getBlock()->getPrev())
	{
		fp_Container* pPrevLine = (fp_Container *) getBlock()->getPrev()->getLastContainer();
		UT_ASSERT(pPrevLine);
		UT_sint32 iBottomMargin = 0;
		if(pPrevLine->getContainerType() == FP_CONTAINER_LINE)
		{
			iBottomMargin = static_cast<fp_Line *>(pPrevLine)->getBlock()->getBottomMargin();
		}
		else if(pPrevLine->getContainerType() == FP_CONTAINER_TABLE)
		{
			iBottomMargin = static_cast<fl_TableLayout *>(pPrevLine->getSectionLayout())->getBottomOffset();
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			iBottomMargin = 0;
		}
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
		fp_Container * pNext = (fp_Container *) getBlock()->getNext()->getFirstContainer();
		if (!pNext)
			return 0;
		if(pNext->getContainerType() != FP_CONTAINER_LINE)
		{
			return getBlock()->getBottomMargin();
		}
		fp_Line * pNextLine = (fp_Line *) pNext;

		UT_ASSERT(pNextLine->isFirstLineInBlock());

		UT_sint32 iBottomMargin = getBlock()->getBottomMargin();

		UT_sint32 iNextTopMargin = pNextLine->getBlock()->getTopMargin();

		UT_sint32 iMargin = UT_MAX(iBottomMargin, iNextTopMargin);

		return iMargin;
	}

	return 0;
}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
UT_sint32 fp_Line::getMarginAfterInLayoutUnits(void) const
{
	if (isLastLineInBlock() && getBlock()->getNext())
	{
		fp_Container * pNext = (fp_Container *) getBlock()->getNext()->getFirstContainer();
		if (!pNext)
			return 0;
		if(pNext->getContainerType() != FP_CONTAINER_LINE)
		{
			return getBlock()->getBottomMarginInLayoutUnits();
		}
		fp_Line * pNextLine = (fp_Line *) pNext;
		UT_ASSERT(pNextLine->isFirstLineInBlock());

		UT_sint32 iBottomMargin = getBlock()->getBottomMarginInLayoutUnits();

		UT_sint32 iNextTopMargin = pNextLine->getBlock()->getTopMarginInLayoutUnits();

		UT_sint32 iMargin = UT_MAX(iBottomMargin, iNextTopMargin);

		return iMargin;
	}

	return 0;
}
#endif

bool fp_Line::recalculateFields(UT_uint32 iUpdateCount)
{
	bool bResult = false;

	UT_uint32 iNumRuns = m_vecRuns.getItemCount();
	for (UT_uint32 i = 0; i < iNumRuns; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		if (pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun* pFieldRun = (fp_FieldRun*) pRun;
			if(iUpdateCount && (iUpdateCount % pFieldRun->needsFrequentUpdates()))
				continue;
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
					 "			iPosition %d, iTabStopPosition %d, iType %d, iLeader %d\n",
					 iStartX, m_iMaxWidth,iPosition, iTabStopPosition,(UT_sint32)iType, (UT_sint32)iLeader));
		return false;
	}
}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
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
					 "			 iPosition %d, iTabStopPosition %d,iType %d, iLeader %d\n",
					 iStartX, m_iMaxWidthLayoutUnits,iPosition, iTabStopPosition,(UT_sint32)iType, (UT_sint32)iLeader));
		return false;
	}
}
#endif

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
					 "			iPosition %d, iTabStopPosition %d, iType %d, iLeader %d\n",
					 iStartX, m_iMaxWidth,iPosition, iTabStopPosition, (UT_sint32)iType, (UT_sint32)iLeader));
		return false;
	}
}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
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
					 "		   iPosition %d, iTabStopPosition %d, iType %d, iLeader %d\n",
					 iStartX, m_iMaxWidthLayoutUnits,iPosition, iTabStopPosition,(UT_sint32)iType, (UT_sint32)iLeader));
		return false;
	}
}
#endif

void fp_Line::recalcMaxWidth(bool bDontClearIfNeeded)
{
	UT_sint32 iX = m_pBlock->getLeftMargin();
	UT_sint32 iMaxWidth = getContainer()->getWidth();

	FriBidiCharType iBlockDir = m_pBlock->getDominantDirection();

	if (isFirstLineInBlock())
	{
		if(iBlockDir == FRIBIDI_TYPE_LTR)
			iX += m_pBlock->getTextIndent();
	}

	setX(iX,bDontClearIfNeeded);

	UT_ASSERT(iMaxWidth > 0);

	fl_DocSectionLayout * pSL =  getBlock()->getDocSectionLayout();
	UT_ASSERT(pSL);
	if(pSL->getNumColumns() > 1)
	{
		if(getContainer()->getContainerType() == FP_CONTAINER_COLUMN ||
			getContainer()->getContainerType() == FP_CONTAINER_COLUMN_SHADOW ||
			getContainer()->getContainerType() == FP_CONTAINER_HDRFTR ||
			getContainer()->getContainerType() == FP_CONTAINER_FOOTNOTE)
		{
			m_iClearToPos = iMaxWidth + pSL->getColumnGap();
			m_iClearLeftOffset = pSL->getColumnGap() - _UL(1);
		}
		else if(getContainer()->getContainerType() == FP_CONTAINER_CELL)
		{
			fp_CellContainer * pCell = (fp_CellContainer *) getContainer();
			m_iClearToPos = (UT_sint32)(iMaxWidth + pCell->getRightPad() * SCALE_TO_SCREEN);
//			m_iClearLeftOffset =  pCell->getCellX(this) - pCell->getLeftPos() - _UL(1);
			m_iClearLeftOffset =  0;
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			m_iClearToPos = iMaxWidth;
			m_iClearLeftOffset = pSL->getLeftMargin() - _UL(1);
		}
	}
	else
	{
		if(getContainer()->getContainerType() == FP_CONTAINER_COLUMN ||
			getContainer()->getContainerType() == FP_CONTAINER_COLUMN_SHADOW ||
			getContainer()->getContainerType() == FP_CONTAINER_HDRFTR ||
			getContainer()->getContainerType() == FP_CONTAINER_FOOTNOTE)
		{
			m_iClearToPos = iMaxWidth + pSL->getRightMargin() - _UL(2);
			m_iClearLeftOffset = pSL->getLeftMargin() - _UL(1);
		}
		else if(getContainer()->getContainerType() == FP_CONTAINER_CELL)
		{
			fp_CellContainer * pCell = (fp_CellContainer *) getContainer();
			m_iClearToPos = (UT_sint32)(iMaxWidth + pCell->getRightPad() * SCALE_TO_SCREEN);
//			m_iClearLeftOffset =  pCell->getCellX(this) - pCell->getLeftPos() - _UL(1);
			m_iClearLeftOffset =  0;
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			m_iClearToPos = iMaxWidth;
			m_iClearLeftOffset = pSL->getLeftMargin() - _UL(1);
		}
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

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	
	// Do same calculation but in layout units.

	iX = m_pBlock->getLeftMarginInLayoutUnits();

	if (isFirstLineInBlock())
	{
		if(iBlockDir == FRIBIDI_TYPE_LTR)
			iX += m_pBlock->getTextIndentInLayoutUnits();
	}

	setXInLayoutUnits(iX);

	iMaxWidth = getContainer()->getWidthInLayoutUnits();
	iMaxWidth -= m_pBlock->getRightMarginInLayoutUnits();
	iMaxWidth -= m_pBlock->getLeftMarginInLayoutUnits();
	if (isFirstLineInBlock())
	{
		iMaxWidth -= m_pBlock->getTextIndentInLayoutUnits();
	}

	// Check that there's actually room for content
	UT_ASSERT(iMaxWidth > 0);

	setMaxWidthInLayoutUnits(iMaxWidth);
#endif
}

fp_Container*	fp_Line::getNextContainerInSection(void) const
{
	if (getNext())
	{
		return (fp_Container *)getNext();
	}

	fl_ContainerLayout* pNextBlock = m_pBlock->getNext();
	if (pNextBlock)
	{
		return (fp_Container *) pNextBlock->getFirstContainer();
	}
	return NULL;
}

fp_Container*	fp_Line::getPrevContainerInSection(void) const
{
	if (getPrev())
	{
		return (fp_Container *) getPrev();
	}

	fl_ContainerLayout* pPrev =  (fl_ContainerLayout *) m_pBlock->getPrev();
	if(pPrev)
	{
		fp_Container * pPrevCon = (fp_Container *) pPrev->getLastContainer();
//
// Have to handle broken tables in the previous layout..
//
		if(pPrevCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_TableContainer * pTab = (fp_TableContainer *) pPrevCon;
			fp_TableContainer * pLLast = pTab;
			fp_TableContainer * pNext = (fp_TableContainer *) pTab->getNext();
			while(pNext)
			{
				pLLast = pNext;
				pNext = (fp_TableContainer *) pNext->getNext();
			}
			pPrevCon = (fp_Container *) pLLast;
		}
		return pPrevCon;
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

	FV_View* pView = getBlock()->getDocLayout()->getView();
	bool bShowHidden = pView->getShowPara();

	// first calc the width of the line
	for (UT_uint32 i = 0; i < iCountRuns; ++i)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		FPVisibility eHidden  = pRun->isHidden();
		if((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
		   || eHidden == FP_HIDDEN_REVISION
		   || eHidden == FP_HIDDEN_REVISION_AND_TEXT)
			continue;

		iX += pRun->getWidth();
	}
	// this is a wrong assert, since line can include trailing spaces
	// that are out of the margins.
	//UT_ASSERT(iX <= m_iMaxWidth);

	m_iWidth = iX;

	return iX;
}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
UT_sint32 fp_Line::calculateWidthOfLineInLayoutUnits(void)
{
	UT_uint32 iCountRuns = m_vecRuns.getItemCount();
	UT_sint32 iX = 0;
	UT_uint32 i;

	FV_View* pView = getBlock()->getDocLayout()->getView();
	bool bShowHidden = pView->getShowPara();

	// first calc the width of the line
	for (i=0; i<iCountRuns; i++)
	{
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(i);

		FPVisibility eHidden  = pRun->isHidden();
		if((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
		   || eHidden == FP_HIDDEN_REVISION
		   || eHidden == FP_HIDDEN_REVISION_AND_TEXT)
			continue;

		iX += pRun->getWidthInLayoutUnits();
	}

	m_iWidthLayoutUnits = iX;

	return iX;
}
#endif

UT_sint32 fp_Line::calculateWidthOfTrailingSpaces(void)
{
	// need to move back until we find the first non blank character and
	// return the distance back to this character.

	UT_ASSERT(!isEmpty());

	UT_sint32 iTrailingBlank = 0;


	FriBidiCharType iBlockDir = m_pBlock->getDominantDirection();
	UT_sint32 i;
	UT_sint32 iCountRuns = m_vecRuns.getItemCount();

	FV_View* pView = getBlock()->getDocLayout()->getView();
	bool bShowHidden = pView->getShowPara();

	for (i=iCountRuns -1 ; i >= 0; i--)
	{
		// work from the run on the visual end of the line
		UT_sint32 k = iBlockDir == FRIBIDI_TYPE_LTR ? i : iCountRuns - i - 1;
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(k));

		FPVisibility eHidden  = pRun->isHidden();
		if((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
		   || eHidden == FP_HIDDEN_REVISION
		   || eHidden == FP_HIDDEN_REVISION_AND_TEXT)
			continue;

		if(!pRun->doesContainNonBlankData())
		{
			iTrailingBlank += pRun->getWidth();
		}
		else
		{
			iTrailingBlank += pRun->findTrailingSpaceDistance();
			break;
		}
	}

	return iTrailingBlank;
}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
UT_sint32 fp_Line::calculateWidthOfTrailingSpacesInLayoutUnits(void)
{
	// need to move back until we find the first non blank character and
	// return the distance back to this character.

	UT_ASSERT(!isEmpty());

	UT_sint32 iTrailingBlank = 0;

	FriBidiCharType iBlockDir = m_pBlock->getDominantDirection();
	UT_sint32 iCountRuns = m_vecRuns.getItemCount();
	UT_sint32 i;

	FV_View* pView = getBlock()->getDocLayout()->getView();
	bool bShowHidden = pView->getShowPara();

	for (i=iCountRuns -1 ; i >= 0; i--)
	{
		// work from the run on the visual end of the line
		UT_sint32 k = iBlockDir == FRIBIDI_TYPE_LTR ? i : iCountRuns - i - 1;
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(k));

		FPVisibility eHidden  = pRun->isHidden();
		if((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
		   || eHidden == FP_HIDDEN_REVISION
		   || eHidden == FP_HIDDEN_REVISION_AND_TEXT)
			continue;

		if(!pRun->doesContainNonBlankData())
		{
			iTrailingBlank += pRun->getWidthInLayoutUnits();
		}
		else
		{
			iTrailingBlank += pRun->findTrailingSpaceDistanceInLayoutUnits();
			break;
		}
	}

	return iTrailingBlank;
}
#endif

UT_uint32 fp_Line::countJustificationPoints(void)
{
	UT_sint32 iCountRuns = m_vecRuns.getItemCount();
	UT_sint32 i;
	UT_uint32 iSpaceCount = 0;
	bool bStartFound = false;

	FriBidiCharType iBlockDir = m_pBlock->getDominantDirection();

	// first calc the width of the line
	for (i=iCountRuns -1 ; i >= 0; i--)
	{
		// work from the run on the visual end of the line
		UT_sint32 k = iBlockDir == FRIBIDI_TYPE_LTR ? i : iCountRuns - i - 1;
		fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(k));

		if (pRun->getType() == FPRUN_TAB)
		{
			// when we hit a tab, we stop this, since tabs "swallow" justification of the
			// runs that preceed them (i.e., endpoint of the tab is given and cannot be
			// moved)
			break;
		}
		else if (pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);
			UT_sint32 iPointCount = pTR->countJustificationPoints();
			if(bStartFound)
			{
				iSpaceCount += abs(iPointCount);
			}
			else
			{
				if(iPointCount >= 0)
				{
					iSpaceCount += iPointCount;
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
		// because the justification means that the spaces are wider than the OS
		// will draw them, we cannot have runs merged across the spaces

		// also, we have to do the spliting  _before_ we can count justification points,
		// otherwise we get problems if the last non blank run ends in spaces and
		// is followed by some space-only runs; in that case the trailing spaces in
		// the non-blank run get counted in when they should not -- this should not cost us
		// too much, since it is unlikely that there is going to be a justified line with
		// no spaces on it

#if 0
		// to avoid spliting the runs at spaces, saving memory and
		// processing time, we now improved fp_TextRun::_draw(), so
		// that it is able to skip over spaces

		_splitRunsAtSpaces();
#endif
		
		UT_uint32 iSpaceCount = countJustificationPoints();
		xxx_UT_DEBUGMSG(("fp_Line::distributeJustificationAmongstSpaces: iSpaceCount %d\n", iSpaceCount));

		if(iSpaceCount)
		{
			bool bFoundStart = false;

			FriBidiCharType iBlockDir = m_pBlock->getDominantDirection();
			UT_sint32 count = m_vecRuns.getItemCount();
			UT_ASSERT(count);

		xxx_UT_DEBUGMSG(("DOM: must split iAmount %d between iSpaceCount %d spaces for count %d runs\n", iAmount, iSpaceCount, count));

			for (UT_sint32 i=count-1; i >= 0 && iSpaceCount > 0; i--)
			{
				// work from the run on the visual end of the line
				UT_sint32 k = iBlockDir == FRIBIDI_TYPE_LTR ? i : count  - i - 1;
				fp_Run* pRun = (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(k));

				if (pRun->getType() == FPRUN_TAB)
				{
					UT_ASSERT(iSpaceCount == 0);
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					break;
				}
				else if (pRun->getType() == FPRUN_TEXT)
				{
					fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);

					UT_sint32 iSpacesInText = pTR->countJustificationPoints();

					if(!bFoundStart && iSpacesInText >= 0)
						bFoundStart = true;

					if(bFoundStart && iSpacesInText)
					{
						UT_uint32 iMySpaces = abs(iSpacesInText);
						UT_sint32 iJustifyAmountForRun = (int)((double)iAmount / (iSpaceCount-1) * iMySpaces);
						if (iSpaceCount == 1) iJustifyAmountForRun = 0;
						pTR->distributeJustificationAmongstSpaces(iJustifyAmountForRun, iMySpaces);

						iAmount -= iJustifyAmountForRun;
						iSpaceCount -= iMySpaces;
					}
					else if(!bFoundStart && iSpacesInText)
					{
						// trailing space, need to do this so that the trailing spaces do not get merged
						// with the last non-blank run (see fp_TextRun::distributeJustificationAmongstSpaces()
						pTR->distributeJustificationAmongstSpaces(0, 0);
					}
				}
			}
		}
	}
}

/*
    I was going split the line from the end up to the last visual tab,
	but in the bidi build this would be extremely expensive because the
	calculation of visual coordinace for the run requires that after every
	split we would recalculated the bidi map, and that is not worth it
*/

void fp_Line::_splitRunsAtSpaces(void)
{
	UT_uint32 count = m_vecRuns.getItemCount();
	if(!count)
		return;

	UT_uint32 countOrig = count;

	for (UT_uint32 i = 0; i < count; i++)
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
				addDirectionUsed(pRun->getDirection(),false);
				pTR->split(iSpacePosition + 1);
				count++;
			}
		}
	}

	fp_Run* pRun = getLastRun();

	if (pRun->getType() == FPRUN_TEXT)
	{
		fp_TextRun* pTR = (fp_TextRun *)pRun;
		UT_sint32 iSpacePosition = pTR->findCharacter(0, UCS_SPACE);

		if ((iSpacePosition > 0) &&
			((UT_uint32) iSpacePosition < pTR->getBlockOffset() + pTR->getLength() - 1))
		{
			addDirectionUsed(pRun->getDirection(),false);
			pTR->split(iSpacePosition + 1);
		}
	}

	count = m_vecRuns.getItemCount();
	if(count != countOrig)
	{
		m_bMapDirty = true;
		_createMapOfRuns();
	}
}

/*!
	Creates a map for conversion from visual to logical position of runs on the line.
	\param void
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

		if(count == 1)	 //if there is just one run, then make sure that it maps on itself and return
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
			s_pMapOfRunsL2V = new FriBidiStrIndex[s_iMapOfRunsSize];
			s_pMapOfRunsV2L = new FriBidiStrIndex[s_iMapOfRunsSize];
			s_pPseudoString    = new FriBidiChar[s_iMapOfRunsSize];
			s_pEmbeddingLevels =  new FriBidiLevel[s_iMapOfRunsSize];


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

			s_pMapOfRunsL2V = new FriBidiStrIndex[s_iMapOfRunsSize];
			s_pMapOfRunsV2L = new FriBidiStrIndex[s_iMapOfRunsSize];
			s_pPseudoString    = new FriBidiChar[RUNS_MAP_SIZE];
			s_pEmbeddingLevels =  new FriBidiLevel[RUNS_MAP_SIZE];


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

			if(count % 2)	//the run in the middle
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

UT_uint32	fp_Line::getVisIndx(fp_Run* pRun)
{
	UT_sint32 i = m_vecRuns.findItem((void *) pRun);
	UT_ASSERT(i >= 0);
	return _getRunVisIndx((UT_uint32) i);
}

fp_Run *	fp_Line::getRunAtVisPos(UT_uint32 i)
{
	if(i >= m_vecRuns.getItemCount())
		return NULL;
	return (fp_Run*) m_vecRuns.getNthItem(_getRunLogIndx(i));
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
		//_createMapOfRuns();
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
		//_createMapOfRuns();
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

/*!
    Scan through the runs on this line, checking for footnote anchor
    fields.  Return true if so.
*/
void fp_Line::_updateContainsFootnoteRef(void)
{
	m_bContainsFootnoteRef = false;

	UT_uint32 count = m_vecRuns.getItemCount();
	for (UT_uint32 i = 0; i < count; i++)
	{
		fp_Run * r = (fp_Run *)m_vecRuns.getNthItem(i);
		if (r->getType() == FPRUN_FIELD)
		{
			fp_FieldRun * fr = (fp_FieldRun*) r;
			if (fr->getFieldType() == FPFIELD_endnote_ref)
				m_bContainsFootnoteRef = true;
		}
	}
}

UT_sint32 fp_Line::getDrawingWidth() const
{
	if(isLastLineInBlock())
	{
		fp_Run * pRun = getLastRun();
		UT_return_val_if_fail(pRun && pRun->getType() == FPRUN_ENDOFPARAGRAPH, m_iWidth);
		return (m_iWidth + ((fp_EndOfParagraphRun*)pRun)->getDrawingWidth());
	}
	else
	{
		return m_iWidth;
	}
}
