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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <gtk/gtk.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "ap_UnixLeftRuler.h"
#include "gr_UnixCairoGraphics.h"
#include "xap_UnixDialogHelper.h"
#include "fv_View.h"
#include "ut_sleep.h"

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

static void
ruler_style_context_changed (GtkWidget 			* /*w*/, 
					 AP_UnixLeftRuler 	*ruler)
{
	ruler->_ruler_style_context_changed();
}

AP_UnixLeftRuler::AP_UnixLeftRuler(XAP_Frame * pFrame)
	: AP_LeftRuler(pFrame),
	  XAP_UnixCustomWidget()
{
	m_rootWindow = NULL;
	m_wLeftRuler = NULL;
	m_pG = NULL;
    // change ruler color on theme change
	GtkWidget * toplevel = static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl())->getTopLevelWindow();
	m_iBackgroundRedrawID = g_signal_connect_after (G_OBJECT(toplevel),
													"style-updated",
							  G_CALLBACK(ruler_style_context_changed),
							  static_cast<gpointer>(this));
}

AP_UnixLeftRuler::~AP_UnixLeftRuler(void)
{
	GtkWidget * toplevel = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl())->getTopLevelWindow();
	if (toplevel && g_signal_handler_is_connected(G_OBJECT(toplevel), m_iBackgroundRedrawID)) {
		g_signal_handler_disconnect(G_OBJECT(toplevel),m_iBackgroundRedrawID);
	}
	DELETEP(m_pG);
}

void AP_UnixLeftRuler::_ruler_style_context_changed (void)
{
	_refreshView();
}

GtkWidget * AP_UnixLeftRuler::createWidget(void)
{
	UT_ASSERT(!m_pG && !m_wLeftRuler);
	
	m_wLeftRuler = gtk_drawing_area_new();

	g_object_set_data(G_OBJECT(m_wLeftRuler), "user_data", this);
	gtk_widget_show(m_wLeftRuler);
	gtk_widget_set_size_request(m_wLeftRuler, s_iFixedWidth, -1);

	gtk_widget_set_events(GTK_WIDGET(m_wLeftRuler), (GDK_EXPOSURE_MASK |
													 GDK_BUTTON_PRESS_MASK |
													 GDK_POINTER_MOTION_MASK |
													 GDK_BUTTON_RELEASE_MASK |
													 GDK_KEY_PRESS_MASK |
													 GDK_KEY_RELEASE_MASK));

	g_signal_connect_swapped(G_OBJECT(m_wLeftRuler), "realize",
					   G_CALLBACK(_fe::realize), this);

	g_signal_connect_swapped(G_OBJECT(m_wLeftRuler), "unrealize",
					   G_CALLBACK(_fe::unrealize), this);

	g_signal_connect_swapped(G_OBJECT(m_wLeftRuler), "draw",
					   G_CALLBACK(XAP_UnixCustomWidget::_fe::draw), static_cast<XAP_UnixCustomWidget *>(this));

	g_signal_connect(G_OBJECT(m_wLeftRuler), "button_press_event",
					   G_CALLBACK(_fe::button_press_event), NULL);

	g_signal_connect(G_OBJECT(m_wLeftRuler), "button_release_event",
					   G_CALLBACK(_fe::button_release_event), NULL);

	g_signal_connect(G_OBJECT(m_wLeftRuler), "motion_notify_event",
					   G_CALLBACK(_fe::motion_notify_event), NULL);

	g_signal_connect(G_OBJECT(m_wLeftRuler), "configure_event",
					   G_CALLBACK(_fe::configure_event), NULL);

	return m_wLeftRuler;
}

void AP_UnixLeftRuler::setView(AV_View * pView)
{
	AP_LeftRuler::setView(pView);

	UT_ASSERT(gtk_widget_get_realized(m_wLeftRuler));

	m_pG->setZoomPercentage(pView->getGraphics()->getZoomPercentage());

	GtkWidget * w = gtk_entry_new();
	((GR_UnixCairoGraphics*)m_pG)->init3dColors(w);
	gtk_widget_destroy(w);
}

void AP_UnixLeftRuler::getWidgetPosition(gint * x, gint * y)
{
	UT_ASSERT(x && y);
	
	gdk_window_get_position(gtk_widget_get_window(m_wLeftRuler), x, y);
}

GdkWindow * AP_UnixLeftRuler::getRootWindow(void)
{
	// TODO move this function somewhere more logical, like
	// TODO the XAP_Frame level, since that's where the
	// TODO root window is common to all descendants.
	if (m_rootWindow)
		return m_rootWindow;

	m_rootWindow  = ::getRootWindow(m_wLeftRuler);
	return m_rootWindow;
}

/*****************************************************************/

void AP_UnixLeftRuler::_fe::realize(AP_UnixLeftRuler *self)
{
	UT_ASSERT(!self->m_pG);

	GR_UnixCairoAllocInfo ai(self->m_wLeftRuler);
	self->m_pG = XAP_App::getApp()->newGraphics(ai);
	UT_ASSERT(self->m_pG);
}

void AP_UnixLeftRuler::_fe::unrealize(AP_UnixLeftRuler *self)
{
	UT_ASSERT(self->m_pG);
	DELETEP(self->m_pG);
}

gint AP_UnixLeftRuler::_fe::button_press_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = static_cast<AP_UnixLeftRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));

	FV_View * pView = static_cast<FV_View *>(pUnixLeftRuler->m_pFrame->getCurrentView());
	if (!pView || pView->getPoint() == 0 || !pUnixLeftRuler->m_pG)
		return 1;

	// grab the mouse for the duration of the drag.
	gtk_grab_add(w);
	
	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;
	
	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	if (e->state & GDK_BUTTON1_MASK)
		emb = EV_EMB_BUTTON1;
	else if (e->state & GDK_BUTTON2_MASK)
		emb = EV_EMB_BUTTON2;
	else if (e->state & GDK_BUTTON3_MASK)
		emb = EV_EMB_BUTTON3;

	pUnixLeftRuler->mousePress(ems, emb, 
				   pUnixLeftRuler->m_pG->tlu(static_cast<UT_uint32>(e->x)), 
				   pUnixLeftRuler->m_pG->tlu(static_cast<UT_uint32>(e->y)));

	return 1;
}

gint AP_UnixLeftRuler::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = static_cast<AP_UnixLeftRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));
	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;

	FV_View * pView = static_cast<FV_View *>(pUnixLeftRuler->m_pFrame->getCurrentView());
	if (!pView || pView->getPoint() == 0 || !pUnixLeftRuler->m_pG)
		return 1;

	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	if (e->state & GDK_BUTTON1_MASK)
		emb = EV_EMB_BUTTON1;
	else if (e->state & GDK_BUTTON2_MASK)
		emb = EV_EMB_BUTTON2;
	else if (e->state & GDK_BUTTON3_MASK)
		emb = EV_EMB_BUTTON3;

	pUnixLeftRuler->mouseRelease(ems, emb, 
				   pUnixLeftRuler->m_pG->tlu(static_cast<UT_uint32>(e->x)),
				   pUnixLeftRuler->m_pG->tlu(static_cast<UT_uint32>(e->y)));

	// release the mouse after we are done.
	gtk_grab_remove(w);
	
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::configure_event(GtkWidget* w, GdkEventConfigure * e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = static_cast<AP_UnixLeftRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));

	// nb: we'd convert here, but we can't: have no graphics class!
	pUnixLeftRuler->setHeight(e->height);
	pUnixLeftRuler->setWidth(e->width);
	
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::motion_notify_event(GtkWidget* w , GdkEventMotion* e)
{
	// a static function
	AP_UnixLeftRuler * pUnixLeftRuler = static_cast<AP_UnixLeftRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));

	FV_View * pView = static_cast<FV_View *>(pUnixLeftRuler->m_pFrame->getCurrentView());
	if (!pView || pView->getPoint() == 0 || !pUnixLeftRuler->m_pG)
		return 1;

	EV_EditModifierState ems = 0;
	
	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	pUnixLeftRuler->mouseMotion(ems, 
				    pUnixLeftRuler->m_pG->tlu(static_cast<UT_uint32>(e->x)),
				    pUnixLeftRuler->m_pG->tlu(static_cast<UT_uint32>(e->y)));
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::key_press_event(GtkWidget* /*w*/, GdkEventKey* /* e */)
{
	// a static function
	return 1;
}
	
gint AP_UnixLeftRuler::_fe::delete_event(GtkWidget * /* w */, GdkEvent * /*event*/, gpointer /*data*/)
{
	// a static function
	return 1;
}

void AP_UnixLeftRuler::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
	// a static function
}
