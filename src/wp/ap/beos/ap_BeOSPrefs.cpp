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

#include "stdlib.h"
#include "string.h"
#include "locale.h"
#include "ctype.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "ap_BeOSPrefs.h"

/*****************************************************************/

AP_BeOSPrefs::AP_BeOSPrefs(XAP_App * pApp)
	: AP_Prefs(pApp)
{
}

const char * AP_BeOSPrefs::_getPrefsPathname(void) const
{
	/* return a pointer to a static buffer */

	const char * szDirectory = m_pApp->getUserPrivateDirectory();
	char * szFile = "AbiWord.Profile";

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

	static char buf[PATH_MAX];
	memset(buf,0,sizeof(buf));

	if (strlen(szDirectory) + strlen(szFile) + 2 >= PATH_MAX)
		return NULL;

	strcpy(buf,szDirectory);
	int len = strlen(buf);
	if ( (len == 0) || (buf[len-1] != '/') )
		strcat(buf,"/");
	strcat(buf,szFile);

	return buf;
}

void AP_BeOSPrefs::overlayEnvironmentPrefs(void)
{
	// TODO steal the appropriate code from the unix version
	// TODO after it is finished.
	printf("no set locale\n");
	if (!m_bUseEnvLocale)
		return;							// nothing to do...

#if 1
	// TODO use various POSIX env variables
	// TODO (such as LANG and LC_*) to compute
	// TODO a name in our locale namespace
	// TODO (see .../src/wp/ap/xp/ap_*_Languages.h)

        // make a copy of the current locale so we can set it back
	char *old_locale = UT_strdup(setlocale(LC_ALL, NULL));

	// this will set our current locale information
	// according to the user's env variables

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
//#if defined (LC_MESSAGES)
//	char * lc_ctype = UT_strdup(setlocale(LC_MESSAGES, NULL));
//#else
	printf("getenv\n");
	char * lc_ctype = getenv("LANG");
	if (lc_ctype) lc_ctype = UT_strdup(lc_ctype);
	else lc_ctype = UT_strdup("en_US");
	printf(lc_ctype);
	printf("\n");
//#endif
	// locale categories seem to always look like this:
	// two letter for language (lowcase) _ two letter country code (upcase)
	// ie. en_US, es_ES, pt_PT
	// which goes to the Abiword format:
	// en-US, es-ES, pt-PT

	// we'll try this quick conversion
	if (lc_ctype != NULL && strlen(lc_ctype) >= 5) {
	   lc_ctype[2] = '-';
		char* modifier = strrchr(lc_ctype,'@');
 		/*
                   remove modifier field. It's a right thing since expat
 		  already converts data in stringset from ANY encoding to
 		  current one (if iconv knows this encoding).
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
		szNewLang = lc_ctype;
	}

	UT_DEBUGMSG(("Prefs: Using LOCALE info from environment [%s]\n",szNewLang));	

	m_builtinScheme->setValue((XML_Char*)AP_PREF_KEY_StringSet,
				  (XML_Char*)szNewLang);

	// free the language id, if it was allocated
	if (lc_ctype != NULL) free(lc_ctype);

	// change back to the previous locale setting
	// although, we might want to leave it in the user's preferred locale?
	if (old_locale != NULL) {
	   setlocale(LC_ALL, old_locale);
	   free(old_locale);
	}
#endif

}
