/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * Copyright (C) 2005 Robert Staudinger <robsta@stereolyzer.net>
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

#include <stdlib.h>
#include <string.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Spell.h"
#include "ap_UnixDialog_Spell.h"



//! Custom response IDs
enum {
	SPELL_RESPONSE_ADD = 0, 
	SPELL_RESPONSE_IGNORE,
	SPELL_RESPONSE_IGNORE_ALL,
	SPELL_RESPONSE_CHANGE,
	SPELL_RESPONSE_CHANGE_ALL
};

//! Column indices for list-store
enum {
	COLUMN_SUGGESTION = 0,
	COLUMN_NUMBER,
	NUM_COLUMNS
};



/*!
* Event dispatcher for button "Add"
*/
static void
AP_UnixDialog_Spell__onAddClicked (GtkButton * /*button*/,
								   gpointer   data)
{
	AP_UnixDialog_Spell *dlg = static_cast<AP_UnixDialog_Spell*>(data);
	gtk_dialog_response (GTK_DIALOG (dlg->getWindow ()), SPELL_RESPONSE_ADD);
}

/*!
* Event dispatcher for button "Ignore"
*/
static void
AP_UnixDialog_Spell__onIgnoreClicked (GtkButton * /*button*/,
									  gpointer   data)
{
	AP_UnixDialog_Spell *dlg = static_cast<AP_UnixDialog_Spell*>(data);
	gtk_dialog_response (GTK_DIALOG (dlg->getWindow ()), SPELL_RESPONSE_IGNORE);
}

/*!
* Event dispatcher for button "Ignore All"
*/
static void
AP_UnixDialog_Spell__onIgnoreAllClicked (GtkButton * /*button*/,
										 gpointer   data)
{
	AP_UnixDialog_Spell *dlg = static_cast<AP_UnixDialog_Spell*>(data);
	gtk_dialog_response (GTK_DIALOG (dlg->getWindow ()), SPELL_RESPONSE_IGNORE_ALL);
}

/*!
* Event dispatcher for button "Change"
*/
static void
AP_UnixDialog_Spell__onChangeClicked (GtkButton * /*button*/,
									  gpointer   data)
{
	AP_UnixDialog_Spell *dlg = static_cast<AP_UnixDialog_Spell*>(data);
	gtk_dialog_response (GTK_DIALOG (dlg->getWindow ()), SPELL_RESPONSE_CHANGE);
}

/*!
* Event dispatcher for button "Change All"
*/
static void
AP_UnixDialog_Spell__onChangeAllClicked (GtkButton * /*button*/,
										 gpointer   data)
{
	AP_UnixDialog_Spell *dlg = static_cast<AP_UnixDialog_Spell*>(data);
	gtk_dialog_response (GTK_DIALOG (dlg->getWindow ()), SPELL_RESPONSE_CHANGE_ALL);
}

/*!
* Event dispatcher for dblclicking a suggestion
*/
static void
AP_UnixDialog_Spell__onSuggestionDblClicked (GtkTreeView       * /*tree*/,
											 GtkTreePath       * /*path*/,
											 GtkTreeViewColumn * /*col*/,
											 gpointer		    data)
{
	AP_UnixDialog_Spell *dlg = static_cast<AP_UnixDialog_Spell*>(data);
	gtk_dialog_response (GTK_DIALOG (dlg->getWindow ()), SPELL_RESPONSE_CHANGE);
}

/*!
* Event dispatcher for selecting a suggestion
*/
static void
AP_UnixDialog_Spell__onSuggestionSelected (GtkButton * /*button*/,
										   gpointer   data)
{
	AP_UnixDialog_Spell *dlg = static_cast<AP_UnixDialog_Spell*>(data);
	dlg->onSuggestionSelected ();
}

/*!
* Event dispatcher for editing the suggestion
*/
static void
AP_UnixDialog_Spell__onSuggestionChanged (GtkButton * /*button*/,
										  gpointer   data)
{
	AP_UnixDialog_Spell *dlg = static_cast<AP_UnixDialog_Spell*>(data);
	dlg->onSuggestionChanged ();
}



/*!
* Static ctor.
*/
XAP_Dialog * 
AP_UnixDialog_Spell::static_constructor (XAP_DialogFactory * pFactory,
										 XAP_Dialog_Id 		 id)
{
	return new AP_UnixDialog_Spell (pFactory,id);
}

/*!
* Ctor.
*/
AP_UnixDialog_Spell::AP_UnixDialog_Spell (XAP_DialogFactory * pDlgFactory,
										  XAP_Dialog_Id 	  id)
	: AP_Dialog_Spell (pDlgFactory, id)
{
	m_wDialog = NULL;
	m_txWrong = NULL;
	m_eChange = NULL;
	m_lvSuggestions = NULL;
}

/*!
* Dtor.
*/
AP_UnixDialog_Spell::~AP_UnixDialog_Spell (void)
{
}



/*!
* Run dialog.
*/
void 
AP_UnixDialog_Spell::runModal (XAP_Frame * pFrame)
{   
    // class the base class method to initialize some basic xp stuff
    AP_Dialog_Spell::runModal(pFrame);
   
    bool bRes = nextMisspelledWord();
   
    if (bRes) { // we need to prepare the dialog
        GtkWidget * mainWindow = _constructWindow();
        UT_ASSERT(mainWindow);

        // Populate the window's data items
        _populateWindowData();
      
        abiSetupModalDialog(GTK_DIALOG(mainWindow), pFrame, this, GTK_RESPONSE_CLOSE);

        // now loop while there are still misspelled words
        while (bRes) {
     
            // show word in main window
            makeWordVisible();
     
			gpointer inst = gtk_tree_view_get_selection (GTK_TREE_VIEW (m_lvSuggestions));
			g_signal_handler_block (inst, m_listHandlerID);
            // update dialog with new misspelled word info/suggestions
            _updateWindow();
			g_signal_handler_unblock (inst, m_listHandlerID);

			// run into the GTK event loop for this window
	    gint response = abiRunModalDialog (GTK_DIALOG(mainWindow), false);
	    UT_DEBUGMSG (("ROB: response='%d'\n", response));
            switch(response) {

	            case SPELL_RESPONSE_CHANGE:
	                onChangeClicked (); break;
	            case SPELL_RESPONSE_CHANGE_ALL:
	                onChangeAllClicked (); break;
	            case SPELL_RESPONSE_IGNORE:
	                onIgnoreClicked (); break;
	            case SPELL_RESPONSE_IGNORE_ALL:
	                onIgnoreAllClicked (); break;
	            case SPELL_RESPONSE_ADD:
	                onAddClicked (); break;
	            default:
					m_bCancelled = TRUE;
		            _purgeSuggestions();
					gtk_widget_destroy (m_wDialog);
					return;
            }

            _purgeSuggestions();
          
            // get the next unknown word
            bRes = nextMisspelledWord();
        }
      
        abiDestroyWidget(mainWindow);
    }
}

/*!
* Set up the dialog.
*/
GtkWidget * 
AP_UnixDialog_Spell::_constructWindow (void)
{
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_Spell.ui");

	m_wDialog = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Spell"));

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Spell_SpellTitle,s);
	gtk_window_set_title (GTK_WINDOW( m_wDialog), s.c_str());

	localizeLabelUnderline(GTK_WIDGET(gtk_builder_get_object(builder, "lbNotInDict")), pSS, AP_STRING_ID_DLG_Spell_UnknownWord);
	localizeLabelUnderline(GTK_WIDGET(gtk_builder_get_object(builder, "lbChangeTo")), pSS, AP_STRING_ID_DLG_Spell_ChangeTo);

	m_txWrong = GTK_WIDGET(gtk_builder_get_object(builder, "txWrong"));
	m_eChange = GTK_WIDGET(gtk_builder_get_object(builder, "eChange"));
	m_lvSuggestions = GTK_WIDGET(gtk_builder_get_object(builder, "tvSuggestions"));

	// localise
	localizeButtonUnderline (GTK_WIDGET(gtk_builder_get_object(builder, "btIgnore")), pSS, AP_STRING_ID_DLG_Spell_Ignore);
	localizeButtonUnderline (GTK_WIDGET(gtk_builder_get_object(builder, "btIgnoreAll")), pSS, AP_STRING_ID_DLG_Spell_IgnoreAll);
	localizeButtonUnderline (GTK_WIDGET(gtk_builder_get_object(builder, "btChange")), pSS, AP_STRING_ID_DLG_Spell_Change);
	localizeButtonUnderline (GTK_WIDGET(gtk_builder_get_object(builder, "btChangeAll")), pSS, AP_STRING_ID_DLG_Spell_ChangeAll);

	// attach signals
	g_signal_connect (GTK_WIDGET(gtk_builder_get_object(builder, "btAdd")), 
					  "clicked", 
					  G_CALLBACK (AP_UnixDialog_Spell__onAddClicked), 
					  (gpointer)this);
	g_signal_connect (GTK_WIDGET(gtk_builder_get_object(builder, "btIgnore")), 
					  "clicked", 
					  G_CALLBACK (AP_UnixDialog_Spell__onIgnoreClicked), 
					  (gpointer)this);
	g_signal_connect (GTK_WIDGET(gtk_builder_get_object(builder, "btIgnoreAll")), 
					  "clicked", 
					  G_CALLBACK (AP_UnixDialog_Spell__onIgnoreAllClicked), 
					  (gpointer)this);
	g_signal_connect (GTK_WIDGET(gtk_builder_get_object(builder, "btChange")), 
					  "clicked", 
					  G_CALLBACK (AP_UnixDialog_Spell__onChangeClicked), 
					  (gpointer)this);
	g_signal_connect (GTK_WIDGET(gtk_builder_get_object(builder, "btChangeAll")), 
					  "clicked", 
					  G_CALLBACK (AP_UnixDialog_Spell__onChangeAllClicked), 
					  (gpointer)this);
	g_signal_connect (GTK_TREE_VIEW (m_lvSuggestions), 
					  "row-activated", 
					  G_CALLBACK (AP_UnixDialog_Spell__onSuggestionDblClicked), 
					  (gpointer)this);
	m_replaceHandlerID = g_signal_connect (G_OBJECT(m_eChange), 
					   "changed",
					   G_CALLBACK (AP_UnixDialog_Spell__onSuggestionChanged),
					   (gpointer)this);


	// highlight our misspelled word in red
	m_highlight.red = 0xffff;
	m_highlight.green = 0x0000;
	m_highlight.blue = 0x0000;

	// Liststore and -view
	GtkListStore *store = gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING, G_TYPE_UINT);
	gtk_tree_view_set_model (GTK_TREE_VIEW (m_lvSuggestions), GTK_TREE_MODEL (store));
	g_object_unref (G_OBJECT (store));

	// Column Suggestion
	GtkCellRenderer *renderer = NULL;
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (m_lvSuggestions),
												-1, "Name", renderer,
												"text", COLUMN_SUGGESTION,
												NULL);
	GtkTreeViewColumn *column = gtk_tree_view_get_column (GTK_TREE_VIEW (m_lvSuggestions), 0);
	gtk_tree_view_column_set_sort_column_id (column, COLUMN_SUGGESTION);

	m_listHandlerID = g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (m_lvSuggestions)), 
				  "changed",
				  G_CALLBACK (AP_UnixDialog_Spell__onSuggestionSelected), 
				  (gpointer)this);

	gtk_widget_show_all (m_wDialog);

	g_object_unref(G_OBJECT(builder));

	return m_wDialog;
}

void 
AP_UnixDialog_Spell::_updateWindow (void)
{             
	GtkTextBuffer * buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_txWrong));
	GtkTextIter iter2;

	// Empty buffer
	gtk_text_buffer_set_text(buffer, "", -1);

	const UT_UCSChar *p;
	UT_sint32 iLength;
	// insert start of sentence
	p = m_pWordIterator->getPreWord(iLength);
	if (0 < iLength)
	{
		gchar * preword = (gchar*) _convertToMB(p, iLength);
		gtk_text_buffer_set_text(buffer, preword, -1);
		FREEP(preword);
	}

	// insert misspelled word (in highlight color)
	p = m_pWordIterator->getCurrentWord(iLength);
	gchar * word = (gchar*) _convertToMB(p, iLength);
	GtkTextTag * txt_tag = gtk_text_buffer_create_tag(buffer, NULL, "foreground-gdk", &m_highlight, NULL); 
	gtk_text_buffer_get_end_iter(buffer, &iter2);
	gtk_text_buffer_insert_with_tags(buffer, &iter2, word, -1, txt_tag, NULL);
	// word is freed at the end of the method...
	
	// insert end of sentence
	p = m_pWordIterator->getPostWord(iLength);
	if (0 < iLength)
	{
		gchar * postword = (gchar*) _convertToMB(p, iLength);
		gtk_text_buffer_get_end_iter(buffer, &iter2);
		gtk_text_buffer_insert(buffer, &iter2, postword, -1);
		FREEP(postword);
	}
	else
	{
		// Insert space to make gtk_text_buffer understand that it
		// really should highlight the selected word. This is a
		// workaround for bug 5459. It really should be fixed in GTK.
		gtk_text_buffer_get_end_iter(buffer, &iter2);
		gtk_text_buffer_insert(buffer, &iter2, " ", -1);
	}
	// TODO: set scroll position so misspelled word is centered


	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (m_lvSuggestions));
	
	// Detach model for faster updates
	g_object_ref (G_OBJECT (model));	
	gtk_tree_view_set_model (GTK_TREE_VIEW (m_lvSuggestions), NULL);
	gtk_list_store_clear (GTK_LIST_STORE (model));	
     
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (m_lvSuggestions));

	UT_DEBUGMSG (("ROB: AP_UnixDialog_Spell::_updateWindow() itemcount=%d\n", m_Suggestions->getItemCount ()));
	if (m_Suggestions->getItemCount () == 0) {

		GtkTreeIter iter;
		gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);

		const XAP_StringSet * pSS = m_pApp->getStringSet();
		std::string s;
		pSS->getValueUTF8(AP_STRING_ID_DLG_Spell_NoSuggestions,s);

		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
				    COLUMN_SUGGESTION, s.c_str(),
							COLUMN_NUMBER, -1,
							-1);

		g_signal_handler_block(G_OBJECT(m_eChange), m_replaceHandlerID);
		gtk_entry_set_text(GTK_ENTRY(m_eChange), word);
		g_signal_handler_unblock(G_OBJECT(m_eChange), m_replaceHandlerID);      
	} 
	else
	{

		GtkTreeIter iter;
		gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

		gchar * suggest = NULL;   
		for (UT_sint32 i = 0; i < m_Suggestions->getItemCount(); i++)
		{
			suggest = (gchar*) _convertToMB((UT_UCSChar*)m_Suggestions->getNthItem(i));
			gtk_list_store_append (GTK_LIST_STORE (model), &iter);
			gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
								COLUMN_SUGGESTION, suggest,  
								COLUMN_NUMBER, i,
								-1);
		}
		// put the first suggestion in the entry
		suggest = (gchar*) _convertToMB((UT_UCSChar*)m_Suggestions->getNthItem(0));
		g_signal_handler_block(G_OBJECT(m_eChange), m_replaceHandlerID);
		gtk_entry_set_text(GTK_ENTRY(m_eChange), suggest);
		g_signal_handler_unblock(G_OBJECT(m_eChange), m_replaceHandlerID);      
	}

	gtk_tree_view_set_model (GTK_TREE_VIEW (m_lvSuggestions), model);
	g_object_unref (G_OBJECT (model));	

	// select first
	if (m_Suggestions->getItemCount () > 0) {
		GtkTreePath *path = gtk_tree_path_new_first ();
		gtk_tree_selection_select_path (selection, path);
		gtk_tree_path_free (path);
	}

	FREEP (word);
}

void 
AP_UnixDialog_Spell::_populateWindowData (void)
{
	// TODO: initialize list of user dictionaries
}



/*!
* Event-handler for button "Change".
*/
void 
AP_UnixDialog_Spell::onChangeClicked ()
{
	UT_DEBUGMSG (("ROB: AP_UnixDialog_Spell::onChangeClicked()\n"));
	UT_UCSChar * replace = NULL;
	replace = _convertFromMB((char*)gtk_entry_get_text(GTK_ENTRY(m_eChange)));
	if (!replace || !UT_UCS4_strlen(replace))
	{
		UT_DEBUGMSG(("replace is 0 length\n"));
		FREEP(replace);
		return;
	}
	changeWordWith(replace);
	FREEP(replace);
}

/*!
* Event-handler for button "Change All".
*/
void 
AP_UnixDialog_Spell::onChangeAllClicked ()
{
	UT_DEBUGMSG (("ROB: AP_UnixDialog_Spell::onChangeAllClicked()\n"));
	UT_UCSChar * replace = NULL;
	replace = _convertFromMB((char*)gtk_entry_get_text(GTK_ENTRY(m_eChange)));
	if (!replace || !UT_UCS4_strlen(replace))
	{
		FREEP(replace);
		return;
	}
	addChangeAll(replace);
	changeWordWith(replace);
	FREEP(replace);
}

/*!
* Event-handler for button "Ignore".
*/
void 
AP_UnixDialog_Spell::onIgnoreClicked ()
{
	UT_DEBUGMSG (("ROB: AP_UnixDialog_Spell::onIgnoreClicked()\n"));
	ignoreWord();
}

/*!
* Event-handler for button "Ignore All".
*/
void 
AP_UnixDialog_Spell::onIgnoreAllClicked ()
{
	UT_DEBUGMSG (("ROB: AP_UnixDialog_Spell::onIgnoreAllClicked()\n"));
	addIgnoreAll();
	ignoreWord();
}

/*!
* Event-handler for button "Add".
*/
void 
AP_UnixDialog_Spell::onAddClicked ()
{
	UT_DEBUGMSG (("ROB: AP_UnixDialog_Spell::onAddClicked()\n"));
	addToDict();   
	ignoreWord();
}

/*!
* Event-handler for selecting a suggestion
*/
void 
AP_UnixDialog_Spell::onSuggestionSelected ()
{
	UT_DEBUGMSG (("ROB: AP_UnixDialog_Spell::onSuggestionSelected()\n"));
	if (!m_Suggestions->getItemCount())
		return;
   
	GtkTreeIter iter;
	gchar * newreplacement = NULL;
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (m_lvSuggestions));
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (m_lvSuggestions));
	gtk_tree_selection_get_selected (selection, &model, &iter);
	gtk_tree_model_get (model, &iter, COLUMN_SUGGESTION, &newreplacement, -1);
	UT_ASSERT(newreplacement);

	g_signal_handler_block(G_OBJECT(m_eChange), m_replaceHandlerID);
	gtk_entry_set_text(GTK_ENTRY(m_eChange), newreplacement);
	g_signal_handler_unblock(G_OBJECT(m_eChange), m_replaceHandlerID);
}

/*!
* Event-handler for editing the suggestion.
*/
void 
AP_UnixDialog_Spell::onSuggestionChanged ()
{
	UT_DEBUGMSG (("ROB: AP_UnixDialog_Spell::onSuggestionChanged()\n"));
	const gchar * modtext = gtk_entry_get_text(GTK_ENTRY(m_eChange));
	UT_ASSERT(modtext);

	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (m_lvSuggestions));
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (m_lvSuggestions));
	GtkTreePath *first = gtk_tree_path_new_first ();
	if (gtk_tree_model_get_iter (model, &iter, first))
	{
		gtk_tree_path_free (first);
		do
		{
			gchar *label = NULL;
			gtk_tree_model_get (model, &iter, COLUMN_SUGGESTION, &label, -1);
			if (g_ascii_strncasecmp (modtext, label, strlen (modtext)) == 0)
			{
				GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
				g_signal_handler_block(G_OBJECT(selection), m_listHandlerID);
				gtk_tree_selection_select_path (selection, path);
				g_signal_handler_unblock(G_OBJECT(selection), m_listHandlerID);
				gtk_tree_path_free (path);
				return;			
			}
		}
	   	while (gtk_tree_model_iter_next (model, &iter));
	}
	else
	{
		gtk_tree_path_free (first);
		gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (m_lvSuggestions)));
	}
}



/*!
* Conversion helper.
*/
char * 
AP_UnixDialog_Spell::_convertToMB (const UT_UCSChar *wword)
{
	UT_UCS4String ucs4(wword);
	return g_strdup(ucs4.utf8_str());
}

/*!
* Conversion helper.
*/
char * 
AP_UnixDialog_Spell::_convertToMB (const UT_UCSChar *wword, 
								   UT_sint32 iLength)
{
	UT_UCS4String ucs4(wword, iLength);
	return g_strdup(ucs4.utf8_str());
}

/*!
* Conversion helper.
*/
UT_UCSChar * 
AP_UnixDialog_Spell::_convertFromMB (const char *word)
{
	UT_UCS4Char * str = 0;
	UT_UCS4String ucs4(word);
	UT_UCS4_cloneString(&str, ucs4.ucs4_str());
	return str;
}
