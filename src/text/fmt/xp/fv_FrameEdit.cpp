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
#include "fp_FrameContainer.h"

FV_FrameEdit::FV_FrameEdit (FV_View * pView)
	: m_pView (pView), 
	  m_iFrameEditMode(FV_FrameEdit_NOT_ACTIVE),
	  m_pFrameLayout(NULL),
	  m_pFrameContainer(NULL),
	  m_iDraggingWhat( FV_FrameEdit_DragNothing),
	  m_iLastX(0),
	  m_iLastY(0),
	  m_recCurFrame(0,0,0,0),
	  m_bBoxOnOff(false),
	  m_iInitialDragX(0),
	  m_iInitialDragY(0)
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

	if(FV_FrameEdit_WAIT_FOR_FIRST_CLICK_INSERT == m_iFrameEditMode)
	{
		xxx_UT_DEBUGMSG(("width after drag %d \n",m_recCurFrame.width));
		_xorBox(m_recCurFrame);
	}
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
	UT_sint32 dx = 0;
	UT_sint32 dy = 0;
	UT_Rect expX(0,m_recCurFrame.top,0,m_recCurFrame.height);
	UT_Rect expY(m_recCurFrame.left,0,m_recCurFrame.width,0);
	switch (m_iDraggingWhat)
	{
	case FV_FrameEdit_DragTopLeftCorner:
		diffx = m_recCurFrame.left - x;
		diffy = m_recCurFrame.top - y;
		m_recCurFrame.left -= diffx;
		m_recCurFrame.top -= diffy;
		dx = -diffx;
		dy = -diffy;
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
		dy = -diffy;
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
		dx = -diffx;
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
		dx = -diffx;
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
		dy = -diffy;
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
	{
		diffx = m_iLastX - x;
		diffy = m_iLastY - y;
		dx = -diffx;
		dy = - diffy;
		m_recCurFrame.left -= diffx;
		m_recCurFrame.top -= diffy;
		UT_sint32 iext = getGraphics()->tlu(2);
		if(dx < 0)
		{
			expX.left = m_recCurFrame.left+m_recCurFrame.width -iext;
			expX.width = -dx + 2*iext;
			if(dy > 0)
			{
				expX.top -=  iext;
				expX.height += dy + 2*iext;
			}
			else
			{
				expX.top -=  iext;
				expX.height += (-dy + 2*iext);
			}
		}
		else
		{
			expX.left = m_recCurFrame.left - dx -iext;
			expX.width = dx + 2*iext;
			if(dy > 0)
			{
				expX.top -=  iext;
				expX.height += dy + 2*iext;
			}
			else
			{
				expX.top -= iext;
				expX.height += (-dy + 2*iext);
			}
		}
		if(dy < 0)
		{
			expY.top = m_recCurFrame.top + m_recCurFrame.height + iext;
			expY.height = -dy + 2*iext;
		}
		else
		{
			expY.top = m_recCurFrame.top - dy -iext;
			expY.height = dy + 2*iext;
		}
	}
	break;
	default:
		break;
	}

// Should draw the new box
	if(FV_FrameEdit_WAIT_FOR_FIRST_CLICK_INSERT == m_iFrameEditMode)
	{
		xxx_UT_DEBUGMSG(("width after drag %d \n",m_recCurFrame.width));
		_xorBox(m_recCurFrame);
	}
	else if (FV_FrameEdit_RESIZE_EXISTING == m_iFrameEditMode)
	{
		UT_sint32 iW = m_recCurFrame.width;
		UT_sint32 iH = m_recCurFrame.height;
		m_pFrameContainer->clearScreen();
		m_pFrameLayout->localCollapse();
		m_pFrameLayout->setFrameWidth(iW);
		m_pFrameLayout->setFrameHeight(iH);
		m_pFrameContainer->_setWidth(iW);
		m_pFrameContainer->_setHeight(iH);
		m_pFrameLayout->miniFormat();
		UT_sint32 newX = m_pFrameContainer->getFullX();
		UT_sint32 newY = m_pFrameContainer->getFullY();
		newX += dx;
		newY += dy;
		m_pFrameContainer->_setX(newX);
		m_pFrameContainer->_setY(newY);
		drawFrame(true);
	}
	else if (FV_FrameEdit_DRAG_EXISTING == m_iFrameEditMode)
	{
		UT_sint32 newX = m_pFrameContainer->getFullX();
		UT_sint32 newY = m_pFrameContainer->getFullY();
		newX += dx;
		newY += dy;
		m_pFrameContainer->clearScreen();
		m_pFrameContainer->_setX(newX);
		m_pFrameContainer->_setY(newY);
		drawFrame(true);
		if(expX.width > 0)
		{
			getGraphics()->setClipRect(&expX);
			m_pView->updateScreen(false);
		}
		if(expY.height > 0)
		{
			getGraphics()->setClipRect(&expY);
			m_pView->updateScreen(false);
		}
		getGraphics()->setClipRect(NULL);
	}
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
void FV_FrameEdit::setDragType(UT_sint32 x, UT_sint32 y, bool bDrawFrame)
{
	xxx_UT_DEBUGMSG(("setDragType called frameEdit mode %d \n",m_iFrameEditMode));
	PT_DocPosition posAtXY = m_pView->getDocPositionFromXY(x,y,false);
	fl_BlockLayout * pBL = m_pView->_findBlockAtPosition(posAtXY);
	if(!isActive())
	{
		m_iFrameEditMode = 	FV_FrameEdit_EXISTING_SELECTED;
		fl_ContainerLayout * pCL = pBL->myContainingLayout();
		while(pCL && (pCL->getContainerType() != FL_CONTAINER_FRAME) && (pCL->getContainerType() != FL_CONTAINER_DOCSECTION))
		{
			pCL = pCL->myContainingLayout();
		}
		UT_ASSERT(pCL);
		if(pCL == NULL)
		{
			return;
		}
		if(pCL->getContainerType() != FL_CONTAINER_FRAME)
		{
			return;
		}
		m_pFrameLayout = static_cast<fl_FrameLayout *>(pCL);
		UT_ASSERT(m_pFrameLayout->getContainerType() == FL_CONTAINER_FRAME);
		m_pFrameContainer = static_cast<fp_FrameContainer *>(m_pFrameLayout->getFirstContainer());
		UT_ASSERT(m_pFrameContainer);
		if(bDrawFrame)
		{
			drawFrame(true);
		}
		m_iLastX = x;
		m_iLastY = y;
		m_iDraggingWhat = FV_FrameEdit_DragWholeFrame;
		return;	
	}
	//
	// OK find the coordinates of the frame.
	//
	UT_sint32 xPage,yPage;
	UT_sint32 xClick, yClick;
	fp_Page* pPage = m_pView->_getPageForXY(x, y, xClick, yClick);
	m_pView->getPageScreenOffsets(pPage,xPage,yPage);
	fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pBL->myContainingLayout());
	fp_FrameContainer * pFCon = static_cast<fp_FrameContainer *>(pFL->getFirstContainer());
	UT_sint32 ires = getGraphics()->tlu(FRAME_HANDLE_SIZE); // 6 pixels wide hit area
	UT_sint32 iLeft = xPage + pFCon->getFullX();
	UT_sint32 iRight = xPage + pFCon->getFullX() + pFCon->getFullWidth();
	UT_sint32 iTop = yPage + pFCon->getFullY();
	UT_sint32 iBot = yPage + pFCon->getFullY() + pFCon->getFullHeight();
	bool bX = (iLeft - ires < x) && (x < iRight + ires);
	bool bY = (iTop - ires < y) && (iBot + ires > y);
	bool bLeft = (iLeft - ires < x) && (x  < iLeft + ires);
	bool bRight = (iRight - ires < x) && (x < iRight + ires);
	bool bTop = (iTop - ires < y) && (y < iTop + ires);
	bool bBot = (iBot - ires < y) && (y < iBot + ires);
//
// top left
//
	if((iLeft < x) && (x < iLeft + ires) && (iTop < y) && (y < iTop + ires))
	{
		m_iDraggingWhat = FV_FrameEdit_DragTopLeftCorner;
	}
//
// top Right
//
	else if((iRight - ires < x) && (x < iRight) && (iTop < y) && (y < iTop + ires))
	{
		m_iDraggingWhat = FV_FrameEdit_DragTopRightCorner;
	}
//
// bot left
//
	else if((iLeft < x) && (x < iLeft + ires) && (iBot > y) && (y > iBot - ires))
	{
		m_iDraggingWhat = FV_FrameEdit_DragBotLeftCorner;
	}
//
// Bot Right
//
	else if((iRight - ires < x) && (x < iRight) && (iBot > y) && (y > iBot - ires))
	{
		m_iDraggingWhat = FV_FrameEdit_DragBotRightCorner;
	}
//
// top Edge
//
	else if( bX && bTop)
	{
		m_iDraggingWhat = FV_FrameEdit_DragTopEdge;
	}
//
// left Edge
//
	else if(bLeft && bY)
	{
		m_iDraggingWhat = FV_FrameEdit_DragLeftEdge;
	}
//
// right Edge
//
	else if(bRight && bY)
	{
		m_iDraggingWhat = FV_FrameEdit_DragRightEdge;
	}
//
// bot Edge
//
	else if(bBot && bX)
	{
		m_iDraggingWhat = FV_FrameEdit_DragBotEdge;
	}
	else
	{
		if( bX && bY)
		{
			m_iDraggingWhat = FV_FrameEdit_DragWholeFrame;
			xxx_UT_DEBUGMSG(("Dragging Whole Frame \n"));
		}
		else
		{
			m_iDraggingWhat = FV_FrameEdit_DragNothing;
			return;
		}
	}
	if(bDrawFrame)
	{
		drawFrame(true);
	}
	m_recCurFrame.left = iLeft;
	m_recCurFrame.top = iTop;
	m_recCurFrame.width = (iRight - iLeft);
	m_recCurFrame.height = (iBot - iTop);
	m_iLastX = x;
	m_iLastY = y;
	m_iInitialDragX = iLeft;
	m_iInitialDragY = iTop;
	xxx_UT_DEBUGMSG(("Initial width %d \n",m_recCurFrame.width));
	xxx_UT_DEBUGMSG((" Dragging What %d \n",m_iDraggingWhat));
	m_pView->setCursorToContext();
}

/*!
 * This method responds to a mouse click and decides what to do with it.
 */
void FV_FrameEdit::mouseLeftPress(UT_sint32 x, UT_sint32 y)
{
	if(!isActive())
	{
		setDragType(x,y,true);
		return;
	}
//
// Find the type of drag we should do.
//
	if(FV_FrameEdit_EXISTING_SELECTED == m_iFrameEditMode )
	{
		setDragType(x,y,true);
		if(FV_FrameEdit_DragNothing == m_iDraggingWhat)
		{
			m_iFrameEditMode = FV_FrameEdit_NOT_ACTIVE;
			drawFrame(false);
			m_pFrameLayout = NULL;
			m_pFrameContainer = NULL;
			m_pView->setCursorToContext();
		}
		else
		{
			//_drawBox(m_recCurFrame);
			if( m_iDraggingWhat != FV_FrameEdit_DragWholeFrame)
			{
				m_iFrameEditMode = FV_FrameEdit_RESIZE_EXISTING;
			}
			else
			{
				m_iFrameEditMode = FV_FrameEdit_DRAG_EXISTING;
				m_iInitialDragX = m_recCurFrame.left;
				m_iInitialDragY = m_recCurFrame.top;
			}
		}
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
		_drawBox(m_recCurFrame);
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
		xxx_UT_DEBUGMSG((" frameEdit y1= %d y2= %d \n",y1,y2));
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
		xxx_UT_DEBUGMSG(("Raw yLineoff %d \n",yLineOff));
		xLineOff = x + pRun->getX() - xLineOff  + xBlockOff;
		yLineOff = y + pRun->getY() - yLineOff  + yBlockOff;
		xxx_UT_DEBUGMSG(("fv_FrameEdit: (x,y) %d %d xLineOff %d yLineOff %d \n",x,y,xLineOff,yLineOff));
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
// If we've just selected the frame, ignore this event.
//
	if(FV_FrameEdit_EXISTING_SELECTED == m_iFrameEditMode)
	{
		UT_DEBUGMSG(("Existing Frame selected now released button \n"));
		return;
	}
// First update the drag box
//
	mouseDrag(x,y);
//
// Clear the drag box
//
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

// Do Resize and Drag of frame

	else if((m_iFrameEditMode == FV_FrameEdit_RESIZE_EXISTING) ||
			(m_iFrameEditMode == FV_FrameEdit_DRAG_EXISTING))
	{
		const PP_AttrProp* pSectionAP = NULL;
		m_pFrameLayout->getAttrProp(&pSectionAP);
		const char * pszXpos = NULL;
		const char * pszYpos = NULL;
		UT_sint32 iX,iY;

// Xpos
		if(!pSectionAP || !pSectionAP->getProperty("xpos",pszXpos))
		{
			UT_DEBUGMSG(("No xpos defined for Frame in FrameEdit !\n"));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return;
		}
		else
		{
			iX = UT_convertToLogicalUnits(pszXpos);
		}
// Ypos
		if(!pSectionAP || !pSectionAP->getProperty("ypos",pszYpos))
		{
			UT_DEBUGMSG(("No ypos defined for Frame in FrameEdit !\n"));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return;
		}
		else
		{
			iY = UT_convertToLogicalUnits(pszYpos);
		}
		UT_sint32 xdiff = m_recCurFrame.left - m_iInitialDragX;
		UT_sint32 ydiff = m_recCurFrame.top - m_iInitialDragY;
		iX += xdiff;
		iY += ydiff;


		// Signal PieceTable Change
		m_pView->_saveAndNotifyPieceTableChange();

	// Turn off list updates

		getDoc()->disableListUpdates();
		getDoc()->beginUserAtomicGlob();

		UT_String sXpos("");
		UT_String sYpos("");
		UT_String sWidth("");
		UT_String sHeight("");


		double dX = static_cast<double>(iX)/static_cast<double>(UT_LAYOUT_RESOLUTION);
		double dY = static_cast<double>(iY)/static_cast<double>(UT_LAYOUT_RESOLUTION);
		sXpos = UT_formatDimensionedValue(dX,"in", NULL);
		sYpos = UT_formatDimensionedValue(dY,"in", NULL);

		double dWidth = static_cast<double>(m_recCurFrame.width)/static_cast<double>(UT_LAYOUT_RESOLUTION);
		double dHeight = static_cast<double>(m_recCurFrame.height)/static_cast<double>(UT_LAYOUT_RESOLUTION);
		sWidth = UT_formatDimensionedValue(dWidth,"in", NULL);
		sHeight = UT_formatDimensionedValue(dHeight,"in", NULL);

		const XML_Char * props[10] = { "width",sWidth.c_str(),
									  "height",sHeight.c_str(),
									  "xpos",sXpos.c_str(),
									  "ypos",sYpos.c_str(),
									  NULL,NULL};
		PT_DocPosition pos = m_pFrameLayout->getPosition()+1;
		getDoc()->changeStruxFmt(PTC_AddFmt,pos,pos,NULL,props,PTX_SectionFrame);

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
	m_pFrameLayout = NULL;
	m_pFrameContainer = NULL;
	m_pView->setCursorToContext();
}

FV_FrameEditDragWhat FV_FrameEdit::mouseMotion(UT_sint32 x, UT_sint32 y)
{
	return getFrameEditDragWhat();
}

void FV_FrameEdit::drawFrame(bool bWithHandles)
{
	if(m_pFrameContainer == NULL)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	fp_Page * pPage = m_pFrameContainer->getPage();
	dg_DrawArgs da;
	da.pG = getGraphics();
	da.bDirtyRunsOnly = false;
	UT_sint32 xPage,yPage;
	m_pView->getPageScreenOffsets(pPage,xPage,yPage);
	da.xoff = xPage + m_pFrameContainer->getX();
	da.yoff = yPage + m_pFrameContainer->getY();
	m_pFrameContainer->clearScreen();
	m_pFrameContainer->draw(&da);
	if(bWithHandles)
	{
		m_pFrameContainer->drawHandles(&da);
	}
}

void FV_FrameEdit::_xorBox(UT_Rect & rec)
{
	xorRect(getGraphics(),rec);
	m_bBoxOnOff = !m_bBoxOnOff;
}


void FV_FrameEdit::_drawBox(UT_Rect & rec)
{
	UT_sint32 iLeft = rec.left;
	UT_sint32 iRight = iLeft + rec.width;
	UT_sint32 iTop = rec.top;
	UT_sint32 iBot = iTop + rec.height;
	getGraphics()->setLineWidth(getGraphics()->tlu(1));
	UT_RGBColor black(0,0,0);
	getGraphics()->setColor(black);
	
	GR_Graphics::JoinStyle js = GR_Graphics::JOIN_MITER;
	GR_Graphics::CapStyle  cs = GR_Graphics::CAP_BUTT;
	getGraphics()->setLineProperties (1, js, cs, GR_Graphics::LINE_SOLID);

    getGraphics()->drawLine(iLeft,iTop,iRight,iTop);
    getGraphics()->drawLine(iRight,iTop,iRight,iBot);
    getGraphics()->drawLine(iLeft,iBot,iRight,iBot);
    getGraphics()->drawLine(iLeft,iTop,iLeft,iBot);
	m_bBoxOnOff = true;
}

