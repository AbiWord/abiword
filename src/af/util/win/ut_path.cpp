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
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string_class.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#error This is Win32 include file!
#endif
#include <windows.h>

/*!	This function takes a char* representing a path to a file and returns
	the pointer to the string which represents the base portion of the path.
	
	For example, if path = "/home/foo/bar.ext", then this function returns
	a char* pointing to "bar.ext".
 */

const char* UT_basename(const char* path)
{
	size_t len = strlen(path);
	const char* str = &path[len];

	while(len > 0 && path[len-1] != '/' && path[len-1] != '\\')
		str = &path[--len];

	return str;
}

bool UT_directoryExists(const char* dir)
{
	struct _stat buf;

#ifdef _WIN32
	WCHAR wFilename[MAX_PATH];
	MultiByteToWideChar(CP_UTF8,0,dir,-1,wFilename,MAX_PATH);
	if( _wstat( wFilename , &buf ) != -1 ) 
#else
	if( _stat( dir , &buf ) != -1 ) 
#endif
	{
		return ( buf.st_mode & _S_IFDIR ) != 0;
	}
	return false;
}

bool UT_isRegularFile(const char* filename)
{
	struct _stat buf;

#ifdef _WIN32
	WCHAR wFilename[MAX_PATH];
	MultiByteToWideChar(CP_UTF8,0,filename,-1,wFilename,MAX_PATH);
	if( _wstat( wFilename , &buf ) != -1 ) 
#else
	if( _stat( filename , &buf ) != -1 ) 
#endif
	{
		UT_DEBUGMSG(("UT_isRegularFile(%s) { _stat(...) succeeded, returning ( st_mode<%X> & _S_IFREG<%X> )!= 0  <%X>\n", filename, buf.st_mode, _S_IFREG, (( buf.st_mode & _S_IFREG ) != 0) ));
		return ( buf.st_mode & _S_IFREG ) != 0;
	}
	UT_DEBUGMSG(("UT_isRegularFile(%s) { _stat(...) failed, returning false. }\n", filename));
	return false;
}

size_t UT_fileSize(const char * filename)
{
	struct _stat buf;
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	if( _stat( filename , &buf ) != -1 ) 
	{
		return buf.st_size;
	}

	return 0;
}


time_t UT_mTime(const char* path)
{
	struct _stat buf;
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	if( _stat( path , &buf ) != -1 ) 
	{
		return buf.st_mtime;
	}

	UT_ASSERT_HARMLESS(1);
	return((time_t)-1);
}

/*!
    check that the given filename is legal and remove any illegal characters
	\param filename [in/out] the suggested file name
    \return false if filename is left unchanged, true otherwise
 */
bool UT_legalizeFileName(std::string &filename)
{
	UT_UTF8String sFilename(filename);
	UT_UTF8String sTmp;
	bool bRet = false;
	
 	UT_UTF8Stringbuf::UTF8Iterator iter = sFilename.getIterator ();
 	if (iter.start())
 	{
		const char * pUTF = iter.current();
 		while (pUTF && *pUTF)
 		{
			UT_UCS4Char c = UT_UTF8Stringbuf::charCode(pUTF);
			if(c < ' ')
			{
				bRet = true;
			}
			else
			{
				switch(c)
				{
					case ':':
					case '/':
					case '|':
					case '<':
					case '>':
					case '\\':
					case '^':
					case '=':
					case '?':
					case '\"':
					case '[':
					case ']':
					case ';':
					case '*':
						bRet = true;
						break;
						
					default: sTmp += c;
				}
			}
			
 			pUTF = iter.advance (); // or ++iter;
 		}
 	}

	if(bRet)
	{
		sFilename = sTmp;
		filename = sFilename.utf8_str();
	}
	
	return bRet;
}

