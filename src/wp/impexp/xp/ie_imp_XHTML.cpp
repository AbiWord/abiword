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
#include "ie_imp_XHTML.h"
#include "ie_types.h"
#include "pd_Document.h"
#include "ut_bytebuf.h"

/*****************************************************************/
/*****************************************************************/

IE_Imp_XHTML::IE_Imp_XHTML(PD_Document * pDocument)
	: IE_Imp_XML(pDocument)
{
  // white space is not significant
  m_bWhiteSignificant = UT_FALSE;

}

IE_Imp_XHTML::~IE_Imp_XHTML()
{
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Imp_XHTML::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	UT_uint32 iLinesToRead = 6 ;  // Only examine the first few lines of the file
	UT_uint32 iBytesScanned = 0 ;
	const char *p ;
	char *magic ;
	p = szBuf ;
	while( iLinesToRead-- )
	{
		magic = "<html " ;
		if ( (iNumbytes - iBytesScanned) < strlen(magic) ) return(UT_FALSE);
		if ( strncmp(p, magic, strlen(magic)) == 0 ) return(UT_TRUE);
		magic = "<!DOCTYPE html" ;
		if ( (iNumbytes - iBytesScanned) < strlen(magic) ) return(UT_FALSE);
		if ( strncmp(p, magic, strlen(magic)) == 0 ) return(UT_TRUE);
		/*  Seek to the next newline:  */
		while ( *p != '\n' && *p != '\r' )
		{
			iBytesScanned++ ; p++ ;
			if( iBytesScanned+2 >= iNumbytes ) return(UT_FALSE);
		}
		/*  Seek past the next newline:  */
		if ( *p == '\n' || *p == '\r' )
		{
			iBytesScanned++ ; p++ ;
			if ( *p == '\n' || *p == '\r' )
			{
				iBytesScanned++ ; p++ ;
			}
		}
	}
	return(UT_FALSE);
}

UT_Bool IE_Imp_XHTML::RecognizeSuffix(const char * szSuffix)
{
	return ((UT_stricmp(szSuffix,".html") == 0) || (UT_stricmp(szSuffix,".xhtml") == 0));
}

UT_Error IE_Imp_XHTML::StaticConstructor(PD_Document * pDocument,
											 IE_Imp ** ppie)
{
	IE_Imp_XHTML * p = new IE_Imp_XHTML(pDocument);
	*ppie = p;
	return UT_OK;
}

UT_Bool	IE_Imp_XHTML::GetDlgLabels(const char ** pszDesc,
									   const char ** pszSuffixList,
									   IEFileType * ft)
{
	*pszDesc = "XHTML (.html)";
	*pszSuffixList = "*.html";
	*ft = IEFT_XHTML;
	return UT_TRUE;
}

UT_Bool IE_Imp_XHTML::SupportsFileType(IEFileType ft)
{
	return (IEFT_XHTML == ft);
}

/*****************************************************************/
/*****************************************************************/

#define TT_OTHER		0
#define TT_DOCUMENT		1		// a document <html>
#define TT_DIV  		2		// a section <div>
#define TT_P    		3		// a paragraph <p>
#define TT_INLINE		4		// inline span of text <span>
#define TT_BREAK		5		// a forced line-break <br>
#define TT_BODY                 6               // a body <body>
#define TT_TR                   7               // a table row <tr>
#define TT_H1                   8               // heading 1 <h1>
#define TT_H2                   9               // heading 2 <h2>
#define TT_H3                   10              // heading 3 <h3>
#define TT_H4                   11              // heading 4 <h4>
#define TT_H5                   12              // heading 5 <h5>
#define TT_H6                   13              // heading 6 <h6>
#define TT_B                    14              // bold <b>
#define TT_I                    15              // italic <i>

// This certainly leaves off lots of tags, but with HTML, this is inevitable - samth

struct _TokenTable
{
	const char *	m_name;
	int				m_type;
};

static struct _TokenTable s_Tokens[] =
{
	{	"html",	        	TT_DOCUMENT		},
	{	"div",		        TT_DIV		        },
	{	"p",			TT_P	        	},
	{	"span",			TT_INLINE		},
	{	"br",			TT_BREAK		},
	{       "body",                 TT_BODY                 },
	{       "tr",                   TT_TR                   },
	{       "h1",                   TT_H1                   },
	{       "h2",                   TT_H2                   },
	{       "h3",                   TT_H3                   },
	{       "h4",                   TT_H4                   },
	{       "h5",                   TT_H5                   },
	{       "h6",                   TT_H6                   },
	{       "b",                    TT_B                    },
	{       "i",                    TT_I                    },
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
				{  m_error = UT_IE_BOGUSDOCUMENT;	\
				   return; } } while (0)

#define X_CheckDocument(b)		do {  if (!(b))										  {  m_error = UT_IE_BOGUSDOCUMENT;											 return; } } while (0)

#define X_CheckError(v)			do {  if (!(v))								\
									  {  m_error = UT_ERROR;			\
										 return; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_error) return; } while (0)

/*****************************************************************/
/*****************************************************************/

void IE_Imp_XHTML::_startElement(const XML_Char *name, const XML_Char **atts)
{
	atts = 0;                                      // we ignore HTML attributes for now - this will be fixed

	UT_DEBUGMSG(("startElement: %s\n", name));

	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed
	                                                // this just avoids all the processing if there is an error

 	const XML_Char **  new_atts = NULL;
	XML_Char * sz = NULL;

	UT_Bool setProps = UT_FALSE;

	UT_uint32 tokenIndex = s_mapNameToToken(name);
	switch (s_Tokens[tokenIndex].m_type)
	{
	case TT_DOCUMENT:
		UT_DEBUGMSG(("Init %d\n", m_parseState));
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;
		return;

	case TT_BODY:
		UT_DEBUGMSG(("Doc %d\n", m_parseState));
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Block;

		//UT_DEBUGMSG(("%d atts: %s\n", i, atts[i]));
		X_CheckError(m_pDocument->appendStrux(PTX_Section,atts));
		X_CheckError(m_pDocument->appendStrux(PTX_Block,atts));
		return;		

	case TT_DIV:
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Block;
		X_CheckError(m_pDocument->appendStrux(PTX_Section,atts));
		X_CheckError(m_pDocument->appendStrux(PTX_Block,atts));
		return;

	case TT_I:
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		m_currentFmt.isBold = UT_TRUE;
		setProps = UT_TRUE;
		UT_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
		new_atts = (const XML_Char **)realloc(new_atts, 2);
		new_atts[0]=sz;
		sz = NULL;
		UT_cloneString(sz, "font-style:italic");
		new_atts[1]=sz;
		X_CheckError(m_pDocument->appendFmt(new_atts));
		return;

	case TT_B:
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		m_currentFmt.isBold = UT_TRUE;
		setProps = UT_TRUE;
		UT_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
		new_atts = (const XML_Char **)realloc(new_atts, 2);
		new_atts[0]=sz;
		sz = NULL;
		UT_cloneString(sz, "font-weight:bold");
		new_atts[1]=sz;
		X_CheckError(m_pDocument->appendFmt(new_atts));
		return;

	case TT_H1:
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		X_CheckError(m_pDocument->appendStrux(PTX_Block,atts));
		UT_cloneString(sz, PT_STYLE_ATTRIBUTE_NAME);
		new_atts = (const XML_Char **)realloc(new_atts, 2);
		new_atts[0]=sz;
		sz = NULL;
		UT_cloneString(sz, "Heading 1");
		new_atts[1]=sz;
		X_CheckError(m_pDocument->appendFmt(new_atts));
		return;

	case TT_H2:
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		X_CheckError(m_pDocument->appendStrux(PTX_Block,atts));
		UT_cloneString(sz, PT_STYLE_ATTRIBUTE_NAME);
		new_atts = (const XML_Char **)realloc(new_atts, 2);
		new_atts[0]=sz;
		sz = NULL;
		UT_cloneString(sz, "Heading 2");
		new_atts[1]=sz;
		X_CheckError(m_pDocument->appendFmt(new_atts));
		return;


	case TT_H3:
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		X_CheckError(m_pDocument->appendStrux(PTX_Block,atts));
		UT_cloneString(sz, PT_STYLE_ATTRIBUTE_NAME);
		new_atts = (const XML_Char **)realloc(new_atts, 2);
		new_atts[0]=sz;
		sz = NULL;
		UT_cloneString(sz, "Heading 3");
		new_atts[1]=sz;
		X_CheckError(m_pDocument->appendFmt(new_atts));
		return;

	case TT_P:
	case TT_TR:
	case TT_H4:
	case TT_H5:
	case TT_H6:
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Block;
		X_CheckError(m_pDocument->appendStrux(PTX_Block,atts));
		return;
		
	case TT_INLINE:
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		X_CheckError(_pushInlineFmt(atts));
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

	case TT_BREAK:
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		{
			UT_UCSChar ucs = UCS_LF;
			X_CheckError(m_pDocument->appendSpan(&ucs,1));
		}
		return;

	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown tag [%s]\n",name));

		//It's imperative that we keep processing after finding an unknown element

		return;
	}
}

void IE_Imp_XHTML::_endElement(const XML_Char *name)
{
	UT_DEBUGMSG(("endElement: %s\n", name));
	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed

	
	UT_uint32 tokenIndex = s_mapNameToToken(name);
	if(name == "html") UT_DEBUGMSG(("tokenindex : %d\n", tokenIndex));
	switch (s_Tokens[tokenIndex].m_type)
	{
	case TT_DOCUMENT:
		UT_DEBUGMSG(("Init %d\n", m_parseState));
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Init;
		return;

	case TT_BODY:
		UT_DEBUGMSG(("Doc %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Doc;
		return;

	case TT_DIV:
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Block;
		return;

	case TT_P:
	case TT_TR:
	case TT_H1:
	case TT_H2:
	case TT_H3:
	case TT_H4:
	case TT_H5:
	case TT_H6:
		UT_ASSERT(m_lenCharDataSeen==0);
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Block;
		X_CheckDocument(_getInlineDepth()==0);
		_popInlineFmt();
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

	case TT_B:
	case TT_I:
		UT_ASSERT(m_lenCharDataSeen==0);
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()==0);
		//_popInlineFmt();
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;
		
	case TT_INLINE:
		UT_ASSERT(m_lenCharDataSeen==0);
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()>0);
		_popInlineFmt();
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

	case TT_BREAK:						// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		return;

	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown end tag [%s]\n",name));
		return;
	}
}



