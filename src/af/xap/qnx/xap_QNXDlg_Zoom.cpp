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
#include "gr_QNXGraphics.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_Zoom.h"
#include "xap_QNXDlg_Zoom.h"
#include "ut_qnxHelper.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * XAP_QNXDialog_Zoom::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	XAP_QNXDialog_Zoom * p = new XAP_QNXDialog_Zoom(pFactory,id);
	return p;
}

XAP_QNXDialog_Zoom::XAP_QNXDialog_Zoom(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: XAP_Dialog_Zoom(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_qnxGraphics = NULL;
	
	m_buttonOK = NULL;
	m_buttonCancel = NULL;

	m_previewFrame = 	NULL;
	m_previewArea = 	NULL;
	
	m_radio200 = 		NULL;
	m_radio100 = 		NULL;
	m_radio75 = 		NULL;
	m_radioPageWidth = 	NULL;
	m_radioWholePage = 	NULL;
	m_radioPercent = 	NULL;

	m_spinPercent = NULL;

	m_radioGroup = NULL;
}

XAP_QNXDialog_Zoom::~XAP_QNXDialog_Zoom(void)
{
	DELETEP(m_qnxGraphics);
}

/*****************************************************************/
static int s_ok_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_OK();
	return Pt_CONTINUE;
}
static int s_cancel_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_Cancel();
	return Pt_CONTINUE;
}
static int s_delete_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}
static int s_radio_200_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_Radio200Clicked();
	return Pt_CONTINUE;
}
static int s_radio_100_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_Radio100Clicked();
	return Pt_CONTINUE;
}
static int s_radio_75_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_Radio75Clicked();
	return Pt_CONTINUE;
}
static int s_radio_PageWidth_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_RadioPageWidthClicked();
	return Pt_CONTINUE;
}
static int s_radio_WholePage_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_RadioWholePageClicked();
	return Pt_CONTINUE;
}
static int s_radio_Percent_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_RadioPercentClicked();
	return Pt_CONTINUE;
}
static int s_spin_Percent_changed(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	printf("Spin changed! \n");
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_SpinPercentChanged();
	return Pt_CONTINUE;
}

static int s_preview_exposed(PtWidget_t * w, PhTile_t * damage) {
	PtArg_t args[1];
	UT_Rect rClip;

   	PhRect_t rect;
   	PtSuperClassDraw(PtBasic, w, damage);
   	PtBasicWidgetCanvas(w, &rect);
	//clip to our basic canvas (it's only polite).
    PtClipAdd( w, &rect );

	XAP_QNXDialog_Zoom *pQNXDlg, **ppQNXDlg = NULL;
	PtSetArg(&args[0], Pt_ARG_USER_DATA, &ppQNXDlg, 0);
	PtGetResources(w, 1, args);
	pQNXDlg = (ppQNXDlg) ? *ppQNXDlg : NULL;

	UT_ASSERT(pQNXDlg);
	pQNXDlg->event_PreviewAreaExposed();

    PtClipRemove();
	return Pt_CONTINUE;
}
	

/*****************************************************************/

void XAP_QNXDialog_Zoom::runModal(XAP_Frame * pFrame)
{
	// Set the parent window for this dialog 
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	PtSetParentWidget(parentWindow);

	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	
	m_qnxGraphics = new GR_QNXGraphics(mainWindow, m_previewArea);
	PtArg_t args[2];
	PtExtentWidget(mainWindow);	
	PtExtentWidget(m_previewArea);	
	PtSetArg(&args[0], Pt_ARG_WIDTH, 0, 0);
	PtSetArg(&args[1], Pt_ARG_HEIGHT, 0, 0);
	PtGetResources(m_previewArea, 2, args);
	printf("Width %d height %d \n", args[0].value, args[1].value);
	_createPreviewFromGC(m_qnxGraphics,
						 args[0].value,	/* Width */
						 args[1].value);/* Height */

	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);
	PtRealizeWidget(mainWindow);
	PgFlush();
	
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

void XAP_QNXDialog_Zoom::event_OK(void)
{
	if (!done++) {
		m_answer = XAP_Dialog_Zoom::a_OK;
	}
}

void XAP_QNXDialog_Zoom::event_Cancel(void)
{
	if (!done++) {
		m_answer = XAP_Dialog_Zoom::a_CANCEL;
	}
}

void XAP_QNXDialog_Zoom::event_WindowDelete(void)
{
	if (!done++) {
		m_answer = XAP_Dialog_Zoom::a_CANCEL;	
	}
}

void XAP_QNXDialog_Zoom::event_Radio200Clicked(void)
{
	//m_zoomType = XAP_Dialog_Zoom::zoomType::z_200;
	m_zoomType = z_200;
	_enablePercentSpin(UT_FALSE);
	_updatePreviewZoomPercent(200);
	PtDamageWidget(m_previewArea);
}

void XAP_QNXDialog_Zoom::event_Radio100Clicked(void)
{
	//m_zoomType = XAP_Dialog_Zoom::zoomType::z_100;
	m_zoomType = z_100;
	_enablePercentSpin(UT_FALSE);
	_updatePreviewZoomPercent(100);
	PtDamageWidget(m_previewArea);
}

void XAP_QNXDialog_Zoom::event_Radio75Clicked(void)
{
	m_zoomType = z_75;
	_enablePercentSpin(UT_FALSE);
	_updatePreviewZoomPercent(75);
	PtDamageWidget(m_previewArea);
}

void XAP_QNXDialog_Zoom::event_RadioPageWidthClicked(void)
{
	m_zoomType = z_PAGEWIDTH;
	_enablePercentSpin(UT_FALSE);
	// TODO : figure out the dimensions
	PtDamageWidget(m_previewArea);
}

void XAP_QNXDialog_Zoom::event_RadioWholePageClicked(void)
{
	m_zoomType = z_WHOLEPAGE;
	_enablePercentSpin(UT_FALSE);
	// TODO : figure out the dimensions
	PtDamageWidget(m_previewArea);
}

void XAP_QNXDialog_Zoom::event_RadioPercentClicked(void)
{
	m_zoomType = z_PERCENT;
	_enablePercentSpin(UT_TRUE);
	// call event_SpinPercentChanged() to do the fetch and update work
	event_SpinPercentChanged();
	PtDamageWidget(m_previewArea);
}

static int get_numeric_value(PtWidget_t *w) {
	PtArg_t arg;
	int     *value;

	PtSetArg(&arg, Pt_ARG_NUMERIC_VALUE, &value, 0);
	PtGetResources(w, 1, &arg);
	return *value;
}

void XAP_QNXDialog_Zoom::event_SpinPercentChanged(void)
{
	m_zoomPercent = get_numeric_value(m_spinPercent);
	_updatePreviewZoomPercent(m_zoomPercent);
}

void XAP_QNXDialog_Zoom::event_PreviewAreaExposed(void)
{
	UT_ASSERT(m_zoomPreview);

    // trigger a draw on the preview area in the base class
	m_zoomPreview->draw();

	//Something isn't right here ...
	PtDamageWidget(m_buttonOK);
	PtDamageWidget(m_buttonCancel);
}

/*****************************************************************/
PtWidget_t * XAP_QNXDialog_Zoom::_constructWindow(void)
{
	PtWidget_t * windowZoom;

	PtWidget_t * hboxFrames;	//Radio on the left, Preview on Right
	PtWidget_t * vboxZoomTo;	//Radio buttons
	PtWidget_t * vboxZoomPreview;	//Preview

	PtWidget_t * radiobutton200;
	PtWidget_t * radiobutton100;
	PtWidget_t * radiobutton75;
	PtWidget_t * radiobuttonPageWidth;
	PtWidget_t * radiobuttonWholePage;
	PtWidget_t * radiobuttonPercent;
	PtWidget_t * spinbuttonPercent_adj;
	PtWidget_t * spinbuttonPercent;

	PtWidget_t * frameSampleText;
	PtWidget_t * drawingareaPreview;

	PtWidget_t * hbuttonboxZoom;
	PtWidget_t * buttonOK;
	PtWidget_t * buttonCancel;
	PtArg_t		args[10];
	int			n;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	char  *unixstr;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(XAP_STRING_ID_DLG_Zoom_ZoomTitle), 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);

	windowZoom = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowZoom, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	n = 0;
#define MARGIN_SIZE 10 
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_HORIZONTAL, Pt_GROUP_HORIZONTAL); 
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, MARGIN_SIZE, 0); 
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, MARGIN_SIZE, 0); 
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_X, MARGIN_SIZE, 0); 
	hboxFrames = PtCreateWidget(PtGroup, windowZoom, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, Pt_GROUP_VERTICAL); 
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
				Pt_GROUP_EXCLUSIVE | Pt_GROUP_EQUAL_SIZE_HORIZONTAL, 
				Pt_GROUP_EXCLUSIVE | Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Zoom To", 0); 
	vboxZoomTo = PtCreateWidget(PtGroup, hboxFrames, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, Pt_GROUP_VERTICAL); 
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Preview", 0); 
	vboxZoomPreview = PtCreateWidget(PtGroup, hboxFrames, n, args);

	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_RadioFrameCaption));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxZoomTo, n, args);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_200));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	radiobutton200 = PtCreateWidget(PtToggleButton, vboxZoomTo, n, args);
	PtAddCallback(radiobutton200, Pt_CB_ACTIVATE, s_radio_200_clicked, this);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_100));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0); 
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	radiobutton100 = PtCreateWidget(PtToggleButton, vboxZoomTo, n, args);
	PtAddCallback(radiobutton100, Pt_CB_ACTIVATE, s_radio_100_clicked, this);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_75));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0); 
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	radiobutton75 = PtCreateWidget(PtToggleButton, vboxZoomTo, n, args);
	PtAddCallback(radiobutton75, Pt_CB_ACTIVATE, s_radio_75_clicked, this);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_PageWidth));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	radiobuttonPageWidth = PtCreateWidget(PtToggleButton, vboxZoomTo, n, args);
	PtAddCallback(radiobuttonPageWidth, Pt_CB_ACTIVATE, s_radio_PageWidth_clicked, this);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_WholePage));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	radiobuttonWholePage = PtCreateWidget(PtToggleButton, vboxZoomTo, n, args);
	PtAddCallback(radiobuttonWholePage, Pt_CB_ACTIVATE, s_radio_WholePage_clicked, this);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_Percent));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	radiobuttonPercent = PtCreateWidget(PtToggleButton, vboxZoomTo, n, args);
	PtAddCallback(radiobuttonPercent, Pt_CB_ACTIVATE, s_radio_Percent_clicked, this);
	FREEP(unixstr);

	n = 0;
	//PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_MAX, 500, 0);
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_MIN,   1, 0);
	spinbuttonPercent = PtCreateWidget(PtNumericInteger, vboxZoomTo, n, args);
	PtAddCallback(radiobuttonPercent, Pt_CB_NUMERIC_CHANGED, s_spin_Percent_changed, this);

	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_PreviewFrame));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxZoomPreview, n, args);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "10 pt Times New Roman", 0);
	frameSampleText = PtCreateWidget(PtLabel, vboxZoomPreview, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 150, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 150, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_HORIZONTAL, Pt_GROUP_HORIZONTAL); 
#define _VS_ANCHOR_ (Pt_LEFT_ANCHORED_RIGHT | Pt_RIGHT_ANCHORED_RIGHT | \
		     Pt_TOP_ANCHORED_TOP | Pt_BOTTOM_ANCHORED_BOTTOM)
	PtSetArg(&args[n++], Pt_ARG_ANCHOR_FLAGS, _VS_ANCHOR_, _VS_ANCHOR_); 
#define _VS_STRETCH_ (Pt_GROUP_STRETCH_HORIZONTAL | Pt_GROUP_STRETCH_VERTICAL)
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, _VS_STRETCH_, _VS_STRETCH_); 
	PtWidget_t *rawgroup = PtCreateWidget(PtGroup, vboxZoomPreview, n, args);

	n = 0;
	void *data = (void *)this;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 150, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 150, 0);
	PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this)); 
	PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &s_preview_exposed, 1); 
	drawingareaPreview = PtCreateWidget(PtRaw, rawgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_HORIZONTAL, Pt_GROUP_HORIZONTAL); 
	PtWidget_t *vboxButtons = PtCreateWidget(PtGroup, vboxZoomPreview, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	buttonOK = PtCreateWidget(PtButton, vboxButtons, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	buttonCancel = PtCreateWidget(PtButton, vboxButtons, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	m_windowMain = windowZoom;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

	m_previewFrame = 	frameSampleText;
	m_previewArea = 	drawingareaPreview;
	
	m_radio200 = 		radiobutton200;
	m_radio100 = 		radiobutton100;
	m_radio75 = 		radiobutton75;
	m_radioPageWidth = 	radiobuttonPageWidth;
	m_radioWholePage = 	radiobuttonWholePage;
	m_radioPercent = 	radiobuttonPercent;

	m_spinPercent = spinbuttonPercent;

	m_radioGroup = vboxZoomTo;
	
	return windowZoom;
}

void XAP_QNXDialog_Zoom::_enablePercentSpin(UT_Bool enable) {
	PtArg_t arg;
	if (enable == UT_FALSE) {
		PtSetArg(&arg, Pt_ARG_FLAGS, Pt_BLOCKED | Pt_GHOST, Pt_BLOCKED | Pt_GHOST);	
		PtSetResources(m_spinPercent, 1, &arg);
	}
	else {
		PtSetArg(&arg, Pt_ARG_FLAGS, 0, Pt_BLOCKED | Pt_GHOST);	
		PtSetResources(m_spinPercent, 1, &arg);
		PtWidgetShowFocus(m_spinPercent);
	}
}

static void set_toggle_button(PtWidget_t *w, int enable) {
	PtArg_t arg;
	int     *value;

	PtSetArg(&arg, Pt_ARG_FLAGS, (enable) ? Pt_SET : 0, Pt_SET);
	PtSetResources(w, 1, &arg);
}

static void set_numeric_value(PtWidget_t *w, int value) {
	PtArg_t arg;

	PtSetArg(&arg, Pt_ARG_NUMERIC_VALUE, value, 0);
	PtSetResources(w, 1, &arg);
}
void XAP_QNXDialog_Zoom::_populateWindowData(void)
{
	// The callbacks for these radio buttons aren't always
	// called when the dialog is being constructed, so we have to
	// set the widget's value, then manually enable/disable
	// the spin button.
	
	// enable the right button
	_enablePercentSpin(UT_FALSE);	// default
	switch(getZoomType())
	{
	case XAP_QNXDialog_Zoom::z_200:
		set_toggle_button(m_radio200, UT_TRUE);
		_updatePreviewZoomPercent(200);
		break;
	case XAP_QNXDialog_Zoom::z_100:
		set_toggle_button(m_radio100, UT_TRUE);
		_updatePreviewZoomPercent(100);		
		break;
	case XAP_QNXDialog_Zoom::z_75:
		set_toggle_button(m_radio75, UT_TRUE);
		_updatePreviewZoomPercent(75);
		break;
	case XAP_QNXDialog_Zoom::z_PAGEWIDTH:
		set_toggle_button(m_radioPageWidth, UT_TRUE);
		break;
	case XAP_QNXDialog_Zoom::z_WHOLEPAGE:
		set_toggle_button(m_radioWholePage, UT_TRUE);
		break;
	case XAP_QNXDialog_Zoom::z_PERCENT:
		set_toggle_button(m_radioPercent, UT_TRUE);
		_enablePercentSpin(UT_TRUE);	// override
		_updatePreviewZoomPercent(getZoomPercent());
		break;
	default:
		// if they haven't set anything yet, default to the 100% radio item
		set_toggle_button(m_radio100, UT_TRUE);		
	}
	
	set_numeric_value(m_spinPercent, (int)getZoomPercent());
}

void XAP_QNXDialog_Zoom::_storeWindowData(void)
{
#if 0
	for (GSList * item = m_radioGroup; item ; item = item->next)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(item->data)))
		{
			m_zoomType = (XAP_Dialog_Zoom::zoomType)
				GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(item->data), WIDGET_ID_TAG_KEY));
			break;
		}
	}

	// store away percentage; the base class decides if it's important when
	// the caller requests the percent
	m_zoomPercent = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_spinPercent));
#endif
}

	
