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

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Columns.h"
#include "ap_QNXDialog_Columns.h"
#include "ut_qnxHelper.h"

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Columns::static_constructor(XAP_DialogFactory * pFactory,
						   XAP_Dialog_Id id)
{
	AP_QNXDialog_Columns * p = new AP_QNXDialog_Columns(pFactory,id);
	return p;
}

AP_QNXDialog_Columns::AP_QNXDialog_Columns(XAP_DialogFactory * pDlgFactory,
					 XAP_Dialog_Id id)
	: AP_Dialog_Columns(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_buttonOK = NULL;
	m_buttonCancel = NULL;

//	m_radioGroup = NULL;
}

AP_QNXDialog_Columns::~AP_QNXDialog_Columns(void)
{
}

/*****************************************************************/

static int s_ok_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Columns * dlg = (AP_QNXDialog_Columns *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
	return Pt_CONTINUE;
}

static int s_cancel_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Columns * dlg = (AP_QNXDialog_Columns *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Columns * dlg = (AP_QNXDialog_Columns *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}

/*****************************************************************/

void AP_QNXDialog_Columns::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(mainWindow,pFrame);
	// Populate the window's data items
	_populateWindowData();
	
	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrame * pQNXFrame = (XAP_QNXFrame *)(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the Window of the parent frame
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);

	// Show the top level dialog,
	PtRealizeWidget(mainWindow);
	
        UT_QNXBlockWidget(parentWindow, 0);

        int count = PtModalStart();
        done = 0;
        while(!done) {
                PtProcessEvent();
        }
        PtModalEnd(MODAL_END_ARG(count));

	_storeWindowData();

        UT_QNXBlockWidget(parentWindow, 0);
        PtDestroyWidget(mainWindow);
}

void AP_QNXDialog_Columns::event_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_Columns::a_OK;
	done++;
}

void AP_QNXDialog_Columns::event_Cancel(void)
{
	m_answer = AP_Dialog_Columns::a_CANCEL;
	done++;
}

void AP_QNXDialog_Columns::event_WindowDelete(void)
{
	if (!done++) {
		m_answer = AP_Dialog_Columns::a_CANCEL;	
	}
}

/*****************************************************************/

PtWidget_t * AP_QNXDialog_Columns::_constructWindow(void)
{
#if 0
	PtWidget_t * windowColumns;
	PtWidget_t * vboxMain;
	PtWidget_t * tableInsert;
	PtWidget_t * labelInsert;
	GSList * tableInsert_group = NULL;
	PtWidget_t * radiobuttonPageBreak;
	PtWidget_t * radiobuttonNextPage;
	PtWidget_t * radiobuttonContinuous;
	PtWidget_t * radiobuttonColumnBreak;
	PtWidget_t * radiobuttonEvenPage;
	PtWidget_t * radiobuttonOddPage;
	PtWidget_t * labelSectionBreaks;
	PtWidget_t * hseparator9;
	PtWidget_t * hseparator10;
	PtWidget_t * hbuttonboxBreak;
	PtWidget_t * buttonOK;
	PtWidget_t * buttonCancel;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	windowColumns = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_object_set_data (GTK_OBJECT (windowColumns), "windowColumns", windowColumns);
	gtk_window_set_title (GTK_WINDOW (windowColumns), pSS->getValue(AP_STRING_ID_DLG_Column_ColumnTitle));
	gtk_window_set_policy (GTK_WINDOW (windowColumns), FALSE, FALSE, FALSE);

	
	vboxMain = gtk_vbox_new (FALSE, 0);
	gtk_object_set_data (GTK_OBJECT (windowColumns), "vboxMain", vboxMain);
	gtk_widget_show (vboxMain);
	gtk_container_add (GTK_CONTAINER (windowColumns), vboxMain);
	gtk_container_set_border_width (GTK_CONTAINER (vboxMain), 10);
/*

	tableInsert = gtk_table_new (7, 2, FALSE);
	gtk_object_set_data (GTK_OBJECT (windowColumns), "tableInsert", tableInsert);
	gtk_widget_show (tableInsert);
	gtk_box_pack_start (GTK_BOX (vboxMain), tableInsert, FALSE, FALSE, 0);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_Insert));
	labelInsert = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_object_set_data (GTK_OBJECT (windowColumns), "labelInsert", labelInsert);
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
	gtk_object_set_data (GTK_OBJECT (windowColumns), "radiobuttonPageBreak", radiobuttonPageBreak);
//	gtk_object_set_data (GTK_OBJECT (radiobuttonPageBreak), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_PAGE));
	gtk_widget_show (radiobuttonPageBreak);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonPageBreak, 0, 1, 1, 2,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_NextPage));	
	radiobuttonNextPage = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonNextPage));
	gtk_object_set_data (GTK_OBJECT (windowColumns), "radiobuttonNextPage", radiobuttonNextPage);
//	gtk_object_set_data (GTK_OBJECT (radiobuttonNextPage), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_NEXTPAGE));
	gtk_widget_show (radiobuttonNextPage);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonNextPage, 0, 1, 4, 5,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_Continuous));	
	radiobuttonContinuous = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonContinuous));
	gtk_object_set_data (GTK_OBJECT (windowColumns), "radiobuttonContinuous", radiobuttonContinuous);
//	gtk_object_set_data (GTK_OBJECT (radiobuttonContinuous), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_CONTINUOUS));
	gtk_widget_show (radiobuttonContinuous);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonContinuous, 0, 1, 5, 6,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_ColumnBreak));	
	radiobuttonColumnBreak = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonColumnBreak));
	gtk_object_set_data (GTK_OBJECT (windowColumns), "radiobuttonColumnBreak", radiobuttonColumnBreak);
//	gtk_object_set_data (GTK_OBJECT (radiobuttonColumnBreak), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_COLUMN));
	gtk_widget_show (radiobuttonColumnBreak);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonColumnBreak, 1, 2, 1, 2,
					  (GtkAttachOptions) GTK_FILL, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_EvenPage));	
	radiobuttonEvenPage = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonEvenPage));
	gtk_object_set_data (GTK_OBJECT (windowColumns), "radiobuttonEvenPage", radiobuttonEvenPage);
//	gtk_object_set_data (GTK_OBJECT (radiobuttonEvenPage), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_EVENPAGE));
	gtk_widget_show (radiobuttonEvenPage);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonEvenPage, 1, 2, 4, 5,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_OddPage));	
	radiobuttonOddPage = gtk_radio_button_new_with_label (tableInsert_group, unixstr);
	FREEP(unixstr);
	tableInsert_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonOddPage));
	gtk_object_set_data (GTK_OBJECT (windowColumns), "radiobuttonOddPage", radiobuttonOddPage);
//	gtk_object_set_data (GTK_OBJECT (radiobuttonOddPage), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(b_ODDPAGE));
	gtk_widget_show (radiobuttonOddPage);
	gtk_table_attach (GTK_TABLE (tableInsert), radiobuttonOddPage, 1, 2, 5, 6,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_SectionBreaks));	
	labelSectionBreaks = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_object_set_data (GTK_OBJECT (windowColumns), "labelSectionBreaks", labelSectionBreaks);
	gtk_widget_show (labelSectionBreaks);
	gtk_table_attach (GTK_TABLE (tableInsert), labelSectionBreaks, 0, 1, 3, 4,
					  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 4);
	gtk_widget_set_usize (labelSectionBreaks, 76, -1);
	gtk_label_set_justify (GTK_LABEL (labelSectionBreaks), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelSectionBreaks), 0, 0.5);
	
	hseparator9 = gtk_hseparator_new ();
	gtk_object_set_data (GTK_OBJECT (windowColumns), "hseparator9", hseparator9);
	gtk_widget_show (hseparator9);
	gtk_table_attach (GTK_TABLE (tableInsert), hseparator9, 0, 2, 6, 7,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 6);

	hseparator10 = gtk_hseparator_new ();
	gtk_object_set_data (GTK_OBJECT (windowColumns), "hseparator10", hseparator10);
	gtk_widget_show (hseparator10);
	gtk_table_attach (GTK_TABLE (tableInsert), hseparator10, 0, 2, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 6);
*/
	hbuttonboxBreak = gtk_hbutton_box_new ();
	gtk_object_set_data (GTK_OBJECT (windowColumns), "hbuttonboxBreak", hbuttonboxBreak);
	gtk_widget_show (hbuttonboxBreak);
	gtk_box_pack_start (GTK_BOX (vboxMain), hbuttonboxBreak, FALSE, FALSE, 4);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonboxBreak), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonboxBreak), 10);
	gtk_button_box_set_child_size (GTK_BUTTON_BOX (hbuttonboxBreak), 85, 24);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (hbuttonboxBreak), 0, 0);


	buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_object_set_data (GTK_OBJECT (windowColumns), "buttonOK", buttonOK);
	gtk_widget_show (buttonOK);
	gtk_container_add (GTK_CONTAINER (hbuttonboxBreak), buttonOK);

	buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_object_set_data (GTK_OBJECT (windowColumns), "buttonCancel", buttonCancel);
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
	
	gtk_signal_connect_after(GTK_OBJECT(windowColumns),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(windowColumns),
							 "destroy",
							 NULL,
							 NULL);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowColumns;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

	m_radioGroup = tableInsert_group;
	
	return windowColumns;
#else
	return NULL;
#endif
}

void AP_QNXDialog_Columns::_populateWindowData(void)
{
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.

//	PtWidget_t * widget = _findRadioByID(m_break);
//	UT_ASSERT(widget);
	
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
}

void AP_QNXDialog_Columns::_storeWindowData(void)
{
//	m_break = _getActiveRadioItem();
}

void AP_QNXDialog_Columns::enableLineBetweenControl(UT_Bool bState)
{
}
