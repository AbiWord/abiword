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

//////////////////////////////////////////////////////////////////
// THIS CODE RUNS BOTH THE "Find" AND THE "Find-Replace" DIALOGS.
//////////////////////////////////////////////////////////////////

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
#include "ap_Dialog_Replace.h"
#include "ap_UnixDialog_Replace.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Replace::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	UT_DEBUGMSG(("AP_UnixDialog_Replace::static_constructor(...) I've been called\n"));

	AP_UnixDialog_Replace * p = new AP_UnixDialog_Replace(pFactory,id);
	return p;
}

AP_UnixDialog_Replace::AP_UnixDialog_Replace(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Replace(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_entryFind = NULL;
	m_entryReplace = NULL;
	m_checkbuttonMatchCase = NULL;

	m_buttonFindNext = NULL;
	m_buttonReplace = NULL;
	m_buttonReplaceAll = NULL;

	m_buttonCancel = NULL;
}

AP_UnixDialog_Replace::~AP_UnixDialog_Replace(void)
{
}

/*****************************************************************/

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
}

static void s_find_clicked(GtkWidget * widget, AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Find();
}

static void s_replace_clicked(GtkWidget * widget, AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Replace();
}

static void s_replace_all_clicked(GtkWidget * widget, AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_ReplaceAll();
}

static void s_cancel_clicked(GtkWidget * widget, AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
}

static void s_match_case_toggled(GtkWidget * widget, AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_MatchCaseToggled();
}

static void s_find_entry_activate(GtkWidget * widget, AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Find();
}

static void s_replace_entry_activate(GtkWidget * widget, AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Replace();
}

/*****************************************************************/

void AP_UnixDialog_Replace::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	
	// To center the dialog, we need the frame of its parent.
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(pUnixFrame);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);
	gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// this dialogs needs this
	setView(static_cast<FV_View *> (pFrame->getCurrentView()));

	// Run into the GTK event loop for this window.
	gtk_main();

	_storeWindowData();
	
	gtk_widget_destroy(mainWindow);
}

void AP_UnixDialog_Replace::event_Find(void)
{
	char * findEntryText;

	findEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(m_entryFind));
	
	UT_UCSChar * findString;

	UT_UCS_cloneString_char(&findString, findEntryText);
	
	setFindString(findString);
	
	findNext();

	FREEP(findString);
}
		
void AP_UnixDialog_Replace::event_Replace(void)
{
	char * findEntryText;
	char * replaceEntryText;

	findEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(m_entryFind));
	replaceEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(m_entryReplace));
	
	UT_UCSChar * findString;
	UT_UCSChar * replaceString;

	UT_UCS_cloneString_char(&findString, findEntryText);
	UT_UCS_cloneString_char(&replaceString, replaceEntryText);
	
	setFindString(findString);
	setReplaceString(replaceString);
	
	findReplace();

	FREEP(findString);
	FREEP(replaceString);
}

void AP_UnixDialog_Replace::event_ReplaceAll(void)
{
	char * findEntryText;
	char * replaceEntryText;

	findEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(m_entryFind));
	replaceEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(m_entryReplace));
	
	UT_UCSChar * findString;
	UT_UCSChar * replaceString;

	UT_UCS_cloneString_char(&findString, findEntryText);
	UT_UCS_cloneString_char(&replaceString, replaceEntryText);
	
	setFindString(findString);
	setReplaceString(replaceString);
	
	findReplaceAll();

	FREEP(findString);
	FREEP(replaceString);
}

void AP_UnixDialog_Replace::event_MatchCaseToggled(void)
{
	setMatchCase(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkbuttonMatchCase)));
}

void AP_UnixDialog_Replace::event_Cancel(void)
{
	m_answer = AP_Dialog_Replace::a_CANCEL;
	gtk_main_quit();
}

void AP_UnixDialog_Replace::event_WindowDelete(void)
{
	m_answer = AP_Dialog_Replace::a_CANCEL;	
	gtk_main_quit();
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_Replace::_constructWindow(void)
{
	GtkWidget *windowReplace;
	GtkWidget *vboxReplace;
	GtkWidget *tableReplace;
	GtkWidget *entryFind;
	GtkWidget *entryReplace;
	GtkWidget *checkbuttonMatchCase;
	GtkWidget *labelFind;
	GtkWidget *labelReplace;
	GtkWidget *hbuttonbox1;
	GtkWidget *buttonFindNext;
	GtkWidget *buttonReplace;
	GtkWidget *buttonReplaceAll;
	GtkWidget *buttonCancel;


	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	// conditionally set title
	if (m_id == AP_DIALOG_ID_FIND)
		UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_FindTitle));
	else
		UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceTitle));		
	windowReplace = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_object_set_data (GTK_OBJECT (windowReplace), "windowReplace", windowReplace);
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_Insert));
	gtk_window_set_title (GTK_WINDOW (windowReplace), "REPLACE THIS STRING");
	// find is smaller
	if (m_id == AP_DIALOG_ID_FIND)
		gtk_widget_set_usize (windowReplace, 390, 102);
	else
		gtk_widget_set_usize (windowReplace, 390, 123);
	
	FREEP(unixstr);
	gtk_window_set_policy (GTK_WINDOW (windowReplace), FALSE, FALSE, FALSE);

	// top level vbox
	vboxReplace = gtk_vbox_new (FALSE, 12);
	gtk_object_set_data (GTK_OBJECT (windowReplace), "vboxReplace", vboxReplace);
	gtk_widget_show (vboxReplace);
	gtk_container_add (GTK_CONTAINER (windowReplace), vboxReplace);
	gtk_container_set_border_width (GTK_CONTAINER (vboxReplace), 10);

	// table up top
	tableReplace = gtk_table_new (3, 2, FALSE);
	gtk_object_set_data (GTK_OBJECT (windowReplace), "tableReplace", tableReplace);
	gtk_widget_show (tableReplace);
	gtk_box_pack_start (GTK_BOX (vboxReplace), tableReplace, FALSE, TRUE, 0);

	// find label is always here
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_FindLabel));
	labelFind = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_object_set_data (GTK_OBJECT (windowReplace), "labelFind", labelFind);
	gtk_widget_show (labelFind);
	gtk_table_attach (GTK_TABLE (tableReplace), labelFind, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (labelFind), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelFind), 0, 0.5);

	// find entry is always here
	entryFind = gtk_entry_new ();
	gtk_object_set_data (GTK_OBJECT (windowReplace), "entryFind", entryFind);
	gtk_widget_show (entryFind);
	gtk_table_attach (GTK_TABLE (tableReplace), entryFind, 1, 2, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	// match case is always here
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_MatchCase));	
	checkbuttonMatchCase = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	gtk_object_set_data (GTK_OBJECT (windowReplace), "checkbuttonMatchCase", checkbuttonMatchCase);
	gtk_widget_show (checkbuttonMatchCase);
	gtk_table_attach (GTK_TABLE (tableReplace), checkbuttonMatchCase, 1, 2, 1, 2,
					  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

	// the replace label and field are only visible if we're a "replace" dialog
	if (m_id == AP_DIALOG_ID_REPLACE)
	{	
		// create replace label
		UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceWithLabel));	
		labelReplace = gtk_label_new (unixstr);
		FREEP(unixstr);
		gtk_object_set_data (GTK_OBJECT (windowReplace), "labelReplace", labelReplace);
		gtk_widget_show (labelReplace);
		gtk_table_attach (GTK_TABLE (tableReplace), labelReplace, 0, 1, 2, 3,
						  (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
						  (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
		gtk_label_set_justify (GTK_LABEL (labelReplace), GTK_JUSTIFY_LEFT);
		gtk_misc_set_alignment (GTK_MISC (labelReplace), 0, 0.5);

		// create replace entry
		entryReplace = gtk_entry_new ();
		gtk_object_set_data (GTK_OBJECT (windowReplace), "entryReplace", entryReplace);
		gtk_widget_show (entryReplace);
		gtk_table_attach (GTK_TABLE (tableReplace), entryReplace, 1, 2, 2, 3,
						  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	}
	
	// button box at the bottom
	hbuttonbox1 = gtk_hbutton_box_new ();
	gtk_object_set_data (GTK_OBJECT (windowReplace), "hbuttonbox1", hbuttonbox1);
	gtk_widget_show (hbuttonbox1);
	gtk_box_pack_start (GTK_BOX (vboxReplace), hbuttonbox1, FALSE, FALSE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox1), 10);
	gtk_button_box_set_child_size (GTK_BUTTON_BOX (hbuttonbox1), 85, 24);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_FindNextButton));	
	buttonFindNext = gtk_button_new_with_label (unixstr);
	FREEP(unixstr);
	gtk_object_set_data (GTK_OBJECT (windowReplace), "buttonFindNext", buttonFindNext);
	gtk_widget_show (buttonFindNext);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), buttonFindNext);

	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceButton));	
		buttonReplace = gtk_button_new_with_label (unixstr);
		FREEP(unixstr);
		gtk_object_set_data (GTK_OBJECT (windowReplace), "buttonReplace", buttonReplace);
		gtk_widget_show (buttonReplace);
		gtk_container_add (GTK_CONTAINER (hbuttonbox1), buttonReplace);

		UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceAllButton));	
		buttonReplaceAll = gtk_button_new_with_label (unixstr);
		FREEP(unixstr);
		gtk_object_set_data (GTK_OBJECT (windowReplace), "buttonReplaceAll", buttonReplaceAll);
		gtk_widget_show (buttonReplaceAll);
		gtk_container_add (GTK_CONTAINER (hbuttonbox1), buttonReplaceAll);
	}

	buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_object_set_data (GTK_OBJECT (windowReplace), "buttonCancel", buttonCancel);
	gtk_widget_show (buttonCancel);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), buttonCancel);

	// Configuration of the title depending on the ID
	if (m_id == AP_DIALOG_ID_FIND)
		gtk_window_set_title(GTK_WINDOW(windowReplace), pSS->getValue(AP_STRING_ID_DLG_FR_FindTitle));
	else
		gtk_window_set_title(GTK_WINDOW(windowReplace), pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceTitle));

	// attach generic signals
	gtk_signal_connect(GTK_OBJECT(checkbuttonMatchCase),
					   "toggled",
					   GTK_SIGNAL_FUNC(s_match_case_toggled),
					   this);

	// If the user hits "enter" in the entry field, we launch a find
	gtk_signal_connect(GTK_OBJECT(entryFind),
					   "activate",
					   GTK_SIGNAL_FUNC(s_find_entry_activate),
					   this);

	// Buttons
	gtk_signal_connect(GTK_OBJECT(buttonFindNext),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_find_clicked),
					   this);
	
	gtk_signal_connect(GTK_OBJECT(buttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   this);

	// Window events
	gtk_signal_connect_after(GTK_OBJECT(windowReplace),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(windowReplace),
							 "destroy",
							 NULL,
							 NULL);

	// signals only useful in "replace mode"
	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		// If the user hits "enter" in the entry field, we launch a replace
		gtk_signal_connect(GTK_OBJECT(entryReplace),
						   "activate",
						   GTK_SIGNAL_FUNC(s_replace_entry_activate),
						   this);

		// Buttons
		gtk_signal_connect(GTK_OBJECT(buttonReplace),
						   "clicked",
						   GTK_SIGNAL_FUNC(s_replace_clicked),
						   this);

		gtk_signal_connect(GTK_OBJECT(buttonReplaceAll),
						   "clicked",
						   GTK_SIGNAL_FUNC(s_replace_all_clicked),
						   this);
	}

	// save pointers to members
	m_windowMain = windowReplace;

	m_entryFind = entryFind;
	m_entryReplace = entryReplace;
	m_checkbuttonMatchCase = checkbuttonMatchCase;

	m_buttonFindNext = buttonFindNext;
	m_buttonReplace = buttonReplace;
	m_buttonReplaceAll = buttonReplaceAll;
	m_buttonCancel = buttonCancel;

	m_buttonCancel = buttonCancel;

	
	return windowReplace;
}

void AP_UnixDialog_Replace::_populateWindowData(void)
{
	UT_ASSERT(m_entryFind && m_checkbuttonMatchCase);

	// last used find string
	{
		UT_UCSChar * bufferUnicode = getFindString();
		char * bufferNormal = (char *) calloc(UT_UCS_strlen(bufferUnicode) + 1, sizeof(char));
		UT_UCS_strcpy_to_char(bufferNormal, bufferUnicode);
		FREEP(bufferUnicode);
		
		gtk_entry_set_text(GTK_ENTRY(m_entryFind), bufferNormal);
		gtk_entry_select_region(GTK_ENTRY(m_entryFind), 0, -1);

		FREEP(bufferNormal);
	}
	
	
	// last used replace string
	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		UT_ASSERT(m_entryReplace);
		
		UT_UCSChar * bufferUnicode = getReplaceString();
		char * bufferNormal = (char *) calloc(UT_UCS_strlen(bufferUnicode) + 1, sizeof(char));
		UT_UCS_strcpy_to_char(bufferNormal, bufferUnicode);
		FREEP(bufferUnicode);
		
		gtk_entry_set_text(GTK_ENTRY(m_entryReplace), bufferNormal);

		FREEP(bufferNormal);
	}

	// match case button
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkbuttonMatchCase), getMatchCase());

	// Find entry should have focus, for immediate typing
	gtk_widget_grab_focus(m_entryFind);	
}

void AP_UnixDialog_Replace::_storeWindowData(void)
{
	// TODO: nothing?  The actual methods store
	// out last used data to the persist variables,
	// since we need to save state when things actually
	// happen (not when the dialog closes).
}

