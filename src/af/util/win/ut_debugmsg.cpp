
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
