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

#include "ut_types.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "gr_QNXGraphics.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"
#include "ap_QNXDialog_PageNumbers.h"
#include "ut_qnxHelper.h"

// static event callbacks
/***************************************************************/

static int s_ok_clicked (PtWidget_t * w, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_PageNumbers *dlg = (AP_QNXDialog_PageNumbers *)data;
	UT_ASSERT(dlg);
	dlg->event_OK();
	return Pt_CONTINUE;
}

static int s_cancel_clicked (PtWidget_t * w, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_PageNumbers *dlg = (AP_QNXDialog_PageNumbers *)data;
	UT_ASSERT(dlg);
	dlg->event_Cancel();
	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t * w, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_PageNumbers *dlg = (AP_QNXDialog_PageNumbers *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}

static int s_preview_exposed(PtWidget_t * w, PhTile_t * damage) 
{
   	PhRect_t rect;
   	PtSuperClassDraw(PtBasic, w, damage);
   	PtBasicWidgetCanvas(w, &rect);
	//clip to our basic canvas (it's only polite).
    PtClipAdd( w, &rect );

	AP_QNXDialog_PageNumbers *pQNXDlg;
	PtGetResource(w, Pt_ARG_POINTER, &pQNXDlg,0);

	UT_ASSERT(pQNXDlg);
	pQNXDlg->event_PreviewExposed();

    PtClipRemove();
	return Pt_CONTINUE;
}

static int s_position_changed (PtWidget_t * w, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_PageNumbers *dlg = (AP_QNXDialog_PageNumbers *)data;
	dlg->event_HdrFtrChanged();
	return Pt_CONTINUE;
}

static int s_alignment_changed (PtWidget_t * w, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_PageNumbers *dlg = (AP_QNXDialog_PageNumbers *)data;
	dlg->event_AlignChanged();
	return Pt_CONTINUE;
}

/***************************************************************/

XAP_Dialog * AP_QNXDialog_PageNumbers::static_constructor(XAP_DialogFactory * pFactory,
                                                         XAP_Dialog_Id id)
{
    AP_QNXDialog_PageNumbers * p = new AP_QNXDialog_PageNumbers(pFactory,id);
    return p;
}

AP_QNXDialog_PageNumbers::AP_QNXDialog_PageNumbers(XAP_DialogFactory * pDlgFactory,
                                                 XAP_Dialog_Id id)
    : AP_Dialog_PageNumbers(pDlgFactory,id)
{
  m_recentAlign   = m_align;
  m_recentControl = m_control;
  m_qnxGraphics  = NULL;
}

AP_QNXDialog_PageNumbers::~AP_QNXDialog_PageNumbers(void)
{
  DELETEP (m_qnxGraphics);
}

void AP_QNXDialog_PageNumbers::event_OK(void)
{
	m_answer = AP_Dialog_PageNumbers::a_OK;

	// set the align and control data
	m_align   = m_recentAlign;
	m_control = m_recentControl;
	done++;
}

void AP_QNXDialog_PageNumbers::event_Cancel(void)
{
	if (!done++) {
		m_answer = AP_Dialog_PageNumbers::a_CANCEL;
	}
}

void AP_QNXDialog_PageNumbers::event_WindowDelete(void)
{
	event_Cancel();
}

void AP_QNXDialog_PageNumbers::event_PreviewExposed(void)
{
	if(m_preview)
		m_preview->draw();
}

void AP_QNXDialog_PageNumbers::event_AlignChanged()
{
	AP_Dialog_PageNumbers::tAlign index;
	
	if(PtWidgetFlags(m_toggleAlignmentLeft) & Pt_SET)
		index = id_LALIGN;
	else if (PtWidgetFlags(m_toggleAlignmentRight) & Pt_SET)
		index = id_RALIGN;
	else if (PtWidgetFlags(m_toggleAlignmentCenter) & Pt_SET)
		index = id_CALIGN;

	m_recentAlign = index;
	_updatePreview(m_recentAlign, m_recentControl);
}

void AP_QNXDialog_PageNumbers::event_HdrFtrChanged()
{
	AP_Dialog_PageNumbers::tControl index;
	if(PtWidgetFlags(m_toggleHeader) & Pt_SET)
		index = id_HDR;
	else if (PtWidgetFlags(m_toggleFooter) & Pt_SET)
		index = id_FTR;

	m_recentControl = index; 
	_updatePreview(m_recentAlign, m_recentControl);
}

void AP_QNXDialog_PageNumbers::runModal(XAP_Frame * pFrame)
{
	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

	PtSetParentWidget(parentWindow);
    // Build the window's widgets and arrange them
    PtWidget_t * mainWindow = _constructWindow();
    UT_ASSERT(mainWindow);

    //connectFocus(mainWindow, pFrame);

    // save for use with event
    m_pFrame = pFrame;

	// Center the widget, make it modal and realize it.
	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);
	PtRealizeWidget(mainWindow);

    // *** this is how we add the gc ***
	{
	  // attach a new graphics context to the drawing area
	  //XAP_QNXApp * app = static_cast<XAP_QNXApp *> (m_pApp);
	  //UT_ASSERT(app);

	  UT_ASSERT(m_previewArea);
	  DELETEP (m_qnxGraphics);
	  
	  // make a new QNX GC
	  // m_qnxGraphics = new GR_QNXGraphics(mainWindow, m_previewArea, m_pApp);
	  GR_QNXAllocInfo ai(mainWindow, m_previewArea, m_pApp);
	  m_qnxGraphics = (GR_QNXGraphics*) XAP_App::getApp()->newGraphics(ai);

	  
	  // let the widget materialize
	  unsigned short *w, *h;
	  w = h = NULL;
	  PtGetResource(m_previewArea, Pt_ARG_WIDTH, &w, 0);
	  PtGetResource(m_previewArea, Pt_ARG_HEIGHT, &h, 0);
	  _createPreviewFromGC(m_qnxGraphics, (UT_uint32)*w, (UT_uint32)*h);

	  // hack in a quick draw here
	  _updatePreview(m_recentAlign, m_recentControl);
	  event_PreviewExposed ();
	}

    // Run into the event loop for this window.
	int count;
	count = PtModalStart();
	done = 0;
	while(!done) {
		PtProcessEvent();
	}
	PtModalEnd(MODAL_END_ARG(count));

    DELETEP (m_qnxGraphics);

	UT_QNXBlockWidget(parentWindow, 0);
	PtDestroyWidget(mainWindow);
}

PtWidget_t * AP_QNXDialog_PageNumbers::_constructWindow (void)
{  

  const XAP_StringSet * pSS = m_pApp->getStringSet();

	m_window = abiCreatePhabDialog("ap_QNXDialog_PageNumbers",pSS,AP_STRING_ID_DLG_PageNumbers_Title);


	SetupContextHelp(m_window,this);
	PtAddHotkeyHandler(m_window,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	PtAddCallback(m_window,Pt_CB_WINDOW_CLOSING,s_delete_clicked,this);

	//Create the first label/toggle combination
	localizeLabel(abiPhabLocateWidget(m_window,"grpPosition"), pSS, AP_STRING_ID_DLG_PageNumbers_Position);
	UT_UTF8String s;
	
	m_toggleFooter = abiPhabLocateWidget(m_window,"togglePositionFooter");
	pSS->getValueUTF8(AP_STRING_ID_DLG_PageNumbers_Footer,s);
	PtSetResource(m_toggleFooter,Pt_ARG_TEXT_STRING,s.utf8_str(),0);
	m_toggleHeader = abiPhabLocateWidget(m_window,"togglePositionHeader");
	pSS->getValueUTF8(AP_STRING_ID_DLG_PageNumbers_Header,s);
	PtSetResource(m_toggleHeader,Pt_ARG_TEXT_STRING,s.utf8_str(),0);
	PtAddCallback(m_toggleFooter, Pt_CB_ACTIVATE, s_position_changed, this);
	PtAddCallback(m_toggleHeader, Pt_CB_ACTIVATE, s_position_changed,this);

	//Create the second label/toggle combination
	localizeLabel(abiPhabLocateWidget(m_window,"grpAlignment"), pSS, AP_STRING_ID_DLG_PageNumbers_Alignment);
	
	m_toggleAlignmentRight = abiPhabLocateWidget(m_window,"toggleAlignmentRight");
	pSS->getValueUTF8(AP_STRING_ID_DLG_PageNumbers_Right,s);
	PtSetResource(m_toggleAlignmentRight,Pt_ARG_TEXT_STRING,s.utf8_str(),0);
	m_toggleAlignmentLeft = abiPhabLocateWidget(m_window,"toggleAlignmentLeft");
	pSS->getValueUTF8(AP_STRING_ID_DLG_PageNumbers_Left,s);
	PtSetResource(m_toggleAlignmentLeft,Pt_ARG_TEXT_STRING,s.utf8_str(),0);
	m_toggleAlignmentCenter = abiPhabLocateWidget(m_window,"toggleAlignmentCenter");
	pSS->getValueUTF8(AP_STRING_ID_DLG_PageNumbers_Center,s);
	PtSetResource(m_toggleAlignmentCenter,Pt_ARG_TEXT_STRING,s.utf8_str(),0);

PtAddCallback(m_toggleAlignmentLeft, Pt_CB_ACTIVATE, s_alignment_changed, this);
PtAddCallback(m_toggleAlignmentCenter, Pt_CB_ACTIVATE, s_alignment_changed, this);
PtAddCallback(m_toggleAlignmentRight, Pt_CB_ACTIVATE, s_alignment_changed, this);

	//Create the preview area
	m_previewArea = abiPhabLocateWidget(m_window,"rawPreview"); 
	PtSetResource(m_previewArea, Pt_ARG_POINTER,this, 0); 
	PtSetResource(m_previewArea, Pt_ARG_RAW_DRAW_F, &s_preview_exposed, 1); 

	m_buttonOK = abiPhabLocateWidget(m_window,"btnOK");
	localizeLabel(m_buttonOK, pSS, XAP_STRING_ID_DLG_OK);
	PtAddCallback(m_buttonOK, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	m_buttonCancel = abiPhabLocateWidget(m_window,"btnCancel"); 
	PtSetResource(m_buttonCancel, Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_Cancel), 0);
	PtAddCallback(m_buttonCancel, Pt_CB_ACTIVATE, s_ok_clicked, this);

	return m_window;
}
