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
#include "ut_units.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"

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
// We're waiting for the first click to interactively enter the initial
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

bool FV_FrameEdit::getFrameStrings(UT_sint32 x, UT_sint32 y, 
										UT_String & sXpos,
										UT_String & sYpos,
										UT_String & sWidth,
										UT_String & sHeight,
								   PT_DocPosition & posAtXY)
{
//
// Find the block that contains (x,y). We'll insert the frame after
// this block in PT and position it on the page relative to this block.
//
		posAtXY = m_pView->getDocPositionFromXY(x,y,true);
		fl_BlockLayout * pBL = NULL;
		fp_Run * pRun = NULL;
		fp_Line * pLine = NULL;
		UT_sint32 x1,x2,y1,y2;
		UT_uint32 height;
		bool b,bDir;
		m_pView->_findPositionCoords(posAtXY,b,x1,y1,x2,y2,height,bDir,&pBL,&pRun);
		UT_DEBUGMSG((" frameEdit y1= %d y2= %d \n",y1,y2));
		if((pBL == NULL) || (pRun == NULL))
		{
			return false;
		}
		pLine = pRun->getLine();
		if(pLine == NULL)
		{
			return false;
		}
//
// Find the screen coords of this line, then work out the offset to the (x,y)
// point. After that workout the offset to the first line of the block.
//
//
		UT_sint32 xBlockOff =0;
		UT_sint32 yBlockOff = 0;
		bool bValid = false;
		bValid = pBL->getXYOffsetToLine(xBlockOff,yBlockOff,pLine);
		UT_sint32 xLineOff = 0;
		UT_sint32 yLineOff = 0;
		pLine->getScreenOffsets(pRun, xLineOff,yLineOff);
		UT_DEBUGMSG(("Raw yLineoff %d \n",yLineOff));
		xLineOff = x + pRun->getX() - xLineOff  + xBlockOff;
		yLineOff = y + pRun->getY() - yLineOff  + yBlockOff;
		UT_DEBUGMSG(("fv_FrameEdit: (x,y) %d %d xLineOff %d yLineOff %d \n",x,y,xLineOff,yLineOff));
//
// The sXpos and sYpos values are the numbers that need to be added from the
// top left corner of thefirst line of the block to reach the top left 
// corner of the frame. We now have these in layout units. Convert to inches
// now
//
		double xPos = static_cast<double>(xLineOff)/static_cast<double>(UT_LAYOUT_RESOLUTION);
		double yPos = static_cast<double>(yLineOff)/static_cast<double>(UT_LAYOUT_RESOLUTION);
		sXpos = UT_formatDimensionedValue(xPos,"in", NULL);
		sYpos = UT_formatDimensionedValue(yPos,"in", NULL);
		double dWidth = static_cast<double>(m_recCurFrame.width)/static_cast<double>(UT_LAYOUT_RESOLUTION);
		double dHeight = static_cast<double>(m_recCurFrame.height)/static_cast<double>(UT_LAYOUT_RESOLUTION);
		sWidth = UT_formatDimensionedValue(dWidth,"in", NULL);
		sHeight = UT_formatDimensionedValue(dHeight,"in", NULL);
		return true;
}

void FV_FrameEdit::mouseRelease(UT_sint32 x, UT_sint32 y)
{
//
// First update the drag box
//
	mouseDrag(x,y);
	_xorBox(m_recCurFrame);
	PT_DocPosition posAtXY = 0;

	if(m_iFrameEditMode == 	FV_FrameEdit_RESIZE_INSERT)
	{
		// Signal PieceTable Change
		m_pView->_saveAndNotifyPieceTableChange();

	// Turn off list updates

		getDoc()->disableListUpdates();
		getDoc()->beginUserAtomicGlob();

		UT_String sXpos("");
		UT_String sYpos("");
		UT_String sWidth("");
		UT_String sHeight("");
		getFrameStrings(m_recCurFrame.left,m_recCurFrame.top,sXpos,sYpos,sWidth,sHeight,posAtXY);
		pf_Frag_Strux * pfFrame = NULL;
		const XML_Char * props[14] = {"frame-type","textbox",
									 "position-to","block-above-text",
									 "xpos",sXpos.c_str(),
									 "ypos",sYpos.c_str(),
									 "width",sWidth.c_str(),
									  "height",sHeight.c_str(),
									  NULL,NULL};
//
// This should place the the frame strux immediately after the block containing
// position posXY.
// It returns the Frag_Strux of the new frame.
//
		getDoc()->insertStrux(posAtXY,PTX_SectionFrame,NULL,props,&pfFrame);
		PT_DocPosition posFrame = pfFrame->getPos();
		getDoc()->insertStrux(posFrame+1,PTX_Block);
		getDoc()->insertStrux(posFrame+2,PTX_EndFrame);

// Place the insertion point in the Frame

		m_pView->setPoint(posFrame+1);

// Finish up with the usual stuff
		getDoc()->endUserAtomicGlob();
		getDoc()->setDontImmediatelyLayout(false);
		m_pView->_generalUpdate();


	// restore updates and clean up dirty lists
		getDoc()->enableListUpdates();
		getDoc()->updateDirtyLists();

	// Signal PieceTable Changes have finished
		m_pView->_restorePieceTableState();
		m_pView->notifyListeners(AV_CHG_MOTION);
		m_pView->_fixInsertionPointCoords();
		m_pView->_ensureInsertionPointOnScreen();
	}
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

