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
#include "ap_Dialog_Field.h"
#include "ap_QNXDialog_Field.h"
#include "ap_QNXDialog_Background.h"
#include "ut_qnxHelper.h"

/*****************************************************************/
static int s_ok_clicked (PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Background * dlg = (AP_QNXDialog_Background *)data;
	UT_ASSERT(dlg);
	dlg->eventOk();
	return Pt_CONTINUE;
}

static int s_cancel_clicked (PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Background * dlg = (AP_QNXDialog_Background *)data;
	UT_ASSERT(dlg);
	dlg->eventCancel();
	return Pt_CONTINUE;
}

static int s_color_changed(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Background * dlg = (AP_QNXDialog_Background *)data;
	UT_ASSERT(dlg);
  
	PgColor_t *clr = NULL;
	PtGetResource(widget, Pt_ARG_CS_COLOR, &clr, 0);
	if (!clr) {
		return Pt_CONTINUE;
	}

	UT_RGBColor col;
	col.m_red = PgRedValue(*clr);
	col.m_grn = PgGreenValue(*clr);
	col.m_blu = PgBlueValue(*clr);

	dlg->setColor (col);
	return Pt_CONTINUE;
}


/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Background::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_Background * p = new AP_QNXDialog_Background(pFactory,id);
	return p;
}

AP_QNXDialog_Background::AP_QNXDialog_Background(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Background(pDlgFactory,id)
{
}

AP_QNXDialog_Background::~AP_QNXDialog_Background(void)
{
}

void AP_QNXDialog_Background::runModal(XAP_Frame * pFrame)
{
	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the window of the parent frame
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	PtSetParentWidget(parentWindow);

	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(mainWindow, pFrame);

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

PtWidget_t * AP_QNXDialog_Background::_constructWindow (void)
{
	PtWidget_t *window;
	PtWidget_t *vboxMain, *hgroup;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	PtArg_t args[10];
	int n;
  
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, 
	    UT_XML_transNoAmpersands(pSS->getValue(AP_STRING_ID_DLG_Background_Title)), 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	window = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(window, Pt_CB_WINDOW_CLOSING, s_cancel_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 5, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EQUAL_SIZE_HORIZONTAL, Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	vboxMain = PtCreateWidget(PtGroup, window, n, args);

	/* Add a colour selector */
	PtWidget_t *colorsel;

	n = 0;
	hgroup = PtCreateWidget(PtGroup, vboxMain, n, args);

	UT_RGBColor c = getColor();

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_CS_COLOR, PgRGB(c.m_red, c.m_grn, c.m_blu), 0);
	colorsel = PtCreateWidget(PtColorPanel, hgroup, n, args);
	PtAddCallback(colorsel, Pt_CB_CS_COLOR_CHANGED, s_color_changed, this);

	/* Bottom row of buttons */
	n = 0;
	hgroup = PtCreateWidget(PtGroup, vboxMain, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *buttonCancel = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *buttonOK = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	return window;
}

void AP_QNXDialog_Background::eventOk (void)
{
	setAnswer (a_OK);
	done++;
}

void AP_QNXDialog_Background::eventCancel (void)
{
	if(!done++) {
		setAnswer(a_CANCEL);
	}
}
