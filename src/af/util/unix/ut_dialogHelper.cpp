/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <glib.h>
#include "ut_debugmsg.h"
#include "ut_assert.h"

GdkWindowPrivate * getRootWindow(GtkWidget * widget)
{
	UT_ASSERT(widget);

	GdkWindowPrivate * priv = (GdkWindowPrivate *) widget->window;
	while (priv->parent && ((GdkWindowPrivate*) priv->parent)->parent)
		priv = (GdkWindowPrivate*) priv->parent;

	return (GdkWindowPrivate *) priv;
}
	  
void centerDialog(GtkWidget * parent, GtkWidget * child)
{
	UT_ASSERT(parent);
	UT_ASSERT(child);

	UT_ASSERT(parent->window);
	gint parentx = 0;
	gint parenty = 0;
	gint parentwidth = 0;
	gint parentheight = 0;
	gdk_window_get_origin(parent->window, &parentx, &parenty);
	gdk_window_get_size(parent->window, &parentwidth, &parentheight);
	UT_ASSERT(parentwidth > 0 && parentheight > 0);

	// this message box's geometry (it won't have a ->window yet, so don't assert it)
	gint width = 0;
	gint height = 0;
	gtk_widget_size_request(child, &child->requisition);
	width = child->requisition.width;
	height = child->requisition.height;
	UT_ASSERT(width > 0 && height > 0);

	// set new place
	gint newx = parentx + ((parentwidth - width) / 2);
	gint newy = parenty + ((parentheight - height) / 2);

	// measure the root window
	gint rootwidth = gdk_screen_width();
	gint rootheight = gdk_screen_height();
	// if the dialog won't fit on the screen, panic and center on the root window
	if ((newx + width) > rootwidth || (newy + height) > rootheight)
	{
		UT_DEBUGMSG(("Dialog [%p] at coordiantes [%d, %d] is in non-viewable area; "
					 "centering on desktop instead.\n", child, newx, newy));
		gtk_window_position(GTK_WINDOW(child), GTK_WIN_POS_CENTER);
	}
	else
		gtk_widget_set_uposition(child, newx, newy);
	
}
