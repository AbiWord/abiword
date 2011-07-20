/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef IE_EXP_HTML_UTIL_H
#define IE_EXP_HTML_UTIL_H

#include <ut_string_class.h>
#include <ut_types.h>

#define MYEOL "\n"

extern const char * s_prop_list[];
extern const UT_uint32 s_PropListLen;
extern const char * s_DTD_XHTML_AWML;
extern const char * s_DTD_XHTML;
extern const char * s_DTD_HTML4;
extern const char * s_Delimiter;
extern const char * s_HeaderCompact;
extern bool m_bSecondPass;
extern bool m_bInAFENote;
extern bool m_bInAnnotation;
extern UT_UTF8String sMathSVGScript;

extern const char * s_Header[2];

UT_UTF8String s_string_to_url (const UT_String & str);
UT_UTF8String s_string_to_url (const UT_UTF8String & str);
bool is_CSS (const char * prop_name, const char ** prop_default = 0);
char * s_removeWhiteSpace (const char * text, UT_UTF8String & utf8str,
								  bool bLowerCase = true);



#endif