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

#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Spell.h"
#include "ap_UnixDialog_Spell.h"


XAP_Dialog * AP_UnixDialog_Spell::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
   AP_UnixDialog_Spell * p = new AP_UnixDialog_Spell(pFactory,id);
   return p;
}

AP_UnixDialog_Spell::AP_UnixDialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : AP_Dialog_Spell(pDlgFactory,id)
{
}

AP_UnixDialog_Spell::~AP_UnixDialog_Spell(void)
{
}

/************************************************************/
void AP_UnixDialog_Spell::runModal(XAP_Frame * pFrame)
{
   UT_DEBUGMSG(("beginning spelling check..."));
   
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
      XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
      UT_ASSERT(pUnixFrame);
      
      // Get the GtkWindow of the parent frame
      GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
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
}

/**********************************************************/

static void s_change_clicked(GtkWidget * widget, AP_UnixDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_Change();
}

static void s_change_all_clicked(GtkWidget * widget, AP_UnixDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_ChangeAll();
}

static void s_ignore_clicked(GtkWidget * widget, AP_UnixDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_Ignore();
}

static void s_ignore_all_clicked(GtkWidget * widget, AP_UnixDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_IgnoreAll();
}

static void s_add_to_dict_clicked(GtkWidget * widget, AP_UnixDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_AddToDict();
}

static void s_cancel_clicked(GtkWidget * widget, AP_UnixDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_Cancel();
}

static void s_suggestion_selected(GtkWidget * widget, gint row, gint column,
				  GdkEventButton * /*event*/, AP_UnixDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_SuggestionSelected(row, column);
}

static void s_replacement_changed(GtkWidget * widget, AP_UnixDialog_Spell * dlg)
{
   UT_ASSERT(widget && dlg);
   dlg->event_ReplacementChanged();
}

static void s_delete_clicked(GtkWidget * /* widget */,
			     gpointer /* data */,
			     AP_UnixDialog_Spell * dlg)
{
   UT_ASSERT(dlg);
   dlg->event_Cancel();
}
      
/********************************************************************/

GtkWidget * AP_UnixDialog_Spell::_constructWindow(void)
{
   GtkWidget *windowSpell;
   GtkWidget *hbox1;
   GtkWidget *vbox2;
   GtkWidget *label1;
   GtkWidget *scroll2;
   GtkWidget *textWord;
   GtkWidget *hbox2;
   GtkWidget *label2;
   GtkWidget *entryChange;
   GtkWidget *scroll1;
   GtkWidget *clistSuggestions;
   GtkWidget *vbox3;
   GtkWidget *buttonChange;
   GtkWidget *buttonChangeAll;
   GtkWidget *hseparator1;
   GtkWidget *buttonIgnore;
   GtkWidget *buttonIgnoreAll;
   GtkWidget *hseparator2;
   GtkWidget *buttonAddToDict;
   GtkWidget *comboDictList;
   GtkWidget *combo_entryDict;
   GtkWidget *alignment1;
   GtkWidget *buttonCancel;

   const XAP_StringSet * pSS = m_pApp->getStringSet();
   XML_Char * unixstr = NULL;      // used for conversions
   
   windowSpell = gtk_window_new (GTK_WINDOW_DIALOG);
   gtk_object_set_data (GTK_OBJECT (windowSpell), "windowSpell", windowSpell);
   gtk_window_set_title (GTK_WINDOW (windowSpell),  pSS->getValue(AP_STRING_ID_DLG_Spell_SpellTitle));
   gtk_window_set_policy (GTK_WINDOW (windowSpell), FALSE, FALSE, FALSE);

   hbox1 = gtk_hbox_new (FALSE, 0);
   gtk_widget_ref (hbox1);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "hbox1", hbox1,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (hbox1);
   gtk_container_add (GTK_CONTAINER (windowSpell), hbox1);

   vbox2 = gtk_vbox_new (FALSE, 2);
   gtk_widget_ref (vbox2);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "vbox2", vbox2,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (vbox2);
   gtk_box_pack_start (GTK_BOX (hbox1), vbox2, TRUE, TRUE, 0);
   gtk_container_set_border_width (GTK_CONTAINER (vbox2), 2);

   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_UnknownWord));
   label1 = gtk_label_new (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (label1);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "label1", label1,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (label1);
   gtk_box_pack_start (GTK_BOX (vbox2), label1, FALSE, FALSE, 0);

   scroll2 = gtk_scrolled_window_new (NULL, NULL);
   gtk_widget_ref (scroll2);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll2),
				   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "scroll2", scroll2,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (scroll2);
   gtk_box_pack_start (GTK_BOX (vbox2), scroll2, FALSE, TRUE, 0);
   
   textWord = gtk_text_new (NULL, NULL);
   gtk_widget_ref (textWord);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "textWord", textWord,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (textWord);
   gtk_container_add (GTK_CONTAINER (scroll2), textWord);
   gtk_text_set_word_wrap(GTK_TEXT(textWord), TRUE);
   gtk_widget_set_usize (textWord, 200, 80);

   gtk_widget_realize (textWord);

   hbox2 = gtk_hbox_new (FALSE, 0);
   gtk_widget_ref (hbox2);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "hbox2", hbox2,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (hbox2);
   gtk_box_pack_start (GTK_BOX (vbox2), hbox2, FALSE, FALSE, 0);

   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_ChangeTo));
   label2 = gtk_label_new (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (label2);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "label2", label2,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (label2);
   gtk_box_pack_start (GTK_BOX (hbox2), label2, FALSE, FALSE, 0);
   gtk_misc_set_padding (GTK_MISC (label2), 5, 0);

   entryChange = gtk_entry_new ();
   gtk_widget_ref (entryChange);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "entryChange", entryChange,
                            (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (entryChange);
   gtk_box_pack_start (GTK_BOX (hbox2), entryChange, TRUE, TRUE, 0);


   scroll1 = gtk_scrolled_window_new (NULL, NULL);
   gtk_widget_ref (scroll1);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll1),
				   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "scroll1", scroll1,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (scroll1);
   gtk_box_pack_end (GTK_BOX (vbox2), scroll1, TRUE, TRUE, 0);

   clistSuggestions = gtk_clist_new (1);
   gtk_widget_ref (clistSuggestions);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "clistSuggestions", 
			     clistSuggestions, (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (clistSuggestions);
   gtk_container_add (GTK_CONTAINER (scroll1), clistSuggestions);
   gtk_widget_set_usize (clistSuggestions, -2, 100);
   gtk_clist_set_column_width (GTK_CLIST (clistSuggestions), 0, 80);
   gtk_clist_column_titles_hide (GTK_CLIST (clistSuggestions));

   
   vbox3 = gtk_vbox_new (FALSE, 9);
   gtk_widget_ref (vbox3);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "vbox3", vbox3,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (vbox3);
   gtk_box_pack_start (GTK_BOX (hbox1), vbox3, FALSE, TRUE, 0);
   gtk_container_set_border_width (GTK_CONTAINER (vbox3), 6);

   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_Change));
   buttonChange = gtk_button_new_with_label (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (buttonChange);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "buttonChange", buttonChange,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (buttonChange);
   gtk_box_pack_start (GTK_BOX (vbox3), buttonChange, FALSE, FALSE, 0);

   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_ChangeAll));
   buttonChangeAll = gtk_button_new_with_label (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (buttonChangeAll);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "buttonChangeAll", buttonChangeAll,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (buttonChangeAll);
   gtk_box_pack_start (GTK_BOX (vbox3), buttonChangeAll, FALSE, FALSE, 0);

   hseparator1 = gtk_hseparator_new ();
   gtk_widget_ref (hseparator1);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "hseparator1", hseparator1,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (hseparator1);
   gtk_box_pack_start (GTK_BOX (vbox3), hseparator1, FALSE, FALSE, 0);

   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_Ignore));
   buttonIgnore = gtk_button_new_with_label (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (buttonIgnore);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "buttonIgnore", buttonIgnore,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (buttonIgnore);
   gtk_box_pack_start (GTK_BOX (vbox3), buttonIgnore, FALSE, FALSE, 0);
   
   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_IgnoreAll));
   buttonIgnoreAll = gtk_button_new_with_label (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (buttonIgnoreAll);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "buttonIgnoreAll", buttonIgnoreAll,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (buttonIgnoreAll);
   gtk_box_pack_start (GTK_BOX (vbox3), buttonIgnoreAll, FALSE, FALSE, 0);
   
   hseparator2 = gtk_hseparator_new ();
   gtk_widget_ref (hseparator2);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "hseparator2", hseparator2,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (hseparator2);
   gtk_box_pack_start (GTK_BOX (vbox3), hseparator2, FALSE, FALSE, 0);
   
   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_AddToDict));
   buttonAddToDict = gtk_button_new_with_label (unixstr);
   FREEP(unixstr);
   gtk_widget_ref (buttonAddToDict);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "buttonAddToDict", buttonAddToDict,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (buttonAddToDict);
   gtk_box_pack_start (GTK_BOX (vbox3), buttonAddToDict, FALSE, FALSE, 0);
   
   comboDictList = gtk_combo_new ();
   gtk_widget_ref (comboDictList);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "comboDictList", comboDictList,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (comboDictList);
   gtk_box_pack_start (GTK_BOX (vbox3), comboDictList, FALSE, FALSE, 0);

   combo_entryDict = GTK_COMBO (comboDictList)->entry;
   gtk_widget_ref (combo_entryDict);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "combo_entryDict", combo_entryDict,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (combo_entryDict);
   
   alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
   gtk_widget_ref (alignment1);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "alignment1", alignment1,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (alignment1);
   gtk_box_pack_end (GTK_BOX (vbox3), alignment1, FALSE, FALSE, 0);
   
   buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
   gtk_widget_ref (buttonCancel);
   gtk_object_set_data_full (GTK_OBJECT (windowSpell), "buttonCancel", buttonCancel,
			     (GtkDestroyNotify) gtk_widget_unref);
   gtk_widget_show (buttonCancel);
   gtk_container_add (GTK_CONTAINER (alignment1), buttonCancel);
   
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
   m_comboDictList = comboDictList;
   
   GdkColormap * cm = gdk_colormap_get_system();
   m_highlight.red = 0xffff;
   m_highlight.green = 0x0000;
   m_highlight.blue = 0x0000;
   gdk_colormap_alloc_color(cm, &m_highlight, FALSE, TRUE);
   
   return windowSpell;
}

void AP_UnixDialog_Spell::_showMisspelledWord(void)
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

void AP_UnixDialog_Spell::_populateWindowData(void)
{
   // TODO: initialize list of user dictionaries
}

void AP_UnixDialog_Spell::_storeWindowData(void)
{
   // TODO: anything to store?
}

/*************************************************************/

void AP_UnixDialog_Spell::event_Change()
{
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
}

void AP_UnixDialog_Spell::event_ChangeAll()
{
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
}

void AP_UnixDialog_Spell::event_Ignore()
{
   ignoreWord();
   gtk_main_quit();
}

void AP_UnixDialog_Spell::event_IgnoreAll()
{
   addIgnoreAll();
   ignoreWord();
   gtk_main_quit();
}

void AP_UnixDialog_Spell::event_AddToDict()
{
   UT_UCSChar * dict = _convertFromMB(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(m_comboDictList)->entry)));
   
   addToDict(dict);
   FREEP(dict);
   
   ignoreWord();
   gtk_main_quit();
}

void AP_UnixDialog_Spell::event_Cancel()
{
   m_bCancelled = UT_TRUE;
   gtk_main_quit();
}

void AP_UnixDialog_Spell::event_SuggestionSelected(gint row, gint column)
{
   if (!m_Suggestions.count) return;
   
   m_iSelectedRow = row;
   
   gchar * newreplacement = NULL;
   gtk_clist_get_text(GTK_CLIST(m_clistSuggestions), row, column, &newreplacement);
   UT_ASSERT(newreplacement);

   gtk_signal_handler_block(GTK_OBJECT(m_entryChange), m_replaceHandlerID);
   gtk_entry_set_text(GTK_ENTRY(m_entryChange), newreplacement);
   gtk_signal_handler_unblock(GTK_OBJECT(m_entryChange), m_replaceHandlerID);
}

void AP_UnixDialog_Spell::event_ReplacementChanged()
{
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
}


// GTK+ uses multibyte strings for i18n
// these functions convert from wide (UCS) to MB and back

// TODO: these are probably generally useful functions,
// TODO: but I don't know about xp support for them.

// make a multibyte encoded version of a string
char * AP_UnixDialog_Spell::_convertToMB(UT_UCSChar *wword)
{
   int mbindex = 0, wcindex = 0;
   int mblen = 0;
   while (wword[wcindex]) {
      int len = wctomb(NULL, wword[wcindex++]);
      mblen += len;
   }
   wcindex = 0;
   int wclen = UT_UCS_strlen(wword);
   char * word = (char*) calloc(mblen + 1, sizeof(char));
   if (word == NULL) return NULL;
     
   while (wword[wcindex]) {
      int len = wctomb(word+mbindex, (wchar_t)wword[wcindex++]);
      mbindex += len; 
   }
   word[mblen] = 0;

   UT_ASSERT(mblen >= mbindex);
   
   return word;
}

// make a wide string from a multibyte string
UT_UCSChar * AP_UnixDialog_Spell::_convertFromMB(char *word)
{
   int wcindex = 0, mbindex = 0;
   int wclen = 0;
   while (word[mbindex]) {
      int len = mbtowc(NULL, word+mbindex, 10);
      mbindex += len;
      wclen++;
   }
   mbindex = 0;
   int mblen = strlen(word);

   wchar_t wch; // we only support two bytes, so this is a temp Unicode char holder
   UT_UCSChar * wword = (UT_UCSChar*) calloc(wclen + 1, sizeof(UT_UCSChar));
   if (wword == NULL) return NULL;
     
   while (word[mbindex]) {
      int len = mbtowc(&wch, word+mbindex, 10);
      mbindex += len;
      wword[wcindex++] = (UT_UCSChar) wch;
   }
   wword[wclen] = 0;
   
   UT_ASSERT(wclen >= wcindex);
   
   return (UT_UCSChar*)wword;
}
