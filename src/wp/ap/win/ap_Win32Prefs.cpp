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

#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include "ut_types.h"

#include "ut_units.h"

#include "ut_debugmsg.h"
#include "ut_Win32Locale.h"
#include "ap_Win32Prefs.h"


/*****************************************************************/

AP_Win32Prefs::AP_Win32Prefs(XAP_App * pApp)
	: AP_Prefs(pApp)
{
}

bool AP_Win32Prefs::loadBuiltinPrefs(void)

{

	char  szLocaleInfo[64];

	// Call base function
	bool ret = AP_Prefs::loadBuiltinPrefs();

	// Add information from Win32 system and user setup
	if( GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_IMEASURE, szLocaleInfo, sizeof( szLocaleInfo ) / sizeof( szLocaleInfo[0] ) ) )
	{
		m_builtinScheme->setValue( AP_PREF_KEY_RulerUnits, UT_dimensionName( szLocaleInfo[0] == '0' ? DIM_CM : DIM_IN ) );
	}

	if (UT_getISO639Language(szLocaleInfo))
	{
		if (UT_getISO3166Country(&szLocaleInfo[3]))
			szLocaleInfo[2] = '-';

		UT_DEBUGMSG(("Prefs: Using LOCALE info from environment [%s]\n", szLocaleInfo));

		m_builtinScheme->setValue( AP_PREF_KEY_MenuLabelSet, szLocaleInfo );
		m_builtinScheme->setValue( AP_PREF_KEY_ToolbarLabelSet, szLocaleInfo );
		m_builtinScheme->setValue( AP_PREF_KEY_StringSet, szLocaleInfo );
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	return ret;
}

const char * AP_Win32Prefs::getPrefsPathname(void) const
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
	if ( (len == 0) || (buf[len-1] != '\\') )
		strcat(buf,"\\");
	strcat(buf,szFile);

	UT_DEBUGMSG(("Constructed preference file name [%s]\n",buf));
	
	return buf;
}

void AP_Win32Prefs::overlayEnvironmentPrefs(void)
{
	// TODO steal the appropriate code from the unix version
	// TODO after it is finished.
}


