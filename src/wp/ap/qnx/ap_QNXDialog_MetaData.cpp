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
#include "ap_QNXDialog_MetaData.h"
#include "ut_qnxHelper.h"


/*****************************************************************/

XAP_Dialog * AP_QNXDialog_MetaData::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_MetaData * p = new AP_QNXDialog_MetaData(pFactory,id);
	return p;
}

AP_QNXDialog_MetaData::AP_QNXDialog_MetaData(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_MetaData(pDlgFactory,id)
{
}

AP_QNXDialog_MetaData::~AP_QNXDialog_MetaData(void)
{
}



int ph_event_cancel(PtWidget_t * widget,AP_QNXDialog_MetaData *dlg,PtCallbackInfo_t *cbinfo)
{
dlg->eventCancel();
return Pt_CONTINUE;
}

void AP_QNXDialog_MetaData::eventCancel()
{
setAnswer(AP_Dialog_MetaData::a_CANCEL);
done=1;
}

int ph_event_close(PtWidget_t * widget,AP_QNXDialog_MetaData *dlg,PtCallbackInfo_t *cbinfo)
{
dlg->done=1;
return Pt_CONTINUE;
}
int ph_event_ok(PtWidget_t * widget,AP_QNXDialog_MetaData *dlg,PtCallbackInfo_t *cbinfo)
{

dlg->eventOK();
return Pt_CONTINUE;
}
#define GRAB_ENTRY_TEXT(name) PtGetResource(m_entry##name,Pt_ARG_TEXT_STRING,&txt,0); \
if(txt && strlen(txt)) \
set##name(txt)

void AP_QNXDialog_MetaData::eventOK()
{
setAnswer(AP_Dialog_MetaData::a_OK);

char *txt = NULL;
  GRAB_ENTRY_TEXT(Title);
  GRAB_ENTRY_TEXT(Subject);
  GRAB_ENTRY_TEXT(Author);
  GRAB_ENTRY_TEXT(Publisher);  
  GRAB_ENTRY_TEXT(CoAuthor);
  GRAB_ENTRY_TEXT(Category);
  GRAB_ENTRY_TEXT(Keywords);
  GRAB_ENTRY_TEXT(Languages);
  GRAB_ENTRY_TEXT(Source);
  GRAB_ENTRY_TEXT(Relation);
  GRAB_ENTRY_TEXT(Coverage);
  GRAB_ENTRY_TEXT(Rights);
  GRAB_ENTRY_TEXT(Description);
  done=1;
}

void AP_QNXDialog_MetaData::runModal(XAP_Frame * pFrame)
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

PtWidget_t * AP_QNXDialog_MetaData::_constructWindow()
{
const XAP_StringSet *pSS = m_pApp->getStringSet();
PtWidget_t *mainwindow;
PtWidget_t *panelgrp;
PtWidget_t *btnOk,*btnCancel;
	  
	PtArg_t args[10];
	int n=0;

	PtSetArg(&args[n++],Pt_ARG_WINDOW_TITLE,_(AP,DLG_MetaData_Title),0);
	PtSetArg(&args[n++],Pt_ARG_WINDOW_RENDER_FLAGS,Pt_FALSE,ABI_MODAL_WINDOW_RENDER_FLAGS);
//	PtSetArg(&args[n++],Pt_ARG_WINDOW_MANAGED_FLAGS,Pt_FALSE,ABI_MODAL_WINDOW_MANAGED_FLAGS);
	PtSetArg(&args[n++],Pt_ARG_RESIZE_FLAGS,Pt_TRUE,Pt_RESIZE_XY_AS_REQUIRED);

	mainwindow = PtCreateWidget(PtWindow,NULL,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_GROUP_ORIENTATION,Pt_GROUP_VERTICAL,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,2,0);
	PtWidget_t *grp =	PtCreateWidget(PtGroup,NULL,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_RESIZE_FLAGS,Pt_TRUE,Pt_RESIZE_XY_AS_REQUIRED);
	PtSetArg(&args[n++],Pt_ARG_CONTAINER_FLAGS,Pt_TRUE,Pt_AUTO_EXTENT);
	panelgrp=PtCreateWidget( PtPanelGroup, NULL, n,args);


	n=0;
	PtSetArg(&args[n++],Pt_ARG_TITLE,_(AP,DLG_MetaData_TAB_General),0);
	PtSetArg(&args[n++],Pt_ARG_RESIZE_FLAGS,Pt_TRUE,Pt_RESIZE_XY_AS_REQUIRED);
	PtSetArg(&args[n++],Pt_ARG_CONTAINER_FLAGS,Pt_TRUE,Pt_SHOW_TITLE);
  PtCreateWidget( PtPane,panelgrp ,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_GROUP_ORIENTATION,Pt_GROUP_VERTICAL,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,5,0);
	PtSetArg(&args[n++],Pt_ARG_RESIZE_FLAGS,Pt_TRUE,Pt_RESIZE_XY_AS_REQUIRED);
	PtCreateWidget(PtGroup,Pt_DEFAULT_PARENT,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Title_LBL),0);
	PtCreateWidget( PtLabel, Pt_DEFAULT_PARENT, n,args);
	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Subject_LBL),0);
	PtCreateWidget( PtLabel, Pt_DEFAULT_PARENT, n,args);
	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Author_LBL),0);
	PtCreateWidget( PtLabel, Pt_DEFAULT_PARENT, n,args);
	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Publisher_LBL),0);
	PtCreateWidget( PtLabel, Pt_DEFAULT_PARENT, n,args);
	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_CoAuthor_LBL),0);
	PtCreateWidget( PtLabel, Pt_DEFAULT_PARENT, n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	PtWidget_t *title_entry=PtCreateWidget( PtText, Pt_DEFAULT_PARENT, n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	PtWidget_t *subject_entry=PtCreateWidget( PtText, Pt_DEFAULT_PARENT,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	PtWidget_t *author_entry = PtCreateWidget( PtText, Pt_DEFAULT_PARENT,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	PtWidget_t *publisher_entry =PtCreateWidget( PtText, Pt_DEFAULT_PARENT,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	PtWidget_t *coauthor_entry =PtCreateWidget( PtText, Pt_DEFAULT_PARENT,n,args );
	n=0;

	PtSetArg(&args[n++],Pt_ARG_TITLE,_(AP,DLG_MetaData_TAB_Summary),0);
	PtSetArg(&args[n++],Pt_ARG_RESIZE_FLAGS,Pt_TRUE,Pt_RESIZE_XY_AS_REQUIRED);
	PtSetArg(&args[n++],Pt_ARG_CONTAINER_FLAGS,Pt_TRUE,Pt_SHOW_TITLE);
	PtCreateWidget( PtPane,panelgrp,n,args );
	n=0;

	PtSetArg(&args[n++],Pt_ARG_GROUP_ORIENTATION,Pt_GROUP_VERTICAL,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,4,0);
	PtSetArg(&args[n++],Pt_ARG_RESIZE_FLAGS,Pt_TRUE,Pt_RESIZE_XY_AS_REQUIRED);
	PtCreateWidget(PtGroup,Pt_DEFAULT_PARENT,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Category_LBL),0);
	PtCreateWidget( PtLabel, Pt_DEFAULT_PARENT, n,args);
	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Keywords_LBL),0);
	PtCreateWidget( PtLabel, Pt_DEFAULT_PARENT, n,args);
	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Languages_LBL),0);
	PtCreateWidget( PtLabel, Pt_DEFAULT_PARENT, n,args);
	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Description_LBL),0);
	PtCreateWidget( PtLabel, Pt_DEFAULT_PARENT, n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	PtWidget_t *category_entry = PtCreateWidget( PtText, NULL, n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	PtWidget_t *keywords_entry = PtCreateWidget( PtText, NULL, n,args );
	n=0;

	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	PtWidget_t *languages_entry = PtCreateWidget( PtText, NULL, n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	PtSetArg(&args[n++],Pt_ARG_MULTITEXT_ROWS,5,0);
	PtWidget_t *description_txt = PtCreateWidget( PtMultiText, NULL, n,args );
	n=0;

	PtSetArg(&args[n++],Pt_ARG_TITLE,_(AP,DLG_MetaData_TAB_Permission),0);
	PtSetArg(&args[n++],Pt_ARG_RESIZE_FLAGS,Pt_TRUE,Pt_RESIZE_XY_AS_REQUIRED);
	PtSetArg(&args[n++],Pt_ARG_CONTAINER_FLAGS,Pt_TRUE,Pt_SHOW_TITLE);
	PtCreateWidget( PtPane, panelgrp, n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_GROUP_ORIENTATION,Pt_GROUP_VERTICAL,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,4,0);
	PtSetArg(&args[n++],Pt_ARG_RESIZE_FLAGS,Pt_TRUE,Pt_RESIZE_XY_AS_REQUIRED);
	PtCreateWidget(PtGroup,Pt_DEFAULT_PARENT,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Source_LBL),0);
	PtCreateWidget( PtLabel, Pt_DEFAULT_PARENT, n,args);
	n=0;	
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Relation_LBL),0);
	PtCreateWidget( PtLabel, Pt_DEFAULT_PARENT, n,args);
	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Coverage_LBL),0);
	PtCreateWidget( PtLabel, Pt_DEFAULT_PARENT, n,args);
	n=0;
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Rights_LBL),0);
	PtCreateWidget( PtLabel, Pt_DEFAULT_PARENT, n,args);
	n=0;
	
	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	PtWidget_t *source_entry = PtCreateWidget( PtText, NULL,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	PtWidget_t *relation_entry = PtCreateWidget( PtText, NULL,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	PtWidget_t * coverage_entry = PtCreateWidget( PtText, NULL,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_COLUMNS,25,0);
	PtWidget_t *rights_entry = PtCreateWidget( PtText, NULL, n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_GROUP_ORIENTATION,Pt_GROUP_HORIZONTAL,0);
	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,2,0);
	PtCreateWidget(PtGroup,grp,n,args);
	n=0;

	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(XAP,DLG_OK),0);
	btnOk = PtCreateWidget( PtButton, Pt_DEFAULT_PARENT,n,args);
	n=0;	

	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,_(XAP,DLG_Cancel),0);
	btnCancel = PtCreateWidget( PtButton, Pt_DEFAULT_PARENT,n,args);

	PtAddCallback(mainwindow,Pt_CB_WINDOW_CLOSING,ph_event_close,this);
	PtAddCallback(btnOk,Pt_CB_ACTIVATE,ph_event_ok,this);
	PtAddCallback(btnCancel,Pt_CB_ACTIVATE,ph_event_cancel,this);

  m_entryTitle = title_entry;
  m_entrySubject = subject_entry;
  m_entryAuthor = author_entry;
  m_entryPublisher = publisher_entry;
  m_entryCoAuthor = coauthor_entry;
  m_entryLanguages = languages_entry;
  m_entryKeywords = keywords_entry;
  m_entryCategory = category_entry;
  m_entrySource = source_entry;
  m_entryRelation = relation_entry;
  m_entryCoverage = coverage_entry;
  m_entryRights = rights_entry;
  m_entryDescription = description_txt;

  UT_String prop ("" );
  //Set the text on the text boxes
  // now set the text
  #define SET_ENTRY_TXT(name) \
  prop = get##name () ; \
  if ( prop.size () > 0 ) { \
    PtSetResource(m_entry##name,Pt_ARG_TEXT_STRING,prop.c_str(),0); \
  }

  SET_ENTRY_TXT(Title)
  SET_ENTRY_TXT(Subject)
  SET_ENTRY_TXT(Author)
  SET_ENTRY_TXT(Publisher)
  SET_ENTRY_TXT(CoAuthor)
  SET_ENTRY_TXT(Category)
  SET_ENTRY_TXT(Keywords)
  SET_ENTRY_TXT(Languages)
  SET_ENTRY_TXT(Source)
  SET_ENTRY_TXT(Relation)
  SET_ENTRY_TXT(Coverage)
  SET_ENTRY_TXT(Rights)
  SET_ENTRY_TXT(Description)
  #undef SET_ENTRY_TXT

return mainwindow;
}
