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

#ifndef NDEBUG				// see assert() manpage

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ut_assert.h"
#include "ut_BeOSAssert.h"

void UT_BeOSAssertMsg(const char * szMsg, const char * szFile, int iLine)
{
	static int count = 0;
	
	printf("\n");
	printf("**** (%d) Assert ****\n", ++count);
	printf("**** (%d) %s at %s:%d ****\n", count,szMsg,szFile,iLine);
	while (1)
	{
		printf("**** (%d) Continue ? (y/n) [y] : ", count);
		fflush(stdout);

		char buf[10];
		memset(buf,0,10);

		fgets(buf,10,stdin);

		switch (buf[0])
		{
		case '\0':
		case '\n':
		case 'y':
		case 'Y':
			return;		// continue the application

		case 'n':
		case 'N':
			abort();	// kill the application
			return;
		default:
			break;		// ?? ask them again
		}
	}
}

#endif // NDEBUG
