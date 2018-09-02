/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
 * Copyright (C) 2004 Francis James Franklin
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_units.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xad_Document.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Frame.h"
#include "xap_Prefs.h"
#include "xap_Toolbar_Layouts.h"

#include "fv_View.h"

#include "fl_DocLayout.h"

#include "ap_Dialog_Options.h"
#include "ap_Prefs_SchemeIds.h"
#include "ap_Strings.h"
#include "ap_FrameData.h"

AP_Dialog_Options::AP_Dialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_TabbedDialog_NonPersistent(pDlgFactory,id, "interface/dialogpreferences"),
	  m_answer(a_OK),
	  m_pFrame(0),	// needs to be set from runModal for some of the event_'s to work
	  m_bInitialPop(false)
{
}

AP_Dialog_Options::~AP_Dialog_Options(void)
{
}

AP_Dialog_Options::tAnswer AP_Dialog_Options::getAnswer(void) const
{
	return m_answer;
}

inline void Save_Pref_Bool(  XAP_PrefsScheme *pPrefsScheme,
							 gchar const * key,
							 bool var )
{
	gchar szBuffer[2] = {0,0};
	szBuffer[0] = ((var)==true ? '1' : '0');
	pPrefsScheme->setValue( key, szBuffer );
}

void AP_Dialog_Options::_storeWindowData(void)
{
	XAP_Prefs *pPrefs = m_pApp->getPrefs();
	UT_return_if_fail (pPrefs);

	AP_FrameData *pFrameData = NULL;
	if(m_pFrame) {
		pFrameData = (AP_FrameData *)m_pFrame->getFrameData();
		UT_return_if_fail (pFrameData);
	}

	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	UT_return_if_fail (pPrefsScheme);

	// turn off all notification to PrefListeners via XAP_Prefs
	pPrefs->startBlockChange();

	// before we continue to remember all the changed values, check to see if
	// we have turned OFF PrefsAutoSave.  If so, toggle that value, then force
	// a prefs save, then update everything else
	//			shack@uiuc.edu
	if ( pPrefs->getAutoSavePrefs() == true && _gatherPrefsAutoSave() == false ) {

		pPrefs->setAutoSavePrefs( false );
		pPrefs->savePrefsFile();				// TODO: check the results
	}
	else {	// otherwise, just set the value
		pPrefs->setAutoSavePrefs( _gatherPrefsAutoSave() );
	}

	// try again to make sure we've got an updatable scheme
	pPrefsScheme = pPrefs->getCurrentScheme(true);
	UT_return_if_fail (pPrefsScheme);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// save the values to the Prefs classes
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_AutoSpellCheck, _gatherSpellCheckAsType() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_AutoGrammarCheck, _gatherGrammarCheck() );
	Save_Pref_Bool( pPrefsScheme, XAP_PREF_KEY_SmartQuotesEnable, _gatherSmartQuotes() );
	Save_Pref_Bool( pPrefsScheme, XAP_PREF_KEY_CustomSmartQuotes, _gatherCustomSmartQuotes() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_SpellCheckCaps, _gatherSpellUppercase() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_SpellCheckNumbers, _gatherSpellNumbers() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_CursorBlink, _gatherViewCursorBlink() );
	
// Not implemented for UNIX or Win32. No need for it.
#if !defined(TOOLKIT_GTK_ALL) && !defined(TOOLKIT_COCOA) && !defined (TOOLKIT_WIN) 
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_RulerVisible, _gatherViewShowRuler() );
	UT_uint32 i;
	for (i = 0; i < m_pApp->getToolbarFactory()->countToolbars(); i++) {
		Save_Pref_Bool( pPrefsScheme, m_pApp->getToolbarFactory()->prefKeyForToolbar(i), _gatherViewShowToolbar(i));
	}

	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_StatusBarVisible, _gatherViewShowStatusBar() );
#endif

	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_ParaVisible, _gatherViewUnprintable() );
#if defined(TOOLKIT_GTK_ALL)
	Save_Pref_Bool( pPrefsScheme, XAP_PREF_KEY_EnableSmoothScrolling, _gatherEnableSmoothScrolling() );
#endif
    Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_InsertModeToggle, _gatherEnableOverwrite() );
	Save_Pref_Bool( pPrefsScheme, XAP_PREF_KEY_AutoLoadPlugins, _gatherAutoLoadPlugins() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_DefaultDirectionRtl, _gatherOtherDirectionRtl() );
	Save_Pref_Bool( pPrefsScheme, XAP_PREF_KEY_ChangeLanguageWithKeyboard, _gatherLanguageWithKeyboard() );
	Save_Pref_Bool( pPrefsScheme, XAP_PREF_KEY_DirMarkerAfterClosingParenthesis, _gatherDirMarkerAfterClosingParenthesis());
	
	// JOAQUIN - fix this: Dom
	UT_DEBUGMSG(("Saving Auto Save File [%i]\n", _gatherAutoSaveFile()));
	Save_Pref_Bool( pPrefsScheme, XAP_PREF_KEY_AutoSaveFile, _gatherAutoSaveFile() );

	UT_String stVal;

	_gatherAutoSaveFileExt(stVal);
	UT_DEBUGMSG(("Saving Auto Save File Ext [%s]\n", stVal.c_str()));
	pPrefsScheme->setValue(XAP_PREF_KEY_AutoSaveFileExt, stVal.c_str());
	_gatherAutoSaveFilePeriod(stVal);
	UT_DEBUGMSG(("Saving Auto Save File with a period of [%s]\n", stVal.c_str()));
	pPrefsScheme->setValue(XAP_PREF_KEY_AutoSaveFilePeriod, stVal.c_str());
	
	// Jordi: win32 specific for now
	
	_gatherUILanguage(stVal);
	if (stVal.length())
	{
		UT_DEBUGMSG(("Setting default UI language to [%s]\n", stVal.c_str()));
		pPrefsScheme->setValue(AP_PREF_KEY_StringSet, stVal.c_str());
	}
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// If we changed whether the ruler is to be visible
	// or hidden, then update the current window:
	// (If we didn't change anything, leave it alone)
#if !defined(TOOLKIT_GTK_ALL) && !defined(TOOLKIT_COCOA) && !defined (TOOLKIT_WIN) 
	if (pFrameData && _gatherViewShowRuler() != pFrameData->m_bShowRuler )
	{
		pFrameData->m_bShowRuler = _gatherViewShowRuler() ;
		if (!pFrameData->m_bIsFullScreen)
		{
			m_pFrame->toggleRuler(pFrameData->m_bShowRuler);
		}
	}

	// Same for status bar
	if (pFrameData && _gatherViewShowStatusBar() != pFrameData->m_bShowStatusBar)
	{
		pFrameData->m_bShowStatusBar = _gatherViewShowStatusBar();
		if (!pFrameData->m_bIsFullScreen)
		{
			m_pFrame->toggleStatusBar(pFrameData->m_bShowStatusBar);
		}
	}


	if(pFrameData) {
		for (i = 0; i < m_pApp->getToolbarFactory()->countToolbars(); i++) {
			if (_gatherViewShowToolbar(i) != pFrameData->m_bShowBar[i])
			{
				pFrameData->m_bShowBar[i] = _gatherViewShowToolbar(i);
				if (!pFrameData->m_bIsFullScreen)
				{
					m_pFrame->toggleBar(i, pFrameData->m_bShowBar[i]);
				}
			}
		}
	}
#endif
	
	if (pFrameData &&  _gatherViewUnprintable() != pFrameData->m_bShowPara )
	{
		pFrameData->m_bShowPara = _gatherViewUnprintable() ;
		AV_View * pAVView = m_pFrame->getCurrentView();
		UT_return_if_fail (pAVView);

		FV_View * pView = static_cast<FV_View *> (pAVView);

		pView->setShowPara(pFrameData->m_bShowPara);
	}

#if defined(TOOLKIT_GTK_ALL)
	if ( _gatherEnableSmoothScrolling() != XAP_App::getApp()->isSmoothScrollingEnabled() )
	{
		XAP_App::getApp()->setEnableSmoothScrolling(_gatherEnableSmoothScrolling());
	}
#endif
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// save ruler units value
	pPrefsScheme->setValue((gchar*)AP_PREF_KEY_RulerUnits,
				   (gchar*)UT_dimensionName( _gatherViewRulerUnits()) );

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// save screen color
	pPrefsScheme->setValue((gchar*)XAP_PREF_KEY_ColorForTransparent,
				   _gatherColorForTransparent() );


	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// allow XAP_Prefs to notify all the listeners of changes

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// TODO: change to snprintf
	gchar szBuffer[40];
	sprintf( szBuffer, "%i", _gatherNotebookPageNum() );
	pPrefsScheme->setValue((gchar*)AP_PREF_KEY_OptionsTabNumber,
				   (gchar*)szBuffer );

	// allow the prefListeners to receive their calls
	pPrefs->endBlockChange();

	// if we hit the Save button, then force a save after the gather
	if ( m_answer == a_SAVE ) {
		pPrefs->savePrefsFile();				// TODO: check the results
	}

}

/* Needed for instant apply and friends. It gathers the value of the widget associated with
 * the tControl gived and sets it as pref */
void AP_Dialog_Options::_storeDataForControl (tControl id)
{
	UT_String stVal;

	XAP_Prefs *pPrefs = m_pApp->getPrefs();
	UT_return_if_fail (pPrefs);

	AP_FrameData *pFrameData = NULL;
	if(m_pFrame) {
		pFrameData = (AP_FrameData *)m_pFrame->getFrameData();
		UT_return_if_fail (pFrameData);
	}

	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	UT_return_if_fail (pPrefsScheme);

	// turn off all notification to PrefListeners via XAP_Prefs
	pPrefs->startBlockChange();

	switch (id)
	{

		case id_CHECK_SPELL_CHECK_AS_TYPE:
			Save_Pref_Bool (pPrefsScheme, AP_PREF_KEY_AutoSpellCheck,
					_gatherSpellCheckAsType());
			break;

		case id_CHECK_GRAMMAR_CHECK:
			Save_Pref_Bool (pPrefsScheme, AP_PREF_KEY_AutoGrammarCheck,
					_gatherGrammarCheck());
			break;

		case id_CHECK_SMART_QUOTES_ENABLE:
			Save_Pref_Bool (pPrefsScheme, XAP_PREF_KEY_SmartQuotesEnable,
					_gatherSmartQuotes());
			break;

		case id_CHECK_CUSTOM_SMART_QUOTES:
			Save_Pref_Bool (pPrefsScheme, XAP_PREF_KEY_CustomSmartQuotes,
					_gatherCustomSmartQuotes());
			break;
            
        case id_LIST_VIEW_OUTER_QUOTE_STYLE:
			pPrefsScheme->setValueInt ((gchar*)XAP_PREF_KEY_OuterQuoteStyle,
						_gatherOuterQuoteStyle());
			break;
            
        case id_LIST_VIEW_INNER_QUOTE_STYLE:
			pPrefsScheme->setValueInt ((gchar*)XAP_PREF_KEY_InnerQuoteStyle,
						_gatherInnerQuoteStyle());
			break;

		case id_CHECK_SPELL_UPPERCASE:
			Save_Pref_Bool (pPrefsScheme, AP_PREF_KEY_SpellCheckCaps,
					_gatherSpellUppercase());
			break;

		case id_CHECK_SPELL_NUMBERS:
			Save_Pref_Bool (pPrefsScheme, AP_PREF_KEY_SpellCheckNumbers,
					_gatherSpellNumbers());
			break;

		case id_CHECK_OTHER_DEFAULT_DIRECTION_RTL:
			Save_Pref_Bool (pPrefsScheme, AP_PREF_KEY_DefaultDirectionRtl,
					_gatherOtherDirectionRtl());
			break;

		case id_CHECK_AUTO_SAVE_FILE:
			UT_DEBUGMSG(("Saving Auto Save File [%i]\n", _gatherAutoSaveFile()));
			Save_Pref_Bool( pPrefsScheme, XAP_PREF_KEY_AutoSaveFile, _gatherAutoSaveFile() );
			break;

		case id_TEXT_AUTO_SAVE_FILE_EXT:
			_gatherAutoSaveFileExt(stVal);
			UT_DEBUGMSG(("Saving Auto Save File Ext [%s]\n", stVal.c_str()));
			pPrefsScheme->setValue(XAP_PREF_KEY_AutoSaveFileExt, stVal.c_str());
			break;

		case id_TEXT_AUTO_SAVE_FILE_PERIOD:
			_gatherAutoSaveFilePeriod(stVal);
			UT_DEBUGMSG(("Saving Auto Save File with a period of [%s]\n", stVal.c_str()));
			pPrefsScheme->setValue(XAP_PREF_KEY_AutoSaveFilePeriod, stVal.c_str());
			break;

		case id_CHECK_VIEW_SHOW_RULER:
#if !defined(TOOLKIT_GTK_ALL) && !defined(TOOLKIT_COCOA) && !defined (TOOLKIT_WIN) 
			{
				bool tmpbool = _gatherViewShowRuler();
				Save_Pref_Bool (pPrefsScheme, AP_PREF_KEY_RulerVisible, tmpbool);
				if (pFrameData && (tmpbool != pFrameData->m_bShowRuler))
				{
					pFrameData->m_bShowRuler = _gatherViewShowRuler() ;
					m_pFrame->toggleRuler(pFrameData->m_bShowRuler);
				}
			}
			
#endif
			break;

		case id_LIST_VIEW_RULER_UNITS:
			pPrefsScheme->setValue ((gchar*)AP_PREF_KEY_RulerUnits,
						(gchar*)UT_dimensionName (_gatherViewRulerUnits()));
			break;

		case id_CHECK_VIEW_CURSOR_BLINK:
			Save_Pref_Bool (pPrefsScheme, AP_PREF_KEY_CursorBlink,
					_gatherViewCursorBlink());
			break;

		case id_CHECK_VIEW_SHOW_STATUS_BAR:
#if !defined(TOOLKIT_GTK_ALL) && !defined(TOOLKIT_COCOA) && !defined (TOOLKIT_WIN) 
			{
				bool tmpbool = _gatherViewShowStatusBar();
				Save_Pref_Bool (pPrefsScheme, AP_PREF_KEY_StatusBarVisible, tmpbool);
				if (pFrameData && (tmpbool != pFrameData->m_bShowStatusBar))
				{
					pFrameData->m_bShowStatusBar = tmpbool;
					m_pFrame->toggleStatusBar(pFrameData->m_bShowStatusBar);
				}
			}
			
#endif
			break;

		case id_PUSH_CHOOSE_COLOR_FOR_TRANSPARENT:
			pPrefsScheme->setValue ((gchar*)XAP_PREF_KEY_ColorForTransparent,
						_gatherColorForTransparent());
			break;

		case id_CHECK_VIEW_UNPRINTABLE:
			Save_Pref_Bool (pPrefsScheme, AP_PREF_KEY_ParaVisible,
					_gatherViewUnprintable());
			break;

		case id_CHECK_ENABLE_SMOOTH_SCROLLING:
#if defined(TOOLKIT_GTK_ALL)
			Save_Pref_Bool (pPrefsScheme, XAP_PREF_KEY_EnableSmoothScrolling,
					_gatherEnableSmoothScrolling());
#endif
			break;
        case id_CHECK_ENABLE_OVERWRITE:
			Save_Pref_Bool (pPrefsScheme, AP_PREF_KEY_InsertModeToggle,
					_gatherEnableOverwrite() );
            break;
		case id_CHECK_AUTO_LOAD_PLUGINS:
			Save_Pref_Bool (pPrefsScheme, XAP_PREF_KEY_AutoLoadPlugins,
					_gatherAutoLoadPlugins() );
			break;

		case id_CHECK_LANG_WITH_KEYBOARD:
			Save_Pref_Bool (pPrefsScheme, XAP_PREF_KEY_ChangeLanguageWithKeyboard,
					_gatherLanguageWithKeyboard() );
			break;

		case id_CHECK_DIR_MARKER_AFTER_CLOSING_PARENTHESIS:
			Save_Pref_Bool (pPrefsScheme, XAP_PREF_KEY_DirMarkerAfterClosingParenthesis,
					_gatherDirMarkerAfterClosingParenthesis());
			break;

		case id_NOTEBOOK:
			gchar szBuffer[40];
			sprintf( szBuffer, "%i", _gatherNotebookPageNum() );
			pPrefsScheme->setValue ((gchar*)AP_PREF_KEY_OptionsTabNumber,
						(gchar*)szBuffer );
			break;

		// Ignore window controls/special buttons
		case id_BUTTON_SAVE:
		case id_BUTTON_DEFAULTS:
		case id_BUTTON_OK:
		case id_BUTTON_CANCEL:
		case id_BUTTON_APPLY:
		case id_BUTTON_SPELL_AUTOREPLACE:
		case id_CHECK_COLOR_FOR_TRANSPARENT_IS_WHITE:
		case id_TEXT_AUTO_SAVE_FILE_PERIOD_SPIN:  // needed by Cocoa FE

		// Not implemented
		case id_CHECK_PREFS_AUTO_SAVE:
		case id_CHECK_SPELL_HIDE_ERRORS:
		case id_CHECK_SPELL_MAIN_ONLY:
		case id_CHECK_SPELL_SUGGEST:
		case id_CHECK_VIEW_ALL:
		case id_CHECK_VIEW_HIDDEN_TEXT:
		case id_COMBO_PREFS_SCHEME:

		// Dummy case, dummy comment :)
		case id_last:
		UT_DEBUGMSG (("WARNING: _storeDataForControl not implemented for this control\n"));
		default:
			break;
	}

	// allow the prefListeners to receive their calls and
	pPrefs->endBlockChange();

	pPrefs->savePrefsFile();
}

void AP_Dialog_Options::_setColorForTransparent(const gchar *
												pzsColorForTransparent)
{
	strncpy(m_CurrentTransparentColor,pzsColorForTransparent,9);
}

const gchar * AP_Dialog_Options::_gatherColorForTransparent(void)
{
	return (const gchar *) m_CurrentTransparentColor;
}

void AP_Dialog_Options::_eventSave(void)
{
	m_answer = a_SAVE;

	_storeWindowData();

	m_answer = a_OK;
}

void AP_Dialog_Options::_populateWindowData(void)
{
	bool			b;
	gint			n = 0;
	XAP_Prefs		*pPrefs = 0;
	const gchar	*pszBuffer = 0;
	m_bInitialPop = true;
	// TODO: move this logic when we get a PrefsListener API and turn this
	//		 dialog into an app-specific

	pPrefs = m_pApp->getPrefs();
	UT_return_if_fail ( pPrefs );

	// ------------ Spell
	if (pPrefs->getPrefsValueBool((gchar*)AP_PREF_KEY_AutoSpellCheck,&b))
		_setSpellCheckAsType (b);

	if (pPrefs->getPrefsValueBool((gchar*)AP_PREF_KEY_SpellCheckCaps,&b))
		_setSpellUppercase (b);

	if (pPrefs->getPrefsValueBool((gchar*)AP_PREF_KEY_SpellCheckNumbers,&b))
		_setSpellNumbers (b);

	if (pPrefs->getPrefsValueBool((gchar*)AP_PREF_KEY_AutoGrammarCheck,&b))
		_setGrammarCheck (b);

	// ------------ Smart Quotes
	if (pPrefs->getPrefsValueBool((gchar*)XAP_PREF_KEY_SmartQuotesEnable,&b))
		_setSmartQuotes (b);

	if (pPrefs->getPrefsValueBool((gchar*)XAP_PREF_KEY_CustomSmartQuotes,&b))
		_setCustomSmartQuotes (b);
		
	if (pPrefs->getPrefsValueInt((gchar*)XAP_PREF_KEY_OuterQuoteStyle, n))
		_setOuterQuoteStyle(n);
		
	if (pPrefs->getPrefsValueInt((gchar*)XAP_PREF_KEY_InnerQuoteStyle, n))
		_setInnerQuoteStyle(n);

	// ------------ Prefs
	_setPrefsAutoSave( pPrefs->getAutoSavePrefs() );

	// ------------ View
	if (pPrefs->getPrefsValue((gchar*)AP_PREF_KEY_RulerUnits,&pszBuffer))
		_setViewRulerUnits (UT_determineDimension(pszBuffer));


#if !defined(TOOLKIT_GTK_ALL) && !defined(TOOLKIT_COCOA) && !defined (TOOLKIT_WIN) 
	if (pPrefs->getPrefsValueBool((gchar*)AP_PREF_KEY_RulerVisible,&b))
		_setViewShowRuler (b);
	UT_uint32 i;
	for (i = 0; i < m_pApp->getToolbarFactory()->countToolbars(); i++) {
		if (pPrefs->getPrefsValueBool((gchar*)m_pApp->getToolbarFactory()->prefKeyForToolbar(i),&b)) {
			_setViewShowToolbar (i, b);
		}
	}

	if (pPrefs->getPrefsValueBool((gchar*)AP_PREF_KEY_StatusBarVisible,&b))
		_setViewShowStatusBar (b);
#endif	


	if (pPrefs->getPrefsValueBool((gchar*)AP_PREF_KEY_InsertModeToggle,&b))
		_setEnableOverwrite (b);

	if (pPrefs->getPrefsValueBool((gchar*)AP_PREF_KEY_ParaVisible,&b))
		_setViewUnprintable (b);

	if (pPrefs->getPrefsValueBool((gchar*)AP_PREF_KEY_CursorBlink,&b))
		_setViewCursorBlink (b);

#if defined(TOOLKIT_GTK_ALL)
	if (pPrefs->getPrefsValueBool((gchar*)XAP_PREF_KEY_EnableSmoothScrolling,&b))
		_setEnableSmoothScrolling(b);
#endif
	if (pPrefs->getPrefsValueBool((gchar*)XAP_PREF_KEY_AutoLoadPlugins,&b))
		_setAutoLoadPlugins(b);

	// TODO: JOAQUIN FIX THIS
	if (pPrefs->getPrefsValueBool((gchar*)XAP_PREF_KEY_AutoSaveFile,&b))
		_setAutoSaveFile (b);

	std::string stBuffer;
	if (pPrefs->getPrefsValue(XAP_PREF_KEY_AutoSaveFileExt, stBuffer))
		_setAutoSaveFileExt(stBuffer);

	if (pPrefs->getPrefsValue(XAP_PREF_KEY_AutoSaveFilePeriod, stBuffer))
		_setAutoSaveFilePeriod(stBuffer);

	//Just for win32
	if (pPrefs->getPrefsValue(AP_PREF_KEY_StringSet, stBuffer))
		_setUILanguage(stBuffer);

	// ------------ Screen Color

	const gchar * pszColorForTransparent = NULL;
	if (pPrefs->getPrefsValue(XAP_PREF_KEY_ColorForTransparent, &pszColorForTransparent))
		_setColorForTransparent(pszColorForTransparent);


	// ------------ the page tab number
	int which = getInitialPageNum ();
	if ((which == -1) && pPrefs->getPrefsValue((gchar*)AP_PREF_KEY_OptionsTabNumber,&pszBuffer))
		_setNotebookPageNum (atoi(pszBuffer));
	else
	  _setNotebookPageNum(which);

	//------------- other
	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_DefaultDirectionRtl,&b))
		_setOtherDirectionRtl (b);

	if (pPrefs->getPrefsValueBool(XAP_PREF_KEY_ChangeLanguageWithKeyboard,&b))
		_setLanguageWithKeyboard (b);

	if (pPrefs->getPrefsValueBool(XAP_PREF_KEY_DirMarkerAfterClosingParenthesis,&b))
		_setDirMarkerAfterClosingParenthesis (b);
	
	// enable/disable controls
	_initEnableControls();
	m_bInitialPop = false;
}


void AP_Dialog_Options::_enableDisableLogic( tControl id )
{
	switch (id)
	{

/*	- Since HIDE_ERRORS is not implemented, no need to toggle it on/off
	case id_CHECK_SPELL_CHECK_AS_TYPE:
		// if we 'check as we type', then enable the 'hide' option
		_controlEnable( id_CHECK_SPELL_HIDE_ERRORS,
						_gatherSpellCheckAsType() );
		break;
*/

	case id_CHECK_DIR_MARKER_AFTER_CLOSING_PARENTHESIS:
		_controlEnable( id_CHECK_DIR_MARKER_AFTER_CLOSING_PARENTHESIS, _gatherLanguageWithKeyboard());
		break;
		
	case id_CHECK_SMART_QUOTES_ENABLE:
	case id_CHECK_CUSTOM_SMART_QUOTES:
		_controlEnable( id_CHECK_CUSTOM_SMART_QUOTES, _gatherSmartQuotes());
		_controlEnable( id_LIST_VIEW_OUTER_QUOTE_STYLE, _gatherSmartQuotes() && _gatherCustomSmartQuotes());
		_controlEnable( id_LIST_VIEW_INNER_QUOTE_STYLE, _gatherSmartQuotes() && _gatherCustomSmartQuotes());
		break;
		
	default:
		// do nothing
		break;
	}
}


void AP_Dialog_Options::_getUnitMenuContent(const XAP_StringSet *pSS, UnitMenuContent & content)
{
    std::string s;
    pSS->getValueUTF8 ( XAP_STRING_ID_DLG_Unit_inch, s );
	content.push_back(std::make_pair(s, (int)DIM_IN));
    pSS->getValueUTF8 ( XAP_STRING_ID_DLG_Unit_cm, s );
	content.push_back(std::make_pair(s, (int)DIM_CM));
    pSS->getValueUTF8 ( XAP_STRING_ID_DLG_Unit_points, s );
	content.push_back(std::make_pair(s, (int)DIM_PT));
    pSS->getValueUTF8 ( XAP_STRING_ID_DLG_Unit_pica, s );
	content.push_back(std::make_pair(s, (int)DIM_PI));
}


// The initialize the controls (i.e., disable controls not coded yet)
void AP_Dialog_Options::_initEnableControls()
{
	// spelling
	_controlEnable( id_CHECK_SPELL_SUGGEST, 		false );
	_controlEnable( id_CHECK_SPELL_HIDE_ERRORS, 	false );
	_controlEnable( id_CHECK_SPELL_MAIN_ONLY,		false );

	// prefs
	_controlEnable( id_COMBO_PREFS_SCHEME,			false );

	// view
	_controlEnable( id_CHECK_VIEW_ALL,				false );
	_controlEnable( id_CHECK_VIEW_HIDDEN_TEXT,		false );

	// general
	_controlEnable( id_BUTTON_SAVE, 				false );

	// language
	// only works on win32 for now (must precede
	// id_CHECK_DIR_MARKER... in the bidi controls !!!)
	_controlEnable( id_CHECK_LANG_WITH_KEYBOARD,    false ); 

																// implemented
	_controlEnable( id_CHECK_DIR_MARKER_AFTER_CLOSING_PARENTHESIS,_gatherLanguageWithKeyboard());  
	//
	// If the prefs color for transparent is white initially disable the choose
	// color button
	// On UNIX/GTK, we have a nice color chooser and ignore this setting.
#ifndef TOOLKIT_GTK_ALL
	if(strcmp(m_CurrentTransparentColor,"ffffff") == 0)
	{
		_controlEnable( id_PUSH_CHOOSE_COLOR_FOR_TRANSPARENT, false);
	}
	else
	{
		_controlEnable( id_PUSH_CHOOSE_COLOR_FOR_TRANSPARENT, true);
	}
#endif

	_initEnableControlsPlatformSpecific();
}

void AP_Dialog_Options::_event_SetDefaults(void)
{
	XAP_Prefs		*pPrefs;
	const gchar	*old_name;
	int currentPage;

	pPrefs = m_pApp->getPrefs();
	UT_return_if_fail (pPrefs);

	// SetDefaults
	//	To set the defaults, save the scheme name and notebook page number,
	//	re-populate the window with the _builtin_ scheme, then reset the
	//	scheme name and page number.
	// If the user hits cancel, then nothing is saved in user_prefs

	old_name = pPrefs->getCurrentScheme()->getSchemeName();

	currentPage = _gatherNotebookPageNum();

	pPrefs->setCurrentScheme("_builtin_");

	_populateWindowData();

	// TODO i'm not sure you want to do the following at this
	// TODO time.  setting to "defaults" should probably just
	// TODO set us to "_builtin_" and that's it.  if the user
	// TODO then changes something, we should create a new
	// TODO scheme and fill in the new value.  --jeff
	_setNotebookPageNum( currentPage );
	pPrefs->setCurrentScheme(old_name);
}

