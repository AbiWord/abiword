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
#include "fl_FootnoteLayout.h"
#include "fp_TableContainer.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"


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
		m_pHeader(0)
{
	UT_ASSERT(pLayout);
	UT_ASSERT(pOwner);

	GR_Graphics * pG = pLayout->getGraphics();
	UT_ASSERT(pG);
	m_vecColumnLeaders.clear();
	m_iResolution = pG->getResolution();
	UT_DEBUGMSG(("Created Page %x \n",this));
}

fp_Page::~fp_Page()
{
	UT_DEBUGMSG(("fpPage: Deleting page %x \n",this));
	if (m_pOwner)
	{
		fl_DocSectionLayout *pDSL = m_pOwner;
		m_pOwner = NULL;
		pDSL->deleteOwnedPage(this);
	}
}

bool fp_Page::isEmpty(void) const
{
	if((m_vecColumnLeaders.getItemCount() == 0) && (m_vecFootnotes.getItemCount() == 0))
	{
		return true;
	}
	return false;
}

UT_sint32 fp_Page::getWidth(void) const
{
	return static_cast<UT_sint32>(m_iResolution * m_pageSize.Width(DIM_IN));
}

UT_sint32 fp_Page::getHeight(void) const
{
	return static_cast<UT_sint32>(m_iResolution * m_pageSize.Height(DIM_IN));
}

UT_sint32 fp_Page::getColumnGap(void) const
{
	return getOwningSection()->getColumnGap();
}

UT_sint32 fp_Page::getAvailableHeight(void) const
{
	fl_DocSectionLayout * pDSL = getNthColumnLeader(0)->getDocSectionLayout();
	UT_sint32 avail = getHeight() - pDSL->getTopMargin() - pDSL->getBottomMargin();
	UT_sint32 i =0;
	for(i=0; i< static_cast<UT_sint32>(countFootnoteContainers()); i++)
	{
		fp_FootnoteContainer * pFC = getNthFootnoteContainer(i);
		avail -= pFC->getHeight();
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
	UT_uint32 i =0;
	fp_Column * prevColumn = NULL;
	bool bstop = false;
	if(prevContainer)
	{
		prevColumn = static_cast<fp_Column *>(prevContainer->getContainer());
	}
	for(i=0; !bstop && (i<  m_vecColumnLeaders.getItemCount()); i++)
	{
		maxHeight = 0;
		pColumn = static_cast<fp_Column *>(m_vecColumnLeaders.getNthItem(i));
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

	xoff += pContainer->getX();
	yoff += pContainer->getY();
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
        fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
        fl_DocSectionLayout* pFirstSectionLayout = (pFirstColumnLeader->getDocSectionLayout());
		UT_ASSERT(m_pOwner == pFirstSectionLayout);


        UT_sint32 iLeftMargin = pFirstSectionLayout->getLeftMargin();
        UT_sint32 iRightMargin = pFirstSectionLayout->getRightMargin();
        UT_sint32 iTopMargin = pFirstSectionLayout->getTopMargin();
        UT_sint32 iBottomMargin = pFirstSectionLayout->getBottomMargin();

        UT_sint32 xoffStart = pDA->xoff + iLeftMargin - 1;
        UT_sint32 yoffStart = pDA->yoff + iTopMargin - 1;
        UT_sint32 xoffEnd = pDA->xoff + getWidth() - iRightMargin + 2;
        UT_sint32 yoffEnd = pDA->yoff + getHeight() - iBottomMargin + 2;

        UT_sint32 iLeftWidth = UT_MIN(iLeftMargin,20);
        UT_sint32 iRightWidth = UT_MIN(iRightMargin,20);
        UT_sint32 iTopHeight = UT_MIN(iTopMargin,20);
        UT_sint32 iBottomHeight = UT_MIN(iBottomMargin,20);

        pDA->pG->setColor(getDocLayout()->getView()->getColorShowPara());

		pDA->pG->setLineProperties(1.0,
									 GR_Graphics::JOIN_MITER,
									 GR_Graphics::CAP_BUTT,
									 GR_Graphics::LINE_SOLID);

        pDA->pG->drawLine(xoffStart, yoffStart, xoffStart, yoffStart - iTopHeight);
        pDA->pG->drawLine(xoffStart, yoffStart, xoffStart - iLeftWidth, yoffStart);

        pDA->pG->drawLine(xoffEnd, yoffStart - iTopHeight, xoffEnd, yoffStart);
        pDA->pG->drawLine(xoffEnd, yoffStart, xoffEnd + iRightWidth, yoffStart);

        pDA->pG->drawLine(xoffStart, yoffEnd, xoffStart, yoffEnd + iBottomHeight);
        pDA->pG->drawLine(xoffStart - iLeftWidth, yoffEnd, xoffStart, yoffEnd);

        pDA->pG->drawLine(xoffEnd, yoffEnd, xoffEnd, yoffEnd + iBottomHeight);
        pDA->pG->drawLine(xoffEnd, yoffEnd, xoffEnd + iRightWidth, yoffEnd);
    }
}

void fp_Page::draw(dg_DrawArgs* pDA, bool bAlwaysUseWhiteBackground)
{
//
// Fill the Page with the page color
//
// only call this for printing and honour the option to not fill the paper with
// color.
//
	int i=0;
	if(!bAlwaysUseWhiteBackground && !pDA->pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		xxx_UT_DEBUGMSG(("Doing a rectangular color fill \n"));
		UT_RGBColor * pClr = getOwningSection()->getPaperColor();
 		UT_sint32 xmin = pDA->xoff;
  		UT_sint32 ymin = pDA->yoff;

		UT_sint32 height =getHeight();
		UT_sint32 width = getWidth();
		pDA->pG->fillRect(*pClr,xmin,ymin,width,height);
	}

    _drawCropMarks(pDA);

	// draw each column on the page
	int count = m_vecColumnLeaders.getItemCount();
	for (i=0; i<count; i++)
	{
		fp_Column* pCol = static_cast<fp_Column*>(m_vecColumnLeaders.getNthItem(i));
		while (pCol)
		{
			dg_DrawArgs da = *pDA;
			da.xoff += pCol->getX();
			da.yoff += pCol->getY();
			pCol->draw(&da);

			fp_Column *pNextCol = pCol->getFollower();

			if(pNextCol && pCol->getDocSectionLayout()->getColumnLineBetween())
			{
				// draw line between columns if required.

				UT_sint32 x = pDA->xoff + (pCol->getX() + pCol->getWidth() + pNextCol->getX()) / 2;
				UT_sint32 y = pDA->yoff + pCol->getY();
				pDA->pG->setColor(m_pView->getColorColumnLine());
				pDA->pG->drawLine(x, y, x, y + pCol->getHeight());
			}

			pCol = pNextCol;
		}
	}

	// draw the page's headers and footers
    if(m_pView->getViewMode() == VIEW_PRINT)
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
		fp_FootnoteContainer* pFC = static_cast<fp_FootnoteContainer*>(m_vecFootnotes.getNthItem(i));
		dg_DrawArgs da = *pDA;
		da.xoff += pFC->getX();
		da.yoff += pFC->getY();
		pFC->draw(&da);
	}

	m_bNeedsRedraw = false;
}

bool fp_Page::needsRedraw(void) const
{
	return m_bNeedsRedraw;
}

UT_uint32 fp_Page::countColumnLeaders(void) const
{
	return m_vecColumnLeaders.getItemCount();
}

fp_Column* fp_Page::getNthColumnLeader(UT_sint32 n) const
{
	return static_cast<fp_Column*>(m_vecColumnLeaders.getNthItem(n));
}

/*!
 * This method scans the current page to make sure that there is space to at least start
 * every column on the page. Columns with space are deleted and their contents redistributed.
 */
bool fp_Page::breakPage(void)
{
	UT_uint32 count = countColumnLeaders();
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
	UT_uint32 i = 0;
	UT_uint32 iFootnoteHeight = 2*pFirstSectionLayout->getFootnoteLineThickness();
	for (i = 0; i < countFootnoteContainers(); i++)
	{
		iFootnoteHeight += getNthFootnoteContainer(i)->getHeight();
	}
	iY =+ iFootnoteHeight;

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

void fp_Page::_reformat(void)
{
	// this is naive, because columns can cause the footnotes
	// to change pages.  But it'll do for now.
	_reformatColumns(); _reformatFootnotes();
}

void fp_Page::_reformatColumns(void)
{
	UT_uint32 count = countColumnLeaders();
	if (count == 0)
		return;

	fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
	fp_Column * pLastCol = NULL;
	fl_DocSectionLayout* pFirstSectionLayout = (pFirstColumnLeader->getDocSectionLayout());
	UT_ASSERT(m_pOwner == pFirstSectionLayout);


	UT_sint32 iLeftMargin = 0;
	UT_sint32 iRightMargin = 0;
	UT_sint32 iTopMargin = pFirstSectionLayout->getTopMargin();
	UT_sint32 iBottomMargin = pFirstSectionLayout->getBottomMargin();
	UT_sint32 iY = iTopMargin;

	// we need the height of the footnotes on this page, to deduct.
	UT_uint32 i = 0;
	UT_uint32 iFootnoteHeight = 2*pFirstSectionLayout->getFootnoteLineThickness();
	for (i = 0; i < countFootnoteContainers(); i++)
	{
		iFootnoteHeight += getNthFootnoteContainer(i)->getHeight();
	}

	for (i = 0; i < count; i++)
	{
		if (iY >= (static_cast<UT_sint32>(getHeight() - iBottomMargin - iFootnoteHeight)))
		{
			UT_DEBUGMSG(("SEVIOR: Page incorrectly laid out iYlayoutuints= %d  \n",iY));
			m_pOwner->markForRebuild();
			UT_ASSERT(0);
			return;
//			break;
		}

		fp_Column* pLeader = getNthColumnLeader(i);
		UT_ASSERT(pLeader->getContainerType() == FP_CONTAINER_COLUMN);
		fl_DocSectionLayout* pSL = (pLeader->getDocSectionLayout());

		iLeftMargin = pSL->getLeftMargin();
		iRightMargin = pSL->getRightMargin();

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
		UT_sint32 iMostHeight = 0;

		while (pTmpCol)
		{
			UT_ASSERT(pTmpCol->getContainerType() == FP_CONTAINER_COLUMN);
			pTmpCol->setX(iX);
			pTmpCol->setY(iY);
			pTmpCol->setMaxHeight(getHeight() - iBottomMargin - iY);
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
	fp_Page *pNext = getNext();
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
			if( !bIsTable && (iY + 3*iYNext) < (getHeight() - getFootnoteHeight() - iBottomMargin))
			{
		   		m_pOwner->markForRebuild();
				//UT_ASSERT(0);
			}
		}
	}
	return;
}

void fp_Page::clearScreenFootnotes(void)
{
	UT_sint32 i =0;
	for (i = 0; i < static_cast<UT_sint32>(countFootnoteContainers()); i++)
	{
		getNthFootnoteContainer(i)->clearScreen();
	}
}

UT_sint32 fp_Page::getFootnoteHeight(void)
{
	UT_uint32 iFootnoteHeight = 0;
	UT_uint32 i = 0;
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
	UT_uint32 iFootnoteHeight = 0;
	UT_uint32 i = 0;
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

		pFC->setX(pSL->getLeftMargin());
		pFC->setY(pageHeight);
		pageHeight += getNthFootnoteContainer(i)->getHeight();
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
		UT_DEBUGMSG(("fp_Page: Remove page %x from DSL %x \n",this,m_pOwner)); 
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
	m_pOwner->setNeedsSectionBreak(true,this);
	if(breakPage())
	{
		_reformat();
	}
	else
	{
		UT_DEBUGMSG(("SEVIOR: Mark for rebuild from footnoteheight changed. \n"));
		m_pOwner->markForRebuild();
	}
}

void fp_Page::columnHeightChanged(fp_Column* pCol)
{
	fp_Column* pLeader = pCol->getLeader();
	xxx_UT_DEBUGMSG(("SEVIOR: Column height changed \n"));
#if !defined(NDEBUG)
	UT_sint32 ndx =
#endif
		m_vecColumnLeaders.findItem(pLeader);
	UT_ASSERT(ndx >= 0);
	if(breakPage())
	{
		_reformat();
	}
	else
	{
		UT_DEBUGMSG(("SEVIOR: Mark for rebuild from columnheight changed. \n"));
		m_pOwner->markForRebuild();
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
		UT_ASSERT(pColumn);
		fp_Container* pFirstContainer = static_cast<fp_Container *>(pColumn->getFirstContainer());
		UT_ASSERT(pFirstContainer);

		fp_Run* pFirstRun = static_cast<fp_Line *>(pFirstContainer)->getFirstRun();
		fl_BlockLayout* pFirstBlock = static_cast<fp_Line *>(pFirstContainer)->getBlock(); // SEVIOR This needs fix me, FIXME

		pos = pFirstRun->getBlockOffset() + pFirstBlock->getPosition();
	}
	else
	{
		fp_Column* pColumn = getNthColumnLeader(cols-1);
		UT_ASSERT(pColumn);
		fp_Container* pLastContainer = static_cast<fp_Container *>(pColumn->getLastContainer());
		UT_ASSERT(pLastContainer);

		fp_Run* pLastRun = static_cast<fp_Line *>(pLastContainer)->getLastRun();
		fl_BlockLayout* pLastBlock = static_cast<fp_Line *>(pLastContainer)->getBlock();

		while (!pLastRun->isFirstRunOnLine() && pLastRun->isForcedBreak())
		{
			pLastRun = pLastRun->getPrev();
		}

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

/*!
 * This method maps an x,y location on the page to the position in the
 * document of the corrsponding element.
 * This variation looks in the header/footer region and returns the
 * SectionLayout shadow of the
 \param x coordinate
 \param y coordinate
 \param bBOL
 \param bEOL
 \return pos The Document position corresponding the text at location x,y
 \return pShadow A pointer to the shadow corresponding to this header/footer
 */
void fp_Page::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool bUseHdrFtr, fl_HdrFtrShadow ** pShadow)
{
	int count = m_vecColumnLeaders.getItemCount();
	UT_uint32 iMinDist = 0xffffffff;
	fp_VerticalContainer * pMinDist = NULL;
	fp_Column* pColumn = NULL;
	UT_uint32 iMinXDist = 0xffffffff;
	fp_VerticalContainer* pMinXDist = NULL;
	UT_uint32 iDist = 0;
	fp_Column* pLeader = NULL;
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
			for (UT_uint32 i = 0; i < NrElements(hf); i++)
			{
				fp_ShadowContainer * p = hf[i];

				if(p == NULL || !p->getFirstContainer())
					continue;

				if ((x >= p->getX())
					&& (x < (p->getX() + p->getWidth()))
					&& (y >= p->getY())
					&& (y < (p->getY() + p->getHeight())))
				{
					p->mapXYToPosition(x - p->getX(), y - p->getY(), pos, bBOL, bEOL);
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
	UT_sint32 i =0;
	for (i=0; i<count; i++)
	{
		pLeader = static_cast<fp_Column*>(m_vecColumnLeaders.getNthItem(i));

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
					pColumn->mapXYToPosition(x - pColumn->getX(), y - pColumn->getY(), pos, bBOL, bEOL);
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
				pFC->mapXYToPosition(x - pFC->getX(), y - pFC->getY(), pos, bBOL, bEOL);
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
	if (pMinXDist)
	{
		pMinXDist->mapXYToPosition(x - pMinXDist->getX(), y - pMinXDist->getY(), pos, bBOL, bEOL);
		return;
	}

	UT_ASSERT(pMinDist);

	pMinDist->mapXYToPosition(x - pMinDist->getX(), y - pMinDist->getY(), pos, bBOL, bEOL);
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
									 getHeight() - m_pOwner->getBottomMargin()+
									              m_pOwner->getFooterMargin(),
									 getWidth() - (m_pOwner->getLeftMargin()+ 
                                                  m_pOwner->getRightMargin()),
									 m_pOwner->getBottomMargin(),
									 pHFSL);
	}

	UT_return_val_if_fail(*ppHF, NULL);

	(*ppHF)->setPage(this);
	UT_DEBUGMSG(("SEVIOR: Page for shadow %x is %x \n",*ppHF,this));
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

UT_uint32 fp_Page::countFootnoteContainers(void) const
{
	return m_vecFootnotes.getItemCount();
}

UT_sint32 fp_Page::findFootnoteContainer(fp_FootnoteContainer * pFC)
{
	UT_sint32 i = m_vecFootnotes.findItem(static_cast<void *>(pFC));
	return i;
}

fp_FootnoteContainer* fp_Page::getNthFootnoteContainer(UT_sint32 n) const 
{
	return static_cast<fp_FootnoteContainer*>(m_vecFootnotes.getNthItem(n));
} 

bool fp_Page::insertFootnoteContainer(fp_FootnoteContainer * pFC)
{
	UT_uint32 i =0;
	UT_uint32 loc =0;
	UT_sint32 fVal = pFC->getValue();
	fp_FootnoteContainer * pFTemp = NULL;
	for(i=0; i< m_vecFootnotes.getItemCount();i++)
	{
		pFTemp = static_cast<fp_FootnoteContainer *>(m_vecFootnotes.getNthItem(i));
		if(fVal < pFTemp->getValue())
		{
			loc = i;
			break;
		}
	}
	if(pFTemp == NULL)
	{
		m_vecFootnotes.addItem(static_cast<void *>(pFC));
	}
	else if( i>= m_vecFootnotes.getItemCount())
	{
		m_vecFootnotes.addItem(static_cast<void *>(pFC));
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

void fp_Page::removeFootnoteContainer(fp_FootnoteContainer * pFC)
{
	UT_sint32 ndx = m_vecFootnotes.findItem(pFC);
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

