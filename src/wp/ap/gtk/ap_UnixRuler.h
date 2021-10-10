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

#pragma once

#include "gr_Graphics.h"
#include "xap_UnixCustomWidget.h"

class AV_View;
class GR_UnixCairoGraphics;
class XAP_Frame;

/**
 * This is the Gtk3 implementation for the rulers.
 * There was a lot of cut&paste between AP_UnixLeftRuler and AP_UnixTopRuler.
 *
 */
class AP_UnixRuler
    : virtual public XAP_UnixCustomWidget
{
public:
    AP_UnixRuler(XAP_Frame* pFrame);

    virtual GtkWidget* getWidget() override
        { return m_wRuler; }

    void getWidgetPosition(gint& x, gint& y) const;

    void _ruler_style_context_changed();
protected:
    GtkWidget* _createWidget(gint w, gint h);
    void _setView(AV_View * pView, GR_UnixCairoGraphics* pG);
    void _aboutToDestroy(XAP_Frame* pFrame);

    virtual XAP_Frame* _getFrame() const = 0;
    virtual GR_Graphics* _getGraphics() const = 0;
    virtual void _setGraphics(GR_Graphics* pG) = 0;
    void _deleteGraphics()
        {
            delete _getGraphics();
            _setGraphics(nullptr);
        }
    virtual void _finishMotionEvent(UT_uint32 x, UT_uint32 y) = 0;

    class _fe
    {
    public:
        static void realize(AP_UnixRuler *self);
        static void unrealize(AP_UnixRuler *self);
        static gint button_press_event(GtkWidget * w, GdkEventButton * e);
        static gint button_release_event(GtkWidget * w, GdkEventButton * e);
        static gint configure_event(GtkWidget* w, GdkEventConfigure *e);
        static gint motion_notify_event(GtkWidget* w, GdkEventMotion* e);
        static gint key_press_event(GtkWidget* w, GdkEventKey* e);
        static gint delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/);
        static void destroy (GtkWidget * /*widget*/, gpointer /*data*/);
    };

    GtkWidget* m_wRuler;
    guint m_iBackgroundRedrawID;
};
