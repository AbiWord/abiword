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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

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
}

AP_UnixDialog_InsertTable::~AP_UnixDialog_InsertTable(void)
{
}

void AP_UnixDialog_InsertTable::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	// Populate the window's data items
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


void AP_UnixDialog_InsertTable::event_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_InsertTable::a_OK;
}

void AP_UnixDialog_InsertTable::event_Cancel(void)
{
	m_answer = AP_Dialog_InsertTable::a_CANCEL;
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_InsertTable::_constructWindow(void)
{
	GtkWidget * vboxMain;
	GtkWidget * windowInsertTable;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	windowInsertTable = abiDialogNew ( "insert table dialog", TRUE, pSS->getValueUTF8(AP_STRING_ID_DLG_InsertTable_TableTitle).c_str());
	
	vboxMain = GTK_DIALOG(windowInsertTable)->vbox ;
	gtk_container_set_border_width (GTK_CONTAINER (vboxMain), 10);	
	gtk_container_add (GTK_CONTAINER (vboxMain), _constructWindowContents());

	abiAddStockButton ( GTK_DIALOG(windowInsertTable), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;
	abiAddStockButton ( GTK_DIALOG(windowInsertTable), GTK_STOCK_OK, BUTTON_OK ) ;

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowInsertTable;

	return windowInsertTable;
}

GtkWidget * AP_UnixDialog_InsertTable::_constructWindowContents(void)
{
	GtkWidget * tableInsert;
	GtkWidget * labelInsert;
	GtkWidget * labelNumRows;
	GtkWidget * spinNumRows;
	GtkAdjustment * spinRangeRows;
	GtkWidget * labelNumCols;
	GtkWidget * spinNumCols;
	GtkAdjustment * spinRangeCols;
	GtkWidget * hseparator1;
	GtkWidget * labelAutoFit;
	GSList * tableInsert_group = NULL;
	GtkWidget * radiobuttonAutoColSize;
	GtkWidget * radiobuttonFixedColSize;
	GtkWidget * spinColWidth;
	GtkAdjustment * spinRangeColWidth;
	GtkWidget * labelUnits;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	tableInsert = gtk_table_new (7, 3, FALSE);
	gtk_widget_show (tableInsert);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_InsertTable_TableSize).c_str());
	labelInsert = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelInsert);
	gtk_table_attach (GTK_TABLE (tableInsert), labelInsert, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_widget_set_usize (labelInsert, 17, -1);
	gtk_label_set_justify (GTK_LABEL (labelInsert), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelInsert), 0, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_InsertTable_NumCols).c_str());
	labelNumCols = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelNumCols);
	gtk_table_attach (GTK_TABLE (tableInsert), labelNumCols, 0, 1, 1, 2,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	gtk_misc_set_alignment (GTK_MISC (labelNumCols), 0, 0.5);

	spinRangeCols = (GtkAdjustment *) gtk_adjustment_new(getNumCols(), 1, 9999, 1, 5, 0);
	spinNumCols = gtk_spin_button_new (spinRangeCols, 1, 0);
	gtk_widget_show (spinNumCols);
	gtk_table_attach (GTK_TABLE (tableInsert), spinNumCols, 1, 2, 1, 2,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_InsertTable_NumRows).c_str());
	labelNumRows = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelNumRows);
	gtk_table_attach (GTK_TABLE (tableInsert), labelNumRows, 0, 1, 2, 3,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	gtk_misc_set_alignment (GTK_MISC (labelNumRows), 0, 0.5);
	
	spinRangeRows = (GtkAdjustment *) gtk_adjustment_new(getNumRows(), 1, 9999, 1, 5, 0);
	spinNumRows = gtk_spin_button_new (spinRangeRows, 1, 0);
	gtk_widget_show (spinNumRows);
	gtk_table_attach (GTK_TABLE (tableInsert), spinNumRows, 1, 2, 2, 3,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);

	hseparator1 = gtk_hseparator_new ();
	gtk_widget_show (hseparator1);
	gtk_table_attach (GTK_TABLE (tableInsert), hseparator1, 0, 3, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 6);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_InsertTable_AutoFit).c_str());
	labelAutoFit = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelAutoFit);
	gtk_table_attach (GTK_TABLE (tableInsert), labelAutoFit, 0, 1, 4, 5,
					  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_widget_set_usize (labelAutoFit, 17, -1);
	gtk_label_set_justify (GTK_LABEL (labelAutoFit), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelAutoFit), 0, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_InsertTable_AutoColSize).c_str());
	radiobuttonAutoColSize = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonAutoColSize));
	g_object_set_data (G_OBJECT (radiobuttonAutoColSize), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_AUTOSIZE));
	gtk_widget_show (radiobuttonAutoColSize);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonAutoColSize, 0, 1, 5, 6,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_InsertTable_FixedColSize).c_str());
	radiobuttonFixedColSize = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonFixedColSize));
	g_object_set_data (G_OBJECT (radiobuttonFixedColSize), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_FIXEDSIZE));
	gtk_widget_show (radiobuttonFixedColSize);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonFixedColSize, 0, 1, 6, 7,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 6);

	spinRangeColWidth = (GtkAdjustment *) gtk_adjustment_new(getColumnWidth(), 0.1, 9999.0, 0.1, 1.0, 0.0);
	spinColWidth = gtk_spin_button_new (spinRangeColWidth, 0.1, 1);
	gtk_widget_show (spinColWidth);
	gtk_table_attach (GTK_TABLE (tableInsert), spinColWidth, 1, 2, 6, 7,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 6);

	UT_XML_cloneNoAmpersands(unixstr, /*pSS->getValueUTF8(AP_STRING_ID_DLG_InsertTable_Units).c_str()*/"inch");
	labelUnits = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelUnits);
	gtk_table_attach (GTK_TABLE (tableInsert), labelUnits, 2, 3, 6, 7,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 6);
	gtk_misc_set_alignment (GTK_MISC (labelNumCols), 0, 0.5);

	m_pRowSpin = spinNumRows;
	m_pColSpin = spinNumCols;
	m_pColWidthSpin = spinColWidth;
	
	m_radioGroup = tableInsert_group;

	m_wContents = tableInsert;
	
	return m_wContents;
}

void AP_UnixDialog_InsertTable::_populateWindowData(void)
{
	// We're (still) a stateless dialog, so there are 
	// no member variables to setyet
}

void AP_UnixDialog_InsertTable::_storeWindowData(void)
{
	m_columnType = _getActiveRadioItem();
	m_numRows = (UT_uint32) gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pRowSpin));
	m_numCols = (UT_uint32) gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pColSpin));
	m_columnWidth = (float) gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(m_pColWidthSpin));
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

