/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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
#include <Pt.h>

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
#include "ap_QNXDialog_ListRevisions.h"
#include "ut_qnxHelper.h"


/*****************************************************************/

XAP_Dialog * AP_QNXDialog_ListRevisions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_ListRevisions * p = new AP_QNXDialog_ListRevisions(pFactory,id);
	return p;
}

AP_QNXDialog_ListRevisions::AP_QNXDialog_ListRevisions(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_ListRevisions(pDlgFactory,id)
{
}

AP_QNXDialog_ListRevisions::~AP_QNXDialog_ListRevisions(void)
{
}

void AP_QNXDialog_ListRevisions::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	/*
	   see the screenshot posted to the dev-list (25/05/2002);
	   use the provided functions getTitle(), getLabel1(),
	   getColumn1Label(), getColumn2Label(), getItemCount(),
	   getNthItemId() and getNthItemText() to fill the list

	   if the user clicks OK but there is no selection, set m_aswer to
	   CANCEL; otherwise do not forget to set m_iId to the id of the
	   selected revision
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

int ph_event_ok(PtWidget_t *widget,AP_QNXDialog_ListRevisions *dlg,PtCallbackInfo_t *cbinfo)
{
dlg->event_ok();
return Pt_CONTINUE;
}

void AP_QNXDialog_ListRevisions::event_ok()
{
  char **items;
  short *selection=0;
  PtGetResource(m_lstRevision,Pt_ARG_ITEMS,&items,0);
  PtGetResource(m_lstRevision,Pt_ARG_SELECTION_INDEXES,&selection,0);
  if(selection && *selection) {
  m_iId=atoi(items[*selection]); 
  }else m_iId = 0;

m_answer=a_OK;
done=1;
}


int ph_event_cancel(PtWidget_t *widget,AP_QNXDialog_ListRevisions *dlg,PtCallbackInfo_t *cbinfo)
{
dlg->event_cancel();
return Pt_CONTINUE;
}

void AP_QNXDialog_ListRevisions::event_cancel()
{
m_answer=a_CANCEL;
done=1;
}

int ph_event_close(PtWidget_t *widget,AP_QNXDialog_ListRevisions *dlg,PtCallbackInfo_t *cbinfo)
{
UT_ASSERT(dlg);
dlg->done=1;
return Pt_CONTINUE;
}

PtWidget_t * AP_QNXDialog_ListRevisions::_constructWindow(void)
{
PtWidget_t *mainwindow;
PtWidget_t *btnOk;
PtWidget_t *btnCancel;
PtWidget_t *lstRevisions;

const XAP_StringSet *pSS = m_pApp->getStringSet();

	mainwindow= abiCreatePhabDialog("ap_QNXDialog_ListRevisions",pSS,XAP_STRING_ID_DLG_Cancel);
	PtSetResource(mainwindow,Pt_ARG_WINDOW_TITLE,(char*)getTitle(),0);
	SetupContextHelp(mainwindow,this);
	PtAddHotkeyHandler(mainwindow,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);

	lstRevisions = abiPhabLocateWidget(mainwindow,"listRevisions"); 
	
	PtSetResource(abiPhabLocateWidget(mainwindow,"lblColumn1"),Pt_ARG_TEXT_STRING,getColumn1Label(),0);

	PtSetResource(abiPhabLocateWidget(mainwindow,"lblColumn2"),Pt_ARG_TEXT_STRING,getColumn2Label(),0);

	btnOk = abiPhabLocateWidget(mainwindow,"btnOK"); 
	localizeLabel(btnOk,pSS,XAP_STRING_ID_DLG_OK);

	btnCancel = abiPhabLocateWidget(mainwindow,"btnCancel"); 
	localizeLabel(btnCancel,pSS,XAP_STRING_ID_DLG_Cancel);

       /* Add items to list */
  for(UT_uint32 i=0;i < getItemCount();i++)
    {
	char text[150];
	char *tmp[1];
	char *str=getNthItemText(i);
	sprintf(text,"%d\t",getNthItemId(i));
	strncat(text,str,140);
	tmp[0] =text;
	FREEP(str);
	PtListAddItems(lstRevisions,(const char **)&tmp,1,0);
      }

	PtAddCallback(btnOk,Pt_CB_ACTIVATE,ph_event_ok,this);
	PtAddCallback(btnCancel,Pt_CB_ACTIVATE,ph_event_cancel,this);
	PtAddCallback(mainwindow,Pt_CB_WINDOW_CLOSING,ph_event_close,this);

	m_lstRevision = lstRevisions;
		
return mainwindow;
}

