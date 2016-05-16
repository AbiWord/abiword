/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
 * Copyright (C) 2011 Ben Martin
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
#include <time.h>

#include <list>
#include <string>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_InsertXMLID.h"
#include "ap_UnixDialog_InsertXMLID.h"
#include "GTKCommon.h"


/*****************************************************************/

#define BUTTON_INSERT 1
#include <gdk/gdkkeysyms.h>
static gboolean __onKeyPressed(G_GNUC_UNUSED GtkWidget* widget,
                               GdkEventKey* event,
                               G_GNUC_UNUSED gpointer user_data )
{
    guint32 uc = gdk_keyval_to_unicode(event->keyval);
    xxx_UT_DEBUGMSG(("__onKeyPressed() uc: %u\n", uc));
    
    if( uc >= 'A' && uc <= 'Z' )
    {
        return false;
    }
    if( uc >= 'a' && uc <= 'z' )
    {
        return false;
    }
    if( uc >= '0' && uc <= '9' )
    {
        return false;
    }
    if( event->keyval == GDK_KEY_Delete
        || event->keyval == GDK_KEY_BackSpace
        || event->keyval == GDK_KEY_Left
        || event->keyval == GDK_KEY_Right )
    {
        return false;
    }

    // filter the rest
    return true;
    
}

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_InsertXMLID::static_constructor( XAP_DialogFactory * pFactory,
                                                            XAP_Dialog_Id id )
{
	AP_UnixDialog_InsertXMLID * p = new AP_UnixDialog_InsertXMLID(pFactory,id);
	return p;
}

AP_UnixDialog_InsertXMLID::AP_UnixDialog_InsertXMLID( XAP_DialogFactory * pDlgFactory,
                                                      XAP_Dialog_Id id )
	: AP_Dialog_InsertXMLID(pDlgFactory,id)
	, m_window(NULL)
	, m_btInsert(NULL)
{
}

AP_UnixDialog_InsertXMLID::~AP_UnixDialog_InsertXMLID(void)
{
}

/*****************************************************************/
/***********************************************************************/

void AP_UnixDialog_InsertXMLID::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	// Populate the window's data items
	_setList();

	switch(abiRunModalDialog( GTK_DIALOG(mainWindow), pFrame, this,
                              BUTTON_INSERT, false))
    {
        case BUTTON_INSERT: event_OK ();     break ;
        case BUTTON_DELETE: event_Delete (); break ;
        default:            event_Cancel (); break ;
    }
	
	abiDestroyWidget ( mainWindow ) ;
}

void
AP_UnixDialog_InsertXMLID::event_OK(void)
{
	UT_ASSERT(m_window);
	// get the bookmark name, if any (return cancel if no name given)
    //std::string mark = tostr(GTK_COMBO_BOX(m_combo));
    std::string mark = tostr(GTK_ENTRY(m_combo));
	if( !mark.empty() )
	{
		xxx_UT_DEBUGMSG(("InsertXMLID: OK pressed, first char 0x%x\n", (UT_uint32)mark[0]));
		setAnswer(AP_Dialog_InsertXMLID::a_OK);
		setString(mark);
	}
	else
	{
		setAnswer(AP_Dialog_InsertXMLID::a_CANCEL);
	}
}

void
AP_UnixDialog_InsertXMLID::event_Cancel(void)
{
	setAnswer(AP_Dialog_InsertXMLID::a_CANCEL);
}

void
AP_UnixDialog_InsertXMLID::event_Delete(void)
{
    //setString(tostr(GTK_COMBO_BOX(m_combo)));
    setString(tostr(GTK_ENTRY(m_combo)));
	setAnswer(AP_Dialog_InsertXMLID::a_DELETE);
}

void
AP_UnixDialog_InsertXMLID::_setList(void)
{
	// std::list<std::string> bookmarks;

	// for(UT_sint32 i = 0; i < getExistingBookmarksCount(); i++)
    // {
	// 	bookmarks.push_back(getNthExistingBookmark(i));
	// }
	
	// GtkComboBoxText * combo = GTK_COMBO_BOX_TEXT(m_combo);
  //   bookmarks.sort();
  //   append( combo, bookmarks );
	
	// GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(m_combo)));
	// if (getBookmark() && strlen(getBookmark()) > 0)
	// {
	//     gtk_entry_set_text(entry, getBookmark());
	// }
	// else
	// {
	//     const UT_UCS4String suggestion = getSuggestedBM ();
	//     if (suggestion.size()>0)
	// 	{
	// 		UT_UTF8String utf8 (suggestion);
	// 		gtk_entry_set_text (entry, utf8.utf8_str());
	// 	}
	// }
}

void
AP_UnixDialog_InsertXMLID::_constructWindowContents(GtkWidget * container )
{
    XAP_String_Id msgid = AP_STRING_ID_DLG_InsertXMLID_Msg;
    
    GtkWidget *label1;
    const XAP_StringSet * pSS = m_pApp->getStringSet();
    std::string s;
    pSS->getValueUTF8(msgid,s);
    label1 = gtk_label_new (s.c_str());
    gtk_widget_show (label1);
    gtk_box_pack_start (GTK_BOX (container), label1, FALSE, FALSE, 0);

    // m_combo = gtk_combo_box_text_new_with_entry();
    // doesn't yet work as a combo box!
    m_combo = gtk_entry_new();
    gtk_widget_show (m_combo);
    gtk_box_pack_start (GTK_BOX (container), m_combo, FALSE, FALSE, 0);

    // GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(m_combo)));

	// g_signal_connect (GTK_ENTRY (entry), "key-press-event", 
	g_signal_connect (GTK_ENTRY (m_combo), "key-press-event", 
					  G_CALLBACK (__onKeyPressed), static_cast <gpointer>(this));


}


GtkWidget*
AP_UnixDialog_InsertXMLID::_constructWindow(void)
{
    GtkWidget *vbox;

    const XAP_StringSet * pSS = m_pApp->getStringSet();
    std::string s;
    pSS->getValueUTF8(AP_STRING_ID_DLG_InsertXMLID_Title,s);
  
    m_window = abiDialogNew("insert RDF link dialog", TRUE, s.c_str());
  
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_show (vbox);
    gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG (m_window))), vbox);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

    _constructWindowContents ( vbox );

    pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel, s);
    abiAddButton(GTK_DIALOG(m_window), s, BUTTON_CANCEL);
    pSS->getValueUTF8(XAP_STRING_ID_DLG_Delete, s);
    abiAddButton(GTK_DIALOG(m_window), s, BUTTON_DELETE);
    pSS->getValueUTF8(AP_STRING_ID_DLG_InsertButton, s);
    m_btInsert = abiAddButton(GTK_DIALOG(m_window), s, BUTTON_INSERT);

    gtk_widget_grab_focus (m_combo);

    return m_window;
}
