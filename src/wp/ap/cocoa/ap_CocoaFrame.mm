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

#if 0
#ifdef ABISOURCE_LICENSED_TRADEMARKS
#include "abiword_48_tm.xpm"
#else
#include "abiword_48.xpm"
#endif
#endif

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/
AP_CocoaFrameHelper::AP_CocoaFrameHelper(AP_CocoaFrame *pCocoaFrame, XAP_CocoaApp *pCocoaApp)
	: XAP_CocoaFrameHelper (pCocoaFrame, pCocoaApp)
{
	m_hScrollbar = NULL;
	m_vScrollbar = NULL;
	m_docAreaGRView = NULL;
}


void AP_CocoaFrameHelper::_createDocView(GR_CocoaGraphics* &pG)
{
	NSView*		docArea = [_getController() getMainView];
	NSArray*	docAreaSubviews;
	
	docAreaSubviews = [docArea subviews];
	if ([docAreaSubviews count] != 0) {
		NSEnumerator *enumerator = [docAreaSubviews objectEnumerator];
		NSView* aSubview;
	
		while (aSubview = [enumerator nextObject]) {
			[aSubview removeFromSuperviewWithoutNeedingDisplay];
		}
		
		m_hScrollbar = NULL;
		m_vScrollbar = NULL;
		m_docAreaGRView = NULL;
	}
		NSRect frame = [docArea bounds];
	NSRect controlFrame;
	
	/* vertical scrollbar */
	controlFrame.origin.y = [NSScroller scrollerWidth];
	controlFrame.size.width = [NSScroller scrollerWidth];
	controlFrame.size.height = frame.size.height - controlFrame.origin.y;
	controlFrame.origin.x = frame.size.width - controlFrame.size.width;
	m_vScrollbar = [[NSScroller alloc] initWithFrame:controlFrame];
	[docArea addSubview:m_vScrollbar];
	[m_vScrollbar setAutoresizingMask:(NSViewMinXMargin |  NSViewHeightSizable)];
	[m_vScrollbar release];
	
	/* horizontal scrollbar */
	controlFrame.origin.x = 0;
	controlFrame.origin.y = 0;
	controlFrame.size.height = [NSScroller scrollerWidth];
	controlFrame.size.width = frame.size.width - controlFrame.size.height;
	m_hScrollbar = [[NSScroller alloc] initWithFrame:controlFrame];
	[docArea addSubview:m_hScrollbar];
	[m_hScrollbar setAutoresizingMask:(NSViewMaxYMargin |  NSViewWidthSizable)];
	[m_hScrollbar release];

	/* doc view */
	controlFrame.origin.x = 0;
	controlFrame.origin.y = [NSScroller scrollerWidth];
	controlFrame.size.height = frame.size.height - controlFrame.origin.y;
	controlFrame.size.width = frame.size.width - [NSScroller scrollerWidth];
	m_docAreaGRView = [[XAP_CocoaNSView alloc] initWith:m_pFrame andFrame:controlFrame];
	[docArea addSubview:m_docAreaGRView];
	[m_docAreaGRView setAutoresizingMask:(NSViewHeightSizable | NSViewWidthSizable)];
	[m_docAreaGRView release];

	
	pG = new GR_CocoaGraphics(m_docAreaGRView, /*fontManager,*/ m_pFrame->getApp());
	static_cast<GR_CocoaGraphics *>(pG)->_setUpdateCallback (&_graphicsUpdateCB, (void *)this);
	

}


void AP_CocoaFrameHelper::_bindToolbars(AV_View * pView)
{
	int nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (int k = 0; k < nrToolbars; k++)
	{
		// TODO Toolbars are a frame-level item, but a view-listener is
		// TODO a view-level item.  I've bound the toolbar-view-listeners
		// TODO to the current view within this frame and have code in the
		// TODO toolbar to allow the view-listener to be rebound to a different
		// TODO view.  in the future, when we have support for multiple views
		// TODO in the frame (think splitter windows), we will need to have
		// TODO a loop like this to help change the focus when the current
		// TODO view changes.
		
		EV_CocoaToolbar * pCocoaToolbar = (EV_CocoaToolbar *)m_vecToolbars.getNthItem(k);
		pCocoaToolbar->bindListenerToView(pView);
	}
}

// Does the initial show/hide of statusbar (based on the user prefs).
// Idem.
void AP_CocoaFrameHelper::_showOrHideStatusbar()
{
	bool bShowStatusBar = static_cast<AP_FrameData*> (m_pFrame->getFrameData())->m_bShowStatusBar && false;
	static_cast<AP_CocoaFrame *>(m_pFrame)->toggleStatusBar(bShowStatusBar);
}

// Does the initial show/hide of toolbars (based on the user prefs).
// This is needed because toggleBar is called only when the user
// (un)checks the show {Stantandard,Format,Extra} toolbar checkbox,
// and thus we have to manually call this function at startup.
void AP_CocoaFrameHelper::_showOrHideToolbars()
{
	bool *bShowBar = static_cast<AP_FrameData*> (m_pFrame->getFrameData())->m_bShowBar;
	UT_uint32 cnt = m_vecToolbarLayoutNames.getItemCount();

	for (UT_uint32 i = 0; i < cnt; i++)
	{
		// TODO: The two next lines are here to bind the EV_Toolbar to the
		// AP_FrameData, but their correct place are next to the toolbar creation (JCA)
		EV_CocoaToolbar * pCocoaToolbar = static_cast<EV_CocoaToolbar *> (m_vecToolbars.getNthItem(i));
		static_cast<AP_FrameData*> (m_pFrame->getFrameData())->m_pToolbar[i] = pCocoaToolbar;
		static_cast<AP_CocoaFrame *>(m_pFrame)->toggleBar(i, bShowBar[i]);
	}
}


/*!
 * Refills the framedata class with pointers to the current toolbars. We 
 * need to do this after a toolbar icon and been dragged and dropped.
 */
void AP_CocoaFrameHelper::_refillToolbarsInFrameData(void)
{
	UT_uint32 cnt = m_vecToolbarLayoutNames.getItemCount();

	for (UT_uint32 i = 0; i < cnt; i++)
	{
		EV_CocoaToolbar * pCocoaToolbar = static_cast<EV_CocoaToolbar *> (m_vecToolbars.getNthItem(i));
		static_cast<AP_FrameData*> (m_pFrame->getFrameData())->m_pToolbar[i] = pCocoaToolbar;
	}
}

void AP_CocoaFrameHelper::_createDocumentWindow()
{
	AP_FrameData* pData = static_cast<AP_FrameData*> (m_pFrame->getFrameData());
	bool bShowRulers = pData->m_bShowRuler;

	// create the rulers
	AP_CocoaTopRuler * pCocoaTopRuler = NULL;
	AP_CocoaLeftRuler * pCocoaLeftRuler = NULL;

	if ( bShowRulers )
	{
		pCocoaTopRuler = new AP_CocoaTopRuler(m_pFrame);
		UT_ASSERT(pCocoaTopRuler);
		pCocoaTopRuler->createWidget();
		
		if (pData->m_pViewMode == VIEW_PRINT) {
		    pCocoaLeftRuler = new AP_CocoaLeftRuler(m_pFrame);
		    UT_ASSERT(pCocoaLeftRuler);
		    pCocoaLeftRuler->createWidget();

		    // get the width from the left ruler and stuff it into the top ruler.
		    pCocoaTopRuler->setOffsetLeftRuler(pCocoaLeftRuler->getWidth());
		}
		else {
//		    m_leftRuler = NULL;
		    pCocoaTopRuler->setOffsetLeftRuler(0);
		}
	}

	pData->m_pTopRuler = pCocoaTopRuler;
	pData->m_pLeftRuler = pCocoaLeftRuler;

	// TODO check what really remains to be done.
//	UT_ASSERT (UT_TODO);
#if 0
//	g_signal_connect(G_OBJECT(m_pHadj), "value_changed", G_CALLBACK(_fe::hScrollChanged), NULL);
//	g_signal_connect(G_OBJECT(m_pVadj), "value_changed", G_CALLBACK(_fe::vScrollChanged), NULL);

	g_signal_connect(G_OBJECT(m_dArea), "expose_event",
					   G_CALLBACK(_fe::expose), NULL);
  
	g_signal_connect(G_OBJECT(m_dArea), "button_press_event",
					   G_CALLBACK(_fe::button_press_event), NULL);

	g_signal_connect(G_OBJECT(m_dArea), "button_release_event",
					   G_CALLBACK(_fe::button_release_event), NULL);

	g_signal_connect(G_OBJECT(m_dArea), "motion_notify_event",
					   G_CALLBACK(_fe::motion_notify_event), NULL);
  
	g_signal_connect(G_OBJECT(m_dArea), "configure_event",
					   G_CALLBACK(_fe::configure_event), NULL);
#endif
}

void AP_CocoaFrameHelper::_createStatusBarWindow(XAP_CocoaNSStatusBar * statusBar)
{
	UT_DEBUGMSG (("AP_CocoaFrame::_createStatusBarWindow ()\n"));
	// TODO: pass the NSView instead of the whole frame
	AP_CocoaStatusBar * pCocoaStatusBar = new AP_CocoaStatusBar(m_pFrame);
	UT_ASSERT(pCocoaStatusBar);

	((AP_FrameData *)m_pFrame->getFrameData())->m_pStatusBar = pCocoaStatusBar;
	
//	GtkWidget * w = pCocoaStatusBar->createWidget();

	//return [_getController() getStatusBar];
}

void AP_CocoaFrameHelper::_setWindowIcon()
{
	// this is NOT needed. Just need to the the title.
}

NSString *	AP_CocoaFrameHelper::_getNibName ()
{
	return @"ap_CocoaFrame";
}

/*!
	Create and intialize the controller.
 */
XAP_CocoaFrameController *AP_CocoaFrameHelper::_createController()
{
	UT_DEBUGMSG (("AP_CocoaFrame::_createController()\n"));
	return [AP_CocoaFrameController createFrom:m_pFrame];
}



bool AP_CocoaFrameHelper::_graphicsUpdateCB(NSRect * aRect, GR_CocoaGraphics *pG, void* param)
{
	// a static function
	AP_CocoaFrame * pCocoaFrame = (AP_CocoaFrame *)param;
	if (!pCocoaFrame)
		return false;

	UT_Rect rClip;
	rClip.left = aRect->origin.x;
	rClip.top = aRect->origin.y;
	rClip.width = aRect->size.width;
	rClip.height = aRect->size.height;
	xxx_UT_DEBUGMSG(("Cocoa in frame expose painting area:  left=%d, top=%d, width=%d, height=%d\n", rClip.left, rClip.top, rClip.width, rClip.height));
	if(pG != NULL)
		pG->doRepaint(&rClip);
	else
		return false;
	return true;
}

void AP_CocoaFrame::setZoomPercentage(UT_uint32 iZoom)
{
	_showDocument(iZoom);
}

UT_uint32 AP_CocoaFrame::getZoomPercentage(void)
{
	return ((AP_FrameData*)m_pData)->m_pG->getZoomPercentage();
}

UT_Error AP_CocoaFrame::_showDocument(UT_uint32 iZoom)
{
	if (!m_pDoc)
	{
		UT_DEBUGMSG(("Can't show a non-existent document\n"));
		return UT_IE_FILENOTFOUND;
	}

	if (!((AP_FrameData*)m_pData))
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return UT_IE_IMPORTERROR;
	}

	GR_CocoaGraphics * pG = NULL;
	FL_DocLayout * pDocLayout = NULL;
	AV_View * pView = NULL;
	AV_ScrollObj * pScrollObj = NULL;
	ap_ViewListener * pViewListener = NULL;
	AD_Document * pOldDoc = NULL;
	ap_Scrollbar_ViewListener * pScrollbarViewListener = NULL;
	AV_ListenerId lid;
	AV_ListenerId lidScrollbarViewListener;
	UT_uint32 point = 0;
	
	NSRect rect;

//	XAP_CocoaFontManager * fontManager = ((XAP_CocoaApp *) getApp())->getFontManager();
	static_cast<AP_CocoaFrameHelper*>(m_pFrameHelper)->_createDocView(pG);
	ENSUREP(pG);
	pG->setZoomPercentage(iZoom);
	pDocLayout = new FL_DocLayout(static_cast<PD_Document *>(m_pDoc), pG);
	ENSUREP(pDocLayout);
	  
	if (m_pView != NULL)
	{
		point = ((FV_View *) m_pView)->getPoint();
	}

	pView = new FV_View(getApp(), this, pDocLayout);
	ENSUREP(pView);

//	bFocus=GPOINTER_TO_INT(g_object_get_data(G_OBJECT(m_wTopLevelWindow),"toplevelWindowFocus"));
#if 0
	pView->setFocus(bFocus && (gtk_grab_get_current()==NULL || gtk_grab_get_current()==m_wTopLevelWindow) ? AV_FOCUS_HERE : !bFocus && gtk_grab_get_current()!=NULL && isTransientWindow(GTK_WINDOW(gtk_grab_get_current()),GTK_WINDOW(m_wTopLevelWindow)) ?  AV_FOCUS_NEARBY : AV_FOCUS_NONE);
#endif
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
	// ON UNIX ONLY: we subclass this with ap_CocoaViewListener
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
	ENSUREP(pScrollObj);
	pViewListener = new ap_CocoaViewListener(this);
	ENSUREP(pViewListener);
	pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
	ENSUREP(pScrollbarViewListener);

	if (!pView->addListener(static_cast<AV_Listener *>(pViewListener),&lid))
		goto Cleanup;

	if (!pView->addListener(static_cast<AV_Listener *>(pScrollbarViewListener),
							&lidScrollbarViewListener))
		goto Cleanup;


	/****************************************************************
	*****************************************************************
	** If we reach this point, everything for the new document has
	** been created.  We can now safely replace the various fields
	** within the structure.  Nothing below this point should fail.
	*****************************************************************
	****************************************************************/
	
	// switch to new view, cleaning up previous settings
	if (((AP_FrameData*)m_pData)->m_pDocLayout)
	{
		pOldDoc = ((AP_FrameData*)m_pData)->m_pDocLayout->getDocument();
	}

	REPLACEP(((AP_FrameData*)m_pData)->m_pG, pG);
	REPLACEP(((AP_FrameData*)m_pData)->m_pDocLayout, pDocLayout);
	if (pOldDoc != m_pDoc)
	{
		UNREFP(pOldDoc);
	}
	REPLACEP(m_pView, pView);
        if(getApp()->getViewSelection())
	       getApp()->setViewSelection(pView);
	REPLACEP(m_pScrollObj, pScrollObj);
	REPLACEP(m_pViewListener, pViewListener);
	m_lid = lid;
	REPLACEP(m_pScrollbarViewListener,pScrollbarViewListener);
	m_lidScrollbarViewListener = lidScrollbarViewListener;

	m_pView->addScrollListener(m_pScrollObj);

	// Associate the new view with the existing TopRuler, LeftRuler.
	// Because of the binding to the actual on-screen widgets we do
	// not destroy and recreate the TopRuler, LeftRuler when we change
	// views, like we do for all the other objects.  We also do not
	// allocate the TopRuler, LeftRuler  here; that is done as the
	// frame is created.
	if (((AP_FrameData*)m_pData)->m_bShowRuler)	{
		if ( ((AP_FrameData*)m_pData)->m_pTopRuler ) {
			((AP_FrameData*)m_pData)->m_pTopRuler->setView(pView, iZoom);
		}
		if ( ((AP_FrameData*)m_pData)->m_pLeftRuler ) {
			((AP_FrameData*)m_pData)->m_pLeftRuler->setView(pView, iZoom);
		}
	}

	if ( ((AP_FrameData*)m_pData)->m_pStatusBar ) {
		((AP_FrameData*)m_pData)->m_pStatusBar->setView(pView);
	}
    ((FV_View *) m_pView)->setShowPara(((AP_FrameData*)m_pData)->m_bShowPara);

	pView->setInsertMode(((AP_FrameData*)m_pData)->m_bInsertMode);

	rect = [[static_cast<XAP_CocoaFrameHelper*>(m_pFrameHelper)->_getController() window] frame];
	m_pView->setWindowSize(rect.size.width , rect.size.height);
	setXScrollRange();
	setYScrollRange();
	updateTitle();

#if 1
	/*
	  UPDATE:  this code is back, but I'm leaving these comments as
	  an audit trail.  See bug 99.  This only happens when loading
	  a document into an empty window -- the case where a frame gets
	  reused.  TODO consider putting an expose into ap_EditMethods.cpp
	  instead of a draw() here.
	*/
	
	/*
	  I've removed this once again.  (Eric)  I replaced it with a call
	  to draw() which is now in the configure event handler in the GTK
	  section of the code.  See me if this causes problems.
	*/
	pDocLayout->fillLayouts();
	if (point != 0) 
		((FV_View *) m_pView)->moveInsPtTo(point);
	m_pView->draw();
#endif	
	
	if ( ((AP_FrameData*)m_pData)->m_bShowRuler  ) 
	{
		if ( ((AP_FrameData*)m_pData)->m_pTopRuler )
			((AP_FrameData*)m_pData)->m_pTopRuler->draw(NULL);

		if ( ((AP_FrameData*)m_pData)->m_pLeftRuler )
			((AP_FrameData*)m_pData)->m_pLeftRuler->draw(NULL);
	}

	if(isStatusBarShown())
	{
		((AP_FrameData*)m_pData)->m_pStatusBar->notify(m_pView, AV_CHG_ALL);
	}
	if(m_pView)
	{
		m_pView->notifyListeners(AV_CHG_ALL);
		m_pView->focusChange(AV_FOCUS_HERE);
	}
	
	return UT_OK;

Cleanup:
	// clean up anything we created here
	DELETEP(pG);
	DELETEP(pDocLayout);
	DELETEP(pView);
	DELETEP(pViewListener);
	DELETEP(pScrollObj);
	DELETEP(pScrollbarViewListener);

	// change back to prior document
	UNREFP(m_pDoc);
	m_pDoc = ((AP_FrameData*)m_pData)->m_pDocLayout->getDocument();

	return UT_IE_ADDLISTENERERROR;
}

void AP_CocoaFrame::setXScrollRange(void)
{
	int width = ((AP_FrameData*)m_pData)->m_pDocLayout->getWidth();
	NSRect rect = [static_cast<AP_CocoaFrameHelper *>(m_pFrameHelper)->m_docAreaGRView frame];
	int windowWidth = rect.size.width;

	int newvalue = ((m_pView) ? m_pView->getXScrollOffset() : 0);
	int newmax = width - windowWidth; /* upper - page_size */
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
	NSRect rect = [static_cast<AP_CocoaFrameHelper *>(m_pFrameHelper)->m_docAreaGRView frame];
	int windowHeight = rect.size.width;

	int newvalue = ((m_pView) ? m_pView->getYScrollOffset() : 0);
	int newmax = height - windowHeight;	/* upper - page_size */
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
	: XAP_Frame (app)
{
	// TODO
	m_pData = NULL;
}

AP_CocoaFrame::AP_CocoaFrame(AP_CocoaFrame * f)
	: XAP_Frame(static_cast<XAP_Frame *>(f))
{
	// TODO
	m_pData = NULL;
}

AP_CocoaFrame::~AP_CocoaFrame()
{
	killFrameData();
}

bool AP_CocoaFrame::initialize()
{
	UT_DEBUGMSG(("AP_CocoaFrame::initialize\n"));
	if (!initFrameData())
		return false;

	if (!XAP_Frame::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings,
								   AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
								   AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet,
								   AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
								   AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet))
		return false;

	static_cast<AP_CocoaFrameHelper *>(m_pFrameHelper)->_createTopLevelWindow();
//	gtk_widget_show(m_wTopLevelWindow);
	if(getFrameMode() == XAP_NormalFrame)
	{
		// needs to be shown so that the following functions work
		// TODO: get rid of cursed flicker caused by initially
		// TODO: showing these and then hiding them (esp.
		// TODO: noticable in the gnome build with a toolbar disabled)
		static_cast<AP_CocoaFrameHelper *>(m_pFrameHelper)->_showOrHideToolbars();
		static_cast<AP_CocoaFrameHelper *>(m_pFrameHelper)->_showOrHideStatusbar();
	}
	return true;
}




/*****************************************************************/

bool AP_CocoaFrame::initFrameData()
{
	UT_ASSERT(!((AP_FrameData*)m_pData));

	AP_FrameData* pData = new AP_FrameData(static_cast<XAP_App *>(m_pApp));

	m_pData = (void*)pData;
	return (pData ? true : false);
}

void AP_CocoaFrame::killFrameData()
{
	AP_FrameData* pData = (AP_FrameData*) m_pData;
	DELETEP(pData);
	m_pData = NULL;
}

UT_Error AP_CocoaFrame::_loadDocument(const char * szFilename, IEFileType ieft,
				     bool createNew)
{
#ifdef DEBUG
	if (szFilename) {
		UT_DEBUGMSG(("DOM: trying to load %s (%d, %d)\n", szFilename, ieft, createNew));
	}
	else {
		UT_DEBUGMSG(("DOM: trying to load %s (%d, %d)\n", "(NULL)", ieft, createNew));
	}
#endif

	// are we replacing another document?
	if (m_pDoc)
	{
		// yep.  first make sure it's OK to discard it, 
		// TODO: query user if dirty...
	}

	// load a document into the current frame.
	// if no filename, create a new document.

	AD_Document * pNewDoc = new PD_Document(getApp());
	UT_ASSERT(pNewDoc);
	
	if (!szFilename || !*szFilename)
	{
		pNewDoc->newDocument();
		m_iUntitled = _getNextUntitledNumber();
		goto ReplaceDocument;
	}
	UT_Error errorCode;
	errorCode = pNewDoc->readFromFile(szFilename, ieft);
	if (!errorCode)
		goto ReplaceDocument;

	if (createNew)
	  {
	    // we have a file name but couldn't load it
	    pNewDoc->newDocument();

	    // here, we want to open a new document if it doesn't exist.
	    // errorCode could also take several other values, indicating
	    // that the document exists but for some reason we could not
	    // open it. in those cases, we do not wish to overwrite the
	    // existing documents, but instead open a new blank document. 
	    // this fixes bug 1668 - DAL
	    if ( UT_IE_FILENOTFOUND == errorCode )
	      errorCode = pNewDoc->saveAs(szFilename, ieft);
	  }
	if (!errorCode)
	  goto ReplaceDocument;
	
	UT_DEBUGMSG(("ap_Frame: could not open the file [%s]\n",szFilename));
	UNREFP(pNewDoc);
	return errorCode;

ReplaceDocument:
	getApp()->forgetClones(this);

	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = pNewDoc;
	return UT_OK;
}
	
UT_Error AP_CocoaFrame::_importDocument(const char * szFilename, int ieft,
									  bool markClean)
{
	UT_DEBUGMSG(("DOM: trying to import %s (%d, %d)\n", szFilename, ieft, markClean));

	// are we replacing another document?
	if (m_pDoc)
	{
		// yep.  first make sure it's OK to discard it, 
		// TODO: query user if dirty...
	}

	// load a document into the current frame.
	// if no filename, create a new document.

	AD_Document * pNewDoc = new PD_Document(getApp());
	UT_ASSERT(pNewDoc);

	if (!szFilename || !*szFilename)
	{
		pNewDoc->newDocument();
		goto ReplaceDocument;
	}
	UT_Error errorCode;
	errorCode = pNewDoc->importFile(szFilename, ieft, markClean);
	if (!errorCode)
		goto ReplaceDocument;

	UT_DEBUGMSG(("ap_Frame: could not open the file [%s]\n",szFilename));

	UNREFP(pNewDoc);
	return errorCode;

ReplaceDocument:
	getApp()->forgetClones(this);

	m_iUntitled = _getNextUntitledNumber();

	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = pNewDoc;
	return UT_OK;
}

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

XAP_Frame * AP_CocoaFrame::buildFrame(XAP_Frame * pF)
{
	AP_CocoaFrame * pClone = static_cast<AP_CocoaFrame *>(pF);
	UT_Error error = UT_OK;
	ENSUREP(pClone);

	if (!pClone->initialize())
		goto Cleanup;

	error = pClone->_showDocument();
	if (error)
		goto Cleanup;

	pClone->show();

	return pClone;

Cleanup:
	// clean up anything we created here
	if (pClone)
	{
		static_cast<XAP_App *>(m_pApp)->forgetFrame(pClone);
		delete pClone;
	}

	return NULL;
}

UT_Error AP_CocoaFrame::loadDocument(const char * szFilename, int ieft,
									bool createNew)
{
	bool bUpdateClones;
	UT_Vector vClones;
	XAP_App * pApp = getApp();

	bUpdateClones = (getViewNumber() > 0);
	if (bUpdateClones)
	{
		pApp->getClones(&vClones, this);
	}
	UT_Error errorCode;
	errorCode =  _loadDocument(szFilename, (IEFileType) ieft, createNew);
	if (errorCode)
	{
		// we could not load the document.
		// we cannot complain to the user here, we don't know
		// if the app is fully up yet.  we force our caller
		// to deal with the problem.
		return errorCode;
	}

	pApp->rememberFrame(this);
	if (bUpdateClones)
	{
		for (UT_uint32 i = 0; i < vClones.getItemCount(); i++)
		{
			AP_CocoaFrame * pFrame = (AP_CocoaFrame *) vClones.getNthItem(i);
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
				pApp->rememberFrame(pFrame, this);
			}
		}
	}

	return _showDocument();
}

UT_Error AP_CocoaFrame::loadDocument(const char * szFilename, int ieft)
{
  return loadDocument(szFilename, ieft, false);
}

UT_Error AP_CocoaFrame::importDocument(const char * szFilename, int ieft,
									  bool markClean)
{
	bool bUpdateClones;
	UT_Vector vClones;
	XAP_App * pApp = getApp();

	bUpdateClones = (getViewNumber() > 0);
	if (bUpdateClones)
	{
		pApp->getClones(&vClones, this);
	}
	UT_Error errorCode;
	errorCode =  _importDocument(szFilename, (IEFileType) ieft, markClean);
	if (errorCode)
	{
		return errorCode;
	}

	pApp->rememberFrame(this);
	if (bUpdateClones)
	{
		for (UT_uint32 i = 0; i < vClones.getItemCount(); i++)
		{
			AP_CocoaFrame * pFrame = (AP_CocoaFrame *) vClones.getNthItem(i);
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
				pApp->rememberFrame(pFrame, this);
			}
		}
	}

	return _showDocument();
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


UT_Error AP_CocoaFrame::_replaceDocument(AD_Document * pDoc)
{
	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = REFP(pDoc);

	return _showDocument();
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
		UT_ASSERT(!pFrameData->m_pTopRuler);

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

		pCocoaTopRuler->setView(m_pView);
	}
	else
	{
		// delete the actual widgets
		g_object_destroy( G_OBJECT(m_topRuler) );
		DELETEP(((AP_FrameData*)m_pData)->m_pTopRuler);
		m_topRuler = NULL;
	}
#endif
	((AP_FrameData*)m_pData)->m_pTopRuler = pCocoaTopRuler;
}

void AP_CocoaFrame::toggleLeftRuler(bool bRulerOn)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);

	AP_CocoaLeftRuler * pCocoaLeftRuler = NULL;

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


/* Objective-C section */

@implementation AP_CocoaFrameController
+ (XAP_CocoaFrameController*)createFrom:(XAP_Frame *)frame
{
	UT_DEBUGMSG (("Cocoa: @AP_CocoaFrameController createFrom:frame\n"));
	AP_CocoaFrameController *obj = [[AP_CocoaFrameController alloc] initWith:frame];
	return obj;
}

- (id)initWith:(XAP_Frame *)frame
{
	UT_DEBUGMSG (("Cocoa: @AP_CocoaFrameController initWith:frame\n"));
	return [super initWith:frame];
}


- (IBAction)rulerClick:(id)sender
{
}

- (XAP_CocoaNSView *)getVRuler
{
	return vRuler;
}

- (XAP_CocoaNSView *)getHRuler
{
	return hRuler;
}


@end
