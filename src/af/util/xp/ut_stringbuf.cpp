/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

// UT_Stringbuf.cpp

// Copyright (C) 2001 Mike Nordell <tamlin@algonet.se>
// Copyright (c) 2007 Hubert Figuiere <hub@figuiere.net>
// 
// This class is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This class is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
// 02110-1301 USA.
//
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <algorithm>

#include <libxml/uri.h>

#include <glib.h>

#include "ut_string.h"
#include "ut_stringbuf.h"
#include "ut_unicode.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// these classes keep zero terminated strings.
// if size() != 0, capacity() is always at least size() + 1.

//////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////
//
//  UTF-8 string: encoding is *always* UTF-8
//
////////////////////////////////////////////////////////////////////////


UT_UTF8Stringbuf::UT_UTF8Stringbuf () :
	m_psz(0),
	m_pEnd(0),
	m_strlen(0),
	m_buflen(0)
{
	// 
}

UT_UTF8Stringbuf::UT_UTF8Stringbuf (const UT_UTF8Stringbuf & rhs) :
	m_psz(0),
	m_pEnd(0),
	m_strlen(0),
	m_buflen(0)
{
	append (rhs);
}

UT_UTF8Stringbuf::UT_UTF8Stringbuf (const char * sz, size_t n /* == 0 => null-termination */) :
	m_psz(0),
	m_pEnd(0),
	m_strlen(0),
	m_buflen(0)
{
	append (sz, n);
}

UT_UTF8Stringbuf::~UT_UTF8Stringbuf ()
{
	clear ();
}

void UT_UTF8Stringbuf::operator=(const UT_UTF8Stringbuf & rhs)
{
	m_pEnd = m_psz;
	m_strlen = 0;
	append (rhs);
}

void UT_UTF8Stringbuf::assign (const char * sz, size_t n /* == 0 => null-termination */)
{
	m_pEnd = m_psz;
	m_strlen = 0;
	append (sz, n);
}

// returns 0 if invalid, or if end of string, i.e. 0
// technically it could differentiate, since UCS-4 is only 31-bit, but...
UT_UTF8Stringbuf::UCS4Char UT_UTF8Stringbuf::charCode (const char * str)
{
	if ( str == 0) return 0;
	if (*str == 0) return 0;

	const char * p = str;

	if ((*p & 0x80) == 0x00) // plain us-ascii part of latin-1
	{
		return (UCS4Char) (*p);
	}

	UCS4Char ret_code = 0;

	int bytesInSequence = 0;
	int bytesExpectedInSequence = 0;

	while (*p)
	{
		// 'continuing' octets:
		if ((*p & 0xc0) == 0x80) // trailing byte in multi-byte sequence
		{
			if (bytesInSequence == 0) break;
			bytesInSequence++;

			ret_code = (ret_code << 6) | (UCS4Char) (*p & 0x3f);

			if (bytesInSequence == bytesExpectedInSequence) break;

			p++;
			continue;
		}

		if (bytesInSequence) break;
		bytesInSequence++;

		/* 4,5,6-byte sequences may require > 2 bytes in UCS-4
		 */
		if ((*p & 0xfe) == 0xfc) // lead byte in 6-byte sequence
		{
			bytesExpectedInSequence = 6;
			ret_code = (UCS4Char) (*p & 0x01);
			p++;
			continue;
		}
		if ((*p & 0xfc) == 0xf8) // lead byte in 5-byte sequence
		{
			bytesExpectedInSequence = 5;
			ret_code = (UCS4Char) (*p & 0x03);
			p++;
			continue;
		}
		if ((*p & 0xf8) == 0xf0) // lead byte in 4-byte sequence
		{
			bytesExpectedInSequence = 4;
			ret_code = (UCS4Char) (*p & 0x07);
			p++;
			continue;
		}

		/* 1,2,3-byte sequences do not require > 2 bytes in UCS-4
		 */
		if ((*p & 0xf0) == 0xe0) // lead byte in 3-byte sequence
		{
			bytesExpectedInSequence = 3;
			ret_code = (UCS4Char) (*p & 0x0f);
			p++;
			continue;
		}
		if ((*p & 0xe0) == 0xc0) // lead byte in 2-byte sequence
		{
			bytesExpectedInSequence = 2;
			ret_code = (UCS4Char) (*p & 0x1f);
			p++;
			continue;
		}

		ret_code = 0;
		break; // invalid byte - not UTF-8
	}
	if (bytesInSequence != bytesExpectedInSequence) ret_code = 0;

	return ret_code;
}

void UT_UTF8Stringbuf::append (const char * sz, size_t n /* == 0 => null-termination */)
{
	if (sz == 0) 
		return;
	if (!grow ((n?n:strlen(sz)) + 1)) 
		return;

	const char * p = sz;
	char buf[6];
	int bytesInSequence = 0;
	int bytesExpectedInSequence = 0;
	size_t np = 0;

	while ((!n && *p) || (np < n))
	{
		if ((*p & 0x80) == 0x00) // plain us-ascii part of latin-1
		{
			if (bytesInSequence) break;

			*m_pEnd++ = *p;
			*m_pEnd = 0;
			m_strlen++;

			p++;
			np++;
			continue;
		}

		// 'continuing' octets:
		if ((*p & 0xc0) == 0x80) // trailing byte in multi-byte sequence
		{
			if (bytesInSequence == 0) break;

			buf[bytesInSequence++] = *p;
			if (bytesInSequence == bytesExpectedInSequence)
			{
				for (int b = 0; b < bytesInSequence; b++) *m_pEnd++ = buf[b];
				*m_pEnd = 0;
				m_strlen++;
				bytesInSequence = 0;
				bytesExpectedInSequence = 0;
			}

			p++;
			np++;
			continue;
		}

		if (bytesInSequence) break;

		buf[bytesInSequence++] = *p;

		/* 4,5,6-byte sequences may require > 2 bytes in UCS-4
		 */
		if ((*p & 0xfe) == 0xfc) // lead byte in 6-byte sequence
		{
			bytesExpectedInSequence = 6;
			p++;
			np++;
			continue;
		}
		if ((*p & 0xfc) == 0xf8) // lead byte in 5-byte sequence
		{
			bytesExpectedInSequence = 5;
			p++;
			np++;
			continue;
		}
		if ((*p & 0xf8) == 0xf0) // lead byte in 4-byte sequence
		{
			bytesExpectedInSequence = 4;
			p++;
			np++;
			continue;
		}

		/* 1,2,3-byte sequences do not require > 2 bytes in UCS-4
		 */
		if ((*p & 0xf0) == 0xe0) // lead byte in 3-byte sequence
		{
			bytesExpectedInSequence = 3;
			p++;
			np++;
			continue;
		}
		if ((*p & 0xe0) == 0xc0) // lead byte in 2-byte sequence
		{
			bytesExpectedInSequence = 2;
			p++;
			np++;
			continue;
		}

		break; // invalid byte - not UTF-8
	}
}

void UT_UTF8Stringbuf::append (const UT_UTF8Stringbuf & rhs)
{
	if (grow (rhs.byteLength () + 1) && rhs.data() != NULL)
	{
		memcpy (m_pEnd, rhs.data (), rhs.byteLength ());
		m_strlen += rhs.utf8Length ();
		m_pEnd = m_pEnd + rhs.byteLength ();
		*m_pEnd = 0;
	}
}

void UT_UTF8Stringbuf::appendUCS4 (const UT_UCS4Char * sz, size_t n /* == 0 => null-termination */)
{
	size_t bytelength = 0;
	size_t i;

	if (!sz || (!n && !*sz))
		return;

	/* The vast majority of calls to appendUCS4 pass in
	   1 for n, so we can halve the number of calls to g_unichar_to_utf8
	   (in most cases) by caching the first byte length. */
	int iCache = 0;
	
	for (i = 0; (i < n) || (n == 0); i++)
	{
		if((0 == sz[i]) && (0 == n))
			break;
		int seql = UT_Unicode::UTF8_ByteLength (sz[i]);
		if(i == 0)
			iCache = seql;

		if (seql < 0) 
			continue; // not UCS-4 !!
		if (seql == 0) 
			break; // end-of-string?
		bytelength += static_cast<size_t>(seql);
	}
	if(bytelength == 0)
		return;
	if (!grow (bytelength + 1)) return;

	for (i = 0; (i < n) || (n == 0); i++)
	{
		if((0 == sz[i]) && (0 == n))
			break;
		int seql;
		if(i == 0)
			seql = iCache;
		else
			seql = UT_Unicode::UTF8_ByteLength (sz[i]);

		if (seql < 0) 
			continue; // not UCS-4 !!
		if (seql == 0) 
			break; // end-of-string?
		UT_Unicode::UCS4_to_UTF8 (m_pEnd, bytelength, sz[i]);
		m_strlen++;
	}
	*m_pEnd = 0;
}

void UT_UTF8Stringbuf::appendUCS2 (const UT_UCS2Char * sz, size_t n /* == 0 => null-termination */)
{
	size_t bytelength = 0;
	size_t i;
	for (i = 0; (i < n) || (n == 0); i++)
	{
		if (sz[i]==0 && n==0) break;
		int seql = UT_Unicode::UTF8_ByteLength ((UT_UCS4Char)sz[i]);
		if (seql < 0) 
			continue; // not UCS-4 !!
		if (seql == 0) 
			break; // end-of-string?
		bytelength += static_cast<size_t>(seql);
	}

	if (!grow (bytelength + 1)) return;

	for (i = 0; (i < n) || (n == 0); i++)
	{
		if (sz[i]==0 && n==0) break;
		int seql = UT_Unicode::UTF8_ByteLength ((UT_UCS4Char)sz[i]);
		if (seql < 0) 
			continue; // not UCS-4 !!
		if (seql == 0) 
			break; // end-of-string?
		UT_Unicode::UCS4_to_UTF8 (m_pEnd, bytelength, (UT_UCS4Char)sz[i]);
		m_strlen++;
	}
	*m_pEnd = 0;
}

/* replaces <str1> with <str2> in the current string
 */
void UT_UTF8Stringbuf::escape (const UT_UTF8String & utf8_str1,
							   const UT_UTF8String & utf8_str2)
{
	size_t diff = 0;
	size_t len1 = utf8_str1.byteLength ();
	size_t len2 = utf8_str2.byteLength ();

	const char * str1 = utf8_str1.utf8_str ();
	const char * str2 = utf8_str2.utf8_str ();

	if (len2 > len1)
	{
		diff = len2 - len1;

		size_t incr = 0;

		char * ptr = m_psz;
		while (ptr + len1 <= m_pEnd)
		{
			if (memcmp (ptr, str1, len1) == 0)
			{
				incr += diff;
				ptr += len1;
			}
			else
			{
				++ptr;
			}
		}
		if (!grow (incr)) return;
	}
	else
	{
		diff = len1 - len2;
	}

	char * ptr = m_psz;
	while (ptr + len1 <= m_pEnd)
	{
		if (memcmp (ptr, str1, len1) == 0)
		{
			if (diff)
			{
				if (len2 > len1)
				{
					memmove (ptr + diff, ptr, m_pEnd - ptr + 1);
					m_pEnd += diff;
				}
				else
				{
					memmove (ptr, ptr + diff, m_pEnd - (ptr + diff) + 1);
					m_pEnd -= diff;
				}
			}
			memcpy (ptr, str2, len2);
			ptr += len2;
			m_strlen += utf8_str2.length () - utf8_str1.length ();
		}
		else
		{
			++ptr;
		}
	}
}

/* FIXME -- these functions assume that &, <, > and " cannot appear in
 *          multi-byte utf8 sequence -- I do not think that holds
 *
 *          Also, the decode function should handle other & tokens
 *
 *          Should use glib to traverse these strings
 */
void UT_UTF8Stringbuf::decodeXML ()
{
	if (!m_psz)
		return;
	
	size_t shrink = 0;
	char * p_src = m_psz;
	char * p_dst = m_psz;
	
	while (p_src < m_pEnd && *p_src)
	{
		if(*p_src == '&')
		{
			if (!strncmp (p_src+1, "amp;", 4))
			{
				*p_dst++ = '&';
				p_src += 5;
				shrink += 4;
				continue;
			}
			else if (!strncmp (p_src+1, "lt;", 3))
			{
				*p_dst++ = '<';
				p_src += 4;
				shrink += 3;
				continue;
			}
			else if (!strncmp (p_src+1, "gt;", 3))
			{
				*p_dst++ = '>';
				p_src += 4;
				shrink += 3;
				continue;
			}
			else if (!strncmp (p_src+1, "quot;", 5))
			{
				*p_dst++ = '"';
				p_src += 6;
				shrink += 5;
				continue;
			}
		}

		*p_dst = *p_src;
		
		p_dst++;
		p_src++;
	}

	*p_dst = 0;
	m_pEnd -= shrink;
}

/* escapes '<', '>', '\"' and '&' in the current string
 */
void UT_UTF8Stringbuf::escapeXML ()
{
	size_t incr = 0;

	char * ptr = m_psz;
	while (ptr < m_pEnd)
		{
			if ((*ptr == '<') || (*ptr == '>')) incr += 3;
			else if (*ptr == '&') incr += 4;
			else if (*ptr == '"') incr += 5;
			ptr++;
		}
	bool bInsert = grow (incr);

	ptr = m_psz;
	while (ptr < m_pEnd)
		{
			if (*ptr == '<')
				{
					if (bInsert)
						{
							*ptr++ = '&';
							insert (ptr, "lt;", 3);
						}
					else *ptr++ = '?';
				}
			else if (*ptr == '>')
				{
					if (bInsert)
						{
							*ptr++ = '&';
							insert (ptr, "gt;", 3);
						}
					else *ptr++ = '?';
				}
			else if (*ptr == '&')
				{
					if (bInsert)
						{
							*ptr++ = '&';
							insert (ptr, "amp;", 4);
						}
					else *ptr++ = '?';
				}
			else if (*ptr == '"')
				{
					if (bInsert)
						{
							*ptr++ = '&';
							insert (ptr, "quot;", 5);
						}
					else *ptr++ = '?';
				}
			else ptr++;
		}
}

/*
   this function escapes the string to provide for conformity with
   http://www.w3.org/TR/xlink/#link-locators, section 5.4

   Just use libxml and hope for the best.
*/
void UT_UTF8Stringbuf::escapeURL ()
{
	if(!m_psz || !*m_psz)
		return;

	xmlChar * uri = xmlURIEscape(BAD_CAST m_psz);
	if(uri) {
		assign((gchar*)uri);
		xmlFree(uri);
	}
}

/* decode %xx encoded characters
 */

static UT_uint32 s_charCode_to_hexval(UT_UCS4Char c)
{
	if(c >= 0x30 && c <= 0x39)
		return c - 0x30;
	else if(c >= 0x41 && c <= 0x46)
		return c - 0x41 + 10;
	else if(c >= 0x61 && c <= 0x66)
		return c - 0x61 + 10;

	UT_return_val_if_fail( UT_SHOULD_NOT_HAPPEN, 0 );
}

void UT_UTF8Stringbuf::decodeURL()
{
	if(!m_psz || !*m_psz)
		return;

	char * buff = (char*)g_try_malloc(byteLength() + 1);
	UT_return_if_fail( buff );
	buff[0] = 0;

	UTF8Iterator J(this);
	const char * ptr = J.current();
	UT_UCS4Char c = charCode(J.current());

	char utf8cache[7]; utf8cache[6] = 0;
	UT_uint32 iCachePos = 0;
	UT_uint32 iCacheNeeded = 0;
	

	while (c != 0)
	{
		if(c == '%')
		{
			J.advance();
			UT_UCS4Char b1 = charCode(J.current());
			J.advance();
			UT_UCS4Char b2 = charCode(J.current());
			J.advance();

			if(isalnum(b1) && isalnum(b2))
			{
				b1 = s_charCode_to_hexval(b1);
				b2 = s_charCode_to_hexval(b2);
					
				UT_UCS4Char code = ((b1 << 4)& 0xf0) | (b2 & 0x0f);

				if(iCacheNeeded == 0)
				{
					// we start new utf8 sequence in the cache
					if ((code & 0x80) == 0)         iCacheNeeded = 1;
					else if ((code & 0xe0) == 0xc0) iCacheNeeded = 2;
					else if ((code & 0xf0) == 0xe0) iCacheNeeded = 3;
					else if ((code & 0xf8) == 0xf0) iCacheNeeded = 4;
					else if ((code & 0xfc) == 0xf8) iCacheNeeded = 5;
					else if ((code & 0xfe) == 0xfc) iCacheNeeded = 6;

					utf8cache[0] = (char) code;
					utf8cache[iCacheNeeded] = 0; // make sure the sequence will be terminated
					iCachePos++;
				}
				else
				{
					// append to our cache
					utf8cache[iCachePos++] = (char) code;
				}

				if(iCacheNeeded == 0 && (code >= 0x7f && code <= 0xff))
				{
					// the present character is not a valid start of utf8 sequence --
					// this is almost certainly a character from the extended ASCII set
					// which was encoded directly according to the RFC 1738 scheme, we
					// just append it
					
					size_t iLenBuff = strlen(buff);
					size_t iLenLeft = byteLength() - iLenBuff;
					
					char * p = buff + iLenBuff;
					UT_Unicode::UCS4_to_UTF8(p, iLenLeft, code);
 
					// we need to null-terminate
					*p = 0;
				}
				
				if(iCacheNeeded && iCacheNeeded <= iCachePos)
				{
					UT_ASSERT_HARMLESS( iCacheNeeded == iCachePos );
					
					// append the cache to our buffer
					UT_uint32 iLenBuff = strlen(buff);
					char * p = buff + iLenBuff;
					strcat(p, utf8cache);

					iCacheNeeded = iCachePos = 0;
				}
			}
			else
			{
				// this should not happen in encoded url and so we will ignore this token;
				// if we are in the middle of utf8 sequence; we will reset it
				iCacheNeeded = iCachePos = 0;
			}
		}
		else
		{
			J.advance(); // advance here, for the sake of the else clause below
			
			if(iCacheNeeded > iCachePos)
			{
				// we are processing a utf sequence, so just append this byte to our cache
				utf8cache[iCachePos++] = (char) c;
			}
			else
			{
				const char * p = J.current();
				UT_uint32 iLen = p ? p - ptr : strlen(ptr);
				strncat(buff, ptr, iLen);
			}
		}

		ptr = J.current();
		c = charCode(J.current());
	}
	
	assign(buff);
	g_free(buff);
}

/* translates the current string to MIME "quoted-printable" format
 */
void UT_UTF8Stringbuf::escapeMIME ()
{
	static const char hex[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
	static const char * s_eol = "=\r\n";

	if (m_strlen == 0) return;

	size_t bytes = 0;
	char * ptr = m_psz;
	while (*ptr)
		{
			char c = *ptr++;
			unsigned char u = static_cast<unsigned char>(c);

			if ((c == '\r') || (c == '\n') || (c == '=') || (u & 0x80)) bytes += 2;
		}
	if (bytes)
		{
			if (!grow (bytes)) return;

			char * pOld = m_pEnd;
			char * pNew = m_pEnd + bytes;

			while (pOld >= m_psz)
				{
					char c = *pOld--;
					unsigned char u = static_cast<unsigned char>(c);

					if ((u & 0x80) || (c == '\r') || (c == '\n') || (c == '='))
						{
							*pNew-- = hex[ u       & 0x0f];
							*pNew-- = hex[(u >> 4) & 0x0f];
							*pNew-- = '=';
						}
					else *pNew-- = c;
				}
			m_pEnd += bytes;
			m_strlen = m_pEnd - m_psz;
		}

	size_t length = 0;
	ptr = m_psz;
	while (true)
		{
			if (*ptr == 0)
				{
					if (length)
						{
							size_t offset = ptr - m_psz;
							if (grow (3))
								{
									ptr = m_psz + offset;
									insert (ptr, s_eol, 3);
								}
						}
					break;
				}
			if (length >= 70)
				{
					size_t offset = ptr - m_psz;
					if (grow (3))
						{
							ptr = m_psz + offset;
							insert (ptr, s_eol, 3);
						}
					length = 0;
				}

			if (*ptr == '=')
				{
					ptr += 3;
					length += 3;
				}
			else
				{
					ptr++;
					length++;
				}
		}
}

UT_UTF8Stringbuf * UT_UTF8Stringbuf::lowerCase ()
{
	if(!byteLength())
		return NULL;

	UT_UTF8Stringbuf * n = new UT_UTF8Stringbuf();
	UT_return_val_if_fail(n, NULL);
	
	UTF8Iterator s(this);
	UT_UCS4Char c = charCode(s.current());

	while(c)
	{
		UT_UCS4Char l = UT_UCS4_tolower(c);
		n->appendUCS4(&l,1);
		c = charCode(s.advance());
	}

	return n;
}

void UT_UTF8Stringbuf::clear ()
{
	if (m_psz) g_free (m_psz);
	m_psz = 0;
	m_pEnd = 0;
	m_strlen = 0;
	m_buflen = 0;
}

void UT_UTF8Stringbuf::insert (char *& ptr, const char * str, size_t utf8length)
{
	if ( str == 0) return;
	if (*str == 0) return;

	if ((ptr < m_psz) || (ptr > m_pEnd)) return;

	char * orig_buf = m_psz;
	char * orig_ptr = ptr;

	size_t length = static_cast<size_t>(strlen(str));

	if (!grow (length)) return;

	ptr = m_psz + (orig_ptr - orig_buf);

	memmove (ptr + length, ptr, (m_pEnd - ptr) + 1);
	memcpy (ptr, str, length);

	ptr += length;
	m_pEnd += length;
	m_strlen += utf8length;
}

void UT_UTF8Stringbuf::reserve(size_t n)
{
	grow(n);
}

bool UT_UTF8Stringbuf::grow (size_t length)
{
	if (length + 1 <= (m_buflen - (m_pEnd - m_psz))) return true;

	if (m_psz == 0)
	{
		if (length == 0) return true;
		m_psz = static_cast<char *>(g_try_malloc(length));
		if (m_psz == 0) return false;
		m_strlen = 0;
		m_buflen = length;
		m_pEnd = m_psz;
		*m_pEnd = 0;
		return true;
	}

	size_t new_length = length + (m_pEnd - m_psz) + 1;
	size_t end_offset = m_pEnd - m_psz;

	char * more = static_cast<char *>(g_try_realloc(static_cast<void *>(m_psz), new_length));
	if (more == 0) return false;
	m_psz = more;
	m_pEnd = m_psz + end_offset;
	m_buflen = new_length;
	return true;
}

UT_UTF8Stringbuf::UTF8Iterator::UTF8Iterator (const UT_UTF8Stringbuf * strbuf) :
	m_strbuf(strbuf),
	m_utfbuf(0),
	m_utfptr(0)
{
	sync ();
}

UT_UTF8Stringbuf::UTF8Iterator::~UTF8Iterator ()
{
	// 
}

void UT_UTF8Stringbuf::UTF8Iterator::operator=(const char * position)
{
	if (!sync ()) return;
	if (static_cast<unsigned>(position- m_utfbuf) > m_strbuf->byteLength ())
	{
		m_utfptr = m_utfbuf + m_strbuf->byteLength ();
	}
	else
	{
		m_utfptr = position;
	}
}

const char * UT_UTF8Stringbuf::UTF8Iterator::current ()
{
	if (!sync ()) return 0;
	if ((*m_utfptr & 0xc0) == 0x80) return 0; // oops - a 'continuing' byte 
	return m_utfptr;
}

const char * UT_UTF8Stringbuf::UTF8Iterator::start ()
{
	if (!sync ()) 
		return 0;
	return m_utfbuf;
}

const char * UT_UTF8Stringbuf::UTF8Iterator::end ()
{
	if (!sync ()) 
		return 0;
	return m_utfbuf + m_strbuf->byteLength ();
}

const char * UT_UTF8Stringbuf::UTF8Iterator::advance ()
{
	if (!sync ()) 
		return 0;
	if (*m_utfptr == 0) 
		return 0;
	do {
		m_utfptr++;
	} while ((*m_utfptr & 0xc0) == 0x80); // a 'continuing' byte 
	return m_utfptr;
}

const char * UT_UTF8Stringbuf::UTF8Iterator::retreat ()
{
	if (!sync ()) 
		return 0;
	if (m_utfptr == m_utfbuf) 
		return 0;
	do {
		m_utfptr--;
	} while ((*m_utfptr & 0xc0) == 0x80); // a 'continuing' byte 
	return m_utfptr;
}

// returns false only if there is no string data
bool UT_UTF8Stringbuf::UTF8Iterator::sync ()
{
	if (m_strbuf == 0) 
		return false;

	const char * utf8_buffer = m_strbuf->data ();
	if (utf8_buffer == 0)
	{
		m_utfbuf = 0;
		m_utfptr = 0;
		return false;
	}

	size_t utf8_length = m_strbuf->byteLength ();

	/* note that this doesn't guarantee that m_utfptr points to the
	 * start of UTF-8 char sequence
	 */
	if (static_cast<unsigned>(m_utfptr- m_utfbuf) > utf8_length)
	{
		m_utfptr = utf8_buffer + utf8_length;
	}
	else
	{
		m_utfptr = utf8_buffer + (m_utfptr - m_utfbuf);
	}
	m_utfbuf = utf8_buffer;

	return true;
}



