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
#include <gdk/gdk.h>
#include <glade/glade.h>
#include "ut_types.h"
#include "ut_string_class.h"
#include "xap_Strings.h"

class XAP_Frame;
class XAP_FrameHelper;
class XAP_App;
class XAP_Dialog;

// This macro sets up stack pointers to be used with the ConvertToUnixString
// macro.
#define SETUP_UNIX_STRING  	XML_Char * newstr = NULL;

// This macro is for use in Unix dialogs where the strings are to
// be stripped of ampersands (Windows accelerator characters).
#define	CONVERT_TO_UNIX_STRING(str, id, newstr)		do { \
                                                    FREEP(newstr); \
													UT_UTF8String _s; \
                                                    pSS->getValueUTF8(id,_s); \
                                                    UT_XML_cloneNoAmpersands(newstr, _s.c_str()); \
                                                    } while (0)
												   

void connectFocus(GtkWidget *widget,const XAP_Frame *frame);
void connectFocusModeless(GtkWidget *widget,const XAP_App *pApp);
void connectFocusModelessOther(GtkWidget *widget,const XAP_App *pApp,gboolean (*other_function)(void) );
bool isTransientWindow(GtkWindow *window,GtkWindow *parent);
												   
// This is a very thin message box; only use it for startup errors
// or places where you can't use the message box class (like when
// you don't have a frame yet).
void messageBoxOK(const char * message);

// Centers a GTK window inside a parent window 
void centerDialog(GtkWidget * parent, GtkWidget * child, bool set_transient_for = true);

// Converts all a given window's &'ed labels into accelerators for the
//		checkbutton/button.  It will also set the accel-groups 
void createLabelAccelerators( GtkWidget * widget );

GtkStyle * get_ensured_style (GtkWidget * w);

// creates a GtkDrawingArea, and pushes/pops correct visual and colormap
GtkWidget *createDrawingArea ();

void abiSetupModalDialog(GtkDialog * me, XAP_Frame *pFrame, XAP_Dialog * pDlg, gint dfl_id);
gint abiRunModalDialog(GtkDialog * me, bool destroyDialog, AtkRole role = ATK_ROLE_DIALOG);
gint abiRunModalDialog(GtkDialog * me, XAP_Frame *pFrame, XAP_Dialog * pDlg, gint dfl_id, bool destroyDialog, AtkRole role = ATK_ROLE_DIALOG);
void abiSetupModelessDialog(GtkDialog * me, XAP_Frame * pFrame, XAP_Dialog * pDlg, gint dfl_id, bool abi_modeless = true, AtkRole role = ATK_ROLE_DIALOG);
void abiDestroyWidget(GtkWidget * me);

GtkWidget* abiAddStockButton (GtkDialog * me, const gchar * btn_id,
			      gint response_id) ;
GtkWidget* abiAddButton(GtkDialog * me, const gchar * btn_id,
			gint response_id);

GtkWidget * abiDialogNew(const char * role, gboolean resizable = FALSE);
GtkWidget * abiDialogNew(const char * role, gboolean resizable, const char * title, ...);
GladeXML * abiDialogNewFromXML(const char * glade_file);
GtkWidget * abiGtkMenuFromCStrVector(const UT_GenericVector<const char*> & vec, GCallback cb, gpointer data);

void abiDialogSetTitle(GtkWidget * dlg, const char * title, ...);

void convertMnemonics(XML_Char * s);

void localizeLabel(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id);
void localizeLabelUnderline(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id);
void localizeLabelMarkup(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id);
void localizeButton(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id);
void localizeButtonUnderline(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id);
void localizeButtonMarkup(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id);
void localizeMenu(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id);
void setLabelMarkup(GtkWidget * widget, const gchar * str);

UT_String abiLocaleToUTF8(const UT_String & inStr);
UT_String abiLocaleToUTF8(const char * str);

// Returns the root window of the X display, useful for doing
// pointer or coordinate measurement on an absolute scale.
GdkWindow * getRootWindow(GtkWidget * widget);

#endif /* UT_DIALOGHELPER_H */
