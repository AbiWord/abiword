/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003, 2009, 2011 Hubert Figuiere
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
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_CocoaDialog_Utilities.h"

#include "xap_Dialog_Id.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Goto.h"
#include "ap_CocoaDialog_Goto.h"

#include "fv_View.h"



/*****************************************************************/
XAP_Dialog * AP_CocoaDialog_Goto::static_constructor(XAP_DialogFactory * pFactory,
													XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_Goto * p = new AP_CocoaDialog_Goto(pFactory, dlgid);
	return p;
}

AP_CocoaDialog_Goto::AP_CocoaDialog_Goto(XAP_DialogFactory * pDlgFactory,
									   XAP_Dialog_Id dlgid)
	: AP_Dialog_Goto(pDlgFactory, dlgid),
		m_dlg(nil)
{
}

AP_CocoaDialog_Goto::~AP_CocoaDialog_Goto(void)
{

}

void AP_CocoaDialog_Goto::runModeless (XAP_Frame * /*pFrame*/)
{
	m_dlg = [[AP_CocoaDialog_GotoController alloc] initFromNib];

	[m_dlg setXAPOwner:this];

	ConstructWindowName();
	[[m_dlg window] setTitle:[NSString stringWithUTF8String:getWindowName()]];

	[m_dlg windowToFront];

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid = (UT_sint32) getDialogId ();
	m_pApp->rememberModelessId(sid, (XAP_Dialog_Modeless *) m_pDialog);
}

void AP_CocoaDialog_Goto::destroy (void)
{
	modeless_cleanup();
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

void AP_CocoaDialog_Goto::activate (void)
{
	ConstructWindowName();
	[[m_dlg window] setTitle:[NSString stringWithUTF8String:getWindowName()]];

	[m_dlg windowToFront];
}

void AP_CocoaDialog_Goto::notifyActiveFrame(XAP_Frame * /*pFrame*/)
{
	ConstructWindowName();
	[[m_dlg window] setTitle:[NSString stringWithUTF8String:getWindowName()]];
}


@implementation AP_CocoaDialog_GotoController

- (id)initFromNib
{
	if((self = [super initWithWindowNibName:@"ap_CocoaDialog_Goto"])) {
		_xap = NULL;
		m_jumpTarget = AP_JUMPTARGET_PAGE;
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
	_xap = dynamic_cast<AP_CocoaDialog_Goto*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

		LocalizeControl(jumpToBtn,	pSS, AP_STRING_ID_DLG_Goto_Btn_Goto);
		LocalizeControl(forwardBtn,	pSS, AP_STRING_ID_DLG_Goto_Btn_Next);
		LocalizeControl(backBtn,	pSS, AP_STRING_ID_DLG_Goto_Btn_Prev);
		LocalizeControl(whatLabel,	pSS, AP_STRING_ID_DLG_Goto_Label_What);

		id widgets[3] = { _pageRadio, _lineRadio, _bookmarkRadio };

		if (const char ** tmp2 = _xap->getJumpTargets()) {
			for (int i = 0; tmp2[i] != NULL && i < 3; i++) {
				[widgets[i] setTitle:[NSString stringWithUTF8String:tmp2[i]]];
			}
		}
	
		[self updateContent];
	}
}

- (void)updateContent
{
	UT_ASSERT(_xap);

	UT_sint32 count = _xap->getExistingBookmarksCount();
	[_bookmarkName removeAllItems];
	if(count <= 10) {
		[_bookmarkName setNumberOfVisibleItems:count];
	}
	else {
		[_bookmarkName setNumberOfVisibleItems:10];
		[_bookmarkName setHasVerticalScroller:YES];
	}
	for(UT_sint32 i = 0; i < count; i++) {
		[_bookmarkName addItemWithObjectValue:[NSString stringWithUTF8String:_xap->getNthExistingBookmark(i).c_str()]];
	}
	if(count) {
		[_bookmarkName selectItemAtIndex:0];
	}
	
	m_docCount = _xap->getView()->countWords(false);
	NSNumberFormatter *formatter;
	formatter = [_pageNum formatter];
	[formatter setMinimum:[NSNumber numberWithInt:1]];
	[formatter setMaximum:[NSNumber numberWithInt:m_docCount.page]];
	[_pageNum setIntValue:1];
	formatter = [_lineNum formatter];
	[formatter setMinimum:[NSNumber numberWithInt:1]];
	[formatter setMaximum:[NSNumber numberWithInt:m_docCount.line]];
	[_lineNum setIntValue:1];

	[self selectedType:_typeMatrix];
}


- (void)windowToFront
{
	[[self window] makeKeyAndOrderFront:self];
	id w = nil;
	switch(m_jumpTarget) {
	case AP_JUMPTARGET_PAGE:
		w = _pageNum;
		break;
	case AP_JUMPTARGET_LINE:
		w = _lineNum;
		break;
	case AP_JUMPTARGET_BOOKMARK:
		w = _bookmarkName;
		break;
	default:
		break;
	}
	[[self window] makeFirstResponder:w];
}


- (IBAction)backAction:(id)sender
{
	UT_UNUSED(sender);
	int idx = -1;
	if(m_jumpTarget == AP_JUMPTARGET_BOOKMARK) {
		idx = [_bookmarkName indexOfItemWithObjectValue:[_bookmarkName stringValue]];
	}
	std::string dest = _xap->performGotoPrev(m_jumpTarget, idx);
	if(m_jumpTarget == AP_JUMPTARGET_BOOKMARK) {
		[_bookmarkName setStringValue:[NSString stringWithUTF8String:dest.c_str()]];
	}
}

- (IBAction)forwardAction:(id)sender
{
	UT_UNUSED(sender);
	int idx = -1;
	if(m_jumpTarget == AP_JUMPTARGET_BOOKMARK) {
		idx = [_bookmarkName indexOfItemWithObjectValue:[_bookmarkName stringValue]];
	}
	std::string dest = _xap->performGotoNext(m_jumpTarget, idx);
	if(m_jumpTarget == AP_JUMPTARGET_BOOKMARK) {
		[_bookmarkName setStringValue:[NSString stringWithUTF8String:dest.c_str()]];
	}
}

- (IBAction)jumpToAction:(id)sender
{
	UT_UNUSED(sender);
	switch(m_jumpTarget) {
	case AP_JUMPTARGET_PAGE:
		_xap->performGoto(m_jumpTarget, [[_pageNum stringValue] UTF8String]);
		break;
	case AP_JUMPTARGET_LINE:
		_xap->performGoto(m_jumpTarget, [[_lineNum stringValue] UTF8String]);
		break;
	case AP_JUMPTARGET_BOOKMARK:
		_xap->performGoto(m_jumpTarget, [[_bookmarkName stringValue] UTF8String]);
		break;
	default:
		break;
	}
}

- (IBAction)selectedType:(id)sender
{
	int selected = [sender selectedRow];
	switch(selected) {
	case 0:
		[_pageNum setEnabled:YES];
		[_lineNum setEnabled:NO];
		[_bookmarkName setEnabled:NO];
		m_jumpTarget = AP_JUMPTARGET_PAGE;
		break;
	case 1:
		[_pageNum setEnabled:NO];
		[_lineNum setEnabled:YES];
		[_bookmarkName setEnabled:NO];	
		m_jumpTarget = AP_JUMPTARGET_LINE;
		break;
	case 2:
		[_pageNum setEnabled:NO];
		[_lineNum setEnabled:NO];
		[_bookmarkName setEnabled:YES];
		m_jumpTarget = AP_JUMPTARGET_BOOKMARK;
		break;
	default:
		break;
	}
}

@end
