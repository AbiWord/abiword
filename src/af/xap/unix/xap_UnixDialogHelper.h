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

#ifndef UT_DIALOGHELPER_H
#define UT_DIALOGHELPER_H

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include"ut_types.h"


class XAP_Frame;
class XAP_App;
class XAP_Dialog;

// This macro sets up stack pointers to be used with the ConvertToUnixString
// macro.
#define SETUP_UNIX_STRING  	XML_Char * newstr = NULL;

// This macro is for use in Unix dialogs where the strings are to
// be stripped of ampersands (Windows accelerator characters).
#define	CONVERT_TO_UNIX_STRING(str, id, newstr)		do { \
                                                    FREEP(newstr); \
                                                    UT_XML_cloneNoAmpersands(newstr, pSS->getValue(id)); \
                                                    } while (0)
												   

void connectFocus(GtkWidget *widget,const XAP_Frame *frame);
void connectFocusModeless(GtkWidget *widget,const XAP_App *pApp);
void connectFocusModelessOther(GtkWidget *widget,const XAP_App *pApp,gboolean (*other_function)(void) );
bool isTransientWindow(GtkWindow *window,GtkWindow *parent);
												   
// This is a very thin message box; only use it for startup errors
// or places where you can't use the message box class (like when
// you don't have a frame yet).
void messageBoxOK(const char * message);

// Returns the root window of the X display, useful for doing
// pointer or coordinate measurement on an absolute scale.
GdkWindowPrivate * getRootWindow(GtkWidget * widget);

// Centers a GTK window inside a parent window 
void centerDialog(GtkWidget * parent, GtkWidget * child);

// Returns the index of an entry in a GtkCList by entry contents
gint searchCList(GtkCList * clist, char * compareText);

// Converts all a given window's &'ed labels into accelerators for the
//		checkbutton/button.  It will also set the accel-groups 
void createLabelAccelerators( GtkWidget * widget );

#ifdef HAVE_GNOME
#include <gnome.h>
void setDefaultButton (GnomeDialog * dlg, int which);
#endif

GtkStyle * get_ensured_style (GtkWidget * w);

// creates a GtkDrawingArea, and pushes/pops correct visual and colormap
GtkWidget *createDrawingArea ();

gint abiRunModalDialog(GtkDialog * me, XAP_Frame * pFrame, XAP_Dialog * pDlg, bool destroyDialog = true);
void abiRunModelessDialog(GtkDialog * me, XAP_Frame * pFrame, XAP_Dialog * pDlg);
void abiDestroyWidget(GtkWidget * me);

GtkWidget * abiDialogNew(gboolean resizable = FALSE);
GtkWidget * abiDialogNew(gboolean resizable, const char * title, ...);

// These are here so that I can start on the GTK+2 port without installing
// GTK+2 quite yet...
#define G_OBJECT(x) GTK_OBJECT(x)
#define G_CALLBACK(x) GTK_SIGNAL_FUNC(x)
#define G_OBJECT_CLASS(x) GTK_OBJECT_CLASS(x)
#define G_IS_OBJECT(x) GTK_IS_OBJECT(x)

#define g_object_ref(x) gtk_object_ref(x)
#define g_object_unref(x) gtk_object_unref(x)
#define g_object_set(a,b,c,d) gtk_object_set(a,b,c,d)
#define g_object_set_data(x,y,z) gtk_object_set_data(x,y,z)
#define g_object_get_data(x,y) gtk_object_get_data(x,y)
#define g_object_set_data_full(a,b,c,d) gtk_object_set_data_full(a,b,c,d)

#define g_signal_connect(o,s,c,d) gtk_signal_connect(o,s,c,d)
#define g_signal_connect_after(a,b,c,d) gtk_signal_connect_after(a,b,c,d)
#define g_signal_handler_block(w,i) gtk_signal_handler_block(w,i)
#define g_signal_handler_unblock(w,i) gtk_signal_handler_block(w,i)
#define g_signal_emit_stop_by_name(w,s) gtk_signal_emit_stop_by_name(w,s)
#define g_signal_emit_by_name(w,s) gtk_signal_emit_by_name(w,s)
#define g_signal_handler_block_by_data(a,b) gtk_signal_handler_block_by_data(a,b)
#define g_signal_handler_unblock_by_data(a,b) gtk_signal_handler_unblock_by_data(a,b)

typedef GtkObject GObject;

#endif /* UT_DIALOGHELPER_H */








