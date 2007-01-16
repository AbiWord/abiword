/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

//////////////////////////////////////////////////////////////////
// THIS CODE RUNS BOTH THE "Find" AND THE "Find-Replace" DIALOGS.
//////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for QNX dialogs,
// like centering them, measuring them, etc.
#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Replace.h"
#include "ap_QNXDialog_Replace.h"
#include "ut_qnxHelper.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Replace::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_QNXDialog_Replace * p = new AP_QNXDialog_Replace(pFactory,id);
	return p;
}

AP_QNXDialog_Replace::AP_QNXDialog_Replace(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Replace(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_comboFind = NULL;
	m_comboReplace = NULL;
	m_checkbuttonMatchCase = NULL;

	m_buttonFindNext = NULL;
	m_buttonReplace = NULL;
	m_buttonReplaceAll = NULL;

	m_buttonCancel = NULL;
}

AP_QNXDialog_Replace::~AP_QNXDialog_Replace(void)
{
}

/*****************************************************************/
static int s_delete_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}

static int s_find_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_Find();
	return Pt_CONTINUE;
}

static int s_replace_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_Replace();
	return Pt_CONTINUE;
}

static int s_replace_all_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_ReplaceAll();
	return Pt_CONTINUE;
}

static int s_cancel_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_Cancel();
	return Pt_CONTINUE;
}

static int s_toggled(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_Toggled();
	return Pt_CONTINUE;
}

static int s_find_entry_activate(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_Find();
	return Pt_CONTINUE;
}

static int s_replace_entry_activate(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_Replace();
	return Pt_CONTINUE;
}
/*****************************************************************/

void AP_QNXDialog_Replace::activate(void)
{
	UT_ASSERT(m_windowMain);
	ConstructWindowName();
	PtSetResource(m_windowMain, Pt_ARG_WINDOW_TITLE, m_WindowName, 0);
//	PtWindowFocus(m_windowMain);
}

void AP_QNXDialog_Replace::destroy(void)
{
	if (!m_windowMain) {
		return;
	}

	_storeWindowData();
	modeless_cleanup();

	PtWidget_t *win = m_windowMain;
	m_windowMain = NULL;
	PtDestroyWidget(win);
}

void AP_QNXDialog_Replace::notifyActiveFrame(XAP_Frame *pFrame)
{
	activate();
}

void AP_QNXDialog_Replace::notifyCloseFrame(XAP_Frame *pFrame)
{
}


void AP_QNXDialog_Replace::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(0);
}

void AP_QNXDialog_Replace::_updateLists() {
	_updateList(m_comboFind,&m_findList);
	_updateList(m_comboReplace,&m_replaceList);
}

void AP_QNXDialog_Replace::_updateList(PtWidget_t *w,UT_GenericVector<unsigned int*> *list)
{
	UT_uint32 i=0;
	if(!w) return;
	if(!list) return;
	
	PtListDeleteAllItems(w);

	for(i = 0;i<list->getItemCount();i++)
	{
		UT_UCS4String ucs4s((UT_UCS4Char*)list->getNthItem(i),0);
		const char *str = ucs4s.utf8_str();
		PtListAddItems(w,&str,1,0);
	}
PtListSelectPos(w,1);
}

void AP_QNXDialog_Replace::runModeless(XAP_Frame * pFrame)
{
	// get the Dialog Id number
	UT_sint32 sid =(UT_sint32)  getDialogId();

	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Save dialog the ID number and pointer to the Dialog
	m_pApp->rememberModelessId( sid,  (XAP_Dialog_Modeless *) m_pDialog);

	// This magic command displays the frame where strings will be found
	connectFocusModeless(mainWindow ,m_pApp);

	// Populate the window's data items
	_populateWindowData();

// this dialog needs this
	setView((FV_View *) (getActiveFrame()->getCurrentView()));

//	UT_QNXCenterWindow(NULL, m_windowMain);

	PtRealizeWidget(m_windowMain);
}

static char *s_get_text_string(PtWidget_t *w) {
	PtArg_t arg;
	char *str = NULL;

	PtSetArg(&arg, Pt_ARG_TEXT_STRING, &str, 0);
	PtGetResources(w, 1, &arg);

	return str;
}

void AP_QNXDialog_Replace::event_Find(void)
{
	char * findEntryText = NULL;

	if (!(findEntryText = s_get_text_string(m_comboFind)) || strlen(findEntryText) == 0) {
		return;
	}
	
	
	setFindString(UT_UCS4String(findEntryText).ucs4_str());
	if(getReverseFind())
		findPrev();
	else
		findNext();
}
		
void AP_QNXDialog_Replace::event_Replace(void)
{
	char * findEntryText;
	char * replaceEntryText;

	findEntryText = s_get_text_string(m_comboFind);
	replaceEntryText = s_get_text_string(m_comboReplace);
	if (!findEntryText || !replaceEntryText) {
		return;
	}
	
	setFindString(UT_UCS4String(findEntryText).ucs4_str());
	setReplaceString(UT_UCS4String(replaceEntryText).ucs4_str());

	if(getReverseFind())
		findReplaceReverse();
	else
		findReplace();
}

void AP_QNXDialog_Replace::event_ReplaceAll(void)
{
	char * findEntryText;
	char * replaceEntryText;

	findEntryText = s_get_text_string(m_comboFind);
	replaceEntryText = s_get_text_string(m_comboReplace);
	if (!findEntryText || !replaceEntryText) {
		return;
	}
		
	setFindString(UT_UCS4String(findEntryText).ucs4_str());
	setReplaceString(UT_UCS4String(replaceEntryText).ucs4_str());	
	findReplaceAll();

}

void AP_QNXDialog_Replace::event_Toggled(void)
{
	UT_ASSERT(m_checkbuttonMatchCase && m_checkbuttonReverse && m_checkbuttonWholeWords);

	setMatchCase((PtWidgetFlags(m_checkbuttonMatchCase) & Pt_SET) ? true: false);
	setWholeWord((PtWidgetFlags(m_checkbuttonWholeWords) & Pt_SET) ? true: false);
	setReverseFind((PtWidgetFlags(m_checkbuttonReverse) & Pt_SET) ? true: false);
}

void AP_QNXDialog_Replace::event_Cancel(void)
{
	m_answer = AP_Dialog_Replace::a_CANCEL;
	done = 1;
	destroy();
}

void AP_QNXDialog_Replace::event_WindowDelete(void)
{
	m_answer = AP_Dialog_Replace::a_CANCEL;	
	done = 1;
	destroy();
}

/*****************************************************************/

PtWidget_t * AP_QNXDialog_Replace::_constructWindow(void)
{
	PtWidget_t *windowReplace;
	PtWidget_t *comboFind;
	PtWidget_t *comboReplace;
	PtWidget_t *checkbuttonMatchCase;
	PtWidget_t *buttonFindNext;
	PtWidget_t *buttonReplace;
	PtWidget_t *buttonReplaceAll;
	PtWidget_t *buttonCancel;



	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// conditionally set title
	if (m_id == AP_DIALOG_ID_FIND)
		windowReplace = abiCreatePhabDialog("ap_QNXDialog_Find",pSS,AP_STRING_ID_DLG_FR_FindTitle);
	else
		windowReplace = abiCreatePhabDialog("ap_QNXDialog_Replace",pSS,AP_STRING_ID_DLG_FR_ReplaceTitle);
		
	SetupContextHelp(windowReplace,this);
	PtAddHotkeyHandler(windowReplace,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	PtAddCallback(windowReplace, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	localizeLabel(abiPhabLocateWidget(windowReplace,"lblFind"),pSS,AP_STRING_ID_DLG_FR_FindLabel);
	
	comboFind = abiPhabLocateWidget(windowReplace,"comboFind"); 
	PtAddCallback(comboFind, Pt_CB_ACTIVATE, s_find_entry_activate, this);

	// the replace label and field are only visible if we're a "replace" dialog
	if (m_id == AP_DIALOG_ID_REPLACE)
	{	
		localizeLabel(abiPhabLocateWidget(windowReplace,"lblReplace"), pSS, AP_STRING_ID_DLG_FR_ReplaceWithLabel);
	
		comboReplace = abiPhabLocateWidget(windowReplace,"comboReplace"); 
		PtAddCallback(comboReplace, Pt_CB_ACTIVATE, s_replace_entry_activate, this);
	} else {
		comboReplace = NULL;
	}
	
	buttonFindNext = abiPhabLocateWidget(windowReplace,"btnFindNext"); 
	localizeLabel(buttonFindNext, pSS, AP_STRING_ID_DLG_FR_FindNextButton);
	PtAddCallback(buttonFindNext, Pt_CB_ACTIVATE, s_find_clicked, this);

	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		buttonReplace = abiPhabLocateWidget(windowReplace,"btnReplace");
		localizeLabel(buttonReplace, pSS, AP_STRING_ID_DLG_FR_ReplaceButton);
		PtAddCallback(buttonReplace, Pt_CB_ACTIVATE, s_replace_clicked, this);

		buttonReplaceAll =abiPhabLocateWidget(windowReplace,"btnReplaceAll"); 
		localizeLabel(buttonReplaceAll, pSS, AP_STRING_ID_DLG_FR_ReplaceAllButton);
		PtAddCallback(buttonReplaceAll, Pt_CB_ACTIVATE, s_replace_all_clicked, this);
	} else {
		buttonReplace = NULL;
		buttonReplaceAll = NULL;
	}

	buttonCancel = abiPhabLocateWidget(windowReplace,"btnCancel"); 
	localizeLabel(buttonCancel, pSS, XAP_STRING_ID_DLG_Cancel);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	checkbuttonMatchCase = abiPhabLocateWidget(windowReplace,"toggleCase");
	localizeLabel(checkbuttonMatchCase, pSS, AP_STRING_ID_DLG_FR_MatchCase);
	PtSetResource(checkbuttonMatchCase,Pt_ARG_FLAGS,getMatchCase(),Pt_SET);
	PtAddCallback(checkbuttonMatchCase, Pt_CB_ACTIVATE, s_toggled, this);


	m_checkbuttonWholeWords = abiPhabLocateWidget(windowReplace,"toggleWholeWord");
	localizeLabel(m_checkbuttonWholeWords,pSS,AP_STRING_ID_DLG_FR_WholeWord);
	PtSetResource(m_checkbuttonWholeWords,Pt_ARG_FLAGS,getWholeWord(),Pt_SET);
	PtAddCallback(m_checkbuttonWholeWords,Pt_CB_ACTIVATE,s_toggled,this);
	
	m_checkbuttonReverse = abiPhabLocateWidget(windowReplace,"toggleReverse");
	localizeLabel(m_checkbuttonReverse,pSS,AP_STRING_ID_DLG_FR_ReverseFind);
	PtSetResource(m_checkbuttonReverse,Pt_ARG_FLAGS,getReverseFind(),Pt_SET);
	PtAddCallback(m_checkbuttonReverse,Pt_CB_ACTIVATE,s_toggled,this);

	// save pointers to members
	m_windowMain = windowReplace;

	m_comboFind = comboFind;
	m_comboReplace = comboReplace;
	m_checkbuttonMatchCase = checkbuttonMatchCase;

	m_buttonFindNext = buttonFindNext;
	m_buttonReplace = buttonReplace;
	m_buttonReplaceAll = buttonReplaceAll;
	m_buttonCancel = buttonCancel;

	m_buttonCancel = buttonCancel;

	
	return windowReplace;
}

void AP_QNXDialog_Replace::_populateWindowData(void)
{
	UT_ASSERT(m_comboFind && m_checkbuttonMatchCase);
	int start=0;
	int end=300;

	// last used find string
	{
		UT_UCS4Char * bufferUnicode = getFindString();
		char * bufferNormal = (char *) UT_calloc(UT_UCS4_strlen(bufferUnicode) + 1, sizeof(char));
		UT_UCS4_strcpy_to_char(bufferNormal, bufferUnicode);
		FREEP(bufferUnicode);
		
		PtSetResource(m_comboFind, Pt_ARG_TEXT_STRING, bufferNormal, 0);

		FREEP(bufferNormal);
	}
	
	
	// last used replace string
	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		UT_ASSERT(m_comboReplace);
		
		UT_UCS4Char * bufferUnicode = getReplaceString();
		char * bufferNormal = (char *) UT_calloc(UT_UCS4_strlen(bufferUnicode) + 1, sizeof(char));
		UT_UCS4_strcpy_to_char(bufferNormal, bufferUnicode);
		FREEP(bufferUnicode);
		
		PtSetResource(m_comboReplace, Pt_ARG_TEXT_STRING, bufferNormal, 0);

		FREEP(bufferNormal);
	}
	_updateLists();

	// match case button
	PtSetResource(m_checkbuttonMatchCase, Pt_ARG_FLAGS, (getMatchCase()) ? Pt_SET : 0, Pt_SET); 

	PtGiveFocus(m_comboFind,NULL);
	PtTextSetSelection(m_comboFind,&start,&end);
}

void AP_QNXDialog_Replace::_storeWindowData(void)
{
	// TODO: nothing?  The actual methods store
	// out last used data to the persist variables,
	// since we need to save state when things actually
	// happen (not when the dialog closes).
}
