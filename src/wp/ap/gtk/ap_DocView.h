/* The AbiWord Document Widget
 *
 * Copyright (C) 2007 Michael Gorse <mgorse@alum.wpi.edu>
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
#ifndef AP_DOCVIEW_H
#define AP_DOCVIEW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define AP_TYPE_DOCVIEW      (ap_DocView_get_type ())
#define AP_DOCVIEW(obj)       (G_TYPE_CHECK_INSTANCE_CAST((obj), AT_TYPE_DOCVIEW, ApDocView))
#define IS_AP_DOCVIEW(obj)     (G_TYPE_CHECK_INSTANCE_TYPE((obj), AT_TYPE_DOCVIEW))
#define IS_AP_DOCVIEW_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), AT_TYPE_DOCVIEW))
#define AP_DOCVIEW_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST ((k), AT_TYPE_DOCVIEW, AbiWidgetClass))

  /* forward declarations */
  typedef struct _ApDocView      ApDocView;
  typedef GtkDrawingAreaClass ApDocViewClass;
  typedef struct _AbiPrivData    AbiPrivData;

  struct _ApDocView
  {
    GtkDrawingArea base;
    GtkWidget * child;
    /* private instance data */
    AbiPrivData * priv;
  };

  /* the public API */
  GtkWidget * ap_DocView_new (void);
  GType     ap_DocView_get_type	(void);

G_END_DECLS

#endif /* AP_DOCVIEW_H */
