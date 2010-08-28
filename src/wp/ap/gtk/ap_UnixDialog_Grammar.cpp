#include <stdlib.h>
#include <string.h>

#include "ut_string.h"
#include "ut_vector.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Grammar.h"
#include "ap_UnixDialog_Grammar.h"

//! Custom response IDs
enum { 
	GRAMMAR_RESPONSE_IGNORE = 0,
	GRAMMAR_RESPONSE_CHANGE
};

//! Column indices for list-store
enum {
	COLUMN_SUGGESTION = 0,
	COLUMN_NUMBER,
	NUM_COLUMNS
};


/*!
* Event dispatcher for button "Ignore"
*/
static void
AP_UnixDialog_Grammar__onIgnoreClicked (GtkButton * /*button*/,
									  gpointer   data)
{
	AP_UnixDialog_Grammar *dlg = static_cast<AP_UnixDialog_Grammar*>(data);
	gtk_dialog_response (GTK_DIALOG (dlg->getWindow ()), GRAMMAR_RESPONSE_IGNORE);
}


/*!
* Event dispatcher for button "Change"
*/
static void
AP_UnixDialog_Grammar__onChangeClicked (GtkButton * /*button*/,
									  gpointer   data)
{
	AP_UnixDialog_Grammar *dlg = static_cast<AP_UnixDialog_Grammar*>(data);
	gtk_dialog_response (GTK_DIALOG (dlg->getWindow ()), GRAMMAR_RESPONSE_CHANGE);
}


/*!
* Event dispatcher for dblclicking a suggestion
*/
static void
AP_UnixDialog_Grammar__onSuggestionDblClicked (GtkTreeView       * /*tree*/,
											   GtkTreePath       * /*path*/,
											   GtkTreeViewColumn * /*col*/,
											   gpointer		      data)
{
	AP_UnixDialog_Grammar *dlg = static_cast<AP_UnixDialog_Grammar*>(data);
	gtk_dialog_response (GTK_DIALOG (dlg->getWindow ()), GRAMMAR_RESPONSE_CHANGE);
}

/*!
* Event dispatcher for selecting a suggestion
*/
static void
AP_UnixDialog_Grammar__onSuggestionSelected (GtkButton *button,
										   gpointer   data)
{
	AP_UnixDialog_Grammar *dlg = static_cast<AP_UnixDialog_Grammar*>(data);
	dlg->onSuggestionSelected ();
}

/*!
* Event dispatcher for editing the suggestion
*/
static void
AP_UnixDialog_Grammar__onSuggestionChanged (GtkButton *button,
										  gpointer   data)
{
	AP_UnixDialog_Grammar *dlg = static_cast<AP_UnixDialog_Grammar*>(data);
	dlg->onSuggestionChanged ();
}

/*!
* Static ctor.
*/
XAP_Dialog * 
AP_UnixDialog_Grammar::static_constructor (XAP_DialogFactory * pFactory,
										   XAP_Dialog_Id 	id)
{
	return new AP_UnixDialog_Grammar(pFactory,id);
}

/*!
* Ctor.
*/
AP_UnixDialog_Grammar::AP_UnixDialog_Grammar (XAP_DialogFactory * pDlgFactory,
										  	  XAP_Dialog_Id 	  id)
	: AP_Dialog_Grammar (pDlgFactory, id)
{
	m_wDialog = NULL;
	m_txOriginal = NULL;
	m_eChange = NULL;
	m_lvSuggestions = NULL;
	m_txExplanation = NULL;
}

/*!
* Dtor.
*/
AP_UnixDialog_Grammar::~AP_UnixDialog_Grammar (void)
{
}

/*!
* Run dialog.
*/
void 
AP_UnixDialog_Grammar::runModal (XAP_Frame * pFrame)
{   
    // class the base class method to initialize some basic xp stuff
	AP_Dialog_Grammar::runModal(pFrame);
   
	bool bRes = nextWrongSentence();
   
	if (bRes) 
	{ 
		// we need to prepare the dialog
		GtkWidget * mainWindow = _constructWindow();
		UT_ASSERT(mainWindow);
      
        abiSetupModalDialog(GTK_DIALOG(mainWindow), pFrame, this, GTK_RESPONSE_CLOSE);

        // now loop while there are still sentences with mistakes
		while (bRes) 
		{
			bool bMist = nextSentenceMistake();
			while (bMist)
			{
				// show sentence in main window
				makeMistakeVisible();
     
				gpointer inst = gtk_tree_view_get_selection (GTK_TREE_VIEW (m_lvSuggestions));
				g_signal_handler_block (inst, m_listHandlerID);
				// update dialog with new misspelled word info/suggestions
				_updateWindow();
				g_signal_handler_unblock (inst, m_listHandlerID);

				// run into the GTK event loop for this window
				gint response = abiRunModalDialog (GTK_DIALOG(mainWindow), false);
				UT_DEBUGMSG (("ROB: response='%d'\n", response));
				switch (response) 
				{
					case GRAMMAR_RESPONSE_CHANGE:
	      				onChangeClicked (); 
						break;
					case GRAMMAR_RESPONSE_IGNORE:
						onIgnoreClicked (); 
						break;
					default:
						m_bCancelled = TRUE;
						_purgeMistakes();
						gtk_widget_destroy (m_wDialog);
						return;
				}
				bMist = nextSentenceMistake();
			}
			_purgeMistakes();
          
			// get the next sentence with mistakes
			bRes = nextWrongSentence();
		}
		abiDestroyWidget(mainWindow);

	}
}

/*!
* Set up the dialog.
*/
GtkWidget * 
AP_UnixDialog_Grammar::_constructWindow (void)
{
    // get the path where our UI file is located
	std::string ui_path = static_cast<XAP_UnixApp*>(XAP_App::getApp())->getAbiSuiteAppUIDir() + "/ap_UnixDialog_Grammar.xml";

	// load the dialog from the UI file
	GtkBuilder* builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, ui_path.c_str(), NULL);

	m_wDialog = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Grammar"));
#if 0
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
    UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
    glade_path += "/ap_UnixDialog_Grammar.glade";

    // load the dialog from the glade file
    GladeXML * pXML = abiDialogNewFromXML (glade_path.c_str() );
    if (!pXML)
    	return NULL;

    m_wDialog = glade_xml_get_widget (pXML, "ap_UnixDialog_Grammar");
#endif

    //const XAP_StringSet * pSS = m_pApp->getStringSet();

		//UT_UTF8String s;
		//pSS->getValueUTF8(AP_STRING_ID_DLG_Spell_SpellTitle,s);
    gtk_window_set_title (GTK_WINDOW(m_wDialog), "Grammar checking");

		//localizeLabelUnderline(glade_xml_get_widget (pXML, "lbNotInDict"), pSS, AP_STRING_ID_DLG_Spell_UnknownWord);
		//localizeLabelUnderline(glade_xml_get_widget (pXML, "lbChangeTo"), pSS, AP_STRING_ID_DLG_Spell_ChangeTo);

    m_txOriginal = GTK_WIDGET(gtk_builder_get_object (builder, "txOriginal"));
    m_eChange = GTK_WIDGET(gtk_builder_get_object (builder, "eChange"));
    m_lvSuggestions = GTK_WIDGET(gtk_builder_get_object (builder, "lvSuggestions"));
	m_txExplanation = GTK_WIDGET(gtk_builder_get_object (builder, "txExplanation"));

		// localise
		//localizeButtonUnderline (glade_xml_get_widget (pXML, "btIgnore"), pSS, AP_STRING_ID_DLG_Spell_Ignore);
		//localizeButtonUnderline (glade_xml_get_widget (pXML, "btIgnoreAll"), pSS, AP_STRING_ID_DLG_Spell_IgnoreAll);
		//localizeButtonUnderline (glade_xml_get_widget (pXML, "btChange"), pSS, AP_STRING_ID_DLG_Spell_Change);
		//localizeButtonUnderline (glade_xml_get_widget (pXML, "btChangeAll"), pSS, AP_STRING_ID_DLG_Spell_ChangeAll);

    // attach signals
	g_signal_connect (GTK_WIDGET(gtk_builder_get_object (builder, "btIgnore")), 
					  "clicked", 
					  G_CALLBACK (AP_UnixDialog_Grammar__onIgnoreClicked), 
					  (gpointer)this);
	/*
	g_signal_connect (GTK_WIDGET(gtk_builder_get_object (builder, "btIgnoreAll")), 
					  "clicked", 
					  G_CALLBACK (AP_UnixDialog_Grammar__onIgnoreAllClicked), 
					  (gpointer)this);
	*/
	g_signal_connect (GTK_WIDGET(gtk_builder_get_object (builder, "btChange")), 
					  "clicked", 
					  G_CALLBACK (AP_UnixDialog_Grammar__onChangeClicked), 
					  (gpointer)this);
	/*
	g_signal_connect (GTK_WIDGET(gtk_builder_get_object (builder, "btChangeAll")), 
					  "clicked", 
					  G_CALLBACK (AP_UnixDialog_Grammar__onChangeAllClicked), 
					  (gpointer)this);
	*/
	g_signal_connect (GTK_TREE_VIEW (m_lvSuggestions), 
					  "row-activated", 
					  G_CALLBACK (AP_UnixDialog_Grammar__onSuggestionDblClicked), 
					  (gpointer)this);
    m_replaceHandlerID = g_signal_connect (G_OBJECT(m_eChange), 
					  "changed",
					  G_CALLBACK (AP_UnixDialog_Grammar__onSuggestionChanged),
					  (gpointer)this);


    // highlight our misspelled word in red

    GdkColormap * cm = gdk_colormap_get_system();
    m_highlight.red = 0xffff;
    m_highlight.green = 0x0000;
    m_highlight.blue = 0x0000;
    gdk_colormap_alloc_color(cm, &m_highlight, FALSE, TRUE);
	
	
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
				  G_CALLBACK (AP_UnixDialog_Grammar__onSuggestionSelected), 
				  (gpointer)this);
	  
    gtk_widget_show_all (m_wDialog);

	g_object_unref(G_OBJECT(builder));

    return m_wDialog;
}

void 
AP_UnixDialog_Grammar::_updateWindow()
{
	_updateOriginal();

	_updateSuggestions();

	_updateExplanation();
}

void 
AP_UnixDialog_Grammar::_updateOriginal()
{
	GtkTextBuffer * buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_txOriginal));
    GtkTextIter iter;
	
	// Empty buffer
	gtk_text_buffer_set_text(buffer, "", -1);

	UT_sint32 iLength;

    // insert misspelled word (in highlight color)
    const UT_UCS4String sentence(m_pSentIterator->getCurrentSentence(iLength));

	if (m_curMistake->iStart > 0)
	{
		gchar * preSent = (gchar*) _convertToMB(sentence.substr(0, m_curMistake->iStart).ucs4_str());
		gtk_text_buffer_set_text(buffer, preSent, -1);
		FREEP(preSent);
	}

	gchar * sent = (gchar*) _convertToMB(sentence.substr(m_curMistake->iStart,
										m_curMistake->iEnd - m_curMistake->iStart).ucs4_str());
    GtkTextTag * txt_tag = gtk_text_buffer_create_tag(buffer, NULL, "foreground-gdk", &m_highlight, NULL); 
	gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert_with_tags(buffer, &iter, sent, -1, txt_tag, NULL);
 
	if (m_curMistake->iEnd < iLength)
	{
		gchar * posSent = (gchar*) _convertToMB(sentence.substr(m_curMistake->iEnd, iLength- m_curMistake->iEnd).ucs4_str());
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_insert(buffer, &iter, posSent, -1);
		FREEP(posSent);
	}

	FREEP(sent);
}

void 
AP_UnixDialog_Grammar::_updateSuggestions()
{
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (m_lvSuggestions));
	
	// Detach model for faster updates
	g_object_ref (G_OBJECT (model));	
	gtk_tree_view_set_model (GTK_TREE_VIEW (m_lvSuggestions), NULL);
	gtk_list_store_clear (GTK_LIST_STORE (model));	
	gtk_entry_set_text(GTK_ENTRY(m_eChange), "");
     
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (m_lvSuggestions));

	GtkTreeIter titer;
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	
	if (!m_curMistake->m_pSuggestion)
		return;
	
	gchar *suggest = NULL;
	for (UT_uint32 i = 0; i < m_curMistake->m_pSuggestion->getItemCount(); i++)  
	{
		suggest = (gchar*) _convertToMB((UT_UCSChar*)m_curMistake->m_pSuggestion->getNthItem(i));
		// UT_DEBUGMSG(("DEBUG: GABRIEL: suggestion = %s\n", suggest));
		gtk_list_store_append (GTK_LIST_STORE (model), &titer);
		gtk_list_store_set (GTK_LIST_STORE (model), &titer, 
								COLUMN_SUGGESTION, suggest,  
								COLUMN_NUMBER, i,
								-1);
	}
	// put the first suggestion in the entry
	suggest = (gchar*) _convertToMB((UT_UCSChar*)m_curMistake->m_pSuggestion->getNthItem(0));
	g_signal_handler_block(G_OBJECT(m_eChange), m_replaceHandlerID);
	gtk_entry_set_text(GTK_ENTRY(m_eChange), suggest);
	g_signal_handler_unblock(G_OBJECT(m_eChange), m_replaceHandlerID);      
    

	gtk_tree_view_set_model (GTK_TREE_VIEW (m_lvSuggestions), model);
	g_object_unref (G_OBJECT (model));	

	// select first
	if (m_curMistake->m_pSuggestion->getItemCount () > 0) 
	{
		GtkTreePath *path = gtk_tree_path_new_first ();
		gtk_tree_selection_select_path (selection, path);
		gtk_tree_path_free (path);
	}

	FREEP(suggest);
}

void 
AP_UnixDialog_Grammar::_updateExplanation()
{
	GtkTextBuffer * buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_txExplanation));
    GtkTextIter iter;

	// Empty buffer
	gtk_text_buffer_set_text(buffer, "", -1);

    const UT_UCSChar *pExp;
    pExp = m_curMistake->pExplanation;
	if (!pExp)
		return;

    gchar * sent = (gchar*) _convertToMB(pExp); 
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, sent, -1);

	FREEP (sent);
}

/*!
* Event-handler for button "Change".
*/
void 
AP_UnixDialog_Grammar::onChangeClicked ()
{
	UT_DEBUGMSG (("GABRIEL: AP_UnixDialog_Grammar::onChangeClicked(): %s\n", (char*)gtk_entry_get_text(GTK_ENTRY(m_eChange))));
	UT_UCSChar * replace = NULL;
    replace = _convertFromMB((char*)gtk_entry_get_text(GTK_ENTRY(m_eChange)));
    if (!replace || !UT_UCS4_strlen(replace)) {
        UT_DEBUGMSG(("replace is 0 length\n"));
        FREEP(replace);
        return;
    }
    changeMistakeWith(replace);
    FREEP(replace);
}


/*!
* Event-handler for button "Ignore".
*/
void 
AP_UnixDialog_Grammar::onIgnoreClicked ()
{
	UT_DEBUGMSG (("GABRIEL: AP_UnixDialog_Grammar::onIgnoreClicked()\n"));
	ignoreMistake();
}


/*!
* Event-handler for selecting a suggestion
*/
void 
AP_UnixDialog_Grammar::onSuggestionSelected ()
{
	UT_DEBUGMSG (("GABRIEL: AP_UnixDialog_Grammar::onSuggestionSelected()\n"));
    if (!m_curMistake->m_pSuggestion)
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
AP_UnixDialog_Grammar::onSuggestionChanged ()
{
	UT_DEBUGMSG (("GABRIEL: AP_UnixDialog_Grammar::onSuggestionChanged()\n"));
}

/*!
* Conversion helper.
*/

char * 
AP_UnixDialog_Grammar::_convertToMB (const UT_UCSChar *wword)
{
    UT_UCS4String ucs4(wword);
    return g_strdup(ucs4.utf8_str());
}

char * 
AP_UnixDialog_Grammar::_convertToMB (const UT_UCSChar *wword, 
								   UT_sint32 iLength)
{
    UT_UCS4String ucs4(wword, iLength);
    return g_strdup(ucs4.utf8_str());
}

UT_UCSChar * 
AP_UnixDialog_Grammar::_convertFromMB (const char *sent)
{
    UT_UCS4Char * ucs4 = 0;
	UT_UCS4_cloneString (&ucs4, UT_UCS4String (sent).ucs4_str());
	return ucs4;
}
