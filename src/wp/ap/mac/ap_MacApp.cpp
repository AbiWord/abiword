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

/*****************************************************************
** Only one of these is created by the application.
*****************************************************************/

#include <stdio.h>
#include <string.h>

#include "xap_Args.h"
#include "ap_MacFrame.h"
#include "ap_MacApp.h"
#include "ap_Strings.h"
#include "sp_spell.h"

#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"       

#include "ie_exp.h"
#include "ie_imp.h"
#include "ie_impexp_Register.h"

/*****************************************************************/

AP_MacApp::AP_MacApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_MacApp(pArgs,szAppName)
{
	m_pStringSet = NULL;
	m_pClipboard = NULL;
    m_pViewSelection = NULL;
}

AP_MacApp::~AP_MacApp(void)
{
	//SpellCheckCleanup(); Spell check doesn't compile...yet (SBK)
	DELETEP(m_pStringSet);
	DELETEP(m_pClipboard);

	IE_Exp::unregisterAllExporters ();
	IE_Imp::unregisterAllImporters ();
}

bool AP_MacApp::initialize(void)
{
	// load preferences, first the builtin set and then any on disk.
	
	m_prefs = new AP_MacPrefs(this);
	m_prefs->loadBuiltinPrefs();
	m_prefs->loadPrefsFile();

	// TODO overlay command line arguments onto preferences...
		   

	// RFC: shouldn't the code above go to XP ?		   
	m_pClipboard = new AP_MacClipboard();
	UT_ASSERT(m_pClipboard);
	
	m_pEMC = AP_GetEditMethods();
	UT_ASSERT(m_pEMC);

	m_pBindingSet = new AP_BindingSet(m_pEMC);
	UT_ASSERT(m_pBindingSet);

	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_ASSERT(m_pMenuActionSet);

	m_pToolbarActionSet = AP_CreateToolbarActionSet();
	UT_ASSERT(m_pToolbarActionSet);
	
	// now that preferences are established, let the xap init
	if (! XAP_MacApp::initialize())
		return false;

	//////////////////////////////////////////////////////////////////
	// Initialize the importers/exporters
	//////////////////////////////////////////////////////////////////
	IE_ImpExp_RegisterXP ();

#if 0
	//////////////////////////////////////////////////////////////////
	// initializes the spell checker.
	//////////////////////////////////////////////////////////////////
	
	{
		const char * szISpellDirectory = NULL;
		const char * szSpellCheckWordList = NULL;
		getPrefsValue(AP_PREF_KEY_MacISpellDirectory,&szISpellDirectory);
		UT_ASSERT((szISpellDirectory) && (*szISpellDirectory));
		getPrefsValue(AP_PREF_KEY_SpellCheckWordList,&szSpellCheckWordList);
		UT_ASSERT((szSpellCheckWordList) && (*szSpellCheckWordList));
		
		char * szPathname = (char *)calloc(sizeof(char),strlen(szISpellDirectory)+strlen(szSpellCheckWordList)+2);
		UT_ASSERT(szPathname);
		
		sprintf(szPathname,"%s/%s",szISpellDirectory,szSpellCheckWordList);		// TODO fix pathname construction
		UT_DEBUGMSG(("Loading SpellCheckWordList [%s]\n",szPathname));
		SpellCheckInit(szPathname);
		free(szPathname);
		
		// we silently go on if we cannot load it....
	}
	
	//////////////////////////////////////////////////////////////////
#endif
	// assume we will be using the builtin set (either as the main
	// set or as the fallback set).
	    
	AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,(XML_Char*)AP_PREF_DEFAULT_StringSet);
	UT_ASSERT(pBuiltinStringSet);
	m_pStringSet = pBuiltinStringSet;

    return true;
}


XAP_Frame * AP_MacApp::newFrame(void)
{
	AP_MacFrame * pMacFrame = new AP_MacFrame(this);

	if (pMacFrame)
		pMacFrame->initialize();

	pMacFrame->loadDocument(NULL, IEFT_Unknown);

	return pMacFrame;
}

bool AP_MacApp::shutdown(void)
{
	if (m_prefs->getAutoSavePrefs())
		m_prefs->savePrefsFile();

	return true;
}

const char * AP_MacApp::getAbiSuiteAppDir(void) const
{
	// we return a static string, use it quickly.
	// TODO
	static XML_Char buf[1024] = "";
	return buf;
}

const XAP_StringSet * AP_MacApp::getStringSet(void) const
{
	return m_pStringSet;
}

void AP_MacApp::copyToClipboard(PD_DocumentRange * /*pDocRange*/)
{
}

void AP_MacApp::pasteFromClipboard(PD_DocumentRange * /*pDocRange*/, bool, bool bHonorFormatting = true)
{
}

bool AP_MacApp::canPasteFromClipboard(void)
{
	return false;
}

/*****************************************************************/

int AP_MacApp::MacMain(const char * szAppName, int argc, char **argv)
{
	// initialize our application.

	XAP_MacApp::InitializeMacToolbox ();

	XAP_Args Args = XAP_Args(argc,argv);
	
	AP_MacApp * pMyMacApp = new AP_MacApp(&Args, szAppName);
	pMyMacApp->initialize();

	// create the first window.

//	AP_MacFrame * pFirstMacFrame = new AP_MacFrame(pMyMacApp);
//	pFirstMacFrame->initialize();
        pMyMacApp->newFrame ();

	// turn over control to windows
	pMyMacApp->run();
	// destroy the App.  It should take care of deleting all frames.

	pMyMacApp->shutdown();
	delete pMyMacApp;

	return 0;
}
