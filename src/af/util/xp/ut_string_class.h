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
#include "ut_string.h"

// yes, this is screaming for a template

//!
//	UT_String, a simple wrapper for zero terminated 'char' strings.
//
class UT_String
{
public:
	UT_String();
	UT_String(const char* sz, size_t n = 0 /* 0 == zero-terminate */);
	UT_String(const UT_String& rhs);
	~UT_String();

	size_t		size() const;
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
bool operator==(const UT_String& s1, const UT_String& s2);
bool operator==(const UT_String& s1, const char*      s2);
bool operator==(const char*      s1, const UT_String& s2);
bool operator!=(const UT_String& s1, const UT_String& s2);
bool operator!=(const UT_String& s1, const char*      s2);
bool operator!=(const char*      s1, const UT_String& s2);

UT_uint32 hashcode(const UT_String& string);

// strcmp ordering
bool operator<(const UT_String& s1, const UT_String& s2);

UT_String operator+(const UT_String& s1, const UT_String& s2);

/***************************************************************************/

//!
//	UT_UCS2String, a simple wrapper for zero terminated 'UCS2' strings.
//

// TODO: add c_str(), utf8_str(), encoded_str(const char * to)

class UT_UCS2String
{
public:
	UT_UCS2String();
	UT_UCS2String(const UT_UCSChar * sz, size_t n = 0 /* 0 == zero-terminate */);
	UT_UCS2String(const UT_UCS2String& rhs);
	~UT_UCS2String();

	size_t		size() const;
	bool		empty() const;
	void        clear() const;

	UT_UCS2String	substr(size_t iStart, size_t nChars) const;

	UT_UCS2String&	operator=(const UT_UCS2String&  rhs);
	UT_UCS2String&	operator=(const UT_UCSChar *    rhs);
	UT_UCS2String&	operator+=(const UT_UCS2String& rhs);
	UT_UCS2String&	operator+=(const UT_UCSChar *   rhs);
	UT_UCS2String&  operator+=(UT_UCSChar rhs);
	UT_UCS2String&  operator+=(char rhs);
	UT_UCS2String&  operator+=(unsigned char rhs);

	UT_UCSChar		operator[](size_t iPos) const;
	UT_UCSChar&		operator[](size_t iPos);

	void		swap(UT_UCS2String& rhs);

	// The returned pointer is valid until the next non-const
	// operation. You will _always_ get a legal pointer back,
	// even if to an empty (0) string.
	const UT_UCSChar* ucs_str() const;

private:
	class UT_UCS2Stringbuf* pimpl;
};

// helpers
bool operator==(const UT_UCS2String& s1, const UT_UCS2String& s2);
bool operator==(const UT_UCS2String& s1, const UT_UCSChar *   s2);
bool operator==(const UT_UCSChar *   s1, const UT_UCS2String& s2);
bool operator!=(const UT_UCS2String& s1, const UT_UCS2String& s2);
bool operator!=(const UT_UCS2String& s1, const UT_UCSChar *   s2);
bool operator!=(const UT_UCSChar *   s1, const UT_UCS2String& s2);

// strcmp ordering
bool operator<(const UT_UCS2String& s1, const UT_UCS2String& s2);

UT_UCS2String operator+(const UT_UCS2String& s1, const UT_UCS2String& s2);

#endif	// UT_STRING_CLASS_H

