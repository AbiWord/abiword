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
#include "ut_rand.h"

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

IE_Exp_RTF_Sniffer::IE_Exp_RTF_Sniffer ()
	: IE_ExpSniffer(IE_IMPEXPNAME_RTF)
{
	// 
}

UT_Confidence_t IE_Exp_RTF_Sniffer::supportsMIME (const char * szMIME)
{
	if (UT_strcmp (szMIME, IE_MIME_RTF) == 0)
		{
			return UT_CONFIDENCE_GOOD;
		}
	return UT_CONFIDENCE_ZILCH;
}

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

/*for attic*/

IE_Exp_RTF_attic_Sniffer::IE_Exp_RTF_attic_Sniffer ()
	: IE_ExpSniffer(IE_IMPEXPNAME_RTFATTIC)
{
	// 
}

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

IE_Exp_MsWord_Hack_Sniffer::IE_Exp_MsWord_Hack_Sniffer ()
	: IE_ExpSniffer(IE_IMPEXPNAME_RTFMSDOC)
{
	// 
}

bool IE_Exp_MsWord_Hack_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".doc"));
}

UT_Error IE_Exp_MsWord_Hack_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_RTF * p = new IE_Exp_RTF(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_MsWord_Hack_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "Microsoft Word (.doc)";
	*pszSuffixList = "*.doc";
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

/*!
 * This method search for the requested header/footer section within
 * PD_DOCument and writes into the stream at the current write point.
 \params pszHdrFtr constchar * string describing the type of header/footer to
                                export.
 \params pszHdrFtrID const char * identification string for the header/footer
 */
void IE_Exp_RTF::exportHdrFtr(const char * pszHdrFtr , const char * pszHdrFtrID)
{

// First find the header/footer section and id in the document.
	m_pListenerWriteDoc->_closeSpan();
	m_pListenerWriteDoc->_closeBlock();
	m_pListenerWriteDoc->_closeSpan();
	m_pListenerWriteDoc->_closeSection();
	m_pListenerWriteDoc->_setTabEaten(false);

	PL_StruxDocHandle hdrSDH = getDoc()->findHdrFtrStrux((const XML_Char *) pszHdrFtr,(const XML_Char * ) pszHdrFtrID);

	if(hdrSDH == NULL)
	{
	  UT_ASSERT_NOT_REACHED();
		return;
	}
	PT_DocPosition posStart = getDoc()->getStruxPosition(hdrSDH);
	PT_DocPosition posEnd = 0;
	PL_StruxDocHandle nextSDH = NULL;
	bool found = getDoc()->getNextStruxOfType(hdrSDH,PTX_SectionHdrFtr ,&nextSDH);

	if(!found || (nextSDH == NULL ))
	{
		getDoc()->getBounds(true, posEnd);
	}
	else
	{
		posEnd =  getDoc()->getStruxPosition(nextSDH);
	}
	posStart++;
	PD_DocumentRange * pExportHdrFtr = new PD_DocumentRange(getDoc(),posStart,posEnd);
//
// Got everything. Now write out an openning brace and HdrFtr type.
//
	if(m_pListenerWriteDoc->m_bStartedList)
	{
		_rtf_close_brace();
	}
	_rtf_nl();
	_rtf_open_brace();
	if(strcmp(pszHdrFtr,"header") == 0)
	{
		_rtf_keyword("header");
	}
	else if(strcmp(pszHdrFtr,"footer") == 0)
	{
		_rtf_keyword("footer");
	}
	else if(strcmp(pszHdrFtr,"header-even") == 0)
	{
		_rtf_keyword("headerl");
	}
	else if(strcmp(pszHdrFtr,"header-first") == 0)
	{
		_rtf_keyword("headerf");
	}
	else if(strcmp(pszHdrFtr,"footer-even") == 0)
	{
		_rtf_keyword("footerl");
	}
	else if(strcmp(pszHdrFtr,"footer-first") == 0)
	{
		_rtf_keyword("footerf");
	}
	_rtf_keyword("pard");
	_rtf_keyword("plain");
	m_pListenerWriteDoc->m_bBlankLine = false;
	m_pListenerWriteDoc->m_bStartedList = false;
	UT_DEBUGMSG(("SEVIOR: Doing header \n"));
//
// Now pump out the contents of the HdrFtr
//
	getDoc()->tellListenerSubset(static_cast<PL_Listener *>(m_pListenerWriteDoc),pExportHdrFtr);
	_rtf_keyword("par");
	delete pExportHdrFtr;
	_rtf_close_brace();
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
	UT_return_if_fail(szColor && *szColor && (_findColor(szColor)==-1));

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

	UT_return_if_fail(m_braceLevel >= 0);
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
	write(UT_String_sprintf("%02x",d));
	m_bLastWasKeyword = false;
}

/* write a non-ascii char into a string class pointer */
void IE_Exp_RTF::_rtf_nonascii_hex2 (UT_sint32 d, UT_String & pStr)
{
	pStr = "\\'";
	pStr += UT_String_sprintf("%02x",d);
}


void IE_Exp_RTF::_rtf_keyword(const char * szKey, UT_sint32 d)
{
	write("\\");
	write(szKey);
	write(UT_String_sprintf("%d",d));
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
	write(UT_String_sprintf("%02x",d));
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
	write(UT_String_sprintf("%d",d));
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
	write(UT_String_sprintf("%d",d));
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
	{
		write(" ");
		m_bLastWasKeyword = false;
	}

	if(0 == buflen)
		return;
	write(pbuf,buflen);
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
		const char* cpgname = wvLIDToCodePageConverter(langcode);
		UT_DEBUGMSG(("Belcon,after wvLIDToCodePageConverter(%d),cpgname=%s\n",langcode,cpgname));
		if (UT_strnicmp(cpgname,"cp",2)==0 && UT_UCS4_isdigit(cpgname[2]))
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
			const char* codepage=XAP_EncodingManager::get_instance()->CodepageFromCharset(const_cast<char*>(cpgname));
			if(UT_strnicmp(codepage,"cp",2)==0 && UT_UCS4_isdigit(codepage[2]))
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
		/*UT_uint32 charsetcode =*/ XAP_EncodingManager::get_instance()->getWinCharsetCode();
		for (k=0; k<kLimit; k++)
		{
			const _rtf_font_info * pk = (const _rtf_font_info *)m_vecFonts.getNthItem(k);
			_rtf_nl();
			_rtf_open_brace();
			_rtf_keyword("f", k);								// font index number
			_rtf_keyword(pk->getFontFamily());								// {\fnil,\froman,\fswiss,...}
			_rtf_keyword("fcharset",pk->getFontCharset());
			_rtf_keyword("fprq",pk->getFontPitch());							// {0==default,1==fixed,2==variable}
			_rtf_keyword((pk->isTrueType()) ? "fttruetype" : "ftnil");	// {\fttruetype,\ftnil}

			// we do nothing with or use default values for
			// \falt \panose \fname \fbias \ftnil \fttruetype \fontfile

			// after we write the various generic font properties, we write
			// the actual font name and a semicolon -- i couldn't see this
			// described in the specification, but it was in other RTF files
			// that i saw and really seems to help Word and WordPad....
			_rtf_fontname(pk->getFontName());

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
	_write_listtable();

	// TODO write the "rev table"...

	// write default character properties at global scope...
	_rtf_nl();
	_rtf_keyword("kerning",0);			// turn off kerning
	_rtf_keyword("cf",0);				// set color 0 -- black
	_rtf_keyword("ftnbj");				// set footnotes to bottom of page
	_rtf_keyword("fet",2);				// Allow both footnotes and endnotes
	_rtf_keyword("ftnstart",1);			// First footnote is one - later use
	                                    // document properties

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
			UT_return_if_fail(iPosLen < 32);
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
		UT_ASSERT_HARMLESS(ndxColor != -1);
		if (ndxColor != 1) // white background, the default
		{
			_rtf_keyword("cb",ndxColor);
			_rtf_keyword("highlight",ndxColor);
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
			_rtf_keyword("abitopline"); // abiword extension
		}
		if (strstr(szFontDecoration,"bottomline") != 0)
		{
			_rtf_keyword("abibotline"); // abiword extension
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
		  UT_ASSERT_NOT_REACHED();
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

#ifdef _MSC_VER	// MSVC++ warns about 'e' : unreferenced local variable
#pragma warning(disable: 4101)
#endif


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
	  // DOM: hack for 2069 - we'll export all styles instead of just
	  // user-defined styles and used styles. To fix it (and export fewer
	  // styles in general) make this routine recursive to include
	  // parent (basedon) styles as well...
	  if (true /* pStyle->isUserDefined() || (vecStyles.findItem((void *) pStyle) >= 0)*/)
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
//
// Now do it for the field font as well
//
				UT_TRY
				{
					_rtf_font_info fi(static_cast<s_RTF_AttrPropAdapter_Style>(pStyle),true);
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
	if(UT_XML_strcmp(szStyle, "Normal Clean")== 0)
	{
		szStyle = "Normal";
	}
	NumberedStyle * pns = (NumberedStyle*)m_hashStyles.pick(szStyle);
	UT_ASSERT_HARMLESS(pns);
	if(pns != NULL )
	{
		return pns->n;
	}
	else
	{
		pns = (NumberedStyle*)m_hashStyles.pick("Normal");
		return pns->n;
	}
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

/*!
 * Write the listatble group of the RTF header.
 */
void IE_Exp_RTF::_write_listtable(void)
{
	UT_uint32 iCount = getDoc()->getListsCount();
    if (iCount == 0) return;
//
// Openning RTF comments for the listtable
//
    _rtf_nl();
    _rtf_open_brace();
    _rtf_keyword("*");
    _rtf_keyword("listtable");
//
// OK scan the lists in the document to build up the list info.
// The first loop just builds a vector of parentless lists.
//
	UT_uint32 i,j,k =0;
	bool bFoundChild = false;
	fl_AutoNum * pAuto = NULL;
	fl_AutoNum * pInner = NULL;
	ie_exp_RTF_MsWord97ListMulti * pList97 = NULL;
	for(i=0; i< iCount; i++)
	{
		pAuto = getDoc()->getNthList(i);
		if(pAuto->getParent() == NULL)
		{
			bFoundChild = false;
			for(j =0; (j< iCount) && !bFoundChild; j++)
			{
				pInner = getDoc()->getNthList(j);
				if(pInner->getParentID() == pAuto->getID())
//
// Found a child of pList97, it must be a multi-level list.
//
				{
					UT_DEBUGMSG(("SEVIOR: Adding %x to multi-level \n",pAuto));
					m_vecMultiLevel.addItem((void *) new ie_exp_RTF_MsWord97ListMulti(pAuto));
					bFoundChild = true;
					break;
				}
			}
			if(!bFoundChild)
			{
				UT_DEBUGMSG(("SEVIOR: Adding %x to simple \n",pAuto));
				m_vecSimpleList.addItem((void *) new ie_exp_RTF_MsWord97ListSimple(pAuto));
			}
		}
	}
//
// OK now fill the MultiLevel list structure.
//

	for(k=0; k < m_vecMultiLevel.getItemCount(); k++)
	{
		pList97 = (ie_exp_RTF_MsWord97ListMulti *) m_vecMultiLevel.getNthItem(k);
//
// For each level in the list RTF97 structure find the first matching
// List.
//
// Start at level 1 (level 0 is the base)
//
		UT_uint32 depth=0;
		bool bFoundAtPrevLevel = true;
		for(depth = 1; depth < 10; depth++)
		{
//
// Now loop through all the lists in the document and find a list whose parent
// is the same as the list one level lower.
//
			if(!bFoundAtPrevLevel)
			{
				ie_exp_RTF_MsWord97List * pCur97 = new ie_exp_RTF_MsWord97List(pList97->getAuto());
				UT_DEBUGMSG(("SEVIOR: Adding NULL level at depth %d \n",depth));
				pList97->addLevel(depth, pCur97);
			}
			else
			{
				bFoundAtPrevLevel = false;
				for(i=0; i < iCount; i++)
				{
					pAuto = (fl_AutoNum *) getDoc()->getNthList(i);
					pInner = pAuto->getParent();
					fl_AutoNum * pAutoLevel = pList97->getListAtLevel(depth-1,0)->getAuto();
//
// OK got it! pAuto is the one we want.
//
					if(pInner != NULL && pInner == pAutoLevel)
					{
						bFoundAtPrevLevel = true;
						ie_exp_RTF_MsWord97List * pCur97 = new ie_exp_RTF_MsWord97List(pAuto);
						UT_DEBUGMSG(("SEVIOR: Adding level %x at depth %d \n",pCur97,depth));
						pList97->addLevel(depth, pCur97);
					}
				}
			}
			if(!bFoundAtPrevLevel)
			{
				ie_exp_RTF_MsWord97List * pCur97 = new ie_exp_RTF_MsWord97List(pList97->getAuto());
				UT_DEBUGMSG(("SEVIOR: Adding NULL level at depth %d \n",depth));
				pList97->addLevel(depth, pCur97);
			}

		}
	}
//
// OK we got the simple and multi-list structures full.
// Now fill the override structure.
	for(i=0; i< iCount; i++)
	{
		pAuto = getDoc()->getNthList(i);
		ie_exp_RTF_ListOveride * pOver = new ie_exp_RTF_ListOveride(pAuto);
		pOver->setOverideID(i+1);
		m_vecOverides.addItem((void *) pOver);
	}
//
// OK that's everything. Now generate the RTF Header.
//
// MultiLevel lists
//
	for(i=0; i< m_vecMultiLevel.getItemCount(); i++)
	{
		_rtf_nl();
		_output_MultiLevelRTF(getNthMultiLevel(i));
	}
//
// Simple Lists
//
	for(i=0; i< m_vecSimpleList.getItemCount(); i++)
	{
		_rtf_nl();
		_output_SimpleListRTF(getNthSimple(i));
	}
//
// \*\listtable is done now!
//
    _rtf_close_brace();
//
// Overides. Start with the \*\listoverridetable keyword.
//
    _rtf_nl();
    _rtf_open_brace();
    _rtf_keyword("*");
    _rtf_keyword("listoverridetable");
	for(i=0; i< m_vecOverides.getItemCount(); i++)
	{
		_rtf_nl();
		_output_OveridesRTF(getNthOveride(i),i);
	}
//
// Finished!
//
    _rtf_close_brace();
	_rtf_nl();
}

/*!
 * Get ith Multilevel list
 */
ie_exp_RTF_MsWord97ListMulti * IE_Exp_RTF::getNthMultiLevel(UT_uint32 i) const
{
	return (ie_exp_RTF_MsWord97ListMulti *) m_vecMultiLevel.getNthItem(i);
}

/*!
 * Get ith Simple list
 */
ie_exp_RTF_MsWord97ListSimple *  IE_Exp_RTF::getNthSimple(UT_uint32 i) const
{
	return (ie_exp_RTF_MsWord97ListSimple *) m_vecSimpleList.getNthItem(i);
}

/*!
 * Get ith Overide
 */
ie_exp_RTF_ListOveride *  IE_Exp_RTF::getNthOveride(UT_uint32 i) const
{
	return (ie_exp_RTF_ListOveride *) m_vecOverides.getNthItem(i);
}

/*!
 * Get Number of multilevel lists in the document
 */
UT_uint32  IE_Exp_RTF::getMultiLevelCount(void) const
{
	return m_vecMultiLevel.getItemCount();
}

/*!
 * Get Number of simple lists in the document
 */
UT_uint32  IE_Exp_RTF::getSimpleListCount(void) const
{
	return m_vecSimpleList.getItemCount();
}
/*!
 * Get Number of overides in the document
 */
UT_uint32  IE_Exp_RTF::getOverideCount(void) const
{
	return m_vecOverides.getItemCount();
}

/*!
 * Return the the number of the overide that matches the given ID.
 * Returns 0 on failure to find a matching ID.
 */
UT_uint32 IE_Exp_RTF::getMatchingOverideNum(UT_uint32 ID)
{
	UT_uint32 baseid = ID;
	UT_uint32 i=0;
	for(i=0; i< getOverideCount(); i++)
	{
		ie_exp_RTF_ListOveride * pOver = getNthOveride(i);
		if(pOver->doesOverideMatch(baseid))
		{
			return pOver->getOverideID();
		}
	}
	return 0;
}

/*!
 * Actually output the RTF from a multi-level list
 \params ie_exp_RTF_MsWord97ListMulti * pMulti pointer to a multi-level list
 * structure.
 */
void IE_Exp_RTF::_output_MultiLevelRTF(ie_exp_RTF_MsWord97ListMulti * pMulti)
{
	_rtf_open_brace();
	_rtf_keyword("list");
	UT_uint32 tempID = UT_rand();
	while(tempID < 10000)
	{
		tempID = UT_rand();
	}
	_rtf_keyword("listtemplateid",tempID);
	UT_uint32 i = 0;
	fl_AutoNum * pAuto = NULL;
	ie_exp_RTF_MsWord97List * pList97 = NULL;
	for(i=0; i < 9 ; i++)
	{
		_rtf_open_brace();
		_rtf_keyword("listlevel");
		pList97 = pMulti->getListAtLevel(i,0);
		if(pList97 != NULL)
		{
//
// Strategy: Dump out all the list info for the first list in each level. Then
// use the overides to redefine subsequent lists at each level.
//
			pAuto = pList97->getAuto();
			if(i==0 && pAuto->getParent() != NULL)
			{
				UT_ASSERT_NOT_REACHED();
			}
			_output_ListRTF(pAuto,i);
		}
		else
		{
			_output_ListRTF(NULL,i);
		}
		_rtf_close_brace();
	}
	_rtf_keyword("listid",pMulti->getID());
	_rtf_close_brace();
}


/*!
 * This method outputs the RTF defintion of the list pointed to by pAuto
 */
void IE_Exp_RTF::_output_LevelText(fl_AutoNum * pAuto, UT_uint32 iLevel, UT_UCSChar bulletsym)
{
	UT_String LevelText;
	UT_String LevelNumbers;
	UT_uint32 lenText;
	UT_uint32 ifoundLevel=iLevel;
//
// Level Text and Level Numbers
//
	_rtf_open_brace();
	_rtf_keyword("leveltext");
	if(bulletsym == 0)
	{
		_generate_level_Text(pAuto,LevelText,LevelNumbers,lenText,ifoundLevel);
		UT_String tmp;
		_rtf_nonascii_hex2(lenText,tmp);
		tmp += LevelText;
		tmp += ";";
		UT_DEBUGMSG(("SEVIOR: Final level text string is %s \n",tmp.c_str()));
		write(tmp.c_str());
		_rtf_close_brace();
		_rtf_open_brace();
		_rtf_keyword("levelnumbers");
		write(LevelNumbers.c_str());
		write(";");
	}
	else
	{
		_rtf_nonascii_hex2(1);
		_rtf_nonascii_hex2((UT_sint32) bulletsym);
		write(" ");
		write(";");
		_rtf_close_brace();
		_rtf_open_brace();
		_rtf_keyword("levelnumbers");
		write(";");
	}
	_rtf_close_brace();
}

/*!
 * This method generates the leveltext and levelnumber strings.HOWEVER it does
 * not generate the leading text string which is the length of the string. It
 * is the responsibility of the calling routine to this.
 */
void IE_Exp_RTF::_generate_level_Text(fl_AutoNum * pAuto,UT_String & LevelText,UT_String &LevelNumbers, UT_uint32 & lenText, UT_uint32 & ifoundLevel)
{
	UT_DEBUGMSG(("SEVIOR: pAuto %x \n",pAuto));
	if(pAuto)
	{
		UT_DEBUGMSG(("SEVIOR: pAuto-getParent() %x \n",pAuto->getParent()));
	}
	if(pAuto && (pAuto->getParent() == NULL))
	{
		UT_String LeftSide = pAuto->getDelim();
		UT_String RightSide;
		_get_LeftRight_Side(LeftSide,RightSide);
		UT_DEBUGMSG(("SEVIOR: Top - leftside = %s rightside = %s \n",LeftSide.c_str(),RightSide.c_str()));
		UT_String place;
		UT_uint32 locPlace = (UT_uint32) LeftSide.size();
		_rtf_nonascii_hex2(locPlace+1,place);
		LevelNumbers = place;
		ifoundLevel = 1;
		LevelText.clear();
		if(LeftSide.size() > 0)
		{
			LevelText = LeftSide;
		}
		place.clear();
		_rtf_nonascii_hex2(ifoundLevel-1,place);
		LevelText += place;
		if(RightSide.size() > 0)
		{
			LevelText += RightSide;
		}
		lenText = LeftSide.size() + RightSide.size() + 1;
		UT_DEBUGMSG(("SEVIOR: Level %d LevelText %s  \n",ifoundLevel,LevelText.c_str()));
		return;
	}
	else if((pAuto != NULL) && ( pAuto->getParent() != NULL))
	{
		_generate_level_Text(pAuto->getParent(),LevelText,LevelNumbers,lenText,
							 ifoundLevel);
		UT_String LeftSide = pAuto->getDelim();
		UT_String RightSide;
		_get_LeftRight_Side(LeftSide,RightSide);
		UT_String str;
//
// FIXME. Implement this when level decimal works

		if(pAuto->getParent()->getDecimal() && *(pAuto->getParent()->getDecimal()))
		{
			if(RightSide.size() > 0)
			{
				UT_DEBUGMSG(("SEVIOR: RightSide =%s last char = %c \n",RightSide.c_str(),RightSide[RightSide.size()-1]));
			}
			if(RightSide.size()== 0)
			{
				RightSide += pAuto->getParent()->getDecimal();
			}
			else if(RightSide[RightSide.size()-1] != '.')
			{
				RightSide += pAuto->getParent()->getDecimal();
			}
		}
		ifoundLevel++;
		UT_uint32 locPlace = lenText + LeftSide.size();
		str.clear();
		_rtf_nonascii_hex2(locPlace+1,str);
		LevelNumbers += str;
		lenText = lenText + LeftSide.size() + RightSide.size() + 1;
		str.clear();
		_rtf_nonascii_hex2(ifoundLevel-1,str);
		LevelText += LeftSide;
		LevelText += str;
		LevelText += RightSide;
		UT_DEBUGMSG(("SEVIOR: Level %d LevelText %s  \n",ifoundLevel,LevelText.c_str()));
		return;
	}
	else
	{
		UT_uint32 i=0;
		lenText = 0;
		LevelText.clear();
		LevelNumbers.clear();
		UT_String str;
		for(i=0; i<= ifoundLevel; i++)
		{
			str.clear();
			_rtf_nonascii_hex2(i,str);
			LevelText += str;
			str.clear();
			_rtf_nonascii_hex2(lenText+1,str);
			LevelNumbers += str;
			if(i<ifoundLevel)
			{
				LevelText += ".";
				lenText += 2;
			}
			else
			{
				lenText += 1;
			}
		}
	}
	return;
}


/*!
 * This method splits the abiword List delim string into text to the left
 * and right of the "%L" marker. The input string is in LeftSide.
 */
void IE_Exp_RTF::_get_LeftRight_Side(UT_String & LeftSide, UT_String & RightSide)
{
	const char * psz = strstr(LeftSide.c_str(),"%L");
	UT_DEBUGMSG(("SEVIOR: Substring = %s Total is %s \n",psz,LeftSide.c_str()));
	if(psz != NULL)
	{
		UT_uint32 index = (UT_uint32) (psz - LeftSide.c_str());
		UT_uint32 len = (UT_uint32) strlen(LeftSide.c_str());
		UT_DEBUGMSG(("SEVIOR: index = %d len =%d \n",index,len));
		if(index+2 < len)
		{
			RightSide = LeftSide.substr(index+2,len);
		}
		else
		{
			RightSide.clear();
		}
		if(index > 0)
		{
			LeftSide = LeftSide.substr(0,index);
		}
		else
		{
			LeftSide.clear();
		}
	}
	else
	{
		RightSide.clear();
	}
}

/*!
 * This method outputs the RTF defintion of the list pointed to by pAuto
 */
void IE_Exp_RTF::_output_ListRTF(fl_AutoNum * pAuto, UT_uint32 iLevel)
{
// List Type
	UT_sint32 Param = 0;
	UT_String fontName;
	UT_UCSChar bulletsym=0;
	List_Type lType = NUMBERED_LIST;
	if(pAuto != NULL)
	{
		lType = pAuto->getType();
	}
	else
	{
		lType = NUMBERED_LIST;
	}
	switch(lType)
	{
	default:
	case NUMBERED_LIST:
		Param = 0;
		break;
	case UPPERROMAN_LIST:
		Param = 1;
		break;
	case LOWERROMAN_LIST:
		Param = 2;
		break;
	case UPPERCASE_LIST:
		Param = 3;
		break;
	case HEBREW_LIST:
		Param = 45;
		break;
	case LOWERCASE_LIST:
		Param = 4;
		break;
	case BULLETED_LIST:
		Param = 23;
		bulletsym = 0xb7;
		fontName = "Symbol";
		break;
	case DASHED_LIST:
		Param = 23;
		bulletsym = '-';
		fontName = "Times New Roman";
		break;
	case SQUARE_LIST:
		Param = 23;
		bulletsym = 0x6E;
		fontName = "Dingbats";
		break;
	case TRIANGLE_LIST:
		Param = 23;
		bulletsym = 0x73;
		fontName = "Dingbats";
		break;
	case DIAMOND_LIST:
		Param = 23;
		bulletsym = 0xA9;
		fontName = "Dingbats";
		break;
	case STAR_LIST:
		Param = 23;
		bulletsym = 0x53;
		fontName = "Dingbats";
		break;
	case IMPLIES_LIST:
		Param = 23;
		bulletsym = 0xDE;
		fontName = "Dingbats";
		break;
	case TICK_LIST:
		Param = 23;
		bulletsym = 0x33;
		fontName = "Dingbats";
		break;
	case BOX_LIST:
		Param = 23;
		bulletsym = 0x72;
		fontName = "Dingbats";
		break;
	case HAND_LIST:
		Param = 23;
		bulletsym = 0x2B;
		fontName = "Dingbats";
		break;
	case HEART_LIST:
		Param = 23;
		bulletsym = 0xAA;
		fontName = "Dingbats";
		break;
	}
	_rtf_keyword("levelnfc",Param);
	UT_sint32 startParam = 0;
	if(pAuto)
	{
		startParam  = pAuto->getStartValue32();
	}
	else
	{
		startParam = 1;
	}
	_rtf_keyword("levelstartat",startParam);
	_rtf_keyword("levelspace",0);
	_rtf_keyword("levelfollow",0);
	if(pAuto == NULL)
	{
		float marg = LIST_DEFAULT_INDENT;
		float indent =  (float)LIST_DEFAULT_INDENT_LABEL;
		UT_String smarg;
		UT_String sindent;
		marg = (((float) iLevel) +1.0f) * marg;
		UT_String_sprintf(smarg,"%fin",marg);
		UT_String_sprintf(sindent,"%fin",indent);
		_rtf_keyword_ifnotdefault_twips("li",(char*)smarg.c_str(),0);
		_rtf_keyword_ifnotdefault_twips("fi",(char*)sindent.c_str(),0);
	}
//
// Output the indents and alignments. Use the first sdh to get these.
//
	else
	{
		PL_StruxDocHandle sdh = pAuto->getFirstItem();
		const char * szIndent = NULL;
		const char * szAlign = NULL;
		if(sdh != NULL)
		{
			bool bres = getDoc()->getPropertyFromSDH(sdh,"text-indent",&szIndent);
			if(bres)
			{
				_rtf_keyword_ifnotdefault_twips("fi",(char*)szIndent,0);
			}
			bres = getDoc()->getPropertyFromSDH(sdh,"margin-left",&szAlign);
			if(bres)
			{
				_rtf_keyword_ifnotdefault_twips("li",(char*)szAlign,0);
			}
		}
	}

// Leveltext and levelnumbers
//
	_output_LevelText(pAuto,iLevel,bulletsym);
//
// Export the bullet font
//
	if(Param == 23)
	{
		_rtf_font_info fi(fontName.c_str());
		UT_sint32 ifont = _findFont(&fi);
		if(ifont < 0)
		{
		  UT_ASSERT_NOT_REACHED();
			ifont = 0;
		}
		_rtf_keyword("f",ifont);
	}
}

/*!
 * Actually output the RTF from a Simple list
 \params ie_exp_RTF_MsWord97ListSimple * pSimple pointer to a Simple list
 * structure.
 */
void IE_Exp_RTF::_output_SimpleListRTF(ie_exp_RTF_MsWord97ListSimple * pSimple)
{
	_rtf_open_brace();
	_rtf_keyword("list");
	UT_uint32 tempID = UT_rand();
	while(tempID < 10000)
	{
		tempID = UT_rand();
	}
	_rtf_keyword("listtemplateid",tempID);
	_rtf_keyword("listsimple");
	fl_AutoNum * pAuto = pSimple->getAuto();
	_rtf_open_brace();
	_rtf_keyword("listlevel");
//
// Strategy: Dump out all the list info for the first list in each level.
//
	_output_ListRTF(pAuto,0);
	_rtf_close_brace();
	_rtf_keyword("listid",pSimple->getID());
	_rtf_close_brace();
}

/*!
 * Actually output the RTF from an Overide
 \params ie_exp_RTF_Overide * pOver pointer to an Overide definition
 */
void IE_Exp_RTF::_output_OveridesRTF(ie_exp_RTF_ListOveride * pOver, UT_uint32 iOver)
{
	_rtf_open_brace();
	_rtf_keyword("listoverride");
	_rtf_keyword("listoverridecount",0);
	fl_AutoNum * pAuto = pOver->getAutoNum();
	fl_AutoNum * pTop = pAuto;
	while(pTop->getParent())
	{
		pTop = pTop->getParent();
	}
	_rtf_keyword("listid",pTop->getID());
//
// Strategy: Dump out all the list info for the first list in each level.
//
	_output_ListRTF(pAuto,0);
	_rtf_keyword("ls",pOver->getOverideID());
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
	UT_return_val_if_fail(pfi, -1);

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
	UT_return_if_fail(pfi && (_findFont(pfi)==-1));
	_rtf_font_info * pNew = new _rtf_font_info(*pfi);
	if (pNew)
		m_vecFonts.addItem(pNew);

	return;
}

_rtf_font_info::_rtf_font_info(const s_RTF_AttrPropAdapter & apa, bool bDoFieldFont)
	UT_THROWS((_rtf_no_font))
{
    // Not a typo. The AbiWord "font-family" property is what RTF
    // calls font name. It has values like "Courier New".
	const char * szName = NULL;
	if(!bDoFieldFont)
	{
		szName = apa.getProperty("font-family");
		if(szName != NULL)
		{
			m_szName = szName;
		}
	}
	else
	{
		szName = apa.getProperty("field-font");
		if(szName != NULL)
		{
			m_szName = szName;
		}
	}
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


_rtf_font_info::~_rtf_font_info(void)
{
}

_rtf_font_info::_rtf_font_info(const char * szFontName)
	UT_THROWS((_rtf_no_font))
{
    // Not a typo. The AbiWord "font-family" property is what RTF
    // calls font name. It has values like "Courier New".
    if (szFontName == NULL)
	{
		_rtf_no_font e;
		UT_THROW(e);
	}

	m_szName = szFontName;

    static const char * t_ff[] = { "fnil", "froman", "fswiss", "fmodern", "fscript", "fdecor", "ftech", "fbidi" };
    GR_Font::FontFamilyEnum ff;
    GR_Font::FontPitchEnum fp;
    bool tt;
    GR_Font::s_getGenericFontProperties(m_szName.c_str(), &ff, &fp, &tt);

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
	if((m_szName.size() > 0) && (fi.m_szName.size() >0))
	{
		bMatchFontName =  UT_strcmp(m_szName.c_str(), fi.m_szName.c_str()) == 0;
	}
	else if ( m_szName.size()  == fi.m_szName.size()) // Both null pointers
	{
		bMatchFontName = true;
	}
    return bMatchFontFamily
	&& nCharset == fi.nCharset
	&& nPitch == fi.nPitch
	&& bMatchFontName
	&& fTrueType == fi.fTrueType;
}

ie_exp_RTF_MsWord97List::ie_exp_RTF_MsWord97List(fl_AutoNum * pAuto)
{
	m_pAutoNum = pAuto;
	m_Id = pAuto->getID();
}

ie_exp_RTF_MsWord97List::~ie_exp_RTF_MsWord97List(void)
{
}

ie_exp_RTF_MsWord97ListSimple::ie_exp_RTF_MsWord97ListSimple(fl_AutoNum * pAuto)
	: ie_exp_RTF_MsWord97List( pAuto)
{
}


ie_exp_RTF_MsWord97ListSimple::~ie_exp_RTF_MsWord97ListSimple(void)
{
}


ie_exp_RTF_MsWord97ListMulti::ie_exp_RTF_MsWord97ListMulti(fl_AutoNum * pAuto)
	: ie_exp_RTF_MsWord97List( pAuto)
{
	UT_uint32 i = 0;
	for(i=0; i < 9 ; i++)
	{
		m_vLevels[i] = NULL;
	}
	addLevel(0, (ie_exp_RTF_MsWord97List *) this);
}


ie_exp_RTF_MsWord97ListMulti::~ie_exp_RTF_MsWord97ListMulti(void)
{
	UT_uint32 i = 0;
	delete m_vLevels[0];
	for(i=1; i < 9 ; i++)
	{
		if(m_vLevels[i] != NULL)
		{
			UT_Vector * pV = m_vLevels[i];
			UT_VECTOR_PURGEALL(ie_exp_RTF_MsWord97List *, (*pV));
			delete pV;
			m_vLevels[i]  = NULL;
		}
	}
}

/*!
 * Add a list to a level
\params iLevel to add the list too
\params ie_exp_RTF_MsWord97List * pList97 list to added at this level
 */
void ie_exp_RTF_MsWord97ListMulti::addLevel(UT_uint32 iLevel, ie_exp_RTF_MsWord97List * pList97)
{
	if(iLevel > 8)
	{
		iLevel = 8;
	}
	if(m_vLevels[iLevel] == NULL)
	{
		UT_Vector * pVecList97 = new UT_Vector;
		pVecList97->addItem((void *) pList97);
		m_vLevels[iLevel] = pVecList97;
		pVecList97->addItem((void *) pList97);
	}
	else
	{
		m_vLevels[iLevel]->addItem((void *) pList97);
	}
}

/*!
 * Return the nthList List at level iLevel
 \params iLevel the level which we want to lists for
 \params nthList the list at the level we want
 */
ie_exp_RTF_MsWord97List * ie_exp_RTF_MsWord97ListMulti::getListAtLevel(UT_uint32 iLevel, UT_uint32 nthList)
{
	if(iLevel > 8)
	{
		iLevel = 8;
	}
	if(m_vLevels[iLevel] == NULL)
	{
		return NULL;
	}
	UT_uint32 icount = m_vLevels[iLevel]->getItemCount();
	if(icount > nthList)
	{
		ie_exp_RTF_MsWord97List * pList97 = (ie_exp_RTF_MsWord97List * ) m_vLevels[iLevel]->getNthItem(nthList);
		return pList97;
	}
	else
	{
		return NULL;
	}
}


/*!
 * Return the listID of the first list element at the level that contains
 * the list that matches listID
 \params listID the listID we're looking for.
 \retval the List ID number of the first list on the level that contains
 the ID. Return 0 if there is no match in the structure.
 */
UT_uint32 ie_exp_RTF_MsWord97ListMulti::getMatchingID(UT_uint32 listID)
{
	UT_uint32 i,j;
	ie_exp_RTF_MsWord97List * pList97 = NULL;
	bool bFound = false;
	UT_uint32 foundID = 0;
	UT_uint32 firstID = 0;
	for(i=0; (i < 8) && !bFound; i++)
	{
		for(j=0; m_vLevels[i] && (j < m_vLevels[i]->getItemCount()) && !bFound; j++)
		{
			pList97 = (ie_exp_RTF_MsWord97List *) m_vLevels[i]->getNthItem(j);
			if(j==0)
			{
				firstID = pList97->getID();
			}
			bFound = pList97->getID() == listID;
			if(bFound)
			{
				foundID = firstID;
			}
		}
	}
	return foundID;
}

ie_exp_RTF_ListOveride::ie_exp_RTF_ListOveride(fl_AutoNum * pAuto)
{
	m_pAutoNum = pAuto;
	m_AbiListID = pAuto->getID();
}


ie_exp_RTF_ListOveride::~ie_exp_RTF_ListOveride(void)
{
}







