/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"
#include "xap_Gtk2Compat.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_InsertHyperlink.h"
#include "ap_UnixDialog_InsertHyperlink.h"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_InsertHyperlink::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_InsertHyperlink * p = new AP_UnixDialog_InsertHyperlink(pFactory,id);
	return p;
}

AP_UnixDialog_InsertHyperlink::AP_UnixDialog_InsertHyperlink(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_InsertHyperlink(pDlgFactory,id),
	m_entry(0),
	m_windowMain(0),
	// m_comboEntry(0),
	m_clist(0),
	m_swindow(0),
	m_titleEntry(0),
	m_iRow(-1)
	
{

}

AP_UnixDialog_InsertHyperlink::~AP_UnixDialog_InsertHyperlink(void)
{
}

/*****************************************************************/

static void s_blist_clicked(GtkTreeSelection * select,
							AP_UnixDialog_InsertHyperlink *me)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	if(gtk_tree_selection_get_selected(select, &model, &iter)) {
		GtkTreePath * path = gtk_tree_model_get_path(model, &iter);
		gint* rows = gtk_tree_path_get_indices(path);
		if(rows) {
			me->setRow(*rows);
			gtk_entry_set_text(GTK_ENTRY(me->m_entry), 
					   me->m_pBookmarks[*rows].c_str());
		}
	}
}


/***********************************************************************/
void AP_UnixDialog_InsertHyperlink::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// select the first row of the list (this must come after the
 	// call to _connectSignals)
// 	gtk_clist_unselect_row(GTK_CLIST(m_clist),0,0);

	switch(abiRunModalDialog(GTK_DIALOG(mainWindow), pFrame, this, BUTTON_CANCEL, false))
	  {
	  case BUTTON_OK:
	    event_OK (); break;
	  default:
	    event_Cancel(); break ;
	  }

	abiDestroyWidget(mainWindow);
}

void AP_UnixDialog_InsertHyperlink::event_OK(void)
{
	UT_ASSERT(m_windowMain);
	// get the bookmark name, if any (return cancel if no name given)
	const gchar * res = gtk_entry_get_text(GTK_ENTRY(m_entry));
	const gchar * title = gtk_entry_get_text(GTK_ENTRY(m_titleEntry));
	if(res && *res)
	{
		setAnswer(AP_Dialog_InsertHyperlink::a_OK);
		setHyperlink(res);
		setHyperlinkTitle(title);
	}
	else
	{
		setAnswer(AP_Dialog_InsertHyperlink::a_CANCEL);
	}
}

void AP_UnixDialog_InsertHyperlink::event_Cancel(void)
{
	setAnswer(AP_Dialog_InsertHyperlink::a_CANCEL);
}

void AP_UnixDialog_InsertHyperlink::_constructWindowContents ( GtkWidget * vbox2 )
{
  const XAP_StringSet * pSS = m_pApp->getStringSet();

  GtkWidget *label1;
  GtkWidget *label2;

  std::string s;
  pSS->getValueUTF8(AP_STRING_ID_DLG_InsertHyperlink_Msg,s);
  label1 = gtk_label_new (s.c_str());
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (vbox2), label1, FALSE, FALSE, 3);

  m_entry = gtk_entry_new();
  gtk_box_pack_start (GTK_BOX (vbox2), m_entry, FALSE, FALSE, 0);
  gtk_widget_show(m_entry);
  
  const gchar * hyperlink = getHyperlink();

  if (hyperlink && *hyperlink)
  {
    if (*hyperlink == '#')
    {
      gtk_entry_set_text ( GTK_ENTRY(m_entry), hyperlink + 1) ;
    }
    else
    {
      gtk_entry_set_text ( GTK_ENTRY(m_entry), hyperlink ) ;
    }
  }

  // the bookmark list
  m_swindow  = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (m_swindow),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_show(m_swindow);
  gtk_box_pack_start (GTK_BOX (vbox2), m_swindow, TRUE, TRUE, 0);
   
  GtkListStore * store = gtk_list_store_new(1, G_TYPE_STRING);

  GtkTreeView * treeview;
  m_clist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
  treeview = GTK_TREE_VIEW(m_clist);
  gtk_tree_view_set_headers_visible(treeview, FALSE);
  gtk_tree_selection_set_mode(gtk_tree_view_get_selection(treeview), 
							  GTK_SELECTION_BROWSE);

  GtkCellRenderer *renderer = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
  GtkTreeViewColumn *col;
  col = gtk_tree_view_column_new_with_attributes("",
												 renderer, "text", 0, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);
  //gtk_box_pack_start (GTK_BOX (vbox2), m_blist, FALSE, FALSE, 0);

  m_pBookmarks.clear();
	
  for (int i = 0; i < static_cast<int>(getExistingBookmarksCount()); i++) {
    m_pBookmarks.push_back(getNthExistingBookmark(i));
  }

  std::sort(m_pBookmarks.begin(), m_pBookmarks.end());

  for (int i = 0; i < static_cast<int>(getExistingBookmarksCount()); i++) {
		  GtkTreeIter iter;
		  gtk_list_store_append(store, &iter);
		  gtk_list_store_set(store, &iter, 0, m_pBookmarks[i].c_str(), -1);
  }

  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(m_swindow),m_clist);

  pSS->getValueUTF8(AP_STRING_ID_DLG_InsertHyperlink_TitleLabel, s);
  label2 = gtk_label_new(s.c_str());
  gtk_widget_show(label2);
  gtk_box_pack_start(GTK_BOX(vbox2), label2, TRUE, TRUE, 3);

  m_titleEntry = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(vbox2), m_titleEntry, FALSE, FALSE, 0);
  gtk_widget_show(m_titleEntry);

  const gchar * hyperlinkTitle = getHyperlinkTitle();

  if (hyperlinkTitle && *hyperlinkTitle)
  {
      gtk_entry_set_text(GTK_ENTRY(m_titleEntry), hyperlinkTitle);
  }
}

GtkWidget*  AP_UnixDialog_InsertHyperlink::_constructWindow(void)
{
  GtkWidget *vbox2;
  GtkWidget *frame1;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  std::string s;
  pSS->getValueUTF8(AP_STRING_ID_DLG_InsertHyperlink_Title,s);
  m_windowMain = abiDialogNew("insert table dialog", TRUE, s.c_str());
#if !GTK_CHECK_VERSION(3,0,0)
  gtk_dialog_set_has_separator(GTK_DIALOG(m_windowMain), FALSE);
#endif

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_box_pack_start(GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG(m_windowMain))), frame1,true,true,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 4);
  gtk_frame_set_shadow_type(GTK_FRAME(frame1), GTK_SHADOW_NONE);

  vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (frame1), vbox2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);

  _constructWindowContents ( vbox2 );

  pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel, s);
  abiAddButton(GTK_DIALOG(m_windowMain), s, BUTTON_CANCEL);
  pSS->getValueUTF8(XAP_STRING_ID_DLG_OK, s);
  abiAddButton(GTK_DIALOG(m_windowMain), s, BUTTON_OK);

  gtk_widget_grab_focus (m_entry);

  // connect all the signals
  _connectSignals ();
  gtk_widget_show_all ( m_windowMain ) ;

  return m_windowMain;
}

void AP_UnixDialog_InsertHyperlink::_connectSignals (void)
{
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_clist));
	g_signal_connect (G_OBJECT(select), "changed",
					  G_CALLBACK (s_blist_clicked), this);
}
