/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <string.h>

#include "ut_types.h"
#include "ut_misc.h"

#include "fp_Page.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fp_Column.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fl_BlockLayout.h"
#include "gr_Graphics.h"
#include "gr_DrawArgs.h"
#include "fv_View.h"
#include "fp_FootnoteContainer.h"
#include "fl_TOCLayout.h"
#include "fl_FootnoteLayout.h"
#include "fp_FrameContainer.h"
#include "fl_FrameLayout.h"
#include "fp_TableContainer.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"
#include "gr_Painter.h"

fp_Page::fp_Page(FL_DocLayout* pLayout,
		 FV_View* pView,
		 const fp_PageSize& pageSize,
		 fl_DocSectionLayout* pOwner)
	:	m_pLayout(pLayout),
		m_pView(pView),
		m_pNext(0),
		m_pPrev(0),
		m_pageSize(pageSize),
		m_bNeedsRedraw(true),
		m_pOwner(pOwner),
		m_pFooter(0),
		m_pHeader(0),
		m_FillType(NULL,NULL,FG_FILL_TRANSPARENT),
		m_pLastMappedTOC(NULL),
		m_iCountWrapPasses(0),
		m_iFieldPageNumber(-1)
{
	UT_ASSERT(pLayout);
	UT_ASSERT(pOwner);

	GR_Graphics * pG = pLayout->getGraphics();
	UT_ASSERT(pG);
	m_vecColumnLeaders.clear();
	m_rDamageRect.left = 0;
	m_rDamageRect.top = 0;
	m_rDamageRect.width = 0;
	m_rDamageRect.height = 0;
	m_vecFootnotes.clear();
	m_vecAnnotations.clear();
	m_vecAboveFrames.clear();
	m_vecBelowFrames.clear();

	xxx_UT_DEBUGMSG(("!!!!!!!!!!!!!!!!!!!!!!!!!!Created Page %x \n",this));
}

fp_Page::~fp_Page()
{
	xxx_UT_DEBUGMSG(("fpPage: Deleting page %x \n",this));
	if (m_pOwner)
	{
		fl_DocSectionLayout *pDSL = m_pOwner;
		m_pOwner = NULL;
		pDSL->deleteOwnedPage(this);
	}
	if((m_pHeader != NULL) || (m_pFooter != NULL))
	{
	    fl_HdrFtrSectionLayout * pHdrFtr = NULL;
	    if(m_pHeader != NULL)
	    {
	         pHdrFtr = m_pHeader->getHdrFtrSectionLayout();
		 if(pHdrFtr != NULL && pHdrFtr->isPageHere(this))
		 {
		   pHdrFtr->deletePage(this);
		   UT_DEBUGMSG(("Remove Page from Hdr %p in page destructor \n",pHdrFtr));
		 }
	    }
	    if(m_pFooter != NULL)
	    {
	         pHdrFtr = m_pFooter->getHdrFtrSectionLayout();
		 if(pHdrFtr != NULL && pHdrFtr->isPageHere(this))
		 {
		   pHdrFtr->deletePage(this);
		   UT_DEBUGMSG(("Remove Page from Ftr %p in page destructor \n",pHdrFtr));
		 }
	    }
	}
	DELETEP(m_pHeader);
	DELETEP(m_pFooter);
}

/*!
 * FillType class for the page.
 */
const fg_FillType & fp_Page::getFillType(void) const
{
	return m_FillType;
}

/*!
 * FillType class for the page.
 */
fg_FillType & fp_Page::getFillType(void)
{
	return m_FillType;
}

bool fp_Page::isEmpty(void) const
{
	if((m_vecColumnLeaders.getItemCount() == 0) && (m_vecFootnotes.getItemCount() == 0) && (m_vecAnnotations.getItemCount() == 0) && (m_vecAboveFrames.getItemCount() == 0) && (m_vecBelowFrames.getItemCount() == 0))
	{
		return true;
	}
	return false;
}

/*!
 * This method sets the page number in all thepages frames.
 */

void fp_Page::setPageNumberInFrames(void)
{
	UT_sint32 iPage = getDocLayout()->findPage(this);
	UT_sint32 i = 0;
	for(i=0; i< static_cast<UT_sint32>(countAboveFrameContainers()); i++)
	{
		fp_FrameContainer * pFrame = getNthAboveFrameContainer(i);
		pFrame->setPreferedPageNo(iPage);
	}
	for(i=0; i< static_cast<UT_sint32>(countBelowFrameContainers()); i++)
	{
		fp_FrameContainer * pFrame = getNthBelowFrameContainer(i);
		pFrame->setPreferedPageNo(iPage);
	}
}

/*!
 * Fill a vector with all the layouts referenced from this page.
 */
void fp_Page::getAllLayouts(UT_GenericVector<fl_ContainerLayout *> & AllLayouts) const
{
	fp_Column * pCol = NULL;
	UT_sint32 i = 0;
	fl_ContainerLayout * pPrevCL = NULL;
	fl_ContainerLayout * pCurCL = NULL;
	for(i= 0; i< m_vecColumnLeaders.getItemCount(); i++)
	{
		pCol = m_vecColumnLeaders.getNthItem(i);
		while(pCol)
		{
			UT_sint32 j= 0;
			fp_ContainerObject * pCon = NULL;
			for(j = 0; j< pCol->countCons(); j++)
			{
				pCon = pCol->getNthCon(j);
				if(pCon->getContainerType() == FP_CONTAINER_LINE)
				{
					pCurCL = static_cast<fl_ContainerLayout *>(static_cast<fp_Line *>(pCon)->getBlock());
					if(pCurCL != pPrevCL)
					{
						pPrevCL = pCurCL;
						AllLayouts.addItem(pPrevCL);
					}
				}
				if(pCon->getContainerType() == FP_CONTAINER_TABLE)
				{
					pCurCL = static_cast<fl_ContainerLayout *>(static_cast<fp_TableContainer *>(pCon)->getSectionLayout());
					if(pCurCL != pPrevCL)
					{
						pPrevCL = pCurCL;
						AllLayouts.addItem(pPrevCL);
					}
				}

			}
			pCol = pCol->getFollower();
		}
	}
}

bool fp_Page::isOnScreen(void)
{
	if(!m_pView)
	{
	    return false;
	}
	UT_sint32 xoff,yoff;
	m_pView->getPageScreenOffsets(this,xoff,yoff);
	if(yoff+getHeight() < 0)
	{
		return false;
	}
	if(!m_pView)
		return false;
	if(yoff > m_pView->getWindowHeight())
	{
		return false;
	}
	return true;
}

UT_sint32 fp_Page::getWidth(void) const
{
	return m_pLayout->getDocPageWidth();
}

UT_sint32 fp_Page::getHeight(void) const
{
	return m_pLayout->getDocPageHeight();
}

UT_sint32 fp_Page::getColumnGap(void) const
{
	return getOwningSection()->getColumnGap();
}

void fp_Page::clearCountWrapNumber(void)
{
	m_iCountWrapPasses = 0;
}
/*!
 * This method scans the page, looking for lines that overlap wrapped
 * positioned frames. As it finds them it records the line and the block
 * After scanning the page it calls a method in the block to rebreak
 * the papargraph starting from the line supplied, but now making sure
 * to not overlap any wrapped objects.
 * If it does a re-break of any paragraph it returns the first container
 * in the page. 
 * and sets pNextColumn to the first column on the page.
 * fp_ColumnBreaker then lays out the page again.
 * If there are no rebreaks it returns NULL and fp_ColumnBreaker moves on to
 * the next page.
 * pNextCol is the next column fb_ColumnBreaker will evaluate it is used
 * as an output..
 */
fp_Container * fp_Page::updatePageForWrapping(fp_Column *& pNextCol)
{
	if(m_iCountWrapPasses > 19)
	{
		return NULL;
	}
	m_iCountWrapPasses++;
#if 0
	UT_sint32 iPage = getDocLayout()->findPage(this);
	UT_DEBUGMSG(("Wrap passes = %d page %x page number %d \n",m_iCountWrapPasses,this,iPage ));
	if(getPrev())
		{
			iPage = getDocLayout()->findPage(getPrev());
			UT_DEBUGMSG(("Prev page %x Prev page number %d Number Frames %d \n",getPrev(),iPage,getPrev()->countAboveFrameContainers() ));
		}
#endif
	if(m_iCountWrapPasses > 10)
    {
		UT_DEBUGMSG(("Number of wrapped passed = %d \n",m_iCountWrapPasses));
	}
	UT_sint32 i= 0;
	UT_sint32 nWrapped = 0;
	fp_Container * pFirst2 = NULL;
	fl_BlockLayout * pFirstBL = NULL;
	for(i=0; i < static_cast<UT_sint32>(countColumnLeaders()); i++)
	{
		fp_Column * pCol = getNthColumnLeader(i);
		if(i == 0)
		{
			pFirst2 = static_cast<fp_Container *>(pCol->getNthCon(0));
			if(pFirst2 == NULL)
			{
				return NULL;
			}
		}
		while(pCol)
		{
			if(m_iCountWrapPasses > 10)
			{
				nWrapped += pCol->countWrapped();
			}
			else
			{
				nWrapped += pCol->getNumWrapped();
			}
			UT_DEBUGMSG(("Page number %d \n",getDocLayout()->findPage(this)));
			UT_DEBUGMSG(("NumWrapped %d \n",nWrapped));
			pCol = static_cast<fp_Column *>(pCol->getFollower());
		}
	}
	UT_sint32 nWrappedObjs = 0;
	for(i=0; i< static_cast<UT_sint32>(countAboveFrameContainers()); i++)
	{
		fp_FrameContainer * pFrame = getNthAboveFrameContainer(i);
		if(pFrame->isWrappingSet())
		{
			nWrappedObjs++;
		}
	}

	if((nWrapped == 0) && (nWrappedObjs == 0))
	{
		xxx_UT_DEBUGMSG(("page %x nWrapped %d nWrappedObjs %d does not need updating \n",this,nWrapped,nWrappedObjs));
		fp_Page * pPrevPage = getPrev();
		//
		// In creating lines that correctly wrap frames on the previous page
		// the next page may have inherited some partially broken lines. 
		// The following code check for the this situation so it can be 
		// fixed here.
		//
		if(pPrevPage && (pPrevPage->countAboveFrameContainers() > 0))
		{
				for(i=0; i< static_cast<UT_sint32>(pPrevPage->countAboveFrameContainers()); i++)
				{
						fp_FrameContainer * pFrame = pPrevPage->getNthAboveFrameContainer(i);
						if(pFrame->isWrappingSet())
						{
								nWrappedObjs++;
						}
				}
				
		}
		if((nWrappedObjs == 0) &&(m_iCountWrapPasses == 1))
		{
			return NULL;
		}
		else if((nWrappedObjs == 0) && (getNext() == NULL))
		{
				return NULL;
		}
		else if(nWrappedObjs == 0)
		{
			fp_Column * pNextNoWrapCol = getNext()->getNthColumnLeader(0);
			if(pNextNoWrapCol == NULL)
			{
				// FIXME this appears to happen sometimes.
				// we should work out a better thing to do this this
				return NULL;
			}
			fp_Container * pNewFirstWrapCon = static_cast<fp_Container *>(pNextNoWrapCol->getNthCon(0));
			getNext()->clearCountWrapNumber();
			return pNewFirstWrapCon;
		}
	}
	bool bFormatAllWrapped = ((nWrapped > 0) && (nWrappedObjs == 0));
	UT_GenericVector<_BL *> vecBL;
	vecBL.clear();
	for(i=0; i < static_cast<UT_sint32>(countColumnLeaders()); i++)
	{
		fp_Column * pCol2 = getNthColumnLeader(i);
		while(pCol2)
		{
			UT_sint32 j = 0;
			for(j=0; j < pCol2->countCons(); j++)
			{
				fp_Container * pCon = static_cast<fp_Container *>(pCol2->getNthCon(j));
				xxx_UT_DEBUGMSG(("Look at con %x at j %d \n",pCon,j));
				if(pCon->getContainerType() == FP_CONTAINER_LINE)
				{
					fp_Line * pLine = static_cast<fp_Line *>(pCon);
					UT_Rect recLeft;
					UT_Rect recRight;
					pLine->genOverlapRects(recLeft,recRight);
					bool bFoundOne = false;
					if(recLeft.width == 0 && recRight.width == 0)
					{
						pLine->setWrapped(false);
					}
					else if((recLeft.width < 0) || (recRight.width <0))
					{
//
// SOMETHING HAS GONE HORRIBALLY WRONG. Try to recover by collapsing the
// content in this column and rebuilding
//
						UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
						UT_DEBUGMSG(("-ve width here!!!! %p left %d right %d \n",pLine,recLeft.width,recRight.width));
						UT_VECTOR_PURGEALL(_BL *, vecBL);
						fl_BlockLayout * pBL = pLine->getBlock();
						fl_BlockLayout * pFirst = pBL;
						fp_Column * pCol = static_cast<fp_Column *>(pLine->getColumn());
						bool bLoop = true;
						while(bLoop)
						{
							if(pBL && pBL->getContainerType() == FL_CONTAINER_BLOCK)
							{
								if(pBL->getFirstContainer() && (pBL->getFirstContainer()->getColumn() == pCol))
								{
									bLoop = true;
									pFirst = pBL;
								}
								pBL = static_cast<fl_BlockLayout *>(pBL->getPrev());
							}
							else
							{
								bLoop = false;
							}
						}
						fp_Column * pFirstCol = static_cast<fp_Column *>(pFirst->getFirstContainer()->getColumn());
						pBL = pFirst;
						UT_GenericVector<fl_BlockLayout *> vecCollapse;
						vecCollapse.addItem(pBL);
						bLoop = true;
						while(bLoop)
						{
							if(pBL && pBL->getContainerType() == FL_CONTAINER_BLOCK)
							{
								if(pBL->getFirstContainer() && (pBL->getFirstContainer()->getColumn() == pCol))
								{
									bLoop = true;
									vecCollapse.addItem(pBL);
								}
								pBL = static_cast<fl_BlockLayout *>(pBL->getNext());
							}
							else
							{
								bLoop = false;
							}
						}
						UT_sint32 k = 0;
						for(k=0; k<vecCollapse.getItemCount();k++)
						{
							pBL = vecCollapse.getNthItem(k);
							pBL->collapse();
							pBL->format();
						}
						pNextCol = pFirstCol;
						fp_Container * pNewFirstCon = static_cast<fp_Container *>(pNextCol->getNthCon(0));
						if(!pNewFirstCon)
						{
							pBL = vecCollapse.getNthItem(0);
							pNewFirstCon = static_cast<fp_Container *>(pBL->getFirstRun()->getLine());
						}
						return pNewFirstCon;
					}
					else
					{
						pLine->setWrapped(true);
						fp_Line * pPrev = static_cast<fp_Line *>(pLine->getPrev());
 						if(pPrev && !pLine->isSameYAsPrevious())
 						{
 							if(pPrev->getY() == pLine->getY())
 							{
 								pLine->setSameYAsPrevious(true);
 							}
 						}
						if(pLine->isSameYAsPrevious())
						{
						  //
						  // Look for a gap between lines.
						  //
						  UT_Rect recBetween;
						  UT_Rect * pPrevRec = pPrev->getScreenRect();
						  UT_Rect * pCurRec = pLine->getScreenRect();
						  recBetween.left = pPrevRec->left+pPrevRec->width;
						  recBetween.width = pCurRec->left - recBetween.left;
						  if(pPrevRec->height != pCurRec->height)
						  {
						         pLine = pPrev;
							 j--;
							 bFoundOne = true;
						  }
						  else
						  {
						         recBetween.height = pPrevRec->height;
							 recBetween.top = pPrevRec->top;
							 if(!overlapsWrappedFrame(recBetween))
							 {
							      UT_DEBUGMSG(("Found a gap! \n"));
							      pLine = pPrev;
							      j--;
							      bFoundOne = true;
							 }
							 else if(recBetween.width < 60)
							 {
							      UT_DEBUGMSG(("Found a tiny gap! %d \n",recBetween.width));
							      pLine = pPrev;
							      j--;
							      bFoundOne = true;
							 }
						  }
						  delete pPrevRec;
						  delete pCurRec;
						}
					}
					if(bFormatAllWrapped)
					{
						if(pLine->isWrapped())
						{
							fl_BlockLayout * pBL = pLine->getBlock();
							bool bPrev = false;
							UT_sint32 k = 0;
							for(k=0; k<vecBL.getItemCount(); k++)
							{
								_BL * ppBL = vecBL.getNthItem(k);
								if(ppBL->m_pBL == pBL)
								{
									bPrev = true;
								}
							}
							if(!bPrev)
							{
								_BL * pBLine = new _BL(pBL,pLine);
								vecBL.addItem(pBLine);
							}
							k =j;
							while(pLine && pLine->getBlock() == pBL)
							{
								k++;
								if(k >= pCol2->countCons())
								{
									break;
								}
								pCon = static_cast<fp_Container *>(pCol2->getNthCon(k));
								if(pCon->getContainerType() == FP_CONTAINER_LINE)
								{
									pLine = static_cast<fp_Line *>(pCon);
								}
								else
								{
									break;
								}
							}
							j = k-1;
						}
					}
					else
					{
//
// OK look to see if this line either overlaps a wrapped object or has
// some space where a wrapped object should be
//
						fp_Line * pPrev = static_cast<fp_Line *>(pLine->getPrev());
						if(pLine->isWrapped())
						{
							if(overlapsWrappedFrame(pLine))
							{
//
// Wrapped line overlaps a wrapped frame
//
								xxx_UT_DEBUGMSG(("Found wrapped line %x that overlaps \n",pLine));
								bFoundOne = true;
							}
							else if(pPrev && pLine->isSameYAsPrevious() && (pPrev->getY() != pLine->getY()))
							{
								pLine = pPrev;
								j--;
								bFoundOne = true;
							}
							else if(m_iCountWrapPasses < 101)
							{
//
// Look to see if the wrapped line has no white space that overlaps
// a wrapped object. We don't do this if we've tried more than 100
// time to layout the page.
//
								
								if(!overlapsWrappedFrame(recLeft) &&
								   !overlapsWrappedFrame(recRight))
								{
									xxx_UT_DEBUGMSG(("Found wrapped line with extra  space %x  \n",pLine));
									bFoundOne = true;
									if(pPrev && pLine->isSameYAsPrevious())
									{
									  pLine = pPrev;
									  j--;
									}
								}
								else if (pPrev &&  pLine->isSameYAsPrevious())
								{
								  //
								  // Look for a gap between lines.
								  //
								  UT_Rect recBetween;
								  UT_Rect * pPrevRec = pPrev->getScreenRect();
								  UT_Rect * pCurRec = pLine->getScreenRect();
								  recBetween.left = pPrevRec->left+pPrevRec->width;
								  recBetween.width = pCurRec->left - recBetween.left;
								  if(pPrevRec->height != pCurRec->height)
								  {
								    pLine = pPrev;
								    j--;
								    bFoundOne = true;
								  }
								  else
								  {
								    recBetween.height = pPrevRec->height;
								    recBetween.top = pPrevRec->top;
								    if(!overlapsWrappedFrame(recBetween))
								    {
								      pLine = pPrev;
								      j--;
								      bFoundOne = true;
								    }
								  }
								  delete pPrevRec;
								  delete pCurRec;
								}
								  
							}
							else
							{
							  xxx_UT_DEBUGMSG(("Not converging \n"));
							}

						}
						else
						{
//
// line overlaps a wrapped frame
//
							if(overlapsWrappedFrame(pLine))
							{
								xxx_UT_DEBUGMSG(("Found unwrapped line %x that overlaps \n",pLine));
								bFoundOne = true;
							}
						}
						if(bFoundOne)
						{
							fl_BlockLayout * pBL = pLine->getBlock();
							bool bPrev = false;
							UT_sint32 k = 0;
							for(k=0; k<vecBL.getItemCount(); k++)
							{
								_BL * ppBL = vecBL.getNthItem(k);
								if(ppBL->m_pBL == pBL)
								{
									bPrev = true;
								}
							}
							if(!bPrev)
							{
								_BL * pBLine = new _BL(pBL,pLine);
								vecBL.addItem(pBLine);
							}
							k =j;
							while(pLine && pLine->getBlock() == pBL)
							{
								k++;
								if(k >= pCol2->countCons())
								{
									break;
								}
								pCon = static_cast<fp_Container *>(pCol2->getNthCon(k));
								if(pCon->getContainerType() == FP_CONTAINER_LINE)
								{
									pLine = static_cast<fp_Line *>(pCon);
								}
								else
								{
									break;
								}
							}
							j = k-1;
						}
					}
				}
				if(j< 0)
				  j = 0;
			}
			pCol2 = static_cast<fp_Column *>(pCol2->getFollower());
		}
	}
	if(vecBL.getItemCount() == 0)
	{
		return NULL;
	}
	_BL * pBLine = vecBL.getNthItem(0);
	pFirstBL = pBLine->m_pBL;
	for(i=0; i<vecBL.getItemCount(); i++)
	{
		pBLine = vecBL.getNthItem(i);
		xxx_UT_DEBUGMSG((" Doing line %x \n",pBLine->m_pL));
#if DEBUG
		if(m_iCountWrapPasses > 100)
		{
			xxx_UT_DEBUGMSG(("Not converging \n"));
		}
#endif
		xxx_UT_DEBUGMSG(("Do regular rebreak \n"));
		pBLine->m_pBL->formatWrappedFromHere(pBLine->m_pL,this);
	}
	UT_VECTOR_PURGEALL(_BL *, vecBL);
	fp_Container * pNewFirstCon = NULL;
	if(pFirstBL)
	{
		pNewFirstCon = pFirstBL->getFirstContainer();
		pNextCol = static_cast<fp_Column *>(pNewFirstCon->getColumn());
	}
	else
	{
		return pNewFirstCon;
	}
	while(pNewFirstCon && pNewFirstCon->getPage() != NULL && pNewFirstCon->getPage() != this)
	{
		pNewFirstCon = static_cast<fp_Container *>(pNewFirstCon->getNext());
	}
	if(pNewFirstCon->getColumn() == NULL)
	{
	       return NULL;
	}
	pNextCol = static_cast<fp_Column *>(pNewFirstCon->getColumn());
	pNewFirstCon = static_cast<fp_Container *>(pNextCol->getNthCon(0));
	if(pNewFirstCon && pNewFirstCon->getContainerType() == FP_CONTAINER_LINE)
	{
#if DEBUG
		fp_Line * pFLine = static_cast<fp_Line *>(pNewFirstCon);
		UT_ASSERT(pFLine->getBlock() && 
				  (pFLine->getBlock()->findLineInBlock(pFLine) >= 0));
	    //	    UT_ASSERT(!pFLine-isEmpty());
#endif
	}
	return pNewFirstCon;
}


/*!
 * Returns true if the supplied rectangle overlaps with one wrapped frame
 * on the page.
 */
bool fp_Page::overlapsWrappedFrame(fp_Line * pLine)
{
	UT_Rect * pRec = pLine->getScreenRect();
	if(pRec == NULL)
	{
		return false;
	}
	bool bRes = overlapsWrappedFrame(*pRec);
	delete pRec;
	return bRes;
}
/*!
 * Returns true if the supplied rectangle overlaps with one wrapped frame
 * on the page. The rectangle is relative to the screen.
 */
bool fp_Page::overlapsWrappedFrame(UT_Rect & rec)
{
	UT_sint32 i=0;
	for(i=0; i<static_cast<UT_sint32>(countAboveFrameContainers());i++)
	{
		fp_FrameContainer * pFC = getNthAboveFrameContainer(i);
		if(!pFC->isWrappingSet())
		{
			continue;
		}
		if(pFC->overlapsRect(rec))
		{
		        return true;
		}
	}
	return false;
}

/*!
 * This method scans the page for the table that contains the point given
 */
fp_TableContainer * fp_Page::getContainingTable(PT_DocPosition pos)
{
	if(!m_pView)
	{
	    return NULL;
	}
	fp_CellContainer * pCell = m_pView->getCellAtPos(pos);
	if(pCell == NULL)
	{
		return NULL;
	}
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pCell->getContainer());
	if(m_pView->isInFrame(pos))
	{
		return pTab;
	}
		
	UT_sint32 i = 0;
	UT_sint32 j =0;
	bool bFound = false;
	fp_Column * pColumn = NULL;
	for(i =0; (i <static_cast<UT_sint32>(countColumnLeaders())) && !bFound; i++)
	{
		pColumn = getNthColumnLeader(i);
		while(pColumn)
		{
			for(j =0; j< pColumn->countCons() && !bFound;j++)
			{
				fp_Container * pCon = static_cast<fp_Container*>(pColumn->getNthCon(j));
				if(pCon->getContainerType() == FP_CONTAINER_TABLE)
				{
					fp_TableContainer * pCurTab = static_cast<fp_TableContainer *>(pCon);
					if(pCurTab->isThisBroken())
					{
						if(pCurTab->getMasterTable() == pTab)
						{
							bFound = true;
							return pCurTab;
						}
					}
					else
					{
						if(pCurTab == pTab)
						{
							bFound = true;
							return pCurTab;
						}
					}
				}
			}
			pColumn = pColumn->getFollower();
		}
	}
	return NULL;
}

UT_sint32 fp_Page::getAvailableHeight(void) const
{
	fl_DocSectionLayout * pDSL = getOwningSection();
	UT_sint32 avail = getHeight() - pDSL->getTopMargin() - pDSL->getBottomMargin();
	UT_sint32 i =0;
	for(i=0; i< static_cast<UT_sint32>(countFootnoteContainers()); i++)
	{
		fp_FootnoteContainer * pFC = getNthFootnoteContainer(i);
		avail -= pFC->getHeight();
	}
	if(getDocLayout()->displayAnnotations())
	{
			for(i=0; i< static_cast<UT_sint32>(countAnnotationContainers()); i++)
			{
					fp_AnnotationContainer * pAC = getNthAnnotationContainer(i);
					avail -= pAC->getHeight();
			}
	}
	return avail;
}

/*!
 * Returns true if there is a column with a page break in it
 */
bool fp_Page::containsPageBreak(void) const
{
	fp_Column * pCol = NULL;
	UT_sint32 i = 0;
	for(i=0; i<countColumnLeaders();i++)
	{
		pCol = getNthColumnLeader(0);
		while(pCol)
		{
			if(pCol->containsPageBreak())
				return true;
			pCol = pCol->getFollower();
		}
	}
	return false;
}

/*!
 * Returns the physical page number (the numbering starts from 0)
 */
UT_sint32 fp_Page::getPageNumber(void)
{
	return m_pLayout->findPage(this);
}

/*!
 * Return the page number as indicated in a page number field run.
 * This value depends on the section properties "section-restart" 
 * and "section-restart-value".
 */

UT_sint32 fp_Page::getFieldPageNumber(void) const
{
	return m_iFieldPageNumber;
}

void fp_Page::setFieldPageNumber(UT_sint32 iPageNum)
{
	m_iFieldPageNumber = iPageNum;
}

void fp_Page::resetFieldPageNumber(void)
{
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(getOwningSection());
	m_iFieldPageNumber = getPageNumber();
	if (m_iFieldPageNumber >= 0)
	{
		m_iFieldPageNumber++;
		while(pDSL && !pDSL->arePageNumbersRestarted())
		{
			pDSL =  pDSL->getPrevDocSection();
		}
		if(pDSL && pDSL->arePageNumbersRestarted())
		{
			fp_Page * pFirstPage = pDSL->getFirstOwnedPage();
			if(pFirstPage)
			{
				UT_sint32 iFirstPage = pFirstPage->getPageNumber();
				m_iFieldPageNumber += pDSL->getRestartedPageNumber() - iFirstPage - 1;
			}
		}
	}
}

/*!
 * This method returns the height available to the requested column. It 
 * subtracts the height given to previous columns on the page as well as the 
 * height given to footnotes and annotations of these columns.
*/  
UT_sint32 fp_Page::getAvailableHeightForColumn(const fp_Column * pColumn) const
{
	fp_Column * pLeader = pColumn->getLeader();
	fp_Column * pCurLeader = getNthColumnLeader(0);
	fl_DocSectionLayout * pDSL = pCurLeader->getDocSectionLayout();
	UT_sint32 avail = getHeight() - pDSL->getTopMargin() - pDSL->getBottomMargin();
	if ((countColumnLeaders() == 1) || (pCurLeader == pLeader))
	{
		return avail;
	}

	// Case with multiple sections on the page
	UT_sint32 i = 0;
	xxx_UT_DEBUGMSG(("fp_Page:: Total avail after margins subtracted %d \n",avail));
	for(i = 0; i < countColumnLeaders(); i++)
	{
		pCurLeader = getNthColumnLeader(i);
		if (pCurLeader == pLeader)
		{
			break;
		}
		UT_sint32 iMostHeight = pCurLeader->getHeight();
		fp_Column* pTmpCol = pCurLeader;
		while(pTmpCol)
		{
			iMostHeight = UT_MAX(iMostHeight, pTmpCol->getHeight());
			pTmpCol = pTmpCol->getFollower();
		}
		xxx_UT_DEBUGMSG(("fp_Page: Subtracting height %d from column %d \n",iMostHeight,i));
		avail -= iMostHeight;
	}
	UT_ASSERT(getNthColumnLeader(i) == pLeader);

	// Now subtract the footnotes and annotations on the page that are part of previous sections

	UT_sint32 iLeader = i;
	for(i=0; i< static_cast<UT_sint32>(countFootnoteContainers()); i++)
	{
		fp_FootnoteContainer * pFC = getNthFootnoteContainer(i);
		fl_DocSectionLayout * pDSLFoot = static_cast<fl_FootnoteLayout*>(pFC->getSectionLayout())->getDocSectionLayout();
		UT_sint32 k = 0;
		for (k = 0; k < iLeader; k++)
		{
			pCurLeader = getNthColumnLeader(i);
			if (pCurLeader == NULL)
				continue;
			if (pDSLFoot == pCurLeader->getDocSectionLayout())
			{
				avail -= pFC->getHeight();
				break;
			}
		}
	}
	if(getDocLayout()->displayAnnotations())
	{
		for(i=0; i< static_cast<UT_sint32>(countAnnotationContainers()); i++)
		{
			fp_AnnotationContainer * pAC = getNthAnnotationContainer(i);
			fl_DocSectionLayout * pDSLAnn = static_cast<fl_AnnotationLayout*>(pAC->getSectionLayout())->getDocSectionLayout();
			UT_sint32 k = 0;
			for (k = 0; k < iLeader; k++)
			{
				if (pDSLAnn == getNthColumnLeader(i)->getDocSectionLayout())
				{
					avail -= pAC->getHeight();
					break;
				}
			}
		}
	}

	return avail;
}

/*!
 * This method scans the page and returns the total height in layout units of all the columns
 * on it.
 * If prevLine is non-NULL the maximum column height up to this line is calculated.
 */
UT_sint32 fp_Page::getFilledHeight(fp_Container * prevContainer) const
{
	UT_sint32 totalHeight = 0;
    UT_sint32 maxHeight = 0;
	fp_Column * pColumn = NULL;
	UT_sint32 i =0;
	fp_Column * prevColumn = NULL;
	bool bstop = false;
	if(prevContainer)
	{
		prevColumn = static_cast<fp_Column *>(prevContainer->getContainer());
	}
	for(i=0; !bstop && (i<  m_vecColumnLeaders.getItemCount()); i++)
	{
		maxHeight = 0;
		pColumn = m_vecColumnLeaders.getNthItem(i);
		totalHeight += pColumn->getDocSectionLayout()->getSpaceAfter();
		while(pColumn != NULL)
		{
			if(prevColumn == pColumn)
			{
				bstop = true;
				fp_Container * pCurContainer = static_cast<fp_Container *>(pColumn->getFirstContainer());
				UT_sint32 curHeight = 0;
				while((pCurContainer != NULL) && (pCurContainer != prevContainer))
				{
					if(pCurContainer->getContainerType() == FP_CONTAINER_TABLE)
					{
						fp_TableContainer * pTC = static_cast<fp_TableContainer *>(pCurContainer);
						curHeight += pTC->getHeight();
					}
					else
					{
						curHeight += pCurContainer->getHeight();
					}
					pCurContainer = static_cast<fp_Container *>(pCurContainer->getNext());
				}
				if(pCurContainer == prevContainer)
				{
					if(pCurContainer->getContainerType() == FP_CONTAINER_TABLE)
					{
						fp_TableContainer * pTC = static_cast<fp_TableContainer *>(pCurContainer);
						curHeight += pTC->getHeight();
					}
					else
					{
						curHeight += pCurContainer->getHeight();
					}
				}
				maxHeight = UT_MAX(curHeight,maxHeight);
			}
			else
			{
				maxHeight = UT_MAX(pColumn->getHeight(),maxHeight);
			}
			pColumn = pColumn->getFollower();
		}
		totalHeight += maxHeight;
	}
	return totalHeight;
}


UT_sint32 fp_Page::getBottom(void) const
{
	int count = countColumnLeaders();
	if (count <= 0)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}

	fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
	fl_DocSectionLayout* pFirstSectionLayout = pFirstColumnLeader->getDocSectionLayout();
	UT_ASSERT(m_pOwner == pFirstSectionLayout);

//	UT_sint32 iTopMargin = pFirstSectionLayout->getTopMargin();
	UT_sint32 iBottomMargin = pFirstSectionLayout->getBottomMargin();

	return getHeight() - iBottomMargin;
}

void fp_Page::getScreenOffsets(fp_Container* pContainer, UT_sint32& xoff, UT_sint32& yoff) const
{
	if(!m_pView)
	{
	    return;
	}
	m_pView->getPageScreenOffsets(this, xoff, yoff);
	if(pContainer)
        {
	    xoff += pContainer->getX();
	    yoff += pContainer->getY();
	}
}

fp_Page* fp_Page::getNext(void) const
{
	return m_pNext;
}

fp_Page* fp_Page::getPrev(void) const
{
	return m_pPrev;
}

void fp_Page::setNext(fp_Page* p)
{
	m_pNext = p;
	xxx_UT_DEBUGMSG(("Next PAge set to %x \n",p));
//	UT_ASSERT(0);
}

void fp_Page::setPrev(fp_Page* p)
{
	m_pPrev = p;
}

FL_DocLayout* fp_Page::getDocLayout() const
{
	return m_pLayout;
}

void fp_Page::_drawCropMarks(dg_DrawArgs* pDA)
{
    if(m_pView->getShowPara()
	   && m_pView->getViewMode() == VIEW_PRINT // only draw the cropmarks if we are in the Print Layout view
	   && pDA->pG->queryProperties(GR_Graphics::DGP_SCREEN)
	   && countColumnLeaders() > 0)
	{
		GR_Painter painter(pDA->pG);

        fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
        fl_DocSectionLayout* pFirstSectionLayout = (pFirstColumnLeader->getDocSectionLayout());
		UT_ASSERT(m_pOwner == pFirstSectionLayout);

        UT_sint32 iLeftMargin = pFirstSectionLayout->getLeftMargin();
        UT_sint32 iRightMargin = pFirstSectionLayout->getRightMargin();
        UT_sint32 iTopMargin = pFirstSectionLayout->getTopMargin();
        UT_sint32 iBottomMargin = pFirstSectionLayout->getBottomMargin();

        UT_sint32 xoffStart = pDA->xoff + iLeftMargin - pDA->pG->tlu(1);
        UT_sint32 yoffStart = pDA->yoff + iTopMargin - pDA->pG->tlu(1);
        UT_sint32 xoffEnd = pDA->xoff + getWidth() - iRightMargin + pDA->pG->tlu(2);
        UT_sint32 yoffEnd = pDA->yoff + getHeight() - iBottomMargin + pDA->pG->tlu(2);

        UT_sint32 iLeftWidth = UT_MIN(iLeftMargin,pDA->pG->tlu(20));
        UT_sint32 iRightWidth = UT_MIN(iRightMargin,pDA->pG->tlu(20));
        UT_sint32 iTopHeight = UT_MIN(iTopMargin,pDA->pG->tlu(20));
        UT_sint32 iBottomHeight = UT_MIN(iBottomMargin,pDA->pG->tlu(20));

        pDA->pG->setColor(getDocLayout()->getView()->getColorShowPara());

		pDA->pG->setLineProperties(pDA->pG->tluD(1.0),
									 GR_Graphics::JOIN_MITER,
									 GR_Graphics::CAP_PROJECTING,
									 GR_Graphics::LINE_SOLID);

        painter.drawLine(xoffStart, yoffStart, xoffStart, yoffStart - iTopHeight);
        painter.drawLine(xoffStart, yoffStart, xoffStart - iLeftWidth, yoffStart);

        painter.drawLine(xoffEnd, yoffStart - iTopHeight, xoffEnd, yoffStart);
        painter.drawLine(xoffEnd, yoffStart, xoffEnd + iRightWidth, yoffStart);

        painter.drawLine(xoffStart, yoffEnd, xoffStart, yoffEnd + iBottomHeight);
        painter.drawLine(xoffStart - iLeftWidth, yoffEnd, xoffStart, yoffEnd);

        painter.drawLine(xoffEnd, yoffEnd, xoffEnd, yoffEnd + iBottomHeight);
        painter.drawLine(xoffEnd, yoffEnd, xoffEnd + iRightWidth, yoffEnd);
    }
}

void fp_Page::draw(dg_DrawArgs* pDA, bool /*bAlwaysUseWhiteBackground*/)
{
//
// Fill the Page with the page color
//
// only call this for printing and honour the option to not fill the paper with
// color.
//
	xxx_UT_DEBUGMSG(("Draw wrap passes = %d \n",m_iCountWrapPasses));
	m_iCountWrapPasses = 0;
	int i=0;
	if(!pDA->pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		getOwningSection()->getDocLayout()->incrementGraphicTick();
	}
	getOwningSection()->checkGraphicTick(pDA->pG);
	if(!pDA->pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		getOwningSection()->getDocLayout()->incrementGraphicTick();
	}
	if(!pDA->bDirtyRunsOnly)
	{
		xxx_UT_DEBUGMSG(("Doing a rectangular color fill \n"));
 		UT_sint32 xmin = pDA->xoff;
  		UT_sint32 ymin = pDA->yoff;

		UT_sint32 height =getHeight();
		UT_sint32 width = getWidth();
		UT_sint32 srcX = 0;
		UT_sint32 srcY = 0;
		getFillType().Fill(pDA->pG,srcX,srcY,xmin,ymin,width,height);
	}

    _drawCropMarks(pDA);

	UT_sint32 count = 0;

	// draw Below Frames
	count = m_vecBelowFrames.getItemCount();
	for (i=0; i<count; i++)
	{
		fp_FrameContainer* pFC = m_vecBelowFrames.getNthItem(i);
		UT_Rect r(pFC->getX(),pFC->getY(),pFC->getWidth(),pFC->getHeight());
		if(m_rDamageRect.intersectsRect(&r))
		{
			pFC->setOverWrote();
		}
		
		dg_DrawArgs da = *pDA;
		da.xoff += pFC->getX();
		da.yoff += pFC->getY();
		pFC->draw(&da);
	}
	//
	// Handle Tight wrapped frames
	//
	count = m_vecAboveFrames.getItemCount();
	for (i=0; i<count; i++)
	{
		fp_FrameContainer* pFC = m_vecAboveFrames.getNthItem(i);
		if(!pFC->isTightWrapped())
			continue;
		UT_Rect r(pFC->getX(),pFC->getY(),pFC->getWidth(),pFC->getHeight());
		if(m_rDamageRect.intersectsRect(&r))
		{
			pFC->setOverWrote();
		}
		
		dg_DrawArgs da = *pDA;
		da.xoff += pFC->getX();
		da.yoff += pFC->getY();
		pFC->draw(&da);
	}


	// draw each column on the page
	count = m_vecColumnLeaders.getItemCount();

	GR_Painter painter(pDA->pG);

	for (i=0; i<count; i++)
	{
		fp_Column* pCol = m_vecColumnLeaders.getNthItem(i);
		while (pCol)
		{
			dg_DrawArgs da = *pDA;
			xxx_UT_DEBUGMSG(("Draw in page page X offset %d \n",pDA->xoff));
			da.xoff += pCol->getX();
			da.yoff += pCol->getY(pDA->pG);

			xxx_UT_DEBUGMSG(("Draw in page col Y offset %d \n",da.yoff));
			pCol->draw(&da);

			fp_Column *pNextCol = pCol->getFollower();

			if(pNextCol && pCol->getDocSectionLayout()->getColumnLineBetween())
			{
				// draw line between columns if required.

				UT_sint32 x = pDA->xoff + (pCol->getX() + pCol->getWidth() + pNextCol->getX()) / 2;
				UT_sint32 y = pDA->yoff + pCol->getY();
				pDA->pG->setColor(m_pView->getColorColumnLine());
				painter.drawLine(x, y, x, y + pCol->getHeight());
			}

			pCol = pNextCol;
		}
	}

	// draw the page's headers and footers
    if(m_pView->getViewMode() == VIEW_PRINT  || pDA->pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
		if (m_pHeader)
		{
			dg_DrawArgs da = *pDA;
			da.xoff += m_pHeader->getX();
			da.yoff += m_pHeader->getY();
			m_pHeader->draw(&da);
		}

		if (m_pFooter)
		{
			dg_DrawArgs da = *pDA;
			da.xoff += m_pFooter->getX();
			da.yoff += m_pFooter->getY();
			m_pFooter->draw(&da);
		}
	}

	// draw footnotes
	count = m_vecFootnotes.getItemCount();
	for (i=0; i<count; i++)
	{
		fp_FootnoteContainer* pFC = m_vecFootnotes.getNthItem(i);
		dg_DrawArgs da = *pDA;
		if(m_pView && (m_pView->getViewMode() != VIEW_PRINT) &&
		   !pDA->pG->queryProperties(GR_Graphics::DGP_PAPER))
		{
			fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
			fl_DocSectionLayout* pFirstSectionLayout = (pFirstColumnLeader->getDocSectionLayout());
			da.yoff -= pFirstSectionLayout->getTopMargin();
		}
		da.xoff += pFC->getX();
		da.yoff += pFC->getY();
		pFC->draw(&da);
	}

	// draw annotations
	if(getDocLayout()->displayAnnotations())
	{
			count = m_vecAnnotations.getItemCount();
			for (i=0; i<count; i++)
			{
					fp_AnnotationContainer* pAC = m_vecAnnotations.getNthItem(i);
					dg_DrawArgs da = *pDA;
					if(m_pView && (m_pView->getViewMode() != VIEW_PRINT) &&
					   !pDA->pG->queryProperties(GR_Graphics::DGP_PAPER))
					{
							fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
							fl_DocSectionLayout* pFirstSectionLayout = (pFirstColumnLeader->getDocSectionLayout());
							da.yoff -= pFirstSectionLayout->getTopMargin();
					}
					da.xoff += pAC->getX();
					da.yoff += pAC->getY();
					pAC->draw(&da);
			}
	}

	// draw Above Frames
	count = m_vecAboveFrames.getItemCount();
	for (i=0; i<count; i++)
	{
		fp_FrameContainer* pFC = m_vecAboveFrames.getNthItem(i);
		if(pFC->isTightWrapped())
			continue;
		UT_Rect r(pFC->getX(),pFC->getY(),pFC->getWidth(),pFC->getHeight());
		if(m_rDamageRect.intersectsRect(&r))
		{
			pFC->setOverWrote();
		}
		dg_DrawArgs da = *pDA;
#if 0
		if(m_pView && (m_pView->getViewMode() != VIEW_PRINT) &&
		   !pDA->pG->queryProperties(GR_Graphics::DGP_PAPER))
		{
			fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
			fl_DocSectionLayout* pFirstSectionLayout = (pFirstColumnLeader->getDocSectionLayout());
			da.yoff -= pFirstSectionLayout->getTopMargin();
		}
#endif
		da.xoff += pFC->getX();
		da.yoff += pFC->getY();
		pFC->draw(&da);
	}

	m_bNeedsRedraw = false;
	m_rDamageRect.left = 0;
	m_rDamageRect.top = 0;
	m_rDamageRect.width = 0;
	m_rDamageRect.height = 0;

}

void   fp_Page::expandDamageRect(UT_sint32 x, UT_sint32 y, 
										 UT_sint32 width, UT_sint32 height)
{
//
// x and y and in screen offsets turn into page coords
//
	UT_sint32 xoff,yoff;
	m_pView->getPageScreenOffsets(this, xoff, yoff);
	x -= xoff;
	y -= yoff;

	if(m_rDamageRect.width == 0)
	{
		m_rDamageRect.left = x;
		m_rDamageRect.top=y;
		m_rDamageRect.width = width;
		m_rDamageRect.height = height;
		return;
	}
	UT_Rect r(x,y,width,height);
	m_rDamageRect.unionRect(&r);
	return;
}

/*!
 * Returns true if the objects intersects the damaged region
 */
bool   fp_Page::intersectsDamagedRect(fp_ContainerObject * pObj)
{
	UT_Rect * pRec = pObj->getScreenRect();
	bool bIntersects = m_rDamageRect.intersectsRect(pRec);
	delete pRec;
	return bIntersects;
}

void   fp_Page::redrawDamagedFrames(dg_DrawArgs* pDA)
{
	// draw Frames
	UT_sint32 count = m_vecAboveFrames.getItemCount();
	UT_sint32 i = 0;
	for (i=0; i<count; i++)
	{
		fp_FrameContainer* pFC = m_vecAboveFrames.getNthItem(i);
		UT_Rect r(pFC->getX(),pFC->getY(),pFC->getWidth(),pFC->getHeight());
		if(m_rDamageRect.intersectsRect(&r))
		{
			pFC->setOverWrote();
		}
		dg_DrawArgs da = *pDA;
#if 0
		if(m_pView && (m_pView->getViewMode() != VIEW_PRINT) &&
		   !pDA->pG->queryProperties(GR_Graphics::DGP_PAPER))
		{
			fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
			fl_DocSectionLayout* pFirstSectionLayout = (pFirstColumnLeader->getDocSectionLayout());
			da.yoff -= pFirstSectionLayout->getTopMargin();
		}
#endif
		da.xoff += pFC->getX();
		da.yoff += pFC->getY();
		pFC->draw(&da);
	}

	m_rDamageRect.left = 0;
	m_rDamageRect.top = 0;
	m_rDamageRect.width = 0;
	m_rDamageRect.height = 0;
}



bool fp_Page::needsRedraw(void) const
{
	return m_bNeedsRedraw;
}

UT_sint32 fp_Page::countColumnLeaders(void) const
{
	return m_vecColumnLeaders.getItemCount();
}

fp_Column* fp_Page::getNthColumnLeader(UT_sint32 n) const
{
	if(n >= m_vecColumnLeaders.getItemCount())
		return NULL;
	return m_vecColumnLeaders.getNthItem(n);
}


fp_Container* fp_Page::getNthColumn(UT_uint32 n,fl_DocSectionLayout *pSection) const
{
	fp_Column *pCol = NULL;
	UT_sint32 j = 0;
	bool b_sectionFound = false;
	
	if ((!pSection) || (n > pSection->getNumColumns()))
	{ return NULL;}
	for (j = 0;j < countColumnLeaders();j++)
	{
		pCol = getNthColumnLeader(j);
		if (pCol && (pCol->getDocSectionLayout() == pSection))
		{
			b_sectionFound = true;
			break;
		}
	}
	if (!b_sectionFound)
	{ return NULL;}

	UT_uint32 k = 0;
	while(pCol && k < n)
	{
		pCol = static_cast <fp_Column *>(pCol->getNext());
		k++;
	}
	return pCol;

}
 
/*!
 * This method is called following a notification of an increase in 
 * HdrFtr size
 */
bool fp_Page::TopBotMarginChanged(void)
{
//
// Adjust header/footer shadows first
//
	UT_sint32 iTopM = m_pOwner->getTopMargin();
	UT_sint32 iBotM = m_pOwner->getBottomMargin();
	clearScreenFrames();
	if(m_pHeader)
	{
		m_pHeader->clearScreen();
		m_pHeader->setMaxHeight(iTopM - m_pOwner->getHeaderMargin());
		m_pHeader->layout();
	}
	if(m_pFooter)
	{
		m_pFooter->clearScreen();
		m_pFooter->setMaxHeight(iBotM - m_pOwner->getFooterMargin());
		m_pFooter->setY(getHeight() - iBotM);
		m_pFooter->layout();
	}
	breakPage();
	_reformat();
	return true;
}
/*!
 * This method scans the current page to make sure that there is space to at least start
 * every column on the page. Columns with space are deleted and their contents redistributed.
 */
bool fp_Page::breakPage(void)
{
	UT_sint32 count = countColumnLeaders();
	if (count == 0)
	{
		return true;
	}
	UT_sint32 iYPrev = 0;
	fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
	fl_DocSectionLayout* pFirstSectionLayout = (pFirstColumnLeader->getDocSectionLayout());
	UT_ASSERT(m_pOwner == pFirstSectionLayout);
	UT_sint32 iTopMargin = pFirstSectionLayout->getTopMargin();
	UT_sint32 iBottomMargin = pFirstSectionLayout->getBottomMargin();
	UT_sint32 iY = iTopMargin;
	UT_sint32 availHeight = getHeight() - iBottomMargin;
		
	// we need the height of the footnotes on this page, to deduct.
	UT_sint32 i = 0;
	UT_uint32 iFootnoteHeight = 2*pFirstSectionLayout->getFootnoteLineThickness();
	for (i = 0; i < countFootnoteContainers(); i++)
	{
		iFootnoteHeight += getNthFootnoteContainer(i)->getHeight();
	}
	iY += iFootnoteHeight;

		
	// we need the height of the annotations on this page, to deduct.
	if(getDocLayout()->displayAnnotations())
	{
		    UT_sint32 iAnnotationHeight = 0;
			for (i = 0; i < countAnnotationContainers(); i++)
			{
					iAnnotationHeight += getNthAnnotationContainer(i)->getHeight();
			}
			iY += iAnnotationHeight;
	}

	for (i=0; i<count; i++)
	{
		fp_Column* pLeader = getNthColumnLeader(i);
		fp_Column* pTmpCol = pLeader;
		UT_sint32 iMostHeight = 0;
		iYPrev = iY;
		while (pTmpCol)
		{
			iMostHeight = UT_MAX(iMostHeight, pTmpCol->getHeight());
			pTmpCol = pTmpCol->getFollower();
		}

		iY += iMostHeight;
		iY += pLeader->getDocSectionLayout()->getSpaceAfter();
		iY += pLeader->getDocSectionLayout()->getSpaceAfter();
		if (iY >= availHeight)
		{
			break;
		}
	}
	if(i < count)
	{
		i++;
	}
	if(i == count)
	{
//
// Clear out 1 line columns at the bottom of the page. If the row of columns
// at the bottom of the page contains a maxium of one line and if adding an
// extra line makes the column bigger than the page size, remove this column
// from the page.
//
		i--;
		if(i < 1)
		{
			return true;
		}
		fp_Column * pPrev = getNthColumnLeader(i);
		UT_sint32 maxContainers= 0;
		UT_sint32 maxContainerHeight = 0;
		fp_Column * pCol = pPrev;
		if (pCol && pCol->getFirstContainer() &&
			(pCol->getFirstContainer()->getContainerType() == FP_CONTAINER_LINE))
		{
			fp_Line * pLine = static_cast <fp_Line*>(pCol->getFirstContainer());
			if (pLine->getFirstRun() &&
				(pLine->getFirstRun()->getType() == FPRUN_FORCEDPAGEBREAK))
			{
				return true;
			}
		}

		while(pCol != NULL)
		{
			UT_sint32 countContainers = 0;
			fp_Container * pContainer = static_cast<fp_Container *>(pCol->getFirstContainer());
			while(pContainer != NULL && pContainer != static_cast<fp_Container *>(pCol->getLastContainer()))
			{
				countContainers++;
				if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
				{
					fp_TableContainer * pTC = static_cast<fp_TableContainer *>(pContainer);
					maxContainerHeight = UT_MAX(maxContainerHeight,pTC->getHeight());
				}
				else
				{
					maxContainerHeight = UT_MAX(maxContainerHeight,pContainer->getHeight());
				}
				pContainer = static_cast<fp_Container *>(pContainer->getNext());			}
			if(pContainer != NULL)
			{
				if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
				{
					fp_TableContainer * pTC = static_cast<fp_TableContainer *>(pContainer);
					maxContainerHeight = UT_MAX(maxContainerHeight,pTC->getHeight());
				}
				else
				{
					maxContainerHeight = UT_MAX(maxContainerHeight,pContainer->getHeight());
				}
				countContainers++;
			}
			maxContainers = UT_MAX(maxContainers,countContainers);
			pCol = pCol->getFollower();
		}
		if(maxContainers > 1)
		{
			return true;
		}
//
//OK this is a candidate to clear off this page. Next test, is the column over
//80% of the way down the page?
//
		double rat = static_cast<double>(iYPrev) / static_cast<double>(availHeight);
		if(rat < 0.80)
			return true;
//
// Finally if iYPrev plus 2* prev line height is greater than or equal to the
// total height, remove the container.
//
		if((iYPrev + 2*maxContainerHeight) < availHeight)
		{
//
// OK we want to delete this column if the docsection of the
// previous column continues onto the next or even subsequent pages.
//
			fp_Page * pPNext = getNext();
			fl_DocSectionLayout * pPrevDSL = getNthColumnLeader(i-1)->getDocSectionLayout();
			if(pPNext== NULL)
			{
				return true;
			}
			if(pPrevDSL == pPrev->getDocSectionLayout())
			{
				return true;
			}
			if(pPNext->countColumnLeaders() == 0)
			{
				return true;
			}
			fp_Column * pCNext = pPNext->getNthColumnLeader(0);
			if(pCNext == NULL)
			{
				return true;
			}
			fl_DocSectionLayout * pNextDSL = pCNext->getDocSectionLayout();
			if(pNextDSL != pPrevDSL)
			{
				return true;
			}
		}
	}
	return false;
}

#if 0
/*!
 * Return true if a column on the page has a docsectionlayout as given.
 */
bool fp_Page::isDSLOnPage(fl_DocSectionLayout * pDSLToFind)
{
	int count = countColumnLeaders();
	if (count <= 0)
	{
		return;
	}
	UT_uint32 i = 0;
	for(i=0; i< count)
	{
		fl_DocSectionLayout * pDSL = getNthColumnLeader(i)->getDocSectionLayout();
		if(pDSL ==  pDSLToFind)
		{
			return true;
		}
	}
	return false;
}

/*!
 * Return the column previous to this one
 */
fp_Column * fp_Page::getPrevColOnPages(fp_Column * pCol, fp_Page * pPage)
{
	UT_sint32 count = pPage->countColumnLeaders();
	UT_sint32 i=0;
	fp_Column * pFound = NULL;
	for(i=0; i< count: i++)
	{
		pFound = static_cast<fp_Column *>(pPage->getNthColumn(i));
		if(pFound == pCol)
		{
			break;
		}
	}
	if( i == count)
	{
		return NULL;
	}
	if(i>0)
	{
		pFound = pPage->getNthColumn(i-1);
		return pFound;
	}
	else
	{
		fp_Page * pPrev = pPage->getPrev();
		if(pPrev == NULL)
		{
			return NULL;
		}
		count = pPage->countColumnLeaders();
		if(count <1 )
		{
			return NULL;
		}
	    else
		{
			pFound = pPrev->getNthColumn(count-1);
		}
	}
}
#endif

/*!
    This function updates the x-offset of all columns without changing any of their
    dimensions (it is used when changing the view mode)
 */
void fp_Page::updateColumnX()
{
	UT_uint32 count = countColumnLeaders();
	if (count == 0)
		return;

	UT_ASSERT(m_pOwner == getNthColumnLeader(0)->getDocSectionLayout());


	UT_sint32 iLeftMargin = 0;
	UT_sint32 iRightMargin = 0;

	for (UT_uint32 i = 0; i < count; i++)
	{
		fp_Column* pLeader = getNthColumnLeader(i);
		UT_ASSERT(pLeader->getContainerType() == FP_CONTAINER_COLUMN);
		fl_DocSectionLayout* pSL = (pLeader->getDocSectionLayout());

		if((m_pView->getViewMode() == VIEW_NORMAL || m_pView->getViewMode() == VIEW_WEB) &&
		   !m_pLayout->getGraphics()->queryProperties(GR_Graphics::DGP_PAPER))
		{
			iLeftMargin = m_pView->getNormalModeXOffset();
			iRightMargin = 0;
		}
		else
		{
			iLeftMargin = pSL->getLeftMargin();
			iRightMargin = pSL->getRightMargin();
		}
		
		UT_uint32 iSpace = getWidth() - iLeftMargin - iRightMargin;
		pSL->checkAndAdjustColumnGap(iSpace);

		UT_uint32 iNumColumns = pSL->getNumColumns();
		UT_uint32 iColumnGap = pSL->getColumnGap();
		UT_uint32 iColWidth = (iSpace - ((iNumColumns - 1) * iColumnGap)) / iNumColumns;

		UT_sint32 iX;
		if(pSL->getColumnOrder())
		{
			iX = getWidth() - iRightMargin - iColWidth;
		}
		else
		{
			iX = iLeftMargin;
		}

		fp_Column* pTmpCol = pLeader;

		while (pTmpCol)
		{
			UT_ASSERT(pTmpCol->getContainerType() == FP_CONTAINER_COLUMN);
			pTmpCol->setX(iX);

			if(pSL->getColumnOrder())
			{
				iX -= (iColWidth + iColumnGap);
			}
			else
			{
				iX += (iColWidth + iColumnGap);
			}

			pTmpCol = pTmpCol->getFollower();
		}
	}
}


void fp_Page::_reformat(void)
{
	// this is naive, because columns can cause the footnotes
	// to change pages.  But it'll do for now.
	_reformatColumns(); 
	_reformatFootnotes();
	_reformatAnnotations();
}

void fp_Page::_reformatColumns(void)
{
	UT_sint32 count = countColumnLeaders();
	if (count == 0)
		return;

	fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
	fp_Column * pLastCol = NULL;
	fl_DocSectionLayout* pFirstSectionLayout = (pFirstColumnLeader->getDocSectionLayout());
	UT_ASSERT(m_pOwner == pFirstSectionLayout);


	UT_sint32 iLeftMargin = 0;
	UT_sint32 iRightMargin = 0;
	UT_sint32 iLeftMarginReal = 0;
	UT_sint32 iRightMarginReal = 0;
	UT_sint32 iTopMargin = pFirstSectionLayout->getTopMargin();
	UT_sint32 iBottomMargin = pFirstSectionLayout->getBottomMargin();
	UT_sint32 iY = iTopMargin;

	// we need the height of the footnotes on this page, to deduct.
	UT_sint32 i = 0;
	UT_uint32 iFootnoteHeight = 2*pFirstSectionLayout->getFootnoteLineThickness();
	for (i = 0; i < countFootnoteContainers(); i++)
	{
		iFootnoteHeight += getNthFootnoteContainer(i)->getHeight();
	}
	UT_uint32 iAnnotationHeight = getAnnotationHeight();
	for (i = 0; i < count; i++)
	{
#if 0
		if (iY >= (static_cast<UT_sint32>(getHeight() - iBottomMargin - iFootnoteHeight -AnnotationHeight)))
		{
			xxx_UT_DEBUGMSG(("SEVIOR: Page incorrectly laid out iYlayoutuints= %d  \n",iY));
//			m_pOwner->markForRebuild();
//
// FIXME see if this code works instead
//
//		m_pOwner->setNeedsSectionBreak(true,getPrev());
			// this triggers in docs with lot of footnotes, not sure why it is here, Tomas
			// UT_ASSERT(0); 
			return;
//			break;
		}
#endif

		fp_Column* pLeader = getNthColumnLeader(i);
		UT_ASSERT(pLeader->getContainerType() == FP_CONTAINER_COLUMN);
		fl_DocSectionLayout* pSL = (pLeader->getDocSectionLayout());

		if((m_pView->getViewMode() == VIEW_NORMAL || m_pView->getViewMode() == VIEW_WEB) &&
		   !m_pLayout->getGraphics()->queryProperties(GR_Graphics::DGP_PAPER))
		{
			iLeftMargin = m_pView->getNormalModeXOffset();
			iRightMargin = 0;
		}
		else
		{
			iLeftMargin = pSL->getLeftMargin();
			iRightMargin = pSL->getRightMargin();
		}
		
		iLeftMarginReal = pSL->getLeftMargin();
		iRightMarginReal = pSL->getRightMargin();

		UT_uint32 iSpace = getWidth() - iLeftMarginReal - iRightMarginReal;
		pSL->checkAndAdjustColumnGap(iSpace);

		UT_uint32 iNumColumns = pSL->getNumColumns();
		UT_uint32 iColumnGap = pSL->getColumnGap();
		UT_uint32 iColWidth = (iSpace - ((iNumColumns - 1) * iColumnGap)) / iNumColumns;

		UT_sint32 iX;
		if(pSL->getColumnOrder())
		{
			iX = getWidth() - iRightMargin - iColWidth;
		}
		else
		{
			iX = iLeftMargin;
		}

		fp_Column* pTmpCol = pLeader;
		UT_sint32 iMostHeight = 0;

		while (pTmpCol)
		{
			UT_ASSERT(pTmpCol->getContainerType() == FP_CONTAINER_COLUMN);
			pTmpCol->setX(iX);
			pTmpCol->setY(iY);
			pTmpCol->setMaxHeight(getHeight() - iBottomMargin - iY - iFootnoteHeight - iAnnotationHeight);
			pTmpCol->setWidth(iColWidth);

			if(pSL->getColumnOrder())
			{
				iX -= (iColWidth + iColumnGap);
			}
			else
			{
				iX += (iColWidth + iColumnGap);
			}

			iMostHeight = UT_MAX(iMostHeight, pTmpCol->getHeight());
			pLastCol = pTmpCol;
			pTmpCol = pTmpCol->getFollower();
		}

		iY += iMostHeight;
		iY += pLeader->getDocSectionLayout()->getSpaceAfter();

	}
//	UT_ASSERT(i == count);
//
// Look for blank space to put more text
//
	fp_Column * pFirstOfNext = NULL;
	fp_Page * pNext = getNext();
	if(pNext && pLastCol)
	{
		fp_Container * pLastContainer = static_cast<fp_Container *>(pLastCol->getLastContainer());
		if(pLastContainer)
		{
			if(pLastContainer->getContainerType() == FP_CONTAINER_LINE
			   && static_cast<fp_Line *>(pLastContainer)->containsForcedPageBreak())
			{
				return;
			}
			pFirstOfNext = pNext->getNthColumnLeader(0);
			if(!pFirstOfNext)
			{
				return;
			}
			fp_Container *pFirstNextContainer = static_cast<fp_Container *>(pFirstOfNext->getFirstContainer());
			if(pFirstNextContainer == NULL)
			{
				return;
			}
			UT_sint32 iYNext = pFirstNextContainer->getHeight();
			bool bIsTable = (pFirstNextContainer->getContainerType() == FP_CONTAINER_TABLE)  || (countFootnoteContainers() > 0) || (pNext->countFootnoteContainers() > 0);
			fl_ContainerLayout * pCNext = static_cast<fl_SectionLayout *>(pFirstNextContainer->getSectionLayout());
			if(pCNext == static_cast<fl_ContainerLayout *>(pLastContainer->getSectionLayout()))
		        {
			  bIsTable = true; // only rebuild if we have a change
			                   // of docsection
			}
			if( !bIsTable && (iY + 3*iYNext) < (getHeight() - getFootnoteHeight() - iBottomMargin))
			{
			  UT_DEBUGMSG(("Extra space on page. Mark for rebuild \n"));
			  //m_pOwner->markForRebuild();
//
// FIXME see if this code works instead
//
//		m_pOwner->setNeedsSectionBreak(true,getPrev());
//		UT_ASSERT(0);

			}
//
// OK now look to see if there are some endnote that should really be on this 
// page
//
#if 0
			UT_sint32 iRemainingSpace = getHeight() - getFootnoteHeight() - iBottomMargin;
			if(pFirstNextContainer  && (pFirstNextContainer->getContainerType() == FP_CONTAINER_ENDNOTE) && (iYNext < iRemainingSpace))
			{
				while(pFirstNextContainer  && (pFirstNextContainer->getContainerType() == FP_CONTAINER_ENDNOTE))
				{
					fl_EndnoteLayout * pECL = static_cast<fl_EndnoteLayout *>(pFirstNextContainer->getSectionLayout());
					pFirstNextContainer = pFirstNextContainer->getNextContainerInSection();
//
// Remove old Container from the next page
//
					pECL->collapse();
				}
			}
#endif				
		}
	}
	return;
}

void fp_Page::clearScreenFootnotes(void)
{
	UT_sint32 i =0;
	for (i = 0; i < countFootnoteContainers(); i++)
	{
		getNthFootnoteContainer(i)->clearScreen();
	}
}

UT_sint32 fp_Page::getFootnoteHeight(void) const
{
	UT_uint32 iFootnoteHeight = 0;
	UT_sint32 i = 0;
	for (i = 0; i < countFootnoteContainers(); i++)
	{
		iFootnoteHeight += getNthFootnoteContainer(i)->getHeight();
	}
	return iFootnoteHeight;
}

void fp_Page::_reformatFootnotes(void)
{
	if(m_vecColumnLeaders.getItemCount() == 0)
	{
//
// Page is being deleted.
//
		return;
	}
	fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
	fl_DocSectionLayout* pFirstSectionLayout = (pFirstColumnLeader->getDocSectionLayout());
	UT_ASSERT(m_pOwner == pFirstSectionLayout);
	UT_sint32 iBottomMargin = pFirstSectionLayout->getBottomMargin();
	UT_uint32 pageHeight = getHeight() - iBottomMargin;
	pageHeight -= getAnnotationHeight();
	UT_uint32 iFootnoteHeight = 0;
	UT_sint32 i = 0;
	for (i = 0; i < countFootnoteContainers(); i++)
	{
		iFootnoteHeight += getNthFootnoteContainer(i)->getHeight();
	}

	pageHeight -= iFootnoteHeight;
	xxx_UT_DEBUGMSG(("got page height %d, footnote height %d\n", pageHeight, iFootnoteHeight));
	for (i = 0; i < countFootnoteContainers(); i++)
	{
		fp_FootnoteContainer * pFC = getNthFootnoteContainer(i);
		fl_DocSectionLayout* pSL = (getNthColumnLeader(0)->getDocSectionLayout());

		if((m_pView->getViewMode() == VIEW_NORMAL || m_pView->getViewMode() == VIEW_WEB) &&
		   !m_pLayout->getGraphics()->queryProperties(GR_Graphics::DGP_PAPER))
			pFC->setX(m_pView->getTabToggleAreaWidth());
		else
			pFC->setX(pSL->getLeftMargin());

		pFC->setY(pageHeight);
		pageHeight += getNthFootnoteContainer(i)->getHeight();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Annotations
//////////////////////////////////////////////////////////////////////////////

void fp_Page::clearScreenAnnotations(void)
{
	UT_sint32 i =0;
	for (i = 0; i < countAnnotationContainers(); i++)
	{
		getNthAnnotationContainer(i)->clearScreen();
	}
}

UT_sint32 fp_Page::getAnnotationHeight(void) const
{
	if(!getDocLayout()->displayAnnotations())
	{
			return 0;
	}
	UT_uint32 iAnnotationHeight = 0;
	UT_sint32 i = 0;
	for (i = 0; i < countAnnotationContainers(); i++)
	{
		iAnnotationHeight += getNthAnnotationContainer(i)->getHeight();
	}
	return iAnnotationHeight;
}

void fp_Page::_reformatAnnotations(void)
{
	if(m_vecColumnLeaders.getItemCount() == 0)
	{
//
// Page is being deleted.
//
		return;
	}
	if(!getDocLayout()->displayAnnotations())
		return;
	fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
	fl_DocSectionLayout* pFirstSectionLayout = (pFirstColumnLeader->getDocSectionLayout());
	UT_ASSERT(m_pOwner == pFirstSectionLayout);
	UT_sint32 iBottomMargin = pFirstSectionLayout->getBottomMargin();
	UT_uint32 pageHeight = getHeight() - iBottomMargin;
	UT_uint32 iAnnotationHeight = 0;
	UT_sint32 i = 0;
	for (i = 0; i < countAnnotationContainers(); i++)
	{
		iAnnotationHeight += getNthAnnotationContainer(i)->getHeight();
	}

	pageHeight -= iAnnotationHeight;
	xxx_UT_DEBUGMSG(("got page height %d, footnote height %d\n", pageHeight, iAnnotationHeight));
	for (i = 0; i < countAnnotationContainers(); i++)
	{
		fp_AnnotationContainer * pAC = getNthAnnotationContainer(i);
		fl_DocSectionLayout* pSL = (getNthColumnLeader(0)->getDocSectionLayout());

		if((m_pView->getViewMode() == VIEW_NORMAL || m_pView->getViewMode() == VIEW_WEB) &&
		   !m_pLayout->getGraphics()->queryProperties(GR_Graphics::DGP_PAPER))
			pAC->setX(m_pView->getTabToggleAreaWidth());
		else
			pAC->setX(pSL->getLeftMargin());

		pAC->setY(pageHeight);
		pageHeight += getNthAnnotationContainer(i)->getHeight();
	}
}

/*!
  Remove column leader from page
  \param pLeader Leader to remove

  This will set the page of all columns in the row to NULL
*/
void fp_Page::removeColumnLeader(fp_Column* pLeader)
{
	UT_sint32 ndx = m_vecColumnLeaders.findItem(pLeader);
	UT_ASSERT(ndx >= 0);

	// Delete leader from list
	m_vecColumnLeaders.deleteNthItem(ndx);

	// Urgh! Changes to the document (logical content) cause graphics
	// updates at various times when the physical representation is
	// still in flux, resulting in both crap performance and code to
	// break due to broken assumptions.  This is a point in case where
	// the current page may get asked to render even though it
	// actually doesn't contain any columns (see bug 1385). So we have
	// to leave the pointer here, even if the page doesn't actually
	// have an owner at this time...
#if 0
	// Deassociate this page from the old owner
	m_pOwner->deleteOwnedPage(this);
	m_pOwner = NULL;
#endif

	// The row of columns are not on this page anymore
	fp_Column* pTmpCol = pLeader;
	while (pTmpCol)
	{
		pTmpCol->setPage(NULL);
		pTmpCol = pTmpCol->getFollower();
	}

	// Are there still any rows on this page?
	int count = countColumnLeaders();
	if (0 == count)
	{
		return;
	}
	// Update owner and reformat
	fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
//
// Handle change of Page ownership. This can happen when the the previous
// Section expands it's text onto a page owned by the next docsection.
//
// An alternative is to destroy and recreate the page but this will be much nicer
// if it can be made to work.
//
	if(pFirstColumnLeader->getDocSectionLayout() != m_pOwner)
	{
//
// Change ownership of the page. First remove this page from the set owned by
// the old docSectionLayout.
//
		UT_DEBUGMSG(("fp_Page: Remove page %p from DSL %p \n",this,m_pOwner)); 
		m_pOwner->deleteOwnedPage(this,false);
		fl_DocSectionLayout * pDSLNew = pFirstColumnLeader->getDocSectionLayout();
//
// Now add it to the new DSL.
//
		pDSLNew->addOwnedPage(this);
		m_pOwner = pDSLNew;
	}
	_reformatColumns();
}

/*!
  Insert column leader on page
  \param pLeader Leader to insert
  \param pAfter The leader to insert after or NULL
  \return True

  This will set the page of all columns in the row to this page.
*/
bool fp_Page::insertColumnLeader(fp_Column* pLeader, fp_Column* pAfter)
{
	UT_ASSERT(pLeader);
	if (pAfter)
	{
		UT_sint32 ndx = m_vecColumnLeaders.findItem(pAfter);
		UT_ASSERT(ndx >= 0);
		m_vecColumnLeaders.insertItemAt(pLeader, ndx+1);
	}
	else
	{
		m_vecColumnLeaders.insertItemAt(pLeader, 0);

		// Update owner and reformat

//
// Handle change of Page ownership. This can happen when the the previous
// Section expands it's text onto a page owned by the next docsection.
//
// An alternative is to destroy and recreate the page but this will be much nicer
// if it can be made to work.
//
		if(pLeader->getDocSectionLayout() != m_pOwner)
		{
//
// Change ownership of the page. First remove this page from the set owned by
// the old docSectionLayout.
//
			xxx_UT_DEBUGMSG(("SEVIOR: Deleting owned Page from Page \n"));
			if(m_pOwner)
				m_pOwner->deleteOwnedPage(this,false);
			fl_DocSectionLayout * pDSLNew = pLeader->getDocSectionLayout();
//
// Now add it to the new DSL.
//
			pDSLNew->addOwnedPage(this);
			m_pOwner = pDSLNew;
		}
	}

	fp_Column* pTmpCol = pLeader;
	while (pTmpCol)
	{
		pTmpCol->setPage(this);

		pTmpCol = pTmpCol->getFollower();
	}

	_reformat();

	return true;
}


void fp_Page::footnoteHeightChanged(void)
{
	clearScreenFootnotes();
	m_pOwner->setNeedsSectionBreak(true,getPrev());
	if(breakPage())
	{
		_reformat();
	}
	else
	{
		UT_DEBUGMSG(("SEVIOR: Mark for rebuild from footnoteheight changed. \n"));
		m_pOwner->markForRebuild();
//
// FIXME see if this code works instead
//
//		m_pOwner->setNeedsSectionBreak(true,getPrev());
		UT_ASSERT(0);
	}
}



void fp_Page::annotationHeightChanged(void)
{
	clearScreenAnnotations();
	m_pOwner->setNeedsSectionBreak(true,getPrev());
	if(breakPage())
	{
		_reformat();
	}
	else
	{
		UT_DEBUGMSG(("SEVIOR: Mark for rebuild from annotationheight changed. \n"));
		m_pOwner->markForRebuild();
//
// FIXME see if this code works instead
//
//		m_pOwner->setNeedsSectionBreak(true,getPrev());
		UT_ASSERT(0);
	}
}

void fp_Page::columnHeightChanged(fp_Column* pCol)
{
	xxx_UT_DEBUGMSG(("SEVIOR: Column height changed \n"));
	UT_UNUSED(pCol);
	UT_ASSERT(m_vecColumnLeaders.findItem(pCol->getLeader()) >= 0);
	if(breakPage())
	{
		_reformat();
	}
	else
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Mark for rebuild from columnheight changed. \n"));
		m_pOwner->markForRebuild();
//
// FIXME see if this code works instead
//
//		m_pOwner->setNeedsSectionBreak(true,getPrev());
		UT_ASSERT(0);
	}
}

PT_DocPosition fp_Page::getFirstLastPos(bool bFirst) const
{
	PT_DocPosition pos;

	UT_sint32 cols = countColumnLeaders();
	UT_ASSERT(cols>0);

	if (bFirst)
	{
		fp_Column* pColumn = getNthColumnLeader(0);
		UT_return_val_if_fail(pColumn, 2);
		fp_Container* pFirstContainer = static_cast<fp_Container *>(pColumn->getFirstContainer());
		while(pFirstContainer && pFirstContainer->getContainerType() != FP_CONTAINER_LINE)
		{
			if(pFirstContainer->getContainerType() == FP_CONTAINER_TABLE)
			{
				fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pFirstContainer);
				pFirstContainer = static_cast<fp_Container *>(pTab->getFirstLineInColumn(pColumn));
			}
			else
			{
				pFirstContainer = static_cast<fp_Container *>(pFirstContainer->getNthCon(0));
			}
		}

		UT_return_val_if_fail(pFirstContainer, 2);
		
		fp_Run* pFirstRun = static_cast<fp_Line *>(pFirstContainer)->getFirstRun();
		fl_BlockLayout* pFirstBlock = static_cast<fp_Line *>(pFirstContainer)->getBlock(); // SEVIOR This needs fix me, FIXME

		pos = pFirstRun->getBlockOffset() + pFirstBlock->getPosition();
	}
	else
	{
		fp_Column* pColumn = getNthColumnLeader(cols-1);
		UT_return_val_if_fail(pColumn, 2);
		fp_Container* pLastContainer = static_cast<fp_Container *>(pColumn->getLastContainer());
		UT_return_val_if_fail(pLastContainer, 2);
		while(pLastContainer && pLastContainer->getContainerType() != FP_CONTAINER_LINE)
		{
			if(pLastContainer->getContainerType() == FP_CONTAINER_TABLE)
			{
				fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pLastContainer);
				pLastContainer = static_cast<fp_Container *>(pTab->getLastLineInColumn(pColumn));
			}
			else
			{
				pLastContainer = static_cast<fp_Container *>(pLastContainer->getNthCon(0));
			}
		}

		UT_return_val_if_fail(pLastContainer, 2);
		
		fp_Run* pLastRun = static_cast<fp_Line *>(pLastContainer)->getLastRun();
		fl_BlockLayout* pLastBlock = static_cast<fp_Line *>(pLastContainer)->getBlock();
		UT_return_val_if_fail(pLastRun && pLastBlock, 2);

		while (pLastRun && !pLastRun->isFirstRunOnLine() && pLastRun->isForcedBreak())
		{
			pLastRun = pLastRun->getPrevRun();
		}
		UT_return_val_if_fail(pLastRun, 2);

		if(pLastRun->isForcedBreak())
		{
			pos = pLastBlock->getPosition() + pLastRun->getBlockOffset();
		}
		else
		{
			pos = pLastBlock->getPosition() + pLastRun->getBlockOffset() + pLastRun->getLength();
		}
	}

	return pos;
}

void fp_Page::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, bool& bBOL, bool& bEOL,bool &isTOC, bool bUseHdrFtr, fl_HdrFtrShadow ** pShadow ) const
{
	fl_HdrFtrShadow * pShad = NULL;
	if(pShadow == NULL)
	{
		mapXYToPosition(false,x,y,pos,bBOL,bEOL,isTOC, bUseHdrFtr, NULL);
		return;
	}
	else
	{
		mapXYToPosition(false,x,y,pos,bBOL,bEOL,isTOC, bUseHdrFtr, &pShad);
	}
	*pShadow = pShad;
}
/*!
 * This method maps an x,y location on the page to the position in the
 * document of the corrsponding element.
 * This variation looks in the header/footer region and returns the
 * SectionLayout shadow of the
 \param bNotFrames if true don't look inside frames
 \param x coordinate
 \param y coordinate
 \param bBOL
 \param bEOL
 \return pos The Document position corresponding the text at location x,y
 \return pShadow A pointer to the shadow corresponding to this header/footer
 */
void fp_Page::mapXYToPosition(bool bNotFrames,UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool &isTOC, bool bUseHdrFtr, fl_HdrFtrShadow ** pShadow ) const
{
	UT_sint32 count = m_vecColumnLeaders.getItemCount();
	UT_uint32 iMinDist = 0xffffffff;
	fp_VerticalContainer * pMinDist = NULL;
	fp_Column* pColumn = NULL;
	UT_uint32 iMinXDist = 0xffffffff;
	fp_VerticalContainer* pMinXDist = NULL;
	UT_uint32 iDist = 0;
	fp_Column* pLeader = NULL;
//
// Start by looking in Frames for this point.
//
	UT_sint32 i =0;
	fp_FrameContainer * pFrameC = NULL;
	if(!bNotFrames)
	{
//
// The iextra distance gives the space around the text box frame inside the
// frame where the user can select the frame rather than the text inside the
// frame. Without this distance clicking inside a text box would always place 
// the caret in text rather than selecting the frame to drag/resize etc.
//
		UT_sint32 iextra = m_pLayout->getGraphics()->tlu(4);
		// loop from high z to low z
		// Because we draw from old to new, the new appears on top
		// and because the new appears on top, it should be treated as such
		for (i = (countAboveFrameContainers()-1); i>=0; i--)
		{
			pFrameC = getNthAboveFrameContainer(i);
			bool isImage = false;
			fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pFrameC->getSectionLayout());
			if(pFL->getFrameType() >= FL_FRAME_WRAPPER_IMAGE)
			{
				isImage = true;
			}
			if ((pFrameC->getFirstContainer()) || isImage )
			{
				if ((x >= (pFrameC->getFullX()- iextra))
					&& (x < (pFrameC->getFullX() + pFrameC->getFullWidth()+iextra))
					&& (y >= (pFrameC->getFullY() - iextra))
					&& (y < (pFrameC->getFullY() + pFrameC->getFullHeight() + iextra))
					)
				{
					if(isImage)
					{
						pos = pFL->getPosition(true);
						return;
					}
					pFrameC->mapXYToPosition(x - pFrameC->getX(), y - pFrameC->getY(), pos, bBOL, bEOL,isTOC);
					return;
				}
				
				iDist = pFrameC->distanceFromPoint(x, y);
//
// The tlu(3) makes the distance of the mouse to the sensitive edge of the
// text box 3 pixels. ie Move the mouse within 3 pixels of the textbox and it
// change to show you can select the text box.
//
// If we're outside this distance make sure all other options are excluded
// before placing the point inside the text box
//

				if(static_cast<UT_sint32>(iDist) > m_pLayout->getGraphics()->tlu(3))
				{
					iDist += 200000;
				}
				if (iDist < iMinDist)
				{
					iMinDist = iDist;
					pMinDist = static_cast<fp_VerticalContainer *>(pFrameC);
				}
				
				if ( (y >= pFrameC->getY())
					 && (y < (pFrameC->getY() + pFrameC->getHeight()))) 
				{
					if (iDist < iMinXDist)
					{
						iMinXDist = iDist;
						pMinXDist = static_cast<fp_VerticalContainer *>(pFrameC);
					}
				}
			}
		}
		for (i = countBelowFrameContainers()-1; i>=0; i--)
		{
			pFrameC = getNthBelowFrameContainer(i);
			bool isImage = false;
			fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pFrameC->getSectionLayout());
			if(pFL->getFrameType() >= FL_FRAME_WRAPPER_IMAGE)
			{
				isImage = true;
			}
			if ((pFrameC->getFirstContainer()) || isImage )
			{
				if ((x >= (pFrameC->getFullX()- iextra))
					&& (x < (pFrameC->getFullX() + pFrameC->getFullWidth()+iextra))
					&& (y >= (pFrameC->getFullY() - iextra))
					&& (y < (pFrameC->getFullY() + pFrameC->getFullHeight() + iextra))
					)
				{
					if(isImage)
					{
						pos = pFL->getPosition(true);
						return;
					}
					pFrameC->mapXYToPosition(x - pFrameC->getX(), y - pFrameC->getY(), pos, bBOL, bEOL,isTOC);
					return;
				}
				
				iDist = pFrameC->distanceFromPoint(x, y);
//
// The tlu(3) makes the distance of the mouse to the sensitive edge of the
// text box 3 pixels. ie Move the mouse within 3 pixels of the textbox and it
// change to show you can select the text box.
//
// If we're outside this distance make sure all other options are excluded
// before placing hte point inside th text box
//

				if(static_cast<UT_sint32>(iDist) > m_pLayout->getGraphics()->tlu(3))
				{
					iDist += 200000;
				}
				if (iDist < iMinDist)
				{
					iMinDist = iDist;
					pMinDist = static_cast<fp_VerticalContainer *>(pFrameC);
				}
				
				if ( (y >= pFrameC->getY())
					 && (y < (pFrameC->getY() + pFrameC->getHeight()))) 
				{
					if (iDist < iMinXDist)
					{
						iMinXDist = iDist;
						pMinXDist = static_cast<fp_VerticalContainer *>(pFrameC);
					}
				}
			}
		}

	}
//
// Look in header for insertion point
//
	if (bUseHdrFtr)
	{
		if (pShadow)
			*pShadow = NULL;
		if(m_pView && m_pView->getViewMode() == VIEW_PRINT)
		{
			fp_ShadowContainer * hf[2] = { m_pHeader, m_pFooter };
			for (UT_uint32 j = 0; j < G_N_ELEMENTS(hf); j++)
			{
				fp_ShadowContainer * p = hf[j];

				if(p == NULL || !p->getFirstContainer())
					continue;

				if ((y >= p->getY()) && (y < (p->getY() + p->getHeight())))
				{
					p->mapXYToPosition(x - p->getX(), y - p->getY(), pos, bBOL, bEOL,isTOC);
					if (pShadow)
						*pShadow = p->getShadow();
					return;
				}
			}
		}
	}

//
// Now look in page
//
	for (i=0; i<count; i++)
	{
		pLeader = m_vecColumnLeaders.getNthItem(i);

		pColumn = pLeader;
		iMinXDist = 0xffffffff;
		pMinXDist = NULL;
		while (pColumn)
		{
			if (pColumn->getFirstContainer())
			{
				if (
					(x >= pColumn->getX())
					&& (x < (pColumn->getX() + pColumn->getWidth()))
					&& (y >= pColumn->getY())
					&& (y < (pColumn->getY() + pColumn->getHeight()))
					)
				{
					pColumn->mapXYToPosition(x - pColumn->getX(), y - pColumn->getY(), pos, bBOL, bEOL,isTOC);
					return;
				}

				iDist = pColumn->distanceFromPoint(x, y);
				if (iDist < iMinDist)
				{
					iMinDist = iDist;
					pMinDist = static_cast<fp_VerticalContainer *>(pColumn);
				}

				if (
					(y >= pColumn->getY())
					&& (y < (pColumn->getY() + pColumn->getHeight()))
					)
				{
					if (iDist < iMinXDist)
					{
						iMinXDist = iDist;
						pMinXDist = static_cast<fp_VerticalContainer *>(pColumn);
					}
				}
			}

			pColumn = pColumn->getFollower();
		}
	}

//
// Now look in footnotes
//
	fp_FootnoteContainer * pFC = NULL;
	for (i=0; i<static_cast<UT_sint32>(countFootnoteContainers()); i++)
	{
		pFC = getNthFootnoteContainer(i);
		if (pFC->getFirstContainer())
		{
			if ((x >= pFC->getX())
				&& (x < (pFC->getX() + pFC->getWidth()))
				&& (y >= pFC->getY())
				&& (y < (pFC->getY() + pFC->getHeight()))
				)
			{
				pFC->mapXYToPosition(x - pFC->getX(), y - pFC->getY(), pos, bBOL, bEOL,isTOC);
					return;
			}

			iDist = pFC->distanceFromPoint(x, y);
			if (iDist < iMinDist)
			{
				iMinDist = iDist;
				pMinDist = static_cast<fp_VerticalContainer *>(pFC);
			}

			if ( (y >= pFC->getY())
				 && (y < (pFC->getY() + pFC->getHeight()))) 
			{
				if (iDist < iMinXDist)
				{
					iMinXDist = iDist;
					pMinXDist = static_cast<fp_VerticalContainer *>(pFC);
				}
			}
		}
	}

//
// Now look in annotations
//
	if(getDocLayout()->displayAnnotations())
	{
			fp_AnnotationContainer * pAC = NULL;
			for (i=0; i<static_cast<UT_sint32>(countAnnotationContainers()); i++)
			{
					pAC = getNthAnnotationContainer(i);
					if (pAC->getFirstContainer())
					{
							if ((x >= pAC->getX())
								&& (x < (pAC->getX() + pAC->getWidth()))
								&& (y >= pAC->getY())
								&& (y < (pAC->getY() + pAC->getHeight()))
								)
						    {
									pAC->mapXYToPosition(x - pAC->getX(), y - pAC->getY(), pos, bBOL, bEOL,isTOC);
									return;
							}
							
							iDist = pAC->distanceFromPoint(x, y);
							if (iDist < iMinDist)
							{
									iMinDist = iDist;
									pMinDist = static_cast<fp_VerticalContainer *>(pAC);
							}
							
							if ( (y >= pAC->getY())
								 && (y < (pAC->getY() + pAC->getHeight()))) 
							{
									if (iDist < iMinXDist)
									{
											iMinXDist = iDist;
											pMinXDist = static_cast<fp_VerticalContainer *>(pAC);
									}
							}
					}
			}
	}



	if (pMinXDist)
	{
		pMinXDist->mapXYToPosition(x - pMinXDist->getX(), y - pMinXDist->getY(), pos, bBOL, bEOL,isTOC);
		return;
	}

	UT_ASSERT(pMinDist);

	if (pMinDist)
	{
		pMinDist->mapXYToPosition(x - pMinDist->getX(), y - pMinDist->getY(), pos, bBOL, bEOL,isTOC);
	}
}

void fp_Page::setView(FV_View* pView)
{
	m_pView = pView;
}

const fp_PageSize&	fp_Page::getPageSize(void) const
{
	return m_pageSize;
}

// ---------------------------------------------------------------------
// The rest of the functions in this file deal with page headers &
// footers.

void fp_Page::removeHdrFtr(HdrFtrType hfType)
{
	UT_ASSERT(hfType >= FL_HDRFTR_HEADER && hfType <= FL_HDRFTR_FOOTER_LAST);
	if(hfType < FL_HDRFTR_FOOTER)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Deleting header from page %x m_pHeader = %x \n",this, m_pHeader));
		if(m_pHeader == NULL)
			return;
		delete m_pHeader;
		m_pHeader = NULL;
	}
	else
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Deleting footer from page %x m_pFooter = %x \n",this,m_pFooter));
		if(m_pFooter == NULL)
			return;
		delete m_pFooter;
		m_pFooter = NULL;
	}
}

fp_ShadowContainer* fp_Page::getHdrFtrP(HdrFtrType hfType) const
{
	if(hfType < FL_HDRFTR_FOOTER)
	{
		return m_pHeader;
	}
	else
	{
		return m_pFooter;
	}
}

fp_ShadowContainer* 
fp_Page::buildHdrFtrContainer(fl_HdrFtrSectionLayout* pHFSL,
							  HdrFtrType hfType)
{
	UT_ASSERT(hfType == FL_HDRFTR_HEADER || hfType == FL_HDRFTR_FOOTER);
	bool bIsHead = (hfType == FL_HDRFTR_HEADER);
	fp_ShadowContainer ** ppHF = bIsHead ? &m_pHeader : &m_pFooter;

	if (*ppHF)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		(*ppHF)->getHdrFtrSectionLayout()->deletePage(this);

		UT_ASSERT_HARMLESS( !*ppHF );
	}
	xxx_UT_DEBUGMSG(("SEVIOR: Building header container. page = %x hdrftr = %x \n",this,pHFSL));

	//
	// headerMargin is the height from the top of the page.
	//
	if (bIsHead)
	{
	    *ppHF = new fp_ShadowContainer(m_pOwner->getLeftMargin(),
									 m_pOwner->getHeaderMargin(),
									 getWidth() - (m_pOwner->getLeftMargin() + 
												   m_pOwner->getRightMargin()),
									 m_pOwner->getTopMargin() - 
									   m_pOwner->getHeaderMargin(),
									 pHFSL);
	}
	else
	{
		*ppHF = new fp_ShadowContainer(m_pOwner->getLeftMargin(),
									 getHeight() - m_pOwner->getBottomMargin(),
									 getWidth() - (m_pOwner->getLeftMargin()+ 
                                                  m_pOwner->getRightMargin()),
									   m_pOwner->getBottomMargin() - m_pOwner->getFooterMargin(),
									 pHFSL);
	}

	UT_return_val_if_fail(*ppHF, NULL);

	(*ppHF)->setPage(this);
	xxx_UT_DEBUGMSG(("SEVIOR: Page for shadow %x is %x \n",*ppHF,this));
	return *ppHF;
}

/** Return the first container for pHFSL. */
fp_ShadowContainer* fp_Page::getHdrFtrContainer(fl_HdrFtrSectionLayout* pHFSL)
{
	if (pHFSL->getHFType() < FL_HDRFTR_FOOTER)
	{
		if (m_pHeader)
			return m_pHeader;
		else
			return buildHdrFtrContainer(pHFSL, FL_HDRFTR_HEADER);
	}
	else
	{
		if (m_pFooter)
			return m_pFooter;
		else
			return buildHdrFtrContainer(pHFSL, FL_HDRFTR_FOOTER);
	}
}

// Frame methods

void fp_Page::frameHeightChanged(void)
{
}

void fp_Page::clearScreenFrames(void)
{
	UT_sint32 i =0;
	for (i = 0; i < countAboveFrameContainers(); i++)
	{
		getNthAboveFrameContainer(i)->clearScreen();
	}
	for (i = 0; i < countBelowFrameContainers(); i++)
	{
		getNthBelowFrameContainer(i)->clearScreen();
	}
}

void fp_Page::markDirtyOverlappingRuns(fp_FrameContainer * pFrameC)
{
	UT_Rect * pMyFrameRect = pFrameC->getScreenRect();
	if(pMyFrameRect == NULL)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	// check each column for redraw

	UT_sint32 count = m_vecColumnLeaders.getItemCount();
	UT_sint32 i = 0;
	for (i=0; i<count; i++)
	{
		fp_Column* pCol = m_vecColumnLeaders.getNthItem(i);
		while (pCol)
		{
			pCol->markDirtyOverlappingRuns(*pMyFrameRect);
			pCol = pCol->getFollower();
		}
	}

	// Now do the headers and footers

	if (m_pHeader)
	{
		m_pHeader->markDirtyOverlappingRuns(*pMyFrameRect);
	}

	if (m_pFooter)
	{
		m_pFooter->markDirtyOverlappingRuns(*pMyFrameRect);
	}

	// Now Footnotes

	count = m_vecFootnotes.getItemCount();
	for (i=0; i<count; i++)
	{
		fp_FootnoteContainer* pFC = m_vecFootnotes.getNthItem(i);
		pFC->markDirtyOverlappingRuns(*pMyFrameRect);
	}


	// Now Annotations
	if(getDocLayout()->displayAnnotations())
	{
			count = m_vecAnnotations.getItemCount();
			for (i=0; i<count; i++)
			{
					fp_AnnotationContainer* pAC = m_vecAnnotations.getNthItem(i);
					pAC->markDirtyOverlappingRuns(*pMyFrameRect);
			}
	}

	// Now Frames

	count = m_vecAboveFrames.getItemCount();
	for (i=0; i<count; i++)
	{
		fp_FrameContainer* pFC = m_vecAboveFrames.getNthItem(i);
		if(pFC != pFrameC)
		{
			pFC->markDirtyOverlappingRuns(*pMyFrameRect);
		}
	}


	count = m_vecBelowFrames.getItemCount();
	for (i=0; i<count; i++)
	{
		fp_FrameContainer* pFC = m_vecBelowFrames.getNthItem(i);
		if(pFC != pFrameC)
		{
			pFC->markDirtyOverlappingRuns(*pMyFrameRect);
		}
	}
	DELETEP(pMyFrameRect);
}


UT_sint32 fp_Page::countAboveFrameContainers(void) const
{
        return m_vecAboveFrames.getItemCount();
}


UT_sint32 fp_Page::countBelowFrameContainers(void) const
{
        return m_vecBelowFrames.getItemCount();
}

UT_sint32 fp_Page::findFrameContainer(fp_FrameContainer * pFC) const
{
        UT_sint32 i; 
        if(pFC->isAbove())
	{
	  i = m_vecAboveFrames.findItem(pFC);
	  return i;

	}
        i = m_vecBelowFrames.findItem(pFC);
	return i;
}

fp_FrameContainer* fp_Page::getNthAboveFrameContainer(UT_sint32 n) const 
{
	return m_vecAboveFrames.getNthItem(n);
} 


fp_FrameContainer* fp_Page::getNthBelowFrameContainer(UT_sint32 n) const 
{
	return m_vecBelowFrames.getNthItem(n);
} 

bool fp_Page::insertFrameContainer(fp_FrameContainer * pFC)
{
        if(pFC->isAbove())
	{
	       m_vecAboveFrames.addItem(pFC);
	}
	else
	{
	       m_vecBelowFrames.addItem(pFC);
	}
	if(pFC)
	{
		pFC->setPage(this);
	}
	_reformat();
	return true;
}

void fp_Page::removeFrameContainer(fp_FrameContainer * _pFC)
{

	markDirtyOverlappingRuns(_pFC);
	UT_sint32 ndx = 0;
	bool isAbove = false;
	if(_pFC->isAbove())
	{
	    ndx = m_vecAboveFrames.findItem(_pFC);
	    isAbove = true;
	}
	else
	{
	    ndx = m_vecBelowFrames.findItem(_pFC);
	}
	if(ndx>=0)
	{
	        if(isAbove)
		{
		        m_vecAboveFrames.deleteNthItem(ndx);
			for(ndx=0; ndx < static_cast<UT_sint32>(countAboveFrameContainers());ndx++)
			{			
			    fp_FrameContainer * pFC = getNthAboveFrameContainer(ndx);
			    fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pFC->getSectionLayout());
			    pFC->clearScreen();
			    pFL->markAllRunsDirty();
			}
		}
		else
		{
		        m_vecBelowFrames.deleteNthItem(ndx);
			for(ndx=0; ndx < static_cast<UT_sint32>(countAboveFrameContainers());ndx++)
			{			
			    fp_FrameContainer * pFC = getNthAboveFrameContainer(ndx);
			    fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pFC->getSectionLayout());
			    pFC->clearScreen();
			    pFL->markAllRunsDirty();
			}
		}
		_reformat();
		return;
	}
	return;
}

// Footnote methods

UT_sint32 fp_Page::countFootnoteContainers(void) const
{
	return m_vecFootnotes.getItemCount();
}

UT_sint32 fp_Page::findFootnoteContainer(fp_FootnoteContainer * pFC) const
{
	UT_sint32 i = m_vecFootnotes.findItem(pFC);
	return i;
}

fp_FootnoteContainer* fp_Page::getNthFootnoteContainer(UT_sint32 n) const 
{
	return m_vecFootnotes.getNthItem(n);
} 

bool fp_Page::insertFootnoteContainer(fp_FootnoteContainer * pFC)
{
	UT_sint32 i =0;
	UT_sint32 j = findFootnoteContainer(pFC);
	if(j >= 0)
	{
		return false;
	}
	UT_uint32 loc =0;
	UT_sint32 fVal = pFC->getValue();
	fp_FootnoteContainer * pFTemp = NULL;
	for(i=0; i< m_vecFootnotes.getItemCount();i++)
	{
		pFTemp = m_vecFootnotes.getNthItem(i);
		if(fVal < pFTemp->getValue())
		{
			loc = i;
			break;
		}
	}
	if(pFTemp == NULL)
	{
		m_vecFootnotes.addItem(pFC);
	}
	else if( i>= m_vecFootnotes.getItemCount())
	{
		m_vecFootnotes.addItem(pFC);
	}
	else
	{
		m_vecFootnotes.insertItemAt(pFC, loc);
	}
	if(pFC)
	{
		pFC->setPage(this);
	}
	_reformat();
	return true;
}

void fp_Page::removeFootnoteContainer(fp_FootnoteContainer * _pFC)
{
	UT_sint32 ndx = m_vecFootnotes.findItem(_pFC);
	if(ndx>=0)
	{
		m_vecFootnotes.deleteNthItem(ndx);
		for(ndx=0; ndx < static_cast<UT_sint32>(countFootnoteContainers());ndx++)
		{			
			fp_FootnoteContainer * pFC = getNthFootnoteContainer(ndx);
			fl_FootnoteLayout * pFL = static_cast<fl_FootnoteLayout *>(pFC->getSectionLayout());
			pFC->clearScreen();
			pFL->markAllRunsDirty();
		}
		_reformat();
		return;
	}
	return;
}



// Annotation methods

UT_sint32 fp_Page::countAnnotationContainers(void) const
{
	return m_vecAnnotations.getItemCount();
}

UT_sint32 fp_Page::findAnnotationContainer(fp_AnnotationContainer * pAC) const
{
	UT_sint32 i = m_vecAnnotations.findItem(pAC);
	return i;
}

fp_AnnotationContainer* fp_Page::getNthAnnotationContainer(UT_sint32 n) const 
{
	return m_vecAnnotations.getNthItem(n);
} 

UT_sint32 fp_Page::getAnnotationPos(UT_uint32 pid) const
{
	fp_AnnotationContainer * pACon = NULL;
	UT_sint32 i = 0;
	for(i = 0; i< countAnnotationContainers(); i++)
	{
			pACon = getNthAnnotationContainer(i);
			if(!pACon)
				return 0;
			if(pid == pACon->getPID())
				return i;
	}
	return 0;
}

bool fp_Page::insertAnnotationContainer(fp_AnnotationContainer * pAC)
{
	UT_sint32 i =0;
	UT_sint32 j = findAnnotationContainer(pAC);
	if(j >= 0)
	{
		return false;
	}
	UT_uint32 loc =0;
	UT_sint32 fVal = pAC->getValue();
	fp_AnnotationContainer * pFTemp = NULL;
	for(i=0; i< m_vecAnnotations.getItemCount();i++)
	{
		pFTemp = m_vecAnnotations.getNthItem(i);
		if(fVal < pFTemp->getValue())
		{
			loc = i;
			break;
		}
	}
	if(pFTemp == NULL)
	{
		m_vecAnnotations.addItem(pAC);
	}
	else if( i>= m_vecAnnotations.getItemCount())
	{
		m_vecAnnotations.addItem(pAC);
	}
	else
	{
		m_vecAnnotations.insertItemAt(pAC, loc);
	}
	if(pAC)
	{
		pAC->setPage(this);
	}
	if(getDocLayout()->displayAnnotations())
	{
		_reformat();
	}
	return true;
}

void fp_Page::removeAnnotationContainer(fp_AnnotationContainer * _pAC)
{
	UT_sint32 ndx = m_vecAnnotations.findItem(_pAC);
	if(ndx>=0)
	{
		m_vecAnnotations.deleteNthItem(ndx);
		if(getDocLayout()->displayAnnotations())
		{
				for(ndx=0; ndx < static_cast<UT_sint32>(countAnnotationContainers());ndx++)
				{			
						fp_AnnotationContainer * pAC = getNthAnnotationContainer(ndx);
						fl_AnnotationLayout * pAL = static_cast<fl_AnnotationLayout *>(pAC->getSectionLayout());
						pAC->clearScreen();
						pAL->markAllRunsDirty();
				}
		}
		_reformat();
		return;
	}
	return;
}
