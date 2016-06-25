/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * xap_TableWidget.h
 * Copyright 2002, Joaquin Cuenca Abela
 *
 * Authors:
 *   Joaquin Cuenca Abela (e98cuenc@yahoo.com)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef ABI_TABLE_H
#define ABI_TABLE_H

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ABITABLE_TYPE_WIDGET     (abi_table_get_type ())
#define ABITABLE_WIDGET(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), ABITABLE_TYPE_WIDGET, AbiTable))
#define IS_ABITABLE_WIDGET(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), ABITABLE_TYPE_WIDGET))
#define IS_ABITABLE_WIDGET_CLASS(obj)  (G_TYPE_CHECK_CLASS_CAST((obj), ABITABLE_TYPE_WIDGET))
#define ABITABLE_WIDGET_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), ABITABLE_TYPE_WIDGET,AbiTableClass))

typedef struct _AbiTable
{
	GtkButton button;

	/*<private>*/
	GtkWidget* button_box;
	GtkWidget* icon;

	GtkWindow* window;
	GtkBox* window_vbox;
	GtkDrawingArea* area;
	GtkLabel* window_label;
	GSList* handlers;

	GtkStyleContext* style_context;

	guint selected_rows;
	guint selected_cols;

	guint total_rows;
	guint total_cols;

	guint max_rows;
	guint max_cols;
	gchar * szTable;
	gchar * szCancel;
} AbiTable;

typedef struct
{
	GtkButtonClass parent_class;

	/* Signals emited by this widget */
	void (* selected) (AbiTable *abi_table, guint rows, guint cols);
} AbiTableClass;


GType    abi_table_get_type   (void);
GtkWidget *abi_table_new        (void);

/* sets the number of selected rows & cols */
void	   abi_table_set_selected   (AbiTable* abi_table, guint rows, guint cols);
/* gets the number of selected rows & cols */
void	   abi_table_get_selected   (const AbiTable* abi_table, guint* rows, guint* cols);

/* sets the maximum number of selected rows & cols */
void	   abi_table_set_max_size   (AbiTable* abi_table, guint rows, guint cols);
/* gets the maximum number of selected rows & cols */
void	   abi_table_get_max_size   (const AbiTable* abi_table, guint* rows, guint* cols);

	/* Sets the labels */
	void abi_table_set_labels(AbiTable* abi_table, const gchar * szTable, const gchar * szCancel);

	/* Sets the table icon to the gtk_image cast as GtkWidget */

	void abi_table_set_icon(AbiTable* abi_table, GtkWidget* icon);

#ifdef __cplusplus
}
#endif

#endif /* ABI_TABLE_H */
