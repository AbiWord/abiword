/* AbiSource Program Utilities
 * Copyright (C) 1998-2001 AbiSource, Inc.
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


#include "ut_path.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*!	This function takes a char* representing a path to a file and returns
	the pointer to the string which represents the base portion of the path.
	
	For example, if path = "/home/foo/bar.ext", then this function returns
	a char* pointing to "bar.ext".
 */

const char* UT_basename(const char* path)
{
	size_t len = strlen(path);
	const char* str = &path[len];

	while(len > 0 && path[len-1] != '/')
		str = &path[--len];

	return str;
}

/*!
	if dir is an existing directory, this function
	returns true, otherwise, it returns false
*/

bool UT_directoryExists(const char* dir)
{
    struct stat buf;
    
    if (stat(dir, &buf) != -1)
    {
		return S_ISDIR (buf.st_mode);
    }
    
    return false;
}

bool UT_isRegularFile(const char* filename)
{
    struct stat buf;
    
    if (stat(filename, &buf) != -1)
    {
		return S_ISREG (buf.st_mode);
    }
    
    return false;
}

/* Untested, but should work */
time_t UT_mTime(const char* path)
{
    struct stat buf;
    
    if (stat(path, &buf) != -1)
    {
        return(buf.st_mtime);
    }
    
    return((time_t)-1);
}
