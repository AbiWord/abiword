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
#include "ap_MacPrefs.h"
#include "FullPath.h"

/*****************************************************************/

AP_MacPrefs::AP_MacPrefs(XAP_App * pApp)
	: AP_Prefs(pApp)
{
}

const char * AP_MacPrefs::getPrefsPathname(void) const
{
	/* return a pointer to a static buffer */

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

	static char buf[PATH_MAX];
	memset(buf,0,sizeof(buf));
	
	short	foundVRefNum, pathLen;
	long	foundDirID;
	OSErr err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
							&foundVRefNum, &foundDirID);
	
	Handle bufHdl = NewHandle(PATH_MAX);
	GetFullPath(foundVRefNum, foundDirID, NULL, &pathLen, &bufHdl);
	strncpy(buf, *bufHdl, pathLen);
	DisposeHandle(bufHdl);
	
	char * szFile = "AbiWord";

	if(pathLen >= PATH_MAX || pathLen == 0)
		return NULL;
	
	if(pathLen && (buf[pathLen-1] == ':'))
		;
	else
		strcat(buf,":");
	strcat(buf,szFile);

	return buf;
}

void AP_MacPrefs::overlayEnvironmentPrefs(void)
{
	// TODO steal the appropriate code from the unix version
	// TODO after it is finished.
}
