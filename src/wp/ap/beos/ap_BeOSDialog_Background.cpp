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

#include <InterfaceKit.h>

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Background.h"
#include "ap_BeOSDialog_Background.h"

#include "ut_Rehydrate.h"

/*****************************************************************/

class BackgroundWin : public BWindow
{
public:
	BackgroundWin(BRect &frame, const XAP_StringSet * pSS);
	void			SetDlg(AP_BeOSDialog_Background *dlg , const XAP_StringSet *pSS);
	virtual void	DispatchMessage(BMessage *msg, BHandler *handler);
	virtual bool	QuitRequested(void);
	static const int _eventOK = 'evok';
	static const int _eventCancel = 'evcl';
	static const int _eventSelected = 'slct';

private:
	AP_BeOSDialog_Background	*m_DlgBackground;
	BColorControl *m_ColorControl;	
	BView *m_Preview;
	sem_id modalSem;
	status_t WaitForDelete(sem_id blocker);
};

XAP_Dialog * AP_BeOSDialog_Background::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_BeOSDialog_Background * p = new AP_BeOSDialog_Background(pFactory,id);
	return p;
}

AP_BeOSDialog_Background::AP_BeOSDialog_Background(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_Background(pDlgFactory,id)
{
}

AP_BeOSDialog_Background::~AP_BeOSDialog_Background(void)
{
}

void AP_BeOSDialog_Background::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	BackgroundWin  *newwin;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	XAP_BeOSFrameImpl * pBeOSFrameImpl = static_cast<XAP_BeOSFrameImpl *>(pFrame->getFrameImpl());
	BRect parentPosition = pBeOSFrameImpl->getTopLevelWindow()->Frame();
	// Center the dialog according to the parent
	BRect dialogPosition = parentPosition;
	// Let us suppose the dialog is 300x150
	dialogPosition.InsetBy(-(300-parentPosition.Width())/2, -(150-parentPosition.Height())/2);
	newwin = new BackgroundWin(dialogPosition, pSS);

	newwin->SetDlg(this , pSS);
	//Take the information here ...
	newwin->Lock();
	newwin->Quit();
}

BackgroundWin::BackgroundWin(BRect &frame, const XAP_StringSet * pSS) :
	BWindow(frame, "BackgroundWin", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	SetTitle(pSS->getValue(AP_STRING_ID_DLG_Background_Title));

	BView *panel = new BView(Bounds(), "BackgroundPanel", B_FOLLOW_ALL_SIDES, B_FRAME_EVENTS | B_WILL_DRAW);
	panel->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(panel);

	frame = Bounds();
	frame.top = 5;
	frame.left = 8;

	BPoint point;
	point.Set(frame.left, frame.top);
	float width, height;

	m_ColorControl = new BColorControl(point, B_CELLS_32x8, 4.0, "ColorControl", 
		new BMessage(_eventSelected), true);
	panel->AddChild(m_ColorControl);
	m_ColorControl->GetPreferredSize(&width, &height);
	
	frame.top += height + 10;
	frame.bottom = frame.top + 50;
	frame.left = 8;
	frame.right = frame.left + width;

	m_Preview = new BView(frame, "BackgroundPreview", B_FOLLOW_TOP | B_FOLLOW_LEFT , B_WILL_DRAW);
	panel->AddChild(m_Preview);

	frame.top = frame.bottom + 5;
	frame.bottom = 0;
	frame.left = 8;
	frame.right = frame.left + 80;

	panel->AddChild(new BButton(frame, "cancelbutton", pSS->getValue(XAP_STRING_ID_DLG_Cancel),
				new BMessage(_eventCancel), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM));
	frame.OffsetBy(frame.Width() + 20, 0);
	panel->AddChild(new BButton(frame, "okbutton", pSS->getValue(XAP_STRING_ID_DLG_OK),
				new BMessage(_eventOK), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM));
	ResizeTo(width + 16, frame.top + 30);
}

void BackgroundWin::SetDlg(AP_BeOSDialog_Background *dlg , const XAP_StringSet *pSS) 
{
	m_DlgBackground = dlg;
	rgb_color color;
	const XML_Char *  pszC = m_DlgBackground->getColor();
	UT_RGBColor c(255,255,255);
	UT_parseColor(pszC,c);
	color.red = c.m_red;
	color.green = c.m_grn;
	color.blue = c.m_blu;
	m_ColorControl->SetValue(color);
	m_Preview->SetHighColor(color);

	modalSem = create_sem(0,"BackgroundSem");
	Show();
	WaitForDelete(modalSem);
	Hide();
}

status_t BackgroundWin::WaitForDelete(sem_id blocker)
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
			snooze(100);
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


void BackgroundWin::DispatchMessage(BMessage *msg, BHandler *handler) {

	int32 selection;
	static char buf_color[12];
	rgb_color color;

	switch(msg->what) {
	case _eventOK:
		color = m_ColorControl->ValueAsColor();
		sprintf(buf_color,"%02x%02x%02x",color.red, color.green, color.blue);
		m_DlgBackground->setColor((const XML_Char *)buf_color);
		be_app->PostMessage(B_QUIT_REQUESTED);
		break;

	case _eventCancel:
		be_app->PostMessage(B_QUIT_REQUESTED);
		break;
	case _eventSelected:
		m_Preview->SetHighColor(m_ColorControl->ValueAsColor());
		m_Preview->FillRect(m_Preview->Bounds(), B_SOLID_HIGH);
		break;
	default:
		BWindow::DispatchMessage(msg, handler);
	}
}


bool BackgroundWin::QuitRequested() {
	UT_ASSERT(m_DlgBackground);

	delete_sem(modalSem);
	
	return(false);
}
