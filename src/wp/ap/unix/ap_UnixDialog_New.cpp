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
#include "ap_Dialog_New.h"
#include "ap_UnixDialog_New.h"

#include "xap_Dlg_FileOpenSaveAs.h"
#include "ie_imp.h"

#define IEFT_AbiWord_1 IE_Imp::fileTypeForSuffix(".abw")

/*************************************************************************/

XAP_Dialog * AP_UnixDialog_New::static_constructor(XAP_DialogFactory * pFactory,
												   XAP_Dialog_Id id)
{
	AP_UnixDialog_New * p = new AP_UnixDialog_New(pFactory,id);
	return p;
}

AP_UnixDialog_New::AP_UnixDialog_New(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_New(pDlgFactory,id), m_pFrame(0)
{
}

AP_UnixDialog_New::~AP_UnixDialog_New(void)
{
}

void AP_UnixDialog_New::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	m_pFrame = pFrame;
	
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);
	
	connectFocus(GTK_WIDGET(mainWindow),pFrame);

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
	
	if(mainWindow && GTK_IS_WIDGET(mainWindow))
		gtk_widget_destroy(mainWindow);
}

/*************************************************************************/
/*************************************************************************/

void AP_UnixDialog_New::event_Ok ()
{
	setAnswer (AP_Dialog_New::a_OK);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioExisting)))
	{
		setOpenType(AP_Dialog_New::open_Existing);
	}
	else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioNew)))
	{
		setOpenType(AP_Dialog_New::open_Template);
	}
	else
	{
		setOpenType(AP_Dialog_New::open_New);
	}

	gtk_main_quit();
}

void AP_UnixDialog_New::event_Cancel ()
{
	setAnswer (AP_Dialog_New::a_CANCEL);
	gtk_main_quit();
}

void AP_UnixDialog_New::event_ToggleUseTemplate (const char * name)
{
	setTemplateName (name);
}

void AP_UnixDialog_New::event_ToggleOpenExisting ()
{
	XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_OPEN;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) m_pFrame->getDialogFactory();

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	pDialog->setCurrentPathname(0);
	pDialog->setSuggestFilename(false);

	UT_uint32 filterCount = IE_Imp::getImporterCount();
	const char ** szDescList = (const char **) calloc(filterCount + 1,
													  sizeof(char *));
	const char ** szSuffixList = (const char **) calloc(filterCount + 1,
														sizeof(char *));
	IEFileType * nTypeList = (IEFileType *) calloc(filterCount + 1,
												   sizeof(IEFileType));
	UT_uint32 k = 0;

	while (IE_Imp::enumerateDlgLabels(k, &szDescList[k], 
									  &szSuffixList[k], &nTypeList[k]))
			k++;

	pDialog->setFileTypeList(szDescList, szSuffixList, 
							 (const UT_sint32 *) nTypeList);

	pDialog->setDefaultFileType(IEFT_AbiWord_1);

	pDialog->runModal(m_pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const char * szResultPathname = pDialog->getPathname();
		if (szResultPathname && *szResultPathname)
		{
			// update the entry box
			gtk_entry_set_text (GTK_ENTRY(m_entryFilename), szResultPathname);
			setFileName (szResultPathname);
		}
	}

	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);
	
	pDialogFactory->releaseDialog(pDialog);
}

void AP_UnixDialog_New::event_ToggleStartNew ()
{	
	// nada
}

/*************************************************************************/
/*************************************************************************/

void s_ok_clicked (GtkWidget *, AP_UnixDialog_New * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_Ok();
}

void s_cancel_clicked (GtkWidget *, AP_UnixDialog_New * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_Cancel();
}

void s_window_delete (GtkWidget *, gpointer, AP_UnixDialog_New * dlg)
{
	s_cancel_clicked (0, dlg);
}

void s_choose_clicked (GtkWidget * w, AP_UnixDialog_New * dlg)
{
	dlg->event_ToggleOpenExisting();
}

/*************************************************************************/
/*************************************************************************/

GtkWidget * AP_UnixDialog_New::_constructWindow ()
{
	GtkWidget *mainWindow;
	GtkWidget *dialog_vbox1;
	GtkWidget *hbuttonbox1;
	GtkWidget *dialog_action_area1;
	GtkWidget *ok_btn;
	GtkWidget *cancel_btn;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	mainWindow = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (mainWindow), pSS->getValue(AP_STRING_ID_DLG_NEW_Title));
	gtk_window_set_policy (GTK_WINDOW (mainWindow), TRUE, TRUE, FALSE);

	dialog_vbox1 = GTK_DIALOG (mainWindow)->vbox;
	gtk_widget_show (dialog_vbox1);

	dialog_action_area1 = GTK_DIALOG (mainWindow)->action_area;
	gtk_widget_show (dialog_action_area1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 10);

	hbuttonbox1 = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox1);
	gtk_box_pack_start (GTK_BOX (dialog_action_area1), hbuttonbox1, TRUE, 
						TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), 
							   GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox1), 0);

	ok_btn = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_widget_show (ok_btn);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), ok_btn);
	GTK_WIDGET_SET_FLAGS (ok_btn, GTK_CAN_DEFAULT);

	cancel_btn = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_widget_show (cancel_btn);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), cancel_btn);
	GTK_WIDGET_SET_FLAGS (cancel_btn, GTK_CAN_DEFAULT);

	// assign pointers to widgets
	m_mainWindow   = mainWindow;
	m_buttonOk     = ok_btn;
	m_buttonCancel = cancel_btn;
	
	// construct the window contents
	_constructWindowContents (dialog_vbox1);
	_connectSignals ();

	return mainWindow;
}

void AP_UnixDialog_New::_constructWindowContents (GtkWidget * container)
{
	GtkWidget *vbox1;
	GtkWidget *hbox1;

	GtkWidget *radio_new;
	GtkWidget *radio_existing;
	GtkWidget *radio_empty;
	GSList    *vbox1_group = NULL;

	GtkWidget *notebook_choices;
	GtkWidget *scrolledWindow;

	GtkWidget *label1;

	GtkWidget *hseparator1;
	GtkWidget *hseparator2;
	GtkWidget *hseparator3;

	GtkWidget *entry_filename;
	GtkWidget *choose_btn;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	vbox1 = gtk_vbox_new (FALSE, 10);
	gtk_widget_show (vbox1);
	gtk_box_pack_start (GTK_BOX (container), vbox1, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), 6);

	hseparator1 = gtk_hseparator_new ();
	gtk_widget_show (hseparator1);
	gtk_box_pack_start (GTK_BOX (vbox1), hseparator1, TRUE, TRUE, 0);

	radio_new = gtk_radio_button_new_with_label (vbox1_group, pSS->getValue(AP_STRING_ID_DLG_NEW_Create));
	vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio_new));
	gtk_widget_show (radio_new);
	gtk_box_pack_start (GTK_BOX (vbox1), radio_new, FALSE, FALSE, 0);

	notebook_choices = gtk_notebook_new ();
	gtk_widget_show (notebook_choices);
	gtk_box_pack_start (GTK_BOX (vbox1), notebook_choices, TRUE, TRUE, 0);
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook_choices), TRUE);

	scrolledWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledWindow);
	gtk_container_add (GTK_CONTAINER (notebook_choices), scrolledWindow);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledWindow), 
									GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);

	for (UT_uint32 i = 0; i < getNumTabs(); i++)
	{
		const TemplateData * td = getListForTab (i+1);
		
		// todo: populate the notebook with this data
	
		label1 = gtk_label_new ((const char *)getTabName(i+1));
		gtk_widget_show (label1);
		gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_choices), 
									gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_choices), i), label1);
	}
	
	hseparator2 = gtk_hseparator_new ();
	gtk_widget_show (hseparator2);
	gtk_box_pack_start (GTK_BOX (vbox1), hseparator2, TRUE, TRUE, 0);

	radio_existing = gtk_radio_button_new_with_label (vbox1_group, pSS->getValue(AP_STRING_ID_DLG_NEW_Open));
	vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio_existing));
	gtk_widget_show (radio_existing);
	gtk_box_pack_start (GTK_BOX (vbox1), radio_existing, FALSE, FALSE, 0);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

	entry_filename = gtk_entry_new ();
	gtk_widget_show (entry_filename);
	gtk_box_pack_start (GTK_BOX (hbox1), entry_filename, TRUE, TRUE, 0);
	gtk_entry_set_editable (GTK_ENTRY (entry_filename), FALSE);
	gtk_entry_set_text (GTK_ENTRY (entry_filename), pSS->getValue(AP_STRING_ID_DLG_NEW_NoFile));

	choose_btn = gtk_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_NEW_Choose));
	gtk_widget_show (choose_btn);
	gtk_box_pack_start (GTK_BOX (hbox1), choose_btn, FALSE, FALSE, 0);

	hseparator3 = gtk_hseparator_new ();
	gtk_widget_show (hseparator3);
	gtk_box_pack_start (GTK_BOX (vbox1), hseparator3, TRUE, TRUE, 0);

	radio_empty = gtk_radio_button_new_with_label (vbox1_group, 
												   pSS->getValue(AP_STRING_ID_DLG_NEW_StartEmpty));
	vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio_empty));
	gtk_widget_show (radio_empty);
	gtk_box_pack_start (GTK_BOX (vbox1), radio_empty, FALSE, FALSE, 0);

	// make this one the default
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_empty), TRUE);

	// connect signals
	gtk_signal_connect (GTK_OBJECT(choose_btn), 
						"clicked",
						GTK_SIGNAL_FUNC(s_choose_clicked), 
						(gpointer)this);

	// set the private pointers
	m_radioNew      = radio_new;
	m_radioExisting = radio_existing;
	m_radioEmpty    = radio_empty;
	m_entryFilename = entry_filename;
}

void AP_UnixDialog_New::_connectSignals ()
{
  	// the control buttons
	gtk_signal_connect(GTK_OBJECT(m_buttonOk),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(m_buttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (gpointer) this);
	
	// the catch-alls
	
	gtk_signal_connect(GTK_OBJECT(m_mainWindow),
					   "delete_event",
					   GTK_SIGNAL_FUNC(s_window_delete),
					   (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(m_mainWindow),
							 "destroy",
							 NULL,
							 NULL);
}
