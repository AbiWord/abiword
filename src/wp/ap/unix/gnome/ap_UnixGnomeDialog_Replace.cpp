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
#include "ap_UnixGnomeDialog_Replace.h"

#include <gnome.h>

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixGnomeDialog_Replace::static_constructor(XAP_DialogFactory * pFactory,
															XAP_Dialog_Id id)
{
	UT_DEBUGMSG(("AP_UnixGnomeDialog_Replace::static_constructor(...) I've been called\n"));

	AP_UnixGnomeDialog_Replace * p = new AP_UnixGnomeDialog_Replace(pFactory,id);
	return p;
}

AP_UnixGnomeDialog_Replace::AP_UnixGnomeDialog_Replace(XAP_DialogFactory * pDlgFactory,
													   XAP_Dialog_Id id)
	: AP_UnixDialog_Replace(pDlgFactory,id)
{
}

AP_UnixGnomeDialog_Replace::~AP_UnixGnomeDialog_Replace(void)
{
}

/*****************************************************************/

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 AP_UnixGnomeDialog_Replace * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
}

static void s_find_clicked(GtkWidget * widget, AP_UnixGnomeDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Find();
}

static void s_replace_clicked(GtkWidget * widget, AP_UnixGnomeDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Replace();
}

static void s_replace_all_clicked(GtkWidget * widget, AP_UnixGnomeDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_ReplaceAll();
}

static void s_cancel_clicked(GtkWidget * widget, AP_UnixGnomeDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
}

static void s_match_case_toggled(GtkWidget * widget, AP_UnixGnomeDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_MatchCaseToggled();
}

static void s_find_entry_activate(GtkWidget * widget, AP_UnixGnomeDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Find();
}

static void s_replace_entry_activate(GtkWidget * widget, AP_UnixGnomeDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Replace();
}

/*****************************************************************/

void AP_UnixGnomeDialog_Replace::runModeless(XAP_Frame * pFrame)
{

	// get the Dialog Id number
	UT_sint32 sid =(UT_sint32)  getDialogId();

	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Save dialog the ID number and pointer to the Dialog
	m_pApp->rememberModelessId( sid,  (XAP_Dialog_Modeless *) m_pDialog);

	// This magic command displays the frame where strings will be found
	connectFocusModeless(GTK_WIDGET(mainWindow),m_pApp);

	// Populate the window's data items
	_populateWindowData();
	
	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// this dialog needs this
	setView(static_cast<FV_View *> (getActiveFrame()->getCurrentView()));
}

GtkWidget * AP_UnixGnomeDialog_Replace::_constructWindow(void)
{
	GtkWidget *windowReplace;
	GtkWidget *vboxReplace;
	GtkWidget *tableReplace;
	GtkWidget *entryFind;
	GtkWidget *entryReplace;
	GtkWidget *checkbuttonMatchCase;
	GtkWidget *labelFind;
	GtkWidget *labelReplace;
	GtkWidget *buttonFindNext;
	GtkWidget *buttonReplace;
	GtkWidget *buttonReplaceAll;
	GtkWidget *buttonCancel;
	GtkWidget *pixmap;


	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	// conditionally set title

	ConstructWindowName();
	windowReplace = gnome_dialog_new ( m_WindowName, NULL);
	gtk_object_set_data (GTK_OBJECT (windowReplace), "windowReplace", windowReplace);

	// find is smaller
//  	if (m_id == AP_DIALOG_ID_FIND)
//  		gtk_widget_set_usize (windowReplace, 390, 102);
//  	else
//  		gtk_widget_set_usize (windowReplace, 390, 123);
	
	gtk_window_set_policy (GTK_WINDOW (windowReplace), FALSE, FALSE, FALSE);

	// top level vbox
  	vboxReplace = GNOME_DIALOG (windowReplace)->vbox;
  	gtk_object_set_data (GTK_OBJECT (windowReplace), "vboxReplace", vboxReplace);

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
	
   	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_FindNextButton));	
   	gnome_dialog_append_button_with_pixmap (GNOME_DIALOG (windowReplace), unixstr, GNOME_STOCK_PIXMAP_SEARCH);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (GNOME_DIALOG (windowReplace)->action_area), 10);
   	FREEP(unixstr);
   	buttonFindNext = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowReplace)->buttons)->data);
   	gtk_object_set_data (GTK_OBJECT (windowReplace), "buttonFindNext", buttonFindNext);

	if (m_id == AP_DIALOG_ID_REPLACE)
	{
  		UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceButton));	
		pixmap = gnome_stock_pixmap_widget_at_size (windowReplace, GNOME_STOCK_PIXMAP_SEARCH, 12, 14);
  		gnome_dialog_append_button_with_pixmap (GNOME_DIALOG (windowReplace), unixstr, GNOME_STOCK_PIXMAP_SRCHRPL);
  		FREEP(unixstr);
  		buttonReplace = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowReplace)->buttons)->data);
  		gtk_object_set_data (GTK_OBJECT (windowReplace), "buttonReplace", buttonReplace);

  		UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceAllButton));	
  		gnome_dialog_append_button_with_pixmap (GNOME_DIALOG (windowReplace), unixstr, GNOME_STOCK_PIXMAP_SRCHRPL);
  		FREEP(unixstr);
  		buttonReplaceAll = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowReplace)->buttons)->data);
  		gtk_object_set_data (GTK_OBJECT (windowReplace), "buttonReplaceAll", buttonReplaceAll);
	}

	//  	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Cancel));	
  	gnome_dialog_append_button (GNOME_DIALOG (windowReplace), GNOME_STOCK_BUTTON_CANCEL);
	//  	FREEP(unixstr);
  	buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowReplace)->buttons)->data);
	gtk_object_set_data (GTK_OBJECT (windowReplace), "buttonCancel", buttonCancel);
	GTK_WIDGET_SET_FLAGS (buttonCancel, GTK_CAN_DEFAULT);

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
	//	gtk_signal_connect_after(GTK_OBJECT(windowReplace),
	gtk_signal_connect(GTK_OBJECT(windowReplace),
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

