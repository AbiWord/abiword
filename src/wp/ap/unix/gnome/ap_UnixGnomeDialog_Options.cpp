/* AbiWord
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

#include <gnome.h>

#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "gr_UnixGraphics.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "xap_Prefs.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"

#include "ap_UnixDialog_Options.h"
#include "ap_UnixGnomeDialog_Options.h"

/*****************************************************************/

XAP_Dialog * AP_UnixGnomeDialog_Options::static_constructor(XAP_DialogFactory * pFactory,
                                                         XAP_Dialog_Id id)
{
    AP_UnixGnomeDialog_Options * p = new AP_UnixGnomeDialog_Options(pFactory,id);
    return p;
}

AP_UnixGnomeDialog_Options::AP_UnixGnomeDialog_Options(XAP_DialogFactory * pDlgFactory,
                                                 XAP_Dialog_Id id)
    : AP_UnixDialog_Options(pDlgFactory,id)
{
}

AP_UnixGnomeDialog_Options::~AP_UnixGnomeDialog_Options(void)
{
}

/*****************************************************************/

GtkWidget* AP_UnixGnomeDialog_Options::_constructWindow ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	XML_Char *unixstr = NULL;
	GtkWidget *windowOptions;

	GtkWidget *buttonDefaults;
	GtkWidget *buttonApply;
	GtkWidget *buttonOk;
	GtkWidget *buttonCancel;

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Options_OptionsTitle) );
	windowOptions = gnome_dialog_new (unixstr, NULL);

	gnome_dialog_append_button(GNOME_DIALOG(windowOptions), GNOME_STOCK_BUTTON_APPLY);
	gnome_dialog_append_button_with_pixmap(GNOME_DIALOG(windowOptions), 
			pSS->getValue(AP_STRING_ID_DLG_Options_Btn_Default), GNOME_STOCK_PIXMAP_REVERT);
	gnome_dialog_append_button(GNOME_DIALOG(windowOptions), GNOME_STOCK_BUTTON_OK);
	gnome_dialog_append_button(GNOME_DIALOG(windowOptions), GNOME_STOCK_BUTTON_CANCEL);

	gtk_object_set_data (GTK_OBJECT (windowOptions), "windowOptions", windowOptions);

	buttonApply = GTK_WIDGET (g_list_nth_data (GNOME_DIALOG (windowOptions)->buttons, 0) );
	buttonDefaults = GTK_WIDGET (g_list_nth_data (GNOME_DIALOG (windowOptions)->buttons, 1) );
	buttonOk = GTK_WIDGET (g_list_nth_data (GNOME_DIALOG (windowOptions)->buttons, 2) );
	buttonCancel = GTK_WIDGET (g_list_nth_data (GNOME_DIALOG (windowOptions)->buttons, 3) );

	// the catch-alls
	gtk_signal_connect(GTK_OBJECT(windowOptions),
			   "delete_event",
			   GTK_SIGNAL_FUNC(s_delete_clicked),
			   (gpointer) this);

	
	gtk_signal_connect_after(GTK_OBJECT(windowOptions),
				 "destroy",
				 NULL,
				 NULL);
	
	//////////////////////////////////////////////////////////////////////
	// the control buttons
	gtk_signal_connect(GTK_OBJECT(buttonOk),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_ok_clicked),
			   (gpointer) this);
    
	gtk_signal_connect(GTK_OBJECT(buttonCancel),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_cancel_clicked),
			   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(buttonDefaults),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_defaults_clicked),
			   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(buttonApply),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_apply_clicked),
			   (gpointer) this);
	
	
	gtk_signal_connect(GTK_OBJECT(windowOptions),
			   "close",
			   GTK_SIGNAL_FUNC(s_cancel_clicked),
			   (gpointer)this);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.
	
	m_windowMain = windowOptions;
	
	_constructWindowContents(GNOME_DIALOG (windowOptions)->vbox);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (windowOptions)->vbox), m_notebook, TRUE, TRUE, 10);
	
	m_buttonDefaults				= buttonDefaults;
	m_buttonApply					= buttonApply;
	m_buttonOK					= buttonOk;
	m_buttonCancel					= buttonCancel;

	// create the accelerators from &'s
	createLabelAccelerators(windowOptions);
	
	// create user data tControl -> stored in widgets 
	for ( int i = 0; i < id_last; i++ )
	  {
	    GtkWidget *w = _lookupWidget( (tControl)i );
	    if(!(w && GTK_IS_WIDGET(w)))
	       continue;

	    /* check to see if there is any data already stored there (note, will
	     * not work if 0's is stored in multiple places  */
	    UT_ASSERT( gtk_object_get_data(GTK_OBJECT(w), "tControl" ) == NULL);
	    
	    gtk_object_set_data( GTK_OBJECT(w), "tControl", (gpointer) i );
	  }

	setDefaultButton (GNOME_DIALOG (windowOptions), 3);
	return windowOptions;
}


