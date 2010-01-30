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

#ifndef NDEBUG							// see assert() manpage

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include "ut_assert.h"
#include "ut_unixAssert.h"

static volatile sig_atomic_t trap_reached = 0;

static void trap_handler(int signal)
{
	trap_reached = 1;
}

/* returns false if no debugger handled the signal */
static bool break_into_debugger()
{
	/* some SIGTRAP magic for convenient debugging */

	trap_reached = 0;

	struct sigaction act;
	act.sa_handler = trap_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	struct sigaction oldact;

	sigaction(SIGTRAP, &act, &oldact);
	kill(0, SIGTRAP);
	sigaction(SIGTRAP, &oldact, NULL);

	return !trap_reached;
}

int UT_UnixAssertMsg(const char * szMsg, const char * szFile, int iLine)
{
	static int count = 0;

	printf("\n");
	printf("**** (%d) Assert ****\n", ++count);
	printf("**** (%d) %s at %s:%d ****\n", count,szMsg,szFile,iLine);

	while (1)
	{
		printf("**** (%d) Continue? (y)es/(n)o/(i)gnore/(b)reak [y] : ", count);
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
			return 1;					// continue the application

		case 'n':
		case 'N':
			abort();					// kill the application
			return 0;

		case 'i':
		case 'I':
			return -1;

		case 'b':
		case 'B':
			if (break_into_debugger())
				return 1;
			else
				printf("**** No debugger attached\n");

		default:
			break;						// ?? ask them again
		}
	}
}

#endif // NDEBUG
