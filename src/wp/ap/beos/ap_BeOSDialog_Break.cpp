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
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Break.h"
#include "ap_BeOSDialog_Break.h"

#include "ut_Rehydrate.h"

/*****************************************************************/
#define RAD_ON(rad, str) ((rad = (BRadioButton *)FindView(str)) && \
			  (rad->Value() == B_CONTROL_ON))

class BreakWin:public BWindow {
	public:
		BreakWin(BMessage *data);
		void SetDlg(AP_BeOSDialog_Break *brk);
		virtual void DispatchMessage(BMessage *msg, BHandler *handler);
		virtual bool QuitRequested(void);
		
	private:
		AP_BeOSDialog_Break 	*m_DlgBreak;
	
		status_t WaitForDelete(sem_id blocker);
		sem_id modalSem;	
};

BreakWin::BreakWin(BMessage *data) 
	  :BWindow(data)
{

} //BreakWin::BreakWin

status_t BreakWin::WaitForDelete(sem_id blocker)
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

void BreakWin::SetDlg(AP_BeOSDialog_Break *brk) {
	BRadioButton *on = NULL;

	m_DlgBreak = brk;
	switch(m_DlgBreak->getBreakType()) {
	case  AP_Dialog_Break::b_PAGE:
		on = (BRadioButton *)FindView("radPageBreak");
		break;
	case  AP_Dialog_Break::b_COLUMN:
		on = (BRadioButton *)FindView("radColBreak");
		break;
	case  AP_Dialog_Break::b_NEXTPAGE:
		on = (BRadioButton *)FindView("radNextBreak");
		break;
	case  AP_Dialog_Break::b_CONTINUOUS:
		on = (BRadioButton *)FindView("radContBreak");
		break;
	case  AP_Dialog_Break::b_EVENPAGE:
		on = (BRadioButton *)FindView("radEvenBreak");
		break;
	case  AP_Dialog_Break::b_ODDPAGE:
		on = (BRadioButton *)FindView("radOddBreak");
		break;
	}

	if (on) {
		on->SetValue(B_CONTROL_ON);
	}


//	We need to tie up the caller thread for a while ...
	Show();

	modalSem = create_sem(0, "semname");
	WaitForDelete(modalSem);
	
	Hide();
}

void BreakWin::DispatchMessage(BMessage *msg, BHandler *handler) {
	BRadioButton *on = NULL;

	switch(msg->what) {
	case 'btok':
		UT_DEBUGMSG(("BREAK: Set the page break \n"));
		UT_ASSERT(m_DlgBreak);
		if (RAD_ON(on, "radPageBreak"))
 			m_DlgBreak->setBreakType(AP_Dialog_Break::b_PAGE);
		else if (RAD_ON(on, "radColBreak"))
 			m_DlgBreak->setBreakType(AP_Dialog_Break::b_COLUMN);
		else if (RAD_ON(on, "radNextBreak"))
 			m_DlgBreak->setBreakType(AP_Dialog_Break::b_NEXTPAGE);
		else if (RAD_ON(on, "radEvenBreak"))
 			m_DlgBreak->setBreakType(AP_Dialog_Break::b_EVENPAGE);
		else if (RAD_ON(on, "radOddBreak"))
 			m_DlgBreak->setBreakType(AP_Dialog_Break::b_ODDPAGE);
		else if (RAD_ON(on, "radContBreak"))
 			m_DlgBreak->setBreakType(AP_Dialog_Break::b_CONTINUOUS);

		m_DlgBreak->setAnswer(AP_Dialog_Break::a_OK);
		delete_sem(modalSem);
		
		break;
	default:
		BWindow::DispatchMessage(msg, handler);
	}
} 

//Behave like a good citizen
bool BreakWin::QuitRequested() {
	UT_ASSERT(m_DlgBreak);
	m_DlgBreak->setAnswer(AP_Dialog_Break::a_CANCEL);

	delete_sem(modalSem);
	
	return(false);
}


/*****************************************************************/

XAP_Dialog * AP_BeOSDialog_Break::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_BeOSDialog_Break * p = new AP_BeOSDialog_Break(pFactory,id);
	return p;
}

AP_BeOSDialog_Break::AP_BeOSDialog_Break(XAP_DialogFactory * pDlgFactory,
					 XAP_Dialog_Id id) 
		   : AP_Dialog_Break(pDlgFactory,id) { 
} 

AP_BeOSDialog_Break::~AP_BeOSDialog_Break(void) {
}

/*****************************************************************/

void AP_BeOSDialog_Break::runModal(XAP_Frame * pFrame)
{
	/*
	  This dialog is non-persistent.
	  
	  This dialog should do the following:

	  - Construct itself to represent the base-class breakTypes
	    b_PAGE, b_COLUMN, b_NEXTPAGE, b_CONTINUOUS, b_EVENPAGE, b_ODDPAGE.
		The Unix one looks just like Microsoft Word 97, with the preview
		ind all (even though it's not hooked up yet).

	  - Set break type to match "m_break"

	  On "OK" (or during user-interaction) the dialog should:

	  - Save the break type to "m_break".
	  
	  On "Cancel" the dialog should:

	  - Just quit, the data items will be ignored by the caller.

	*/
	BMessage msg;
	BreakWin  *newwin;
	if (RehydrateWindow("BreakWindow", &msg)) {
                newwin = new BreakWin(&msg);
		newwin->SetDlg(this);
		//Take the information here ...
		newwin->Lock();
		newwin->Quit();
        }                                                
}

