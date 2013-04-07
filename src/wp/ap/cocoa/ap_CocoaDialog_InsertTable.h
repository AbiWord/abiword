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

#ifndef AP_COCOADIALOG_INSERTTABLE_H
#define AP_COCOADIALOG_INSERTTABLE_H

#import <Cocoa/Cocoa.h>

#include "ap_Dialog_InsertTable.h"

class XAP_CocoaFrame;
@class AP_CocoaDialog_InsertTableController;

/*****************************************************************/

class AP_CocoaDialog_InsertTable: public AP_Dialog_InsertTable
{
public:
	AP_CocoaDialog_InsertTable(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_InsertTable(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	void					event_OK();
	void					event_Cancel();
protected:
	void					_populateWindowData(void);
	void					_storeWindowData(void);
	AP_Dialog_InsertTable::columnType _getActiveRadioItem(void);
private:
	AP_CocoaDialog_InsertTableController*	m_dlg;
};



@interface AP_CocoaDialog_InsertTableController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButtonCell *_autoColBtn;
    IBOutlet NSBox *_autofitBox;
    IBOutlet NSButton *_cancelBtn;
    IBOutlet NSButtonCell *_fixedColSizeBtn;
    IBOutlet NSTextField *_fixedColSizeData;
    IBOutlet NSStepper *_fixedColSizeStepper;
    IBOutlet NSTextField *_numOfColData;
    IBOutlet NSTextField *_numOfColLabel;
    IBOutlet NSStepper *_numOfColStepper;
	IBOutlet NSMatrix *_radioMatrix;
    IBOutlet NSTextField *_numOfRowData;
    IBOutlet NSTextField *_numOfRowLabel;
    IBOutlet NSStepper *_numOfRowStepper;
    IBOutlet NSButton *_okBtn;
    IBOutlet NSBox *_tableSizeBox;
    IBOutlet NSTextField *_unitLabel;
	AP_CocoaDialog_InsertTable *_xap;
}
- (IBAction)cancelAction:(id)sender;
- (IBAction)colSizeAction:(id)sender;
- (IBAction)fixedColSizeAction:(id)sender;
- (IBAction)fixedColSizeStepperAction:(id)sender;
- (IBAction)numColAction:(id)sender;
- (IBAction)numColStepperAction:(id)sender;
- (IBAction)numRowAction:(id)sender;
- (IBAction)numRowStepperAction:(id)sender;
- (IBAction)okAction:(id)sender;

- (int)numRows;
- (int)numCols;
- (float)colWidth;
- (AP_Dialog_InsertTable::columnType)autoSizeType;
@end


#endif /* AP_COCOADIALOG_INSERTTABLE_H */
