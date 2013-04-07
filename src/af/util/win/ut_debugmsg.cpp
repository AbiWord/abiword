/* AbiSource Program Utilities
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
 


#include <windows.h>

#include <stdio.h>
#include <stdarg.h>

#include "ut_debugmsg.h"

// TODO This is Win32-specific, and should not be.

void _UT_OutputMessage(const char *s, ...)
{
	UT_DEBUG_ONLY_ARG(s);
#ifdef DEBUG
	char sBuf[1024];
	va_list marker;

	va_start(marker, s);

#if 0
	vsprintf(sBuf, s, marker);
#else
	// MPritchett or others: REVERT THIS IF NECESSARY
	_vsnprintf(sBuf, sizeof(sBuf), s, marker);
#endif

	WCHAR wBuf[1024];
	MultiByteToWideChar(CP_UTF8,0,sBuf,-1,wBuf,1024);
	OutputDebugStringW(wBuf);
#endif
}


#ifdef _UT_WarningMessage
#undef _UT_WarningMessage
#endif
/*
 * Similar to UT_DEBUGMSG, except exists even in production (non-debug) builds
 */
void _UT_WarningMessage(const char *s, ...)
{
	char sBuf[1024];
	va_list marker;

	va_start(marker, s);
	_vsnprintf(sBuf, sizeof(sBuf), s, marker);

	MessageBoxA(NULL, sBuf, "Warning", MB_OK | MB_ICONWARNING); //!TODO Using ANSI function
}
