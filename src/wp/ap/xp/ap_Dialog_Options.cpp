/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Prefs.h"

#include "ap_Dialog_Options.h"
#include "ap_Prefs_SchemeIds.h"

AP_Dialog_Options::AP_Dialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{
	m_answer = a_OK;
}

AP_Dialog_Options::~AP_Dialog_Options(void)
{
}

AP_Dialog_Options::tAnswer AP_Dialog_Options::getAnswer(void) const
{
	return m_answer;
}

inline void Save_Pref_Bool(  XAP_PrefsScheme *pPrefsScheme, 
						XML_Char const * key, 
						UT_Bool var ) 
{
	XML_Char szBuffer[2] = {0,0};
	szBuffer[0] = ((var)==UT_TRUE ? '1' : '0'); 
	pPrefsScheme->setValue( key, szBuffer );
}

void AP_Dialog_Options::_storeWindowData(void)
{
	XAP_Prefs *pPrefs = m_pApp->getPrefs();
	UT_ASSERT(pPrefs);

	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	UT_ASSERT(pPrefs->getCurrentScheme());

	// check to see if there is any other scheme besides _buildin_
	UT_ASSERT( pPrefsScheme );	
	if ( !strcmp( pPrefs->getCurrentScheme()->getSchemeName(), "_builtin_") ) {
		// TODO: need to decide on a scheme default name
		const XML_Char new_name[] = "user_prefs";
		
		pPrefsScheme = new XAP_PrefsScheme(pPrefs, new_name);
		UT_ASSERT( pPrefsScheme );	
		pPrefs->addScheme( pPrefsScheme );
		pPrefs->setCurrentScheme( new_name );
	}

	// save the values to the Prefs classes
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_AutoSpellCheck, _gatherSpellCheckAsType() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_SpellCheckCaps, _gatherSpellUppercase() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_SpellCheckNumbers, _gatherSpellNumbers() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_SpellCheckInternet, _gatherSpellInternet() );


	//TODO m_pApp  get current view so we can _toggleAutoSpell

	// do we want to save the preferences automatically here?
	// probably not, but for now, lets do until I add the TODO 
	// "do you want to save your changes?" dialog
	//			shack@uiuc.edu
	if ( pPrefs->getAutoSavePrefs() == UT_FALSE ) {
		// TODO: check the results
		pPrefs->savePrefsFile();
	}
}

void AP_Dialog_Options::_populateWindowData(void)
{
	UT_Bool			b;
	XAP_Prefs		*pPrefs;
	// TODO: move this logic when we get a PrefsListener API and turn this
	//		 dialog into an app-specific

	pPrefs = m_pApp->getPrefs();
	UT_ASSERT( pPrefs );

	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_AutoSpellCheck,&b))
		_setSpellCheckAsType (b);

	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_SpellCheckCaps,&b))
		_setSpellUppercase (b);

	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_SpellCheckNumbers,&b))
		_setSpellNumbers (b);

	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_SpellCheckInternet,&b))
		_setSpellInternet (b);

	// enable/disable controls
	_initEnableControls();
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

	default:
		// do nothing
		break;
	}	
}

// The initialize the controls (i.e., disable controls not coded yet)
void AP_Dialog_Options::_initEnableControls()
{
	_controlEnable( id_CHECK_SPELL_SUGGEST,			UT_FALSE );
	_controlEnable( id_CHECK_SPELL_HIDE_ERRORS,		UT_FALSE );
	_controlEnable( id_CHECK_SPELL_MAIN_ONLY,		UT_FALSE );
	_controlEnable( id_CHECK_SPELL_NUMBERS,			UT_FALSE );
	_controlEnable( id_CHECK_SPELL_INTERNET,		UT_FALSE );
	_controlEnable( id_LIST_DICTIONARY,				UT_FALSE );
	_controlEnable( id_CHECK_PREFS_AUTO_SAVE,		UT_FALSE );
	_controlEnable( id_COMBO_PREFS_SCHEME,			UT_FALSE );
	_controlEnable( id_CHECK_VIEW_SHOW_RULER,		UT_FALSE );
	_controlEnable( id_LIST_VIEW_RULER_UNITS,		UT_FALSE );
	_controlEnable( id_CHECK_VIEW_SHOW_TOOLBARS,	UT_FALSE );
	_controlEnable( id_CHECK_VIEW_ALL,				UT_FALSE );
	_controlEnable( id_CHECK_VIEW_HIDDEN_TEXT,		UT_FALSE );
	_controlEnable( id_CHECK_VIEW_UNPRINTABLE,		UT_FALSE );
	_controlEnable( id_BUTTON_SAVE,					UT_FALSE );
}

void AP_Dialog_Options::_event_SetDefaults(void)
{
	XAP_Prefs	*pPrefs;
	XML_Char	*old_name;

	pPrefs = m_pApp->getPrefs();
	UT_ASSERT(pPrefs);

	old_name = pPrefs->getCurrentScheme()->getSchemeName();

	pPrefs->setCurrentScheme("_builtin_");		

	_populateWindowData();

	pPrefs->setCurrentScheme(old_name);
}
