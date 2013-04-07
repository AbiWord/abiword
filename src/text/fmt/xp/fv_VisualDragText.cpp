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

#include <math.h>
#include "fv_VisualDragText.h"
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
#include "fv_ViewDoubleBuffering.h"

#define MIN_DRAG_PIXELS 8

FV_VisualDragText::FV_VisualDragText (FV_View * pView)
	: m_pView (pView), 
	  m_iVisualDragMode(FV_VisualDrag_NOT_ACTIVE),
	  m_pDragImage(NULL),
	  m_iLastX(0),
	  m_iLastY(0),
	  m_recCurFrame(0,0,0,0),
	  m_iInitialOffX(0),
	  m_iInitialOffY(0),
	  m_recOrigLeft(0,0,0,0),
	  m_recOrigRight(0,0,0,0),
	  m_bTextCut(false),
	  m_pDocUnderCursor(NULL),
	  m_bCursorDrawn(false),
	  m_recCursor(0,0,0,0),
	  m_pAutoScrollTimer(NULL),
	  m_xLastMouse(1),
	  m_yLastMouse(1),
	  m_bDoingCopy(false),
	  m_bNotDraggingImage(false),
	  m_bSelectedRow(false)
{
	UT_ASSERT_HARMLESS(pView);
}

FV_VisualDragText::~FV_VisualDragText()
{
	DELETEP(m_pDragImage);
	if(m_pAutoScrollTimer != NULL)
	{
		m_pAutoScrollTimer->stop();
		DELETEP(m_pAutoScrollTimer);
	}
}

bool FV_VisualDragText::isActive(void) const
{
	return (FV_VisualDrag_NOT_ACTIVE != m_iVisualDragMode);
}

GR_Graphics * FV_VisualDragText::getGraphics(void) const
{
	return m_pView->getGraphics();
}

void FV_VisualDragText::setMode(FV_VisualDragMode iEditMode)
{
	m_iVisualDragMode = iEditMode;
	if(iEditMode == FV_VisualDrag_NOT_ACTIVE)
	{	
	    m_iInitialOffX = 0;
	    m_iInitialOffY = 0;
	    m_iLastX = 0;
	    m_iLastY = 0;
	    m_xLastMouse = 0;
	    m_yLastMouse = 0;
	}
}

static UT_sint32 iExtra = 0;
static bool bScrollRunning = false;
static UT_Worker * s_pScroll = NULL;

void FV_VisualDragText::_actuallyScroll(UT_Worker * pWorker)
{
	UT_return_if_fail(pWorker);

	// this is a static callback method and does not have a 'this' pointer.

	FV_VisualDragText * pVis = static_cast<FV_VisualDragText *>(pWorker->getInstanceData());
	UT_return_if_fail(pVis);
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
	        UT_sint32 minScroll = pView->getGraphics()->tlu(20);

		if(bScrollUp)
		{
		        UT_sint32 yscroll = abs(y);
			if(yscroll < minScroll)
			    yscroll = minScroll;
			pView->cmdScroll(AV_SCROLLCMD_LINEUP, static_cast<UT_uint32>( yscroll) + iExtra);
		}
		else if(bScrollDown)
		{
		        UT_sint32 yscroll = y - pView->getWindowHeight();
			if(yscroll < minScroll)
			    yscroll = minScroll;
			pView->cmdScroll(AV_SCROLLCMD_LINEDOWN, static_cast<UT_uint32>(yscroll) + iExtra);
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
		iExtra = 0;
		return;
	}
	else
	{
		if(pVis->m_pAutoScrollTimer)
			pVis->m_pAutoScrollTimer->stop();
		DELETEP(pVis->m_pAutoScrollTimer);
	}
	s_pScroll->stop();
	delete s_pScroll;
	s_pScroll = NULL;
	bScrollRunning = false;
	iExtra = 0;

}

void FV_VisualDragText::_autoScroll(UT_Worker * pWorker)
{

	// this is a static callback method and does not have a 'this' pointer.

	UT_return_if_fail(pWorker);
	FV_VisualDragText * pVis = static_cast<FV_VisualDragText *>(pWorker->getInstanceData());
	UT_return_if_fail(pVis);
	if(bScrollRunning)
	{
	    UT_DEBUGMSG(("Dropping VisualDragText autoscroll !!!!!!! \n"));
	    if(iExtra < pVis->getGraphics()->tlu(600))
	      iExtra += pVis->getGraphics()->tlu(20);
	    return;
	}

	int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	s_pScroll = UT_WorkerFactory::static_constructor (_actuallyScroll,pVis, inMode, outMode);

	// If the worker is working on a timer instead of in the idle
	// time, set the frequency of the checks.
	if ( UT_WorkerFactory::TIMER == outMode )
	{
		// this is really a timer, so it's safe to static_cast it
		static_cast<UT_Timer*>(s_pScroll)->set(100);
	}
	bScrollRunning = true;
	iExtra = 0;
	s_pScroll->start();
}

void FV_VisualDragText::mouseDrag(UT_sint32 x, UT_sint32 y)
{
  _mouseDrag(x,y);
}

void FV_VisualDragText::_mouseDrag(UT_sint32 x, UT_sint32 y)
{
 	  //
	  // Don't try to drag the entire document.
	  //
  if(!m_bDoingCopy && (m_pView->isSelectAll() && !m_pView->isHdrFtrEdit())&&(m_iVisualDragMode != FV_VisualDrag_DRAGGING))
	 {
	     setMode(FV_VisualDrag_NOT_ACTIVE);
	     return;
	 }

        if(m_iVisualDragMode == FV_VisualDrag_NOT_ACTIVE)
        {
	  m_iInitialOffX = x;
	  m_iInitialOffY = y;
	  m_iVisualDragMode = FV_VisualDrag_WAIT_FOR_MOUSE_DRAG;
	  UT_DEBUGMSG(("Initial call for drag -1\n"));
	  return;
	}
	if((m_iInitialOffX == 0) && (m_iInitialOffY == 0))
	{
	  m_iInitialOffX = x;
	  m_iInitialOffY = y;
	  m_iVisualDragMode = FV_VisualDrag_WAIT_FOR_MOUSE_DRAG;
	  UT_DEBUGMSG(("Initial call for drag -2 \n"));
	}
	if(m_iVisualDragMode == FV_VisualDrag_WAIT_FOR_MOUSE_DRAG)
	{
          double diff = sqrt((static_cast<double>(x) - static_cast<double>(m_iInitialOffX))*(static_cast<double>(x) - static_cast<double>(m_iInitialOffX)) +
                              (static_cast<double>(y) - static_cast<double>(m_iInitialOffY))*(static_cast<double>(y) - static_cast<double>(m_iInitialOffY)));
          if(diff < static_cast<double>(getGraphics()->tlu(MIN_DRAG_PIXELS)))
          {
	    UT_DEBUGMSG(("Not yet dragged enough.%f \n", diff));
            //
            // Have to drag 4 pixels before initiating the drag
            //
            return;
          }
	  else
	  {
	    m_iVisualDragMode = FV_VisualDrag_START_DRAGGING;
	    XAP_Frame * pFrame = static_cast<XAP_Frame*>(m_pView->getParentData());
	    if (pFrame)
	      pFrame->dragText();
	  }
        }
	if((m_iVisualDragMode != FV_VisualDrag_DRAGGING) && (m_iVisualDragMode != FV_VisualDrag_WAIT_FOR_MOUSE_DRAG) && !m_bDoingCopy)
	{
//
// Haven't started the drag yet so create our image and cut the text.
//
		m_pView->getDocument()->beginUserAtomicGlob();
		mouseCut(m_iInitialOffX,m_iInitialOffY);
		m_bTextCut = true;

	}
	clearCursor();
	if(m_iVisualDragMode == FV_VisualDrag_START_DRAGGING)
	{
	  reposOffsets(x,y);
	}
	m_iVisualDragMode = FV_VisualDrag_DRAGGING;
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
		m_pAutoScrollTimer = UT_Timer::static_constructor(_autoScroll, this);
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
	m_recOrigLeft.left += dx;
	m_recOrigLeft.top += dy;
	m_recOrigRight.left += dx;
	m_recOrigRight.top += dy;
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

	if(!m_bNotDraggingImage && (expX.width > 0))
	{
		getGraphics()->setClipRect(&expX);
		if(m_bSelectedRow)

		{
		      m_pView->setSelectionMode(FV_SelectionMode_NONE);
		}
		m_pView->updateScreen(false);
		if(m_bSelectedRow)
		{
		      m_pView->setSelectionMode(FV_SelectionMode_TableRow);
		}

	}
	if(!m_bNotDraggingImage && (expY.height > 0))
	{
	  xxx_UT_DEBUGMSG(("expY left %d top %d width %d height %d \n",expY.left,expY.top,expY.width,expY.height));
		getGraphics()->setClipRect(&expY);
		if(m_bSelectedRow)
		{
		      m_pView->setSelectionMode(FV_SelectionMode_NONE);
		}
		m_pView->updateScreen(false);
		if(m_bSelectedRow)
		{
		      m_pView->setSelectionMode(FV_SelectionMode_TableRow);
		}

	}
	if(!m_bNotDraggingImage && (expX.height > 0))
	{
	  xxx_UT_DEBUGMSG(("expY left %d top %d width %d height %d \n",expX.left,expX.top,expX.width,expX.height));
		getGraphics()->setClipRect(&expX);
		if(m_bSelectedRow)
		{
		      m_pView->setSelectionMode(FV_SelectionMode_NONE);
		}
		m_pView->updateScreen(false);
		if(m_bSelectedRow)
		{
		      m_pView->setSelectionMode(FV_SelectionMode_TableRow);
		}

	}
	if(!m_bNotDraggingImage)
	{
	        getGraphics()->setClipRect(NULL);
		drawImage();
		if(m_recOrigLeft.width > 0)
		{
		     getGraphics()->setClipRect(&m_recOrigLeft);
		     m_pView->updateScreen(false);
		}
		if(m_recOrigRight.width > 0)
		{
		     getGraphics()->setClipRect(&m_recOrigRight);
		     m_pView->updateScreen(false);
		}
	}
	m_iLastX = x;
	m_iLastY = y;
	getGraphics()->setClipRect(NULL);
	PT_DocPosition posAtXY = getPosFromXY(x,y);
	m_pView->_setPoint(posAtXY);
	xxx_UT_DEBUGMSG(("Point at visual drag set to %d \n",m_pView->getPoint()));
//	m_pView->_fixInsertionPointCoords();
	drawCursor(posAtXY);
}

/*!
 * This method is called at the commencement of a visual drag. If the offsets
 * to the caret are too big, this method will adjust them and shift the image
 * of the dragged text to a comfortable distance fromthe caret.
 * Returns true if the offsets are shifted.
 * UT_sint32 x pos of the caret
 * UT_sint32 y pos of  the caret 
 */
bool FV_VisualDragText::reposOffsets(UT_sint32 x, UT_sint32 y)
{
  UT_sint32 dx = 0;
  UT_sint32 dy = 0;
  bool bAdjustX = false;
  bool bAdjustY = false;
  UT_sint32 iext = getGraphics()->tlu(3);
  dx = x - m_recCurFrame.left - m_recOrigLeft.width;
  dy = y - m_recCurFrame.top;
  UT_DEBUGMSG((" repos dx = %d \n",dx));
  UT_DEBUGMSG((" repos dy = %d \n",dy));
  UT_Rect expX(0,m_recCurFrame.top,0,m_recCurFrame.height);
  UT_Rect expY(m_recCurFrame.left,0,m_recCurFrame.width,0);
  if(abs(dx) > getGraphics()->tlu(40))
  {
    bAdjustX = true;
    dx -= getGraphics()->tlu(20);
    m_iInitialOffX -= dx;
    expX.set(0,m_recCurFrame.top,0,m_recCurFrame.height);
    m_recCurFrame.left += dx;
    m_recOrigLeft.left += dx;
    m_recOrigRight.left += dx;
  }
  if(dy > getGraphics()->tlu(40))
  {
    bAdjustY = true;
    dy -= getGraphics()->tlu(20);
    m_iInitialOffY -= dy;
    expY.set(m_recCurFrame.left,0,m_recCurFrame.width,0);
    m_recCurFrame.top += dy;
    m_recOrigLeft.top += dy;
    m_recOrigRight.top += dy;
  }

  if(bAdjustX && dx < 0)
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
  else if(bAdjustX)
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
  if(bAdjustY && dy < 0)
  {
        expY.top = m_recCurFrame.top + m_recCurFrame.height -iext;
	expY.height = -dy + 2*iext;
  }
  else if(bAdjustY)
  {
        expY.top = m_recCurFrame.top - dy - iext;
	expY.height = dy + 2*iext;
  }

  if(bAdjustX && expX.width > 0)
  {
      getGraphics()->setClipRect(&expX);
      m_pView->updateScreen(false);
  }
  if(bAdjustY && (expY.height > 0))
  {
      getGraphics()->setClipRect(&expY);
      m_pView->updateScreen(false);
  }
  if(bAdjustX || bAdjustY)
  {
      getGraphics()->setClipRect(NULL);
      drawImage();
      if(m_recOrigLeft.width > 0)
      {
	  getGraphics()->setClipRect(&m_recOrigLeft);
	  m_pView->updateScreen(false);
      }
      if(m_recOrigRight.width > 0)
      {
	  getGraphics()->setClipRect(&m_recOrigRight);
	  m_pView->updateScreen(false);
      }
      return true;
  }
  return false;
}

void FV_VisualDragText::clearCursor(void)
{
	if(m_bCursorDrawn)
	{
		if(m_pDocUnderCursor)
		{
		        getGraphics()->allCarets()->disable(true);
		        m_pView->m_countDisable++;
			GR_Painter painter(getGraphics());
			painter.drawImage(m_pDocUnderCursor,m_recDoc.left,m_recCursor.top);
			m_bCursorDrawn = false;
			DELETEP(m_pDocUnderCursor);
		}
	}
}

void FV_VisualDragText::drawCursor(PT_DocPosition newPos)
{
        if(m_bCursorDrawn)
	    return;
	getGraphics()->allCarets()->disable(true);
	m_pView->m_countDisable++;
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
	m_recDoc.left = xLow - getGraphics()->tlu(1);
	m_recDoc.top = yLow - getGraphics()->tlu(1);
	m_recDoc.width =  getGraphics()->tlu(3);
	m_recDoc.height = heightCaret + getGraphics()->tlu(1);
	UT_ASSERT(m_pDocUnderCursor == NULL);
	GR_Painter painter(getGraphics());
	m_pDocUnderCursor = painter.genImageFromRectangle(m_recDoc);
	UT_RGBColor black(0,0,0);
	painter.fillRect( black, m_recCursor);
	m_bCursorDrawn = true;
}

/*!
 * This method creates an image from the current selection. It sets
 * the drag rectangle, the initial offsets and the initial positions 
 * of the cursor.
 */
void FV_VisualDragText::getImageFromSelection(UT_sint32 x, UT_sint32 y)
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
	fp_Page * pPageLow = NULL;
	fp_Page * pPageHigh = NULL;
	if(m_pView->getSelectionMode() < 	FV_SelectionMode_Multiple)
	{
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
	}
	else
	{
		UT_sint32 num = m_pView->getNumSelections();
		PD_DocumentRange * pR = m_pView->getNthSelection(0);
		posLow = pR->m_pos1+1;
		fl_BlockLayout * pBlock = NULL;

		bDirection =  false;
		bEOL = false;

		m_pView->_findPositionCoords(posLow, bEOL, xLow, yLow, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRunLow);
		while(pBlock->isEmbeddedType())
		{
			posLow++;
			m_pView->_findPositionCoords(posLow, bEOL, xLow, yLow, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRunLow);
		}
		fl_ContainerLayout * pCL = pBlock->myContainingLayout();
		UT_return_if_fail(pCL->getContainerType() == FL_CONTAINER_CELL);
		fp_CellContainer * pCCon = static_cast<fp_CellContainer *>(pCL->getFirstContainer());
		UT_return_if_fail(pCCon);
		UT_Rect * pRect = pCCon->getScreenRect();
		xLow = pRect->left;
		yLow = pRect->top;
		m_recCurFrame.left = xLow;
		m_recCurFrame.top = yLow;
		delete pRect;
//
// Now the other end of the column
//
	    pR = m_pView->getNthSelection(num-1);
		posHigh = pR->m_pos1+1;
		m_pView->_findPositionCoords(posHigh, bEOL, xHigh, yHigh, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRunLow);
		while(pBlock->isEmbeddedType())
		{
			posHigh++;
			m_pView->_findPositionCoords(posHigh, bEOL, xHigh, yHigh, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRunLow);
		}
		pCL = pBlock->myContainingLayout();
		UT_return_if_fail(pCL->getContainerType() == FL_CONTAINER_CELL);
		pCCon = static_cast<fp_CellContainer *>(pCL->getFirstContainer());
		UT_return_if_fail(pCCon);
		pRect = pCCon->getScreenRect();
		xHigh = pRect->left+ pRect->width;
		yHigh = pRect->top + pRect->height;
		delete pRect;
		m_recCurFrame.width = xHigh - xLow;
		m_recCurFrame.height = yHigh - yLow;
		m_recOrigLeft.width = 0;
		m_recOrigLeft.height = 0;
		m_recOrigLeft.left = 0;
		m_recOrigLeft.top = 0;
		m_recOrigRight.width = 0;
		m_recOrigRight.height = 0;
		m_recOrigRight.left = 0;
		m_recOrigRight.top = 0;
		m_iLastX = x;
		m_iLastY = y;
		m_iInitialOffX = x - m_recCurFrame.left;
		m_iInitialOffY = y - m_recCurFrame.top;
		GR_Painter painter(getGraphics());
		m_pDragImage = painter.genImageFromRectangle(m_recCurFrame);
		return;
	}
	fp_Run * pRunLow2 = NULL;
	m_pView->_findPositionCoords(posLow+1, bEOL, xLow, yLow, xCaret2, yCaret2, heightCaret, bDirection, NULL, &pRunLow2);
	UT_return_if_fail( pRunLow2 );
	fl_BlockLayout * pBLow2 = pRunLow2->getBlock();
	m_pView->_findPositionCoords(posLow, bEOL, xLow, yLow, xCaret2, yCaret2, heightCaret, bDirection, NULL, &pRunLow);
	UT_return_if_fail( pRunLow );
	fl_BlockLayout * pBLow1 = pRunLow->getBlock();
	bool bUseNext = false;
	bool bIsTable = false;
	if(m_pView->getDocument()->isTableAtPos(posLow))
	{
	    posLow += 2;
	    bIsTable = true;
	}
	fl_TableLayout * pTabLow = m_pView->getTableAtPos(posLow+1);
	fl_TableLayout * pTabHigh = m_pView->getTableAtPos(posHigh);
	if(bIsTable && (pTabLow != pTabHigh))
	{
	    posLow -= 2;
	}
	if(pBLow2 != pBLow1)
	{
		pRunLow = pRunLow2;
		bUseNext = true;
	}
	fp_Line * pLineLow = pRunLow->getLine();
	fp_Run * pRunHigh = NULL;
	m_pView->_findPositionCoords(posHigh, bEOL, xHigh, yHigh, xCaret2, yCaret2, heightCaret, bDirection, NULL, &pRunHigh);
	fp_Line * pLineHigh = pRunHigh->getLine();
	pPageLow = pLineLow->getPage();
	pPageHigh = pLineHigh->getPage();
	//
	// Decide if the selection is fully on screen and all on the same page.
	//
	bool bOffscreen = false;
	if(pPageLow != pPageHigh)
	{
	     bOffscreen = true;
	}
	if(!bOffscreen && ((yLow <0) || (yHigh >  m_pView->getWindowHeight())))
	{
	     bOffscreen = true;
	}
	if(!bOffscreen && ((xLow <0) || (xHigh <0) || (xLow >  m_pView->getWindowWidth()) ||(xLow >  m_pView->getWindowWidth())))
	{
	     bOffscreen = true;
	}
	if(bOffscreen)
	{
	     m_bNotDraggingImage = true;
	     m_recCurFrame.left = x - 1;
	     m_recCurFrame.top = y - 1;
	     m_recCurFrame.width = 2;
	     m_recCurFrame.height = 2;
	     m_iInitialOffX = 1;
	     m_iInitialOffY = 1;
	     m_iLastX = x;
	     m_iLastY = y;
	     m_recOrigLeft.width = 2;
	     m_recOrigLeft.height = 2;
	     m_recOrigLeft.left = x-1;
	     m_recOrigLeft.top = y-1;
	     GR_Graphics::Cursor cursor = GR_Graphics::GR_CURSOR_DRAGTEXT;
	     if(isDoingCopy())
	     {
	       cursor = GR_Graphics::GR_CURSOR_COPYTEXT;
	     }
	     getGraphics()->setCursor(cursor);
	     return;
	}
	m_bNotDraggingImage = false;
//
// OK deal with the nice case of the selection just on the single line
//
	bool bDoBroken = true;
	if(pLineLow == pLineHigh)
	{
//
// Grab that first charcter!
//
	        if(!bUseNext)
		{
		    m_pView->_findPositionCoords(posLow, bEOL, xLow, yLow, xCaret2, yCaret2, heightCaret, bDirection, NULL, &pRunLow2);
		}
		else
		{
		    m_pView->_findPositionCoords(posLow+1, bEOL, xLow, yLow, xCaret2, yCaret2, heightCaret, bDirection, NULL, &pRunLow2);
		}
		UT_sint32 xx,yy;
		pLineLow->getScreenOffsets(pRunLow,xx,yy);
		m_recCurFrame.left = xLow < xHigh ? xLow : xHigh;
		m_recCurFrame.width = xHigh > xLow ? xHigh - xLow : xLow - xHigh;
		m_recCurFrame.top = yy;
		m_recCurFrame.height = pLineLow->getHeight();
		m_recOrigLeft.width = 0;
		m_recOrigLeft.height = 0;
		m_recOrigLeft.left = 0;
		m_recOrigLeft.top = 0;
		m_recOrigRight.width = 0;
		m_recOrigRight.height = 0;
		m_recOrigRight.left = 0;
		m_recOrigRight.top = 0;
		bDoBroken = false;
	}
	if ( bDoBroken && (pTabLow != NULL) && (pTabHigh == pTabLow))
	{
	    //
	    // Look to see if we have a rectangular table selection
	    //
	    UT_sint32 nExtra = 1;
	    if(m_pView->getDocument()->isTableAtPos(posLow+nExtra))
	    {
	      nExtra++;
	    }
	    if(m_pView->getDocument()->isCellAtPos(posLow+nExtra))
	    {
	      nExtra++;
	    }
	    if(m_pView->getDocument()->isBlockAtPos(posLow+nExtra))
	    {
	      nExtra++;
	    }
	    fp_CellContainer * pCellConLow = m_pView->getCellAtPos(posLow+nExtra);
	    if(pCellConLow == NULL)
	      goto do_broken;
	    fl_CellLayout * pCellLow = static_cast<fl_CellLayout *>(pCellConLow->getSectionLayout());
	    if(m_pView->getDocument()->isEndTableAtPos(posHigh-1))
	    {
		posHigh--;
	    }
	    fp_CellContainer * pCellConHigh = m_pView->getCellAtPos(posHigh);
	    if(pCellConHigh == NULL)
	      goto do_broken;
	    fl_CellLayout * pCellHigh = static_cast<fl_CellLayout *>(pCellConHigh->getSectionLayout());
	    if((pCellLow->getPosition(true) >= (posLow -1)) &&
	       ((pCellHigh->getPosition(true) + pCellHigh->getLength()-1) <= (posHigh + 1) ))
	    {
	      UT_sint32 numCols = static_cast<fp_TableContainer *>(pCellConLow->getContainer())->getNumCols();
	      if(((pCellConLow->getLeftAttach() == 0) && 
		  (pCellConHigh->getRightAttach() == numCols)) || 
		 (pCellConLow->getTopAttach() == pCellConHigh->getTopAttach()))
	      {
		//
		// OK the low and high cells are fully selected.
		//
		  UT_DEBUGMSG(("Fully selected low and high positions \n"));
//
// Grab that first charcter!
//
	          if(!bUseNext)
		  {
		      m_pView->_findPositionCoords(posLow, bEOL, xLow, yLow, xCaret2, yCaret2, heightCaret, bDirection, NULL, &pRunLow2);
		  }
		  else
		  {
		      m_pView->_findPositionCoords(posLow+1, bEOL, xLow, yLow, xCaret2, yCaret2, heightCaret, bDirection, NULL, &pRunLow2);
		  }
		  if((pCellConLow->getLeftAttach() == 0) && 
		     (pCellConHigh->getRightAttach() == numCols))
		  {
		      m_bSelectedRow = true;
		  }
		  UT_Rect * pLow = pCellConLow->getScreenRect();
		  UT_Rect * pHigh = pCellConHigh->getScreenRect();
		  UT_return_if_fail(pLow);
		  UT_return_if_fail(pHigh);
		  m_recCurFrame.left = pLow->left;
		  m_recCurFrame.width = pHigh->left + pHigh->width - pLow->left;
		  m_recCurFrame.top = pLow->top;
		  m_recCurFrame.height = pHigh->top + pHigh->height - pLow->top;
		  DELETEP(pLow);
		  DELETEP(pHigh);
		  m_recOrigLeft.width = 0;
		  m_recOrigLeft.height = 0;
		  m_recOrigLeft.left = 0;
		  m_recOrigLeft.top = 0;
		  m_recOrigRight.width = 0;
		  m_recOrigRight.height = 0;
		  m_recOrigRight.left = 0;
		  m_recOrigRight.top = 0;
		  bDoBroken = false;
	      }
	    }
	}
 do_broken:	if(bDoBroken)
	{
//
// low and high are on different rows. First get top, left
//
//
		UT_sint32 xx,yy;
		fp_Run * pRun = pLineLow->getFirstRun();
		pLineLow->getScreenOffsets(pRun,xx,yy);
		xx -= pRun->getX();
		xx -= pLineLow->getX();
		m_recOrigLeft.left = xx < xLow ? xx : xLow;
		m_recOrigLeft.width = xLow > xx ? xLow - xx : xx - xLow;
		m_recOrigLeft.top = yy;
		m_recOrigLeft.height = pLineLow->getHeight();
		m_recCurFrame.left = xx < xLow ? xx : xLow;
		m_recCurFrame.top = yy;
		fp_Line * pNext = pLineLow;
		UT_sint32 width = 0;
		while(pNext && (pNext != pLineHigh))
		{
			pRun = pNext->getFirstRun();
			pNext->getScreenOffsets(pRun,xx,yy);
			xx += pNext->getMaxWidth();
			if(xx > width)
			{
				width = xx;
			}
			fp_ContainerObject * pCon = pNext->getNext();
			if(pCon)
			{
				pNext = static_cast<fp_Line *>(pCon);
			}
			else
			{
				fl_BlockLayout * pBL = pNext->getBlock();
				pBL = pBL->getNextBlockInDocument();
				if(pBL)
				{
					pNext = static_cast<fp_Line *>(pBL->getFirstContainer());
				}
				else
				{ 
					pNext = NULL;
				}
			}
		}
		if(pNext == NULL)
		{
			UT_DEBUGMSG(("Last line of selection not found! \n"));
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return;
		}
		pRun = pLineHigh->getFirstRun();
		pLineHigh->getScreenOffsets(pRun,xx,yy);
		yy += pLineHigh->getHeight();
		m_recCurFrame.width = width > m_recCurFrame.left ? width - m_recCurFrame.left : m_recCurFrame.left - width;
		m_recCurFrame.height = yy - m_recCurFrame.top;
		if(m_recCurFrame.top + m_recCurFrame.height > m_pView->getWindowHeight())
		{
			m_recCurFrame.height = m_pView->getWindowHeight() - m_recCurFrame.top;
		}
		fl_DocSectionLayout * pDSL = pRun->getBlock()->getDocSectionLayout();
		if(pDSL == NULL)
		{
			UT_DEBUGMSG(("No DocSectionLayout \n"));
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return;
		}
			
		if(m_recCurFrame.width > pDSL->getActualColumnWidth())
		{
			m_recCurFrame.width = pDSL->getActualColumnWidth();
		}
		m_recOrigRight.left = xHigh > xLow ? xHigh : xLow;
		m_recOrigRight.width = m_recCurFrame.left + m_recCurFrame.width > xHigh ?
			m_recCurFrame.left + m_recCurFrame.width - xHigh : xHigh - (m_recCurFrame.left + m_recCurFrame.width);
		m_recOrigRight.top = yy - pLineHigh->getHeight();
		m_recOrigRight.height = pLineHigh->getHeight();

	}
	m_iLastX = x;
	m_iLastY = y;
	m_iInitialOffX = x - m_recCurFrame.left;
	m_iInitialOffY = y - m_recCurFrame.top;
	GR_Painter painter(getGraphics());
	UT_RGBColor black(0,0,0);
	UT_RGBColor trans(0,0,0,true);
	m_pDragImage = painter.genImageFromRectangle(m_recCurFrame);
}

void FV_VisualDragText::mouseCut(UT_sint32 x, UT_sint32 y)
{
	getImageFromSelection(x,y);
	bool bPasteTableCol = (m_pView->getSelectionMode() == FV_SelectionMode_TableColumn);

	// flag up on the document level that we are dragging with the mouse
	// (in revisions mode the PT needs to be able to make a distinction between normal
	// cut/delete and the mouse cut)
	m_pView->getDocument()->setVDNDinProgress(true);

	FV_ViewDoubleBuffering dblBuffObj(m_pView, true, true);
	dblBuffObj.beginDoubleBuffering();
	
	if(bPasteTableCol)
	{
		m_pView->cmdCut();
	}
	else
	{
		PT_DocPosition pos1 = m_pView->getSelectionAnchor();
		PT_DocPosition pos2 = m_pView->getPoint();
		if(pos1 > pos2)
		{
			pos2 = m_pView->getSelectionAnchor();
			pos1 = m_pView->getPoint();
		}
		if(m_bSelectedRow)
		{
		    m_pView->copyToLocal(pos1,pos2);
		    m_pView->cmdDeleteRow(pos1+2);
		    m_pView->setSelectionMode(FV_SelectionMode_TableRow);
		}
		else
		{
		    m_pView->copyToLocal(pos1,pos2);
		    m_pView->cmdCharDelete(true,1);
		}
	}
	m_pView->getDocument()->setVDNDinProgress(false);
	
	m_pView->updateScreen(false);

	dblBuffObj.endDoubleBuffering();

	drawImage();
}


void FV_VisualDragText::mouseCopy(UT_sint32 x, UT_sint32 y)
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
	m_iVisualDragMode= FV_VisualDrag_START_DRAGGING;
	m_bTextCut = false;
	m_bDoingCopy = true;
	m_pView->_resetSelection();
}

PT_DocPosition FV_VisualDragText::getPosFromXY(UT_sint32 x, UT_sint32 y)
{
//
// Convert this to a document position and paste!
//
	x -= m_iInitialOffX;
	y -= m_iInitialOffY;
	y += getGraphics()->tlu(6); //Otherwise it's too easy to hit the line above
	x += m_recOrigLeft.width; // Add in offset 
	PT_DocPosition posAtXY = m_pView->getDocPositionFromXY(x,y,false);
	xxx_UT_DEBUGMSG(("Point at x %d y %d is %d \n",x,y,posAtXY));
	return posAtXY;
}

/*!
 * This method oborts the current visual drag.
 */
void   FV_VisualDragText::abortDrag(void)
{
        if(m_pAutoScrollTimer != NULL)
	{
		m_pAutoScrollTimer->stop();
		DELETEP(m_pAutoScrollTimer);
	}
	m_bSelectedRow = false;
	bool bDidCopy = m_bDoingCopy;
	m_bDoingCopy = false;
	m_bNotDraggingImage = false;
	clearCursor();
	UT_DEBUGMSG(("Abort Drag! \n"));
	if(m_iVisualDragMode != FV_VisualDrag_DRAGGING)
	{
//
// we didn't actually drag anything. Just release the selection.
//
	        setMode(FV_VisualDrag_NOT_ACTIVE);
		return;
	}
	getGraphics()->setClipRect(&m_recCurFrame);
	m_pView->updateScreen(false);
	getGraphics()->setClipRect(NULL);
	setMode(FV_VisualDrag_NOT_ACTIVE);
	if(!bDidCopy)
	{
	  m_pView->cmdUndo(1);
	}
	return;
}
/*!
 * x and y is the location in the document windows of the mouse in logical
 * units.
 */
void FV_VisualDragText::mouseRelease(UT_sint32 x, UT_sint32 y)
{
        if(m_pAutoScrollTimer != NULL)
	{
		m_pAutoScrollTimer->stop();
		DELETEP(m_pAutoScrollTimer);
	}
	m_bDoingCopy = false;
	m_bNotDraggingImage = false;
	m_bSelectedRow = false;
	clearCursor();
	if(m_iVisualDragMode != FV_VisualDrag_DRAGGING)
	{
//
// we didn't actually drag anything. Just release the selection.
//
		m_pView->warpInsPtToXY(x, y,true);
		return;
	}
	
	FV_ViewDoubleBuffering dblBuffObj(m_pView, true, true);
	dblBuffObj.beginDoubleBuffering();

	PT_DocPosition posAtXY = getPosFromXY(x,y);
	m_pView->setPoint(posAtXY);
	fl_BlockLayout * pCurB = m_pView->getCurrentBlock();
	if(pCurB)
	{
	    fl_ContainerLayout * pCL = pCurB->myContainingLayout();
	    if(pCL && pCL->getContainerType() == FL_CONTAINER_SHADOW)
	    {
	         m_pView->setHdrFtrEdit(static_cast<fl_HdrFtrShadow *>(pCL));
	    }
	}
	getGraphics()->setClipRect(&m_recCurFrame);
	m_pView->updateScreen(false);
	getGraphics()->setClipRect(NULL);
	m_iVisualDragMode = FV_VisualDrag_NOT_ACTIVE;
	m_pView->getMouseContext(x,y);
	m_iInitialOffX = 0;
	m_iInitialOffY = 0;
	PT_DocPosition oldPoint = m_pView->getPoint();
	if(oldPoint < 2)
	{
	  oldPoint = 2;
	}
	bool bInFrame = m_pView->isInFrame(oldPoint);

	bool bPasteTableCol = (m_pView->getPrevSelectionMode() == FV_SelectionMode_TableColumn);
	if(!bPasteTableCol)
	  {
	    m_pView->pasteFromLocalTo(m_pView->getPoint());
	  }
	else
	  {
	    m_pView->cmdPaste();
	  }

	dblBuffObj.endDoubleBuffering();
	
	m_bSelectedRow = false;
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
	    if(!bPasteTableCol)
	      {
		m_pView->cmdSelect(oldPoint,newPoint);
	      }
	    else
	      {
		m_pView->cmdSelectColumn(newPoint);
	      }
	  }
	m_bTextCut = false;
}

void FV_VisualDragText::drawImage(void)
{
        if(m_bNotDraggingImage)
	{ 
	  GR_Graphics::Cursor cursor = GR_Graphics::GR_CURSOR_DRAGTEXT;
	  if(isDoingCopy())
	  {
	      cursor = GR_Graphics::GR_CURSOR_COPYTEXT;
	  }
	  getGraphics()->setCursor(cursor);
	  return;
	}
	if(m_pDragImage == NULL)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	GR_Painter painter(getGraphics());
	if( m_recOrigLeft.width > 0 || m_recOrigRight.width>0 )
	{
		UT_Rect dest;
		dest.left = m_recCurFrame.left +  m_recOrigLeft.width;
		dest.top = m_recCurFrame.top;
		dest.width = m_recCurFrame.width -  m_recOrigLeft.width;
		dest.height = m_recOrigLeft.height;
		UT_Rect src;
		src.left = m_recOrigLeft.width;
		src.top = 0;
		src.height =  m_recOrigLeft.height;
		src.width = dest.width;
		if((src.height > getGraphics()->tlu(2)) && (src.width >getGraphics()->tlu(2)) )
		{
			painter.fillRect(m_pDragImage,&src,&dest);
		}
		dest.left = m_recCurFrame.left;
		dest.top = m_recCurFrame.top + m_recOrigLeft.height ;
		dest.width = m_recCurFrame.width;
		dest.height = m_recCurFrame.height - m_recOrigLeft.height - m_recOrigRight.height;
		src.left = 0;
		src.top  = m_recOrigLeft.height ;
		src.width = m_recCurFrame.width;
		src.height = dest.height;
		if(src.height > getGraphics()->tlu(2) && src.width >getGraphics()->tlu(2) )
		{
			painter.fillRect(m_pDragImage,&src,&dest);
		}
		dest.left = m_recCurFrame.left;
		dest.top = m_recCurFrame.top + m_recCurFrame.height - m_recOrigRight.height;
		dest.width = m_recCurFrame.width - m_recOrigRight.width;
		dest.height = m_recOrigRight.height;
		src.top = m_recCurFrame.height - m_recOrigRight.height;
		src.left = 0;
		src.width = m_recCurFrame.width - m_recOrigRight.width;
		src.height = m_recOrigRight.height;
		if((src.height > getGraphics()->tlu(2)) && (src.width >getGraphics()->tlu(2)) )
		{
			painter.fillRect(m_pDragImage,&src,&dest);
		}
		return;
	}
	painter.drawImage(m_pDragImage,m_recCurFrame.left,m_recCurFrame.top);
}
