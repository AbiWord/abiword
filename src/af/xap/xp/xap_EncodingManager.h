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

#ifndef XAP_ENCMGR_H
#define XAP_ENCMGR_H

#include "ut_types.h"
#include "ut_pair.h"

#include "ut_xml.h"

extern "C" {
#include "iconv.h"
}

struct XAP_LangInfo
{
	/*no memeber can have NULL value. If string is empty, then value is
	 not defined. All fields are strings to simplify searches.
	*/
	enum fieldidx { longname_idx, /*this field is not empty*/
			isoshortname_idx /*ISO*/, 
			winlangcode_idx, /*0x400 + atoi() it to get a value*/
			macname_idx, /*e.g. "langRussian" or empty*/
			maclangcode_idx, /*atoi() it to get a value*/
			max_idx = maclangcode_idx };

	const char*		fields[max_idx+1];
};


class XAP_EncodingManager
{
public:
    /*
	this shouldn't return NULL. Don't free or write to returned string. 
	The string should be uppercased (extra font tarballs assume this).
    */
    virtual const char* getNativeEncodingName() const;

	inline virtual UT_Bool isUnicodeLocale() const {return m_bIsUnicodeLocale;}

    /*
	This shouldn't return NULL. Don't free or write to returned string. 
	Returns ISO two-letter name like "en"
    */    
    virtual const char* getLanguageISOName() const;

    /*
	This can return NULL. Don't free or write to returned string. 
	Returns ISO two-letter territory name like "UK".
    */    

    virtual const char* getLanguageISOTerritory() const;

#if 0
    /*
	for exporting to Tex - in order to provide proper argument for 
	{inputenc}, e.g. \usepackage[koi8-r]{inputenc}
	If NULL is returned, then package 'inputenc' is not used at all.
    */
    virtual const char* getNativeTexEncodingName() const;
#else
    virtual void placeholder() {};//to be removed
#endif    
    /*
	Should return "\n"-terminated prologue that loads required packages, 
	etc.
    */
    virtual const char* getTexPrologue() const;
    
    /*these return 0 if they can't convert*/
    /*
	This won't work for c>0xff. Use UT_Mbtowc!
    */
    virtual UT_UCSChar try_nativeToU(UT_UCSChar c) const;
    /*
	If returned value is > 0xff, then multibyte seq is returned by iconv
	and return value mean nothing (except that it notes that singlebyte 
	encoding can't be used for this character.
    */
    virtual UT_UCSChar try_UToNative(UT_UCSChar c)  const;

    /*these are used for reading/writing of doc and rtf files. */
    virtual UT_UCSChar try_WindowsToU(UT_UCSChar c) const;
    virtual UT_UCSChar try_UToWindows(UT_UCSChar c)  const;
    
    
    
    
    virtual char fallbackChar(UT_UCSChar c) const;

    virtual ~XAP_EncodingManager();

    /*  This tries to approximate the character with the string, e.g.
	horizontal-elipsis -> "...". Returns # of chars written or 0 if can't. 
	The returned string will be in ascii (i.e. it will be representable 
	in any encoding).
	If 'max_length' is 1, then approrixmation with exactly one character is
	requested. If 'max_length' is not 1, then it's rather large (e.g. 16) - 
	so there is no need to check whether there is enough free space in the 
	buffer.	
    */
    virtual UT_uint32  approximate(char* out,UT_uint32 max_length,UT_UCSChar c) const;

    /* 
       This should return 0 if it's unknown. Used only when exporting the 
       document.
    */
	virtual UT_uint32  getWinLanguageCode() const;

       /* 
	  0 means Ascii. See _CHARSET macros (e.g RUSSIAN_CHARSET) in wingdi.h
	  from Win32 SDK.	Used only when exporting RTF.
        */
	virtual UT_uint32  getWinCharsetCode() const;
	
	/*can be called several times - e.g. by constructor of port-specific
	 implementation. */
	virtual void initialize();
	/*
	    returns 1 if current langauge is CJK (chinese, japanese, korean)
	*/
	inline virtual bool cjk_locale() const { return is_cjk_; }

	/*  whether words can be broken at any character of the word (wide 
	    character, not byte). True for japanese.
	*/
	virtual bool can_break_words() const;

	/*
	    returns true if there is no distinction between upper and lower
	    letters.
	*/
	virtual bool single_case() const;	

	/*
	    returns true if all letters are non-CJK. Under non-cjk locales
	    it returns 1. Under cjk locales, returns 1 if all chars <0xff 
	    in that range.
	*/
	virtual bool noncjk_letters(const UT_UCSChar* str,int len) const;

	/*
	    This one correlates with can_break_words() very tightly.
            Under CJK locales it returns 1 for cjk letters. 
	    Under non-CJK locales returns 0.
	*/
	virtual bool can_break_at(const UT_UCSChar c) const;

	/*
	    This should be as precise as possible.
	*/
	virtual bool is_cjk_letter(UT_UCSChar c) const;
	
	/*
	    This is rather smart wrapper for wvLIDToCodePageConverter.
	    Not all CP* are known by current iconv's, so this function
	    will try first to return charset string that iconv knows.
	*/
	virtual const char* charsetFromCodepage(int lid) const;

	/*
	    This is convert charset to codepage.
	*/
	virtual const char* CodepageFromCharset(char *charset) const;

	/*
	    returns charsetFromCodepage( getWinLanguageCode() )
	*/
	virtual const char* WindowsCharsetName() const;
	
        /* these use try_ methods, and if they fail, fallbackChar() is returned*/
	UT_UCSChar nativeToU(UT_UCSChar c) const;
	UT_UCSChar UToNative(UT_UCSChar c)  const;
	UT_UCSChar WindowsToU(UT_UCSChar c) const;
	UT_UCSChar UToWindows(UT_UCSChar c)  const;
	
	/*
	  this will convert the string 'in' from charset 'charset' to
	  native charset. This is mostly shorthand function.
	  it returns ptr to translated string - it will be either
	  'in' or will be in static storage that shouldn't be freed.
	  Allowed values for 'charset' include "" and NULL in which case
	  'in' be returned. Don't ask to translate strings longer than 2K.
          This code is mostly used for translating localized menuitems to
	  the proper charset.

	  Of course it uses iconv internally.
	*/
	const char* strToNative(const char* in,const char* charset) const;

	/*
	 Same as above, but it will use buffer provided by caller.
	*/
	const char* strToNative(const char* in,const char* charset,char* buf,int bufsz) const;
#ifndef	HAVE_LIBXML2
	/*this is used by code that reads xml using expat*/
	static int XAP_XML_UnknownEncodingHandler(void *encodingHandlerData,
                                          const XML_Char *name,
                                          XML_Encoding *info);
#endif
	XAP_EncodingManager();
	static XAP_EncodingManager*		instance;	
	
	/*it's terminated with the record with NULL in language name. */
	static const XAP_LangInfo		langinfo[];
	/*
	    Precise meaning:
		swap_utos: the following seq should produce a seq in buf that
			    iconv will understand correctly when converting
			    from UCS to mbs.
		    unsigned short V;
		    char buf[2];
		    b0 = V&0xff, b1 = V>>8;
		    buf[swap_utos]=b0;
		    buf[!swap_utos]=b1;
		swap_stou: the following seq should produce a correct value V
		    that iconv will understand correctly when converting
			    from mbs to UCS (i.e. return value).
		    iconv(cd,&inptr,&inlen,&outptr,&outlen);
		    unsigned short V;
		    b0 = outptr_orig[swap_stou],b1 = outptr_orig[!swap_stou];
		    V = b0 | (b1<<8);		    
	*/
	static bool 				swap_utos,swap_stou;
	
	/* these are utility functions. Since all fields are strings, 
	we can use the same routine. Returns NULL if nothing was found. */
	static const XAP_LangInfo* findLangInfo(const char* key,
		XAP_LangInfo::fieldidx column);
		
			/*word uses non-ascii names of fonts in .doc*/
	static UT_Pair				cjk_word_fontname_mapping,
			/* CJK users need slightly different set of fontsizes*/
						fontsizes_list;
protected:
	void describe();

	const char* TexPrologue;
	UT_uint32 WinLanguageCode,WinCharsetCode;
	UT_Bool is_cjk_,can_break_words_,m_bIsUnicodeLocale;
};

/*
    This one returns NULL-terminated vector of strings in static buffers (i.e.
	don't try to free anything). On next call, filled data will be lost.
    returns the following strings surrounded by prefix and suffix:
    if (!skip_fallback)
	"";
	//next ones also include 'sep' to the left of them
    "%s"	XAP_E..M..::instance->getLanguageISOName()
    "%s"	XAP_E..M..::getNativeEncodingName()    
    "%s-%s"	XAP_E..M..::getLanguageISOName(),XAP_E..M..::getLanguageISOTerritory()
    "%s-%s.%s"  XAP_E..M..::getLanguageISOName(), \
	    XAP_E..M..::getLanguageISOTerritory(), getNativeEncodingName()
*/
const char** localeinfo_combinations(const char* prefix,const char* suffix,const char* sep, bool skip_fallback=0);

/*these one are used by ispell*/
/* placate win32 compiler */
extern "C" {
extern int XAP_EncodingManager__swap_stou,XAP_EncodingManager__swap_utos;
const char * xap_encoding_manager_get_language_iso_name(void);
}
/*
 Use this instead of iconv(cd,NULL,NULL,NULL,NULL) since iconv in glibc-2.[01]
 will crash otherwise.
*/
void UT_iconv_reset(iconv_t cd);


#endif /* XAP_APP_H */
