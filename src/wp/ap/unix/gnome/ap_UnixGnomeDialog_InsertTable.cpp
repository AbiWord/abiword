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
