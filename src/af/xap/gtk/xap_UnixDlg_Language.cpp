/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "xap_UnixDialogHelper.h"
#include "xap_UnixDlg_Language.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"


XAP_Dialog * XAP_UnixDialog_Language::static_constructor(XAP_DialogFactory * pFactory,
							 XAP_Dialog_Id id)
{
	return new XAP_UnixDialog_Language(pFactory,id);
}

XAP_UnixDialog_Language::XAP_UnixDialog_Language(XAP_DialogFactory * pDlgFactory,
						 XAP_Dialog_Id id)
  : XAP_Dialog_Language(pDlgFactory,id), m_pLanguageList ( NULL )
{
}

void XAP_UnixDialog_Language::s_lang_dblclicked(GtkTreeView * /*treeview*/,
												GtkTreePath * /*arg1*/,
												GtkTreeViewColumn * /*arg2*/,
												XAP_UnixDialog_Language * me)
{
	gtk_dialog_response (GTK_DIALOG(me->m_windowMain), GTK_RESPONSE_CLOSE);
}

XAP_UnixDialog_Language::~XAP_UnixDialog_Language(void)
{
}

void XAP_UnixDialog_Language::event_setLang()
{
	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;

	gint row = 0;

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_pLanguageList) );

	// if there is no selection, or the selection's data (GtkListItem widget)
	// is empty, return cancel.  GTK can make this happen.
	if ( !selection || 
		 !gtk_tree_selection_get_selected (selection, &model, &iter)
	   )
	{
		m_answer = XAP_Dialog_Language::a_CANCEL;
		return;
	}

	// get the ID of the selected Type
	gtk_tree_model_get (model, &iter, 1, &row, -1);
  
	if (row >= 0) {
	  if (!m_pLanguage || g_ascii_strcasecmp(m_pLanguage, m_ppLanguages[row]))
	    {
	      _setLanguage(m_ppLanguages[row]);
	      m_bChangedLanguage = true;
		  m_answer = XAP_Dialog_Language::a_OK;

		  bool b = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(m_cbDefaultLanguage));
		  setMakeDocumentDefault(b);
	    }
	  else {
		  m_answer = XAP_Dialog_Language::a_CANCEL;
	  }
	} else {
		UT_ASSERT_NOT_REACHED();
		m_answer = XAP_Dialog_Language::a_CANCEL;
	}
}

GtkWidget * XAP_UnixDialog_Language::constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	GtkBuilder * builder = newDialogBuilder("xap_UnixDlg_Language.ui");

	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	m_windowMain = GTK_WIDGET(gtk_builder_get_object(builder, "xap_UnixDlg_Language"));
	m_pLanguageList = GTK_WIDGET(gtk_builder_get_object(builder, "tvAvailableLanguages"));
	m_lbDefaultLanguage = GTK_WIDGET(gtk_builder_get_object(builder, "lbDefaultLanguage"));
	m_cbDefaultLanguage = GTK_WIDGET(gtk_builder_get_object(builder, "cbDefaultLanguage"));

	std::string s;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_ULANG_LangTitle,s);
	gtk_window_set_title (GTK_WINDOW(m_windowMain), s.c_str());
	localizeLabelMarkup (GTK_WIDGET(gtk_builder_get_object(builder, "lbAvailableLanguages")), pSS, XAP_STRING_ID_DLG_ULANG_AvailableLanguages);
	getDocDefaultLangDescription(s);
	gtk_label_set_text (GTK_LABEL(m_lbDefaultLanguage), s.c_str());
	getDocDefaultLangCheckboxLabel(s);
	gtk_button_set_label (GTK_BUTTON(m_cbDefaultLanguage), s.c_str());
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(m_cbDefaultLanguage), isMakeDocumentDefault());

	// add a column to our TreeViews

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Format",
													   renderer,
													   "text", 
													   0,
													   NULL);
	gtk_tree_view_append_column( GTK_TREE_VIEW(m_pLanguageList), column);
	  
	g_object_unref(G_OBJECT(builder));

	return m_windowMain;
}

void XAP_UnixDialog_Language::_populateWindowData()
{
	GtkListStore *model;
	GtkTreeIter iter;
	
	model = gtk_list_store_new (2, 
							    G_TYPE_STRING,
								G_TYPE_INT);
	
	for (UT_uint32 i = 0; i < m_iLangCount; i++)
	{		
		// Add a new row to the model
		gtk_list_store_append (model, &iter);
		
		gtk_list_store_set (model, &iter,
							0, m_ppLanguages[i],
							1, i,
							-1);
	}
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_pLanguageList), reinterpret_cast<GtkTreeModel *>(model));
	
	g_object_unref (model);	
	
	// now select first item in box
 	gtk_widget_grab_focus (m_pLanguageList);
	
	if (m_pLanguage) {
		gint foundAt = -1;
		for (UT_uint32 i = 0; i < m_iLangCount; i++)
		{
			if (!g_ascii_strcasecmp(m_pLanguage, m_ppLanguages[i])) {
				foundAt = i;
				break;
			}
		}  
		
		if (foundAt != -1) {
			GtkTreePath* path = gtk_tree_path_new ();
			gtk_tree_path_append_index (path, foundAt);
			
			gtk_tree_view_set_cursor(GTK_TREE_VIEW(m_pLanguageList),
									 path, 
									 gtk_tree_view_get_column (GTK_TREE_VIEW(m_pLanguageList), 0), 
									 FALSE);
			gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(m_pLanguageList),
						     path, NULL, TRUE, 0.5, 0.0);
			gtk_widget_grab_focus (m_pLanguageList);
			
			gtk_tree_path_free (path);
		}
	}
}

void XAP_UnixDialog_Language::runModal(XAP_Frame * pFrame)
{
  // build the dialog
  GtkWidget * cf = constructWindow();    
  UT_return_if_fail(cf);	
	
  _populateWindowData();
  
  // connect a dbl-clicked signal to the column
  g_signal_connect_after(G_OBJECT(m_pLanguageList),
						   "row-activated",
						   G_CALLBACK(s_lang_dblclicked),
						   static_cast<gpointer>(this));

  abiRunModalDialog ( GTK_DIALOG(cf), pFrame, this, GTK_RESPONSE_CLOSE, false );
  event_setLang();
  
  abiDestroyWidget(cf);
}
