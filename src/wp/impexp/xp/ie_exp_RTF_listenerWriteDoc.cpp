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
#include <time.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_png.h"
#include "ut_bytebuf.h"
#include "ie_exp_RTF_listenerWriteDoc.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"

#include "xap_EncodingManager.h"
void s_RTF_ListenerWriteDoc::_closeSection(void)
{
	m_apiThisSection = 0;
	return;
}

void s_RTF_ListenerWriteDoc::_closeBlock(void)
{
	m_apiThisBlock = 0;
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
	UT_sint32 ndxColor = m_pie->_findColor((char*)szColor);
	UT_ASSERT(ndxColor != -1);
	if (ndxColor != 0)
		m_pie->_rtf_keyword("cf",ndxColor);

   	_rtf_font_info fi(pSpanAP,pBlockAP,pSectionAP);
	UT_sint32 ndxFont = m_pie->_findFont(&fi);
	UT_ASSERT(ndxFont != -1);
	m_pie->_rtf_keyword("f",ndxFont);	// font index in fonttbl

	const XML_Char * szFontSize = PP_evalProperty("font-size",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	double dbl = UT_convertToPoints(szFontSize);
	UT_sint32 d = (UT_sint32)(dbl*2.0);
	if (d != 24)
		m_pie->_rtf_keyword("fs",d);	// font size in half points

	const XML_Char * szFontStyle = PP_evalProperty("font-style",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	if (szFontStyle && *szFontStyle && (UT_strcmp(szFontStyle,"italic")==0))
		m_pie->_rtf_keyword("i");

	const XML_Char * szFontWeight = PP_evalProperty("font-weight",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	if (szFontWeight && *szFontWeight && (UT_strcmp(szFontWeight,"bold")==0))
		m_pie->_rtf_keyword("b");

	const XML_Char * szFontDecoration = PP_evalProperty("text-decoration",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	if (szFontDecoration && *szFontDecoration)
	{
		if (strstr(szFontDecoration,"underline") != 0)
			m_pie->_rtf_keyword("ul");
		if (strstr(szFontDecoration,"overline") != 0)
			m_pie->_rtf_keyword("ol");
		if (strstr(szFontDecoration,"line-through") != 0)
			m_pie->_rtf_keyword("strike");
	}

	const XML_Char * szFontPosition = PP_evalProperty("text-position",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	if (szFontPosition && *szFontPosition)
	{
		if (!UT_strcmp(szFontPosition,"superscript"))
			m_pie->_rtf_keyword("super");
		else if (!UT_strcmp(szFontPosition,"subscript"))
			m_pie->_rtf_keyword("sub");
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
	char mbbuf[30];
	int mblen;

	for (pBuf=buf, pData=data; (pData<data+length); /**/)
	{
		if (pBuf >= (buf+MY_BUFFER_SIZE-MY_HIGHWATER_MARK))
			FlushBuffer();

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
			if (XAP_EncodingManager::instance->cjk_locale())
			{
				/*FIXME: can it happen that wctomb will fail under CJK locales? */
				m_wctomb.wctomb_or_fallback(mbbuf,mblen,*pData++);
				if (mbbuf[0] & 0x80)
				{
					FlushBuffer();
					for(int i=0;i<mblen;++i) {
						unsigned char c = mbbuf[i];
						m_pie->_rtf_nonascii_hex2(c);
					}
				}
				else
				{
					for(int i=0;i<mblen;++i) {
						switch (mbbuf[i]) 
						{
							case '\\':
							case '{':
							case '}':
								*pBuf++ = '\\';
						}
						*pBuf++ = mbbuf[i];
					}
				}
			} else if (!m_pie->m_atticFormat) 
			{
				if (*pData > 0x00ff)		// emit unicode character
				{
					FlushBuffer();

					// RTF spec says that we should emit an ASCII-equivalent
					// character for each unicode character, so that dumb/older
					// readers don't lose a char.  i don't have a good algorithm
					// for deciding how to do this, so i'm not going to put out
					// any chars.  so i'm setting \uc0 before emitting \u<u>.
					// TODO decide if we should be smarter here and do a \uc1\u<u><A> ??
					// TODO if so, we may need to begin a sub-brace level to avoid
					// TODO polluting the global context w/r/t \uc.
					
					UT_UCSChar lc = XAP_EncodingManager::instance->try_UToWindows(*pData);
					m_pie->_rtf_keyword("uc",lc && lc<256 ? 1 : 0);
					unsigned short ui = ((unsigned short)(*pData));	// RTF is limited to +/-32K ints
					signed short si = *((signed short *)(&ui));		// so we need to write negative
					m_pie->_rtf_keyword("u",si);					// numbers for large unicode values.
					if (lc && lc <256)
						m_pie->_rtf_nonascii_hex2(lc);
					pData++;
				}
				else if (*pData > 0x007f)
				{
					FlushBuffer();

					// for chars between 7f and ff, we could just send them
					// out as is, or we could send them out in hex or as a
					// unicode sequence.  when i originally did this, i chose
					// hex, so i'm not going to change it now.
				
					m_pie->_rtf_nonascii_hex2(*pData);
					pData++;
				}
				else
				{
					*pBuf++ = (UT_Byte)*pData++;
				}
			} else {
				/* 
				    wordpad (and probably word6/7 don't understand
				    \uc0\u<UUUU> format at all.
				*/
				UT_UCSChar c = *pData++;
				UT_UCSChar lc = XAP_EncodingManager::instance->try_UToWindows(c);
				if (lc==0 || lc >255) 
				{
					/*
					    can't be represented in windows encoding. 
					    So emit unicode (though attic apps won't understand it.
					    This branch is shamelessly copied from
					    branch if (*pData > 0x00ff) above.
					*/
					FlushBuffer();

					// RTF spec says that we should emit an ASCII-equivalent
					// character for each unicode character, so that dumb/older
					// readers don't lose a char.  i don't have a good algorithm
					// for deciding how to do this, so i'm not going to put out
					// any chars.  so i'm setting \uc0 before emitting \u<u>.
					// TODO decide if we should be smarter here and do a \uc1\u<u><A> ??
					// TODO if so, we may need to begin a sub-brace level to avoid
					// TODO polluting the global context w/r/t \uc.
					
					m_pie->_rtf_keyword("uc",0);
					unsigned short ui = ((unsigned short)(*pData));	// RTF is limited to +/-32K ints
					signed short si = *((signed short *)(&ui));		// so we need to write negative
					m_pie->_rtf_keyword("u",si);					// numbers for large unicode values.
				}
				else 
				{
					if (lc > 0x007f) 
					{
						FlushBuffer();

						// for chars between 7f and ff, we could just send them
						// out as is, or we could send them out in hex or as a
						// unicode sequence.  when i originally did this, i chose
						// hex, so i'm not going to change it now.
				
						m_pie->_rtf_nonascii_hex2(lc);
					}
					else
					{
						*pBuf++ = (UT_Byte)lc;
					}
				}
			};
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
	m_apiThisSection = 0;
	m_apiThisBlock = 0;

	m_bToClipboard = bToClipboard;
	// when we are going to the clipboard, we should implicitly
	// assume that we are starting in the middle of a section
	// and block.  when going to a file we should not.
	m_bJustStartingDoc = !m_bToClipboard;
	m_bJustStartingSection = !m_bToClipboard;

	m_wctomb.setOutCharset(XAP_EncodingManager::instance->WindowsCharsetName());
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
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
		       switch (pcro->getObjectType())
			{
			case PTO_Image:
				_closeSpan();
				_writeImageInRTF(pcro);
				return UT_TRUE;

				//#if 0
			// TODO deal with these other inline objects....
			
			case PTO_Field:
				_closeSpan();
				_openTag("field","/",UT_FALSE,api);
				return UT_TRUE;

				//#endif
			default:
				return UT_FALSE;
			}
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return UT_TRUE;
		
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}


void	 s_RTF_ListenerWriteDoc::_openTag(const char * szPrefix, const char * szSuffix,
								 UT_Bool bNewLineAfter, PT_AttrPropIndex api)
{
  UT_DEBUGMSG(("TODO: Write code to go in here. In _openTag, szPrefix = %s  api = %x \n",szPrefix,api));
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
	m_pie->_rtf_keyword_ifnotdefault_twips("deftab",(char*)szDefaultTabs,720);

	// <docfmt> -- document views and zoom level
	
	m_pie->_rtf_keyword("viewkind",1);	/* PageLayout */

	// TODO <docfmt> -- footnotes and endnotes

	// <docfmt> -- page information

	const XML_Char * szPaperWidth = "8.5in"; // TODO look this up in the document
	m_pie->_rtf_keyword_ifnotdefault_twips("paperw",(char*)szPaperWidth,0);
	const XML_Char * szPaperHeight = "11in"; // TODO look this up in the document
	m_pie->_rtf_keyword_ifnotdefault_twips("paperh",(char*)szPaperHeight,0);
	
	const XML_Char * szLeftMargin = PP_evalProperty("page-margin-left",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,UT_TRUE);
	m_pie->_rtf_keyword_ifnotdefault_twips("margl",(char*)szLeftMargin,1800);
	const XML_Char * szRightMargin = PP_evalProperty("page-margin-right",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,UT_TRUE);
	m_pie->_rtf_keyword_ifnotdefault_twips("margr",(char*)szRightMargin,1800);
	const XML_Char * szTopMargin = PP_evalProperty("page-margin-top",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,UT_TRUE);
	m_pie->_rtf_keyword_ifnotdefault_twips("margt",(char*)szTopMargin,1440);
	const XML_Char * szBottomMargin = PP_evalProperty("page-margin-Bottom",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,UT_TRUE);
	m_pie->_rtf_keyword_ifnotdefault_twips("margb",(char*)szBottomMargin,1440);
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
	m_pie->_rtf_keyword_ifnotdefault("cols",(char*)szColumns,1);
	m_pie->_rtf_keyword_ifnotdefault_twips("colsx",(char*)szColumnGap,720);

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

	// Do abi specific list information.

	const XML_Char * szListid=NULL;
	const XML_Char * szParentid=NULL;
	const XML_Char * szLevel=NULL;
	const XML_Char * szListStyle=NULL;

	if (!pBlockAP || !pBlockAP->getAttribute((const XML_Char*)"listid", szListid))
		szListid = NULL;
	if (!pBlockAP || !pBlockAP->getAttribute((const XML_Char*)"parentid", szParentid))
		szParentid = NULL;
	if (!pBlockAP || !pBlockAP->getAttribute((const XML_Char*)"level", szLevel))
		szLevel = NULL;
	if (!pBlockAP || !pBlockAP->getAttribute((const XML_Char*)"style", szListStyle))
		szListStyle = NULL;
	const XML_Char * szAbiListDecimal = PP_evalProperty("list-decimal",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	const XML_Char * szAbiStartValue = PP_evalProperty("start-value",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	const XML_Char * szAbiFieldFont = PP_evalProperty("field-font",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);
	const XML_Char * szAbiListDelim = PP_evalProperty("list-delim",pSpanAP,pBlockAP,pSectionAP,m_pDocument,UT_TRUE);

	m_pie->_rtf_nl();

	if (m_bJustStartingSection)			// 'par' is a delimiter, rather than a plain start.
		m_bJustStartingSection = UT_FALSE;
	else
		m_pie->_rtf_keyword("par");		// begin a new paragraph
	
	m_pie->_rtf_keyword("pard");		// restore all defaults for this paragraph


	///
	/// OK if there is list info in this paragraph we encase it inside
	/// the {\*\abilist..} extension
	///
	if(szListid != NULL)
	{
	        m_pie->_rtf_open_brace();
	        m_pie->_rtf_keyword("*");
	        m_pie->_rtf_keyword("abilist");
		m_pie->_rtf_keyword_ifnotdefault("abilistid",(char *) szListid,-1);
		m_pie->_rtf_keyword_ifnotdefault("abilistparentid",(char *) szParentid,-1);
		m_pie->_rtf_keyword_ifnotdefault("abilistlevel",(char *) szLevel,-1);
		m_pie->_rtf_keyword_ifnotdefault("abistartat",(char *) szAbiStartValue,-1);
		/// field font

	        m_pie->_rtf_open_brace();
	        m_pie->_rtf_keyword("abifieldfont");
	        m_pie->_rtf_chardata( (const char *) szAbiFieldFont ,strlen(szAbiFieldFont));
		m_pie->_rtf_close_brace();

		/// list decimal
	        
		m_pie->_rtf_open_brace();
	        m_pie->_rtf_keyword("abilistdecimal");
	        m_pie->_rtf_chardata((const char *)  szAbiListDecimal ,strlen(szAbiListDecimal));
		m_pie->_rtf_close_brace();

		/// list delim
	        
		m_pie->_rtf_open_brace();
	        m_pie->_rtf_keyword("abilistdelim");
	        m_pie->_rtf_chardata((const char *)  szAbiListDelim ,strlen( szAbiListDelim));
		m_pie->_rtf_close_brace();

		/// list style
	        
		m_pie->_rtf_open_brace();
	        m_pie->_rtf_keyword("abiliststyle");
	        m_pie->_rtf_chardata((const char *)  szListStyle ,strlen( szListStyle));
		m_pie->_rtf_close_brace();
		
		/// Finished!
		
		m_pie->_rtf_close_brace();

	}



	// if string is "left" use "ql", but that is the default, so we don't need to write it out.
	if (UT_strcmp(szTextAlign,"right")==0)		// output one of q{lrcj} depending upon paragraph alignment
		m_pie->_rtf_keyword("qr");
	else if (UT_strcmp(szTextAlign,"center")==0)
		m_pie->_rtf_keyword("qc");
	else if (UT_strcmp(szTextAlign,"justify")==0)
		m_pie->_rtf_keyword("qj");

	m_pie->_rtf_keyword_ifnotdefault_twips("fi",(char*)szFirstLineIndent,0);
	m_pie->_rtf_keyword_ifnotdefault_twips("li",(char*)szLeftIndent,0);
	m_pie->_rtf_keyword_ifnotdefault_twips("ri",(char*)szRightIndent,0);
	m_pie->_rtf_keyword_ifnotdefault_twips("sb",(char*)szTopMargin,0);
	m_pie->_rtf_keyword_ifnotdefault_twips("sa",(char*)szBottomMargin,0);

	if (strcmp(szLineHeight,"1.0") != 0)
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

	if (UT_strcmp(szKeepTogether,"yes")==0)
		m_pie->_rtf_keyword("keep");
	if (UT_strcmp(szKeepWithNext,"yes")==0)
		m_pie->_rtf_keyword("keepn");

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

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void s_RTF_ListenerWriteDoc::_writeImageInRTF(const PX_ChangeRecord_Object * pcro)
{
	PT_AttrPropIndex api = pcro->getIndexAP();
	const PP_AttrProp * pImageAP = NULL;
	m_pDocument->getAttrProp(api,&pImageAP);

	// fetch the "name" of the image and use it to fetch the actual image data.
	
	const XML_Char * szDataID = NULL;
	UT_Bool bFoundDataID = pImageAP->getAttribute((XML_Char*)"dataid",szDataID);
	if (!bFoundDataID)
	{
		UT_DEBUGMSG(("RTF_Export: cannot get dataid for image\n"));
		return;
	}
	const UT_ByteBuf * pbb = NULL;
	void * pToken = NULL;
	void * pHandle = NULL;
	UT_Bool bFoundDataItem = m_pDocument->getDataItemDataByName((char*)szDataID,&pbb,&pToken,&pHandle);
	if (!bFoundDataItem)
	{
		UT_DEBUGMSG(("RTF_Export: cannot get dataitem for image\n"));
		return;
	}
	
	// see if the image has a width/height attribute that should
	// override the actual pixel size of the image.
	
	const XML_Char * szWidthProp = NULL;
	const XML_Char * szHeightProp = NULL;
	UT_Bool bFoundWidthProperty = pImageAP->getProperty((XML_Char*)"width",szWidthProp);
	UT_Bool bFoundHeightProperty = pImageAP->getProperty((XML_Char*)"height",szHeightProp);

	// get the width/height of the image from the image itself.

	UT_sint32 iImageWidth, iImageHeight;
	UT_PNG_getDimensions(pbb,iImageWidth,iImageHeight);

	// TODO compute scale factors...

	// if everything is ok, we need to dump the image data (in hex)
	// to the RTF stream with some screwy keywords...
	//
	// we need to emit:     {\*\shppict{\pict <stuff>}}
	// we do not deal with: {\*\nonshppict...}
	//
	// <stuff> ::= <brdr>? <shading>? <pictype> <pictsize> <metafileinfo>? <data>

	m_pie->_rtf_open_brace();
	{
		m_pie->_rtf_keyword("*");
		m_pie->_rtf_keyword("shppict");
		m_pie->_rtf_open_brace();
		{
			m_pie->_rtf_keyword("pict");
			// TODO deal with <brdr>
			// TODO deal with <shading>

			// <pictype> -- we store everything internall as PNG, so that's all
			//              we output here.  TODO consider listing multiple formats
			//              here -- word97 seems to, but this really bloats the file.

			m_pie->_rtf_keyword("pngblip");
			
			// <pictsize>

			m_pie->_rtf_keyword("picw",iImageWidth);
			m_pie->_rtf_keyword("pich",iImageHeight);
			if (bFoundWidthProperty)
				m_pie->_rtf_keyword_ifnotdefault_twips("picwgoal",(char*)szWidthProp,0);
			if (bFoundHeightProperty)
				m_pie->_rtf_keyword_ifnotdefault_twips("pichgoal",(char*)szHeightProp,0);
			// we use the default values for picscale[xy]==100, piccrop[tblr]==0
			
			// TODO deal with <metafileinfo>

			// <data>

			// TODO create meaningful values for bliptag and bliduid...
			// we emit "\bliptag<N>{\*\blipuid <N16>}"
			// where <N> is an integer.
			// where <N16> is a 16-byte integer in hex.

			m_pie->_rtf_nl();
			UT_uint32 tag = time(NULL);
			m_pie->_rtf_keyword("bliptag",tag);
			m_pie->_rtf_open_brace();
			{
				m_pie->_rtf_keyword("*");
				m_pie->_rtf_keyword("blipuid");
				char buf[100];
				sprintf(buf,"%032x",tag);
				m_pie->_rtf_chardata(buf,strlen(buf));
			}
			m_pie->_rtf_close_brace();

			UT_uint32 lenData = pbb->getLength();
			const UT_Byte * pData = pbb->getPointer(0);
			UT_uint32 k;

			for (k=0; k<lenData; k++)
			{
				if (k%32==0)
					m_pie->_rtf_nl();
				char buf[10];
				sprintf(buf,"%02x",pData[k]);
				m_pie->_rtf_chardata(buf,2);
			}
		}
		m_pie->_rtf_close_brace();
	}
	m_pie->_rtf_close_brace();
}
