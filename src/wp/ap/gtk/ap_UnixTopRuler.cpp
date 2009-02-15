/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
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

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"

#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"

#include "gr_UnixCairoGraphics.h"
#include "ap_UnixTopRuler.h"
#include "fv_View.h"

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

static void
ruler_style_changed (GtkWidget 			* /*w*/, 
					 GtkStyle 			* /*previous_style*/,
					 AP_UnixTopRuler 	*ruler)
{
	ruler->_ruler_style_changed();
}

AP_UnixTopRuler::AP_UnixTopRuler(XAP_Frame * pFrame)
	: AP_TopRuler(pFrame)
{
	m_rootWindow = NULL;
	m_wTopRuler = NULL;
	m_pG = NULL;

	// change ruler color on theme change
	GtkWidget * toplevel = static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl())->getTopLevelWindow();
	m_iStyleID = g_signal_connect_after (G_OBJECT(toplevel),
							  "style-set",
							  G_CALLBACK(ruler_style_changed),
							  static_cast<gpointer>(this));
}

AP_UnixTopRuler::~AP_UnixTopRuler(void)
{
	GtkWidget * toplevel = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getTopLevelWindow();
	if (toplevel && g_signal_handler_is_connected(G_OBJECT(toplevel), m_iStyleID)) {	
		g_signal_handler_disconnect(G_OBJECT(toplevel),	m_iStyleID);
	}
	DELETEP(m_pG);
}

void AP_UnixTopRuler::_ruler_style_changed (void)
{
	_refreshView();
}

GtkWidget * AP_UnixTopRuler::createWidget(void)
{
	UT_ASSERT(!m_pG && !m_wTopRuler);
	
	m_wTopRuler = createDrawingArea ();

	g_object_set_data(G_OBJECT(m_wTopRuler),"user_data", this);
	gtk_widget_show(m_wTopRuler);
	gtk_widget_set_size_request(m_wTopRuler, -1, s_iFixedHeight);

	gtk_widget_set_events(GTK_WIDGET(m_wTopRuler), (GDK_EXPOSURE_MASK |
													GDK_BUTTON_PRESS_MASK |
													GDK_POINTER_MOTION_MASK |
													GDK_BUTTON_RELEASE_MASK |
													GDK_KEY_PRESS_MASK |
													GDK_KEY_RELEASE_MASK));

	g_signal_connect(G_OBJECT(m_wTopRuler), "expose_event",
					   G_CALLBACK(_fe::expose), NULL);
  
	g_signal_connect(G_OBJECT(m_wTopRuler), "button_press_event",
					   G_CALLBACK(_fe::button_press_event), NULL);

	g_signal_connect(G_OBJECT(m_wTopRuler), "button_release_event",
					   G_CALLBACK(_fe::button_release_event), NULL);

	g_signal_connect(G_OBJECT(m_wTopRuler), "motion_notify_event",
					   G_CALLBACK(_fe::motion_notify_event), NULL);
  
	g_signal_connect(G_OBJECT(m_wTopRuler), "configure_event",
					   G_CALLBACK(_fe::configure_event), NULL);

	return m_wTopRuler;
}

void AP_UnixTopRuler::setView(AV_View * pView)
{
	AP_TopRuler::setView(pView);

	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wTopRuler->window)
	// is not created until the frame's top-level window is
	// shown.

	DELETEP(m_pG);

	GR_UnixCairoAllocInfo ai(m_wTopRuler->window);
	m_pG = XAP_App::getApp()->newGraphics(ai);

	UT_ASSERT(m_pG);
	m_pG->setZoomPercentage(pView->getGraphics()->getZoomPercentage());
	GtkWidget * ruler = gtk_hruler_new ();
	((GR_UnixCairoGraphics*)m_pG)->init3dColors(get_ensured_style(ruler));
	//abiDestroyWidget (ruler);
}

void AP_UnixTopRuler::getWidgetPosition(gint * x, gint * y)
{
	UT_ASSERT(x && y);
	
	gdk_window_get_position(m_wTopRuler->window, x, y);
}

GdkWindow * AP_UnixTopRuler::getRootWindow(void)
{
	// TODO move this function somewhere more logical, like
	// TODO the XAP_Frame level, since that's where the
	// TODO root window is common to all descendants.
	if (m_rootWindow)
		return m_rootWindow;

	m_rootWindow  = ::getRootWindow(m_wTopRuler);
	return m_rootWindow;
}

		
/*****************************************************************/

gint AP_UnixTopRuler::_fe::button_press_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_UnixTopRuler * pUnixTopRuler = static_cast<AP_UnixTopRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));

	// grab the mouse for the duration of the drag.
	gtk_grab_add(w);
	
	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;

	if (!pUnixTopRuler->getGraphics())
		return 1;
	
	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	if (1 == e->button )
		emb = EV_EMB_BUTTON1;
	else if (2 == e->button )
		emb = EV_EMB_BUTTON2;
	else if (3 == e->button)
		emb = EV_EMB_BUTTON3;

	UT_DEBUGMSG(("SEVIOR: e->button = %x \n",e->button));
	pUnixTopRuler->mousePress(ems, emb, 
				  pUnixTopRuler->getGraphics()->tlu(static_cast<UT_uint32>(e->x)), 
				  pUnixTopRuler->getGraphics()->tlu(static_cast<UT_uint32>(e->y)));
	return 1;
}

gint AP_UnixTopRuler::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_UnixTopRuler * pUnixTopRuler = static_cast<AP_UnixTopRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));

	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;
	
	if (!pUnixTopRuler->getGraphics())
		return 1;

	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	if (1 == e->button )
		emb = EV_EMB_BUTTON1;
	else if (2 == e->button )
		emb = EV_EMB_BUTTON2;
	else if (3 == e->button)
		emb = EV_EMB_BUTTON3;

	pUnixTopRuler->mouseRelease(ems, emb, 
				  pUnixTopRuler->getGraphics()->tlu(static_cast<UT_uint32>(e->x)), 
				  pUnixTopRuler->getGraphics()->tlu(static_cast<UT_uint32>(e->y)));

	// release the mouse after we are done.
	gtk_grab_remove(w);
	
	return 1;
}
	
gint AP_UnixTopRuler::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// a static function
	AP_UnixTopRuler * pUnixTopRuler = static_cast<AP_UnixTopRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));

// nb: we'd convert here, but we can't: have no graphics class!
	pUnixTopRuler->setHeight(static_cast<UT_uint32>(e->height));
	pUnixTopRuler->setWidth(static_cast<UT_uint32>(e->width));
	
	return 1;
}

	
gint AP_UnixTopRuler::_fe::motion_notify_event(GtkWidget* w, GdkEventMotion* e)
{
	// a static function
	AP_UnixTopRuler * pUnixTopRuler = static_cast<AP_UnixTopRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));

	XAP_App * pApp = XAP_App::getApp();
	XAP_Frame * pFrame = pApp->getLastFocussedFrame();
	if(pFrame == NULL)
		return 1;

	AV_View * pView = pFrame->getCurrentView();
	if(pView == NULL || pView->getPoint() == 0 || pUnixTopRuler->getGraphics() == NULL)
		return 1;

	EV_EditModifierState ems = 0;
	
	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	// Map the mouse into coordinates relative to our window.

	pUnixTopRuler->mouseMotion(ems, 
				  pUnixTopRuler->getGraphics()->tlu(static_cast<UT_uint32>(e->x)), 
				  pUnixTopRuler->getGraphics()->tlu(static_cast<UT_uint32>(e->y)));
	pUnixTopRuler->isMouseOverTab(pUnixTopRuler->getGraphics()->tlu(static_cast<UT_uint32>(e->x)), 
				  pUnixTopRuler->getGraphics()->tlu(static_cast<UT_uint32>(e->y)));

	return 1;

}
	
gint AP_UnixTopRuler::_fe::key_press_event(GtkWidget* /*w*/, GdkEventKey* /* e */)
{
	return 1;
}
	
gint AP_UnixTopRuler::_fe::delete_event(GtkWidget * /* w */, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	return 1;
}
	
gint AP_UnixTopRuler::_fe::expose(GtkWidget * w, GdkEventExpose* pExposeEvent)
{
	// a static function
	AP_UnixTopRuler * pUnixTopRuler = static_cast<AP_UnixTopRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));
	if (!pUnixTopRuler)
		return 0;

	GR_Graphics * pG = pUnixTopRuler->getGraphics();
	if(pG != NULL)
	{
		UT_Rect rClip;
		rClip.left = pG->tlu(pExposeEvent->area.x);
		rClip.top = pG->tlu(pExposeEvent->area.y);
		rClip.width = pG->tlu(pExposeEvent->area.width);
		rClip.height = pG->tlu(pExposeEvent->area.height);
		pUnixTopRuler->draw (&rClip);
	}

	return 0;
}

void AP_UnixTopRuler::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
	// a static function
}
