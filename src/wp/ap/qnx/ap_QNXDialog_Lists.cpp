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
}

AP_QNXDialog_Lists::~AP_QNXDialog_Lists(void)
{
}

/**********************************************************************/

static int s_customChanged(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	dlg->customChanged ();
	return Pt_CONTINUE;
}

static int s_typeChangedNone(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	dlg->typeChanged(0);
	return Pt_CONTINUE;
}

static int s_typeChangedBullet(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	dlg->typeChanged(1);
	return Pt_CONTINUE;
}

static int s_typeChangedNumbered (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	dlg->typeChanged(2);
	return Pt_CONTINUE;
}

static int s_styleChanged (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	dlg->setMemberVariables();
	dlg->previewExposed();
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
	UT_Rect rClip;

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
	printf("Calling expose area ... \n");
	pQNXDlg->previewExposed();

    PtClipRemove();
	return Pt_CONTINUE;
}

static int s_update (void)
{
/*
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	Current_Dialog->updateDialog();
*/
	return UT_TRUE;
}


/**********************************************************************/

void AP_QNXDialog_Lists::activate()
{
	ConstructWindowName();
	PtSetResource(m_mainWindow, Pt_ARG_WINDOW_TITLE, m_WindowName, NULL);
	updateDialog();
	//Raise the window ...
}

void AP_QNXDialog_Lists::destroy()
{
	if (!m_mainWindow) {
		return;
	}
	m_bDestroy_says_stopupdating = UT_TRUE;
	while (m_bAutoUpdate_happening_now == UT_TRUE) ;
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
		XAP_QNXApp * app = static_cast<XAP_QNXApp *> (m_pApp);
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

	// Now Display the dialog
	PtRealizeWidget(m_mainWindow);

	// Next construct a timer for auto-updating the dialog
	m_pAutoUpdateLists = UT_Timer::static_constructor(autoupdateLists,this);
	m_bDestroy_says_stopupdating = UT_FALSE;

	// OK fire up the auto-updater for 0.5 secs
	//m_pAutoUpdateLists->set(500);
}

void    AP_QNXDialog_Lists::autoupdateLists(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);
	// this is a static callback method and does not have a 'this' pointer.
	AP_QNXDialog_Lists * pDialog =  (AP_QNXDialog_Lists *) pTimer->getInstanceData();
	// Handshaking code

#if 0
	if( pDialog->m_bDestroy_says_stopupdating != UT_TRUE)
	{
		pDialog->m_bAutoUpdate_happening_now = UT_TRUE;
		pDialog->updateDialog();
		pDialog->m_bAutoUpdate_happening_now = UT_FALSE;
	}
#endif
}

void AP_QNXDialog_Lists::previewExposed(void)
{
	if(m_pPreviewWidget) {
		//setMemberVariables();
		event_PreviewAreaExposed();
	}
} 


void  AP_QNXDialog_Lists::typeChanged(int style)
{
  // 
  // code to change list list
  //
	//gtk_option_menu_remove_menu(GTK_OPTION_MENU (m_wListStyleBox));
	if(style == 0)
	{
	  //     gtk_widget_destroy(GTK_WIDGET(m_wListStyleBulleted_menu));
/*
	  	m_wListStyleNone_menu = gtk_menu_new();
		m_wListStyle_menu = m_wListStyleNone_menu;
		_fillNoneStyleMenu(m_wListStyleNone_menu);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (m_wListStyleBox), m_wListStyleNone_menu);
*/
	}
	else if(style == 1)
	{
	  //    gtk_widget_destroy(GTK_WIDGET(m_wListStyleBulleted_menu));
/*
       		m_wListStyleBulleted_menu = gtk_menu_new();
		m_wListStyle_menu = m_wListStyleBulleted_menu;
        _fillBulletedStyleMenu(m_wListStyleBulleted_menu);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (m_wListStyleBox), m_wListStyleBulleted_menu);
*/
	}
	else if(style == 2)
	{
	  //  gtk_widget_destroy(GTK_WIDGET(m_wListStyleNumbered_menu));
/*
	  	m_wListStyleNumbered_menu = gtk_menu_new();
		m_wListStyle_menu = m_wListStyleNumbered_menu;
        _fillNumberedStyleMenu(m_wListStyleNumbered_menu);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (m_wListStyleBox), m_wListStyleNumbered_menu);
*/
	}

/*
	PtWidget_t * wlisttype=gtk_menu_get_active(GTK_MENU(m_wListStyle_menu));
	m_newListType =  (List_Type) GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(wlisttype)));
*/
	previewExposed();
}

void  AP_QNXDialog_Lists::setMemberVariables(void)
{
	//
	// Failsafe code to make sure the start, stop and change flags are set
        // as shown on the GUI.
	//
#if 0	
	PtWidget_t * wlisttype=gtk_menu_get_active(GTK_MENU(m_wListStyle_menu));
	m_newListType =  (List_Type) GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(wlisttype)));
	if(m_bisCustomized == UT_TRUE)
	{
	        _gatherData();
	}
	if (GTK_TOGGLE_BUTTON (m_wStartNewList)->active)
	{
	        m_bStartNewList = UT_TRUE;
		m_bApplyToCurrent = UT_FALSE;
                m_bStartSubList = UT_FALSE;
	}
	else if (GTK_TOGGLE_BUTTON (m_wApplyCurrent)->active)
	{
	        m_bStartNewList = UT_FALSE;
		m_bApplyToCurrent = UT_TRUE;
                m_bStartSubList = UT_FALSE;
	}
	else if (GTK_TOGGLE_BUTTON (m_wStartSubList)->active)
	{
	        m_bStartNewList = UT_FALSE;
		m_bApplyToCurrent = UT_FALSE;
                m_bStartSubList = UT_TRUE;
	}
#endif
}


void  AP_QNXDialog_Lists::applyClicked(void)
{
	setMemberVariables();
	previewExposed();
	Apply();
}

void  AP_QNXDialog_Lists::customChanged(void)
{
	if(m_bisCustomFrameHidden == UT_TRUE)
	{
		fillWidgetFromDialog();
/*
		gtk_widget_show(m_wCustomFrame);
		gtk_arrow_set(GTK_ARROW(m_wCustomArrow),GTK_ARROW_DOWN,GTK_SHADOW_OUT);
*/
		m_bisCustomFrameHidden = UT_FALSE;
		m_bisCustomized = UT_TRUE;
		setMemberVariables();
		previewExposed();
	}
	else
	{
/*
		gtk_widget_hide(m_wCustomFrame);
		gtk_arrow_set(GTK_ARROW(m_wCustomArrow),GTK_ARROW_RIGHT,GTK_SHADOW_OUT);
*/
		m_bisCustomFrameHidden = UT_TRUE;
		m_bisCustomized = UT_FALSE;
		_setData();
	}
}


void AP_QNXDialog_Lists::fillWidgetFromDialog(void)
{
 	PopulateDialogData();
	_setData();
}

void AP_QNXDialog_Lists::updateDialog(void)
{
	List_Type oldlist = m_iListType;
	if(m_bisCustomized == UT_FALSE)
		_populateWindowData();
	if((oldlist != m_iListType) && (m_bisCustomized == UT_FALSE))
		m_newListType = m_iListType;
	if(m_bisCustomized == UT_FALSE)
		_setData();
}

void AP_QNXDialog_Lists::setAllSensitivity(void)
{ 
	PopulateDialogData();
	if(m_isListAtPoint == UT_TRUE) 
	{
	}
}


PtWidget_t * AP_QNXDialog_Lists::_constructWindow (void)
{
	PtWidget_t *vgroup;
	PtWidget_t *lblStyle, *listStyle;
	PtWidget_t *lblType, *listType;
	PtWidget_t *togCustomize, *grpCustomize;
	PtWidget_t *lblFormat;
	PtWidget_t *numListLevel, *numListAlign, *numIndentAlign, *numStart;
	PtWidget_t *radnewlist, *radexisting, *radsublist, *radresumelist;
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

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
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
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Style:", 0);
	lblStyle = PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 1.5*ABI_DEFAULT_BUTTON_WIDTH, 0);
	listStyle = PtCreateWidget(PtComboBox, group, n, args);	

	n = 0;
	group = PtCreateWidget(PtGroup, ctlgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Type:", 0);
	lblType = PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 1.5*ABI_DEFAULT_BUTTON_WIDTH, 0);
	listType = PtCreateWidget(PtComboBox, group, n, args);	

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Customize ...", 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_OUTLINE, 0);
	togCustomize = PtCreateWidget(PtToggleButton, ctlgroup, n, args);	

	n = 0;
#define OUTLINE_GROUP (Pt_TOP_OUTLINE | Pt_TOP_BEVEL | \
                                           Pt_BOTTOM_OUTLINE | Pt_BOTTOM_BEVEL | \
                                           Pt_LEFT_OUTLINE | Pt_LEFT_BEVEL | \
                                           Pt_RIGHT_OUTLINE | Pt_RIGHT_BEVEL)
	PtSetArg(&args[n++], Pt_ARG_BASIC_FLAGS, OUTLINE_GROUP, OUTLINE_GROUP);
	PtSetArg(&args[n++], Pt_ARG_BEVEL_WIDTH, 1, 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_HIGHLIGHTED, Pt_HIGHLIGHTED);
	grpCustomize = PtCreateWidget(PtGroup, ctlgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING, 5, 0);
	group = PtCreateWidget(PtGroup, grpCustomize, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Label:", 0);
	PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "", 0);
	PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Level:", 0);
	PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Start At:", 0);
	PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "List Align:", 0);
	PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Indent Align:", 0);
	PtCreateWidget(PtLabel, group, n, args);	

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	group = PtCreateWidget(PtGroup, grpCustomize, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 1.5*ABI_DEFAULT_BUTTON_WIDTH, 0);
	lblFormat = PtCreateWidget(PtText, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Font ...", 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtCreateWidget(PtButton, group, n, args);	
	n = 0;
	numListLevel = PtCreateWidget(PtNumericInteger, group, n, args);	
	n = 0;
	numStart = PtCreateWidget(PtNumericInteger, group, n, args);	
	n = 0;
	numListAlign = PtCreateWidget(PtNumericInteger, group, n, args);	
	n = 0;
	numIndentAlign = PtCreateWidget(PtNumericInteger, group, n, args);	

	/*** Create the preview in the next dialog ***/
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 200, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 300, 0);
	PtSetArg(&args[n++], Pt_ARG_FILL_COLOR, Pg_WHITE, 0);
	PtCreateWidget(PtLabel, hgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  200, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT,  300, 0);
	m_wPreviewGroup = PtCreateWidget(PtGroup, hgroup, n, args);

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
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Start new list", 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_SET, Pt_SET);
	radnewlist = PtCreateWidget(PtToggleButton, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Apply to current list", 0);
	radexisting = PtCreateWidget(PtToggleButton, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Start sub list", 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	radsublist = PtCreateWidget(PtToggleButton, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Resume current list", 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	radresumelist = PtCreateWidget(PtToggleButton, group, n, args);	
		
	/*** Then we have the final cancellation buttons ***/
	n = 0;
	group = PtCreateWidget(PtGroup, vgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 200, 0);
	PtCreateWidget(PtLabel, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "OK", 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	butOK = PtCreateWidget(PtButton, group, n, args);	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Cancel", 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	butCancel = PtCreateWidget(PtButton, group, n, args);	

	/** Done **/
	m_wCustomFrame = grpCustomize;
	m_wCustomLabel = togCustomize;

	m_wListStyleBox = lblFormat;
	m_wLevelSpin = numListLevel;
	m_wStartSpin = numStart;
	m_wAlignListSpin = numListAlign;
	m_wIndentAlignSpin = numIndentAlign;

	m_wStartNewList = radnewlist;
	m_wApplyCurrent = radexisting;
	m_wStartSubList = radsublist;
	m_wResumeList = radresumelist;

	m_wApply = butOK;
	m_wClose = butCancel;

	return m_mainWindow;
}

void AP_QNXDialog_Lists::_fillNoneStyleMenu( PtWidget_t *listmenu)
{
#if 0
        PtWidget_t *glade_menuitem;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Type_none));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) NOT_A_LIST ));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);
#endif
}

void AP_QNXDialog_Lists::_fillNumberedStyleMenu( PtWidget_t *listmenu)
{
#if 0
        PtWidget_t *glade_menuitem;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Numbered_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) NUMBERED_LIST ));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


        glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Case_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) LOWERCASE_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Case_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) UPPERCASE_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Roman_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) LOWERROMAN_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Roman_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) UPPERROMAN_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);
#endif
}


void AP_QNXDialog_Lists::_fillBulletedStyleMenu( PtWidget_t *listmenu)
{
#if 0
        PtWidget_t *glade_menuitem;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Bullet_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) BULLETED_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Dashed_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) DASHED_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Square_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) SQUARE_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Triangle_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) TRIANGLE_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Diamond_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) DIAMOND_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Star_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) STAR_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Implies_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) IMPLIES_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Tick_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) TICK_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Box_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) BOX_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);



	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Hand_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) HAND_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Heart_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) HEART_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

#endif
}

void AP_QNXDialog_Lists::_populateWindowData (void) 
{
#if 0
  //	char *tmp;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
        PopulateDialogData();
	if(m_isListAtPoint == UT_TRUE)
	{
	  // Button 0 is stop list, button 2 is startsub list
	       gtk_label_set_text( GTK_LABEL(m_wStartNew_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Stop_Current_List));
	       gtk_label_set_text( GTK_LABEL(m_wStartSub_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Start_Sub));

	}
	else
	{
	  // Button 0 is Start New List, button 2 is resume list
	       gtk_label_set_text( GTK_LABEL(m_wStartNew_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Start_New));
	       gtk_label_set_text( GTK_LABEL(m_wStartSub_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Resume));
	}
#endif
}

#if 0
void AP_QNXDialog_Lists::_connectSignals(void)
{
	gtk_signal_connect_after(GTK_OBJECT(m_wMainWindow),
							 "destroy",
							 NULL,
							 NULL);
	//
        // Don't use connect_after in modeless dialog
	gtk_signal_connect(GTK_OBJECT(m_wMainWindow),
						     "delete_event",
						     GTK_SIGNAL_FUNC(s_deleteClicked), (gpointer) this);

	gtk_signal_connect (GTK_OBJECT (m_wApply), "clicked",
						GTK_SIGNAL_FUNC (s_applyClicked), this);
	gtk_signal_connect (GTK_OBJECT (m_wClose), "clicked",
						GTK_SIGNAL_FUNC (s_closeClicked), this);
	gtk_signal_connect (GTK_OBJECT (m_wCustomLabel), "clicked",
						GTK_SIGNAL_FUNC (s_customChanged), this);
	gtk_signal_connect (GTK_OBJECT (m_wMenu_None), "activate",
					GTK_SIGNAL_FUNC (s_typeChangedNone), this);
	gtk_signal_connect (GTK_OBJECT (m_wMenu_Bull), "activate",
					GTK_SIGNAL_FUNC (s_typeChangedBullet), this);
	gtk_signal_connect (GTK_OBJECT (m_wMenu_Num), "activate",
					GTK_SIGNAL_FUNC (s_typeChangedNumbered), this);
	gtk_signal_connect (GTK_OBJECT (m_oStartSpin_adj), "value_changed",
				      GTK_SIGNAL_FUNC (s_styleChanged), this);
	gtk_signal_connect (GTK_OBJECT (m_oLevelSpin_adj), "value_changed",
				      GTK_SIGNAL_FUNC (s_styleChanged), this);
	gtk_signal_connect (GTK_OBJECT (m_oAlignList_adj), "value_changed",
				      GTK_SIGNAL_FUNC (s_styleChanged), this);
	gtk_signal_connect (GTK_OBJECT (m_oIndentAlign_adj), "value_changed",
				      GTK_SIGNAL_FUNC (s_styleChanged), this);
	gtk_signal_connect (GTK_OBJECT (GTK_ENTRY(m_wDelimEntry)), "changed",
				      GTK_SIGNAL_FUNC (s_styleChanged), this);


	// the expose event of the preview
	             gtk_signal_connect(GTK_OBJECT(m_wPreviewArea),
					   "expose_event",
					   GTK_SIGNAL_FUNC(s_preview_exposed),
					   (gpointer) this);

	
		     gtk_signal_connect_after(GTK_OBJECT(m_wMainWindow),
		     					 "expose_event",
		     				 GTK_SIGNAL_FUNC(s_window_exposed),
		    					 (gpointer) this);

}
#endif

void AP_QNXDialog_Lists::_setData(void)
{
#if 0
  //
  // This function reads the various elements in customize box and loads
  // the member variables with them
  //
        gint i;
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_wLevelSpin), (float) m_iLevel);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_wAlignListSpin),m_fAlign);
	float indent = m_fAlign + m_fIndent;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON( m_wIndentAlignSpin),indent);
	if( (m_fIndent + m_fAlign) < 0.0)
	{
	         m_fIndent = - m_fAlign;
                 gtk_spin_button_set_value(GTK_SPIN_BUTTON( m_wIndentAlignSpin), 0.0);

	}
	//
	// Code to work out which is active Font
	//
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
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_wStartSpin),(float) m_iStartValue);
	// first, we stop the entry from sending the changed signal to our handler
	gtk_signal_handler_block_by_data(  GTK_OBJECT(m_wDelimEntry), (gpointer) this );

	gtk_entry_set_text( GTK_ENTRY(m_wDelimEntry), (const gchar *) m_pszDelim);
	// turn signals back on
	gtk_signal_handler_unblock_by_data(  GTK_OBJECT(m_wDelimEntry), (gpointer) this );
#endif
}


void AP_QNXDialog_Lists::_gatherData(void)
{
#if 0
  //
  // This function reads the various elements in customize box and loads
  // the member variables with them
  //
        m_iLevel =  gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_wLevelSpin));
	m_fAlign = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(m_wAlignListSpin));
	float indent = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON( m_wIndentAlignSpin));
	m_fIndent = indent - m_fAlign;
	if( (m_fIndent + m_fAlign) < 0.0)
	{
	         m_fIndent = - m_fAlign;
                 gtk_spin_button_set_value(GTK_SPIN_BUTTON( m_wIndentAlignSpin), 0.0);

	}
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
	UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
	m_iStartValue =  gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_wStartSpin));
	gchar * pszDel = gtk_entry_get_text( GTK_ENTRY(m_wDelimEntry));
        UT_XML_strncpy((XML_Char *)m_pszDelim, 80, (const XML_Char *) pszDel);
#endif
}


