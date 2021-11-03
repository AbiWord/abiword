/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2019 Hubert Figuière
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

#include "ap_UnixRuler.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "ev_EditBits.h"
#include "gr_UnixCairoGraphics.h"
#include "fv_View.h"
#include "ap_Ruler.h"

static void
ruler_style_context_changed (GtkWidget* /*w*/,
                             AP_UnixRuler* ruler)
{
    ruler->_ruler_style_context_changed();
}

AP_UnixRuler::AP_UnixRuler(XAP_Frame* pFrame)
    : m_wRuler(nullptr)
    , m_iBackgroundRedrawID(0)
{
    // change ruler color on theme change
    GtkWidget* toplevel = static_cast<XAP_UnixFrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();
    m_iBackgroundRedrawID = g_signal_connect_after(
        G_OBJECT(toplevel), "style-updated", G_CALLBACK(ruler_style_context_changed),
        static_cast<gpointer>(this));
}

void AP_UnixRuler::_aboutToDestroy(XAP_Frame* pFrame)
{
    GtkWidget* toplevel = static_cast<XAP_UnixFrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();
    if (toplevel && g_signal_handler_is_connected(G_OBJECT(toplevel), m_iBackgroundRedrawID)) {
        g_signal_handler_disconnect(G_OBJECT(toplevel), m_iBackgroundRedrawID);
    }
}

void AP_UnixRuler::_ruler_style_context_changed()
{
    AP_Ruler* ruler = dynamic_cast<AP_Ruler*>(this);
    UT_ASSERT(ruler);
    if (ruler) {
        ruler->_refreshView();
    }
}

GtkWidget* AP_UnixRuler::_createWidget(gint w, gint h)
{
    UT_ASSERT(!m_wRuler);

    m_wRuler = gtk_drawing_area_new();

    g_object_set_data(G_OBJECT(m_wRuler), "user_data", this);
    gtk_widget_show(m_wRuler);
    gtk_widget_set_size_request(m_wRuler, w, h);

    gtk_widget_set_events(GTK_WIDGET(m_wRuler), (GDK_EXPOSURE_MASK |
                                                 GDK_BUTTON_PRESS_MASK |
                                                 GDK_POINTER_MOTION_MASK |
                                                 GDK_BUTTON_RELEASE_MASK |
                                                 GDK_KEY_PRESS_MASK |
                                                 GDK_KEY_RELEASE_MASK));

    g_signal_connect_swapped(G_OBJECT(m_wRuler), "realize",
                             G_CALLBACK(_fe::realize), this);

    g_signal_connect_swapped(G_OBJECT(m_wRuler), "unrealize",
                             G_CALLBACK(_fe::unrealize), this);

    g_signal_connect_swapped(G_OBJECT(m_wRuler), "draw",
                             G_CALLBACK(XAP_UnixCustomWidget::_fe::draw),
                             static_cast<XAP_UnixCustomWidget *>(this));

    g_signal_connect(G_OBJECT(m_wRuler), "button_press_event",
                     G_CALLBACK(_fe::button_press_event), nullptr);

    g_signal_connect(G_OBJECT(m_wRuler), "button_release_event",
                     G_CALLBACK(_fe::button_release_event), nullptr);

    g_signal_connect(G_OBJECT(m_wRuler), "motion_notify_event",
                     G_CALLBACK(_fe::motion_notify_event), nullptr);

    g_signal_connect(G_OBJECT(m_wRuler), "configure_event",
                     G_CALLBACK(_fe::configure_event), nullptr);

    return m_wRuler;
}

void AP_UnixRuler::_setView(AV_View* pView, GR_UnixCairoGraphics* pG)
{
    UT_ASSERT(gtk_widget_get_realized(m_wRuler));

    pG->setZoomPercentage(pView->getGraphics()->getZoomPercentage());

    GtkWidget* w = gtk_entry_new();
    g_object_ref_sink(w);
    pG->init3dColors(w);
    g_object_unref(w);
}

void AP_UnixRuler::getWidgetPosition(gint& x, gint& y) const
{
    gdk_window_get_position(gtk_widget_get_window(m_wRuler), &x, &y);
}

void AP_UnixRuler::_fe::realize(AP_UnixRuler* self)
{
    UT_ASSERT(!self->_getGraphics());

    GR_UnixCairoAllocInfo ai(self->m_wRuler);
    self->_setGraphics(XAP_App::getApp()->newGraphics(ai));
    UT_ASSERT(self->_getGraphics());
}

void AP_UnixRuler::_fe::unrealize(AP_UnixRuler* self)
{
    UT_ASSERT(self->_getGraphics());
    self->_deleteGraphics();
}

gint AP_UnixRuler::_fe::button_press_event(GtkWidget* w, GdkEventButton* e)
{
    // a static function
    AP_UnixRuler* pRuler = static_cast<AP_UnixRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));
    AP_Ruler* ruler = dynamic_cast<AP_Ruler*>(pRuler);
    UT_ASSERT(ruler);

    FV_View* pView = static_cast<FV_View *>(ruler->getFrame()->getCurrentView());
    if (!pView || pView->getPoint() == 0 || !ruler->getGraphics()) {
        return 1;
    }

    // grab the mouse for the duration of the drag.
    gtk_grab_add(w);

    EV_EditModifierState ems = 0;
    EV_EditMouseButton emb = 0;

    GdkModifierType ev_state = (GdkModifierType)0;
    gdk_event_get_state((GdkEvent*)e, &ev_state);

    if (ev_state & GDK_SHIFT_MASK) {
        ems |= EV_EMS_SHIFT;
    }
    if (ev_state & GDK_CONTROL_MASK) {
        ems |= EV_EMS_CONTROL;
    }
    if (ev_state & GDK_MOD1_MASK) {
        ems |= EV_EMS_ALT;
    }

    guint ev_button = 0;
    gdk_event_get_button((GdkEvent*)e, &ev_button);

    if (1 == ev_button) {
        emb = EV_EMB_BUTTON1;
    } else if (2 == ev_button) {
        emb = EV_EMB_BUTTON2;
    } else if (3 == ev_button) {
        emb = EV_EMB_BUTTON3;
    }

    UT_DEBUGMSG(("SEVIOR: ev_button = %x \n",ev_button));
    gdouble ev_x, ev_y;
    ev_x = ev_y = 0.0f;
    gdk_event_get_coords((GdkEvent*)e, &ev_x, &ev_y);

    auto pG = ruler->getGraphics();
    ruler->mousePress(ems, emb,
                       pG->tlu(static_cast<UT_uint32>(ev_x)),
                       pG->tlu(static_cast<UT_uint32>(ev_y)));
    return 1;
}

gint AP_UnixRuler::_fe::button_release_event(GtkWidget* w, GdkEventButton* e)
{
    AP_UnixRuler* pRuler = static_cast<AP_UnixRuler*>(g_object_get_data(G_OBJECT(w), "user_data"));
    UT_ASSERT(pRuler);
    AP_Ruler* ruler = dynamic_cast<AP_Ruler*>(pRuler);
    UT_ASSERT(ruler);

    EV_EditModifierState ems = 0;
    EV_EditMouseButton emb = 0;

    FV_View* pView = static_cast<FV_View*>(ruler->getFrame()->getCurrentView());
    if (!pView || pView->getPoint() == 0 || !ruler->getGraphics()) {
        return 1;
    }

    GdkModifierType ev_state = (GdkModifierType)0;
    gdk_event_get_state((GdkEvent*)e, &ev_state);

    if (ev_state & GDK_SHIFT_MASK) {
        ems |= EV_EMS_SHIFT;
    }
    if (ev_state & GDK_CONTROL_MASK) {
        ems |= EV_EMS_CONTROL;
    }
    if (ev_state & GDK_MOD1_MASK) {
        ems |= EV_EMS_ALT;
    }

    guint ev_button = 0;
    gdk_event_get_button((GdkEvent*)e, &ev_button);

    if (1 == ev_button) {
        emb = EV_EMB_BUTTON1;
    } else if (2 == ev_button) {
        emb = EV_EMB_BUTTON2;
    } else if (3 == ev_button) {
        emb = EV_EMB_BUTTON3;
    }

    gdouble ev_x, ev_y;
    ev_x = ev_y = 0.0f;
    gdk_event_get_coords((GdkEvent*)e, &ev_x, &ev_y);
    auto pG = ruler->getGraphics();
    ruler->mouseRelease(ems, emb,
                         pG->tlu(static_cast<UT_uint32>(ev_x)),
                         pG->tlu(static_cast<UT_uint32>(ev_y)));

    // release the mouse after we are done.
    gtk_grab_remove(w);

    return 1;
}

gint AP_UnixRuler::_fe::configure_event(GtkWidget* w, GdkEventConfigure * e)
{
    AP_UnixRuler* pRuler = static_cast<AP_UnixRuler*>(g_object_get_data(G_OBJECT(w), "user_data"));
    AP_Ruler* ruler = dynamic_cast<AP_Ruler*>(pRuler);
    UT_ASSERT(ruler);

    // nb: we'd convert here, but we can't: have no graphics class!
    ruler->setHeight(e->height);
    ruler->setWidth(e->width);

    return 1;
}

gint AP_UnixRuler::_fe::motion_notify_event(GtkWidget* w, GdkEventMotion* e)
{
    AP_UnixRuler* pRuler = static_cast<AP_UnixRuler *>(g_object_get_data(G_OBJECT(w), "user_data"));
    AP_Ruler* ruler = dynamic_cast<AP_Ruler*>(pRuler);
    UT_ASSERT(ruler);

    XAP_App* pApp = XAP_App::getApp();
    XAP_Frame* pFrame = pApp->getLastFocussedFrame();
    if (pFrame == nullptr) {
        return 1;
    }

    AV_View * pView = pFrame->getCurrentView();
    if(pView == nullptr || pView->getPoint() == 0 || !ruler->getGraphics()) {
        return 1;
    }

    EV_EditModifierState ems = 0;

    GdkModifierType ev_state = (GdkModifierType)0;
    gdk_event_get_state((GdkEvent*)e, &ev_state);

    if (ev_state & GDK_SHIFT_MASK) {
        ems |= EV_EMS_SHIFT;
    }
    if (ev_state & GDK_CONTROL_MASK) {
        ems |= EV_EMS_CONTROL;
    }
    if (ev_state & GDK_MOD1_MASK) {
        ems |= EV_EMS_ALT;
    }

    // Map the mouse into coordinates relative to our window.
    gdouble ev_x, ev_y;
    ev_x = ev_y = 0.0f;
    gdk_event_get_coords((GdkEvent*)e, &ev_x, &ev_y);

    UT_uint32 x = ruler->getGraphics()->tlu(static_cast<UT_uint32>(ev_x));
    UT_uint32 y = ruler->getGraphics()->tlu(static_cast<UT_uint32>(ev_y));
    ruler->mouseMotion(ems, x, y);
    pRuler->_finishMotionEvent(x, y);

    return 1;
}

gint AP_UnixRuler::_fe::key_press_event(GtkWidget* /*w*/, GdkEventKey* /* e */)
{
    return 1;
}

gint AP_UnixRuler::_fe::delete_event(GtkWidget * /* w */, GdkEvent * /*event*/, gpointer /*data*/)
{
    return 1;
}

void AP_UnixRuler::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
}
