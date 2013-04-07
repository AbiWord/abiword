/* ut_process.h
 *
 * Copyright (C) 2002 AbiWord developers (see CREDITS.TXT for list).
 * Initially written by Kenneth J. Davis, I disclaim any copyright.
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

#ifndef UT_PROCESS_H
#define UT_PROCESS_H


/* include standard headers */
#include <stddef.h>			/* NULL and other common definitions */
#include <stdlib.h>
#include <stdio.h>

/* Note: sys/types.h should be included before sys/stat.h */
#include <sys/types.h>
#include <sys/stat.h>


/* platform specific code */
#define WIN32_LEAN_AND_MEAN		/* skip some unnecessary extras     */
#include <windows.h>
typedef PROCESS_INFORMATION ProcessInfo;	/* Win32 specific structure   */


// returns true if process is still alive
bool isProcessStillAlive(ProcessInfo &pI);
// returns true if process successfully started, false otherwise
bool createChildProcess(const char *app, const char *args, ProcessInfo *pI);
// will [try to] kill the process if it is still running
void endProcess(ProcessInfo &pI);


#endif /* UT_PROCESS_H */
