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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "xap_BeOSDlg_FontChooser.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "ut_Rehydrate.h"

/*****************************************************************/
class FontWin:public BWindow {
	public:
		FontWin(BMessage *data);
		void SetDlg(XAP_BeOSDialog_FontChooser *font);
		virtual void DispatchMessage(BMessage *msg, BHandler *handler);
		virtual bool QuitRequested(void);
		
	private:
		int 			spin;
		XAP_BeOSDialog_FontChooser *m_FontChooser;
};

FontWin::FontWin(BMessage *data) 
	  :BWindow(data) {
	spin = 1;	
} //FontWin::FontWin

void FontWin::SetDlg(XAP_BeOSDialog_FontChooser *font) {
	m_FontChooser = font;

//	We need to tie up the caller thread for a while ...
	Show();
	while (spin) { snooze(1); }
	Hide();
}

void FontWin::DispatchMessage(BMessage *msg, BHandler *handler) {
	switch(msg->what) {
	default:
		BWindow::DispatchMessage(msg, handler);
	}
} 

//Behave like a good citizen
bool FontWin::QuitRequested() {
/*
	UT_ASSERT(m_DlgFont);
	m_DlgFont->setAnswer(AP_Dialog_Font::a_CANCEL);
*/
	spin = 0;
	return(true);
}

/*****************************************************************/

XAP_Dialog * XAP_BeOSDialog_FontChooser::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	XAP_BeOSDialog_FontChooser * p = new XAP_BeOSDialog_FontChooser(pFactory,id);
	return p;
}

XAP_BeOSDialog_FontChooser::XAP_BeOSDialog_FontChooser(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: XAP_Dialog_FontChooser(pDlgFactory,id)
{
	bAbusingTheFontSize = UT_FALSE;
}

XAP_BeOSDialog_FontChooser::~XAP_BeOSDialog_FontChooser(void)
{
}

void XAP_BeOSDialog_FontChooser::runModal(XAP_Frame * pFrame)
{
	BMessage msg;
	FontWin  *newwin;
	if (RehydrateWindow("FontWindow", &msg)) {
                newwin = new FontWin(&msg);
		newwin->SetDlg(this);
		//Take the information here ...
		newwin->Close();
        }                                                
}
