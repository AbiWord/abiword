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
#include <string.h>

#include "ut_debugmsg.h"

void _UT_OutputMessage(const char *s, ...)
{
#ifdef DEBUG
#define DEBUG_MSG "DEBUG: "
	va_list marker;

	va_start(marker, s);

	fwrite(DEBUG_MSG, 1, strlen(DEBUG_MSG), stderr);
	vfprintf(stderr, s, marker);

	va_end(marker);
#undef DEBUG_MSG
#else
	UT_UNUSED(s);
#endif
}
