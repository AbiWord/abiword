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
#include <malloc.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_AbiWord_1.h"
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
}

IE_Imp_AbiWord_1::IE_Imp_AbiWord_1(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
	m_iestatus = IES_OK;
	m_parseState = _PS_Init;
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Imp_AbiWord_1::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".abw") == 0);
}

IEStatus IE_Imp_AbiWord_1::StaticConstructor(const char * szSuffix,
										PD_Document * pDocument,
										IE_Imp ** ppie)
{
	UT_ASSERT(RecognizeSuffix(szSuffix));
	
	IE_Imp_AbiWord_1 * p = new IE_Imp_AbiWord_1(pDocument);
	*ppie = p;
	return IES_OK;
}

UT_Bool	IE_Imp_AbiWord_1::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList)
{
	*pszDesc = "AbiWord (.abw)";
	*pszSuffixList = "*.abw";
	return UT_TRUE;
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

struct _TokenTable
{
	const char *	m_name;
	int				m_type;
};

static struct _TokenTable s_Tokens[] =
{	{	"awml",			TT_DOCUMENT		},
	{	"section",		TT_SECTION		},
	{	"p",			TT_BLOCK		},
	{	"c",			TT_INLINE		},
	{	"i",			TT_IMAGE		},
	{	"f",			TT_FIELD		},
	{	"br",			TT_BREAK		},
	{	"data",			TT_DATASECTION	},
	{	"d",			TT_DATAITEM		},
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
			UT_UCSChar ucs = 0x000a;	// LF
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
		// notify the piece table that we are starting a DataItem.
		// this is to give it the attrs, we'll send the actual
		// data after we've seen the end tag.
		X_CheckError(m_pDocument->createDataItem(atts,&m_currentDataItemHandle));
		return;
		
	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown tag [%s]\n",name));
		m_iestatus = IES_BogusDocument;
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
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth()==0);
		return;
		
	case TT_INLINE:
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()>0);
		_popInlineFmt();
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

	case TT_IMAGE:						// not a container, so we don't pop stack
		X_VerifyParseState(_PS_Block);
		return;

	case TT_FIELD:						// not a container, so we don't pop stack
		X_VerifyParseState(_PS_Block);
		return;

	case TT_BREAK:						// not a container, so we don't pop stack
		X_VerifyParseState(_PS_Block);
		return;

	case TT_DATASECTION:
		X_VerifyParseState(_PS_DataSec);
		m_parseState = _PS_Doc;
		return;

	case TT_DATAITEM:
		X_VerifyParseState(_PS_DataItem);
		m_parseState = _PS_DataSec;
		// give the piece table a temporary ByteBuf containing
		// the DataItem's data.  we assume that this will go
		// with the last DataItem we started.  It is the piece
		// table's responsibility to make a copy of this data
		// if it wants it.
		//
		// we set the flag true to indicate that we have Base64
		// data rather than plain data.
		X_CheckError(m_pDocument->setDataItemData(m_currentDataItemHandle,UT_TRUE,&m_currentDataItem));
		return;
		
	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown end tag [%s]\n",name));
		m_iestatus = IES_BogusDocument;
		return;
	}
}

void IE_Imp_AbiWord_1::_charData(const XML_Char *s, int len)
{
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
			// TODO fix this to use proper methods to convert XML_Char to UT_UCSChar

			UT_ASSERT(sizeof(XML_Char) != sizeof(UT_UCSChar));

			UT_UCSChar * xx = new UT_UCSChar[len];
			int iConverted = 0;

			// This code is intended to convert a single EOL to a single space.

			for (int k=0; k<len; k++)
			{
				if (13 == s[k])			// CR
				{
					xx[iConverted++] = 32;	// space
					if (10 == s[k+1])
					{
						k++;			// skip the LF
					}
				}
				else if (10 == s[k])
				{
					xx[iConverted++] = 32;	// space
				}
				else
				{
					xx[iConverted++] = s[k];
				}
			}
			
			UT_Bool bResult = m_pDocument->appendSpan(xx,iConverted);
			delete xx;
			X_CheckError(bResult);

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

