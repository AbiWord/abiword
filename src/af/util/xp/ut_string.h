/* AbiSource Program Utilities
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
 

#ifndef UT_STRING_H
#define UT_STRING_H

#include "ut_types.h"
#include "xmlparse.h"
class UT_GrowBuf;

UT_BEGIN_EXTERN_C

UT_sint32 UT_strcmp(const char *s1, const char *s2);
UT_sint32 UT_stricmp(const char *s1, const char *s2);
UT_sint32 UT_strnicmp(const char *s1, const char *s2, int lenS1);
UT_Bool UT_cloneString(char *& rszDest, const char * szSource);
UT_Bool UT_replaceString(char *& rszDest, const char * szSource);

UT_uint32 UT_XML_strlen(const XML_Char * sz);
UT_Bool UT_XML_cloneString(XML_Char *& rszDest, const XML_Char * szSource);
UT_Bool UT_XML_cloneList(XML_Char **& rszDest, const XML_Char ** szSource);
UT_Bool UT_XML_replaceList(XML_Char **& rszDest, const XML_Char ** szSource);
UT_sint32 UT_XML_stricmp(const XML_Char * sz1, const XML_Char * sz2);
UT_sint32 UT_XML_strcmp(const XML_Char * sz1, const XML_Char * sz2);

// this function allocates (and returns a pointer to) new memory for the new string
UT_Bool UT_XML_cloneNoAmpersands(XML_Char *& rszDest, const XML_Char * szSource);

UT_uint32 UT_pointerArrayLength(void ** array);
   
// the naming convention has deviated from the above.  it's kind
// of a mutant libc/C++ naming convention.  
UT_sint32 		UT_UCS_strcmp(const UT_UCSChar* left, const UT_UCSChar* right);
UT_UCSChar * 	UT_UCS_strstr(const UT_UCSChar * phaystack, const UT_UCSChar * pneedle);
UT_UCSChar * 	UT_UCS_stristr(const UT_UCSChar * phaystack, const UT_UCSChar * pneedle);
UT_uint32 		UT_UCS_strlen(const UT_UCSChar * string);
UT_UCSChar * 	UT_UCS_strcpy(UT_UCSChar * dest, const UT_UCSChar * src);
UT_UCSChar * 	UT_UCS_strcpy_char(UT_UCSChar * dest, const char * src);
char *			UT_UCS_strcpy_to_char(char * dest, const UT_UCSChar * src);
UT_Bool			UT_UCS_cloneString(UT_UCSChar ** dest, const UT_UCSChar * src);
UT_Bool			UT_UCS_cloneString_char(UT_UCSChar ** dest, const char * src);

char * UT_upperString(char * string);
char * UT_lowerString(char * string);

UT_UCSChar UT_decodeUTF8char(const XML_Char * p, UT_uint32 len);
void UT_decodeUTF8string(const XML_Char * p, UT_uint32 len, UT_GrowBuf * pResult);

#define UT_UCS_isdigit(x)	(((x) >= '0') && ((x) <= '9'))
#define UT_UCS_isupper(x)	(((x) >= 'A') && ((x) <= 'Z'))		// HACK: not UNICODE-safe

#ifdef WIN32
#define snprintf _snprintf

#define _(String) (String)
#define N_(String) (String)

#endif /* WIN32 */
UT_END_EXTERN_C

#endif /* UT_STRING_H */
