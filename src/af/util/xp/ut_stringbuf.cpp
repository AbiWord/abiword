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
#include <string.h>	// memcpy
#include "ut_stringbuf.h"
#include "ut_debugmsg.h"

// these classes keep zero terminated strings.
// if size() != 0, capacity() is always at least size() + 1.

//////////////////////////////////////////////////////////////////

static const float g_rGrowBy = 1.5;

static inline size_t priv_max(size_t a, size_t b)
{
	return a < b ? b : a;
}

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


static inline
void my_ut_swap(UT_Stringbuf::char_type*&  a, UT_Stringbuf::char_type*&  b)
	{ UT_Stringbuf::char_type*  t = a; a = b; b = t; }

static inline
void my_ut_swap(size_t& a, size_t& b)
	{ size_t t = a; a = b; b = t; }

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
		n = priv_max(n, (size_t)(nCurSize * g_rGrowBy));
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
	memcpy(pDest, pSrc, n * sizeof(char_type));
}


/***************************************************************************/
/***************************************************************************/

/* scans a buffer for the next valid UTF-8 sequence and returns the corresponding
 * UCS-2 value for that sequence; the pointer and length-remaining are incremented
 * and decremented respectively; returns 0 if no valid UTF-8 sequence found by the
 * end of the string
 */
UT_UCSChar UT_UCS2Stringbuf::UTF8_to_UCS2 (const char *& buffer, size_t & length)
{
	UT_UCSChar ucs2;

	while (true) {
		ucs2 = 0;
		if (length == 0) break;

		unsigned char c = static_cast<unsigned char>(*buffer);
		buffer++;
		length--;

		if ((c & 0x80) == 0) { // ascii, single-byte sequence
			ucs2 = static_cast<UT_UCSChar>(c);
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
			ucs2 = static_cast<UT_UCSChar>(c & 0x1f);
		}
		else if ((c & 0xf0) == 0xe0) {
			seql = 3;
			ucs2 = static_cast<UT_UCSChar>(c & 0x0f);
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
			ucs2 = ucs2 << 6 | static_cast<UT_UCSChar>(c & 0x3f);
		}
		if (okay) break;
	}
	return ucs2;
}

UT_UCS2Stringbuf::UT_UCS2Stringbuf()
:	m_psz(0),
	m_pEnd(0),
	m_size(0)
{
}

UT_UCS2Stringbuf::UT_UCS2Stringbuf(const UT_UCS2Stringbuf& rhs)
:	m_psz(new char_type[rhs.capacity()]),
	m_pEnd(m_psz + rhs.size()),
	m_size(rhs.capacity())
{
	copy(m_psz, rhs.m_psz, rhs.capacity());
}

UT_UCS2Stringbuf::UT_UCS2Stringbuf(const char_type* sz, size_t n)
:	m_psz(new char_type[n+1]),
	m_pEnd(m_psz + n),
	m_size(n+1)
{
	copy(m_psz, sz, n);
	m_psz[n] = 0;
}

UT_UCS2Stringbuf::~UT_UCS2Stringbuf()
{
	clear();
}


void UT_UCS2Stringbuf::operator=(const UT_UCS2Stringbuf& rhs)
{
	if (this != &rhs)
	{
		clear();
		assign(rhs.m_psz, rhs.size());
	}
}

void UT_UCS2Stringbuf::assign(const char_type* sz, size_t n)
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

void UT_UCS2Stringbuf::append(const char_type* sz, size_t n)
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

void UT_UCS2Stringbuf::append(const UT_UCS2Stringbuf& rhs)
{
	append(rhs.m_psz, rhs.size());
}


static inline
void my_ut_swap(UT_UCS2Stringbuf::char_type*&  a, UT_UCS2Stringbuf::char_type*&  b)
	{ UT_UCS2Stringbuf::char_type*  t = a; a = b; b = t; }

void UT_UCS2Stringbuf::swap(UT_UCS2Stringbuf& rhs)
{
	my_ut_swap(m_psz , rhs.m_psz );
	my_ut_swap(m_pEnd, rhs.m_pEnd);
	my_ut_swap(m_size, rhs.m_size);
}

void UT_UCS2Stringbuf::clear()
{
	if (m_psz)
	{
		delete[] m_psz;
		m_psz = 0;
		m_pEnd = 0;
		m_size = 0;
	}
}

void UT_UCS2Stringbuf::grow_nocopy(size_t n)
{
	grow_common(n, false);
}

void UT_UCS2Stringbuf::grow_copy(size_t n)
{
	grow_common(n, true);
}

void UT_UCS2Stringbuf::grow_common(size_t n, bool bCopy)
{
	++n;	// allow for zero termination
	if (n > capacity())
	{
		const size_t nCurSize = size();
		n = priv_max(n, (size_t)(nCurSize * g_rGrowBy));
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

void UT_UCS2Stringbuf::copy(char_type* pDest, const char_type* pSrc, size_t n)
{
	memcpy(pDest, pSrc, n * sizeof(char_type));
}


/***************************************************************************/
/***************************************************************************/


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

UT_UTF8Stringbuf::UT_UTF8Stringbuf (const char * sz) :
	m_psz(0),
	m_pEnd(0),
	m_strlen(0),
	m_buflen(0)
{
	append (sz);
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

void UT_UTF8Stringbuf::assign (const char * sz)
{
	m_pEnd = m_psz;
	m_strlen = 0;
	append (sz);
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

void UT_UTF8Stringbuf::append (const char * sz)
{
	if (sz == 0) return;
	if (!grow (strlen (sz) + 1)) return;

	const char * p = sz;
	char buf[6];
	int bytesInSequence = 0;
	int bytesExpectedInSequence = 0;

	while (*p)
	{
		if ((*p & 0x80) == 0x00) // plain us-ascii part of latin-1
		{
			if (bytesInSequence) break;

			*m_pEnd++ = *p;
			*m_pEnd = 0;
			m_strlen++;

			p++;
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
			continue;
		}
		if ((*p & 0xfc) == 0xf8) // lead byte in 5-byte sequence
		{
			bytesExpectedInSequence = 5;
			p++;
			continue;
		}
		if ((*p & 0xf8) == 0xf0) // lead byte in 4-byte sequence
		{
			bytesExpectedInSequence = 4;
			p++;
			continue;
		}

		/* 1,2,3-byte sequences do not require > 2 bytes in UCS-4
		 */
		if ((*p & 0xf0) == 0xe0) // lead byte in 3-byte sequence
		{
			bytesExpectedInSequence = 3;
			p++;
			continue;
		}
		if ((*p & 0xe0) == 0xc0) // lead byte in 2-byte sequence
		{
			bytesExpectedInSequence = 2;
			p++;
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

void UT_UTF8Stringbuf::clear ()
{
	if (m_psz) free (m_psz);
	m_psz = 0;
	m_pEnd = 0;
	m_strlen = 0;
	m_buflen = 0;
}

bool UT_UTF8Stringbuf::grow (size_t length)
{
	if (length <= (m_buflen - (m_pEnd - m_psz))) return true;

	if (m_psz == 0)
	{
		m_psz = (char *) malloc (length);
		if (m_psz == 0) return false;
		m_strlen = 0;
		m_buflen = length;
		m_pEnd = m_psz;
		*m_pEnd = 0;
		return true;
	}

	size_t new_length = length + (m_pEnd - m_psz);
	size_t end_offset = m_pEnd - m_psz;

	char * more = (char *) realloc ((void *) m_psz, new_length);
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
	if ((position - m_utfbuf) > m_strbuf->byteLength ())
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
	 * start of utf8 char sequence
	 */
	if ((m_utfptr - m_utfbuf) > utf8_length)
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
