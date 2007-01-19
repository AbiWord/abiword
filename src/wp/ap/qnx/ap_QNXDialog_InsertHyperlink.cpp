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
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

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
		setHyperlink((gchar*)res);
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
  XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

	PtSetParentWidget(parentWindow); 
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


	windowHyperlink= abiCreatePhabDialog("ap_QNXDialog_InsertHyperlink",pSS,AP_STRING_ID_DLG_InsertHyperlink_Title); 
	SetupContextHelp(windowHyperlink,this);
	PtAddHotkeyHandler(windowHyperlink,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);

	localizeLabel(abiPhabLocateWidget(windowHyperlink,"lblMsg"),pSS,AP_STRING_ID_DLG_InsertHyperlink_Msg);

	mSelectedBookmark=abiPhabLocateWidget(windowHyperlink,"txtName");

	mBookmarkList = abiPhabLocateWidget(windowHyperlink,"listHyperlink");

	PtButton_cancel= abiPhabLocateWidget(windowHyperlink,"btnCancel");
	localizeLabel(PtButton_cancel,pSS,XAP_STRING_ID_DLG_Cancel);

	PtButton_ok = abiPhabLocateWidget(windowHyperlink,"btnOK");
	localizeLabel(PtButton_ok,pSS,XAP_STRING_ID_DLG_OK);

	//Add existing bookmarks to the list.
 gchar ** pBookmarks = (gchar **)UT_calloc(getExistingBookmarksCount(),sizeof(gchar*));
	if(getHyperlink())
	{
	int start,end;
	start=0;	
	end=1000;
		PtSetResource(mSelectedBookmark,Pt_ARG_TEXT_STRING,getHyperlink(),0);
		PtTextSetSelection(mSelectedBookmark,&start,&end);
	}
	
  for (int i = 0; i < (int)getExistingBookmarksCount(); i++)
   pBookmarks[i] =(gchar *) getNthExistingBookmark(i);
	PtListAddItems(mBookmarkList,(const char **)pBookmarks,getExistingBookmarksCount(),0);
	g_free(pBookmarks);	
	
	PtAddCallback(PtButton_cancel,Pt_CB_ACTIVATE,ph_event_cancel,this);
	PtAddCallback(PtButton_ok,Pt_CB_ACTIVATE,ph_event_ok,this);
	PtAddCallback(mSelectedBookmark,Pt_CB_ACTIVATE,ph_event_ok,this);
	PtAddCallback(mBookmarkList,Pt_CB_SELECTION,ph_listselection,this);
	PtAddCallback(windowHyperlink,Pt_CB_WINDOW_CLOSING,ph_event_close,this);

	return windowHyperlink;
}
