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
#include <locale.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_png.h"
#include "ut_bytebuf.h"
#include "ut_math.h"
#include "ie_exp_RTF_listenerWriteDoc.h"
#include "ie_exp_RTF_AttrProp.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "pt_Types.h"
#include "fl_AutoNum.h"
#include "fl_AutoLists.h"
#include "fl_BlockLayout.h"
#include "fp_Run.h"
#include "fl_Layout.h"
#include "fl_TableLayout.h"

#include "xap_EncodingManager.h"
#include "ut_string_class.h"

void s_RTF_ListenerWriteDoc::_closeSection(void)
{
	m_apiThisSection = 0;
	m_sdh = NULL;
	return;
}

void s_RTF_ListenerWriteDoc::_closeBlock(PT_AttrPropIndex  nextApi)
{
//
// Force the output of char properties for blank lines or list items.
//
	bool bInList = false;
	const XML_Char * szListid=NULL;
	const PP_AttrProp * pBlockAP = NULL;
	xxx_UT_DEBUGMSG(("SEVIOR: Close Block \n"));
	if(nextApi != 0)
	{
		m_pDocument->getAttrProp(nextApi,&pBlockAP);

		if (!pBlockAP || !pBlockAP->getAttribute((const XML_Char*)"listid", szListid))
		{
			szListid = NULL;
		}
		if(szListid != NULL)
		{
			bInList = false; // was true
		}
	}

 	if(!m_bInSpan && m_sdh && (m_bBlankLine || bInList) && !m_bJustStartingSection && m_pDocument->getStruxType(m_sdh) == PTX_Block )
  	{
//
// This is a blankline or list item
//
// output the character properties for this break.
//
		const PP_AttrProp * pSpanAP = NULL;
		m_pDocument->getSpanAttrProp(m_sdh,0,true,&pSpanAP);
		xxx_UT_DEBUGMSG(("SEVIOR: Close Block -open span \n"));
		_openSpan(m_apiThisBlock,pSpanAP);
	}
	else
	{
		m_bBlankLine = false;
	}
	m_apiThisBlock = 0;
	m_sdh = NULL;
	return;
}

void s_RTF_ListenerWriteDoc::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	m_pie->_rtf_close_brace();
	m_bInSpan = false;
	return;
}

void s_RTF_ListenerWriteDoc::_openSpan(PT_AttrPropIndex apiSpan,  const PP_AttrProp * pInSpanAP)
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
	if(pInSpanAP == NULL)
	{
		m_pDocument->getAttrProp(apiSpan,&pSpanAP);
	}
	else
	{
		pSpanAP = pInSpanAP;
	}

	m_pie->_write_charfmt(s_RTF_AttrPropAdapter_AP(pSpanAP, pBlockAP, pSectionAP, m_pDocument));
	m_bBlankLine = false;
	m_bInSpan = true;
	m_apiLastSpan = apiSpan;
}

void s_RTF_ListenerWriteDoc::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	UT_String sBuf;
	const UT_UCSChar * pData;
	char mbbuf[30];
	int mblen;

	#define FlushBuffer() do {m_pie->_rtf_chardata(sBuf.c_str(), sBuf.size()); sBuf.clear();} while (0)

	UT_ASSERT(sizeof(UT_Byte) == sizeof(char));

	for (pData=data; (pData<data+length); /**/)
	{
		switch (*pData)
		{
		case '\\':
		case '{':
		case '}':
			sBuf += '\\';
			sBuf += (char)*pData++;
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
			if (XAP_EncodingManager::get_instance()->cjk_locale())
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
								sBuf += '\\';
						}
						sBuf += mbbuf[i];
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

					UT_UCSChar lc = XAP_EncodingManager::get_instance()->try_UToWindows(*pData);
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
					sBuf += (char)*pData++;
				}
			} else {
				/*
				    wordpad (and probably word6/7) don't understand
				    \uc0\u<UUUU> format at all.
				*/
				UT_UCSChar c = *pData++;
				UT_UCSChar lc = XAP_EncodingManager::get_instance()->try_UToWindows(c);
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
						sBuf += (char)lc;
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
											   bool bToClipboard)
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
	m_bInSpan = false;
	m_apiLastSpan = 0;
	m_apiThisSection = 0;
	m_apiThisBlock = 0;
	m_sdh = NULL;
	m_bToClipboard = bToClipboard;
	m_bStartedList = false;
	m_bBlankLine = false;
	m_Table.setDoc(m_pDocument);
	m_iCurRow = -1;
	m_bNewTable = false;
	m_iLeft = -1;
	m_iRight = -1;
	m_iTop = -1;
	m_iBot = -1;
	_setTabEaten(false);
	_setListBlock(false);

	// when we are going to the clipboard, we should implicitly
	// assume that we are starting in the middle of a section
	// and block.  when going to a file we should not.
	m_bJustStartingDoc = !m_bToClipboard;
	m_bJustStartingSection = !m_bToClipboard;

	m_wctomb.setOutCharset(XAP_EncodingManager::get_instance()->WindowsCharsetName());
	// TODO emit <info> if desired
	m_currID = 0;
	_rtf_info ();
	_rtf_docfmt();						// deal with <docfmt>

	// <section>+ will be handled by the populate code.
}

s_RTF_ListenerWriteDoc::~s_RTF_ListenerWriteDoc()
{
	_closeSpan();
	_closeBlock();
	_closeSection();
}

bool s_RTF_ListenerWriteDoc::populate(PL_StruxFmtHandle /*sfh*/,
									  const PX_ChangeRecord * pcr)
{
	m_posDoc = pcr->getPosition();
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

			PT_AttrPropIndex api = pcr->getIndexAP();

			PT_BufIndex bi = pcrs->getBufIndex();
			const UT_UCSChar * pData = m_pDocument->getPointer(bi);
//
// Code to deal with the tab following a list label. Eat it!!
//
			UT_uint32 length = pcrs->getLength();
			if(_isListBlock() && !_isTabEaten())
			{
				if(*pData == UCS_TAB)
				{
					_setTabEaten(true);
					pData++;
					length--;
					if(length == 0)
						return true;
				}
			}
			_openSpan(api);
			_outputData(pData,length);

			return true;
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
				return true;

				//#if 0
			// TODO deal with these other inline objects....

			case PTO_Field:
				_closeSpan();
				_openTag("field","/",false,api);
				return true;

				//#endif

			case PTO_Bookmark:
				_closeSpan ();
				_writeBookmark(pcro);
				return true;
			case PTO_Hyperlink:
				_closeSpan ();
				_writeHyperlink(pcro);
			    return true;

			default:
				return false;
			}
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;

	default:
		UT_ASSERT_NOT_REACHED();
		return false;
	}
}

/*!
 * This method writes out the all the boiler plate needed before every field
 * definition.
 */
void s_RTF_ListenerWriteDoc::_writeFieldPreamble(const PP_AttrProp * pSpanAP)
{
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);
	m_pDocument->getAttrProp(m_apiThisBlock,&pBlockAP);
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("field");
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("*");
	m_pie->_rtf_keyword("fldinst");
	m_pie->write(" ");
	m_pie->_rtf_open_brace();
	m_pie->_write_charfmt(s_RTF_AttrPropAdapter_AP(pSpanAP, pBlockAP, pSectionAP, m_pDocument));
	m_pie->write(" ");
}


/*!
 * This method writes out the current field value and closes all the braces.
 */
void s_RTF_ListenerWriteDoc::_writeFieldTrailer(void)
{
	const UT_UCSChar * szFieldValue = _getFieldValue();
	if(szFieldValue == NULL)
	{
		m_pie->_rtf_close_brace();
		return;
	}
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("fldrslt");
	m_pie->write(" ");
	m_pie->_rtf_open_brace();
	m_pie->_rtf_keyword("noproof");
	m_pie->write(" ");
	UT_uint32 len = UT_UCS4_strlen(szFieldValue);
	_outputData(szFieldValue,len);
	m_pie->_rtf_close_brace();
	m_pie->_rtf_close_brace();
	m_pie->_rtf_close_brace();
}

/*!
 * This method returns the field value at the current document location.
 * If there is not a field at the current document location return NULL.
 */
const UT_UCSChar * s_RTF_ListenerWriteDoc::_getFieldValue(void)
{
//
// Grab the first format handle in the PieceTable and turn it into a layout class.
// Check that is it a block.
//
	if(m_sdh == NULL)
	{
		m_pDocument->getStruxOfTypeFromPosition(m_posDoc,PTX_Block,&m_sdh);
	}
	PL_StruxFmtHandle sfh = m_pDocument->getNthFmtHandle(m_sdh,0);
	fl_Layout * pL = (fl_Layout *) sfh;
	if(pL->getType() != PTX_Block)
	{
	  UT_return_val_if_fail(0, NULL);
	}
	fl_BlockLayout* pBL = (fl_BlockLayout *) pL;
	bool bDirection;
	UT_sint32 x, y, x2, y2, height;
	fp_Run * pRun = pBL->findPointCoords(m_posDoc,false,x,y,x2,y2,height,bDirection);
//
// Check the run to make sure it is a field.
//
	while(pRun && pRun->getType() == FPRUN_FMTMARK)
	{
		pRun = pRun->getNext();
	}
	if((pRun== NULL) || pRun->getType() != FPRUN_FIELD )
	{
	  UT_return_val_if_fail(0, NULL);
	}
//
// Now get the value of this field
//
	return ((fp_FieldRun *) pRun)->getValue();
}

void	 s_RTF_ListenerWriteDoc::_openTag(const char * szPrefix, const char * szSuffix,
								 bool bNewLineAfter, PT_AttrPropIndex api)
{
	 xxx_UT_DEBUGMSG(("TODO: Write code to go in here. In _openTag, szPrefix = %s  szSuffix = %s api = %x \n",szPrefix,szSuffix,api));
	 if(UT_XML_strcmp(szPrefix,"field") == 0)
	 {
		 const PP_AttrProp * pSpanAP = NULL;
		 const XML_Char * pszType = NULL;
		 m_pDocument->getAttrProp(api, &pSpanAP);
		 pSpanAP->getAttribute("type", pszType);
		 if(UT_XML_strcmp(pszType,"list_label") == 0)
		 {
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"page_number") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("PAGE ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"time") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"page_ref") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDpageDref"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"page_count") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("NUMPAGES ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"date") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"dddd, MMMM dd, yyyy\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"date_mmddyy") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("DATE ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"date_ddmmyy") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" ""m/d/yy"" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"date_mdy") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"MMMM d, yyyy\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"date_mthdy") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"MMM d, yy\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"date_dfl") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("SAVEDATE  ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"date_ntdfl") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"MM-d-yy\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"date_wkday") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"dddd\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"date_doy") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDdateDdoy"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"time_miltime") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"HH:mm:ss\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"time_ampm") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("TIME  \\");
			 m_pie->_rtf_keyword("@");
			 m_pie->write(" \"h:mm:ss am/pm\" ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"time_zone") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDtimeDzone"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"time_epoch") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDtimeDepoch"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"word_count") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
			 m_pie->write("NUMWORDS ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"char_count") == 0)
		 {
			 _writeFieldPreamble(pSpanAP);
  			 m_pie->write("NUMCHARS  ");
             m_pie->_rtf_close_brace();
             m_pie->_rtf_close_brace();
			 _writeFieldTrailer();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"line_count") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDlineDcount"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"para_count") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDparaDcount"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 UT_DEBUGMSG(("SEVIOR: paragraph count field here \n"));
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"nbsp_count") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDnbspDcount"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"file_name") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDfileDname"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 UT_DEBUGMSG(("SEVIOR: File Name field here \n"));
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"app_ver") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDappDver"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"app_id") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDappDid"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 UT_DEBUGMSG(("SEVIOR: Application ID field here \n"));
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"app_options") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDappDoptions"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"app_target") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDappDtarget"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"app_compiledate") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDappDcompiledate"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else if(UT_XML_strcmp(pszType,"app_compiletime") == 0)
		 {
             m_pie->_rtf_open_brace();
			 m_pie->_rtf_keyword("*");
			 m_pie->_rtf_keyword("abifieldDappDcompiletime"); // abiword extension for now.
             m_pie->_rtf_close_brace();
			 return;
		 }
		 else
		 {
			 UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			 return;
		 }
	 }
}

void s_RTF_ListenerWriteDoc::_open_cell(PT_AttrPropIndex api)
{
	UT_sint32 iOldRow = m_iTop;
	UT_sint32 i =0;
	m_Table.OpenCell(api);
	bool bNewRow = false;
	UT_DEBUGMSG(("iOldRow %d newTop %d \n",iOldRow,m_Table.getTop()));
	if(	m_Table.getTop() != iOldRow)
	{
		if(m_bNewTable)
		{
			m_pie->_rtf_open_brace();
			_newRow();
		}
		else
		{
			bNewRow = true;
			m_pie->_rtf_keyword("row");
			_newRow();
		}
	}
//
// reset api. It may have been screwed in _newRow
//
	m_Table.OpenCell(api);
	if(bNewRow)
	{
//
// Output cell markers for all vertically merged cells at the start of the row
//
		if(m_Table.getNestDepth() < 2)
		{
			for(i = 0; i < m_Table.getLeft(); i++)
			{
				m_pie->_rtf_keyword("cell");
			}
		}
		else
		{
			for(i = 0; i < m_Table.getLeft(); i++)
			{
				UT_DEBUGMSG(("Writing nestcell in wrong spot 1 \n"));
				m_pie->_rtf_keyword("nestcell");
			}
		}
	}
//
// Now output vertically merged cell markers between the last right position and this cell's left.
//
	else
	{
		if(!m_bNewTable)
		{
			if(m_Table.getNestDepth() < 2)
			{
				for(i = m_iRight; i < m_Table.getLeft(); i++)
				{
					m_pie->_rtf_keyword("cell");
				}
			}
			else
			{
				for(i = m_iRight; i < m_Table.getLeft(); i++)
				{
					UT_DEBUGMSG(("Writing nestcell in wrong spot 2 \n"));
					m_pie->_rtf_keyword("nestcell");
				}
			}
		}
	}
	m_bNewTable = false;
	m_iLeft = m_Table.getLeft();
	m_iRight = m_Table.getRight();
	m_iTop = m_Table.getTop();
	m_iBot = m_Table.getBot();
}

void s_RTF_ListenerWriteDoc::_newRow(void)
{
	UT_sint32 i;
	m_pie->_rtf_nl();
	if((m_Table.getNestDepth() > 1) && m_bNewTable)
	{
		m_pie->_rtf_open_brace();
		m_pie->_rtf_keyword("*");
		m_pie->_rtf_keyword("nesttableprops");
	}
	m_pie->_rtf_keyword("trowd");
	m_pie->write(" ");
//
// Set spacing between cells
//
	const char * szColSpace = m_Table.getTableProp("table-col-spacing");
	if(szColSpace && *szColSpace)
	{
		double dspace = UT_convertToInches(szColSpace) * 360.0;
		UT_sint32 iSpace =0;
		iSpace = (UT_sint32) dspace;
		m_pie->_rtf_keyword("trgaph",iSpace);
	}
	else
	{
		m_pie->_rtf_keyword("trgaph",36);
		szColSpace = "0.05in";
	}
	double dColSpace = UT_convertToInches(szColSpace);
//
// Hardwire left-justification (for now)
//
	m_pie->_rtf_keyword("trql");
//
// Height of row. Hardwired to zero (take maximum cell height for row) for now.
//
	m_pie->_rtf_keyword("trrh",0);
//
// Lookup column positions.
//
	const char * szColumnProps = NULL;
	const char * szColumnLeftPos = NULL;
	szColumnProps = m_Table.getTableProp("table-column-props");
	szColumnLeftPos = m_Table.getTableProp("table-column-leftpos");
	double cellLeftPos = 0;
	if(szColumnLeftPos && *szColumnLeftPos)
	{
		cellLeftPos = UT_convertToInches(szColumnLeftPos);
	}
	UT_sint32 iLeftTwips = 0;
	iLeftTwips =  (UT_sint32) (cellLeftPos*720.0);
	m_pie->_rtf_keyword("trleft",iLeftTwips);
	UT_Vector vecColProps;
	vecColProps.clear();
	if(szColumnProps && *szColumnProps)
	{
		UT_String sProps = szColumnProps;
		UT_sint32 sizes = sProps.size();
		i =0 ;
		UT_sint32 j =0;
		while(i < sizes)
		{
			for (j=i; (j<sizes) && (sProps[j] != '/') ; j++) {}
			if((j+1)>i && sProps[j] == '/')
			{
				char * pszSub = UT_strdup(sProps.substr(i,(j-i)).c_str());
				i = j + 1;
				double colWidth = UT_convertToInches(pszSub)* 10000.0;
				fl_ColProps * pColP = new fl_ColProps;
				pColP->m_iColWidth = (UT_sint32) colWidth;
				vecColProps.addItem((void *) pColP);
				delete [] pszSub;
			}
		}
	}
	else
	{
//
// Autofit (or not) the row. Look up col widths
//
		m_pie->_rtf_keyword("trautofit",1);
	}
//
// Handle table line types.
//
	const char * szLineThick = m_Table.getTableProp("table-line-thickness");
	UT_sint32 iThick = -1;
	if(szLineThick && *szLineThick)
	{
		iThick = atoi(szLineThick);
		if(iThick > 0)
		{
			_outputTableBorders(iThick);
		}
	}
	else
	{
		_outputTableBorders(1);
	}
//
// OK now output all the cell properties, including merged cell controls.
//
	UT_sint32 row = m_Table.getTop();
	UT_sint32 col = m_Table.getLeft();
	double cellpos = cellLeftPos + dColSpace*0.5;
	double colwidth = 0.0;
	double dcells = (double) m_Table.getNumCols();
	colwidth = (_getColumnWidthInches() - dColSpace*0.5)/dcells;
	for(i=0; i < m_Table.getNumCols(); i = m_Table.getRight())
	{
		m_Table.setCellRowCol(row,i);
		UT_DEBUGMSG(("SEVIOR: set to row %d i %d left %d top %d \n",row,i,m_Table.getLeft(),m_Table.getRight()));
		m_pie->_rtf_keyword("clvertalt"); // Top aligned vertical alignment. ONly one for now
		if(iThick > 0)
		{
			_outputCellBorders(iThick);
		}
		else if(iThick < 0)
		{
			_outputCellBorders(1);
		}
		m_pie->_rtf_keyword("cltxlrtb"); // Text flow left to right, top to bottom
		                                 // Hardwired for now.
//
// Look if we have a vertically merged cell at this (row,i)
//
		bool vMerge = false;
		if(m_Table.getTop() < row)
		{
			m_pie->_rtf_keyword("clvmrg");
			vMerge = true;
		}
//
// Look to see if this is the first cell of a set of vertically merged cells
//		
		if(m_Table.getBot() > row +1)
		{
			m_pie->_rtf_keyword("clvmgf");
		}
#if 0
//
// Look to see if we have a horizontally merged cell.
//
		if(m_bNewTable && (m_Table.getLeft() < i))
		{
			m_pie->_rtf_keyword("clmrg");
		}
//
// Look to see if this is the first of a group of horizonatally merged cells.
//
		if(m_bNewTable && (m_Table.getRight() > i +1))
		{
			m_pie->_rtf_keyword("clmrgf");
		}
#endif
//
// output cellx for each cell
//
		double thisX = 0.0;
		UT_sint32 j =0;
		if(vecColProps.getItemCount() > 0)
		{
			for(j= 0; j< m_Table.getRight(); j++)
			{
				fl_ColProps * pColP = (fl_ColProps *) vecColProps.getNthItem(j);
				double bigWidth = (double)  pColP->m_iColWidth;
				thisX += bigWidth/10000.0;
			}
		}
		else
		{
			for(j= 0; j< m_Table.getRight(); j++)
			{
				thisX += colwidth;
			}
		}
		thisX += cellpos;
		UT_sint32 iCellTwips = 0;
		iCellTwips = (UT_sint32) (thisX*1440.0);
		m_pie->_rtf_keyword("cellx",iCellTwips);
	}
	if(vecColProps.getItemCount() > 0)
	{
		UT_VECTOR_PURGEALL(fl_ColProps *,vecColProps);
	}
	m_Table.setCellRowCol(row,col);
}

void s_RTF_ListenerWriteDoc::_outputTableBorders(UT_sint32 iThick)
{
	m_pie->_rtf_keyword("trbrdrt"); // top border
	m_pie->_rtf_keyword("brdrs"); // plain border
	m_pie->_rtf_keyword("brdrw",10*iThick); //border thickness
	m_pie->write(" ");											
	m_pie->_rtf_keyword("trbrdrl"); // left border
	m_pie->_rtf_keyword("brdrs");
	m_pie->_rtf_keyword("brdrw",10*iThick); // border thickness
	m_pie->write(" ");											
	m_pie->_rtf_keyword("trbrdrb"); // bottom border
	m_pie->_rtf_keyword("brdrs");
	m_pie->_rtf_keyword("brdrw",10*iThick); // border thickness
	m_pie->write(" ");											
	m_pie->_rtf_keyword("trbrdrr"); // right border
	m_pie->_rtf_keyword("brdrs");
	m_pie->_rtf_keyword("brdrw",10*iThick); // border thickness
	m_pie->write(" ");											
}

void s_RTF_ListenerWriteDoc::_outputCellBorders(UT_sint32 iThick)
{
	m_pie->_rtf_keyword("clbrdrt"); // cell top border
	m_pie->_rtf_keyword("brdrs"); // plain border
	m_pie->_rtf_keyword("brdrw",10*iThick); //border thickness
	m_pie->write(" ");											
	m_pie->_rtf_keyword("clbrdrl"); // cell left border
	m_pie->_rtf_keyword("brdrs");
	m_pie->_rtf_keyword("brdrw",10*iThick); // border thickness
	m_pie->write(" ");											
	m_pie->_rtf_keyword("clbrdrb"); // cell bottom border
	m_pie->_rtf_keyword("brdrs");
	m_pie->_rtf_keyword("brdrw",10*iThick); // border thickness
	m_pie->write(" ");											
	m_pie->_rtf_keyword("clbrdrr"); // cell right border
	m_pie->_rtf_keyword("brdrs");
	m_pie->_rtf_keyword("brdrw",10*iThick); // border thickness
	m_pie->write(" ");											
}

double s_RTF_ListenerWriteDoc::_getColumnWidthInches(void)
{
	double pageWidth = m_pDocument->m_docPageSize.Width(DIM_IN);

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;
	m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);
	const XML_Char * szColumns = PP_evalProperty("columns",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);
	const XML_Char * szColumnGap = PP_evalProperty("column-gap",
												   pSpanAP,pBlockAP,pSectionAP,
												   m_pDocument,true);
	const XML_Char * szMarginLeft = PP_evalProperty("page-margin-left",
													pSpanAP,pBlockAP,pSectionAP,
													m_pDocument,true);
	const XML_Char * szMarginRight = PP_evalProperty("page-margin-right",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,true);
	UT_sint32 iNumCols = 1;
	if(szColumns && *szColumns)
	{
		iNumCols = atoi(szColumns);
	}
	double dNumCols = (double) iNumCols;
	double lMarg = UT_convertToInches(szMarginLeft);
	double rMarg = UT_convertToInches(szMarginRight);
	double dGap = UT_convertToInches(szColumnGap);
	double colWidth = pageWidth - lMarg - rMarg - dGap*(dNumCols - 1.0);
	colWidth = colWidth/dNumCols;
	return colWidth;
}

void s_RTF_ListenerWriteDoc::_open_table(PT_AttrPropIndex api)
{
	if(m_Table.getNestDepth() > 1)
	{
	}
	m_Table.OpenTable(m_sdh,api);
	m_bNewTable = true;
	m_iLeft = -1;
	m_iRight = -1;
	m_iTop = -1;
	m_iBot = -1;
}

void s_RTF_ListenerWriteDoc::_close_cell(void)
{
	if(m_Table.getNestDepth() < 2)
	{
		m_pie->_rtf_keyword("cell");
	}
	else
	{
		m_pie->_rtf_keyword("nestcell");
	}
	m_Table.CloseCell();
}

void s_RTF_ListenerWriteDoc::_close_table(void)
{
//
// Close off the last row
//
	if(m_Table.getNestDepth() < 2)
	{
		m_pie->_rtf_keyword("row");
	}
	else
	{
		m_pie->_rtf_keyword("nestrow");
	}
	m_pie->_rtf_close_brace();
	if(m_Table.getNestDepth() > 1)
	{
		m_pie->_rtf_close_brace();
	}
	m_Table.CloseTable();
	if(m_Table.getNestDepth() < 1)
	{
		m_iCurRow = -1;
		m_iLeft = -1;
		m_iRight = -1;
		m_iTop = -1;
		m_iBot = -1;
	}
	else
	{
		m_iCurRow = m_Table.getTop();
		m_iLeft = m_Table.getLeft();
		m_iRight = m_Table.getRight();
		m_iTop = m_Table.getTop();
		m_iBot = m_Table.getBot();
	}
}


bool s_RTF_ListenerWriteDoc::populateStrux(PL_StruxDocHandle sdh,
										   const PX_ChangeRecord * pcr,
										   PL_StruxFmtHandle * psfh)
{
	UT_return_val_if_fail(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.

	m_posDoc = pcrx->getPosition();
	switch (pcrx->getStruxType())
	{
	case PTX_Section:
		{

			// begin a new section.  in RTF this is expressed as
			//
			// <section> := <secfmt>* <hdrftr>? <para>+ (\sect <section>)?
			//
			// here we deal with everything except for the <para>+
//
// OK first we have so see if there is a header/footer associated with this section
//
			PT_AttrPropIndex indexAP = pcr->getIndexAP();
			const PP_AttrProp* pAP = NULL;
			m_pDocument->getAttrProp(indexAP, &pAP);
			const XML_Char* pszHdrFtrID = NULL;

			pAP->getAttribute("header", pszHdrFtrID);
			if(pszHdrFtrID != NULL)
			{
				m_pie->exportHdrFtr("header",pszHdrFtrID);
			}
			pszHdrFtrID = NULL;
			pAP->getAttribute("footer", pszHdrFtrID);
			if(pszHdrFtrID != NULL)
			{
				m_pie->exportHdrFtr("footer",pszHdrFtrID);
			}

			pszHdrFtrID = NULL;
			pAP->getAttribute("header-even", pszHdrFtrID);
			if(pszHdrFtrID != NULL)
			{
				m_pie->exportHdrFtr("header-even",pszHdrFtrID);
			}

			pszHdrFtrID = NULL;
			pAP->getAttribute("footer-even", pszHdrFtrID);
			if(pszHdrFtrID != NULL)
			{
				m_pie->exportHdrFtr("footer-even",pszHdrFtrID);
			}

			pAP->getAttribute("header-first", pszHdrFtrID);
			pszHdrFtrID = NULL;
			if(pszHdrFtrID != NULL)
			{
				m_pie->exportHdrFtr("header-first",pszHdrFtrID);
			}

			pszHdrFtrID = NULL;
			pAP->getAttribute("footer-first", pszHdrFtrID);
			if(pszHdrFtrID != NULL)
			{
				m_pie->exportHdrFtr("footer-first",pszHdrFtrID);
			}
			_closeSpan();
			_closeBlock();
			_closeSection();
			_setTabEaten(false);

			m_sdh = sdh;
			_rtf_open_section(pcr->getIndexAP());
			return true;
		}

	case PTX_SectionHdrFtr:
		{
			_closeSpan();
			_closeBlock();
			_closeSection();
			_setTabEaten(false);
			return false;
#if 0
//
// We should have already outputting this.

			// begin a header/footer.  in RTF this is expressed as
			//
			// {' <hdrctl> <para>+ '}' where <hdrctl> is one of
			// \header or \footer for headers or footers on all pages
			// \headerl or \headerr or \headerf for headers on left, right, and first pages
			// \footerl or \footerr or \footerf for footers on left, right, and first pages
			//
			// here we deal with everything except for the <para>+
			m_sdh = sdh;
			m_pie->_rtf_nl();
			m_pie->_rtf_open_brace();
			PT_AttrPropIndex indexAP = pcr->getIndexAP();
			const PP_AttrProp* pAP = NULL;
			m_pDocument->getAttrProp(indexAP, &pAP);
			const XML_Char* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);
			if(0 == UT_strcmp(pszSectionType, "header"))
				m_pie->_rtf_keyword("header");
			else if(0 == UT_strcmp(pszSectionType, "footer"))
				m_pie->_rtf_keyword("footer");
			else
				UT_ASSERT_NOT_REACHED();
			return true;
#endif
		}
	case PTX_SectionTable:
	    {
			_closeSpan();
			_closeBlock();
			_setTabEaten(false);
			m_sdh = sdh;
			_open_table(pcr->getIndexAP());
			return true;
		}
	case PTX_SectionCell:
	    {
			_closeSpan();
			_closeBlock();
			_setTabEaten(false);
			m_sdh = sdh;
			_open_cell(pcr->getIndexAP());
			return true;
		}
	case PTX_EndTable:
	    {
			_closeSpan();
			_closeBlock();
			_setTabEaten(false);
			m_sdh = sdh;
			_close_table();
			return true;
		}
	case PTX_EndCell:
	    {

			_closeSpan();
			_closeBlock();
			_setTabEaten(false);
			m_sdh = sdh;
			_close_cell();
			return true;
		}
	case PTX_Block:
		{
			_closeSpan();
			_closeBlock(pcr->getIndexAP());
			_setListBlock(false);
			_setTabEaten(false);
			m_sdh = sdh;
			_rtf_open_block(pcr->getIndexAP());
			m_bBlankLine = true;
			return true;
		}

	default:
		UT_ASSERT_NOT_REACHED();
		return false;
	}
}

bool s_RTF_ListenerWriteDoc::change(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT_NOT_REACHED();	// this function is not used.
	return false;
}

bool s_RTF_ListenerWriteDoc::insertStrux(PL_StruxFmtHandle /*sfh*/,
										  const PX_ChangeRecord * /*pcr*/,
										  PL_StruxDocHandle /*sdh*/,
										  PL_ListenerId /* lid */,
										  void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
																	  PL_ListenerId /* lid */,
																	  PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT_NOT_REACHED();	// this function is not used.
	return false;
}

bool s_RTF_ListenerWriteDoc::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT_NOT_REACHED();	// this function is not used.
	return false;
}

//////////////////////////////////////////////////////////////////

/*
  {info

  {\title     #PCDATA}
  {\author    #PCDATA}
  {\manager   #PCDATA}
  {\company   #PCDATA}
  {\category  #PCDATA}
  {\keywords  #PCDATA}
  {\comment   #PCDATA}
  {\doccomm   #PCDATA}

  TODO:
  \userprops
    \propname
    \proptype
    \staticval

  }
 */
void s_RTF_ListenerWriteDoc::_rtf_info(void)
{
  UT_String propVal ;
  
  m_pie->_rtf_open_brace () ;
  m_pie->_rtf_keyword("info");

  if(m_pDocument->getMetaDataProp (PD_META_KEY_TITLE, propVal) && propVal.size())
    { m_pie->_rtf_open_brace () ; m_pie->_rtf_keyword("title ",propVal.c_str()); m_pie->_rtf_close_brace(); }
  if(m_pDocument->getMetaDataProp (PD_META_KEY_CREATOR, propVal) && propVal.size())
    { m_pie->_rtf_open_brace () ; m_pie->_rtf_keyword("author ",propVal.c_str()); m_pie->_rtf_close_brace(); }
  if(m_pDocument->getMetaDataProp (PD_META_KEY_CONTRIBUTOR, propVal) && propVal.size())
    { m_pie->_rtf_open_brace () ; m_pie->_rtf_keyword("manager ",propVal.c_str()); m_pie->_rtf_close_brace(); }
  if(m_pDocument->getMetaDataProp (PD_META_KEY_PUBLISHER, propVal) && propVal.size())
    { m_pie->_rtf_open_brace () ; m_pie->_rtf_keyword("company ",propVal.c_str()); m_pie->_rtf_close_brace(); }
  if(m_pDocument->getMetaDataProp (PD_META_KEY_SUBJECT, propVal) && propVal.size())
    { m_pie->_rtf_open_brace () ; m_pie->_rtf_keyword("category ",propVal.c_str()); m_pie->_rtf_close_brace(); }
  if(m_pDocument->getMetaDataProp (PD_META_KEY_KEYWORDS, propVal) && propVal.size())
    { m_pie->_rtf_open_brace () ; m_pie->_rtf_keyword("keywords ",propVal.c_str()); m_pie->_rtf_close_brace(); }
  if(m_pDocument->getMetaDataProp (PD_META_KEY_DESCRIPTION, propVal) && propVal.size())
    { m_pie->_rtf_open_brace () ; m_pie->_rtf_keyword("comment ",propVal.c_str()); m_pie->_rtf_close_brace(); }
  if(m_pDocument->getMetaDataProp (PD_META_KEY_DESCRIPTION, propVal) && propVal.size())
    { m_pie->_rtf_open_brace () ; m_pie->_rtf_keyword("doccomm ",propVal.c_str()); m_pie->_rtf_close_brace(); }

  m_pie->_rtf_close_brace();
}

void s_RTF_ListenerWriteDoc::_rtf_docfmt(void)
{
	// emit everything necessary for <docfmt>* portion of the document

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	// <docfmt>

	const XML_Char * szDefaultTabs = PP_evalProperty("default-tab-interval",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,true);
	m_pie->_rtf_keyword_ifnotdefault_twips("deftab",(char*)szDefaultTabs,720);

	// <docfmt> -- document views and zoom level

	m_pie->_rtf_keyword("viewkind",1);	/* PageLayout */

	// TODO <docfmt> -- footnotes and endnotes

	// <docfmt> -- page information

	char * old_locale;
	old_locale = setlocale (LC_NUMERIC, "C");

	double width = m_pDocument->m_docPageSize.Width(DIM_IN);
	double height = m_pDocument->m_docPageSize.Height(DIM_IN);
	bool landscape = !m_pDocument->m_docPageSize.isPortrait();

	UT_String szPaperWidth;
	UT_String szPaperHeight;

	UT_String_sprintf(szPaperWidth, "%fin", width);
	UT_String_sprintf(szPaperHeight, "%fin", height);

	setlocale (LC_NUMERIC, old_locale);

	m_pie->_rtf_keyword_ifnotdefault_twips("paperw",szPaperWidth.c_str(),0);
	m_pie->_rtf_keyword_ifnotdefault_twips("paperh",szPaperHeight.c_str(),0);

	const XML_Char * szLeftMargin = PP_evalProperty("page-margin-left",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,true);
	m_pie->_rtf_keyword_ifnotdefault_twips("margl",(char*)szLeftMargin,1800);
	const XML_Char * szRightMargin = PP_evalProperty("page-margin-right",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,true);
	m_pie->_rtf_keyword_ifnotdefault_twips("margr",(char*)szRightMargin,1800);
	const XML_Char * szTopMargin = PP_evalProperty("page-margin-top",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,true);
	m_pie->_rtf_keyword_ifnotdefault_twips("margt",(char*)szTopMargin,1440);
	const XML_Char * szBottomMargin = PP_evalProperty("page-margin-bottom",
													 pSpanAP,pBlockAP,pSectionAP,
													 m_pDocument,true);
	m_pie->_rtf_keyword_ifnotdefault_twips("margb",(char*)szBottomMargin,1440);

	if (landscape)
		m_pie->_rtf_keyword("landscape");
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
												 m_pDocument,true);
	const XML_Char * szColumnGap = PP_evalProperty("column-gap",
												   pSpanAP,pBlockAP,pSectionAP,
												   m_pDocument,true);

	const XML_Char * szColumnLine = PP_evalProperty("column-line",
													pSpanAP,pBlockAP,pSectionAP,
													m_pDocument,true);


	const XML_Char * szMarginLeft = PP_evalProperty("page-margin-left",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	const XML_Char * szMarginTop = PP_evalProperty("page-margin-top",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	const XML_Char * szMarginRight = PP_evalProperty("page-margin-right",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	const XML_Char * szMarginBottom = PP_evalProperty("page-margin-bottom",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	const XML_Char * szHeaderY = PP_evalProperty("page-margin-header",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	const XML_Char * szFooterY = PP_evalProperty("page-margin-footer",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);


// 	const XML_Char * szSpaceAfter = PP_evalProperty("section-space-after",
// 												 pSpanAP,pBlockAP,pSectionAP,
// 												 m_pDocument,true);

	const XML_Char * szRestartNumbering = PP_evalProperty("section-restart",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	const XML_Char * szRestartAt = PP_evalProperty("section-restart-value",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);
	const XML_Char * szHeaderExists = NULL;
	pSectionAP->getAttribute("header", szHeaderExists);
	const XML_Char * szFooterExists = NULL;
	pSectionAP->getAttribute("footer", szFooterExists);
	const XML_Char * szDomDir = PP_evalProperty("dom-dir",
												 pSpanAP,pBlockAP,pSectionAP,
												 m_pDocument,true);

	bool bSectRTL = UT_strcmp (szDomDir,"rtl") == 0;

	bool bColLine = false;
	if (szColumnLine && !UT_strcmp (szColumnLine, "on"))
		bColLine = true;

	// TODO add other properties here

	m_pie->_rtf_nl();
	_closeSpan();                   // In case it's open.
	if(m_bStartedList)
	{
		m_pie->_rtf_close_brace();
		m_bStartedList = false;
	}
	if (m_bJustStartingDoc)			// 'sect' is a delimiter, rather than a plain start
		m_bJustStartingDoc = false;
	else
		m_pie->_rtf_keyword("sect");							// begin a new section
	m_bJustStartingSection = true;

	m_pie->_rtf_keyword("sectd");								// restore all defaults for this section
	m_pie->_rtf_keyword("sbknone");								// no page break implied
	m_pie->_rtf_keyword_ifnotdefault("cols",(char*)szColumns,1);
	m_pie->_rtf_keyword_ifnotdefault_twips("colsx",(char*)szColumnGap,720);
	char * old_locale;
	old_locale = setlocale (LC_NUMERIC, "C");

	if (bColLine)
	{
		m_pie->_rtf_keyword ("linebetcol");
	}
	if(szHeaderExists)
	{
		double hMarg = UT_convertToInches(szHeaderY);
		UT_String sHeaderY;

		UT_String_sprintf(sHeaderY,"%fin",hMarg);
		m_pie->_rtf_keyword_ifnotdefault_twips("headery", (char*)sHeaderY.c_str(), 720);

	}
	if(szMarginTop)
	{
		double tMarg = UT_convertToInches(szMarginTop);
		UT_String sRtfTop;


		UT_String_sprintf(sRtfTop,"%fin",tMarg);
		m_pie->_rtf_keyword_ifnotdefault_twips("margtsxn", (char*)sRtfTop.c_str(), 1440);
	}
	if(szFooterExists  && szMarginBottom)
	{

		double bMarg = UT_convertToInches(szMarginBottom);
		double FooterY = UT_convertToInches(szFooterY);
		FooterY = bMarg - FooterY;
		if(FooterY < 0.0)
		{
			FooterY = bMarg;
		}
		UT_String sFooterY;
		UT_String_sprintf(sFooterY,"%fin",FooterY);
		m_pie->_rtf_keyword_ifnotdefault_twips("footery", (char*)sFooterY.c_str(), 720);
	}
	if(szMarginBottom)
	{
		double bMarg = UT_convertToInches(szMarginBottom);
		UT_String sRtfBot;
		UT_String_sprintf(sRtfBot,"%fin",bMarg);
		m_pie->_rtf_keyword_ifnotdefault_twips("margbsxn", (char*)sRtfBot.c_str(), 1440);
	}
	setlocale (LC_NUMERIC, old_locale);

	if(szMarginLeft)
	{
		m_pie->_rtf_keyword_ifnotdefault_twips("marglsxn", (char*)szMarginLeft, 1440);
	}
	if(szMarginRight)
	{
		m_pie->_rtf_keyword_ifnotdefault_twips("margrsxn", (char*)szMarginRight, 1440);
	}

	if(szRestartNumbering && UT_strcmp(szRestartNumbering,"1") == 0)
	{
		m_pie->_rtf_keyword("pgnrestart");
		if(szRestartAt)
		{
			UT_sint32 num = atoi(szRestartAt);
			m_pie->_rtf_keyword("pgnx",num);
		}
	}
	else
	{
		m_pie->_rtf_keyword("pgncont");
	}

	if (bSectRTL)
		m_pie->_rtf_keyword("rtlsect");
	else
		m_pie->_rtf_keyword("ltrsect");
}

//////////////////////////////////////////////////////////////////

void s_RTF_ListenerWriteDoc::_rtf_open_block(PT_AttrPropIndex api)
{
	m_apiThisBlock = api;

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);
	m_pDocument->getAttrProp(m_apiThisBlock,&pBlockAP);

	const XML_Char * szTextAlign = PP_evalProperty("text-align",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);
	const XML_Char * szFirstLineIndent = PP_evalProperty("text-indent",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);
	const XML_Char * szLeftIndent = PP_evalProperty("margin-left",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);
	const XML_Char * szRightIndent = PP_evalProperty("margin-right",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);
	const XML_Char * szTopMargin = PP_evalProperty("margin-top",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);
	const XML_Char * szBottomMargin = PP_evalProperty("margin-bottom",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);
	const XML_Char * szLineHeight = PP_evalProperty("line-height",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);
	const XML_Char * szKeepTogether = PP_evalProperty("keep-together",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);
	const XML_Char * szKeepWithNext = PP_evalProperty("keep-with-next",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);
	const XML_Char * szTabStops = PP_evalProperty("tabstops",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);

	// TODO add other properties here

	// Do abi specific list information.

	const XML_Char * szListid=NULL;
	const XML_Char * szParentid=NULL;
	const XML_Char * szListStyle=NULL;

	if (!pBlockAP || !pBlockAP->getAttribute((const XML_Char*)"listid", szListid))		szListid = NULL;
	if (!pBlockAP || !pBlockAP->getAttribute((const XML_Char*)"parentid", szParentid))
		szParentid = NULL;
	UT_uint32 listid = 0;
	const XML_Char * szAbiListDelim = NULL;
	const XML_Char * szAbiListDecimal = NULL;
	static UT_String szAbiStartValue;
	static UT_String szLevel;
	if(szListid!=NULL)
	{
		listid = atoi(szListid);
		if(listid != 0)
		{
			fl_AutoNum * pAuto = m_pDocument->getListByID(listid);
			szAbiListDelim = pAuto->getDelim();
			szAbiListDecimal = pAuto->getDecimal();
			UT_String_sprintf(szAbiStartValue,"%i",pAuto->getStartValue32());
			UT_String_sprintf(szLevel,"%i",pAuto->getLevel());
		}
	}
	szListStyle = PP_evalProperty("list-style",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);
	const XML_Char * szAbiFieldFont = PP_evalProperty("field-font",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);

	m_pie->_rtf_nl();

	if (m_bJustStartingSection)			// 'par' is a delimiter, rather than a plain start.
		m_bJustStartingSection = false;

	else
	{
		// begin a new paragraph. The previous
		// definitions get applied now.

		m_pie->_rtf_keyword("par");
		if(m_bStartedList)
		{
			m_pie->_rtf_close_brace();
		}
		m_bStartedList = false;
	}

	UT_uint32 id = 0;
	if(szListid != NULL)
		id = atoi(szListid);
	if(id == 0)
	{
//
// If span was openned in a previous closeBlock because of a blank line
// close it now.
//
		_closeSpan();
		m_pie->_rtf_keyword("pard");		// begin a new paragraph
		m_pie->_rtf_keyword("plain");		// begin a new paragraph
	}
	else
	{
		m_bStartedList = true;
	}
	///
	/// Output fallback numbered/bulleted label for rtf readers that don't
	/// know /*/pn
	///
	if(id != 0 )
	{
		m_pie->_rtf_open_brace();
		m_pie->_rtf_keyword("listtext");
		// if string is "left" use "ql", but that is the default, so we don't need to write it out.
		m_pie->_rtf_keyword("pard");		// restore all defaults for this paragraph
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

		fl_AutoNum * pAuto = m_pDocument->getListByID(id);
		UT_return_if_fail(pAuto);
		if(pAuto->getType()==BULLETED_LIST)
		{
			m_pie->_rtf_keyword("bullet");
		}
		else
		{
			const UT_UCSChar * lab = pAuto->getLabel(m_sdh);
            //
			// TODO Handle all our interesting symbols properly
            // In the meantime convert to char *
			static char tmp[100];
			if(lab != NULL)
			{
				UT_uint32 len = UT_MIN(UT_UCS4_strlen(lab),100);
				UT_uint32 i;
				for(i=0; i<=len; i++)
					tmp[i] = (char ) (unsigned char)  *lab++;
				m_pie->_rtf_chardata(tmp,len);
			}
			else
			{
			    UT_DEBUGMSG(("SEVIOR: We should not be here! id = %d \n",id));
				m_pie->_rtf_chardata(" ",1);
			}
		}
		//
        // Put in Tab for braindead RTF importers (like Ted) that can't
        // do numbering.
		//
		char tab = (char) 9;
		m_pie->_rtf_chardata(&tab,1);
		m_pie->_rtf_close_brace();
//
// Span was openned in a previous closeBlock because of a List item
// close it now.
//
		_closeSpan();
		m_pie->_rtf_keyword("pard");		// restore all defaults for this paragraph
		m_pie->_rtf_keyword("plain");		// restore all defaults for this paragraph
	}
	if(m_bStartedList)
	{
		m_pie->_rtf_open_brace();
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

		const XML_Char * szBidiDir = PP_evalProperty("dom-dir",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);
		xxx_UT_DEBUGMSG(("bidi paragraph: pSectionAp 0x%x, pBlockAP 0x%x, dom-dir\"%s\"\n",pSectionAP,pBlockAP,szBidiDir));
		if (szBidiDir)
		{
			if (!UT_strcmp (szBidiDir, "ltr"))
				m_pie->_rtf_keyword ("ltrpar");
			else
				m_pie->_rtf_keyword ("rtlpar");
		}

	const XML_Char * szStyle = NULL;
	if (pBlockAP->getAttribute("style", szStyle))
	{
	    m_pie->_rtf_keyword("s", m_pie->_getStyleNumber(szStyle));
	}
///
/// OK we need to output the char props if there is a list here
///
	if(id != 0)
	{
		const PP_AttrProp * pSpanAP = NULL;
		const PP_AttrProp * pBlockAP = NULL;
		const PP_AttrProp * pSectionAP = NULL;

		m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);
		m_pDocument->getAttrProp(m_apiThisBlock,&pBlockAP);
		m_pie->_write_charfmt(s_RTF_AttrPropAdapter_AP(pSpanAP, pBlockAP, pSectionAP, m_pDocument));
	}

	///
	/// OK if there is list info in this paragraph we encase it inside
	/// the {\*\abilist..} extension
	///
	if(id != 0 )
	{
		_setListBlock( true);

		m_pie->_rtf_open_brace();
		m_pie->_rtf_keyword("*");
		m_pie->_rtf_keyword("abilist");
		m_pie->_rtf_keyword_ifnotdefault("abilistid",(char *) szListid,-1);
		m_pie->_rtf_keyword_ifnotdefault("abilistparentid",(char *) szParentid,-1);
		m_pie->_rtf_keyword_ifnotdefault("abilistlevel",szLevel.c_str(),-1);
		m_pie->_rtf_keyword_ifnotdefault("abistartat",szAbiStartValue.c_str(),-1);
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

	///
	/// OK Now output word-95 style lists
	///

	if(id != 0 )
	{
		m_pie->_rtf_open_brace();
		m_pie->_rtf_keyword("*");
		m_pie->_rtf_keyword("pn");
		fl_AutoNum * pAuto = m_pDocument->getListByID(id);
		UT_return_if_fail(pAuto);
		m_pie->_rtf_keyword("pnql");
		m_pie->_rtf_keyword("pnstart",pAuto->getStartValue32());
		List_Type lType = pAuto->getType();

		///
		/// extract text before and after numbering symbol
		///
		static XML_Char p[80],leftDelim[80],rightDelim[80];
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
			m_pie->_rtf_keyword("pnlvlbody");
		}
		else if(lType >= BULLETED_LIST && (lType != NOT_A_LIST))
		{
			m_pie->_rtf_keyword("pnlvlblt");
		}
		else
		{
			m_pie->_rtf_keyword("pnprev");
			m_pie->_rtf_keyword("pnlvl",9);
		}
		if(lType == NUMBERED_LIST)
		{
			m_pie->_rtf_keyword("pndec");
		}
		else if(lType == LOWERCASE_LIST)
		{
			m_pie->_rtf_keyword("pnlcltr");
		}
		else if(lType == UPPERCASE_LIST)
		{
			m_pie->_rtf_keyword("pnucltr");
		}
		else if(lType == LOWERROMAN_LIST)
		{
			m_pie->_rtf_keyword("pnlcrm");
		}
		else if(lType == UPPERROMAN_LIST)
		{
			m_pie->_rtf_keyword("pnucrm");
		}
		else if(lType == NOT_A_LIST)
		{
			m_pie->_rtf_keyword("pnucrm");
		}
		if(lType < BULLETED_LIST)
		{
			m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("pntxtb");
			m_pie->_rtf_chardata((const char *)leftDelim,strlen((const char *) leftDelim));
			m_pie->_rtf_close_brace();
			m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("pntxta");
			m_pie->_rtf_chardata((const char *)rightDelim,strlen((const char *) rightDelim));
			m_pie->_rtf_close_brace();
		}
		else if(lType == BULLETED_LIST)
		{
			m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("pntxtb");
			m_pie->_rtf_keyword("bullet");
			m_pie->_rtf_close_brace();
		}
		else if(lType > BULLETED_LIST)
		{
			m_pie->_rtf_open_brace();
			m_pie->_rtf_keyword("pntxtb");
			const UT_UCSChar * tmp = pAuto->getLabel(m_sdh);
			UT_ASSERT_HARMLESS(tmp);

			if(!tmp) // Should not happen, if it does attempt to recover
			{
				m_pie->_rtf_chardata(" ",1);
			}
			else
			{
				UT_uint32 i,len;
				len = UT_UCS4_strlen(tmp);
				for(i=0;i<=len;i++)
					p[i] = (char) (unsigned char) *tmp++;
				m_pie->_rtf_chardata(p,len);
			}
			m_pie->_rtf_close_brace();
		}
		m_pie->_rtf_close_brace();
	}



	///
	/// OK Now output word-97 style lists. First detect if we've moved to
    /// a new list list structure. We need m_currID to track the previous list
    /// we were in.
	///
	xxx_UT_DEBUGMSG(("SEVIOR: Doing output of list structure id = %d\n",id));
	if(id != 0 )
	{
		UT_uint32 iOver = m_pie->getMatchingOverideNum(id);
		UT_uint32 iLevel = 0;
		fl_AutoNum * pAuto = m_pDocument->getListByID(id);

//		if(id != m_currID)
		{
			UT_return_if_fail(iOver);
			fl_AutoNum * pAuto = m_pDocument->getListByID(id);
			UT_return_if_fail(pAuto);
			while(pAuto->getParent() != NULL)
			{
				pAuto = pAuto->getParent();
				iLevel++;
			}
			if(iLevel > 8)
			{
				UT_ASSERT(0);
				iLevel = 8;
			}
			m_currID = id;
		}
		/* This is changed so that Word97 can see the numbers in
		   numbered lists */
		if(pAuto->getType() < BULLETED_LIST)
		{
	        	m_pie->_rtf_keyword_ifnotdefault_twips("fn",(char*)szFirstLineIndent,0);
	        	m_pie->_rtf_keyword_ifnotdefault_twips("li",(char*)szLeftIndent,0);
		}
		m_pie->_rtf_keyword("ls",iOver);
		m_pie->_rtf_keyword("ilvl",iLevel);
	}

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

//
// Output Paragraph Cell nesting level.
//
	m_pie->_rtf_keyword("itap",m_Table.getNestDepth());

	if (UT_strcmp(szKeepTogether,"yes")==0)
		m_pie->_rtf_keyword("keep");
	if (UT_strcmp(szKeepWithNext,"yes")==0)
		m_pie->_rtf_keyword("keepn");

	m_pie->_write_tabdef(szTabStops);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void s_RTF_ListenerWriteDoc::_writeBookmark(const PX_ChangeRecord_Object * pcro)
{
	PT_AttrPropIndex api = pcro->getIndexAP();
	const PP_AttrProp * pBookmarkAP = NULL;
	m_pDocument->getAttrProp(api,&pBookmarkAP);

	const XML_Char * szType = NULL;
	bool bFound = pBookmarkAP->getAttribute("type", szType);
	if (!bFound) {
		UT_DEBUGMSG (("RTF_Export: cannot get type for bookmark\n"));
		return;
	}
	const XML_Char * szName = NULL;
	bFound = pBookmarkAP->getAttribute("name", szName);
	if (!bFound) {
		UT_DEBUGMSG (("RTF_Export: cannot get name for bookmark\n"));
		return;
	}
	m_pie->_rtf_open_brace();
	{
		m_pie->_rtf_keyword("*");
		if (UT_strcmp (szType, "start") == 0) {
			m_pie->_rtf_keyword("bkmkstart");
		}
		else if (UT_strcmp (szType, "end") == 0) {
			m_pie->_rtf_keyword("bkmkend");
		}
		m_pie->_rtf_chardata(szName, strlen(szName));
		m_pie->_rtf_close_brace();
	}
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void s_RTF_ListenerWriteDoc::_writeHyperlink(const PX_ChangeRecord_Object * pcro)
{
#if 0
	PT_AttrPropIndex api = pcro->getIndexAP();
	const PP_AttrProp * pHyperlinkAP = NULL;
	m_pDocument->getAttrProp(api,&pHyperlinkAP);

	const XML_Char * szHyper = NULL;
	bool bFound = pHyperlinkAP->getAttribute("xlink:href", szHyper);
	if (!bFound)
	{
		UT_DEBUGMSG (("RTF_Export: cannot get address for hyperlink\n"));
		return;
	}
	_writeFieldPreamble(pHyperlinkAP);
	m_pie->write("HYPERLINK ");
	m_pie->write("\"");
	m_pie->write(szHyper);
	m_pie->write("\"");
	m_pie->_rtf_close_brace();
	m_pie->_rtf_close_brace();
	m_pie->_rtf_close_brace();
#endif
	return;
}


void s_RTF_ListenerWriteDoc::_writeImageInRTF(const PX_ChangeRecord_Object * pcro)
{
	PT_AttrPropIndex api = pcro->getIndexAP();
	const PP_AttrProp * pImageAP = NULL;
	m_pDocument->getAttrProp(api,&pImageAP);

	// fetch the "name" of the image and use it to fetch the actual image data.

	const XML_Char * szDataID = NULL;
	bool bFoundDataID = pImageAP->getAttribute("dataid",szDataID);
	if (!bFoundDataID)
	{
		UT_DEBUGMSG(("RTF_Export: cannot get dataid for image\n"));
		return;
	}
	const UT_ByteBuf * pbb = NULL;
	void * pToken = NULL;
	void * pHandle = NULL;
	bool bFoundDataItem = m_pDocument->getDataItemDataByName((char*)szDataID,&pbb,&pToken,&pHandle);
	if (!bFoundDataItem)
	{
		UT_DEBUGMSG(("RTF_Export: cannot get dataitem for image\n"));
		return;
	}

	// see if the image has a width/height attribute that should
	// override the actual pixel size of the image.

	const XML_Char * szWidthProp = NULL;
	const XML_Char * szHeightProp = NULL;
	bool bFoundWidthProperty = pImageAP->getProperty("width",szWidthProp);
	bool bFoundHeightProperty = pImageAP->getProperty("height",szHeightProp);

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
			UT_uint32 tag = UT_newNumber ();
			m_pie->_rtf_keyword("bliptag",tag);
			m_pie->_rtf_open_brace();
			{
				m_pie->_rtf_keyword("*");
				m_pie->_rtf_keyword("blipuid");
				UT_String buf;
				UT_String_sprintf(buf,"%032x",tag);
				m_pie->_rtf_chardata(buf.c_str(),buf.size());
			}
			m_pie->_rtf_close_brace();

			UT_uint32 lenData = pbb->getLength();
			const UT_Byte * pData = pbb->getPointer(0);
			UT_uint32 k;

			for (k=0; k<lenData; k++)
			{
				if (k%32==0)
					m_pie->_rtf_nl();
				UT_String buf;
				UT_String_sprintf(buf,"%02x",pData[k]);
				m_pie->_rtf_chardata(buf.c_str(),2);
			}
		}
		m_pie->_rtf_close_brace();
	}
	m_pie->_rtf_close_brace();
}
