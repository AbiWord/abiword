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

	m_iResolution = pG->getResolution();
}

fp_Page::~fp_Page()
{
	if (m_pOwner)
	{
		m_pOwner->deleteOwnedPage(this);
		m_pOwner = NULL;
	}
}

bool fp_Page::isEmpty(void) const
{
	return m_vecColumnLeaders.getItemCount() == 0;
}

UT_sint32 fp_Page::getWidth(void) const
{
	return (UT_sint32)(m_iResolution * m_pageSize.Width(DIM_IN));
}

UT_sint32 fp_Page::getWidthInLayoutUnits(void) const
{
	return (UT_sint32)UT_convertSizeToLayoutUnits(m_pageSize.Width(DIM_IN), DIM_IN);
}

UT_sint32 fp_Page::getHeight(void) const
{
	return (UT_sint32)(m_iResolution * m_pageSize.Height(DIM_IN));
}

UT_sint32 fp_Page::getHeightInLayoutUnits(void) const
{
	return (UT_sint32)UT_convertSizeToLayoutUnits(m_pageSize.Height(DIM_IN), DIM_IN);
}

/*!
 * Returns the page height minus the top and bottom margins in layout units
 */
UT_sint32 fp_Page::getAvailableHeightInLayoutUnits(void) const
{
	fl_DocSectionLayout * pDSL = getNthColumnLeader(0)->getDocSectionLayout();
	UT_sint32 avail = getHeightInLayoutUnits() - pDSL->getTopMarginInLayoutUnits() - pDSL->getBottomMarginInLayoutUnits();
	return avail;
}

/*!
 * This method scans the page and returns the total height in layout units of all the columns 
 * on it.
 * If prevLine is non-NULL the maximum column height up to this line is calculated.
 */
UT_sint32 fp_Page::getFilledHeightInLayoutUnits(fp_Line * prevLine) const
{
	UT_sint32 totalHeight = 0;
    UT_sint32 maxHeight = 0;
	fp_Column * pColumn = NULL;
	UT_uint32 i =0;
	fp_Column * prevColumn = NULL;
	bool bstop = false;
	if(prevLine)
	{
		prevColumn = (fp_Column *) prevLine->getContainer();
	}
	for(i=0; !bstop && (i<  m_vecColumnLeaders.getItemCount()); i++)
	{
		maxHeight = 0;
		pColumn = (fp_Column *) m_vecColumnLeaders.getNthItem(i);
		totalHeight += pColumn->getDocSectionLayout()->getSpaceAfterInLayoutUnits();
		while(pColumn != NULL)
		{
			if(prevColumn == pColumn)
			{
				bstop = true;
				fp_Line * pCurLine = pColumn->getFirstLine();
				UT_sint32 curHeight = 0;
				while((pCurLine != NULL) && (pCurLine != prevLine))
				{
					curHeight += pCurLine->getHeightInLayoutUnits();
					pCurLine = pCurLine->getNext();
				}
				if(pCurLine == prevLine)
				{
					curHeight += pCurLine->getHeightInLayoutUnits();
				}
				maxHeight = UT_MAX(curHeight,maxHeight);
			}
			else
			{
				maxHeight = UT_MAX(pColumn->getHeightInLayoutUnits(),maxHeight);
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

void fp_Page::getScreenOffsets(fp_Container* pContainer, UT_sint32& xoff, UT_sint32& yoff)
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
}

void fp_Page::setPrev(fp_Page* p)
{
	m_pPrev = p;
}

FL_DocLayout* fp_Page::getDocLayout()
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

        UT_RGBColor clrShowPara(127,127,127);
        pDA->pG->setColor(clrShowPara);
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
	// draw each column on the page
	int count = m_vecColumnLeaders.getItemCount();
//
// Fill the Page with the page color
//
// only call this for printing and honour the option to not fill the paper with
// color.
//
	if(!bAlwaysUseWhiteBackground && !pDA->pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		xxx_UT_DEBUGMSG(("Doing a rectangular color fill \n"));
		UT_RGBColor * pClr = getOwningSection()->getPaperColor();
		double ScaleLayoutUnitsToScreen;
		ScaleLayoutUnitsToScreen = (double)pDA->pG->getResolution() / UT_LAYOUT_UNITS;
 		UT_sint32 xmin = pDA->xoff;
  		UT_sint32 ymin = pDA->yoff;
		UT_sint32 height = (UT_sint32) ((double)getHeightInLayoutUnits() * ScaleLayoutUnitsToScreen);
		UT_sint32 width = (UT_sint32) ((double)getWidthInLayoutUnits() * ScaleLayoutUnitsToScreen);
		pDA->pG->fillRect(*pClr,xmin,ymin,width,height);
	}

    _drawCropMarks(pDA);

	for (int i=0; i<count; i++)
	{
		fp_Column* pCol = (fp_Column*) m_vecColumnLeaders.getNthItem(i);

		fp_Column* pTmpCol = pCol;
		while (pTmpCol)
		{
			dg_DrawArgs da = *pDA;
			da.xoff += pTmpCol->getX();
			da.yoff += pTmpCol->getY();
			pTmpCol->draw(&da);

			fp_Column *pNextCol = pTmpCol->getFollower();

			if(pNextCol && pTmpCol->getDocSectionLayout()->getColumnLineBetween())
			{
				// draw line between columns if required.

				UT_sint32 x = pDA->xoff + (pTmpCol->getX() + pTmpCol->getWidth() + pNextCol->getX()) / 2;
				UT_sint32 y = pDA->yoff + pTmpCol->getY();
				UT_RGBColor Line_color(0, 0, 0);
				pDA->pG->setColor(Line_color);
				pDA->pG->drawLine(x, y, x, y + pTmpCol->getHeight());
			}

			pTmpCol = pNextCol;
		}
	}
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
	return (fp_Column*) m_vecColumnLeaders.getNthItem(n);
}

/*!
 * This method scans the current page to make sure that there is space to at least start
 * every column on the page. Columns with space are deleted and their contents redistributed.
 */
bool fp_Page::breakPage(void)
{
	UT_sint32 count = countColumnLeaders();
	if (count <= 0)
	{
		return true;
	}
	UT_sint32 iYPrev = 0;
	fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
	fl_DocSectionLayout* pFirstSectionLayout = (pFirstColumnLeader->getDocSectionLayout());
	UT_ASSERT(m_pOwner == pFirstSectionLayout);
	UT_sint32 iTopMarginLayoutUnits = pFirstSectionLayout->getTopMarginInLayoutUnits();
	UT_sint32 iBottomMarginLayoutUnits = pFirstSectionLayout->getBottomMarginInLayoutUnits();
	UT_sint32 iYLayoutUnits = iTopMarginLayoutUnits;
	UT_sint32 availHeight = getHeightInLayoutUnits() - iBottomMarginLayoutUnits;
	UT_sint32 i;
	for (i=0; i<count; i++)
	{
		fp_Column* pLeader = getNthColumnLeader(i);
		fp_Column* pTmpCol = pLeader;
		UT_sint32 iMostHeightLayoutUnits = 0;
		iYPrev = iYLayoutUnits;
		while (pTmpCol)
		{
			iMostHeightLayoutUnits = UT_MAX(iMostHeightLayoutUnits, pTmpCol->getHeightInLayoutUnits());
			pTmpCol = pTmpCol->getFollower();
		}
		iYLayoutUnits += iMostHeightLayoutUnits;
		iYLayoutUnits += pLeader->getDocSectionLayout()->getSpaceAfterInLayoutUnits();
		iYLayoutUnits += pLeader->getDocSectionLayout()->getSpaceAfterInLayoutUnits();
		if (iYLayoutUnits >= availHeight)
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
		UT_sint32 maxLines= 0;
		UT_sint32 maxLineHeight = 0;
		fp_Column * pCol = pPrev;
		while(pCol != NULL)
		{
			UT_sint32 countLines = 0;
			fp_Line * pLine = pCol->getFirstLine();
			while(pLine != NULL && pLine != pCol->getLastLine())
			{
				countLines++;
				maxLineHeight = UT_MAX(maxLineHeight,pLine->getHeightInLayoutUnits());
				pLine = pLine->getNext();
			}
			if(pLine != NULL)
			{
				maxLineHeight = UT_MAX(maxLineHeight,pLine->getHeightInLayoutUnits());
				countLines++;
			}
			maxLines = UT_MAX(maxLines,countLines);
			pCol = pCol->getFollower();
		}
		if(maxLines > 1)
		{
			return true;
		}
//
//OK this is a candidate to clear off this page. Next test, is the column over
//80% of the way down the page?
//
		double rat = (double) iYPrev / (double) availHeight;
		if(rat < 0.80)
			return true;
//
// Finally if iYPrev plus 2* prev line height is greater than or equal to the
// total height, remove the container.
//
		if((iYPrev + 2*maxLineHeight) < availHeight)
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
		pFound = (fp_Column *) pPage->getNthColumn(i);
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
	int count = countColumnLeaders();
	if (count <= 0)
	{
		return;
	}

	fp_Column* pFirstColumnLeader = getNthColumnLeader(0);
	fp_Column * pLastCol = NULL;
	fl_DocSectionLayout* pFirstSectionLayout = (pFirstColumnLeader->getDocSectionLayout());
	UT_ASSERT(m_pOwner == pFirstSectionLayout);
	

	UT_sint32 iLeftMargin = 0;
	UT_sint32 iRightMargin = 0;
	UT_sint32 iTopMargin = pFirstSectionLayout->getTopMargin();
	UT_sint32 iBottomMargin = pFirstSectionLayout->getBottomMargin();


	UT_sint32 iLeftMarginLayoutUnits = 0;
	UT_sint32 iRightMarginLayoutUnits = 0;

	UT_sint32 iTopMarginLayoutUnits = pFirstSectionLayout->getTopMarginInLayoutUnits();
	UT_sint32 iBottomMarginLayoutUnits = pFirstSectionLayout->getBottomMarginInLayoutUnits();
	
	UT_sint32 iY = iTopMargin;
	UT_sint32 iYLayoutUnits = iTopMarginLayoutUnits;
	
	int i;
	for (i=0; i<count; i++)
	{
		if (iYLayoutUnits >= (getHeightInLayoutUnits() - iBottomMarginLayoutUnits))
		{
			UT_DEBUGMSG(("SEVIOR: Page incorrectly laid out iYlayoutuints= %d  \n",iYLayoutUnits));
			m_pOwner->markForRebuild();
			return;
//			break;
		}

		fp_Column* pLeader = getNthColumnLeader(i);
		fl_DocSectionLayout* pSL = (pLeader->getDocSectionLayout());
	
		iLeftMargin = pSL->getLeftMargin();
		iRightMargin = pSL->getRightMargin();

		UT_uint32 iSpace = getWidth() - iLeftMargin - iRightMargin;
		pSL->checkAndAdjustColumnGap(iSpace);

		UT_uint32 iNumColumns = pSL->getNumColumns();
		UT_uint32 iColumnGap = pSL->getColumnGap();
		UT_uint32 iColumnGapLayoutUnits = pSL->getColumnGapInLayoutUnits();

		iLeftMarginLayoutUnits = pSL->getLeftMarginInLayoutUnits();
		iRightMarginLayoutUnits = pSL->getRightMarginInLayoutUnits();

		UT_uint32 iSpaceLayoutUnits = getWidthInLayoutUnits() - iLeftMarginLayoutUnits - iRightMarginLayoutUnits;
		UT_uint32 iColWidth = (iSpace - ((iNumColumns - 1) * iColumnGap)) / iNumColumns;
		UT_uint32 iColWidthLayoutUnits = (iSpaceLayoutUnits - ((iNumColumns - 1) * iColumnGapLayoutUnits)) / iNumColumns;
		
#ifdef BIDI_ENABLED
		UT_sint32 iX;
		if(pSL->getColumnOrder())
		{
			iX = getWidth() - iRightMargin - iColWidth;
		}
		else
		{
			iX = iLeftMargin;
		}
#else		
		UT_sint32 iX = iLeftMargin;
#endif
		
		fp_Column* pTmpCol = pLeader;
		UT_sint32 iMostHeight = 0;
		UT_sint32 iMostHeightLayoutUnits = 0;
		while (pTmpCol)
		{
			pTmpCol->setX(iX);
			pTmpCol->setY(iY);
			pTmpCol->setMaxHeight(getHeight() - iBottomMargin - iY);
			pTmpCol->setMaxHeightInLayoutUnits(getHeightInLayoutUnits() - iBottomMarginLayoutUnits - iYLayoutUnits);
			pTmpCol->setWidth(iColWidth);
			pTmpCol->setWidthInLayoutUnits(iColWidthLayoutUnits);
#ifdef BIDI_ENABLED			
		if(pSL->getColumnOrder())
		{
			iX -= (iColWidth + iColumnGap);
		}
		else
		{
			iX += (iColWidth + iColumnGap);
		}		
#else
			iX += (iColWidth + iColumnGap);
#endif
			iMostHeight = UT_MAX(iMostHeight, pTmpCol->getHeight());
			iMostHeightLayoutUnits = UT_MAX(iMostHeightLayoutUnits, pTmpCol->getHeightInLayoutUnits());
			pLastCol = pTmpCol;
			pTmpCol = pTmpCol->getFollower();
		}

		iY += iMostHeight;
		iYLayoutUnits += iMostHeightLayoutUnits;

		iY += pLeader->getDocSectionLayout()->getSpaceAfter();
		iYLayoutUnits += pLeader->getDocSectionLayout()->getSpaceAfterInLayoutUnits();
	}
//	UT_ASSERT(i == count);
//
// Look for blank space to put more text
//
	fp_Column * pFirstOfNext = NULL;
	fp_Page *pNext = getNext();
	if(pNext && pLastCol)
	{
		fp_Line * pLastLine = pLastCol->getLastLine();
		if(pLastLine)
		{
			if(pLastLine->containsForcedPageBreak())
			{
				return;
			}
			pFirstOfNext = pNext->getNthColumnLeader(0);
			if(!pFirstOfNext)
			{
				return;
			}
			fp_Line *pFirstNextLine = pFirstOfNext->getFirstLine();
			if(pFirstNextLine == NULL)
			{
				return;
			}
			UT_sint32 iYLayoutNext = pFirstNextLine->getHeightInLayoutUnits();
			if( (iYLayoutUnits + 3*iYLayoutNext) < (getHeightInLayoutUnits() - iBottomMarginLayoutUnits))
			{
				UT_DEBUGMSG(("SEVIOR: Mark for rebuild to fill blank gap. iYLayoutUnits =%d iYnext = %d \n",iYLayoutUnits,iYLayoutNext));
				m_pOwner->markForRebuild();
				//UT_ASSERT(0);
			}
		}
	}
	return;
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
		m_pOwner->deleteOwnedPage(this);
		fl_DocSectionLayout * pDSLNew = pFirstColumnLeader->getDocSectionLayout();
//
// Now add it to the new DSL.
//
		pDSLNew->addOwnedPage(this);
		m_pOwner = pDSLNew;
	}
	_reformat();
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
				m_pOwner->deleteOwnedPage(this);
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

void fp_Page::columnHeightChanged(fp_Column* pCol)
{
	fp_Column* pLeader = pCol->getLeader();
	xxx_UT_DEBUGMSG(("SEVIOR: Column height changed \n"));
	UT_sint32 ndx = m_vecColumnLeaders.findItem(pLeader);
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
		fp_Line* pFirstLine = pColumn->getFirstLine();
		UT_ASSERT(pFirstLine);

		fp_Run* pFirstRun = pFirstLine->getFirstRun();
		fl_BlockLayout* pFirstBlock = pFirstLine->getBlock();

		pos = pFirstRun->getBlockOffset() + pFirstBlock->getPosition();
	}
	else
	{
		fp_Column* pColumn = getNthColumnLeader(cols-1);
		UT_ASSERT(pColumn);
		fp_Line* pLastLine = pColumn->getLastLine();
		UT_ASSERT(pLastLine);

		fp_Run* pLastRun = pLastLine->getLastRun();
		fl_BlockLayout* pLastBlock = pLastLine->getBlock();

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
void fp_Page::mapXYToPositionClick(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, fl_HdrFtrShadow *& pShadow, bool& bBOL, bool& bEOL)
{
	int count = m_vecColumnLeaders.getItemCount();
	UT_uint32 iMinDist = 0xffffffff;
	fp_Column* pMinDist = NULL;
	fp_Column* pColumn = NULL;
	UT_uint32 iMinXDist = 0xffffffff;
	fp_Column* pMinXDist = NULL;
	UT_uint32 iDist = 0;
	fp_Column* pLeader = NULL;
//
// Look in header for insertion point
//
	pShadow = NULL;
	if(m_pView && m_pView->getViewMode() == VIEW_PRINT)
	{
		if(m_pHeader != NULL)
		{
			if (m_pHeader->getFirstLine())
			{
				if (
					(x >= m_pHeader->getX())
					&& (x < (m_pHeader->getX() + m_pHeader->getWidth()))
					&& (y >= m_pHeader->getY())
					&& (y < (m_pHeader->getY() + m_pHeader->getHeight()))
					)
				{
					m_pHeader->mapXYToPosition(x - m_pHeader->getX(), y - m_pHeader->getY(), pos, bBOL, bEOL);
					pShadow = m_pHeader->getShadow();
					return;
				}
			}
		}
//
// Look in footer for insertion point
//
		if(m_pFooter != NULL)
		{
			if (m_pFooter->getFirstLine())
			{
				if (
					(x >= m_pFooter->getX())
					&& (x < (m_pFooter->getX() + m_pFooter->getWidth()))
					&& (y >= m_pFooter->getY())
					&& (y < (m_pFooter->getY() + m_pFooter->getHeight()))
					)
				{
					m_pFooter->mapXYToPosition(x - m_pFooter->getX(), y - m_pFooter->getY(), pos, bBOL, bEOL);
					pShadow = m_pFooter->getShadow();
					return;
				}
			}
		}
	}
//
// Now look in page
//
	for (int i=0; i<count; i++)
	{
		pLeader = (fp_Column*) m_vecColumnLeaders.getNthItem(i);

		pColumn = pLeader;
		iMinXDist = 0xffffffff;
		pMinXDist = NULL;
		while (pColumn)
		{
			if (pColumn->getFirstLine())
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
					pMinDist = pColumn;
				}
				
				if (
					(y >= pColumn->getY())
					&& (y < (pColumn->getY() + pColumn->getHeight()))
					)
				{
					if (iDist < iMinXDist)
					{
						iMinXDist = iDist;
						pMinXDist = pColumn;
					}
				}
			}
			
			pColumn = pColumn->getFollower();
		}

		if (pMinXDist)
		{
			pMinXDist->mapXYToPosition(x - pMinXDist->getX(), y - pMinXDist->getY(), pos, bBOL, bEOL);
			return;
		}
	}

	UT_ASSERT(pMinDist);

	pMinDist->mapXYToPosition(x - pMinDist->getX(), y - pMinDist->getY(), pos, bBOL, bEOL);
}


void fp_Page::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, bool& bBOL, bool& bEOL)
{
	int count = m_vecColumnLeaders.getItemCount();
	UT_uint32 iMinDist = 0xffffffff;
	fp_Column* pMinDist = NULL;
	fp_Column* pColumn = NULL;
	UT_uint32 iMinXDist = 0xffffffff;
	fp_Column* pMinXDist = NULL;
	UT_uint32 iDist = 0;
	fp_Column* pLeader = NULL;
//
// Now look in page
//
	for (int i=0; i<count; i++)
	{
		pLeader = (fp_Column*) m_vecColumnLeaders.getNthItem(i);

		pColumn = pLeader;
		iMinXDist = 0xffffffff;
		pMinXDist = NULL;
		while (pColumn)
		{
			if (pColumn->getFirstLine())
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
					pMinDist = pColumn;
				}
				
				if (
					(y >= pColumn->getY())
					&& (y < (pColumn->getY() + pColumn->getHeight()))
					)
				{
					if (iDist < iMinXDist)
					{
						iMinXDist = iDist;
						pMinXDist = pColumn;
					}
				}
			}
			
			pColumn = pColumn->getFollower();
		}

		if (pMinXDist)
		{
			pMinXDist->mapXYToPosition(x - pMinXDist->getX(), y - pMinXDist->getY(), pos, bBOL, bEOL);
			return;
		}
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
fp_ShadowContainer* fp_Page::getHdrFtrP(HdrFtrType hfType)
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

fp_ShadowContainer* fp_Page::getHeaderContainer(fl_HdrFtrSectionLayout* pHFSL)
{
	if (m_pHeader)
	{
		return m_pHeader;
	}

	// TODO fix these coordinates - Done!
	//
	// headerMargin is the height from the top of the page.
	//
	m_pHeader = new fp_ShadowContainer(m_pOwner->getLeftMargin(),
									   m_pOwner->getHeaderMargin(),
									   getWidth() - (m_pOwner->getLeftMargin() + m_pOwner->getRightMargin()),
									   m_pOwner->getTopMargin() - m_pOwner->getHeaderMargin(),
									   getWidthInLayoutUnits() - (m_pOwner->getLeftMarginInLayoutUnits() + m_pOwner->getRightMarginInLayoutUnits()),
									   m_pOwner->getTopMarginInLayoutUnits() - m_pOwner->getHeaderMarginInLayoutUnits(),
									   pHFSL);
	// TODO outofmem

	m_pHeader->setPage(this);

	return m_pHeader;
}


fp_ShadowContainer* fp_Page::buildHeaderContainer(fl_HdrFtrSectionLayout* pHFSL)
{
	if (m_pHeader)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		m_pHeader->getHdrFtrSectionLayout()->deletePage(this);
	}
	xxx_UT_DEBUGMSG(("SEVIOR: Building header container. page = %x hdrftr = %x \n",this,pHFSL)); 
	// TODO fix these coordinates - Done!
	//
	// headerMargin is the height from the top of the page.
	//
	m_pHeader = new fp_ShadowContainer(m_pOwner->getLeftMargin(),
									   m_pOwner->getHeaderMargin(),
									   getWidth() - (m_pOwner->getLeftMargin() + m_pOwner->getRightMargin()),
									   m_pOwner->getTopMargin() - m_pOwner->getHeaderMargin(),
									   getWidthInLayoutUnits() - (m_pOwner->getLeftMarginInLayoutUnits() + m_pOwner->getRightMarginInLayoutUnits()),
									   m_pOwner->getTopMarginInLayoutUnits() - m_pOwner->getHeaderMarginInLayoutUnits(),
									   pHFSL);
	// TODO outofmem

	m_pHeader->setPage(this);

	return m_pHeader;
}

fp_ShadowContainer* fp_Page::getFooterContainer(fl_HdrFtrSectionLayout* pHFSL)
{
	if (m_pFooter)
	{
		return m_pFooter;
	}

	// TODO fix these coordinates -Done !
	//
	// footerMargin is the distance from the bottom of the text to the
	// top of the footer
	//
	m_pFooter = new fp_ShadowContainer(m_pOwner->getLeftMargin(),
									   getHeight() - m_pOwner->getBottomMargin() + m_pOwner->getFooterMargin(),
									   getWidth() - (m_pOwner->getLeftMargin() + m_pOwner->getRightMargin()),
									   m_pOwner->getBottomMargin(),
									   getWidthInLayoutUnits() - (m_pOwner->getLeftMarginInLayoutUnits() + m_pOwner->getRightMarginInLayoutUnits()),
									   m_pOwner->getBottomMarginInLayoutUnits() - m_pOwner->getFooterMarginInLayoutUnits(),
									   pHFSL);
	// TODO outofmem

	m_pFooter->setPage(this);

	return m_pFooter;
}

fp_ShadowContainer* fp_Page::buildFooterContainer(fl_HdrFtrSectionLayout* pHFSL)
{
	if (m_pFooter)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		m_pHeader->getHdrFtrSectionLayout()->deletePage(this);
	}

	// TODO fix these coordinates -Done !
	//
	// footerMargin is the distance from the bottom of the text to the
	// top of the footer
	//
	m_pFooter = new fp_ShadowContainer(m_pOwner->getLeftMargin(),
									   getHeight() - m_pOwner->getBottomMargin() + m_pOwner->getFooterMargin(),
									   getWidth() - (m_pOwner->getLeftMargin() + m_pOwner->getRightMargin()),
									   m_pOwner->getBottomMargin(),
									   getWidthInLayoutUnits() - (m_pOwner->getLeftMarginInLayoutUnits() + m_pOwner->getRightMarginInLayoutUnits()),
									   m_pOwner->getBottomMarginInLayoutUnits() - m_pOwner->getFooterMarginInLayoutUnits(),
									   pHFSL);
	// TODO outofmem

	m_pFooter->setPage(this);

	return m_pFooter;
}
