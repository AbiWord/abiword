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

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include <fribidi/fribidi.h>
#include "ut_mbtowc.h"
#include "ut_wctomb.h"

#include "ut_string_class.h"

#include "xap_EncodingManager.h"

#define UT_STRING_CPP
#include "ut_case.h"
#undef  UT_STRING_CPP

// really simple way to determine if something is a hyperlink
// probably not 100% correct, but better than !stricmp(http://), ...
bool UT_isUrl ( const char * szName )
{
  UT_return_val_if_fail(szName, false);

  int len = strlen ( szName );
  int mailto_len = strlen ( "mailto:" );

  if ( NULL != strstr ( szName, "://") ) // dumb check, but probably true
    return true;
  else if ( ( len >= mailto_len ) &&
	    !UT_XML_strnicmp ( "mailto:", szName, mailto_len ) )
    return true;
  else
    return false;
}

/*
 * This is cut & pasted from glib 1.3 (c) RedHat
 * We need this for to convert UTF16 to UTF8
 */
int
unichar_to_utf8 (int c, unsigned char *outbuf)
{
  size_t len = 0;
  int first;
  int i;

  if (c < 0x80)
    {
      first = 0;
      len = 1;
    }
  else if (c < 0x800)
    {
      first = 0xc0;
      len = 2;
    }
  else if (c < 0x10000)
    {
      first = 0xe0;
      len = 3;
    }
   else if (c < 0x200000)
    {
      first = 0xf0;
      len = 4;
    }
  else if (c < 0x4000000)
    {
      first = 0xf8;
      len = 5;
    }
  else
    {
      first = 0xfc;
      len = 6;
    }

  if (outbuf)
    {
      for (i = len - 1; i > 0; --i)
	{
	  outbuf[i] = (c & 0x3f) | 0x80;
	  c >>= 6;
	}
      outbuf[0] = c | first;
    }

  return len;
}

//////////////////////////////////////////////////////////////////
// char * UT_catPathname(const char * szPath, const char * szFile);
// is defined in platform-specific code.
//////////////////////////////////////////////////////////////////

char * UT_strdup(const char * szSource)
{
  UT_return_val_if_fail(szSource, NULL);

	int len = strlen(szSource)+1;
	if(char * ret = (char *)UT_calloc(len, sizeof(char)))
		return((char *)memcpy(ret, szSource, len));
	else
		return(NULL);
}

UT_sint32 UT_stricmp(const char * s1, const char * s2)
{
  UT_return_val_if_fail(s1, 1);
  UT_return_val_if_fail(s2, -1);

	// Lifted from glibc.  Looks better (in a constant-factor sort of way)
	// than what we had before.  Ideally this should be per-platform.
	const unsigned char *p1 = (const unsigned char *) s1;
	const unsigned char *p2 = (const unsigned char *) s2;
	unsigned char c1, c2;

	if (s1 == s2)
		return 0;

	do
    {
		c1 = tolower (*p1++);
		c2 = tolower (*p2++);
		if (c1 == '\0')
			break;
    }
	while (c1 == c2);

	return c1 - c2;
}

// should really be a size_t, but that might break compilation on weird
// platforms.  I don't know.
UT_sint32 UT_strnicmp(const char *s1, const char *s2, int n)
{
  UT_return_val_if_fail(s1, 1);
  UT_return_val_if_fail(s2, -1);

	const unsigned char *p1 = (const unsigned char *) s1;
	const unsigned char *p2 = (const unsigned char *) s2;
	unsigned char c1, c2;

	if (p1 == p2 || n == 0)
		return 0;

	do
    {
		c1 = tolower (*p1++);
		c2 = tolower (*p2++);
		if (c1 == '\0' || c1 != c2)
			return c1 - c2;
    } while (--n > 0);

	return c1 - c2;
}

bool UT_cloneString(char *& rszDest, const char * szSource)
{
	if (szSource && *szSource)
	{
		UT_uint32 length = strlen(szSource) + 1;
		rszDest = (char *) malloc(length);

		if (!rszDest)
			return false;

		memmove(rszDest, szSource, length);
	}
	else
		rszDest = NULL;
	
	return true;
}

bool UT_replaceString(char *& rszDest, const char * szSource)
{
	if (rszDest)
		free(rszDest);
	rszDest = NULL;

	return UT_cloneString(rszDest,szSource);
}

UT_uint32 UT_XML_strlen(const XML_Char * sz)
{
	if (!sz || !*sz)
		return 0;

	UT_uint32 k = 0;
	while (sz[k])
		k++;

	return k;
}

// Is this function implemented somewhere else?

bool UT_XML_cloneList(XML_Char **& rszDest, const XML_Char ** szSource)
{
	if (!szSource)
		return true;

	XML_Char ** newmemory = (XML_Char **)
		UT_calloc(UT_pointerArrayLength((void **) szSource) + 1, sizeof(XML_Char *));

	if (newmemory == NULL)
		return false;

	memcpy((void *) newmemory, (const void *) szSource,
		   UT_pointerArrayLength((void **) szSource ) * sizeof(XML_Char *));

	rszDest = newmemory;

	return true;
}

bool UT_XML_replaceList(XML_Char **& rszDest, const XML_Char ** szSource)
{
	FREEP(rszDest);

	return UT_XML_cloneList(rszDest, szSource);
}

bool UT_XML_cloneString(XML_Char *& rszDest, const XML_Char * szSource)
{
	UT_uint32 length = UT_XML_strlen(szSource) + 1;
	rszDest = (XML_Char *)UT_calloc(length,sizeof(XML_Char));
	if (!rszDest)
		return false;
	memmove(rszDest,szSource,length*sizeof(XML_Char));
	return true;
}

UT_sint32 UT_XML_stricmp(const XML_Char * sz1, const XML_Char * sz2)
{
	UT_ASSERT(sizeof(char) == sizeof(XML_Char));
	return UT_stricmp((const char*)sz1,(const char*)sz2);
}

UT_sint32 UT_XML_strnicmp(const XML_Char * sz1, const XML_Char * sz2, const UT_uint32 n)
{
	UT_ASSERT(sizeof(char) == sizeof(XML_Char));
	return UT_strnicmp((const char*)sz1,(const char*)sz2,n);
}

UT_sint32 UT_XML_strcmp(const XML_Char * sz1, const XML_Char * sz2)
{
	UT_ASSERT(sizeof(char) == sizeof(XML_Char));
	return strcmp((const char*)sz1,(const char*)sz2);
}

bool UT_XML_cloneNoAmpersands(XML_Char *& rszDest, const XML_Char * szSource)
{
	if (szSource == NULL)
		return false;

	UT_uint32 length = UT_XML_strlen(szSource) + 1;
	rszDest = (XML_Char *) UT_calloc(length, sizeof(XML_Char));

	if (!rszDest)
		return false;

	const XML_Char * o = szSource;
	XML_Char * n = rszDest;
	while (*o != 0)
	{
		if (*o != '&')
		{
			*n = *o;
			n++;
		}
		o++;
	}

	return true;
}

/* This uses the clone no ampersands but dumps into a static buffer */
XML_Char *UT_XML_transNoAmpersands(const XML_Char * szSource)
{
	static XML_Char *rszDestBuffer = NULL;
	static UT_uint32 iDestBufferLength = 0;

	if (szSource == NULL)
		return NULL;

	UT_uint32 length = UT_XML_strlen(szSource) + 1;
	if (length > iDestBufferLength) {
		if (rszDestBuffer && iDestBufferLength) {
			free(rszDestBuffer);
		}
		iDestBufferLength = 0;
		rszDestBuffer = (XML_Char *) UT_calloc(length, sizeof(XML_Char));

		if (!rszDestBuffer)
			return NULL;

		iDestBufferLength = length;
	}
	memset(rszDestBuffer, 0, iDestBufferLength);

	const XML_Char * o = szSource;
	XML_Char * n = rszDestBuffer;
	while (*o != 0)
	{
		if (*o != '&')
		{
			*n = *o;
			n++;
		}
		o++;
	}

	return rszDestBuffer;
}


// TODO : put a better strncpy here; resolve to platform version if available

UT_uint32 UT_XML_strncpy(XML_Char * szDest, UT_uint32 nLen, const XML_Char * szSource)
{
	if (!szSource)
		return 0;

	UT_ASSERT(szDest);

	UT_uint32 i = 0;

	while (i < nLen)
	{
		szDest[i] = szSource[i];

		// if we just wrote NULL, return
		if (szDest[i] == 0)
			return i;

		i++;
	}

	return i;
}

UT_uint32 UT_pointerArrayLength(void ** array)
{
	if (! (array && *array))
		return 0;

	UT_uint32 i = 0;
	while (array[i])
		i++;

	return i;
}

/*
  The following code is from the GNU C library, version 2.0.6.
  It has been reformatted and tweaked to do Unicode strstrs.
  All this licensing stuff is kinda ugly, but I didn't want
  to risk merging the licensing for fear I might break some law.
*/

/* Copyright (C) 1994, 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/*
 * My personal strstr() implementation that beats most other algorithms.
 * Until someone tells me otherwise, I assume that this is the
 * fastest implementation of strstr() in C.
 * I deliberately chose not to comment it.  You should have at least
 * as much fun trying to understand it, as I had to write it :-).
 *
 * Stephen R. van den Berg, berg@pool.informatik.rwth-aachen.de */

UT_UCS2Char * UT_UCS2_strstr(const UT_UCS2Char * phaystack, const UT_UCS2Char * pneedle)
{
	register const UT_UCS2Char *haystack, *needle;
	register UT_UCS2Char b, c;

	haystack = (const UT_UCS2Char *) phaystack;
	needle = (const UT_UCS2Char *) pneedle;

	b = *needle;
	if (b != '\0')
    {
		haystack--;                               /* possible ANSI violation */
		do
        {
			c = *++haystack;
			if (c == '\0')
				goto ret0;
        }
		while (c != b);

		c = *++needle;
		if (c == '\0')
			goto foundneedle;
		++needle;
		goto jin;

		for (;;)
        {
			register UT_UCS2Char a;
			register const UT_UCS2Char *rhaystack, *rneedle;

			do
            {
				a = *++haystack;
				if (a == '\0')
					goto ret0;
				if (a == b)
					break;
				a = *++haystack;
				if (a == '\0')
					goto ret0;
			shloop: ; // need a statement here for EGCS 1.1.1 to accept it
			}
			while (a != b);

		jin:	a = *++haystack;
			if (a == '\0')
				goto ret0;

			if (a != c)
				goto shloop;

			rhaystack = haystack-- + 1;
			rneedle = needle;
			a = *rneedle;

			if (*rhaystack == a)
				do
				{
					if (a == '\0')
						goto foundneedle;
					++rhaystack;
					a = *++needle;
					if (*rhaystack != a)
						break;
					if (a == '\0')
						goto foundneedle;
					++rhaystack;
					a = *++needle;
				}
				while (*rhaystack == a);

			needle = rneedle;             /* took the register-poor approach */

			if (a == '\0')
				break;
        }
    }
 foundneedle:
	return (UT_UCS2Char *) haystack;
 ret0:
	return 0;
}

UT_sint32 UT_UCS2_strcmp(const UT_UCS2Char* left, const UT_UCS2Char* right)
{
	UT_ASSERT(left);
	UT_ASSERT(right);

	while (*left && *right)
	{
		if (*left < *right)
		{
			return -1;
		}

		if (*left > *right)
		{
			return 1;
		}

		left++;
		right++;
	}

	if (*left)
	{
		return -1;
	}
	else if (*right)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
  Latin-1 Unicode case-insensitive string comparison and casing done by
  Pierre Sarrazin <ps@cam.org>.
*/

/**
 * Convert a given character to uppercase
 */
UT_UCS2Char UT_UCS2_toupper(UT_UCS2Char c)
{
        if (c < 128) // in ASCII range
	  return toupper(c);

	if (XAP_EncodingManager::get_instance()->single_case())
		return c;
	/*let's trust libc! -- does not seem to work :(*/
    case_entry * letter = (case_entry *)bsearch(&c, &case_table, NrElements(case_table),sizeof(case_entry),s_cmp_case);
    if(!letter || letter->type == 1)
        return c;
    return letter->other;
}


/*	Converts the given character to lowercase if it is an uppercase letter.
	Returns it unchanged if it is not.
	This function created by Pierre Sarrazin 1999-02-06
*/

UT_UCS2Char UT_UCS2_tolower(UT_UCS2Char c)
{
	if (c < 128)
		return tolower(c);
	if (XAP_EncodingManager::get_instance()->single_case())
		return c;
	/*let's trust libc!*/
    case_entry * letter = (case_entry *)bsearch(&c, &case_table, NrElements(case_table),sizeof(case_entry),s_cmp_case);
    if(!letter || letter->type == 0)
        return c;
    return letter->other;
}


/*	Characters are converted to lowercase (if applicable) when they
	are read from the needle or the haystack. See UT_UCS_tolower().
	This function created by Pierre Sarrazin 1999-02-06
*/

UT_UCS2Char * UT_UCS2_stristr(const UT_UCS2Char * phaystack, const UT_UCS2Char * pneedle)
{
	register const UT_UCS2Char *haystack, *needle;
	register UT_UCS2Char b, c;

	haystack = (const UT_UCS2Char *) phaystack;
	needle = (const UT_UCS2Char *) pneedle;

	b = UT_UCS2_tolower(*needle);
	if (b != '\0')
    {
		haystack--;                               /* possible ANSI violation */
		do
        {
			c = UT_UCS2_tolower(*++haystack);
			if (c == '\0')
				goto ret0;
        }
		while (c != b);

		c = UT_UCS2_tolower(*++needle);
		if (c == '\0')
			goto foundneedle;
		++needle;
		goto jin;

		for (;;)
        {
			register UT_UCS2Char a;
			register const UT_UCS2Char *rhaystack, *rneedle;

			do
            {
				a = UT_UCS2_tolower(*++haystack);
				if (a == '\0')
					goto ret0;
				if (a == b)
					break;
				a = UT_UCS2_tolower(*++haystack);
				if (a == '\0')
					goto ret0;
			shloop: ; // need a statement here for EGCS 1.1.1 to accept it
			}
			while (a != b);

		jin:	a = UT_UCS2_tolower(*++haystack);
			if (a == '\0')
				goto ret0;

			if (a != c)
				goto shloop;

			rhaystack = haystack-- + 1;
			rneedle = needle;
			a = UT_UCS2_tolower(*rneedle);

			if (UT_UCS2_tolower(*rhaystack) == a)
				do
				{
					if (a == '\0')
						goto foundneedle;
					++rhaystack;
					a = UT_UCS2_tolower(*++needle);
					if (UT_UCS2_tolower(*rhaystack) != a)
						break;
					if (a == '\0')
						goto foundneedle;
					++rhaystack;
					a = UT_UCS2_tolower(*++needle);
				}
				while (UT_UCS2_tolower(*rhaystack) == a);

			needle = rneedle;             /* took the register-poor approach */

			if (a == '\0')
				break;
        }
    }
 foundneedle:
	return (UT_UCS2Char *) haystack;
 ret0:
	return 0;
}
/****************************************************************************/

UT_uint32 UT_UCS2_strlen(const UT_UCS2Char * string)
{
	UT_uint32 i;

	for(i = 0; *string != 0; string++, i++)
		;

	return i;
}

UT_UCS2Char * UT_UCS2_strcpy(UT_UCS2Char * dest, const UT_UCS2Char * src)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_UCS2Char * d = dest;
	UT_UCS2Char * s = (UT_UCS2Char *) src;

	while (*s != 0)
		*d++ = *s++;
	*d = 0;

	return dest;
}

UT_UCS2Char * UT_UCS2_strcpy_char(UT_UCS2Char * dest, const char * src)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_UCS2Char * d 		= dest;
	unsigned char * s	= (unsigned char *) src;

	static UT_UCS2_mbtowc m(XAP_EncodingManager::get_instance()->getNative8BitEncodingName());
	UT_UCS2Char wc;

	while (*s != 0)
	  {
		if(m.mbtowc(wc,*s))*d++=wc;
		s++;
	  }
	*d = 0;

	return dest;
}

char * UT_UCS2_strcpy_to_char(char * dest, const UT_UCS2Char * src)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_ASSERT_NOT_REACHED();

	return NULL;
}

bool UT_UCS2_cloneString(UT_UCS2Char ** dest, const UT_UCS2Char * src)
{
	UT_uint32 length = UT_UCS2_strlen(src) + 1;
	*dest = (UT_UCS2Char *)UT_calloc(length,sizeof(UT_UCS2Char));
	if (!*dest)
		return false;
	memmove(*dest,src,length*sizeof(UT_UCS2Char));

	return true;
}

bool UT_UCS2_cloneString_char(UT_UCS2Char ** dest, const char * src)
{
  UT_ASSERT_NOT_REACHED();
  return false;
}

// convert each character in a string to ASCII uppercase
char * UT_upperString(char * string)
{
	if (!string)
		return 0;

	for (char * ch = string; *ch != 0; ch++)
		*ch = toupper(*ch);

	return string;
}

// convert each character in a string to ASCII lowercase
char * UT_lowerString(char * string)
{
	if (!string)
		return 0;

	for (char * ch = string; *ch != 0; ch++)
		*ch = tolower(*ch);

	return string;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

UT_UCSChar UT_decodeUTF8char(const XML_Char * p, UT_uint32 len)
{
	UT_UCSChar ucs, ucs1, ucs2, ucs3;

	switch (len)
	{
	case 2:
		ucs1 = (UT_UCSChar)(p[0] & 0x1f);
		ucs2 = (UT_UCSChar)(p[1] & 0x3f);
		ucs  = (ucs1 << 6) | ucs2;
		return ucs;

	case 3:
		ucs1 = (UT_UCSChar)(p[0] & 0x0f);
		ucs2 = (UT_UCSChar)(p[1] & 0x3f);
		ucs3 = (UT_UCSChar)(p[2] & 0x3f);
		ucs  = (ucs1 << 12) | (ucs2 << 6) | ucs3;
		return ucs;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
}

void UT_decodeUTF8string(const XML_Char * pString, UT_uint32 len, UT_GrowBuf * pResult)
{
	// decode the given string [ p[0]...p[len] ] and append to the given growbuf.

	UT_ASSERT(sizeof(XML_Char) == sizeof(UT_Byte));
	UT_Byte * p = (UT_Byte *)pString;	// XML_Char is signed...

	int bytesInSequence = 0;
	int bytesExpectedInSequence = 0;
	XML_Char buf[5];

	for (UT_uint32 k=0; k<len; k++)
	{
		if (p[k] < 0x80)						// plain us-ascii part of latin-1
		{
			UT_ASSERT(bytesInSequence == 0);
			UT_UCSChar c = p[k];
			pResult->append((UT_GrowBufElement *)&c,1);
		}
		else if ((p[k] & 0xf0) == 0xf0)			// lead byte in 4-byte surrogate pair
		{
			// surrogate pairs are defined in section 3.7 of the
			// unicode standard version 2.0 as an extension
			// mechanism for rare characters in future extensions
			// of the unicode standard.
			UT_ASSERT(bytesInSequence == 0);
			UT_ASSERT(UT_NOT_IMPLEMENTED);
		}
		else if ((p[k] & 0xe0) == 0xe0)			// lead byte in 3-byte sequence
		{
			UT_ASSERT(bytesInSequence == 0);
			bytesExpectedInSequence = 3;
			buf[bytesInSequence++] = p[k];
		}
		else if ((p[k] & 0xc0) == 0xc0)			// lead byte in 2-byte sequence
		{
			UT_ASSERT(bytesInSequence == 0);
			bytesExpectedInSequence = 2;
			buf[bytesInSequence++] = p[k];
		}
		else if ((p[k] & 0x80) == 0x80)			// trailing byte in multi-byte sequence
		{
			UT_ASSERT(bytesInSequence > 0);
			buf[bytesInSequence++] = p[k];
			if (bytesInSequence == bytesExpectedInSequence)		// final byte in multi-byte sequence
			{
				UT_UCSChar c = UT_decodeUTF8char(buf,bytesInSequence);
				pResult->append((UT_GrowBufElement *)&c,1);
				bytesInSequence = 0;
				bytesExpectedInSequence = 0;
			}
		}
	}
}

#if 1 // i didn't get a chance to test this -- jeff
XML_Char* UT_encodeUTF8char(UT_UCSChar cIn)
{
	// convert the given unicode character into a UTF8 sequence
	// return pointer to static buffer.

	static XML_Char sBuf[10];

	memset(sBuf,0,sizeof(sBuf));
	if (cIn < 0x0080)
	{
		sBuf[0] = (XML_Char)cIn;
	}
	else if (cIn < 0x0800)
	{
		// 110xxxxx 10xxxxxx
		sBuf[0] = 0x00c0 | ((cIn >> 6) & 0x001f);
		sBuf[1] = 0x0080 | (    cIn    & 0x003f);
	}
	else
	{
		// 1110xxxx 10xxxxxx 10xxxxxx
		sBuf[0] = 0x00e0 | ((cIn >> 12) & 0x000f);
		sBuf[1] = 0x0080 | ((cIn >>  6) & 0x003f);
		sBuf[2] = 0x0080 | (    cIn     & 0x003f);
	}

	return sBuf;
}
#endif // --jeff

bool UT_isSmartQuotableCharacter(UT_UCSChar c)
{
	// TODO:  this is anglo-centric; really need a locale argument or
	// TODO:  something to get smart quote rules for the rest of the world
	bool result;
	switch (c)
	{
	case '"':
	case '`':
	case '\'':
		result = true;
		break;
	default:
		result = false;
		break;
	}
	return (result);
}

bool UT_isSmartQuotedCharacter(UT_UCSChar c)
{
	// TODO:  this is anglo-centric; really need a locale argument or
	// TODO:  something to get smart quote rules for the rest of the world
	bool result;
	switch (c)
	{
	case UCS_LQUOTE:
	case UCS_RQUOTE:
	case UCS_LDBLQUOTE:
	case UCS_RDBLQUOTE:
		result = true;
		break;
	default:
		result = false;
		break;
	}
	return (result);
}

bool UT_UCS2_isupper(UT_UCS2Char c)
{
	if(c < 127)
		return isupper(c)!=0;

    case_entry * letter = (case_entry *)bsearch(&c, &case_table, NrElements(case_table),sizeof(case_entry),s_cmp_case);
    if(letter && letter->type == 1)
        return true;
    return false;
};

bool UT_UCS2_islower(UT_UCS2Char c)
{
	if(c < 127)
		return islower(c)!=0;

    case_entry * letter = (case_entry *)bsearch(&c, &case_table, NrElements(case_table),sizeof(case_entry),s_cmp_case);
    if(!letter || letter->type == 0)
        return true;
    return false;
};

bool UT_UCS2_isspace(UT_UCS2Char c)
{
	// the whitespace table is small, so use linear search
	for (UT_uint32 i = 0; i < NrElements(whitespace_table); i++)
	{
		if(whitespace_table[i].high < c)
			continue;
		if(whitespace_table[i].low <= c)
			return true;
		// if we got here, then low > c
		return false;
	}
	return false;
};

bool UT_UCS2_isalpha(UT_UCS2Char c)
{
    FriBidiCharType type = fribidi_get_type(c);
    return (FRIBIDI_IS_LETTER(type) != 0);
};

bool UT_UCS2_isSentenceSeparator(UT_UCS2Char c)
{
	switch(c)
	{
		case '.':
			return true;

		default:
			return false;
	}
}


bool UT_UCS4_isupper(UT_UCS4Char c)
{
	if(c < 127)
		return isupper(c)!=0;

    case_entry * letter = (case_entry *)bsearch(&c, &case_table, NrElements(case_table),sizeof(case_entry),s_cmp_case);
    if(letter && letter->type == 1)
        return true;
    return false;
};

bool UT_UCS4_islower(UT_UCS4Char c)
{
	if(c < 127)
		return islower(c)!=0;

    case_entry * letter = (case_entry *)bsearch(&c, &case_table, NrElements(case_table),sizeof(case_entry),s_cmp_case);
    if(!letter || letter->type == 0)
        return true;
    return false;
};

bool UT_UCS4_isspace(UT_UCS4Char c)
{
	// the whitespace table is small, so use linear search
	for (UT_uint32 i = 0; i < NrElements(whitespace_table); i++)
	{
		if(whitespace_table[i].high < c)
			continue;
		if(whitespace_table[i].low <= c)
			return true;
		// if we got here, then low > c
		return false;
	}
	return false;
};

bool UT_UCS4_isalpha(UT_UCS4Char c)
{
    FriBidiCharType type = fribidi_get_type(c);
    return (FRIBIDI_IS_LETTER(type) != 0);
};

bool UT_UCS4_isSentenceSeparator(UT_UCS4Char c)
{
	switch(c)
	{
		case '.':
			return true;

		default:
			return false;
	}
}


/*
 this one prints floating point value but using dot as fractional serparator
 independent of the current locale's settings.
*/
const char* std_size_string(float f)
{
  static char string[10];
  int i=(int)f;
  if(f-i<0.1)
	sprintf(string, "%d", i);
  else {
          int fr = int(10*(f-i));
	sprintf(string,"%d.%d", i,fr);
  };
  return string;
};

/* copies exactly n-chars from src to dest; NB! does not check for 00 i src
*/
UT_UCS2Char * UT_UCS2_strncpy(UT_UCS2Char * dest, const UT_UCS2Char * src, UT_uint32 n)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_UCS2Char * d = dest;
	UT_UCS2Char * s = (UT_UCS2Char *) src;

	for (; d < (UT_UCS2Char *)dest + n;)
		*d++ = *s++;
	*d = '\0';

	return dest;
}


/* reverses str of len n; used by BiDi which always knows the len of string to process
   thus we can save ourselves searching for the 00 */
UT_UCS2Char * UT_UCS2_strnrev(UT_UCS2Char * src, UT_uint32 n)
{
    UT_UCS2Char t;
    UT_uint32 i;

    for(i = 0; i < n/2; i++)
    {
        t = *(src + i);
        *(src + i) = *(src + n - i - 1); //-1 so that we do not move the 00
        *(src + n - i - 1) = t;
    }
    return src;
}

/* copies exactly n-chars from src to dest; NB! does not check for 00 i src
*/
UT_UCS4Char * UT_UCS4_strncpy(UT_UCS4Char * dest, const UT_UCS4Char * src, UT_uint32 n)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_UCSChar * d = dest;
	UT_UCSChar * s = (UT_UCS4Char *) src;

	for (; d < (UT_UCS4Char *)dest + n;)
		*d++ = *s++;
	*d = '\0';

	return dest;
}


/* reverses str of len n; used by BiDi which always knows the len of string to process
   thus we can save ourselves searching for the 00 */
UT_UCS4Char * UT_UCS4_strnrev(UT_UCS4Char * src, UT_uint32 n)
{
    UT_UCS4Char t;
    UT_uint32 i;

    for(i = 0; i < n/2; i++)
    {
        t = *(src + i);
        *(src + i) = *(src + n - i - 1); //-1 so that we do not move the 00
        *(src + n - i - 1) = t;
    }
    return src;
}


UT_UCS4Char * UT_UCS4_strstr(const UT_UCS4Char * phaystack, const UT_UCS4Char * pneedle)
{
	register const UT_UCS4Char *haystack, *needle;
	register UT_UCS4Char b, c;

	haystack = (const UT_UCS4Char *) phaystack;
	needle = (const UT_UCS4Char *) pneedle;

	b = *needle;
	if (b != '\0')
    {
		haystack--;                               /* possible ANSI violation */
		do
        {
			c = *++haystack;
			if (c == '\0')
				goto ret0;
        }
		while (c != b);

		c = *++needle;
		if (c == '\0')
			goto foundneedle;
		++needle;
		goto jin;

		for (;;)
        {
			register UT_UCS4Char a;
			register const UT_UCS4Char *rhaystack, *rneedle;

			do
            {
				a = *++haystack;
				if (a == '\0')
					goto ret0;
				if (a == b)
					break;
				a = *++haystack;
				if (a == '\0')
					goto ret0;
			shloop: ; // need a statement here for EGCS 1.1.1 to accept it
			}
			while (a != b);

		jin:	a = *++haystack;
			if (a == '\0')
				goto ret0;

			if (a != c)
				goto shloop;

			rhaystack = haystack-- + 1;
			rneedle = needle;
			a = *rneedle;

			if (*rhaystack == a)
				do
				{
					if (a == '\0')
						goto foundneedle;
					++rhaystack;
					a = *++needle;
					if (*rhaystack != a)
						break;
					if (a == '\0')
						goto foundneedle;
					++rhaystack;
					a = *++needle;
				}
				while (*rhaystack == a);

			needle = rneedle;             /* took the register-poor approach */

			if (a == '\0')
				break;
        }
    }
 foundneedle:
	return (UT_UCS4Char *) haystack;
 ret0:
	return 0;
}

UT_sint32 UT_UCS4_strcmp(const UT_UCS4Char* left, const UT_UCS4Char* right)
{
	UT_ASSERT(left);
	UT_ASSERT(right);

	while (*left && *right)
	{
		if (*left < *right)
		{
			return -1;
		}

		if (*left > *right)
		{
			return 1;
		}

		left++;
		right++;
	}

	if (*left)
	{
		return -1;
	}
	else if (*right)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
  Latin-1 Unicode case-insensitive string comparison and casing done by
  Pierre Sarrazin <ps@cam.org>.
*/

/**
 * Convert a given character to uppercase
 */
UT_UCS4Char UT_UCS4_toupper(UT_UCS4Char c)
{
        if (c < 128) // in ASCII range
	  return toupper(c);

	if (XAP_EncodingManager::get_instance()->single_case())
		return c;
	/*let's trust libc! -- does not seem to work :(*/
    case_entry * letter = (case_entry *)bsearch(&c, &case_table, NrElements(case_table),sizeof(case_entry),s_cmp_case);
    if(!letter || letter->type == 1)
        return c;
    return letter->other;
}


/*	Converts the given character to lowercase if it is an uppercase letter.
	Returns it unchanged if it is not.
	This function created by Pierre Sarrazin 1999-02-06
*/

UT_UCS4Char UT_UCS4_tolower(UT_UCS4Char c)
{
	if (c < 128)
		return tolower(c);

	if (XAP_EncodingManager::get_instance()->single_case())
		return c;
	/*let's trust libc!*/
    case_entry * letter = (case_entry *)bsearch(&c, &case_table, NrElements(case_table),sizeof(case_entry),s_cmp_case);
    if(!letter || letter->type == 0)
        return c;
    return letter->other;
}


/*	Characters are converted to lowercase (if applicable) when they
	are read from the needle or the haystack. See UT_UCS_tolower().
	This function created by Pierre Sarrazin 1999-02-06
*/

UT_UCS4Char * UT_UCS4_stristr(const UT_UCS4Char * phaystack, const UT_UCS4Char * pneedle)
{
	register const UT_UCS4Char *haystack, *needle;
	register UT_UCS4Char b, c;

	haystack = (const UT_UCS4Char *) phaystack;
	needle = (const UT_UCS4Char *) pneedle;

	b = UT_UCS4_tolower(*needle);
	if (b != '\0')
    {
		haystack--;                               /* possible ANSI violation */
		do
        {
			c = UT_UCS4_tolower(*++haystack);
			if (c == '\0')
				goto ret0;
        }
		while (c != b);

		c = UT_UCS4_tolower(*++needle);
		if (c == '\0')
			goto foundneedle;
		++needle;
		goto jin;

		for (;;)
        {
			register UT_UCS4Char a;
			register const UT_UCS4Char *rhaystack, *rneedle;

			do
            {
				a = UT_UCS4_tolower(*++haystack);
				if (a == '\0')
					goto ret0;
				if (a == b)
					break;
				a = UT_UCS4_tolower(*++haystack);
				if (a == '\0')
					goto ret0;
			shloop: ; // need a statement here for EGCS 1.1.1 to accept it
			}
			while (a != b);

		jin:	a = UT_UCS4_tolower(*++haystack);
			if (a == '\0')
				goto ret0;

			if (a != c)
				goto shloop;

			rhaystack = haystack-- + 1;
			rneedle = needle;
			a = UT_UCS4_tolower(*rneedle);

			if (UT_UCS4_tolower(*rhaystack) == a)
				do
				{
					if (a == '\0')
						goto foundneedle;
					++rhaystack;
					a = UT_UCS4_tolower(*++needle);
					if (UT_UCS4_tolower(*rhaystack) != a)
						break;
					if (a == '\0')
						goto foundneedle;
					++rhaystack;
					a = UT_UCS4_tolower(*++needle);
				}
				while (UT_UCS4_tolower(*rhaystack) == a);

			needle = rneedle;             /* took the register-poor approach */

			if (a == '\0')
				break;
        }
    }
 foundneedle:
	return (UT_UCS4Char *) haystack;
 ret0:
	return 0;
}
/****************************************************************************/

UT_uint32 UT_UCS4_strlen(const UT_UCS4Char * string)
{
	UT_uint32 i;

	for(i = 0; *string != 0; string++, i++)
		;

	return i;
}

UT_UCS4Char * UT_UCS4_strcpy(UT_UCS4Char * dest, const UT_UCS4Char * src)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_UCS4Char * d = dest;
	UT_UCS4Char * s = (UT_UCS4Char *) src;

	while (*s != 0)
		*d++ = *s++;
	*d = 0;

	return dest;
}

// TODO shouldn't all of the 'char *' strings be 'unsigned char *' strings ??

UT_UCS4Char * UT_UCS4_strcpy_char(UT_UCS4Char * dest, const char * src)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_UCS4Char * d 		= dest;
	unsigned char * s	= (unsigned char *) src;

	static UT_UCS4_mbtowc m(XAP_EncodingManager::get_instance()->getNative8BitEncodingName());
	UT_UCS4Char wc;

	while (*s != 0)
	  {
		if(m.mbtowc(wc,*s))*d++=wc;
		s++;
	  }
	*d = 0;

	return dest;
}

UT_UCS4Char * UT_UCS4_strcpy_utf8_char(UT_UCS4Char * dest, const char * src)
{
	// FIXME: This could be more efficient than it is, on the other
	// hand, it should be correct

	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_UCS4String ucs4str(src); // constructs a string from utf8 by default
	dest = UT_UCS4_strcpy(dest, ucs4str.ucs4_str());

	return dest;
}


char * UT_UCS4_strcpy_to_char(char * dest, const UT_UCS4Char * src)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	char * 			d = dest;
	UT_UCS4Char * 	s = (UT_UCS4Char *) src;

	UT_Wctomb w(XAP_EncodingManager::get_instance()->getNative8BitEncodingName());

	while (*s != 0)
	  {
		int length;
		w.wctomb_or_fallback(d,length,*s++);
		d+=length;
	  }
	*d = 0;

	return dest;
}

bool UT_UCS4_cloneString(UT_UCS4Char ** dest, const UT_UCS4Char * src)
{
	UT_uint32 length = UT_UCS4_strlen(src) + 1;
	*dest = (UT_UCS4Char *)UT_calloc(length,sizeof(UT_UCS4Char));
	if (!*dest)
		return false;
	memmove(*dest,src,length*sizeof(UT_UCS4Char));

	return true;
}

bool UT_UCS4_cloneString_char(UT_UCS4Char ** dest, const char * src)
{
  UT_uint32 length = strlen(src) + 1;
  *dest = (UT_UCS4Char *)UT_calloc(length,sizeof(UT_UCS4Char));
  if (!*dest)
    return false;
  UT_UCS4_strcpy_char(*dest, src);
  
  return true;
}
