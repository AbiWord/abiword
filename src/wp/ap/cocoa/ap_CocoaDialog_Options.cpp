/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002-2003, 2009 Hubert Figuiere
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
#include "xap_EncodingManager.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"

#import "ap_CocoaDialog_Options.h"


XAP_Dialog * AP_CocoaDialog_Options::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id dlgid)
{
    AP_CocoaDialog_Options * p = new AP_CocoaDialog_Options(pFactory,dlgid);
    return p;
}

AP_CocoaDialog_Options::AP_CocoaDialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid)
	: AP_Dialog_Options(pDlgFactory, dlgid)
	, ctrl(nil)
	, m_reinit(false)
	, m_boolEnableSmoothScrolling(false)
	, m_boolPrefsAutoSave(false)
	, m_boolViewAll(false)
	, m_boolViewHiddenText(false)
	, m_boolViewShowRuler(false)
	, m_boolViewShowStatusBar(false)
	, m_boolViewUnprintable(false)
{
	// 
}

AP_CocoaDialog_Options::~AP_CocoaDialog_Options()
{
	// 
	if(ctrl) {
		[ctrl release];
	}
}

void AP_CocoaDialog_Options::runModal(XAP_Frame * pFrame)
{
    m_pFrame = pFrame;

	ctrl = [[AP_CocoaDialog_OptionsController alloc] initFromNib];
	if (ctrl)
	{
		[ctrl setXAPOwner:this];

		[ctrl window];
		
		_populateWindowData();

		[NSApp runModalForWindow:[ctrl window]];

		[ctrl close];
		[ctrl release];
		ctrl = nil;
	}
}


void AP_CocoaDialog_Options::_setupSmartQuotesCombos(NSPopUpButton * popup)
{
	unichar wszDisplayString[4];
	NSMenu * menu = [popup menu];
	[popup removeAllItems];
	for (size_t i = 0; XAP_EncodingManager::smartQuoteStyles[i].leftQuote != (UT_UCSChar)0; ++i)
	{
		wszDisplayString[0] = XAP_EncodingManager::smartQuoteStyles[i].leftQuote;
		wszDisplayString[1] = (unichar)'O';
		wszDisplayString[2] = XAP_EncodingManager::smartQuoteStyles[i].rightQuote;
		wszDisplayString[3] = (unichar)0;
		
		NSMenuItem * menuItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithCharacters:wszDisplayString length:3] 
			action:nil	keyEquivalent:@""];
		[menuItem setTag:i];
		[menu addItem:menuItem];
		[menuItem release];
	}
}


id AP_CocoaDialog_Options::_lookupWidget( tControl cid )
{
    switch ( cid )
    {
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // spell
        case id_CHECK_SPELL_CHECK_AS_TYPE:
            return ctrl->m_checkbuttonSpellCheckAsType;

        case id_CHECK_SPELL_HIDE_ERRORS:
            return ctrl->m_checkbuttonSpellHideErrors;

        case id_CHECK_SPELL_SUGGEST:
            return ctrl->m_checkbuttonSpellSuggest;

        case id_CHECK_SPELL_MAIN_ONLY:
            return ctrl->m_checkbuttonSpellMainOnly;

        case id_CHECK_SPELL_UPPERCASE:
            return ctrl->m_checkbuttonSpellUppercase;

        case id_CHECK_SPELL_NUMBERS:
            return ctrl->m_checkbuttonSpellNumbers;

        case id_CHECK_GRAMMAR_CHECK:
            return ctrl->m_checkbuttonGrammarCheck;

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // Smart quotes

        case id_CHECK_SMART_QUOTES_ENABLE:
            return ctrl->m_checkbuttonSmartQuotes;

        case id_CHECK_CUSTOM_SMART_QUOTES:
            return ctrl->m_checkbuttonCustomSmartQuotes;
            
        case id_LIST_VIEW_OUTER_QUOTE_STYLE:
            return ctrl->m_comboOuterQuote;
            
        case id_LIST_VIEW_INNER_QUOTE_STYLE:
            return ctrl->m_comboInnerQuote;

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // other
        case id_CHECK_OTHER_DEFAULT_DIRECTION_RTL:
            return ctrl->m_checkbuttonOtherDirectionRtl;

        case id_CHECK_AUTO_SAVE_FILE:
            return ctrl->m_checkbuttonAutoSaveFile;

        case id_TEXT_AUTO_SAVE_FILE_EXT:
            return ctrl->m_textAutoSaveFileExt;

        case id_TEXT_AUTO_SAVE_FILE_PERIOD:
            return ctrl->m_textAutoSaveFilePeriod;

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // view
        case id_LIST_VIEW_RULER_UNITS:
            return ctrl->m_menuUnits;

        case id_CHECK_AUTO_LOAD_PLUGINS:
            return ctrl->m_checkbuttonAutoLoadPlugins;
		case id_CHECK_ENABLE_OVERWRITE:
			return ctrl->m_checkbuttonEnableOverwrite;

//        case id_PUSH_CHOOSE_COLOR_FOR_TRANSPARENT:
//            return ctrl->m_pushbuttonNewTransparentColor;

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            // general

        case id_BUTTON_DEFAULTS:
            return ctrl->m_buttonDefaults;

            // not implemented
        case id_CHECK_VIEW_SHOW_STATUS_BAR:
        case id_CHECK_VIEW_SHOW_RULER:
        case id_CHECK_VIEW_UNPRINTABLE:
        case id_CHECK_ENABLE_SMOOTH_SCROLLING:
        case id_CHECK_VIEW_ALL:
        case id_CHECK_VIEW_HIDDEN_TEXT:
        case id_COMBO_PREFS_SCHEME:
        case id_CHECK_PREFS_AUTO_SAVE:

        case id_BUTTON_SAVE:
        case id_BUTTON_APPLY:
        case id_BUTTON_CANCEL:
        case id_BUTTON_OK:
            return nil;

        default:
            UT_ASSERT ( "Unknown Widget" );
            return nil;
    }
}


void AP_CocoaDialog_Options::_controlEnable( tControl cid, bool value )
{
	NSControl *control = _lookupWidget(cid);
	if(control) {
		[control setEnabled:(value?YES:NO)];
	}
}




#define DEFINE_GET_SET_BOOL(button) \
    bool     AP_CocoaDialog_Options::_gather##button(void) {    \
        UT_ASSERT(ctrl->m_checkbutton##button); \
        return ([ctrl->m_checkbutton##button state] == NSOnState); }   \
    void        AP_CocoaDialog_Options::_set##button(bool b) { \
        UT_ASSERT(ctrl->m_checkbutton##button); \
        [ctrl->m_checkbutton##button setState:(b?NSOnState:NSOffState)]; }

#define DEFINE_GET_SET_BOOL_D(button) \
    bool     AP_CocoaDialog_Options::_gather##button(void) {    \
                return false; }   \
    void        AP_CocoaDialog_Options::_set##button(bool) { \
               }

#define DEFINE_GET_SET_TEXT(widget) \
    char *  AP_CocoaDialog_Options::_gather##widget() {    \
        UT_ASSERT(ctrl->m_text##widget)); \
        return [[ctrl->m_text##widget stringValue] UTF8String]; }   \
    \
    void  AP_CocoaDialog_Options::_set##widget(const char *t) { \
        UT_ASSERT(ctrl->m_text##widget); \
		[ctrl->m_text##widget setStringValue:[NSString stringWithCString:t encoding:NSUTF8StringEncoding]]; \
    }

#ifdef ENABLE_SPELL
DEFINE_GET_SET_BOOL ( SpellCheckAsType )
DEFINE_GET_SET_BOOL ( SpellHideErrors )
DEFINE_GET_SET_BOOL ( SpellSuggest )
DEFINE_GET_SET_BOOL ( SpellMainOnly )
DEFINE_GET_SET_BOOL ( SpellUppercase )
DEFINE_GET_SET_BOOL ( SpellNumbers )
#else
DEFINE_GET_SET_BOOL_D ( SpellCheckAsType )
DEFINE_GET_SET_BOOL_D ( SpellHideErrors )
DEFINE_GET_SET_BOOL_D ( SpellSuggest )
DEFINE_GET_SET_BOOL_D ( SpellMainOnly )
DEFINE_GET_SET_BOOL_D ( SpellUppercase )
DEFINE_GET_SET_BOOL_D ( SpellNumbers )
#endif
#ifndef _DISABLE_GRAMMAR
DEFINE_GET_SET_BOOL ( GrammarCheck )
#else
// TODO FIX this hack I do this to avoid the assert.
bool     AP_CocoaDialog_Options::_gatherGrammarCheck(void) 
{
    return false;
}
void        AP_CocoaDialog_Options::_setGrammarCheck(bool) 
{
}
#endif
DEFINE_GET_SET_BOOL ( SmartQuotes )
DEFINE_GET_SET_BOOL ( CustomSmartQuotes )

DEFINE_GET_SET_BOOL ( OtherDirectionRtl )

DEFINE_GET_SET_BOOL ( AutoSaveFile )

DEFINE_GET_SET_BOOL ( EnableOverwrite )

// dummy implementations. XP pref backend isn't very smart.
#define DEFINE_GET_SET_BOOL_DUMMY(Bool)     \
    bool AP_CocoaDialog_Options::_gather##Bool(void) {   \
        return m_bool##Bool;     \
    }        \
    void AP_CocoaDialog_Options::_set##Bool(bool b) {   \
        m_bool##Bool = b;     \
    }

DEFINE_GET_SET_BOOL_DUMMY ( EnableSmoothScrolling )
DEFINE_GET_SET_BOOL_DUMMY ( PrefsAutoSave )
DEFINE_GET_SET_BOOL_DUMMY ( ViewAll )
DEFINE_GET_SET_BOOL_DUMMY ( ViewHiddenText )
DEFINE_GET_SET_BOOL_DUMMY ( ViewShowRuler )
DEFINE_GET_SET_BOOL_DUMMY ( ViewShowStatusBar )
DEFINE_GET_SET_BOOL_DUMMY ( ViewUnprintable )

void AP_CocoaDialog_Options::_gatherAutoSaveFileExt ( UT_String &stRetVal )
{
	UT_ASSERT(ctrl->m_textAutoSaveFileExt);
	stRetVal = [[ctrl->m_textAutoSaveFileExt stringValue] UTF8String];
}

void AP_CocoaDialog_Options::_setAutoSaveFileExt ( const UT_String &stExt )
{
	UT_ASSERT(ctrl->m_textAutoSaveFileExt);
	[ctrl->m_textAutoSaveFileExt setStringValue:[NSString stringWithCString:stExt.c_str() encoding:NSUTF8StringEncoding]];
}

void AP_CocoaDialog_Options::_gatherAutoSaveFilePeriod ( UT_String &stRetVal )
{
	UT_ASSERT(ctrl->m_textAutoSaveFilePeriod);
	stRetVal = [[ctrl->m_textAutoSaveFilePeriod stringValue] UTF8String];
}

void AP_CocoaDialog_Options::_setAutoSaveFilePeriod ( const UT_String &stPeriod )
{
	UT_ASSERT(ctrl->m_textAutoSaveFilePeriod);
	[ctrl->m_textAutoSaveFilePeriod setStringValue:[NSString stringWithCString:stPeriod.c_str() encoding:NSUTF8StringEncoding]];
}

UT_Dimension AP_CocoaDialog_Options::_gatherViewRulerUnits ( void )
{
    UT_ASSERT (ctrl->m_menuUnits);
	return (UT_Dimension)[[ctrl->m_menuUnits selectedItem] tag];
}

gint AP_CocoaDialog_Options::_gatherOuterQuoteStyle ( void )
{
    UT_ASSERT (ctrl->m_comboOuterQuote);
	return [[ctrl->m_comboOuterQuote selectedItem] tag];
}


gint AP_CocoaDialog_Options::_gatherInnerQuoteStyle ( void )
{
    UT_ASSERT (ctrl->m_comboInnerQuote);
	return [[ctrl->m_comboInnerQuote selectedItem] tag];
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void AP_CocoaDialog_Options::_setViewRulerUnits ( UT_Dimension dim )
{
    UT_ASSERT (ctrl->m_menuUnits);

	[ctrl->m_menuUnits selectItemWithTag:(int)dim];
}


void AP_CocoaDialog_Options::_setOuterQuoteStyle ( gint nIndex )
{
    UT_ASSERT (ctrl->m_comboOuterQuote);
	
	[ctrl->m_comboOuterQuote selectItemWithTag:nIndex];
}

void AP_CocoaDialog_Options::_setInnerQuoteStyle ( gint nIndex )
{
    UT_ASSERT (ctrl->m_comboInnerQuote);
	
	[ctrl->m_comboInnerQuote selectItemWithTag:nIndex];
}


DEFINE_GET_SET_BOOL ( AutoLoadPlugins )

#undef DEFINE_GET_SET_BOOL


int AP_CocoaDialog_Options::_gatherNotebookPageNum (void)
{
    UT_ASSERT (ctrl->oTabView);
	return [ctrl->oTabView indexOfTabViewItem:[ctrl->oTabView selectedTabViewItem]];
}

void AP_CocoaDialog_Options::_setNotebookPageNum (int pn)
{
    UT_ASSERT (ctrl->oTabView);
	[ctrl->oTabView selectTabViewItemAtIndex:pn];
}


void AP_CocoaDialog_Options::_populateWindowData(void)
{
	AP_Dialog_Options::_populateWindowData();
	
	setReinit(true);
	[ctrl autoSaveClicked:ctrl->m_checkbuttonAutoSaveFile];
	[ctrl checkboxClicked:ctrl->m_checkbuttonSmartQuotes];
	[ctrl checkboxClicked:ctrl->m_checkbuttonCustomSmartQuotes];
	[ctrl checkboxClicked:ctrl->m_checkbuttonSpellCheckAsType];
	setReinit(false);
}


/* ******************************** AP_CocoaDialog_OptionsController ******************************** */

@implementation AP_CocoaDialog_OptionsController

- (id)initFromNib
{
	if (self = [super initWithWindowNibName:@"ap_CocoaDialog_Options"]) {

	}
	return self;
}

- (oneway void)dealloc
{
	[super dealloc];
}

- (void)windowDidLoad
{
	UT_DEBUGMSG(("Option Dialog did load\n"));
	/* Localize labels, etc.
	 */
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	LocalizeControl([self window], pSS, AP_STRING_ID_DLG_Options_OptionsTitle);

	LocalizeControl([oTabView tabViewItemAtIndex:0], pSS, AP_STRING_ID_DLG_Options_Label_General);
	LocalizeControl([oTabView tabViewItemAtIndex:1], pSS, AP_STRING_ID_DLG_Options_Label_Documents);
	LocalizeControl([oTabView tabViewItemAtIndex:2], pSS, AP_STRING_ID_DLG_Options_SpellCheckingTitle);
	LocalizeControl([oTabView tabViewItemAtIndex:3], pSS, AP_STRING_ID_DLG_Options_TabLabel_SmartQuotes);

	LocalizeControl(oBox_UserInterface,				pSS, AP_STRING_ID_DLG_Options_Label_UI);
	LocalizeControl(oBox_BiDiOptions,				pSS, AP_STRING_ID_DLG_Options_Label_BiDiOptions);
	LocalizeControl(oBox_General,					pSS, AP_STRING_ID_DLG_Options_Label_General);
	LocalizeControl(oBox_Ignore,					pSS, AP_STRING_ID_DLG_Options_Label_Ignore);
	LocalizeControl(oBox_Grammar,                   pSS, AP_STRING_ID_DLG_Options_Label_Grammar);

	LocalizeControl(m_checkbuttonAutoLoadPlugins,	pSS, AP_STRING_ID_DLG_Options_Label_CheckAutoLoadPlugins);
	LocalizeControl(oLabel_Minutes,					pSS, AP_STRING_ID_DLG_Options_Label_Minutes);
	LocalizeControl(oLabel_WithExtension,			pSS, AP_STRING_ID_DLG_Options_Label_WithExtension);

// TODO: Screen color
	LocalizeControl(oButton_ChooseScreenColor,		pSS, AP_STRING_ID_DLG_Options_Label_ChooseForTransparent);

	LocalizeControl(oLabel_Units,					pSS, AP_STRING_ID_DLG_Options_Label_ViewUnits);
	LocalizeControl(m_checkbuttonEnableOverwrite,   pSS, AP_STRING_ID_DLG_Options_Label_EnableOverwrite);
	LocalizeControl(m_checkbuttonOtherDirectionRtl,	pSS, AP_STRING_ID_DLG_Options_Label_DirectionRtl);
	LocalizeControl(m_checkbuttonSpellCheckAsType,		pSS, AP_STRING_ID_DLG_Options_Label_SpellCheckAsType);
	LocalizeControl(m_checkbuttonGrammarCheck,			pSS, AP_STRING_ID_DLG_Options_Label_GrammarCheck);
	LocalizeControl(m_checkbuttonSpellHideErrors,		pSS, AP_STRING_ID_DLG_Options_Label_SpellHighlightMisspelledWords);
	LocalizeControl(m_checkbuttonSpellUppercase,		pSS, AP_STRING_ID_DLG_Options_Label_SpellUppercase);
	LocalizeControl(m_checkbuttonSpellNumbers,			pSS, AP_STRING_ID_DLG_Options_Label_SpellNumbers);

	LocalizeControl(m_buttonDefaults,				pSS, AP_STRING_ID_DLG_Options_Btn_Default);
	LocalizeControl(oButton_Close,					pSS,XAP_STRING_ID_DLG_Close);

	[oStepper_Minutes setIntValue:1];

	[m_menuUnits removeAllItems];
	AP_Dialog_Options::UnitMenuContent content;
	NSMenu *menu = [m_menuUnits menu];
	m_xap->_getUnitMenuContent(pSS, content);
	
	for(AP_Dialog_Options::UnitMenuContent::const_iterator iter = content.begin();
		iter != content.end(); ++iter) {
		
		NSMenuItem * menuItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithCString:iter->first.c_str() encoding:NSUTF8StringEncoding] action:nil
			keyEquivalent:@""];
		[menuItem setTag:iter->second];
		[menu addItem:menuItem];
		[menuItem release];
	}
	
	 // create user data tControl -> stored in widgets
	for (int i = 0; i < (int)AP_Dialog_Options::id_last; i++ )
    {
        NSControl *w = m_xap->_lookupWidget ((AP_Dialog_Options::tControl)i);
        if (!w)
            continue;

        /* check to see if there is any data already stored there (note, will
         * not work if 0's is stored in multiple places  */
		[w setTag:(int)i];
		if([w action] == nil) {
			[w setTarget:self];
			[w setAction:@selector(controlChanged:)];
		}
	}
	m_xap->_setupSmartQuotesCombos(m_comboOuterQuote);
	m_xap->_setupSmartQuotesCombos(m_comboInnerQuote);

	[self autoSaveClicked:m_checkbuttonAutoSaveFile];

    std::string s;
    pSS->getValueUTF8(AP_STRING_ID_DLG_Options_OptionsTitle, s);
	[[self window] setTitle:[NSString stringWithCString:s.c_str() encoding:NSUTF8StringEncoding]];
}

- (IBAction)revertClicked:(id)sender
{
	UT_UNUSED(sender);
    m_xap->_event_SetDefaults();

	[self controlChanged:sender];
}


- (IBAction)checkboxClicked:(id)sender
{
    int cid = [sender tag];
    m_xap->_enableDisableLogic ((AP_Dialog_Options::tControl)cid);

	if(!m_xap->getReinit()) {
		[self controlChanged:sender];
	}
}

- (IBAction)autoSaveClicked:(id)sender
{
	BOOL enable = (([sender state] == NSOnState) ? YES : NO);
	[oLabel_WithExtension setEnabled:enable];
	[oLabel_Minutes setEnabled:enable];
	[m_textAutoSaveFilePeriod setEnabled:enable];
	[m_textAutoSaveFileExt setEnabled:enable];
	[oStepper_Minutes setEnabled:enable];
	
	if(!m_xap->getReinit()) {
		[self controlChanged:sender];
	}
}


- (IBAction)controlChanged:(id)sender
{
    UT_DEBUGMSG ( ( "Control changed\n" ) );
    UT_ASSERT ( sender && m_xap );

    int cid = [sender tag];
    m_xap->_storeDataForControl ((AP_Dialog_Options::tControl)cid);
}

- (IBAction)closeClicked:(id)sender
{
	UT_UNUSED(sender);
	[NSApp stopModal];
}

- (void)setXAPOwner:(AP_CocoaDialog_Options *)owner
{
	m_xap = owner;
}


@end



