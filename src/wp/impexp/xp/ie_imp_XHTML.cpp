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
#include "ut_string_class.h"
#include "ie_imp_XHTML.h"
#include "ie_types.h"
#include "pd_Document.h"
#include "ut_bytebuf.h"

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

ABI_PLUGIN_DECLARE("HTML")

// we use a reference-counted sniffer
static IE_Imp_XHTML_Sniffer * m_sniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Imp_XHTML_Sniffer ();
	}
	else
	{
		m_sniffer->ref();
	}

	mi->name = "XHTML Importer";
	mi->desc = "Import XHTML Documents";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Imp::registerImporter (m_sniffer);
	return 1;
}

ABI_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	UT_ASSERT (m_sniffer);

	IE_Imp::unregisterImporter (m_sniffer);
	if (!m_sniffer->unref())
	{
		m_sniffer = 0;
	}

	return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
								 UT_uint32 release)
{
  return 1;
}

#endif

/*****************************************************************/
/*****************************************************************/

UT_Confidence_t IE_Imp_XHTML_Sniffer::recognizeContents(const char * szBuf, 
											 UT_uint32 iNumbytes)
{
	UT_uint32 iLinesToRead = 6 ;  // Only examine the first few lines of the file
	UT_uint32 iBytesScanned = 0 ;
	const char *p ;
	char *magic ;
	p = szBuf ;
	while( iLinesToRead-- )
	{
		magic = "<html " ;
		if ( (iNumbytes - iBytesScanned) < strlen(magic) ) return(UT_CONFIDENCE_ZILCH);
		if ( strncmp(p, magic, strlen(magic)) == 0 ) return(UT_CONFIDENCE_PERFECT);
		magic = "<!DOCTYPE html" ;
		if ( (iNumbytes - iBytesScanned) < strlen(magic) ) return(UT_CONFIDENCE_ZILCH);
		if ( strncmp(p, magic, strlen(magic)) == 0 ) return(UT_CONFIDENCE_PERFECT);
		/*  Seek to the next newline:  */
		while ( *p != '\n' && *p != '\r' )
		{
			iBytesScanned++ ; p++ ;
			if( iBytesScanned+2 >= iNumbytes ) return(UT_CONFIDENCE_ZILCH);
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
	return(UT_CONFIDENCE_ZILCH);
}

UT_Confidence_t IE_Imp_XHTML_Sniffer::recognizeSuffix(const char * szSuffix)
{
  if (!(UT_stricmp(szSuffix,".html")) || !(UT_stricmp(szSuffix,".xhtml"))
      || !(UT_stricmp(szSuffix,".htm")))
    return UT_CONFIDENCE_PERFECT;
  return UT_CONFIDENCE_ZILCH;    
}

UT_Error IE_Imp_XHTML_Sniffer::constructImporter(PD_Document * pDocument,
												 IE_Imp ** ppie)
{
	IE_Imp_XHTML * p = new IE_Imp_XHTML(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_XHTML_Sniffer::getDlgLabels(const char ** pszDesc,
										   const char ** pszSuffixList,
										   IEFileType * ft)
{
	*pszDesc = "XHTML (.html, .htm, .xhtml)";
	*pszSuffixList = "*.html; *.htm; *.xhtml";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

  // hackishly, we append two lists to every document
  // to represent the <ol> and <ul> list types
  // <ol> has an id of 1 and <ul> is given the list id of 2

  static const XML_Char *ol_atts[] =
  {
    "id", "1",
    "parentid", "0",
    "type", "0",
    "start-value", "1",
    "list-delim", "%L.",
    "list-decimal", ".",
    NULL, NULL
  };

  static const XML_Char *ol_p_atts[] = 
  {
    "level", "1",
    "listid", "1",
    "parentid", "0",
    "props", "list-style:Numbered List; color:000000; font-family:Times New Roman; font-size:12pt; font-style:normal; font-weight:normal; start-value:1; text-decoration:none; text-indent:-0.3in; text-position:normal; field-font: NULL;",
	/* margin-left is purposefully left out of the props.  It is computed
	   based on the depth of the list, and appended to this list of
	   attributes. */
    NULL, NULL
  };

  static const XML_Char *ul_atts[] =
  {
    "id", "2",
    "parentid", "0",
    "type", "5",
    "start-value", "0",
    "list-delim", "%L",
    "list-decimal", "NULL",
    NULL, NULL
  };

  static const XML_Char *ul_p_atts[] =
  {
    "level", "1",
    "listid", "2",
    "parentid", "0",
    "props", "list-style:Bullet List;color:000000; font-family:Times New Roman; font-size:12pt; font-style:normal; font-weight:normal; start-value:0; text-decoration:none; text-indent:-0.3in; text-position:normal; field-font: Symbol;",
	/* margin-left is purposefully left out of the props.  It is computed
	   based on the depth of the list, and appended to this list of
	   attributes. */
    NULL, NULL
  };

IE_Imp_XHTML::IE_Imp_XHTML(PD_Document * pDocument)
  : IE_Imp_XML(pDocument, false), m_listType(L_NONE),
    m_iListID(0), m_bFirstDiv(true), m_iNewListID(0), m_szBookMarkName(NULL)
{
}

IE_Imp_XHTML::~IE_Imp_XHTML()
{
}

// to get lists to work:
// 1. append ol and ul type lists (done in constructor)
// 2. a ol or ul tag begins a paragraph with level=1, parentid=0, listid=[1|2]
//    style=[Numbered List|Bullet List]
// 3. append a field of type "list_label"
// 4. insert character run with type "list_label"
// 5. insert a tab

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

#define TT_OL                   33              // ordered list
#define TT_UL                   34              // unordered list
#define TT_LI                   35              // list item
#define TT_HEAD                 36              // head tag
#define TT_META                 37              // meta info tag
#define TT_TITLE                38              // title tag
#define TT_STYLE                39              // style tag

#define TT_PRE					40				// preformatted tag

#define TT_HREF                 41              // <a> anchor tag


// This certainly leaves off lots of tags, but with HTML, this is inevitable - samth

static struct xmlToIdMapping s_Tokens[] =
{
	{       "a",              	    TT_HREF                 },
	{       "address",              TT_ADDRESS              },
	{       "b",                    TT_B                    },
	{       "blockquote",           TT_BLOCKQUOTE           },
	{       "body",                 TT_BODY                 },
	{	"br",			TT_BREAK		},
	{       "cite",                 TT_CITE                 },
	{       "code",                 TT_CODE                 },
	{       "def",                  TT_DFN                  },
	{	"div",		        TT_DIV		        },
	{       "em",                   TT_EM                   },
	{       "font",                 TT_FONT                 },
	{       "h1",                   TT_H1                   },
	{       "h2",                   TT_H2                   },
	{       "h3",                   TT_H3                   },
	{       "h4",                   TT_H4                   },
	{       "h5",                   TT_H5                   },
	{       "h6",                   TT_H6                   },
	{       "head",                 TT_HEAD                 },
	{	"html",	        	TT_DOCUMENT		},
	{       "i",                    TT_I                    },
	{       "kbd",                  TT_KBD                  },
	{       "li",                   TT_LI                   },
	{       "meta",                 TT_META                 },
	{       "ol",                   TT_OL                   },
	{	"p",			TT_P	        	},
	{		"pre",					TT_PRE					},
	{       "q",                    TT_Q                    },
	{       "s",                    TT_S                    },
	{       "samp",                 TT_SAMP                 },
	{	"span",			TT_INLINE		},
	{       "strike",               TT_STRIKE               },
	{       "strong",               TT_STRONG               },
	{       "style",                TT_STYLE                },
	{       "sub",                  TT_SUB                  },
	{       "sup",                  TT_SUP                  },
	{       "title",                TT_TITLE                },
	{       "tr",                   TT_TR                   },	
	{       "u",                    TT_UNDERLINE            },
	{       "ul",                   TT_UL                   },
	{       "var",                  TT_VAR                  }
};

#define TokenTableSize	((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

/*****************************************************************/	
/*****************************************************************/	

#define X_TestParseState(ps)	((m_parseState==(ps)))

#define X_VerifyParseState(ps)	do {  if (!(X_TestParseState(ps)))      \
				{  m_error = UT_IE_BOGUSDOCUMENT;	\
                                   UT_DEBUGMSG(("DOM: unhandled tag: %d (ps: %d)\n", tokenIndex, ps)); \
				   failLine = __LINE__; goto X_Fail; } } while (0)

#define X_CheckDocument(b)		do {  if (!(b))	\
					{  m_error = UT_IE_BOGUSDOCUMENT;	\
					failLine = __LINE__; goto X_Fail; } } while (0)

#define X_CheckError(v)			do {  if (!(v))	\
					  {  m_error = UT_ERROR; \
					     UT_DEBUGMSG(("JOHN: X_CheckError\n")); \
					failLine = __LINE__; goto X_Fail; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_error) { failLine = __LINE__; goto X_Fail; } } while (0)

/*****************************************************************/
/*****************************************************************/

static void convertFontFace(UT_String & szDest, const char *szFrom)
{
	// TODO: make me better
	// TODO: and handle things like comma lists of font faces
	char *newFont;
	
	// default...
	if(szFrom == NULL)
		newFont = "Times New Roman";
	else
		newFont = (char*)szFrom;
	
	szDest = newFont;
}

static void convertFontSize(UT_String & szDest, const char *szFrom)
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
	
	UT_String_sprintf(szDest, "%2d", sz);
}

static void convertColor(UT_String & szDest, const char *szFrom, int dfl = 0x000000)
{

  // if it starts with a #, send it through
  // if it starts with a number, send it through
  // else convert color from values in XHTML DTD

	int col = dfl;
	
	if(szFrom == NULL)
    {
		col = dfl;
    }
	if(*szFrom == '#')
    {
		col = atoi(szFrom+1);
    }
	else if(UT_UCS_isdigit(*szFrom))
    {
		col = atoi(szFrom);
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "green"))
    {
		col = 0x008000;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "silver"))
    {
		col = 0xC0C0C0;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "lime"))
    {
		col = 0x00FF00;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "gray"))
    {
		col = 0x808080;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "olive"))
    {
		col = 0x808000;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "white"))
    {
		col = 0xFFFFFF;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "yellow"))
    {
		col = 0xFFFF00;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "maroon"))
    {
		col = 0x800000;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "navy"))
    {
		col = 0x000080;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "red"))
    {
		col = 0xFF0000;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "blue"))
    {
		col = 0x0000FF;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "purple"))
    {
		col = 0x800080;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "teal"))
    {
		col = 0x008080;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "fuchsia"))
    {
		col = 0xFF00FF;
    }
	else if(!UT_XML_stricmp((XML_Char *)szFrom, "aqua"))
    {
		col = 0x00FFFF;
    }
	
	UT_String_sprintf(szDest, "%6x", col);
	UT_DEBUGMSG(("DOM: color: %0xd (%s => %s)\n", col, szFrom, szDest.c_str()));
}

/*****************************************************************/
/*****************************************************************/

void IE_Imp_XHTML::startElement(const XML_Char *name, const XML_Char **atts)
{
	int failLine;
	failLine = 0;
	UT_DEBUGMSG(("startElement: %s, parseState: %u, listType: %u\n", name, m_parseState, m_listType));

	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed
	                                                // this just avoids all the processing if there is an error
#define NEW_ATTR_SZ 3
 	const XML_Char *new_atts[NEW_ATTR_SZ];
	XML_Char * sz;
	sz = NULL;

	for(int i = 0; i < NEW_ATTR_SZ; i++)
	  new_atts[i] = NULL;
#undef NEW_ATTR_SZ
	UT_uint16 *parentID;

	UT_uint32 tokenIndex;
	tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);

	switch (tokenIndex)
	{
	case TT_DOCUMENT:
	  //UT_DEBUGMSG(("Init %d\n", m_parseState));
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;
		return;

	case TT_BODY:
	  //UT_DEBUGMSG(("Doc %d\n", m_parseState));
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Sec;
		X_CheckError(getDoc()->appendStrux(PTX_Section,NULL));
		return;		

	case TT_DIV:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Sec);
		if( !m_bFirstDiv )
		{
			X_CheckError(getDoc()->appendStrux(PTX_Section,NULL));
		}
		else
		{
			m_bFirstDiv = false;
		}
		return;

	case TT_Q:
	case TT_SAMP:
	case TT_VAR:
	case TT_KBD:
	case TT_ADDRESS:
	case TT_CITE:
	case TT_EM:
	case TT_I:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
		new_atts[0]=sz;
		sz = NULL;
		UT_XML_cloneString(sz, "font-style:italic");
		new_atts[1]=sz;
		X_CheckError(getDoc()->appendFmt(new_atts));
		return;

	case TT_CODE:
	case TT_DFN:
	case TT_STRONG:
	case TT_B:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
		new_atts[0]=sz;
		sz = NULL;
		UT_XML_cloneString(sz, "font-weight:bold");
		new_atts[1]=sz;
		X_CheckError(getDoc()->appendFmt(new_atts));
		return;

	case TT_UNDERLINE:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
		new_atts[0]=sz;
		sz = NULL;
		UT_XML_cloneString(sz, "text-decoration:underline");
		new_atts[1]=sz;
		X_CheckError(getDoc()->appendFmt(new_atts));
		return;

	case TT_SUP:
	case TT_SUB:
		//UT_DEBUGMSG(("Super or Subscript\n"));
		X_VerifyParseState(_PS_Block);
		UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
		new_atts[0]=sz;
		sz = NULL;
		if(tokenIndex == TT_SUP)    
			UT_XML_cloneString(sz, "text-position:superscript");
		else
			UT_XML_cloneString(sz, "text-position:subscript");
		new_atts[1]=sz;
		X_CheckError(getDoc()->appendFmt(new_atts));
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
		X_CheckError(getDoc()->appendFmt(new_atts));
		return;
  
	case TT_FONT:
		UT_DEBUGMSG(("Font tag encountered\n"));
		{
			const XML_Char *p_val;
			UT_String color, bgcolor, size, face;
			UT_String output;
			
			p_val = _getXMLPropValue((const XML_Char *)"color", atts);
			convertColor(color, p_val, 0x000000);

			p_val = _getXMLPropValue((const XML_Char *)"background", atts);
			if ( p_val )
			  convertColor(bgcolor, p_val, 0xFFFFFF);
			else
			  bgcolor = "none";

			p_val = _getXMLPropValue((const XML_Char *)"size", atts);
			convertFontSize(size, p_val);
			
			p_val = _getXMLPropValue((const XML_Char *)"face", atts);
			convertFontFace(face, p_val);
			
			UT_String_sprintf(output, "color:%s; bgcolor: %s; font-family:%s; size:%spt", color.c_str(), bgcolor.c_str(), face.c_str(), size.c_str());
			UT_DEBUGMSG(("Font properties: %s\n", output.c_str()));
			
			UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
			new_atts[0] = sz;
			sz = NULL;
			UT_XML_cloneString(sz, output.c_str());
			new_atts[1] = sz;
			X_CheckError(getDoc()->appendFmt(new_atts));
		}
		return;

	case TT_BLOCKQUOTE:
	case TT_H1:
	case TT_H2:
	case TT_H3:
	{
	    //    UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		
		const XML_Char *p_val;
		
		p_val = _getXMLPropValue((const XML_Char *)"align", atts);
		if(p_val == NULL)
			X_CheckError(getDoc()->appendStrux(PTX_Block,NULL));
		else
		{
		    sz = NULL;
		    
		    if(!UT_XML_strcmp(p_val, "right"))
				UT_XML_cloneString(sz, "text-align:right");
		    else if(!UT_XML_strcmp(p_val, "center"))
				UT_XML_cloneString(sz, "text-align:center");
		    
		    if(sz != NULL)
			{
				new_atts[1] = sz;
				UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
				new_atts[0] = sz;		      
			}
			
		    X_CheckError(getDoc()->appendStrux(PTX_Block,new_atts));
		}
		
		UT_XML_cloneString(sz, PT_STYLE_ATTRIBUTE_NAME);
		new_atts[0]=sz;
		sz = NULL;
		
		if(tokenIndex == TT_H1)
			UT_XML_cloneString(sz, "Heading 1");
		else if(tokenIndex == TT_H2)
			UT_XML_cloneString(sz, "Heading 2");
		else if(tokenIndex == TT_H3)
			UT_XML_cloneString(sz, "Heading 3");
		else
			UT_XML_cloneString(sz, "Block Text");
		
		new_atts[1]=sz;
		X_CheckError(getDoc()->appendFmt(new_atts));
		return;
	}

	case TT_OL:	  
	case TT_UL:
	{
		if(tokenIndex == TT_OL)
	  		m_listType = L_OL;
	  	else 
			m_listType = L_UL;

		if(m_parseState != _PS_Sec)
		{
			endElement("li");
			m_parseState = _PS_Sec;
			m_bWasSpace = false;
			/* this sort of tag shuffling can mess up the space tracking */
		}

		parentID = new UT_uint16(m_iListID);
		m_utsParents.push(parentID);

		/* new list, increment list depth */
		m_iNewListID++;
		m_iListID = m_iNewListID;

		const XML_Char** listAtts;
		listAtts = (tokenIndex == TT_OL ? ol_atts : ul_atts);

		UT_String szListID, szParentID;
		UT_String_sprintf(szListID, "%u", m_iNewListID);
		UT_String_sprintf(szParentID, "%u", *parentID);

		const int IDpos = 1;
		const int parentIDpos = 3;

		listAtts[IDpos] = szListID.c_str();
		listAtts[parentIDpos] = szParentID.c_str();

		X_CheckError(getDoc()->appendList (listAtts));

		return;
	}
	case TT_LI:
	{
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;

		XML_Char *sz;

		if (m_listType != L_NONE)
		{
			UT_uint16 thisID = m_iListID;
			m_utsParents.viewTop((void**) &parentID);

			const XML_Char** listAtts;
			listAtts = (m_listType == L_OL ? ol_p_atts : ul_p_atts);

			/* assign the appropriate list ID, parent ID, and level
			   to this list item's attributes */

			UT_String szListID, szParentID, szLevel, szMarginLeft;
			UT_String_sprintf(szListID, "%u", thisID);
			UT_String_sprintf(szParentID, "%u", *parentID);
			UT_String_sprintf(szLevel, "%u", m_utsParents.getDepth());
			UT_String_sprintf(szMarginLeft, " margin-left: %.2fin", 
					  m_utsParents.getDepth() * 0.5);

			const int LevelPos = 1;
			const int IDpos = 3;
			const int parentIDpos = 5;
			const int propsPos = 7;

			UT_String props = listAtts[propsPos];
			props += szMarginLeft;

			listAtts[LevelPos] = szLevel.c_str();
			listAtts[IDpos] = szListID.c_str();
			listAtts[parentIDpos] = szParentID.c_str();

			XML_Char* temp = (XML_Char*) listAtts[propsPos];
			listAtts[propsPos] = props.c_str();

			X_CheckError(getDoc()->appendStrux(PTX_Block, listAtts));

			listAtts[propsPos] = temp;

			// append a field
			UT_XML_cloneString(sz, "type");
			new_atts[0] = sz;
			UT_XML_cloneString(sz, "list_label");
			new_atts[1] = sz;
			X_CheckError(getDoc()->appendObject(PTO_Field, new_atts));

			// append the character run
			UT_XML_cloneString(sz, "type");
			new_atts[0] = sz;
			UT_XML_cloneString(sz, "list_label");
			new_atts[1] = sz;
			X_CheckError(getDoc()->appendFmt(new_atts));
		}
		return;
	}

	case TT_P:
	case TT_PRE:
	case TT_TR:
	case TT_H4:
	case TT_H5:
	case TT_H6:
		//UT_DEBUGMSG(("B %d\n", m_parseState));
	{
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		
		if(tokenIndex == TT_PRE)
		{
			m_bWhiteSignificant = true;
		}
		
		const XML_Char *p_val;

		p_val = _getXMLPropValue((const XML_Char *)"align", atts);
		if(p_val == NULL)
		{
			X_CheckError(getDoc()->appendStrux(PTX_Block,NULL));
		}
		else
		{
		    sz = NULL;
			
		    if(!UT_XML_strcmp(p_val, "right"))
				UT_XML_cloneString(sz, "text-align:right");
		    else if(!UT_XML_strcmp(p_val, "center"))
				UT_XML_cloneString(sz, "text-align:center");
		    
		    if(sz != NULL)
			{
				new_atts[1] = sz;
				UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
				new_atts[0] = sz;
			}
			
		    X_CheckError(getDoc()->appendStrux(PTX_Block,new_atts));
		}
		return;
	}
	
	case TT_INLINE:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		const XML_Char *p_val;
		
		p_val = _getXMLPropValue((const XML_Char *)"style", atts);
		if(p_val)
		{
		    UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
		    new_atts[0] = sz;
		    sz = NULL;
			
		    UT_XML_cloneString(sz, p_val);
		    new_atts[1] = sz;
		}
		
		_pushInlineFmt(new_atts);
		getDoc()->appendFmt(&m_vecInlineFmt);
		return;

	case TT_BREAK:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		if(m_parseState == _PS_Block)
		{
			UT_UCSChar ucs = UCS_LF;
			X_CheckError(getDoc()->appendSpan(&ucs,1));
		}
		return;

	case TT_HREF:
	{
		X_VerifyParseState(_PS_Block);
		const XML_Char *p_val;
		p_val = _getXMLPropValue((const XML_Char *)"href", atts);
		if( p_val )
		{
		    UT_XML_cloneString(sz, "xlink:href");
		    new_atts[0] = sz;
	    	sz = NULL;
		    UT_XML_cloneString(sz, p_val);
		    new_atts[1] = sz;
			X_CheckError(getDoc()->appendObject(PTO_Hyperlink,new_atts));
		}
		else
		{
			p_val = _getXMLPropValue((const XML_Char *)"name", atts);
			if( p_val )
			{
				UT_sint32 i;
 				const XML_Char *bm_new_atts[5];
				for( i = 0; i < 5; i++) bm_new_atts[i] = NULL;
	    		sz = NULL;
			    UT_XML_cloneString(sz, "type");
				bm_new_atts[0] = sz; 
	    		sz = NULL;
			    UT_XML_cloneString(sz, "start");
				bm_new_atts[1] = sz;
	    		sz = NULL;
			    UT_XML_cloneString(sz, "name");
			    bm_new_atts[2] = sz;
	    		sz = NULL;
		    	UT_XML_cloneString(sz, p_val);
				UT_XML_cloneString(m_szBookMarkName, p_val);
			    bm_new_atts[3] = sz;
				X_CheckError(getDoc()->appendObject(PTO_Bookmark,bm_new_atts));
				for( i = 0; i < 5; i++) FREEP(bm_new_atts[i]);
			}
		}
		return;
	}

	case TT_HEAD:
	case TT_TITLE:
	case TT_META:
	case TT_STYLE:
		// these tags are ignored for the time being
		return;
		
	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown tag [%s]\n",name));

		//It's imperative that we keep processing after finding an unknown element

		return;
	}

	return;
X_Fail:
	UT_DEBUGMSG (("X_Fail at %d\n", failLine));
	return;
}

void IE_Imp_XHTML::endElement(const XML_Char *name)
{
	int failLine;
	failLine = 0;
	UT_DEBUGMSG(("endElement: %s, parseState: %u, listType: %u\n", name, m_parseState, m_listType));
	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed
	
	
	UT_uint32 tokenIndex;
	tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);
	//if(!UT_strcmp(name == "html")) UT_DEBUGMSG(("tokenindex : %d\n", tokenIndex));
	switch (tokenIndex)
	{
	case TT_DOCUMENT:
	  //UT_DEBUGMSG(("Init %d\n", m_parseState));
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Init;
		return;

	case TT_BODY:
	  //UT_DEBUGMSG(("Doc %d\n", m_parseState));
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Doc;
		return;

	case TT_DIV:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Sec);
		return;

	case TT_OL:
	case TT_UL:
		UT_uint16 *temp;

		if(m_utsParents.pop((void**) &temp))
		{
			m_iListID = *temp;
			DELETEP(temp);
		}

		if(m_utsParents.getDepth() == 0)
			m_listType = L_NONE; 
			
		return;

	case TT_LI:
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
	  	if(m_parseState != _PS_Block) return;
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth()==0);
		_popInlineFmt();
		X_CheckError(getDoc()->appendFmt(&m_vecInlineFmt));
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
		X_CheckError(getDoc()->appendFmt(&m_vecInlineFmt));
		return;
		
	case TT_INLINE:
		UT_ASSERT(m_lenCharDataSeen==0);
		//UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()>0);
		_popInlineFmt();
		getDoc()->appendFmt(&m_vecInlineFmt);
		return;

	case TT_BREAK:						// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		//UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		return;

	case TT_HEAD:
	case TT_TITLE:
	case TT_META:
	case TT_STYLE:
		return;

	case TT_HREF:
		if( m_szBookMarkName )
		{
			UT_sint32 i;
			XML_Char * sz = NULL;
			const XML_Char *bm_new_atts[5];
			for(i = 0; i < 5; i++) bm_new_atts[i] = NULL;
		    UT_XML_cloneString(sz, "type");
			bm_new_atts[0] = sz; 
    		sz = NULL;
		    UT_XML_cloneString(sz, "end");
			bm_new_atts[1] = sz;
    		sz = NULL;
		    UT_XML_cloneString(sz, "name");
		    bm_new_atts[2] = sz;
    		sz = NULL;
	    	UT_XML_cloneString(sz, m_szBookMarkName);
		    bm_new_atts[3] = sz;
			X_CheckError(getDoc()->appendObject(PTO_Bookmark,bm_new_atts));
			for(i = 0; i < 5; i++) FREEP(bm_new_atts[i]);
			FREEP(m_szBookMarkName);
			m_szBookMarkName = NULL;
		}
		return;

	case TT_PRE:
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth()==0);
		m_bWhiteSignificant = false;
		return;

	case TT_OTHER:
	default:
	  	UT_DEBUGMSG(("Unknown end tag [%s]\n",name));
		return;
	}
	return;
X_Fail:
	UT_DEBUGMSG (("X_Fail at %d\n", failLine));
	return;
}


void IE_Imp_XHTML::charData (const XML_Char * buffer, int length)
{
	int failLine;
	failLine = 0;
	bool bResetState = false;
	if( m_parseState == _PS_Sec )
    { 
		// Sets a block Strux and falls through.  
		// Hack to work around the need for <p> etc to enter data 
		// from HTML. 
		X_CheckError(getDoc()->appendStrux(PTX_Block,NULL)); 
		m_parseState = _PS_Block;
		bResetState = true;
	} 

	IE_Imp_XML::charData ( buffer, length );

	if( bResetState )
	{
		m_parseState = _PS_Sec;
	}
	return;
X_Fail:
	UT_DEBUGMSG (("X_Fail at %d\n", failLine));
	return;
}
