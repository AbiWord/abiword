/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2000
 * Orignially by Vlad Harchev <hvv@hippo.ru>
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

#include "xap_EncodingManager.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_bijection.h"
#include "ut_iconv.h"

#include <stdio.h>
#include <string.h>

static UT_iconv_t iconv_handle_N2U      = UT_ICONV_INVALID;
static UT_iconv_t iconv_handle_U2N      = UT_ICONV_INVALID;
static UT_iconv_t iconv_handle_U2Latin1 = UT_ICONV_INVALID;
static UT_iconv_t iconv_handle_U2Win    = UT_ICONV_INVALID;
static UT_iconv_t iconv_handle_Win2U    = UT_ICONV_INVALID;

XAP_EncodingManager*	XAP_EncodingManager::_instance = NULL;

/*!
 * Returns the native encoding, no matter what
 *
 * If the OS supports a system locale and a user locale,
 * this will be the user locale's encoding.
 */
const char* XAP_EncodingManager::getNativeEncodingName() const
{
    return "ISO-8859-1"; /* this will definitely work*/
}

/*!
 * Returns the system's underlying native encoding, no matter what
 *
 * If the OS supports a system locale and a user locale,
 * this will be the system locale's encoding.
 */
const char* XAP_EncodingManager::getNativeSystemEncodingName() const
{
    return getNativeEncodingName();
}

/*!
 * Returns the native 8-bit encoding
 *
 * Always returns an 8-bit encoding, even when the native
 * encoding is wide char like UCS-2 on Windows NT.
 * This includes multibyte encodings such as UTF-8 and CJK encodings.
 * Any encoding which inludes the ASCII range is OK.
 */
const char* XAP_EncodingManager::getNative8BitEncodingName() const
{
    return getNativeEncodingName();
}

/*!
 * Returns the native non-Unicode encoding
 *
 * Always returns a non-Unicode encoding, even when the native
 * encoding is Unicode like UCS-2 on Windows NT or UTF-8 on *nix.
 */
const char* XAP_EncodingManager::getNativeNonUnicodeEncodingName() const
{
    return getNativeEncodingName();
}

/*!
 * Returns the native Unicode encoding
 *
 * Typically UTF-8 on *nix and UCS-2 on Windows NT
 */
const char* XAP_EncodingManager::getNativeUnicodeEncodingName() const
{
    return "UTF-8"; /* this will definitely work*/
}

static const char * UCS2BEName = 0;
static const char * UCS2LEName = 0;
static const char * UCS4BEName = 0;
static const char * UCS4LEName = 0;

/*!
 * Returns the name this system uses for UCS-2, big endian
 *
 * UCS-2BE is standard but some older iconvs use UCS-2-BE
 */
const char* XAP_EncodingManager::getUCS2BEName() const
{
	return UCS2BEName;
}

/*!
 * Returns the name this system uses for UCS-2, little endian
 *
 * UCS-2LE is standard but some older iconvs use UCS-2-LE
 */
const char* XAP_EncodingManager::getUCS2LEName() const
{
	return UCS2LEName;
}

/*!
 * Returns the name this system uses for UCS-4, big endian
 *
 * UCS-4BE is standard
 */
const char* XAP_EncodingManager::getUCS4BEName() const
{
	return UCS4BEName;
}

/*!
 * Returns the name this system uses for UCS-4, little endian
 *
 * UCS-4LE is standard
 */
const char* XAP_EncodingManager::getUCS4LEName() const
{
	return UCS4LEName;
}

XAP_EncodingManager::~XAP_EncodingManager()
{
	UT_iconv_close(iconv_handle_N2U);
	UT_iconv_close(iconv_handle_U2N);
	UT_iconv_close(iconv_handle_U2Latin1);
	UT_iconv_close(iconv_handle_U2Win);
	UT_iconv_close(iconv_handle_Win2U);
}

XAP_EncodingManager::XAP_EncodingManager() { }

void XAP_EncodingManager::Delete_instance()
{
	delete _instance;
	_instance = NULL;
}

const char* XAP_EncodingManager::getLanguageISOName() const 
{
    return "en";
}

const char* XAP_EncodingManager::getLanguageISOTerritory() const
{
    return NULL;
}

// TODO Do we need an equivalent function which can return
// TODO U+FFFD "REPLACEMENT CHARACTER" or U+25A0 "BLACK SQUARE"
// TODO for translating into Unicode?
char XAP_EncodingManager::fallbackChar(UT_UCSChar) const 
{ 
    return '?'; 
}

/* in fact this method should provide full implementation */

UT_uint32 XAP_EncodingManager::approximate(char* out,UT_uint32 max_length,UT_UCSChar c) const
{
	if (max_length==0)
		return 0;
	if (max_length==1)
	{
		switch (c)
		{
			case 0x201d:
			case 0x201c:
				*out = '"'; return 1;
			default:
				return 0;
		}
	} 	
	else 
	{
		/* 
		 this case can't happen with current code, so there is no
		 proper implementation.
		*/
	}
	return 0;
}

UT_UCSChar XAP_EncodingManager::nativeToU(UT_UCSChar c) const
{
    UT_UCSChar ret = try_nativeToU(c);
    return ret ? ret : (UT_UCSChar)fallbackChar(c);	
}

UT_UCSChar XAP_EncodingManager::UToNative(UT_UCSChar c)  const
{
    UT_UCSChar ret = try_UToNative(c);
    if (!ret || ret>0xff) 
    {
    	char repl;
	int repl_len = approximate(&repl,1,c);
	return repl_len == 1? repl : fallbackChar(c);
    }
    else
    	return ret;
}

UT_UCSChar XAP_EncodingManager::WindowsToU(UT_UCSChar c) const
{
    UT_UCSChar ret = try_WindowsToU(c);
    return ret ? ret : (UT_UCSChar)fallbackChar(c);	
}


UT_UCSChar XAP_EncodingManager::UToWindows(UT_UCSChar c)  const
{
    UT_UCSChar ret = try_UToWindows(c);
    return ret && ret<=0xff ? ret : (UT_UCSChar)fallbackChar(c);	
}


const char* XAP_EncodingManager::strToNative(const char* in,const char* charset, bool bReverse, bool bUseSysEncoding) const
{
	static char buf[500];
	return strToNative(in, charset, buf, sizeof(buf), bReverse, bUseSysEncoding);
}

const char* XAP_EncodingManager::strToNative(const char* in, const char* charset, char* buf, int bufsz,
											 bool bReverse, bool bUseSysEncoding) const
{
	if (!charset || !*charset || !in || !*in || !buf)
		return in; /*won't translate*/
#if 1
	// TODO this gets around the fact that gtk 1.2 cannot handle UTF-8 input
	// when we move to gtk 2 this original branch needs to be enabled
	UT_iconv_t iconv_handle;
	if (!bReverse)
		iconv_handle = UT_iconv_open(
		    bUseSysEncoding ? getNativeSystemEncodingName() : getNativeEncodingName(), charset);
	else
		iconv_handle = UT_iconv_open(
		    charset, bUseSysEncoding ? getNativeSystemEncodingName() : getNativeEncodingName());

#else
	UT_iconv_t iconv_handle;
	const char * pNative =  bUseSysEncoding ? getNativeSystemEncodingName() : getNativeEncodingName();
	
	if(!g_ascii_strcasecmp(pNative, "UTF-8"))
		pNative = getNativeNonUnicodeEncodingName();

	iconv_handle = UT_iconv_open(pNative, charset);
	xxx_UT_DEBUGMSG(("xap_EncodingManager::strToNative: pNative %s, iconv_handle 0x%x\n",pNative, iconv_handle));
#endif

	if (!UT_iconv_isValid(iconv_handle))
		return in;

	const char* inptr = in;
	char* outptr = buf;
	size_t inbytes = strlen(in);
	size_t outbytes = bufsz;	
	size_t donecnt = UT_iconv(iconv_handle, &inptr, &inbytes, &outptr, &outbytes);
	const char* retstr = in;

	if (donecnt != (size_t) -1 && inbytes == 0)
	{
		retstr = buf;
		buf[bufsz - outbytes] = '\0';/*for sure*/
	}

	UT_iconv_close(iconv_handle);
	return retstr;
}

int XAP_EncodingManager::XAP_XML_UnknownEncodingHandler(void* /*encodingHandlerData*/,
                                          const gchar *name,
                                          XML_Encoding *info)
{
#ifndef	HAVE_EXPAT
	UT_UNUSED(name);
	UT_UNUSED(info);
	return 0;
#else
	/*this is used by code that reads xml using expat*/
	if (get_instance()->cjk_locale())
	    return 0;/*this handler doesn't support multibyte encodings*/
	UT_iconv_t iconv_handle = UT_iconv_open("UCS-2",name);
	if (!UT_iconv_isValid(iconv_handle))
		return 0;
	info->convert = NULL;
	info->release = NULL;
	{
		char ibuf[1],obuf[2];		
		for(int i=0;i<256;++i)
		{
			size_t ibuflen = 1, obuflen=2;
			const char* iptr = ibuf;
			char* optr = obuf;
			ibuf[0] = static_cast<unsigned char>(i);
			size_t donecnt = UT_iconv(iconv_handle,&iptr,&ibuflen,&optr,&obuflen);			
			if (donecnt!=(size_t)-1 && ibuflen==0) 
			{
				unsigned short uval;
				unsigned short b0 = static_cast<unsigned char>(obuf[swap_stou]);
				unsigned short b1 = static_cast<unsigned char>(obuf[!swap_stou]);
				uval =  b0 | (b1<<8);
				info->map[i] = static_cast<unsigned int>(uval);
			}
			else
				info->map[i] = -1;/* malformed character. Such cases exist - e.g. 0x98 in cp1251*/
			
		}
	}
	UT_iconv_close(iconv_handle);
	return 1;
#endif
}

extern "C" { char *wvLIDToCodePageConverter(unsigned short lid); }

static void init_values(const XAP_EncodingManager* that)
{
	const char * ucs4i = ucs4Internal ();
	const char * naten = that->getNativeEncodingName ();

	iconv_handle_N2U = UT_iconv_open (ucs4i, naten);
	if (!UT_iconv_isValid(iconv_handle_N2U))
		{
			UT_DEBUGMSG(("WARNING: UT_iconv_open(%s,%s) failed!\n",ucs4i,naten));
		}
	iconv_handle_U2N = UT_iconv_open (naten, ucs4i);
	if (!UT_iconv_isValid(iconv_handle_U2N))
		{
			UT_DEBUGMSG(("WARNING: UT_iconv_open(%s,%s) failed!\n",naten,ucs4i));
		}
	iconv_handle_U2Latin1 = UT_iconv_open ("ISO-8859-1", ucs4i);
	if (!UT_iconv_isValid(iconv_handle_U2Latin1))
		{
			UT_DEBUGMSG(("WARNING: UT_iconv_open(ISO-8859-1,%s) failed!\n",ucs4i));
		}
	
	char* winencname = wvLIDToCodePageConverter(that->getWinLanguageCode());
	iconv_handle_Win2U = UT_iconv_open(ucs4Internal(),winencname);
	iconv_handle_U2Win = UT_iconv_open(winencname,ucs4Internal());
}


static UT_UCSChar try_CToU(UT_UCSChar c,UT_iconv_t iconv_handle)
{
	if (!UT_iconv_isValid (iconv_handle)) return 0;
	UT_iconv_reset (iconv_handle);

	/* We don't support multibyte chars yet. wcstombcs should be used.
	 * NOTE: if so, then we shouldn't be using this function at all! [TODO: ugh.]
	 */
	if (c > 255)
		{
			UT_DEBUGMSG(("WARNING: character code %x received, substituting 'E'\n", c));
			c = 'E'; // was return 0, but that can be dangerous...
		}

	char ibuf[1];
	char obuf[4];
	size_t ibuflen = 1;
	size_t obuflen = 4;
	const char * iptr = ibuf;
	char * optr = obuf;

	ibuf[0]	= static_cast<unsigned char>(c);

	size_t donecnt = UT_iconv(iconv_handle,&iptr,&ibuflen,&optr,&obuflen);			

	UT_UCSChar uval = 0;

	if (donecnt!=(size_t)-1 && ibuflen==0) 
	{
		if (XAP_EncodingManager::swap_stou)
			{
				uval = static_cast<UT_UCSChar>(static_cast<unsigned char>(obuf[3]));
				uval = static_cast<UT_UCSChar>(static_cast<unsigned char>(obuf[2])) | (uval<<8);
				uval = static_cast<UT_UCSChar>(static_cast<unsigned char>(obuf[1])) | (uval<<8);
				uval = static_cast<UT_UCSChar>(static_cast<unsigned char>(obuf[0])) | (uval<<8);
			}
		else
			{
				uval = static_cast<UT_UCSChar>(static_cast<unsigned char>(obuf[0]));
				uval = static_cast<UT_UCSChar>(static_cast<unsigned char>(obuf[1])) | (uval<<8);
				uval = static_cast<UT_UCSChar>(static_cast<unsigned char>(obuf[2])) | (uval<<8);
				uval = static_cast<UT_UCSChar>(static_cast<unsigned char>(obuf[3])) | (uval<<8);
			}
	}
	return uval;
}

static UT_UCSChar try_UToC(UT_UCSChar c,UT_iconv_t iconv_handle)
{
	if (!UT_iconv_isValid(iconv_handle)) return 0;
	UT_iconv_reset (iconv_handle);

	char ibuf[4];
	char obuf[6];
	size_t ibuflen = 4;
	size_t obuflen = 6;
	const char * iptr = ibuf;
	char * optr = obuf;

	if (XAP_EncodingManager::swap_utos)
		{
			ibuf[0] = static_cast<char>((c      ) & 0xff);
			ibuf[1] = static_cast<char>((c >>  8) & 0xff);
			ibuf[2] = static_cast<char>((c >> 16) & 0xff);
			ibuf[3] = static_cast<char>((c >> 24) & 0xff);
		}
	else
		{
			ibuf[0] = static_cast<char>((c >> 24) & 0xff);
			ibuf[1] = static_cast<char>((c >> 16) & 0xff);
			ibuf[2] = static_cast<char>((c >>  8) & 0xff);
			ibuf[3] = static_cast<char>((c      ) & 0xff);
		}

	size_t donecnt = UT_iconv (iconv_handle,&iptr,&ibuflen,&optr,&obuflen);

	UT_UCSChar byte = 0;

	if (donecnt!=(size_t)-1 && ibuflen==0) 
	{
		if (obuflen != 5) // grr... [TODO: ugh.]
			{
				UT_DEBUGMSG(("WARNING: character code %x received, substituting 'E'\n", c));
				byte = static_cast<UT_UCSChar>(static_cast<unsigned char>('E'));
			}
		else byte = static_cast<UT_UCSChar>(static_cast<unsigned char>(obuf[0]));
	}
	return byte;
}

UT_UCSChar XAP_EncodingManager::try_nativeToU(UT_UCSChar c) const
{
	return try_CToU(c,iconv_handle_N2U);
}

UT_UCSChar XAP_EncodingManager::try_UToNative(UT_UCSChar c)  const
{
	return try_UToC(c,iconv_handle_U2N);
}

UT_UCSChar XAP_EncodingManager::try_UToLatin1(UT_UCSChar c)  const
{
	return try_UToC(c,iconv_handle_U2Latin1);
}

UT_UCSChar XAP_EncodingManager::try_WindowsToU(UT_UCSChar c) const 
{ 
	return try_CToU(c,iconv_handle_Win2U);
}

UT_UCSChar XAP_EncodingManager::try_UToWindows(UT_UCSChar c) const 
{ 
	return try_UToC(c,iconv_handle_U2Win);
}

/*reverse map*/
struct _rmap
{
	const char* value;//it can't be NULL for non-special entries like last or first
	const char** keys;//NULL-teminated array of strings
};

static const char* search_rmap(const _rmap* m,const char* key,bool* is_default = NULL)
{
	const _rmap* cur = m+1;	
	if (is_default)
		*is_default = false;
	for(;cur->value;++cur) 
	{
		if (!cur->keys)
		{
			if (!g_ascii_strcasecmp(cur->value,key))
				return cur->value;
			else
				continue;
		};
		const char** curkey = cur->keys;
		for(;*curkey;++curkey)
			if (!g_ascii_strcasecmp(*curkey,key))
				return cur->value;			
	}
	if (is_default)	
		*is_default = true;
	return m->value;
}

static const char* search_rmap_with_opt_suffix(const _rmap* m,const char* key,const char* fallback_key=NULL,const char* fallback_key_final=NULL)
{
	bool is_default;
	const char* value = search_rmap(m,key,&is_default);
	if (!is_default || !fallback_key)
		return value;
	return search_rmap_with_opt_suffix(m,fallback_key,fallback_key_final);
}


struct _map
{
	const char* key;
	const char* value;
};
static const char* search_map(const _map* m,const char* key,bool* is_default = NULL)
{
	const _map* cur = m+1;	
	if (is_default)
		*is_default = false;
	for(;cur->key;++cur)
		if (!g_ascii_strcasecmp(cur->key,key))
			return cur->value;
	if (is_default)
		*is_default = true;
	return m->value;
}

static const char* search_map_with_opt_suffix(const _map* m,const char* key,const char* fallback_key=NULL,const char* fallback_key_final=NULL)
{
	bool is_default;
	const char* value = search_map(m,key,&is_default);
	if (!is_default || !fallback_key)
		return value;
	return search_map_with_opt_suffix(m,fallback_key,fallback_key_final);
}

/* ************************* here begin tables *************************/

/* this array describes mapping form current encoding to Tex's encoding name.
 The 1st entry is default value.
 If the 'keys' field is NULL, then the 'value' of that entry should be used 
 (in the same case) if searched key matches the value.
*/
static const char* texenc_iso88595[] = { "iso8859-5", NULL };
static const _rmap native_tex_enc_map[]=
{
	{ NULL, NULL }, /* the 1st item tells default value */
	{ "koi8-r", NULL }, { "koi8-ru", NULL }, 
	{ "koi8-u", NULL }, { "cp1251", NULL }, { "cp866", NULL },
	{ "iso88595", texenc_iso88595},
	{ NULL, NULL } /*last entry has NULL as 'value'*/
};


static const _map langcode_to_babelarg[]=
{
	{NULL,NULL},
	{"ru","english,russian"},
	
	/* I'm not sure that this is correct, but my teTeX 0.9.17 works only 
	   this way (i.e. only with "russian" in the middle) - hvv */
	{"uk","english,russian,ukrainian"},
	
	/* I'm not sure again - my teTeX 0.9.17 doesn't know 'byelorussian' 
	   language - hvv */
	{"be","english,russian"},
	{NULL,NULL}
};


/*
 Mapping from langcode to windows charset code.
 See wingdi.h for charset codes windows defines, macros *_CHARSET (e.g. 
 RUSSIAN_CHARSET).
*/
static const char* wincharsetcode_ru[]= /* russian charset */
{ "ru", "be", "uk" , NULL };
static const char* wincharsetcode_el[]=  /* greek charset*/
{ "el", NULL };

static const char* wincharsetcode_tr[]=  /* turkish charset*/
{ "tr", NULL };

static const char* wincharsetcode_vi[]=  /* vietnamese charset*/
{ "vi", NULL };

static const char* wincharsetcode_th[]=  /* thai charset*/
{ "th", NULL };

/*I'm not sure that charset code is the same for Big5 and GB2312.
  Tested with GB2312 only.  
*/
static const char* wincharsetcode_zh_GB2312[]= /* chinese*/
{ "zh_CN.GB2312", "zh_CN.GBK", "zh_CN.GB18030", NULL };

static const char* wincharsetcode_zh_BIG5[]= /* chinese*/
{ "zh_TW.BIG5", "zh_HK.BIG5-HKSCS", NULL };

static const _rmap langcode_to_wincharsetcode[]=
{
	{"0", NULL }, /* default value - ansi charset*/
	{"204",wincharsetcode_ru},
	{"161",wincharsetcode_el},
	{"162",wincharsetcode_tr},
	{"163",wincharsetcode_vi},
	{"222",wincharsetcode_th},	
	{"134",wincharsetcode_zh_GB2312},
	{"136",wincharsetcode_zh_BIG5},	
	{NULL, NULL}
};

#if 0
static const UT_Bijection::pair_data zh_TW_big5[]=
{
/*
    This data was constructed from the HJ's patch for support  of Big5 to 
    AW-0.7.10 - VH
*/
    {"song","\xe5\xae\x8b\xe4\xbd\x93"},
    {"fangsong","\xe4\xbb\xbf\xe5\xae\x8b"},
    {"hei","\xe9\xbb\x91\xe4\xbd\x93"},
    {"kai","\xe6\xa5\xb7\xe4\xbd\x93"},
    {NULL,NULL}
};

static const char* zh_TW_big5_keys[]=
{  "zh_TW.BIG5", NULL };

static const _rmap cjk_word_fontname_mapping_data[]=
{
    {NULL, NULL},
    {reinterpret_cast<const char*>(zh_TW_big5),zh_TW_big5_keys},
    {NULL, NULL}
};
#endif


/*all CJK language codes should be listed here to be marked as CJK*/
static const char* cjk_languages[]=
{ "zh", "ja", "ko", NULL }; 

static const _rmap langcode_to_cjk[]=
{
	{"0", NULL}, /* default value - non-CJK environment */    
	{"1",cjk_languages},
	{NULL, NULL}
};



/*
 This table is useful since some iconv implementations don't know some cpNNNN 
 charsets but under some different name.
*/
static const _map MSCodepagename_to_charset_name_map[]=
{
	/*key,		value*/
	{ NULL,			NULL },

//	{ "CP0",		?? },					// ANSI code page
//	{ "CP1",		?? },					// OEM code page
//	{ "CP2",		?? },					// Macintosh code page
//	{ "CP3",		?? },					// The current thread's ANSI code page (Win2k)

//	{ "CP42",		?? },					// Symbol code page (Win2k)

//	{ "CP037",		?? },					// EBCDIC

	{ "CP437",		"CP437" },				// MS-DOS United States

//	{ "CP500",		?? },					// EBCDIC "500V1"

	{ "CP708",		"ASMO-708" },			// Arabic (ASMO 708)
//	{ "CP709",		?? },					// Arabic (ASMO 449+, BCON V4)
//	{ "CP710",		?? },					// Arabic (Transparent Arabic)
//	{ "CP720",		?? },					// Arabic (Transparent ASMO)

	{ "CP737",		"CP737" },				// Greek (formerly 437G)
	{ "CP775",		"CP775" },				// Baltic
	{ "CP850",		"CP850" },				// MS-DOS Multilingual (Latin I)
	{ "CP852",		"CP852" },				// MS-DOS Slavic (Latin II)
	{ "CP855",		"CP855" },				// IBM Cyrillic (primarily Russian)
	{ "CP857",		"CP857" },				// IBM Turkish
	{ "CP860",		"CP861" },				// MS-DOS Portuguese
	{ "CP861",		"CP861" },				// MS-DOS Icelandic
	{ "CP862",		"CP862" },				// Hebrew
	{ "CP863",		"CP863" },				// MS-DOS Canadian-French
	{ "CP864",		"CP864" },				// Arabic
	{ "CP865",		"CP865" },				// MS-DOS Nordic
	{ "CP866",		"CP866" },				// MS-DOS Russian
	{ "CP869",		"CP869" },				// IBM Modern Greek
	{ "CP874",		"CP874" },				// Thai
//	{ "CP875",		?? },					// EBCDIC

	{ "CP932",		"SJIS" },				// Japanese
	{ "CP936",		"GBK" },				// Chinese (PRC, Singapore)
	{ "CP949",		"CP949" },				// Korean
	{ "CP950",		"BIG5" },				// Chinese (Taiwan; Hong Kong SAR, PRC)

//	{ "CP1026",		?? },					// EBCDIC

	{ "CP1200",		"UCS-2" },				// Unicode (BMP of ISO 10646)	// ??

	{ "CP1250",		"CP1250" },				// Windows 3.1 Eastern European
	{ "CP1251",		"CP1251" },				// Windows 3.1 Cyrillic
	{ "CP1252",		"CP1252" },				// Windows 3.1 US (ANSI)
	{ "CP1253",		"CP1253" },				// Windows 3.1 Greek
	{ "CP1254",		"CP1254" },				// Windows 3.1 Turkish
	{ "CP1255",		"CP1255" },				// Hebrew
	{ "CP1256",		"CP1256" },				// Arabic
	{ "CP1257",		"CP1257" },				// Baltic

	{ "CP1361",		"JOHAB" },				// Korean (Johab)

	{ "CP10000",	"MACINTOSH" },			// Macintosh Roman (alias name can be MACROMAN, 
	                                        //  but iconv from glibc seems to not have clue
	{ "CP10001",	"SJIS" },				// Macintosh Japanese	// ??
	{ "CP10006",	"MACGREEK" },			// Macintosh Greek I
	{ "CP10007",	"MACCYRILLIC" },		// Macintosh Cyrillic
	{ "CP10029",	"MACCENTRALEUROPE" },	// Macintosh Latin 2
	{ "CP10079",	"MACICELAND" },			// Macintosh Icelandic
	{ "CP10081",	"MACTURKISH" },			// Macintosh Turkish

	{ "CP65000",	"UTF-7" },				// Translate using UTF-7 (Win2k, NT 4.0)	// ??
	{ "CP65001",	"UTF-8" },				// Translate using UTF-8 (Win2k, NT 4.0)	// ??

    { NULL,			NULL }
};

/*
 This table is only concern CJK RTF part.  It is a reverse table of
 MSCodepagename_to_charset_name_map.  Iconv doesn't know some cpNNNN,
 but M$Word knows.
*/
static const _map charset_name_to_MSCodepagename_map[]=
{
/*key,value*/
    {NULL,NULL},
	// libiconv also lists "SHIFT_JIS", "SHIFT-JIS", "MS_KANJI", "csShiftJIS"
	{"SJIS","CP932"},
    {"GB2312","CP936"},
    {"GBK","CP936"},
    {"GB18030","CP936"},
    {"BIG5","CP950"},
    {"BIG5-HKSCS","CP950"},
	{"JOHAB","CP1361"},
    {NULL,NULL}
};

/*warning: 0x400 won't be added to the values in this table. */
static const _map langcode_to_winlangcode[]=
{
/*key, value*/
    {NULL, NULL},
   {"zh_CN.GB2312",	"0x804"},     
   {"zh_CN.GBK",	"0x804"}, 
   {"zh_CN.GB18030",	"0x804"}, 
   {"zh_CN.UTF-8",	"0x804"}, 
   {"zh_HK.BIG5-HKSCS",	"0x404"},  
   {"zh_HK.UTF-8",	"0x404"},  
   {"zh_TW.BIG5",	"0x404"},  
   {"zh_TW.UTF-8",	"0x404"},  
	{NULL, NULL}
};

#undef v
#define v(x) #x
static const char* non_cjk_fontsizes[]=
{
    /* this is a list of sizes AW 0.7.11 had */
    v(8),v(9),v(10),v(11),v(12),v(14),v(16),v(18),v(20),v(22),v(24),v(26),
    v(28),v(36),v(48),v(72),NULL
};

static const char* cjk_fontsizes[]=
{
    /* this list of font sizes was in HJ's Big5 patch to AW-0.7.10 */
    v(5),v(5.5),v(6.5),v(7.5),v(9),v(10.5),v(12),v(14),v(15),v(16),v(18),
    v(22),v(24),v(26),v(36),v(42),NULL
};
#undef v

/* This structure is built from 
	http://www.unicode.org/unicode/onlinedat/languages.html 
   using lynx, sed and hands of VH.
*/
const XAP_SmartQuoteStyle XAP_EncodingManager::smartQuoteStyles[] =
{
	{ UCS_LDBLQUOTE, UCS_RDBLQUOTE }, // 0 English double quotes
	{ UCS_LQUOTE, UCS_RQUOTE }, // 1 English single quotes
	{ ((UT_UCSChar)0x00ab), ((UT_UCSChar)0x00bb) }, // 2 French double
	{ ((UT_UCSChar)0x00bb), ((UT_UCSChar)0x00ab) }, // 3 Danish double
	{ ((UT_UCSChar)0x00bb), ((UT_UCSChar)0x00bb) }, // 4
	{ UCS_LQUOTE, ((UT_UCSChar)0x201a) }, // 5
	{ UCS_RQUOTE, UCS_RQUOTE }, // 6
	{ ((UT_UCSChar)0x201a), UCS_LQUOTE }, // 7 German single
	{ ((UT_UCSChar)0x201a), UCS_RQUOTE }, // 8
	{ UCS_LDBLQUOTE, ((UT_UCSChar)0x201e) }, // 9
	{ UCS_RDBLQUOTE, UCS_RDBLQUOTE }, // 10
	{ ((UT_UCSChar)0x201e), UCS_RDBLQUOTE }, // 11
	{ ((UT_UCSChar)0x201e), UCS_LDBLQUOTE }, // 12 German double
	{ ((UT_UCSChar)0x2039), ((UT_UCSChar)0x203a) }, // 13 French single
	{ ((UT_UCSChar)0x203a), ((UT_UCSChar)0x2039) }, // 14 reverse French single
	{ ((UT_UCSChar)0x300c), ((UT_UCSChar)0x300d) }, // 15 Dark corner bracket
	{ ((UT_UCSChar)0x300e), ((UT_UCSChar)0x300f) }, // 16 White corner bracket
	{ UCS_RDBLQUOTE, UCS_LDBLQUOTE }, // 17 - Same as English, but RTL for Hebrew
	{ '\"', '\"' }, // 18 - ASCII double quote
	{ '\'', '\'' }, // 19 - ASCII single quote
	{ 0, 0 } // End of the list
};

/* This structure is built from 
	http://www.unicode.org/unicode/onlinedat/languages.html 
   using lynx, sed and hands of VH.
*/
const XAP_LangInfo XAP_EncodingManager::langinfo[] =
{
    {{   "Abkhazian",         "ab",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Afar",              "aa",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Afrikaans",         "af",     "",     "0x0036", "",                                  "",     },  12,   8   },
    {{   "Albanian",          "sq",     "",     "0x001c", "langAlbanian",                      "36",   },   2,  13   },
    {{   "Amharic",           "am",     "",     "",       "langAmharic",                       "85",   },   0,   1   },
    {{   "Arabic",            "ar",     "",     "0x0001", "langArabic",                        "12",   },   0,   1   },
    {{   "Armenian",          "hy",     "",     "",       "langArmenian",                      "51",   },   0,   1   },
    {{   "Assamese",          "as",     "",     "",       "langAssamese",                      "68",   },   0,   1   },
    {{   "Aymara",            "ay",     "",     "",       "langAymara",                        "134",  },   0,   1   },
    {{   "Azerbaijani",       "az",     "",     "",       "langAzerbaijani(Latin)",            "49",   },   0,   1   },
    {{   "Azerbaijani",       "az",     "",     "",       "langAzerbaijanAr(Arabic)",          "50",   },   0,   1   },
    {{   "Bashkir",           "ba",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Basque",            "eu",     "",     "0x002d", "langBasque",                        "129",  },   0,   1   },
    {{   "Bengali",           "bn",     "",     "",       "langBengali",                       "67",   },   0,   1   },
    {{   "Bangla",            "bn",     "",     "",       "langBengali",                       "67",   },   0,   1   },
    {{   "Bhutani",           "dz",     "",     "",       "langDzongkha",                      "137",  },   0,   1   },
    {{   "Bihari",            "bh",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Bislama",           "bi",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Breton",            "br",     "",     "",       "langBreton",                        "142",  },   0,   1   },
    {{   "Bulgarian",         "bg",     "",     "0x0002", "langBulgarian",                     "44",   },  11,   1   },
    {{   "Burmese",           "my",     "",     "",       "langBurmese",                       "77",   },   0,   1   },
    {{   "Byelorussian",      "be",     "",     "0x0023", "langByelorussian",                  "46",   },   2,   1   },
    {{   "Cambodian",         "km",     "",     "",       "langKhmer",                         "78",   },   0,   1   },
    {{   "Catalan",           "ca",     "",     "0x0003", "langCatalan",                       "130",  },   2,   0   },
    {{   "Chewa",             "",       "",     "",       "langChewa",                         "92",   },   0,   1   },
    {{   "Chinese",           "zh",     "",     "0x0004", "langTradChinese",                   "19",   },  15,  16   },
    {{   "Chinese",           "zh",     "",     "0x0004", "langSimpChinese",                   "33",   },   0,   1   },
    {{   "Cornish",	          "kw",     "",     "",       "",                                  "",     },   0,   1   },	// Jordi 19/10/2002 
    {{   "Corsican",          "co",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Croatian",          "hr",     "",     "0x001a", "langCroatian",                      "18",   },  11,   1   },
    {{   "Czech",             "cs",     "",     "0x0005", "langCzech",                         "38",   },  11,   7   },
    {{   "Danish",            "da",     "",     "0x0006", "langDanish",                        "7",    },   3,  14   },
    {{   "Dutch",             "nl",     "",     "0x0013", "langDutch",                         "4",    },  12,   8   },
    {{   "English",           "en",     "",     "0x0009", "langEnglish",                       "0",    },   0,   1   },
    {{   "Esperanto",         "eo",     "",     "",       "langEsperanto",                     "94",   },   0,   1   },
    {{   "Estonian",          "et",     "",     "0x0025", "langEstonian",                      "27",   },  11,  19   },
    {{   "Faeroese",          "fo",     "",     "0x0038", "langFaeroese",                      "30",   },   0,   1   },
    {{   "Farsi",             "fa",     "",     "0x0029", "langFarsi",                         "31",   },   0,   1   },
    {{   "Farsi",             "fa",     "",     "0x0029", "langPersian",                       "31",   },   0,   1   },
    {{   "Fiji",              "fj",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Finnish",           "fi",     "",     "0x000b", "langFinnish",                       "13",   },  10,   6   },
    {{   "Flemish",           "",       "",     "",       "langFlemish",                       "34",   },   0,   1   },
    {{   "French",            "fr",     "",     "0x000c", "langFrench",                        "1",    },   2,   1   },
    {{   "French",            "fr",     "CH",   "0x000c", "langFrench",                        "1",    },   2,  13   },
    {{   "Frisian",           "fy",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Galician",          "gl",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Galla",             "",       "",     "",       "langGalla",                         "87",   },   0,   1   },
    {{   "Georgian",          "ka",     "",     "",       "langGeorgian",                      "52",   },   0,   1   },
    {{   "German",            "de",     "",     "0x0007", "langGerman",                        "2",    },  12,   7   },
    {{   "German",            "de",     "CH",   "0x0007", "langGerman",                        "2",    },   2,  13   }, // Swiss
    {{   "Greek",             "el",     "",     "0x0008", "langGreek",                         "14",   },   2,  19   },
    {{   "Greenlandic",       "kl",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Guarani",           "gn",     "",     "",       "langGuarani",                       "133",  },   0,   1   },
    {{   "Gujarati",          "gu",     "",     "",       "langGujarati",                      "69",   },   0,   1   },
    {{   "Hausa",             "ha",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Hebrew",            "he",     "",     "0x000d", "langHebrew",                        "10",   },  17,   1   },
    {{   "Hebrew",            "iw",     "",     "0x000d", "langHebrew",                        "10",   },  17,   1   },
    {{   "Hindi",             "hi",     "",     "0x0039", "langHindi",                         "21",   },   0,   1   },
    {{   "Hungarian",         "hu",     "",     "0x000e", "langHungarian",                     "26",   },  12,   3   },
    {{   "Icelandic",         "is",     "",     "0x000f", "langIcelandic",                     "15",   },  11,   7   },
    {{   "Indonesian",        "in",     "",     "0x0021", "langIndonesian",                    "81",   },   0,   1   },
    {{   "Indonesian",        "id",     "",     "0x0021", "langIndonesian",                    "81",   },   0,   1   },
    {{   "Interlingua",       "ia",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Interlingue",       "ie",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Inuktitut",         "iu",     "",     "",       "langInuktitut",                     "143",  },   0,   1   },
    {{   "Inupiak",           "ik",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Irish",             "ga",     "",     "",       "langIrish",                         "35",   },   0,   1   },
    {{   "Italian",           "it",     "",     "0x0010", "langItalian",                       "3",    },   2,  19   },
    {{   "Italian",           "it",     "CH",   "0x0010", "langItalian",                       "3",    },   2,  13   },
    {{   "Japanese",          "ja",     "",     "0x0011", "langJapanese",                      "11",   },  15,  16   },
    {{   "Javanese",          "jw",     "",     "",       "langJavaneseRom",                   "138",  },   0,   1   },
    {{   "Kannada",           "kn",     "",     "",       "langKannada",                       "73",   },   0,   1   },
    {{   "Kashmiri",          "ks",     "",     "",       "langKashmiri",                      "61",   },   0,   1   },
    {{   "Kazakh",            "kk",     "",     "",       "langKazakh",                        "48",   },   0,   1   },
    {{   "Kinyarwanda",       "rw",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Kirghiz",           "ky",     "",     "",       "langKirghiz",                       "54",   },   0,   1   },
    {{   "Kirundi",           "rn",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Korean",            "ko",     "",     "0x0012", "langKorean",                        "23",   },   0,   1   },
    {{   "Kurdish",           "ku",     "",     "",       "langKurdish",                       "60",   },   0,   1   },
    {{   "Laothian",          "lo",     "",     "",       "langLao",                           "79",   },   0,   1   },
    {{   "Lappish",           "",       "",     "",       "langLappish",                       "29",   },   0,   1   },
    {{   "Lappish",           "",       "",     "",       "langSaamisk",                       "29",   },   0,   1   },
    {{   "Latin",             "la",     "",     "",       "langLatin",                         "131",  },   0,   1   },
    {{   "Latvian",           "lv",     "",     "0x0026", "langLatvian",                       "28",   },   2,  12   },
    {{   "Lettish",           "lv",     "",     "0x0026", "langLatvian",                       "28",   },   0,   1   },
    {{   "Lingala",           "ln",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Lithuanian",        "lt",     "",     "0x0027", "langLithuanian",                    "24",   },  11,   7   },
    {{   "Macedonian",        "mk",     "",     "0x002f", "langMacedonian",                    "43",   },   0,   1   },
    {{   "Malagasy",          "mg",     "",     "",       "langMalagasy",                      "93",   },   0,   1   },
    {{   "Malay",             "ms",     "",     "0x003e", "langMalayRoman(Latin)",             "83",   },   0,   1   },
    {{   "Malay",             "ms",     "",     "0x003e", "langMalayArabic(Arabic)",           "84",   },   0,   1   },
    {{   "Malayalam",         "ml",     "",     "",       "langMalayalam",                     "72",   },   0,   1   },
    {{   "Maltese",           "mt",     "",     "",       "langMaltese",                       "16",   },   0,   1   },
    {{   "Manx Gaelic",       "gv",     "",     "",       "langGailck",                        "141",  },   0,   1   },
    {{   "Maori",             "mi",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Marathi",           "mr",     "",     "",       "langMarathi",                       "66",   },   0,   1   },
    {{   "Marshallese",       "mh",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Moldavian",         "mo",     "",     "",       "langMoldavian",                     "53",   },   0,   1   },
    {{   "Mongolian",         "mn",     "",     "",       "langMongolian(Mongolian)",          "57",   },   0,   1   },
    {{   "Mongolian",         "mn",     "",     "",       "langMongolianCyr(Cyrillic)",        "58",   },   0,   1   },
    {{   "Nauru",             "na",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Nepali",            "ne",     "",     "",       "langNepali",                        "64",   },   0,   1   },
    {{   "Norwegian",         "no",     "",     "0x0014", "langNorwegian",                     "9",    },   2,   1   },
    {{   "Occitan",           "oc",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Oriya",             "or",     "",     "",       "langOriya",                         "71",   },   0,   1   },
    {{   "Oromo",             "om",     "",     "",       "langOromo",                         "87",   },   0,   1   },
    {{   "Afan",              "om",     "",     "",       "langOromo",                         "87",   },   0,   1   },
    {{   "Pashto",            "ps",     "",     "",       "langPashto",                        "59",   },   0,   1   },
    {{   "Pushto",            "ps",     "",     "",       "langPashto",                        "59",   },   0,   1   },
    {{   "Polish",            "pl",     "",     "0x0015", "langPolish",                        "25",   },  12,   8   },
    {{   "Portuguese",        "pt",     "",     "0x0016", "langPortuguese",                    "8",    },   2,   0   },
    {{   "Portuguese",        "pt",     "BR",   "0x0016", "langPortuguese",                    "8",    },   0,   1   },
    {{   "Punjabi",           "pa",     "",     "",       "langPunjabi",                       "70",   },   0,   1   },
    {{   "Quechua",           "qu",     "",     "",       "langQuechua",                       "132",  },   0,   1   },
    {{   "Rhaeto-Romance",    "rm",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Romanian",          "ro",     "",     "0x0018", "langRomanian",                      "37",   },  12,   2   },
    {{   "Ruanda",            "",       "",     "",       "langRuanda",                        "90",   },   0,   1   },
    {{   "Rundi",             "",       "",     "",       "langRundi",                         "91",   },   0,   1   },
    {{   "Russian",           "ru",     "",     "0x0019", "langRussian",                       "32",   },   2,  11   },
    {{   "Samoan",            "sm",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Sangro",            "sg",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Sanskrit",          "sa",     "",     "",       "langSanskrit",                      "65",   },   0,   1   },
    {{   "Sardinian",         "sc",     "",     "",       "",        			               "",     },   0,   1   }, // Jordi 19/10/2002 
    {{   "Scots Gaelic",      "gd",     "",     "",       "langGaidhlig",                      "140",  },   0,   1   },
    {{   "Serbian",           "sr",     "",     "0x001a", "langSerbian",                       "42",   },  11,   7   },
    {{   "Serbo-Croatian",    "sh",     "",     "",       "",                                  "",     },  11,   7   },
    {{   "Sesotho",           "st",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Setswana",          "tn",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Shona",             "sn",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Sindhi",            "sd",     "",     "",       "langSindhi",                        "62",   },   0,   1   },
    {{   "Singhalese",        "si",     "",     "",       "langSinhalese",                     "76",   },   0,   1   },
    {{   "Siswati",           "ss",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Slovak",            "sk",     "",     "0x001b", "langSlovak",                        "39",   },  11,   7   },
    {{   "Slovenian",         "sl",     "",     "0x0024", "langSlovenian",                     "40",   },  11,   7   },
    {{   "Somali",            "so",     "",     "",       "langSomali",                        "88",   },   0,   1   },
    {{   "Spanish",           "es",     "",     "0x000a", "langSpanish",                       "6",    },   2,   0   },
    {{   "Sundanese",         "su",     "",     "",       "langSundaneseRom",                  "139",  },   0,   1   },
    {{   "Swahili",           "sw",     "",     "0x0041", "langSwahili",                       "89",   },   0,   1   },
    {{   "Swedish",           "sv",     "",     "0x001d", "langSwedish",                       "5",    },  10,   6   },
    {{   "Tagalog",           "tl",     "",     "",       "langTagalog",                       "82",   },   0,   1   },
    {{   "Tajik",             "tg",     "",     "",       "langTajiki",                        "55",   },   0,   1   },
    {{   "Tamil",             "ta",     "",     "",       "langTamil",                         "74",   },   0,   1   },
    {{   "Tatar",             "tt",     "",     "",       "langTatar",                         "135",  },   0,   1   },
    {{   "Telugu",            "te",     "",     "",       "langTelugu",                        "75",   },   0,   1   },
    {{   "Thai",              "th",     "",     "0x001e", "langThai",                          "22",   },   0,   1   },
    {{   "Tibetan",           "bo",     "",     "",       "langTibetan",                       "63",   },   0,   1   },
    {{   "Tigrinya",          "ti",     "",     "",       "langTigrinya",                      "86",   },   0,   1   },
    {{   "Tonga",             "to",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Tsonga",            "ts",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Turkish",           "tr",     "",     "0x001f", "langTurkish",                       "17",   },   2,  13   },
    {{   "Turkmen",           "tk",     "",     "",       "langTurkmen",                       "56",   },   0,   1   },
    {{   "Twi",               "tw",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Uighur",            "ug",     "",     "",       "langUighur",                        "136",  },   0,   1   },
    {{   "Ukrainian",         "uk",     "",     "0x0022", "langUkrainian",                     "45",   },   2,  19   },
    {{   "Urdu",              "ur",     "",     "0x0020", "langUrdu",                          "20",   },   0,   1   },
    {{   "Uzbek",             "uz",     "",     "",       "langUzbek",                         "47",   },   0,   1   },
    {{   "Vietnamese",        "vi",     "",     "0x002a", "langVietnamese",                    "80",   },   0,   1   },
    {{   "Volap�k",           "vo",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Welsh",             "cy",     "",     "",       "langWelsh",                         "128",  },   1,   0   },
    {{   "Wolof",             "wo",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Xhosa",             "xh",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Yiddish",           "ji",     "",     "",       "langYiddish",                       "41",   },  17,   1   },
    {{   "Yiddish",           "yi",     "",     "",       "langYiddish",                       "41",   },  17,   1   },
    {{   "Yoruba",            "yo",     "",     "",       "",                                  "",     },   0,   1   },
    {{   "Zulu",              "zu",     "",     "",       "",                                  "",     },   0,   1   },
    {{   NULL,                "",       "",     "",       "",                                  "" ,    },   0,   1   },
};

/* 
 * Line Breaking tables 
 */


enum EUniCat {
	NONATOMIC=0, 
	ATOMIC=1, 
	PUNCNOEND=2, 
	PUNCNOSTART=3, 
	PUNCFORCE=4,
	UNKNOWN=5
};
// Prototype.
static enum EUniCat categoriseUniChar(UT_UCS4Char c);

struct SCatRange {
	UT_UCS4Char start;
	UT_UCS4Char end;
	enum EUniCat cat;
};

/* 
 * This table categorises all known Unicode characters.
 * The entries are inclusive ranges which must be in numerical order.
 *
 * Defaults should be provided by access functions for unknown characters.
 *
 * Description of categories:
 *
 * NONATOMIC    - The character does not form an atomic (i.e. non-breakable)
 *                unit. For example, English characters are of this type, 
 *                since several are needed to make a word.
 *
 * ATOMIC       - The character does form an atomic unit. For example, Chinese
 *                characters are of this type; although several characters
 *                combine to make a word, it is legal to break between any two
 *                Chinese characters.
 *
 * PUNCNOEND    - This is punctuation which must not be allow to end a line
 *                (unless forced by PUNCFORCE); for example, an open bracket.
 *                In both English and Chinese you cannot break before the
 *                '(' in "(word)".
 *
 * PUNCNOSTART  - Like PUNCNOSTART, but for punctuation which cannot start a
 *                line, e.g: ) , :
 *
 * PUNCFORCE    - Punctuation which it is always possible to break before and
 *                after regardless of what the surrounding characters are. This
 *                is for punctuation that acts like a space.
 * 
 */
struct SCatRange UniCharCats[] = {
	{0x20, 0x20, PUNCFORCE},   // Space
	{0x21, 0x21, PUNCNOSTART}, // !
	{0x22, 0x27, NONATOMIC},   // "#$%&'
	{0x28, 0x28, PUNCNOEND},   // ( 
	{0x29, 0x29, PUNCNOSTART},  // ) 
	{0x2a, 0x2b, NONATOMIC},   // *+
	{0x2c, 0x2e, PUNCNOSTART},  // ,-.
	{0x2f, 0x2f, NONATOMIC},   // /
	{0x30, 0x39, NONATOMIC},   // Western numerals.
	{0x3a, 0x3b, PUNCNOSTART}, // :;
	{0x3c, 0x3c, PUNCNOEND},   // <
	{0x3d, 0x3d, NONATOMIC},   // =
	{0x3e, 0x3f, PUNCNOSTART}, // >?
	{0x40, 0x40, NONATOMIC},   // @
	{0x41, 0x5a, NONATOMIC},   // Western A-Z.
	{0x5b, 0x5b, PUNCNOEND},   // [ 
	{0x5c, 0x5c, NONATOMIC},   // "\"
	{0x5d, 0x5d, PUNCNOSTART}, // ]
	{0x5e, 0x60, NONATOMIC},   // ^_`
	{0x61, 0x7a, NONATOMIC},   // Western a-z.
	{0x7b, 0x7b, PUNCNOEND},   // {
	{0x7c, 0x7c, NONATOMIC},   // "\" 
	{0x7d, 0x7d, PUNCNOSTART}, // }
	{0x7e, 0x7e, NONATOMIC},   // ~ 

	/* 
	  Korean Hangul Jamo 
	  Korean line breaking is similar to English. However, I'm not
	  sure if these phonetic elements can occur on their own and, if
	  they can, what the breaking rules should be.
	*/
	{0x1100, 0x11ff, NONATOMIC},     // Korean phonetic elements.

	/* General punctuation */
	{0x2002, 0x2003, PUNCFORCE},     // en-space, em-space
	{0x2010, 0x2010, PUNCNOSTART},   // Hyphen
	{0x2013, 0x2014, PUNCFORCE},     // en-dash, em-dash
	{0x2018, 0x2018, PUNCNOEND},     // Open single quote
	{0x2019, 0x2019, NONATOMIC},     // Close single quote (usually used as ')
	{0x201c, 0x201c, PUNCNOEND},     // Open double quote
	{0x201d, 0x201d, PUNCNOSTART},   // Close double quote
	{0x2020, 0x2021, PUNCNOSTART},   // Long cross, double dagger
	{0x2026, 0x2026, PUNCNOSTART},   // ...
	{0x2030, 0x2031, PUNCNOSTART},   // Per mille/10000 signs
	{0x2032, 0x2034, PUNCNOSTART},   // Single, double, triple primes

	/* CJK Blocks */

	{0x3002, 0x3002, PUNCNOSTART},   //  Ideographic full stop
	{0x3008, 0x3008, PUNCNOEND},     //  CJK <
	{0x3009, 0x3009, PUNCNOSTART},   //  CJK >
	{0x300a, 0x300a, PUNCNOEND},     //  CJK <<
	{0x300b, 0x300b, PUNCNOSTART},   //  CJK >>
	{0x300c, 0x300c, PUNCNOEND},     //  CJK quote open
	{0x300d, 0x300d, PUNCNOSTART},   //  CJK quote close
	{0x300e, 0x300e, PUNCNOEND},     //  CJK thick quote open
	{0x300f, 0x300f, PUNCNOSTART},   //  CJK thick quote close
	{0x3010, 0x3010, PUNCNOEND},     //  CJK |(
	{0x3011, 0x3011, PUNCNOSTART},   //  CJK )|
	{0x3014, 0x3014, PUNCNOEND},     //  CJK [
	{0x3015, 0x3015, PUNCNOSTART},   //  CJK ]
	{0x3016, 0x3016, PUNCNOEND},     //  CJK [(
	{0x3017, 0x3017, PUNCNOSTART},   //  CJK )]
	{0x3018, 0x3018, PUNCNOEND},     //  CJK [[
	{0x3019, 0x3019, PUNCNOSTART},   //  CJK ]]
	{0x301a, 0x301a, PUNCNOEND},     //  CJK ]|
	{0x301b, 0x301b, PUNCNOSTART},   //  CJK |[
	{0x301d, 0x301d, PUNCNOEND},     //  CJK ``
	{0x301e, 0x301e, PUNCNOSTART},   //  CJK ''

	/* Hangul Compatibility Jamo */
	{0x3130, 0x318f, NONATOMIC},

	/* More CJK blocks */
	{0x3200, 0x32ff, ATOMIC},        // Enclosed CJK Letters and Months
	{0x3300, 0x33ff, ATOMIC},        // CJK Compatibility
	{0x3400, 0x34ff, ATOMIC},        // CJK Unified Ideographs Ext. A
	{0x4e00, 0x9faf, ATOMIC},        // CJK Unified Ideographs 

	/* Hangul Syllabuls */
	{0xac00, 0xd7af, NONATOMIC},
	
	/* Another CJK block */
	{0xf900,  0xfaff, ATOMIC},       // CJK Compatibility Ideographs

	/* Halfwidth and Fullwidth Forms. */
	{0xff01, 0xff01, PUNCNOSTART},   // !
	{0xff02, 0xff02, ATOMIC},        // "
	{0xff05, 0xff05, PUNCNOSTART},   // %
	{0xff06, 0xff07, ATOMIC},        // &'
	{0xff08, 0xff08, PUNCNOEND},     // (
	{0xff09, 0xff09, PUNCNOSTART},   // )
	{0xff0a, 0xff0b, ATOMIC},        // *+
	{0xff0c, 0xff0c, PUNCNOSTART},   // ,
	{0xff0d, 0xff0d, ATOMIC},        // -
	{0xff0e, 0xff0e, PUNCNOSTART},   // .
	{0xff0f, 0xff0f, ATOMIC},        // /
	{0xff10, 0xff19, ATOMIC},        // Numerals
	{0xff1a, 0xff1b, PUNCNOSTART},   // :;
	{0xff1c, 0xff1c, PUNCNOEND},     // <
	{0xff1d, 0xff1d, ATOMIC},        // =
	{0xff1e, 0xff1e, PUNCNOSTART},   // >
	{0xff1f, 0xff1f, PUNCNOSTART},   // ?
	{0xff20, 0xff20, ATOMIC},        // @
	{0xff21, 0xff3a, ATOMIC},        // A-Z
	{0xff3b, 0xff3b, PUNCNOEND},     // [
	{0xff3c, 0xff3c, ATOMIC},        // "\" 
	{0xff3d, 0xff3d, PUNCNOSTART},   // ]
	{0xff3e, 0xff5a, ATOMIC},        // ^_`a-z
	{0xff5b, 0xff5b, PUNCNOEND},     // {
	{0xff5c, 0xff5c, ATOMIC},        // |
	{0xff5d, 0xff5d, PUNCNOSTART},   // }
	{0xff5e, 0xff5e, ATOMIC},        // ~
	{0xff61, 0xff61, PUNCNOSTART},   // Ideographic full stop
	{0xff62, 0xff62, PUNCNOEND},     // Halfwidth left corner bracket
	{0xff63, 0xff63, PUNCNOSTART},   // Halfwidth right corner bracket
	{0xff64, 0xff64, PUNCNOSTART},   // Halfwidth ideographic comma
	{0xffe0, 0xffe0, PUNCNOEND},     // Fullwidth Cent sign
	{0xffe1, 0xffe1, PUNCNOEND},     // Fullwidth Pound sign
	{0xffe5, 0xffe5, PUNCNOEND},     // Fullwidth Yen sign

    /* More CJK blocks. */
	
	{0x20000, 0x2a6df, ATOMIC},      // CJK Unified Ideographs Ext. B
	{0x2f800, 0x2fa1f, ATOMIC},      // CJK Compatibility Ideographs Sup.
	{0,0,ATOMIC}
};

/*
 * Boolean rules for whether a line break is allowed between all possible
 * combinations of two categories.
 */
static bool blineBreakRules[] = {
	// 2nd char:   NONATOMIC, ATOMIC, PUNCNOEND, PUNCNOSTART, PUNCFORCE
/* 1st char    */
/* NONATOMIC   */  false,     true,   false,     false,       true,
/* ATOMIC      */  true,      true,   true,      false,       true,
/* PUNCNOEND   */  false,     false,  false,     false,       true,
/* PUNCNOSTART */  true,      true,   true,      false,       true,
/* PUNCFORCE   */  true,      true,   true,      true,        true}; 

/* ************************* here end tables *************************/

const XAP_LangInfo* XAP_EncodingManager::findLangInfo(const char* key,XAP_LangInfo::fieldidx idx)
{
	if (idx > XAP_LangInfo::max_idx)
		return NULL;
	const XAP_LangInfo* cur = langinfo;
	for(; cur->fields[0]; ++cur)
		if (!g_ascii_strcasecmp(cur->fields[idx],key))
			return cur;
	return NULL;
}

const XAP_LangInfo* XAP_EncodingManager::findLangInfoByLocale(const char* locale)
{
	if (!locale)
		return NULL;
		
	std::string strISOName(locale, 2);
	std::string strCountryCode;
	
	if (strlen(locale) == 5)
	{
		strCountryCode = (locale + 3);
	}
	
	const XAP_LangInfo* cur = langinfo;
	const XAP_LangInfo* temp = NULL;
	for(; cur->fields[0]; ++cur)
	{
		if (strISOName == cur->fields[XAP_LangInfo::isoshortname_idx])
		{
			if (! *cur->fields[XAP_LangInfo::countrycode_idx])
			{
				if (strCountryCode.empty())
					return cur;
				else
					temp = cur; // store, just in case
			}
			else if (strCountryCode == cur->fields[XAP_LangInfo::countrycode_idx])
			{
				return cur;
			}
		}
	}
	
	if (temp != NULL)
	{
		return temp;
	}
	
	return NULL;
}

bool XAP_EncodingManager::swap_utos = false;
bool XAP_EncodingManager::swap_stou = false;

UT_Bijection XAP_EncodingManager::fontsizes_mapping;

void XAP_EncodingManager::initialize()
{	
	const char* isocode = getLanguageISOName(), 
	*terrname = getLanguageISOTerritory(),
	*enc = getNativeEncodingName();
	
	// UCS-2 Encoding Names
	static const char * (szUCS2BENames[]) = {
		"UTF-16BE",			// superset
		"UTF-16-BE",		// my guess		
		"UCS-2BE",			// preferred
		"UCS-2-BE",			// older libiconv
		"UNICODEBIG",		// older glibc
		"UNICODE-1-1",		// in libiconv source
		0 };
	static const char * (szUCS2LENames[]) = {
		"UTF-16LE",			// superset
		"UTF-16-LE",		// my guess
		"UCS-2LE",			// preferred
		"UCS-2-LE",			// older libiconv
		"UNICODELITTLE",	// older glibc
		0 };

	// UCS-4 Encoding Names
	static const char * (szUCS4BENames[]) = {
		"UCS-4BE",			// preferred
		"UCS-4-BE",			// older libiconv (??)
		0 };
	static const char * (szUCS4LENames[]) = {
		"UCS-4LE",			// preferred
		"UCS-4-LE",			// older libiconv (??)
		0 };

	const char ** p = 0;
	UT_iconv_t iconv_handle = UT_ICONV_INVALID;

	for (p = szUCS2BENames; *p; ++p)
	{
		if ((iconv_handle = UT_iconv_open(*p,*p)) != UT_ICONV_INVALID)
		{
			UT_iconv_close(iconv_handle);
			UCS2BEName = *p;
			break;
		}
	}
	for (p = szUCS2LENames; *p; ++p)
	{
		if ((iconv_handle = UT_iconv_open(*p,*p)) != UT_ICONV_INVALID)
		{
			UT_iconv_close(iconv_handle);
			UCS2LEName = *p;
			break;
		}
	}
	if (UCS2BEName) {
		UT_DEBUGMSG(("This iconv supports UCS-2BE as \"%s\"\n",UCS2BEName));
	}
	else {
		UT_DEBUGMSG(("This iconv does not support UCS-2BE!\n"));
	}
	if (UCS2LEName) {
		UT_DEBUGMSG(("This iconv supports UCS-2LE as \"%s\"\n",UCS2LEName));
	}
	else {
		UT_DEBUGMSG(("This iconv does not support UCS-2LE!\n"));
	}

	for (p = szUCS4BENames; *p; ++p)
	{
		if ((iconv_handle = UT_iconv_open(*p,*p)) != UT_ICONV_INVALID)
		{
			UT_iconv_close(iconv_handle);
			UCS4BEName = *p;
			break;
		}
	}
	for (p = szUCS4LENames; *p; ++p)
	{
		if ((iconv_handle = UT_iconv_open(*p,*p)) != UT_ICONV_INVALID)
		{
			UT_iconv_close(iconv_handle);
			UCS4LEName = *p;
			break;
		}
	}
	if (UCS4BEName) {
		UT_DEBUGMSG(("This iconv supports UCS-4BE as \"%s\"\n",UCS4BEName));
	}
	else {
		UT_DEBUGMSG(("This iconv does not support UCS-4BE!\n"));
	}
	if (UCS4LEName) {
		UT_DEBUGMSG(("This iconv supports UCS-4LE as \"%s\"\n",UCS4LEName));
	}
	else {
		UT_DEBUGMSG(("This iconv does not support UCS-4LE!\n"));
	}

	if(!g_ascii_strcasecmp(enc, "UTF-8") || !g_ascii_strcasecmp(enc, "UTF8")
		|| !g_ascii_strcasecmp(enc, "UTF-16") || !g_ascii_strcasecmp(enc, "UTF16")
		|| !g_ascii_strcasecmp(enc, "UCS-2") || !g_ascii_strcasecmp(enc, "UCS2"))
		m_bIsUnicodeLocale = true;
	else
		m_bIsUnicodeLocale = false;
		   		
#define SEARCH_PARAMS  fulllocname, langandterr, isocode
	char fulllocname[40],langandterr[40];
	if (terrname) {
		sprintf(langandterr,"%s_%s",isocode,terrname);
		sprintf(fulllocname,"%s_%s.%s",isocode,terrname,enc);		
	}
	else {
		strncpy(langandterr,isocode,sizeof(langandterr)-1);
		langandterr[sizeof(langandterr)-1] = '\0';
		sprintf(fulllocname,"%s.%s",isocode,enc);
	}
	const char* NativeTexEncodingName = search_rmap_with_opt_suffix(native_tex_enc_map,enc);
	const char* NativeBabelArgument = search_map_with_opt_suffix(langcode_to_babelarg,SEARCH_PARAMS);
	{
		const char* str = search_rmap_with_opt_suffix(langcode_to_wincharsetcode,SEARCH_PARAMS);
		WinCharsetCode = str ? atoi(str) : 0;			
	}
	{
		const XAP_LangInfo* found = findLangInfo(getLanguageISOName(),XAP_LangInfo::isoshortname_idx);
		const char* str;
		WinLanguageCode = 0;
		if (found && *(str=found->fields[XAP_LangInfo::winlangcode_idx]))
		{
			int val;
			if (sscanf(str,"%i",&val)==1)
				WinLanguageCode	= 0x400 + val;
		}
		str = search_map_with_opt_suffix(langcode_to_winlangcode,SEARCH_PARAMS);
		if (str) {
			int val;
			if (sscanf(str,"%i",&val)==1)
				WinLanguageCode	= val;
		};
	}
	{	
	    const char* str = search_rmap_with_opt_suffix(langcode_to_cjk,SEARCH_PARAMS);
	    is_cjk_ = *str == '1';
	}
	{
	    if (cjk_locale()) {
			/* CJK guys should do something similar to 'else' branch */	
			TexPrologue = " ";
	    } else {
		char buf[500];
		int len = 0;
		if (NativeTexEncodingName)
		    len += sprintf(buf+len,"\\usepackage[%s]{inputenc}\n",NativeTexEncodingName);
		if (NativeBabelArgument)
		    len += sprintf(buf+len,"\\usepackage[%s]{babel}\n",NativeBabelArgument);
		TexPrologue = len ? g_strdup(buf)  : " ";
	    };
	}
	{
	    fontsizes_mapping.clear();
	    const char** fontsizes = cjk_locale() ? cjk_fontsizes : non_cjk_fontsizes;
	    for(const char** cur=fontsizes; *cur; ++cur) 
		{
		  UT_String buf;// ( " " );
		  buf += *cur;
//		  buf += " ";
		  fontsizes_mapping.add(*cur, buf.c_str());
	    }
	}
	
	init_values(this); /*do this unconditionally! */	
	{
	    swap_utos = swap_stou = false;
	    swap_utos = UToNative(0x20) != 0x20;
	    swap_stou = nativeToU(0x20) != 0x20;
	    
	    XAP_EncodingManager__swap_stou = swap_stou;
	    XAP_EncodingManager__swap_utos = swap_utos;
	}
}

int XAP_EncodingManager__swap_stou,XAP_EncodingManager__swap_utos;

/*
    I'm not sure whether any non-cjk language doesn't make distinction
    between upper and lower case of the letter, but let's be prepared.
	TODO Arabic, Hebrew, Thai, Lao, all Indic scripts
*/
bool XAP_EncodingManager::single_case() const { return cjk_locale(); }

bool XAP_EncodingManager::is_cjk_letter(UT_UCSChar c) const
{
    if (!cjk_locale())
	return 0;
    return (c>0xff);
}

bool XAP_EncodingManager::noncjk_letters(const UT_UCSChar* str,int len) const
{
    if (!cjk_locale())
	return 1;
    for(int i=0;i<len;++i) {
	if (is_cjk_letter(str[i]))
	    return 0;
    };
    return 1;
}

/*
 * Returns true or false depending on whether a break between c[0] and c[1]
 * is permissible.
 */
bool XAP_EncodingManager::canBreakBetween(const UT_UCS4Char c[2]) const
{
	UT_uint8 rule;

	// CJK special case: Can't break between two em dashes.
	if (c[0] == UCS_EM_DASH && c[1] == UCS_EM_DASH)
		return false;

	// Finnish special case: opening '' quote (#13037)
	if (c[0] == UCS_RDBLQUOTE && categoriseUniChar(c[1]) == NONATOMIC)
		return false;

	// Find rule number based on character categories.
	rule = categoriseUniChar(c[0]) * 5 + categoriseUniChar(c[1]);

	// Return corresponding answer.
	return blineBreakRules[rule];
}


const char* XAP_EncodingManager::getTexPrologue() const
{
    return TexPrologue;
}

// Warning:
// This code forces us to use "GB2312", "BIG5", etc instead
// of "CP936", "CP950", etc even when our iconv supports
// the "CPxxx" form and the encodings differ.
// Be sure this is what you want if you call this function.
const char* XAP_EncodingManager::charsetFromCodepage(int lid) const
{
    static char buf[100];
    sprintf(buf,"CP%d",lid);    
    char* cpname = buf;
    bool is_default;
    const char* ret = search_map(MSCodepagename_to_charset_name_map,cpname,&is_default);
    return is_default ? cpname : ret;
}

const char* XAP_EncodingManager::CodepageFromCharset(const char *charset) const
{
    bool is_default;
    const char* ret = search_map(charset_name_to_MSCodepagename_map,charset,&is_default);
    UT_DEBUGMSG(("Belcon:in XAP_EncodingManager::CodepageFromCharset,charset=%s,ret=%s,is_default=%d\n",charset,ret,is_default));
    return is_default ? charset : ret;
}

const char* XAP_EncodingManager::WindowsCharsetName() const
{
    char* cpname = wvLIDToCodePageConverter(getWinLanguageCode());
    bool is_default;
    const char* ret = search_map(MSCodepagename_to_charset_name_map,cpname,&is_default);
    return is_default ? cpname : ret;
}

UT_uint32  XAP_EncodingManager::getWinLanguageCode() const
{
	return WinLanguageCode;
}

UT_uint32  XAP_EncodingManager::getWinCharsetCode() const
{
	return WinCharsetCode;
}

void 	XAP_EncodingManager::describe()
{
	UT_DEBUGMSG(("EncodingManager reports the following:\n"
		"	LanguageISOName is %s, LanguageISOTerritory is %s\n"		
		"	NativeEncodingName is %s, NativeSystemEncodingName is %s,\n"
		"   Native8BitEncodingName is %s, NativeNonUnicodeEncodingName is %s,\n"
		"	NativeUnicodeEncodingName is %s,\n"
		"	fallbackchar is '%c'\n"		
		"	TexPrologue follows:\n"
		"---8<--------------\n" 
			"%s" 
		"--->8--------------\n"
		
		"	WinLanguageCode is 0x%04x, WinCharsetCode is %d\n"
		"	cjk_locale %d, swap_utos %d, swap_stou %d\n",
		getLanguageISOName(), getLanguageISOTerritory() ? getLanguageISOTerritory() : "NULL",
		getNativeEncodingName(),getNativeSystemEncodingName(),
		getNative8BitEncodingName(),getNativeNonUnicodeEncodingName(),
		getNativeUnicodeEncodingName(),
		fallbackChar(1072),
		getTexPrologue(),
		getWinLanguageCode(), getWinCharsetCode(),
		int(cjk_locale()), int(swap_utos),int(swap_stou)
		));
	UT_ASSERT( UT_iconv_isValid(iconv_handle_N2U) && UT_iconv_isValid(iconv_handle_U2N) );
}


/*
    This one returns NULL-terminated vector of strings in static buffers (i.e.
	don't try to g_free anything). On next call, filled data will be lost.
    returns the following strings surrounded by prefix and suffix:
    if (!skip_fallback)
	"";
    "%s"	XAP_E..M..::instance->getLanguageISOName()
    "%s"	XAP_E..M..::getNativeEncodingName()
    "%s-%s"	XAP_E..M..::getLanguageISOName(),XAP_E..M..::getLanguageISOTerritory()
    "%s-%s.%s"  XAP_E..M..::getLanguageISOName(), \
	    XAP_E..M..::getLanguageISOTerritory(), XAP_E..M..::getNativeEncodingName()
    
*/
const char** localeinfo_combinations(const char* prefix,const char* suffix,const char* sep, bool skip_fallback)
{
	//_DEBUGMSG(("locale combinations: prefix %s, suffix %s, sep %s\n", prefix,suffix,sep));
	static UT_String buf[5];
	static const char *ptrs[6];

	for (size_t i = 1; i < 5; i++)
		buf[i] = prefix;

    int idx = 0;
    if (!skip_fallback)
	{
		buf[idx] = prefix;

		if(suffix && *suffix) // do not append nothing
			buf[idx++] += suffix;
		else idx++;
	}

    UT_String lang (XAP_EncodingManager::get_instance()->getLanguageISOName());
	UT_String territory (XAP_EncodingManager::get_instance()->getLanguageISOTerritory());
	UT_String enc (XAP_EncodingManager::get_instance()->getNativeEncodingName());

	buf[idx] += sep;
	buf[idx] += lang;
	if(suffix && *suffix)
		buf[idx++] += suffix;
	else idx++;
	
	buf[idx] += sep;
	buf[idx] += enc;
	if(suffix && *suffix)
		buf[idx++] += suffix;
	else idx++;

	buf[idx] += sep;
	buf[idx] += lang;
	buf[idx] += '-';
	buf[idx] += territory;
	if(suffix && *suffix)
		buf[idx++] += suffix;
	else idx++;

	buf[idx] += sep;
	buf[idx] += lang;
	buf[idx] += '-';
	buf[idx] += territory;
	buf[idx] += '.';
	buf[idx] += enc;
	if(suffix && *suffix)
		buf[idx++] += suffix;

	for (size_t j = 0; j < 5; ++j)
		ptrs[j] = buf[j].c_str();
	ptrs[5] = 0;
	
    return ptrs;
}

/* pspell hack */
extern "C" {
const char * xap_encoding_manager_get_language_iso_name(void)
{
  return XAP_EncodingManager::get_instance()->getLanguageISOName();
}

}


/*!
 * Return the line breaking catagory that "c" belongs to.
 *
 * Note: For performance reasons this function assumes that the entries
 *       in UniCharCats are in NUMERICAL ORDER starting with the smallest 
 *       value.
 */

static int s_compare_unichar_cats(const void * pC, const void *puc)
{
	UT_UCS4Char c = *((const UT_UCS4Char*)pC);
	const SCatRange * pUC = (const SCatRange*) puc;

	if(c < pUC->start)
		return -1;
	else if(c > pUC->end)
		return 1;

	return 0;
}

static enum EUniCat categoriseUniChar(UT_UCS4Char c) {
	enum EUniCat cat=UNKNOWN;

	// use linear search for the bottom (western part of the table, and bsearch for the
	// rest

	SCatRange * pUC = (SCatRange*)bsearch(&c, UniCharCats, G_N_ELEMENTS(UniCharCats), sizeof(SCatRange),
											  s_compare_unichar_cats);

	if(pUC)
		cat = pUC->cat;

	/*
	 * Crude defaults:
	 *
	 * If the character is not listed then assume it's nonatomic (like western
	 * letters) for all code blocks below "Armenian". If it belongs to the
	 * "Armenian" block or above we assume CJK like atomic letters.
	 *
	 * I have extended that up to 0x0800, so that Hebrew, Arabic and Syriac mostly work
	 * (see bug 9792); this might need some fine tuning though.
	 *
	 * This is not sensible, but it should at least mean that Greek, Cyrillic,
	 * maybe Korean, Chinese and maybe Japanese get handled OK.
	 */
	if (cat == UNKNOWN)
	{
		if (c < 0x0800) 
			cat = NONATOMIC;
		else
			cat = ATOMIC;
	}
	return cat;
}

