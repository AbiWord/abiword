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
#include "ap_QNXDialog_InsertHyperlink.h"
#include "ut_qnxHelper.h"


/*****************************************************************/
int ph_event_ok( PtWidget_t *widget, AP_QNXDialog_InsertHyperlink * dlg, 
           PtCallbackInfo_t *info)
{
UT_ASSERT(widget && dlg);
dlg->event_OK();

return Pt_CONTINUE;
}

void AP_QNXDialog_InsertHyperlink::event_OK(void)
{
	char *res;
	// get the bookmark name, if any (return cancel if no name given)
	
	PtGetResource(mSelectedBookmark,Pt_ARG_TEXT_STRING,&res,0);
	
	if(res && *res)	
	{
		setAnswer(AP_Dialog_InsertHyperlink::a_OK);
		setHyperlink((XML_Char*)res);
	}
	else
	{
		setAnswer(AP_Dialog_InsertHyperlink::a_CANCEL);
	}
	done=1;
}

int ph_event_cancel( PtWidget_t *widget, AP_QNXDialog_InsertHyperlink * dlg, 
           PtCallbackInfo_t *info)
{
UT_ASSERT(widget && dlg);
dlg->event_Cancel();
return Pt_CONTINUE;
}
void AP_QNXDialog_InsertHyperlink::event_Cancel(void)
{
	setAnswer(AP_Dialog_InsertHyperlink::a_CANCEL);
	done=1;
}

int ph_listselection( PtWidget_t *widget, AP_QNXDialog_InsertHyperlink * dlg, PtCallbackInfo_t *info)
{
UT_ASSERT(widget && dlg);

PtListCallback_t *cb=(PtListCallback_t*)info->cbdata;

if(info->reason_subtype == (Pt_LIST_SELECTION_FINAL || Pt_LIST_SELECTION_BROWSE))
{
PtSetResource(dlg->mSelectedBookmark,Pt_ARG_TEXT_STRING,cb->item,0);
}
return Pt_CONTINUE;
}
int ph_event_close( PtWidget_t *widget, AP_QNXDialog_InsertHyperlink * dlg, PtCallbackInfo_t *info)
{
UT_ASSERT(widget && dlg);
dlg->done=1;
return Pt_CONTINUE;
}

XAP_Dialog * AP_QNXDialog_InsertHyperlink::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_InsertHyperlink * p = new AP_QNXDialog_InsertHyperlink(pFactory,id);
	return p;
}

AP_QNXDialog_InsertHyperlink::AP_QNXDialog_InsertHyperlink(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_InsertHyperlink(pDlgFactory,id)
{
}

AP_QNXDialog_InsertHyperlink::~AP_QNXDialog_InsertHyperlink(void)
{
}

void AP_QNXDialog_InsertHyperlink::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
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

PtWidget_t *AP_QNXDialog_InsertHyperlink::_constructWindow(void)
{
const XAP_StringSet *pSS = m_pApp->getStringSet();
PtWidget_t *windowHyperlink;
PtWidget_t *PtButton_ok;
PtWidget_t *PtButton_cancel;


		PhDim_t dim = { 250,100 };
		PtArg_t args[] = {
			Pt_ARG(Pt_ARG_DIM,&dim,0),
			Pt_ARG(Pt_ARG_WINDOW_TITLE,pSS->getValueUTF8(AP_STRING_ID_DLG_InsertHyperlink_Title).c_str(),0),
			Pt_ARG(Pt_ARG_WINDOW_RENDER_FLAGS,0,ABI_MODAL_WINDOW_RENDER_FLAGS),
			Pt_ARG(Pt_ARG_WINDOW_MANAGED_FLAGS,0,ABI_MODAL_WINDOW_MANAGE_FLAGS)
		};
		//Top Pane.
	 PhArea_t area1 = { { 0, 0 }, { 248, 202 } };
	 PtArg_t args1[] = {
		Pt_ARG( Pt_ARG_AREA, &area1, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BEVEL_WIDTH, 1, 0 ),
		};
	//PtList.
	 PhArea_t area2 = { { 3, 65 }, { 239, 131 } };
	 PtArg_t args2[] = {
		Pt_ARG( Pt_ARG_AREA, &area2, 0 ),
		};
	//PtList.
	 PhArea_t area3 = { { 2, 35 }, { 240, 27 } };
	 PtArg_t args3[] = {
		Pt_ARG( Pt_ARG_AREA, &area3, 0 ),
		};
	//Msg label.
	 PhArea_t area4 = { { 2, 5 }, { 241, 21 } };
	 PtArg_t args4[] = {
		Pt_ARG( Pt_ARG_AREA, &area4, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValueUTF8(AP_STRING_ID_DLG_InsertHyperlink_Msg).c_str() , 0 ),
		};
	//Bottom pane.
	 PhArea_t area5 = { { 0, 201 }, { 248, 33 } };
	 PtArg_t args5[] = {
		Pt_ARG( Pt_ARG_AREA, &area5, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 258,1334445470 ),
		Pt_ARG( Pt_ARG_BEVEL_WIDTH, 1, 0 ),
		Pt_ARG( Pt_ARG_FILL_COLOR, 0xc0c0c0, 0 ),
		Pt_ARG( Pt_ARG_BASIC_FLAGS, 67056,4194303 ),
		};
	//Cancel button
	 PhArea_t area6 = { { 72, 2 }, { 82, 27 } };
	 PtArg_t args6[] = {
		Pt_ARG( Pt_ARG_AREA, &area6, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel).c_str(), 0 ),
		};
	//OK button
	 PhArea_t area7 = { { 160, 2 }, { 82, 27 } };
	 PtArg_t args7[] = {
		Pt_ARG( Pt_ARG_AREA, &area7, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(XAP_STRING_ID_DLG_OK).c_str(), 0 ),
		};

	windowHyperlink= PtCreateWidget(PtWindow,NULL,sizeof(args) / sizeof(PtArg_t),args);

	PtCreateWidget( PtPane, NULL, sizeof(args1) / sizeof(PtArg_t), args1 );

	mBookmarkList=PtCreateWidget( PtList, NULL, sizeof(args2) / sizeof(PtArg_t), args2 );

	mSelectedBookmark=PtCreateWidget( PtText, NULL, sizeof(args3) / sizeof(PtArg_t), args3 );

	PtCreateWidget( PtLabel, NULL, sizeof(args4) / sizeof(PtArg_t), args4 );

	PtCreateWidget( PtPane, windowHyperlink, sizeof(args5) / sizeof(PtArg_t), args5 );

	PtButton_cancel=PtCreateWidget( PtButton, NULL, sizeof(args6) / sizeof(PtArg_t), args6 );

	PtButton_ok=PtCreateWidget( PtButton, NULL, sizeof(args7) / sizeof(PtArg_t), args7 );

	//Add existing bookmarks to the list.
 XML_Char ** pBookmarks = (XML_Char **)calloc(getExistingBookmarksCount(),sizeof(XML_Char*));
	if(getHyperlink())
	{
	int start,end;
	start=0;	
	end=1000;
		PtSetResource(mSelectedBookmark,Pt_ARG_TEXT_STRING,getHyperlink(),0);
		PtTextSetSelection(mSelectedBookmark,&start,&end);
	}
	
  for (int i = 0; i < (int)getExistingBookmarksCount(); i++)
   pBookmarks[i] =(XML_Char *) getNthExistingBookmark(i);
	PtListAddItems(mBookmarkList,(const char **)pBookmarks,getExistingBookmarksCount(),0);
	free(pBookmarks);	
	
	PtAddCallback(PtButton_cancel,Pt_CB_ACTIVATE,ph_event_cancel,this);
	PtAddCallback(PtButton_ok,Pt_CB_ACTIVATE,ph_event_ok,this);
	PtAddCallback(mSelectedBookmark,Pt_CB_ACTIVATE,ph_event_ok,this);
	PtAddCallback(mBookmarkList,Pt_CB_SELECTION,ph_listselection,this);
	PtAddCallback(windowHyperlink,Pt_CB_WINDOW_CLOSING,ph_event_close,this);

	return windowHyperlink;
}
