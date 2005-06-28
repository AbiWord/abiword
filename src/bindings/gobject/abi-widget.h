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
#include "abi-widget-priv.h"
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
