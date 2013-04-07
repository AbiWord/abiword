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


#include "ut_Win32LocaleString.h"
#include "xap_App.h"
#include "ut_iconv.h"
#include "xap_EncodingManager.h"

#include "ut_mbtowc.h"


//static const UT_UCS2Char *ucsEmpty = 0;
static const UT_UCS2Char ucsEmpty[] = { 0 };

UT_uint32 UT_UCS_strlen(const UT_UCS2Char * string)
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

UT_UCS2Char * UT_UCS2_strcpy_char(UT_UCS2Char * dest, char * src)
{
	UT_ASSERT(dest);
	UT_ASSERT(src);

	UT_UCS2Char * d 	= dest;
	unsigned char * s	= (unsigned char *)(src);

	//static UT_UCS2_mbtowc m(XAP_EncodingManager::get_instance()->getNative8BitEncodingName());
	static UT_UCS2_mbtowc m( "UCS-2LE");
	UT_UCS2Char wc;

	while (*s != 0)
	  {
		if(m.mbtowc(wc,*s))*d++=wc;
		s++;
	  }
	*d = 0;

	return dest;
}


//static float g_rGrowBy = 1.5f;

static inline size_t priv_max(size_t a, size_t b)
{
	return a < b ? b : a;
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

UT_UCS2String::UT_UCS2String()
:	pimpl(new UT_UCS2Stringbuf)
{
}

UT_UCS2String::UT_UCS2String(const UT_UCS2Char* sz, size_t n)
:	pimpl(new UT_UCS2Stringbuf(sz, n ? n : (sz) ? UT_UCS_strlen(sz) : 0))
{
}

UT_UCS2String::UT_UCS2String(const UT_UCS2String& rhs)
:	pimpl(new UT_UCS2Stringbuf(*rhs.pimpl))
{
}

UT_UCS2String::~UT_UCS2String()
{
	delete pimpl;
}


//////////////////////////////////////////////////////////////////
// accessors

size_t UT_UCS2String::size() const
{
	return pimpl->size();
}

bool UT_UCS2String::empty() const
{
	return pimpl->empty();
}

void UT_UCS2String::clear() const
{
	pimpl->clear();
}

UT_UCS2String UT_UCS2String::substr(size_t iStart, size_t nChars) const
{
	const size_t nSize = pimpl->size();

	if (iStart >= nSize || !nChars) {
		return UT_UCS2String();
	}

	const UT_UCS2Char* p = pimpl->data() + iStart;
	if (iStart + nChars > nSize) {
		nChars = nSize - iStart;
	}

	return UT_UCS2String(p, nChars);
}

const UT_UCS2Char* UT_UCS2String::ucs2_str() const
{
//	int size = pimpl->size();
//	char *p = (char*) pimpl->data();

	return pimpl->size() ? pimpl->data() : ucsEmpty;
}


//////////////////////////////////////////////////////////////////
// mutators

UT_UCS2String& UT_UCS2String::operator=(const UT_UCS2String& rhs)
{
	if (this != &rhs) {
		*pimpl = *rhs.pimpl;
	}
	return *this;
}

UT_UCS2String& UT_UCS2String::operator=(const UT_UCS2Char* rhs)
{
	pimpl->assign(rhs, UT_UCS_strlen(rhs));
	return *this;
}




UT_Win32LocaleString::UT_Win32LocaleString ()
{

}


const wchar_t * UT_Win32LocaleString::c_str() const
{
	return (wchar_t * )ucs2_str();
}

UT_UTF8String UT_Win32LocaleString::utf8_str() const
{
	char * pText = UT_convert ((const char *) ucs2_str(),
							  size () * sizeof (wchar_t),
							  "UCS-2LE",
							  "UTF-8",
							  NULL, NULL);

	UT_UTF8String str (pText);
	g_free(pText);
	return str;
}

UT_UCS4String UT_Win32LocaleString::ucs4_str() const
{
	UT_UCS4Char * pText = (UT_UCS4Char *) UT_convert ((const char *) c_str(),
							  (size()+1) * sizeof (wchar_t),
							  "UCS-2LE",
							  "UCS-4LE",
							  NULL, NULL);

	UT_UCS4String str (pText);
	g_free(pText);
	return str;
}

void UT_Win32LocaleString::fromUCS2 (const UT_UCS2Char * /*szIn*/)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

#ifdef DEBUG
static UT_UCS4Char ds[]={'F','I','X','M','E','4',')',0};
#endif

void UT_Win32LocaleString::fromUCS4 (const UT_UCS4Char* usc4_in)
{
#ifdef DEBUG
	if (!usc4_in) usc4_in=ds;
#endif
	UT_UCS2Char * pText = (UT_UCS2Char *) UT_convert ((const char*) usc4_in,
							  (UT_UCS4_strlen(usc4_in)+1) * 4,
							  "UCS-4LE",
							  "UCS-2LE",
							  NULL, NULL);
	if (!pText || !*pText) {
    	pimpl->clear ();
    	return;
    }

    pimpl->assign(pText, wcslen((const wchar_t *)pText));
	g_free(pText);
}

void UT_Win32LocaleString::fromUTF8 (const char* utf8str)
{
#ifdef DEBUG
	if (!utf8str) utf8str="FIXME: NULL passed to UT_Win32LocaleString::fromUTF8";
#endif
	UT_UCS2Char * pText = (UT_UCS2Char *) UT_convert (utf8str,
							  strlen(utf8str)+1,
							  "UTF-8",
							  "UCS-2LE",
							  NULL, NULL);

	if (!pText || !*pText) {
		pimpl->clear ();
		return;
	}

	pimpl->assign(pText, wcslen((const wchar_t *)pText));
	g_free(pText);
}

void UT_Win32LocaleString::fromASCII (const char* szASCII, size_t sz)
{
#ifdef DEBUG
	if (!szASCII) szASCII="FIXME: NULL passed to UT_Win32LocaleString::fromASCII";
#endif
	size_t len = (sz == -1) ? strlen(szASCII) : sz;
	UT_UCS2Char * src = new UT_UCS2Char [len + 1];
	char *p_str = (char *)szASCII;
	for (UT_uint32 i = 0; i < len; i++)
	{
		src[i] = *p_str;
		p_str++;
	}
	pimpl->assign(src, len);
	delete src;
}

void UT_Win32LocaleString::fromLocale (const wchar_t* szLocale)
{
	size_t len = wcslen (szLocale);
	pimpl->assign((UT_UCS2Char*)szLocale, len);
}

void UT_Win32LocaleString::appendLocale (const wchar_t* szLocale)
{
	size_t len = wcslen (szLocale);
	pimpl->append((UT_UCS2Char*)szLocale, len);
}

void UT_Win32LocaleString::appendASCII (const char* szASCII)
{
	size_t len = strlen(szASCII);
	UT_UCS2Char * src = new UT_UCS2Char [len + 1];
	char *p_str = (char *)szASCII;
	for (UT_uint32 i = 0; i < strlen (szASCII); i++)
	{
		src[i] = *p_str;
		p_str++;
	}
	pimpl->append(src, len);
	delete src;
}

UT_Win32LocaleString UT_Win32LocaleString::substr(size_t iStart, size_t nChars) const
{
	UT_Win32LocaleString str;
	const size_t nSize = pimpl->size();

	if (iStart >= nSize || !nChars) {
		return str;
	}

	const UT_UCS2Char* p = pimpl->data() + iStart;
	if (iStart + nChars > nSize) {
		nChars = nSize - iStart;
	}

	str.pimpl->append(p, nChars);
	return str;
}


bool operator==(const UT_Win32LocaleString& s1, const wchar_t* s2)
{
	return wcscmp(s1.c_str(), s2) == 0;
}

