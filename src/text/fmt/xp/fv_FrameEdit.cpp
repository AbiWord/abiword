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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "fv_FrameEdit.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_Graphics.h"

FV_FrameEdit::FV_FrameEdit (FV_View * pView)
	: m_pView (pView), 
	  m_iFrameEditMode(FV_FrameEdit_NOT_ACTIVE),
	  m_pFrameLayout(NULL),
	  m_pFrameContainer(NULL),
	  m_iDraggingWhat;( FV_FrameEdit_DragNothing),
	  m_iLastX(0),
	  m_iLastY(0),
	  m_recCurFrame(0,0,0,0),
	  m_bBoxOnOff
{
	UT_ASSERT (pView);
}

FV_FrameEdit::~FV_FrameEdit()
{
}

bool FV_FrameEdit::isActive(void) const
{
	return (FV_FrameEdit_NOT_ACTIVE != m_iFrameEditMode);
}

void FV_FrameEdit::setMode(FV_FrameEditMode iEditMode)
{
	m_iFrameEditMode = iEditMode;
}

void FV_FrameEdit::mouseDrag(UT_sint32 x, UT_sint32 y)
{

// Should clear the old box

	_xorBox(m_recCurFrame);
	UT_sint32 diffx = 0;
	UT_sint32 diffy = 0;
	switch (m_iDraggingWhat)
	{
	case FV_FrameEdit_DragTopLeftCorner:
		diffx = m_recCurFrame.left - x;
		diffy = m_recCurFrame.top - y;
		m_recCurFrame.left -= diffx;
		m_recCurFrame.top -= diffy;
		m_recCurFrame.width += diffx;
		m_recCurFrame.height += diffy;
		if(m_recCurFrame.width < 0)
		{
			m_recCurFrame.left = x;
			m_recCurFrame.width = -m_recCurFrame.width;
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
		}
		break;
	case FV_FrameEdit_DragTopRightCorner:
		diffx = m_recCurFrame.left + m_recCurFrame.width - x;
		diffy = m_recCurFrame.top - y;
		m_recCurFrame.top -= diffy;
		m_recCurFrame.width -= diffx;
		m_recCurFrame.height += diffy;
		if(m_recCurFrame.width < 0)
		{
			m_recCurFrame.left = x;
			m_recCurFrame.width = -m_recCurFrame.width;
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
		}
		break;
	case FV_FrameEdit_DragBotLeftCorner:
		diffx = m_recCurFrame.left - x;
		diffy = m_recCurFrame.top + m_recCurFrame.height - y;
		m_recCurFrame.left -= diffx;
		m_recCurFrame.width += diffx;
		m_recCurFrame.height -= diffy;
		if(m_recCurFrame.width < 0)
		{
			m_recCurFrame.left = x;
			m_recCurFrame.width = -m_recCurFrame.width;
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
		}
		break;
	case FV_FrameEdit_DragBotRightCorner:
		diffx = m_recCurFrame.left + m_recCurFrame.width - x;
		diffy = m_recCurFrame.top + m_recCurFrame.height - y;
		m_recCurFrame.width -= diffx;
		m_recCurFrame.height -= diffy;
		if(m_recCurFrame.width < 0)
		{
			m_recCurFrame.left = x;
			m_recCurFrame.width = -m_recCurFrame.width;
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
		}
		break;
	case FV_FrameEdit_DragLeftEdge:
		diffx = m_recCurFrame.left - x;
		m_recCurFrame.left -= diffx;
		m_recCurFrame.width += diffx;
		if(m_recCurFrame.width < 0)
		{
			m_recCurFrame.left = x;
			m_recCurFrame.width = -m_recCurFrame.width;
		}
		break;
	case FV_FrameEdit_DragRightEdge:
		diffx = m_recCurFrame.left + m_recCurFrame - x;
		m_recCurFrame.width -= diffx;
		if(m_recCurFrame.width < 0)
		{
			m_recCurFrame.left = x;
			m_recCurFrame.width = -m_recCurFrame.width;
		}
		break;
	case FV_FrameEdit_DragTopEdge:
		diffy = m_recCurFrame.top - y;
		m_recCurFrame.top -= diffy;
		m_recCurFrame.height += diffy;
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
		}
		break;
	case FV_FrameEdit_DragBotEdge:
		diffy = m_recCurFrame.top + m_recCurFrame.height - y;
		m_recCurFrame.height -= diffy;
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
		}
		break;
	case FV_FrameEdit_DragWholeFrame:
		diffx = m_iLastX - x;
		diffy = m_iLastY - y;
		m_recCurFrame.left -= diffx;
		m_recCurFrame.top -= diffy;
	default:
		break;
	}

// Should draw the new box

	_xorBox(m_recCurFrame);
	m_iLastX = x;
	m_iLastY = y;
}

/*!
 * This method finds the correct frame on the screen associated with the
 * (x,y) point.
 * It sets the FV_FrameEditMode and the FV_FrameEditDragWhat mode depending
 * on which control is selected at the (x,y) position.
 * 
 *        top left     top edge     top right
 *             X----------X----------X
 *             |                     |
 *             |                     |
 * left edge   X                     X right edge
 *             |                     |
 *             |                     |
 *             X----------X----------X
 *        Bot left    Bot edge    Bot right
 *
 * An (x,y) point that does not fall in a specified control sets drag 
 * whole frame mode.
 *
 * The method also sets the fl_FrameLayout and FP_FrameContainer pointers. 
 */
void FV_FrameEdit::setDragType(UT_sint32 x, UT_sint32 y)
{
}

void FV_FrameEdit::mouseLeftPress(UT_sint32 x, UT_sint32 y)
{
	if(!isActive())
	{
		setDragType(x,y);
	}
}


void FV_FrameEdit::mouseRelease(UT_sint32 x, UT_sint32 y)
{

}

FV_FrameEditDragWhat FV_FrameEdit::mouseMotion(UT_sint32 x, UT_sint32 y)
{
	return getFrameEditDragWhat();
}

void FV_FrameEdit::drawFrame(void)
{

}

void FV_FrameEdit::_xorBox(UT_Rect & rec)
{
	xorRect(getGraphics(),rec);
	m_bBoxOnOff = !m_bOnOff;
}
