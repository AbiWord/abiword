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


PtWidget_t *AP_QNXDialog_InsertBookmark::_constructWindow(void)
{
PtWidget_t *PtLabel_msg;
PtWidget_t *PtButton_cancel;
PtWidget_t *PtButton_ok; 
PtWidget_t *MainWindow;
PtWidget_t *PtButton_delete;
int numBookmark = getExistingBookmarksCount();
XML_Char **bookmarkList=(XML_Char **)calloc(numBookmark,sizeof(XML_Char*));
int i;

const XAP_StringSet * pSS = m_pApp->getStringSet();


	 //Main Window
	 PhDim_t dim = { 450,100};
		PtArg_t args[] = {
		Pt_ARG(Pt_ARG_DIM,&dim,0),
		Pt_ARG(Pt_ARG_WINDOW_TITLE,pSS->getValueUTF8(AP_STRING_ID_DLG_InsertBookmark_Title).c_str(),0),
		Pt_ARG(Pt_ARG_WINDOW_RENDER_FLAGS,0,ABI_MODAL_WINDOW_RENDER_FLAGS),
		Pt_ARG(Pt_ARG_WINDOW_MANAGED_FLAGS,0,ABI_MODAL_WINDOW_MANAGE_FLAGS)
		};

	 //PtPane.
	 PhArea_t area1 = { { 0, 0 }, { 450, 70 } };
	 PtArg_t args1[] = {
		Pt_ARG( Pt_ARG_AREA, &area1, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BEVEL_WIDTH, 1, 0 ),
		};
	// Combo Box
	 PhArea_t area2 = { { 5, 35 }, { 440, 27 } };
	 PtArg_t args2[] = {
		Pt_ARG( Pt_ARG_AREA, &area2, 0 ),
		};
	// Dialog MSG.
	 PhArea_t area3 = { { 5, 0 }, { 450, 20 } };
	 PtArg_t args3[] = {
		Pt_ARG( Pt_ARG_AREA, &area3, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_InsertBookmark_Msg).c_str() , 0 ),
		};

		// Bottom most PtPane (where OK and cancel button are..)
	 PhArea_t area4 = { { 0, 69 }, { 450, 30 } };
	 PtArg_t args4[] = {
		Pt_ARG( Pt_ARG_AREA, &area4, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 258,1334445470 ),
		Pt_ARG( Pt_ARG_BEVEL_WIDTH, 1, 0 ),
		Pt_ARG( Pt_ARG_FILL_COLOR, 0xc0c0c0, 0 ),
		Pt_ARG( Pt_ARG_BASIC_FLAGS, 67056,4194303 ),
		};
		//Cancel button.
	 PhArea_t area5 = { { 285, 0 }, { 50, 27 } };
	 PtArg_t args5[] = {
		Pt_ARG( Pt_ARG_AREA, &area5, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel).c_str(), 0 ),
		};

	//Delete button
	PhArea_t area6 = { { 340, 0 }, { 50, 27 } };
	PtArg_t args6[] = {
		Pt_ARG(Pt_ARG_AREA,&area6,0),
		Pt_ARG(Pt_ARG_TEXT_STRING,pSS->getValueUTF8(XAP_STRING_ID_DLG_Delete).c_str(),0)
		};
		//OK Button.
	 PhArea_t area7 = { { 395, 0 }, { 50, 27 } };
	 PtArg_t args7[] = {
		Pt_ARG( Pt_ARG_AREA, &area7, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValueUTF8(XAP_STRING_ID_DLG_OK).c_str(), 0 ),
		};
	

	MainWindow= PtCreateWidget(PtWindow,NULL,sizeof(args) / sizeof(PtArg_t),args);
	
	PtCreateWidget( PtPane, NULL, sizeof(args1) / sizeof(PtArg_t), args1 );

	m_comboBox = PtCreateWidget( PtComboBox, NULL, sizeof(args2) / sizeof(PtArg_t), args2 );

	//Add existing bookmarks to the widget.
	for(i=0;i<numBookmark;i++)
	{
	bookmarkList[i]=(XML_Char *)getNthExistingBookmark(i);
	}	
	PtListAddItems(m_comboBox,(const XML_Char **)bookmarkList,i,0);
	free(bookmarkList);

	PtLabel_msg = PtCreateWidget( PtLabel, NULL, sizeof(args3) / sizeof(PtArg_t), args3 );

	PtCreateWidget( PtPane, MainWindow, sizeof(args4) / sizeof(PtArg_t), args4 );

	PtButton_cancel = PtCreateWidget( PtButton, NULL, sizeof(args5) / sizeof(PtArg_t), args5 );
	PtButton_delete = PtCreateWidget( PtButton, NULL,sizeof(args6) / sizeof(PtArg_t),args6);
	PtButton_ok = PtCreateWidget( PtButton, NULL, sizeof(args7) / sizeof(PtArg_t), args7 );

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

