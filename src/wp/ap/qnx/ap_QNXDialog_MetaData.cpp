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

PtWidget_t * AP_QNXDialog_MetaData::_constructWindow()
{
const XAP_StringSet *pSS = m_pApp->getStringSet();
PtWidget_t *mainwindow;
PtWidget_t *panelgrp;
PtWidget_t *btnOk,*btnCancel;
	  
	mainwindow = abiCreatePhabDialog("ap_QNXDialog_MetaData",_(AP,DLG_MetaData_Title)); 
	SetupContextHelp(mainwindow,this);
	PtAddHotkeyHandler(mainwindow,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);

	panelgrp=abiPhabLocateWidget(mainwindow,"panelgrp");


	PtSetResource(abiPhabLocateWidget(mainwindow,"paneGeneral"),Pt_ARG_TITLE,_(AP,DLG_MetaData_TAB_General),0);

	PtSetResource(abiPhabLocateWidget(mainwindow,"lblTitle"),Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Title_LBL),0);

	PtSetResource(abiPhabLocateWidget(mainwindow,"lblSubject"),Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Subject_LBL),0);
	PtSetResource(abiPhabLocateWidget(mainwindow,"lblAuthor"),Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Author_LBL),0);
	PtSetResource(abiPhabLocateWidget(mainwindow,"lblPublisher"),Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Publisher_LBL),0);
	PtSetResource(abiPhabLocateWidget(mainwindow,"lblCoAuthor"),Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_CoAuthor_LBL),0);

	PtWidget_t *title_entry=abiPhabLocateWidget(mainwindow,"textTitle");
	PtWidget_t *subject_entry= abiPhabLocateWidget(mainwindow,"textSubject");
	PtWidget_t *author_entry =abiPhabLocateWidget(mainwindow,"textAuthor"); 
	PtWidget_t *publisher_entry =abiPhabLocateWidget(mainwindow,"textPublisher");
	PtWidget_t *coauthor_entry =abiPhabLocateWidget(mainwindow,"textCoAuthor");

	PtSetResource(abiPhabLocateWidget(mainwindow,"paneSummary"),Pt_ARG_TITLE,_(AP,DLG_MetaData_TAB_Summary),0);

	PtSetResource(abiPhabLocateWidget(mainwindow,"lblCategory"),Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Category_LBL),0);
	PtSetResource(abiPhabLocateWidget(mainwindow,"lblKeywords"),Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Keywords_LBL),0);
	PtSetResource(abiPhabLocateWidget(mainwindow,"lblLanguages"),Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Languages_LBL),0);
	PtSetResource(abiPhabLocateWidget(mainwindow,"lblDescription"),Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Description_LBL),0);

	PtWidget_t *category_entry = abiPhabLocateWidget(mainwindow,"textCategory"); 
	PtWidget_t *keywords_entry =abiPhabLocateWidget(mainwindow,"textKeywords");
	PtWidget_t *languages_entry = abiPhabLocateWidget(mainwindow,"textLanguages");
	PtWidget_t *description_txt = abiPhabLocateWidget(mainwindow,"multiDescription");

	PtSetResource(abiPhabLocateWidget(mainwindow,"panePermissions"),Pt_ARG_TITLE,_(AP,DLG_MetaData_TAB_Permission),0);

	PtSetResource(abiPhabLocateWidget(mainwindow,"lblSource"),Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Source_LBL),0);
	PtSetResource(abiPhabLocateWidget(mainwindow,"lblRelation"),Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Relation_LBL),0);
	PtSetResource(abiPhabLocateWidget(mainwindow,"lblCoverage"),Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Coverage_LBL),0);
	PtSetResource(abiPhabLocateWidget(mainwindow,"lblRights"),Pt_ARG_TEXT_STRING,_(AP,DLG_MetaData_Rights_LBL),0);
	
	PtWidget_t *source_entry = abiPhabLocateWidget(mainwindow,"textSource"); 
	PtWidget_t *relation_entry = abiPhabLocateWidget(mainwindow,"textRelation");
	PtWidget_t * coverage_entry = abiPhabLocateWidget(mainwindow,"textCoverage");
	PtWidget_t *rights_entry = abiPhabLocateWidget(mainwindow,"textRights");

	btnOk = abiPhabLocateWidget(mainwindow,"btnOK"); 
	PtSetResource(btnOk,Pt_ARG_TEXT_STRING,_(XAP,DLG_OK),0);

	btnCancel = abiPhabLocateWidget(mainwindow,"btnCancel"); 
	PtSetResource(btnCancel,Pt_ARG_TEXT_STRING,_(XAP,DLG_Cancel),0);

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
