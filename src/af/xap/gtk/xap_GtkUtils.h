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

#ifndef _XAP_GTK2UTILS_H_
#define _XAP_GTK2UTILS_H_

#include <gtk/gtk.h>

// Now onto the device were the deprecated functions are to be
// with several line of code.
GdkGrabStatus XAP_gdk_keyboard_grab(GdkWindow *window,
				    gboolean owner_events,
				    guint32 time_);

GdkGrabStatus XAP_gdk_pointer_grab(GdkWindow *window,
				   gboolean owner_events,
				   GdkEventMask event_mask,
				   GdkCursor *cursor,
				   guint32 time_);

// http://permalink.gmane.org/gmane.comp.gnome.svn/520942
void XAP_gdk_keyboard_ungrab(guint32 t);

void XAP_gdk_pointer_ungrab(guint32 t);

#endif
