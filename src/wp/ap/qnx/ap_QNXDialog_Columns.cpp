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
#include "xap_QNXFrame.h"

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
	m_windowMain = NULL;

	m_buttonOK = NULL;
	m_buttonCancel = NULL;

//	m_radioGroup = NULL;
}

AP_QNXDialog_Columns::~AP_QNXDialog_Columns(void)
{
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
	XAP_QNXFrame * pQNXFrame = (XAP_QNXFrame *)(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the Window of the parent frame
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);

	// Show the top level dialog,
	PtRealizeWidget(mainWindow);
	
        UT_QNXBlockWidget(parentWindow, 0);

        int count = PtModalStart();
        done = 0;
        while(!done) {
                PtProcessEvent();
        }
        PtModalEnd(MODAL_END_ARG(count));

	_storeWindowData();

        UT_QNXBlockWidget(parentWindow, 0);
        PtDestroyWidget(mainWindow);
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

/*****************************************************************/

int pretty_group(PtWidget_t *w, const char *title) {
	int n, width;
	PtArg_t args[10];

	n = 0;

	if (title && *title) {
		PhRect_t rect;
		const char *font = NULL;
		char *defaultfont = { "helv10" };

		PtGetResource(w, Pt_ARG_TITLE_FONT, &font, 0);
		if (!font) {
			font = defaultfont;	
		}

		PfExtentText(&rect, NULL, font, title, 0);
		//PtSetArg(&args[n++], Pt_ARG_WIDTH, rect.lr.x - rect.ul.x + 10, 0);

		PtSetArg(&args[n++], Pt_ARG_TITLE, title, 0);
		PtSetArg(&args[n++], Pt_ARG_CONTAINER_FLAGS, 
			Pt_SHOW_TITLE | Pt_ETCH_TITLE_AREA, 
			Pt_SHOW_TITLE | Pt_ETCH_TITLE_AREA);
	}
#define OUTLINE_GROUP (Pt_TOP_OUTLINE | Pt_TOP_BEVEL | \
					   Pt_BOTTOM_OUTLINE | Pt_BOTTOM_BEVEL | \
					   Pt_LEFT_OUTLINE | Pt_LEFT_BEVEL | \
					   Pt_RIGHT_OUTLINE | Pt_RIGHT_BEVEL)
	PtSetArg(&args[n++], Pt_ARG_BASIC_FLAGS, OUTLINE_GROUP, OUTLINE_GROUP);
	PtSetArg(&args[n++], Pt_ARG_BEVEL_WIDTH, 1, 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_HIGHLIGHTED, Pt_HIGHLIGHTED);

	PtSetResources(w, n, args);

	return 0;
}

PtWidget_t * AP_QNXDialog_Columns::_constructWindow(void)
{
	PtWidget_t * windowColumns;
	PtWidget_t * vboxMain;
	PtWidget_t * buttonOK;
	PtWidget_t * buttonCancel;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	int n;
	PtArg_t args[10];

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(AP_STRING_ID_DLG_Column_ColumnTitle), 0);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	windowColumns = PtCreateWidget(PtWindow, NULL, n, args);
//    PtAddCallback(windowColumns, Pt_CB_WINDOW_CLOSING, s_deleteClicked, this);
	
	/* Create a vertical box in which to stuff things */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 5, 0);
	vboxMain = PtCreateWidget(PtGroup, windowColumns, n, args);

	/* Create a horizontal box for the components */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_X, 15, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EQUAL_SIZE_VERTICAL, 0);
	PtWidget_t *hitem = PtCreateWidget(PtGroup, vboxMain, n, args);

	/* Create a vertical group for the columns */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_X, 5, 0);
	PtWidget_t *vbuttons = PtCreateWidget(PtGroup, hitem, n, args);
	pretty_group(vbuttons, NULL /* "Number of Columns" */);
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Number of Columns", 0);
	PtCreateWidget(PtLabel, vbuttons, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 24, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 24, 0);
	PtCreateWidget(PtButton, vbuttons, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 24, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 24, 0);
	PtCreateWidget(PtButton, vbuttons, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 24, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 24, 0);
	PtCreateWidget(PtButton, vbuttons, n, args);

	/* Create a vertical group for the preview */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtWidget_t *vpreview = PtCreateWidget(PtGroup, hitem, n, args);
	pretty_group(vpreview, NULL /* "Preview" */);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Preview", 0);
	PtCreateWidget(PtLabel, vpreview, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "PREVIEW HERE", 0);
	PtCreateWidget(PtLabel, vpreview, n, args);

	/* Create a vertical group for the response buttons */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtWidget_t *vaction = PtCreateWidget(PtGroup, hitem, n, args);

	n = 0;
    PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	buttonOK = PtCreateWidget(PtButton, vaction, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	n = 0;
    PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	buttonCancel = PtCreateWidget(PtButton, vaction, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	/* Put a "line between" toggle at the bottom */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Line between", 0);
	buttonCancel = PtCreateWidget(PtToggleButton, vboxMain, n, args);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowColumns;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

	//m_radioGroup = tableInsert_group;
	
	return windowColumns;
}

void AP_QNXDialog_Columns::_populateWindowData(void)
{
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.

//	PtWidget_t * widget = _findRadioByID(m_break);
//	UT_ASSERT(widget);
	
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
}

void AP_QNXDialog_Columns::_storeWindowData(void)
{
//	m_break = _getActiveRadioItem();
}

void AP_QNXDialog_Columns::enableLineBetweenControl(UT_Bool bState)
{
}
