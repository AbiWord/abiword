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
#include "gr_Painter.h"
#include "xap_App.h"

FV_FrameEdit::FV_FrameEdit (FV_View * pView)
	: m_pView (pView), 
	  m_iFrameEditMode(FV_FrameEdit_NOT_ACTIVE),
	  m_pFrameLayout(NULL),
	  m_pFrameContainer(NULL),
	  m_iDraggingWhat( FV_FrameEdit_DragNothing),
	  m_iLastX(0),
	  m_iLastY(0),
	  m_recCurFrame(0,0,0,0),
	  m_iInitialDragX(0),
	  m_iInitialDragY(0),
	  m_bFirstDragDone(false),
	  m_bInitialClick(false),
	  m_pFrameImage(NULL)
{
	UT_ASSERT (pView);
}

FV_FrameEdit::~FV_FrameEdit()
{
	DELETEP(m_pFrameImage);
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
	if(iEditMode == FV_FrameEdit_NOT_ACTIVE)
	{
		m_pFrameLayout = NULL;
		m_pFrameContainer = NULL;
		DELETEP(m_pFrameImage);
		m_recCurFrame.width = 0;
		m_recCurFrame.height = 0;
	    m_iDraggingWhat = FV_FrameEdit_DragNothing;
		m_iLastX = 0;
		m_iLastY = 0;
	}
	m_iFrameEditMode = iEditMode;
}

void FV_FrameEdit::mouseDrag(UT_sint32 x, UT_sint32 y)
{
	m_bFirstDragDone = true;
	UT_sint32 diffx = 0;
	UT_sint32 diffy = 0;
	UT_sint32 dx = 0;
	UT_sint32 dy = 0;
	UT_Rect expX(0,m_recCurFrame.top,0,m_recCurFrame.height);
	UT_Rect expY(m_recCurFrame.left,0,m_recCurFrame.width,0);
	UT_sint32 iext = getGraphics()->tlu(3);
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
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_W);
			m_iDraggingWhat =  FV_FrameEdit_DragRightEdge;
		}
		break;
	case FV_FrameEdit_DragRightEdge:
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
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_E);
			m_iDraggingWhat =  FV_FrameEdit_DragLeftEdge;
		}
		break;
	case FV_FrameEdit_DragTopEdge:
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
			getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_S);
			m_iDraggingWhat =  FV_FrameEdit_DragBotEdge;
		}
		break;
	case FV_FrameEdit_DragBotEdge:
		diffy = m_recCurFrame.top + m_recCurFrame.height - y;
		m_recCurFrame.height -= diffy;
		if(diffy > 0)
		{
			expY.top = m_recCurFrame.top + m_recCurFrame.height;
			expY.height = diffy + iext;
			expY.left -= iext;
			expY.width += 2*iext;
		}
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
			expX.left = m_recCurFrame.left - dx - iext;
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
		expY.left -= iext;
		expY.width += 2*iext;
		if(dy < 0)
		{
			expY.top = m_recCurFrame.top + m_recCurFrame.height -iext;
			expY.height = -dy + 2*iext;
		}
		else
		{
			expY.top = m_recCurFrame.top - dy - iext;
			expY.height = dy + 2*iext;
		}
	}
	break;
	default:
		break;
	}


	if(FV_FrameEdit_RESIZE_INSERT == m_iFrameEditMode)
	{
		xxx_UT_DEBUGMSG(("width after drag %d \n",m_recCurFrame.width));
	}
	else if (FV_FrameEdit_RESIZE_EXISTING == m_iFrameEditMode)
	{
		UT_sint32 iW = m_recCurFrame.width;
		UT_sint32 iH = m_recCurFrame.height;
		UT_sint32 newX = m_pFrameContainer->getFullX();
		UT_sint32 newY = m_pFrameContainer->getFullY();
		m_pFrameLayout->localCollapse();
		m_pFrameLayout->setFrameWidth(iW);
		m_pFrameLayout->setFrameHeight(iH);
		m_pFrameContainer->_setWidth(iW);
		m_pFrameContainer->_setHeight(iH);
		m_pFrameLayout->miniFormat();
		m_pFrameLayout->getDocSectionLayout()->setNeedsSectionBreak(false,NULL);
		newX += dx;
		newY += dy;
		m_pFrameContainer->_setX(newX);
		m_pFrameContainer->_setY(newY);
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

		drawFrame(true);
	}
	else if (FV_FrameEdit_DRAG_EXISTING == m_iFrameEditMode)
	{
		UT_sint32 newX = m_pFrameContainer->getFullX();
		UT_sint32 newY = m_pFrameContainer->getFullY();
		newX += dx;
		newY += dy;
		m_pFrameContainer->_setX(newX);
		m_pFrameContainer->_setY(newY);
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
		drawFrame(true);
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
	fp_FrameContainer * pFCon = NULL;
	fl_FrameLayout * pFL = NULL;
	UT_sint32 xPage,yPage;
	UT_sint32 xClick, yClick;
	fp_Page* pPage = m_pView->_getPageForXY(x, y, xClick, yClick);
	m_pView->getPageScreenOffsets(pPage,xPage,yPage);
	if(m_iFrameEditMode == FV_FrameEdit_EXISTING_SELECTED)
	{
		pFCon = m_pFrameContainer;
		pFL = m_pFrameLayout;
	}
	else
	{
		pFL = static_cast<fl_FrameLayout *>(pBL->myContainingLayout());
		pFCon = static_cast<fp_FrameContainer *>(pFL->getFirstContainer());
	}
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
	if(bDrawFrame && (m_recCurFrame.width > 0) && (m_recCurFrame.height >0))
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
			DELETEP(m_pFrameImage);
			m_pView->setCursorToContext();
		}
		else
		{
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
//
// Now insert a text box
//
		UT_sint32 origX = x;
		UT_sint32 origY = y;
		UT_sint32 iSize = getGraphics()->tlu(32);
		x = x - iSize + getGraphics()->tlu(4) ;
		y = y - iSize + getGraphics()->tlu(4);
		m_recCurFrame.left = x;
		m_recCurFrame.top = y;
		m_recCurFrame.width = iSize;
		m_recCurFrame.height = iSize;
		m_iFrameEditMode = FV_FrameEdit_RESIZE_INSERT;
		getDoc()->beginUserAtomicGlob();
		mouseRelease(x,y);
		m_iFrameEditMode = FV_FrameEdit_RESIZE_EXISTING;
		m_iLastX = origX;
		m_iLastY = origY;
		m_iInitialDragX = x;
		m_iInitialDragY = y;
		m_iDraggingWhat = FV_FrameEdit_DragBotRightCorner;
		m_bFirstDragDone = false;
		m_bInitialClick = true;
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
		bool bEOL=false;
		bool bDir;
		m_pView->_findPositionCoords(posAtXY,bEOL,x1,y1,x2,y2,height,bDir,&pBL,&pRun);
		UT_DEBUGMSG((" Requested y %d frameEdit y1= %d y2= %d \n",y,y1,y2));
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
		fp_Line * pFirstL = static_cast<fp_Line *>(pBL->getFirstContainer());
		UT_sint32 xFirst,yFirst;
		pFirstL->getScreenOffsets(pFirstL->getFirstRun(),xFirst,yFirst);
		UT_DEBUGMSG(("First line x  %d y %d \n",xFirst,yFirst));
		UT_DEBUGMSG(("xBlockOffset %d yBlockOffset %d \n",xBlockOff,yBlockOff));
		UT_sint32 xLineOff = 0;
		UT_sint32 yLineOff = 0;
		fp_VerticalContainer * pVCon = static_cast<fp_VerticalContainer *>(pLine->getContainer());
		pVCon->getOffsets(pLine,xLineOff,yLineOff);
		UT_DEBUGMSG(("Closest Line yLineoff %d \n",yLineOff));

// OK correct for page offsets
		fp_Page * pPage = pVCon->getPage();
		if(pPage == NULL)
		{
			return false;
		}
		UT_sint32 xp,yp;
		m_pView->getPageScreenOffsets(pPage,xp,yp);
		xLineOff = x -xp - xLineOff;
//		yLineOff = y + pRun->getY() - yLineOff  + yBlockOff;
		yLineOff = y - yp - yLineOff + yBlockOff;
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
// If we've just selected the frame, ignore this event.
//
	if(FV_FrameEdit_EXISTING_SELECTED == m_iFrameEditMode)
	{
		UT_DEBUGMSG(("Existing Frame selected now released button \n"));
		return;
	}

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
									 "frame-width",sWidth.c_str(),
									  "frame-height",sHeight.c_str(),
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

		m_pView->setPoint(posFrame+2);

// Finish up with the usual stuff
		getDoc()->endUserAtomicGlob();
		getDoc()->setDontImmediatelyLayout(false);
		m_pView->_generalUpdate();


	// restore updates and clean up dirty lists
		getDoc()->enableListUpdates();
		getDoc()->updateDirtyLists();

	// Signal PieceTable Changes have finished
		m_pView->_restorePieceTableState();
		m_pView->notifyListeners(AV_CHG_HDRFTR);
		m_pView->_fixInsertionPointCoords();
		m_pView->_ensureInsertionPointOnScreen();

//
		m_iFrameEditMode = 	FV_FrameEdit_EXISTING_SELECTED;
		fl_BlockLayout * pBL = m_pView->_findBlockAtPosition(posFrame+2);
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
		drawFrame(true);
		return;
	}

// Do Resize and Drag of frame

	else if((m_iFrameEditMode == FV_FrameEdit_RESIZE_EXISTING) ||
			(m_iFrameEditMode == FV_FrameEdit_DRAG_EXISTING))
	{
		const PP_AttrProp* pSectionAP = NULL;
		m_pFrameLayout->getAP(pSectionAP);

//
// If there was no drag, the user just clicked and released the left mouse
// no need to change anything.
//
		if(!m_bFirstDragDone)
		{
			getDoc()->endUserAtomicGlob();
			return;
		}
//
// OK get the properties of the current frame, update them with the new
// the position and size of this drag.
//

//  Frame Image

		const XML_Char * pszDataID = NULL;
		pSectionAP->getAttribute(PT_STRUX_IMAGE_DATAID, (const XML_Char *&)pszDataID);

		UT_String sFrameProps;
		UT_String sProp;
		UT_String sVal;

		const XML_Char *pszFrameType = NULL;
		const XML_Char *pszPositionTo = NULL;
		const XML_Char *pszXpad = NULL;
		const XML_Char *pszYpad = NULL;

		const XML_Char * pszColor = NULL;
		const XML_Char * pszBorderColor = NULL;
		const XML_Char * pszBorderStyle = NULL;
		const XML_Char * pszBorderWidth = NULL;


// Frame Type

		sProp = "frame-type";
		if(!pSectionAP || !pSectionAP->getProperty("frame-type",pszFrameType))
		{
			sVal = "textbox";
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}
		else 
		{
			sVal = pszFrameType;
		}
		UT_String_setProperty(sFrameProps,sProp,sVal);		
		

// Position-to

		sProp = "position-to";
		if(!pSectionAP || !pSectionAP->getProperty("position-to",pszPositionTo))
		{
			sVal = "block-above-text";
		}
		else
		{
			sVal = pszPositionTo;
		}
		UT_String_setProperty(sFrameProps,sProp,sVal);		

// Xpadding

		sProp = "xpad";
		if(!pSectionAP || !pSectionAP->getProperty("xpad",pszXpad))
		{
			sVal = "0.03in";
		}
		else
		{
			sVal= pszXpad;
		}
		UT_String_setProperty(sFrameProps,sProp,sVal);		

// Ypadding

		sProp = "ypad";
		if(!pSectionAP || !pSectionAP->getProperty("ypad",pszYpad))
		{
			sVal = "0.03in";
		}
		else
		{
			sVal = pszYpad;
		}
		UT_String_setProperty(sFrameProps,sProp,sVal);		


	/* Frame-border properties:
	 */

		pSectionAP->getProperty ("color", pszColor);
		if(pszColor)
		{
			sProp = "color";
			sVal = pszColor;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}

		pSectionAP->getProperty ("bot-color",pszBorderColor);
		pSectionAP->getProperty ("bot-style",pszBorderStyle);
		pSectionAP->getProperty ("bot-thickness",pszBorderWidth);
		if(pszBorderColor)
		{
			sProp = "bot-color";
			sVal = pszBorderColor;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}
		if(pszBorderStyle)
		{
			sProp = "bot-style";
			sVal = pszBorderStyle;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}
		if(pszBorderWidth)
		{
			sProp = "bot-thickness";
			sVal = pszBorderWidth;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}

		pszBorderColor = NULL;
		pszBorderStyle = NULL;
		pszBorderWidth = NULL;

		pSectionAP->getProperty ("left-color", pszBorderColor);
		pSectionAP->getProperty ("left-style", pszBorderStyle);
		pSectionAP->getProperty ("left-thickness", pszBorderWidth);
		if(pszBorderColor)
		{
			sProp = "left-color";
			sVal = pszBorderColor;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}
		if(pszBorderStyle)
		{
			sProp = "left-style";
			sVal = pszBorderStyle;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}
		if(pszBorderWidth)
		{
			sProp = "left-thickness";
			sVal = pszBorderWidth;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}

		pszBorderColor = NULL;
		pszBorderStyle = NULL;
		pszBorderWidth = NULL;

		pSectionAP->getProperty ("right-color",pszBorderColor);
		pSectionAP->getProperty ("right-style",pszBorderStyle);
		pSectionAP->getProperty ("right-thickness", pszBorderWidth);
		if(pszBorderColor)
		{
			sProp = "right-color";
			sVal = pszBorderColor;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}
		if(pszBorderStyle)
		{
			sProp = "right-style";
			sVal = pszBorderStyle;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}
		if(pszBorderWidth)
		{
			sProp = "right-thickness";
			sVal = pszBorderWidth;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}

		pszBorderColor = NULL;
		pszBorderStyle = NULL;
		pszBorderWidth = NULL;

		pSectionAP->getProperty ("top-color",  pszBorderColor);
		pSectionAP->getProperty ("top-style",  pszBorderStyle);
		pSectionAP->getProperty ("top-thickness",pszBorderWidth);
		if(pszBorderColor)
		{
			sProp = "top-color";
			sVal = pszBorderColor;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}
		if(pszBorderStyle)
		{
			sProp = "top-style";
			sVal = pszBorderStyle;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}
		if(pszBorderWidth)
		{
			sProp = "top-thickness";
			sVal = pszBorderWidth;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}

	/* Frame fill
	 */

		const XML_Char * pszBgStyle = NULL;
		const XML_Char * pszBgColor = NULL;
		const XML_Char * pszBackgroundColor = NULL;

		pSectionAP->getProperty ("bg-style",    pszBgStyle);
		pSectionAP->getProperty ("bgcolor",     pszBgColor);
		pSectionAP->getProperty ("background-color", pszBackgroundColor);
		if(pszBgStyle)
		{
			sProp = "bg-style";
			sVal = pszBgStyle;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}
		if(pszBgColor)
		{
			sProp = "bgcolor";
			sVal = pszBgColor;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}
		if(pszBackgroundColor)
		{
			sProp = "background-color";
			sVal = pszBackgroundColor;
			UT_String_setProperty(sFrameProps,sProp,sVal);		
		}

		UT_String sXpos("");
		UT_String sYpos("");
		UT_String sWidth("");
		UT_String sHeight("");
		getFrameStrings(m_recCurFrame.left,m_recCurFrame.top,sXpos,sYpos,sWidth,sHeight,posAtXY);
		
		sProp = "xpos";
		sVal = sXpos;
		UT_String_setProperty(sFrameProps,sProp,sVal);		
		sProp = "ypos";
		sVal = sYpos;
		UT_String_setProperty(sFrameProps,sProp,sVal);		

		sProp = "frame-width";
		sVal = sWidth;
		UT_String_setProperty(sFrameProps,sProp,sVal);		
		sProp = "frame-height";
		sVal = sHeight;
		UT_String_setProperty(sFrameProps,sProp,sVal);		

		PT_DocPosition oldPoint = m_pView->getPoint();
		PT_DocPosition oldFramePoint = m_pFrameLayout->getPosition(true);
		UT_uint32 oldFrameLen = m_pFrameLayout->getLength();

		// Signal PieceTable Change
		m_pView->_saveAndNotifyPieceTableChange();

	// Turn off list updates

		getDoc()->disableListUpdates();
		getDoc()->beginUserAtomicGlob();

		m_pView->_clearSelection();

// Copy the content of the frame to the clipboard

		PT_DocPosition posStart = m_pFrameLayout->getPosition(true);
		PT_DocPosition posEnd = posStart + m_pFrameLayout->getLength();

		PD_DocumentRange dr_oldFrame;
		dr_oldFrame.set(getDoc(),posStart+2,posEnd-1);
		UT_DEBUGMSG(("SEVIOR: Copy to clipboard changing frame \n"));
		getDoc()->getApp()->copyToClipboard(&dr_oldFrame);

// Delete the frame

		posStart = m_pFrameLayout->getPosition(true);
		posEnd = posStart + m_pFrameLayout->getLength();
		UT_uint32 iRealDeleteCount;
		PP_AttrProp * p_AttrProp_Before = NULL;

		getDoc()->deleteSpan(posStart, posEnd, p_AttrProp_Before, iRealDeleteCount,true);

		m_pFrameLayout = NULL;

// Insert the new frame struxes
//
// This should place the the frame strux immediately after the block containing
// position posXY.
// It returns the Frag_Strux of the new frame.
//
		const XML_Char ** atts = NULL;
		if( pszDataID == NULL)
		{
			atts = new const XML_Char * [3];
			atts[0] = "props";
			atts[1] = sFrameProps.c_str();
			atts[2] =  NULL;
		}
		else
		{
			atts = new const XML_Char * [5];
			atts[0] = PT_STRUX_IMAGE_DATAID;
			atts[1] = pszDataID;
			atts[2] = "props";
			atts[3] = sFrameProps.c_str();
			atts[4] =  NULL;
		}
		pf_Frag_Strux * pfFrame = NULL;
		getDoc()->insertStrux(posAtXY,PTX_SectionFrame,atts,NULL,&pfFrame);
		PT_DocPosition posFrame = pfFrame->getPos();
		getDoc()->insertStrux(posFrame+1,PTX_Block);
		getDoc()->insertStrux(posFrame+2,PTX_EndFrame);
		delete [] atts;

// paste in the contents of the new frame.
//
		PD_DocumentRange dr_dest(getDoc(),posFrame+2,posFrame+2);
		UT_DEBUGMSG(("SEVIOR: Pasting from clipboard Frame changed \n"));
		getDoc()->getApp()->pasteFromClipboard(&dr_dest,true,true);

// Finish up with the usual stuff
		getDoc()->endUserAtomicGlob();
		getDoc()->setDontImmediatelyLayout(false);
		m_pView->_generalUpdate();


	// restore updates and clean up dirty lists
		getDoc()->enableListUpdates();
		getDoc()->updateDirtyLists();

	// Signal PieceTable Changes have finished
		m_pView->_restorePieceTableState();
//
// OK get a pointer to the new frameLayout
//
		m_pFrameLayout = m_pView->getFrameLayout(posFrame+2);
		UT_ASSERT(m_pFrameLayout);

// Set the point back to the same position on the screen

		PT_DocPosition newFramePoint = m_pFrameLayout->getPosition(true);
		UT_sint32 newFrameLen = m_pFrameLayout->getLength();
		PT_DocPosition newPoint = 0;
		if((oldPoint < oldFramePoint) && (newFramePoint < oldPoint))
		{
			newPoint = oldPoint + newFrameLen;
		}
		else if((oldPoint < oldFramePoint) && (oldPoint <= newFramePoint))
		{
			newPoint = oldPoint;
		}
		else if((oldPoint >= oldFramePoint) && (oldPoint >= newFramePoint))
		{
			newPoint = oldPoint  - oldFrameLen + newFrameLen;;
		}
		else // oldPoint >= oldFramePoint && (oldPoint < newFramePoint)
		{
			newPoint = oldPoint - oldFrameLen;
		}
		m_pView->setPoint(newPoint);
		bool bOK = true;
		while(!m_pView->isPointLegal() && bOK)
		{
			bOK = m_pView->_charMotion(true,1);
		}
		m_pView->notifyListeners(AV_CHG_HDRFTR);
		m_pView->_fixInsertionPointCoords();
		m_pView->_ensureInsertionPointOnScreen();
//
// If this was a drag following the initial click, wrap it in a 
// endUserAtomicGlob so it undo's in a single click.
//
		if(m_bFirstDragDone && m_bInitialClick)
		{
			getDoc()->endUserAtomicGlob();
		}
		m_bInitialClick = false;
//
// Finish up by putting the editmode back to existing selected.
//	
		DELETEP(m_pFrameImage);
		m_iFrameEditMode = FV_FrameEdit_EXISTING_SELECTED;
		m_pFrameContainer = static_cast<fp_FrameContainer *>(m_pFrameLayout->getFirstContainer());
		drawFrame(true);
	}
}


/*!
 * This method deletes the current selected frame
 */
void FV_FrameEdit::deleteFrame(void)
{
	if(m_pFrameLayout == NULL)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	PP_AttrProp * p_AttrProp_Before = NULL;

	// Signal PieceTable Change
	m_pView->_saveAndNotifyPieceTableChange();

	// Turn off list updates

	getDoc()->disableListUpdates();
	getDoc()->beginUserAtomicGlob();
	getDoc()->setDontImmediatelyLayout(true);

	PT_DocPosition posStart = m_pFrameLayout->getPosition(true);
	PT_DocPosition posEnd = posStart + m_pFrameLayout->getLength();
	UT_uint32 iRealDeleteCount;

	getDoc()->deleteSpan(posStart, posEnd, p_AttrProp_Before, iRealDeleteCount,true);

	//special handling is required for delete in revisions mode
	//where we have to move the insertion point
	if(m_pView->isMarkRevisions())
	{
		UT_ASSERT( iRealDeleteCount <= posEnd - posStart + 1 );
		m_pView->_charMotion(true,posEnd - posStart - iRealDeleteCount);
	}

// Finish up with the usual stuff
	getDoc()->endUserAtomicGlob();
	getDoc()->setDontImmediatelyLayout(false);
	m_pView->_generalUpdate();


	// restore updates and clean up dirty lists
	getDoc()->enableListUpdates();
	getDoc()->updateDirtyLists();

	// Signal PieceTable Changes have finished
	m_pView->_restorePieceTableState();
	m_pView->notifyListeners(AV_CHG_HDRFTR);
	m_pView->_fixInsertionPointCoords();
	m_pView->_ensureInsertionPointOnScreen();

// Clear all internal variables

	m_pFrameLayout = NULL;
	m_pFrameContainer = NULL;
	DELETEP(m_pFrameImage);
	m_recCurFrame.width = 0;
	m_recCurFrame.height = 0;
	m_iDraggingWhat = FV_FrameEdit_DragNothing;
	m_iLastX = 0;
	m_iLastY = 0;

	m_iFrameEditMode = FV_FrameEdit_NOT_ACTIVE;
}

/*!
 * Method to deal with mouse coordinates.
 */
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
	if((m_pFrameImage == NULL) || (m_iDraggingWhat != FV_FrameEdit_DragWholeFrame) )
	{
		m_pFrameContainer->clearScreen();
		m_pFrameContainer->draw(&da);
		if(bWithHandles)
		{
			m_pFrameContainer->drawHandles(&da);
		}
		if(m_iDraggingWhat == FV_FrameEdit_DragWholeFrame)
		{
			GR_Painter painter (getGraphics());
			m_pFrameImage = painter.genImageFromRectangle(m_recCurFrame);
		}
	}
	else
	{
		GR_Painter painter (getGraphics());
		painter.drawImage(m_pFrameImage,m_recCurFrame.left,m_recCurFrame.top);
	}
}
