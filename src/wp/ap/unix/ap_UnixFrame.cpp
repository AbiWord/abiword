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

#include <gtk/gtk.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "ap_FrameData.h"
#ifdef HAVE_GNOME
#include "xap_UnixGnomeFrame.h"
#else
#include "xap_UnixFrame.h"
#endif
#include "ev_UnixToolbar.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_UnixGraphics.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_UnixFrame.h"
#include "xap_UnixApp.h"
#include "ap_UnixTopRuler.h"
#include "ap_UnixLeftRuler.h"
#include "xap_UnixFontManager.h"
#include "ap_UnixStatusBar.h"
#include "ap_UnixViewListener.h"
#include"ut_dialogHelper.h"

#ifdef ABISOURCE_LICENSED_TRADEMARKS
#include "abiword_48_tm.xpm"
#else
#include "abiword_48.xpm"
#endif

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

void AP_UnixFrame::setZoomPercentage(UT_uint32 iZoom)
{
	_showDocument(iZoom);
}

UT_uint32 AP_UnixFrame::getZoomPercentage(void)
{
	return ((AP_FrameData*)m_pData)->m_pG->getZoomPercentage();
}

UT_Error AP_UnixFrame::_showDocument(UT_uint32 iZoom)
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

	GR_UnixGraphics * pG = NULL;
	FL_DocLayout * pDocLayout = NULL;
	AV_View * pView = NULL;
	AV_ScrollObj * pScrollObj = NULL;
	ap_ViewListener * pViewListener = NULL;
	AD_Document * pOldDoc = NULL;
	ap_Scrollbar_ViewListener * pScrollbarViewListener = NULL;
	AV_ListenerId lid;
	AV_ListenerId lidScrollbarViewListener;
	UT_uint32 nrToolbars;
	UT_uint32 point = 0;
	gboolean bFocus;
	XAP_UnixFontManager * fontManager = ((XAP_UnixApp *) getApp())->getFontManager();
	
	pG = new GR_UnixGraphics(m_dArea->window, fontManager, getApp());
	ENSUREP(pG);
	pG->setZoomPercentage(iZoom);
	
	pDocLayout = new FL_DocLayout(static_cast<PD_Document *>(m_pDoc), pG);
	ENSUREP(pDocLayout);
  
//	pDocLayout->formatAll();

	pView = new FV_View(getApp(), this, pDocLayout);
	if (m_pView != NULL)
	{
		point = ((FV_View *) m_pView)->getPoint();
	}
	ENSUREP(pView);
	bFocus=GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(m_wTopLevelWindow),"toplevelWindowFocus"));
	pView->setFocus(bFocus && (gtk_grab_get_current()==NULL || gtk_grab_get_current()==m_wTopLevelWindow) ? AV_FOCUS_HERE : !bFocus && gtk_grab_get_current()!=NULL && isTransientWindow(GTK_WINDOW(gtk_grab_get_current()),GTK_WINDOW(m_wTopLevelWindow)) ?  AV_FOCUS_NEARBY : AV_FOCUS_NONE);
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
	ENSUREP(pScrollObj);
	pViewListener = new ap_UnixViewListener(this);
	ENSUREP(pViewListener);
	pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
	ENSUREP(pScrollbarViewListener);

	if (!pView->addListener(static_cast<AV_Listener *>(pViewListener),&lid))
		goto Cleanup;

	if (!pView->addListener(static_cast<AV_Listener *>(pScrollbarViewListener),
							&lidScrollbarViewListener))
		goto Cleanup;

	nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (UT_uint32 k=0; k < nrToolbars; k++)
	{
		// TODO Toolbars are a frame-level item, but a view-listener is
		// TODO a view-level item.  I've bound the toolbar-view-listeners
		// TODO to the current view within this frame and have code in the
		// TODO toolbar to allow the view-listener to be rebound to a different
		// TODO view.  in the future, when we have support for multiple views
		// TODO in the frame (think splitter windows), we will need to have
		// TODO a loop like this to help change the focus when the current
		// TODO view changes.
		
		EV_UnixToolbar * pUnixToolbar = (EV_UnixToolbar *)m_vecToolbars.getNthItem(k);
		pUnixToolbar->bindListenerToView(pView);
	}

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
	if ( ((AP_FrameData*)m_pData)->m_bShowRuler )
	{
	  if ( ((AP_FrameData*)m_pData)->m_pTopRuler )
	    ((AP_FrameData*)m_pData)->m_pTopRuler->setView(pView, iZoom);
	  if ( ((AP_FrameData*)m_pData)->m_pLeftRuler )
		((AP_FrameData*)m_pData)->m_pLeftRuler->setView(pView, iZoom);
	}

	if ( ((AP_FrameData*)m_pData)->m_pStatusBar )
	  ((AP_FrameData*)m_pData)->m_pStatusBar->setView(pView);

	pView->setInsertMode(((AP_FrameData*)m_pData)->m_bInsertMode);
    ((FV_View *) m_pView)->setShowPara(((AP_FrameData*)m_pData)->m_bShowPara);
	
	m_pView->setWindowSize(GTK_WIDGET(m_dArea)->allocation.width,
						   GTK_WIDGET(m_dArea)->allocation.height);
	setXScrollRange();
	setYScrollRange();
	updateTitle();

	if (point != 0)
		((FV_View *) m_pView)->moveInsPtTo(point);
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
	m_pView->draw();
#endif	
	
	if ( ((AP_FrameData*)m_pData)->m_bShowRuler  ) 
	{
		if ( ((AP_FrameData*)m_pData)->m_pTopRuler )
			((AP_FrameData*)m_pData)->m_pTopRuler->draw(NULL);

		if ( ((AP_FrameData*)m_pData)->m_pLeftRuler )
			((AP_FrameData*)m_pData)->m_pLeftRuler->draw(NULL);
	}

	((AP_FrameData*)m_pData)->m_pStatusBar->draw();
	
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

void AP_UnixFrame::setXScrollRange(void)
{
	int width = ((AP_FrameData*)m_pData)->m_pDocLayout->getWidth();
	int windowWidth = GTK_WIDGET(m_dArea)->allocation.width;

	int newvalue = ((m_pView) ? m_pView->getXScrollOffset() : 0);
	int newmax = width - windowWidth; /* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;

	bool bDifferentPosition = (newvalue != (int)m_pHadj->value);
	bool bDifferentLimits = ((width-windowWidth) != (int)(m_pHadj->upper-m_pHadj->page_size));
	
	m_pHadj->value = newvalue;
	m_pHadj->lower = 0.0;
	m_pHadj->upper = (gfloat) width;
	m_pHadj->step_increment = 20.0;
	m_pHadj->page_increment = (gfloat) windowWidth;
	m_pHadj->page_size = (gfloat) windowWidth;
	gtk_signal_emit_by_name(GTK_OBJECT(m_pHadj), "changed");

	if (m_pView && (bDifferentPosition || bDifferentLimits))
		m_pView->sendHorizontalScrollEvent(newvalue, (int)(m_pHadj->upper-m_pHadj->page_size));
}

void AP_UnixFrame::setYScrollRange(void)
{
	int height = ((AP_FrameData*)m_pData)->m_pDocLayout->getHeight();
	int windowHeight = GTK_WIDGET(m_dArea)->allocation.height;

	int newvalue = ((m_pView) ? m_pView->getYScrollOffset() : 0);
	int newmax = height - windowHeight;	/* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;

	bool bDifferentPosition = (newvalue != (int)m_pVadj->value);
	bool bDifferentLimits ((height-windowHeight) != (int)(m_pVadj->upper-m_pVadj->page_size));
	
	m_pVadj->value = newvalue;
	m_pVadj->lower = 0.0;
	m_pVadj->upper = (gfloat) height;
	m_pVadj->step_increment = 20.0;
	m_pVadj->page_increment = (gfloat) windowHeight;
	m_pVadj->page_size = (gfloat) windowHeight;
	gtk_signal_emit_by_name(GTK_OBJECT(m_pVadj), "changed");

	if (m_pView && (bDifferentPosition || bDifferentLimits))
		m_pView->sendVerticalScrollEvent(newvalue, (int)(m_pVadj->upper-m_pVadj->page_size));
}


AP_UnixFrame::AP_UnixFrame(XAP_UnixApp * app)
	: XAP_UNIXBASEFRAME(app)
{
	// TODO
	m_pData = NULL;
}

AP_UnixFrame::AP_UnixFrame(AP_UnixFrame * f)
	: XAP_UNIXBASEFRAME(static_cast<XAP_UNIXBASEFRAME *>(f))
{
	// TODO
	m_pData = NULL;
}

AP_UnixFrame::~AP_UnixFrame(void)
{
	killFrameData();
}

bool AP_UnixFrame::initialize(void)
{
	UT_DEBUGMSG(("AP_UnixFrame::initialize"));
	if (!initFrameData())
		return false;

	if (!XAP_UNIXBASEFRAME::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings,
								   AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
								   AP_PREF_KEY_MenuLabelSet, AP_PREF_DEFAULT_MenuLabelSet,
								   AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
								   AP_PREF_KEY_ToolbarLabelSet, AP_PREF_DEFAULT_ToolbarLabelSet))
		return false;

	_createTopLevelWindow();
	_showOrHideToolbars();
	_showOrHideStatusbar();
	gtk_widget_show(m_wTopLevelWindow);

	return true;
}

// Does the initial show/hide of toolbars (based on the user prefs).
// This is needed because toggleBar is called only when the user
// (un)checks the show {Stantandard,Format,Extra} toolbar checkbox,
// and thus we have to manually call this function at startup.
void AP_UnixFrame::_showOrHideToolbars(void)
{
	bool *bShowBar = static_cast<AP_FrameData*> (m_pData)->m_bShowBar;

	for (UT_uint32 i = 0; i < m_vecToolbarLayoutNames.getItemCount(); i++)
	{
		// TODO: The two next lines are here to bind the EV_Toolbar to the
		// AP_FrameData, but their correct place are next to the toolbar creation (JCA)
		EV_UnixToolbar * pUnixToolbar = static_cast<EV_UnixToolbar *> (m_vecToolbars.getNthItem(i));
		static_cast<AP_FrameData*> (m_pData)->m_pToolbar[i] = pUnixToolbar;
		toggleBar(i, bShowBar[i]);
	}
}

// Does the initial show/hide of toolbars (based on the user prefs).
// Idem.
void AP_UnixFrame::_showOrHideStatusbar(void)
{
        //bool bShowStatusBar = static_cast<AP_FrameData*> (m_pData)->m_bShowStatusBar;
	// I don't know why it doesn't work...
	//	toggleStatusBar(bShowStatusBar);
}

/*****************************************************************/

bool AP_UnixFrame::initFrameData(void)
{
	UT_ASSERT(!((AP_FrameData*)m_pData));

	AP_FrameData* pData = new AP_FrameData(m_pUnixApp);

	m_pData = (void*)pData;
	return (pData ? true : false);
}

void AP_UnixFrame::killFrameData(void)
{
	AP_FrameData* pData = (AP_FrameData*) m_pData;
	DELETEP(pData);
	m_pData = NULL;
}

UT_Error AP_UnixFrame::_loadDocument(const char * szFilename, IEFileType ieft)
{
	// are we replacing another document?
	if (m_pDoc)
	{
		// yep.  first make sure it's OK to discard it, 
		// TODO: query user if dirty...
	}

	// load a document into the current frame.
	// if no filename, create a new document.

	AD_Document * pNewDoc = new PD_Document();
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
	
	UT_DEBUGMSG(("ap_Frame: could not open the file [%s]\n",szFilename));
	UNREFP(pNewDoc);
	return errorCode;

ReplaceDocument:
	getApp()->forgetClones(this);

	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = pNewDoc;
	return UT_OK;
}
	
XAP_Frame * AP_UnixFrame::cloneFrame(void)
{
	AP_UnixFrame * pClone = new AP_UnixFrame(this);
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
		m_pUnixApp->forgetFrame(pClone);
		delete pClone;
	}

	return NULL;
}

UT_Error AP_UnixFrame::loadDocument(const char * szFilename, int ieft)
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
	errorCode =  _loadDocument(szFilename, (IEFileType) ieft);
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
			AP_UnixFrame * pFrame = (AP_UnixFrame *) vClones.getNthItem(i);
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
				pApp->rememberFrame(pFrame, this);
			}
		}
	}

	return _showDocument();
}

void AP_UnixFrame::_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 /*yrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	
	AP_UnixFrame * pUnixFrame = static_cast<AP_UnixFrame *>(pData);
	AV_View * pView = pUnixFrame->getCurrentView();
	
	// we've been notified (via sendVerticalScrollEvent()) of a scroll (probably
	// a keyboard motion).  push the new values into the scrollbar widgets
	// (with clamping).  then cause the view to scroll.
	
	gfloat yoffNew = (gfloat)yoff;
	gfloat yoffMax = pUnixFrame->m_pVadj->upper - pUnixFrame->m_pVadj->page_size;
	if (yoffMax <= 0)
		yoffNew = 0;
	else if (yoffNew > yoffMax)
		yoffNew = yoffMax;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(pUnixFrame->m_pVadj),yoffNew);
	pView->setYScrollOffset((UT_sint32)yoffNew);
}

void AP_UnixFrame::_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 /*xrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	
	AP_UnixFrame * pUnixFrame = static_cast<AP_UnixFrame *>(pData);
	AV_View * pView = pUnixFrame->getCurrentView();
	
	// we've been notified (via sendScrollEvent()) of a scroll (probably
	// a keyboard motion).  push the new values into the scrollbar widgets
	// (with clamping).  then cause the view to scroll.

	gfloat xoffNew = (gfloat)xoff;
	gfloat xoffMax = pUnixFrame->m_pHadj->upper - pUnixFrame->m_pHadj->page_size;
	if (xoffMax <= 0)
		xoffNew = 0;
	else if (xoffNew > xoffMax)
		xoffNew = xoffMax;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(pUnixFrame->m_pHadj),xoffNew);
	pView->setXScrollOffset((UT_sint32)xoffNew);
}

GtkWidget * AP_UnixFrame::_createDocumentWindow(void)
{
	bool bShowRulers = static_cast<AP_FrameData*> (m_pData)->m_bShowRuler;

	// create the rulers
	AP_UnixTopRuler * pUnixTopRuler = NULL;
	AP_UnixLeftRuler * pUnixLeftRuler = NULL;
	if ( bShowRulers )
	{
		pUnixTopRuler = new AP_UnixTopRuler(this);
		UT_ASSERT(pUnixTopRuler);
		m_topRuler = pUnixTopRuler->createWidget();
		
		pUnixLeftRuler = new AP_UnixLeftRuler(this);
		UT_ASSERT(pUnixLeftRuler);
		m_leftRuler = pUnixLeftRuler->createWidget();
		
		// get the width from the left ruler and stuff it into the top ruler.
		pUnixTopRuler->setOffsetLeftRuler(pUnixLeftRuler->getWidth());
	}
	else
	{
		m_topRuler = NULL;
		m_leftRuler = NULL;
	}
	((AP_FrameData*)m_pData)->m_pTopRuler = pUnixTopRuler;
	((AP_FrameData*)m_pData)->m_pLeftRuler = pUnixLeftRuler;

	// set up for scroll bars.
	m_pHadj = (GtkAdjustment*) gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	gtk_object_set_user_data(GTK_OBJECT(m_pHadj),this);
	m_hScroll = gtk_hscrollbar_new(m_pHadj);
	gtk_object_set_user_data(GTK_OBJECT(m_hScroll),this);

	gtk_signal_connect(GTK_OBJECT(m_pHadj), "value_changed", GTK_SIGNAL_FUNC(_fe::hScrollChanged), NULL);

	m_pVadj = (GtkAdjustment*) gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	gtk_object_set_user_data(GTK_OBJECT(m_pVadj),this);
	m_vScroll = gtk_vscrollbar_new(m_pVadj);
	gtk_object_set_user_data(GTK_OBJECT(m_vScroll),this);

	gtk_signal_connect(GTK_OBJECT(m_pVadj), "value_changed", GTK_SIGNAL_FUNC(_fe::vScrollChanged), NULL);

	// we don't want either scrollbar grabbing events from us
	GTK_WIDGET_UNSET_FLAGS(m_hScroll, GTK_CAN_FOCUS);
	GTK_WIDGET_UNSET_FLAGS(m_vScroll, GTK_CAN_FOCUS);

	// create a drawing area in the for our document window.
	m_dArea = gtk_drawing_area_new();
	
	gtk_object_set_user_data(GTK_OBJECT(m_dArea),this);
	gtk_widget_set_events(GTK_WIDGET(m_dArea), (GDK_EXPOSURE_MASK |
												GDK_BUTTON_PRESS_MASK |
												GDK_POINTER_MOTION_MASK |
												GDK_BUTTON_RELEASE_MASK |
												GDK_KEY_PRESS_MASK |
												GDK_KEY_RELEASE_MASK));

	gtk_signal_connect(GTK_OBJECT(m_dArea), "expose_event",
					   GTK_SIGNAL_FUNC(_fe::expose), NULL);
  
	gtk_signal_connect(GTK_OBJECT(m_dArea), "button_press_event",
					   GTK_SIGNAL_FUNC(_fe::button_press_event), NULL);

	gtk_signal_connect(GTK_OBJECT(m_dArea), "button_release_event",
					   GTK_SIGNAL_FUNC(_fe::button_release_event), NULL);

	gtk_signal_connect(GTK_OBJECT(m_dArea), "motion_notify_event",
					   GTK_SIGNAL_FUNC(_fe::motion_notify_event), NULL);
  
	gtk_signal_connect(GTK_OBJECT(m_dArea), "configure_event",
					   GTK_SIGNAL_FUNC(_fe::configure_event), NULL);

	// create a table for scroll bars, rulers, and drawing area

	m_table = gtk_table_new(1, 1, FALSE);
	gtk_object_set_user_data(GTK_OBJECT(m_table),this);

	// NOTE:  in order to display w/ and w/o rulers, gtk needs two tables to
	// work with.  The 2 2x2 tables, (i)nner and (o)uter divide up the 3x3
	// table as follows.  The inner table is at the 1,1 table.
	//	+-----+---+
	//	| i i | o |
	//	| i i |   |
	//	+-----+---+
	//	|  o  | o |
	//	+-----+---+
		
	// scroll bars
	gtk_table_attach(GTK_TABLE(m_table), m_hScroll, 0, 1, 1, 2,
					 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					 (GtkAttachOptions) (GTK_FILL),
					 0, 0);

	gtk_table_attach(GTK_TABLE(m_table), m_vScroll, 1, 2, 0, 1,
					 (GtkAttachOptions) (GTK_FILL),
					 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					 0, 0);


	// arrange the widgets within our inner table.
	m_innertable = gtk_table_new(2,2,FALSE);
	gtk_table_attach( GTK_TABLE(m_table), m_innertable, 0, 1, 0, 1,
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 0, 0); 

	if ( bShowRulers )
	{
		gtk_table_attach(GTK_TABLE(m_innertable), m_topRuler, 0, 2, 0, 1,
						 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
						 (GtkAttachOptions)(GTK_FILL),
						 0, 0);

		gtk_table_attach(GTK_TABLE(m_innertable), m_leftRuler, 0, 1, 1, 2,
						 (GtkAttachOptions)(GTK_FILL),
						 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
						 0, 0);

		gtk_table_attach(GTK_TABLE(m_innertable), m_dArea,   1, 2, 1, 2,
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 0, 0); 
	}
	else	// no rulers
	{
		gtk_table_attach(GTK_TABLE(m_innertable), m_dArea,   1, 2, 1, 2,
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						 0, 0); 
	}

	// create a 3d box and put the table in it, so that we
	// get a sunken in look.
	m_wSunkenBox = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(m_wSunkenBox), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(m_wSunkenBox), m_table);

	gtk_widget_show(m_hScroll);
	gtk_widget_show(m_vScroll);
	gtk_widget_show(m_dArea);
	gtk_widget_show(m_innertable);
	gtk_widget_show(m_table);

	return m_wSunkenBox;
}

void AP_UnixFrame::translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y)
{
	// translate the given document mouse coordinates into absolute screen coordinates.

	Window child;
	gint tx;
	gint ty;
  
	GdkWindowPrivate * priv = (GdkWindowPrivate*) m_dArea->window;
	if (!priv->destroyed)
		XTranslateCoordinates (priv->xdisplay, priv->xwindow, gdk_root_window, x, y, &tx, &ty, &child);
  
	x = tx;
	y = ty;
}

GtkWidget * AP_UnixFrame::_createStatusBarWindow(void)
{
	AP_UnixStatusBar * pUnixStatusBar = new AP_UnixStatusBar(this);
	UT_ASSERT(pUnixStatusBar);

	((AP_FrameData *)m_pData)->m_pStatusBar = pUnixStatusBar;
	
	GtkWidget * w = pUnixStatusBar->createWidget();

	return w;
}

void AP_UnixFrame::setStatusMessage(const char * szMsg)
{
	((AP_FrameData *)m_pData)->m_pStatusBar->setStatusMessage(szMsg);
}

void AP_UnixFrame::_setWindowIcon(void)
{
	// attach program icon to window
	GtkWidget * window = getTopLevelWindow();
	UT_ASSERT(window);

	// create a pixmap from our included data
	GdkBitmap * mask;
	GdkPixmap * pixmap = gdk_pixmap_create_from_xpm_d(window->window,
													  &mask,
													  NULL,
													  abiword_48_xpm);
	UT_ASSERT(pixmap && mask);
		
	gdk_window_set_icon(window->window, NULL, pixmap, mask);
	gdk_window_set_icon_name(window->window, "AbiWord Application Icon");
}

UT_Error AP_UnixFrame::_replaceDocument(AD_Document * pDoc)
{
	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = REFP(pDoc);

	return _showDocument();
}

void AP_UnixFrame::toggleRuler(bool bRulerOn)
{
	//UT_DEBUGMSG(("AP_UnixFrame::toggleRuler %d\n", bRulerOn));	

	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);
		
	AP_UnixTopRuler * pUnixTopRuler = NULL;
	AP_UnixLeftRuler * pUnixLeftRuler = NULL;
		

	if ( bRulerOn )
	{
		UT_ASSERT(!pFrameData->m_pTopRuler);
		UT_ASSERT(!pFrameData->m_pLeftRuler);
	
		pUnixTopRuler = new AP_UnixTopRuler(this);
		UT_ASSERT(pUnixTopRuler);
		m_topRuler = pUnixTopRuler->createWidget();
		
		pUnixLeftRuler = new AP_UnixLeftRuler(this);
		UT_ASSERT(pUnixLeftRuler);
		m_leftRuler = pUnixLeftRuler->createWidget();
		
		// get the width from the left ruler and stuff it into the top ruler.
		pUnixTopRuler->setOffsetLeftRuler(pUnixLeftRuler->getWidth());
	
		// attach everything	
		gtk_table_attach(GTK_TABLE(m_innertable), m_topRuler, 0, 2, 0, 1,
						 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
						 (GtkAttachOptions)(GTK_FILL),
						 0,0);

		gtk_table_attach(GTK_TABLE(m_innertable), m_leftRuler, 0, 1, 1, 2,
						 (GtkAttachOptions)(GTK_FILL),
						 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
						 0,0);

		pUnixTopRuler->setView(m_pView);
		pUnixLeftRuler->setView(m_pView);

	}
	else	// turning rulers off
	{
		// delete the actual widgets
		gtk_object_destroy( GTK_OBJECT(m_topRuler) );
		gtk_object_destroy( GTK_OBJECT(m_leftRuler) );

		DELETEP(((AP_FrameData*)m_pData)->m_pTopRuler);
		DELETEP(((AP_FrameData*)m_pData)->m_pLeftRuler);

		m_topRuler = NULL;
		m_leftRuler = NULL;
	}
	
	((AP_FrameData*)m_pData)->m_pTopRuler = pUnixTopRuler;
	((AP_FrameData*)m_pData)->m_pLeftRuler = pUnixLeftRuler;
}

void AP_UnixFrame::toggleBar(UT_uint32 iBarNb, bool bBarOn)
{
	UT_DEBUGMSG(("AP_UnixFrame::toggleBar %d, %d\n", iBarNb, bBarOn));	

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
	UT_ASSERT(pFrameData);
	
	if (bBarOn)
		pFrameData->m_pToolbar[iBarNb]->show();
	else	// turning toolbar off
		pFrameData->m_pToolbar[iBarNb]->hide();
}

void AP_UnixFrame::toggleStatusBar(bool bStatusBarOn)
{
	UT_DEBUGMSG(("AP_UnixFrame::toggleStatusBar %d\n", bStatusBarOn));	

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
	UT_ASSERT(pFrameData);
	
	if (bStatusBarOn)
		pFrameData->m_pStatusBar->show();
	else	// turning status bar off
		pFrameData->m_pStatusBar->hide();
}
