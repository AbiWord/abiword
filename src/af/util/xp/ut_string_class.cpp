// ut_string_class.cpp

// A simple string class for use where templates are not
// allowed.
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
#include <stdlib.h>				// size_t
#include <string.h>				// strcmp
#include <locale.h>
#include <ctype.h>
#include <stdarg.h>

#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_stringbuf.h"
#include "ut_debugmsg.h"		// UT_DEBUGMSG
#include "ut_assert.h"			// UT_ASSERT

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
//   and finally 3) locking slow us down.
//
// [1] It's impossible to get a soring order other than plain strcmp
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

static const char pszEmpty[] = "";


UT_String::UT_String()
:	pimpl(new UT_Stringbuf)
{
}

UT_String::UT_String(const char* sz, size_t n)
:	pimpl(new UT_Stringbuf(sz, n ? n : (sz) ? strlen(sz) : 0))
{
}

UT_String::UT_String(const UT_String& rhs)
:	pimpl(new UT_Stringbuf(*rhs.pimpl))
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
	pimpl->assign(rhs, strlen(rhs));
	return *this;
}

UT_String& UT_String::operator+=(const UT_String& rhs)
{
	if (this != &rhs) {
		pimpl->append(*rhs.pimpl);
	} else {
		UT_Stringbuf t(*rhs.pimpl);
		pimpl->append(t);
	}
	return *this;
}

UT_String& UT_String::operator+=(const char* rhs)
{
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
	UT_Stringbuf* p = pimpl;
	pimpl = rhs.pimpl;
	rhs.pimpl = p;
}

//////////////////////////////////////////////////////////////////
// End of class members, start of free functions
//////////////////////////////////////////////////////////////////

static UT_uint32
UT_printf_string_upper_bound (const char* format,
			      va_list      args)
{
  UT_uint32 len = 1;

  while (*format)
    {
      bool long_int = false;
      bool extra_long = false;
      char c;

      c = *format++;

      if (c == '%')
	{
	  bool done = false;

	  while (*format && !done)
	    {
	      switch (*format++)
		{
		  char *string_arg;

		case '*':
		  len += va_arg (args, int);
		  break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		  /* add specified format length, since it might exceed the
		   * size we assume it to have.
		   */
		  format -= 1;
		  len += strtol (format, (char**) &format, 10);
		  break;
		case 'h':
		  /* ignore short int flag, since all args have at least the
		   * same size as an int
		   */
		  break;
		case 'l':
		  if (long_int)
		    extra_long = true; /* linux specific */
		  else
		    long_int = true;
		  break;
		case 'q':
		case 'L':
		  long_int = true;
		  extra_long = true;
		  break;
		case 's':
		  string_arg = va_arg (args, char *);
		  if (string_arg)
		    len += strlen (string_arg);
		  else
		    {
		      /* add enough padding to hold "(null)" identifier */
		      len += 16;
		    }
		  done = true;
		  break;
		case 'd':
		case 'i':
		case 'o':
		case 'u':
		case 'x':
		case 'X':
		    {
		      if (long_int)
			(void) va_arg (args, long);
		      else
			(void) va_arg (args, int);
		    }
		  len += extra_long ? 64 : 32;
		  done = true;
		  break;
		case 'D':
		case 'O':
		case 'U':
		  (void) va_arg (args, long);
		  len += 32;
		  done = true;
		  break;
		case 'e':
		case 'E':
		case 'f':
		case 'g':
		    (void) va_arg (args, double);
		  len += extra_long ? 64 : 32;
		  done = true;
		  break;
		case 'c':
		  (void) va_arg (args, int);
		  len += 1;
		  done = true;
		  break;
		case 'p':
		case 'n':
		  (void) va_arg (args, void*);
		  len += 32;
		  done = true;
		  break;
		case '%':
		  len += 1;
		  done = true;
		  break;
		default:
		  /* ignore unknow/invalid flags */
		  break;
		}
	    }
	}
      else
	len += 1;
    }

  return len;
}

#if !defined (VA_COPY)
#  if defined (__GNUC__) && defined (__PPC__) && (defined (_CALL_SYSV) || defined (_WIN32) || defined(WIN32)) || defined(__s390__)
#  define VA_COPY(ap1, ap2)	  (*(ap1) = *(ap2))
#  elif defined (VA_COPY_AS_ARRAY)
#  define VA_COPY(ap1, ap2)	  memmove ((ap1), (ap2), sizeof (va_list))
#  else /* va_list is a pointer */
#  define VA_COPY(ap1, ap2)	  ((ap1) = (ap2))
#  endif /* va_list is a pointer */
#endif /* !VA_COPY */

UT_String& UT_String_vprintf (UT_String & inStr, const char *format,
			      va_list      args1)
{
  char *buffer;
  va_list args2;

  VA_COPY (args2, args1);

  buffer = new char [ UT_printf_string_upper_bound (format, args1) ];
  vsprintf (buffer, format, args2);
  va_end (args2);

  inStr = buffer;

  delete [] buffer;

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

//////////////////////////////////////////////////////////////////
// Helpers

bool operator==(const UT_String& s1, const UT_String& s2)
{
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
	const char* p = string.c_str();
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

/**************************************************************************/
/*************************************************************************/

//static const UT_UCSChar ucsEmpty[] = "";
static const UT_UCSChar *ucsEmpty = 0;

UT_UCS2String::UT_UCS2String()
:	pimpl(new UT_UCS2Stringbuf)
{
}

UT_UCS2String::UT_UCS2String(const UT_UCSChar* sz, size_t n)
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

	const UT_UCSChar* p = pimpl->data() + iStart;
	if (iStart + nChars > nSize) {
		nChars = nSize - iStart;
	}

	return UT_UCS2String(p, nChars);
}

const UT_UCSChar* UT_UCS2String::ucs_str() const
{
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

UT_UCS2String& UT_UCS2String::operator=(const UT_UCSChar* rhs)
{
	pimpl->assign(rhs, UT_UCS_strlen(rhs));
	return *this;
}

UT_UCS2String& UT_UCS2String::operator+=(const UT_UCS2String& rhs)
{
	if (this != &rhs) {
		pimpl->append(*rhs.pimpl);
	} else {
		UT_UCS2Stringbuf t(*rhs.pimpl);
		pimpl->append(t);
	}
	return *this;
}

UT_UCS2String& UT_UCS2String::operator+=(const UT_UCSChar* rhs)
{
	pimpl->append(rhs, UT_UCS_strlen(rhs));
	return *this;
}

UT_UCS2String& UT_UCS2String::operator+=(UT_UCSChar rhs)
{
	UT_UCSChar cs = rhs;
	pimpl->append(&cs, 1);
	return *this;
}

UT_UCS2String& UT_UCS2String::operator+=(char rhs)
{
	UT_UCSChar cs[2];
	char rs[2];

	// TODO: is this nonsense needed?
	rs[0] = rhs; rs[1] = 0;
	UT_UCS_strcpy_char (cs, rs);

	pimpl->append(cs, 1);
	return *this;
}

UT_UCS2String& UT_UCS2String::operator+=(unsigned char rhs)
{
	UT_UCSChar cs[2];
	char rs[2];

	// TODO: is this nonsense needed?
	rs[0] = (char)rhs; rs[1] = 0; // TODO: is this loss of 'unsigned' safe?
	UT_UCS_strcpy_char (cs, rs);

	pimpl->append(cs, 1);
	return *this;
}

void UT_UCS2String::swap(UT_UCS2String& rhs)
{
	UT_UCS2Stringbuf* p = pimpl;
	pimpl = rhs.pimpl;
	rhs.pimpl = p;
}


//////////////////////////////////////////////////////////////////
// End of class members, start of free functions
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// Helpers

bool operator==(const UT_UCS2String& s1, const UT_UCS2String& s2)
{
	return UT_UCS_strcmp(s1.ucs_str(), s2.ucs_str()) == 0;
}

bool operator==(const UT_UCS2String& s1, const UT_UCSChar* s2)
{
	return UT_UCS_strcmp(s1.ucs_str(), s2) == 0;
}

bool operator==(const UT_UCSChar* s1, const UT_UCS2String& s2)
{
	return s2 == s1;
}

bool operator!=(const UT_UCS2String& s1, const UT_UCS2String& s2)
{
	return !(s1 == s2);
}

bool operator!=(const UT_UCS2String& s1, const UT_UCSChar*  s2)
{
	return !(s1 == s2);
}

bool operator!=(const UT_UCSChar* s1, const UT_UCS2String& s2)
{
	return !(s2 == s1);
}

bool operator<(const UT_UCS2String& s1, const UT_UCS2String& s2)
{
	return UT_UCS_strcmp(s1.ucs_str(), s2.ucs_str()) < 0;
}

UT_UCS2String operator+(const UT_UCS2String& s1, const UT_UCS2String& s2)
{
	UT_UCS2String s(s1);
	s += s2;
	return s;
}

UT_UCSChar UT_UCS2String::operator[](size_t iPos) const
{
	UT_ASSERT(iPos <= size());
	if (iPos == size())
		return '\0';
	return pimpl->data()[iPos];
}

UT_UCSChar& UT_UCS2String::operator[](size_t iPos)
{
	UT_ASSERT(iPos <= size());
	return pimpl->data()[iPos];
}

// ---------------------------------------------------------------------------------- //

UT_UTF8String::UT_UTF8String () :
	pimpl(new UT_UTF8Stringbuf)
{
	// 
}

UT_UTF8String::UT_UTF8String (const char * sz) :
	pimpl(new UT_UTF8Stringbuf(sz))
{
	// 
}

UT_UTF8String::UT_UTF8String (const UT_UTF8String & rhs) :
	pimpl(new UT_UTF8Stringbuf(*rhs.pimpl))
{
	// 
}

UT_UTF8String::UT_UTF8String (const UT_UCS2String & rhs) :
	pimpl(new UT_UTF8Stringbuf)
{
	if (rhs.size ()) append (rhs.ucs_str (), rhs.size ());
}

UT_UTF8String::UT_UTF8String (const UT_UCSChar * sz, size_t n) :
	pimpl(new UT_UTF8Stringbuf)
{
	append (sz, n);
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

bool UT_UTF8String::empty () const
{
	return pimpl->empty ();
}

void UT_UTF8String::clear () const
{
	pimpl->clear ();
}

UT_UTF8String &	UT_UTF8String::operator=(const char * rhs)
{
	pimpl->assign (rhs);
	return *this;
}

UT_UTF8String &	UT_UTF8String::operator=(const UT_UTF8String & rhs)
{
	if (this != &rhs) {
		*pimpl = *rhs.pimpl;
	}
	return *this;
}

UT_UTF8String &	UT_UTF8String::operator=(const UT_UCS2String & rhs)
{
	pimpl->clear ();
	if (rhs.size ()) append (rhs.ucs_str (), rhs.size ());
	return *this;
}

UT_UTF8String &	UT_UTF8String::operator+=(const char * rhs)
{
	pimpl->append (rhs);
	return *this;
}

UT_UTF8String &	UT_UTF8String::operator+=(const UT_UTF8String & rhs)
{
	pimpl->append (*rhs.pimpl);
	return *this;
}

UT_UTF8String &	UT_UTF8String::operator+=(const UT_UCS2String & rhs)
{
	if (rhs.size ()) append (rhs.ucs_str (), rhs.size ());
	return *this;
}

const char * UT_UTF8String::utf8_str () const
{
	static const char * utf8Empty = "";
	return pimpl->utf8Length () ? pimpl->data() : utf8Empty;
}

void UT_UTF8String::append (const UT_UCSChar * sz, size_t n /* 0 = null-terminated */)
{
	if (sz == 0) return;

	size_t ucs2_length = 0;
	size_t utf8_length = 0;

	// check UCS-2 string length and calculate UTF-8 string length
	if (n == 0)
	{
		const UT_UCSChar * p = sz;
		while (*p)
		{
			if ((*p & 0xff80) == 0x0000)
			{
				utf8_length++;
			}
			else if ((*p & 0xf800) == 0x0000)
			{
				utf8_length += 2;
			}
			else
			{
				utf8_length += 3;
			}
			p++;
			ucs2_length++;
		}
	}
	else
	{
		const UT_UCSChar * p = sz;
		for (size_t i = 0; i < n; i++)
		{
			if (*p == 0) break;
			if ((*p & 0xff80) == 0x0000)
			{
				utf8_length++;
			}
			else if ((*p & 0xf800) == 0x0000)
			{
				utf8_length += 2;
			}
			else
			{
				utf8_length += 3;
			}
			p++;
			ucs2_length++;
		}
	}

	if (utf8_length == 0) return;

	char * utf8_buffer = (char *) malloc (utf8_length + 1);
	if (utf8_buffer == 0) return;

	const UT_UCSChar * p = sz;
	char * b = utf8_buffer;
	for (size_t i = 0; i < utf8_length; i++)
	{
		if ((*p & 0xff80) == 0x0000)
		{
			*b++ = (char) (*p & 0x7f);
		}
		else if ((*p & 0xf800) == 0x0000)
		{
			*b++ = 0xc0 | (char) ((*p >> 6) & 0x1f);
			*b++ = 0x80 | (char) ( *p       & 0x3f);
		}
		else
		{
			*b++ = 0xe0 | (char) ((*p >> 12) & 0x0f);
			*b++ = 0x80 | (char) ((*p >>  6) & 0x3f);
			*b++ = 0x80 | (char) ( *p        & 0x3f);
		}
		p++;
	}
	*b = 0;

	pimpl->append (utf8_buffer);

	free (utf8_buffer);
}

UT_UCS2String UT_UTF8String::ucs2_str ()
{
	UT_UCS2String ucs2_string;

	UT_UTF8Stringbuf::UTF8Iterator utf(pimpl);

	utf = utf.start ();
	if (utf.current () == 0) return ucs2_string;

	while (true)
	{
		const char * pUTF8 = utf.current ();
		utf.advance ();

		UT_UTF8Stringbuf::UCS4Char ucs4 = UT_UTF8Stringbuf::charCode (pUTF8);

		if (ucs4 == 0) break; // end of string
		if (ucs4 & 0xffff0000) continue; // oops - UCS-2 can't handle this...

		ucs2_string += (UT_UCSChar) (ucs4 & 0x0000ffff);
	}

	return ucs2_string;
}

UT_UTF8String operator+(const UT_UTF8String & s1, const UT_UTF8String & s2)
{
	UT_UTF8String s(s1);
	s += s2;
	return s;
}
