/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef ABI_TABLE_H
#define ABI_TABLE_H

#include <gtk/gtkwidget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _AbiTable
{
	GtkButton button;

	/*<private>*/
	GtkStockItem stock_item;

	GtkWidget* button_box;
	GtkWidget* label;
	GtkWidget* icon;

	GtkWindow* window;
	GtkVBox* window_vbox;
	GtkDrawingArea* area;
	GtkLabel* window_label;
	GSList* handlers;
	
	GdkGC* selected_gc;
	
	guint selected_rows;
	guint selected_cols;

	guint total_rows;
	guint total_cols;

	guint max_rows;
	guint max_cols;
} AbiTable;

typedef struct
{
	GtkButtonClass parent_class;

	/* Signals emited by this widget */
	void (* selected) (AbiTable *abi_table, int rows, int cols);
} AbiTableClass;

#define ABI_TABLE_TYPE     (abi_table_get_type ())
#define ABI_TABLE(obj)     (GTK_CHECK_CAST((obj), ABI_TABLE_TYPE, AbiTable))
#define ABI_TABLE_CLASS(k) (GTK_CHECK_CLASS_CAST(k), ABI_TABLE_TYPE)
#define IS_ABI_TABLE(obj)  (GTK_CHECK_TYPE((obj), ABI_TABLE_TYPE))


GtkType    abi_table_get_type   (void);
GtkWidget *abi_table_new        (void);

/* sets the number of selected rows & cols */
void	   abi_table_set_selected   (AbiTable* abi_table, guint rows, guint cols);
/* gets the number of selected rows & cols */
void	   abi_table_get_selected   (const AbiTable* abi_table, guint* rows, guint* cols);

/* sets the maximum number of selected rows & cols */
void	   abi_table_set_max_size   (AbiTable* abi_table, guint rows, guint cols);
/* gets the maximum number of selected rows & cols */
void	   abi_table_get_max_size   (const AbiTable* abi_table, guint* rows, guint* cols);

void	   abi_table_embed_on_toolbar (AbiTable* abi_table, GtkToolbar* toolbar);

#ifdef __cplusplus
}
#endif

#endif /* ABI_TABLE_H */
