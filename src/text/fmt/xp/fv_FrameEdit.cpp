/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (c) 2003 Martin Sevior <msevior@physics.unimelb.edu.au>
 * Copyright (c) 2016 Hubert Figuière
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
#include "ut_debugmsg.h"
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
#include "fv_ViewDoubleBuffering.h"

FV_FrameEdit::FV_FrameEdit (FV_View * pView)
	: FV_Base (pView), 
	  m_iFrameEditMode(FV_FrameEdit_NOT_ACTIVE),
	  m_pFrameLayout(NULL),
	  m_pFrameContainer(NULL),
	  m_iLastX(0),
	  m_iLastY(0),
	  m_iInitialDragX(0),
	  m_iInitialDragY(0),
	  m_bInitialClick(false),
	  m_pFrameImage(NULL),
	  m_pAutoScrollTimer(NULL),
	  m_iInitialFrameX(0),
	  m_iInitialFrameY(0),
	  m_sRelWidth(""),
	  m_sMinHeight(""),
	  m_sExpandHeight("")
{
	UT_ASSERT_HARMLESS(pView);
}

FV_FrameEdit::~FV_FrameEdit()
{
	DELETEP(m_pFrameImage);
	if(m_pAutoScrollTimer)
	{
		m_pAutoScrollTimer->stop();
		DELETEP(m_pAutoScrollTimer);
	}
}

void FV_FrameEdit::setPointInside(void)
{
  fl_FrameLayout * pFL = getFrameLayout();
  if(pFL == NULL)
  {
    return;
  }
  PT_DocPosition pos = pFL->getPosition(true) + pFL->getLength()-1;
  setMode(FV_FrameEdit_NOT_ACTIVE);
  m_pView->_setPoint(pos);
}

bool FV_FrameEdit::isActive(void) const
{
	return (FV_FrameEdit_NOT_ACTIVE != m_iFrameEditMode);
}

void FV_FrameEdit::setMode(FV_FrameEditMode iEditMode)
{
    UT_DEBUGMSG(("Frame Edit mode set to %d \n",iEditMode));
	if(iEditMode == FV_FrameEdit_NOT_ACTIVE)
	{
		m_pFrameLayout = NULL;
		m_pFrameContainer = NULL;
		DELETEP(m_pFrameImage);
		m_recCurFrame.width = 0;
		m_recCurFrame.height = 0;
		setDragWhat( FV_DragNothing );
		m_iLastX = 0;
		m_iLastY = 0;
	}
	m_iFrameEditMode = iEditMode;
	if(getGraphics()  && iEditMode !=  FV_FrameEdit_NOT_ACTIVE)
	{
	        getGraphics()->allCarets()->disable();
		m_pView->m_countDisable++;
	}
}


static bool bScrollRunning = false;
static UT_Worker * s_pScroll = NULL;
static UT_sint32 iExtra = 0;

void FV_FrameEdit::_actuallyScroll(UT_Worker * pWorker)
{
	// this is a static callback method and does not have a 'this' pointer.

	FV_FrameEdit * pFE = static_cast<FV_FrameEdit *>(pWorker->getInstanceData());
	UT_return_if_fail(pFE);
	if(pFE->getFrameEditMode() != FV_FrameEdit_DRAG_EXISTING)
	{
		if(pFE->m_pAutoScrollTimer)
			pFE->m_pAutoScrollTimer->stop();
		DELETEP(pFE->m_pAutoScrollTimer);
		iExtra = 0;
		s_pScroll->stop();
		delete s_pScroll;
		s_pScroll = NULL;
		bScrollRunning = false;
		return;
	}
	FV_View * pView = pFE->m_pView;
	UT_sint32 x = pFE->m_xLastMouse;
	UT_sint32 y = pFE->m_yLastMouse;
	bool bScrollDown = false;
	bool bScrollUp = false;
	bool bScrollLeft = false;
	bool bScrollRight = false;
	bool bStop = false;
	if(y<=0)
	{
	  if(pView->getYScrollOffset() <= 10)
	  {
	      pView->setYScrollOffset(0);
	      pView->updateScreen(false);
	      bStop = true;
	  }
	  else
	  {
	      bScrollUp = true;
	  }
	}
	else if( y >= pView->getWindowHeight())
	{
	  if((pView->getYScrollOffset()+pView->getWindowHeight()+10) >= pView->getLayout()->getHeight())
	  {
	      pView->setYScrollOffset(pView->getLayout()->getHeight() - pView->getWindowHeight());
	      pView->updateScreen(false);
	      bStop = true;
	      UT_DEBUGMSG(("!!!!!!!!!!!!PLLLLLLLLLEEEEAAAASSSEEEEE STOOPPP!!!! \n"));
	  }
	  else
	  {
		bScrollDown = true;
	  }
	}
	if(x <= 0)
	{
		bScrollLeft = true;
	}
	else if(x >= pView->getWindowWidth())
	{
		bScrollRight = true;
	}
	if(!bStop && (bScrollDown || bScrollUp || bScrollLeft || bScrollRight))
	{
		pFE->getGraphics()->setClipRect(&pFE->m_recCurFrame);
		pView->updateScreen(false);
		pFE->getGraphics()->setClipRect(NULL);
		UT_sint32 minScroll = pFE->getGraphics()->tlu(20);
		if(bScrollUp)
		{
		        UT_sint32 yscroll = abs(y);
			if(yscroll < minScroll)
			    yscroll = minScroll;
			pView->cmdScroll(AV_SCROLLCMD_LINEUP, static_cast<UT_uint32>( yscroll +iExtra));
		}
		else if(bScrollDown)
		{
		        UT_sint32 yscroll = y - pView->getWindowHeight();
			if(yscroll < minScroll)
			    yscroll = minScroll;
			pView->cmdScroll(AV_SCROLLCMD_LINEDOWN, static_cast<UT_uint32>(yscroll+iExtra));
		}
		if(bScrollLeft)
		{
			pView->cmdScroll(AV_SCROLLCMD_LINELEFT, static_cast<UT_uint32>(-x));
		}
		else if(bScrollRight)
		{
			pView->cmdScroll(AV_SCROLLCMD_LINERIGHT, static_cast<UT_uint32>(x -pView->getWindowWidth()));
		}
		pFE->drawFrame(true);
		iExtra = 0;
		return;
	}
	else
	{
		if(pFE->m_pAutoScrollTimer)
			pFE->m_pAutoScrollTimer->stop();
		DELETEP(pFE->m_pAutoScrollTimer);

	}
	iExtra = 0;
	s_pScroll->stop();
	delete s_pScroll;
	s_pScroll = NULL;
	bScrollRunning = false;
}

void FV_FrameEdit::_autoScroll(UT_Worker * pWorker)
{
	UT_return_if_fail(pWorker);
	// this is a static callback method and does not have a 'this' pointer.

	FV_FrameEdit * pFE = static_cast<FV_FrameEdit *>(pWorker->getInstanceData());
	UT_return_if_fail(pFE);
	if(bScrollRunning)
	{
	    if(iExtra < pFE->getGraphics()->tlu(600))
	      iExtra += pFE->getGraphics()->tlu(20);
	    UT_DEBUGMSG(("Dropping FrameEditautoscroll !!!!!!! \n"));
	    return;
	}

	UT_DEBUGMSG(("_autoscroll started!! \n"));
	int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	//	int inMode = UT_WorkerFactory::TIMER;
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	s_pScroll = UT_WorkerFactory::static_constructor (_actuallyScroll,pFE, inMode, outMode);

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

/*!
 * Returns 0 if the frame has never been dragged at all.
 * Returns 1 if the frame has been dragged a little bit
 * Return 10 if the frame has been dragged a lot.
 */
UT_sint32 FV_FrameEdit::haveDragged(void) const
{
	if(!m_bFirstDragDone)
	{
		return 0;
	}
	if((abs(m_xLastMouse - m_iFirstEverX) + abs(m_yLastMouse - m_iFirstEverY)) <
		getGraphics()->tlu(3))
	{
	  UT_DEBUGMSG(("Not dragged enough - return 1 \n"));
		return 1;
	}
	return 10;
}

void FV_FrameEdit::_mouseDrag(UT_sint32 x, UT_sint32 y)
{
	FV_ViewDoubleBuffering dblBuffObj(m_pView, false, false);
	dblBuffObj.beginDoubleBuffering();
	
	UT_sint32 dx = 0;
	UT_sint32 dy = 0;
	UT_Rect expX(0,m_recCurFrame.top,0,m_recCurFrame.height);
	UT_Rect expY(m_recCurFrame.left,0,m_recCurFrame.width,0);
	
	FV_Base::_doMouseDrag( x, y, dx, dy, expX, expY );
	if (getDragWhat()==FV_DragWhole) 
	{
		UT_sint32 diffx = 0;
		UT_sint32 diffy = 0;
		UT_sint32 iext = getGraphics()->tlu(3);
		
		bool bScrollDown = false;
		bool bScrollUp = false;
		bool bScrollLeft = false;
		bool bScrollRight = false;
		if(y<=0)
		{
		  if(m_pView->getYScrollOffset() <= 0)
		    {
		      m_pView->setYScrollOffset(0);
		      m_pView->updateScreen(false);
		      if(m_pAutoScrollTimer)
			m_pAutoScrollTimer->stop();
		      DELETEP(m_pAutoScrollTimer);
		    }
		  else
		    {
		      bScrollUp = true;
		    }
		}
		else if( y >= m_pView->getWindowHeight())
		{
		  if(m_pView->getYScrollOffset() >= m_pView->getLayout()->getHeight())
		  {
		      m_pView->setYScrollOffset(m_pView->getLayout()->getHeight());
		      m_pView->updateScreen(false);
		      if(m_pAutoScrollTimer)
			m_pAutoScrollTimer->stop();
		      DELETEP(m_pAutoScrollTimer);
		  }
		  else
		  {
		      bScrollDown = true;
		  }
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
		diffx = m_iLastX - x;
		diffy = m_iLastY - y;
		dx = -diffx;
		dy = - diffy;
		m_recCurFrame.left -= diffx;
		m_recCurFrame.top -= diffy;
		UT_DEBUGMSG(("Doing drag whole frame left %d top %d \n",m_recCurFrame.left, m_recCurFrame.top));
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
	_checkDimensions();

	if(FV_FrameEdit_RESIZE_INSERT == m_iFrameEditMode)
	{
		xxx_UT_DEBUGMSG(("width after drag %d \n",m_recCurFrame.width));
	}
	else 
	{
		if (FV_FrameEdit_RESIZE_EXISTING == m_iFrameEditMode)
		{
			UT_sint32 iW = m_recCurFrame.width;
			UT_sint32 iH = m_recCurFrame.height;
			m_pFrameLayout->localCollapse();
			m_pFrameLayout->setFrameWidth(iW);
			m_pFrameLayout->setFrameHeight(iH);
			m_pFrameContainer->_setWidth(iW);
			m_pFrameContainer->_setHeight(iH);
			m_pFrameLayout->miniFormat();
			m_pFrameLayout->getDocSectionLayout()->setNeedsSectionBreak(false,NULL);
		}

		if (FV_FrameEdit_RESIZE_EXISTING == m_iFrameEditMode || FV_FrameEdit_DRAG_EXISTING == m_iFrameEditMode)
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
	fp_FrameContainer * pFCon = NULL;
	fl_FrameLayout * pFL = NULL;
  	fl_BlockLayout * pBL = NULL;
	if(getDoc()->isFrameAtPos(posAtXY))
	{
		fl_ContainerLayout* psfh = NULL;
		getDoc()->getStruxOfTypeFromPosition(m_pView->getLayout()->getLID(),posAtXY+1,
										   PTX_SectionFrame, &psfh);
		pFL = static_cast<fl_FrameLayout *>(psfh);
		UT_ASSERT(pFL->getContainerType() == FL_CONTAINER_FRAME);
		pFCon = static_cast<fp_FrameContainer *>(pFL->getFirstContainer());
		UT_ASSERT(pFCon->getContainerType() == FP_CONTAINER_FRAME);
	}
	else
	{
		pBL = m_pView->_findBlockAtPosition(posAtXY);
		UT_return_if_fail(pBL);
	}
	if(!isActive() && (pFCon == NULL))
	{
		m_iFrameEditMode = 	FV_FrameEdit_EXISTING_SELECTED;
		if(getGraphics())
		{
		      getGraphics()->allCarets()->disable();
		      m_pView->m_countDisable++;
		}
		fl_ContainerLayout * pCL = pBL->myContainingLayout();
		while(pCL && (pCL->getContainerType() != FL_CONTAINER_FRAME) && (pCL->getContainerType() != FL_CONTAINER_DOCSECTION))
		{
			pCL = pCL->myContainingLayout();
		}
		UT_return_if_fail(pCL);
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
		setDragWhat( FV_DragWhole );
		return;	
	}
	if(!isActive())
	{
		m_iFrameEditMode = 	FV_FrameEdit_EXISTING_SELECTED;
		if(getGraphics())
		{
		      getGraphics()->allCarets()->disable();
		      m_pView->m_countDisable++;
		}
		m_pFrameLayout = pFL;
		UT_ASSERT(m_pFrameLayout->getContainerType() == FL_CONTAINER_FRAME);
		m_pFrameContainer = pFCon;
		UT_ASSERT(m_pFrameContainer);
		if(bDrawFrame)
		{
			drawFrame(true);
		}
		m_iLastX = x;
		m_iLastY = y;
		setDragWhat( FV_DragWhole );
		return;
	}
	//
	// OK find the coordinates of the frame.
	//
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
		if(pBL)
		{
			pFL = static_cast<fl_FrameLayout *>(pBL->myContainingLayout());
			pFCon = static_cast<fp_FrameContainer *>(pFL->getFirstContainer());
		}
		else
		{
			UT_ASSERT(pFL);
			UT_ASSERT(pFCon);
		}
	}
	UT_return_if_fail(pFCon);
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
		setDragWhat( FV_DragTopLeftCorner );
	}
//
// top Right
//
	else if((iRight - ires < x) && (x < iRight) && (iTop < y) && (y < iTop + ires))
	{
		setDragWhat( FV_DragTopRightCorner );
	}
//
// bot left
//
	else if((iLeft < x) && (x < iLeft + ires) && (iBot > y) && (y > iBot - ires))
	{
		setDragWhat( FV_DragBotLeftCorner );
	}
//
// Bot Right
//
	else if((iRight - ires < x) && (x < iRight) && (iBot > y) && (y > iBot - ires))
	{
		setDragWhat( FV_DragBotRightCorner );
	}
//
// top Edge
//
	else if( bX && bTop)
	{
		setDragWhat( FV_DragTopEdge );
	}
//
// left Edge
//
	else if(bLeft && bY)
	{
		setDragWhat( FV_DragLeftEdge );
	}
//
// right Edge
//
	else if(bRight && bY)
	{
		setDragWhat( FV_DragRightEdge );
	}
//
// bot Edge
//
	else if(bBot && bX)
	{
		setDragWhat( FV_DragBotEdge );
	}
	else
	{
		if( bX && bY)
		{
			setDragWhat( FV_DragWhole );
			xxx_UT_DEBUGMSG(("Dragging Whole Frame \n"));
		}
		else
		{
			setDragWhat( FV_DragNothing );
			return;
		}
	}
	if(bDrawFrame && (m_recCurFrame.width > 0) && (m_recCurFrame.height >0))
	{
		drawFrame(true);
	}
	const PP_AttrProp * pAP = NULL;
	pFL->getAP(pAP);
	const char * pszPercentWidth = NULL;
	const char * pszMinHeight = NULL;
	const char * pszExpandHeight = NULL;
	if(pAP && pAP->getProperty("frame-rel-width",pszPercentWidth))
	{
		if(pszPercentWidth)
		{
		     m_sRelWidth = pszPercentWidth;
		}
	}
	if(pAP && pAP->getProperty("frame-min-height",pszMinHeight))
	{
		if(pszMinHeight)
		{
		     m_sMinHeight = pszMinHeight;
		}
	}
	if(pAP && pAP->getProperty("frame-expand-height",pszExpandHeight))
	{
	        m_sExpandHeight = pszExpandHeight;
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
	xxx_UT_DEBUGMSG((" Dragging What %d \n",getDragWhat()));
	m_pView->setCursorToContext();
	if(getGraphics())
	{
	    getGraphics()->allCarets()->disable();
	    m_pView->m_countDisable++;
	}
}

/*!
 * This method responds to a mouse click and decides what to do with it.
 */
void FV_FrameEdit::mouseLeftPress(UT_sint32 x, UT_sint32 y)
{
	m_bFirstDragDone = false;
	UT_DEBUGMSG(("Mouse Left Press \n"));
	m_pView->_clearSelection();
	if(!isActive())
	{
		setDragType(x,y,true);
		UT_DEBUGMSG(("Was not active now %d FrameLayout %p \n",getFrameEditMode(),getFrameLayout()));
		return;
	}
//
// Find the type of drag we should do.
//
	if(FV_FrameEdit_EXISTING_SELECTED == m_iFrameEditMode )
	{
		setDragType(x,y,true);
		UT_DEBUGMSG(("Was Existing Selected now %d \n",getFrameEditMode()));
		if(FV_DragNothing == getDragWhat())
		{
			m_bFirstDragDone = false;
			m_iFrameEditMode = FV_FrameEdit_NOT_ACTIVE;
			drawFrame(false);
			if(m_pFrameContainer && m_pFrameLayout)
			{
			  if(m_pFrameLayout->getFrameType() > FL_FRAME_TEXTBOX_TYPE)
			  {
			    if(m_pFrameContainer->isTightWrapped())
			    {
			      //			      m_pFrameContainer->clearScreen();
			      m_pView->updateScreen(false);
			    }
			  }
			} 
			m_pFrameLayout = NULL;
			m_pFrameContainer = NULL;
			DELETEP(m_pFrameImage);
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
			setDragWhat( FV_DragNothing );
			m_iLastX = 0;
			m_iLastY = 0;
			while(m_iGlobCount > 0)
				_endGlob();
			m_pView->warpInsPtToXY(x,y,true);
		}
		else
		{
			if( getDragWhat() != FV_DragWhole)
			{
				m_iFrameEditMode = FV_FrameEdit_RESIZE_EXISTING;
			}
			else
			{
				m_iFrameEditMode = FV_FrameEdit_DRAG_EXISTING;
				m_iInitialDragX = m_recCurFrame.left;
				m_iInitialDragY = m_recCurFrame.top;
				m_iInitialFrameX = m_pFrameContainer->getFullX();
				m_iInitialFrameY = m_pFrameContainer->getFullY();
			}
			if(getGraphics())
			{
			        getGraphics()->allCarets()->disable();
			        m_pView->m_countDisable++;
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
		UT_sint32 iCursorOff = getGraphics()->tlu(8);
		UT_sint32 origX = x + iCursorOff;
		UT_sint32 origY = y + iCursorOff;
		UT_sint32 iSize = getGraphics()->tlu(32);
		m_recCurFrame.left = origX - iSize;
		m_recCurFrame.top = origY - iSize;
		m_recCurFrame.width = iSize;
		m_recCurFrame.height = iSize;
		m_iFrameEditMode = FV_FrameEdit_RESIZE_INSERT;
		_beginGlob();
//		mouseRelease(x,y);
		mouseRelease(origX,origY);
		m_iFrameEditMode = FV_FrameEdit_RESIZE_EXISTING;
		m_iLastX = x;
		m_iLastY = y;
		m_iInitialDragX = m_recCurFrame.left;
		m_iInitialDragY = m_recCurFrame.top;
		setDragWhat( FV_DragBotRightCorner );
		m_bFirstDragDone = false;
		m_bInitialClick = true;
		if(getGraphics())
		{
		      getGraphics()->allCarets()->disable();
		      m_pView->m_countDisable++;
		}
		getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGESIZE_SE);
	}
}

bool FV_FrameEdit::isImageWrapper(void) const
{
        if(m_pFrameLayout == NULL)
	{
	        return false;
	}
	if(m_pFrameLayout->getFrameType() == FL_FRAME_WRAPPER_IMAGE)
	{
	        return true;
	}
	return false;
}

bool FV_FrameEdit::getFrameStrings(UT_sint32 x, UT_sint32 y,
				   fv_FrameStrings & FrameStrings,
				   fl_BlockLayout ** pCloseBL,
				   fp_Page ** ppPage)
{
//
// Find the block that contains (x,y). We'll insert the frame after
// this block in PT and position it on the page relative to this block.
//
//
// X and y are the (x,y) coords of the frame on the screen.
//
                PT_DocPosition posAtXY = 0;
		posAtXY = m_pView->getDocPositionFromXY(x,y,true);
		fl_BlockLayout * pBL = NULL;
		fp_Run * pRun = NULL;
		fp_Line * pLine = NULL;
		UT_sint32 x1,x2,y1,y2;
		UT_uint32 height;
		bool bEOL=false;
		bool bDir=false;
		m_pView->_findPositionCoords(posAtXY,bEOL,x1,y1,x2,y2,height,bDir,&pBL,&pRun);
		xxx_UT_DEBUGMSG((" Requested y %d frameEdit y1= %d y2= %d \n",y,y1,y2));
		fp_Run * pRunOrig = pRun;
		if((pBL == NULL) || (pRun == NULL))
		{
			return false;
		}
		fl_BlockLayout * pPrevBL = pBL;
		while(pBL && pBL->myContainingLayout() &&
		((pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_ENDNOTE) ||
		(pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_FOOTNOTE) ||
		(pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_TOC) ||
		(pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_FRAME) ||
		(pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_CELL) ||
		(pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_SHADOW) ||
		(pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_HDRFTR)))
		{
		    UT_DEBUGMSG(("Skipping Block %p \n",pBL));
		    pPrevBL = pBL;
		    pBL = pBL->getPrevBlockInDocument();
		}
		if(pBL == NULL)
		{
		     pBL = pPrevBL;
		}
		pLine = pRun->getLine();
		if(pLine == NULL)
		{
			return false;
		}
		UT_ASSERT(pBL->myContainingLayout() && (pBL->myContainingLayout()->getContainerType() != FL_CONTAINER_HDRFTR) 
			  && (pBL->myContainingLayout()->getContainerType() != FL_CONTAINER_SHADOW));
		*pCloseBL = pBL;
		posAtXY = pBL->getPosition();

		// don't let widths and heights be too big

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
//
// Need this for offset to column
//
		UT_return_val_if_fail(pBL->getFirstRun(),false);
		UT_return_val_if_fail(pBL->getFirstRun()->getLine(),false);
		UT_return_val_if_fail(pBL->getFirstRun()->getLine()->getColumn(),false);
		fp_Container * pCol = pRunOrig->getLine()->getColumn();
		UT_ASSERT(pCol->getContainerType() == FP_CONTAINER_COLUMN);
//
// Find the screen coords of pCol and substract then from x,y
//
		UT_sint32 iColx = 0;
		UT_sint32 iColy = 0;
		fp_Page * pPage = pCol->getPage();
		if(!pPage)
		{
		    return false;
		}
		else
		{
		    pPage->getScreenOffsets(pCol,iColx,iColy);
		}
		UT_sint32 xp=0;
		UT_sint32 yp= 0;
		m_pView->getPageScreenOffsets(pPage,xp,yp);
		UT_sint32 finalColx = x - iColx;
		if(finalColx + iColx - xp < 0)
		{
			x += -finalColx -iColx +xp;
		}
		else if(pPage && (finalColx + iColx + m_recCurFrame.width - xp > pPage->getWidth()))
		{
		    x = pPage->getWidth() - m_recCurFrame.width;
		}
		finalColx = x - iColx;

		UT_sint32 finalColy = y - iColy;
		if(finalColy + iColy - yp < 0 )
		{
			y += -iColy - finalColy +yp;
		}
		else if (pPage && (finalColy + iColy - yp+  m_recCurFrame.height  > pPage->getHeight()))
		{
		        y = pPage->getHeight() - m_recCurFrame.height;
		}
		finalColy = y - iColy;

		double xPos = static_cast<double>(finalColx)/static_cast<double>(UT_LAYOUT_RESOLUTION);
		double yPos = static_cast<double>(finalColy)/static_cast<double>(UT_LAYOUT_RESOLUTION);
		FrameStrings.sColXpos = UT_formatDimensionedValue(xPos,"in", NULL);
		FrameStrings.sColYpos = UT_formatDimensionedValue(yPos,"in", NULL);
		//
		// OK calculate relative to page now
		//
		xPos += static_cast<double>(pCol->getX())/static_cast<double>(UT_LAYOUT_RESOLUTION);
		yPos += static_cast<double>(pCol->getY())/static_cast<double>(UT_LAYOUT_RESOLUTION);
		FrameStrings.sPageXpos = UT_formatDimensionedValue(xPos,"in", NULL);
		FrameStrings.sPageYpos = UT_formatDimensionedValue(yPos,"in", NULL);

//
// Find the screen coords of pLine, then work out the offset to the (x,y)
// point. After that workout the offset to the first line of the block.
//
//
		UT_sint32 xBlockOff =0;
		UT_sint32 yBlockOff = 0;
		UT_DebugOnly<bool> bValid = false;
		bValid = pBL->getXYOffsetToLine(xBlockOff,yBlockOff,pLine);
		UT_ASSERT(bValid);
		fp_Line * pFirstL = static_cast<fp_Line *>(pBL->getFirstContainer());
		UT_sint32 xFirst,yFirst;
		pFirstL->getScreenOffsets(pFirstL->getFirstRun(),xFirst,yFirst);
		xxx_UT_DEBUGMSG(("First line x  %d y %d \n",xFirst,yFirst));
		xxx_UT_DEBUGMSG(("xBlockOffset %d yBlockOffset %d \n",xBlockOff,yBlockOff));
		UT_sint32 xLineOff = 0;
		UT_sint32 yLineOff = 0;
		fp_VerticalContainer * pVCon = static_cast<fp_VerticalContainer *>(pLine->getContainer());
		pVCon->getOffsets(pLine,xLineOff,yLineOff);
		xLineOff -= pLine->getX();
		xxx_UT_DEBUGMSG(("Closest Line yLineoff %d \n",yLineOff));

// OK correct for page offsets
		pPage = pVCon->getPage();
		if(pPage == NULL)
		{
			return false;
		}
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
		xPos = static_cast<double>(xLineOff)/static_cast<double>(UT_LAYOUT_RESOLUTION);
		yPos = static_cast<double>(yLineOff)/static_cast<double>(UT_LAYOUT_RESOLUTION);
		FrameStrings.sXpos = UT_formatDimensionedValue(xPos,"in", NULL);
		FrameStrings.sYpos = UT_formatDimensionedValue(yPos,"in", NULL);
		FrameStrings.sWidth = UT_formatDimensionedValue(dWidth,"in", NULL);
		FrameStrings.sHeight = UT_formatDimensionedValue(dHeight,"in", NULL);
		*ppPage = pPage;
		UT_sint32 iPage = getView()->getLayout()->findPage(pPage);
		UT_String_sprintf(FrameStrings.sPrefPage,"%d",iPage);
		fp_Column * pColumn = static_cast<fp_Column *>(pCol);
		UT_sint32 iColumn = pColumn->getColumnIndex();
		UT_String_sprintf(FrameStrings.sPrefColumn,"%d",iColumn);

		return true;
}

//
// Abort the current drag
//
//
void FV_FrameEdit::abortDrag(void)
{
  FV_ViewDoubleBuffering dblBuffObj(m_pView, true, true);
  dblBuffObj.beginDoubleBuffering();


  UT_DEBUGMSG(("Doing Abort Drag \n"));
  m_xLastMouse = m_iFirstEverX;
  m_yLastMouse = m_iFirstEverY;
  mouseRelease(m_iInitialDragX,m_iInitialDragY);
  getView()->updateScreen(false);
}

void FV_FrameEdit::mouseRelease(UT_sint32 x, UT_sint32 y)
{
//
// If we've just selected the frame, ignore this event.
//

	FV_ViewDoubleBuffering dblBuffObj(m_pView, true, true);
	dblBuffObj.beginDoubleBuffering();

	UT_DEBUGMSG(("Doing mouse release now! Mode %d \n", getFrameEditMode()));
	if(FV_FrameEdit_EXISTING_SELECTED == m_iFrameEditMode)
	{
		UT_DEBUGMSG(("Existing Frame selected now released button isActive() %d \n",isActive()));
		return;
	}
	if(m_pAutoScrollTimer != NULL)
	{
		m_pAutoScrollTimer->stop();
		DELETEP(m_pAutoScrollTimer);
	}

	PT_DocPosition posAtXY = 0;

	if(m_iFrameEditMode == 	FV_FrameEdit_RESIZE_INSERT)
	{
		// Signal PieceTable Change
		m_pView->_saveAndNotifyPieceTableChange();

	// Turn off list updates

		getDoc()->disableListUpdates();
		_beginGlob();

		fv_FrameStrings FrameStrings;
		fl_BlockLayout * pCloseBL = NULL;
		fp_Page * pPage = NULL;
		getFrameStrings(m_recCurFrame.left,m_recCurFrame.top,FrameStrings,&pCloseBL,&pPage);
		pf_Frag_Strux * pfFrame = NULL;
		// WARNING: Will need to change this to accomodate variable styles without constantly resetting to solid.
		//				 Recommend to do whatever is done for thickness, which must also have a default set but not
		//				 reverted to on every change.
		// TODO: if(pAP->getProperty("*-thickness", somePropHolder)) sLeftThickness = gchar_strdup(somePropHolder); else sLeftThickness = "1px";
		PP_PropertyVector props = {
			"frame-type", "textbox",
			"wrap-mode", "wrapped-both",
			"position-to", "column-above-text",
			"xpos", FrameStrings.sXpos.c_str(),
			"ypos", FrameStrings.sYpos.c_str(),
			"frame-width", FrameStrings.sWidth.c_str(),
			"frame-height", FrameStrings.sHeight.c_str(),
			"frame-col-xpos", FrameStrings.sColXpos.c_str(),
			"frame-col-ypos", FrameStrings.sColYpos.c_str(),
			"frame-page-xpos", FrameStrings.sPageXpos.c_str(),
			"frame-page-ypos", FrameStrings.sPageYpos.c_str(),
			"frame-pref-page", FrameStrings.sPrefPage.c_str(),
			"frame-pref-column", FrameStrings.sPrefColumn.c_str(),
			"background-color", "ffffff",
			"left-style", "1",
			"right-style", "1",
			"top-style", "1",
			"bot-style", "1",
			"bg-style", "1",
			"tight-wrap", "0",
			"frame-rel-width", m_sRelWidth.c_str(),
			"frame-min-height", m_sMinHeight.c_str(),
			"frame-expand-height", m_sExpandHeight.c_str(),
		};
//
// This should place the the frame strux immediately after the block containing
// position posXY.
// It returns the Frag_Strux of the new frame.
//
		const PP_AttrProp* pBlockAP = NULL;
		pCloseBL->getAP(pBlockAP);
		posAtXY = pCloseBL->getPosition();
		getDoc()->insertStrux(posAtXY, PTX_SectionFrame, PP_NOPROPS, props, &pfFrame);
		PT_DocPosition posFrame = pfFrame->getPos();
		PT_DocPosition posEOD= 0;
		m_pView->getEditableBounds(true,posEOD);
		getDoc()->insertStrux(posFrame+1,PTX_Block,PP_NOPROPS,pBlockAP->getProperties());
		getDoc()->insertStrux(posFrame+2,PTX_EndFrame);
		m_pView->insertParaBreakIfNeededAtPos(posFrame+3);

// Place the insertion point in the Frame

		m_pView->setPoint(posFrame+2);

// Finish up with the usual stuff

		m_pView->_generalUpdate();

		_endGlob();


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
		if(getGraphics())
		{
		      getGraphics()->allCarets()->disable();
		      m_pView->m_countDisable++;
		}
		fl_BlockLayout * pBL = m_pView->_findBlockAtPosition(posFrame+2);
		fl_ContainerLayout * pCL = pBL->myContainingLayout();
		while(pCL && (pCL->getContainerType() != FL_CONTAINER_FRAME) && (pCL->getContainerType() != FL_CONTAINER_DOCSECTION))
		{
			pCL = pCL->myContainingLayout();
		}
		UT_return_if_fail(pCL);
		if(pCL->getContainerType() != FL_CONTAINER_FRAME)
		{
			return;
		}
		m_pFrameLayout = static_cast<fl_FrameLayout *>(pCL);
		UT_ASSERT(m_pFrameLayout->getContainerType() == FL_CONTAINER_FRAME);
		m_pFrameContainer = static_cast<fp_FrameContainer *>(m_pFrameLayout->getFirstContainer());
		UT_ASSERT(m_pFrameContainer);
		drawFrame(true);
		m_bFirstDragDone = false;
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
		if(haveDragged() < 10)
		{
			m_iFrameEditMode = FV_FrameEdit_NOT_ACTIVE;
			setDragWhat( FV_DragNothing );
			m_pFrameContainer->_setX(m_iInitialFrameX);
			m_pFrameContainer->_setY(m_iInitialFrameY);
			m_iInitialFrameX = 0;
			m_iInitialFrameY = 0;
			drawFrame(false);
			m_pFrameLayout = NULL;
			m_pFrameContainer = NULL;
			DELETEP(m_pFrameImage);
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
			m_iLastX = 0;
			m_iLastY = 0;
			m_bFirstDragDone = false;
			while(m_iGlobCount > 0)
				_endGlob();
			m_pView->warpInsPtToXY(x,y,true);
			UT_DEBUGMSG(("Completed small drag \n"));
			return;
		}
//
// OK get the properties of the current frame, update them with the new
// the position and size of this drag.
//

//  Frame Image

		fv_FrameStrings FrameStrings;
		fl_BlockLayout * pCloseBL = NULL;
		fp_Page * pPage = NULL;
		fl_FrameLayout *pFL = m_pFrameLayout;
		getFrameStrings(m_recCurFrame.left,m_recCurFrame.top,FrameStrings,
				&pCloseBL,&pPage);
		posAtXY = pCloseBL->getPosition();

		const PP_PropertyVector props = {
			"xpos", FrameStrings.sXpos.c_str(),
			"ypos", FrameStrings.sYpos.c_str(),
			"frame-col-xpos", FrameStrings.sColXpos.c_str(),
			"frame-col-ypos", FrameStrings.sColYpos.c_str(),
			"frame-page-xpos", FrameStrings.sPageXpos.c_str(),
			"frame-page-ypos", FrameStrings.sPageYpos.c_str(),
			"frame-pref-page", FrameStrings.sPrefPage.c_str(),
			"frame-pref-column", FrameStrings.sPrefColumn.c_str(),
			"frame-width", FrameStrings.sWidth.c_str(),
			"frame-height", FrameStrings.sHeight.c_str()
		};
		// Signal PieceTable Change
		m_pView->_saveAndNotifyPieceTableChange();
		getDoc()->disableListUpdates();
		_beginGlob();

		m_pView->_clearSelection();

		if (pFL->getParentContainer() == pCloseBL)
		{
		    PT_DocPosition posStart = pFL->getPosition(true)+1;
		    PT_DocPosition posEnd = posStart;
		    UT_DebugOnly<bool> bRet = getDoc()->changeStruxFmt(PTC_AddFmt, posStart, posEnd, PP_NOPROPS,
								       props, PTX_SectionFrame);
		    UT_ASSERT(bRet);
		}
		else
		{
			pFL = getLayout()->relocateFrame(pFL, pCloseBL, PP_NOPROPS, props);
		}

        // Finish up with the usual stuff
		m_pView->_generalUpdate();

	// restore updates and clean up dirty lists
		getDoc()->enableListUpdates();
		getDoc()->updateDirtyLists();
		m_pView->_restorePieceTableState();
		PT_DocPosition posFrame = getDoc()->getStruxPosition(pFL->getStruxDocHandle());
		m_pView->setPoint(posFrame+1);
		bool bOK = true;
		while(!m_pView->isPointLegal() && bOK)
		{
		    bOK = m_pView->_charMotion(true,1);
		}
		m_pView->notifyListeners(AV_CHG_HDRFTR);
		m_pView->_fixInsertionPointCoords();
//
// If this was a drag following the initial click, wrap it in a 
// endUserAtomicGlob so it undo's in a single click.
//
		while(m_iGlobCount > 0)
			_endGlob();

		m_bInitialClick = false;
//
// Finish up by putting the editmode back to existing selected.
//	
		m_pView->updateScreen(false);
		m_pFrameLayout = pFL;
		setMode(FV_FrameEdit_EXISTING_SELECTED);
		if(getGraphics())
		{
		      getGraphics()->allCarets()->disable();
		      m_pView->m_countDisable++;
		}
		if(m_pFrameLayout)
			m_pFrameContainer = static_cast<fp_FrameContainer *>(m_pFrameLayout->getFirstContainer());
		drawFrame(true);
		m_bFirstDragDone = false;
		// This must be done after painting because it will finish the cairo surface, see #13622
		m_pView->_ensureInsertionPointOnScreen();
	}
	m_bFirstDragDone = false;
}

/*
 * Return the bytebuf of the image for this.
 */
const char * FV_FrameEdit::getPNGImage(const UT_ByteBuf ** ppByteBuf )
{

//  Frame Image
      const PP_AttrProp* pSectionAP = NULL;
      m_pFrameLayout->getAP(pSectionAP);
      
      const char * pszDataID = NULL;
      pSectionAP->getAttribute(PT_STRUX_IMAGE_DATAID, (const gchar *&)pszDataID);
      if(!pszDataID)
      {
	  *ppByteBuf = NULL;
	  return NULL;
      }
      m_pView->getDocument()->getDataItemDataByName(pszDataID,ppByteBuf,NULL,NULL);
      return pszDataID;
}

/*!
 * This method deletes the current selected frame
 */
void FV_FrameEdit::deleteFrame(fl_FrameLayout * pFL)
{
	if(m_pFrameLayout == NULL)
	{
	        m_pFrameLayout = pFL;
		if(m_pFrameLayout == NULL)
		{
		  UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		  return;
		}
	}

	FV_ViewDoubleBuffering dblBuffObj(m_pView, true, true);
	dblBuffObj.beginDoubleBuffering();

	PP_AttrProp * p_AttrProp_Before = NULL;

	// Signal PieceTable Change
	m_pView->_saveAndNotifyPieceTableChange();

	// Turn off list updates

	getDoc()->disableListUpdates();
	_beginGlob();

// Delete the frame

	pf_Frag_Strux* sdhStart =  m_pFrameLayout->getStruxDocHandle();
	pf_Frag_Strux* sdhEnd = NULL;
	PT_DocPosition posStart = getDoc()->getStruxPosition(sdhStart);
	getDoc()->getNextStruxOfType(sdhStart, PTX_EndFrame, &sdhEnd);
	PT_DocPosition posEnd = getDoc()->getStruxPosition(sdhEnd)+1;	
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

	m_pView->_generalUpdate();

	// restore updates and clean up dirty lists
	getDoc()->enableListUpdates();
	getDoc()->updateDirtyLists();

	// Signal PieceTable Changes have finished
	m_pView->_restorePieceTableState();
	m_pView->notifyListeners(AV_CHG_HDRFTR);
	m_pView->_fixInsertionPointCoords();
	m_pView->_ensureInsertionPointOnScreen();
	while(m_iGlobCount > 0)
		_endGlob();

// Clear all internal variables

	m_pFrameLayout = NULL;
	m_pFrameContainer = NULL;
	DELETEP(m_pFrameImage);
	m_recCurFrame.width = 0;
	m_recCurFrame.height = 0;
	setDragWhat( FV_DragNothing );
	m_iLastX = 0;
	m_iLastY = 0;

	m_iFrameEditMode = FV_FrameEdit_NOT_ACTIVE;
	m_bFirstDragDone = false;
	m_pView->_setPoint(m_pView->getPoint());
}

/*!
 * Method to deal with mouse coordinates.
 */
FV_DragWhat FV_FrameEdit::mouseMotion(UT_sint32 /*x*/, UT_sint32 /*y*/)
{
	return getDragWhat();
}

void FV_FrameEdit::drawFrame(bool bWithHandles)
{
	if(m_pFrameContainer == NULL)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
	if((m_pFrameImage == NULL) || (getDragWhat() != FV_DragWhole) )
	{
//		m_pFrameContainer->clearScreen();
		m_pFrameContainer->draw(&da);
		if(bWithHandles)
		{
			m_pFrameContainer->drawHandles(&da);
		}
		if(getDragWhat() == FV_DragWhole)
		{
			GR_Painter painter (getGraphics());
			if(m_pFrameLayout->getFrameType() == FL_FRAME_TEXTBOX_TYPE)
			{
			      m_pFrameImage = painter.genImageFromRectangle(m_recCurFrame);
			}
			else
		        {
			      UT_Rect rec = m_recCurFrame;
			      rec.left = 0;
			      rec.top = 0;
				  UT_return_if_fail(m_pFrameLayout->getBackgroundImage());
			      m_pFrameImage = m_pFrameLayout->getBackgroundImage()->createImageSegment(getGraphics(),rec);
			}
		}
	}
	else
	{
		GR_Painter painter(getGraphics());
		m_pView->draw(&m_recCurFrame);
		painter.drawImage(m_pFrameImage,m_recCurFrame.left,m_recCurFrame.top);
	}
}
