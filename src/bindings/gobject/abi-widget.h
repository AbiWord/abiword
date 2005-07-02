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


#ifndef __ABI_WIDGET_H__
#define __ABI_WIDGET_H__


#include <gtk/gtk.h>
#ifdef ABIWORD_INTERNAL
#include "private/abi-widget-priv.h"
#else
#include <abiword/private/abi-widget-priv.h>
#endif /* ABIWORD_INTERNAL */
#include "abi-doc.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define ABI_TYPE_WIDGET                  (abi_widget_get_type ())
#define ABI_WIDGET(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), ABI_TYPE_WIDGET, AbiWidget))
#define ABI_WIDGET_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), ABI_TYPE_WIDGET, AbiWidgetClass))
#define ABI_IS_WIDGET(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ABI_TYPE_WIDGET))
#define ABI_IS_WIDGET_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), ABI_TYPE_WIDGET))
#define ABI_WIDGET_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), ABI_TYPE_WIDGET, AbiWidgetClass))

typedef struct _AbiWidget AbiWidget;
typedef struct _AbiWidgetClass AbiWidgetClass;

struct _AbiWidget {
	GtkFrame frame;

	AbiWidgets *widgets;

	AP_UnixTopRuler *top_ruler;
	AP_UnixLeftRuler *left_ruler;

	PD_Document *doc;
	FL_DocLayout *layout;
	GR_Graphics *graphics;
	FV_View *view;

	GtkIMContext *im_cx;

	AbiViewMode		view_mode;
	XAP_Frame::tZoomType zoom_type;
	gint zoom_percentage;

	AbiWidgetPrivate *priv;
};

struct _AbiWidgetClass {
	GtkFrameClass parent_class;
};


GType      abi_widget_get_type         (void) G_GNUC_CONST;
GtkWidget* abi_widget_new              (void);
gboolean   abi_widget_load_file		 (AbiWidget *self, const gchar *filename);
/*
AbiDoc*    abi_widget_get_doc 		 (AbiWidget *view);
void	   abi_widget_set_doc			 (AbiWidget *view, AbiDoc *doc);
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __ABI_WIDGET_H__ */
