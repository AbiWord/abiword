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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_WordCount.h"
#include "ap_QNXDialog_WordCount.h"

#include "ut_qnxHelper.h"
#include <stdio.h>

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_WordCount::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	AP_QNXDialog_WordCount * p = new AP_QNXDialog_WordCount(pFactory,id);
	return p;
}

AP_QNXDialog_WordCount::AP_QNXDialog_WordCount(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_WordCount(pDlgFactory,id)
{
	m_windowMain = NULL;
	m_buttonOK = NULL;

}

AP_QNXDialog_WordCount::~AP_QNXDialog_WordCount(void)
{
}

/*****************************************************************/

static int s_ok_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_WordCount * dlg = (AP_QNXDialog_WordCount *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
	return Pt_CONTINUE;
}


static int s_delete_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_WordCount * dlg = (AP_QNXDialog_WordCount *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}

/*****************************************************************/

void AP_QNXDialog_WordCount::runModal(XAP_Frame * pFrame)
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

	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);

	PtRealizeWidget(mainWindow);
	int count = PtModalStart();
	done = 0;
	while(!done) {
		PtProcessEvent();
	}
	PtModalEnd(count);

	UT_QNXBlockWidget(parentWindow, 0);
	PtDestroyWidget(mainWindow);
}

void AP_QNXDialog_WordCount::event_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_WordCount::a_OK;
	done = 1;
}

void AP_QNXDialog_WordCount::event_WindowDelete(void)
{
	if (!done)
		m_answer = AP_Dialog_WordCount::a_CANCEL;	
	done = 1;
}

/*****************************************************************/

PtWidget_t * AP_QNXDialog_WordCount::_constructWindow(void)
{

	PtWidget_t * windowWordCount;
	PtWidget_t * buttonOK;
	PtWidget_t * labelWCount;
	PtWidget_t * labelWords;
	PtWidget_t * labelPCount;
	PtWidget_t * labelPara;
	PtWidget_t * labelCCount;
	PtWidget_t * labelChar;
	PtWidget_t * dataTable;

	PtArg_t	args[10];
	char 	buffer[200];
	int 	n;


	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(AP_STRING_ID_DLG_WordCount_WordCountTitle), 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	windowWordCount = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowWordCount, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_HORIZONTAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ROWS_COLS, 2, 0);
#define MARGIN_SIZE 10 
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, MARGIN_SIZE, 0);
	PtWidget_t *hbox = PtCreateWidget(PtGroup, windowWordCount, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtWidget_t *vboxlabel = PtCreateWidget(PtGroup, hbox, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtWidget_t *vboxvalue = PtCreateWidget(PtGroup, hbox, n, args);

	/* Statistics placeholder */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Statistics:", 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	n = 0;	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, " ", 0);
	PtCreateWidget(PtLabel, vboxvalue, n, args);


	/* Pages */
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Pages));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	FREEP(unixstr);

	n = 0;
	sprintf(buffer, "%d", m_count.page);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, buffer, 0);
	PtSetArg(&args[n++], Pt_ARG_HORIZONTAL_ALIGNMENT, Pt_RIGHT, 0);
	PtCreateWidget(PtLabel, vboxvalue, n, args);

	/* Words */
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Words));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	FREEP(unixstr);

	n = 0;
	sprintf(buffer, "%d", m_count.word);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, buffer, 0);
	PtSetArg(&args[n++], Pt_ARG_HORIZONTAL_ALIGNMENT, Pt_RIGHT, 0);
	PtCreateWidget(PtLabel, vboxvalue, n, args);

	/* Characters (no spaces) */
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Characters_No));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	FREEP(unixstr);

	n = 0;
	sprintf(buffer, "%d", m_count.ch_no);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, buffer, 0);
	PtSetArg(&args[n++], Pt_ARG_HORIZONTAL_ALIGNMENT, Pt_RIGHT, 0);
	labelWCount = PtCreateWidget(PtLabel, vboxvalue, n, args);

	/* Characters (spaces) */
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Characters_Sp));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	FREEP(unixstr);

	n = 0;
	sprintf(buffer, "%d", m_count.ch_sp);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, buffer, 0);
	PtSetArg(&args[n++], Pt_ARG_HORIZONTAL_ALIGNMENT, Pt_RIGHT, 0);
	labelWCount = PtCreateWidget(PtLabel, vboxvalue, n, args);


	/* Paragraphs */
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Paragraphs));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	FREEP(unixstr);

	n = 0;
	sprintf(buffer, "%d", m_count.para);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, buffer, 0);
	PtSetArg(&args[n++], Pt_ARG_HORIZONTAL_ALIGNMENT, Pt_RIGHT, 0);
	PtCreateWidget(PtLabel, vboxvalue, n, args);

	/* Lines */
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Lines));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	FREEP(unixstr);

	n = 0;
	sprintf(buffer, "%d", m_count.line);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, buffer, 0);
	PtSetArg(&args[n++], Pt_ARG_HORIZONTAL_ALIGNMENT, Pt_RIGHT, 0);
	PtCreateWidget(PtLabel, vboxvalue, n, args);

	/* Close button */	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, " ", 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Close), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 80, 0);
	buttonOK = PtCreateWidget(PtButton, vboxvalue, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	m_windowMain = windowWordCount;
	m_buttonOK = buttonOK;

	return windowWordCount;
}

void AP_QNXDialog_WordCount::_populateWindowData(void)
{
}



