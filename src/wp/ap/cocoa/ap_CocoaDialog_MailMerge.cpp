/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
 * Copyright (C) 2005 Francis James Franklin
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

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "xap_Strings.h"

#include "ap_Dialog_Id.h"
#include "ap_Dialog_MailMerge.h"
#include "ap_Strings.h"

#include "ap_CocoaDialog_MailMerge.h"

/*****************************************************************/

XAP_Dialog * AP_CocoaDialog_MailMerge::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
	AP_CocoaDialog_MailMerge * p = new AP_CocoaDialog_MailMerge(pFactory, dlgid);
	return p;
}

AP_CocoaDialog_MailMerge::AP_CocoaDialog_MailMerge(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
	AP_Dialog_MailMerge(pDlgFactory, dlgid),
	m_dlg(nil)
{
	// 
}

AP_CocoaDialog_MailMerge::~AP_CocoaDialog_MailMerge(void)
{
	destroy();
}

void AP_CocoaDialog_MailMerge::runModeless(XAP_Frame * /*pFrame*/)
{
	m_dlg = [[AP_CocoaDialog_MailMerge_Controller alloc] initFromNib];
	if (m_dlg)
	{
		[m_dlg setXAPOwner:this];
		[m_dlg window];

		// TODO

		// Save dialog the ID number and pointer to the widget
		UT_sint32 sid = (UT_sint32) getDialogId();
		m_pApp->rememberModelessId(sid, (XAP_Dialog_Modeless *) m_pDialog);
		XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
		if (pFrame)
		{
			setActiveFrame(pFrame);
			init();
		}

		activate();
	}
}

void AP_CocoaDialog_MailMerge::activate(void)
{
	if (m_dlg)
	{
		[m_dlg windowToFront];
		[m_dlg updateAvailableFields];
	}
}

void AP_CocoaDialog_MailMerge::destroy(void)
{
	if (m_dlg)
	{
		[m_dlg close];
		[m_dlg release];

		modeless_cleanup();
	}
	m_dlg = 0;
}

void AP_CocoaDialog_MailMerge::eventInsert(NSString * field_name)
{
	UT_UTF8String name([field_name UTF8String]);

	setMergeField(name);
	addClicked();
}

void AP_CocoaDialog_MailMerge::setFieldList()
{
	if (m_dlg)
	{
		[m_dlg updateAvailableFields];
	}
}

@implementation AP_CocoaDialog_MailMerge_Controller

- (id)initFromNib
{
	if (![super initWithWindowNibName:@"ap_CocoaDialog_MailMerge"]) {
		return nil;
	}
	_xap = NULL;

	m_AvailableFields = [[NSMutableArray alloc] initWithCapacity:32];
	if (!m_AvailableFields)
	{
		[self release];
		return nil;
	}
	return self;
}

- (void)dealloc
{
	if (m_AvailableFields)
		{
			[m_AvailableFields release];
			m_AvailableFields = 0;
		}
	[super dealloc];
}

- (void)setXAPOwner:(XAP_Dialog *)owner
{
	_xap = static_cast<AP_CocoaDialog_MailMerge *>(owner);
}

- (void)discardXAP
{
	_xap = 0;
}

- (void)windowDidLoad
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	LocalizeControl([self window],    pSS, AP_STRING_ID_DLG_MailMerge_MailMergeTitle);

	LocalizeControl(oAvailableFields, pSS, AP_STRING_ID_DLG_MailMerge_AvailableFields);
	LocalizeControl(oFieldNameCell,   pSS, AP_STRING_ID_DLG_MailMerge_Insert);
	LocalizeControl(oOpenFile,        pSS, AP_STRING_ID_DLG_MailMerge_OpenFile);
	LocalizeControl(oClose,           pSS,XAP_STRING_ID_DLG_Close);
	LocalizeControl(oInsert,          pSS, AP_STRING_ID_DLG_InsertButton);

	[oFieldsTable setDataSource:self];
	[oFieldsTable setDelegate:self];
}

- (void)windowToFront
{
	[[self window] makeKeyAndOrderFront:self];
	[[self window] makeFirstResponder:oFieldName];
}

- (IBAction)aFieldsTable:(id)sender
{
	UT_UNUSED(sender);
	// 
}

- (IBAction)aFieldName:(id)sender
{
	[self aInsert:sender];
}

- (IBAction)aOpenFile:(id)sender
{
	UT_UNUSED(sender);
	if (_xap)
		if (XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame())
			{
				_xap->setActiveFrame(pFrame);
				_xap->eventOpen();
			}
}

- (IBAction)aClose:(id)sender
{
	UT_UNUSED(sender);
	if (_xap)
		_xap->destroy();
}

- (IBAction)aInsert:(id)sender
{
	UT_UNUSED(sender);
	NSString * field_name = [oFieldName stringValue];

	if ([field_name length])
		if (_xap)
			_xap->eventInsert(field_name);
}

- (void)updateAvailableFields
{
	if (_xap)
		{
			[m_AvailableFields removeAllObjects];

			UT_uint32 count = _xap->fieldCount();

			for (UT_uint32 i = 0; i < count; i++)
				{
					[m_AvailableFields addObject:[NSString stringWithUTF8String:_xap->field(i).c_str()]];
				}
			[oFieldsTable reloadData];
		}
}

/* NSTableViewDataSource methods
 */
- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
	UT_UNUSED(aTableView);
	return (int) [m_AvailableFields count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
	UT_UNUSED(aTableView);
	UT_UNUSED(aTableColumn);
	return [m_AvailableFields objectAtIndex:rowIndex];
}

/* NSTableView delegate methods
 */
- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	int row = [oFieldsTable selectedRow];
	if (row >= 0)
		{
			[oFieldName setStringValue:[m_AvailableFields objectAtIndex:row]];
		}
	else
		{
			[oFieldName setStringValue:@""];
		}
}

- (void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
	UT_UNUSED(aTableView);
	UT_UNUSED(aTableColumn);
	UT_UNUSED(rowIndex);
	[aCell setFont:[NSFont systemFontOfSize:10.0f]];
}

@end
