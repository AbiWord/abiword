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

//////////////////////////////////////////////////////////////////
// THIS CODE RUNS BOTH THE "Find" AND THE "Find-Replace" DIALOGS.
//////////////////////////////////////////////////////////////////

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
#include "ap_Dialog_Replace.h"
#include "ap_QNXDialog_Replace.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Replace::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_QNXDialog_Replace * p = new AP_QNXDialog_Replace(pFactory,id);
	return p;
}

AP_QNXDialog_Replace::AP_QNXDialog_Replace(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Replace(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_entryFind = NULL;
	m_entryReplace = NULL;
	m_checkbuttonMatchCase = NULL;

	m_buttonFindNext = NULL;
	m_buttonReplace = NULL;
	m_buttonReplaceAll = NULL;

	m_buttonCancel = NULL;
}

AP_QNXDialog_Replace::~AP_QNXDialog_Replace(void)
{
}

/*****************************************************************/
static int s_delete_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}

static int s_find_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_Find();
}

static int s_replace_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_Replace();
}

static int s_replace_all_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_ReplaceAll();
}

static int s_cancel_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_Cancel();
}

static int s_match_case_toggled(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_MatchCaseToggled();
}

static int s_find_entry_activate(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_Find();
}

static int s_replace_entry_activate(PtWidget_t *w, void *data, PtCallbackInfo_t * e)
{
	AP_QNXDialog_Replace *dlg = (AP_QNXDialog_Replace *)data;
	dlg->event_Replace();
}
/*****************************************************************/

void AP_QNXDialog_Replace::runModal(XAP_Frame * pFrame)
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
	GtkWidget * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);
	gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// this dialogs needs this
	setView(static_cast<FV_View *> (pFrame->getCurrentView()));
#endif

	printf("Running the find main window loop \n");	
	PtRealizeWidget(m_windowMain);
	int count = PtModalStart();
	done = 0;
	while(!done) {
		PtProcessEvent();
	}
	PtModalEnd(count);

	_storeWindowData();
	
	PtDestroyWidget(mainWindow);
}

static char *s_get_text_string(PtWidget_t *w) {
	PtArg_t arg;
	char *str = NULL;

	PtSetArg(&arg, Pt_ARG_TEXT_STRING, &str, 0);
	PtGetResources(w, 1, &arg);

	return str;
}

void AP_QNXDialog_Replace::event_Find(void)
{
	char * findEntryText = NULL;

	if (!(findEntryText = s_get_text_string(m_entryFind))) {
		return;
	}
	printf("Find [%s] \n", findEntryText);
	
	UT_UCSChar * findString;

	UT_UCS_cloneString_char(&findString, findEntryText);
	
	setFindString(findString);
	
	findNext();

	FREEP(findString);
}
		
void AP_QNXDialog_Replace::event_Replace(void)
{
	char * findEntryText;
	char * replaceEntryText;

	findEntryText = s_get_text_string(m_entryFind);
	replaceEntryText = s_get_text_string(m_entryReplace);
	if (!findEntryText || !replaceEntryText) {
		return;
	}
	printf("Find [%s] Replace w/ [%s] \n", findEntryText, replaceEntryText);
	
	UT_UCSChar * findString;
	UT_UCSChar * replaceString;

	UT_UCS_cloneString_char(&findString, findEntryText);
	UT_UCS_cloneString_char(&replaceString, replaceEntryText);
	
	setFindString(findString);
	setReplaceString(replaceString);
	
	findReplace();

	FREEP(findString);
	FREEP(replaceString);
}

void AP_QNXDialog_Replace::event_ReplaceAll(void)
{
	char * findEntryText;
	char * replaceEntryText;

	findEntryText = s_get_text_string(m_entryFind);
	replaceEntryText = s_get_text_string(m_entryReplace);
	if (!findEntryText || !replaceEntryText) {
		return;
	}
	printf("Find [%s] Replace w/ [%s] \n", findEntryText, replaceEntryText);
		
	UT_UCSChar * findString;
	UT_UCSChar * replaceString;

	UT_UCS_cloneString_char(&findString, findEntryText);
	UT_UCS_cloneString_char(&replaceString, replaceEntryText);
	
	setFindString(findString);
	setReplaceString(replaceString);
	
	findReplaceAll();

	FREEP(findString);
	FREEP(replaceString);
}

void AP_QNXDialog_Replace::event_MatchCaseToggled(void)
{
	//setMatchCase(m_checkbuttonMatchCase));
}

void AP_QNXDialog_Replace::event_Cancel(void)
{
	m_answer = AP_Dialog_Replace::a_CANCEL;
	done = 1;
}

void AP_QNXDialog_Replace::event_WindowDelete(void)
{
	m_answer = AP_Dialog_Replace::a_CANCEL;	
	done = 1;
}

/*****************************************************************/

PtWidget_t * AP_QNXDialog_Replace::_constructWindow(void)
{
	PtWidget_t *windowReplace;
	PtWidget_t *vboxReplace;
	PtWidget_t *tableReplace;
	PtWidget_t *entryFind;
	PtWidget_t *entryReplace;
	PtWidget_t *checkbuttonMatchCase;
	PtWidget_t *labelFind;
	PtWidget_t *labelReplace;
	PtWidget_t *hbuttonbox1;
	PtWidget_t *buttonFindNext;
	PtWidget_t *buttonReplace;
	PtWidget_t *buttonReplaceAll;
	PtWidget_t *buttonCancel;

	PtArg_t args[10];
	PhArea_t area;
	int     width, height, n;


	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	// conditionally set title
	if (m_id == AP_DIALOG_ID_FIND)
		UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_FindTitle));
	else
		UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceTitle));		

	// find is smaller
	if (m_id == AP_DIALOG_ID_FIND) {
		width = 350; height = 90;
	}
	else {
		width = 355; height = 120;
	}
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, height, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, width, 0);
	PtSetArg(&args[n++], Pt_ARG_TITLE, unixstr, 0);
	if (m_id == AP_DIALOG_ID_FIND)
		PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(AP_STRING_ID_DLG_FR_FindTitle), 0);
	else
		PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceTitle), 0);
	windowReplace = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowReplace, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

#define LABEL_WIDTH   90 
#define TEXT_WIDTH    150
#define BUTTON_WIDTH  80
#define GEN_HEIGHT    20
#define GEN_SPACER    10
	// find label is always here
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_FindLabel));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	area.pos.x = GEN_SPACER; area.pos.y = GEN_SPACER;
	area.size.w = LABEL_WIDTH; area.size.h = GEN_HEIGHT;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	labelFind = PtCreateWidget(PtLabel, windowReplace, n, args);
	
	// find entry is always here
	n = 0;
	area.pos.x += area.size.w + GEN_SPACER; 
	area.size.w = TEXT_WIDTH;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	entryFind = PtCreateWidget(PtText, windowReplace, n, args);
	PtAddCallback(entryFind, Pt_CB_ACTIVATE, s_find_entry_activate, this);

	// the replace label and field are only visible if we're a "replace" dialog
	if (m_id == AP_DIALOG_ID_REPLACE)
	{	
		// create replace label
		n = 0;
		UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceWithLabel));	
		PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
		area.pos.x = GEN_SPACER; area.pos.y += area.size.h + GEN_SPACER;
		area.size.w = LABEL_WIDTH; area.size.h = GEN_HEIGHT;
		PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
		labelReplace = PtCreateWidget(PtLabel, windowReplace, n, args);
	
		// create replace entry
		n = 0;
		area.pos.x += area.size.w + GEN_SPACER; 
		area.size.w = TEXT_WIDTH;
		PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
		entryReplace = PtCreateWidget(PtText, windowReplace, n, args);
		PtAddCallback(entryReplace, Pt_CB_ACTIVATE, s_replace_entry_activate, this);

	}
	
	// button box at the side
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
			 Pt_GROUP_EQUAL_SIZE_HORIZONTAL, 
			 Pt_GROUP_EQUAL_SIZE_HORIZONTAL); 	
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	//	PtSetArg(args[n++], Pt_ARG_GROUP_ROWS_COLS, 4, 0);
	area.pos.x = GEN_SPACER + LABEL_WIDTH + GEN_SPACER + TEXT_WIDTH + GEN_SPACER;
	area.pos.y = GEN_SPACER; 
	area.size.w = BUTTON_WIDTH; 
	area.size.h = GEN_HEIGHT; 
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	hbuttonbox1 = PtCreateWidget(PtGroup, windowReplace, n, args);

	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_FindNextButton));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	buttonFindNext = PtCreateWidget(PtButton, hbuttonbox1, n, args);
	PtAddCallback(buttonFindNext, Pt_CB_ACTIVATE, s_find_clicked, this);

	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		n = 0;
		UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceButton));	
		PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
		buttonReplace = PtCreateWidget(PtButton, hbuttonbox1, n, args);
		PtAddCallback(buttonReplace, Pt_CB_ACTIVATE, s_replace_clicked, this);

		n = 0;
		UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceAllButton));	
		PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
		buttonReplaceAll = PtCreateWidget(PtButton, hbuttonbox1, n, args);
		PtAddCallback(buttonReplaceAll, Pt_CB_ACTIVATE, s_replace_all_clicked, this);
	}

	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceAllButton));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	buttonCancel = PtCreateWidget(PtButton, hbuttonbox1, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);
	
	// match case is always here at the bottom
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_FR_MatchCase));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	area.pos.x = GEN_SPACER; 
	area.pos.y = GEN_SPACER + GEN_HEIGHT + GEN_SPACER; 
	if (m_id == AP_DIALOG_ID_REPLACE) {
		area.pos.y += GEN_HEIGHT + GEN_SPACER;
	}
	area.size.w = LABEL_WIDTH + GEN_SPACER + TEXT_WIDTH;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TICK, 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	checkbuttonMatchCase = PtCreateWidget(PtToggleButton, windowReplace, n, args);
	PtAddCallback(checkbuttonMatchCase, Pt_CB_ACTIVATE, s_match_case_toggled, this);

	// save pointers to members
	m_windowMain = windowReplace;

	m_entryFind = entryFind;
	m_entryReplace = entryReplace;
	m_checkbuttonMatchCase = checkbuttonMatchCase;

	m_buttonFindNext = buttonFindNext;
	m_buttonReplace = buttonReplace;
	m_buttonReplaceAll = buttonReplaceAll;
	m_buttonCancel = buttonCancel;

	m_buttonCancel = buttonCancel;

	
	return windowReplace;
}

void AP_QNXDialog_Replace::_populateWindowData(void)
{
#if 0
	UT_ASSERT(m_entryFind && m_checkbuttonMatchCase);

	// last used find string
	{
		UT_UCSChar * bufferUnicode = getFindString();
		char * bufferNormal = (char *) calloc(UT_UCS_strlen(bufferUnicode) + 1, sizeof(char));
		UT_UCS_strcpy_to_char(bufferNormal, bufferUnicode);
		FREEP(bufferUnicode);
		
		gtk_entry_set_text(GTK_ENTRY(m_entryFind), bufferNormal);
		gtk_entry_select_region(GTK_ENTRY(m_entryFind), 0, -1);

		FREEP(bufferNormal);
	}
	
	
	// last used replace string
	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		UT_ASSERT(m_entryReplace);
		
		UT_UCSChar * bufferUnicode = getReplaceString();
		char * bufferNormal = (char *) calloc(UT_UCS_strlen(bufferUnicode) + 1, sizeof(char));
		UT_UCS_strcpy_to_char(bufferNormal, bufferUnicode);
		FREEP(bufferUnicode);
		
		gtk_entry_set_text(GTK_ENTRY(m_entryReplace), bufferNormal);

		FREEP(bufferNormal);
	}

	// match case button
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkbuttonMatchCase), getMatchCase());

	// Find entry should have focus, for immediate typing
	gtk_widget_grab_focus(m_entryFind);	
#endif
}

void AP_QNXDialog_Replace::_storeWindowData(void)
{
	// TODO: nothing?  The actual methods store
	// out last used data to the persist variables,
	// since we need to save state when things actually
	// happen (not when the dialog closes).
}
