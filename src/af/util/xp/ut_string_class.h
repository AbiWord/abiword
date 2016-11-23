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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301 USA.
//

#include <stdlib.h>
#include <stdarg.h>

#if defined(__MINGW32__)
#  undef snprintf
#  if __GNUC__ <= 3
#    define _GLIBCXX_USE_C99_DYNAMIC 1
#  endif
#endif

#include <string>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_string.h"
#include "ut_stringbuf.h"
#include "ut_bytebuf.h"

// Forward declarations
class UT_UCS4_mbtowc;
class UT_String;
class UT_UTF8String;
class UT_UCS4String;


// yes, this is screaming for a template

////////////////////////////////////////////////////////////////////////
//
//  8-bit string
//
//  String is built of 8-bit units (bytes)
//  Encoding could be any single-byte or multi-byte encoding
//
////////////////////////////////////////////////////////////////////////

//!
//	UT_String, a simple wrapper for zero terminated 'char' strings.
//
class ABI_EXPORT UT_String
{
public:
	UT_String();
	UT_String(const char* sz, size_t n = 0 /* 0 == zero-terminate */);
	UT_String(const UT_String& rhs);
	UT_String(const std::basic_string<char> &s);
	~UT_String();

	size_t		size() const;
	size_t length () const { return size () ; }
	void            reserve(size_t n);
	bool		empty() const;
	void        clear() const;

	UT_String	substr(size_t iStart, size_t nChars) const;

	UT_String&	operator=(const UT_String& rhs);
	UT_String&	operator=(const char*      rhs);
	UT_String&	operator=(const std::basic_string<char> & rhs);
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
	class UT_StringImpl<char>* pimpl;
};

// helpers
ABI_EXPORT bool operator==(const UT_String& s1, const UT_String& s2);
ABI_EXPORT bool operator==(const UT_String& s1, const char*      s2);
ABI_EXPORT bool operator==(const char*      s1, const UT_String& s2);
ABI_EXPORT bool operator!=(const UT_String& s1, const UT_String& s2);
ABI_EXPORT bool operator!=(const UT_String& s1, const char*      s2);
ABI_EXPORT bool operator!=(const char*      s1, const UT_String& s2);

ABI_EXPORT UT_uint32 hashcode(const UT_String& string);
ABI_EXPORT UT_uint32 hashcode(const char *s);

// strcmp ordering
ABI_EXPORT bool operator<(const UT_String& s1, const UT_String& s2);

ABI_EXPORT UT_String operator+(const UT_String& s1, const UT_String& s2);

ABI_EXPORT size_t UT_String_findCh(const UT_String &st, char ch);
ABI_EXPORT size_t UT_String_findRCh(const UT_String &st, char ch);

/****************************************************************************/

/*!
 * Fill \inStr with the results of evaulating the printf formatted string
 * \inFormat and return the reference to \inStr
 */
ABI_EXPORT UT_String& UT_String_sprintf(UT_String & inStr, const char * inFormat, ...) ABI_PRINTF_FORMAT(2,3);
ABI_EXPORT UT_String& UT_String_vprintf (UT_String & inStr, const char *format,
                                         va_list      args1)
    ABI_PRINTF_FORMAT(2,0);
ABI_EXPORT UT_String& UT_String_vprintf (UT_String & inStr, const UT_String & format,
					 va_list      args1);

/*!
 * Returns a new UT_String object with the results of evaluating the printf
 * formatted string \inFormat
 */
ABI_EXPORT UT_String UT_String_sprintf(const char * inFormat, ...)
    ABI_PRINTF_FORMAT(1,2);
ABI_EXPORT UT_String UT_String_vprintf(const char * inFormat, va_list args1)
    ABI_PRINTF_FORMAT(1,0);
ABI_EXPORT UT_String UT_String_vprintf(const UT_String & inFormat, va_list args1);

/***************************************************************************/

/***************************************************************************/
/*!
 * Some functions to add/subtract and extract UT_String properties from a UT_String of properties.
 */

ABI_EXPORT UT_String UT_String_getPropVal(const UT_String & sPropertyString, const UT_String & sProp);
ABI_EXPORT void UT_String_removeProperty(UT_String & sPropertyString, const UT_String & sProp);
ABI_EXPORT void UT_String_setProperty(UT_String & sPropertyString, const UT_String &sProp, const UT_String & sVal);
ABI_EXPORT void UT_String_addPropertyString(UT_String & sPropertyString, const UT_String & sNewProp);

////////////////////////////////////////////////////////////////////////
//
//  UTF-8 string: encoding is *always* UTF-8
//
////////////////////////////////////////////////////////////////////////

//!
//	UT_UTF8String, a simple wrapper for zero terminated 'UTF-8' strings.
//

class ABI_EXPORT UT_UTF8String
{
public:
	UT_UTF8String ();
	UT_UTF8String (const char * sz, size_t n = 0 /* 0 == null-termination */);
	UT_UTF8String (const char *sz, const char *encoding);

	UT_UTF8String (const UT_UTF8String & rhs);
	UT_UTF8String (const UT_UCS4String & rhs);
	UT_UTF8String (const UT_UCSChar * sz, size_t n = 0 /* 0 == zero-terminate */);

	~UT_UTF8String ();

	size_t		size () const;
	size_t length () const { return size () ; }

	void            reserve(size_t n);
	bool		empty () const;
	void		clear () const;
	size_t		byteLength() const;
	void        dump(void) const;
	UT_UTF8String	substr(size_t iStart, size_t nChars) const;

	UT_UTF8String &	operator=(const char *          rhs);
	UT_UTF8String &	operator=(const std::string &   rhs);
	UT_UTF8String &	operator=(const UT_UTF8String & rhs);
	UT_UTF8String &	operator=(const UT_UCS4String & rhs);

	UT_UTF8String &	operator+=(const UT_UCS4Char     rhs);
	UT_UTF8String &	operator+=(const char *          rhs);
	UT_UTF8String &	operator+=(const std::string &   rhs);
	UT_UTF8String &	operator+=(const UT_UTF8String & rhs);
	UT_UTF8String &	operator+=(const UT_UCS4String & rhs);

	// The returned pointer is valid until the next non-const
	// operation. You will _always_ get a legal pointer back,
	// even if to an empty (0) string.
	const char * utf8_str () const;
	UT_UCS4String ucs4_str ();

	void		assign (const char * sz, size_t n = 0 /* 0 == null-termination */);
	void		append (const char * sz, size_t n = 0 /* 0 == null-termination */);
	void        appendBuf(const UT_ConstByteBufPtr & buf, UT_UCS4_mbtowc & converter);

	void		appendUCS4 (const UT_UCS4Char * sz, size_t n = 0 /* 0 == null-termination */);
	void		appendUCS2 (const UT_UCS2Char * sz, size_t n = 0 /* 0 == null-termination */);

	const UT_UTF8String & escape (const UT_UTF8String & str1,
				      const UT_UTF8String & str2);  // replaces <str1> with <str2> in the current string
	const UT_UTF8String & escapeXML ();  // escapes '<', '>', '"', & '&' in the current string
	const UT_UTF8String & decodeXML ();  // unescapes '<', '>', '"', & '&' in the current string
	const UT_UTF8String & escapeMIME (); // translates the current string to MIME "quoted-printable" format
	const UT_UTF8String & lowerCase ();  // forces current string to lowercase
	const UT_UTF8String & escapeURL ();  // make URL confirm to RFC 1738
	const UT_UTF8String & decodeURL ();

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
	 * 	MIQ: Note that for these replace methods, one might use ut_std_string/replace_all()
	 *
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

ABI_EXPORT bool operator<(const UT_UTF8String& s1, const UT_UTF8String& s2);
ABI_EXPORT bool operator==(const UT_UTF8String& s1, const UT_UTF8String& s2);
ABI_EXPORT bool operator!=(const UT_UTF8String& s1, const UT_UTF8String& s2);
ABI_EXPORT bool operator==(const UT_UTF8String& s1, const char * s2);
ABI_EXPORT bool operator!=(const UT_UTF8String& s1, const char * s2);
ABI_EXPORT bool operator==(const UT_UTF8String& s1, const std::string & s2);
ABI_EXPORT bool operator!=(const UT_UTF8String& s1, const std::string & s2);
ABI_EXPORT bool operator==(const std::string & s2, const UT_UTF8String& s1);
ABI_EXPORT bool operator!=(const std::string & s2, const UT_UTF8String& s1);
ABI_EXPORT UT_UTF8String operator+(const UT_UTF8String & s1, const UT_UTF8String & s2);
ABI_EXPORT UT_UTF8String UT_UTF8String_sprintf(const char * inFormat, ...);
ABI_EXPORT UT_UTF8String & UT_UTF8String_sprintf(UT_UTF8String & inStr, const char * inFormat, ...);


/***************************************************************************/
/*!
 * Some functions to add/subtract and extract UT_String properties from a UT_String of properties.
 */

ABI_EXPORT UT_UTF8String UT_UTF8String_getPropVal(const UT_UTF8String & sPropertyString, const UT_UTF8String & sProp);

ABI_EXPORT void UT_UTF8String_removeProperty(UT_UTF8String & sPropertyString, const UT_UTF8String & sProp);

ABI_EXPORT void UT_UTF8String_setProperty(UT_UTF8String & sPropertyString, const UT_UTF8String &sProp, const UT_UTF8String & sVal);

ABI_EXPORT void UT_UTF8String_addPropertyString(UT_UTF8String & sPropertyString, const UT_UTF8String & sNewProp);

ABI_EXPORT void UT_UTF8String_replaceString(UT_UTF8String & sString, const UT_UTF8String & sOldValue,const UT_UTF8String & sNewValue );

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

//!
//	UT_UCS4String, a simple wrapper for zero terminated 'UCS4' strings.
//

// TODO: add c_str(), encoded_str(const char * to)

class ABI_EXPORT UT_UCS4String
{
public:
	UT_UCS4String();
	UT_UCS4String(const UT_UCS4Char * sz, size_t n = 0 /* 0 == zero-terminate */);
	UT_UCS4String(const UT_UCS4String& rhs);

	/* construct from a string in UTF-8 format
	 */
	UT_UCS4String(const char * utf8_str, size_t bytelength = 0 /* 0 == zero-terminate */);
	UT_UCS4String(const std::string & str /* zero-terminated utf-8 encoded */);

	/* construct from a string in UTF-8 format
	 * if (strip_whitespace == true) replace all white space sequences with a single UCS_SPACE
	 * if (strip_whitespace != true) replace CR-LF & CR by LF
	 * non-breaking spaces (&nbsp; UCS_NBSP 0x0a) are not white space; see UT_UCS4_isspace()
	 */
	UT_UCS4String(const char * utf8_str, size_t bytelength /* 0 == zero-terminate */, bool strip_whitespace);

	~UT_UCS4String();

	size_t	size() const;
	size_t length () const { return size () ; }

	void            reserve(size_t n);
	bool		empty() const;
	void        clear() const;

	UT_UCS4String	substr(size_t iStart, size_t nChars) const;
	UT_UCS4String	substr(size_t iStart) const;
	UT_UCS4String	substr( const UT_UCS4Char* iter ) const;

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

    // The same valid constraints as ucs4_str() applies to begin and end
    const UT_UCS4Char* begin() const;
    const UT_UCS4Char* end()   const;

	const char * utf8_str ();

private:
	void _loadUtf8(const char * utf8_str, size_t bytelength); // implementation detail for the UTF-8 constructor
	class UT_StringImpl<UT_UCS4Char>* pimpl;
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



#endif	// UT_STRING_CLASS_H
