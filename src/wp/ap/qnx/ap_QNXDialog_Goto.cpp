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
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Goto.h"
#include "ap_QNXDialog_Goto.h"
#include "ut_qnxHelper.h"

#include "fv_View.h"

/*****************************************************************/
XAP_Dialog * AP_QNXDialog_Goto::static_constructor(XAP_DialogFactory * pFactory,
												   XAP_Dialog_Id id)
{
	AP_QNXDialog_Goto * p = new AP_QNXDialog_Goto(pFactory,id);
	return p;
}

AP_QNXDialog_Goto::AP_QNXDialog_Goto(XAP_DialogFactory * pDlgFactory,
									   XAP_Dialog_Id id)
	: AP_Dialog_Goto(pDlgFactory,id)
{
	m_mainWindow = NULL;
}

AP_QNXDialog_Goto::~AP_QNXDialog_Goto(void)
{
}

/********/
static int s_gotoClicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Goto *dlg = (AP_QNXDialog_Goto *)data;
	UT_ASSERT(dlg);

	dlg->processGoto (NULL);

	return Pt_CONTINUE;
}

static int s_nextClicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Goto *dlg = (AP_QNXDialog_Goto *)data;
	UT_ASSERT(dlg);

	dlg->processGoto ("+1");

	return Pt_CONTINUE;
}

static int s_prevClicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Goto *dlg = (AP_QNXDialog_Goto *)data;
	UT_ASSERT(dlg);

	dlg->processGoto ("-1");

	return Pt_CONTINUE;
}

static int s_closeClicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Goto *dlg = (AP_QNXDialog_Goto *)data;
	UT_ASSERT(dlg);

	dlg->destroy();
	
	return Pt_CONTINUE;
}

static int s_deleteClicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Goto *dlg = (AP_QNXDialog_Goto *)data;
	UT_ASSERT(dlg);

	dlg->destroy();
	
	return Pt_CONTINUE;
}


void AP_QNXDialog_Goto::runModeless (XAP_Frame * pFrame)
{
	_constructWindow ();
	UT_ASSERT (m_mainWindow);

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid = (UT_sint32) getDialogId ();

	m_pApp->rememberModelessId(sid, (XAP_Dialog_Modeless *) m_pDialog);

	// This magic command displays the frame that characters will be
	// inserted into.
	connectFocusModeless (m_mainWindow, m_pApp);

	UT_QNXCenterWindow(NULL, m_mainWindow);

	PtRealizeWidget(m_mainWindow);
}

void AP_QNXDialog_Goto::activate (void)
{
	UT_ASSERT (m_mainWindow);
	ConstructWindowName();

	ConstructWindowName();
	PtSetResource(m_mainWindow, Pt_ARG_WINDOW_TITLE, m_WindowName, 0);
	PtWindowFocus(m_mainWindow);
}

void AP_QNXDialog_Goto::destroy (void)
{
	if (!m_mainWindow) {
		return;
	}
	
	modeless_cleanup();
	PtWidget_t *win = m_mainWindow;
	m_mainWindow = NULL;
	PtDestroyWidget(win);
}

void AP_QNXDialog_Goto::notifyActiveFrame(XAP_Frame *pFrame)
{
	activate();
}

#define _TR(x) (char *)UT_XML_transNoAmpersands((x))

PtWidget_t * AP_QNXDialog_Goto::_constructWindow (void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	PtArg_t  args[10];
	int		  n;

	n = 0;
	ConstructWindowName();
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, m_WindowName, 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	m_mainWindow = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(m_mainWindow, Pt_CB_WINDOW_CLOSING, s_deleteClicked, this);
	
	// Vertical grouping initially
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);	
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 5, 0);
	PtWidget_t *vgroup = PtCreateWidget(PtGroup, m_mainWindow, n, args);

	// Horizontal grouping for the text box
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_X, 5, 0);
	PtWidget_t *htextgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	// One vertical group for the list & label
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);	
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
			 Pt_GROUP_EQUAL_SIZE_HORIZONTAL, Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	//PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 5, 0);
	PtWidget_t *vlistgroup = PtCreateWidget(PtGroup, htextgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _TR(pSS->getValue(AP_STRING_ID_DLG_Goto_Label_What)), 0);
	PtCreateWidget(PtLabel, vlistgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_VISIBLE_COUNT, 10, 0);
	m_wList = PtCreateWidget(PtList, vlistgroup, n, args);
	char **tmp2 = getJumpTargets ();
	int indx;
	for (indx = 0; tmp2[indx] != NULL; indx++) { ; }
	PtListAddItems(m_wList, (const char **)tmp2, indx, 0);
	PtListSelectPos(m_wList, 1);

	// One vertical group for the text & instructions
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);	
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
			 Pt_GROUP_EQUAL_SIZE_HORIZONTAL, Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	//PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 5, 0);
	PtWidget_t *vlabelgroup = PtCreateWidget(PtGroup, htextgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _TR(pSS->getValue(AP_STRING_ID_DLG_Goto_Label_Number)), 0);
	PtCreateWidget(PtLabel, vlabelgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 300, 0);
	m_wEntry = PtCreateWidget(PtText, vlabelgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_MULTITEXT_ROWS, 10, 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_SET | Pt_HIGHLIGHTED);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (AP_STRING_ID_DLG_Goto_Label_Help), 0);
	PtCreateWidget(PtMultiText, vlabelgroup, n, args);

	// Horizontal grouping of buttons
	n = 0;
	PtWidget_t *hbuttongroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (AP_STRING_ID_DLG_Goto_Btn_Prev), NULL);
	m_wPrev = PtCreateWidget(PtButton, hbuttongroup, n, args);
	PtAddCallback(m_wPrev, Pt_CB_ACTIVATE, s_prevClicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (AP_STRING_ID_DLG_Goto_Btn_Next), NULL);
	m_wNext = PtCreateWidget(PtButton, hbuttongroup, n, args);
	PtAddCallback(m_wNext, Pt_CB_ACTIVATE, s_nextClicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (AP_STRING_ID_DLG_Goto_Btn_Goto), NULL);
	m_wGoto = PtCreateWidget(PtButton, hbuttongroup, n, args);
	PtAddCallback(m_wGoto, Pt_CB_ACTIVATE, s_gotoClicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_Close), NULL);
	m_wClose = PtCreateWidget(PtButton, hbuttongroup, n, args);
	PtAddCallback(m_wClose, Pt_CB_ACTIVATE, s_closeClicked, this);

	return (m_mainWindow);
}

int AP_QNXDialog_Goto::getSelectedRow (void)
{
	short *index = NULL;

	PtGetResource(m_wList, Pt_ARG_SELECTION_INDEXES, &index, 0);

	if (!index) {
		return 0;
	}
	
	return index[0]-1;
}

void AP_QNXDialog_Goto::processGoto (const char *number)
{
	//If a NULL is passed as the number, grab the string from the text box
	if (!number) {
		char *value = NULL;
		PtGetResource(m_wEntry, Pt_ARG_TEXT_STRING, &value, 0);
		number = value;
	}

	if (!number || !*number) {
		UT_DEBUGMSG(("No goto string!"));
		return;
	}

	UT_UCSChar *ucsnumber = (UT_UCSChar *) malloc (sizeof (UT_UCSChar) * (strlen(number) + 1));
	UT_UCS_strcpy_char (ucsnumber, number);

	int target = getSelectedRow ();
	getView()->gotoTarget ((AP_JumpTarget) target, ucsnumber);

	free (ucsnumber);
}


