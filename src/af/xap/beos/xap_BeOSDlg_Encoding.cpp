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

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Encoding.h"
#include "xap_BeOSDlg_Encoding.h"

#include <Window.h>
#include <View.h>
#include <ListView.h>
#include <ScrollView.h>

/*****************************************************************/

class EncodingWin : public BWindow {
	static const int _eventOK = 'evok';
	static const int _eventCancel = 'evcl';
	sem_id modalSem;
	int32 selection;
	BListView *listBox;
public:
	EncodingWin(BRect &frame, const XAP_StringSet * pSS, UT_uint32 encodCount,
			const XML_Char ** encodList, UT_uint32 currentEncodIndex);
	bool SetDlg(UT_uint32 * newEncod);
	void MessageReceived(BMessage *msg);
	bool QuitRequested(void);
};

EncodingWin::EncodingWin(BRect &frame, const XAP_StringSet * pSS, UT_uint32 encodCount,
			const XML_Char ** encodList, UT_uint32 currentEncodIndex) :
	BWindow(frame, "EncodingWin", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS)
{
	selection = -1;
	modalSem = create_sem(0,"ParagraphSem");

	SetTitle(pSS->getValue(XAP_STRING_ID_DLG_UENC_EncTitle));

	BView *panel = new BView(Bounds(), "EncodingPanel", B_FOLLOW_ALL_SIDES, B_FRAME_EVENTS | B_WILL_DRAW);
	panel->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(panel);

	BRect frame(10,Bounds().bottom-30, 80, 0);
	panel->AddChild(new BButton(frame, "cancelbutton", pSS->getValue(XAP_STRING_ID_DLG_Cancel),
				new BMessage(_eventCancel), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM));
	frame.OffsetBy(frame.Width() + 20, 0);
	panel->AddChild(new BButton(frame, "okbutton", pSS->getValue(XAP_STRING_ID_DLG_OK),
				new BMessage(_eventOK), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM));

	frame = Bounds();
	frame.InsetBy(10,10);
	frame.right -= B_V_SCROLL_BAR_WIDTH;
	frame.bottom -= 30.0;	// This space is for the buttons
	listBox = new BListView(frame,
			"EncodingList", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES);

	// Create a scrollView for the list
	panel->AddChild(new BScrollView("scroll_encodings", listBox,
		B_FOLLOW_ALL_SIDES, 0, false, true));

	// Add all Elements
	for (UT_uint32 k=0; k<encodCount; k++) {
		listBox->AddItem(new BStringItem(encodList[k]));
	}

	// HighLight the current selection
	listBox->Select(currentEncodIndex);
	listBox->ScrollToSelection();
}

bool EncodingWin::SetDlg(UT_uint32 * newEncod) {
	status_t        result;
	Show();
	thread_id       this_tid = find_thread(NULL);
	BLooper         *pLoop;
	BWindow         *pWin = 0;

	pLoop = BLooper::LooperForThread(this_tid);
	if (pLoop)
		pWin = dynamic_cast<BWindow*>(pLoop);
	do {
		/* HACK : we are in a modal behaviour so events to
		 * the main window are not delivered. That's why
		 * we explicitly call UpdateIfNeeded()
		 */
		if (pWin) {
			pWin->Unlock(); // Who will know?=)
			snooze(100);
			pWin->Lock();

			// update the window periodically
			pWin->UpdateIfNeeded();
		}

		// just wait for exit
		result = acquire_sem_etc(modalSem, 1, B_TIMEOUT, 1000);
	} while (result != B_BAD_SEM_ID);

	if (selection >= 0) {
		*newEncod = selection;
		return true;
	} else {
		return false;
	}
}

void EncodingWin::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case _eventOK :
			selection = listBox->CurrentSelection();
			// FALL THROUGH
		case _eventCancel :
			// Delete the semaphore to end the modal dialogue
			delete_sem(modalSem);
			return;
	}
	BWindow::MessageReceived(msg);
}

bool EncodingWin::QuitRequested(void) {
	delete_sem(modalSem);	// Release the modal behaviour
	return true;		// Ok to quit this dialog
}

XAP_Dialog * XAP_BeOSDialog_Encoding::static_constructor(XAP_DialogFactory * pFactory,
								XAP_Dialog_Id id)
{
	XAP_BeOSDialog_Encoding * p = new XAP_BeOSDialog_Encoding(pFactory,id);
	return p;
}

XAP_BeOSDialog_Encoding::XAP_BeOSDialog_Encoding(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: XAP_Dialog_Encoding(pDlgFactory,id)
{
}

XAP_BeOSDialog_Encoding::~XAP_BeOSDialog_Encoding(void)
{
}

void XAP_BeOSDialog_Encoding::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	XAP_BeOSFrame * pBeOSFrame = static_cast<XAP_BeOSFrame *>(pFrame);
	BRect parentPosition = pBeOSFrame->getTopLevelWindow()->Frame();
	// Center the dialog according to the parent
	BRect dialogPosition = parentPosition;
	// Let us suppose the dialog is 200x300
	dialogPosition.InsetBy(-(200-parentPosition.Width())/2, -(300-parentPosition.Height())/2);
	EncodingWin * dlg = new EncodingWin(dialogPosition, m_pApp->getStringSet(),
					_getEncodingsCount(), _getAllEncodings(), _getSelectionIndex());

	UT_uint32 newEncod = 0;
	if (dlg->SetDlg(&newEncod)) {	// Show the window and let the user choose
		// The user has click "OK"
		_setEncoding(_getAllEncodings()[newEncod]);
		_setAnswer(XAP_Dialog_Encoding::a_OK);
	} else {
		_setAnswer(XAP_Dialog_Encoding::a_CANCEL);
	}
	dlg->Lock();		// Lock it...
	dlg->Quit();		// to delete it
}
