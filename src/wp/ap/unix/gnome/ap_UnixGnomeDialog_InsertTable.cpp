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
#include <gnome.h>

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
#include "ap_Dialog_Break.h"
#include "ap_UnixDialog_Break.h"
#include "ap_UnixGnomeDialog_InsertTable.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixGnomeDialog_InsertTable::static_constructor(XAP_DialogFactory * pFactory,
															    XAP_Dialog_Id id)
{
	AP_UnixGnomeDialog_InsertTable * p = new AP_UnixGnomeDialog_InsertTable(pFactory,id);
	return p;
}

AP_UnixGnomeDialog_InsertTable::AP_UnixGnomeDialog_InsertTable(XAP_DialogFactory * pDlgFactory,
														       XAP_Dialog_Id id)
	: AP_UnixDialog_InsertTable(pDlgFactory, id)
{
}

AP_UnixGnomeDialog_InsertTable::~AP_UnixGnomeDialog_InsertTable(void)
{
}

/*****************************************************************/

static void s_ok_clicked(GtkWidget * widget, AP_UnixDialog_Break * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
}

static void s_cancel_clicked(GtkWidget * widget, AP_UnixDialog_Break * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
}

static void s_row_spin(GtkWidget * widget, AP_UnixDialog_InsertTable * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_SpinRows();
}

static void s_col_spin(GtkWidget * widget, AP_UnixDialog_InsertTable * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_SpinCols();
}

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 AP_UnixDialog_Break * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
}

/*****************************************************************/

GtkWidget * AP_UnixGnomeDialog_InsertTable::_constructWindow(void)
{
	GtkWidget * windowInsertTable;
	GtkWidget * vboxMain;
	GtkWidget * tableInsert;
	GtkWidget * labelInsert;
	GtkWidget * labelNumRows;
	GtkWidget * spinNumRows;
	GtkAdjustment * spinRangeRows;
	GtkWidget * labelNumCols;
	GtkWidget * spinNumCols;
	GtkAdjustment * spinRangeCols;
	GtkWidget * hseparator;
	GtkWidget * hbuttonboxInsertTable;
	GtkWidget * buttonOK;
	GtkWidget * buttonCancel;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	windowInsertTable = gtk_window_new (GTK_WINDOW_DIALOG);
	g_object_set_data (G_OBJECT (windowInsertTable), "windowInsertTable", windowInsertTable);
	gtk_window_set_title (GTK_WINDOW (windowInsertTable), "Insert Table"/*pSS->getValue(AP_STRING_ID_DLG_InsertTable)*/);
	gtk_window_set_policy (GTK_WINDOW (windowInsertTable), FALSE, FALSE, FALSE);
	
	vboxMain = gtk_vbox_new (FALSE, 0);
	g_object_set_data (G_OBJECT (windowInsertTable), "vboxMain", vboxMain);
	gtk_widget_show (vboxMain);
	gtk_container_add (GTK_CONTAINER (windowInsertTable), vboxMain);
	gtk_container_set_border_width (GTK_CONTAINER (vboxMain), 10);

	tableInsert = gtk_table_new (4, 2, FALSE);
	g_object_set_data (G_OBJECT (windowInsertTable), "tableInsert", tableInsert);
	gtk_widget_show (tableInsert);
	gtk_box_pack_start (GTK_BOX (vboxMain), tableInsert, FALSE, FALSE, 0);

	UT_XML_cloneNoAmpersands(unixstr, /*pSS->getValue(AP_STRING_ID_DLG_InsertTable_TableSize)*/"Table size");
	labelInsert = gtk_label_new (unixstr);
	FREEP(unixstr);
	g_object_set_data (G_OBJECT (windowInsertTable), "labelInsert", labelInsert);
	gtk_widget_show (labelInsert);
	gtk_table_attach (GTK_TABLE (tableInsert), labelInsert, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_widget_set_usize (labelInsert, 17, -1);
	gtk_label_set_justify (GTK_LABEL (labelInsert), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelInsert), 0, 0.5);

	UT_XML_cloneNoAmpersands(unixstr, /*pSS->getValue(AP_STRING_ID_DLG_InsertTable_NumCols)*/"Number of columns:");
	labelNumCols = gtk_label_new (unixstr);
	FREEP(unixstr);
	g_object_set_data (G_OBJECT (windowInsertTable), "labelNumCols", labelNumCols);
	gtk_widget_show (labelNumCols);
	gtk_table_attach (GTK_TABLE (tableInsert), labelNumCols, 0, 1, 1, 2,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	gtk_misc_set_alignment (GTK_MISC (labelNumCols), 0, 0.5);

	spinRangeCols = (GtkAdjustment *) gtk_adjustment_new(5, 1, 9999, 1, 1, 0);
	spinNumCols = gtk_spin_button_new (spinRangeCols, 1, 0);
	g_object_set_data (G_OBJECT (windowInsertTable), "spinNumRows", spinRangeCols);
	gtk_widget_show (spinNumCols);
	gtk_table_attach (GTK_TABLE (tableInsert), spinNumCols, 1, 2, 1, 2,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);

	UT_XML_cloneNoAmpersands(unixstr, /*pSS->getValue(AP_STRING_ID_DLG_InsertTable_NumCols)*/"Number of rows:");
	labelNumRows = gtk_label_new (unixstr);
	FREEP(unixstr);
	g_object_set_data (G_OBJECT (windowInsertTable), "labelNumRows", labelNumRows);
	gtk_widget_show (labelNumRows);
	gtk_table_attach (GTK_TABLE (tableInsert), labelNumRows, 0, 1, 2, 3,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	gtk_misc_set_alignment (GTK_MISC (labelNumRows), 0, 0.5);
	
	spinRangeRows = (GtkAdjustment *) gtk_adjustment_new(2, 1, 9999, 1, 1, 0);
	spinNumRows = gtk_spin_button_new (spinRangeRows, 1, 0);
	g_object_set_data (G_OBJECT (windowInsertTable), "spinNumRows", spinNumRows);
	gtk_widget_show (spinNumRows);
	gtk_table_attach (GTK_TABLE (tableInsert), spinNumRows, 1, 2, 2, 3,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);

	hseparator = gtk_hseparator_new ();
	g_object_set_data (G_OBJECT (windowInsertTable), "hseparator", hseparator);
	gtk_widget_show (hseparator);
	gtk_table_attach (GTK_TABLE (tableInsert), hseparator, 0, 2, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 6);

	hbuttonboxInsertTable = gtk_hbutton_box_new ();
	g_object_set_data (G_OBJECT (windowInsertTable), "hbuttonboxInsertTable", hbuttonboxInsertTable);
	gtk_widget_show (hbuttonboxInsertTable);
	gtk_box_pack_start (GTK_BOX (vboxMain), hbuttonboxInsertTable, FALSE, FALSE, 4);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonboxInsertTable), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonboxInsertTable), 10);
	gtk_button_box_set_child_size (GTK_BUTTON_BOX (hbuttonboxInsertTable), 85, 24);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (hbuttonboxInsertTable), 0, 0);

	buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	g_object_set_data (G_OBJECT (windowInsertTable), "buttonOK", buttonOK);
	gtk_widget_show (buttonOK);
	gtk_container_add (GTK_CONTAINER (hbuttonboxInsertTable), buttonOK);

	buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	g_object_set_data (G_OBJECT (windowInsertTable), "buttonCancel", buttonCancel);
	gtk_widget_show (buttonCancel);
	gtk_container_add (GTK_CONTAINER (hbuttonboxInsertTable), buttonCancel);

	// the control buttons
	g_signal_connect(G_OBJECT(buttonOK),
					   "clicked",
					   G_CALLBACK(s_ok_clicked),
					   (gpointer) this);
	
	g_signal_connect(G_OBJECT(buttonCancel),
					   "clicked",
					   G_CALLBACK(s_cancel_clicked),
					   (gpointer) this);


	// the spin controls
	g_signal_connect (G_OBJECT (spinNumRows), "value_changed",
						G_CALLBACK (s_row_spin),
						(gpointer) this);

	g_signal_connect (G_OBJECT (spinNumCols), "value_changed",
						G_CALLBACK (s_col_spin),
						(gpointer) this);

	// the catch-alls
	
	g_signal_connect(G_OBJECT(windowInsertTable),
			   "delete_event",
			   G_CALLBACK(s_delete_clicked),
			   (gpointer) this);

	g_signal_connect_after(G_OBJECT(windowInsertTable),
							 "destroy",
							 NULL,
							 NULL);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowInsertTable;
	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

	m_pRowspin = spinNumRows;
	m_pColspin = spinNumCols;

	return windowInsertTable;
}
