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
	UT_UCSChar cs;
	char rs[2];

	// TODO: is this nonsense needed?
	rs[0] = rhs; rs[1] = 0;
	UT_UCS_strcpy_char (&cs, rs);

	pimpl->append(&cs, 1);
	return *this;
}

UT_UCS2String& UT_UCS2String::operator+=(unsigned char rhs)
{
	UT_UCSChar cs;
	char rs[2];

	// TODO: is this nonsense needed?
	rs[0] = (char)rhs; rs[1] = 0; // TODO: is this loss of 'unsigned' safe?
	UT_UCS_strcpy_char (&cs, rs);

	pimpl->append(&cs, 1);
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
