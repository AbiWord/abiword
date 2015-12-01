/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
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

#ifndef AP_COCOADIALOG_FIELD_H
#define AP_COCOADIALOG_FIELD_H

#include "ap_Dialog_Field.h"

#import "xap_Cocoa_NSTableUtils.h"

class XAP_CocoaFrame;
@class AP_CocoaDialog_FieldController;
@protocol XAP_CocoaDialogProtocol;

/*****************************************************************/

class AP_CocoaDialog_Field: public AP_Dialog_Field
{
public:
	AP_CocoaDialog_Field(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *pFactory , XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_Field(void);

	virtual void runModal(XAP_Frame * pFrame);

	void event_OK(void);
	void event_Cancel(void);
	void types_changed(int row);
	void setTypesList(void);
	void setFieldsList(void);

private:
	void _populateCategories(void);
	XAP_StringListDataSource *m_typeList;
	XAP_StringListDataSource *m_fieldList;
	AP_CocoaDialog_FieldController* m_dlg;
};



@interface AP_CocoaDialog_FieldController
    : NSWindowController <XAP_CocoaDialogProtocol, NSTableViewDelegate>
{
    IBOutlet NSButton *_cancelBtn;
    IBOutlet NSTextField *_extraParamData;
    IBOutlet NSTextField *_extraParamLabel;
    IBOutlet NSTextField *_fieldsLabel;
    IBOutlet NSTableView *_fieldsList;
    IBOutlet NSButton *_okBtn;
    IBOutlet NSTextField *_typesLabel;
    IBOutlet NSTableView *_typesList;
    AP_CocoaDialog_Field *_xap;
}
- (int)selectedType;
- (int)selectedField;
- (NSString*)extraParam;
- (void)setTypeList:(XAP_StringListDataSource*)tl andFieldList:(XAP_StringListDataSource*)fl;
- (void)typesAction:(id)sender;
- (IBAction)cancelAction:(id)sender;
- (IBAction)okAction:(id)sender;

/* NSTableView delegate methods
 */
- (void)tableViewSelectionDidChange:(NSNotification *)aNotification;
@end

#endif /* AP_COCOADIALOG_FIELD_H */






