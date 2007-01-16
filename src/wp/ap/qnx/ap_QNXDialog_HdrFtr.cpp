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
#include "xap_QNXFrameImpl.h"

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
dlg->RestartChanged();
return Pt_CONTINUE;
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
PtWidget_t *UpDown;
PtWidget_t *RestartLabel;

	mainWindow= abiCreatePhabDialog("ap_QNXDialog_HdrFtr",pSS,AP_STRING_ID_DLG_HdrFtr_Title);
	PtAddHotkeyHandler(mainWindow,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	SetupContextHelp(mainWindow,this);

	localizeLabel(abiPhabLocateWidget(mainWindow,"grpHdr"),pSS,AP_STRING_ID_DLG_HdrFtr_HeaderFrame);

	toggleHeadEven= abiPhabLocateWidget(mainWindow,"HdrEven");
	localizeLabel(toggleHeadEven,pSS,AP_STRING_ID_DLG_HdrFtr_HeaderEven);

	toggleHeadFirst=abiPhabLocateWidget(mainWindow,"HdrFirst");
	localizeLabel(toggleHeadFirst,pSS,AP_STRING_ID_DLG_HdrFtr_HeaderFirst);

	toggleHeadLast= abiPhabLocateWidget(mainWindow,"HdrLast");
	localizeLabel(toggleHeadLast,pSS,AP_STRING_ID_DLG_HdrFtr_HeaderLast);

	localizeLabel(abiPhabLocateWidget(mainWindow,"grpFtr"),pSS,AP_STRING_ID_DLG_HdrFtr_FooterFrame);

	toggleFootEven= abiPhabLocateWidget(mainWindow,"FtrEven");
	localizeLabel(toggleFootEven,pSS,AP_STRING_ID_DLG_HdrFtr_FooterEven);

	toggleFootFirst= abiPhabLocateWidget(mainWindow,"FtrFirst");
	localizeLabel(toggleFootFirst,pSS,AP_STRING_ID_DLG_HdrFtr_FooterFirst);

	toggleFootLast=abiPhabLocateWidget(mainWindow,"FtrLast");
	localizeLabel(toggleFootLast,pSS,AP_STRING_ID_DLG_HdrFtr_FooterLast);


	toggleRestartNewSection=abiPhabLocateWidget(mainWindow,"toggleRestart"); 
	localizeLabel(toggleRestartNewSection,pSS,AP_STRING_ID_DLG_HdrFtr_RestartCheck);

	RestartLabel = abiPhabLocateWidget(mainWindow,"lblRestart");
	localizeLabel(RestartLabel,pSS,AP_STRING_ID_DLG_HdrFtr_RestartNumbers);

	UpDown=abiPhabLocateWidget(mainWindow,"NumericRestart");

	PtButton_ok=abiPhabLocateWidget(mainWindow,"btnOK");
	localizeLabel(PtButton_ok,pSS,XAP_STRING_ID_DLG_OK);

	PtButton_cancel=abiPhabLocateWidget(mainWindow,"btnCancel");
	localizeLabel(PtButton_cancel,pSS,XAP_STRING_ID_DLG_Cancel);

	
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
	PtAddCallback(UpDown,Pt_CB_NUMERIC_CHANGED,ph_event_activate_updown,this);

	m_wHdrFtr[HdrEven]= toggleHeadEven;
	m_wHdrFtr[HdrFirst]= toggleHeadFirst;
	m_wHdrFtr[HdrLast]= toggleHeadLast;
	m_wHdrFtr[FtrEven]= toggleFootEven;
	m_wHdrFtr[FtrFirst]= toggleFootFirst;
	m_wHdrFtr[FtrLast]= toggleFootLast;
	m_RestartLabel=RestartLabel;
	m_RestartUpDown= UpDown;
	m_RestartToggle=toggleRestartNewSection;
	/* Set up the default state */
	
	if(isRestart()) {
		PtSetResource(toggleRestartNewSection,Pt_ARG_FLAGS,Pt_TRUE,Pt_SET);
	} else {
		PtSetResource(m_RestartLabel,	Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);
		PtSetResource(m_RestartUpDown,Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);
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
int *num;
long * flags;
UT_sint32 RestartValue;

PtGetResource(m_RestartUpDown,Pt_ARG_NUMERIC_VALUE,&num,0);
PtGetResource(m_RestartToggle,Pt_ARG_FLAGS,&flags,0);

RestartValue=*num;
if(Pt_SET & *flags)
	{
		PtSetResource(m_RestartLabel,	Pt_ARG_FLAGS,Pt_FALSE,Pt_GHOST|Pt_BLOCKED);
		PtSetResource(m_RestartUpDown,Pt_ARG_FLAGS,Pt_FALSE,Pt_GHOST|Pt_BLOCKED);
		setRestart(true,RestartValue,true);
	}else {
		PtSetResource(m_RestartLabel,	Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);
		PtSetResource(m_RestartUpDown,Pt_ARG_FLAGS,Pt_TRUE,Pt_GHOST|Pt_BLOCKED);
		setRestart(false,RestartValue,true);
	}
}
