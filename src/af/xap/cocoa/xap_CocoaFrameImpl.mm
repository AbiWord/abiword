/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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


#import <Cocoa/Cocoa.h>

#import "ut_debugmsg.h"
#import "ut_assert.h"
#import "ut_sleep.h"

#import "fv_View.h"
#import "ev_EditMethod.h"
#import "ev_CocoaKeyboard.h"
#import "ev_CocoaMouse.h"
#import "ev_CocoaMenu.h"
#import "ev_CocoaMenuBar.h"
#import "ev_CocoaMenuPopup.h"
#import "ev_CocoaToolbar.h"
#import "ev_Toolbar.h"
#import "gr_CocoaGraphics.h"
#import "xap_App.h"
#import "xap_CocoaApp.h"
#import "xap_CocoaFrameImpl.h"
#import "xap_CocoaTimer.h"
#import "xap_FrameImpl.h"
#import "xap_Frame.h"

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

#if 0
/****************************************************************/
void XAP_CocoaFrameImpl::_fe::realize(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
  GdkICAttr *ic_attr=(GdkICAttr *)g_object_get_data(G_OBJECT(widget), "ic_attr");
  GdkIC * ic=(GdkIC *)g_object_get_data(G_OBJECT(widget), "ic");
  if (gdk_im_ready () && (ic_attr = gdk_ic_attr_new ()) != NULL)
    {
      gint width, height;
      int mask;
	  GdkColormap *colormap;
      GdkICAttr *attr = ic_attr;
      int attrmask = GDK_IC_ALL_REQ;
      GdkIMStyle style;

      int supported_style =(GdkIMStyle)(GDK_IM_PREEDIT_NONE |
				   GDK_IM_PREEDIT_NOTHING |
			           GDK_IM_PREEDIT_POSITION |
			           GDK_IM_STATUS_NONE |
				   GDK_IM_STATUS_NOTHING);

      if (widget->style && widget->style->font->type != GDK_FONT_FONTSET)
		supported_style &= ~GDK_IM_PREEDIT_POSITION;

      attr->style = style = gdk_im_decide_style ((GdkIMStyle)supported_style);
      attr->client_window = widget->window;

      if ((colormap = gtk_widget_get_colormap (widget)) !=
		  gtk_widget_get_default_colormap ())
		{
		  attrmask |= GDK_IC_PREEDIT_COLORMAP;
		  attr->preedit_colormap = colormap;
		}
      attrmask |= GDK_IC_PREEDIT_FOREGROUND;
      attrmask |= GDK_IC_PREEDIT_BACKGROUND;
      attr->preedit_foreground = widget->style->fg[GTK_STATE_NORMAL];
      attr->preedit_background = widget->style->base[GTK_STATE_NORMAL];

      switch (style & GDK_IM_PREEDIT_MASK)
		{
		case GDK_IM_PREEDIT_POSITION:
		  if (widget->style && widget->style->font->type != GDK_FONT_FONTSET)
			{
			  g_warning ("over-the-spot style requires fontset");
			  break;
			}

		  gdk_window_get_size (widget->window, &width, &height);
		  
		  attrmask |= GDK_IC_PREEDIT_POSITION_REQ;
		  attr->spot_location.x = 0;
		  attr->spot_location.y = height;
		  attr->preedit_area.x = 0;
		  attr->preedit_area.y = 0;
		  attr->preedit_area.width = width;
		  attr->preedit_area.height = height;
		  attr->preedit_fontset = widget->style->font;
		  
		  break;
		}
      ic = gdk_ic_new (attr, (GdkICAttributesType)attrmask);
	  
      if (ic == NULL)
		g_warning ("Can't create input context.");
      else
		{
		  mask = gdk_window_get_events (widget->window);
		  mask |= (GdkEventMask)gdk_ic_get_events (ic);
		  gdk_window_set_events (widget->window,(GdkEventMask) mask);

		  if (GTK_WIDGET_HAS_FOCUS(widget))
			gdk_im_begin (ic, widget->window);
		}
	}
  g_object_set_data(G_OBJECT(widget), "ic_attr", ic_attr);
  g_object_set_data(G_OBJECT(widget), "ic", ic);
}

void XAP_CocoaFrameImpl::_fe::unrealize(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
  GdkICAttr *ic_attr=(GdkICAttr *)g_object_get_data(G_OBJECT(widget), "ic_attr");
  GdkIC * ic=(GdkIC *)g_object_get_data(G_OBJECT(widget), "ic");
  if (ic)
    {
      gdk_ic_destroy (ic);
      ic = (GdkIC *)NULL;
    }
  if (ic_attr)
    {
      gdk_ic_attr_destroy (ic_attr);
      ic_attr = (GdkICAttr *)NULL;
    }
  g_object_set_data(G_OBJECT(widget), "ic_attr", ic_attr);
  g_object_set_data(G_OBJECT(widget), "ic", ic);
}

void XAP_CocoaFrameImpl::_fe::sizeAllocate(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
  GdkICAttr *ic_attr=(GdkICAttr *)g_object_get_data(G_OBJECT(widget), "ic_attr");
  GdkIC * ic=(GdkIC *)g_object_get_data(G_OBJECT(widget), "ic");
  if (ic &&
	  (gdk_ic_get_style (ic) & GDK_IM_PREEDIT_POSITION))
	{
	  gint width, height;

	  gdk_window_get_size (widget->window, &width, &height);
	  ic_attr->preedit_area.width = width;
	  ic_attr->preedit_area.height = height;
	  gdk_ic_set_attr (ic, ic_attr,
	      		   GDK_IC_PREEDIT_AREA);
	}
}

gint XAP_CocoaFrameImpl::_fe::focusIn(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
  GdkIC * ic=(GdkIC *)g_object_get_data(G_OBJECT(widget), "ic");
  if (ic)
    gdk_im_begin (ic, widget->window);
  return FALSE;
}

gint XAP_CocoaFrameImpl::_fe::focusOut(GtkWidget * /* w*/, GdkEvent * /*e*/,gpointer /*data*/)
{
  gdk_im_end ();
  return FALSE;
}
gboolean XAP_CocoaFrameImpl::_fe::focus_in_event(GtkWidget *w,GdkEvent */*event*/,gpointer /*user_data*/)
{
	XAP_CocoaFrame * pFrame = (XAP_CocoaFrame *) g_object_get_user_data(G_OBJECT(w));
	UT_ASSERT(pFrame);
	g_object_set_data(G_OBJECT(w), "toplevelWindowFocus",
						GINT_TO_POINTER(TRUE));
	if (pFrame->getCurrentView())
		pFrame->getCurrentView()->focusChange(gtk_grab_get_current() == NULL || gtk_grab_get_current() == w ? AV_FOCUS_HERE : AV_FOCUS_NEARBY);
	return FALSE;
}

gboolean XAP_CocoaFrameImpl::_fe::focus_out_event(GtkWidget *w,GdkEvent */*event*/,gpointer /*user_data*/)
{
	XAP_CocoaFrame * pFrame = (XAP_CocoaFrame *)g_object_get_user_data(G_OBJECT(w));
	UT_ASSERT(pFrame);
	g_object_set_data(G_OBJECT(w), "toplevelWindowFocus",
						GINT_TO_POINTER(FALSE));
	if (pFrame->getCurrentView())
		pFrame->getCurrentView()->focusChange(AV_FOCUS_NONE);
	return FALSE;
}

gint XAP_CocoaFrameImpl::_fe::button_press_event(GtkWidget * w, GdkEventButton * e)
{
	XAP_CocoaFrame * pCocoaFrame = (XAP_CocoaFrame *)g_object_get_user_data(G_OBJECT(w));
	pCocoaFrame->setTimeOfLastEvent(e->time);
	AV_View * pView = pCocoaFrame->getCurrentView();
	EV_CocoaMouse * pCocoaMouse = static_cast<EV_CocoaMouse *>(pCocoaFrame->getMouse());

	//UT_DEBUGMSG(("Grabbing mouse.\n"));
	gtk_grab_add(w);
	
	if (pView)
		pCocoaMouse->mouseClick(pView,e);
	return 1;
}

gint XAP_CocoaFrameImpl::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	XAP_CocoaFrame * pCocoaFrame = (XAP_CocoaFrame *)g_object_get_user_data(G_OBJECT(w));
	pCocoaFrame->setTimeOfLastEvent(e->time);
	AV_View * pView = pCocoaFrame->getCurrentView();

	EV_CocoaMouse * pCocoaMouse = static_cast<EV_CocoaMouse *>(pCocoaFrame->getMouse());

	//UT_DEBUGMSG(("Ungrabbing mouse.\n"));
	gtk_grab_remove(w);
	
	if (pView)
		pCocoaMouse->mouseUp(pView,e);
	
	return 1;
}
	
gint XAP_CocoaFrameImpl::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// This is basically a resize event.
		
	XAP_CocoaFrame * pCocoaFrame = (XAP_CocoaFrame *)g_object_get_user_data(G_OBJECT(w));
	AV_View * pView = pCocoaFrame->getCurrentView();

	if (pView)
		pView->setWindowSize(e->width, e->height);

	// Dynamic Zoom Implimentation
   pCocoaFrame->updateZoom();

	return 1;
}

	
gint XAP_CocoaFrameImpl::_fe::expose(GtkWidget * w, GdkEventExpose* pExposeEvent)
{
	UT_Rect rClip;
	rClip.left = pExposeEvent->area.x;
	rClip.top = pExposeEvent->area.y;
	rClip.width = pExposeEvent->area.width;
	rClip.height = pExposeEvent->area.height;
	xxx_UT_DEBUGMSG(("gtk in Frame expose:  left=%d, top=%d, width=%d, height=%d\n", rClip.left, rClip.top, rClip.width, rClip.height));
	XAP_CocoaFrame * pCocoaFrame = (XAP_CocoaFrame *)g_object_get_user_data(G_OBJECT(w));
	FV_View * pView = (FV_View *) pCocoaFrame->getCurrentView();
	if(pView)
	{
		GR_Graphics * pG = pView->getGraphics();
		pG->doRepaint(&rClip);
	}
	return 0;
}

void XAP_CocoaFrameImpl::_fe::vScrollChanged(GtkAdjustment * w, gpointer /*data*/)
{
	XAP_CocoaFrame * pCocoaFrame = (XAP_CocoaFrame *)g_object_get_user_data(G_OBJECT(w));
	AV_View * pView = pCocoaFrame->getCurrentView();
	
	//UT_DEBUGMSG(("gtk vScroll: value %ld\n",(UT_sint32)w->value));
	
	if (pView)
		pView->sendVerticalScrollEvent((UT_sint32) w->value);
}
	
void XAP_CocoaFrameImpl::_fe::hScrollChanged(GtkAdjustment * w, gpointer /*data*/)
{
	XAP_CocoaFrame * pCocoaFrame = (XAP_CocoaFrame *)g_object_get_user_data(G_OBJECT(w));
	AV_View * pView = pCocoaFrame->getCurrentView();
	
	if (pView)
		pView->sendHorizontalScrollEvent((UT_sint32) w->value);
}
	
void XAP_CocoaFrameImpl::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
	// I think this is right:
	// 	We shouldn't have to call gtk_main_quit() here because
	//  this signal catcher is only inserted before the GTK
	//  default handler (which will continue to destroy the window
	//  if we don't return TRUE).
	//
	//  This function should be for things to happen immediately
	//  before a frame gets hosed once and for all.
	
	//gtk_main_quit ();
}
#endif

/*!
 * Background abi repaint function.
\param XAP_CocoaFrameImpl * p pointer to the Frame that initiated this background
       repainter.
 */
int XAP_CocoaFrameImpl::_fe::abi_expose_repaint(void * p)
{
//
// Grab our pointer so we can do useful stuff.
//
	UT_Rect localCopy;
	XAP_Frame * pF = (XAP_Frame *)p;
	FV_View * pV = (FV_View *) pF->getCurrentView();
	if(!pV)
	{ 
		return TRUE;
	}
	GR_Graphics * pG = pV->getGraphics();
	if(pG->isDontRedraw())
	{
//
// Come back later
//
		return TRUE;
	}
	pG->setSpawnedRedraw(true);
	if(pG->isExposePending())
	{
		while(pG->isExposedAreaAccessed())
		{
			UT_usleep(10); // 10 microseconds
		}
		pG->setExposedAreaAccessed(true);
		localCopy.set(pG->getPendingRect()->left,pG->getPendingRect()->top,
					  pG->getPendingRect()->width,pG->getPendingRect()->height);
//
// Clear out this set of expose info
//
		pG->setExposePending(false);
		pG->setExposedAreaAccessed(false);
//			UT_DEBUGMSG(("Painting area:  left=%d, top=%d, width=%d, height=%d\n", localCopy.left, localCopy.top, localCopy.width, localCopy.height));
		xxx_UT_DEBUGMSG(("SEVIOR: Repaint now \n"));
		pV->draw(&localCopy);
	}
//
// OK we've finshed. Wait for the next signal
//
	pG->setSpawnedRedraw(false);
	return TRUE;
}
	
/*****************************************************************/

XAP_CocoaFrameImpl::XAP_CocoaFrameImpl(XAP_Frame* frame, XAP_CocoaApp * app)
	: XAP_FrameImpl (frame),
	  m_dialogFactory(frame, app),
	  m_pCocoaApp(app),
	  m_pCocoaMenu(NULL),
	  m_pCocoaPopup(NULL),
	  m_frameController(nil),
	  m_iAbiRepaintID(0)
{
//	m_pView = NULL;
}

// TODO when cloning a new frame from an existing one
// TODO should we also clone any frame-persistent
// TODO dialog data ??
/*
XAP_CocoaFrameImpl::XAP_CocoaFrameImpl(XAP_CocoaFrameImpl * f)
	: XAP_FrameImpl(f),
	  m_dialogFactory(f->m_pFrame, static_cast<XAP_App *>(f->m_pCocoaApp)),
	  m_pCocoaApp(f->m_pCocoaApp),
	  m_pCocoaMenu(NULL),
	  m_pCocoaPopup(NULL),
	  m_frameController(nil),
	  m_iAbiRepaintID(0)
{
	m_pView = NULL;
}
*/

XAP_CocoaFrameImpl::~XAP_CocoaFrameImpl(void)
{
	// only delete the things we created...
	if(m_iAbiRepaintID)
	{
		XAP_stopCocoaTimer(m_iAbiRepaintID);
	}

	if 	(m_frameController != nil) {
		[m_frameController release];
	}

	DELETEP(m_pCocoaMenu);
	DELETEP(m_pCocoaPopup);
}

/*
bool XAP_CocoaFrameImpl::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
								  const char * szMenuLayoutKey, const char * szMenuLayoutDefaultValue,
								  const char * szMenuLabelSetKey, const char * szMenuLabelSetDefaultValue,
								  const char * szToolbarLayoutsKey, const char * szToolbarLayoutsDefaultValue,
								  const char * szToolbarLabelSetKey, const char * szToolbarLabelSetDefaultValue)
{
	bool bResult;

	// invoke our base class first.
	
	bResult = XAP_Frame::initialize(szKeyBindingsKey, szKeyBindingsDefaultValue,
									szMenuLayoutKey, szMenuLayoutDefaultValue,
									szMenuLabelSetKey, szMenuLabelSetDefaultValue,
									szToolbarLayoutsKey, szToolbarLayoutsDefaultValue,
									szToolbarLabelSetKey, szToolbarLabelSetDefaultValue);
	UT_ASSERT(bResult);
*/
void XAP_CocoaFrameImpl::_initialize()
{
   	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.

	EV_EditEventMapper * pEEM = getFrame()->getEditEventMapper();
	UT_ASSERT(pEEM);

	m_pKeyboard = new ev_CocoaKeyboard(pEEM);
	UT_ASSERT(m_pKeyboard);
	
	m_pMouse = new EV_CocoaMouse(pEEM);
	UT_ASSERT(m_pMouse);

//
// Start background repaint
//
	if(m_iAbiRepaintID == 0)
	{
		m_iAbiRepaintID = XAP_newCocoaTimer(100, _fe::abi_expose_repaint, this);
	}
	else
	{
		XAP_stopCocoaTimer(m_iAbiRepaintID);
		m_iAbiRepaintID = XAP_newCocoaTimer(100, _fe::abi_expose_repaint, this);
	}
}

UT_sint32 XAP_CocoaFrameImpl::_setInputMode(const char * szName)
{
	XAP_Frame*	pFrame = getFrame();
	UT_sint32 result = pFrame->XAP_Frame::setInputMode(szName);
	if (result == 1)
	{
		// if it actually changed we need to update keyboard and mouse

		EV_EditEventMapper * pEEM = pFrame->getEditEventMapper();
		UT_ASSERT(pEEM);

		m_pKeyboard->setEditEventMap(pEEM);
		m_pMouse->setEditEventMap(pEEM);
	}

	return result;
}

NSWindow * XAP_CocoaFrameImpl::getTopLevelWindow(void) const
{
	UT_ASSERT (m_frameController);
	return [m_frameController window];
}

NSView * XAP_CocoaFrameImpl::getVBoxWidget(void) const
{
	UT_ASSERT (m_frameController);
	return [m_frameController getMainView];
}

XAP_DialogFactory * XAP_CocoaFrameImpl::_getDialogFactory(void)
{
	return &m_dialogFactory;
}

void XAP_CocoaFrameImpl::_nullUpdate() const
{
#if 0
  gtk_main_iteration ();
#endif
}

void XAP_CocoaFrameImpl::_createTopLevelWindow(void)
{
	// create a top-level window for us.
	m_frameController = _createController();
	UT_ASSERT (m_frameController);

	NSWindow * theWindow = [m_frameController window];
	UT_ASSERT (theWindow);
	[theWindow setTitle:[NSString stringWithCString:m_pCocoaApp->getApplicationTitleForTitleBar()]];
//	NSView * docArea = [m_frameController getMainView];
/*  	[scroller setHasHorizontalScroller:YES];
  	[scroller setHasVerticalScroller:YES];*/

	// synthesize a menu from the info in our base class.

	m_pCocoaMenu = new EV_CocoaMenuBar(m_pCocoaApp,(AP_CocoaFrame*)getFrame(),
									 m_szMenuLayoutName,
									 m_szMenuLabelSetName);
	UT_ASSERT(m_pCocoaMenu);
	bool bResult = m_pCocoaMenu->synthesizeMenuBar([m_frameController getMenuBar]);
	UT_ASSERT(bResult);

	// create a toolbar instance for each toolbar listed in our base class.
	// TODO for some reason, the toolbar functions require the TLW to be
	// TODO realized (they reference m_wTopLevelWindow->window) before we call them.

	//gtk_widget_realize(m_wTopLevelWindow);

	_createToolbars();

	// Let the app-specific frame code create the contents of
	// the child area of the window (between the toolbars and
	// the status bar).
	_createDocumentWindow();
#if 0
	gtk_container_add(GTK_CONTAINER(m_wVBox), m_wSunkenBox);
	gtk_widget_show(m_wSunkenBox);
#endif
	// Let the app-specific frame code create the status bar
	// if it wants to.  we will put it below the document
	// window (a peer with toolbars and the overall sunkenbox)
	// so that it will appear outside of the scrollbars.
	_createStatusBarWindow([m_frameController getStatusBar]);
#if 0
	if (m_wStatusBar)
	{
		gtk_widget_show(m_wStatusBar);
		gtk_box_pack_end(GTK_BOX(m_wVBox), m_wStatusBar, FALSE, FALSE, 0);
	}
	
	gtk_widget_show(m_wVBox);
#endif

	// set the icon
	_setWindowIcon();

	// set geometry hints as the user requested
	int x, y;
	UT_uint32 width, height;
	XAP_CocoaApp::windowGeometryFlags f;

	m_pCocoaApp->getGeometry(&x, &y, &width, &height, &f);

	// Set the size if requested

	if (f & XAP_CocoaApp::GEOMETRY_FLAG_SIZE)
	{
		NSRect windowFrame;
		NSRect screenFrame = [[NSScreen mainScreen] visibleFrame];

		windowFrame.size.width = UT_MIN( screenFrame.size.width - 30, 813);
		windowFrame.size.height = UT_MIN( screenFrame.size.height - 100, 836);
		windowFrame.origin.x = 0;
		windowFrame.origin.y = 0;
		[theWindow setFrame:windowFrame display:YES];
	}


	// Because we're clever, we only honor this flag when we
	// are the first (well, only) top level frame available.
	// This is so the user's window manager can find better
	// places for new windows, instead of having our windows
	// pile upon each other.

#if 0
	if (m_pCocoaApp->getFrameCount() <= 1)
		if (f & XAP_CocoaApp::GEOMETRY_FLAG_POS)
			gtk_widget_set_uposition(m_wTopLevelWindow,
									 x,
									 y);
#endif

	// we let our caller decide when to show m_wTopLevelWindow.
	return;
}

/*!
 * This code is used by the dynamic menu API to rebuild the menus after a
 * a change in the menu structure.
 */
void XAP_CocoaFrameImpl::rebuildMenus(void)
{
//
// Destroy the old menu bar
//
	m_pCocoaMenu->destroy();
//
// Delete the old class
//
	DELETEP(m_pCocoaMenu);
//
// Build a new one.
//
	m_pCocoaMenu = new EV_CocoaMenuBar(m_pCocoaApp,(AP_CocoaFrame*)getFrame(),
									 m_szMenuLayoutName,
									 m_szMenuLabelSetName);
	UT_ASSERT(m_pCocoaMenu);
	bool bResult = m_pCocoaMenu->rebuildMenuBar();
	UT_ASSERT(bResult);

}


/*!
 * This code is used by the dynamic toolbar API to rebuild a toolbar after a
 * a change in the toolbar structure.
 */
void XAP_CocoaFrameImpl::_rebuildToolbar(UT_uint32 ibar)
{
	XAP_Frame* pFrame = getFrame();
//
// Destroy the old toolbar
//
	EV_Toolbar * pToolbar = (EV_Toolbar *) m_vecToolbars.getNthItem(ibar);
	const char * szTBName = (const char *) m_vecToolbarLayoutNames.getNthItem(ibar);
	EV_CocoaToolbar * pUTB = static_cast<EV_CocoaToolbar *>( pToolbar);
	UT_sint32 oldpos = pUTB->destroy();
//
// Delete the old class
//
	delete pToolbar;
	if(oldpos < 0)
	{
		return;
	}
//
// Build a new one.
//
	pToolbar = _newToolbar(m_pCocoaApp, pFrame, szTBName,
						   (const char *) m_szToolbarLabelSetName);
	static_cast<EV_CocoaToolbar *>(pToolbar)->rebuildToolbar(oldpos);
	m_vecToolbars.setNthItem(ibar, (void *) pToolbar, NULL);
//
// Refill the framedata pointers
//
	pFrame->refillToolbarsInFrameData();
	pFrame->repopulateCombos();
}

bool XAP_CocoaFrameImpl::_close()
{
	UT_DEBUGMSG (("XAP_CocoaFrame::close()\n"));
	[m_frameController close];
	return true;
}

bool XAP_CocoaFrameImpl::_raise()
{
	UT_DEBUGMSG (("XAP_CocoaFrame::raise()\n"));
	[[m_frameController window] makeKeyAndOrderFront:m_frameController];

	return true;
}

bool XAP_CocoaFrameImpl::_show()
{
	UT_DEBUGMSG (("XAP_CocoaFrame::raise()\n"));
	[[m_frameController window] makeKeyAndOrderFront:m_frameController];

	return true;
}

bool XAP_CocoaFrameImpl::_openURL(const char * szURL)
{  
	NSURL *URL = [[NSURL alloc] initWithString:[NSString stringWithCString:szURL]];		
	
	NSWorkspace * space = [NSWorkspace sharedWorkspace];
	[space openURL:URL];

	[URL release];
	
	return true;
}

UT_RGBColor XAP_CocoaFrameImpl::getColorSelBackground () const
{
	return XAP_FrameImpl::getColorSelBackground();
#if 0
	/* this code is disabled as the NSColor returned is not RGB compatible. */
	static UT_RGBColor * c = NULL;
	if (c == NULL) {
		c = new UT_RGBColor();
		GR_CocoaGraphics::_utNSColorToRGBColor ([NSColor selectedTextBackgroundColor], *c);
	}
	return *c;
#endif
}


bool XAP_CocoaFrameImpl::_updateTitle()
{
	if (!XAP_FrameImpl::_updateTitle())
	{
		// no relevant change, so skip it
		return false;
	}

	char buf[256];
	buf[0] = 0;

	const char * szAppName = m_pCocoaApp->getApplicationTitleForTitleBar();

	int len = 256 - strlen(szAppName) - 4;
	
	const char * szTitle = getFrame()->getTitle(len);

	//sprintf(buf, "%s - %s", szTitle, szAppName);
	/* TODO discard this sprintf and you NSString features instead */
	NSWindow * theWindow = [m_frameController window];
	UT_ASSERT (theWindow);
	NSString * str = [NSString stringWithCString:szTitle];
	[theWindow setTitleWithRepresentedFilename:str];

	return true;
}

/*****************************************************************/
bool XAP_CocoaFrameImpl::_runModalContextMenu(AV_View * /* pView */, const char * szMenuName,
										   UT_sint32 x, UT_sint32 y)
{
#if 0
	bool bResult = true;

	UT_ASSERT(!m_pCocoaPopup);

	m_pCocoaPopup = new EV_CocoaMenuPopup(m_pCocoaApp,this,szMenuName,m_szMenuLabelSetName);
	if (m_pCocoaPopup && m_pCocoaPopup->synthesizeMenuPopup())
	{
		// the popup will steal the mouse and so we won't get the
		// button_release_event and we won't know to release our
		// grab.  so let's do it here.  (when raised from a keyboard
		// context menu, we may not have a grab, but that should be ok.

		GtkWidget * w = gtk_grab_get_current();
		if (w)
		{
			//UT_DEBUGMSG(("Ungrabbing mouse [before popup].\n"));
			gtk_grab_remove(w);
		}

		translateDocumentToScreen(x,y);

		UT_DEBUGMSG(("ContextMenu: %s at [%d,%d]\n",szMenuName,x,y));
		UT_Point pt;
		pt.x = x;
		pt.y = y;
//
// OK lets not immediately drop the menu if the user releases the mouse button.
// From the gtk FAQ.
//
		GdkEvent * event = gtk_get_current_event();
		GdkEventButton *bevent = (GdkEventButton *) event; 
		gtk_menu_popup(GTK_MENU(m_pCocoaPopup->getMenuHandle()), NULL, NULL,
					   s_gtkMenuPositionFunc, &pt, bevent->button, bevent->time);

		// We run this menu synchronously, since GTK doesn't.
		// Popup menus have a special "unmap" function to call
		// gtk_main_quit() when they're done.
		gtk_main();
	}
	XAP_Frame * pFrame = (XAP_Frame *) this;
	if (pFrame->getCurrentView())
	{
		pFrame->getCurrentView()->focusChange( AV_FOCUS_HERE);
	}
	DELETEP(m_pCocoaPopup);
	return bResult;
#endif
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return false;
}

void XAP_CocoaFrameImpl::setTimeOfLastEvent(NSTimeInterval timestamp)
{
	m_pCocoaApp->setTimeOfLastEvent(timestamp);
}

EV_Toolbar * XAP_CocoaFrameImpl::_newToolbar(XAP_App *app, XAP_Frame *frame,
					const char *szLayout,
					const char *szLanguage)
{
	return (new EV_CocoaToolbar(static_cast<XAP_CocoaApp *>(app), 
							   (AP_CocoaFrame *)frame, 
							   szLayout, szLanguage));
}

void XAP_CocoaFrameImpl::_queue_resize()
{
	UT_DEBUGMSG(("XAP_CocoaFrameImpl::queue_resize\n"));	
	UT_ASSERT (UT_NOT_IMPLEMENTED);
//	gtk_widget_queue_resize(m_wTopLevelWindow);
}


EV_Menu* XAP_CocoaFrameImpl::_getMainMenu()
{
	return m_pCocoaMenu;
}


void XAP_CocoaFrameImpl::_setController (XAP_CocoaFrameController * ctrl)
{
	if ((m_frameController) && (ctrl != m_frameController)){
		[m_frameController release];
	}
	m_frameController = ctrl; 
}


/* Objective C section */

@implementation XAP_CocoaFrameController

- (BOOL)windowShouldClose:(id)sender
{
	UT_DEBUGMSG (("shouldCloseDocument\n"));
	UT_ASSERT (m_frame);
	XAP_App * pApp = m_frame->getFrame()->getApp();
	UT_ASSERT(pApp);

	const EV_Menu_ActionSet * pMenuActionSet = pApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
	UT_ASSERT(pEMC);
	
	const EV_EditMethod * pEM = pEMC->findEditMethodByName("closeWindowX");
	UT_ASSERT(pEM);
	
	if (pEM)
	{
		if (pEM->Fn(m_frame->getFrame()->getCurrentView(),NULL))
		{
			// returning YES means close the window
			
			return YES;
		}
	}
		
	// returning NO means do NOT close the window
	return NO;
}


- (void)keyDown:(NSEvent *)theEvent
{
	XAP_Frame * pFrame = m_frame->getFrame();
//  	pFrame->setTimeOfLastEvent([theEvent timestamp]);
	AV_View * pView = pFrame->getCurrentView();
	ev_CocoaKeyboard * pCocoaKeyboard = static_cast<ev_CocoaKeyboard *>
		(pFrame->getKeyboard());

	if (pView)
		pCocoaKeyboard->keyPressEvent(pView, theEvent);
}


/*!
	Returns an instance.
 */
+ (XAP_CocoaFrameController*)createFrom:(XAP_CocoaFrameImpl *)frame
{
	UT_DEBUGMSG (("Cocoa: createFrom:frame\n"));
	XAP_CocoaFrameController *obj = [[XAP_CocoaFrameController alloc] initWith:frame];
	return [obj autorelease];
}

- (id)initWith:(XAP_CocoaFrameImpl *)frame
{
	UT_DEBUGMSG (("Cocoa: @XAP_CocoaFrameController initWith:frame\n"));
	m_frame = frame;
	[self initWithWindowNibName:frame->_getNibName()];	/* this one will make the call to [super init]  */
	[[self window] setAcceptsMouseMovedEvents:YES];		/* can't we set that from IB (FIXME) */
	return self;
}

- (NSView *)getMainView
{
	return mainView;
}

- (NSMenu *)getMenuBar
{
	return menuBar;
}

- (XAP_CocoaNSStatusBar *)getStatusBar
{
	return statusBar;
}

- (XAP_CocoaFrameImpl *)frameImpl
{
	return m_frame;
}

- (NSMenuItem *)_aboutMenu
{
	return m_aboutMenu;
}

- (NSMenuItem *)_quitMenu
{
	return m_quitMenu;
}

- (NSMenuItem *)_preferenceMenu
{
	return m_preferenceMenu;
}


@end
