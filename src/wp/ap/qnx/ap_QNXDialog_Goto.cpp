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

#include <Ap.h>
#include <stdlib.h>
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Goto.h"
#include "ap_QNXDialog_Goto.h"
#include "ut_qnxHelper.h"

#include "fv_View.h"

/*****************************************************************/
XAP_Dialog * AP_QNXDialog_Goto::static_constructor(XAP_DialogFactory * pFactory,
												   XAP_Dialog_Id id)
{
	AP_QNXDialog_Goto * p = new AP_QNXDialog_Goto(pFactory,id);
	return p;
}

AP_QNXDialog_Goto::AP_QNXDialog_Goto(XAP_DialogFactory * pDlgFactory,
									   XAP_Dialog_Id id)
	: AP_Dialog_Goto(pDlgFactory,id)
{
	m_mainWindow = NULL;
}

AP_QNXDialog_Goto::~AP_QNXDialog_Goto(void)
{
}

/********/
static int s_gotoClicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Goto *dlg = (AP_QNXDialog_Goto *)data;
	UT_ASSERT(dlg);

	dlg->processGoto (NULL);

	return Pt_CONTINUE;
}

static int s_nextClicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Goto *dlg = (AP_QNXDialog_Goto *)data;
	UT_ASSERT(dlg);

	dlg->processGoto ("+1");

	return Pt_CONTINUE;
}

static int s_prevClicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Goto *dlg = (AP_QNXDialog_Goto *)data;
	UT_ASSERT(dlg);

	dlg->processGoto ("-1");

	return Pt_CONTINUE;
}

static int s_closeClicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Goto *dlg = (AP_QNXDialog_Goto *)data;
	UT_ASSERT(dlg);

	dlg->destroy();
	
	return Pt_CONTINUE;
}

static int s_deleteClicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Goto *dlg = (AP_QNXDialog_Goto *)data;
	UT_ASSERT(dlg);

	dlg->destroy();
	
	return Pt_CONTINUE;
}


void AP_QNXDialog_Goto::runModeless (XAP_Frame * pFrame)
{


	_constructWindow ();
	UT_ASSERT (m_mainWindow);

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid = (UT_sint32) getDialogId ();

	m_pApp->rememberModelessId(sid, (XAP_Dialog_Modeless *) m_pDialog);

	// This magic command displays the frame that characters will be
	// inserted into.
	connectFocusModeless (m_mainWindow, m_pApp);

	UT_QNXCenterWindow(NULL, m_mainWindow);

	PtRealizeWidget(m_mainWindow);
}

void AP_QNXDialog_Goto::activate (void)
{
	UT_ASSERT (m_mainWindow);
	ConstructWindowName();
	PtSetResource(m_mainWindow, Pt_ARG_WINDOW_TITLE, m_WindowName, 0);
}

void AP_QNXDialog_Goto::destroy (void)
{
	if (!m_mainWindow) {
		return;
	}
	
	modeless_cleanup();
	PtWidget_t *win = m_mainWindow;
	m_mainWindow = NULL;
	PtDestroyWidget(win);
}

void AP_QNXDialog_Goto::notifyActiveFrame(XAP_Frame *pFrame)
{
	activate();
}

#define _TR(x) (char *)UT_XML_transNoAmpersands((x))

PtWidget_t * AP_QNXDialog_Goto::_constructWindow (void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	PtArg_t  args[10];
	int		  n;

	n = 0;
	ConstructWindowName();
	m_mainWindow = abiCreatePhabDialog("ap_QNXDialog_Goto",pSS,XAP_STRING_ID_DLG_OK);
	PtSetResource(m_mainWindow,Pt_ARG_WINDOW_TITLE,m_WindowName,0);
	SetupContextHelp(m_mainWindow,this);
	PtAddHotkeyHandler(m_mainWindow,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	PtAddCallback(m_mainWindow, Pt_CB_WINDOW_CLOSING, s_deleteClicked, this);
	
	localizeLabel(abiPhabLocateWidget(m_mainWindow,"lblWhat"), pSS, (AP_STRING_ID_DLG_Goto_Label_What ));

	m_wList = abiPhabLocateWidget(m_mainWindow,"listWhat");
	char **tmp2 = getJumpTargets ();
	int indx;
	for (indx = 0; tmp2[indx] != NULL; indx++) { ; }
	PtListAddItems(m_wList, (const char **)tmp2, indx, 0);
	PtListSelectPos(m_wList, 1);

	localizeLabel(abiPhabLocateWidget(m_mainWindow,"lblNumber"), pSS, (AP_STRING_ID_DLG_Goto_Label_Number ));

	m_wEntry = abiPhabLocateWidget(m_mainWindow,"txtNumber"); 

	localizeLabel(abiPhabLocateWidget(m_mainWindow,"mTxtInfo"), pSS, AP_STRING_ID_DLG_Goto_Label_Help);

	m_wPrev = abiPhabLocateWidget(m_mainWindow,"btnPrev");
	localizeLabel(m_wPrev, pSS, AP_STRING_ID_DLG_Goto_Btn_Prev);
	PtAddCallback(m_wPrev, Pt_CB_ACTIVATE, s_prevClicked, this);

	m_wNext = abiPhabLocateWidget(m_mainWindow,"btnNext");
	localizeLabel(m_wNext, pSS, AP_STRING_ID_DLG_Goto_Btn_Next);
	PtAddCallback(m_wNext, Pt_CB_ACTIVATE, s_nextClicked, this);

	m_wGoto = abiPhabLocateWidget(m_mainWindow,"btnGoto");
	localizeLabel(m_wGoto, pSS, AP_STRING_ID_DLG_Goto_Btn_Goto);
	PtAddCallback(m_wGoto, Pt_CB_ACTIVATE, s_gotoClicked, this);

	m_wClose = abiPhabLocateWidget(m_mainWindow,"btnClose");
	localizeLabel(m_wClose, pSS, XAP_STRING_ID_DLG_Close);
	PtAddCallback(m_wClose, Pt_CB_ACTIVATE, s_closeClicked, this);

	return (m_mainWindow);
}

int AP_QNXDialog_Goto::getSelectedRow (void)
{
	short *index = NULL;

	PtGetResource(m_wList, Pt_ARG_SELECTION_INDEXES, &index, 0);

	if (!index) {
		return 0;
	}
	
	return index[0]-1;
}

void AP_QNXDialog_Goto::processGoto (const char *number)
{
	//If a NULL is passed as the number, grab the string from the text box
	if (!number) {
		char *value = NULL;
		PtGetResource(m_wEntry, Pt_ARG_TEXT_STRING, &value, 0);
		number = value;
	}

	if (!number || !*number) {
		UT_DEBUGMSG(("No goto string!"));
		return;
	}

	UT_UCSChar *ucsnumber = (UT_UCSChar *) g_try_malloc (sizeof (UT_UCSChar) * (strlen(number) + 1));
	UT_UCS4_strcpy_char (ucsnumber, number);

	int target = getSelectedRow ();
	getView()->gotoTarget ((AP_JumpTarget) target, ucsnumber);

	FREEP (ucsnumber);
}


