

/* AbiWord
 * Copyright (C) 2009 Hubert Figuiere
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


#include "xap_GtkComboBoxHelpers.h"


void XAP_makeGtkComboBoxText(GtkComboBox * combo, bool withIntData)
{
	GtkListStore * store;
	if (withIntData) {
		store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
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

