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

// bold elements
#define TT_B                    14              // bold <b>
#define TT_EM                   15              // emphasis
#define TT_STRONG               16              // strong
#define TT_DFN                  17              // definitional
#define TT_CODE                 18              // programming language code

// italic elements
#define TT_I                    19              // italic <i>
#define TT_ADDRESS              20              // author's address
#define TT_SAMP                 21              // sample
#define TT_KBD                  22              // keyboard
#define TT_VAR                  23              // variable (programming)
#define TT_CITE                 24              // citation
#define TT_Q                    25              // quote

#define TT_SUP                  26              // superscript
#define TT_SUB                  27              // subscript
#define TT_S                    28              // strike-through
#define TT_STRIKE               29              // strike-through

#define TT_FONT                 30              // mother of all...
#define TT_BLOCKQUOTE           31              // blockquote
#define TT_UNDERLINE            32              // underline

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
	{       "em",                   TT_EM                   },
	{       "strong",               TT_STRONG               },
	{       "def",                  TT_DFN                  },
	{       "code",                 TT_CODE                 },
	{       "i",                    TT_I                    },
	{       "address",              TT_ADDRESS              },
	{       "samp",                 TT_SAMP                 },
	{       "kbd",                  TT_KBD                  },
	{       "var",                  TT_VAR                  },
	{       "cite",                 TT_CITE                 },
	{       "q",                    TT_Q                    },
	{       "sup",                  TT_SUP                  },
	{       "sub",                  TT_SUB                  },
	{       "s",                    TT_S                    },
	{       "strike",               TT_STRIKE               },
	{       "font",                 TT_FONT                 },
	{       "blockquote",           TT_BLOCKQUOTE           },
	{       "u",                    TT_UNDERLINE            },
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

static void convertFontFace(char *szDest, const char *szFrom)
{
  // TODO: make me better
  // TODO: and handle things like comma lists of font faces
  char *newFont;

  // default...
  if(szFrom == NULL)
    newFont = "Times New Roman";
  else
    newFont = (char*)szFrom;

  strcpy(szDest, newFont);
}

static void convertFontSize(char *szDest, const char *szFrom)
{
  // if it starts with a +
  // if it starts with a -
  // if it is a number, clamp it

  int sz = 12;

  if(szFrom == NULL)
    {
      sz = 12;
    }
  else if(UT_UCS_isdigit(*szFrom))
    {
      sz = atoi(szFrom);
    }
  else if(*szFrom == '+')
    {
      switch(atoi(szFrom+1))
	{
	case 1:
	  sz = 10; break;
	case 2:
	  sz = 12; break;
	case 3:
	  sz = 16; break;
	case 4:
	  sz = 20; break;
	case 5:
	  sz = 24; break;
	case 6:
	  sz = 36; break;
	default:
	  sz = 12; break;
	}
    }
  else if(*szFrom == '-')
    {
      switch(atoi(szFrom+1))
	{
	case 1:
	  sz = 9; break;
	default:
	  sz = 8; break;
	}
    }

  // clamp the value
  if(sz > 36)
    sz = 36;
  else if(sz < 8)
    sz = 8;

  sprintf(szDest, "%2d", sz);
}

static void convertFontColor(char *szDest, const char *szFrom)
{

  // if it starts with a #, send it through
  // if it starts with a number, send it through
  // else convert color from values in XHTML DTD

  int col = 0; // black

  if(szFrom == NULL)
    {
      col = 0;
    }
  if(*szFrom == '#')
    {
      col = atoi(szFrom+1);
    }
  else if(UT_UCS_isdigit(*szFrom))
    {
      col = atoi(szFrom);
    }
  else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"green"))
    {
      col = 0x008000;
    }
  else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"silver"))
    {
      col = 0xC0C0C0;
    }
  else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"lime"))
    {
      col = 0x00FF00;
    }
   else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"gray"))
    {
      col = 0x808080;
    }
  else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"olive"))
    {
      col = 0x808000;
    }
  else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"white"))
    {
      col = 0xFFFFFF;
    }
  else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"yellow"))
    {
      col = 0xFFFF00;
    }
  else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"maroon"))
    {
      col = 0x800000;
    }
  else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"navy"))
    {
      col = 0x000080;
    }
  else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"red"))
    {
      col = 0xFF0000;
    }
   else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"blue"))
    {
      col = 0x0000FF;
    }
  else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"purple"))
    {
      col = 0x800080;
    }
  else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"teal"))
    {
      col = 0x008080;
    }
  else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"fuchsia"))
    {
      col = 0xFF00FF;
    }
  else if(!UT_XML_stricmp((XML_Char *)szFrom, (XML_Char *)"aqua"))
    {
      col = 0x00FFFF;
    }

  sprintf(szDest, "%6x", col);
}

/*****************************************************************/
/*****************************************************************/

void IE_Imp_XHTML::_startElement(const XML_Char *name, const XML_Char **atts)
{
	UT_DEBUGMSG(("startElement: %s\n", name));

	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed
	                                                // this just avoids all the processing if there is an error

#define NEW_ATTR_SZ 3
 	const XML_Char *new_atts[NEW_ATTR_SZ];
	XML_Char * sz = NULL;

	for(int i = 0; i < NEW_ATTR_SZ; i++)
	  new_atts[i] = NULL;
#undef NEW_ATTR_SZ

	UT_uint32 tokenIndex = s_mapNameToToken(name);
	switch (s_Tokens[tokenIndex].m_type)
	{
	case TT_DOCUMENT:
	  //UT_DEBUGMSG(("Init %d\n", m_parseState));
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;
		return;

	case TT_BODY:
	  //UT_DEBUGMSG(("Doc %d\n", m_parseState));
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Block;

		//UT_DEBUGMSG(("%d atts: %s\n", i, atts[i]));
		X_CheckError(m_pDocument->appendStrux(PTX_Section,NULL));
		X_CheckError(m_pDocument->appendStrux(PTX_Block,NULL));
		return;		

	case TT_DIV:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Block;
		X_CheckError(m_pDocument->appendStrux(PTX_Section,NULL));
		X_CheckError(m_pDocument->appendStrux(PTX_Block,NULL));
		return;

	case TT_Q:
	case TT_SAMP:
	case TT_VAR:
	case TT_KBD:
	case TT_ADDRESS:
	case TT_CITE:
	case TT_I:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
		new_atts[0]=sz;
		sz = NULL;
		UT_XML_cloneString(sz, "font-style:italic");
		new_atts[1]=sz;
		X_CheckError(m_pDocument->appendFmt(new_atts));
		return;

	case TT_CODE:
	case TT_DFN:
	case TT_STRONG:
	case TT_EM:
	case TT_B:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
		new_atts[0]=sz;
		sz = NULL;
		UT_XML_cloneString(sz, "font-weight:bold");
		new_atts[1]=sz;
		X_CheckError(m_pDocument->appendFmt(new_atts));
		return;

	case TT_UNDERLINE:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
		new_atts[0]=sz;
		sz = NULL;
		UT_XML_cloneString(sz, "text-decoration:underline");
		new_atts[1]=sz;
		X_CheckError(m_pDocument->appendFmt(new_atts));
		return;

	case TT_SUP:
	case TT_SUB:
	  //UT_DEBUGMSG(("Super or Subscript\n"));
	  X_VerifyParseState(_PS_Block);
	  UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
	  new_atts[0]=sz;
	  sz = NULL;
	  if(s_Tokens[tokenIndex].m_type==TT_SUP)    
	    UT_XML_cloneString(sz, "text-position:superscript");
	  else
	    UT_XML_cloneString(sz, "text-position:subscript");
	  new_atts[1]=sz;
	  X_CheckError(m_pDocument->appendFmt(new_atts));
	  return;

	case TT_S:
	case TT_STRIKE:
	  //UT_DEBUGMSG(("Strike\n"));
	  X_VerifyParseState(_PS_Block);
	  UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
	  new_atts[0]=sz;
	  sz = NULL;
	  UT_XML_cloneString(sz, "text-decoration:line-through");
	  new_atts[1]=sz;
	  X_CheckError(m_pDocument->appendFmt(new_atts));
	  return;
  
	case TT_FONT:
	  UT_DEBUGMSG(("Font tag encountered\n"));
	  {
	    const XML_Char *p_val;
	    char color[7], size[3], face[64];
	    XML_Char output[128];

	    p_val = _getXMLPropValue("color", atts);
	    convertFontColor(color, p_val);

	    p_val = _getXMLPropValue("size", atts);
	    convertFontSize(size, p_val);

	    p_val = _getXMLPropValue("face", atts);
	    convertFontFace(face, p_val);

	    sprintf(output, "color:%s; font-family:%s; size:%spt", color, face, size);
	    UT_DEBUGMSG(("Font properties: %s\n", output));

	    UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
	    new_atts[0] = sz;
	    sz = NULL;
	    UT_XML_cloneString(sz, output);
	    new_atts[1] = sz;
	    X_CheckError(m_pDocument->appendFmt(new_atts));
	  }
	  return;

	case TT_BLOCKQUOTE:
	case TT_H1:
	case TT_H2:
	case TT_H3:
	  {
	    //    UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);

		const XML_Char *p_val;

		p_val = _getXMLPropValue("align", atts);
		if(p_val == NULL)
		  X_CheckError(m_pDocument->appendStrux(PTX_Block,NULL));
		else
		  {
		    sz = NULL;
		    
		    if(!UT_XML_stricmp(p_val, "right"))
		      UT_XML_cloneString(sz, "text-align:right");
		    else if(!UT_XML_stricmp(p_val, "center"))
		      UT_XML_cloneString(sz, "text-align:center");
		    
		    if(sz != NULL)
		      {
			new_atts[1] = sz;
			UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
			new_atts[0] = sz;		      
		      }

		    X_CheckError(m_pDocument->appendStrux(PTX_Block,new_atts));
		  }

		UT_XML_cloneString(sz, PT_STYLE_ATTRIBUTE_NAME);
		new_atts[0]=sz;
		sz = NULL;

		if(s_Tokens[tokenIndex].m_type == TT_H1)
		  UT_XML_cloneString(sz, "Heading 1");
		else if(s_Tokens[tokenIndex].m_type ==TT_H2)
		  UT_XML_cloneString(sz, "Heading 2");
		else if(s_Tokens[tokenIndex].m_type == TT_H3)
		  UT_XML_cloneString(sz, "Heading 3");
		else
		  UT_XML_cloneString(sz, "Block Text");

		new_atts[1]=sz;
		X_CheckError(m_pDocument->appendFmt(new_atts));
		return;
	  }

	case TT_P:
	case TT_TR:
	case TT_H4:
	case TT_H5:
	case TT_H6:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
	  {
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Block;
		
		const XML_Char *p_val;

		p_val = _getXMLPropValue("align", atts);
		if(p_val == NULL)
		  X_CheckError(m_pDocument->appendStrux(PTX_Block,NULL));
		else
		  {
		    sz = NULL;

		    if(!UT_XML_stricmp(p_val, "right"))
		      UT_XML_cloneString(sz, "text-align:right");
		    else if(!UT_XML_stricmp(p_val, "center"))
		      UT_XML_cloneString(sz, "text-align:center");
		    
		    if(sz != NULL)
		      {
			new_atts[1] = sz;
			UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
			new_atts[0] = sz;
		      }

		    X_CheckError(m_pDocument->appendStrux(PTX_Block,new_atts));
		  }
		return;
	  }

	case TT_INLINE:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		const XML_Char *p_val;

		p_val = _getXMLPropValue("style", atts);
		if(p_val)
		  {
		    UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
		    new_atts[0] = sz;
		    sz = NULL;
		    UT_XML_cloneString(sz, p_val);
		    new_atts[1] = sz;
		  }

		X_CheckError(_pushInlineFmt(new_atts));
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

	case TT_BREAK:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
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
	  //UT_DEBUGMSG(("Init %d\n", m_parseState));
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Init;
		return;

	case TT_BODY:
	  //UT_DEBUGMSG(("Doc %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Doc;
		return;

	case TT_DIV:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
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
	case TT_BLOCKQUOTE:
	  UT_ASSERT(m_lenCharDataSeen==0);
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Block;
		X_CheckDocument(_getInlineDepth()==0);
		_popInlineFmt();
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

		// text formatting
	case TT_Q:
	case TT_SAMP:
	case TT_VAR:
	case TT_KBD:
	case TT_ADDRESS:
	case TT_CITE:
	case TT_CODE:
	case TT_DFN:
	case TT_STRONG:
	case TT_EM:	
	case TT_S:
	case TT_STRIKE:
	case TT_SUP:
	case TT_SUB:
	case TT_B:
	case TT_I:
	case TT_UNDERLINE:
	case TT_FONT:
		UT_ASSERT(m_lenCharDataSeen==0);
		//UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()==0);
		//_popInlineFmt();
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;
		
	case TT_INLINE:
		UT_ASSERT(m_lenCharDataSeen==0);
		//UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()>0);
		_popInlineFmt();
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;

	case TT_BREAK:						// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		//UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		return;

	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown end tag [%s]\n",name));
		return;
	}
}



