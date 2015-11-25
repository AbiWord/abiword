/* Copyright (C) 2008 AbiSource Corporation B.V.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include "xap_App.h"
#include "ap_UnixApp.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixDialogHelper.h"
#include "ut_string_class.h"
#include <session/xp/AbiCollabSessionManager.h>

#include "ap_UnixDialog_GenericInput.h"

static void s_text_changed(GtkWidget * /*wid*/, AP_UnixDialog_GenericInput * dlg)
{
	dlg->eventTextChanged();
}

static void s_ok_clicked(GtkWidget * /*wid*/, AP_UnixDialog_GenericInput * dlg)
{
	dlg->eventOk();
}

XAP_Dialog * AP_UnixDialog_GenericInput::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return static_cast<XAP_Dialog *>(new AP_UnixDialog_GenericInput(pFactory, id));
}
pt2Constructor ap_Dialog_GenericInput_Constructor = &AP_UnixDialog_GenericInput::static_constructor;

AP_UnixDialog_GenericInput::AP_UnixDialog_GenericInput(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_GenericInput(pDlgFactory, id),
	m_wWindowMain(NULL),
	m_wOk(NULL)
{
}

void AP_UnixDialog_GenericInput::runModal(XAP_Frame * pFrame)
{
	// Build the dialog's window
	m_wWindowMain = _constructWindow();
	UT_return_if_fail(m_wWindowMain);

	_populateWindowData();

	switch ( abiRunModalDialog ( GTK_DIALOG(m_wWindowMain),
								 pFrame, this, GTK_RESPONSE_OK, false ) )
	{
		case GTK_RESPONSE_CANCEL:
			m_answer = AP_UnixDialog_GenericInput::a_CANCEL;
			break;
		case GTK_RESPONSE_OK:
			m_answer = AP_UnixDialog_GenericInput::a_OK;
			break;
		default:
			m_answer = AP_UnixDialog_GenericInput::a_CANCEL;
			break;
	}

	abiDestroyWidget(m_wWindowMain);
}

/*****************************************************************/
GtkWidget * AP_UnixDialog_GenericInput::_constructWindow(void)
{
	GtkWidget* window;
	
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_GenericInput.ui");
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_GenericInput"));
	m_wOk = GTK_WIDGET(gtk_builder_get_object(builder, "btOK"));
	m_wInput = GTK_WIDGET(gtk_builder_get_object(builder, "edInput"));

	// set the dialog title
	abiDialogSetTitle(window, "%s", getTitle().utf8_str());
	
	// set the question
	gtk_label_set_text(GTK_LABEL(GTK_WIDGET(gtk_builder_get_object(builder, "lbQuestion"))), getQuestion().utf8_str());
	gtk_label_set_text(GTK_LABEL(GTK_WIDGET(gtk_builder_get_object(builder, "lbLabel"))), getLabel().utf8_str());

	// "enter" in the edit box should trigger the default action
	gtk_entry_set_activates_default(GTK_ENTRY(m_wInput), true);

	// connect our signals
	g_signal_connect(G_OBJECT(m_wInput),
							"changed",
							G_CALLBACK(s_text_changed),
							static_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wOk),
							"clicked",
							G_CALLBACK(s_ok_clicked),
							static_cast<gpointer>(this));

	g_object_unref(G_OBJECT(builder));
	return window;
}

void AP_UnixDialog_GenericInput::_populateWindowData()
{
	// set the focus on the text input
	// TODO: implement me
	
	// set the password style input if requested
	gtk_entry_set_visibility(GTK_ENTRY(m_wInput), !isPassword());

	// set the initial input
	gtk_entry_set_text(GTK_ENTRY(m_wInput), getInput().utf8_str());

	// force the initial sensitivy state of the buttons
	eventTextChanged();
}

void AP_UnixDialog_GenericInput::eventTextChanged()
{
	const gchar* szText = gtk_entry_get_text(GTK_ENTRY(m_wInput));
	if (!szText || strlen(szText) < getMinLenght())
		gtk_widget_set_sensitive(m_wOk, false);
	else
		gtk_widget_set_sensitive(m_wOk, true);
}

void AP_UnixDialog_GenericInput::eventOk()
{
	setInput(gtk_entry_get_text(GTK_ENTRY(m_wInput)));
}

