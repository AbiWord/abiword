/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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


// TODO: still getting some artifacts when doing highligh/replacements

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Spell.h"
#include "ap_QNXDialog_Spell.h"


XAP_Dialog * AP_QNXDialog_Spell::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
   AP_QNXDialog_Spell * p = new AP_QNXDialog_Spell(pFactory,id);
   return p;
}

AP_QNXDialog_Spell::AP_QNXDialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : AP_Dialog_Spell(pDlgFactory,id)
{
}

AP_QNXDialog_Spell::~AP_QNXDialog_Spell(void)
{
}

/************************************************************/
void AP_QNXDialog_Spell::runModal(XAP_Frame * pFrame)
{
   UT_DEBUGMSG(("beginning spelling check..."));
#if 0
   
   // class the base class method to initialize some basic xp stuff
   AP_Dialog_Spell::runModal(pFrame);
   
   m_bCancelled = UT_FALSE;
   UT_Bool bRes = nextMisspelledWord();
   
   if (bRes) { // we need to prepare the dialog
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
      // soo it won't get lost underneath
      centerDialog(parentWindow, mainWindow);
      gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));
      
      // Show the top level dialog
      gtk_widget_show(mainWindow);
      
      // Make it modal, and stick it up top
      gtk_grab_add(mainWindow);
      
      // now loop while there are still misspelled words
      while (bRes) {
	 
	 // show word in main window
	 makeWordVisible();
	 
	 // update dialog with new misspelled word info/suggestions
	 _showMisspelledWord();
	 
	 // run into the GTK event loop for this window
	 gtk_main();
	 
	 for (int i = 0; i < m_Suggestions.count; i++)
	   FREEP(m_Suggestions.word[i]);
	 FREEP(m_Suggestions.word);
	 FREEP(m_Suggestions.score);
	 m_Suggestions.count = 0;
	 
	 if (m_bCancelled) break;
	 
	 // get the next unknown word
	 bRes = nextMisspelledWord();
      }
      
      _storeWindowData();
      
      gtk_widget_destroy(mainWindow);
   }
   
   // TODO: all done message?
   UT_DEBUGMSG(("spelling check complete."));
#endif
}

/**********************************************************/
#if 0
static void s_change_clicked(GtkWidget * widget, AP_QNXDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_Change();
}

static void s_change_all_clicked(GtkWidget * widget, AP_QNXDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_ChangeAll();
}

static void s_ignore_clicked(GtkWidget * widget, AP_QNXDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_Ignore();
}

static void s_ignore_all_clicked(GtkWidget * widget, AP_QNXDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_IgnoreAll();
}

static void s_add_to_dict_clicked(GtkWidget * widget, AP_QNXDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_AddToDict();
}

static void s_cancel_clicked(GtkWidget * widget, AP_QNXDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_Cancel();
}

static void s_suggestion_selected(GtkWidget * widget, int row, int column,
				  GdkEventButton * /*event*/, AP_QNXDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_SuggestionSelected(row, column);
}

static void s_replacement_changed(GtkWidget * widget, AP_QNXDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_ReplacementChanged();
}

static void s_delete_clicked(GtkWidget * /* widget */,
			     gpointer /* data */,
			     AP_QNXDialog_Spell * dlg)
{
   UT_ASSERT(dlg);
   dlg->event_Cancel();
}
#endif      
/********************************************************************/
#if 0
GtkWidget * AP_QNXDialog_Spell::_constructWindow(void)
{
   GtkWidget *windowSpell;
   GtkWidget *tableMain;
   
   GtkWidget *label1;
   GtkWidget *scroll2;
   GtkWidget *textWord;
   GtkWidget *label2;
   GtkWidget *entryChange;
   GtkWidget *scroll1;
   GtkWidget *clistSuggestions;
   GtkWidget *buttonChange;
   GtkWidget *buttonChangeAll;
   GtkWidget *buttonIgnore;
   GtkWidget *buttonIgnoreAll;
   GtkWidget *buttonAddToDict;
   GtkWidget *buttonCancel;

   const XAP_StringSet * pSS = m_pApp->getStringSet();
   XML_Char * unixstr = NULL;      // used for conversions
   
   windowSpell = gtk_window_new (GTK_WINDOW_DIALOG);
   gtk_object_set_data (GTK_OBJECT (windowSpell), "windowSpell", windowSpell);
   gtk_window_set_title (GTK_WINDOW (windowSpell),  pSS->getValue(AP_STRING_ID_DLG_Spell_SpellTitle));
   gtk_window_set_policy (GTK_WINDOW (windowSpell), TRUE, TRUE, FALSE);

   tableMain = gtk_table_new (10, 3, FALSE);
   gtk_widget_ref (tableMain);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "tableMain", tableMain,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (tableMain);
   gtk_container_add (GTK_CONTAINER (windowSpell), tableMain);
   gtk_container_set_border_width (GTK_CONTAINER(tableMain), 5);
   gtk_table_set_row_spacings (GTK_TABLE(tableMain), 2);
   gtk_table_set_row_spacing (GTK_TABLE(tableMain), 4, 7);
   gtk_table_set_row_spacing (GTK_TABLE(tableMain), 8, 7);
   gtk_table_set_col_spacings (GTK_TABLE(tableMain), 2);
   
   GtkWidget * alignmentLabel = gtk_alignment_new (0, 1, 0, 0);
   gtk_widget_ref (alignmentLabel);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "alignmentLabel", alignmentLabel,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (alignmentLabel);
   gtk_table_attach (GTK_TABLE(tableMain), alignmentLabel, 0, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_UnknownWord));
   label1 = gtk_label_new (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (label1);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "label1", label1,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (label1);
   gtk_label_set_justify (GTK_LABEL(label1), GTK_JUSTIFY_LEFT);
   gtk_container_add (GTK_CONTAINER(alignmentLabel), label1);
   
   scroll2 = gtk_scrolled_window_new (NULL, NULL);
   gtk_widget_ref (scroll2);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll2),
				   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "scroll2", scroll2,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (scroll2);
   gtk_table_attach_defaults (GTK_TABLE(tableMain), scroll2, 0, 2, 1, 4);   

   textWord = gtk_text_new (NULL, NULL);
   gtk_widget_ref (textWord);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "textWord", textWord,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (textWord);
   gtk_container_add (GTK_CONTAINER (scroll2), textWord);
   gtk_text_set_word_wrap(GTK_TEXT(textWord), TRUE);
   gtk_widget_set_usize (textWord, 350, 80);

   gtk_widget_realize (textWord);

   // ignore button set
   GtkWidget * vboxIgnoreButtons = gtk_vbox_new(FALSE, 5);
   gtk_widget_ref (vboxIgnoreButtons);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "vboxIgnoreButtons", vboxIgnoreButtons,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (vboxIgnoreButtons);
   gtk_table_attach_defaults (GTK_TABLE(tableMain), vboxIgnoreButtons, 2, 3, 1, 4);
   
   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_Ignore));
   buttonIgnore = gtk_button_new_with_label (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (buttonIgnore);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "buttonIgnore", buttonIgnore,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (buttonIgnore);
   gtk_box_pack_start (GTK_BOX(vboxIgnoreButtons), buttonIgnore, FALSE, FALSE, 5);
//   gtk_table_attach (GTK_TABLE(tableMain), buttonIgnore, 2, 3, 1, 2, GTK_FILL, GTK_EXPAND, 2, 0);
   
   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_IgnoreAll));
   buttonIgnoreAll = gtk_button_new_with_label (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (buttonIgnoreAll);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "buttonIgnoreAll", buttonIgnoreAll,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (buttonIgnoreAll);
   gtk_box_pack_start (GTK_BOX(vboxIgnoreButtons), buttonIgnoreAll, FALSE, FALSE, 5);
//   gtk_table_attach (GTK_TABLE(tableMain), buttonIgnoreAll, 2, 3, 2, 3, GTK_FILL, GTK_EXPAND, 2, 5);
   
   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_AddToDict));
   buttonAddToDict = gtk_button_new_with_label (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (buttonAddToDict);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "buttonAddToDict", buttonAddToDict,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (buttonAddToDict);
   gtk_box_pack_start (GTK_BOX(vboxIgnoreButtons), buttonAddToDict, FALSE, FALSE, 5);
//   gtk_table_attach (GTK_TABLE(tableMain), buttonAddToDict, 2, 3, 3, 4, GTK_FILL, GTK_EXPAND, 2, 0);


   // suggestion half
   GtkWidget * hboxChangeTo = gtk_hbox_new(FALSE, 5);
   gtk_widget_ref (hboxChangeTo);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "hboxChangeTo", hboxChangeTo,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (hboxChangeTo);
   gtk_table_attach_defaults (GTK_TABLE(tableMain), hboxChangeTo, 0, 2, 5, 6);

   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_ChangeTo));
   label2 = gtk_label_new (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (label2);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "label2", label2,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (label2);
   gtk_box_pack_start (GTK_BOX(hboxChangeTo), label2, FALSE, FALSE, 5);
//   gtk_table_attach (GTK_TABLE(tableMain), label2, 0, 1, 5, 6, GTK_SHRINK, GTK_SHRINK, 0, 0);
   gtk_label_set_justify( GTK_LABEL(label2), GTK_JUSTIFY_LEFT);
//   gtk_misc_set_padding (GTK_MISC (label2), 5, 0);

   entryChange = gtk_entry_new ();
   gtk_widget_ref (entryChange);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "entryChange", entryChange,
                            (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (entryChange);
   gtk_box_pack_start (GTK_BOX(hboxChangeTo), entryChange, TRUE, TRUE, 0);
//   gtk_table_attach_defaults (GTK_TABLE(tableMain), entryChange, 1, 2, 5, 6);

   scroll1 = gtk_scrolled_window_new (NULL, NULL);
   gtk_widget_ref (scroll1);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll1),
				   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "scroll1", scroll1,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (scroll1);
   gtk_table_attach_defaults (GTK_TABLE(tableMain), scroll1, 0, 2, 6, 9);

   clistSuggestions = gtk_clist_new (1);
   gtk_widget_ref (clistSuggestions);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "clistSuggestions", 
			     clistSuggestions, (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (clistSuggestions);
   gtk_container_add (GTK_CONTAINER (scroll1), clistSuggestions);
   gtk_widget_set_usize (clistSuggestions, -2, 100);
   gtk_clist_set_column_width (GTK_CLIST (clistSuggestions), 0, 80);
   gtk_clist_column_titles_hide (GTK_CLIST (clistSuggestions));

   
   // change buttons
   GtkWidget * vboxChangeButtons = gtk_vbox_new(FALSE, 5);
   gtk_widget_ref (vboxChangeButtons);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "vboxChangeButtons", vboxChangeButtons,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (vboxChangeButtons);
   gtk_table_attach_defaults (GTK_TABLE(tableMain), vboxChangeButtons, 2, 3, 6, 9);

   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_Change));
   buttonChange = gtk_button_new_with_label (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (buttonChange);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "buttonChange", buttonChange,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (buttonChange);
   gtk_box_pack_start (GTK_BOX(vboxChangeButtons), buttonChange, FALSE, FALSE, 5);
//   gtk_table_attach (GTK_TABLE(tableMain), buttonChange, 2, 3, 6, 7, GTK_FILL, GTK_EXPAND, 2, 0);
   
   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_ChangeAll));
   buttonChangeAll = gtk_button_new_with_label (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (buttonChangeAll);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "buttonChangeAll", buttonChangeAll,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (buttonChangeAll);
   gtk_box_pack_start (GTK_BOX(vboxChangeButtons), buttonChangeAll, FALSE, FALSE, 5);
//   gtk_table_attach (GTK_TABLE(tableMain), buttonChangeAll, 2, 3, 7, 8, GTK_FILL, GTK_EXPAND, 2, 5);
   
   
   
   buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
   gtk_widget_ref (buttonCancel);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "buttonCancel", buttonCancel,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (buttonCancel);
   gtk_table_attach (GTK_TABLE(tableMain), buttonCancel, 2, 3, 9, 10, GTK_FILL, GTK_EXPAND, 2, 5);
   
   // connect signals to handlers

   // buttons
   gtk_signal_connect(GTK_OBJECT(buttonChange), "clicked",
		      GTK_SIGNAL_FUNC(s_change_clicked),
		      (gpointer) this);
   gtk_signal_connect(GTK_OBJECT(buttonChangeAll), "clicked",
		      GTK_SIGNAL_FUNC(s_change_all_clicked),
		      (gpointer) this);
   gtk_signal_connect(GTK_OBJECT(buttonIgnore), "clicked",
		      GTK_SIGNAL_FUNC(s_ignore_clicked),
		      (gpointer) this);
   gtk_signal_connect(GTK_OBJECT(buttonIgnoreAll), "clicked",
		      GTK_SIGNAL_FUNC(s_ignore_all_clicked),
		      (gpointer) this);
   gtk_signal_connect(GTK_OBJECT(buttonAddToDict), "clicked",
		      GTK_SIGNAL_FUNC(s_add_to_dict_clicked),
		      (gpointer) this);
   gtk_signal_connect(GTK_OBJECT(buttonCancel), "clicked",
		      GTK_SIGNAL_FUNC(s_cancel_clicked),
		      (gpointer) this);

   // suggestion list
   m_listHandlerID = gtk_signal_connect(GTK_OBJECT(clistSuggestions), "select-row",
		      GTK_SIGNAL_FUNC(s_suggestion_selected),
		      (gpointer) this);
   
   // replacement edited
   m_replaceHandlerID = gtk_signal_connect(GTK_OBJECT(entryChange), "changed",
		      GTK_SIGNAL_FUNC(s_replacement_changed),
		      (gpointer) this);
   
   // the catch-alls
   gtk_signal_connect_after(GTK_OBJECT(windowSpell),
			    "delete_event",
			    GTK_SIGNAL_FUNC(s_delete_clicked),
			    (gpointer) this);
         
   gtk_signal_connect_after(GTK_OBJECT(windowSpell),
			    "destroy",
			    NULL,
			    NULL);
   
   // update member variables with the important widgets 
   // that we'll interact with later
   
   m_buttonChange = buttonChange;
   m_buttonChangeAll = buttonChangeAll;
   m_buttonIgnore = buttonIgnore;
   m_buttonIgnoreAll = buttonIgnoreAll;
   m_buttonAddToDict = buttonAddToDict;
   m_buttonCancel = buttonCancel;
   
   m_textWord = textWord;
   m_entryChange = entryChange;
   m_clistSuggestions = clistSuggestions;
   
   GdkColormap * cm = gdk_colormap_get_system();
   m_highlight.red = 0xffff;
   m_highlight.green = 0x0000;
   m_highlight.blue = 0x0000;
   gdk_colormap_alloc_color(cm, &m_highlight, FALSE, TRUE);
   
   return windowSpell;
}

void AP_QNXDialog_Spell::_showMisspelledWord(void)
{                                
   
   gtk_text_freeze( GTK_TEXT(m_textWord) );
   
   gtk_text_set_point( GTK_TEXT(m_textWord), 0 );
   gtk_text_forward_delete( GTK_TEXT(m_textWord), 
			   gtk_text_get_length( GTK_TEXT(m_textWord) ) );
   
   UT_UCSChar *p;

   // insert start of sentence
   p = _getPreWord();
   gchar * preword = (gchar*) _convertToMB(p);
   FREEP(p);
   gtk_text_insert(GTK_TEXT(m_textWord), NULL, NULL, NULL,
		   preword, strlen(preword));
   FREEP(preword);
   
   // insert misspelled word (in highlight color)
   p = _getCurrentWord();
   gchar * word = (gchar*) _convertToMB(p);
   FREEP(p);
   gtk_text_insert(GTK_TEXT(m_textWord) , NULL, &m_highlight, NULL,
		   word, strlen(word));
   FREEP(word);
   
   // insert end of sentence
   p = _getPostWord();
   gchar * postword = (gchar*) _convertToMB(p);
   FREEP(p);
   gtk_text_insert(GTK_TEXT(m_textWord), NULL, NULL, NULL,
		   postword, strlen(postword));
   FREEP(postword);
   
   // TODO: set scroll position so misspelled word is centered

   gtk_text_thaw( GTK_TEXT(m_textWord) );
   
   gtk_clist_freeze( GTK_CLIST(m_clistSuggestions) );
   
   gtk_clist_clear(GTK_CLIST(m_clistSuggestions));
   
   gchar *suggest[2] = {NULL, NULL};
   
   for (int i = 0; i < m_Suggestions.count; i++) {
      suggest[0] = (gchar*) _convertToMB((UT_UCSChar*)m_Suggestions.word[i]);
      gtk_clist_append( GTK_CLIST(m_clistSuggestions), suggest);
      FREEP(suggest[0]);
   }
   
   if (!m_Suggestions.count) {

      const XAP_StringSet * pSS = m_pApp->getStringSet();
      UT_XML_cloneNoAmpersands(suggest[0], pSS->getValue(AP_STRING_ID_DLG_Spell_NoSuggestions));
      gtk_clist_append( GTK_CLIST(m_clistSuggestions), suggest);
      FREEP(suggest[0]);
      gtk_clist_set_selectable( GTK_CLIST(m_clistSuggestions), 0, FALSE);

      gtk_signal_handler_block(GTK_OBJECT(m_entryChange), m_replaceHandlerID);
      gtk_entry_set_text(GTK_ENTRY(m_entryChange), "");
      gtk_signal_handler_unblock(GTK_OBJECT(m_entryChange), m_replaceHandlerID);

      m_iSelectedRow = -1;
      
   } else {

      // select first on the list; signal handler should update our entry box
      gtk_clist_select_row( GTK_CLIST(m_clistSuggestions), 0, 0);

   }
   
   gtk_clist_thaw( GTK_CLIST(m_clistSuggestions) );
   
}

void AP_QNXDialog_Spell::_populateWindowData(void)
{
   // TODO: initialize list of user dictionaries
}

void AP_QNXDialog_Spell::_storeWindowData(void)
{
   // TODO: anything to store?
}
#endif

/*************************************************************/

void AP_QNXDialog_Spell::event_Change()
{
#if 0
   UT_UCSChar * replace = NULL;
   if (m_iSelectedRow != -1)
     {
	replace = (UT_UCSChar*) m_Suggestions.word[m_iSelectedRow];
	changeWordWith(replace);
     }
   else
     {
	replace = _convertFromMB((char*)gtk_entry_get_text(GTK_ENTRY(m_entryChange)));
	if (!UT_UCS_strlen(replace)) {
	   FREEP(replace);
	   return;
	}
	changeWordWith(replace);
	FREEP(replace);
     }

   gtk_main_quit();
#endif
}

void AP_QNXDialog_Spell::event_ChangeAll()
{
#if 0
   UT_UCSChar * replace = NULL;
   if (m_iSelectedRow != -1)
     {
	replace = (UT_UCSChar*) m_Suggestions.word[m_iSelectedRow];
	addChangeAll(replace);
	changeWordWith(replace);
     }
   else
     {
	replace = _convertFromMB((char*)gtk_entry_get_text(GTK_ENTRY(m_entryChange)));
	if (!UT_UCS_strlen(replace)) {
	   FREEP(replace);
	   return;
	}
	addChangeAll(replace);
	changeWordWith(replace);
	FREEP(replace);
     }
   
   gtk_main_quit();
#endif
}

void AP_QNXDialog_Spell::event_Ignore()
{
#if 0
   ignoreWord();
   gtk_main_quit();
#endif
}

void AP_QNXDialog_Spell::event_IgnoreAll()
{
#if 0
   addIgnoreAll();
   ignoreWord();
   gtk_main_quit();
#endif
}

void AP_QNXDialog_Spell::event_AddToDict()
{
#if 0
   addToDict();
   
   ignoreWord();
   gtk_main_quit();
#endif
}

void AP_QNXDialog_Spell::event_Cancel()
{
#if 0
   m_bCancelled = UT_TRUE;
   gtk_main_quit();
#endif
}

void AP_QNXDialog_Spell::event_SuggestionSelected(int row, int column)
{
#if 0
   if (!m_Suggestions.count) return;
   
   m_iSelectedRow = row;
   
   gchar * newreplacement = NULL;
   gtk_clist_get_text(GTK_CLIST(m_clistSuggestions), row, column, &newreplacement);
   UT_ASSERT(newreplacement);

   gtk_signal_handler_block(GTK_OBJECT(m_entryChange), m_replaceHandlerID);
   gtk_entry_set_text(GTK_ENTRY(m_entryChange), newreplacement);
   gtk_signal_handler_unblock(GTK_OBJECT(m_entryChange), m_replaceHandlerID);
#endif
}

void AP_QNXDialog_Spell::event_ReplacementChanged()
{
#if 0
   gchar * modtext = gtk_entry_get_text(GTK_ENTRY(m_entryChange));
   UT_ASSERT(modtext);
   
   gchar * suggest = NULL;
   for (int i = 0; i < GTK_CLIST(m_clistSuggestions)->row_height; i++)
     {
	gtk_clist_get_text(GTK_CLIST(m_clistSuggestions), i, 0, &suggest);
	UT_ASSERT(suggest);
	if (strcmp(modtext, suggest) == 0) {
	   gtk_signal_handler_block(GTK_OBJECT(m_clistSuggestions), m_listHandlerID);
	   gtk_clist_select_row(GTK_CLIST(m_clistSuggestions), i, 0);
	   gtk_signal_handler_unblock(GTK_OBJECT(m_clistSuggestions), m_listHandlerID);
	   return;
	}
     }
   
   gtk_clist_unselect_all(GTK_CLIST(m_clistSuggestions));
   m_iSelectedRow = -1;
#endif
}


// GTK+ uses multibyte strings for i18n
// these functions convert from wide (UCS) to MB and back

// TODO: these are probably generally useful functions,
// TODO: but I don't know about xp support for them.

// make a multibyte encoded version of a string
char * AP_QNXDialog_Spell::_convertToMB(UT_UCSChar *wword)
{
#if 0
   int mbindex = 0, wcindex = 0;
   int mblength = 0;
   int wclength = UT_UCS_strlen(wword);
   while (wcindex < wclength) {
      int len = wctomb(NULL, (wchar_t)(wword[wcindex]));
      UT_ASSERT(len >= 0);
      mblength += len;
      wcindex++;
   }
   wcindex = 0;
   char * word = (char*) calloc(mblength + 1, sizeof(char));
   if (word == NULL) return NULL;
     
   while (mbindex < mblength) {
      int len = wctomb(word+mbindex, (wchar_t)(wword[wcindex]));
      UT_ASSERT(len >= 0);
      mbindex += len;
      wcindex++;
   }
   word[mblength] = 0;

//   UT_DEBUGMSG(("wc2mb: wc %i/%i - mb %i/%i", wcindex, wclength, mbindex, mblength));
   UT_ASSERT(mblength >= mbindex);
   
   return word;
#endif
	return(NULL);
}

// make a wide string from a multibyte string
UT_UCSChar * AP_QNXDialog_Spell::_convertFromMB(char *word)
{
#if 0
   int wcindex = 0, mbindex = 0;
   int wclength = 0;
   int mblength = strlen(word);
   while (mbindex < mblength) {
      int len = mblen(word+mbindex, mblength-mbindex);
      UT_ASSERT(len >= 0);
      mbindex += len;
      wclength++;
   }
   mbindex = 0;

   wchar_t wch; // we only support two bytes, so this is a temp Unicode char holder
   UT_UCSChar * wword = (UT_UCSChar*) calloc(wclength + 1, sizeof(UT_UCSChar));
   if (wword == NULL) return NULL;
     
   while (wcindex < wclength) {
      int len = mbtowc(&wch, word+mbindex, mblength-mbindex);
      UT_ASSERT(len >= 0);
      mbindex += len;
      wword[wcindex++] = (UT_UCSChar) wch;
   }
   wword[wclength] = 0;
   
//   UT_DEBUGMSG(("mb2wc: mb %i/%i - wc %i/%i", mbindex, mblength, wcindex, wclength));
   UT_ASSERT(wclength >= wcindex);
   
   return (UT_UCSChar*)wword;
#endif
	return(NULL);
}
