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
#include "xad_Document.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Prefs.h"
#include "fv_View.h"
#include "fl_DocLayout.h"

#include "ap_Dialog_Options.h"
#include "ap_Prefs_SchemeIds.h"
#include "ap_Strings.h"
#include "ap_FrameData.h"

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

	AP_FrameData *pFrameData = (AP_FrameData *)m_pFrame->getFrameData();
	UT_ASSERT(pFrameData);

	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	UT_ASSERT(pPrefsScheme);

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

	// try again to make sure we've got an updatable scheme
	pPrefsScheme = pPrefs->getCurrentScheme(UT_TRUE);
	UT_ASSERT(pPrefsScheme);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	// save the values to the Prefs classes
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_AutoSpellCheck, _gatherSpellCheckAsType() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_SpellCheckCaps, _gatherSpellUppercase() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_SpellCheckNumbers, _gatherSpellNumbers() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_SpellCheckInternet, _gatherSpellInternet() );

	Save_Pref_Bool( pPrefsScheme, XAP_PREF_KEY_SmartQuotesEnable, _gatherSmartQuotesEnable() );

	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_CursorBlink, _gatherViewCursorBlink() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_RulerVisible, _gatherViewShowRuler() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_StandardBarVisible, _gatherViewShowStandardBar() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_FormatBarVisible, _gatherViewShowFormatBar() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_ExtraBarVisible, _gatherViewShowExtraBar() );
	Save_Pref_Bool( pPrefsScheme, AP_PREF_KEY_ParaVisible, _gatherViewUnprintable() );

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	// If we changed whether the ruler is to be visible
	// or hidden, then update the current window:
	// (If we didn't change anything, leave it alone)
	if ( _gatherViewShowRuler() != pFrameData->m_bShowRuler )
	{
		pFrameData->m_bShowRuler = _gatherViewShowRuler() ;
		m_pFrame->toggleRuler( _gatherViewShowRuler() );
	}

	// TODO: Don't use 0, 1, 2, but AP_TOOLBAR_STANDARD, AP_TOOLBAR_FORMAT, AP_TOOLBAR_EXTRA...
	if (_gatherViewShowStandardBar() != pFrameData->m_bShowBar[0])
	{
		pFrameData->m_bShowBar[0] = _gatherViewShowStandardBar();
		m_pFrame->toggleBar(0, _gatherViewShowStandardBar());
	}

	if (_gatherViewShowFormatBar() != pFrameData->m_bShowBar[1])
	{
		pFrameData->m_bShowBar[1] = _gatherViewShowFormatBar();
		m_pFrame->toggleBar(1, _gatherViewShowFormatBar());
	}

	if (_gatherViewShowExtraBar() != pFrameData->m_bShowBar[2])
	{
		pFrameData->m_bShowBar[2] = _gatherViewShowExtraBar();
		m_pFrame->toggleBar(2, _gatherViewShowExtraBar());
	}

    if ( _gatherViewUnprintable() != pFrameData->m_bShowPara )
    {
        pFrameData->m_bShowPara = _gatherViewUnprintable() ;
        AV_View * pAVView = m_pFrame->getCurrentView();
        UT_ASSERT(pAVView);

        FV_View * pView = static_cast<FV_View *> (pAVView);

        pView->setShowPara( _gatherViewUnprintable() );
    }

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	// save ruler units value
	pPrefsScheme->setValue( AP_PREF_KEY_RulerUnits, UT_dimensionName( _gatherViewRulerUnits()) );

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	// allow XAP_Prefs to notify all the listeners of changes

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	// TODO: change to snprintf
	XML_Char szBuffer[40];
	sprintf( szBuffer, "%i", _gatherNotebookPageNum() );
	pPrefsScheme->setValue( AP_PREF_KEY_OptionsTabNumber, szBuffer );

	// allow the prefListeners to receive their calls
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
	
	// ------------ Smart Quotes
	if (pPrefs->getPrefsValueBool(XAP_PREF_KEY_SmartQuotesEnable,&b))
		_setSmartQuotesEnable (b);

	// ------------ Prefs	
	_setPrefsAutoSave( pPrefs->getAutoSavePrefs() );

	// ------------ View
	if (pPrefs->getPrefsValue(AP_PREF_KEY_RulerUnits,&pszBuffer))
		_setViewRulerUnits (UT_determineDimension(pszBuffer));

	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_RulerVisible,&b))
		_setViewShowRuler (b);

	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_StandardBarVisible,&b))
		_setViewShowStandardBar (b);

	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_FormatBarVisible,&b))
		_setViewShowFormatBar (b);

	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_ExtraBarVisible,&b))
		_setViewShowExtraBar (b);

    if (pPrefs->getPrefsValueBool(AP_PREF_KEY_ParaVisible,&b))
        _setViewUnprintable (b);

	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_CursorBlink,&b))
		_setViewCursorBlink (b);

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
	_controlEnable( id_CHECK_VIEW_ALL,				UT_FALSE );
	_controlEnable( id_CHECK_VIEW_HIDDEN_TEXT,		UT_FALSE );

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
	//  re-populate the window with the _builtin_ scheme, then reset the 
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
	UT_DEBUGMSG(("AP_Dialog_Options::_event_IgnoreReset\n"));
	UT_ASSERT( m_pFrame );

	// TODO:  shack@uiuc.edu: waiting for a vote for reset strings...

	// Ask "Do you want to reset ignored words in the current document?" 
    XAP_Dialog_MessageBox::tAnswer ans = m_pFrame->showMessageBox(AP_STRING_ID_DLG_Options_Prompt_IgnoreResetCurrent,
								XAP_Dialog_MessageBox::b_YNC,
								XAP_Dialog_MessageBox::a_NO); // should this be YES?


	// if hit cancel, go no further
	// if no hit, don't do anything else, even prompt for other docs
	if (ans == XAP_Dialog_MessageBox::a_CANCEL ||
		ans == XAP_Dialog_MessageBox::a_NO )
	{
		UT_DEBUGMSG(("No/Canceled\n"));
		return;
	}

	// do it
	UT_DEBUGMSG(("Yes\n"));
	UT_ASSERT(ans == XAP_Dialog_MessageBox::a_YES );

	// ask another question : "Do you want to reset ignored words in all the
	// documents?" , but only if # frames > 1
	XAP_App *pApp = m_pFrame->getApp();
	UT_ASSERT(pApp);
	if (pApp->getFrameCount() > 1)
	{
		
		ans = m_pFrame->showMessageBox(AP_STRING_ID_DLG_Options_Prompt_IgnoreResetAll,
									XAP_Dialog_MessageBox::b_YNC,
									XAP_Dialog_MessageBox::a_NO); // should this be YES?


		// if cancel, don't to ANYTHING	
		if (ans == XAP_Dialog_MessageBox::a_CANCEL )
		{
			UT_DEBUGMSG(("No/Canceled\n"));
			return;
		}
	} 

	// ------------------------- actually do it
	if ( ans == XAP_Dialog_MessageBox::a_NO )
	{
		// if no to all documents, then just reset current (because we made it
		// this far
		m_pFrame->getCurrentDoc()->clearIgnores();

		((FV_View *)m_pFrame->getCurrentView())->getLayout()->recheckIgnoredWords();
	}	
	else 
	{
		// reset all doc's ignored words
		UT_uint32 ndx;
		for ( ndx = 0; ndx < pApp->getFrameCount(); ndx++ )
		{
			XAP_Frame *pFrame = pApp->getFrame(ndx);

			pFrame->getCurrentDoc()->clearIgnores();
			((FV_View *)pFrame->getCurrentView())->getLayout()->recheckIgnoredWords();
		}
	}

	// TODO : recheck spelling

}

void AP_Dialog_Options::_event_IgnoreEdit(void)
{
	UT_DEBUGMSG(("AP_Dialog_Options::_event_IgnoreEdit\n"));
}

void AP_Dialog_Options::_event_DictionaryEdit(void)
{
	UT_DEBUGMSG(("AP_Dialog_Options::_event_DictionaryEdit\n"));
}
