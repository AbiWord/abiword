/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
 * Copyright (C) 2001-2003 Hubert Figuiere
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

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "xap_CocoaDialog_Utilities.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_ListRevisions.h"
#include "ap_CocoaDialog_ListRevisions.h"


static const NSString * REVID_COL_ID = @"revid";
static const NSString * DATE_COL_ID = @"date";
static const NSString * COMMENT_COL_ID = @"comment";

@interface AP_ListRevisions_DataSource : NSObject<NSTableViewDataSource> {
	NSMutableArray*		_array;
}
- (id)init;

- (oneway void)dealloc;

- (void)addLine:(const char*)col1 withCol2:(const char*)col2 withCol3:(const char*)col3;
/* NSTableDataSource */
- (int)numberOfRowsInTableView:(NSTableView *)tableView;
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row;
@end

@implementation AP_ListRevisions_DataSource

- (id)init
{
	if(self = [super init]) {
		_array = [[NSMutableArray alloc] init];
	}
	return self;
}

- (oneway void)dealloc
{
	[_array release];
	[super dealloc];
}

- (void)addLine:(const char*)col1 withCol2:(const char*)col2 withCol3:(const char*)col3
{
	if (col1 == NULL) {
		col1 = "";
	}
	if (col2 == NULL) {
		col2 = "";		
	}
	if (col3 == NULL) {
		col3 = "";
	}
	NSArray * line = [NSArray arrayWithObjects:[NSString stringWithUTF8String:col1],
		[NSString stringWithUTF8String:col2], [NSString stringWithUTF8String:col3], nil];
	[_array addObject:line];
}


/* NSTableDataSource */
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
	UT_UNUSED(tableView);
	return [_array count];
}


- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
	UT_UNUSED(tableView);
	int idx = -1;
	NSArray *line = [_array objectAtIndex:row];
	
	if ([REVID_COL_ID isEqualToString:[tableColumn identifier]]) {
		idx = 0;
	}
	else if ([DATE_COL_ID isEqualToString:[tableColumn identifier]]) {
		idx = 1;
	}
	else if ([COMMENT_COL_ID isEqualToString:[tableColumn identifier]]) {
		idx = 2;	
	}
	else {
		return nil;
	}
	return [line objectAtIndex:idx];
}

@end

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_ListRevisions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_ListRevisions * p = new AP_CocoaDialog_ListRevisions(pFactory,dlgid);
	return p;
}

AP_CocoaDialog_ListRevisions::AP_CocoaDialog_ListRevisions(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id dlgid)
	: AP_Dialog_ListRevisions(pDlgFactory,dlgid),
		m_dlg(nil),
		m_dataSource(nil)
{
}

AP_CocoaDialog_ListRevisions::~AP_CocoaDialog_ListRevisions(void)
{
	[m_dataSource release];
}


void AP_CocoaDialog_ListRevisions::event_Cancel()
{
	m_iId = 0 ;
	m_answer = AP_Dialog_ListRevisions::a_CANCEL ;
	[NSApp stopModal];
}

void AP_CocoaDialog_ListRevisions::event_OK()
{
	m_answer = AP_Dialog_ListRevisions::a_OK ;
	[NSApp stopModal];
}

void AP_CocoaDialog_ListRevisions::event_Select(int idx)
{
	if (idx >= 0) {
		m_iId = getNthItemId(idx);
	}
	else {
		m_iId = 0;
	}
}


void AP_CocoaDialog_ListRevisions::runModal(XAP_Frame * /*pFrame*/)
{
	/*
	   see the screenshot posted to the dev-list (25/05/2002);
	   use the provided functions getTitle(), getLabel1(),
	   getColumn1Label(), getColumn2Label(), getItemCount(),
	   getNthItemId() and getNthItemText() to fill the list

	   if the user clicks OK but there is no selection, set m_iId to 0
           otherwise set m_iId to the id of the selected revision
	*/
	m_dlg = [[AP_CocoaDialog_ListRevisionsController alloc] initFromNib];
	
	[m_dlg setXAPOwner:this];

	NSWindow * window = [m_dlg window];
	UT_ASSERT(window);
	m_dataSource = [[AP_ListRevisions_DataSource alloc] init];
	
	UT_uint32 itemCnt = getItemCount () ;
	for (UT_uint32 i = 0; i < itemCnt; i++) {
		char buf [35] ;
		const char *text;
		const char *time;
		
		snprintf(buf, sizeof(buf), "%d", getNthItemId(i)) ;
		text = static_cast<const char*>(getNthItemText(i));
		time = static_cast<const char*>(getNthItemTime(i));
		[m_dataSource addLine:buf withCol2:time	withCol3:text];
		FREEP(text);
		//FREEP(time);	
	}
	[m_dlg setDataSource:m_dataSource];

	[NSApp runModalForWindow:window];

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
	[m_dataSource release];
	m_dataSource = nil;
}

@implementation AP_CocoaDialog_ListRevisionsController

- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_ListRevisions"];
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
	_xap = dynamic_cast<AP_CocoaDialog_ListRevisions*>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		NSArray *columns;
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
		LocalizeControl([self window], pSS, AP_STRING_ID_DLG_MarkRevisions_Title);
		[[self window] setTitle:[NSString stringWithUTF8String:_xap->getTitle()]];
		LocalizeControl(_okBtn, pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn, pSS, XAP_STRING_ID_DLG_Cancel);
		[_label setStringValue:[NSString stringWithUTF8String:_xap->getLabel1()]];
		columns = [_list tableColumns];
		[[[columns objectAtIndex:0] headerCell] setStringValue:[NSString stringWithUTF8String:_xap->getColumn1Label()]];
		[[[columns objectAtIndex:1] headerCell] setStringValue:[NSString stringWithUTF8String:_xap->getColumn2Label()]];
		[[[columns objectAtIndex:2] headerCell] setStringValue:[NSString stringWithUTF8String:_xap->getColumn3Label()]];
		[_list setAction:@selector(listAction:)];
		[_list setDoubleAction:@selector(okAction:)];
		[_list setTarget:self];
	}
}


- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Cancel();
}

- (IBAction)listAction:(id)sender
{
	int idx = [sender selectedRow];
	_xap->event_Select(idx);
}

- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_OK();
}

- (void)setDataSource:(AP_ListRevisions_DataSource*)ds
{
	[_list setDataSource:ds];
}

@end

