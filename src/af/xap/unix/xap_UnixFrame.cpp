/* AbiSource Application Framework
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
#include <stdio.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "ev_UnixKeyboard.h"
#include "ev_UnixMouse.h"
#include "ev_UnixMenuBar.h"
#include "ev_UnixMenuPopup.h"
#include "ev_UnixToolbar.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "xad_Document.h"


/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/****************************************************************/
void XAP_UnixFrame::_fe::realize(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
  GdkICAttr *ic_attr=(GdkICAttr *)gtk_object_get_data(GTK_OBJECT(widget), "ic_attr");
  GdkIC * ic=(GdkIC *)gtk_object_get_data(GTK_OBJECT(widget), "ic");
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
  gtk_object_set_data(GTK_OBJECT(widget), "ic_attr", ic_attr);
  gtk_object_set_data(GTK_OBJECT(widget), "ic", ic);
}

void XAP_UnixFrame::_fe::unrealize(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
  GdkICAttr *ic_attr=(GdkICAttr *)gtk_object_get_data(GTK_OBJECT(widget), "ic_attr");
  GdkIC * ic=(GdkIC *)gtk_object_get_data(GTK_OBJECT(widget), "ic");
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
  gtk_object_set_data(GTK_OBJECT(widget), "ic_attr", ic_attr);
  gtk_object_set_data(GTK_OBJECT(widget), "ic", ic);
}

void XAP_UnixFrame::_fe::sizeAllocate(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
  GdkICAttr *ic_attr=(GdkICAttr *)gtk_object_get_data(GTK_OBJECT(widget), "ic_attr");
  GdkIC * ic=(GdkIC *)gtk_object_get_data(GTK_OBJECT(widget), "ic");
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

gint XAP_UnixFrame::_fe::focusIn(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
  GdkIC * ic=(GdkIC *)gtk_object_get_data(GTK_OBJECT(widget), "ic");
  if (ic)
    gdk_im_begin (ic, widget->window);
  return FALSE;
}

gint XAP_UnixFrame::_fe::focusOut(GtkWidget * /* w*/, GdkEvent * /*e*/,gpointer /*data*/)
{
  gdk_im_end ();
  return FALSE;
}
gboolean XAP_UnixFrame::_fe::focus_in_event(GtkWidget *w,GdkEvent */*event*/,gpointer /*user_data*/)
{
	XAP_UnixFrame * pFrame = (XAP_UnixFrame *) gtk_object_get_user_data(GTK_OBJECT(w));
	UT_ASSERT(pFrame);
	gtk_object_set_data(GTK_OBJECT(w), "toplevelWindowFocus",
						GINT_TO_POINTER(TRUE));
	pFrame->getCurrentView()->focusChange(gtk_grab_get_current() == NULL || gtk_grab_get_current() == w ? AV_FOCUS_HERE : AV_FOCUS_NEARBY);
	return FALSE;
}

gboolean XAP_UnixFrame::_fe::focus_out_event(GtkWidget *w,GdkEvent */*event*/,gpointer /*user_data*/)
{
	XAP_UnixFrame * pFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_ASSERT(pFrame);
	gtk_object_set_data(GTK_OBJECT(w), "toplevelWindowFocus",
						GINT_TO_POINTER(FALSE));
	pFrame->getCurrentView()->focusChange(AV_FOCUS_NONE);
	return FALSE;
}

gint XAP_UnixFrame::_fe::button_press_event(GtkWidget * w, GdkEventButton * e)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	pUnixFrame->setTimeOfLastEvent(e->time);
	AV_View * pView = pUnixFrame->getCurrentView();
	EV_UnixMouse * pUnixMouse = static_cast<EV_UnixMouse *>(pUnixFrame->getMouse());

	//UT_DEBUGMSG(("Grabbing mouse.\n"));
	gtk_grab_add(w);
	
	if (pView)
		pUnixMouse->mouseClick(pView,e);
	return 1;
}

gint XAP_UnixFrame::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	pUnixFrame->setTimeOfLastEvent(e->time);
	AV_View * pView = pUnixFrame->getCurrentView();
	EV_UnixMouse * pUnixMouse = static_cast<EV_UnixMouse *>(pUnixFrame->getMouse());

	//UT_DEBUGMSG(("Ungrabbing mouse.\n"));
	gtk_grab_remove(w);
	
	if (pView)
		pUnixMouse->mouseUp(pView,e);
	
	return 1;
}
	
gint XAP_UnixFrame::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// This is basically a resize event.
		
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	AV_View * pView = pUnixFrame->getCurrentView();

	if (pView)
		pView->setWindowSize(e->width, e->height);

	return 1;
}
	
gint XAP_UnixFrame::_fe::motion_notify_event(GtkWidget* w, GdkEventMotion* e)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	pUnixFrame->setTimeOfLastEvent(e->time);
	AV_View * pView = pUnixFrame->getCurrentView();
	EV_UnixMouse * pUnixMouse = static_cast<EV_UnixMouse *>(pUnixFrame->getMouse());
	
	if (pView)
		pUnixMouse->mouseMotion(pView, e);
	
	return 1;
}
	
gint XAP_UnixFrame::_fe::key_press_event(GtkWidget* w, GdkEventKey* e)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	pUnixFrame->setTimeOfLastEvent(e->time);
	AV_View * pView = pUnixFrame->getCurrentView();
	ev_UnixKeyboard * pUnixKeyboard = static_cast<ev_UnixKeyboard *>(pUnixFrame->getKeyboard());
	
	if (pView)
		pUnixKeyboard->keyPressEvent(pView, e);

	// HACK : This one's ugly.  If we continue through the callback chain,
	// HACK : GTK will pick up key presses and hand them off to widgets with
	// HACK : focus (like toolbar combos).  This is bad, since we have already
	// HACK : acted on the key press (like space, we would insert a space
	// HACK : in the document, but GTK will let space mean "open the menu"
	// HACK : to a combo).  The user is confused and things are annoying.
	// HACK :
	// HACK : We _could_ block all GTK key handling, and do everything 
	// HACK : ourselves, but then we lose the automatic menu accelerator
	// HACK : bindings (Alt-F for File menu).  
	// HACK :
	// HACK : What we do is let ONLY Alt-modified keys through to GTK.

	// If a modifier is down, return to let GTK catch

	// don't let GTK handle keys when mod2 (numlock) or mod5 (scroll lock) are down

	// What's "LOCK_MASK"?  I can't seem to trigger it with caps lock, scroll lock, or
	// num lock.
//		(e->state & GDK_LOCK_MASK))		// catch all keys with "num lock" down for now
	
	if ((e->state & GDK_MOD1_MASK) ||
		(e->state & GDK_MOD3_MASK) ||
		(e->state & GDK_MOD4_MASK))
	{
		return 0;
	}

	// ... else, stop this signal
	gtk_signal_emit_stop_by_name(GTK_OBJECT(w), "key_press_event");
	return 1;
}
	
gint XAP_UnixFrame::_fe::delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *) gtk_object_get_user_data(GTK_OBJECT(w));
	XAP_App * pApp = pUnixFrame->getApp();
	UT_ASSERT(pApp);

	const EV_Menu_ActionSet * pMenuActionSet = pApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
	UT_ASSERT(pEMC);
	
	const EV_EditMethod * pEM = pEMC->findEditMethodByName("closeWindow");
	UT_ASSERT(pEM);
	
	if (pEM)
	{
		if ((*pEM->getFn())(pUnixFrame->getCurrentView(),NULL))
		{
			// returning FALSE means destroy the window, continue along the
			// chain of Gtk destroy events

			return FALSE;
		}
	}
		
	// returning TRUE means do NOT destroy the window; halt the message
	// chain so it doesn't see destroy
	return TRUE;
}
	
gint XAP_UnixFrame::_fe::expose(GtkWidget * w, GdkEventExpose* pExposeEvent)
{
	UT_Rect rClip;
	rClip.left = pExposeEvent->area.x;
	rClip.top = pExposeEvent->area.y;
	rClip.width = pExposeEvent->area.width;
	rClip.height = pExposeEvent->area.height;
	
	//UT_DEBUGMSG(("gtk expose:  left=%d, top=%d, width=%d, height=%d\n", rClip.left, rClip.top, rClip.width, rClip.height));
	
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	AV_View * pView = pUnixFrame->getCurrentView();
	if (pView)
	{
		pView->draw(&rClip);
	}
	return 0;
}
	
void XAP_UnixFrame::_fe::vScrollChanged(GtkAdjustment * w, gpointer /*data*/)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	AV_View * pView = pUnixFrame->getCurrentView();
	
	//UT_DEBUGMSG(("gtk vScroll: value %ld\n",(UT_sint32)w->value));
	
	if (pView)
		pView->sendVerticalScrollEvent((UT_sint32) w->value);
}
	
void XAP_UnixFrame::_fe::hScrollChanged(GtkAdjustment * w, gpointer /*data*/)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	AV_View * pView = pUnixFrame->getCurrentView();
	
	if (pView)
		pView->sendHorizontalScrollEvent((UT_sint32) w->value);
}
	
void XAP_UnixFrame::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
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
	
/*****************************************************************/

XAP_UnixFrame::XAP_UnixFrame(XAP_UnixApp * app)
	: XAP_Frame(static_cast<XAP_App *>(app)),
	  m_dialogFactory(this, static_cast<XAP_App *>(app))
{
	m_pUnixApp = app;
	m_pUnixMenu = NULL;
	m_pUnixPopup = NULL;
	m_pView = NULL;
}

// TODO when cloning a new frame from an existing one
// TODO should we also clone any frame-persistent
// TODO dialog data ??

XAP_UnixFrame::XAP_UnixFrame(XAP_UnixFrame * f)
	: XAP_Frame(static_cast<XAP_Frame *>(f)),
	  m_dialogFactory(this, static_cast<XAP_App *>(f->m_pUnixApp))
{
	m_pUnixApp = f->m_pUnixApp;
	m_pUnixMenu = NULL;
	m_pUnixPopup = NULL;
	m_pView = NULL;
}

XAP_UnixFrame::~XAP_UnixFrame(void)
{
	// only delete the things we created...
	
	DELETEP(m_pUnixMenu);
	DELETEP(m_pUnixPopup);
}

UT_Bool XAP_UnixFrame::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
								  const char * szMenuLayoutKey, const char * szMenuLayoutDefaultValue,
								  const char * szMenuLabelSetKey, const char * szMenuLabelSetDefaultValue,
								  const char * szToolbarLayoutsKey, const char * szToolbarLayoutsDefaultValue,
								  const char * szToolbarLabelSetKey, const char * szToolbarLabelSetDefaultValue)
{
	UT_Bool bResult;

	// invoke our base class first.
	
	bResult = XAP_Frame::initialize(szKeyBindingsKey, szKeyBindingsDefaultValue,
									szMenuLayoutKey, szMenuLayoutDefaultValue,
									szMenuLabelSetKey, szMenuLabelSetDefaultValue,
									szToolbarLayoutsKey, szToolbarLayoutsDefaultValue,
									szToolbarLabelSetKey, szToolbarLabelSetDefaultValue);
	UT_ASSERT(bResult);

   	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.

	EV_EditEventMapper * pEEM = getEditEventMapper();
	UT_ASSERT(pEEM);

	m_pKeyboard = new ev_UnixKeyboard(pEEM);
	UT_ASSERT(m_pKeyboard);
	
	m_pMouse = new EV_UnixMouse(pEEM);
	UT_ASSERT(m_pMouse);

	return UT_TRUE;
}

UT_sint32 XAP_UnixFrame::setInputMode(const char * szName)
{
	UT_sint32 result = XAP_Frame::setInputMode(szName);
	if (result == 1)
	{
		// if it actually changed we need to update keyboard and mouse

		EV_EditEventMapper * pEEM = getEditEventMapper();
		UT_ASSERT(pEEM);

		m_pKeyboard->setEditEventMap(pEEM);
		m_pMouse->setEditEventMap(pEEM);
	}

	return result;
}

GtkWidget * XAP_UnixFrame::getTopLevelWindow(void) const
{
	return m_wTopLevelWindow;
}

GtkWidget * XAP_UnixFrame::getVBoxWidget(void) const
{
	return m_wVBox;
}

XAP_DialogFactory * XAP_UnixFrame::getDialogFactory(void)
{
	return &m_dialogFactory;
}

void XAP_UnixFrame::_createTopLevelWindow(void)
{
	// create a top-level window for us.

	UT_Bool bResult;

	m_wTopLevelWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "ic_attr", NULL);
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "ic", NULL);
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "toplevelWindow",
						m_wTopLevelWindow);
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "toplevelWindowFocus",
						GINT_TO_POINTER(FALSE));
	gtk_object_set_user_data(GTK_OBJECT(m_wTopLevelWindow),this);
	gtk_window_set_title(GTK_WINDOW(m_wTopLevelWindow),
						 m_pUnixApp->getApplicationTitleForTitleBar());
	gtk_window_set_policy(GTK_WINDOW(m_wTopLevelWindow), TRUE, TRUE, FALSE);
	gtk_window_set_wmclass(GTK_WINDOW(m_wTopLevelWindow),
						   m_pUnixApp->getApplicationName(),
						   m_pUnixApp->getApplicationName());

	// This is now done with --geometry parsing.
	//gtk_widget_set_usize(GTK_WIDGET(m_wTopLevelWindow), 700, 650);

	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "realize",
					   GTK_SIGNAL_FUNC(_fe::realize), NULL);
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "unrealize",
					   GTK_SIGNAL_FUNC(_fe::unrealize), NULL);
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "size_allocate",
					   GTK_SIGNAL_FUNC(_fe::sizeAllocate), NULL);
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "focus_in_event",
					   GTK_SIGNAL_FUNC(_fe::focusIn), NULL);
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "focus_out_event",
					   GTK_SIGNAL_FUNC(_fe::focusOut), NULL);

	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "delete_event",
					   GTK_SIGNAL_FUNC(_fe::delete_event), NULL);
	// here we connect the "destroy" event to a signal handler.  
	// This event occurs when we call gtk_widget_destroy() on the window,
	// or if we return 'FALSE' in the "delete_event" callback.
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "destroy",
					   GTK_SIGNAL_FUNC(_fe::destroy), NULL);

	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "focus_in_event",
					   GTK_SIGNAL_FUNC(_fe::focus_in_event), NULL);
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "focus_out_event",
					   GTK_SIGNAL_FUNC(_fe::focus_out_event), NULL);

	// create a VBox inside it.
	
	m_wVBox = gtk_vbox_new(FALSE,0);
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "vbox", m_wVBox);
	gtk_object_set_user_data(GTK_OBJECT(m_wVBox),this);
	gtk_container_add(GTK_CONTAINER(m_wTopLevelWindow), m_wVBox);

	// synthesize a menu from the info in our base class.

	m_pUnixMenu = new EV_UnixMenuBar(m_pUnixApp,this,
									 m_szMenuLayoutName,
									 m_szMenuLabelSetName);
	UT_ASSERT(m_pUnixMenu);
	bResult = m_pUnixMenu->synthesizeMenuBar();
	UT_ASSERT(bResult);

	// create a toolbar instance for each toolbar listed in our base class.
	// TODO for some reason, the toolbar functions require the TLW to be
	// TODO realized (they reference m_wTopLevelWindow->window) before we call them.

	gtk_widget_realize(m_wTopLevelWindow);



	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "key_press_event",
					   GTK_SIGNAL_FUNC(_fe::key_press_event), NULL);


	_createToolbars();

	// Let the app-specific frame code create the contents of
	// the child area of the window (between the toolbars and
	// the status bar).

	m_wSunkenBox = _createDocumentWindow();
	gtk_container_add(GTK_CONTAINER(m_wVBox), m_wSunkenBox);
	gtk_widget_show(m_wSunkenBox);

	// Let the app-specific frame code create the status bar
	// if it wants to.  we will put it below the document
	// window (a peer with toolbars and the overall sunkenbox)
	// so that it will appear outside of the scrollbars.

	m_wStatusBar = _createStatusBarWindow();
	if (m_wStatusBar)
	{
		gtk_widget_show(m_wStatusBar);
		gtk_box_pack_end(GTK_BOX(m_wVBox), m_wStatusBar, FALSE, FALSE, 0);
	}
	
	gtk_widget_show(m_wVBox);
	
	// set the icon
	_setWindowIcon();

	// set geometry hints as the user requested
	gint x, y;
	guint width, height;
	XAP_UnixApp::windowGeometryFlags f;

	m_pUnixApp->getGeometry(&x, &y, &width, &height, &f);

	// Set the size if requested

	if (f & XAP_UnixApp::GEOMETRY_FLAG_SIZE)
	{
                gint abi_width = UT_MIN( gdk_screen_width() - 30, 950);
                gint abi_height = UT_MIN( gdk_screen_height() - 50, 1200);
	        gtk_widget_set_usize(m_wTopLevelWindow,
							 abi_width,
							 abi_height);
	}


	// Because we're clever, we only honor this flag when we
	// are the first (well, only) top level frame available.
	// This is so the user's window manager can find better
	// places for new windows, instead of having our windows
	// pile upon each other.

	if (m_pUnixApp->getFrameCount() <= 1)
		if (f & XAP_UnixApp::GEOMETRY_FLAG_POS)
			gtk_widget_set_uposition(m_wTopLevelWindow,
									 x,
									 y);

	// we let our caller decide when to show m_wTopLevelWindow.
	return;
}

UT_Bool XAP_UnixFrame::close()
{
	gtk_widget_destroy(getTopLevelWindow());
	return UT_TRUE;
}

UT_Bool XAP_UnixFrame::raise()
{
	GtkWidget * tlw = getTopLevelWindow();
	UT_ASSERT(tlw);
	
	gdk_window_raise(tlw->window);

	return UT_TRUE;
}

UT_Bool XAP_UnixFrame::show()
{
	gtk_widget_show(m_wTopLevelWindow);

	return UT_TRUE;
}

UT_Bool XAP_UnixFrame::openURL(const char * szURL)
{
	// TODO : FIX THIS.  Find a better way to search for
	// TODO : other browsers on your machine.

	// Try to connect to a running Netscape, if not, start new one

	char execstring[4096];

	g_snprintf(execstring, 4096, "netscape -remote openURL\\(%s\\) "
			   "|| netscape %s &", szURL, szURL);
	system(execstring);
	
	return UT_FALSE;
}

UT_Bool XAP_UnixFrame::updateTitle()
{
	if (!XAP_Frame::updateTitle())
	{
		// no relevant change, so skip it
		return UT_FALSE;
	}

	char buf[256];
	buf[0] = 0;

	const char * szAppName = m_pUnixApp->getApplicationTitleForTitleBar();

	int len = 256 - strlen(szAppName) - 4;
	
	const char * szTitle = getTitle(len);

	sprintf(buf, "%s - %s", szTitle, szAppName);
	
	gtk_window_set_title(GTK_WINDOW(m_wTopLevelWindow), buf);

	return UT_TRUE;
}

/*****************************************************************/

static void s_gtkMenuPositionFunc(GtkMenu * /* menu */, gint * x, gint * y, gpointer user_data)
{
	struct UT_Point * p = (struct UT_Point *)user_data;
	
	*x = p->x;
	*y = p->y;
}

UT_Bool XAP_UnixFrame::runModalContextMenu(AV_View * /* pView */, const char * szMenuName,
										   UT_sint32 x, UT_sint32 y)
{
	UT_Bool bResult = UT_TRUE;

	UT_ASSERT(!m_pUnixPopup);

	m_pUnixPopup = new EV_UnixMenuPopup(m_pUnixApp,this,szMenuName,m_szMenuLabelSetName);
	if (m_pUnixPopup && m_pUnixPopup->synthesizeMenuPopup())
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
		gtk_menu_popup(GTK_MENU(m_pUnixPopup->getMenuHandle()), NULL, NULL,
					   s_gtkMenuPositionFunc, &pt, 3, 0);

		// We run this menu synchronously, since GTK doesn't.
		// Popup menus have a special "unmap" function to call
		// gtk_main_quit() when they're done.
		gtk_main();
	}

	DELETEP(m_pUnixPopup);
	return bResult;
}

void XAP_UnixFrame::setTimeOfLastEvent(guint32 eventTime)
{
	m_pUnixApp->setTimeOfLastEvent(eventTime);
}

EV_Toolbar * XAP_UnixFrame::_newToolbar(XAP_App *app, XAP_Frame *frame,
					const char *szLayout,
					const char *szLanguage)
{
	return (new EV_UnixToolbar(static_cast<XAP_UnixApp *>(app), 
							   static_cast<XAP_UnixFrame *>(frame), 
							   szLayout, szLanguage));
}
