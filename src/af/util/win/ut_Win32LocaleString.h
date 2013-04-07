/* AbiWord
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

#ifndef UT_WIN32LOCALESTRING_H
#define UT_WIN32LOCALESTRING_H

#include "ut_string_class.h"
#include <windows.h>
#include <wchar.h>



class UT_UCS2Stringbuf
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


class ABI_EXPORT UT_UCS2String
{
public:
	UT_UCS2String();
	UT_UCS2String(const UT_UCS2Char * sz, size_t n = 0 /* 0 == zero-terminate */);
	UT_UCS2String(const UT_UCS2String& rhs);
	~UT_UCS2String();

	size_t		size() const;
	bool		empty() const;
	void        clear() const;
	size_t		length() { return size(); }

	UT_UCS2String	substr(size_t iStart, size_t nChars) const;

	UT_UCS2String&	operator=(const UT_UCS2String&  rhs);
	UT_UCS2String&	operator=(const UT_UCS2Char *    rhs);

	// The returned pointer is valid until the next non-const
	// operation. You will _always_ get a legal pointer back,
	// even if to an empty (0) string.
	const UT_UCS2Char* ucs2_str() const;

protected:
	class UT_UCS2Stringbuf* pimpl;
};


class ABI_EXPORT UT_Win32LocaleString : public UT_UCS2String
{
public:

	UT_Win32LocaleString ();

	void fromUCS2 (const UT_UCS2Char * szIn);
	void fromUCS4 (const UT_UCS4Char * szIn);
	void fromUTF8 (const char* szUTF8);
	void fromASCII (const char* szASCII, size_t size = -1);
	void fromLocale (const wchar_t* szLocale);

	void appendASCII (const char* szASCII);
	void appendLocale (const wchar_t* szLocale);
	const wchar_t * c_str() const;
	UT_UTF8String utf8_str() const;
	UT_UCS4String ucs4_str() const;
	UT_Win32LocaleString substr(size_t iStart, size_t nChars) const;

	wchar_t operator[](size_t iPos) const
	{
		UT_ASSERT(iPos <= size());
		if (iPos == size())
			return L'\0';
		return (wchar_t) pimpl->data()[iPos];
	}

	wchar_t& operator[](size_t iPos)
	{
		UT_ASSERT(iPos <= size());
		return (wchar_t &) pimpl->data()[iPos];
	}
};

bool operator==(const UT_Win32LocaleString& s1, const wchar_t* s2);

#endif /* UT_WIN32LOCALESTRING_H */
