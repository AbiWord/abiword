/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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
#include <string.h>

#include "fp_Column.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fv_View.h"
#include "fl_SectionLayout.h"
#include "gr_DrawArgs.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"

/*!
  Create container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_Container::fp_Container(FP_ContainerType iType, fl_SectionLayout* pSectionLayout)
	:       m_iType(iType),
			m_pPage(0),
			m_iWidth(0),
			m_iWidthLayoutUnits(0),
			m_iHeight(0),
			m_iMaxHeight(0),
			m_iHeightLayoutUnits(0),
			m_iMaxHeightLayoutUnits(0),
			m_iX(0),
			m_iY(0),
			m_pSectionLayout(pSectionLayout),
			m_bIntentionallyEmpty(0),
			m_imaxLineHeight(0)
{
	UT_ASSERT(pSectionLayout);
	m_pG = m_pSectionLayout->getDocLayout()->getGraphics();
}

/*!
  Destruct container
  \note The lines (fp_Line) in the container are not
        destructed. They are owned by the logical hierarchy (i.e.,
		fl_BlockLayout), not the physical hierarchy.
 */
fp_Container::~fp_Container()
{
}

/*!
  Set page
  \param pPage Page container is located on
 */
void fp_Container::setPage(fp_Page* pPage)
{
	m_pPage = pPage;
}

/*!
  Set width
  \param iWidth Width of container
  \todo Should force re-line-break operations on all blocks in the
        container
 */
void fp_Container::setWidth(UT_sint32 iWidth)
{
	if (iWidth == m_iWidth)
	{
		return;
	}
	m_iWidth = iWidth;

	// TODO we really need to force a re-line-break operation on every block herein

//	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

/*!
 Set width in layout units
 \param iWidth Width in layout units of container
 */
void fp_Container::setWidthInLayoutUnits(UT_sint32 iWidth)
{
	m_iWidthLayoutUnits = iWidth;
}

/*!
 Set height
 \param iHeight Height of container
 */
void fp_Container::setHeight(UT_sint32 iHeight)
{
	if (iHeight == m_iHeight)
	{
		return;
	}
	
	m_iHeight = iHeight;
}

/*!
 Set maximum height
 \param iMaxHeight Maximum height of container
 */
void fp_Container::setMaxHeight(UT_sint32 iMaxHeight)
{
	UT_ASSERT(iMaxHeight > 0);

	if (iMaxHeight == m_iMaxHeight)
	{
		return;
	}
	
	m_iMaxHeight = iMaxHeight;
}

/*!
 Set maximum height in layout units
 \param iMaxHeight Maximum height in layout units of container
 */
void fp_Container::setMaxHeightInLayoutUnits(UT_sint32 iMaxHeight)
{
	UT_ASSERT(iMaxHeight > 0);

	if (iMaxHeight == m_iMaxHeightLayoutUnits)
	{
		return;
	}
	
	m_iMaxHeightLayoutUnits = iMaxHeight;
}

/*!
  Get container's X position
  \return X position
*/
UT_sint32 fp_Container::getX(void) const
{
	return m_iX;
}

/*!
  Get container's Y position.
  \return Y position
*/
UT_sint32 fp_Container::getY(void) const
{
	if(getSectionLayout()->getDocLayout()->getView()  && (getSectionLayout()->getDocLayout()->getView()->getViewMode() != VIEW_PRINT))
	{
		return m_iY - static_cast<fl_DocSectionLayout *>(getSectionLayout())->getTopMargin();
	}
	return m_iY;
}

/*!
  Get line's offsets relative to this container
 \param  pLine Line
 \retval xoff Line's X offset relative to container
 \retval yoff Line's Y offset relative to container
 */
void fp_Container::getOffsets(fp_Line* pLine, UT_sint32& xoff, UT_sint32& yoff)
{
	xoff = getX() + pLine->getX();
	yoff = getY() + pLine->getY();
}

/*!
  Get line's offsets relative to the screen
 \param  pLine Line
 \retval xoff Line's X offset relative the screen
 \retval yoff Line's Y offset relative the screen
 */
void fp_Container::getScreenOffsets(fp_Line* pLine,
									UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pPage->getScreenOffsets(this, my_xoff, my_yoff);
	
	xoff = my_xoff + pLine->getX();
	yoff = my_yoff + pLine->getY();
}

/*!
 Remove line from container
 \param pLine Line
 \note The line is not destructed, as it is owned by the logical
       hierarchy.
 */
void fp_Container::removeLine(fp_Line* pLine)
{
	UT_sint32 iCount = m_vecLines.getItemCount();
	if(iCount == 0)
		return;
	UT_sint32 ndx = m_vecLines.findItem(pLine);
	UT_ASSERT(ndx >= 0);

	m_vecLines.deleteNthItem(ndx);

	// don't delete the line here, it's deleted elsewhere.
}

/*!
 Insert line at the front/top of the container
 \param pNewLine Line
 */
bool fp_Container::insertLine(fp_Line* pNewLine)
{
	m_vecLines.insertItemAt(pNewLine, 0);
	pNewLine->setContainer(this);
	pNewLine->recalcMaxWidth();

	return true;
}

/*!
 Append line at the end/bottom of the container
 \param pNewLine Line
 */
bool fp_Container::addLine(fp_Line* pNewLine)
{
	m_vecLines.addItem(pNewLine);
	pNewLine->setContainer(this);
	pNewLine->recalcMaxWidth();

	return true;
}

/*!
 Insert line in container after specified line
 \param pNewLine   Line to be inserted
 \param pAfterLine After this line
 \todo This function has been hacked to handle the case where
       pAfterLine is NULL. That case should not happen. Bad callers
       should be identified and fixed, and this function should be
       cleaned up.
 */
bool fp_Container::insertLineAfter(fp_Line*	pNewLine, fp_Line*	pAfterLine)
{
	UT_ASSERT(pAfterLine);
	UT_ASSERT(pNewLine);
	
	UT_sint32 count = m_vecLines.getItemCount();
	UT_sint32 ndx = m_vecLines.findItem(pAfterLine);
	UT_ASSERT( (count > 0) || (ndx == -1) );
	
	/*
	  TODO this routine should not be allowing pAfterLine to be NULL.
	  Right now, we've fixed the symptom, but we really should fix
	  the problem.  */
	UT_ASSERT(ndx >= 0);

	if ( (ndx+1) == count )				// append after last line in vector
		m_vecLines.addItem(pNewLine);
	else if (ndx >= 0)					// append after this item within the vector
		m_vecLines.insertItemAt(pNewLine, ndx+1);
	else
	{
		// TODO remove this....
		m_vecLines.insertItemAt(pNewLine, 0);
	}

	pNewLine->setContainer(this);
	pNewLine->recalcMaxWidth();

	return true;
}

/*!
  Determine if container is empty
 \return True if container is empty, otherwise false.
 */
bool fp_Container::isEmpty(void) const
{
	return (m_vecLines.getItemCount() == 0);
}

/*!
  Clear container content from screen.

  \fixme Needs to clear outline as well
*/
void fp_Container::clearScreen(void)
{
	int count = m_vecLines.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

		pLine->clearScreen();
	}
}

/*!
 Draw container outline
 \param pDA Draw arguments
 */
void fp_Container::_drawBoundaries(dg_DrawArgs* pDA)
{
    UT_ASSERT(pDA->pG == m_pG);
    if(m_pPage->getDocLayout()->getView()->getShowPara() && m_pG->queryProperties(GR_Graphics::DGP_SCREEN)){
        UT_sint32 xoffBegin = pDA->xoff - 1;
        UT_sint32 yoffBegin = pDA->yoff - 1;
        UT_sint32 xoffEnd = pDA->xoff + m_iWidth + 2;
        UT_sint32 yoffEnd = pDA->yoff + m_iMaxHeight + 2;

		UT_RGBColor clrShowPara(127,127,127);
		m_pG->setColor(clrShowPara);

        m_pG->drawLine(xoffBegin, yoffBegin, xoffEnd, yoffBegin);
        m_pG->drawLine(xoffBegin, yoffEnd, xoffEnd, yoffEnd);
        m_pG->drawLine(xoffBegin, yoffBegin, xoffBegin, yoffEnd);
        m_pG->drawLine(xoffEnd, yoffBegin, xoffEnd, yoffEnd);
    }
}

/*!
 * Returns the maximum line height as determined from the layout method
 * This used by the draw method to determine if a line should be drawn in
 * a clipping rectangle
 */
UT_sint32 fp_Container::getMaxLineHeight(void) const
{
	return m_imaxLineHeight;
}

/*!
 * Set the maximum line Height
\params UT_sint32 iLineHeight the largest line height yet found.
 */
void fp_Container::setMaxLineHeight( UT_sint32 iLineHeight)
{
	m_imaxLineHeight = iLineHeight;
}

/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_Container::draw(dg_DrawArgs* pDA)
{
	UT_sint32 count = m_vecLines.getItemCount();
	const UT_Rect * pClipRect = pDA->pG->getClipRect();
	UT_sint32 ytop,ybot;
	UT_sint32 i;
	UT_sint32 imax = (UT_sint32)(((UT_uint32)(1<<31)) - 1);
	if(pClipRect)
	{
		ybot = UT_MAX(pClipRect->height,getMaxLineHeight());
		ytop = pClipRect->top;
        ybot += ytop + 1;
	}
	else
	{
		ytop = 0;
		ybot = imax;
	}
	bool bStop = false;
	bool bStart = false;
//
// Only draw the lines in the clipping region.
//
	for ( i = 0; (i<count && !bStop); i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

		dg_DrawArgs da = *pDA;
		da.xoff += pLine->getX();
		da.yoff += pLine->getY();
		UT_sint32 ydiff = da.yoff + pLine->getHeight();
		if((da.yoff >= ytop && da.yoff <= ybot) || (ydiff >= ytop && ydiff <= ybot))
		{
			bStart = true;
			pLine->draw(&da);
		}
		else if(bStart)
		{
			bStop = true;
		}
	}

    _drawBoundaries(pDA);
}

/*!
  Find document position from X and Y coordinates
 \param  x X coordinate
 \param  y Y coordinate
 \retval pos Document position
 \retval bBOL True if position is at begining of line, otherwise false
 \retval bEOL True if position is at end of line, otherwise false
 */
void fp_Container::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos,
								   bool& bBOL, bool& bEOL)
{
	int count = m_vecLines.getItemCount();

	UT_ASSERT(count > 0);
	
	fp_Line* pLine = NULL;
	int i = 0;
	// Find first line that has its lower level below the desired Y
	// position. Note that X-positions are completely ignored here.
	do {
		pLine = (fp_Line*) m_vecLines.getNthItem(i++);
	} while ((i < count) 
			 && (y > (pLine->getY() + pLine->getHeight())));
	// Undo the postincrement.
	i--;
	// Now check if the position is actually between the found line
	// and the line before it (ignore check on the top-most line).
	if (i > 0 && y < pLine->getY())
	{
		fp_Line* pLineUpper = (fp_Line*) m_vecLines.getNthItem(i-1);

		// Be careful with the signedness here - bug 172 leared us a
		// lesson!
		
		// Now pick the line that is closest to the point - or the
		// upper if it's a stalemate.
		if ((pLine->getY() - y) >= (y - (pLineUpper->getY() + (UT_sint32) pLineUpper->getHeight())))
		{
			pLine = pLineUpper;
		}
	}

	pLine->mapXYToPosition(x - pLine->getX(), y - pLine->getY() - pLine->getHeight(), pos, bBOL, bEOL);
}

/*!
 Compute the distance from point to the container's circumference
 \param x X coordinate of point
 \param y Y coordinate of point
 \return Distance between container's circumference and point
 */
UT_uint32 fp_Container::distanceFromPoint(UT_sint32 x, UT_sint32 y)
{
	UT_sint32 dx;
	UT_sint32 dy;
	
	if (x < m_iX)
	{
		dx = m_iX - x;
	}
	else if (x > (m_iX + m_iWidth - 1))
	{
		dx = x - (m_iX + m_iWidth - 1);
	}
	else
	{
		dx = 0;
	}

	if (y < m_iY)
	{
		dy = m_iY - y;
	}
	else if (y > (m_iY + m_iHeight - 1))
	{
		dy = y - (m_iY + m_iHeight - 1);
	}
	else
	{
		dy = 0;
	}

	if (dx == 0)
	{
		return dy;
	}

	if (dy == 0)
	{
		return dx;
	}

	UT_uint32 dist = (UT_uint32) (sqrt((float)(dx * dx) + (dy * dy)));

	UT_ASSERT(dist > 0);
	
	return dist;
}

/*!
 Set X position of container
 \param iX New X position

 Before the postition of the container is changed, its content is
 first cleared from the screen.
 */
void fp_Container::setX(UT_sint32 iX)
{
	if (iX == m_iX)
	{
		return;
	}
	
	clearScreen();
	
	m_iX = iX;
}

/*!
 Set Y position of container
 \param iY New Y position

 Before the postition of the container is changed, its content is
 first cleared from the screen.
 */
void fp_Container::setY(UT_sint32 iY)
{
	if (iY == m_iY)
	{
		return;
	}

	clearScreen();
	
	m_iY = iY;
}

/*!
 Return first line in the container
 \return The first line, or NULL if the container is empty
 */
fp_Line* fp_Container::getFirstLine(void) const
{
	if (m_vecLines.getItemCount() > 0)
	{
		return (fp_Line*) m_vecLines.getNthItem(0);
	}
	else
	{
		return NULL;
	}
}

/*!
 Return last line in the container
 \return The last line, or NULL if the container is empty
 */
fp_Line* fp_Container::getLastLine(void) const
{
	UT_uint32 iCount = m_vecLines.getItemCount();
	
	if (iCount > 0)
	{
		return (fp_Line*) m_vecLines.getNthItem(iCount - 1);
	}
	else
	{
		return NULL;
	}
}

/*!
  Create column
  \param pSectionLayout Section layout type used for this container

  The section the column is created in specifies the number of column
  rows. There is always created columns for all rows at the same
  time. The first (left-most) column is the leader.

  \fixme I suspect BIDI does not work with multiple columns since the
         leader would then have to be the right-most column.
*/
fp_Column::fp_Column(fl_SectionLayout* pSectionLayout) : fp_Container(FP_CONTAINER_COLUMN, pSectionLayout)
{
	m_pNext = NULL;
	m_pPrev = NULL;

	m_pLeader = NULL;
	m_pFollower = NULL;
}

fp_Column::~fp_Column()
{

}

/*!
 Draw column outline
 \param pDA Draw arguments

 This differs from the container function in that it will use draw the
 outline based on the tallest column in the row. 
*/
void fp_Column::_drawBoundaries(dg_DrawArgs* pDA)
{
    UT_ASSERT(pDA->pG == getGraphics());
    if(getPage()->getDocLayout()->getView()->getShowPara() && getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
    {
        UT_RGBColor clrShowPara(127,127,127);
        getGraphics()->setColor(clrShowPara);
        UT_sint32 xoffBegin = pDA->xoff - 1;
        UT_sint32 yoffBegin = pDA->yoff - 1;
        UT_sint32 xoffEnd = pDA->xoff + getWidth() + 2;

        UT_sint32 iHeight = 0;
		fp_Column* pCol = getLeader();
		if (getPage()->getNthColumnLeader(getPage()->countColumnLeaders()-1) == pCol)
		{
			// If there's no column rows after this one on the page, use max height
			iHeight = getMaxHeight();
		}
		else
		{
			// Find max column height in row
			while (pCol)
			{
				if (pCol->getHeight() > iHeight)
					iHeight = pCol->getHeight();
				pCol = pCol->getFollower();
			}
		}
		UT_sint32 yoffEnd = pDA->yoff + iHeight + 2;

       	getGraphics()->drawLine(xoffBegin, yoffBegin, xoffEnd, yoffBegin);
		getGraphics()->drawLine(xoffBegin, yoffEnd, xoffEnd, yoffEnd);
        getGraphics()->drawLine(xoffBegin, yoffBegin, xoffBegin, yoffEnd);
        getGraphics()->drawLine(xoffEnd, yoffBegin, xoffEnd, yoffEnd);
    }
}

/*!
  Layout lines in the column

  This function iterates over the lines in the column and computes
  their screen position from their accumulative heights in layout
  units. 

  Since this code accumulates fractions of the conversion process, the
  difference between Y positions of two lines may differ from the
  pre-computed height of the upper line. This is due to simple
  rounding errors and general lack of precision (screen coordinates
  are integer while the computation yields fractions).

  To make XY/position conversion precise, remove the gaps by updating
  the line heights. Note that the line heights get updated next time
  there's a line lookup - so this does not in any way affect layout,
  only the precision of the XY/position conversion code.

  Sevior: I put in the 0.5 to deal with truncation errors and the +1 to deal
  with the last line.

  \see fp_Line::setAssignedScreenHeight, fp_Line::recalcHeight
*/
void fp_Column::layout(void)
{
	setMaxLineHeight(0);
	UT_sint32 iYLayoutUnits = 0;
	UT_sint32 iY = 0, iPrevY = 0;
	double ScaleLayoutUnitsToScreen;
	ScaleLayoutUnitsToScreen = (double)getGraphics()->getResolution() / UT_LAYOUT_UNITS;
	UT_uint32 iCountLines = m_vecLines.getItemCount();
	fp_Line *pLine, *pPrevLine = NULL;
	long imax = (1<<30) -1;

	for (UT_uint32 i=0; i < iCountLines; i++)
	{
		pLine = (fp_Line*) m_vecLines.getNthItem(i);
//
// This is to speedup redraws.
//
		if(pLine->getHeight() > getMaxLineHeight())
			setMaxLineHeight(pLine->getHeight());

		UT_sint32 iLineHeightLayoutUnits = pLine->getHeightInLayoutUnits();
//		UT_sint32 iLineMarginBefore = (i != 0) ? pLine->getMarginBefore() : 0;
		UT_sint32 iLineMarginAfterLayoutUnits = pLine->getMarginAfterInLayoutUnits();

//		iY += iLineMarginBefore;
		iY = (int)(ScaleLayoutUnitsToScreen * iYLayoutUnits + 0.5);
		if(pLine->getY() != iY)
		{
			pLine->clearScreen();
		}
		pLine->setY(iY);
		pLine->setYInLayoutUnits(iYLayoutUnits);

		iYLayoutUnits += iLineHeightLayoutUnits;
		iYLayoutUnits += iLineMarginAfterLayoutUnits;
		if((long) iYLayoutUnits > imax)
		{
		       UT_ASSERT(0);
		}
		// Update height of previous line now we know the gap between
		// it and the current line. 

		if (pPrevLine)
		{
			pPrevLine->setAssignedScreenHeight(iY - iPrevY);
		}
		pPrevLine = pLine;
		iPrevY = iY;
	}

	// Correct height position of the last line
	if (pPrevLine)
	{
		iY = (int)(ScaleLayoutUnitsToScreen * iYLayoutUnits +0.5);
		pPrevLine->setAssignedScreenHeight(iY - iPrevY + 1);
	}

	UT_sint32 iNewHeight = (int)(ScaleLayoutUnitsToScreen * iYLayoutUnits);
	if (getHeight() == iNewHeight)
	{
		return;
	}

	setHeight(iNewHeight);
	setHeightLayoutUnits(iYLayoutUnits);
	
	getPage()->columnHeightChanged(this);
}

/*!
  Bump lines from this column to the next
  \param pLastLineToKeep Last line to keep in this column or NULL for none
*/
void fp_Column::bumpLines(fp_Line* pLastLineToKeep)
{
	UT_sint32 ndx = (NULL == pLastLineToKeep) ? 0 : (m_vecLines.findItem(pLastLineToKeep)+1);
	UT_ASSERT(ndx >= 0);
	UT_sint32 iCount = m_vecLines.getItemCount();
	UT_sint32 i;

	fp_Column* pNextColumn = getNext();
	UT_ASSERT(pNextColumn);

	if (pNextColumn->isEmpty())
	{
		for (i=ndx; i<iCount; i++)
		{
			fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

			pNextColumn->addLine(pLine);
		}
	}
	else
	{
		for (i=iCount - 1; i >= ndx; i--)
		{
			fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);
		
			pNextColumn->insertLine(pLine);
		}
	}
	
	for (i=iCount - 1; i >= ndx; i--)
	{
		m_vecLines.deleteNthItem(i);
	}
}

fl_DocSectionLayout* fp_Column::getDocSectionLayout(void) const
{
	UT_ASSERT(getSectionLayout()->getType() == FL_SECTION_DOC ||
			  getSectionLayout()->getType() == FL_SECTION_ENDNOTE);

	return (fl_DocSectionLayout*) getSectionLayout();
}

/*!
 * This container is actually to display HdrFtrShadows which are repeated
 * for every page in the document. If the text is too high it is clipped to
 * to fit in the container. It's up to the user to adjust the height of the 
 * header/footer region to fit the text.
 */
fp_ShadowContainer::fp_ShadowContainer(UT_sint32 iX,
									   UT_sint32 iY,
									   UT_sint32 iWidth,
									   UT_sint32 iHeight,
									   UT_sint32 iWidthLayout,
									   UT_sint32 iHeightLayout,
									   fl_SectionLayout* pSectionLayout) 
	: fp_Container(FP_CONTAINER_SHADOW, pSectionLayout)
{
	_setX(iX);
	_setY(iY);
	setWidth(iWidth);
	setHeight(iHeight);
	setWidthInLayoutUnits(iWidthLayout);
	setHeightLayoutUnits(iHeightLayout);
	setMaxHeight(iHeight);
	setMaxHeightInLayoutUnits( (UT_sint32)(  (double) getHeight() * ( (double) iHeightLayout/ (double) getMaxHeight()))) ;
   m_bHdrFtrBoxDrawn = false;
}

fp_ShadowContainer::~fp_ShadowContainer()
{
}

void fp_ShadowContainer::layout(void)
{
	double ScaleLayoutUnitsToScreen;
	double yHardOffset = 5.0; // Move 5 pixels away from the very edge 
	ScaleLayoutUnitsToScreen = (double)getGraphics()->getResolution() / UT_LAYOUT_UNITS;
	UT_sint32 iYLayoutUnits = (UT_sint32) (yHardOffset/	ScaleLayoutUnitsToScreen);
	UT_uint32 iCountLines = m_vecLines.getItemCount();
	FV_View * pView = getPage()->getDocLayout()->getView();
	bool doLayout = true;
	if(pView)
	{
	    doLayout =	pView->getViewMode() == VIEW_PRINT;
	}
	for (UT_uint32 i=0; i < iCountLines; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);
		
		UT_sint32 iLineHeightLayoutUnits = pLine->getHeightInLayoutUnits();
		UT_sint32 iLineMarginAfterLayoutUnits = pLine->getMarginAfterInLayoutUnits();
		UT_sint32 sum = iLineHeightLayoutUnits + iLineMarginAfterLayoutUnits;
		if(((iYLayoutUnits + sum) <= (getMaxHeightInLayoutUnits())) && doLayout)
		{
//			if(pLine->getYInLayoutUnits() != iYLayoutUnits)
//				pLine->clearScreen();
			pLine->setY((UT_sint32)(ScaleLayoutUnitsToScreen * iYLayoutUnits));
			pLine->setYInLayoutUnits(iYLayoutUnits);
		}
		else
		{
//
// FIXME: Dirty hack to clip.
// 
			pLine->setY(-1000000);
			pLine->setYInLayoutUnits(-1000000);
		}
		iYLayoutUnits += iLineHeightLayoutUnits;
		iYLayoutUnits += iLineMarginAfterLayoutUnits;
	}

	UT_sint32 iNewHeight = (int)(ScaleLayoutUnitsToScreen * iYLayoutUnits);
	if (getHeight() == iNewHeight)
	{
		return;
	}
	if(iYLayoutUnits <= getMaxHeightInLayoutUnits())
	{
		setHeight(iNewHeight);
		setHeightLayoutUnits(iYLayoutUnits);
	}
	
}

/*!
 *    get the shadow associated with this hdrftrContainer
 */

fl_HdrFtrShadow * fp_ShadowContainer::getShadow(void)
{
    fl_HdrFtrSectionLayout* pHdrFtrSL = getHdrFtrSectionLayout();
	return  pHdrFtrSL->findShadow( getPage() );
}

fl_HdrFtrSectionLayout* fp_ShadowContainer::getHdrFtrSectionLayout(void) const
{
	UT_ASSERT(getSectionLayout()->getType() == FL_SECTION_HDRFTR);

	return (fl_HdrFtrSectionLayout*) getSectionLayout();
}


/*!
  Clear container content from screen.
*/
void fp_ShadowContainer::clearScreen(void)
{
	FV_View * pView = getPage()->getDocLayout()->getView();
	if(pView->getViewMode() !=  VIEW_PRINT)
	{
		UT_DEBUGMSG(("SEVIOR: Attempting to clear Header/Footer in Normal Mode \n"));
		return;
	}
	int count = m_vecLines.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

		pLine->clearScreen();
	}
	clearHdrFtrBoundaries();
}



/*!
 Draw container content
 \param pDA Draw arguments
 */

void fp_ShadowContainer::draw(dg_DrawArgs* pDA)
{
	FV_View * pView = getPage()->getDocLayout()->getView();
	if(pView->getViewMode() !=  VIEW_PRINT)
	{
		UT_DEBUGMSG(("SEVIOR: Attempting to draw Header/Footer in Normal Mode \n"));
		return;
	}
	UT_sint32 count = m_vecLines.getItemCount();
	UT_sint32 iY= 0;
	for (UT_sint32 i = 0; i<count; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

		dg_DrawArgs da = *pDA;
		da.xoff += pLine->getX();
		da.yoff += pLine->getY();
		
		UT_sint32 iLineHeightLayoutUnits = pLine->getHeightInLayoutUnits();
		UT_sint32 iLineMarginAfterLayoutUnits = pLine->getMarginAfterInLayoutUnits();
		iY += iLineHeightLayoutUnits;
		iY += iLineMarginAfterLayoutUnits;
//
// Clip to keep inside header/footer container
//
		if(iY > getMaxHeightInLayoutUnits())
			break;
		pLine->draw(&da);
	}
    if(pView && pView->isHdrFtrEdit() && getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN) && pView->getEditShadow() == getShadow())
	{
		_drawHdrFtrBoundaries(pDA);
	}
	else
	{
        clearHdrFtrBoundaries();
		_drawBoundaries(pDA);
	}
}

/*! 
 * This method draws a solid box around the currently editted Header/Footer
 */
void fp_ShadowContainer::_drawHdrFtrBoundaries(dg_DrawArgs * pDA)
{
    UT_ASSERT(pDA->pG == getGraphics());
	FV_View * pView = getPage()->getDocLayout()->getView();
	if(pView->getViewMode() !=  VIEW_PRINT)
	{
		UT_DEBUGMSG(("SEVIOR: Attempting to draw Header/Footer in Normal Mode \n"));
		return;
	}
//
// Can put this in to speed things up.
//
//	if(m_bHdrFtrBoxDrawn)
//		return;
	UT_RGBColor clrDrawHdrFtr(0,0,0);
	getGraphics()->setLineWidth(1);
	getGraphics()->setColor(clrDrawHdrFtr);
//
// These magic numbers stop clearscreens from blanking the lines
//
	m_ixoffBegin = pDA->xoff-2; 
	m_iyoffBegin = pDA->yoff+2;
	m_ixoffEnd = pDA->xoff + getWidth() +1;
	m_iyoffEnd = pDA->yoff + getMaxHeight() -1;

	getGraphics()->drawLine(m_ixoffBegin, m_iyoffBegin, m_ixoffEnd, m_iyoffBegin);
	getGraphics()->drawLine(m_ixoffBegin, m_iyoffEnd, m_ixoffEnd, m_iyoffEnd);
	getGraphics()->drawLine(m_ixoffBegin, m_iyoffBegin, m_ixoffBegin, m_iyoffEnd);
	getGraphics()->drawLine(m_ixoffEnd, m_iyoffBegin, m_ixoffEnd, m_iyoffEnd);
	getGraphics()->setLineWidth(1);
    m_bHdrFtrBoxDrawn = true;
}


/*! 
 * This method clears the solid box around the curently editted Header/Footer
 */
void fp_ShadowContainer::clearHdrFtrBoundaries(void)
{
	if(!m_bHdrFtrBoxDrawn)
		return;
	UT_RGBColor * pClr = getPage()->getOwningSection()->getPaperColor();
	getGraphics()->setLineWidth(1);
	getGraphics()->setColor(*pClr);
//
// Paint over the previous lines with the page color
//
	getGraphics()->drawLine(m_ixoffBegin, m_iyoffBegin, m_ixoffEnd, m_iyoffBegin);
	getGraphics()->drawLine(m_ixoffBegin, m_iyoffEnd, m_ixoffEnd, m_iyoffEnd);
	getGraphics()->drawLine(m_ixoffBegin, m_iyoffBegin, m_ixoffBegin, m_iyoffEnd);
	getGraphics()->drawLine(m_ixoffEnd, m_iyoffBegin, m_ixoffEnd, m_iyoffEnd);
	getGraphics()->setLineWidth(1);
	m_bHdrFtrBoxDrawn = false;
}

/*!
 * Ok this container class is for the hdrftrSectionLayout. It never gets drawn
 * on the screen, only the shadows get drawn. The page pointer contains a NULL.
 * This makes it possible to format the hdrftrSectionLayout and to do 
 * editting operations on header/footers like regular text.
\param iwidth width of the page in pixels?? I think.
\param IwidthLayout width of the screen in layout units
\param fl_SectionLayout * pSectionLayout pointer to the 
       fl_HdrFtrSectionLayout that owns this container.
*/

fp_HdrFtrContainer::fp_HdrFtrContainer(UT_sint32 iWidth,
									   UT_sint32 iWidthLayout,
									   fl_SectionLayout* pSectionLayout) 
	: fp_Container(FP_CONTAINER_HDRFTR, pSectionLayout)
{
	_setX(0);
	_setY(0);
	setWidth(iWidth);
	setHeight(0);
	setWidthInLayoutUnits(iWidthLayout);
	setHeightLayoutUnits(0);
	setPage(NULL); // Never a page associated with this
}

fp_HdrFtrContainer::~fp_HdrFtrContainer()
{
}

/*!
 * Overloaded layout for VirtualCOntainer. We don't care about the height or
 * of the container.
 */

void fp_HdrFtrContainer::layout(void)
{
	UT_sint32 iYLayoutUnits = 0;
	double ScaleLayoutUnitsToScreen;
	ScaleLayoutUnitsToScreen = (double)getGraphics()->getResolution() / UT_LAYOUT_UNITS;
	UT_uint32 iCountLines = m_vecLines.getItemCount();
	
	for (UT_uint32 i=0; i < iCountLines; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);
		
		UT_sint32 iLineHeightLayoutUnits = pLine->getHeightInLayoutUnits();
//		UT_sint32 iLineMarginBefore = (i != 0) ? pLine->getMarginBefore() : 0;
		UT_sint32 iLineMarginAfterLayoutUnits = pLine->getMarginAfterInLayoutUnits();

//		iY += iLineMarginBefore;
		pLine->setY((int)(ScaleLayoutUnitsToScreen * iYLayoutUnits));
		pLine->setYInLayoutUnits(iYLayoutUnits);
		iYLayoutUnits += iLineHeightLayoutUnits;
		iYLayoutUnits += iLineMarginAfterLayoutUnits;
	}

	UT_sint32 iNewHeight = (int)(ScaleLayoutUnitsToScreen * iYLayoutUnits);
	if (getHeight() == iNewHeight)
	{
		return;
	}

	setHeight(iNewHeight);
	setHeightLayoutUnits(iYLayoutUnits);
	
}

/*!
 * Returns a pointer to the HdrFtrSectionLayout that owns this container
 */
fl_HdrFtrSectionLayout* fp_HdrFtrContainer::getHdrFtrSectionLayout(void) const
{
	UT_ASSERT(getSectionLayout()->getType() == FL_SECTION_HDRFTR);

	return (fl_HdrFtrSectionLayout*) getSectionLayout();
}


/*!
  Get line's offsets relative to the screen for this method we just return 
  * -100000 since virtual containers are never drawn.
 \param  pLine Line
 \retval xoff Line's X offset relative the screen actually -10000
 \retval yoff Line's Y offset relative the screen actually -10000
 */
void fp_HdrFtrContainer::getScreenOffsets(fp_Line* pLine,
									UT_sint32& xoff, UT_sint32& yoff)
{
	xoff = -100000;
	yoff = -100000;
}


/*!
  NOP's for clear screen.
*/
void fp_HdrFtrContainer::clearScreen(void)
{

}

/*!
 NOP for Draw's
 \param pDA Draw arguments
 */
void fp_HdrFtrContainer::draw(dg_DrawArgs* pDA)
{

}
