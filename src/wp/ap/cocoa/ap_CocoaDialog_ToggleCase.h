/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
 * Copyright (C) 2003 Mark Pazolli
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

#ifndef AP_COCOADIALOG_ToggleCase_H
#define AP_COCOADIALOG_ToggleCase_H

#include <Cocoa/Cocoa.h>

#include "ap_Dialog_ToggleCase.h"

class AP_CocoaDialog_ToggleCase;

@interface AP_CocoaDialog_ToggleCaseController : NSWindowController <XAP_CocoaDialogProtocol>
{
	IBOutlet NSButton *		_okBtn;
	IBOutlet NSButton *		_cancelBtn;

	IBOutlet NSMatrix *		_caseMatrix;

	IBOutlet NSButtonCell *	_initialBtn;
	IBOutlet NSButtonCell *	_lowerBtn;
	IBOutlet NSButtonCell *	_sentenceBtn;
//  IBOutlet NSButtonCell *	_titleBtn;
	IBOutlet NSButtonCell *	_toggleBtn;
	IBOutlet NSButtonCell *	_upperBtn;

	AP_CocoaDialog_ToggleCase *	_xap;
}
- (IBAction)cancelAction:(id)sender;
- (IBAction)okAction:(id)sender;
@end

/*****************************************************************/

class AP_CocoaDialog_ToggleCase: public AP_Dialog_ToggleCase
{
 public:
	AP_CocoaDialog_ToggleCase(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_ToggleCase(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
private:
	AP_CocoaDialog_ToggleCaseController*	m_dlg;
};

#endif /* AP_COCOADIALOG_ToggleCase_H */
