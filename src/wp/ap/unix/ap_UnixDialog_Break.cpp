/* AbiWord
 * Copyright (C) 1998-2002 AbiSource, Inc.
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
#include <glade/glade.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "ap_UnixApp.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Break.h"
#include "ap_UnixDialog_Break.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Break::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_UnixDialog_Break * p = new AP_UnixDialog_Break(pFactory,id);
	return p;
}

AP_UnixDialog_Break::AP_UnixDialog_Break(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Break(pDlgFactory,id)
{
	m_windowMain = NULL;
	m_radioGroup = NULL;
}

AP_UnixDialog_Break::~AP_UnixDialog_Break(void)
{
}

/*****************************************************************/
/*****************************************************************/

void AP_UnixDialog_Break::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
	// Build the window's widgets and arrange them
	m_windowMain = _constructWindow();
	UT_return_if_fail(m_windowMain);

	_populateWindowData();

	switch ( abiRunModalDialog ( GTK_DIALOG(m_windowMain),
								 pFrame, this, GTK_RESPONSE_CANCEL, false ) )
	{
		case GTK_RESPONSE_OK:
			m_answer = AP_Dialog_Break::a_OK;
			break;
		default:
			m_answer = AP_Dialog_Break::a_CANCEL;
			break;
	}

	_storeWindowData();
	
	abiDestroyWidget ( m_windowMain ) ;
}

/*****************************************************************/
GtkWidget * AP_UnixDialog_Break::_constructWindow(void)
{
	GtkWidget * window;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our glade file is located
	AP_UnixApp * unixApp = static_cast<AP_UnixApp*>(m_pApp);
	UT_String glade_path( unixApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_Break.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = glade_xml_get_widget(xml, "ap_UnixDialog_Break");
	m_radioGroup = gtk_radio_button_group (GTK_RADIO_BUTTON ( glade_xml_get_widget(xml, "rbPageBreak") ));
	
	abiDialogSetTitle(window, pSS->getValueUTF8(AP_STRING_ID_DLG_Break_BreakTitle).c_str());
	
	// localize the strings in our dialog, and set tags for some widgets
	
	localizeLabelMarkup(pSS, AP_STRING_ID_DLG_Break_Insert, glade_xml_get_widget(xml, "lbInsertBreak"), 
	  gtk_label_get_label (GTK_LABEL(glade_xml_get_widget(xml, "lbInsertBreak")))
	);
	
	localizeButton(pSS, AP_STRING_ID_DLG_Break_PageBreak, glade_xml_get_widget(xml, "rbPageBreak"));
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbPageBreak")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_PAGE));

	localizeButton(pSS, AP_STRING_ID_DLG_Break_ColumnBreak, glade_xml_get_widget(xml, "rbColumnBreak"));
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbColumnBreak")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_COLUMN));

	localizeLabelMarkup(pSS, AP_STRING_ID_DLG_Break_SectionBreaks, glade_xml_get_widget(xml, "lbInsertSectionBreak"),
	  gtk_label_get_label (GTK_LABEL(glade_xml_get_widget(xml, "lbInsertSectionBreak")))
	);
	
	localizeButton(pSS, AP_STRING_ID_DLG_Break_NextPage, glade_xml_get_widget(xml, "rbNextPage"));
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbNextPage")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_NEXTPAGE));

	localizeButton(pSS, AP_STRING_ID_DLG_Break_Continuous, glade_xml_get_widget(xml, "rbContinuous"));
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbContinuous")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_CONTINUOUS));

	localizeButton(pSS, AP_STRING_ID_DLG_Break_EvenPage, glade_xml_get_widget(xml, "rbEvenPage"));
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbEvenPage")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_EVENPAGE));

	localizeButton(pSS, AP_STRING_ID_DLG_Break_OddPage, glade_xml_get_widget(xml, "rbOddPage"));
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbOddPage")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_ODDPAGE));

	return window;
}

void AP_UnixDialog_Break::_populateWindowData(void)
{
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.

	GtkWidget * widget = _findRadioByID(m_break);
	UT_ASSERT(widget);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
}

void AP_UnixDialog_Break::_storeWindowData(void)
{
	m_break = _getActiveRadioItem();
}

// TODO if this function is useful elsewhere, move it to Unix dialog
// TODO helpers and standardize on a user-data tag for WIDGET_ID_TAG_KEY
GtkWidget * AP_UnixDialog_Break::_findRadioByID(AP_Dialog_Break::breakType b)
{
	UT_ASSERT(m_radioGroup);
	for (GSList * item = m_radioGroup ; item ; item = item->next)
	{
		if (GPOINTER_TO_INT(g_object_get_data(G_OBJECT(item->data), WIDGET_ID_TAG_KEY)) ==
			(gint) b)
			return (GtkWidget *) item->data;
	}

	return NULL;
}

AP_Dialog_Break::breakType AP_UnixDialog_Break::_getActiveRadioItem(void)
{
	UT_ASSERT(m_radioGroup);
	for (GSList * item = m_radioGroup ; item ; item = item->next)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(item->data)))
		{
			return (AP_Dialog_Break::breakType)
				GPOINTER_TO_INT(g_object_get_data(G_OBJECT(item->data), WIDGET_ID_TAG_KEY));
		}
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return AP_Dialog_Break::b_PAGE;
}
