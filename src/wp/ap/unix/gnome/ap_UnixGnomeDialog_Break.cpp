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
#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Break.h"
#include "ap_UnixDialog_Break.h"
#include "ap_UnixGnomeDialog_Break.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixGnomeDialog_Break::static_constructor(XAP_DialogFactory * pFactory,
														  XAP_Dialog_Id id)
{
	AP_UnixGnomeDialog_Break * p = new AP_UnixGnomeDialog_Break(pFactory,id);
	return p;
}

AP_UnixGnomeDialog_Break::AP_UnixGnomeDialog_Break(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: AP_UnixDialog_Break(pDlgFactory, id)
{
}

AP_UnixGnomeDialog_Break::~AP_UnixGnomeDialog_Break(void)
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

GtkWidget * AP_UnixGnomeDialog_Break::_constructWindow(void)
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
	GtkWidget * hseparator10;
	GtkWidget * buttonOK;
	GtkWidget * buttonCancel;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	windowBreak = gnome_dialog_new (pSS->getValue(AP_STRING_ID_DLG_Break_BreakTitle),
									GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_CANCEL, NULL);
	gtk_object_set_data (GTK_OBJECT (windowBreak), "windowBreak", windowBreak);
	gtk_window_set_policy (GTK_WINDOW (windowBreak), FALSE, FALSE, FALSE);
	
  	vboxMain = GNOME_DIALOG (windowBreak)->vbox;
	gtk_object_set_data (GTK_OBJECT (windowBreak), "vboxMain", vboxMain);

	tableInsert = gtk_table_new (7, 2, FALSE);
	gtk_object_set_data (GTK_OBJECT (windowBreak), "tableInsert", tableInsert);
	gtk_widget_show (tableInsert);
	gtk_box_pack_start (GTK_BOX (vboxMain), tableInsert, FALSE, FALSE, 0);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_Insert));
	labelInsert = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_object_set_data (GTK_OBJECT (windowBreak), "labelInsert", labelInsert);
	gtk_widget_show (labelInsert);
	gtk_table_attach (GTK_TABLE (tableInsert), labelInsert, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_widget_set_usize (labelInsert, 17, -1);
	gtk_label_set_justify (GTK_LABEL (labelInsert), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelInsert), 0, 0.5);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_PageBreak));	
	radiobuttonPageBreak = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonPageBreak));
	gtk_object_set_data (GTK_OBJECT (windowBreak), "radiobuttonPageBreak", radiobuttonPageBreak);
	gtk_object_set_data (GTK_OBJECT (radiobuttonPageBreak), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_PAGE));
	gtk_widget_show (radiobuttonPageBreak);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonPageBreak, 0, 1, 1, 2,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_NextPage));	
	radiobuttonNextPage = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonNextPage));
	gtk_object_set_data (GTK_OBJECT (windowBreak), "radiobuttonNextPage", radiobuttonNextPage);
	gtk_object_set_data (GTK_OBJECT (radiobuttonNextPage), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_NEXTPAGE));
	gtk_widget_show (radiobuttonNextPage);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonNextPage, 0, 1, 4, 5,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_Continuous));	
	radiobuttonContinuous = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonContinuous));
	gtk_object_set_data (GTK_OBJECT (windowBreak), "radiobuttonContinuous", radiobuttonContinuous);
	gtk_object_set_data (GTK_OBJECT (radiobuttonContinuous), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_CONTINUOUS));
	gtk_widget_show (radiobuttonContinuous);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonContinuous, 0, 1, 5, 6,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_ColumnBreak));	
	radiobuttonColumnBreak = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonColumnBreak));
	gtk_object_set_data (GTK_OBJECT (windowBreak), "radiobuttonColumnBreak", radiobuttonColumnBreak);
	gtk_object_set_data (GTK_OBJECT (radiobuttonColumnBreak), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_COLUMN));
	gtk_widget_show (radiobuttonColumnBreak);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonColumnBreak, 1, 2, 1, 2,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_EvenPage));	
	radiobuttonEvenPage = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonEvenPage));
	gtk_object_set_data (GTK_OBJECT (windowBreak), "radiobuttonEvenPage", radiobuttonEvenPage);
	gtk_object_set_data (GTK_OBJECT (radiobuttonEvenPage), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_EVENPAGE));
	gtk_widget_show (radiobuttonEvenPage);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonEvenPage, 1, 2, 4, 5,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_OddPage));	
	radiobuttonOddPage = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonOddPage));
	gtk_object_set_data (GTK_OBJECT (windowBreak), "radiobuttonOddPage", radiobuttonOddPage);
	gtk_object_set_data (GTK_OBJECT (radiobuttonOddPage), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_ODDPAGE));
	gtk_widget_show (radiobuttonOddPage);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonOddPage, 1, 2, 5, 6,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_SectionBreaks));	
	labelSectionBreaks = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_object_set_data (GTK_OBJECT (windowBreak), "labelSectionBreaks", labelSectionBreaks);
	gtk_widget_show (labelSectionBreaks);
	gtk_table_attach (GTK_TABLE (tableInsert), labelSectionBreaks, 0, 1, 3, 4,
					  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 4);
	gtk_widget_set_usize (labelSectionBreaks, 76, -1);
	gtk_label_set_justify (GTK_LABEL (labelSectionBreaks), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelSectionBreaks), 0, 0.5);
	
	hseparator10 = gtk_hseparator_new ();
	gtk_object_set_data (GTK_OBJECT (windowBreak), "hseparator10", hseparator10);
	gtk_widget_show (hseparator10);
	gtk_table_attach (GTK_TABLE (tableInsert), hseparator10, 0, 2, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 6);

	buttonOK = (GtkWidget *) g_list_previous (g_list_last (GNOME_DIALOG (windowBreak)->buttons))->data;
	gtk_object_set_data (GTK_OBJECT (windowBreak), "buttonOK", buttonOK);

  	buttonCancel = (GtkWidget *) g_list_last (GNOME_DIALOG (windowBreak)->buttons)->data;
	gtk_object_set_data (GTK_OBJECT (windowBreak), "buttonCancel", buttonCancel);

	// the control buttons
	gtk_signal_connect(GTK_OBJECT(buttonOK),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(buttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (gpointer) this);

	// the catch-alls
	
	gtk_signal_connect(GTK_OBJECT(windowBreak),
			   "delete_event",
			   GTK_SIGNAL_FUNC(s_delete_clicked),
			   (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(windowBreak),
				 "destroy",
				 NULL,
				 NULL);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowBreak;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

	m_radioGroup = tableInsert_group;
	
	return windowBreak;
}
