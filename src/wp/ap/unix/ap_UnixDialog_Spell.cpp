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
#include <string.h>

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
   UT_DEBUGMSG(("beginning spelling check...\n"));
   
   // class the base class method to initialize some basic xp stuff
   AP_Dialog_Spell::runModal(pFrame);
   
   m_bCancelled = false;
   bool bRes = nextMisspelledWord();
   
   if (bRes) { // we need to prepare the dialog
      GtkWidget * mainWindow = _constructWindow();
      UT_ASSERT(mainWindow);
      
      connectFocus(GTK_WIDGET(mainWindow),pFrame);
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
      
      // Show the top level dialog
      gtk_widget_show_all(mainWindow);

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
	 
	 _purgeSuggestions();
	 
	 if (m_bCancelled) break;
	 
	 // get the next unknown word
	 bRes = nextMisspelledWord();
      }
      
      _storeWindowData();
      
      if(mainWindow && GTK_IS_WIDGET(mainWindow))
	gtk_widget_destroy(mainWindow);
   }
   
   // TODO: all done message?
   UT_DEBUGMSG(("spelling check complete.\n"));
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
   GtkWidget *vbox;

   const XAP_StringSet * pSS = m_pApp->getStringSet();
   
   //windowSpell = gtk_window_new (GTK_WINDOW_DIALOG);
   windowSpell = gtk_dialog_new();
   gtk_window_set_title (GTK_WINDOW (windowSpell),  pSS->getValue(AP_STRING_ID_DLG_Spell_SpellTitle));
   gtk_window_set_policy (GTK_WINDOW (windowSpell), TRUE, TRUE, FALSE);

   // *very* important to add the vbox
   // to the window so that it gets a
   // gdkwindow associated with it
   //
   m_windowMain = windowSpell;
   vbox = GTK_DIALOG(m_windowMain)->vbox;

   _constructWindowContents(vbox);
   _connectSignals();

   gtk_widget_show_all(windowSpell);
   return windowSpell;
}

void AP_UnixDialog_Spell::_constructWindowContents(GtkWidget *box)
{
   GtkWidget *tableMain;
   
   GtkWidget *label1;
   GtkWidget *scroll2;
   GtkWidget *label2;
   GtkWidget *scroll1;

   const XAP_StringSet * pSS = m_pApp->getStringSet();
   XML_Char * unixstr = NULL;      // used for conversions

   // create the buttons right away
   _createButtons();

   tableMain = gtk_table_new (10, 3, FALSE);
   gtk_container_add (GTK_CONTAINER (box), tableMain);
   gtk_container_set_border_width (GTK_CONTAINER(tableMain), 5);
   gtk_table_set_row_spacings (GTK_TABLE(tableMain), 2);
   gtk_table_set_row_spacing (GTK_TABLE(tableMain), 4, 7);
   gtk_table_set_row_spacing (GTK_TABLE(tableMain), 8, 7);
   gtk_table_set_col_spacings (GTK_TABLE(tableMain), 2);

   GtkWidget * alignmentLabel = gtk_alignment_new (0, 1, 0, 0);
   gtk_table_attach (GTK_TABLE(tableMain), alignmentLabel, 0, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_UnknownWord));
   label1 = gtk_label_new (unixstr);
   FREEP(unixstr);
   gtk_label_set_justify (GTK_LABEL(label1), GTK_JUSTIFY_LEFT);
   gtk_container_add (GTK_CONTAINER(alignmentLabel), label1);

   scroll2 = gtk_scrolled_window_new (NULL, NULL);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll2),
				   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   gtk_table_attach_defaults (GTK_TABLE(tableMain), scroll2, 0, 2, 1, 4);

   m_textWord = gtk_text_new (NULL, NULL);
   gtk_widget_ref (m_textWord);
   gtk_container_add (GTK_CONTAINER (scroll2), m_textWord);
   gtk_text_set_word_wrap(GTK_TEXT(m_textWord), TRUE);
   gtk_widget_set_usize (m_textWord, 350, 80);
   gtk_widget_realize (m_textWord);

   // ignore button set
   GtkWidget * vboxIgnoreButtons = gtk_vbox_new(FALSE, 5);
   gtk_table_attach_defaults (GTK_TABLE(tableMain), vboxIgnoreButtons, 2, 3, 1, 4);
   
   gtk_box_pack_start (GTK_BOX(vboxIgnoreButtons), m_buttonIgnore, FALSE, FALSE, 5);
   gtk_box_pack_start (GTK_BOX(vboxIgnoreButtons), m_buttonIgnoreAll, FALSE, FALSE, 5);
   gtk_box_pack_start (GTK_BOX(vboxIgnoreButtons), m_buttonAddToDict, FALSE, FALSE, 5);

   // suggestion half
   GtkWidget * hboxChangeTo = gtk_hbox_new(FALSE, 5);
   gtk_table_attach_defaults (GTK_TABLE(tableMain), hboxChangeTo, 0, 2, 5, 6);

   UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Spell_ChangeTo));
   label2 = gtk_label_new (unixstr);
   FREEP(unixstr);
   gtk_box_pack_start (GTK_BOX(hboxChangeTo), label2, FALSE, FALSE, 5);
   gtk_label_set_justify( GTK_LABEL(label2), GTK_JUSTIFY_LEFT);

   m_entryChange = gtk_entry_new ();
   gtk_box_pack_start (GTK_BOX(hboxChangeTo), m_entryChange, TRUE, TRUE, 0);

   scroll1 = gtk_scrolled_window_new (NULL, NULL);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll1),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   gtk_table_attach_defaults (GTK_TABLE(tableMain), scroll1, 0, 2, 6, 9);

   m_clistSuggestions = gtk_clist_new (1);
   gtk_container_add (GTK_CONTAINER (scroll1), m_clistSuggestions);
   gtk_widget_set_usize (m_clistSuggestions, -2, 100);
   gtk_clist_set_column_width (GTK_CLIST (m_clistSuggestions), 0, 80);
   gtk_clist_column_titles_hide (GTK_CLIST (m_clistSuggestions));
   
   // change button set
   GtkWidget * vboxChangeButtons = gtk_vbox_new(FALSE, 5);
   gtk_table_attach_defaults (GTK_TABLE(tableMain), vboxChangeButtons, 2, 3, 6, 9);

   gtk_box_pack_start (GTK_BOX(vboxChangeButtons), m_buttonChange, FALSE, FALSE, 5);
   gtk_box_pack_start (GTK_BOX(vboxChangeButtons), m_buttonChangeAll, FALSE, FALSE, 5);
   gtk_box_pack_start (GTK_BOX(vboxChangeButtons), m_buttonCancel, FALSE, FALSE, 5);
   // gtk_table_attach (GTK_TABLE(tableMain), buttonCancel, 2, 3, 9, 10, GTK_FILL, GTK_EXPAND, 2, 5);

   // highlight our misspelled word in red
   GdkColormap * cm = gdk_colormap_get_system();
   m_highlight.red = 0xffff;
   m_highlight.green = 0x0000;
   m_highlight.blue = 0x0000;
   gdk_colormap_alloc_color(cm, &m_highlight, FALSE, TRUE);
   
   gtk_widget_show_all(tableMain);
}

// create normal gtk buttons
// override for gnome buttons
void AP_UnixDialog_Spell::_createButtons(void)
{
  const XAP_StringSet * pSS = m_pApp->getStringSet();
  XML_Char * unixstr = NULL;      // used for conversions

  UT_XML_cloneNoAmpersands(unixstr,
			   pSS->getValue(AP_STRING_ID_DLG_Spell_Change));
  m_buttonChange = gtk_button_new_with_label (unixstr);
  FREEP(unixstr);
  
  UT_XML_cloneNoAmpersands(unixstr, 
			   pSS->getValue(AP_STRING_ID_DLG_Spell_ChangeAll));
  m_buttonChangeAll = gtk_button_new_with_label (unixstr);
  FREEP(unixstr);

  UT_XML_cloneNoAmpersands(unixstr, 
			   pSS->getValue(AP_STRING_ID_DLG_Spell_Ignore));
  m_buttonIgnore = gtk_button_new_with_label (unixstr);
  FREEP(unixstr);

  UT_XML_cloneNoAmpersands(unixstr, 
			   pSS->getValue(AP_STRING_ID_DLG_Spell_IgnoreAll));
  m_buttonIgnoreAll = gtk_button_new_with_label (unixstr);
  FREEP(unixstr);

  UT_XML_cloneNoAmpersands(unixstr, 
			   pSS->getValue(AP_STRING_ID_DLG_Spell_AddToDict));
  m_buttonAddToDict = gtk_button_new_with_label (unixstr);
  FREEP(unixstr);

   // add the cancel button
   m_buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
}

void AP_UnixDialog_Spell::_connectSignals(void)
{
   // connect signals to handlers

   // buttons
   gtk_signal_connect(GTK_OBJECT(m_buttonChange), "clicked",
		      GTK_SIGNAL_FUNC(s_change_clicked),
		      (gpointer) this);
   gtk_signal_connect(GTK_OBJECT(m_buttonChangeAll), "clicked",
		      GTK_SIGNAL_FUNC(s_change_all_clicked),
		      (gpointer) this);
   gtk_signal_connect(GTK_OBJECT(m_buttonIgnore), "clicked",
		      GTK_SIGNAL_FUNC(s_ignore_clicked),
		      (gpointer) this);
   gtk_signal_connect(GTK_OBJECT(m_buttonIgnoreAll), "clicked",
		      GTK_SIGNAL_FUNC(s_ignore_all_clicked),
		      (gpointer) this);
   gtk_signal_connect(GTK_OBJECT(m_buttonAddToDict), "clicked",
		      GTK_SIGNAL_FUNC(s_add_to_dict_clicked),
		      (gpointer) this);
   gtk_signal_connect(GTK_OBJECT(m_buttonCancel), "clicked",
		      GTK_SIGNAL_FUNC(s_cancel_clicked),
		      (gpointer) this);

   // suggestion list
   m_listHandlerID = gtk_signal_connect(GTK_OBJECT(m_clistSuggestions), "select-row",
					GTK_SIGNAL_FUNC(s_suggestion_selected),
					(gpointer) this);
   
   // replacement edited
   m_replaceHandlerID = gtk_signal_connect(GTK_OBJECT(m_entryChange), "changed",
					   GTK_SIGNAL_FUNC(s_replacement_changed),
					   (gpointer) this);
   
   // the catch-alls
   gtk_signal_connect(GTK_OBJECT(m_windowMain),
		      "delete_event",
		      GTK_SIGNAL_FUNC(s_delete_clicked),
		      (gpointer) this);
         
   gtk_signal_connect_after(GTK_OBJECT(m_windowMain),
			    "destroy",
			    NULL,
			    NULL);
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

   // insert misspelled word (in highlight color)
   p = _getCurrentWord();
   gchar * word = (gchar*) _convertToMB(p);
   FREEP(p);

   gtk_text_insert(GTK_TEXT(m_textWord) , NULL, &m_highlight, NULL,
		   word, strlen(word));
   
   // insert end of sentence
   p = _getPostWord();
   gchar * postword = (gchar*) _convertToMB(p);
   FREEP(p);
   gtk_text_insert(GTK_TEXT(m_textWord), NULL, NULL, NULL,
		   postword, strlen(postword));

   // TODO: set scroll position so misspelled word is centered

   gtk_text_thaw( GTK_TEXT(m_textWord) );   
   gtk_clist_freeze( GTK_CLIST(m_clistSuggestions) );   
   gtk_clist_clear(GTK_CLIST(m_clistSuggestions));
   
   gchar *suggest[2] = {NULL, NULL};
   
   for (int i = 0; i < m_Suggestions->getItemCount(); i++) {
      suggest[0] = (gchar*) _convertToMB((UT_UCSChar*)m_Suggestions->getNthItem(i));
      gtk_clist_append( GTK_CLIST(m_clistSuggestions), suggest);
   }
   
   if (!m_Suggestions->getItemCount()) {

      const XAP_StringSet * pSS = m_pApp->getStringSet();
      UT_XML_cloneNoAmpersands(suggest[0], pSS->getValue(AP_STRING_ID_DLG_Spell_NoSuggestions));
      gtk_clist_append( GTK_CLIST(m_clistSuggestions), suggest);
      FREEP(suggest[0]);
      gtk_clist_set_selectable(GTK_CLIST(m_clistSuggestions), 0, FALSE);

      gtk_signal_handler_block(GTK_OBJECT(m_entryChange), m_replaceHandlerID);
      gtk_entry_set_text(GTK_ENTRY(m_entryChange), "");
      gtk_signal_handler_unblock(GTK_OBJECT(m_entryChange), m_replaceHandlerID);

      m_iSelectedRow = -1;
      
   } else {

      // select first on the list; signal handler should update our entry box
      gtk_clist_select_row(GTK_CLIST(m_clistSuggestions), 0, 0);

   }
   
   gtk_clist_thaw(GTK_CLIST(m_clistSuggestions) );
   
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
	UT_DEBUGMSG(("m_iSelectedRow is %i\n", m_iSelectedRow));
	if (m_iSelectedRow != -1)
	{
		replace = (UT_UCSChar*) m_Suggestions->getNthItem(m_iSelectedRow);
		UT_DEBUGMSG(("Replacing with %s\n", (char*) replace));
		//fprintf(stderr, "Replacing with %s\n", replace);
		changeWordWith(replace);		
	}
	else
	  {
	        replace = _convertFromMB((char*)gtk_entry_get_text(GTK_ENTRY(m_entryChange)));
		if (!UT_UCS_strlen(replace)) {
		  UT_DEBUGMSG(("replace is 0 length\n"));
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
	replace = (UT_UCSChar*) m_Suggestions->getNthItem(m_iSelectedRow);
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
   addToDict();
   
   ignoreWord();
   gtk_main_quit();
}

void AP_UnixDialog_Spell::event_Cancel()
{
   m_bCancelled = true;
   gtk_main_quit();
}

void AP_UnixDialog_Spell::event_SuggestionSelected(gint row, gint column)
{
   if (!m_Suggestions->getItemCount()) return;
   
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
	char *word = (char *) malloc (UT_UCS_strlen(wword)*2);
	UT_UCS_strcpy_to_char(word,wword);
	return word;
}

// make a wide string from a multibyte string
UT_UCSChar * AP_UnixDialog_Spell::_convertFromMB(char *word)
{
	UT_UCSChar *wword = (UT_UCSChar *) malloc (strlen(word)*2);
	UT_UCS_strcpy_char(wword,word);
	return wword;
}
