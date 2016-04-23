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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <math.h>
#include <ctype.h>

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include <fribidi.h>
#include "ut_mbtowc.h"
#include "ut_wctomb.h"

#include "ut_string_class.h"

#include "xap_EncodingManager.h"

#define UT_STRING_CPP
#include "ut_case.h"
#undef  UT_STRING_CPP

bool UT_XML_cloneNoAmpersands(gchar *& rszDest, const gchar * szSource)
{
	if (szSource == NULL)
		return false;

	UT_uint32 length = strlen(szSource) + 1;
	rszDest = static_cast<gchar *>(UT_calloc(length, sizeof(gchar)));

	if (!rszDest)
		return false;

	const gchar * o = szSource;
	gchar * n = rszDest;
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

bool UT_XML_cloneConvAmpersands(gchar *& rszDest, const gchar * szSource)
{
	if (szSource == NULL)
		return false;

	UT_uint32 length = strlen(szSource) + 1;
	rszDest = static_cast<gchar *>(UT_calloc(length, sizeof(gchar)));

	if (!rszDest)
		return false;

	const gchar * o = szSource;
	gchar * n = rszDest;
	while (*o != 0)
	{
		if (*o != '&')
		{
			*n = *o;
		} else {
			if (o[1] == '&') {
				*n++ = '&';
			}
			else *n = '_';
		}
		n++; o++;
	}

	return true;
}

/* This uses the clone no ampersands but dumps into a static buffer */
const gchar *UT_XML_transNoAmpersands(const gchar * szSource)
{
	static gchar *rszDestBuffer = NULL;
	static UT_uint32 iDestBufferLength = 0;

	if (szSource == NULL)
		return NULL;

	UT_uint32 length = strlen(szSource) + 1;
	if (length > iDestBufferLength) {
		if (rszDestBuffer && iDestBufferLength) {
			g_free(rszDestBuffer);
		}
		iDestBufferLength = 0;
		rszDestBuffer = static_cast<gchar *>(UT_calloc(length, sizeof(gchar)));

		if (!rszDestBuffer)
			return NULL;

		iDestBufferLength = length;
	}
	memset(rszDestBuffer, 0, iDestBufferLength);

	const gchar * o = szSource;
	gchar * n = rszDestBuffer;
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

/**
 *  XML cannot contain any control characters except \t, \n, \r, see bug 8565
 *  (http://www.w3.org/TR/REC-xml/#charsets)
 *
 *  This function removes any illegal characters and invalid utf-8 sequences.
 *
 *  @param str the string to modify in place
 *  @retval %true if the string was valid before. %false if it needed changes.
 */
bool UT_ensureValidXML(std::string & str)
{
  if(str.empty()) {
    return true;
  }

  bool isValid = true;
  const gchar* end = nullptr;
  if(!g_utf8_validate(str.c_str(), str.size(), &end)) {
    isValid = false;
  }

  const UT_Byte *ps = reinterpret_cast<const UT_Byte *>(str.c_str());
  while(*ps) {
    if(*ps < ' ' && *ps != '\t' && *ps != '\n' && *ps != '\r')  {
      isValid = false;
      break;
    }
    ++ps;
  }

  xxx_UT_DEBUGMSG(("isValid = %d\n", isValid));

  if (!isValid) {
    const UT_Byte *start = std::min(ps, reinterpret_cast<const UT_Byte*>(end));

    const UT_Byte * p = reinterpret_cast<const UT_Byte *>(str.c_str());	// gchar is signed...

    xxx_UT_DEBUGMSG(("p = %p, ps = %p, start = %p\n", p, ps, start));
    UT_uint32 len = str.size();

    int bytesInSequence = 0;
    int bytesExpectedInSequence = 0;

    std::string s(p, start);
    s.reserve(len);

    UT_uint32 k = start - p;

    for (; k < len; k++) {
      if (p[k] < 0x80) { // plain us-ascii part of latin-1

        // UT_Byte is unsigned char, hence p[k] always >= 0
        if(p[k] < ' ' /*&& p[k] >= 0*/ && p[k] != '\t' &&
           p[k] != '\n' && p[k] != '\r') {

        } else {
          s += p[k];
        }

        bytesInSequence = 0;
        bytesExpectedInSequence = 0;

      } else if ((p[k] & 0xf0) == 0xf0)	{ // lead byte in 4-byte surrogate pair

        UT_ASSERT_HARMLESS( UT_NOT_IMPLEMENTED );
        bytesExpectedInSequence = 4;
        bytesInSequence = 1;

      } else if ((p[k] & 0xe0) == 0xe0) { // lead byte in 3-byte sequence

        bytesExpectedInSequence = 3;
        bytesInSequence = 1;

      } else if ((p[k] & 0xc0) == 0xc0) {  // lead byte in 2-byte sequence

        bytesExpectedInSequence = 2;
        bytesInSequence = 1;

      } else if ((p[k] & 0x80) == 0x80) { // trailing byte in multi-byte sequence

        bytesInSequence++;
        if (bytesInSequence == bytesExpectedInSequence) { // final byte in multi-byte sequence
          for(UT_sint32 i = k - bytesInSequence + 1; i <= (UT_sint32)k; i++) {
            s += p[i];
          }

          bytesInSequence = 0;
          bytesExpectedInSequence = 0;
        }
      }
    }

    str = std::move(s);
  }

  return isValid;
}

/*! \fn bool UT_isValidXML(const char *s)
	 \param s The string of characters which is to be checked for XML-validity.
	 \retval TRUE if the characters are all valid for XML, FALSE if any one of them is not.

	 NB: this function also checks that the string is valid utf-8
*/
bool UT_isValidXML(const char *pString)
{
	if(!pString)
		return true;

	if(!g_utf8_validate(pString, -1, NULL))
		return false;

	const UT_Byte * s = reinterpret_cast<const UT_Byte *>(pString);
	
	while(*s)
	{
		if(*s < ' ' && *s != '\t' && *s != '\n' && *s != '\r')
		{
			return false;
		}
		
		++s;
	}

	return true;
}

/*!
    XML cannot contain any control characters except \t, \n, \r, see bug 8565
    (http://www.w3.org/TR/REC-xml/#charsets)
    
    This function removes any illegal characters and invalid utf-8 sequences.

    The return value of true indicates that the string was modified
*/
bool UT_validXML(char * pString)
{
	if(!pString)
		return false;

	UT_ASSERT(sizeof(gchar) == sizeof(UT_Byte));
	const UT_Byte * p = reinterpret_cast<const UT_Byte *>(pString);	// gchar is signed...
	
	bool bChanged = false;
	UT_uint32 len = strlen(pString);

	int bytesInSequence = 0;
	int bytesExpectedInSequence = 0;

	UT_String s;
	s.reserve(len);

	for (UT_uint32 k=0; k<len; k++)
	{
		if (p[k] < 0x80)						// plain us-ascii part of latin-1
		{
			if(bytesInSequence != 0)
				bChanged = true;

			// UT_Byte is unsigned char, hence p[k] always >= 0
			if(p[k] < ' ' /*&& p[k] >= 0*/ && p[k] != '\t' && p[k] != '\n' && p[k] != '\r')
			{
				bChanged = true;
			}
			else
				s += p[k];
				
			bytesInSequence = 0;
			bytesExpectedInSequence = 0;
		}
		else if ((p[k] & 0xf0) == 0xf0)			// lead byte in 4-byte surrogate pair
		{
			if(bytesInSequence != 0)
				bChanged = true;
			
			UT_ASSERT_HARMLESS( UT_NOT_IMPLEMENTED );
			bytesExpectedInSequence = 4;
			bytesInSequence = 1;
		}
		else if ((p[k] & 0xe0) == 0xe0)			// lead byte in 3-byte sequence
		{
			if(bytesInSequence != 0)
				bChanged = true;
			
			bytesExpectedInSequence = 3;
			bytesInSequence = 1;
		}
		else if ((p[k] & 0xc0) == 0xc0)			// lead byte in 2-byte sequence
		{
			if(bytesInSequence != 0)
				bChanged = true;
			
			bytesExpectedInSequence = 2;
			bytesInSequence = 1;
		}
		else if ((p[k] & 0x80) == 0x80)			// trailing byte in multi-byte sequence
		{
			bytesInSequence++;
			if (bytesInSequence == bytesExpectedInSequence)		// final byte in multi-byte sequence
			{
				for(UT_sint32 i = k - bytesInSequence + 1; i <= (UT_sint32)k; i++)
				{
					s += p[i];
				}
				
				bytesInSequence = 0;
				bytesExpectedInSequence = 0;
			}
		}
	}

	strncpy(pString, s.c_str(), s.length());

	// make sure we null-terminate
	pString[s.length()] = 0;
	return bChanged;
}

void UT_decodeUTF8string(const gchar * pString, UT_uint32 len, UT_GrowBuf * pResult)
{
	// decode the given string [ p[0]...p[len] ] and append to the given growbuf.

	UT_ASSERT(sizeof(gchar) == sizeof(UT_Byte));
	const UT_Byte * p = reinterpret_cast<const UT_Byte *>(pString);	// gchar is signed...

	int bytesInSequence = 0;
	int bytesExpectedInSequence = 0;
	gchar buf[5];

	for (UT_uint32 k=0; k<len; k++)
	{
		if (p[k] < 0x80)						// plain us-ascii part of latin-1
		{
			UT_ASSERT(bytesInSequence == 0);
			UT_UCSChar c = p[k];
			pResult->append(reinterpret_cast<UT_GrowBufElement *>(&c),1);
		}
		else if ((p[k] & 0xf0) == 0xf0)			// lead byte in 4-byte surrogate pair
		{
			// surrogate pairs are defined in section 3.7 of the
			// unicode standard version 2.0 as an extension
			// mechanism for rare characters in future extensions
			// of the unicode standard.
			UT_ASSERT(bytesInSequence == 0);
			bytesExpectedInSequence = 4;
			buf[bytesInSequence++] = p[k];
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
				UT_UCSChar c = g_utf8_get_char(buf);
				pResult->append(reinterpret_cast<UT_GrowBufElement *>(&c),1);
				bytesInSequence = 0;
				bytesExpectedInSequence = 0;
			}
		}
	}
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
   write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301 USA.  */

////////////////////////////////////////////////////////////////////////
//
//  UCS-2 string (UT_UCS2Char)
//
//  String is built of 16-bit units (words)
//
//  TODO: Is this really UCS-2 or UTF-16?
//  TODO:  meaning, does it support surrogates or is it intended to
//  TODO:  support them at any time in the future?
//  TODO: Correctly, UCS-2 does not support surrogates and UTF-16 does.
//  TODO: BUT Microsoft calls their native Unicode encoding UCS-2
//  TODO:  while it supports surrogates and is thus really UTF-16.
//  TODO: Surrogates are Unicode characters with codepoints above
//  TODO:  65535 which cannot therefore fit into a 2-byte word.
//  TODO: This means that TRUE UCS-2 is a single-word encoding and
//  TODO:  UTF-16 is a multi-word encoding.
//
//  NOTE: We shouldn't actually need 16-bit strings anymore since
//  NOTE:  AbiWord is now fully converted to using 32-bit Unicode
//  NOTE:  internally. The only possible needs for this is for
//  NOTE:  Windows GUI, filesystem and API functions where applicable;
//  NOTE:  and perhaps some file formats or external libraries
//
////////////////////////////////////////////////////////////////////////

// Don't ifdef out strlen since it's used by the MSWord importer...

// TODO is this really UCS-2 or UTF-16?
// TODO and are we using strlen for the number of 16-bit words
// TODO or the number of characters?
// TODO Because UTF-16 characters are sometimes expressed as 2 words

UT_uint32 UT_UCS2_strlen(const UT_UCS2Char * string)
{
	UT_uint32 i;

	for(i = 0; *string != 0; string++, i++)
		;

	return i;
}

#ifdef ENABLE_UCS2_STRINGS
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

	haystack = phaystack;
	needle = pneedle;

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
	return static_cast<UT_UCS2Char *>(haystack);
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
    case_entry * letter = static_cast<case_entry *>(bsearch(&c, &case_table, G_N_ELEMENTS(case_table),sizeof(case_entry),s_cmp_case));
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
    case_entry * letter = static_cast<case_entry *>(bsearch(&c, &case_table, G_N_ELEMENTS(case_table),sizeof(case_entry),s_cmp_case));
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

	haystack = phaystack;
	needle = pneedle;

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
	return static_cast<UT_UCS2Char *>(haystack);
 ret0:
	return 0;
}
/****************************************************************************/

UT_UCS2Char * UT_UCS2_strcpy(UT_UCS2Char * dest, const UT_UCS2Char * src)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_UCS2Char * d = dest;
	UT_UCS2Char * s = static_cast<UT_UCS2Char *>(src);

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
	unsigned char * s	= static_cast<unsigned char *>(src);

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
	*dest = static_cast<UT_UCS2Char *>(UT_calloc(length,sizeof(UT_UCS2Char)));
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

bool UT_UCS2_isupper(UT_UCS2Char c)
{
	if(c < 127)
		return isupper(c)!=0;

    case_entry * letter = static_cast<case_entry *>(bsearch(&c, &case_table, G_N_ELEMENTS(case_table),sizeof(case_entry),s_cmp_case));
    if(letter && letter->type == 1)
        return true;
    return false;
};

bool UT_UCS2_islower(UT_UCS2Char c)
{
	if(c < 127)
		return islower(c)!=0;

    case_entry * letter = static_cast<case_entry *>(bsearch(&c, &case_table, G_N_ELEMENTS(case_table),sizeof(case_entry),s_cmp_case));
    if(!letter || letter->type == 0)
        return true;
    return false;
};

bool UT_UCS2_isspace(UT_UCS2Char c)
{
	// the whitespace table is small, so use linear search
	for (UT_uint32 i = 0; i < G_N_ELEMENTS(whitespace_table); i++)
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
    UT_BidiCharType type = UT_bidiGetCharType(c);
    return (UT_BIDI_IS_LETTER(type) != 0);
};

bool UT_UCS2_isSentenceSeparator(UT_UCS2Char c)
{
	switch(c)
	{
	case '?': // fall-through
	case '!': // fall-through
	case '.':
		return true;

	default:
		return false;
	}
}

/* copies exactly n-chars from src to dest; NB! does not check for 00 i src
*/
UT_UCS2Char * UT_UCS2_strncpy(UT_UCS2Char * dest, const UT_UCS2Char * src, UT_uint32 n)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_UCS2Char * d = dest;
	UT_UCS2Char * s = static_cast<UT_UCS2Char *>(src);

	for (; d < static_cast<UT_UCS2Char *>(dest) + n;)
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

#endif


////////////////////////////////////////////////////////////////////////
//
//  UCS string (UT_UCSChar)
//
//  String is built of units based on UT_UCSChar, which used to be
//   UT_UCS2Char and is now UT_UCS4Char
//
////////////////////////////////////////////////////////////////////////

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
	bool result;
	switch (c)
	{
	case UCS_LQUOTE:
	case UCS_RQUOTE:
	case UCS_LDBLQUOTE:
	case UCS_RDBLQUOTE:
	case 0x201a:
	case 0x201e:
	case 0x2039:
	case 0x203a:
	case 0x300c:
	case 0x300d:
	case 0x300e:
	case 0x300f:
	case '\"':
	case '\'':
		result = true;
		break;
	default:
		result = false;
		break;
	}
	return (result);
}

////////////////////////////////////////////////////////////////////////
//
//  UCS-4 string
//
//  String is built of 32-bit units (longs)
//
//  NOTE: Ambiguity between UCS-2 and UTF-16 above makes no difference
//  NOTE:  in the case of UCS-4 and UTF-32 since they really are
//  NOTE:  identical
//
////////////////////////////////////////////////////////////////////////

bool UT_UCS4_isupper(UT_UCS4Char c)
{
	if(c < 127)
		return isupper(c)!=0;

    case_entry * letter = static_cast<case_entry *>(bsearch(&c, &case_table, G_N_ELEMENTS(case_table),sizeof(case_entry),s_cmp_case));
    if(letter && letter->type == 1)
        return true;
    return false;
}

bool UT_UCS4_islower(UT_UCS4Char c)
{
	if(c < 127)
		return islower(c)!=0;

    case_entry * letter = static_cast<case_entry *>(bsearch(&c, &case_table, G_N_ELEMENTS(case_table),sizeof(case_entry),s_cmp_case));
    if(!letter || letter->type == 0)
        return true;
    return false;
}

bool UT_UCS4_isspace(UT_UCS4Char c)
{
	// the whitespace table is small, so use linear search
	for (UT_uint32 i = 0; i < G_N_ELEMENTS(whitespace_table); i++)
	{
		if(whitespace_table[i].high < c)
			continue;
		if(whitespace_table[i].low <= c)
			return true;
		// if we got here, then low > c
		return false;
	}
	return false;
}

bool UT_UCS4_isalpha(UT_UCS4Char c)
{
    UT_BidiCharType type = UT_bidiGetCharType(c);
    return (UT_BIDI_IS_LETTER(type) != 0);
}

bool UT_UCS4_isSentenceSeparator(UT_UCS4Char c)
{
	switch(c)
	{
	case '?': // fall-through
	case '!': // fall-through
	case '.':
		return true;

	default:
		return false;
	}
}

bool UT_UCS4_isdigit(UT_UCS4Char c)
{
	if (c < 0x700) {
		for (unsigned int i=0; i < G_N_ELEMENTS(digits_table); i++) {
			if (c < digits_table[i].low) break;
			if (c <= digits_table[i].high)
				return true;
		}
	} else {
		ucs_range * rng = static_cast<ucs_range *>(bsearch(&c, &digits_table, 
			G_N_ELEMENTS(digits_table),sizeof(ucs_range),s_cmp_digits));
		if (rng) return true;
	}
	return false;
}

/* copies exactly n-chars from src to dest; NB! does not check for 00 i src
*/
UT_UCS4Char * UT_UCS4_strncpy(UT_UCS4Char * dest, const UT_UCS4Char * src, UT_uint32 n)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_UCSChar * d = dest;
	const UT_UCSChar * s = static_cast<const UT_UCS4Char *>(src);

	for (; d < static_cast<UT_UCS4Char *>(dest) + n;)
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

	haystack = static_cast<const UT_UCS4Char *>(phaystack);
	needle = static_cast<const UT_UCS4Char *>(pneedle);

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
	return const_cast<UT_UCS4Char *>(haystack);
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
    case_entry * letter = static_cast<case_entry *>(bsearch(&c, &case_table, G_N_ELEMENTS(case_table),sizeof(case_entry),s_cmp_case));
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
    case_entry * letter = static_cast<case_entry *>(bsearch(&c, &case_table, G_N_ELEMENTS(case_table),sizeof(case_entry),s_cmp_case));
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

	haystack = static_cast<const UT_UCS4Char *>(phaystack);
	needle = static_cast<const UT_UCS4Char *>(pneedle);

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
	return const_cast<UT_UCS4Char *>(haystack);
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

UT_uint32 UT_UCS4_strlen_as_char(const UT_UCS4Char * string)
{
	UT_uint32 i = 0;

	char d[4]; // assuming that any character can be coded with no more that 4 bytes.

	UT_Wctomb w(XAP_EncodingManager::get_instance()->getNative8BitEncodingName());

	while (*string != 0)
	  {
		int length;
		w.wctomb_or_fallback(d,length,*string++);
		i+=length;
	  }

	return i;
}

UT_UCS4Char * UT_UCS4_strcpy(UT_UCS4Char * dest, const UT_UCS4Char * src)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_UCS4Char * d = dest;
	const UT_UCS4Char * s = static_cast<const UT_UCS4Char *>(src);

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
	const char * s	= static_cast<const char *>(src);

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

UT_UCS4Char * UT_UCS4_strncpy_char(UT_UCS4Char * dest, const char * src, int n)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_UCS4Char * d 		= dest;
	const char * s	= static_cast<const char *>(src);

	static UT_UCS4_mbtowc m(XAP_EncodingManager::get_instance()->getNative8BitEncodingName());
	UT_UCS4Char wc;

	while (*s != 0 && n > 0)
	  {
		if(m.mbtowc(wc,*s))*d++=wc;
		s++;
		n--;
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

	UT_UCS4String ucs4str(src); // constructs a string from UTF-8 by default
	dest = UT_UCS4_strcpy(dest, ucs4str.ucs4_str());

	return dest;
}


char * UT_UCS4_strcpy_to_char(char * dest, const UT_UCS4Char * src)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	char * 			d = dest;
	const UT_UCS4Char * 	s = static_cast<const UT_UCS4Char *>(src);

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

char * UT_UCS4_strncpy_to_char(char * dest, const UT_UCS4Char * src, int n)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	char * 			d = dest;
	const UT_UCS4Char * 	s = static_cast<const UT_UCS4Char *>(src);

	UT_Wctomb w(XAP_EncodingManager::get_instance()->getNative8BitEncodingName());

	while (*s != 0 && n > 0)
	  {
		int length;
		w.wctomb_or_fallback(d,length,*s++, n);
		d+=length;
		n-=length;
	  }
	*d = 0;

	return dest;
}

bool UT_UCS4_cloneString(UT_UCS4Char ** dest, const UT_UCS4Char * src)
{
	UT_uint32 length = UT_UCS4_strlen(src) + 1;
	*dest = static_cast<UT_UCS4Char *>(UT_calloc(length,sizeof(UT_UCS4Char)));
	if (!*dest)
		return false;
	memmove(*dest,src,length*sizeof(UT_UCS4Char));

	return true;
}

bool UT_UCS4_cloneString_char(UT_UCS4Char ** dest, const char * src)
{
  UT_uint32 length = strlen(src) + 1;
  *dest = static_cast<UT_UCS4Char *>(UT_calloc(length,sizeof(UT_UCS4Char)));
  if (!*dest)
    return false;
  UT_UCS4_strcpy_char(*dest, src);
  
  return true;
}

static const char * s_pass_name (const char *& csstr, char end)
{
	const char * name_end = csstr;
	
	while (*csstr)
	{
		unsigned char u = static_cast<unsigned char>(*csstr);
		if (u & 0x80)
		{
			UT_UTF8Stringbuf::UCS4Char ucs4 = UT_UTF8Stringbuf::charCode (csstr);
			if (UT_UCS4_isspace (ucs4))
			{
				name_end = csstr;
				break;
			}
			while (static_cast<unsigned char>(*++csstr) & 0x80)
				;
			continue;
		}
		else if ((isspace (static_cast<int>(u))) || (*csstr == end))
		{
			name_end = csstr;
			break;
		}
		csstr++;
	}
	return name_end;
}


static const char * s_pass_value (const char *& csstr)
{
	const char * value_end = csstr;
	
	bool bQuoted = false;
	while (*csstr)
	{
		bool bSpace = false;
		unsigned char u = static_cast<unsigned char>(*csstr);
		if (u & 0x80)
		{
			UT_UTF8Stringbuf::UCS4Char ucs4 = UT_UTF8Stringbuf::charCode (csstr);
			
			if (!bQuoted)
				if (UT_UCS4_isspace (ucs4))
				{
					bSpace = true;
					break;
				}
			while (static_cast<unsigned char>(*++csstr) & 0x80)
				;
			if (!bSpace) 
				value_end = csstr;
			continue;
		}
		else if ((*csstr == '\'') || (*csstr == '"'))
		{
			bQuoted = (bQuoted ? false : true);
		}
		else if (*csstr == ';')
		{
			if (!bQuoted)
			{
				csstr++;
				break;
			}
		}
		else if (!bQuoted && isspace (static_cast<int>(u))) 
			bSpace = true;
		
		csstr++;
		if (!bSpace) 
			value_end = csstr;
	}
	return value_end;
}


static const char * s_pass_string (const char *& csstr_ptr)
{
	if (*csstr_ptr == 0) 
		return 0;
	
	const char * csstr = csstr_ptr;
	
	char quote = 0;
	
	if ((*csstr == '\'') || (*csstr == '"')) 
		quote = *csstr;

	bool valid = true;
	bool skip = false;

	while (true)
	{
		unsigned char u = static_cast<unsigned char>(*++csstr);
		
		if ((u & 0xc0) == 0x80) 
			continue; // trailing byte
		if (u == 0)
		{
			valid = false;
			break;
		}
		if (skip)
		{
			skip = false;
			continue;
		}
		if (*csstr == quote)
		{
			++csstr;
			break;
		}
		if (*csstr == '\\') 
			skip = true;
	}
	if (valid)
	{
		csstr_ptr = csstr;
		csstr--;
	}
	else
	{
		csstr = csstr_ptr;
	}
	return csstr; // points to end quote on success, and to start quote on failure
}

static void s_pass_whitespace (const char *& csstr)
{
	while (*csstr)
	{
		unsigned char u = static_cast<unsigned char>(*csstr);
		if (u & 0x80)
		{
			UT_UTF8Stringbuf::UCS4Char ucs4 = UT_UTF8Stringbuf::charCode (csstr);
			if (UT_UCS4_isspace (ucs4))
			{
				while (static_cast<unsigned char>(*++csstr) & 0x80) 
					;
				continue;
			}
		}
		else if (isspace (static_cast<int>(u)))
		{
			csstr++;
			continue;
		}
		break;
	}
}


void UT_parse_attributes(const char * attributes,
						 std::map<std::string, std::string> & map)
{
	if ( attributes == 0) 
		return;
	if (*attributes == 0) 
		return;

	const char * atstr = attributes;

	std::string name;
	std::string value;

	while (*atstr)
	{
		s_pass_whitespace (atstr);
		
		const char * name_start = atstr;
		const char * name_end   = s_pass_name (atstr, '=');
		
		if (*atstr != '=') 
			break; // whatever we have, it's not a name="value" pair
		if (name_start == name_end) 
			break; // ?? stray equals?

		name.clear();
		std::copy(name_start, name_end, name.begin());

		atstr++;
		
		if ((*atstr != '\'') && (*atstr != '"')) 
			break; // whatever we have, it's not a name="value" pair

		const char * value_start = atstr;
		const char * value_end   = s_pass_string (atstr);

		if (value_start == value_end) 
			break; // ?? no value...

		value_start++;

		value.clear();
		std::copy(value_start, value_end, value.begin());

		map[name] = value;
	}
}


void UT_parse_properties(const char * properties,
									std::map<std::string, std::string> & map)
{
	if ( properties == 0) 
		return;
	if (*properties == 0) 
		return;
	
	const char * csstr = properties;
	
	std::string name;
	std::string value;
	
	bool bSkip = false;
	
	while (*csstr)
	{
		if (bSkip)
		{
			if (*csstr == ';')
				bSkip = false;
			++csstr;
			continue;
		}
		s_pass_whitespace (csstr);
		
		const char * name_start = csstr;
		const char * name_end   = s_pass_name (csstr, ':');
		
		if (*csstr == 0) break; // whatever we have, it's not a "name:value;" pair
		if (name_start == name_end) // ?? stray colon?
		{
			bSkip = true;
			continue;
		}
		name.resize(name_end - name_start);
		std::copy(name_start, name_end, name.begin());
		
		s_pass_whitespace (csstr);
		if (*csstr != ':') // whatever we have, it's not a "name:value;" pair
		{
			bSkip = true;
			continue;
		}
		
		csstr++;
		s_pass_whitespace (csstr);
		
		if (*csstr == 0) 
			break; // whatever we have, it's not a "name:value;" pair
		
		const char * value_start = csstr;
		const char * value_end   = s_pass_value (csstr);
		
		if (value_start == value_end) // ?? no value...
		{
			bSkip = true;
			continue;
		}
		value.resize(value_end - value_start);
		std::copy(value_start, value_end, value.begin());
		
		map[name] = value;
	}
}

/*
 this one prints floating point value but using dot as fractional separator
 independent of the current locale's settings.
*/
const char* std_size_string(float f)
{
  static char string[10];
  int i=static_cast<int>(f);
  if(f-i<0.1)
	sprintf(string, "%d", i);
  else {
          int fr = int(10*(f-i));
	sprintf(string,"%d.%d", i,fr);
  };
  return string;
}

#ifndef TOOLKIT_WIN

UT_BidiCharType UT_bidiGetCharType(UT_UCS4Char c)
{
#ifndef NO_BIDI_SUPPORT
	return fribidi_get_type(c);
#else
	return UT_BIDI_LTR;
#endif
}

/*!
    pStrOut needs to contain space for len characters + terminating 0
*/
bool UT_bidiReorderString(const UT_UCS4Char * pStrIn, UT_uint32 len, UT_BidiCharType baseDir,
						  UT_UCS4Char * pStrOut)
{
	UT_return_val_if_fail( pStrIn && pStrOut, false );
	
#ifndef NO_BIDI_SUPPORT
	// this works around 8685; this should be left here, in fact any decent optimising
	// compiler should remove this code if the bug does not exist
	if(sizeof(FriBidiChar) > sizeof(UT_UCS4Char))
	{
		static FriBidiChar* pFBDC = NULL;
		static FriBidiChar* pFBDC2 = NULL;
		static UT_uint32 iFBDlen = 0;

		if(iFBDlen < len + 1)
		{
			delete [] pFBDC; delete [] pFBDC2;
			iFBDlen = 0;
			
			pFBDC = new FriBidiChar [len + 1];
			pFBDC2 = new FriBidiChar [len + 1];

			UT_return_val_if_fail( pFBDC && pFBDC2, false );

			iFBDlen = len + 1;
		}

		UT_uint32 i;
		for(i = 0; i < len; ++i)
		{
			pFBDC[i] = (FriBidiChar) pStrIn[i];
		}

		pFBDC[i] = 0;

		int iRet = fribidi_log2vis (pFBDC, len, &baseDir, pFBDC2, NULL, NULL, NULL);

		for(i = 0; i < len; ++i)
		{
			pStrOut[i] = (UT_UCS4Char) pFBDC2[i];
		}

		pStrOut[i] = 0;

		return iRet;
	}
	else
	{
		return (0 != fribidi_log2vis ((FriBidiChar *)pStrIn, len, &baseDir, (FriBidiChar*)pStrOut, NULL, NULL, NULL));
	}
	
#else
	if(!pStrIn || !*pStrIn)
		return true;

	UT_return_val_if_fail( pStrOut, false );

	UT_UCS4_strncpy(pStrOut, pStrIn, len);
	return true;
#endif
}

bool UT_bidiMapLog2Vis(const UT_UCS4Char * pStrIn, UT_uint32 len, UT_BidiCharType baseDir,
					   UT_uint32 *pL2V, UT_uint32 * pV2L, UT_Byte * pEmbed)
{
#ifndef NO_BIDI_SUPPORT
	// if this assert fails, we have a serious problem ...
	UT_ASSERT_HARMLESS( sizeof(UT_UCS4Char) == sizeof(FriBidiChar) );
	return (0 != fribidi_log2vis ((FriBidiChar *)pStrIn, len, &baseDir,
								  NULL, (FriBidiStrIndex*)pL2V, (FriBidiStrIndex*)pV2L, (FriBidiLevel*)pEmbed));
#else
	UT_return_val_if_fail( pL2V && pV2L && pEmbed, false );
	for(UT_uint32 i = 0; i < len; ++i)
	{
		pL2V[i] = i;
		pV2L[i] = i;
		pEmbed[i] = 0;
	}

	return true;
#endif
}

bool UT_bidiGetMirrorChar(UT_UCS4Char c, UT_UCS4Char &mc)
{
#ifndef NO_BIDI_SUPPORT
	return (0 != fribidi_get_mirror_char(c, (FriBidiChar*)&mc));
#else
	return false;
#endif
}


#endif
