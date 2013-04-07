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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 


#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "ut_debugmsg.h"

void _UT_OutputMessage(const char *s, ...)
{
#ifdef DEBUG
#define DEBUG_MSG "DEBUG: "
	static bool debug_msg = true;
#endif
	va_list marker;

	va_start(marker, s);

#ifdef DEBUG
	if (debug_msg) fwrite(DEBUG_MSG, 1, strlen(DEBUG_MSG), stderr);
	debug_msg = (s && *s && s[strlen(s) - 1] == '\n');
#undef DEBUG_MSG
#endif

	vfprintf(stderr, s, marker);

	va_end(marker);
}
