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
#include "ap_UnixDialog_MetaData.h"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_MetaData::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_MetaData * p = new AP_UnixDialog_MetaData(pFactory,id);
	return p;
}

AP_UnixDialog_MetaData::AP_UnixDialog_MetaData(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_MetaData(pDlgFactory,id)
{
}

AP_UnixDialog_MetaData::~AP_UnixDialog_MetaData(void)
{
}

void AP_UnixDialog_MetaData::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
	// Build the window's widgets and arrange them
	m_windowMain = _constructWindow();
	UT_return_if_fail(m_windowMain);
	
	switch(abiRunModalDialog(GTK_DIALOG(m_windowMain), pFrame, this,
				   GTK_RESPONSE_CANCEL, false))
	{
		case GTK_RESPONSE_OK:
			eventOK();
			break;
		default:
			eventCancel();
			break ;
	}
	
	abiDestroyWidget(m_windowMain);
}

void AP_UnixDialog_MetaData::eventCancel ()
{
	setAnswer ( AP_Dialog_MetaData::a_CANCEL ) ;
}

#define GRAB_ENTRY_TEXT(name) txt = gtk_entry_get_text(GTK_ENTRY(m_entry##name)) ; \
if( txt ) \
set##name ( txt )

void AP_UnixDialog_MetaData::eventOK ()
{
	setAnswer ( AP_Dialog_MetaData::a_OK ) ;
	
	// TODO: gather data
	const char * txt = NULL ;
	
	GRAB_ENTRY_TEXT(Title);
	GRAB_ENTRY_TEXT(Subject);
	GRAB_ENTRY_TEXT(Author);
	GRAB_ENTRY_TEXT(Publisher);  
	GRAB_ENTRY_TEXT(CoAuthor);
	GRAB_ENTRY_TEXT(Category);
	GRAB_ENTRY_TEXT(Keywords);
	GRAB_ENTRY_TEXT(Languages);
	GRAB_ENTRY_TEXT(Source);
	GRAB_ENTRY_TEXT(Relation);
	GRAB_ENTRY_TEXT(Coverage);
	GRAB_ENTRY_TEXT(Rights);
	
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

GtkWidget * AP_UnixDialog_MetaData::_constructWindow ()
{
	GtkWidget * window;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_MetaData.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	if (!xml)
		return NULL;
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = glade_xml_get_widget(xml, "ap_UnixDialog_MetaData");
	m_entryTitle = glade_xml_get_widget(xml, "enTitle");
	m_entrySubject = glade_xml_get_widget(xml, "enSubject");
	m_entryAuthor = glade_xml_get_widget(xml, "enAuthor");
	m_entryPublisher = glade_xml_get_widget(xml, "enPublisher");
	m_entryCoAuthor = glade_xml_get_widget(xml, "enContributors");
	m_entryCategory = glade_xml_get_widget(xml, "enCategory");
	m_entryKeywords = glade_xml_get_widget(xml, "enKeywords");
	m_entryLanguages = glade_xml_get_widget(xml, "enLanguages");
	m_textDescription = glade_xml_get_widget(xml, "tvDescription");
	m_entrySource = glade_xml_get_widget(xml, "enSource");
	m_entryRelation = glade_xml_get_widget(xml, "enRelation");
	m_entryCoverage = glade_xml_get_widget(xml, "enCoverage");
	m_entryRights = glade_xml_get_widget(xml, "enRights");
	
	// set the dialog title
	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Title,s);
	abiDialogSetTitle(window, s.utf8_str());	
	
	// localize the strings in our dialog, and set some userdata for some widgets
	localizeLabel(glade_xml_get_widget(xml, "lbTitle"), pSS, AP_STRING_ID_DLG_MetaData_Title_LBL);
	localizeLabel(glade_xml_get_widget(xml, "lbSubject"), pSS, AP_STRING_ID_DLG_MetaData_Subject_LBL);
	localizeLabel(glade_xml_get_widget(xml, "lbAuthor"), pSS, AP_STRING_ID_DLG_MetaData_Author_LBL);
	localizeLabel(glade_xml_get_widget(xml, "lbPublisher"), pSS, AP_STRING_ID_DLG_MetaData_Publisher_LBL);
	localizeLabel(glade_xml_get_widget(xml, "lbContributors"), pSS, AP_STRING_ID_DLG_MetaData_CoAuthor_LBL);

	localizeLabel(glade_xml_get_widget(xml, "lbCategory"), pSS, AP_STRING_ID_DLG_MetaData_Category_LBL);
	localizeLabel(glade_xml_get_widget(xml, "lbKeywords"), pSS, AP_STRING_ID_DLG_MetaData_Keywords_LBL);
	localizeLabel(glade_xml_get_widget(xml, "lbLanguages"), pSS, AP_STRING_ID_DLG_MetaData_Languages_LBL);
	localizeLabel(glade_xml_get_widget(xml, "lbDescription"), pSS, AP_STRING_ID_DLG_MetaData_Description_LBL);

	localizeLabel(glade_xml_get_widget(xml, "lbSource"), pSS, AP_STRING_ID_DLG_MetaData_Source_LBL);
	localizeLabel(glade_xml_get_widget(xml, "lbRelation"), pSS, AP_STRING_ID_DLG_MetaData_Relation_LBL);
	localizeLabel(glade_xml_get_widget(xml, "lbCoverage"), pSS, AP_STRING_ID_DLG_MetaData_Coverage_LBL);
	localizeLabel(glade_xml_get_widget(xml, "lbRights"), pSS, AP_STRING_ID_DLG_MetaData_Rights_LBL);
	
	localizeLabel(glade_xml_get_widget(xml, "lbGeneral_Tab"), pSS, AP_STRING_ID_DLG_MetaData_TAB_General);
	localizeLabel(glade_xml_get_widget(xml, "lbSummary_Tab"), pSS, AP_STRING_ID_DLG_MetaData_TAB_Summary);
	localizeLabel(glade_xml_get_widget(xml, "lbPermissions_Tab"), pSS, AP_STRING_ID_DLG_MetaData_TAB_Permission);
	
	// now set the text in all the fields
	UT_UTF8String prop ( "" ) ;
	
	#define SET_ENTRY_TXT(name) \
	prop = get##name ().utf8_str() ; \
	if ( prop.size () > 0 ) { \
		gtk_entry_set_text (GTK_ENTRY(m_entry##name), prop.utf8_str() ) ; \
	}
	
	SET_ENTRY_TXT(Title)
	SET_ENTRY_TXT(Subject)
	SET_ENTRY_TXT(Author)
	SET_ENTRY_TXT(Publisher)
	SET_ENTRY_TXT(CoAuthor)
	SET_ENTRY_TXT(Category)
	SET_ENTRY_TXT(Keywords)
	SET_ENTRY_TXT(Languages)
	SET_ENTRY_TXT(Source)
	SET_ENTRY_TXT(Relation)
	SET_ENTRY_TXT(Coverage)
	SET_ENTRY_TXT(Rights)
	
	#undef SET_ENTRY_TXT
	
	prop = getDescription ().utf8_str() ;
	if ( prop.size () )
	{
		GtkTextBuffer * buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW(m_textDescription) ) ;
		gtk_text_buffer_set_text ( buffer, prop.utf8_str(), -1 ) ;
	}	
	
	return window;
}
