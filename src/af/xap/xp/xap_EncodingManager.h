/*
    By Vlad Harchev <hvv@hippo.ru>
*/

#ifndef XAP_ENCMGR_H
#define XAP_ENCMGR_H

#include "ut_types.h"

#ifdef HAVE_GNOME_XML2
#include <libxml/parser.h>
#else
#include "xmlparse.h"
#endif

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

	char*		fields[max_idx+1];
};


class XAP_EncodingManager
{
public:
    /*
	this shouldn't return NULL. Don't free or write to returned string. 
	The string should be uppercased (extra font tarballs assume this).
    */
    virtual const char* getNativeEncodingName() const;
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
    /*
	for exporting to Tex - in order to provide proper argument for 
	{inputenc}, e.g. \usepackage[koi8-r]{inputenc}
	If NULL is returned, then package 'inputenc' is not used at all.
    */
    virtual const char* getNativeTexEncodingName() const;
    /*
	For exporting to Text - in order to provide argument for package
	'babel', e.g. \usepackage[english,russian]{babel}. The returned
	value is inserted between square brackets - nothing is appended or
	prepended. If NULL is returned, entire line "\usepackage" is not
	emitted.
    */
    virtual const char* getNativeBabelArgument() const;
    
    /*these return 0 if they can't convert*/
    virtual UT_UCSChar try_nativeToU(UT_UCSChar c) const;
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
#ifndef	HAVE_GNOME_XML2
	/*this is used by code that reads xml using expat*/
	static int XAP_XML_UnknownEncodingHandler(void *encodingHandlerData,
                                          const XML_Char *name,
                                          XML_Encoding *info);
#endif
	XAP_EncodingManager();
	static XAP_EncodingManager*		instance;	
	
	/*it's terminated with the record with NULL in language name. */
	static const XAP_LangInfo			langinfo[];
	
	/* these are utility functions. Since all fields are strings, 
	we can use the same routine. Returns NULL if nothing was found. */
	static const XAP_LangInfo* findLangInfo(const char* key,
		XAP_LangInfo::fieldidx column);
protected:
	void describe();		
};

#endif /* XAP_APP_H */
