/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#ifndef AP_CocoaDialog_PageSetup_H
#define AP_CocoaDialog_PageSetup_H

#import <Cocoa/Cocoa.h>
#include "ap_Dialog_PageSetup.h"

class XAP_Frame;
class AP_CocoaDialog_PageSetup;


@interface AP_CocoaDialog_PageSetup_Controller : NSObject
{
    IBOutlet NSTextField *_adjustData;
    IBOutlet NSTextField *_adjustLabel;
    IBOutlet NSStepper *_adjustStepper;
    IBOutlet NSTextField *_bottomMargin;
    IBOutlet NSTextField *_footerMargin;
    IBOutlet NSTextField *_headerMargin;
    IBOutlet NSImageView *_icon;
    IBOutlet NSTextField *_leftMargin;
    IBOutlet NSBox *_marginBox;
    IBOutlet NSTextField *_percentLabel;
    IBOutlet NSTextField *_rightMargin;
    IBOutlet NSBox *_scaleBox;
    IBOutlet NSTextField *_topMargin;
    IBOutlet NSTextField *_unitLabel;
    IBOutlet NSPopUpButton *_unitPopup;
    IBOutlet NSView *_view;
	AP_CocoaDialog_PageSetup*	_xap;
	UT_Dimension _last_margin_unit;
}
- (void)setXAPOwner:(AP_CocoaDialog_PageSetup*)owner;
- (NSView*)view;
- (void)fetchData;

- (IBAction)adjustAction:(id)sender;
- (IBAction)adjustStepperAction:(id)sender;
- (IBAction)unitAction:(id)sender;
@end


class AP_CocoaDialog_PageSetup : public AP_Dialog_PageSetup
{
public:
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	AP_CocoaDialog_PageSetup(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);

	virtual ~AP_CocoaDialog_PageSetup();

	virtual void runModal(XAP_Frame *pFrame);

private:
	bool					_validate(AP_CocoaDialog_PageSetup_Controller* ctrl, NSPrintInfo * printInfo);

	AP_CocoaDialog_PageSetup_Controller *	m_ctrl;
	XAP_Frame *								m_pFrame;
};

#endif // AP_CocoaDialog_PageSetup_H
