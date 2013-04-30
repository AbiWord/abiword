/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (c) 2013
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ut_files.h"
#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_types.h"

/*!
	If progName is a fully qualified path (i.e. it begins with '/'),
	this function looks for the program with this path, returning true
	if it exists.  Otherwise, progExists searchs the system PATH for the
	program progName, and returns true if it exists (i.e. is a regular
	file or symlink).

	Ex: if(progExists("vi") { do_stuff(); }
 */

bool progExists(const char* progName)
{
  char *prog;

  prog = g_find_program_in_path (progName);
  if (!prog)
    return false;

  g_free (prog);
  return true;
}

/*!
  Creates a directory if the specified one does not yet exist.
  /param A character string representing the to-be-created directory.
  /return True, if the directory already existed, or was successfully
	created.  False, if the input path was already a file, not a
	directory, or if the directory was unable to be created.
  /todo Do domething with error status if the directory couldn't be
	created?
*/
bool UT_createDirectoryIfNecessary(const char * szDir, bool publicdir)
{
    struct stat statbuf;

    if (stat(szDir,&statbuf) == 0)		// if it exists
    {
		if (S_ISDIR(statbuf.st_mode))	// and is a directory
			return true;

		UT_DEBUGMSG(("Pathname [%s] is not a directory.\n", szDir));
		return false;
    }
#ifdef LOGFILE
	fprintf(getlogfile(),"New Directory created \n");
#endif

	bool success = true;
    mode_t old_mask = umask (0);
    if (mkdir (szDir, publicdir ? 0775 : 0700))
	{
		UT_DEBUGMSG(("Could not create Directory [%s].\n", szDir));
		success = false;
	}
	umask (old_mask);
	return success;
}

