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
#include "ap_QNXPrefs.h"

/*****************************************************************/

AP_QNXPrefs::AP_QNXPrefs(XAP_App * pApp)
	: AP_Prefs(pApp)
{
}

const char * AP_QNXPrefs::getPrefsPathname(void) const
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

void AP_QNXPrefs::overlayEnvironmentPrefs(void)
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
	char *old_locale = UT_strdup(setlocale(LC_ALL, NULL));

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
	// how about LC_CTYPE?
	// (they are probably all the same, anyway)
	
	const char * szNewLang = "EnUS"; // default to US English
	char * lc_ctype = UT_strdup(setlocale(LC_CTYPE, NULL));

	// locale categories seem to always look like this:
	// two letter for language (lowcase) _ two letter country code (upcase)
        // ie. en_US, es_ES, pt_PT
	// which goes to the Abiword format:
	// EnUS, EsES, PtPT

	// we'll try this quick conversion
	if (lc_ctype != NULL && strlen(lc_ctype) >= 5) {
	   lc_ctype[0] = toupper(lc_ctype[0]);
	   lc_ctype[2] = lc_ctype[3];
	   lc_ctype[3] = lc_ctype[4];
	   lc_ctype[4] = 0;
	   szNewLang = lc_ctype;
	}

	UT_DEBUGMSG(("Prefs: Using LOCALE info from environment [%s]\n",szNewLang));
	m_builtinScheme->setValue(AP_PREF_KEY_MenuLabelSet,szNewLang);
	m_builtinScheme->setValue(AP_PREF_KEY_ToolbarLabelSet,szNewLang);
	m_builtinScheme->setValue(AP_PREF_KEY_StringSet,szNewLang);

	// free the language id, if it was allocated
	if (lc_ctype != NULL) free(lc_ctype);

	// change back to the previous locale setting
	// although, we might want to leave it in the user's preferred locale?
	if (old_locale != NULL) {
	   setlocale(LC_ALL, old_locale);
	   free(old_locale);
	}
#endif
	return;
}
