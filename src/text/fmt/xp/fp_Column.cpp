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
fp_Container::fp_Container(UT_uint32 iType, fl_SectionLayout* pSectionLayout)
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
			m_pSectionLayout(pSectionLayout)
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
 \bug This function does not appear to have any use as it asserts if
      the height of the container is ever attempted changed. 
 */
void fp_Container::setHeight(UT_sint32 iHeight)
{
	if (iHeight == m_iHeight)
	{
		return;
	}
	
	m_iHeight = iHeight;

	// what do we do here?  does it ever happen?
	UT_ASSERT(UT_NOT_IMPLEMENTED);
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
        UT_RGBColor clrShowPara(127,127,127);
        m_pG->setColor(clrShowPara);
        UT_sint32 xoffBegin = pDA->xoff - 1;
        UT_sint32 yoffBegin = pDA->yoff - 1;
        UT_sint32 xoffEnd = pDA->xoff + m_iWidth + 2;
        UT_sint32 yoffEnd = pDA->yoff + m_iMaxHeight + 2;

        m_pG->drawLine(xoffBegin, yoffBegin, xoffEnd, yoffBegin);
        m_pG->drawLine(xoffBegin, yoffEnd, xoffEnd, yoffEnd);
        m_pG->drawLine(xoffBegin, yoffBegin, xoffBegin, yoffEnd);
        m_pG->drawLine(xoffEnd, yoffBegin, xoffEnd, yoffEnd);
    }
}

/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_Container::draw(dg_DrawArgs* pDA)
{
	int count = m_vecLines.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

		dg_DrawArgs da = *pDA;
		da.xoff += pLine->getX();
		da.yoff += pLine->getY();
		pLine->draw(&da);
	}

    _drawBoundaries(pDA);

#if 0
	m_pG->drawLine(pDA->xoff, pDA->yoff, pDA->xoff + m_iWidth, pDA->yoff);
	m_pG->drawLine(pDA->xoff + m_iWidth, pDA->yoff, pDA->xoff + m_iWidth, pDA->yoff + m_iMaxHeight);
	m_pG->drawLine(pDA->xoff + m_iWidth, pDA->yoff + m_iMaxHeight, pDA->xoff, pDA->yoff + m_iMaxHeight);
	m_pG->drawLine(pDA->xoff, pDA->yoff + m_iMaxHeight, pDA->xoff, pDA->yoff);
#endif	
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
	
	for (int i = 0; i<count; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

		// when hit testing lines within a column, we ignore the X coord.
//		if (((x - pLine->getX()) >= 0) && ((x - pLine->getX()) < (pLine->getWidth())))
		{
			if (((y - pLine->getY()) >= 0) && ((y - pLine->getY()) < (UT_sint32)(pLine->getHeight())))
			{
				pLine->mapXYToPosition(x - pLine->getX(), y - pLine->getY(), pos, bBOL, bEOL);
				return;
			}
		}

	    /*
		  Be careful with the comparisons of signed vs. unsigned
		  below.  (bug 172 resulted from that kind of problem.)
		*/
		
		if ((i + 1) < count)
		{
			if (y >= (pLine->getY() + (UT_sint32) pLine->getHeight()))
			{
				fp_Line* pLine2 = (fp_Line*) m_vecLines.getNthItem(i+1);
				if (y < (pLine2->getY()))
				{
					/*
					  The point is between these two lines.  Pick one.
					*/

					if ((pLine2->getY() - y) < (y - (pLine->getY() + (UT_sint32) pLine->getHeight())))
					{
						pLine2->mapXYToPosition(x - pLine2->getX(), y - pLine2->getY(), pos, bBOL, bEOL);
					}
					else
					{
						pLine->mapXYToPosition(x - pLine->getX(), y - pLine->getY(), pos, bBOL, bEOL);
					}
					return;
				}
			}
		}

		// TODO it might be better to move these special cases outside the loop
		if ((i == 0) && (y < pLine->getY()))
		{
			pLine->mapXYToPosition(x - pLine->getX(), y - pLine->getY(), pos, bBOL, bEOL);
			return;
		}
		
		if ((i == (count-1)) && (y >= (pLine->getY() + (UT_sint32)pLine->getHeight())))
		{
			pLine->mapXYToPosition(x - pLine->getX(), y - pLine->getY(), pos, bBOL, bEOL);
			return;
		}
	}

	// TODO pick the closest line
	UT_ASSERT(UT_NOT_IMPLEMENTED);
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

	UT_uint32 dist = (UT_uint32) (sqrt((dx * dx) + (dy * dy)));

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

fp_Column::fp_Column(fl_SectionLayout* pSectionLayout) : fp_Container(FP_CONTAINER_COLUMN, pSectionLayout)
{
	m_pNext = NULL;
	m_pPrev = NULL;

	m_pLeader = NULL;
	m_pNextFollower = NULL;
}

fp_Column::~fp_Column()
{

}

void fp_Column::setLeader(fp_Column* p)
{
	m_pLeader = p;
}

void fp_Column::setFollower(fp_Column* p)
{
	m_pNextFollower = p;
}

void fp_Column::setNext(fp_Column*p)
{
	m_pNext = p;
}

void fp_Column::setPrev(fp_Column*p)
{
	m_pPrev = p;
}

void fp_Column::layout(void)
{
	UT_sint32 iYLayoutUnits = 0;
	double ScaleLayoutUnitsToScreen;
	ScaleLayoutUnitsToScreen = (double)m_pG->getResolution() / UT_LAYOUT_UNITS;
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
	if (m_iHeight == iNewHeight)
	{
		return;
	}

	m_iHeight = iNewHeight;
	m_iHeightLayoutUnits = iYLayoutUnits;
	
	m_pPage->columnHeightChanged(this);
}

void fp_Column::bumpLines(fp_Line* pLastLineToKeep)
{
	UT_sint32 ndx = m_vecLines.findItem(pLastLineToKeep);
	UT_ASSERT(ndx >= 0);
	UT_sint32 iCount = m_vecLines.getItemCount();
	UT_sint32 i;

	fp_Column* pNextColumn = getNext();
	UT_ASSERT(pNextColumn);

	if (pNextColumn->isEmpty())
	{
		for (i=ndx + 1; i<iCount; i++)
		{
			fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);

			pNextColumn->addLine(pLine);
		}
	}
	else
	{
		for (i=iCount - 1; i>=(ndx+1); i--)
		{
			fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);
		
			pNextColumn->insertLine(pLine);
		}
	}
	
	for (i=iCount - 1; i > ndx; i--)
	{
		m_vecLines.deleteNthItem(i);
	}
}

fl_DocSectionLayout* fp_Column::getDocSectionLayout(void) const
{
	UT_ASSERT(m_pSectionLayout->getType() == FL_SECTION_DOC);

	return (fl_DocSectionLayout*) m_pSectionLayout;
}

/*!
 * This container is actually to display HdrFtrShadows which are repeated
 * for every page in the document. If the text is too high it is clipped to
 * to fit in the container. It's up to the user to adjust the height of the 
 * header/footer region to fit the text.
 */
fp_HdrFtrContainer::fp_HdrFtrContainer(UT_sint32 iX,
									   UT_sint32 iY,
									   UT_sint32 iWidth,
									   UT_sint32 iHeight,
									   UT_sint32 iWidthLayout,
									   UT_sint32 iHeightLayout,
									   fl_SectionLayout* pSectionLayout) 
	: fp_Container(FP_CONTAINER_HDRFTR, pSectionLayout)
{
	m_iX = iX;
	m_iY = iY;
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iWidthLayoutUnits = iWidthLayout;
	m_iHeightLayoutUnits = iHeightLayout;
	m_iMaxHeight = m_iHeight;
	m_iMaxHeightLayoutUnits = (UT_sint32)(  (double) m_iHeight * ( (double) iHeightLayout/ (double) m_iMaxHeight)) ;
   m_bHdrFtrBoxDrawn = false;
}

fp_HdrFtrContainer::~fp_HdrFtrContainer()
{
}

void fp_HdrFtrContainer::layout(void)
{
	double ScaleLayoutUnitsToScreen;
	double yHardOffset = 5.0; // Move 5 pixels away from the very edge 
	ScaleLayoutUnitsToScreen = (double)m_pG->getResolution() / UT_LAYOUT_UNITS;
	UT_sint32 iYLayoutUnits = (UT_sint32) (yHardOffset/	ScaleLayoutUnitsToScreen);
	UT_uint32 iCountLines = m_vecLines.getItemCount();
	
	for (UT_uint32 i=0; i < iCountLines; i++)
	{
		fp_Line* pLine = (fp_Line*) m_vecLines.getNthItem(i);
		
		UT_sint32 iLineHeightLayoutUnits = pLine->getHeightInLayoutUnits();
		UT_sint32 iLineMarginAfterLayoutUnits = pLine->getMarginAfterInLayoutUnits();
		UT_sint32 sum = iLineHeightLayoutUnits + iLineMarginAfterLayoutUnits;
		if((iYLayoutUnits + sum) <= (m_iMaxHeightLayoutUnits))
		{
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
	if (m_iHeight == iNewHeight)
	{
		return;
	}
	if(iYLayoutUnits <= m_iMaxHeightLayoutUnits)
	{
		m_iHeight = iNewHeight;
		m_iHeightLayoutUnits = iYLayoutUnits;
	}
	
}

/*!
 *    get the shadow associated with this hdrftrContainer
 */

fl_HdrFtrShadow * fp_HdrFtrContainer::getShadow(void)
{
    fl_HdrFtrSectionLayout* pHdrFtrSL = getHdrFtrSectionLayout();
	return  pHdrFtrSL->findShadow( getPage() );
}

fl_HdrFtrSectionLayout* fp_HdrFtrContainer::getHdrFtrSectionLayout(void) const
{
	UT_ASSERT(m_pSectionLayout->getType() == FL_SECTION_HDRFTR);

	return (fl_HdrFtrSectionLayout*) m_pSectionLayout;
}


/*!
  Clear container content from screen.
*/
void fp_HdrFtrContainer::clearScreen(void)
{
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

void fp_HdrFtrContainer::draw(dg_DrawArgs* pDA)
{
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
		if(iY > m_iMaxHeightLayoutUnits)
			break;
		pLine->draw(&da);
	}
	FV_View * pView = m_pPage->getDocLayout()->getView();
    if(pView && pView->isHdrFtrEdit() && m_pG->queryProperties(GR_Graphics::DGP_SCREEN) && pView->getEditShadow() == getShadow())
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
void fp_HdrFtrContainer::_drawHdrFtrBoundaries(dg_DrawArgs * pDA)
{
    UT_ASSERT(pDA->pG == m_pG);
//
// Can put this in to speed things up.
//
//	if(m_bHdrFtrBoxDrawn)
//		return;
	UT_RGBColor clrDrawHdrFtr(0,0,0);
	m_pG->setLineWidth(1);
	m_pG->setColor(clrDrawHdrFtr);
//
// These magic numbers stop clearscreens from blanking the lines
//
	m_ixoffBegin = pDA->xoff-2; 
	m_iyoffBegin = pDA->yoff+2;
	m_ixoffEnd = pDA->xoff + m_iWidth +1;
	m_iyoffEnd = pDA->yoff + m_iMaxHeight -1;

	m_pG->drawLine(m_ixoffBegin, m_iyoffBegin, m_ixoffEnd, m_iyoffBegin);
	m_pG->drawLine(m_ixoffBegin, m_iyoffEnd, m_ixoffEnd, m_iyoffEnd);
	m_pG->drawLine(m_ixoffBegin, m_iyoffBegin, m_ixoffBegin, m_iyoffEnd);
	m_pG->drawLine(m_ixoffEnd, m_iyoffBegin, m_ixoffEnd, m_iyoffEnd);
	m_pG->setLineWidth(1);
    m_bHdrFtrBoxDrawn = true;
}


/*! 
 * This method clears the solid box around the curently editted Header/Footer
 */
void fp_HdrFtrContainer::clearHdrFtrBoundaries(void)
{
	if(!m_bHdrFtrBoxDrawn)
		return;
	UT_RGBColor * pClr = getPage()->getOwningSection()->getPaperColor();
	m_pG->setLineWidth(1);
	m_pG->setColor(*pClr);
//
// Paint over the previous lines with the page color
//
	m_pG->drawLine(m_ixoffBegin, m_iyoffBegin, m_ixoffEnd, m_iyoffBegin);
	m_pG->drawLine(m_ixoffBegin, m_iyoffEnd, m_ixoffEnd, m_iyoffEnd);
	m_pG->drawLine(m_ixoffBegin, m_iyoffBegin, m_ixoffBegin, m_iyoffEnd);
	m_pG->drawLine(m_ixoffEnd, m_iyoffBegin, m_ixoffEnd, m_iyoffEnd);
	m_pG->setLineWidth(1);
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

fp_VirtualContainer::fp_VirtualContainer( UT_sint32 iWidth,
									   UT_sint32 iWidthLayout,
									   fl_SectionLayout* pSectionLayout) 
	: fp_Container(FP_CONTAINER_VIRTUAL, pSectionLayout)
{
	m_iX = 0;
	m_iY = 0;
	m_iWidth = iWidth;
	m_iHeight = 0;
	m_iWidthLayoutUnits = iWidthLayout;
	m_iHeightLayoutUnits = 0;
	m_pPage = NULL; // Never a page associated with this
}

fp_VirtualContainer::~fp_VirtualContainer()
{
}

/*!
 * Overloaded layout for VirtualCOntainer. We don't care about the height or
 * of the container.
 */

void fp_VirtualContainer::layout(void)
{
	UT_sint32 iYLayoutUnits = 0;
	double ScaleLayoutUnitsToScreen;
	ScaleLayoutUnitsToScreen = (double)m_pG->getResolution() / UT_LAYOUT_UNITS;
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
	if (m_iHeight == iNewHeight)
	{
		return;
	}

	m_iHeight = iNewHeight;
	m_iHeightLayoutUnits = iYLayoutUnits;
	
}

/*!
 * Returns a pointer to the HdrFtrSectionLayout that owns this container
 */
fl_HdrFtrSectionLayout* fp_VirtualContainer::getHdrFtrSectionLayout(void) const
{
	UT_ASSERT(m_pSectionLayout->getType() == FL_SECTION_HDRFTR);

	return (fl_HdrFtrSectionLayout*) m_pSectionLayout;
}


/*!
  NOP's for clear screen.
*/
void fp_VirtualContainer::clearScreen(void)
{

}

/*!
 NOP for Draw's
 \param pDA Draw arguments
 */
void fp_VirtualContainer::draw(dg_DrawArgs* pDA)
{

}




