/*
 * Copyright (C) 2005 Robert Staudinger <robsta@stereolyzer.net>
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


#ifndef __ABI_WIDGET_PRIV_H__
#define __ABI_WIDGET_PRIV_H__


#include "ap_UnixFrameImpl.h"


class AP_WidgetApp;
class AP_WidgetFrame;
class AP_WidgetFrameImpl;
class AP_UnixTopRuler;
class AP_UnixLeftRuler;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <gtk/gtk.h>
#include "abi-widget.h"


enum AbiViewMode {
	ABI_VIEW_NORMAL, 
	ABI_VIEW_PRINT
};


typedef struct AbiWidgets {
	GObject *h_adjust;
	GObject *v_adjust;
	GtkWidget *h_scroll;
	GtkWidget *v_scroll;
	GtkWidget *inner_table;
	GtkWidget *table;
	GtkWidget *d_area;
};

typedef struct AbiWidgetPrivate {

	gboolean show_rulers;
	gint zoom_idle_cb_handle;
	gboolean do_zoom_update;
	gint new_width;
	gint new_height;

	AP_WidgetApp *app;
	AP_WidgetFrame *frame;
	AP_WidgetFrameImpl *frame_impl;

	gboolean is_disposed;
};

typedef struct AbiWidget {
	GtkFrame frame;

	AbiWidgets *widgets;

	AP_UnixTopRuler *top_ruler;
	AP_UnixLeftRuler *left_ruler;

	PD_Document *doc;
	FL_DocLayout *layout;
	GR_Graphics *graphics;
	FV_View *view;

	AbiViewMode		view_mode;
	XAP_Frame::tZoomType zoom_type;
	gint zoom_percentage;

	AbiWidgetPrivate *priv;
};

typedef struct AbiWidgetClass {
	GtkFrameClass parent_class;
};


static AbiWidgets* abi_widget_priv_create_widgets  (AbiWidget *self);
gboolean abi_widget_priv_show_doc (AbiWidget *self);
void abi_widget_priv_quick_zoom (AbiWidget *self);
void abi_widget_priv_replace_view (AbiWidget *self, FL_DocLayout *pDocLayout,
			    AV_ScrollObj * pScrollObj,
			    ap_ViewListener *pViewListener,
			    ap_Scrollbar_ViewListener *pScrollbarViewListener,
			    AV_ListenerId lid, AV_ListenerId lidScrollbarViewListener);
void abi_widget_priv_set_x_scroll_range (AbiWidget *self);
void abi_widget_priv_set_y_scroll_range (AbiWidget *self);
void abi_widget_priv_set_scroll_range (AbiWidget *self, apufi_ScrollType scrollType, int iValue, gfloat fUpperLimit, gfloat fSize);
static void abi_widget_priv_scroll_func_x (gpointer self, UT_sint32 xoff, UT_sint32 /*xrange*/);
static void abi_widget_priv_scroll_func_y (gpointer self, UT_sint32 yoff, UT_sint32 /*yrange*/);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ABI_WIDGET_PRIV_H__ */
