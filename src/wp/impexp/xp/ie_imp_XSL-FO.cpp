/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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
#include <string.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_XSL-FO.h"
#include "ie_types.h"
#include "pd_Document.h"
#include "ut_growbuf.h"

#include "ut_string_class.h"

/*
 * This is meant to import XSL-FO documents. XSL-FO are XML/XSL
 * Formatting objects, meant to be similar in scope to LaTeX.
 * The reference I've been using is located at:
 * http://zvon.org/xxl/xslfoReference/Output/index.html
 *
 * Dom
 */

// this importer is of Beta quality
// it handles a lot of XSL-FO but also doesn't handle a
// lot of key things

/*****************************************************************/
/*****************************************************************/

IE_Imp_XSL_FO::~IE_Imp_XSL_FO()
{
}

IE_Imp_XSL_FO::IE_Imp_XSL_FO(PD_Document * pDocument)
	: IE_Imp_XML(pDocument, false)
{
}

/*****************************************************************/
/*****************************************************************/

bool IE_Imp_XSL_FO::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	UT_uint32 iLinesToRead = 6;
	UT_uint32 iBytesScanned = 0;

	const char *p;
	char * magic;

	p = szBuf;

	while ( iLinesToRead-- )
	{
		magic = "<fo:root ";
		if ( (iNumbytes - iBytesScanned) < strlen(magic) ) return(false);
		if ( strncmp(p, magic, strlen(magic)) == 0 ) return(true);
		/*  Seek to the next newline:  */
		while ( *p != '\n' && *p != '\r' )
		{
			iBytesScanned++; p++;
			if( iBytesScanned+2 >= iNumbytes ) return(false);
		}
		/*  Seek past the next newline:  */
		if ( *p == '\n' || *p == '\r' )
		{
			iBytesScanned++ ; p++ ;
			if ( *p == '\n' || *p == '\r' )
			{
				iBytesScanned++; p++;
			}
		}
	}

  return false;
}

bool IE_Imp_XSL_FO::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".fo") == 0);
}

UT_Error IE_Imp_XSL_FO::StaticConstructor(PD_Document * pDocument,
					IE_Imp ** ppie)
{
	IE_Imp_XSL_FO * p = new IE_Imp_XSL_FO(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_XSL_FO::GetDlgLabels(const char ** pszDesc,
									const char ** pszSuffixList,
									IEFileType * ft)
{
	*pszDesc = "XSL-FO (.fo)";
	*pszSuffixList = "*.fo";
	*ft = IEFT_XSL_FO;
	return true;
}

bool IE_Imp_XSL_FO::SupportsFileType(IEFileType ft)
{
	return (IEFT_XSL_FO == ft);
}

/*****************************************************************/
/*****************************************************************/

// we handle a small subset of the XSL-FO spec

#define TT_OTHER    0
#define TT_DOCUMENT 1
#define TT_SECTION  2
#define TT_BLOCK    3
#define TT_INLINE   4
#define TT_CHAR     5
#define TT_IMAGE    6

#define TT_LAYOUT_MASTER_SET  7
#define TT_SIMPLE_PAGE_MASTER 8
#define TT_REGION_BODY        9
#define TT_PAGE_SEQUENCE      10

static struct xmlToIdMapping s_Tokens[] =
{
	{"fo:block",              TT_BLOCK},
	{"fo:character",          TT_CHAR},
	{"fo:external-graphic",   TT_IMAGE},
	{"fo:flow",               TT_SECTION},
	{"fo:inline",             TT_INLINE},
	{"fo:layout-master-set",  TT_LAYOUT_MASTER_SET},
	{"fo:page-sequence",      TT_PAGE_SEQUENCE},
	{"fo:region-body",        TT_REGION_BODY},
	{"fo:root",               TT_DOCUMENT},
	{"fo:simple-page-master", TT_SIMPLE_PAGE_MASTER},
};

#define TokenTableSize	((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

/*****************************************************************/	
/*****************************************************************/	

#define X_TestParseState(ps)	((m_parseState==(ps)))

#define X_VerifyParseState(ps)	do {  if (!(X_TestParseState(ps)))			\
									  {  m_error = UT_IE_BOGUSDOCUMENT;	\
										 return; } } while (0)

#define X_CheckDocument(b)		do {  if (!(b))								\
									  {  m_error = UT_IE_BOGUSDOCUMENT;	\
										 return; } } while (0)

#define X_CheckError(v)			do {  if (!(v))								\
									  {  m_error = UT_ERROR;			\
										 return; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_error) return; } while (0)

/*****************************************************************/
/*****************************************************************/

#define USED() do {if(used) sBuf+="; "; else used = true;} while (0)

void IE_Imp_XSL_FO::_startElement(const XML_Char *name,
								  const XML_Char **atts)
{
	// xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();
	
	UT_uint32 tokenIndex = mapNameToToken (name, s_Tokens, TokenTableSize);

    const XML_Char * buf[3];
    const XML_Char ** p_atts;
	buf[0] = (XML_Char *)"props";
	buf[2] = 0;
	
	UT_String sBuf;
	XML_Char * pVal = 0;
	
	bool used = false;

	switch (tokenIndex)
	{
	case TT_DOCUMENT:
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;
		return;

	case TT_SECTION:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Sec;
		X_CheckError(m_pDocument->appendStrux(PTX_Section,
											  (const XML_Char **)NULL));
		return;

	case TT_BLOCK:
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		{
			pVal = (XML_Char*)_getXMLPropValue("background-color", atts);
			if (pVal)
			{
				USED();
				sBuf += "bgcolor:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("color", atts);
			if (pVal)
			{
				USED();
				sBuf += "color:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("language", atts);
			if (pVal)
			{
				USED();
				sBuf += "lang:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("font-size", atts);
			if (pVal)
			{
				USED();
				sBuf += "font-size:";
				sBuf += (char *)pVal;
			}
			
			pVal = (XML_Char*)_getXMLPropValue("font-family", atts);
			if (pVal)
			{
				USED();
				sBuf += "font-family:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("font-weight", atts);
			if (pVal)
			{
				USED();
				sBuf += "font-weight:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("font-style", atts);
			if (pVal)
			{
				USED();
				sBuf += "font-style:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("font-stretch", atts);
			if (pVal)
			{
				USED();
				sBuf += "font-stretch:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("keep-together", atts);
			if (pVal)
			{
				USED();
				sBuf += "keep-together:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("keep-with-next", atts);
			if (pVal)
			{
				USED();
				sBuf += "keep-with-next:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("line-height", atts);
			if (pVal)
			{
				USED();
				sBuf += "line-height:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("margin-bottom", atts);
			if (pVal)
			{
				USED();
				sBuf += "margin-bottom:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("margin-top", atts);
			if (pVal)
			{
				USED();
				sBuf += "margin-top:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("margin-left", atts);
			if (pVal)
			{
				USED();
				sBuf += "margin-left:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("margin-right", atts);
			if (pVal)
			{
				USED();
				sBuf += "margin-right:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("text-align", atts);
			if (pVal)
			{
				USED();
				sBuf += "text-align:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("widows", atts);
			if (pVal)
			{
				USED();
				sBuf += "widows:";
				sBuf += (char *)pVal;
			}

			buf[1] = sBuf.c_str();

			xxx_UT_DEBUGMSG(("FO import: block props='%s'\n", sBuf.c_str()));

			// append the atts/block to the document
			X_CheckError(m_pDocument->appendStrux(PTX_Block,buf));
		}
		break;

		// we treat both of these as if they were the same
		// they represent character-level formatting
	case TT_CHAR:
	case TT_INLINE:
		X_VerifyParseState(_PS_Block);
		{
			pVal = (XML_Char*)_getXMLPropValue("background-color", atts);
			if (pVal)
			{
				USED();
				sBuf += "bgcolor:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("color", atts);
			if (pVal)
			{
				USED();
				sBuf += "color:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("language", atts);
			if (pVal)
			{
				USED();
				sBuf += "lang:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("font-size", atts);
			if (pVal)
			{
				USED();
				sBuf += "font-size:";
				sBuf += (char *)pVal;
			}
			
			pVal = (XML_Char*)_getXMLPropValue("font-family", atts);
			if (pVal)
			{
				USED();
				sBuf += "font-family:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("font-weight", atts);
			if (pVal)
			{
				USED();
				sBuf += "font-weight:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("font-style", atts);
			if (pVal)
			{
				USED();
				sBuf += "font-style:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("font-stretch", atts);
			if (pVal)
			{
				USED();
				sBuf += "font-stretch:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("keep-together", atts);
			if (pVal)
			{
				USED();
				sBuf += "keep-together:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("keep-with-next", atts);
			if (pVal)
			{
				USED();
				sBuf += "keep-with-next:";
				sBuf += (char *)pVal;
			}

			pVal = (XML_Char*)_getXMLPropValue("text-decoration", atts);
			if (pVal)
			{
				USED();
				sBuf += "text-decoration:";
				sBuf += (char *)pVal;
			}

			buf[1] = sBuf.c_str();

			xxx_UT_DEBUGMSG(("FO import: inline props='%s'\n", sBuf.c_str()));

            p_atts = (const XML_Char **)buf;
			X_CheckError(_pushInlineFmt(p_atts));
			X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		}
		break;

		// here we set the page size
	case TT_SIMPLE_PAGE_MASTER:		
		X_VerifyParseState(_PS_Doc);
		{
			// TODO: we should do some cool stuff based on these prop=val keys:
			// margin-top, margin-bottom, margin-left, margin-right,
			// page-width, page-height
			m_pDocument->setDefaultPageSize();
		}
		break;

		// we should really try to get this working
        // at least with local graphics
	case TT_IMAGE:
		X_VerifyParseState(_PS_Block);
		{
			UT_ASSERT(UT_TODO);
		}
		break;

		// these we just plain ignore
	case TT_LAYOUT_MASTER_SET:
	case TT_REGION_BODY:
	case TT_PAGE_SEQUENCE:
		break;

	default:
		xxx_UT_DEBUGMSG(("Unknown or knowingly unhandled tag [%s]\n",name));
		break;
	}

}

#undef USED

void IE_Imp_XSL_FO::_endElement(const XML_Char *name)
{
	// xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();
	
   	UT_uint32 tokenIndex = mapNameToToken (name, s_Tokens, TokenTableSize);

	switch (tokenIndex)
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
		UT_ASSERT(m_lenCharDataSeen == 0);
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth() == 0);
		return;

	case TT_INLINE:
	case TT_CHAR:
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth() > 0);
		_popInlineFmt();
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

	case TT_IMAGE:
		X_VerifyParseState(_PS_Block);
		return;

	case TT_OTHER:
	case TT_LAYOUT_MASTER_SET:
	case TT_SIMPLE_PAGE_MASTER:
	case TT_REGION_BODY:
	case TT_PAGE_SEQUENCE:
	default:
		xxx_UT_DEBUGMSG(("Unknown or intentionally unhandled end tag [%s]\n",name));
		break;
	}
}

