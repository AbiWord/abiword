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
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_QNXDlg_PluginManager.h"
#include "xap_QNXDlg_FileOpenSaveAs.h"

#include "xap_Module.h"
#include "xap_ModuleManager.h"

#include "ut_qnxHelper.h"

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

XAP_Dialog * XAP_QNXDialog_PluginManager::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	XAP_QNXDialog_PluginManager * p = new XAP_QNXDialog_PluginManager(pFactory,id);
	return p;
}

XAP_QNXDialog_PluginManager::XAP_QNXDialog_PluginManager(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: XAP_Dialog_PluginManager(pDlgFactory,id)
{
}

XAP_QNXDialog_PluginManager::~XAP_QNXDialog_PluginManager(void)
{
}

/*****************************************************************/
/*****************************************************************/

void XAP_QNXDialog_PluginManager::event_DeactivateAll ()
{
	deactivateAllPlugins ();
	_refreshAll ();
}



void XAP_QNXDialog_PluginManager::event_Close () {
	if(!done++) {
		;//Set some state
	}
}

void XAP_QNXDialog_PluginManager::event_Deactivate ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	XAP_Module * pModule = 0;

	unsigned short * selectedRow = NULL;
	PtGetResource(m_clist, Pt_ARG_SELECTION_INDEXES, &selectedRow, 0);
	if (selectedRow)
	{
			pModule = (XAP_Module *) XAP_ModuleManager::instance().enumModules()->getNthItem(selectedRow[0] + 1);
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

void XAP_QNXDialog_PluginManager::event_Load ()
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

	// todo: cd to the proper plugin directory
	
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

void XAP_QNXDialog_PluginManager::event_Select1 ()
{
	_refreshTab2 ();
}

/*****************************************************************/
/*****************************************************************/

void XAP_QNXDialog_PluginManager::_refreshAll ()
{
	_refreshTab1();

	PtListSelectPos(m_clist, 1);

	_refreshTab2();
}

void XAP_QNXDialog_PluginManager::_refreshTab1 ()
{
	const char * text[2] = {NULL, NULL};
	XAP_Module * pModule = 0;

	// first, refresh the CList
	PtListDeleteAllItems(m_clist);
	
	const UT_Vector * pVec = XAP_ModuleManager::instance().enumModules ();

	for (UT_uint32 i = 0; i < pVec->size(); i++) {
		pModule = (XAP_Module *)pVec->getNthItem (i);
		text [0] = pModule->getModuleInfo()->name;
		PtListAddItems(m_clist, text, 1, 0);
	}
}

void XAP_QNXDialog_PluginManager::_refreshTab2 ()
{

	PtSetResource(m_desc, Pt_ARG_TEXT_STRING, "", 0);

	XAP_Module * pModule = 0;

	unsigned short *selectedRow = NULL;
	PtGetResource(m_clist, Pt_ARG_SELECTION_INDEXES, &selectedRow, 0);
	printf("Selected row is %d \n", (selectedRow) ? selectedRow[0] : -12);
	if (selectedRow && selectedRow[0])
	{
			pModule = (XAP_Module *) XAP_ModuleManager::instance().enumModules()->getNthItem(selectedRow[0] + 1);
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

	PtSetResource(m_name, Pt_ARG_TEXT_STRING, name, 0);
	PtSetResource(m_author, Pt_ARG_TEXT_STRING, author, 0);
	PtSetResource(m_version,  Pt_ARG_TEXT_STRING, version, 0);
	PtSetResource(m_desc, Pt_ARG_TEXT_STRING, desc, 0);
}

/*****************************************************************/
/*****************************************************************/

static int s_close_clicked (PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_PluginManager * dlg = (XAP_QNXDialog_PluginManager *)data;
	UT_ASSERT (dlg);

	dlg->event_Close();
	return Pt_CONTINUE;
}

static int s_deactivate_clicked (PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_PluginManager * dlg = (XAP_QNXDialog_PluginManager *)data;
	UT_ASSERT (dlg);

	dlg->event_Deactivate();
	return Pt_CONTINUE;
}

static int s_deactivate_all_clicked (PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_PluginManager * dlg = (XAP_QNXDialog_PluginManager *)data;
	UT_ASSERT (dlg);

	dlg->event_DeactivateAll ();
	return Pt_CONTINUE;
}

static int s_load_clicked (PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_PluginManager * dlg = (XAP_QNXDialog_PluginManager *)data;
	UT_ASSERT (dlg);

	dlg->event_Load ();
	return Pt_CONTINUE;
}

static int s_clist_selected (PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_PluginManager * dlg = (XAP_QNXDialog_PluginManager *)data;
	UT_ASSERT (dlg);

	dlg->event_Select1 ();
	return Pt_CONTINUE;
}

/*****************************************************************/
/*****************************************************************/

void XAP_QNXDialog_PluginManager::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;

	// Set the parent window for this dialog 
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	PtSetParentWidget(parentWindow);

	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(mainWindow, pFrame);

	// Populate the window's data items
	//_populateWindowData();
	
	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);
	PtRealizeWidget(mainWindow);

	_refreshAll();
	
	int count;
	count = PtModalStart();
	done = 0;
	while(!done) {
		PtProcessEvent();
	}
	PtModalEnd(MODAL_END_ARG(count));

	UT_QNXBlockWidget(parentWindow, 0);
	PtDestroyWidget(mainWindow);
}

PtWidget_t * XAP_QNXDialog_PluginManager::_constructWindow ()
{
	PtWidget_t *windowPlugins;
	
	PtWidget_t *clistPlugins;
	PtWidget_t *lblActivePlugins;
	PtWidget_t *btnDeactivate;
	PtWidget_t *btnDeactivateAll;
	PtWidget_t *btnInstall;
	PtWidget_t *lblPluginList;
	PtWidget_t *lblName;
	PtWidget_t *lblDesc;
	PtWidget_t *lblAuthor;
	PtWidget_t *lblVersion;
	PtWidget_t *entryName;
	PtWidget_t *entryAuthor;
	PtWidget_t *entryVersion;
	PtWidget_t *textDescription;
	PtWidget_t *lblPluginDetails;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	PtArg_t args[10];
	int n;

	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(XAP_STRING_ID_DLG_PLUGIN_MANAGER_TITLE) , 0);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	windowPlugins = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowPlugins, Pt_CB_WINDOW_CLOSING, s_close_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 10, 0);
	PtWidget_t *vgroup = PtCreateWidget(PtGroup, windowPlugins, n, args);

	n = 0;
#define PANEL_WIDTH 300
#define PANEL_HEIGHT 200
	PtSetArg(&args[n++], Pt_ARG_WIDTH, PANEL_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, PANEL_HEIGHT, 0);
	PtWidget_t *panelGroup = PtCreateWidget(PtPanelGroup, vgroup, n, args);	
	//PtAddCallback(panelGroup, Pt_CB_PG_PANEL_SWITCHING, s_tabs_clicked, this);

	/* Tab 1 -- Plugin Listing */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TITLE, pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_LIST), 0);
	PtWidget_t *tabList = PtCreateWidget(PtPane, panelGroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EQUAL_SIZE_HORIZONTAL, Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	PtWidget_t *hgroup = PtCreateWidget(PtGroup, tabList, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 150, 0);
	clistPlugins = PtCreateWidget(PtList, hgroup, n, args);
	//lblActivePlugins = gtk_label_new (pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_ACTIVE));
/*
	gtk_signal_connect (GTK_OBJECT(clistPlugins), "select_row",
						GTK_SIGNAL_FUNC(s_clist_selected),
						(gpointer)this);
*/
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EQUAL_SIZE_HORIZONTAL, Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	PtWidget_t *vgroupbut = PtCreateWidget(PtGroup, hgroup, n, args);
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE), 0);
	btnDeactivate = PtCreateWidget(PtButton, vgroupbut, n, args);
	PtAddCallback(btnDeactivate, Pt_CB_ACTIVATE, s_deactivate_clicked, this);
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE_ALL), 0);
	btnDeactivateAll = PtCreateWidget(PtButton, vgroupbut, n, args);
	PtAddCallback(btnDeactivateAll, Pt_CB_ACTIVATE, s_deactivate_all_clicked, this);
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_INSTALL), 0);
	btnInstall =  PtCreateWidget(PtButton, vgroupbut, n, args);
	PtAddCallback(btnInstall, Pt_CB_ACTIVATE, s_load_clicked, this);


	/* Tab 2 -- Plugin Details */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TITLE, pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_LIST), 0);
	PtWidget_t *tabDetail = PtCreateWidget(PtPane, panelGroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ROWS_COLS, 2, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EQUAL_SIZE_HORIZONTAL, Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	PtWidget_t *detailgroup = PtCreateWidget(PtGroup, tabDetail, n, args);	
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_NAME), 0);
	lblName = PtCreateWidget(PtLabel, detailgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 100, 0);
	entryName = PtCreateWidget(PtText, detailgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_DESC), 0);
	lblDesc = PtCreateWidget(PtLabel, detailgroup, n, args);

	n = 0;
	textDescription =  PtCreateWidget(PtText, detailgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_AUTHOR), 0);
	lblAuthor = PtCreateWidget(PtLabel, detailgroup, n, args);

	n = 0;
	entryAuthor =  PtCreateWidget(PtText, detailgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_PLUGIN_MANAGER_VERSION), 0);
	lblVersion = PtCreateWidget(PtLabel, detailgroup, n, args);

	n = 0;
	entryVersion =  PtCreateWidget(PtText, detailgroup, n, args);

/*
	n = 0;
	lblPluginDetails =  PtCreateWidget(PtText, detailgroup, n, args);
*/

	/* Put the bottom row of buttons in */
	n = 0;
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	PtWidget_t *buttonOK = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_close_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Close), 0);
	PtWidget_t *buttonClose = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonClose, Pt_CB_ACTIVATE, s_close_clicked, this);
	
	// assign pointers to important widgets
	m_clist = clistPlugins;
	m_name = entryName;
	m_author = entryAuthor;
	m_version = entryVersion;
	m_desc = textDescription;
	
	m_windowMain = windowPlugins;

	return windowPlugins;
}
