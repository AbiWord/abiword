/* AbiSource Application Framework
 * Copyright (C) 2001-2003 AbiSource, Inc.
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
#include "ut_assert.h"
#include "ut_string.h"
#include "xap_UnixDialogHelper.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"

#include "xap_Dialog_Id.h"
#include "xap_UnixDlg_PluginManager.h"
#include "xap_UnixDlg_FileOpenSaveAs.h"

#include "xap_Module.h"
#include "xap_ModuleManager.h"

#include "ie_types.h"
#include "ut_string_class.h"

/*****************************************************************/

static void _errorMessage (XAP_Frame * pFrame, XAP_String_Id id)
{
	// just a little simple error message box
	UT_return_if_fail(pFrame);
	pFrame->showMessageBox (id,
							XAP_Dialog_MessageBox::b_O,
							XAP_Dialog_MessageBox::a_OK);
}

static void selectFirstEntry(GtkWidget * list)
{
	gtk_widget_grab_focus (list);

	GtkTreePath* path = gtk_tree_path_new_first();

	gtk_tree_view_set_cursor(GTK_TREE_VIEW(list),
							 path, 
							 gtk_tree_view_get_column (GTK_TREE_VIEW(list), 0), 
							 FALSE);
	
	gtk_tree_path_free (path);
}

/*****************************************************************/

XAP_Dialog * XAP_UnixDialog_PluginManager::static_constructor(XAP_DialogFactory * pFactory,
							      XAP_Dialog_Id id)
{
	return new XAP_UnixDialog_PluginManager(pFactory,id);
}

XAP_UnixDialog_PluginManager::XAP_UnixDialog_PluginManager(XAP_DialogFactory * pDlgFactory,
							   XAP_Dialog_Id id)
  : XAP_Dialog_PluginManager(pDlgFactory,id)
{
}

XAP_UnixDialog_PluginManager::~XAP_UnixDialog_PluginManager(void)
{
}

/*****************************************************************/

void XAP_UnixDialog_PluginManager::event_DeactivateAll ()
{
	deactivateAllPlugins ();
	_refresh ();
}

void XAP_UnixDialog_PluginManager::event_Deactivate ()
{  
	XAP_Module * pModule = 0;

	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;
	gint rowNumber = -1;
	
	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_list) );	
	if (selection && gtk_tree_selection_get_selected (selection, &model, &iter)) {
		gtk_tree_model_get (model, &iter, 1, &rowNumber, -1);
	}
	
	if (rowNumber != -1)
		pModule = static_cast<XAP_Module *>(XAP_ModuleManager::instance().enumModules()->getNthItem(rowNumber));
	else 
    {
		// error message box - didn't select a plugin
		_errorMessage (m_pFrame, 
					   XAP_STRING_ID_DLG_PLUGIN_MANAGER_NONE_SELECTED);
		return;
    }
	
	if (pModule)
    {
		if (deactivatePlugin (pModule))
		{
			// worked
			_refresh ();
		}
		else
		{
			// error message box
			_errorMessage (m_pFrame, 
						   XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_UNLOAD);
		}
    }
	else
    {
		// error message box
		_errorMessage (m_pFrame, 
					   XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_UNLOAD);
    }
}

void XAP_UnixDialog_PluginManager::event_Load ()
{
	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(m_pFrame->getDialogFactory());
	
	XAP_Dialog_FileOpenSaveAs * pDialog
		= static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_FILE_OPEN));
	UT_ASSERT(pDialog);
	
	// set the intial plugin directory to the user-local plugin directory
	// could also set to: XAP_App::getApp()->getAbiSuiteLibDir()/plugins
	UT_String pluginDir (XAP_App::getApp()->getUserPrivateDirectory());
	pluginDir += "/AbiWord-2.0/plugins/";
	pDialog->setCurrentPathname (pluginDir.c_str());
	pDialog->setSuggestFilename(false);
	
	UT_uint32 filterCount = 1;
	const char ** szDescList = static_cast<const char **>(UT_calloc(filterCount + 1,
																	sizeof(char *)));
	const char ** szSuffixList = static_cast<const char **>(UT_calloc(filterCount + 1,
																	  sizeof(char *)));
	IEFileType * nTypeList = static_cast<IEFileType *>(UT_calloc(filterCount + 1,
																 sizeof(IEFileType)));
	
	// we probably shouldn't hardcode this
	// HP-UX uses .sl, for instance
	szDescList[0] = "AbiWord Plugin (.so)";
	szSuffixList[0] = "*.so";
	nTypeList[0] = static_cast<IEFileType>(1);
	
	pDialog->setFileTypeList(szDescList, szSuffixList, 
							 static_cast<const UT_sint32 *>(nTypeList));
  
	pDialog->setDefaultFileType(static_cast<IEFileType>(1));
	
	pDialog->runModal(m_pFrame);
	
	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);
	
	if (bOK)
    {
		const char * szResultPathname = pDialog->getPathname();
		if (szResultPathname && *szResultPathname)
		{
			if (activatePlugin (szResultPathname))
			{
				// worked!
				_refresh ();
			}
			else
			{
				// error message
				_errorMessage (m_pFrame, 
							   XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_LOAD);
			}
		}
    }
	
	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);
	
	pDialogFactory->releaseDialog(pDialog);
}

/*****************************************************************/

void XAP_UnixDialog_PluginManager::setPluginList()
{
	const UT_Vector * pVec = XAP_ModuleManager::instance().enumModules ();
	
	GtkListStore *model;
	GtkTreeIter iter;
	
	model = gtk_list_store_new (2, 
							    G_TYPE_STRING,
								G_TYPE_INT
	                            );
	
 	// build a list of all items
    for (UT_uint32 i = 0; i < pVec->size(); i++)
	{
		XAP_Module * pModule = static_cast<XAP_Module *>(pVec->getNthItem (i));
		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter,
					  		0, pModule->getModuleInfo()->name,
							1, i,
					  		-1);
	}
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_list), reinterpret_cast<GtkTreeModel *>(model));
		
	if(pVec->size())
		selectFirstEntry(m_list);

	g_object_unref (model);
}

void XAP_UnixDialog_PluginManager::_refresh ()
{
	XAP_Module * pModule = 0;
	
	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;
	gint rowNumber = -1;
	
	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_list) );
	if (selection && gtk_tree_selection_get_selected (selection, &model, &iter)) {
		gtk_tree_model_get (model, &iter, 1, &rowNumber, -1);		
	} else {
	        if(XAP_ModuleManager::instance().enumModules()->size()) 
				selectFirstEntry(m_list);
	}
	
	if (rowNumber != -1 && XAP_ModuleManager::instance().enumModules()->size() > 0)
		pModule = static_cast<XAP_Module *>(XAP_ModuleManager::instance().enumModules()->getNthItem(rowNumber));
	else
		pModule = 0;
	
	// just a blank space, to represent an empty entry
	const char * name = " ";
	const char * author = " ";
	const char * version = " ";
	const char * desc = " ";
	
	if (pModule)
	{
		const XAP_ModuleInfo * mi = pModule->getModuleInfo ();
		if (mi)
		{
			name = mi->name;
			author = mi->author;
			desc = mi->desc;
			version = mi->version;
		}
	}
	
	gtk_entry_set_text (GTK_ENTRY (m_name), name);
	gtk_entry_set_text (GTK_ENTRY (m_author), author);
	gtk_entry_set_text (GTK_ENTRY (m_version), version);
	
	GtkTextBuffer * buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW(m_desc) ) ;
	gtk_text_buffer_set_text ( buffer, desc, -1 ) ;
}

/*****************************************************************/

void XAP_UnixDialog_PluginManager::s_deactivate_clicked (GtkWidget * w, 
														 XAP_UnixDialog_PluginManager * dlg)
{
	UT_return_if_fail (dlg);
	dlg->event_Deactivate ();
}

void XAP_UnixDialog_PluginManager::s_deactivate_all_clicked (GtkWidget * w, 
															 XAP_UnixDialog_PluginManager * dlg)
{
	UT_return_if_fail (dlg);
	dlg->event_DeactivateAll ();
}

void XAP_UnixDialog_PluginManager::s_load_clicked (GtkWidget * w,
												   XAP_UnixDialog_PluginManager * dlg)
{
	UT_return_if_fail (dlg);
	dlg->event_Load ();
}

void XAP_UnixDialog_PluginManager::s_list_clicked(GtkTreeView *treeview,
												  XAP_UnixDialog_PluginManager * dlg)
{
	UT_return_if_fail(dlg);
	dlg->_refresh ();
}

/*****************************************************************/

void XAP_UnixDialog_PluginManager::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;	
	
	// build the dialog
	GtkWidget * cf = _constructWindow();
	UT_return_if_fail(cf);
	
	gtk_window_set_default_size(GTK_WINDOW(cf), 500, 300);
	
	// load the data
	setPluginList();
	_refresh();
	
	abiRunModalDialog (GTK_DIALOG(cf), pFrame, this, GTK_RESPONSE_CLOSE, true);
}

GtkWidget * XAP_UnixDialog_PluginManager::_constructWindow ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/xap_UnixDlg_PluginManager.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	if (!xml)
		return NULL;
	
	m_windowMain = glade_xml_get_widget(xml, "xap_UnixDlg_PluginManager");
	m_list = glade_xml_get_widget(xml, "tvPlugins");
	m_name = glade_xml_get_widget(xml, "enName");
	m_author = glade_xml_get_widget(xml, "enAuthor");
	m_version = glade_xml_get_widget(xml, "enVersion");
	m_desc = glade_xml_get_widget(xml, "tvDescription");

	gtk_window_set_title(GTK_WINDOW(m_windowMain), pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_TITLE));

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbActivePlugins"), pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_ACTIVE);
	localizeLabelMarkup(glade_xml_get_widget(xml, "lbPluginDetails"), pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_DETAILS);
	localizeLabel(glade_xml_get_widget(xml, "lbPluginName"), pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_NAME);
	localizeLabel(glade_xml_get_widget(xml, "lbPluginDescription"), pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_DESC);
	localizeLabel(glade_xml_get_widget(xml, "lbPluginAuthor"), pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_AUTHOR);
	localizeLabel(glade_xml_get_widget(xml, "lbPluginVersion"), pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_VERSION);

	GtkWidget * btDeactivate = glade_xml_get_widget(xml, "btDeactivate");
	GtkWidget * btDeactivateAll = glade_xml_get_widget(xml, "btDeactivateAll");
	GtkWidget * btInstall = glade_xml_get_widget(xml, "btInstall");

	localizeButton(btDeactivate, pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE);
	localizeButton(btDeactivateAll, pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE_ALL);
	localizeButton(btInstall, pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_INSTALL);

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Format",
													   renderer,
													   "text", 
													   0,
													   NULL);
	gtk_tree_view_append_column( GTK_TREE_VIEW(m_list), column);

	g_signal_connect (G_OBJECT(btDeactivate), "clicked",
					  G_CALLBACK(s_deactivate_clicked), 
					  static_cast<gpointer>(this));
	
	g_signal_connect (G_OBJECT(btDeactivateAll), "clicked",
					  G_CALLBACK(s_deactivate_all_clicked), 
					  static_cast<gpointer>(this));
	
	g_signal_connect (G_OBJECT(btInstall), "clicked",
					  G_CALLBACK(s_load_clicked), 
					  static_cast<gpointer>(this));

	g_signal_connect_after(G_OBJECT(m_list),
						   "cursor-changed",
						   G_CALLBACK(s_list_clicked),
						   static_cast<gpointer>(this));

	return m_windowMain;
}
