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

static int s_startChanged (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	dlg->startChanged ();
	return Pt_CONTINUE;
}

static int s_stopChanged (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	dlg->stopChanged ();
	return Pt_CONTINUE;
}

static int s_startvChanged (PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Lists * dlg = (AP_QNXDialog_Lists *)data;
	dlg->startvChanged ();
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

	if( pDialog->m_bDestroy_says_stopupdating != UT_TRUE)
	{
		pDialog->m_bAutoUpdate_happening_now = UT_TRUE;
		pDialog->updateDialog();
		pDialog->m_bAutoUpdate_happening_now = UT_FALSE;
	}
}


void  AP_QNXDialog_Lists::applyClicked(void)
{
#if 0
    char * szStartValue;
	PtWidget_t * wlisttype;

	//
	// Failsafe code to make sure the start, stop and change flags are set
    // as shown on the GUI.
	//

       if (GTK_TOGGLE_BUTTON (m_wCheckstartlist)->active)
       {
	       wlisttype=gtk_menu_get_active(GTK_MENU(m_wOption_types_menu));
	       m_iListType = GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(wlisttype)));
	       szStartValue =gtk_entry_get_text( GTK_ENTRY (m_wNew_startingvaluev) );
	       m_bStartList = UT_TRUE;
	       if(m_iListType == 0)
	       {
		      m_newStartValue = atoi(szStartValue);
		      strcpy((gchar *) m_newListType, "%*%d.");
	       }
	       else if (m_iListType == 1)
	       {
		      m_newStartValue = atoi(szStartValue);
		      strcpy((gchar *) m_newListType,"%*%a.");
	       }
	       else if (m_iListType == 2)
	       {
		      m_newStartValue = atoi(szStartValue);
		      strcpy((gchar *) m_newListType,"%*%A.");
	       }
	       else if (m_iListType == 3)
	       {
		 //	      gchar c = *szStartValue;
		      m_newStartValue = 1;
		      strcpy((gchar *) m_newListType, "%b");
	       }
       }
       else
	       m_bStartList = UT_FALSE;

       if (GTK_TOGGLE_BUTTON (m_wCheckstoplist)->active)
	       m_bStopList = UT_TRUE;
       else
	       m_bStopList = UT_FALSE;

       if (GTK_TOGGLE_BUTTON (m_wCur_changestart_button)->active)
       {
	       m_bChangeStartValue = UT_TRUE;
	       wlisttype=gtk_menu_get_active(GTK_MENU(m_wCur_Option_types_menu));
	       m_iListType = GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(wlisttype)));
	       szStartValue =gtk_entry_get_text( GTK_ENTRY (m_wCur_startingvaluev) );
	       m_bStartList = UT_TRUE;
	       if(m_iListType == 0)
	       {
		      m_curStartValue = atoi(szStartValue);
		      strcpy((gchar *) m_newListType, "%*%d.");
	       }
	       else if (m_iListType == 1)
	       {
		      m_curStartValue = atoi(szStartValue);
		      strcpy((gchar *) m_newListType,"%*%a.");
	       }
	       else if (m_iListType == 2)
	       {
		      m_curStartValue = atoi(szStartValue);
		      strcpy((gchar *) m_newListType,"%*%A.");
	       }
	       else if (m_iListType == 3)
	       {
		 //	      gchar c = *szStartValue;
		      m_curStartValue = 1;
		      strcpy((gchar *) m_newListType, "%b");
	       }
       }
       else
	       m_bChangeStartValue = UT_FALSE;

	Apply();

	// Make all checked buttons inactive 
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckstoplist),FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCur_changestart_button),FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckstartlist),FALSE);
	setAllSensitivity();
#endif
}

void  AP_QNXDialog_Lists::startChanged(void)
{
#if 0
	if (GTK_TOGGLE_BUTTON (m_wCheckstartlist)->active)
	{
	       m_bStartList = UT_TRUE;
	       m_bStopList = UT_FALSE;
	       m_bChangeStartValue = UT_FALSE;
           gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCur_changestart_button),FALSE);
           gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckresumelist),FALSE);
           gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckstoplist),FALSE);
    }
    else
    {
		m_bStartList = UT_FALSE;
	}
	setAllSensitivity();
#endif
}


void  AP_QNXDialog_Lists::stopChanged(void)
{
#if 0
	if (GTK_TOGGLE_BUTTON (m_wCheckstoplist)->active)
	{
		m_bStopList = UT_TRUE;
		m_bChangeStartValue = UT_FALSE;
		m_bStartList = UT_FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCur_changestart_button),FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckresumelist),FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckstartlist),FALSE);
	}
	else
	{
		m_bStopList = UT_FALSE;
	}
	setAllSensitivity();
#endif
}


void  AP_QNXDialog_Lists::startvChanged(void)
{
#if 0
	if (GTK_TOGGLE_BUTTON (m_wCur_changestart_button)->active)
	{
		m_bChangeStartValue = UT_TRUE;
		m_bStartList = UT_FALSE;
		m_bStopList = UT_FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckresumelist),FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckstoplist),FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckstartlist),FALSE);
	}
	else
	{
		m_bChangeStartValue = UT_FALSE;
	}
	setAllSensitivity();
#endif
}


void AP_QNXDialog_Lists::updateDialog(void)
{
	_populateWindowData();
	setAllSensitivity();
}

PtWidget_t * AP_QNXDialog_Lists::_constructWindow (void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	PtArg_t args[10];
	int n;

	n = 0;
	ConstructWindowName();
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, m_WindowName, 0);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	m_mainWindow = PtCreateWidget(PtWindow, NULL, n, args);


	/* Everything goes into a vertical group */
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 5, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
			Pt_GROUP_EQUAL_SIZE_HORIZONTAL, 
			Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	PtWidget_t *vgroup = PtCreateWidget(PtGroup, m_mainWindow, n, args);

	/* Create the labels across the top */
	n = 0;
	PtWidget_t *hlabelgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Current_List_Type), 0);
	m_wCur_listtype = PtCreateWidget(PtLabel, hlabelgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "xxxxxxxx", 0);
	m_wCur_listtypev = PtCreateWidget(PtLabel, hlabelgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(AP_STRING_ID_DLG_Lists_Current_List_Label), 0);
	m_wCur_listlabel = PtCreateWidget(PtLabel, hlabelgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "xxxxxxxx", 0);
	m_wCur_listlabelv = PtCreateWidget(PtLabel, hlabelgroup, n, args);

	/* Create the radio groupings for the actions  ... */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EXCLUSIVE, Pt_GROUP_EXCLUSIVE);
	PtWidget_t *hradiogroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, 
					"Start New List",
					/* pSS->getValue(AP_STRING_ID_DLG_Lists_Cur_Change_Start), */ 
					0);
	m_wCheckstartlist = PtCreateWidget(PtToggleButton, hradiogroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING,
					"Change Current List",
					/* pSS->getValue(AP_STRING_ID_DLG_Lists_Start_New_List), */
					 0);
	m_wCheckcurlist = PtCreateWidget(PtToggleButton, hradiogroup, n, args);

	/* Create the actual button actions here */
	n = 0;
	PtWidget_t *hactiongroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "List Type:", 0);
	m_wMenuListTypeLabel = PtCreateWidget(PtLabel, hactiongroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	m_wMenuListType = PtCreateWidget(PtComboBox, hactiongroup, n, args);
	const char *item;
	item = (const char *)pSS->getValue(AP_STRING_ID_DLG_Lists_Numbered_List);
	PtListAddItems(m_wMenuListType, &item, 1, 0);
	item = (const char *)pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Case_List);
	PtListAddItems(m_wMenuListType, &item, 1, 0);
	item = (const char *)pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Case_List);
	PtListAddItems(m_wMenuListType, &item, 1, 0);
	item = (const char *)pSS->getValue(AP_STRING_ID_DLG_Lists_Bullet_List);
	PtListAddItems(m_wMenuListType, &item, 1, 0);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Start Value:", 0);
	m_wStartValueLabel = PtCreateWidget(PtLabel, hactiongroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	m_wStartValue = PtCreateWidget(PtText, hactiongroup, n, args);


	/* Buttons for the bottom */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_X, 5, 0);
	PtWidget_t *hbutgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Stop", 0);
	m_wStop = PtCreateWidget(PtButton, hbutgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Resume", 0);
	m_wResume = PtCreateWidget(PtButton, hbutgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_Apply), 0);
	m_wApply = PtCreateWidget(PtButton, hbutgroup, n, args);
	PtAddCallback(m_wApply, Pt_CB_ACTIVATE, s_applyClicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue (XAP_STRING_ID_DLG_Close), 0);
	m_wClose = PtCreateWidget(PtButton, hbutgroup, n, args);
	PtAddCallback(m_wClose, Pt_CB_ACTIVATE, s_closeClicked, this);

	return m_mainWindow;
}


void AP_QNXDialog_Lists::_populateWindowData (void) 
{
	PopulateDialogData();

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	/* We are inside a current list, new & change are enabled */
	if(m_isListAtPoint == UT_TRUE) {
		int *flags = NULL;

		PtSetResource(m_wCur_listlabelv, Pt_ARG_TEXT_STRING, m_curListLabel, 0);
		PtSetResource(m_wCur_listtypev, Pt_ARG_TEXT_STRING, m_curListType, 0);

		PtGetResource(m_wCheckcurlist, Pt_ARG_FLAGS, &flags, 0);
		if (!flags && *flags & Pt_SET) { 	/* Change current selected */
			char tmp[20];
			int  pos;
		
			sprintf(tmp, "%d", m_curStartValue);
			PtSetResource(m_wStartValue, Pt_ARG_TEXT_STRING, tmp, 0);
		
			if(!strcmp(m_curListType, pSS->getValue(AP_STRING_ID_DLG_Lists_Numbered_List)))
					pos = 1;
			else if(!strcmp(m_curListType, pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Case_List)))
					pos = 2;
			else if(!strcmp(m_curListType, pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Case_List)))
					pos = 3;
			else if(!strcmp(m_curListType, pSS->getValue(AP_STRING_ID_DLG_Lists_Bullet_List)))
					pos = 4;

			UT_QNXComboSetPos(m_wMenuListType, pos);
		}
	}
	/* We are not inside a current list, change disabled, new enabled */
	else {
		PtSetResource(m_wCur_listlabelv, Pt_ARG_TEXT_STRING, "NONE", 0);
		PtSetResource(m_wCur_listtypev, Pt_ARG_TEXT_STRING, "NONE", 0);
	}

}

void AP_QNXDialog_Lists::setAllSensitivity(void)
{ 
	PopulateDialogData();

	/* If we are at a point, then all things are valid */
	if(m_isListAtPoint == UT_TRUE) {
		PtSetResource(m_wMenuListType, Pt_ARG_FLAGS, 0, Pt_GHOST | Pt_BLOCKED);
		PtSetResource(m_wMenuListTypeLabel, Pt_ARG_FLAGS, 0, Pt_GHOST | Pt_BLOCKED);
		PtSetResource(m_wStartValue, Pt_ARG_FLAGS, 0, Pt_GHOST | Pt_BLOCKED);
		PtSetResource(m_wStartValueLabel, Pt_ARG_FLAGS, 0, Pt_GHOST | Pt_BLOCKED);
		PtSetResource(m_wStop, Pt_ARG_FLAGS, 0, Pt_GHOST | Pt_BLOCKED);
	}
	else {
		PtSetResource(m_wMenuListType, Pt_ARG_FLAGS, 
				Pt_GHOST | Pt_BLOCKED, Pt_GHOST | Pt_BLOCKED);
		PtSetResource(m_wMenuListTypeLabel, Pt_ARG_FLAGS, 
				Pt_GHOST | Pt_BLOCKED, Pt_GHOST | Pt_BLOCKED);
		PtSetResource(m_wStartValue, Pt_ARG_FLAGS, 
				Pt_GHOST | Pt_BLOCKED, Pt_GHOST | Pt_BLOCKED);
		PtSetResource(m_wStartValueLabel, Pt_ARG_FLAGS, 
				Pt_GHOST | Pt_BLOCKED, Pt_GHOST | Pt_BLOCKED);
		PtSetResource(m_wStop, Pt_ARG_FLAGS, 
				Pt_GHOST | Pt_BLOCKED, Pt_GHOST | Pt_BLOCKED);
		PtSetResource(m_wResume, Pt_ARG_FLAGS, 
				Pt_GHOST | Pt_BLOCKED, Pt_GHOST | Pt_BLOCKED);

	}

	if(m_previousListExistsAtPoint == UT_TRUE) {
		PtSetResource(m_wResume, Pt_ARG_FLAGS, 0, Pt_GHOST | Pt_BLOCKED);
	}
	else {
		PtSetResource(m_wResume, Pt_ARG_FLAGS, 
				Pt_GHOST | Pt_BLOCKED, Pt_GHOST | Pt_BLOCKED);
	}
}



