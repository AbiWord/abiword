/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "ev_UnixKeyboard.h"
#include "ev_UnixMouse.h"
#include "ev_UnixMenu.h"
#include "ev_UnixToolbar.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "xad_Document.h"

/*****************************************************************/

#define DELETEP(p)		do { if (p) delete p; } while (0)
#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/****************************************************************/

gint XAP_UnixFrame::_fe::button_press_event(GtkWidget * w, GdkEventButton * e)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	AV_View * pView = pUnixFrame->getCurrentView();
	EV_UnixMouse * pUnixMouse = pUnixFrame->getUnixMouse();
	
	if (pView)
		pUnixMouse->mouseClick(pView,e);
	return 1;
}

gint XAP_UnixFrame::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	AV_View * pView = pUnixFrame->getCurrentView();
	EV_UnixMouse * pUnixMouse = pUnixFrame->getUnixMouse();
		
	if (pView)
		pUnixMouse->mouseUp(pView,e);
	return 1;
}
	
gint XAP_UnixFrame::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// This is basically a resize event.
		
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	AV_View * pView = pUnixFrame->getCurrentView();

	// TODO I've commented out the following block and the
	// TODO call to draw() a little further down, since it
	// TODO seems to cause excessive flashing -- a flash of
	// TODO white before we redraw the page-view.  When we
	// TODO were just doing a normal-view, this was not
	// TODO noticable, but it is now.  The call to draw()
	// TODO also caused us to completely redraw the page,
	// TODO but this is unnecessary since we'll get one or
	// TODO two (don't ask me why) expose events shortly.
	// TODO I've left the code here, until we've verified
	// TODO this doesn't mess anything up -- Jeff 1/4/99
//		GdkColor clr;
//		clr.red = 255 << 8;
//		clr.green = 255 << 8;
//		clr.blue = 255 << 8;
//		GdkColormap*  pColormap = gdk_colormap_get_system();
//		gdk_color_alloc(pColormap, &clr);
//		gdk_window_set_background(w->window, &clr);
	
	if (pView)
	{
		pView->setWindowSize(e->width, e->height);
//		pView->draw();
	}
	return 1;
}
	
gint XAP_UnixFrame::_fe::motion_notify_event(GtkWidget* w, GdkEventMotion* e)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	AV_View * pView = pUnixFrame->getCurrentView();
	EV_UnixMouse * pUnixMouse = pUnixFrame->getUnixMouse();
	
	if (pView)
	{
		if (e->state & GDK_BUTTON1_MASK)
		{
			pUnixMouse->mouseMotion(pView, e);
		}
	}
	
	return 1;
}
	
gint XAP_UnixFrame::_fe::key_press_event(GtkWidget* w, GdkEventKey* e)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	AV_View * pView = pUnixFrame->getCurrentView();
	ev_UnixKeyboard * pUnixKeyboard = pUnixFrame->getUnixKeyboard();
		
	if (pView)
	{
		pUnixKeyboard->keyPressEvent(pView, e);
	}
	return 1;
}
	
gint XAP_UnixFrame::_fe::delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *) gtk_object_get_user_data(GTK_OBJECT(w));
	AP_App * pApp = pUnixFrame->getApp();
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
	
	UT_DEBUGMSG(("gtk expose:  left=%d, top=%d, width=%d, height=%d\n", rClip.left, rClip.top, rClip.width, rClip.height));
	
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
	
	UT_DEBUGMSG(("gtk vScroll: value %ld\n",(UT_sint32)w->value));
	
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

XAP_UnixFrame::XAP_UnixFrame(AP_UnixApp * app)
	: XAP_Frame(static_cast<AP_App *>(app)),
	  m_dialogFactory(this)
{
	m_pUnixApp = app;
	m_pUnixKeyboard = NULL;
	m_pUnixMouse = NULL;
	m_pUnixMenu = NULL;
	m_pView = NULL;
}

// TODO when cloning a new frame from an existing one
// TODO should we also clone any frame-persistent
// TODO dialog data ??

XAP_UnixFrame::XAP_UnixFrame(XAP_UnixFrame * f)
	: XAP_Frame(static_cast<XAP_Frame *>(f)),
	  m_dialogFactory(this)
{
	m_pUnixApp = f->m_pUnixApp;
	m_pUnixKeyboard = NULL;
	m_pUnixMouse = NULL;
	m_pUnixMenu = NULL;
	m_pView = NULL;
}

XAP_UnixFrame::~XAP_UnixFrame(void)
{
	// only delete the things we created...
	
	DELETEP(m_pUnixKeyboard);
	DELETEP(m_pUnixMouse);
	DELETEP(m_pUnixMenu);
	UT_VECTOR_PURGEALL(EV_UnixToolbar *, m_vecUnixToolbars);
}

UT_Bool XAP_UnixFrame::initialize(void)
{
	UT_Bool bResult;

	// invoke our base class first.
	
	bResult = XAP_Frame::initialize();
	UT_ASSERT(bResult);

	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	
	m_pUnixKeyboard = new ev_UnixKeyboard(m_pEEM);
	UT_ASSERT(m_pUnixKeyboard);
	
	m_pUnixMouse = new EV_UnixMouse(m_pEEM);
	UT_ASSERT(m_pUnixMouse);

	return UT_TRUE;
}


GtkWidget * XAP_UnixFrame::getTopLevelWindow(void) const
{
	return m_wTopLevelWindow;
}

GtkWidget * XAP_UnixFrame::getVBoxWidget(void) const
{
	return m_wVBox;
}

EV_UnixMouse * XAP_UnixFrame::getUnixMouse(void)
{
	return m_pUnixMouse;
}

ev_UnixKeyboard * XAP_UnixFrame::getUnixKeyboard(void)
{
	return m_pUnixKeyboard;
}

AP_DialogFactory * XAP_UnixFrame::getDialogFactory(void)
{
	return &m_dialogFactory;
}

void XAP_UnixFrame::_createTopLevelWindow(void)
{
	// create a top-level window for us.

	UT_Bool bResult;

	m_wTopLevelWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "toplevelWindow",
						m_wTopLevelWindow);
	gtk_object_set_user_data(GTK_OBJECT(m_wTopLevelWindow),this);
	gtk_window_set_title(GTK_WINDOW(m_wTopLevelWindow),
						 m_pUnixApp->getApplicationTitleForTitleBar());
	gtk_window_set_policy(GTK_WINDOW(m_wTopLevelWindow), TRUE, TRUE, FALSE);
	gtk_window_set_wmclass(GTK_WINDOW(m_wTopLevelWindow),
						   m_pUnixApp->getApplicationName(),
						   m_pUnixApp->getApplicationName());

	// TODO get the following values from a preferences or something.
	gtk_container_set_border_width(GTK_CONTAINER(m_wTopLevelWindow), 4);
	
	gtk_widget_set_usize(GTK_WIDGET(m_wTopLevelWindow), 700,650);

	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "delete_event",
						GTK_SIGNAL_FUNC(_fe::delete_event), NULL);
	// here we connect the "destroy" event to a signal handler.  
	// This event occurs when we call gtk_widget_destroy() on the window,
	// or if we return 'FALSE' in the "delete_event" callback.
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "destroy",
						GTK_SIGNAL_FUNC(_fe::destroy), NULL);

	// create a VBox inside it.
	
	m_wVBox = gtk_vbox_new(FALSE,0);
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "vbox", m_wVBox);
	gtk_object_set_user_data(GTK_OBJECT(m_wVBox),this);
	gtk_container_add(GTK_CONTAINER(m_wTopLevelWindow), m_wVBox);

	// TODO get the following values from a preferences or something.

// HEY! This is commented out because versions of GTK > than 1.1.5
//	call this gtk_container_SET_border_width() and commenting it out
//  makes it build everywhere, and it doesn't look any different.

//	gtk_container_set_border_width(GTK_CONTAINER(m_wVBox), 0);

	// synthesize a menu from the info in our base class.

	m_pUnixMenu = new EV_UnixMenu(m_pUnixApp,this,
								  m_szMenuLayoutName,
								  m_szMenuLabelSetName);
	UT_ASSERT(m_pUnixMenu);
	bResult = m_pUnixMenu->synthesize();
	UT_ASSERT(bResult);

	// create a toolbar instance for each toolbar listed in our base class.
	// TODO for some reason, the toolbar functions require the TLW to be
	// TODO realized (they reference m_wTopLevelWindow->window) before we call them.
	gtk_widget_realize(m_wTopLevelWindow);

	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "key_press_event",
					   GTK_SIGNAL_FUNC(_fe::key_press_event), NULL);

	UT_uint32 nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (UT_uint32 k=0; k < nrToolbars; k++)
	{
		EV_UnixToolbar * pUnixToolbar
			= new EV_UnixToolbar(m_pUnixApp,this,
								 (const char *)m_vecToolbarLayoutNames.getNthItem(k),
								 m_szToolbarLabelSetName);
		UT_ASSERT(pUnixToolbar);
		bResult = pUnixToolbar->synthesize();
		UT_ASSERT(bResult);

		m_vecUnixToolbars.addItem(pUnixToolbar);
	}

	// Let the app-specific frame code create the contents of
	// the child area of the window (between the toolbars and
	// the status bar).

	m_wSunkenBox = _createDocumentWindow();
	gtk_container_add(GTK_CONTAINER(m_wVBox), m_wSunkenBox);
	gtk_widget_show(m_wSunkenBox);

	// TODO decide what to do with accelerators
	// gtk_window_add_accelerator_table(GTK_WINDOW(window), accel);

	gtk_widget_show(m_wVBox);

	
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
