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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
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
#include "ut_exception.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_units.h"

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

	AP_FrameData *pFrameData = (AP_FrameData *)m_pFrame->getFrameData();
	UT_return_if_fail (pFrameData);

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
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_SpellCheckCaps, _gatherSpellUppercase() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_SpellCheckNumbers, _gatherSpellNumbers() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_CursorBlink, _gatherViewCursorBlink() );
	
// Not implemented for UNIX or Win32. No need for it.
#if !defined(XP_UNIX_TARGET_GTK) && !defined(XP_TARGET_COCOA) && !defined (WIN32) 
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_RulerVisible, _gatherViewShowRuler() );
	UT_uint32 i;
	for (i = 0; i < m_pApp->getToolbarFactory()->countToolbars(); i++) {
		Save_Pref_Bool( pPrefsScheme, m_pApp->getToolbarFactory()->prefKeyForToolbar(i), _gatherViewShowToolbar(i));
	}

	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_StatusBarVisible, _gatherViewShowStatusBar() );
#endif

	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_ParaVisible, _gatherViewUnprintable() );
	Save_Pref_Bool( pPrefsScheme, XAP_PREF_KEY_AllowCustomToolbars, _gatherAllowCustomToolbars() );
#if defined(XP_UNIX_TARGET_GTK)
	Save_Pref_Bool( pPrefsScheme, XAP_PREF_KEY_EnableSmoothScrolling, _gatherEnableSmoothScrolling() );
#endif
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
#if !defined(XP_UNIX_TARGET_GTK) && !defined(XP_TARGET_COCOA) && !defined (WIN32) 
	if ( _gatherViewShowRuler() != pFrameData->m_bShowRuler )
	{
		pFrameData->m_bShowRuler = _gatherViewShowRuler() ;
		if (!pFrameData->m_bIsFullScreen)
		{
			m_pFrame->toggleRuler(pFrameData->m_bShowRuler);
		}
	}

	// Same for status bar
	if (_gatherViewShowStatusBar() != pFrameData->m_bShowStatusBar)
	{
		pFrameData->m_bShowStatusBar = _gatherViewShowStatusBar();
		if (!pFrameData->m_bIsFullScreen)
		{
			m_pFrame->toggleStatusBar(pFrameData->m_bShowStatusBar);
		}
	}


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
#endif
	
	if ( _gatherViewUnprintable() != pFrameData->m_bShowPara )
	{
		pFrameData->m_bShowPara = _gatherViewUnprintable() ;
		AV_View * pAVView = m_pFrame->getCurrentView();
		UT_return_if_fail (pAVView);

		FV_View * pView = static_cast<FV_View *> (pAVView);

		pView->setShowPara(pFrameData->m_bShowPara);
	}


	if ( _gatherAllowCustomToolbars() != XAP_App::getApp()->areToolbarsCustomizable() )
	{
		XAP_App::getApp()->setToolbarsCustomizable(_gatherAllowCustomToolbars());
	}
#if defined(XP_UNIX_TARGET_GTK)
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

	AP_FrameData *pFrameData = (AP_FrameData *)m_pFrame->getFrameData();
	UT_return_if_fail (pFrameData);

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
#if !defined(XP_UNIX_TARGET_GTK) && !defined(XP_TARGET_COCOA) && !defined (WIN32) 
			{
				bool tmpbool = _gatherViewShowRuler();
				Save_Pref_Bool (pPrefsScheme, AP_PREF_KEY_RulerVisible, tmpbool);
				if (tmpbool != pFrameData->m_bShowRuler)
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
#if !defined(XP_UNIX_TARGET_GTK) && !defined(XP_TARGET_COCOA) && !defined (WIN32) 
			{
				bool tmpbool = _gatherViewShowStatusBar();
				Save_Pref_Bool (pPrefsScheme, AP_PREF_KEY_StatusBarVisible, tmpbool);
				if (tmpbool != pFrameData->m_bShowStatusBar)
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

		case id_CHECK_ALLOW_CUSTOM_TOOLBARS:
			Save_Pref_Bool (pPrefsScheme, XAP_PREF_KEY_AllowCustomToolbars,
					_gatherAllowCustomToolbars());
			break;

		case id_CHECK_ENABLE_SMOOTH_SCROLLING:
#if defined(XP_UNIX_TARGET_GTK)
			Save_Pref_Bool (pPrefsScheme, XAP_PREF_KEY_EnableSmoothScrolling,
					_gatherEnableSmoothScrolling());
#endif
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
		case id_LIST_VIEW_TOOLBARS: // this is needed for the Cocoa front-end to fetch the control
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

	// ------------ Prefs
	_setPrefsAutoSave( pPrefs->getAutoSavePrefs() );

	// ------------ View
	if (pPrefs->getPrefsValue((gchar*)AP_PREF_KEY_RulerUnits,&pszBuffer))
		_setViewRulerUnits (UT_determineDimension(pszBuffer));


#if !defined(XP_UNIX_TARGET_GTK) && !defined(XP_TARGET_COCOA) && !defined (WIN32) 
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

	if (pPrefs->getPrefsValueBool((gchar*)AP_PREF_KEY_ParaVisible,&b))
		_setViewUnprintable (b);

	if (pPrefs->getPrefsValueBool((gchar*)AP_PREF_KEY_CursorBlink,&b))
		_setViewCursorBlink (b);

	if (pPrefs->getPrefsValueBool((gchar*)XAP_PREF_KEY_AllowCustomToolbars,&b))
		_setAllowCustomToolbars(b);

#if defined(XP_UNIX_TARGET_GTK)
	if (pPrefs->getPrefsValueBool((gchar*)XAP_PREF_KEY_EnableSmoothScrolling,&b))
		_setEnableSmoothScrolling(b);
#endif
	if (pPrefs->getPrefsValueBool((gchar*)XAP_PREF_KEY_AutoLoadPlugins,&b))
		_setAutoLoadPlugins(b);

	// TODO: JOAQUIN FIX THIS
	if (pPrefs->getPrefsValueBool((gchar*)XAP_PREF_KEY_AutoSaveFile,&b))
		_setAutoSaveFile (b);

	UT_String stBuffer;
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
		
	default:
		// do nothing
		break;
	}
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
#ifndef XP_UNIX_TARGET_GTK
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

#ifdef XP_TARGET_COCOA

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
			BoolOption bo = static_cast<BoolOption>(i);

			m_BOData[bo].m_original = m_BOData[bo].m_current;
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
	UT_TRY
		{
			manager = new AP_PreferenceSchemeManager;
		}
	UT_CATCH(...)
		{
			manager = 0;
		}
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
			UT_TRY
				{
					current = new AP_PreferenceScheme(manager, pScheme);
				}
			UT_CATCH(...)
				{
					current = 0;
				}
			if (current)
				{
					manager->m_vecSchemes.addItem(current);
					manager->m_pCurrentScheme = current;
				}
			else
				{
					DELETEP(manager);
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

	if (tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_inch))
		if (strcmp (tmp, PopUp_Units) == 0)
			return s_internal_units[0];
	if (tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_cm))
		if (strcmp (tmp, PopUp_Units) == 0)
			return s_internal_units[1];
	if (tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_mm))
		if (strcmp (tmp, PopUp_Units) == 0)
			return s_internal_units[2];
	if (tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_points))
		if (strcmp (tmp, PopUp_Units) == 0)
			return s_internal_units[3];
	if (tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_pica))
		if (strcmp (tmp, PopUp_Units) == 0)
			return s_internal_units[4];

	return s_internal_units[0];
}

void AP_PreferenceSchemeManager::_constructPopUpArrays()
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	const gchar * tmp = 0;
	char * tmpcopy = 0;

	m_PopUp_UnitsCount = 0;

	if (tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_inch))
		if (tmpcopy = g_strdup(tmp))
			m_PopUp_UnitsList[m_PopUp_UnitsCount++] = tmpcopy;
	if (tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_cm))
		if (tmpcopy = g_strdup(tmp))
			m_PopUp_UnitsList[m_PopUp_UnitsCount++] = tmpcopy;
	if (tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_mm))
		if (tmpcopy = g_strdup(tmp))
			m_PopUp_UnitsList[m_PopUp_UnitsCount++] = tmpcopy;
	if (tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_points))
		if (tmpcopy = g_strdup(tmp))
			m_PopUp_UnitsList[m_PopUp_UnitsCount++] = tmpcopy;
	if (tmp = pSS->getValue(XAP_STRING_ID_DLG_Unit_pica))
		if (tmpcopy = g_strdup(tmp))
			m_PopUp_UnitsList[m_PopUp_UnitsCount++] = tmpcopy;

	UT_ASSERT(m_PopUp_UnitsCount == 5); // must match size of s_internal_units[] // TODO: make dynamic!

	// TODO
}

#endif /* XP_TARGET_COCOA */
