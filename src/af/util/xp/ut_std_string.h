/* AbiSource Program Utilities
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
 * Copyright (C) 2011 Ben Martin
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


/** replacement for UT_UTF8String::escapeXML 
 *  escapes '<', '>', '\"' and '&' in the current string
 */
ABI_EXPORT std::string UT_escapeXML(const std::string &);

ABI_EXPORT std::string& UT_std_string_vprintf (std::string & inStr, 
                                               const char *format,
                                               va_list      args1)
    ABI_PRINTF_FORMAT(2,0);
 
ABI_EXPORT std::string UT_std_string_sprintf(const char * inFormat, ...)
    ABI_PRINTF_FORMAT(1,2);

/**
 * true if fullstring starts with exactly prefix.
 */
ABI_EXPORT bool starts_with( const std::string& fullstring, const std::string& prefix );
ABI_EXPORT bool ends_with(   const std::string& fullstring, const std::string& ending );
ABI_EXPORT std::string replace_all( const std::string& s, char oldc, char newc );
ABI_EXPORT std::string replace_all( const std::string& s,
                                    const std::string& olds,
                                    const std::string& news );

ABI_EXPORT std::string UT_XML_cloneNoAmpersands( const std::string& szSource );



/*!
 * Some functions to add/subtract and extract std::string properties from a std::string of properties.
 */
ABI_EXPORT std::string UT_std_string_getPropVal(const std::string & sPropertyString, const std::string & sProp);
ABI_EXPORT void UT_std_string_removeProperty(std::string & sPropertyString, const std::string & sProp);
ABI_EXPORT void UT_std_string_setProperty(std::string & sPropertyString, const std::string &sProp, const std::string & sVal);



#endif

