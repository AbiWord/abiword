/* AbiSource Application Framework
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include <stdio.h>

// This header defines some functions for QNX dialogs,
// like centering them, measuring them, etc.
#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_WindowMore.h"
#include "xap_QNXDlg_WindowMore.h"
#include "ut_qnxHelper.h"

/*****************************************************************/

XAP_Dialog * XAP_QNXDialog_WindowMore::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	XAP_QNXDialog_WindowMore * p = new XAP_QNXDialog_WindowMore(pFactory,id);
	return p;
}

XAP_QNXDialog_WindowMore::XAP_QNXDialog_WindowMore(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: XAP_Dialog_WindowMore(pDlgFactory,id)
{
}

XAP_QNXDialog_WindowMore::~XAP_QNXDialog_WindowMore(void)
{
}

/*****************************************************************/
// These are all static callbacks, bound to GTK or GDK events.

static int s_ok_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_WindowMore *dlg = (XAP_QNXDialog_WindowMore *)data;
	UT_ASSERT(widget && dlg);

	dlg->event_OK();
	return Pt_CONTINUE;
}

static int s_cancel_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_WindowMore *dlg = (XAP_QNXDialog_WindowMore *)data;
	UT_ASSERT(widget && dlg);

	dlg->event_Cancel();
	return Pt_CONTINUE;
}

static int s_clist_event(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_WindowMore *dlg = (XAP_QNXDialog_WindowMore *)data;
	UT_ASSERT(widget && dlg);

	// Only respond to double clicks
#if 0
	dlg->event_DoubleClick();
#endif
	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_WindowMore *dlg = (XAP_QNXDialog_WindowMore *)data;
	UT_ASSERT(dlg);

	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}
/*****************************************************************/

void XAP_QNXDialog_WindowMore::runModal(XAP_Frame * pFrame)
{
	// Initialize member so we know where we are now
	m_ndxSelFrame = m_pApp->findFrame(pFrame);
	UT_ASSERT(m_ndxSelFrame >= 0);

	XAP_QNXFrame *pQNXFrame = (XAP_QNXFrame *)pFrame;
	UT_ASSERT(pQNXFrame);

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

	UT_QNXBlockWidget(parentWindow, 0);
	PtDestroyWidget(mainWindow);
}

void XAP_QNXDialog_WindowMore::event_OK(void)
{
	int row;
	
	if((row = _GetFromList()) > 0) {
		m_ndxSelFrame = row;
	}
	m_answer = XAP_Dialog_WindowMore::a_OK;
	done = 1;
}

void XAP_QNXDialog_WindowMore::event_Cancel(void)
{
	m_answer = XAP_Dialog_WindowMore::a_CANCEL;
	done = 1;
}

void XAP_QNXDialog_WindowMore::event_DoubleClick(void)
{
	event_OK();
}

void XAP_QNXDialog_WindowMore::event_WindowDelete(void)
{
	if(!done++) {
		m_answer = XAP_Dialog_WindowMore::a_CANCEL;	
	}
}


/*****************************************************************/

PtWidget_t * XAP_QNXDialog_WindowMore::_constructWindow(void)
{
	// This is the top level GTK widget, the window.
	// It's created with a "dialog" style.
	PtWidget_t *windowMain;
	PtWidget_t *vboxMain;
	PtWidget_t *vboxGroup;
	PtWidget_t *hboxGroup;

	// The child of the scrollable area is our list of windows
	PtWidget_t *clistWindows;
	
	// These are the buttons.
	PtWidget_t *buttonOK;
	PtWidget_t *buttonCancel;
	PtArg_t    args[10];
	int 		n;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// Create the new top level window.
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(XAP_STRING_ID_DLG_MW_MoreWindows), 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	windowMain = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowMain, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

#define MARGIN_SIZE 10

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, MARGIN_SIZE, 0); 
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, MARGIN_SIZE, 0); 
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, MARGIN_SIZE, 0); 
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
					Pt_GROUP_EQUAL_SIZE_HORIZONTAL,
					Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	vboxMain = PtCreateWidget(PtGroup, windowMain, n, args);
	pretty_group(vboxMain, pSS->getValue(XAP_STRING_ID_DLG_MW_Activate));

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
				Pt_GROUP_STRETCH_VERTICAL | Pt_GROUP_STRETCH_HORIZONTAL,
				Pt_GROUP_STRETCH_VERTICAL | Pt_GROUP_STRETCH_HORIZONTAL);
	vboxGroup = PtCreateWidget(PtGroup, vboxMain, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 2 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	clistWindows = PtCreateWidget(PtList, vboxGroup, n, args);
	PtAddCallback(clistWindows, Pt_CB_SELECTION, s_clist_event, this);


	/* Buttons on the bottom */
	n = 0;
	hboxGroup = PtCreateWidget(PtGroup, vboxMain, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonOK = PtCreateWidget(PtButton, hboxGroup, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonCancel = PtCreateWidget(PtButton, hboxGroup, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	m_windowMain = windowMain;
	m_clistWindows = clistWindows;
	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

	return windowMain;
}

void XAP_QNXDialog_WindowMore::_populateWindowData(void) {
	// We just do one thing here, which is fill the list with
	// all the windows.

	for (UT_uint32 i = 0; i < m_pApp->getFrameCount(); i++) {
		XAP_Frame * f = m_pApp->getFrame(i);
		UT_ASSERT(f);
		const char * s = f->getTitle(128);	// TODO: chop this down more? 
		
		PtListAddItems(m_clistWindows, &s, 1, 0);
	} 
	PtListSelectPos(m_clistWindows, 1);
	m_ndxSelFrame = 0;
}

int XAP_QNXDialog_WindowMore::_GetFromList(void) {
	unsigned short *index;
	
	PtGetResource(m_clistWindows, Pt_ARG_SELECTION_INDEXES, &index, 0);

	if (index && *index) {
		return (*index) - 1;
	}

	return -1;
}


