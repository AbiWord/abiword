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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdio.h>

#include "ap_Features.h"
#include "ap_Prefs.h"
#include "xap_App.h"
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ap_Args.h"

#include "xap_EncodingManager.h"
/*****************************************************************/

AP_Prefs::AP_Prefs()
	: XAP_Prefs()
{
}

AP_Prefs::~AP_Prefs(void)
{
}

/*****************************************************************/

void AP_Prefs::fullInit(void)
{
	// do full init from all the various sources.
	//
	// first we load the builtin set (from memory) and overlay
	// any system defaults onto it.
	//
	// next we load the user's personal preference file from
	// disk (in the user's home directory).  this contains
	// global settings and possibly one or more different sets
	// of preferences.
	//
	// we then optionally overlay the environment prefs
	// (primarily for locale stuff) onto the builtin set.
	// we do this last because the UseEnv... directive is
	// in the <select...> of the user's profile.  note,
	// we do not do any reinterpretation of the user's
	// disk settings (that is, we do not overlay the
	// environment onto explicitly set user preferences).
	//
	// TODO overlay command line arguments onto preferences...

	// group all the prefListener signals into one block change
	startBlockChange();	

	loadBuiltinPrefs();
	overlayEnvironmentPrefs();
	loadPrefsFile();

	// stop blocking the signal and send the combined one
	endBlockChange();
}
		   
/*****************************************************************/

const gchar * AP_Prefs::getBuiltinSchemeName(void) const
{
	return "_builtin_";
}


bool AP_Prefs::loadBuiltinPrefs(void)
{
	// we have a built-in table of name/value pairs
	// (see {xap,ap}_Prefs_SchemeIds.h) that we
	// load into a prefs objects.
	//
	// we then overlay any name/value pairs in the
	// system default file.  this is installed in
	// a system library directory (see the canonical
	// layout).  we let the System Administrator to
	// override our built-in values.
	
	const gchar * szBuiltinSchemeName = getBuiltinSchemeName();
	
	XAP_PrefsScheme * pScheme = new XAP_PrefsScheme(this, szBuiltinSchemeName);
	if (!pScheme)
		return false;

	struct _table
	{
		const gchar *		m_szKey;
		const gchar *		m_szValue;
	};

	struct _table _t[] =
	{
#		define dcl(basename)			{ XAP_PREF_KEY_##basename, XAP_PREF_DEFAULT_##basename },
#		include "xap_Prefs_SchemeIds.h"
#		undef dcl

#		define dcl(basename)			{ AP_PREF_KEY_##basename, AP_PREF_DEFAULT_##basename },
#		include "ap_Prefs_SchemeIds.h"
#		undef dcl
	};

	// Must do XML special character decoding on the default values
	// since that will automatically happen in the case of values
	// values read from preferences files.
	UT_uint32 k;
	for (k=0; k<G_N_ELEMENTS(_t); k++)
	{
		const gchar *xp;
		bool bDelete = true;

		// do not decode empty strings
		if(_t[k].m_szValue && !*(_t[k].m_szValue))
		{
			xp = _t[k].m_szValue;
			bDelete = false;
		}
		else
		{
			xp =  (gchar*)UT_XML_Decode(_t[k].m_szValue);
		}
		
		UT_DEBUGMSG(("DEFAULT %s |%s|%s|\n", _t[k].m_szKey, _t[k].m_szValue, xp));
		bool bOK = pScheme->setValue(_t[k].m_szKey, xp);
		if(bDelete)
			FREEP(xp);
		if (!bOK)
		{
			goto Failed;
		}
	}
	addScheme(pScheme);					// set the builtin scheme in the base class
	overlaySystemPrefs();				// so that the base class parser can overlay it.
	
	return setCurrentScheme(szBuiltinSchemeName);
	
Failed:
	DELETEP(pScheme);
	return false;
}

void AP_Prefs::overlaySystemPrefs(void)
{
	// read system prefs file and overlay builtin values.
	const char** items = localeinfo_combinations("system.profile","","-",0);
	std::string path;
	while(*items) {
	    const char * item = *items++;
#ifdef _MSC_VER
		const char* subdir = "profiles";
#else
		const char* subdir = NULL;
#endif
	    if (XAP_App::getApp()->findAbiSuiteAppFile(path, item, subdir))
			loadSystemDefaultPrefsFile(path.c_str());
	};
}


const char * AP_Prefs::getPrefsPathname(void) const
{
	if(AP_Args::m_sUserProfile)
	{
		UT_DEBUGMSG(("AP_Prefs::getPrefsPathname: using cmd line value [%s]\n",
					 AP_Args::m_sUserProfile));
		
		return AP_Args::m_sUserProfile;
	}
	else
		return _getPrefsPathname();
}

