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


#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Lists.h"
#include "ap_QNXDialog_Lists.h"

#include "ut_qnxHelper.h"

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Lists::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_Lists * p = new AP_QNXDialog_Lists(pFactory,id);
	return p;
}

AP_QNXDialog_Lists::AP_QNXDialog_Lists(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Lists(pDlgFactory,id)
{
	//Manually set this for now
	m_newListType = m_iListType = NUMBERED_LIST;
}

AP_QNXDialog_Lists::~AP_QNXDialog_Lists(void)
{
}

/**********************************************************************/

static int s_customChanged(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	dlg->setDirty();
	dlg->customChanged();
	return Pt_CONTINUE;
}

/*
 The style is the sub-type of the list.
*/
static int s_styleChanged (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	PtListCallback_t *lcb = (PtListCallback_t *)info->cbdata;

	if (info->reason == Pt_CB_SELECTION && info->reason_subtype != Pt_LIST_SELECTION_FINAL) {
		return Pt_CONTINUE;
	}

  	dlg->setDirty();
	dlg->setXPFromLocal();
	dlg->previewExposed();
	
	return Pt_CONTINUE;
}

/*
 The type is the main type of the list: None, Numbered, Bulleted
*/
static int s_typeChanged(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	PtListCallback_t *lcb = (PtListCallback_t *)info->cbdata;

	if (info->reason == Pt_CB_SELECTION && info->reason_subtype != Pt_LIST_SELECTION_FINAL) {
		return Pt_CONTINUE;
	}

/*
	if(dlg->dontUpdate()) {
		return Pt_CONTINUE;
	}
*/

	dlg->setDirty();
	/* 0: None 1: Bullet 2: Numbered */
	dlg->styleChanged(lcb->item_pos - 1);

	return Pt_CONTINUE;
}

static int s_applyClicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	dlg->applyClicked();
	return Pt_CONTINUE;
}

static int s_closeClicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	dlg->destroy();
	return Pt_CONTINUE;
}

static int s_deleteClicked (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	dlg->destroy();
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

	AP_QNXDialog_Lists *pQNXDlg, **ppQNXDlg = NULL;
	PtSetArg(&args[0], Pt_ARG_USER_DATA, &ppQNXDlg, 0);
	PtGetResources(w, 1, args);
	pQNXDlg = (ppQNXDlg) ? *ppQNXDlg : NULL;

	UT_ASSERT(pQNXDlg);
	pQNXDlg->previewExposed();

    PtClipRemove();
	return Pt_CONTINUE;
}

static int s_update (void)
{
	printf("TODO: Static update \n");
//	Current_Dialog->updateDialog();
	return true;
}


/**********************************************************************/

void AP_QNXDialog_Lists::activate()
{
	ConstructWindowName();
	PtSetResource(m_mainWindow, Pt_ARG_WINDOW_TITLE, m_WindowName, NULL);
	m_bDontUpdate = false;
	updateDialog();
	//Raise the window ...
}

void AP_QNXDialog_Lists::destroy()
{
	if (!m_mainWindow) {
		return;
	}
	m_bDestroy_says_stopupdating = true;
	while (m_bAutoUpdate_happening_now == true) ;
	m_pAutoUpdateLists->stop();
	m_answer = AP_Dialog_Lists::a_CLOSE;	
	modeless_cleanup();
	
	PtWidget_t *tmp = m_mainWindow;
	m_mainWindow = NULL;
	PtDestroyWidget(tmp);
}

void AP_QNXDialog_Lists::notifyActiveFrame(XAP_Frame *pFrame)
{
	activate();
}

void AP_QNXDialog_Lists::runModeless(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	m_mainWindow = _constructWindow ();
	UT_ASSERT (m_mainWindow);

	clearDirty();

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid = (UT_sint32) getDialogId ();
	m_pApp->rememberModelessId(sid, (XAP_Dialog_Modeless *) m_pDialog);
 
	// This magic command displays the frame that characters will be
	// inserted into.
    // This variation runs the additional static function shown afterwards.
    // Only use this if you need to to update the dialog upon focussing.
	//connectFocusModelessOther (m_mainWindow, m_pApp, s_update);
	connectFocusModeless(m_mainWindow, m_pApp);

	{
		// attach a new graphics context to the drawing area
		XAP_QNXApp * app;
		app = static_cast<XAP_QNXApp *> (m_pApp);
		UT_ASSERT(app);

		UT_ASSERT(m_wPreviewArea);

		// make a new QNX GC
		m_pPreviewWidget = new GR_QNXGraphics(m_mainWindow, m_wPreviewArea, pFrame->getApp());
		unsigned short w, h;

		// let the widget materialize
		UT_QNXGetWidgetArea(m_wPreviewArea, NULL, NULL, &w, &h);
		_createPreviewFromGC(m_pPreviewWidget, w, h);

	}

	// Populate the dialog
	updateDialog();
	m_bDontUpdate = false;

	// Now Display the dialog
	UT_QNXCenterWindow(NULL, m_mainWindow);
	PtRealizeWidget(m_mainWindow);

	// Next construct a timer for auto-updating the dialog
	m_pAutoUpdateLists = UT_Timer::static_constructor(autoupdateLists,this);
	m_bDestroy_says_stopupdating = false;

	// OK fire up the auto-updater for 0.5 secs
	//m_pAutoUpdateLists->set(500);
}

void    AP_QNXDialog_Lists::autoupdateLists(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);
	// this is a static callback method and does not have a 'this' pointer.
	AP_QNXDialog_Lists * pDialog =  (AP_QNXDialog_Lists *) pTimer->getInstanceData();
	// Handshaking code

	if(pDialog->isDirty()) {
		return;
	}

/*
	if(pDialog->getAvView()->getTick() != pDialog->getTick())
	{
		pDialog->setTick(pDialog->getAvView()->getTick());
		if( pDialog->m_bDestroy_says_stopupdating != true)
		{
			pDialog->m_bAutoUpdate_happening_now = true;
			pDialog->updateDialog();
			pDialog->previewExposed();
			pDialog->m_bAutoUpdate_happening_now = false;
		}
	}
*/
}

void AP_QNXDialog_Lists::previewExposed(void)
{
	if(m_pPreviewWidget) {
		m_bisCustomized = true; //Why is this so?
		event_PreviewAreaExposed();
	}
} 


/*
 THIS FUNCTION IS MIS-NAMED:  This is really the case of the
_type_ changing, which results in a changing of the styles
combo box.
*/
void AP_QNXDialog_Lists::styleChanged(int type)
{
	//Fill in the entries for the appropriate list
	switch(type) {
	case 0:
		_fillNoneStyleMenu(m_wListStyle_menu);
		m_newListType = NOT_A_LIST;
		break;
	case 1:
		_fillBulletedStyleMenu(m_wListStyle_menu);
        m_newListType = BULLETED_LIST;
		break;
	case 2:
		_fillNumberedStyleMenu(m_wListStyle_menu);
        m_newListType = NUMBERED_LIST;
		break;
	default:
		UT_ASSERT(0);
	}

	//When we re-fill the style list, set it to the first item
	UT_QNXComboSetPos(m_wListStyle_menu, 1);

	setXPFromLocal();
	previewExposed();
}

void  AP_QNXDialog_Lists::setXPFromLocal(void)
{
	unsigned short *value;

	//
	// Failsafe code to make sure the start, stop and change flags are set
	// as shown on the GUI.
	//
	PtGetResource(m_wListStyle_menu, Pt_ARG_CBOX_SEL_ITEM, &value, 0);
	if (*value > m_styleVector.getItemCount() || *value == 0) {
		UT_ASSERT(0);
		m_newListType = NOT_A_LIST;
	} else {
		void *junk;
		junk = m_styleVector.getNthItem(*value - 1);
		m_newListType = (enum List_Type)((int)junk);
	}
	printf("setMemberVariable newListType to %d from index %d \n", m_newListType, *value);

	_gatherData();

	m_bStartNewList = false;
	m_bApplyToCurrent = false;
	m_bStartSubList = false;

	PtGetResource(m_wStartNewList, Pt_ARG_FLAGS, &value, 0);
	if ((*value) & Pt_SET) {
		m_bStartNewList = true;
	}

	PtGetResource(m_wApplyCurrent, Pt_ARG_FLAGS, &value, 0);
	if ((*value) & Pt_SET) {
		m_bApplyToCurrent = true;
	}

	PtGetResource(m_wStartSubList, Pt_ARG_FLAGS, &value, 0);
	if ((*value) & Pt_SET) {
		m_bStartSubList = true;
	}
}


void  AP_QNXDialog_Lists::applyClicked(void)
{
	setXPFromLocal();
	previewExposed();
	Apply();
}

void  AP_QNXDialog_Lists::customChanged(void)
{
	fillUncustomizedValues();
	_loadXPDataIntoLocal();
}

void AP_QNXDialog_Lists::updateFromDocument(void)
{
	PopulateDialogData();
	_setRadioButtonLabels();
	m_newListType = m_iListType;
	_loadXPDataIntoLocal();
}

void AP_QNXDialog_Lists::updateDialog(void)
{
	if(!isDirty()) {
		updateFromDocument();
	} else {
		setXPFromLocal();
	}
}

#if 0
void AP_UnixDialog_Lists::setAllSensitivity(void)
{ 
	PopulateDialogData();
	if(m_isListAtPoint == true)
	{
	}
}
#endif

PtWidget_t * AP_QNXDialog_Lists::_constructWindow (void)
{
	PtWidget_t *vgroup;
	PtWidget_t *lblStyle, *listStyle;
	PtWidget_t *lblType, *listType;
	PtWidget_t *togCustomize, *grpCustomize;
	PtWidget_t *lblFormat, *lblFormat2;
	PtWidget_t *numListLevel, *numListAlign, *numIndentAlign, *numStart;
	PtWidget_t *radnewlist, *radexisting, *radsublist;
	PtWidget_t *butOK, *butCancel;

   	PtArg_t    	args[10];
	int			n;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	n = 0;
	ConstructWindowName();
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, m_WindowName, 0);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	m_mainWindow = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(m_mainWindow, Pt_CB_WINDOW_CLOSING, s_deleteClicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_STRETCH_HORIZONTAL, Pt_GROUP_STRETCH_HORIZONTAL);
   	vgroup = PtCreateWidget(PtGroup, m_mainWindow, n, args);

	PtWidget_t *hgroup;
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, 10, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, 10, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING, 5, 0);
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	/*** Create the controls in a vertical group here ***/
	PtWidget_t *ctlgroup;
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING, 10, 0);
	ctlgroup = PtCreateWidget(PtGroup, hgroup, n, args);

	PtWidget_t *group;
	n = 0;
	group = PtCreateWidget(PtGroup, ctlgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Type), 0);
	lblType = PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2*ABI_DEFAULT_BUTTON_WIDTH, 0);
	listType = PtCreateWidget(PtComboBox, group, n, args);	
	PtAddCallback(listType, Pt_CB_SELECTION, s_typeChanged, this);

	const char *text;
	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Type_none);
	PtListAddItems(listType, &text, 1, 0);
	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Type_bullet);
	PtListAddItems(listType, &text, 1, 0);
	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Type_numbered);
	PtListAddItems(listType, &text, 1, 0);

	n = 0;
	group = PtCreateWidget(PtGroup, ctlgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Style), 0);
	lblStyle = PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 2*ABI_DEFAULT_BUTTON_WIDTH, 0);
	listStyle = PtCreateWidget(PtComboBox, group, n, args);	
	PtAddCallback(listStyle, Pt_CB_SELECTION, s_styleChanged, this);

#if 0
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Customize), 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_OUTLINE, 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_SET);
	togCustomize = PtCreateWidget(PtToggleButton, ctlgroup, n, args);	
	PtAddCallback(togCustomize, Pt_CB_ACTIVATE, s_customChanged, this);
	m_bisCustomFrameHidden = true;
#endif

	n = 0;
#define OUTLINE_GROUP (Pt_TOP_OUTLINE | Pt_TOP_BEVEL | \
                                           Pt_BOTTOM_OUTLINE | Pt_BOTTOM_BEVEL | \
                                           Pt_LEFT_OUTLINE | Pt_LEFT_BEVEL | \
                                           Pt_RIGHT_OUTLINE | Pt_RIGHT_BEVEL)
	PtSetArg(&args[n++], Pt_ARG_BASIC_FLAGS, OUTLINE_GROUP, OUTLINE_GROUP);
	PtSetArg(&args[n++], Pt_ARG_BEVEL_WIDTH, 1, 0);
/*
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_DELAY_REALIZE | Pt_HIGHLIGHTED, Pt_DELAY_REALIZE | Pt_HIGHLIGHTED);
*/
	grpCustomize = PtCreateWidget(PtGroup, ctlgroup, n, args);
	pretty_group(grpCustomize, pSS->getValue(AP_STRING_ID_DLG_Lists_Customize));

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING, 5, 0);
	group = PtCreateWidget(PtGroup, grpCustomize, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Format), 0);
	PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Font), 0);
	PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Level), 0);
	PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Start), 0);
	PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Align), 0);
	PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Indent), 0);
	PtCreateWidget(PtLabel, group, n, args);	

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	group = PtCreateWidget(PtGroup, grpCustomize, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 1.5*ABI_DEFAULT_BUTTON_WIDTH, 0);
	lblFormat = PtCreateWidget(PtText, group, n, args);	
#if 0
	/* This is for something ... I just don't know what */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 1.5*ABI_DEFAULT_BUTTON_WIDTH, 0);
	lblFormat2 = PtCreateWidget(PtText, group, n, args);	
#endif

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtWidget_t *font = PtCreateWidget(PtComboBox, group, n, args);	
	//TODO: Fill this with the current fonts 
	{
		text = "Current Font";
		PtListAddItems(font, &text, 1, 0);
	}

	n = 0;
	numListLevel = PtCreateWidget(PtNumericInteger, group, n, args);	
	PtAddCallback(numListLevel, Pt_CB_NUMERIC_CHANGED, s_customChanged, this);
	n = 0;
	numStart = PtCreateWidget(PtNumericInteger, group, n, args);	
	PtAddCallback(numStart, Pt_CB_NUMERIC_CHANGED, s_customChanged, this);
	n = 0;
	numListAlign = PtCreateWidget(PtNumericFloat, group, n, args);	
	PtAddCallback(numListAlign, Pt_CB_NUMERIC_CHANGED, s_customChanged, this);
	n = 0;
	numIndentAlign = PtCreateWidget(PtNumericFloat, group, n, args);	
	PtAddCallback(numIndentAlign, Pt_CB_NUMERIC_CHANGED, s_customChanged, this);

	/*** Create the preview in the next dialog ***/
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  200, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT,  300, 0);
	m_wPreviewGroup = PtCreateWidget(PtGroup, hgroup, n, args);
	pretty_group(m_wPreviewGroup, "Preview");

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  200, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT,  300, 0);
	void *data = (void *)this;
	PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this)); 
	PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &s_preview_exposed, 1); 
	m_wPreviewArea = PtCreateWidget(PtRaw, m_wPreviewGroup, n, args);

	/*** Create the radio buttons below this group ***/
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EXCLUSIVE, Pt_GROUP_EXCLUSIVE);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING, 5, 0);
	group = PtCreateWidget(PtGroup, vgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Start_New), 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_SET, Pt_SET);
	radnewlist = PtCreateWidget(PtToggleButton, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Apply_Current), 0);
	radexisting = PtCreateWidget(PtToggleButton, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Start_Sub), 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	radsublist = PtCreateWidget(PtToggleButton, group, n, args);	
		
	/*** Then we have the final cancellation buttons ***/
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_HORZ_ALIGN, Pt_GROUP_HORZ_RIGHT, 0);
	group = PtCreateWidget(PtGroup, vgroup, n, args);
/*
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 200, 0);
	PtCreateWidget(PtLabel, group, n, args);	
*/
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_Apply), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	butOK = PtCreateWidget(PtButton, group, n, args);	
	PtAddCallback(butOK, Pt_CB_ACTIVATE, s_applyClicked, this);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_Close), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	butCancel = PtCreateWidget(PtButton, group, n, args);	
	PtAddCallback(butCancel, Pt_CB_ACTIVATE, s_closeClicked, this);

	/** Done **/
	m_wDelimEntry = lblFormat;
	m_wDecimalEntry = lblFormat2;

	m_wCustomFrame = grpCustomize;
	m_wCustomLabel = togCustomize;

	m_wListStyle_menu = listStyle;
	m_wListType_menu = listType;

	m_wListStyleBox = lblFormat;
	m_wLevelSpin = numListLevel;
	m_wStartSpin = numStart;
	m_wAlignListSpin = numListAlign;
	m_wIndentAlignSpin = numIndentAlign;

	m_wStartNewList = radnewlist;
	m_wApplyCurrent = radexisting;
	m_wStartSubList = radsublist;

	m_wApply = butOK;
	m_wClose = butCancel;

	return m_mainWindow;
}

void AP_QNXDialog_Lists::_fillNoneStyleMenu( PtWidget_t *listmenu)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	const char *text;

	PtListDeleteAllItems(listmenu);
	m_styleVector.clear();

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Type_none);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)NOT_A_LIST);
}

void AP_QNXDialog_Lists::_fillNumberedStyleMenu( PtWidget_t *listmenu)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	const char *text;

	PtListDeleteAllItems(listmenu);
	m_styleVector.clear();

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Numbered_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)NUMBERED_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Case_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)LOWERCASE_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Case_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)UPPERCASE_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Roman_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)LOWERROMAN_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Roman_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)UPPERROMAN_LIST);
}

void AP_QNXDialog_Lists::_fillBulletedStyleMenu( PtWidget_t *listmenu)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	const char *text;

	PtListDeleteAllItems(listmenu);
	m_styleVector.clear();

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Bullet_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)BULLETED_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Dashed_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)DASHED_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Square_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)SQUARE_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Triangle_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)TRIANGLE_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Diamond_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)DIAMOND_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Star_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)STAR_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Implies_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)IMPLIES_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Tick_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)TICK_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Box_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)BOX_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Hand_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)HAND_LIST);

	text = pSS->getValue(AP_STRING_ID_DLG_Lists_Heart_List);
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)HEART_LIST);
}

void AP_QNXDialog_Lists::_setRadioButtonLabels(void) 
{
	//	char *tmp;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	PopulateDialogData();
	// Button 0 is Start New List, button 2 is resume list
	PtSetResource(m_wStartNewList, Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Start_New), 0);
	PtSetResource(m_wStartSubList, Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Resume), 0);
/*
	gtk_label_set_text( GTK_LABEL(m_wStartNew_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Start_New));
	gtk_label_set_text( GTK_LABEL(m_wStartSub_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Resume));
*/
}

//
// This function reads the various memeber variables and loads them into
// into the dialog variables.
//
void AP_QNXDialog_Lists::_loadXPDataIntoLocal(void)
{
	int i;
	double d;

	m_bDontUpdate = true;

	UT_DEBUGMSG(("_loadXP newListType = %d \n",m_newListType));
	
	//PtSetResource(m_wLevelSpin, Pt_ARG_NUMERIC_VALUE, m_iLevel, 0);

	d = m_fAlign;
	PtSetResource(m_wAlignListSpin, Pt_ARG_NUMERIC_VALUE, &d, 0);

	d = m_fAlign + m_fIndent;
	if(d < 0.0) {
		m_fIndent = -m_fAlign;
		d = 0.0;
		PtSetResource(m_wIndentAlignSpin, Pt_ARG_NUMERIC_VALUE, &d, 0);
	} else {
		PtSetResource(m_wIndentAlignSpin, Pt_ARG_NUMERIC_VALUE, &d, 0); 
	}

	//
	// Code to work out which is active Font
	//
#if 0
	if(strcmp((char *) m_pszFont,"NULL") == 0 )
	{
		gtk_option_menu_set_history (GTK_OPTION_MENU (m_wFontOptions), 0 );
	}
	else
	{
		for(i=0; i < (gint) g_list_length(m_glFonts);i++)
		{
			if(strcmp((char *) m_pszFont,(char *) g_list_nth_data(m_glFonts,i)) == 0)
				break;
		}
		if(i < (gint) g_list_length(m_glFonts))
		{
			gtk_option_menu_set_history (GTK_OPTION_MENU (m_wFontOptions), i+ 1 );
		}
		else
		{
			gtk_option_menu_set_history (GTK_OPTION_MENU (m_wFontOptions), 0 );
		}
	}
#endif

	PtSetResource(m_wStartSpin, Pt_ARG_NUMERIC_VALUE, m_iStartValue, 0);

#if 0
	PtSetResource(m_wDecimalEntry, Pt_ARG_TEXT_STRING, m_pszDecimal, 0);
#endif
	PtSetResource(m_wDelimEntry, Pt_ARG_TEXT_STRING, m_pszDelim, 0);

	//
	// Now set the list type and style
	List_Type save = m_newListType;
	if(m_newListType == NOT_A_LIST) {
		styleChanged(0);
		m_newListType = save;
		PtSetResource(m_wListType_menu, Pt_ARG_CBOX_SEL_ITEM, 1, 0);
	} else if(m_newListType >= BULLETED_LIST) {
		styleChanged(1);
		m_newListType = save;
		PtSetResource(m_wListType_menu, Pt_ARG_CBOX_SEL_ITEM, 2, 0);
	} else {
		styleChanged(2);
		m_newListType = save;
		PtSetResource(m_wListType_menu, Pt_ARG_CBOX_SEL_ITEM, 3, 0);
	}

	/* Determine which of the styles should be set */
	for (i=0; i<m_styleVector.getItemCount(); i++) {
		void *junk;
		junk = m_styleVector.getNthItem(i);
		if ((enum List_Type)((int)junk) == m_newListType) {
			printf("Matched style %d \n", m_newListType);
			PtSetResource(m_wListStyle_menu, Pt_ARG_CBOX_SEL_ITEM, i+1, 0);
			break;
		}
	}

	//
	// HACK to allow an update during this method
	//
	m_bDontUpdate = false;
}

bool AP_QNXDialog_Lists::dontUpdate(void)
{
        return m_bDontUpdate;
}

void AP_QNXDialog_Lists::_gatherData(void)
{
	double indent;
	int *idata;
	double *ddata;
	char   *sdata;

/*
	PtGetResource(m_wLevelSpin, Pt_ARG_NUMERIC_VALUE, &idata, 0);
	m_iLevel = *idata;
*/
	m_iLevel = 1;

	PtGetResource(m_wAlignListSpin, Pt_ARG_NUMERIC_VALUE, &ddata, 0);
	m_fAlign = *ddata;

	PtGetResource(m_wIndentAlignSpin, Pt_ARG_NUMERIC_VALUE, &ddata, 0);
	indent = *ddata;

	m_fIndent = indent - m_fAlign;

	if( (m_fIndent + m_fAlign) < 0.0) {
		m_fIndent = - m_fAlign;
		indent = 0.0;
		PtSetResource(m_wIndentAlignSpin, Pt_ARG_NUMERIC_VALUE, &indent, sizeof(indent));
	}

#if 0
	PtWidget_t * wfont = gtk_menu_get_active(GTK_MENU(m_wFontOptions_menu));
	gint ifont =  GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(wfont)));
	if(ifont == 0)
	{
                 UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *)  "NULL");
	}
	else
	{
                 UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *)  g_list_nth_data(m_glFonts, ifont-1));
	}
#else
	UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *)  "NULL");
#endif


	PtGetResource(m_wStartSpin, Pt_ARG_NUMERIC_VALUE, &idata, 0);
	m_iStartValue =  *idata;

	sdata = NULL;
	PtGetResource(m_wDelimEntry, Pt_ARG_TEXT_STRING, &sdata, 0);
	UT_XML_strncpy((XML_Char *)m_pszDelim, 80, (const XML_Char *) sdata);

#if 0
	sdata = NULL;
	PtGetResource(m_wDecimalEntry, Pt_ARG_TEXT_STRING, &sdata, 0);
	UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
#else
	UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
#endif
}


