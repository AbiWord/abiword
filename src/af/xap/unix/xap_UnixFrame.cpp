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
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_files.h"
#include "ut_sleep.h"
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
#include "fv_View.h"
#include "xad_Document.h"
#include "gr_Graphics.h"
#include "xap_UnixDialogHelper.h"

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/****************************************************************/
void XAP_UnixFrame::_fe::realize(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
}

void XAP_UnixFrame::_fe::unrealize(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
}

void XAP_UnixFrame::_fe::sizeAllocate(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
}

gint XAP_UnixFrame::_fe::focusIn(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
  return FALSE;
}

gint XAP_UnixFrame::_fe::focusOut(GtkWidget * /* w*/, GdkEvent * /*e*/,gpointer /*data*/)
{
  return FALSE;
}
gboolean XAP_UnixFrame::_fe::focus_in_event(GtkWidget *w,GdkEvent */*event*/,gpointer /*user_data*/)
{
	XAP_UnixFrame * pFrame = (XAP_UnixFrame *) gtk_object_get_user_data(GTK_OBJECT(w));
	UT_ASSERT(pFrame);
	g_object_set_data(G_OBJECT(w), "toplevelWindowFocus",
						GINT_TO_POINTER(TRUE));
	if (pFrame->getCurrentView())
		pFrame->getCurrentView()->focusChange(gtk_grab_get_current() == NULL || gtk_grab_get_current() == w ? AV_FOCUS_HERE : AV_FOCUS_NEARBY);
	return FALSE;
}

gboolean XAP_UnixFrame::_fe::focus_out_event(GtkWidget *w,GdkEvent */*event*/,gpointer /*user_data*/)
{
	XAP_UnixFrame * pFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	UT_ASSERT(pFrame);
	g_object_set_data(G_OBJECT(w), "toplevelWindowFocus",
						GINT_TO_POINTER(FALSE));
	if (pFrame->getCurrentView())
		pFrame->getCurrentView()->focusChange(AV_FOCUS_NONE);
	return FALSE;
}

gint XAP_UnixFrame::_fe::button_press_event(GtkWidget * w, GdkEventButton * e)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	pUnixFrame->setTimeOfLastEvent(e->time);
	AV_View * pView = pUnixFrame->getCurrentView();
	EV_UnixMouse * pUnixMouse = static_cast<EV_UnixMouse *>(pUnixFrame->getMouse());

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

/*!
 * Background zoom updater. It updates the view zoom level after all configure
 * events have been processed. This is
 */
gint XAP_UnixFrame::_fe::do_ZoomUpdate(gpointer /* XAP_UnixFrame * */ p)
{
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(p);
	AV_View * pView = pUnixFrame->getCurrentView();
	if(!pView)
	{
		pUnixFrame->m_iZoomUpdateID = 0;
		pUnixFrame->m_bDoZoomUpdate = false;
		return FALSE;
	}
	if(pUnixFrame->m_bDoZoomUpdate && (pView->getWindowWidth() == pUnixFrame->m_iNewWidth) && (pView->getWindowHeight() == pUnixFrame->m_iNewHeight))
	{
		pUnixFrame->m_iZoomUpdateID = 0;
		pUnixFrame->m_bDoZoomUpdate = false;
		return FALSE;
	}

    pUnixFrame->m_bDoZoomUpdate = true;
	UT_sint32 iNewWidth = 0;
	UT_sint32 iNewHeight = 0;
	do
	{
		AV_View * pView = pUnixFrame->getCurrentView();
		if(!pView)
		{
			pUnixFrame->m_iZoomUpdateID = 0;
			pUnixFrame->m_bDoZoomUpdate = false;
			return FALSE;
		}
		while(pView->isLayoutFilling())
		{
//
// Comeback when it's finished.
//
			return TRUE;
		}
		iNewWidth = pUnixFrame->m_iNewWidth;
		iNewHeight = pUnixFrame->m_iNewHeight;
		pView = pUnixFrame->getCurrentView();
		if(pView)
		{
			pUnixFrame->_startViewAutoUpdater();
			pView->setWindowSize(iNewWidth, iNewHeight);
			pUnixFrame->updateZoom();
		}
		else
		{
			pUnixFrame->m_iZoomUpdateID = 0;
			pUnixFrame->m_bDoZoomUpdate = false;
			return FALSE;
		}
	}
	while((iNewWidth != pUnixFrame->m_iNewWidth) || (iNewHeight != pUnixFrame->m_iNewHeight));
	pUnixFrame->m_iZoomUpdateID = 0;
	pUnixFrame->m_bDoZoomUpdate = false;
	return FALSE;
}

gint XAP_UnixFrame::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// This is basically a resize event.

	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	AV_View * pView = pUnixFrame->getCurrentView();

	if (pView)
	{
		pUnixFrame->m_iNewWidth = e->width;
		pUnixFrame->m_iNewHeight = e->height;
		// Dynamic Zoom Implimentation
		if(!pUnixFrame->m_bDoZoomUpdate && (pUnixFrame->m_iZoomUpdateID == 0))
		{
			pUnixFrame->m_iZoomUpdateID = gtk_idle_add((GtkFunction) do_ZoomUpdate, (gpointer) pUnixFrame);
		}
	}
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

gint XAP_UnixFrame::_fe::scroll_notify_event(GtkWidget* w, GdkEventScroll* e)
{
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	pUnixFrame->setTimeOfLastEvent(e->time);
	AV_View * pView = pUnixFrame->getCurrentView();
	EV_UnixMouse * pUnixMouse = static_cast<EV_UnixMouse *>(pUnixFrame->getMouse());

	if (pView)
		pUnixMouse->mouseScroll(pView, e);

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

	const EV_EditMethod * pEM = pEMC->findEditMethodByName("closeWindowX");
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
	xxx_UT_DEBUGMSG(("gtk in Frame expose:  left=%d, top=%d, width=%d, height=%d\n", rClip.left, rClip.top, rClip.width, rClip.height));
	XAP_UnixFrame * pUnixFrame = (XAP_UnixFrame *)gtk_object_get_user_data(GTK_OBJECT(w));
	FV_View * pView = (FV_View *) pUnixFrame->getCurrentView();
	if(pView)
	{
		GR_Graphics * pG = pView->getGraphics();
		pG->doRepaint(&rClip);
	}
	return 0;
}

/*!
 * Background abi repaint function.
\param XAP_UnixFrame * p pointer to the Frame that initiated this background
       repainter.
 */
gint XAP_UnixFrame::_fe::abi_expose_repaint( gpointer p)
{
//
// Grab our pointer so we can do useful stuff.
//
	UT_Rect localCopy;
	XAP_UnixFrame * pF = static_cast<XAP_UnixFrame *>(p);
	FV_View * pV = (FV_View *) pF->getCurrentView();
	if(!pV || (pV->getPoint() == 0))
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
		xxx_UT_DEBUGMSG(("Painting area:  left=%d, top=%d, width=%d, height=%d\n", localCopy.left, localCopy.top, localCopy.width, localCopy.height));
		xxx_UT_DEBUGMSG(("SEVIOR: Repaint now \n"));
		pV->draw(&localCopy);
	}
//
// OK we've finshed. Wait for the next signal
//
	pG->setSpawnedRedraw(false);
	return TRUE;
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
	m_iAbiRepaintID = 0;
	m_bDoZoomUpdate = false;
	m_iZoomUpdateID = 0;
	m_wTopLevelWindow = NULL;
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
	m_iAbiRepaintID = 0;
	m_bDoZoomUpdate = false;
	m_iZoomUpdateID = 0;
}

XAP_UnixFrame::~XAP_UnixFrame(void)
{
	// only delete the things we created...
	if(m_iAbiRepaintID)
	{
		gtk_timeout_remove(m_iAbiRepaintID);
	}
	if(m_bDoZoomUpdate)
	{
		gtk_timeout_remove(m_iZoomUpdateID);
	}
	DELETEP(m_pUnixMenu);
	DELETEP(m_pUnixPopup);
}

bool XAP_UnixFrame::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
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

   	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.

	EV_EditEventMapper * pEEM = getEditEventMapper();
	UT_ASSERT(pEEM);

	m_pKeyboard = new ev_UnixKeyboard(pEEM);
	UT_ASSERT(m_pKeyboard);

	m_pMouse = new EV_UnixMouse(pEEM);
	UT_ASSERT(m_pMouse);

//
// Start background repaint
//
	if(m_iAbiRepaintID == 0)
	{
		m_iAbiRepaintID = gtk_timeout_add(100,(GtkFunction) _fe::abi_expose_repaint, (gpointer) this);
	}
	else
	{
		gtk_timeout_remove(m_iAbiRepaintID);
		m_iAbiRepaintID = gtk_timeout_add(100,(GtkFunction) _fe::abi_expose_repaint, (gpointer) this);
	}
	return true;
}


void XAP_UnixFrame::setCursor(GR_Graphics::Cursor c)
{
//	if (m_cursor == c)
//		return;
//	m_cursor = c;
	FV_View * pView = (FV_View *) getCurrentView();
	if(pView)
	{
		GR_Graphics * pG = pView->getGraphics();
		if(pG && pG->queryProperties( GR_Graphics::DGP_PAPER))
		{
			return;
		}
	}
	if(getTopLevelWindow() == NULL || (m_iFrameMode != XAP_NormalFrame))
	{
		return;
	}
	GdkCursorType cursor_number;

	switch (c)
	{
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		/*FALLTHRU*/
	case GR_Graphics::GR_CURSOR_DEFAULT:
		cursor_number = GDK_TOP_LEFT_ARROW;
		break;

	case GR_Graphics::GR_CURSOR_IBEAM:
		cursor_number = GDK_XTERM;
		break;

	//I have changed the shape of the arrow so get a consistent
	//behaviour in the bidi build; I think the new arrow is better
	//for the purpose anyway

	case GR_Graphics::GR_CURSOR_RIGHTARROW:
		cursor_number = GDK_SB_RIGHT_ARROW; //GDK_ARROW;
		break;

	case GR_Graphics::GR_CURSOR_LEFTARROW:
		cursor_number = GDK_SB_LEFT_ARROW; //GDK_LEFT_PTR;
		break;

	case GR_Graphics::GR_CURSOR_IMAGE:
		cursor_number = GDK_FLEUR;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_NW:
		cursor_number = GDK_TOP_LEFT_CORNER;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_N:
		cursor_number = GDK_TOP_SIDE;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_NE:
		cursor_number = GDK_TOP_RIGHT_CORNER;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_E:
		cursor_number = GDK_RIGHT_SIDE;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_SE:
		cursor_number = GDK_BOTTOM_RIGHT_CORNER;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_S:
		cursor_number = GDK_BOTTOM_SIDE;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_SW:
		cursor_number = GDK_BOTTOM_LEFT_CORNER;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_W:
		cursor_number = GDK_LEFT_SIDE;
		break;

	case GR_Graphics::GR_CURSOR_LEFTRIGHT:
		cursor_number = GDK_SB_H_DOUBLE_ARROW;
		break;

	case GR_Graphics::GR_CURSOR_UPDOWN:
		cursor_number = GDK_SB_V_DOUBLE_ARROW;
		break;

	case GR_Graphics::GR_CURSOR_EXCHANGE:
		cursor_number = GDK_EXCHANGE;
		break;

	case GR_Graphics::GR_CURSOR_GRAB:
		cursor_number = GDK_HAND1;
		break;

	case GR_Graphics::GR_CURSOR_LINK:
		cursor_number = GDK_HAND2;
		break;

	case GR_Graphics::GR_CURSOR_WAIT:
		cursor_number = GDK_WATCH;
		break;
	}

	GdkCursor * cursor = gdk_cursor_new(cursor_number);
	gdk_window_set_cursor(getTopLevelWindow()->window, cursor);
	gdk_window_set_cursor(getVBoxWidget()->window, cursor);
	gdk_window_set_cursor(m_wSunkenBox->window, cursor);
	gdk_window_set_cursor(m_wStatusBar->window, cursor);
	gdk_cursor_destroy(cursor);

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

void XAP_UnixFrame::nullUpdate() const
{
	UT_uint32 i =0;
	while(gtk_events_pending() && (i < 5))
	{
		gtk_main_iteration ();
		i++;
	}
}


void XAP_UnixFrame::_createTopLevelWindow(void)
{
	// create a top-level window for us.

	static GdkPixbuf * wmIcon = NULL ;

	// load the icon only once
	if (!wmIcon)
	{
		GError *err = NULL ;
		UT_String icon_location = XAP_App::getApp()->getAbiSuiteLibDir();
		icon_location += "/icons/abiword_16.xpm" ;
		wmIcon = gdk_pixbuf_new_from_file(icon_location.c_str(),&err);
	}
	
	bool bResult;
	if(m_iFrameMode == XAP_NormalFrame)
	{
		m_wTopLevelWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		g_object_set_data(G_OBJECT(m_wTopLevelWindow), "ic_attr", NULL);
		g_object_set_data(G_OBJECT(m_wTopLevelWindow), "ic", NULL);
		gtk_window_set_title(GTK_WINDOW(m_wTopLevelWindow),
							 m_pUnixApp->getApplicationTitleForTitleBar());
		gtk_window_set_policy(GTK_WINDOW(m_wTopLevelWindow), TRUE, TRUE, FALSE);
		gtk_window_set_wmclass(GTK_WINDOW(m_wTopLevelWindow),
							   m_pUnixApp->getApplicationName(),
							   m_pUnixApp->getApplicationName());

		if ( wmIcon )
			gtk_window_set_icon(GTK_WINDOW(m_wTopLevelWindow), wmIcon);
	}
	g_object_set_data(G_OBJECT(m_wTopLevelWindow), "toplevelWindow",
						m_wTopLevelWindow);
	g_object_set_data(G_OBJECT(m_wTopLevelWindow), "toplevelWindowFocus",
						GINT_TO_POINTER(FALSE));
	gtk_object_set_user_data(GTK_OBJECT(m_wTopLevelWindow),this);

	// This is now done with --geometry parsing.
	//gtk_widget_set_usize(GTK_WIDGET(m_wTopLevelWindow), 700, 650);

	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "realize",
					   G_CALLBACK(_fe::realize), NULL);
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "unrealize",
					   G_CALLBACK(_fe::unrealize), NULL);
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "size_allocate",
					   G_CALLBACK(_fe::sizeAllocate), NULL);
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "focus_in_event",
					   G_CALLBACK(_fe::focusIn), NULL);
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "focus_out_event",
					   G_CALLBACK(_fe::focusOut), NULL);

	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "delete_event",
					   G_CALLBACK(_fe::delete_event), NULL);
	// here we connect the "destroy" event to a signal handler.
	// This event occurs when we call gtk_widget_destroy() on the window,
	// or if we return 'FALSE' in the "delete_event" callback.
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "destroy",
					   G_CALLBACK(_fe::destroy), NULL);

	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "focus_in_event",
					   G_CALLBACK(_fe::focus_in_event), NULL);
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "focus_out_event",
					   G_CALLBACK(_fe::focus_out_event), NULL);

	// create a VBox inside it.

	m_wVBox = gtk_vbox_new(FALSE,0);
	g_object_set_data(G_OBJECT(m_wTopLevelWindow), "vbox", m_wVBox);
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


	if(m_iFrameMode == XAP_NormalFrame)
	{
		gtk_widget_realize(m_wTopLevelWindow);
	}


	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "key_press_event",
					   G_CALLBACK(_fe::key_press_event), NULL);


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
	m_wStatusBar = NULL;
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
	UT_uint32 f;

	m_pUnixApp->getWinGeometry(&x, &y, &width, &height, &f);

	// Get fall-back defaults from preferences
	UT_uint32 pref_flags, pref_width, pref_height;
	UT_sint32 pref_x, pref_y;
	m_pUnixApp->getPrefs()->getGeometry(&pref_x, &pref_y, &pref_width,
										&pref_height, &pref_flags);
	if (!(f & XAP_UnixApp::GEOMETRY_FLAG_SIZE)
		&& (pref_flags & PREF_FLAG_GEOMETRY_SIZE))
	{
		width = pref_width;
		height = pref_height;
		f |= XAP_UnixApp::GEOMETRY_FLAG_SIZE;
	}
	if (!(f & XAP_UnixApp::GEOMETRY_FLAG_POS)
		&& (pref_flags & PREF_FLAG_GEOMETRY_POS))
	{
		x = pref_x;
		y = pref_y;
		f |= XAP_UnixApp::GEOMETRY_FLAG_POS;
	}

	// Set the size if requested

	if (f & XAP_UnixApp::GEOMETRY_FLAG_SIZE)
	{
		gint abi_width = UT_MIN( gdk_screen_width() - 30, (gint)width);
		gint abi_height = UT_MIN( gdk_screen_height() - 100, (gint)height);
		gtk_widget_set_usize(m_wTopLevelWindow, abi_width, abi_height);
	}

	// Because we're clever, we only honor this flag when we
	// are the first (well, only) top level frame available.
	// This is so the user's window manager can find better
	// places for new windows, instead of having our windows
	// pile upon each other.
	if (m_pUnixApp->getFrameCount() <= 1)
	{
		if (f & XAP_UnixApp::GEOMETRY_FLAG_POS)
		{
			gtk_widget_set_uposition(m_wTopLevelWindow, x, y);
		}
	}

	// Remember geometry settings for next time
	m_pUnixApp->getPrefs()->setGeometry(x, y, width, height, f);

	// we let our caller decide when to show m_wTopLevelWindow.
	return;
}

/*!
 * This code is used by the dynamic menu API to rebuild the menus after a
 * a change in the menu structure.
 */
void XAP_UnixFrame::rebuildMenus(void)
{
//
// Destroy the old menu bar
//
	m_pUnixMenu->destroy();
//
// Delete the old class
//
	DELETEP(m_pUnixMenu);
//
// Build a new one.
//
	m_pUnixMenu = new EV_UnixMenuBar(m_pUnixApp,this,
									 m_szMenuLayoutName,
									 m_szMenuLabelSetName);
	UT_ASSERT(m_pUnixMenu);
	bool bResult = m_pUnixMenu->rebuildMenuBar();
	UT_ASSERT(bResult);

}


/*!
 * This code is used by the dynamic toolbar API to rebuild a toolbar after a
 * a change in the toolbar structure.
 */
void XAP_UnixFrame::rebuildToolbar(UT_uint32 ibar)
{
//
// Destroy the old toolbar
//
	EV_Toolbar * pToolbar = (EV_Toolbar *) m_vecToolbars.getNthItem(ibar);
	const char * szTBName = (const char *) m_vecToolbarLayoutNames.getNthItem(ibar);
	EV_UnixToolbar * pUTB = static_cast<EV_UnixToolbar *>( pToolbar);
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
	pToolbar = _newToolbar(m_app, (XAP_Frame *) this,szTBName,
						   (const char *) m_szToolbarLabelSetName);
	static_cast<EV_UnixToolbar *>(pToolbar)->rebuildToolbar(oldpos);
	m_vecToolbars.setNthItem(ibar, (void *) pToolbar, NULL);
//
// Refill the framedata pointers
//
	refillToolbarsInFrameData();
	repopulateCombos();
}

bool XAP_UnixFrame::close()
{
	gtk_widget_destroy(getTopLevelWindow());
	return true;
}

bool XAP_UnixFrame::raise()
{
	GtkWidget * tlw = getTopLevelWindow();
	UT_ASSERT(tlw);

	gdk_window_raise(tlw->window);

	return true;
}

bool XAP_UnixFrame::show()
{
	if(m_wTopLevelWindow)
	{
		gtk_widget_show(m_wTopLevelWindow);
	}
	return true;
}

bool XAP_UnixFrame::openURL(const char * szURL)
{
	static char *fmtstring = NULL;
  	char *execstring = NULL;

	if(fmtstring)			// lookup browser result when we have
	  {				// already calculated it once before
		if(strstr(fmtstring, "netscape"))
		{
		  execstring = g_strdup_printf(fmtstring, szURL, szURL);
		}
		else
		{
		  execstring = g_strdup_printf(fmtstring, szURL);
		}

		system(execstring);
		g_free (execstring);
		return false;					// why do we return false?
	}

	// TODO: we should move this into a preferences item,
	// TODO: but every other platform has a default browser
	// TODO: or text/html handler in a global registry

	// ORDER:
	// Use value of environment variable BROWSER, if valid; otherwise:
	// 1) konqueror
	// 2) mozilla
	// 3) netscape
	// 4) kdehelp
	// 5) lynx in an xterm
  	char *env_browser = getenv ("BROWSER");
  	if (env_browser)
  	{
		if(progExists(env_browser))
		{
			if (strstr (env_browser, "netscape"))
			{
				fmtstring = g_strdup_printf("%s -remote openURL\\('%%s'\\) || %s '%%s' &", env_browser, env_browser);
				if (fmtstring) execstring = g_strdup_printf(fmtstring, szURL, szURL);
			}
			else
			{
				fmtstring = g_strdup_printf("%s '%%s' &", env_browser);
				if (fmtstring) execstring = g_strdup_printf(fmtstring, szURL);
			}
		}
  	}
	if (fmtstring == 0)
	{
		if(progExists("konqueror"))
		{
			fmtstring = "konqueror '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("galeon"))
		{
		  	fmtstring = "galeon '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		// Anyone know how to find out where it might be, regardless?
		else if(progExists("mozilla"))
		{
		        fmtstring = "mozilla '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("netscape"))
		{
			// Try to connect to a running Netscape, if not, start new one
			fmtstring = "netscape -remote openURL\\('%s'\\) || netscape '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL, szURL);
		}
		else if(progExists("khelpcenter"))
		{
			fmtstring = "khelpcenter '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("gnome-help-browser"))
		{
			fmtstring = "gnome-help-browser '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("lynx"))
		{
			fmtstring = "xterm -e lynx '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("w3m"))
		{
			fmtstring = "xterm -e w3m '%s' &";
	 		execstring = g_strdup_printf(fmtstring, szURL);
	 	}
	}
	if (execstring)
	{
		system (execstring);
		g_free (execstring);
	}
	return false;
}

bool XAP_UnixFrame::updateTitle()
{
	if (!XAP_Frame::updateTitle() || (m_wTopLevelWindow== NULL) || (m_iFrameMode != XAP_NormalFrame))
	{
		// no relevant change, so skip it
		return false;
	}

	char buf[256];
	buf[0] = 0;

	const char * szAppName = m_pUnixApp->getApplicationTitleForTitleBar();

	int len = 256 - strlen(szAppName) - 4;

	const char * szTitle = getTitle(len);

	sprintf(buf, "%s - %s", szTitle, szAppName);

	gtk_window_set_title(GTK_WINDOW(m_wTopLevelWindow), buf);

	return true;
}

/*****************************************************************/

static void s_gtkMenuPositionFunc(GtkMenu * /* menu */, gint * x, gint * y, gboolean * push_in, gpointer user_data)
{
	struct UT_Point * p = (struct UT_Point *)user_data;

	*x = p->x;
	*y = p->y;
	*push_in = TRUE ;
}

bool XAP_UnixFrame::runModalContextMenu(AV_View * /* pView */, const char * szMenuName,
										   UT_sint32 x, UT_sint32 y)
{
	bool bResult = true;

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

		//
		// OK lets not immediately drop the menu if the user releases the mouse button.
		// From the gtk FAQ.
		//
		GdkEvent * event = gtk_get_current_event();
		GdkEventButton *bevent = (GdkEventButton *) event;

		GtkRequisition req ;
		gtk_widget_size_request (m_pUnixPopup->getMenuHandle(), &req);
		gdk_window_get_origin(w->window, &x,&y);
		x += bevent->x;
		y += bevent->y;

		UT_DEBUGMSG(("ContextMenu: %s at [%d,%d]\n",szMenuName,x,y));
		UT_Point pt;
		pt.x = x;
		pt.y = y;

		gtk_menu_popup(GTK_MENU(m_pUnixPopup->getMenuHandle()), NULL, NULL,
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

void XAP_UnixFrame::queue_resize()
{
	xxx_UT_DEBUGMSG(("XAP_UnixFrame::queue_resize\n"));
	gtk_widget_queue_resize(m_wTopLevelWindow);
}

EV_Menu* XAP_UnixFrame::getMainMenu()
{
	return m_pUnixMenu;
}
