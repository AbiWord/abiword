/* AbiSource Application Framework
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

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_Zoom.h"
#include "xap_BeOSDlg_Zoom.h"
#include "gr_BeOSGraphics.h"
#include "ut_Rehydrate.h"

/*****************************************************************/
class ZoomWin:public BWindow {
	public:
		ZoomWin(BMessage *data);
		~ZoomWin();
		void SetDlg(XAP_BeOSDialog_Zoom *brk);
		virtual void DispatchMessage(BMessage *msg, BHandler *handler);
		virtual void MessageReceived(BMessage *msg);
		void GetAnswer(XAP_BeOSDialog_Zoom::zoomType &ZoomType,UT_uint32 &ZoomPercent);
		
		virtual bool QuitRequested();
		
	private:
		
		XAP_BeOSDialog_Zoom 	*m_DlgZoom;
		GR_BeOSGraphics		*m_BeOSGraphics;
		BTextControl 		*m_CustomText;	
		UT_uint32		 m_CurrentPercent;
		XAP_BeOSDialog_Zoom::zoomType	 m_CurrentType;
		UT_Bool			 m_Okay;
		
		sem_id modalSem;
		status_t WaitForDelete(sem_id deleteSem);
		
};	

status_t ZoomWin::WaitForDelete(sem_id blocker)
{
	status_t	result;
	thread_id	this_tid = find_thread(NULL);
	BLooper		*pLoop;
	BWindow		*pWin = 0;

	pLoop = BLooper::LooperForThread(this_tid);
	if (pLoop)
		pWin = dynamic_cast<BWindow*>(pLoop);

	// block until semaphore is deleted (modal is finished)
	if (pWin) {
		do {
			// update the window periodically			
			pWin->UpdateIfNeeded();
			result = acquire_sem_etc(blocker, 1, B_TIMEOUT, 10000);
		} while (result != B_BAD_SEM_ID);
	} else {
		do {
			// just wait for exit
			result = acquire_sem(blocker);
		} while (result != B_BAD_SEM_ID);
	}
	return result;
}

bool ZoomWin::QuitRequested()
{
	delete_sem(modalSem);
	Hide();
	
	return (false); // We can't quit twice, so, the little close box would fail if we didn't do this.
}

void ZoomWin::GetAnswer(XAP_BeOSDialog_Zoom::zoomType &ZoomType, UT_uint32 &ZoomPercent)
{
	if (m_Okay==UT_TRUE)
	{
		ZoomType=m_CurrentType;
		ZoomPercent=m_CurrentPercent;
	}
	else
	{
		ZoomPercent=0;
	}
}
ZoomWin::~ZoomWin()
{
	DELETEP(m_BeOSGraphics);
}
ZoomWin::ZoomWin(BMessage *data) 
	  :BWindow(data) 
{
	
	m_Okay=UT_FALSE;
} //ZoomWin::ZoomWin
void ZoomWin::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case '200p':
		{
			m_CustomText->SetEnabled(false);
			m_DlgZoom->_updatePreviewZoomPercent(200);
			m_CurrentPercent=200;
			m_CurrentType=XAP_Frame::z_200;
		}
		break;
		case '100p':
		{
			m_CustomText->SetEnabled(false);
			m_DlgZoom->_updatePreviewZoomPercent(100);
			m_CurrentPercent=100;
			m_CurrentType=XAP_Frame::z_100;
		}
		break;
		case '075p':
		{
			m_CustomText->SetEnabled(false);
			m_DlgZoom->_updatePreviewZoomPercent(75);
			m_CurrentType=XAP_Frame::z_75;
			m_CurrentPercent=75;
		}
		break;
		case 'cust':
		{
			m_CustomText->SetEnabled(true);
		}
		break;
		case 'pwid':
		{
			m_CustomText->SetEnabled(false);
			//Might want to figure out the page width
			m_CurrentType=XAP_Frame::z_PAGEWIDTH;
			m_CurrentPercent=100;
		}
		break;
		case 'whpg':
		{
			m_CustomText->SetEnabled(false);
			m_CurrentType=XAP_Frame::z_WHOLEPAGE;
			m_CurrentPercent=100;
		}	
		break;
		case 'perc':
		{
			m_DlgZoom->_updatePreviewZoomPercent(atoi(m_CustomText->Text()));
			m_CurrentType=XAP_Frame::z_PERCENT;
			m_CurrentPercent=atoi(m_CustomText->Text());
		}
		case 'appl':
		{
			printf("Setting okay to true\n");
			m_Okay=UT_TRUE;
			
			PostMessage(B_QUIT_REQUESTED);
		}
		break;
		default:
			BWindow::MessageReceived(msg);
	}
	
}
void ZoomWin::SetDlg(XAP_BeOSDialog_Zoom *dlg) {

	m_DlgZoom=dlg;
	BView *preview=(BView*)FindView("previewview");
	m_CustomText=(BTextControl *)FindView("custxt");
	Show();
	//Create our preview window graphics
	m_BeOSGraphics=new GR_BeOSGraphics(preview, dlg->m_pApp);
	if (preview->Window()->Lock())
	{
		dlg->_createPreviewFromGC(m_BeOSGraphics,preview->Frame().Width(),preview->Frame().Height());
		this->PostMessage(new BMessage('100p'));
		BRadioButton *ohbutton=(BRadioButton *)FindView("OneHundred");
		ohbutton->SetValue(B_CONTROL_ON);
		preview->Window()->Unlock();
	}

//	while () { snooze(1000); }
	modalSem = create_sem(0,"zoomsem");
	
	WaitForDelete(modalSem);
	
	Hide();
}

void ZoomWin::DispatchMessage(BMessage *msg, BHandler *handler) {
	switch(msg->what) {
	default:
		BWindow::DispatchMessage(msg, handler);
	}
} 

/*****************************************************************/

XAP_Dialog * XAP_BeOSDialog_Zoom::static_constructor(XAP_DialogFactory * pFactory,
													XAP_Dialog_Id id)
{
	XAP_BeOSDialog_Zoom * p = new XAP_BeOSDialog_Zoom(pFactory,id);
	return p;
}

XAP_BeOSDialog_Zoom::XAP_BeOSDialog_Zoom(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_Zoom(pDlgFactory,id)
{

}

XAP_BeOSDialog_Zoom::~XAP_BeOSDialog_Zoom(void)
{
}

/*****************************************************************/

void XAP_BeOSDialog_Zoom::runModal(XAP_Frame * pFrame)
{

	/*
	  This dialog is non-persistent.
	  
	  This dialog should do the following:

	  - Construct itself to represent the base-class zoomTypes
	    z_200, z_100, z_75, z_PageWidth, z_WholePage, and z_Percent.
		The Unix one looks just like Microsoft Word 97, with the preview
		and all (even though it's not hooked up yet).

	  - Set zoom type to match "m_zoomType" and value of radio button
	    to match "m_zoomPercent".

	  On "OK" (or during user-interaction) the dialog should:

	  - Save the zoom type to "m_zoomType".
	  
	  - Save the value in the Percent spin button box to "m_zoomPercent".

	  On "Cancel" the dialog should:

	  - Just quit, the data items will be ignored by the caller.

	*/

	// TODO build the dialog, attach events, etc., etc.
	BMessage msg;
	ZoomWin  *newwin;
	if (RehydrateWindow("ZoomWindow", &msg)) {
                newwin = new ZoomWin(&msg);
		newwin->SetDlg(this);
		//Take the information here ...
		if (newwin->Lock())
		{
			newwin->GetAnswer(m_zoomType,m_zoomPercent);
			if (m_zoomPercent != 0)
			{
				 UT_DEBUGMSG(("Okaying, m_zoomType=%d, m_zoomPercent=%d\n",m_zoomType,m_zoomPercent));
			 	m_answer=XAP_Dialog_Zoom::a_OK;
			}
			else
			{
				UT_DEBUGMSG(("Cancelling, m_zoomType=%d, m_zoomPercent=%d\n",m_zoomType,m_zoomPercent));
				m_answer=XAP_Dialog_Zoom::a_CANCEL;
			}
		}

		newwin->Close();
		
		
        }                                                
}

/*****************************************************************/
