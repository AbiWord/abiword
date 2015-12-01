/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
 * Copyright (C) 2005 Francis Franklin
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

#ifndef AP_COCOADIALOG_NEW_H
#define AP_COCOADIALOG_NEW_H

#include <Cocoa/Cocoa.h>
#include "ap_Dialog_New.h"
#import "xap_Cocoa_NSTableUtils.h"

class XAP_CocoaFrame;
class AP_CocoaDialog_New;

@interface AP_CocoaDialog_NewController
	: NSWindowController <XAP_CocoaDialogProtocol, NSTableViewDelegate>
{
	IBOutlet NSButton *		_cancelBtn;
	IBOutlet NSButton *		_chooseFileBtn;
	IBOutlet NSButton *		_createNewBtn;
	IBOutlet NSButton *		_okBtn;
	IBOutlet NSButton *		_openBtn;

	IBOutlet NSTextField *	_documentNameData;

	IBOutlet NSTableView *	_templateList;

	AP_CocoaDialog_New *	_xap;

	XAP_StringListDataSource *	_dataSource;

	NSMutableArray *	m_templates;
}
- (void)tableViewSelectionDidChange:(NSNotification *)aNotification;
- (IBAction)cancelAction:(id)sender;
- (IBAction)radioButtonAction:(id)sender;
- (IBAction)chooseAction:(id)sender;
- (IBAction)okAction:(id)sender;
- (void)synchronizeGUI:(NSControl*)control;
- (BOOL)existingBtnState;
- (void)setExistingBtnState:(BOOL)state;
- (NSString *)newBtnState;
- (void)setFileName:(NSString*)name;
@end

/*****************************************************************/

class AP_CocoaDialog_New: public AP_Dialog_New
{
public:
	AP_CocoaDialog_New(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_New(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *,
											   XAP_Dialog_Id dlgid);

	void event_Ok ();
	void event_Cancel ();
	void event_ToggleUseTemplate (const char * name);
	void event_ToggleOpenExisting ();
	void event_ToggleStartNew ();

private:
	XAP_Frame * m_pFrame;
	AP_CocoaDialog_NewController*	m_dlg;
};

#endif /* AP_COCOADIALOG_NEW_H */
