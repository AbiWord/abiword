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
#include <stdio.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Break.h"
#include "ap_QNXDialog_Break.h"
#include "ut_qnxHelper.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Break::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_QNXDialog_Break * p = new AP_QNXDialog_Break(pFactory,id);
	return p;
}

AP_QNXDialog_Break::AP_QNXDialog_Break(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Break(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_buttonOK = NULL;
	m_buttonCancel = NULL;

	m_radioGroup = NULL;
}

AP_QNXDialog_Break::~AP_QNXDialog_Break(void)
{
}

/*****************************************************************/
static int s_ok_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Break *dlg = (AP_QNXDialog_Break *)data;
	dlg->event_OK();
	return Pt_CONTINUE;
}

static int s_cancel_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Break *dlg = (AP_QNXDialog_Break *)data;
	dlg->event_Cancel();
	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Break *dlg = (AP_QNXDialog_Break *)data;
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}

static int s_radio_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Break *dlg = (AP_QNXDialog_Break *)data;
	dlg->event_RadioSelected(w);
	return Pt_CONTINUE;
}

/*****************************************************************/

void AP_QNXDialog_Break::runModal(XAP_Frame * pFrame)
{
	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the GtkWindow of the parent frame
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	PtSetParentWidget(parentWindow);

	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();

		
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

	_storeWindowData();

	UT_QNXBlockWidget(parentWindow, 0);
	PtDestroyWidget(mainWindow);
}

void AP_QNXDialog_Break::event_RadioSelected(PtWidget_t *w) {
	int i;
	for (i = 0; i < BREAK_COUNT; i++) {
		if (bm[i].widget != w) {
			PtSetResource(bm[i].widget, Pt_ARG_FLAGS, 0, Pt_SET);
		}
	}
	
}

void AP_QNXDialog_Break::event_OK(void)
{
	// TODO save out state of radio items
	if (!done++) {
		m_answer = AP_Dialog_Break::a_OK;
	}
}

void AP_QNXDialog_Break::event_Cancel(void)
{
	if (!done++) {
		m_answer = AP_Dialog_Break::a_CANCEL;
	}
}

void AP_QNXDialog_Break::event_WindowDelete(void)
{
	if (!done++) {
		m_answer = AP_Dialog_Break::a_CANCEL;	
	}
}

/*****************************************************************/
PtWidget_t * AP_QNXDialog_Break::_constructWindow(void)
{

	PtWidget_t * windowBreak;
	PtWidget_t * vgroup, * hgroup;
	PtWidget_t * radiobuttonPageBreak;
	PtWidget_t * radiobuttonNextPage;
	PtWidget_t * radiobuttonContinuous;
	PtWidget_t * radiobuttonColumnBreak;
	PtWidget_t * radiobuttonEvenPage;
	PtWidget_t * radiobuttonOddPage;
	PtWidget_t * buttonOK;
	PtWidget_t * buttonCancel;
	PtArg_t	   args[10];
	int 	   bmi, n;
	PhRect_t	zero;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

#define _ANCHOR_WIDTH (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT)
#define _ANCHOR_ALL (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT |\
				     Pt_TOP_ANCHORED_TOP | Pt_BOTTOM_ANCHORED_BOTTOM)
	memset(&zero, 0, sizeof(zero));

	n = bmi = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, 
	    UT_XML_transNoAmpersands(pSS->getValue(AP_STRING_ID_DLG_Break_BreakTitle)), 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WIDTH, 200, 0);
	windowBreak = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowBreak, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 5, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_STRETCH_HORIZONTAL, Pt_GROUP_STRETCH_HORIZONTAL);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EXCLUSIVE, Pt_GROUP_EXCLUSIVE);
	PtSetArg(&args[n++], Pt_ARG_RESIZE_FLAGS, Pt_RESIZE_XY_AS_REQUIRED, 
											  Pt_RESIZE_XY_AS_REQUIRED | Pt_RESIZE_XY_ALWAYS);
    PtSetArg(&args[n++], Pt_ARG_WIDTH, 200, 0);
	vgroup = PtCreateWidget(PtGroup, windowBreak, n, args);

	/* First group ... "Insert" */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ROWS_COLS, 2, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EQUAL_SIZE_HORIZONTAL, Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);	
	pretty_group(hgroup, UT_XML_transNoAmpersands(pSS->getValue(AP_STRING_ID_DLG_Break_Insert)));

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, 
		UT_XML_transNoAmpersands(pSS->getValue(AP_STRING_ID_DLG_Break_PageBreak)), 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_SET, Pt_SET);
	bm[bmi].widget =
	radiobuttonPageBreak = PtCreateWidget(PtToggleButton, hgroup, n, args);
	PtAddCallback(radiobuttonPageBreak, Pt_CB_ACTIVATE, s_radio_clicked, this);
	bm[bmi++].type = AP_Dialog_Break::b_PAGE;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, 
		UT_XML_transNoAmpersands(pSS->getValue(AP_STRING_ID_DLG_Break_ColumnBreak)), 0);	
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	bm[bmi].widget = 
	radiobuttonColumnBreak = PtCreateWidget(PtToggleButton, hgroup, n, args);
	PtAddCallback(radiobuttonColumnBreak, Pt_CB_ACTIVATE, s_radio_clicked, this);
	bm[bmi++].type = AP_Dialog_Break::b_COLUMN;

	/* Second group ... "Section Breaks" */
#if 0
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ROWS_COLS, 2, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EQUAL_SIZE_HORIZONTAL, Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EXCLUSIVE, Pt_GROUP_EXCLUSIVE);
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);
	pretty_group(hgroup, UT_XML_transNoAmpersands(pSS->getValue(AP_STRING_ID_DLG_Break_SectionBreaks)));
#else
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, UT_XML_transNoAmpersands(pSS->getValue(AP_STRING_ID_DLG_Break_SectionBreaks)), 0);
	PtCreateWidget(PtLabel, hgroup, n, args);
	n = 0;
	PtCreateWidget(PtLabel, hgroup, n, args);
#endif

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, 
		UT_XML_transNoAmpersands(pSS->getValue(AP_STRING_ID_DLG_Break_NextPage)), 0);	
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	bm[bmi].widget = 
	radiobuttonNextPage = PtCreateWidget(PtToggleButton, hgroup, n, args);
	PtAddCallback(radiobuttonNextPage, Pt_CB_ACTIVATE, s_radio_clicked, this);
	bm[bmi++].type = AP_Dialog_Break::b_NEXTPAGE;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, 
		UT_XML_transNoAmpersands(pSS->getValue(AP_STRING_ID_DLG_Break_EvenPage)), 0);	
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	bm[bmi].widget = 
	radiobuttonEvenPage = PtCreateWidget(PtToggleButton, hgroup, n, args);
	PtAddCallback(radiobuttonEvenPage, Pt_CB_ACTIVATE, s_radio_clicked, this);
	bm[bmi++].type = AP_Dialog_Break::b_EVENPAGE;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, 
		UT_XML_transNoAmpersands(pSS->getValue(AP_STRING_ID_DLG_Break_Continuous)), 0);	
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	bm[bmi].widget = 
	radiobuttonContinuous = PtCreateWidget(PtToggleButton, hgroup, n, args);
	PtAddCallback(radiobuttonContinuous, Pt_CB_ACTIVATE, s_radio_clicked, this);
	bm[bmi++].type = AP_Dialog_Break::b_CONTINUOUS;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, 
		UT_XML_transNoAmpersands(pSS->getValue(AP_STRING_ID_DLG_Break_OddPage)), 0);	
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	bm[bmi].widget = 
	radiobuttonOddPage = PtCreateWidget(PtToggleButton, hgroup, n, args);
	PtAddCallback(radiobuttonOddPage, Pt_CB_ACTIVATE, s_radio_clicked, this);
	bm[bmi++].type = AP_Dialog_Break::b_ODDPAGE;

	/* Bottom row of buttons */
	n = 0;
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtCreateWidget(PtLabel, hgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonCancel = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonOK = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	m_windowMain = windowBreak;
	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;
	
	return windowBreak;
}

void AP_QNXDialog_Break::_populateWindowData(void)
{
}

void AP_QNXDialog_Break::_storeWindowData(void)
{
	m_break = _getActiveRadioItem();
}

AP_Dialog_Break::breakType AP_QNXDialog_Break::_getActiveRadioItem(void)
{
	int		*value;

	for (int i = 0; i < BREAK_COUNT; i++) {
		value = NULL;
		
		PtGetResource(bm[i].widget, Pt_ARG_FLAGS, &value, 0);
		if (value && *value & Pt_SET) {
			return (AP_Dialog_Break::breakType)(bm[i].type);
		}
	}
	m_answer = AP_Dialog_Break::a_CANCEL;
	return AP_Dialog_Break::b_PAGE;
}

