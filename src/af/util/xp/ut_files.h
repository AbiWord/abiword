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

#ifndef UT_FILES_H
#define UT_FILES_H

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

bool progExists(const char* progName);

/*!
  Creates a directory if the specified one does not yet exist.
  /param A character string representing the to-be-created directory. 
  /return True, if the directory already existed, or was successfully
	created.  False, if the input path was already a file, not a
	directory, or if the directory was unable to be created.
  /todo Do domething with error status if the directory couldn't be
	created? 
*/
bool UT_createDirectoryIfNecessary(const char * szDir, bool publicdir = false);

#endif /* UT_FILES_H */
