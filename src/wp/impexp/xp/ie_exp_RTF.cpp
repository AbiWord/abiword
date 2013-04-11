/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <string>
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "ut_vector.h"
#include "ut_endian.h"
#include "pt_Types.h"
#include "ie_impexp_RTF.h"
#include "ie_exp_RTF.h"
#include "pd_Document.h"
#include "pd_DocumentRDF.h"
#include "pd_RDFSupport.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "pp_Revision.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "pd_Style.h"
#include "gr_Graphics.h"
#include "ut_rand.h"
#include "ut_std_string.h"
#include "pl_ListenerCoupleCloser.h"

#include "wv.h" //for wvLIDToCodePageConverter
#include "xap_EncodingManager.h"
#include "ut_debugmsg.h"

#include <sstream>

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
	m_bNeedUnicodeText = false;
	m_braceLevel = 0;
	m_bLastWasKeyword = false;
	m_atticFormat = false;
	m_CharRTL = UT_BIDI_UNSET;
	m_conv = UT_iconv_open("UCS-4", "utf-8");
}

IE_Exp_RTF::IE_Exp_RTF(PD_Document * pDocument,bool atticFormat)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListenerWriteDoc = NULL;
	m_bNeedUnicodeText = false;
	m_braceLevel = 0;
	m_bLastWasKeyword = false;
	m_atticFormat = atticFormat;
	m_CharRTL = UT_BIDI_UNSET;
	m_conv = UT_iconv_open("UCS-4", "utf-8");
}

IE_Exp_RTF::~IE_Exp_RTF()
{
	UT_VECTOR_FREEALL(char *,m_vecColors);
	UT_VECTOR_PURGEALL(_rtf_font_info *,m_vecFonts);
	_clearStyles();
	if (UT_iconv_isValid(m_conv))
	{
		UT_iconv_close(m_conv);
	}
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_RTF_Sniffer::IE_Exp_RTF_Sniffer ()
	: IE_ExpSniffer(IE_IMPEXPNAME_RTF, true)
{
	// 
}

UT_Confidence_t IE_Exp_RTF_Sniffer::supportsMIME (const char * szMIME)
{
	if (strcmp (szMIME, IE_MIMETYPE_RTF) == 0)
		{
			return UT_CONFIDENCE_PERFECT;
		}
	return UT_CONFIDENCE_ZILCH;
}

bool IE_Exp_RTF_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!g_ascii_strcasecmp(szSuffix,".rtf"));
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
	: IE_ExpSniffer(IE_IMPEXPNAME_RTFATTIC, false)
{
	// 
}

bool IE_Exp_RTF_attic_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!g_ascii_strcasecmp(szSuffix,".rtf"));
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
	return (!g_ascii_strcasecmp(szSuffix,".doc"));
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
	return _writeDocumentLocal(false);
}

UT_Error IE_Exp_RTF::_writeDocumentLocal(bool bSkipHeader)
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

	s_RTF_ListenerGetProps * listenerGetProps = new s_RTF_ListenerGetProps(getDoc(),this);
	if (!listenerGetProps)
		return UT_IE_NOMEMORY;
	if (getDocRange() && !bSkipHeader)
		getDoc()->tellListenerSubset(listenerGetProps,getDocRange());
	else
		getDoc()->tellListener(listenerGetProps);

	// if the bit to be pasted contains a new block anywhere within it,
	// then we also want the block props for the first block in the
	// clipboard.  mark this info down here, use it in the listener.
	bool hasBlock = listenerGetProps->hasBlock();

	DELETEP(listenerGetProps);

	// Important: This must come before the header is written so
        // every font used in a style is properly entered in the font table.
	_selectStyles();

	// write rtf header
	if(!bSkipHeader)
	{
		if (!_write_rtf_header())
			return UT_IE_COULDNOTWRITE;
	}
	// create and install a listener to receive the document
	// and write its content in rtf.

	m_pListenerWriteDoc = new s_RTF_ListenerWriteDoc(getDoc(),this, (getDocRange()!=NULL), hasBlock);
	if (!m_pListenerWriteDoc)
		return UT_IE_NOMEMORY;
    PL_ListenerCoupleCloser* pCloser = new PL_ListenerCoupleCloser();
	if (getDocRange())
		getDoc()->tellListenerSubset(m_pListenerWriteDoc,getDocRange(),pCloser);
	else
		getDoc()->tellListener(m_pListenerWriteDoc);
    DELETEP(pCloser);
	DELETEP(m_pListenerWriteDoc);

	// write any rtf trailer matter

	if(!bSkipHeader)
	{
			if (!_write_rtf_trailer())
				return UT_IE_COULDNOTWRITE;
	}
	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}



/*!
 * This method search for the requested header/footer section within
 * PD_DOCument and writes into the stream at the current write point.
 \param pszHdrFtr constchar * string describing the type of header/footer to
                                export.
 \param pszHdrFtrID const char * identification string for the header/footer
 */
void IE_Exp_RTF::exportHdrFtr(const char * pszHdrFtr , const char * pszHdrFtrID, const char * pszKeyWord)
{

// First find the header/footer section and id in the document.
	m_pListenerWriteDoc->_closeSpan();
#if 0 //#TF
	m_pListenerWriteDoc->_closeBlock();
	m_pListenerWriteDoc->_closeSpan();
	m_pListenerWriteDoc->_closeSection();
#endif
	m_pListenerWriteDoc->_setTabEaten(false);

	pf_Frag_Strux* hdrSDH = getDoc()->findHdrFtrStrux((const gchar *) pszHdrFtr,(const gchar * ) pszHdrFtrID);

	if(hdrSDH == NULL)
	{
	  UT_ASSERT_NOT_REACHED();
		return;
	}
	PT_DocPosition posStart = getDoc()->getStruxPosition(hdrSDH);
	PT_DocPosition posEnd = 0;
	pf_Frag_Strux* nextSDH = NULL;
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
	_rtf_keyword(pszKeyWord);
	_rtf_keyword("pard");
	_rtf_keyword("plain");
	m_pListenerWriteDoc->m_bBlankLine = true;
	m_pListenerWriteDoc->m_bStartedList = false;
	UT_DEBUGMSG(("SEVIOR: Doing header \n"));
//
// Now pump out the contents of the HdrFtr
//
	getDoc()->tellListenerSubset(static_cast<PL_Listener *>(m_pListenerWriteDoc),pExportHdrFtr);
#if 0 //#TF
	_rtf_keyword("par");
#endif
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
				jSize = UT_MIN(72,(jLimit-j));
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
		if (g_ascii_strcasecmp(sz,szColor) == 0)
			return k;
	}

	return -1;
}

void IE_Exp_RTF::_addColor(const char * szColor)
{
	UT_return_if_fail(szColor && *szColor && (_findColor(szColor)==-1));

	char * sz = g_strdup(szColor);
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


void IE_Exp_RTF::_rtf_keyword_space(const char * szKey, UT_sint32 d)
{
	write("\\");
	write(szKey);
	write(UT_String_sprintf(" %d",d));
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
	/*  map "Helvetic" to "Helvetica", since on Windows
	    font "Helvetic" contains only Latin1 chars, while
	    "Helvetica" contains all needed chars.
	    This is safe to do for both attic and non-attic format
	    because we handle "helvetica" in a special way on import.
	*/
	if (g_ascii_strcasecmp(szFontName,"helvetic")==0)
		write("Helvetica");
	else
	{
		_rtf_pcdata(szFontName, true);
	}

	_rtf_semi();
	return;
}


void IE_Exp_RTF::_rtf_chardata(const std::string& buf)
{
    _rtf_chardata( buf.c_str(), buf.length() );
}

/*
 * I'm increasingly suspicious of this function. It seems to be
 * being used to output PCDATA (like stylesheet names). For 
 * such a use it is certainly broken. Does it have any genuine use?
 *
 * It's broken because it converts the input data to UCS-4 and 
 * then writes out any characters between 0x80 and 0xff as hex escaped
 * sequences. However, it takes no account of the rtf files encoding
 * set by the \ansicpg command in the header
 * - R.Kay (Aug '05)
 *
 */
void IE_Exp_RTF::_rtf_chardata(const char * pbuf, UT_uint32 buflen)
{
	const char * current = pbuf;
	UT_uint32 count = 0;

	xxx_UT_DEBUGMSG(("Buffer length = %d \n",buflen));
	if(buflen < 400)
	{ 
		xxx_UT_DEBUGMSG(("data = %s\n", pbuf));
	}
	if (m_bLastWasKeyword)
	{
		write(" ");
		m_bLastWasKeyword = false;
	}

	if(0 == buflen) {
		return;
	}

	UT_return_if_fail (UT_iconv_isValid(m_conv));
	while (count < buflen) {
		if (*current & 0x80) {  // check for non-ASCII value
			UT_UCS4Char wc;
			size_t insz, sz;
			char * dest = (char*)(&wc);
			insz = buflen - count;
			sz = sizeof(wc);
			UT_iconv(m_conv, &current, &insz, &dest, &sz);
			if (wc > 0x00ff) {
//				UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
			}
			else {
				_rtf_nonascii_hex2(wc);
			}
			if(buflen - insz > 0)
			{
				count += buflen - insz;
			}
			else
			{
				count++;
			}
		}
		else {
			write (current, 1);
			current++;
			count++;
		}
	}
}

/*
 * This writes out UTF8 data by escaping non-ASCII chars.
 * using \uXXXX.
 */
void IE_Exp_RTF::_rtf_pcdata(UT_UTF8String &sPCData, bool bSupplyUC, UT_uint32 iAltChars)
{
	bool bEscaped;

	UT_UTF8String sEscapedData;
	UT_UCS4String sUCS4PCData = sPCData.ucs4_str();

	// Escape the string.
	bEscaped = s_escapeString(sEscapedData, sUCS4PCData, iAltChars);
	// If escaping was necessary and we've been asked to do it, supply
	// the appropriate \uc command.
	if (bEscaped && bSupplyUC)
		_rtf_keyword("uc", iAltChars); 
	if (m_bLastWasKeyword) {
		write(" ");
		m_bLastWasKeyword = false;
	}
	// Write the string to the file.
	write(sEscapedData.utf8_str());
}

/*
 * Access functions for above
 */
void IE_Exp_RTF::_rtf_pcdata(const std::string & szPCData, bool bSupplyUC, UT_uint32 iAltChars)
{
	UT_UTF8String str(szPCData);
	_rtf_pcdata(str, bSupplyUC, iAltChars);
}

/*
 * Access functions for above
 */
void IE_Exp_RTF::_rtf_pcdata(const char * szPCData, bool bSupplyUC, UT_uint32 iAltChars)
{
	UT_UTF8String str(szPCData);
	_rtf_pcdata(str, bSupplyUC, iAltChars);
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
		xxx_UT_DEBUGMSG(("Belcon,after wvLIDToCodePageConverter(%d),cpgname=%s\n",langcode,cpgname));
		if (g_ascii_strncasecmp(cpgname,"cp",2)==0 && UT_UCS4_isdigit(cpgname[2]))
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
			if(g_ascii_strncasecmp(codepage,"cp",2)==0 && UT_UCS4_isdigit(codepage[2]))
			{
				int cpg;
				if (sscanf(codepage+2,"%d",&cpg)==1)
				{
					_rtf_keyword("ansicpg",cpg);
					wrote_cpg = 1;
				}
			}
			xxx_UT_DEBUGMSG(("Belcon:after XAP_EncodingManager::get_instance()->CodepageFromCharset(%s),codepage=%s\n",cpgname,codepage));
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
	// Default uc value for unicode characters
	_rtf_keyword("uc",1);

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
	const gchar * pszFootnoteType = NULL;
	const PP_AttrProp* pDocAP = getDoc()->getAttrProp();
	UT_return_val_if_fail (pDocAP, false);
	pDocAP->getProperty("document-footnote-type", (const gchar *&)pszFootnoteType);
	if (pszFootnoteType == NULL)
	{
		_rtf_keyword("ftnnar");			// Numeric Footnotes
	}
	else if(pszFootnoteType[0] == 0)
	{
		_rtf_keyword("ftnnar");			// Numeric Footnotes
	}
	else if(strcmp(pszFootnoteType,"numeric") == 0)
	{
		_rtf_keyword("ftnnar");			// Numeric Footnotes
	}
	else if(strcmp(pszFootnoteType,"numeric-square-brackets") == 0)
	{
		_rtf_keyword("ftnnar");			// Numeric Footnotes
	}
	else if(strcmp(pszFootnoteType,"numeric-paren") == 0)
	{
		_rtf_keyword("ftnnar");			// Numeric Footnotes
	}
	else if(strcmp(pszFootnoteType,"numeric-open-paren") == 0)
	{
		_rtf_keyword("ftnnar");			// Numeric Footnotes
	}
	else if(strcmp(pszFootnoteType,"upper") == 0)
	{
		_rtf_keyword("ftnnauc");			// Alphabetic Upper case
	}
	else if(strcmp(pszFootnoteType,"upper-paren") == 0)
	{
		_rtf_keyword("ftnnauc");			// Alphabetic Upper case
	}
	else if(strcmp(pszFootnoteType,"upper-paren-open") == 0)
	{
		_rtf_keyword("ftnnauc");			// Alphabetic Upper case
	}
	else if(strcmp(pszFootnoteType,"lower") == 0)
	{
		_rtf_keyword("ftnnalc");			// Alphabetic Lower case
	}
	else if(strcmp(pszFootnoteType,"lower-paren") == 0)
	{
		_rtf_keyword("ftnnalc");			// Alphabetic Lower case
	}
	else if(strcmp(pszFootnoteType,"lower-paren-open") == 0)
	{
		_rtf_keyword("ftnnalc");			// Alphabetic Lower case
	}
	else if(strcmp(pszFootnoteType,"lower-roman") == 0)
	{
		_rtf_keyword("ftnnrlc");			// Roman Lower case
	}
	else if(strcmp(pszFootnoteType,"lower-roman-paren") == 0)
	{
		_rtf_keyword("ftnnrlc");			// Roman Lower case
	}
	else if(strcmp(pszFootnoteType,"upper-roman") == 0)
	{
		_rtf_keyword("ftnnruc");			// Roman Upper case
	}
	else if(strcmp(pszFootnoteType,"upper-roman-paren") == 0)
	{
		_rtf_keyword("ftnnruc");			// Roman Upper case
	}
	else
	{
		_rtf_keyword("ftnnar");			// Numeric Footnotes
	}

	const gchar * pszEndnoteType = NULL;
	pDocAP->getProperty("document-endnote-type", (const gchar *&)pszEndnoteType);
	if (pszEndnoteType == NULL)
	{
		_rtf_keyword("aftnnar");			// Numeric Endnotes
	}
	else if(pszEndnoteType[0] == 0)
	{
		_rtf_keyword("aftnnar");			// Numeric Endnotes
	}
	else if(strcmp(pszEndnoteType,"numeric") == 0)
	{
		_rtf_keyword("aftnnar");			// Numeric Endnotes
	}
	else if(strcmp(pszEndnoteType,"numeric-square-brackets") == 0)
	{
		_rtf_keyword("aftnnar");			// Numeric Endnotes
	}
	else if(strcmp(pszEndnoteType,"numeric-paren") == 0)
	{
		_rtf_keyword("aftnnar");			// Numeric Endnotes
	}
	else if(strcmp(pszEndnoteType,"numeric-open-paren") == 0)
	{
		_rtf_keyword("aftnnar");			// Numeric Endnotes
	}
	else if(strcmp(pszEndnoteType,"upper") == 0)
	{
		_rtf_keyword("aftnnauc");			// Alphabetic Upper Endnotes
	}
	else if(strcmp(pszEndnoteType,"upper-paren") == 0)
	{
		_rtf_keyword("aftnnauc");			// Alphabetic Upper Endnotes
	}
	else if(strcmp(pszEndnoteType,"upper-paren-open") == 0)
	{
		_rtf_keyword("aftnnauc");			// Alphabetic Upper Endnotes
	}
	else if(strcmp(pszEndnoteType,"lower") == 0)
	{
		_rtf_keyword("aftnnalc");			// Alphabetic Lower Endnotes
	}
	else if(strcmp(pszEndnoteType,"lower-paren") == 0)
	{
		_rtf_keyword("aftnnalc");			// Alphabetic Lower Endnotes
	}
	else if(strcmp(pszEndnoteType,"lower-paren-open") == 0)
	{
		_rtf_keyword("aftnnalc");			// Alphabetic Lower Endnotes
	}
	else if(strcmp(pszEndnoteType,"lower-roman") == 0)
	{
		_rtf_keyword("aftnnrlc");			// Roman Lower Endnotes
	}
	else if(strcmp(pszEndnoteType,"lower-roman-paren") == 0)
	{
		_rtf_keyword("aftnnrlc");			// Roman Lower Endnotes
	}
	else if(strcmp(pszEndnoteType,"upper-roman") == 0)
	{
		_rtf_keyword("aftnnruc");			// Roman Upper Endnotes
	}
	else if(strcmp(pszEndnoteType,"upper-roman-paren") == 0)
	{
		_rtf_keyword("aftnnruc");			// Roman Upper Endnotes
	}
	else
	{
		_rtf_keyword("aftnnar");			// Numeric Endnotes
	}

	const gchar * pszTmp = NULL;
	pDocAP->getProperty("document-footnote-initial", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		_rtf_keyword("ftnstart",atoi(pszTmp));			// First footnote
	}
	else
	{
		_rtf_keyword("ftnstart",1);			// First footnote
	}

	pDocAP->getProperty("document-footnote-restart-section", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(strcmp(pszTmp,"1") == 0)
		{
			_rtf_keyword("ftnrestart");			// footnote restarts each section
		}
	}

	pDocAP->getProperty("document-footnote-restart-page", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(strcmp(pszTmp,"1") == 0)
		{
			_rtf_keyword("ftnrstpg");			// footnote restarts each page
		}
	}

	pDocAP->getProperty("document-endnote-initial", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		_rtf_keyword("aftnstart", atoi(pszTmp)); // initial endnote value
	}
	pDocAP->getProperty("document-endnote-restart-section", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(strcmp(pszTmp,"1") == 0)
		{
			_rtf_keyword("aftnrestart"); // restart endnotes each section
		}
	}

	pDocAP->getProperty("document-endnote-place-endsection", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(strcmp(pszTmp,"1") == 0)
		{
			_rtf_keyword("aendnotes"); // endnotes at end of section
		}
	}

	pDocAP->getProperty("document-endnote-place-enddoc", (const gchar *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(strcmp(pszTmp,"1") == 0)
		{
			_rtf_keyword("aenddoc"); // endnotes at end of document
		}
	}
//
// Write out the facingp and titlepg keywords so that we can export our fancy
// header-footers to RTF
//
	pf_Frag_Strux* sdh = NULL;
	getDoc()->getStruxOfTypeFromPosition(2,PTX_Section,&sdh);
	if(sdh != NULL)
	{
	        PT_AttrPropIndex api = getDoc()->getAPIFromSDH(sdh);
		const PP_AttrProp * pSectionAP = NULL;
		getDoc()->getAttrProp(api,&pSectionAP);
		const char * pszAtt = NULL;
		if(pSectionAP != NULL)
		{
		     if(pSectionAP->getAttribute("header-even",pszAtt))
		     {
		          _rtf_keyword("facingp"); // Allow odd-even headers/footers
		     }
		     else if(pSectionAP->getAttribute("footer-even",pszAtt))
		     {
		          _rtf_keyword("facingp"); // Allow odd-even headers/footers
		     }
		     if(pSectionAP->getAttribute("header-first",pszAtt))
		     {
			 _rtf_keyword("titlepg"); // Allow first page headers/footers
		     }
		     else if(pSectionAP->getAttribute("footer-first",pszAtt))
		     {
			 _rtf_keyword("titlepg"); // Allow first page headers/footers
		     }
		}
		
	}
	// revisions stuff
	const UT_GenericVector<AD_Revision*> & Revs = getDoc()->getRevisions();

	if(Revs.getItemCount())
	{
		_rtf_open_brace();
		_rtf_keyword("*");
		_rtf_keyword("revtbl");
		
		UT_UTF8String s;
		UT_UCS4String s4;

		// MS Word assumes that the table always starts with author Unknown and it will
		// not bother to lookup the 0-th author, so we have to insert that dummy entry
		_rtf_open_brace();
		_rtf_chardata("Unknown", 7);
		_rtf_semi();
		_rtf_close_brace();
		
		for(UT_sint32 i = 0; i < Revs.getItemCount(); ++i)
		{
			AD_Revision* pRev = Revs.getNthItem(i);
			UT_continue_if_fail(pRev);

			s4 = pRev->getDescription();

			// construct author name from our numerical id and comment
			// (the id guarantees us uniqueness)
			UT_UTF8String_sprintf(s, "rev %d (%s)",pRev->getId(),s4.utf8_str());
			_rtf_open_brace();
			_rtf_chardata(s.utf8_str(),s.byteLength());
			_rtf_semi();
			_rtf_close_brace();
		}
		
		_rtf_close_brace();
	}

	// underline newly inserted text
	UT_uint32 iRevMode = 0;
	if(getDoc()->isShowRevisions())
	{
		iRevMode = 3;
	}
	
	_rtf_keyword("revprop", iRevMode);
	
	if(getDoc()->isMarkRevisions())
	{
		_rtf_keyword("revisions");
	}

    //
    // export the RDF too
    //
    {
        _rtf_open_brace();
		_rtf_keyword("*");
		_rtf_keyword("rdf");
        if( getDocRange() )
        {
            PD_DocumentRDFHandle rdf = getDoc()->getDocumentRDF();
            std::set< std::string > xmlids;
            rdf->addRelevantIDsForRange( xmlids, getDocRange() );
            UT_DEBUGMSG(("MIQ: RTF export creating restricted RDF model xmlids.sz:%lu\n", 
						 (unsigned long)xmlids.size()));
            PD_RDFModelHandle subm = rdf->createRestrictedModelForXMLIDs( xmlids );
            std::string rdfxml = toRDFXML( subm );
            _rtf_chardata( s_escapeXMLString(rdfxml) );

            
            // std::stringstream countss;
            // countss << subm->size();
            // _rtf_chardata( countss.str() );
            
            // PD_RDFModelIterator iter = subm->begin();
            // PD_RDFModelIterator    e = subm->end();
            // for( ; iter != e; ++iter )
            // {
            //     const PD_RDFStatement& st = *iter;
            //     _rtf_open_brace();
            //     _rtf_keyword("*");
            //     _rtf_keyword("triple");
            //     std::stringstream ss;
            //     st.getSubject().write( ss );
            //     st.getPredicate().write( ss );
            //     st.getObject().write( ss );
            //     _rtf_chardata( ss.str() );
            //     _rtf_close_brace();
            // }
            
        }
		_rtf_close_brace();
    }
    
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
					  const gchar * szPropName,
					  const char * szRTFName)
{
	const gchar * sz = NULL;
	if (pStyle->getProperty((const gchar *)szPropName, sz)) {
		_rtf_keyword_ifnotdefault_twips(szRTFName, sz, 0);
	}
}

/*!
 * Write an RTF keyword if the given property is "yes".
 */
void IE_Exp_RTF::_write_prop_ifyes(const PD_Style * pStyle,
				   const gchar * szPropName,
				   const char * szRTFName)
{
    const gchar * sz = NULL;
    if (pStyle->getProperty((const gchar *)szPropName, sz) && strcmp(sz, "yes") == 0) {
	    _rtf_keyword(szRTFName);
    }
}

/*
 * Used to hold tab information by _write_tabdef.
 */
class ABI_EXPORT _t
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
 * Get style
 */
const gchar * IE_Exp_RTF::_getStyleProp(
	s_RTF_AttrPropAdapter_Style * pADStyle,
	const s_RTF_AttrPropAdapter * apa,
	const char * szProp)
{
	const gchar *szVal = NULL;
	if(pADStyle != NULL)
	{
		szVal = pADStyle->getProperty(szProp);
		if(szVal == NULL)
		{
			szVal = apa->getProperty(szProp);
		}
		else
		{
			szVal = NULL;
		}
	}
	else
	{
		szVal = apa->getProperty(szProp);
	}
	return szVal;
}

/*!
 * Write out paragraphs-specific fmt. This
 * does not print opening and closing braces.
 */
void IE_Exp_RTF::_write_parafmt(const PP_AttrProp * pSpanAP, const PP_AttrProp * pBlockAP, const PP_AttrProp * pSectionAP,
								bool & bStartedList, pf_Frag_Strux* sdh, UT_uint32 & iCurrID, bool &bIsListBlock,
								UT_sint32 iNestLevel)
{
	const gchar * szTextAlign = PP_evalProperty("text-align",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * szFirstLineIndent = PP_evalProperty("text-indent",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * szLeftIndent = PP_evalProperty("margin-left",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * szRightIndent = PP_evalProperty("margin-right",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * szTopMargin = PP_evalProperty("margin-top",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * szBottomMargin = PP_evalProperty("margin-bottom",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * szLineHeight = PP_evalProperty("line-height",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * szKeepTogether = PP_evalProperty("keep-together",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * szKeepWithNext = PP_evalProperty("keep-with-next",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * szTabStops = PP_evalProperty("tabstops",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);

	// Borders

	const gchar * pszCanMergeBorders = PP_evalProperty("border-merge",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * pszBotBorderColor = PP_evalProperty("bot-color",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * pszBotBorderStyle = NULL;
	pBlockAP->getProperty ("bot-style",pszBotBorderStyle );
	const gchar * pszBotBorderWidth = PP_evalProperty("bot-thickness",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * pszBotBorderSpacing = PP_evalProperty("bot-space",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);

	const gchar * pszLeftBorderColor = PP_evalProperty("left-color",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * pszLeftBorderStyle = NULL;
	pBlockAP->getProperty ("left-style",pszLeftBorderStyle );
	const gchar * pszLeftBorderWidth = PP_evalProperty("left-thickness",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * pszLeftBorderSpacing = PP_evalProperty("left-space",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);

	const gchar * pszRightBorderColor = PP_evalProperty("right-color",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * pszRightBorderStyle = NULL;
	pBlockAP->getProperty ("right-style",pszRightBorderStyle );
	const gchar * pszRightBorderWidth = PP_evalProperty("right-thickness",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * pszRightBorderSpacing = PP_evalProperty("right-space",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);

	const gchar * pszTopBorderColor = PP_evalProperty("top-color",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * pszTopBorderStyle = NULL;
	pBlockAP->getProperty ("top-style",pszTopBorderStyle );
	const gchar * pszTopBorderWidth = PP_evalProperty("top-thickness",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * pszTopBorderSpacing = PP_evalProperty("top-space",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);

	// Shading

	const gchar * szPattern = PP_evalProperty("shading-pattern",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * szShadingForeCol =  PP_evalProperty("shading-foreground-color",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	//const gchar * szShadingBackCol =  PP_evalProperty("shading-background-color",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);


	// TODO add other properties here

	// Do abi specific list information.

	const gchar * szListid=NULL;
	const gchar * szParentid=NULL;
	const gchar * szListStyle=NULL;

	if (!pBlockAP || !pBlockAP->getAttribute(static_cast<const gchar*>("listid"), szListid))		szListid = NULL;
	if (!pBlockAP || !pBlockAP->getAttribute(static_cast<const gchar*>("parentid"), szParentid))
		szParentid = NULL;
	UT_uint32 listid = 0;
	const gchar * szAbiListDelim = NULL;
	const gchar * szAbiListDecimal = NULL;
	static UT_String szAbiStartValue;
	static UT_String szLevel;
	if(szListid!=NULL)
	{
		listid = atoi(szListid);
		if(listid != 0)
		{
			fl_AutoNum * pAuto = getDoc()->getListByID(listid);
			if(pAuto)
			{
				szAbiListDelim = pAuto->getDelim();
				szAbiListDecimal = pAuto->getDecimal();
				UT_String_sprintf(szAbiStartValue,"%i",pAuto->getStartValue32());
				UT_String_sprintf(szLevel,"%i",pAuto->getLevel());
			}
		}
	}
	szListStyle = PP_evalProperty("list-style",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);
	const gchar * szAbiFieldFont = PP_evalProperty("field-font",pSpanAP,pBlockAP,pSectionAP,getDoc(),true);

		
	UT_uint32 id = 0;
	if(szListid != NULL)
		id = atoi(szListid);
	if(id == 0)
	{
		_rtf_keyword("pard");		// begin a new paragraph
		_rtf_keyword("plain");		// begin a new paragraph
	}
	else
	{
		bStartedList = true;
	}
	///
	/// Output fallback numbered/bulleted label for rtf readers that don't
	/// know /*/pn
	///
	if(id != 0 )
	{
		_rtf_open_brace();
		_rtf_keyword("listtext");
		// if string is "left" use "ql", but that is the default, so we don't need to write it out.
		_rtf_keyword("pard");		// restore all defaults for this paragraph
		if (strcmp(szTextAlign,"right")==0)		// output one of q{lrcj} depending upon paragraph alignment
			_rtf_keyword("qr");
		else if (strcmp(szTextAlign,"center")==0)
			_rtf_keyword("qc");
		else if (strcmp(szTextAlign,"justify")==0)
			_rtf_keyword("qj");
		_rtf_keyword_ifnotdefault_twips("fi",static_cast<const char*>(szFirstLineIndent),0);
		_rtf_keyword_ifnotdefault_twips("li",static_cast<const char*>(szLeftIndent),0);
		_rtf_keyword_ifnotdefault_twips("ri",static_cast<const char*>(szRightIndent),0);
		_rtf_keyword_ifnotdefault_twips("sb",static_cast<const char*>(szTopMargin),0);
		_rtf_keyword_ifnotdefault_twips("sa",static_cast<const char*>(szBottomMargin),0);

		fl_AutoNum * pAuto = getDoc()->getListByID(id);
		UT_return_if_fail(pAuto);
		if(pAuto->getType()==BULLETED_LIST)
		{
			_rtf_keyword("bullet");
		}
		else
		{
			const UT_UCSChar * lab = pAuto->getLabel(sdh);

			if(lab != NULL)
			{
				UT_UTF8String tmp = lab;
				_rtf_chardata(tmp.utf8_str(),tmp.byteLength());
			}
			else
			{
			    UT_DEBUGMSG(("SEVIOR: We should not be here! id = %d \n",id));
				_rtf_chardata(" ",1);
			}
		}
		//
        // Put in Tab for braindead RTF importers (like Ted) that can't
        // do numbering.
		//
		char tab = static_cast<char>(9);
		_rtf_chardata(&tab,1);
		_rtf_close_brace();
		_rtf_keyword("pard");		// restore all defaults for this paragraph
		_rtf_keyword("plain");		// restore all defaults for this paragraph
	}
	if(bStartedList)
	{
		_rtf_open_brace();
	}

	// it is essential that the \rtlpar and \ltrpar tokens are issued
	// before any formatting, otherwise it can cause difficulties
	const gchar * szBidiDir = PP_evalProperty("dom-dir",
												 pSpanAP,
												 pBlockAP,
												 pSectionAP,
												 getDoc(),
												 true);

	xxx_UT_DEBUGMSG(("bidi paragraph: pSectionAp 0x%x, pBlockAP 0x%x, dom-dir\"%s\"\n",pSectionAP,pBlockAP,szBidiDir));
	if (szBidiDir)
	{
		if (!strcmp (szBidiDir, "ltr"))
			_rtf_keyword ("ltrpar");
		else
			_rtf_keyword ("rtlpar");
	}

	// if string is "left" use "ql", but that is the default, so we
	// don't need to write it out.
	// Except that if the alignment overrides one prescribed by a
	// style, we probably need to issue this (Tomas, Apr 12, 2003)

	// output q{lrcj depending upon paragraph alignment
	if (strcmp(szTextAlign,"left")==0)	
		_rtf_keyword("ql");
	else if (strcmp(szTextAlign,"justify")==0)
		_rtf_keyword("qj");
	else if (strcmp(szTextAlign,"right")==0)	
		_rtf_keyword("qr");
	else if (strcmp(szTextAlign,"center")==0)
		_rtf_keyword("qc");



	_rtf_keyword_ifnotdefault_twips("fi",static_cast<const char*>(szFirstLineIndent),0);
	_rtf_keyword_ifnotdefault_twips("li",static_cast<const char*>(szLeftIndent),0);
	_rtf_keyword_ifnotdefault_twips("ri",static_cast<const char*>(szRightIndent),0);
	_rtf_keyword_ifnotdefault_twips("sb",static_cast<const char*>(szTopMargin),0);
	_rtf_keyword_ifnotdefault_twips("sa",static_cast<const char*>(szBottomMargin),0);

	const gchar * szStyle = NULL;
	if (pBlockAP->getAttribute("style", szStyle))
	{
	    _rtf_keyword("s", _getStyleNumber(szStyle));
	}
///
/// OK we need to output the char props if there is a list here
	// no, we must not output character properties here, because the properties that are
	// output here will apply to everyting that will follow in this paragraph: see bug 5693
	// -- I am really not sure what the rationale for the char props output here was, so
	// if commenting this out creates some other problem, please let me know. Tomas, Sep
	// 2, 2004
#if 0 //#TF
	if(id != 0)
	{
		const PP_AttrProp * pSpanAP = NULL;
		const PP_AttrProp * pBlockAP = NULL;
		const PP_AttrProp * pSectionAP = NULL;

		getDoc()->getAttrProp(m_apiThisSection,&pSectionAP);
		getDoc()->getAttrProp(m_apiThisBlock,&pBlockAP);
		_write_charfmt(s_RTF_AttrPropAdapter_AP(pSpanAP, pBlockAP, pSectionAP, getDoc()));
	}
#endif
	///
	/// OK if there is list info in this paragraph we encase it inside
	/// the {\*\abilist..} extension
	///
	if(id != 0 )
	{
		bIsListBlock = true;

		_rtf_open_brace();
		_rtf_keyword("*");
		_rtf_keyword("abilist");
		_rtf_keyword_ifnotdefault("abilistid",static_cast<const char *>(szListid),-1);
		_rtf_keyword_ifnotdefault("abilistparentid",static_cast<const char *>(szParentid),-1);
		_rtf_keyword_ifnotdefault("abilistlevel",szLevel.c_str(),-1);
		_rtf_keyword_ifnotdefault("abistartat",szAbiStartValue.c_str(),-1);
		/// field font

		_rtf_open_brace();
		_rtf_keyword("abifieldfont");
		_rtf_chardata( static_cast<const char *>(szAbiFieldFont) ,strlen(szAbiFieldFont));
		_rtf_close_brace();

		/// list decimal

		_rtf_open_brace();
		_rtf_keyword("abilistdecimal");
		_rtf_chardata(static_cast<const char *>(szAbiListDecimal) ,strlen(szAbiListDecimal));
		_rtf_close_brace();

		/// list delim

		_rtf_open_brace();
		_rtf_keyword("abilistdelim");
		_rtf_chardata(static_cast<const char *>(szAbiListDelim) ,strlen( szAbiListDelim));
		_rtf_close_brace();

		/// list style

		_rtf_open_brace();
		_rtf_keyword("abiliststyle");
		_rtf_chardata(static_cast<const char *>(szListStyle) ,strlen( szListStyle));
		_rtf_close_brace();

		/// Finished!

		_rtf_close_brace();

	}

	///
	/// OK Now output word-95 style lists
	///

	if(id != 0 )
	{
		_rtf_open_brace();
		_rtf_keyword("*");
		_rtf_keyword("pn");
		fl_AutoNum * pAuto = getDoc()->getListByID(id);
		UT_return_if_fail(pAuto);
		_rtf_keyword("pnql");
		_rtf_keyword("pnstart",pAuto->getStartValue32());
		FL_ListType lType = pAuto->getType();

		///
		/// extract text before and after numbering symbol
		///
		static gchar p[80],leftDelim[80],rightDelim[80];
		sprintf(p, "%s",pAuto->getDelim());
		UT_uint32 rTmp;

		UT_uint32 i = 0;

		while (p[i] && p[i] != '%' && p[i+1] != 'L')
		{
			leftDelim[i] = p[i];
			i++;
		}
		leftDelim[i] = '\0';
		i += 2;
		rTmp = i;
		while (p[i] || p[i] != '\0')
		{
			rightDelim[i - rTmp] = p[i];
			i++;
		}
		rightDelim[i - rTmp] = '\0';

		fl_AutoNum * pParent = pAuto->getParent();
		if(pParent == NULL && (lType < BULLETED_LIST))
		{
			_rtf_keyword("pnlvlbody");
		}
		else if(lType >= BULLETED_LIST && (lType != NOT_A_LIST))
		{
			_rtf_keyword("pnlvlblt");
		}
		else
		{
			_rtf_keyword("pnprev");
			_rtf_keyword("pnlvl",9);
		}
		if(lType == NUMBERED_LIST)
		{
			_rtf_keyword("pndec");
		}
		else if(lType == LOWERCASE_LIST)
		{
			_rtf_keyword("pnlcltr");
		}
		else if(lType == UPPERCASE_LIST)
		{
			_rtf_keyword("pnucltr");
		}
		else if(lType == LOWERROMAN_LIST)
		{
			_rtf_keyword("pnlcrm");
		}
		else if(lType == UPPERROMAN_LIST)
		{
			_rtf_keyword("pnucrm");
		}
		else if(lType == NOT_A_LIST)
		{
			_rtf_keyword("pnucrm");
		}
		if(lType < BULLETED_LIST)
		{
			_rtf_open_brace();
			_rtf_keyword("pntxtb");
			_rtf_chardata(static_cast<const char *>(leftDelim),strlen(static_cast<const char *>(leftDelim)));
			_rtf_close_brace();
			_rtf_open_brace();
			_rtf_keyword("pntxta");
			_rtf_chardata(static_cast<const char *>(rightDelim),strlen(static_cast<const char *>(rightDelim)));
			_rtf_close_brace();
		}
		else if(lType >= BULLETED_LIST && lType != NOT_A_LIST)
		{
			_rtf_open_brace();
			_rtf_keyword("pntxtb");
			_rtf_keyword("bullet");
			_rtf_close_brace();
		}

		_rtf_close_brace();
	}



	///
	/// OK Now output word-97 style lists. First detect if we've moved to
    /// a new list list structure. We need m_currID to track the previous list
    /// we were in.
	///
	xxx_UT_DEBUGMSG(("SEVIOR: Doing output of list structure id = %d\n",id));
	if(id != 0 )
	{
		UT_uint32 iOver = getMatchingOverideNum(id);
		UT_uint32 iLevel = 0;
		fl_AutoNum * pAuto = getDoc()->getListByID(id);

//		if(id != m_currID)
		{
			UT_return_if_fail(iOver);
//			fl_AutoNum * pAuto = getDoc()->getListByID(id);
			UT_return_if_fail(pAuto);
			while(pAuto->getParent() != NULL)
			{
				pAuto = pAuto->getParent();
				iLevel++;
			}
			if(iLevel > 8)
			{
				UT_ASSERT_HARMLESS(0);
				iLevel = 8;
			}
			iCurrID = id;
		}
		/* This is changed so that Word97 can see the numbers in
		   numbered lists */
		if(pAuto->getType() < BULLETED_LIST)
		{
	        	_rtf_keyword_ifnotdefault_twips("fn",static_cast<const char*>(szFirstLineIndent),0);
	        	_rtf_keyword_ifnotdefault_twips("li",static_cast<const char*>(szLeftIndent),0);
		}
		_rtf_keyword("ls",iOver);
		_rtf_keyword("ilvl",iLevel);
	}

	if (strcmp(szLineHeight,"1.0") != 0)
	{
		double f = UT_convertDimensionless(szLineHeight);


		if (f > 0.000001) 
		{                                   // we get zero on bogus strings....
		        const char * pPlusFound = strrchr(szLineHeight, '+');
		        if (pPlusFound && *(pPlusFound + 1) == 0)             //  "+" means "at least" line spacing
			{
				UT_sint32 dSpacing = (UT_sint32)(f * 20.0);
				_rtf_keyword("sl",dSpacing);
				_rtf_keyword("slmult",0);
			}
			else if (UT_hasDimensionComponent(szLineHeight)) //  use exact line spacing
			{
			        UT_sint32 dSpacing = (UT_sint32)(f * 20.0);
			        _rtf_keyword("sl",-dSpacing);
			        _rtf_keyword("slmult",0);
		        }
			else // multiple line spacing
			{
			        UT_sint32 dSpacing = (UT_sint32)(f * 240.0);
			        _rtf_keyword("sl",dSpacing);
			        _rtf_keyword("slmult",1);
		        }
		}
	}
//
// Output Paragraph Cell nesting level.
//
	if(iNestLevel > 0)
	{
		_rtf_keyword("intbl");
	}
	_rtf_keyword("itap",iNestLevel);

	if (strcmp(szKeepTogether,"yes")==0)
		_rtf_keyword("keep");
	if (strcmp(szKeepWithNext,"yes")==0)
		_rtf_keyword("keepn");

	_write_tabdef(szTabStops);

	// Export Borders
	UT_sint32 ndx_col = 0;
	if(pszCanMergeBorders != NULL && strcmp(pszCanMergeBorders,"0") != 0)
	{
		_rtf_keyword("brdrbtw");
	}
	if(pszBotBorderStyle != NULL && *pszBotBorderStyle && strcmp(pszBotBorderStyle,"0") != 0)
	{
		UT_DEBUGMSG(("pszBotBorderStyle is %s \n",pszBotBorderStyle));
		write(" ");
		_rtf_keyword("brdrb");
		_rtf_keyword("brdrs");
		ndx_col =_findOrAddColor(pszBotBorderColor);
		if(ndx_col < 0)
			ndx_col = 0;
		_rtf_keyword("brdrcf",ndx_col);
		if(pszBotBorderWidth)
		{
			_rtf_keyword_ifnotdefault_twips("brdrw",static_cast<const char*>(pszBotBorderWidth),0);			
		}
		if(pszBotBorderSpacing)
		{
			_rtf_keyword_ifnotdefault_twips("brsp",static_cast<const char*>(pszBotBorderSpacing),0);			
		}
		write(" ");
	}	
	if(pszLeftBorderStyle != NULL && *pszLeftBorderStyle && strcmp(pszLeftBorderStyle,"0") != 0)
	{
		_rtf_keyword("brdrl");
		_rtf_keyword("brdrs");
		ndx_col =_findOrAddColor(pszLeftBorderColor);
		if(ndx_col < 0)
			ndx_col = 0;
		_rtf_keyword("brdrcf",ndx_col);
		if(pszLeftBorderWidth)
		{
			_rtf_keyword_ifnotdefault_twips("brdrw",static_cast<const char*>(pszLeftBorderWidth),0);			
		}
		if(pszLeftBorderSpacing)
		{
			_rtf_keyword_ifnotdefault_twips("brsp",static_cast<const char*>(pszLeftBorderSpacing),0);			
		}
		write(" ");
	}	
	if(pszRightBorderStyle != NULL && *pszRightBorderStyle && strcmp(pszRightBorderStyle,"0") != 0)
	{
		_rtf_keyword("brdrr");
		_rtf_keyword("brdrs");
		ndx_col =_findOrAddColor(pszRightBorderColor);
		if(ndx_col < 0)
			ndx_col = 0;
		_rtf_keyword("brdrcf",ndx_col);
		if(pszRightBorderWidth)
		{
			_rtf_keyword_ifnotdefault_twips("brdrw",static_cast<const char*>(pszRightBorderWidth),0);			
		}
		if(pszRightBorderSpacing)
		{
			_rtf_keyword_ifnotdefault_twips("brsp",static_cast<const char*>(pszRightBorderSpacing),0);			
		}
		write(" ");
	}	
	if(pszTopBorderStyle != NULL && *pszTopBorderStyle && strcmp(pszTopBorderStyle,"0") != 0)
	{
		_rtf_keyword("brdrt");
		_rtf_keyword("brdrs");
		ndx_col =_findOrAddColor(pszTopBorderColor);
		if(ndx_col < 0)
			ndx_col = 0;
		_rtf_keyword("brdrcf",ndx_col);
		if(pszTopBorderWidth)
		{
			_rtf_keyword_ifnotdefault_twips("brdrw",static_cast<const char*>(pszTopBorderWidth),0);			
		}
		if(pszTopBorderSpacing)
		{
			_rtf_keyword_ifnotdefault_twips("brsp",static_cast<const char*>(pszTopBorderSpacing),0);			
		}
		write(" ");
	}	

	// export shadings

	if(szPattern != NULL && *szPattern && strcmp(szPattern,"1") == 0)
	{
		// Can only handle solid shadings right now
		ndx_col =_findOrAddColor(szShadingForeCol);
		if(ndx_col < 0)
			ndx_col = 0;
		_rtf_keyword("cbpat",ndx_col);
		
	}

}


/*!
 * Write out the <charfmt> paragraph or character formatting. This
 * does not print opening and closing braces.
 */
void IE_Exp_RTF::_write_charfmt(const s_RTF_AttrPropAdapter & apa)
{
	//const gchar * szStyle = apa.getAttribute(PT_STYLE_ATTRIBUTE_NAME);
	//UT_sint32 iStyle = -1;
	s_RTF_AttrPropAdapter_Style * pADStyle = NULL;
#if 0
	if(szStyle != NULL)
	{
		PD_Style * pStyle = NULL;
		iStyle = static_cast<UT_sint32>(_getStyleNumber(szStyle));
		getDoc()->getStyle(szStyle,&pStyle);
		pADStyle = new s_RTF_AttrPropAdapter_Style(pStyle);
//
// OK now we have to make sure all these character props aren't in the style
//
	}
#endif
	const gchar * szColor = _getStyleProp(pADStyle,&apa,"color");

	UT_sint32 ndxColor = -1;
	if(szColor)
	{
		ndxColor = _findColor((char*)szColor);
		if( ndxColor == -1)
		{
			return;
		}
		UT_return_if_fail (ndxColor != -1);

		if (ndxColor != 0) // black text, the default
			_rtf_keyword("cf",ndxColor);
	}

	szColor = _getStyleProp(pADStyle,&apa,"bgcolor");

	if (szColor && g_ascii_strcasecmp (szColor, "transparent") != 0)
	{
		ndxColor = _findColor((char*)szColor);
		UT_ASSERT_HARMLESS(ndxColor != -1);
		if (ndxColor != 1) // white background, the default
		{
			_rtf_keyword("cb",ndxColor);
			_rtf_keyword("highlight",ndxColor);
		}
	}
	const gchar * szFont = NULL;
	if(pADStyle != NULL)
	{
		szFont = pADStyle->getProperty("font-family");
	}
	if(szFont == NULL)
	{
		UT_sint32 ndxFont = _findFont(&apa);
		if(ndxFont != -1)
			_rtf_keyword("f",ndxFont);	// font index in fonttbl
	}

	const gchar * szFontSize = _getStyleProp(pADStyle,&apa,"font-size");
	double dbl = UT_convertToPoints(szFontSize);
	UT_sint32 d = (UT_sint32)(dbl*2.0);

	// if (d != 24) - always write this out
	if(szFontSize != NULL)
	{
		if(d == 0)
			d = 24;
		_rtf_keyword("fs",d);	// font size in half points
	}
	const gchar * szFontStyle = _getStyleProp(pADStyle,&apa,"font-style");
	if (szFontStyle && *szFontStyle && (strcmp(szFontStyle,"italic")==0))
		_rtf_keyword("i");

	const gchar * szFontWeight = _getStyleProp(pADStyle,&apa,"font-weight");
	if (szFontWeight && *szFontWeight && (strcmp(szFontWeight,"bold")==0))
		_rtf_keyword("b");

	const gchar * szFontDecoration = _getStyleProp(pADStyle,&apa,"text-decoration");
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

	const gchar * szFontPosition = _getStyleProp(pADStyle,&apa,"text-position");
	if (szFontPosition && *szFontPosition)
	{
		if (!strcmp(szFontPosition,"superscript"))
			_rtf_keyword("super");
		else if (!strcmp(szFontPosition,"subscript"))
			_rtf_keyword("sub");
	}

	// export the language of the run of text
	const gchar * szLang = _getStyleProp(pADStyle,&apa,"lang");
	if ( szLang )
	  {
	    xxx_UT_DEBUGMSG(("DOM: lang,lid = %s,%d\n", szLang, wvLangToLIDConverter(szLang)));
	    _rtf_keyword("lang", wvLangToLIDConverter(szLang));
	  }

	//###TF const gchar * szDir = apa.getProperty("dir");
	const gchar * szDirOvrr = _getStyleProp(pADStyle,&apa,"dir-override");

	//bool bProceed = true;
	if (szDirOvrr)
	{
		if (!strcmp (szDirOvrr, "ltr"))
		{
			_rtf_keyword ("ltrch");
			_rtf_keyword ("abiltr");
			m_CharRTL = UT_BIDI_LTR;
			//bProceed = false;
		}
		else if (!strcmp (szDirOvrr, "rtl"))
		{
			_rtf_keyword ("rtlch");
			_rtf_keyword ("abirtl");
			m_CharRTL = UT_BIDI_RTL;
			//bProceed  = false;
		}
	}
	/*
	if (bProceed || szDir)
	{
		if (!strcmp (szDir, "ltr"))
			_rtf_keyword ("ltrch");
		else if (!strcmp (szDir, "rtl"))
			_rtf_keyword ("rtlch");
	}  */

	const gchar * szHidden = _getStyleProp(pADStyle,&apa,"display");
	if(szHidden && *szHidden && !strcmp(szHidden, "none"))
	{
		_rtf_keyword ("v");
	}
	
	
	const gchar * szListTag = apa.getProperty("list-tag");
	if (szListTag && *szListTag)
	{
		_rtf_open_brace();
		_rtf_keyword("*");
		UT_uint32 id = atoi(szListTag);
		_rtf_keyword("listtag",id);
		_rtf_close_brace();
	}
	DELETEP(pADStyle);

	// TODO do something with our font-stretch and font-variant properties
	// note: we assume that kerning has been turned off at global scope.

	// MUST BE LAST after all other props have been processed
	bool b1,b2; UT_uint32 u1; // these are only used if the bPara parameter is true
	_output_revision(apa, false, NULL, 0, b1, b2, u1);
}

void IE_Exp_RTF::_output_revision(const s_RTF_AttrPropAdapter & apa, bool bPara,
								  pf_Frag_Strux* sdh, UT_sint32 iNestLevel,
								  bool & bStartedList,  bool &bIsListBlock, UT_uint32 &iCurrID)
{
	const gchar *szRevisions = apa.getAttribute("revision");
	if (szRevisions && *szRevisions)
	{
		PP_RevisionAttr RA(szRevisions);

		if(!RA.getRevisionsCount())
		{
			UT_return_if_fail( UT_SHOULD_NOT_HAPPEN );
		}

		// we dump the revision attribute directly using our extended keyword
		// 'abirevision' (I do this only reluctantly)
		
		_rtf_open_brace();
		_rtf_keyword("*");
		_rtf_keyword("abirevision");

		UT_UTF8String s;
		const char * p = szRevisions;

		// have to escape \, {, }
		while(p && *p)
		{
			if(*p == '\\' || *p == '{' || *p == '}')
				s += '\\';

			s += *p++;
		}
		
		
		_rtf_chardata(s.utf8_str(), s.byteLength());
		_rtf_close_brace();
		

		// for now, we will just dump the lot; later we need to figure out how to deal
		// with revision conflicts ...
		for(UT_uint32 i = 0; i < RA.getRevisionsCount(); ++i)
		{
			const PP_Revision * pRev = RA.getNthRevision(i); // a revision found in our attribute
			UT_continue_if_fail(pRev);

			UT_uint32 iId = pRev->getId();

			// now we translated the revision id to and index into the revision table of
			// the document
			UT_sint32 iIndx = getDoc()->getRevisionIndxFromId(iId);
			const UT_GenericVector<AD_Revision*> & RevTbl = getDoc()->getRevisions();
			if(iIndx < 0 || RevTbl.getItemCount() == 0)
			{
				UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
				continue;
			}

			AD_Revision * pRevTblItem = RevTbl.getNthItem(iIndx);
			UT_continue_if_fail(pRevTblItem);			
			
			// the revisions in rtf are marked by a peculiar timestamp, which we now need
			// to construct
			time_t t = pRevTblItem->getStartTime();
			struct tm * pT = gmtime(&t);

			// NB: gmtime counts months 0-11, while dttm 1-12

			UT_uint32 iDttm = pT->tm_min | (pT->tm_hour << 6) | (pT->tm_mday << 11) | ((pT->tm_mon + 1) << 16)
				| (pT->tm_year << 20) | (pT->tm_wday << 29);

#if 0
			// according to the docs, we are supposed to emit each of the four bytes of
			// the dttm int as an ascii char, and if > 127 convert to hex; however, Word
			// emits this as a normal int and so will we -- the code below is disabled
			// 
			// Now that we have the int, we need to convert the 4 bytes to ascii string.
			// We need to output this in little endian order, I think, since win32 is
			// inherently LE
			char Dttm[4];
			const char * pDttm = (const char *) & iDttm;
			
#ifdef UT_LITTLE_ENDIAN
			Dttm[0] = *pDttm;
			Dttm[1] = *(pDttm + 1);
			Dttm[2] = *(pDttm + 2);
			Dttm[3] = *(pDttm + 3);
#else
			Dttm[3] = *pDttm;
			Dttm[2] = *(pDttm + 1);
			Dttm[1] = *(pDttm + 2);
			Dttm[0] = *(pDttm + 3);
#endif
			UT_UTF8String s;

			for(UT_uint32 j = 0; j < 4; j++)
			{
				if(Dttm[i] <= 127)
				{
					s += Dttm[i];
				}
				else
				{
					UT_String s2;
					_rtf_nonascii_hex2((UT_sint32)Dttm[i], s2);
					s += s2.c_str();
				}
			}
#endif
			if(iIndx < 0)
			{
				UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
				continue;
			}

			bool bRevisedProps = false;

			const char * pAD     = bPara ? "pnrnot": "revised";
			const char * pADauth = bPara ? "pnrauth" : "revauth";
			const char * pADdttm = bPara ? "pnrdate" : "revdttm";
			
			const char pDEL[]     = "deleted";
			const char pDELauth[] = "revauthdel";
			const char pDELdttm[] = "revdttmdel";

			// it seems that block props cannot be changed in rev mode
			const char * pCHauth = bPara ? NULL : "crauth";
			const char * pCHdttm = bPara ? NULL : "crdate";

			switch(pRev->getType())
			{
				case PP_REVISION_ADDITION_AND_FMT:
					bRevisedProps = true;
					// fall through
				case PP_REVISION_ADDITION:
					_rtf_keyword(pAD);
					_rtf_keyword(pADauth, iIndx+1);
					_rtf_keyword(pADdttm, iDttm);
					break;
					
				case PP_REVISION_DELETION:
					_rtf_keyword(pDEL);
					_rtf_keyword(pDELauth, iIndx+1);
					_rtf_keyword(pDELdttm, iDttm);
					break;
					
				case PP_REVISION_FMT_CHANGE:
					bRevisedProps = true;

					if(bPara)
					{
						break;
					}
					
					_rtf_keyword(pCHauth, iIndx+1);
					_rtf_keyword(pCHdttm, iDttm);
					break;

				default:
					UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			}

			if(bRevisedProps)
			{
				// need to dump the props that belong to our revision ...
				// NB: this might be a recursive call, since _output_revision()
				// gets (among others) called by _write_charfmt()
				const PP_AttrProp * pSpanAP = pRev;
				const PP_AttrProp * pBlockAP = NULL;
				const PP_AttrProp * pSectionAP = NULL;
				
				_write_charfmt(s_RTF_AttrPropAdapter_AP(pSpanAP, pBlockAP, pSectionAP, getDoc()));

				if(bPara)
				{
					UT_continue_if_fail(sdh);
					
					_write_parafmt(NULL, pRev, NULL,
								   bStartedList, sdh, iCurrID, bIsListBlock, iNestLevel);
				}
				
			}
			
		} // for
	} // if(pRevisions)
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

    const gchar * sz = NULL;
    if (pStyle->getProperty((const gchar *)"text-align", sz))
	{
		if (strcmp(sz, "left") == 0)
		{
			// Default, so no need to print anything
		}
		else if (strcmp(sz, "right") == 0)
		{
			_rtf_keyword("qr");
		}
		else if (strcmp(sz, "center") == 0)
		{
			_rtf_keyword("qc");
		}
		else if (strcmp(sz, "justify") == 0)
		{
			_rtf_keyword("qj");
		}
		else
		{
		  UT_ASSERT_NOT_REACHED();
		}
    }

    const gchar * szLineHeight = NULL;
    if (pStyle->getProperty((const gchar *) "line-height", szLineHeight)
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
    if (pStyle->getProperty((const gchar *) "tabstops", sz)) _write_tabdef(sz);


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

    NumberedStyle(const PD_Style * _pStyle, UT_uint32 _n) :
		pStyle(_pStyle), n(_n) {}
};

/*!
 * Clear the style hash.
 */
void IE_Exp_RTF::_clearStyles()
{
	m_hashStyles.purgeData();
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
	UT_GenericVector<PD_Style*> vecStyles;
	getDoc()->getAllUsedStyles(&vecStyles);

	UT_GenericVector<PD_Style*> * pStyles = NULL;
	getDoc()->enumStyles(pStyles);
	UT_return_if_fail( pStyles );
	UT_uint32 iStyleCount = getDoc()->getStyleCount();

	for (i = 0; i < iStyleCount; ++i)
	{
		// DOM: hack for 2069 - we'll export all styles instead of just
		// user-defined styles and used styles. To fix it (and export fewer
		// styles in general) make this routine recursive to include
		// parent (basedon) styles as well...
		pStyle = pStyles->getNthItem(i);
		UT_return_if_fail( pStyle );

		szName = pStyle->getName();
		
		if (true /* pStyle->isUserDefined() || (vecStyles.findItem((void *) pStyle) >= 0)*/)
		{
			//
			// Add this style to the hash
			//
			NumberedStyle * pns = (NumberedStyle *) m_hashStyles.pick(szName);
			if(pns == NULL)
			{
				m_hashStyles.insert(szName, new NumberedStyle(pStyle, ++nStyleNumber));
				{
					_rtf_font_info fi;

					if (fi.init(static_cast<s_RTF_AttrPropAdapter_Style>(pStyle))) {
						if (_findFont(&fi) == -1)
							_addFont(&fi);
					}
				}
				//
				// Now do it for the field font as well
				//
				{
					_rtf_font_info fi;

					if (fi.init(static_cast<s_RTF_AttrPropAdapter_Style>(pStyle),true)) {
						if (_findFont(&fi) == -1)
							_addFont(&fi);
					}
				}
			}
		}
    }

	delete pStyles;
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
UT_uint32 IE_Exp_RTF::_getStyleNumber(const gchar * szStyle)
{
	if(strcmp(szStyle, "Normal Clean")== 0)
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

    UT_GenericStringMap<NumberedStyle*>::UT_Cursor hc(&m_hashStyles);
    const NumberedStyle * pns;
    for (pns = hc.first(); hc.is_valid(); pns = hc.next())
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

		const PD_Style * pStyleBasedOn =  reinterpret_cast<const PD_Style *> (pStyle->getBasedOn());
		// TODO: Can this really return NULL?
		if (pStyleBasedOn != NULL)
		{
			_rtf_keyword("sbasedon", _getStyleNumber(pStyleBasedOn));
		}

		const PD_Style * pStyleNext = reinterpret_cast<const PD_Style *> (pStyle->getFollowedBy());
		// TODO: Can this really return NULL?
		if (pStyleNext != NULL)
		{
			_rtf_keyword("snext", _getStyleNumber(pStyleNext));
		}
		_rtf_pcdata(pStyle->getName(), true); 
		_rtf_semi();
		_rtf_close_brace();
    }

    _rtf_close_brace();
}

/*!
 * Write the listatble group of the RTF header.
 */
void IE_Exp_RTF::_write_listtable(void)
{
	UT_sint32 iCount = getDoc()->getListsCount();
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
	UT_sint32 i,j =0;
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
					xxx_UT_DEBUGMSG(("SEVIOR: Adding %x to multi-level \n",pAuto));
					m_vecMultiLevel.addItem((void *) new ie_exp_RTF_MsWord97ListMulti(pAuto));
					bFoundChild = true;
					break;
				}
			}
			if(!bFoundChild)
			{
				xxx_UT_DEBUGMSG(("SEVIOR: Adding %x to simple \n",pAuto));
				m_vecSimpleList.addItem((void *) new ie_exp_RTF_MsWord97ListSimple(pAuto));
			}
		}
	}
//
// OK now fill the MultiLevel list structure.
//

	UT_sint32 k;
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
				xxx_UT_DEBUGMSG(("SEVIOR: Adding NULL level at depth %d \n",depth));
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
						xxx_UT_DEBUGMSG(("SEVIOR: Adding level %x at depth %d \n",pCur97,depth));
						pList97->addLevel(depth, pCur97);
					}
				}
			}
			if(!bFoundAtPrevLevel)
			{
				ie_exp_RTF_MsWord97List * pCur97 = new ie_exp_RTF_MsWord97List(pList97->getAuto());
				xxx_UT_DEBUGMSG(("SEVIOR: Adding NULL level at depth %d \n",depth));
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
	for(k=0; k< m_vecMultiLevel.getItemCount(); k++)
	{
		_rtf_nl();
		_output_MultiLevelRTF(getNthMultiLevel(k));
	}
//
// Simple Lists
//
	for(k=0; k< m_vecSimpleList.getItemCount(); k++)
	{
		_rtf_nl();
		_output_SimpleListRTF(getNthSimple(k));
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
 \param ie_exp_RTF_MsWord97ListMulti * pMulti pointer to a multi-level list
 * structure.
 */
void IE_Exp_RTF::_output_MultiLevelRTF(ie_exp_RTF_MsWord97ListMulti * pMulti)
{
	_rtf_open_brace();
	_rtf_keyword("list");
#if 0
	UT_uint32 tempID = UT_rand();
	while(tempID < 10000)
	{
		tempID = UT_rand();
	}
#else
	UT_uint32 tempID = getDoc()->getUID(UT_UniqueId::List);
#endif
	
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
		xxx_UT_DEBUGMSG(("SEVIOR: Final level text string is %s \n",tmp.c_str()));
		write(tmp.c_str());
		_rtf_close_brace();
		_rtf_open_brace();
		_rtf_keyword("levelnumbers");
		write(LevelNumbers.c_str());
		write(";");
	}
	else
	{
		_rtf_keyword("'01");
		std::string sBullet = UT_std_string_sprintf("\\u%d",(UT_sint32) bulletsym);
		write(sBullet.c_str());
		write(" ;");
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
	xxx_UT_DEBUGMSG(("SEVIOR: pAuto %x \n",pAuto));
	if(pAuto)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: pAuto-getParent() %x \n",pAuto->getParent()));
	}
	if(pAuto && (pAuto->getParent() == NULL))
	{
		UT_String LeftSide = pAuto->getDelim();
		UT_String RightSide;
		_get_LeftRight_Side(LeftSide,RightSide);
		xxx_UT_DEBUGMSG(("SEVIOR: Top - leftside = %s rightside = %s \n",LeftSide.c_str(),RightSide.c_str()));
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
		xxx_UT_DEBUGMSG(("SEVIOR: Level %d LevelText %s  \n",ifoundLevel,LevelText.c_str()));
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
				xxx_UT_DEBUGMSG(("SEVIOR: RightSide =%s last char = %c \n",RightSide.c_str(),RightSide[RightSide.size()-1]));
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
		xxx_UT_DEBUGMSG(("SEVIOR: Level %d LevelText %s  \n",ifoundLevel,LevelText.c_str()));
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
	xxx_UT_DEBUGMSG(("SEVIOR: Substring = %s Total is %s \n",psz,LeftSide.c_str()));
	if(psz != NULL)
	{
		UT_uint32 index = (UT_uint32) (psz - LeftSide.c_str());
		UT_uint32 len = (UT_uint32) strlen(LeftSide.c_str());
		xxx_UT_DEBUGMSG(("SEVIOR: index = %d len =%d \n",index,len));
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
	UT_UCSChar bulletsym=0;
	FL_ListType lType = NUMBERED_LIST;
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
		bulletsym = 0x2022;
		break;
	case DASHED_LIST:
		Param = 23;
		bulletsym = 0x002D;
		break;
	case SQUARE_LIST:
		Param = 23;
		bulletsym = 0x25A0;
		break;
	case TRIANGLE_LIST:
		Param = 23;
		bulletsym = 0x25B2;
		break;
	case DIAMOND_LIST:
		Param = 23;
		bulletsym = 0x2666;
		break;
	case STAR_LIST:
		Param = 23;
		bulletsym = 0x2733;
		break;
	case IMPLIES_LIST:
		Param = 23;
		bulletsym = 0x21D2;
		break;
	case TICK_LIST:
		Param = 23;
		bulletsym = 0x2713;
		break;
	case BOX_LIST:
		Param = 23;
		bulletsym = 0x2752;
		break;
	case HAND_LIST:
		Param = 23;
		bulletsym = 0x261E;
		break;
	case HEART_LIST:
		Param = 23;
		bulletsym = 0x2665;
		break;
	case ARROWHEAD_LIST:
		Param = 23;
		bulletsym = 0x27A3;
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
		pf_Frag_Strux* sdh = pAuto->getFirstItem();
		const char * szIndent = NULL;
		const char * szAlign = NULL;
		// TODO -- we have a problem here; props and attrs are, due to revisions, view dependent and
		// we have no access to the view, so we will assume that revisions are showing and will ask
		// for the cumulative result of all of them (revision level PD_MAX_REVISION)
		// 
		if(sdh != NULL)
		{
			bool bres = getDoc()->getPropertyFromSDH(sdh,true,PD_MAX_REVISION,"text-indent",&szIndent);
			if(bres)
			{
				_rtf_keyword_ifnotdefault_twips("fi",(char*)szIndent,0);
			}
			bres = getDoc()->getPropertyFromSDH(sdh,true,PD_MAX_REVISION,"margin-left",&szAlign);
			if(bres)
			{
				_rtf_keyword_ifnotdefault_twips("li",(char*)szAlign,0);
			}
		}
	}

// Leveltext and levelnumbers
//
	_output_LevelText(pAuto,iLevel,bulletsym);
}

/*!
 * Actually output the RTF from a Simple list
 \param ie_exp_RTF_MsWord97ListSimple * pSimple pointer to a Simple list
 * structure.
 */
void IE_Exp_RTF::_output_SimpleListRTF(ie_exp_RTF_MsWord97ListSimple * pSimple)
{
	_rtf_open_brace();
	_rtf_keyword("list");
#if 0
	UT_uint32 tempID = UT_rand();
	while(tempID < 10000)
	{
		tempID = UT_rand();
	}
#else
	UT_uint32 tempID = getDoc()->getUID(UT_UniqueId::List);
#endif
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
 \param ie_exp_RTF_Overide * pOver pointer to an Overide definition
 */
void IE_Exp_RTF::_output_OveridesRTF(ie_exp_RTF_ListOveride * pOver, UT_uint32 /*iOver*/)
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

	_rtf_font_info fi;
	
	if (fi.init(*apa)) {
		ifont = _findFont(&fi);
		return ifont;
	}
	
	return -1;
}

/*!
 * Add a font to the font table. The font must not be present
 * in the font table at the start of the call.
 */
void IE_Exp_RTF::_addFont(const _rtf_font_info * pfi)
{
	UT_return_if_fail(pfi && (_findFont(pfi)==-1));

	_rtf_font_info * pNew = new _rtf_font_info (*pfi);
	
	if (pNew)
		m_vecFonts.addItem(pNew);
}

/*
 * Convert a UCS4 string into an ASCII string by using \uXXXXX
 * sequences to escape any non-ascii data.
 *
 * returns true if escaping was necesary and false otherwise.
 * iAltChars specifies the number of alternative characters to provide.
 */
bool IE_Exp_RTF::s_escapeString(UT_UTF8String &sOutStr, 
                                UT_UCS4String &sInStr,
                                UT_uint32 iAltChars)
{
	sOutStr = "";  // Empty output string.
	bool bRetVal = false;
	
	// Loop for each character in the UCS-4 string checking for
	// non-ascii characters.
	for (UT_uint32 i=0; i<sInStr.size(); i++)
	{
		// If an ASCII character append to output string.
		if (sInStr[i] <= 0x7f) 
		{
			sOutStr += sInStr[i];
			continue;
		}
		// If a code point representable in UCS-2 (without surrogates)
		// write out a \uXXXXX escaped sequence.
		if (sInStr[i] > 0x7f && sInStr[i] <=0xffff)
		{
			bRetVal = true;
			// RTF is limited to +-32K ints, therefore negative numbers
			// are needed to represent some unicode chars.
			signed short tmp = (signed short) ((unsigned short) sInStr[i]);
			// Append a \uXXXXX sequence.
			sOutStr += UT_UTF8String_sprintf("\\u%d",tmp);
			// Append alternative chars.
			if (iAltChars)
				sOutStr += " ";
			for (UT_uint32 j=0; j<iAltChars; j++)
				sOutStr += "?";
			continue;
		}
		// TODO: Strictly speaking we should be using the UTF-16 encoding,
		// since we ought to represent unicode values above 0xffff as surrogate
		// pairs. However, the abi util classes lack a UT_UCS2String class.
		// We could use UT_convert() to convert to UCS-2, but that's rather messy.
		// In any case, unicode points above 0xffff are rather rare, so for the time
		// being we just assert and replace such code points with a "?".
		UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
		sOutStr += "?";
	}

	return bRetVal;
}

/* 
 * Various access functions for above.
 */
bool IE_Exp_RTF::s_escapeString(UT_UTF8String &sOutStr, 
                                const char * szInStr,
                                UT_uint32 iSize,  /* 0 == NULL terminated */
                                UT_uint32 iAltChars)
{
	UT_UCS4String sUCS4InStr(szInStr, iSize);
	return IE_Exp_RTF::s_escapeString(sOutStr, sUCS4InStr, iAltChars);
}

bool IE_Exp_RTF::s_escapeString( std::string& outStr, const std::string& inStr, 
                                 UT_uint32 iAltChars )
{
    UT_UTF8String sOutStr;
    bool ret = s_escapeString( sOutStr, inStr.c_str(), inStr.length(), iAltChars );
    outStr = sOutStr.utf8_str();
    return ret;
}

std::string IE_Exp_RTF::s_escapeString( const std::string& inStr, UT_uint32 iAltChars )
{
    UT_UTF8String sOutStr;
    /*bool ret =*/ s_escapeString( sOutStr, inStr.c_str(), inStr.length(), iAltChars );
    return (std::string)sOutStr.utf8_str();
}


std::string IE_Exp_RTF::s_escapeXMLString( const std::string& inStr )
{
    //
    // &7d; is }
    //
    std::string s = inStr;
    s = replace_all( s, "&7d;",  "&7d;&7d;" );
    s = replace_all( s, "}",  "&7d;" );
//    s = s_escapeString( s );
    return s;
}



_rtf_font_info::_rtf_font_info()
{
	// TODO: set these to some default bogus values
}

bool _rtf_font_info::init(const s_RTF_AttrPropAdapter & apa, bool bDoFieldFont)
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
	if (szName == NULL || strcmp(szName, "NULL") == 0)  // Field-font is "NULL" when there is no special field-font.
	{                                                      // We don't want it in the \fonttbl
		return false;
	}

	static const char * t_ff[] = { "fnil", "froman", "fswiss", "fmodern", "fscript", "fdecor", "ftech", "fbidi" };
	GR_Font::FontFamilyEnum ff;
	GR_Font::FontPitchEnum fp;
	bool tt;
	GR_Font::s_getGenericFontProperties((char*)szName, &ff, &fp, &tt);

	if ((ff >= 0) && (ff < (int)G_N_ELEMENTS(t_ff)))
		szFamily = t_ff[ff];
	else
		szFamily = t_ff[GR_Font::FF_Unknown];
	nCharset = XAP_EncodingManager::get_instance()->getWinCharsetCode();
	nPitch = fp;
	fTrueType = tt;

	return true;
}

_rtf_font_info::~_rtf_font_info(void)
{
}

bool _rtf_font_info::init(const char * szFontName)
{
    // Not a typo. The AbiWord "font-family" property is what RTF
    // calls font name. It has values like "Courier New".
    if (szFontName == NULL)
	{
		return false;
	}

	m_szName = szFontName;

    static const char * t_ff[] = { "fnil", "froman", "fswiss", "fmodern", "fscript", "fdecor", "ftech", "fbidi" };
    GR_Font::FontFamilyEnum ff;
    GR_Font::FontPitchEnum fp;
    bool tt;
    GR_Font::s_getGenericFontProperties(m_szName.c_str(), &ff, &fp, &tt);

    if ((ff >= 0) && (ff < (int)G_N_ELEMENTS(t_ff)))
		szFamily = t_ff[ff];
    else
		szFamily = t_ff[GR_Font::FF_Unknown];
    nCharset = XAP_EncodingManager::get_instance()->getWinCharsetCode();
    nPitch = fp;
    fTrueType = tt;

	return true;
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
		bMatchFontFamily =  strcmp(szFamily, fi.szFamily) == 0;
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
		bMatchFontName =  strcmp(m_szName.c_str(), fi.m_szName.c_str()) == 0;
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
\param iLevel to add the list too
\param ie_exp_RTF_MsWord97List * pList97 list to added at this level
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
 \param iLevel the level which we want to lists for
 \param nthList the list at the level we want
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
 \param listID the listID we're looking for.
 \retval the List ID number of the first list on the level that contains
 the ID. Return 0 if there is no match in the structure.
 */
UT_uint32 ie_exp_RTF_MsWord97ListMulti::getMatchingID(UT_uint32 listID)
{
	UT_uint32 i;
	UT_sint32 j;
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







