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

#include <stdlib.h>
#include <glade/glade.h>

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
#include "ap_Dialog_InsertTable.h"
#include "ap_UnixDialog_InsertTable.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_InsertTable::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_UnixDialog_InsertTable * p = new AP_UnixDialog_InsertTable(pFactory,id);
	return p;
}

AP_UnixDialog_InsertTable::AP_UnixDialog_InsertTable(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_InsertTable(pDlgFactory,id)
{
	m_windowMain = NULL;
	m_pColSpin = NULL;
	m_pRowSpin = NULL;
	m_pColWidthSpin = NULL;
	m_radioGroup = NULL;
}

AP_UnixDialog_InsertTable::~AP_UnixDialog_InsertTable(void)
{
}

void AP_UnixDialog_InsertTable::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * m_windowMain = _constructWindow();
	UT_return_if_fail(m_windowMain);

	// Populate the window's data items
	_populateWindowData();

	switch ( abiRunModalDialog ( GTK_DIALOG(m_windowMain),
								 pFrame, this, GTK_RESPONSE_CANCEL, false ) )
	{
		case GTK_RESPONSE_OK:
			m_answer = AP_Dialog_InsertTable::a_OK;
			break;
		default:
			m_answer = AP_Dialog_InsertTable::a_CANCEL;
			break;
	}

	_storeWindowData();

	abiDestroyWidget ( m_windowMain ) ;
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_InsertTable::_constructWindow(void)
{
	GtkWidget * window;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_InsertTable.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );

	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = glade_xml_get_widget(xml, "ap_UnixDialog_InsertTable");
	m_radioGroup = gtk_radio_button_group (GTK_RADIO_BUTTON ( glade_xml_get_widget(xml, "rbAutoColSize") ));
	m_pColSpin = glade_xml_get_widget(xml, "sbNumCols");
	m_pRowSpin = glade_xml_get_widget(xml, "sbNumRows");
	m_pColWidthSpin = glade_xml_get_widget(xml, "sbColSize");
	
	// set the dialog title
	abiDialogSetTitle(window, pSS->getValueUTF8(AP_STRING_ID_DLG_InsertTable_TableTitle).c_str());
	
	// localize the strings in our dialog, and set tags for some widgets
	
	localizeLabelMarkup(glade_xml_get_widget(xml, "lbTableSize"), pSS, AP_STRING_ID_DLG_InsertTable_TableSize);
	
	localizeLabel(glade_xml_get_widget(xml, "lbNumCols"), pSS, AP_STRING_ID_DLG_InsertTable_NumCols);
	
	localizeLabel(glade_xml_get_widget(xml, "lbNumRows"), pSS, AP_STRING_ID_DLG_InsertTable_NumRows);
	
	localizeLabelMarkup(glade_xml_get_widget(xml, "lbAutoFit"), pSS, AP_STRING_ID_DLG_InsertTable_AutoFit);
	
	localizeButton(glade_xml_get_widget(xml, "rbAutoColSize"), pSS, AP_STRING_ID_DLG_InsertTable_AutoColSize);
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbAutoColSize")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_AUTOSIZE));	
	
	localizeButton(glade_xml_get_widget(xml, "rbFixedColSize"), pSS, AP_STRING_ID_DLG_InsertTable_FixedColSize);
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbFixedColSize")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_FIXEDSIZE));

	return window;
}

void AP_UnixDialog_InsertTable::_populateWindowData(void)
{
	// We're (still) a stateless dialog, so there are 
	// no member variables to setyet
}

void AP_UnixDialog_InsertTable::_storeWindowData(void)
{
	m_columnType = _getActiveRadioItem();
	m_numRows = static_cast<UT_uint32>(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pRowSpin)));
	m_numCols = static_cast<UT_uint32>(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pColSpin)));
	m_columnWidth = static_cast<float>(gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(m_pColWidthSpin)));
}

AP_Dialog_InsertTable::columnType AP_UnixDialog_InsertTable::_getActiveRadioItem(void)
{
	UT_ASSERT(m_radioGroup);
	for (GSList * item = m_radioGroup ; item ; item = item->next)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(item->data)))
		{
			return (AP_Dialog_InsertTable::columnType)
				GPOINTER_TO_INT(g_object_get_data(G_OBJECT(item->data), WIDGET_ID_TAG_KEY));
		}
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return AP_Dialog_InsertTable::b_AUTOSIZE;
}

