/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "ap_FrameData.h"
#include "ev_BeOSToolbar.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_BeOSGraphics.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_BeOSFrame.h"
#include "xap_BeOSApp.h"
#include "ap_BeOSTopRuler.h"
#include "ap_BeOSLeftRuler.h"
#include "ap_Prefs.h"
#include "ap_BeOSStatusBar.h"
//#include "ap_BeOSViewListener.h"

#include "ev_BeOSKeyboard.h"
#include "ev_BeOSMouse.h"


/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/


/*
 This function actually sets up the value of the scroll bar in 
 terms of it max/min and step size, and will set an initial
 value of the widget by calling the scroll event which
 will eventually call _scrollFuncX()
*/
void AP_BeOSFrame::setXScrollRange(void)
{
// Code removed because it was broken. See older version for reference when reimplementing.
}

void AP_BeOSFrame::setYScrollRange(void)
{
// Code removed because it was broken. See older version for reference when reimplementing.
}


AP_BeOSFrame::AP_BeOSFrame(XAP_BeOSApp * app)
	: AP_Frame(new AP_BeOSFrameImpl(this,app),app)
{
	m_pData = NULL;
	setFrameLocked(false);	
}

AP_BeOSFrame::AP_BeOSFrame(AP_BeOSFrame * f)
	: AP_Frame(static_cast<AP_Frame *>(f))
{
	m_pData = NULL;
}

AP_BeOSFrame::~AP_BeOSFrame()
{
	killFrameData();
}

XAP_Frame * AP_BeOSFrame::cloneFrame(void)
{
	AP_BeOSFrame * pClone = new AP_BeOSFrame(this);
	ENSUREP(pClone);
	return static_cast<XAP_Frame *>(pClone);

Cleanup:
	// clean up anything we created here
	if (pClone)
	{
		// BROKEN, probably leaking memory now!
		//m_pBeOSApp->forgetFrame(pClone);
		delete pClone;
	}

	return NULL;
}

bool AP_BeOSFrame::initialize(XAP_FrameMode frameMode)
{
	if (!initFrameData())
		return false;

	if (!XAP_Frame::initialize(AP_PREF_KEY_KeyBindings, AP_PREF_DEFAULT_KeyBindings,
								   AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
								   AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet,
								   AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
								   AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet))
		return false;
/* BROKEN:
	_createTopLevelWindow();

	//At this point in time the BeOS widgets are all
	//realized so we should be able to go and attach
	//the various input filters to them.
	ev_BeOSKeyboard * pBeOSKeyboard = static_cast<ev_BeOSKeyboard *>(m_pKeyboard);
	pBeOSKeyboard->synthesize(m_pBeOSApp, this);

	ev_BeOSMouse * pBeOSMouse = static_cast<ev_BeOSMouse *>(m_pMouse);
	pBeOSMouse->synthesize(m_pBeOSApp, this);

	//Actually show the window to the world
	m_pBeWin->Show();
 	//getTopLevelWindow()->Show();
*/
	return true;
}

/*****************************************************************/








/*
  We've been notified (via sendScrollEvent()) of a scroll (probably
  a keyboard motion or user scroll event).  push the new values into 
  the scrollbar widgets (with clamping).  Then cause the view to scroll.
*/
void AP_BeOSFrame::_scrollFuncX(void * pData, 
								UT_sint32 xoff, 
								UT_sint32 /*xrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	AP_BeOSFrame * pBeOSFrame = static_cast<AP_BeOSFrame *>(pData);
	AV_View * pView = pBeOSFrame->getCurrentView();

	//Actually set the scroll bar value ...
	pView->setXScrollOffset(xoff);

	AP_BeOSFrameImpl *pBeOSFrameImpl = static_cast<AP_BeOSFrameImpl *>(pBeOSFrame->getFrameImpl());	
	BScrollBar *hscroll = pBeOSFrameImpl->m_hScroll;
	if (hscroll->Window()->Lock()) {
		float min, max;
		hscroll->GetRange(&min, &max);
		if (xoff < min)	xoff = (UT_sint32)min;
		if (xoff > max)	xoff = (UT_sint32)max;
  		hscroll->SetValue(xoff);
    	hscroll->Window()->Unlock(); 
	}
}
void AP_BeOSFrame::_scrollFuncY(void * pData, 
								UT_sint32 yoff, 
								UT_sint32 /*yrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	AP_BeOSFrame * pBeOSFrame = static_cast<AP_BeOSFrame *>(pData);
	AV_View * pView = pBeOSFrame->getCurrentView();

	//Actually set the scroll bar value ...
	pView->setYScrollOffset(yoff);

	AP_BeOSFrameImpl *pBeOSFrameImpl = static_cast<AP_BeOSFrameImpl *>(pBeOSFrame->getFrameImpl());	
   	BScrollBar *vscroll = pBeOSFrameImpl->m_vScroll;
   	if (vscroll->Window()->Lock()) {
		float min, max;
		vscroll->GetRange(&min, &max);
		if (yoff < min)	yoff = (UT_sint32)min;
		if (yoff > max)	yoff = (UT_sint32)max;
		vscroll->SetValue(yoff);
   		vscroll->Window()->Unlock(); 
	}
}

void AP_BeOSFrame::setStatusMessage(const char * szMsg)
{
	((AP_FrameData *)m_pData)->m_pStatusBar->setStatusMessage(szMsg);
}                                                                        

void AP_BeOSFrame::toggleBar(UT_uint32 iBarNb, bool bBarOn)
{
	UT_DEBUGMSG(("AP_BeOSFrame::toggleBar %d\n", bBarOn));

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
	UT_ASSERT(pFrameData);
	UT_ASSERT(pFrameData->m_pToolbar);
	
	AP_BeOSFrameImpl *pBeOSFrameImpl = static_cast<AP_BeOSFrameImpl *>(getFrameImpl());	
	pBeOSFrameImpl->m_pBeWin->Lock();

	EV_Toolbar *pToolbar = pFrameData->m_pToolbar[iBarNb];
	
	UT_ASSERT(pToolbar);

	int height = 35;//TODO:tempolary

	if (bBarOn)
	{
		pToolbar->show();
		height *= -1;
		printf("Show Toolbar #%d\n", iBarNb);
	}
	else	// turning toolbar off
	{
		pToolbar->hide();
		printf("Hide Toolbar #%d\n", iBarNb);
	}

	pBeOSFrameImpl->m_pBeWin->FindView("TopRuler")->MoveBy(0, height * -1);
	pBeOSFrameImpl->m_pBeWin->FindView("LeftRuler")->MoveBy(0, height * -1);
	pBeOSFrameImpl->m_pBeWin->FindView("LeftRuler")->ResizeBy(0, height);

/* BROKEN
	be_DocView *pView = getBeDocView();

	if(pView)
	{
		pView->MoveBy(0, height * -1);
		pView->ResizeBy(0, height);

		if (iBarNb = 0)
		{
			pFrameData->m_pToolbar[0]->moveby(height * - 1);//NOTE Why? different 1
			pFrameData->m_pToolbar[2]->moveby(height * - 1);
		}
		if (iBarNb = 1)
			pFrameData->m_pToolbar[2]->moveby(height * - 1);

	}
*/
	pBeOSFrameImpl->m_vScroll->MoveBy(0, height * -1);
	pBeOSFrameImpl->m_vScroll->ResizeBy(0, height);

	pBeOSFrameImpl->m_pBeWin->Unlock();
}

void AP_BeOSFrame::toggleStatusBar(bool bStatusBarOn)
{
	UT_DEBUGMSG(("AP_BeOSFrame::toggleStatusBar %d\n", bStatusBarOn));

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
	UT_ASSERT(pFrameData);

	AP_BeOSFrameImpl *pBeOSFrameImpl = static_cast<AP_BeOSFrameImpl *>(getFrameImpl());	
	pBeOSFrameImpl->m_pBeWin->Lock();
	if ( pBeOSFrameImpl->m_pBeWin->FindView("StatusBar")->IsHidden() == bStatusBarOn)
	{
		int height = STATUS_BAR_HEIGHT + 1;
		
		if (bStatusBarOn)
		{
			pFrameData->m_pStatusBar->show();
			height *= -1;
			printf("Show Statusbar\n");
		}
		else    // turning status bar off
		{
			pFrameData->m_pStatusBar->hide();
			printf("Hide Statusbar\n");
		}

		/* BROKEN:
		be_DocView *pView = getBeDocView();
		if(pView)
		{
			pView->ResizeBy(0, height);
		} */
		pBeOSFrameImpl->m_pBeWin->FindView("LeftRuler")->ResizeBy(0, height);
		pBeOSFrameImpl->m_hScroll->MoveBy(0, height);
		pBeOSFrameImpl->m_vScroll->ResizeBy(0, height * 2 / 3);
	}
	pBeOSFrameImpl->m_pBeWin->Unlock();
}

void AP_BeOSFrame::toggleRuler(bool bRulerOn)
{
	toggleTopRuler(bRulerOn);
	toggleLeftRuler(bRulerOn);
}

void AP_BeOSFrame::toggleTopRuler(bool bRulerOn)
{
	UT_DEBUGMSG(("AP_BeOSFrame::toggleRuler %d", bRulerOn));	
	UT_ASSERT(pFrameData);

	AP_BeOSFrameImpl *pBeOSFrameImpl = static_cast<AP_BeOSFrameImpl *>(getFrameImpl());	
	pBeOSFrameImpl->m_pBeWin->Lock();

// BROKEN:
//	be_DocView *pView = getBeDocView();
	BRect rect = pBeOSFrameImpl->m_pBeWin->FindView("TopRuler")->Frame();
	int height = (int)rect.Height() + 1;

	if (pBeOSFrameImpl->m_pBeWin->FindView("TopRuler")->IsHidden() == bRulerOn)
	if (bRulerOn)
	{
		printf("Show Top Ruler\n");
		pBeOSFrameImpl->m_pBeWin->FindView("TopRuler")->Show();
/*
		if(pView)
		{
			pView->ResizeBy(0, height * -1);
			pView->MoveBy(0, height);
		}
*/
	}
	else
	{
		printf("Hide Top Ruler\n");
		pBeOSFrameImpl->m_pBeWin->FindView("TopRuler")->Hide();
/*
		if(pView)
		{
			pView->ResizeBy(0, height);
			pView->MoveBy(0, height * -1);
		}
*/
	}
	pBeOSFrameImpl->m_pBeWin->Unlock();
}

void AP_BeOSFrame::toggleLeftRuler(bool bRulerOn)
{
	UT_DEBUGMSG(("AP_BeOSFrame::toggleRuler %d", bRulerOn));
	UT_ASSERT(pFrameData);

	AP_BeOSFrameImpl *pBeOSFrameImpl = static_cast<AP_BeOSFrameImpl *>(getFrameImpl());	
	pBeOSFrameImpl->m_pBeWin->Lock();

// BROKEN:
//	be_DocView *pView = getBeDocView();
	BRect rect = pBeOSFrameImpl->m_pBeWin->FindView("LeftRuler")->Frame();
	int width = (int)rect.Width() + 1;

	if (pBeOSFrameImpl->m_pBeWin->FindView("LeftRuler")->IsHidden() == bRulerOn)
	{
		if (bRulerOn)
		{
			printf("Show Left Ruler\n");
			pBeOSFrameImpl->m_pBeWin->FindView("LeftRuler")->Show();
/*
			if(pView)
			{
				pView->ResizeBy(width * -1, 0);
				pView->MoveBy(width, 0);
			}
*/
		}
		else
		{
			printf("Hide Left Ruler\n");
			pBeOSFrameImpl->m_pBeWin->FindView("LeftRuler")->Hide();
/*
			if(pView)
			{
				pView->ResizeBy(width , 0);
				pView->MoveBy(width * -1, 0);
			}
*/
		}
	}
	pBeOSFrameImpl->m_pBeWin->Unlock();
}

void AP_BeOSFrame::translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y)
{ 
	// translate the given document mouse coordinates into absolute screen coordinates.

	BPoint pt(x,y);// = { x, y };

	AP_BeOSFrameImpl *pBeOSFrameImpl = static_cast<AP_BeOSFrameImpl *>(getFrameImpl());	
	pBeOSFrameImpl->m_pBeWin->Lock();
/*	
	be_DocView *pView = getBeDocView();
	
	if(pView)
	{
		pView->ConvertToScreen(&pt);
	}
*/	
	pBeOSFrameImpl->m_pBeWin->Unlock();
	
	x = (UT_sint32)pt.x;
	y = (UT_sint32)pt.y;
}



UT_sint32 AP_BeOSFrame::_getDocumentAreaWidth()
{
	return 0;
}

UT_sint32 AP_BeOSFrame::_getDocumentAreaHeight()
{
	return 0;
}

bool AP_BeOSFrame::_createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom)
{
//	pG = new GR_BeOSGraphics(static_cast<AP_BeOSFrameImpl *>(getFrameImpl())->getTopLevelWindow(),static_cast<AP_BeOSFrameImpl *>(getFrameImpl())->m_dArea,getApp());
//	UT_ASSERT(pG);
//	pG->setZoomPercentage(iZoom);
	return true;
}

void AP_BeOSFrame::_setViewFocus(AV_View *pView)
{
}

void AP_BeOSFrame::_bindToolbars(AV_View *pView)
{
	static_cast<AP_BeOSFrameImpl *>(getFrameImpl())->_bindToolbars(pView);
}

bool AP_BeOSFrame::_createScrollBarListeners(AV_View * pView, AV_ScrollObj *& pScrollObj, 
					     ap_ViewListener *& pViewListener, ap_Scrollbar_ViewListener *& pScrollbarViewListener,
					     AV_ListenerId &lid, AV_ListenerId &lidScrollbarViewListener)
{
/*
	pScrollObj = new AV_ScrollObj(this,_scrollFuncX,_scrollFuncY);
	UT_ASSERT(pScrollObj);

	pViewListener = new ap_BeOSViewListener(this);
	UT_ASSERT(pViewListener);
	pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
	UT_ASSERT(pScrollbarViewListener);
	
	if (!pView->addListener(static_cast<AV_Listener *>(pViewListener),&lid))
		return false;
	if (!pView->addListener(static_cast<AV_Listener *>(pScrollbarViewListener),
							&lidScrollbarViewListener))
		return false;
*/
	return true;
}
