/* AbiSource Program Utilities
 * Copyright (C) 1998,1999 AbiSource, Inc.
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


#ifndef __UT_STD_STRING_H__
#define __UT_STD_STRING_H__

#include <string>

#include "ut_types.h"


#if 0 // TEST this first
/** replacement for UT_UTF8String::escapeXML 
 *  escapes '<', '>', '\"' and '&' in the current string
 */
ABI_EXPORT std::string & UT_escapeXML(std::string &);
#endif

ABI_EXPORT std::string& UT_std_string_vprintf (std::string & inStr, 
                                               const char *format,
                                               va_list      args1)
    ABI_PRINTF_FORMAT(2,0);
 
ABI_EXPORT std::string UT_std_string_sprintf(const char * inFormat, ...)
    ABI_PRINTF_FORMAT(1,2);



#endif

