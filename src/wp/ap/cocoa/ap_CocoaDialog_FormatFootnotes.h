/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef AP_COCOADIALOG_STUB_H
#define AP_COCOADIALOG_STUB_H

#include "ap_Dialog_FormatFootnotes.h"
#import "xap_CocoaDialog_Utilities.h"

class XAP_CocoaFrame;
@class AP_CocoaDialog_FormatFootnotes_Controller;


/*****************************************************************/

class AP_CocoaDialog_FormatFootnotes: public AP_Dialog_FormatFootnotes
{
public:
	AP_CocoaDialog_FormatFootnotes(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_FormatFootnotes(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	void event_OK(void);
	void event_Cancel(void);
	void refreshVals(void);
	void event_MenuStyleChange(NSPopUpButton* widget, bool bIsFootnote);
	void event_MenuEndNtPlacementChange(NSPopUpButton* widget);
	void event_MenuFtNtRestartChange(NSPopUpButton* widget);
	void event_FootInitialValueChange(NSTextField* widget);
	void event_EndInitialValueChange(NSTextField* widget);
	void event_EndRestartSection(NSButton* widget);

private:
	AP_CocoaDialog_FormatFootnotes_Controller*	m_dlg;
};


@interface AP_CocoaDialog_FormatFootnotes_Controller : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSButton *_cancelBtn;
	IBOutlet NSBox *_endNtBox;
    IBOutlet NSTextField *_endNtInitialValue;
    IBOutlet NSTextField *_endNtInitialValueLabel;
    IBOutlet NSStepper *_endNtInitialValueStepper;
    IBOutlet NSPopUpButton *_endNtPlacementPopup;
    IBOutlet NSTextField *_endNtPlacementLabel;
    IBOutlet NSButton *_endNtRestartSectionBtn;
    IBOutlet NSTextField *_endNtStyleLabel;
    IBOutlet NSPopUpButton *_endNtStylePopup;
	IBOutlet NSBox *_ftNtBox;
    IBOutlet NSTextField *_ftNtInitialValue;
    IBOutlet NSTextField *_ftNtInitialValueLabel;
    IBOutlet NSStepper *_ftNtInitialValueStepper;
    IBOutlet NSPopUpButton *_ftNtRestartPopup;
    IBOutlet NSTextField *_ftNtRestartLabel;
    IBOutlet NSTextField *_ftNtStyleLabel;
    IBOutlet NSPopUpButton *_ftNtStylePopup;
    IBOutlet NSButton *_okBtn;

	AP_CocoaDialog_FormatFootnotes*	_xap;
}
- (IBAction)cancelAction:(id)sender;
- (IBAction)initialEndNtValueAction:(id)sender;
- (IBAction)initialEndNtValueStepperAction:(id)sender;
- (IBAction)initialFtNtValueAction:(id)sender;
- (IBAction)initialFtNtValueStepperAction:(id)sender;
- (IBAction)okAction:(id)sender;
- (IBAction)styleMenuAction:(id)sender;
- (IBAction)ftNtRestartMenuAction:(id)sender;
- (IBAction)endNtPlacementMenuAction:(id)sender;
- (IBAction)endNtRestartSecAction:(id)sender;

- (void)setFtNtInitialValue:(int)val;
- (void)setEndNtInitialValue:(int)val;
- (void)setFtNtRestart:(int)val;
- (void)setEndNtPlacement:(int)val;
- (void)setEndNtRestartOnSec:(bool)val;
- (void)setNtType:(FootnoteType)type isFtNt:(BOOL)isFootnote;
@end

#endif /* AP_COCOADIALOG_STUB_H */
