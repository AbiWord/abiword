/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
// ut_string_class.cpp

// A simple string class for use where templates are not
// allowed.
//
// Copyright (C) 2001 Mike Nordell <tamlin@algonet.se>
// Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
// Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
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
#include <stdio.h>
#include <stdlib.h>				// size_t
#include <string.h>				// strcmp
#include <locale.h>
#include <ctype.h>
#include <stdarg.h>
#include <algorithm>

#include <glib.h>

#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_stringbuf.h"
#include "ut_debugmsg.h"		// UT_DEBUGMSG
#include "ut_iconv.h"
#include "ut_assert.h"			// UT_ASSERT
#include "ut_mbtowc.h"
#include "ut_bytebuf.h"
#include "ut_unicode.h"

//
// This string class is intended to meet the following requirements.
//
// - It shall not use templates.
// - It shall not provide a sorting order [1].
// - It shall allow dated compilers to use it [2].
// - It shall work with non-conforming library implementations.
// - It shall not use reference counting since that is 1) not
//   platform independent (the need for some kind of locking mechanism)
//   and 2) in a multi threaded environment every single string would
//   still have to be copied, where the ref-counting would be useless
//   and finally 3) locking would slow us down.
//
// [1] It's impossible to get a sorting order other than plain strcmp
//     without adding locale information. This would make this class
//     unacceptably large, and it would still be close to impossible
//     to make it "right". Note that there is however a non-member
//     operator< to make it possible to put a UT_String in a STL
//     (std C++ library) container. It _only_ provides strcmp ordering.
//
// [2] This is somewhat arbitrary, but it basically means you should
//     be able to use it with an old compiler.
//

//////////////////////////////////////////////////////////////////

static const char pszEmpty[] = { 0 };

static const UT_UCS4Char ucs4Empty[] = { 0 };


template <> ABI_EXPORT
const char* UT_StringImpl<UT_UCS4Char>::utf8_data() 
{ 
	if (m_utf8string) 
		return m_utf8string;

	size_t utf8length = size ();
	size_t bytelength = 0;
	size_t i;
	for (i = 0; i < utf8length; i++)
	{
		int seql = UT_Unicode::UTF8_ByteLength (m_psz[i]);
		if (seql < 0) 
			continue; // not UCS-4 !!
		if (seql == 0) 
			break; // huh? premature end-of-string?
		bytelength += static_cast<size_t>(seql);
	}
	m_utf8string = new char[bytelength+1];

	char * utf8string = m_utf8string;
	for (i = 0; i < utf8length; i++)
	{
		int seql = UT_Unicode::UTF8_ByteLength (m_psz[i]);
		if (seql < 0) 
			continue; // not UCS-4 !!
		if (seql == 0) 
			break; // huh? premature end-of-string?
		UT_Unicode::UCS4_to_UTF8 (utf8string, bytelength, m_psz[i]);
	}
	*utf8string = 0;

	return m_utf8string;
}


////////////////////////////////////////////////////////////////////////
//
//  8-bit string
//
//  String is built of 8-bit units (bytes)
//  Encoding could be any single-byte or multi-byte encoding
//
////////////////////////////////////////////////////////////////////////

UT_String::UT_String()
:	pimpl(new UT_StringImpl<char>)
{
}

UT_String::UT_String(const char* sz, size_t n)
:	pimpl(new UT_StringImpl<char>(sz, n ? n : (sz && *sz ? strlen(sz) : 0)))
{
}

UT_String::UT_String(const std::basic_string<char> & s)
	: pimpl(new UT_StringImpl<char>(s))
{
}


UT_String::UT_String(const UT_String& rhs)
:	pimpl(new UT_StringImpl<char>(*rhs.pimpl))
{
}

UT_String::~UT_String()
{
	delete pimpl;
}


//////////////////////////////////////////////////////////////////
// accessors

size_t UT_String::size() const
{
	return pimpl->size();
}

bool UT_String::empty() const
{
	return pimpl->empty();
}

void UT_String::clear() const
{
	pimpl->clear();
}

UT_String UT_String::substr(size_t iStart, size_t nChars) const
{
	const size_t nSize = pimpl->size();

	if (iStart >= nSize || !nChars) {
		return UT_String();
	}

	const char* p = pimpl->data() + iStart;
	if (iStart + nChars > nSize) {
		nChars = nSize - iStart;
	}

	return UT_String(p, nChars);
}

const char* UT_String::c_str() const
{
	return pimpl->size() ? pimpl->data() : pszEmpty;
}

//////////////////////////////////////////////////////////////////
// mutators

UT_String& UT_String::operator=(const UT_String& rhs)
{
	if (this != &rhs) {
		*pimpl = *rhs.pimpl;
	}
	return *this;
}

UT_String& UT_String::operator=(const char* rhs)
{
  if (!rhs || !*rhs)
    pimpl->clear ();
  else
    pimpl->assign(rhs, strlen(rhs));
  return *this;
}

UT_String& UT_String::operator=(const std::basic_string<char> & rhs)
{
	pimpl->assign(rhs.c_str(), rhs.size());
	return *this;
}


UT_String& UT_String::operator+=(const UT_String& rhs)
{
	if (this != &rhs) {
		pimpl->append(*rhs.pimpl);
	} else {
		UT_StringImpl<char> t(*rhs.pimpl);
		pimpl->append(t);
	}
	return *this;
}

// TODO What encoding do these functions think the
// TODO  right-hand character is in?  Same as the left-hand side?
// TODO  ASCII?  ISO-8859-1?  System encoding?
// TODO  any old 8-bit single-byte or multibyte encoding?

UT_String& UT_String::operator+=(const char* rhs)
{
	UT_return_val_if_fail(rhs && *rhs, *this);
	pimpl->append(rhs, strlen(rhs));
	return *this;
}


UT_String& UT_String::operator+=(char rhs)
{
	char cs = rhs;
	pimpl->append(&cs, 1);
	return *this;
}

void UT_String::swap(UT_String& rhs)
{
	std::swap(pimpl, rhs.pimpl);
}

void UT_String::reserve(size_t n)
{
        pimpl->reserve(n);
}

//////////////////////////////////////////////////////////////////
// End of class members, start of g_free functions
//////////////////////////////////////////////////////////////////

size_t UT_String_findCh(const UT_String &st, char ch)
{
  for (size_t i = 0 ; i < st.size(); i++)
    if (st[i] == ch)
      return i;
  return (size_t)-1;
}

size_t UT_String_findRCh(const UT_String &st, char ch)
{
  for (size_t i = st.size() ; i > 0; i--)
    if (st[i] == ch)
      return i;
  return (size_t)-1;
}

UT_String& UT_String_vprintf (UT_String & inStr, const char *format,
			      va_list      args1)
{
  char *buffer = g_strdup_vprintf(format, args1);
  inStr = buffer;
  g_free(buffer);

  return inStr;
}

UT_String& UT_String_vprintf (UT_String & inStr, const UT_String & format,
			va_list      args1)
{
  return UT_String_vprintf ( inStr, format.c_str(), args1 ) ;
}

UT_String& UT_String_sprintf(UT_String & inStr, const char * inFormat, ...)
{
  va_list args;
  va_start (args, inFormat);
  UT_String_vprintf (inStr, inFormat, args);
  va_end (args);

  return inStr;
}

UT_String UT_String_sprintf(const char * inFormat, ...)
{
  UT_String outStr ("");

  va_list args;
  va_start (args, inFormat);
  UT_String_vprintf (outStr, inFormat, args);
  va_end (args);

  return outStr;
}

UT_String UT_String_vprintf(const char * inFormat, va_list args1)
{
  UT_String outStr ("");  
  return UT_String_vprintf( outStr, inFormat, args1 );
}

UT_String UT_String_vprintf(const UT_String & inFormat, va_list args1)
{
  UT_String outStr ("");
  return UT_String_vprintf( outStr, inFormat, args1 );
}

/*!
 * Assuming a string of standard abiword properties eg. "fred:nerk; table-width:1.0in; table-height:10.in"
 * Return the value of the property sProp or NULL if it is not present.
 * This UT_String * should be deleted by the calling programming after it is finished with it.
 */
UT_String UT_String_getPropVal(const UT_String & sPropertyString, const UT_String & sProp)
{
	UT_String sWork(sProp);
	sWork += ":";

	const char * szWork = sWork.c_str();
	const char * szProps = sPropertyString.c_str();
	const char * szLoc = strstr(szProps,szWork);
	if(szLoc == NULL)
	{
		return UT_String();
	}
//
// Look if this is the last property in the string.
//
	const char * szDelim = strchr(szLoc,';');
	if(szDelim == NULL)
	{
//
// Remove trailing spaces
//
		UT_sint32 iSLen = strlen(szProps);
		while(iSLen > 0 && szProps[iSLen-1] == ' ')
		{
			iSLen--;
		}
//
// Calculate the location of the substring
//
		UT_sint32 offset = static_cast<UT_sint32>(reinterpret_cast<size_t>(szLoc) - reinterpret_cast<size_t>(szProps));
		offset += strlen(szWork);
		return UT_String(sPropertyString.substr(offset,(iSLen - offset)));
	}
	else
	{
		szDelim = strchr(szLoc,';');
		if(szDelim == NULL)
		{
//
// bad property string
//
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return UT_String();
		}
//
// Remove trailing spaces.
//
		while(*szDelim == ';' || *szDelim == ' ')
		{
			szDelim--;
		}
//
// Calculate the location of the substring
//
		UT_sint32 offset = static_cast<UT_sint32>(reinterpret_cast<size_t>(szLoc) - reinterpret_cast<size_t>(szProps));
		offset += strlen(szWork);
		UT_sint32 iLen = static_cast<UT_sint32>(reinterpret_cast<size_t>(szDelim) - reinterpret_cast<size_t>(szProps)) + 1;
		return UT_String(sPropertyString.substr(offset,(iLen - offset)));
	}
}
/*!
 * Assuming a string of standard abiword properties eg. "fred:nerk; table-width:1.0in; table-height:10.in"
 * Add aother propety string, updating previously defined properties with
 * values in the new string.
 */
void UT_String_addPropertyString(UT_String & sPropertyString, const UT_String & sNewProp)
{
	UT_sint32 iSize = static_cast<UT_sint32>(sNewProp.size());
	UT_sint32 iBase  =0;
	UT_String sProp;
	UT_String sVal;
	UT_String sSubStr;
	const char * szWork = NULL;
	const char * szLoc = NULL;
	while(iBase < iSize)
	{
		bool bBreakAtEnd = false;
		sSubStr = sNewProp.substr(iBase, iSize-iBase);
		szWork = sSubStr.c_str();
		szLoc = strstr(szWork,":");
		if(szLoc)
		{
			sProp = sNewProp.substr(iBase,szLoc - szWork);
		}
		else
		{
			break;
		}
		iBase += szLoc-szWork+1;
		sSubStr = sNewProp.substr(iBase, iSize-iBase);
		szWork = sSubStr.c_str();
		szLoc = strstr(szWork,";");
		if(szLoc)
		{
			sVal = sNewProp.substr(iBase,szLoc - szWork);
			iBase += szLoc-szWork+1;
		}
		else
		{
			sVal = sNewProp.substr(iBase,iSize-iBase);
			bBreakAtEnd = true;
		}
		if((sProp.size()>0) && (sVal.size() >0))
		{
			UT_String_setProperty(sPropertyString,sProp,sVal);
		}
		else
		{
			break;
		}
		if(bBreakAtEnd)
		{
			break;
		}
	}
}

/*!
 * Assuming a string of standard abiword properties eg. "fred:nerk; table-width:1.0in; table-height:10.in"
 * Add the property sProp with value sVal to the string of properties. If the property is already present, replace the 
 * old value with the new value.
 */
void UT_String_setProperty(UT_String & sPropertyString, const UT_String & sProp, const UT_String & sVal)
{
//
// Remove the old value if it exists and tack the new property on the end.
//
	UT_String_removeProperty(sPropertyString, sProp);
	if(sPropertyString.size() > 0)
	{
		sPropertyString += "; ";
	}
	sPropertyString += sProp;
	sPropertyString += ":";
	sPropertyString += sVal;
}

/*!
 * Assuming a string of standard abiword properties eg. "fred:nerk; table-width:1.0in; table-height:10.in"
 * Remove the property sProp and it's value from the string of properties. 
 */
void UT_String_removeProperty(UT_String & sPropertyString, const UT_String & sProp)
{
	UT_String sWork ( sProp );
	sWork += ":";
	const char * szWork = sWork.c_str();
	const char * szProps = sPropertyString.c_str();
	const char * szLoc = strstr(szProps,szWork);
	if(szLoc == NULL)
	{
		//Not here, do nothing
		return ;
	}
	// Check if this is a real match
	if (szLoc != szProps)
	{
		// This is not the first property. It could be a false match
		// for example, 'frame-col-xpos' and 'xpos'
		UT_String sWorkCheck("; ");
		sWorkCheck += sWork;
		const char * szLocCheck = strstr(szProps,sWorkCheck.c_str());
		if (!szLocCheck)
		{
			// False match
			return;
		}
		szLoc = szLocCheck;
	}	    

	UT_sint32 locLeft = static_cast<UT_sint32>(reinterpret_cast<size_t>(szLoc) - reinterpret_cast<size_t>(szProps));
	UT_String sLeft;
	if(locLeft == 0)
	{
		sLeft.clear();
	}
	else
	{
		sLeft = sPropertyString.substr(0,locLeft);
	}
	locLeft = static_cast<UT_sint32>(sLeft.size());

	UT_String sNew;
	if(locLeft > 0)
	{
		sNew = sLeft.substr(0,locLeft+1);
	}
	else
	{
		sNew.clear();
	}

	// Look for ";" to get right part
	const char * szDelim = strchr(szLoc,';');
	if(szDelim == NULL)
	{
		// No properties after this, just assign and return
		sPropertyString = sNew;
	}
	else
	{
		// Just slice off the properties and tack them onto the pre-existing sNew
		while(*szDelim == ';' || *szDelim == ' ')
		{
			szDelim++;
		}
		UT_sint32 offset = static_cast<UT_sint32>(reinterpret_cast<size_t>(szDelim) - reinterpret_cast<size_t>(szProps));
		UT_sint32 iLen = sPropertyString.size() - offset;
		if(sNew.size() > 0)
		{
			sNew += "; ";
		}
		sNew += sPropertyString.substr(offset,iLen);
		sPropertyString = sNew;
	}
}

//////////////////////////////////////////////////////////////////
// Helpers

bool operator==(const UT_String& s1, const UT_String& s2)
{
        if (s1.size() != s2.size()) return false;
	return strcmp(s1.c_str(), s2.c_str()) == 0;
}

bool operator==(const UT_String& s1, const char* s2)
{
	return strcmp(s1.c_str(), s2) == 0;
}

bool operator==(const char* s1, const UT_String& s2)
{
	return s2 == s1;
}

bool operator!=(const UT_String& s1, const UT_String& s2)
{
	return !(s1 == s2);
}

bool operator!=(const UT_String& s1, const char*  s2)
{
	return !(s1 == s2);
}

bool operator!=(const char* s1, const UT_String& s2)
{
	return !(s2 == s1);
}

bool operator<(const UT_String& s1, const UT_String& s2)
{
	return strcmp(s1.c_str(), s2.c_str()) < 0;
}

UT_String operator+(const UT_String& s1, const UT_String& s2)
{
	UT_String s(s1);
	s += s2;
	return s;
}

char UT_String::operator[](size_t iPos) const
{
	UT_ASSERT(iPos <= size());
	if (iPos == size())
		return '\0';
	return pimpl->data()[iPos];
}

char& UT_String::operator[](size_t iPos)
{
	UT_ASSERT(iPos <= size());
	return pimpl->data()[iPos];
}


UT_uint32 hashcode(const UT_String& string)
{
	// from glib
	return hashcode(string.c_str());
}

UT_uint32 hashcode(const char *p)
{
	// from glib
	UT_return_val_if_fail(p,0);
	UT_uint32 h = (UT_uint32)*p;
	
	if (h)
	{
		for (p += 1; *p != '\0'; p++)
		{
			h = (h << 5) - h + *p;
		}
	}
	
	return h;
}

////////////////////////////////////////////////////////////////////////
//
//  UTF-8 string: encoding is *always* UTF-8
//
////////////////////////////////////////////////////////////////////////

UT_UTF8String::UT_UTF8String () :
	pimpl(new UT_UTF8Stringbuf)
{
	// 
}

UT_UTF8String::UT_UTF8String (const char * sz, size_t n /* == 0 => null-termination */) :
	pimpl(new UT_UTF8Stringbuf(sz,n))
{
	// 
}

UT_UTF8String::UT_UTF8String (const char *str, const char *encoding)
{
	UT_uint32 iRead, iWritten;
	char *pUTF8Buf = UT_convert(str,
				    strlen(str),
				    encoding,
				    "UTF-8",
				    &iRead,
				    &iWritten);
	pimpl = new UT_UTF8Stringbuf(pUTF8Buf);
	FREEP(pUTF8Buf);
}


UT_UTF8String::UT_UTF8String (const UT_UTF8String & rhs)
	: pimpl(new UT_UTF8Stringbuf(*rhs.pimpl))
{
	// 
}

UT_UTF8String::UT_UTF8String (const UT_UCS4String & rhs) :
	pimpl(new UT_UTF8Stringbuf)
{
	if (rhs.size ()) appendUCS4 (rhs.ucs4_str (), rhs.size ());
}

UT_UTF8String::UT_UTF8String (const UT_UCS4Char * sz, size_t n) :
	pimpl(new UT_UTF8Stringbuf)
{
	appendUCS4 (sz, n);
}

UT_UTF8String::~UT_UTF8String ()
{
	delete pimpl;
}

size_t UT_UTF8String::size () const
{
	return pimpl->utf8Length ();
}

size_t UT_UTF8String::byteLength () const
{
	return pimpl->byteLength ();
}

void UT_UTF8String::dump (void) const
{
#if DEBUG
	char line[120];
	UT_sint32 i =0;
	const char * psz = utf8_str();
	while(psz && *psz)
	{
		for(i=0; (i< 60) && (*psz != 0); i++)
		{
			line[i] = *psz;
			psz++;
		}
		line[i] = 0;
		UT_DEBUGMSG(("%s \n",line));
		if(*psz == 0)
		{
			break;
		}
	}
#endif
}
bool UT_UTF8String::empty () const
{
	return pimpl->empty ();
}

void UT_UTF8String::clear () const
{
	pimpl->clear ();
}

void UT_UTF8String::reserve(size_t n)
{
        pimpl->reserve(n);
}

UT_UTF8String &	UT_UTF8String::operator=(const char * rhs)
{
  // treat null string assignment as a clear
  if (!rhs || !*rhs)
    pimpl->clear();
  else
    pimpl->assign (rhs);

  return *this;
}

UT_UTF8String &	UT_UTF8String::operator=(const std::string & rhs)
{
  // treat null string assignment as a clear
  if (rhs.size() == 0)
    pimpl->clear();
  else
    pimpl->assign (rhs.c_str());

  return *this;
}

UT_UTF8String &	UT_UTF8String::operator=(const UT_UTF8String & rhs)
{
	if (this != &rhs) {
		*pimpl = *rhs.pimpl;
	}
	return *this;
}


UT_UTF8String &	UT_UTF8String::operator+=(const UT_UCS4Char            rhs)
{
	pimpl->appendUCS4 (&rhs, 1);
	return *this;
}


UT_UTF8String &	UT_UTF8String::operator+=(const char * rhs)
{
	UT_return_val_if_fail(rhs, *this);
	if(*rhs)
		pimpl->append (rhs);
	return *this;
}

UT_UTF8String& UT_UTF8String::operator+=(const std::string& rhs)
{
	pimpl->append(rhs.c_str());
	return *this;
}

UT_UTF8String &	UT_UTF8String::operator+=(const UT_UTF8String & rhs)
{
	pimpl->append (*rhs.pimpl);
	return *this;
}

const char * UT_UTF8String::utf8_str () const
{
	return pimpl->utf8Length () ? pimpl->data() : pszEmpty;
}

void UT_UTF8String::assign (const char * sz, size_t n /* == 0 => null-termination */)
{
	pimpl->assign (sz, n);
}

void UT_UTF8String::append (const char * sz, size_t n /* == 0 => null-termination */)
{
	pimpl->append (sz, n);
}

void UT_UTF8String::appendBuf (const UT_ByteBuf & buf, UT_UCS4_mbtowc & converter)
{
	UT_uint32 i;
	UT_UCS4Char wc;
	const UT_Byte *ptr = buf.getPointer(0);
	
	for (i = 0; i < buf.getLength(); i++) 
	{
	  if (converter.mbtowc(wc, static_cast<char>(ptr[i])))
	        pimpl->appendUCS4(&wc, 1);
 	}
}

void UT_UTF8String::appendUCS4 (const UT_UCS4Char * sz, size_t n /* == 0 => null-termination */)
{
	pimpl->appendUCS4 (sz, n);
}

void UT_UTF8String::appendUCS2 (const UT_UCS2Char * sz, size_t n /* == 0 => null-termination */)
{
	pimpl->appendUCS2 (sz, n);
}

/* replaces <str1> with <str2> in the current string
 */
const UT_UTF8String & UT_UTF8String::escape (const UT_UTF8String & str1, const UT_UTF8String & str2)
{
	pimpl->escape (str1, str2);
	return *this;
}

/* escapes '<', '>', '"', & '&' in the current string
 */
const UT_UTF8String & UT_UTF8String::escapeXML ()
{
	pimpl->escapeXML ();
	return *this;
}

/* unescapes '<', '>', '"', & '&' in the current string
 */
const UT_UTF8String & UT_UTF8String::decodeXML ()
{
	pimpl->decodeXML ();
	return *this;
}

/* translates the current string to MIME "quoted-printable" format
 */
const UT_UTF8String & UT_UTF8String::escapeMIME ()
{
	pimpl->escapeMIME ();
	return *this;
}

/* makes string conform to RFC 1738
 */
const UT_UTF8String & UT_UTF8String::escapeURL ()
{
	pimpl->escapeURL ();
	return *this;
}

/* decodes %xx tokens in string
 */
const UT_UTF8String & UT_UTF8String::decodeURL ()
{
	pimpl->decodeURL ();
	return *this;
}

const UT_UTF8String & UT_UTF8String::lowerCase ()
{
	if(!byteLength())
		return *this;
	
	UT_UTF8Stringbuf * n = pimpl->lowerCase ();
	if(n)
	{
		delete pimpl;
		pimpl = n;
	}
	
	return *this;
}


UT_UTF8String  UT_UTF8String::substr(size_t iStart, size_t nChars) const
{
	const size_t nSize = pimpl->utf8Length ();

	if (iStart >= nSize || !nChars) {
		return UT_UTF8String();
	}

	const char* p = pimpl->data() + iStart;
	if (iStart + nChars > nSize) {
		nChars = nSize - iStart;
	}

	return UT_UTF8String(p, nChars);

}

///////////////////////////////////////////////////////////////////////////
//
// Martin's property string functions for UT_UTF8Strings.....
//
///////////////////////////////////////////////////////////////////////////

/*!
 * Assuming a string of standard abiword properties eg. "fred:nerk; table-width:1.0in; table-height:10.in"
 * Return the value of the property sProp or NULL if it is not present.
 * This UT_UTF8String * should be deleted by the calling programming after it is finished with it.
 */
UT_UTF8String UT_UTF8String_getPropVal(const UT_UTF8String & sPropertyString, const UT_UTF8String & sProp)
{
	UT_UTF8String sWork(sProp);
	sWork += ":";

	const char * szWork = sWork.utf8_str();
	const char * szProps = sPropertyString.utf8_str();
	const char * szLoc = strstr(szProps,szWork);
	if(szLoc == NULL)
	{
		return UT_UTF8String();
	}
//
// Look if this is the last property in the string.
//
	const char * szDelim = strchr(szLoc,';');
	if(szDelim == NULL)
	{
//
// Remove trailing spaces
//
		UT_sint32 iSLen = strlen(szProps);
		while(iSLen > 0 && szProps[iSLen-1] == ' ')
		{
			iSLen--;
		}
//
// Calculate the location of the substring
//
		UT_sint32 offset = static_cast<UT_sint32>(reinterpret_cast<size_t>(szLoc) - reinterpret_cast<size_t>(szProps));
		offset += strlen(szWork);
		return UT_UTF8String(sPropertyString.substr(offset,(iSLen - offset)));
	}
	else
	{
		szDelim = strchr(szLoc,';');
		if(szDelim == NULL)
		{
//
// bad property string
//
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return UT_UTF8String();
		}
//
// Remove trailing spaces.
//
		while(*szDelim == ';' || *szDelim == ' ')
		{
			szDelim--;
		}
//
// Calculate the location of the substring
//
		UT_sint32 offset = static_cast<UT_sint32>(reinterpret_cast<size_t>(szLoc) - reinterpret_cast<size_t>(szProps));
		offset += strlen(szWork);
		UT_sint32 iLen = static_cast<UT_sint32>(reinterpret_cast<size_t>(szDelim) - reinterpret_cast<size_t>(szProps)) + 1;
		return UT_UTF8String(sPropertyString.substr(offset,(iLen - offset)));
	}
}
/*!
 * Assuming a string of standard abiword properties eg. "fred:nerk; table-width:1.0in; table-height:10.in"
 * Add aother propety string, updating previously defined properties with
 * values in the new string.
 */
void UT_UTF8String_addPropertyString(UT_UTF8String & sPropertyString, const UT_UTF8String & sNewProp)
{
	UT_sint32 iSize = static_cast<UT_sint32>(sNewProp.size());
	UT_sint32 iBase  =0;
	UT_UTF8String sProp;
	UT_UTF8String sVal;
	UT_UTF8String sSubStr;
	const char * szWork = NULL;
	const char * szLoc = NULL;
	while(iBase < iSize)
	{
		bool bBreakAtEnd = false;
		sSubStr = sNewProp.substr(iBase, iSize-iBase);
		szWork = sSubStr.utf8_str();
		szLoc = strstr(szWork,":");
		UT_sint32 iextra = 0;
		if(szLoc)
		{
		        UT_sint32 k = iBase;
			while(*sNewProp.substr(k,k).utf8_str() == ' ')
			{
			  k++;
			  iextra++;
			}
			sProp = sNewProp.substr(k,szLoc - szWork-iextra);
		}
		else
		{
			break;
		}
		iBase += szLoc-szWork+1;
		sSubStr = sNewProp.substr(iBase, iSize-iBase);
		szWork = sSubStr.utf8_str();
		szLoc = strstr(szWork,";");
		if(szLoc)
		{
			sVal = sNewProp.substr(iBase,szLoc - szWork);
			iBase += szLoc-szWork+1;
		}
		else
		{
			sVal = sNewProp.substr(iBase,iSize-iBase);
			bBreakAtEnd = true;
		}
		if((sProp.size()>0) && (sVal.size() >0))
		{
			UT_UTF8String_setProperty(sPropertyString,sProp,sVal);
		}
		else
		{
			break;
		}
		if(bBreakAtEnd)
		{
			break;
		}
	}
}

/*!
 * Assuming a string of standard abiword properties eg. "fred:nerk; table-width:1.0in; table-height:10.in"
 * Add the property sProp with value sVal to the string of properties. If the property is already present, replace the 
 * old value with the new value.
 */
void UT_UTF8String_setProperty(UT_UTF8String & sPropertyString, const UT_UTF8String & sProp, const UT_UTF8String & sVal)
{
//
// Remove the old value if it exists and tack the new property on the end.
//
	UT_UTF8String_removeProperty(sPropertyString, sProp);
	if(sPropertyString.size() > 0)
	{
		sPropertyString += "; ";
	}
	sPropertyString += sProp;
	sPropertyString += ":";
	sPropertyString += sVal;
}

/*!
 * Assuming a string of standard abiword properties eg. "fred:nerk; table-width:1.0in; table-height:10.in"
 * Remove the property sProp and it's value from the string of properties. 
 */
void UT_UTF8String_removeProperty(UT_UTF8String & sPropertyString, const UT_UTF8String & sProp)
{
//
// Warning, warning!!! lots of brutal const casts and assignments into
// strings to handle utf8 encoding.
//
	size_t offset = 0;
	UT_UTF8String sWork ( sProp );
	sWork += ":";
	const char * szWork = sWork.utf8_str();
	const char * szProps = sPropertyString.utf8_str();
	const char * szLoc = strstr(szProps,szWork);

	if(!szLoc)
	{
		//Not here, do nothing
		return ;
	}

	// Check if this is a real match
	if (szLoc != szProps)
	{
		// This is not the first property. It could be a false match
		// for example, 'frame-col-xpos' and 'xpos'
		UT_UTF8String sWorkCheck("; ");
		sWorkCheck += sWork;
		const char * szLocCheck = strstr(szProps,sWorkCheck.utf8_str());
		if (!szLocCheck)
		{
			// False match
			return;
		}
		szLoc = szLocCheck;
		offset = 1;
	}	    

	UT_sint32 locLeft = static_cast<UT_sint32>(reinterpret_cast<size_t>(szLoc) - reinterpret_cast<size_t>(szProps));
	UT_UTF8String sLeft;
	if(locLeft == 0)
	{
		sLeft.clear();
	}
	else
	{
		sLeft = sPropertyString.substr(0,locLeft);
	}

	UT_UTF8String sNew;
	if(locLeft > 0)
	{
		sNew = sLeft;
	}
	else
	{
		sNew.clear();
	}
	//
	// Look for ";" to get right part
	//
	const char * szDelim = strchr(szLoc + offset,';');
	if(szDelim == NULL)
	{
		// No properties after this, just assign and return
		sPropertyString = sNew;
	}
	else
	{
		// Just slice off the properties and tack them onto the pre-existing sNew
		while(*szDelim == ';' || *szDelim == ' ')
		{
			szDelim++;
		}
		UT_UTF8String sRight = szDelim;
		if(sNew.size() > 0)
		{
			sNew += "; ";
		}
		sNew += sRight;
		sPropertyString = sNew;
	}
}

/////////////////////////////////////////////////////////////////////////////

UT_UCS4String UT_UTF8String::ucs4_str ()
{
	UT_UCS4String ucs4string;

	const char * utf8string = pimpl->data ();
	size_t bytelength = pimpl->byteLength ();

	while (true)
	{
		UT_UCS4Char ucs4 = UT_Unicode::UTF8_to_UCS4 (utf8string, bytelength);
		if (ucs4 == 0) 
			break;
		ucs4string += ucs4;
	}
	return ucs4string;
}

bool operator<(const UT_UTF8String& s1, const UT_UTF8String& s2)
{
	return strcmp(s1.utf8_str(), s2.utf8_str()) < 0;
}
bool operator==(const UT_UTF8String& s1, const UT_UTF8String& s2)
{
        if (s1.size() != s2.size()) return false;
	return strcmp(s1.utf8_str(), s2.utf8_str()) == 0;
}

bool operator!=(const UT_UTF8String& s1, const UT_UTF8String& s2)
{
	return !(s1 == s2);
}

bool operator==(const UT_UTF8String& s1, const char * s2)
{
	return s2 ? (strcmp(s1.utf8_str(), s2) == 0) : false;
}

bool operator!=(const UT_UTF8String& s1, const char * s2)
{
	return s2 ? (strcmp(s1.utf8_str(), s2) != 0) : true;
}

bool operator==(const UT_UTF8String& s1, const std::string &s2)
{
        if (s1.size() != s2.size()) return false;
	return s1.utf8_str() == s2;
}

bool operator!=(const UT_UTF8String& s1, const std::string &s2)
{
        if (s1.size() != s2.size()) return true;
	return s1.utf8_str() != s2;
}

bool operator==(const std::string &s2, const UT_UTF8String& s1)
{
	return s1.utf8_str() == s2;
}

bool operator!=(const std::string &s2, const UT_UTF8String& s1)
{
	return s1.utf8_str() != s2;
}

UT_UTF8String operator+(const UT_UTF8String & s1, const UT_UTF8String & s2)
{
	UT_UTF8String s(s1);
	s += s2;
	return s;
}

UT_UTF8String UT_UTF8String_sprintf(const char * inFormat, ...)
{
  UT_String str ("");

  va_list args;
  va_start (args, inFormat);
  UT_String_vprintf (str, inFormat, args);
  va_end (args);

  // create & return a validated UTF-8 string based on the input
  return UT_UTF8String(str.c_str());
}

UT_UTF8String & UT_UTF8String_sprintf(UT_UTF8String & inStr, const char * inFormat, ...)
{
  UT_String str ("");

  va_list args;
  va_start (args, inFormat);
  UT_String_vprintf (str, inFormat, args);
  va_end (args);

  // create a validated UTF-8 string based on the input
  inStr = str.c_str();
  return inStr;
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

UT_UCS4String::UT_UCS4String()
	: pimpl(new UT_StringImpl<UT_UCS4Char>)
{
}

UT_UCS4String::UT_UCS4String(const UT_UCS4Char* sz, size_t n)
	:	pimpl(new UT_StringImpl<UT_UCS4Char>(sz, n ? n : (sz) ? UT_UCS4_strlen(sz) : 0))
{
}

UT_UCS4String::UT_UCS4String(const UT_UCS4String& rhs)
	:	pimpl(new UT_StringImpl<UT_UCS4Char>(*rhs.pimpl))
{
}

void UT_UCS4String::_loadUtf8(const char * _utf8_str, size_t bytelength)
{
	UT_UCS4Char ucs4;
	do {
		ucs4 = UT_Unicode::UTF8_to_UCS4 (_utf8_str, bytelength);
		if (ucs4) {
			pimpl->append (&ucs4, 1);
		}
	} while(ucs4 != 0);
}


/* construct from a string in UTF-8 format
 */
UT_UCS4String::UT_UCS4String(const char * _utf8_str, size_t bytelength /* 0 == zero-terminate */)
	:	pimpl(new UT_StringImpl<UT_UCS4Char>)
{
	if (bytelength == 0) {
		if (_utf8_str == 0 || *_utf8_str == '\0') return;
		bytelength = strlen (_utf8_str);
	}
	_loadUtf8(_utf8_str, bytelength);
}

UT_UCS4String::UT_UCS4String(const std::string & str /* zero-terminated utf-8 encoded */)
	:	pimpl(new UT_StringImpl<UT_UCS4Char>)
{
	_loadUtf8(str.c_str(), str.size());
}

/* construct from a string in UTF-8 format
 * if (strip_whitespace == true) replace all white space sequences with a single UCS_SPACE
 * if (strip_whitespace != true) replace CR-LF & CR by LF
 * non-breaking spaces (&nbsp; UCS_NBSP 0x0a) are not white space
 */
UT_UCS4String::UT_UCS4String(const char * _utf8_str, size_t bytelength /* 0 == zero-terminate */, bool strip_whitespace)
	:	pimpl(new UT_StringImpl<UT_UCS4Char>)
{
	if (bytelength == 0) {
		if (_utf8_str == 0 || *_utf8_str == '\0') return;
		bytelength = strlen (_utf8_str);
	}
	UT_UCS4Char ucs4a = UT_Unicode::UTF8_to_UCS4 (_utf8_str, bytelength);
	while (true) {
		if (ucs4a == 0) 
			break; // end-of-string
		UT_UCS4Char ucs4b = UT_Unicode::UTF8_to_UCS4 (_utf8_str, bytelength);
		if ((UCS_NBSP != ucs4a) && UT_UCS4_isspace (ucs4a)) {
			if (strip_whitespace) {
				if (!UT_UCS4_isspace (ucs4b)) {
					ucs4a = UCS_SPACE;
					pimpl->append (&ucs4a, 1);
					ucs4a = ucs4b;
				}
			} else if (ucs4a == UCS_CR) {
				if (ucs4b == UCS_LF) {
					ucs4a = ucs4b;
				} else {
					ucs4a = UCS_LF;
					pimpl->append (&ucs4a, 1);
					ucs4a = ucs4b;
				}
			} else {
				pimpl->append (&ucs4a, 1);
				ucs4a = ucs4b;
			}
		} else {
			pimpl->append (&ucs4a, 1);
			ucs4a = ucs4b;
		}
	}
}

UT_UCS4String::~UT_UCS4String()
{
	delete pimpl;
}


//////////////////////////////////////////////////////////////////
// accessors

size_t UT_UCS4String::size() const
{
	return pimpl->size();
}

bool UT_UCS4String::empty() const
{
	return pimpl->empty();
}

void UT_UCS4String::clear() const
{
	pimpl->clear();
}

UT_UCS4String UT_UCS4String::substr( const UT_UCS4Char* iter ) const
{
	const size_t nSize = pimpl->size();
    const UT_UCS4Char* b = ucs4_str();
    size_t i = 0;
    for( ; i<nSize && b != iter ; )
    {
        ++b;
        ++i;
    }
    return substr( i );
}


UT_UCS4String UT_UCS4String::substr(size_t iStart) const
{
	const size_t nSize = pimpl->size();
    if( iStart >= nSize )
    {
        return UT_UCS4String();
	}
    size_t nChars = nSize - iStart;
    return substr( iStart, nChars );
}


UT_UCS4String UT_UCS4String::substr(size_t iStart, size_t nChars) const
{
	const size_t nSize = pimpl->size();

	if (iStart >= nSize || !nChars) {
		return UT_UCS4String();
	}

	const UT_UCS4Char* p = pimpl->data() + iStart;
	if (iStart + nChars > nSize) {
		nChars = nSize - iStart;
	}

	return UT_UCS4String(p, nChars);
}

const UT_UCS4Char* UT_UCS4String::ucs4_str() const
{
	return pimpl->size() ? pimpl->data() : ucs4Empty;
}

const char* UT_UCS4String::utf8_str()
{
	return pimpl->size() ? pimpl->utf8_data() : pszEmpty;
}

const UT_UCS4Char* UT_UCS4String::begin() const
{
	return pimpl->size() ? pimpl->data() : 0;
}

const UT_UCS4Char* UT_UCS4String::end() const
{
    const UT_UCS4Char* b = begin();
    if( !b )
        return b;
    
    std::advance( b, length() );
    return b;
}



//////////////////////////////////////////////////////////////////
// mutators

void UT_UCS4String::reserve(size_t n)
{
        pimpl->reserve(n);
}

UT_UCS4String& UT_UCS4String::operator=(const UT_UCS4String& rhs)
{
	if (this != &rhs) {
		*pimpl = *rhs.pimpl;
	}
	return *this;
}

UT_UCS4String& UT_UCS4String::operator=(const UT_UCS4Char* rhs)
{
	UT_return_val_if_fail(rhs, *this);
	pimpl->assign(rhs, UT_UCS4_strlen(rhs));
	return *this;
}

UT_UCS4String& UT_UCS4String::operator+=(const UT_UCS4String& rhs)
{
	if (this != &rhs) {
		pimpl->append(*rhs.pimpl);
	} else {
		UT_StringImpl<UT_UCS4Char> t(*rhs.pimpl);
		pimpl->append(t);
	}
	return *this;
}

UT_UCS4String& UT_UCS4String::operator+=(const UT_UCS4Char* rhs)
{
	UT_return_val_if_fail(rhs, *this);
	pimpl->append(rhs, UT_UCS4_strlen(rhs));
	return *this;
}

UT_UCS4String& UT_UCS4String::operator+=(UT_UCS4Char rhs)
{
	UT_UCS4Char cs = rhs;
	pimpl->append(&cs, 1);
	return *this;
}

// TODO What encoding do these functions think the 8-bit
// TODO  character is in?  ASCII?  ISO-8859-1?  System encoding?
// TODO  any old 8-bit single-byte or multibyte encoding?

UT_UCS4String& UT_UCS4String::operator+=(char rhs)
{
	return this->operator+=(static_cast<unsigned char>(rhs));
}

UT_UCS4String& UT_UCS4String::operator+=(unsigned char rhs)
{
	UT_UCS4Char cs[2];
	char rs[2];

	rs[0] = static_cast<char>(rhs); rs[1] = 0;
	UT_UCS4_strcpy_char (cs, rs);

	pimpl->append(cs, 1);
	return *this;
}

void UT_UCS4String::swap(UT_UCS4String& rhs)
{
	std::swap(pimpl, rhs.pimpl);
}


//////////////////////////////////////////////////////////////////
// End of class members, start of g_free functions
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// Helpers

bool operator==(const UT_UCS4String& s1, const UT_UCS4String& s2)
{
        if (s1.size() != s2.size()) return false;
	return UT_UCS4_strcmp(s1.ucs4_str(), s2.ucs4_str()) == 0;
}

bool operator==(const UT_UCS4String& s1, const UT_UCS4Char* s2)
{
	return UT_UCS4_strcmp(s1.ucs4_str(), s2) == 0;
}

bool operator==(const UT_UCS4Char* s1, const UT_UCS4String& s2)
{
	return s2 == s1;
}

bool operator!=(const UT_UCS4String& s1, const UT_UCS4String& s2)
{
	return !(s1 == s2);
}

bool operator!=(const UT_UCS4String& s1, const UT_UCS4Char*  s2)
{
	return !(s1 == s2);
}

bool operator!=(const UT_UCS4Char* s1, const UT_UCS4String& s2)
{
	return !(s2 == s1);
}

bool operator<(const UT_UCS4String& s1, const UT_UCS4String& s2)
{
	return UT_UCS4_strcmp(s1.ucs4_str(), s2.ucs4_str()) < 0;
}

UT_UCS4String operator+(const UT_UCS4String& s1, const UT_UCS4String& s2)
{
	UT_UCS4String s(s1);
	s += s2;
	return s;
}

UT_UCS4Char UT_UCS4String::operator[](size_t iPos) const
{
	UT_ASSERT(iPos <= size());
	if (iPos == size())
		return '\0';
	return pimpl->data()[iPos];
}

UT_UCS4Char& UT_UCS4String::operator[](size_t iPos)
{
	UT_ASSERT(iPos <= size());
	return pimpl->data()[iPos];
}
