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
#include "xap_QNXFrame.h"

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
  if(PtWidgetFlags(m_toggle2) & Pt_SET)
  {
   char *text;
    PtGetResource(m_text1,Pt_ARG_TEXT_STRING,&text,0);
    setComment2(text);
  }
m_answer= a_OK;
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
    XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
    UT_ASSERT(pQNXFrame);
    
    // Get the Window of the parent frame
    PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
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


	static const PhDim_t dim = { 277,236 };
	static const PtArg_t args[] = {
		Pt_ARG(Pt_ARG_DIM,&dim,0),
		Pt_ARG(Pt_ARG_WINDOW_TITLE,getTitle(),0),
		Pt_ARG(Pt_ARG_WINDOW_MANAGED_FLAGS,0,ABI_MODAL_WINDOW_MANAGE_FLAGS),
		Pt_ARG(Pt_ARG_WINDOW_RENDER_FLAGS,0,ABI_MODAL_WINDOW_RENDER_FLAGS),
	      };

	static const PhArea_t area1 = { { 201, 205 }, { 70, 27 } };
	static const PtArg_t args1[] = {
		Pt_ARG( Pt_ARG_AREA, &area1, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(XAP_STRING_ID_DLG_OK).c_str(), 0 ),
		};

	static const PhArea_t area2 = { { 129, 205 }, { 70, 27 } };
	static const PtArg_t args2[] = {
		Pt_ARG( Pt_ARG_AREA, &area2, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel).c_str(), 0 ),
		};

	static const PhArea_t area3 = { { 6, 190 }, { 265, 7 } };
	static const PtArg_t args3[] = {
		Pt_ARG( Pt_ARG_AREA, &area3, 0 ),
		};

	static const PhArea_t area4 = { { 3, 66 }, { 268, 7 } };
	static const PtArg_t args4[] = {
		Pt_ARG( Pt_ARG_AREA, &area4, 0 ),
		};

	static const PhArea_t area5 = { { 6, 7 }, { 75, 24 } };
	static const PtArg_t args5[] = {
		Pt_ARG( Pt_ARG_AREA, &area5, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		Pt_ARG( Pt_ARG_INDICATOR_TYPE, 3, 0 ),
		};

	static const PhArea_t area6 = { { 6, 84 }, { 75, 24 } };
	static const PtArg_t args6[] = {
		Pt_ARG( Pt_ARG_AREA, &area6, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Toggle 2", 0 ),
		Pt_ARG( Pt_ARG_INDICATOR_TYPE, 3, 0 ),
		};

	static const PhArea_t area7 = { { 30, 37 }, { 235, 21 } };
	static const PtArg_t args7[] = {
		Pt_ARG( Pt_ARG_AREA, &area7, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Label 1", 0 ),
		};

	static const PhArea_t area8 = { { 30, 117 }, { 236, 21 } };
	static const PtArg_t args8[] = {
		Pt_ARG( Pt_ARG_AREA, &area8, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Label 2", 0 ),
		};

	static const PhArea_t area9 = { { 30, 144 }, { 241, 27 } };
	static const PtArg_t args9[] = {
		Pt_ARG( Pt_ARG_AREA, &area9, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		};
      
	mainwindow = PtCreateWidget( PtWindow,NULL,sizeof(args) / sizeof(PtArg_t), args);

	btnOk=PtCreateWidget( PtButton, NULL, sizeof(args1) / sizeof(PtArg_t), args1 );

	btnCancel=PtCreateWidget( PtButton, NULL, sizeof(args2) / sizeof(PtArg_t), args2 );

	PtCreateWidget( PtSeparator, NULL, sizeof(args3) / sizeof(PtArg_t), args3 );

	PtCreateWidget( PtSeparator, NULL, sizeof(args4) / sizeof(PtArg_t), args4 );

	toggle1=PtCreateWidget( PtToggleButton, NULL, sizeof(args5) / sizeof(PtArg_t), args5 );

	toggle2=PtCreateWidget( PtToggleButton, NULL, sizeof(args6) / sizeof(PtArg_t), args6 );

	label1 = PtCreateWidget( PtLabel, NULL, sizeof(args7) / sizeof(PtArg_t), args7 );

	label2 =PtCreateWidget( PtLabel, NULL, sizeof(args8) / sizeof(PtArg_t), args8 );

	text1=PtCreateWidget( PtText, NULL, sizeof(args9) / sizeof(PtArg_t), args9 );

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
	PtSetResource(toggle1,Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);
	PtSetResource(toggle2,Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);
	PtSetResource(label1,Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);

      }
PtSetResource(label2,Pt_ARG_TEXT_STRING,getComment2Label(),0);

      PtAddCallback(btnOk,Pt_CB_ACTIVATE,ph_event_ok,this);
      PtAddCallback(btnCancel,Pt_CB_ACTIVATE,ph_event_ok,this);
      PtAddCallback(mainwindow,Pt_CB_WINDOW_CLOSING,ph_event_close,this);
      PtAddCallback(toggle1,Pt_CB_ACTIVATE,ph_event_toggle_change,this);
      PtAddCallback(toggle2,Pt_CB_ACTIVATE,ph_event_toggle_change,this);

return mainwindow;
}

