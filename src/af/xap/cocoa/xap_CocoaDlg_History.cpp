/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2004 Martin Sevior <msevior@physics.unimelb.edu.au>
 * Copyright (C) 2004, 2009 Hubert Figuiere
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
#include "xap_Frame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_History.h"
#include "xap_CocoaDlg_History.h"


/*****************************************************************/

XAP_Dialog * XAP_CocoaDialog_History::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_History * p = new XAP_CocoaDialog_History(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_History::XAP_CocoaDialog_History(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: XAP_Dialog_History(pDlgFactory,dlgid)
{
}

XAP_CocoaDialog_History::~XAP_CocoaDialog_History(void)
{
}

void XAP_CocoaDialog_History::runModal(XAP_Frame * /*pFrame*/)
{
	m_dlg = [[XAP_CocoaDialog_HistoryController alloc] initFromNib];
	
	// used similarly to convert between text and numeric arguments
	[m_dlg setXAPOwner:this];
	
	// build the dialog
	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);
	
	_populateWindowData();  

	[NSApp runModalForWindow:window];

	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}


void XAP_CocoaDialog_History::_populateWindowData(void)
{
	[m_dlg populate];
}

void XAP_CocoaDialog_History::event_OK()
{
	m_answer = a_OK;
	[NSApp stopModal];
}


void XAP_CocoaDialog_History::event_Cancel()
{
	m_answer = a_CANCEL;
	[NSApp stopModal];
}


@implementation XAP_CocoaDialog_HistoryController
- (id)initFromNib
{
	return [super initWithWindowNibName:@"xap_CocoaDlg_History"];
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
	_xap = dynamic_cast<XAP_CocoaDialog_History*>(owner);
}

static void addCol(NSTableView *table, XAP_CocoaDialog_History* xap, int idx)
{
	NSTableColumn *col = [[NSTableColumn alloc] 
		initWithIdentifier:[[NSNumber numberWithLong:idx] stringValue]];
	[col setEditable:NO];
	[[col headerCell] setTitle:[NSString stringWithUTF8String:xap->getListHeader(idx)]];
	[table addTableColumn:col];
	[col release];
}


-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, XAP_STRING_ID_DLG_History_WindowLabel);
		LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(_historyBox, pSS, XAP_STRING_ID_DLG_History_DocumentDetails);
		
		addCol(_historyList, _xap, 0);
		addCol(_historyList, _xap, 1);
		addCol(_historyList, _xap, 2);
	}
}


- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Cancel();
}

- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_OK();
}

- (IBAction)historySelect:(id)sender
{
	UT_uint32 item = [sender selectedRow];
	UT_DEBUGMSG(("Value of id selected %d \n",item));
	_xap->setSelectionId(_xap->getListItemId(item));
}


- (void)populate
{
	[_docNameLabel setStringValue:[NSString stringWithUTF8String:_xap->getHeaderLabel(0)]];
	[_docNameData setStringValue:[NSString stringWithUTF8String:_xap->getHeaderValue(0)]];
	[_versionLabel setStringValue:[NSString stringWithUTF8String:_xap->getHeaderLabel(1)]];
	[_versionData setStringValue:[NSString stringWithUTF8String:_xap->getHeaderValue(1)]];
	[_createdLabel setStringValue:[NSString stringWithUTF8String:_xap->getHeaderLabel(2)]];
	[_createdData setStringValue:[NSString stringWithUTF8String:_xap->getHeaderValue(2)]];
	[_lastSavedLabel setStringValue:[NSString stringWithUTF8String:_xap->getHeaderLabel(3)]];
	[_lastSavedData setStringValue:[NSString stringWithUTF8String:_xap->getHeaderValue(3)]];
	[_editTimeLabel setStringValue:[NSString stringWithUTF8String:_xap->getHeaderLabel(4)]];
	[_editTimeData setStringValue:[NSString stringWithUTF8String:_xap->getHeaderValue(4)]];
	[_identifierLabel setStringValue:[NSString stringWithUTF8String:_xap->getHeaderLabel(5)]];
	[_identifierData setStringValue:[NSString stringWithUTF8String:_xap->getHeaderValue(5)]];
	[_historyBox setTitle:[NSString stringWithUTF8String:_xap->getListTitle()]];
	[_historyList setDataSource:self];	
}

- (id)tableView:(NSTableView *)aTableView
    objectValueForTableColumn:(NSTableColumn *)aTableColumn
    row:(int)rowIndex
{
	UT_UNUSED(aTableView);
	id i = [aTableColumn identifier];
	int col = [ i longValue];
	NSString *s = [NSString stringWithUTF8String:_xap->getListValue(rowIndex, col)];

	return s;
}


- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
	UT_UNUSED(aTableView);
	return _xap->getListItemCount();
}


@end

