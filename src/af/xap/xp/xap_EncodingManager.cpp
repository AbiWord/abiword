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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include "xap_EncodingManager.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"

#ifdef __FreeBSD__
extern "C" {
#include "iconv.h"
}
#else
#include "iconv.h"
#endif

#include <stdio.h>
#include <string.h>


XAP_EncodingManager*	XAP_EncodingManager::instance = NULL;
const char* XAP_EncodingManager::getNativeEncodingName() const
{
    return "ISO-8859-1"; /* this will definitely work*/
}

XAP_EncodingManager::~XAP_EncodingManager() {};
XAP_EncodingManager::XAP_EncodingManager() { instance = this; initialize(); }

const char* XAP_EncodingManager::getLanguageISOName() const 
{
    return "en";
};

const char* XAP_EncodingManager::getLanguageISOTerritory() const
{
    return NULL;
};

char XAP_EncodingManager::fallbackChar(UT_UCSChar c) const 
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
};

UT_UCSChar XAP_EncodingManager::nativeToU(UT_UCSChar c) const
{
    UT_UCSChar ret = try_nativeToU(c);
    return ret ? ret : fallbackChar(c);	
};

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
};

UT_UCSChar XAP_EncodingManager::WindowsToU(UT_UCSChar c) const
{
    UT_UCSChar ret = try_WindowsToU(c);
    return ret ? ret : fallbackChar(c);	
};


UT_UCSChar XAP_EncodingManager::UToWindows(UT_UCSChar c)  const
{
    UT_UCSChar ret = try_UToWindows(c);
    return ret && ret<=0xff ? ret : fallbackChar(c);	
};


const char* XAP_EncodingManager::strToNative(const char* in,const char* charset) const
{
	static char buf[8000];
	return strToNative(in,charset,buf,sizeof(buf));
};

const char* XAP_EncodingManager::strToNative(const char* in,const char* charset,char* buf,int bufsz) const
{
	if (!charset || !*charset || !in || !*in || !buf)
		return in; /*won't translate*/
	iconv_t iconv_handle = iconv_open(getNativeEncodingName(),charset);
	if (iconv_handle == (iconv_t)-1)
		return in;
	const char* inptr = in;
	char* outptr = buf;
	size_t inbytes = strlen(in), outbytes = bufsz;	
	size_t donecnt = iconv(iconv_handle,const_cast<ICONV_CONST char**>(&inptr),&inbytes,&outptr,&outbytes);
	const char* retstr = in;
	if (donecnt!=(size_t)-1 && inbytes==0) {
		retstr = buf;
		buf[bufsz - outbytes] = '\0';/*for sure*/
	};
	iconv_close(iconv_handle);
	return retstr;
};

#ifndef	HAVE_LIBXML2
	/*this is used by code that reads xml using expat*/
int XAP_EncodingManager::XAP_XML_UnknownEncodingHandler(void* /*encodingHandlerData*/,
                                          const XML_Char *name,
                                          XML_Encoding *info)
{
	if (instance->cjk_locale())
	    return 0;/*this handler doesn't support multibyte encodings*/
	iconv_t iconv_handle = iconv_open("UCS-2",name);
	if (iconv_handle == (iconv_t)-1)
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
			ibuf[0] = (unsigned char)i;
			size_t donecnt = iconv(iconv_handle,const_cast<ICONV_CONST char**>(&iptr),&ibuflen,&optr,&obuflen);			
			if (donecnt!=(size_t)-1 && ibuflen==0) 
			{
				unsigned short uval;
				unsigned short b0 = (unsigned char)obuf[swap_stou];
				unsigned short b1 = (unsigned char)obuf[!swap_stou];
				uval =  b0 | (b1<<8);
				info->map[i] = (unsigned int) uval;
			}
			else
				info->map[i] = -1;/* malformed character. Such cases exist - e.g. 0x98 in cp1251*/
			
		}
	}
	iconv_close(iconv_handle);
	return 1;
};
#endif

static 	iconv_t iconv_handle_N2U,iconv_handle_U2N,
	iconv_handle_U2Win,iconv_handle_Win2U;



extern "C" { char *wvLIDToCodePageConverter(unsigned short lid); }
static void init_values(const XAP_EncodingManager* that)
{
	iconv_handle_N2U = iconv_open("UCS-2",that->getNativeEncodingName());
	iconv_handle_U2N = iconv_open(that->getNativeEncodingName(),"UCS-2");
	
	char* winencname = wvLIDToCodePageConverter(that->getWinLanguageCode());
	iconv_handle_Win2U = iconv_open("UCS-2",winencname);
	iconv_handle_U2Win = iconv_open(winencname,"UCS-2");
};


static UT_UCSChar try_CToU(UT_UCSChar c,iconv_t iconv_handle)
{
	 /* 
	   We don't support multibyte chars yet. wcstombcs should be used. 	   
	 */
	if (c>255)
		return 0;			
	if (iconv_handle == (iconv_t)-1)
		return 0;
	char ibuf[1],obuf[2];			
	size_t ibuflen = 1, obuflen=2;
	const char* iptr = ibuf;
	char* optr = obuf;
	ibuf[0]	= (unsigned char)c;	
	size_t donecnt = iconv(iconv_handle,const_cast<ICONV_CONST char**>(&iptr),&ibuflen,&optr,&obuflen);			
	if (donecnt!=(size_t)-1 && ibuflen==0) 
	{
		unsigned short uval;
		unsigned short b0 = (unsigned char)obuf[XAP_EncodingManager::swap_stou];
		unsigned short b1 = (unsigned char)obuf[!XAP_EncodingManager::swap_stou];
		uval = (b1<<8) | b0;
		return uval;
	} else
		return  0;
};

static UT_UCSChar try_UToC(UT_UCSChar c,iconv_t iconv_handle)
{
	if (iconv_handle == (iconv_t)-1)
		return 0;
	char ibuf[2],obuf[10];			
	size_t ibuflen = sizeof(ibuf), obuflen=sizeof(obuf);
	const char* iptr = ibuf;
	char* optr = obuf;
	{
		unsigned char b0 = c & 0xff, b1 = c >>8;
		ibuf[XAP_EncodingManager::swap_utos] = b0;
		ibuf[!XAP_EncodingManager::swap_utos] = b1;
	}
	size_t donecnt = iconv(iconv_handle,const_cast<ICONV_CONST char**>(&iptr),&ibuflen,&optr,&obuflen);
	/* reset state */
	iconv(iconv_handle,NULL,NULL,NULL,NULL);
	if (donecnt!=(size_t)-1 && ibuflen==0) 
	{
		int len = sizeof(obuf) - obuflen;
		if (len!=1)
			return 0x1ff;/* tell that singlebyte encoding can't represent it*/
		else
			return (unsigned char)*obuf;
	} else
		return  0;
};

UT_UCSChar XAP_EncodingManager::try_nativeToU(UT_UCSChar c) const
{
	return try_CToU(c,iconv_handle_N2U);
};

UT_UCSChar XAP_EncodingManager::try_UToNative(UT_UCSChar c)  const
{
	return try_UToC(c,iconv_handle_U2N);
};

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

static const char* search_rmap(const _rmap* m,const char* key,UT_Bool* is_default = NULL)
{
	const _rmap* cur = m+1;	
	if (is_default)
		*is_default = UT_FALSE;
	for(;cur->value;++cur) 
	{
		if (!cur->keys)
		{
			if (!UT_stricmp(cur->value,key))
				return cur->value;
			else
				continue;
		};
		const char** curkey = cur->keys;
		for(;*curkey;++curkey)
			if (!UT_stricmp(*curkey,key))
				return cur->value;			
	}
	if (is_default)	
		*is_default = UT_TRUE;
	return m->value;
};

static const char* search_rmap_with_opt_suffix(const _rmap* m,const char* key,const char* fallback_key=NULL,const char* fallback_key_final=NULL)
{
	UT_Bool is_default;
	const char* value = search_rmap(m,key,&is_default);
	if (!is_default || !fallback_key)
		return value;
	return search_rmap_with_opt_suffix(m,fallback_key,fallback_key_final);
};


struct _map
{
	char* key;
	char* value;
};
static const char* search_map(const _map* m,const char* key,UT_Bool* is_default = NULL)
{
	const _map* cur = m+1;	
	if (is_default)
		*is_default = UT_FALSE;
	for(;cur->key;++cur)
		if (!UT_stricmp(cur->key,key))
			return cur->value;
	if (is_default)
		*is_default = UT_TRUE;
	return m->value;
};

static const char* search_map_with_opt_suffix(const _map* m,const char* key,const char* fallback_key=NULL,const char* fallback_key_final=NULL)
{
	UT_Bool is_default;
	const char* value = search_map(m,key,&is_default);
	if (!is_default || !fallback_key)
		return value;
	return search_map_with_opt_suffix(m,fallback_key,fallback_key_final);
};

/* ************************* here begin tables *************************/

/* this array describes mapping form current encoding to Tex's encoding name.
 The 1st entry is default value.
 If the 'keys' field is NULL, then the 'value' of that entry should be used 
 (in the same case) if searched key matches the value.
*/
static const char* texenc_iso88595[] = { "iso8859-5", NULL };
static const _rmap native_tex_enc_map[]=
{
	{ NULL}, /* the 1st item tells default value */
	{ "koi8-r"}, { "koi8-ru"}, { "koi8-u"}, { "cp1251"}, { "cp866"},
	{ "iso88595", texenc_iso88595},
	{ NULL } /*last entry has NULL as 'value'*/
};


static const _map langcode_to_babelarg[]=
{
	{NULL,NULL},
	{"ru","english,russian"},
	
	/* I'm not sure that this is correct, but my TeTex 0.9.17 works only 
	   this way (i.e. only with "russian" in the middle) - hvv */
	{"uk","english,russian,ukrainian"},
	
	/* I'm not sure again - my TeTex 0.9.17 doesn't know 'byelorussian' 
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
{ "ru","be", "uk" , NULL };
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
{ "zh_CN.GB2312", "zh_TW.GB2312", NULL };

static const char* wincharsetcode_zh_BIG5[]= /* chinese*/
{ "zh_CN.BIG5", "zh_TW.BIG5", NULL };

static const _rmap langcode_to_wincharsetcode[]=
{
	{"0"}, /* default value - ansi charset*/
	{"204",wincharsetcode_ru},
	{"161",wincharsetcode_el},
	{"162",wincharsetcode_tr},
	{"163",wincharsetcode_vi},
	{"222",wincharsetcode_th},	
	{"134",wincharsetcode_zh_GB2312},
	{"136",wincharsetcode_zh_BIG5},	
	{NULL}
};

static const UT_Pair::pair_data zh_CN_big5[]=
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

static const char* zh_CN_big5_keys[]=
{  "zh_CN.BIG5", NULL };

static const _rmap cjk_word_fontname_mapping_data[]=
{
    {NULL},
    {(char*)zh_CN_big5,zh_CN_big5_keys},
    {NULL}
};


/*all CJK language codes should be listed here to be marked as CJK*/
static const char* cjk_languages[]=
{ "zh","ja","ko",NULL}; 

static const _rmap langcode_to_cjk[]=
{
	{"0"}, /* default value - non-CJK environment */    
	{"1",cjk_languages},
	{NULL}
};


static const _rmap can_break_words_data[]=
{
	{"0"}, /* default value - can't break words at any character. */    
	{"1",cjk_languages},
	{NULL}
};

/*
 This table is useful since iconv implementations don't know some cpNNNN 
 charsets but under some different name.
*/
static const _map MSCodepagename_to_charset_name_map[]=
{
/*key, value*/
    {NULL,NULL},
    {"CP936","GB2312"},
    {"CP950","BIG5"},  
    {NULL,NULL}
};

/*
 This table is only concern CJK RTF part.It is a reverse table of
 MSCodepagename_to_charset_name_map.Iconv doesn't know some cpNNNN,
 but M$Word know.
*/
static const _map charset_name_to_MSCodepagename_map[]=
{
/*key,value*/
    {NULL,NULL},
    {"GB2312","CP936"},
    {"BIG5","CP950"},
    {NULL,NULL}
};

/*warning: 0x400 won't be added to the values in this table. */
static const _map langcode_to_winlangcode[]=
{
/*key, value*/
    {NULL},
   {"zh_CN.BIG5",	"0x404"},  
   {"zh_CN.GB2312",	"0x804"},     
   {"zh_TW.BIG5",	"0x404"},  
   {"zh_TW.GB2312",	"0x804"}, 
    {NULL}
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
const XAP_LangInfo XAP_EncodingManager::langinfo[] =
{
    {{   "Abkhazian",         "ab",     "",       "",                                  ""      }},
    {{   "Afar",              "aa",     "",       "",                                  ""      }},
    {{   "Afrikaans",         "af",     "0x0036", "",                                  ""      }},
    {{   "Albanian",          "sq",     "0x001c", "langAlbanian",                      "36"    }},
    {{   "Amharic",           "am",     "",       "langAmharic",                       "85"    }},
    {{   "Arabic",            "ar",     "0x0001", "langArabic",                        "12"    }},
    {{   "Armenian",          "hy",     "",       "langArmenian",                      "51"    }},
    {{   "Assamese",          "as",     "",       "langAssamese",                      "68"    }},
    {{   "Aymara",            "ay",     "",       "langAymara",                        "134"   }},
    {{   "Azerbaijani",       "az",     "",       "langAzerbaijani(Latin)",            "49"    }},
    {{   "Azerbaijani",       "az",     "",       "langAzerbaijanAr(Arabic)",          "50"    }},
    {{   "Bashkir",           "ba",     "",       "",                                  ""      }},
    {{   "Basque",            "eu",     "0x002d", "langBasque",                        "129"   }},
    {{   "Bengali",           "bn",     "",       "langBengali",                       "67"    }},
    {{   "Bangla",            "bn",     "",       "langBengali",                       "67"    }},
    {{   "Bhutani",           "dz",     "",       "langDzongkha",                      "137"   }},
    {{   "Bihari",            "bh",     "",       "",                                  ""      }},
    {{   "Bislama",           "bi",     "",       "",                                  ""      }},
    {{   "Breton",            "br",     "",       "langBreton",                        "142"   }},
    {{   "Bulgarian",         "bg",     "0x0002", "langBulgarian",                     "44"    }},
    {{   "Burmese",           "my",     "",       "langBurmese",                       "77"    }},
    {{   "Byelorussian",      "be",     "0x0023", "langByelorussian",                  "46"    }},
    {{   "Cambodian",         "km",     "",       "langKhmer",                         "78"    }},
    {{   "Catalan",           "ca",     "0x0003", "langCatalan",                       "130"   }},
    {{   "Chewa",             "",       "",       "langChewa",                         "92"    }},
    {{   "Chinese",           "zh",     "0x0004", "langTradChinese",                   "19"    }},
    {{   "Chinese",           "zh",     "0x0004", "langSimpChinese",                   "33"    }},
    {{   "Corsican",          "co",     "",       "",                                  ""      }},
    {{   "Croatian",          "hr",     "0x001a", "langCroatian",                      "18"    }},
    {{   "Czech",             "cs",     "0x0005", "langCzech",                         "38"    }},
    {{   "Danish",            "da",     "0x0006", "langDanish",                        "7"     }},
    {{   "Dutch",             "nl",     "0x0013", "langDutch",                         "4"     }},
    {{   "English",           "en",     "0x0009", "langEnglish",                       "0"     }},
    {{   "Esperanto",         "eo",     "",       "langEsperanto",                     "94"    }},
    {{   "Estonian",          "et",     "0x0025", "langEstonian",                      "27"    }},
    {{   "Faeroese",          "fo",     "0x0038", "langFaeroese",                      "30"    }},
    {{   "Farsi",             "fa",     "0x0029", "langFarsi",                         "31"    }},
    {{   "Farsi",             "fa",     "0x0029", "langPersian",                       "31"    }},
    {{   "Fiji",              "fj",     "",       "",                                  ""      }},
    {{   "Finnish",           "fi",     "0x000b", "langFinnish",                       "13"    }},
    {{   "Flemish",           "",       "",       "langFlemish",                       "34"    }},
    {{   "French",            "fr",     "0x000c", "langFrench",                        "1"     }},
    {{   "Frisian",           "fy",     "",       "",                                  ""      }},
    {{   "Galician",          "gl",     "",       "",                                  ""      }},
    {{   "Galla",             "",       "",       "langGalla",                         "87"    }},
    {{   "Georgian",          "ka",     "",       "langGeorgian",                      "52"    }},
    {{   "German",            "de",     "0x0007", "langGerman",                        "2"     }},
    {{   "Greek",             "el",     "0x0008", "langGreek",                         "14"    }},
    {{   "Greenlandic",       "kl",     "",       "",                                  ""      }},
    {{   "Guarani",           "gn",     "",       "langGuarani",                       "133"   }},
    {{   "Gujarati",          "gu",     "",       "langGujarati",                      "69"    }},
    {{   "Hausa",             "ha",     "",       "",                                  ""      }},
    {{   "Hebrew",            "he",     "0x000d", "langHebrew",                        "10"    }},
    {{   "Hebrew",            "iw",     "0x000d", "langHebrew",                        "10"    }},
    {{   "Hindi",             "hi",     "0x0039", "langHindi",                         "21"    }},
    {{   "Hungarian",         "hu",     "0x000e", "langHungarian",                     "26"    }},
    {{   "Icelandic",         "is",     "0x000f", "langIcelandic",                     "15"    }},
    {{   "Indonesian",        "in",     "0x0021", "langIndonesian",                    "81"    }},
    {{   "Indonesian",        "id",     "0x0021", "langIndonesian",                    "81"    }},
    {{   "Interlingua",       "ia",     "",       "",                                  ""      }},
    {{   "Interlingue",       "ie",     "",       "",                                  ""      }},
    {{   "Inuktitut",         "iu",     "",       "langInuktitut",                     "143"   }},
    {{   "Inupiak",           "ik",     "",       "",                                  ""      }},
    {{   "Irish",             "ga",     "",       "langIrish",                         "35"    }},
    {{   "Italian",           "it",     "0x0010", "langItalian",                       "3"     }},
    {{   "Japanese",          "ja",     "0x0011", "langJapanese",                      "11"    }},
    {{   "Javanese",          "jw",     "",       "langJavaneseRom",                   "138"   }},
    {{   "Kannada",           "kn",     "",       "langKannada",                       "73"    }},
    {{   "Kashmiri",          "ks",     "",       "langKashmiri",                      "61"    }},
    {{   "Kazakh",            "kk",     "",       "langKazakh",                        "48"    }},
    {{   "Kinyarwanda",       "rw",     "",       "",                                  ""      }},
    {{   "Kirghiz",           "ky",     "",       "langKirghiz",                       "54"    }},
    {{   "Kirundi",           "rn",     "",       "",                                  ""      }},
    {{   "Korean",            "ko",     "0x0012", "langKorean",                        "23"    }},
    {{   "Kurdish",           "ku",     "",       "langKurdish",                       "60"    }},
    {{   "Laothian",          "lo",     "",       "langLao",                           "79"    }},
    {{   "Lappish",           "",       "",       "langLappish",                       "29"    }},
    {{   "Lappish",           "",       "",       "langSaamisk",                       "29"    }},
    {{   "Latin",             "la",     "",       "langLatin",                         "131"   }},
    {{   "Latvian",           "lv",     "0x0026", "langLatvian",                       "28"    }},
    {{   "Lettish",           "lv",     "0x0026", "langLatvian",                       "28"    }},
    {{   "Lingala",           "ln",     "",       "",                                  ""      }},
    {{   "Lithuanian",        "lt",     "0x0027", "langLithuanian",                    "24"    }},
    {{   "Macedonian",        "mk",     "0x002f", "langMacedonian",                    "43"    }},
    {{   "Malagasy",          "mg",     "",       "langMalagasy",                      "93"    }},
    {{   "Malay",             "ms",     "0x003e", "langMalayRoman(Latin)",             "83"    }},
    {{   "Malay",             "ms",     "0x003e", "langMalayArabic(Arabic)",           "84"    }},
    {{   "Malayalam",         "ml",     "",       "langMalayalam",                     "72"    }},
    {{   "Maltese",           "mt",     "",       "langMaltese",                       "16"    }},
    {{   "Manx Gaelic",       "gv",     "",       "langGailck",                        "141"   }},
    {{   "Maori",             "mi",     "",       "",                                  ""      }},
    {{   "Marathi",           "mr",     "",       "langMarathi",                       "66"    }},
    {{   "Moldavian",         "mo",     "",       "langMoldavian",                     "53"    }},
    {{   "Mongolian",         "mn",     "",       "langMongolian(Mongolian)",          "57"    }},
    {{   "Mongolian",         "mn",     "",       "langMongolianCyr(Cyrillic)",        "58"    }},
    {{   "Nauru",             "na",     "",       "",                                  ""      }},
    {{   "Nepali",            "ne",     "",       "langNepali",                        "64"    }},
    {{   "Norwegian",         "no",     "0x0014", "langNorwegian",                     "9"     }},
    {{   "Occitan",           "oc",     "",       "",                                  ""      }},
    {{   "Oriya",             "or",     "",       "langOriya",                         "71"    }},
    {{   "Oromo",             "om",     "",       "langOromo",                         "87"    }},
    {{   "Afan",              "om",     "",       "langOromo",                         "87"    }},
    {{   "Pashto",            "ps",     "",       "langPashto",                        "59"    }},
    {{   "Pushto",            "ps",     "",       "langPashto",                        "59",   }},
    {{   "Polish",            "pl",     "0x0015", "langPolish",                        "25"    }},
    {{   "Portuguese",        "pt",     "0x0016", "langPortuguese",                    "8"     }},
    {{   "Punjabi",           "pa",     "",       "langPunjabi",                       "70"    }},
    {{   "Quechua",           "qu",     "",       "langQuechua",                       "132"   }},
    {{   "Rhaeto-Romance",    "rm",     "",       "",                                  ""      }},
    {{   "Romanian",          "ro",     "0x0018", "langRomanian",                      "37"    }},
    {{   "Ruanda",            "",       "",       "langRuanda",                        "90"    }},
    {{   "Rundi",             "",       "",       "langRundi",                         "91"    }},
    {{   "Russian",           "ru",     "0x0019", "langRussian",                       "32"    }},
    {{   "Samoan",            "sm",     "",       "",                                  "" 	   }},
    {{   "Sangro",            "sg",     "",       "",                                  "" 	   }},
    {{   "Sanskrit",          "sa",     "",       "langSanskrit",                      "65"    }},
    {{   "Scots Gaelic",      "gd",     "",       "langGaidhlig",                      "140"   }},
    {{   "Serbian",           "sr",     "0x001a", "langSerbian",                       "42"    }},
    {{   "Serbo-Croatian",    "sh",     "",       "",                                  ""      }},
    {{   "Sesotho",           "st",     "",       "",                                  ""      }},
    {{   "Setswana",          "tn",     "",       "",                                  ""      }},
    {{   "Shona",             "sn",     "",       "",                                  ""      }},
    {{   "Sindhi",            "sd",     "",       "langSindhi",                        "62"    }},
    {{   "Singhalese",        "si",     "",       "langSinhalese",                     "76"    }},
    {{   "Siswati",           "ss",     "",       "",                                  ""      }},
    {{   "Slovak",            "sk",     "0x001b", "langSlovak",                        "39"    }},
    {{   "Slovenian",         "sl",     "0x0024", "langSlovenian",                     "40"    }},
    {{   "Somali",            "so",     "",       "langSomali",                        "88"    }},
    {{   "Spanish",           "es",     "0x000a", "langSpanish",                       "6"     }},
    {{   "Sundanese",         "su",     "",       "langSundaneseRom",                  "139"   }},
    {{   "Swahili",           "sw",     "0x0041", "langSwahili",                       "89"    }},
    {{   "Swedish",           "sv",     "0x001d", "langSwedish",                       "5"     }},
    {{   "Tagalog",           "tl",     "",       "langTagalog",                       "82"    }},
    {{   "Tajik",             "tg",     "",       "langTajiki",                        "55"    }},
    {{   "Tamil",             "ta",     "",       "langTamil",                         "74"    }},
    {{   "Tatar",             "tt",     "",       "langTatar",                         "135"   }},
    {{   "Telugu",            "te",     "",       "langTelugu",                        "75"    }},
    {{   "Thai",              "th",     "0x001e", "langThai",                          "22"    }},
    {{   "Tibetan",           "bo",     "",       "langTibetan",                       "63"    }},
    {{   "Tigrinya",          "ti",     "",       "langTigrinya",                      "86"    }},
    {{   "Tonga",             "to",     "",       "",                                  "" 	   }},
    {{   "Tsonga",            "ts",     "",       "",                                  "" 	   }},
    {{   "Turkish",           "tr",     "0x001f", "langTurkish",                       "17"    }},
    {{   "Turkmen",           "tk",     "",       "langTurkmen",                       "56"    }},
    {{   "Twi",               "tw",     "",       "",                                  ""      }},
    {{   "Uighur",            "ug",     "",       "langUighur",                        "136"   }},
    {{   "Ukrainian",         "uk",     "0x0022", "langUkrainian",                     "45"    }},
    {{   "Urdu",              "ur",     "0x0020", "langUrdu",                          "20"    }},
    {{   "Uzbek",             "uz",     "",       "langUzbek",                         "47"    }},
    {{   "Vietnamese",        "vi",     "0x002a", "langVietnamese",                    "80"    }},
    {{   "VolapØk",           "vo",     "",       "",                                  ""      }},
    {{   "Welsh",             "cy",     "",       "langWelsh",                         "128"   }},
    {{   "Wolof",             "wo",     "",       "",                                  ""      }},
    {{   "Xhosa",             "xh",     "",       "",                                  ""      }},
    {{   "Yiddish",           "ji",     "",       "langYiddish",                       "41"    }},
    {{   "Yiddish",           "yi",     "",       "langYiddish",                       "41"    }},
    {{   "Yoruba",            "yo",     "",       "",                                  "" 	   }},
    {{   "Zulu",              "zu",     "",       "",                                  "" 	   }},
    {{   NULL,                "",       "",       "",                                  "" 	   }}
};

/* ************************* here end tables *************************/

const XAP_LangInfo* XAP_EncodingManager::findLangInfo(const char* key,XAP_LangInfo::fieldidx idx)
{
	if (idx > XAP_LangInfo::max_idx)
		return NULL;
	const XAP_LangInfo* cur = langinfo;
	for(; cur->fields[0]; ++cur)
		if (!UT_stricmp(cur->fields[idx],key))
			return cur;
	return NULL;
};

static const char* TexPrologue;
static UT_uint32 WinLanguageCode,WinCharsetCode;
static bool is_cjk_,can_break_words_;

bool XAP_EncodingManager::swap_utos = 0;
bool XAP_EncodingManager::swap_stou = 0;

UT_Pair XAP_EncodingManager::cjk_word_fontname_mapping;
UT_Pair XAP_EncodingManager::fontsizes_list;

void XAP_EncodingManager::initialize()
{	
	const char* isocode = getLanguageISOName(), 
		   *terrname = getLanguageISOTerritory(),
		   *enc = getNativeEncodingName();
#define SEARCH_PARAMS  fulllocname, langandterr, isocode
	char fulllocname[40],langandterr[40];
	if (terrname) {
		sprintf(langandterr,"%s_%s",isocode,terrname);
		sprintf(fulllocname,"%s_%s.%s",isocode,terrname,enc);		
	}
	else {
		strcpy(langandterr,isocode);
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
	    str = search_rmap_with_opt_suffix(can_break_words_data,SEARCH_PARAMS);
	    can_break_words_ = *str == '1';
	}
	{
	    if (cjk_locale()) {
		/* CJK guys should do something similar to 'else' branch */	
	    } else {
		char buf[500];
		int len = 0;
		if (NativeTexEncodingName)
		    len += sprintf(buf+len,"\\usepackage[%s]{inputenc}\n",NativeTexEncodingName);
		if (NativeBabelArgument)
		    len += sprintf(buf+len,"\\usepackage[%s]{babel}\n",NativeBabelArgument);
		TexPrologue = len ? UT_strdup(buf)  : " ";
	    };
	}
	if (cjk_locale()) {
	    /* load fontname mapping */
	    UT_Pair::pair_data* data = (UT_Pair::pair_data* )search_rmap_with_opt_suffix(
		    cjk_word_fontname_mapping_data,SEARCH_PARAMS);
	    if (data)
		cjk_word_fontname_mapping.add(data);
	};
	{
	    fontsizes_list.clear();
	    const char** fontsizes = cjk_locale() ? cjk_fontsizes: non_cjk_fontsizes;
	    char buf[30];
	    for(const char** cur=fontsizes; *cur; ++cur) {
		sprintf(buf," %s ",*cur);
		fontsizes_list.add(*cur,buf);
	    };
	};
	
	init_values(this); /*do this unconditionally! */	
	{
	    swap_utos = swap_stou = 0;
	    swap_utos = UToNative(0x20) != 0x20;
	    swap_stou = nativeToU(0x20) != 0x20;
	    
	    XAP_EncodingManager__swap_stou = swap_stou;
	    XAP_EncodingManager__swap_utos = swap_utos;
	}
}

int XAP_EncodingManager__swap_stou,XAP_EncodingManager__swap_utos;

bool XAP_EncodingManager::can_break_words() const
{
    return can_break_words_;
};

bool XAP_EncodingManager::cjk_locale() const
{
    return is_cjk_;
};

/*
    I'm not sure whether any non-cjk language doesn't make distinction
    between upper and lower case of the letter, but let's be prepared.
*/
bool XAP_EncodingManager::single_case() const { return cjk_locale(); }

bool XAP_EncodingManager::is_cjk_letter(UT_UCSChar c) const
{
    if (!cjk_locale())
	return 0;
    return (c>0xff);
};

bool XAP_EncodingManager::noncjk_letters(const UT_UCSChar* str,int len) const
{
    if (!cjk_locale())
	return 1;
    for(int i=0;i<len;++i) {
	if (is_cjk_letter(str[i]))
	    return 0;
    };
    return 1;
};

/*
    This one correlates with can_break_words() very tightly.
        Under CJK locales it returns 1 for cjk letters. 
    Under non-CJK locales returns 0.
*/
bool XAP_EncodingManager::can_break_at(const UT_UCSChar c) const
{
    if (c == UCS_SPACE)
	return 1;
    return is_cjk_letter(c);
};


const char* XAP_EncodingManager::getTexPrologue() const
{
    return TexPrologue;
};

const char* XAP_EncodingManager::charsetFromCodepage(int lid) const
{
    static char buf[100];
    sprintf(buf,"CP%d",lid);    
    char* cpname = buf;
    UT_Bool is_default;
    const char* ret = search_map(MSCodepagename_to_charset_name_map,cpname,&is_default);
    return is_default ? cpname : ret;
};

const char* XAP_EncodingManager::CodepageFromCharset(char *charset) const
{
    UT_Bool is_default;
    const char* ret = search_map(charset_name_to_MSCodepagename_map,charset,&is_default);
    UT_DEBUGMSG(("Belcon:in XAP_EncodingManager::CodepageFromCharset,charset=%s,ret=%s,is_default=%d\n",charset,ret,is_default));
    return is_default ? charset : ret;
}

const char* XAP_EncodingManager::WindowsCharsetName() const
{
    char* cpname = wvLIDToCodePageConverter(getWinLanguageCode());
    UT_Bool is_default;
    const char* ret = search_map(MSCodepagename_to_charset_name_map,cpname,&is_default);
    return is_default ? cpname : ret;
};

UT_uint32  XAP_EncodingManager::getWinLanguageCode() const
{
	return WinLanguageCode;
};

UT_uint32  XAP_EncodingManager::getWinCharsetCode() const
{
	return WinCharsetCode;
};

void 	XAP_EncodingManager::describe()
{
	UT_DEBUGMSG(("EncodingManager reports the following:\n"
		"	NativeEncodingName is %s, LanguageISOName is %s,\n"
		"	LanguageISOTerritory is %s,  fallbackchar is '%c'\n"		
		"	TexPrologue follows:\n"
		"---8<--------------\n" 
			"%s" 
		"--->8--------------\n"
		
		"	WinLanguageCode is 0x%04x, WinCharsetCode is %d\n"
		"	cjk_locale %d, can_break_words %d, swap_utos %d, swap_stou %d\n"
		,getNativeEncodingName(),getLanguageISOName(),
		getLanguageISOTerritory() ? getLanguageISOTerritory() : "NULL",
		fallbackChar(1072), getTexPrologue(),getWinLanguageCode(),
		 getWinCharsetCode(),
		int(cjk_locale()), int(can_break_words()),int(swap_utos),int(swap_stou)
		));
	UT_ASSERT( (iconv_handle_N2U!=(iconv_t)-1) && (iconv_handle_U2N!=(iconv_t)-1));
};


/*
    This one returns NULL-terminated vector of strings in static buffers (i.e.
	don't try to free anything). On next call, filled data will be lost.
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
    typedef char buf_t[1024];
    static buf_t bufs[6];
    static char* ptrs[7];
    int idx = 0;
    if (!skip_fallback)
	if (sprintf(ptrs[idx]=bufs[idx],"%s%s",prefix,suffix)!=-1)
	    ++idx;
    const char* lang = XAP_EncodingManager::instance->getLanguageISOName(),
	*territory = XAP_EncodingManager::instance->getLanguageISOTerritory(),
	*enc = XAP_EncodingManager::instance->getNativeEncodingName();
    if (sprintf(ptrs[idx]=bufs[idx],"%s%s%s%s",prefix,sep,lang,suffix)!=-1)
	++idx;
    if (sprintf(ptrs[idx]=bufs[idx],"%s%s%s%s",prefix,sep,enc,suffix)!=-1)
	++idx;	
    if (sprintf(ptrs[idx]=bufs[idx],"%s%s%s-%s%s",prefix,sep,lang,territory,suffix)!=-1)
	++idx;
    if (sprintf(ptrs[idx]=bufs[idx],"%s%s%s-%s.%s%s",prefix,sep,lang,territory,enc,suffix)!=-1)
	++idx;
    ptrs[idx]=NULL;
    return (const char **)ptrs;
};

/* pspell hack */
extern "C"
const char * xap_encoding_manager_get_language_iso_name(void)
{
  return XAP_EncodingManager::instance->getLanguageISOName();
}
