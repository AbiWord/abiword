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

// This header defines some functions for QNX dialogs,
// like centering them, measuring them, etc.

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Break.h"
#include "ap_QNXDialog_Break.h"

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
}

static int s_cancel_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Break *dlg = (AP_QNXDialog_Break *)data;
	dlg->event_Cancel();
}

static int s_delete_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
        AP_QNXDialog_Break *dlg = (AP_QNXDialog_Break *)data;
        dlg->event_WindowDelete();
        return Pt_CONTINUE;
}

/*****************************************************************/

void AP_QNXDialog_Break::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();

#if 0	
	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the GtkWindow of the parent frame
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);
	gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// Run into the GTK event loop for this window.
	gtk_main();

	
	gtk_widget_destroy(mainWindow);
#endif
	printf("Running the break main window loop \n");
	PtRealizeWidget(m_windowMain);
	int count = PtModalStart();
	done = 0;
	while(!done) {
		PtProcessEvent();
	}
	PtModalEnd(count);

	_storeWindowData();

	PtDestroyWidget(m_windowMain);
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
	PtWidget_t * boxTop, *boxBottom;
	PtWidget_t * tableInsert;
	PtWidget_t * labelInsert;
	PtWidget_t * radiobuttonPageBreak;
	PtWidget_t * radiobuttonNextPage;
	PtWidget_t * radiobuttonContinuous;
	PtWidget_t * radiobuttonColumnBreak;
	PtWidget_t * radiobuttonEvenPage;
	PtWidget_t * radiobuttonOddPage;
	PtWidget_t * labelSectionBreaks;
	PtWidget_t * hseparator9;
	PtWidget_t * hseparator10;
	PtWidget_t * hbuttonboxBreak;
	PtWidget_t * buttonOK;
	PtWidget_t * buttonCancel;
	PtArg_t	   args[10];
	int 	   bmi, n, height, width;
	PhArea_t 	area;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	n = bmi = 0;
	height = 140; width = 300;
    UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_BreakTitle));
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, height, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, width, 0);
	PtSetArg(&args[n++], Pt_ARG_TITLE, unixstr, 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, Ph_WM_RENDER_RESIZE);
	windowBreak = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowBreak, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	/* TODO: Add all these items to a group */

#define LABEL_WIDTH  100
#define RADIO_WIDTH  150 
#define GEN_HEIGHT   20 
#define GEN_OFFSET   5
/* 
	printf("Area set to %d,%d %d/%d \n", 
			area.pos.x, area.pos.y, area.size.w, area.size.h);
*/

	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_Insert));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	area.pos.x = GEN_OFFSET; area.pos.y = GEN_OFFSET;
	area.size.w = LABEL_WIDTH; area.size.h = GEN_HEIGHT;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	labelInsert = PtCreateWidget(PtLabel, windowBreak, n, args);

	/* TODO: Add a seperator */

	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_PageBreak));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	area.pos.x = 2*GEN_OFFSET; area.pos.y += area.size.h + GEN_OFFSET;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	bm[bmi].widget =
	radiobuttonPageBreak = PtCreateWidget(PtToggleButton, windowBreak, n, args);
	bm[bmi++].type = AP_Dialog_Break::b_PAGE;

	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_ColumnBreak));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	area.pos.x += area.size.w + GEN_OFFSET; 
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	bm[bmi].widget = 
	radiobuttonColumnBreak = PtCreateWidget(PtToggleButton, windowBreak, n, args);
	bm[bmi++].type = AP_Dialog_Break::b_COLUMN;

	/* --- */

	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_SectionBreaks));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	area.pos.x = GEN_OFFSET; area.pos.y += area.size.h + GEN_OFFSET;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	labelSectionBreaks = PtCreateWidget(PtLabel, windowBreak, n, args);

	/* TODO: Add a seperator */

	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_NextPage));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	area.pos.x = 2*GEN_OFFSET; area.pos.y += area.size.h + GEN_OFFSET;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	bm[bmi].widget = 
	radiobuttonNextPage = PtCreateWidget(PtToggleButton, windowBreak, n, args);
	bm[bmi++].type = AP_Dialog_Break::b_NEXTPAGE;

	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_EvenPage));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	area.pos.x += area.size.w + GEN_OFFSET; 
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	bm[bmi].widget = 
	radiobuttonEvenPage = PtCreateWidget(PtToggleButton, windowBreak, n, args);
	bm[bmi++].type = AP_Dialog_Break::b_EVENPAGE;

	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_Continuous));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	area.pos.x = 2*GEN_OFFSET; area.pos.y += area.size.h + GEN_OFFSET;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	bm[bmi].widget = 
	radiobuttonContinuous = PtCreateWidget(PtToggleButton, windowBreak, n, args);
	bm[bmi++].type = AP_Dialog_Break::b_CONTINUOUS;

	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Break_OddPage));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	area.pos.x += area.size.w + GEN_OFFSET; 
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	bm[bmi].widget = 
	radiobuttonOddPage = PtCreateWidget(PtToggleButton, windowBreak, n, args);
	bm[bmi++].type = AP_Dialog_Break::b_ODDPAGE;


	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	area.pos.x = GEN_OFFSET; area.pos.y += area.size.h + GEN_OFFSET;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	buttonOK = PtCreateWidget(PtButton, windowBreak, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	area.pos.x += area.size.w + GEN_OFFSET; 
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	buttonCancel = PtCreateWidget(PtButton, windowBreak, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowBreak;
	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;
	
	return windowBreak;
}

void AP_QNXDialog_Break::_populateWindowData(void)
{
#if 0
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.

	PtWidget_t * widget = _findRadioByID(m_break);
	UT_ASSERT(widget);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
#endif
}

void AP_QNXDialog_Break::_storeWindowData(void)
{
	m_break = _getActiveRadioItem();
}

AP_Dialog_Break::breakType AP_QNXDialog_Break::_getActiveRadioItem(void)
{
	PtArg_t arg;
	int		*value;

	for (int i = 0; i < BREAK_COUNT; i++) {
		value = NULL;
		PtSetArg(&arg, Pt_ARG_FLAGS, &value, 0);
		PtGetResources(bm[i].widget, 1, &arg);
		if (*value & Pt_SET) {
			return ( AP_Dialog_Break::breakType)(bm[i].type);
		}
	}
	m_answer = AP_Dialog_Break::a_CANCEL;
	return AP_Dialog_Break::b_PAGE;
}

