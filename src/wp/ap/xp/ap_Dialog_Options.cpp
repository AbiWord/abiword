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
#include "ut_units.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Prefs.h"

#include "ap_Dialog_Options.h"
#include "ap_Prefs_SchemeIds.h"
#include "ap_Strings.h"

AP_Dialog_Options::AP_Dialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{
	m_answer = a_OK;
	m_pFrame = (XAP_Frame *)0;		// needs to be set from runModal for some of the event_'s to work
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

	// turn off all notification to PrefListeners via XAP_Prefs
	pPrefs->startBlockChange();

	// before we continue to remember all the changed values, check to see if
	// we have turned OFF PrefsAutoSave.  If so, toggle that value, then force
	// a prefs save, then update everything else
	//			shack@uiuc.edu
	if ( pPrefs->getAutoSavePrefs() == UT_TRUE && _gatherPrefsAutoSave() == UT_FALSE ) {

		pPrefs->setAutoSavePrefs( UT_FALSE );
		pPrefs->savePrefsFile();				// TODO: check the results
	}
	else {	// otherwise, just set the value
		pPrefs->setAutoSavePrefs( _gatherPrefsAutoSave() );
	}

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


	// TODO: change to snprintf
	XML_Char szBuffer[40];
	sprintf( szBuffer, "%i", _gatherNotebookPageNum() );
	pPrefsScheme->setValue( AP_PREF_KEY_OptionsTabNumber, szBuffer );

	// save ruler units value
	pPrefsScheme->setValue( AP_PREF_KEY_RulerUnits, UT_dimensionName( _gatherViewRulerUnits()) );

	// allow XAP_Prefs to notify all the listeners of changes
	pPrefs->endBlockChange();

	// if we hit the Save button, then force a save after the gather
	if ( m_answer == a_SAVE ) {
		pPrefs->savePrefsFile();				// TODO: check the results
	}

}

void AP_Dialog_Options::_eventSave(void)
{
	m_answer = a_SAVE;

	_storeWindowData();	

	m_answer = a_OK;
}

void AP_Dialog_Options::_populateWindowData(void)
{
	UT_Bool			b;
	XAP_Prefs		*pPrefs;
	const XML_Char	*pszBuffer;	

	// TODO: move this logic when we get a PrefsListener API and turn this
	//		 dialog into an app-specific

	pPrefs = m_pApp->getPrefs();
	UT_ASSERT( pPrefs );

	// ------------ Spell
	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_AutoSpellCheck,&b))
		_setSpellCheckAsType (b);

	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_SpellCheckCaps,&b))
		_setSpellUppercase (b);

	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_SpellCheckNumbers,&b))
		_setSpellNumbers (b);

	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_SpellCheckInternet,&b))
		_setSpellInternet (b);
	
	// ------------ Prefs	
	_setPrefsAutoSave( pPrefs->getAutoSavePrefs() );

	// ------------ View
	if (pPrefs->getPrefsValue(AP_PREF_KEY_RulerUnits,&pszBuffer))
		_setViewRulerUnits (UT_determineDimension(pszBuffer));

	// ------------ the page tab number 
	if (pPrefs->getPrefsValue(AP_PREF_KEY_OptionsTabNumber,&pszBuffer))
		_setNotebookPageNum (atoi(pszBuffer));


	// enable/disable controls
	_initEnableControls();
}


void AP_Dialog_Options::_enableDisableLogic( tControl id )
{
#if 0
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
#endif
}

// The initialize the controls (i.e., disable controls not coded yet)
void AP_Dialog_Options::_initEnableControls()
{
	// spelling
	_controlEnable( id_CHECK_SPELL_SUGGEST,			UT_FALSE );
	_controlEnable( id_CHECK_SPELL_HIDE_ERRORS,		UT_FALSE );
	_controlEnable( id_CHECK_SPELL_MAIN_ONLY,		UT_FALSE );
	_controlEnable( id_CHECK_SPELL_INTERNET,		UT_FALSE );
	_controlEnable( id_LIST_DICTIONARY,				UT_FALSE );
	_controlEnable( id_BUTTON_DICTIONARY_EDIT,		UT_FALSE );
	_controlEnable( id_BUTTON_IGNORE_EDIT,			UT_FALSE );

	// prefs
	_controlEnable( id_COMBO_PREFS_SCHEME,			UT_FALSE );

	// view
	_controlEnable( id_CHECK_VIEW_SHOW_RULER,		UT_FALSE );
	_controlEnable( id_CHECK_VIEW_SHOW_TOOLBARS,	UT_FALSE );
	_controlEnable( id_CHECK_VIEW_ALL,				UT_FALSE );
	_controlEnable( id_CHECK_VIEW_HIDDEN_TEXT,		UT_FALSE );
	_controlEnable( id_CHECK_VIEW_UNPRINTABLE,		UT_FALSE );

	// general
	_controlEnable( id_BUTTON_SAVE,					UT_FALSE );
}

void AP_Dialog_Options::_event_SetDefaults(void)
{
	XAP_Prefs		*pPrefs;
	const XML_Char	*old_name;
	int currentPage;

	pPrefs = m_pApp->getPrefs();
	UT_ASSERT(pPrefs);

	// SetDefaults
	//	To set the defaults, save the scheme name and notebook page number,
	//  re-populate the window with the _buildit_ scheme, then reset the 
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

void AP_Dialog_Options::_event_IgnoreReset(void)
{
	UT_DEBUGMSG(("AP_Dialog_Options::_event_IgnoreReset"));
	UT_ASSERT( m_pFrame );

#if 0
	// TODO:  shack@uiuc.edu: waiting for a vote for reset strings...

    XAP_DialogFactory * pDialogFactory
        = (XAP_DialogFactory *)(m_pFrame->getDialogFactory());

    XAP_Dialog_MessageBox * pDialog
        = (XAP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
    UT_ASSERT(pDialog);

    const XAP_StringSet * pSS = m_pApp->getStringSet();

    pDialog->setMessage(pSS->getValue(AP_STRING_ID_DLG_Options_Spell_IgnoreResetAll));
    pDialog->setButtons(XAP_Dialog_MessageBox::b_YNC);
    pDialog->setDefaultAnswer(XAP_Dialog_MessageBox::a_NO); // should this be YES?

    pDialog->runModal(m_pFrame);

    XAP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();

    pDialogFactory->releaseDialog(pDialog);

	// if hit cancel, go no further
	if (ans == XAP_Dialog_MessageBox::a_CANCEL)
	{
		UT_DEBUGMSG(("Canceled"));
		return;
	}

    if (ans == XAP_Dialog_MessageBox::a_YES)
	{
		UT_DEBUGMSG(("Yes"));
		// do it

	}
#endif
}

void AP_Dialog_Options::_event_IgnoreEdit(void)
{
	UT_DEBUGMSG(("AP_Dialog_Options::_event_IgnoreEdit"));
}

void AP_Dialog_Options::_event_DictionaryEdit(void)
{
	UT_DEBUGMSG(("AP_Dialog_Options::_event_DictionaryEdit"));
}
