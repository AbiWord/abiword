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
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	_populateWindowData();

	switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow),
								 pFrame, this, BUTTON_CANCEL, false ) )
	{
		case BUTTON_OK:
			event_OK () ; break ;
		default:
			event_Cancel () ; break ;
	}

	_storeWindowData();

	abiDestroyWidget ( mainWindow ) ;
}

void AP_UnixDialog_Break::event_OK(void)
{
	m_answer = AP_Dialog_Break::a_OK;
}

void AP_UnixDialog_Break::event_Cancel(void)
{
	m_answer = AP_Dialog_Break::a_CANCEL;
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_Break::_constructWindow(void)
{

	GtkWidget * windowBreak;
	GtkWidget * vboxMain;
	GtkWidget * tableInsert;
	GtkWidget * labelInsert;
	GSList * tableInsert_group = NULL;
	GtkWidget * radiobuttonPageBreak;
	GtkWidget * radiobuttonNextPage;
	GtkWidget * radiobuttonContinuous;
	GtkWidget * radiobuttonColumnBreak;
	GtkWidget * radiobuttonEvenPage;
	GtkWidget * radiobuttonOddPage;
	GtkWidget * labelSectionBreaks;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions


	windowBreak = abiDialogNew ( "break dialog", TRUE, pSS->getValueUTF8(AP_STRING_ID_DLG_Break_BreakTitle).c_str());
	
	vboxMain = GTK_DIALOG(windowBreak)->vbox ;
	gtk_container_set_border_width (GTK_CONTAINER (vboxMain), 10);

	tableInsert = gtk_table_new (6, 2, FALSE);
	gtk_widget_show (tableInsert);
	gtk_box_pack_start (GTK_BOX (vboxMain), tableInsert, FALSE, FALSE, 0);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_Break_Insert).c_str());
	labelInsert = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelInsert);
	gtk_table_attach (GTK_TABLE (tableInsert), labelInsert, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_widget_set_usize (labelInsert, 17, -1);
	gtk_label_set_justify (GTK_LABEL (labelInsert), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelInsert), 0, 0.5);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_Break_PageBreak).c_str());	
	radiobuttonPageBreak = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonPageBreak));
	g_object_set_data (G_OBJECT (radiobuttonPageBreak), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_PAGE));
	gtk_widget_show (radiobuttonPageBreak);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonPageBreak, 0, 1, 1, 2,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_Break_NextPage).c_str());	
	radiobuttonNextPage = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonNextPage));
	g_object_set_data (G_OBJECT (radiobuttonNextPage), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_NEXTPAGE));
	gtk_widget_show (radiobuttonNextPage);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonNextPage, 0, 1, 4, 5,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_Break_Continuous).c_str());	
	radiobuttonContinuous = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonContinuous));
	g_object_set_data (G_OBJECT (radiobuttonContinuous), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_CONTINUOUS));
	gtk_widget_show (radiobuttonContinuous);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonContinuous, 0, 1, 5, 6,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_Break_ColumnBreak).c_str());	
	radiobuttonColumnBreak = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonColumnBreak));
	g_object_set_data (G_OBJECT (radiobuttonColumnBreak), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_COLUMN));
	gtk_widget_show (radiobuttonColumnBreak);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonColumnBreak, 1, 2, 1, 2,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_Break_EvenPage).c_str());	
	radiobuttonEvenPage = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonEvenPage));
	g_object_set_data (G_OBJECT (radiobuttonEvenPage), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_EVENPAGE));
	gtk_widget_show (radiobuttonEvenPage);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonEvenPage, 1, 2, 4, 5,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_Break_OddPage).c_str());	
	radiobuttonOddPage = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonOddPage));
	g_object_set_data (G_OBJECT (radiobuttonOddPage), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_ODDPAGE));
	gtk_widget_show (radiobuttonOddPage);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonOddPage, 1, 2, 5, 6,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_Break_SectionBreaks).c_str());	
	labelSectionBreaks = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelSectionBreaks);
	gtk_table_attach (GTK_TABLE (tableInsert), labelSectionBreaks, 0, 1, 3, 4,
					  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 4);
	gtk_widget_set_usize (labelSectionBreaks, 76, -1);
	gtk_label_set_justify (GTK_LABEL (labelSectionBreaks), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelSectionBreaks), 0, 0.5);

#if 0
	hseparator9 = gtk_hseparator_new ();
	gtk_widget_show (hseparator9);
	gtk_table_attach (GTK_TABLE (tableInsert), hseparator9, 0, 2, 6, 7,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 6);
#endif

	GtkWidget * hseparator10;
	hseparator10 = gtk_hseparator_new ();
	gtk_widget_show (hseparator10);
	gtk_table_attach (GTK_TABLE (tableInsert), hseparator10, 0, 2, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 6);
	
	abiAddStockButton ( GTK_DIALOG(windowBreak), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;
	abiAddStockButton ( GTK_DIALOG(windowBreak), GTK_STOCK_OK, BUTTON_OK ) ;

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowBreak;
	m_radioGroup = tableInsert_group;
	
	return windowBreak;
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


