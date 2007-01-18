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

// for gtk_list - part of gtkcombo
#undef GTK_DISABLE_DEPRECATED

//////////////////////////////////////////////////////////////////
// THIS CODE RUNS BOTH THE "Find" AND THE "Find-Replace" DIALOGS.
//////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Replace.h"
#include "ap_UnixDialog_Replace.h"

XAP_Dialog * AP_UnixDialog_Replace::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	return new AP_UnixDialog_Replace(pFactory,id);
}

AP_UnixDialog_Replace::AP_UnixDialog_Replace(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Replace(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_comboFind = NULL;
	m_comboReplace = NULL;
	m_checkbuttonMatchCase = NULL;
	m_checkbuttonWholeWord = NULL;
	m_checkbuttonReverseFind = NULL;
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

static void s_whole_word_toggled(GtkWidget * widget, AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_WholeWordToggled();
}

static void s_reverse_find_toggled(GtkWidget * widget, AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_ReverseFindToggled();
}

static void s_destroy_clicked(GtkWidget * /* widget */,
			      AP_UnixDialog_Replace * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_Cancel();
}

static void s_find_clicked(GtkWidget *btn, GtkWidget * dlg)
{
	gtk_dialog_response (GTK_DIALOG(dlg), AP_UnixDialog_Replace::BUTTON_FIND);
}

static void s_findreplace_clicked(GtkWidget *btn, GtkWidget * dlg)
{
	gtk_dialog_response (GTK_DIALOG(dlg), AP_UnixDialog_Replace::BUTTON_REPLACE);
}

static void s_replaceall_clicked(GtkWidget *btn, GtkWidget * dlg)
{
	gtk_dialog_response (GTK_DIALOG(dlg), AP_UnixDialog_Replace::BUTTON_REPLACE_ALL);
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
	char * findEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(m_comboFind)->entry));
	if (strlen(findEntryText) == 0) // do nothing when the find field is empty
		return;

	// utf8->ucs4
	setFindString(UT_UCS4String(findEntryText).ucs4_str());

	char * replaceEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(m_comboReplace)->entry));
	setReplaceString(UT_UCS4String(replaceEntryText).ucs4_str());

	if (!getReverseFind())	
		findNext();
	else
		findPrev();
}

void AP_UnixDialog_Replace::event_FindEntryChange(void)
{
	const char *input = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(m_comboFind)->entry));
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

	findEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(m_comboFind)->entry));
	replaceEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(m_comboReplace)->entry));
	
	setFindString(UT_UCS4String(findEntryText).ucs4_str());
	setReplaceString(UT_UCS4String(replaceEntryText).ucs4_str());

	if(!getReverseFind())	
		findReplace();
	else
		findReplaceReverse();
}

void AP_UnixDialog_Replace::event_ReplaceAll(void)
{
	char * findEntryText;
	char * replaceEntryText;

	findEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(m_comboFind)->entry));
	replaceEntryText = (char *) gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(m_comboReplace)->entry));
	
	setFindString(UT_UCS4String(findEntryText).ucs4_str());
	setReplaceString(UT_UCS4String(replaceEntryText).ucs4_str());
	
	findReplaceAll();
}

void AP_UnixDialog_Replace::event_MatchCaseToggled(void)
{
	setMatchCase(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkbuttonMatchCase)));
}

void AP_UnixDialog_Replace::event_WholeWordToggled(void)
{
	setWholeWord(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkbuttonWholeWord)));
}

void AP_UnixDialog_Replace::event_ReverseFindToggled(void)
{
	setReverseFind(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkbuttonReverseFind)));
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
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	char * unixstr;

	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_Replace.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML(glade_path.c_str());
	if (!xml)
		return NULL;
	
	m_windowMain = glade_xml_get_widget(xml,"ap_UnixDialog_Replace");
	m_buttonFind = glade_xml_get_widget(xml,"btnFind");
	m_buttonFindReplace = glade_xml_get_widget(xml,"btnFindReplace");
	m_buttonReplaceAll = glade_xml_get_widget(xml,"btnReplaceAll");
	m_comboFind = glade_xml_get_widget(xml,"comboFind");
	m_comboReplace = glade_xml_get_widget(xml,"comboReplace");
	m_checkbuttonMatchCase = glade_xml_get_widget(xml,"chkMatchCase");
	m_checkbuttonWholeWord = glade_xml_get_widget(xml,"chkWholeWord");
	m_checkbuttonReverseFind = glade_xml_get_widget(xml,"chkReverseFind");

	GtkWidget * labelFind = glade_xml_get_widget(xml,"lblFind");
	GtkWidget * labelReplace = glade_xml_get_widget(xml,"lblReplace");

	ConstructWindowName();
	gtk_window_set_title(GTK_WINDOW(m_windowMain), m_WindowName);

	gtk_combo_disable_activate (GTK_COMBO(m_comboFind)); 
	gtk_combo_disable_activate (GTK_COMBO(m_comboReplace));

	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_FR_MatchCase,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());	
	gtk_button_set_label(GTK_BUTTON(m_checkbuttonMatchCase), unixstr); 
	FREEP(unixstr);

	pSS->getValueUTF8(AP_STRING_ID_DLG_FR_WholeWord,s);
    UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	gtk_button_set_label(GTK_BUTTON(m_checkbuttonWholeWord), unixstr);
    FREEP(unixstr);

	pSS->getValueUTF8(AP_STRING_ID_DLG_FR_ReverseFind,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	gtk_button_set_label(GTK_BUTTON(m_checkbuttonReverseFind), unixstr);
	FREEP(unixstr);

	pSS->getValueUTF8(AP_STRING_ID_DLG_FR_ReplaceWithLabel,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());	
	gtk_label_set_text(GTK_LABEL(labelReplace), unixstr);
	FREEP(unixstr);

	pSS->getValueUTF8(AP_STRING_ID_DLG_FR_FindLabel,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	gtk_label_set_text(GTK_LABEL(labelFind), unixstr);
	FREEP(unixstr);

	pSS->getValueUTF8(AP_STRING_ID_DLG_FR_ReplaceAllButton,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());	
	gtk_button_set_label(GTK_BUTTON(m_buttonReplaceAll), unixstr);
	FREEP(unixstr);

	// create and disable the find button initially
	gtk_widget_set_sensitive(m_buttonFind, FALSE);
	gtk_widget_set_sensitive(m_buttonFindReplace, FALSE);
	gtk_widget_set_sensitive(m_buttonReplaceAll, FALSE);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkbuttonMatchCase), getMatchCase());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkbuttonWholeWord), getWholeWord());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkbuttonReverseFind), getReverseFind());
	
	gtk_widget_show_all (m_windowMain);

	
	if (m_id != AP_DIALOG_ID_REPLACE){
		// todo: get rid of this code once bug # 5085 is closed
		gtk_widget_hide (labelReplace);
		gtk_widget_hide (m_comboReplace);
		gtk_widget_hide (m_buttonFindReplace);
		gtk_widget_hide (m_buttonReplaceAll);
	}

	g_signal_connect(G_OBJECT(m_windowMain), "response", 
					 G_CALLBACK(s_response_triggered), this);

	// attach generic signals
	g_signal_connect(G_OBJECT(m_checkbuttonMatchCase),
					 "toggled",
					 G_CALLBACK(s_match_case_toggled),
					 this);

	g_signal_connect(G_OBJECT(m_checkbuttonWholeWord),
					 "toggled",
					 G_CALLBACK(s_whole_word_toggled),
					 this);

	g_signal_connect(G_OBJECT(m_checkbuttonReverseFind),
					 "toggled",
					 G_CALLBACK(s_reverse_find_toggled),
					 this);

	// If the user hits "enter" in the entry field, we launch a find
	g_signal_connect(G_OBJECT(GTK_COMBO(m_comboFind)->entry),
					 "activate",
					 G_CALLBACK(s_find_entry_activate),
					 this);
	
	g_signal_connect(G_OBJECT(GTK_COMBO(m_comboFind)->entry),
					 "changed",
					 G_CALLBACK(s_find_entry_change),
					 (gpointer) this);

	g_signal_connect (G_OBJECT(m_buttonFind), "clicked", G_CALLBACK(s_find_clicked), m_windowMain);
	g_signal_connect (G_OBJECT(m_buttonFindReplace), "clicked", G_CALLBACK(s_findreplace_clicked), m_windowMain);
	g_signal_connect (G_OBJECT(m_buttonReplaceAll), "clicked", G_CALLBACK(s_replaceall_clicked), m_windowMain);

	// signals only useful in "replace mode"
	// If the user hits "enter" in the entry field, we launch a replace
	g_signal_connect(G_OBJECT(GTK_COMBO(m_comboReplace)->entry),
					 "activate",
					 G_CALLBACK(s_replace_entry_activate),
					 this);

	// the catch-alls
	// Dont use gtk_signal_connect_after for modeless dialogs
	g_signal_connect(G_OBJECT(m_windowMain),
					   "destroy",
					   G_CALLBACK(s_destroy_clicked),
					   (gpointer) this);
	g_signal_connect(G_OBJECT(m_windowMain),
					   "delete_event",
					   G_CALLBACK(s_delete_clicked),
					   (gpointer) this);
	
	gtk_widget_queue_resize (m_windowMain);
	return m_windowMain;
}

void AP_UnixDialog_Replace::_populateWindowData(void)
{
	UT_ASSERT(m_comboFind && m_checkbuttonMatchCase);

	// last used find string
	{
		UT_UCSChar * bufferUnicode = getFindString();
		char * bufferNormal = (char *) UT_calloc(UT_UCS4_strlen(bufferUnicode) + 1, sizeof(char));
		UT_UCS4_strcpy_to_char(bufferNormal, bufferUnicode);
		FREEP(bufferUnicode);
		
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(m_comboFind)->entry), bufferNormal);
		gtk_editable_select_region(GTK_EDITABLE(GTK_COMBO(m_comboFind)->entry), 0, -1);

		FREEP(bufferNormal);
	}	
	
	// last used replace string
	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		UT_ASSERT(m_comboReplace);
		
		UT_UCSChar * bufferUnicode = getReplaceString();
		char * bufferNormal = (char *) UT_calloc(UT_UCS4_strlen(bufferUnicode) + 1, sizeof(char));
		UT_UCS4_strcpy_to_char(bufferNormal, bufferUnicode);
		FREEP(bufferUnicode);
		
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(m_comboReplace)->entry), bufferNormal);

		FREEP(bufferNormal);
	}
	
	// update lists
	_updateLists();

	// match case button
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkbuttonMatchCase), getMatchCase());

	// Find entry should have focus, for immediate typing
	gtk_widget_grab_focus(m_comboFind);	
}

void AP_UnixDialog_Replace::_storeWindowData(void)
{
	// TODO: nothing?  The actual methods store
	// out last used data to the persist variables,
	// since we need to save state when things actually
	// happen (not when the dialog closes).
}

void AP_UnixDialog_Replace::_updateLists()
{
	_updateList(m_comboFind, &m_findList);
	_updateList(m_comboReplace, &m_replaceList);
}

void AP_UnixDialog_Replace::_updateList(GtkWidget* combo, UT_GenericVector<UT_UCS4Char*>* list)
{
	if (!combo) return; // no combo? do nothing
	if (!list) return; // no list? do nothing
	
	UT_uint32 i = 0;
	GList *items = NULL;
	
	gtk_list_clear_items(GTK_LIST(GTK_COMBO(combo)->list), 0, -1); 
	items = NULL;
	for (i = 0; i<list->getItemCount(); i++)
	{
		// leaving the size 0 causes the string class to determine the length itself
		UT_UCS4String ucs4s(list->getNthItem(i), 0); 
		
		// clone the string, since we can't use utf8_str()'s result -> ucs4s will disappear from stack
		char* utf8s = g_strdup(ucs4s.utf8_str()); 
		
		// add it to the list
		UT_DEBUGMSG(("FODDEX: find/replace list: %d = '%s'\n", i, utf8s));
    	items = g_list_append (items, static_cast<void*>(utf8s));
	}
	if (items)
		gtk_combo_set_popdown_strings(GTK_COMBO(combo), items);
}
