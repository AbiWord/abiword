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
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Field.h"
#include "ap_QNXDialog_Field.h"
#include "ap_QNXDialog_Columns.h"
#include "ap_QNXDialog_FormatFrame.h"

#include "ut_qnxHelper.h"


/*****************************************************************/

XAP_Dialog * AP_QNXDialog_FormatFrame::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_FormatFrame * p = new AP_QNXDialog_FormatFrame(pFactory,id);
	return p;
}

AP_QNXDialog_FormatFrame::AP_QNXDialog_FormatFrame(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_FormatFrame(pDlgFactory,id)
{
}

AP_QNXDialog_FormatFrame::~AP_QNXDialog_FormatFrame(void)
{
}

void AP_QNXDialog_FormatFrame::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(0); //DEPRICATED.
}

void AP_QNXDialog_FormatFrame::runModeless(XAP_Frame *pFrame)
{
	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);

	// This magic command displays the frame that characters will be
	// inserted into.
	connectFocusModeless(mainWindow, m_pApp);

	//m_pPreviewWidget = new GR_QNXGraphics(m_mainWindow,m_wPreviewArea,pFrame->getApp());	
	GR_QNXAllocInfo ai(mainWindow, m_wPreviewArea, pFrame->getApp());
	m_pPreviewWidget = (GR_QNXGraphics*) XAP_App::getApp()->newGraphics(ai);

	unsigned short w,h;
	UT_QNXGetWidgetArea(m_wPreviewArea, NULL, NULL, &w, &h);
	_createPreviewFromGC(m_pPreviewWidget,w,h);

	PtRealizeWidget(mainWindow);

	m_pFormatFramePreview->draw();
	startUpdater();
}

int s_apply_clicked(PtWidget_t *w,AP_QNXDialog_FormatFrame *dlg,PtCallbackInfo_t *cbinfo)
{
dlg->lineClicked();
dlg->applyChanges();
return Pt_CONTINUE;
}

int s_delete_clicked(PtWidget_t *w,AP_QNXDialog_FormatFrame *dlg,PtCallbackInfo_t *cbinfo)
{
dlg->event_WindowDelete();
return Pt_CONTINUE;
}

void AP_QNXDialog_FormatFrame::event_WindowDelete()
{
finalize();
PtDestroyWidget(m_mainWindow);
}

int s_line_clicked(PtWidget_t *w,AP_QNXDialog_FormatFrame *dlg,PtCallbackInfo_t *cbinfo)
{
dlg->lineClicked();
return Pt_CONTINUE;
}

void AP_QNXDialog_FormatFrame::lineClicked()
{
bool top,left,right,bottom;
top = left = right = bottom = false;

if(PtWidgetFlags(m_wLineTop) & Pt_SET) top = true;
if(PtWidgetFlags(m_wLineLeft) & Pt_SET) left = true;
if(PtWidgetFlags(m_wLineRight) & Pt_SET) right = true;
if(PtWidgetFlags(m_wLineBottom) & Pt_SET) bottom = true;

toggleLineType(AP_Dialog_FormatFrame::toggle_bottom,bottom);
toggleLineType(AP_Dialog_FormatFrame::toggle_right,right);
toggleLineType(AP_Dialog_FormatFrame::toggle_left,left);
toggleLineType(AP_Dialog_FormatFrame::toggle_top,top);

event_previewExposed();

}

int s_color_background(PtWidget_t *w,AP_QNXDialog_FormatFrame *dlg,PtCallbackInfo_t *cbinfo)
{
PtColorSelectInfo_t colorinfo;
memset(&colorinfo,0,sizeof(colorinfo));

colorinfo.flags |= Pt_COLORSELECT_MODAL;

PtColorSelect(PtGetParent(w,PtWindow),"Select Background color",&colorinfo);

if(colorinfo.flags & Pt_COLORSELECT_ACCEPT)
{
dlg->setBGColor(UT_RGBColor(PgRedValue(colorinfo.rgb),PgGreenValue(colorinfo.rgb),PgBlueValue(colorinfo.rgb)));
PtSetResource(w,Pt_ARG_FILL_COLOR,colorinfo.rgb,0);
dlg->event_previewExposed();
}
return Pt_CONTINUE;
}
int s_color_border(PtWidget_t *w,AP_QNXDialog_FormatFrame *dlg,PtCallbackInfo_t *cbinfo)
{
PtColorSelectInfo_t colorinfo;

memset(&colorinfo,0,sizeof(colorinfo));
colorinfo.flags |= Pt_COLORSELECT_MODAL;
PtColorSelect(PtGetParent(w,PtWindow),"Select Background color",&colorinfo);

if(colorinfo.flags & Pt_COLORSELECT_ACCEPT)
{
dlg->setBorderColor(UT_RGBColor(PgRedValue(colorinfo.rgb),PgGreenValue(colorinfo.rgb),PgBlueValue(colorinfo.rgb)));
PtSetResource(w,Pt_ARG_FILL_COLOR,colorinfo.rgb,0);
dlg->event_previewExposed();
}
return Pt_CONTINUE;
}
PtWidget_t* AP_QNXDialog_FormatFrame::_constructWindow()
{
	PtArg_t	args[10];
	int 	n;


	const XAP_StringSet * pSS = m_pApp->getStringSet();

	ConstructWindowName();

	n = 0;
	m_mainWindow = abiCreatePhabDialog("ap_QNXDialog_FormatFrame",pSS,XAP_STRING_ID_DLG_Cancel);
	PtSetResource(m_mainWindow,Pt_ARG_WINDOW_TITLE,m_WindowName,0); 
	PtAddHotkeyHandler(m_mainWindow,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	SetupContextHelp(m_mainWindow,this);
	PtAddCallback(m_mainWindow, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	localizeLabel(abiPhabLocateWidget(m_mainWindow,"grpBorder"),pSS,AP_STRING_ID_DLG_FormatFrame_Borders);

	localizeLabel(abiPhabLocateWidget(m_mainWindow,"lblBorderColor"),pSS,AP_STRING_ID_DLG_FormatFrame_Border_Color);
	m_wBorderColorButton = abiPhabLocateWidget(m_mainWindow,"btnBorderColor");
	PtAddCallback(m_wBorderColorButton,Pt_CB_ACTIVATE,s_color_border,this);
	

	localizeLabel(abiPhabLocateWidget(m_mainWindow,"grpBackground"),pSS,AP_STRING_ID_DLG_FormatFrame_Background);
	localizeLabel(abiPhabLocateWidget(m_mainWindow,"lblBackgroundColor"),pSS,AP_STRING_ID_DLG_FormatFrame_Background_Color);
	m_wBackgroundColorButton = abiPhabLocateWidget(m_mainWindow,"btnBackgroundColor");
	PtAddCallback(m_wBackgroundColorButton,Pt_CB_ACTIVATE,s_color_background,this);

	localizeLabel(abiPhabLocateWidget(m_mainWindow,"grpPreview"),pSS,AP_STRING_ID_DLG_FormatFrame_Preview);

	m_wLineTop = abiPhabLocateWidget(m_mainWindow,"btnTop");
	m_wLineLeft =abiPhabLocateWidget(m_mainWindow,"btnLeft"); 
	m_wLineRight =abiPhabLocateWidget(m_mainWindow,"btnRight"); 
	m_wLineBottom =abiPhabLocateWidget(m_mainWindow,"btnBottom"); 

	// place some nice pixmaps on our border toggle buttons
	label_button_with_abi_pixmap(m_wLineTop, "tb_LineTop_xpm");
	label_button_with_abi_pixmap(m_wLineLeft, "tb_LineLeft_xpm");
	label_button_with_abi_pixmap(m_wLineRight, "tb_LineRight_xpm");
	label_button_with_abi_pixmap(m_wLineBottom, "tb_LineBottom_xpm");
	PtSetResource(m_wLineTop,Pt_ARG_LABEL_TYPE,Pt_IMAGE,0);
	PtSetResource(m_wLineLeft,Pt_ARG_LABEL_TYPE,Pt_IMAGE,0);
	PtSetResource(m_wLineRight,Pt_ARG_LABEL_TYPE,Pt_IMAGE,0);
	PtSetResource(m_wLineBottom,Pt_ARG_LABEL_TYPE,Pt_IMAGE,0);

	PtSetResource(m_wLineTop,Pt_ARG_FLAGS,getTopToggled() ? Pt_TRUE : Pt_FALSE,Pt_SET);
	PtSetResource(m_wLineLeft,Pt_ARG_FLAGS,getLeftToggled() ? Pt_TRUE : Pt_FALSE,Pt_SET);
	PtSetResource(m_wLineRight,Pt_ARG_FLAGS,getRightToggled() ? Pt_TRUE : Pt_FALSE ,Pt_SET);
	PtSetResource(m_wLineBottom,Pt_ARG_FLAGS,getBottomToggled() ? Pt_TRUE : Pt_FALSE,Pt_SET);

	PtAddCallback(m_wLineTop, Pt_CB_ACTIVATE, s_line_clicked, this);
	PtAddCallback(m_wLineLeft, Pt_CB_ACTIVATE, s_line_clicked, this);
	PtAddCallback(m_wLineRight, Pt_CB_ACTIVATE, s_line_clicked, this);
	PtAddCallback(m_wLineBottom, Pt_CB_ACTIVATE, s_line_clicked, this);

	m_wPreviewArea = abiPhabLocateWidget(m_mainWindow,"rawPreview");

	PtWidget_t *buttonClose = abiPhabLocateWidget(m_mainWindow,"btnClose");
	localizeLabel(buttonClose, pSS, XAP_STRING_ID_DLG_Close);
	PtAddCallback(buttonClose, Pt_CB_ACTIVATE, s_delete_clicked, this);

	m_wApplyButton = abiPhabLocateWidget(m_mainWindow,"btnApply");
	localizeLabel(m_wApplyButton, pSS, XAP_STRING_ID_DLG_Apply);
	PtAddCallback(m_wApplyButton, Pt_CB_ACTIVATE, s_apply_clicked, this);

	return m_mainWindow;
}

void AP_QNXDialog_FormatFrame::setSensitivity(bool onoff)
{
	PtSetResource(m_wBorderColorButton,Pt_ARG_FLAGS, onoff ? Pt_FALSE : Pt_TRUE,Pt_BLOCKED|Pt_GHOST);
	PtSetResource(m_wBackgroundColorButton,Pt_ARG_FLAGS, onoff ? Pt_FALSE : Pt_TRUE,Pt_BLOCKED|Pt_GHOST);	
	PtSetResource(m_wLineLeft,Pt_ARG_FLAGS, onoff ? Pt_FALSE : Pt_TRUE,Pt_BLOCKED);
	PtSetResource(m_wLineRight,Pt_ARG_FLAGS, onoff ? Pt_FALSE : Pt_TRUE,Pt_BLOCKED);
	PtSetResource(m_wLineTop,Pt_ARG_FLAGS, onoff ? Pt_FALSE : Pt_TRUE,Pt_BLOCKED);
	PtSetResource(m_wLineBottom,Pt_ARG_FLAGS, onoff ? Pt_FALSE : Pt_TRUE,Pt_BLOCKED);
	PtSetResource(m_wApplyButton,Pt_ARG_FLAGS, onoff ? Pt_FALSE : Pt_TRUE,Pt_BLOCKED|Pt_GHOST);
}

void AP_QNXDialog_FormatFrame::activate()
{

ConstructWindowName();
PtSetResource(m_mainWindow,Pt_ARG_WINDOW_TITLE,m_WindowName,0);
setAllSensitivities();
}

void AP_QNXDialog_FormatFrame::notifyActiveFrame(XAP_Frame *pFrame)
{
ConstructWindowName();
PtSetResource(m_mainWindow,Pt_ARG_WINDOW_TITLE,m_WindowName,0);
setAllSensitivities();
}
void AP_QNXDialog_FormatFrame::destroy()
{
finalize();
PtDestroyWidget(m_mainWindow);
}


void AP_QNXDialog_FormatFrame::event_previewExposed()
{
if(m_pFormatFramePreview)
	m_pFormatFramePreview->draw();

}
