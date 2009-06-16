/* AbiSource Program Utilities
 * Copyright (C) 2001 AbiSource, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include "ut_iconv.h"
#include "ut_Encoding.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Strings.h"

// Please keep the list below alphabetised by the encoding; even though 
// this is not required for it to work, it will make it easier to maintain.

// to add a new encoding:
// (1) check on this list it is not already there;
// (2) add it to the list in xap_String_Id.h 
// (3) add it here, using the ID corresponding to the one
//     from xap_String_Id.h

// This list is based on libiconv.
// Since iconv implementations differ and iconv has no enumeration API,
// this code attempts to enumerate the supported encodings.
//
// There is an array of possible names for each encoding
// in order of standardization or popularity.  We attempt to
// open each before deciding the encoding is not supported.
//
// Another approach is to do these tests in an external program which
// outputs the C++ code for the following table.
//
// TODO Note that certain operations in AbiWord currently try to open or
// TODO compare certain encodings via hard-coded names.  This should be
// TODO discouraged and replaced with names derived as in these tables.
//
// TODO This code should probably move into the Encoding Manager.
//
// TODO Some platforms use specific names not covered by this alias table
// TODO such as the solaris iso-8559-n problem just fixed.  Root out any
// TODO more instances of this to easily avoid problems that can cause a
// TODO huge PITA when misdiagnosed.

static const gchar * enc_armscii[]	= {"ARMSCII-8",0};
static const gchar * enc_big5[]	= {"BIG5","BIG-5","BIG-FIVE","BIGFIVE","CN-BIG5",0};
static const gchar * enc_big5hkscs[]	= {"BIG5-HKSCS","BIG5HKSCS",0};
#ifdef WIN32 /* DOS/Win32 console encodings, peer iconv supports, others may not */
static const gchar * enc_cp437[]	= {"C437","IBM437","437",0};
static const gchar * enc_cp850[]	= {"C850","IBM850","850",0};
#endif
static const gchar * enc_cp874[]	= {"CP874",0};
static const gchar * enc_cp932[]	= {"CP932",0};
static const gchar * enc_cp936[]	= {"CP936","GBK",0};
static const gchar * enc_cp949[]	= {"CP949","UHC",0};
static const gchar * enc_cp950[]	= {"CP950",0};
static const gchar * enc_cp1250[]	= {"CP1250","WINDOWS-1250","MS-EE",0};
static const gchar * enc_cp1251[]	= {"CP1251","WINDOWS-1251","MS-CYRL",0};
static const gchar * enc_cp1252[]	= {"CP1252","WINDOWS-1252","MS-ANSI",0};
static const gchar * enc_cp1253[]	= {"CP1253","WINDOWS-1253","MS-GREEK",0};
static const gchar * enc_cp1254[]	= {"CP1254","WINDOWS-1254","MS-TURK",0};
static const gchar * enc_cp1255[]	= {"CP1255","WINDOWS-1255","MS-HEBR",0};
static const gchar * enc_cp1256[]	= {"CP1256","WINDOWS-1256","MS-ARAB",0};
static const gchar * enc_cp1257[]	= {"CP1257","WINDOWS-1257","WINBALTRIM",0};
static const gchar * enc_cp1258[]	= {"CP1258","WINDOWS-1258",0};
static const gchar * enc_euc_cn[]	= {"EUC-CN","EUCCN","GB2312","CN-GB",0};	// Cf. GB_2312-80
static const gchar * enc_euc_jp[]	= {"EUC-JP","EUCJP",0};
static const gchar * enc_euc_kr[]	= {"EUC-KR","EUCKR",0};
static const gchar * enc_euc_tw[]	= {"EUC-TW","EUCTW",0};
static const gchar * enc_gb2312[]	= {"GB_2312-80","ISO-IR-58","CHINESE",0};	// Cf. EUC-CN
static const gchar * enc_georga[]	= {"GEORGIAN-ACADEMY",0};
static const gchar * enc_georgps[]	= {"GEORGIAN-PS",0};
static const gchar * enc_hp[]		= {"HP-ROMAN8","ROMAN8","R8",0};
static const gchar * enc_hz[]		= {"HZ","HZ-GB-2312",0};
static const gchar * enc_8859_1[]	= {"ISO-8859-1","ISO_8859-1","8859-1","LATIN1","L1",0};
static const gchar * enc_8859_2[]	= {"ISO-8859-2","ISO_8859-2","8859-2","LATIN2","L2",0};
//static const gchar * enc_8859_3[]  = {"ISO-8859-3","ISO_8859-3","8859-3","LATIN3","L3",0};
static const gchar * enc_8859_4[]	= {"ISO-8859-4","ISO_8859-4","8859-4","LATIN4","L4",0};
static const gchar * enc_8859_5[]	= {"ISO-8859-5","ISO_8859-5","8859-5","CYRILLIC",0};
static const gchar * enc_8859_6[]	= {"ISO-8859-6","ISO_8859-6","8859-6","ECMA-114","ASMO-708","ARABIC",0};
static const gchar * enc_8859_7[]	= {"ISO-8859-7","ISO_8859-7","8859-7","ECMA-118","ELOT_928","GREEK8","GREEK",0};
static const gchar * enc_8859_8[]	= {"ISO-8859-8","ISO_8859-8","8859-8","HEBREW",0};
static const gchar * enc_8859_9[]	= {"ISO-8859-9","ISO_8859-9","8859-9","LATIN5","L5",0};
//static const gchar * enc_8859_10[]	= {"ISO-8859-10","ISO_8859-10","8859-10","LATIN6","L6",0};
//static const gchar * enc_8859_13[]	= {"ISO-8859-13","ISO_8859-13","8859-13","LATIN7","L7",0};
//static const gchar * enc_8859_14[]	= {"ISO-8859-14","ISO_8859-14","8859-14","LATIN8","L8",0};
//static const gchar * enc_8859_15[]	= {"ISO-8859-15","ISO_8859-15","8859-15",0};
//static const gchar * enc_8859_16[]	= {"ISO-8859-16","ISO_8859-16","8859-16",0};
static const gchar * enc_2022_jp[]	= {"ISO-2022-JP",0};
// There are 4 JIS encodings which are not Shift-JIS...
static const gchar * enc_johab[]	= {"JOHAB","CP1361",0};
static const gchar * enc_koi8r[]	= {"KOI8-R",0};
static const gchar * enc_koi8u[]	= {"KOI8-U",0};
static const gchar * enc_ksc5601[]	= {"KSC_5601","KS_C_5601-1987","KS_C_5601-1989","KOREAN",0};
static const gchar * enc_macarab[]	= {"MacArabic",0};
static const gchar * enc_macceur[] = {"MacCentralEurope",0};
static const gchar * enc_maccroat[]	= {"MacCroatian",0};
static const gchar * enc_maccyr[]	= {"MacCyrillic",0};
static const gchar * enc_macgrk[]	= {"MacGreek",0};
static const gchar * enc_macheb[]	= {"MacHebrew",0};
static const gchar * enc_macice[]	= {"MacIceLand",0};
static const gchar * enc_macrom[]	= {"MacRoman","MACINTOSH","MAC",0};
static const gchar * enc_macrman[]	= {"MacRomania",0};
static const gchar * enc_macthai[]	= {"MacThai",0};
static const gchar * enc_macturk[]	= {"MacTurkish",0};
static const gchar * enc_macukr[]	= {"MacUkraine",0};
//static const gchar * enc_mulao[]	= {"MULELAO-1",0};
static const gchar * enc_next[]	= {"NEXTSTEP",0};
static const gchar * enc_sjis[]	= {"SJIS","SHIFT_JIS","SHIFT-JIS","MS_KANJI",0};
static const gchar * enc_tcvn[]	= {"TCVN","TCVN-5712","TCVN5712-1",0};
static const gchar * enc_tis620[]	= {"TIS-620","TIS620","TIS620-0",0};
static const gchar * enc_ucs2be[]	= {"UCS-2BE","UCS-2-BE","UNICODEBIG","UNICODE-1-1",0};
static const gchar * enc_ucs2le[]	= {"UCS-2LE","UCS-2-LE","UNICODELITTLE",0};
static const gchar * enc_ucs4be[]	= {"UCS-4BE","UCS-4-BE",0};
static const gchar * enc_ucs4le[]	= {"UCS-4LE","UCS-4-LE",0};
// US-ASCII has more aliases if we need them
static const gchar * enc_usascii[]	= {"US-ASCII","ASCII","US",0};
static const gchar * enc_utf7[]	= {"UTF-7","UNICODE-1-1-UTF-7",0};
static const gchar * enc_utf8[]	= {"UTF-8",0};
static const gchar * enc_utf16be[]	= {"UTF-16BE","UTF-16-BE",0};
static const gchar * enc_utf16le[]	= {"UTF-16LE","UTF-16-LE",0};
static const gchar * enc_utf32be[]	= {"UTF-32BE","UTF-32-BE",0};
static const gchar * enc_utf32le[]	= {"UTF-32LE","UTF-32-LE",0};
static const gchar * enc_viscii[]	= {"VISCII",0};

static enc_entry s_Table[] = {{}};

static int s_compareQ(const void * a, const void *b)
{
	const enc_entry * A = static_cast<const enc_entry *>(a);
	const enc_entry * B = static_cast<const enc_entry *>(b);

	if (A->id < B->id) 
	{
		return -1;
	}
	else if (A->id > B->id) 
	{
		return 1;
	}
	return 0;
}

static int s_compareB(const void * l, const void *e)
{
	const gchar * L   = static_cast<const gchar *>(l);
	const enc_entry * E = static_cast<const enc_entry *>(e);
	return strcmp(L, E->encs[0]);
}

bool UT_Encoding::s_Init = true;
UT_uint32 UT_Encoding::s_iCount = 0;


/*!
  Construct encoding class

 Find out which encodings the iconv on this system supports.
 We try several possible names for each encoding.
 If any name is successfully opened it becomes the only name for this encoding.
 If no name is successfully opened the encoding is removed from the table.
 */
UT_Encoding::UT_Encoding()
{
	if (s_Init) //only do this once
	{
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
		
		// Test all the encodings in our master table
		// Build a list of only those supported by the current iconv
		UT_uint32 iCheckIndex = 0;
		UT_uint32 iOkayIndex = 0;

		while (iCheckIndex < G_N_ELEMENTS(s_Table))
		{
			const gchar * szName;
			const gchar * szEnc;
			UT_uint32 iAltIndex;
			bool bFound = false;

			UT_DEBUGMSG(("Encoding '%s' = ",s_Table[iCheckIndex].encs[0]));
			for (iAltIndex = 0; (szEnc = s_Table[iCheckIndex].encs[iAltIndex]); ++iAltIndex)
			{
				UT_iconv_t iconv_handle = UT_iconv_open(szEnc,szEnc);
				if (UT_iconv_isValid(iconv_handle))
				{
					bFound = true;
					UT_iconv_close(iconv_handle);
					s_Table[iOkayIndex].encs[0] = szEnc;
					s_Table[iOkayIndex].encs[1] = 0;
					s_Table[iOkayIndex].desc = szName;
					s_Table[iOkayIndex].id = s_Table[iCheckIndex].id;
					UT_DEBUGMSG(("'%s' (alias %d)\n",szEnc,iAltIndex+1));
					++iOkayIndex;
					break;
				}
			}
			if (bFound == false)
			{
				UT_DEBUGMSG(("** Not supported **\n"));
			}
			++iCheckIndex;
		}
		s_iCount = iOkayIndex;

		qsort(s_Table, s_iCount, sizeof(enc_entry), s_compareQ);

		s_Init = false;
	}
}

UT_uint32 UT_Encoding::getCount()
{
	UT_ASSERT (s_Init == false);
	return s_iCount;
}

const gchar * UT_Encoding::getNthEncoding(UT_uint32 n)
{
	UT_ASSERT (s_Init == false);
	return (s_Table[n].encs[0]);
}

const gchar * UT_Encoding::getNthDescription(UT_uint32 n)
{
	UT_ASSERT (s_Init == false);
	return (s_Table[n].desc);
}


const gchar * UT_Encoding::getEncodingFromDescription(const gchar * desc)
{
	UT_ASSERT (s_Init == false);
	for (UT_uint32 i = 0; i < s_iCount; i++)
	{
		if (!strcmp(desc, s_Table[i].desc))
		{
			return s_Table[i].encs[0];
		}
	}
	return NULL;
}

UT_uint32 UT_Encoding::getIndxFromEncoding(const gchar * enc)
{
	UT_ASSERT(s_Init == false);
	for (UT_uint32 i = 0; i < s_iCount; i++)
	{
		if (!strcmp(enc, s_Table[i].encs[0]))
		{
			return i;
		}
	}
	return 0;
}

UT_uint32 UT_Encoding::getIdFromEncoding(const gchar * enc)
{
	UT_ASSERT (s_Init == false);
	enc_entry * e = static_cast<enc_entry *>(bsearch(enc, s_Table, s_iCount, sizeof(enc_entry), s_compareB));
	if (e)
	{
		return e->id;
	}
	else
	{
		return 0;
	}
}

