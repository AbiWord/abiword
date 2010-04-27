/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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
#include "ap_Dialog_InsertBookmark.h"
#include "ap_UnixDialog_InsertBookmark.h"

/*****************************************************************/

#define BUTTON_INSERT 1

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_InsertBookmark::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_InsertBookmark * p = new AP_UnixDialog_InsertBookmark(pFactory,id);
	return p;
}

AP_UnixDialog_InsertBookmark::AP_UnixDialog_InsertBookmark(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_InsertBookmark(pDlgFactory,id)
	, m_windowMain(NULL)
	, m_buttonInsert(NULL)
{
}

AP_UnixDialog_InsertBookmark::~AP_UnixDialog_InsertBookmark(void)
{
}

/*****************************************************************/
/***********************************************************************/

void AP_UnixDialog_InsertBookmark::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	// Populate the window's data items
	_setList();

	switch(abiRunModalDialog(GTK_DIALOG(mainWindow), pFrame, this,
				 BUTTON_INSERT, false))
	  {
	  case BUTTON_INSERT:
	    event_OK () ; break ;
	  case BUTTON_DELETE:
	    event_Delete () ; break ;
	  default:
	    event_Cancel () ; break ;
	  }
	
	abiDestroyWidget ( mainWindow ) ;
}

void AP_UnixDialog_InsertBookmark::event_OK(void)
{
	UT_ASSERT(m_windowMain);
	// get the bookmark name, if any (return cancel if no name given)
	GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(m_comboBookmark)));
	UT_ASSERT(entry);
	const gchar *mark = gtk_entry_get_text(entry);
	if(mark && *mark)
	{
		xxx_UT_DEBUGMSG(("InsertBookmark: OK pressed, first char 0x%x\n", (UT_uint32)*mark));
		setAnswer(AP_Dialog_InsertBookmark::a_OK);
		setBookmark(mark);
	}
	else
	{
		setAnswer(AP_Dialog_InsertBookmark::a_CANCEL);
	}
}

void AP_UnixDialog_InsertBookmark::event_Cancel(void)
{
	setAnswer(AP_Dialog_InsertBookmark::a_CANCEL);
}

void AP_UnixDialog_InsertBookmark::event_Delete(void)
{
	GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(m_comboBookmark)));
	UT_ASSERT(entry);
	const gchar *mark = gtk_entry_get_text(entry);
	if (mark && *mark)
		setBookmark(mark);
	setAnswer(AP_Dialog_InsertBookmark::a_DELETE);
}

void AP_UnixDialog_InsertBookmark::_setList(void)
{
	std::list<std::string> bookmarks;

	for(UT_sint32 i = 0; i < getExistingBookmarksCount(); i++) {
		bookmarks.push_back(getNthExistingBookmark(i));
	}
	
	GtkComboBox * combo = GTK_COMBO_BOX(m_comboBookmark);

	if (bookmarks.size())
	{
		bookmarks.sort();
		std::list<std::string>::iterator iter(bookmarks.begin());
		for( ; iter != bookmarks.end(); ++iter) {
			gtk_combo_box_append_text(combo, iter->c_str());
		}
	}
	
	GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(m_comboBookmark)));
	if (getBookmark() && strlen(getBookmark()) > 0)
	{
	    gtk_entry_set_text(entry, getBookmark());
	}
	else
	{
	    const UT_UCS4String suggestion = getSuggestedBM ();
	    if (suggestion.size()>0)
		{
			UT_UTF8String utf8 (suggestion);
			gtk_entry_set_text (entry, utf8.utf8_str());
		}
	}
}

void  AP_UnixDialog_InsertBookmark::_constructWindowContents(GtkWidget * container )
{
  GtkWidget *label1;
  const XAP_StringSet * pSS = m_pApp->getStringSet();
  UT_UTF8String s;
  pSS->getValueUTF8(AP_STRING_ID_DLG_InsertBookmark_Msg,s);
  label1 = gtk_label_new (s.utf8_str());
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (container), label1, FALSE, FALSE, 0);

  m_comboBookmark = gtk_combo_box_entry_new_text();
  gtk_widget_show (m_comboBookmark);
  gtk_box_pack_start (GTK_BOX (container), m_comboBookmark, FALSE, FALSE, 0);
}

GtkWidget*  AP_UnixDialog_InsertBookmark::_constructWindow(void)
{
  GtkWidget *vbox;

  const XAP_StringSet * pSS = m_pApp->getStringSet();
  UT_UTF8String s;
  pSS->getValueUTF8(AP_STRING_ID_DLG_InsertBookmark_Title,s);
  
  m_windowMain = abiDialogNew("insert bookmark dialog", TRUE, s.utf8_str());

  
  vbox = gtk_vbox_new (FALSE, 6);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (m_windowMain)->vbox), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

  _constructWindowContents ( vbox );

  abiAddStockButton(GTK_DIALOG(m_windowMain), GTK_STOCK_CANCEL, BUTTON_CANCEL);
  abiAddStockButton(GTK_DIALOG(m_windowMain), GTK_STOCK_DELETE, BUTTON_DELETE);
  m_buttonInsert = abiAddButton(GTK_DIALOG(m_windowMain), "", BUTTON_INSERT);
  localizeButtonUnderline (m_buttonInsert, pSS, AP_STRING_ID_DLG_InsertButton);

  gtk_widget_grab_focus (m_comboBookmark);

  return m_windowMain;
}
