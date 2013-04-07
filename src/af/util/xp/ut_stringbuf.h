/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

// ut_stringbuf.h
//
#ifndef UT_STRINGBUF_H
#define UT_STRINGBUF_H

//
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

#include <stdlib.h>	// size_t

#include <string>
#include <algorithm>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_assert.h"
#include "ut_unicode.h"

//////////////////////////////////////////////////////////////////

#define g_rGrowBy 1.5f


template <typename char_type>
class UT_StringImpl
{
public:
	UT_StringImpl();
	UT_StringImpl(const UT_StringImpl<char_type>& rhs);
	UT_StringImpl(const char_type* sz, size_t n);
	UT_StringImpl(const std::basic_string<char_type> &s);
	~UT_StringImpl();

	void		operator=(const UT_StringImpl<char_type>& rhs);

	void		assign(const char_type* sz, size_t n);
	void		append(const char_type* sz, size_t n);
	void		append(const UT_StringImpl<char_type>& rhs);

	void		swap(UT_StringImpl<char_type>& rhs);
	void		clear();
	void        reserve(size_t n);

	bool				empty()		const { return m_psz == m_pEnd; }
	size_t				size()		const { return m_pEnd - m_psz; }
	size_t				capacity()	const { return m_size; }
	const char_type*	data()		const { return m_psz; }
	char_type*			data() 			  { return m_psz; }
	/** return the utf8 content. Only for UCS4Char */
	const char*			utf8_data();

private:
	void	grow_nocopy(size_t n);
	void	grow_copy(size_t n);
	void	grow_common(size_t n, bool bCopy);

	static void copy(char_type* pDest, const char_type* pSrc, size_t n);

	char_type*	m_psz;
	char_type*	m_pEnd;
	size_t		m_size;
	char*		m_utf8string;
};


class UT_UTF8String;

class ABI_EXPORT UT_UTF8Stringbuf
{
public:
	typedef UT_UCSChar   UCS2Char;
	typedef unsigned int UCS4Char;

	static UCS4Char charCode (const char * str);

	UT_UTF8Stringbuf ();
	UT_UTF8Stringbuf (const UT_UTF8Stringbuf & rhs);
	UT_UTF8Stringbuf (const char * sz, size_t n = 0 /* 0 == null-termination */);

	~UT_UTF8Stringbuf ();

	void		operator=(const UT_UTF8Stringbuf & rhs);

	void		assign (const char * sz, size_t n = 0 /* 0 == null-termination */);
	void		append (const char * sz, size_t n = 0 /* 0 == null-termination */);
	void		append (const UT_UTF8Stringbuf & rhs);

	void		appendUCS2 (const UT_UCS2Char * sz, size_t n /* == 0 => null-termination */);
	void		appendUCS4 (const UT_UCS4Char * sz, size_t n /* == 0 => null-termination */);

	void		escape (const UT_UTF8String & str1,
						const UT_UTF8String & str2);  // replaces <str1> with <str2> in the current string
	void		escapeXML ();  // escapes '<', '>', '"', & '&' in the current string
	void		decodeXML ();  // unescapes '<', '>', '"', & '&' in the current string
	void		escapeMIME (); // translates the current string to
							   // MIME "quoted-printable" format
	void        escapeURL ();  // makes string conform to RFC 1738
	void        decodeURL ();

	UT_UTF8Stringbuf * lowerCase ();

	void		clear ();
	void        reserve(size_t n);

	bool		empty ()	const { return m_psz == m_pEnd; }
	size_t		byteLength ()	const { return m_pEnd - m_psz; }
	size_t		utf8Length ()	const { return m_strlen; }
	const char *	data ()		const { return m_psz; }

	class ABI_EXPORT UTF8Iterator
	{
	public:
		UTF8Iterator (const UT_UTF8Stringbuf * strbuf);
		~UTF8Iterator ();

		void operator=(const char * position);

		UTF8Iterator & operator++() { advance (); return *this; } // prefix operators
		UTF8Iterator & operator--() { retreat (); return *this; }

		const char * current (); // return 0 if current position is invalid
		const char * start ();   // return 0 if no string exists
		const char * end ();     // return 0 if no string exists
		const char * advance (); // return 0 if unable to advance
		const char * retreat (); // return 0 if unable to retreat

	private:
		const UT_UTF8Stringbuf * m_strbuf;

		const char * m_utfbuf;
		const char * m_utfptr;

		bool sync ();
	};

private:
	void	insert (char *& ptr, const char * str, size_t utf8length);

	char *	m_psz;
	char *	m_pEnd;
	size_t	m_strlen;
	size_t	m_buflen;

	bool	grow (size_t length);
};



////////////////////////////////////////////////////////////////////////
//
//  Generic string implementation
//
//  String is built of char_type units
//  Encoding could be any single-byte or multi-byte encoding
//
////////////////////////////////////////////////////////////////////////

template <typename char_type>
UT_StringImpl<char_type>::UT_StringImpl()
	:	m_psz(0),
		m_pEnd(0),
		m_size(0),
		m_utf8string(0)
{
}

template <typename char_type>
UT_StringImpl<char_type>::UT_StringImpl(const UT_StringImpl<char_type>& rhs)
	:	m_psz(new char_type[rhs.capacity()]),
		m_pEnd(m_psz + rhs.size()),
		m_size(rhs.capacity()),
		m_utf8string(0)
{
	copy(m_psz, rhs.m_psz, rhs.capacity());
}

template <typename char_type>
UT_StringImpl<char_type>::UT_StringImpl(const char_type* sz, size_t n)
:	m_psz(new char_type[n+1]),
	m_pEnd(m_psz + n),
	m_size(n+1),
	m_utf8string(0)
{
	copy(m_psz, sz, n);
	m_psz[n] = 0;
}

template <typename char_type>
UT_StringImpl<char_type>::UT_StringImpl(const std::basic_string<char_type> &s)
:	m_psz(new char_type[s.size()+1]),
	m_pEnd(m_psz + s.size()),
	m_size(s.size()+1),
	m_utf8string(0)
{
	// string is terminated here, so we know
	strcpy(m_psz, s.c_str());
}


template <typename char_type>
UT_StringImpl<char_type>::~UT_StringImpl()
{
	clear();
}


template <typename char_type>
void UT_StringImpl<char_type>::operator=(const UT_StringImpl<char_type>& rhs)
{
	if (this != &rhs)
	{
		clear();
		assign(rhs.m_psz, rhs.size());
	}
}

template <typename char_type>
void UT_StringImpl<char_type>::assign(const char_type* sz, size_t n)
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
		delete[] m_utf8string;
		m_utf8string = 0;
	} else {
		clear();
	}
}

template <typename char_type>
void UT_StringImpl<char_type>::append(const char_type* sz, size_t n)
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

template <typename char_type>
void UT_StringImpl<char_type>::append(const UT_StringImpl<char_type>& rhs)
{
	append(rhs.m_psz, rhs.size());
}

template <typename char_type>
void UT_StringImpl<char_type>::swap(UT_StringImpl<char_type>& rhs)
{
	std::swap(m_psz , rhs.m_psz );
	std::swap(m_pEnd, rhs.m_pEnd);
	std::swap(m_size, rhs.m_size);
	std::swap(m_utf8string, rhs.m_utf8string);
}

template <typename char_type>
void UT_StringImpl<char_type>::clear()
{
	if (m_psz)
	{
		delete[] m_psz;
		m_psz = 0;
		m_pEnd = 0;
		m_size = 0;
	}
	if(m_utf8string) {
		delete[] m_utf8string;
		m_utf8string = 0;
	}
}

template <typename char_type>
void UT_StringImpl<char_type>::reserve(size_t n)
{
	grow_nocopy(n);
}


template <typename char_type>
const char*	UT_StringImpl<char_type>::utf8_data()
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return "";
}


template <typename char_type>
void UT_StringImpl<char_type>::grow_nocopy(size_t n)
{
	grow_common(n, false);
}

template <typename char_type>
void UT_StringImpl<char_type>::grow_copy(size_t n)
{
	grow_common(n, true);
}

template <typename char_type>
void UT_StringImpl<char_type>::grow_common(size_t n, bool bCopy)
{
	++n;	// allow for zero termination
	if (n > capacity())
	{
		const size_t nCurSize = size();
		n = std::max(n, static_cast<size_t>(nCurSize * g_rGrowBy));
		char_type* pNew = new char_type[n];
		if (bCopy && m_psz)
		{
			copy(pNew, m_psz, size() + 1);
		}
		delete[] m_psz;
		m_psz  = pNew;
		m_pEnd = m_psz + nCurSize;
		m_size = n;
		delete[] m_utf8string;
		m_utf8string = 0;
	}
}

template <typename char_type>
void UT_StringImpl<char_type>::copy(char_type* pDest, const char_type* pSrc, size_t n)
{
	if (pDest && pSrc && n)
		memcpy(pDest, pSrc, n * sizeof(char_type));
}


#endif	// UT_STRINGBUF_H
