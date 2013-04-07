/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
 

#include "ut_files.h"
#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "ut_string_class.h"

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
