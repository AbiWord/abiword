/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_InsertTable.h"
#include "ap_UnixDialog_InsertTable.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"
#define CUSTOM_RESPONSE_INSERT 1

/*****************************************************************/

static void
s_auto_colsize_toggled (GtkToggleButton *radio,
                        GtkWidget       *spinner)
{
	gtk_widget_set_sensitive (spinner, !gtk_toggle_button_get_active (radio));
}

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
    // Build the dialog's window
	m_windowMain = _constructWindow();
	UT_return_if_fail(m_windowMain);

	// Populate the window's data items
	_populateWindowData();

	switch ( abiRunModalDialog ( GTK_DIALOG(m_windowMain),
								 pFrame, this, CUSTOM_RESPONSE_INSERT, false ) )
	{
		case CUSTOM_RESPONSE_INSERT:
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
	
	// get the path where our UI file is located
	std::string ui_path = static_cast<XAP_UnixApp*>(XAP_App::getApp())->getAbiSuiteAppUIDir() + "/ap_UnixDialog_InsertTable.xml";
	
	// load the dialog from the UI file
	GtkBuilder* builder = gtk_builder_new();
	gtk_builder_set_translation_domain(builder, GETTEXT_PACKAGE);
	gtk_builder_add_from_file(builder, ui_path.c_str(), NULL);
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_InsertTable"));
	m_radioGroup = gtk_radio_button_get_group (GTK_RADIO_BUTTON ( GTK_WIDGET(gtk_builder_get_object(builder, "rbAutoColSize")) ));
	m_pColSpin = GTK_WIDGET(gtk_builder_get_object(builder, "sbNumCols"));
	m_pRowSpin = GTK_WIDGET(gtk_builder_get_object(builder, "sbNumRows"));
	m_pColWidthSpin = GTK_WIDGET(gtk_builder_get_object(builder, "sbColSize"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_pColSpin), getNumCols());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_pRowSpin), getNumRows());

	GtkWidget *rbAutoColSize = GTK_WIDGET(gtk_builder_get_object(builder, "rbAutoColSize"));
	s_auto_colsize_toggled (GTK_TOGGLE_BUTTON (rbAutoColSize), m_pColWidthSpin);
	g_signal_connect (G_OBJECT (rbAutoColSize), "toggled", G_CALLBACK (s_auto_colsize_toggled), m_pColWidthSpin);
	
	// Units
	double spinstep = getSpinIncr ();
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(m_pColWidthSpin), spinstep, spinstep * 5);
	double spinmin = getSpinMin ();
	gtk_spin_button_set_range (GTK_SPIN_BUTTON(m_pColWidthSpin), spinmin, spinmin * 1000);
	
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_pColWidthSpin), m_columnWidth);
	// set tags for some widgets

	g_object_set_data (G_OBJECT (GTK_WIDGET(gtk_builder_get_object(builder, "rbAutoColSize"))), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_AUTOSIZE));	
	g_object_set_data (G_OBJECT (GTK_WIDGET(gtk_builder_get_object(builder, "rbFixedColSize"))), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_FIXEDSIZE));

	g_object_unref(G_OBJECT(builder));

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
	m_columnWidth = static_cast<float>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_pColWidthSpin)));
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
