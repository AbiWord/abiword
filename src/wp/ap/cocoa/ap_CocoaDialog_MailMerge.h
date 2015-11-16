/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef AP_COCOADIALOG_MAILMERGE_H
#define AP_COCOADIALOG_MAILMERGE_H

#include <string>

#import <Cocoa/Cocoa.h>

#include "xap_CocoaDialog_Utilities.h"

#include "ap_Dialog_MailMerge.h"

class XAP_CocoaFrame;

/*****************************************************************/

@class AP_CocoaDialog_MailMerge_Controller;

class AP_CocoaDialog_MailMerge: public AP_Dialog_MailMerge
{
public:
	AP_CocoaDialog_MailMerge(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);

	virtual ~AP_CocoaDialog_MailMerge(void);

	virtual void			runModeless(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void			destroy(void);
	virtual void			activate(void);

	void					eventInsert(NSString * field_name);

	virtual void			setFieldList();

	UT_uint32 fieldCount() const
	{
		return m_vecFields.size();
	}
	const std::string & field(UT_uint32 index)
	{
		return m_vecFields[index];
	}

protected:
	AP_CocoaDialog_MailMerge_Controller *	m_dlg;
};

@interface AP_CocoaDialog_MailMerge_Controller
	: NSWindowController <XAP_CocoaDialogProtocol, NSTableViewDataSource, NSTableViewDelegate>
{
	IBOutlet NSTextField *	oAvailableFields;

	IBOutlet NSTableView *	oFieldsTable;

	IBOutlet NSForm *		oFieldName;
	IBOutlet NSFormCell *	oFieldNameCell;

	IBOutlet NSButton *		oOpenFile;
	IBOutlet NSButton *		oClose;
	IBOutlet NSButton *		oInsert;

	NSMutableArray *		m_AvailableFields;

	AP_CocoaDialog_MailMerge *	_xap;
}
- (void)dealloc;

- (void)windowToFront;

- (IBAction)aFieldsTable:(id)sender;
- (IBAction)aFieldName:(id)sender;
- (IBAction)aOpenFile:(id)sender;
- (IBAction)aClose:(id)sender;
- (IBAction)aInsert:(id)sender;

- (void)updateAvailableFields;

/* NSTableViewDataSource methods
 */
- (int)numberOfRowsInTableView:(NSTableView *)aTableView;
- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;

/* NSTableView delegate methods
 */
- (void)tableViewSelectionDidChange:(NSNotification *)aNotification;
- (void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;
@end

#endif /* AP_COCOADIALOG_MAILMERGE_H */
