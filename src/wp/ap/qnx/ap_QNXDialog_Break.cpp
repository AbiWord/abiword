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

// This header defines some functions for QNX dialogs,
// like centering them, measuring them, etc.

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Break.h"
#include "ap_QNXDialog_Break.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Break::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_QNXDialog_Break * p = new AP_QNXDialog_Break(pFactory,id);
	return p;
}

AP_QNXDialog_Break::AP_QNXDialog_Break(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Break(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_buttonOK = NULL;
	m_buttonCancel = NULL;

	m_radioGroup = NULL;
}

AP_QNXDialog_Break::~AP_QNXDialog_Break(void)
{
}

/*****************************************************************/
#if 0
static void s_ok_clicked(GtkWidget * widget, AP_QNXDialog_Break * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
}

static void s_cancel_clicked(GtkWidget * widget, AP_QNXDialog_Break * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
}

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 AP_QNXDialog_Break * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
}
#endif
/*****************************************************************/

void AP_QNXDialog_Break::runModal(XAP_Frame * pFrame)
{
#if 0
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	
	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);
	gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// Run into the GTK event loop for this window.
	gtk_main();

	_storeWindowData();
	
	gtk_widget_destroy(mainWindow);
#endif
}

void AP_QNXDialog_Break::event_OK(void)
{
#if 0
	// TODO save out state of radio items
	m_answer = AP_Dialog_Break::a_OK;
	gtk_main_quit();
#endif
}

void AP_QNXDialog_Break::event_Cancel(void)
{
#if 0
	m_answer = AP_Dialog_Break::a_CANCEL;
	gtk_main_quit();
#endif
}

void AP_QNXDialog_Break::event_WindowDelete(void)
{
#if 0
	m_answer = AP_Dialog_Break::a_CANCEL;	
	gtk_main_quit();
#endif
}

/*****************************************************************/
#if 0
GtkWidget * AP_QNXDialog_Break::_constructWindow(void)
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
	GtkWidget * hseparator9;
	GtkWidget * hseparator10;
	GtkWidget * hbuttonboxBreak;
	GtkWidget * buttonOK;
	GtkWidget * buttonCancel;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	windowBreak = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_object_set_data (GTK_OBJECT (windowBreak), "windowBreak", windowBreak);
	gtk_window_set_title (GTK_WINDOW (windowBreak), pSS->getValue(AP_STRING_ID_DLG_Break_BreakTitle));
	gtk_window_set_policy (GTK_WINDOW (windowBreak), FALSE, FALSE, FALSE);
	
	vboxMain = gtk_vbox_new (FALSE, 0);
	gtk_object_set_data (GTK_OBJECT (windowBreak), "vboxMain", vboxMain);
	gtk_widget_show (vboxMain);
	gtk_container_add (GTK_CONTAINER (windowBreak), vboxMain);
	gtk_container_set_border_width (GTK_CONTAINER (vboxMain), 10);

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
	
	hseparator9 = gtk_hseparator_new ();
	gtk_object_set_data (GTK_OBJECT (windowBreak), "hseparator9", hseparator9);
	gtk_widget_show (hseparator9);
	gtk_table_attach (GTK_TABLE (tableInsert), hseparator9, 0, 2, 6, 7,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 6);

	hseparator10 = gtk_hseparator_new ();
	gtk_object_set_data (GTK_OBJECT (windowBreak), "hseparator10", hseparator10);
	gtk_widget_show (hseparator10);
	gtk_table_attach (GTK_TABLE (tableInsert), hseparator10, 0, 2, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 6);

	hbuttonboxBreak = gtk_hbutton_box_new ();
	gtk_object_set_data (GTK_OBJECT (windowBreak), "hbuttonboxBreak", hbuttonboxBreak);
	gtk_widget_show (hbuttonboxBreak);
	gtk_box_pack_start (GTK_BOX (vboxMain), hbuttonboxBreak, FALSE, FALSE, 4);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonboxBreak), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonboxBreak), 10);
	gtk_button_box_set_child_size (GTK_BUTTON_BOX (hbuttonboxBreak), 85, 24);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (hbuttonboxBreak), 0, 0);

	buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_object_set_data (GTK_OBJECT (windowBreak), "buttonOK", buttonOK);
	gtk_widget_show (buttonOK);
	gtk_container_add (GTK_CONTAINER (hbuttonboxBreak), buttonOK);

	buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_object_set_data (GTK_OBJECT (windowBreak), "buttonCancel", buttonCancel);
	gtk_widget_show (buttonCancel);
	gtk_container_add (GTK_CONTAINER (hbuttonboxBreak), buttonCancel);

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
	
	gtk_signal_connect_after(GTK_OBJECT(windowBreak),
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

void AP_QNXDialog_Break::_populateWindowData(void)
{
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.

	GtkWidget * widget = _findRadioByID(m_break);
	UT_ASSERT(widget);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
}

void AP_QNXDialog_Break::_storeWindowData(void)
{
	m_break = _getActiveRadioItem();
}

// TODO if this function is useful elsewhere, move it to QNX dialog
// TODO helpers and standardize on a user-data tag for WIDGET_ID_TAG_KEY
GtkWidget * AP_QNXDialog_Break::_findRadioByID(AP_Dialog_Break::breakType b)
{
	UT_ASSERT(m_radioGroup);
	for (GSList * item = m_radioGroup ; item ; item = item->next)
	{
		if (GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(item->data), WIDGET_ID_TAG_KEY)) ==
			(int) b)
			return (GtkWidget *) item->data;
	}

	return NULL;

}

AP_Dialog_Break::breakType AP_QNXDialog_Break::_getActiveRadioItem(void)
{
	UT_ASSERT(m_radioGroup);
	for (GSList * item = m_radioGroup ; item ; item = item->next)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(item->data)))
		{
			return (AP_Dialog_Break::breakType)
				GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(item->data), WIDGET_ID_TAG_KEY));
		}
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return AP_Dialog_Break::b_PAGE;
}

#endif
