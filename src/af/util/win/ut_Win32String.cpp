/* AbiSource Program Utilities
 * Copyright (C) 1998,1999 AbiSource, Inc.
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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <fribidi.h>

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_string.h"

char * UT_catPathname(const char * szPath, const char * szFile)
{
	UT_ASSERT((szPath) && (*szPath));
	UT_ASSERT((szFile) && (*szFile));
	
	char * szPathname = (char *)UT_calloc(sizeof(char),strlen(szPath)+strlen(szFile)+2);
	UT_ASSERT(szPathname);
	
	sprintf(szPathname,"%s%s%s",
			szPath,
			((szPath[strlen(szPath)-1]=='\\') ? "" : "\\"),
			szFile);

	return szPathname;
}

char * UT_tmpnam(char * pszBase)
{
	char szTempPath[ 1024 ];

	UT_ASSERT(pszBase);

	// Get a path to a temp directory...
	GetTempPathA( 1024, szTempPath ); //!TODO Using ANSI function

	// Then get a temp file name in the temp directory we just got.
	GetTempFileNameA( szTempPath, "abi", 0, pszBase ); //!TODO Using ANSI function

	return pszBase;
}

void UT_unlink (const char * base)
{
	// note: both remove & unlink are available in VC5, and both
	// should perform identically, but remove was choosen since it is ANSI C (stdio.h)
	// unlink (base);
	remove(base);
}

UT_BidiCharType UT_bidiGetCharType(UT_UCS4Char c)
{
	return fribidi_get_type(c);
}

bool UT_bidiReorderString(const UT_UCS4Char * pStrIn, UT_uint32 len, UT_BidiCharType baseDir,
						  UT_UCS4Char * pStrOut)
{
	// if this assert fails, we have a serious problem ...
	UT_ASSERT_HARMLESS( sizeof(UT_UCS4Char) == sizeof(FriBidiChar) );
	return (0 != fribidi_log2vis ((FriBidiChar *)pStrIn, len, &baseDir, (FriBidiChar*)pStrOut, NULL, NULL, NULL));
}

bool UT_bidiMapLog2Vis(const UT_UCS4Char * pStrIn, UT_uint32 len, UT_BidiCharType baseDir,
					   UT_uint32 *pL2V, UT_uint32 * pV2L, UT_Byte * pEmbed)
{
	// if this assert fails, we have a serious problem ...
	UT_ASSERT_HARMLESS( sizeof(UT_UCS4Char) == sizeof(FriBidiChar) );
	return (0 != fribidi_log2vis ((FriBidiChar *)pStrIn, len, &baseDir,
								  NULL, (FriBidiStrIndex*)pL2V, (FriBidiStrIndex*)pV2L, (FriBidiLevel*)pEmbed));
}


bool UT_bidiGetMirrorChar(UT_UCS4Char c, UT_UCS4Char &mc)
{
	return (0 != fribidi_get_mirror_char(c, (FriBidiChar*)&mc));
}
