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

	m_unixGraphics = NULL;
	
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
	DELETEP(m_unixGraphics);
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
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_SpinPercentChanged();
	return Pt_CONTINUE;
}

static int s_preview_exposed(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_PreviewAreaExposed();

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
	
	
#if 0
	m_qnxGraphics = new GR_QNXGraphics(mainWindow, m_dc);

#endif

	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);
	PtRealizeWidget(mainWindow);
	
	int count = PtModalStart();
	done = 0;
	while(!done) {
		PtProcessEvent();
	}
	PtModalEnd(count);

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
}

void XAP_QNXDialog_Zoom::event_Radio100Clicked(void)
{
	//m_zoomType = XAP_Dialog_Zoom::zoomType::z_100;
	m_zoomType = z_100;
	_enablePercentSpin(UT_FALSE);
	_updatePreviewZoomPercent(100);
}

void XAP_QNXDialog_Zoom::event_Radio75Clicked(void)
{
	m_zoomType = z_75;
	_enablePercentSpin(UT_FALSE);
	_updatePreviewZoomPercent(75);
}

void XAP_QNXDialog_Zoom::event_RadioPageWidthClicked(void)
{
	m_zoomType = z_PAGEWIDTH;
	_enablePercentSpin(UT_FALSE);
	// TODO : figure out the dimensions
}

void XAP_QNXDialog_Zoom::event_RadioWholePageClicked(void)
{
	m_zoomType = z_WHOLEPAGE;
	_enablePercentSpin(UT_FALSE);
	// TODO : figure out the dimensions
}

void XAP_QNXDialog_Zoom::event_RadioPercentClicked(void)
{
	m_zoomType = z_PERCENT;
	_enablePercentSpin(UT_TRUE);
	// call event_SpinPercentChanged() to do the fetch and update work
	event_SpinPercentChanged();
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
#if 0
	_updatePreviewZoomPercent((UT_uint32) gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON(m_spinPercent)));
#endif
}

void XAP_QNXDialog_Zoom::event_PreviewAreaExposed(void)
{
#if 0
	UT_ASSERT(m_zoomPreview);

    // trigger a draw on the preview area in the base class
	m_zoomPreview->draw();
#endif
}

/*****************************************************************/
PtWidget_t * XAP_QNXDialog_Zoom::_constructWindow(void)
{
	PtWidget_t * windowZoom;

	PtWidget_t * vboxZoom;
	PtWidget_t * hboxFrames;
	PtWidget_t * frameZoomTo;
	PtWidget_t * vboxZoomTo;
	PtWidget_t * vboxZoomTo_group = NULL;

	PtWidget_t * radiobutton200;
	PtWidget_t * radiobutton100;
	PtWidget_t * radiobutton75;
	PtWidget_t * radiobuttonPageWidth;
	PtWidget_t * radiobuttonWholePage;
	PtWidget_t * radiobuttonPercent;
	PtWidget_t * spinbuttonPercent_adj;
	PtWidget_t * spinbuttonPercent;

	PtWidget_t * framePreview;
	PtWidget_t * frameSampleText;
	PtWidget_t * drawingareaPreview;

	PtWidget_t * hbuttonboxZoom;
	PtWidget_t * buttonOK;
	PtWidget_t * buttonCancel;
	PtArg_t		args[10];
	int			n;
	PhArea_t	area;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	char  *unixstr;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(XAP_STRING_ID_DLG_Zoom_ZoomTitle), 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, Ph_WM_RENDER_RESIZE);
/*
	UT_QNXGetWidgetArea(PtGetParentWidget(), &area.pos.x, &area.pos.y, NULL, NULL);
	area.pos.x += 10; area.pos.y += 10;
    PtSetArg(&args[n++], Pt_ARG_POS, &area.pos, 0);
*/
	windowZoom = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowZoom, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

#define RADIO_GROUP  	100
#define PREVIEW_GROUP  	200
#define GROUP_HEIGHTS   200
#define BUTTON_WIDTH    80
#define BUTTON_HEIGHT   20
#define V_SPACER		15
#define H_SPACER		15

	n = 0;
	area.pos.x = H_SPACER; area.pos.y = V_SPACER;
	area.size.w = RADIO_GROUP; area.size.h = GROUP_HEIGHTS;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, Pt_GROUP_VERTICAL); 
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
				Pt_GROUP_EXCLUSIVE | Pt_GROUP_EQUAL_SIZE_HORIZONTAL, 
				Pt_GROUP_EXCLUSIVE | Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	//frameZoomTo = gtk_frame_new (pSS->getValue(XAP_STRING_ID_DLG_Zoom_RadioFrameCaption));
	frameZoomTo = PtCreateWidget(PtGroup, windowZoom, n, args);
	//vboxZoomTo = gtk_vbox_new (FALSE, 0);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_200));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	radiobutton200 = PtCreateWidget(PtToggleButton, frameZoomTo, n, args);
	PtAddCallback(radiobutton200, Pt_CB_ACTIVATE, s_radio_200_clicked, this);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_100));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0); 
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	radiobutton100 = PtCreateWidget(PtToggleButton, frameZoomTo, n, args);
	PtAddCallback(radiobutton100, Pt_CB_ACTIVATE, s_radio_100_clicked, this);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_75));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0); 
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	radiobutton75 = PtCreateWidget(PtToggleButton, frameZoomTo, n, args);
	PtAddCallback(radiobutton75, Pt_CB_ACTIVATE, s_radio_75_clicked, this);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_PageWidth));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	radiobuttonPageWidth = PtCreateWidget(PtToggleButton, frameZoomTo, n, args);
	PtAddCallback(radiobuttonPageWidth, Pt_CB_ACTIVATE, s_radio_PageWidth_clicked, this);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_WholePage));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	radiobuttonWholePage = PtCreateWidget(PtToggleButton, frameZoomTo, n, args);
	PtAddCallback(radiobuttonWholePage, Pt_CB_ACTIVATE, s_radio_WholePage_clicked, this);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Zoom_Percent));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_ONE_OF_MANY, 0);
	radiobuttonPercent = PtCreateWidget(PtToggleButton, frameZoomTo, n, args);
	PtAddCallback(radiobuttonPercent, Pt_CB_ACTIVATE, s_radio_Percent_clicked, this);
	FREEP(unixstr);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_MAX, 500, 0);
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_MIN,   1, 0);
	spinbuttonPercent = PtCreateWidget(PtNumericInteger, frameZoomTo, n, args);
	PtAddCallback(radiobuttonPercent, Pt_CB_NUMERIC_CHANGED, s_spin_Percent_changed, this);

	n = 0;
	area.pos.x += area.size.w + H_SPACER; 
	area.size.w = PREVIEW_GROUP; area.size.h = GROUP_HEIGHTS;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, Pt_GROUP_VERTICAL); 
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
						Pt_GROUP_EQUAL_SIZE_HORIZONTAL, 
						Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	//PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Zoom_PreviewFrame), 0);
	framePreview = PtCreateWidget(PtGroup, windowZoom, n, args);
	
	// TODO: do something dynamically here?  How do we set this "sample" font?
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "10 pt Times New Roman", 0);
	frameSampleText = PtCreateWidget(PtLabel, framePreview, n, args);

	// *** This is how we do a preview widget ***
#if 0
	{
		drawingareaPreview = gtk_drawing_area_new ();
		gtk_object_set_data (GTK_OBJECT (windowZoom), "drawingareaPreview", drawingareaPreview);
		gtk_widget_show (drawingareaPreview);
		gtk_container_add (GTK_CONTAINER (frameSampleText), drawingareaPreview);
		gtk_widget_set_usize (drawingareaPreview, 149, 10);
   	}
#endif
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Preview coming soon!", 0);
	PtCreateWidget(PtLabel, framePreview, n, args);
	
	n = 0;
	area.pos.y += area.size.h + V_SPACER;
	area.size.w = BUTTON_WIDTH; 
	area.size.h = 0; 
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	buttonOK = PtCreateWidget(PtButton, windowZoom, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	n = 0;
	area.pos.x += area.size.w + H_SPACER;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	buttonCancel = PtCreateWidget(PtButton, windowZoom, n, args);
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

	m_radioGroup = vboxZoomTo_group;
	
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

	
