/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#import <Cocoa/Cocoa.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaDialog_Utilities.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_InsertTable.h"
#include "ap_CocoaDialog_InsertTable.h"

/*****************************************************************/


XAP_Dialog * AP_CocoaDialog_InsertTable::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_InsertTable * p = new AP_CocoaDialog_InsertTable(pFactory, dlgid);
	return p;
}

AP_CocoaDialog_InsertTable::AP_CocoaDialog_InsertTable(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id dlgid)
	: AP_Dialog_InsertTable(pDlgFactory, dlgid)
{
}

AP_CocoaDialog_InsertTable::~AP_CocoaDialog_InsertTable(void)
{
}

void AP_CocoaDialog_InsertTable::runModal(XAP_Frame * /*pFrame*/)
{
	m_dlg = [[AP_CocoaDialog_InsertTableController alloc] initFromNib];
	[m_dlg setXAPOwner:this];

	NSWindow * window = [m_dlg window];

	_populateWindowData();

	[NSApp runModalForWindow:window];

	_storeWindowData();

	[m_dlg discardXAP];
	[m_dlg close];
	[m_dlg release];
	m_dlg = nil;
}

/*****************************************************************/
void AP_CocoaDialog_InsertTable::event_OK()
{
	m_answer = AP_Dialog_InsertTable::a_OK;
	[NSApp stopModal];
}


void AP_CocoaDialog_InsertTable::event_Cancel()
{
	m_answer = AP_Dialog_InsertTable::a_CANCEL;
	[NSApp stopModal];
}



void AP_CocoaDialog_InsertTable::_populateWindowData(void)
{
	// We're (still) a stateless dialog, so there are 
	// no member variables to setyet
}

void AP_CocoaDialog_InsertTable::_storeWindowData(void)
{
	m_columnType = [m_dlg autoSizeType];
	m_numRows = [m_dlg numRows];
	m_numCols = [m_dlg numCols];
	m_columnWidth = [m_dlg colWidth];
}


@implementation AP_CocoaDialog_InsertTableController
- (id)initFromNib
{
	return [super initWithWindowNibName:@"ap_CocoaDialog_InsertTable"];
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
	_xap = dynamic_cast<AP_CocoaDialog_InsertTable *>(owner);
}

-(void)windowDidLoad
{
	if (_xap) {
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

		LocalizeControl([self window],		pSS, AP_STRING_ID_DLG_InsertTable_TableTitle);

		LocalizeControl(_okBtn,				pSS, XAP_STRING_ID_DLG_OK);
		LocalizeControl(_cancelBtn,			pSS, XAP_STRING_ID_DLG_Cancel);
		LocalizeControl(_tableSizeBox,		pSS, AP_STRING_ID_DLG_InsertTable_TableSize);
		LocalizeControl(_numOfColLabel,		pSS, AP_STRING_ID_DLG_InsertTable_NumCols);
		LocalizeControl(_numOfRowLabel,		pSS, AP_STRING_ID_DLG_InsertTable_NumRows);
		LocalizeControl(_autofitBox,		pSS, AP_STRING_ID_DLG_InsertTable_AutoFit);
		LocalizeControl(_autoColBtn,		pSS, AP_STRING_ID_DLG_InsertTable_AutoColSize);
		LocalizeControl(_fixedColSizeBtn,	pSS, AP_STRING_ID_DLG_InsertTable_FixedColSize);

		[_numOfColData setIntValue:_xap->getNumCols()];
		[_numOfRowData setIntValue:_xap->getNumRows()];

		[_autoColBtn      setTag:(int)AP_Dialog_InsertTable::b_AUTOSIZE];
		[_fixedColSizeBtn setTag:(int)AP_Dialog_InsertTable::b_FIXEDSIZE];

		[_fixedColSizeData    setFloatValue:_xap->getColumnWidth()];
		[_fixedColSizeStepper setFloatValue:_xap->getColumnWidth()];

		[_fixedColSizeData    setEnabled:NO];
		[_fixedColSizeStepper setEnabled:NO];
	}
}

- (IBAction)cancelAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_Cancel();
}

- (IBAction)colSizeAction:(id)sender
{
	UT_UNUSED(sender);
	BOOL bEnabled = (AP_Dialog_InsertTable::b_FIXEDSIZE == (AP_Dialog_InsertTable::columnType) [[_radioMatrix selectedCell] tag]) ? YES : NO;

	[_fixedColSizeData    setEnabled:bEnabled];
	[_fixedColSizeStepper setEnabled:bEnabled];
}

- (IBAction)fixedColSizeAction:(id)sender
{
	[_fixedColSizeStepper setFloatValue:[sender floatValue]];
}

- (IBAction)fixedColSizeStepperAction:(id)sender
{
	[_fixedColSizeData setFloatValue:[sender floatValue]];
}

- (IBAction)numColAction:(id)sender
{
	int count = [sender intValue];

	count = (count < 1) ? 1 : ((count > 64) ? 64 : count);

	[_numOfColData    setIntValue:count];
	[_numOfColStepper setIntValue:count];
}

- (IBAction)numColStepperAction:(id)sender
{
	[_numOfColData setIntValue:[sender intValue]];
}

- (IBAction)numRowAction:(id)sender
{
	int count = [sender intValue];

	count = (count < 1) ? 1 : ((count > 500) ? 500 : count);

	[_numOfRowData    setIntValue:count];
	[_numOfRowStepper setIntValue:count];
}

- (IBAction)numRowStepperAction:(id)sender
{
	[_numOfRowData setIntValue:[sender intValue]];
}

- (IBAction)okAction:(id)sender
{
	UT_UNUSED(sender);
	_xap->event_OK();
}

- (int)numRows
{
	int count = [_numOfRowData intValue];

	count = (count < 1) ? 1 : ((count > 500) ? 500 : count);

	return count;
}

- (int)numCols
{
	int count = [_numOfColData intValue];

	count = (count < 1) ? 1 : ((count > 64) ? 64 : count);

	return count;
}

- (float)colWidth
{
	return [_fixedColSizeData floatValue];
}

- (AP_Dialog_InsertTable::columnType)autoSizeType
{
	return (AP_Dialog_InsertTable::columnType)[[_radioMatrix selectedCell] tag];
}

@end
