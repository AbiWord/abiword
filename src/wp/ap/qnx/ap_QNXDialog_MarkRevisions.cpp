/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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
#include "ap_QNXDialog_MarkRevisions.h"
#include "ut_qnxHelper.h"


/*****************************************************************/

XAP_Dialog * AP_QNXDialog_MarkRevisions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_MarkRevisions * p = new AP_QNXDialog_MarkRevisions(pFactory,id);
	return p;
}

AP_QNXDialog_MarkRevisions::AP_QNXDialog_MarkRevisions(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_MarkRevisions(pDlgFactory,id)
{
}

AP_QNXDialog_MarkRevisions::~AP_QNXDialog_MarkRevisions(void)
{
}
int ph_event_ok(PtWidget_t *widget,AP_QNXDialog_MarkRevisions *dlg,PtCallbackInfo_t *cbinfo)
{
dlg->event_OK();
return Pt_CONTINUE;
}

void AP_QNXDialog_MarkRevisions::event_OK()
{
char *text;

m_answer= a_OK;
PtGetResource(m_text1,Pt_ARG_TEXT_STRING,&text,0);
setComment2(text);
done=1;
}

int ph_event_cancel(PtWidget_t *widget,AP_QNXDialog_MarkRevisions *dlg,PtCallbackInfo_t *cbinfo)
{
dlg->event_Cancel();
return Pt_CONTINUE;
}

void AP_QNXDialog_MarkRevisions::event_Cancel()
{
m_answer = a_CANCEL;
done=1;
}

int ph_event_close(PtWidget_t *widget,AP_QNXDialog_MarkRevisions *dlg,PtCallbackInfo_t *cbinfo)
{
dlg->done=1;
return Pt_CONTINUE;
}

int ph_event_toggle_change(PtWidget_t *widget,AP_QNXDialog_MarkRevisions *dlg,PtCallbackInfo_t *cbinfo)
{
dlg->event_toggle(widget);
return Pt_CONTINUE;
}

void AP_QNXDialog_MarkRevisions::event_toggle(PtWidget_t *widget)
{

if(widget == m_toggle1)
{
PtSetResource(m_text1,Pt_ARG_FLAGS,Pt_TRUE,Pt_BLOCKED|Pt_GHOST);
} else if(widget == m_toggle2)
{
PtSetResource(m_text1,Pt_ARG_FLAGS,Pt_FALSE,Pt_BLOCKED|Pt_GHOST);
}
}

void AP_QNXDialog_MarkRevisions::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	/*
	   This is the only function you need to implement, and the MarkRevisions
	   dialogue should look like this:

	   ----------------------------------------------------
	   | Title                                            |
     ----------------------------------------------------
	   |                                                  |
	   | O Radio1                                         |
	   |    Comment1 (a label)                            |
	   |                                                  |
	   | O Radio2                                         |
	   |    Comment2Label                                 |
	   |    Comment2 (an edit control)                    |
     |                                                  |
     |                                                  |
     |     OK_BUTTON              CANCEL_BUTTON         |
	   ----------------------------------------------------

	   Where: Title, Comment1 and Comment2Label are labels, Radio1-2
	   is are radio buttons, Comment2 is an Edit control.

	   Use getTitle(), getComment1(), getComment2Label(), getRadio1Label()
	   and getRadio2Label() to get the labels (the last two for the radio
	   buttons), note that you are responsible for freeing the
	   pointers returned by getLable1() and getComment1() using FREEP
	   (but not the rest!)

	   if getLabel1() returns NULL, hide the radio buttons and enable
	   the Edit box; otherwise the Edit box should be only enabled when
	   Radio2 is selected.

	   Use setComment2(const char * pszString) to store the contents of the Edit control
       when the dialogue closes; make sure that you freee pszString afterwards.
	*/

	PtWidget_t *mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);
	connectFocus(mainWindow,pFrame);
	
   // To center the dialog, we need the frame of its parent.
 	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

    // Center our new dialog in its parent and make it a transient
    // so it won't get lost underneath
    // Make it modal, and stick it up top
	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);

    // Show the top level dialog,
	PtRealizeWidget(mainWindow);

    // Run the event loop for this window.
	int count;
	count = PtModalStart();
	done=0;
	do {
    		PtProcessEvent();
	} while (!done);

	PtModalEnd(MODAL_END_ARG(count));
	UT_QNXBlockWidget(parentWindow,0);
	PtDestroyWidget(mainWindow);
}

PtWidget_t * AP_QNXDialog_MarkRevisions::_constructWindow()
{
PtWidget_t *mainwindow;
PtWidget_t *btnOk;
PtWidget_t *btnCancel;
PtWidget_t *toggle1,*toggle2;
PtWidget_t *text1;
PtWidget_t *label1,*label2;

const XAP_StringSet *pSS = m_pApp->getStringSet();


	mainwindow = abiCreatePhabDialog("ap_QNXDialog_MarkRevisions",pSS,XAP_STRING_ID_DLG_Cancel);

	PtSetResource(mainwindow,Pt_ARG_WINDOW_TITLE,(char *)getTitle(),0); 
	SetupContextHelp(mainwindow,this);
	PtAddHotkeyHandler(mainwindow,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);

	toggle1= abiPhabLocateWidget(mainwindow,"toggleContinue"); 
	toggle2 = abiPhabLocateWidget(mainwindow,"toggleNew");  
	label2 = abiPhabLocateWidget(mainwindow,"lblNewName"); 
	text1 =abiPhabLocateWidget(mainwindow,"textName");  


	btnOk = abiPhabLocateWidget(mainwindow,"btnOK");
	localizeLabel(btnOk,pSS,XAP_STRING_ID_DLG_OK);		
	btnCancel = abiPhabLocateWidget(mainwindow,"btnCancel");
	localizeLabel(btnCancel,pSS,XAP_STRING_ID_DLG_Cancel);

	char *pStr = getRadio1Label(); 
	if(pStr){
	  PtSetResource(toggle1,Pt_ARG_TEXT_STRING,pStr,0);
	  PtSetResource(toggle1,Pt_ARG_FLAGS,Pt_TRUE,Pt_SET);
	  FREEP(pStr);

	  pStr = getComment1();
	  PtSetResource(label1,Pt_ARG_TEXT_STRING,pStr,0);
	  FREEP(pStr);
	  PtSetResource(toggle2,Pt_ARG_TEXT_STRING,getRadio2Label(),0);
	  //Disable edit box.	  
	  PtSetResource(text1,Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);
	}else { //There are no revisions in this doc yet, so everything is ghosted except edit control.
	PtSetResource(toggle1,Pt_ARG_FLAGS,Pt_TRUE,Pt_DELAY_REALIZE);
	PtSetResource(toggle2,Pt_ARG_FLAGS,Pt_TRUE,Pt_DELAY_REALIZE);
	PtSetResource(abiPhabLocateWidget(mainwindow,"seperator"),Pt_ARG_FLAGS,Pt_TRUE,Pt_DELAY_REALIZE);
      }

      PtSetResource(label2,Pt_ARG_TEXT_STRING,getComment2Label(),0);

      PtAddCallback(btnOk,Pt_CB_ACTIVATE,ph_event_ok,this);
      PtAddCallback(text1,Pt_CB_ACTIVATE,ph_event_ok,this);
      PtAddCallback(btnCancel,Pt_CB_ACTIVATE,ph_event_cancel,this);
      PtAddCallback(mainwindow,Pt_CB_WINDOW_CLOSING,ph_event_close,this);
      PtAddCallback(toggle1,Pt_CB_ACTIVATE,ph_event_toggle_change,this);
      PtAddCallback(toggle2,Pt_CB_ACTIVATE,ph_event_toggle_change,this);

			m_toggle2 = toggle2;
			m_text1		= text1;
			m_toggle1	= toggle1;
return mainwindow;
}

