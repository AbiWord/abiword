/* AbiWord
 * Copyright (c) 2003 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#include "fv_FrameEdit.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_DrawArgs.h"
#include "gr_Graphics.h"
#include "ut_units.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "fp_FrameContainer.h"
#include "fv_View.h"
#include "ev_Mouse.h"
#include "xap_Frame.h"
#include "gr_Painter.h"
#include "xap_App.h"

FV_Base::FV_Base( FV_View* pView )
// protected
: m_pView( pView )
, m_iGlobCount(0)
, m_recCurFrame(0,0,0,0)
, m_bFirstDragDone(false)
, m_iFirstEverX(0)
, m_iFirstEverY(0)
, m_xLastMouse(1)
, m_yLastMouse(1)
// private
, m_iDraggingWhat( FV_DragNothing)
{
}

FV_Base::~FV_Base()
{
}

PD_Document * FV_Base::getDoc(void) const
{
	return m_pView->getDocument();
}

FL_DocLayout * FV_Base::getLayout(void) const
{
	return m_pView->getLayout();
}

GR_Graphics * FV_Base::getGraphics(void) const
{
	return m_pView->getGraphics();
}

void FV_Base::_beginGlob(void)
{
	getDoc()->beginUserAtomicGlob();
	m_iGlobCount++;
	UT_DEBUGMSG(("Begin Glob count %d \n",m_iGlobCount));
}

void FV_Base::_endGlob(void)
{
	getDoc()->endUserAtomicGlob();
	m_iGlobCount--;
	UT_DEBUGMSG(("End Glob count %d \n",m_iGlobCount));
}

UT_sint32 FV_Base::getGlobCount() const
{
	return m_iGlobCount;
}

void FV_Base::_checkDimensions()
{
	// don't left widths and heights be too big
	double dWidth = static_cast<double>(m_recCurFrame.width)/static_cast<double>(UT_LAYOUT_RESOLUTION);
	double dHeight = static_cast<double>(m_recCurFrame.height)/static_cast<double>(UT_LAYOUT_RESOLUTION);
	if(m_pView->getPageSize().Width(DIM_IN) < dWidth)
	{
	  dWidth = m_pView->getPageSize().Width(DIM_IN)*0.99;
	  m_recCurFrame.width = static_cast<UT_sint32>(dWidth*UT_LAYOUT_RESOLUTION);
	}
	if(m_pView->getPageSize().Height(DIM_IN) < dHeight)
	{
	  dHeight = m_pView->getPageSize().Height(DIM_IN)*0.99;
	  m_recCurFrame.height = static_cast<UT_sint32>(dHeight*UT_LAYOUT_RESOLUTION);
	}
}

void FV_Base::mouseDrag(UT_sint32 x, UT_sint32 y)
{
  _mouseDrag(x,y);
}

void FV_Base::_doMouseDrag(UT_sint32 x, UT_sint32 y, UT_sint32& dx, UT_sint32& dy, UT_Rect& expX, UT_Rect& expY)
{
	if(!m_bFirstDragDone)
	{
		m_iFirstEverX = x;
		m_iFirstEverY = y;
	}
	m_bFirstDragDone = true;
	UT_sint32 diffx = 0;
	UT_sint32 diffy = 0;
	UT_sint32 iext = getGraphics()->tlu(3);
	m_xLastMouse = x;
	m_yLastMouse = y;
	switch (getDragWhat())
	{
	case FV_DragTopLeftCorner:
		diffx = m_recCurFrame.left - x;
		diffy = m_recCurFrame.top - y;
		m_recCurFrame.left -= diffx;
		m_recCurFrame.top -= diffy;
		dx = -diffx;
		dy = -diffy;
		m_recCurFrame.width += diffx;
		m_recCurFrame.height += diffy;
		if(diffx < 0)
		{
			expX.left = m_recCurFrame.left + diffx -iext;
			expX.width = -diffx + iext;
			if(diffy > 0)
			{
				expX.top -=  diffy + iext;
				expX.height += diffy + 2*iext;
			}
			else
			{
				expX.top -=  iext;
				expX.height += (-diffy + 2*iext);
			}
		}
		if(diffy < 0)
		{
			expY.top = m_recCurFrame.top + diffy - iext;
			expY.height = -diffy + 2*iext;
		}
		if(m_recCurFrame.width < 0)
		{
			m_recCurFrame.left = x;
			m_recCurFrame.width = -m_recCurFrame.width;
			setDragWhat( FV_DragTopRightCorner );
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			setDragWhat( FV_DragBotLeftCorner );
		}
		break;
	case FV_DragTopRightCorner:
		diffx = m_recCurFrame.left + m_recCurFrame.width - x;
		diffy = m_recCurFrame.top - y;
		m_recCurFrame.top -= diffy;
		dy = -diffy;
		m_recCurFrame.width -= diffx;
		m_recCurFrame.height += diffy;
		if(diffx > 0)
		{
			expX.left = m_recCurFrame.left + m_recCurFrame.width;
			expX.width = diffx + iext;
			if(diffy > 0)
			{
				expX.top -=  iext;
				expX.height += diffy + 2*iext;
			}
			else
			{
				expX.top -=  iext;
				expX.height += (-diffy + 2*iext);
			}
		}
		if(diffy < 0)
		{
			expY.top = m_recCurFrame.top + diffy - iext;
			expY.height = -diffy + iext;
		}
		if(m_recCurFrame.width < 0)
		{
			m_recCurFrame.left = x;
			m_recCurFrame.width = -m_recCurFrame.width;
			setDragWhat( FV_DragTopLeftCorner );
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			setDragWhat( FV_DragBotRightCorner );
		}
		break;
	case FV_DragBotLeftCorner:
		diffx = m_recCurFrame.left - x;
		diffy = m_recCurFrame.top + m_recCurFrame.height - y;
		m_recCurFrame.left -= diffx;
		dx = -diffx;
		m_recCurFrame.width += diffx;
		m_recCurFrame.height -= diffy;
		if(diffx < 0)
		{
			expX.left = m_recCurFrame.left + diffx -iext;
			expX.width = -diffx + iext;
			if(diffy > 0)
			{
				expX.top -=  diffy + iext;
				expX.height += diffy + 2*iext;
			}
			else
			{
				expX.top -=  iext;
				expX.height += (-diffy + 2*iext);
			}
		}
		if(diffy > 0)
		{
			expY.top = m_recCurFrame.top + m_recCurFrame.height - iext;
			expY.height = diffy + 2*iext;
		}
		if(m_recCurFrame.width < 0)
		{
			m_recCurFrame.left = x;
			m_recCurFrame.width = -m_recCurFrame.width;
			setDragWhat( FV_DragBotRightCorner );
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			setDragWhat( FV_DragTopLeftCorner );
		}
		break;
	case FV_DragBotRightCorner:
		diffx = m_recCurFrame.left + m_recCurFrame.width - x;
		diffy = m_recCurFrame.top + m_recCurFrame.height - y;
		m_recCurFrame.width -= diffx;
		m_recCurFrame.height -= diffy;
		if(diffx > 0)
		{
			expX.left = m_recCurFrame.left + m_recCurFrame.width;
			expX.width = diffx + iext;
			if(diffy > 0)
			{
				expX.top -=  iext;
				expX.height += diffy + 2*iext;
			}
			else
			{
				expX.top -=  iext;
				expX.height += (-diffy + 2*iext);
			}
		}
		if(diffy > 0)
		{
			expY.top = m_recCurFrame.top + m_recCurFrame.height;
			expY.height = diffy + iext;
		}
		if(m_recCurFrame.width < 0)
		{
			m_recCurFrame.left = x;
			m_recCurFrame.width = -m_recCurFrame.width;
			setDragWhat( FV_DragBotLeftCorner );
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			setDragWhat( FV_DragTopRightCorner );
		}
		break;
	case FV_DragLeftEdge:
		diffx = m_recCurFrame.left - x;
		m_recCurFrame.left -= diffx;
		dx = -diffx;
		m_recCurFrame.width += diffx;
		if(diffx < 0)
		{
			expX.left = m_recCurFrame.left + diffx - iext;
			expX.width = -diffx + iext;
			expX.top -=  iext;
			expX.height += 2*iext;
		}
		if(m_recCurFrame.width < 0)
		{
			m_recCurFrame.left = x;
			m_recCurFrame.width = -m_recCurFrame.width;
			setDragWhat( FV_DragRightEdge );
		}
		break;
	case FV_DragRightEdge:
		diffx = m_recCurFrame.left + m_recCurFrame.width - x;
		m_recCurFrame.width -= diffx;
		if(diffx > 0)
		{
			expX.left = m_recCurFrame.left + m_recCurFrame.width;
			expX.width = diffx + iext;
			expX.top -=  iext;
			expX.height += 2*iext;
		}
		if(m_recCurFrame.width < 0)
		{
			m_recCurFrame.left = x;
			m_recCurFrame.width = -m_recCurFrame.width;
			setDragWhat( FV_DragLeftEdge );
		}
		break;
	case FV_DragTopEdge:
		diffy = m_recCurFrame.top - y;
		m_recCurFrame.top -= diffy;
		dy = -diffy;
		m_recCurFrame.height += diffy;
		if(diffy < 0)
		{
			expY.top = m_recCurFrame.top + diffy - iext;
			expY.height = -diffy + iext;
			expY.left -= iext;
			expY.width += 2*iext;
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			setDragWhat( FV_DragBotEdge );
		}
		break;
	case FV_DragBotEdge:
		diffy = m_recCurFrame.top + m_recCurFrame.height - y;
		m_recCurFrame.height -= diffy;
		if(diffy > 0)
		{
			expY.top = m_recCurFrame.top + m_recCurFrame.height;
			expY.height = diffy + iext;
			expY.left -= iext;
			expY.width += 2*iext;
			xxx_UT_DEBUGMSG(("expY.top %d expY.height %d \n",expY.top,expY.height));
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			setDragWhat( FV_DragTopEdge );
		}
		break;
	default:
		break;
	}
}
