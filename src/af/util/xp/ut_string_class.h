// ut_string_class.h
//
// A simple string class for use where templates are not
// allowed.
//
#ifndef UT_STRING_CLASS_H
#define UT_STRING_CLASS_H

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

#include <stdlib.h>
#include "ut_vector.h"

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

	UT_String	substr(size_t iStart, size_t nChars) const;

	UT_String&	operator=(const UT_String& rhs);
	UT_String&	operator=(const char*      rhs);
	UT_String&	operator+=(const UT_String& rhs);
	UT_String&	operator+=(const char*      rhs);

	void		swap(UT_String& rhs);
	UT_Vector	*simplesplit(char separator = ' ', 
				     size_t max = 0 /* 0 == full split */);

	// The returned pointer is valid until the next non-const
	// operation. You will _always_ get a legal pointer back,
	// even if to an empty string.
	const char* c_str() const;

	// Convenience operator. Should possibly be removed
	operator const char*() const { return c_str(); }

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

// strcmp ordering
bool operator<(const UT_String& s1, const UT_String& s2);

UT_String operator+(const UT_String& s1, const UT_String& s2);

#endif	// UT_STRING_CLASS_H

