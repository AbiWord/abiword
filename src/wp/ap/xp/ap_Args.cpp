/* AbiWord
 * Copyright (C) 2002 Patrick Lam
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

#include "xap_Strings.h"
#include "ap_Strings.h"
#include "ap_Prefs_SchemeIds.h"
#include "ap_Args.h"
#include "ap_App.h"
#include "ap_Convert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"

#ifdef ABI_OPT_PERL
#include "ut_PerlBindings.h"
#endif

#include <popt.h>

/*****************************************************************/

// Static initializations:
#ifdef ABI_OPT_PERL
const char * AP_Args::m_sScript = NULL;
#endif
#ifdef DEBUG
int AP_Args::m_iDumpstrings = 0;
#endif
const char * AP_Args::m_sGeometry = NULL;
const char * AP_Args::m_sTo = NULL;
int    AP_Args::m_iToPNG = 0;
const char * AP_Args::m_sPrintTo = NULL;
int    AP_Args::m_iVerbose = 1;
const char * AP_Args::m_sPlugin = NULL;
int    AP_Args::m_iNosplash = 0;
const char * AP_Args::m_sFile = NULL;
int    AP_Args::m_iVersion = 0;
int    AP_Args::m_iHelp = 0;
const char * AP_Args::m_sDisplay = NULL;
struct poptOption * AP_Args::options = NULL;
int  AP_Args::m_iAbiControl = 0;
const char * AP_Args::m_sMerge = NULL;

AP_Args::AP_Args(XAP_Args * pArgs, const char * szAppName, AP_App * pApp)
	: XArgs (pArgs), poptcon(NULL), m_bShowSplash(true), m_pApp(pApp)
{
	pApp->initPopt (this);
}

AP_Args::~AP_Args()
{
	if (poptcon != NULL)
		poptFreeContext(poptcon);
	FREEP(options);
}

/*****************************************************************/

/*! Processes all the command line options and puts them in AP_Args.
 * Leaves the files to open in the poptContext for ::openCmdLineFiles
 * to handle.
 *
 * Note that GNOME does this for us!
 */
void AP_Args::parsePoptOpts ()
{
	int nextopt;
	poptcon = poptGetContext("AbiWord", 
				       XArgs->m_argc, XArgs->m_argv, 
				       options, 0);

    while ((nextopt = poptGetNextOpt (poptcon)) > 0 &&
		   nextopt != POPT_ERROR_BADOPT)
        /* do nothing */ ;

    if (nextopt != -1) 
	{
		m_pApp->errorMsgBadArg(this, nextopt);
        exit (1);
    }

 	if (m_iVersion)
 	{		
 		printf("%s\n", XAP_App::s_szBuild_Version);
		#ifdef _WIN32
			MessageBox(NULL, XAP_App::s_szBuild_Version, "Version", MB_OK|MB_ICONINFORMATION);
		#endif
		exit(0);
 	}

	if (m_iHelp)
	{
		poptPrintHelp(poptcon, stdout, 0);
		exit(0);
	}
}

/*!
 * Handles arguments which require an XAP_App but no windows.
 * It has a callback to getApp()::doWindowlessArgs().
 */
bool AP_Args::doWindowlessArgs()
{
#ifdef DEBUG
	if (m_iDumpstrings)
	{
		// dump the string table in english as a template for translators.
		// see abi/docs/AbiSource_Localization.abw for details.
		AP_BuiltinStringSet * pBuiltinStringSet = 
			new AP_BuiltinStringSet(getApp(),
									static_cast<XML_Char*>(AP_PREF_DEFAULT_StringSet));
		pBuiltinStringSet->dumpBuiltinSet("en-US.strings");
		delete pBuiltinStringSet;
	}
#endif

#ifdef ABI_OPT_PERL
	if (m_sScript)
	{
		UT_PerlBindings& pb(UT_PerlBindings::getInstance());
		if (!pb.evalFile(m_sScript))
			printf("%s\n", pb.errmsg().c_str());
	}
#endif

	if (m_sTo) 
	{
		AP_Convert * conv = new AP_Convert();
		conv->setVerbose(m_iVerbose);
		while ((m_sFile = poptGetArg (poptcon)) != NULL)
			conv->convertTo(m_sFile, m_sTo);
		delete conv;
		return false;
	}
	
	if (m_iToPNG) {
	  AP_Convert * conv = new AP_Convert();
	  conv->setVerbose(m_iVerbose);
// this has the problem that it calls the convertToPNG method on
// ie_impGraphicPNG, which perhaps doesn't like you freeing the
// returned buffer.
	  while ((m_sFile = poptGetArg (poptcon)) != NULL)
	    conv->convertToPNG(m_sFile);
	  delete conv;

	  return false;
	}

	if (!m_pApp->doWindowlessArgs(this))
		return false;

	if (m_sTo || m_iToPNG || m_sPrintTo || m_iNosplash || m_sPlugin)
	{
	    m_bShowSplash = false;
	}

	return true;
}

/*****************************************************************/

const struct poptOption AP_Args::const_opts[] =
	{{"geometry", 'g', POPT_ARG_STRING, &m_sGeometry, 0, "Set initial frame geometry", "GEOMETRY"},
	 {"nosplash", 'n', POPT_ARG_NONE,   &m_iNosplash, 0, "Do not show splash screen", NULL},
#ifdef ABI_OPT_PERL
	 {"script", 's', POPT_ARG_STRING, &m_sScript, 0, "Execute FILE as script", "FILE"},
#endif
#ifdef DEBUG
	 {"dumpstrings", 'd', POPT_ARG_NONE, &m_iDumpstrings, 0, "Dump strings to file", NULL},
#endif
	 {"to", 't', POPT_ARG_STRING, &m_sTo, 0, "Target format of the file (abw, zabw, rtf, txt, utf8, html, latex)", "FORMAT"},
	 {"to-png", '\0', POPT_ARG_NONE, &m_iToPNG, 0, "Convert incoming file to a PNG image", ""},
	 {"verbose", 'v', POPT_ARG_INT, &m_iVerbose, 0, "Set verbosity level (0, 1, 2)", "LEVEL"},
	 {"print", 'p', POPT_ARG_STRING, &m_sPrintTo, 0, "Print this file to FILE or printer", "FILE or |lpr"},
	 {"plugin", '\0', POPT_ARG_STRING, &m_sPlugin, 0, "Execute plugin NAME instead of the main application", "NAME"},
	 {"merge", 'm', POPT_ARG_STRING, &m_sMerge, 0, "Mail-merge", "FILE"},
	 {"AbiControl", '\0', POPT_ARG_NONE, &m_iAbiControl, 0, "Execute plugin AbiControl instead of the main application", ""},
	 // GNOME build kills everything after "version"
	 {"version", '\0', POPT_ARG_NONE, &m_iVersion, 0, "Print AbiWord version", NULL},
 	 {"help", '?', POPT_ARG_NONE, &m_iHelp, 0, "Display help", NULL},
	 {NULL, '\0', 0, NULL, 0, NULL, NULL} /* end the list */
	};
