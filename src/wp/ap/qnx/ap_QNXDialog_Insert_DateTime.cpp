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
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Insert_DateTime.h"
#include "ap_QNXDialog_Insert_DateTime.h"
#include "ut_qnxHelper.h"

#include <stdio.h>

/*****************************************************************/

#define	LIST_ITEM_INDEX_KEY "index"

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Insert_DateTime::static_constructor(XAP_DialogFactory * pFactory,
															   XAP_Dialog_Id id)
{
	AP_QNXDialog_Insert_DateTime * p = new AP_QNXDialog_Insert_DateTime(pFactory,id);
	return p;
}

AP_QNXDialog_Insert_DateTime::AP_QNXDialog_Insert_DateTime(XAP_DialogFactory * pDlgFactory,
															 XAP_Dialog_Id id)
	: AP_Dialog_Insert_DateTime(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_buttonOK = NULL;
	m_buttonCancel = NULL;

	m_listFormats = NULL;
}

AP_QNXDialog_Insert_DateTime::~AP_QNXDialog_Insert_DateTime(void)
{
}

/*****************************************************************/

static int s_ok_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Insert_DateTime * dlg = (AP_QNXDialog_Insert_DateTime *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
	return Pt_CONTINUE;
}

static int s_cancel_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Insert_DateTime * dlg = (AP_QNXDialog_Insert_DateTime *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Insert_DateTime * dlg = (AP_QNXDialog_Insert_DateTime *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}

static int s_item_selected(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Insert_DateTime * dlg = (AP_QNXDialog_Insert_DateTime *)data;
	PtListCallback_t *plist = (PtListCallback_t *)info->cbdata;
	dlg->m_index = plist->item_pos;
	return Pt_CONTINUE;
}

/*****************************************************************/

void AP_QNXDialog_Insert_DateTime::runModal(XAP_Frame * pFrame)
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

	UT_QNXBlockWidget(parentWindow, 0);
	PtDestroyWidget(mainWindow);
}

void AP_QNXDialog_Insert_DateTime::event_OK(void)
{
	UT_ASSERT(m_windowMain && m_listFormats);
/*	
	// find item selected in list box, save it to m_iFormatIndex
	PtArg_t arg;
	int     pindex = 0;

	PtSetArg(&arg, Pt_ARG_CBOX_SEL_ITEM, &pindex, 0);
	PtGetResources(m_listFormats, 1, &arg);
	m_iFormatIndex = pindex;

	printf("Got selected item %d \n", m_iFormatIndex);

	if (m_iFormatIndex == 0) {
		m_answer = AP_Dialog_Insert_DateTime::a_CANCEL;
		done = 1;
		return;
	}
	m_iFormatIndex--;
*/
	m_iFormatIndex = m_index -1;
	m_answer = AP_Dialog_Insert_DateTime::a_OK;
	done = 1;
}

void AP_QNXDialog_Insert_DateTime::event_Cancel(void)
{
	m_answer = AP_Dialog_Insert_DateTime::a_CANCEL;
	done = 1;
}

void AP_QNXDialog_Insert_DateTime::event_WindowDelete(void)
{
	if (!done)
		m_answer = AP_Dialog_Insert_DateTime::a_CANCEL;	
	done = 1;
}

/*****************************************************************/
PtWidget_t * AP_QNXDialog_Insert_DateTime::_constructWindow(void)
{
	PtWidget_t *windowMain;
	PtWidget_t *vboxMain;
	PtWidget_t *vboxGroup, *hboxGroup;
	PtWidget_t *listFormats;
	PtWidget_t *buttonOK;
	PtWidget_t *buttonCancel;

	PtArg_t args[10];
	int 	n;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, UT_XML_transNoAmpersands(pSS->getValue(AP_STRING_ID_DLG_DateTime_DateTimeTitle)), 0);
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

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
				Pt_GROUP_STRETCH_VERTICAL | Pt_GROUP_STRETCH_HORIZONTAL,
				Pt_GROUP_STRETCH_VERTICAL | Pt_GROUP_STRETCH_HORIZONTAL);
	vboxGroup =  PtCreateWidget(PtGroup, vboxMain, n, args);
  	pretty_group(vboxGroup, pSS->getValue(AP_STRING_ID_DLG_DateTime_AvailableFormats));

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_FLAGS, 0, Pt_EDITABLE);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 2 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	listFormats = PtCreateWidget(PtList, vboxGroup, n, args);
	PtAddCallback(listFormats, Pt_CB_SELECTION, s_item_selected, this);

	/* Buttons on the bottom */
	n = 0;
	hboxGroup =  PtCreateWidget(PtGroup, vboxMain, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING,  pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	buttonOK = PtCreateWidget(PtButton, hboxGroup, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING,  pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	buttonCancel = PtCreateWidget(PtButton, hboxGroup, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);


	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowMain;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;
	m_listFormats = listFormats;

	return windowMain;
}

void AP_QNXDialog_Insert_DateTime::_populateWindowData(void)
{
	const char *items[1];
	int i;

	UT_ASSERT(m_windowMain && m_listFormats);

	// this constant comes from ap_Dialog_Insert_DateTime.h
    char szCurrentDateTime[CURRENT_DATE_TIME_SIZE];
	
    time_t tim = time(NULL);
    struct tm *pTime = localtime(&tim);

    for (i = 0, items[0] = szCurrentDateTime; InsertDateTimeFmts[i]; i++) {
        strftime(szCurrentDateTime, CURRENT_DATE_TIME_SIZE, InsertDateTimeFmts[i], pTime);
		PtListAddItems(m_listFormats, items, 1, 0);
	}
	//Select the first one?
	PtArg_t arg;
	PtSetArg(&arg, Pt_ARG_CBOX_SEL_ITEM, 1, 0);
	PtSetResources(m_listFormats, 1, &arg);
	m_index = 1;
}

