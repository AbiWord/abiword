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
#include "ap_Dialog_MergeCells.h"
#include "ap_QNXDialog_MergeCells.h"
#include "ut_qnxHelper.h"
#include "ap_QNXDialog_Columns.h"

/* Callbacks */

int s_delete_clicked(PtWidget_t *w,AP_QNXDialog_MergeCells *mc,PtCallbackInfo_t *cbinfo)
{
mc->event_Close();
return Pt_CONTINUE;
}

int s_merge_left(PtWidget_t *w,AP_QNXDialog_MergeCells *mc,PtCallbackInfo_t *cbinfo)
{
mc->setMergeType(AP_Dialog_MergeCells::radio_left);
mc->onMerge();
return Pt_CONTINUE;
}
int s_merge_right(PtWidget_t *w,AP_QNXDialog_MergeCells *mc,PtCallbackInfo_t *cbinfo)
{
mc->setMergeType(AP_Dialog_MergeCells::radio_right);
mc->onMerge();
return Pt_CONTINUE;
}
int s_merge_below(PtWidget_t *w,AP_QNXDialog_MergeCells *mc,PtCallbackInfo_t *cbinfo)
{
mc->setMergeType(AP_Dialog_MergeCells::radio_below);
mc->onMerge();
return Pt_CONTINUE;
}
int s_merge_above(PtWidget_t *w,AP_QNXDialog_MergeCells *mc,PtCallbackInfo_t *cbinfo)
{
mc->setMergeType(AP_Dialog_MergeCells::radio_above);
mc->onMerge();
return Pt_CONTINUE;
}

/*end callbacks */
XAP_Dialog * AP_QNXDialog_MergeCells::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_QNXDialog_MergeCells * p = new AP_QNXDialog_MergeCells(pFactory,id);
	return p;
}

AP_QNXDialog_MergeCells::AP_QNXDialog_MergeCells(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_MergeCells(pDlgFactory,id)
{
}

AP_QNXDialog_MergeCells::~AP_QNXDialog_MergeCells(void)
{
}

void AP_QNXDialog_MergeCells::runModeless(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them

	PtWidget_t *mainWindow = _constructWindow();

	UT_sint32 sid = (UT_sint32) getDialogId();
	m_pApp->rememberModelessId(sid,(XAP_Dialog_Modeless *)m_pDialog);
	connectFocusModeless(mainWindow,m_pApp);

	PtRealizeWidget(mainWindow);
	startUpdater();
}

void AP_QNXDialog_MergeCells::setSensitivity(AP_Dialog_MergeCells::mergeWithCell mergeThis, bool bSens)
{
int status;
if(bSens==true)
	status=Pt_FALSE;
else
	status=Pt_TRUE;

	switch(mergeThis)
	{
		case AP_Dialog_MergeCells::radio_left:
			PtSetResource(m_MergeLeft,Pt_ARG_FLAGS,status,Pt_GHOST|Pt_BLOCKED);
			break;
		case AP_Dialog_MergeCells::radio_right:
			PtSetResource(m_MergeRight,Pt_ARG_FLAGS,status,Pt_GHOST|Pt_BLOCKED);
			break;
		case AP_Dialog_MergeCells::radio_above:
			PtSetResource(m_MergeAbove,Pt_ARG_FLAGS,status,Pt_GHOST|Pt_BLOCKED);
			break;
		case AP_Dialog_MergeCells::radio_below:
			PtSetResource(m_MergeBelow,Pt_ARG_FLAGS,status,Pt_GHOST|Pt_BLOCKED);
			break;
	}

}

void AP_QNXDialog_MergeCells::event_Close(void)
{
	m_answer = AP_Dialog_MergeCells::a_CANCEL;
	destroy();
}

void AP_QNXDialog_MergeCells::destroy(void)
{
	finalize();
	PtDestroyWidget(m_windowMain);
}

void AP_QNXDialog_MergeCells::activate(void)
{
	UT_ASSERT (m_windowMain);
        
	ConstructWindowName();

	PtSetResource(m_windowMain,Pt_ARG_WINDOW_TITLE,m_WindowName,0);
	setAllSensitivities();

}

void AP_QNXDialog_MergeCells::notifyActiveFrame(XAP_Frame *pFrame)
{
    UT_ASSERT(m_windowMain);
	ConstructWindowName();
	PtSetResource(m_windowMain,Pt_ARG_WINDOW_TITLE,m_WindowName,0);
	setAllSensitivities();
}

/*****************************************************************/
PtWidget_t * AP_QNXDialog_MergeCells::_constructWindow(void)
{
PtWidget_t *mainwindow;
int n=0;
PtArg_t args[10];
PtWidget_t *btn,*maingroup;
ConstructWindowName();
const XAP_StringSet * pSS = m_pApp->getStringSet();



	PtSetArg(&args[n++],Pt_ARG_WINDOW_TITLE,m_WindowName,0);
  PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
  PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, Ph_WM_FFRONT, ABI_MODAL_WINDOW_MANAGE_FLAGS|Ph_WM_FFRONT);
	PtSetArg(&args[n++],Pt_ARG_WINDOW_STATE,Pt_TRUE,Ph_WM_STATE_ISFRONT);
	PtSetArg(&args[n++],Pt_ARG_FLAGS,Pt_FALSE,Pt_GETS_FOCUS );
	PtSetArg(&args[n++],Pt_ARG_RESIZE_FLAGS,Pt_TRUE,Pt_RESIZE_XY_AS_REQUIRED);
	mainwindow= PtCreateWidget(PtWindow,Pt_NO_PARENT,n,args);
	SetupContextHelp(mainwindow,this);
	PtAddHotkeyHandler(mainwindow,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	PtAddCallback(mainwindow, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,5,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_ORIENTATION,Pt_GROUP_VERTICAL,0);
	maingroup=PtCreateWidget(PtGroup,mainwindow,n,args);
	pretty_group(maingroup,_(AP,DLG_MergeCells_Frame ));

	n=0;
PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MergeCells_Left),0);
	PtCreateWidget(PtLabel,maingroup,n,args);

	n=0;
PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MergeCells_Right),0);
	PtCreateWidget(PtLabel,maingroup,n,args);

	n=0;
PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MergeCells_Above),0);
	PtCreateWidget(PtLabel,maingroup,n,args);

	n=0;
PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MergeCells_Below),0);
	PtCreateWidget(PtLabel,maingroup,n,args);

	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,"Close",0);
	btn=PtCreateWidget(PtButton,maingroup,n,args);
	PtAddCallback(btn,Pt_CB_ACTIVATE,s_delete_clicked,this);

	n=0;
	PtSetArg(&args[n++],Pt_ARG_LABEL_TYPE,Pt_IMAGE,0);
	btn=PtCreateWidget(PtButton,maingroup,n,args);
	m_MergeLeft=btn;
	label_button_with_abi_pixmap(btn,"tb_MergeLeft_xpm");
	PtAddCallback(btn,Pt_CB_ACTIVATE,s_merge_left,this);

	n=0;
	PtSetArg(&args[n++],Pt_ARG_LABEL_TYPE,Pt_IMAGE,0);
	btn=PtCreateWidget(PtButton,maingroup,n,args);
	m_MergeRight=btn;
	label_button_with_abi_pixmap(btn,"tb_MergeRight_xpm");
	PtAddCallback(btn,Pt_CB_ACTIVATE,s_merge_right,this);
	
		n=0;
	PtSetArg(&args[n++],Pt_ARG_LABEL_TYPE,Pt_IMAGE,0);
	btn=PtCreateWidget(PtButton,maingroup,n,args);
	m_MergeAbove = btn;
	label_button_with_abi_pixmap(btn,"tb_MergeAbove_xpm");
	PtAddCallback(btn,Pt_CB_ACTIVATE,s_merge_above,this);
	
		n=0;
	PtSetArg(&args[n++],Pt_ARG_LABEL_TYPE,Pt_IMAGE,0);
	btn=PtCreateWidget(PtButton,maingroup,n,args);
	m_MergeBelow=btn;
	label_button_with_abi_pixmap(btn,"tb_MergeBelow_xpm");
	PtAddCallback(btn,Pt_CB_ACTIVATE,s_merge_below,this);

m_windowMain = mainwindow;

return mainwindow;
}


