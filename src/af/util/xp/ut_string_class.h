// ut_string_class.h
//
// A simple string class for use where templates are not
// allowed.
//
#ifndef UT_STRING_CLASS_H
#define UT_STRING_CLASS_H

//
// Copyright (C) 2001 Mike Nordell <tamlin@algonet.se>
// Copyright (C) 2001 Dom Lachowicz <dominicl@seas.upenn.edu>
// Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net> 
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
#include <stdarg.h>
#include "ut_string.h"
#include "ut_stringbuf.h"
#include "ut_types.h"

// yes, this is screaming for a template

//!
//	UT_String, a simple wrapper for zero terminated 'char' strings.
//
class ABI_EXPORT UT_String
{
public:
	UT_String();
	UT_String(const char* sz, size_t n = 0 /* 0 == zero-terminate */);
	UT_String(const UT_String& rhs);
	~UT_String();

	size_t		size() const;
	size_t length () const { return size () ; }

	bool		empty() const;
	void        clear() const;

	UT_String	substr(size_t iStart, size_t nChars) const;

	UT_String&	operator=(const UT_String& rhs);
	UT_String&	operator=(const char*      rhs);
	UT_String&	operator+=(const UT_String& rhs);
	UT_String&	operator+=(const char*      rhs);
	UT_String&  operator+=(char rhs);

	char		operator[](size_t iPos) const;
	char&		operator[](size_t iPos);

	void		swap(UT_String& rhs);

	// The returned pointer is valid until the next non-const
	// operation. You will _always_ get a legal pointer back,
	// even if to an empty string.
	const char* c_str() const;

private:
	class UT_Stringbuf* pimpl;
};

// helpers
ABI_EXPORT bool operator==(const UT_String& s1, const UT_String& s2);
ABI_EXPORT bool operator==(const UT_String& s1, const char*      s2);
ABI_EXPORT bool operator==(const char*      s1, const UT_String& s2);
ABI_EXPORT bool operator!=(const UT_String& s1, const UT_String& s2);
ABI_EXPORT bool operator!=(const UT_String& s1, const char*      s2);
ABI_EXPORT bool operator!=(const char*      s1, const UT_String& s2);

ABI_EXPORT UT_uint32 hashcode(const UT_String& string);

// strcmp ordering
ABI_EXPORT bool operator<(const UT_String& s1, const UT_String& s2);

ABI_EXPORT UT_String operator+(const UT_String& s1, const UT_String& s2);

/****************************************************************************/

/*!
 * Fill \inStr with the results of evaulating the printf formatted string 
 * \inFormat and return the reference to \inStr
 */
ABI_EXPORT UT_String& UT_String_sprintf(UT_String & inStr, const char * inFormat, ...);
ABI_EXPORT UT_String& UT_String_vprintf (UT_String & inStr, const char *format,
				   va_list      args1);
ABI_EXPORT UT_String& UT_String_vprintf (UT_String & inStr, const UT_String & format,
					 va_list      args1);

/*!
 * Returns a new UT_String object with the results of evaluating the printf
 * formatted string \inFormat
 */
ABI_EXPORT UT_String UT_String_sprintf(const char * inFormat, ...);
ABI_EXPORT UT_String UT_String_vprintf(const char * inFormat, va_list args1);
ABI_EXPORT UT_String UT_String_vprintf(const UT_String & inFormat, va_list args1);

/***************************************************************************/

//!
//	UT_UCS2String, a simple wrapper for zero terminated 'UCS2' strings.
//

// TODO: add c_str(), utf8_str(), encoded_str(const char * to)

class ABI_EXPORT UT_UCS2String
{
public:
	UT_UCS2String();
	UT_UCS2String(const UT_UCS2Char * sz, size_t n = 0 /* 0 == zero-terminate */);
	UT_UCS2String(const UT_UCS2String& rhs);
	~UT_UCS2String();

	size_t		size() const;
	size_t length () const { return size () ; }

	bool		empty() const;
	void        clear() const;

	UT_UCS2String	substr(size_t iStart, size_t nChars) const;

	UT_UCS2String&	operator=(const UT_UCS2String&  rhs);
	UT_UCS2String&	operator=(const UT_UCS2Char *    rhs);
	UT_UCS2String&	operator+=(const UT_UCS2String& rhs);
	UT_UCS2String&	operator+=(const UT_UCS2Char *   rhs);
	UT_UCS2String&  operator+=(UT_UCS2Char rhs);
	UT_UCS2String&  operator+=(char rhs);
	UT_UCS2String&  operator+=(unsigned char rhs);

	UT_UCS2Char		operator[](size_t iPos) const;
	UT_UCS2Char&	operator[](size_t iPos);

	void		swap(UT_UCS2String& rhs);

	// The returned pointer is valid until the next non-const
	// operation. You will _always_ get a legal pointer back,
	// even if to an empty (0) string.
	const UT_UCS2Char* ucs2_str() const;
	const UT_UCS4Char* ucs4_str();

private:
	class UT_UCS2Stringbuf* pimpl;
	UT_UCS4Char* m_pUCS4;
	UT_uint32    m_iUCS4Size;
};

// helpers
bool operator==(const UT_UCS2String& s1, const UT_UCS2String& s2);
bool operator==(const UT_UCS2String& s1, const UT_UCS2Char *  s2);
bool operator==(const UT_UCS2Char *  s1, const UT_UCS2String& s2);
bool operator!=(const UT_UCS2String& s1, const UT_UCS2String& s2);
bool operator!=(const UT_UCS2String& s1, const UT_UCS2Char *  s2);
bool operator!=(const UT_UCS2Char *  s1, const UT_UCS2String& s2);

// strcmp ordering
bool operator<(const UT_UCS2String& s1, const UT_UCS2String& s2);

UT_UCS2String operator+(const UT_UCS2String& s1, const UT_UCS2String& s2);

/***************************************************************************/

//!
//	UT_UCS4String, a simple wrapper for zero terminated 'UCS4' strings.
//

// TODO: add c_str(), utf8_str(), encoded_str(const char * to)

class ABI_EXPORT UT_UCS4String
{
public:
	UT_UCS4String();
	UT_UCS4String(const UT_UCS4Char * sz, size_t n = 0 /* 0 == zero-terminate */);
	UT_UCS4String(const UT_UCS4String& rhs);
	UT_UCS4String(const UT_UCS2String& rhs);

	/* construct from a string in UTF-8 format
	 */
	UT_UCS4String(const char * utf8_str, size_t bytelength = 0 /* 0 == zero-terminate */);

	/* construct from a string in UTF-8 format
	 * if (strip_whitespace == true) replace all white space sequences with a single UCS_SPACE
	 * if (strip_whitespace != true) replace CR-LF & CR by LF
	 * non-breaking spaces (&nbsp; UCS_NBSP 0x0a) are not white space; see UT_UCS4_isspace()
	 */
	UT_UCS4String(const char * utf8_str, size_t bytelength /* 0 == zero-terminate */, bool strip_whitespace);

	~UT_UCS4String();

	size_t		size() const;
	size_t length () const { return size () ; }

	bool		empty() const;
	void        clear() const;

	UT_UCS4String	substr(size_t iStart, size_t nChars) const;

	UT_UCS4String&	operator=(const UT_UCS4String&  rhs);
	UT_UCS4String&	operator=(const UT_UCS4Char *   rhs);
	UT_UCS4String&	operator+=(const UT_UCS4String& rhs);
	UT_UCS4String&	operator+=(const UT_UCS4Char *  rhs);
	UT_UCS4String&  operator+=(UT_UCS4Char rhs);
	UT_UCS4String&  operator+=(char rhs);
	UT_UCS4String&  operator+=(unsigned char rhs);

	UT_UCS4Char		operator[](size_t iPos) const;
	UT_UCS4Char&	operator[](size_t iPos);

	void		swap(UT_UCS4String& rhs);

	// The returned pointer is valid until the next non-const
	// operation. You will _always_ get a legal pointer back,
	// even if to an empty (0) string.
	const UT_UCS4Char* ucs4_str() const;

private:
	class UT_UCS4Stringbuf* pimpl;
};

// helpers
bool operator==(const UT_UCS4String& s1, const UT_UCS4String& s2);
bool operator==(const UT_UCS4String& s1, const UT_UCS4Char *  s2);
bool operator==(const UT_UCS4Char *  s1, const UT_UCS4String& s2);
bool operator!=(const UT_UCS4String& s1, const UT_UCS4String& s2);
bool operator!=(const UT_UCS4String& s1, const UT_UCS4Char *  s2);
bool operator!=(const UT_UCS4Char *  s1, const UT_UCS4String& s2);

// strcmp ordering
bool operator<(const UT_UCS4String& s1, const UT_UCS4String& s2);

UT_UCS4String operator+(const UT_UCS4String& s1, const UT_UCS4String& s2);


/***************************************************************************/

//!
//	UT_UTF8String, a simple wrapper for zero terminated 'UTF8' strings.
//

class ABI_EXPORT UT_UTF8String
{
public:
	UT_UTF8String ();
	UT_UTF8String (const char * sz);
	UT_UTF8String (const UT_UTF8String & rhs);
	UT_UTF8String (const UT_UCS2String & rhs);
	UT_UTF8String (const UT_UCS4String & rhs);
	UT_UTF8String (const UT_UCSChar * sz, size_t n = 0 /* 0 == zero-terminate */);

	~UT_UTF8String ();

	size_t		size () const;
	size_t length () const { return size () ; }

	bool		empty () const;
	void		clear () const;
	size_t		byteLength() const;

	UT_UTF8String &	operator=(const char *          rhs);
	UT_UTF8String &	operator=(const UT_UTF8String & rhs);
	UT_UTF8String &	operator=(const UT_UCS2String & rhs);
	UT_UTF8String &	operator=(const UT_UCS4String & rhs);

	UT_UTF8String &	operator+=(const char *          rhs);
	UT_UTF8String &	operator+=(const UT_UTF8String & rhs);
	UT_UTF8String &	operator+=(const UT_UCS2String & rhs);
	UT_UTF8String &	operator+=(const UT_UCS4String & rhs);

	// The returned pointer is valid until the next non-const
	// operation. You will _always_ get a legal pointer back,
	// even if to an empty (0) string.
	const char * utf8_str () const;

	UT_UCS2String ucs2_str ();

	void appendUCS4 (const UT_UCS4Char * sz, size_t n = 0 /* 0 == zero-terminate */);
	void appendUCS2 (const UT_UCS2Char * sz, size_t n = 0 /* 0 == zero-terminate */);

	/* UTF8String - NOTES
	 * 
	 * TODO:
	 * 1. Maybe have a search&replace function, something like:
	 * 
	 * 	int replace (const char * utf_newstr, const char * utf_oldstr);
	 * 
	 *    which could be used to do substitutions, e.g.:
	 * 
	 * 	UTF8String xmlstr = "expr: if ((c > 0) && (c < 0x80)) return c;";
	 * 	xmlstr.replace ("&lt;", "<");
	 * 	xmlstr.replace ("&gt;", ">");
	 * 	xmlstr.replace ("&amp;","&");
	 * 
	 * getIterator:
	 * returns a home-made iterator associated with the UTF-8 string, e.g.:
	 * 
	 * 	UTF8String str = "This is a UTF-8 string.";
	 * 	UT_UTF8Stringbuf::UTF8Iterator & iter = str.getIterator ();
	 * 	iter = iter.start (); // iter.start() returns 0 if no string, so:
	 * 	if (iter.current ())
	 * 	{
	 * 		while (true)
	 * 		{
	 * 			char * pUTF = iter.current ();
	 * 			if (*pUTF == 0) break; // end-of-string
	 * 			// etc.
	 * 			iter.advance (); // or ++iter;
	 * 		}
	 * 	}
	 * 
	 * The iterator will be well behaved provided the string is not being edited.
	 */
	UT_UTF8Stringbuf::UTF8Iterator getIterator () const
	{
		return UT_UTF8Stringbuf::UTF8Iterator(pimpl);
	}

private:
	class UT_UTF8Stringbuf * pimpl;
};

UT_UTF8String operator+(const UT_UTF8String & s1, const UT_UTF8String & s2);


#endif	// UT_STRING_CLASS_H

