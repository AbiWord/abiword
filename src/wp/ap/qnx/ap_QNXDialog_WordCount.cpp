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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_WordCount.h"
#include "ap_QNXDialog_WordCount.h"

#include "ut_qnxHelper.h"
#include <stdio.h>

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_WordCount::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	AP_QNXDialog_WordCount * p = new AP_QNXDialog_WordCount(pFactory,id);
	return p;
}

AP_QNXDialog_WordCount::AP_QNXDialog_WordCount(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_WordCount(pDlgFactory,id)
{
	m_windowMain = NULL;
	//I should initialize everything else ... 
}

AP_QNXDialog_WordCount::~AP_QNXDialog_WordCount(void)
{
}

/*****************************************************************/

static int s_ok_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_WordCount * dlg = (AP_QNXDialog_WordCount *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
	return Pt_CONTINUE;
}

static int s_update_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_WordCount * dlg = (AP_QNXDialog_WordCount *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Update();
	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_WordCount * dlg = (AP_QNXDialog_WordCount *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}

static int s_autoupdate_clicked(PtWidget_t *widget,AP_QNXDialog_WordCount *dlg,PtCallbackInfo_t* info)
{
dlg->event_Checkbox();
return Pt_CONTINUE;
}

static int s_autospinner_changed(PtWidget_t *widget,AP_QNXDialog_WordCount *dlg,PtCallbackInfo_t *info)
{
dlg->event_Spin();
return Pt_CONTINUE;
}

/*****************************************************************/

void  AP_QNXDialog_WordCount::activate(void)
{
	UT_ASSERT (m_windowMain);

	ConstructWindowName();
	PtSetResource(m_windowMain, Pt_ARG_WINDOW_TITLE, m_WindowName, 0);
//	PtWindowFocus(m_windowMain);
        
	setCountFromActiveFrame ();
	_updateWindowData ();
}

void AP_QNXDialog_WordCount::destroy(void)
{
	if (!m_windowMain) {
		return;
	}

	m_bDestroy_says_stopupdating = true;
	m_pAutoUpdateWC->stop();
	m_answer = AP_Dialog_WordCount::a_CANCEL;	
	modeless_cleanup();

	PtWidget_t *win = m_windowMain;
	m_windowMain = NULL;
	PtDestroyWidget(win);
	DELETEP(m_pAutoUpdateWC);
}

void AP_QNXDialog_WordCount::notifyActiveFrame(XAP_Frame *pFrame)
{
	activate();
}


void AP_QNXDialog_WordCount::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(0);	//DEPRECATED
}

void AP_QNXDialog_WordCount::runModeless(XAP_Frame * pFrame)
{	
	// To center the dialog, we need the frame of its parent.
	
	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);

	// This magic command displays the frame that characters will be
	// inserted into.
	connectFocusModeless(mainWindow, m_pApp);

	event_Update();
	
//	UT_QNXCenterWindow(parentWindow, mainWindow);
	PtRealizeWidget(mainWindow);

	// Now construct the timer for auto-updating
	m_pAutoUpdateWC = UT_Timer::static_constructor(autoupdateWC, this);

	if(/* i_WordCountunix_first_time == 0 */ 1 )
	{	
		//  Set it update evey second to start with
		m_Update_rate = 1000;
		m_bAutoWC = true;
	}

	setUpdateCounter();
}

void    AP_QNXDialog_WordCount::setUpdateCounter( void )
{
	m_bDestroy_says_stopupdating = false;
	m_bAutoUpdate_happening_now = false;

	if(m_bAutoWC == true)
	{
		m_pAutoUpdateWC->stop();
		m_pAutoUpdateWC->set(m_Update_rate);
	}
	else
	{
	}
//	set_sensitivity();
}         

void    AP_QNXDialog_WordCount::autoupdateWC(UT_Worker * pTimer)
{
	UT_ASSERT(pTimer);

	// this is a static callback method and does not have a 'this' pointer.
	AP_QNXDialog_WordCount * pDialog =  (AP_QNXDialog_WordCount *) pTimer->getInstanceData();

	// Handshaking code
	if( pDialog->m_bDestroy_says_stopupdating != true)
	{
		pDialog->m_bAutoUpdate_happening_now = true;
		pDialog->event_Update();
		pDialog->m_bAutoUpdate_happening_now = false;
	}
}        

void AP_QNXDialog_WordCount::event_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_WordCount::a_OK;
	done = 1;
	destroy();
}

void AP_QNXDialog_WordCount::event_Update(void)
{
	setCountFromActiveFrame();
	_updateWindowData();
}

void AP_QNXDialog_WordCount::event_Checkbox(void)
{

	if(PtWidgetFlags(m_pAutocheck) & Pt_SET) {
		m_pAutoUpdateWC->stop();
		m_pAutoUpdateWC->set(m_Update_rate);
		m_bAutoWC = true;
	}else {
		m_pAutoUpdateWC->stop();
		m_bAutoWC = false;
	}	
}

void AP_QNXDialog_WordCount::event_Spin(void)
{
	double *val;
	PtGetResource(m_pAutospin,Pt_ARG_NUMERIC_VALUE,&val,0);

	m_Update_rate = 1000.0 * *val;
	m_pAutoUpdateWC->stop();
	m_pAutoUpdateWC->set(m_Update_rate);
}


void AP_QNXDialog_WordCount::event_WindowDelete(void)
{
	if (!done)
		m_answer = AP_Dialog_WordCount::a_CANCEL;	
	done = 1;
	destroy();
}

/*****************************************************************/

PtWidget_t * AP_QNXDialog_WordCount::_constructWindow(void)
{
	PtWidget_t *buttonOK;
	PtWidget_t *buttonUpdate;

	PtArg_t	args[10];
	char 	buffer[200];
	int 	n;


	const XAP_StringSet * pSS = m_pApp->getStringSet();

	n = 0;
	m_windowMain = abiCreatePhabDialog("ap_QNXDialog_WordCount",pSS,AP_STRING_ID_DLG_WordCount_WordCountTitle); 
	PtAddHotkeyHandler(m_windowMain,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	SetupContextHelp(m_windowMain,this);
	PtAddCallback(m_windowMain, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	PtSetResource(abiPhabLocateWidget(m_windowMain,"grpStorage"),Pt_ARG_TITLE,"WordCount",0);

	m_pAutocheck = abiPhabLocateWidget(m_windowMain,"toggleAupdate");
		PtAddCallback(m_pAutocheck,Pt_CB_ACTIVATE,s_autoupdate_clicked,this);
	m_pAutospin = abiPhabLocateWidget(m_windowMain,"aUpdateNFloat");
		PtAddCallback(m_pAutospin,Pt_CB_NUMERIC_CHANGED,s_autospinner_changed,this);

	/* Pages */
	localizeLabel(abiPhabLocateWidget(m_windowMain,"lblPages"),pSS,AP_STRING_ID_DLG_WordCount_Pages);
	m_labelPgCount = abiPhabLocateWidget(m_windowMain,"lblNumPages");

	/* Words */
	localizeLabel(abiPhabLocateWidget(m_windowMain,"lblWords"), pSS,AP_STRING_ID_DLG_WordCount_Words );
	m_labelWCount = abiPhabLocateWidget(m_windowMain,"lblNumWords");
	
	/* Characters (no spaces) */
	localizeLabel(abiPhabLocateWidget(m_windowMain,"lblCharNoSpace"), pSS, (AP_STRING_ID_DLG_WordCount_Characters_No ));

	m_labelCNCount = abiPhabLocateWidget(m_windowMain,"lblNumCharsNoSpace");

	/* Characters (spaces) */
	localizeLabel(abiPhabLocateWidget(m_windowMain,"lblCharWithSpace"), pSS, (AP_STRING_ID_DLG_WordCount_Characters_Sp ));

	m_labelCCount = abiPhabLocateWidget(m_windowMain,"lblNumCharsWithSpace"); 

	/* Paragraphs */
	localizeLabel(abiPhabLocateWidget(m_windowMain,"lblParagraphs"), pSS, (AP_STRING_ID_DLG_WordCount_Paragraphs ));

	m_labelPCount = abiPhabLocateWidget(m_windowMain,"lblNumParagraphs");

	/* Lines */
	localizeLabel(abiPhabLocateWidget(m_windowMain,"lblLines"), pSS, (AP_STRING_ID_DLG_WordCount_Lines ));

	m_labelLCount = abiPhabLocateWidget(m_windowMain,"lblNumLines"); 

	/* Close button */	
	buttonUpdate = abiPhabLocateWidget(m_windowMain,"btnUpdate");
	localizeLabel(buttonUpdate, pSS, XAP_STRING_ID_DLG_Update);
	PtAddCallback(buttonUpdate, Pt_CB_ACTIVATE, s_update_clicked, this);

	buttonOK = abiPhabLocateWidget(m_windowMain,"btnClose");
	localizeLabel(buttonOK, pSS, XAP_STRING_ID_DLG_Close);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	return m_windowMain;
}

void AP_QNXDialog_WordCount::_updateWindowData(void)
{
	char tmpbuf[50];

	// Update the data in the word count
	sprintf(tmpbuf, "%d", m_count.word);
	PtSetResource(m_labelWCount, Pt_ARG_TEXT_STRING, tmpbuf, 0);

	sprintf(tmpbuf, "%d", m_count.para);
	PtSetResource(m_labelPCount, Pt_ARG_TEXT_STRING, tmpbuf, 0);

	sprintf(tmpbuf, "%d", m_count.ch_sp);
	PtSetResource(m_labelCCount, Pt_ARG_TEXT_STRING, tmpbuf, 0);

	sprintf(tmpbuf, "%d", m_count.ch_no);
	PtSetResource(m_labelCNCount, Pt_ARG_TEXT_STRING, tmpbuf, 0);

	sprintf(tmpbuf, "%d", m_count.line);
	PtSetResource(m_labelLCount, Pt_ARG_TEXT_STRING, tmpbuf, 0);

	sprintf(tmpbuf, "%d", m_count.page);
	PtSetResource(m_labelPgCount, Pt_ARG_TEXT_STRING, tmpbuf, 0);
}

void AP_QNXDialog_WordCount::_populateWindowData(void)
{
	UT_ASSERT(0);	//DEPRECATED
}


