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

#include "iconv.h"

#include "ut_Encoding.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Strings.h"
#include <stdlib.h>


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
// TODO Note that certain operations in Abiword currently try to open or
// TODO compare certain encodings via hard-coded names.  This should be
// TODO discouraged and replaced with names derived as in these tables.
//
// TODO This code should probably move into the Encoding Manager.

static XML_Char * enc_armscii[]	= {"ARMSCII-8",0};
static XML_Char * enc_ascii[]	= {"US-ASCII","ASCII","US",0};
static XML_Char * enc_big5[]	= {"BIG5","BIG-5","BIG-FIVE","BIGFIVE","CN-BIG5",0};
static XML_Char * enc_cp874[]	= {"CP874",0};
static XML_Char * enc_cp932[]	= {"CP932",0};
static XML_Char * enc_cp936[]	= {"CP936","GBK",0};
static XML_Char * enc_cp949[]	= {"CP949","UHC",0};
static XML_Char * enc_cp950[]	= {"CP950",0};
static XML_Char * enc_cp1250[]	= {"CP1250","WINDOWS-1250","MS-EE",0};
static XML_Char * enc_cp1251[]	= {"CP1251","WINDOWS-1251","MS-CYRL",0};
static XML_Char * enc_cp1252[]	= {"CP1252","WINDOWS-1252","MS-ANSI",0};
static XML_Char * enc_cp1253[]	= {"CP1253","WINDOWS-1253","MS-GREEK",0};
static XML_Char * enc_cp1254[]	= {"CP1254","WINDOWS-1254","MS-TURK",0};
static XML_Char * enc_cp1255[]	= {"CP1255","WINDOWS-1255","MS-HEBR",0};
static XML_Char * enc_cp1256[]	= {"CP1256","WINDOWS-1256","MS-ARAB",0};
static XML_Char * enc_cp1257[]	= {"CP1257","WINDOWS-1257","WINBALTRIM",0};
static XML_Char * enc_cp1258[]	= {"CP1258","WINDOWS-1258",0};
static XML_Char * enc_euc_cn[]	= {"EUC-CN","EUCCN","GB2312","CN-GB",0};	// Cf. GB_2312-80
static XML_Char * enc_euc_jp[]	= {"EUC-JP","EUCJP",0};
static XML_Char * enc_euc_kr[]	= {"EUC-KR","EUCKR",0};
static XML_Char * enc_euc_tw[]	= {"EUC-TW","EUCTW",0};
static XML_Char * enc_gb2312[]	= {"GB_2312-80","ISO-IR-58","CHINESE",0};	// Cf. EUC-CN
static XML_Char * enc_georga[]	= {"GEORGIAN-ACADEMY",0};
static XML_Char * enc_georgps[]	= {"GEORGIAN-PS",0};
static XML_Char * enc_hz[]		= {"HZ","HZ-GB-2312",0};
static XML_Char * enc_8859_1[]	= {"ISO-8859-1","ISO_8859-1","LATIN1","L1",0};
static XML_Char * enc_8859_2[]	= {"ISO-8859-2","ISO_8859-2","LATIN2","L2",0};
static XML_Char * enc_8859_3[]	= {"ISO-8859-3","ISO_8859-3","LATIN3","L3",0};
static XML_Char * enc_8859_4[]	= {"ISO-8859-4","ISO_8859-4","LATIN4","L4",0};
static XML_Char * enc_8859_5[]	= {"ISO-8859-5","ISO_8859-5","CYRILLIC",0};
static XML_Char * enc_8859_6[]	= {"ISO-8859-6","ISO_8859-6","ECMA-114","ASMO-708","ARABIC",0};
static XML_Char * enc_8859_7[]	= {"ISO-8859-7","ISO_8859-7","ECMA-118","ELOT_928","GREEK8","GREEK",0};
static XML_Char * enc_8859_8[]	= {"ISO-8859-8","ISO_8859-8","HEBREW",0};
static XML_Char * enc_8859_9[]	= {"ISO-8859-9","ISO_8859-9","LATIN5","L5",0};
static XML_Char * enc_2022_jp[]	= {"ISO-2022-JP",0};
static XML_Char * enc_johab[]	= {"JOHAB","CP1361",0};
static XML_Char * enc_koi8r[]	= {"KOI8-R",0};
static XML_Char * enc_koi8u[]	= {"KOI8-U",0};
static XML_Char * enc_ksc5601[]	= {"KSC_5601","KS_C_5601-1987","KS_C_5601-1989","KOREAN",0};
static XML_Char * enc_macarab[]	= {"MacArabic",0};
static XML_Char * enc_macceur[] = {"MacCentralEurope",0};
static XML_Char * enc_maccyr[]	= {"MacCyrillic",0};
static XML_Char * enc_macgrk[]	= {"MacGreek",0};
static XML_Char * enc_macheb[]	= {"MacHebrew",0};
static XML_Char * enc_macrom[]	= {"MacRoman","MACINTOSH","MAC",0};
static XML_Char * enc_macthai[]	= {"MacThai",0};
static XML_Char * enc_macturk[]	= {"MacTurkish",0};
static XML_Char * enc_macukr[]	= {"MacUkraine",0};
static XML_Char * enc_sjis[]	= {"SJIS","SHIFT_JIS","SHIFT-JIS","MS_KANJI",0};
static XML_Char * enc_tcvn[]	= {"TCVN","TCVN-5712","TCVN5712-1",0};
static XML_Char * enc_tis620[]	= {"TIS-620","TIS620","TIS620-0",0};
static XML_Char * enc_ucs2be[]	= {"UCS-2BE","UCS-2-BE","UNICODEBIG","UNICODE-1-1",0};
static XML_Char * enc_ucs2le[]	= {"UCS-2LE","UCS-2-LE","UNICODELITTLE",0};
static XML_Char * enc_utf7[]	= {"UTF-7","UNICODE-1-1-UTF-7",0};
static XML_Char * enc_utf8[]	= {"UTF-8",0};
static XML_Char * enc_viscii[]	= {"VISCII",0};

static enc_entry s_Table[] = 
{
	//the property value, the localised translation, the numerical id
	{enc_armscii,			NULL, XAP_STRING_ID_ENC_33},
    {enc_ascii,	    		NULL, XAP_STRING_ID_ENC_0},
	{enc_big5,				NULL, XAP_STRING_ID_ENC_40},
	{enc_cp874,				NULL, XAP_STRING_ID_ENC_22},
	{enc_cp932,				NULL, XAP_STRING_ID_ENC_46},
	{enc_cp936,				NULL, XAP_STRING_ID_ENC_39},
	{enc_cp949,				NULL, XAP_STRING_ID_ENC_50},
	{enc_cp950,				NULL, XAP_STRING_ID_ENC_42},
	{enc_cp1250,			NULL, XAP_STRING_ID_ENC_5},
	{enc_cp1251,			NULL, XAP_STRING_ID_ENC_14},
	{enc_cp1252,			NULL, XAP_STRING_ID_ENC_2},
	{enc_cp1253,			NULL, XAP_STRING_ID_ENC_10},
	{enc_cp1254,			NULL, XAP_STRING_ID_ENC_19},
	{enc_cp1255,			NULL, XAP_STRING_ID_ENC_28},
	{enc_cp1256,			NULL, XAP_STRING_ID_ENC_31},
	{enc_cp1257,			NULL, XAP_STRING_ID_ENC_8},
	{enc_cp1258,			NULL, XAP_STRING_ID_ENC_26},
	{enc_euc_cn,			NULL, XAP_STRING_ID_ENC_36},
	{enc_euc_jp,			NULL, XAP_STRING_ID_ENC_44},
	{enc_euc_kr,			NULL, XAP_STRING_ID_ENC_48},
	{enc_euc_tw,			NULL, XAP_STRING_ID_ENC_41},
	{enc_gb2312,			NULL, XAP_STRING_ID_ENC_37},
	{enc_georga,			NULL, XAP_STRING_ID_ENC_34},
	{enc_georgps,			NULL, XAP_STRING_ID_ENC_35},
	{enc_hz,				NULL, XAP_STRING_ID_ENC_38},
	{enc_8859_1,			NULL, XAP_STRING_ID_ENC_1},
	{enc_8859_2,			NULL, XAP_STRING_ID_ENC_4},
	// 8859-3
	{enc_8859_4,			NULL, XAP_STRING_ID_ENC_7},
	{enc_8859_5,			NULL, XAP_STRING_ID_ENC_12},
	{enc_8859_6,			NULL, XAP_STRING_ID_ENC_30},
	{enc_8859_7,			NULL, XAP_STRING_ID_ENC_9},
	{enc_8859_8,			NULL, XAP_STRING_ID_ENC_27},
	{enc_8859_9,			NULL, XAP_STRING_ID_ENC_18},
	// 8859-10, 8859-13-16
	{enc_2022_jp,			NULL, XAP_STRING_ID_ENC_43},
	{enc_johab,				NULL, XAP_STRING_ID_ENC_49},
	{enc_koi8r,				NULL, XAP_STRING_ID_ENC_13},
	{enc_koi8u,				NULL, XAP_STRING_ID_ENC_16},
	{enc_ksc5601,			NULL, XAP_STRING_ID_ENC_47},	// ISO
	{enc_macarab,			NULL, XAP_STRING_ID_ENC_32},
	{enc_macceur,			NULL, XAP_STRING_ID_ENC_6},
	{enc_maccyr,			NULL, XAP_STRING_ID_ENC_15},
	{enc_macgrk,			NULL, XAP_STRING_ID_ENC_11},
	{enc_macheb,			NULL, XAP_STRING_ID_ENC_29},
	{enc_macrom,			NULL, XAP_STRING_ID_ENC_3},
	{enc_macthai,			NULL, XAP_STRING_ID_ENC_23},
	{enc_macturk,			NULL, XAP_STRING_ID_ENC_20},
	{enc_macukr,			NULL, XAP_STRING_ID_ENC_17},
	// other mac encodings
	{enc_sjis,				NULL, XAP_STRING_ID_ENC_45},
	{enc_tcvn,				NULL, XAP_STRING_ID_ENC_25},
	{enc_tis620,			NULL, XAP_STRING_ID_ENC_21},
	{enc_ucs2be,			NULL, XAP_STRING_ID_ENC_53},
	{enc_ucs2le,			NULL, XAP_STRING_ID_ENC_54},
	// UCS-4 be and le
	{enc_utf7,				NULL, XAP_STRING_ID_ENC_51},
	{enc_utf8,				NULL, XAP_STRING_ID_ENC_52},
	// UTF-16, UTF-32 be and le
	{enc_viscii,			NULL, XAP_STRING_ID_ENC_24},
};

static int s_compareQ(const void * a, const void *b)
{
	const enc_entry * A = (const enc_entry *) a;
	const enc_entry * B = (const enc_entry *) b;

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
	const XML_Char * L   = (const XML_Char * ) l;
	const enc_entry * E = (const enc_entry *) e;
	return UT_strcmp(L, E->encs[0]);
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

		while (iCheckIndex < NrElements(s_Table))
		{
			XML_Char * szName = (XML_Char *) pSS->getValue(s_Table[iCheckIndex].id);
			XML_Char * szEnc;
			UT_uint32 iAltIndex;
			bool bFound = false;

			for (iAltIndex = 0; (szEnc = s_Table[iCheckIndex].encs[iAltIndex]); ++iAltIndex)
			{
				iconv_t iconv_handle = iconv_open(szEnc,szEnc);
				if (iconv_handle != (iconv_t)-1)
				{
					bFound = true;
					iconv_close(iconv_handle);
					s_Table[iOkayIndex].encs[0] = szEnc;
					s_Table[iOkayIndex].encs[1] = 0;
					s_Table[iOkayIndex].desc = szName;
					s_Table[iOkayIndex].id = s_Table[iCheckIndex].id;
					++iOkayIndex;
					break;
				}
				else
				{
					UT_DEBUGMSG(("Encoding '%s' unknown\n",szEnc));
				}
			}
			if (bFound == false)
			{
				//don't do this as it SEGfaults. What was the author original
				//intent?
				//UT_DEBUGMSG(("Encoding '%s' not supported at all\n",s_Table[iCheckIndex].desc[0]));
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

const XML_Char * UT_Encoding::getNthEncoding(UT_uint32 n)
{
	UT_ASSERT (s_Init == false);
	return (s_Table[n].encs[0]);
}

const XML_Char * UT_Encoding::getNthDescription(UT_uint32 n)
{
	UT_ASSERT (s_Init == false);
	return (s_Table[n].desc);
}


const XML_Char * UT_Encoding::getEncodingFromDescription(const XML_Char * desc)
{
	UT_ASSERT (s_Init == false);
	for (UT_uint32 i = 0; i < s_iCount; i++)
	{
		if (!UT_strcmp(desc, s_Table[i].desc))
		{
			return s_Table[i].encs[0];
		}
	}
	return NULL;
}

UT_uint32 UT_Encoding::getIndxFromEncoding(const XML_Char * enc)
{
	UT_ASSERT(s_Init == false);
	for (UT_uint32 i = 0; i < s_iCount; i++)
	{
		if (!UT_strcmp(enc, s_Table[i].encs[0]))
		{
			return i;
		}
	}
	return 0;
}

UT_uint32 UT_Encoding::getIdFromEncoding(const XML_Char * enc)
{
	UT_ASSERT (s_Init == false);
	enc_entry * e = (enc_entry *) bsearch(enc, s_Table, s_iCount, sizeof(enc_entry), s_compareB);
	if (e)
	{
		return e->id;
	}
	else
	{
		return 0;
	}
}

// this function is not as useless as might seem; it takes a pointer to a property string, finds the same
// property in the static table and returns the pointer to it
// this is used by fp_TextRun to set its m_pEncoding member; by always refering into the static table
// it is possible to compare the encoding property by simply comparing the pointers, rather than
// having to use strcmp

const XML_Char *  UT_Encoding::getEncodingFromEncoding(const XML_Char * enc)
{
	UT_ASSERT (s_Init == false);
	enc_entry * e = (enc_entry *) bsearch(enc, s_Table, s_iCount, sizeof(enc_entry), s_compareB);
	if (e)
	{
		return e->encs[0];
	}
	else
	{
		return 0;
	}
}

