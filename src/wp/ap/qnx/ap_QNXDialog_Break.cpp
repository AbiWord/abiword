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
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

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
	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
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
	PtWidget_t * buttonOK;
	PtWidget_t * buttonCancel;
	int 	   bmi, n;
	PhRect_t	zero;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	bmi=0;
	windowBreak = abiCreatePhabDialog("ap_QNXDialog_Break",pSS,AP_STRING_ID_DLG_Break_BreakTitle); 
	PtAddHotkeyHandler(windowBreak,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	SetupContextHelp(windowBreak,this);
	PtAddCallback(windowBreak, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	localizeLabel(abiPhabLocateWidget(windowBreak,"grpInsert"),pSS,AP_STRING_ID_DLG_Break_Insert);

	bm[bmi].widget = abiPhabLocateWidget(windowBreak,"togglePageBreak");
	localizeLabel(bm[bmi].widget, pSS, AP_STRING_ID_DLG_Break_PageBreak);

	PtAddCallback(bm[bmi].widget, Pt_CB_ACTIVATE, s_radio_clicked, this);
	bm[bmi++].type = AP_Dialog_Break::b_PAGE;

	bm[bmi].widget = abiPhabLocateWidget(windowBreak,"toggleColumnBreak");
	localizeLabel(bm[bmi].widget, pSS, AP_STRING_ID_DLG_Break_ColumnBreak );
	PtAddCallback(bm[bmi].widget, Pt_CB_ACTIVATE, s_radio_clicked, this);
	bm[bmi++].type = AP_Dialog_Break::b_COLUMN;

	localizeLabel(abiPhabLocateWidget(windowBreak,"grpSectionBreak"),pSS,AP_STRING_ID_DLG_Break_SectionBreaks);

	bm[bmi].widget = abiPhabLocateWidget(windowBreak,"toggleNextPage");
	localizeLabel(bm[bmi].widget, pSS, AP_STRING_ID_DLG_Break_NextPage );
	PtAddCallback(bm[bmi].widget, Pt_CB_ACTIVATE, s_radio_clicked, this);
	bm[bmi++].type = AP_Dialog_Break::b_NEXTPAGE;

	bm[bmi].widget = abiPhabLocateWidget(windowBreak,"toggleEvenPage");
	localizeLabel(bm[bmi].widget, pSS, AP_STRING_ID_DLG_Break_EvenPage );
	PtAddCallback(bm[bmi].widget, Pt_CB_ACTIVATE, s_radio_clicked, this);
	bm[bmi++].type = AP_Dialog_Break::b_EVENPAGE;

	bm[bmi].widget = abiPhabLocateWidget(windowBreak,"toggleContinuous");
	localizeLabel(bm[bmi].widget, pSS, AP_STRING_ID_DLG_Break_Continuous );
	PtAddCallback(bm[bmi].widget, Pt_CB_ACTIVATE, s_radio_clicked, this);
	bm[bmi++].type = AP_Dialog_Break::b_CONTINUOUS;

	bm[bmi].widget = abiPhabLocateWidget(windowBreak,"toggleOddPage");
	localizeLabel(bm[bmi].widget, pSS, AP_STRING_ID_DLG_Break_OddPage );
	PtAddCallback(bm[bmi].widget, Pt_CB_ACTIVATE, s_radio_clicked, this);
	bm[bmi++].type = AP_Dialog_Break::b_ODDPAGE;

	buttonCancel = abiPhabLocateWidget(windowBreak,"btnCancel");
	localizeLabel(buttonCancel, pSS, XAP_STRING_ID_DLG_Cancel);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	buttonOK = abiPhabLocateWidget(windowBreak,"btnOK");
	localizeLabel(buttonOK, pSS, XAP_STRING_ID_DLG_OK);
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

