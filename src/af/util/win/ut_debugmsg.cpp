 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiSource Utilities.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#include <windows.h>

#include <stdio.h>
#include <stdarg.h>

#include "ut_debugmsg.h"

// TODO aaaaagh!  This is Win32-specific

void _UT_OutputMessage(char *s, ...)
{
	char sBuf[1024];
	va_list marker;

	va_start(marker, s);

	vsprintf(sBuf, s, marker);

	OutputDebugString(sBuf);
}
