/* AbiWord
 * Copyright (C) 2009,2012 Hubert Figuiere
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

#include "xap_GtkComboBoxHelpers.h"

void XAP_makeGtkComboBoxText(GtkComboBox * combo, GType secondary)
{
	GtkListStore * store;
	if (secondary != G_TYPE_NONE) {
		store = gtk_list_store_new(2, G_TYPE_STRING, secondary);
	}
	else {
		store = gtk_list_store_new(1, G_TYPE_STRING);
	}
	gtk_combo_box_set_model(combo, GTK_TREE_MODEL(store));
	
	gtk_cell_layout_clear(GTK_CELL_LAYOUT(combo));
	GtkCellRenderer *cell = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), cell, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), cell,
								   "text", 0, NULL);
}

void XAP_makeGtkComboBoxText2(GtkComboBox * combo, GType secondary,
							  GType tertiary)
{
	GtkListStore * store;
	store = gtk_list_store_new(3, G_TYPE_STRING, secondary, tertiary);
	gtk_combo_box_set_model(combo, GTK_TREE_MODEL(store));
	
	gtk_cell_layout_clear(GTK_CELL_LAYOUT(combo));
	GtkCellRenderer *cell = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), cell, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), cell,
								   "text", 0, NULL);
}

void XAP_populateComboBoxWithIndex(GtkComboBox * combo, 
								   const UT_GenericVector<const char*> & vec)
{
	GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(combo));
	GtkTreeIter iter;
	
	for(UT_sint32 i = 0; i < vec.getItemCount(); i++) {
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, vec[i], 1, i, -1);
	}
}

void XAP_appendComboBoxText(GtkComboBox* combo, const char* text)
{
	GtkTreeIter iter;
	GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(combo));
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, text, -1);
}

void XAP_appendComboBoxTextAndInt(GtkComboBox * combo, const char * text,
								  int value)
{
	GtkTreeIter iter;
	GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(combo));
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, text, 1, value, -1);
}

void XAP_appendComboBoxTextAndString(GtkComboBox * combo, const char * text,
									 const char * value)
{
	GtkTreeIter iter;
	GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(combo));
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, text, 1, value, -1);
}

void XAP_appendComboBoxTextAndStringString(GtkComboBox * combo, 
										   const char * text,
										   const char * value,
										   const char *value2)
{
	GtkTreeIter iter;
	GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(combo));
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, text, 1, value, 2, value2, -1);
}

void XAP_appendComboBoxTextAndIntString(GtkComboBox * combo, 
										const char * text,
										const int value,
										const char *value2)
{
	GtkTreeIter iter;
	GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(combo));
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, text, 1, value, 2, value2, -1);
}

int  XAP_comboBoxGetActiveInt(GtkComboBox * combo)
{
	int value = 0;
	GtkTreeIter iter;
	gtk_combo_box_get_active_iter(combo, &iter);
	GtkTreeModel *store = gtk_combo_box_get_model(combo);
	gtk_tree_model_get(store, &iter, 1, &value, -1);
	return value;
}

std::string XAP_comboBoxGetActiveText(GtkComboBox * combo)
{
    char* value = 0;
	GtkTreeIter iter;
	gtk_combo_box_get_active_iter(combo, &iter);
	GtkTreeModel *store = gtk_combo_box_get_model(combo);
	gtk_tree_model_get(store, &iter, 0, &value, -1);
	return value;
}

bool XAP_comboBoxSetActiveFromIntCol(GtkComboBox * combo, 
									 int col, int value)
{
	GtkTreeIter iter;
	GtkTreeModel *store = gtk_combo_box_get_model(combo);
	if(gtk_tree_model_get_iter_first(store, &iter)) {
		do {
			int v;
			gtk_tree_model_get(store, &iter, col, &v, -1);
			if(v == value) {
				gtk_combo_box_set_active_iter(combo, &iter);
				return true;
			}
		} while(gtk_tree_model_iter_next(store, &iter));
	}
	return false;
}
