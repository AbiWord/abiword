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
fp_CellContainer::fp_CellContainer(fl_SectionLayout* pSectionLayout) : fp_VerticalContainer(FP_CONTAINER_CELL, pSectionLayout)
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
	if (iWidth == m_iWidth)
	{
		return;
	}
	m_iWidth = iWidth;
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

void fp_CellContainer::sizeRequest(fp_Request * pRequest)
{
	UT_sint32 count = countCons();
	UT_sint32 i =0;
	UT_sint32 height = 0;
	UT_sint32 width = 0;
	for(i=0 ; i < count; i++)
	{
		fp_Container * pCon = getNthCon(i);
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
			fp_Request pReq;
			if(pCon->getType() == FP_CONTAINER_TABLE)
			{
				static_cast<fp_TableContainer *>(pCon)->sizeRequest(&pReq);
			}
			else if(pCon->getType() == FP_CONTAINER_CELL)
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
		}
	}
	pRequest->width = width;
	pRequest->height = height;
	m_MyRequest.width = width;
	m_MyRequest.height = height;
}

void fp_CellContainer::sizeAllocate(fp_Allocate * pAllocate)
{
	m_MyAllocation.width = pAllocate->width;
	m_MyAllocation.height = pAllocate->height;
	m_MyAllocation.x = pAllocate->x;
	m_MyAllocation.y = pAllocate->y;
}
