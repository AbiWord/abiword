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

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"

// default GTK message box button width, in GTK screen units (pixels)
#define DEFAULT_BUTTON_WIDTH	85

/*****************************************************************/

gint s_key_pressed(GtkWidget * widget, GdkEventKey * e)
{
	UT_ASSERT(e);

	guint key = e->keyval;

	// TODO this is hard coded, maybe fix it
	if (key == 'k' ||
		key == GDK_Escape)
	{
		gtk_main_quit();
	}

	return TRUE;
}

/*
  This is a small message box for startup warnings and/or
  errors.  Please do NOT use this for normal system execution
  user messages; use the XAP_UnixDialog_MessageBox class for that.
  We can't use that here because there is no parent frame, etc.
*/
void messageBoxOK(const char * message)
{
	// New GTK+ dialog window
	GtkWidget * dialog_window = gtk_dialog_new();								 

	gtk_signal_connect_after (GTK_OBJECT (dialog_window),
							  "destroy",
							  NULL,
							  NULL);
	gtk_signal_connect_after (GTK_OBJECT (dialog_window),
							  "delete_event",
							  GTK_SIGNAL_FUNC(gtk_main_quit),
							  NULL);

	gtk_window_set_title(GTK_WINDOW(dialog_window), "AbiWord");

	// don't let user shrink or expand, but auto-size to
	// contents initially
    gtk_window_set_policy(GTK_WINDOW(dialog_window),
						  FALSE,
						  FALSE,
						  TRUE);

	// Intercept key strokes
	gtk_signal_connect(GTK_OBJECT(dialog_window),
					   "key_press_event",
					   GTK_SIGNAL_FUNC(s_key_pressed),
					   NULL);

	// Add our label string to the dialog in the message area
	GtkWidget * label = gtk_label_new(message);
	gtk_misc_set_padding(GTK_MISC(label), 10, 10);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG (dialog_window)->vbox),
					   label, TRUE, TRUE, 0);
	gtk_widget_show(label);

	GtkWidget *		ok_label;
	GtkWidget * 	ok_button;
	guint			ok_accel;
	
	// create the OK
	ok_label = gtk_label_new("SHOULD NOT APPEAR");
	ok_accel = gtk_label_parse_uline(GTK_LABEL(ok_label), "O_K");
	gtk_widget_show(ok_label);
	ok_button = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(ok_button), ok_label);
	gtk_signal_connect (GTK_OBJECT (ok_button),
						"clicked",
						GTK_SIGNAL_FUNC(gtk_main_quit),
						NULL);
	GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);
	gtk_widget_set_usize(ok_button, DEFAULT_BUTTON_WIDTH, 0);

	// pack the OK
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_window)->action_area),
						ok_button, FALSE, FALSE, 0);
	gtk_widget_grab_default (ok_button);
	gtk_widget_show (ok_button);

	// set the size of the dialog to size with the label inside
	gtk_widget_size_request(dialog_window, &dialog_window->requisition);
	gtk_widget_set_usize(dialog_window, dialog_window->requisition.width + 40, 0);
	
	// center it (in what?)
//	GdkWindowPrivate * rootWindow = ::getRootWindow(dialog_window);
//  centerDialog((GdkWindow *) rootWindow, dialog_window);
//	gtk_window_set_transient_for(GTK_WINDOW(dialog_window), GTK_WINDOW(parent));

	gtk_grab_add(GTK_WIDGET(dialog_window));
	gtk_widget_show(dialog_window);
	
	gtk_main();

	// clean up
	gtk_widget_destroy(GTK_WIDGET(dialog_window));
}

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

gint searchCList(GtkCList * clist, char * compareText)
{
	UT_ASSERT(clist);

	// if text is null, it's not found
	if (!compareText)
		return -1;
	
	gchar * text[2] = {NULL, NULL};
	
	for (gint i = 0; i < clist->rows; i++)
	{
		gtk_clist_get_text(clist, i, 0, text);
		if (text && text[0])
			if (!UT_stricmp(text[0], compareText))
				return i;
	}

	return -1;
}
