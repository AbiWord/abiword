/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2002 AbiSource, Inc.
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
#include <ctype.h>
#include <locale.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_exception.h"
#include "ut_bytebuf.h"
#include "ut_path.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_string_class.h"

#include "fg_GraphicRaster.h"

#include "pd_Document.h"

#include "ie_types.h"
#include "ie_impGraphic.h"
#include "ie_impexp_HTML.h"
#include "ie_imp_XHTML.h"

#define CSS_MASK_INLINE (1<<0)
#define CSS_MASK_BLOCK  (1<<1)
#define CSS_MASK_IMAGE  (1<<2)
#define CSS_MASK_BODY   (1<<3)

static UT_UTF8String s_parseCSStyle (const UT_UTF8String & style, UT_uint32 css_mask);

/*****************************************************************/
/*****************************************************************/

#if defined(ENABLE_PLUGINS) || defined(ABI_PLUGIN_SOURCE)

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

IE_Imp_XHTML_Sniffer::IE_Imp_XHTML_Sniffer ()
#ifdef XHTML_NAMED_CONSTRUCTORS
	: IE_ImpSniffer("AbiXHTML::XHTML")
#endif
{
  // 
}

#ifdef XHTML_NAMED_CONSTRUCTORS

UT_Confidence_t IE_Imp_XHTML_Sniffer::supportsMIME (const char * szMIME)
{
  if (UT_strcmp (IE_FileInfo::mapAlias (szMIME), IE_MIME_XHTML) == 0)
    {
      return UT_CONFIDENCE_GOOD;
    }
  return UT_CONFIDENCE_ZILCH;
}

#endif /* XHTML_NAMED_CONSTRUCTORS */

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

IE_Imp_XHTML::IE_Imp_XHTML(PD_Document * pDocument) :
	IE_Imp_XML(pDocument,false),
	m_listType(L_NONE),
    m_iListID(0),
	m_bFirstDiv(true),
	m_iNewListID(0),
	m_iNewImage(0),
	m_szBookMarkName(NULL),
	m_addedPTXSection(false)
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

// This certainly leaves off lots of tags, but with HTML, this is inevitable - samth

static struct xmlToIdMapping s_Tokens[] =
{
	{ "a",			TT_A			},
	{ "address",	TT_ADDRESS		},
	{ "b",			TT_B			},
	{ "blockquote",	TT_BLOCKQUOTE	},
	{ "body",		TT_BODY			},
	{ "br",			TT_BR			},
	{ "cite",		TT_CITE			},
	{ "code",		TT_CODE			},
	{ "dd",			TT_DD			},
	{ "dfn",		TT_DFN			},
	{ "div",		TT_DIV			},
	{ "dl",			TT_DL			},
	{ "dt",			TT_DT			},
	{ "em",			TT_EM			},
	{ "font",		TT_FONT			},
	{ "h1",			TT_H1			},
	{ "h2",			TT_H2			},
	{ "h3",			TT_H3			},
	{ "h4",			TT_H4			},
	{ "h5",			TT_H5			},
	{ "h6",			TT_H6			},
	{ "head",		TT_HEAD			},
	{ "html",		TT_HTML			},
	{ "i",			TT_I			},
	{ "img",		TT_IMG			},
	{ "kbd",		TT_KBD			},
	{ "li",			TT_LI			},
	{ "meta",		TT_META			},
	{ "ol",			TT_OL			},
	{ "p",			TT_P			},
	{ "pre",		TT_PRE			},
	{ "q",			TT_Q			},
#ifdef XHTML_RUBY_SUPPORTED
	{ "rp",			TT_RP			},
	{ "rt",			TT_RT			},
	{ "ruby",		TT_RUBY			},
#endif
	{ "s",			TT_S			},
	{ "samp",		TT_SAMP			},
	{ "span",		TT_SPAN			},
	{ "strike",		TT_STRIKE		},
	{ "strong",		TT_STRONG		},
	{ "style",		TT_STYLE		},
	{ "sub",		TT_SUB			},
	{ "sup",		TT_SUP			},
	{ "title",		TT_TITLE		},
	{ "tr",			TT_TR			},	
	{ "tt",			TT_TT			},
	{ "u",			TT_U			},
	{ "ul",			TT_UL			},
	{ "var",		TT_VAR			}
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
#ifdef XHTML_UCS4
	else if(UT_UCS4_isdigit(*szFrom))
#else
	else if(UT_UCS_isdigit(*szFrom))
#endif
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
#ifdef XHTML_UCS4
	else if(UT_UCS4_isdigit(*szFrom))
#else
	else if(UT_UCS_isdigit(*szFrom))
#endif
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

UT_Error IE_Imp_XHTML::importFile(const char * szFilename)
{
	if ( szFilename == 0) return UT_IE_BOGUSDOCUMENT;
	if (*szFilename == 0) return UT_IE_BOGUSDOCUMENT;

	int path_length = strlen (szFilename);
	int name_length = strlen (UT_basename (szFilename));

	if (path_length > name_length)
		m_dirname = UT_String(szFilename,path_length-name_length);
	else
		m_dirname = "";

	UT_Error e = IE_Imp_XML::importFile(szFilename);
 	if (!m_addedPTXSection) e = UT_IE_BOGUSDOCUMENT;
	return e;
}

/*****************************************************************/
/*****************************************************************/

void IE_Imp_XHTML::startElement(const XML_Char *name, const XML_Char **atts)
{
	int i = 0;
	int failLine;
	failLine = 0;
	UT_DEBUGMSG(("startElement: %s, parseState: %u, listType: %u\n", name, m_parseState, m_listType));

	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed
	                                                // this just avoids all the processing if there is an error
#define NEW_ATTR_SZ 3
 	const XML_Char *new_atts[NEW_ATTR_SZ];
	XML_Char * sz;
	sz = NULL;

	for(i = 0; i < NEW_ATTR_SZ; i++)
	  new_atts[i] = NULL;
#undef NEW_ATTR_SZ
	UT_uint16 *parentID;

	UT_uint32 tokenIndex;
	tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);

	switch (tokenIndex)
	{
	case TT_HTML:
	  //UT_DEBUGMSG(("Init %d\n", m_parseState));
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;
		return;

	case TT_BODY:
	  //UT_DEBUGMSG(("Doc %d\n", m_parseState));
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Sec;
		X_CheckError(getDoc()->appendStrux(PTX_Section,NULL));
		m_addedPTXSection = true;
		return;		

	case TT_DIV:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Sec);
		if( !m_bFirstDiv )
		{
			X_CheckError(getDoc()->appendStrux(PTX_Section,NULL));
			m_addedPTXSection = true;
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

	case TT_CODE:
	case TT_TT:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		UT_XML_cloneString(sz, PT_PROPS_ATTRIBUTE_NAME);
		new_atts[0]=sz;
		sz = NULL;
		UT_XML_cloneString(sz, "font-family:Courier");
		new_atts[1]=sz;
		X_CheckError(getDoc()->appendFmt(new_atts));
		return;

	case TT_U:
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
		
	case TT_S://	case TT_STRIKE:
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
	case TT_PRE:
	{
	    //    UT_DEBUGMSG(("B %d\n", m_parseState));
//		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		
		if (tokenIndex == TT_PRE) m_bWhiteSignificant = true;
		
		const XML_Char * api_atts[5];

		sz = NULL;
		UT_XML_cloneString (sz, PT_STYLE_ATTRIBUTE_NAME);
		X_CheckError(sz);
		api_atts[0] = sz;
		api_atts[1] = NULL;
		sz = NULL;
		UT_XML_cloneString (sz, PT_PROPS_ATTRIBUTE_NAME);
		X_CheckError(sz);
		api_atts[2] = sz;
		api_atts[3] = NULL;
		api_atts[4] = NULL;

		const XML_Char * p_val = _getXMLPropValue ((const XML_Char *) "awml:style", atts);
		sz = NULL;
		if (p_val)
			UT_XML_cloneString (sz, p_val);
		else if (tokenIndex == TT_H1)
			UT_XML_cloneString (sz, "Heading 1");
		else if (tokenIndex == TT_H2)
			UT_XML_cloneString (sz, "Heading 2");
		else if (tokenIndex == TT_H3)
			UT_XML_cloneString (sz, "Heading 3");
		else if (tokenIndex == TT_PRE)
			UT_XML_cloneString (sz, "Plain Text");
		else
			UT_XML_cloneString (sz, "Block Text");
		X_CheckError(sz);
		api_atts[1] = sz;
		
		UT_UTF8String utf8val;

		p_val = _getXMLPropValue ((const XML_Char *) "style", atts);
		if (p_val)
		{
			utf8val = (const char *) p_val;
			utf8val = s_parseCSStyle (utf8val, CSS_MASK_BLOCK);
			UT_DEBUGMSG(("CSS->Props (utf8val): [%s]\n",utf8val.utf8_str()));
		}
		if (strstr (utf8val.utf8_str (), "text-align") == 0)
		{
			p_val = _getXMLPropValue ((const XML_Char *) "align", atts);
			if (p_val)
			{
				if (!UT_XML_strcmp (p_val, "right"))
				{
					if (utf8val.byteLength ()) utf8val += "; ";
					utf8val += "text-align:right";
				}
				else if (!UT_XML_strcmp (p_val, "center"))
				{
					if (utf8val.byteLength ()) utf8val += "; ";
					utf8val += "text-align:center";
				}
			}
		}
		sz = NULL;
		UT_XML_cloneString (sz, utf8val.utf8_str ());
		X_CheckError(sz);
		api_atts[3] = sz;

		X_CheckError(getDoc()->appendStrux (PTX_Block, api_atts));
		return;
	}

	case TT_OL:	  
	case TT_UL:
	case TT_DL:
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
	case TT_DT:
	case TT_DD:
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
			char * old_locale = setlocale (LC_NUMERIC, "C");
			UT_String_sprintf(szMarginLeft, " margin-left: %.2fin", 
					  m_utsParents.getDepth() * 0.5);
			setlocale (LC_NUMERIC, old_locale);

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
	case TT_TR:
	case TT_H4:
	case TT_H5:
	case TT_H6:
		//UT_DEBUGMSG(("B %d\n", m_parseState));
	{
//		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;

		const XML_Char * api_atts[5];

		sz = NULL;
		UT_XML_cloneString (sz, PT_STYLE_ATTRIBUTE_NAME);
		X_CheckError(sz);
		api_atts[0] = sz;
		api_atts[1] = NULL;
		sz = NULL;
		UT_XML_cloneString (sz, PT_PROPS_ATTRIBUTE_NAME);
		X_CheckError(sz);
		api_atts[2] = sz;
		api_atts[3] = NULL;
		api_atts[4] = NULL;

		const XML_Char * p_val = _getXMLPropValue ((const XML_Char *) "awml:style", atts);
		sz = NULL;
		if (p_val)
			UT_XML_cloneString (sz, p_val);
		else
			UT_XML_cloneString (sz, "Normal");
		X_CheckError(sz);
		api_atts[1] = sz;
		
		UT_UTF8String utf8val;

		p_val = _getXMLPropValue ((const XML_Char *) "style", atts);
		if (p_val)
		{
			utf8val = (const char *) p_val;
			utf8val = s_parseCSStyle (utf8val, CSS_MASK_BLOCK);
			UT_DEBUGMSG(("CSS->Props (utf8val): [%s]\n",utf8val.utf8_str()));
		}
		if (strstr (utf8val.utf8_str (), "text-align") == 0)
		{
			p_val = _getXMLPropValue ((const XML_Char *) "align", atts);
			if (p_val)
			{
				if (!UT_XML_strcmp (p_val, "right"))
				{
					if (utf8val.byteLength ()) utf8val += "; ";
					utf8val += "text-align:right";
				}
				else if (!UT_XML_strcmp (p_val, "center"))
				{
					if (utf8val.byteLength ()) utf8val += "; ";
					utf8val += "text-align:center";
				}
			}
		}
		sz = NULL;
		UT_XML_cloneString (sz, utf8val.utf8_str ());
		X_CheckError(sz);
		api_atts[3] = sz;

		X_CheckError(getDoc()->appendStrux (PTX_Block, api_atts));
		return;
	}
	
	case TT_SPAN:
		{
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);

		new_atts[0] = NULL;
		new_atts[1] = NULL;

		const XML_Char * p_val = _getXMLPropValue ((const XML_Char *) "style", atts);
		if (p_val)
		{
			UT_UTF8String utf8val = (const char *) p_val;
			utf8val = s_parseCSStyle (utf8val, CSS_MASK_INLINE);
			UT_DEBUGMSG(("CSS->Props (utf8val): [%s]\n",utf8val.utf8_str()));

		    sz = NULL;
		    UT_XML_cloneString (sz, PT_PROPS_ATTRIBUTE_NAME);
			X_CheckError(sz);
		    new_atts[0] = sz;

		    sz = NULL;
		    UT_XML_cloneString (sz, utf8val.utf8_str ());
		    new_atts[1] = sz;
			X_CheckError(sz);
		}
		
		_pushInlineFmt (new_atts);
		X_CheckError(getDoc()->appendFmt (&m_vecInlineFmt));
		}
		return;

	case TT_BR:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		if(m_parseState == _PS_Block)
		{
			UT_UCSChar ucs = UCS_LF;
			X_CheckError(getDoc()->appendSpan(&ucs,1));
		}
		return;

	case TT_A:
	{
		const XML_Char * p_val = 0;
		p_val = _getXMLPropValue((const XML_Char *)"xlink:href", atts);
		if (p_val == 0) p_val = _getXMLPropValue((const XML_Char *)"href", atts);
		if( p_val )
		{
			X_VerifyParseState(_PS_Block);
		    UT_XML_cloneString(sz, "xlink:href");
		    new_atts[0] = sz;
	    	sz = NULL;
		    UT_XML_cloneString(sz, p_val);
		    new_atts[1] = sz;
			X_CheckError(getDoc()->appendObject(PTO_Hyperlink,new_atts));
		}
		else
		{
			p_val = _getXMLPropValue((const XML_Char *)"id", atts);
			if (p_val == 0) p_val = _getXMLPropValue((const XML_Char *)"name", atts);
			if( p_val )
			{
				if (m_parseState == _PS_Sec)
				{
					/* AbiWord likes things to sit inside blocks, but XHTML has
					 * no such requirement.
					 */
					X_CheckError(getDoc()->appendStrux(PTX_Block,NULL));
				}
				else X_VerifyParseState(_PS_Block);

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
				if (m_szBookMarkName)
				{
					X_CheckError(getDoc()->appendObject(PTO_Bookmark,bm_new_atts));
				}
				else for (i = 0; i < 4; i++) FREEP(bm_new_atts[i]);

				if (m_szBookMarkName && (m_parseState == _PS_Sec))
				{
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

					FREEP(m_szBookMarkName);
					m_szBookMarkName = NULL;
				}
			}
		}
		return;
	}

	case TT_IMG:
		{
		const XML_Char * szSrc    = _getXMLPropValue ((const XML_Char *) "src",    atts);
		const XML_Char * szStyle  = _getXMLPropValue ((const XML_Char *) "style",  atts);
		const XML_Char * szWidth  = _getXMLPropValue ((const XML_Char *) "width",  atts);
		const XML_Char * szHeight = _getXMLPropValue ((const XML_Char *) "height", atts);

		if ( szSrc == 0) break;
		if (*szSrc == 0) break;

		const char * szFile = (const char *) szSrc;

		if (strncmp (szFile, "http://", 7) == 0)
			{
				UT_DEBUGMSG(("found web image reference (%s) - skipping...\n",szFile));
				break;
			}
		if (strncmp (szFile, "data:", 5) == 0)
			{
				UT_DEBUGMSG(("found data-URL encoded image - skipping...\n"));
				// TODO??
				break;
			}
		if (strncmp (szFile, "file://", 7) == 0)
			szFile += 7;
		else if (strncmp (szFile, "file:", 5) == 0)
			szFile += 5;

		// TODO: this is a URL and may be encoded using %AC%BE%C1 etc.

		UT_String extended_path;

		if (*szFile != '/')
			{
				/* since this is a URL, directories should be delimited by '/'
				 * anyway, this looks like a relative link, so prefix the dirname
				 */
				extended_path = m_dirname;
			}
		extended_path += szFile;

		szFile = extended_path.c_str ();

		if (!UT_isRegularFile (szFile))
			{
				UT_DEBUGMSG(("found image reference (%s) - not found! skipping... \n",szFile));
				break;
			}
		UT_DEBUGMSG(("found image reference (%s) - loading... \n",szFile));

		IE_ImpGraphic * pieg = 0;
		if (IE_ImpGraphic::constructImporter (szFile, IEGFT_Unknown, &pieg) != UT_OK)
			{
				UT_DEBUGMSG(("unable to construct image importer!\n"));
				break;
			}
		X_CheckError(pieg);

		FG_Graphic * pfg = 0;
		UT_Error import_status = pieg->importGraphic (szFile, &pfg);
		delete pieg;
		if (import_status != UT_OK)
			{
				UT_DEBUGMSG(("unable to import image!\n"));
				break;
			}
		UT_DEBUGMSG(("image loaded successfully\n"));
		X_CheckError(pfg);

		UT_ByteBuf * pBB = static_cast<FG_GraphicRaster *>(pfg)->getRaster_PNG ();
		X_CheckError(pBB);

		char * mimetype = UT_strdup ("image/png");
		X_CheckError(mimetype);

		UT_UTF8String utf8val;
		if (szStyle)
			{
				utf8val = (const char *) szStyle;
				utf8val = s_parseCSStyle (utf8val, CSS_MASK_IMAGE);
				UT_DEBUGMSG(("CSS->Props (utf8val): [%s]\n",utf8val.utf8_str()));
			}
		if (szWidth && (strstr (utf8val.utf8_str (), "width") == 0))
			{
				UT_Dimension units = UT_determineDimension (szWidth, DIM_PX);
				double d = UT_convertDimensionless (szWidth);
				float width = (float) UT_convertDimensions (d, units, DIM_IN);
				UT_String tmp;
				char * old_locale = setlocale (LC_NUMERIC, "C");
				UT_String_sprintf (tmp, "%gin", width);
				setlocale (LC_NUMERIC, old_locale);
				if (!tmp.empty ())
					{
						if (utf8val.byteLength ()) utf8val += "; ";
						utf8val += "width:";
						utf8val += tmp.c_str ();
					}
			}
		if (szHeight && (strstr (utf8val.utf8_str (), "height") == 0))
			{
				UT_Dimension units = UT_determineDimension (szHeight, DIM_PX);
				double d = UT_convertDimensionless (szHeight);
				float height = (float) UT_convertDimensions (d, units, DIM_IN);
				UT_String tmp;
				char * old_locale = setlocale (LC_NUMERIC, "C");
				UT_String_sprintf (tmp, "%gin", height);
				setlocale (LC_NUMERIC, old_locale);
				if (!tmp.empty ())
					{
						if (utf8val.byteLength ()) utf8val += "; ";
						utf8val += "height:";
						utf8val += tmp.c_str ();
					}
			}
		if ((strstr (utf8val.utf8_str (), "width")  == 0) ||
			(strstr (utf8val.utf8_str (), "height") == 0))
			{
				float width  = static_cast<float>(pfg->getWidth ());
				float height = static_cast<float>(pfg->getHeight ());

				if ((width > 0) && (height > 0))
					{
						UT_DEBUGMSG(("missing width or height; reverting to image defaults\n"));
					}
				else
					{
						UT_DEBUGMSG(("missing width or height; setting these to 100x100\n"));
						width  = static_cast<float>(100);
						height = static_cast<float>(100);
					}
				const char * old_locale = setlocale (LC_NUMERIC, "C");
				UT_String tmp;
				UT_String_sprintf (tmp, "width:%gin; height:%gin", width, height);
				setlocale (LC_NUMERIC, old_locale);

				utf8val = tmp.c_str ();
			}

		const XML_Char * api_atts[5];

		UT_String dataid;
		UT_String_sprintf (dataid, "image%u", (unsigned int) m_iNewImage++);

		sz = NULL;
		UT_XML_cloneString (sz, PT_PROPS_ATTRIBUTE_NAME);
		X_CheckError(sz);
		api_atts[0] = sz;
		sz = NULL;
		UT_XML_cloneString (sz, utf8val.utf8_str ());
		X_CheckError(sz);
		api_atts[1] = sz;
		sz = NULL;
		UT_XML_cloneString (sz, "dataid");
		X_CheckError(sz);
		api_atts[2] = sz;
		sz = NULL;
		UT_XML_cloneString (sz, dataid.c_str ());
		X_CheckError(sz);
		api_atts[3] = sz;
		api_atts[4] = NULL;

		if (m_parseState == _PS_Sec)
			{
				X_CheckError(getDoc()->appendStrux (PTX_Block, NULL));
				m_parseState = _PS_Block;
			}
		UT_DEBUGMSG(("inserting `%s' as `%s' [%s]\n",szFile,dataid.c_str(),utf8val.utf8_str()));

		X_CheckError(getDoc()->appendObject (PTO_Image, api_atts));
		X_CheckError(getDoc()->createDataItem (dataid.c_str(), false, pBB, (void*) mimetype, NULL));

		UT_DEBUGMSG(("insertion successful\n"));
		}
		return;

	case TT_HEAD:
	case TT_TITLE:
	case TT_META:
	case TT_STYLE:
		// these tags are ignored for the time being
		return;
		
	case TT_RUBY:
	case TT_RP:
	case TT_RT:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		// For now we just want to render any text between these tags
		// Eventually we want to render the <rt> text above the
		// <ruby> text.  The <rp> text will not be rendered but should
		// be retained so it can be exported.
		X_VerifyParseState(_PS_Block);
		X_CheckError(getDoc()->appendFmt(new_atts));
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
	case TT_HTML:
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
	case TT_DL:
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
	case TT_DT:
	case TT_DD:
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
	case TT_S://	case TT_STRIKE:
	case TT_SUP:
	case TT_SUB:
	case TT_B:
	case TT_I:
	case TT_U:
	case TT_FONT:
	case TT_TT:
		UT_ASSERT(m_lenCharDataSeen==0);
		//UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()==0);
		//_popInlineFmt();
		X_CheckError(getDoc()->appendFmt(&m_vecInlineFmt));
		return;
		
	case TT_SPAN:
		UT_ASSERT(m_lenCharDataSeen==0);
		//UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()>0);
		_popInlineFmt();
		getDoc()->appendFmt(&m_vecInlineFmt);
		return;

	case TT_BR:						// not a container, so we don't pop stack
		UT_ASSERT(m_lenCharDataSeen==0);
		//UT_DEBUGMSG(("B %d\n", m_parseState));
//		X_VerifyParseState(_PS_Block);
		return;

	case TT_HEAD:
	case TT_TITLE:
	case TT_META:
	case TT_STYLE:
		return;

	case TT_A:
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
		else if (m_parseState == _PS_Block)
		{
			/* if (m_parseState == _PS_Sec) then this is an anchor outside
			 * of a block, not a hyperlink (see TT_A in startElement)
			 */
 			X_CheckError(getDoc()->appendObject(PTO_Hyperlink,0));
		}
		return;

	case TT_PRE:
		UT_ASSERT(m_lenCharDataSeen==0);
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth()==0);
		m_bWhiteSignificant = false;
		return;

	case TT_RUBY:
	case TT_RT:
	case TT_RP:
		// For now we just want to render any text between these tags
		// Eventually we want to render the <rt> text above the
		// <ruby> text.  The <rp> text will not be rendered but should
		// be retained so it can be exported.
		UT_ASSERT(m_lenCharDataSeen==0);
		//UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()==0);
		//_popInlineFmt();
		X_CheckError(getDoc()->appendFmt(&m_vecInlineFmt));
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
	/* No need to insert new blocks if we're just looking at the spaces
	 * between XML elements - unless we're in a <pre> sequence
	 */
	if (!m_bWhiteSignificant)
	{
#ifdef XHTML_UCS4
		UT_UCS4String buf(buffer,static_cast<size_t>(length),!m_bWhiteSignificant);
#else
		UT_UCS2String buf(buffer,static_cast<size_t>(length),!m_bWhiteSignificant);
#endif
		if (buf.size () == 0) return; // probably shouldn't happen; not sure
		if ((buf.size () == 1) && (buf[0] == UCS_SPACE)) return;
	}

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

static void s_pass_whitespace (const char *& csstr)
{
	while (*csstr)
	{
		unsigned char u = static_cast<unsigned char>(*csstr);
		if (u & 0x80)
		{
			UT_UTF8Stringbuf::UCS4Char ucs4 = UT_UTF8Stringbuf::charCode (csstr);
#ifdef XHTML_UCS4
			if (UT_UCS4_isspace (ucs4))
#else
			if ((ucs4 & 0x7fff) || UT_UCS_isspace (static_cast<UT_UCSChar>(ucs4 & 0xffff)))
#endif
			{
				while (static_cast<unsigned char>(*++csstr) & 0x80) { }
				continue;
			}
		}
		else if (isspace ((int) u))
		{
			csstr++;
			continue;
		}
		break;
	}
}

static const char * s_pass_name (const char *& csstr)
{
	const char * name_end = csstr;

	while (*csstr)
	{
		unsigned char u = static_cast<unsigned char>(*csstr);
		if (u & 0x80)
		{
			UT_UTF8Stringbuf::UCS4Char ucs4 = UT_UTF8Stringbuf::charCode (csstr);
#ifndef XHTML_UCS4
			if ((ucs4 & 0x7fff) == 0)
#endif
#ifdef XHTML_UCS4
				if (UT_UCS4_isspace (ucs4))
#else
				if (UT_UCS_isspace (static_cast<UT_UCSChar>(ucs4 & 0xffff)))
#endif
				{
					name_end = csstr;
					break;
				}
			while (static_cast<unsigned char>(*++csstr) & 0x80) { }
			continue;
		}
		else if ((isspace ((int) u)) || (*csstr == ':'))
		{
			name_end = csstr;
			break;
		}
		csstr++;
	}
	return name_end;
}

static const char * s_pass_value (const char *& csstr)
{
	const char * value_end = csstr;

	bool bQuoted = false;
	while (*csstr)
	{
		bool bSpace = false;
		unsigned char u = static_cast<unsigned char>(*csstr);
		if (u & 0x80)
		{
			UT_UTF8Stringbuf::UCS4Char ucs4 = UT_UTF8Stringbuf::charCode (csstr);
#ifdef XHTML_UCS4
			if (!bQuoted)
				if (UT_UCS4_isspace (ucs4))
#else
			if (!bQuoted && ((ucs4 & 0x7fff) == 0))
				if (UT_UCS_isspace (static_cast<UT_UCSChar>(ucs4 & 0xffff)))
#endif
				{
					bSpace = true;
					break;
				}
			while (static_cast<unsigned char>(*++csstr) & 0x80) { }
			if (!bSpace) value_end = csstr;
			continue;
		}
		else if ((*csstr == '\'') || (*csstr == '"'))
		{
			bQuoted = (bQuoted ? false : true);
		}
		else if (*csstr == ';')
		{
			if (!bQuoted)
			{
				csstr++;
				break;
			}
		}
		else if (!bQuoted && isspace ((int) u)) bSpace = true;

		csstr++;
		if (!bSpace) value_end = csstr;
	}
	return value_end;
}

static bool s_pass_number (char *& ptr, bool & bIsPercent)
{
	while (*ptr)
		{
			if (*ptr != ' ') break;
			ptr++;
		}
	unsigned char u = static_cast<unsigned char>(*ptr);
	if (!isdigit ((int) u)) return false;

	while (*ptr)
		{
			u = static_cast<unsigned char>(*ptr);
			if (!isdigit ((int) u)) break;
			ptr++;
		}
	if (*ptr == '%')
		{
			bIsPercent = true;
			*ptr = ' ';
		}
	else if ((*ptr == ' ') || (*ptr == 0))
		{
			bIsPercent = false;
		}
	else return false;

	return true;
}

static unsigned char s_rgb_number (float f, bool bIsPercent)
{
	if (f < 0) return 0;

	if (bIsPercent) f *= 2.55;

	if (f > 254.5) return 0xff;

	return (unsigned char) ((int) (f + 0.5));
}

static void s_props_append (UT_UTF8String & props, UT_uint32 css_mask,
							const char * name, char * value)
{
	UT_HashColor color;

	const char * verbatim = 0;

	if (css_mask & CSS_MASK_INLINE)
		{
			if (UT_strcmp (name, "font-weight") == 0)
				switch (*value)
					{
					case '1': case '2': case '3': case '4': case '5': case 'n': case 'l':
						if (props.byteLength ()) props += "; ";
						props += name;
						props += ":";
						props += "normal";
						break;
					case '6': case '7': case '8': case '9': case 'b':
						if (props.byteLength ()) props += "; ";
						props += name;
						props += ":";
						props += "bold";
						break;
					default: // inherit
						break;
					}
			else if (UT_strcmp (name, "font-style") == 0)
				{
					if (UT_strcmp (value, "normal") == 0)
						{
							if (props.byteLength ()) props += "; ";
							props += name;
							props += ":";
							props += "normal";
						}
					else if ((UT_strcmp (value, "italic") == 0) ||
							 (UT_strcmp (value, "oblique") == 0))
						{
							if (props.byteLength ()) props += "; ";
							props += name;
							props += ":";
							props += "italic";
						}
					// else inherit
				}
			else if ((UT_strcmp (name, "font-size")    == 0) ||
					 (UT_strcmp (name, "font-stretch") == 0) ||
					 (UT_strcmp (name, "font-variant") == 0))
				{
					verbatim = value;
				}
			else if (UT_strcmp (name, "font-family") == 0)
				{
					if ((UT_strcmp (value, "serif")      == 0) ||
						(UT_strcmp (value, "sans-serif") == 0) ||
						(UT_strcmp (value, "cursive")    == 0) ||
						(UT_strcmp (value, "fantasy")    == 0) ||
						(UT_strcmp (value, "monospace")  == 0))
						{
							verbatim = value;
						}
					else if ((*value == '\'') || (*value == '"'))
						{
							/* CSS requires font-family names to be quoted, and also allows
							 * a sequence of these to be specified; AbiWord doesn't quote,
							 * and allows only one font-family name.
							 */
							char * value_end = ++value;
							while (*value_end)
								{
									if ((*value_end == '\'') || (*value_end == '"')) break;
									value_end++;
								}
							if (*value_end)
								{
									*value_end = 0;
									verbatim = value;
								}
						}
				}
			else if (UT_strcmp (name, "text-decoration") == 0)
				{
					bool bInherit     = (strstr (value, "inherit")      != NULL);
					bool bUnderline   = (strstr (value, "underline")    != NULL);
					bool bLineThrough = (strstr (value, "line-through") != NULL);
					bool bOverline    = (strstr (value, "overline")     != NULL);

					if (bUnderline || bLineThrough || bOverline)
						{
							if (props.byteLength ()) props += "; ";
							props += name;
							props += ":";

							if (bUnderline) props += "underline";
							if (bLineThrough)
								{
									if (bUnderline) props += " ";
									props += "line-through";
								}
							if (bOverline)
								{
									if (bUnderline || bLineThrough) props += " ";
									props += "overline";
								}
						}
					else if (!bInherit)
						{
							if (props.byteLength ()) props += "; ";
							props += name;
							props += ":";
							props += "none";
						}
				}
			else if (UT_strcmp (name, "vertical-align") == 0)
				{
					/* AbiWord uses "text-position" for CSS's "vertical-align" in the case
					 * of super-/subscripts.
					 */
					if ((UT_strcmp (value, "superscript") == 0) ||
						(UT_strcmp (value, "subscript") == 0))
						{
							static const char * text_position = "text-position";
							name = text_position;
							verbatim = value;
						}
				}
			else if ((UT_strcmp (name, "color") == 0) || (UT_strcmp (name, "background") == 0))
				{
					/* AbiWord uses rgb hex-sequence w/o the # prefix used by CSS
					 * and uses "bgcolor" instead of background
					 */
					static const char * bgcolor = "bgcolor";

					if (UT_strcmp (name, "background") == 0) name = bgcolor;

					if (*value == '#')
						{
							value++;
							if (strlen (value) == 3)
							{
								unsigned int rgb;
								if (sscanf (value, "%x", &rgb) == 1)
								{
									unsigned int uir = (rgb & 0x0f00) >> 8;
									unsigned int uig = (rgb & 0x00f0) >> 4;
									unsigned int uib = (rgb & 0x000f);

									unsigned char r = static_cast<unsigned char>(uir|(uir<<4));
									unsigned char g = static_cast<unsigned char>(uig|(uig<<4));
									unsigned char b = static_cast<unsigned char>(uib|(uib<<4));

									verbatim = color.setColor (r, g, b);
								}
							}
							else if (strlen (value) == 6) verbatim = value;
						}
					else if (strncmp (value, "rgb(", 4) == 0)
						{
							value += 4;

							char * ptr = value;
							while (*ptr)
								{
									unsigned char u = static_cast<unsigned char>(*ptr);
									if ((isdigit ((int) u)) || (*ptr == '%'))
										{
											ptr++;
											continue;
										}
									*ptr++ = ' ';
								}
							bool bValid = true;

							bool b1pc = false;
							bool b2pc = false;
							bool b3pc = false;

							ptr = value;
							if (bValid) bValid = s_pass_number (ptr, b1pc);
							if (bValid) bValid = s_pass_number (ptr, b2pc);
							if (bValid) bValid = s_pass_number (ptr, b3pc);

							if (bValid)
							{
								float fr;
								float fg;
								float fb;

								if (sscanf (value, "%f %f %f", &fr, &fg, &fb) == 3)
								{
									unsigned char r = s_rgb_number (fr, b1pc);
									unsigned char g = s_rgb_number (fg, b2pc);
									unsigned char b = s_rgb_number (fb, b3pc);

									verbatim = color.setColor (r, g, b);
								}
							}
						}
					else
						{
							verbatim = color.lookupNamedColor (value);
						}
				}
		}
	if (css_mask & CSS_MASK_BLOCK)
		{
			/* potentially dangerous; TODO: check list-state??
			 */
			if ((UT_strcmp (name, "margin-left")   == 0) ||
				(UT_strcmp (name, "margin-right")  == 0) ||
				(UT_strcmp (name, "text-align")    == 0) ||
				(UT_strcmp (name, "text-indent")   == 0) ||
				(UT_strcmp (name, "orphans")       == 0) ||
				(UT_strcmp (name, "widows")        == 0))
				{
					verbatim = value;
				}
		}
	if (css_mask & CSS_MASK_IMAGE)
		{
			if ((UT_strcmp (name, "width")  == 0) ||
				(UT_strcmp (name, "height") == 0))
				{
					UT_Dimension units = UT_determineDimension (value, DIM_PX);
					double d = UT_convertDimensionless (value);
					float dim = (float) UT_convertDimensions (d, units, DIM_IN);
					UT_String tmp;
					char * old_locale = setlocale (LC_NUMERIC, "C");
					UT_String_sprintf (tmp, "%gin", dim);
					setlocale (LC_NUMERIC, old_locale);
					if (!tmp.empty ())
						{
							if (props.byteLength ()) props += "; ";
							props += name;
							props += ":";
							props += tmp.c_str ();
						}
				}
		}
	if (css_mask & CSS_MASK_BODY)
		{
			if ((UT_strcmp (name, "margin-bottom")    == 0) ||
				(UT_strcmp (name, "margin-top")       == 0) ||
				(UT_strcmp (name, "background-color") == 0))
				{
					verbatim = value;
				}
		}

	if (verbatim)
		{
			if (props.byteLength ()) props += "; ";
			props += name;
			props += ":";
			props += verbatim;
		}
}

static UT_UTF8String s_parseCSStyle (const UT_UTF8String & style, UT_uint32 css_mask)
{
	UT_UTF8String props;

	const char * csstr = style.utf8_str ();
	while (*csstr)
	{
		s_pass_whitespace (csstr);

		const char * name_start = csstr;
		const char * name_end   = s_pass_name (csstr);

		if (*csstr == 0) break; // whatever we have, it's not a "name:value;" pair
		if (name_start == name_end) break; // ?? stray colon?

		s_pass_whitespace (csstr);
		if (*csstr != ':') break; // whatever we have, it's not a "name:value;" pair

		csstr++;
		s_pass_whitespace (csstr);

		if (*csstr == 0) break; // whatever we have, it's not a "name:value;" pair

		const char * value_start = csstr;
		const char * value_end   = s_pass_value (csstr);

		if (value_start == value_end) break; // ?? no value...

		/* unfortunately there's no easy way to turn these two sequences into strings :-(
		 * atm, anyway
		 */
		char * name = (char *) malloc (name_end - name_start + 1);
		if (name)
		{
			strncpy (name, name_start, name_end - name_start);
			name[name_end - name_start] = 0;
		}
		char * value = (char *) malloc (value_end - value_start + 1);
		if (value)
		{
			strncpy (value, value_start, value_end - value_start);
			value[value_end - value_start] = 0;
		}

		if (name && value) s_props_append (props, css_mask, name, value);

		FREEP (name);
		FREEP (value);
	}
	return props;
}
