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
#include "xap_UnixDialogHelper.h"

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
	m_buttonFind = NULL;
	m_buttonFindReplace = NULL;
	m_buttonReplaceAll = NULL;
}

AP_UnixDialog_Replace::~AP_UnixDialog_Replace(void)
{
}

/*****************************************************************/

void AP_UnixDialog_Replace::s_response_triggered(GtkWidget * widget, gint resp, AP_UnixDialog_Replace * dlg)
{
  UT_DEBUGMSG(("DOM: %d response\n", resp));
	UT_return_if_fail(widget && dlg);

	if ( resp == BUTTON_FIND )
	  dlg->event_Find();
	else if ( resp == BUTTON_REPLACE)
	  dlg->event_Replace();
	else if ( resp == BUTTON_REPLACE_ALL)
	  dlg->event_ReplaceAll();
	else
	  abiDestroyWidget ( widget ) ; // will trigger other events
}

static void s_find_entry_activate(GtkWidget * widget, AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Find();
}

static void s_find_entry_change(GtkWidget * widget, AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_FindEntryChange();
}

static void s_replace_entry_activate(GtkWidget * widget, AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Replace();
}

static void s_match_case_toggled(GtkWidget * widget, AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_MatchCaseToggled();
}

static void s_destroy_clicked(GtkWidget * /* widget */,
			      AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_Cancel();
}

static void s_delete_clicked(GtkWidget * widget,
			     gpointer,
			     gpointer * dlg)
{
	abiDestroyWidget(widget);
}

/*****************************************************************/

void AP_UnixDialog_Replace::activate(void)
{
	UT_ASSERT(m_windowMain);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	gdk_window_raise(m_windowMain->window);
}


void AP_UnixDialog_Replace::notifyActiveFrame(XAP_Frame *pFrame)
{
	UT_ASSERT(m_windowMain);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
}

void AP_UnixDialog_Replace::runModeless(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	abiSetupModelessDialog (GTK_DIALOG(mainWindow), pFrame, this, BUTTON_CANCEL) ;

	// Populate the window's data items
	_populateWindowData();
	
	// this dialog needs this
	setView(static_cast<FV_View *> (getActiveFrame()->getCurrentView()));
}

void AP_UnixDialog_Replace::event_Find(void)
{
	char * findEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(m_entryFind));
	if (strlen(findEntryText) == 0) // do nothing when the find field is empty
		return;
	
	UT_UCSChar * findString;

	UT_UCS4_cloneString_char(&findString, findEntryText);
	
	setFindString(findString);
	
	findNext();

	FREEP(findString);
}

void AP_UnixDialog_Replace::event_FindEntryChange(void)
{
	const char *input = gtk_entry_get_text(GTK_ENTRY(m_entryFind));
	bool enable = strlen(input) != 0;
	gtk_widget_set_sensitive(m_buttonFind, enable);
	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		gtk_widget_set_sensitive(m_buttonFindReplace, enable);
		gtk_widget_set_sensitive(m_buttonReplaceAll, enable);
	}
}		

void AP_UnixDialog_Replace::event_Replace(void)
{
	char * findEntryText;
	char * replaceEntryText;

	findEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(m_entryFind));
	replaceEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(m_entryReplace));
	
	UT_UCSChar * findString;
	UT_UCSChar * replaceString;

	UT_UCS4_cloneString_char(&findString, findEntryText);
	UT_UCS4_cloneString_char(&replaceString, replaceEntryText);
	
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

	UT_UCS4_cloneString_char(&findString, findEntryText);
	UT_UCS4_cloneString_char(&replaceString, replaceEntryText);
	
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
	destroy();
}

void AP_UnixDialog_Replace::destroy(void)
{
	_storeWindowData();
	modeless_cleanup();
	abiDestroyWidget(m_windowMain);
	m_windowMain = NULL;
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_Replace::_constructWindow(void)
{
	GtkWidget *windowReplace;
	GtkWidget *vboxReplace;
	GtkWidget *tableReplace;
	GtkWidget *entryFind;
	GtkWidget *entryReplace = 0;
	GtkWidget *checkbuttonMatchCase;
	GtkWidget *labelFind;
	GtkWidget *labelReplace;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	ConstructWindowName();
	windowReplace = abiDialogNew ("find and replace dialog", TRUE, m_WindowName);
	gtk_window_set_default_size(GTK_WINDOW (windowReplace), 400, 100);

	// top level vbox
	vboxReplace = GTK_DIALOG(windowReplace)->vbox;
	gtk_widget_show (vboxReplace);

	// table up top
	tableReplace = gtk_table_new (3, 2, FALSE);
	gtk_widget_show (tableReplace);
	gtk_box_pack_start (GTK_BOX (vboxReplace), tableReplace, FALSE, TRUE, 0);

	// find label is always here
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_FR_FindLabel).c_str());
	labelFind = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_show (labelFind);
	gtk_table_attach (GTK_TABLE (tableReplace), labelFind, 0, 1, 0, 1,
			  GtkAttachOptions(GTK_FILL),
			  GtkAttachOptions(0), 5, 0);
	gtk_misc_set_alignment (GTK_MISC (labelFind), 0, 0.5);

	// find entry is always here
	entryFind = gtk_entry_new ();
	gtk_widget_show (entryFind);
	gtk_table_attach (GTK_TABLE (tableReplace), entryFind, 1, 2, 0, 1,
			  GtkAttachOptions(GTK_EXPAND | GTK_FILL),
			  GtkAttachOptions(GTK_EXPAND | GTK_FILL), 0, 0);

	// match case is always here
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_FR_MatchCase).c_str());	
	checkbuttonMatchCase = gtk_check_button_new_with_label (unixstr);
	FREEP(unixstr);
	gtk_widget_show (checkbuttonMatchCase);
	gtk_table_attach (GTK_TABLE (tableReplace), checkbuttonMatchCase, 1, 2, 1, 2,
			  GtkAttachOptions(GTK_FILL),
			  GtkAttachOptions(0), 0, 0);

	// the replace label and field are only visible if we're a "replace" dialog
	if (m_id == AP_DIALOG_ID_REPLACE)
	{	
		// create replace label
		UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_FR_ReplaceWithLabel).c_str());	
		labelReplace = gtk_label_new (unixstr);
		FREEP(unixstr);
		gtk_widget_show (labelReplace);
		gtk_table_attach (GTK_TABLE (tableReplace), labelReplace, 0, 1, 2, 3,
				  GtkAttachOptions(0), GtkAttachOptions(0), 5, 0);
		gtk_misc_set_alignment (GTK_MISC (labelReplace), 0, 0.5);

		// create replace entry
		entryReplace = gtk_entry_new ();
		gtk_widget_show (entryReplace);
		gtk_table_attach (GTK_TABLE (tableReplace), entryReplace, 1, 2, 2, 3,
				  GtkAttachOptions(GTK_EXPAND | GTK_FILL),
				  GtkAttachOptions(GTK_EXPAND | GTK_FILL), 0, 0);

	}
	
	abiAddStockButton(GTK_DIALOG(windowReplace), GTK_STOCK_CANCEL, BUTTON_CANCEL);
	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		m_buttonFindReplace = abiAddStockButton ( GTK_DIALOG(windowReplace), GTK_STOCK_FIND_AND_REPLACE, BUTTON_REPLACE ) ;
		gtk_widget_set_sensitive( m_buttonFindReplace, false );
		
		UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_FR_ReplaceAllButton).c_str());	
		m_buttonReplaceAll = abiAddButton ( GTK_DIALOG(windowReplace), unixstr, BUTTON_REPLACE_ALL );
		gtk_widget_set_sensitive( m_buttonReplaceAll, false );
		FREEP(unixstr);
	}

	// create and disable the find button initially
	m_buttonFind = abiAddStockButton (GTK_DIALOG(windowReplace), GTK_STOCK_FIND, BUTTON_FIND);
	gtk_widget_set_sensitive( m_buttonFind, false );

	g_signal_connect(G_OBJECT(windowReplace), "response", 
			 G_CALLBACK(s_response_triggered), this);

	// attach generic signals
	g_signal_connect(G_OBJECT(checkbuttonMatchCase),
			 "toggled",
			 G_CALLBACK(s_match_case_toggled),
			 this);

	// If the user hits "enter" in the entry field, we launch a find
	g_signal_connect(G_OBJECT(entryFind),
			 "activate",
			 G_CALLBACK(s_find_entry_activate),
			 this);

	g_signal_connect(G_OBJECT(entryFind),
			 "changed",
			 G_CALLBACK(s_find_entry_change),
			 (gpointer) this);


	// signals only useful in "replace mode"
	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		// If the user hits "enter" in the entry field, we launch a replace
		g_signal_connect(G_OBJECT(entryReplace),
				 "activate",
				 G_CALLBACK(s_replace_entry_activate),
				 this);
	}

	// save pointers to members
	m_windowMain = windowReplace;

	m_entryFind = entryFind;
	m_entryReplace = entryReplace;
	m_checkbuttonMatchCase = checkbuttonMatchCase;

	// the catch-alls
	// Dont use gtk_signal_connect_after for modeless dialogs
	gtk_signal_connect(GTK_OBJECT(m_windowMain),
			   "destroy",
			   GTK_SIGNAL_FUNC(s_destroy_clicked),
			   (gpointer) this);
	gtk_signal_connect(GTK_OBJECT(m_windowMain),
			   "delete_event",
			   GTK_SIGNAL_FUNC(s_delete_clicked),
			   (gpointer) this);

	return windowReplace;
}

void AP_UnixDialog_Replace::_populateWindowData(void)
{
	UT_ASSERT(m_entryFind && m_checkbuttonMatchCase);

	// last used find string
	{
		UT_UCSChar * bufferUnicode = getFindString();
		char * bufferNormal = (char *) UT_calloc(UT_UCS4_strlen(bufferUnicode) + 1, sizeof(char));
		UT_UCS4_strcpy_to_char(bufferNormal, bufferUnicode);
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
		char * bufferNormal = (char *) UT_calloc(UT_UCS4_strlen(bufferUnicode) + 1, sizeof(char));
		UT_UCS4_strcpy_to_char(bufferNormal, bufferUnicode);
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

