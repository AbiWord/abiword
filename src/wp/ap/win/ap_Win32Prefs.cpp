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

#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include "ut_debugmsg.h"
#include "ap_Win32Prefs.h"

/*****************************************************************/

AP_Win32Prefs::AP_Win32Prefs(XAP_App * pApp)
	: AP_Prefs(pApp)
{
}

static void s_getPrefsDirectory(char * pbuf, UT_uint32 bufLen)
{
	DWORD len, len1, len2;

	// On NT, USERPROFILE seems to be set to the directory containing per-user
	// information.  we'll try that first.
	
	len = GetEnvironmentVariable("USERPROFILE",pbuf,bufLen);
	if (len)
	{
		UT_DEBUGMSG(("Getting preferences directory from USERPROFILE [%s].\n",pbuf));
		return;
	}

	// If that doesn't work, look for HOMEDRIVE and HOMEPATH.  HOMEPATH
	// is mentioned in the GetWindowsDirectory() documentation at least.
	// These may be set if the SysAdmin did so in the Admin tool....
	
	len1 = GetEnvironmentVariable("HOMEDRIVE",pbuf,bufLen);
	len2 = GetEnvironmentVariable("HOMEPATH",&pbuf[len1],bufLen-len1);
	if (len1 && len2)
	{
		UT_DEBUGMSG(("Getting preferences directory from HOMEDRIVE and HOMEPATH [%s].\n",pbuf));
		return;
	}

	// If that doesn't work, let's just stick it in the WINDOWS directory.

	len = GetWindowsDirectory(pbuf,bufLen);
	if (len)
	{
		UT_DEBUGMSG(("Getting preferences directory from GetWindowsDirectory() [%s].\n",pbuf));
		return;
	}

	// If that doesn't work, stick it in "C:\"...

	strcpy(pbuf,"C:\\");
	
	return;
}
	
const char * AP_Win32Prefs::getPrefsPathname(void) const
{
	/* return a pointer to a static buffer */
	
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

	static char buf[PATH_MAX];
	memset(buf,0,sizeof(buf));

	char * szFile = "abiword.profile";
	int lenFile = strlen(szFile);
	
	s_getPrefsDirectory(buf,PATH_MAX-lenFile-2);

	if (buf[strlen(buf)-1] != '\\')
		strcat(buf,"\\");

	strcat(buf,szFile);

	UT_DEBUGMSG(("Constructed preference file name [%s]\n",buf));
	
	return buf;
}
