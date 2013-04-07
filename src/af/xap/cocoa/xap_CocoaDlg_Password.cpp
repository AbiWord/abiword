/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_CocoaDlg_Password.h"

/*****************************************************************/
void XAP_CocoaDialog_Password::event_Ok ()
{
	UT_UTF8String pass ([[m_dlg password] UTF8String]);
	
	UT_DEBUGMSG(("ok: %s\n", pass.utf8_str()));
	
	setPassword (pass);
	setAnswer(XAP_Dialog_Password::a_OK);
	[NSApp stopModal];
}

void XAP_CocoaDialog_Password::event_Cancel ()
{
	UT_DEBUGMSG(("cancel\n"));
	setAnswer(XAP_Dialog_Password::a_Cancel);
	[NSApp stopModal];
}

XAP_Dialog * XAP_CocoaDialog_Password::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_Password * p = new XAP_CocoaDialog_Password(pFactory, dlgid);
	return p;
}

XAP_CocoaDialog_Password::XAP_CocoaDialog_Password(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: XAP_Dialog_Password(pDlgFactory, dlgid)
{
}

XAP_CocoaDialog_Password::~XAP_CocoaDialog_Password(void)
{
}

void XAP_CocoaDialog_Password::runModal(XAP_Frame * /*pFrame*/)
{
	m_dlg = [[XAP_CocoaDlg_PasswordController alloc] initFromNib];
	
	// used similarly to convert between text and numeric arguments
	[m_dlg setXAPOwner:this];

	// build the dialog
	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);

	[NSApp runModalForWindow:window];

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}





@implementation XAP_CocoaDlg_PasswordController
- (id)initFromNib
{
	if(![super initWithWindowNibName:@"xap_CocoaDlg_Password"]) {
		return nil;
	}
	return self;
}

-(void)discardXAP
{
	_xap = NULL; 
}

-(void)dealloc
{
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = dynamic_cast<XAP_CocoaDialog_Password*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, XAP_STRING_ID_DLG_Password_Title);
		LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(_passwordLabel, pSS, XAP_STRING_ID_DLG_Password_Password);
	}
}

- (NSString*)password
{
	return [_passwordData stringValue];
}

- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Cancel ();
}

- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Ok ();
}

@end
