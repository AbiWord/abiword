/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

/*
 * Port to Maemo Development Platform 
 * Author: INdT - Renato Araujo <renato.filho@indt.org.br>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <string>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <goffice/gtk/goffice-gtk.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "xav_View.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "xap_App.h"
#include "xap_UnixDialogHelper.h"
#include "xap_Dialog.h"
#include "xap_Gtk2Compat.h"
#include "xap_Strings.h"

/*****************************************************************/
/*****************************************************************/

static gboolean focus_in_event(GtkWidget *widget,GdkEvent */*event*/,gpointer /*user_data*/)
{
      XAP_Frame *pFrame=static_cast<XAP_Frame *>(g_object_get_data(G_OBJECT(widget), "frame"));
	  if (pFrame && pFrame->getCurrentView())
		  pFrame->getCurrentView()->focusChange(AV_FOCUS_NEARBY);
      return FALSE;
}

static gboolean destroy_event(GtkWidget * /*widget*/ ,GdkEvent */*event*/,gpointer /*user_data*/)
{
      return FALSE;
}

static gboolean focus_out_event(GtkWidget *widget,GdkEvent */*event*/,gpointer /*user_data*/)
{
      XAP_Frame *pFrame=static_cast<XAP_Frame *>(g_object_get_data(G_OBJECT(widget), "frame"));
      if(pFrame == NULL) return FALSE;
      AV_View * pView = pFrame->getCurrentView();
      if(pView!= NULL)
      {
	     pView->focusChange(AV_FOCUS_NONE);
      }
      return FALSE;
}

static gboolean focus_out_event_Modeless(GtkWidget *widget,GdkEvent */*event*/,gpointer /*user_data*/)
{
      XAP_App *pApp = static_cast<XAP_App *>(g_object_get_data(G_OBJECT(widget), "pApp"));
      XAP_Frame *pFrame = pApp->getLastFocussedFrame();
      if(pFrame ==static_cast<XAP_Frame *>(NULL)) 
      {
             UT_uint32 nframes =  pApp->getFrameCount();
             if(nframes > 0 && nframes < 10)
	     {     
	            pFrame = pApp->getFrame(0);
	     }
             else
	     {
	            return FALSE;
	     }
      }
      if(pFrame == static_cast<XAP_Frame *>(NULL)) return FALSE;
      AV_View * pView = pFrame->getCurrentView();
      UT_ASSERT_HARMLESS(pView);
      if(pView!= NULL)
      {
	     pView->focusChange(AV_FOCUS_NONE);
      }
      return FALSE;
}


static gboolean focus_in_event_Modeless(GtkWidget *widget,GdkEvent */*event*/,gpointer /*user_data*/)
{
      XAP_App *pApp=static_cast<XAP_App *>(g_object_get_data(G_OBJECT(widget), "pApp"));
      XAP_Frame *pFrame= pApp->getLastFocussedFrame();
      if(pFrame ==static_cast<XAP_Frame *>(NULL))
      {
             UT_uint32 nframes =  pApp->getFrameCount();
             if(nframes > 0 && nframes < 10)
	     {     
	            pFrame = pApp->getFrame(0);
	     }
             else
	     {
	            return FALSE;
	      }
      }
      if(pFrame == static_cast<XAP_Frame *>(NULL)) return FALSE;
      AV_View * pView = pFrame->getCurrentView();
      if(pView!= NULL)
      {
            pView->focusChange(AV_FOCUS_MODELESS);
      }
      return FALSE;
}


static gboolean focus_in_event_ModelessOther(GtkWidget *widget,GdkEvent */*event*/,
	std::pointer_to_unary_function<int, gboolean> *other_function)
{
      XAP_App *pApp=static_cast<XAP_App *>(g_object_get_data(G_OBJECT(widget), "pApp"));
      XAP_Frame *pFrame= pApp->getLastFocussedFrame();
      if(pFrame == static_cast<XAP_Frame *>(NULL)) 
      {
             UT_uint32 nframes =  pApp->getFrameCount();
             if(nframes > 0 && nframes < 10)
	     {     
	            pFrame = pApp->getFrame(0);
	     }
             else
	     {
	            return FALSE;
	      }
      }
      if(pFrame == static_cast<XAP_Frame *>(NULL)) 
	return FALSE;
      AV_View * pView = pFrame->getCurrentView();
      if(pView!= NULL)
      {
            pView->focusChange(AV_FOCUS_MODELESS);
            (*other_function)(0);
      }
      return FALSE;
}

/*****************************************************************/

GtkBuilder * newDialogBuilder(const char * name)
{
    UT_ASSERT(name);
	std::string ui_path = static_cast<XAP_UnixApp*>(XAP_App::getApp())->getAbiSuiteAppUIDir() + "/" + name;

	// load the dialog from the UI file
	GtkBuilder* builder = gtk_builder_new();
    GError * error = NULL;
	guint result = gtk_builder_add_from_file(builder, ui_path.c_str(), &error);
    if(result == 0) {
        if(error) {
            UT_DEBUGMSG(("gtk_builder_add() for %s failed with '%s'.", 
                         ui_path.c_str(), error->message));
            g_error_free(error);
        }
        g_object_unref((GObject*)builder);
        return NULL;
    }
    return builder;
}


/*****************************************************************/

void connectFocus(GtkWidget *widget,const XAP_Frame *frame)
{
      g_object_set_data(G_OBJECT(widget), "frame",
					  const_cast<void *>(static_cast<const void *>(frame)));
      g_signal_connect(G_OBJECT(widget), "focus_in_event",
					 G_CALLBACK(focus_in_event), NULL);
      g_signal_connect(G_OBJECT(widget), "focus_out_event",
					 G_CALLBACK(focus_out_event), NULL);
      g_signal_connect(G_OBJECT(widget), "destroy",
					 G_CALLBACK(destroy_event), NULL);
}

void connectFocusModelessOther(GtkWidget *widget,const XAP_App * pApp, 
			       std::pointer_to_unary_function<int, gboolean> *other_function)
{
      g_object_set_data(G_OBJECT(widget), "pApp",
					  const_cast<void *>(static_cast<const void *>(pApp)));
      g_signal_connect(G_OBJECT(widget), "focus_in_event",
					 G_CALLBACK(focus_in_event_ModelessOther),
					 (gpointer) other_function); // leave as C-style cast
      g_signal_connect(G_OBJECT(widget), "focus_out_event",
					 G_CALLBACK(focus_out_event_Modeless), NULL);
      g_signal_connect(G_OBJECT(widget), "destroy",
					 G_CALLBACK(focus_out_event_Modeless), NULL);
}


void connectFocusModeless(GtkWidget *widget,const XAP_App * pApp)
{
      g_object_set_data(G_OBJECT(widget), "pApp",
					  const_cast<void *>(static_cast<const void *>(pApp)));
      g_signal_connect(G_OBJECT(widget), "focus_in_event",
					 G_CALLBACK(focus_in_event_Modeless), NULL);
      g_signal_connect(G_OBJECT(widget), "focus_out_event",
					 G_CALLBACK(focus_out_event_Modeless), NULL);
      g_signal_connect(G_OBJECT(widget), "destroy",
		       G_CALLBACK(destroy_event), NULL);
}


bool isTransientWindow(GtkWindow *window,GtkWindow *parent)
{
  GtkWindow *transient;
  if(window)
	{
	  while((transient=gtk_window_get_transient_for(window)))
		{
		  window=transient;
		  if(window==parent)
			return true;
		}
	}
  return false;
}

/****************************************************************/
/****************************************************************/

// in ap_editmethods.cpp
extern bool helpLocalizeAndOpenURL(const char* pathBeforeLang, const char* pathAfterLang, const char *remoteURLbase);

static void sDoHelp ( XAP_Dialog * pDlg )
{
	// should always be valid, but just in case...
	if (!pDlg)
		return;

	// open the url
	if ( pDlg->getHelpUrl().size () > 0 )
    {
		helpLocalizeAndOpenURL ("help", pDlg->getHelpUrl().c_str(), NULL );
    }
	else
    {
		// TODO: warn no help on this topic
		UT_DEBUGMSG(("NO HELP FOR THIS TOPIC!!\n"));
    }
}

/*!
 * Catch F1 keypress over a dialog and open up the help file, if any
 */
static gint modal_keypress_cb ( GtkWidget * /*wid*/, GdkEventKey * event, 
								XAP_Dialog * pDlg )
{
	// propegate keypress up if not F1
	if ( event->keyval == GDK_KEY_F1 || event->keyval == GDK_KEY_Help )
	{
		sDoHelp( pDlg ) ;

		// stop F1 propegation
		return TRUE ;
	}
	
	return FALSE ;		
}

/*!
 * Catch F1 keypress over a dialog and open up the help file, if any
 */
static gint nonmodal_keypress_cb ( GtkWidget * /*wid*/, GdkEventKey * event, 
								   XAP_Dialog * pDlg )
{
	// propegate keypress up if not F1
	if ( event->keyval == GDK_KEY_F1 || event->keyval == GDK_KEY_Help )
	{
		sDoHelp( pDlg ) ;
		return TRUE ;
	}

	return FALSE ;
}

#if !defined(EMBEDDED_TARGET) || EMBEDDED_TARGET != EMBEDDED_TARGET_HILDON
static void help_button_cb (GObject * /*button*/, XAP_Dialog * pDlg)
{
    if (pDlg) {
        sDoHelp (pDlg);
    }
}
#endif

static void sAddHelpButton (GtkDialog * me, XAP_Dialog * pDlg)
{
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
    UT_UNUSED(me);
    UT_UNUSED(pDlg);
#else
  // prevent help button from being added twice
    gint has_button = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (me), "has-help-button"));

    if (has_button)
        return;

    if (pDlg && pDlg->getHelpUrl().size () > 0) {

        std::string s;
        const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
        pSS->getValueUTF8(XAP_STRING_ID_DLG_HelpButton, s);

        GtkWidget * button = gtk_dialog_add_button(me,
                                                   convertMnemonics(s).c_str(),
                                                   GTK_RESPONSE_HELP);
        g_signal_connect (G_OBJECT (button), "clicked",
                          G_CALLBACK(help_button_cb), pDlg);
        g_object_set_data (G_OBJECT (me), "has-help-button", GINT_TO_POINTER (1));
    }
#endif
}

/*!
 * Centers a dialog, makes it transient, sets up the right window icon
 */
void centerDialog(GtkWidget * parent, GtkWidget * child, bool set_transient_for)
{
	UT_return_if_fail(parent);
	UT_return_if_fail(child);

	if (GTK_IS_DIALOG(child))
	  go_dialog_guess_alternative_button_order(GTK_DIALOG(child));
	if(GTK_IS_WINDOW(parent) != TRUE)
		parent  = gtk_widget_get_parent(parent);
	xxx_UT_DEBUGMSG(("center IS WIDGET_TOP_LEVL %d \n",(GTK_WIDGET_TOPLEVEL(parent))));
	xxx_UT_DEBUGMSG(("center IS WIDGET WINDOW %d \n",(GTK_IS_WINDOW(parent))));
	xxx_UT_DEBUGMSG(("center child IS WIDGET WINDOW %d \n",(GTK_IS_WINDOW(child))));
	if (set_transient_for)
	  gtk_window_set_transient_for(GTK_WINDOW(child),
				       GTK_WINDOW(parent));

	GdkPixbuf * icon = gtk_window_get_icon(GTK_WINDOW(parent));	
	if ( NULL != icon )
	{
		gtk_window_set_icon(GTK_WINDOW(child), icon);
	}
}

void abiSetupModalDialog(GtkDialog * dialog, XAP_Frame *pFrame, XAP_Dialog * pDlg, gint defaultResponse)
{
	GtkWidget *popup = GTK_WIDGET (dialog);
	gtk_dialog_set_default_response (GTK_DIALOG (popup), defaultResponse);
	gtk_window_set_modal (GTK_WINDOW(popup), TRUE);

	// To center the dialog, we need the frame of its parent.
	if (pFrame)
	{
		XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl());
		GtkWidget * parentWindow = pUnixFrameImpl->getTopLevelWindow();
		if (GTK_IS_WINDOW(parentWindow) != TRUE)
			parentWindow  = gtk_widget_get_parent(parentWindow);
		centerDialog (parentWindow, GTK_WIDGET(popup));
	}
	connectFocus (GTK_WIDGET(popup), pFrame);

	// connect F1 to the help subsystem
	g_signal_connect (G_OBJECT(popup), "key-press-event",
					  G_CALLBACK(modal_keypress_cb), pDlg);

	// set the default response
	sAddHelpButton (GTK_DIALOG (popup), pDlg);

	// show the window
	gtk_widget_show (GTK_WIDGET (popup));
}

gint abiRunModalDialog(GtkDialog * me, bool destroyDialog, AtkRole role)
{
	atk_object_set_role (gtk_widget_get_accessible (GTK_WIDGET (me)), role);

    // now run the dialog
    gint result = GTK_RESPONSE_NONE;
    while((result = gtk_dialog_run ( me )) == GTK_RESPONSE_HELP) {

    }

    // destroy the dialog
    if ( destroyDialog ) {
        abiDestroyWidget ( GTK_WIDGET ( me ) );
    }

    return result ;
}

/*!
 * Runs the dialog \me as a modal dialog
 * 1) Connect focus to toplevel frame
 * 2) Centers dialog over toplevel window
 * 3) Connects F1 to help system
 * 4) Makes dialog modal
 * 5) Sets the default button to defaultResponse, sets ESC to close
 * 6) Returns value of gtk_dialog_run(me)
 * 7) If \destroyDialog is true, destroys the dialog, else you have to call abiDestroyWidget()
 */
gint abiRunModalDialog(GtkDialog * me, XAP_Frame *pFrame, XAP_Dialog * pDlg,
					   gint defaultResponse, bool destroyDialog, AtkRole role)
{
  abiSetupModalDialog(me, pFrame, pDlg, defaultResponse);
  gint ret = abiRunModalDialog(me, destroyDialog, role);
  if( pDlg )
  {
      pDlg->maybeReallowPopupPreviewBubbles();
  }
  return ret;
}

/*!
 * Sets up the dialog \me as a modeless dialog
 * 1) Connect focus to toplevel frame
 * 2) Centers dialog over toplevel window
 * 3) Makes the App remember this modeless dialog
 * 4) Connects F1 to help system
 * 5) Makes dialog non-modal (modeless)
 * 
6) Sets the default button to defaultResponse, sets ESC to close
 */
void abiSetupModelessDialog(GtkDialog * me, XAP_Frame * pFrame, XAP_Dialog * pDlg,
							gint defaultResponse, bool abi_modeless, AtkRole /*role*/ )
{
	if (abi_modeless)
	{
		// remember the modeless id
		XAP_App::getApp()->rememberModelessId( pDlg->getDialogId(), static_cast<XAP_Dialog_Modeless *>(pDlg));

		// connect focus to our parent frame
		connectFocusModeless(GTK_WIDGET(me), XAP_App::getApp());
	}

	// To center the dialog, we need the frame of its parent.
	if (pFrame)
	{
		XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl());
		GtkWidget * parentWindow = gtk_widget_get_toplevel (pUnixFrameImpl->getTopLevelWindow());
		centerDialog ( parentWindow, GTK_WIDGET(me), false ) ;
	}
	
	// connect F1 to the help subsystem
	g_signal_connect (G_OBJECT(me), "key-press-event",
					  G_CALLBACK(nonmodal_keypress_cb), pDlg);
	
	// set the default response
	gtk_dialog_set_default_response ( me, defaultResponse ) ;
	sAddHelpButton (me, pDlg);

	// and mark it as modeless
	gtk_window_set_modal ( GTK_WINDOW(me), FALSE ) ;
	// FIXME: shouldn't we pass role here?
	atk_object_set_role (gtk_widget_get_accessible (GTK_WIDGET (me)), ATK_ROLE_ALERT);

    pDlg->maybeClosePopupPreviewBubbles();
        
	// show the window
	gtk_widget_show ( GTK_WIDGET(me) ) ;
}

/*!
 * Create a new GtkDialog
 */
GtkWidget * abiDialogNew(const char * role, gboolean resizable)
{
  GtkWidget * dlg = gtk_dialog_new () ;
  if ( role )
    gtk_window_set_role ( GTK_WINDOW(dlg), role ) ;
  gtk_window_set_resizable ( GTK_WINDOW(dlg), resizable ) ;
  gtk_container_set_border_width ( GTK_CONTAINER (dlg), 5 ) ;
  gtk_box_set_spacing ( GTK_BOX ( gtk_dialog_get_content_area(GTK_DIALOG (dlg))), 2 ) ;
  return dlg ;
}

/*!
 * Create a new GtkDialog with this title
 */
GtkWidget * abiDialogNew(const char * role, gboolean resizable, const char * title, ...)
{
  GtkWidget * dlg = abiDialogNew(role, resizable);
  
  if ( title != NULL && strlen ( title ) )
  {
    UT_String titleStr ( "" ) ;

    va_list args;
    va_start (args, title);
    UT_String_vprintf (titleStr, title, args);
    va_end (args);

    // create the title
    gtk_window_set_title ( GTK_WINDOW(dlg), titleStr.c_str() ) ;
  }

  return dlg ;
}

/*!
 * Returns a GtkMenu with items having label fetched from UTF-8 CStr 
 * from UT_Vector.
 * All menu item will have the index of the item stored in its user-data
 */
GtkWidget * abiGtkMenuFromCStrVector(const UT_GenericVector<const char*> & vec, GCallback cb, gpointer data)
{
	UT_sint32 i;
	GtkWidget * menu = gtk_menu_new();
	
	for (i = 0; i < vec.getItemCount(); i++) {
		GtkWidget * menuItem = gtk_menu_item_new_with_label(vec[i]);
		g_object_set_data(G_OBJECT(menuItem), "user_data", reinterpret_cast<void*>(i));
		g_signal_connect(G_OBJECT(menuItem), "activate", G_CALLBACK(cb), data);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
	}
	gtk_widget_show_all(menu);
	return menu;
}


/*!
 * Set the title of a gtk dialog
 */
void abiDialogSetTitle(GtkWidget * dlg, const char * title, ...)
{
  if ( title != NULL && strlen ( title ) )
  {
    UT_String titleStr ( "" ) ;

    va_list args;
    va_start (args, title);
    UT_String_vprintf (titleStr, title, args);
    va_end (args);

    // create the title
    gtk_window_set_title ( GTK_WINDOW(dlg), titleStr.c_str() ) ;
  }
}


/*!
 * Add this stock button to the dialog and make it sensitive
 */
GtkWidget * abiAddStockButton (GtkDialog * me, const gchar * btn_id,
			       gint response_id)
{
	UT_return_val_if_fail(me, NULL);
	UT_return_val_if_fail(btn_id, NULL);
	
	GtkWidget * wid = gtk_dialog_add_button(me, btn_id, response_id);
	gtk_dialog_set_response_sensitive(me, response_id, TRUE);

	return wid;
}

/*!
 * Add this locale-sensitive button to the dialog and
 * make it sensitive
 */
GtkWidget* abiAddButton(GtkDialog * me, const gchar * btn_id,
			gint response_id)
{
	UT_return_val_if_fail(me, NULL);
	UT_return_val_if_fail(btn_id, NULL);

	// todo: possibly make me locale sensitive -> UTF-8

	GtkWidget * wid = gtk_dialog_add_button(me, btn_id, response_id);
	gtk_dialog_set_response_sensitive(me, response_id, TRUE);

	return wid ;
}

/*!
 * Calls gtk_widget_destroy on \me if \me is non-null
 * and GTK_IS_WIDGET(me)
 */
void abiDestroyWidget(GtkWidget * me)
{
  if(me && GTK_IS_WIDGET(me))
    gtk_widget_destroy(me);
}

/*!
 * Localizes a label given the string id
 */
void localizeLabel(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id)
{
	gchar * unixstr = NULL;	// used for conversions
	std::string s;
	pSS->getValueUTF8(id,s);
	UT_XML_cloneNoAmpersands(unixstr, s.c_str());
	gtk_label_set_text (GTK_LABEL(widget), unixstr);
	FREEP(unixstr);	
}

void convertMnemonics(gchar * s)
{
	UT_return_if_fail(s);

	for (UT_uint32 i = 0; s[i] != 0; i++) 
	{
		if ( s[i] == '&' ) {
			if (i > 0 && s[i-1] == '\\')
			{
				s[i-1] = '&';
				strcpy( &s[i], &s[i+1]);
				i--;
				}
			else
				s[i] = '_';
		}
	}
}


// probably much slower....
std::string & convertMnemonics(std::string & s)
{
	for (UT_uint32 i = 0; s[i] != 0; i++) 
	{
		if ( s[i] == '&' ) {
			if (i > 0 && s[i-1] == '\\')
			{
				s[i-1] = '&';
                s.erase(i);
				i--;
            }
			else
				s[i] = '_';
		}
	}

    return s;
}

/*!
 * Localizes the label of a widget given the string id
 * Ampersands will be converted to underscores/mnemonics
 */
void localizeLabelUnderline(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id)
{
	std::string s;
	pSS->getValueUTF8(id,s);
	gchar * newlbl = g_strdup(s.c_str());
	UT_ASSERT(newlbl);
	convertMnemonics(newlbl);
	gtk_label_set_text_with_mnemonic (GTK_LABEL(widget), newlbl);
	FREEP(newlbl);	
}

/*!
 * Localizes the label of a widget given the string id
 * It formats the label using the current label of the widget as a format
 * string. The current label is assumed to be something like
 * "<span size="larger">%s</span>".
 */
void localizeLabelMarkup(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id)
{
	gchar * unixstr = NULL;	// used for conversions
	std::string s;
	pSS->getValueUTF8(id,s);
	UT_XML_cloneNoAmpersands(unixstr, s.c_str());
	std::string markupStr = UT_std_string_sprintf(gtk_label_get_label (GTK_LABEL(widget)), unixstr);
	gtk_label_set_markup (GTK_LABEL(widget), markupStr.c_str());
	FREEP(unixstr);	
}

/*!
 * Localizes a button given the string id
 */
void localizeButton(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id)
{
	gchar * unixstr = NULL;	// used for conversions
	std::string s;
	pSS->getValueUTF8(id,s);
	UT_XML_cloneNoAmpersands(unixstr, s.c_str());
	gtk_button_set_label (GTK_BUTTON(widget), unixstr);
	FREEP(unixstr);	
}

/*!
 * Localizes a button given the string id
 * Ampersands will be converted to underscores/mnemonics
 */
void localizeButtonUnderline(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id)
{
	std::string s;
	pSS->getValueUTF8(id,s);
	gchar * newlbl = g_strdup(s.c_str());
	UT_ASSERT(newlbl);
	convertMnemonics(newlbl);
	gtk_button_set_use_underline (GTK_BUTTON(widget), TRUE);
	gtk_button_set_label (GTK_BUTTON(widget), newlbl);
	FREEP(newlbl);	
}

/*!
 * Localizes a button given the string id
 * It formats its label using the current button label as a format
 * string. It is assumed to be something like
 * "<span size="larger">%s</span>".
 * Note that in addition to doing markup, ampersands will be converted
 * to underscores/mnemonic since this makes sense for buttons
 */
void localizeButtonMarkup(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id)
{
	std::string s;
	pSS->getValueUTF8(id,s);
	gchar * newlbl = g_strdup(s.c_str());
	UT_ASSERT(newlbl);
	convertMnemonics(newlbl);
	std::string markupStr = UT_std_string_sprintf(gtk_button_get_label (GTK_BUTTON(widget)), newlbl);
	gtk_button_set_use_underline (GTK_BUTTON(widget), TRUE);
	gtk_button_set_label (GTK_BUTTON(widget), markupStr.c_str());

	// by default, they don't like markup, so we teach them
	GtkWidget * button_child = gtk_bin_get_child (GTK_BIN(widget));
	if (GTK_IS_LABEL (button_child))
		gtk_label_set_use_markup (GTK_LABEL(button_child), TRUE);

	FREEP(newlbl);	
}

/*!
 * Localizes the label of a Menu Item widget given the string id
 */
void localizeMenuItem(GtkWidget * widget, const XAP_StringSet * pSS, XAP_String_Id id)
{
	gchar *unixstr = NULL;
	std::string s;
	pSS->getValueUTF8(id, s);
	UT_XML_cloneConvAmpersands(unixstr, s.c_str());
	gtk_menu_item_set_label(GTK_MENU_ITEM(widget), unixstr);
	FREEP(unixstr);	
}

/*!
 * Sets the label of "widget" to "str".
 * It formats the label using the current label of the widget as a format string. The
 * current label is assumed to be something like "<span size="larger">%s</span>".
 */
void setLabelMarkup(GtkWidget * widget, const gchar * str)
{
	std::string markupStr = UT_std_string_sprintf(gtk_label_get_label (GTK_LABEL(widget)), str);
	gtk_label_set_markup (GTK_LABEL(widget), markupStr.c_str());
}

#if !GTK_CHECK_VERSION(3,0,0)
/*!
 * For a parented/displayed widget, this will just return
 * gtk_widget_ensure_style(w). For a non-displayed widgets,
 * This will return a valid GtkStyle for that widget
 */
GtkStyle *
get_ensured_style (GtkWidget * w)
{
	GtkStyle  * style = NULL;
	GtkWidget * hidden_window = NULL;

	if (w->parent == NULL)
	{
		hidden_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_container_add (GTK_CONTAINER (hidden_window), w);
	}

	gtk_widget_ensure_style (w);
	gtk_widget_realize (w);

	style = gtk_widget_get_style (w);
	UT_ASSERT(style);

	if (hidden_window)
	{
		// now we destroy the hidden window
		gtk_container_remove (GTK_CONTAINER(hidden_window), w);
		gtk_widget_destroy (hidden_window);
	}

	return style;
}
#endif

/*!
 * Creates a new GdkDrawingArea with the proper colormap and visual
 * This returns a single-buffered widget.
 * DEPRECATED: please use gtk_drawing_area_new() directly and make your
 * implementation work with double-buffering in new code!
 */
GtkWidget *createDrawingArea ()
{
  GtkWidget *area;
  
  area = gtk_drawing_area_new ();
  gtk_widget_set_double_buffered(area, FALSE);

  return area;
}

/*!
 * This is a small message box for startup warnings and/or
 * errors.  Please do NOT use this for normal system execution
 * user messages; use the XAP_UnixDialog_MessageBox class for that.
 * We can't use that here because there is no parent frame, etc.
 */
void messageBoxOK(const char * message)
{
	GtkWidget * msg = gtk_message_dialog_new ( NULL,
						   GTK_DIALOG_MODAL,
						   GTK_MESSAGE_INFO,
						   GTK_BUTTONS_OK,
						   "%s", message ) ;

	gtk_window_set_title(GTK_WINDOW(msg), "AbiWord");
	gtk_window_set_role(GTK_WINDOW(msg), "message dialog");

	gtk_widget_show ( msg ) ;
	gtk_dialog_run ( GTK_DIALOG(msg) ) ;
	gtk_widget_destroy ( msg ) ;
}

/****************************************************************/
/****************************************************************/

GdkWindow * getRootWindow(GtkWidget * widget)
{
	UT_return_val_if_fail(widget, NULL);
	return gdk_get_default_root_window() ;
}


static void activate_button( GtkEntry * /*entry*/, gpointer user_data )
{
//    UT_DEBUGMSG(("activate_button() ud:%x\n", user_data ));

    GtkWidget* w = GTK_WIDGET(user_data);
    gtk_widget_activate(w);
}

/**
 * When the source widget gets the activate signal, sent activate to the button.
 * This allows GtkEntry widgets to explicitly close the dialog with the OK button
 * when the user presses return while leaving the default dialog action to be CANCEL.
 */
void abiSetActivateOnWidgetToActivateButton( GtkWidget* source, GtkWidget* button )
{
    g_signal_connect( G_OBJECT( source ), "activate",
                      G_CALLBACK(activate_button), button );
}

