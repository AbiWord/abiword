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
	printf("Running the time main window loop \n");
	PtRealizeWidget(mainWindow);
	int count = PtModalStart();
	done = 0;
	while(!done) {
		PtProcessEvent();
	}
	PtModalEnd(count);

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
	PtWidget_t *labelFormats;
	PtWidget_t *listFormats;
	PtWidget_t *buttonOK;
	PtWidget_t *buttonCancel;

	PtArg_t args[10];
	int     n;
	PhArea_t area;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions


#define WIN_HEIGHT 100
#define WIN_WIDTH  200
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(AP_STRING_ID_DLG_DateTime_DateTimeTitle), 0);
	area.size.w = WIN_WIDTH; 
	area.size.h = WIN_HEIGHT;
	PtSetArg(&args[n++], Pt_ARG_DIM, &area.size, 0);
	PtSetParentWidget(NULL);
	windowMain = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowMain, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

#define LABEL_HEIGHT 15
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_DateTime_AvailableFormats));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	area.pos.x = 10; area.pos.y = 10;
	PtSetArg(&args[n++], Pt_ARG_POS, &area.pos, 0);
	labelFormats = PtCreateWidget(PtLabel, windowMain, n, args);
	FREEP(unixstr);


	//Add in a list with the formats
#define COMBO_HEIGHT LABEL_HEIGHT
	n = 0;
	area.pos.y += LABEL_HEIGHT + 10;
	PtSetArg(&args[n++], Pt_ARG_POS, &area.pos, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, WIN_WIDTH - 10, 0);
	PtSetArg(&args[n++], Pt_ARG_CBOX_MAX_VISIBLE_COUNT, 3, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_FLAGS, 0, Pt_EDITABLE);
	listFormats = PtCreateWidget(PtComboBox, windowMain, n, args);
	PtAddCallback(listFormats, Pt_CB_SELECTION, s_item_selected, this);

#define BUTTON_HEIGHT 24
#define BUTTON_WIDTH  80
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	area.pos.y += COMBO_HEIGHT + 20;
	area.pos.x = WIN_WIDTH - BUTTON_WIDTH - 5;
	PtSetArg(&args[n++], Pt_ARG_POS, &area.pos, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, BUTTON_WIDTH, 0);
	buttonCancel = PtCreateWidget(PtButton, windowMain, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	area.pos.x -= BUTTON_WIDTH + 10;
	PtSetArg(&args[n++], Pt_ARG_POS, &area.pos, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, BUTTON_WIDTH, 0);
	buttonOK = PtCreateWidget(PtButton, windowMain, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

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

