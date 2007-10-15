/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002-2003 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Prefs.h"
#include "xap_Toolbar_Layouts.h"
#include "xap_CocoaDialog_Utilities.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"

#import "ap_CocoaDialog_Options.h"

#ifdef defn
#undef defn
#endif

XAP_Dialog * AP_CocoaDialog_Options::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
    AP_CocoaDialog_Options * p = new AP_CocoaDialog_Options(pFactory,dlgid);
    return p;
}

AP_CocoaDialog_Options::AP_CocoaDialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid) :
	XAP_TabbedDialog_NonPersistent(pDlgFactory, dlgid, "interface/dialogpreferences")
{
	// 
}

AP_CocoaDialog_Options::~AP_CocoaDialog_Options()
{
	// 
}

void AP_CocoaDialog_Options::runModal(XAP_Frame * pFrame)
{
	AP_PreferenceSchemeManager * SchemeManager = AP_PreferenceSchemeManager::create_manager();
	UT_ASSERT(SchemeManager);
	if (!SchemeManager)
		return;

	AP_CocoaDialog_OptionsController *	controller = [[AP_CocoaDialog_OptionsController alloc] initFromNib];
	if (controller)
		{
			[controller setXAPOwner:this];
			[controller setSchemeManager:SchemeManager];

			[NSApp runModalForWindow:[controller window]];

			[controller close];
			[controller release];
		}
	DELETEP(SchemeManager);
}

/* ******************************** AP_CocoaSchemeManager ******************************** */

@implementation AP_CocoaSchemeManager

- (void)initialize:(AP_CocoaDialog_OptionsController *)controller
{
	m_controller = controller;
	// TODO: data sources / delegates
}

- (void)cleanup
{
	// TODO: data sources / delegates
}

- (void)windowDidLoad
{
#define defn(X)		Button_List[CDS_##X] = o##X

	defn(Button_Close);
	defn(Button_Delete);
	defn(Button_Duplicate);
	defn(Button_New);
	defn(Button_Rename);

#undef defn

	int count = static_cast<int>(CDS_Button__count);

	for (int i = 0; i < count; i++)
		{
			[Button_List[i] setTag:i];
			[Button_List[i] setEnabled:NO];
		}

	// TODO: data sources / delegates
	// TODO: IBOutlet NSTableView *	oTableView_Preferences;
	// TODO: IBOutlet NSTableView *	oTableView_Schemes;
}

- (IBAction)aClick:(id)sender
{
	switch (static_cast<AP_CocoaDialog_SchemesButton_ID>([sender tag]))
		{
		case CDS_Button_Close:
			// TODO
			break;
		case CDS_Button_Delete:
			// TODO
			break;
		case CDS_Button_Duplicate:
			// TODO
			break;
		case CDS_Button_New:
			// TODO
			break;
		case CDS_Button_Rename:
			// TODO
			break;
		default:
			UT_DEBUGMSG(("AP_CocoaSchemeManager -aClick: unexpected [sender tag]!\n"));
			break;
		}
}

@end

/* ******************************** AP_CocoaDialog_OptionsController ******************************** */

@implementation AP_CocoaDialog_OptionsController

- (id)initFromNib
{
	if (self = [super initWithWindowNibName:@"ap_CocoaDialog_Options"])
		{
			m_pSchemeManager = 0;
			m_pActiveScheme  = 0;

			m_LanguageList = 0;
			m_UnitsList    = 0;

			m_LanguageList = [[NSMutableArray alloc] initWithCapacity:128];
			if (!m_LanguageList)
				{
					[self release];
					self = 0;
				}
		}
	if (self)
		{
			m_UnitsList = [[NSMutableArray alloc] initWithCapacity:4];
			if (!m_UnitsList)
				{
					[self release];
					self = 0;
				}
		}
	return self;
}

- (oneway void)dealloc
{
	if (m_LanguageList)
		{
			[m_LanguageList release];
			m_LanguageList = 0;
		}
	if (m_UnitsList)
		{
			[m_UnitsList release];
			m_UnitsList = 0;
		}
	[super dealloc];
}

- (void)windowDidLoad
{
	int count = 0;

#define defn(X)		Button_List[CDO_##X] = o##X

	defn(Button_Apply);
	defn(Button_Cancel);
	defn(Button_ChooseScreenColor);
	defn(Button_Close);
	defn(Button_Defaults);
	defn(Button_Dictionary);
	defn(Button_EditViewSchemes);
	defn(Button_IgnoredEdit);
	defn(Button_IgnoredReset);

#undef defn

	count = static_cast<int>(CDO_Button__count);

	for (int i = 0; i < count; i++)
		{
			[Button_List[i] setTag:i];
			[Button_List[i] setEnabled:NO];
		}
	[oButton_Cancel setEnabled:YES]; // this never changes

#define defn(X)		PopUp_List[CDO_##X] = o##X

	defn(PopUp_CurrentScheme);
	defn(PopUp_CustomDictionary);
	defn(PopUp_PageSize);
	defn(PopUp_Units);

#undef defn

	count = static_cast<int>(CDO_PopUp__count);

	for (int i = 0; i < count; i++)
		{
			[PopUp_List[i] setTag:i];
			[PopUp_List[i] setEnabled:NO];
			[PopUp_List[i] removeAllItems];
		}

#define defn(X,Y)		Switch_List[CDO_##X] = o##X; m_BOList[CDO_##X] = Y

	defn(Switch_AutoSave,				AP_PreferenceScheme::bo_AutoSave);
	defn(Switch_CheckSpelling,			AP_PreferenceScheme::bo_CheckSpelling);
	defn(Switch_CheckGrammar,			AP_PreferenceScheme::bo_CheckGrammar);
	defn(Switch_CursorBlink,			AP_PreferenceScheme::bo_CursorBlink);
	defn(Switch_DirectionMarkers,		AP_PreferenceScheme::bo_DirectionMarkers);
	defn(Switch_DirectionRTL,			AP_PreferenceScheme::bo_DirectionRTL);
	defn(Switch_GlyphSaveVisual,		AP_PreferenceScheme::bo_GlyphSaveVisual);
	defn(Switch_GlyphShaping,			AP_PreferenceScheme::bo_GlyphShaping);
	defn(Switch_HighlightMisspelled,	AP_PreferenceScheme::bo_HighlightMisspelled);
	defn(Switch_IgnoreNumbered,			AP_PreferenceScheme::bo_IgnoreNumbered);
	defn(Switch_IgnoreUppercase,		AP_PreferenceScheme::bo_IgnoreUppercase);
	defn(Switch_IgnoreURLs,				AP_PreferenceScheme::bo_IgnoreURLs);
	defn(Switch_LayoutMarks,			AP_PreferenceScheme::bo_LayoutMarks);
	defn(Switch_MainDictionaryOnly,		AP_PreferenceScheme::bo_MainDictionaryOnly);
	defn(Switch_Plugins,				AP_PreferenceScheme::bo_Plugins);
	defn(Switch_Ruler,					AP_PreferenceScheme::bo_Ruler);
	defn(Switch_SaveScheme,				AP_PreferenceScheme::bo_SaveScheme);
	defn(Switch_ScreenColor,			AP_PreferenceScheme::bo_ScreenColor);
	defn(Switch_StatusBar,				AP_PreferenceScheme::bo_StatusBar);
	defn(Switch_SuggestCorrections,		AP_PreferenceScheme::bo_SuggestCorrections);
	defn(Switch_ToolbarExtra,			AP_PreferenceScheme::bo_ToolbarExtra);
	defn(Switch_ToolbarFormat,			AP_PreferenceScheme::bo_ToolbarFormat);
	defn(Switch_ToolbarStandard,		AP_PreferenceScheme::bo_ToolbarStandard);
	defn(Switch_ToolbarTable,			AP_PreferenceScheme::bo_ToolbarTable);
	defn(Switch_ViewAll,				AP_PreferenceScheme::bo_ViewAll);
	defn(Switch_ViewHidden,				AP_PreferenceScheme::bo_ViewHidden);

#undef defn

	count = static_cast<int>(CDO_Switch__count);

	for (int i = 0; i < count; i++)
		{
			[Switch_List[i] setTag:i];
			[Switch_List[i] setEnabled:NO];
		}

	/* Localize labels, etc.
	 */
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	LocalizeControl([self window], pSS, AP_STRING_ID_DLG_Options_OptionsTitle);

#define defn(Y,Z)		LocalizeControl([oTabView tabViewItemAtIndex:[oTabView indexOfTabViewItemWithIdentifier:Y]], pSS, Z)

	defn(@"General",   AP_STRING_ID_DLG_Options_Label_General);
// TODO: Interface
	defn(@"Documents", AP_STRING_ID_DLG_Options_Label_Documents);
	defn(@"Spelling",  AP_STRING_ID_DLG_Options_SpellCheckingTitle);

#undef defn

	LocalizeControl(oBox_ApplicationStartup,		pSS, AP_STRING_ID_DLG_Options_Label_AppStartup);
	LocalizeControl(oBox_PreferenceSchemes,			pSS, AP_STRING_ID_DLG_Options_Label_Schemes);
	LocalizeControl(oBox_AutoSave,					pSS, AP_STRING_ID_DLG_Options_Label_AutoSave);
	LocalizeControl(oBox_UserInterface,				pSS, AP_STRING_ID_DLG_Options_Label_UI);
	LocalizeControl(oBox_InterfaceLanguage,			pSS, AP_STRING_ID_DLG_Options_Label_UILang);
// TODO: Document Setup
	LocalizeControl(oBox_View,						pSS, AP_STRING_ID_DLG_Options_TabLabel_View);
// TODO: Editing
	LocalizeControl(oBox_BiDiOptions,				pSS, AP_STRING_ID_DLG_Options_Label_BiDiOptions);
	LocalizeControl(oBox_General,					pSS, AP_STRING_ID_DLG_Options_Label_General);
	LocalizeControl(oBox_Ignore,					pSS, AP_STRING_ID_DLG_Options_Label_Ignore);

	LocalizeControl(oSwitch_Plugins,				pSS, AP_STRING_ID_DLG_Options_Label_CheckAutoLoadPlugins);
	LocalizeControl(oSwitch_SaveScheme,				pSS, AP_STRING_ID_DLG_Options_Label_PrefsAutoSave);
	LocalizeControl(oLabel_CurrentPreferences,		pSS, AP_STRING_ID_DLG_Options_Label_PrefsCurrentScheme);
// TODO: Edit/View Schemes
	LocalizeControl(oSwitch_AutoSave,				pSS, AP_STRING_ID_DLG_Options_Label_AutoSaveCurrent);
	LocalizeControl(oLabel_Minutes,					pSS, AP_STRING_ID_DLG_Options_Label_Minutes);
	LocalizeControl(oLabel_WithExtension,			pSS, AP_STRING_ID_DLG_Options_Label_WithExtension);

	LocalizeControl(oSwitch_Ruler,					pSS, AP_STRING_ID_DLG_Options_Label_ViewRuler);
	LocalizeControl(oSwitch_StatusBar,				pSS, AP_STRING_ID_DLG_Options_Label_ViewStatusBar);
	LocalizeControl(oSwitch_CursorBlink,			pSS, AP_STRING_ID_DLG_Options_Label_ViewCursorBlink);
	LocalizeControl(oSwitch_ToolbarStandard,		pSS, AP_STRING_ID_DLG_Options_Label_ViewStandardTB);
	LocalizeControl(oSwitch_ToolbarFormat,			pSS, AP_STRING_ID_DLG_Options_Label_ViewFormatTB);
	LocalizeControl(oSwitch_ToolbarTable,			pSS, AP_STRING_ID_DLG_Options_Label_ViewTableTB);
	LocalizeControl(oSwitch_ToolbarExtra,			pSS, AP_STRING_ID_DLG_Options_Label_ViewExtraTB);
// TODO: Screen color
	LocalizeControl(oButton_ChooseScreenColor,		pSS, AP_STRING_ID_DLG_Options_Label_ChooseForTransparent);

	LocalizeControl(oLabel_Units,					pSS, AP_STRING_ID_DLG_Options_Label_ViewUnits);
	LocalizeControl(oLabel_PageSize,				pSS, AP_STRING_ID_DLG_Options_Label_DefaultPageSize);
	LocalizeControl(oSwitch_ViewAll,				pSS, AP_STRING_ID_DLG_Options_Label_ViewAll);
	LocalizeControl(oSwitch_ViewHidden,				pSS, AP_STRING_ID_DLG_Options_Label_ViewHiddenText);
	LocalizeControl(oSwitch_LayoutMarks,			pSS, AP_STRING_ID_DLG_Options_Label_ViewUnprintable);
	LocalizeControl(oSwitch_DirectionRTL,			pSS, AP_STRING_ID_DLG_Options_Label_DirectionRtl);
	LocalizeControl(oSwitch_GlyphShaping,			pSS, AP_STRING_ID_DLG_Options_Label_HebrewContextGlyphs);
// TODO: Save visual glyph shapes
	LocalizeControl(oSwitch_DirectionMarkers,		pSS,XAP_STRING_ID_DLG_Options_Label_DirMarkerAfterClosingParenthesis);

	LocalizeControl(oSwitch_CheckSpelling,			pSS, AP_STRING_ID_DLG_Options_Label_SpellCheckAsType);
	LocalizeControl(oSwitch_CheckGrammar,			pSS, AP_STRING_ID_DLG_Options_Label_GrammarCheck);
	LocalizeControl(oSwitch_HighlightMisspelled,	pSS, AP_STRING_ID_DLG_Options_Label_SpellHighlightMisspelledWords);
	LocalizeControl(oSwitch_SuggestCorrections,		pSS, AP_STRING_ID_DLG_Options_Label_SpellSuggest);
	LocalizeControl(oSwitch_MainDictionaryOnly,		pSS, AP_STRING_ID_DLG_Options_Label_SpellMainOnly);
	LocalizeControl(oSwitch_IgnoreUppercase,		pSS, AP_STRING_ID_DLG_Options_Label_SpellUppercase);
	LocalizeControl(oSwitch_IgnoreNumbered,			pSS, AP_STRING_ID_DLG_Options_Label_SpellNumbers);
// TODO: Internet and file addresses
	LocalizeControl(oLabel_CustomDictionary,		pSS, AP_STRING_ID_DLG_Options_Label_SpellCustomDict);
	LocalizeControl(oLabel_IgnoredWords,			pSS, AP_STRING_ID_DLG_Options_Label_SpellIgnoredWord);
	LocalizeControl(oButton_Dictionary,				pSS, AP_STRING_ID_DLG_Options_Btn_CustomDict);
	LocalizeControl(oButton_IgnoredReset,			pSS, AP_STRING_ID_DLG_Options_Btn_IgnoreReset);
	LocalizeControl(oButton_IgnoredEdit,			pSS, AP_STRING_ID_DLG_Options_Btn_IgnoreEdit);

	LocalizeControl(oButton_Apply,					pSS, AP_STRING_ID_DLG_Options_Btn_Apply);
	LocalizeControl(oButton_Defaults,				pSS, AP_STRING_ID_DLG_Options_Btn_Default);
	LocalizeControl(oButton_Close,					pSS,XAP_STRING_ID_DLG_Close);
	LocalizeControl(oButton_Cancel,					pSS,XAP_STRING_ID_DLG_Cancel);

// TODO: schemes popup
// TODO: page sizes popup
// TODO: custom dictionaries popup
// ?? AP_STRING_ID_DLG_Options_Label_Grammar

	// ...

	[oStepper_Minutes setIntValue:1];

	[oTableView_InterfaceLanguage setDataSource:self];
	[oTableView_InterfaceLanguage setDelegate:self];

	[oPopUp_Units removeAllItems];
	[oPopUp_Units addItemsWithTitles:m_UnitsList];

	[self sync];

	switch (m_xap->getInitialPageNum())
		{
		default:
		case 1:
			[oTabView selectTabViewItemWithIdentifier:@"General"];
			break;
		case 2:
			[oTabView selectTabViewItemWithIdentifier:@"Interface"];
			break;
		case 3:
			[oTabView selectTabViewItemWithIdentifier:@"Documents"];
			break;
		case 4:
			[oTabView selectTabViewItemWithIdentifier:@"Spelling"];
			break;
		}
}

- (IBAction)aButton:(id)sender
{
	bool bStopModal = false;

	UT_ASSERT(m_pActiveScheme);

	NSButton * button = (NSButton *) sender;

	switch (static_cast<AP_CocoaDialog_OptionsButton_ID>([button tag]))
		{
		case CDO_Button_Apply:
			{
				if (m_pActiveScheme)
					m_pActiveScheme->saveChanges();
			}
			break;
		case CDO_Button_Cancel:
			{
				bStopModal = true;
			}
			break;
		case CDO_Button_ChooseScreenColor:
			// TODO
			break;
		case CDO_Button_Close:
			{
				if (m_pActiveScheme)
					m_pActiveScheme->applySettings();
				bStopModal = true;
			}
			break;
		case CDO_Button_Defaults:
			{
				if (m_pActiveScheme)
					m_pActiveScheme->restoreDefaults();
			}
			break;
		case CDO_Button_Dictionary:
			// TODO
			break;
		case CDO_Button_EditViewSchemes:
			// TODO
			break;
		case CDO_Button_IgnoredEdit:
			// TODO
			break;
		case CDO_Button_IgnoredReset:
			// TODO
			break;
		default:
			UT_DEBUGMSG(("AP_CocoaDialog_OptionsController -aButton: unexpected [sender tag]!\n"));
			break;
		}
	if (bStopModal)
		{
			[NSApp stopModal];
		}
	else
		{
			[self sync];
		}
}

- (IBAction)aColorWell:(id)sender
{
	// TODO
}

- (IBAction)aField_Extension:(id)sender
{
	UT_ASSERT(m_pActiveScheme);
	if (m_pActiveScheme)
		{
			m_pActiveScheme->setAutoSaveExtension([[oField_Extension stringValue] UTF8String]);
		}
	[self sync];
}

- (IBAction)aField_Minutes:(id)sender
{
	UT_ASSERT(m_pActiveScheme);
	if (m_pActiveScheme)
		{
			m_pActiveScheme->setAutoSaveMinutes([[oField_Minutes stringValue] UTF8String]);
		}
	[self sync];
}

- (IBAction)aPopUp:(id)sender
{
	UT_ASSERT(m_pActiveScheme);
	if (m_pActiveScheme)
		{
			NSPopUpButton * button = (NSPopUpButton *) sender;

			switch (static_cast<AP_CocoaDialog_OptionsPopUp_ID>([button tag]))
				{
				case CDO_PopUp_CurrentScheme:
					// TODO
					break;
				case CDO_PopUp_CustomDictionary:
					// TODO
					break;
				case CDO_PopUp_PageSize:
					// TODO
					break;
				case CDO_PopUp_Units:
					m_pActiveScheme->setUnitsIndex(static_cast<UT_uint32>([oPopUp_Units indexOfSelectedItem]));
					break;
				default:
					UT_DEBUGMSG(("AP_CocoaDialog_OptionsController -aPopUp: unexpected [sender tag]!\n"));
					break;
				}
		}
	[self sync];
}

- (IBAction)aStepper:(id)sender
{
	UT_ASSERT(m_pActiveScheme);
	if (m_pActiveScheme)
		{
			if ([oStepper_Minutes intValue] > 1.5)
				[oField_Minutes setStringValue:[NSString stringWithUTF8String:(m_pActiveScheme->incrementAutoSaveMinutes())]];
			else
				[oField_Minutes setStringValue:[NSString stringWithUTF8String:(m_pActiveScheme->decrementAutoSaveMinutes())]];
		}
	[oStepper_Minutes setIntValue:1];

	[self sync];
}

- (IBAction)aSwitch:(id)sender
{
	UT_ASSERT(m_pActiveScheme);
	if (m_pActiveScheme)
		{
			NSButton * button = (NSButton *) sender;

			AP_PreferenceScheme::BoolOption bo = m_BOList[static_cast<AP_CocoaDialog_OptionsSwitch_ID>([button tag])];

			bool bValue = (([button state] == NSOnState) ? true : false);

			m_pActiveScheme->setBoolOptionValue(bo, bValue);
		}
	[self sync];
}

- (void)sync
{
	for (int i = 0; i < static_cast<int>(CDO_Switch__count); i++)
		{
			NSButton * button = Switch_List[i];

			AP_PreferenceScheme::BoolOption bo = m_BOList[i];

			bool bCurrent  = false;
			bool bEditable = false;

			if (m_pActiveScheme)
				{
					bCurrent = m_pActiveScheme->getBoolOptionValue(bo, bEditable);
				}
			[button setState:(bCurrent ? NSOnState : NSOffState)];
			[button setEnabled:(bEditable ? YES : NO)];
		}

	// TODO

	if (m_pActiveScheme)
		{
			bool bEditable = false;

			BOOL bAutoSaveEnabled = m_pActiveScheme->getBoolOptionValue(AP_PreferenceScheme::bo_AutoSave, bEditable) ? YES : NO;

			[oStepper_Minutes setEnabled:bAutoSaveEnabled];

			[oField_Minutes setEnabled:bAutoSaveEnabled];
			[oField_Minutes setStringValue:[NSString stringWithUTF8String:(m_pActiveScheme->getAutoSaveMinutes())]];

			[oField_Extension setEnabled:bAutoSaveEnabled];
			[oField_Extension setStringValue:[NSString stringWithUTF8String:(m_pActiveScheme->getAutoSaveExtension())]];

			[oTableView_InterfaceLanguage selectRow:((int) m_pActiveScheme->getUILangIndex()) byExtendingSelection:NO];
			[oTableView_InterfaceLanguage setEnabled:YES];

			[oPopUp_Units setEnabled:YES];
			[oPopUp_Units selectItemAtIndex:((int) m_pActiveScheme->getUnitsIndex())];

			[oButton_Apply    setEnabled:(m_pActiveScheme->currentIsOriginal() ? NO : YES)];
			[oButton_Defaults setEnabled:(m_pActiveScheme->currentIsDefaults() ? NO : YES)];
		}
	else
		{
			[oStepper_Minutes setEnabled:NO];

			[oField_Minutes setEnabled:NO];
			[oField_Minutes setStringValue:@""];

			[oField_Extension setEnabled:NO];
			[oField_Extension setStringValue:@""];

			[oTableView_InterfaceLanguage setEnabled:NO];

			[oPopUp_Units setEnabled:NO];

			[oButton_Apply    setEnabled:NO];
			[oButton_Defaults setEnabled:NO];
		}
	if (m_pSchemeManager)
		{
			[oButton_Close setEnabled:(m_pSchemeManager->haveUnsavedChanges() ? NO : YES)];
		}
	else
		{
			[oButton_Close setEnabled:NO];
		}
}

- (void)setXAPOwner:(AP_CocoaDialog_Options *)owner
{
	m_xap = owner;
}

- (void)setSchemeManager:(AP_PreferenceSchemeManager *)schemeManager
{
	m_pSchemeManager = schemeManager;
	m_pActiveScheme  = schemeManager->getCurrentScheme();

	UT_uint32 count;

	count = m_pSchemeManager->getLanguageCount();

	for (UT_uint32 i = 0; i < count; i++)
		{
			[m_LanguageList addObject:[NSString stringWithUTF8String:(m_pSchemeManager->getNthLanguage(i))]];
		}

	count = m_pSchemeManager->getPopUp_UnitsCount();

	for (UT_uint32 i = 0; i < count; i++)
		{
			[m_UnitsList addObject:[NSString stringWithUTF8String:(m_pSchemeManager->getPopUp_NthUnits(i))]];
		}
}

- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return (int) [m_LanguageList count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
	return [m_LanguageList objectAtIndex:((unsigned) rowIndex)];
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
	UT_ASSERT(m_pActiveScheme);
	if (m_pActiveScheme)
		{
			int row = [oTableView_InterfaceLanguage selectedRow];
			if (row >= 0)
				{
					m_pActiveScheme->setUILangIndex(static_cast<UT_uint32>(row));
				}
		}
	[self sync];
}

- (void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
	[aCell setFont:[NSFont systemFontOfSize:10.0f]];
}

@end
