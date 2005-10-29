/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

// UT_Stringbuf.cpp

// Copyright (C) 2001 Mike Nordell <tamlin@algonet.se>
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
// 02111-1307, USA.
//
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "ut_string.h"
#include "ut_stringbuf.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// these classes keep zero terminated strings.
// if size() != 0, capacity() is always at least size() + 1.

//////////////////////////////////////////////////////////////////

static inline
void my_ut_swap(size_t & a, size_t & b)
	{ size_t t = a; a = b; b = t; }

static inline
void my_ut_swap(char *& a, char *& b)
	{ char * t = a; a = b; b = t; }

static inline
void my_ut_swap(UT_UCS2Char *& a, UT_UCS2Char *& b)
	{ UT_UCS2Char * t = a; a = b; b = t; }

static inline
void my_ut_swap(UT_UCS4Char *& a, UT_UCS4Char *& b)
	{ UT_UCS4Char * t = a; a = b; b = t; }

//////////////////////////////////////////////////////////////////

static const float g_rGrowBy = 1.5;

static inline size_t priv_max(size_t a, size_t b)
{
	return a < b ? b : a;
}

////////////////////////////////////////////////////////////////////////
//
//  8-bit string
//
//  String is built of 8-bit units (bytes)
//  Encoding could be any single-byte or multi-byte encoding
//
////////////////////////////////////////////////////////////////////////

UT_Stringbuf::UT_Stringbuf()
:	m_psz(0),
	m_pEnd(0),
	m_size(0)
{
}

UT_Stringbuf::UT_Stringbuf(const UT_Stringbuf& rhs)
	:	m_psz(new char_type[rhs.capacity()]),
	m_pEnd(m_psz + rhs.size()),
	m_size(rhs.capacity())
{
	copy(m_psz, rhs.m_psz, rhs.capacity());
}

UT_Stringbuf::UT_Stringbuf(const char_type* sz, size_t n)
:	m_psz(new char_type[n+1]),
	m_pEnd(m_psz + n),
	m_size(n+1)
{
	copy(m_psz, sz, n);
	m_psz[n] = 0;
}

UT_Stringbuf::~UT_Stringbuf()
{
	clear();
}


void UT_Stringbuf::operator=(const UT_Stringbuf& rhs)
{
	if (this != &rhs)
	{
		clear();
		assign(rhs.m_psz, rhs.size());
	}
}

void UT_Stringbuf::assign(const char_type* sz, size_t n)
{
	if (n)
	{
		if (n >= capacity())
		{
			grow_nocopy(n);
		}
		copy(m_psz, sz, n);
		m_psz[n] = 0;
		m_pEnd = m_psz + n;
	} else {
		clear();
	}
}

void UT_Stringbuf::append(const char_type* sz, size_t n)
{
	if (!n)
	{
		return;
	}
	if (!capacity())
	{
		assign(sz, n);
		return;
	}
	const size_t nLen = size();
	grow_copy(nLen + n);
	copy(m_psz + nLen, sz, n);
	m_psz[nLen + n] = 0;
	m_pEnd += n;
}

void UT_Stringbuf::append(const UT_Stringbuf& rhs)
{
	append(rhs.m_psz, rhs.size());
}

void UT_Stringbuf::swap(UT_Stringbuf& rhs)
{
	my_ut_swap(m_psz , rhs.m_psz );
	my_ut_swap(m_pEnd, rhs.m_pEnd);
	my_ut_swap(m_size, rhs.m_size);
}

void UT_Stringbuf::clear()
{
	if (m_psz)
	{
		delete[] m_psz;
		m_psz = 0;
		m_pEnd = 0;
		m_size = 0;
	}
}

void UT_Stringbuf::reserve(size_t n)
{
	grow_nocopy(n);
}

void UT_Stringbuf::grow_nocopy(size_t n)
{
	grow_common(n, false);
}

void UT_Stringbuf::grow_copy(size_t n)
{
	grow_common(n, true);
}

void UT_Stringbuf::grow_common(size_t n, bool bCopy)
{
	++n;	// allow for zero termination
	if (n > capacity())
	{
		const size_t nCurSize = size();
		n = priv_max(n, static_cast<size_t>(nCurSize * g_rGrowBy));
		char_type* pNew = new char_type[n];
		if (bCopy && m_psz)
		{
			copy(pNew, m_psz, size() + 1);
		}
		delete[] m_psz;
		m_psz  = pNew;
		m_pEnd = m_psz + nCurSize;
		m_size = n;
	}
}

void UT_Stringbuf::copy(char_type* pDest, const char_type* pSrc, size_t n)
{
	if (pDest && pSrc && n)
		memcpy(pDest, pSrc, n * sizeof(char_type));
}


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
	if (sz == 0) return;
	if (!grow (strlen (sz) + 1)) return;

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
	if (grow (rhs.byteLength () + 1))
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
	for (i = 0; (i < n) || (n == 0); i++)
	{
		int seql = UT_UCS4Stringbuf::UTF8_ByteLength (sz[i]);
		if (seql < 0) continue; // not UCS-4 !!
		if (seql == 0) break; // end-of-string?
		bytelength += static_cast<size_t>(seql);
	}
	if (!grow (bytelength + 1)) return;

	for (i = 0; (i < n) || (n == 0); i++)
	{
		int seql = UT_UCS4Stringbuf::UTF8_ByteLength (sz[i]);
		if (seql < 0) continue; // not UCS-4 !!
		if (seql == 0) break; // end-of-string?
		UT_UCS4Stringbuf::UCS4_to_UTF8 (m_pEnd, bytelength, sz[i]);
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
		int seql = UT_UCS4Stringbuf::UTF8_ByteLength ((UT_UCS4Char)sz[i]);
		if (seql < 0) continue; // not UCS-4 !!
		if (seql == 0) break; // end-of-string?
		bytelength += static_cast<size_t>(seql);
	}
	if (!grow (bytelength + 1)) return;

	for (i = 0; (i < n) || (n == 0); i++)
	{
		int seql = UT_UCS4Stringbuf::UTF8_ByteLength ((UT_UCS4Char)sz[i]);
		if (seql < 0) continue; // not UCS-4 !!
		if (seql == 0) break; // end-of-string?
		UT_UCS4Stringbuf::UCS4_to_UTF8 (m_pEnd, bytelength, (UT_UCS4Char)sz[i]);
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

/* escapes '<', '>' & '&' in the current string
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
*/
void UT_UTF8Stringbuf::escapeURL ()
{
	if(!m_psz || !*m_psz)
		return;
	
	// now work out how many exra characters we will need
	// need to do this first of all, since growing the string will invalidate all pointers
	UTF8Iterator I(this);
	UT_UCS4Char c;
	UT_uint32 iIncrease = 0;

	for(c = charCode(I.current()); c != 0; c = charCode(I.advance()))
	{
		UT_sint32 iByteLen = UT_UCS4Stringbuf::UTF8_ByteLength(c);

		if(iByteLen > 1)
			iIncrease += iByteLen;
		else if(c <= 0x20 || c > 0x7e || (!isalnum(c) && !strchr("$-_.+!*'(),", c)))
			iIncrease += 2;
	}

	grow(iIncrease);
	
	UT_uint32 iScheme = 0;
	if(!UT_strnicmp(m_psz, "ftp://", 6))            iScheme = 1;
	else if(!UT_strnicmp(m_psz, "http://", 7))      iScheme = 2;
	else if(!UT_strnicmp(m_psz, "gopher://", 9))    iScheme = 3;
	else if(!UT_strnicmp(m_psz, "mailto:", 7))      iScheme = 4;
	else if(!UT_strnicmp(m_psz, "news:", 5))      iScheme = 5;
	else if(!UT_strnicmp(m_psz, "nntp://", 7))      iScheme = 6;
	else if(!UT_strnicmp(m_psz, "telnet://", 9))    iScheme = 7;
	else if(!UT_strnicmp(m_psz, "wais://", 7))      iScheme = 8;
	else if(!UT_strnicmp(m_psz, "file://", 7))      iScheme = 9;
	else if(!UT_strnicmp(m_psz, "prospero://", 11)) iScheme = 10;

	// now we parse the string into its constituent parts
	char * p = strstr(m_psz, "://");
	char * schm = NULL;
	char * user = NULL;
	char * pswd = NULL;
	char * host = NULL;
	char * port = NULL;
	char * last_quest = NULL;
	char * last_hash = NULL;
	char * last_slash = NULL;
	
	if(p)
	{
		user = p + 3;
		schm = user;
		p = strchr(p+3, '/');
	}
	else if(iScheme == 4)
	{
		p = m_psz + 7;
	}
	else if(iScheme == 5)
	{
		p = m_psz + 5;
	}
	
	char * urlpath = p ? p : m_psz;

	if(urlpath != m_psz && iScheme != 4 && iScheme != 5)
	{
		*urlpath = 0;
		char * at = strrchr(user, '@');

		if(!at)
		{
			user = NULL;
		}
		else
		{
			host = at + 1;
			port = strchr(host, ':');
			if(port) port++;

			*at = 0;

			pswd = strchr(user, ':');
			if(pswd) pswd++;

			*at = '@';
		}

		*urlpath = '/';
	}

	// find out the last /, ? and # -- we need these to work out if ?#& should be escaped
	// in http or not
	last_slash = strrchr(urlpath, '/');
	last_quest = strrchr(urlpath, '?');
	last_hash = strrchr(urlpath, '#');

	if(last_quest < last_slash) last_quest = NULL; // this is not a query questionmark
	if(last_hash < last_slash)  last_hash  = NULL;
	char buff[30];
	UTF8Iterator J(this);
	
	for(c = charCode(J.current()); c != 0; c = charCode(J.advance()))
	{
		char * p = (char*) J.current();
		UT_sint32 iByteLen = UT_UCS4Stringbuf::UTF8_ByteLength(c);
		
		if (iByteLen > 1) // mutlibyte in utf-8; each byte is to be encoded
		{
			char bytes[20]; bytes[0] = 0;
			UT_sint32 j;
			for(j = 0; j < iByteLen; ++j)
			{
				UT_uint32 v = (unsigned char)p[j];
				snprintf(buff, 30, "%%%02x", v);
				strcat(bytes,buff);
			}

			char * b = bytes;
			for(j = 0; j < iByteLen; ++j)
			{
				*p++ = *b++;
			}
			
			insert(p, b, strlen(b));

			for(j = 0; j < iByteLen; ++j)
			{
				J.advance();
				J.advance();
				J.advance();
			}

			J.retreat();
		}
		else if(// all single byte chars that always have to be encoded
		   (c <= 0x20 || c > 0x7e || (!isalnum(c) && !strchr("$-_.+!*'(),;/?:@=&#", c)))
		   
		   // between the path element and the scheme marker all reserved chars other than @ and : also need to
		   // be encode
		   || (p < urlpath && p >= schm && strchr(";/?=&#",c))
		
		   // in user name and pswd, colons and @ have to be encoded
		   || ((user && host && p >= user && p < host - 1) && ((c == ':' && (!pswd || p != pswd - 1)) || c == '@'))

		   // in the host part we also encode @
		   || (c == '@' && p >= host && p < urlpath)

		   // in url paths, the requirements are scheme-specific
		   // http scheme: "/?;" are reserved; encode all # other than the fragment marker,
		   // all = before the parameter ? as well as all :, @, &
		   || (p > urlpath &&
			   ((iScheme == 0 || iScheme == 2) && 
				((c=='?' && p!=last_quest) || (c=='#' && p!=last_hash) || (c=='=' && p<last_quest)
				 || strchr(":@&", c))))

		   // in mailto are no reserved characters
		   || (p > urlpath &&
			   (iScheme == 4 && strchr(";?:@=&#/",c)))

		   // news, only @ is reserved
		   || (p > urlpath &&
			   (iScheme == 5 && strchr(";?:=&#/",c)))
		   
		   // in all other schemes we escape the reserved characters except /
		   || (p > urlpath &&
			   (iScheme != 0 && iScheme != 2 && iScheme != 4 && iScheme != 5) && strchr(";?:@=&#", c)))
		{
			UT_return_if_fail( p );

			// we have to adjust any pointers we keep in line with the insertion
			if(last_quest >= p) last_quest += 2;
			if(last_hash >=  p) last_hash  += 2;
			if(last_slash >= p) last_slash += 2;
			if(host >= p) host += 2;
			if(pswd >= p) pswd += 2;
			if(user >= p) user += 2;
			if(port >= p) port += 2;

			UT_uint32 v = *p;
			
			snprintf(buff, 30, "%02x", v);
			*p++ = '%';
			insert(p, buff, strlen(buff));

			// move past the two new chars
			J.advance();
			J.advance();

		}
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

	char * buff = (char*)malloc(byteLength() + 1);
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
					UT_UCS4Stringbuf::UCS4_to_UTF8(p, iLenLeft, code);
 
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
	free(buff);
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
	if (m_psz) free (m_psz);
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
		m_psz = static_cast<char *>(malloc(length));
		if (m_psz == 0) return false;
		m_strlen = 0;
		m_buflen = length;
		m_pEnd = m_psz;
		*m_pEnd = 0;
		return true;
	}

	size_t new_length = length + (m_pEnd - m_psz) + 1;
	size_t end_offset = m_pEnd - m_psz;

	char * more = static_cast<char *>(realloc(static_cast<void *>(m_psz), new_length));
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
	if (!sync ()) return 0;
	return m_utfbuf;
}

const char * UT_UTF8Stringbuf::UTF8Iterator::end ()
{
	if (!sync ()) return 0;
	return m_utfbuf + m_strbuf->byteLength ();
}

const char * UT_UTF8Stringbuf::UTF8Iterator::advance ()
{
	if (!sync ()) return 0;
	if (*m_utfptr == 0) return 0;
	do m_utfptr++;
	while ((*m_utfptr & 0xc0) == 0x80); // a 'continuing' byte 
	return m_utfptr;
}

const char * UT_UTF8Stringbuf::UTF8Iterator::retreat ()
{
	if (!sync ()) return 0;
	if (m_utfptr == m_utfbuf) return 0;
	do m_utfptr--;
	while ((*m_utfptr & 0xc0) == 0x80); // a 'continuing' byte 
	return m_utfptr;
}

// returns false only if there is no string data
bool UT_UTF8Stringbuf::UTF8Iterator::sync ()
{
	if (m_strbuf == 0) return false;

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

/* scans a buffer for the next valid UTF-8 sequence and returns the corresponding
 * UCS-4 value for that sequence; the pointer and length-remaining are incremented
 * and decremented respectively; returns 0 if no valid UTF-8 sequence found by the
 * end of the string
 */
UT_UCS4Char UT_UCS4Stringbuf::UTF8_to_UCS4 (const char *& buffer, size_t & length)
{
	UT_UCS4Char ucs4;

	while (true) {
		ucs4 = 0;
		if (length == 0) break;

		unsigned char c = static_cast<unsigned char>(*buffer);
		buffer++;
		length--;

		if ((c & 0x80) == 0) { // ascii, single-byte sequence
			ucs4 = static_cast<UT_UCS4Char>(c);
			break;
		}
		if ((c & 0xc0) == 0x80) { // hmm, continuing byte - let's just ignore it
			continue;
		}

		/* we have a multi-byte sequence...
		 */
		size_t seql;

		if ((c & 0xe0) == 0xc0) {
			seql = 2;
			ucs4 = static_cast<UT_UCS4Char>(c & 0x1f);
		}
		else if ((c & 0xf0) == 0xe0) {
			seql = 3;
			ucs4 = static_cast<UT_UCS4Char>(c & 0x0f);
		}
		else if ((c & 0xf8) == 0xf0) {
			seql = 4;
			ucs4 = static_cast<UT_UCS4Char>(c & 0x07);
		}
		else if ((c & 0xfc) == 0xf8) {
			seql = 5;
			ucs4 = static_cast<UT_UCS4Char>(c & 0x03);
		}
		else if ((c & 0xfe) == 0xfc) {
			seql = 6;
			ucs4 = static_cast<UT_UCS4Char>(c & 0x01);
		}
		else { // or perhaps we don't :-( - whatever it is, let's just ignore it
			continue;
		}

		if (length < seql - 1) { // huh? broken sequence perhaps? anyway, let's just ignore it
			continue;
		}

		bool okay = true;
		for (size_t i = 1; i < seql; i++) {
			c = static_cast<unsigned char>(*buffer);
			buffer++;
			length--;
			if ((c & 0xc0) != 0x80) { // not a continuing byte? grr!
				okay = false;
				break;
			}
			ucs4 = ucs4 << 6 | static_cast<UT_UCS4Char>(c & 0x3f);
		}
		if (okay) break;
	}
	return ucs4;
}

/* Returns -1 if ucs4 is not valid UCS-4, 0 if ucs4 is 0, 1-6 otherwise
 */
int UT_UCS4Stringbuf::UTF8_ByteLength (UT_UCS4Char u)
{
	if ((u & 0x7fffffff) != u) return -1; // UCS-4 is only 31-bit!

	if (u == 0) return 0; // end-of-string

	if ((u & 0x7fffff80) == 0) return 1;
	if ((u & 0x7ffff800) == 0) return 2;
	if ((u & 0x7fff0000) == 0) return 3;
	if ((u & 0x7fe00000) == 0) return 4;
	if ((u & 0x7c000000) == 0) return 5;
	return 6;
}

/* appends to the buffer the UTF-8 sequence corresponding to the UCS-4 value;
 * the pointer and length-remaining are incremented and decremented respectively;
 * returns false if not valid UCS-4 or if (length < UTF8_ByteLength (ucs4))
 */
bool UT_UCS4Stringbuf::UCS4_to_UTF8 (char *& buffer, size_t & length, UT_UCS4Char ucs4)
{
	int seql = UT_UCS4Stringbuf::UTF8_ByteLength (ucs4);
	if (seql < 0) return false;
	if (seql == 0) {
		if (length == 0) return false;
		*buffer++ = 0;
		length--;
		return true;
	}
	if (length < static_cast<unsigned>(seql)) return false;
	length -= seql;

	switch (seql) {
	case 1:
		*buffer++ = static_cast<char>(static_cast<unsigned char>(ucs4 & 0x7f));
		break;
	case 2:
		*buffer++ = static_cast<char>(0xc0 | static_cast<unsigned char>((ucs4 >> 6) & 0x1f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>(ucs4 & 0x3f));
		break;
	case 3:
		*buffer++ = static_cast<char>(0xe0 | static_cast<unsigned char>((ucs4 >> 12) & 0x0f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 6) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>(ucs4 & 0x3f));
		break;
	case 4:
		*buffer++ = static_cast<char>(0xf0 | static_cast<unsigned char>((ucs4 >> 18) & 0x07));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 12) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 6) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>(ucs4 & 0x3f));
		break;
	case 5:
		*buffer++ = static_cast<char>(0xf8 | static_cast<unsigned char>((ucs4 >> 24) & 0x03));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 18) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 12) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 6) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>(ucs4 & 0x3f));
		break;
	case 6:
		*buffer++ = static_cast<char>(0xfc | static_cast<unsigned char>((ucs4 >> 30) & 0x01));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 24) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 18) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 12) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 6) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>(ucs4 & 0x3f));
		break;
	default: // huh?
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	return true;
}

UT_UCS4Stringbuf::UT_UCS4Stringbuf()
:	m_psz(0),
	m_pEnd(0),
	m_size(0),
	m_utf8string(0)
{
}

UT_UCS4Stringbuf::UT_UCS4Stringbuf(const UT_UCS4Stringbuf& rhs)
:	m_psz(new char_type[rhs.capacity()]),
	m_pEnd(m_psz + rhs.size()),
	m_size(rhs.capacity()),
	m_utf8string(0)
{
	copy(m_psz, rhs.m_psz, rhs.capacity());
}

UT_UCS4Stringbuf::UT_UCS4Stringbuf(const char_type* sz, size_t n)
:	m_psz(new char_type[n+1]),
	m_pEnd(m_psz + n),
	m_size(n+1),
	m_utf8string(0)
{
	copy(m_psz, sz, n);
	m_psz[n] = 0;
}

UT_UCS4Stringbuf::~UT_UCS4Stringbuf()
{
	clear();
}


void UT_UCS4Stringbuf::operator=(const UT_UCS4Stringbuf& rhs)
{
	if (this != &rhs)
	{
		clear();
		assign(rhs.m_psz, rhs.size());
	}
}

void UT_UCS4Stringbuf::assign(const char_type* sz, size_t n)
{
	if (m_utf8string) // buffered internal UTF-8 string is invalid
	{
		delete[] m_utf8string;
		m_utf8string = 0;
	}
	if (n)
	{
		if (n >= capacity())
		{
			grow_nocopy(n);
		}
		copy(m_psz, sz, n);
		m_psz[n] = 0;
		m_pEnd = m_psz + n;
	} else {
		clear();
	}
}

void UT_UCS4Stringbuf::append(const char_type* sz, size_t n)
{
	if (!n)
	{
		return;
	}
	if (!capacity())
	{
		assign(sz, n);
		return;
	}
	if (m_utf8string) // buffered internal UTF-8 string is invalid
	{
		delete[] m_utf8string;
		m_utf8string = 0;
	}
	const size_t nLen = size();
	grow_copy(nLen + n);
	copy(m_psz + nLen, sz, n);
	m_psz[nLen + n] = 0;
	m_pEnd += n;
}

void UT_UCS4Stringbuf::append(const UT_UCS4Stringbuf& rhs)
{
	append(rhs.m_psz, rhs.size());
}

void UT_UCS4Stringbuf::swap(UT_UCS4Stringbuf& rhs)
{
	my_ut_swap(m_psz , rhs.m_psz );
	my_ut_swap(m_pEnd, rhs.m_pEnd);
	my_ut_swap(m_size, rhs.m_size);

	my_ut_swap(m_utf8string, rhs.m_utf8string);
}

void UT_UCS4Stringbuf::clear()
{
	if (m_psz)
	{
		delete[] m_psz;
		m_psz = 0;
		m_pEnd = 0;
		m_size = 0;
	}
	if (m_utf8string)
	{
		delete[] m_utf8string;
		m_utf8string = 0;
	}
}

const char* UT_UCS4Stringbuf::utf8_data()
{
	if (m_utf8string) return m_utf8string;

	size_t utf8length = size ();
	size_t bytelength = 0;
	size_t i;
	for (i = 0; i < utf8length; i++)
	{
		int seql = UT_UCS4Stringbuf::UTF8_ByteLength (m_psz[i]);
		if (seql < 0) continue; // not UCS-4 !!
		if (seql == 0) break; // huh? premature end-of-string?
		bytelength += static_cast<size_t>(seql);
	}
	m_utf8string = new char[bytelength+1];

	char * utf8string = m_utf8string;
	for (i = 0; i < utf8length; i++)
	{
		int seql = UT_UCS4Stringbuf::UTF8_ByteLength (m_psz[i]);
		if (seql < 0) continue; // not UCS-4 !!
		if (seql == 0) break; // huh? premature end-of-string?
		UT_UCS4Stringbuf::UCS4_to_UTF8 (utf8string, bytelength, m_psz[i]);
	}
	*utf8string = 0;

	return m_utf8string;
}

void UT_UCS4Stringbuf::reserve(size_t n)
{
	grow_nocopy(n);
}

void UT_UCS4Stringbuf::grow_nocopy(size_t n)
{
	grow_common(n, false);
}

void UT_UCS4Stringbuf::grow_copy(size_t n)
{
	grow_common(n, true);
}

void UT_UCS4Stringbuf::grow_common(size_t n, bool bCopy)
{
	++n;	// allow for zero termination
	if (n > capacity())
	{
		const size_t nCurSize = size();
		n = priv_max(n, static_cast<size_t>(nCurSize * g_rGrowBy));
		char_type* pNew = new char_type[n];
		if (bCopy && m_psz)
		{
			copy(pNew, m_psz, size() + 1);
		}
		delete[] m_psz;
		m_psz  = pNew;
		m_pEnd = m_psz + nCurSize;
		m_size = n;
	}
}

void UT_UCS4Stringbuf::copy(char_type* pDest, const char_type* pSrc, size_t n)
{
	if(pSrc && pDest)
		memcpy(pDest, pSrc, n * sizeof(char_type));
}
