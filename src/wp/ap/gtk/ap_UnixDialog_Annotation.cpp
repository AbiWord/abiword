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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_Annotation.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	if (!xml)
		return NULL;
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = glade_xml_get_widget(xml, "ap_UnixDialog_Annotation");
	m_entryTitle = glade_xml_get_widget(xml, "enTitle");
	m_entryAuthor = glade_xml_get_widget(xml, "enAuthor");
	m_textDescription = glade_xml_get_widget(xml, "tvDescription");
	
	// set the dialog title
	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Annotation_Title,s);
	abiDialogSetTitle(window, s.utf8_str());	
	
	// localize the strings in our dialog, and set some userdata for some widgets
	localizeLabel(glade_xml_get_widget(xml, "lbTitle"), pSS, AP_STRING_ID_DLG_Annotation_Title_LBL);
	localizeLabel(glade_xml_get_widget(xml, "lbAuthor"), pSS, AP_STRING_ID_DLG_Annotation_Author_LBL);
	localizeLabel(glade_xml_get_widget(xml, "lbDescription"), pSS, AP_STRING_ID_DLG_Annotation_Description_LBL);
	
	// now set the text in all the fields
	UT_UTF8String prop ( "" ) ;
	
	#define SET_ENTRY_TXT(name) \
	prop = get##name ().utf8_str() ; \
	if ( prop.size () > 0 ) { \
		gtk_entry_set_text (GTK_ENTRY(m_entry##name), prop.utf8_str() ) ; \
	}
	
	GtkWidget * wOK = glade_xml_get_widget(xml, "btOK");
	GtkWidget * wReplace = glade_xml_get_widget(xml, "btReplace");
	pSS->getValueUTF8(AP_STRING_ID_DLG_Annotation_Replace_LBL,s);
	gtk_button_set_label(GTK_BUTTON(wReplace),s.utf8_str()); 
	pSS->getValueUTF8(AP_STRING_ID_DLG_Annotation_OK_tooltip,s);
	gtk_widget_set_tooltip_text (wOK,s.utf8_str());
	pSS->getValueUTF8(AP_STRING_ID_DLG_Annotation_Replace_tooltip,s);
	gtk_widget_set_tooltip_text (wReplace,s.utf8_str());


	SET_ENTRY_TXT(Title)
	SET_ENTRY_TXT(Author)
	
	#undef SET_ENTRY_TXT
	
	prop = getDescription ().utf8_str() ;
	if ( prop.size () )
	{
		GtkTextBuffer * buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW(m_textDescription) ) ;
		gtk_text_buffer_set_text ( buffer, prop.utf8_str(), -1 ) ;
	}	
	
	return window;
}
