/* AbiSource Application Framework
 * Copyright (C) 1998-2002 AbiSource, Inc.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "xap_UnixDialogHelper.h"
#include "xap_UnixDlg_Language.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "gr_UnixGraphics.h"

XAP_Dialog * XAP_UnixDialog_Language::static_constructor(XAP_DialogFactory * pFactory,
							 XAP_Dialog_Id id)
{
  XAP_UnixDialog_Language * p = new XAP_UnixDialog_Language(pFactory,id);
  return p;
}

XAP_UnixDialog_Language::XAP_UnixDialog_Language(XAP_DialogFactory * pDlgFactory,
						 XAP_Dialog_Id id)
  : XAP_Dialog_Language(pDlgFactory,id), m_pLanguageList ( NULL )
{
}

void XAP_UnixDialog_Language::s_lang_dblclicked(GtkTreeView *treeview,
												GtkTreePath *arg1,
												GtkTreeViewColumn *arg2,
												XAP_UnixDialog_Language * me)
{
	gtk_dialog_response (GTK_DIALOG(me->m_windowMain), GTK_RESPONSE_OK);
}

XAP_UnixDialog_Language::~XAP_UnixDialog_Language(void)
{
}

void XAP_UnixDialog_Language::event_Ok()
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
	  if (!m_pLanguage || UT_stricmp(m_pLanguage, m_ppLanguages[row]))
	    {
	      _setLanguage(m_ppLanguages[row]);
	      m_bChangedLanguage = true;
		  m_answer = XAP_Dialog_Language::a_OK;
	    }
	  else {
		  m_answer = XAP_Dialog_Language::a_CANCEL;
	  }
	} else {
		UT_ASSERT_NOT_REACHED();
		m_answer = XAP_Dialog_Language::a_CANCEL;
	}
}

void XAP_UnixDialog_Language::event_Cancel()
{
	m_answer = XAP_Dialog_Language::a_CANCEL;
}

GtkWidget * XAP_UnixDialog_Language::constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/xap_UnixDlg_Language.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );

	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	m_windowMain = glade_xml_get_widget(xml, "windowMain");
	m_pLanguageList = glade_xml_get_widget(xml, "treeview1");

	gtk_window_set_title (GTK_WINDOW(m_windowMain), pSS->getValueUTF8(XAP_STRING_ID_DLG_ULANG_LangTitle).c_str());
	localizeLabelMarkup(glade_xml_get_widget(xml, "label1"), pSS, XAP_STRING_ID_DLG_ULANG_LangLabel);

	// add a column to our TreeViews

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Format",
													   renderer,
													   "text", 
													   0,
													   NULL);
	gtk_tree_view_append_column( GTK_TREE_VIEW(m_pLanguageList), column);
	
	// connect a dbl-clicked signal to the column
	
	g_signal_connect_after(G_OBJECT(m_pLanguageList),
						   "row-activated",
						   G_CALLBACK(s_lang_dblclicked),
						   static_cast<gpointer>(this));
  
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
			if (!UT_stricmp(m_pLanguage, m_ppLanguages[i])) {
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
			
			gtk_tree_path_free (path);
		}
	}
}

void XAP_UnixDialog_Language::runModal(XAP_Frame * pFrame)
{
  // build the dialog
  GtkWidget * cf = constructWindow();    
  _populateWindowData();  

  switch ( abiRunModalDialog ( GTK_DIALOG(cf), pFrame, this, GTK_RESPONSE_CANCEL, false ) )
    {
    case GTK_RESPONSE_OK:
		event_Ok(); break;
    default:
		event_Cancel(); break;
    }
  
  abiDestroyWidget(cf);
}

