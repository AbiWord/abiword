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

const char * AP_QNXPrefs::_getPrefsPathname(void) const
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

	char szNewLang[10] = "en-US"; // default to US English
	char *ABLANG;
	char *val;
	// locale categories seem to always look like this:
	// two letter for language (lowcase) _ two letter country code (upcase)
	// ie. en_US, es_ES, pt_PT
	// which goes to the Abiword format:
	// en-US, es-ES, pt-PT
if((ABLANG=getenv("ABLANG")))
	{
			val=strsep(&ABLANG,"_");
			if(val && ABLANG)
				{
					snprintf((char*)&szNewLang,6,"%s-%s",val,ABLANG);
				}
	}

	UT_DEBUGMSG(("Prefs: Using LOCALE info from environment [%s]\n",szNewLang));
	m_builtinScheme->setValue(AP_PREF_KEY_StringSet,szNewLang);

	return;
}
