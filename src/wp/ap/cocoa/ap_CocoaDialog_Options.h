/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003, 2009 Hubert Figuiere
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

#ifndef AP_COCOADIALOG_OPTIONS_H
#define AP_COCOADIALOG_OPTIONS_H

#import <Cocoa/Cocoa.h>

#import "xap_Cocoa_NSTableUtils.h"

#import "ap_CocoaFrame.h"

#include "ap_Dialog_Options.h"
#include "ap_Prefs.h"

class XAP_Frame;

class AP_CocoaDialog_Options;

@interface AP_CocoaDialog_OptionsController : NSWindowController
{
	AP_CocoaDialog_Options *			m_xap;

@public
	IBOutlet NSBox *            m_boxAutoSave;
	IBOutlet NSBox *            m_appStartup;
	IBOutlet NSBox *			oBox_BiDiOptions;
	IBOutlet NSBox *			oBox_General;
	IBOutlet NSBox *			oBox_Ignore;
	IBOutlet NSBox *			oBox_Dictionaries;
	IBOutlet NSBox *			oBox_Grammar;
	IBOutlet NSBox *			oBox_UserInterface;

	IBOutlet NSButton *			oButton_ChooseScreenColor;
	IBOutlet NSButton *			oButton_Close;
	IBOutlet NSButton *			m_buttonDefaults;

	IBOutlet NSColorWell *		oColorWell_Screen;

	IBOutlet NSTextField *      oLabel_WithExtension;
	IBOutlet NSTextField *      oLabel_Minutes;
	IBOutlet NSTextField *      oLabel_Units;
	IBOutlet NSTextField *      m_textAutoSaveFilePeriod;
	IBOutlet NSTextField *      m_textAutoSaveFileExt;
	IBOutlet NSBox *      m_labelOuterQuoteStyle;
	IBOutlet NSBox *      m_labelInnerQuoteStyle;

	IBOutlet NSPopUpButton *	m_menuUnits;
	IBOutlet NSPopUpButton *    m_comboOuterQuote;
	IBOutlet NSPopUpButton *    m_comboInnerQuote;

	IBOutlet NSStepper *		oStepper_Minutes;

	IBOutlet NSButton *			m_checkbuttonSpellCheckAsType;
	IBOutlet NSButton *			m_checkbuttonGrammarCheck;
	IBOutlet NSButton *			m_checkbuttonOtherDirectionRtl;
	IBOutlet NSButton *			m_checkbuttonSpellHideErrors;
	IBOutlet NSButton *         m_checkbuttonSpellSuggest;
	IBOutlet NSButton *			m_checkbuttonSpellNumbers;
	IBOutlet NSButton *			m_checkbuttonSpellUppercase;
	IBOutlet NSButton *         m_checkbuttonSpellMainOnly;
	IBOutlet NSButton *			m_checkbuttonAutoLoadPlugins;
	IBOutlet NSButton *			oSwitch_ScreenColor;
	IBOutlet NSButton *         m_checkbuttonSmartQuotes;
	IBOutlet NSButton *         m_checkbuttonCustomSmartQuotes;
	IBOutlet NSButton *         m_checkbuttonAutoSaveFile;
	IBOutlet NSButton *         m_checkbuttonEnableOverwrite;

	IBOutlet NSTabView *		oTabView;
}
- (id)initFromNib;
- (oneway void)dealloc;
- (void)windowDidLoad;

- (void)setXAPOwner:(AP_CocoaDialog_Options *)owner;

- (IBAction)revertClicked:(id)sender;
- (IBAction)checkboxClicked:(id)sender;
- (IBAction)autoSaveClicked:(id)sender;

- (IBAction)controlChanged:(id)sender;

- (IBAction)closeClicked:(id)sender;
@end

/*****************************************************************/

class AP_CocoaDialog_Options
	: public AP_Dialog_Options
	, public XAP_NotebookDialog
{
public:
	AP_CocoaDialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);

	virtual ~AP_CocoaDialog_Options();

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	id _lookupWidget( tControl cid );
	void _setupSmartQuotesCombos(NSPopUpButton *);

	void setReinit(bool value)
		{
			m_reinit = value;
		}
	bool getReinit()
		{
			return m_reinit;
		}

	virtual void _populateWindowData(void);
protected:

	virtual void _controlEnable( tControl cid, bool value );

	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
		virtual void _set##a(t)


	SET_GATHER			(NotebookPageNum,		int);

	//	// Tabs
	//		// Categories
	//			// Subordinate Controls

	// General

	// User Interface

	SET_GATHER (ViewRulerUnits,	 UT_Dimension);

	// not implemented
	virtual bool _gatherViewCursorBlink(void) { return true; }
	virtual void _setViewCursorBlink(const bool) {}

	// Application Startup

	SET_GATHER (AutoLoadPlugins,	 bool);

	// Documents

	// AutoSave

	SET_GATHER (AutoSaveFile,	      bool);
	virtual void _gatherAutoSaveFilePeriod (      UT_String &stRetVal);
	virtual void _setAutoSaveFilePeriod    (const UT_String &stPeriod);
	virtual void _gatherAutoSaveFileExt    (      UT_String &stRetVal);
	virtual void _setAutoSaveFileExt       (const UT_String &stExt);

	// RTL Text Layout

	SET_GATHER (OtherDirectionRtl,	      bool);

	// Spell Checking

	// General

	SET_GATHER (SpellCheckAsType, bool);
	SET_GATHER (SpellHideErrors,  bool);

	// Ignore Words

	SET_GATHER (SpellUppercase,   bool);
	SET_GATHER (SpellNumbers,     bool);

	// Dictionaries

	SET_GATHER (SpellSuggest,     bool);
	SET_GATHER (SpellMainOnly,    bool);

	// Grammar Check

	SET_GATHER (GrammarCheck,    bool);

	// Smart Quotes

	SET_GATHER (SmartQuotes,   bool);
	SET_GATHER (CustomSmartQuotes,   bool);
	SET_GATHER (OuterQuoteStyle,	gint);
	SET_GATHER (InnerQuoteStyle,	gint);

	// unimplemented UI-wise. We need dummy implementations to satisfy the XP framework, though

	SET_GATHER			(PrefsAutoSave,			bool);
	SET_GATHER			(ViewShowRuler,			bool);
	SET_GATHER			(ViewShowStatusBar,		bool);
	SET_GATHER			(ViewAll,			bool);
	SET_GATHER			(ViewHiddenText,		bool);
	SET_GATHER			(ViewUnprintable,		bool);
	SET_GATHER			(EnableSmoothScrolling,		bool);
	SET_GATHER          (EnableOverwrite,       bool);

#undef SET_GATHER

	virtual void addPage(const XAP_NotebookDialog::Page*)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
private:
	AP_CocoaDialog_OptionsController *	ctrl;
	bool m_reinit;
	//
	//
	bool m_boolEnableSmoothScrolling;
	bool m_boolPrefsAutoSave;
	bool m_boolViewAll;
	bool m_boolViewHiddenText;
	bool m_boolViewShowRuler;
	bool m_boolViewShowStatusBar;
	bool m_boolViewUnprintable;
};

#endif /* AP_COCOADIALOG_OPTIONS_H */
