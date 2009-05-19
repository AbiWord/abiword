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
 
#ifndef UT_PATH_H
#define UT_PATH_H

#include <glib.h>
#include <time.h>

#if defined(MAXPATHLEN) && !defined(PATH_MAX)
#define PATH_MAX MAXPATHLEN
#else
#include <limits.h>
#endif

#if !defined(PATH_MAX)
#error Huh, neither MAXPATHLEN nor PATH_MAX available, fix for this platform needed.
#endif

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include "ut_string_class.h"
#include "ut_go_file.h"
#include "ut_rand.h"

class UT_UTF8String;

// todo: deprecate me
ABI_EXPORT const char* UT_basename(const char* path);

static inline UT_UTF8String UT_go_basename(const char* uri)
{
  UT_UTF8String _base_name;
  char *base_name = UT_go_basename_from_uri(uri);
  if(base_name) {
    _base_name = base_name;
    g_free(base_name);
  }
  return _base_name;
}

// useful for win32, since we can't use the glib functions that return
// a file descriptor in msvc (not unless someone adds a g_close() function
// to glib
static inline std::string UT_createTmpFile(const std::string& prefix, const std::string& extenstion)
{
	const gchar *filename = g_build_filename (g_get_tmp_dir (), prefix.c_str(), NULL);
    UT_return_val_if_fail(filename, false);

    std::string sName = filename;
    FREEP(filename);

	UT_UTF8String rand = UT_UTF8String_sprintf("%X", UT_rand() * 0xFFFFFF);
    sName += rand.utf8_str();
    sName += extenstion;

    FILE* f = fopen (sName.c_str(), "w+b");
	if (!f)
		return false;

    fclose(f);
	return sName;
}

ABI_EXPORT bool UT_directoryExists(const char* dir);

ABI_EXPORT bool UT_isRegularFile(const char* filename);

ABI_EXPORT size_t UT_fileSize(const char * filename);

ABI_EXPORT time_t UT_mTime(const char* path);

ABI_EXPORT bool UT_legalizeFileName(UT_UTF8String &filename);
#endif /* UT_PATH_H */
