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

if(PtWidgetFlags(m_widgetToggleFixed) & Pt_SET) { 
	m_columnType = AP_Dialog_InsertTable::b_FIXEDSIZE;
	double *val;
	PtGetResource(m_widgetFixedSize,Pt_ARG_NUMERIC_VALUE,&val,0);
	m_columnWidth = *val;
}
else m_columnType = AP_Dialog_InsertTable::b_AUTOSIZE;

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
	PtWidget_t *toggle_auto;
	PtWidget_t *toggle_fixed;
	PtWidget_t *fixed_size_value;
	PtWidget_t *btn_ok;
	PtWidget_t *btn_cancel;

PtArg_t args[10];
int n=0;
const XAP_StringSet *pSS = m_pApp->getStringSet();	


PtSetArg(&args[n++],Pt_ARG_WINDOW_TITLE,_(AP,DLG_InsertTable_TableTitle),0);
	mainwindow= abiCreatePhabDialog("ap_QNXDialog_InsertTable",_(AP,DLG_InsertTable_TableTitle)); 
	SetupContextHelp(mainwindow,this);
	PtAddHotkeyHandler(mainwindow,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);


	PtSetResource(abiPhabLocateWidget(mainwindow,"grpTableSize"),Pt_ARG_TITLE,_(AP,DLG_InsertTable_TableSize),0);

	PtSetResource(abiPhabLocateWidget(mainwindow,"lblNumCols"),Pt_ARG_TEXT_STRING,_(AP,DLG_InsertTable_NumCols),0);

	PtSetResource(abiPhabLocateWidget(mainwindow,"lblNumRows"),Pt_ARG_TEXT_STRING,_(AP,DLG_InsertTable_NumRows),0);

	PtSetResource(abiPhabLocateWidget(mainwindow,"grpAutoFit"),Pt_ARG_TITLE,_(AP,DLG_InsertTable_AutoFit),0);

	toggle_auto = abiPhabLocateWidget(mainwindow,"toggleAutoColSize");
	PtSetResource(toggle_auto,Pt_ARG_TEXT_STRING,_(AP,DLG_InsertTable_AutoColSize),0);
 
	toggle_fixed = abiPhabLocateWidget(mainwindow,"toggleFixed");
	PtSetResource(toggle_fixed,Pt_ARG_TEXT_STRING,_(AP,DLG_InsertTable_FixedColSize),0);

	m_widgetNumCol= abiPhabLocateWidget(mainwindow,"NumericCols");
	PtSetResource(m_widgetNumCol,Pt_ARG_NUMERIC_VALUE,getNumCols(),0);

	m_widgetNumRow = abiPhabLocateWidget(mainwindow,"NumericRows");
	PtSetResource(m_widgetNumRow,Pt_ARG_NUMERIC_VALUE,getNumRows(),0);

	fixed_size_value = abiPhabLocateWidget(mainwindow,"NumericFloatFixedValue"); 
	double val= getColumnWidth();
	PtSetResource(fixed_size_value,Pt_ARG_NUMERIC_VALUE,&val,0);

	btn_ok = abiPhabLocateWidget(mainwindow,"btnOK"); 
	PtSetResource(btn_ok,Pt_ARG_TEXT_STRING,_(XAP,DLG_OK),0);

	btn_cancel = abiPhabLocateWidget(mainwindow,"btnCancel");
	PtSetResource(btn_cancel,Pt_ARG_TEXT_STRING,_(XAP,DLG_Cancel),0);

	PtAddCallback(btn_ok,Pt_CB_ACTIVATE,ph_event_ok,this);
	PtAddCallback(btn_cancel,Pt_CB_ACTIVATE,ph_event_cancel,this);	
	PtAddCallback(mainwindow,Pt_CB_WINDOW_CLOSING,ph_event_close,this);


	m_widgetFixedSize = fixed_size_value;
	m_widgetToggleFixed = toggle_fixed;

return mainwindow;
}

