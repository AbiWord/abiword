/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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

#include <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "ap_FrameData.h"
#include "xap_CocoaFrame.h"
#include "ev_CocoaToolbar.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_CocoaGraphics.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_CocoaFrame.h"
#include "xap_CocoaApp.h"
#include "ap_CocoaTopRuler.h"
#include "ap_CocoaLeftRuler.h"
#include "ap_CocoaStatusBar.h"
#include "ap_CocoaViewListener.h"
#if 1
#include "ev_CocoaMenuBar.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Labels.h"
#include "ev_Menu_Actions.h"
#endif

#import "ap_CocoaFrameImpl.h"

#if 0
#ifdef ABISOURCE_LICENSED_TRADEMARKS
#include "abiword_48_tm.xpm"
#else
#include "abiword_48.xpm"
#endif
#endif

/*****************************************************************/
#define ENSUREP_RF(p)            do { UT_ASSERT(p); if (!p) return false; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)


void AP_CocoaFrame::setZoomPercentage(UT_uint32 iZoom)
{
	XAP_Frame::setZoomPercentage(iZoom);
	_showDocument(iZoom);
}

UT_uint32 AP_CocoaFrame::getZoomPercentage(void)
{
	return ((AP_FrameData*)m_pData)->m_pG->getZoomPercentage();
}


void AP_CocoaFrame::setXScrollRange(void)
{
	int width = ((AP_FrameData*)m_pData)->m_pDocLayout->getWidth();
	NSRect rect = [static_cast<AP_CocoaFrameImpl *>(getFrameImpl())->m_docAreaGRView frame];
	float windowWidth = rect.size.width;

	float newvalue = ((m_pView) ? m_pView->getXScrollOffset() : 0);
	float newmax = width - windowWidth; /* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;
	
//	[m_hScrollbar setHorizontalPageScroll:windowWidth];
//	[m_hScrollbar setHorizontalLineScroll:20.0f];
	
#if 0
	bool bDifferentPosition = (newvalue != (int)m_pHadj->value);
	bool bDifferentLimits = ((width-windowWidth) != (int)(m_pHadj->upper-m_pHadj->page_size));
	
	m_pHadj->value = newvalue;
	m_pHadj->lower = 0.0;
	m_pHadj->upper = (gfloat) width;
	m_pHadj->step_increment = 20.0;
	m_pHadj->page_increment = (gfloat) windowWidth;
	m_pHadj->page_size = (gfloat) windowWidth;
#endif
//	UT_ASSERT (UT_TODO);
//	[scroller drawIfNeeded];
#if 0
	if (m_pView && (bDifferentPosition || bDifferentLimits))
		m_pView->sendHorizontalScrollEvent(newvalue, (int)(m_pHadj->upper-m_pHadj->page_size));
#endif
}

void AP_CocoaFrame::setYScrollRange(void)
{
	int height = ((AP_FrameData*)m_pData)->m_pDocLayout->getHeight();
	NSRect rect = [static_cast<AP_CocoaFrameImpl *>(getFrameImpl())->m_docAreaGRView frame];
	float windowHeight = rect.size.width;

	float newvalue = ((m_pView) ? m_pView->getYScrollOffset() : 0);
	float newmax = height - windowHeight;	/* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;

//	[m_vScrollbar setVerticalPageScroll:windowHeight];
//	[m_vScrollbar setVerticalLineScroll:20.0f];

#if 0
	bool bDifferentPosition = (newvalue != (int)m_pVadj->value);
	bool bDifferentLimits ((height-windowHeight) != (int)(m_pVadj->upper-m_pVadj->page_size));
	
	m_pVadj->value = newvalue;
	m_pVadj->lower = 0.0;
	m_pVadj->upper = (gfloat) height;
	m_pVadj->step_increment = 20.0;
	m_pVadj->page_increment = (gfloat) windowHeight;
	m_pVadj->page_size = (gfloat) windowHeight;
#endif
//	UT_ASSERT (UT_TODO);
//	[scroller drawIfNeeded];
#if 0
	if (m_pView && (bDifferentPosition || bDifferentLimits))
		m_pView->sendVerticalScrollEvent(newvalue, (int)(m_pVadj->upper-m_pVadj->page_size));
#endif
}


AP_CocoaFrame::AP_CocoaFrame(XAP_CocoaApp * app)
	: AP_Frame (new AP_CocoaFrameImpl(this, app), app)
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

bool AP_CocoaFrame::initialize(XAP_FrameMode frameMode)
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
	pFrameImpl->_show();
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
		static_cast<XAP_App *>(m_pApp)->forgetFrame(pClone);
		delete pClone;
	}

	return NULL;
}


void AP_CocoaFrame::_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 /*yrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	
	AP_CocoaFrame * pCocoaFrame = static_cast<AP_CocoaFrame *>(pData);
	AV_View * pView = pCocoaFrame->getCurrentView();
	
	// we've been notified (via sendVerticalScrollEvent()) of a scroll (probably
	// a keyboard motion).  push the new values into the scrollbar widgets
	// (with clamping).  then cause the view to scroll.
	
	float yoffNew = (float)yoff;
	float yoffMax = 0;
//TODO	float yoffMax = pCocoaFrame->m_pVadj->upper - pCocoaFrame->m_pVadj->page_size;
	if (yoffMax <= 0)
		yoffNew = 0;
	else if (yoffNew > yoffMax)
		yoffNew = yoffMax;
//TODO	gtk_adjustment_set_value(GTK_ADJUSTMENT(pCocoaFrame->m_pVadj),yoffNew);
	
	pView->setYScrollOffset((UT_sint32)yoffNew);
}

void AP_CocoaFrame::_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 /*xrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	
	AP_CocoaFrame * pCocoaFrame = static_cast<AP_CocoaFrame *>(pData);
	AV_View * pView = pCocoaFrame->getCurrentView();
	
	// we've been notified (via sendScrollEvent()) of a scroll (probably
	// a keyboard motion).  push the new values into the scrollbar widgets
	// (with clamping).  then cause the view to scroll.

	float xoffNew = (float)xoff;
	float xoffMax = 0;
//TODO	float xoffMax = pCocoaFrame->m_pHadj->upper - pCocoaFrame->m_pHadj->page_size;
	if (xoffMax <= 0)
		xoffNew = 0;
	else if (xoffNew > xoffMax)
		xoffNew = xoffMax;
//TODO	gtk_adjustment_set_value(GTK_ADJUSTMENT(pCocoaFrame->m_pHadj),xoffNew);
	pView->setXScrollOffset((UT_sint32)xoffNew);
}



void AP_CocoaFrame::translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y)
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
	UT_ASSERT (UT_NOT_IMPLEMENTED);
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);
		
	AP_CocoaTopRuler * pCocoaTopRuler = NULL;

	UT_DEBUGMSG(("AP_CocoaFrame::toggleTopRuler %d, %d\n", 
		     bRulerOn, pFrameData->m_pTopRuler));
	UT_ASSERT (UT_TODO);
#if 0
	if ( bRulerOn )
	{
		AP_TopRuler * pTop = pFrameData->m_pTopRuler;
		if(pTop)
		{
			delete pTop;
		}

		pCocoaTopRuler = new AP_CocoaTopRuler(this);
		UT_ASSERT(pCocoaTopRuler);
		m_topRuler = pCocoaTopRuler->createWidget();

		// get the width from the left ruler and stuff it into the 
		// top ruler.

		if (((AP_FrameData*)m_pData)->m_pLeftRuler)
		  pCocoaTopRuler->setOffsetLeftRuler(((AP_FrameData*)m_pData)->m_pLeftRuler->getWidth());
		else
		  pCocoaTopRuler->setOffsetLeftRuler(0);

		// attach everything	
		gtk_table_attach(GTK_TABLE(m_innertable), m_topRuler, 0, 2, 0,
				 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_FILL),
				 0, 0);
		FV_View * pView = static_cast<FV_View *>(m_pView);
		UT_uint32 iZoom = pView->getGraphics()->getZoomPercentage();
		static_cast<AP_TopRuler *>(pUnixTopRuler)->setView(m_pView,iZoom);

	}
	else
	{
		// delete the actual widgets
		g_object_destroy( G_OBJECT(m_topRuler) );
		DELETEP(((AP_FrameData*)m_pData)->m_pTopRuler);
		m_topRuler = NULL;
	        static_cast<FV_View *>(m_pView)->setTopRuler(NULL);
	}
#endif
	((AP_FrameData*)m_pData)->m_pTopRuler = pCocoaTopRuler;
}

void AP_CocoaFrame::toggleLeftRuler(bool bRulerOn)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);

	//AP_CocoaLeftRuler * pCocoaLeftRuler = NULL;

	UT_DEBUGMSG(("AP_CocoaFrame::toggleLeftRuler %d, %d\n", 
		     bRulerOn, pFrameData->m_pLeftRuler));
	UT_ASSERT (UT_TODO);

#if 0
	if (bRulerOn)
	  {
	    if (m_leftRuler) {
	      // there is already a ruler. Just return
	      return;
	    }
		pCocoaLeftRuler = new AP_CocoaLeftRuler(this);
		UT_ASSERT(pCocoaLeftRuler);
		m_leftRuler = pCocoaLeftRuler->createWidget();

		gtk_table_attach(GTK_TABLE(m_innertable), m_leftRuler, 0, 1, 1, 2,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 0,0);
		pCocoaLeftRuler->setView(m_pView);
		setYScrollRange ();
	  }
	else
	  {
	    if (m_leftRuler && GTK_IS_OBJECT(m_leftRuler))
		g_object_destroy( G_OBJECT(m_leftRuler) );
	    
	    DELETEP(((AP_FrameData*)m_pData)->m_pLeftRuler);
	    m_leftRuler = NULL;
	  }

	((AP_FrameData*)m_pData)->m_pLeftRuler = pCocoaLeftRuler;
#endif
}

void AP_CocoaFrame::toggleRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);

	toggleTopRuler(bRulerOn);
	toggleLeftRuler(bRulerOn && (pFrameData->m_pViewMode == VIEW_PRINT));
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

void AP_CocoaFrame::_setViewFocus(AV_View *pView)
{
	AP_CocoaFrameImpl * pFrameImpl = static_cast<AP_CocoaFrameImpl *>(getFrameImpl());
#warning not implemented
//	bool bFocus=GPOINTER_TO_INT(g_object_get_data(G_OBJECT(pFrameImpl->getTopLevelWindow()),
//						 "toplevelWindowFocus"));
//	pView->setFocus(bFocus && (gtk_grab_get_current()==NULL || gtk_grab_get_current()==pFrameImpl->getTopLevelWindow()) ? AV_FOCUS_HERE : !bFocus && gtk_grab_get_current()!=NULL && isTransientWindow(GTK_WINDOW(gtk_grab_get_current()),GTK_WINDOW(pFrameImpl->getTopLevelWindow())) ?  AV_FOCUS_NEARBY : AV_FOCUS_NONE);
}

void AP_CocoaFrame::_bindToolbars(AV_View *pView)
{
	static_cast<AP_CocoaFrameImpl *>(getFrameImpl())->_bindToolbars(pView);
}

