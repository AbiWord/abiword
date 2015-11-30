/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_HdrFtr.h"
#include "ut_debugmsg.h"


static void s_HdrEven(GtkWidget * /*btn*/, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->CheckChanged( AP_Dialog_HdrFtr::HdrEven);
}

static void s_HdrFirst(GtkWidget * /*btn*/, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->CheckChanged(AP_Dialog_HdrFtr::HdrFirst);
}


static void s_HdrLast(GtkWidget * /*btn*/, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->CheckChanged(AP_Dialog_HdrFtr::HdrLast);
}

static void s_FtrEven(GtkWidget * /*btn*/, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->CheckChanged(AP_Dialog_HdrFtr::FtrEven);
}

static void s_FtrFirst(GtkWidget * /*btn*/, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->CheckChanged(AP_Dialog_HdrFtr::FtrFirst);
}

static void s_FtrLast(GtkWidget * /*btn*/, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->CheckChanged(AP_Dialog_HdrFtr::FtrLast);
}

static void s_restart_toggled(GtkWidget * /*btn*/, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->RestartChanged();
}

static void s_spin_changed(GtkWidget * /*btn*/, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->RestartSpinChanged();
}

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_HdrFtr::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_HdrFtr * p = new AP_UnixDialog_HdrFtr(pFactory,id);
	return static_cast<XAP_Dialog *>(p);
}

AP_UnixDialog_HdrFtr::AP_UnixDialog_HdrFtr(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_HdrFtr(pDlgFactory,id)
{
}

AP_UnixDialog_HdrFtr::~AP_UnixDialog_HdrFtr(void)
{
}

void AP_UnixDialog_HdrFtr::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);

	// Build the window's widgets and arrange them
	m_windowMain = _constructWindow();
	UT_return_if_fail(m_windowMain);

	switch(abiRunModalDialog(GTK_DIALOG(m_windowMain), pFrame, this,
							 GTK_RESPONSE_OK, true ))
	{
		case GTK_RESPONSE_OK:
			setAnswer(a_OK);
			break;
		default:
			setAnswer(a_CANCEL);
			break;
	}
}

/*!
 * A check button has controlling a footer type has been changed.
 */
void AP_UnixDialog_HdrFtr::CheckChanged(HdrFtr_Control which)
{
	bool value = false;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_wHdrFtrCheck[which])))
	{
		value = true;
	}
	setValue(which, value, true);
}

/*!
 * Update the XP values of the spin button.
 */
void AP_UnixDialog_HdrFtr::RestartSpinChanged(void)
{
	UT_sint32 RestartValue = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_wSpin));
	setRestart(true, RestartValue, true);
}

/*!
 * The Check button controlling whether page numbering should be restarted at
 * section has been changed.
 */
void AP_UnixDialog_HdrFtr::RestartChanged(void)
{
	UT_sint32 RestartValue = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_wSpin));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_wRestartButton)))
	{
		gtk_widget_set_sensitive(m_wRestartLabel,TRUE);
		gtk_widget_set_sensitive(m_wSpin,TRUE);
		setRestart(true, RestartValue, true);
	}
	else
	{
		gtk_widget_set_sensitive(m_wRestartLabel,FALSE);
		gtk_widget_set_sensitive(m_wSpin,FALSE);
		setRestart(false, RestartValue, true);
	}
}

/*!
 * construct the dialog window.
 */
GtkWidget * AP_UnixDialog_HdrFtr::_constructWindow (void)
{
	GtkWidget * window;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	GtkBuilder * builder = newDialogBuilder("ap_UnixDialog_HdrFtr.ui");

	// Update our member variables with the important widgets that
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_HdrFtr"));
	m_wHdrFtrCheck[HdrEven] = GTK_WIDGET(gtk_builder_get_object(builder, "cbHeaderFacingPages"));
	m_wHdrFtrCheck[HdrFirst] = GTK_WIDGET(gtk_builder_get_object(builder, "cbHeaderFirstPage"));
	m_wHdrFtrCheck[HdrLast] = GTK_WIDGET(gtk_builder_get_object(builder, "cbHeaderLastPage"));
	m_wHdrFtrCheck[FtrEven] = GTK_WIDGET(gtk_builder_get_object(builder, "cbFooterFacingPages"));
	m_wHdrFtrCheck[FtrFirst] = GTK_WIDGET(gtk_builder_get_object(builder, "cbFooterFirstPage"));
	m_wHdrFtrCheck[FtrLast] = GTK_WIDGET(gtk_builder_get_object(builder, "cbFooterLastPage"));
	m_wRestartLabel = GTK_WIDGET(gtk_builder_get_object(builder, "lbRestartNumbering"));
	m_wRestartButton = GTK_WIDGET(gtk_builder_get_object(builder, "lbRestartPageNumbers"));
	m_wSpin = GTK_WIDGET(gtk_builder_get_object(builder, "sbRestartNumberingAt"));
	m_spinAdj = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON(m_wSpin) );
	
	// set the dialog title
	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_Title,s);
	abiDialogSetTitle(window, "%s", s.c_str());

	// localize the strings in our dialog
	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbHeaderProperties")), pSS, AP_STRING_ID_DLG_HdrFtr_HeaderFrame);
	localizeButton(m_wHdrFtrCheck[HdrEven], pSS, AP_STRING_ID_DLG_HdrFtr_HeaderEven);
	localizeButton(m_wHdrFtrCheck[HdrFirst], pSS, AP_STRING_ID_DLG_HdrFtr_HeaderFirst);
	localizeButton(m_wHdrFtrCheck[HdrLast], pSS, AP_STRING_ID_DLG_HdrFtr_HeaderLast);

	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbFooterProperties")), pSS, AP_STRING_ID_DLG_HdrFtr_FooterFrame);
	localizeButton(m_wHdrFtrCheck[FtrEven], pSS, AP_STRING_ID_DLG_HdrFtr_FooterEven);
	localizeButton(m_wHdrFtrCheck[FtrFirst], pSS, AP_STRING_ID_DLG_HdrFtr_FooterFirst);
	localizeButton(m_wHdrFtrCheck[FtrLast], pSS, AP_STRING_ID_DLG_HdrFtr_FooterLast);

	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbPageNumberProperties")), pSS, AP_STRING_ID_DLG_HdrFtr_PageNumberProperties);
	localizeButton(m_wRestartButton, pSS, AP_STRING_ID_DLG_HdrFtr_RestartCheck);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbRestartNumbering")), pSS, AP_STRING_ID_DLG_HdrFtr_RestartNumbers);

	// Now set initial state of the dialog
	
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_wSpin),static_cast<gfloat>(getRestartValue()));
	if(isRestart())
	{
		gtk_widget_set_sensitive(m_wSpin,TRUE);
		gtk_widget_set_sensitive(m_wRestartLabel,TRUE);
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(m_wRestartButton),TRUE);
	}
	else
	{
		gtk_widget_set_sensitive(m_wSpin,FALSE);
		gtk_widget_set_sensitive(m_wRestartLabel,FALSE);
	}
	UT_sint32 j = static_cast<UT_sint32>(HdrEven);
	for(j = static_cast<UT_sint32>(HdrEven) ; j<= static_cast<UT_sint32>(FtrLast); j++)
	{
		bool value = getValue( static_cast<HdrFtr_Control>(j));
		if(value)
		{
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(m_wHdrFtrCheck[j]),TRUE);
		}
		else
		{
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(m_wHdrFtrCheck[j]),FALSE);
		}
	}

	_connectSignals();
  	
	g_object_unref(G_OBJECT(builder));

	return window;
}

void AP_UnixDialog_HdrFtr::_connectSignals(void)
{
	g_signal_connect (G_OBJECT(m_wHdrFtrCheck[HdrEven]), 
						"toggled", 
						G_CALLBACK(s_HdrEven), 
						reinterpret_cast<gpointer>(this));

	g_signal_connect (G_OBJECT(m_wHdrFtrCheck[HdrFirst]), 
						"toggled", 
						G_CALLBACK(s_HdrFirst), 
						reinterpret_cast<gpointer>(this));

	g_signal_connect (G_OBJECT(m_wHdrFtrCheck[HdrLast]), 
						"toggled", 
						G_CALLBACK(s_HdrLast), 
						reinterpret_cast<gpointer>(this));

	g_signal_connect (G_OBJECT(m_wHdrFtrCheck[FtrEven]), 
						"toggled", 
						G_CALLBACK(s_FtrEven), 
						reinterpret_cast<gpointer>(this));

	g_signal_connect (G_OBJECT(m_wHdrFtrCheck[FtrFirst]), 
						"toggled", 
						G_CALLBACK(s_FtrFirst), 
						reinterpret_cast<gpointer>(this));

	g_signal_connect (G_OBJECT(m_wHdrFtrCheck[FtrLast]), 
						"toggled", 
						G_CALLBACK(s_FtrLast), 
						reinterpret_cast<gpointer>(this));

	g_signal_connect (G_OBJECT(m_wRestartButton), 
						"toggled", 
						G_CALLBACK(s_restart_toggled), 
						reinterpret_cast<gpointer>(this));

	g_signal_connect (G_OBJECT (m_spinAdj), "value_changed",
						G_CALLBACK (s_spin_changed),
						reinterpret_cast<gpointer>(this));

	g_signal_connect (G_OBJECT(m_wRestartButton), 
						"toggled", 
						G_CALLBACK(s_restart_toggled), 
						reinterpret_cast<gpointer>(this));
}
