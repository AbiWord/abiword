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

// for gtkclist
#undef GTK_DISABLE_DEPRECATED

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
    // class the base class method to initialize some basic xp stuff
    AP_Dialog_Spell::runModal(pFrame);
   
    m_bCancelled = false;
    bool bRes = nextMisspelledWord();
   
    if (bRes) { // we need to prepare the dialog
        GtkWidget * mainWindow = _constructWindow();
        UT_ASSERT(mainWindow);

        // Populate the window's data items
        _populateWindowData();
      
        abiSetupModalDialog(GTK_DIALOG(mainWindow), pFrame, this, BUTTON_CANCEL);

        // now loop while there are still misspelled words
        while (bRes) {
     
            // show word in main window
            makeWordVisible();
     
            // update dialog with new misspelled word info/suggestions
            _showMisspelledWord();
     
            // run into the GTK event loop for this window
            switch(abiRunModalDialog(GTK_DIALOG(mainWindow), false))
            {
            case BUTTON_CHANGE:
                event_Change(); break ;
            case BUTTON_CHANGE_ALL:
                event_ChangeAll(); break ;
            case BUTTON_IGNORE:
                event_Ignore(); break;
            case BUTTON_IGNORE_ALL:
                event_IgnoreAll(); break;
            case BUTTON_ADD:
                event_AddToDict(); break;
            default:
                event_Cancel(); break;
            }
     
            _purgeSuggestions();
     
            if (m_bCancelled) 
                break;
     
            // get the next unknown word
            bRes = nextMisspelledWord();
        }
      
        _storeWindowData();
      
        abiDestroyWidget(mainWindow);
    }
}

/**********************************************************/

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

/********************************************************************/

GtkWidget * AP_UnixDialog_Spell::_constructWindow(void)
{
    GtkWidget *windowSpell;
    GtkWidget *vbox;

    const XAP_StringSet * pSS = m_pApp->getStringSet();
   
    windowSpell = abiDialogNew("spelling dialog", TRUE, pSS->getValueUTF8(AP_STRING_ID_DLG_Spell_SpellTitle).utf8_str());

    // *very* important to add the vbox
    // to the window so that it gets a
    // gdkwindow associated with it
    //
    m_windowMain = windowSpell;
    vbox = GTK_DIALOG(m_windowMain)->vbox;
   
    XML_Char * unixstr = NULL;      // used for conversions

    m_buttonCancel = abiAddStockButton(GTK_DIALOG(windowSpell), GTK_STOCK_CANCEL, BUTTON_CANCEL);

    UT_XML_cloneNoAmpersands(unixstr,
                             pSS->getValueUTF8(AP_STRING_ID_DLG_Spell_Change).utf8_str());
    m_buttonChange = abiAddButton (GTK_DIALOG(windowSpell), unixstr, BUTTON_CHANGE);
    FREEP(unixstr);
  
    UT_XML_cloneNoAmpersands(unixstr, 
                             pSS->getValueUTF8(AP_STRING_ID_DLG_Spell_ChangeAll).utf8_str());
    m_buttonChangeAll = abiAddButton (GTK_DIALOG(windowSpell), unixstr, BUTTON_CHANGE_ALL);
    FREEP(unixstr);

    UT_XML_cloneNoAmpersands(unixstr, 
                             pSS->getValueUTF8(AP_STRING_ID_DLG_Spell_Ignore).utf8_str());
    m_buttonIgnore = abiAddButton (GTK_DIALOG(windowSpell), unixstr, BUTTON_IGNORE);
    FREEP(unixstr);

    UT_XML_cloneNoAmpersands(unixstr, 
                             pSS->getValueUTF8(AP_STRING_ID_DLG_Spell_IgnoreAll).utf8_str());
    m_buttonIgnoreAll = abiAddButton (GTK_DIALOG(windowSpell), unixstr, BUTTON_IGNORE_ALL);
    FREEP(unixstr);

    m_buttonAddToDict = abiAddStockButton (GTK_DIALOG(windowSpell), GTK_STOCK_ADD, BUTTON_ADD);

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

    UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_Spell_UnknownWord).utf8_str());
    label1 = gtk_label_new (unixstr);
    FREEP(unixstr);
    gtk_label_set_justify (GTK_LABEL(label1), GTK_JUSTIFY_LEFT);
    gtk_container_add (GTK_CONTAINER(alignmentLabel), label1);

    scroll2 = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll2),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_table_attach_defaults (GTK_TABLE(tableMain), scroll2, 0, 2, 1, 4);

    m_textWord = gtk_text_view_new ();
    gtk_widget_ref (m_textWord);
    gtk_text_view_set_editable (GTK_TEXT_VIEW(m_textWord), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(m_textWord), FALSE);
    gtk_container_add (GTK_CONTAINER (scroll2), m_textWord);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_textWord), GTK_WRAP_WORD);
    gtk_widget_set_size_request (m_textWord, 350, 80);
    gtk_widget_realize (m_textWord);

    // suggestion half
    GtkWidget * hboxChangeTo = gtk_hbox_new(FALSE, 5);
    gtk_table_attach_defaults (GTK_TABLE(tableMain), hboxChangeTo, 0, 2, 5, 6);

    UT_XML_cloneNoAmpersands(unixstr, pSS->getValueUTF8(AP_STRING_ID_DLG_Spell_ChangeTo).utf8_str());
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
    gtk_widget_set_size_request (m_clistSuggestions, -2, 100);
    gtk_clist_set_column_width (GTK_CLIST (m_clistSuggestions), 0, 80);
    gtk_clist_column_titles_hide (GTK_CLIST (m_clistSuggestions));   

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
}

void AP_UnixDialog_Spell::_connectSignals(void)
{
    // connect signals to handlers

    // suggestion list
    m_listHandlerID = g_signal_connect(G_OBJECT(m_clistSuggestions), "select-row",
                                       G_CALLBACK(s_suggestion_selected),
                                       (gpointer) this);
   
    // replacement edited
    m_replaceHandlerID = g_signal_connect(G_OBJECT(m_entryChange), "changed",
                                          G_CALLBACK(s_replacement_changed),
                                          (gpointer) this);
}

void AP_UnixDialog_Spell::_showMisspelledWord(void)
{             
    GtkTextBuffer * buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_textWord));
    GtkTextIter iter;

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
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert_with_tags(buffer, &iter, word, -1, txt_tag, NULL);
	// word is freed at the end of the method...
	
    // insert end of sentence
    p = m_pWordIterator->getPostWord(iLength);
	if (0 < iLength)
	{
		gchar * postword = (gchar*) _convertToMB(p, iLength);
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_insert(buffer, &iter, postword, -1);
		FREEP(postword);
	}
    else
    {
        // Insert space to make gtk_text_buffer understand that it
        // really should highlight the selected word. This is a
        // workaround for bug 5459. It really should be fixed in GTK.
        gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_insert(buffer, &iter, " ", -1);
    }
    // TODO: set scroll position so misspelled word is centered

    gtk_clist_freeze( GTK_CLIST(m_clistSuggestions) );   
    gtk_clist_clear(GTK_CLIST(m_clistSuggestions));
   
    gchar *suggest[2] = {NULL, NULL};
   
    for (UT_uint32 i = 0; i < m_Suggestions->getItemCount(); i++) {
        suggest[0] = (gchar*) _convertToMB((UT_UCSChar*)m_Suggestions->getNthItem(i));
        gtk_clist_append( GTK_CLIST(m_clistSuggestions), suggest);
    }
   
    if (!m_Suggestions->getItemCount()) {

        const XAP_StringSet * pSS = m_pApp->getStringSet();
        UT_XML_cloneNoAmpersands(suggest[0], pSS->getValueUTF8(AP_STRING_ID_DLG_Spell_NoSuggestions).utf8_str());
        gtk_clist_append( GTK_CLIST(m_clistSuggestions), suggest);
        FREEP(suggest[0]);
        gtk_clist_set_selectable(GTK_CLIST(m_clistSuggestions), 0, FALSE);

        g_signal_handler_block(G_OBJECT(m_entryChange), m_replaceHandlerID);
        gtk_entry_set_text(GTK_ENTRY(m_entryChange), word);
        g_signal_handler_unblock(G_OBJECT(m_entryChange), m_replaceHandlerID);

        m_iSelectedRow = -1;
      
    } else {

        // select first on the list; signal handler should update our entry box
        gtk_clist_select_row(GTK_CLIST(m_clistSuggestions), 0, 0);

    }
   
    gtk_clist_thaw(GTK_CLIST(m_clistSuggestions) );
    FREEP(word);
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
        if (!replace || !UT_UCS4_strlen(replace)) {
            UT_DEBUGMSG(("replace is 0 length\n"));
            FREEP(replace);
            return;
        }
        changeWordWith(replace);
        FREEP(replace);
    }   
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
        if (!replace || !UT_UCS4_strlen(replace)) {
            FREEP(replace);
            return;
        }
        addChangeAll(replace);
        changeWordWith(replace);
        FREEP(replace);
    }
}

void AP_UnixDialog_Spell::event_Ignore()
{
    ignoreWord();
}

void AP_UnixDialog_Spell::event_IgnoreAll()
{
    addIgnoreAll();
    ignoreWord();
}

void AP_UnixDialog_Spell::event_AddToDict()
{
    addToDict();
   
    ignoreWord();
}

void AP_UnixDialog_Spell::event_Cancel()
{
    m_bCancelled = true;
}

void AP_UnixDialog_Spell::event_SuggestionSelected(gint row, gint column)
{
    if (!m_Suggestions->getItemCount()) return;
   
    m_iSelectedRow = row;
   
    gchar * newreplacement = NULL;
    gtk_clist_get_text(GTK_CLIST(m_clistSuggestions), row, column, &newreplacement);
    UT_ASSERT(newreplacement);

    g_signal_handler_block(G_OBJECT(m_entryChange), m_replaceHandlerID);
    gtk_entry_set_text(GTK_ENTRY(m_entryChange), newreplacement);
    g_signal_handler_unblock(G_OBJECT(m_entryChange), m_replaceHandlerID);
}

void AP_UnixDialog_Spell::event_ReplacementChanged()
{
    const gchar * modtext = gtk_entry_get_text(GTK_ENTRY(m_entryChange));
    UT_ASSERT(modtext);
   
    gchar * suggest = NULL;
    for (int i = 0; i < GTK_CLIST(m_clistSuggestions)->row_height; i++)
    {
        gtk_clist_get_text(GTK_CLIST(m_clistSuggestions), i, 0, &suggest);
        UT_ASSERT(suggest);
        if (strcmp(modtext, suggest) == 0) {
            g_signal_handler_block(G_OBJECT(m_clistSuggestions), m_listHandlerID);
            gtk_clist_select_row(GTK_CLIST(m_clistSuggestions), i, 0);
            g_signal_handler_unblock(G_OBJECT(m_clistSuggestions), m_listHandlerID);
            return;
        }
    }
   
    gtk_clist_unselect_all(GTK_CLIST(m_clistSuggestions));
    m_iSelectedRow = -1;
}

char * AP_UnixDialog_Spell::_convertToMB(const UT_UCSChar *wword)
{
    UT_UCS4String ucs4(wword);
    return UT_strdup(ucs4.utf8_str());
}

char * AP_UnixDialog_Spell::_convertToMB(const UT_UCSChar *wword, UT_sint32 iLength)
{
    UT_UCS4String ucs4(wword, iLength);
    return UT_strdup(ucs4.utf8_str());
}

UT_UCSChar * AP_UnixDialog_Spell::_convertFromMB(const char *word)
{
    UT_UCS4Char * str = 0;
    UT_UCS4String ucs4(word);
    UT_UCS4_cloneString(&str, ucs4.ucs4_str());
    return str;
}
