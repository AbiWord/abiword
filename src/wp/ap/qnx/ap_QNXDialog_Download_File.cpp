/* AbiWord
 * Copyright (C) 2002 Gabriel Gerhardsson
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
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "xap_Dialog_Id.h"
#include "ap_QNXDialog_Download_File.h"
#include "ap_Strings.h"

#include "ut_bytebuf.h"
#include "ut_png.h"
#include "ut_worker.h"
#include "ut_qnxHelper.h"


XAP_Dialog * AP_QNXDialog_Download_File::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_Download_File * p = new AP_QNXDialog_Download_File(pFactory,id);
	return (XAP_Dialog*)p;
}

AP_QNXDialog_Download_File::AP_QNXDialog_Download_File(XAP_DialogFactory * pDlgFactory,
											 XAP_Dialog_Id id)
	: AP_Dialog_Download_File(pDlgFactory,id)
{
}

AP_QNXDialog_Download_File::~AP_QNXDialog_Download_File(void)
{
}


void AP_QNXDialog_Download_File::_runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	PtWidget_t *mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);
	connectFocus(mainWindow,pFrame);
	
   // To center the dialog, we need the frame of its parent.
    XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
    UT_ASSERT(pQNXFrame);
    
    // Get the Window of the parent frame
    PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
    UT_ASSERT(parentWindow);
    
    // Center our new dialog in its parent and make it a transient
    // so it won't get lost underneath
    // Make it modal, and stick it up top
	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);

    // Show the top level dialog,
	PtRealizeWidget(mainWindow);

    // Run the event loop for this window.
	int count;
	count = PtModalStart();
	done=0;
	do {
    		PtProcessEvent();
	} while (!done);

	PtModalEnd(MODAL_END_ARG(count));
	UT_QNXBlockWidget(parentWindow,0);
	PtDestroyWidget(mainWindow);

}

void
AP_QNXDialog_Download_File::_abortDialog(void)
{
done=1;

}

void AP_QNXDialog_Download_File::event_Cancel(void)
{
	_setUserAnswer(a_CANCEL);
	done=1;	
}

int ph_event_cancel(PtWidget_t *w,AP_QNXDialog_Download_File *data,PtCallbackInfo_t *cbinfo)
{
data->event_Cancel();
return Pt_CONTINUE;
}

PtWidget_t * AP_QNXDialog_Download_File::_constructWindow(void)
{
	PtWidget_t *mainwindow;
	PtWidget_t *buttonCancel;
	PtWidget_t *group;
	PtArg_t args[10];
	int n=0;
	char buf[4096];

	const XAP_StringSet *pSS = m_pApp->getStringSet();
	
	PtSetArg(&args[n++],Pt_ARG_WINDOW_RENDER_FLAGS,0,ABI_MODAL_WINDOW_RENDER_FLAGS);
	PtSetArg(&args[n++],Pt_ARG_WINDOW_MANAGED_FLAGS,0,ABI_MODAL_WINDOW_MANAGE_FLAGS);
	PtSetArg(&args[n++],Pt_ARG_WINDOW_TITLE,getTitle()),0);
	PtSetArg(&args[n++],Pt_ARG_RESIZE_FLAGS,Pt_RESIZE_XY_AS_REQUIRED,Pt_RESIZE_XY_AS_REQUIRED);
	mainwindow=PtCreateWidget(PtWindow,0,n,args);

	n=0;
	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,2,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_ORIENTATION,Pt_GROUP_VERTICAL,0);
	group = PtCreateWidget(PtGroup,mainwindow,n,args);

	n=0;

	sprintf(buf,pSS->getValue(AP_STRING_ID_DLG_DlFile_Status),getDescription(),getURL());
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,&buf,0);
	PtSetArg(&args[n++],Pt_ARG_RESIZE_FLAGS,Pt_RESIZE_XY_AS_REQUIRED,Pt_RESIZE_XY_AS_REQUIRED);
	PtCreateWidget(PtLabel,group,n,args);

	n=0;

	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,pSS->getValue(XAP_STRING_ID_DLG_Cancel),0);
	buttonCancel = PtCreateWidget(PtButton,group,n,args);
	
	PtAddCallback(buttonCancel,Pt_CB_ACTIVATE,ph_event_cancel,this);

return mainwindow;
}
