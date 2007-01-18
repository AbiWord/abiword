/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

// UT_Unicode.cpp

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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
// 02111-1307, USA.
//


#include "ut_assert.h"
#include "ut_unicode.h"


/* Returns -1 if ucs4 is not valid UCS-4, 0 if ucs4 is 0, 1-6 otherwise
 */
int UT_Unicode::UTF8_ByteLength (UT_UCS4Char u)
{
	if ((u & 0x7fffffff) != u) return -1; // UCS-4 is only 31-bit!

	if (u == 0) return 0; // end-of-string

	if ((u & 0x7fffff80) == 0) return 1;
	if ((u & 0x7ffff800) == 0) return 2;
	if ((u & 0x7fff0000) == 0) return 3;
	if ((u & 0x7fe00000) == 0) return 4;
	if ((u & 0x7c000000) == 0) return 5;
	return 6;
}

/* appends to the buffer the UTF-8 sequence corresponding to the UCS-4 value;
 * the pointer and length-remaining are incremented and decremented respectively;
 * returns false if not valid UCS-4 or if (length < UTF8_ByteLength (ucs4))
 */
bool UT_Unicode::UCS4_to_UTF8 (char *& buffer, size_t & length, UT_UCS4Char ucs4)
{
	int seql = UTF8_ByteLength (ucs4);
	if (seql < 0) return false;
	if (seql == 0) {
		if (length == 0) return false;
		*buffer++ = 0;
		length--;
		return true;
	}
	if (length < static_cast<unsigned>(seql)) return false;
	length -= seql;

	switch (seql) {
	case 1:
		*buffer++ = static_cast<char>(static_cast<unsigned char>(ucs4 & 0x7f));
		break;
	case 2:
		*buffer++ = static_cast<char>(0xc0 | static_cast<unsigned char>((ucs4 >> 6) & 0x1f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>(ucs4 & 0x3f));
		break;
	case 3:
		*buffer++ = static_cast<char>(0xe0 | static_cast<unsigned char>((ucs4 >> 12) & 0x0f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 6) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>(ucs4 & 0x3f));
		break;
	case 4:
		*buffer++ = static_cast<char>(0xf0 | static_cast<unsigned char>((ucs4 >> 18) & 0x07));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 12) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 6) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>(ucs4 & 0x3f));
		break;
	case 5:
		*buffer++ = static_cast<char>(0xf8 | static_cast<unsigned char>((ucs4 >> 24) & 0x03));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 18) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 12) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 6) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>(ucs4 & 0x3f));
		break;
	case 6:
		*buffer++ = static_cast<char>(0xfc | static_cast<unsigned char>((ucs4 >> 30) & 0x01));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 24) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 18) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 12) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>((ucs4 >> 6) & 0x3f));
		*buffer++ = static_cast<char>(0x80 | static_cast<unsigned char>(ucs4 & 0x3f));
		break;
	default: // huh?
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}
	return true;
}

/** scans a buffer for the next valid UTF-8 sequence and returns the corresponding
 * UCS-4 value for that sequence; the pointer and length-remaining are incremented
 * and decremented respectively; returns 0 if no valid UTF-8 sequence found by the
 * end of the string
 */
UT_UCS4Char UT_Unicode::UTF8_to_UCS4 (const char *& buffer, size_t & length)
{
	UT_UCS4Char ucs4;
	
	while (true) {
		ucs4 = 0;
		if (length == 0) break;
		
		unsigned char c = static_cast<unsigned char>(*buffer);
		buffer++;
		length--;
		
		if ((c & 0x80) == 0) { // ascii, single-byte sequence
			ucs4 = static_cast<UT_UCS4Char>(c);
			break;
		}
		if ((c & 0xc0) == 0x80) { // hmm, continuing byte - let's just ignore it
			continue;
		}

		/* we have a multi-byte sequence...
		 */
		size_t seql;

		if ((c & 0xe0) == 0xc0) {
			seql = 2;
			ucs4 = static_cast<UT_UCS4Char>(c & 0x1f);
		}
		else if ((c & 0xf0) == 0xe0) {
			seql = 3;
			ucs4 = static_cast<UT_UCS4Char>(c & 0x0f);
		}
		else if ((c & 0xf8) == 0xf0) {
			seql = 4;
			ucs4 = static_cast<UT_UCS4Char>(c & 0x07);
		}
		else if ((c & 0xfc) == 0xf8) {
			seql = 5;
			ucs4 = static_cast<UT_UCS4Char>(c & 0x03);
		}
		else if ((c & 0xfe) == 0xfc) {
			seql = 6;
			ucs4 = static_cast<UT_UCS4Char>(c & 0x01);
		}
		else { // or perhaps we don't :-( - whatever it is, let's just ignore it
			continue;
		}

		if (length < seql - 1) { // huh? broken sequence perhaps? anyway, let's just ignore it
			continue;
		}

		bool okay = true;
		for (size_t i = 1; i < seql; i++) {
			c = static_cast<unsigned char>(*buffer);
			buffer++;
			length--;
			if ((c & 0xc0) != 0x80) { // not a continuing byte? grr!
				okay = false;
				break;
			}
			ucs4 = ucs4 << 6 | static_cast<UT_UCS4Char>(c & 0x3f);
		}
		if (okay)
			break;
	}
	return ucs4;
}

