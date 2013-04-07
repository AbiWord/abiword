/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include <ctype.h>
#include "ut_locale.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_App.h"
#include "ap_UnixPrefs.h"
#include "ut_string_class.h"

/*****************************************************************/

AP_UnixPrefs::AP_UnixPrefs()
	: AP_Prefs()
{
}

const char * AP_UnixPrefs::_getPrefsPathname(void) const
{
	/* return a pointer to a static buffer */
	static UT_String buf;

	if(!buf.empty())
	  return buf.c_str();

	const char * szDirectory = XAP_App::getApp()->getUserPrivateDirectory();
	const char * szFile = "profile";

	buf = szDirectory;
	if (!buf.size() || szDirectory[buf.size()-1] != '/')
	  buf += "/";
	buf += szFile;

	// migration / legacy
	XAP_App::getApp()->migrate("/AbiWord.Profile", szFile, buf.c_str());  

	return buf.c_str();
}

void AP_UnixPrefs::overlayEnvironmentPrefs(void)
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

	const char * szNewLang = "en-US"; // default to US English
#if defined (LC_MESSAGES) && defined (UNDEF) // raphael
// #if defined (LC_MESSAGES)
	char * lc_ctype = g_strdup(setlocale(LC_MESSAGES, NULL));
#else
	char * lc_ctype = getenv("LC_ALL");
	if (!lc_ctype || !*lc_ctype) {
		// TODO: implement $LANGUAGE parsing here
		lc_ctype = getenv("LC_MESSAGES");
		if (!lc_ctype || !*lc_ctype) {
			lc_ctype = getenv("LANG");
		}
	}
	if (lc_ctype) lc_ctype = g_strdup(lc_ctype);
	else lc_ctype = g_strdup("en_US");
#endif
	// locale categories seem to always look like this:
	// two letter for language (lowcase) _ two letter country code (upcase)
	// ie. en_US, es_ES, pt_PT
	// which goes to the Abiword format:
	// en-US, es-ES, pt-PT

	// we'll try this quick conversion
	if (lc_ctype != NULL && strlen(lc_ctype) >= 5) 
	{
		char * uscore = strchr(lc_ctype, '_'); 
		if (uscore)
			*uscore = '-';

		char* modifier = strrchr(lc_ctype,'@');
		/*
		  Temporarily remove modifier field to strip charset.
		*/
		if (modifier)
		  *modifier = '\0'; 

		char* dot = strrchr(lc_ctype,'.');
		/*
                  remove charset field. It's a right thing since expat
  		  already converts data in stringset from ANY encoding to
		  current one (if iconv knows this encoding).
		 */
		if (dot)
			*dot = '\0'; 

		if (modifier) {
			// put modifier (if present) back
			// memmove for overlapping regions caveat
			char * dest = &lc_ctype[strlen(lc_ctype)];
			*modifier = '@';
			memmove(dest,modifier,strlen(modifier)+1);
		}
		
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
