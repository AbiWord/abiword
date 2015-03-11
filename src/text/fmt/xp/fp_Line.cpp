/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
#include "fp_FootnoteContainer.h"
#include "fp_FrameContainer.h"
#include "ut_color.h"

#ifdef USE_STATIC_MAP
//initialize the static members of the class
UT_UCS4Char * fp_Line::s_pPseudoString = 0;
UT_uint32   * fp_Line::s_pMapOfRunsL2V = 0;
UT_uint32   * fp_Line::s_pMapOfRunsV2L = 0;
UT_Byte     * fp_Line::s_pEmbeddingLevels = 0;
UT_sint32     fp_Line::s_iMapOfRunsSize = 0;
fp_Line     * fp_Line::s_pMapOwner = 0;
#else
//make sure that any references to the static members are renamed to their non-static versions
#define s_iMapOfRunsSize m_iMapOfRunsSize
#define s_pMapOfRuns m_pMapOfRuns
#endif

#define STATIC_BUFFER_INITIAL 150

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
	m_iY(INITIAL_OFFSET), // So setY(0) triggers a clearscreen and redraw!
		            // I do not like this at all; we have no business
		            // of clearing at fictional coordinances
	//m_bRedoLayout(true),
	m_bNeedsRedraw(false),
	m_bMapDirty(true), //map that has not been initialized is dirty by deafault
	m_iRunsRTLcount(0),
	m_iRunsLTRcount(0),
	m_bIsCleared(true),
	m_bContainsFootnoteRef(false),
	m_bIsWrapped(false),
	m_bIsSameYAsPrevious(false),
	m_bIsAlongTopBorder(false),
	m_bIsAlongBotBorder(false),
	m_iAdditionalMarginAfter(0),
	m_iLeftThick(0),
	m_iRightThick(0),
	m_iTopThick(0),
	m_iBotThick(0)
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
		s_pMapOfRunsL2V = new UT_uint32 [RUNS_MAP_SIZE];
		s_pMapOfRunsV2L = new UT_uint32[RUNS_MAP_SIZE];
		s_pPseudoString    = new UT_UCS4Char[RUNS_MAP_SIZE];
		s_pEmbeddingLevels =  new UT_Byte[RUNS_MAP_SIZE];
		s_iMapOfRunsSize = RUNS_MAP_SIZE;
	}
    #else
	m_pMapOfRunsL2V = new UT_uint32[RUNS_MAP_SIZE];
	m_pMapOfRunsV2L = new UT_uint32[RUNS_MAP_SIZE];
	m_pPseudoString    = new UT_UCS4Char[RUNS_MAP_SIZE];
	m_pEmbeddingLevels =  new UT_Byte[RUNS_MAP_SIZE];
	m_iMapOfRunsSize = RUNS_MAP_SIZE;
    #endif

	UT_ASSERT(s_pMapOfRunsL2V && s_pMapOfRunsV2L && s_pPseudoString && s_pEmbeddingLevels);

	++s_iClassInstanceCounter; // this tells us how many instances of Line are out there
							   //we use this to decide whether the above should be
							   //deleted by the destructor

	UT_ASSERT((getPrev() == NULL));
	UT_ASSERT((getNext() == NULL));
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
	xxx_UT_DEBUGMSG(("Line %x delete refCount %d \n",this,getRefCount()));
	//UT_ASSERT(getRefCount() == 0);
}

//
// All these methods need to be adjusted to take account of the thinkness
// of the borders drawn around paragraphs. The borders take up additional space 
// within the dimensions of the lines assigned by the layout calculations.
//
//
// Calculation of the border thickness occurs when:
// 1. The previous paragraph changes it's properties
// 2. The line is placed in a different container
// 3. When the next or previous line is placed in a different container.

// The Ascent of the line is the  max ascent of all the runs (plus the top 
// border thickness for first line of a block)
UT_sint32 fp_Line::getAscent(void) const
{
    if (getBlock() && getBlock()->hasBorders() && isAlongTopBorder())
    {
	return m_iAscent + getTopThick();
    }
    else
    {
	return m_iAscent;
    }
}

//
// The Descent is the max descent of all the runs (plus the bottom 
// border thickness for last line of a block)
UT_sint32 fp_Line::getDescent(void) const
{
    if (getBlock() && getBlock()->hasBorders() && isAlongBotBorder())
    {
	return m_iDescent + getBotThick();
    }
    else
    {
	return m_iDescent;
    }
}

//
// The height is calculated from the sum of all the getAscents and getDescents
// so has the height of the top and bot borders included.
//
UT_sint32 fp_Line::getHeight(void) const
{
  return m_iHeight;
}

void fp_Line::drawBorders(GR_Graphics * pG)
{
  if(!getBlock())
      return;
  fp_Line * pFirst = const_cast<fp_Line *>(getFirstInContainer());
  bool bDrawTop = false;
  bool bDrawBot = false;
  if(!pFirst)
      return;
  fp_Line * pLast = const_cast<fp_Line *>(getLastInContainer());
  if(!pLast)
      return;
  if(pFirst->canDrawTopBorder())
      bDrawTop = true;
  if(pLast->canDrawBotBorder())
      bDrawBot = true;

  UT_Rect * pFirstR = pFirst->getScreenRect();
  if(!pFirstR)
      return;
  UT_Rect * pLastR = pLast->getScreenRect();
  if(!pLastR)
  {
      delete pFirstR;
      return;
  }
  UT_Rect * pConR = static_cast<fp_VerticalContainer *>(getContainer())->getScreenRect();
  if(!pConR)
  {
      delete pFirstR;
      delete pLastR;
      return;
  }
  UT_sint32 iTop = pFirstR->top;
  UT_sint32 iBot = pLastR->top + pLastR->height;
  UT_sint32 iLeft = pConR->left + getLeftEdge();
  UT_sint32 iRight = pConR->left + getRightEdge();
  if(getBlock()->getBottom().m_t_linestyle > 1)
  {
      iBot = iBot-getBlock()->getBottom().m_thickness;
  }
  //
  // Now correct for printing
  //
  fp_Page * pPage = getPage();
  if(pPage == NULL)
       return;
  if(pPage->getDocLayout()->getView() && pG->queryProperties(GR_Graphics::DGP_PAPER))
  {
       UT_sint32 xdiff,ydiff;
       pPage->getDocLayout()->getView()->getPageScreenOffsets(pPage, xdiff, ydiff);
       iTop = iTop - ydiff;
       iBot = iBot - ydiff;
       iLeft = iLeft - xdiff;
       iRight = iRight - xdiff;
       if(pPage->getDocLayout()->getView()->getViewMode() != VIEW_PRINT)
       {
	    iTop += static_cast<fl_DocSectionLayout *>(getSectionLayout()->getDocSectionLayout())->getTopMargin();
	    iBot += static_cast<fl_DocSectionLayout *>(getSectionLayout()->getDocSectionLayout())->getTopMargin();
       }
  }

  //
  // Draw top border
  PP_PropertyMap::Line line;

  line = getBlock()->getLeft();
  iLeft += line.m_thickness/2;
  line = getBlock()->getRight();
  iRight -= line.m_thickness/2;
 
  if(bDrawTop && (getBlock()->getTop().m_t_linestyle > 1))
  {
      line = getBlock()->getTop();
      drawLine(line,iLeft,iTop,iRight,iTop,pG);
  }
  if(getBlock()->getLeft().m_t_linestyle > 1)
  {
      line = getBlock()->getLeft();
      drawLine(line,iLeft,iTop,iLeft,iBot,pG);
  }
  if(getBlock()->getRight().m_t_linestyle > 1)
  {
      line = getBlock()->getRight();
      drawLine(line,iRight,iTop,iRight,iBot,pG);
  }
  if(bDrawBot && (getBlock()->getBottom().m_t_linestyle > 1))
  {
      line = getBlock()->getBottom();
      drawLine(line,iLeft,iBot,iRight,iBot,pG);
  }
  delete pFirstR;
  delete pLastR;
  delete pConR;
}

/*!
 * The left most esge of the paragraph relative to it's container.
 */
UT_sint32 fp_Line::getLeftEdge(void) const
{
        if(!getBlock())
	     return 0;
	UT_sint32 iLeft = getBlock()->getLeftMargin();
	if(getBlock()->getTextIndent() < 0)
	{
	     iLeft += getBlock()->getTextIndent();
	}
	return iLeft;
}


/*!
 * The left most esge of the paragraph relative to it's container.
 */
UT_sint32 fp_Line::getRightEdge(void) const
{
	fp_VerticalContainer * pVCon = static_cast<fp_VerticalContainer *>(getContainer());
	if(!pVCon)
	  return getMaxWidth();
	if(!getBlock())
	  return getMaxWidth();
	UT_sint32 iRight = pVCon->getWidth() - getBlock()->getRightMargin();
	return iRight;
}

/*!
 * The absolue left and right edge of the paragraph in terms of the
 * current graphics context.
 */
bool    fp_Line::getAbsLeftRight(UT_sint32& left,UT_sint32& right)
{
	fp_VerticalContainer * pVCon = static_cast<fp_VerticalContainer *>(getContainer());
	if(!pVCon)
	  return false;
	if(!getBlock())
	  return false;
	UT_Rect * pR = pVCon->getScreenRect();
	left = pR->left + getLeftEdge();
	right = pR->left + pVCon->getWidth() - getBlock()->getRightMargin();
	delete pR;
	//
	// Correct for printing
	//
	fp_Page * pPage = getPage();
	if(pPage == NULL)
	    return false;
	if(pPage->getDocLayout()->getView() &&  getGraphics()->queryProperties(GR_Graphics::DGP_PAPER))
	{
	    UT_sint32 xdiff,ydiff;
	    pPage->getDocLayout()->getView()->getPageScreenOffsets(pPage, xdiff, ydiff);
	    left = left - xdiff;
	    right= right - xdiff;
	}
	return true;
}
//
// The location of the start of the line should take account of the
// LeftBorder thickness
//
UT_sint32 fp_Line::getX(void) const
{
  return m_iX;
}

//
// The top of the line locates the position of the line including
// the top border thickness
//
UT_sint32 fp_Line::getY(void) const
{
  return m_iY;
}

//
// The calculation of the MaxWidth includes the effects of the thickness
// of the left and right borders
//
UT_sint32 fp_Line::getMaxWidth(void) const
{
  return m_iMaxWidth;
}

UT_sint32 fp_Line::getAvailableWidth(void) const
{
  return getMaxWidth() - getLeftThick() - getRightThick();
}

UT_sint32 fp_Line::calcLeftBorderThick(void)
{
  m_iLeftThick = 0;
  if(getBlock() && !getBlock()->hasBorders())
  {
      m_iLeftThick = 0;
  }
  else if(getBlock())
  {
      bool bGetThick = true;
      if(!getPrev() || getPrev()->getContainerType() != FP_CONTAINER_LINE)
      {
	  bGetThick = true;
      }
      else if (isSameYAsPrevious())
      {
	  bGetThick = false;
      }
      if(bGetThick)
      {
	  m_iLeftThick =getBlock()->getLeft().m_thickness + getBlock()->getLeft().m_spacing;
      }
  }
  return m_iLeftThick;
}

UT_sint32 fp_Line::calcRightBorderThick(void)
{
  m_iRightThick = 0;
  if(getBlock() && !getBlock()->hasBorders())
  {
      m_iRightThick = 0;
  }
  else if(getBlock())
  {
      bool bGetThick = true;
      if(!getNext() || getNext()->getContainerType() != FP_CONTAINER_LINE)
      {
	  bGetThick = true;
      }
      else if (static_cast<fp_Line *>(getNext())->isSameYAsPrevious())
      {
	  bGetThick = false;
      }
      if(bGetThick)
      {
	  m_iRightThick =getBlock()->getRight().m_thickness + getBlock()->getRight().m_spacing;
      }
  }
  return m_iRightThick;
}

bool fp_Line::hasBordersOrShading(void) const
{
  if(getBlock() && (getBlock()->hasBorders() || (getBlock()->getPattern() > 0)))
  {
    return true;
  }
  return false;
}

UT_sint32 fp_Line::calcTopBorderThick(void)
{
  m_iTopThick = 0;
  if(getBlock() && !getBlock()->hasBorders())
  {
       m_iTopThick = 0;
  }
  else if(getBlock() && canDrawTopBorder())
  {
       m_iTopThick = getBlock()->getTop().m_thickness + getBlock()->getTop().m_spacing;
  }
  return m_iTopThick;
}

UT_sint32 fp_Line::calcBotBorderThick(void)
{
  m_iBotThick = 0;
  if(getBlock() && !getBlock()->hasBorders())
  {
       m_iBotThick = 0;
  }
  else if(getBlock() && canDrawBotBorder())
  {
       m_iBotThick = getBlock()->getBottom().m_thickness + getBlock()->getBottom().m_spacing;
  }
  return m_iBotThick;
}

void fp_Line::calcBorderThickness(void)
{
    calcLeftBorderThick();
    calcRightBorderThick();
    calcTopBorderThick();
    calcBotBorderThick();

// set the boolean flags m_bIsAlongTopBorder and m_bIsAlongBotBorder
    if (canDrawTopBorder())
    {
	if (isFirstLineInBlock())
	{
	    m_bIsAlongTopBorder = true;
	}
	if (isSameYAsPrevious())
	{
	    fp_Line * ppLine = static_cast<fp_Line *> (getPrev());
	    while(ppLine && ppLine->isSameYAsPrevious())
	    {
		ppLine = static_cast<fp_Line *>(ppLine->getPrev());
	    }
	    if (ppLine && ppLine->isFirstLineInBlock())
	    { 
		m_bIsAlongTopBorder = true;
	    }
	}
    }
    if(canDrawBotBorder())
    {
	if (isLastLineInBlock())
	{
	    m_bIsAlongBotBorder = true;
	}
	if (isWrapped())
	{
	    fp_Line * npLine = static_cast<fp_Line *>(getNext());;
	    if (npLine && isSameYAsPrevious())
	    {
		do
		{
		    if (npLine->isLastLineInBlock())
		    {
			m_bIsAlongBotBorder = true;
			break;
		    }
		    npLine = static_cast<fp_Line *>(npLine->getNext());
		}
		while(npLine && npLine->isSameYAsPrevious());
	    }
	}
	if (m_bIsAlongBotBorder)
	{
	    fp_Line * ppLine =this;
	    while(ppLine && ppLine->isSameYAsPrevious())
	    {
		ppLine = static_cast<fp_Line *>(ppLine->getPrev());
	    }
	    if(ppLine)
	      ppLine = static_cast<fp_Line *> (ppLine->getPrev());
	    while (ppLine && ppLine->isAlongBotBorder())
	    {
		ppLine->setAlongBotBorder(false);
		ppLine->recalcHeight();
	    }
	}
    }

    if (isFirstLineInBlock() && !canDrawTopBorder())
    {
	fl_BlockLayout *pBl = static_cast < fl_BlockLayout * > (getBlock()->getPrev());
	fp_Line *pLine = static_cast < fp_Line * > (pBl->getLastContainer());
	if(pLine && pLine->isAlongBotBorder())
	{
	    pBl->setLineHeightBlockWithBorders(-1);
	}
    }
}

const fp_Line * fp_Line::getFirstInContainer(void) const
{
  const fp_Container * pMyCon = getContainer();
  if(pMyCon == NULL)
    return NULL;
  const fp_ContainerObject * pPrev = getPrev();
  const fp_ContainerObject * pCurrent = static_cast<const fp_ContainerObject*>(this);
  while(pPrev && (pPrev->getContainerType() == FP_CONTAINER_LINE) &&
	(static_cast<const fp_Line *>(pPrev)->getBlock()) &&
	(static_cast<const fp_Line *>(pPrev)->getBlock() == getBlock()) &&
	(static_cast<const fp_Line *>(pPrev)->getContainer() == pMyCon))
  {
        pCurrent = pPrev;
        pPrev = pPrev->getPrev();
  }
  return static_cast<const fp_Line *>(pCurrent);
}

const fp_Line * fp_Line::getLastInContainer(void) const
{
  const fp_Container * pMyCon = getContainer();
  if(pMyCon == NULL)
    return NULL;
  const fp_ContainerObject * pNext = getNext();
  const fp_ContainerObject * pCurrent = static_cast<const fp_ContainerObject *>(this);
  while(pNext && (pNext->getContainerType() == FP_CONTAINER_LINE) &&
	(static_cast<const fp_Line *>(pNext)->getBlock()) &&
	(static_cast<const fp_Line *>(pNext)->getBlock() == getBlock()) &&
	(static_cast<const fp_Line *>(pNext)->getContainer() == pMyCon))
  {
        pCurrent = pNext;
        pNext = pNext->getNext();
  }
  return static_cast<const fp_Line *>(pCurrent);
}

bool fp_Line::canDrawTopBorder(void) const
{
  const fp_Line * pFirst = getFirstInContainer();
  if(pFirst == NULL)
    return false;
  //
  // This line could be wrapped at the same Y as the first line
  //
  if((pFirst != this) && (pFirst->getY() != getY()))
     return false;
  fp_Container * pMyCon = getContainer();
  if(pMyCon == NULL)
    return false;
  if(pFirst == pMyCon->getNthCon(0))
    return true;
  if(!getBlock())
    return true;
  fp_ContainerObject * pPrev = pFirst->getPrevContainerInSection();
  if(!pPrev || (pPrev->getContainerType() != FP_CONTAINER_LINE))
    return true;
  fl_BlockLayout * pPrevBlock = static_cast<fp_Line *>(pPrev)->getBlock();
  if(pPrevBlock->canMergeBordersWithNext())
    return false;
  return (pFirst == this);
}


bool fp_Line::canDrawBotBorder(void) const
{
  const fp_Line * pLast = getLastInContainer();
  if(pLast == NULL)
    return false;
  //
  // This line could be wrapped at the same Y as the last line
  //
  if((pLast != this) && (pLast->getY() != getY()))
     return false;
  fp_Container * pMyCon = getContainer();
  if(pMyCon == NULL)
    return false;
  fp_Container * pNext = pLast->getNextContainerInSection();
  if(pNext == NULL)
    return true;
  fp_Line * pNextL = static_cast<fp_Line *>(pNext);
  if(pNextL->getContainer() == NULL)
    return true;
  if(pNextL->getContainer() != pMyCon)
    return true;
  fl_BlockLayout * pNextBlock = pNextL->getBlock();
  if(pNextBlock->canMergeBordersWithPrev())
    return false;
  return (pLast == this);
}

UT_sint32 fp_Line::getLeftThick(void) const
{
  return m_iLeftThick;
}


UT_sint32 fp_Line::getRightThick(void) const
{
  return m_iRightThick;
}


UT_sint32 fp_Line::getTopThick(void) const
{
  return m_iTopThick;
}


UT_sint32 fp_Line::getBotThick(void) const
{
  return m_iBotThick;
}

#ifdef DEBUG
bool fp_Line::assertLineListIntegrity(void)
{
	UT_sint32 k =0;
	UT_sint32 width = 0;
	xxx_UT_DEBUGMSG(("For line %x \n",this));
	fp_Run * pRunBlock = getFirstRun();
	fp_Run * pRunLine = NULL;
	for(k=0;k<getNumRunsInLine();k++)
	{
		pRunLine = getRunFromIndex(k);
		xxx_UT_DEBUGMSG(("Width of run %d pointer %x width %d \n",k,pRunLine,pRunLine->getWidth()));
		width += pRunLine->getWidth();
		if(pRunLine != pRunBlock)
		{
			UT_DEBUGMSG(("Whoops! bug in Line at run %d %p offset %d Type %d \n",k,pRunLine,pRunLine->getBlockOffset(),pRunLine->getType()));
			pRunLine->printText();
			UT_sint32 i =0;
			for(i=0;i<getNumRunsInLine();i++)
			{
				fp_Run *pRun = getRunFromIndex(i);
				pRun->printText();
				UT_DEBUGMSG(("Line run %d is %p \n",i,pRun));
			}
			UT_ASSERT(pRunLine == pRunBlock);
		}
		UT_return_val_if_fail(pRunBlock,false);
		pRunBlock = pRunBlock->getNextRun();
	}
	xxx_UT_DEBUGMSG(("Line %x Width of line is %d num runs is %d \n",this,width,k)); //   UT_ASSERT(width < getMaxWidth());
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
	return (static_cast<fp_Column *>(getColumn()))->getColumnGap();
}

bool fp_Line::containsOffset(PT_DocPosition blockOffset)
{
	fp_Run * pRun = getFirstVisRun();
	if(blockOffset < pRun->getBlockOffset())
	{
		return false;
	}
	pRun = getLastVisRun();
	if(blockOffset > (pRun->getBlockOffset() + pRun->getLength()))
	{
		return false;
	}
	return true;
}

/*!
 * Return two rectangles that represent the space left to the left and right
 * right of the line where a wrapped object might be.
 */
void fp_Line::genOverlapRects(UT_Rect & recLeft,UT_Rect & recRight)
{
	UT_Rect * pRec = getScreenRect();
	UT_ASSERT(pRec);
	if(pRec == NULL)
	{
		return;
	}
	recLeft.top = pRec->top;
	recRight.top = pRec->top;
	recLeft.height = pRec->height;
	recRight.height = pRec->height;

	UT_sint32 iLeftX = m_pBlock->getLeftMargin();
	UT_sint32 iMaxWidth = getContainer()->getWidth();
	UT_BidiCharType iBlockDir = m_pBlock->getDominantDirection();
	if (isFirstLineInBlock())
	{
		if(iBlockDir == UT_BIDI_LTR)
			iLeftX += m_pBlock->getTextIndent();
	}
	UT_sint32 xdiff = pRec->left - getX();
	fp_Line * pPrev = static_cast<fp_Line *>(getPrev());
	if(pPrev && isSameYAsPrevious())
	{
		recLeft.left = pPrev->getX() + pPrev->getMaxWidth() + xdiff;
		recLeft.width = getX() + xdiff - recLeft.left;
		if(recLeft.width < 0)
		  {
		    UT_DEBUGMSG(("Same Prev left width -ve!! %d \n",recLeft.width));
		  }
	}
	else
	{
		recLeft.left = iLeftX + xdiff;
		recLeft.width = pRec->left - recLeft.left;
		if(recLeft.width < 0)
		  {
		    UT_DEBUGMSG(("RecLeft width -ve!! %d \n",recLeft.width));
		  }
	}
	recRight.left = pRec->left + pRec->width;
	fp_Line * pNext = static_cast<fp_Line *>(getNext());
	if(pNext && pNext->isSameYAsPrevious())
	{
		recRight.width =  pNext->getX() - (getX() + getMaxWidth());
		if(recRight.width < 0)
		  {
		    UT_DEBUGMSG(("Line %p Same Prev RecRight width -ve!! %d \n",this,recRight.width));
		  }
	}
	else
	{
		iMaxWidth -= m_pBlock->getRightMargin();
		recRight.width = iMaxWidth +xdiff - recRight.left;
		if(recRight.width < 0)
		  {
		    UT_DEBUGMSG(("Line %p Not Same RecRight width -ve!! %d \n",this,recRight.width));
		  }
	}
//	UT_ASSERT(recLeft.width >= 0);
//	UT_ASSERT(recRight.width >= 0);
	delete pRec;
}

void fp_Line::setSameYAsPrevious(bool bSameAsPrevious)
{
  if(getMaxWidth() > 9000 && bSameAsPrevious)
    {
      UT_DEBUGMSG(("Same as Previous with Max width %d \n",getMaxWidth())); 
    }
  xxx_UT_DEBUGMSG(("Line %x Same as Previous with Max width %d Same %d \n",this,getMaxWidth(),bSameAsPrevious)); 
  m_bIsSameYAsPrevious = bSameAsPrevious;
}

/*!
 * return an rectangle that covers this object on the screen
 * The calling routine is responsible for deleting the returned struct
 */
UT_Rect * fp_Line::getScreenRect(void)
{
	UT_sint32 xoff = 0;
	UT_sint32 yoff = 0;
	UT_Rect * pRec = NULL; 
	getScreenOffsets(NULL,xoff,yoff);
	if (getBlock() && getBlock()->hasBorders())
	{
		xoff -= getLeftThick();
	}
	pRec= new UT_Rect(xoff,yoff,getMaxWidth(),getHeight());
	return pRec;
}
	
/*!
 * Marks Dirty any runs that overlap the supplied rectangle. This rectangle
 * is relative to the screen.
 */
void fp_Line::markDirtyOverlappingRuns(UT_Rect & recScreen)
{
	UT_Rect * pRec = NULL;
	pRec = getScreenRect();
	if(pRec && recScreen.intersectsRect(pRec))
	{
		DELETEP(pRec);
		fp_Run * pRun = fp_Line::getFirstRun();
		fp_Run * pLastRun = fp_Line::getLastRun();
		while(pRun && pRun != pLastRun)
		{
			pRun->markDirtyOverlappingRuns(recScreen);
			pRun = pRun->getNextRun();
		}
		if(pRun)
		{
			pRun->markDirtyOverlappingRuns(recScreen);
		}
		return;
	}
	DELETEP(pRec);
	return;
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
	else if(pCon->getContainerType() == FP_CONTAINER_FRAME)
        {
	    fp_Page * pPage = static_cast<fp_FrameContainer *>(pCon)->getPage();
	    if(pPage == NULL)
	      return NULL;
	    fp_Container * pCol = static_cast<fp_Container *>(pPage->getNthColumnLeader(0));
	    return pCol;

	}
	else if(pCon->getContainerType() != FP_CONTAINER_CELL)
	{
		return pCon->getColumn();
	}

	fp_CellContainer * pCell = static_cast<fp_CellContainer *>(pCon);
	return pCell->getColumn(this);
}

/*!
 * Returns the page containing this line. Takes account of broken tables.
 */
fp_Page * fp_Line::getPage(void)
{
	fp_Container * pCon = getColumn();
	if(pCon)
	{
		return pCon->getPage();
	}
	else
	{
		return NULL;
	}
}

bool fp_Line::canContainPoint() const
{
	if(!m_pBlock)
		return false;

	return m_pBlock->canContainPoint();
}

/*!
 * Returns true if this is the first line in the block.
 */
bool fp_Line::isFirstLineInBlock(void) const
{
	return (m_pBlock->getFirstContainer() == static_cast<const fp_Container *>(this));
}

/*!
 * Returns true if this is the last line in the block.
 */
bool fp_Line::isLastLineInBlock(void) const
{
	return (m_pBlock->getLastContainer() == static_cast<const fp_Container *>(this));
}

/*!
 * Mark containing block for reformat from the first run of the line.
 */
void fp_Line::setReformat(void)
{
  if(!getFirstRun())
      return;
  UT_sint32 iOff = getFirstRun()->getBlockOffset();
  if(getBlock())
      getBlock()->setNeedsReformat(getBlock(),iOff);
}
void fp_Line::setMaxWidth(UT_sint32 iMaxWidth)
{
	if(iMaxWidth < 60) // This is hardwired to disallow -ve or too small values
	{
	        UT_DEBUGMSG(("Attempt Max width set to... %d now 60 \n",iMaxWidth));
		iMaxWidth = 60;
	}
	if((m_iMaxWidth > 0) && (m_iMaxWidth != iMaxWidth))
	{
	    setReformat();
	}
	m_iMaxWidth = iMaxWidth;
	xxx_UT_DEBUGMSG(("Line %x MaxWidth set %d SameY %d \n",this,iMaxWidth,isSameYAsPrevious()));
	if(m_iMaxWidth > 9000 && isSameYAsPrevious())
	{
	    UT_DEBUGMSG(("Found unlikely width set!!! \n"));
	}
	//
	// OK set up the clearscreen parameters
	//
	m_iClearToPos = iMaxWidth;
	if(hasBordersOrShading())
	{
	  m_iClearToPos = getRightEdge();
	}
	//
	// The problem we're trying to solve here is that some character have
	// extensions to left of their position on a line. So if you just
	// clear from the start of a line you leave a bit of screen dirt
	// from character extension. To solve this we have to clear a bit to
	// the left of the line.
	// The code below is a heuristic to give us a first approximation for
	// when we do not have the info we need. We recalculate later in 
	// recalcHeight
	//
	m_iClearLeftOffset = getHeight()/5;
	if(getGraphics() && (m_iClearLeftOffset < getGraphics()->tlu(3)))
	{
		m_iClearLeftOffset = getGraphics()->tlu(3);
	}
	if(hasBordersOrShading())
	{
		m_iClearLeftOffset = 0;
	}
	if (getPage() && (getPage()->getWidth() - m_iMaxWidth < m_iClearLeftOffset))
	{
		m_iClearLeftOffset = getPage()->getWidth() - m_iMaxWidth;
		UT_ASSERT(m_iClearLeftOffset >= 0);
	}
}

void fp_Line::setContainer(fp_Container* pContainer)
{
	if (pContainer == getContainer())
	{
		return;
	}

	if (getContainer() && (pContainer != NULL))
	{
		xxx_UT_DEBUGMSG(("SetContainer clearScreen in fp_Line %x page %x \n",this,getPage())); 
		clearScreen();
	}
	if(pContainer != NULL)
	{
		getFillType().setParent(&pContainer->getFillType());
	}
	else
	{
		getFillType().setParent(NULL);
	}

	fp_Container::setContainer(pContainer);
	if(pContainer == NULL)
	{
		return;
	}
	if(getMaxWidth()  == 0 || (pContainer->getWidth() < getMaxWidth()))
	{
		setMaxWidth(pContainer->getWidth());
	}
	if (getBlock() && getBlock()->hasBorders())
	{
	    calcBorderThickness();
	}
	recalcHeight();
}

UT_sint32 fp_Line::getWidthToRun(fp_Run * pLastRun)
{
	calcLeftBorderThick();
	UT_sint32 width = getLeftThick();
	UT_sint32 count = m_vecRuns.getItemCount();
	UT_sint32 i = 0;
	for(i=0;i<count;i++)
	{
		fp_Run * pRun = m_vecRuns.getNthItem(i);
		if(pRun == pLastRun)
		{
			return width;
		}
		width += pRun->getWidth();
	}
	return getLeftThick();
}

UT_sint32 fp_Line::getFilledWidth(void)
{
	UT_sint32 width = getLeftThick();
	UT_sint32 count = m_vecRuns.getItemCount();
	UT_sint32 i = 0;
	UT_sint32 incr;
	for(i=0;i<count;i++)
	{
		fp_Run * pRun = m_vecRuns.getNthItem(i);
		width += (incr = pRun->getWidth());
		/* return the largest possible value if there is an obvious overflow.
		 * See 13709 */
		if (incr < 0 || width < 0)
			return G_MAXINT32;
	}
	return width;
}

bool fp_Line::removeRun(fp_Run* pRun, bool bTellTheRunAboutIt)
{
	// need to tell the previous run to redraw, in case this run contained
	// overstriking characters
//	fp_Run* pPrevRun  = pRun->getPrevRun();
//	if(pPrevRun)
//		pPrevRun->clearScreen();

        if(pRun->getType() == FPRUN_FORCEDPAGEBREAK)
	{
	    getBlock()->forceSectionBreak();
	}
	if (bTellTheRunAboutIt)
	{
	        if(pRun == getLastRun())
	        {
		     clearScreenFromRunToEnd(pRun);
		}
		pRun->setLine(NULL);
	}

	UT_sint32 ndx = m_vecRuns.findItem(pRun);
	UT_return_val_if_fail(ndx>=0,false);
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
		fp_FieldRun * fr = static_cast<fp_FieldRun*>(pNewRun);
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
		fp_FieldRun * fr = static_cast<fp_FieldRun*>(pNewRun);
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
		fp_FieldRun * fr = static_cast<fp_FieldRun*>(pNewRun);
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
	        pNext->unref();
		pNext->setPrev(pPrev);
		unref();
	}

	if (pPrev)
	{
	        pPrev->unref();
		pPrev->setNext(pNext);
		unref();
	}
	if(m_pBlock && (m_pBlock->getDocSectionLayout()->isCollapsing()))
	        return;
	if(getContainer())
	{
		xxx_UT_DEBUGMSG(("Removing line %x from container \n",this));
		static_cast<fp_VerticalContainer *>(getContainer())->removeContainer(this);
		setContainer(NULL);
	}
#ifdef USE_STATIC_MAP
	if (s_pMapOwner == this)
		s_pMapOwner = NULL;
#endif
	fp_Line * pNLine = static_cast<fp_Line *>(pNext);
	if(pNLine && pNLine->isSameYAsPrevious())
	{
	  if(!isSameYAsPrevious())
	  {
	       pNLine->setSameYAsPrevious(false);
	       pNLine->setY(getY());
	  }
	}
}

void fp_Line::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos,
							  bool& bBOL, bool& bEOL, bool &isTOC)
{
	UT_sint32 count = m_vecRuns.getItemCount();
	UT_sint32 i = 0;
	fp_Run* pFirstRun;
	xxx_UT_DEBUGMSG(("fp_line: mapXYToPosition this %x Y %d \n",this,getY()));
	do {

		pFirstRun = m_vecRuns.getNthItem(_getRunLogIndx(i++)); //#TF retrieve first visual run
		UT_ASSERT(pFirstRun);

	}while((i < count) && pFirstRun->isHidden());

	if((i < count) && pFirstRun->isHidden())
	{
		//all runs on this line are hidden, at the moment just assert
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
	}



	bBOL = false;
	if ( pFirstRun && (x <= pFirstRun->getX()))
	{
		xxx_UT_DEBUGMSG(("fp_Line::mapXYToPosition [0x%x]: x=%d, first run x=%d\n", this, x, pFirstRun->getX()));
		bBOL = true;
		bool bBBOL = true;
		UT_sint32 y2 = y - pFirstRun->getY() - getAscent() + pFirstRun->getAscent();
		pFirstRun->mapXYToPosition(0, y2, pos, bBBOL, bEOL,isTOC);
		return;
	}

	// check all of the runs.

	fp_Run* pClosestRun = NULL;
	UT_sint32 iClosestDistance = 0;

	for (i=0; i<count; i++)
	{
		fp_Run* pRun2 = m_vecRuns.getNthItem(_getRunLogIndx(i));	//#TF get i-th visual run

		if (pRun2->canContainPoint() || pRun2->isField())
		{
		  UT_sint32 y2 = y - pRun2->getY() - getAscent() + pRun2->getAscent();
			if ((x >= static_cast<UT_sint32>(pRun2->getX())) && (x < static_cast<UT_sint32>(pRun2->getX() + pRun2->getWidth())))
			{
				// when hit testing runs within a line, we ignore the Y coord
//			if (((y2) >= 0) && ((y2) < (pRun2->getHeight())))
				{
					pRun2->mapXYToPosition(x - pRun2->getX(), y2, pos, bBOL, bEOL,isTOC);
					xxx_UT_DEBUGMSG(("Found run %x offset %d pos %d \n",pRun2,pRun2->getBlockOffset(),pos));
					return;
				}
			}
			else if (((x - pRun2->getX()) == 0) && (pRun2->getWidth() == 0))
			{
				// Zero-length run. This should only happen with
				// FmtMrk Runs.
				// #TF this can also happen legitimately with overstriking text runs
				//UT_ASSERT(FPRUN_FMTMARK == pRun2->getType());

				pRun2->mapXYToPosition(x - pRun2->getX(), y2, pos, bBOL, bEOL,isTOC);
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
		  UT_sint32 y2 = y - pClosestRun->getY() - getAscent() + pClosestRun->getAscent();
			pClosestRun->mapXYToPosition(x - pClosestRun->getX(), y2, pos, bBOL, bEOL,isTOC);
			return;
		}
		
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		pos = 2; // start of document; this is just to avoid crashing
		return;
	}
	

	UT_sint32 y2 = y - pClosestRun->getY() - getAscent() + pClosestRun->getAscent();
	if(pClosestRun->isField())
	{
		UT_uint32 width = pClosestRun->getWidth() + 1;
		pClosestRun->mapXYToPosition(width , y2, pos, bBOL, bEOL,isTOC);
	}
	else
	{
		pClosestRun->mapXYToPosition(x - pClosestRun->getX(), y2, pos, bBOL, bEOL,isTOC);
	}
}

void fp_Line::getOffsets(fp_Run* pRun, UT_sint32& xoff, UT_sint32& yoff)
{
	// This returns the baseline of run. ie the bottom of the line of text
	 //
	UT_sint32 my_xoff = -31999;
	UT_sint32 my_yoff = -31999;
	fp_VerticalContainer * pVCon= (static_cast<fp_VerticalContainer *>(getContainer()));
	pVCon->getOffsets(this, my_xoff, my_yoff);
	xoff = my_xoff + pRun->getX();
	yoff = my_yoff + pRun->getY() + getAscent() - pRun->getAscent();
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
	fp_VerticalContainer * pVCon= (static_cast<fp_VerticalContainer *>(getContainer()));
	pVCon->getScreenOffsets(this, my_xoff, my_yoff);
	if(pRun)
        {
	  xoff = my_xoff + pRun->getX();
	  yoff = my_yoff + pRun->getY();
	}
	else
	{
	  xoff = my_xoff;
	  yoff = my_yoff;
	}
}

void fp_Line::setHeight(UT_sint32 i)
{
    xxx_UT_DEBUGMSG(("Line %p set to height %d \n",this,i));
    m_iHeight = i;
}

void fp_Line::setBlock(fl_BlockLayout * pBlock)
{
    if(pBlock != NULL)
    {
      UT_ASSERT(m_pBlock == NULL);
      //      UT_ASSERT(pBlock->findLineInBlock(this) >= 0);
    }
    m_pBlock = pBlock;
    if(m_pBlock && (m_pBlock->getPattern() > 0))
    {
        UT_RGBColor c = m_pBlock->getShadingingForeColor();
        getFillType().setColor(c);
    }
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
void fp_Line::recalcHeight(fp_Run * pLastRun)
{
	UT_sint32 count = m_vecRuns.getItemCount();
	if(count == 0)
	{
		return;
	}
	UT_sint32 i;

	UT_sint32 iMaxAscent = 0;
	UT_sint32 iMaxDescent = 0;
	UT_sint32 iMaxImage =0;
	UT_sint32 iMaxText = 0;
	fp_Line * pPrev = static_cast<fp_Line *>(getPrev());
	if(pPrev && isSameYAsPrevious())
	{
		iMaxAscent = pPrev->getAscent();
		iMaxDescent = pPrev->getDescent();
		iMaxText = pPrev->getHeight();
	}
	bool bSetByImage = false;
	fp_Run* pRun = m_vecRuns.getNthItem(0);
	xxx_UT_DEBUGMSG(("Orig Height = %d \n",getHeight()));
	UT_sint32 iOldHeight = getHeight();
	UT_sint32 iOldAscent = getAscent();
	UT_sint32 iOldDescent = getDescent();
	for (i=0; (i<count && ((pRun != pLastRun) || ((i== 0) && (getHeight() ==0)))); i++)
	{
		UT_sint32 iAscent;
		UT_sint32 iDescent;

		pRun = m_vecRuns.getNthItem(i);

		iAscent = pRun->getAscent();
		iDescent = pRun->getDescent();
		
		if (pRun->isSuperscript() || pRun->isSubscript())
		{
			iAscent += iAscent * 1/2;
			iDescent += iDescent;
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
	}
	//
	// More accurate calculation of the amount we need to clear to the
	// left of the line.
	//
	m_iClearLeftOffset = iMaxDescent;
	if(hasBordersOrShading())
	{
		m_iClearLeftOffset = 0;
	}
	if (getPage() && (getPage()->getWidth() - m_iMaxWidth < m_iClearLeftOffset))
	{
		m_iClearLeftOffset = getPage()->getWidth() - m_iMaxWidth;
		UT_ASSERT(m_iClearLeftOffset >= 0);
	}

	UT_sint32 iNewHeight = iMaxAscent + iMaxDescent;
	UT_sint32 iNewAscent = iMaxAscent;
	UT_sint32 iNewDescent = iMaxDescent;

	// adjust line height to include leading
	double dLineSpace;

	fl_BlockLayout::eSpacingPolicy eSpacing;

	m_pBlock->getLineSpacing(dLineSpace, eSpacing);
	
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
		iNewHeight = static_cast<UT_sint32>(dLineSpace);
	}
	else if (eSpacing == fl_BlockLayout::spacing_ATLEAST)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: recalcHeight at least \n"));
		iNewHeight = UT_MAX(iNewHeight, static_cast<UT_sint32>(dLineSpace));
	}
	else
	{
		// multiple
		if(!bSetByImage)
		{
			iNewHeight = static_cast<UT_sint32>(iNewHeight * dLineSpace +0.5);
		}
		else
		{
			iNewHeight = UT_MAX(iMaxAscent+static_cast<UT_sint32>(iMaxDescent*dLineSpace + 0.5), static_cast<UT_sint32>(dLineSpace));
		}
	}
	if(getBlock() && getBlock()->hasBorders())
	{
		if (isAlongTopBorder())
		{
			iNewHeight += m_iTopThick;
		}
		if (isAlongBotBorder())
		{
			iNewHeight += m_iBotThick;
		}
	}
	if(isSameYAsPrevious() && pPrev)
	{
		if(iNewHeight > pPrev->getHeight())
		{
			getBlock()->forceSectionBreak();
			pPrev->clearScreen();
			pPrev->setHeight(iNewHeight);
			pPrev->setAscent(iNewAscent);
			pPrev->setDescent(iNewDescent);
			pPrev->setScreenHeight(-1);
			while(pPrev->getPrev() && pPrev->isSameYAsPrevious())
			{
				pPrev = static_cast<fp_Line *>(pPrev->getPrev());
				pPrev->clearScreen();
				pPrev->setHeight(iNewHeight);
				pPrev->setAscent(iNewAscent);
				pPrev->setDescent(iNewDescent);
				pPrev->setScreenHeight(-1);
			}
			return;
		}
		else if(iNewHeight < pPrev->getHeight())
		{
			clearScreen();
			setHeight(pPrev->getHeight());
			setAscent(pPrev->getAscent());
			m_iScreenHeight = -1;	// undefine screen height
			setDescent(pPrev->getDescent());
			return;
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
		getBlock()->forceSectionBreak();
		setHeight(iNewHeight);
		m_iScreenHeight = -1;	// undefine screen height
		m_iAscent = iNewAscent;
		m_iDescent = iNewDescent;
	}
	if((getHeight() == 0) && (pLastRun != NULL))
	{
		setHeight(pLastRun->getHeight());
		m_iAscent = pLastRun->getAscent();
		m_iDescent = pLastRun->getDescent();
	}
// See bug 11158
// According to TF this happen for hidden text.
// Disable the assert.
//	UT_ASSERT(getHeight() > 0);
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
	if(count > 0 && static_cast<UT_sint32>(runIndex) < count)
	{
		pRun = m_vecRuns.getNthItem(runIndex);
	}
	return pRun;
}

void fp_Line::clearScreen(void)
{
	if(getBlock() == NULL)
	{
		return;
	}
	if(getBlock()->isHdrFtr() || isScreenCleared())
	{
		return;
	}
	xxx_UT_DEBUGMSG(("fp_Line: Doing regular full clearscreen %x \n",this));
	UT_sint32 count = m_vecRuns.getItemCount();
	if(!getPage() || !getPage()->isOnScreen())
	{
		return;
	}
	getFillType().setIgnoreLineLevel(true);
	if(count)
	{
		fp_Run* pRun;
		bool bNeedsClearing = true;

		UT_sint32 j;

		pRun = m_vecRuns.getNthItem(0);
		if(!pRun->getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
			return;

		for (j = 0; j < count; j++)
		{
			pRun = m_vecRuns.getNthItem(j);

			if(!pRun->isDirty())
			{
				bNeedsClearing = true;

				pRun->markAsDirty();
			}
		}

		if(bNeedsClearing)
		{
			pRun = m_vecRuns.getNthItem(0);

			UT_sint32 xoffLine, yoffLine;
			fp_VerticalContainer * pVCon= (static_cast<fp_VerticalContainer *>(getContainer()));
			pVCon->getScreenOffsets(this, xoffLine, yoffLine);

			// Note: we use getHeight here instead of m_iScreenHeight
			// in case the line is asked to render before it's been
			// assigned a height. Call it robustness, if you want.

			UT_sint32 height = getHeight();
			UT_sint32 sHeight = m_iScreenHeight;
			if(sHeight > height)
			{
				height = sHeight;
			}
//
// Screen Height might extend beyond the bottom of the column. We take account
// of that here
//
			if(pVCon->getHeight() < (getY() + height))
			{
				height -= (getY() + height - pVCon->getHeight());
			}
			xxx_UT_DEBUGMSG(("ClearScreen height is %d \n",height));
			// I have added the +1 to clear dirt after squiggles and
			// revision underlines
			if(getPage() == NULL)
			{
			        getFillType().setIgnoreLineLevel(false);
				return;
			}
//			UT_sint32 iExtra = getGraphics()->getFontAscent()/2;
			fl_DocSectionLayout * pSL =  getBlock()->getDocSectionLayout();
			UT_sint32 iExtra = getGraphics()->tlu(2);
			xxx_UT_DEBUGMSG(("full clearscren prect %x \n",getGraphics()->getClipRect()));
			if(getContainer() && (getContainer()->getContainerType() != FP_CONTAINER_CELL) && (getContainer()->getContainerType() != FP_CONTAINER_FRAME))

			{
				if(pSL->getNumColumns() >1)
				{
					iExtra = pSL->getColumnGap()/2;
				}
				else
				{
					iExtra = pSL->getRightMargin()/2;
				}
			}
			UT_ASSERT(m_iClearToPos + m_iClearLeftOffset <= getPage()->getWidth());
//			pRun->Fill(getGraphics(),xoffLine - m_iClearLeftOffset, yoffLine, m_iClearToPos + m_iClearLeftOffset+iExtra, height);
			pRun->Fill(getGraphics(),xoffLine - m_iClearLeftOffset, yoffLine, getMaxWidth() + m_iClearLeftOffset +iExtra, height);
			xxx_UT_DEBUGMSG(("Clear pLine %x clearoffset %d xoffline %d width %d \n",this,m_iClearLeftOffset,xoffLine,getMaxWidth() + m_iClearLeftOffset +iExtra));
//
// Sevior: I added this for robustness.
//
			setScreenCleared(true);
			m_pBlock->setNeedsRedraw();
			setNeedsRedraw();
			UT_sint32 i;
			for(i=0; i < m_vecRuns.getItemCount();i++)
			{
				pRun = m_vecRuns.getNthItem(i);
				pRun->markAsDirty();
				pRun->setCleared();
			}
		}
	}
	getFillType().setIgnoreLineLevel(false);
}

/*!
   helper function containing the code shared by the two
   clearScreenFromRunToEnd() functions
*/
void fp_Line::_doClearScreenFromRunToEnd(UT_sint32 runIndex)
{
	// need to get _visually_ first run on screen
	xxx_UT_DEBUGMSG((" _doClearScreenFromRunIndex %d this %x \n",runIndex,this));
	fp_Run* pRun = m_vecRuns.getNthItem(_getRunLogIndx(0));
	UT_sint32 count = m_vecRuns.getItemCount();

	if(count > 0 && !pRun->getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
		return;

	getFillType().setIgnoreLineLevel(true);

	// not sure what the reason for this is (Tomas, Oct 25, 2003)
	fp_Run * pLeftVisualRun = pRun;
	fp_Run * pRunToEraseFrom  = m_vecRuns.getNthItem(runIndex);
	
	bool bUseFirst = false;
	
	if(runIndex == 1)
	{
		bUseFirst = true;
	}
	
	// Find the first non dirty run. NO!! fp_Run::clearScreen sets the 
	// run Dirty first thing is it. Lets just do what we're told to do..

	// the run index is visual and if we are in LTR paragraph we
	// are deleting to the right of the run, while if we are in RTL
	// paragraph we are deleting to the _left_ (Tomas, Oct 25, 2003)
	UT_BidiCharType iDomDirection = m_pBlock->getDominantDirection();

#if 0
	UT_sint32 i;
	if(iDomDirection == UT_BIDI_LTR)
	{
		for(i = runIndex; i < count; i++)
		{
			pRun = m_vecRuns.getNthItem(_getRunLogIndx(i));

			if(pRun->isDirty())
			{
				if(runIndex < count-1)
				{
					runIndex++;
				}
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		for(i = runIndex; i>=0; i--)
		{
			pRun = m_vecRuns.getNthItem(_getRunLogIndx(i));

			if(pRun->isDirty() && runIndex > 0)
			{
				runIndex--;
			}
			else
			{
				break;
			}
		}
	}
	
#endif
	// if we have a valid index to clear from, let's do it ...

	if(runIndex < count)
	{
		UT_sint32 xoff, yoff;

		// get the run at the (visual) index
		pRun = m_vecRuns.getNthItem(_getRunLogIndx(runIndex));

		// Handle case where character extends behind the left side
		// like italic Times New Roman f. Clear a litle bit before if
		// there is clear screen there
		UT_sint32 j = runIndex - 1;

		// need to get previous _visual_ run
		fp_Run * pPrev = j >= 0 ? getRunAtVisPos(j) : NULL;
		
		UT_sint32 leftClear = 0;
		while(j >= 0 && pPrev != NULL && pPrev->getLength() == 0)
		{
			//pPrev = static_cast<fp_Run *>(m_vecRuns.getNthItem(j));
			pPrev->markAsDirty();
			pPrev = getRunAtVisPos(j);
			j--;
		}

		// now mark the last pPrev run as dirty, so we do not have to do
		// that lateer

		if(pPrev)
			pPrev->markAsDirty();
 
		leftClear = pRun->getDescent();
		if(j>0 && pPrev != NULL && pPrev->getType() == FPRUN_TEXT)
		{
			// the text run is not at the very left edge, so we need
			// not to clear into the margin
			leftClear = 0;
		}
		else if(j>=0 && pPrev != NULL && pPrev->getType() == FPRUN_FIELD)
		{
			leftClear = 0;
		}
		else if(j>=0 && pPrev != NULL && pPrev->getType() == FPRUN_IMAGE)
		{
			leftClear = 0;
		}
		if(pRun->getType() == FPRUN_IMAGE)
		{
			leftClear = 0;
		}
		if(bUseFirst)
		{
			getScreenOffsets(pLeftVisualRun, xoff, yoff);
		}
		else
		{
			getScreenOffsets(pRun, xoff, yoff);
		}
		
		UT_sint32 xoffLine, yoffLine;
#if DEBUG
		//UT_sint32 oldheight = getHeight();
#endif
		recalcHeight();
#if DEBUG
		//UT_ASSERT(oldheight == getHeight());
#endif
		
		fp_VerticalContainer * pVCon= (static_cast<fp_VerticalContainer *>(getContainer()));
		pVCon->getScreenOffsets(this, xoffLine, yoffLine);
		UT_sint32 diff = xoff - xoffLine;
		UT_ASSERT(yoff == yoffLine);

		fp_Line * pPrevLine = static_cast<fp_Line *>(getPrevContainerInSection());
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
		        leftClear = m_iClearLeftOffset;
		if(getPage() == NULL)
		{
			xxx_UT_DEBUGMSG(("pl_Line _doClear no Page \n"));
			getFillType().setIgnoreLineLevel(false);
			return;
		}

		UT_sint32 iExtra = getGraphics()->tlu(2);

		// this code adds half of the section/column margin to the left of the erasure are
		// -- we can only do this if the run we are to erase from is the leftmost run
		// (otherwise we erase area that belongs to runs that will not redraw themselves)

		if(pLeftVisualRun == pRunToEraseFrom)
		{
			fl_DocSectionLayout * pSL =  getBlock()->getDocSectionLayout();
			if(getContainer() &&
			   (getContainer()->getContainerType() != FP_CONTAINER_CELL) &&
			   (getContainer()->getContainerType() != FP_CONTAINER_FRAME))
			{
				if(pSL->getNumColumns() >1)
				{
					iExtra = pSL->getColumnGap()/2;
				}
				else
				{
					iExtra = pSL->getRightMargin()/2;
				}
			}
		}
		
//		UT_ASSERT((m_iClearToPos + leftClear - (xoff-xoffLine)) <= getPage()->getWidth());
		xxx_UT_DEBUGMSG(("Clear from runindex to end height %d \n",getHeight()));
		xxx_UT_DEBUGMSG(("Width of clear %d \n",m_iClearToPos + leftClear - xoff));
		xxx_UT_DEBUGMSG((" m_iClearToPos %d leftClear %d xoff %d xoffline %d \n",m_iClearToPos,leftClear,xoff,xoffLine));
		xxx_UT_DEBUGMSG(("_doclearscren prect %x \n",getGraphics()->getClipRect()));
		// now we do the clearing
		if(iDomDirection == UT_BIDI_LTR)
		{
			// clear from the start of the run (minus any adjustments)
			// to the end of the line
			pRun->Fill(getGraphics(), xoff - leftClear,
					   yoff,
					   getMaxWidth() + leftClear + iExtra -diff,
					   getHeight());
		}
		else
		{
			// clear from the left edge of the line (minus any
			// adjustments) to the end of the run
			pRun->Fill(getGraphics(), xoffLine - leftClear,
					   yoff,
					   (xoff-xoffLine) + pRun->getWidth() + leftClear,
					   getHeight());
		}
	
	
//
// Sevior: I added this for robustness.
//
		getBlock()->setNeedsRedraw();
		setNeedsRedraw();

		// finally, mark all runs concerned as dirty, starting with
		// the first run
		if(bUseFirst)
		{
			//pRun = static_cast<fp_Run*>(m_vecRuns.getNthItem(_getRunLogIndx(0)));
			pRun = pLeftVisualRun;
			runIndex = 0;
		}
		pRun->markAsDirty();
		pRun->setCleared();
		xxx_UT_DEBUGMSG(("Run %x marked Dirty \n",pRun));
		// now we need to mark all the runs between us and the end of
		// the line as dirty
		if(iDomDirection == UT_BIDI_RTL)
		{
			// we are working from the visual index to the left
			if(runIndex > 0)
			{
				runIndex--;
			
				while(runIndex >= 0)
				{
					pRun = m_vecRuns.getNthItem(_getRunLogIndx(runIndex));
					UT_ASSERT(pRun);
					pRun->markAsDirty();
					// do not have to check for line identity, since we
					// are working with runs on this line only
					runIndex--;
				}
			}
		}
		else
		{
			// we are working from the visual index to the right
			runIndex++;
			while(runIndex < count)
			{
				pRun = m_vecRuns.getNthItem(_getRunLogIndx(runIndex));
				UT_ASSERT(pRun);
				pRun->markAsDirty();				
				runIndex++;
			}
		}
		

	}
	else
	{
		xxx_UT_DEBUGMSG(("Invalid run index %d count %d \n",runIndex,count));
		clearScreen();
		// no valid indext to clear from ...
		getBlock()->setNeedsRedraw();
		setNeedsRedraw();
	}
	getFillType().setIgnoreLineLevel(false);
}

/*!
 * This method clears a line from the run given to the end of the line.
\param fp_Run * pRun
*/
void fp_Line::clearScreenFromRunToEnd(fp_Run * ppRun)
{
	xxx_UT_DEBUGMSG(("Clear 1 from run %x to end of line %x \n",ppRun,this));
	if(getBlock()->isHdrFtr())
	{
		return;
	}

	fp_Run * pRun = NULL;
	UT_sint32 count =  m_vecRuns.getItemCount();
	if(count > 0)
	{
		pRun = m_vecRuns.getNthItem(0);
		if(!pRun->getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
			return;

		UT_sint32 k = m_vecRuns.findItem(ppRun);
		if(k>=0)
		{
			UT_sint32 runIndex = _getRunVisIndx((UT_uint32)k);
			_doClearScreenFromRunToEnd(runIndex);
		}
	}
}


/*!
 * This method clears a line from the first non-dirty run at the given index
 * to the end of the line.
\param UT_uint32 runIndex - visual run index
\note clearing to end in RTL paragraph means clearing from run end to
the left edge
*/

void fp_Line::clearScreenFromRunToEnd(UT_uint32 runIndex)
{
	xxx_UT_DEBUGMSG(("Clear 2 from run %d to end line %x \n",runIndex,this));
	if(getBlock()->isHdrFtr())
	{
		return;
	}

	_doClearScreenFromRunToEnd((UT_sint32)runIndex);
}


void fp_Line::setNeedsRedraw(void)
{
	m_bNeedsRedraw = true;
	if(getContainer() && getContainer()->getContainerType() == FP_CONTAINER_CELL)
	{
		fp_CellContainer * pCell = static_cast<fp_CellContainer *>(getContainer());
		pCell->markAsDirty();
	}
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
		draw(m_vecRuns.getNthItem(0)->getGraphics());
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
	fp_VerticalContainer * pVCon= (static_cast<fp_VerticalContainer *>(getContainer()));
	pVCon->getScreenOffsets(this, my_xoff, my_yoff);
	xxx_UT_DEBUGMSG(("SEVIOR: Drawing line in line pG, my_yoff=%d  my_xoff %d \n",my_yoff,my_xoff));

	if(((my_yoff < -128000) || (my_yoff > 128000)) && pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		UT_DEBUGMSG(("FREM: Not drawing line in line pG, my_yoff=%d  my_xoff %d \n",my_yoff,my_xoff));
//
// offscreen don't bother.
//
		return;
	}
	dg_DrawArgs da;

	da.yoff = my_yoff + getAscent();
	da.xoff = my_xoff;
	da.pG = pG;
	da.bDirtyRunsOnly = true; //magic line to give a factor 2 speed up!
	const UT_Rect* pRect = pG->getClipRect();
	bool bDoShade = (getBlock() && (getBlock()->getPattern() > 0));
	if(bDoShade)
        {
	    da.bDirtyRunsOnly = false;
	    //
	    // Calculate the region of the fill for a shaded paragraph.
	    //
	    UT_Rect * pVRec = 	pVCon->getScreenRect();
	    UT_sint32 xs = pVRec->left + getLeftEdge();
	    UT_sint32 width = getRightEdge() - getLeftEdge(); 
	    UT_sint32 ys = my_yoff;
	    getFillType().Fill(pG,xs,ys,xs,ys,width,getHeight());
	    xxx_UT_DEBUGMSG(("pG Fill at y %d and to width %d \n",ys,getMaxWidth()));
	}
	for (int i=0; i < count; i++)
	{
		fp_Run* pRun = getRunAtVisPos(i);
		if(pRun->isHidden())
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
		fp_CellContainer * pCell = static_cast<fp_CellContainer *>(pCon);
		pCell->drawLinesAdjacent();
	}
#endif
	if(getBlock() && getBlock()->hasBorders())
	     drawBorders(pG);
}

void fp_Line::draw(dg_DrawArgs* pDA)
{
	UT_sint32 count = m_vecRuns.getItemCount();
	if(count <= 0)
		return;
	bool bQuickPrint = pDA->pG->canQuickPrint();
	UT_sint32 i = 0;
	if(bQuickPrint)
        {
	      for (i=0; i<count; i++)
	      {
		   fp_Run* pRun = static_cast<fp_Run*>(m_vecRuns.getNthItem(i));
		   pRun->lookupProperties(pDA->pG);
	      }
	      if(getBlock()->getAlignment() && getBlock()->getAlignment()->getType() == FB_ALIGNMENT_JUSTIFY)
	      {
		   getBlock()->getAlignment()->initialize(this);
	      }
	}
	xxx_UT_DEBUGMSG(("Drawing line %x in line pDA, width %d \n",this,getWidth()));
	pDA->yoff += getAscent();
	xxx_UT_DEBUGMSG(("fp_Line::draw getAscent() %d getAscent() %d yoff %d \n",getAscent(),getAscent(),pDA->yoff));
	const UT_Rect* pRect = pDA->pG->getClipRect();
	if(getBlock() && (getBlock()->getPattern() > 0))
	{
	      xxx_UT_DEBUGMSG(("pRect in fp_Line::draw is %p \n",pRect));
	      UT_sint32 x = pDA->xoff;
	      UT_sint32 y = pDA->yoff - getAscent();
	      x= x - getX() + getLeftEdge();
	      UT_sint32 width = getRightEdge() - getLeftEdge();
	      if(!pDA->bDirtyRunsOnly)
	      {
		    getFillType().Fill(pDA->pG,x,y,x,y,width,getHeight());
		    xxx_UT_DEBUGMSG(("Fill at y %d and to width %d \n",y,getMaxWidth()));
	      }
	}
	for (i=0; i<count; i++)
	{
	      fp_Run* pRun = getRunAtVisPos(i);
	      if(pRun->isHidden())
		  continue;

	      FP_RUN_TYPE rType = pRun->getType();

	      dg_DrawArgs da = *pDA;

	      // for these two types of runs, we want to draw for the
	      // entire line-width on the next line. see bug 1301
	      if (rType == FPRUN_FORCEDCOLUMNBREAK ||
			rType == FPRUN_FORCEDPAGEBREAK)
	      {
		   UT_sint32 my_xoff = 0, my_yoff = 0;
		   fp_VerticalContainer * pVCon= (static_cast<fp_VerticalContainer *>(getContainer()));
		   pVCon->getScreenOffsets(this, my_xoff, my_yoff);
		   da.xoff = my_xoff;
	      }
	      else
	      {
		   da.xoff += pRun->getX();
	      }

	      da.yoff += pRun->getY();
	      UT_Rect runRect(da.xoff, da.yoff - pRun->getAscent(), pRun->getWidth(), pRun->getHeight());
	      xxx_UT_DEBUGMSG(("fp_Line: Draw run in line at yoff %d pRun->getY() \n",da.yoff,pRun->getY()));
	      if (pRect == NULL || pRect->intersectsRect(&runRect))
	      {
		   pRun->draw(&da);
	      }
	      else
	      {
		   xxx_UT_DEBUGMSG(("Run not in clip, pRect top %d height %d run top %d height %d \n",pRect->top,pRect->height,runRect.top,runRect.height));
	      }
	      da.yoff -= pRun->getY();
	}
	if(bQuickPrint)
        {
	      if(getBlock()->getAlignment() && getBlock()->getAlignment()->getType() == FB_ALIGNMENT_JUSTIFY)
	      {
		   getBlock()->getAlignment()->initialize(this);
	      }
	}
//
// Check if this is in a cell, if so redraw the lines around it.
//
#if 0
	fp_Container * pCon = getContainer();
	if(pCon->getContainerType() == FP_CONTAINER_CELL)
	{
		fp_CellContainer * pCell = static_cast<fp_CellContainer *>(pCon);
		pCell->drawLinesAdjacent();
	}
#endif
	if(getBlock() && getBlock()->hasBorders())
	        drawBorders(pDA->pG);

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
	UT_BidiCharType iDomDirection = m_pBlock->getDominantDirection();

	eWorkingDirection = WORK_FORWARD;
	eUseTabStop = USE_NEXT_TABSTOP;

	// now from the current alignment work out which way we need to process the line
	// and the corresponding starting positions

	switch (eAlignment)
	{
		case FB_ALIGNMENT_LEFT:
			if(iDomDirection == UT_BIDI_RTL)
				eUseTabStop = USE_PREV_TABSTOP;
			else
				eUseTabStop = USE_NEXT_TABSTOP;

			eWorkingDirection = WORK_FORWARD;
			break;

		case FB_ALIGNMENT_RIGHT:
			if(iDomDirection == UT_BIDI_RTL)
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
			if(iDomDirection == UT_BIDI_RTL)
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

fp_Run* fp_Line::calculateWidthOfRun(UT_sint32 &iWidth, UT_uint32 iIndxVisual, FL_WORKING_DIRECTION eWorkingDirection, FL_WHICH_TABSTOP eUseTabStop)
{
	const UT_sint32 iCountRuns		  = m_vecRuns.getItemCount();
	UT_ASSERT(iCountRuns > static_cast<UT_sint32>(iIndxVisual));

	//work out the real index based on working direction
	UT_uint32 iIndx =0;
	iIndx = eWorkingDirection == WORK_FORWARD ? iIndxVisual : iCountRuns - iIndxVisual - 1;

	// of course, the loop is running in visual order, but the vector is
	// in logical order
	fp_Run* pRun = m_vecRuns.getNthItem(_getRunLogIndx(iIndx));

	// find out the direction of the paragraph
	UT_BidiCharType iDomDirection = m_pBlock->getDominantDirection();

	UT_sint32 iXreal = 0;
	if(iDomDirection == UT_BIDI_RTL)
	  iXreal = getMaxWidth() - iWidth;
	else
		iXreal = iWidth;

	xxx_UT_DEBUGMSG(("fp_Line::calculateWidthOfRun (0x%x L 0x%x): \n"
				 "		 iXreal %d, iXLreal %d, iIndxVisual %d, iCountRuns %d,\n"
				 "		 eWorkingDirection %d, eUseTabStop %d,\n"
						 ,pRun,this,iXreal, iXLreal, iIndxVisual, iCountRuns,
						 eWorkingDirection, eUseTabStop
				));

	_calculateWidthOfRun(iXreal,
						 pRun,
						 iIndxVisual,
						 iCountRuns,
						 eWorkingDirection,
						 eUseTabStop,
						 iDomDirection
						 );

//xxx_UT_DEBUGMSG(("new iXreal %d, iXLreal %d\n", iXreal, iXLreal));

	if(iDomDirection == UT_BIDI_RTL)
	{
	  iWidth = getMaxWidth() - iXreal;
	}
	else
	{
		iWidth = iXreal;
	}

	return pRun;
}

// private version of the above, which expect both the index and run prointer
// to be passed to it.
inline void fp_Line::_calculateWidthOfRun(	UT_sint32 &iX,
									fp_Run * pRun,
									UT_uint32 iIndx,
									UT_uint32 iCountRuns,
									FL_WORKING_DIRECTION eWorkingDirection,
									FL_WHICH_TABSTOP eUseTabStop,
									UT_BidiCharType iDomDirection
									)
{
	if(!pRun)
		return;

	// If the run is to be hidden just return, since the positions
	// should remain as they were, but it would be preferable if this
	// situation was trapped higher up (thus the assert)
	if(pRun->isHidden())
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}

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
				UT_sint32	iPos = 0;
				eTabType	iTabType =FL_TAB_LEFT ;
				eTabLeader	iTabLeader = FL_LEADER_NONE;
				UT_DebugOnly<bool> bRes = false;
				if(pTabRun->isTOCTab())
				{
					iTabLeader = getBlock()->getTOCTabLeader(10);
					iTabType =  FL_TAB_LEFT;
					iPos =  getBlock()->getTOCTabPosition(10);
					bRes = true;
				}
				else if(pTabRun->isTOCTabListLabel())
				{
					iTabLeader = FL_LEADER_NONE;
					iTabType =  FL_TAB_LEFT;
					bRes =  findNextTabStop(iX, iPos, iTabType, iTabLeader);
				}

				// now find the tabstop for this tab, depending on whether we
				// are to use next or previous tabstop
				else if(eUseTabStop == USE_NEXT_TABSTOP)
				{
					if(iDomDirection == UT_BIDI_RTL)
					{
						UT_sint32 iStartPos = getContainer()->getWidth() - iX;
						bRes = findNextTabStop(iStartPos, iPos, iTabType, iTabLeader);
						iPos = getContainer()->getWidth() - iPos;
					}
					else
						bRes = findNextTabStop(iX, iPos, iTabType, iTabLeader);
				}
				else
				{
					if(iDomDirection == UT_BIDI_RTL)
					{
						UT_sint32 iStartPos = getContainer()->getWidth() - iX;
						
						bRes = findPrevTabStop(iStartPos, iPos, iTabType, iTabLeader);
						iPos = getContainer()->getWidth() - iPos;
					}
					else
						bRes = findPrevTabStop(iX, iPos, iTabType, iTabLeader);
				}
				UT_ASSERT(bRes);
				
				fp_Run *pScanRun = NULL;
				UT_sint32 iScanWidth = 0;
				pTabRun->setLeader(iTabLeader);
				pTabRun->setTabType(iTabType);
				// we need to remember what the iX was
				UT_sint32 iXprev;
				iXprev = iX;
				
				UT_BidiCharType iVisDirection = pTabRun->getVisDirection();
				
				switch ( iTabType )
				{
				case FL_TAB_LEFT:
					if(iVisDirection == UT_BIDI_LTR && iDomDirection == UT_BIDI_LTR)
					{
						iX = iPos;
						
						iWidth = abs(iX - iXprev);
						xxx_UT_DEBUGMSG(("left tab (ltr para), iX %d, iWidth %d\n", iX,iWidth));
					}
					else
					{
						iScanWidth = 0;
						for ( UT_uint32 j = iIndx+1; j < iCountRuns; j++ )
						{
							UT_uint32 iJ;
							iJ = eWorkingDirection == WORK_FORWARD ? j : iCountRuns - j - 1;

							pScanRun = m_vecRuns.getNthItem(_getRunLogIndx(iJ));
							if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
								break;

							iScanWidth += pScanRun->getWidth();
						}

						if ( iScanWidth > abs(iPos - iX))
						{
							iWidth = 0;
						}
						else
						{
							iX += iPos - iX - eWorkingDirection * iScanWidth;
							iWidth = abs(iX - iXprev);
						}
					}

					break;

					case FL_TAB_CENTER:
					{
						iScanWidth = 0;
						for ( UT_uint32 j = iIndx+1; j < iCountRuns; j++ )
						{
							UT_uint32 iJ;
							iJ = eWorkingDirection == WORK_FORWARD ? j : iCountRuns - j - 1;

							pScanRun = m_vecRuns.getNthItem(_getRunLogIndx(iJ));

							if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
								break;
							iScanWidth += pScanRun->getWidth();
						}

						if ( iScanWidth / 2 > abs(iPos - iX))
							iWidth = 0;
						else
						{
							iX += iPos - iX - static_cast<UT_sint32>(eWorkingDirection * iScanWidth / 2);
							iWidth = abs(iX - iXprev);
						}
						break;
					}

					case FL_TAB_RIGHT:
						if(iVisDirection == UT_BIDI_RTL && iDomDirection == UT_BIDI_RTL)
						{
							iX = iPos;
							iWidth = abs(iX - iXprev);
							xxx_UT_DEBUGMSG(("right tab (rtl para), iX %d, width %d\n", iX,pTabRun->getWidth()));
						}
						else
						{
							xxx_UT_DEBUGMSG(("right tab (ltr para), ii %d\n",ii));
							iScanWidth = 0;
							for ( UT_uint32 j = iIndx+1; j < iCountRuns; j++ )
							{
								UT_uint32 iJ;
								iJ = eWorkingDirection == WORK_FORWARD ? j : iCountRuns - j - 1;
								xxx_UT_DEBUGMSG(("iJ %d\n", iJ));

								pScanRun = m_vecRuns.getNthItem(_getRunLogIndx(iJ));

								if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
									break;
								iScanWidth += pScanRun->getWidth();
							}

							if ( iScanWidth > abs(iPos - iX))
							{
								iWidth = 0;
							}
							else
							{
								iX += iPos - iX - static_cast<UT_sint32>(eWorkingDirection * iScanWidth);
								iWidth = abs(iX - iXprev);
							}

						}
						break;

					case FL_TAB_DECIMAL:
					{
						UT_UCSChar *pDecimalStr =NULL;
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
						for ( UT_uint32 j = iIndx+1; j < iCountRuns; j++ )
						{
							UT_uint32 iJ;
							iJ = eWorkingDirection == WORK_FORWARD ? j : iCountRuns - j - 1;

							pScanRun = m_vecRuns.getNthItem(_getRunLogIndx(iJ));

							if(!pScanRun || pScanRun->getType() == FPRUN_TAB)
								break;

							bool foundDecimal = false;
							if(pScanRun->getType() == FPRUN_TEXT)
							{
								UT_sint32 decimalBlockOffset = (static_cast<fp_TextRun *>(pScanRun))->findCharacter(0, pDecimalStr[0]);
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
								iScanWidth += static_cast<fp_TextRun *>(pScanRun)->simpleRecalcWidth(runLen);
								break; // we found our decimal, don't search any further
							}
							else
							{
								iScanWidth += pScanRun->getWidth();
							}
						}
						if ( iScanWidth > abs(iPos - iX))
						{

							// out of space before the decimal point;
							// tab collapses to nothing
							iWidth = 0;
						}
						else {
							iX = iPos - eWorkingDirection * iScanWidth;
							iWidth = abs(iX - iXprev);
						}
						FREEP(pDecimalStr);
						break;
					}

					case FL_TAB_BAR:
					{
						iX = iPos;
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
				iX += iWidth;

				xxx_UT_DEBUGMSG(("run[%d] (type %d) width=%d\n", i,pRun->getType(),iWidth));
			}

			pTabRun->setTabWidth(iWidth);
			return;
		}

		case FPRUN_TEXT:
		{
			pRun->recalcWidth();
			//and fall through to default
		}

		default:
		{
			if(eWorkingDirection == WORK_FORWARD)
			{
				iX += pRun->getWidth();
			}
			else
			{
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
	calcLeftBorderThick();
	calcRightBorderThick();
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
	UT_return_if_fail(pAlignment);
	FB_AlignmentType eAlignment 	  = pAlignment->getType();

	//we have to remember the old X coordinances of our runs
	//to be able to decide latter whether and where from to erase
	//(this is a real nuisance, but since it takes two passes to do the layout
	//I do not see a way to avoid this)
	//we will use a static buffer for this initialised to a decent size and
	//reallocated as needed
#ifdef DEBUG
	const UT_uint32 iDefinesLine = __LINE__;

	UT_uint32 iRealocCount = 0;
#endif
	while(static_cast<UT_sint32>(s_iOldXsSize) < iCountRuns + 1)
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

	UT_sint32 iStartX = getLeftThick();

	// find out the direction of the paragraph
	UT_BidiCharType iDomDirection = m_pBlock->getDominantDirection();

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
			if(iDomDirection == UT_BIDI_RTL)
				eUseTabStop = USE_PREV_TABSTOP;
			else
				eUseTabStop = USE_NEXT_TABSTOP;

			eWorkingDirection = WORK_FORWARD;
			break;

		case FB_ALIGNMENT_RIGHT:
			if(iDomDirection == UT_BIDI_RTL)
				eUseTabStop = USE_NEXT_TABSTOP;
			else
				eUseTabStop = USE_PREV_TABSTOP;

			eWorkingDirection = WORK_BACKWARD;
			iStartX = getAvailableWidth();
			break;

		case FB_ALIGNMENT_CENTER:
			eWorkingDirection = WORK_FORWARD;
			eUseTabStop = USE_FIXED_TABWIDTH;
			// we will pretend the line starts at pos 0, work out the width
			// and then shift it by the necessary amount to the right
			iStartX = 0;
			break;

		case FB_ALIGNMENT_JUSTIFY:
			if(iDomDirection == UT_BIDI_RTL)
			{
				eWorkingDirection = WORK_BACKWARD;
				iStartX = getMaxWidth();
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
	//variables to keep information about how to erase the line once we are
	//in position to do so
	bool bLineErased		= false;
	UT_sint32 iIndxToEraseFrom = -1;

#if 0 //def DEBUG

	//some extra but lengthy debug stuff
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

	UT_DEBUGMSG(("fp_Line::layout(), this = 0x%x\n"
				 "		 alignment [%s], working direction [%s], using tabstops [%s]\n"
				 "		 iStartX	= %d, \n"
				 "		 iCountRuns = %d\n",
				 this, al, d, t, iStartX, iCountRuns
	));

#endif //end of the debug stuff


	// now we work our way through the runs on this line
	xxx_UT_DEBUGMSG(("fp_Line::layout ------------------- \n"));

	UT_sint32 ii = 0;
	for (; ii<iCountRuns; ++ii)
	{
		//work out the real index based on working direction
		UT_uint32 iIndx;
		iIndx = eWorkingDirection == WORK_FORWARD ? ii : iCountRuns - ii - 1;

		// of course, the loop is running in visual order, but the vector is
		// in logical order
		fp_Run* pRun = m_vecRuns.getNthItem(_getRunLogIndx(iIndx));


		// if this tab is to be hidden, we must treat it as if its
		// width was 0
		if(pRun->isHidden())
			continue;

		// if we are working from the left, we want to set the
		// X coordinance now; if from the right we will do it
		// after we have got to the width worked out
		// also, decide whether erasure is needed
		if(eWorkingDirection == WORK_FORWARD)
		{
			s_pOldXs[iIndx] = pRun->getX();
				pRun->Run_setX(iX,FP_CLEARSCREEN_NEVER);
		}
		xxx_UT_DEBUGMSG(("fp_Line::layout: iX %d, ii %d, iCountRuns %d\n"
					 "		 run type %d\n",
					iX, ii, iCountRuns, pRun->getType()));
		_calculateWidthOfRun(iX,
							 pRun,
							 ii,
							 iCountRuns,
							 eWorkingDirection,
							 eUseTabStop,
							 iDomDirection
							);

		// if working backwards, set the new X coordinance
		// and decide if line needs erasing
		if(eWorkingDirection == WORK_BACKWARD)
		{
			s_pOldXs[iIndx] = pRun->getX();
			pRun->Run_setX(iX,FP_CLEARSCREEN_NEVER);
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
					fp_Run* pRun = m_vecRuns.getNthItem(_getRunLogIndx(k));
					UT_ASSERT(pRun);

					// if this tab is to be hidden, we must treated as if its
					// width was 0
					if(pRun->isHidden())
						continue;

					//eClearScreen = iStartX == pOldXs[k] ?
					//FP_CLEARSCREEN_NEVER : FP_CLEARSCREEN_FORCE;

					// if we are in LTR context, the first visual run that
					// shifts is the one to erase from; in RTL context
					// it is the last visual run that shifts
					
					if(!bLineErased && iStartX != s_pOldXs[k])
					{
						if(iDomDirection == UT_BIDI_LTR)
							bLineErased = true;
						
						iIndxToEraseFrom = k;
					}

					pRun->Run_setX(iStartX,FP_CLEARSCREEN_NEVER);
					iStartX += pRun->getWidth();
				}
				xxx_UT_DEBUGMSG(("Final width of line %d maxwidth %d \n",iStartX,getMaxWidth()));
			}
		break;
		case FB_ALIGNMENT_JUSTIFY:
			{
				// now we need to shift the x-coordinances to reflect the new widths
				// of the spaces
				UT_sint32 k;
#if 0
				// if we are working backwards, we have to
				// ignore trailing spaces on the line ...
				if(eWorkingDirection == WORK_BACKWARD)
				{
					// work from first visual run to the right ...
					for (k = 0; k < iCountRuns; k++)
					{
						fp_Run* pRun = static_cast<fp_Run*>(m_vecRuns.getNthItem(_getRunLogIndx(k)));
						UT_ASSERT(pRun);
					
						if(!pRun->doesContainNonBlankData())
						{
							iStartX += pRun->getWidth();
						}
						else
						{
							iStartX += pRun->findTrailingSpaceDistance();
							break;
						}
					}
				}
#endif
				for (k = 0; k < iCountRuns; k++)
				{
					UT_uint32 iK = (eWorkingDirection == WORK_FORWARD) ? k : iCountRuns - k - 1;
					fp_Run* pRun = static_cast<fp_Run*>(m_vecRuns.getNthItem(_getRunLogIndx(iK)));
					UT_ASSERT(pRun);

					// if this tab is to be hidden, we must treated as if its
					// width was 0
					if(pRun->isHidden())
						continue;

					if(eWorkingDirection == WORK_BACKWARD)
					{
						iStartX -= pRun->getWidth();
						//eClearScreen = iStartX == pOldXs[iK] ? FP_CLEARSCREEN_NEVER : FP_CLEARSCREEN_FORCE;
						if(!bLineErased && iStartX != s_pOldXs[iK])
						{
							if(iDomDirection == UT_BIDI_LTR)
								bLineErased = true;
							
							iIndxToEraseFrom = iK;
						}

						pRun->Run_setX(iStartX, FP_CLEARSCREEN_NEVER);
					}
					else
					{
						//eClearScreen = iStartX == pOldXs[iK] ? FP_CLEARSCREEN_NEVER : FP_CLEARSCREEN_FORCE;
						if(!bLineErased && iStartX != s_pOldXs[iK])
						{
							if(iDomDirection == UT_BIDI_LTR)
								bLineErased = true;
							
							iIndxToEraseFrom = iK;
						}
						xxx_UT_DEBUGMSG(("Run %x has width %d \n",pRun,pRun->getWidth()));
						pRun->Run_setX(iStartX, FP_CLEARSCREEN_NEVER);
						iStartX += pRun->getWidth();
					}
				}
				xxx_UT_DEBUGMSG(("Final width of line %d maxwidth %d \n",iStartX,getMaxWidth()));
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
					fp_Run* pRun = m_vecRuns.getNthItem(_getRunLogIndx(k));
					UT_ASSERT(pRun);

					// if this tab is to be hidden, we must treated as if its
					// width was 0
					if(pRun->isHidden())
						continue;

					UT_sint32 iCurX = pRun->getX();
					//eClearScreen = iCurX + iStartX == pOldXs[k] ? FP_CLEARSCREEN_NEVER : FP_CLEARSCREEN_FORCE;
					if(!bLineErased && iCurX + iStartX != s_pOldXs[k])
					{
						if(iDomDirection == UT_BIDI_LTR)
							bLineErased = true;
						
						iIndxToEraseFrom = k;
					}

					pRun->Run_setX(iCurX + iStartX, FP_CLEARSCREEN_NEVER);
				}
			}
		break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	} //switch eAlignment

	// if the line needs erasure, then either bLineErased is set or 
	if(iIndxToEraseFrom >= 0)
	{
		xxx_UT_DEBUGMSG(("fp_Line::layout (0x%x): clearling line from indx %d\n",
						 this, iIndxToEraseFrom));
		clearScreenFromRunToEnd((UT_uint32)iIndxToEraseFrom);
	}
	else
	{
		xxx_UT_DEBUGMSG(("fp_Line::layout (0x%x): nothing to clear\n", this));
	}

}

bool fp_Line::containsFootnoteReference(void)
{
	fp_Run * pRun = NULL;
	UT_sint32 i =0;
	bool bFound = false;
	for(i=0; (i< countRuns()) && !bFound; i++)
	{
		pRun = getRunFromIndex(static_cast<UT_uint32>(i));
		if(pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun * pFRun = static_cast<fp_FieldRun *>(pRun);
			if(pFRun->getFieldType() == FPFIELD_footnote_ref)
			{
				bFound = true;
				break;
			}
		}
	}
	return bFound;
}

bool fp_Line::getFootnoteContainers(UT_GenericVector<fp_FootnoteContainer*> * pvecFoots)
{
	fp_Run * pRun = NULL;
	UT_uint32 i =0;
	bool bFound = false;
	fp_FootnoteContainer * pFC = NULL;
	PT_DocPosition posStart = getBlock()->getPosition();
	PT_DocPosition posEnd = posStart + getLastRun()->getBlockOffset() + getLastRun()->getLength();
	posStart += getFirstRun()->getBlockOffset();
	for(i=0; (i< static_cast<UT_uint32>(countRuns())); i++)
	{
		pRun = getRunFromIndex(i);
		if(pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun * pFRun = static_cast<fp_FieldRun *>(pRun);
			if(pFRun->getFieldType() == FPFIELD_footnote_ref)
			{
				fp_FieldFootnoteRefRun * pFNRun = static_cast<fp_FieldFootnoteRefRun *>(pFRun);
				fl_FootnoteLayout * pFL = getBlock()->getDocLayout()->findFootnoteLayout(pFNRun->getPID());
				
				UT_ASSERT(pFL);
				xxx_UT_DEBUGMSG(("Pos of footnote %d start of run %d end of run %d \n",pFL->getDocPosition(),posStart,posEnd));
				if(pFL && pFL->getDocPosition()>= posStart && pFL->getDocPosition() <= posEnd)
				{
					pFC = static_cast<fp_FootnoteContainer *>(pFL->getFirstContainer());
					bFound = true;
					pvecFoots->addItem(pFC);
				}
			}
		}
	}
	return bFound;
}



bool fp_Line::containsAnnotations(void)
{
	fp_Run * pRun = NULL;
	UT_sint32 i =0;
	bool bFound = false;
	for(i=0; (i< countRuns()) && !bFound; i++)
	{
		pRun = getRunFromIndex(static_cast<UT_uint32>(i));
		if(pRun->getType() == FPRUN_HYPERLINK)
		{
			fp_HyperlinkRun * pHRun = static_cast<fp_HyperlinkRun *>(pRun);
			if(pHRun->getHyperlinkType() == HYPERLINK_ANNOTATION )
			{
			        fp_AnnotationRun * pARun = static_cast<fp_AnnotationRun *>(pHRun);
				UT_uint32 iPID = pARun->getPID();
				if(iPID > 0)
				{
				     bFound = true;
				     break;
				}
			}
		}
	}
	return bFound;
}

bool fp_Line::getAnnotationContainers(UT_GenericVector<fp_AnnotationContainer*> * pvecAnns)
{
	fp_Run * pRun = NULL;
	UT_uint32 i =0;
	bool bFound = false;
	fp_AnnotationContainer * pAC = NULL;
	PT_DocPosition posStart = getBlock()->getPosition();
	PT_DocPosition posEnd = posStart + getLastRun()->getBlockOffset() + getLastRun()->getLength();
	posStart += getFirstRun()->getBlockOffset();
	for(i=0; (i< static_cast<UT_uint32>(countRuns())); i++)
	{
		pRun = getRunFromIndex(i);
		if(pRun->getType() == FPRUN_HYPERLINK)
		{
			fp_HyperlinkRun * pHRun = static_cast<fp_HyperlinkRun *>(pRun);
			if(pHRun->getHyperlinkType() == HYPERLINK_ANNOTATION )
			{
			        fp_AnnotationRun * pARun = static_cast<fp_AnnotationRun *>(pHRun);
				UT_uint32 iPID = pARun->getPID();
				if(iPID > 0)
				{
				      fl_AnnotationLayout * pAL = getBlock()->getDocLayout()->findAnnotationLayout(pARun->getPID());
				
				      UT_ASSERT(pAL);
				      xxx_UT_DEBUGMSG(("Pos of Annotation %d start of run %d end of run %d \n",pAL->getDocPosition(),posStart,posEnd));
				      if(pAL && pAL->getDocPosition()>= posStart && pAL->getDocPosition() <= posEnd)
				      {
					   pAC = static_cast<fp_AnnotationContainer *>(pAL->getFirstContainer());
					   bFound = true;
					   pvecAnns->addItem(pAC);
				      }
				}
			}
		}
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
	xxx_UT_DEBUGMSG(("SetX Line %x X %d \n",this,iX));
	if(iX < m_iX)
	  {
	    UT_DEBUGMSG(("m_iX decreaed in value old %d new %d \n",m_iX,iX));
	  }
	m_iX = iX;
}

void fp_Line::setY(UT_sint32 iY)
{
	if (m_iY == iY)
	{
		return;
	}
	if((m_iY !=INITIAL_OFFSET) && (m_iY != 0)  && isWrapped())
	{
	    setReformat();
	}
	clearScreen();
	m_iY = iY;
}

UT_sint32 fp_Line::getMarginBefore(void) const
{
	if (isFirstLineInBlock() && getBlock()->getPrev())
	{
		fl_ContainerLayout * pPrevC = getBlock()->getPrev();
		UT_sint32 iBottomMargin = 0;
		bool bLoop = true;
		while(bLoop)
		{
			if(pPrevC->getContainerType() == FL_CONTAINER_BLOCK)
			{
				bLoop = false;
				iBottomMargin = static_cast<fl_BlockLayout *>(pPrevC)->getBottomMargin();
			}
			else if(pPrevC->getContainerType() == FL_CONTAINER_TABLE)
			{
				bLoop = false;
				iBottomMargin = static_cast<fl_TableLayout *>(pPrevC)->getBottomOffset();
			}
			else
			{
				if(pPrevC->getPrev())
				{
					pPrevC = pPrevC->getPrev();
				}
				else
				{
				    bLoop = false;
				    return 0;
				}
			}
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
		fl_ContainerLayout * pNext = getBlock()->getNext();
		if (!pNext)
			return 0;

		UT_sint32 iBottomMargin = getBlock()->getBottomMargin();

		UT_sint32 iNextTopMargin = 0;
		bool bLoop = true;
		while(bLoop)
		{
			if(pNext->getContainerType() == FL_CONTAINER_BLOCK)
			{
				iNextTopMargin = static_cast<fl_BlockLayout *>(pNext)->getTopMargin();
				bLoop = false;
			}
			else if (pNext->getContainerType() == FL_CONTAINER_TABLE)
			{
				iNextTopMargin = 0;
				bLoop = false;
			}
			else
			{
				if(pNext->getNext())
				{
					pNext = pNext->getNext();
				}
				else
				{
					bLoop = false;
				}
			}
		}
		UT_sint32 iMargin = UT_MAX(iBottomMargin, iNextTopMargin);
		return iMargin + m_iAdditionalMarginAfter;
	}

	return m_iAdditionalMarginAfter;
}

bool fp_Line::recalculateFields(UT_uint32 iUpdateCount)
{
	bool bResult = false;

	UT_sint32 iNumRuns = m_vecRuns.getItemCount();
	for (UT_sint32 i = 0; i < iNumRuns; i++)
	{
		fp_Run* pRun = m_vecRuns.getNthItem(i);

		if (pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun* pFieldRun = static_cast<fp_FieldRun*>(pRun);
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
		return (m_vecRuns.getLastItem());
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
		pRun = m_vecRuns.getLastItem();
		while(pRun != NULL && pRun->getType() != FPRUN_TEXT)
		{
			pRun = pRun->getPrevRun();
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

	UT_DebugOnly<bool> bRes = m_pBlock->findNextTabStop(iStartX + getX(), 
							    getX() + getMaxWidth(), 
							    iTabStopPosition, iTabStopType, 
							    iTabStopLeader);
	UT_ASSERT(bRes);

	iTabStopPosition -= getX();

	//has to be <=
	if (iTabStopPosition <= getMaxWidth())
	{
		iPosition = iTabStopPosition;
		iType = iTabStopType;
		iLeader = iTabStopLeader;

		return true;
	}
	else
	{
		UT_DEBUGMSG(("fp_Line::findNextTabStop: iStartX %d, getMaxWidth %d\n"
					 "			iPosition %d, iTabStopPosition %d, iType %d, iLeader %d\n",
			     iStartX, getMaxWidth(),iPosition, iTabStopPosition,static_cast<UT_sint32>(iType), static_cast<UT_sint32>(iLeader)));
		return false;
	}
}

bool	fp_Line::findPrevTabStop(UT_sint32 iStartX, UT_sint32& iPosition, eTabType & iType, eTabLeader & iLeader )
{
	UT_sint32	iTabStopPosition = 0;
	eTabType	iTabStopType = FL_TAB_NONE;
	eTabLeader	iTabStopLeader = FL_LEADER_NONE;

	UT_DebugOnly<bool> bRes = m_pBlock->findPrevTabStop(iStartX + getX(), 
							    getX() + getMaxWidth(), 
							    iTabStopPosition, 
							    iTabStopType, iTabStopLeader);
	UT_ASSERT(bRes);

	iTabStopPosition -= getX();

	if (iTabStopPosition <= getMaxWidth())
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
			     iStartX, getMaxWidth(),iPosition, iTabStopPosition, static_cast<UT_sint32>(iType), static_cast<UT_sint32>(iLeader)));
		return false;
	}
}

void fp_Line::recalcMaxWidth(bool bDontClearIfNeeded)
{
	if(m_pBlock == NULL)
	{
		return;
	}
        calcLeftBorderThick();
	UT_sint32 iX = m_pBlock->getLeftMargin();
	UT_sint32 iMaxWidth = getContainer()->getWidth();

	UT_BidiCharType iBlockDir = m_pBlock->getDominantDirection();

	if (isFirstLineInBlock())
	{
		if(iBlockDir == UT_BIDI_LTR)
			iX += m_pBlock->getTextIndent();
	}
	setSameYAsPrevious(false);
	setWrapped(false);
	setX(iX,bDontClearIfNeeded);

	UT_ASSERT(iMaxWidth > 0);

	fl_DocSectionLayout * pSL =  getBlock()->getDocSectionLayout();
	UT_ASSERT(pSL);
	if(pSL->getNumColumns() > 1)
	{
		if(getContainer()->getContainerType() == FP_CONTAINER_COLUMN ||
			getContainer()->getContainerType() == FP_CONTAINER_COLUMN_SHADOW ||
			getContainer()->getContainerType() == FP_CONTAINER_HDRFTR ||
			getContainer()->getContainerType() == FP_CONTAINER_TOC ||
			getContainer()->getContainerType() == FP_CONTAINER_FOOTNOTE||
			getContainer()->getContainerType() == FP_CONTAINER_ANNOTATION||
			getContainer()->getContainerType() == FP_CONTAINER_ENDNOTE)
		{
			m_iClearToPos = iMaxWidth + pSL->getColumnGap();
			m_iClearLeftOffset = pSL->getColumnGap() - getGraphics()->tlu(1);
		}
		else if(getContainer()->getContainerType() == FP_CONTAINER_CELL)
		{
			fp_CellContainer * pCell = static_cast<fp_CellContainer *>(getContainer());
			m_iClearToPos = static_cast<UT_sint32>(iMaxWidth + pCell->getRightPad());
//			m_iClearLeftOffset = pCell->getCellX(this) - pCell->getLeftPos() - getGraphics()->tlu(1);
			m_iClearLeftOffset =  0;
		}
		else if(getContainer()->getContainerType() == FP_CONTAINER_FRAME)
		{
			m_iClearToPos = iMaxWidth;
			m_iClearLeftOffset = 0;
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			m_iClearToPos = iMaxWidth;
			m_iClearLeftOffset = pSL->getLeftMargin() - getGraphics()->tlu(1);
		}
	}
	else
	{
		if(getContainer()->getContainerType() == FP_CONTAINER_COLUMN ||
			getContainer()->getContainerType() == FP_CONTAINER_COLUMN_SHADOW ||
			getContainer()->getContainerType() == FP_CONTAINER_HDRFTR ||
			getContainer()->getContainerType() == FP_CONTAINER_TOC||
			getContainer()->getContainerType() == FP_CONTAINER_FOOTNOTE||
			getContainer()->getContainerType() == FP_CONTAINER_ANNOTATION||
			getContainer()->getContainerType() == FP_CONTAINER_ENDNOTE)
		{
			m_iClearToPos = iMaxWidth + pSL->getRightMargin() - getGraphics()->tlu(2);
			m_iClearLeftOffset = pSL->getLeftMargin() - getGraphics()->tlu(1);
		}
		else if(getContainer()->getContainerType() == FP_CONTAINER_FRAME)
		{
			m_iClearToPos = iMaxWidth;
			m_iClearLeftOffset = 0;
		}
		else if(getContainer()->getContainerType() == FP_CONTAINER_CELL)
		{
			fp_CellContainer * pCell = static_cast<fp_CellContainer *>(getContainer());
			m_iClearToPos = static_cast<UT_sint32>(iMaxWidth + pCell->getRightPad());
//			m_iClearLeftOffset =  pCell->getCellX(this) - pCell->getLeftPos() - getGraphics()->tlu(1);
			m_iClearLeftOffset =  0;
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			m_iClearToPos = iMaxWidth;
			m_iClearLeftOffset = pSL->getLeftMargin() - getGraphics()->tlu(1);
		}
	}

	if (m_iClearLeftOffset < 0)
	{
		m_iClearLeftOffset = 0;
	}
	if(hasBordersOrShading())
	{
		m_iClearToPos = getRightEdge();
		m_iClearLeftOffset = 0;
	}
	if (getPage() && (getPage()->getWidth() - m_iMaxWidth < m_iClearLeftOffset))
	{
		m_iClearLeftOffset = getPage()->getWidth() - m_iMaxWidth;
		UT_ASSERT(m_iClearLeftOffset >= 0);
	}


	iMaxWidth -= m_pBlock->getRightMargin();
	iMaxWidth -= m_pBlock->getLeftMargin();
	m_iClearToPos -= m_pBlock->getLeftMargin();
	if (isFirstLineInBlock())
	{
		iMaxWidth -= m_pBlock->getTextIndent();
	}

	// Check that there's actually room for content -- assert is not good enough,
	// we need to handle this; there are docs around that exhibit this (e.g. 6722)
	// UT_ASSERT(iMaxWidth > 0);
	if(iMaxWidth <= 0)
	{
		// we will ignore the block indents ...
		// (we should probably change the block indents)
		iX = 0;
		iMaxWidth = getContainer()->getWidth();
	}
	
	if(getPage())
	{
		UT_ASSERT(iMaxWidth <= getPage()->getWidth());
	}
	if(iMaxWidth < 60)
	{
		iMaxWidth = 60;
	}
	setMaxWidth(iMaxWidth);
}

fp_Container*	fp_Line::getNextContainerInSection(void) const
{
	if (getNext())
	{
		return static_cast<fp_Container *>(getNext());
	}

	fl_ContainerLayout* pNextBlock = m_pBlock->getNext();
	while(pNextBlock && 
		  ((pNextBlock->getContainerType() == FL_CONTAINER_ENDNOTE) || 
		   (pNextBlock->getContainerType() == FL_CONTAINER_FRAME) ||
		  (pNextBlock->isHidden() == FP_HIDDEN_FOLDED)))
	{
		pNextBlock = pNextBlock->getNext();
	}
	if (pNextBlock)
	{
		return static_cast<fp_Container *>(pNextBlock->getFirstContainer());
	}
	return NULL;
}

fp_Container*	fp_Line::getPrevContainerInSection(void) const
{
	if (getPrev())
	{
		return static_cast<fp_Container *>(getPrev());
	}

	fl_ContainerLayout* pPrev =  static_cast<fl_ContainerLayout *>(m_pBlock->getPrev());
	while(pPrev && 
		  ((pPrev->getContainerType() == FL_CONTAINER_ENDNOTE) || 
		   (pPrev->getContainerType() == FL_CONTAINER_FRAME) ||
		   (pPrev->isHidden() == FP_HIDDEN_FOLDED)))
	{
		pPrev = pPrev->getPrev();
	}
	if(pPrev)
	{
		fp_Container * pPrevCon = static_cast<fp_Container *>(pPrev->getLastContainer());
//
// Have to handle broken tables in the previous layout..
//
		if(pPrevCon && pPrevCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pPrevCon);
			fp_TableContainer * pLLast = pTab;
			fp_TableContainer * pNext = static_cast<fp_TableContainer *>(pTab->getNext());
			while(pNext)
			{
				pLLast = pNext;
				pNext = static_cast<fp_TableContainer *>(pNext->getNext());
			}
			pPrevCon = static_cast<fp_Container *>(pLLast);
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
		if(pRun->getPrevRun() && (pRun->getPrevRun()->getType() == FPRUN_FORCEDCOLUMNBREAK))
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
		if(pRun->getPrevRun() && (pRun->getPrevRun()->getType() == FPRUN_FORCEDPAGEBREAK))
		{
			return true;
		}
	}
	return false;
}

/*!
    Call to this function must be followed by calling fp_Line::layout() since coalescing
    runs might require shaping of the resulting run, which in turn might result in lost of
    justification information (this is due to the fact that the data used by the external
    shaping engine can be opaque to us, so we are not able to combine it for the two
    original runs -- we have to ask the external engine to recalculate it for the whole
    combined run).

    At the moment we simply call coalesceRuns() just before doing layout() in fb_LineBreaker().
*/
void fp_Line::coalesceRuns(void)
{
	xxx_UT_DEBUGMSG(("coalesceRuns (line 0x%x)\n", this));
	UT_sint32 count = m_vecRuns.getItemCount();
	for (UT_sint32 i=0; i < static_cast<UT_sint32>(count-1); i++)
	{
		fp_Run* pRun = m_vecRuns.getNthItem(static_cast<UT_uint32>(i));
		if (pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);
			xxx_UT_DEBUGMSG(("Looking at %d Text run \n",i));
			//	pTR->printText();
			if (pTR->canMergeWithNext())
			{
			        xxx_UT_DEBUGMSG(("Can merge \n"));
				//pTR->printText();
			        fp_Run * pNext = pRun->getNextRun();
				//
				// Look if we have a redundant fmtMark.
				// If so remove it
				//
				if(pNext->getType() == FPRUN_FMTMARK)
				{
				    pRun->setNextRun(pNext->getNextRun(), false);
				    pNext->getNextRun()->setPrevRun(pRun, false);
				    removeRun(pNext, false);
				    delete pNext;
				    count--;
				    continue;
				}
				pTR->mergeWithNext();
				count--;
				i--; //test the newly merged run with the next
			}
		}
		
	}
}

UT_sint32 fp_Line::calculateWidthOfLine(void)
{
	const UT_sint32 iCountRuns = m_vecRuns.getItemCount();
	UT_sint32 iX = 0;

	// first calc the width of the line
	for (UT_sint32 i = 0; i < iCountRuns; ++i)
	{
		const fp_Run* pRun = m_vecRuns.getNthItem(i);

		if(pRun->isHidden())
			continue;

		iX += pRun->getWidth();
	}
	// this is a wrong assert, since line can include trailing spaces
	// that are out of the margins.
	//UT_ASSERT(iX <= m_iMaxWidth);

	m_iWidth = iX;
//	UT_ASSERT(m_iWidth > 0);
	return iX;
}

UT_sint32 fp_Line::calculateWidthOfTrailingSpaces(void)
{
	// need to move back until we find the first non blank character and
	// return the distance back to this character.

	UT_ASSERT(!isEmpty());

	UT_sint32 iTrailingBlank = 0;


	UT_BidiCharType iBlockDir = m_pBlock->getDominantDirection();
	UT_sint32 i;
	UT_sint32 iCountRuns = m_vecRuns.getItemCount();

	for (i=iCountRuns -1 ; i >= 0; i--)
	{
		// work from the run on the visual end of the line
		UT_sint32 k = iBlockDir == UT_BIDI_LTR ? i : iCountRuns - i - 1;
		fp_Run* pRun = m_vecRuns.getNthItem(_getRunLogIndx(k));

		if(pRun->isHidden())
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

UT_uint32 fp_Line::countJustificationPoints(void)
{
	UT_sint32 iCountRuns = m_vecRuns.getItemCount();
	UT_sint32 i;
	UT_uint32 iSpaceCount = 0;
	bool bStartFound = false;

	UT_BidiCharType iBlockDir = m_pBlock->getDominantDirection();

	// first calc the width of the line
	for (i=iCountRuns -1 ; i >= 0; i--)
	{
		// work from the run on the visual end of the line
		UT_sint32 k = iBlockDir == UT_BIDI_LTR ? i : iCountRuns - i - 1;
		fp_Run* pRun = m_vecRuns.getNthItem(_getRunLogIndx(k));

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
			UT_sint32 iPointCount = pTR->countJustificationPoints(!bStartFound);
			if(bStartFound)
			{
				iSpaceCount += abs(iPointCount);
			}
			else
			{
				if(iPointCount >= 0)
				{
					// we found our first non-blank run; the point
					iSpaceCount += iPointCount;
					bStartFound = true;
				}

			}
		}
		else if (pRun->getType () == FPRUN_FORCEDLINEBREAK || pRun->getType () == FPRUN_FORCEDPAGEBREAK ||
				 pRun->getType () == FPRUN_FORCEDCOLUMNBREAK)
		{
			// why do we count these types of run as justifiable spaces? Tomas, Apr 8, 2004
			iSpaceCount++;
		}
		else if (   pRun->getType () == FPRUN_DIRECTIONMARKER || pRun->getType () == FPRUN_FMTMARK
				 || pRun->getType () == FPRUN_BOOKMARK || pRun->getType () == FPRUN_HYPERLINK)
		{
			// these runs do not expand under justification, but neither do they contain non-blank data
			continue;
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

void fp_Line::resetJustification(bool bPermanent)
{
	UT_sint32 count = m_vecRuns.getItemCount();
	for (UT_sint32 i=0; i<count; i++)
	{
		fp_Run* pRun = m_vecRuns.getNthItem(i);

		if (pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);

			pTR->resetJustification(bPermanent);
		}
	}
}


void fp_Line::justify(UT_sint32 iAmount)
{
	if(iAmount > 0)
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

			UT_BidiCharType iBlockDir = m_pBlock->getDominantDirection();
			UT_sint32 count = m_vecRuns.getItemCount();
			UT_ASSERT(count);

			xxx_UT_DEBUGMSG(("DOM: must split iAmount %d between iSpaceCount %d spaces for count %d runs\n", iAmount, iSpaceCount, count));

			for (UT_sint32 i=count-1; i >= 0 && iSpaceCount > 0; i--)
			{
				// work from the run on the visual end of the line
				UT_sint32 k = iBlockDir == UT_BIDI_LTR ? i : count  - i - 1;
				fp_Run* pRun = m_vecRuns.getNthItem(_getRunLogIndx(k));

				if (pRun->getType() == FPRUN_TAB)
				{
					UT_ASSERT(iSpaceCount == 0);
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					break;
				}
				else if (pRun->getType() == FPRUN_TEXT)
				{
					fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);

					UT_sint32 iSpacesInText = pTR->countJustificationPoints(!bFoundStart);

					if(!bFoundStart && iSpacesInText >= 0)
						bFoundStart = true;

					if(bFoundStart && iSpacesInText)
					{
						UT_uint32 iMySpaces = abs(iSpacesInText);
						UT_sint32 iJustifyAmountForRun;						

						if (iSpaceCount-1 > 0)
							iJustifyAmountForRun = static_cast<int>(static_cast<double>(iAmount) / (iSpaceCount) * iMySpaces);
						else
							iJustifyAmountForRun = iAmount;

						// I am not sure why this was here, but it breaks justification
						// at least on win32 with Uniscribe we can have runs that contain
						// only a space and that do count toward justification. Tomas
						// if (iSpaceCount == 1) iJustifyAmountForRun = 0;
						pTR->justify(iJustifyAmountForRun, iMySpaces);

						iAmount -= iJustifyAmountForRun;
						iSpaceCount -= iMySpaces;
					}
					else if(!bFoundStart && iSpacesInText)
					{
						// trailing space, need to do this so that the
						// trailing spaces do not get included when
						// this run is merged with previous one
						
						pTR->justify(0, 0);
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
	UT_sint32 count = m_vecRuns.getItemCount();
	if(!count)
		return;

	UT_sint32 countOrig = count;

	for (UT_sint32 i = 0; i < count; i++)
	{
		fp_Run* pRun = m_vecRuns.getNthItem(i);

		if (pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);
			UT_sint32 iSpacePosition;

			iSpacePosition = pTR->findCharacter(0, UCS_SPACE);

			if ((iSpacePosition > 0) &&
				(static_cast<UT_uint32>(iSpacePosition) < pTR->getBlockOffset() + pTR->getLength() - 1))
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
		fp_TextRun* pTR = static_cast<fp_TextRun *>(pRun);
		UT_sint32 iSpacePosition = pTR->findCharacter(0, UCS_SPACE);

		if ((iSpacePosition > 0) &&
			(static_cast<UT_uint32>(iSpacePosition) < pTR->getBlockOffset() + pTR->getLength() - 1))
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
	UT_sint32 i=0;

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
		UT_sint32 count = m_vecRuns.getItemCount();
		if(!count)
			return UT_OK;  // do not even try to map a line with no runs

#if 0
		if(count == 1)	 //if there is just one run, then make sure that it maps on itself and return
		{
			s_pMapOfRunsL2V[0] = 0;
			s_pMapOfRunsV2L[0] = 0;
			return UT_OK;
		}
#endif
		if (count >= s_iMapOfRunsSize) //the MapOfRuns member is too small, reallocate
		{
			delete[] s_pMapOfRunsL2V;
			delete[] s_pMapOfRunsV2L;
			delete[] s_pPseudoString;
			delete[] s_pEmbeddingLevels;

			s_iMapOfRunsSize = count + 20; //allow for 20 extra runs, so that we do not have to
										   //do this immediately again
			s_pMapOfRunsL2V = new UT_uint32[s_iMapOfRunsSize];
			s_pMapOfRunsV2L = new UT_uint32[s_iMapOfRunsSize];
			s_pPseudoString    = new UT_UCS4Char[s_iMapOfRunsSize];
			s_pEmbeddingLevels =  new UT_Byte[s_iMapOfRunsSize];


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

			s_pMapOfRunsL2V = new UT_uint32[s_iMapOfRunsSize];
			s_pMapOfRunsV2L = new UT_uint32[s_iMapOfRunsSize];
			s_pPseudoString    = new UT_UCS4Char[RUNS_MAP_SIZE];
			s_pEmbeddingLevels =  new UT_Byte[RUNS_MAP_SIZE];


			UT_ASSERT(s_pMapOfRunsL2V && s_pMapOfRunsV2L && s_pPseudoString && s_pEmbeddingLevels);
		}

		FV_View * pView = getSectionLayout()->getDocLayout()->getView();
		
		if((pView && pView->getBidiOrder() == FV_Order_Logical_LTR) || !m_iRunsRTLcount)
		{
			xxx_UT_DEBUGMSG(("_createMapOfRuns: ltr line only (line 0x%x)\n", this));
			for (i = 0; i < count; i++)
			{
				s_pMapOfRunsL2V[i] = i;
				s_pMapOfRunsV2L[i] = i;
				m_vecRuns.getNthItem(i)->setVisDirection(UT_BIDI_LTR);
			}
			return UT_OK;
		}
		else

		//if this is unidirectional rtl text, we just fill the map sequentially
		//from back to start
		if((pView && pView->getBidiOrder() == FV_Order_Logical_RTL) || !m_iRunsLTRcount)
		{
			xxx_UT_DEBUGMSG(("_createMapOfRuns: rtl line only (line 0x%x)\n", this));
			for(i = 0; i < count/2; i++)
			{
				s_pMapOfRunsL2V[i]= count - i - 1;
				s_pMapOfRunsV2L[i]= count - i - 1;
				s_pMapOfRunsL2V[count - i - 1] = i;
				s_pMapOfRunsV2L[count - i - 1] = i;
				m_vecRuns.getNthItem(i)->setVisDirection(UT_BIDI_RTL);
				m_vecRuns.getNthItem(count - i - 1)->setVisDirection(UT_BIDI_RTL);
			}

			if(count % 2)	//the run in the middle
			{
				s_pMapOfRunsL2V[count/2] = count/2;
				s_pMapOfRunsV2L[count/2] = count/2;
				m_vecRuns.getNthItem(count/2)->setVisDirection(UT_BIDI_RTL);

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
				iRunDirection = m_vecRuns.getNthItem(i)->getDirection();
				switch(iRunDirection)
				{
					case UT_BIDI_LTR : s_pPseudoString[i] = static_cast<UT_UCS4Char>('a'); break;
					case UT_BIDI_RTL : s_pPseudoString[i] = static_cast<UT_UCS4Char>(0x05d0); break;
					//case UT_BIDI_WL
					//case UT_BIDI_WR
					case UT_BIDI_EN  : s_pPseudoString[i] = static_cast<UT_UCS4Char>('0'); break;
					case UT_BIDI_ES  : s_pPseudoString[i] = static_cast<UT_UCS4Char>('/'); break;
					case UT_BIDI_ET  : s_pPseudoString[i] = static_cast<UT_UCS4Char>('#'); break;
					case UT_BIDI_AN  : s_pPseudoString[i] = static_cast<UT_UCS4Char>(0x0660); break;
					case UT_BIDI_CS  : s_pPseudoString[i] = static_cast<UT_UCS4Char>(','); break;
					case UT_BIDI_BS  : s_pPseudoString[i] = static_cast<UT_UCS4Char>(0x000A); break;
					case UT_BIDI_SS  : s_pPseudoString[i] = static_cast<UT_UCS4Char>(0x000B); break;
					case UT_BIDI_WS  : s_pPseudoString[i] = static_cast<UT_UCS4Char>(' '); break;
					case UT_BIDI_AL  : s_pPseudoString[i] = static_cast<UT_UCS4Char>(0x062D); break;
					case UT_BIDI_NSM : s_pPseudoString[i] = static_cast<UT_UCS4Char>(0x0300); break;
					case UT_BIDI_LRE : s_pPseudoString[i] = static_cast<UT_UCS4Char>(0x202A); break;
					case UT_BIDI_RLE : s_pPseudoString[i] = static_cast<UT_UCS4Char>(0x202B); break;
					case UT_BIDI_LRO : s_pPseudoString[i] = static_cast<UT_UCS4Char>(0x202D); break;
					case UT_BIDI_RLO : s_pPseudoString[i] = static_cast<UT_UCS4Char>(0x202E); break;
					case UT_BIDI_PDF : s_pPseudoString[i] = static_cast<UT_UCS4Char>(0x202C); break;
					case UT_BIDI_ON  : s_pPseudoString[i] = static_cast<UT_UCS4Char>('!'); break;

				}
				xxx_UT_DEBUGMSG(("fp_Line::_createMapOfRuns: pseudo char 0x%x\n",s_pPseudoString[i]));
			}

			UT_BidiCharType iBlockDir = m_pBlock->getDominantDirection();

			/*bool bRet =*/ UT_bidiMapLog2Vis(s_pPseudoString, count, iBlockDir,
										  s_pMapOfRunsL2V, s_pMapOfRunsV2L, s_pEmbeddingLevels);

			 //the only other thing that remains is to pass the visual
			 //directions down to the runs.
			 for (i=0; i<count;i++)
			 {
				m_vecRuns.getNthItem(i)->setVisDirection(s_pEmbeddingLevels[i]%2 ? UT_BIDI_RTL : UT_BIDI_LTR);
				xxx_UT_DEBUGMSG(("L2V %d, V2L %d, emb. %d [run 0x%x]\n", s_pMapOfRunsL2V[i],s_pMapOfRunsV2L[i],s_pEmbeddingLevels[i],m_vecRuns.getNthItem(i)));
			 }
		}//if/else only rtl
	}

	return(UT_OK);
}

/* the following two functions convert the position of a run from logical to visual
   and vice versa */

UT_uint32 fp_Line::_getRunLogIndx(UT_sint32 indx)
{
#ifdef DEBUG
	UT_sint32 iCount = m_vecRuns.getItemCount();
	if(iCount <= indx)
		UT_DEBUGMSG(("fp_Line::_getRunLogIndx: indx %d, iCount %d\n", indx,iCount));
#endif
	UT_ASSERT((m_vecRuns.getItemCount() > indx));

	if(!m_iRunsRTLcount)
		return(indx);

	_createMapOfRuns();
	return(s_pMapOfRunsV2L[indx]);
}


UT_uint32 fp_Line::_getRunVisIndx(UT_sint32 indx)
{
	UT_ASSERT(m_vecRuns.getItemCount() > indx);

	if(!m_iRunsRTLcount)
		return(indx);

	_createMapOfRuns();
	return(s_pMapOfRunsL2V[indx]);
}

UT_uint32	fp_Line::getVisIndx(fp_Run* pRun)
{
	UT_sint32 i = m_vecRuns.findItem(pRun);
	UT_ASSERT(i >= 0);
	return _getRunVisIndx(static_cast<UT_uint32>(i));
}

fp_Run *	fp_Line::getRunAtVisPos(UT_sint32 i)
{
	if(i >= m_vecRuns.getItemCount())
		return NULL;
	return m_vecRuns.getNthItem(_getRunLogIndx(i));
}

fp_Run * fp_Line::getLastVisRun()
{
	if(!m_iRunsRTLcount)
		return(getLastRun());

	_createMapOfRuns();
	UT_sint32 count = m_vecRuns.getItemCount();
	UT_ASSERT(count > 0);
	return m_vecRuns.getNthItem(s_pMapOfRunsV2L[count - 1]);
}

fp_Run * fp_Line::getFirstVisRun()
{
	if(!m_iRunsRTLcount)
		return(0);

	_createMapOfRuns();
	return m_vecRuns.getNthItem(s_pMapOfRunsV2L[0]);
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

void fp_Line::addDirectionUsed(UT_BidiCharType dir, bool bRefreshMap)
{
	if(UT_BIDI_IS_RTL(dir))
	{
		m_iRunsRTLcount++;
		xxx_UT_DEBUGMSG(("fp_Line::addDirectionUsed: increased RTL run count [%d, this=0x%x, bRefresh=%d]\n", m_iRunsRTLcount, this, bRefreshMap));
	}
	else if(!UT_BIDI_IS_NEUTRAL(dir))
	{
		m_iRunsLTRcount++;
		xxx_UT_DEBUGMSG(("fp_Line::addDirectionUsed: increased LTR run count [%d, this=0x%x, bRefresh=%d]\n", m_iRunsLTRcount, this, bRefreshMap));
	}
	
	if(bRefreshMap && (dir != static_cast<UT_BidiCharType>(UT_BIDI_UNSET)))
	{
		m_bMapDirty = true;
		//_createMapOfRuns();
	}
}

void fp_Line::removeDirectionUsed(UT_BidiCharType dir, bool bRefreshMap)
{
	if(UT_BIDI_IS_RTL(dir))
	{
		m_iRunsRTLcount--;
		xxx_UT_DEBUGMSG(("fp_Line::removeDirectionUsed: increased RTL run count [%d, this=0x%x, bRefresh=%d]\n", m_iRunsRTLcount, this, bRefreshMap));
	}
	else if(!UT_BIDI_IS_NEUTRAL(dir))
	{
		m_iRunsLTRcount--;
		xxx_UT_DEBUGMSG(("fp_Line::removeDirectionUsed: increased LTR run count [%d, this=0x%x, bRefresh=%d]\n", m_iRunsLTRcount, this, bRefreshMap));
	}

	if(bRefreshMap && (dir != static_cast<UT_BidiCharType>(UT_BIDI_UNSET)))
	{
		m_bMapDirty = true;
		//_createMapOfRuns();
	}
}

void fp_Line::changeDirectionUsed(UT_BidiCharType oldDir, UT_BidiCharType newDir, bool bRefreshMap)
{
	if(oldDir == newDir)
		return;
	
	if(UT_BIDI_IS_RTL(newDir))
	{
		m_iRunsRTLcount++;
		xxx_UT_DEBUGMSG(("fp_Line::changeDirectionUsed: increased RTL run count [%d, this=0x%x, bRefresh=%d]\n", m_iRunsRTLcount, this, bRefreshMap));
	}
	else if(!UT_BIDI_IS_NEUTRAL(newDir))
	{
		m_iRunsLTRcount++;
		xxx_UT_DEBUGMSG(("fp_Line::changeDirectionUsed: increased LTR run count [%d, this=0x%x, bRefresh=%d]\n", m_iRunsLTRcount, this, bRefreshMap));
	}
	
	if(UT_BIDI_IS_RTL(oldDir))
	{
		m_iRunsRTLcount--;
		xxx_UT_DEBUGMSG(("fp_Line::changeDirectionUsed: increased RTL run count [%d, this=0x%x, bRefresh=%d]\n", m_iRunsRTLcount, this, bRefreshMap));
	}
	else if(!UT_BIDI_IS_NEUTRAL(oldDir))
	{
		m_iRunsLTRcount--;
		xxx_UT_DEBUGMSG(("fp_Line::changeDirectionUsed: increased LTR run count [%d, this=0x%x, bRefresh=%d]\n", m_iRunsLTRcount, this, bRefreshMap));
	}


	if(bRefreshMap && (newDir != static_cast<UT_BidiCharType>(UT_BIDI_UNSET)))
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

	UT_sint32 count = m_vecRuns.getItemCount();
	for (UT_sint32 i = 0; i < count; i++)
	{
		const fp_Run * r = static_cast<const fp_Run *>(m_vecRuns.getNthItem(i));
		if (r->getType() == FPRUN_FIELD)
		{
			const fp_FieldRun * fr = static_cast<const fp_FieldRun*>(r);
			if (fr->getFieldType() == FPFIELD_endnote_ref)
				m_bContainsFootnoteRef = true;
		}
	}
}

UT_sint32 fp_Line::getDrawingWidth() const
{
	if(isLastLineInBlock())
	{
		const fp_Run * pRun = getLastRun();
		UT_return_val_if_fail(pRun && pRun->getType() == FPRUN_ENDOFPARAGRAPH, m_iWidth);
		return (m_iWidth + (static_cast<const fp_EndOfParagraphRun*>(pRun))->getDrawingWidth());
	}
	else
	{
		return m_iWidth;
	}
}
