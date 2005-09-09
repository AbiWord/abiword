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

#include <math.h>
#include "fv_InlineImage.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_Graphics.h"
#include "ut_units.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fl_TableLayout.h"
#include "fp_TableContainer.h"
#include "fv_View.h"
#include "gr_Painter.h"
#include "xap_Frame.h"
#include "ev_Mouse.h"
#include "ut_locale.h"

#define FRAME_HANDLE_SIZE 6

#define MIN_DRAG_PIXELS 8

FV_VisualInlineImage::FV_VisualInlineImage (FV_View * pView)
	: m_pView (pView), 
	  m_iInlineDragMode(FV_InlineDrag_NOT_ACTIVE),
	  m_pDragImage(NULL),
	  m_iLastX(0),
	  m_iLastY(0),
	  m_recCurFrame(0,0,0,0),
	  m_iInitialOffX(0),
	  m_iInitialOffY(0),
	  m_iFirstEverX(0),
	  m_iFirstEverY(0),
	  m_bTextCut(false),
	  m_pDocUnderCursor(NULL),
	  m_bCursorDrawn(false),
	  m_recCursor(0,0,0,0),
	  m_pAutoScrollTimer(NULL),
	  m_xLastMouse(1),
	  m_yLastMouse(1),
	  m_bDoingCopy(false),
	  m_iDraggingWhat( FV_Inline_DragNothing ),
	  m_iGlobCount(0),
	  m_pImageAP(NULL),
	  m_screenCache(NULL),
	  m_bFirstDragDone(false),
	  m_bIsEmbedded(false)
{
	UT_ASSERT (pView);
}

FV_VisualInlineImage::~FV_VisualInlineImage()
{
	DELETEP(m_pDragImage);
	if(m_pAutoScrollTimer != NULL)
	{
		m_pAutoScrollTimer->stop();
		DELETEP(m_pAutoScrollTimer);
	}
	DELETEP(m_screenCache);
	DELETEP(m_pDocUnderCursor);
}

bool FV_VisualInlineImage::isActive(void) const
{
	return (FV_InlineDrag_NOT_ACTIVE != m_iInlineDragMode);
}

GR_Graphics * FV_VisualInlineImage::getGraphics(void) const
{
	return m_pView->getGraphics();
}

void FV_VisualInlineImage::setMode(FV_InlineDragMode iEditMode)
{
	m_iInlineDragMode = iEditMode;
}

void FV_VisualInlineImage::_autoScroll(UT_Worker * pWorker)
{
	UT_ASSERT(pWorker);

	// this is a static callback method and does not have a 'this' pointer.

	FV_VisualInlineImage * pVis = static_cast<FV_VisualInlineImage *>(pWorker->getInstanceData());
	UT_ASSERT(pVis);
	FV_View * pView = pVis->m_pView;
	pVis->getGraphics()->setClipRect(&pVis->m_recCurFrame);
	pView->updateScreen(false);
	pView->getGraphics()->setClipRect(NULL);
	bool bScrollDown = false;
	bool bScrollUp = false;
	bool bScrollLeft = false;
	bool bScrollRight = false;
	UT_sint32 y = pVis->m_yLastMouse;
	UT_sint32 x = pVis->m_xLastMouse;
	if(y<=0)
	{
		bScrollUp = true;
	}
	else if(y >= pView->getWindowHeight())
	{
		bScrollDown = true;
	}
	if(x <= 0)
	{
		bScrollLeft = true;
	}
	else if(x >= pView->getWindowWidth())
	{
		bScrollRight = true;
	}
	if(bScrollDown || bScrollUp || bScrollLeft || bScrollRight)
	{
		if(bScrollUp)
		{
			pView->cmdScroll(AV_SCROLLCMD_LINEUP, static_cast<UT_uint32>( -y));
		}
		else if(bScrollDown)
		{
			pView->cmdScroll(AV_SCROLLCMD_LINEDOWN, static_cast<UT_uint32>(y - pView->getWindowHeight()));
		}
		if(bScrollLeft)
		{
			pView->cmdScroll(AV_SCROLLCMD_LINELEFT, static_cast<UT_uint32>(-x));
		}
		else if(bScrollRight)
		{
			pView->cmdScroll(AV_SCROLLCMD_LINERIGHT, static_cast<UT_uint32>(x -pView->getWindowWidth()));
		}
		pVis->drawImage();
#if 0
		PT_DocPosition posAtXY = pVis->getPosFromXY(x,y);
		pView->_setPoint(posAtXY);
		pVis->drawCursor(posAtXY);
#endif
		return;
	}
	else
	{
		pVis->m_pAutoScrollTimer->stop();
		DELETEP(pVis->m_pAutoScrollTimer);
	}

}

void FV_VisualInlineImage::mouseDrag(UT_sint32 x, UT_sint32 y)
{
	GR_Graphics * pG = getGraphics();

  if(m_iDraggingWhat == FV_Inline_DragWholeImage)
  {
        if(m_iInlineDragMode  == FV_InlineDrag_NOT_ACTIVE)
        {
	  m_iFirstEverX = x;
	  m_iFirstEverY = y;
	  m_iInlineDragMode  = FV_InlineDrag_WAIT_FOR_MOUSE_DRAG;
	  xxx_UT_DEBUGMSG(("Initial call for drag -1\n"));
	  return;
	}
	if((m_iFirstEverX == 0) && (m_iFirstEverY == 0))
	{
	  m_iFirstEverX = x;
	  m_iFirstEverY = y;
	  m_iInlineDragMode = FV_InlineDrag_WAIT_FOR_MOUSE_DRAG;
	  xxx_UT_DEBUGMSG(("Initial call for drag -2 \n"));
	}
	if(m_iInlineDragMode == FV_InlineDrag_WAIT_FOR_MOUSE_DRAG)
	{
          float diff = sqrt((static_cast<float>(x) - static_cast<float>(m_iFirstEverX))*(static_cast<float>(x) - static_cast<float>(m_iFirstEverX)) +
                              (static_cast<float>(y) - static_cast<float>(m_iFirstEverY))*(static_cast<float>(y) - static_cast<float>(m_iFirstEverY)));
          if(diff < static_cast<float>(pG->tlu(MIN_DRAG_PIXELS)))
          {
	    xxx_UT_DEBUGMSG(("Not yet dragged enough.%f \n", diff));
            //
            // Have to drag 4 pixels before initiating the drag
            //
            return;
          }
	  else
	  {
	    m_iInlineDragMode = FV_InlineDrag_START_DRAGGING;	    
	  }
        }
	m_bFirstDragDone = true;
	if((m_iInlineDragMode != FV_InlineDrag_DRAGGING) && (m_iInlineDragMode != FV_InlineDrag_WAIT_FOR_MOUSE_DRAG) && !m_bDoingCopy)
	{
//
// Haven't started the drag yet so create our image and cut the text.
//
	  _beginGlob();
	  mouseCut(m_iFirstEverX,m_iFirstEverY);
	  m_bTextCut = true;
	}
	clearCursor();
	m_iInlineDragMode = FV_InlineDrag_DRAGGING;
	xxx_UT_DEBUGMSG(("x = %d y = %d width \n",x,y));
	bool bScrollDown = false;
	bool bScrollUp = false;
	bool bScrollLeft = false;
	bool bScrollRight = false;
	m_xLastMouse = x;
	m_yLastMouse = y;
	if(y<=0)
	{
		bScrollUp = true;
	}
	else if( y >= m_pView->getWindowHeight())
	{
		bScrollDown = true;
	}
	if(x <= 0)
	{
		bScrollLeft = true;
	}
	else if(x >= m_pView->getWindowWidth())
	{
		bScrollRight = true;
	}
	if(bScrollDown || bScrollUp || bScrollLeft || bScrollRight)
	{
		if(m_pAutoScrollTimer != NULL)
		{
			return;
		}
		m_pAutoScrollTimer = UT_Timer::static_constructor(_autoScroll, this, pG);
		m_pAutoScrollTimer->set(AUTO_SCROLL_MSECS);
		m_pAutoScrollTimer->start();
		return;
	}
	UT_sint32 dx = 0;
	UT_sint32 dy = 0;
	UT_Rect expX(0,m_recCurFrame.top,0,m_recCurFrame.height);
	UT_Rect expY(m_recCurFrame.left,0,m_recCurFrame.width,0);
	UT_sint32 iext = pG->tlu(3);
	dx = x - m_iLastX;
	dy = y - m_iLastY;
	m_recCurFrame.left += dx;
	m_recCurFrame.top += dy;
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

	if(expX.width > 0)
	{
		pG->setClipRect(&expX);
		m_pView->updateScreen(false);
	}
	if(expY.height > 0)
	{
		pG->setClipRect(&expY);
		m_pView->updateScreen(false);
	}
	pG->setClipRect(NULL);
	bool b = drawImage();
	if(!b)
	{
	  cleanUP();
	  return;
	}
	m_iLastX = x;
	m_iLastY = y;
	pG->setClipRect(NULL);
	PT_DocPosition posAtXY = getPosFromXY(x,y);
	m_pView->_setPoint(posAtXY);
//	m_pView->_fixInsertionPointCoords();
	drawCursor(posAtXY);
  }
  else
  {
        m_iInlineDragMode = FV_InlineDrag_RESIZE;
	if(!m_bFirstDragDone)
	{
		m_iFirstEverX = x;
		m_iFirstEverY = y;
	}
	m_bFirstDragDone = true;
	UT_sint32 diffx = 0;
	UT_sint32 diffy = 0;
	UT_sint32 dx = 0;
	UT_sint32 dy = 0;
	UT_Rect prevRect = m_recCurFrame;
	UT_Rect expX(0,m_recCurFrame.top,0,m_recCurFrame.height);
	UT_Rect expY(m_recCurFrame.left,0,m_recCurFrame.width,0);
	UT_sint32 iext = pG->tlu(3);
	m_xLastMouse = x;
	m_yLastMouse = y;
	switch (m_iDraggingWhat)
	{
	case FV_Inline_DragTopLeftCorner:
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
			pG->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_NE);
			m_iDraggingWhat =  FV_Inline_DragTopRightCorner;
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			pG->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_SW);
			m_iDraggingWhat =  FV_Inline_DragBotLeftCorner;
		}
		break;
	case FV_Inline_DragTopRightCorner:
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
			pG->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_NW);
			m_iDraggingWhat =  FV_Inline_DragTopLeftCorner;
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			pG->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_SE);
			m_iDraggingWhat =  FV_Inline_DragBotRightCorner;
		}
		break;
	case FV_Inline_DragBotLeftCorner:
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
			pG->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_SE);
			m_iDraggingWhat =  FV_Inline_DragBotRightCorner;

		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			pG->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_NW);
			m_iDraggingWhat =  FV_Inline_DragTopLeftCorner;
		}
		break;
	case FV_Inline_DragBotRightCorner:
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
			pG->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_SW);
			m_iDraggingWhat =  FV_Inline_DragBotLeftCorner;
		}
		if(m_recCurFrame.height < 0)
		{
			m_recCurFrame.top = y;
			m_recCurFrame.height = -m_recCurFrame.height;
			pG->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_NE);
			m_iDraggingWhat =  FV_Inline_DragTopRightCorner;
		}
		break;
	case FV_Inline_DragLeftEdge:
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
			pG->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_W);
			m_iDraggingWhat =  FV_Inline_DragRightEdge;
		}
		break;
	case FV_Inline_DragRightEdge:
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
			pG->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_E);
			m_iDraggingWhat =  FV_Inline_DragLeftEdge;
		}
		break;
	case FV_Inline_DragTopEdge:
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
			pG->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_S);
			m_iDraggingWhat =  FV_Inline_DragBotEdge;
		}
		break;
	case FV_Inline_DragBotEdge:
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
			pG->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_N);
			m_iDraggingWhat =  FV_Inline_DragTopEdge;
		}
		break;
	default:
	  break;
	}
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
	if(expX.width > 0)
	{
	        pG->setClipRect(&expX);
		m_pView->updateScreen(false);
	}
	if(expY.height > 0)
	{
	        pG->setClipRect(&expY);
		xxx_UT_DEBUGMSG(("expY.top %d expY.height %d \n",expY.top,expY.height));
		m_pView->updateScreen(false);
	}
	pG->setClipRect(NULL);
	GR_Painter painter(pG);
	//
	// Clear the previous line.
	//
	if(m_screenCache != NULL)
	{
	  prevRect.left -= pG->tlu(1);
	  prevRect.top -= pG->tlu(1);
	  painter.drawImage(m_screenCache,prevRect.left,prevRect.top);
	  DELETEP(m_screenCache);
	}
	//
	// Save the current screen
	//
	UT_Rect rCache = m_recCurFrame;
	rCache.left -= pG->tlu(1);
	rCache.top -= pG->tlu(1);
	rCache.width += pG->tlu(2);
	rCache.height += pG->tlu(2);
	m_screenCache = painter.genImageFromRectangle(rCache);

	// Draw new image box
	UT_Rect box(m_recCurFrame.left, m_recCurFrame.top - pG->tlu(1), m_recCurFrame.width - pG->tlu(1), m_recCurFrame.height - pG->tlu(1));
	m_pView->drawSelectionBox(box, false);
  }
}

void FV_VisualInlineImage::clearCursor(void)
{
	if(m_bCursorDrawn)
	{
		if(m_pDocUnderCursor)
		{
			GR_Painter painter(getGraphics());
			painter.drawImage(m_pDocUnderCursor,m_recCursor.left,m_recCursor.top);
			m_bCursorDrawn = false;
			DELETEP(m_pDocUnderCursor);
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}
}

void FV_VisualInlineImage::drawCursor(PT_DocPosition newPos)
{

	fp_Run * pRunLow = NULL;
	fl_BlockLayout * pBlock = NULL;
	UT_sint32 xLow, yLow;
	UT_uint32 heightCaret;
	UT_sint32 xCaret2, yCaret2;
	bool bDirection=false;
	bool bEOL=false;
	m_pView->_findPositionCoords(newPos, bEOL, xLow, yLow, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRunLow);
	m_recCursor.left = xLow;
	m_recCursor.top = yLow;
	m_recCursor.width =  getGraphics()->tlu(2); // the cursor is 2 device units wide, not
												// logical units
	m_recCursor.height = heightCaret;
	UT_ASSERT(m_pDocUnderCursor == NULL);
	GR_Painter painter(getGraphics());
	m_pDocUnderCursor = painter.genImageFromRectangle(m_recCursor);
	UT_RGBColor black(0,0,0);
	painter.fillRect( black, m_recCursor);
	m_bCursorDrawn = true;
}

/*!
 * This method returns the Attributes/Properties pointer of the image at 
 location (X,y). It return NULL if there is no image at (x,y)
*/
PP_AttrProp * FV_VisualInlineImage::getImageAPFromXY(UT_sint32 x, UT_sint32 y)
{
  PP_AttrProp * pAP = NULL;
  getImageFromSelection(x, y, &pAP );
  return pAP;
}
/*!
 * This method creates an image from the current selection. It sets
 * the drag rectangle, the initial offsets and the initial positions 
 * of the cursor.
 */
void FV_VisualInlineImage::getImageFromSelection(UT_sint32 x, UT_sint32 y,PP_AttrProp ** pAP )
{
//
// OK first work out the locations in the document of the anchor and point
//	
	PT_DocPosition posAtXY = m_pView->getDocPositionFromXY(x,y,false);
	UT_sint32 x1,y1,x2,y2;
	UT_uint32 height;
	
	bool bEOL = false;
	bool bDirection;
	fl_BlockLayout * pBlock = NULL;
	fp_Run * pRun = NULL;
	m_pView->_findPositionCoords(posAtXY,bEOL,x1,y1,x2,y2,height,bDirection,&pBlock,&pRun);
	if(!pBlock)
	{
	    if(pAP != NULL)
	    {
	      *pAP = NULL;
	      return;
	    }
	    m_iInlineDragMode = FV_InlineDrag_NOT_ACTIVE;
	    return;
	}
	if(!pRun)
	{
	    if(pAP != NULL)
	    {
	      *pAP = NULL;
	      return;
	    }
	    m_iInlineDragMode = FV_InlineDrag_NOT_ACTIVE;
	    return;
	}
	while(pRun && (pRun->getLength() == 0))
	  pRun = pRun->getNextRun();
	if(pAP != NULL)
	{
	    if(pRun == NULL)
	      *pAP = NULL;
	    else
	      *pAP = const_cast<PP_AttrProp*>(pRun->getSpanAP());
	    return;
	}
	if(pRun == NULL)
	{
	  m_iInlineDragMode = FV_InlineDrag_NOT_ACTIVE;
	  return;
	}
	if((pRun->getType() !=  FPRUN_IMAGE) && (pRun->getType() !=  FPRUN_EMBED)  )
	{
	  m_iInlineDragMode = FV_InlineDrag_NOT_ACTIVE;
	  return;
	}
	if(pRun->getType() ==  FPRUN_EMBED)
	{
	  m_bIsEmbedded = true;
	}
	else
        {
	  m_bIsEmbedded = false;
	}
	UT_sint32 xoff = 0, yoff = 0;
	pRun->getLine()->getScreenOffsets(pRun, xoff, yoff);
	// Sevior's infamous + 1....
	yoff += pRun->getLine()->getAscent() - pRun->getAscent() + getGraphics()->tlu(1);	// Set the image size in the image selection rect
	m_recCurFrame = UT_Rect(xoff,yoff,pRun->getWidth(),pRun->getHeight());
	if(	m_iInlineDragMode != FV_InlineDrag_WAIT_FOR_MOUSE_DRAG)
	{
	  m_iLastX = x;
	  m_iLastY = y;
	  m_iInitialOffX = x - m_recCurFrame.left;
	  m_iInitialOffY = y - m_recCurFrame.top;
	  GR_Painter painter(getGraphics());
	  DELETEP(m_pDragImage);
	  m_pDragImage = painter.genImageFromRectangle(m_recCurFrame);
	//
	// Record the image attributes/properties
	//
	  m_pImageAP = const_cast<PP_AttrProp*>(pRun->getSpanAP());
	  m_iInlineDragMode = FV_InlineDrag_WAIT_FOR_MOUSE_DRAG;
	}
}

void FV_VisualInlineImage::mouseCut(UT_sint32 x, UT_sint32 y)
{
	getImageFromSelection(x,y);
	m_bDoingCopy = false;
	UT_DEBUGMSG(("Doing Mouse Cut \n"));
	PT_DocPosition posImage = m_pView->getDocPositionFromXY(x,y);
	_beginGlob();
	PT_DocPosition posLow = m_pView->getSelectionAnchor();
	PT_DocPosition posHigh = m_pView->getPoint();
	if(posLow > posHigh)
	{
	     PT_DocPosition pos = posLow;
	     posLow = posHigh;
	     posHigh = pos;
	}
	if((posImage > posHigh) || (posImage < posLow))
	{
	  m_pView->_clearSelection();
	  posLow = posImage;
	  posHigh = posLow+1;
	  m_pView->setPoint(posLow);
	  m_pView->_setSelectionAnchor();
	  m_pView->setPoint(posHigh);
	}
	fl_BlockLayout * pBlock = m_pView->getBlockAtPosition(posLow);
	if(pBlock)
	{
		UT_sint32 x1,x2,y1,y2,iHeight;
		bool bEOL = false;
		bool bDir = false;
		
		fp_Run * pRun = NULL;
		
		pRun = pBlock->findPointCoords(posLow,bEOL,x1,y1,x2,y2,iHeight,bDir);
		while(pRun && ((pRun->getType() != FPRUN_IMAGE) && (pRun->getType() != FPRUN_EMBED)))
		{
			pRun = pRun->getNextRun();
		}
		if(pRun && ((pRun->getType() == FPRUN_IMAGE) || ((pRun->getType() == FPRUN_EMBED))))
		{
		        posLow = pBlock->getPosition() + pRun->getBlockOffset();
			// we've found an image: do not move the view, just select the image and exit
			m_pView->cmdSelect(posLow,posLow+1);
			// Set the cursor context to image selected.
			// m_pView->getMouseContext(x,y);
		}
	}
	m_pView->cmdCharDelete(true,1);
	m_pView->updateScreen(false);
	drawImage();
}


UT_sint32 FV_VisualInlineImage::getGlobCount()
{
	return m_iGlobCount;
}

void  FV_VisualInlineImage::_beginGlob(void)
{
	getDoc()->beginUserAtomicGlob();
	m_iGlobCount++;
	xxx_UT_DEBUGMSG(("Begin Glob count %d \n",m_iGlobCount));
}


void  FV_VisualInlineImage::_endGlob(void)
{
	getDoc()->endUserAtomicGlob();
	m_iGlobCount--;
	xxx_UT_DEBUGMSG(("End Glob count %d \n",m_iGlobCount));
}

PD_Document *    FV_VisualInlineImage::getDoc(void) const
{
	return m_pView->getDocument();
}

FL_DocLayout *   FV_VisualInlineImage::getLayout(void) const
{
        return m_pView->getLayout();
}

void FV_VisualInlineImage::mouseLeftPress(UT_sint32 x, UT_sint32 y)
{
	if(!isActive())
	{
		m_iInlineDragMode = FV_InlineDrag_WAIT_FOR_MOUSE_CLICK;
		setDragType(x,y,false);
		return;
	}
	if(getImageAPFromXY(x,y) != m_pImageAP)
	{
	        cleanUP();
		m_iInlineDragMode = FV_InlineDrag_WAIT_FOR_MOUSE_CLICK;
		setDragType(x,y,false);
		//
		// Select the new image
		//
		PT_DocPosition pos = m_pView->getDocPositionFromXY(x, y);
		fl_BlockLayout * pBlock = m_pView->getBlockAtPosition(pos);
		UT_sint32 x1,x2,y1,y2,iHeight;
		bool bEOL = false;
		bool bDir = false;
		fp_Run * pRun = NULL;
		pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,iHeight,bDir);
		while(pRun && ((pRun->getType() != FPRUN_IMAGE) && (pRun->getType() != FPRUN_EMBED)))
		{
			pRun = pRun->getNextRun();
		}
		if(pRun && ((pRun->getType() == FPRUN_IMAGE) || ((pRun->getType() == FPRUN_EMBED))))
		{
			// we've found an image: do not move the view, just select the image and exit
			m_pView->cmdSelect(pos,pos+1);
			// Set the cursor context to image selected.
			m_pView->getMouseContext(x, y);
		}
	}
//
// Find the type of drag we should do.
//
	if((FV_InlineDrag_WAIT_FOR_MOUSE_CLICK == m_iInlineDragMode)  ||
	   (FV_InlineDrag_WAIT_FOR_MOUSE_DRAG ==  m_iInlineDragMode))
	{
	        m_iInlineDragMode = FV_InlineDrag_WAIT_FOR_MOUSE_DRAG;
		setDragType(x,y,false); // was true
		if(FV_Inline_DragNothing == m_iDraggingWhat)
		{
		  cleanUP();
		  m_pView->warpInsPtToXY(x,y,true);
		}
		else
		{
			if( m_iDraggingWhat != FV_Inline_DragWholeImage)
			{
				m_iInlineDragMode = FV_InlineDrag_RESIZE;
			}
			else
			{
				m_iInlineDragMode = FV_InlineDrag_DRAGGING;
				m_iLastX = x;
				m_iLastY = y;
			}
			if(getGraphics() && getGraphics()->getCaret())
			{
			        getGraphics()->getCaret()->disable();
			        m_pView->m_countDisable++;
			}
		}
		return;
	}
	drawImage();
}

void FV_VisualInlineImage::cleanUP(void)
{
  m_iInlineDragMode = FV_InlineDrag_NOT_ACTIVE;
  m_iDraggingWhat = FV_Inline_DragNothing;
  DELETEP(m_pDragImage);
  DELETEP(m_pDocUnderCursor);
  DELETEP(m_screenCache);
  m_recCurFrame.top =0;
  m_recCurFrame.left =0;
  m_recCurFrame.width =0;
  m_recCurFrame.height =0;
  XAP_Frame * pFrame = static_cast<XAP_Frame*>(m_pView->getParentData());
  if(pFrame)
  {
      EV_Mouse * pMouse = pFrame->getMouse();
      if(pMouse)
      {
	  pMouse->clearMouseContext();
      }
  }
  m_pView->m_prevMouseContext = EV_EMC_TEXT;
  m_pView->setCursorToContext();
  m_iLastX = 0;
  m_iLastY = 0;
  m_iFirstEverX = 0;
  m_iFirstEverY = 0;
  m_iInitialOffX = 0;
  m_iInitialOffY = 0;
  m_bFirstDragDone = false;
  m_pImageAP = NULL;
  m_bDoingCopy = false;
  while(m_iGlobCount > 0)
    _endGlob();
 
}

/*!
 * This method finds the correct image on the screen associated with the
 * (x,y) point.
 * It sets the FV_ImageDragMode and the FV_InlineDragWhat mode depending
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
 * Also sets the image attributes/property list variables 
 * m_sAttributes, m_sProperties
 */
void FV_VisualInlineImage::setDragType(UT_sint32 x,UT_sint32 y, bool bDrawImage){
	xxx_UT_DEBUGMSG(("setDragType called InlineDrag mode %d \n",m_iInlineDragMode));
	//
	// OK find the coordinates of the image and generate the image to be
	// dragged and resized.
	//
	getImageFromSelection(x, y);

	if( m_iInlineDragMode == FV_InlineDrag_NOT_ACTIVE)
	{
	  return;
	}
	UT_sint32 ires = getGraphics()->tlu(FRAME_HANDLE_SIZE); // 6 pixels wide hit area
	UT_sint32 iLeft = m_recCurFrame.left;
	UT_sint32 iRight = m_recCurFrame.left + m_recCurFrame.width;
	UT_sint32 iTop = m_recCurFrame.top;
	UT_sint32 iBot = iTop + m_recCurFrame.height;
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
		m_iDraggingWhat = FV_Inline_DragTopLeftCorner;
	}
//
// top Right
//
	else if((iRight - ires < x) && (x < iRight) && (iTop < y) && (y < iTop + ires))
	{
		m_iDraggingWhat = FV_Inline_DragTopRightCorner;
	}
//
// bot left
//
	else if((iLeft < x) && (x < iLeft + ires) && (iBot > y) && (y > iBot - ires))
	{
		m_iDraggingWhat = FV_Inline_DragBotLeftCorner;
	}

//
// Bot Right
//
	else if((iRight - ires < x) && (x < iRight) && (iBot > y) && (y > iBot - ires))
	{
		m_iDraggingWhat = FV_Inline_DragBotRightCorner;
	}
//
// top Edge
//
	else if( bX && bTop)
	{
		m_iDraggingWhat = FV_Inline_DragTopEdge;
	}
//
// left Edge
//
	else if(bLeft && bY)
	{
		m_iDraggingWhat = FV_Inline_DragLeftEdge;
	}
//
// right Edge
//
	else if(bRight && bY)
	{
		m_iDraggingWhat = FV_Inline_DragRightEdge;
	}
//
// bot Edge
//
	else if(bBot && bX)
	{
		m_iDraggingWhat = FV_Inline_DragBotEdge;
	}
	else
	{
		if( bX && bY)
		{
			m_iDraggingWhat = FV_Inline_DragWholeImage;
			xxx_UT_DEBUGMSG(("Dragging Whole Image \n"));
		}
		else
		{
			m_iDraggingWhat = FV_Inline_DragNothing;
			return;
		}
	}
	if(bDrawImage && (m_recCurFrame.width > 0) && (m_recCurFrame.height >0))
	{
		drawImage();
	}
	m_iLastX = x;
	m_iLastY = y;
	xxx_UT_DEBUGMSG(("Initial width %d height %d \n",m_recCurFrame.width,m_recCurFrame.height));
	xxx_UT_DEBUGMSG((" Dragging What %d \n",m_iDraggingWhat));
	m_pView->setCursorToContext();
}

void FV_VisualInlineImage::mouseCopy(UT_sint32 x, UT_sint32 y)
{
        if(m_pView->isSelectionEmpty())
	{
	  PT_DocPosition pos = m_pView->getDocPositionFromXY(x, y);
	  fl_BlockLayout * pBlock = m_pView->getBlockAtPosition(pos);
	  if(pBlock)
	  {
		UT_sint32 x1,x2,y1,y2,iHeight;
		bool bEOL = false;
		bool bDir = false;
		
		fp_Run * pRun = NULL;
		
		pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,iHeight,bDir);
		while(pRun && ((pRun->getType() != FPRUN_IMAGE) && (pRun->getType() != FPRUN_EMBED)  ))
		{
			pRun = pRun->getNextRun();
		}
		if(pRun && ((pRun->getType() == FPRUN_IMAGE) || (pRun->getType() == FPRUN_EMBED)  ))
		{
			// we've found an image: do not move the view, just select the image and exit
		        if(pRun->getType() == FPRUN_EMBED)
			{
			     m_bIsEmbedded = true;
			}
			else
			{
			     m_bIsEmbedded = false;
			}
			m_pView->cmdSelect(pos,pos+1);
			// Set the cursor context to image selected.
			m_pView->getMouseContext(x, y);
		}
		else
		{
		  cleanUP();
		  return;
		}
	  }
	  else
	  {
	    cleanUP();
	    return;
	  }
	}
	m_iInlineDragMode= FV_InlineDrag_START_DRAGGING;
	m_iDraggingWhat = FV_Inline_DragWholeImage;
	getImageFromSelection(x,y);
	m_pView->m_prevMouseContext = EV_EMC_IMAGESIZE;
	m_pView->setCursorToContext();
	m_pView->updateScreen(false);
	drawImage();
	m_bTextCut = false;
	m_bDoingCopy = true;
	//
	// Get a copy of the image data
	//
	const UT_ByteBuf * pBytes = NULL;
	const char * dataId = NULL;
	m_pView->getSelectedImage(&dataId);
	if(dataId == NULL)
	{
	  UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	  cleanUP();
	  return;
	}
	getDoc()->getDataItemDataByName ( dataId, &pBytes, NULL, NULL );
	//
	// Save it in the document under a new name
	//
	UT_uint32 uid = getDoc()->getUID(UT_UniqueId::Image);
	UT_UTF8String sDataID = dataId;
	UT_UTF8String sUID;
	UT_UTF8String_sprintf(sUID, "%d",uid);
	sDataID += sUID;
	_beginGlob();
	//
	// Make a copy of it and save it under a new name.
	//
	getDoc()->createDataItem(sDataID.utf8_str(), false, pBytes, NULL,NULL);
	m_sCopyName = sDataID;
	m_pView->_resetSelection();
}

PT_DocPosition FV_VisualInlineImage::getPosFromXY(UT_sint32 x, UT_sint32 y)
{
//
// Convert this to a document position and paste!
//
	x -= m_iInitialOffX;
	y -= m_iInitialOffY;
	y += getGraphics()->tlu(6); //Otherwise it's too easy to hit the line above
	PT_DocPosition posAtXY = m_pView->getDocPositionFromXY(x,y,false);
	return posAtXY;
}

/*!
 * x and y is the location in the document windows of the mouse in logical
 * units.
 */
void FV_VisualInlineImage::mouseRelease(UT_sint32 x, UT_sint32 y)
{
        if(m_pAutoScrollTimer != NULL)
	{
		m_pAutoScrollTimer->stop();
		DELETEP(m_pAutoScrollTimer);
	}
	clearCursor();
	if(((m_iInlineDragMode != FV_InlineDrag_DRAGGING) && (m_iInlineDragMode != FV_InlineDrag_RESIZE) ) || !m_bFirstDragDone)
	{
	     PT_DocPosition pos = m_pView->getDocPositionFromXY(x,y);
	     fl_BlockLayout * pBlock = m_pView->getBlockAtPosition(pos);
	     if(pBlock)
	     {
		   UT_sint32 x1,x2,y1,y2,iHeight;
		   bool bEOL = false;
		   bool bDir = false;
		
		   fp_Run * pRun = NULL;
		   
		   pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,iHeight,bDir);
		   while(pRun && ((pRun->getType() != FPRUN_IMAGE) && (pRun->getType() != FPRUN_EMBED)))
		   {
			pRun = pRun->getNextRun();
		   }
		   if(pRun && ((pRun->getType() == FPRUN_IMAGE) || ((pRun->getType() == FPRUN_EMBED))))
		   {
			m_bFirstDragDone = false;
			return;
		   }
		   else
		   {
//
// we didn't actually drag anything. Just release the selection.
//
		        cleanUP();
			m_pView->warpInsPtToXY(x, y,true);
			return;
		   }
	     }
	}
	m_bFirstDragDone = false;
	if(FV_Inline_DragWholeImage == m_iDraggingWhat)
	{
	  PT_DocPosition posAtXY = getPosFromXY(x,y);
	  m_pView->setPoint(posAtXY);
	  getGraphics()->setClipRect( &m_recCurFrame);
	  m_pView->updateScreen(false);
	  getGraphics()->setClipRect(NULL);
	  m_iInlineDragMode  = FV_InlineDrag_NOT_ACTIVE;
	  m_pView->getMouseContext(x,y);
	  m_iInitialOffX = 0;
	  m_iInitialOffY = 0;
	  PT_DocPosition oldPoint = m_pView->getPoint();
	  if(oldPoint < 2)
	  {
	      oldPoint = 2;
	  }
	  m_iFirstEverX = 0;
	  m_iFirstEverY = 0;
	//
	// Fixme Put in code to insert image here.
	//
	  const XML_Char* szDataID = 0;
	  const XML_Char* szTitle = 0;
	  const XML_Char* szDescription = 0;
	  const  XML_Char* szWidth = 0;
	  const  XML_Char * szHeight = 0;
	  const XML_Char * szEmbed= NULL;
	  if(!m_bDoingCopy)
	  {
	    bool bFound = m_pImageAP->getAttribute("dataid",szDataID);
	    if(!bFound)
	    {
	      return;
	    }
	  }
	  else
	  {
	    szDataID = m_sCopyName.utf8_str();
	  }
	  if(m_bIsEmbedded)
	  {
	    bool bFound = m_pImageAP->getProperty("embed-type",szEmbed);
	    if(!bFound)
	    {
	      return;
	    }
	  }
	  m_bDoingCopy = false;
	  UT_String sProps;
	  UT_String sProp;
	  UT_String sVal;
	  bool bFound = m_pImageAP->getProperty("width",szWidth);
	  if(bFound)
	  {
	    sProp = "width";
	    sVal = szWidth;
	    UT_String_setProperty(sProps,sProp,sVal);
	  }
	  bFound = m_pImageAP->getProperty("height",szHeight);
	  if(!bFound)
	  {
	    sProp = "height";
	    sVal = szHeight;
	    UT_String_setProperty(sProps,sProp,sVal);
	  }

	  bFound = m_pImageAP->getProperty("title",szTitle);
	  bFound = m_pImageAP->getProperty("alt",szDescription);
	  sProps += "; height:";
	  sProps += szHeight;
	  const XML_Char*	attributes[] = {
	    "dataid", NULL,
	    "title",NULL,
	    "alt",NULL,
	    PT_PROPS_ATTRIBUTE_NAME, NULL,
	    NULL,NULL};
	  attributes[1] = szDataID;
	  attributes[3] = szTitle;
	  attributes[5] = szDescription;
	  if(m_bIsEmbedded)
	  {
	      sProp="embed-type";
	      sVal = szEmbed;
	      UT_String_setProperty(sProps,sProp,sVal);
	  }
	  if(sProps.size() > 0)
	  {
	    attributes[7] = sProps.c_str();
	  }
	  else
	  {
	    attributes[6] = NULL;
	  }
	  m_pView->_saveAndNotifyPieceTableChange();
	  xxx_UT_DEBUGMSG(("Doing Insert Image at %d \n",m_pView->getPoint()));
	  if(!m_bIsEmbedded)
	  {
	    getDoc()->insertObject(m_pView->getPoint(), PTO_Image, attributes, NULL);
	  }
	  else
	  {
	    getDoc()->insertObject(m_pView->getPoint(), PTO_Embed, attributes, NULL);
	  }
	  m_pView->_restorePieceTableState();
	  m_pView->_updateInsertionPoint();
	  m_pView->_generalUpdate();
	  PT_DocPosition newPoint = m_pView->getPoint();
	  DELETEP(m_pDragImage);
	  while(m_iGlobCount > 0)
	    _endGlob();
	  m_pView->cmdSelect(oldPoint,newPoint);
	  m_bTextCut = false;
	}
	else
	{
	  //
	  // Do a resize.
	  //
	  //	  getGraphics()->setClipRect( &m_recCurFrame);
	  //m_pView->updateScreen(false);
	  //getGraphics()->setClipRect(NULL);
	  m_bDoingCopy = false;
	  m_iInlineDragMode  = FV_InlineDrag_NOT_ACTIVE;
	  UT_Rect newImgBounds = m_recCurFrame;
	  const fp_PageSize & page = m_pView->getPageSize ();		
	  double max_width = 0., max_height = 0.;
	  max_width  = page.Width (DIM_PX);
	  max_height = page.Height (DIM_PX);
		
	  // some range checking stuff
	  newImgBounds.width = abs(newImgBounds.width);
	  newImgBounds.height = abs(newImgBounds.height);
		
	  if (newImgBounds.width > max_width)
	    newImgBounds.width = static_cast<UT_sint32>(max_width);
		
	  if (newImgBounds.height > max_height)
			newImgBounds.height = static_cast<UT_sint32>(max_height);
		
	  if (newImgBounds.width == 0)
	    newImgBounds.width = getGraphics()->tlu(2);

	  if (newImgBounds.height == 0)
	    newImgBounds.height = getGraphics()->tlu(2);
		
	//
	// Clear the previous line.
	//
	    GR_Painter painter(getGraphics());
	    if(m_screenCache != NULL)
	    {
	        UT_Rect prevRect = m_recCurFrame;
		prevRect.left -= getGraphics()->tlu(1);
		prevRect.top -= getGraphics()->tlu(1);
		painter.drawImage(m_screenCache,prevRect.left,prevRect.top);
		DELETEP(m_screenCache);
	    }
	    getGraphics()->setLineProperties(getGraphics()->tlu(1), GR_Graphics::JOIN_MITER, GR_Graphics::CAP_PROJECTING, GR_Graphics::LINE_SOLID);

	    //
	    // Do the resize
	    //		
	    UT_UTF8String sWidth;
	    UT_UTF8String sHeight;

	    const XML_Char * properties[] = {"width", NULL, "height", NULL, 0};

	    {
	      UT_LocaleTransactor t(LC_NUMERIC, "C");
	      UT_UTF8String_sprintf(sWidth, "%fin", UT_convertDimToInches(newImgBounds.width, DIM_PX));
	      UT_UTF8String_sprintf(sHeight, "%fin", UT_convertDimToInches(newImgBounds.height, DIM_PX));
	    }
	    properties[1] = sWidth.utf8_str();
	    properties[3] = sHeight.utf8_str();
	    m_pView->setCharFormat(properties);
	    cleanUP();
	}
}

bool FV_VisualInlineImage::drawImage(void)
{
	if(m_pDragImage == NULL)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	GR_Painter painter(getGraphics());
	xxx_UT_DEBUGMSG(("Draw Inline image \n"));
	painter.drawImage(m_pDragImage,m_recCurFrame.left,m_recCurFrame.top);
	return true;
}
