/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2008 Ryan Pavlik <abiryan@ryand.net>
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
#include "ap_Dialog_EditStyle.h"
#include "ap_UnixDialog_EditStyle.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"
#define CUSTOM_RESPONSE_INSERT 1

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_EditStyle::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_UnixDialog_EditStyle * p = new AP_UnixDialog_EditStyle(pFactory,id);
	return p;
}

AP_UnixDialog_EditStyle::AP_UnixDialog_EditStyle(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_EditStyle(pDlgFactory,id),
	m_windowMain(NULL),
	m_wPropList(NULL),
	m_wNameRenderer(NULL),
	m_wValueRenderer(NULL),
	m_wModel(NULL),
	m_wPropListContainer(NULL)

{
}

AP_UnixDialog_EditStyle::~AP_UnixDialog_EditStyle(void)
{
}

/*****************************************************************/
/*****************************************************************/

void AP_UnixDialog_EditStyle::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
    // Build the dialog's window
	m_windowMain = _constructWindow();
	UT_return_if_fail(m_windowMain);

	_populateWindowData();

	switch ( abiRunModalDialog ( GTK_DIALOG(m_windowMain),
								 pFrame, this, CUSTOM_RESPONSE_INSERT, false ) )
	{
		case CUSTOM_RESPONSE_INSERT:
			m_answer = AP_Dialog_EditStyle::a_OK;
			break;
		default:
			m_answer = AP_Dialog_EditStyle::a_CANCEL;
			break;
	}

	_storeWindowData();
	
	abiDestroyWidget ( m_windowMain ) ;
}

/*****************************************************************/
GtkWidget * AP_UnixDialog_EditStyle::_constructWindow(void)
{
	GtkWidget * window;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_EditStyle.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	if (!xml)
		return NULL;
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = glade_xml_get_widget(xml, "ap_UnixDialog_EditStyle");
	//m_radioGroup = gtk_radio_button_get_group (GTK_RADIO_BUTTON ( glade_xml_get_widget(xml, "rbPageBreak") ));
	m_wPropListContainer = glade_xml_get_widget(xml, "TreeViewContainer");
	
	// set the dialog title
	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_EditStyle_Title,s);
	abiDialogSetTitle(window, s.utf8_str());
	
	/*
	// localize the strings in our dialog, and set tags for some widgets
	
	localizeLabelMarkup(glade_xml_get_widget(xml, "lbInsertBreak"), pSS, AP_STRING_ID_DLG_Break_Insert);
	
	localizeButton(glade_xml_get_widget(xml, "rbPageBreak"), pSS, AP_STRING_ID_DLG_Break_PageBreak);
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbPageBreak")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_PAGE));

	localizeButton(glade_xml_get_widget(xml, "rbColumnBreak"), pSS, AP_STRING_ID_DLG_Break_ColumnBreak);
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbColumnBreak")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_COLUMN));

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbInsertSectionBreak"), pSS, AP_STRING_ID_DLG_Break_SectionBreaks_Capital);
	
	localizeButton(glade_xml_get_widget(xml, "rbNextPage"), pSS, AP_STRING_ID_DLG_Break_NextPage);
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbNextPage")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_NEXTPAGE));

	localizeButton(glade_xml_get_widget(xml, "rbContinuous"), pSS, AP_STRING_ID_DLG_Break_Continuous);
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbContinuous")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_CONTINUOUS));

	localizeButton(glade_xml_get_widget(xml, "rbEvenPage"), pSS, AP_STRING_ID_DLG_Break_EvenPage);
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbEvenPage")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_EVENPAGE));

	localizeButton(glade_xml_get_widget(xml, "rbOddPage"), pSS, AP_STRING_ID_DLG_Break_OddPage);
	g_object_set_data (G_OBJECT (glade_xml_get_widget(xml, "rbOddPage")), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_ODDPAGE));
	localizeButtonUnderline(glade_xml_get_widget(xml, "btInsert"), pSS, AP_STRING_ID_DLG_InsertButton);
	*/
	return window;
}

void AP_UnixDialog_EditStyle::_populateWindowData(void)
{
	GtkTreeIter iter;
	GtkTreeSelection *sel;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UTF8String s;
	
	
	_deconstructStyle ();
	
	m_wModel = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
	
	unsigned int i=0;
	for (i=0; i< m_vPropertyID.size(); i++)
	{
		// use the property id as an offset in the string table - yikes!
		pSS->getValueUTF8(AP_STRING_ID_DLG_EditStyle_Prop_P_TEXT_ALIGN +m_vPropertyID[i],s);
		gtk_list_store_append (m_wModel, &iter);
		gtk_list_store_set (m_wModel, &iter, 0, g_strdup(s.utf8_str()), 1, m_vPropertyValues[i].c_str(), 2, i, -1);
	}
	
	// create a new treeview
	m_wPropList = gtk_tree_view_new_with_model (GTK_TREE_MODEL (m_wModel));
	g_object_unref (G_OBJECT (m_wModel));
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (m_wPropList), true);
	
	// get the current selection
	sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (m_wPropList));
	gtk_tree_selection_set_mode (sel, GTK_SELECTION_SINGLE);
//	gtk_tree_selection_set_select_function (sel, tree_select_filter_common,
//											NULL, NULL);
	

	m_wNameRenderer = gtk_cell_renderer_text_new ();
	pSS->getValueUTF8(AP_STRING_ID_DLG_EditStyle_PropName,s);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (m_wPropList),
												 -1, s.utf8_str(),
												 m_wNameRenderer, "text", 0, NULL); 
	
	m_wValueRenderer = gtk_cell_renderer_text_new ();
	g_object_set(m_wValueRenderer, "editable", TRUE, NULL);
	
	pSS->getValueUTF8(AP_STRING_ID_DLG_EditStyle_PropValue,s);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (m_wPropList),
												 -1, s.utf8_str(),
												 m_wValueRenderer, "text", 0, NULL); 	

	gtk_container_add (GTK_CONTAINER (m_wPropListContainer), m_wPropList);

	/*
	g_signal_connect_after(G_OBJECT(m_wPropList),
						   "cursor-changed",
						   G_CALLBACK(s_types_clicked),
						   static_cast<gpointer>(this));

	g_signal_connect_after(G_OBJECT(m_wPropList),
						   "row-activated",
						   G_CALLBACK(s_types_dblclicked),
						   static_cast<gpointer>(this));
	*/
	gtk_widget_show_all(m_wPropList);
	//setStyleTreeChanged(false);
	
}

void AP_UnixDialog_EditStyle::_storeWindowData(void)
{
	//m_break = _getActiveRadioItem();
}
/*
// TODO if this function is useful elsewhere, move it to Unix dialog
// TODO helpers and standardize on a user-data tag for WIDGET_ID_TAG_KEY
GtkWidget * AP_UnixDialog_EditStyle::_findRadioByID(AP_Dialog_EditStyle::breakType b)
{
	UT_ASSERT(m_radioGroup);
	for (GSList * item = m_radioGroup ; item ; item = item->next)
	{
		if (GPOINTER_TO_INT(g_object_get_data(G_OBJECT(item->data), WIDGET_ID_TAG_KEY)) ==
			static_cast<gint>(b))
			return static_cast<GtkWidget *>(item->data);
	}

	return NULL;
}

AP_Dialog_EditStyle::breakType AP_UnixDialog_EditStyle::_getActiveRadioItem(void)
{
	UT_ASSERT(m_radioGroup);
	for (GSList * item = m_radioGroup ; item ; item = item->next)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(item->data)))
		{
			return (AP_Dialog_EditStyle::breakType)
				GPOINTER_TO_INT(g_object_get_data(G_OBJECT(item->data), WIDGET_ID_TAG_KEY));
		}
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return AP_Dialog_EditStyle::b_PAGE;
}
*/
