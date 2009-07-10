/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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


#include <string.h>

#include "ut_std_string.h"

#if 0
std::string & UT_escapeXML(std::string &s)
{
    gsize incr = 0;

	const char * ptr = s.c_str();
	while (*ptr) {
        if ((*ptr == '<') || (*ptr == '>')) {
            incr += 3;
        }
        else if (*ptr == '&') {
            incr += 4;
        }
        else if (*ptr == '"') {
            incr += 5;
        }
        ptr++;
    }

    gsize slice_size = s.size() + incr + 1;
    char * dest = (char *)g_slice_alloc(slice_size);
    char * current = dest;

	ptr = s.c_str();
	while (*ptr)
    {
        if (*ptr == '<')
        {
            memcpy(dest, "&lt;", 4);
            current += 4;
        }
        else if (*ptr == '>')
        {
            memcpy(dest, "&gt;", 4);
            current += 4;
        }
        else if (*ptr == '&')
        {
            memcpy(dest, "&amp;", 5);
            current += 5;
        }
        else if (*ptr == '"')
        {
            memcpy(dest, "&quot;", 6);
            current += 6;
        }
        ptr++;
    }
    *dest = 0;
    s = dest;
    g_slice_free1(slice_size, dest);
    return s;
}
#endif

 
std::string& UT_std_string_vprintf (std::string & inStr, const char *format,
                                    va_list      args1)
{
    char *buffer = g_strdup_vprintf(format, args1);
    inStr = buffer;
    g_free(buffer);

    return inStr;
}


std::string UT_std_string_sprintf(const char * inFormat, ...)
{
    std::string outStr;

    va_list args;
    va_start (args, inFormat);
    UT_std_string_vprintf (outStr, inFormat, args);
    va_end (args);

    return outStr;
}

