/* AbiSource Program Utilities
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#include <string.h>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#define UT_strcmp(a, b) strcmp((a), (b))
#define UT_strcoll(a, b) strcoll((a), (b))

class UT_GrowBuf;

UT_BEGIN_EXTERN_C

///////////////////////////////////////////////////////////////////////////////
// UTFXX->UTF8 character conversion
ABI_EXPORT int  unichar_to_utf8 (int c, unsigned char *outbuf);

///////////////////////////////////////////////////////////////////////////////
// Replacements for common non-ANSI functions

ABI_EXPORT char *  UT_strdup(const char * szSource);

// Below are case-insensitive strcmp-like functions prototypes.  The functions
// lexographically compare strings, returning <0, 0, >0 as strcmp(...) does.
// For strings containing characters between 'Z' and 'a' in the ASCII table,
// the strings /do/ compare differently depending on case.  For example,
// "ABCDE" < "ABCD^", but "abcde" > "abcd^".
// This functionality is comparable with the 'standard' GNU strcasecmp(...)
// and the Microsoft stricmp functions.
ABI_EXPORT UT_sint32 UT_stricmp(const char *s1, const char *s2);
ABI_EXPORT UT_sint32 UT_strnicmp(const char *s1, const char *s2, int ilen);

///////////////////////////////////////////////////////////////////////////////

ABI_EXPORT bool  UT_cloneString(char *& rszDest, const char * szSource);
ABI_EXPORT bool  UT_replaceString(char *& rszDest, const char * szSource);

ABI_EXPORT UT_uint32  UT_XML_strlen(const XML_Char * sz);
ABI_EXPORT bool  UT_XML_cloneString(XML_Char *& rszDest, const XML_Char * szSource);
ABI_EXPORT bool  UT_XML_cloneList(XML_Char **& rszDest, const XML_Char ** szSource);
ABI_EXPORT bool  UT_XML_replaceList(XML_Char **& rszDest, const XML_Char ** szSource);
ABI_EXPORT UT_sint32  UT_XML_stricmp(const XML_Char * sz1, const XML_Char * sz2);
ABI_EXPORT UT_sint32  UT_XML_strnicmp(const XML_Char * sz1, const XML_Char * sz2, const UT_uint32 n);
ABI_EXPORT UT_sint32  UT_XML_strcmp(const XML_Char * sz1, const XML_Char * sz2);
ABI_EXPORT UT_uint32  UT_XML_strncpy(XML_Char * szDest, UT_uint32 nLen, const XML_Char * szSource);

// this function allocates (and returns a pointer to) new memory for the new string
ABI_EXPORT bool  UT_XML_cloneNoAmpersands(XML_Char *& rszDest, const XML_Char * szSource);
// This function uses a static buffer to do the translation
ABI_EXPORT XML_Char *  UT_XML_transNoAmpersands(const XML_Char * szSource);

ABI_EXPORT UT_uint32  UT_pointerArrayLength(void ** array);

// the naming convention has deviated from the above.  it's kind
// of a mutant libc/C++ naming convention.
ABI_EXPORT UT_sint32 		 UT_UCS2_strcmp(const UT_UCS2Char* left, const UT_UCS2Char* right);
ABI_EXPORT UT_UCS2Char * 	 UT_UCS2_strstr(const UT_UCS2Char * phaystack, const UT_UCS2Char * pneedle);
ABI_EXPORT UT_UCS2Char * 	 UT_UCS2_stristr(const UT_UCS2Char * phaystack, const UT_UCS2Char * pneedle);
ABI_EXPORT UT_uint32 		 UT_UCS2_strlen(const UT_UCS2Char * string);
ABI_EXPORT UT_UCS2Char * 	 UT_UCS2_strcpy(UT_UCS2Char * dest, const UT_UCS2Char * src);
ABI_EXPORT UT_UCS2Char * 	 UT_UCS2_strcpy_char(UT_UCS2Char * dest, const char * src);
ABI_EXPORT char *			 UT_UCS2_strcpy_to_char(char * dest, const UT_UCS2Char * src);
ABI_EXPORT bool			 UT_UCS2_cloneString(UT_UCS2Char ** dest, const UT_UCS2Char * src);
ABI_EXPORT bool			 UT_UCS2_cloneString_char(UT_UCS2Char ** dest, const char * src);
ABI_EXPORT UT_UCS2Char *     UT_UCS2_strncpy(UT_UCS2Char * dest, const UT_UCS2Char * src, UT_uint32 n);
ABI_EXPORT UT_UCS2Char *     UT_UCS2_strnrev(UT_UCS2Char * dest, UT_uint32 n);

ABI_EXPORT UT_UCS2Char		 UT_UCS2_tolower(UT_UCS2Char c);
ABI_EXPORT UT_UCS2Char       UT_UCS2_toupper(UT_UCS2Char c);

ABI_EXPORT UT_sint32 		 UT_UCS4_strcmp(const UT_UCS4Char* left, const UT_UCS4Char* right);
ABI_EXPORT UT_UCS4Char * 	 UT_UCS4_strstr(const UT_UCS4Char * phaystack, const UT_UCS4Char * pneedle);
ABI_EXPORT UT_UCS4Char * 	 UT_UCS4_stristr(const UT_UCS4Char * phaystack, const UT_UCS4Char * pneedle);
ABI_EXPORT UT_uint32 		 UT_UCS4_strlen(const UT_UCS4Char * string);
ABI_EXPORT UT_UCS4Char * 	 UT_UCS4_strcpy(UT_UCS4Char * dest, const UT_UCS4Char * src);
ABI_EXPORT UT_UCS4Char * 	 UT_UCS4_strcpy_char(UT_UCS4Char * dest, const char * src);
ABI_EXPORT char *			 UT_UCS4_strcpy_to_char(char * dest, const UT_UCS4Char * src);
ABI_EXPORT bool			     UT_UCS4_cloneString(UT_UCS4Char ** dest, const UT_UCS4Char * src);
ABI_EXPORT bool			     UT_UCS4_cloneString_char(UT_UCS4Char ** dest, const char * src);
ABI_EXPORT UT_UCS4Char *     UT_UCS4_strncpy(UT_UCS4Char * dest, const UT_UCS4Char * src, UT_uint32 n);
ABI_EXPORT UT_UCS4Char *     UT_UCS4_strnrev(UT_UCS4Char * dest, UT_uint32 n);

ABI_EXPORT UT_UCS4Char		 UT_UCS4_tolower(UT_UCS4Char c);
ABI_EXPORT UT_UCS4Char       UT_UCS4_toupper(UT_UCS4Char c);

ABI_EXPORT char *  UT_upperString(char * string);
ABI_EXPORT char *  UT_lowerString(char * string);

ABI_EXPORT char *  UT_catPathname(const char * szPath, const char * szFile);
ABI_EXPORT char *  UT_tmpnam(char *);
ABI_EXPORT void    UT_unlink(const char *);

ABI_EXPORT UT_UCSChar  UT_decodeUTF8char(const XML_Char * p, UT_uint32 len);
ABI_EXPORT void  UT_decodeUTF8string(const XML_Char * p, UT_uint32 len, UT_GrowBuf * pResult);
ABI_EXPORT XML_Char *  UT_encodeUTF8char(UT_UCSChar cIn);

ABI_EXPORT bool UT_isUrl ( const char * sz );

/* ABI_EXPORT XML_Char *  UT_decodeXMLstring(XML_Char *pcIn);
 * This has moved to ut_xml.cpp as UT_XML::decode ()
 */

ABI_EXPORT bool  UT_isSmartQuotableCharacter(UT_UCSChar c);
ABI_EXPORT bool  UT_isSmartQuotedCharacter(UT_UCSChar c);

#define UT_UCS4_isdigit(x)	(((x) >= '0') && ((x) <= '9'))  // TODO: make UNICODE-wise
#define UT_UCS2_isdigit(x)	(((x) >= '0') && ((x) <= '9'))  // TODO: make UNICODE-wise

/*these are unicode-safe*/
ABI_EXPORT bool  UT_UCS2_isupper(UT_UCS2Char c);
ABI_EXPORT bool  UT_UCS2_islower(UT_UCS2Char c);
ABI_EXPORT bool  UT_UCS2_isalpha(UT_UCS2Char c);
ABI_EXPORT bool	 UT_UCS2_isSentenceSeparator(UT_UCS2Char c);
#define UT_UCS2_isalnum(x)	(UT_UCS2_isalpha(x) || UT_UCS2_isdigit(x)) // HACK: not UNICODE-safe
ABI_EXPORT bool UT_UCS2_isspace(UT_UCS2Char c);
#define UT_UCS2_ispunct(x)   ((!UT_UCS2_isspace(x)  &&  !UT_UCS2_isalnum(x)  &&  (x)>' '))  // HACK: not UNICODE safe

/*these are unicode-safe*/
ABI_EXPORT bool  UT_UCS4_isupper(UT_UCS4Char c);
ABI_EXPORT bool  UT_UCS4_islower(UT_UCS4Char c);
ABI_EXPORT bool  UT_UCS4_isalpha(UT_UCS4Char c);
ABI_EXPORT bool	 UT_UCS4_isSentenceSeparator(UT_UCS4Char c);
#define UT_UCS4_isalnum(x)	(UT_UCS4_isalpha(x) || UT_UCS4_isdigit(x)) // HACK: not UNICODE-safe
ABI_EXPORT bool UT_UCS4_isspace(UT_UCS4Char c);
#define UT_UCS4_ispunct(x)   ((!UT_UCS4_isspace(x)  &&  !UT_UCS4_isalnum(x)  &&  (x)>' '))  // HACK: not UNICODE safe


#ifdef WIN32
#define snprintf _snprintf

#define _(String) (String)
#define N_(String) (String)

#endif /* WIN32 */

#if defined (SNPRINTF_MISSING)
  extern int snprintf(char *str, size_t size, const  char  *format, ...);
#endif

/*
 this one prints floating point value but using dot as fractional serparator
 independent of the current locale's settings.
*/
ABI_EXPORT const char*  std_size_string(float f);

UT_END_EXTERN_C

#endif /* UT_STRING_H */
