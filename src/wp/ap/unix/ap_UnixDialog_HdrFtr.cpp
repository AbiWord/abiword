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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_HdrFtr.h"
#include "ut_debugmsg.h"


static void s_HdrEven(GtkWidget * btn, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->CheckChanged( AP_Dialog_HdrFtr::HdrEven);
}

static void s_HdrFirst(GtkWidget * btn, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->CheckChanged(AP_Dialog_HdrFtr::HdrFirst);
}


static void s_HdrLast(GtkWidget * btn, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->CheckChanged(AP_Dialog_HdrFtr::HdrLast);
}

static void s_FtrEven(GtkWidget * btn, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->CheckChanged(AP_Dialog_HdrFtr::FtrEven);
}

static void s_FtrFirst(GtkWidget * btn, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->CheckChanged(AP_Dialog_HdrFtr::FtrFirst);
}

static void s_FtrLast(GtkWidget * btn, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->CheckChanged(AP_Dialog_HdrFtr::FtrLast);
}

static void s_restart_toggled(GtkWidget * btn, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->RestartChanged();
}

static void s_spin_changed(GtkWidget * btn, AP_UnixDialog_HdrFtr * dlg)
{
	UT_return_if_fail(dlg);
	dlg->RestartSpinChanged();
}

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_HdrFtr::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_HdrFtr * p = new AP_UnixDialog_HdrFtr(pFactory,id);
	return (XAP_Dialog *) p;
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
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	switch(abiRunModalDialog(GTK_DIALOG(mainWindow), pFrame, this,
							 BUTTON_CANCEL, true ))
	{
		case BUTTON_OK:
			eventOk(); break ;
		default:
			eventCancel() ; break ;
	}
}

void AP_UnixDialog_HdrFtr::eventOk (void)
{
	setAnswer (a_OK);
}

void AP_UnixDialog_HdrFtr::eventCancel (void)
{
	setAnswer(a_CANCEL);
}

/*!
 * A check button has controlling a footer type has been changed.
 */
void AP_UnixDialog_HdrFtr::CheckChanged(HdrFtr_Control which)
{
	bool value = false;
	if(GTK_TOGGLE_BUTTON (m_wHdrFtrCheck[which])->active)
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
	if(GTK_TOGGLE_BUTTON (m_wRestartButton)->active)
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
	GtkWidget *HdrFtrDialog;
	GtkWidget *vbox1;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	HdrFtrDialog = abiDialogNew ( "headers and footers dialog", TRUE, pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_Title).c_str()) ;

	vbox1 = GTK_DIALOG(HdrFtrDialog)->vbox ;

    _constructWindowContents (vbox1);
    
    abiAddStockButton ( GTK_DIALOG(HdrFtrDialog), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;
    abiAddStockButton ( GTK_DIALOG(HdrFtrDialog), GTK_STOCK_OK, BUTTON_OK ) ;
	
	m_wHdrFtrDialog = HdrFtrDialog;

	_connectSignals();
  	
	return HdrFtrDialog;
}

void AP_UnixDialog_HdrFtr::_constructWindowContents (GtkWidget * parent)
{
	GtkWidget *HeaderFrame;
	GtkWidget *vbox2;
	GtkWidget *HeaderEven;
	GtkWidget *HeaderFirst;
	GtkWidget *HeaderLast;
	GtkWidget *FooterFrame;
	GtkWidget *vbox3;
	GtkWidget *FooterEven;
	GtkWidget *FooterFirst;
	GtkWidget *FooterLast;
	GtkWidget *hbox1;
	GtkWidget *ReStartButton;
	GtkWidget *restartLabel;
	GtkObject *spinbutton1_adj;
	GtkWidget *spinbutton1;


	const XAP_StringSet * pSS = m_pApp->getStringSet();

	HeaderFrame = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_HeaderFrame).c_str());
	gtk_widget_show (HeaderFrame);
	gtk_box_pack_start (GTK_BOX (parent), HeaderFrame, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (HeaderFrame), 6);
	gtk_frame_set_shadow_type (GTK_FRAME (HeaderFrame), GTK_SHADOW_ETCHED_OUT);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_container_add (GTK_CONTAINER (HeaderFrame), vbox2);

	HeaderEven = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_HeaderEven).c_str());
	gtk_widget_show (HeaderEven);
	gtk_box_pack_start (GTK_BOX (vbox2), HeaderEven, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (HeaderEven), 1);

	HeaderFirst = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_HeaderFirst).c_str());
	gtk_widget_show (HeaderFirst);
	gtk_box_pack_start (GTK_BOX (vbox2), HeaderFirst, FALSE, TRUE, 0);

	HeaderLast = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_HeaderLast).c_str());
	gtk_widget_show (HeaderLast);
	gtk_box_pack_start (GTK_BOX (vbox2), HeaderLast, FALSE, TRUE, 0);

	FooterFrame = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_FooterFrame).c_str());
	gtk_widget_show (FooterFrame);
	gtk_box_pack_start (GTK_BOX (parent), FooterFrame, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (FooterFrame), 5);
	gtk_frame_set_shadow_type (GTK_FRAME (FooterFrame), GTK_SHADOW_ETCHED_OUT);

	vbox3 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox3);
	gtk_container_add (GTK_CONTAINER (FooterFrame), vbox3);

	FooterEven = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_FooterEven).c_str());
	gtk_widget_show (FooterEven);
	gtk_box_pack_start (GTK_BOX (vbox3), FooterEven, FALSE, TRUE, 0);

	FooterFirst = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_FooterFirst).c_str());
	gtk_widget_show (FooterFirst);
	gtk_box_pack_start (GTK_BOX (vbox3), FooterFirst, FALSE, TRUE, 0);

	FooterLast = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_FooterLast).c_str());
	gtk_widget_show (FooterLast);
	gtk_box_pack_start (GTK_BOX (vbox3), FooterLast, FALSE, FALSE, 0);

	hbox1 = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (parent), hbox1, TRUE, TRUE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (hbox1), 3);

	ReStartButton = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_RestartCheck).c_str());
	gtk_widget_show (ReStartButton);
	gtk_box_pack_start (GTK_BOX (hbox1), ReStartButton, FALSE, FALSE, 0);

	restartLabel = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_RestartNumbers).c_str());
	gtk_widget_show (restartLabel);
	gtk_box_pack_start (GTK_BOX (hbox1), restartLabel, TRUE, TRUE, 0);
	gtk_label_set_justify (GTK_LABEL (restartLabel), GTK_JUSTIFY_RIGHT);

	spinbutton1_adj = gtk_adjustment_new (1, 0, 10000, 1, 10, 10);
	spinbutton1 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton1_adj), 1, 0);
	gtk_widget_show (spinbutton1);
	gtk_box_pack_end (GTK_BOX (hbox1), spinbutton1, FALSE, FALSE, 2);

	m_wHdrFtrCheck[HdrEven] = HeaderEven;
	m_wHdrFtrCheck[HdrFirst] = HeaderFirst;
	m_wHdrFtrCheck[HdrLast] = HeaderLast;
	m_wHdrFtrCheck[FtrEven] = FooterEven;
	m_wHdrFtrCheck[FtrFirst] = FooterFirst;
	m_wHdrFtrCheck[FtrLast] = FooterLast;
	m_wRestartButton = ReStartButton;
	m_wRestartLabel = restartLabel;
	m_oSpinAdj = spinbutton1_adj;
	m_wSpin = spinbutton1;

//
// Now set initial state of the dialog.
//
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_wSpin),(gfloat) getRestartValue());
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
	UT_sint32 j = (UT_sint32) HdrEven;
	for(j = (UT_sint32) HdrEven ; j<= (UT_sint32) FtrLast; j++)
	{
		bool value = getValue( (HdrFtr_Control) j);
		if(value)
		{
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(m_wHdrFtrCheck[j]),TRUE);
		}
		else
		{
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(m_wHdrFtrCheck[j]),FALSE);
		}
	}
}


void AP_UnixDialog_HdrFtr::_connectSignals(void)
{
	g_signal_connect (G_OBJECT(m_wHdrFtrCheck[HdrEven]), 
						"toggled", 
						G_CALLBACK(s_HdrEven), 
						(gpointer)this);

	g_signal_connect (G_OBJECT(m_wHdrFtrCheck[HdrFirst]), 
						"toggled", 
						G_CALLBACK(s_HdrFirst), 
						(gpointer)this);

	g_signal_connect (G_OBJECT(m_wHdrFtrCheck[HdrLast]), 
						"toggled", 
						G_CALLBACK(s_HdrLast), 
						(gpointer)this);

	g_signal_connect (G_OBJECT(m_wHdrFtrCheck[FtrEven]), 
						"toggled", 
						G_CALLBACK(s_FtrEven), 
						(gpointer)this);

	g_signal_connect (G_OBJECT(m_wHdrFtrCheck[FtrFirst]), 
						"toggled", 
						G_CALLBACK(s_FtrFirst), 
						(gpointer)this);

	g_signal_connect (G_OBJECT(m_wHdrFtrCheck[FtrLast]), 
						"toggled", 
						G_CALLBACK(s_FtrLast), 
						(gpointer)this);

	g_signal_connect (G_OBJECT(m_wRestartButton), 
						"toggled", 
						G_CALLBACK(s_restart_toggled), 
						(gpointer)this);

	g_signal_connect (G_OBJECT (m_oSpinAdj), "value_changed",
						G_CALLBACK (s_spin_changed),
						(gpointer) this);

	g_signal_connect (G_OBJECT(m_wRestartButton), 
						"toggled", 
						G_CALLBACK(s_restart_toggled), 
						(gpointer)this);
}
