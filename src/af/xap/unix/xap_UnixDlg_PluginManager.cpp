/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
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
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_dialogHelper.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_UnixDlg_PluginManager.h"
#include "xap_UnixDlg_FileOpenSaveAs.h"

#include "xap_Module.h"
#include "xap_ModuleManager.h"

#include "ie_types.h"

/*****************************************************************/
/*****************************************************************/

static void _errorMessage (XAP_Frame * pFrame, const char * msg)
{
	// just a little simple error message box

	pFrame->showMessageBox (msg,
							XAP_Dialog_MessageBox::b_O,
							XAP_Dialog_MessageBox::a_OK);
}

/*****************************************************************/
/*****************************************************************/

XAP_Dialog * XAP_UnixDialog_PluginManager::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	XAP_UnixDialog_PluginManager * p = new XAP_UnixDialog_PluginManager(pFactory,id);
	return p;
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
/*****************************************************************/

void XAP_UnixDialog_PluginManager::event_DeactivateAll ()
{
	deactivateAllPlugins ();
	_refreshAll ();
}

void XAP_UnixDialog_PluginManager::event_Deactivate ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	XAP_Module * pModule = 0;

	GList * selectedRow = 0;
	selectedRow = GTK_CLIST(m_clist)->selection;
	if (selectedRow)
	{
			gint which = GPOINTER_TO_INT(selectedRow->data);
			pModule = (XAP_Module *) XAP_ModuleManager::instance().enumModules()->getNthItem(which);
	} 
	else 
	{
		// error message box - didn't select a plugin
		_errorMessage (m_pFrame, 
					   pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_NONE_SELECTED));
		return;
	}

	if (pModule)
	{
		if (deactivatePlugin (pModule))
		{
			// worked
			_refreshAll ();
		}
		else
		{
			// error message box
			_errorMessage (m_pFrame, 
						   pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_UNLOAD));
		}
	}
	else
	{
		// error message box
		_errorMessage (m_pFrame, 
					   pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_UNLOAD));
	}
}

void XAP_UnixDialog_PluginManager::event_Load ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) m_pFrame->getDialogFactory();
	
	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_FILE_OPEN));
	UT_ASSERT(pDialog);
	
	pDialog->setCurrentPathname(0);
	pDialog->setSuggestFilename(false);
	
	UT_uint32 filterCount = 1;
	const char ** szDescList = (const char **) calloc(filterCount + 1,
													  sizeof(char *));
	const char ** szSuffixList = (const char **) calloc(filterCount + 1,
														sizeof(char *));
	IEFileType * nTypeList = (IEFileType *) calloc(filterCount + 1,
												   sizeof(IEFileType));
	
	// we probably shouldn't hardcode this
	// HP-UX uses .sl, for instance
	szDescList[0] = "AbiWord Plugin (.so)";
	szSuffixList[0] = "*.so";
	nTypeList[0] = (IEFileType)1;
	
	pDialog->setFileTypeList(szDescList, szSuffixList, 
							 (const UT_sint32 *) nTypeList);
	
	pDialog->setDefaultFileType((IEFileType)1);
	
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
				_refreshAll ();
			}
			else
			{
				// error message
				_errorMessage (m_pFrame, 
							   pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_LOAD));
			}
		}
	}
	
	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);
	
	pDialogFactory->releaseDialog(pDialog);
}

void XAP_UnixDialog_PluginManager::event_Select1 ()
{
	_refreshTab2 ();
}

/*****************************************************************/
/*****************************************************************/

void XAP_UnixDialog_PluginManager::_refreshAll ()
{
	_refreshTab1();

	gtk_clist_select_row (GTK_CLIST(m_clist), 0, 0);

	_refreshTab2();
}

void XAP_UnixDialog_PluginManager::_refreshTab1 ()
{
	gchar * text[2] = {NULL, NULL};
	XAP_Module * pModule = 0;

	// first, refresh the CList
	gtk_clist_freeze (GTK_CLIST (m_clist));
	gtk_clist_clear (GTK_CLIST (m_clist));
	
	const UT_Vector * pVec = XAP_ModuleManager::instance().enumModules ();

	for (UT_uint32 i = 0; i < pVec->size(); i++)
	{
		pModule = (XAP_Module *)pVec->getNthItem (i);
		text [0] = pModule->getModuleInfo()->name;
		gtk_clist_append(GTK_CLIST(m_clist), text);
	}

	gtk_clist_thaw (GTK_CLIST (m_clist));
}

void XAP_UnixDialog_PluginManager::_refreshTab2 ()
{
	gint txt_len = gtk_text_get_length (GTK_TEXT(m_desc));
	gint txt_pos = 0;
	if (txt_len > 0) {
		gtk_editable_delete_text (GTK_EDITABLE (m_desc), 0, 
								  txt_len);
	}

	XAP_Module * pModule = 0;
	GList * selectedRow = 0;
	selectedRow = GTK_CLIST(m_clist)->selection;
	if (selectedRow)
	{
			gint rowNumber = GPOINTER_TO_INT(selectedRow->data);
			pModule = (XAP_Module *) XAP_ModuleManager::instance().enumModules()->getNthItem(rowNumber);
	}

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

	gtk_editable_insert_text (GTK_EDITABLE (m_desc),
							  desc,
							  strlen (desc),
							  &txt_pos);
}

/*****************************************************************/
/*****************************************************************/

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 gpointer /* dialog */)
{
	gtk_main_quit ();
}

static void s_close_clicked (GtkWidget * /* widget */, gpointer /* dlg */)
{
	gtk_main_quit ();
}

static void s_deactivate_clicked (GtkWidget * w, 
								  XAP_UnixDialog_PluginManager * dlg)
{
	UT_ASSERT (dlg);

	// TODO: get selected row

	dlg->event_Deactivate ();
}

static void s_deactivate_all_clicked (GtkWidget * w, 
									  XAP_UnixDialog_PluginManager * dlg)
{
	UT_ASSERT (dlg);

	dlg->event_DeactivateAll ();
}

static void s_load_clicked (GtkWidget * w,
							XAP_UnixDialog_PluginManager * dlg)
{
	UT_ASSERT (dlg);
	dlg->event_Load ();
}

static void s_clist_selected (GtkWidget * w,
							  gint /* row */,
							  gint /* column */,
							  GdkEventButton * /* event */,
							  XAP_UnixDialog_PluginManager * dlg)
{
	dlg->event_Select1 ();
}

/*****************************************************************/
/*****************************************************************/

void XAP_UnixDialog_PluginManager::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(m_pApp);
	UT_ASSERT(pFrame);

	m_pFrame = pFrame;	

	// build the dialog
	GtkWidget * cf = _constructWindow();
	UT_ASSERT(cf);
	connectFocus(GTK_WIDGET(cf),pFrame);

	// get top level window and its GtkWidget *
	XAP_UnixFrame * frame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(frame);
	GtkWidget * parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);

	// center it
	centerDialog(parent, cf);
	
	// Run the dialog
	gtk_widget_show (cf);
	gtk_grab_add (cf);

	// select the 1st row && force a refresh
	gtk_clist_select_row(GTK_CLIST(m_clist), 0, 0);
	_refreshAll();

	gtk_main();

	if (cf && GTK_IS_WIDGET(cf))
	  gtk_widget_destroy (cf);
}

/*****************************************************************/
/*****************************************************************/

void 
XAP_UnixDialog_PluginManager::_constructWindowContents (GtkWidget * container)
{
	GtkWidget *notebook1;
	GtkWidget *vbox1;
	GtkWidget *hbox1;
	GtkWidget *scrolledwindow1;
	GtkWidget *clistPlugins;
	GtkWidget *lblActivePlugins;
	GtkWidget *vbox3;
	GtkWidget *btnDeactivate;
	GtkWidget *btnDeactivateAll;
	GtkWidget *btnInstall;
	GtkWidget *lblPluginList;
	GtkWidget *vbox2;
	GtkWidget *table1;
	GtkWidget *lblName;
	GtkWidget *lblDesc;
	GtkWidget *lblAuthor;
	GtkWidget *lblVersion;
	GtkWidget *entryName;
	GtkWidget *entryAuthor;
	GtkWidget *entryVersion;
	GtkWidget *scrolledwindow2;
	GtkWidget *textDescription;
	GtkWidget *lblPluginDetails;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	notebook1 = gtk_notebook_new ();
	gtk_widget_show (notebook1);
	gtk_box_pack_start (GTK_BOX (container), notebook1, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (notebook1), 2);
	
	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (notebook1), vbox1);
	
	hbox1 = gtk_hbox_new (FALSE, 4);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox1), 2);
	
	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow1);
	gtk_box_pack_start (GTK_BOX (hbox1), scrolledwindow1, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), 
									GTK_POLICY_AUTOMATIC, 
									GTK_POLICY_AUTOMATIC);
	
	clistPlugins = gtk_clist_new (1);
	gtk_widget_show (clistPlugins);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), clistPlugins);
	gtk_clist_set_column_width (GTK_CLIST (clistPlugins), 0, 80);
	gtk_clist_column_titles_show (GTK_CLIST (clistPlugins));
	
	lblActivePlugins = gtk_label_new (pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_ACTIVE));
	gtk_widget_show (lblActivePlugins);
	gtk_clist_set_column_widget (GTK_CLIST (clistPlugins), 0, lblActivePlugins);
	
	vbox3 = gtk_vbox_new (FALSE, 7);
	gtk_widget_show (vbox3);
	gtk_box_pack_start (GTK_BOX (hbox1), vbox3, TRUE, TRUE, 0);
	
	btnDeactivate = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE));
	gtk_widget_show (btnDeactivate);
	gtk_box_pack_start (GTK_BOX (vbox3), btnDeactivate, FALSE, FALSE, 0);
	
	btnDeactivateAll = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE_ALL));
	gtk_widget_show (btnDeactivateAll);
	gtk_box_pack_start (GTK_BOX (vbox3), btnDeactivateAll, FALSE, FALSE, 0);
	
	btnInstall = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_INSTALL));
	gtk_widget_show (btnInstall);
	gtk_box_pack_start (GTK_BOX (vbox3), btnInstall, FALSE, FALSE, 0);
	
	lblPluginList = gtk_label_new (pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_LIST));
	gtk_widget_show (lblPluginList);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), 
								gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), lblPluginList);
	
	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_container_add (GTK_CONTAINER (notebook1), vbox2);
	
	table1 = gtk_table_new (4, 2, FALSE);
	gtk_widget_show (table1);
	gtk_box_pack_start (GTK_BOX (vbox2), table1, TRUE, TRUE, 0);
	gtk_table_set_row_spacings (GTK_TABLE (table1), 3);
	gtk_table_set_col_spacings (GTK_TABLE (table1), 3);
	
	lblName = gtk_label_new (pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_NAME));
	gtk_widget_show (lblName);
	gtk_table_attach (GTK_TABLE (table1), lblName, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (lblName), 0, 0.5);
	
	lblDesc = gtk_label_new (pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_DESC));
	gtk_widget_show (lblDesc);
	gtk_table_attach (GTK_TABLE (table1), lblDesc, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (lblDesc), 0, 0.5);
	
	lblAuthor = gtk_label_new (pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_AUTHOR));
	gtk_widget_show (lblAuthor);
	gtk_table_attach (GTK_TABLE (table1), lblAuthor, 0, 1, 2, 3,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (lblAuthor), 0, 0.5);
	
	lblVersion = gtk_label_new (pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_VERSION));
	gtk_widget_show (lblVersion);
	gtk_table_attach (GTK_TABLE (table1), lblVersion, 0, 1, 3, 4,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (lblVersion), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lblVersion), 0, 0.5);
	
	entryName = gtk_entry_new ();
	gtk_entry_set_editable (GTK_ENTRY (entryName), FALSE);
	gtk_widget_show (entryName);
	gtk_table_attach (GTK_TABLE (table1), entryName, 1, 2, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	
	entryAuthor = gtk_entry_new ();
	gtk_entry_set_editable (GTK_ENTRY (entryAuthor), FALSE);
	gtk_widget_show (entryAuthor);
	gtk_table_attach (GTK_TABLE (table1), entryAuthor, 1, 2, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	
	entryVersion = gtk_entry_new ();
	gtk_entry_set_editable (GTK_ENTRY (entryVersion), FALSE);
	gtk_widget_show (entryVersion);
	gtk_table_attach (GTK_TABLE (table1), entryVersion, 1, 2, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	
	scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow2);
	gtk_table_attach (GTK_TABLE (table1), scrolledwindow2, 1, 2, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), 
									GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	
	textDescription = gtk_text_new (NULL, NULL);
	gtk_widget_show (textDescription);
	gtk_text_set_word_wrap (GTK_TEXT (textDescription), TRUE);
	gtk_text_set_editable (GTK_TEXT (textDescription), FALSE);
	gtk_container_add (GTK_CONTAINER (scrolledwindow2), textDescription);
	
	lblPluginDetails = gtk_label_new (pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_DETAILS));
	gtk_widget_show (lblPluginDetails);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), 
								gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), lblPluginDetails);

	// connect some relvant signals

	gtk_signal_connect (GTK_OBJECT(btnDeactivate), "clicked",
						GTK_SIGNAL_FUNC(s_deactivate_clicked), 
						(gpointer)this);

	gtk_signal_connect (GTK_OBJECT(btnDeactivateAll), "clicked",
						GTK_SIGNAL_FUNC(s_deactivate_all_clicked), 
						(gpointer)this);

	gtk_signal_connect (GTK_OBJECT(btnInstall), "clicked",
						GTK_SIGNAL_FUNC(s_load_clicked), 
						(gpointer)this);

	gtk_signal_connect (GTK_OBJECT(clistPlugins), "select_row",
						GTK_SIGNAL_FUNC(s_clist_selected),
						(gpointer)this);

	// assign pointers to important widgets
	m_clist = clistPlugins;
	m_name = entryName;
	m_author = entryAuthor;
	m_version = entryVersion;
	m_desc = textDescription;
}

GtkWidget * XAP_UnixDialog_PluginManager::_constructWindow ()
{
	GtkWidget *windowPlugins;
	GtkWidget *dialog_vbox1;
	GtkWidget *dialog_action_area1;
	GtkWidget *hbuttonbox1;
	GtkWidget *btnClose;
	
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowPlugins = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (windowPlugins), 
						  pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_TITLE));
	gtk_window_set_policy (GTK_WINDOW (windowPlugins), TRUE, TRUE, FALSE);
	
	dialog_vbox1 = GTK_DIALOG (windowPlugins)->vbox;
	gtk_widget_show (dialog_vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox1), 3);
	
	dialog_action_area1 = GTK_DIALOG (windowPlugins)->action_area;
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 10);
	
	hbuttonbox1 = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox1);
	gtk_box_pack_start (GTK_BOX (dialog_action_area1), hbuttonbox1, 
						TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), 
							   GTK_BUTTONBOX_END);
	
	btnClose = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_Close));
	gtk_widget_show (btnClose);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), btnClose);
	GTK_WIDGET_SET_FLAGS (btnClose, GTK_CAN_DEFAULT);  
	
	// connect signals
	
	gtk_signal_connect_after(GTK_OBJECT(windowPlugins),
							 "destroy",
							 NULL,
							 NULL);
	
	gtk_signal_connect(GTK_OBJECT(windowPlugins),
					   "delete_event",
					   GTK_SIGNAL_FUNC(s_delete_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(btnClose),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_close_clicked),
					   (gpointer) this);
	
	m_windowMain = windowPlugins;
	_constructWindowContents (dialog_vbox1);

	return windowPlugins;
}
