/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz
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
#include "ap_UnixDialog_MetaData.h"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_MetaData::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_MetaData * p = new AP_UnixDialog_MetaData(pFactory,id);
	return p;
}

AP_UnixDialog_MetaData::AP_UnixDialog_MetaData(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_MetaData(pDlgFactory,id)
{
}

AP_UnixDialog_MetaData::~AP_UnixDialog_MetaData(void)
{
}

void AP_UnixDialog_MetaData::runModal(XAP_Frame * pFrame)
{
  UT_return_if_fail(pFrame);

  // Build the window's widgets and arrange them
  GtkWidget * mainWindow = _constructWindow();
  UT_return_if_fail(mainWindow);
  
  switch(abiRunModalDialog(GTK_DIALOG(mainWindow), pFrame, this,
						   BUTTON_CANCEL, false))
  {
	  case BUTTON_OK:
		  eventOK(); break;
	  default:
		  eventCancel(); break ;
  }
  
  abiDestroyWidget(mainWindow);
}

void AP_UnixDialog_MetaData::eventCancel ()
{
  setAnswer ( AP_Dialog_MetaData::a_CANCEL ) ;
}

#define GRAB_ENTRY_TEXT(name) txt = gtk_entry_get_text(GTK_ENTRY(m_entry##name)) ; \
if( txt && strlen(txt) ) \
set##name ( txt )

void AP_UnixDialog_MetaData::eventOK ()
{
  setAnswer ( AP_Dialog_MetaData::a_OK ) ;

  // TODO: gather data
  const char * txt = NULL ;

  GRAB_ENTRY_TEXT(Title);
  GRAB_ENTRY_TEXT(Subject);
  GRAB_ENTRY_TEXT(Author);
  GRAB_ENTRY_TEXT(Publisher);  
  GRAB_ENTRY_TEXT(CoAuthor);
  GRAB_ENTRY_TEXT(Category);
  GRAB_ENTRY_TEXT(Keywords);
  GRAB_ENTRY_TEXT(Languages);
  GRAB_ENTRY_TEXT(Source);
  GRAB_ENTRY_TEXT(Relation);
  GRAB_ENTRY_TEXT(Coverage);
  GRAB_ENTRY_TEXT(Rights);

  GtkTextIter start, end;

  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_textDescription));
  gtk_text_buffer_get_iter_at_offset ( buffer, &start, 0 );
  gtk_text_buffer_get_iter_at_offset ( buffer, &end, -1 );

  char * editable_txt = gtk_text_buffer_get_text ( buffer, &start, &end, FALSE );

  if (editable_txt && strlen(editable_txt)) {
    setDescription ( editable_txt ) ;
    g_free(editable_txt);
  }
}

#undef GRAB_ENTRY_TEXT

GtkWidget * AP_UnixDialog_MetaData::_constructWindow ()
{
  GtkWidget *main_dlg;
  GtkWidget *dialog_vbox1;

  const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

  // create dialog

  main_dlg = abiDialogNew ( "metadata dialog", TRUE, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Title).c_str());
  gtk_container_set_border_width (GTK_CONTAINER (main_dlg), 3);

  dialog_vbox1 = GTK_DIALOG (main_dlg)->vbox;
  gtk_widget_show (dialog_vbox1);

  abiAddStockButton ( GTK_DIALOG(main_dlg), GTK_STOCK_OK, BUTTON_OK ) ;
  abiAddStockButton ( GTK_DIALOG(main_dlg), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;

  // construct window contents

  _constructWindowContents ( dialog_vbox1 ) ;

  return main_dlg;
}

void AP_UnixDialog_MetaData::_constructWindowContents ( GtkWidget * dialog_vbox1 )
{
  GtkWidget *notebook1;
  GtkWidget *notebook_lbl;
  GtkWidget *table2;
  GtkWidget *title_lbl;
  GtkWidget *subject_lbl;
  GtkWidget *author_lbl;
  GtkWidget *publisher_lbl;
  GtkWidget *coauthor_lbl;
  GtkWidget *title_entry;
  GtkWidget *subject_entry;
  GtkWidget *author_entry;
  GtkWidget *publisher_entry;
  GtkWidget *coauthor_entry;
  GtkWidget *table3;
  GtkWidget *category_lbl;
  GtkWidget *keywords_lbl;
  GtkWidget *language_lbl;
  GtkWidget *desc_lbl;
  GtkWidget *languages_entry;
  GtkWidget *keywords_entry;
  GtkWidget *scrolledwindow1;
  GtkWidget *description_txt;
  GtkWidget *category_entry;
  GtkWidget *table4;
  GtkWidget *source_lbl;
  GtkWidget *relation_lbl;
  GtkWidget *coverage_lbl;
  GtkWidget *rights_lbl;
  GtkWidget *source_entry;
  GtkWidget *relation_entry;
  GtkWidget *coverage_entry;
  GtkWidget *rights_entry;

  const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);

  gtk_container_add(GTK_CONTAINER(dialog_vbox1), notebook1);

  table2 = gtk_table_new (5, 2, FALSE);
  gtk_widget_show (table2);
  gtk_container_add(GTK_CONTAINER(notebook1), table2);
  gtk_container_set_border_width (GTK_CONTAINER (table2), 3);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 3);

  title_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Title_LBL).c_str());
  gtk_widget_show (title_lbl);
  gtk_table_attach (GTK_TABLE (table2), title_lbl, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (title_lbl), 0, 0.5);

  subject_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Subject_LBL).c_str());
  gtk_widget_show (subject_lbl);
  gtk_table_attach (GTK_TABLE (table2), subject_lbl, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (subject_lbl), 0, 0.5);

  author_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Author_LBL).c_str());
  gtk_widget_show (author_lbl);
  gtk_table_attach (GTK_TABLE (table2), author_lbl, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (author_lbl), 0, 0.5);

  publisher_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Publisher_LBL).c_str());
  gtk_widget_show (publisher_lbl);
  gtk_table_attach (GTK_TABLE (table2), publisher_lbl, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (publisher_lbl), 0, 0.5);

  coauthor_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_CoAuthor_LBL).c_str());
  gtk_widget_show (coauthor_lbl);
  gtk_table_attach (GTK_TABLE (table2), coauthor_lbl, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (coauthor_lbl), 0, 0.5);

  title_entry = gtk_entry_new ();
  gtk_widget_show (title_entry);
  gtk_table_attach (GTK_TABLE (table2), title_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  subject_entry = gtk_entry_new ();
  gtk_widget_show (subject_entry);
  gtk_table_attach (GTK_TABLE (table2), subject_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  author_entry = gtk_entry_new ();
  gtk_widget_show (author_entry);
  gtk_table_attach (GTK_TABLE (table2), author_entry, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  publisher_entry = gtk_entry_new ();
  gtk_widget_show (publisher_entry);
  gtk_table_attach (GTK_TABLE (table2), publisher_entry, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  coauthor_entry = gtk_entry_new ();
  gtk_widget_show (coauthor_entry);
  gtk_table_attach (GTK_TABLE (table2), coauthor_entry, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  table3 = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (table3);
  gtk_container_add(GTK_CONTAINER(notebook1), table3);
  gtk_container_set_border_width (GTK_CONTAINER (table3), 3);
  gtk_table_set_row_spacings (GTK_TABLE (table3), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table3), 3);

  category_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Category_LBL).c_str());
  gtk_widget_show (category_lbl);
  gtk_table_attach (GTK_TABLE (table3), category_lbl, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (category_lbl), 0, 0.5);

  keywords_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Keywords_LBL).c_str());
  gtk_widget_show (keywords_lbl);
  gtk_table_attach (GTK_TABLE (table3), keywords_lbl, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (keywords_lbl), 0, 0.5);

  language_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Languages_LBL).c_str());
  gtk_widget_show (language_lbl);
  gtk_table_attach (GTK_TABLE (table3), language_lbl, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (language_lbl), 0, 0.5);

  desc_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Description_LBL).c_str());
  gtk_widget_show (desc_lbl);
  gtk_table_attach (GTK_TABLE (table3), desc_lbl, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (desc_lbl), 0, 0.5);

  languages_entry = gtk_entry_new ();
  gtk_widget_show (languages_entry);
  gtk_table_attach (GTK_TABLE (table3), languages_entry, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  keywords_entry = gtk_entry_new ();
  gtk_widget_show (keywords_entry);
  gtk_table_attach (GTK_TABLE (table3), keywords_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_table_attach (GTK_TABLE (table3), scrolledwindow1, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  description_txt = gtk_text_view_new ();
  gtk_widget_show (description_txt);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), description_txt);

  category_entry = gtk_entry_new ();
  gtk_widget_show (category_entry);
  gtk_table_attach (GTK_TABLE (table3), category_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  table4 = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (table4);
  gtk_container_add(GTK_CONTAINER(notebook1), table4);
  gtk_container_set_border_width (GTK_CONTAINER (table4), 3);
  gtk_table_set_row_spacings (GTK_TABLE (table4), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table4), 3);

  source_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Source_LBL).c_str());
  gtk_widget_show (source_lbl);
  gtk_table_attach (GTK_TABLE (table4), source_lbl, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (source_lbl), 0, 0.5);

  relation_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Relation_LBL).c_str());
  gtk_widget_show (relation_lbl);
  gtk_table_attach (GTK_TABLE (table4), relation_lbl, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (relation_lbl), 0, 0.5);

  coverage_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Coverage_LBL).c_str());
  gtk_widget_show (coverage_lbl);
  gtk_table_attach (GTK_TABLE (table4), coverage_lbl, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (coverage_lbl), 0, 0.5);

  rights_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Rights_LBL).c_str());
  gtk_widget_show (rights_lbl);
  gtk_table_attach (GTK_TABLE (table4), rights_lbl, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (rights_lbl), 0, 0.5);

  source_entry = gtk_entry_new ();
  gtk_widget_show (source_entry);
  gtk_table_attach (GTK_TABLE (table4), source_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  relation_entry = gtk_entry_new ();
  gtk_widget_show (relation_entry);
  gtk_table_attach (GTK_TABLE (table4), relation_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  coverage_entry = gtk_entry_new ();
  gtk_widget_show (coverage_entry);
  gtk_table_attach (GTK_TABLE (table4), coverage_entry, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  rights_entry = gtk_entry_new ();
  gtk_widget_show (rights_entry);
  gtk_table_attach (GTK_TABLE (table4), rights_entry, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  // assign tab names

  notebook_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_TAB_General).c_str());
  gtk_widget_show(notebook_lbl);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), notebook_lbl); 

  notebook_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_TAB_Summary).c_str());
  gtk_widget_show(notebook_lbl);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), notebook_lbl);

  notebook_lbl = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_TAB_Permission).c_str());
  gtk_widget_show(notebook_lbl);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), notebook_lbl);

  // assign member variables

  m_entryTitle = title_entry;
  m_entrySubject = subject_entry;
  m_entryAuthor = author_entry;
  m_entryPublisher = publisher_entry;
  m_entryCoAuthor = coauthor_entry;
  m_entryLanguages = languages_entry;
  m_entryKeywords = keywords_entry;
  m_entryCategory = category_entry;
  m_entrySource = source_entry;
  m_entryRelation = relation_entry;
  m_entryCoverage = coverage_entry;
  m_entryRights = rights_entry;

  m_textDescription = description_txt;

  UT_String prop ( "" ) ;

  // now set the text
  #define SET_ENTRY_TXT(name) \
  prop = get##name () ; \
  if ( prop.size () > 0 ) { \
    gtk_entry_set_text (GTK_ENTRY(m_entry##name), prop.c_str() ) ; \
  }

  SET_ENTRY_TXT(Title)
  SET_ENTRY_TXT(Subject)
  SET_ENTRY_TXT(Author)
  SET_ENTRY_TXT(Publisher)
  SET_ENTRY_TXT(CoAuthor)
  SET_ENTRY_TXT(Category)
  SET_ENTRY_TXT(Keywords)
  SET_ENTRY_TXT(Languages)
  SET_ENTRY_TXT(Source)
  SET_ENTRY_TXT(Relation)
  SET_ENTRY_TXT(Coverage)
  SET_ENTRY_TXT(Rights)

  #undef SET_ENTRY_TXT

  gint unused_pos = 0 ;
  prop = getDescription () ;
  if ( prop.size () )
    {
      GtkTextBuffer * buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW(description_txt) ) ;
      gtk_text_buffer_set_text ( buffer, prop.c_str(), -1 ) ;
    }
}
