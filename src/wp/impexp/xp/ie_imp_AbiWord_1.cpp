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


#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_AbiWord_1.h"
#include "ie_types.h"
#include "pd_Document.h"
#include "ut_bytebuf.h"


/*****************************************************************
******************************************************************
** C-style callback functions that we register with the XML parser
******************************************************************
*****************************************************************/

static void startElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	IE_Imp_AbiWord_1* pDocReader = (IE_Imp_AbiWord_1*) userData;
	pDocReader->_startElement(name, atts);
}

static void endElement(void *userData, const XML_Char *name)
{
	IE_Imp_AbiWord_1* pDocReader = (IE_Imp_AbiWord_1*) userData;
	pDocReader->_endElement(name);
}

static void charData(void* userData, const XML_Char *s, int len)
{
	IE_Imp_AbiWord_1* pDocReader = (IE_Imp_AbiWord_1*) userData;
	pDocReader->_charData(s, len);
}

/*****************************************************************/
/*****************************************************************/

IEStatus IE_Imp_AbiWord_1::importFile(const char * szFilename)
{
	XML_Parser parser = NULL;
	FILE *fp = NULL;
	int done = 0;
	char buf[4096];

	fp = fopen(szFilename, "r");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		m_iestatus = IES_FileNotFound;
		goto Cleanup;
	}
	
	parser = XML_ParserCreate(NULL);
	XML_SetUserData(parser, this);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, charData);

	while (!done)
	{
		size_t len = fread(buf, 1, sizeof(buf), fp);
		done = (len < sizeof(buf));

		if (!XML_Parse(parser, buf, len, done)) 
		{
			UT_DEBUGMSG(("%s at line %d\n",
						 XML_ErrorString(XML_GetErrorCode(parser)),
						 XML_GetCurrentLineNumber(parser)));
			m_iestatus = IES_BogusDocument;
			goto Cleanup;
		}

		if (m_iestatus != IES_OK)
		{
			UT_DEBUGMSG(("Problem reading document\n"));
			goto Cleanup;
		}
	} 
	
	m_iestatus = IES_OK;

Cleanup:
	if (parser)
		XML_ParserFree(parser);
	if (fp)
		fclose(fp);
	return m_iestatus;
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_AbiWord_1::~IE_Imp_AbiWord_1()
{
	FREEP(m_currentDataItemName);
}

IE_Imp_AbiWord_1::IE_Imp_AbiWord_1(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
	m_iestatus = IES_OK;
	m_parseState = _PS_Init;
	m_lenCharDataSeen = 0;
	m_lenCharDataExpected = 0;
	m_bSeenCR = UT_FALSE;

	m_currentDataItemName = NULL;
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Imp_AbiWord_1::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".abw") == 0);
}

IEStatus IE_Imp_AbiWord_1::StaticConstructor(PD_Document * pDocument,
											 IE_Imp ** ppie)
{
	IE_Imp_AbiWord_1 * p = new IE_Imp_AbiWord_1(pDocument);
	*ppie = p;
	return IES_OK;
}

UT_Bool	IE_Imp_AbiWord_1::GetDlgLabels(const char ** pszDesc,
									   const char ** pszSuffixList,
									   IEFileType * ft)
{
	*pszDesc = "AbiWord (.abw)";
	*pszSuffixList = "*.abw";
	*ft = IEFT_AbiWord_1;
	return UT_TRUE;
}

UT_Bool IE_Imp_AbiWord_1::SupportsFileType(IEFileType ft)
{
	return (IEFT_AbiWord_1 == ft);
}

/*****************************************************************/
/*****************************************************************/

#define TT_OTHER		0
#define TT_DOCUMENT		1		// a document <awml>
#define TT_SECTION		2		// a section <section>
#define TT_BLOCK		3		// a paragraph <p>
#define TT_INLINE		4		// inline span of text <c>
#define TT_IMAGE		5		// an image object <i>
#define TT_FIELD		6		// a computed field object <f>
#define TT_BREAK		7		// a forced line-break <br>
#define TT_DATASECTION	8		// a data section <data>
#define TT_DATAITEM		9		// a data item <d> within a data section
#define TT_COLBREAK		10		// a forced column-break <cbr>
#define TT_PAGEBREAK	11		// a forced page-break <pbr>
#define TT_STYLESECTION	12		// a style section <styles>
#define TT_STYLE		13		// a style <s> within a style section

struct _TokenTable
{
	const char *	m_name;
	int				m_type;
};

/*
  TODO remove tag synonyms.  We're currently accepted
  synonyms for tags, as follows:

  abiword	awml
  field		f
  image		i

  The renaming of these tags occurred 26 Mar 1999, shortly
  after tarball 0.5.2.  Eventually, this backwards compatibility
  code should be removed.
*/

static struct _TokenTable s_Tokens[] =
{
	{	"abiword",		TT_DOCUMENT		},
	{	"awml",			TT_DOCUMENT		},
	{	"section",		TT_SECTION		},
	{	"p",			TT_BLOCK		},
	{	"c",			TT_INLINE		},
	{	"i",			TT_IMAGE		},
	{	"image",		TT_IMAGE		},
	{	"f",			TT_FIELD		},
	{	"field",		TT_FIELD		},
	{	"br",			TT_BREAK		},
	{	"data",			TT_DATASECTION	},
	{	"d",			TT_DATAITEM		},
	{	"cbr",			TT_COLBREAK		},
	{	"pbr",			TT_PAGEBREAK	},
	{	"styles",		TT_STYLESECTION	},
	{	"s",			TT_STYLE		},
	{	"*",			TT_OTHER		}};	// must be last

#define TokenTableSize	((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

static UT_uint32 s_mapNameToToken(const XML_Char * name)
{
	for (unsigned int k=0; k<TokenTableSize; k++)
		if (s_Tokens[k].m_name[0] == '*')
			return k;
		else if (UT_stricmp(s_Tokens[k].m_name,name)==0)
			return k;
	UT_ASSERT(0);
	return 0;
}

/*****************************************************************/	
/*****************************************************************/	

#define X_TestParseState(ps)	((m_parseState==(ps)))

#define X_VerifyParseState(ps)	do {  if (!(X_TestParseState(ps)))			\
									  {  m_iestatus = IES_BogusDocument;	\
										 return; } } while (0)

#define X_CheckDocument(b)		do {  if (!(b))								\
									  {  m_iestatus = IES_BogusDocument;	\
										 return; } } while (0)

#define X_CheckError(v)			do {  if (!(v))								\
									  {  m_iestatus = IES_Error;			\
										 return; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_iestatus != IES_OK) return; } while (0)

/*****************************************************************/
/*****************************************************************/

void IE_Imp_AbiWord_1::_startElement(const XML_Char *name, const XML_Char **atts)
{
	xxx_UT_DEBUGMSG(("startElement: %s\n", name));

	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed
	
	UT_uint32 tokenIndex = s_mapNameToToken(name);
	switch (s_Tokens[tokenIndex].m_type)
	{
	case TT_DOCUMENT:
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;
		return;

	case TT_SECTION:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Sec;
		X_CheckError(m_pDocument->appendStrux(PTX_Section,atts));
		return;

	case TT_BLOCK:
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		X_CheckError(m_pDocument->appendStrux(PTX_Block,atts));
		return;
		
	case TT_INLINE:
		X_VerifyParseState(_PS_Block);
		X_CheckError(_pushInlineFmt(atts));
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

		// Images and Fields are not containers.  Therefore we don't
		// push the ParseState (_PS_...).
		// TODO should Images or Fields inherit the (possibly nested)
		// TODO inline span formatting.
		
	case TT_IMAGE:
		X_VerifyParseState(_PS_Block);
		X_CheckError(m_pDocument->appendObject(PTO_Image,atts));
		return;

	case TT_FIELD:
		X_VerifyParseState(_PS_Block);
		X_CheckError(m_pDocument->appendObject(PTO_Field,atts));
		return;

		// Forced Line Breaks are not containers.  Therefore we don't
		// push the ParseState (_PS_...).  Breaks are marked with a
		// tag, but are translated into character data (LF).  This may
		// seem a little odd (perhaps an &lf; entity would be better).
		// Anyway, this distinction from ordinary LF's in the document
		// (which get mapped into SPACE) keeps the file sanely editable.

	case TT_BREAK:
		X_VerifyParseState(_PS_Block);
		// TODO decide if we should push and pop the attr's
		// TODO that came in with the <br/>.  that is, decide
		// TODO if <br/>'s will have any attributes or will
		// TODO just inherit everything from the surrounding
		// TODO spans.
		{
			UT_UCSChar ucs = UCS_LF;
			X_CheckError(m_pDocument->appendSpan(&ucs,1));
		}
		return;

	case TT_COLBREAK:
		X_VerifyParseState(_PS_Block);
		// TODO decide if we should push and pop the attr's
		// TODO that came in with the <cbr/>.  that is, decide
		// TODO if <cbr/>'s will have any attributes or will
		// TODO just inherit everything from the surrounding
		// TODO spans.
		{
			UT_UCSChar ucs = UCS_VTAB;
			X_CheckError(m_pDocument->appendSpan(&ucs,1));
		}
		return;

	case TT_PAGEBREAK:
		X_VerifyParseState(_PS_Block);
		// TODO decide if we should push and pop the attr's
		// TODO that came in with the <pbr/>.  that is, decide
		// TODO if <pbr/>'s will have any attributes or will
		// TODO just inherit everything from the surrounding
		// TODO spans.
		{
			UT_UCSChar ucs = UCS_FF;
			X_CheckError(m_pDocument->appendSpan(&ucs,1));
		}
		return;

	case TT_DATASECTION:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_DataSec;
		// We don't need to notify the piece table of the data section,
		// it will get the hint when we begin sending data items.
		return;

	case TT_DATAITEM:
		X_VerifyParseState(_PS_DataSec);
		m_parseState = _PS_DataItem;
		m_currentDataItem.truncate(0);
		X_CheckError(UT_XML_cloneString(m_currentDataItemName,_getDataItemName(atts)));
		return;
		
	case TT_STYLESECTION:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_StyleSec;
		// We don't need to notify the piece table of the style section,
		// it will get the hint when we begin sending styles.
		return;

	case TT_STYLE:
		X_VerifyParseState(_PS_StyleSec);
		m_parseState = _PS_Style;
		X_CheckError(m_pDocument->appendStyle(atts));
		return;
		
	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown tag [%s]\n",name));
#if 0
		m_iestatus = IES_BogusDocument;
#endif		
		return;
	}
}

void IE_Imp_AbiWord_1::_endElement(const XML_Char *name)
{
	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed
	
	UT_uint32 tokenIndex = s_mapNameToToken(name);

	switch (s_Tokens[tokenIndex].m_type)
	{
	case TT_DOCUMENT:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Init;
		return;

	case TT_SECTION:
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Doc;
		return;

	case TT_BLOCK:
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth()==0);
		return;
		
	case TT_INLINE:
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()>0);
		_popInlineFmt();
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

	case TT_IMAGE:						// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		return;

	case TT_FIELD:						// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		return;

	case TT_BREAK:						// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		return;

	case TT_COLBREAK:					// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		return;

	case TT_PAGEBREAK:					// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		return;

	case TT_DATASECTION:
		X_VerifyParseState(_PS_DataSec);
		m_parseState = _PS_Doc;
		return;

	case TT_DATAITEM:
		X_VerifyParseState(_PS_DataItem);
		m_parseState = _PS_DataSec;
		X_CheckError(m_pDocument->createDataItem(m_currentDataItemName,UT_TRUE,&m_currentDataItem,NULL,NULL));
		FREEP(m_currentDataItemName);
		return;
		
	case TT_STYLESECTION:
		X_VerifyParseState(_PS_StyleSec);
		m_parseState = _PS_Doc;
		return;

	case TT_STYLE:
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Style);
		m_parseState = _PS_StyleSec;
		return;
		
	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown end tag [%s]\n",name));
#if 0
		m_iestatus = IES_BogusDocument;
#endif		
		return;
	}
}

void IE_Imp_AbiWord_1::_charData(const XML_Char *s, int len)
{
	// TODO XML_Char is defined in the xml parser
	// TODO as a 'char' not as a 'unsigned char'.
	// TODO does this cause any problems ??
	
	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed

	switch (m_parseState)
	{
	default:
		{
			xxx_UT_DEBUGMSG(("charData DISCARDED [length %d]\n",len));
			return;
		}
		
	case _PS_Block:
		{
			UT_ASSERT(sizeof(XML_Char) == sizeof(UT_Byte));
			UT_ASSERT(sizeof(XML_Char) != sizeof(UT_UCSChar));

			// parse UTF-8 text and convert to Unicode.
			// also take care of some white-space issues:
			//    [] convert CRLF to SP.
			//    [] convert CR to SP.
			//    [] convert LF to SP.

			UT_Byte * ss = (UT_Byte *)s;
			UT_UCSChar buf[1024];
			int bufLen = 0;

			for (int k=0; k<len; k++)
			{
				if (bufLen == NrElements(buf))		// pump it out in chunks
				{
					X_CheckError(m_pDocument->appendSpan(buf,bufLen));
					bufLen = 0;
				}

				if ((ss[k] < 0x80) && (m_lenCharDataSeen > 0))
				{
					// is it us-ascii and we are in a UTF-8
					// multi-byte sequence.  puke.
					X_CheckError(0);
				}

				if (ss[k] == UCS_CR)
				{
					buf[bufLen++] = UCS_SPACE;		// substitute a SPACE
					m_bSeenCR = UT_TRUE;
					continue;
				}

				if (ss[k] == UCS_LF)				// LF
				{
					if (!m_bSeenCR)					// if not immediately after a CR,
						buf[bufLen++] = UCS_SPACE;	// substitute a SPACE.  otherwise, eat.
					m_bSeenCR = UT_FALSE;
					continue;
				}
				
				m_bSeenCR = UT_FALSE;

				if (ss[k] < 0x80)					// plain us-ascii part of latin-1
				{
					buf[bufLen++] = ss[k];			// copy as is.
				}
				else if ((ss[k] & 0xf0) == 0xf0)	// lead byte in 4-byte surrogate pair
				{
					// surrogate pairs are defined in section 3.7 of the
					// unicode standard version 2.0 as an extension
					// mechanism for rare characters in future extensions
					// of the unicode standard.
					UT_ASSERT(m_lenCharDataSeen == 0);
					UT_ASSERT(UT_NOT_IMPLEMENTED);
				}
				else if ((ss[k] & 0xe0) == 0xe0)		// lead byte in 3-byte sequence
				{
					UT_ASSERT(m_lenCharDataSeen == 0);
					m_lenCharDataExpected = 3;
					m_charDataSeen[m_lenCharDataSeen++] = ss[k];
				}
				else if ((ss[k] & 0xc0) == 0xc0)		// lead byte in 2-byte sequence
				{
					UT_ASSERT(m_lenCharDataSeen == 0);
					m_lenCharDataExpected = 2;
					m_charDataSeen[m_lenCharDataSeen++] = ss[k];
				}
				else if ((ss[k] & 0x80) == 0x80)		// trailing byte in multi-byte sequence
				{
					UT_ASSERT(m_lenCharDataSeen > 0);
					m_charDataSeen[m_lenCharDataSeen++] = ss[k];
					if (m_lenCharDataSeen == m_lenCharDataExpected)
					{
						buf[bufLen++] = UT_decodeUTF8char(m_charDataSeen,m_lenCharDataSeen);
						m_lenCharDataSeen = 0;
					}
				}
			}

			// flush out the last piece of a buffer

			if (bufLen > 0)
				X_CheckError(m_pDocument->appendSpan(buf,bufLen));
			return;
		}

	case _PS_DataItem:
		{
#define MyIsWhite(c)			(((c)==' ') || ((c)=='\t') || ((c)=='\n') || ((c)=='\r'))
			
			// DataItem data consists of Base64 encoded data with
			// white space added for readability.  strip out any
			// white space and put the rest in the ByteBuf.

			UT_ASSERT((sizeof(XML_Char) == sizeof(UT_Byte)));
			
			const UT_Byte * ss = (UT_Byte *)s;
			const UT_Byte * ssEnd = ss + len;
			while (ss < ssEnd)
			{
				while ((ss < ssEnd) && MyIsWhite(*ss))
					ss++;
				UT_uint32 k=0;
				while ((ss+k < ssEnd) && ( ! MyIsWhite(ss[k])))
					k++;
				if (k > 0)
					m_currentDataItem.ins(m_currentDataItem.getLength(),ss,k);

				ss += k;
			}

			return;
#undef MyIsWhite
		}
	}
}

/*****************************************************************/
/*****************************************************************/

UT_uint32 IE_Imp_AbiWord_1::_getInlineDepth(void) const
{
	return m_stackFmtStartIndex.getDepth();
}

UT_Bool IE_Imp_AbiWord_1::_pushInlineFmt(const XML_Char ** atts)
{
	UT_uint32 start = m_vecInlineFmt.getItemCount()+1;
	UT_uint32 k;

	for (k=0; (atts[k]); k++)
	{
		XML_Char * p;
		if (!UT_XML_cloneString(p,atts[k]))
			return UT_FALSE;
		if (m_vecInlineFmt.addItem(p)!=0)
			return UT_FALSE;
	}
	if (!m_stackFmtStartIndex.push((void*)start))
		return UT_FALSE;
	return UT_TRUE;
}

void IE_Imp_AbiWord_1::_popInlineFmt(void)
{
	UT_uint32 start;
	if (!m_stackFmtStartIndex.pop((void **)&start))
		return;
	UT_uint32 k;
	UT_uint32 end = m_vecInlineFmt.getItemCount();
	for (k=end; k>=start; k--)
	{
		const XML_Char * p = (const XML_Char *)m_vecInlineFmt.getNthItem(k-1);
		m_vecInlineFmt.deleteNthItem(k-1);
		if (p)
			free((void *)p);
	}
}

const XML_Char * IE_Imp_AbiWord_1::_getDataItemName(const XML_Char ** atts)
{
	// find the 'name="value"' pair and return the "value".
	// ignore everything else (which there shouldn't be)

	for (const XML_Char ** a = atts; (*a); a++)
		if (UT_XML_stricmp(a[0],"name") == 0)
			return a[1];
	return NULL;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void IE_Imp_AbiWord_1::pasteFromBuffer(PD_DocumentRange * pDocRange,
									   unsigned char * pData, UT_uint32 lenData)
{
}
