/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gtk-combo-box.h - a customizable combobox
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Miguel de Icaza <miguel@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _GTK_COMBO_BOX_H_
#define _GTK_COMBO_BOX_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_COMBO_BOX_TYPE          (gtk_combo_box_get_type())
#define GTK_COMBO_BOX(obj)	    G_TYPE_CHECK_INSTANCE_CAST (obj, gtk_combo_box_get_type (), GtkComboBox)
#define GTK_COMBO_BOX_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST (klass, gtk_combo_box_get_type (), GtkComboBoxClass)
#define GTK_IS_COMBO_BOX(obj)       G_TYPE_CHECK_INSTANCE_TYPE (obj, gtk_combo_box_get_type ())

typedef struct _GtkComboBox	   GtkComboBox;
typedef struct _GtkComboBoxPrivate GtkComboBoxPrivate;
typedef struct _GtkComboBoxClass   GtkComboBoxClass;

struct _GtkComboBox {
	GtkHBox hbox;
	GtkComboBoxPrivate *priv;
};

struct _GtkComboBoxClass {
	GtkHBoxClass parent_class;

	GtkWidget *(*pop_down_widget) (GtkComboBox *cbox);

	/*
	 * invoked when the popup has been hidden, if the signal
	 * returns TRUE, it means it should be killed from the
	 */
	gboolean  *(*pop_down_done)   (GtkComboBox *cbox, GtkWidget *);

	/*
	 * Notification signals.
	 */
	void      (*pre_pop_down)     (GtkComboBox *cbox);
	void      (*post_pop_hide)    (GtkComboBox *cbox);
};

/* public */
GtkType    gtk_combo_box_get_type    (void);
void       gtk_combo_box_construct   (GtkComboBox *combo_box,
				      GtkWidget   *display_widget,
				      GtkWidget   *optional_pop_down_widget);
GtkWidget *gtk_combo_box_new         (GtkWidget *display_widget,
				      GtkWidget *optional_pop_down_widget);
void       gtk_combo_box_set_title   (GtkComboBox *combo,
				      const gchar *title);
void       gtk_combo_box_set_tearable        (GtkComboBox *combo,
					      gboolean tearable);
void       gtk_combo_box_set_arrow_sensitive (GtkComboBox *combo,
					      gboolean sensitive);
void       gtk_combo_box_set_arrow_relief    (GtkComboBox *cc,
					      GtkReliefStyle relief);

/* protected */
void       gtk_combo_box_get_pos     (GtkComboBox *combo_box, int *x, int *y);

void       gtk_combo_box_popup_hide  (GtkComboBox *combo_box);

void       gtk_combo_box_popup_display (GtkComboBox *combo_box);

void       gtk_combo_box_set_display (GtkComboBox *combo_box,
				      GtkWidget *display_widget);

G_END_DECLS

#endif /* _GTK_COMBO_BOX_H_ */
