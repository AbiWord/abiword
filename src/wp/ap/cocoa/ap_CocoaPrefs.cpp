/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001,2009 Hubert Figuiere
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

#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>

#include <string>

#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_App.h"
#include "ap_CocoaPrefs.h"

/*****************************************************************/

AP_CocoaPrefs::AP_CocoaPrefs()
	: AP_Prefs()
{
}

const char * AP_CocoaPrefs::_getPrefsPathname(void) const
{
	/* return a pointer to a static buffer */
	static std::string buf;

	if(!buf.empty())
	  return buf.c_str();

	const char * szDirectory = XAP_App::getApp()->getUserPrivateDirectory();
	const char * szFile = "AbiWord.Profile";

	buf = szDirectory;
	if (!buf.size() || szDirectory[buf.size()-1] != '/')
	  buf += "/";
	buf += szFile;

	return buf.c_str();
}

void AP_CocoaPrefs::overlayEnvironmentPrefs(void)
{
	// modify the "_builtin_" preferences with
	// using information in the user's environment.
	// we do not overlay a custom set of preferences.

	if (!m_bUseEnvLocale)
		return;							// nothing to do...

#if 1
	// TODO use various POSIX env variables
	// TODO (such as LANG and LC_*) to compute
	// TODO a name in our locale namespace
	// TODO (see .../src/wp/ap/xp/ap_*_Languages.h)

        // make a copy of the current locale so we can set it back
	char *old_locale = g_strdup(setlocale(LC_ALL, NULL));

	// this will set our current locale information
	// according to the user's env variables
	setlocale(LC_ALL, "");

	// locale categories:
	// LC_COLLATE - collation of strings (functions strcoll and strxfrm)
	// LC_CTYPE - classification and conversion of characters
	// LC_MONETARY - formatting monetary values
	// LC_NUMERIC - formatting numeric values that are not monetary
	// LC_TIME - formatting date and time values
	// LC_MESSAGES - language of messages and look of affirmative/negative answer

	// now, which of the categories should we use?
	// we used to use LC_CTYPE, but decided that LC_MESSAGES was a better idea
	// (most likely, all of LC_* are the same)
	
	const char * szNewLang = "en-US"; // default to US English
#if defined (LC_MESSAGES)
	char * lc_ctype = g_strdup(setlocale(LC_MESSAGES, NULL));
#else
	char * lc_ctype = g_strdup(setlocale(LC_CTYPE, NULL));
#endif
	// locale categories seem to always look like this:
	// two letter for language (lowcase) _ two letter country code (upcase)
	// ie. en_US, es_ES, pt_PT
	// which goes to the Abiword format:
	// en-US, es-ES, pt-PT

	// we'll try this quick conversion
	if (lc_ctype != NULL && strlen(lc_ctype) >= 5) 
	{
		lc_ctype[2] = '-';
		char* dot = strrchr(lc_ctype,'.');

		/*
                  remove charset field. It's a right thing since expat
  		  already converts data in stringset from ANY encoding to
		  current one (if iconv knows this encoding).
		 */
		if (dot)
			*dot = '\0'; 
		szNewLang = lc_ctype;	
	}

	UT_DEBUGMSG(("Prefs: Using LOCALE info from environment [%s]\n",szNewLang));
	m_builtinScheme->setValue((gchar*)AP_PREF_KEY_StringSet,
				  (gchar*)szNewLang);

	// g_free the language id, if it was allocated
	if (lc_ctype != NULL) g_free(lc_ctype);

	// change back to the previous locale setting
	// although, we might want to leave it in the user's preferred locale?
	if (old_locale != NULL) {
	   setlocale(LC_ALL, old_locale);
	   g_free(old_locale);
	}
#endif
	return;
}
