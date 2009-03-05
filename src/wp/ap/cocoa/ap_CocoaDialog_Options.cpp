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

void AP_CocoaDialog_Options::runModal(XAP_Frame * /*pFrame*/)
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
	if (![super initWithWindowNibName:@"ap_CocoaDialog_Options"]) {
		return nil;
	}
	m_pSchemeManager = 0;
	m_pActiveScheme  = 0;

	m_LanguageList = 0;
	m_UnitsList    = 0;

	m_LanguageList = [[NSMutableArray alloc] initWithCapacity:128];
	if (!m_LanguageList) {
		[self release];
		return nil;
	}
	m_UnitsList = [[NSMutableArray alloc] initWithCapacity:4];
	if (!m_UnitsList) {
		[self release];
		return nil;
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
	UT_UNUSED(sender);
	// TODO
}

- (IBAction)aField_Extension:(id)sender
{
	UT_UNUSED(sender);
	UT_ASSERT(m_pActiveScheme);
	if (m_pActiveScheme)
		{
			m_pActiveScheme->setAutoSaveExtension([[oField_Extension stringValue] UTF8String]);
		}
	[self sync];
}

- (IBAction)aField_Minutes:(id)sender
{
	UT_UNUSED(sender);
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
	UT_UNUSED(sender);
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
	UT_UNUSED(aTableView);
	return (int) [m_LanguageList count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
	UT_UNUSED(aTableView);
	UT_UNUSED(aTableColumn);
	return [m_LanguageList objectAtIndex:((unsigned) rowIndex)];
}

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
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
	UT_UNUSED(aTableView);
	UT_UNUSED(aTableColumn);
	UT_UNUSED(rowIndex);
	[aCell setFont:[NSFont systemFontOfSize:10.0f]];
}

@end



static const char * s_null_extension = XAP_PREF_DEFAULT_AutoSaveFileExt;

#ifdef FREEP_EXT
#undef FREEP_EXT
#endif
#define FREEP_EXT(S)	do { if (S) { if ((S) != s_null_extension) g_free((void *) (S)); }; (S) = 0; } while (0)

#ifdef CHECK_EXT
#undef CHECK_EXT
#endif
#define CHECK_EXT(S)	do { if (!(S)) (S) = s_null_extension; } while (0)

AP_PreferenceScheme::AP_PreferenceScheme(AP_PreferenceSchemeManager * pSchemeManager, XAP_PrefsScheme * pPrefsScheme) :
	m_pSchemeManager(pSchemeManager),
	m_pPrefsScheme(pPrefsScheme),
	m_bCurrentIsDefaults(false),
	m_bCurrentIsOriginal(true)
{
	UT_uint32 i = 0;
	UT_uint32 count = static_cast<UT_uint32>(bo__count);

	for (i = 0; i < count; i++)
		{
			BoolOption bo = static_cast<BoolOption>(i);

			m_BOData[bo].m_default  = false;
			m_BOData[bo].m_editable = false;
		}
	lookupDefaultOptionValues();

	for (i = 0; i < count; i++)
		{
			BoolOption bo = static_cast<BoolOption>(i);

			m_BOData[bo].m_original = m_BOData[bo].m_default;
		}

	bool bValue = false;

	if (m_pPrefsScheme->getValueBool(XAP_PREF_KEY_AutoSaveFile,							&bValue))
		m_BOData[bo_AutoSave		].m_original = bValue;
	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_AutoSpellCheck,						&bValue))
		m_BOData[bo_CheckSpelling	].m_original = bValue;
	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_AutoGrammarCheck,						&bValue))
		m_BOData[bo_CheckGrammar	].m_original = bValue;
	if (m_pPrefsScheme->getValueBool( XAP_PREF_KEY_SmartQuotesEnable,						&bValue))
		m_BOData[bo_SmartQuotes	].m_original = bValue;
	if (m_pPrefsScheme->getValueBool( XAP_PREF_KEY_CustomSmartQuotes,						&bValue))
		m_BOData[bo_CustomSmartQuotes	].m_original = bValue;
	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_CursorBlink,							&bValue))
		m_BOData[bo_CursorBlink		].m_original = bValue;
	if (m_pPrefsScheme->getValueBool(XAP_PREF_KEY_DirMarkerAfterClosingParenthesis,		&bValue))
		m_BOData[bo_DirectionMarkers].m_original = bValue;
	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_DefaultDirectionRtl,					&bValue))
		m_BOData[bo_DirectionRTL	].m_original = bValue;
	if (m_pPrefsScheme->getValueBool(XAP_PREF_KEY_SaveContextGlyphs,					&bValue))
		m_BOData[bo_GlyphSaveVisual	].m_original = bValue;

	// NOT (YET?) IMPLEMENTED: if (m_pPrefsScheme->getValueBool("",&bValue)) m_BOData[bo_HighlightMisspelled].m_original = bValue;

	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_SpellCheckNumbers,					&bValue))
		m_BOData[bo_IgnoreNumbered	].m_original = bValue; // TODO: Is this reversed?
	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_SpellCheckCaps,						&bValue))
		m_BOData[bo_IgnoreUppercase	].m_original = bValue; // TODO: Is this reversed?
	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_SpellCheckInternet,					&bValue))
		m_BOData[bo_IgnoreURLs		].m_original = bValue; // TODO: Is this reversed?
	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_ParaVisible,							&bValue))
		m_BOData[bo_LayoutMarks		].m_original = bValue;

	// NOT (YET?) IMPLEMENTED: if (m_pPrefsScheme->getValueBool("",&bValue)) m_BOData[bo_MainDictionaryOnly	].m_original = bValue;

	if (m_pPrefsScheme->getValueBool(XAP_PREF_KEY_AutoLoadPlugins,						&bValue))
		m_BOData[bo_Plugins			].m_original = bValue;
	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_RulerVisible,							&bValue))
		m_BOData[bo_Ruler			].m_original = bValue;

	// NOT (YET?) IMPLEMENTED: if (m_pPrefsScheme->getValueBool("",&bValue)) m_BOData[bo_SaveScheme			].m_original = bValue;

	// TODO: if (m_pPrefsScheme->getValueBool("",&bValue)) m_BOData[bo_ScreenColor			].m_original = bValue;

	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_StatusBarVisible,						&bValue))
		m_BOData[bo_StatusBar		].m_original = bValue;

	// NOT (YET?) IMPLEMENTED: if (m_pPrefsScheme->getValueBool("",&bValue)) m_BOData[bo_SuggestCorrections	].m_original = bValue;

	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_ExtraBarVisible,						&bValue))
		m_BOData[bo_ToolbarExtra	].m_original = bValue;
	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_FormatBarVisible,						&bValue))
		m_BOData[bo_ToolbarFormat	].m_original = bValue;
	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_StandardBarVisible,					&bValue))
		m_BOData[bo_ToolbarStandard	].m_original = bValue;
	if (m_pPrefsScheme->getValueBool( AP_PREF_KEY_TableBarVisible,						&bValue))
		m_BOData[bo_ToolbarTable	].m_original = bValue;

	// NOT (YET?) IMPLEMENTED: if (m_pPrefsScheme->getValueBool("",&bValue)) m_BOData[bo_ViewAll			].m_original = bValue;
	// NOT (YET?) IMPLEMENTED: if (m_pPrefsScheme->getValueBool("",&bValue)) m_BOData[bo_ViewHidden			].m_original = bValue;

	for (i = 0; i < count; i++)
		{
			BoolOption bo = static_cast<BoolOption>(i);

			m_BOData[bo].m_current = m_BOData[bo].m_original;
		}

	const gchar * pszValue = 0;

	/* Auto-Save Period
	 */
	m_ioAutoSaveMinutes.m_original = m_ioAutoSaveMinutes.m_default;

	pszValue = 0;
	if (m_pPrefsScheme->getValue(XAP_PREF_KEY_AutoSaveFilePeriod, &pszValue))
		{
			UT_ASSERT(pszValue);

			int ASM = static_cast<int>(m_ioAutoSaveMinutes.m_default);

			if (pszValue)
				if (sscanf(pszValue, "%d", &ASM) == 1)
					{
						if (ASM < 1)
							ASM = 1;
						if (ASM > 60)
							ASM = 60;

						m_ioAutoSaveMinutes.m_original = static_cast<UT_sint32>(ASM);
					}
		}
	m_ioAutoSaveMinutes.m_current = m_ioAutoSaveMinutes.m_original;

	sprintf(m_szAutoSaveMinutes, "%d", static_cast<int>(m_ioAutoSaveMinutes.m_current));
		
	/* Auto-Save Extension
	 */
	m_soAutoSaveExtension.m_original = 0;
	m_soAutoSaveExtension.m_current  = 0;

	pszValue = 0;
	if (m_pPrefsScheme->getValue(XAP_PREF_KEY_AutoSaveFileExt, &pszValue))
		{
			UT_ASSERT(pszValue);
			if (pszValue)
				{
					m_soAutoSaveExtension.m_original = g_strdup(pszValue);
					CHECK_EXT(m_soAutoSaveExtension.m_original);
				}
		}
	if (m_soAutoSaveExtension.m_original == 0)
		{
			m_soAutoSaveExtension.m_original = g_strdup(m_soAutoSaveExtension.m_default);
			CHECK_EXT(m_soAutoSaveExtension.m_original);
		}
	m_soAutoSaveExtension.m_current = g_strdup(m_soAutoSaveExtension.m_original);
	CHECK_EXT(m_soAutoSaveExtension.m_current);

	/* StringSet
	 */
	m_ioUILangIndex.m_original = m_ioUILangIndex.m_default;

	pszValue = 0;
	if (m_pPrefsScheme->getValue(AP_PREF_KEY_StringSet, &pszValue))
		{
			UT_ASSERT(pszValue);
			if (pszValue)
				m_ioUILangIndex.m_original = static_cast<UT_sint32>(m_pSchemeManager->getLanguageIndex(pszValue));
		}
	m_ioUILangIndex.m_current = m_ioUILangIndex.m_original;

	/* PopUp Units
	 */
	m_ioUnitsIndex.m_original = m_ioUnitsIndex.m_default;

	pszValue = 0;
	if (m_pPrefsScheme->getValue(AP_PREF_KEY_RulerUnits, &pszValue))
		{
			UT_ASSERT(pszValue);
			if (pszValue)
				m_ioUnitsIndex.m_original = static_cast<UT_sint32>(m_pSchemeManager->getPopUp_UnitsIndex(pszValue));
		}
	m_ioUnitsIndex.m_current = m_ioUnitsIndex.m_original;

	// TODO

	sync();
}

AP_PreferenceScheme::~AP_PreferenceScheme()
{
	FREEP_EXT(m_soAutoSaveExtension.m_default);
	FREEP_EXT(m_soAutoSaveExtension.m_original);
	FREEP_EXT(m_soAutoSaveExtension.m_current);
	// 
}

void AP_PreferenceScheme::setBoolOptionValue(BoolOption bo, bool bNewValue)
{
	if ((bo < 0) || (bo >= bo__count))
		{
			UT_DEBUGMSG(("AP_PreferenceScheme::setBoolOptionValue: BoolOption value out of range!\n"));
			return;
		}
	if (!m_BOData[bo].m_editable)
		{
			UT_DEBUGMSG(("AP_PreferenceScheme::setBoolOptionValue: BoolOption value is not editable!\n"));
			return;
		}
	m_BOData[bo].m_current = bNewValue;

	sync();
}

/* sets current values to the default
 */
void AP_PreferenceScheme::restoreDefaults()
{
	if (m_bCurrentIsDefaults) return;

	for (UT_uint32 i = 0; i < static_cast<UT_uint32>(bo__count); i++)
		{
			BoolOption bo = static_cast<BoolOption>(i);

			m_BOData[bo].m_current = m_BOData[bo].m_default;
		}

	/* Auto-Save Period
	 */
	m_ioAutoSaveMinutes.m_current = m_ioAutoSaveMinutes.m_default;

	sprintf(m_szAutoSaveMinutes, "%d", static_cast<int>(m_ioAutoSaveMinutes.m_current));
		
	/* Auto-Save Extension
	 */
	FREEP_EXT(m_soAutoSaveExtension.m_current);

	m_soAutoSaveExtension.m_current = g_strdup(m_soAutoSaveExtension.m_default);
	CHECK_EXT(m_soAutoSaveExtension.m_current);

	/* StringSet
	 */
	m_ioUILangIndex.m_current = m_ioUILangIndex.m_default;

	/* PopUp Units
	 */
	m_ioUnitsIndex.m_current = m_ioUnitsIndex.m_default;

	// TODO

	sync();
}

/* saves any changes to the scheme
 */
void AP_PreferenceScheme::saveChanges()
{
	if (m_bCurrentIsOriginal) return;

	BoolOption bo;

	bo = bo_AutoSave;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool(XAP_PREF_KEY_AutoSaveFile,						 m_BOData[bo].m_current);

	bo = bo_CheckSpelling;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_AutoSpellCheck,					 m_BOData[bo].m_current);


	bo = bo_CheckGrammar;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_AutoGrammarCheck,					 m_BOData[bo].m_current);

	bo = bo_SmartQuotes;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( XAP_PREF_KEY_SmartQuotesEnable,					 m_BOData[bo].m_current);

	bo = bo_CustomSmartQuotes;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( XAP_PREF_KEY_CustomSmartQuotes,					 m_BOData[bo].m_current);

	bo = bo_CursorBlink;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_CursorBlink,						 m_BOData[bo].m_current);

	bo = bo_DirectionMarkers;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool(XAP_PREF_KEY_DirMarkerAfterClosingParenthesis,	 m_BOData[bo].m_current);

	bo = bo_DirectionRTL;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_DefaultDirectionRtl,				 m_BOData[bo].m_current);

	bo = bo_GlyphSaveVisual;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool(XAP_PREF_KEY_SaveContextGlyphs,				 m_BOData[bo].m_current);

	// NOT (YET?) IMPLEMENTED: m_pPrefsScheme->setValueBool("", m_BOData[bo_HighlightMisspelled	].m_current);

	bo = bo_IgnoreNumbered;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_SpellCheckNumbers,				 m_BOData[bo].m_current); // TODO: Is this reversed?

	bo = bo_IgnoreUppercase;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_SpellCheckCaps,					 m_BOData[bo].m_current); // TODO: Is this reversed?

	bo = bo_IgnoreURLs;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_SpellCheckInternet,				 m_BOData[bo].m_current); // TODO: Is this reversed?

	bo = bo_LayoutMarks;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_ParaVisible,						 m_BOData[bo].m_current);

	// NOT (YET?) IMPLEMENTED: m_pPrefsScheme->setValueBool("", m_BOData[bo_MainDictionaryOnly	].m_current);

	bo = bo_Plugins;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool(XAP_PREF_KEY_AutoLoadPlugins,					 m_BOData[bo].m_current);

	bo = bo_Ruler;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_RulerVisible,						 m_BOData[bo].m_current);

	// NOT (YET?) IMPLEMENTED: m_pPrefsScheme->setValueBool("", m_BOData[bo_SaveScheme			].m_current);

	// TODO: m_pPrefsScheme->setValueBool("", m_BOData[bo_ScreenColor				].m_current);

	bo = bo_StatusBar;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_StatusBarVisible,					 m_BOData[bo].m_current);

	// NOT (YET?) IMPLEMENTED: m_pPrefsScheme->setValueBool("", m_BOData[bo_SuggestCorrections	].m_current);

	bo = bo_ToolbarExtra;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_ExtraBarVisible,					 m_BOData[bo].m_current);

	bo = bo_ToolbarFormat;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_FormatBarVisible,					 m_BOData[bo].m_current);

	bo = bo_ToolbarStandard;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_StandardBarVisible,				 m_BOData[bo].m_current);

	bo = bo_ToolbarTable;
	if (m_BOData[bo].m_current != m_BOData[bo].m_original)
		m_pPrefsScheme->setValueBool( AP_PREF_KEY_TableBarVisible,					 m_BOData[bo].m_current);

	// NOT (YET?) IMPLEMENTED: m_pPrefsScheme->setValueBool("", m_BOData[bo_ViewAll				].m_current);
	// NOT (YET?) IMPLEMENTED: m_pPrefsScheme->setValueBool("", m_BOData[bo_ViewHidden			].m_current);

	for (UT_uint32 i = 0; i < static_cast<UT_uint32>(bo__count); i++)
	{
		BoolOption bo1 = static_cast<BoolOption>(i);

		m_BOData[bo1].m_original = m_BOData[bo1].m_current;
	}

	/* Auto-Save Period
	 */
	m_pPrefsScheme->setValue(XAP_PREF_KEY_AutoSaveFilePeriod, m_szAutoSaveMinutes);

	m_ioAutoSaveMinutes.m_original = m_ioAutoSaveMinutes.m_current;

	/* Auto-Save Extension
	 */
	m_pPrefsScheme->setValue(XAP_PREF_KEY_AutoSaveFileExt, m_soAutoSaveExtension.m_current);

	FREEP_EXT(m_soAutoSaveExtension.m_original);

	m_soAutoSaveExtension.m_original = g_strdup(m_soAutoSaveExtension.m_current);
	CHECK_EXT(m_soAutoSaveExtension.m_original);

	const gchar * szValue = 0;

	/* StringSet
	 */
	szValue = m_pSchemeManager->getNthLanguageCode(static_cast<UT_uint32>(m_ioUILangIndex.m_current));
	m_pPrefsScheme->setValue(AP_PREF_KEY_StringSet, szValue);

	m_ioUILangIndex.m_original = m_ioUILangIndex.m_current;

	/* PopUp Units
	 */
	szValue = m_pSchemeManager->getPopUp_NthUnits(static_cast<UT_uint32>(m_ioUnitsIndex.m_current));
	m_pPrefsScheme->setValue(AP_PREF_KEY_RulerUnits, AP_PreferenceSchemeManager::reverseTranslate(szValue));

	m_ioUnitsIndex.m_original = m_ioUnitsIndex.m_current;

	// TODO

	sync();
}

/* update the interface to match the current settings
 */
void AP_PreferenceScheme::applySettings()
{
	// TODO
}

const char * AP_PreferenceScheme::setAutoSaveMinutes(const char * szAutoSaveMinutes)
{
	int ASM = 5;

	if (szAutoSaveMinutes)
		if (sscanf(szAutoSaveMinutes, "%d", &ASM) == 1)
			{
				if (ASM < 1)
					ASM = 1;
				if (ASM > 60)
					ASM = 60;

				m_ioAutoSaveMinutes.m_current = static_cast<UT_sint32>(ASM);

				sprintf(m_szAutoSaveMinutes, "%d", static_cast<int>(m_ioAutoSaveMinutes.m_current));

				sync();
			}
	return m_szAutoSaveMinutes;
}

const char * AP_PreferenceScheme::incrementAutoSaveMinutes()
{
	if (m_ioAutoSaveMinutes.m_current < 60)
		{
			m_ioAutoSaveMinutes.m_current++;

			sprintf(m_szAutoSaveMinutes, "%d", static_cast<int>(m_ioAutoSaveMinutes.m_current));

			sync();
		}
	return m_szAutoSaveMinutes;
}

const char * AP_PreferenceScheme::decrementAutoSaveMinutes()
{
	if (m_ioAutoSaveMinutes.m_current > 1)
		{
			m_ioAutoSaveMinutes.m_current--;

			sprintf(m_szAutoSaveMinutes, "%d", static_cast<int>(m_ioAutoSaveMinutes.m_current));

			sync();
		}
	return m_szAutoSaveMinutes;
}

const char * AP_PreferenceScheme::setAutoSaveExtension(const char * szAutoSaveExtension)
{
	if (szAutoSaveExtension)
		{
			FREEP_EXT(m_soAutoSaveExtension.m_current);

			m_soAutoSaveExtension.m_current = g_strdup(szAutoSaveExtension);
			CHECK_EXT(m_soAutoSaveExtension.m_current);

			sync();
		}
	return m_soAutoSaveExtension.m_current;
}

UT_uint32 AP_PreferenceScheme::setUILangIndex(UT_uint32 index)
{
	if (index < m_pSchemeManager->getLanguageCount())
		{
			m_ioUILangIndex.m_current = static_cast<UT_sint32>(index);

			sync();
		}
	return m_ioUILangIndex.m_current;
}

UT_uint32 AP_PreferenceScheme::setUnitsIndex(UT_uint32 index)
{
	if (index < m_pSchemeManager->getPopUp_UnitsCount())
		{
			m_ioUnitsIndex.m_current = static_cast<UT_sint32>(index);

			sync();
		}
	return m_ioUnitsIndex.m_current;
}

void AP_PreferenceScheme::lookupDefaultOptionValues()
{
	XAP_Prefs * pPrefs = XAP_App::getApp()->getPrefs();
	UT_ASSERT(pPrefs);
	if (!pPrefs) return;

	XAP_PrefsScheme * pScheme = pPrefs->getScheme(pPrefs->getBuiltinSchemeName());
	UT_ASSERT(pScheme);
	if (!pScheme) return;

	pScheme->getValueBool(XAP_PREF_KEY_AutoSaveFile,						&(m_BOData[bo_AutoSave			].m_default));
	pScheme->getValueBool( AP_PREF_KEY_AutoSpellCheck,						&(m_BOData[bo_CheckSpelling		].m_default));
	pScheme->getValueBool( AP_PREF_KEY_AutoGrammarCheck,						&(m_BOData[bo_CheckGrammar		].m_default));
	pScheme->getValueBool( XAP_PREF_KEY_SmartQuotesEnable,						&(m_BOData[bo_SmartQuotes		].m_default));
	pScheme->getValueBool( XAP_PREF_KEY_CustomSmartQuotes,						&(m_BOData[bo_CustomSmartQuotes		].m_default));
	pScheme->getValueBool( AP_PREF_KEY_CursorBlink,							&(m_BOData[bo_CursorBlink		].m_default));
	pScheme->getValueBool(XAP_PREF_KEY_DirMarkerAfterClosingParenthesis,	&(m_BOData[bo_DirectionMarkers	].m_default));
	pScheme->getValueBool( AP_PREF_KEY_DefaultDirectionRtl,					&(m_BOData[bo_DirectionRTL		].m_default));
	pScheme->getValueBool(XAP_PREF_KEY_SaveContextGlyphs,					&(m_BOData[bo_GlyphSaveVisual	].m_default));
	// NOT (YET?) IMPLEMENTED: pScheme->getValueBool("",&(m_BOData[bo_HighlightMisspelled	].m_default));
	pScheme->getValueBool( AP_PREF_KEY_SpellCheckNumbers,					&(m_BOData[bo_IgnoreNumbered	].m_default)); // TODO: Is this reversed?
	pScheme->getValueBool( AP_PREF_KEY_SpellCheckCaps,						&(m_BOData[bo_IgnoreUppercase	].m_default)); // TODO: Is this reversed?
	pScheme->getValueBool( AP_PREF_KEY_SpellCheckInternet,					&(m_BOData[bo_IgnoreURLs		].m_default)); // TODO: Is this reversed?
	pScheme->getValueBool( AP_PREF_KEY_ParaVisible,							&(m_BOData[bo_LayoutMarks		].m_default));
	// NOT (YET?) IMPLEMENTED: pScheme->getValueBool("",&(m_BOData[bo_MainDictionaryOnly	].m_default));
	pScheme->getValueBool(XAP_PREF_KEY_AutoLoadPlugins,						&(m_BOData[bo_Plugins			].m_default));
	pScheme->getValueBool( AP_PREF_KEY_RulerVisible,						&(m_BOData[bo_Ruler				].m_default));
	// NOT (YET?) IMPLEMENTED: pScheme->getValueBool("",&(m_BOData[bo_SaveScheme			].m_default));
	// TODO: pScheme->getValueBool("",&(m_BOData[bo_ScreenColor			].m_default));
	pScheme->getValueBool( AP_PREF_KEY_StatusBarVisible,					&(m_BOData[bo_StatusBar			].m_default));
	// NOT (YET?) IMPLEMENTED: pScheme->getValueBool("",&(m_BOData[bo_SuggestCorrections	].m_default));
	pScheme->getValueBool( AP_PREF_KEY_ExtraBarVisible,						&(m_BOData[bo_ToolbarExtra		].m_default));
	pScheme->getValueBool( AP_PREF_KEY_FormatBarVisible,					&(m_BOData[bo_ToolbarFormat		].m_default));
	pScheme->getValueBool( AP_PREF_KEY_StandardBarVisible,					&(m_BOData[bo_ToolbarStandard	].m_default));
	pScheme->getValueBool( AP_PREF_KEY_TableBarVisible,						&(m_BOData[bo_ToolbarTable		].m_default));
	// NOT (YET?) IMPLEMENTED: pScheme->getValueBool("",&(m_BOData[bo_ViewAll				].m_default));
	// NOT (YET?) IMPLEMENTED: pScheme->getValueBool("",&(m_BOData[bo_ViewHidden			].m_default));

	// m_BOData[bo_IgnoreNumbered	].m_default = !m_BOData[bo_IgnoreNumbered	].m_default;
	// m_BOData[bo_IgnoreUppercase	].m_default = !m_BOData[bo_IgnoreUppercase	].m_default;
	// m_BOData[bo_IgnoreURLs		].m_default = !m_BOData[bo_IgnoreURLs		].m_default;

	const gchar * pszValue = 0;

	/* Auto-Save Period
	 */
	int ASM = 5;

	pszValue = 0;
	pScheme->getValue(XAP_PREF_KEY_AutoSaveFilePeriod, &pszValue);
	UT_ASSERT(pszValue);
	if (pszValue)
		sscanf(pszValue, "%d", &ASM);
	if (ASM < 1)
		ASM = 1;
	if (ASM > 60)
		ASM = 60;

	m_ioAutoSaveMinutes.m_default = static_cast<UT_sint32>(ASM);

	/* Auto-Save Extension
	 */
	m_soAutoSaveExtension.m_default = 0;

	pszValue = 0;
	pScheme->getValue(XAP_PREF_KEY_AutoSaveFileExt, &pszValue);
	UT_ASSERT(pszValue);
	if (pszValue)
		m_soAutoSaveExtension.m_default = g_strdup(pszValue);

	CHECK_EXT(m_soAutoSaveExtension.m_default);

	/* StringSet
	 */
	pszValue = 0;
	pScheme->getValue(AP_PREF_KEY_StringSet, &pszValue);
	UT_ASSERT(pszValue);
	if (pszValue)
		m_ioUILangIndex.m_default = static_cast<UT_sint32>(m_pSchemeManager->getLanguageIndex(pszValue));
	else
		m_ioUILangIndex.m_default = static_cast<UT_sint32>(m_pSchemeManager->getLanguageIndex(AP_PREF_DEFAULT_StringSet));

	/* PopUp Units
	 */
	pszValue = 0;
	pScheme->getValue(AP_PREF_KEY_RulerUnits, &pszValue);
	UT_ASSERT(pszValue);
	if (pszValue)
		m_ioUnitsIndex.m_default = static_cast<UT_sint32>(m_pSchemeManager->getPopUp_UnitsIndex(pszValue));
	else
		m_ioUnitsIndex.m_default = static_cast<UT_sint32>(m_pSchemeManager->getPopUp_UnitsIndex(AP_PREF_DEFAULT_RulerUnits));

	// TODO
}

void AP_PreferenceScheme::sync()
{
	bool bPreviousWasOriginal = m_bCurrentIsOriginal;

	m_bCurrentIsDefaults = true;
	m_bCurrentIsOriginal = true;

	for (UT_uint32 i = 0; i < static_cast<UT_uint32>(bo__count); i++)
		{
			BoolOption bo = static_cast<BoolOption>(i);

			m_BOData[bo].m_editable = true;

			if (m_BOData[bo].m_current != m_BOData[bo].m_default)
				m_bCurrentIsDefaults = false;
			if (m_BOData[bo].m_current != m_BOData[bo].m_original)
				m_bCurrentIsOriginal = false;
		}

	/* Auto-Save Period
	 */
	if (m_ioAutoSaveMinutes.m_current != m_ioAutoSaveMinutes.m_default)
		m_bCurrentIsDefaults = false;
	if (m_ioAutoSaveMinutes.m_current != m_ioAutoSaveMinutes.m_original)
		m_bCurrentIsOriginal = false;

	/* Auto-Save Extension
	 */
	if (strcmp(m_soAutoSaveExtension.m_current, m_soAutoSaveExtension.m_default))
		m_bCurrentIsDefaults = false;
	if (strcmp(m_soAutoSaveExtension.m_current, m_soAutoSaveExtension.m_original))
		m_bCurrentIsOriginal = false;

	/* StringSet
	 */
	if (m_ioUILangIndex.m_current != m_ioUILangIndex.m_default)
		m_bCurrentIsDefaults = false;
	if (m_ioUILangIndex.m_current != m_ioUILangIndex.m_original)
		m_bCurrentIsOriginal = false;

	/* PopUp Units
	 */
	if (m_ioUnitsIndex.m_current != m_ioUnitsIndex.m_default)
		m_bCurrentIsDefaults = false;
	if (m_ioUnitsIndex.m_current != m_ioUnitsIndex.m_original)
		m_bCurrentIsOriginal = false;

	if (bPreviousWasOriginal != m_bCurrentIsOriginal)
		{
			m_pSchemeManager->updateUnsavedChanges(!m_bCurrentIsOriginal);
		}

	// NOT (YET?) IMPLEMENTED:
	m_BOData[bo_HighlightMisspelled	].m_editable = false;
	m_BOData[bo_MainDictionaryOnly	].m_editable = false;
	m_BOData[bo_SaveScheme			].m_editable = false;
	m_BOData[bo_SuggestCorrections	].m_editable = false;
	m_BOData[bo_ViewAll				].m_editable = false;
	m_BOData[bo_ViewHidden			].m_editable = false;

	// TODO:
	m_BOData[bo_ScreenColor			].m_editable = false;
}


AP_PreferenceSchemeManager::AP_PreferenceSchemeManager() :
	m_bHaveUnsavedChanges(false),
	m_LanguageCount(0),
	m_ppLanguage(0),
	m_ppLanguageCode(0)
{
	_constructLanguageArrays();
	_constructPopUpArrays();
	// 
}

AP_PreferenceSchemeManager::~AP_PreferenceSchemeManager()
{
	UT_uint32 count = m_vecSchemes.getItemCount();

	for (UT_uint32 i = 0; i < count; i++)
		{
			AP_PreferenceScheme * pScheme = m_vecSchemes.getNthItem(i);
			DELETEP(pScheme);
		}

	DELETEPV(m_ppLanguage);
	DELETEPV(m_ppLanguageCode);

	while (m_PopUp_UnitsCount)
		{
			g_free(m_PopUp_UnitsList[--m_PopUp_UnitsCount]);
		}
}

AP_PreferenceSchemeManager * AP_PreferenceSchemeManager::create_manager()
{
	AP_PreferenceSchemeManager * manager = 0;
	manager = new AP_PreferenceSchemeManager;
	if (manager)
	{
		// TODO: we need all the schemes, not just the current...

		XAP_Prefs * pPrefs = XAP_App::getApp()->getPrefs();
		UT_ASSERT(pPrefs);
		if (!pPrefs)
		{
			DELETEP(manager);
			return 0;
		}
		XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
		UT_ASSERT(pScheme);
		if (!pScheme)
		{
			DELETEP(manager);
			return 0;
		}

		AP_PreferenceScheme * current = 0;

		current = new AP_PreferenceScheme(manager, pScheme);

		if (current)
		{
			manager->m_vecSchemes.addItem(current);
			manager->m_pCurrentScheme = current;
		}
	}
	return manager;
}

void AP_PreferenceSchemeManager::updateUnsavedChanges(bool bCallerHasUnsavedChanges)
{
	if (bCallerHasUnsavedChanges)
	{
		m_bHaveUnsavedChanges = true;
		return;
	}

	UT_uint32 count = m_vecSchemes.getItemCount();

	m_bHaveUnsavedChanges = false;

	for (UT_uint32 i = 0; i < count; i++)
	{
		AP_PreferenceScheme * pScheme = m_vecSchemes.getNthItem(i);
		if (!pScheme->currentIsOriginal())
		{
			m_bHaveUnsavedChanges = true;
			break;
		}
	}
}

UT_uint32 AP_PreferenceSchemeManager::getLanguageIndex(const gchar * szLanguageCode) const
{
	UT_uint32 index = 1;

	if (szLanguageCode)
		for (UT_uint32 i = 1; i < m_LanguageCount; i++)
			if (strcmp(m_ppLanguageCode[i], szLanguageCode) == 0)
				{
					index = i;
					break;
				}
	return (index - 1);
}

static int s_compareQ(const void * a, const void * b)
{
	const gchar ** A = (const gchar **) a;
	const gchar ** B = (const gchar **) b;

	return g_utf8_collate(*A,*B);
}

void AP_PreferenceSchemeManager::_constructLanguageArrays()
{
	m_LanguageCount = m_LanguageTable.getCount();

	const gchar ** ppLanguageTemp = new const gchar * [m_LanguageCount];

	m_ppLanguage     = new const gchar * [m_LanguageCount];
	m_ppLanguageCode = new const gchar * [m_LanguageCount];

	UT_ASSERT(ppLanguageTemp && m_ppLanguage && m_ppLanguageCode);

	if (!(ppLanguageTemp && m_ppLanguage && m_ppLanguageCode))
		{
			DELETEPV(  ppLanguageTemp);
			DELETEPV(m_ppLanguage);
			DELETEPV(m_ppLanguageCode);

			m_LanguageCount = 0; // grr...
			return;
		}

	UT_uint32 i;
	UT_uint32 nDontSort = 0;
	UT_uint32 nSort     = 0;

	for(i = 0; i < m_LanguageCount; i++)
		{
			if (m_LanguageTable.getNthId(i) == XAP_STRING_ID_LANG_0) // Unsorted languages
				{
					m_ppLanguage[nDontSort] = m_LanguageTable.getNthLangName(i);
					nDontSort++;
				}
			else
				{
					ppLanguageTemp[nSort] = m_LanguageTable.getNthLangName(i);
					nSort++;
				}
		}

	/* sort the temporary array
	 */
	qsort(ppLanguageTemp, m_LanguageCount-nDontSort, sizeof(gchar *), s_compareQ);

	/* Copy the sorted codes and a ssign each language its code
	 */
	for (UT_uint32 nLang = 0; nLang < m_LanguageCount; nLang++)
		{
			if (nLang >= nDontSort)
				{
					m_ppLanguage[nLang] = ppLanguageTemp[nLang-nDontSort];
				}
			for(i = 0; i < m_LanguageCount; i++)
				{
					if (strcmp (m_ppLanguage[nLang], m_LanguageTable.getNthLangName(i)) == 0)
						{
							m_ppLanguageCode[nLang] = m_LanguageTable.getNthLangCode(i);
							break;
						}
				}
		}
	DELETEPV(ppLanguageTemp);
}

/* TODO: make this dynamic!
 */
static const gchar * s_internal_units[5] = { "in", "cm", "mm", "pt", "pi" };

UT_uint32 AP_PreferenceSchemeManager::getPopUp_UnitsIndex(const gchar * szUnits) const
{
	UT_uint32 index = 0;

	if (szUnits)
		for (UT_uint32 i = 0; i < 5; i++)
			if ((strcmp(s_internal_units[i], szUnits) == 0) || (strcmp(m_PopUp_UnitsList[i], szUnits) == 0))
				{
					index = i;
					break;
				}
	return index;
}

const gchar * AP_PreferenceSchemeManager::reverseTranslate(const char * PopUp_Units)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	const gchar * tmp = 0;

	tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_inch);
	if (tmp && (strcmp (tmp, PopUp_Units) == 0))
		return s_internal_units[0];
	tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_cm);
	if (tmp && (strcmp (tmp, PopUp_Units) == 0))
		return s_internal_units[1];
	tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_mm);
	if (tmp && (strcmp (tmp, PopUp_Units) == 0))
		return s_internal_units[2];
	tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_points);
	if (tmp && (strcmp (tmp, PopUp_Units) == 0))
		return s_internal_units[3];
	tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_pica);
	if (tmp && (strcmp (tmp, PopUp_Units) == 0))
		return s_internal_units[4];

	return s_internal_units[0];
}

void AP_PreferenceSchemeManager::_constructPopUpArrays()
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	const gchar * tmp = 0;

	m_PopUp_UnitsCount = 0;

	tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_inch);
	if (tmp)
		m_PopUp_UnitsList[m_PopUp_UnitsCount++] = g_strdup(tmp);
	tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_cm);
	if (tmp)
		m_PopUp_UnitsList[m_PopUp_UnitsCount++] = g_strdup(tmp);
	tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_mm);
	if (tmp)
		m_PopUp_UnitsList[m_PopUp_UnitsCount++] = g_strdup(tmp);
	tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_points);
	if (tmp)
		m_PopUp_UnitsList[m_PopUp_UnitsCount++] = g_strdup(tmp);
	tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_pica);
	if (tmp)
		m_PopUp_UnitsList[m_PopUp_UnitsCount++] = g_strdup(tmp);

	UT_ASSERT(m_PopUp_UnitsCount == 5); // must match size of s_internal_units[] // TODO: make dynamic!

	// TODO
}
