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
#include "fp_TableContainer.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"

/*!
  Create container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_VerticalContainer::fp_VerticalContainer(FP_ContainerType iType, fl_SectionLayout* pSectionLayout) : fp_Container(iType, pSectionLayout),
												       m_iWidth(0),
			m_iHeight(0),
			m_iMaxHeight(0),
			m_iX(0),
			m_iY(0),
			m_bIntentionallyEmpty(0),
												       m_imaxContainerHeight(0)
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		   ,m_iWidthLayoutUnits(0),
			m_iHeightLayoutUnits(0),
			m_iMaxHeightLayoutUnits(0)
#endif
{
}

/*!
  Destruct container
  \note The Containers in vector of the container are not
        destructed. They are owned by the logical hierarchy (i.e.,
		the fl_Container classes like fl_BlockLayout), not the physical
        hierarchy.
 */
fp_VerticalContainer::~fp_VerticalContainer()
{
}

/*!
  Set width
  \param iWidth Width of container
  \todo Should force re-line-break operations on all blocks in the
        container
 */
void fp_VerticalContainer::setWidth(UT_sint32 iWidth)
{
	if (iWidth == m_iWidth)
	{
		return;
	}
	m_iWidth = iWidth;
	// TODO we really need to force a re-line-break operation on every block herein

//	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
/*!
 Set width in layout units
 \param iWidth Width in layout units of container
 */
void fp_VerticalContainer::setWidthInLayoutUnits(UT_sint32 iWidth)
{
	m_iWidthLayoutUnits = iWidth;
}
#endif

/*!
 Set height
 \param iHeight Height of container
 */
void fp_VerticalContainer::setHeight(UT_sint32 iHeight)
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
void fp_VerticalContainer::setMaxHeight(UT_sint32 iMaxHeight)
{
	UT_ASSERT(iMaxHeight > 0);

	if (iMaxHeight == m_iMaxHeight)
	{
		return;
	}

	m_iMaxHeight = iMaxHeight;
}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
/*!
 Set maximum height in layout units
 \param iMaxHeight Maximum height in layout units of container
 */
void fp_VerticalContainer::setMaxHeightInLayoutUnits(UT_sint32 iMaxHeight)
{
	UT_ASSERT(iMaxHeight > 0);

	if (iMaxHeight == m_iMaxHeightLayoutUnits)
	{
		return;
	}

	m_iMaxHeightLayoutUnits = iMaxHeight;
}
#endif

/*!
  Get container's X position
  \return X position
*/
UT_sint32 fp_VerticalContainer::getX(void) const
{
	return m_iX;
}

/*!
  Get container's Y position.
  \return Y position
*/
UT_sint32 fp_VerticalContainer::getY(void) const
{
	if(getSectionLayout()->getDocLayout()->getView()  && (getSectionLayout()->getDocLayout()->getView()->getViewMode() != VIEW_PRINT))
	{
		return m_iY - static_cast<fl_DocSectionLayout *>(getSectionLayout())->getTopMargin();
	}
	return m_iY;
}

/*!
 * This method returns the vertical offset due to a table broken
 * across more than 1 page.
 */
UT_sint32 fp_VerticalContainer::getYoffsetFromTable(fp_Container * pT,
													fp_Container* pCell,
													fp_ContainerObject * pCon)
{
	fp_TableContainer * pTable = static_cast<fp_TableContainer *>(pT)->getFirstBrokenTable();
//	UT_ASSERT(pTable);
	UT_sint32 offset = 0;
	bool bFound = false;
	while(pTable != NULL && !bFound)
	{
		bFound = pTable->isInBrokenTable((fp_CellContainer *) pCell,(fp_Container *) pCon);
		if(bFound)
		{
			offset = -pTable->getYBreak();
		}
		pTable = (fp_TableContainer *) pTable->getNext();
	}
	return offset;
}


/*!
 * This method returns the correct broken table for this line.
 */
fp_TableContainer * fp_VerticalContainer::getCorrectBrokenTable(fp_Container * pLine)
{
	bool bFound = false;
	UT_return_val_if_fail(pLine->getContainerType() == FP_CONTAINER_LINE, NULL);
	fp_CellContainer * pCell = (fp_CellContainer *) pLine->getContainer();
	if(!pCell)
	{
		return NULL;
	}
	UT_return_val_if_fail(pCell->getContainerType() == FP_CONTAINER_CELL,NULL);
	//
	// Recursively search for the table that contains this cell.
	//
	fp_Container * pCur = (fp_Container *)pCell;
	while(pCur->getContainer() && 
		  ( pCur->getContainer()->getContainerType() != FP_CONTAINER_COLUMN &&
			pCur->getContainer()->getContainerType() != FP_CONTAINER_COLUMN_SHADOW))
	{
		pCur = pCur->getContainer();
	}
	
	fp_TableContainer * pMasterTab = (fp_TableContainer *) pCur;
	UT_return_val_if_fail(pMasterTab && pMasterTab->getContainerType() == FP_CONTAINER_TABLE,NULL);
	fp_TableContainer * pTab = pMasterTab->getFirstBrokenTable();
	bFound = false;
	while(pTab && !bFound)
	{
		if(pTab->isInBrokenTable(pCell,pLine))
		{
			bFound = true;
		}
		else
		{
			pTab = (fp_TableContainer *) pTab->getNext();
		}
	}
	if(bFound)
	{
		return  pTab;
	}
	return pMasterTab;
}

/*!
  Get line's offsets relative to this container
 \param  pContainer Container
 \retval xoff Container's X offset relative to container
 \retval yoff Container's Y offset relative to container
 */
void fp_VerticalContainer::getOffsets(fp_ContainerObject* pContainer, UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff = 0;
	UT_sint32 my_yoff = 0;
	fp_Container * pCon = (fp_Container *) this;
	fp_Container * pPrev = NULL;
	while(pCon && !pCon->isColumnType())
	{
		my_xoff += pCon->getX();
		UT_sint32 iycon = pCon->getY();
		my_yoff += iycon;
//
// Handle offsets from tables broken across pages.
//
// We detect
// line->cell->table->cell->table->cell->table->column
//
		if(pCon->getContainerType() == FP_CONTAINER_TABLE && 
		   pCon->getContainer()->isColumnType())
		{
			fp_VerticalContainer * pVCon= (fp_VerticalContainer *) pCon;
//
// Lines and Cells are actually always in the Master table. To make
// Them print on the right pages broken tables are created which
// sit in different columns. Here we hijack the recursive search and
// move it up the correct broken table line when we come across a cell
// 
			pVCon = getCorrectBrokenTable((fp_Container *) pContainer);
			if(pPrev && pPrev->getContainerType() == FP_CONTAINER_CELL)
			{
				UT_sint32 iTable =  getYoffsetFromTable(pCon,pPrev,pContainer);
				my_yoff += iTable;
				fp_TableContainer * pTab = (fp_TableContainer *) pVCon; 
				if(pTab->isThisBroken() && pTab != pTab->getMasterTable()->getFirstBrokenTable())
				{
					my_yoff = my_yoff + pVCon->getY() -iycon;
				}
				UT_sint32 col_xV =0;
				UT_sint32 col_yV =0;
				fp_Column * pCol = (fp_Column *) pVCon->getColumn();
				pCol->getPage()->getScreenOffsets(pCol, col_xV, col_yV);
				UT_sint32 col_x =0;
				UT_sint32 col_y =0;
				pCol =(fp_Column *)  pCon->getColumn();
				pCol->getPage()->getScreenOffsets(pCol, col_x, col_y);
				UT_sint32 ydiff = col_yV - col_y;
				pCon = (fp_Container *) pVCon;
				my_yoff += ydiff;
			}
			else
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		}
		pPrev = pCon;
		pCon = pCon->getContainer();
	}
	if(pCon)
	{
		xoff = pCon->getX() + my_xoff + pContainer->getX();
		yoff = pCon->getY() + my_yoff + pContainer->getY();
	}
	else
	{
		xoff = 0;
		yoff = 0;
	}
}

/*!
  Get Containers' offsets relative to the screen
 \param  pContainer Container which we want to find the absolute 
                    position of.
 \retval xoff Container's X offset relative the screen
 \retval yoff Container's Y offset relative the screen
 */
void fp_VerticalContainer::getScreenOffsets(fp_ContainerObject* pContainer,
									UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff =0;
	UT_sint32 my_yoff =0;

	if((getPage() == NULL) || (pContainer == NULL))
	{
		xoff = 0;
		yoff = 0;
		return;
	}

	fp_Container * pCon = (fp_Container *) this;
	fp_Container * pPrev = NULL;
	while(!pCon->isColumnType())
	{
		my_xoff += pCon->getX();
		UT_sint32 iycon = pCon->getY();
		my_yoff += iycon;
//
// Handle offsets from tables broken across pages.
//
// We detect
// line->cell->table->cell->table->cell->table->column
//
		if(pCon->getContainerType() == FP_CONTAINER_TABLE&& 
		   (pCon->getContainer()->getContainerType() ==  FP_CONTAINER_COLUMN ||
			pCon->getContainer()->getContainerType() ==  FP_CONTAINER_COLUMN_SHADOW))
		{
			fp_VerticalContainer * pVCon= (fp_VerticalContainer *) pCon;
//
// Lines and Cells are actually always in the Master table. To make
// Them print on the right pages broken tables are created which
// sit in different columns. Here we put in a recursive search find
// the correct broken table line when we come across a cell
// 
// Then we have to get all the offsets right for the broken table.
//
			pVCon = getCorrectBrokenTable((fp_Container *) pContainer);
			if(pPrev && pPrev->getContainerType() == FP_CONTAINER_CELL)
			{
				my_yoff += getYoffsetFromTable(pCon,pPrev,pContainer);
				fp_TableContainer * pTab = (fp_TableContainer *) pVCon; 
				if(pTab->isThisBroken() && pTab != pTab->getMasterTable()->getFirstBrokenTable())
				{
					my_yoff = my_yoff + pVCon->getY() -iycon;
				}
				pCon = (fp_Container *) pVCon;
			}
			else
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		}
		pPrev = pCon;
		pCon = pCon->getContainer();
	}
	UT_sint32 col_x =0;
	UT_sint32 col_y =0;
	xoff = my_xoff + pContainer->getX();
	yoff = my_yoff + pContainer->getY();

	if (pCon->getContainerType() == FP_CONTAINER_COLUMN)
	{
		fp_Column * pCol = (fp_Column *) pCon;
		pCol->getPage()->getScreenOffsets(pCol, col_x, col_y);

		xoff += col_x;
		yoff += col_y;
	}
	else if (pCon->getContainerType() == FP_CONTAINER_COLUMN_SHADOW)
	{
		fp_ShadowContainer * pCol = (fp_ShadowContainer *) pCon;
		pCol->getPage()->getScreenOffsets(pCol, col_x, col_y);

		xoff += col_x;
		yoff += col_y;
	}
	else
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

/*!
 Remove line from container
 \param pContainer Container
 \note The line is not destructed, as it is owned by the logical
       hierarchy.
 */
void fp_VerticalContainer::removeContainer(fp_Container* pContainer)
{
	UT_sint32 iCount = countCons();
	if(iCount == 0)
		return;
	UT_sint32 ndx = findCon(pContainer);
	UT_ASSERT(ndx >= 0);

	deleteNthCon(ndx);

	// don't delete the line here, it's deleted elsewhere.
}

/*!
 Insert line at the front/top of the container
 \param pNewContainer Container
 */
bool fp_VerticalContainer::insertContainer(fp_Container* pNewContainer)
{
	pNewContainer->clearScreen();
	insertConAt(pNewContainer, 0);
	pNewContainer->setContainer((fp_Container *)this);
	pNewContainer->recalcMaxWidth(true);

	return true;
}

/*!
  Get column gap from page the container is located on
  \return Column gap
*/
UT_sint32	fp_VerticalContainer::getColumnGap(void) const
{
	return getColumn()->getPage()->getColumnGap();
}

/*!
 Append line at the end/bottom of the container
 \param pNewContainer Container
 */
bool fp_VerticalContainer::addContainer(fp_Container* pNewContainer)
{
	pNewContainer->clearScreen();
	addCon(pNewContainer);
	pNewContainer->setContainer(this);
	pNewContainer->recalcMaxWidth(true);

	return true;
}

/*!
 Insert line in container after specified line
 \param pNewContainer   Container to be inserted
 \param pAfterContainer After this line
 \todo This function has been hacked to handle the case where
       pAfterContainer is NULL. That case should not happen. Bad callers
       should be identified and fixed, and this function should be
       cleaned up.
 */
bool fp_VerticalContainer::insertContainerAfter(fp_Container*	pNewContainer, fp_Container*	pAfterContainer)
{
	UT_ASSERT(pAfterContainer);
	UT_ASSERT(pNewContainer);

	UT_sint32 count = countCons();
	UT_sint32 ndx = findCon(pAfterContainer);
	UT_ASSERT( (count > 0) || (ndx == -1) );

	/*
	  TODO this routine should not be allowing pAfterContainer to be NULL.
	  Right now, we've fixed the symptom, but we really should fix
	  the problem.  */
	UT_ASSERT(ndx >= 0);
	pNewContainer->clearScreen();
	if ( (ndx+1) == count )				// append after last line in vector
		addCon(pNewContainer);
	else if (ndx >= 0)					// append after this item within the vector
		insertConAt(pNewContainer, ndx+1);
	else
	{
		// TODO remove this....
		insertConAt(pNewContainer, 0);
	}

	pNewContainer->setContainer(this);
	pNewContainer->recalcMaxWidth(true);

	return true;
}


/*!
  Clear container content from screen.

  \fixme Needs to clear outline as well
*/
void fp_VerticalContainer::clearScreen(void)
{
	int count = countCons();
	for (int i = 0; i<count; i++)
	{
		fp_ContainerObject* pContainer = (fp_ContainerObject*) getNthCon(i);

		pContainer->clearScreen();
	}
}

/*!
 Draw container outline
 \param pDA Draw arguments
 */
void fp_VerticalContainer::_drawBoundaries(dg_DrawArgs* pDA)
{
    UT_ASSERT(pDA->pG == getGraphics());
	UT_ASSERT(getPage());
	UT_ASSERT(getPage()->getDocLayout()->getView());
	if(getPage() == NULL)
	{
		return;
	}
	if(getPage()->getDocLayout()->getView() == NULL)
	{
		return;
	}
    if(getPage()->getDocLayout()->getView()->getShowPara() && getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN)){
        UT_sint32 xoffBegin = pDA->xoff - 1;
        UT_sint32 yoffBegin = pDA->yoff - 1;
        UT_sint32 xoffEnd = pDA->xoff + m_iWidth + 2;
        UT_sint32 yoffEnd = pDA->yoff + m_iMaxHeight + 2;

		UT_RGBColor clrShowPara(127,127,127);
		getGraphics()->setColor(clrShowPara);

        getGraphics()->drawLine(xoffBegin, yoffBegin, xoffEnd, yoffBegin);
        getGraphics()->drawLine(xoffBegin, yoffEnd, xoffEnd, yoffEnd);
        getGraphics()->drawLine(xoffBegin, yoffBegin, xoffBegin, yoffEnd);
        getGraphics()->drawLine(xoffEnd, yoffBegin, xoffEnd, yoffEnd);
    }
}

/*!
 * Returns the maximum line height as determined from the layout method
 * This used by the draw method to determine if a line should be drawn in
 * a clipping rectangle
 */
UT_sint32 fp_VerticalContainer::_getMaxContainerHeight(void) const
{
	return m_imaxContainerHeight;
}

/*!
 * Set the maximum line Height
\params UT_sint32 iLineHeight the largest line height yet found.
 */
void fp_VerticalContainer::_setMaxContainerHeight( UT_sint32 iLineHeight)
{
	m_imaxContainerHeight = iLineHeight;
}

/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_VerticalContainer::draw(dg_DrawArgs* pDA)
{
	const UT_Rect * pClipRect = pDA->pG->getClipRect();
	UT_sint32 ytop = 0, ybot = (UT_sint32)(((UT_uint32)(1<<31)) - 1);

	if(pClipRect)
	{
		ytop = pClipRect->top;
		ybot = UT_MAX(pClipRect->height,_getMaxContainerHeight()) 
			+ ytop + _UL(1);
		xxx_UT_DEBUGMSG(("clipRect height %d \n",pClipRect->height));
	}

//
// Only draw the lines in the clipping region.
//
	bool bStartedDrawing = false;
	dg_DrawArgs da = *pDA;
	UT_uint32 count = countCons();
	for (UT_uint32 i = 0; i < count; i++)
	{
		fp_ContainerObject* pContainer = (fp_ContainerObject*) getNthCon(i);
		bool bInTable = false;

		da.xoff = pDA->xoff + pContainer->getX();
		da.yoff = pDA->yoff + pContainer->getY();

		if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
		{
			fp_TableContainer * pTab = (fp_TableContainer *) pContainer;
			if(pTab->isThisBroken())
				da.xoff = pDA->xoff + pTab->getMasterTable()->getX();

			UT_sint32 iTableBot = da.yoff + pTab->getHeight();
			/* we're in the table if iTableBot < ytop, or table top > ybot */
			bInTable = (iTableBot < ytop || da.yoff > ybot);
		}

		UT_sint32 sumHeight = pContainer->getHeight() + (ybot-ytop);
		UT_sint32 totDiff;
		if(da.yoff < ytop)
			totDiff = ybot - da.yoff;
		else
			totDiff = da.yoff + pContainer->getHeight() - ytop;

//		if(bTable || (da.yoff >= ytop && da.yoff <= ybot) || (ydiff >= ytop && ydiff <= ybot))
		if(bInTable || (totDiff < sumHeight)  || (pClipRect == NULL))
		{
			bStartedDrawing = true;
			pContainer->draw(&da);
		}
		else if(bStartedDrawing)
		{
			// we've started drawing and now we're not, so we're done.
			break;
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
void fp_VerticalContainer::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos,
								   bool& bBOL, bool& bEOL)
{
	int count = countCons();
	xxx_UT_DEBUGMSG(("SEVIOR: count cons %d x %d y %d \n",count,x,y));
	if(count == 0)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: In container type %d return with bBOL set \n",getContainerType()));
		if(getContainerType() == FP_CONTAINER_TABLE)
		{
			xxx_UT_DEBUGMSG(("SEVIOR: Table container with no containers \n"));
			return;
		}
		pos = 2;
		bBOL = true;
		bEOL = true;
		return;
	}


	fp_ContainerObject* pContainer = NULL;
	int i = 0;
	// Find first container that contains the point. First has its lower level below the desired Y
	// position. Note that X-positions are completely ignored here.
	UT_sint32 iHeight = 0;
	do 
	{
		pContainer = (fp_ContainerObject*) getNthCon(i++);
		if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
		{
			iHeight = static_cast<fp_TableContainer *>(pContainer)->getHeight();
		}
		else
		{
			iHeight = pContainer->getHeight();
		}
		xxx_UT_DEBUGMSG(("SEVIOR: IN column looking at x %d y %d height %d \n",pContainer->getX(),pContainer->getY(),iHeight));
	} while ((i < count)
			 && (y > (pContainer->getY() + iHeight)));
	// Undo the postincrement.
	i--;
	// Now check if the position is actually between the found container
	// and the line before it (ignore check on the top-most line).
	UT_sint32 iUHeight =0;
	if (i > 0 && y < pContainer->getY())
	{
		fp_ContainerObject* pContainerUpper = (fp_ContainerObject*) getNthCon(i-1);
		if(pContainerUpper->getContainerType() == FP_CONTAINER_TABLE)
		{
			iUHeight = static_cast<fp_TableContainer *>(pContainerUpper)->getHeight();
		}
		else
		{
			iUHeight = pContainer->getHeight();
		}

		// Be careful with the signedness here - bug 172 leared us a
		// lesson!

		// Now pick the line that is closest to the point - or the
		// upper if it's a stalemate.
		if ((pContainer->getY() - y) >= (y - (pContainerUpper->getY() + (UT_sint32) iUHeight)))
		{
			pContainer = pContainerUpper;
		}
	}
	if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Looking in a table \n"));
		fp_TableContainer * pTab = (fp_TableContainer *) pContainer;
		xxx_UT_DEBUGMSG(("SEVIOR: do map to position for %x \n",pContainer));
		pTab->mapXYToPosition(x - pContainer->getX(),
								y - pContainer->getY() , 
								pos, bBOL, bEOL);
	}
	else
	{
		xxx_UT_DEBUGMSG(("SEVIOR: do map to position for %x \n",pContainer));
		pContainer->mapXYToPosition(x - pContainer->getX(),
								y - pContainer->getY() , 
									pos, bBOL, bEOL);
		xxx_UT_DEBUGMSG(("SEVIOR: Found pos %d in column \n",pos));
	}
}

/*!
 Compute the distance from point to the container's circumference
 \param x X coordinate of point
 \param y Y coordinate of point
 \return Distance between container's circumference and point
 */
UT_uint32 fp_VerticalContainer::distanceFromPoint(UT_sint32 x, UT_sint32 y)
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
void fp_VerticalContainer::setX(UT_sint32 iX, bool bDontClearIfNeeded)
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
void fp_VerticalContainer::setY(UT_sint32 iY)
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
fp_Container* fp_VerticalContainer::getFirstContainer(void) const
{
	if (countCons() > 0)
	{
		return (fp_Container*) getNthCon(0);
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
fp_Container* fp_VerticalContainer::getLastContainer(void) const
{
	UT_uint32 iCount = countCons();

	if (iCount > 0)
	{
		return (fp_Container*) getNthCon(iCount - 1);
	}
	else
	{
		return NULL;
	}
}


/*!
  Bump Containers from this Container to the next
  \param pLastContainerToKeep Last line to keep in this column or NULL for none
*/
void fp_VerticalContainer::bumpContainers(fp_ContainerObject* pLastContainerToKeep)
{
	UT_sint32 ndx = (NULL == pLastContainerToKeep) ? 0 : (findCon(pLastContainerToKeep)+1);
	UT_ASSERT(ndx >= 0);
	UT_sint32 iCount = countCons();
	UT_sint32 i;

	fp_VerticalContainer* pNextContainer = (fp_VerticalContainer*) getNext();
	UT_ASSERT(pNextContainer);

	if (pNextContainer->isEmpty())
	{
		for (i=ndx; i<iCount; i++)
		{
			fp_Container* pContainer = (fp_Container*) getNthCon(i);
			pContainer->clearScreen();
			pNextContainer->addContainer(pContainer);
		}
	}
	else
	{
		for (i=iCount - 1; i >= ndx; i--)
		{
			fp_Container* pContainer = (fp_Container*) getNthCon(i);
			pContainer->clearScreen();
			pNextContainer->insertContainer(pContainer);
		}
	}

	for (i=iCount - 1; i >= ndx; i--)
	{
		deleteNthCon(i);
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
fp_Column::fp_Column(fl_SectionLayout* pSectionLayout) : fp_VerticalContainer(FP_CONTAINER_COLUMN, pSectionLayout)
{
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

  \see fp_Line::setAssignedScreenHeight, fp_Container::recalcHeight
*/
void fp_Column::layout(void)
{
	_setMaxContainerHeight(0);
	UT_sint32 iY = 0, iPrevY = 0;
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	UT_sint32 iYLayoutUnits = 0;
	double ScaleLayoutUnitsToScreen;
	ScaleLayoutUnitsToScreen = (double)getGraphics()->getResolution() / UT_LAYOUT_UNITS;
#endif

	UT_uint32 iCountContainers = countCons();
	fp_Container *pContainer, *pPrevContainer = NULL;
	long imax = (1<<30) -1;
 
	for (UT_uint32 i=0; i < iCountContainers ; i++)
	{
		pContainer = (fp_Container*) getNthCon(i);
		
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		iY = (int)(ScaleLayoutUnitsToScreen * iYLayoutUnits + 0.5);
#endif
//
// Set the location first so the height of a table can be calculated
// and adjusted.
//
		if(pContainer->getY() != iY)
		{
			pContainer->clearScreen();
		}
		pContainer->setY(iY);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		pContainer->setYInLayoutUnits(iYLayoutUnits);
#endif

//
// This is to speedup redraws.
//
		fp_TableContainer * pTab = NULL;
		UT_sint32 iHeight = pContainer->getHeight();
		if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
		{
			pTab = (fp_TableContainer *) pContainer;
		}
		if(iHeight > _getMaxContainerHeight())
		{
			_setMaxContainerHeight(iHeight);
		}
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		UT_sint32 iContainerHeightLayoutUnits = pContainer->getHeightInLayoutUnits();
		if(pTab)
		{
			iContainerHeightLayoutUnits = pTab->getHeightInLayoutUnits();
		}
		UT_sint32 iContainerMarginAfterLayoutUnits = pContainer->getMarginAfterInLayoutUnits();
#else
		UT_sint32 iContainerHeight = iHeight;
		if(pTab)
		{
			iContainerHeight = pTab->getHeight();
		}
		UT_sint32 iContainerMarginAfter = pContainer->getMarginAfter();
#endif

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		iYLayoutUnits += iContainerHeightLayoutUnits;
		iYLayoutUnits += iContainerMarginAfterLayoutUnits;
#else
		iY += iContainerHeight;
		iY += iContainerMarginAfter;
		//iY +=  0.5;

#endif

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		if((long) iYLayoutUnits > imax)
		{
		       UT_ASSERT(0);
		}
		// Update height of previous line now we know the gap between
		// it and the current line.
#endif
		if (pPrevContainer)
		{
			pPrevContainer->setAssignedScreenHeight(iY - iPrevY);
		}
		pPrevContainer = pContainer;
		iPrevY = iY;
	}

	// Correct height position of the last line
	if (pPrevContainer)
	{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		iY = (int)(ScaleLayoutUnitsToScreen * iYLayoutUnits +0.5);
#endif
		pPrevContainer->setAssignedScreenHeight(iY - iPrevY + 1);
	}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	UT_sint32 iNewHeight = (int)(ScaleLayoutUnitsToScreen * iYLayoutUnits);
#else
	UT_sint32 iNewHeight = iY;
#endif

	if (getHeight() == iNewHeight)
	{
		return;
	}

	setHeight(iNewHeight);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	setHeightLayoutUnits(iYLayoutUnits);
#endif
	getPage()->columnHeightChanged(this);
}

fl_DocSectionLayout* fp_Column::getDocSectionLayout(void) const
{
	UT_ASSERT(getSectionLayout()->getType() == FL_SECTION_DOC ||
			  getSectionLayout()->getType() == FL_SECTION_HDRFTR ||
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
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
 									   UT_sint32 iWidthLayout,
									   UT_sint32 iHeightLayout,
#endif
									   fl_SectionLayout* pSectionLayout)
	: fp_VerticalContainer(FP_CONTAINER_COLUMN_SHADOW, pSectionLayout)
{
	_setX(iX);
	_setY(iY);
	setWidth(iWidth);
	setHeight(iHeight);
	setMaxHeight(iHeight);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	setWidthInLayoutUnits(iWidthLayout);
	setHeightLayoutUnits(iHeightLayout);
	setMaxHeightInLayoutUnits( (UT_sint32)(  (double) getHeight() * ( (double) iHeightLayout/ (double) getMaxHeight()))) ;
#endif
   m_bHdrFtrBoxDrawn = false;
}

fp_ShadowContainer::~fp_ShadowContainer()
{
}

void fp_ShadowContainer::layout(void)
{
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	double ScaleLayoutUnitsToScreen = (double)getGraphics()->getResolution() / UT_LAYOUT_UNITS;
	double yHardOffset = 5.0; // Move 5 pixels away from the very edge
	UT_sint32 iYLayoutUnits = (UT_sint32) (yHardOffset/	ScaleLayoutUnitsToScreen);
#else
	UT_sint32 iY = 5;
#endif
	UT_uint32 iCountContainers = countCons();
	FV_View * pView = getPage()->getDocLayout()->getView();
	bool doLayout = true;
	if(pView)
	{
	    doLayout =	pView->getViewMode() == VIEW_PRINT;
	}
	for (UT_uint32 i=0; i < iCountContainers; i++)
	{
		fp_Container* pContainer = (fp_Container*) getNthCon(i);
		fp_TableContainer * pTab = NULL;
		if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
		{
			pTab = (fp_TableContainer *) pContainer;
		}
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		UT_sint32 iContainerHeightLayoutUnits = pContainer->getHeightInLayoutUnits();
		if(pTab != NULL)
		{
			iContainerHeightLayoutUnits = pTab->getHeightInLayoutUnits();
		}
		UT_sint32 iContainerMarginAfterLayoutUnits = pContainer->getMarginAfterInLayoutUnits();
		UT_sint32 sum = iContainerHeightLayoutUnits + iContainerMarginAfterLayoutUnits;
		if(((iYLayoutUnits + sum) <= (getMaxHeightInLayoutUnits())) && doLayout)
		{
			if(pTab == NULL)
			{
				pContainer->setY((UT_sint32)(ScaleLayoutUnitsToScreen * iYLayoutUnits));
				pContainer->setYInLayoutUnits(iYLayoutUnits);
			}
			else
			{
				pTab->setY((UT_sint32)(ScaleLayoutUnitsToScreen * iYLayoutUnits));
				pTab->setYInLayoutUnits(iYLayoutUnits);
			}
		}
#else

		UT_sint32 iContainerHeight = pContainer->getHeight();
		if(pTab != NULL)
		{
			iContainerHeight = pTab->getHeight();
		}
		UT_sint32 iContainerMarginAfter = pContainer->getMarginAfter();
		UT_sint32 sum = iContainerHeight + iContainerMarginAfter;
		if(((iY + sum) <= (getMaxHeight())) && doLayout)
		{
			pContainer->setY(iY);
		}
#endif
		else
		{
//
// FIXME: Dirty hack to clip.
//
			pContainer->setY(-1000000);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
			pContainer->setYInLayoutUnits(-1000000);
#endif
		}
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		iYLayoutUnits += iContainerHeightLayoutUnits;
		iYLayoutUnits += iContainerMarginAfterLayoutUnits;
#else
		iY += iContainerHeight;
		iY += iContainerMarginAfter;
#endif
	}
	
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	UT_sint32 iNewHeight = (int)(ScaleLayoutUnitsToScreen * iYLayoutUnits);
#else
	UT_sint32 iNewHeight = iY;
#endif
	if (getHeight() == iNewHeight)
	{
		return;
	}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	if(iYLayoutUnits <= getMaxHeightInLayoutUnits())
	{
		setHeight(iNewHeight);
		setHeightLayoutUnits(iYLayoutUnits);
	}
#else
	if(iY <= getMaxHeight())
	{
		setHeight(iNewHeight);
	}
#endif

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
	int count = countCons();
	for (int i = 0; i<count; i++)
	{
		fp_ContainerObject* pContainer = (fp_ContainerObject*) getNthCon(i);

		pContainer->clearScreen();
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
	UT_sint32 count = countCons();
	UT_sint32 iY= 0;
	for (UT_sint32 i = 0; i<count; i++)
	{
		fp_Container* pContainer = (fp_Container*) getNthCon(i);

		dg_DrawArgs da = *pDA;
		da.xoff += pContainer->getX();
		da.yoff += pContainer->getY();
		fp_TableContainer * pTab = NULL;
		if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
		{
			pTab = static_cast<fp_TableContainer *>(pContainer);
		}
		
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		UT_sint32 iContainerHeightLayoutUnits = pContainer->getHeightInLayoutUnits();
		if(pTab)
		{
			 iContainerHeightLayoutUnits = pTab->getHeightInLayoutUnits();
		}
		UT_sint32 iContainerMarginAfterLayoutUnits = pContainer->getMarginAfterInLayoutUnits();
		iY += iContainerHeightLayoutUnits;
		iY += iContainerMarginAfterLayoutUnits;
//
// Clip to keep inside header/footer container
//
		if(iY > getMaxHeightInLayoutUnits())
			break;
#else
		UT_sint32 iContainerHeight = pContainer->getHeight();
		if(pTab)
		{
			UT_sint32 iContainerHeight = pTab->getHeight();
		}
		UT_sint32 iContainerMarginAfter = pContainer->getMarginAfter();
		iY += iContainerHeight;
		iY += iContainerMarginAfter;
//
// Clip to keep inside header/footer container
//
		if(iY > getMaxHeight())
			break;
#endif

		pContainer->draw(&da);
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
	: fp_VerticalContainer(FP_CONTAINER_HDRFTR, pSectionLayout)
{
	_setX(0);
	_setY(0);
	setWidth(iWidth);
	setHeight(0);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	setWidthInLayoutUnits(iWidthLayout);
	setHeightLayoutUnits(0);
#endif
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
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	UT_sint32 iYLayoutUnits = 0;
	double ScaleLayoutUnitsToScreen;
	ScaleLayoutUnitsToScreen = (double)getGraphics()->getResolution() / UT_LAYOUT_UNITS;
#else
	UT_sint32 iY = 0;
#endif

	UT_uint32 iCountContainers = countCons();

	for (UT_uint32 i=0; i < iCountContainers; i++)
	{
		fp_Container* pContainer = (fp_Container*) getNthCon(i);
		fp_TableContainer * pTab = NULL;
		if(pContainer->getContainerType() == FP_CONTAINER_TABLE)
		{
			pTab = static_cast<fp_TableContainer *>(pContainer);
		}
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		UT_sint32 iContainerHeightLayoutUnits = pContainer->getHeightInLayoutUnits();
		if(pTab)
		{
			iContainerHeightLayoutUnits = pTab->getHeightInLayoutUnits();
		}
		UT_sint32 iContainerMarginAfterLayoutUnits = pContainer->getMarginAfterInLayoutUnits();

		pContainer->setY((int)(ScaleLayoutUnitsToScreen * iYLayoutUnits));
		pContainer->setYInLayoutUnits(iYLayoutUnits);
		iYLayoutUnits += iContainerHeightLayoutUnits;
		iYLayoutUnits += iContainerMarginAfterLayoutUnits;
#else

		UT_sint32 iContainerHeight = pContainer->getHeight();
		if(pTab)
		{
			iContainerHeight = pTab->getHeight();
		}
		UT_sint32 iContainerMarginAfter = pContainer->getMarginAfter();

		pContainer->setY(iY);
		iY += iContainerHeight;
		iY += iContainerMarginAfter;
#endif
	}

#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	UT_sint32 iNewHeight = (int)(ScaleLayoutUnitsToScreen * iYLayoutUnits);
#else
	UT_sint32 iNewHeight = iY;
#endif

	if (getHeight() == iNewHeight)
	{
		return;
	}

	setHeight(iNewHeight);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	setHeightLayoutUnits(iYLayoutUnits);
#endif
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
 \param  pContainer Container
 \retval xoff Container's X offset relative the screen actually -10000
 \retval yoff Container's Y offset relative the screen actually -10000
 */
void fp_HdrFtrContainer::getScreenOffsets(fp_ContainerObject* pContainer,
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
