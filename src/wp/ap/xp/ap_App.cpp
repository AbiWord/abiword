/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz and others
 * Copyright (C) 2004, 2009 Hubert Figuiere
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

#include <stdio.h>

#include "ev_EditMethod.h"
#include "ap_Features.h"
#include "ap_App.h"
#include "ap_Args.h"
#include "ap_Prefs_SchemeIds.h"
#include "ap_Strings.h"
#include "xap_Frame.h"
#include "xap_ModuleManager.h"
#include "pd_Document.h"
#include "ie_imp.h"

#if defined(TOOLKIT_WIN)

#include "ut_Win32LocaleString.h"
#include "ap_Win32App.h" //needed for AP_Win32App::s_fromWinLocaleToUTF8()

AP_App::AP_App (HINSTANCE hInstance, const char * szAppName)
  : XAP_App_BaseClass ( hInstance, szAppName )
#else
AP_App::AP_App (const char * szAppName)
  : XAP_App_BaseClass ( szAppName )
#endif
{
}

AP_App::~AP_App ()
{
}

/*!
 *  Open windows requested on commandline.
 * 
 * \return False if an unknown command line option was used, true
 * otherwise.  
 */
bool AP_App::openCmdLineFiles(const AP_Args * args)
{
	int kWindowsOpened = 0;
	const char *file = NULL;

	if (AP_Args::m_sFiles == NULL) {
		// no files to open, this is ok
		XAP_Frame * pFrame = newFrame();
		pFrame->loadDocument((const char *)NULL, IEFT_Unknown);
		return true;
	}

	int i = 0;
	while ((file = AP_Args::m_sFiles[i++]) != NULL) {
		char * uri = NULL;

		uri = UT_go_shell_arg_to_uri (file);

		XAP_Frame * pFrame = newFrame();

		UT_Error error = pFrame->loadDocument (uri, IEFT_Unknown, true);
		g_free (uri);

		if (UT_IS_IE_SUCCESS(error))
		{
			kWindowsOpened++;
			if (error == UT_IE_TRY_RECOVER) {
				pFrame->showMessageBox(AP_STRING_ID_MSG_OpenRecovered,
                           XAP_Dialog_MessageBox::b_O,
                           XAP_Dialog_MessageBox::a_OK);
			}
		}
		else
		{
			// TODO we crash if we just delete this without putting something
			// TODO in it, so let's go ahead and open an untitled document
			// TODO for now.  this would cause us to get 2 untitled documents
			// TODO if the user gave us 2 bogus pathnames....

			// Because of the incremental loader, we should not crash anymore;
			// I've got other things to do now though.
			kWindowsOpened++;
			pFrame->loadDocument((const char *)NULL, IEFT_Unknown);
			pFrame->raise();

			errorMsgBadFile (pFrame, file, error);
		}

		if (args->m_sMerge) {
			PD_Document * pDoc = static_cast<PD_Document*>(pFrame->getCurrentDoc());
			pDoc->setMailMergeLink(args->m_sMerge);
		}
	}

	if (kWindowsOpened == 0)
	{
		// no documents specified or openable, open an untitled one
		
		XAP_Frame * pFrame = newFrame();
		pFrame->loadDocument((const char *)NULL, IEFT_Unknown);
		if (args->m_sMerge) {
			PD_Document * pDoc = static_cast<PD_Document*>(pFrame->getCurrentDoc());
			pDoc->setMailMergeLink(args->m_sMerge);
		}
	}

	return true;
}

bool AP_App::openCmdLinePlugins(const AP_Args * Args, bool &bSuccess)
{
	if(Args->m_sPluginArgs)
	{
//
// Start a plugin rather than the main abiword application.
//
	    const char * szName = NULL;
		XAP_Module * pModule = NULL;
		const char * szRequest = NULL;
		bool bFound = false;	
		if(Args->m_sPluginArgs[0])
		{
			szRequest = Args->m_sPluginArgs[0];
			const UT_GenericVector<XAP_Module*> * pVec = XAP_ModuleManager::instance().enumModules ();
			UT_DEBUGMSG((" %d plugins loaded \n",pVec->getItemCount()));
			for (UT_sint32 i = 0; (i < pVec->size()) && !bFound; i++)
			{
				pModule = pVec->getNthItem (i);
				szName = pModule->getModuleInfo()->name;
				UT_DEBUGMSG(("%s\n", szName));
				if(strcmp(szName,szRequest) == 0)
				{
					bFound = true;
				}
			}
		}
		if(!bFound)
		{
			fprintf(stderr, "Plugin %s not found or loaded \n",szRequest);
			bSuccess = false;
			return false;
		}
//
// You must put the name of the ev_EditMethod in the usage field
// of the plugin registered information.
//
		const char * evExecute = pModule->getModuleInfo()->usage;
		EV_EditMethodContainer* pEMC = Args->getApp()->getEditMethodContainer();
		const EV_EditMethod * pInvoke = pEMC->findEditMethodByName(evExecute);
		if(!pInvoke)
		{
			fprintf(stderr, "Plugin %s invoke method %s not found \n",
					Args->m_sPluginArgs[0],evExecute);
			bSuccess = false;
			return false;
		}
//
// Execute the plugin, then quit
//
		UT_String *sCommandLine = Args->getPluginOptions();
		bSuccess = ev_EditMethod_invoke(pInvoke, *sCommandLine);
		delete sCommandLine;
		return false;
	}
	return true;
}


bool	AP_App::initialize(void)
{
	return XAP_App_BaseClass::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings);
}

void AP_App::errorMsgBadArg (AP_Args *, int)
{
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
}

void AP_App::errorMsgBadFile(XAP_Frame *, const char *, UT_Error)
{
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
}

bool AP_App::doWindowlessArgs (const AP_Args *, bool & /*bSuccess*/)
{
	return false;
}

void AP_App::saveRecoveryFiles()
{
	IEFileType abiType = IE_Imp::fileTypeForSuffix(".abw");

	for(UT_sint32 i = 0; i < m_vecFrames.getItemCount(); i++) {
		XAP_Frame * curFrame = m_vecFrames[i];
		if(!curFrame) {
			continue;
		}
		try {
			if (NULL == curFrame->getFilename()) {
				curFrame->backup(".abw.saved",abiType);
			}
			else {
				curFrame->backup(".saved",abiType);
			}
		}
		catch(...) {
			// just continue
		}
	}
}

