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
#include <stdio.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Field.h"
#include "ap_QNXDialog_Field.h"
#include "ap_QNXDialog_InsertTable.h"
#include "ut_qnxHelper.h"

#include "ap_Strings.h"

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_InsertTable::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_InsertTable * p = new AP_QNXDialog_InsertTable(pFactory,id);
	return p;
}

AP_QNXDialog_InsertTable::AP_QNXDialog_InsertTable(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_InsertTable(pDlgFactory,id)
{
}

AP_QNXDialog_InsertTable::~AP_QNXDialog_InsertTable(void)
{
}

void AP_QNXDialog_InsertTable::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	PtWidget_t *mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);
	connectFocus(mainWindow,pFrame);
	
   // To center the dialog, we need the frame of its parent.
 	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
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

int ph_event_close(PtWidget_t *widget,AP_QNXDialog_InsertTable *dialog,PtCallbackInfo_t *cbinfo)
{
dialog->done=1;
return Pt_CONTINUE;
}

int ph_event_ok(PtWidget_t *widget,AP_QNXDialog_InsertTable *dialog,PtCallbackInfo_t *cbinfo)
{
dialog->event_OK();
return Pt_CONTINUE;
}

int ph_event_cancel(PtWidget_t *widget,AP_QNXDialog_InsertTable *dialog,PtCallbackInfo_t *cbinfo)
{
dialog->event_Cancel();
return Pt_CONTINUE;
}

void AP_QNXDialog_InsertTable::event_OK()
{
int *num;

PtGetResource(m_widgetNumCol,Pt_ARG_NUMERIC_VALUE,&num,0);
m_numCols=*num;
PtGetResource(m_widgetNumRow,Pt_ARG_NUMERIC_VALUE,&num,0);
m_numRows=*num;
//Add the radio buttons.
m_columnType = AP_Dialog_InsertTable::b_AUTOSIZE;

done=1;
m_answer = AP_Dialog_InsertTable::a_OK;
}

void AP_QNXDialog_InsertTable::event_Cancel()
{
done=1;
m_answer = AP_Dialog_InsertTable::a_CANCEL;
}

PtWidget_t * AP_QNXDialog_InsertTable::_constructWindow(void)
{
PtWidget_t *mainwindow;
PtWidget_t *num_col,*num_row;
	PtWidget_t *toggle_auto;
	PtWidget_t *toggle_fixed;
	PtWidget_t *fixed_size_value;
PtWidget_t *group_btn;
	PtWidget_t *btn_ok;
	PtWidget_t *btn_cancel;
PtWidget_t *group_main;

PtArg_t args[10];
int n=0;
const XAP_StringSet *pSS = m_pApp->getStringSet();	


PtSetArg(&args[n++],Pt_ARG_WINDOW_TITLE,_(AP,DLG_InsertTable_TableTitle),0);
	PtSetArg(&args[n++],Pt_ARG_WINDOW_RENDER_FLAGS,0,ABI_MODAL_WINDOW_RENDER_FLAGS);
	PtSetArg(&args[n++],Pt_ARG_WINDOW_MANAGED_FLAGS,0,ABI_MODAL_WINDOW_MANAGE_FLAGS);
	PtSetArg(&args[n++],Pt_ARG_RESIZE_FLAGS,Pt_RESIZE_XY_AS_REQUIRED,Pt_RESIZE_XY_AS_REQUIRED);

	mainwindow= PtCreateWidget(PtWindow,0,n,args);

	n=0;

	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,6,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_ORIENTATION,Pt_GROUP_VERTICAL,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_FLAGS,Pt_GROUP_EXCLUSIVE,Pt_TRUE);
	group_main = PtCreateWidget(PtGroup,mainwindow,n,args);
	pretty_group(group_main,_(AP,DLG_InsertTable_TableSize));

	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_InsertTable_NumCols),0);
	PtCreateWidget(PtLabel,Pt_DEFAULT_PARENT,n,args);

	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_InsertTable_NumRows),0);
	PtCreateWidget(PtLabel,Pt_DEFAULT_PARENT,n,args);	
	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_InsertTable_AutoFit),0);
	PtCreateWidget(PtLabel,Pt_DEFAULT_PARENT,n,args);	
	
	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_InsertTable_AutoColSize),0);
	PtSetArg(&args[n++],Pt_ARG_INDICATOR_TYPE,Pt_TOGGLE_RADIO,0);
	toggle_auto = PtCreateWidget(PtToggleButton,Pt_DEFAULT_PARENT,n,args);

	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_InsertTable_FixedColSize),0);
	PtSetArg(&args[n++],Pt_ARG_INDICATOR_TYPE,Pt_TOGGLE_RADIO,0);
	toggle_fixed = PtCreateWidget(PtToggleButton,Pt_DEFAULT_PARENT,n,args);
	n=0;
	//Space fill
	PtCreateWidget(PtLabel,Pt_DEFAULT_PARENT,n,args);
	
	n=0;
	PtSetArg(&args[n++],Pt_ARG_NUMERIC_MIN,1,0);
	PtSetArg(&args[n++],Pt_ARG_NUMERIC_MAX,9999,0);
	PtSetArg(&args[n++],Pt_ARG_NUMERIC_VALUE,getNumCols(),0);
	num_col=PtCreateWidget(PtNumericInteger,Pt_DEFAULT_PARENT,n,args);
	n=0;

	n=0;
	PtSetArg(&args[n++],Pt_ARG_NUMERIC_MIN,1,0);
	PtSetArg(&args[n++],Pt_ARG_NUMERIC_MAX,9999,0);
	PtSetArg(&args[n++],Pt_ARG_NUMERIC_VALUE,getNumRows(),0);
	num_row=PtCreateWidget(PtNumericInteger,Pt_DEFAULT_PARENT,n,args);

	n=0; //space filler.
	PtCreateWidget(PtLabel,Pt_DEFAULT_PARENT,n,args);
	PtCreateWidget(PtLabel,Pt_DEFAULT_PARENT,n,args);

	n=0;
	double min=0;
	PtSetArg(&args[n++],Pt_ARG_NUMERIC_MIN,&min,0);
	double max=100000; //just for sizing the widget correctly.
	PtSetArg(&args[n++],Pt_ARG_NUMERIC_MAX,&max,0);
	fixed_size_value = PtCreateWidget(PtNumericFloat,Pt_DEFAULT_PARENT,n,args);

	n=0;
	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,2,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_ORIENTATION,Pt_GROUP_HORIZONTAL,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_HORZ_ALIGN,Pt_GROUP_HORZ_RIGHT,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_FLAGS,Pt_TRUE,Pt_GROUP_EQUAL_SIZE);
	group_btn = PtCreateWidget(PtGroup,Pt_DEFAULT_PARENT,n,args);

	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(XAP,DLG_OK),0);
	btn_ok = PtCreateWidget(PtButton,group_btn,n,args);

	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(XAP,DLG_Cancel),0);
	btn_cancel = PtCreateWidget(PtButton,group_btn,n,args);

	PtAddCallback(btn_ok,Pt_CB_ACTIVATE,ph_event_ok,this);
	PtAddCallback(btn_cancel,Pt_CB_ACTIVATE,ph_event_cancel,this);	
	PtAddCallback(mainwindow,Pt_CB_WINDOW_CLOSING,ph_event_close,this);


	m_widgetNumCol=num_col;
	m_widgetNumRow=num_row;

return mainwindow;
}

