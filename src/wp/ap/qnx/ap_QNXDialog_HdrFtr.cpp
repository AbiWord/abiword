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
#include <Ph.h>
#include <Pt.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_HdrFtr.h"
#include "ap_QNXDialog_HdrFtr.h"
#include "ut_qnxHelper.h"

/*****************************************************************/

int ph_event_ok( PtWidget_t *widget, AP_QNXDialog_HdrFtr * dlg, 
           PtCallbackInfo_t *info)
{
UT_ASSERT(widget && dlg);
dlg->event_OK();
return Pt_CONTINUE;
}

void AP_QNXDialog_HdrFtr::event_OK(void)
{
setAnswer(AP_Dialog_HdrFtr::a_OK);
done=1;
}

int ph_event_cancel( PtWidget_t *widget,AP_QNXDialog_HdrFtr *dlg, PtCallbackInfo_t *info)
{
UT_ASSERT(widget && dlg);
dlg->event_Cancel();
return Pt_CONTINUE;
}

void AP_QNXDialog_HdrFtr::event_Cancel(void)
{

setAnswer(AP_Dialog_HdrFtr::a_CANCEL);
done=1;
}

int ph_event_close( PtWidget_t *widget, AP_QNXDialog_HdrFtr * dlg, PtCallbackInfo_t *info)
{
UT_ASSERT(widget && dlg);
dlg->done=1;
return Pt_CONTINUE;
}

int ph_event_activate_updown(PtWidget_t *widget,AP_QNXDialog_HdrFtr *dlg,PtCallbackInfo_t *info)
{
dlg->UpDown(info->reason_subtype);
return Pt_CONTINUE;
}
void AP_QNXDialog_HdrFtr::UpDown(int updown)
{
char *textstr;
char strnew[10];
int number;

if(updown==Pt_UPDOWN_UP)
{
	PtGetResource(m_RestartText,Pt_ARG_TEXT_STRING,&textstr,0);
	number=atoi(textstr);
	number++;
	snprintf((char*)&strnew,10,"%d",number);
	PtSetResource(m_RestartText,Pt_ARG_TEXT_STRING,&strnew,0);
}else
{
	PtGetResource(m_RestartText,Pt_ARG_TEXT_STRING,&textstr,0);
	number=atoi(textstr);
	number--;
	snprintf((char*)&strnew,10,"%d",number);
	PtSetResource(m_RestartText,Pt_ARG_TEXT_STRING,&strnew,0);
}
}
int ph_activate_restart( PtWidget_t *widget, AP_QNXDialog_HdrFtr * dlg, PtCallbackInfo_t *info)
{
UT_ASSERT(widget && dlg);
dlg->RestartChanged();
return Pt_CONTINUE;
}

int ph_HdrFirst( PtWidget_t *widget,AP_QNXDialog_HdrFtr * dlg, PtCallbackInfo_t *info)
{
UT_ASSERT(dlg);
dlg->CheckChanged(AP_Dialog_HdrFtr::HdrFirst);
return Pt_CONTINUE;
}
int ph_HdrLast( PtWidget_t *widget,AP_QNXDialog_HdrFtr * dlg, PtCallbackInfo_t *info)
{
UT_ASSERT(dlg);
dlg->CheckChanged(AP_Dialog_HdrFtr::HdrLast);
return Pt_CONTINUE;
}
int ph_HdrEven( PtWidget_t *widget,AP_QNXDialog_HdrFtr * dlg, PtCallbackInfo_t *info)
{
UT_ASSERT(dlg);
dlg->CheckChanged(AP_Dialog_HdrFtr::HdrEven);
return Pt_CONTINUE;
}
int ph_FtrFirst( PtWidget_t *widget,AP_QNXDialog_HdrFtr * dlg, PtCallbackInfo_t *info)
{
UT_ASSERT(dlg);
dlg->CheckChanged(AP_Dialog_HdrFtr::FtrFirst);
return Pt_CONTINUE;
}
int ph_FtrLast( PtWidget_t *widget,AP_QNXDialog_HdrFtr * dlg, PtCallbackInfo_t *info)
{
UT_ASSERT(dlg);
dlg->CheckChanged(AP_Dialog_HdrFtr::FtrLast);
return Pt_CONTINUE;
}
int ph_FtrEven( PtWidget_t *widget,AP_QNXDialog_HdrFtr * dlg, PtCallbackInfo_t *info)
{
UT_ASSERT(dlg);
dlg->CheckChanged(AP_Dialog_HdrFtr::FtrEven);
return Pt_CONTINUE;
}

XAP_Dialog * AP_QNXDialog_HdrFtr::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_QNXDialog_HdrFtr * p = new AP_QNXDialog_HdrFtr(pFactory,id);
	return p;
}

AP_QNXDialog_HdrFtr::AP_QNXDialog_HdrFtr(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_HdrFtr(pDlgFactory,id)
{
}

AP_QNXDialog_HdrFtr::~AP_QNXDialog_HdrFtr(void)
{
}

void AP_QNXDialog_HdrFtr::runModal(XAP_Frame * pFrame)
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

PtWidget_t * AP_QNXDialog_HdrFtr::_constructWindow(void)
{
const XAP_StringSet *pSS = m_pApp->getStringSet();
PtWidget_t *mainWindow;
PtWidget_t *PtButton_ok;
PtWidget_t *PtButton_cancel;
PtWidget_t *toggleHeadEven;
PtWidget_t *toggleHeadFirst;
PtWidget_t *toggleHeadLast;
PtWidget_t *toggleFootEven;
PtWidget_t *toggleFootFirst;
PtWidget_t *toggleFootLast;
PtWidget_t *toggleRestartNewSection;
PtWidget_t *textRestartNumber;
PtWidget_t *UpDown;
PtWidget_t *Pane1;
PtWidget_t *RestartLabel;

	/* The window itself */
	PhDim_t dim = { 496,355 };
	PtArg_t args[] = {
		Pt_ARG(Pt_ARG_DIM,&dim,0),
		Pt_ARG(Pt_ARG_WINDOW_TITLE,pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_Title).c_str(),0),
		Pt_ARG(Pt_ARG_WINDOW_RENDER_FLAGS,0,ABI_MODAL_WINDOW_RENDER_FLAGS),
		Pt_ARG(Pt_ARG_WINDOW_MANAGED_FLAGS,0,ABI_MODAL_WINDOW_MANAGE_FLAGS)
		};

	 /* Top PtPane, lower */ 
	 PhArea_t area1 = { { 0, 0 }, { 496, 322 } };
	 PtArg_t args1[] = {
		Pt_ARG( Pt_ARG_AREA, &area1, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_ANCHOR_FLAGS, 1440,8191 ),
		};
/* 'Header' PtPane */
	 PhArea_t area2 = { { 3, 23 }, { 486, 95 } };
	 PtArg_t args2[] = {
		Pt_ARG( Pt_ARG_AREA, &area2, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BEVEL_WIDTH, 1, 0 ),
		};
	/* PtToggle , diff head even page */
	 PhArea_t area3 = { { 5, 5 }, { 466, 24 } };
	 PtArg_t args3[] = {
		Pt_ARG( Pt_ARG_AREA, &area3, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_HeaderEven).c_str(), 0 ),
		};
		/* PtToggle, diff head first page */
	 PhArea_t area4 = { { 5, 31 }, { 469, 24 } };
	 PtArg_t args4[] = {
		Pt_ARG( Pt_ARG_AREA, &area4, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_HeaderFirst).c_str(), 0 ),
		};
	 /* PtToggle diff head last page */
	 PhArea_t area5 = { { 5, 58 }, { 469, 24 } };
	 PtArg_t args5[] = {
		Pt_ARG( Pt_ARG_AREA, &area5, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_HeaderLast).c_str() , 0 ),
		};
	/* Head label  */
	 PhArea_t area6 = { { 5, 0 }, { 206, 21 } };
	 PtArg_t args6[] = {
		Pt_ARG( Pt_ARG_AREA, &area6, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_HeaderFrame).c_str() , 0 ),
		};
		/* Footer label */
	 PhArea_t area7 = { { 5, 133 }, { 308, 21 } };
	 PtArg_t args7[] = {
		Pt_ARG( Pt_ARG_AREA, &area7, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_FooterFrame).c_str() , 0 ),
		};
 	/*PtPane  Footer*/
	 PhArea_t area8 = { { 3, 159 }, { 486, 95 } };
	 PtArg_t args8[] = {
		Pt_ARG( Pt_ARG_AREA, &area8, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BEVEL_WIDTH, 1, 0 ),
		};
	/*PtToggle Diff foot first page */
	 PhArea_t area9 = { { 5, 31 }, { 472, 24 } };
	 PtArg_t args9[] = {
		Pt_ARG( Pt_ARG_AREA, &area9, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_FooterFirst).c_str(), 0 ),
		};
	 /*PtToggle Diff foor last page */
	 PhArea_t area10 = { { 5, 58 }, { 469, 24 } };
	 PtArg_t args10[] = {
		Pt_ARG( Pt_ARG_AREA, &area10, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_FooterLast).c_str(), 0 ),
		};
	/*PtToggle, diff foot on even page */
	 PhArea_t area11 = { { 5, 5 }, { 472, 24 } };
	 PtArg_t args11[] = {
		Pt_ARG( Pt_ARG_AREA, &area11, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_FooterEven).c_str(), 0 ),
		};
	 /* PtToggle, restart page number on new section */
	 PhArea_t area12 = { { 10, 282 }, { 268, 24 } };
	 PtArg_t args12[] = {
		Pt_ARG( Pt_ARG_AREA, &area12, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_RestartCheck).c_str(), 0 ),
		};
		/* PtLabel restart numbering at.. */
	 PhArea_t area13 = { { 288, 285 }, { 139, 21 } };
	 PtArg_t args13[] = {
		Pt_ARG( Pt_ARG_AREA, &area13, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValueUTF8(AP_STRING_ID_DLG_HdrFtr_RestartNumbers).c_str() , 0 ),
		};
		/*PtText, where to restart numbering.  */
	 PhArea_t area14 = { { 432, 280 }, { 37, 27 } };
	 PtArg_t args14[] = {
		Pt_ARG( Pt_ARG_AREA, &area14, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING,"0",0),
		Pt_ARG( Pt_ARG_EDIT_MASK, "####", 0 ),
		};
		/*PtUpDown */
	 PhArea_t area15 = { { 468, 280 }, { 18, 27 } };
	 PtArg_t args15[] = {
		Pt_ARG( Pt_ARG_AREA, &area15, 0 ),
		};
	/*PtPane for OK buttons. */
	 PhArea_t area16 = { { 0, 321 }, { 496, 33 } };
	 PtArg_t args16[] = {
		Pt_ARG( Pt_ARG_AREA, &area16, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 258,1334445470 ),
		Pt_ARG( Pt_ARG_FILL_COLOR, 0xc0c0c0, 0 ),
		Pt_ARG( Pt_ARG_BASIC_FLAGS, 67056,4194303 ),
		};
		/* PtButton Cancel */
	 PhArea_t area17 = { { 320, 2 }, { 82, 27 } };
	 PtArg_t args17[] = {
		Pt_ARG( Pt_ARG_AREA, &area17, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING,pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel).c_str() , 0 ),
		Pt_ARG( Pt_ARG_ANCHOR_FLAGS, 48,8191 ),
		};
		/* PtButton OK*/
	 PhArea_t area18 = { { 408, 2 }, { 82, 27 } };
	 PtArg_t args18[] = {
		Pt_ARG( Pt_ARG_AREA, &area18, 0 ),
Pt_ARG( Pt_ARG_TEXT_STRING, pSS->getValueUTF8(XAP_STRING_ID_DLG_OK).c_str(), 0 ),
		};

	mainWindow=PtCreateWidget( PtWindow,NULL,sizeof(args) / sizeof(PtArg_t),args);
	Pane1=PtCreateWidget( PtPane, NULL, sizeof(args1) / sizeof(PtArg_t), args1 );
		PtCreateWidget( PtPane, NULL, sizeof(args2) / sizeof(PtArg_t), args2 );
	toggleHeadEven=PtCreateWidget( PtToggleButton, NULL, sizeof(args3) / sizeof(PtArg_t), args3 );
	toggleHeadFirst=PtCreateWidget( PtToggleButton, NULL, sizeof(args4) / sizeof(PtArg_t), args4 );
	toggleHeadLast=PtCreateWidget( PtToggleButton, NULL, sizeof(args5) / sizeof(PtArg_t), args5 );
		PtCreateWidget( PtLabel, Pane1, sizeof(args6) / sizeof(PtArg_t), args6 );
		PtCreateWidget( PtLabel,Pane1, sizeof(args7) / sizeof(PtArg_t), args7 );
		PtCreateWidget( PtPane, Pane1, sizeof(args8) / sizeof(PtArg_t), args8 );
	toggleFootFirst=PtCreateWidget( PtToggleButton, NULL, sizeof(args9) / sizeof(PtArg_t), args9 );
	toggleFootLast=PtCreateWidget( PtToggleButton, NULL, sizeof(args10) / sizeof(PtArg_t), args10 );
	toggleFootEven=PtCreateWidget( PtToggleButton, NULL, sizeof(args11) / sizeof(PtArg_t), args11 );
	toggleRestartNewSection=PtCreateWidget( PtToggleButton, Pane1, sizeof(args12) / sizeof(PtArg_t), args12 );
	RestartLabel=PtCreateWidget( PtLabel, Pane1, sizeof(args13) / sizeof(PtArg_t), args13 );
	textRestartNumber=PtCreateWidget( PtText, Pane1, sizeof(args14) / sizeof(PtArg_t), args14 );
	UpDown=PtCreateWidget( PtUpDown, Pane1, sizeof(args15) / sizeof(PtArg_t), args15 );
		PtCreateWidget( PtPane, mainWindow, sizeof(args16) / sizeof(PtArg_t), args16 );
	PtButton_cancel=PtCreateWidget( PtButton, NULL, sizeof(args17) / sizeof(PtArg_t), args17 );
	PtButton_ok=PtCreateWidget( PtButton, NULL, sizeof(args18) / sizeof(PtArg_t), args18 );

	
	PtAddCallback(PtButton_cancel,Pt_CB_ACTIVATE,ph_event_cancel,this);
	PtAddCallback(PtButton_ok,Pt_CB_ACTIVATE,ph_event_ok,this);
	PtAddCallback(mainWindow,Pt_CB_WINDOW_CLOSING,ph_event_close,this);
	PtAddCallback(toggleRestartNewSection,Pt_CB_ACTIVATE,ph_activate_restart,this);
	PtAddCallback(toggleHeadEven,Pt_CB_ACTIVATE,ph_HdrEven,this);
	PtAddCallback(toggleHeadFirst,Pt_CB_ACTIVATE,ph_HdrFirst,this);
	PtAddCallback(toggleHeadLast,Pt_CB_ACTIVATE,ph_HdrLast,this);
	PtAddCallback(toggleFootEven,Pt_CB_ACTIVATE,ph_FtrEven,this);
	PtAddCallback(toggleFootFirst,Pt_CB_ACTIVATE,ph_FtrFirst,this);
	PtAddCallback(toggleFootLast,Pt_CB_ACTIVATE,ph_FtrLast,this);
	PtAddCallback(UpDown,Pt_CB_ACTIVATE,ph_event_activate_updown,this);

	m_wHdrFtr[HdrEven]= toggleHeadEven;
	m_wHdrFtr[HdrFirst]= toggleHeadFirst;
	m_wHdrFtr[HdrLast]= toggleHeadLast;
	m_wHdrFtr[FtrEven]= toggleFootEven;
	m_wHdrFtr[FtrFirst]= toggleFootFirst;
	m_wHdrFtr[FtrLast]= toggleFootLast;
	m_RestartLabel=RestartLabel;
	m_RestartUpDown= UpDown;
	m_RestartText=textRestartNumber;
	m_RestartToggle=toggleRestartNewSection;
	/* Set up the default state */
	
	if(isRestart()) {
		PtSetResource(toggleRestartNewSection,Pt_ARG_FLAGS,Pt_TRUE,Pt_SET);
	} else {
		PtSetResource(m_RestartLabel,	Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);
		PtSetResource(m_RestartUpDown,Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);
		PtSetResource(m_RestartText,Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);
	}

	UT_sint32 j = (UT_sint32) HdrEven;
	for(j = (UT_sint32) HdrEven ; j<= (UT_sint32) FtrLast; j++)
	{
		bool value = getValue ( (HdrFtr_Control) j);
		if(value)
		{
		 PtSetResource(m_wHdrFtr[j],Pt_ARG_FLAGS,Pt_TRUE,Pt_SET);
		}
		else
		{
			 PtSetResource(m_wHdrFtr[j],Pt_ARG_FLAGS,Pt_FALSE,Pt_SET);
		}
	}
return mainWindow;
}

void AP_QNXDialog_HdrFtr::CheckChanged(AP_Dialog_HdrFtr::HdrFtr_Control which)
{
bool value=false;
long *flags;

PtGetResource(m_wHdrFtr[which],Pt_ARG_FLAGS,&flags,0);
if(*flags & Pt_SET)
	value=true;
setValue(which,value,true);
}


void AP_QNXDialog_HdrFtr::RestartChanged(void)
{
char *text;
long * flags;
UT_sint32 RestartValue;

PtGetResource(m_RestartText,Pt_ARG_TEXT_STRING,&text,0);
PtGetResource(m_RestartToggle,Pt_ARG_FLAGS,&flags,0);

RestartValue=atoi(text);
if(Pt_SET & *flags)
	{
		PtSetResource(m_RestartLabel,	Pt_ARG_FLAGS,Pt_FALSE,Pt_GHOST|Pt_BLOCKED);
		PtSetResource(m_RestartUpDown,Pt_ARG_FLAGS,Pt_FALSE,Pt_GHOST|Pt_BLOCKED);
		PtSetResource(m_RestartText,Pt_ARG_FLAGS,Pt_FALSE,Pt_GHOST|Pt_BLOCKED);
		setRestart(true,RestartValue,true);
	}else {
		PtSetResource(m_RestartLabel,	Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);
		PtSetResource(m_RestartUpDown,Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);
		PtSetResource(m_RestartText,Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);
		setRestart(false,RestartValue,true);
	}
}
