/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 


#include <stdio.h>
#include <stdarg.h>

#include "ut_debugmsg.h"

static FILE * s_debug = 0;

bool _UT_OutputMessage_Divert (const char * debug_filename)
{
	if (s_debug && (s_debug != stderr))
		fclose (s_debug);

	s_debug = fopen (debug_filename, "w");

	return (s_debug ? true : false);
}

void _UT_OutputMessage(const char *s, ...)
{
	char sBuf[20*1024];
	va_list marker;

	va_start(marker, s);

	vsprintf(sBuf, s, marker);

	if (!s_debug)
		s_debug = stderr;

	fprintf(s_debug,"DEBUG: %s",sBuf);

	if (s_debug != stderr)
		fflush (s_debug);
}
