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
#include "fv_View.h"

FV_FrameEdit::FV_FrameEdit (FV_View * pView)
	: m_pView (pView), 
	  m_iFrameEditMode(FV_FrameEdit_NOT_ACTIVE),
	  m_pFrameLayout(NULL),
	  m_pFrameContainer(NULL),
	  m_iDraggingWhat( FV_FrameEdit_DragNothing),
	  m_iLastX(0),
	  m_iLastY(0),
	  m_recCurFrame(0,0,0,0),
	  m_bBoxOnOff(false)
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

PD_Document * FV_FrameEdit::getDoc(void) const
{
	return m_pView->getDocument();
}

FL_DocLayout * FV_FrameEdit::getLayout(void) const
{
	return m_pView->getLayout();
}

GR_Graphics * FV_FrameEdit::getGraphics(void) const
{
	return m_pView->getGraphics();
}

void FV_FrameEdit::setMode(FV_FrameEditMode iEditMode)
{
	m_iFrameEditMode = iEditMode;
}

void FV_FrameEdit::mouseDrag(UT_sint32 x, UT_sint32 y)
{

// Should clear the old box

	_xorBox(m_recCurFrame);
//
// Clear the initial mark.
//
	if(m_iFrameEditMode == FV_FrameEdit_WAIT_FOR_FIRST_CLICK_INSERT)
	{
		m_iFrameEditMode = FV_FrameEdit_RESIZE_INSERT;
		UT_sint32 len = getGraphics()->tlu(25);		
		UT_sint32 one = getGraphics()->tlu(1);
	
		getGraphics()->xorLine(m_iLastX-one,m_iLastY,m_iLastX-len,m_iLastY);
		getGraphics()->xorLine(m_iLastX,m_iLastY-one,m_iLastX,m_iLastY-len);
	}
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
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_NE);
			m_iDraggingWhat =  FV_FrameEdit_DragTopRightCorner;
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_SW);
			m_iDraggingWhat =  FV_FrameEdit_DragBotLeftCorner;
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
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_NW);
			m_iDraggingWhat =  FV_FrameEdit_DragTopLeftCorner;
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_SE);
			m_iDraggingWhat =  FV_FrameEdit_DragBotRightCorner;
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
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_SE);
			m_iDraggingWhat =  FV_FrameEdit_DragBotRightCorner;

		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_NW);
			m_iDraggingWhat =  FV_FrameEdit_DragTopLeftCorner;
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
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_SW);
			m_iDraggingWhat =  FV_FrameEdit_DragBotLeftCorner;
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_NE);
			m_iDraggingWhat =  FV_FrameEdit_DragTopRightCorner;
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
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_W);
			m_iDraggingWhat =  FV_FrameEdit_DragRightEdge;
		}
		break;
	case FV_FrameEdit_DragRightEdge:
		diffx = m_recCurFrame.left + m_recCurFrame.width - x;
		m_recCurFrame.width -= diffx;
		if(m_recCurFrame.width < 0)
		{
			m_recCurFrame.left = x;
			m_recCurFrame.width = -m_recCurFrame.width;
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_E);
			m_iDraggingWhat =  FV_FrameEdit_DragLeftEdge;
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
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_S);
			m_iDraggingWhat =  FV_FrameEdit_DragBotEdge;
		}
		break;
	case FV_FrameEdit_DragBotEdge:
		diffy = m_recCurFrame.top + m_recCurFrame.height - y;
		m_recCurFrame.height -= diffy;
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_N);
			m_iDraggingWhat =  FV_FrameEdit_DragTopEdge;
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

/*!
 * This method responds to a mouse click and decides what to do with it.
 */
void FV_FrameEdit::mouseLeftPress(UT_sint32 x, UT_sint32 y)
{
	if(!isActive())
	{
		setDragType(x,y);
		return;
	}
//
// Find the type of drag we should do.
//
	if(FV_FrameEdit_EXISTING_SELECTED == m_iFrameEditMode )
	{
		setDragType(x,y);
		return;
	}
//
// We're waiting for the first click to interactively eneter the initial
// frame size.
//
	if(	FV_FrameEdit_WAIT_FOR_FIRST_CLICK_INSERT == m_iFrameEditMode )
	{
//
// Draw a marker at the current position
//
		UT_sint32 len = getGraphics()->tlu(25);
		UT_sint32 one = getGraphics()->tlu(1);
		getGraphics()->xorLine(x-one,y,x-len,y);
		getGraphics()->xorLine(x,y-one,x,y-len);
//
// Initial box size of 1
//
		m_recCurFrame.top = y;
		m_recCurFrame.left = x;
		m_recCurFrame.width = one;
		m_recCurFrame.height = one;
		m_bBoxOnOff = false;
		m_iLastX = x;
		m_iLastY = y;
		_xorBox(m_recCurFrame);
		m_iDraggingWhat = FV_FrameEdit_DragBotRightCorner;
		getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_SE);
	}
}


void FV_FrameEdit::mouseRelease(UT_sint32 x, UT_sint32 y)
{
	_xorBox(m_recCurFrame);

//
// Finish up by clearing the editmode and dragging what modes
//	
	m_iFrameEditMode = FV_FrameEdit_NOT_ACTIVE;
	m_iDraggingWhat =  FV_FrameEdit_DragNothing;
	m_pView->setCursorToContext();
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
	m_bBoxOnOff = !m_bBoxOnOff;
}

