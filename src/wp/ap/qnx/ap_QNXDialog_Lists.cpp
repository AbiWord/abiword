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
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

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
	//m_newListType = m_iListType = NUMBERED_LIST;
}

AP_QNXDialog_Lists::~AP_QNXDialog_Lists(void)
{
}

/**********************************************************************/

static int s_somethingChanged(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	dlg->setDirty();
	dlg->setXPFromLocal();
	dlg->previewExposed();

	return Pt_CONTINUE;
}


/*
 The style is the sub-type of the list.
*/
static int s_styleChanged (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;

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
   	PhRect_t rect;
   	PtSuperClassDraw(PtBasic, w, damage);
   	PtBasicWidgetCanvas(w, &rect);
	//clip to our basic canvas (it's only polite).
    PtClipAdd( w, &rect );

	AP_QNXDialog_Lists *pQNXDlg;
	PtGetResource(w, Pt_ARG_POINTER, &pQNXDlg,0);

	UT_ASSERT(pQNXDlg);
	pQNXDlg->previewExposed();

    PtClipRemove();
	return Pt_CONTINUE;
}

/**********************************************************************/

void AP_QNXDialog_Lists::activate()
{
	ConstructWindowName();
	PtSetResource(m_mainWindow, Pt_ARG_WINDOW_TITLE, getWindowName(), NULL);
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
	m_pAutoUpdateLists->stop();
	setAnswer(AP_Dialog_Lists::a_CLOSE);
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
		//m_pPreviewWidget = new GR_QNXGraphics(m_mainWindow, m_wPreviewArea, pFrame->getApp());
		GR_QNXAllocInfo ai(m_mainWindow, m_wPreviewArea, pFrame->getApp());
		m_pPreviewWidget = (GR_QNXGraphics*) XAP_App::getApp()->newGraphics(ai);

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

void    AP_QNXDialog_Lists::autoupdateLists(UT_Worker * pTimer)
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
		setbisCustomized(true);
		event_PreviewAreaExposed();
	}
}


void AP_QNXDialog_Lists::setFoldLevelInGUI(void)
{
	UT_ASSERT(0);
}

bool AP_QNXDialog_Lists::isPageLists(void)
{
	UT_ASSERT(0);
	return true;
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
		setNewListType(NOT_A_LIST);
		break;
	case 1:
		_fillBulletedStyleMenu(m_wListStyle_menu);
        setNewListType(BULLETED_LIST);
		break;
	case 2:
		_fillNumberedStyleMenu(m_wListStyle_menu);
        setNewListType(NUMBERED_LIST);
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
		setNewListType(NOT_A_LIST);
	} else {
		const void *junk;
		junk = m_styleVector.getNthItem(*value - 1);
		setNewListType((enum FL_ListType)((int)junk));
	}

	_gatherData();

	setbStartNewList(false);
	setbApplyToCurrent(false);
	setbResumeList(false);

	PtGetResource(m_wStartNewList, Pt_ARG_FLAGS, &value, 0);
	if ((*value) & Pt_SET) {
		setbStartNewList(true);
	}

	PtGetResource(m_wApplyCurrent, Pt_ARG_FLAGS, &value, 0);
	if ((*value) & Pt_SET) {
		setbApplyToCurrent(true);
	}

	PtGetResource(m_wStartSubList, Pt_ARG_FLAGS, &value, 0);
	if ((*value) & Pt_SET) {
		setbResumeList(true);
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
	setNewListType(getDocListType());
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
	PtWidget_t *lblStyle, *listStyle;
	PtWidget_t *lblType, *listType;
	PtWidget_t  *lblFormat;
	PtWidget_t *togCustomize, *grpCustomize;
	PtWidget_t *numListLevel, *numListAlign, *numIndentAlign, *numStart;
	PtWidget_t *radnewlist, *radexisting, *radsublist;
	PtWidget_t *butOK, *butCancel;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	ConstructWindowName();
	m_mainWindow = abiCreatePhabDialog("ap_QNXDialog_Lists",pSS,XAP_STRING_ID_DLG_Cancel);

	PtSetResource(m_mainWindow,Pt_ARG_WINDOW_TITLE,getWindowName(),NULL); 

	SetupContextHelp(m_mainWindow,this);
	PtAddHotkeyHandler(m_mainWindow,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	PtAddCallback(m_mainWindow, Pt_CB_WINDOW_CLOSING, s_deleteClicked, this);

	localizeLabel(abiPhabLocateWidget(m_mainWindow,"lblType"), pSS, AP_STRING_ID_DLG_Lists_Type);
	
	listType = abiPhabLocateWidget(m_mainWindow,"comboType");
	PtAddCallback(listType, Pt_CB_SELECTION, s_typeChanged, this);

	const char *text;
	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Type_none,s);
	text = s.utf8_str();
	PtListAddItems(listType, &text, 1, 0);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Type_bullet,s);
	text = s.utf8_str();
	PtListAddItems(listType, &text, 1, 0);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Type_numbered,s);
	text = s.utf8_str();
	PtListAddItems(listType, &text, 1, 0);

	localizeLabel(abiPhabLocateWidget(m_mainWindow,"lblStyle"), pSS, AP_STRING_ID_DLG_Lists_Style);

	listStyle = abiPhabLocateWidget(m_mainWindow,"comboStyle"); 
	PtAddCallback(listStyle, Pt_CB_SELECTION, s_styleChanged, this);

	localizeLabel(abiPhabLocateWidget(m_mainWindow,"grpCustomLists"),pSS,AP_STRING_ID_DLG_Lists_Customize );

	localizeLabel(abiPhabLocateWidget(m_mainWindow,"lblFormat"), pSS, AP_STRING_ID_DLG_Lists_Format);
	localizeLabel(abiPhabLocateWidget(m_mainWindow,"lblFont"), pSS, AP_STRING_ID_DLG_Lists_Font);
	localizeLabel(abiPhabLocateWidget(m_mainWindow,"lblLevelDelimiter"), pSS, AP_STRING_ID_DLG_Lists_DelimiterString);
	localizeLabel(abiPhabLocateWidget(m_mainWindow,"lblStartAt"), pSS, AP_STRING_ID_DLG_Lists_Start);
	localizeLabel(abiPhabLocateWidget(m_mainWindow,"lblTextAlign"), pSS, AP_STRING_ID_DLG_Lists_Align);
	localizeLabel(abiPhabLocateWidget(m_mainWindow,"lblLabelAlign"), pSS, AP_STRING_ID_DLG_Lists_Indent);

	lblFormat = abiPhabLocateWidget(m_mainWindow,"textFormat"); 
	PtAddCallback(lblFormat, Pt_CB_ACTIVATE, s_somethingChanged, this);

	PtWidget_t *font = abiPhabLocateWidget(m_mainWindow,"comboFonts"); 
	//TODO: Fill this with the current fonts
	{
		text = "Current Font";
		PtListAddItems(font, &text, 1, 0);
	}
	UT_QNXComboSetPos(font, 1);

	numListLevel = abiPhabLocateWidget(m_mainWindow,"NumericLevelDelimiter"); 
	PtAddCallback(numListLevel, Pt_CB_NUMERIC_CHANGED, s_somethingChanged, this);

	numStart = abiPhabLocateWidget(m_mainWindow,"NumericStartAt");
	PtAddCallback(numStart, Pt_CB_NUMERIC_CHANGED, s_somethingChanged, this);
	
	numListAlign = abiPhabLocateWidget(m_mainWindow,"NumericTextAlign"); 
	PtAddCallback(numListAlign, Pt_CB_NUMERIC_CHANGED, s_somethingChanged, this);
	
	numIndentAlign =abiPhabLocateWidget(m_mainWindow,"NumericLabelAlign"); 
	PtAddCallback(numIndentAlign, Pt_CB_NUMERIC_CHANGED, s_somethingChanged, this);

	 localizeLabel(abiPhabLocateWidget(m_mainWindow,"grpPreview"),pSS,AP_STRING_ID_DLG_Lists_Preview);

	m_wPreviewArea = abiPhabLocateWidget(m_mainWindow,"rawPreview"); 
	PtSetResource(m_wPreviewArea, Pt_ARG_POINTER, this, 0);
	PtSetResource(m_wPreviewArea, Pt_ARG_RAW_DRAW_F, &s_preview_exposed, 1);

	radnewlist = abiPhabLocateWidget(m_mainWindow,"toggleNew"); 
	localizeLabel(radnewlist, pSS, AP_STRING_ID_DLG_Lists_Start_New);

	radexisting = abiPhabLocateWidget(m_mainWindow,"toggleCurrent");
	localizeLabel(radexisting, pSS, AP_STRING_ID_DLG_Lists_Apply_Current);

	radsublist = abiPhabLocateWidget(m_mainWindow,"togglePrevious"); 
	localizeLabel(radsublist, pSS, AP_STRING_ID_DLG_Lists_Start_Sub);

	butOK = abiPhabLocateWidget(m_mainWindow,"btnApply"); 
	PtSetResource(butOK, Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_Apply), 0);
	PtAddCallback(butOK, Pt_CB_ACTIVATE, s_applyClicked, this);


	butCancel = abiPhabLocateWidget(m_mainWindow,"btnClose");
	PtSetResource(butCancel, Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_Close), 0);
	PtAddCallback(butCancel, Pt_CB_ACTIVATE, s_closeClicked, this);

	/** Done **/
	m_wDelimEntry = lblFormat;
//	m_wDecimalEntry = lblFormat2;

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
	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Type_none,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)NOT_A_LIST);
}

void AP_QNXDialog_Lists::_fillNumberedStyleMenu( PtWidget_t *listmenu)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	const char *text;

	PtListDeleteAllItems(listmenu);
	m_styleVector.clear();
	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Numbered_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)NUMBERED_LIST);
	
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Lower_Case_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)LOWERCASE_LIST);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Upper_Case_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)UPPERCASE_LIST);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Lower_Roman_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)LOWERROMAN_LIST);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Upper_Roman_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)UPPERROMAN_LIST);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Arabic_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)ARABICNUMBERED_LIST);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Hebrew_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)HEBREW_LIST);
}

void AP_QNXDialog_Lists::_fillBulletedStyleMenu( PtWidget_t *listmenu)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	const char *text;
	UT_UTF8String s;
	
	PtListDeleteAllItems(listmenu);
	m_styleVector.clear();

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Bullet_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)BULLETED_LIST);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Dashed_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)DASHED_LIST);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Square_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)SQUARE_LIST);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Triangle_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)TRIANGLE_LIST);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Diamond_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)DIAMOND_LIST);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Star_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)STAR_LIST);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Implies_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)IMPLIES_LIST);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Tick_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)TICK_LIST);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Box_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)BOX_LIST);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Hand_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)HAND_LIST);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Heart_List,s);
	text = s.utf8_str();
	PtListAddItems(listmenu, &text, 1, 0);
	m_styleVector.addItem((void *)HEART_LIST);
}

void AP_QNXDialog_Lists::_setRadioButtonLabels(void)
{
	//	char *tmp;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UTF8String s;
	PopulateDialogData();
	// Button 0 is Start New List, button 2 is resume list
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Start_New,s);
	PtSetResource(m_wStartNewList, Pt_ARG_TEXT_STRING, s.utf8_str(), 0);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Resume,s);
	PtSetResource(m_wStartSubList, Pt_ARG_TEXT_STRING, s.utf8_str(), 0);
}

//
// This function reads the various memeber variables and loads them into
// into the dialog variables.
//
void AP_QNXDialog_Lists::_loadXPDataIntoLocal(void)
{
	unsigned int i;
	double d;

	m_bDontUpdate = true;

	UT_DEBUGMSG(("_loadXP newListType = %d \n", getNewListType()));

	//PtSetResource(m_wLevelSpin, Pt_ARG_NUMERIC_VALUE, m_iLevel, 0);

	d = getfAlign();
	PtSetResource(m_wAlignListSpin, Pt_ARG_NUMERIC_VALUE, &d, 0);

	d = getfAlign() + getfIndent();
	if(d < 0.0) {
		setfIndent(-getfAlign());
		d = 0.0;
		PtSetResource(m_wIndentAlignSpin, Pt_ARG_NUMERIC_VALUE, &d, 0);
	} else {
		PtSetResource(m_wIndentAlignSpin, Pt_ARG_NUMERIC_VALUE, &d, 0);
	}


	PtSetResource(m_wStartSpin, Pt_ARG_NUMERIC_VALUE, getiStartValue(), 0);

#if 0
	PtSetResource(m_wDecimalEntry, Pt_ARG_TEXT_STRING, getDecimal(), 0);
#endif
	PtSetResource(m_wDelimEntry, Pt_ARG_TEXT_STRING, getDelim(), 0);

	//
	// Now set the list type and style
	FL_ListType save = getNewListType();
	if(save == NOT_A_LIST) {
		styleChanged(0);
		setNewListType(save);
		PtSetResource(m_wListType_menu, Pt_ARG_CBOX_SEL_ITEM, 1, 0);
	} else if(save >= BULLETED_LIST) {
		styleChanged(1);
		setNewListType(save);
		PtSetResource(m_wListType_menu, Pt_ARG_CBOX_SEL_ITEM, 2, 0);
	} else {
		styleChanged(2);
		setNewListType(save);
		PtSetResource(m_wListType_menu, Pt_ARG_CBOX_SEL_ITEM, 3, 0);
	}

	/* Determine which of the styles should be set */
	for (i=0; i<m_styleVector.getItemCount(); i++) {
		const void *junk;
		junk = m_styleVector.getNthItem(i);
		if ((enum FL_ListType)((int)junk) == getNewListType()) {
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
	setiLevel(1);

	PtGetResource(m_wAlignListSpin, Pt_ARG_NUMERIC_VALUE, &ddata, 0);
	setfAlign(*ddata);

	PtGetResource(m_wIndentAlignSpin, Pt_ARG_NUMERIC_VALUE, &ddata, 0);
	indent = *ddata;

	setfIndent(indent - getfAlign());

	if( (getfIndent() + getfAlign()) < 0.0) {
		setfIndent(- getfAlign());
		indent = 0.0;
		PtSetResource(m_wIndentAlignSpin, Pt_ARG_NUMERIC_VALUE, &indent, sizeof(indent));
	}

	copyCharToFont( "NULL");


	PtGetResource(m_wStartSpin, Pt_ARG_NUMERIC_VALUE, &idata, 0);
	setiStartValue(*idata);

	sdata = NULL;
	PtGetResource(m_wDelimEntry, Pt_ARG_TEXT_STRING, &sdata, 0);
	//strncpy((gchar *)m_pszDelim, (const gchar *) sdata, 80);
	copyCharToDelim((const char *) sdata);

#if 0
	sdata = NULL;
	PtGetResource(m_wDecimalEntry, Pt_ARG_TEXT_STRING, &sdata, 0);
	strncpy( (gchar *) m_pszDecimal, (const gchar *) ".", 80);
#else
	copyCharToDecimal( (const char *) ".");
#endif
}


