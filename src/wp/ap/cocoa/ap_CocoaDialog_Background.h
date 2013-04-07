/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#ifndef AP_COCOADIALOG_BACKGROUND_H
#define AP_COCOADIALOG_BACKGROUND_H

#import <Cocoa/Cocoa.h>

#include "xap_CocoaDialog_Utilities.h"

#include "ap_Dialog_Background.h"

class AP_CocoaDialog_Background : public AP_Dialog_Background
{
public:
	AP_CocoaDialog_Background(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);

	virtual ~AP_CocoaDialog_Background(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	void					_setAnswer (AP_Dialog_Background::tAnswer ans) { setAnswer(ans); }
};

@interface AP_CocoaDialog_Background_Controller : NSWindowController <XAP_CocoaDialogProtocol>
{
	IBOutlet NSButton *		oClear;
	IBOutlet NSButton *		oCancel;
	IBOutlet NSButton *		oOK;

	IBOutlet NSColorWell *	oColorWell;

	AP_CocoaDialog_Background *	_xap;
}
- (void)dealloc;

- (IBAction)aColor:(id)sender;
- (IBAction)aClear:(id)sender;
- (IBAction)aCancel:(id)sender;
- (IBAction)aOK:(id)sender;
@end

#endif /* AP_COCOADIALOG_BACKGROUND_H */
