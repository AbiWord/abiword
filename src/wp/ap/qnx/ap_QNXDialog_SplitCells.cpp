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

#include <stdlib.h>
#include <Pt.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_SplitCells.h"
#include "ap_QNXDialog_SplitCells.h"
#include "ut_qnxHelper.h"
#include "ap_QNXDialog_Columns.h"

/* Callbacks */

int s_delete_clicked(PtWidget_t *w,AP_QNXDialog_SplitCells *mc,PtCallbackInfo_t *cbinfo)
{
mc->event_Close();
return Pt_CONTINUE;
}

int s_split_left(PtWidget_t *w,AP_QNXDialog_SplitCells *mc,PtCallbackInfo_t *cbinfo)
{
mc->setSplitType(hori_left);
mc->onSplit();
return Pt_CONTINUE;
}

int s_split_right(PtWidget_t *w,AP_QNXDialog_SplitCells *mc,PtCallbackInfo_t *cbinfo)
{
mc->setSplitType(hori_right);
mc->onSplit();
return Pt_CONTINUE;
}

int s_split_below(PtWidget_t *w,AP_QNXDialog_SplitCells *mc,PtCallbackInfo_t *cbinfo)
{
mc->setSplitType(vert_below);
mc->onSplit();
return Pt_CONTINUE;
}
int s_split_above(PtWidget_t *w,AP_QNXDialog_SplitCells *mc,PtCallbackInfo_t *cbinfo)
{
mc->setSplitType(vert_above);
mc->onSplit();
return Pt_CONTINUE;
}

/*end callbacks */
XAP_Dialog * AP_QNXDialog_SplitCells::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_QNXDialog_SplitCells * p = new AP_QNXDialog_SplitCells(pFactory,id);
	return p;
}

AP_QNXDialog_SplitCells::AP_QNXDialog_SplitCells(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_SplitCells(pDlgFactory,id)
{
}

AP_QNXDialog_SplitCells::~AP_QNXDialog_SplitCells(void)
{
}

void AP_QNXDialog_SplitCells::runModeless(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them

	PtWidget_t *mainWindow = _constructWindow();

	UT_sint32 sid = (UT_sint32) getDialogId();
	m_pApp->rememberModelessId(sid,(XAP_Dialog_Modeless *)m_pDialog);
	connectFocusModeless(mainWindow,m_pApp);

	PtRealizeWidget(mainWindow);
	startUpdater();
}

void AP_QNXDialog_SplitCells::setSensitivity(AP_CellSplitType splitThis, bool bSens)
{
int status;
if(bSens==true)
	status=Pt_FALSE;
else
	status=Pt_TRUE;

}

void AP_QNXDialog_SplitCells::event_Close(void)
{
	m_answer = AP_Dialog_SplitCells::a_CANCEL;
	destroy();
}

void AP_QNXDialog_SplitCells::destroy(void)
{
	finalize();
	PtDestroyWidget(m_windowMain);
}

void AP_QNXDialog_SplitCells::activate(void)
{
	UT_ASSERT (m_windowMain);
        
	ConstructWindowName();

	PtSetResource(m_windowMain,Pt_ARG_WINDOW_TITLE,m_WindowName,0);
	setAllSensitivities();

}

void AP_QNXDialog_SplitCells::notifyActiveFrame(XAP_Frame *pFrame)
{
    UT_ASSERT(m_windowMain);
	ConstructWindowName();
	PtSetResource(m_windowMain,Pt_ARG_WINDOW_TITLE,m_WindowName,0);
	setAllSensitivities();
}

/*****************************************************************/
PtWidget_t * AP_QNXDialog_SplitCells::_constructWindow(void)
{
PtWidget_t *mainwindow;
PtWidget_t *btn;
ConstructWindowName();
const XAP_StringSet * pSS = m_pApp->getStringSet();



	mainwindow= abiCreatePhabDialog("ap_QNXDialog_MergeCells",pSS,XAP_STRING_ID_DLG_Cancel);
	PtSetResource(mainwindow,Pt_ARG_WINDOW_TITLE,m_WindowName,0);
	SetupContextHelp(mainwindow,this);
	PtAddHotkeyHandler(mainwindow,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	PtAddCallback(mainwindow, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	localizeLabel(abiPhabLocateWidget(mainwindow,"grpSplitCells"),pSS,AP_STRING_ID_DLG_SplitCells_Frame );

localizeLabel(abiPhabLocateWidget(mainwindow,"lblSplitLeft"),pSS,AP_STRING_ID_DLG_SplitCells_Left);

localizeLabel(abiPhabLocateWidget(mainwindow,"lblSplitRight"),pSS,AP_STRING_ID_DLG_SplitCells_Right);

localizeLabel(abiPhabLocateWidget(mainwindow,"lblSplitAbove"),pSS,AP_STRING_ID_DLG_SplitCells_Above);

localizeLabel(abiPhabLocateWidget(mainwindow,"lblSplitBelow"),pSS,AP_STRING_ID_DLG_SplitCells_Below);

	btn=abiPhabLocateWidget(mainwindow,"btnClose");
	localizeLabel(btn,pSS,XAP_STRING_ID_DLG_Close);
	PtAddCallback(btn,Pt_CB_ACTIVATE,s_delete_clicked,this);

	btn=abiPhabLocateWidget(mainwindow,"btnSplitLeft");
	m_SplitLeft=btn;
	label_button_with_abi_pixmap(btn,"tb_SplitLeft_xpm");
	PtAddCallback(btn,Pt_CB_ACTIVATE,s_split_left,this);

	btn=abiPhabLocateWidget(mainwindow,"btnSplitRight");
	m_SplitRight=btn;
	label_button_with_abi_pixmap(btn,"tb_SplitRight_xpm");
	PtAddCallback(btn,Pt_CB_ACTIVATE,s_split_right,this);
	
	btn=abiPhabLocateWidget(mainwindow,"btnSplitAbove");
	m_SplitAbove = btn;
	label_button_with_abi_pixmap(btn,"tb_SplitAbove_xpm");
	PtAddCallback(btn,Pt_CB_ACTIVATE,s_split_above,this);
	
	btn=abiPhabLocateWidget(mainwindow,"btnSplitBelow");
	m_SplitBelow=btn;
	label_button_with_abi_pixmap(btn,"tb_SplitBelow_xpm");
	PtAddCallback(btn,Pt_CB_ACTIVATE,s_split_below,this);

m_windowMain = mainwindow;

return mainwindow;
}


