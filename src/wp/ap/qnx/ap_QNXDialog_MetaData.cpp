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
	  
	static const PhDim_t dim = {361,317};
	static const PtArg_t args[] = {
     		Pt_ARG(Pt_ARG_DIM,&dim,0),
		Pt_ARG(Pt_ARG_WINDOW_TITLE,pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Title).c_str(),0),
		Pt_ARG(Pt_ARG_WINDOW_RENDER_FLAGS,0,ABI_MODAL_WINDOW_RENDER_FLAGS),
		Pt_ARG(Pt_ARG_WINDOW_MANAGED_FLAGS,0,ABI_MODAL_WINDOW_MANAGE_FLAGS)
	};

	static const PhArea_t area1 = { { 3, 2 }, { 355, 283 } };
	static const PtArg_t args1[] = {
		Pt_ARG( Pt_ARG_AREA, &area1, 0 ),
		};

	static const PhArea_t area2 = { { 0, 0 }, { 345, 247 } };
	static const PtArg_t args2[] = {
		Pt_ARG( Pt_ARG_AREA, &area2, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BEVEL_WIDTH, 1, 0 ),
Pt_ARG( Pt_ARG_TITLE, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_TAB_General).c_str(), 0 ),
		};

	static const PhArea_t area3 = { { 92, 6 }, { 247, 27 } };
	static const PtArg_t args3[] = {
		Pt_ARG( Pt_ARG_AREA, &area3, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		};

	static const PhArea_t area4 = { { 7, 9 }, { 61, 21 } };
	static const PtArg_t args4[] = {
		Pt_ARG( Pt_ARG_AREA, &area4, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Title_LBL).c_str(), 0 ),
		};

	static const PhArea_t area5 = { { 92, 38 }, { 247, 27 } };
	static const PtArg_t args5[] = {
		Pt_ARG( Pt_ARG_AREA, &area5, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		};

	static const PhArea_t area6 = { { 7, 41 }, { 61, 21 } };
	static const PtArg_t args6[] = {
		Pt_ARG( Pt_ARG_AREA, &area6, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Subject_LBL).c_str(), 0 ),
		};

	static const PhArea_t area7 = { { 92, 70 }, { 247, 27 } };
	static const PtArg_t args7[] = {
		Pt_ARG( Pt_ARG_AREA, &area7, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		};

	static const PhArea_t area8 = { { 7, 73 }, { 61, 21 } };
	static const PtArg_t args8[] = {
		Pt_ARG( Pt_ARG_AREA, &area8, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Author_LBL).c_str(), 0 ),
		};

	static const PhArea_t area9 = { { 92, 102 }, { 247, 27 } };
	static const PtArg_t args9[] = {
		Pt_ARG( Pt_ARG_AREA, &area9, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		};

	static const PhArea_t area10 = { { 7, 105 }, { 62, 21 } };
	static const PtArg_t args10[] = {
		Pt_ARG( Pt_ARG_AREA, &area10, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Publisher_LBL).c_str(), 0 ),
		};

	static const PhArea_t area11 = { { 92, 134 }, { 247, 27 } };
	static const PtArg_t args11[] = {
		Pt_ARG( Pt_ARG_AREA, &area11, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		};

	static const PhArea_t area12 = { { 7, 137 }, { 75, 21 } };
	static const PtArg_t args12[] = {
		Pt_ARG( Pt_ARG_AREA, &area12, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_CoAuthor_LBL).c_str() , 0 ),
		};

	static const PhArea_t area13 = { { -32768, -32768 }, { 345, 247 } };
	static const PtArg_t args13[] = {
		Pt_ARG( Pt_ARG_AREA, &area13, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BEVEL_WIDTH, 1, 0 ),
Pt_ARG( Pt_ARG_TITLE, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_TAB_Summary).c_str(), 0 ),
		};

	static const PhArea_t area14 = { { 5, 9 }, { 61, 21 } };
	static const PtArg_t args14[] = {
		Pt_ARG( Pt_ARG_AREA, &area14, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Category_LBL).c_str(), 0 ),
		};

	static const PhArea_t area15 = { { 93, 6 }, { 247, 27 } };
	static const PtArg_t args15[] = {
		Pt_ARG( Pt_ARG_AREA, &area15, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		};

	static const PhArea_t area16 = { { 93, 38 }, { 247, 27 } };
	static const PtArg_t args16[] = {
		Pt_ARG( Pt_ARG_AREA, &area16, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		};

	static const PhArea_t area17 = { { 5, 41 }, { 70, 21 } };
	static const PtArg_t args17[] = {
		Pt_ARG( Pt_ARG_AREA, &area17, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Keywords_LBL).c_str(), 0 ),
		};

	static const PhArea_t area18 = { { 93, 70 }, { 247, 27 } };
	static const PtArg_t args18[] = {
		Pt_ARG( Pt_ARG_AREA, &area18, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		};

	static const PhArea_t area19 = { { 5, 73 }, { 91, 21 } };
	static const PtArg_t args19[] = {
		Pt_ARG( Pt_ARG_AREA, &area19, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Languages_LBL).c_str(), 0 ),
		};

	static const PhArea_t area20 = { { 5, 163 }, { 74, 22 } };
	static const PtArg_t args20[] = {
		Pt_ARG( Pt_ARG_AREA, &area20, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Description_LBL).c_str(), 0 ),
		};

	static const PhArea_t area21 = { { 92, 121 }, { 248, 116 } };
	static const PtArg_t args21[] = {
		Pt_ARG( Pt_ARG_AREA, &area21, 0 ),
		Pt_ARG(Pt_ARG_TEXT_STRING,"",0),
		};

	static const PhArea_t area22 = { { -32768, -32768 }, { 345, 247 } };
	static const PtArg_t args22[] = {
		Pt_ARG( Pt_ARG_AREA, &area22, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BEVEL_WIDTH, 1, 0 ),
Pt_ARG( Pt_ARG_TITLE, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_TAB_Permission).c_str(), 0 ),
		};

	static const PhArea_t area23 = { { 93, 5 }, { 247, 27 } };
	static const PtArg_t args23[] = {
		Pt_ARG( Pt_ARG_AREA, &area23, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		};

	static const PhArea_t area24 = { { 7, 8 }, { 61, 21 } };
	static const PtArg_t args24[] = {
		Pt_ARG( Pt_ARG_AREA, &area24, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Source_LBL).c_str(), 0 ),
		};

	static const PhArea_t area25 = { { 93, 42 }, { 247, 27 } };
	static const PtArg_t args25[] = {
		Pt_ARG( Pt_ARG_AREA, &area25, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		};

	static const PhArea_t area26 = { { 7, 45 }, { 61, 21 } };
	static const PtArg_t args26[] = {
		Pt_ARG( Pt_ARG_AREA, &area26, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Relation_LBL).c_str(), 0 ),
		};

	static const PhArea_t area27 = { { 93, 74 }, { 247, 27 } };
	static const PtArg_t args27[] = {
		Pt_ARG( Pt_ARG_AREA, &area27, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		};

	static const PhArea_t area28 = { { 7, 77 }, { 65, 21 } };
	static const PtArg_t args28[] = {
		Pt_ARG( Pt_ARG_AREA, &area28, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Coverage_LBL).c_str(), 0 ),
		};

	static const PhArea_t area29 = { { 93, 106 }, { 247, 27 } };
	static const PtArg_t args29[] = {
		Pt_ARG( Pt_ARG_AREA, &area29, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "", 0 ),
		};

	static const PhArea_t area30 = { { 7, 109 }, { 61, 21 } };
	static const PtArg_t args30[] = {
		Pt_ARG( Pt_ARG_AREA, &area30, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_MetaData_Rights_LBL).c_str(), 0 ),
		};

	static const PhArea_t area31 = { { 288, 290 }, { 70, 27 } };
	static const PtArg_t args31[] = {
		Pt_ARG( Pt_ARG_AREA, &area31, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(XAP_STRING_ID_DLG_OK).c_str(), 0 ),
		};

	static const PhArea_t area32 = { { 214, 290 }, { 70, 27 } };
	static const PtArg_t args32[] = {
		Pt_ARG( Pt_ARG_AREA, &area32, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel).c_str() , 0 ),
		};

	mainwindow = PtCreateWidget(PtWindow,NULL,sizeof(args) / sizeof(PtArg_t),args);
	panelgrp=PtCreateWidget( PtPanelGroup, NULL, sizeof(args1) / sizeof(PtArg_t), args1 );

      PtCreateWidget( PtPane,panelgrp , sizeof(args2) / sizeof(PtArg_t), args2 );

PtWidget_t *title_entry=PtCreateWidget( PtText, NULL, sizeof(args3) / sizeof(PtArg_t), args3 );

	PtCreateWidget( PtLabel, NULL, sizeof(args4) / sizeof(PtArg_t), args4 );

PtWidget_t *subject_entry=PtCreateWidget( PtText, NULL, sizeof(args5) / sizeof(PtArg_t), args5 );

	PtCreateWidget( PtLabel, NULL, sizeof(args6) / sizeof(PtArg_t), args6 );

PtWidget_t *author_entry = PtCreateWidget( PtText, NULL, sizeof(args7) / sizeof(PtArg_t), args7 );

	PtCreateWidget( PtLabel, NULL, sizeof(args8) / sizeof(PtArg_t), args8 );

PtWidget_t * publisher_entry =PtCreateWidget( PtText, NULL, sizeof(args9) / sizeof(PtArg_t), args9 );

	PtCreateWidget( PtLabel, NULL, sizeof(args10) / sizeof(PtArg_t), args10 );

PtWidget_t *coauthor_entry =PtCreateWidget( PtText, NULL, sizeof(args11) / sizeof(PtArg_t), args11 );

	PtCreateWidget( PtLabel, NULL, sizeof(args12) / sizeof(PtArg_t), args12 );

	PtCreateWidget( PtPane,panelgrp, sizeof(args13) / sizeof(PtArg_t), args13 );

	PtCreateWidget( PtLabel, NULL, sizeof(args14) / sizeof(PtArg_t), args14 );

PtWidget_t *category_entry = PtCreateWidget( PtText, NULL, sizeof(args15) / sizeof(PtArg_t), args15 );

PtWidget_t *keywords_entry = PtCreateWidget( PtText, NULL, sizeof(args16) / sizeof(PtArg_t), args16 );

	PtCreateWidget( PtLabel, NULL, sizeof(args17) / sizeof(PtArg_t), args17 );

PtWidget_t *languages_entry = PtCreateWidget( PtText, NULL, sizeof(args18) / sizeof(PtArg_t), args18 );

	PtCreateWidget( PtLabel, NULL, sizeof(args19) / sizeof(PtArg_t), args19 );

	PtCreateWidget( PtLabel, NULL, sizeof(args20) / sizeof(PtArg_t), args20 );

PtWidget_t *description_txt = PtCreateWidget( PtMultiText, NULL, sizeof(args21) / sizeof(PtArg_t), args21 );

	PtCreateWidget( PtPane, panelgrp, sizeof(args22) / sizeof(PtArg_t), args22 );

PtWidget_t *source_entry = PtCreateWidget( PtText, NULL, sizeof(args23) / sizeof(PtArg_t), args23 );

	PtCreateWidget( PtLabel, NULL, sizeof(args24) / sizeof(PtArg_t), args24 );

PtWidget_t *relation_entry = PtCreateWidget( PtText, NULL, sizeof(args25) / sizeof(PtArg_t), args25 );

	PtCreateWidget( PtLabel, NULL, sizeof(args26) / sizeof(PtArg_t), args26 );

PtWidget_t * coverage_entry = PtCreateWidget( PtText, NULL, sizeof(args27) / sizeof(PtArg_t), args27 );

	PtCreateWidget( PtLabel, NULL, sizeof(args28) / sizeof(PtArg_t), args28 );

PtWidget_t *rights_entry = PtCreateWidget( PtText, NULL, sizeof(args29) / sizeof(PtArg_t), args29 );

	PtCreateWidget( PtLabel, NULL, sizeof(args30) / sizeof(PtArg_t), args30 );

	btnOk = PtCreateWidget( PtButton, mainwindow, sizeof(args31) / sizeof(PtArg_t), args31 );

	btnCancel = PtCreateWidget( PtButton, mainwindow, sizeof(args32) / sizeof(PtArg_t), args32 );

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
