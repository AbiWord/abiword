/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
 * Copyright (C) 2003-2004-2021 Hubert Figuière
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

#pragma once

#import <Cocoa/Cocoa.h>

#import "xap_CocoaToolbar_Icons.h"
#import "ap_Dialog_FormatFrame.h"


class XAP_CocoaFrame;
class GR_CocoaGraphics;
@class AP_CocoaDialog_FormatFrameController;

/*****************************************************************/

class AP_CocoaDialog_FormatFrame: public AP_Dialog_FormatFrame
{
public:
	AP_CocoaDialog_FormatFrame(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_FormatFrame(void);

	virtual void runModeless(XAP_Frame * pFrame) override;

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// callbacks can fire these events
	virtual void			event_Close(void);
	void					event_ApplyToChanged(void);
	void event_previewInvalidate(void);
	virtual void setBorderThicknessInGUI(UT_UTF8String & sThick) override;
	virtual void setSensitivity(bool bSens) override;
	virtual void destroy(void) override;
	virtual void activate(void) override;
	virtual void notifyActiveFrame(XAP_Frame * pFrame) override;
	char * 					getWindowName()
						{ return m_WindowName; };
protected:
	void					_populateWindowData(void);
	void					_storeWindowData(void);

	GR_CocoaGraphics* m_pPreviewWidget;
private:
	AP_CocoaDialog_FormatFrameController*	m_dlg;
};

@class XAP_CocoaToolbarButton;

@interface AP_CocoaDialog_FormatFrameController : NSWindowController <XAP_CocoaDialogProtocol, XAP_CocoaButtonController>
{
	IBOutlet NSColorWell *		_bgColorWell;
	IBOutlet NSColorWell *		_borderColorWell;
	IBOutlet NSColorWell *		_rightBorderColorWell;
	IBOutlet NSColorWell *		_leftBorderColorWell;
	IBOutlet NSColorWell *		_topBorderColorWell;
	IBOutlet NSColorWell *		_bottomBorderColorWell;

//	IBOutlet NSTextField *		_applyToLabel;
//	IBOutlet NSPopUpButton *	_applyToPopup;

	IBOutlet XAP_CocoaNSView *	_preview;

	IBOutlet NSBox *			_previewBox;
	IBOutlet NSBox *			_borderBox;
	IBOutlet NSBox *			_bgBox;

	IBOutlet XAP_CocoaToolbarButton *	_rightBorderBtn;
	IBOutlet XAP_CocoaToolbarButton *	_leftBorderBtn;
	IBOutlet XAP_CocoaToolbarButton *	_topBorderBtn;
	IBOutlet XAP_CocoaToolbarButton *	_bottomBorderBtn;

	IBOutlet NSButton *			_setImageBtn;
	IBOutlet NSButton *			_noImageBtn;

	IBOutlet NSButton *			_applyBtn;
	IBOutlet NSButton *			_wrapSwitch;

	IBOutlet NSTextField *		_bgColorLabel;
	IBOutlet NSTextField *		_borderColorLabel;

	IBOutlet NSFormCell *		_borderNumber;
	IBOutlet NSFormCell *		_rightBorderNumber;
	IBOutlet NSFormCell *		_leftBorderNumber;
	IBOutlet NSFormCell *		_topBorderNumber;
	IBOutlet NSFormCell *		_bottomBorderNumber;

	IBOutlet NSForm *			_borderNumberForm;
	IBOutlet NSForm *			_rightBorderNumberForm;
	IBOutlet NSForm *			_leftBorderNumberForm;
	IBOutlet NSForm *			_topBorderNumberForm;
	IBOutlet NSForm *			_bottomBorderNumberForm;

	IBOutlet NSStepper *		_borderStepper;
	IBOutlet NSStepper *		_rightBorderStepper;
	IBOutlet NSStepper *		_leftBorderStepper;
	IBOutlet NSStepper *		_topBorderStepper;
	IBOutlet NSStepper *		_bottomBorderStepper;

	IBOutlet NSMenu *			_linestyleMenu;
	IBOutlet NSMenuItem *		_linestyleNone;
	IBOutlet NSMenuItem *		_linestyleNormal;
	IBOutlet NSMenuItem *		_linestyleDotted;
	IBOutlet NSMenuItem *		_linestyleDashed;

	IBOutlet NSPopUpButton *	_positionPopUp;

	AP_CocoaDialog_FormatFrame *	_xap;

	BOOL	m_bEnabled;

	NSInteger m_menuButtonTag;
	NSInteger m_activeMenuTag;
}
- (IBAction)applyAction:(id)sender;
- (IBAction)wrapAction:(id)sender;
- (IBAction)positionAction:(id)sender;
- (IBAction)bgColorAction:(id)sender;
- (IBAction)borderThicknessField:(id)sender;
- (IBAction)borderThicknessStepper:(id)sender;
- (IBAction)borderColorAction:(id)sender;
- (IBAction)borderLineStyleAction:(id)sender;
- (IBAction)bottomBorderAction:(id)sender;
- (IBAction)leftBorderAction:(id)sender;
- (IBAction)removeImageAction:(id)sender;
- (IBAction)rightBorderAction:(id)sender;
- (IBAction)selectImageAction:(id)sender;
- (IBAction)topBorderAction:(id)sender;
- (IBAction)applyToAction:(id)sender;

- (XAP_CocoaNSView *)preview;

- (void)setSensitivity:(bool)bSens;
- (void)setWrapState:(int)state;
- (void)setPositionState:(int)state;
- (void)setBorderColor:(NSColor *)color;
- (void)setBackgroundColor:(NSColor *)color;

- (void)menuWillActivate:(NSMenu *)menu forButton:(XAP_CocoaToolbarButton *)button;
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;

- (void) setRightBorderState:(int)state borderColor:(NSColor *)color borderThickness:(NSString *)thickness stepperValue:(float)value;
- (void)  setLeftBorderState:(int)state borderColor:(NSColor *)color borderThickness:(NSString *)thickness stepperValue:(float)value;
- (void)   setTopBorderState:(int)state borderColor:(NSColor *)color borderThickness:(NSString *)thickness stepperValue:(float)value;
- (void)setBottomBorderState:(int)state borderColor:(NSColor *)color borderThickness:(NSString *)thickness stepperValue:(float)value;

//- (int)applyItemTag;
@end
