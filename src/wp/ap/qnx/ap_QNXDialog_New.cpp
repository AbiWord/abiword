/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_New.h"
#include "ap_QNXDialog_New.h"
#include "ut_qnxHelper.h"

#include "xap_Dlg_FileOpenSaveAs.h"
#include "ie_imp.h"

#define IEFT_AbiWord_1 IE_Imp::fileTypeForSuffix(".abw")

/*************************************************************************/

XAP_Dialog * AP_QNXDialog_New::static_constructor(XAP_DialogFactory * pFactory,
												   XAP_Dialog_Id id)
{
	AP_QNXDialog_New * p = new AP_QNXDialog_New(pFactory,id);
	return p;
}

AP_QNXDialog_New::AP_QNXDialog_New(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_New(pDlgFactory,id), m_pFrame(0)
{
}

AP_QNXDialog_New::~AP_QNXDialog_New(void)
{
}

void AP_QNXDialog_New::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	m_pFrame = pFrame;
	
	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the window of the parent frame
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	PtSetParentWidget(parentWindow);

	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(mainWindow, pFrame);

	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);
	PtRealizeWidget(mainWindow);

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

/*************************************************************************/
/*************************************************************************/

void AP_QNXDialog_New::event_Ok ()
{
	char *name;
	int *flags;

	setAnswer (AP_Dialog_New::a_OK);
	
	if (PtGetResource(m_radioExisting, Pt_ARG_FLAGS, &flags, 0) == 0 && *flags & Pt_SET)
	{
		setOpenType(AP_Dialog_New::open_Existing);
		//Not in unix version
		PtGetResource(m_entryFilename, Pt_ARG_TEXT_STRING, &name, NULL);
		setFileName(name);
	}
	else if (PtGetResource(m_radioNew, Pt_ARG_FLAGS, &flags, 0) == 0 && *flags & Pt_SET)
	{
		setOpenType(AP_Dialog_New::open_Template);
		/*Not in unix version */
		PtTreeItem_t *item;
		item = PtTreeGetCurrent(m_tree);
		setTemplateName(item->string);
	}
	else
	{
		setOpenType(AP_Dialog_New::open_New);
	}

	done++;
}

void AP_QNXDialog_New::event_Cancel ()
{
	if(!done++) {
		setAnswer (AP_Dialog_New::a_CANCEL);
	}
}

void AP_QNXDialog_New::event_ToggleUseTemplate (const char * name)
{
	setTemplateName (name);
}

void AP_QNXDialog_New::event_ToggleOpenExisting ()
{
	XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_OPEN;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) m_pFrame->getDialogFactory();

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	pDialog->setCurrentPathname(0);
	pDialog->setSuggestFilename(false);

	UT_uint32 filterCount = IE_Imp::getImporterCount();
	const char ** szDescList = (const char **) calloc(filterCount + 1,
													  sizeof(char *));
	const char ** szSuffixList = (const char **) calloc(filterCount + 1,
														sizeof(char *));
	IEFileType * nTypeList = (IEFileType *) calloc(filterCount + 1,
												   sizeof(IEFileType));
	UT_uint32 k = 0;

	while (IE_Imp::enumerateDlgLabels(k, &szDescList[k], 
									  &szSuffixList[k], &nTypeList[k]))
			k++;

	pDialog->setFileTypeList(szDescList, szSuffixList, 
							 (const UT_sint32 *) nTypeList);

	pDialog->setDefaultFileType(IEFT_AbiWord_1);

	pDialog->runModal(m_pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const char * szResultPathname = pDialog->getPathname();
		if (szResultPathname && *szResultPathname)
		{
			// update the entry box
			PtSetResource(m_entryFilename, Pt_ARG_TEXT_STRING, szResultPathname, NULL);
			setFileName (szResultPathname);
		}
	}

	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);
	
	pDialogFactory->releaseDialog(pDialog);
}

void AP_QNXDialog_New::event_ToggleStartNew ()
{	
	// nada
}

void AP_QNXDialog_New::event_ToggleSelection(PtWidget_t *w) {
	if(m_radioNew != w) {
		PtSetResource(m_radioNew, Pt_ARG_FLAGS, 0, Pt_SET);
	}
	if(m_radioExisting != w) {
		PtSetResource(m_radioExisting, Pt_ARG_FLAGS, 0, Pt_SET);
	}
	if(m_radioEmpty != w) {
		PtSetResource(m_radioEmpty, Pt_ARG_FLAGS, 0, Pt_SET);
	}
}

/*************************************************************************/
/*************************************************************************/

int s_ok_clicked (PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_New * dlg = (AP_QNXDialog_New *)data;
	UT_ASSERT(dlg);
	dlg->event_Ok();
	return Pt_CONTINUE;
}

int s_cancel_clicked (PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_New * dlg = (AP_QNXDialog_New *)data;
	UT_ASSERT(dlg);
	dlg->event_Cancel();
	return Pt_CONTINUE;
}

int s_choose_clicked (PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_New * dlg = (AP_QNXDialog_New *)data;
	dlg->event_ToggleOpenExisting();
	return Pt_CONTINUE;
}

int s_radio_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t *info) {
	AP_QNXDialog_New * dlg = (AP_QNXDialog_New *)data;
	dlg->event_ToggleSelection(w);
	return Pt_CONTINUE;
}

/*************************************************************************/
/*************************************************************************/

PtWidget_t * AP_QNXDialog_New::_constructWindow ()
{
	PtWidget_t *mainWindow;
	PtWidget_t *dialog_vbox1;
	PtWidget_t *hgroup, *vgroup;

	PtArg_t args[10];
	int 	n;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, 
	    UT_XML_transNoAmpersands(pSS->getValue(AP_STRING_ID_DLG_NEW_Title)), 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WIDTH, 200, 0);
	mainWindow = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(mainWindow, Pt_CB_WINDOW_CLOSING, s_cancel_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 5, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_STRETCH_HORIZONTAL, Pt_GROUP_STRETCH_HORIZONTAL);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EXCLUSIVE, Pt_GROUP_EXCLUSIVE);
	PtSetArg(&args[n++], Pt_ARG_RESIZE_FLAGS, Pt_RESIZE_XY_AS_REQUIRED, 
											  Pt_RESIZE_XY_AS_REQUIRED | Pt_RESIZE_XY_ALWAYS);
	vgroup = PtCreateWidget(PtGroup, mainWindow, n, args);

	/* New Document from Template */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ROWS_COLS, 1, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EQUAL_SIZE_HORIZONTAL, Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_NEW_Create), NULL);
	//Disable this choice for now
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_BLOCKED, Pt_SELECTABLE | Pt_BLOCKED);
	m_radioNew = PtCreateWidget(PtToggleButton, hgroup, n, args);
	PtAddCallback(m_radioNew, Pt_CB_ACTIVATE, s_radio_clicked, this);

	//Others do this in a tab ... I think a tree is better
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 50, 0);
	//Disable this choice for now
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_BLOCKED, Pt_SELECTABLE | Pt_BLOCKED);
	m_tree = PtCreateWidget(PtTree, hgroup, n, args);

	PtTreeItem_t *item, *brother;
	for (UT_uint32 i = 0; i < getNumTabs(); i++) {
		const TemplateData * td = getListForTab (i+1);
		
		// todo: populate the notebook with this data
		item = PtTreeAllocItem(m_tree, (char *)getTabName(i + 1), -1, -1);
		if(i == 0) {
			PtTreeAddFirst(m_tree, item, NULL);	
		} else {
			PtTreeAddAfter(m_tree, item, brother);
		}
		brother = item;
	}

	/* Open Existing Document */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ROWS_COLS, 1, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EQUAL_SIZE_HORIZONTAL, Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_NEW_Open), NULL);
	//Disable this choice for now
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_BLOCKED, Pt_SELECTABLE | Pt_BLOCKED);
	m_radioExisting = PtCreateWidget(PtToggleButton, hgroup, n, args);
	PtAddCallback(m_radioExisting, Pt_CB_ACTIVATE, s_radio_clicked, this);

	n = 0;
	PtWidget_t *hgroup2 = PtCreateWidget(PtGroup, hgroup, n, args);

	n = 0; 
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2*ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_NEW_NoFile), NULL);
	//Disable this choice for now
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_BLOCKED, Pt_SELECTABLE | Pt_BLOCKED);
	m_entryFilename = PtCreateWidget(PtText, hgroup2, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_NEW_Choose), NULL);
	//Disable this choice for now
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_BLOCKED, Pt_SELECTABLE | Pt_BLOCKED);
	PtWidget_t *choose = PtCreateWidget(PtButton, hgroup2, n, args);
	PtAddCallback(choose, Pt_CB_ACTIVATE, s_choose_clicked, this);

	/* Start w/ Empty Document */
	n = 0;
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_SET, Pt_SET);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_NEW_StartEmpty), NULL);
	m_radioEmpty = PtCreateWidget(PtToggleButton, hgroup, n, args);
	PtAddCallback(m_radioEmpty, Pt_CB_ACTIVATE, s_radio_clicked, this);

	/* Bottom row of buttons */
	n = 0;
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtCreateWidget(PtLabel, hgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *buttonCancel = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *buttonOK = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	
	// assign pointers to widgets
	m_mainWindow   = mainWindow;
	m_buttonOk     = buttonOK;
	m_buttonCancel = buttonCancel;

	return mainWindow;
}


