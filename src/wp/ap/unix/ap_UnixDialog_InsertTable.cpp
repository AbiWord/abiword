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

	m_buttonOK = NULL;
	m_buttonCancel = NULL;
}

AP_UnixDialog_InsertTable::~AP_UnixDialog_InsertTable(void)
{
}

/*****************************************************************/

static void s_ok_clicked(GtkWidget * widget, AP_UnixDialog_InsertTable * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
}

static void s_cancel_clicked(GtkWidget * widget, AP_UnixDialog_InsertTable * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
}

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 AP_UnixDialog_InsertTable * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
}

/*****************************************************************/

void AP_UnixDialog_InsertTable::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(GTK_WIDGET(mainWindow),pFrame);
	// Populate the window's data items
	_populateWindowData();
	
	// To center the dialog, we need the frame of its parent.
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(pUnixFrame);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
	centerDialog(parentWindow, mainWindow);

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// Run into the GTK event loop for this window.
	gtk_main();

	// Store the window settings
	_storeWindowData();

	if(mainWindow && GTK_IS_WIDGET(mainWindow))
	  gtk_widget_destroy(mainWindow);
}

void AP_UnixDialog_InsertTable::event_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_InsertTable::a_OK;
	gtk_main_quit();
}

void AP_UnixDialog_InsertTable::event_Cancel(void)
{
	m_answer = AP_Dialog_InsertTable::a_CANCEL;
	gtk_main_quit();
}

void AP_UnixDialog_InsertTable::event_WindowDelete(void)
{
	m_answer = AP_Dialog_InsertTable::a_CANCEL;	
	gtk_main_quit();
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_InsertTable::_constructWindow(void)
{
	GtkWidget * windowInsertTable;
	GtkWidget * hbuttonboxInsertTable;
	GtkWidget * buttonOK;
	GtkWidget * buttonCancel;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowInsertTable = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_window_set_title (GTK_WINDOW (windowInsertTable), "Insert Table"/*pSS->getValue(AP_STRING_ID_DLG_InsertTable)*/);
	gtk_window_set_policy (GTK_WINDOW (windowInsertTable), FALSE, FALSE, FALSE);
	
	gtk_container_add (GTK_CONTAINER (windowInsertTable), _constructWindowContents());

	hbuttonboxInsertTable = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonboxInsertTable);
	gtk_box_pack_start (GTK_BOX (m_wContents), hbuttonboxInsertTable, FALSE, FALSE, 4);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonboxInsertTable), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonboxInsertTable), 10);
	gtk_button_box_set_child_size (GTK_BUTTON_BOX (hbuttonboxInsertTable), 85, 24);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (hbuttonboxInsertTable), 0, 0);

	buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_widget_show (buttonOK);
	gtk_container_add (GTK_CONTAINER (hbuttonboxInsertTable), buttonOK);

	buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_widget_show (buttonCancel);
	gtk_container_add (GTK_CONTAINER (hbuttonboxInsertTable), buttonCancel);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowInsertTable;
	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

	_connectSignals();

	return windowInsertTable;
}

GtkWidget * AP_UnixDialog_InsertTable::_constructWindowContents(void)
{
	GtkWidget * vboxMain;
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
	GtkWidget * hseparator2;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions
	
	vboxMain = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vboxMain);
	gtk_container_set_border_width (GTK_CONTAINER (vboxMain), 10);

	tableInsert = gtk_table_new (7, 3, FALSE);
	gtk_widget_show (tableInsert);
	gtk_box_pack_start (GTK_BOX (vboxMain), tableInsert, FALSE, FALSE, 0);

	UT_XML_cloneNoAmpersands(unixstr, /*pSS->getValue(AP_STRING_ID_DLG_InsertTable_TableSize)*/"Table size");
	labelInsert = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelInsert);
	gtk_table_attach (GTK_TABLE (tableInsert), labelInsert, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_widget_set_usize (labelInsert, 17, -1);
	gtk_label_set_justify (GTK_LABEL (labelInsert), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelInsert), 0, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, /*pSS->getValue(AP_STRING_ID_DLG_InsertTable_NumCols)*/"Number of columns:");
	labelNumCols = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelNumCols);
	gtk_table_attach (GTK_TABLE (tableInsert), labelNumCols, 0, 1, 1, 2,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	gtk_misc_set_alignment (GTK_MISC (labelNumCols), 0, 0.5);

	spinRangeCols = (GtkAdjustment *) gtk_adjustment_new(5, 1, 9999, 1, 5, 0);
	spinNumCols = gtk_spin_button_new (spinRangeCols, 1, 0);
	//g_object_set_data (G_OBJECT (windowInsertTable), "spinNumCols", spinNumCols);
	gtk_widget_show (spinNumCols);
	gtk_table_attach (GTK_TABLE (tableInsert), spinNumCols, 1, 2, 1, 2,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);

	UT_XML_cloneNoAmpersands(unixstr, /*pSS->getValue(AP_STRING_ID_DLG_InsertTable_NumRows)*/"Number of rows:");
	labelNumRows = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelNumRows);
	gtk_table_attach (GTK_TABLE (tableInsert), labelNumRows, 0, 1, 2, 3,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	gtk_misc_set_alignment (GTK_MISC (labelNumRows), 0, 0.5);
	
	spinRangeRows = (GtkAdjustment *) gtk_adjustment_new(2, 1, 9999, 1, 5, 0);
	spinNumRows = gtk_spin_button_new (spinRangeRows, 1, 0);
	gtk_widget_show (spinNumRows);
	gtk_table_attach (GTK_TABLE (tableInsert), spinNumRows, 1, 2, 2, 3,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);

	hseparator1 = gtk_hseparator_new ();
	gtk_widget_show (hseparator1);
	gtk_table_attach (GTK_TABLE (tableInsert), hseparator1, 0, 3, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 6);

	UT_XML_cloneNoAmpersands(unixstr, /*pSS->getValue(AP_STRING_ID_DLG_InsertTable_AutoFit)*/"AutoFit behavior");
	labelAutoFit = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelAutoFit);
	gtk_table_attach (GTK_TABLE (tableInsert), labelAutoFit, 0, 1, 4, 5,
					  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_widget_set_usize (labelAutoFit, 17, -1);
	gtk_label_set_justify (GTK_LABEL (labelAutoFit), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelAutoFit), 0, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, /*pSS->getValue(AP_STRING_ID_DLG_InsertTable_AutoColSize)*/"Automatic column size");
	radiobuttonAutoColSize = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonAutoColSize));
	g_object_set_data (G_OBJECT (radiobuttonAutoColSize), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_AUTOSIZE));
	gtk_widget_show (radiobuttonAutoColSize);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonAutoColSize, 0, 1, 5, 6,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, /*pSS->getValue(AP_STRING_ID_DLG_InsertTable_FixedColSize)*/"Fixed column size:");
	radiobuttonFixedColSize = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonFixedColSize));
	g_object_set_data (G_OBJECT (radiobuttonFixedColSize), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_FIXEDSIZE));
	gtk_widget_show (radiobuttonFixedColSize);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonFixedColSize, 0, 1, 6, 7,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);

	spinRangeColWidth = (GtkAdjustment *) gtk_adjustment_new(0.7, 0.1, 9999.0, 0.1, 1.0, 0.0);
	spinColWidth = gtk_spin_button_new (spinRangeColWidth, 0.1, 1);
	gtk_widget_show (spinColWidth);
	gtk_table_attach (GTK_TABLE (tableInsert), spinColWidth, 1, 2, 6, 7,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);

	UT_XML_cloneNoAmpersands(unixstr, /*pSS->getValue(AP_STRING_ID_DLG_InsertTable_Units)*/"inch");
	labelUnits = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelUnits);
	gtk_table_attach (GTK_TABLE (tableInsert), labelUnits, 2, 3, 6, 7,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	gtk_misc_set_alignment (GTK_MISC (labelNumCols), 0, 0.5);

	hseparator2 = gtk_hseparator_new ();
	gtk_widget_show (hseparator2);
	gtk_table_attach (GTK_TABLE (tableInsert), hseparator2, 0, 3, 7, 8,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 6);

	m_pRowSpin = spinNumRows;
	m_pColSpin = spinNumCols;
	m_pColWidthSpin = spinColWidth;
	
	m_radioGroup = tableInsert_group;

	m_wContents = vboxMain;
	
	return m_wContents;
}

void AP_UnixDialog_InsertTable::_connectSignals(void)
{
	g_signal_connect(G_OBJECT(m_buttonOK),
					   "clicked",
					   G_CALLBACK(s_ok_clicked),
					   (gpointer) this);
	
	g_signal_connect(G_OBJECT(m_buttonCancel),
					   "clicked",
					   G_CALLBACK(s_cancel_clicked),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_windowMain),
			   "delete_event",
			   G_CALLBACK(s_delete_clicked),
			   (gpointer) this);

	g_signal_connect_after(G_OBJECT(m_windowMain),
							 "destroy",
							 NULL,
							 NULL);
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

