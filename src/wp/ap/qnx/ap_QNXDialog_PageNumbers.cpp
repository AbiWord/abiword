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
#include "xap_QNXFrame.h"

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
	PtArg_t args[1];

   	PhRect_t rect;
   	PtSuperClassDraw(PtBasic, w, damage);
   	PtBasicWidgetCanvas(w, &rect);
	//clip to our basic canvas (it's only polite).
    PtClipAdd( w, &rect );

	AP_QNXDialog_PageNumbers *pQNXDlg, **ppQNXDlg = NULL;
	PtSetArg(&args[0], Pt_ARG_USER_DATA, &ppQNXDlg, 0);
	PtGetResources(w, 1, args);
	pQNXDlg = (ppQNXDlg) ? *ppQNXDlg : NULL;

	UT_ASSERT(pQNXDlg);
	pQNXDlg->event_PreviewExposed();

    PtClipRemove();
	return Pt_CONTINUE;
}

static int s_position_changed (PtWidget_t * w, void *data, PtCallbackInfo_t *info) 
{
	PtListCallback_t *ldata = (PtListCallback_t *)info->cbdata;
	AP_QNXDialog_PageNumbers *dlg = (AP_QNXDialog_PageNumbers *)data;
	if (info->reason_subtype != Pt_LIST_SELECTION_FINAL) {
		return Pt_CONTINUE;
	}
	dlg->event_HdrFtrChanged(ldata->item_pos - 1);
	return Pt_CONTINUE;
}

static int s_alignment_changed (PtWidget_t * w, void *data, PtCallbackInfo_t *info) 
{
	PtListCallback_t *ldata = (PtListCallback_t *)info->cbdata;
	AP_QNXDialog_PageNumbers *dlg = (AP_QNXDialog_PageNumbers *)data;
	if (info->reason_subtype != Pt_LIST_SELECTION_FINAL) {
		return Pt_CONTINUE;
	}
	dlg->event_AlignChanged(ldata->item_pos - 1);
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

void AP_QNXDialog_PageNumbers::event_AlignChanged(int index /* AP_Dialog_PageNumbers::tAlign align */)
{
	m_recentAlign = (AP_Dialog_PageNumbers::tAlign)((int)m_vecalign.getNthItem(index));
	_updatePreview(m_recentAlign, m_recentControl);
}

void AP_QNXDialog_PageNumbers::event_HdrFtrChanged(int index /* AP_Dialog_PageNumbers::tControl control */)
{
	m_recentControl = (AP_Dialog_PageNumbers::tControl)((int)m_vecposition.getNthItem(index));
	_updatePreview(m_recentAlign, m_recentControl);
}

void AP_QNXDialog_PageNumbers::runModal(XAP_Frame * pFrame)
{
	// To center the dialog, we need the frame of its parent.
    XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
    UT_ASSERT(pQNXFrame);
    
    // Get the of the parent frame
    PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
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
	  m_qnxGraphics = new GR_QNXGraphics(mainWindow, m_previewArea, m_pApp);
	  
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
  PtWidget_t *vgroup;
  PtWidget_t *vgroup_inner;
  PtWidget_t *hgroup;

  PtWidget_t *label1;
  PtWidget_t *combo1;
  PtWidget_t *label2;
  PtWidget_t *combo2;

  PtArg_t	args[10];
  int		n;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Title), 0);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	m_window = PtCreateWidget(PtWindow, NULL, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 5, 0);
	vgroup = PtCreateWidget(PtGroup, m_window, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_X, 5, 0);
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EQUAL_SIZE_HORIZONTAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 5, 0);
	vgroup_inner = PtCreateWidget(PtGroup, hgroup, n, args);

	//Create the first label/combo combination
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Position), 0);
	label1 = PtCreateWidget(PtLabel, vgroup_inner, n, args);

	n = 0;
    PtSetArg(&args[n++], Pt_ARG_WIDTH,  2* ABI_DEFAULT_BUTTON_WIDTH, 0);
	combo1 = PtCreateWidget(PtComboBox, vgroup_inner, n, args);
	const char *add;
	add = pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Footer); 
	PtListAddItems(combo1, &add, 1, 0);
	add = pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Header); 
	PtListAddItems(combo1, &add, 1, 0);
	m_vecposition.addItem((void *)AP_Dialog_PageNumbers::id_FTR);
	m_vecposition.addItem((void *)AP_Dialog_PageNumbers::id_HDR);
	PtAddCallback(combo1, Pt_CB_SELECTION, s_position_changed, this);
	UT_QNXComboSetPos(combo1, 1);

	//Create the second label/combo combination
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Alignment), 0);
	label2 = PtCreateWidget(PtLabel, vgroup_inner, n, args);

	n = 0;
    PtSetArg(&args[n++], Pt_ARG_WIDTH,  2* ABI_DEFAULT_BUTTON_WIDTH, 0);
	combo2 = PtCreateWidget(PtComboBox, vgroup_inner, n, args);
	add = pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Right); 
	PtListAddItems(combo2, &add, 1, 0);
	add = pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Left); 
	PtListAddItems(combo2, &add, 1, 0);
	add = pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Center); 
	PtListAddItems(combo2, &add, 1, 0);
	m_vecalign.addItem((void *)AP_Dialog_PageNumbers::id_RALIGN);
	m_vecalign.addItem((void *)AP_Dialog_PageNumbers::id_LALIGN);
	m_vecalign.addItem((void *)AP_Dialog_PageNumbers::id_CALIGN);
	PtAddCallback(combo2, Pt_CB_SELECTION, s_alignment_changed, this);
	UT_QNXComboSetPos(combo2, 1);

	//Create the preview area
	//frame1 = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Preview));
	n = 0;
	PtWidget_t *rgroup = PtCreateWidget(PtGroup, hgroup, n, args);
	n = 0;
	void *data = (void *)this;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 95, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 115, 0);
	PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this)); 
	PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &s_preview_exposed, 1); 
	m_previewArea = PtCreateWidget( PtRaw, rgroup, n, args);

	//Create the buttons on the bottom
	n = 0;
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
    PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH + ABI_DEFAULT_BUTTON_WIDTH / 2,  0);
	PtCreateWidget(PtLabel, hgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_OK), 0);
    PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	m_buttonOK = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(m_buttonOK, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_Cancel), 0);
    PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	m_buttonCancel = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(m_buttonCancel, Pt_CB_ACTIVATE, s_ok_clicked, this);

	return m_window;
}
