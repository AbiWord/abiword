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


	int n=0;
	PtArg_t args[10];
	
	PtSetArg(&args[n++],Pt_ARG_WINDOW_TITLE,_(AP,DLG_InsertHyperlink_Title),0);
	PtSetArg(&args[n++],Pt_ARG_WINDOW_RENDER_FLAGS,Pt_FALSE,ABI_MODAL_WINDOW_RENDER_FLAGS);
	windowHyperlink= PtCreateWidget(PtWindow,NULL,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,4,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_ORIENTATION,Pt_GROUP_VERTICAL,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_FLAGS,Pt_TRUE,Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	PtCreateWidget(PtGroup,Pt_DEFAULT_PARENT,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_InsertHyperlink_Msg),0);
	PtCreateWidget(PtLabel,Pt_DEFAULT_PARENT,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	mSelectedBookmark=PtCreateWidget( PtText, Pt_DEFAULT_PARENT, n, args );
	n=0;

	PtSetArg(&args[n++],Pt_ARG_HEIGHT,50,0);
	mBookmarkList=PtCreateWidget( PtList,Pt_DEFAULT_PARENT,n,args );
	n=0;

	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,2,0);
	PtCreateWidget(PtGroup,Pt_DEFAULT_PARENT,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(XAP,DLG_Cancel),0);
	PtButton_cancel=PtCreateWidget( PtButton, Pt_DEFAULT_PARENT,n,args);

	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(XAP,DLG_OK),0);
	PtButton_ok=PtCreateWidget( PtButton, Pt_DEFAULT_PARENT,n,args);

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
