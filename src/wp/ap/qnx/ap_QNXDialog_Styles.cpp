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

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_QNXDialog_Styles.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fv_View.h"
#include "pd_Style.h"
#include "ut_string_class.h"

#include "ut_qnxHelper.h"

XAP_Dialog * AP_QNXDialog_Styles::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_QNXDialog_Styles * p = new AP_QNXDialog_Styles(pFactory,id);
	return p;
}

AP_QNXDialog_Styles::AP_QNXDialog_Styles(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
  : AP_Dialog_Styles(pDlgFactory,id), m_whichRow(0), m_whichCol(0), m_whichType(AP_QNXDialog_Styles::USED_STYLES)
{
	m_windowMain = NULL;

	m_wbuttonApply = NULL;
	m_wbuttonClose = NULL;
	m_wGnomeButtons = NULL;
	m_wParaPreviewArea = NULL;
	m_pParaPreviewGR = NULL;
	m_wCharPreviewArea = NULL;
	m_pCharPreviewGR = NULL;

	m_wclistStyles = NULL;
	m_wlistTypes = NULL;
	m_wlabelDesc = NULL;

	m_wModifyDialog = NULL;
	m_wStyleNameEntry = NULL;
	m_wBasedOnCombo = NULL;
	m_wBasedOnEntry = NULL;
	m_wFollowingCombo = NULL;
	m_wFollowingEntry = NULL;
	m_wStyleTypeCombo = NULL;
	m_wStyleTypeEntry = NULL;
	m_wLabDescription = NULL;

	m_pAbiPreviewGR = NULL;
	m_wModifyDrawingArea = NULL;

	m_wModifyOk = NULL;
	m_wModifyCancel = NULL;
	m_wFormatMenu = NULL;
	m_wModifyShortCutKey = NULL;

	m_wFormat = NULL;
	m_wModifyParagraph = NULL;
	m_wModifyFont = NULL;
	m_wModifyNumbering = NULL;
	m_wModifyLanguage = NULL;
	//m_gbasedOnStyles = NULL;
	//m_gfollowedByStyles = NULL;
	//m_gStyleType = NULL;
	m_bBlockModifySignal = false;

}

AP_QNXDialog_Styles::~AP_QNXDialog_Styles(void)
{
	DELETEP (m_pParaPreviewGR);
	DELETEP (m_pCharPreviewGR);
	DELETEP (m_pAbiPreviewGR);
}

/*****************************************************************/

static int s_clist_clicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	PtListCallback_t *cblist = (PtListCallback_t *)info->cbdata;

	if(info->reason_subtype != Pt_LIST_SELECTION_FINAL) {
		return Pt_CONTINUE;
	}
	dlg->event_ClistClicked (cblist->item_pos - 1 /*row*/, 0 /*col*/);
	return Pt_CONTINUE;
}

static int s_typeslist_changed (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	PtListCallback_t *cblist = (PtListCallback_t *)info->cbdata;

	if(info->reason_subtype != Pt_LIST_SELECTION_FINAL) {
		return Pt_CONTINUE;
	}
	dlg->event_ListClicked(cblist->item);
	return Pt_CONTINUE;
}

static int s_deletebtn_clicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	dlg->event_DeleteClicked ();
	return Pt_CONTINUE;
}

static int s_modifybtn_clicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	dlg->event_ModifyClicked ();
	return Pt_CONTINUE;
}

static int s_newbtn_clicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	dlg->event_NewClicked ();
	return Pt_CONTINUE;
}

static int s_remove_property(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_RemoveProperty();
	return Pt_CONTINUE;
}

static int s_apply_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Apply();
	return Pt_CONTINUE;
}

static int s_close_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Close();
	return Pt_CONTINUE;
}


static int s_modify_ok_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Modify_OK();
	return Pt_CONTINUE;
}

static int s_modify_cancel_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Modify_Cancel();
	return Pt_CONTINUE;
}

static int s_paraPreview_exposed(PtWidget_t * w, PhTile_t * damage) 
{
	PtArg_t args[1];

   	PhRect_t rect;
   	PtSuperClassDraw(PtBasic, w, damage);
   	PtBasicWidgetCanvas(w, &rect);
	//clip to our basic canvas (it's only polite).
    PtClipAdd( w, &rect );

	AP_QNXDialog_Styles *pQNXDlg, **ppQNXDlg = NULL;
	PtSetArg(&args[0], Pt_ARG_USER_DATA, &ppQNXDlg, 0);
	PtGetResources(w, 1, args);
	pQNXDlg = (ppQNXDlg) ? *ppQNXDlg : NULL;

	UT_ASSERT(pQNXDlg);
	pQNXDlg->event_paraPreviewExposed();

    PtClipRemove();
	return Pt_CONTINUE;
}

static int s_charPreview_exposed(PtWidget_t * w, PhTile_t * damage) 
{
	PtArg_t args[1];

   	PhRect_t rect;
   	PtSuperClassDraw(PtBasic, w, damage);
   	PtBasicWidgetCanvas(w, &rect);
	//clip to our basic canvas (it's only polite).
    PtClipAdd( w, &rect );

	AP_QNXDialog_Styles *pQNXDlg, **ppQNXDlg = NULL;
	PtSetArg(&args[0], Pt_ARG_USER_DATA, &ppQNXDlg, 0);
	PtGetResources(w, 1, args);
	pQNXDlg = (ppQNXDlg) ? *ppQNXDlg : NULL;

	UT_ASSERT(pQNXDlg);
	pQNXDlg->event_charPreviewExposed();

    PtClipRemove();
	return Pt_CONTINUE;
}

static int s_modifyPreview_exposed(PtWidget_t * w, PhTile_t * damage) 
{
	PtArg_t args[1];

   	PhRect_t rect;
   	PtSuperClassDraw(PtBasic, w, damage);
   	PtBasicWidgetCanvas(w, &rect);
	//clip to our basic canvas (it's only polite).
    PtClipAdd( w, &rect );

	AP_QNXDialog_Styles *pQNXDlg, **ppQNXDlg = NULL;
	PtSetArg(&args[0], Pt_ARG_USER_DATA, &ppQNXDlg, 0);
	PtGetResources(w, 1, args);
	pQNXDlg = (ppQNXDlg) ? *ppQNXDlg : NULL;

	UT_ASSERT(pQNXDlg);
	pQNXDlg->event_ModifyPreviewExposed();

    PtClipRemove();
	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}


static int s_modify_delete_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	UT_ASSERT(dlg);
	dlg->event_ModifyDelete();
	return Pt_CONTINUE;
}


static int s_modify_format(PtWidget_t *widget, void *data, PtCallbackInfo_t *info) {
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	UT_ASSERT(dlg);
	dlg->event_ModifySelected(widget, data, info);
	return Pt_CONTINUE;
}

static int s_modify_paragraph(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	UT_ASSERT(dlg);
	dlg->event_ModifyParagraph();
	return Pt_CONTINUE;
}

static int s_modify_font(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	UT_ASSERT(dlg);
	dlg->event_ModifyFont();
	return Pt_CONTINUE;
}


static int s_modify_numbering(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	UT_ASSERT(dlg);
	dlg->event_ModifyNumbering();
	return Pt_CONTINUE;
}


static int s_modify_language (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	UT_ASSERT(dlg);
	dlg->event_ModifyLanguage();
	return Pt_CONTINUE;
}

static int s_modify_tabs(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Styles * dlg = (AP_QNXDialog_Styles *)data;
	UT_ASSERT(dlg);
	dlg->event_ModifyTabs();
	return Pt_CONTINUE;
}


/*****************************************************************/

void AP_QNXDialog_Styles::runModal(XAP_Frame * pFrame)
{
//
// Get View and Document pointers. Place them in member variables
//
	setFrame(pFrame);
	setView((FV_View *) pFrame->getCurrentView());
	UT_ASSERT(getView());

	setDoc(getView()->getLayout()->getDocument());

	UT_ASSERT(getDoc());

	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

	PtSetParentWidget(parentWindow);

	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(mainWindow,pFrame);
		
	//Center and show the window
	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);
	PtRealizeWidget(mainWindow);
	
    // populate the member variables for the  previews
	_populatePreviews(false);

	// *** this is how we add the gc for the para and char Preview's ***
	// attach a new graphics context to the drawing area

	unsigned short w, h;
	XAP_QNXApp * app = static_cast<XAP_QNXApp *> (m_pApp);

	UT_ASSERT(app);
	UT_ASSERT(m_wParaPreviewArea);

	// make a new QNX GC for Paragraph Preview
	DELETEP (m_pParaPreviewGR);
	//m_pParaPreviewGR = new GR_QNXGraphics(mainWindow, m_wParaPreviewArea, m_pApp);
	GR_QNXAllocInfo ai(mainWindow, m_wParaPreviewArea, m_pApp);
	m_pParaPreviewGR = (GR_QNXGraphics*) XAP_App::getApp()->newGraphics(ai);
	
	UT_QNXGetWidgetArea(m_wParaPreviewArea, NULL, NULL, &w, &h);
	_createParaPreviewFromGC(m_pParaPreviewGR, w, h);

	// make a new QNX GC for Character Preview
	DELETEP (m_pCharPreviewGR);
	//m_pCharPreviewGR = new GR_QNXGraphics(mainWindow, m_wCharPreviewArea, m_pApp);
	GR_QNXAllocInfo ai_(mainWindow, m_wCharPreviewArea, m_pApp);
	m_pCharPreviewGR = (GR_QNXGraphics*) XAP_App::getApp()->newGraphics(ai_);

	UT_QNXGetWidgetArea(m_wCharPreviewArea, NULL, NULL, &w, &h);
	_createCharPreviewFromGC(m_pCharPreviewGR, w, h);

	// Populate the window's data items
	_populateWindowData();

	event_paraPreviewExposed();
	event_charPreviewExposed();

	int count;
	count = PtModalStart();
	done = 0;
	while(!done) {
		PtProcessEvent();
	}
	PtModalEnd(MODAL_END_ARG(count));

	DELETEP (m_pParaPreviewGR);
	DELETEP (m_pCharPreviewGR);
	
	UT_QNXBlockWidget(parentWindow, 0);
	PtDestroyWidget(mainWindow);
}

/*****************************************************************/

void AP_QNXDialog_Styles::event_Apply(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_Styles::a_OK;
	const gchar * szStyle = getCurrentStyle();
	if(szStyle && *szStyle)
	{
		getView()->setStyle(szStyle);
	}
	//done++;
}

void AP_QNXDialog_Styles::event_Close(void)
{
	if(!done++) {
		m_answer = AP_Dialog_Styles::a_CANCEL;
	}
}

void AP_QNXDialog_Styles::event_WindowDelete(void)
{
	if(!done++) {
		m_answer = AP_Dialog_Styles::a_CANCEL;
	}
}

void AP_QNXDialog_Styles::event_paraPreviewExposed(void)
{
	if(m_pParaPreview)
		m_pParaPreview->draw();
}


void AP_QNXDialog_Styles::event_charPreviewExposed(void)
{
	if(m_pCharPreview)
		event_charPreviewUpdated();
}

void AP_QNXDialog_Styles::event_DeleteClicked(void)
{
	if (m_whichRow != -1)
    {
		unsigned short *index;
		char **styles, *style;

		index = NULL;
		styles = NULL;
		PtGetResource(m_wclistStyles, Pt_ARG_SELECTION_INDEXES, &index, 0);
		PtGetResource(m_wclistStyles, Pt_ARG_ITEMS, &styles, 0);

		if (!index || !styles || index[0] < 1)
			return; // ok, nothing's selected. that's fine

		UT_ASSERT(m_whichRow == index[0] -1);

		style = styles[index[0] - 1];
		UT_DEBUGMSG(("DOM: attempting to delete style %s\n", style));

		if (!getDoc()->removeStyle(style)) // actually remove the style
		{
			const XAP_StringSet * pSS = m_pApp->getStringSet();
			const gchar * msg = pSS->getValue (AP_STRING_ID_DLG_Styles_ErrStyleCantDelete);
		
			getFrame()->showMessageBox ((const char *)msg,
										XAP_Dialog_MessageBox::b_O,
										XAP_Dialog_MessageBox::a_OK);
			return;
		}

		getFrame()->repopulateCombos();
		_populateWindowData(); // force a refresh
		getDoc()->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
    }
}

void AP_QNXDialog_Styles::event_NewClicked(void)
{
	setIsNew(true);
	modifyRunModal();
	if(m_answer == AP_Dialog_Styles::a_OK)
	{
		createNewStyle(getNewStyleName());
		_populateCList();
	}
}

void AP_QNXDialog_Styles::event_ClistClicked(int row, int col)
{
	m_whichRow = row;
	m_whichCol = col;

	// refresh the previews
	_populatePreviews(false);
}

void AP_QNXDialog_Styles::event_ListClicked(const char * which)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_LBL_InUse,s);
	if (!strcmp(which, s.utf8_str()))
		m_whichType = USED_STYLES;
	else
	{
		pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_LBL_UserDefined,s);
		if (!strcmp(which, s.utf8_str()))
			m_whichType = USER_STYLES;
		else
			m_whichType = ALL_STYLES;
	}
	
	// force a refresh of everything
	_populateWindowData();
}

/*****************************************************************/

PtWidget_t * AP_QNXDialog_Styles::_constructWindow(void)
{
#if 0
	PtWidget_t * windowStyles;
	PtWidget_t * buttonApply;
	PtWidget_t * buttonClose;

	PtWidget_t * vgroup, * hgroup;

	PtWidget_t *	listStyles;

	PtWidget_t * comboList;

	PtWidget_t * frameParaPrev;
	PtWidget_t * ParaPreviewArea;

	PtWidget_t * frameCharPrev;
	PtWidget_t * CharPreviewArea;

	PtWidget_t * frameDescription;
	PtWidget_t * DescriptionArea;

	PtWidget_t * buttonNew;
	PtWidget_t * buttonModify;
	PtWidget_t * buttonDelete;


	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UTF8String s;
	PtArg_t args[10];
	void *data;
	int n;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, 
_(AP,DLG_Styles_StylesTitle), 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	windowStyles = PtCreateWidget(PtWindow, NULL, n, args);
	SetupContextHelp(windowStyles,this);
	PtAddHotkeyHandler(windowStyles,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);

	PtAddCallback(windowStyles, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 5, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_STRETCH_HORIZONTAL, Pt_GROUP_STRETCH_HORIZONTAL);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EXCLUSIVE, Pt_GROUP_EXCLUSIVE);
	PtSetArg(&args[n++], Pt_ARG_RESIZE_FLAGS, Pt_RESIZE_XY_AS_REQUIRED, 
											  Pt_RESIZE_XY_AS_REQUIRED | Pt_RESIZE_XY_ALWAYS);
	vgroup = PtCreateWidget(PtGroup, windowStyles, n, args);

	/*** ***/

	/* Left pane is a list of styles and types */
	n = 0;
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	PtWidget_t *top_vgrouplist, *top_vgroupshow;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	top_vgrouplist = PtCreateWidget(PtGroup, hgroup, n, args);

#define PREVIEW_WIDTH 175

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, PREVIEW_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 200, 0);
	listStyles = PtCreateWidget(PtList, top_vgrouplist, n, args);
	PtAddCallback(listStyles, Pt_CB_SELECTION, s_clist_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, PREVIEW_WIDTH, 0);
	comboList = PtCreateWidget(PtComboBox, top_vgrouplist, n, args);
	const char *items[3];
	pSS->getValueUTF8 (AP_STRING_ID_DLG_Styles_LBL_InUse,s);
	items[0] = s.utf8_str();
	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_LBL_All,s);
	items[1] = s.utf8_str();
	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_LBL_UserDefined,s);
	items[2] = s.utf8_str();
	PtListAddItems(comboList, items, 3, 0);  
	UT_QNXComboSetPos(comboList, 1);
	PtAddCallback(comboList, Pt_CB_SELECTION, s_typeslist_changed, this);

	/* Previewing and description goes in the top right */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	top_vgroupshow = PtCreateWidget(PtGroup, hgroup, n, args);

	n = 0;
	frameParaPrev = PtCreateWidget(PtGroup, top_vgroupshow, n, args);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ParaPrev,s);
	pretty_group(frameParaPrev, s.utf8_str());
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, PREVIEW_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 70, 0);
	data = (void *)this;
	PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this)); 
	PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &s_paraPreview_exposed, 1); 
	ParaPreviewArea = PtCreateWidget(PtRaw, frameParaPrev, n, args);

	n = 0;
	frameCharPrev = PtCreateWidget(PtGroup, top_vgroupshow, n, args);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_CharPrev,s);
	pretty_group(frameCharPrev, s.utf8_str());
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, PREVIEW_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 50, 0);
	data = (void *)this;
	PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this)); 
	PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &s_charPreview_exposed, 1); 
	CharPreviewArea = PtCreateWidget(PtRaw, frameCharPrev, n, args);

	n = 0;
	frameDescription = PtCreateWidget(PtGroup, top_vgroupshow, n, args);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_Description,s);
	pretty_group(frameDescription, s.utf8_str());
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, PREVIEW_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 60, 0);
	PtSetArg(&args[n++], Pt_ARG_SCROLLBAR_Y_DISPLAY, Pt_AS_REQUIRED, Pt_AS_REQUIRED);
	DescriptionArea = PtCreateWidget(PtMultiText, frameDescription, n, args);

	/* These buttons are above the normal buttons */
	n = 0;
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP,DLG_Styles_New), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonNew = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonNew, Pt_CB_ACTIVATE, s_newbtn_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP,DLG_Styles_Modify), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonModify = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonModify, Pt_CB_ACTIVATE, s_modifybtn_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP,DLG_Styles_Delete), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonDelete = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonDelete, Pt_CB_ACTIVATE, s_deletebtn_clicked, this);

	/* Bottom row of buttons */
	n = 0;
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(XAP,DLG_Close), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonClose = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonClose, Pt_CB_ACTIVATE, s_close_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(XAP,DLG_Apply), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonApply = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonApply, Pt_CB_ACTIVATE, s_apply_clicked, this);

	m_windowMain = windowStyles;
	m_wlistTypes = comboList;
	m_wclistStyles = listStyles;
	m_wParaPreviewArea = ParaPreviewArea;
	m_wCharPreviewArea = CharPreviewArea;
	m_wlabelDesc = DescriptionArea;
	m_wbuttonNew = buttonNew;
	m_wbuttonModify = buttonModify;
	m_wbuttonDelete = buttonDelete;
	m_wbuttonApply = buttonApply;
	m_wbuttonClose = buttonClose;

	return windowStyles;
#endif
}

void AP_QNXDialog_Styles::_populateCList(void) const
{
	const PD_Style * pStyle;
	const char * name = NULL;

	size_t nStyles = getDoc()->getStyleCount();
	xxx_UT_DEBUGMSG(("DOM: we have %d styles\n", nStyles));

	PtListDeleteAllItems(m_wclistStyles);

	for (UT_uint32 i = 0; i < nStyles; i++)
	{
	    const char * data[1];

	    getDoc()->enumStyles((UT_uint32)i, &name, &pStyle);

		// style has been deleted probably
		if (!pStyle)
			continue;

	    // all of this is safe to do... append should take a const char **
	    data[0] = name;

	    if ((m_whichType == ALL_STYLES) || 
			(m_whichType == USED_STYLES && pStyle->isUsed()) ||
			(m_whichType == USER_STYLES && pStyle->isUserDefined()))
		{
			PtListAddItems(m_wclistStyles, data, 1, 0);
		}
	}

	PtListSelectPos(m_wclistStyles, 1);
}

void AP_QNXDialog_Styles::_populateWindowData(void)
{
	_populateCList();
	_populatePreviews(false);
}

void AP_QNXDialog_Styles::setDescription(const char * desc) const
{
	UT_ASSERT(m_wlabelDesc);
	PtSetResource(m_wlabelDesc, Pt_ARG_TEXT_STRING, desc, 0);
}

const char * AP_QNXDialog_Styles::getCurrentStyle (void) const
{
	static UT_UTF8String szStyleBuf;
	char **items;

	UT_ASSERT(m_wclistStyles);

	if (m_whichRow < 0 || m_whichCol < 0)
		return NULL;

	items = NULL;
	PtGetResource(m_wclistStyles, Pt_ARG_ITEMS, &items, 0);

	if(!items) {
		return NULL;
	}

	szStyleBuf = items[m_whichRow + 1];

	return szStyleBuf.utf8_str();
}

/***
 MODIFY PANE
***/

char *_combo_or_text_entry(PtWidget_t *widget, const char *newentry) {
	char *entry = NULL;

	if(PtWidgetIsClass(widget, PtComboBox)) {
		char **items;
		unsigned short *selected;

		if(newentry) {
			if(!PtListItemExists(widget, newentry)) {
				PtListAddItems(widget, &newentry, 1, 0);
			}

			PtSetResource(widget, Pt_ARG_CBOX_SEL_ITEM, 
							PtListItemPos(widget, newentry), 0);
		}

		selected = NULL;
		PtGetResource(widget, Pt_ARG_CBOX_SEL_ITEM, &selected, 0);
		PtGetResource(widget, Pt_ARG_ITEMS, &items, 0);
		if(selected) {
			entry = items[((*selected)-1)];
		} else {
			entry = items[0];	//First item selected by default
		}
	} else {
		if(newentry) {
			PtSetResource(widget, Pt_ARG_TEXT_STRING, newentry, 0);
		}

		PtGetResource(widget, Pt_ARG_TEXT_STRING, &entry, 0);
	}

	return entry;
}

PtWidget_t *  AP_QNXDialog_Styles::_constructModifyDialog(void)
{
#if 0
	PtWidget_t *windowModify;
	PtWidget_t *vgroup, *hgroup;

	PtWidget_t *nameLabel  = NULL;
	PtWidget_t *basedOnLabel  = NULL;
	PtWidget_t *followingLabel = NULL;
	PtWidget_t *styleTypeLabel = NULL;
	PtWidget_t *styleNameEntry = NULL;
	PtWidget_t *basedOnCombo = NULL;
	PtWidget_t *basedOnEntry = NULL;
	PtWidget_t *followingCombo = NULL;
	PtWidget_t *followingEntry = NULL;
	PtWidget_t *styleTypeCombo = NULL;
	PtWidget_t *styleTypeEntry = NULL;
	PtWidget_t *previewFrame = NULL;
	PtWidget_t *modifyDrawingArea = NULL;
	PtWidget_t *descriptionFrame = NULL;
	PtWidget_t *descriptionText = NULL;
	PtWidget_t *deletePropCombo = NULL;
	PtWidget_t *deletePropEntry = NULL;
	PtWidget_t *deletePropButton = NULL;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	PtArg_t args[10];
	void *data;
	int n;

	n = 0;
	if(!isNew()) {
		PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, 
_(AP,DLG_Styles_ModifyTitle), 0);
	} else {
		PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, 
_(AP,DLG_Styles_NewTitle), 0);
	}
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	windowModify = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowModify, Pt_CB_WINDOW_CLOSING, s_modify_delete_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 5, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_STRETCH_HORIZONTAL, Pt_GROUP_STRETCH_HORIZONTAL);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EXCLUSIVE, Pt_GROUP_EXCLUSIVE);
	PtSetArg(&args[n++], Pt_ARG_RESIZE_FLAGS, Pt_RESIZE_XY_AS_REQUIRED, 
											  Pt_RESIZE_XY_AS_REQUIRED | Pt_RESIZE_XY_ALWAYS);
	vgroup = PtCreateWidget(PtGroup, windowModify, n, args);

	/* User selection portion */
	n = 0;
    PtSetArg(&args[n++], Pt_ARG_GROUP_ROWS_COLS, 2, 0);
    PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EQUAL_SIZE_HORIZONTAL, Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP,DLG_Styles_ModifyName), 0);
	nameLabel = PtCreateWidget(PtLabel, hgroup, n, args);

	n = 0;
PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP,DLG_Styles_ModifyType), 0);
	styleTypeLabel = PtCreateWidget(PtLabel, hgroup, n, args);

	n = 0;
	styleNameEntry = PtCreateWidget(PtText, hgroup, n, args);

	n = 0;
	if(isNew()) {
		styleTypeEntry = PtCreateWidget(PtComboBox, hgroup, n, args);
	} else {
		styleTypeEntry = PtCreateWidget(PtText, hgroup, n, args);
	}


	n = 0;
PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP,DLG_Styles_ModifyBasedOn), 0);
	basedOnLabel = PtCreateWidget(PtLabel, hgroup, n, args);

	n = 0;
PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP,DLG_Styles_ModifyFollowing), 0);
	followingLabel = PtCreateWidget(PtLabel, hgroup, n, args);

	n = 0;
	if(isNew()) {
		basedOnEntry = PtCreateWidget(PtComboBox, hgroup, n, args);
	} else {
		basedOnEntry = PtCreateWidget(PtText, hgroup, n, args);
	}

	n = 0;
	if(isNew()) {
		followingEntry = PtCreateWidget(PtComboBox, hgroup, n, args);
	} else {
		followingEntry = PtCreateWidget(PtText, hgroup, n, args);
	}

	/* Preview portion */
	n = 0;
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);
	n = 0;
	previewFrame = PtCreateWidget(PtGroup, hgroup, n, args);
	pretty_group(previewFrame, _(AP,DLG_Styles_ModifyPreview ));
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, PREVIEW_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 120, 0);
	data = (void *)this;
	PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this)); 
	PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &s_modifyPreview_exposed, 1); 
	modifyDrawingArea = PtCreateWidget(PtRaw, previewFrame, n, args);

	/* Description portion */
	n = 0;
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);
	n = 0;
	descriptionFrame = PtCreateWidget(PtGroup, hgroup, n, args);
	pretty_group(descriptionFrame, _(AP,DLG_Styles_ModifyDescription ));
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, PREVIEW_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 120, 0);
	descriptionText = PtCreateWidget(PtMultiText, previewFrame, n, args);

	/* Remove the following properties */
	n = 0;
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP,DLG_Styles_RemoveLab), 0);
	PtCreateWidget(PtLabel, hgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	deletePropEntry = PtCreateWidget(PtComboBox, hgroup, n, args);

	n = 0;
PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP,DLG_Styles_RemoveButton), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	deletePropButton = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(deletePropButton, Pt_CB_ACTIVATE, s_remove_property, this);

	/* Add the bottom group of buttons */
	PtWidget_t *buttonOK;
	PtWidget_t *buttonCancel;
	PtWidget_t *FormatMenu;
	PtWidget_t *shortCutButton;

	n = 0;
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(XAP,DLG_Cancel), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonCancel = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_modify_cancel_clicked, this);

	n = 0;
PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(XAP,DLG_OK), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonOK = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_modify_ok_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	FormatMenu = PtCreateWidget(PtComboBox, hgroup, n, args);

	_constructFormatList(FormatMenu);

	n = 0;
PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP,DLG_Styles_ModifyShortCut), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	shortCutButton = PtCreateWidget(PtButton, hgroup, n, args);
	//PtAddCallback(shortCutButton, Pt_CB_ACTIVATE, s_modify_ok_clicked, this);

	m_wStyleNameEntry = styleNameEntry;
	m_wBasedOnCombo = basedOnCombo;
	m_wBasedOnEntry = basedOnEntry;
    m_wFollowingCombo = followingCombo;
	m_wFollowingEntry = followingEntry;
	m_wStyleTypeCombo = styleTypeCombo;
	m_wStyleTypeEntry = styleTypeEntry;
	m_wModifyDrawingArea = modifyDrawingArea;
	m_wLabDescription = descriptionText;
	m_wModifyDialog = windowModify;
	m_wDeletePropCombo = deletePropCombo;
	m_wDeletePropEntry = deletePropEntry;
	m_wDeletePropButton = deletePropButton;

	m_wModifyOk = buttonOK;
	m_wModifyCancel = buttonCancel;
	m_wFormatMenu = FormatMenu;
	m_wModifyShortCutKey = shortCutButton;
	
	return windowModify;
#endif
}

void  AP_QNXDialog_Styles::_constructFormatList(PtWidget_t * FormatMenu)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UTF8String s;
	const char *item;

	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ModifyFormat,s);
	item = s.utf8_str();
	PtListAddItems(FormatMenu, &item, 1, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ModifyParagraph,s);
	item = s.utf8_str();
	PtListAddItems(FormatMenu, &item, 1, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ModifyFont,s);
	item = s.utf8_str();
	PtListAddItems(FormatMenu, &item, 1, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ModifyTabs,s);
	item = s.utf8_str();
	PtListAddItems(FormatMenu, &item, 1, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ModifyNumbering,s);
	item = s.utf8_str();
	PtListAddItems(FormatMenu, &item, 1, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ModifyLanguage,s);
	item = s.utf8_str();
	PtListAddItems(FormatMenu, &item, 1, 0);

	PtAddCallback(FormatMenu, Pt_CB_SELECTION, s_modify_format, this);
	PtSetResource(FormatMenu, Pt_ARG_CBOX_SEL_ITEM, 1, 0);

	m_wFormat = 
	m_wModifyParagraph = 
	m_wModifyFont = 
	m_wModifyNumbering = 
	m_wModifyTabs = 
	m_wModifyLanguage = NULL;
}

int AP_QNXDialog_Styles::event_ModifySelected(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	unsigned short index;
	PtGetResource(w, Pt_ARG_CBOX_SEL_ITEM, &index, 0);

	switch(index) {
	case 1:
		return Pt_CONTINUE;
	case 2:
		return s_modify_paragraph(w, data, info);
	case 3:
	   return s_modify_font(w, data, info);
	case 4:
		return s_modify_tabs(w, data, info);
	case 5:
		return s_modify_numbering(w, data, info);
	case 6:
		return s_modify_language(w, data, info);
	default:
		return Pt_CONTINUE;
	}
}


void AP_QNXDialog_Styles::event_Modify_OK(void)
{
	const char * text = NULL;

	text = _combo_or_text_entry(m_wStyleNameEntry, NULL);

	if (!text || !strlen (text))
    {
      // error message!
      const XAP_StringSet * pSS = m_pApp->getStringSet ();
      const char * msg = pSS->getValue (AP_STRING_ID_DLG_Styles_ErrBlankName);

      getFrame()->showMessageBox ((const char *)msg,
				  XAP_Dialog_MessageBox::b_O,
				  XAP_Dialog_MessageBox::a_OK);

      return;
    }

	// TODO save out state of radio items
	m_answer = AP_Dialog_Styles::a_OK;
	modifydone++;
}

/*!
 * fill the properties vector with the values the given style.
 */
void AP_QNXDialog_Styles::new_styleName(void)
{
	static char message[200];
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UTF8String s,s1;
	char * psz = NULL;
	psz = _combo_or_text_entry(m_wStyleNameEntry, NULL);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_DefNone,s);
	if(psz && strcmp(psz,s.utf8_str())== 0)
	{
		// TODO: do a real error dialog
		pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ErrNotTitle1,s);
		sprintf(message,"%s%s%s",s.utf8_str(),psz,pSS->getValue(AP_STRING_ID_DLG_Styles_ErrNotTitle2));
		//messageBoxOK((const char *) message);
		return;
	}

	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_DefCurrent,s);
	if(psz && strcmp(psz,s.utf8_str())== 0)
	{
		// TODO: do a real error dialog
		pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ErrNotTitle1,s);
		pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ErrNotTitle2,s1);
		sprintf(message,"%s%s%s",s.utf8_str(),psz,s1.utf8_str());
		//messageBoxOK((const char *) message);
		return;
	}

	snprintf((char *) m_newStyleName,40,"%s",psz);
	addOrReplaceVecAttribs(PT_NAME_ATTRIBUTE_NAME,getNewStyleName());
}

/*!
 * Remove the property from the current style shown in the remove combo box
 */
void AP_QNXDialog_Styles::event_RemoveProperty(void)
{
	char * psz = NULL;
	psz = _combo_or_text_entry(m_wDeletePropEntry, NULL);
	removeVecProp(psz);
	rebuildDeleteProps();
	updateCurrentStyle();
}

void AP_QNXDialog_Styles::rebuildDeleteProps(void)
{
}

/*!
 * Update the properties and Attributes vector given the new basedon name
 */
void AP_QNXDialog_Styles::event_basedOn(void)
{
	char * psz = NULL;
	psz = _combo_or_text_entry(m_wBasedOnEntry, NULL);

	snprintf((char *) m_basedonName,40,"%s",psz);
	addOrReplaceVecAttribs("basedon",getBasedonName());
	fillVecWithProps(getBasedonName(),false);
	updateCurrentStyle();
}


/*!
 * Update the Attributes vector given the new followedby name
 */
void AP_QNXDialog_Styles::event_followedBy(void)
{
	char * psz = NULL;
	psz = _combo_or_text_entry(m_wFollowingEntry, NULL);

	snprintf((char *) m_followedbyName,40,"%s",psz);
	addOrReplaceVecAttribs("followedby",getFollowedbyName());
}


/*!
 * Update the Attributes vector given the new Style Type
 */
void AP_QNXDialog_Styles::event_styleType(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UTF8String s;
	char * psz = NULL;
	psz = _combo_or_text_entry(m_wStyleTypeEntry, NULL);

	snprintf((char *) m_styleType,40,"%s",psz);
	const gchar * pszSt = "P";
	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ModifyCharacter,s);
	if(strstr(m_styleType, s.utf8_str()) != 0)
		pszSt = "C";
	addOrReplaceVecAttribs("type",pszSt);
}

void AP_QNXDialog_Styles::event_Modify_Cancel(void)
{
	if(!modifydone++) {
		m_answer = AP_Dialog_Styles::a_CANCEL;
	}
}

void AP_QNXDialog_Styles::event_ModifyDelete(void)
{
	if(!modifydone++) {
		m_answer = AP_Dialog_Styles::a_CANCEL;
	}
}

//
// pointer to the widget is stored in m_wModifyDialog
//
void  AP_QNXDialog_Styles::modifyRunModal(void)
{
	PtWidget_t *mainWindow;

	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)getFrame()->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);
	
	PtSetParentWidget(parentWindow);

	m_wModifyDialog =
	mainWindow = _constructModifyDialog();

	connectFocus(m_wModifyDialog, getFrame());

    if(!_populateModify())
	{
		PtDestroyWidget(m_wModifyDialog);
		return;
	}

	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);
	PtRealizeWidget(mainWindow);

	// make a new QNX GC
	unsigned short w, h;
	XAP_QNXApp * app = static_cast<XAP_QNXApp *> (m_pApp);

	UT_ASSERT(app);
	UT_ASSERT(m_wModifyDrawingArea);

	DELETEP (m_pAbiPreviewGR);
	GR_QNXAllocInfo ai(mainWindow, m_wParaPreviewArea, m_pApp);
	m_pAbiPreviewGR = (GR_QNXGraphics*) XAP_App::getApp()->newGraphics(ai);	

	UT_QNXGetWidgetArea(m_wModifyDrawingArea, NULL, NULL, &w, &h);
	_createAbiPreviewFromGC(m_pAbiPreviewGR, w, h);

    _populateAbiPreview(isNew());
	event_ModifyPreviewExposed();

	int count;
	count = PtModalStart();
	modifydone = 0;
	while(!modifydone) {
		PtProcessEvent();
	}
	PtModalEnd(MODAL_END_ARG(count));

/*
	if(m_wModifyDialog) 
	{
		if(m_gbasedOnStyles != NULL)
		{	
			//g_list_free (m_gbasedOnStyles);
			m_gbasedOnStyles = NULL;
		}

		if(m_gfollowedByStyles != NULL)
		{
			//g_list_free (m_gfollowedByStyles);
			m_gfollowedByStyles = NULL;
		}

		if(m_gStyleType != NULL)
		{
			//g_list_free (m_gStyleType);
			m_gStyleType = NULL;
		}
	}
*/

	destroyAbiPreview();
	DELETEP(m_pAbiPreviewGR);

	UT_QNXBlockWidget(parentWindow, 0);
	PtDestroyWidget(mainWindow);
}

void AP_QNXDialog_Styles::event_ModifyPreviewExposed(void)
{
	drawLocal();
}

void AP_QNXDialog_Styles::event_ModifyClicked(void)
{
}

void  AP_QNXDialog_Styles::setModifyDescription( const char * desc)
{
	PtSetResource(m_wLabDescription, Pt_ARG_TEXT_STRING, desc, 0);
}

bool  AP_QNXDialog_Styles::_populateModify(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UTF8String s;
	
	//
	// Don't do any callback while setting up stuff here.
	//
	setModifySignalBlocked(true);
	setModifyDescription( m_curStyleDesc.c_str());

	//
	// Get Style name and put in in the text entry
	//
	const char * szCurrentStyle = NULL;
	if(!isNew())
	{
		szCurrentStyle= getCurrentStyle();
		if(!szCurrentStyle)
		{
			// TODO: change me to use a real messagebox
//messageBoxOK( pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ErrNoStyle ).utf8_str();
			m_answer = AP_Dialog_Styles::a_CANCEL;
			return false;
		}
		_combo_or_text_entry(m_wStyleNameEntry, getCurrentStyle());
	}
	else
	{
	}

	//
	// Next interogate the current style and find the based on and followed by
	// Styles
	//
	const char * szBasedOn = NULL;
	const char * szFollowedBy = NULL;
	PD_Style * pBasedOnStyle = NULL;
	PD_Style * pFollowedByStyle = NULL;
	if(!isNew())
	{
		PD_Style * pStyle = NULL;
		if(szCurrentStyle)
			getDoc()->getStyle(szCurrentStyle,&pStyle);
		if(!pStyle)
		{
			// TODO: do a real error dialog
//messageBoxOK( pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ErrStyleNot ).utf8_str();
			m_answer = AP_Dialog_Styles::a_CANCEL;
			return false;
		}
		//
		// Valid style get the Based On and followed by values
		//
	    pBasedOnStyle = pStyle->getBasedOn();
		pFollowedByStyle = pStyle->getFollowedBy();
	}

	//
	// Next make a glists of all styles and attach them to the BasedOn and FollowedBy
	//
	size_t nStyles = getDoc()->getStyleCount();
	const char * name = NULL;
	const PD_Style * pcStyle = NULL;
	for (UT_uint32 i = 0; i < nStyles; i++)
	{
	    getDoc()->enumStyles(i, &name, &pcStyle);

		if(pBasedOnStyle && pcStyle == pBasedOnStyle) {
			szBasedOn = name;
		}

		if(pFollowedByStyle && pcStyle == pFollowedByStyle) {
			szFollowedBy = name;
		}

		if(szCurrentStyle && strcmp(name,szCurrentStyle) != 0) {
			PtListAddItems(m_wBasedOnEntry, &name, 1, 0);
		} else if(szCurrentStyle == NULL) {
			PtListAddItems(m_wBasedOnEntry, &name, 1, 0);
		}

		PtListAddItems(m_wFollowingEntry, &name, 1, 0);
	}

	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_DefCurrent,s);
	const char *item = s.utf8_str();
	_combo_or_text_entry(m_wFollowingEntry, item); 

	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_DefNone,s);
	item = s.utf8_str();
	_combo_or_text_entry(m_wBasedOnEntry, item); 

	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ModifyParagraph,s);
	item = s.utf8_str();
	_combo_or_text_entry(m_wStyleTypeEntry, item); 

	pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ModifyCharacter,s);
	item = s.utf8_str();
	_combo_or_text_entry(m_wStyleTypeEntry, item); 
 
	/*
	if(isNew()) {
		PtSetResource(m_wStyleTypeCombo, Pt_ARG_CBOX_SEL_ITEM, 1, 0);
	}
	*/

	//
	// OK here we set intial values for the basedOn and followedBy
	//
	if(!isNew())
	{
		if(pBasedOnStyle != NULL)
			_combo_or_text_entry(m_wBasedOnEntry, szBasedOn);
		else
		{
			pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_DefNone,s);
			_combo_or_text_entry(m_wBasedOnEntry, s.utf8_str());
		}
		
		if(pFollowedByStyle != NULL)
			_combo_or_text_entry(m_wFollowingEntry, szFollowedBy);
		else
		{
			pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_DefCurrent,s);
			_combo_or_text_entry(m_wFollowingEntry, s.utf8_str());
		}
		
		if(strstr(getAttsVal("type"),"P") != 0)
		{
			pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ModifyParagraph,s);
			_combo_or_text_entry(m_wStyleTypeEntry, s.utf8_str());
		}
		else
		{
			pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ModifyCharacter,s);
			_combo_or_text_entry(m_wStyleTypeEntry, s.utf8_str());
		}
	}
	else
	{
		//
		// Hardwire defaults for "new"
		//
		pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_DefNone,s);
		_combo_or_text_entry(m_wBasedOnEntry, s.utf8_str());
		pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_DefCurrent,s);
		_combo_or_text_entry(m_wFollowingEntry, s.utf8_str());
		pSS->getValueUTF8(AP_STRING_ID_DLG_Styles_ModifyParagraph,s);
		_combo_or_text_entry(m_wStyleTypeEntry, s.utf8_str());
	}

	//
	// Set these in our attributes vector
	//
	event_basedOn();
	event_followedBy();
	event_styleType();
	if(isNew())
	{
		fillVecFromCurrentPoint();
	}
	else
	{
		fillVecWithProps(szCurrentStyle,true);
	}
	//
	// Allow callback's now.
	//
	setModifySignalBlocked(false);

	//
	// Now set the list of properties which can be deleted.
	//
	rebuildDeleteProps();
	_combo_or_text_entry(m_wDeletePropEntry, ""); 
	return true;
}

void   AP_QNXDialog_Styles::event_ModifyParagraph()
{
}

void   AP_QNXDialog_Styles::event_ModifyFont()
{
}

void AP_QNXDialog_Styles::event_ModifyLanguage()
{
}

void   AP_QNXDialog_Styles::event_ModifyNumbering()
{
}


void   AP_QNXDialog_Styles::event_ModifyTabs()
{
}

bool  AP_QNXDialog_Styles::isModifySignalBlocked(void) const
{
	return m_bBlockModifySignal;
}

void  AP_QNXDialog_Styles::setModifySignalBlocked( bool val)
{
	m_bBlockModifySignal = val;
}









