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


#include "ut_path.h"
#include "ut_string_class.h"
#include "ut_assert.h"

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

size_t UT_fileSize(const char * filename)
{
    struct stat buf;
    
    if (stat(filename, &buf) != -1)
    {
		return buf.st_size;
    }
    
    return 0;
}



/*!
	If the directory/file 'path' exist its mtime (modification time) is returned
*/
time_t UT_mTime(const char* path)
{
    struct stat buf;
    
    if (stat(path, &buf) != -1)
    {
        return(buf.st_mtime);
    }
    
    return((time_t)-1);
}

/*!
    check that the given filename is legal and remove any illegal characters
	\param filename [in/out] the suggested file name
    \return false if filename is left unchanged, true otherwise
 */
bool UT_legalizeFileName(std::string &filename)
{
	bool bRet = false;

	char *tmp = g_strdup(filename.c_str());
	char *ptr = tmp;

	while (*ptr) {
		if (*ptr == '/') {
			*ptr = '-';
			bRet = true;
		}
		ptr++;
	}
	
	if(bRet) {
		filename = tmp;
	}

	FREEP(tmp);

	return bRet;
}
