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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Columns.h"
#include "ap_QNXDialog_Columns.h"
#include "ut_qnxHelper.h"


/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Columns::static_constructor(XAP_DialogFactory * pFactory,
						   XAP_Dialog_Id id)
{
	AP_QNXDialog_Columns * p = new AP_QNXDialog_Columns(pFactory,id);
	return p;
}

AP_QNXDialog_Columns::AP_QNXDialog_Columns(XAP_DialogFactory * pDlgFactory,
					 XAP_Dialog_Id id)
	: AP_Dialog_Columns(pDlgFactory,id)
{
	m_windowMain = m_wlineBetween = NULL;
	m_buttonOK = m_buttonCancel = NULL;
	m_wtoggleOne = m_wtoggleTwo = m_wtoggleThree = NULL;
	m_pPreviewWidget = NULL;
}

AP_QNXDialog_Columns::~AP_QNXDialog_Columns(void)
{
	DELETEP(m_pPreviewWidget);
}

/*****************************************************************/

static int s_ok_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Columns * dlg = (AP_QNXDialog_Columns *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
	return Pt_CONTINUE;
}

static int s_cancel_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Columns * dlg = (AP_QNXDialog_Columns *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Columns * dlg = (AP_QNXDialog_Columns *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}

static int s_toggle_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Columns * dlg = (AP_QNXDialog_Columns *)data;
	UT_ASSERT(dlg);
	dlg->event_Toggle(widget);
	return Pt_CONTINUE;
}

static int s_preview_exposed(PtWidget_t * w, PhTile_t * damage) 
{
   	PhRect_t rect;
   	PtSuperClassDraw(PtBasic, w, damage);
   	PtBasicWidgetCanvas(w, &rect);
	//clip to our basic canvas (it's only polite).
    PtClipAdd( w, &rect );

	AP_QNXDialog_Columns *pQNXDlg;
	PtGetResource(w, Pt_ARG_POINTER, &pQNXDlg,0);

	UT_ASSERT(pQNXDlg);
	pQNXDlg->event_previewExposed();

    PtClipRemove();
	return Pt_CONTINUE;
}

/*****************************************************************/

void AP_QNXDialog_Columns::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(mainWindow,pFrame);
	// Populate the window's data items
	_populateWindowData();
	
	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

	PtSetParentWidget(parentWindow);

	DELETEP (m_pPreviewWidget);
	//m_pPreviewWidget = new GR_QNXGraphics(mainWindow, m_wpreviewArea, m_pApp);
	GR_QNXAllocInfo ai(mainWindow, m_wpreviewArea, m_pApp);
	m_pPreviewWidget = (GR_QNXGraphics*) XAP_App::getApp()->newGraphics(ai);
	
	unsigned short *w, *h;
	w = h = NULL;
	PtGetResource(m_wpreviewArea, Pt_ARG_WIDTH, &w, 0);
	PtGetResource(m_wpreviewArea, Pt_ARG_HEIGHT, &h, 0);
	if (w && h) {
		_createPreviewFromGC(m_pPreviewWidget, (int)*w, (int)*h);
	}
	else {
		UT_ASSERT(0);
		m_answer = AP_Dialog_Columns::a_CANCEL;	
		return;
	}
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);

	// Show the top level dialog,
	PtRealizeWidget(mainWindow);
	
    UT_QNXBlockWidget(parentWindow, 0);

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

void AP_QNXDialog_Columns::event_Toggle( PtWidget_t *widget)
{
	int *flags = NULL;

	PtGetResource(m_wlineBetween, Pt_ARG_FLAGS, &flags, 0);
	if (flags == NULL) {
		setLineBetween(false);
	}
	else if (*flags & Pt_SET) {
		setLineBetween(true);
	}
	else {
		setLineBetween(false);
	}

	/* There has to be a better way to do this ... */
	if (widget == m_wtoggleOne) {
		//PtSetResource(m_wtoggleOne, Pt_ARG_FLAGS, Pt_SET, Pt_SET);
		PtSetResource(m_wtoggleTwo, Pt_ARG_FLAGS, 0, Pt_SET);
		PtSetResource(m_wtoggleThree, Pt_ARG_FLAGS, 0, Pt_SET);
		setColumns(1);
	}
	else if (widget == m_wtoggleTwo) {
		PtSetResource(m_wtoggleOne, Pt_ARG_FLAGS, 0, Pt_SET);
		//PtSetResource(m_wtoggleTwo, Pt_ARG_FLAGS, Pt_SET, Pt_SET);
		PtSetResource(m_wtoggleThree, Pt_ARG_FLAGS, 0, Pt_SET);
		setColumns(2);
	}
	else if (widget == m_wtoggleThree) {
		PtSetResource(m_wtoggleOne, Pt_ARG_FLAGS, 0, Pt_SET);
		PtSetResource(m_wtoggleTwo, Pt_ARG_FLAGS, 0, Pt_SET);
		//PtSetResource(m_wtoggleThree, Pt_ARG_FLAGS, Pt_SET, Pt_SET);
		setColumns(3);
	}

	m_pColumnsPreview->draw();
	PtDamageWidget(m_wpreviewArea);
}


void AP_QNXDialog_Columns::event_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_Columns::a_OK;
	done++;
}

void AP_QNXDialog_Columns::event_Cancel(void)
{
	m_answer = AP_Dialog_Columns::a_CANCEL;
	done++;
}

void AP_QNXDialog_Columns::event_WindowDelete(void)
{
	if (!done++) {
		m_answer = AP_Dialog_Columns::a_CANCEL;	
	}
}

void AP_QNXDialog_Columns::event_previewExposed(void)
{
        if(m_pColumnsPreview)
	       m_pColumnsPreview->draw();
}


/*****************************************************************/

PtWidget_t * AP_QNXDialog_Columns::_constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();


	m_windowMain = abiCreatePhabDialog("ap_QNXDialog_Columns",pSS,AP_STRING_ID_DLG_Column_ColumnTitle);
	SetupContextHelp(m_windowMain,this);
	PtAddHotkeyHandler(m_windowMain,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
  PtAddCallback(m_windowMain, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);
	
	localizeLabel(abiPhabLocateWidget(m_windowMain,"grpColumnNumber"),pSS, AP_STRING_ID_DLG_Column_Number );
	
	m_wtoggleOne = abiPhabLocateWidget(m_windowMain,"toggleOne"); 
	localizeLabel(abiPhabLocateWidget(m_windowMain,"lblOne"), pSS,AP_STRING_ID_DLG_Column_One);
	label_button_with_abi_pixmap(m_wtoggleOne, "tb_1column_xpm");
	PtAddCallback(m_wtoggleOne, Pt_CB_ACTIVATE, s_toggle_clicked, this);

	m_wtoggleTwo = abiPhabLocateWidget(m_windowMain,"toggleTwo"); 
	localizeLabel(abiPhabLocateWidget(m_windowMain,"lblTwo"), pSS,AP_STRING_ID_DLG_Column_Two);
	label_button_with_abi_pixmap(m_wtoggleTwo, "tb_2column_xpm");
	PtAddCallback(m_wtoggleTwo, Pt_CB_ACTIVATE, s_toggle_clicked, this);

	m_wtoggleThree = abiPhabLocateWidget(m_windowMain,"toggleThree"); 
	localizeLabel(abiPhabLocateWidget(m_windowMain,"lblThree"), pSS,AP_STRING_ID_DLG_Column_Three);
	label_button_with_abi_pixmap(m_wtoggleThree, "tb_3column_xpm");
	PtAddCallback(m_wtoggleThree, Pt_CB_ACTIVATE, s_toggle_clicked, this);

	localizeLabel(abiPhabLocateWidget(m_windowMain,"grpPreview"),pSS, AP_STRING_ID_DLG_Column_Preview );

	m_wpreviewArea = abiPhabLocateWidget(m_windowMain,"rawPreview");
	PtSetResource(m_wpreviewArea, Pt_ARG_POINTER,this,0); 
	PtSetResource(m_wpreviewArea, Pt_ARG_RAW_DRAW_F, &s_preview_exposed, 1); 

	m_wlineBetween = abiPhabLocateWidget(m_windowMain,"toggleLineBetween"); 
	localizeLabel(m_wlineBetween, pSS, AP_STRING_ID_DLG_Column_Line_Between);
	PtAddCallback(m_wlineBetween, Pt_CB_ACTIVATE, s_toggle_clicked, this);

	m_buttonOK = abiPhabLocateWidget(m_windowMain,"btnOK"); 
	localizeLabel(m_buttonOK, pSS, XAP_STRING_ID_DLG_OK);
	PtAddCallback(m_buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	m_buttonCancel = abiPhabLocateWidget(m_windowMain,"btnCancel"); 
	localizeLabel(m_buttonCancel, pSS, XAP_STRING_ID_DLG_Cancel);
	PtAddCallback(m_buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);
	return m_windowMain;
}

void AP_QNXDialog_Columns::_populateWindowData(void)
{
}

void AP_QNXDialog_Columns::_storeWindowData(void)
{
}

void AP_QNXDialog_Columns::enableLineBetweenControl(bool bState)
{
}
