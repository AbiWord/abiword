/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz and others
 * Copyright (C) 2004 Hubert Figuiere
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

#include "ap_Features.h"
#include "ap_App.h"
#include "ap_Args.h"
#include "ap_Prefs_SchemeIds.h"
#include "xap_Frame.h"
#include "pd_Document.h"

#include "ie_imp.h"

#if defined(WIN32)

#include "ap_Win32App.h" //needed for AP_Win32App::s_fromWinLocaleToUTF8()

AP_App::AP_App (HINSTANCE hInstance, XAP_Args * pArgs, const char * szAppName)
  : XAP_App_BaseClass ( hInstance, pArgs, szAppName )
#else
AP_App::AP_App (XAP_Args * pArgs, const char * szAppName)
  : XAP_App_BaseClass ( pArgs, szAppName )
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
bool AP_App::openCmdLineFiles(AP_Args * args)
{
	int kWindowsOpened = 0;
	const char *file = NULL;
	poptContext poptcon = args->poptcon;

	while ((file = poptGetArg (poptcon)) != NULL) {
		XAP_Frame * pFrame = newFrame();

		char * uri = NULL;

#if defined(WIN32)
		uri = UT_go_shell_arg_to_uri (AP_Win32App::s_fromWinLocaleToUTF8(file).utf8_str());
#else
		uri = UT_go_shell_arg_to_uri (file);
#endif

		UT_Error error = pFrame->loadDocument (uri, IEFT_Unknown, true);
		g_free (uri);

		if (!error)
		{
			kWindowsOpened++;
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

bool	AP_App::initialize(void)
{
	return XAP_App_BaseClass::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings);
}


/*! Prepares for popt to be callable by setting up Args->options.
 * Needs to be in AP_App so that platform code can subclass (eg GNOME).
 */
void AP_App::initPopt (AP_Args * Args)
{
	UT_uint32 i;

	for (i = 0; Args->const_opts[i].longName != NULL; i++)
		;

	struct poptOption * opts = (struct poptOption *)
		UT_calloc(i+1, sizeof(struct poptOption));
	for (UT_uint32 j = 0; j < i; j++)
		opts[j] = Args->const_opts[j];

	Args->options = opts;
}

void AP_App::errorMsgBadArg (AP_Args *, int)
{
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
}

void AP_App::errorMsgBadFile(XAP_Frame *, const char *, UT_Error)
{
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
}

bool AP_App::doWindowlessArgs (const AP_Args *, bool & bSuccess)
{
	return false;
}

