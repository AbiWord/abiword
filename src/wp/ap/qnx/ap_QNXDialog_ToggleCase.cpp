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
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_ToggleCase.h"
#include "ap_QNXDialog_ToggleCase.h"
#include "ut_qnxHelper.h"

/*****************************************************************/

static ToggleCase toggle_entries[] = {	
		CASE_SENTENCE, 
		CASE_LOWER, 
		CASE_UPPER, 
		CASE_TITLE, 
		CASE_TOGGLE,
};

static int s_toggled (PtWidget_t *w, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_ToggleCase* dlg = (AP_QNXDialog_ToggleCase *)data;
	ToggleCase tc;
	void *ptr;

	ptr = NULL;
	PtGetResource(w, Pt_ARG_POINTER, &ptr, 0);
	if(ptr) {
		tc = *((ToggleCase *)ptr);
	}

	dlg->setCase(tc);

	return Pt_CONTINUE;
}

static int s_ok_clicked (PtWidget_t* w, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_ToggleCase* dlg = (AP_QNXDialog_ToggleCase *)data;
	dlg->setFinalAnswer(AP_Dialog_ToggleCase::a_OK);

	return Pt_CONTINUE;
}

static int s_cancel_clicked(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_ToggleCase* dlg = (AP_QNXDialog_ToggleCase *)data;
	dlg->setFinalAnswer(AP_Dialog_ToggleCase::a_CANCEL);

	return Pt_CONTINUE;
}


/*****************************************************************/

XAP_Dialog * AP_QNXDialog_ToggleCase::static_constructor(XAP_DialogFactory * pFactory,
													     XAP_Dialog_Id id)
{
	AP_QNXDialog_ToggleCase * p = new AP_QNXDialog_ToggleCase(pFactory,id);
	return p;
}

AP_QNXDialog_ToggleCase::AP_QNXDialog_ToggleCase(XAP_DialogFactory * pDlgFactory,
											     XAP_Dialog_Id id)
	: AP_Dialog_ToggleCase(pDlgFactory,id)
{
}

AP_QNXDialog_ToggleCase::~AP_QNXDialog_ToggleCase(void)
{
}

void AP_QNXDialog_ToggleCase::runModal(XAP_Frame * pFrame)
{
	// get top level window 
	XAP_QNXFrame * frame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(frame);

	PtWidget_t *parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);
	PtSetParentWidget(parent);

	// build the dialog
	PtWidget_t * window = _constructWindow();
	UT_ASSERT(window);
	connectFocus(window, pFrame);

	UT_QNXCenterWindow(parent, window);
	UT_QNXBlockWidget(parent, 1);
	
	// Run the dialog
	PtRealizeWidget(window);

	int count;
	count = PtModalStart();
	done = 0;
	while(!done) {
		PtProcessEvent();
	}
	PtModalEnd(MODAL_END_ARG(count));

	UT_QNXBlockWidget(parent, 0);
	PtDestroyWidget(window);
}

static ToggleCase *find_entry(ToggleCase match) {
	int i;
	for(i=0; i< sizeof(toggle_entries) / sizeof(*toggle_entries); i++) {
		if(toggle_entries[i] == match) {
			return &toggle_entries[i];
		}
	}
	return NULL;
}

PtWidget_t * AP_QNXDialog_ToggleCase::_constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	PtWidget_t *window, *widget;
	PtWidget_t *vboxMain;
	PtWidget_t *vboxOuter;

	PtWidget_t *hboxBut;
	PtWidget_t *buttonOK;
	PtWidget_t *buttonCancel;

	PtArg_t args[10];
	int n;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(AP_STRING_ID_DLG_ToggleCase_Title), 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	window = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(window, Pt_CB_WINDOW_CLOSING, s_cancel_clicked, this);

#define MARGIN_SIZE 10 

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, MARGIN_SIZE, 0); 
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, MARGIN_SIZE, 0); 
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, MARGIN_SIZE, 0); 
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
					Pt_GROUP_EQUAL_SIZE_HORIZONTAL,
					Pt_GROUP_EQUAL_SIZE_HORIZONTAL); 
	vboxOuter = PtCreateWidget(PtGroup, window, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
				Pt_GROUP_STRETCH_VERTICAL | Pt_GROUP_STRETCH_HORIZONTAL | Pt_GROUP_EXCLUSIVE,
				Pt_GROUP_STRETCH_VERTICAL | Pt_GROUP_STRETCH_HORIZONTAL | Pt_GROUP_EXCLUSIVE);
	vboxMain =  PtCreateWidget(PtGroup, vboxOuter, n, args);
  	pretty_group(vboxMain, pSS->getValue(AP_STRING_ID_DLG_ToggleCase_Title));

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_ToggleCase_SentenceCase), 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_POINTER, find_entry(CASE_SENTENCE), 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_SET, Pt_SET);
	widget = PtCreateWidget(PtToggleButton, vboxMain, n, args);
	PtAddCallback(widget, Pt_CB_ACTIVATE, s_toggled, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_ToggleCase_LowerCase), 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_POINTER, find_entry(CASE_LOWER), 0);
	widget = PtCreateWidget(PtToggleButton, vboxMain, n, args);
	PtAddCallback(widget, Pt_CB_ACTIVATE, s_toggled, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_ToggleCase_UpperCase), 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_POINTER, find_entry(CASE_UPPER), 0);
	widget = PtCreateWidget(PtToggleButton, vboxMain, n, args);
	PtAddCallback(widget, Pt_CB_ACTIVATE, s_toggled, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_ToggleCase_TitleCase), 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_POINTER, find_entry(CASE_TITLE), 0);
	widget = PtCreateWidget(PtToggleButton, vboxMain, n, args);
	PtAddCallback(widget, Pt_CB_ACTIVATE, s_toggled, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_ToggleCase_ToggleCase), 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_POINTER, find_entry(CASE_TOGGLE), 0);
	widget = PtCreateWidget(PtToggleButton, vboxMain, n, args);
	PtAddCallback(widget, Pt_CB_ACTIVATE, s_toggled, this);

	/* Buttons along the bottom */
	n = 0;
	hboxBut =  PtCreateWidget(PtGroup, vboxOuter, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING,  pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	buttonOK = PtCreateWidget(PtButton, hboxBut, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING,  pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	buttonCancel = PtCreateWidget(PtButton, hboxBut, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	return window;
}

void AP_QNXDialog_ToggleCase::setFinalAnswer(AP_Dialog_ToggleCase::tAnswer ans) {
	if(!done++) {
		setAnswer(ans);
	}
}
	
