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
#include "ap_QNXDialog_InsertBookmark.h"
#include "ut_qnxHelper.h"


/*****************************************************************/
int ph_event_ok( PtWidget_t *widget, AP_QNXDialog_InsertBookmark * dlg, 
           PtCallbackInfo_t *info)
{
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
    return Pt_CONTINUE;
}
int ph_close(PtWidget_t *widget,AP_QNXDialog_InsertBookmark *dlg,PtCallbackInfo_t *info)
{
UT_ASSERT(widget &&dlg);
dlg->done=1;
return Pt_CONTINUE;
}
int ph_event_cancel( PtWidget_t *widget, AP_QNXDialog_InsertBookmark * dlg, 
           PtCallbackInfo_t *info)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
  return Pt_CONTINUE;
}

int ph_event_delete( PtWidget_t *widget, AP_QNXDialog_InsertBookmark * dlg, 
           PtCallbackInfo_t *info)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Delete();
    return Pt_CONTINUE;
}

XAP_Dialog * AP_QNXDialog_InsertBookmark::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_InsertBookmark * p = new AP_QNXDialog_InsertBookmark(pFactory,id);
	return p;
}

AP_QNXDialog_InsertBookmark::AP_QNXDialog_InsertBookmark(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_InsertBookmark(pDlgFactory,id)
{
}

AP_QNXDialog_InsertBookmark::~AP_QNXDialog_InsertBookmark(void)
{
}

void AP_QNXDialog_InsertBookmark::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	PtWidget_t *mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);
	connectFocus(mainWindow,pFrame);
	
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


PtWidget_t *AP_QNXDialog_InsertBookmark::_constructWindow(void)
{
PtWidget_t *PtButton_cancel;
PtWidget_t *PtButton_ok; 
PtWidget_t *MainWindow;
PtWidget_t *PtButton_delete;
int numBookmark = getExistingBookmarksCount();
XML_Char **bookmarkList=(XML_Char **)calloc(numBookmark,sizeof(XML_Char*));
int i;
const XAP_StringSet * pSS = m_pApp->getStringSet();


PtArg_t args[10];	
int n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_InsertBookmark_Title),0);
	PtSetArg(&args[n++],Pt_ARG_WINDOW_RENDER_FLAGS,Pt_FALSE,ABI_MODAL_WINDOW_RENDER_FLAGS);
//	PtSetArg(&args[n++],Pt_ARG_WINDOW_MODAL_FLAGS,Pt_FALSE,ABI_MODAL_WDINWO_MANAGE_FLAGS);
	MainWindow= PtCreateWidget(PtWindow,NULL,n,args);
	SetupContextHelp(MainWindow,this);
	PtAddHotkeyHandler(MainWindow,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);

	n=0;
	
	PtSetArg(&args[n++],Pt_ARG_GROUP_ORIENTATION,Pt_GROUP_VERTICAL,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,3,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_FLAGS,Pt_TRUE,Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	PtCreateWidget(PtGroup,Pt_DEFAULT_PARENT,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_InsertBookmark_Msg),0);
	PtCreateWidget(PtLabel,Pt_DEFAULT_PARENT,n,args);
	n=0;
	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	m_comboBox = PtCreateWidget( PtComboBox, Pt_DEFAULT_PARENT, n,args);

	//Add existing bookmarks to the widget.
	for(i=0;i<numBookmark;i++)
	{
		bookmarkList[i]=(XML_Char *)getNthExistingBookmark(i);
	}	
	PtListAddItems(m_comboBox,(const XML_Char **)bookmarkList,i,0);
	free(bookmarkList);


	n=0;
	PtSetArg(&args[n++],Pt_ARG_GROUP_ORIENTATION,Pt_GROUP_HORIZONTAL,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,3,0);
	PtCreateWidget(PtGroup,Pt_DEFAULT_PARENT,n,args);

	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(XAP,DLG_Cancel),0);
	PtButton_cancel = PtCreateWidget( PtButton, Pt_DEFAULT_PARENT, n,args );

	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(XAP,DLG_Delete),0);
	PtButton_delete = PtCreateWidget( PtButton, Pt_DEFAULT_PARENT,n,args);

	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(XAP,DLG_OK),0);
	PtButton_ok = PtCreateWidget( PtButton, Pt_DEFAULT_PARENT,n,args);

	PtAddCallback(PtButton_cancel,Pt_CB_ACTIVATE,ph_event_cancel,this);
	PtAddCallback(PtButton_delete,Pt_CB_ACTIVATE,ph_event_delete,this);
	PtAddCallback(PtButton_ok,Pt_CB_ACTIVATE,ph_event_ok,this);
	PtAddCallback(m_comboBox,Pt_CB_ACTIVATE,ph_event_ok,this);
	PtAddCallback(MainWindow,Pt_CB_WINDOW_CLOSING,ph_close,this);
	return MainWindow;

}


void AP_QNXDialog_InsertBookmark::event_OK(void)
{
	UT_ASSERT(m_comboBox);
	// get the bookmark name, if any (return cancel if no name given)	
	XML_Char *mark;
	
	PtGetResource(m_comboBox,Pt_ARG_TEXT_STRING,&mark,0);
	if(mark && *mark)
	{
		xxx_UT_DEBUGMSG(("InsertBookmark: OK pressed, first char 0x%x\n", (UT_uint32)*mark));
		setAnswer(AP_Dialog_InsertBookmark::a_OK);
		setBookmark(mark);
	}
	else
	{
		setAnswer(AP_Dialog_InsertBookmark::a_CANCEL);
	}
	done=1;
}
void AP_QNXDialog_InsertBookmark::event_Cancel(void)
{
	setAnswer(AP_Dialog_InsertBookmark::a_CANCEL);
	done=1;
}
void AP_QNXDialog_InsertBookmark::event_Delete(void)
{
	UT_ASSERT(m_comboBox);
	// get the bookmark name, if any (return cancel if no name given)	
	XML_Char *mark;
	
	PtGetResource(m_comboBox,Pt_ARG_TEXT_STRING,&mark,0);
	if(mark && *mark)
	{
		setBookmark(mark);
		setAnswer(AP_Dialog_InsertBookmark::a_DELETE);
	}
	else
	{
		setAnswer(AP_Dialog_InsertBookmark::a_CANCEL);
	}
	done=1;
}

