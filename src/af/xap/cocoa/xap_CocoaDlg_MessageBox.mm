/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>

#include "ut_assert.h"
#include "ut_vector.h"
#include "xap_CocoaDlg_MessageBox.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "xap_CocoaDialog_Utilities.h"

// default GTK message box button width, in GTK screen units (pixels)
#define DEFAULT_BUTTON_WIDTH	85

/*****************************************************************/
XAP_Dialog * XAP_CocoaDialog_MessageBox::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_MessageBox * p = new XAP_CocoaDialog_MessageBox(pFactory, dlgid);
	return p;
}

XAP_CocoaDialog_MessageBox::XAP_CocoaDialog_MessageBox(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id dlgid)
	: XAP_Dialog_MessageBox(pDlgFactory, dlgid), m_dlg (nil)
{
}

XAP_CocoaDialog_MessageBox::~XAP_CocoaDialog_MessageBox(void)
{
	if (m_dlg != nil) {
		m_dlg = nil;
	}
}

/*****************************************************************/
void XAP_CocoaDialog_MessageBox::_setAnswer(XAP_Dialog_MessageBox::tAnswer answer)
{
	m_answer = answer;
}

void XAP_CocoaDialog_MessageBox::runModal(XAP_Frame * pFrame)
{
	m_pCocoaFrame = (XAP_CocoaFrame *)pFrame;
	UT_ASSERT(m_pCocoaFrame);
	XAP_CocoaApp * pApp = (XAP_CocoaApp *)m_pCocoaFrame->getApp();
	UT_ASSERT(pApp);

	const char * szCaption = pApp->getApplicationTitleForTitleBar();

	m_dlg = [XAP_CocoaDlg_MessageBoxController loadFromNibWithButtons:m_buttons];	// autoreleased
	[m_dlg setXAPOwner:this];
	NSWindow *win = [m_dlg window];		// force the window to be loaded.
	[m_dlg setMessage:[NSString stringWithCString:m_szMessage]];	// string autoreleased



	[NSApp runModalForWindow:win];

	m_pCocoaFrame = NULL;
}



@implementation XAP_CocoaDlg_MessageBoxController

+ (XAP_CocoaDlg_MessageBoxController *)loadFromNibWithButtons:(XAP_Dialog_MessageBox::tButtons)buttons
{
	XAP_CocoaDlg_MessageBoxController * box = [[XAP_CocoaDlg_MessageBoxController alloc] initWithWindowNibName:@"xap_CocoaDlg_MessageBox"];
	[box setButtons:buttons];
	return [box autorelease];
}

- (void)windowDidLoad
{
	XAP_CocoaFrame *pFrame = m_xap->_getFrame ();
	// we get all our strings from the application string set
	const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
	LocalizeControl (m_okBtn, pSS, XAP_STRING_ID_DLG_OK);
	LocalizeControl (m_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
	LocalizeControl (m_yesBtn, pSS, XAP_STRING_ID_DLG_UnixMB_Yes);
	LocalizeControl (m_noBtn, pSS, XAP_STRING_ID_DLG_UnixMB_No);

	switch (m_buttons)
	{
	case XAP_Dialog_MessageBox::b_O:
		[[m_cancelBtn retain] removeFromSuperview];
		[[m_yesBtn retain] removeFromSuperview];
		[[m_noBtn retain] removeFromSuperview];
		[[self window] makeFirstResponder:m_okBtn];
		break;

	case XAP_Dialog_MessageBox::b_OC:
		[[m_yesBtn retain] removeFromSuperview];
		[[m_noBtn retain] removeFromSuperview];
		[[self window] makeFirstResponder:m_cancelBtn];
		break;

	case XAP_Dialog_MessageBox::b_YN:
		[[m_okBtn retain] removeFromSuperview];
		[[m_cancelBtn retain] removeFromSuperview];
		[[self window] makeFirstResponder:m_noBtn];
		break;
	case XAP_Dialog_MessageBox::b_YNC:
		[[m_okBtn retain] removeFromSuperview];
		[[self window] makeFirstResponder:m_cancelBtn];
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
}


- (void)setButtons:(XAP_Dialog_MessageBox::tButtons)buttons
{
	m_buttons = buttons;
}


/*
	Set owner (XAP class) so that IBAction can do something...
 */
- (void)setXAPOwner:(XAP_CocoaDialog_MessageBox *)owner
{
	m_xap = owner;
}


- (void)setOkBtnLabel:(NSString *)label
{
	[m_okBtn setTitle:label];
}


- (void)setCancelBtnLabel:(NSString *)label
{
	[m_cancelBtn setTitle:label];
}


- (void)setYesBtnLabel:(NSString *)label
{
	[m_yesBtn setTitle:label];
}


- (void)setNoBtnLabel:(NSString *)label
{
	[m_noBtn setTitle:label];
}

- (void)setMessage:(NSString *)message
{
	[m_messageField setStringValue:message];
}

- (IBAction)okAction:(id)sender
{
	UT_ASSERT (m_xap);
	m_xap->_setAnswer (XAP_Dialog_MessageBox::a_OK);
	[NSApp stopModal];
}

- (IBAction)cancelAction:(id)sender
{
	UT_ASSERT (m_xap);
	m_xap->_setAnswer (XAP_Dialog_MessageBox::a_CANCEL);
	[NSApp stopModal];
}

- (IBAction)yesAction:(id)sender
{
	UT_ASSERT (m_xap);
	m_xap->_setAnswer (XAP_Dialog_MessageBox::a_YES);
	[NSApp stopModal];
}

- (IBAction)noAction:(id)sender
{
	UT_ASSERT (m_xap);
	m_xap->_setAnswer (XAP_Dialog_MessageBox::a_NO);
	[NSApp stopModal];
}

@end
