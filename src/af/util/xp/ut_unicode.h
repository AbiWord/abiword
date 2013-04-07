/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

// UT_Unicode.h

// Copyright (C) 2001 Mike Nordell <tamlin@algonet.se>
// Copyright (c) 2007 Hubert Figuiere <hub@figuiere.net>
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


#ifndef __UT_UNICODE_H_
#define __UT_UNICODE_H_

#ifndef UT_TYPES_H
#include "ut_types.h"
#endif


/** contain various unicode function. Just a namespace */
class ABI_EXPORT UT_Unicode
{
public:
/** scans a buffer for the next valid UTF-8 sequence and returns the corresponding
 * UCS-4 value for that sequence; the pointer and length-remaining are incremented
 * and decremented respectively; returns 0 if no valid UTF-8 sequence found by the
 * end of the string
 */
	static UT_UCS4Char UTF8_to_UCS4 (const char *& buffer, size_t & length);


/** Returns -1 if ucs4 is not valid UCS-4, 0 if ucs4 is 0, 1-6 otherwise
 */
	static int UTF8_ByteLength (UT_UCS4Char ucs4);

/** appends to the buffer the UTF-8 sequence corresponding to the UCS-4 value;
 * the pointer and length-remaining are incremented and decremented respectively;
 * returns false if not valid UCS-4 or if (length < UTF8_ByteLength (ucs4))
 */
	static bool UCS4_to_UTF8 (char *& buffer, size_t & length, UT_UCS4Char ucs4);
};


#endif
