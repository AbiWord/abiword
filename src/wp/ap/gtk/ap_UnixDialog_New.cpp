/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001, 2002 Dom Lachowicz
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

#include <stdlib.h>
#include <unistd.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_path.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_New.h"
#include "ap_UnixDialog_New.h"

#include "xap_Dlg_FileOpenSaveAs.h"
#include "ie_imp.h"

#include "ut_string_class.h"

/*************************************************************************/

XAP_Dialog * AP_UnixDialog_New::static_constructor(XAP_DialogFactory * pFactory,
												   XAP_Dialog_Id id)
{
	return new AP_UnixDialog_New(pFactory,id);
}

AP_UnixDialog_New::AP_UnixDialog_New(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
  : AP_Dialog_New(pDlgFactory,id), m_pFrame(0)
{
}

AP_UnixDialog_New::~AP_UnixDialog_New(void)
{
  UT_VECTOR_PURGEALL(UT_UTF8String*, mTemplates);
}

void AP_UnixDialog_New::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);

	m_pFrame = pFrame;
	
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);
	
	switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow), pFrame, this,
								 GTK_RESPONSE_OK, false ) )
	{
		case GTK_RESPONSE_OK:
			event_Ok (); break ;
		default:
			event_Cancel () ; break ;
	}
	
	abiDestroyWidget ( mainWindow ) ;
}

/*************************************************************************/
/*************************************************************************/

void AP_UnixDialog_New::event_Ok ()
{
	setAnswer (AP_Dialog_New::a_OK);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioExisting)))
	{
		setOpenType(AP_Dialog_New::open_Existing);
	}
	else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioNew)))
	{
		GtkTreeSelection * selection;
		GtkTreeIter iter;
		GtkTreeModel * model;
		
		selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_choicesList) );

		// if there is no selection, or the selection's data (GtkListItem widget)
		// is empty, return cancel.  GTK can make this happen.
		if ( !selection || 
			 !gtk_tree_selection_get_selected (selection, &model, &iter)
			)
		{
		    // fall back
		    setOpenType(AP_Dialog_New::open_New);
		} else {
			// get the ID of the selected Type
			int mRow;
			gtk_tree_model_get (model, &iter, 1, &mRow, -1);
			
			UT_UTF8String * tmpl = (UT_UTF8String*)mTemplates[mRow] ;
			if (tmpl && tmpl->utf8_str())
			{
				char * uri = UT_go_filename_to_uri (tmpl->utf8_str());
				setFileName (uri);
				g_free (uri);
				setOpenType(AP_Dialog_New::open_Template);
			}
			else
			{
				// fall back
				setOpenType(AP_Dialog_New::open_New);
			}
		}
	}
	else
	{
		setOpenType(AP_Dialog_New::open_New);
	}
}

void AP_UnixDialog_New::event_Cancel ()
{
	setAnswer (AP_Dialog_New::a_CANCEL);
}

void AP_UnixDialog_New::event_ToggleOpenExisting ()
{
	XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_OPEN;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) m_pFrame->getDialogFactory();

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	pDialog->setCurrentPathname("");
	pDialog->setSuggestFilename(false);

	UT_uint32 filterCount = IE_Imp::getImporterCount();
	const char ** szDescList = (const char **) UT_calloc(filterCount + 1,
													  sizeof(char *));
	const char ** szSuffixList = (const char **) UT_calloc(filterCount + 1,
														sizeof(char *));
	IEFileType * nTypeList = (IEFileType *) UT_calloc(filterCount + 1,
												   sizeof(IEFileType));
	UT_uint32 k = 0;

	while (IE_Imp::enumerateDlgLabels(k, &szDescList[k], 
									  &szSuffixList[k], &nTypeList[k]))
			k++;

	pDialog->setFileTypeList(szDescList, szSuffixList, 
							 (const UT_sint32 *) nTypeList);

	pDialog->setDefaultFileType(IE_Imp::fileTypeForSuffix(".abw"));

	pDialog->runModal(m_pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const std::string & resultPathname = pDialog->getPathname();
		if (!resultPathname.empty()) {
			setFileName (resultPathname.c_str());
		}

		// open file from filecooser without extra click
		gtk_dialog_response (GTK_DIALOG (m_mainWindow), GTK_RESPONSE_OK);
	}

	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);
	
	pDialogFactory->releaseDialog(pDialog);
}

/*************************************************************************/
/*************************************************************************/

void AP_UnixDialog_New::event_RadioButtonSensitivity ()
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioNew)))
	{	// ^ from template
		gtk_widget_set_sensitive (m_choicesList, TRUE);
		gtk_widget_set_sensitive (m_buttonFilename, FALSE);
	}
	else
	{ // ^ open existing document
		gtk_widget_set_sensitive (m_choicesList, FALSE);
		gtk_widget_set_sensitive (m_buttonFilename, TRUE);
	}
}

/*************************************************************************/
/*************************************************************************/

static void s_choose_clicked (GtkWidget * /*w*/, AP_UnixDialog_New * dlg)
{
	dlg->event_ToggleOpenExisting();
}

/*************************************************************************/
/*************************************************************************/

static void s_radiobutton_clicked (GtkWidget * /*w*/, AP_UnixDialog_New * dlg)
{
	dlg->event_RadioButtonSensitivity();
}

/*************************************************************************/
/*************************************************************************/

// TODO we could make this some utility function and use for all platforms
// Can return NULL
// The list must be free'd by the caller, but the filenames are owned by the system.
static GSList * awt_only (const char *path) {

	GDir 		*dir;
	GSList 		*list;
	const char 	*name;
	const char  *suffix;
	int			 len;
	GError 		*err;

	if (!g_file_test (path, G_FILE_TEST_IS_DIR))
		return NULL;

	err = NULL;
	dir = g_dir_open (path, 0, &err);
	if (err) {
		g_warning ("%s", err->message);
		g_error_free (err), err = NULL;
		return NULL;
	}

	list = NULL;
	while (NULL != (name = g_dir_read_name (dir))) {
		len = strlen (name);
		if (len < 5)
			continue;
		suffix = name+(len-4);
		if(0 == strcmp (suffix, ".awt") || 0 == strcmp (suffix, ".dot")) {
			list = g_slist_prepend (list, (void *) name);
		}
	}
	g_dir_close (dir), dir = NULL;

	return list;
}

/*************************************************************************/
/*************************************************************************/

static void s_template_clicked(GtkTreeView *treeview,
							   AP_UnixDialog_New * dlg)
{
	UT_DEBUG_ONLY_ARG(treeview);
	UT_ASSERT(treeview && dlg);
	dlg->event_ListClicked();
}

void AP_UnixDialog_New::s_template_dblclicked(GtkTreeView * /*treeview*/,
											  GtkTreePath * /*arg1*/,
											  GtkTreeViewColumn * /*arg2*/,
											  AP_UnixDialog_New * me)
{
	me->event_ListClicked();
	gtk_dialog_response (GTK_DIALOG(me->m_mainWindow), GTK_RESPONSE_OK);
}

GtkWidget * AP_UnixDialog_New::_constructWindow ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_New.ui");
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	m_mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_New"));
	gtk_window_set_title(GTK_WINDOW(m_mainWindow), 
						 pSS->getValue(AP_STRING_ID_DLG_NEW_Title));

	m_radioNew = GTK_WIDGET(gtk_builder_get_object(builder, "rdTemplate"));
	m_radioExisting = GTK_WIDGET(gtk_builder_get_object(builder, "rdOpen"));
	m_buttonFilename = GTK_WIDGET(gtk_builder_get_object(builder, "btFile"));
	m_choicesList = GTK_WIDGET(gtk_builder_get_object(builder, "tvTemplates"));

	localizeButton(m_radioNew, pSS, AP_STRING_ID_DLG_NEW_Create);
	localizeButton(m_radioExisting, pSS, AP_STRING_ID_DLG_NEW_Open);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Format",
							 renderer,
							 "text", 
							 0,
							 (gchar*)NULL);
	gtk_tree_view_append_column( GTK_TREE_VIEW(m_choicesList), column);

	UT_UTF8String templateList[2];
	UT_UTF8String templateDir;

	// the locally installed templates (per-user basis)
	templateDir = XAP_App::getApp()->getUserPrivateDirectory();
	templateDir += "/templates/";
	templateList[0] = templateDir;
	
	// the globally installed templates
	templateDir = XAP_App::getApp()->getAbiSuiteLibDir();
	templateDir += "/templates/";
	templateList[1] = templateDir;

	GtkListStore *model;
	GtkTreeIter iter;
	
	model = gtk_list_store_new (2, 
							    G_TYPE_STRING,
								G_TYPE_INT
	                            );

	for ( unsigned int i = 0; i < G_N_ELEMENTS(templateList); i++ )
	  {
		templateDir = templateList[i];
		GSList *list = awt_only(templateDir.utf8_str());

		while (list) {

			UT_UTF8String * myTemplate = new UT_UTF8String(templateDir + (const char *) list->data);
			mTemplates.addItem ( myTemplate ) ;		    
			
			// Add a new row to the model
			gtk_list_store_append (model, &iter);
			gtk_list_store_set (model, &iter,
								0, UT_basename(myTemplate->utf8_str()),
								1, mTemplates.size()-1,
								-1);
			list = g_slist_remove(list, list->data);
		}
	  }

	gtk_tree_view_set_model( GTK_TREE_VIEW(m_choicesList), reinterpret_cast<GtkTreeModel *>(model));

	g_object_unref (model);	


	if (getOpenType() == open_Existing)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_radioExisting), true);
 		gtk_widget_grab_focus (m_buttonFilename);
	}
	else 
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_radioNew), true);
		// select first item in box
 		gtk_widget_grab_focus (m_choicesList);
	}
	// set the initial widget sensitivity state
	event_RadioButtonSensitivity ();


	g_signal_connect_after(G_OBJECT(m_choicesList),
						   "cursor-changed",
						   G_CALLBACK(s_template_clicked),
						   static_cast<gpointer>(this));

	g_signal_connect_after(G_OBJECT(m_choicesList),
						   "row-activated",
						   G_CALLBACK(s_template_dblclicked),
						   static_cast<gpointer>(this));

	// connect signals
	g_signal_connect (G_OBJECT(m_buttonFilename), 
					  "clicked",
					  G_CALLBACK(s_choose_clicked), 
					  (gpointer)this);

	g_signal_connect (G_OBJECT(m_radioNew),
					"clicked",
					G_CALLBACK(s_radiobutton_clicked),
					(gpointer)this);

	g_signal_connect (G_OBJECT(m_radioExisting),
					"clicked",
					G_CALLBACK(s_radiobutton_clicked),
					(gpointer)this);

	g_object_unref(G_OBJECT(builder));

	return m_mainWindow;
}

void AP_UnixDialog_New::event_ListClicked()
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(m_radioNew), TRUE);
}
