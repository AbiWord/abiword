/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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
#include "ap_Dialog_InsertHyperlink.h"
#include "ap_CocoaDialog_InsertHyperlink.h"


@interface AP_CocoaDialog_InsertHyperlinkController 
: NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *_addBtn;
    IBOutlet NSTextField *_hyperlinkValue;
    IBOutlet NSBox *_hyperlinkLabel;
	IBOutlet NSTableView *_bookmarkList;
    IBOutlet NSButton *_cancelBtn;
	AP_CocoaDialog_InsertHyperlink* _xap;
	XAP_StringListDataSource *_datasource;
}
- (void)setDataSource:(XAP_StringListDataSource*)datasource;
- (NSString*) bookmarkText;
- (IBAction)addBtn:(id)sender;
- (IBAction)cancelBtn:(id)sender;
- (IBAction)selectBtn:(id)sender;
@end


/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_InsertHyperlink::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_InsertHyperlink * p = new AP_CocoaDialog_InsertHyperlink(pFactory, dlgid);
	return p;
}

AP_CocoaDialog_InsertHyperlink::AP_CocoaDialog_InsertHyperlink(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_InsertHyperlink(pDlgFactory, dlgid),
		m_pBookmarks(nil),
		m_dlg(nil)
{
}

AP_CocoaDialog_InsertHyperlink::~AP_CocoaDialog_InsertHyperlink(void)
{
}


/***********************************************************************/
void AP_CocoaDialog_InsertHyperlink::runModal(XAP_Frame * /*pFrame*/)
{

	m_dlg = [[AP_CocoaDialog_InsertHyperlinkController alloc] initFromNib];
	
	// used similarly to convert between text and numeric arguments
	[m_dlg setXAPOwner:this];

	// build the dialog
	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);
	m_pBookmarks = [[XAP_StringListDataSource alloc] init];
  	for (int i = 0; i < (int)getExistingBookmarksCount(); i++) {
	  [m_pBookmarks addString:[NSString stringWithUTF8String:getNthExistingBookmark(i).c_str()]];
	}
	[m_dlg setDataSource:m_pBookmarks];
	[NSApp runModalForWindow:window];

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	[m_pBookmarks release];
	m_dlg = nil;
}

void AP_CocoaDialog_InsertHyperlink::event_OK(void)
{
	NSString * str = [m_dlg bookmarkText];
	if(str)	{
		setAnswer(AP_Dialog_InsertHyperlink::a_OK);
		setHyperlink((gchar*)[str UTF8String]);
	}
	else {
		setAnswer(AP_Dialog_InsertHyperlink::a_CANCEL);
	}
		
	[NSApp stopModal];
}

void AP_CocoaDialog_InsertHyperlink::event_Cancel(void)
{
	setAnswer(AP_Dialog_InsertHyperlink::a_CANCEL);
	[NSApp stopModal];
}


@implementation AP_CocoaDialog_InsertHyperlinkController

- (id)initFromNib
{
  if((self = [super initWithWindowNibName:@"ap_CocoaDialog_InsertHyperlink"])) {

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
	_xap = dynamic_cast<AP_CocoaDialog_InsertHyperlink*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, AP_STRING_ID_DLG_InsertHyperlink_Title);
		LocalizeControl(_addBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(_hyperlinkLabel, pSS, AP_STRING_ID_DLG_InsertHyperlink_Msg);
		const gchar* href = _xap->getHyperlink();
		if (href && *href) {
			if (*href == '#') {
				[_hyperlinkValue setStringValue:[NSString stringWithUTF8String:(href + 1)]];
			}
			else {
				[_hyperlinkValue setStringValue:[NSString stringWithUTF8String:href]];
			}
		}
		[_bookmarkList setAction:@selector(selectBtn:)];
	}
}

- (void)setDataSource:(XAP_StringListDataSource*)datasource
{
	[_bookmarkList setDataSource:datasource];
	[_bookmarkList deselectAll:self];
	_datasource = datasource;
}

- (NSString*) bookmarkText
{
	return [_hyperlinkValue stringValue];
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

- (IBAction)selectBtn:(id)sender
{
	UT_UNUSED(sender);
	int row = [_bookmarkList selectedRow];
	[_hyperlinkValue setStringValue:[[_datasource array] objectAtIndex:row]];
}

@end

