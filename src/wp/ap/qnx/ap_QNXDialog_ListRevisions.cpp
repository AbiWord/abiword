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
#include "xap_QNXFrame.h"

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
  m_id=atoi(items[*selection]); 
  }else m_id = 0;

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
PtWidget_t *title;
PtWidget_t *lstRevisions;

const XAP_StringSet *pSS = m_pApp->getStringSet();

	static const PhDim_t dim = { 313,344}; 
	static const PtArg_t args[] = {
	    Pt_ARG( Pt_ARG_DIM,&dim, 0),
	    Pt_ARG( Pt_ARG_WINDOW_TITLE,getTitle(),0),
	Pt_ARG(Pt_ARG_WINDOW_RENDER_FLAGS,0,ABI_MODAL_WINDOW_RENDER_FLAGS),
	Pt_ARG(Pt_ARG_WINDOW_MANAGED_FLAGS,0,ABI_MODAL_WINDOW_MANAGE_FLAGS),
	};

	static const PhArea_t area1 = { { 3, 3 }, { 308, 308 } };
	static const PtArg_t args1[] = {
		Pt_ARG( Pt_ARG_AREA, &area1, 0 ),
		Pt_ARG( Pt_ARG_ANCHOR_FLAGS, 1440,8191 ),
		};

	static const PhArea_t area2 = { { 1, 0 }, { 341, 30 } };
	static const PtArg_t args2[] = {
		Pt_ARG( Pt_ARG_AREA, &area2, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 1280,1280 ),
		Pt_ARG( Pt_ARG_BEVEL_WIDTH, 1, 0 ),
		Pt_ARG( Pt_ARG_RESIZE_FLAGS, 0x1200000,0x3f00000 ),
		};

	static const PhArea_t area3 = { { 0, 0 }, { 78, 28 } };
	static const PtArg_t args3[] = {
		Pt_ARG( Pt_ARG_AREA, &area3, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, getColumn1Label(), 0 ),
		};

	static const PhArea_t area4 = { { 78, 0 }, { 261, 28 } };
	static const PtArg_t args4[] = {
		Pt_ARG( Pt_ARG_AREA, &area4, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, getColumn2Label(), 0 ),
		};

	static const PhArea_t area5 = { { 241, 314 }, { 70, 27 } };
	static const PtArg_t args5[] = {
		Pt_ARG( Pt_ARG_AREA, &area5, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(XAP_STRING_ID_DLG_OK).c_str(), 0 ),
		};

	static const PhArea_t area6 = { { 166, 314 }, { 70, 27 } };
	static const PtArg_t args6[] = {
		Pt_ARG( Pt_ARG_AREA, &area6, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel).c_str(), 0 ),
		};

	mainwindow=PtCreateWidget(PtWindow,NULL,sizeof(args) / sizeof(PtArg_t),args);

	lstRevisions = PtCreateWidget( PtList, NULL, sizeof(args1) / sizeof(PtArg_t), args1 );

	PtCreateWidget( PtDivider, NULL, sizeof(args2) / sizeof(PtArg_t), args2 );

	PtCreateWidget( PtLabel, NULL, sizeof(args3) / sizeof(PtArg_t), args3 );

	PtCreateWidget( PtLabel, NULL, sizeof(args4) / sizeof(PtArg_t), args4 );

	btnOk = PtCreateWidget( PtButton, mainwindow, sizeof(args5) / sizeof(PtArg_t), args5 );

	btnCancel = PtCreateWidget( PtButton, mainwindow, sizeof(args6) / sizeof(PtArg_t), args6 );

       /* Add items to list */
       for(UT_uint32 i=0;i < getItemCount();i++)
      {
	char text[150];
	char *str=getNthItemText(i);
	sprintf(text,"%d\t",getNthItemId(i));
	strncat(text,str,140);
	FREEP(str);
	PtListAddItems(lstRevisions,(const char **)&text,1,0);
      }
	PtAddCallback(btnOk,Pt_CB_ACTIVATE,ph_event_ok,this);
	PtAddCallback(btnCancel,Pt_CB_ACTIVATE,ph_event_cancel,this);
	PtAddCallback(mainwindow,Pt_CB_WINDOW_CLOSING,ph_event_close,this);

	m_lstRevision = lstRevisions;
		
return mainwindow;
}

