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


#define MIN_DRAG_PIXELS 1

FV_VisualInlineImage::FV_VisualInlineImage (FV_View * pView)
	: m_pView (pView), 
	  m_iInlineDragMode(FV_InlineDrag_NOT_ACTIVE),
	  m_pDragImage(NULL),
	  m_iLastX(0),
	  m_iLastY(0),
	  m_recCurFrame(0,0,0,0),
	  m_iInitialOffX(0),
	  m_iInitialOffY(0),
	  m_bTextCut(false),
	  m_pDocUnderCursor(NULL),
	  m_bCursorDrawn(false),
	  m_recCursor(0,0,0,0),
	  m_pAutoScrollTimer(NULL),
	  m_xLastMouse(1),
	  m_yLastMouse(1),
	  m_bDoingCopy(false),
	  m_iDraggingWhat( FV_Inline_DragNothing ),
	  m_iGlobCount(0)
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
        if(m_iInlineDragMode  == FV_InlineDrag_NOT_ACTIVE)
        {
	  m_iInitialOffX = x;
	  m_iInitialOffY = y;
	  m_iInlineDragMode  = FV_InlineDrag_WAIT_FOR_MOUSE_DRAG;
	  UT_DEBUGMSG(("Initial call for drag -1\n"));
	  return;
	}
	if((m_iInitialOffX == 0) && (m_iInitialOffY == 0))
	{
	  m_iInitialOffX = x;
	  m_iInitialOffY = y;
	  m_iInlineDragMode = FV_InlineDrag_WAIT_FOR_MOUSE_DRAG;
	  UT_DEBUGMSG(("Initial call for drag -2 \n"));
	}
	if(m_iInlineDragMode == FV_InlineDrag_WAIT_FOR_MOUSE_DRAG)
	{
          float diff = sqrt((static_cast<float>(x) - static_cast<float>(m_iInitialOffX))*(static_cast<float>(x) - static_cast<float>(m_iInitialOffX)) +
                              (static_cast<float>(y) - static_cast<float>(m_iInitialOffY))*(static_cast<float>(y) - static_cast<float>(m_iInitialOffY)));
          if(diff < static_cast<float>(getGraphics()->tlu(MIN_DRAG_PIXELS)))
          {
	    UT_DEBUGMSG(("Not yet dragged enough.%f \n", diff));
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
	if((m_iInlineDragMode != FV_InlineDrag_DRAGGING) && (m_iInlineDragMode != FV_InlineDrag_WAIT_FOR_MOUSE_DRAG) && !m_bDoingCopy)
	{
//
// Haven't started the drag yet so create our image and cut the text.
//

		m_pView->getDocument()->beginUserAtomicGlob();
		mouseCut(m_iInitialOffX,m_iInitialOffY);
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
		m_pAutoScrollTimer = UT_Timer::static_constructor(_autoScroll, this, getGraphics());
		m_pAutoScrollTimer->set(AUTO_SCROLL_MSECS);
		m_pAutoScrollTimer->start();
		return;
	}
	UT_sint32 dx = 0;
	UT_sint32 dy = 0;
	UT_Rect expX(0,m_recCurFrame.top,0,m_recCurFrame.height);
	UT_Rect expY(m_recCurFrame.left,0,m_recCurFrame.width,0);
	UT_sint32 iext = getGraphics()->tlu(3);
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
		getGraphics()->setClipRect(&expX);
		m_pView->updateScreen(false);
	}
	if(expY.height > 0)
	{
		getGraphics()->setClipRect(&expY);
		m_pView->updateScreen(false);
	}
	getGraphics()->setClipRect(NULL);
	drawImage();
	m_iLastX = x;
	m_iLastY = y;
	getGraphics()->setClipRect(NULL);
	PT_DocPosition posAtXY = getPosFromXY(x,y);
	m_pView->_setPoint(posAtXY);
//	m_pView->_fixInsertionPointCoords();
	drawCursor(posAtXY);
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
 * This method creates an image from the current selection. It sets
 * the drag rectangle, the initial offsets and the initial positions 
 * of the cursor.
 */
void FV_VisualInlineImage::getImageFromSelection(UT_sint32 x, UT_sint32 y)
{
//
// OK first work out the locations in the document of the anchor and point
//	
	PT_DocPosition posLow = 0;
	PT_DocPosition posHigh = 0;

	fp_Run * pRunLow = NULL;
	UT_sint32 xLow, yLow,xHigh,yHigh;
	UT_uint32 heightCaret;
	UT_sint32 xCaret2, yCaret2;
	bool bDirection = false;
	bool bEOL = false;
	if(m_pView->getSelectionAnchor() < m_pView->getPoint())
	{
	  posLow = m_pView->getSelectionAnchor();
	  posHigh = m_pView->getPoint();
	}
	else
	{
	  posLow = m_pView->getPoint();
	  posHigh = m_pView->getSelectionAnchor();
	}
	m_pView->_findPositionCoords(posLow, bEOL, xLow, yLow, xCaret2, yCaret2, heightCaret, bDirection, NULL, &pRunLow);
	fp_Line * pLineLow = pRunLow->getLine();
	fp_Run * pRunHigh = NULL;
	m_pView->_findPositionCoords(posHigh, bEOL, xHigh, yHigh, xCaret2, yCaret2, heightCaret, bDirection, NULL, &pRunHigh);
//
// OK deal with the nice case of the selection just on the single line
//
	m_pView->_findPositionCoords(posLow, bEOL, xLow, yLow, xCaret2, yCaret2, heightCaret, bDirection, NULL, &pRunLow);
	UT_sint32 xx,yy;
	pLineLow->getScreenOffsets(pRunLow,xx,yy);
	m_recCurFrame.left = xLow;
	m_recCurFrame.width = xHigh - xLow;
	m_recCurFrame.top = yy;
	m_recCurFrame.height = pLineLow->getHeight();

	m_iLastX = x;
	m_iLastY = y;
	m_iInitialOffX = x - m_recCurFrame.left;
	m_iInitialOffY = y - m_recCurFrame.top;
	GR_Painter painter(getGraphics());
	UT_RGBColor black(0,0,0);
	UT_RGBColor trans(0,0,0,true);
	m_pDragImage = painter.genImageFromRectangle(m_recCurFrame);
}

void FV_VisualInlineImage::mouseCut(UT_sint32 x, UT_sint32 y)
{
	getImageFromSelection(x,y);
	PT_DocPosition pos1 = m_pView->getSelectionAnchor();
	PT_DocPosition pos2 = m_pView->getPoint();
	if(pos1 > pos2)
	{
	  pos2 = m_pView->getSelectionAnchor();
	  pos1 = m_pView->getPoint();
	}
	m_pView->copyToLocal(pos1,pos2);
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
	UT_DEBUGMSG(("Begin Glob count %d \n",m_iGlobCount));
}


void  FV_VisualInlineImage::_endGlob(void)
{
	getDoc()->endUserAtomicGlob();
	m_iGlobCount--;
	UT_DEBUGMSG(("End Glob count %d \n",m_iGlobCount));
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
		setDragType(x,y,true);
		return;
	}
//
// Find the type of drag we should do.
//
	if((FV_InlineDrag_WAIT_FOR_MOUSE_CLICK == m_iInlineDragMode)  ||
	   (FV_InlineDrag_WAIT_FOR_MOUSE_DRAG ==  m_iInlineDragMode))
	{
		setDragType(x,y,true);
		if(FV_Inline_DragNothing == m_iDraggingWhat)
		{
		        m_iInlineDragMode =FV_InlineDrag_NOT_ACTIVE;
			DELETEP(m_pDragImage);
			DELETEP(m_pDocUnderCursor);
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
			m_recCurFrame.width = 0;
			m_recCurFrame.height = 0;
			m_iDraggingWhat = FV_Inline_DragNothing;
			m_iLastX = 0;
			m_iLastY = 0;
			while(m_iGlobCount > 0)
				_endGlob();
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
				m_iInitialOffX = m_recCurFrame.left;
				m_iInitialOffY = m_recCurFrame.top;
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

void FV_VisualInlineImage::setDragType(UT_sint32 x,UT_sint32 y, bool bDrawImage){

}

void FV_VisualInlineImage::mouseCopy(UT_sint32 x, UT_sint32 y)
{
	getImageFromSelection(x,y);
	bool bPasteTableCol = (m_pView->getPrevSelectionMode() == FV_SelectionMode_TableColumn);
	if(!bPasteTableCol)
	{
		PT_DocPosition pos1 = m_pView->getSelectionAnchor();
		PT_DocPosition pos2 = m_pView->getPoint();
		if(pos1 > pos2)
		{
			pos2 = m_pView->getSelectionAnchor();
			pos1 = m_pView->getPoint();
		}
		m_pView->copyToLocal(pos1,pos2);
	}
	else
	{
		m_pView->cmdCopy();
	}
	m_pView->updateScreen(false);
	drawImage();
	m_iInlineDragMode= FV_InlineDrag_START_DRAGGING;
	m_bTextCut = false;
	m_bDoingCopy = true;
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
	m_bDoingCopy = false;
	clearCursor();
	if(m_iInlineDragMode != FV_InlineDrag_DRAGGING)
	{
//
// we didn't actually drag anything. Just release the selection.
//
		m_pView->warpInsPtToXY(x, y,true);
		return;
	}
	PT_DocPosition posAtXY = getPosFromXY(x,y);
	m_pView->setPoint(posAtXY);
	getGraphics()->setClipRect(&m_recCurFrame);
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
	bool bInFrame = m_pView->isInFrame(oldPoint);
	m_pView->pasteFromLocalTo(m_pView->getPoint());
	PT_DocPosition newPoint = m_pView->getPoint();
	DELETEP(m_pDragImage);
	if(m_bTextCut)
	{
		m_pView->getDocument()->endUserAtomicGlob(); // End the big undo block
	}
	if(m_pView->getDocument()->isEndFootnoteAtPos(newPoint))
	{
	        newPoint++;
	}
	bool bFinalFrame = m_pView->isInFrame(newPoint) && !m_pView->getDocument()->isFrameAtPos(newPoint);
	bool bDoSelect = true;
	if(bInFrame && !bFinalFrame)
	{
	     bDoSelect = false;
	}
	if(bDoSelect)
	{
	  m_pView->cmdSelect(oldPoint,newPoint);
	}
	m_bTextCut = false;
}

void FV_VisualInlineImage::drawImage(void)
{
	if(m_pDragImage == NULL)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	GR_Painter painter(getGraphics());
	painter.drawImage(m_pDragImage,m_recCurFrame.left,m_recCurFrame.top);
}
