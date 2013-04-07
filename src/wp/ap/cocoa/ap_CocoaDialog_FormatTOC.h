/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2004 Hubert Figuiere
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

#ifndef AP_COCOADIALOG_FORMATTOC_H
#define AP_COCOADIALOG_FORMATTOC_H

#include <Cocoa/Cocoa.h>

#include "ap_Dialog_FormatTOC.h"


@class AP_CocoaDialog_FormatTOC_Controller;
@protocol XAP_CocoaDialogProtocol;

/*****************************************************************/

class AP_CocoaDialog_FormatTOC: public AP_Dialog_FormatTOC
{
public:
	AP_CocoaDialog_FormatTOC(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_FormatTOC(void);
	virtual void			runModeless(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
	virtual void            notifyActiveFrame(XAP_Frame * pFrame);
	virtual void            setTOCPropsInGUI(void);
    virtual void            setSensitivity(bool bSensitive);
	virtual void            destroy(void);
	virtual void            activate(void);
private:
	void			_populateWindowData(void);
	AP_CocoaDialog_FormatTOC_Controller *m_dlg;
};


@interface AP_CocoaDialog_FormatTOC_Controller : NSWindowController <XAP_CocoaDialogProtocol>
{
	IBOutlet NSButton *			_applyBtn;
	IBOutlet NSButton *			_hasHeadingBtn;
	IBOutlet NSButton *			_hasLabelBtn;
	IBOutlet NSButton *			_inheritLabelBtn;
	IBOutlet NSButton *			_displayStyleBtn;
	IBOutlet NSButton *			_fillStyleBtn;
	IBOutlet NSButton *			_headingStyleBtn;

	IBOutlet NSPopUpButton *	_layoutLevelPopup;
	IBOutlet NSPopUpButton *	_mainLevelPopup;
	IBOutlet NSPopUpButton *	_numberingTypeData;
	IBOutlet NSPopUpButton *	_pageNumberingData;
	IBOutlet NSPopUpButton *	_tabLeadersData;

	IBOutlet NSTextField *		_displayStyleLabel;
	IBOutlet NSTextField *		_displayStyleData;
	IBOutlet NSTextField *		_fillStyleLabel;
	IBOutlet NSTextField *		_fillStyleData;
	IBOutlet NSTextField *		_headingStyleLabel;
	IBOutlet NSTextField *		_headingStyleData;
	IBOutlet NSTextField *		_headingTextData;
	IBOutlet NSTextField *		_headingTextLabel;
	IBOutlet NSTextField *		_indentData;
	IBOutlet NSTextField *		_indentLabel;
	IBOutlet NSTextField *		_numberingTypeLabel;
	IBOutlet NSTextField *		_pageNumberingLabel;
	IBOutlet NSTextField *		_startAtData;
	IBOutlet NSTextField *		_startAtLabel;
	IBOutlet NSTextField *		_tabLeadersLabel;
	IBOutlet NSTextField *		_textAfterData;
	IBOutlet NSTextField *		_textAfterLabel;
	IBOutlet NSTextField *		_textBeforeData;
	IBOutlet NSTextField *		_textBeforeLabel;
	IBOutlet NSBox *			_defineMainLabel;
	IBOutlet NSBox *			_labelDefinitionsLabel;
	IBOutlet NSBox *			_tabsPageNoLabel;

	IBOutlet NSStepper *		_startAtStepper;
	IBOutlet NSStepper *		_indentStepper;

//	IBOutlet NSBox *			_hasHeadingBox;
//	IBOutlet NSBox *			_labelDefBox;
//	IBOutlet NSBox *			_mainPropBox;
//	IBOutlet NSBox *			_tabsAndPageNumbBox;

	IBOutlet NSTabView *		_tabView;

	AP_CocoaDialog_FormatTOC *	_xap;
}
- (IBAction)startAtStepperAction:(id)sender;
- (IBAction)startAtAction:(id)sender;
- (IBAction)indentStepperAction:(id)sender;
- (IBAction)indentAction:(id)sender;
- (IBAction)mainLevelAction:(id)sender;
- (IBAction)detailLevelAction:(id)sender;
- (IBAction)headingStyleAction:(id)sender;
- (IBAction)fillStyleAction:(id)sender;
- (IBAction)displayStyleAction:(id)sender;
- (IBAction)applyAction:(id)sender;

- (void)setSensitivity:(BOOL)enable;

- (void)createLevelItems:(NSPopUpButton *)popup;
- (void)createNumberingItems:(NSPopUpButton *)popup;

- (void)sync;
- (void)syncMainLevelSettings;
- (void)syncDetailLevelSettings;

- (void)saveMainLevelSettings;
- (void)saveDetailLevelSettings;
@end

#endif /* AP_COCOADIALOG_FORMATOC_H */
