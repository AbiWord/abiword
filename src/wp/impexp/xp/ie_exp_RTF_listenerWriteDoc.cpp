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

/******************************************************************
** This file is considered private to ie_exp_RTF.cpp
** This is a PL_Listener.  It's purpose is to actually write
** the contents of the document to the RTF file.
******************************************************************/

#include <string.h>
#include <stdlib.h>
#include "ut_string.h"
#include "ut_units.h"
#include "ie_exp_RTF_listenerWriteDoc.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"

void s_RTF_ListenerWriteDoc::_closeSection(void)
{
	m_apiThisSection = NULL;
	return;
}

void s_RTF_ListenerWriteDoc::_closeBlock(void)
{
	m_apiThisBlock = NULL;
	return;
}

void s_RTF_ListenerWriteDoc::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	m_pie->_rtf_close_brace();
	m_bInSpan = UT_FALSE;
	return;
}

void s_RTF_ListenerWriteDoc::_openSpan(PT_AttrPropIndex apiSpan)
{
	if (m_bInSpan)
	{
		if (m_apiLastSpan == apiSpan)
			return;
		_closeSpan();
	}

	m_pie->_rtf_open_brace();

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);
	m_pDocument->getAttrProp(m_apiThisBlock,&pBlockAP);
	m_pDocument->getAttrProp(apiSpan,&pSpanAP);

	const XML_Char * szColor = PP_evalProperty("color",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	UT_sint32 ndxColor = m_pie->_findColor(szColor);
	UT_ASSERT(ndxColor != -1);
	if (ndxColor != 0)
		m_pie->_rtf_keyword("cf",ndxColor);

	UT_sint32 ndxFont = m_pie->_findFont(&_rtf_font_info(pSpanAP,pBlockAP,pSectionAP));
	UT_ASSERT(ndxFont != -1);
	m_pie->_rtf_keyword("f",ndxFont);	// font index in fonttbl

	const XML_Char * szFontSize = PP_evalProperty("font-size",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	double dbl = UT_convertToPoints(szFontSize);
	UT_sint32 d = (UT_sint32)(dbl*2.0);
	if (d != 24)
		m_pie->_rtf_keyword("fs",d);	// font size in half points

	const XML_Char * szFontStyle = PP_evalProperty("font-style",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	if (szFontStyle && *szFontStyle && (UT_stricmp(szFontStyle,"italic")==0))
		m_pie->_rtf_keyword("i");

	const XML_Char * szFontWeight = PP_evalProperty("font-weight",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	if (szFontWeight && *szFontWeight && (UT_stricmp(szFontWeight,"bold")==0))
		m_pie->_rtf_keyword("b");

	const XML_Char * szFontDecoration = PP_evalProperty("text-decoration",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	if (szFontDecoration && *szFontDecoration)
	{
		if (strstr(szFontDecoration,"underline") != 0)
			m_pie->_rtf_keyword("ul");
		if (strstr(szFontDecoration,"line-through") != 0)
			m_pie->_rtf_keyword("strike");
	}

	// TODO do something with our font-stretch and font-variant properties
	// note: we assume that kerning has been turned off at global scope.
	
	m_bInSpan = UT_TRUE;
	m_apiLastSpan = apiSpan;
	return;
}

void s_RTF_ListenerWriteDoc::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
#define MY_BUFFER_SIZE		1024
#define MY_HIGHWATER_MARK	20
#define FlushBuffer()		do { if (pBuf > buf) { m_pie->_rtf_chardata(buf,(pBuf-buf)); pBuf=buf; } } while (0)

	char buf[MY_BUFFER_SIZE];
	char * pBuf;
	const UT_UCSChar * pData;

	for (pBuf=buf, pData=data; (pData<data+length); /**/)
	{
		if (pBuf >= (buf+MY_BUFFER_SIZE-MY_HIGHWATER_MARK))
			FlushBuffer();

		UT_ASSERT(*pData < 256);		// TODO deal with unicode.  for now we assume latin-1.

		switch (*pData)
		{
		case '\\':
		case '{':
		case '}':
			*pBuf++ = '\\';
			*pBuf++ = (UT_Byte)*pData++;
			break;

		case UCS_LF:					// LF -- representing a Forced-Line-Break
			FlushBuffer();
			m_pie->_rtf_keyword("line");
			pData++;
			break;
			
		case UCS_VTAB:					// VTAB -- representing a Forced-Column-Break
			FlushBuffer();
			m_pie->_rtf_keyword("column");
			pData++;
			break;
			
		case UCS_FF:					// FF -- representing a Forced-Page-Break
			FlushBuffer();
			m_pie->_rtf_keyword("page");
			pData++;
			break;

		case UCS_NBSP:					// NBSP -- non breaking space
			FlushBuffer();
			m_pie->_rtf_keyword("~");
			pData++;
			break;

		case UCS_TAB:					// TAB -- a tab
			FlushBuffer();
			m_pie->_rtf_keyword("tab");
			pData++;
			break;

		default:
			if (*pData > 0x007f)
			{
				FlushBuffer();
				m_pie->_rtf_keyword_hex2("'",*pData);
				pData++;
			}
			else
			{
				*pBuf++ = (UT_Byte)*pData++;
			}
			break;
		}
	}

	FlushBuffer();
}

s_RTF_ListenerWriteDoc::s_RTF_ListenerWriteDoc(PD_Document * pDocument,
											   IE_Exp_RTF * pie,
											   UT_Bool bToClipboard)
{
	// The overall syntax for an RTF file is:
	//
	// <file> := '{' <header> <document> '}'
	//
	// We are responsible for <document>
	//
	// <document> := <info>? <docfmt>* <section>+
	
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInSpan = UT_FALSE;
	m_apiLastSpan = 0;
	m_apiThisSection = NULL;
	m_apiThisBlock = NULL;

	m_bToClipboard = bToClipboard;
	// when we are going to the clipboard, we should implicitly
	// assume that we are starting in the middle of a section
	// and block.  when going to a file we should not.
	m_bJustStartingDoc = !m_bToClipboard;
	m_bJustStartingSection = !m_bToClipboard;
	
	// TODO emit <info> if desired

	_rtf_docfmt();						// deal with <docfmt>

	// <section>+ will be handled by the populate code.
}

s_RTF_ListenerWriteDoc::~s_RTF_ListenerWriteDoc()
{
	_closeSpan();
	_closeBlock();
	_closeSection();
}

UT_Bool s_RTF_ListenerWriteDoc::populate(PL_StruxFmtHandle /*sfh*/,
									  const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

			PT_AttrPropIndex api = pcr->getIndexAP();
			_openSpan(api);
			
			PT_BufIndex bi = pcrs->getBufIndex();
			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

			return UT_TRUE;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
#if 0
			// TODO deal with inline objects....
			
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
			{
			case PTO_Image:
				_closeSpan();
				_openTag("image","/",UT_FALSE,api);
				return UT_TRUE;

			case PTO_Field:
				_closeSpan();
				_openTag("field","/",UT_FALSE,api);
				return UT_TRUE;

			default:
				UT_ASSERT(0);
				return UT_FALSE;
			}
#endif
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return UT_TRUE;
		
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

UT_Bool s_RTF_ListenerWriteDoc::populateStrux(PL_StruxDocHandle /*sdh*/,
										   const PX_ChangeRecord * pcr,
										   PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
		{
			_closeSpan();
			_closeBlock();
			_closeSection();

			// begin a new section.  in RTF this is expressed as
			//
			// <section> := <secfmt>* <hdrftr>? <para>+ (\sect <section>)?
			//
			// here we deal with everything except for the <para>+

			_rtf_open_section(pcr->getIndexAP());
			return UT_TRUE;
		}

	case PTX_Block:
		{
			_closeSpan();
			_closeBlock();
			_rtf_open_block(pcr->getIndexAP());
			return UT_TRUE;
		}

	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

UT_Bool s_RTF_ListenerWriteDoc::change(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);	// this function is not used.
	return UT_FALSE;
}

UT_Bool s_RTF_ListenerWriteDoc::insertStrux(PL_StruxFmtHandle /*sfh*/,
										  const PX_ChangeRecord * /*pcr*/,
										  PL_StruxDocHandle /*sdh*/,
										  PL_ListenerId /* lid */,
										  void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
																	  PL_ListenerId /* lid */,
																	  PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);	// this function is not used.
	return UT_FALSE;
}

UT_Bool s_RTF_ListenerWriteDoc::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);	// this function is not used.
	return UT_FALSE;
}

//////////////////////////////////////////////////////////////////

void s_RTF_ListenerWriteDoc::_rtf_docfmt(void)
{
	// emit everything necessary for <docfmt>* portion of the document

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	// <docfmt>
	
	const XML_Char * szDefaultTabs = PP_evalProperty("default-tab-interval",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,UT_TRUE);
	m_pie->_rtf_keyword_ifnotdefault_twips("deftab",szDefaultTabs,720);

	// <docfmt> -- document views and zoom level
	
	m_pie->_rtf_keyword("viewkind",1);	/* PageLayout */

	// TODO <docfmt> -- footnotes and endnotes

	// <docfmt> -- page information

	const XML_Char * szPaperWidth = "8.5in"; // TODO look this up in the document
	m_pie->_rtf_keyword_ifnotdefault_twips("paperw",szPaperWidth,0);
	const XML_Char * szPaperHeight = "11in"; // TODO look this up in the document
	m_pie->_rtf_keyword_ifnotdefault_twips("paperh",szPaperHeight,0);
	
	const XML_Char * szLeftMargin = PP_evalProperty("page-margin-left",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,UT_TRUE);
	m_pie->_rtf_keyword_ifnotdefault_twips("margl",szLeftMargin,1800);
	const XML_Char * szRightMargin = PP_evalProperty("page-margin-right",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,UT_TRUE);
	m_pie->_rtf_keyword_ifnotdefault_twips("margr",szRightMargin,1800);
	const XML_Char * szTopMargin = PP_evalProperty("page-margin-top",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,UT_TRUE);
	m_pie->_rtf_keyword_ifnotdefault_twips("margt",szTopMargin,1440);
	const XML_Char * szBottomMargin = PP_evalProperty("page-margin-Bottom",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,UT_TRUE);
	m_pie->_rtf_keyword_ifnotdefault_twips("margb",szBottomMargin,1440);
	m_pie->_rtf_keyword("widowctl");	// enable widow and orphan control
	
	// TODO <docfmt> -- linked styles
	// TODO <docfmt> -- compatibility options
	// TODO <docfmt> -- forms
	// TODO <docfmt> -- revision marks
	// TODO <docfmt> -- comments (annotations)
	// TODO <docfmt> -- bidirectional controls
	// TODO <docfmt> -- page borders
}

//////////////////////////////////////////////////////////////////

void s_RTF_ListenerWriteDoc::_rtf_open_section(PT_AttrPropIndex api)
{
	m_apiThisSection = api;

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);

	const XML_Char * szColumns = PP_evalProperty("columns",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,UT_TRUE);
	const XML_Char * szColumnGap = PP_evalProperty("column-gap",
												   pSpanAP,pBlockAP,pSectionAP,
												   m_pDocument,UT_TRUE);
		
	// TODO add other properties here

	m_pie->_rtf_nl();

	if (m_bJustStartingDoc)			// 'sect' is a delimiter, rather than a plain start
		m_bJustStartingDoc = UT_FALSE;
	else
		m_pie->_rtf_keyword("sect");							// begin a new section
	m_bJustStartingSection = UT_TRUE;

	m_pie->_rtf_keyword("sectd");								// restore all defaults for this section
	m_pie->_rtf_keyword("sbknone");								// no page break implied
	m_pie->_rtf_keyword_ifnotdefault("cols",szColumns,1);
	m_pie->_rtf_keyword_ifnotdefault_twips("colsx",szColumnGap,720);

}

//////////////////////////////////////////////////////////////////

class _t 
{
public:
	_t(const char * szTT, const char * szTK, UT_sint32 tp)
		{
			m_szTabTypeKeyword = szTT;
			m_szTabKindKeyword = szTK;
			m_iTabPosition = tp;
		}
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

void s_RTF_ListenerWriteDoc::_rtf_open_block(PT_AttrPropIndex api)
{
	m_apiThisBlock = api;

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);
	m_pDocument->getAttrProp(m_apiThisBlock,&pBlockAP);

	const XML_Char * szTextAlign = PP_evalProperty("text-align",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	const XML_Char * szFirstLineIndent = PP_evalProperty("text-indent",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	const XML_Char * szLeftIndent = PP_evalProperty("margin-left",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	const XML_Char * szRightIndent = PP_evalProperty("margin-right",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	const XML_Char * szTopMargin = PP_evalProperty("margin-top",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	const XML_Char * szBottomMargin = PP_evalProperty("margin-bottom",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	const XML_Char * szLineHeight = PP_evalProperty("line-height",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	const XML_Char * szKeepTogether = PP_evalProperty("keep-together",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	const XML_Char * szKeepWithNext = PP_evalProperty("keep-with-next",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	const XML_Char * szTabStops = PP_evalProperty("tabstops",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);

	// TODO add other properties here

	m_pie->_rtf_nl();

	if (m_bJustStartingSection)			// 'par' is a delimiter, rather than a plain start.
		m_bJustStartingSection = UT_FALSE;
	else
		m_pie->_rtf_keyword("par");		// begin a new paragraph
	
	m_pie->_rtf_keyword("pard");		// restore all defaults for this paragraph

	// if string is "left" use "ql", but that is the default, so we don't need to write it out.
	if (UT_stricmp(szTextAlign,"right")==0)		// output one of q{lrcj} depending upon paragraph alignment
		m_pie->_rtf_keyword("qr");
	else if (UT_stricmp(szTextAlign,"center")==0)
		m_pie->_rtf_keyword("qc");
	else if (UT_stricmp(szTextAlign,"justify")==0)
		m_pie->_rtf_keyword("qj");

	m_pie->_rtf_keyword_ifnotdefault_twips("fi",szFirstLineIndent,0);
	m_pie->_rtf_keyword_ifnotdefault_twips("li",szLeftIndent,0);
	m_pie->_rtf_keyword_ifnotdefault_twips("ri",szRightIndent,0);
	m_pie->_rtf_keyword_ifnotdefault_twips("sb",szTopMargin,0);
	m_pie->_rtf_keyword_ifnotdefault_twips("sa",szBottomMargin,0);

	if (UT_strcmp(szLineHeight,"1.0") != 0)
	{
		double f = UT_convertDimensionless(szLineHeight);
		if (f != 0.0)					// we get zero on bogus strings....
		{
			// don't ask me to explain the details of this conversion factor,
			// because i don't know....
			UT_sint32 dSpacing = (UT_sint32)(f * 240.0);
			m_pie->_rtf_keyword("sl",dSpacing);
			m_pie->_rtf_keyword("slmult",1);
		}
	}

	if (UT_stricmp(szKeepTogether,"yes")==0)
		m_pie->_rtf_keyword("keep");
	if (UT_stricmp(szKeepWithNext,"yes")==0)
		m_pie->_rtf_keyword("keepn");

	if (szTabStops && *szTabStops)
	{
		// write tabstops for this paragraph
		// TODO the following parser was copied from abi/src/text/fmt/xp/fl_BlockLayout.cpp
		// TODO we should extract both of them and share the code.

		unsigned char iType = 0;
		UT_sint32 iPosition = 0;
		UT_Vector vecTabs;
		
		const char* pStart = szTabStops;
		while (*pStart)
		{
			const char * szTT = "tx";	// TabType -- assume text tab (use "tb" for bar tab)
			const char * szTK = NULL;	// TabKind -- assume left tab
			
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
				case 'B':	szTT = "tb";	break; // TabKind == bar tab
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
			
			_t * p_t = new _t(szTT,szTK,d);
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
				m_pie->_rtf_keyword(p_t->m_szTabKindKeyword);
			// TODO write leader character in <tablead>
			// write tab type
			m_pie->_rtf_keyword(p_t->m_szTabTypeKeyword,p_t->m_iTabPosition);

			delete p_t;
		}
	}

}
