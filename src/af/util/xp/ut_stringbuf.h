// ut_stringbuf.h
//
#ifndef UT_STRINGBUF_H
#define UT_STRINGBUF_H

//
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

#include <stdlib.h>	// size_t
#include "ut_types.h" // UT_UCSChar

// yes, i know that this is screaming for templates... -Dom

class ABI_EXPORT UT_Stringbuf
{
public:
	typedef char char_type;

	UT_Stringbuf();
	UT_Stringbuf(const UT_Stringbuf& rhs);
	UT_Stringbuf(const char_type* sz, size_t n);
	~UT_Stringbuf();

	void		operator=(const UT_Stringbuf& rhs);

	void		assign(const char_type* sz, size_t n);
	void		append(const char_type* sz, size_t n);
	void		append(const UT_Stringbuf& rhs);

	void		swap(UT_Stringbuf& rhs);
	void		clear();

	bool				empty()		const { return m_psz == m_pEnd; }
	size_t				size()		const { return m_pEnd - m_psz; }
	size_t				capacity()	const { return m_size; }
	const char_type*	data()		const { return m_psz; }
	char_type*			data() 			  { return m_psz; }

private:
	void	grow_nocopy(size_t n);
	void	grow_copy(size_t n);
	void	grow_common(size_t n, bool bCopy);

	static void copy(char_type* pDest, const char_type* pSrc, size_t n);

	char_type*	m_psz;
	char_type*	m_pEnd;
	size_t		m_size;
};

class ABI_EXPORT UT_UCS2Stringbuf
{
public:
	typedef UT_UCS2Char char_type;

	UT_UCS2Stringbuf();
	UT_UCS2Stringbuf(const UT_UCS2Stringbuf& rhs);
	UT_UCS2Stringbuf(const char_type* sz, size_t n);
	~UT_UCS2Stringbuf();

	void		operator=(const UT_UCS2Stringbuf& rhs);

	void		assign(const char_type* sz, size_t n);
	void		append(const char_type* sz, size_t n);
	void		append(const UT_UCS2Stringbuf& rhs);

	void		swap(UT_UCS2Stringbuf& rhs);
	void		clear();

	bool				empty()		const { return m_psz == m_pEnd; }
	size_t				size()		const { return m_pEnd - m_psz; }
	size_t				capacity()	const { return m_size; }
	const char_type*	data()		const { return m_psz; }
	char_type*			data() 			  { return m_psz; }

private:
	void	grow_nocopy(size_t n);
	void	grow_copy(size_t n);
	void	grow_common(size_t n, bool bCopy);

	static void copy(char_type* pDest, const char_type* pSrc, size_t n);

	char_type*	m_psz;
	char_type*	m_pEnd;
	size_t		m_size;
};

class ABI_EXPORT UT_UTF8Stringbuf
{
public:
	typedef UT_UCSChar   UCS2Char;
	typedef unsigned int UCS4Char;

	static UCS4Char charCode (const char * str);

	UT_UTF8Stringbuf ();
	UT_UTF8Stringbuf (const UT_UTF8Stringbuf & rhs);
	UT_UTF8Stringbuf (const char * sz);

	~UT_UTF8Stringbuf ();

	void		operator=(const UT_UTF8Stringbuf & rhs);

	void		assign (const char * sz);
	void		append (const char * sz);
	void		append (const UT_UTF8Stringbuf & rhs);

	void		clear ();

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
	char *	m_psz;
	char *	m_pEnd;
	size_t	m_strlen;
	size_t	m_buflen;

	bool	grow (size_t length);
};

class ABI_EXPORT UT_UCS4Stringbuf
{
public:
	typedef UT_UCS4Char char_type;

	/* scans a buffer for the next valid UTF-8 sequence and returns the corresponding
	 * UCS-4 value for that sequence; the pointer and length-remaining are incremented
	 * and decremented respectively; returns 0 if not valid UTF-8 sequence found by the
	 * end of the string
	 */
	static UT_UCS4Char UTF8_to_UCS4 (const char *& buffer, size_t & length);

	/* Returns -1 if ucs4 is not valid UCS-4, 0 if ucs4 is 0, 1-6 otherwise
	 */
	static int UTF8_ByteLength (UT_UCS4Char ucs4);

	/* appends to the buffer the UTF-8 sequence corresponding to the UCS-4 value;
	 * the pointer and length-remaining are incremented and decremented respectively;
	 * returns false if not valid UCS-4 or if (length < UTF8_ByteLength (ucs4))
	 */
	static bool UCS4_to_UTF8 (char *& buffer, size_t & length, UT_UCS4Char ucs4);

	UT_UCS4Stringbuf();
	UT_UCS4Stringbuf(const UT_UCS4Stringbuf& rhs);
	UT_UCS4Stringbuf(const char_type* sz, size_t n);
	~UT_UCS4Stringbuf();

	void		operator=(const UT_UCS4Stringbuf& rhs);

	void		assign(const char_type* sz, size_t n);
	void		append(const char_type* sz, size_t n);
	void		append(const UT_UCS4Stringbuf& rhs);

	void		swap(UT_UCS4Stringbuf& rhs);
	void		clear();

	bool				empty()		const { return m_psz == m_pEnd; }
	size_t				size()		const { return m_pEnd - m_psz; }
	size_t				capacity()	const { return m_size; }
	const char_type*	data()		const { return m_psz; }
	char_type*			data() 			  { return m_psz; }

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

#endif	// UT_STRINGBUF_H
