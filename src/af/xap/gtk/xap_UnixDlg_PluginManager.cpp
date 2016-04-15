/* AbiSource Application Framework
 * Copyright (C) 2003 Marc Maurer
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
	// deactivate all plugins in the Plugin Manager
	deactivateAllPlugins ();

	// remove all plugins from the TreeView
	GtkTreeModel * model;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_list));
	gtk_list_store_clear(GTK_LIST_STORE (model));
}

void XAP_UnixDialog_PluginManager::event_Deactivate ()
{  
	XAP_Module * pModule = 0;

	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;
	
	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_list) );	
	if (selection && gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		// remove the plugin from our TreeView
		GtkTreePath *path;
		path = gtk_tree_model_get_path (model, &iter);
		guint rowNumber = gtk_tree_path_get_indices (path)[0];
		gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

		if ((UT_sint32)rowNumber < XAP_ModuleManager::instance().enumModules()->size() - 1)
		{
			// select the next item in the TreeView
			gtk_tree_view_set_cursor(GTK_TREE_VIEW(m_list),
									 path, 
									 gtk_tree_view_get_column (GTK_TREE_VIEW(m_list), 0), 
									 FALSE);		
		}
		gtk_tree_path_free (path);		

		pModule = static_cast<XAP_Module *>(XAP_ModuleManager::instance().enumModules()->getNthItem(rowNumber));
		if (pModule)
		{
			if (deactivatePlugin (pModule))
			{
				// worked
				_updatePluginList ();
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
	else 
	{
		// error message box - didn't select a plugin
		_errorMessage (m_pFrame, 
					   XAP_STRING_ID_DLG_PLUGIN_MANAGER_NONE_SELECTED);
		return;
	}
}

void XAP_UnixDialog_PluginManager::event_Load ()
{
	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(m_pFrame->getDialogFactory());
	
	XAP_Dialog_FileOpenSaveAs * pDialog
		= static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_FILE_OPEN));
	UT_return_if_fail(pDialog);
	
	// set the intial plugin directory to the user-local plugin directory
	// could also set to: XAP_App::getApp()->getAbiSuiteLibDir()/plugins
	UT_String pluginDir (XAP_App::getApp()->getUserPrivateDirectory());
	pluginDir += "/";
	pluginDir += PACKAGE_NAME;
	pluginDir += "-";
	pluginDir += ABIWORD_SERIES;
	pluginDir += "/plugins/";
	pDialog->setCurrentPathname (pluginDir.c_str());
	pDialog->setSuggestFilename(false);
	
	UT_uint32 filterCount = 1;
	const char ** szDescList = static_cast<const char **>(UT_calloc(filterCount + 1,
																	sizeof(char *)));
	const char ** szSuffixList = static_cast<const char **>(UT_calloc(filterCount + 1,
																	  sizeof(char *)));
	IEFileType * nTypeList = static_cast<IEFileType *>(UT_calloc(filterCount + 1,
																 sizeof(IEFileType)));
	
	szDescList[0] = "AbiWord Plugin (.G_MODULE_SUFFIX)";
	szSuffixList[0] = "*.G_MODULE_SUFFIX";
	nTypeList[0] = static_cast<IEFileType>(1);
	
	pDialog->setFileTypeList(szDescList, szSuffixList, 
							 static_cast<const UT_sint32 *>(nTypeList));
  
	pDialog->setDefaultFileType(static_cast<IEFileType>(1));
	
	pDialog->runModal(m_pFrame);
	
	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);
	
	if (bOK) {
		const std::string & resultPathname = pDialog->getPathname();
		if (!resultPathname.empty()) {
			if (activatePlugin (resultPathname.c_str())) {
				// worked!
				_updatePluginList ();
			} else {
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

/*!
* Update the list of loaded plugins.
*/
void XAP_UnixDialog_PluginManager::_updatePluginList ()
{
	const UT_GenericVector<XAP_Module*> * pVec = XAP_ModuleManager::instance().enumModules ();
	
	GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW (m_list));
	GtkListStore *model = NULL;
	if (tm != NULL) {
		model = GTK_LIST_STORE (tm);
		// detach model for faster updates
		g_object_ref (G_OBJECT (model));
		gtk_tree_view_set_model (GTK_TREE_VIEW (m_list), NULL);
		gtk_list_store_clear (model);
	}
	else {	
		model = gtk_list_store_new (1, G_TYPE_STRING);
	}
	
 	// build a list of all items
	GtkTreeIter iter;
    for (UT_sint32 i = 0; i < pVec->size(); i++) {

		XAP_Module * pModule = pVec->getNthItem (i);
		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter,
					  		0, pModule->getModuleInfo()->name,
					  		-1);
	}
	
	gtk_tree_view_set_model (GTK_TREE_VIEW (m_list), reinterpret_cast<GtkTreeModel *>(model));
		
	if(pVec->size ())
		_selectFirstEntry ();

	g_object_unref (model);
}

void XAP_UnixDialog_PluginManager::_refresh ()
{
	XAP_Module * pModule = 0;
	
	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;
	
	if (XAP_ModuleManager::instance().enumModules()->size())
	{
		selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_list) );
		if (selection && gtk_tree_selection_get_selected (selection, &model, &iter))
		{
			// get the selected module from the TreeView
			GtkTreePath *path;		
			path = gtk_tree_model_get_path (model, &iter);
			gint rowNumber = gtk_tree_path_get_indices (path)[0];
			
			pModule = static_cast<XAP_Module *>(XAP_ModuleManager::instance().enumModules()->getNthItem(rowNumber));
			gtk_tree_path_free(path);
		} 
	}
	
	// just a blank space, to represent an empty entry
	const char * name = NULL;
	const char * author = NULL;
	const char * version = NULL;
	const char * desc = NULL;
	
	const char * na = m_pApp->getStringSet()->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_NOT_AVAILABLE);

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

	if (!name) name = na;
	if (!author) author = na;
	if (!version) version = na;
	if (!desc) desc = na;
	
	gtk_label_set_text (GTK_LABEL (m_name), name);
	gtk_label_set_text (GTK_LABEL (m_author), author);
	gtk_label_set_text (GTK_LABEL (m_version), version);	
	gtk_label_set_text (GTK_LABEL (m_desc), desc) ;
}

/*****************************************************************/

void XAP_UnixDialog_PluginManager::s_load_clicked (GtkWidget *,
												   XAP_UnixDialog_PluginManager * dlg)
{
	UT_return_if_fail (dlg);
	dlg->event_Load ();
}

void XAP_UnixDialog_PluginManager::s_list_clicked(GtkTreeSelection *,
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
	_updatePluginList ();
	
	abiRunModalDialog (GTK_DIALOG(cf), pFrame, this, GTK_RESPONSE_CLOSE, true);
}

GtkWidget * XAP_UnixDialog_PluginManager::_constructWindow ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("xap_UnixDlg_PluginManager.ui");
	m_windowMain = GTK_WIDGET(gtk_builder_get_object(builder, "xap_UnixDlg_PluginManager"));
	m_list = GTK_WIDGET(gtk_builder_get_object(builder, "tvPlugins"));
	m_name = GTK_WIDGET(gtk_builder_get_object(builder, "lbPluginName"));
	m_author = GTK_WIDGET(gtk_builder_get_object(builder, "lbPluginAuthor"));
	m_version = GTK_WIDGET(gtk_builder_get_object(builder, "lbPluginVersion"));
	m_desc = GTK_WIDGET(gtk_builder_get_object(builder, "lbPluginDescription"));

	gtk_window_set_title(GTK_WINDOW(m_windowMain), pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_TITLE));

	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbActivePlugins")), pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_ACTIVE);
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbPluginDetails")), pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_DETAILS);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbNameLabel")), pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_NAME);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbDescriptionLabel")), pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_DESC);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbAuthorLabel")), pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_AUTHOR);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbVersionLabel")), pSS, XAP_STRING_ID_DLG_PLUGIN_MANAGER_VERSION);

	GtkWidget * btInstall = GTK_WIDGET(gtk_builder_get_object(builder, "btInstall"));

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

	g_signal_connect (G_OBJECT(btInstall), "clicked",
					  G_CALLBACK(s_load_clicked), 
					  static_cast<gpointer>(this));

	g_signal_connect_after(G_OBJECT(gtk_tree_view_get_selection (GTK_TREE_VIEW (m_list))),
						   "changed",
						   G_CALLBACK(s_list_clicked),
						   static_cast<gpointer>(this));

	g_object_unref(G_OBJECT(builder));

	return m_windowMain;
}

/*!
* Select the plugin list's first entry.
*/
void 
XAP_UnixDialog_PluginManager::_selectFirstEntry ()
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (m_list));
	GtkTreePath* path = gtk_tree_path_new_first();
	gtk_tree_selection_select_path (selection, path);
	gtk_tree_path_free (path);
}
