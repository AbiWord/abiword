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
#include "pt_Types.h"
#include "ie_exp_RTF.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "xap_App.h"
#include "pd_Style.h"
#include "gr_Graphics.h"

#include "wv.h" //for wvLIDToCodePageConverter
#include "xap_EncodingManager.h"

/*****************************************************************/
/*****************************************************************/

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
	m_bNeedUnicodeText = UT_FALSE;
	m_braceLevel = 0;
	m_bLastWasKeyword = UT_FALSE;
	m_atticFormat = UT_FALSE;
}

IE_Exp_RTF::IE_Exp_RTF(PD_Document * pDocument,UT_Bool atticFormat)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListenerWriteDoc = NULL;
	m_pListenerGetProps = NULL;
	m_bNeedUnicodeText = UT_FALSE;
	m_braceLevel = 0;
	m_bLastWasKeyword = UT_FALSE;
	m_atticFormat = atticFormat;
}

IE_Exp_RTF::~IE_Exp_RTF()
{
	UT_VECTOR_PURGEALL(char *,m_vecColors);
	UT_VECTOR_PURGEALL(_rtf_font_info *,m_vecFonts);
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Exp_RTF::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".rtf") == 0);
}

UT_Error IE_Exp_RTF::StaticConstructor(PD_Document * pDocument,
									   IE_Exp ** ppie)
{
	IE_Exp_RTF * p = new IE_Exp_RTF(pDocument);
	*ppie = p;
	return UT_OK;
}

UT_Bool	IE_Exp_RTF::GetDlgLabels(const char ** pszDesc,
								 const char ** pszSuffixList,
								 IEFileType * ft)
{
	*pszDesc = "Rich Text Format (.rtf)";
	*pszSuffixList = "*.rtf";
	*ft = IEFT_RTF;
	return UT_TRUE;
}

UT_Bool IE_Exp_RTF::SupportsFileType(IEFileType ft)
{
	return (IEFT_RTF == ft);
}

/*for attic*/
UT_Bool IE_Exp_RTF::RecognizeSuffix_attic(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".rtf") == 0);
}

UT_Error IE_Exp_RTF::StaticConstructor_attic(PD_Document * pDocument,
									   IE_Exp ** ppie)
{
	IE_Exp_RTF * p = new IE_Exp_RTF(pDocument,1);
	*ppie = p;
	return UT_OK;
}

UT_Bool	IE_Exp_RTF::GetDlgLabels_attic(const char ** pszDesc,
								 const char ** pszSuffixList,
								 IEFileType * ft)
{
	*pszDesc = "Rich Text Format for old apps (.rtf)";
	*pszSuffixList = "*.rtf";
	*ft = IEFT_RTF_attic;
	return UT_TRUE;
}

UT_Bool IE_Exp_RTF::SupportsFileType_attic(IEFileType ft)
{
	return (IEFT_RTF_attic == ft);
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
		
	m_pListenerGetProps = new s_RTF_ListenerGetProps(m_pDocument,this);
	if (!m_pListenerGetProps)
		return UT_IE_NOMEMORY;
	if (m_pDocRange)
		m_pDocument->tellListenerSubset(static_cast<PL_Listener *>(m_pListenerGetProps),m_pDocRange);
	else
		m_pDocument->tellListener(static_cast<PL_Listener *>(m_pListenerGetProps));
	DELETEP(m_pListenerGetProps);

	// write rtf header

	if (!_write_rtf_header())
		return UT_IE_COULDNOTWRITE;
	
	// create and install a listener to receive the document
	// and write its content in rtf.
	
	m_pListenerWriteDoc = new s_RTF_ListenerWriteDoc(m_pDocument,this, (m_pDocRange!=NULL));
	if (!m_pListenerWriteDoc)
		return UT_IE_NOMEMORY;
	if (m_pDocRange)
		m_pDocument->tellListenerSubset(static_cast<PL_Listener *>(m_pListenerWriteDoc),m_pDocRange);
	else
		m_pDocument->tellListener(static_cast<PL_Listener *>(m_pListenerWriteDoc));
	DELETEP(m_pListenerWriteDoc);

	// write any rtf trailer matter

	if (!_write_rtf_trailer())
		return UT_IE_COULDNOTWRITE;
	
	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}

/*****************************************************************/
/*****************************************************************/
#if 0
void s_RTF_Listener::_handleStyles(void)
{
	UT_Bool bWroteOpenStyleSection = UT_FALSE;

	const char * szName;
	const PD_Style * pStyle;

	for (UT_uint32 k=0; (m_pDocument->enumStyles(k,&szName,&pStyle)); k++)
	{
		if (!pStyle->isUsed())
			continue;

		if (!bWroteOpenStyleSection)
		{
			m_pie->write("<styles>\n");
			bWroteOpenStyleSection = UT_TRUE;
		}

		PT_AttrPropIndex api = pStyle->getIndexAP();
		_openTag("s","/",UT_TRUE,api);
	}

	if (bWroteOpenStyleSection)
		m_pie->write("</styles>\n");

	return;
}

void s_RTF_Listener::_handleDataItems(void)
{
	UT_Bool bWroteOpenDataSection = UT_FALSE;

	const char * szName;
	const UT_ByteBuf * pByteBuf;

	UT_ByteBuf bb64(1024);

	for (UT_uint32 k=0; (m_pDocument->enumDataItems(k,NULL,&szName,&pByteBuf,NULL)); k++)
	{
		if (!bWroteOpenDataSection)
		{
			m_pie->write("<data>\n");
			bWroteOpenDataSection = UT_TRUE;
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

/*****************************************************************/
/*****************************************************************/

void IE_Exp_RTF::_rtf_open_brace(void)
{
	m_braceLevel++;
	write("{");
	m_bLastWasKeyword = UT_FALSE;
}

void IE_Exp_RTF::_rtf_close_brace(void)
{
	m_braceLevel--;
	write("}");
	m_bLastWasKeyword = UT_FALSE;

	UT_ASSERT(m_braceLevel >= 0);
}

void IE_Exp_RTF::_rtf_keyword(const char * szKey)
{
	write("\\");
	write(szKey);
	m_bLastWasKeyword = UT_TRUE;
}

/* output a non-ascii char in the RTF stream. */
void IE_Exp_RTF::_rtf_nonascii_hex2 (UT_sint32 d)
{
	write("\\'");
        char buf[100];
	sprintf(buf,"%02x",d);
	write(buf);
	m_bLastWasKeyword = UT_FALSE;
}


void IE_Exp_RTF::_rtf_keyword(const char * szKey, UT_sint32 d)
{
	write("\\");
	write(szKey);
	char buf[100];
	sprintf(buf,"%d",d);
	write(buf);
	m_bLastWasKeyword = UT_TRUE;
}

void IE_Exp_RTF::_rtf_keyword_hex2(const char * szKey, UT_sint32 d)
{
	write("\\");
	write(szKey);
	char buf[100];
	sprintf(buf,"%02x",d);
	write(buf);
	m_bLastWasKeyword = UT_TRUE;
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
	m_bLastWasKeyword = UT_TRUE;
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
	m_bLastWasKeyword = UT_TRUE;
}

void IE_Exp_RTF::_rtf_semi(void)
{
	write(";");
	m_bLastWasKeyword = UT_FALSE;
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
	write(pbuf,buflen);
	m_bLastWasKeyword = UT_FALSE;
}

void IE_Exp_RTF::_rtf_nl(void)
{
	write("\n");
}

UT_Bool IE_Exp_RTF::_write_rtf_header(void)
{
	UT_uint32 k,kLimit;

	UT_uint32 langcode = XAP_EncodingManager::instance->getWinLanguageCode();	
	// write <rtf-header>
	// return UT_FALSE on error

	_rtf_open_brace();
	_rtf_keyword("rtf",1);				// major version number of spec version 1.5
	
	_rtf_keyword("ansi");
	UT_Bool wrote_cpg = 0;
	if (langcode) 
	{
		char* cpgname = wvLIDToCodePageConverter(langcode);
		if (UT_strnicmp(cpgname,"cp",2)==0 && UT_UCS_isdigit(cpgname[2])) 
		{
			int cpg;
			if (sscanf(cpgname+2,"%d",&cpg)==1) 
			{
				_rtf_keyword("ansicpg",cpg);
				wrote_cpg = 1;
			}
		};
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
	_rtf_nl();
	_rtf_open_brace();
	_rtf_keyword("fonttbl");
	UT_uint32 charsetcode = XAP_EncodingManager::instance->getWinCharsetCode();
	for (k=0; k<kLimit; k++)
	{
		const _rtf_font_info * pk = (const _rtf_font_info *)m_vecFonts.getNthItem(k);
		const char * szFontName = NULL;
		const char * szFamily = NULL;
		int pitch;
		UT_Bool bTrueType;
		
		_rtf_compute_font_properties(pk,&szFontName,&szFamily,&pitch,&bTrueType);
		
		_rtf_nl();
		_rtf_open_brace();
		_rtf_keyword("f",k);								// font index number
		_rtf_keyword(szFamily);								// {\fnil,\froman,\fswiss,...}
		_rtf_keyword("fcharset",charsetcode);
		_rtf_keyword("fprq",pitch);							// {0==default,1==fixed,2==variable}
		_rtf_keyword((bTrueType) ? "fttruetype" : "ftnil");	// {\fttruetype,\ftnil}
		
		// we do nothing with or use default values for
		// \falt \panose \fname \fbias \ftnil \fttruetype \fontfile

		// after we write the various generic font properties, we write
		// the actual font name and a semicolon -- i couldn't see this
		// described in the specification, but it was in other RTF files
		// that i saw and really seems to help Word and WordPad....
		_rtf_fontname(szFontName);
		
		_rtf_close_brace();
	}
	_rtf_close_brace();
	
	// TODO write the "file table" if necessary...

	kLimit = m_vecColors.getItemCount();
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

	// TODO write the "style sheets"...
	// TODO write the "list table"...
	// TODO write the "rev table"...

	// write default character properties at global scope...
	_rtf_nl();
	_rtf_keyword("kerning",0);			// turn off kerning
	_rtf_keyword("cf",0);				// set color 0 -- black
	
	return (m_error == 0);
}

UT_Bool IE_Exp_RTF::_write_rtf_trailer(void)
{
	_rtf_close_brace();
	return (m_error == 0);
}

UT_sint32 IE_Exp_RTF::_findFont(const _rtf_font_info * pfi) const
{
	UT_ASSERT(pfi);

	UT_uint32 k;
	UT_uint32 kLimit = m_vecFonts.getItemCount();

	for (k=0; k<kLimit; k++)
	{
		const _rtf_font_info * pk = (const _rtf_font_info *)m_vecFonts.getNthItem(k);
		if (pk->_is_same(pfi))
			return k;
	}

	return -1;
}

void IE_Exp_RTF::_addFont(const _rtf_font_info * pfi)
{
	UT_ASSERT(pfi && (_findFont(pfi)==-1));

	// note: this does not guarantee uniqueness of actual fonts,
	// note: since the three AP's may have other stuff besides
	// note: just font info -- two identical fonts with different
	// note: colors, for example -- will appear as two distinct
	// note: entries -- we don't care.
	
	_rtf_font_info * pNew = new _rtf_font_info(*pfi);
	if (pNew)
		m_vecFonts.addItem(pNew);

	return;
}

void IE_Exp_RTF::_rtf_compute_font_properties(const _rtf_font_info * pfi,
											  const char ** p_sz_font_name,
											  const char ** p_sz_rtf_family,
											  int * p_rtf_pitch,
											  UT_Bool * p_rtf_bTrueType) const
{
	static const char * t_ff[] = { "fnil", "froman", "fswiss", "fmodern", "fscript", "fdecor", "ftech", "fbidi" };

	const XML_Char * szFontFamily = PP_evalProperty("font-family",
													pfi->m_pSpanAP,pfi->m_pBlockAP,pfi->m_pSectionAP,
													m_pDocument,UT_TRUE);

	GR_Font::FontFamilyEnum ff;
	GR_Font::FontPitchEnum fp;
	UT_Bool tt;
	
	GR_Font::s_getGenericFontProperties((char*)szFontFamily, &ff, &fp, &tt);

	// TODO there is a general confusion in this program between fontname and fontfamily.
	// TODO one is "Courier New" and the other is "Modern".  it seems that we interchange
	// TODO these in a few places....
	
	*p_sz_font_name = szFontFamily;
	
	if ((ff >= 0) && (ff < (int)NrElements(t_ff)))
		*p_sz_rtf_family = t_ff[ff];
	else
		*p_sz_rtf_family = t_ff[GR_Font::FF_Unknown];

	*p_rtf_pitch = fp;

	*p_rtf_bTrueType = tt;
}
