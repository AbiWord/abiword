/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz
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

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_Annotation.h"

/*****************************************************************/

static gboolean s_focus_out(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	UT_UNUSED(event);
	UT_UNUSED(user_data);

	gtk_editable_select_region(GTK_EDITABLE(widget), 0, 0);
	return FALSE;
}

XAP_Dialog * AP_UnixDialog_Annotation::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_Annotation * p = new AP_UnixDialog_Annotation(pFactory,id);
	return p;
}

AP_UnixDialog_Annotation::AP_UnixDialog_Annotation(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Annotation(pDlgFactory,id)
{
}

AP_UnixDialog_Annotation::~AP_UnixDialog_Annotation(void)
{
}

void AP_UnixDialog_Annotation::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
	// Build the window's widgets and arrange them
	m_windowMain = _constructWindow();
	UT_return_if_fail(m_windowMain);
	
	switch(abiRunModalDialog(GTK_DIALOG(m_windowMain), pFrame, this,
				   GTK_RESPONSE_CANCEL, false))
	{
		case GTK_RESPONSE_APPLY:
			eventApply();
			break;

		case GTK_RESPONSE_OK:
			eventOK();
			break;
		default:
			eventCancel();
			break ;
	}
	
	abiDestroyWidget(m_windowMain);
}

void AP_UnixDialog_Annotation::eventCancel ()
{
	setAnswer ( AP_Dialog_Annotation::a_CANCEL ) ;
}

#define GRAB_ENTRY_TEXT(name) txt = gtk_entry_get_text(GTK_ENTRY(m_entry##name)) ; \
if( txt ) \
set##name ( txt )

void AP_UnixDialog_Annotation::eventOK ()
{
	setAnswer ( AP_Dialog_Annotation::a_OK ) ;
	
	// TODO: gather data
	const char * txt = NULL ;
	
	GRAB_ENTRY_TEXT(Title);
	GRAB_ENTRY_TEXT(Author);
	
	GtkTextIter start, end;
	
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_textDescription));
	gtk_text_buffer_get_iter_at_offset ( buffer, &start, 0 );
	gtk_text_buffer_get_iter_at_offset ( buffer, &end, -1 );
	
	char * editable_txt = gtk_text_buffer_get_text ( buffer, &start, &end, FALSE );
	
	if (editable_txt && strlen(editable_txt))
	{
		setDescription ( editable_txt ) ;
		g_free(editable_txt);
	}
}

void AP_UnixDialog_Annotation::eventApply ()
{
	setAnswer ( AP_Dialog_Annotation::a_APPLY ) ;
	
	// TODO: gather data
	const char * txt = NULL ;
	
	GRAB_ENTRY_TEXT(Title);
	GRAB_ENTRY_TEXT(Author);
	
	GtkTextIter start, end;
	
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_textDescription));
	gtk_text_buffer_get_iter_at_offset ( buffer, &start, 0 );
	gtk_text_buffer_get_iter_at_offset ( buffer, &end, -1 );
	
	char * editable_txt = gtk_text_buffer_get_text ( buffer, &start, &end, FALSE );
	
	if (editable_txt && strlen(editable_txt))
	{
		setDescription ( editable_txt ) ;
		g_free(editable_txt);
	}
}

#undef GRAB_ENTRY_TEXT

GtkWidget * AP_UnixDialog_Annotation::_constructWindow ()
{
	GtkWidget * window;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_Annotation.ui");
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Annotation"));
	m_entryTitle = GTK_WIDGET(gtk_builder_get_object(builder, "enTitle"));
	m_entryAuthor = GTK_WIDGET(gtk_builder_get_object(builder, "enAuthor"));
	m_textDescription = GTK_WIDGET(gtk_builder_get_object(builder, "tvDescription"));
	
	// set the dialog title
	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Annotation_Title,s);
	abiDialogSetTitle(window, "%s", s.c_str());
	
	// localize the strings in our dialog, and set some userdata for some widgets
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbTitle")), pSS, AP_STRING_ID_DLG_Annotation_Title_LBL);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbAuthor")), pSS, AP_STRING_ID_DLG_Annotation_Author_LBL);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbDescription")), pSS, AP_STRING_ID_DLG_Annotation_Description_LBL);
	
	// signals
	g_signal_connect(G_OBJECT(m_entryTitle), "focus-out-event", G_CALLBACK(s_focus_out), static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_entryAuthor), "focus-out-event", G_CALLBACK(s_focus_out), static_cast<gpointer>(this));
	
	// now set the text in all the fields
	std::string prop;
	
	#define SET_ENTRY_TXT(name) \
	prop = get##name () ;                      \
	if ( !prop.empty() ) {                          \
		gtk_entry_set_text (GTK_ENTRY(m_entry##name), prop.c_str() ) ; \
	}
	
	GtkWidget * wReplace = GTK_WIDGET(gtk_builder_get_object(builder, "btReplace"));
	pSS->getValueUTF8(AP_STRING_ID_DLG_Annotation_Replace_LBL,s);
	gtk_button_set_label(GTK_BUTTON(wReplace),s.c_str()); 
	GtkWidget * wOK = GTK_WIDGET(gtk_builder_get_object(builder, "btOK"));
	pSS->getValueUTF8(AP_STRING_ID_DLG_Annotation_OK_tooltip,s);
	gtk_widget_set_tooltip_text (wOK,s.c_str());
	pSS->getValueUTF8(AP_STRING_ID_DLG_Annotation_Replace_tooltip,s);
	gtk_widget_set_tooltip_text (wReplace,s.c_str());


	SET_ENTRY_TXT(Title)
	SET_ENTRY_TXT(Author)
	
	#undef SET_ENTRY_TXT
	
	prop = getDescription ();
	if ( !prop.empty() )
	{
		GtkTextBuffer * buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW(m_textDescription) ) ;
		gtk_text_buffer_set_text ( buffer, prop.c_str(), -1 ) ;
	}	

	g_object_unref(G_OBJECT(builder));
	
	return window;
}
