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

UT_BEGIN_EXTERN_C

UT_sint32 UT_stricmp(const char *s1, const char *s2);
UT_sint32 UT_strnicmp(const char *s1, const char *s2, int lenS1);
UT_Bool UT_cloneString(char *& rszDest, const char * szSource);
UT_Bool UT_replaceString(char *& rszDest, const char * szSource);

UT_uint32 UT_XML_strlen(const XML_Char * sz);
UT_Bool UT_XML_cloneString(XML_Char *& rszDest, const XML_Char * szSource);
UT_sint32 UT_XML_stricmp(const XML_Char * sz1, const XML_Char * sz2);


#ifdef WIN32
#define snprintf _snprintf

#define _(String) (String)
#define N_(String) (String)

#endif /* WIN32 */
UT_END_EXTERN_C

#endif /* UT_STRING_H */
