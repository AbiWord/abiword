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
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"
#include "xav_View.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_Zoom.h"
#include "xap_QNXDlg_Zoom.h"
#include "xap_Preview_Zoom.h"
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
static int s_delete_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}
static int s_radio_clicked(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	char *str;
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	
	PtGetResource(w,Pt_ARG_USER_DATA,&str,0);	

	if(strcmp(str,"radio200")==NULL) {
		dlg->event_Radio200Clicked();
	}
	else if(strcmp(str,"radio100")==NULL) {
		dlg->event_Radio100Clicked();
	}
	else if(strcmp(str,"radio75")==NULL) {
		dlg->event_Radio75Clicked();
	}
	else if(strcmp(str,"radioPageWidth")==NULL) {
		dlg->event_RadioPageWidthClicked();
	} 
	else if(strcmp(str,"radioWholePage")==NULL) {
		dlg->event_RadioWholePageClicked();
	}
	else if(strcmp(str,"radioPercent")==NULL) {
		dlg->event_RadioPercentClicked();
	}
	return Pt_CONTINUE;
}
static int s_spin_Percent_changed(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Zoom *dlg = (XAP_QNXDialog_Zoom *)data;
	UT_ASSERT(dlg);
	dlg->event_SpinPercentChanged();
	return Pt_CONTINUE;
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
	m_zoomType = XAP_Frame::z_200;
	_enablePercentSpin(false);
	_updatePreviewZoomPercent(200);
	PtDamageWidget(m_previewArea);
}

void XAP_QNXDialog_Zoom::event_Radio100Clicked(void)
{
	//m_zoomType = XAP_Dialog_Zoom::zoomType::z_100;
	m_zoomType = XAP_Frame::z_100;
	_enablePercentSpin(false);
	_updatePreviewZoomPercent(100);
	PtDamageWidget(m_previewArea);
}

void XAP_QNXDialog_Zoom::event_Radio75Clicked(void)
{
	m_zoomType = XAP_Frame::z_75;
	_enablePercentSpin(false);
	_updatePreviewZoomPercent(75);
	PtDamageWidget(m_previewArea);
}

void XAP_QNXDialog_Zoom::event_RadioPageWidthClicked(void)
{
	m_zoomType = XAP_Frame::z_PAGEWIDTH;
	_enablePercentSpin(false);
	_updatePreviewZoomPercent(m_pFrame->getCurrentView()->calculateZoomPercentForPageWidth());
	PtDamageWidget(m_previewArea);
}

void XAP_QNXDialog_Zoom::event_RadioWholePageClicked(void)
{
	m_zoomType = XAP_Frame::z_WHOLEPAGE;
	_enablePercentSpin(false);
	_updatePreviewZoomPercent(m_pFrame->getCurrentView()->calculateZoomPercentForWholePage());
	PtDamageWidget(m_previewArea);
}

void XAP_QNXDialog_Zoom::event_RadioPercentClicked(void)
{
	m_zoomType = XAP_Frame::z_PERCENT;
	_enablePercentSpin(true);
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
	PtSetResource(m_radioPercent, Pt_ARG_FLAGS, Pt_SET, Pt_SET);
}


static int s_preview_exposed(PtWidget_t * w, PhTile_t * damage) {

   	PhRect_t rect;
   	PtSuperClassDraw(PtBasic, w, damage);
   	PtCalcCanvas(w, &rect);
	//clip to our basic canvas (it's only polite).
    PtClipAdd( w, &rect );

	XAP_QNXDialog_Zoom *pQNXDlg;
	PtGetResource(w, Pt_ARG_POINTER, &pQNXDlg,0);

	UT_ASSERT(pQNXDlg);
	pQNXDlg->event_PreviewAreaExposed();

    PtClipRemove();
	return Pt_CONTINUE;
}
	

/*****************************************************************/

void XAP_QNXDialog_Zoom::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;

	// Set the parent window for this dialog 
	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

	PtSetParentWidget(parentWindow);

	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	
	//m_qnxGraphics = new GR_QNXGraphics(mainWindow, m_previewArea, pFrame->getApp());
	GR_QNXAllocInfo ai(mainWindow, m_previewArea, pFrame->getApp());
	m_qnxGraphics = (GR_QNXGraphics*) XAP_App::getApp()->newGraphics(ai);

	unsigned short w, h;
	UT_QNXGetWidgetArea(m_previewArea, NULL, NULL, &w, &h);
	_createPreviewFromGC(m_qnxGraphics, w, h);

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

	_storeWindowData();
	
	UT_QNXBlockWidget(parentWindow, 0);
	PtDestroyWidget(mainWindow);
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

	PtWidget_t * radiobutton200;
	PtWidget_t * radiobutton100;
	PtWidget_t * radiobutton75;
	PtWidget_t * radiobuttonPageWidth;
	PtWidget_t * radiobuttonWholePage;
	PtWidget_t * radiobuttonPercent;
	PtWidget_t * spinbuttonPercent;

	PtWidget_t * frameSampleText;
	PtWidget_t * drawingareaPreview;

	PtWidget_t * buttonOK;
	PtWidget_t * buttonCancel;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowZoom = abiCreatePhabDialog("xap_QNXDlg_Zoom",pSS,XAP_STRING_ID_DLG_Zoom_ZoomTitle); 
	SetupContextHelp(windowZoom,this);
	PtAddHotkeyHandler(windowZoom,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	PtAddCallback(windowZoom, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);



	localizeLabel(abiPhabLocateWidget(windowZoom,"grpZoomTo"), pSS,XAP_STRING_ID_DLG_Zoom_RadioFrameCaption );

	localizeLabel(abiPhabLocateWidget(windowZoom,"grpPreview"), pSS,XAP_STRING_ID_DLG_Zoom_PreviewFrame );


	radiobutton200 = abiPhabLocateWidget(windowZoom,"radio200");
	localizeLabel(radiobutton200, pSS, XAP_STRING_ID_DLG_Zoom_200);
	PtAddCallback(radiobutton200, Pt_CB_ACTIVATE, s_radio_clicked, this);

	radiobutton100 = abiPhabLocateWidget(windowZoom,"radio100"); 
	localizeLabel(radiobutton100, pSS, XAP_STRING_ID_DLG_Zoom_100); 
	PtAddCallback(radiobutton100, Pt_CB_ACTIVATE, s_radio_clicked, this);

	radiobutton75 = abiPhabLocateWidget(windowZoom,"radio75"); 
	localizeLabel(radiobutton75, pSS, XAP_STRING_ID_DLG_Zoom_75); 
	PtAddCallback(radiobutton75, Pt_CB_ACTIVATE, s_radio_clicked, this);

	radiobuttonPageWidth = abiPhabLocateWidget(windowZoom,"radioPageWidth"); 
	localizeLabel(radiobuttonPageWidth, pSS, XAP_STRING_ID_DLG_Zoom_PageWidth); 
	PtAddCallback(radiobuttonPageWidth, Pt_CB_ACTIVATE, s_radio_clicked, this);

	radiobuttonWholePage = abiPhabLocateWidget(windowZoom,"radioWholePage"); 
	localizeLabel(radiobuttonWholePage, pSS, XAP_STRING_ID_DLG_Zoom_WholePage); 
	PtAddCallback(radiobuttonWholePage, Pt_CB_ACTIVATE, s_radio_clicked, this);

	radiobuttonPercent = abiPhabLocateWidget(windowZoom,"radioPercent"); 
	localizeLabel(radiobuttonPercent, pSS, XAP_STRING_ID_DLG_Zoom_Percent); 
	PtAddCallback(radiobuttonPercent, Pt_CB_ACTIVATE, s_radio_clicked, this);

	spinbuttonPercent = abiPhabLocateWidget(windowZoom,"NumericPercent");
	PtAddCallback(spinbuttonPercent, Pt_CB_NUMERIC_CHANGED, s_spin_Percent_changed, this);
	PtAddCallback(spinbuttonPercent, Pt_CB_ACTIVATE, s_spin_Percent_changed, this);


	drawingareaPreview = abiPhabLocateWidget(windowZoom,"rawPreview"); 
	PtSetResource(drawingareaPreview, Pt_ARG_POINTER, this,0 ); 
	PtSetResource(drawingareaPreview, Pt_ARG_RAW_DRAW_F, &s_preview_exposed, 1); 

	buttonOK = abiPhabLocateWidget(windowZoom,"btnOK"); 
	localizeLabel(buttonOK, pSS, XAP_STRING_ID_DLG_OK);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

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

	
	return windowZoom;
}

void XAP_QNXDialog_Zoom::_enablePercentSpin(bool enable) {
	if (enable == false) {
		PtSetResource(m_spinPercent, Pt_ARG_FLAGS, Pt_BLOCKED | Pt_GHOST, Pt_BLOCKED | Pt_GHOST);
	}
	else {
		PtSetResource(m_spinPercent, Pt_ARG_FLAGS, 0, Pt_BLOCKED | Pt_GHOST);
		//PtWidgetShowFocus(m_spinPercent);
	}
}

static void set_toggle_button(PtWidget_t *w, int enable) {
	PtArg_t arg;

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
	_enablePercentSpin(false);	// default
	switch(getZoomType())
	{
	case XAP_Frame::z_200:
		set_toggle_button(m_radio200, true);
		_updatePreviewZoomPercent(200);
		break;
	case XAP_Frame::z_100:
		set_toggle_button(m_radio100, true);
		_updatePreviewZoomPercent(100);		
		break;
	case XAP_Frame::z_75:
		set_toggle_button(m_radio75, true);
		_updatePreviewZoomPercent(75);
		break;
	case XAP_Frame::z_PAGEWIDTH:
		set_toggle_button(m_radioPageWidth, true);
		_updatePreviewZoomPercent(m_pFrame->getCurrentView()->calculateZoomPercentForPageWidth());
		break;
	case XAP_Frame::z_WHOLEPAGE:
		set_toggle_button(m_radioWholePage, true);
		_updatePreviewZoomPercent(m_pFrame->getCurrentView()->calculateZoomPercentForWholePage());
		break;
	case XAP_Frame::z_PERCENT:
		set_toggle_button(m_radioPercent, true);
		_enablePercentSpin(true);	// override
		_updatePreviewZoomPercent(getZoomPercent());
		break;
	default:
		// if they haven't set anything yet, default to the 100% radio item
		set_toggle_button(m_radio100, true);		
	}
	
	set_numeric_value(m_spinPercent, (int)getZoomPercent());
}

void XAP_QNXDialog_Zoom::_storeWindowData(void)
{
}

