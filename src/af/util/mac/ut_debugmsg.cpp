/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2000 Hubert Figuière <hfiguiere@teaser.fr>
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

#include <MacTypes.h>

#include "ut_debugmsg.h"

void _UT_OutputMessage(char *s, ...)
{
	va_list marker;

	va_start(marker, s);
#ifndef CARBON_ON_MACH_O
	Str255 sBuf;
	vsprintf(&((char*)sBuf)[1], s, marker);

	sBuf[0] = 1;
	sBuf[0] = strlen ((char *)sBuf) - 1;

	DebugStr(sBuf);
#else
	char sBuf[1024];
	vsprintf(sBuf, s, marker);

        fprintf(stderr,"DEBUG: %s",sBuf);
#endif
}
