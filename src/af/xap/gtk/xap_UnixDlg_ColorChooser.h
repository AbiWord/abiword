/* AbiWord
 * Copyright (C) 2013 Hubert Figuiere
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

#ifndef __XAP_UNIXDLG_COLORCHOOSER_H__
#define __XAP_UNIXDLG_COLORCHOOSER_H__

#include <memory>

#include <gtk/gtk.h>

#include "ut_color.h"


// TODO I'm sure we can make this XP. currently only Gtk
// The use of unique_ptr<> is only meant to ensure the deletion
// out of scope when returning.
std::unique_ptr<UT_RGBColor> XAP_UnixDlg_RunColorChooser(GtkWindow* parent,
						       GtkColorButton* colorBtn);

#endif
