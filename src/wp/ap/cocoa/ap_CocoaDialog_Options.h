/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003 Hubert Figuiere
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef AP_COCOADIALOG_OPTIONS_H
#define AP_COCOADIALOG_OPTIONS_H

#import <Cocoa/Cocoa.h>

#import "xap_Cocoa_NSTableUtils.h"

#import "ap_CocoaFrame.h"

#include "ap_Dialog_Options.h"
#include "ap_Prefs.h"

class XAP_Frame;

class AP_CocoaDialog_Options;

@class AP_CocoaDialog_OptionsController;

enum AP_CocoaDialog_SchemesButton_ID
	{
		CDS_Button_Close = 0,
		CDS_Button_Delete,
		CDS_Button_Duplicate,
		CDS_Button_New,
		CDS_Button_Rename,
		CDS_Button__count
	};

@interface AP_CocoaSchemeManager : NSWindowController
{
	AP_CocoaDialog_OptionsController *	m_controller;

	NSButton *				Button_List[CDS_Button__count];

	IBOutlet NSButton *		oButton_Close;
	IBOutlet NSButton *		oButton_Delete;
	IBOutlet NSButton *		oButton_Duplicate;
	IBOutlet NSButton *		oButton_New;
	IBOutlet NSButton *		oButton_Rename;

	IBOutlet NSTableView *	oTableView_Preferences;
	IBOutlet NSTableView *	oTableView_Schemes;
}
- (void)initialize:(AP_CocoaDialog_OptionsController *)controller;
- (void)cleanup;
- (void)windowDidLoad;
- (IBAction)aClick:(id)sender;
@end

enum AP_CocoaDialog_OptionsButton_ID
	{
		CDO_Button_Apply = 0,
		CDO_Button_Cancel,
		CDO_Button_ChooseScreenColor,
		CDO_Button_Close,
		CDO_Button_Defaults,
		CDO_Button_Dictionary,
		CDO_Button_EditViewSchemes,
		CDO_Button_IgnoredEdit,
		CDO_Button_IgnoredReset,
		CDO_Button__count
	};
enum AP_CocoaDialog_OptionsPopUp_ID
	{
		CDO_PopUp_CurrentScheme = 0,
		CDO_PopUp_CustomDictionary,
		CDO_PopUp_PageSize,			// Redundant ?? This is probably a "normal.awt" template setting...
		CDO_PopUp_Units,
		CDO_PopUp__count
	};
enum AP_CocoaDialog_OptionsSwitch_ID
	{
		CDO_Switch_AutoSave = 0,
		CDO_Switch_CheckSpelling,
		CDO_Switch_CheckGrammar,
		CDO_Switch_CursorBlink,
		CDO_Switch_DirectionMarkers,
		CDO_Switch_DirectionRTL,
		CDO_Switch_GlyphSaveVisual,
		CDO_Switch_GlyphShaping,
		CDO_Switch_HighlightMisspelled,
		CDO_Switch_IgnoreNumbered,
		CDO_Switch_IgnoreUppercase,
		CDO_Switch_IgnoreURLs,
		CDO_Switch_LayoutMarks,
		CDO_Switch_MainDictionaryOnly,
		CDO_Switch_Plugins,
		CDO_Switch_Ruler,
		CDO_Switch_SaveScheme,
		CDO_Switch_ScreenColor,
		CDO_Switch_StatusBar,
		CDO_Switch_SuggestCorrections,
		CDO_Switch_ToolbarExtra,
		CDO_Switch_ToolbarFormat,
		CDO_Switch_ToolbarStandard,
		CDO_Switch_ToolbarTable,
		CDO_Switch_ViewAll,
		CDO_Switch_ViewHidden,
		CDO_Switch__count
	};

@interface AP_CocoaDialog_OptionsController : NSWindowController
{
	AP_CocoaDialog_Options *			m_xap;

	AP_PreferenceSchemeManager *		m_pSchemeManager;
	AP_PreferenceScheme *				m_pActiveScheme;

	AP_PreferenceScheme::BoolOption		m_BOList[CDO_Switch__count];

	NSMutableArray *			m_LanguageList;
	NSMutableArray *			m_UnitsList;

	NSButton *					Button_List[CDO_Button__count];
	NSButton *					Switch_List[CDO_Switch__count];

	NSPopUpButton *				PopUp_List[CDO_PopUp__count];

	IBOutlet NSBox *			oBox_ApplicationStartup;
	IBOutlet NSBox *			oBox_AutoSave;
	IBOutlet NSBox *			oBox_BiDiOptions;
	IBOutlet NSBox *			oBox_DocumentSetup;
	IBOutlet NSBox *			oBox_General;
	IBOutlet NSBox *			oBox_Ignore;
	IBOutlet NSBox *			oBox_InterfaceLanguage;
	IBOutlet NSBox *			oBox_PreferenceSchemes;
	IBOutlet NSBox *			oBox_UserInterface;
	IBOutlet NSBox *			oBox_View;

	IBOutlet NSButton *			oButton_Apply;
	IBOutlet NSButton *			oButton_Cancel;
	IBOutlet NSButton *			oButton_ChooseScreenColor;
	IBOutlet NSButton *			oButton_Close;
	IBOutlet NSButton *			oButton_Defaults;
	IBOutlet NSButton *			oButton_Dictionary;
	IBOutlet NSButton *			oButton_EditViewSchemes;
	IBOutlet NSButton *			oButton_IgnoredEdit;
	IBOutlet NSButton *			oButton_IgnoredReset;

	IBOutlet NSColorWell *		oColorWell_Screen;

	IBOutlet NSTextField *		oField_Extension;
	IBOutlet NSTextField *		oField_Minutes;

	IBOutlet NSTextField *		oLabel_CurrentPreferences;
	IBOutlet NSTextField *		oLabel_CustomDictionary;
	IBOutlet NSTextField *		oLabel_IgnoredWords;
	IBOutlet NSTextField *		oLabel_Minutes;
	IBOutlet NSTextField *		oLabel_PageSize;
	IBOutlet NSTextField *		oLabel_WithExtension;
	IBOutlet NSTextField *		oLabel_Units;

	IBOutlet NSPopUpButton *	oPopUp_CurrentScheme;
	IBOutlet NSPopUpButton *	oPopUp_CustomDictionary;
	IBOutlet NSPopUpButton *	oPopUp_PageSize;
	IBOutlet NSPopUpButton *	oPopUp_Units;

	IBOutlet NSStepper *		oStepper_Minutes;

	IBOutlet NSButton *			oSwitch_AutoSave;
	IBOutlet NSButton *			oSwitch_CheckSpelling;
	IBOutlet NSButton *			oSwitch_CheckGrammar;
	IBOutlet NSButton *			oSwitch_CursorBlink;
	IBOutlet NSButton *			oSwitch_DirectionMarkers;
	IBOutlet NSButton *			oSwitch_DirectionRTL;
	IBOutlet NSButton *			oSwitch_GlyphSaveVisual;
	IBOutlet NSButton *			oSwitch_GlyphShaping;
	IBOutlet NSButton *			oSwitch_HighlightMisspelled;
	IBOutlet NSButton *			oSwitch_IgnoreNumbered;
	IBOutlet NSButton *			oSwitch_IgnoreUppercase;
	IBOutlet NSButton *			oSwitch_IgnoreURLs;
	IBOutlet NSButton *			oSwitch_LayoutMarks;
	IBOutlet NSButton *			oSwitch_MainDictionaryOnly;
	IBOutlet NSButton *			oSwitch_Plugins;
	IBOutlet NSButton *			oSwitch_Ruler;
	IBOutlet NSButton *			oSwitch_SaveScheme;
	IBOutlet NSButton *			oSwitch_ScreenColor;
	IBOutlet NSButton *			oSwitch_StatusBar;
	IBOutlet NSButton *			oSwitch_SuggestCorrections;
	IBOutlet NSButton *			oSwitch_ToolbarExtra;
	IBOutlet NSButton *			oSwitch_ToolbarFormat;
	IBOutlet NSButton *			oSwitch_ToolbarStandard;
	IBOutlet NSButton *			oSwitch_ToolbarTable;
	IBOutlet NSButton *			oSwitch_ViewAll;
	IBOutlet NSButton *			oSwitch_ViewHidden;

	IBOutlet NSTableView *		oTableView_InterfaceLanguage;

	IBOutlet NSTabView *		oTabView;

	IBOutlet AP_CocoaSchemeManager *	oPreferenceSchemeManager;
}
- (id)initFromNib;
- (oneway void)dealloc;
- (void)windowDidLoad;

- (void)setXAPOwner:(AP_CocoaDialog_Options *)owner;
- (void)setSchemeManager:(AP_PreferenceSchemeManager *)schemeManager;

- (IBAction)aButton:(id)sender;
- (IBAction)aColorWell:(id)sender;
- (IBAction)aField_Extension:(id)sender;
- (IBAction)aField_Minutes:(id)sender;
- (IBAction)aPopUp:(id)sender;
- (IBAction)aStepper:(id)sender;
- (IBAction)aSwitch:(id)sender;

- (void)sync;

/* NSTableViewDataSource methods
 */
- (int)numberOfRowsInTableView:(NSTableView *)aTableView;
- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;

/* NSTableView delegate methods
 */
- (void)tableViewSelectionDidChange:(NSNotification *)aNotification;
- (void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;
@end

/*****************************************************************/

class AP_CocoaDialog_Options : public XAP_TabbedDialog_NonPersistent
{
public:
	AP_CocoaDialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);

	virtual ~AP_CocoaDialog_Options();

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);
};

#endif /* AP_COCOADIALOG_OPTIONS_H */
