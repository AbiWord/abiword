/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <stdlib.h>
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "ie_exp_RTF.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "pd_Style.h"
#include "gr_Graphics.h"

#include "wv.h" //for wvLIDToCodePageConverter
#include "xap_EncodingManager.h"
#include "ut_debugmsg.h"

/*****************************************************************/
/*****************************************************************/

#include "ie_exp_RTF_AttrProp.h"
#include "ie_exp_RTF_listenerWriteDoc.h"
#include "ie_exp_RTF_listenerGetProps.h"

/*****************************************************************/
/*****************************************************************/

IE_Exp_RTF::IE_Exp_RTF(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListenerWriteDoc = NULL;
	m_pListenerGetProps = NULL;
	m_bNeedUnicodeText = false;
	m_braceLevel = 0;
	m_bLastWasKeyword = false;
	m_atticFormat = false;
}

IE_Exp_RTF::IE_Exp_RTF(PD_Document * pDocument,bool atticFormat)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListenerWriteDoc = NULL;
	m_pListenerGetProps = NULL;
	m_bNeedUnicodeText = false;
	m_braceLevel = 0;
	m_bLastWasKeyword = false;
	m_atticFormat = atticFormat;
}

IE_Exp_RTF::~IE_Exp_RTF()
{
	UT_VECTOR_FREEALL(char *,m_vecColors);
	UT_VECTOR_PURGEALL(_rtf_font_info *,m_vecFonts);
	_clearStyles();
}

/*****************************************************************/
/*****************************************************************/

bool IE_Exp_RTF_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".rtf"));
}

UT_Error IE_Exp_RTF_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_RTF * p = new IE_Exp_RTF(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_RTF_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "Rich Text Format (.rtf)";
	*pszSuffixList = "*.rtf";
	*ft = getFileType();
	return true;
}

#if !defined(DEBUG) || defined(NDEBUG)

// Dom: hack - i hate doing this, but it will shut thousands of people up

bool IE_Exp_MsWord_97_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".doc"));
}

UT_Error IE_Exp_MsWord_97_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_RTF * p = new IE_Exp_RTF(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_MsWord_97_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "Microsoft Word (.doc)";
	*pszSuffixList = "*.doc";
	*ft = getFileType();
	return true;
}

#endif // word97 hack

/*for attic*/

bool IE_Exp_RTF_attic_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".rtf"));
}

UT_Error IE_Exp_RTF_attic_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_RTF * p = new IE_Exp_RTF(pDocument,1);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_RTF_attic_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "Rich Text Format for old apps (.rtf)";
	*pszSuffixList = "*.rtf";
	*ft = getFileType();
	return true;
}
	  
/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_RTF::_writeDocument(void)
{
	// The overall syntax for an RTF file is:
	//
	// <file> := '{' <header> <document> '}'
	//
	// We are responsible for everything except for <document>;
	// We let _ListenerWriteDoc() deal with that.
	//
	// create and install a listener to sniff over the
	// document and find things that we need to write
	// into the rtf header.  this includes the font table
	// and the color table.

	_addColor("000000");				// load black as color 0.
	_addColor("ffffff");                // load white as color 1.
		
	m_pListenerGetProps = new s_RTF_ListenerGetProps(getDoc(),this);
	if (!m_pListenerGetProps)
		return UT_IE_NOMEMORY;
	if (getDocRange())
		getDoc()->tellListenerSubset(static_cast<PL_Listener *>(m_pListenerGetProps),getDocRange());
	else
		getDoc()->tellListener(static_cast<PL_Listener *>(m_pListenerGetProps));
	DELETEP(m_pListenerGetProps);

	// Important: This must come before the header is written so
        // every font used in a style is properly entered in the font table.
	_selectStyles();

	// write rtf header

	if (!_write_rtf_header())
		return UT_IE_COULDNOTWRITE;
	
	// create and install a listener to receive the document
	// and write its content in rtf.
	
	m_pListenerWriteDoc = new s_RTF_ListenerWriteDoc(getDoc(),this, (getDocRange()!=NULL));
	if (!m_pListenerWriteDoc)
		return UT_IE_NOMEMORY;
	if (getDocRange())
		getDoc()->tellListenerSubset(static_cast<PL_Listener *>(m_pListenerWriteDoc),getDocRange());
	else
		getDoc()->tellListener(static_cast<PL_Listener *>(m_pListenerWriteDoc));
	DELETEP(m_pListenerWriteDoc);

	// write any rtf trailer matter

	if (!_write_rtf_trailer())
		return UT_IE_COULDNOTWRITE;
	
	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}

/*****************************************************************/
/*****************************************************************/
#if 0

void s_RTF_Listener::_handleDataItems(void)
{
	bool bWroteOpenDataSection = false;

	const char * szName;
	const UT_ByteBuf * pByteBuf;

	UT_ByteBuf bb64(1024);

	for (UT_uint32 k=0; (m_pDocument->enumDataItems(k,NULL,&szName,&pByteBuf,NULL)); k++)
	{
		if (!bWroteOpenDataSection)
		{
			m_pie->write("<data>\n");
			bWroteOpenDataSection = true;
		}

		if (UT_Base64Encode(&bb64, pByteBuf))
		{
			m_pie->write("<d name=\"");
			m_pie->write(szName);
			m_pie->write("\">\n");

			// break up the Base64 blob as a series lines
			// like MIME does.

			UT_uint32 jLimit = bb64.getLength();
			UT_uint32 jSize;
			UT_uint32 j;
			for (j=0; j<jLimit; j+=72)
			{
				jSize = MyMin(72,(jLimit-j));
				m_pie->write((const char *)bb64.getPointer(j),jSize);
				m_pie->write("\n");
			}
			m_pie->write("</d>\n");
		}
	}

	if (bWroteOpenDataSection)
		m_pie->write("</data>\n");

	return;
}
#endif
/*****************************************************************/
/*****************************************************************/

UT_sint32 IE_Exp_RTF::_findColor(const char * szColor) const
{
	if (!szColor || !*szColor)
		return 0;						// black
	
	UT_uint32 k;
	UT_uint32 kLimit = m_vecColors.getItemCount();

	for (k=0; k<kLimit; k++)
	{
		const char * sz = (const char *)m_vecColors.getNthItem(k);
		if (UT_stricmp(sz,szColor) == 0)
			return k;
	}

	return -1;
}

void IE_Exp_RTF::_addColor(const char * szColor)
{
	UT_ASSERT(szColor && *szColor && (_findColor(szColor)==-1));

	char * sz = NULL;
	UT_cloneString(sz,szColor);
	if (sz)
		m_vecColors.addItem(sz);
	return;
}

UT_sint32 IE_Exp_RTF::_findOrAddColor(const char * szColor)
{
	UT_sint32 ndx = _findColor (szColor);

	if (ndx != -1)
		return ndx;

	_addColor (szColor);
	return _findColor (szColor);
}

/*****************************************************************/
/*****************************************************************/

void IE_Exp_RTF::_rtf_open_brace(void)
{
	m_braceLevel++;
	write("{");
	m_bLastWasKeyword = false;
}

void IE_Exp_RTF::_rtf_close_brace(void)
{
	m_braceLevel--;
	write("}");
	m_bLastWasKeyword = false;

	UT_ASSERT(m_braceLevel >= 0);
}

void IE_Exp_RTF::_rtf_keyword(const char * szKey)
{
	write("\\");
	write(szKey);
	m_bLastWasKeyword = true;
}

/* output a non-ascii char in the RTF stream. */
void IE_Exp_RTF::_rtf_nonascii_hex2 (UT_sint32 d)
{
	write("\\'");
        char buf[100];
	sprintf(buf,"%02x",d);
	write(buf);
	m_bLastWasKeyword = false;
}


void IE_Exp_RTF::_rtf_keyword(const char * szKey, UT_sint32 d)
{
	write("\\");
	write(szKey);
	char buf[100];
	sprintf(buf,"%d",d);
	write(buf);
	m_bLastWasKeyword = true;
}

void IE_Exp_RTF::_rtf_keyword(const char * szKey, const char * val)
{
  write("\\");
  write(szKey);
  write(val);
  m_bLastWasKeyword = true;
}

void IE_Exp_RTF::_rtf_keyword_hex2(const char * szKey, UT_sint32 d)
{
	write("\\");
	write(szKey);
	char buf[100];
	sprintf(buf,"%02x",d);
	write(buf);
	m_bLastWasKeyword = true;
}

void IE_Exp_RTF::_rtf_keyword_ifnotdefault(const char * szKey, const char * szValue, UT_sint32 defaultValue)
{
	if (!szValue || !*szValue)
		return;

	UT_sint32 d = atol(szValue);
	if (d == defaultValue)
		return;

	write("\\");
	write(szKey);
	char buf[100];
	sprintf(buf,"%d",d);
	write(buf);
	m_bLastWasKeyword = true;
}

void IE_Exp_RTF::_rtf_keyword_ifnotdefault_twips(const char * szKey, const char * szValue, UT_sint32 defaultValue)
{
	if (!szValue || !*szValue)
		return;

	// convert dimensioned value into twips (twentieths of a point) (aka 720 twips/inch)
	double dbl = UT_convertToPoints(szValue);
	UT_sint32 d = (UT_sint32)(dbl * 20.0);
	
	if (d == defaultValue)
		return;

	write("\\");
	write(szKey);
	char buf[100];
	sprintf(buf,"%d",d);
	write(buf);
	m_bLastWasKeyword = true;
}

void IE_Exp_RTF::_rtf_semi(void)
{
	write(";");
	m_bLastWasKeyword = false;
}

void IE_Exp_RTF::_rtf_fontname(const char * szFontName)
{
	write(" ");
#if 0 
	/*we handle 'helvetica' in a special way on import - so it's safe
	 to output "Helvetica" unconditionally.
	*/	
	if (!m_atticFormat && 0)
	    write(szFontName);
	else
#endif	
	{
		/*  map "Helvetic" to "Helvetica", since on Windows
		    font "Helvetic" contains only Latin1 chars, while 
		    "Helvetica" contains all needed chars. This is innocient
		    since we do this when attic format is requested.
		*/
		if (UT_stricmp(szFontName,"helvetic")==0)
			write("Helvetica");
		else
			write(szFontName);
	}
	_rtf_semi();
}

void IE_Exp_RTF::_rtf_chardata(const char * pbuf, UT_uint32 buflen)
{
	if (m_bLastWasKeyword)
		write(" ");
	if(0 == buflen)
		return;
	write(pbuf,buflen);
	m_bLastWasKeyword = false;
}

void IE_Exp_RTF::_rtf_nl(void)
{
	write("\n");
}

bool IE_Exp_RTF::_write_rtf_header(void)
{
	UT_uint32 k,kLimit;

	UT_uint32 langcode = XAP_EncodingManager::get_instance()->getWinLanguageCode();
	// write <rtf-header>
	// return false on error
	UT_DEBUGMSG(("Belcon:in  IE_Exp_RTF::_write_rtf_header,langcode=%d\n",langcode));

	_rtf_open_brace();
	_rtf_keyword("rtf",1);				// major version number of spec version 1.5
	
	_rtf_keyword("ansi");
	bool wrote_cpg = 0;
	if (langcode) 
	{
		char* cpgname = wvLIDToCodePageConverter(langcode);
		UT_DEBUGMSG(("Belcon,after wvLIDToCodePageConverter(%d),cpgname=%s\n",langcode,cpgname));
		if (UT_strnicmp(cpgname,"cp",2)==0 && UT_UCS_isdigit(cpgname[2])) 
		{
			int cpg;
			if (sscanf(cpgname+2,"%d",&cpg)==1) 
			{
				_rtf_keyword("ansicpg",cpg);
				wrote_cpg = 1;
			}
		}
		// NOTE:we will get "GB2312","BIG5" or something else after we call
		// NOTE:wvLIDToCodePageConverter,we can deal with them quite fine,
		// NOTE:but we need convert them to CPxx.:-(
		else
		{
			const char* codepage=XAP_EncodingManager::get_instance()->CodepageFromCharset(cpgname);
			if(UT_strnicmp(codepage,"cp",2)==0 && UT_UCS_isdigit(codepage[2]))
			{
				int cpg;
				if (sscanf(codepage+2,"%d",&cpg)==1)
				{
					_rtf_keyword("ansicpg",cpg);
					wrote_cpg = 1;
				}
			}
			UT_DEBUGMSG(("Belcon:after XAP_EncodingManager::get_instance()->CodepageFromCharset(%s),codepage=%s\n",cpgname,codepage));
		}
	};
	if (!wrote_cpg)
	    _rtf_keyword("ansicpg",1252);		// TODO what CodePage do we want here ??

	_rtf_keyword("deff",0);				// default font is index 0 aka black
	if (m_atticFormat)
	{	
		/* I'm not sure whether this makes any sense - VH */
		if (langcode)
			_rtf_keyword("deflang",langcode);
	}

	// write the "font table"....

	kLimit = m_vecFonts.getItemCount();
	// don't write a font table group if we don't have any font to write.
	// see bug 1383
	if (kLimit > 0)
	{
		_rtf_nl();
		_rtf_open_brace();
		_rtf_keyword("fonttbl");
		UT_uint32 charsetcode = XAP_EncodingManager::get_instance()->getWinCharsetCode();
		for (k=0; k<kLimit; k++)
		{
			const _rtf_font_info * pk = (const _rtf_font_info *)m_vecFonts.getNthItem(k);
			_rtf_nl();
			_rtf_open_brace();
			_rtf_keyword("f", k);								// font index number
			_rtf_keyword(pk->szFamily);								// {\fnil,\froman,\fswiss,...}
			_rtf_keyword("fcharset",pk->nCharset);
			_rtf_keyword("fprq",pk->nPitch);							// {0==default,1==fixed,2==variable}
			_rtf_keyword((pk->fTrueType) ? "fttruetype" : "ftnil");	// {\fttruetype,\ftnil}
			
			// we do nothing with or use default values for
			// \falt \panose \fname \fbias \ftnil \fttruetype \fontfile
			
			// after we write the various generic font properties, we write
			// the actual font name and a semicolon -- i couldn't see this
			// described in the specification, but it was in other RTF files
			// that i saw and really seems to help Word and WordPad....
			_rtf_fontname(pk->szName);
			
			_rtf_close_brace();
		}
		_rtf_close_brace();
	}
	
	// TODO write the "file table" if necessary...

	kLimit = m_vecColors.getItemCount();
	if (kLimit > 0)
	{
		_rtf_nl();
		_rtf_open_brace();
		_rtf_keyword("colortbl");
		for (k=0; k<kLimit; k++)
		{
			const char * szColor = (const char *)m_vecColors.getNthItem(k);
			UT_RGBColor localColor;
			UT_parseColor(szColor,localColor);
			_rtf_nl();
			_rtf_keyword("red",  localColor.m_red);
			_rtf_keyword("green",localColor.m_grn);
			_rtf_keyword("blue", localColor.m_blu);
			_rtf_semi();
		}
		_rtf_close_brace();
	}

	_write_stylesheets();
	// TODO write the "list table"...
	// TODO write the "rev table"...

	// write default character properties at global scope...
	_rtf_nl();
	_rtf_keyword("kerning",0);			// turn off kerning
	_rtf_keyword("cf",0);				// set color 0 -- black
	
	return (m_error == 0);
}

/*!
 * Write an rtf keyword if the given property isn't the default
 * value. Use this only with twips-valued properties.
 *
 * !param pStyle       A style.
 * !param szPropName   The property to check.
 * !param szRTFName    The RTF keyword to use if the property
 *                     doesn't have the default value.
 */
void IE_Exp_RTF::_write_prop_ifnotdefault(const PD_Style * pStyle, 
					  const XML_Char * szPropName, 
					  const char * szRTFName)
{
    const XML_Char * sz = NULL;
    if (pStyle->getProperty((const XML_Char *)szPropName, sz)) {
	_rtf_keyword_ifnotdefault_twips(szRTFName, sz, 0);
    }
}

/*!
 * Write an RTF keyword if the given property is "yes".
 */
void IE_Exp_RTF::_write_prop_ifyes(const PD_Style * pStyle, 
				   const XML_Char * szPropName, 
				   const char * szRTFName)
{
    const XML_Char * sz = NULL;
    if (pStyle->getProperty((const XML_Char *)szPropName, sz) && UT_strcmp(sz, "yes") == 0) {
	    _rtf_keyword(szRTFName);
    }
}

/*
 * Used to hold tab information by _write_tabdef.
 */
class _t 
{
public:
	_t(const char * szTL, const char * szTT, const char * szTK, UT_sint32 tp)
		{
			m_szTabLeaderKeyword = szTL;
			m_szTabTypeKeyword = szTT;
			m_szTabKindKeyword = szTK;
			m_iTabPosition = tp;
		}
	const char *    m_szTabLeaderKeyword;
	const char *	m_szTabTypeKeyword;
	const char *	m_szTabKindKeyword;
	UT_sint32		m_iTabPosition;
};

static int compare_tabs(const void* p1, const void* p2)
{
	_t ** ppTab1 = (_t **) p1;
	_t ** ppTab2 = (_t **) p2;

	if ((*ppTab1)->m_iTabPosition < (*ppTab2)->m_iTabPosition)
		return -1;
	if ((*ppTab1)->m_iTabPosition > (*ppTab2)->m_iTabPosition)
		return 1;
	return 0;
}

/*!
 * Write out the <tabdef> paragraph formatting.
 */
void IE_Exp_RTF::_write_tabdef(const char * szTabStops)
{
	if (szTabStops && *szTabStops)
	{
		// write tabstops for this paragraph
		// TODO the following parser was copied from abi/src/text/fmt/xp/fl_BlockLayout.cpp
		// TODO we should extract both of them and share the code.

		UT_Vector vecTabs;
		
		const char* pStart = szTabStops;
		while (*pStart)
		{
			const char * szTT = "tx";	// TabType -- assume text tab (use "tb" for bar tab)
			const char * szTK = NULL;	// TabKind -- assume left tab
			const char * szTL = NULL;    // TabLeader
			const char* pEnd = pStart;
			while (*pEnd && (*pEnd != ','))
				pEnd++;
			const char* p1 = pStart;
			while ((p1 < pEnd) && (*p1 != '/'))
				p1++;
			if ( (p1 == pEnd) || ((p1+1) == pEnd) )
				;						// left-tab is default
			else
			{
				switch (p1[1])
				{
				default:
				case 'L': 	szTK = NULL; 	break;
				case 'R':	szTK = "tqr";	break;
				case 'C':	szTK = "tqc";	break;
				case 'D':	szTK = "tqdec";	break;
				case 'B':	szTT = "tb";    szTK= NULL;	break; // TabKind == bar tab
				}
				switch (p1[2])
				{
				default:
				case '0': szTL = NULL;      break;
				case '1': szTL = "tldot";   break;
				case '2': szTL = "tlhyph";    break;
				case '3': szTL = "tlul";    break;
				case '4': szTL = "tleq";    break;
				}
			}

			char pszPosition[32];
			UT_uint32 iPosLen = p1 - pStart;
			UT_ASSERT(iPosLen < 32);
			UT_uint32 k;
			for (k=0; k<iPosLen; k++)
				pszPosition[k] = pStart[k];
			pszPosition[k] = 0;
			// convert position into twips
			double dbl = UT_convertToPoints(pszPosition);
			UT_sint32 d = (UT_sint32)(dbl * 20.0);
			
			_t * p_t = new _t(szTL,szTT,szTK,d);
			vecTabs.addItem(p_t);

			pStart = pEnd;
			if (*pStart)
			{
				pStart++;	// skip past delimiter
				while (*pStart == UCS_SPACE)
					pStart++;
			}
		}

		// write each tab in order:
		// <tabdef> ::= ( <tab> | <bartab> )+
		// <tab>    ::= <tabkind>? <tablead>? \tx
		// <bartab> ::= <tablead>? \tb

		vecTabs.qsort(compare_tabs);

		UT_uint32 k;
		UT_uint32 kLimit = vecTabs.getItemCount();
		for (k=0; k<kLimit; k++)
		{
			_t * p_t = (_t *)vecTabs.getNthItem(k);
			// write <tabkind>
			if (p_t->m_szTabKindKeyword && *p_t->m_szTabKindKeyword)
				_rtf_keyword(p_t->m_szTabKindKeyword);
			if (p_t->m_szTabLeaderKeyword && *p_t->m_szTabLeaderKeyword)
				_rtf_keyword(p_t->m_szTabLeaderKeyword);
			_rtf_keyword(p_t->m_szTabTypeKeyword,p_t->m_iTabPosition);

			delete p_t;
		}
	}
}

/*!
 * Write out the <charfmt> paragraph or character formatting. This
 * does not print opening and closing braces.
 */
void IE_Exp_RTF::_write_charfmt(const s_RTF_AttrPropAdapter & apa)
{
	const XML_Char * szColor = apa.getProperty("color");
	UT_sint32 ndxColor = _findColor((char*)szColor);
	UT_ASSERT(ndxColor != -1);

	if (ndxColor != 0) // black text, the default
		_rtf_keyword("cf",ndxColor);

	szColor = apa.getProperty("bgcolor");

	if (szColor && UT_stricmp (szColor, "transparent") != 0)
	{
		ndxColor = _findColor((char*)szColor);
		UT_ASSERT(ndxColor != -1);
		if (ndxColor != 1) // white background, the default
		{
			_rtf_keyword("cb",ndxColor);
		}
	}

	UT_sint32 ndxFont = _findFont(&apa);
	if(ndxFont != -1)
		_rtf_keyword("f",ndxFont);	// font index in fonttbl

	const XML_Char * szFontSize = apa.getProperty("font-size");
	double dbl = UT_convertToPoints(szFontSize);
	UT_sint32 d = (UT_sint32)(dbl*2.0);

	// if (d != 24) - always write this out
	if(szFontSize != NULL)
	{
		if(d == 0)
			d = 24;
		_rtf_keyword("fs",d);	// font size in half points
	}
	const XML_Char * szFontStyle = apa.getProperty("font-style");
	if (szFontStyle && *szFontStyle && (UT_strcmp(szFontStyle,"italic")==0))
		_rtf_keyword("i");

	const XML_Char * szFontWeight = apa.getProperty("font-weight");
	if (szFontWeight && *szFontWeight && (UT_strcmp(szFontWeight,"bold")==0))
		_rtf_keyword("b");

	const XML_Char * szFontDecoration = apa.getProperty("text-decoration");
	if (szFontDecoration && *szFontDecoration)
	{
		if (strstr(szFontDecoration,"underline") != 0)
			_rtf_keyword("ul");
		if (strstr(szFontDecoration,"overline") != 0)
			_rtf_keyword("ol");
		if (strstr(szFontDecoration,"line-through") != 0)
			_rtf_keyword("strike");
		if (strstr(szFontDecoration,"topline") != 0)
		{
			_rtf_keyword("*");
			_rtf_keyword("topline");
		}
		if (strstr(szFontDecoration,"bottomline") != 0)
		{
			_rtf_keyword("*");
			_rtf_keyword("botline");
		}
	}

	const XML_Char * szFontPosition = apa.getProperty("text-position");
	if (szFontPosition && *szFontPosition)
	{
		if (!UT_strcmp(szFontPosition,"superscript"))
			_rtf_keyword("super");
		else if (!UT_strcmp(szFontPosition,"subscript"))
			_rtf_keyword("sub");
	}

	// export the language of the run of text
	const XML_Char * szLang = apa.getProperty("lang");
	if ( szLang )
	  {
	    UT_DEBUGMSG(("DOM: lang,lid = %s,%d\n", szLang, wvLangToLIDConverter(szLang)));
	    _rtf_keyword("lang", wvLangToLIDConverter(szLang)); 
	  }

#ifdef BIDI_ENABLED

	//###TF const XML_Char * szDir = apa.getProperty("dir");
	const XML_Char * szDirOvrr = apa.getProperty("dir-override");
	
	bool bProceed = true;
	if (szDirOvrr)
	{
		if (!UT_strcmp (szDirOvrr, "ltr"))
		{
			_rtf_keyword ("ltrch");
			bProceed = false;
		}
		else if (!UT_strcmp (szDirOvrr, "rtl"))
		{
			_rtf_keyword ("rtlch");
			bProceed  = false;
		}
	}
	/*
	if (bProceed || szDir)
	{
		if (!UT_strcmp (szDir, "ltr"))
			_rtf_keyword ("ltrch");
		else if (!UT_strcmp (szDir, "rtl"))
			_rtf_keyword ("rtlch");
	}  */

#endif
	const XML_Char * szListTag = apa.getProperty("list-tag");
	if (szListTag && *szListTag)
	{
		_rtf_open_brace();
		_rtf_keyword("*");
		UT_uint32 id = atoi(szListTag);
		_rtf_keyword("listtag",id);
		_rtf_close_brace();
	}


	// TODO do something with our font-stretch and font-variant properties
	// note: we assume that kerning has been turned off at global scope.

}

/*!
 * Write out the formatting group for one style in the RTF header.
 */
void IE_Exp_RTF::_write_style_fmt(const PD_Style * pStyle)
{
    // brdrdef: not implemented because AbiWord does not have borders
    // at time of this writing.

    // parfmt
    _write_prop_ifyes(pStyle, "keep-together", "keep");
    _write_prop_ifyes(pStyle, "keep-with-next", "keepn");

    const XML_Char * sz = NULL;
    if (pStyle->getProperty((const XML_Char *)"text-align", sz)) 
	{
		if (UT_strcmp(sz, "left") == 0) 
		{
			// Default, so no need to print anything
		} 
		else if (UT_strcmp(sz, "right") == 0) 
		{
			_rtf_keyword("qr");
		} 
		else if (UT_strcmp(sz, "center") == 0) 
		{
			_rtf_keyword("qc");
		}
		else if (UT_strcmp(sz, "justify") == 0) 
		{
			_rtf_keyword("qj");
		} 
		else 
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
    }

    const XML_Char * szLineHeight = NULL;
    if (pStyle->getProperty((const XML_Char *) "line-height", szLineHeight)
	&& strcmp(szLineHeight,"1.0") != 0) 
	{
		double f = UT_convertDimensionless(szLineHeight);
		if (f != 0.0) 
		{
			UT_sint32 dSpacing = (UT_sint32)(f * 240.0);
			_rtf_keyword("sl",dSpacing);
			_rtf_keyword("slmult",1);
		}
    }

    _write_prop_ifnotdefault(pStyle, "text-indent", "fi");
    _write_prop_ifnotdefault(pStyle, "margin-left", "li");
    _write_prop_ifnotdefault(pStyle, "margin-right", "ri");
    _write_prop_ifnotdefault(pStyle, "margin-top", "sb");
    _write_prop_ifnotdefault(pStyle, "margin-bottom", "sa");

    // apoctl

    // tabdef
    if (pStyle->getProperty((const XML_Char *) "tabstops", sz)) _write_tabdef(sz);
    

    // shading

    // chrfmt
    _write_charfmt(s_RTF_AttrPropAdapter_Style(pStyle));
}

/*!
 * This is just a pair: style and style number. It is used for
 * two-way association of PD_Style and RTF style number.
 */
struct NumberedStyle 
{
    const PD_Style * pStyle;
    UT_uint32 n;

    NumberedStyle(const PD_Style * pStyle, UT_uint32 n) : 
	pStyle(pStyle), n(n) {}
};

/*!
 * Clear the style hash.
 */
void IE_Exp_RTF::_clearStyles()
{
    UT_HASH_PURGEDATA(NumberedStyle *, &m_hashStyles, delete);
}

/*!
 * Select styles for export. This inserts all styles to be exported
 * into the style hash. Also, it makes sure that all fonts used in
 * styles are present in the font table.
 */
void IE_Exp_RTF::_selectStyles()
{
    _clearStyles();

    UT_uint32 i;
    UT_uint32 nStyleNumber = 0;
    const char * szName;
    const PD_Style * pStyle;
	UT_Vector vecStyles;
	getDoc()->getAllUsedStyles(&vecStyles);
    for (i = 0; getDoc()->enumStyles(i, &szName, &pStyle); ++i) 
	{
		if (pStyle->isUserDefined() || (vecStyles.findItem((void *) pStyle) >= 0)) 
		{
//
// Add this style to the hash
//
			NumberedStyle * pns = (NumberedStyle *) m_hashStyles.pick(szName);
			if(pns == NULL)
			{
				m_hashStyles.insert(szName, new NumberedStyle(pStyle, ++nStyleNumber));
				UT_TRY
				{
					_rtf_font_info fi(static_cast<s_RTF_AttrPropAdapter_Style>(pStyle));
					if (_findFont(&fi) == -1)
						_addFont(&fi);
				}
				UT_CATCH(_rtf_no_font e)
				{
				}
				UT_END_CATCH
			}
		}
    }
}


/*!
 * Return the style number that was assigned to the given style.
 * The style must be present in the style hash.
 */
UT_uint32 IE_Exp_RTF::_getStyleNumber(const PD_Style * pStyle)
{
    return _getStyleNumber(pStyle->getName());
}

/*!
 * Return the style number that was assigned to the named style.
 * The style must be present in the style hash.
 */
UT_uint32 IE_Exp_RTF::_getStyleNumber(const XML_Char * szStyle)
{
    const NumberedStyle * pns = reinterpret_cast<const NumberedStyle *>(m_hashStyles.pick(szStyle));
    UT_ASSERT(pns != NULL);
    return pns->n;
}

/*!
 * Write the stylesheets group of the RTF header. Only styles that
 * are used by the document are written.
 */
void IE_Exp_RTF::_write_stylesheets(void)
{
    if (getDoc()->getStyleCount() == 0) return;

    _rtf_nl();
    _rtf_open_brace();
    _rtf_keyword("stylesheet");

    UT_StringPtrMap::UT_Cursor hc(&m_hashStyles);
    const NumberedStyle * pns;
    for (pns = reinterpret_cast<const NumberedStyle *>(hc.first()); 
		 hc.is_valid(); 
		 pns = reinterpret_cast<const NumberedStyle *>(hc.next())) 
	{
		const PD_Style * pStyle = pns->pStyle;
		_rtf_nl();
		_rtf_open_brace();
		
		if (pStyle->isCharStyle()) 
		{
			_rtf_keyword("*");
			_rtf_keyword("cs", pns->n);
		} 
		else 
		{
			_rtf_keyword("s", pns->n);
		}
		
		_write_style_fmt(pStyle);
		
		const PD_Style * pStyleBasedOn =  reinterpret_cast<const PD_Style *> (const_cast<PD_Style *>(pStyle)->getBasedOn());
		// TODO: Can this really return NULL?
		if (pStyleBasedOn != NULL) 
		{
			_rtf_keyword("sbasedon", _getStyleNumber(pStyleBasedOn));
		}
		
		const PD_Style * pStyleNext = reinterpret_cast<const PD_Style *> (const_cast<PD_Style *>(pStyle)->getFollowedBy());
		// TODO: Can this really return NULL?
		if (pStyleNext != NULL) 
		{
			_rtf_keyword("snext", _getStyleNumber(pStyleNext));
		}
	
		_rtf_chardata(pStyle->getName(), strlen(pStyle->getName()));
		_rtf_chardata(";",1);
		_rtf_close_brace();
    }

    _rtf_close_brace();
}

bool IE_Exp_RTF::_write_rtf_trailer(void)
{
	while (m_braceLevel>0)
		_rtf_close_brace();
	return (m_error == 0);
}

/*!
 * Find a font in the font table. Return the index of the font. If
 * it is not found, return -1.
 */
UT_sint32 IE_Exp_RTF::_findFont(const _rtf_font_info * pfi) const
{
	UT_ASSERT(pfi);

	UT_uint32 k;
	UT_uint32 kLimit = m_vecFonts.getItemCount();

	for (k=0; k<kLimit; k++)
	{
		const _rtf_font_info * pk = (const _rtf_font_info *)m_vecFonts.getNthItem(k);
		if (pk->_is_same(*pfi))
			return k;
	}

	return -1;
}

UT_sint32 IE_Exp_RTF::_findFont(const s_RTF_AttrPropAdapter * apa) const
{
	static UT_sint32 ifont = 0;
	UT_TRY
	  {
	    _rtf_font_info fi(*apa);
	    ifont = _findFont(&fi);
	    return ifont;
	  }
	UT_CATCH(_rtf_no_font e)
	  {
	    return -1;
	  }
	UT_END_CATCH
}

/*!
 * Add a font to the font table. The font must not be present
 * in the font table at the start of the call.
 */
void IE_Exp_RTF::_addFont(const _rtf_font_info * pfi)
{
	UT_ASSERT(pfi && (_findFont(pfi)==-1));
	_rtf_font_info * pNew = new _rtf_font_info(*pfi);
	if (pNew)
		m_vecFonts.addItem(pNew);

	return;
}

_rtf_font_info::_rtf_font_info(const s_RTF_AttrPropAdapter & apa) 
	UT_THROWS((_rtf_no_font))
{
    // Not a typo. The AbiWord "font-family" property is what RTF
    // calls font name. It has values like "Courier New".
    szName = apa.getProperty("font-family");
    if (szName == NULL)
      {
	_rtf_no_font e;
	UT_THROW(e);
      }    

    static const char * t_ff[] = { "fnil", "froman", "fswiss", "fmodern", "fscript", "fdecor", "ftech", "fbidi" };
    GR_Font::FontFamilyEnum ff;
    GR_Font::FontPitchEnum fp;
    bool tt;
    GR_Font::s_getGenericFontProperties((char*)szName, &ff, &fp, &tt);

    if ((ff >= 0) && (ff < (int)NrElements(t_ff)))
	szFamily = t_ff[ff];
    else
	szFamily = t_ff[GR_Font::FF_Unknown];
    nCharset = XAP_EncodingManager::get_instance()->getWinCharsetCode();
    nPitch = fp;
    fTrueType = tt;
}

/*!
 * True if the two objects represent the same RTF font.
 */
bool _rtf_font_info::_is_same(const _rtf_font_info & fi) const
{
	bool bMatchFontFamily = false;
	bool bMatchFontName = true;
	if(szFamily && *szFamily && fi.szFamily && *fi.szFamily)
	{
		bMatchFontFamily =  UT_strcmp(szFamily, fi.szFamily) == 0;
	}
	else if ( szFamily == fi.szFamily) // Both null pointers
	{
		bMatchFontFamily = true;
	}
	else if (  szFamily && fi.szFamily && *szFamily == *fi.szFamily) // Both pointer to NULLs
	{
		bMatchFontFamily = true;
	}
	if(szName && *szName && fi.szName && *fi.szName)
	{
		bMatchFontName =  UT_strcmp(szName, fi.szName) == 0;
	}
	else if ( szName == fi.szName) // Both null pointers
	{
		bMatchFontName = true;
	}
	else if ( szName && fi.szName && *szName == *fi.szName) // Both pointer to NULLs
	{
		bMatchFontName = true;
	}

    return bMatchFontFamily
	&& nCharset == fi.nCharset
	&& nPitch == fi.nPitch
	&& bMatchFontName
	&& fTrueType == fi.fTrueType;
}



