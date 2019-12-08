/* AbiWord
 * Copyright (C) 2011-2016 Hubert Figuiere
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


#include <gtk/gtk.h>

#include "xap_GtkUtils.h"

void XAP_gtk_window_raise(GtkWidget* w)
{
  gdk_window_raise(gtk_widget_get_window(w));
}

void XAP_gtk_widget_set_margin(GtkWidget* w, gint margin)
{
  gtk_widget_set_margin_bottom(w, margin);
  gtk_widget_set_margin_top(w, margin);
  gtk_widget_set_margin_start(w, margin);
  gtk_widget_set_margin_end(w, margin);
}

void XAP_gtk_keyboard_ungrab(GtkWidget* widget)
{
  GdkSeat *seat = gdk_display_get_default_seat(gtk_widget_get_display(widget));
  gdk_seat_ungrab(seat);
}
