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
#include "xap_QNXFrame.h"

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

static int s_autocheck_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_WordCount * dlg = (AP_QNXDialog_WordCount *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Checkbox();
	return Pt_CONTINUE;
}

static int s_updateRate_changed(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_WordCount * dlg = (AP_QNXDialog_WordCount *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Spin();
	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_WordCount * dlg = (AP_QNXDialog_WordCount *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}

/*****************************************************************/

void  AP_QNXDialog_WordCount::activate(void)
{
	UT_ASSERT (m_windowMain);

	ConstructWindowName();
	PtSetResource(m_windowMain, Pt_ARG_WINDOW_TITLE, m_WindowName, 0);
	PtWindowFocus(m_windowMain);
        
	setCountFromActiveFrame ();
	_updateWindowData ();
}

void AP_QNXDialog_WordCount::destroy(void)
{
	if (!m_windowMain) {
		return;
	}

	m_bDestroy_says_stopupdating = UT_TRUE;
	while (m_bAutoUpdate_happening_now == UT_TRUE) ;
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
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the Window of the parent frame
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	PtSetParentWidget(parentWindow);

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
	
	UT_QNXCenterWindow(parentWindow, mainWindow);
	PtRealizeWidget(mainWindow);

	// Now construct the timer for auto-updating
	m_pAutoUpdateWC = UT_Timer::static_constructor(autoupdateWC, this);

	if(/* i_WordCountunix_first_time == 0 */ 1 )
	{	
		//  Set it update evey second to start with
		m_Update_rate = 1000;
		m_bAutoWC = UT_TRUE;
		//i_WordCountunix_first_time = 1;
	}

	setUpdateCounter();
}

void    AP_QNXDialog_WordCount::setUpdateCounter( void )
{
	m_bDestroy_says_stopupdating = UT_FALSE;
	m_bAutoUpdate_happening_now = UT_FALSE;

	float f_Update_rate = ((float) m_Update_rate)/ 1000.0;
/*
	gtk_adjustment_set_value( m_Spinrange, f_Update_rate );
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON( m_pAutospin), m_Spinrange);
*/
	if(m_bAutoWC == UT_TRUE)
	{
		m_pAutoUpdateWC->stop();
		m_pAutoUpdateWC->set(m_Update_rate);
//		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_pAutocheck), TRUE);
	}
	else
	{
//		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_pAutocheck), FALSE);
	}
//	set_sensitivity();
}         

void    AP_QNXDialog_WordCount::autoupdateWC(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);

	// this is a static callback method and does not have a 'this' pointer.
	AP_QNXDialog_WordCount * pDialog =  (AP_QNXDialog_WordCount *) pTimer->getInstanceData();

	// Handshaking code
	if( pDialog->m_bDestroy_says_stopupdating != UT_TRUE)
	{
		pDialog->m_bAutoUpdate_happening_now = UT_TRUE;
		pDialog->event_Update();
		pDialog->m_bAutoUpdate_happening_now = UT_FALSE;
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
#if 0
	if(m_pAutocheck->active)
	{
		m_pAutoUpdateWC->stop();
		// This actually does gtk_timer_add...
		m_pAutoUpdateWC->set(m_Update_rate);
		m_bAutoWC = UT_TRUE;
	}
	else
	{
		m_pAutoUpdateWC->stop();
		m_bAutoWC = UT_FALSE;
	}
	set_sensitivity();
#endif
}

void AP_QNXDialog_WordCount::event_Spin(void)
{
#if 0
	float update_rate =  1000.0*gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(m_pAutospin));
	m_Update_rate = (guint) update_rate;

	// We need this because calling adds a new timer to the gtk list!
	// So we have to stop the timer to remove it from the gtk list before
	// changing the speed of the timer.

	m_pAutoUpdateWC->stop();

	// This actually does gtk_timer_add...

	m_pAutoUpdateWC->set(m_Update_rate);
#endif
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
	XML_Char * unixstr = NULL;	// used for conversions

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(AP_STRING_ID_DLG_WordCount_WordCountTitle), 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	m_windowMain = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(m_windowMain, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_HORIZONTAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ROWS_COLS, 2, 0);
#define MARGIN_SIZE 10 
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, MARGIN_SIZE, 0);
	PtWidget_t *hbox = PtCreateWidget(PtGroup, m_windowMain, n, args);

	/* TODO: Add in a checkbox and spinner at the top here, and
	         Remove the Statistics stuff
	*/
	m_pAutocheck = NULL;
	m_pAutospin = NULL;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtWidget_t *vboxlabel = PtCreateWidget(PtGroup, hbox, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtWidget_t *vboxvalue = PtCreateWidget(PtGroup, hbox, n, args);

	/* Statistics placeholder */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, "Statistics:", 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	n = 0;	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, " ", 0);
	PtCreateWidget(PtLabel, vboxvalue, n, args);

	/* Pages */
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Pages));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	FREEP(unixstr);

	n = 0;
	sprintf(buffer, "%d", m_count.page);
	//PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, buffer, 0);
	PtSetArg(&args[n++], Pt_ARG_HORIZONTAL_ALIGNMENT, Pt_RIGHT, 0);
	m_labelPgCount = PtCreateWidget(PtLabel, vboxvalue, n, args);

	/* Words */
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Words));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	FREEP(unixstr);

	n = 0;
	sprintf(buffer, "%d", m_count.word);
	//PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, buffer, 0);
	PtSetArg(&args[n++], Pt_ARG_HORIZONTAL_ALIGNMENT, Pt_RIGHT, 0);
	m_labelWCount = PtCreateWidget(PtLabel, vboxvalue, n, args);

	/* Characters (no spaces) */
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Characters_No));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	FREEP(unixstr);

	n = 0;
	sprintf(buffer, "%d", m_count.ch_no);
	//PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, buffer, 0);
	PtSetArg(&args[n++], Pt_ARG_HORIZONTAL_ALIGNMENT, Pt_RIGHT, 0);
	m_labelCNCount = PtCreateWidget(PtLabel, vboxvalue, n, args);

	/* Characters (spaces) */
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Characters_Sp));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	FREEP(unixstr);

	n = 0;
	sprintf(buffer, "%d", m_count.ch_sp);
	//PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, buffer, 0);
	PtSetArg(&args[n++], Pt_ARG_HORIZONTAL_ALIGNMENT, Pt_RIGHT, 0);
	m_labelCCount = PtCreateWidget(PtLabel, vboxvalue, n, args);


	/* Paragraphs */
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Paragraphs));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	FREEP(unixstr);

	n = 0;
	sprintf(buffer, "%d", m_count.para);
	//PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, buffer, 0);
	PtSetArg(&args[n++], Pt_ARG_HORIZONTAL_ALIGNMENT, Pt_RIGHT, 0);
	m_labelPCount = PtCreateWidget(PtLabel, vboxvalue, n, args);

	/* Lines */
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Lines));	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	PtCreateWidget(PtLabel, vboxlabel, n, args);
	FREEP(unixstr);

	n = 0;
	sprintf(buffer, "%d", m_count.line);
	//PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, buffer, 0);
	PtSetArg(&args[n++], Pt_ARG_HORIZONTAL_ALIGNMENT, Pt_RIGHT, 0);
	m_labelLCount = PtCreateWidget(PtLabel, vboxvalue, n, args);

	/* Close button */	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Update), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonUpdate = PtCreateWidget(PtButton, vboxlabel, n, args);
	PtAddCallback(buttonUpdate, Pt_CB_ACTIVATE, s_update_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Close), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonOK = PtCreateWidget(PtButton, vboxvalue, n, args);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	return m_windowMain;
}

void AP_QNXDialog_WordCount::_updateWindowData(void)
{
	char tmpbuf[50];

	// Update the data in the word count
	sprintf(tmpbuf, "%10d", m_count.word);
	PtSetResource(m_labelWCount, Pt_ARG_TEXT_STRING, tmpbuf, 0);

	sprintf(tmpbuf, "%10d", m_count.para);
	PtSetResource(m_labelPCount, Pt_ARG_TEXT_STRING, tmpbuf, 0);

	sprintf(tmpbuf, "%10d", m_count.ch_sp);
	PtSetResource(m_labelCCount, Pt_ARG_TEXT_STRING, tmpbuf, 0);

	sprintf(tmpbuf, "%10d", m_count.ch_no);
	PtSetResource(m_labelCNCount, Pt_ARG_TEXT_STRING, tmpbuf, 0);

	sprintf(tmpbuf, "%10d", m_count.line);
	PtSetResource(m_labelLCount, Pt_ARG_TEXT_STRING, tmpbuf, 0);

	sprintf(tmpbuf, "%10d", m_count.page);
	PtSetResource(m_labelPgCount, Pt_ARG_TEXT_STRING, tmpbuf, 0);
}

void AP_QNXDialog_WordCount::_populateWindowData(void)
{
	UT_ASSERT(0);	//DEPRECATED
}


