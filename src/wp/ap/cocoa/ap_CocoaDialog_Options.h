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

// I have no clue what the AP_PreferenceScheme and AP_PreferenceSchemeManager are for -- hub
// But they are moved out of the XP code.

/* AP_PreferenceScheme AP_PreferenceSchemeManager are helper classes for the Options dialog.
 */
class AP_PreferenceSchemeManager;

class ABI_EXPORT AP_PreferenceScheme
{
private:
	struct BoolOptionData
	{
		bool	m_default;	// Default value for option from _builtin_ scheme
		bool	m_original;	// This scheme's original value
		bool	m_current;	// New value for this scheme specified by user
		bool	m_editable;	// Whether the user can change the value (possibly unused)
	};
	struct IntOptionData
	{
		UT_sint32	m_default;	// Default value for option from _builtin_ scheme
		UT_sint32	m_original;	// This scheme's original value
		UT_sint32	m_current;	// New value for this scheme specified by user
		bool		m_editable;	// Whether the user can change the value (possibly unused)
	};
	struct StringOptionData
	{
		const char *	m_default;	// Default value for option from _builtin_ scheme
		const char *	m_original;	// This scheme's original value
		const char *	m_current;	// New value for this scheme specified by user
		bool			m_editable;	// Whether the user can change the value (possibly unused)
	};
public:
	enum BoolOption
		{
			bo_AutoSave = 0,
			bo_CheckSpelling,
			bo_CursorBlink,
			bo_DirectionMarkers,
			bo_DirectionRTL,
			bo_GlyphSaveVisual,
			bo_GlyphShaping,
			bo_HighlightMisspelled,	// NOT (YET?) IMPLEMENTED
			bo_IgnoreNumbered,
			bo_IgnoreUppercase,
			bo_IgnoreURLs,
			bo_CheckGrammar,
			bo_SmartQuotes,
			bo_CustomSmartQuotes,
			bo_LayoutMarks,
			bo_MainDictionaryOnly,	// NOT (YET?) IMPLEMENTED
			bo_Plugins,
			bo_Ruler,
			bo_SaveScheme,			// NOT (YET?) IMPLEMENTED
			bo_ScreenColor,
			bo_StatusBar,
			bo_SuggestCorrections,	// NOT (YET?) IMPLEMENTED
			bo_ToolbarExtra,
			bo_ToolbarFormat,
			bo_ToolbarStandard,
			bo_ToolbarTable,
			bo_ViewAll,				// NOT (YET?) IMPLEMENTED
			bo_ViewHidden,			// NOT (YET?) IMPLEMENTED
			bo__count
		};

	AP_PreferenceScheme(AP_PreferenceSchemeManager * pSchemeManager, XAP_PrefsScheme * pPrefsScheme);

	~AP_PreferenceScheme();

	bool getBoolOptionValue(BoolOption bo, bool & bIsEditable)
	{
		bool bCurrentValue = false;

		if (bo < bo__count)
			{
				bCurrentValue = m_BOData[bo].m_current;
				bIsEditable   = m_BOData[bo].m_editable;
			}
		else
			{
				bIsEditable = false;
			}
		return bCurrentValue;
	}
	void			setBoolOptionValue(BoolOption bo, bool bNewValue);

	bool			currentIsDefaults() const	{ return m_bCurrentIsDefaults; }
	bool			currentIsOriginal() const	{ return m_bCurrentIsOriginal; }

	void			restoreDefaults();	// sets current values to the default
	void			saveChanges();		// saves any changes to the scheme
	void			applySettings();	// update the interface to match the current settings

	/* This imposes a range check on the data, so that minutes is an integer between 1 and 60 inclusive;
	 * all functions return the (new) current value.
	 */
	const char *	getAutoSaveMinutes() const { return m_szAutoSaveMinutes; }
	const char *	setAutoSaveMinutes(const char * szAutoSaveMinutes);
	const char *	incrementAutoSaveMinutes();
	const char *	decrementAutoSaveMinutes();

	const char *	getAutoSaveExtension() const { return m_soAutoSaveExtension.m_current; }
	const char *	setAutoSaveExtension(const char * szAutoSaveExtension);

	UT_uint32		getUILangIndex() const { return static_cast<UT_uint32>(m_ioUILangIndex.m_current); }
	UT_uint32		setUILangIndex(UT_uint32 index);

	UT_uint32		getUnitsIndex() const { return static_cast<UT_uint32>(m_ioUnitsIndex.m_current); }
	UT_uint32		setUnitsIndex(UT_uint32 index);

private:
	void	lookupDefaultOptionValues();
	void	sync();

	AP_PreferenceSchemeManager *	m_pSchemeManager;

	XAP_PrefsScheme *				m_pPrefsScheme;

	bool							m_bCurrentIsDefaults;
	bool							m_bCurrentIsOriginal;

	IntOptionData					m_ioAutoSaveMinutes;
	char							m_szAutoSaveMinutes[8];

	StringOptionData				m_soAutoSaveExtension;

	IntOptionData					m_ioUILangIndex;
	IntOptionData					m_ioUnitsIndex;

	struct BoolOptionData			m_BOData[bo__count];
};

class ABI_EXPORT AP_PreferenceSchemeManager
{
private:
	AP_PreferenceSchemeManager();
public:
	~AP_PreferenceSchemeManager();

	static AP_PreferenceSchemeManager *		create_manager();

	AP_PreferenceScheme *	getCurrentScheme() const { return m_pCurrentScheme; }

	bool					haveUnsavedChanges() const { return m_bHaveUnsavedChanges; }
	void					updateUnsavedChanges(bool bCallerHasUnsavedChanges = false);

	// TODO

	const gchar * getNthLanguage(UT_uint32 n) const
	{
		return ((n + 1) < m_LanguageCount) ? m_ppLanguage[n+1] : 0;
	}
	const gchar * getNthLanguageCode(UT_uint32 n) const
	{
		return ((n + 1) < m_LanguageCount) ? m_ppLanguageCode[n+1] : 0;
	}
	UT_uint32				getLanguageCount() const { return (m_LanguageCount - 1); }
	UT_uint32				getLanguageIndex(const gchar * szLanguageCode) const;

	const char * getPopUp_NthUnits(UT_uint32 n) const
	{
		return ((n < m_PopUp_UnitsCount) ? m_PopUp_UnitsList[n] : 0);
	}
	UT_uint32				getPopUp_UnitsCount() const { return m_PopUp_UnitsCount; }
	UT_uint32				getPopUp_UnitsIndex(const gchar * szUnits) const;

	static const gchar *	reverseTranslate(const char * PopUp_Units);

private:
	void					_constructLanguageArrays();
	void					_constructPopUpArrays();

	bool										m_bHaveUnsavedChanges;

	AP_PreferenceScheme *						m_pCurrentScheme;

	UT_GenericVector<AP_PreferenceScheme *>		m_vecSchemes;

	UT_Language									m_LanguageTable;
	UT_uint32									m_LanguageCount;
	const gchar **							m_ppLanguage;
	const gchar **							m_ppLanguageCode;

	UT_uint32									m_PopUp_UnitsCount;
	char *										m_PopUp_UnitsList[4];
};


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
