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
#include "ie_imp_WML.h"
#include "ie_types.h"
#include "pd_Document.h"
#include "ut_growbuf.h"

/*
 * This file is meant to import WML documents.
 * WML is an XML derivate and is the standard markup
 * language for wireless communication devices such
 * as web-phones and other PDAs.
 *
 * The correspondence between WML and Word processing
 * documents isn't as good as say, HTML is, but everyone
 * gets a stiffie now when you mention the word "wireless"
 * in front of anything, so...
 */

/*****************************************************************/
/*****************************************************************/

IE_Imp_WML::~IE_Imp_WML()
{
}

IE_Imp_WML::IE_Imp_WML(PD_Document * pDocument)
	: IE_Imp_XML(pDocument)
{
  // white space is not significant
  m_bWhiteSignificant = UT_FALSE;
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Imp_WML::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
  // first look that the first line is "<?xml "
  // then search for "<wml"
  // no doubt, this could be better
  // but this should suffice

  if(strncmp(szBuf, "<?xml ", strlen("<?xml ")))
    return UT_FALSE;

  if(strstr(szBuf, "<wml") == NULL)
    return UT_FALSE;

  return UT_TRUE;
}

UT_Bool IE_Imp_WML::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".wml") == 0);
}

UT_Error IE_Imp_WML::StaticConstructor(PD_Document * pDocument,
					IE_Imp ** ppie)
{
	IE_Imp_WML * p = new IE_Imp_WML(pDocument);
	*ppie = p;
	return UT_OK;
}

UT_Bool	IE_Imp_WML::GetDlgLabels(const char ** pszDesc,
				  const char ** pszSuffixList,
				  IEFileType * ft)
{
	*pszDesc = "WML (.wml)";
	*pszSuffixList = "*.wml";
	*ft = IEFT_Text;
	return UT_TRUE;
}

UT_Bool IE_Imp_WML::SupportsFileType(IEFileType ft)
{
	return (IEFT_WML == ft);
}

/*****************************************************************/
/*****************************************************************/

#define TT_OTHER		0               // anything else
#define TT_DOCUMENT	        1		// a document <wml>
#define TT_SECTION              2               // card or section
#define TT_BLOCK		3		// a paragraph <p>
#define TT_IMAGE		4		// an image object <img>
#define TT_BREAK		5		// a forced line-break <br/>
#define TT_BOLD                 6               // bold text
#define TT_ITALIC               7               // italic text
#define TT_UNDERLINE            8               // underlined text
#define TT_STRONG               9               // strong(bold) text
#define TT_EMPHASIS             10              // emphasis(bold) text
#define TT_BIG                  11              // big(super script) text
#define TT_SMALL                12              // small(sub script) text

struct _TokenTable
{
	const char *	m_name;
	int             m_type;
};

static struct _TokenTable s_Tokens[] =
{
	{	"wml",			TT_DOCUMENT		},
	{       "card",                 TT_SECTION              },
	{	"p",			TT_BLOCK		},
	{	"img",		        TT_IMAGE		},
	{	"br",			TT_BREAK		},
	{       "b",                    TT_BOLD                 },
	{       "i",                    TT_ITALIC               },
	{       "u",                    TT_UNDERLINE            },
	{       "strong",               TT_STRONG               },
	{       "em",                   TT_EMPHASIS             },
	{       "big",                  TT_BIG                  },
	{       "small",                TT_SMALL                },
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

void IE_Imp_WML::_startElement(const XML_Char *name,
			       const XML_Char **atts)
{
	UT_DEBUGMSG(("WML import: startElement: %s\n", name));

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
		{
		  XML_Char *p_val;
		  XML_Char *buf[3];
		  UT_Bool left = UT_FALSE;

		  UT_XML_cloneString(buf[0], (XML_Char*)"props");
		  buf[2] = NULL;

		  p_val = (XML_Char*)_getXMLPropValue("align", atts);
		  if(!p_val || !atts) {
		    UT_DEBUGMSG(("WML: got <p> with no props\n"));
		    left = UT_TRUE;
		  }
		  else {
		    if(!UT_XML_strcmp(p_val, (XML_Char *)"center"))
		      {
			UT_XML_cloneString(buf[1], (XML_Char*)"text-align:center");
		      }
		    else if(!UT_XML_strcmp(p_val, (XML_Char *)"right"))
		      {
			UT_XML_cloneString(buf[1], (XML_Char*)"text-align:right");
		      }
		    else
		      left = UT_TRUE;
		  }
		  X_CheckError(m_pDocument->appendStrux(PTX_Block, (left ? NULL : (const XML_Char **)buf)));
		}
		return;
		
	case TT_IMAGE:
	       	// todo: we don't do images yet and probably never will
		X_VerifyParseState(_PS_Block);
		return;

	case TT_BREAK:
		X_VerifyParseState(_PS_Block);
		{
			UT_UCSChar ucs = UCS_LF;
			X_CheckError(m_pDocument->appendSpan(&ucs,1));
		}
		return;

	case TT_ITALIC:
	case TT_UNDERLINE:
	case TT_BOLD:
	case TT_STRONG:
	case TT_EMPHASIS:
	case TT_BIG:
	case TT_SMALL:
	  X_VerifyParseState(_PS_Block);
	  {
	    const XML_Char **p_atts;
	    XML_Char *buf[3];
	    UT_XML_cloneString(buf[0], (XML_Char *)"props");
	    buf[2] = NULL;

	    switch(s_Tokens[tokenIndex].m_type) {
	    case TT_ITALIC: 
	      UT_XML_cloneString(buf[1], (XML_Char *)"font-style:italic"); 
	      break;
	    case TT_UNDERLINE: 
	       UT_XML_cloneString(buf[1], (XML_Char *)"text-decoration:underline"); 
	      break;
	    case TT_BOLD:
	    case TT_STRONG:
	    case TT_EMPHASIS:
	       UT_XML_cloneString(buf[1],  (XML_Char *)"font-weight:bold"); 
	      break;
	    case TT_BIG: 
	       UT_XML_cloneString(buf[1], (XML_Char *)"text-position:superscript"); 
	      break;
	    case TT_SMALL: 
	       UT_XML_cloneString(buf[1], (XML_Char *)"text-position:subscript"); 
	      break;

	    default:
	      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	      break;
	    }

	    p_atts = (const XML_Char **)&buf;
	    X_CheckError(_pushInlineFmt(p_atts));
	    X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
	  }
	  return;

	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown or knowingly unhandled tag [%s]\n",name));
	}

}

void IE_Imp_WML::_endElement(const XML_Char *name)
{
  
        UT_DEBUGMSG(("WML import: endElement: %s\n", name));

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

	case TT_BLOCK:
	        UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth()==0);
		return;
		
	case TT_IMAGE:
		X_VerifyParseState(_PS_Block);
		return;

	case TT_BREAK:
		X_VerifyParseState(_PS_Block);
		return;

	case TT_ITALIC:
	case TT_UNDERLINE:
	case TT_BOLD:
	case TT_STRONG:
	case TT_EMPHASIS:
	case TT_BIG:
	case TT_SMALL:
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

