#include "xap_EncodingManager.h"
#include "ut_debugmsg.h"
#include "ut_string.h"

#include "iconv.h"
#include <stdio.h>
#include <string.h>


/*
    By Vlad Harchev <hvv@hippo.ru>
*/
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
    if (!ret) 
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
    return ret ? ret : fallbackChar(c);	
};


const char* XAP_EncodingManager::strToNative(const char* in,const char* charset) const
{
	static char buf[2000];
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
	size_t donecnt = iconv(iconv_handle,&inptr,&inbytes,&outptr,&outbytes);
	const char* retstr = in;
	if (donecnt!=(size_t)-1 && inbytes==0) {
		retstr = buf;
		buf[bufsz - outbytes] = '\0';/*for sure*/
	};
	iconv_close(iconv_handle);
	return retstr;
};

#ifndef	HAVE_GNOME_XML2
	/*this is used by code that reads xml using expat*/
int XAP_EncodingManager::XAP_XML_UnknownEncodingHandler(void* /*encodingHandlerData*/,
                                          const XML_Char *name,
                                          XML_Encoding *info)
{
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
			size_t donecnt = iconv(iconv_handle,&iptr,&ibuflen,&optr,&obuflen);			
			if (donecnt!=(size_t)-1 && ibuflen==0) 
			{
				unsigned short uval;
				unsigned short b1 = (unsigned char)obuf[0];
				unsigned short b2 = (unsigned char)obuf[1];
				uval = (b1<<8) | b2;
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
	size_t donecnt = iconv(iconv_handle,&iptr,&ibuflen,&optr,&obuflen);			
	if (donecnt!=(size_t)-1 && ibuflen==0) 
	{
		unsigned short uval;
		unsigned short b1 = (unsigned char)obuf[0];
		unsigned short b2 = (unsigned char)obuf[1];
		uval = (b1<<8) | b2;
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
		ibuf[0] = (unsigned char)(c>>8);
		ibuf[1] = (unsigned char)(c & 0xff);
	}
	size_t donecnt = iconv(iconv_handle,&iptr,&ibuflen,&optr,&obuflen);			
	if (donecnt!=(size_t)-1 && ibuflen==0) 
	{
		int len = sizeof(obuf) - obuflen;
		if (len!=1)
	 		/* 
	   		 We don't support multibyte chars yet. mbstowcs should be used.
	 		*/			
			return 0;
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

static const char* search_rmap_with_opt_suffix(const _rmap* m,const char* key,const char* fallback_key=NULL)
{
	UT_Bool is_default;
	const char* value = search_rmap(m,key,&is_default);
	if (!is_default || !fallback_key)
		return value;
	return search_rmap(m,fallback_key);
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

static const char* search_map_with_opt_suffix(const _map* m,const char* key,const char* fallback_key=NULL)
{
	UT_Bool is_default;
	const char* value = search_map(m,key,&is_default);
	if (!is_default || !fallback_key)
		return value;
	return search_map(m,fallback_key);
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

static const _rmap langcode_to_wincharsetcode[]=
{
	{"0"}, /* default value - ansi charset*/
	{"204",wincharsetcode_ru},
	{"161",wincharsetcode_el},
	{"162",wincharsetcode_tr},
	{"163",wincharsetcode_vi},
	{"222",wincharsetcode_th},	
	{NULL}
};

/* This structure is built from 
	http://www.unicode.org/unicode/onlinedat/languages.html 
   using lynx, sed and hands of VH.
   
   gcc for some reason barfs for each entry of this table:
   	"warning: aggregate has a partly bracketed initializer"
*/
const XAP_LangInfo XAP_EncodingManager::langinfo[]=
{
	{   "Abkhazian",         "ab",     "",      "",                                                    ""},
	{   "Afar",              "aa",     "",      "",                                                    ""},
	{   "Afrikaans",         "af",     "0x0036", "",                                                    ""},
	{   "Albanian",          "sq",     "0x001c", "langAlbanian",                                         "36"},
	{   "Amharic",           "am",     "",      "langAmharic",                                          "85"},
	{   "Arabic",            "ar",     "0x0001", "langArabic",                                           "12"},
	{   "Armenian",          "hy",     "",      "langArmenian",                                         "51"},
	{   "Assamese",          "as",     "",      "langAssamese",                                         "68"},
	{   "Aymara",            "ay",     "",      "langAymara",                                           "134"},
	{   "Azerbaijani",       "az",     "",      "langAzerbaijani(Latin)",     				"49"},
	{   "Azerbaijani",       "az",     "",      "langAzerbaijanAr(Arabic)",     			"50",   },
	{   "Bashkir",           "ba",     "",      "",                                                    "",},
	{   "Basque",            "eu",     "0x002d", "langBasque",                                           "129",},
	{   "Bengali",  		"bn",     "",      "langBengali",                                          "67",},
	{   "Bangla",  		"bn",     "",      "langBengali",                                          "67",   },
	{   "Bhutani",           "dz",     "",      "langDzongkha",                                         "137",},
	{   "Bihari",            "bh",     "",      "",                                                    "",},
	{   "Bislama",           "bi",     "",      "",                                                    "",},
	{   "Breton",            "br",     "",      "langBreton",                                           "142",},
	{   "Bulgarian",         "bg",     "0x0002", "langBulgarian",                                        "44",},
	{   "Burmese",           "my",     "",      "langBurmese",                                          "77",},
	{   "Byelorussian",      "be",     "0x0023", "langByelorussian",                                     "46",},
	{   "Cambodian",         "km",     "",      "langKhmer",                                            "78",},
	{   "Catalan",           "ca",     "0x0003", "langCatalan",                                          "130",},
	{   "Chewa",             "",      "",      "langChewa",                                            "92",},
	{   "Chinese",           "zh",     "0x0004", "langTradChinese",                     			"19",},
	{   "Chinese",           "zh",     "0x0004", "langSimpChinese",                     			"33",   },
	{   "Corsican",          "co",     "",      "",                                                    "",},
	{   "Croatian",          "hr",     "0x001a", "langCroatian",                                         "18",},
	{   "Czech",             "cs",     "0x0005", "langCzech",                                            "38",},
	{   "Danish",            "da",     "0x0006", "langDanish",                                           "7",},
	{   "Dutch",             "nl",     "0x0013", "langDutch",                                            "4",},
	{   "English",           "en",     "0x0009", "langEnglish",                                          "0",},
	{   "Esperanto",         "eo",     "",      "langEsperanto",                                        "94",},
	{   "Estonian",          "et",     "0x0025", "langEstonian",                                         "27",},
	{   "Faeroese",          "fo",     "0x0038", "langFaeroese",                                         "30",},
	{   "Farsi",             "fa",     "0x0029", "langFarsi",                               		"31",},
	{   "Farsi",             "fa",     "0x0029", "langPersian",                               		"31",   },
	{   "Fiji",              "fj",     "",      "",                                                    "",},
	{   "Finnish",           "fi",     "0x000b", "langFinnish",                                          "13",},
	{   "Flemish",           "",      "",      "langFlemish",                                          "34",},
	{   "French",            "fr",     "0x000c", "langFrench",                                           "1",},
	{   "Frisian",           "fy",     "",      "",                                                    "",},
	{   "Galician",          "gl",     "",      "",                                                    "",},
	{   "Galla",             "",      "",      "langGalla",                                            "87",},
	{   "Georgian",          "ka",     "",      "langGeorgian",                                         "52",},
	{   "German",            "de",     "0x0007", "langGerman",                                           "2",},
	{   "Greek",             "el",     "0x0008", "langGreek",                                            "14",},
	{   "Greenlandic",       "kl",     "",      "",                                                    "",},
	{   "Guarani",           "gn",     "",      "langGuarani",                                          "133",},
	{   "Gujarati",          "gu",     "",      "langGujarati",                                         "69",},
	{   "Hausa",             "ha",     "",      "",                                                    "",},
	{   "Hebrew",            "he", "0x000d", "langHebrew",                                           	"10",},
	{   "Hebrew",            "iw", "0x000d", "langHebrew",                                           	"10",   },
	{   "Hindi",             "hi",     "0x0039", "langHindi",                                            "21",},
	{   "Hungarian",         "hu",     "0x000e", "langHungarian",                                        "26",},
	{   "Icelandic",         "is",     "0x000f", "langIcelandic",                                        "15",},
	{   "Indonesian",        "in", "0x0021", "langIndonesian",                                       	"81",},
	{   "Indonesian",        "id", "0x0021", "langIndonesian",                                       	"81",   },
	{   "Interlingua",       "ia",     "",      "",                                                    "",},
	{   "Interlingue",       "ie",     "",      "",                                                    "",},
	{   "Inuktitut",         "iu",     "",      "langInuktitut",                                        "143",},
	{   "Inupiak",           "ik",     "",      "",                                                    "",},
	{   "Irish",             "ga",     "",      "langIrish",                                            "35",},
	{   "Italian",           "it",     "0x0010", "langItalian",                                          "3",},
	{   "Japanese",          "ja",     "0x0011", "langJapanese",                                         "11",},
	{   "Javanese",          "jw",     "",      "langJavaneseRom",                                      "138",},
	{   "Kannada",           "kn",     "",      "langKannada",                                          "73",},
	{   "Kashmiri",          "ks",     "",      "langKashmiri",                                         "61",},
	{   "Kazakh",            "kk",     "",      "langKazakh",                                           "48",},
	{   "Kinyarwanda",       "rw",     "",      "",                                                    "",},
	{   "Kirghiz",           "ky",     "",      "langKirghiz",                                          "54",},
	{   "Kirundi",           "rn",     "",      "",                                                    "",},
	{   "Korean",            "ko",     "0x0012", "langKorean",                                           "23",},
	{   "Kurdish",           "ku",     "",      "langKurdish",                                          "60",},
	{   "Laothian",          "lo",     "",      "langLao",                                              "79",},
	{   "Lappish",           "",      "",      "langLappish",                             		"29",},
	{   "Lappish",           "",      "",      "langSaamisk",                             		"29",   },
	{   "Latin",             "la",     "",      "langLatin",                                            "131",},
	{   "Latvian", "lv",     "0x0026", "langLatvian",                                          "28",},
	{   "Lettish", "lv",     "0x0026", "langLatvian",                                          "28",   },
	{   "Lingala",           "ln",     "",      "",                                                    "",},
	{   "Lithuanian",        "lt",     "0x0027", "langLithuanian",                                       "24",},
	{   "Macedonian",        "mk",     "0x002f", "langMacedonian",                                       "43",},
	{   "Malagasy",          "mg",     "",      "langMalagasy",                                         "93",},
	{   "Malay",             "ms",     "0x003e", "langMalayRoman(Latin)",       				"83",},
	{   "Malay",             "ms",     "0x003e", "langMalayArabic(Arabic)",       			"84",   },
	{   "Malayalam",         "ml",     "",      "langMalayalam",                                        "72",},
	{   "Maltese",           "mt",     "",      "langMaltese",                                          "16",},
	{   "Manx Gaelic",       "gv",    "",      "langGailck",                                           "141",},
	{   "Maori",             "mi",     "",      "",                                                    "",},
	{   "Marathi",           "mr",     "",      "langMarathi",                                          "66",},
	{   "Moldavian",         "mo",     "",      "langMoldavian",                                        "53",},
	{   "Mongolian",         "mn",     "",      "langMongolian(Mongolian)", 				"57",},
	{   "Mongolian",         "mn",     "",      "langMongolianCyr(Cyrillic)", 				"58",},
	{   "Nauru",             "na",     "",      "",                                                    "",},
	{   "Nepali",            "ne",     "",      "langNepali",                                           "64",},
	{   "Norwegian",         "no",     "0x0014", "langNorwegian",                                        "9",},
	{   "Occitan",           "oc",     "",      "",                                                    "",},
	{   "Oriya",             "or",     "",      "langOriya",                                            "71",},
	{   "Oromo",      "om",     "",      "langOromo",                                            "87",},
	{   "Afan",      "om",     "",      "langOromo",                                            "87",   },
	{   "Pashto",   "ps",     "",      "langPashto",                                           "59",},
	{   "Pushto",   "ps",     "",      "langPashto",                                           "59",   },
	{   "Polish",            "pl",     "0x0015", "langPolish",                                           "25",},
	{   "Portuguese",        "pt",     "0x0016", "langPortuguese",                                       "8",},
	{   "Punjabi",           "pa",     "",      "langPunjabi",                                          "70",},
	{   "Quechua",           "qu",     "",      "langQuechua",                                          "132",},
	{   "Rhaeto-Romance",    "rm",     "",      "",                                                    "",},
	{   "Romanian",          "ro",     "0x0018", "langRomanian",                                         "37",},
	{   "Ruanda",            "",      "",      "langRuanda",                                           "90",},
	{   "Rundi",             "",      "",      "langRundi",                                            "91",},
	{   "Russian",           "ru",     "0x0019", "langRussian",                                          "32",},
	{   "Samoan",            "sm",     "",      "",                                                    "",},
	{   "Sangro",            "sg",     "",      "",                                                    "",},
	{   "Sanskrit",          "sa",     "",      "langSanskrit",                                         "65",},
	{   "Scots Gaelic",      "gd",     "",      "langGaidhlig",                                         "140",},
	{   "Serbian",           "sr",     "0x001a", "langSerbian",                                          "42",},
	{   "Serbo-Croatian",    "sh",     "",      "",                                                    "",},
	{   "Sesotho",           "st",     "",      "",                                                    "",},
	{   "Setswana",          "tn",     "",      "",                                                    "",},
	{   "Shona",             "sn",     "",      "",                                                    "",},
	{   "Sindhi",            "sd",     "",      "langSindhi",                                           "62",},
	{   "Singhalese",        "si",     "",      "langSinhalese",                                        "76",},
	{   "Siswati",           "ss",     "",      "",                                                    "",},
	{   "Slovak",            "sk",     "0x001b", "langSlovak",                                           "39",},
	{   "Slovenian",         "sl",     "0x0024", "langSlovenian",                                        "40",},
	{   "Somali",            "so",     "",      "langSomali",                                           "88",},
	{   "Spanish",           "es",     "0x000a", "langSpanish",                                          "6",},
	{   "Sundanese",         "su",     "",      "langSundaneseRom",                                     "139",},
	{   "Swahili",           "sw",     "0x0041", "langSwahili",                                          "89",},
	{   "Swedish",           "sv",     "0x001d", "langSwedish",                                          "5",},
	{   "Tagalog",           "tl",     "",      "langTagalog",                                          "82",},
	{   "Tajik",             "tg",     "",      "langTajiki",                                           "55",},
	{   "Tamil",             "ta",     "",      "langTamil",                                            "74",},
	{   "Tatar",             "tt",     "",      "langTatar",                                            "135",},
	{   "Telugu",            "te",     "",      "langTelugu",                                           "75",},
	{   "Thai",              "th",     "0x001e", "langThai",                                             "22",},
	{   "Tibetan",           "bo",     "",      "langTibetan",                                          "63",},
	{   "Tigrinya",          "ti",     "",      "langTigrinya",                                         "86",},
	{   "Tonga",             "to",     "",      "",                                                    "",},
	{   "Tsonga",            "ts",     "",      "",                                                    "",},
	{   "Turkish",           "tr",     "0x001f", "langTurkish",                                          "17",},
	{   "Turkmen",           "tk",     "",      "langTurkmen",                                          "56",},
	{   "Twi",               "tw",     "",      "",                                                    "",},
	{   "Uighur",            "ug",     "",      "langUighur",                                           "136",},
	{   "Ukrainian",         "uk",     "0x0022", "langUkrainian",                                        "45",},
	{   "Urdu",              "ur",     "0x0020", "langUrdu",                                             "20",},
	{   "Uzbek",             "uz",     "",      "langUzbek",                                            "47",},
	{   "Vietnamese",        "vi",     "0x002a", "langVietnamese",                                       "80",},
	{   "VolapØk",           "vo",     "",      "",                                                    "",},
	{   "Welsh",             "cy",     "",      "langWelsh",                                            "128",},
	{   "Wolof",             "wo",     "",      "",                                                    "",},
	{   "Xhosa",             "xh",     "",      "",                                                    "",},
	{   "Yiddish",           "ji",     "",      "langYiddish",                                          "41",},
	{   "Yiddish",           "yi",     "",      "langYiddish",                                          "41",},
	{   "Yoruba",            "yo",     "",      "",                                                    "",},
	{   "Zulu",              "zu",     "",      "",                                                    "",},
	{ 	NULL,		  "",	   "",	    "",					 		   ""}
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

static const char* NativeTexEncodingName,*NativeBabelArgument;
static UT_uint32 WinLanguageCode,WinCharsetCode;

void XAP_EncodingManager::initialize()
{	
	const char* isocode = getLanguageISOName(), 
		   *terrname = getLanguageISOTerritory();
	char fulllocname[30];
	if (terrname)
		sprintf(fulllocname,"%s_%s",isocode,terrname);
	else
		strcpy(fulllocname,isocode);
	NativeTexEncodingName = search_rmap(native_tex_enc_map,getNativeEncodingName());
	NativeBabelArgument = search_map_with_opt_suffix(langcode_to_babelarg,fulllocname,isocode);
	{
		const char* str = search_rmap_with_opt_suffix(langcode_to_wincharsetcode,fulllocname,isocode);
		WinCharsetCode = str ? atoi(str) : 0;			
	}
	{
		const XAP_LangInfo* found = findLangInfo(getLanguageISOName(),XAP_LangInfo::isoshortname_idx);
		char* str;
		WinLanguageCode = 0;
		if (found && *(str=found->fields[XAP_LangInfo::winlangcode_idx]))
		{
			int val;
			if (sscanf(str,"%i",&val)==1)
				WinLanguageCode	= 0x400 + val;
		}
	}	
	init_values(this); /*do this unconditionally! */	
}

const char* XAP_EncodingManager::getNativeTexEncodingName() const
{
	return NativeTexEncodingName;
};

const char* XAP_EncodingManager::getNativeBabelArgument() const
{
	return NativeBabelArgument;	
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
		"	LanguageISOTerritory is %s, NativeTexEncodingName is %s\n"
		"	NativeBabelArgument is [%s], fallbackchar is '%c'\n"
		"	WinLanguageCode is 0x%04x, WinCharsetCode is %d\n",	
		getNativeEncodingName(),getLanguageISOName(),
		getLanguageISOTerritory() ? getLanguageISOTerritory() : "NULL",
		getNativeTexEncodingName() ? getNativeTexEncodingName() : "NULL",
		getNativeBabelArgument() ? getNativeBabelArgument() : "NULL",
		fallbackChar(1072), getWinLanguageCode(), getWinCharsetCode()   ));
};
