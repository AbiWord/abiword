/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003, 2005-2021 Hubert Figuière
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

#pragma once

#import <Cocoa/Cocoa.h>

#include "ap_Dialog_Lists.h"
#include "ut_timer.h"
#include "xap_CocoaDialog_Utilities.h"

class GR_CocoaGraphics;
class XAP_CocoaFrame;
class AP_CocoaDialog_Lists;

@interface AP_CocoaDialog_ListsController : NSWindowController <XAP_CocoaDialogProtocol>
{
@public
    IBOutlet NSButton *_applyBtn;
    IBOutlet NSCell *_applyToCurrentBtn;
    IBOutlet NSCell *_attachToPreviousBtn;
    IBOutlet NSButton *_cancelBtn;
    IBOutlet NSTextField *_fontLabel;
    IBOutlet NSPopUpButton *_fontPopup;
    IBOutlet NSTextField *_formatData;
    IBOutlet NSTextField *_formatLabel;
    IBOutlet NSTextField *_labelAlignData;
    IBOutlet NSTextField *_labelAlignLabel;
    IBOutlet NSStepper *_labelAlignStepper;
    IBOutlet NSTextField *_levelDelimData;
    IBOutlet NSTextField *_levelDelimLabel;
	IBOutlet NSMatrix *_listActionMatrix;
    IBOutlet XAP_CocoaNSView *_preview;
    IBOutlet NSBox *_previewBox;
    IBOutlet NSButton *_setDefaultBtn;
    IBOutlet NSTextField *_startAtData;
    IBOutlet NSTextField *_startAtLabel;
    IBOutlet NSStepper *_startAtStepper;
    IBOutlet NSCell *_startNewListBtn;
    IBOutlet NSTextField *_styleLabel;
    IBOutlet NSPopUpButton *_stylePopup;
    IBOutlet NSTextField *_textAlignData;
    IBOutlet NSTextField *_textAlignLabel;
    IBOutlet NSStepper *_textAlignStepper;
    IBOutlet NSTextField *_typeLabel;
    IBOutlet NSPopUpButton *_typePopup;

	IBOutlet NSTabView	*_mainTab;

	IBOutlet NSBox *_hideTextLabel;
	IBOutlet NSMatrix *_foldingMatrix;
	IBOutlet NSCell	*_noFoldingBtn;
	IBOutlet NSCell	*_foldLevel1Btn;
	IBOutlet NSCell	*_foldLevel2Btn;
	IBOutlet NSCell	*_foldLevel3Btn;
	IBOutlet NSCell	*_foldLevel4Btn;

	IBOutlet NSMenu*	m_listStyleNone_menu;
	IBOutlet NSMenu*	m_listStyleNumbered_menu;
	IBOutlet NSMenu*	m_listStyleBulleted_menu;
	AP_CocoaDialog_Lists	*_xap;
}
- (id)initFromNib;
- (void)setXAPOwner:(XAP_Dialog *)owner;
- (void)discardXAP;
- (void)windowDidLoad;

- (void)windowDidBecomeKey:(NSNotification *)aNotification;

- (XAP_CocoaNSView*)preview;
- (NSMenuItem*)selectedListStyle;
- (NSMenuItem*)selectedListType;
- (void)setStyleMenu:(int)type;
- (void)selectFolding:(int)folding;
- (NSInteger)selectedTab;

- (NSInteger)listAction;

- (IBAction)applyAction:(id)sender;
- (IBAction)cancelAction:(id)sender;
- (IBAction)labelAlignAction:(id)sender;
- (IBAction)labelAlignActionStepper:(id)sender;
- (IBAction)setDefaultAction:(id)sender;
- (IBAction)startAtAction:(id)sender;
- (IBAction)startAtStepperAction:(id)sender;
- (IBAction)styleChangedAction:(id)sender;
- (IBAction)textAlignAction:(id)sender;
- (IBAction)textAlignActionStepper:(id)sender;
- (IBAction)typeChangedAction:(id)sender;
- (IBAction)valueChangedAction:(id)sender;

- (IBAction)foldingChanged:(id)sender;
@end

/*****************************************************************/

class AP_CocoaDialog_Lists: public AP_Dialog_Lists
{
 public:
	AP_CocoaDialog_Lists(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_CocoaDialog_Lists(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void runModeless(XAP_Frame * pFrame) override;
	virtual void destroy(void) override;
	virtual void activate(void) override;
	virtual void notifyActiveFrame(XAP_Frame *pFrame) override;
	virtual void runModal(XAP_Frame * pFrame) override;
	virtual void setFoldLevelInGUI(void) override;
	virtual bool isPageLists(void) override;
	/* CALLBACKS */

	void					customChanged(void);
	void					applyClicked(void);
	void					typeChanged(int type);
	void previewInvalidate(void);

	/* Just Plain Useful Functions */

	void                    setListTypeFromWidget(void);
	void					setXPFromLocal(void);
	void					loadXPDataIntoLocal(void);
	void					updateFromDocument(void);
	void					setAllSensitivity(void);
	void					updateDialog(void);
	bool                                    dontUpdate(void);
	static void				autoupdateLists(UT_Worker * pTimer);
	void					_fillNumberedStyleMenu(NSMenu *listmenu);
	void					_fillBulletedStyleMenu(NSMenu *listmenu);
	void					_fillNoneStyleMenu(NSMenu *listmenu);
	void					_fillFontMenu(NSPopUpButton* menu);

	void _foldingChanged(NSInteger i)
		{
			setCurrentFold(static_cast<UT_sint32>(i));
		}
 protected:
	void					_setRadioButtonLabels(void);
	void					_gatherData(void);
 private:
	GR_CocoaGraphics* m_pPreviewWidget;
	UT_Timer *				m_pAutoUpdateLists;
	bool					m_bDontUpdate;
	bool					m_bDestroy_says_stopupdating;
	bool					m_bAutoUpdate_happening_now;
	AP_CocoaDialog_ListsController* m_dlg;
};
