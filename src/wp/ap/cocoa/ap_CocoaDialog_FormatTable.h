/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
 * Copyright (C) 2003-2021 Hubert Figuiere
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

#include "ap_Dialog_FormatTable.h"

class GR_CocoaGraphics;
class XAP_CocoaFrame;
@class AP_CocoaDialog_FormatTableController;

/*****************************************************************/

class AP_CocoaDialog_FormatTable: public AP_Dialog_FormatTable
{
public:
	AP_CocoaDialog_FormatTable(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_FormatTable(void);

	virtual void runModeless(XAP_Frame * pFrame) override;

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// callbacks can fire these events
	virtual void			event_Close(void);
	void					event_ApplyToChanged(void);
	void event_previewInvalidate(void);
	void                    event_BorderThicknessChanged(NSPopUpButton *ctrl);
	virtual void setSensitivity(bool bSens) override;
	virtual void setBackgroundColorInGUI(UT_RGBColor clr) override;
	virtual void setBorderThicknessInGUI(UT_UTF8String & sThick) override;
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
	AP_CocoaDialog_FormatTableController*	m_dlg;
};


@interface AP_CocoaDialog_FormatTableController : NSWindowController <XAP_CocoaDialogProtocol>
{
@public
    IBOutlet NSButton *_applyBtn;
    IBOutlet NSBox *_bgBox;
	IBOutlet NSBox *_imageBgBox;
    IBOutlet NSTextField *_bgColorLabel;
    IBOutlet NSColorWell *_bgColorWell;
    IBOutlet NSBox *_borderBox;
    IBOutlet NSTextField *_borderColorLabel;
    IBOutlet NSColorWell *_borderColorWell;
    IBOutlet NSTextField *_applyToLabel;
    IBOutlet NSPopUpButton *_applyToPopup;
    IBOutlet NSButton *_bottomBorderBtn;
    IBOutlet NSButton *_leftBorderBtn;
    IBOutlet XAP_CocoaNSView *_preview;
    IBOutlet NSBox *_previewBox;
    IBOutlet NSButton *_rightBorderBtn;
    IBOutlet NSButton *_topBorderBtn;
    IBOutlet NSButton *_setImageBtn;
    IBOutlet NSButton *_noImageBtn;
	IBOutlet NSTextField *_thicknessLabel;
	IBOutlet NSPopUpButton *_thicknessPopup;
	AP_CocoaDialog_FormatTable*	_xap;
}
- (IBAction)applyAction:(id)sender;
- (IBAction)bgColorAction:(id)sender;
- (IBAction)borderColorAction:(id)sender;
- (IBAction)bottomBorderAction:(id)sender;
- (IBAction)leftBorderAction:(id)sender;
- (IBAction)removeImageAction:(id)sender;
- (IBAction)rightBorderAction:(id)sender;
- (IBAction)selectImageAction:(id)sender;
- (IBAction)topBorderAction:(id)sender;
- (IBAction)applyToAction:(id)sender;
- (IBAction)borderThicknessAction:(id)sender;

- (XAP_CocoaNSView*)preview;
- (void)setSensitivity:(bool)bSens;
- (int)applyItemTag;
@end
