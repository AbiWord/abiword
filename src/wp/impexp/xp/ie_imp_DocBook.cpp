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
#include <string.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_DocBook.h"
#include "ie_types.h"
#include "pd_Document.h"
#include "ut_growbuf.h"

/*
 * DocBook is a SGML derivate with lots of friggin' tags
 * We hardly support any of them now, only the ones we export
 */

/*****************************************************************/
/*****************************************************************/

IE_Imp_DocBook::~IE_Imp_DocBook()
{
}

IE_Imp_DocBook::IE_Imp_DocBook(PD_Document * pDocument)
	: IE_Imp_XML(pDocument)
{
  // white space is not significant
  m_bWhiteSignificant = UT_FALSE;
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Imp_DocBook::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
  // no doubt, this could be better
  // but this should suffice for all I care

  if(strstr(szBuf, "<!DOCTYPE book") == NULL && strstr(szBuf, "<!doctype book") == NULL)
    return UT_FALSE;

  if(strstr(szBuf, "<book") == NULL)
    return UT_FALSE;

  return UT_TRUE;
}

UT_Bool IE_Imp_DocBook::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".dbk") == 0);
}

UT_Error IE_Imp_DocBook::StaticConstructor(PD_Document * pDocument,
					IE_Imp ** ppie)
{
	IE_Imp_DocBook * p = new IE_Imp_DocBook(pDocument);
	*ppie = p;
	return UT_OK;
}

UT_Bool	IE_Imp_DocBook::GetDlgLabels(const char ** pszDesc,
				  const char ** pszSuffixList,
				  IEFileType * ft)
{
	*pszDesc = "DocBook (.dbk)";
	*pszSuffixList = "*.dbk";
	*ft = IEFT_DocBook;
	return UT_TRUE;
}

UT_Bool IE_Imp_DocBook::SupportsFileType(IEFileType ft)
{
	return (IEFT_DocBook == ft);
}

/*****************************************************************/
/*****************************************************************/

#define TT_OTHER		0               // anything else
#define TT_DOCUMENT	        1		// a document <wml>
#define TT_SECTION              2               // card or section
#define TT_BLOCK		3		// a paragraph <p>
#define TT_PHRASE               4               // formatted text
#define TT_EMPHASIS             5               // emphasized (italic) text
#define TT_SUPERSCRIPT          6               // superscript
#define TT_SUBSCRIPT            7               // subscript
#define TT_BLOCKQUOTE           8               // block quote

struct _TokenTable
{
	const char *	m_name;
	int             m_type;
};

static struct _TokenTable s_Tokens[] =
{
	{	"book",			TT_DOCUMENT		},
	{       "chapter",              TT_SECTION              },
	{	"para",			TT_BLOCK		},
	{       "phrase",               TT_PHRASE               },
	{       "emphasis",             TT_EMPHASIS             },
	{       "superscript",          TT_SUPERSCRIPT          },
	{       "subscript",            TT_SUBSCRIPT            },
	{	"*",			TT_OTHER		}
};

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

void IE_Imp_DocBook::_startElement(const XML_Char *name,
				   const XML_Char **atts)
{
	UT_DEBUGMSG(("DocBook import: startElement: %s\n", name));

	// xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();
	
	UT_uint32 tokenIndex = s_mapNameToToken(name);
	switch (s_Tokens[tokenIndex].m_type)
	{
	case TT_DOCUMENT:
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;
		return;

	case TT_SECTION:
	  {
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Sec;
		
		// I'm torn as to where to put this:
		// TT_DOCUMENT or here. Oh well :-)
		X_CheckError(m_pDocument->appendStrux(PTX_Section,(const XML_Char **)NULL));
		return;
	  }

	case TT_BLOCK:
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		X_CheckError(m_pDocument->appendStrux(PTX_Block, NULL));
		return;
		
	case TT_BLOCKQUOTE:
	        X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		{
		  const XML_Char **p_atts;
		  XML_Char *buf[3];
		  buf[2] = NULL;

		  X_CheckError(m_pDocument->appendStrux(PTX_Block, NULL));
		  UT_XML_cloneString(buf[0], PT_STYLE_ATTRIBUTE_NAME);
		  UT_XML_cloneString(buf[1], (XML_Char *)"Block Text");
		  p_atts = (const XML_Char **)buf;
		  X_CheckError(m_pDocument->appendFmt(p_atts));
		  return;
		}

	case TT_PHRASE:
	case TT_EMPHASIS:
	case TT_SUPERSCRIPT:
	case TT_SUBSCRIPT:
	  X_VerifyParseState(_PS_Block);
	  {
	    const XML_Char **p_atts;
	    XML_Char *buf[3];
	    UT_XML_cloneString(buf[0], (XML_Char *)"props");
	    buf[2] = NULL;

	    switch(s_Tokens[tokenIndex].m_type) {
	    case TT_EMPHASIS: 
	      UT_XML_cloneString(buf[1], (XML_Char *)"font-style:italic"); 
	      break;
	    case TT_SUPERSCRIPT: 
	       UT_XML_cloneString(buf[1], (XML_Char *)"text-position:superscript"); 
	      break;
	    case TT_SUBSCRIPT: 
	       UT_XML_cloneString(buf[1], (XML_Char *)"text-position:subscript"); 
	      break;
	    case TT_PHRASE:
	      {
		const XML_Char *p_val = _getXMLPropValue("role", atts);
		if(p_val != NULL && !strcmp(p_val, "strong"))
		  UT_XML_cloneString(buf[1],  (XML_Char *)"font-weight:bold");
		else
		  buf[0] = NULL;
		break;
	      }

	    default:
	      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	      break;
	    }

	    p_atts = (const XML_Char **)buf;
	    X_CheckError(_pushInlineFmt(p_atts));
	    X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
	  }
	  return;

	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown or knowingly unhandled tag [%s]\n",name));
	}

}

void IE_Imp_DocBook::_endElement(const XML_Char *name)
{
  
        UT_DEBUGMSG(("DocBook import: endElement: %s\n", name));

        // xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();
	
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

	case TT_BLOCKQUOTE:
	        UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Block;
		X_CheckDocument(_getInlineDepth()==0);
		_popInlineFmt();
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

	case TT_BLOCK:
	        UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth()==0);
		return;
		
	case TT_PHRASE:
	case TT_EMPHASIS:
	case TT_SUPERSCRIPT:
	case TT_SUBSCRIPT:
		UT_ASSERT(m_lenCharDataSeen==0);
	        X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()>0);
		_popInlineFmt();
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown or intentionally unhandled end tag [%s]\n",name));
	}
}

