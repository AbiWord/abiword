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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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
	struct stat statbuf;
	int laststat;

	if(*progName == '/')
	{
		laststat = stat(progName, &statbuf);
		if(laststat == 0 && (S_ISREG(statbuf.st_mode) || S_ISLNK(statbuf.st_mode)))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	UT_String envpath = getenv("PATH");
	UT_String* path;

	UT_GenericVector<UT_String*> * utvPath = simpleSplit(envpath, ':');
	if (!utvPath)
	  return false;

	bool found = false;
	for(UT_uint32 i = 0; i < utvPath->getItemCount(); i++)
	{
		path = utvPath->getNthItem(i);
		char * path2 = UT_catPathname(path->c_str(), progName);
		laststat = stat(path2, &statbuf);
		FREEP(path2);

		if(laststat == 0 && (S_ISREG(statbuf.st_mode) || S_ISLNK(statbuf.st_mode)))
		{
			found = true;
			break;
		}
	}

	UT_VECTOR_PURGEALL(UT_String*, (*utvPath));
	DELETEP(utvPath);
	return found;
}
