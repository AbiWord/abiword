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
#include <stdio.h>
#include <string.h>

// This header defines some functions for QNX dialogs,
// like centering them, measuring them, etc.
#include "gr_QNXGraphics.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Dialog_Id.h"

#include "ap_Strings.h"

#include "ap_Preview_Paragraph.h"
#include "ap_QNXDialog_Paragraph.h"
#include "ut_qnxHelper.h"

/*****************************************************************/
/* Why aren't these Photon calls? */
/* For combo boxes */
/* For normal strings */
void TFSetTextString(PtWidget_t *cb, char *str) {
	PtArg_t arg;
	PtSetArg(&arg, Pt_ARG_TEXT_STRING, str, 0);
	PtSetResources(cb, 1, &arg);
}
char *TFGetTextString(PtWidget_t *cb) {
	PtArg_t arg;
	char 	*str = NULL;
	PtSetArg(&arg, Pt_ARG_TEXT_STRING, &str, 0);
	PtGetResources(cb, 1, &arg);
	return str;
}

/* For integer controls */
void TFSetTextStringInt(PtWidget_t *sp, char *str) {
	char	*post;
	PtArg_t arg[2];
	int     n = 0;
	PtSetArg(&arg[n++], Pt_ARG_NUMERIC_VALUE, strtoul(str, &post, 10), 0);
	if (post && *post) {
		PtSetArg(&arg[n++], Pt_ARG_NUMERIC_SUFFIX, post, 0);
	}
	PtSetResources(sp, n, arg);
}
void TFSetTextStringFloat(PtWidget_t *sp, char *str) {
	char	*post;
	double  d;
	PtArg_t arg[2];
	int     n = 0;
	
	d = strtod(str, &post);	
	PtSetArg(&arg[n++], Pt_ARG_NUMERIC_VALUE, &d, 0);
	if (post && *post) {
		PtSetArg(&arg[n++], Pt_ARG_NUMERIC_SUFFIX, post, 0);
	}
	PtSetResources(sp, n, arg);
}
int TFGetNumericString(PtWidget_t *sp, char *buffer, unsigned int len) {
	PtArg_t arg;
	char   *str;

	strcpy(buffer, "");

	str = NULL;
	PtSetArg(&arg, Pt_ARG_NUMERIC_PREFIX, &str, 0);
	PtGetResources(sp, 1, &arg);
	if (str && strlen(str) < len) {
		strcpy(buffer, str);	
	}

	str = NULL;
	PtSetArg(&arg, Pt_ARG_TEXT_STRING, &str, 0);
	PtGetResources(sp, 1, &arg);
	if (str && strlen(buffer) + strlen(str) < len) {
		strcat(buffer, str);	
	}

	str = NULL;
	PtSetArg(&arg, Pt_ARG_NUMERIC_SUFFIX, &str, 0);
	PtGetResources(sp, 1, &arg);
	if (str && strlen(buffer) + strlen(str) < len) {
		strcat(buffer, str);	
	}
	return 0;
}

/* For toggle buttons */
void TFToggleSetState(PtWidget_t *toggle, int on) {
	PtArg_t arg;
	PtSetArg(&arg, Pt_ARG_FLAGS, (on) ? Pt_SET : 0, Pt_SET);
	PtSetResources(toggle, 1, &arg);
}
int TFToggleGetState(PtWidget_t *toggle) {
	PtArg_t arg;
	int     *flags = NULL;
	PtSetArg(&arg, Pt_ARG_FLAGS, &flags, 0);
	PtGetResources(toggle, 1, &arg);
	UT_ASSERT(flags);
	return ((*flags & Pt_SET) == Pt_SET);
}

/* For setting/getting data */
void TFSetData(PtWidget_t *widget, void *data, int len) {
	PtArg_t arg;
	PtSetArg(&arg, Pt_ARG_USER_DATA, data, len);
	PtSetResources(widget, 1, &arg);
}
void TFSetDataInt(PtWidget_t *widget, int value) {
	TFSetData(widget, &value, sizeof(value));
}
void *TFGetData(PtWidget_t *widget) {
	void    *data;
	PtArg_t arg;
	PtSetArg(&arg, Pt_ARG_USER_DATA, &data, 0);
	PtGetResources(widget, 1, &arg);
	return data;
}
int TFGetDataInt(PtWidget_t *widget) {
	int *id;
	id = (int *)TFGetData(widget);
	return *id;
}

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Paragraph::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	AP_QNXDialog_Paragraph * p = new AP_QNXDialog_Paragraph(pFactory,id);
	return p;
}

AP_QNXDialog_Paragraph::AP_QNXDialog_Paragraph(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_Paragraph(pDlgFactory,id)
{
	m_unixGraphics = NULL;
	m_bEditChanged = false;
}

AP_QNXDialog_Paragraph::~AP_QNXDialog_Paragraph(void)
{
	DELETEP(m_unixGraphics);
}

/*****************************************************************/
/* These are static callbacks for dialog widgets                 */
/*****************************************************************/

static int s_ok_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{ UT_ASSERT(widget && data); AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data; dlg->event_OK(); return Pt_CONTINUE; }

static int s_cancel_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t * info)
{ UT_ASSERT(widget && data); AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data; dlg->event_Cancel(); return Pt_CONTINUE; }

static int s_tabs_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t * info) 
{ UT_ASSERT(widget && data); AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data;	dlg->event_Tabs(widget, info); return Pt_CONTINUE; }

static int s_delete_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t * info)
{ UT_ASSERT(data);  AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data; dlg->event_WindowDelete(); return Pt_CONTINUE; }

static int s_spin_changed(PtWidget_t * widget, void *data, PtCallbackInfo_t * info)
{
	// notify the dialog that an edit has changed
	AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data;
	dlg->event_SpinChanged(widget);
	return Pt_CONTINUE;
}

static int s_menu_item_activate(PtWidget_t * widget, void *data, PtCallbackInfo_t * info)
{
	UT_ASSERT(widget && data);
	
	AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data;
	PtListCallback_t *linfo = (PtListCallback_t *)info->cbdata;

	dlg->event_MenuChanged(widget, linfo->item_pos - 1);
	return Pt_CONTINUE;
}

static int s_check_toggled(PtWidget_t * widget, void *data, PtCallbackInfo_t * info)
{
	UT_ASSERT(widget && data);
	AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data;
	dlg->event_CheckToggled(widget);
	return Pt_CONTINUE;
}

static int s_preview_exposed(PtWidget_t * w, PhTile_t * damage) 
{
	PtArg_t args[1];

   	PhRect_t rect;
   	PtSuperClassDraw(PtBasic, w, damage);
   	PtBasicWidgetCanvas(w, &rect);
	//clip to our basic canvas (it's only polite).
    PtClipAdd( w, &rect );

	AP_QNXDialog_Paragraph *pQNXDlg, **ppQNXDlg = NULL;
	PtSetArg(&args[0], Pt_ARG_USER_DATA, &ppQNXDlg, 0);
	PtGetResources(w, 1, args);
	pQNXDlg = (ppQNXDlg) ? *ppQNXDlg : NULL;

	UT_ASSERT(pQNXDlg);
	pQNXDlg->event_PreviewAreaExposed();

    PtClipRemove();
	return Pt_CONTINUE;
}

/*****************************************************************/

void AP_QNXDialog_Paragraph::runModal(XAP_Frame * pFrame)
{
	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the GtkWindow of the parent frame
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	PtSetParentWidget(parentWindow);
	
	m_pFrame = pFrame;
	
	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();

	// Attach signals (after data settings, so we don't trigger
	// updates yet)
	_connectCallbackSignals();

	// *** this is how we add the gc ***
	{
		// attach a new graphics context to the drawing area
		XAP_QNXApp * qnxapp;
		qnxapp = static_cast<XAP_QNXApp *> (m_pApp);
		UT_ASSERT(qnxapp);

		UT_ASSERT(m_drawingareaPreview);

		// make a new QNX GC
		m_qnxGraphics = new GR_QNXGraphics(mainWindow, m_drawingareaPreview, pFrame->getApp());
		unsigned short w, h;

		// let the widget materialize
		UT_QNXGetWidgetArea(m_drawingareaPreview, NULL, NULL, &w, &h);
		_createPreviewFromGC(m_qnxGraphics, w, h);

	}

	// sync all controls once to get started
	// HACK: the first arg gets ignored
	//_syncControls(id_MENU_ALIGNMENT, true);

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

/*****************************************************************/

void AP_QNXDialog_Paragraph::event_OK(void)
{
	m_answer = AP_Dialog_Paragraph::a_OK;
	done = 1;
}

void AP_QNXDialog_Paragraph::event_Cancel(void)
{
	m_answer = AP_Dialog_Paragraph::a_CANCEL;
	done = 1;
}

void AP_QNXDialog_Paragraph::event_Tabs(PtWidget_t *widget, PtCallbackInfo_t *info)
{
	//Re-parent the preview dialog from the first tab
	PtPanelGroupCallback_t *cb = (PtPanelGroupCallback_t *)info->cbdata;

	PtWidget_t *newpanel = PtPGFindPanelByIndex(widget, cb->new_panel_index);
	UT_ASSERT(newpanel);

	PtReparentWidget(m_drawingareaPreviewGroup, newpanel);

/*
	m_answer = AP_Dialog_Paragraph::a_TABS;
	done = 1;
*/
}

void AP_QNXDialog_Paragraph::event_WindowDelete(void)
{
	if (!done) {
		m_answer = AP_Dialog_Paragraph::a_CANCEL;	
	}
	done = 1;
}

void AP_QNXDialog_Paragraph::event_MenuChanged(PtWidget_t * widget, int value)
{
	UT_ASSERT(widget);

	tControl id = (tControl) TFGetDataInt(widget); 

	_setMenuItemValue(id, value);
}

void AP_QNXDialog_Paragraph::event_SpinIncrement(PtWidget_t * widget)
{
	UT_ASSERT(widget);
}

void AP_QNXDialog_Paragraph::event_SpinDecrement(PtWidget_t * widget)
{
	UT_ASSERT(widget);
}

void AP_QNXDialog_Paragraph::event_SpinFocusOut(PtWidget_t * widget)
{
	/*** NOT USED ***/
}

void AP_QNXDialog_Paragraph::event_SpinChanged(PtWidget_t * widget)
{
	char	 buffer[100];
	tControl id = (tControl) TFGetDataInt(widget); 

	m_bEditChanged = true;
	
	TFGetNumericString(widget, buffer, 100);
	//_setSpinItemValue(id, buffer, op_SYNC);
	_setSpinItemValue(id, buffer);
}
	   
void AP_QNXDialog_Paragraph::event_CheckToggled(PtWidget_t * widget)
{
	UT_ASSERT(widget);

	tControl id = (tControl) TFGetDataInt(widget); 
	tCheckState cs;

	// TODO : handle tri-state boxes !!!
	if (TFToggleGetState(widget)) 
		cs = check_TRUE;
	else
		cs = check_FALSE;

	_setCheckItemValue(id, cs, op_SYNC);
}

void AP_QNXDialog_Paragraph::event_PreviewAreaExposed(void)
{
	if (m_paragraphPreview)
		m_paragraphPreview->draw();
}

/*****************************************************************/
PtWidget_t * AP_QNXDialog_Paragraph::_constructWindow(void)
{
	// grab the string set
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	PtWidget_t * windowParagraph;
	PtWidget_t * listAlignment;
	PtWidget_t * spinbuttonLeft;
	PtWidget_t * spinbuttonRight;
	PtWidget_t * listSpecial;
	PtWidget_t * spinbuttonBy;
	PtWidget_t * spinbuttonBefore;
	PtWidget_t * spinbuttonAfter;
	PtWidget_t * listLineSpacing;
	PtWidget_t * spinbuttonAt;
	PtWidget_t * labelAlignment;
	PtWidget_t * labelBy;
	PtWidget_t * labelIndentation;
	PtWidget_t * labelLeft;
	PtWidget_t * labelRight;
	PtWidget_t * labelSpecial;
	PtWidget_t * labelAfter;
	PtWidget_t * labelLineSpacing;
	PtWidget_t * labelAt;
	PtWidget_t * labelPreview;

	PtWidget_t * drawingareaPreview;
	PtWidget_t * drawingareaPreviewGroup;

	PtWidget_t * labelBefore;
	PtWidget_t * labelPagination;
	PtWidget_t * labelPreview2;
	PtWidget_t * checkbuttonWindowOrphan;
	PtWidget_t * checkbuttonKeepLines;
	PtWidget_t * checkbuttonPagebreak;
	PtWidget_t * checkbuttonSuppress;
	PtWidget_t * checkbuttonHyphenate;
	PtWidget_t * checkbuttonKeepNext;
	PtWidget_t * buttonTabs;
	PtWidget_t * buttonOK;
	PtWidget_t * buttonCancel;
	PtWidget_t * panelGroup;

	XML_Char * unixstr = NULL;
	const char *litem[1];

	litem[0] = unixstr;

	PtArg_t args[10];
	int     n = 0;
	double  inc = 0.01;

	//TODO: Put a vertical group in the window first, set the offset etc.

#define WIN_WIDTH  550	
#define WIN_HEIGHT 400	
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_ParaTitle));
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, unixstr, 0);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	windowParagraph = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowParagraph, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_HORZ_ALIGN, Pt_GROUP_HORZ_CENTER, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 10, 0);
	PtWidget_t *vwindowgroup = PtCreateWidget(PtGroup, windowParagraph, n, args);

	n = 0;
#define PANEL_WIDTH (530)
#define PANEL_HEIGHT (390)
	PtSetArg(&args[n++], Pt_ARG_WIDTH, PANEL_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, PANEL_HEIGHT, 0);
	panelGroup = PtCreateWidget(PtPanelGroup, vwindowgroup, n, args);	
	PtAddCallback(panelGroup, Pt_CB_PG_PANEL_SWITCHING, s_tabs_clicked, this);

	/* Create the Indent Tab */
	/* Code Fragment generated by PhAB200 */
	{
	static const PhArea_t area1 = { { 12, 13 }, { 534, 351 } };
	static const PtArg_t args1[] = {
		Pt_ARG( Pt_ARG_AREA, &area1, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BORDER_WIDTH, 1, 0 ),
		Pt_ARG( Pt_ARG_TITLE, "Indents and Spacing", 0 ),
		};

	static const PhArea_t area2 = { { 5, 43 }, { 79, 20 } };
	static const PtArg_t args2[] = {
		Pt_ARG( Pt_ARG_AREA, &area2, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Indentation", 0 ),
		Pt_ARG( Pt_ARG_TEXT_FONT, "TextFont10", 0 ),
		};

	static const PhArea_t area3 = { { 92, 47 }, { 429, 12 } };
	static const PtArg_t args3[] = {
		Pt_ARG( Pt_ARG_AREA, &area3, 0 ),
		};

	static const PhArea_t area4 = { { 91, 12 }, { 104, 26 } };
	static const PtArg_t args4[] = {
		Pt_ARG( Pt_ARG_TEXT_FLAGS, 0, Pt_EDITABLE ),
		Pt_ARG( Pt_ARG_AREA, &area4, 0 ),
		};

	static const PhArea_t area5 = { { 6, 15 }, { 70, 19 } };
	static const PtArg_t args5[] = {
		Pt_ARG( Pt_ARG_AREA, &area5, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Alignment:", 0 ),
		};

	static const PhArea_t area6 = { { 26, 70 }, { 35, 19 } };
	static const PtArg_t args6[] = {
		Pt_ARG( Pt_ARG_AREA, &area6, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Left:", 0 ),
		};

	static const PhArea_t area7 = { { 26, 102 }, { 40, 19 } };
	static const PtArg_t args7[] = {
		Pt_ARG( Pt_ARG_AREA, &area7, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Right:", 0 ),
		};

	static const PhArea_t area8 = { { 91, 68 }, { 87, 24 } };
	static const PtArg_t args8[] = {
		Pt_ARG( Pt_ARG_AREA, &area8, 0 ),
		};

	static const PhArea_t area9 = { { 91, 99 }, { 87, 24 } };
	static const PtArg_t args9[] = {
		Pt_ARG( Pt_ARG_AREA, &area9, 0 ),
		};

	static const PhArea_t area10 = { { 250, 95 }, { 109, 26 } };
	static const PtArg_t args10[] = {
		Pt_ARG( Pt_ARG_TEXT_FLAGS, 0, Pt_EDITABLE ),
		Pt_ARG( Pt_ARG_AREA, &area10, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "(none)", 0 ),
		};

	static const PhArea_t area11 = { { 378, 95 }, { 37, 24 } };
	static const PtArg_t args11[] = {
		Pt_ARG( Pt_ARG_AREA, &area11, 0 ),
		};

	static const PhArea_t area12 = { { 396, 75 }, { 35, 19 } };
	static const PtArg_t args12[] = {
		Pt_ARG( Pt_ARG_AREA, &area12, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "By:", 0 ),
		};

	static const PhArea_t area13 = { { 248, 73 }, { 51, 19 } };
	static const PtArg_t args13[] = {
		Pt_ARG( Pt_ARG_AREA, &area13, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Special:", 0 ),
		};

	static const PhArea_t area14 = { { 64, 138 }, { 455, 12 } };
	static const PtArg_t args14[] = {
		Pt_ARG( Pt_ARG_AREA, &area14, 0 ),
		};

	static const PhArea_t area15 = { { 3, 134 }, { 76, 20 } };
	static const PtArg_t args15[] = {
		Pt_ARG( Pt_ARG_AREA, &area15, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Spacing", 0 ),
		Pt_ARG( Pt_ARG_TEXT_FONT, "TextFont10", 0 ),
		};

	static const PhArea_t area16 = { { 248, 165 }, { 84, 19 } };
	static const PtArg_t args16[] = {
		Pt_ARG( Pt_ARG_AREA, &area16, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Line spacing:", 0 ),
		};

	static const PhArea_t area17 = { { 396, 167 }, { 35, 19 } };
	static const PtArg_t args17[] = {
		Pt_ARG( Pt_ARG_AREA, &area17, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "At:", 0 ),
		};

	static const PhArea_t area18 = { { 91, 160 }, { 87, 24 } };
	static const PtArg_t args18[] = {
		Pt_ARG( Pt_ARG_AREA, &area18, 0 ),
		};

	static const PhArea_t area19 = { { 26, 162 }, { 56, 19 } };
	static const PtArg_t args19[] = {
		Pt_ARG( Pt_ARG_AREA, &area19, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Before:", 0 ),
		};

	static const PhArea_t area20 = { { 378, 188 }, { 37, 24 } };
	static const PtArg_t args20[] = {
		Pt_ARG( Pt_ARG_AREA, &area20, 0 ),
		};

	static const PhArea_t area21 = { { 250, 188 }, { 109, 26 } };
	static const PtArg_t args21[] = {
		Pt_ARG( Pt_ARG_TEXT_FLAGS, 0, Pt_EDITABLE ),
		Pt_ARG( Pt_ARG_AREA, &area21, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "(none)", 0 ),
		};

	static const PhArea_t area22 = { { 91, 192 }, { 87, 24 } };
	static const PtArg_t args22[] = {
		Pt_ARG( Pt_ARG_AREA, &area22, 0 ),
		};

	static const PhArea_t area23 = { { 26, 195 }, { 42, 19 } };
	static const PtArg_t args23[] = {
		Pt_ARG( Pt_ARG_AREA, &area23, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "After:", 0 ),
		};

	static const PhArea_t area24 = { { 65, 223 }, { 455, 12 } };
	static const PtArg_t args24[] = {
		Pt_ARG( Pt_ARG_AREA, &area24, 0 ),
		};

	static const PhArea_t area25 = { { 4, 219 }, { 76, 20 } };
	static const PtArg_t args25[] = {
		Pt_ARG( Pt_ARG_AREA, &area25, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Preview", 0 ),
		Pt_ARG( Pt_ARG_TEXT_FONT, "TextFont10", 0 ),
		};

	static const PhArea_t area26 = { { 15, 243 }, { 493, 96 } };
	static const PtArg_t args26[] = {
		Pt_ARG( Pt_ARG_AREA, &area26, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BORDER_WIDTH, 1, 0 ),
		};

	PtCreateWidget( PtPane, panelGroup, sizeof(args1) / sizeof(PtArg_t), args1 );

	labelIndentation = PtCreateWidget( PtLabel, NULL, sizeof(args2) / sizeof(PtArg_t), args2 );

	PtCreateWidget( PtSeparator, NULL, sizeof(args3) / sizeof(PtArg_t), args3 );

	listAlignment = PtCreateWidget( PtComboBox, NULL, sizeof(args4) / sizeof(PtArg_t), args4 );
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignLeft)); litem[0]= unixstr;
	PtListAddItems(listAlignment, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignCentered)); litem[0]= unixstr;
	PtListAddItems(listAlignment, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignRight)); litem[0]= unixstr;
	PtListAddItems(listAlignment, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignJustified)); litem[0]= unixstr;
	PtListAddItems(listAlignment, litem, 1, 0); FREEP(unixstr);
	TFSetDataInt(listAlignment, id_MENU_ALIGNMENT);

	labelAlignment = PtCreateWidget( PtLabel, NULL, sizeof(args5) / sizeof(PtArg_t), args5 );

	labelLeft = PtCreateWidget( PtLabel, NULL, sizeof(args6) / sizeof(PtArg_t), args6 );

	labelRight = PtCreateWidget( PtLabel, NULL, sizeof(args7) / sizeof(PtArg_t), args7 );

	spinbuttonLeft = PtCreateWidget( PtNumericFloat, NULL, sizeof(args8) / sizeof(PtArg_t), args8 );
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_INCREMENT, &inc, 0);
	PtSetResources(spinbuttonLeft, n, args);
	TFSetDataInt(spinbuttonLeft, id_SPIN_LEFT_INDENT);

	spinbuttonRight = PtCreateWidget( PtNumericFloat, NULL, sizeof(args9) / sizeof(PtArg_t), args9 );
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_INCREMENT, &inc, 0);
	PtSetResources(spinbuttonLeft, n, args);
	TFSetDataInt(spinbuttonRight, id_SPIN_RIGHT_INDENT);

	listSpecial = PtCreateWidget( PtComboBox, NULL, sizeof(args10) / sizeof(PtArg_t), args10 );
	PtListDeleteAllItems(listSpecial);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpecialNone)); litem[0]= unixstr;
	PtListAddItems(listSpecial, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpecialFirstLine)); litem[0]= unixstr;
	PtListAddItems(listSpecial, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpecialHanging)); litem[0]= unixstr;
	PtListAddItems(listSpecial, litem, 1, 0); FREEP(unixstr);
	TFSetDataInt(listSpecial, id_MENU_SPECIAL_INDENT);

	spinbuttonBy = PtCreateWidget( PtNumericFloat, NULL, sizeof(args11) / sizeof(PtArg_t), args11 );
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_INCREMENT, &inc, 0);
	PtSetResources(spinbuttonLeft, n, args);
	TFSetDataInt(spinbuttonBy, id_SPIN_SPECIAL_INDENT);

	labelBy = PtCreateWidget( PtLabel, NULL, sizeof(args12) / sizeof(PtArg_t), args12 );

	labelSpecial = PtCreateWidget( PtLabel, NULL, sizeof(args13) / sizeof(PtArg_t), args13 );

	PtCreateWidget( PtSeparator, NULL, sizeof(args14) / sizeof(PtArg_t), args14 );

	PtCreateWidget( PtLabel, NULL, sizeof(args15) / sizeof(PtArg_t), args15 );

	labelLineSpacing = PtCreateWidget( PtLabel, NULL, sizeof(args16) / sizeof(PtArg_t), args16 );

	labelAt = PtCreateWidget( PtLabel, NULL, sizeof(args17) / sizeof(PtArg_t), args17 );

	spinbuttonBefore = PtCreateWidget( PtNumericFloat, NULL, sizeof(args18) / sizeof(PtArg_t), args18 );
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_INCREMENT, &inc, 0);
	PtSetResources(spinbuttonLeft, n, args);
	TFSetDataInt(spinbuttonBefore, id_SPIN_BEFORE_SPACING);

	labelBefore = PtCreateWidget( PtLabel, NULL, sizeof(args19) / sizeof(PtArg_t), args19 );

	//TODO: Make this a numeric float!
	spinbuttonAt = PtCreateWidget( PtNumericFloat, NULL, sizeof(args20) / sizeof(PtArg_t), args20 );
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_INCREMENT, &inc, 0);
	PtSetResources(spinbuttonLeft, n, args);
	TFSetDataInt(spinbuttonAt, id_SPIN_SPECIAL_SPACING);

	listLineSpacing = PtCreateWidget( PtComboBox, NULL, sizeof(args21) / sizeof(PtArg_t), args21 );
	PtListDeleteAllItems(listLineSpacing);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingSingle)); litem[0]= unixstr;
	PtListAddItems(listLineSpacing, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingHalf)); litem[0]= unixstr;
	PtListAddItems(listLineSpacing, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingDouble)); litem[0]= unixstr;
	PtListAddItems(listLineSpacing, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingAtLeast)); litem[0]= unixstr;
	PtListAddItems(listLineSpacing, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingExactly)); litem[0]= unixstr;
	PtListAddItems(listLineSpacing, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingMultiple)); litem[0]= unixstr;
	PtListAddItems(listLineSpacing, litem, 1, 0); FREEP(unixstr);
	TFSetDataInt(listLineSpacing, id_MENU_SPECIAL_SPACING);

	spinbuttonAfter = PtCreateWidget( PtNumericFloat, NULL, sizeof(args22) / sizeof(PtArg_t), args22 );
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_INCREMENT, &inc, 0);
	PtSetResources(spinbuttonLeft, n, args);
	TFSetDataInt(spinbuttonAfter, id_SPIN_AFTER_SPACING);

	labelAfter = PtCreateWidget( PtLabel, NULL, sizeof(args23) / sizeof(PtArg_t), args23 );

	PtCreateWidget( PtSeparator, NULL, sizeof(args24) / sizeof(PtArg_t), args24 );

	labelPreview = PtCreateWidget( PtLabel, NULL, sizeof(args25) / sizeof(PtArg_t), args25 );

	//This preview needs to be put into a group ...
	drawingareaPreviewGroup = PtCreateWidget(PtGroup, NULL, sizeof(args26) / sizeof(PtArg_t), args26);
	drawingareaPreview = PtCreateWidget( PtRaw, drawingareaPreviewGroup, 
										 sizeof(args26) / sizeof(PtArg_t), args26);
	n = 0;
	void *data = (void *)this;
	PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this)); 
	PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &s_preview_exposed, 1); 
	PtSetResources(drawingareaPreview, n, args);


	}
	/* Code Fragment complete */

	/* Create the Break tab */ 
	/* Code Fragment generated by PhAB200 */
	{
	static const PhArea_t area1 = { { 10, 13 }, { 533, 352 } };
	static const PtArg_t args1[] = {
		Pt_ARG( Pt_ARG_AREA, &area1, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BORDER_WIDTH, 1, 0 ),
		Pt_ARG( Pt_ARG_TITLE, "Line and Page Breaks", 0 ),
		};

	static const PhArea_t area2 = { { 6, 8 }, { 79, 20 } };
	static const PtArg_t args2[] = {
		Pt_ARG( Pt_ARG_AREA, &area2, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Pagination", 0 ),
		Pt_ARG( Pt_ARG_TEXT_FONT, "TextFont10", 0 ),
		};

	static const PhArea_t area3 = { { 88, 12 }, { 434, 12 } };
	static const PtArg_t args3[] = {
		Pt_ARG( Pt_ARG_AREA, &area3, 0 ),
		};

	static const PhArea_t area4 = { { 23, 30 }, { 164, 24 } };
	static const PtArg_t args4[] = {
		Pt_ARG( Pt_ARG_AREA, &area4, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Window/Orphan control", 0 ),
		};

	static const PhArea_t area5 = { { 23, 54 }, { 140, 24 } };
	static const PtArg_t args5[] = {
		Pt_ARG( Pt_ARG_AREA, &area5, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Keep lines together", 0 ),
		};

	static const PhArea_t area6 = { { 247, 30 }, { 112, 24 } };
	static const PtArg_t args6[] = {
		Pt_ARG( Pt_ARG_AREA, &area6, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Keep with next", 0 ),
		};

	static const PhArea_t area7 = { { 247, 54 }, { 132, 24 } };
	static const PtArg_t args7[] = {
		Pt_ARG( Pt_ARG_AREA, &area7, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Page break before", 0 ),
		};

	static const PhArea_t area8 = { { 23, 103 }, { 162, 24 } };
	static const PtArg_t args8[] = {
		Pt_ARG( Pt_ARG_AREA, &area8, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Suppress line numbers", 0 ),
		};

	static const PhArea_t area9 = { { 23, 125 }, { 131, 24 } };
	static const PtArg_t args9[] = {
		Pt_ARG( Pt_ARG_AREA, &area9, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Don't hyphenate", 0 ),
		};

	static const PhArea_t area10 = { { 10, 88 }, { 512, 12 } };
	static const PtArg_t args10[] = {
		Pt_ARG( Pt_ARG_AREA, &area10, 0 ),
		};

	static const PhArea_t area11 = { { 65, 223 }, { 455, 12 } };
	static const PtArg_t args11[] = {
		Pt_ARG( Pt_ARG_AREA, &area11, 0 ),
		};

	static const PhArea_t area12 = { { 4, 219 }, { 76, 20 } };
	static const PtArg_t args12[] = {
		Pt_ARG( Pt_ARG_AREA, &area12, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Preview", 0 ),
		Pt_ARG( Pt_ARG_TEXT_FONT, "TextFont10", 0 ),
		};

	PtCreateWidget( PtPane, panelGroup, sizeof(args1) / sizeof(PtArg_t), args1 );

	labelPagination = PtCreateWidget( PtLabel, NULL, sizeof(args2) / sizeof(PtArg_t), args2 );

	PtCreateWidget( PtSeparator, NULL, sizeof(args3) / sizeof(PtArg_t), args3 );

	checkbuttonWindowOrphan = PtCreateWidget( PtToggleButton, NULL, sizeof(args4) / sizeof(PtArg_t), args4 );
	TFSetDataInt(checkbuttonWindowOrphan, id_CHECK_WIDOW_ORPHAN);

	checkbuttonKeepLines = PtCreateWidget( PtToggleButton, NULL, sizeof(args5) / sizeof(PtArg_t), args5 );
	TFSetDataInt(checkbuttonKeepLines, id_CHECK_KEEP_LINES);

	checkbuttonKeepNext = PtCreateWidget( PtToggleButton, NULL, sizeof(args6) / sizeof(PtArg_t), args6 );
	TFSetDataInt(checkbuttonKeepNext, id_CHECK_KEEP_NEXT);

	checkbuttonPagebreak = PtCreateWidget( PtToggleButton, NULL, sizeof(args7) / sizeof(PtArg_t), args7 );
	TFSetDataInt(checkbuttonPagebreak, id_CHECK_PAGE_BREAK);

	checkbuttonSuppress = PtCreateWidget( PtToggleButton, NULL, sizeof(args8) / sizeof(PtArg_t), args8 );
	TFSetDataInt(checkbuttonSuppress, id_CHECK_SUPPRESS);

	checkbuttonHyphenate = PtCreateWidget( PtToggleButton, NULL, sizeof(args9) / sizeof(PtArg_t), args9 );
	TFSetDataInt(checkbuttonHyphenate, id_CHECK_NO_HYPHENATE);

	PtCreateWidget( PtSeparator, NULL, sizeof(args10) / sizeof(PtArg_t), args10 );

	PtCreateWidget( PtSeparator, NULL, sizeof(args11) / sizeof(PtArg_t), args11 );

	labelPreview2 = PtCreateWidget( PtLabel, NULL, sizeof(args12) / sizeof(PtArg_t), args12 );

	//This preview needs to be put into a group ...
#if 0
	PtCreateWidget(PtGroup, NULL, sizeof(args13) / sizeof(PtArg_t), args13);
	drawingareaPreview2 = PtCreateWidget( PtRaw, NULL, sizeof(args13) / sizeof(PtArg_t), args13 );
	n = 0;
	void *data = (void *)this;
	PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this)); 
	PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &s_preview_exposed, 1); 
	PtSetResources(drawingareaPreview2, n, args);
#endif


	}
	/* Code Fragment complete */

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_X, 5, 0);
	PtWidget_t *hbuttongroup = PtCreateWidget(PtGroup, vwindowgroup, n, args);

	/* Now at the bottom add in some buttons ... */
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_ButtonTabs));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	buttonTabs = PtCreateWidget(PtButton, hbuttongroup, n, args);
	FREEP(unixstr);

	/** Add some padding here **/
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 4 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtCreateWidget(PtLabel, hbuttongroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	buttonCancel = PtCreateWidget(PtButton, hbuttongroup, n, args);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_OK));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	buttonOK = PtCreateWidget(PtButton, hbuttongroup, n, args);
	FREEP(unixstr);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowParagraph;

	m_listAlignment = listAlignment;

	m_spinbuttonLeft = spinbuttonLeft;
	
	m_spinbuttonRight = spinbuttonRight;
	m_listSpecial = listSpecial;
	m_spinbuttonBy = spinbuttonBy;
	m_spinbuttonBefore = spinbuttonBefore;
	m_spinbuttonAfter = spinbuttonAfter;
	m_listLineSpacing = listLineSpacing;
	m_spinbuttonAt = spinbuttonAt;

	m_drawingareaPreview = drawingareaPreview;
	m_drawingareaPreviewGroup = drawingareaPreviewGroup;

	m_checkbuttonWidowOrphan = checkbuttonWindowOrphan;
	m_checkbuttonKeepLines = checkbuttonKeepLines;
	m_checkbuttonPageBreak = checkbuttonPagebreak;
	m_checkbuttonSuppress = checkbuttonSuppress;
	m_checkbuttonHyphenate = checkbuttonHyphenate;
	m_checkbuttonKeepNext = checkbuttonKeepNext;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;
	m_buttonTabs = buttonTabs;

	return windowParagraph;
}

void AP_QNXDialog_Paragraph::_connectCallbackSignals(void)
{
	PtAddCallback(m_buttonTabs, Pt_CB_ACTIVATE, s_tabs_clicked, this);
	PtAddCallback(m_buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);
	PtAddCallback(m_buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	PtAddCallback(m_listLineSpacing, Pt_CB_SELECTION, s_menu_item_activate, this);
	PtAddCallback(m_listSpecial, Pt_CB_SELECTION, s_menu_item_activate, this);
	PtAddCallback(m_listAlignment, Pt_CB_SELECTION, s_menu_item_activate, this);

	PtAddCallback(m_spinbuttonLeft, Pt_CB_NUMERIC_CHANGED, s_spin_changed, this);
	PtAddCallback(m_spinbuttonRight, Pt_CB_NUMERIC_CHANGED, s_spin_changed, this);
	PtAddCallback(m_spinbuttonBy, Pt_CB_NUMERIC_CHANGED, s_spin_changed, this);
	PtAddCallback(m_spinbuttonBefore, Pt_CB_NUMERIC_CHANGED, s_spin_changed, this);
	PtAddCallback(m_spinbuttonAfter, Pt_CB_NUMERIC_CHANGED, s_spin_changed, this);
	PtAddCallback(m_spinbuttonAt, Pt_CB_NUMERIC_CHANGED, s_spin_changed, this);
	
	// all the checkbuttons
	PtAddCallback(m_checkbuttonWidowOrphan, Pt_CB_ACTIVATE, s_check_toggled, this);
	PtAddCallback(m_checkbuttonKeepLines, Pt_CB_ACTIVATE, s_check_toggled, this);
	PtAddCallback(m_checkbuttonPageBreak, Pt_CB_ACTIVATE, s_check_toggled, this);
	PtAddCallback(m_checkbuttonSuppress, Pt_CB_ACTIVATE, s_check_toggled, this);
	PtAddCallback(m_checkbuttonHyphenate, Pt_CB_ACTIVATE, s_check_toggled, this);
	PtAddCallback(m_checkbuttonKeepNext, Pt_CB_ACTIVATE, s_check_toggled, this);

}

void AP_QNXDialog_Paragraph::_populateWindowData(void)
{
	// alignment option menu 
	UT_ASSERT(m_listAlignment);
	//Photon lists are 1 based
	UT_QNXComboSetPos(m_listAlignment, _getMenuItemValue(id_MENU_ALIGNMENT) + 1);

	// indent and paragraph margins
	UT_ASSERT(m_spinbuttonLeft);
	TFSetTextStringFloat(m_spinbuttonLeft, (char *)_getSpinItemValue(id_SPIN_LEFT_INDENT));

	UT_ASSERT(m_spinbuttonRight);
	TFSetTextStringFloat(m_spinbuttonRight, (char *)_getSpinItemValue(id_SPIN_RIGHT_INDENT));

	UT_ASSERT(m_spinbuttonBy);
	TFSetTextStringFloat(m_spinbuttonBy, (char *)_getSpinItemValue(id_SPIN_SPECIAL_INDENT));

	UT_ASSERT(m_listSpecial);
	UT_QNXComboSetPos(m_listSpecial, _getMenuItemValue(id_MENU_SPECIAL_INDENT) + 1);

	// spacing
	UT_ASSERT(m_spinbuttonLeft);
	TFSetTextStringFloat(m_spinbuttonBefore, (char *)_getSpinItemValue(id_SPIN_BEFORE_SPACING));

	UT_ASSERT(m_spinbuttonRight);
	TFSetTextStringFloat(m_spinbuttonAfter, (char *)_getSpinItemValue(id_SPIN_AFTER_SPACING));

	UT_ASSERT(m_spinbuttonAt);
	TFSetTextStringFloat(m_spinbuttonAt, (char *)_getSpinItemValue(id_SPIN_SPECIAL_SPACING));

	UT_ASSERT(m_listLineSpacing);
	UT_QNXComboSetPos(m_listLineSpacing, _getMenuItemValue(id_MENU_SPECIAL_SPACING) + 1);

	// set the check boxes
	// TODO : handle tri-state boxes !!!
	TFToggleSetState(m_checkbuttonWidowOrphan,
						 (_getCheckItemValue(id_CHECK_WIDOW_ORPHAN) == check_TRUE));

	TFToggleSetState(m_checkbuttonKeepLines,
								 (_getCheckItemValue(id_CHECK_KEEP_LINES) == check_TRUE));
	TFToggleSetState(m_checkbuttonPageBreak,
								 (_getCheckItemValue(id_CHECK_PAGE_BREAK) == check_TRUE));
	TFToggleSetState(m_checkbuttonSuppress,
								 (_getCheckItemValue(id_CHECK_SUPPRESS) == check_TRUE));
	TFToggleSetState(m_checkbuttonHyphenate,
								 (_getCheckItemValue(id_CHECK_NO_HYPHENATE) == check_TRUE));
	TFToggleSetState(m_checkbuttonKeepNext,
								 (_getCheckItemValue(id_CHECK_KEEP_NEXT) == check_TRUE));
}

void AP_QNXDialog_Paragraph::_syncControls(tControl changed, bool bAll /* = false */)
{
	// let parent sync any member variables first
	AP_Dialog_Paragraph::_syncControls(changed, bAll);

	// sync the display

	// 1.  link the "hanging indent by" combo and spinner
	if (bAll || (changed == id_SPIN_SPECIAL_INDENT))
	{
		// typing in the control can change the associated combo
		if (_getMenuItemValue(id_MENU_SPECIAL_INDENT) == indent_FIRSTLINE)
		{
			UT_QNXComboSetPos(m_listSpecial, _getMenuItemValue(id_MENU_SPECIAL_INDENT) + 1);
		}
	}
	if (bAll || (changed == id_MENU_SPECIAL_INDENT))
	{
		switch(_getMenuItemValue(id_MENU_SPECIAL_INDENT))
		{
		case indent_NONE:
			// clear the spin control
			TFSetTextStringFloat(m_spinbuttonBy, "0.0in");
			break;

		default:
			// set the spin control
			TFSetTextStringFloat(m_spinbuttonBy, (char *)_getSpinItemValue(id_SPIN_SPECIAL_INDENT));
			break;
		}
	}

	// 2.  link the "line spacing at" combo and spinner

	if (bAll || (changed == id_SPIN_SPECIAL_SPACING))
	{
		// typing in the control can change the associated combo
		if (_getMenuItemValue(id_MENU_SPECIAL_SPACING) == spacing_MULTIPLE)
		{
			UT_QNXComboSetPos(m_listLineSpacing, _getMenuItemValue(id_MENU_SPECIAL_SPACING) + 1);
		}
	}
	if (bAll || (changed == id_MENU_SPECIAL_SPACING))
	{
		switch(_getMenuItemValue(id_MENU_SPECIAL_SPACING))
		{
		case spacing_SINGLE:
		case spacing_ONEANDHALF:
		case spacing_DOUBLE:
			// clear the spin control
			TFSetTextStringFloat(m_spinbuttonAt, "0");
			break;

		default:
			// set the spin control
			TFSetTextStringFloat(m_spinbuttonAt, (char *)_getSpinItemValue(id_SPIN_SPECIAL_SPACING));
			break;
		}
	}

	// 3.  move results of _doSpin() back to screen

	if (!bAll)
	{
		// spin controls only sync when spun
		switch (changed)
		{
		case id_SPIN_LEFT_INDENT:
			TFSetTextStringFloat(m_spinbuttonLeft, 	(char *)_getSpinItemValue(id_SPIN_LEFT_INDENT));
		case id_SPIN_RIGHT_INDENT:
			TFSetTextStringFloat(m_spinbuttonRight, 	(char *)_getSpinItemValue(id_SPIN_RIGHT_INDENT));
		case id_SPIN_SPECIAL_INDENT:
			TFSetTextStringFloat(m_spinbuttonBy, 		(char *)_getSpinItemValue(id_SPIN_SPECIAL_INDENT));
		case id_SPIN_BEFORE_SPACING:
			TFSetTextStringFloat(m_spinbuttonBefore, 	(char *)_getSpinItemValue(id_SPIN_BEFORE_SPACING));
		case id_SPIN_AFTER_SPACING:
			TFSetTextStringFloat(m_spinbuttonAfter, 	(char *)_getSpinItemValue(id_SPIN_AFTER_SPACING));
		case id_SPIN_SPECIAL_SPACING:
			TFSetTextStringFloat(m_spinbuttonAt, 	(char *)_getSpinItemValue(id_SPIN_SPECIAL_SPACING));
		default:
			break;
		}
	}
}
