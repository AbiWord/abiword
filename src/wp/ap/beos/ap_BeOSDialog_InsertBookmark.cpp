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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_BeOSDialog_InsertBookmark.h"
#include "ap_BeOSDialog_All.h"

//#include "ut_Rehydrate.h"

#include "StorageKit.h"
#include "SupportKit.h"
#include "fs_attr.h"

/*****************************************************************/

class BookmarkWin:public BWindow {
	public:
		BookmarkWin(BRect &frame, const XAP_StringSet * pSS);
		void SetDlg(AP_BeOSDialog_InsertBookmark *dlg, const XAP_StringSet *pSS);
		virtual void DispatchMessage(BMessage *msg, BHandler *handler);
		virtual bool QuitRequested(void);
		static const int _eventOK = 'evok';
		static const int _eventCancel = 'evcl';
		static const int _eventDelete = 'evdl';
		static const int _eventSelected = 'slct';

	private:
		AP_BeOSDialog_InsertBookmark 	*m_DlgBookmark;
		BTextControl 	*m_CustomText;	
		BListView		*m_Listview;	

		sem_id modalSem;
		status_t WaitForDelete(sem_id blocker);
};

status_t BookmarkWin::WaitForDelete(sem_id blocker)
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
			pWin->Unlock();
			snooze(200);
			pWin->Lock();
			
			// update the window periodically
			pWin->UpdateIfNeeded();
			result = acquire_sem_etc(blocker, 1, B_TIMEOUT, 1000);
		} while (result != B_BAD_SEM_ID);
	} else {
		do {
			// just wait for exit
			result = acquire_sem(blocker);
		} while (result != B_BAD_SEM_ID);
	}
	return result;
}

void BookmarkWin::SetDlg(AP_BeOSDialog_InsertBookmark *dlg , const XAP_StringSet *pSS)
{
	BPath path;
	m_DlgBookmark = dlg;
	int i;

	// This semaphore ties up the window until after it deletes..
	modalSem = create_sem(0,"BookmarkModalSem");

	for(i = 0; i < m_DlgBookmark->getExistingBookmarksCount(); i++)
	{
		BStringItem *item = new BStringItem(m_DlgBookmark->getNthExistingBookmark(i), 0, true);
		m_Listview->AddItem(item);
	}


	Show();
	WaitForDelete(modalSem);
	Hide();
}

void BookmarkWin::DispatchMessage(BMessage *msg, BHandler *handler)
{
	BListItem *item;

	switch(msg->what) {
	case _eventOK:
		m_DlgBookmark->setAnswer(AP_Dialog_InsertBookmark::a_OK);
		if (m_CustomText->Text() != NULL)
			m_DlgBookmark->setBookmark(m_CustomText->Text());
		be_app->PostMessage(B_QUIT_REQUESTED);
		break;
	case _eventDelete:
		m_DlgBookmark->setAnswer(AP_Dialog_InsertBookmark::a_DELETE);
		if (m_CustomText->Text() != NULL)
			m_DlgBookmark->setBookmark(m_CustomText->Text());
		be_app->PostMessage(B_QUIT_REQUESTED);
		break;
	case _eventCancel:
		m_DlgBookmark->setAnswer(AP_Dialog_InsertBookmark::a_CANCEL);
		be_app->PostMessage(B_QUIT_REQUESTED);
		break;
	case _eventSelected:
		item = m_Listview->ItemAt(m_Listview->CurrentSelection());
		m_CustomText->SetText(dynamic_cast<BStringItem*>(item)->Text());
		break;
	default:
		BWindow::DispatchMessage(msg, handler);
	}
}


BookmarkWin::BookmarkWin(BRect &frame, const XAP_StringSet * pSS) :
	BWindow(frame, "BookmarkWin", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS)
{
	SetTitle(pSS->getValue(AP_STRING_ID_DLG_InsertBookmark_Title));
	SetSizeLimits(250,350,150,350);

	BView *panel = new BView(Bounds(), "BookmarkPanel", B_FOLLOW_ALL_SIDES, B_FRAME_EVENTS | B_WILL_DRAW);
	panel->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(panel);

	BRect frame(10,Bounds().bottom-30, 80, 0);
	panel->AddChild(new BButton(frame, "cancelbutton", pSS->getValue(XAP_STRING_ID_DLG_Cancel),
				new BMessage(_eventCancel), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM));
	frame.OffsetBy(frame.Width() + 10, 0);
	panel->AddChild(new BButton(frame, "deltetebutton", pSS->getValue(XAP_STRING_ID_DLG_Delete),
				new BMessage(_eventDelete), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM));
	frame.OffsetBy(frame.Width() + 10, 0);
	panel->AddChild(new BButton(frame, "okbutton", pSS->getValue(XAP_STRING_ID_DLG_OK),
				new BMessage(_eventOK), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM));

	frame = Bounds();
	frame.top = 5;
	frame.left = 8;
	frame.bottom = frame.top + 20;
	frame.right -= 8;

	BStringView *stringview = new BStringView(frame, "BookmarkText", 
		pSS->getValue(AP_STRING_ID_DLG_InsertBookmark_Msg), B_FOLLOW_TOP, B_WILL_DRAW);
	panel->AddChild(stringview);

	frame = Bounds();
	frame.top = 30;
	frame.bottom = frame.top + 30;
	frame.left = 8;
	frame.right -= 8;

	m_CustomText = new BTextControl(frame, "BookmarkTextControl", 
		NULL, NULL, NULL, B_FOLLOW_TOP | B_FOLLOW_LEFT | B_FOLLOW_RIGHT, B_WILL_DRAW);
	m_CustomText->SetDivider(0.0);
	panel->AddChild(m_CustomText);
	m_CustomText->MakeFocus(true);

	frame = Bounds();
	frame.top = 60;
	frame.bottom -= 40;
	frame.right -= (8 + B_V_SCROLL_BAR_WIDTH);
	frame.left = 8;

	m_Listview = new BListView(frame, "BookmarkListView", 
		B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES);
	m_Listview->SetSelectionMessage(new BMessage(_eventSelected)); 
		
	BScrollView *scroll=new BScrollView("scroll",m_Listview, B_FOLLOW_ALL_SIDES, 0,false,true);
	panel->AddChild(scroll);

}

bool BookmarkWin::QuitRequested() {
	UT_ASSERT(m_DlgBookmark);

	delete_sem(modalSem);
	
	return(false);
}

XAP_Dialog * AP_BeOSDialog_InsertBookmark::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_BeOSDialog_InsertBookmark * p = new AP_BeOSDialog_InsertBookmark(pFactory,id);
	int m_pBookmarks = 0;
	return p;
}

AP_BeOSDialog_InsertBookmark::AP_BeOSDialog_InsertBookmark(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_InsertBookmark(pDlgFactory,id)
{
}

AP_BeOSDialog_InsertBookmark::~AP_BeOSDialog_InsertBookmark(void)
{
}

void AP_BeOSDialog_InsertBookmark::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	BookmarkWin  *newwin;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	XAP_BeOSFrame * pBeOSFrame = static_cast<XAP_BeOSFrame *>(pFrame);
	BRect parentPosition = pBeOSFrame->getTopLevelWindow()->Frame();
	// Center the dialog according to the parent
	BRect dialogPosition = parentPosition;
	// Let us suppose the dialog is 300x300
	dialogPosition.InsetBy(-(300-parentPosition.Width())/2, -(250-parentPosition.Height())/2);
	newwin = new BookmarkWin(dialogPosition, pSS);

	newwin->SetDlg(this , pSS);

	//Take the information here ...
	newwin->Lock();
	newwin->Quit();
}
