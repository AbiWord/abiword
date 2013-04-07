/* Abiword
 * Copyright (C) 2001 Christian Biesinger <cbiesinger@web.de>
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
/* The above copyright notice of course only applies to the new code
 * that was written by me, not the part from the OpenOffice code (see below) */

// Taken from the file sal/inc/rtl/textenc.h lines 75ff
// Interestingly, glib's list of encodings differs from the one of the
// seperately available library... thus, some numbers have multiple names
// in this list
// And some are not present in glib's iconv, as of 2.3.1
// The libiconv list is up-to-date as of Oct. 27, 2002, per
// the charset list at http://www.gnu.org/software/libiconv/
//
// libiconv extra means that libiconv must've been configured with --enable-extra-encodings

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_iconv.h"

#include "ie_imp_StarOffice_encodings.h"

struct SOEncoding {
	UT_uint16 number;
	const char* name;
} static const SOEncodings[] = {
	{ 0, "ISO-8859-1" }, // use latin1 as default charset; but 0 actually means unknown
	{ 1, "WINDOWS-1252" },
	{ 2, "MAC" },
	{ 3, "CP437" }, // glibc / libiconv extra
	{ 4, "CP850" },
	{ 5, "CP860" }, // glibc / libiconv extra
	{ 6, "CP861" }, // glibc / libiconv extra
	{ 7, "CP863" }, // glibc / libiconv extra
	{ 8, "CP865" }, // glibc / libiconv extra
	/* RESERVED 9, System encoding */
	/* 10: Symbol encoding (?) */
	{ 11, "US-ASCII" },
	{ 12, "ISO-8859-1" },
	{ 13, "ISO-8859-2" },
	{ 14, "ISO-8859-3" },
	{ 15, "ISO-8859-4" },
	{ 16, "ISO-8859-5" },
	{ 17, "ISO-8859-6" },
	{ 18, "ISO-8859-7" },
	{ 19, "ISO-8859-8" },
	{ 20, "ISO-8859-9" },
	{ 21, "ISO-8859-14" },
	{ 22, "ISO-8859-15" },
	{ 23, "CP737" },  // glibc / libiconv extra
	{ 24, "CP775" }, // glibc / libiconv extra
	{ 25, "CP852" }, // glibc / libiconv extra
	{ 26, "CP855" }, // glibc / libiconv extra
	{ 27, "CP857" }, // glibc / libiconv extra
	{ 28, "CP862" },
	{ 29, "CP864" }, // glibc / libiconv extra
	{ 30, "CP866" },
	{ 31, "CP869" }, // glibc / libiconv extra
	{ 32, "CP874" }, // XXX Correct? Should be MS 874. aka WINDOWS-874
	{ 33, "WINDOWS-1250" },
	{ 34, "WINDOWS-1251" },
	{ 35, "WINDOWS-1253" },
	{ 36, "WINDOWS-1254" },
	{ 37, "WINDOWS-1255" },
	{ 38, "WINDOWS-1256" },
	{ 39, "WINDOWS-1257" },
	{ 40, "WINDOWS-1258" },
	{ 41, "MacArabic" },        // libiconv
	{ 42, "MacCentralEurope" }, // libiconv
	{ 43, "MacCroatian" },      // libiconv
	{ 44, "MAC-CYRILLIC" },     // glibc
	{ 44, "MacCyrillic" },      // libiconv
	/* 45: Apple Devanagari
	 * 46: Apple Farsi */
	{ 47, "MacGreek" }, // libiconv
	/* 48: Apple Gujarati
	 * 49: Apple Gurmukhi */
	{ 50, "MacHebrew" }, // libiconv
	{ 51, "MAC-IS" }, // XXX Correct? Should be Apple Iceland. glibc.
	{ 51, "MacIceland" }, // libiconv
	{ 52, "MacRomania" }, // libiconv
	{ 53, "MacThai" },    // libiconv
	{ 54, "MacTurkish" }, // libiconv
	{ 55, "MACUKRAINIAN" }, // glibc
	{ 55, "MacUkraine" }, // libiconv
	/* 56: Apple Chinese Simplified
	 * 57: Apple Chinese Traditional
	 * 58: Apple Japanese
	 * 59: Apple Korean */
	{ 60, "CP932" },
	{ 61, "CP936" },
	{ 62, "CP949" },
	{ 63, "CP950" },
	{ 64, "SHIFT-JIS" },
	{ 65, "GB2312" },
	/* 66: GBT 12345 */
	{ 67, "GBK" },
	{ 68, "BIG5" },
	{ 69, "EUC-JP" },
	{ 70, "EUC-CN" },
	{ 71, "EUC-TW" },
	{ 72, "ISO-2022-JP" },
	{ 73, "ISO-2022-CN" },
	{ 74, "KOI8-R" },
	{ 75, "UTF-7" },
	{ 76, "UTF-8" },
	{ 77, "ISO-8859-10" },
	{ 78, "ISO-8859-13" },
	{ 79, "EUC-KR" },
	{ 80, "ISO-2022-KR" },
	{ 81, "JIS_X0201" }, // libiconv. interestingly, this one and the next two are not mentioned on the libiconv homepage. they seem to be supported anyway.
	{ 82, "JIS_X0208" }, // libiconv
	{ 83, "JIS_X0212" }, // libiconv
	{ 84, "CP1361" },
	{ 85, "GB18030" },
	{ 0xFFFE, "UCS-4" },
	{ 0xFFFF, "UCS-2" }
};


UT_iconv_t findConverter(UT_uint8 id) {
	UT_iconv_t converter = (UT_iconv_t)(-1);
	for (unsigned int i = 0; i < sizeof(SOEncodings)/sizeof(SOEncodings[0]); i++) {
		if (SOEncodings[i].number == id) {
			UT_DEBUGMSG(("SDW: Found charset %s for encoding #%i\n", SOEncodings[i].name, id));
			converter = UT_iconv_open(UCS_INTERNAL, SOEncodings[i].name);
			if (UT_iconv_isValid(converter))
				break;
		}
	}
	return converter;
}


