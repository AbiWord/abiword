/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_iconv.h"
#include "iconv.h"

/*****************************************************************/

#if 1
UT_iconv_t  UT_iconv_open( const char* to, const char* from )
{
    return iconv_open( to, from );
}

size_t  UT_iconv( UT_iconv_t cd, const char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft )
{
    return iconv( (iconv_t)cd, inbuf, inbytesleft, outbuf, outbytesleft );
}

int  UT_iconv_close( UT_iconv_t cd )
{
    return iconv_close( (iconv_t)cd );
}
#else
static struct
{
    int cp;
    char *tag;
} s_LookUp[] = {
	{ CP_UTF8, "UTF8" },			// Unicode UTF8
	{ CP_UTF8, "UTF-8" },			// Unicode UTF8
	{ 1200, "UCS-2" },				// Unicode (BMP of ISO 10646)		
	{ 1200, "UCS-2-INTERNAL" }, 	// Unicode (BMP of ISO 10646)		
//	{ 437, "MS-DOS" },
//	{ 708, "Arabic (ASMO 708)" },
//	{ 709, "Arabic (ASMO 449+, BCON V4)" },
//	{ 710, "Arabic (Transparent Arabic)" },
//	{ 720, "Arabic (Transparent ASMO)" },
//	{ 737, "Greek (formerly 437G)" },
//	{ 775, "Baltic" },
//	{ 850, "MS-DOS Multilingual (Latin I)" },
//	{ 852, "MS-DOS Slavic (Latin II)" },
//	{ 855, "IBM Cyrillic (primarily Russian)" },
//	{ 857, "IBM Turkish" },
//	{ 860, "MS-DOS Portuguese" },
//	{ 861, "MS-DOS Icelandic" },
//	{ 862, "Hebrew" },
//	{ 863, "MS-DOS Canadian-French" },
//	{ 864, "Arabic" },
//	{ 865, "MS-DOS Nordic" },
//	{ 866, "MS-DOS Russian" },
//	{ 869, "IBM Modern Greek" },
//	{ 874, "Thai" },				// Thai
//	{ 932, "Japan" },				// Japan
	{ 936, "GBK" }					// Chinese (PRC, Singapore)
	{ 949, "KOREAN" },				// Korean
//	{ 950, "Chinese" }, 			// Chinese (Taiwan Region; Hong Kong SAR, PRC)
	{ 1250, "MS-EE" },				// Windows 3.1 Eastern European
	{ 1251, "MS-CYRL" },			// Windows 3.1 Cyrillic
	{ 1252, "MS-ANSI" },			// Windows 3.1 US (ANSI)
	{ 1253, "MS-GREEK" },			// Windows 3.1 Greek
	{ 1254, "MS-TURK" },			// Windows 3.1 Turkish
	{ 1255, "MS-HEBR" },			// Hebrew
	{ 1256, "MS-ARAB" },			// Arabic
	{ 1257, "WINBALTRIM" }, 		// Baltic
	{ 1361, "JOHAB" },				// Korean (Johab)
	{ 10000, "MacRoman" },			// Macintosh Roman
	{ 10001, "MacJapanese" },		// Macintosh Japanese
	{ 10006, "MacGreek" },			// Macintosh Greek I
	{ 10007, "MacCyrillic" },		// Macintosh Cyrillic
	{ 10029, "MacCentralEurope" },	// Macintosh Latin 2
	{ 10079, "MacIceland" },		// Macintosh Icelandic
	{ 10081, "MacTurkish" },		// Macintosh Turkish
	{ CP_UTF7, "UTF7" },			// Unicode UTF7
	{ CP_UTF7, "UTF-7" },			// Unicode UTF7
	{ 0, NULL } };
#endif

 

 
