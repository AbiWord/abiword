/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_WordCount.h"
#include "ap_MacDialog_WordCount.h"

////////////////////////////////////////////////////////////////////////////////

XAP_Dialog * AP_MacDialog_WordCount::static_constructor(XAP_DialogFactory * pFactory,
								 XAP_Dialog_Id id)
{
	AP_MacDialog_WordCount * p = new AP_MacDialog_WordCount(pFactory,id);
	p->newwin = NULL;
	p->windowIsValid = false;
	return p;
}

AP_MacDialog_WordCount::AP_MacDialog_WordCount(XAP_DialogFactory * pDlgFactory,
							 XAP_Dialog_Id id)
	: AP_Dialog_WordCount(pDlgFactory,id)
{
	newwin = NULL;
	windowIsValid = false;
}

AP_MacDialog_WordCount::~AP_MacDialog_WordCount(void)
{
}

void AP_MacDialog_WordCount::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
/*	
	BMessage msg;
	if (RehydrateWindow("WordCount", &msg))
		{
        newwin = new WordCountWin(&msg);
		newwin->SetDlg(this);
		//Take the information here ...
		newwin->Lock();
		newwin->Close();
        } */
}

void AP_MacDialog_WordCount::runModeless(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
		
	if(windowIsValid && newwin != NULL)
	{
                ::ShowWindow (newwin);
                ::BringToFront (newwin);
		return;
	}
		
/*
	if (RehydrateWindow("WordCount", &msg))
		{
        newwin = new WordCountWin(&msg);
		newwin->SetDlg(this);
		
		GR_Graphics * pG = NULL;
		m_pAutoUpdateWC = UT_Timer::static_constructor(autoupdateWC,this,pG);
		setUpdateCounter( 1 );
	
		windowIsValid = true;
		//Take the information here ...
//		newwin->Lock();
//		newwin->Close();
        } 
*/
}

void  AP_MacDialog_WordCount::setUpdateCounter( UT_uint32 iRate )
{
	UT_uint32 iFactor = 1000;

//	m_bDestroy_says_stopupdating = false;
//	m_bAutoUpdate_happening_now = false;

	// Make a special case for 0 seconds in
	// an attempt to reduce screen flicker
	if( iRate == 0 )
		iFactor = 100;

	m_pAutoUpdateWC->stop();

	m_iUpdateRate = iRate;

//	if(m_bAutoWC == true)
	m_pAutoUpdateWC->set(m_iUpdateRate * iFactor);
}         

void AP_MacDialog_WordCount::_updateWindowData(void)
{
#if 0
//	SetDlgItemInt(hWnd,AP_RID_DIALOG_WORDCOUNT_VAL_PAGE,m_count.page,FALSE)
	BStringView* pView;
	char numBuf[12];
	
	pView = (BStringView *)newwin->FindView("page");
	if(pView)
	{
		sprintf(numBuf,"%i" , m_count.page);
		pView->SetText(numBuf);
	}
	
	pView = (BStringView *)newwin->FindView("word");
	if(pView)
	{
		sprintf(numBuf,"%i" , m_count.word);
		pView->SetText(numBuf);
	}
	
	pView = (BStringView *)newwin->FindView("ch_no");
	if(pView)
	{
		sprintf(numBuf,"%i" , m_count.ch_no);
		pView->SetText(numBuf);
	}
	
	pView = (BStringView *)newwin->FindView("ch_sp");
	if(pView)
	{
		sprintf(numBuf,"%i" , m_count.ch_sp);
		pView->SetText(numBuf);
	}
	
	pView = (BStringView *)newwin->FindView("para");
	if(pView)
	{
		sprintf(numBuf,"%i" , m_count.para);
		pView->SetText(numBuf);
	}
	
	pView = (BStringView *)newwin->FindView("line");
	if(pView)
	{
		sprintf(numBuf,"%i" , m_count.line);
		pView->SetText(numBuf);
	}
	
	//_DSI(WORDCOUNT_VAL_PAGE,		page);
	//_DSI(WORDCOUNT_VAL_WORD,		word);
	//_DSI(WORDCOUNT_VAL_CH,			ch_no);
	//_DSI(WORDCOUNT_VAL_CHSP,		ch_sp);
	//_DSI(WORDCOUNT_VAL_PARA,		para);
	//_DSI(WORDCOUNT_VAL_LINE,		line);

	// Update the caption in case the name of the document has changed
//	ConstructWindowName();
//	SetWindowText(hWnd, m_WindowName);
#endif
}

void AP_MacDialog_WordCount::event_Update(void)
{
	setCountFromActiveFrame();
	_updateWindowData();
}

void AP_MacDialog_WordCount::autoupdateWC(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);

	// this is a static callback method and does not have a 'this' pointer.

	AP_MacDialog_WordCount * pDialog =  (AP_MacDialog_WordCount *) pTimer->getInstanceData();

	pDialog->event_Update();
} 

void AP_MacDialog_WordCount::destroy(void)
{
	if(newwin)
	{
		m_pAutoUpdateWC->stop();
		windowIsValid = false;
	}
}

void AP_MacDialog_WordCount::activate(void)
{
#if 0
	if(newwin && windowIsValid)
	{
		newwin->Lock();
		newwin->Activate();
		newwin->Unlock();
	}
#endif
}
  
void AP_MacDialog_WordCount::notifyActiveFrame(XAP_Frame *pFrame)
{

}

void AP_MacDialog_WordCount::notifyCloseFrame(XAP_Frame *pFrame)
{

}
