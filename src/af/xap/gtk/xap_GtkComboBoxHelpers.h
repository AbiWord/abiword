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

#ifndef __XAP_GTKCOMBOBOXHELPERS_H__
#define __XAP_GTKCOMBOBOXHELPERS_H__

#include <gtk/gtk.h>

#include "ut_vector.h"
#include <string>

void XAP_makeGtkComboBoxText(GtkComboBox * combo, GType secondaryType);
void XAP_makeGtkComboBoxText2(GtkComboBox * combo, GType secondaryType,
							  GType tertiaryType);
void XAP_populateComboBoxWithIndex(GtkComboBox * combo,
								   const UT_GenericVector<const char*> & vec);

void XAP_appendComboBoxText(GtkComboBox* combo, const char* text);
void XAP_appendComboBoxTextAndInt(GtkComboBox * combo, const char * text, int value);
void XAP_appendComboBoxTextAndString(GtkComboBox * combo, const char * text,
									 const char * value);
void XAP_appendComboBoxTextAndStringString(GtkComboBox * combo,
										   const char * text,
										   const char * value1,
										   const char * value2);
void XAP_appendComboBoxTextAndIntString(GtkComboBox * combo,
										   const char * text,
										   int value1,
										   const char * value2);
int  XAP_comboBoxGetActiveInt(GtkComboBox * combo);
std::string XAP_comboBoxGetActiveText(GtkComboBox * combo);



/** set the active item based on a column value
 * @param combo the combobox
 * @param col the column
 * @param value the value to look for
 * @return true if set, false if not found.
 */
bool XAP_comboBoxSetActiveFromIntCol(GtkComboBox * combo,
									 int col, int value);

#endif
