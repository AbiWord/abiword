/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Martin Sevior <msevior@physics.unimelb.edu.au>
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
/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "fp_TableContainer.h"
#include "fp_Column.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fv_View.h"
#include "fl_SectionLayout.h"
#include "fl_TableLayout.h"
#include "gr_DrawArgs.h"
#include "ut_vector.h"
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"

#define SCALE_TO_SCREEN (double)getGraphics()->getResolution() / UT_LAYOUT_UNITS

fp_TableRowColumn::fp_TableRowColumn(void) :
		requisition(0),
        allocation(0),
        spacing(0),
        need_expand(false),
		need_shrink(true),
		expand(true),
        shrink(true),
        empty(true)
{
}

fp_TableRowColumn::~fp_TableRowColumn(void)
{
}

/*!
  Create Cell container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_CellContainer::fp_CellContainer(fl_SectionLayout* pSectionLayout) 
	: fp_VerticalContainer(FP_CONTAINER_CELL, pSectionLayout),
	  m_iLeftAttach(0),
	  m_iRightAttach(0),
	  m_iTopAttach(0),
	  m_iBottomAttach(0),
	  m_iLeftPad(0),
	  m_iRightPad(0),
	  m_iTopPad(0),
	  m_iBotPad(0),
	  m_pNextInTable(NULL),
	  m_pPrevInTable(NULL),
	  m_bXexpand(true),
	  m_bYexpand(false),
	  m_bXshrink(false),
	  m_bYshrink(true),
	  m_bXfill(true),
	  m_bYfill(false)
{
}

/*!
  Destruct container
  \note The Containers in vector of the container are not
        destructed. They are owned by the logical hierarchy (i.e.,
		the fl_Container classes like fl_BlockLayout), not the physical
        hierarchy.
 */
fp_CellContainer::~fp_CellContainer()
{
}

void fp_CellContainer::setHeight(UT_sint32 iHeight)
{
	if (iHeight == getHeight())
	{
		return;
	}
	fp_VerticalContainer::setHeight(iHeight);
	fl_SectionLayout * pSL = getSectionLayout();
	pSL = (fl_SectionLayout *) pSL->myContainingLayout();
	UT_ASSERT(pSL->getContainerType() == FL_CONTAINER_TABLE);
	static_cast<fl_TableLayout *>(pSL)->setDirty();
}

/*!
  Set width
  \param iWidth Width of container
  \todo Should force re-line-break operations on all blocks in the
        container
 */
void fp_CellContainer::setWidth(UT_sint32 iWidth)
{
	UT_sint32 myWidth = getWidth();
	if (iWidth == myWidth)
	{
		return;
	}
	fp_VerticalContainer::setWidth(iWidth);
	fl_SectionLayout * pSL = getSectionLayout();
	pSL = (fl_SectionLayout *) pSL->myContainingLayout();
	UT_ASSERT(pSL->getContainerType() == FL_CONTAINER_TABLE);
	static_cast<fl_TableLayout *>(pSL)->setDirty();
	fl_CellLayout * pCellL = (fl_CellLayout *) getSectionLayout();
	pCellL->setNeedsReformat();
	pCellL->localCollapse();
	pCellL->format();
	UT_sint32 i = 0;
	for(i =0; i< (UT_sint32) countCons(); i++)
	{
		fp_Container * pCon = (fp_Container *) getNthCon(i);
		if(pCon->getContainerType() == FP_CONTAINER_LINE)
		{
			static_cast<fp_Line *>(pCon)->layout();
		}
		else if(pCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			static_cast<fp_TableContainer *>(pCon)->layout();
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}
}

void fp_CellContainer::setContainer(fp_Container * pContainer)
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
	UT_ASSERT(pContainer->getContainerType() == FP_CONTAINER_TABLE);
	fp_TableContainer * pTable = (fp_TableContainer *) pContainer;
	UT_sint32 iWidth = pTable->getWidth();

	fp_CellContainer::setWidth(iWidth);
#ifndef WITH_PANGO
	UT_sint32 iWidthLayout = pTable->getWidthInLayoutUnits();
	setWidthInLayoutUnits(iWidthLayout);
#endif
}

/*!
 Draw container outline
 \param pDA Draw arguments
 */
void fp_CellContainer::_drawBoundaries(dg_DrawArgs* pDA)
{
    UT_ASSERT(pDA->pG == getGraphics());
	UT_ASSERT(getPage());
	if(getPage() == NULL)
	{
		return;
	}
	if(getPage()->getDocLayout()->getView() == NULL)
	{
		return;
	}
    if(getPage()->getDocLayout()->getView()->getShowPara() && getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN)){
        UT_sint32 xoffBegin = pDA->xoff + getX();
        UT_sint32 yoffBegin = pDA->yoff + getY();
        UT_sint32 xoffEnd = pDA->xoff + getX() + getWidth();
        UT_sint32 yoffEnd = pDA->yoff + getY() + getHeight();

		UT_RGBColor clrShowPara(127,127,127);
		getGraphics()->setColor(clrShowPara);
		xxx_UT_DEBUGMSG(("SEVIOR: cell boundaries xleft %d xright %d ytop %d ybot %d \n",xoffBegin,xoffEnd,yoffBegin,yoffEnd));
        getGraphics()->drawLine(xoffBegin, yoffBegin, xoffEnd, yoffBegin);
        getGraphics()->drawLine(xoffBegin, yoffEnd, xoffEnd, yoffEnd);
        getGraphics()->drawLine(xoffBegin, yoffBegin, xoffBegin, yoffEnd);
        getGraphics()->drawLine(xoffEnd, yoffBegin, xoffEnd, yoffEnd);
    }
}

/*!
 * Return the topmost table in this structure. The one embedded in the 
 * column.
 */
fp_TableContainer * fp_CellContainer::getTopmostTable() const
{
	fp_Container * pUp = getContainer();
	fp_Container * pPrev = pUp;
	while(pUp->getContainerType() != FP_CONTAINER_COLUMN)
	{
		pPrev = pUp;
		pUp = pUp->getContainer();
	}
	if(pPrev->getContainerType() == FP_CONTAINER_TABLE)
	{
		return (fp_TableContainer *) pPrev;
	}
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

/*!
 * Return the x coordinate offset of this cell. 
 * We need to know the line for situations where the cell is broken over
 * different pages.
 */
UT_sint32 fp_CellContainer::getCellX(fp_Line * pLine) const
{
	return 0;
}

/*!
 * Return the y coordinate offset of this cell. 
 * We need to know the line for situations where the cell is broken over
 * different pages.
 */
UT_sint32 fp_CellContainer::getCellY(fp_Line * pLine) const
{
	fp_TableContainer * pTab = getTopmostTable();
	return pTab->getY();
}

/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_CellContainer::draw(dg_DrawArgs* pDA)
{
	UT_sint32 count = countCons();
	const UT_Rect * pClipRect = pDA->pG->getClipRect();
	UT_sint32 ytop,ybot;
	UT_sint32 i;
	UT_sint32 imax = (UT_sint32)(((UT_uint32)(1<<31)) - 1);
	if(pClipRect)
	{
		ybot = UT_MAX(pClipRect->height,_getMaxContainerHeight());
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
	xxx_UT_DEBUGMSG(("SEVIOR: Drawing cell %x x %d, y %d width %d height %d \n",this,getX(),getY(),getWidth(),getHeight()));
//
// Only draw the lines in the clipping region.
//
	for ( i = 0; (i<count && !bStop); i++)
	{
		fp_ContainerObject* pContainer = (fp_ContainerObject*) getNthCon(i);
// fixme look here!!!
// X and Y are definately in screen coords!
		dg_DrawArgs da = *pDA;
//
// pDA has xoff set at the columns left edge, we need to offset this
//     by the cell x position.
// pDA has yoffset at the last ypos in the column relative to the screen
//     The position Ypos is the absolute position on the screen we need
//     to offset this with the position of the container holding this
//     cell.

		da.xoff += pContainer->getX() + getX();
		da.yoff += pContainer->getY() + getY();
		UT_sint32 ydiff = da.yoff + pContainer->getHeight();
		if((da.yoff >= ytop && da.yoff <= ybot) || (ydiff >= ytop && ydiff <= ybot))
		{
			bStart = true;
			pContainer->draw(&da);
		}
		else if(bStart)
		{
			bStop = true;
		}
	}

    _drawBoundaries(pDA);
}

bool fp_CellContainer::isVBreakable(void)
{
	return true;
}

fp_ContainerObject * fp_CellContainer::VBreakAt(UT_sint32 vpos)
{
	return NULL;
}

UT_sint32 fp_CellContainer::wantVBreakAt(UT_sint32 vpos)
{
	return 0;
}

fp_Container * fp_CellContainer::getNextContainerInSection() const
{

	fl_ContainerLayout * pCL = (fl_ContainerLayout *) getSectionLayout();
	fl_ContainerLayout * pNext = pCL->getNext();
	if(pNext)
	{
		return pNext->getFirstContainer();
	}
	return NULL;
}


fp_Container * fp_CellContainer::getPrevContainerInSection() const
{

	fl_ContainerLayout * pCL = (fl_ContainerLayout *) getSectionLayout();
	fl_ContainerLayout * pPrev = pCL->getPrev();
	if(pPrev)
	{
		return pPrev->getLastContainer();
	}
	return NULL;
}


void fp_CellContainer::sizeRequest(fp_Requisition * pRequest)
{
	UT_sint32 count = countCons();
	UT_sint32 i =0;
	UT_sint32 height = 0;
	UT_sint32 width = 0;
	for(i=0 ; i < count; i++)
	{
		fp_Container * pCon = (fp_Container *) getNthCon(i);
		if(pCon->getContainerType() == FP_CONTAINER_LINE)
		{
			static_cast<fp_Line *>(pCon)->recalcHeight();
			if(width < pCon->getWidthInLayoutUnits())
			{
				width = pCon->getWidthInLayoutUnits();

			}
			height = height + pCon->getHeight();
		}
		else
		{
			fp_Requisition pReq;
			if(pCon->getContainerType() == FP_CONTAINER_TABLE)
			{
				static_cast<fp_TableContainer *>(pCon)->sizeRequest(&pReq);
			}
			else if(pCon->getContainerType() == FP_CONTAINER_CELL)
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
			else
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
			if(width < pReq.width)
			{
				width = pReq.width;
			}
			height = height + pReq.height;
		}
	}
	if(pRequest)
	{
		pRequest->width = width;
		pRequest->height = height;
	}
	m_MyRequest.width = width;
	m_MyRequest.height = height;
	xxx_UT_DEBUGMSG(("Sevior: Total height  %d width %d \n",height,width));
}

void fp_CellContainer::sizeAllocate(fp_Allocation * pAllocate)
{
	m_MyAllocation.width = pAllocate->width;
	m_MyAllocation.height = pAllocate->height;
	m_MyAllocation.x = pAllocate->x;
	m_MyAllocation.y = pAllocate->y;
}

void fp_CellContainer::layout(void)
{
	_setMaxContainerHeight(0);
	UT_sint32 iY = 0, iPrevY = 0;
	iY= 0;
#ifndef WITH_PANGO
	
	double ScaleLayoutUnitsToScreen;
	ScaleLayoutUnitsToScreen = (double)getGraphics()->getResolution() / UT_LAYOUT_UNITS;
	UT_sint32 iYLayoutUnits = 0;
#endif
	UT_uint32 iCountContainers = countCons();
	fp_Container *pContainer, *pPrevContainer = NULL;
	long imax = (1<<30) -1;

	for (UT_uint32 i=0; i < iCountContainers; i++)
	{
		pContainer = (fp_Container*) getNthCon(i);
//
// This is to speedup redraws.
//
		if(pContainer->getHeight() > _getMaxContainerHeight())
			_setMaxContainerHeight(pContainer->getHeight());
#ifndef WITH_PANGO
		UT_sint32 iContainerHeightLayoutUnits = pContainer->getHeightInLayoutUnits();
//		UT_sint32 iContainerMarginBefore = (i != 0) ? pContainer->getMarginBefore() : 0;
		UT_sint32 iContainerMarginAfterLayoutUnits = pContainer->getMarginAfterInLayoutUnits();
#else
		UT_sint32 iContainerHeight = pContainer->getHeight();
		UT_sint32 iContainerMarginAfter = pContainer->getMarginAfter();
#endif

#ifndef WITH_PANGO
//		iY += iContainerMarginBefore;
		iY = (int)(ScaleLayoutUnitsToScreen * iYLayoutUnits + 0.5);
#endif

		if(pContainer->getY() != iY)
		{
			pContainer->clearScreen();
		}
		pContainer->setY(iY);

#ifndef WITH_PANGO
		pContainer->setYInLayoutUnits(iYLayoutUnits);
		iYLayoutUnits += iContainerHeightLayoutUnits;
		iYLayoutUnits += iContainerMarginAfterLayoutUnits;
#else
		iY += iContainerHeight;
		iY += iContainerMarginAfter;
		//iY +=  0.5;

#endif

#ifndef WITH_PANGO
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
#ifndef WITH_PANGO
		iY = (int)(ScaleLayoutUnitsToScreen * iYLayoutUnits +0.5);
#endif
		pPrevContainer->setAssignedScreenHeight(iY - iPrevY + 1);
	}

#ifndef WITH_PANGO
	UT_sint32 iNewHeight = (int)(ScaleLayoutUnitsToScreen * iYLayoutUnits);
#else
	UT_sint32 iNewHeight = iY;
#endif
}

void fp_CellContainer::setToAllocation(void)
{
	setWidthInLayoutUnits(m_MyAllocation.width);
	setWidth(m_MyAllocation.width * SCALE_TO_SCREEN);
	setHeightLayoutUnits(m_MyAllocation.height);
	setHeight(m_MyAllocation.height);
	setYInLayoutUnits(m_MyAllocation.y);
	setX(m_MyAllocation.x * SCALE_TO_SCREEN);
	UT_DEBUGMSG(("SEVIOR: set to width %d, height %d,y %d,x %d \n", m_MyAllocation.width,m_MyAllocation.height,m_MyAllocation.y,m_MyAllocation.x));
	setMaxHeight(m_MyAllocation.height);
	setY(m_MyAllocation.y);
	layout();
}

//---------------------------------------------------------------------

/*!
  Create Table container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_TableContainer::fp_TableContainer(fl_SectionLayout* pSectionLayout) 
	: fp_VerticalContainer(FP_CONTAINER_TABLE, pSectionLayout),
	  m_iRows(0),
	  m_iCols(0),
	  m_iBorderWidth(0),
	  m_bIsHomogeneous(true),
	  m_iRowSpacing(0),
	  m_iColSpacing(0)
{
}

/*!
  Destruct container
  \note The Containers in vector of the container are not
        destructed. They are owned by the logical hierarchy (i.e.,
		the fl_Container classes like fl_BlockLayout), not the physical
        hierarchy.
 */
fp_TableContainer::~fp_TableContainer()
{
	UT_VECTOR_PURGEALL(fp_TableRowColumn *, m_vecRows);
	UT_VECTOR_PURGEALL(fp_TableRowColumn *, m_vecColumns);
}



/*!
  Find document position from X and Y coordinates
 \param  x X coordinate
 \param  y Y coordinate
 \retval pos Document position
 \retval bBOL True if position is at begining of line, otherwise false
 \retval bEOL True if position is at end of line, otherwise false
 */
void fp_TableContainer::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos,
								   bool& bBOL, bool& bEOL)
{
	UT_sint32 count = countCons();
	if(count == 0)
	{
		pos = 2;
		bBOL = true;
		bEOL = true;
		return;
	}

	fp_VerticalContainer* pC = NULL;
	UT_sint32 i = 0;
	bool bFound = false;

	// First see if there is a container with the point inside it.
	for(i=0; (i< count) && !bFound; i++)
	{
		pC = (fp_VerticalContainer*) getNthCon(i);
		if(x >= pC->getX() && x <  pC->getX() + pC->getWidth() &&
		   y >=  pC->getY() && y < pC->getY()+ pC->getMaxHeight())
		{
			bFound = true;
		}
	}
	if(bFound)
	{
		pC->mapXYToPosition(x - pC->getX(), y - pC->getY(), pos, bBOL, bEOL);
		return;
	}
//
// No cell directly overlaps. Look first for a column that overlaps and
// then choose the cell that is closest within that. Otherwise choose
// the closest cell.
//
	fp_VerticalContainer * pCloseX = NULL;
	fp_VerticalContainer * pCloseTot = NULL;
	UT_sint32 dclosex = 231456789;
	UT_sint32 dclosetot = 231456789;
	UT_sint32 d = 0;
	for(i=0; (i< count) && !bFound; i++)
	{
		pC = (fp_VerticalContainer*) getNthCon(i);
		if(x >= pC->getX() && x < pC->getX() + pC->getWidth())
		{
			d = y - pC->getY();
			if(d < 0) 
			    d = - d;
			if(d < dclosex)
			{
				dclosex = d;
				pCloseX = pC;
			}
		}
		d = pC->distanceFromPoint(x,y);
		if(d < dclosetot)
		{
			pCloseTot = pC;
			dclosetot = d;
		}
	}
	if(pCloseX != NULL)
	{
		pC = pCloseX;
	}
	else
	{
		pC = pCloseTot;
	}
	UT_ASSERT( pC != NULL);
	pC->mapXYToPosition(x - pC->getX(), y  - pC->getY(), pos, bBOL, bEOL);
}

void fp_TableContainer::resize(UT_sint32 n_rows, UT_sint32 n_cols)
{
  
  if (n_rows != m_iRows ||
      n_cols != m_iCols)
  {
	  fp_CellContainer * child = (fp_CellContainer *) getNthCon(0);
      while(child)
	  {
		  n_rows = UT_MAX (n_rows, child->getBottomAttach());
		  n_cols = UT_MAX (n_cols, child->getRightAttach());
		  child =  (fp_CellContainer *) child->getNext();
	  }
      
      if (n_rows != m_iRows)
	  {
		  UT_sint32 i;

		  i = m_iRows;
	      m_iRows = n_rows;
		  UT_VECTOR_PURGEALL(fp_TableRowColumn *, m_vecRows);
		  m_vecRows.clear();
		  for(i=0; i< m_iRows; i++)
		  {
			  m_vecRows.addItem((void*) new fp_TableRowColumn());
			  getNthRow(i)->requisition = 0;
			  getNthRow(i)->allocation = 0;
			  getNthRow(i)->spacing = m_iRowSpacing;
			  getNthRow(i)->need_expand = 0;
			  getNthRow(i)->need_shrink = 0;
			  getNthRow(i)->expand = 0;
			  getNthRow(i)->shrink = 0;
		  }
	  }

      if (n_cols != m_iCols)
	  {
		  UT_sint32 i;

		  i = m_iCols;
	      m_iCols = n_cols;
		  UT_VECTOR_PURGEALL(fp_TableRowColumn *, m_vecColumns);
		  m_vecColumns.clear();
		  for(i=0; i< m_iCols; i++)
		  {
			  m_vecColumns.addItem((void*) new fp_TableRowColumn());
			  getNthCol(i)->requisition = 0;
			  getNthCol(i)->allocation = 0;
			  getNthCol(i)->spacing = m_iColSpacing;
			  getNthCol(i)->need_expand = 0;
			  getNthCol(i)->need_shrink = 0;
			  getNthCol(i)->expand = 0;
			  getNthCol(i)->shrink = 0;
		  }
	  }
  }
  UT_DEBUGMSG(("SEVIOR: m_iRowSpacing = %d \n",m_iRowSpacing));
}

bool fp_TableContainer::isVBreakable(void)
{
	return true;
}

fp_ContainerObject * fp_TableContainer::VBreakAt(UT_sint32 vpos)
{
	return NULL;
}

UT_sint32 fp_TableContainer::wantVBreakAt(UT_sint32 vpos)
{
	return 0;
}


fp_Container * fp_TableContainer::getNextContainerInSection() const
{

	fl_ContainerLayout * pCL = (fl_ContainerLayout *) getSectionLayout();
	fl_ContainerLayout * pNext = pCL->getNext();
	if(pNext)
	{
		return pNext->getFirstContainer();
	}
	return NULL;
}


fp_Container * fp_TableContainer::getPrevContainerInSection() const
{

	fl_ContainerLayout * pCL = (fl_ContainerLayout *) getSectionLayout();
	fl_ContainerLayout * pPrev = pCL->getPrev();
	if(pPrev)
	{
		return pPrev->getLastContainer();
	}
	return NULL;
}


void fp_TableContainer::tableAttach (fp_CellContainer *child)
{
	UT_sint32 count = countCons();
	if(count > 0)
	{
		fp_Container * pLast = (fp_Container *) getNthCon(count - 1);
		pLast->setNext(child);
		child->setPrev(pLast);
	}
    if (child->getRightAttach() >= m_iCols)
	{
		resize (m_iRows, child->getRightAttach());
	}

	if (child->getBottomAttach() >=  m_iRows)
	{
		resize (child->getBottomAttach(), m_iCols);
	}
	UT_DEBUGMSG(("SEVIOR: Attaching cell %x to table \n",child));
	addContainer(child);
	child->setContainer((fp_Container *) this);
	queueResize();
}

void fp_TableContainer::setContainer(fp_Container * pContainer)
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
	setWidth(pContainer->getWidth());
#ifndef WITH_PANGO
	setWidthInLayoutUnits(pContainer->getWidthInLayoutUnits());
#endif
}


void fp_TableContainer::setRowSpacing (UT_sint32 row, UT_sint32  spacing)
{
  if (getNthRow(row)->spacing != spacing)
  {
      getNthRow(row)->spacing = spacing;
	  queueResize();
  }
}

void fp_TableContainer::setColSpacing(UT_sint32 column,UT_sint32 spacing)
{
  if (getNthCol(column)->spacing != spacing)
  {
      getNthCol(column)->spacing = spacing;
	  queueResize();
  }
}

void fp_TableContainer::setRowSpacings ( UT_sint32 spacing)
{
    UT_sint32 row;
	m_iRowSpacing = spacing;
	for (row = 0; row < m_iRows; row++)
	{
		getNthRow(row)->spacing = spacing;
	}
	queueResize();
}

void fp_TableContainer::setColSpacings (UT_sint32  spacing)
{
  UT_sint32 col;
  m_iColSpacing = spacing;
  for (col = 0; col < m_iCols; col++)
  {
	  getNthCol(col)->spacing = spacing;
  }
  queueResize();
}

void fp_TableContainer::setHomogeneous (bool bIsHomogeneous)
{
  if (bIsHomogeneous != m_bIsHomogeneous)
  {
      m_bIsHomogeneous = bIsHomogeneous;
	  queueResize();
  }
}

void fp_TableContainer::setBorderWidth(UT_sint32 iBorder)
{
	if(iBorder == m_iBorderWidth)
	{
		return;
	}
	m_iBorderWidth = iBorder;
	queueResize();
}

void fp_TableContainer::queueResize(void)
{
	static_cast<fl_TableLayout *>(getSectionLayout())->setDirty();
}
void fp_TableContainer::layout(void)
{
	static fp_Requisition requisition;
	static fp_Allocation alloc;
	sizeRequest(&requisition);
	alloc.x = getX();
	alloc.y = getY();
	alloc.width = getWidthInLayoutUnits();
	alloc.height = requisition.height;
	sizeAllocate(&alloc);
	setToAllocation();
}

void fp_TableContainer::setToAllocation(void)
{
	setWidthInLayoutUnits(m_MyAllocation.width);
	double dHeightLO = (double) m_MyAllocation.height ;
	double scale = SCALE_TO_SCREEN;
	dHeightLO = dHeightLO / scale;
	UT_sint32 iHeightLO = (UT_sint32) dHeightLO;
	setHeightLayoutUnits(iHeightLO);
	setMaxHeightInLayoutUnits(iHeightLO);
	setWidth(m_MyAllocation.width * SCALE_TO_SCREEN);
	setHeight(m_MyAllocation.height);
	setMaxHeight(m_MyAllocation.height);

	fp_CellContainer * pCon = (fp_CellContainer *) getNthCon(0);
	while(pCon)
	{
		pCon->setToAllocation();
		pCon = (fp_CellContainer *) pCon->getNext();
	}
}

void  fp_TableContainer::_size_request_init(void)
{
  UT_sint32 row, col;
  
  for (row = 0; row < m_iRows; row++)
  {
	  getNthRow(row)->requisition = 0;
  }
  for (col = 0; col < m_iCols; col++)
  {
	  getNthCol(col)->requisition = 0;
  }

  fp_CellContainer * pCell = (fp_CellContainer *) getNthCon(0);
  while (pCell)
  {
	  UT_ASSERT(pCell->getContainerType() == FP_CONTAINER_CELL);
	  pCell->sizeRequest(NULL);
	  pCell = (fp_CellContainer *) pCell->getNext();
  }
}

void  fp_TableContainer::_drawBoundaries(dg_DrawArgs* pDA)
{
    UT_ASSERT(pDA->pG == getGraphics());
	UT_ASSERT(getPage());
	if(getPage() == NULL)
	{
		return;
	}
	if(getPage()->getDocLayout()->getView() == NULL)
	{
		return;
	}
    if(getPage()->getDocLayout()->getView()->getShowPara() && getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN)){
        UT_sint32 xoffBegin = pDA->xoff - 1 + getX();
        UT_sint32 yoffBegin = pDA->yoff - 1;
        UT_sint32 xoffEnd = pDA->xoff + getX() + getWidth() + 2;
        UT_sint32 yoffEnd = pDA->yoff + getHeight() + 2;

		UT_RGBColor clrShowPara(127,127,127);
		getGraphics()->setColor(clrShowPara);
		xxx_UT_DEBUGMSG(("SEVIOR: Table Top (getY()) = %d \n",getY()));
		xxx_UT_DEBUGMSG(("SEVIOR: Table boundaries xleft %d xright %d ytop %d ybot %d \n",xoffBegin,xoffEnd,yoffBegin,yoffEnd));
        getGraphics()->drawLine(xoffBegin, yoffBegin, xoffEnd, yoffBegin);
        getGraphics()->drawLine(xoffBegin, yoffEnd, xoffEnd, yoffEnd);
        getGraphics()->drawLine(xoffBegin, yoffBegin, xoffBegin, yoffEnd);
        getGraphics()->drawLine(xoffEnd, yoffBegin, xoffEnd, yoffEnd);
    }

}


void  fp_TableContainer::_size_request_pass1(void)
{
  UT_sint32 width;
  UT_sint32 height;
  
  fp_CellContainer * child = (fp_CellContainer *) getNthCon(0);
  while (child)
  {

	  fp_Requisition child_requisition;
	  child->sizeRequest(&child_requisition);

	  /* Child spans a single column.
	   */
	  if (child->getLeftAttach() == (child->getRightAttach() - 1))
	  {
	      width = child_requisition.width + child->getLeftPad() + child->getRightPad();
	      getNthCol(child->getLeftAttach())->requisition = UT_MAX (getNthCol(child->getLeftAttach())->requisition, width);
	  }
	  
	  /* Child spans a single row.
	   */
	  if (child->getTopAttach() == (child->getBottomAttach() - 1))
	  {
	      height = child_requisition.height + child->getTopPad() + child->getBotPad();
	      getNthRow(child->getTopAttach())->requisition = UT_MAX (getNthRow(child->getTopAttach())->requisition, height);
	  }
	  child = (fp_CellContainer *) child->getNext();
  }
}

void  fp_TableContainer::clearScreen(void)
{
	fp_Container * pCell = (fp_Container *) getNthCon(0);
	while(pCell)
	{
		pCell->clearScreen();
		pCell = (fp_Container *) pCell->getNext();
	}
}

void fp_TableContainer::draw(dg_DrawArgs* pDA)
{
	fp_Container * pCell = (fp_Container *) getNthCon(0);
	while(pCell)
	{
		pCell->draw(pDA);
		pCell = (fp_Container *) pCell->getNext();
	}
    _drawBoundaries(pDA);

}

void  fp_TableContainer::_size_request_pass2(void)
{
  UT_sint32 max_width;
   UT_sint32 col;
  
  if (m_bIsHomogeneous)
  {
      max_width = 0;
      
      for (col = 0; col < m_iCols; col++)
	  {
		  max_width = UT_MAX (max_width, getNthCol(col)->requisition);
	  }
      for (col = 0; col < m_iCols; col++)
	  {
		  getNthCol(col)->requisition = max_width;
	  }
//
// Don't want homogeneous in height
//
#if 0
      UT_sint32 max_height = 0;
	  UT_sint32 row = 0;
	  for (row = 0; row < m_iRows; row++)
	  {
		  max_height = UT_MAX (max_height, getNthRow(row)->requisition);
      }
      for (row = 0; row < m_iRows; row++)
	  {
		  getNthRow(row)->requisition = max_height;
	  }
#endif
  }
}

void  fp_TableContainer::_size_request_pass3(void)
{
  fp_CellContainer  *child;
  UT_sint32 width, height;
  UT_sint32 row, col;
  UT_sint32 extra;
  
  child = (fp_CellContainer *) getNthCon(0);
  while (child)
  {
	  /* Child spans multiple columns.
	   */
	  if (child->getLeftAttach() != (child->getRightAttach() - 1))
	  {
	      fp_Requisition child_requisition;

	      child->sizeRequest(&child_requisition);
	      
	      /* Check and see if there is already enough space
	       *  for the child.
	       */
	      width = 0;
	      for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
		  {
			  width += getNthCol(col)->requisition;
			  if ((col + 1) < child->getRightAttach())
			  {
				  width += getNthCol(col)->spacing;
			  }
		  }
	      
	      /* If we need to request more space for this child to fill
	       *  its requisition, then divide up the needed space evenly
	       *  amongst the columns it spans.
	       */
	      if (width < child_requisition.width + child->getLeftPad() + child->getRightPad())
		  {
			  width = child_requisition.width + child->getLeftPad() + child->getRightPad();
		  
			  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
			  {
				  extra = width / (child->getRightAttach() - col);
				  getNthCol(col)->requisition += extra;
				  width -= extra;
			  }
		  }
	  }
	  
	  /* Child spans multiple rows.
	   */
	  if (child->getTopAttach() != (child->getBottomAttach() - 1))
	  {
	      fp_Requisition child_requisition;

	      child->sizeRequest(&child_requisition);

	      /* Check and see if there is already enough space
	       *  for the child.
	       */
	      height = 0;
	      for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
		  {
			  height += getNthRow(row)->requisition;
			  if ((row + 1) < child->getBottomAttach())
				  height +=  getNthRow(row)->spacing;
		  }
	      
	      /* If we need to request more space for this child to fill
	       *  its requisition, then divide up the needed space evenly
	       *  amongst the columns it spans.
	       */
	      if (height < child_requisition.height + child->getTopPad() + child->getBotPad())
		  {
			  height = child_requisition.height + child->getTopPad() + child->getBotPad() - height;
		  
			  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
			  {
				  extra = height / (child->getBottomAttach() - row);
				  getNthRow(row)->requisition += extra;
				  height -= extra;
			  }
		  }
	  }
	  child = (fp_CellContainer *) child->getNext();
    }
}

void  fp_TableContainer::_size_allocate_init(void)
{
  fp_CellContainer * child;
  UT_sint32 row, col;
  UT_sint32 has_expand;
  UT_sint32 has_shrink;
  
  /* Initialize the rows and cols.
   *  By default, rows and cols do not expand and do shrink.
   *  Those values are modified by the children that occupy
   *  the rows and cols.
   */
  for (col = 0; col < m_iCols; col++)
  {
      getNthCol(col)->allocation = getNthCol(col)->requisition;
      getNthCol(col)->need_expand = false;
      getNthCol(col)->need_shrink = true;
      getNthCol(col)->expand = false;
      getNthCol(col)->shrink = true;
      getNthCol(col)->empty = true;
  }
  for (row = 0; row < m_iRows; row++)
  {
      getNthRow(row)->allocation = getNthRow(row)->requisition;
      getNthRow(row)->need_expand = false;
      getNthRow(row)->need_shrink = true;
      getNthRow(row)->expand = false;
      getNthRow(row)->shrink = true;
      getNthRow(row)->empty = true;
  }
  
  /* Loop over all the children and adjust the row and col values
   *  based on whether the children want to be allowed to expand
   *  or shrink. This loop handles children that occupy a single
   *  row or column.
   */
  child = (fp_CellContainer *) getNthCon(0);
  while (child)
  {
	  if (child->getLeftAttach() == (child->getRightAttach() - 1))
	  {
		  if (child->getXexpand())
		  {
			  getNthCol(child->getLeftAttach())->expand = true;
		  }
		  if (!child->getXshrink())
		  {
			  getNthCol(child->getLeftAttach())->shrink = false;
		  }
		  getNthCol(child->getLeftAttach())->empty = false;
	  }
	  
	  if (child->getTopAttach() == (child->getBottomAttach() - 1))
	  {
		  if (child->getYshrink())
		  {
			  getNthRow(child->getTopAttach())->expand = true;
		  }
		  if (!child->getYshrink())
		  {			
			  getNthRow(child->getTopAttach())->shrink = false;
		  }
		  getNthRow(child->getTopAttach())->empty = false;
	  }
	  child = (fp_CellContainer *) child->getNext();
  }
  
  /* Loop over all the children again and this time handle children
   *  which span multiple rows or columns.
   */
  child = (fp_CellContainer *) getNthCon(0);
  while (child)
  {
	  if (child->getLeftAttach() != (child->getRightAttach() - 1))
	  {
		  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
		  {
			  getNthCol(col)->empty = false;
		  }
		  if (child->getXexpand())
		  {
			  has_expand = false;
			  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
			  {
				  if (getNthCol(col)->expand)
				  {
					  has_expand = true;
					  break;
				  }
			  }
			  if (!has_expand)
			  {
				  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
				  {
					  getNthCol(col)->need_expand = true;
				  }
			  }
		  }
			  
		  if (!child->getXshrink())
		  {
			  has_shrink = true;
			  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
			  {
				  if (!getNthCol(col)->shrink)
				  {
					  has_shrink = false;
					  break;
				  }
			  }
			  if (has_shrink)
			  {
				  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
				  {
						  getNthCol(col)->need_shrink = false;
				  }
			  }
		  }
	  
		  if (child->getTopAttach() != (child->getBottomAttach() - 1))
		  {
			  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
			  {
				  getNthRow(row)->empty = false;
			  }
			  if (child->getYexpand())
			  {
				  has_expand = false;
				  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
				  {
					  if (getNthRow(row)->expand)
					  {
						  has_expand = true;
						  break;
					  }
				  }		  
				  if (!has_expand)
				  {
					  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
						{
							getNthRow(row)->need_expand = true;
						}
				  }
			  }
	      
			  if (!child->getYshrink())
			  {
				  has_shrink = true;
				  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
				  {
					  if (!getNthRow(row)->shrink)
					  {
						  has_shrink = false;
						  break;
					  }
				  }
				  if (has_shrink)
				  {
					  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
					  {
						  getNthRow(row)->need_shrink = false;
					  }
				  }
			  }
		  }
	  }
	  child = (fp_CellContainer *) child->getNext();
  }
  
  /* Loop over the columns and set the expand and shrink values
   *  if the column can be expanded or shrunk.
   */
  for (col = 0; col < m_iCols; col++)
  {
      if (getNthCol(col)->empty)
	  {
		  getNthCol(col)->expand = false;
		  getNthCol(col)->shrink = false;
	  }
      else
	  {
		  if (getNthCol(col)->need_expand)
		  {
			  getNthCol(col)->expand = true;
		  }
		  if (!getNthCol(col)->need_shrink)
		  {
			  getNthCol(col)->shrink = false;
		  }
	  }
  }
  
  /* Loop over the rows and set the expand and shrink values
   *  if the row can be expanded or shrunk.
   */
  for (row = 0; row < m_iRows; row++)
  {
      if (getNthRow(row)->empty)
	  {
		  getNthRow(row)->expand = false;
		  getNthRow(row)->shrink = false;
	  }
      else
	  {
		  if (getNthRow(row)->need_expand)
		  {
			  getNthRow(row)->expand = true;
		  }
		  if (!getNthRow(row)->need_shrink)
		  {
			  getNthRow(row)->shrink = false;
		  }
	  }
  }
}

void  fp_TableContainer::_size_allocate_pass1(void)
{

  UT_sint32 real_width;
  UT_sint32 real_height;
  UT_sint32 width, height;
  UT_sint32 row, col;
  UT_sint32 nexpand;
  UT_sint32 nshrink;
  UT_sint32 extra;
  
  /* If we were allocated more space than we requested
   *  then we have to expand any expandable rows and columns
   *  to fill in the extra space.
   */
  
  real_width = m_MyAllocation.width - m_iBorderWidth * 2;
  real_height = m_MyAllocation.height - SCALE_TO_SCREEN*m_iBorderWidth * 2;
  
  if (m_bIsHomogeneous)
  {
      nexpand = 0;
      for (col = 0; col < m_iCols; col++)
	  {
		  if (getNthCol(col)->expand)
		  {
			  nexpand += 1;
			  break;
		  }
	  }
      
      if (nexpand > 0)
	  {
		  width = real_width;
	  
		  for (col = 0; col + 1 < m_iCols; col++)
		  {
			  width -= getNthCol(col)->spacing;
		  }
	  
		  for (col = 0; col < m_iCols; col++)
		  {
			  extra = width / (m_iCols - col);
			  getNthCol(col)->allocation = UT_MAX (1, extra);
			  width -= extra;
		  }
	  }
  }
  else
  {
      width = 0;
      nexpand = 0;
      nshrink = 0;
      
      for (col = 0; col < m_iCols; col++)
	  {
		  width += getNthCol(col)->requisition;
		  if (getNthCol(col)->expand)
		  {
			  nexpand += 1;
		  }
		  if (getNthCol(col)->shrink)
		  {
			  nshrink += 1;
		  }
	  }
      for (col = 0; col + 1 < m_iCols; col++)
	  {
		  width += getNthCol(col)->spacing;
      }
      /* Check to see if we were allocated more width than we requested.
       */
      if ((width < real_width) && (nexpand >= 1))
	  {
		  width = real_width - width;
	  
		  for (col = 0; col < m_iCols; col++)
		  {
			  if (getNthCol(col)->expand)
			  {
				  extra = width / nexpand;
				  getNthCol(col)->allocation += extra;
				  width -= extra;
				  nexpand -= 1;
			  }
		  }
	  }
      
      /* Check to see if we were allocated less width than we requested,
       * then shrink until we fit the size give.
       */
      if (width > real_width)
	  {
		  UT_sint32 total_nshrink = nshrink;

		  extra = width - real_width;
		  while (total_nshrink > 0 && extra > 0)
		  {
			  nshrink = total_nshrink;
			  for (col = 0; col < m_iCols; col++)
			  {
				  if (getNthCol(col)->shrink)
				  {
					  UT_sint32 allocation = getNthCol(col)->allocation;
					  getNthCol(col)->allocation = UT_MAX (1, (UT_sint32) getNthCol(col)->allocation - extra / nshrink);
					  extra -= allocation - getNthCol(col)->allocation;
					  nshrink -= 1;
					  if (getNthCol(col)->allocation < 2)
					  {
						  total_nshrink -= 1;
						  getNthCol(col)->shrink = false;
					  }
				  }
			  }
		  }
	  }
  }
  
//
// Don't want homogenous in height
//
  if (m_bIsHomogeneous && false)
  {
	  nexpand = 0;
	  for (row = 0; row < m_iRows; row++)
	  {
		  if (getNthRow(row)->expand)
		  {
			  nexpand += 1;
			  break;
		  }
	  }
      
	  if (nexpand > 0)
	  {
		  height = real_height;
		  for (row = 0; row + 1 < m_iRows; row++)
		  {
			  height -= getNthRow(row)->spacing;
		  }
		  for (row = 0; row < m_iRows; row++)
		  {
			  extra = height / (m_iRows - row);
			  getNthRow(row)->allocation = UT_MAX (1, extra);
			  height -= extra;
		  }
	  }
  }
  else
  {
	  height = 0;
	  nexpand = 0;
	  nshrink = 0;
	  for (row = 0; row < m_iRows; row++)
	  {
		  height += getNthRow(row)->requisition;
		  if (getNthRow(row)->expand)
		  {
			  nexpand += 1;
		  }
		  if (getNthRow(row)->shrink)
		  {
			  nshrink += 1;
		  }
	  }
	  for (row = 0; row + 1 < m_iRows; row++)
	  {
		  height += getNthRow(row)->spacing;
	  }      
      /* Check to see if we were allocated more height than we requested.
       */
      if ((height < real_height) && (nexpand >= 1))
	  {
		  height = real_height - height;
		  for (row = 0; row < m_iRows; row++)
		  {
			  if (getNthRow(row)->expand)
			  {
				  extra = height / nexpand;
				  getNthRow(row)->allocation += extra;
				  height -= extra;
				  nexpand -= 1;
			  }
		  }
	  }
      
      /* Check to see if we were allocated less height than we requested.
       * then shrink until we fit the size give.
       */
      if (height > real_height)
	  {
		  UT_sint32 total_nshrink = nshrink;
		  extra = height - real_height;
		  while (total_nshrink > 0 && extra > 0)
		  {
			  nshrink = total_nshrink;
			  for (row = 0; row < m_iRows; row++)
			  {
				  if (getNthRow(row)->shrink)
				  {
					  UT_sint32 allocation = getNthRow(row)->allocation;
		    
					  getNthRow(row)->allocation = UT_MAX (1, (UT_sint32) getNthRow(row)->allocation - extra / nshrink);
					  extra -= allocation - getNthRow(row)->allocation;
					  nshrink -= 1;
					  if (getNthRow(row)->allocation < 2)
					  {
						  total_nshrink -= 1;
						  getNthRow(row)->shrink = false;
					  }
				  }
			  }
		  }
	  }
  }
}

void  fp_TableContainer::_size_allocate_pass2(void)
{
  fp_CellContainer  *child;
  UT_sint32 max_width;
  UT_sint32 max_height;
  UT_sint32 x, y;
  UT_sint32 row, col;
  fp_Allocation allocation;
  
  child = (fp_CellContainer *) getNthCon(0);
  while (child)
  {
	  fp_Requisition child_requisition;
	  child->sizeRequest(&child_requisition);

	  x = m_MyAllocation.x + m_iBorderWidth;
	  y = m_MyAllocation.y + m_iBorderWidth * SCALE_TO_SCREEN;
	  max_width = 0;
	  max_height = 0;
	  
	  for (col = 0; col < child->getLeftAttach(); col++)
	  {
		  x += getNthCol(col)->allocation;
		  x += getNthCol(col)->spacing;
	  }
	  
	  for (col = child->getLeftAttach(); col < child->getRightAttach(); col++)
	  {
		  max_width += getNthCol(col)->allocation;
		  if ((col + 1) < child->getRightAttach())
		  {
			  max_width += getNthCol(col)->spacing;
		  }
	  }
	  
	  for (row = 0; row < child->getTopAttach(); row++)
	  {
		  y += getNthRow(row)->allocation;
		  y += getNthRow(row)->spacing;
	  }
	  
	  for (row = child->getTopAttach(); row < child->getBottomAttach(); row++)
	  {
		  max_height += getNthRow(row)->allocation;
		  if ((row + 1) < child->getBottomAttach())
		  {
			  max_height += getNthRow(row)->spacing;
		  }
	  }
	  
	  if (child->getXfill())
	  {
		  allocation.width = UT_MAX (1, max_width - (UT_sint32)child->getLeftPad() - child->getRightPad());
		  allocation.x = x + (max_width - allocation.width) / 2;
	  }
	  else
	  {
		  allocation.width = child_requisition.width;
		  allocation.x = x + (max_width - allocation.width) / 2;
	  }
	  // fixme sevior look here!!!
	  if (child->getYfill())
	  {
		  allocation.height = UT_MAX (1, max_height - (UT_sint32)child->getTopPad() - child->getBotPad());
//		  allocation.height = max_height;
//		  allocation.y = y + (max_height - allocation.height) / 2;
		  allocation.y = y;
	  }
	  else
	  {
		  allocation.height = child_requisition.height;
//		  allocation.y = y + (max_height - allocation.height) / 2;
		  allocation.y = y;
	  }
	  UT_DEBUGMSG(("SEVIOR: max_height = %d height =%d \n",max_height,allocation.height));
	  child->sizeAllocate( &allocation);
	  child = (fp_CellContainer *) child->getNext();
  }
}

fp_TableRowColumn * fp_TableContainer::getNthCol(UT_sint32 i)
{
	UT_ASSERT(i < (UT_sint32) m_vecColumns.getItemCount());
	return (fp_TableRowColumn *) m_vecColumns.getNthItem(i);
}

fp_TableRowColumn * fp_TableContainer::getNthRow(UT_sint32 i)
{
	UT_ASSERT(i < (UT_sint32) m_vecRows.getItemCount());
	return (fp_TableRowColumn *) m_vecRows.getNthItem(i);
}


void fp_TableContainer::sizeRequest(fp_Requisition * pRequisition)
{
  UT_sint32 row, col;
  
  pRequisition->width = 0;
  pRequisition->height = 0;
  
  _size_request_init ();
  _size_request_pass1 ();
  _size_request_pass2 ();
  _size_request_pass3 ();
  _size_request_pass2 ();
  
  for (col = 0; col < m_iCols; col++)
  {
	  pRequisition->width += getNthCol(col)->requisition;
  }
  for (col = 0; col + 1 < m_iCols; col++)
  {
	  pRequisition->width += getNthCol(col)->spacing;
  }
  for (row = 0; row < m_iRows; row++)
  {
	  pRequisition->height += getNthRow(row)->requisition;
	  UT_DEBUGMSG(("SEVIOR: requisition height %d \n", pRequisition->height));
  }
  for (row = 0; row + 1 < m_iRows; row++)
  {
	  pRequisition->height += getNthRow(row)->spacing;
	  UT_DEBUGMSG(("SEVIOR: requisition height 2 is %d \n", pRequisition->height));
  }
}

void fp_TableContainer::sizeAllocate(fp_Allocation * pAllocation)
{
	m_MyAllocation.width = pAllocation->width;
	m_MyAllocation.height = pAllocation->height;
	m_MyAllocation.x = pAllocation->x;
	m_MyAllocation.y = pAllocation->y;
	if(getContainer()->getContainerType() == FP_CONTAINER_COLUMN)
	{
//
// This is the topmost container. All offsets are reltive to this. Come
// drawing time the cells will be passed an offset relative to this.
//
		m_MyAllocation.y = 0;
	}
	UT_DEBUGMSG(("SEVIOR: Initial allocation height is %d \n", pAllocation->height));
	
	_size_allocate_init ();
	UT_DEBUGMSG(("SEVIOR: Initial allocation height 1 is %d \n", m_MyAllocation.height));
	_size_allocate_pass1 ();
	UT_DEBUGMSG(("SEVIOR: Initial allocation height 2 is %d \n", m_MyAllocation.height));
	_size_allocate_pass2 ();
	UT_DEBUGMSG(("SEVIOR: Initial allocation height 3 is %d \n", m_MyAllocation.height));
//	fp_Requisition pReq;
//	sizeRequest(&pReq);
//	m_MyAllocation.height = pReq.height;
}




