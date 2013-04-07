/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2003,2011 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_InsertBookmark.h"
#include "ap_CocoaDialog_InsertBookmark.h"

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_InsertBookmark::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_InsertBookmark * p = new AP_CocoaDialog_InsertBookmark(pFactory, dlgid);
	return p;
}

AP_CocoaDialog_InsertBookmark::AP_CocoaDialog_InsertBookmark(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_InsertBookmark(pDlgFactory,dlgid),
		m_dlg(nil)
{
}

AP_CocoaDialog_InsertBookmark::~AP_CocoaDialog_InsertBookmark(void)
{
}

/*****************************************************************/
void AP_CocoaDialog_InsertBookmark::runModal(XAP_Frame * /*pFrame*/)
{

	m_dlg = [[AP_CocoaDialog_InsertBookmarkController alloc] initFromNib];
	
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

void AP_CocoaDialog_InsertBookmark::event_OK(void)
{
	NSString * str = [m_dlg bookmarkText];
	if(str)	{
		setAnswer(AP_Dialog_InsertBookmark::a_OK);
		setBookmark([str UTF8String]);
	}
	else {
		setAnswer(AP_Dialog_InsertBookmark::a_CANCEL);
	}
		
	[NSApp stopModal];
}

void AP_CocoaDialog_InsertBookmark::event_Cancel(void)
{
	setAnswer(AP_Dialog_InsertBookmark::a_CANCEL);
	[NSApp stopModal];
}


@implementation AP_CocoaDialog_InsertBookmarkController

- (id)initFromNib
{
	if((self = [super initWithWindowNibName:@"ap_CocoaDialog_InsertBookmark"])) {

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
	_xap = dynamic_cast<AP_CocoaDialog_InsertBookmark*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, AP_STRING_ID_DLG_InsertBookmark_Title);
		LocalizeControl(_addBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(_bookmarkLabel, pSS, AP_STRING_ID_DLG_InsertBookmark_Msg);
		for(UT_sint32 i = 0; i < _xap->getExistingBookmarksCount(); i++) {
		    [_bookmarkCombo addItemWithObjectValue:[NSString stringWithUTF8String:_xap->getNthExistingBookmark(i).c_str()]];
		}
		[_bookmarkCombo selectItemWithObjectValue:[NSString stringWithUTF8String:_xap->getBookmark()]];
	}
}

- (NSString*) bookmarkText
{
	return [_bookmarkCombo stringValue];
}


- (IBAction)addBtn:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_OK();
}

- (IBAction)cancelBtn:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Cancel();
}


@end
