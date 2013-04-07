/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001-2003 Hubert Figuiere
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

#include <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "ap_FrameData.h"
#include "xap_CocoaFrame.h"
#include "ev_CocoaToolbar.h"
#include "xav_View.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "gr_CocoaCairoGraphics.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_CocoaFrame.h"
#include "xap_CocoaApp.h"
#include "ap_CocoaTopRuler.h"
#include "ap_CocoaLeftRuler.h"
#include "ap_CocoaStatusBar.h"
#include "ap_CocoaViewListener.h"

#import "ap_CocoaFrameImpl.h"

/*****************************************************************/
#define ENSUREP_RF(p)            do { UT_ASSERT(p); if (!p) return false; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)


void AP_CocoaFrame::setXScrollRange(void)
{
	GR_Graphics*	pGr = ((AP_FrameData*)m_pData)->m_pG;
	AP_CocoaFrameImpl* pFrameImpl = static_cast<AP_CocoaFrameImpl *>(getFrameImpl());
	UT_sint32 width = ((AP_FrameData*)m_pData)->m_pDocLayout->getWidth();
	NSRect rect = [pFrameImpl->m_docAreaGRView frame];
	UT_sint32 visibleWidth = pGr->tlu(lrintf(rect.size.width));
	pFrameImpl->_setHVisible(visibleWidth);
	UT_DEBUGMSG(("visibleWidth: %d, doc width:%d\n", visibleWidth, width));
	if (m_pView == NULL) {
		UT_DEBUGMSG(("m_pView is NULL\n"));
	}

	UT_sint32 newvalue = ((m_pView) ? m_pView->getXScrollOffset() : 0);
	UT_sint32 newmax = width - visibleWidth; /* upper - page_size */
	if (newmax <= 0)
		newmax = 0;
	else if (newvalue > newmax)
		newvalue = newmax;
	UT_DEBUGMSG (("newmax = %d, newvalue = %d\n", newmax, newvalue));
	pFrameImpl->_setHScrollMax(newmax);
	pFrameImpl->_setHScrollValue(newvalue);

	m_pView->sendHorizontalScrollEvent(newvalue, newmax);
}

void AP_CocoaFrame::setYScrollRange(void)
{
	GR_Graphics*	pGr = ((AP_FrameData*)m_pData)->m_pG;
	AP_CocoaFrameImpl * pFrameImpl = static_cast<AP_CocoaFrameImpl *>(getFrameImpl());
	UT_sint32 height = ((AP_FrameData*)m_pData)->m_pDocLayout->getHeight();
	NSRect rect = [pFrameImpl->m_docAreaGRView frame];
	UT_sint32 visibleHeight = pGr->tlu(lrintf(rect.size.height));
	pFrameImpl->_setVVisible(visibleHeight);
	UT_DEBUGMSG(("visibleHeight: %d, doc height:%d\n", visibleHeight, height));
	if (m_pView == NULL) {
		UT_DEBUGMSG(("m_pView is NULL\n"));
	}

	UT_sint32 newvalue = ((m_pView) ? m_pView->getYScrollOffset() : 0);
	UT_sint32 newmax = height - visibleHeight;	/* upper - page_size */
	if (newmax <= 0)
		newmax = 0;
	else if (newvalue > newmax)
		newvalue = newmax;
	UT_DEBUGMSG (("newmax = %d, newvalue = %d\n", newmax, newvalue));
	pFrameImpl->_setVScrollMax(newmax);
	pFrameImpl->_setVScrollValue(newvalue);

	// TODO optimize
	m_pView->sendVerticalScrollEvent(newvalue, newmax);
}


AP_CocoaFrame::AP_CocoaFrame()
	: AP_Frame (new AP_CocoaFrameImpl(this))
{
	m_pData = NULL;
//	static_cast<AP_CocoaFrameImpl *>(m_pFrameImpl)->setShowDocLocked(false);
}

AP_CocoaFrame::AP_CocoaFrame(AP_CocoaFrame * f)
	: AP_Frame(static_cast<AP_Frame *>(f))
{
	m_pData = NULL;
}

AP_CocoaFrame::~AP_CocoaFrame()
{
	killFrameData();
}

bool AP_CocoaFrame::initialize(XAP_FrameMode /*frameMode*/)
{
	AP_CocoaFrameImpl* pFrameImpl = static_cast<AP_CocoaFrameImpl *>(getFrameImpl());
	UT_DEBUGMSG(("AP_CocoaFrame::initialize\n"));
	if (!initFrameData())
		return false;

	if (!XAP_Frame::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings,
								   AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
								   AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet,
								   AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
								   AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet))
		return false;

	pFrameImpl->_createTopLevelWindow();
//	gtk_widget_show(m_wTopLevelWindow);
	if(getFrameMode() == XAP_NormalFrame)
	{
		// needs to be shown so that the following functions work
		// TODO: get rid of cursed flicker caused by initially
		// TODO: showing these and then hiding them (esp.
		// TODO: noticable in the gnome build with a toolbar disabled)
		pFrameImpl->_showOrHideToolbars();
		pFrameImpl->_showOrHideStatusbar();
	}
	// pFrameImpl->_show(); // defer this

	return true;
}




/*****************************************************************/

XAP_Frame * AP_CocoaFrame::cloneFrame()
{
	AP_CocoaFrame * pClone = new AP_CocoaFrame(this);
	ENSUREP(pClone);
	return static_cast<XAP_Frame *> (pClone);

Cleanup:
	// clean up anything we created here
	if (pClone)
	{
		XAP_App::getApp()->forgetFrame(pClone);
		delete pClone;
	}

	return NULL;
}


void AP_CocoaFrame::_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 /*yrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	AP_CocoaFrame * pCocoaFrame = static_cast<AP_CocoaFrame *>(pData);
	AP_CocoaFrameImpl* pFrameImpl = static_cast<AP_CocoaFrameImpl*>(pCocoaFrame->getFrameImpl());
	AV_View * pView = pCocoaFrame->getCurrentView();
	
	if (pFrameImpl->_getVScrollMin() > yoff) {
		yoff = pFrameImpl->_getVScrollMin();
	}
	if (pFrameImpl->_getVScrollMax() < yoff) {
		yoff = pFrameImpl->_getVScrollMax();
	}
	pFrameImpl->_setVScrollValue(yoff);

	pView->setYScrollOffset(yoff);
}

void AP_CocoaFrame::_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 /*xrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	
	AP_CocoaFrame * pCocoaFrame = static_cast<AP_CocoaFrame *>(pData);
	AP_CocoaFrameImpl* pFrameImpl = static_cast<AP_CocoaFrameImpl*>(pCocoaFrame->getFrameImpl());
	AV_View * pView = pCocoaFrame->getCurrentView();
	
	if (pFrameImpl->_getHScrollMin() > xoff) {
		xoff = pFrameImpl->_getHScrollMin();
	}
	if (pFrameImpl->_getHScrollMax() < xoff) {
		xoff = pFrameImpl->_getHScrollMax();
	}
	pFrameImpl->_setHScrollValue(xoff);

	pView->setXScrollOffset(xoff);
}



void AP_CocoaFrame::translateDocumentToScreen(UT_sint32 & /*x*/, UT_sint32 & /*y*/)
{
	// translate the given document mouse coordinates into absolute screen coordinates.
	UT_ASSERT (UT_NOT_IMPLEMENTED);
#if 0
	Window child;
	gint tx;
	gint ty;
  
	GdkWindowPrivate * priv = (GdkWindowPrivate*) m_dArea->window;
	if (!priv->destroyed)
		XTranslateCoordinates (priv->xdisplay, priv->xwindow, gdk_root_window, x, y, &tx, &ty, &child);
  
	x = tx;
	y = ty;
#endif
}


void AP_CocoaFrame::setStatusMessage(const char * szMsg)
{
	((AP_FrameData *)m_pData)->m_pStatusBar->setStatusMessage(szMsg);
}


void AP_CocoaFrame::toggleTopRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(getFrameData());
	UT_ASSERT(pFrameData);
		
	AP_CocoaTopRuler * pCocoaTopRuler = NULL;

	UT_DEBUGMSG(("AP_CocoaFrame::toggleTopRuler %d, %p\n", 
		     bRulerOn, pFrameData->m_pTopRuler));
	if (bRulerOn) {
		AP_TopRuler * pTop = pFrameData->m_pTopRuler;
		if(pTop) {
			delete pTop;
		}

		pCocoaTopRuler = new AP_CocoaTopRuler(this);
		UT_ASSERT(pCocoaTopRuler);

		// get the width from the left ruler and stuff it into the 
		// top ruler.

		if (static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler) {
			pCocoaTopRuler->setOffsetLeftRuler(((AP_FrameData*)m_pData)->m_pLeftRuler->getWidth());
		}
		else {
			pCocoaTopRuler->setOffsetLeftRuler(0);
		}

		// attach everything	
		static_cast<AP_CocoaFrameImpl *>(getFrameImpl())->_showTopRulerNSView();
		FV_View * pView = static_cast<FV_View *>(m_pView);
		UT_uint32 iZoom = pView->getGraphics()->getZoomPercentage();
		static_cast<AP_TopRuler *>(pCocoaTopRuler)->setView(m_pView,iZoom);
	}
	else {
		static_cast<AP_CocoaFrameImpl *>(getFrameImpl())->_hideTopRulerNSView();
		DELETEP(((AP_FrameData*)m_pData)->m_pTopRuler);
		static_cast<FV_View *>(m_pView)->setTopRuler(NULL);
	}
	static_cast<AP_FrameData*>(m_pData)->m_pTopRuler = pCocoaTopRuler;
}

void AP_CocoaFrame::toggleLeftRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);

	UT_DEBUGMSG(("AP_CocoaFrame::toggleLeftRuler %d, %p\n", bRulerOn, pFrameData->m_pLeftRuler));
	
	if (bRulerOn) {
		AP_CocoaLeftRuler* pCocoaLeftRuler;
		FV_View * pView = static_cast<FV_View *>(m_pView);		
		UT_uint32 iZoom = pView->getGraphics()->getZoomPercentage();

		pCocoaLeftRuler = new AP_CocoaLeftRuler(this);
		UT_ASSERT(pCocoaLeftRuler);
		pFrameData->m_pLeftRuler = pCocoaLeftRuler;

		static_cast<AP_CocoaFrameImpl *>(getFrameImpl())->_showLeftRulerNSView();
		static_cast<AP_LeftRuler *>(pCocoaLeftRuler)->setView(m_pView, iZoom);
		setYScrollRange ();
	}
	else  {
		if (pFrameData->m_pLeftRuler) {
			DELETEP(pFrameData->m_pLeftRuler);
			static_cast<AP_CocoaFrameImpl *>(getFrameImpl())->_hideLeftRulerNSView();
			static_cast<FV_View *>(m_pView)->setLeftRuler(NULL);
		}
		else {
			UT_DEBUGMSG(("Left Ruler already hidden\n"));
		}
	}

}

void AP_CocoaFrame::toggleRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);

	toggleTopRuler(bRulerOn);
	toggleLeftRuler(bRulerOn && (pFrameData->m_pViewMode == VIEW_PRINT));

	[(static_cast<AP_CocoaFrameImpl *>(getFrameImpl())->m_docAreaGRView) setNeedsDisplay:YES];
}

void AP_CocoaFrame::toggleBar(UT_uint32 iBarNb, bool bBarOn)
{
	UT_DEBUGMSG(("AP_CocoaFrame::toggleBar %d, %d\n", iBarNb, bBarOn));	

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
	UT_ASSERT(pFrameData);
	
	if (bBarOn) {
		pFrameData->m_pToolbar[iBarNb]->show();
	}
	else {	// turning toolbar off
		pFrameData->m_pToolbar[iBarNb]->hide();
	}
}

void AP_CocoaFrame::toggleStatusBar(bool bStatusBarOn)
{
	UT_DEBUGMSG(("AP_CocoaFrame::toggleStatusBar %d\n", bStatusBarOn));	

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
	UT_ASSERT(pFrameData);
	
	if (bStatusBarOn) {
		pFrameData->m_pStatusBar->show();
	}
	else {	// turning status bar off
		pFrameData->m_pStatusBar->hide();
	}
}

bool AP_CocoaFrame::_createScrollBarListeners(AV_View * pView, AV_ScrollObj *& pScrollObj, 
					     ap_ViewListener *& pViewListener, ap_Scrollbar_ViewListener *& pScrollbarViewListener,
					     AV_ListenerId &lid, AV_ListenerId &lidScrollbarViewListener)
{
	// The "AV_ScrollObj pScrollObj" receives
	// send{Vertical,Horizontal}ScrollEvents
	// from both the scroll-related edit methods
	// and from the UI callbacks.
	// 
	// The "ap_ViewListener pViewListener" receives
	// change notifications as the document changes.
	// This ViewListener is responsible for keeping
	// the title-bar up to date (primarily title
	// changes, dirty indicator, and window number).
	// ON UNIX ONLY: we subclass this with ap_UnixViewListener
	// ON UNIX ONLY: so that we can deal with X-Selections.
	//
	// The "ap_Scrollbar_ViewListener pScrollbarViewListener"
	// receives change notifications as the doucment changes.
	// This ViewListener is responsible for recalibrating the
	// scrollbars as pages are added/removed from the document.
	//
	// Each Toolbar will also get a ViewListener so that
	// it can update toggle buttons, and other state-indicating
	// controls on it.
	//
	// TODO we ***really*** need to re-do the whole scrollbar thing.
	// TODO we have an addScrollListener() using an m_pScrollObj
	// TODO and a View-Listener, and a bunch of other widget stuff.
	// TODO and its very confusing.

	pScrollObj = new AV_ScrollObj(this,_scrollFuncX,_scrollFuncY);
	ENSUREP_RF(pScrollObj);

	pViewListener = new ap_CocoaViewListener(this);
	ENSUREP_RF(pViewListener);
	pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
	ENSUREP_RF(pScrollbarViewListener);
	
	if (!pView->addListener(static_cast<AV_Listener *>(pViewListener),&lid))
		return false;
	if (!pView->addListener(static_cast<AV_Listener *>(pScrollbarViewListener),
							&lidScrollbarViewListener))
		return false;

	return true;
}


UT_sint32 AP_CocoaFrame::_getDocumentAreaWidth()
{
	return (UT_sint32)[static_cast<AP_CocoaFrameImpl *>(getFrameImpl())->m_docAreaGRView frame].size.width;
}

UT_sint32 AP_CocoaFrame::_getDocumentAreaHeight()
{
	return (UT_sint32)[static_cast<AP_CocoaFrameImpl *>(getFrameImpl())->m_docAreaGRView frame].size.height;
}

bool AP_CocoaFrame::_createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom)
{

	static_cast<AP_CocoaFrameImpl*>(getFrameImpl())->_createDocView(pG);
	ENSUREP_RF(pG);
	pG->setZoomPercentage(iZoom);

	return true;
}

void AP_CocoaFrame::_setViewFocus(AV_View * /*pView*/)
{
	AP_CocoaFrameImpl * pFrameImpl = static_cast<AP_CocoaFrameImpl *>(getFrameImpl());
	pFrameImpl->giveFocus();
}

void AP_CocoaFrame::_bindToolbars(AV_View *pView)
{
	static_cast<AP_CocoaFrameImpl *>(getFrameImpl())->_bindToolbars(pView);
}

