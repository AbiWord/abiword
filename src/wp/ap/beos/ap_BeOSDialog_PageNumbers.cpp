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
#include <stdio.h>
#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "ap_Strings.h"
#include "ap_BeOSDialog_PageNumbers.h"

#include "ut_Rehydrate.h"
#include "gr_BeOSGraphics.h"

/*****************************************************************/

class PageNumberWindow:public BWindow {
	public:
		PageNumberWindow(BMessage *data);
		virtual ~PageNumberWindow();
		
		void SetDlg(AP_BeOSDialog_PageNumbers *brk);
		
		virtual void MessageReceived(BMessage *pMsg);
		virtual bool QuitRequested(void);
		
	private:
		AP_BeOSDialog_PageNumbers *m_DlgPage;
		GR_BeOSGraphics			*m_BeOSGraphics;
		
		sem_id modalSem;
		status_t WaitForDelete(sem_id deleteSem);
};

PageNumberWindow::PageNumberWindow(BMessage* archive)
				 :BWindow(archive)
{


}

PageNumberWindow::~PageNumberWindow()
{
	DELETEP(m_BeOSGraphics);
}

void PageNumberWindow::MessageReceived(BMessage* pMsg)
{
	int32 index = 0;
	
	switch(pMsg->what)
	{
		case 'okay':
			m_DlgPage->setAnswer( AP_Dialog_PageNumbers::a_OK );//tAnswer = AP_Dialog_PageNumbers::a_OK;
			PostMessage(B_QUIT_REQUESTED);	
			break;
			
		case 'posi':
			if( pMsg->FindInt32( "index" , &index) == B_OK)
				m_DlgPage->m_recentControl = ( AP_Dialog_PageNumbers::tControl )index;
				
			m_DlgPage->_updatePreview(m_DlgPage->m_recentAlign, m_DlgPage->m_recentControl);
			break;
			
		case 'alig':
			if( pMsg->FindInt32( "index" , &index) == B_OK)
				m_DlgPage->m_recentAlign = ( AP_Dialog_PageNumbers::tAlign )index;

			m_DlgPage->_updatePreview(m_DlgPage->m_recentAlign, m_DlgPage->m_recentControl);
			break;
			
		default:
			BWindow::MessageReceived(pMsg);
	}
}

void PageNumberWindow::SetDlg(AP_BeOSDialog_PageNumbers *pDlg)
{
	m_DlgPage = pDlg;
	m_DlgPage->setAnswer( AP_Dialog_PageNumbers::a_CANCEL );//tAnswer = AP_Dialog_PageNumbers::a_CANCEL; // Initially cancel until ok hit.
	
	const XAP_StringSet * pSS = m_DlgPage->m_pApp->getStringSet();
	
	BMenuField* positionMenu = (BMenuField *)FindView("positionMenu");
	if(positionMenu)
	{
		positionMenu->Menu()->AddItem( new BMenuItem( pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Right), new BMessage('alig')));
		positionMenu->Menu()->AddItem( new BMenuItem( pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Left), new BMessage('alig')));
		positionMenu->Menu()->AddItem( new BMenuItem( pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Center), new BMessage('alig')));
		positionMenu->Menu()->ItemAt(0)->SetMarked(true);		
	}
	
	BMenuField* alignmentMenu = (BMenuField *)FindView("alignMenu");
	if(alignmentMenu)
	{
		alignmentMenu->Menu()->AddItem( new BMenuItem( pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Header) , new BMessage('posi')));
		alignmentMenu->Menu()->AddItem( new BMenuItem( pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Footer) , new BMessage('posi')));
		alignmentMenu->Menu()->ItemAt(0)->SetMarked(true);
	}
	
	Show();

	if( Lock() )
	{
		BView *preview = (BView*)FindView("preview");	
	
		m_BeOSGraphics  = new GR_BeOSGraphics(preview, m_DlgPage->m_pApp);
		
		m_DlgPage->_createPreviewFromGC(m_BeOSGraphics,preview->Frame().Width(),preview->Frame().Height());
	
		// hack in a quick draw here.
		m_DlgPage->_updatePreview( m_DlgPage->m_recentAlign , m_DlgPage->m_recentControl );
		
		Unlock();
	}
	
	// hack in a quick draw here.
	m_DlgPage->_updatePreview( m_DlgPage->m_recentAlign , m_DlgPage->m_recentControl );

	modalSem = create_sem(0,"ColumnWindowSem");
	WaitForDelete(modalSem);
}

status_t PageNumberWindow::WaitForDelete(sem_id blocker)
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


//Behave like a good citizen
bool PageNumberWindow::QuitRequested() 
{
	delete_sem(modalSem);
	
	return(true);
}

/*****************************************************************/

AP_BeOSDialog_PageNumbers::AP_BeOSDialog_PageNumbers (XAP_DialogFactory * pDlgFactory,
						     XAP_Dialog_Id id) : AP_Dialog_PageNumbers (pDlgFactory, id)
{

}

XAP_Dialog * AP_BeOSDialog_PageNumbers::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_BeOSDialog_PageNumbers * p = new AP_BeOSDialog_PageNumbers(pFactory,id);
	return p;
}

AP_BeOSDialog_PageNumbers::~AP_BeOSDialog_PageNumbers(void)
{
}

void AP_BeOSDialog_PageNumbers::runModal(XAP_Frame * pFrame)
{
	BMessage msg;
	PageNumberWindow* newwin;
	
	m_recentControl = id_HDR;
	m_recentAlign = id_RALIGN;
	
	if (RehydrateWindow("PageNumber", &msg)) 
	{
        newwin = new PageNumberWindow(&msg);
		newwin->SetDlg(this);
		
		if( m_answer == a_OK)
		{
			m_align = m_recentAlign;
			m_control = m_recentControl;
    	}
    }
}