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
#include "gr_DrawArgs.h"
#include "ut_vector.h"
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"

fp_TableRowColumn::fp_TableRowColumn(void) :
		requisition(0),
        allocation(0),
        spacing(0),
        need_expand(true),
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
	  m_bYexpand(true),
	  m_bXshrink(false),
	  m_bYshrink(true),
	  m_bXfill(true),
	  m_bYfill(true)
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

/*!
  Set width
  \param iWidth Width of container
  \todo Should force re-line-break operations on all blocks in the
        container
 */
void fp_CellContainer::setWidth(UT_sint32 iWidth)
{
	if (iWidth == fp_VerticalContainer::getWidth())
	{
		return;
	}
	fp_VerticalContainer::setWidth(iWidth);
	getSectionLayout()->format();
}


/*!
 Draw container outline
 \param pDA Draw arguments
 */
void fp_CellContainer::_drawBoundaries(dg_DrawArgs* pDA)
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
        UT_sint32 xoffEnd = pDA->xoff + getWidth() + 2;
        UT_sint32 yoffEnd = pDA->yoff + getMaxHeight() + 2;

		UT_RGBColor clrShowPara(127,127,127);
		getGraphics()->setColor(clrShowPara);

        getGraphics()->drawLine(xoffBegin, yoffBegin, xoffEnd, yoffBegin);
        getGraphics()->drawLine(xoffBegin, yoffEnd, xoffEnd, yoffEnd);
        getGraphics()->drawLine(xoffBegin, yoffBegin, xoffBegin, yoffEnd);
        getGraphics()->drawLine(xoffEnd, yoffBegin, xoffEnd, yoffEnd);
    }
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
//
// Only draw the lines in the clipping region.
//
	for ( i = 0; (i<count && !bStop); i++)
	{
		fp_ContainerObject* pContainer = (fp_ContainerObject*) getNthCon(i);

		dg_DrawArgs da = *pDA;
		da.xoff += pContainer->getX();
		da.yoff += pContainer->getY();
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
			if(width < pCon->getWidth())
			{
				width = pCon->getWidth();
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
				static_cast<fp_TableContainer *>(pCon)->sizeRequest(&pReq);
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
}

void fp_CellContainer::sizeAllocate(fp_Allocation * pAllocate)
{
	m_MyAllocation.width = pAllocate->width;
	m_MyAllocation.height = pAllocate->height;
	m_MyAllocation.x = pAllocate->x;
	m_MyAllocation.y = pAllocate->y;
}

//---------------------------------------------------------------------

/*!
  Create Cell container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_TableContainer::fp_TableContainer(fl_SectionLayout* pSectionLayout) 
	: fp_VerticalContainer(FP_CONTAINER_TABLE, pSectionLayout),
	  m_iRows(0),
	  m_iCols(0),
	  m_iBorderwidth(0),
	  m_bIsHomogeneous(true)	  
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

//--------------------------------------------------------------
//-- Automatic chacges made here--------------------------------
//
void  fp_TableContainer::_size_request_pass2(void)
{
  UT_sint32 max_width;
  UT_sint32 max_height;
  UT_sint32 row, col;
  
  if (m_bIsHomogeneous)
  {
      max_width = 0;
      max_height = 0;
      
      for (col = 0; col < m_iCols; col++)
	  {
		  max_width = UT_MAX (max_width, getNthCol(col)->requisition);
	  }
	  for (row = 0; row < m_iRows; row++)
	  {
		  max_height = UT_MAX (max_height, getNthRow(row)->requisition);
      }
      for (col = 0; col < m_iCols; col++)
	  {
		  getNthCol(col)->requisition = max_width;
	  }
      for (row = 0; row < m_iRows; row++)
	  {
		  getNthRow(row)->requisition = max_height;
	  }
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
				  height += getNthRow(row)->spacing;
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
  
  real_width = m_MyAllocation.width - m_iBorderwidth * 2;
  real_height = m_MyAllocation.height - m_iBorderwidth * 2;
  
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
  
  if (m_bIsHomogeneous)
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

	  x = m_MyAllocation.x + m_iBorderwidth;
	  y = m_MyAllocation.y + m_iBorderwidth;
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
	  
	  if (child->getYfill())
	  {
		  allocation.height = UT_MAX (1, max_height - (UT_sint32)child->getTopPad() - child->getBotPad());
		  allocation.y = y + (max_height - allocation.height) / 2;
	  }
	  else
	  {
		  allocation.height = child_requisition.height;
		  allocation.y = y + (max_height - allocation.height) / 2;
	  }
	  
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
  }
  for (row = 0; row + 1 < m_iRows; row++)
  {
	  pRequisition->height += getNthRow(row)->spacing;
  }
}

void fp_TableContainer::sizeAllocate(fp_Allocation * pAllocation)
{
	m_MyAllocation = *pAllocation;
  
	_size_allocate_init ();
	_size_allocate_pass1 ();
	_size_allocate_pass2 ();
}
