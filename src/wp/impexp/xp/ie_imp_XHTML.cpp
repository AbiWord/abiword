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

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_exception.h"
#include "ut_types.h"
#include "ut_base64.h"
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

typedef enum section_class
{
	sc_section = 0,
	sc_other
} SectionClass;

static const char * s_section_classes[sc_other] = {
	"Section"
};

#define CSS_MASK_INLINE (1<<0)
#define CSS_MASK_BLOCK  (1<<1)
#define CSS_MASK_IMAGE  (1<<2)
#define CSS_MASK_BODY   (1<<3)

static UT_UTF8String s_parseCSStyle (const UT_UTF8String & style, UT_uint32 css_mask);

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
    "props", "list-style:Numbered List; start-value:1; text-indent:-0.3in; field-font: NULL;",
	/* margin-left is purposefully left out of the props.  It is computed
	   based on the depth of the list, and appended to this list of
	   attributes.
	*/
    "style", "Normal",
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
    "props", "list-style:Bullet List; start-value:0; text-indent:-0.3in; field-font: Symbol;",
	/* margin-left is purposefully left out of the props.  It is computed
	   based on the depth of the list, and appended to this list of
	   attributes.
	*/
    "style", "Normal",
    NULL, NULL
  };

IE_Imp_XHTML::IE_Imp_XHTML(PD_Document * pDocument) :
	IE_Imp_XML(pDocument,false),
	m_listType(L_NONE),
    m_iListID(0),
	m_bFirstDiv(true),
	m_bUseTidy(false),
	m_iNewListID(0),
	m_iNewImage(0),
	m_szBookMarkName(NULL),
	m_addedPTXSection(false),
	m_iPreCount(0)
{
}

IE_Imp_XHTML::~IE_Imp_XHTML()
{
	UT_VECTOR_PURGEALL(UT_UTF8String *,m_divStyles);
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

static SectionClass s_class_query (const char * class_value)
{
	if ( class_value == 0) return sc_other;
	if (*class_value == 0) return sc_other;

	SectionClass sc = sc_other;

	for (int i = 0; i < static_cast<int>(sc_other); i++)
		if (UT_strcmp (class_value, s_section_classes[i]) == 0)
			{
				sc = static_cast<SectionClass>(i);
				break;
			}
	return sc;
}

static void s_append_font_family (UT_UTF8String & style, const char * face)
{
	unsigned char u;

	while (*face)
		{
			u = static_cast<unsigned char>(*face);
			if (!isspace (static_cast<int>(u))) break;
			++face;
		}
	if (*face == 0) return;

	char quote = ',';
	if ((*face == '"') || (*face == '\''))
		{
			quote = *face++;
		}

	char * value = UT_strdup (face);
	if (value == 0) return;

	char * ptr = value;
	while (*ptr)
		{
			if (*ptr == quote)
				{
					*ptr = 0;
					break;
				}
			++ptr;
		}
	if (quote == ',')
		while (ptr > value)
			{
				--ptr;
				u = static_cast<unsigned char>(*ptr);
				if (!isspace (static_cast<int>(u))) break;
				*ptr = 0;
			}
	if (strlen (value))
		{
			if (style.byteLength ())
				{
					style += "; ";
				}
			style += "font-family:";
			style += value;
		}
	free (value);
}

static const char * s_font_size[7] = {
	"8pt",
	"10pt",
	"11pt",
	"12pt",
	"16pt",
	"24pt",
	"32pt"
};

static void s_append_font_size (UT_UTF8String & style, const char * size)
{
	unsigned char u;

	while (*size)
		{
			u = static_cast<unsigned char>(*size);
			if (!isspace (static_cast<int>(u))) break;
			++size;
		}
	if (*size == 0) return;

	if (*size == '+')
		{
			int sz = atoi (size + 1);
			if ((sz < 1) || (sz > 7)) return;
			if (sz > 3) sz = 3;

			if (style.byteLength ())
				{
					style += "; ";
				}
			style += "font-size:";
			style += s_font_size[3+sz];
		}
	else if (*size == '-')
		{
			int sz = atoi (size + 1);
			if ((sz < 1) || (sz > 7)) return;
			if (sz > 3) sz = 3;

			if (style.byteLength ())
				{
					style += "; ";
				}
			style += "font-size:";
			style += s_font_size[3-sz];
		}
	else
		{
			int sz = atoi (size);
			if (sz == 0) return;

			if (style.byteLength ())
				{
					style += "; ";
				}
			style += "font-size:";

			if (sz <= 7)
				{
					style += s_font_size[sz-1];
				}
			else
				{
					UT_String pt_size;
					UT_String_sprintf(pt_size, "%2dpt", sz);
					style += pt_size.c_str ();
				}
		}
}

static void s_append_color (UT_UTF8String & style, const char * color, const char * property)
{
	unsigned char u;

	while (*color)
		{
			u = static_cast<unsigned char>(*color);
			if (!isspace (static_cast<int>(u))) break;
			++color;
		}
	if (*color == 0) return;

	char * value = UT_strdup (color);
	if (value == 0) return;

	int length = 0;

	bool bValid = true;
	bool bHexal = true;

	char * ptr = value;
	if (*ptr == '#') ++ptr;
	while (*ptr)
		{
			char c = *ptr;
			u = static_cast<unsigned char>(c);
			if (isspace (static_cast<int>(u)))
				{
					*ptr = 0;
					break;
				}
			if (!isalnum (static_cast<int>(u)))
				{
					bValid = false;
					break;
				}
			if (bHexal)
				if (!isdigit (static_cast<int>(u)))
					if (((c < 'a') && (c > 'f')) && ((c < 'A') && (c > 'F')))
						bHexal = false;
			++ptr;
			++length;
		}
	if (!bValid)
		{
			free (value);
			return;
		}
	if (*value == '#')
		if (!bHexal || ((length != 3) && (length != 6)))
			{
				free (value);
				return;
			}

	UT_HashColor hash_color;
	UT_UTF8String hex_color;

	if (*value == '#')
		{
			if (length == 3)
				{
					unsigned int rgb;
					if (sscanf (value + 1, "%x", &rgb) == 1)
						{
							unsigned int uir = (rgb & 0x0f00) >> 8;
							unsigned int uig = (rgb & 0x00f0) >> 4;
							unsigned int uib = (rgb & 0x000f);

							unsigned char r = static_cast<unsigned char>(uir|(uir<<4));
							unsigned char g = static_cast<unsigned char>(uig|(uig<<4));
							unsigned char b = static_cast<unsigned char>(uib|(uib<<4));

							hex_color = hash_color.setColor (r, g, b) + 1;
						}
				}
			else hex_color = value + 1;
		}
	else if (bHexal && (length == 6))
		{
			hex_color = value;
		}
	else
		{
			hex_color = hash_color.lookupNamedColor (color) + 1;
		}
	free (value);

	if (hex_color.byteLength () == 0) return;

	if (style.byteLength ())
		{
			style += "; ";
		}
	style += property;
	style += ":";
	style += hex_color;
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

void IE_Imp_XHTML::pasteFromBuffer(PD_DocumentRange * pDocRange,
								   const unsigned char * pData, 
								   UT_uint32 lenData, 
								   const char * szEncoding)
{
	UT_return_if_fail(getDoc() == pDocRange->m_pDoc);
	UT_return_if_fail(pDocRange->m_pos1 == pDocRange->m_pos2);

	setClipboard (pDocRange->m_pos1);

	UT_XML xml;
	xml.setListener (this);
	UT_ByteBuf buf (lenData);
	buf.append (pData, lenData);
	xml.parse (&buf);
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
		m_parseState = _PS_StyleSec;
		return;

	case TT_BODY:
	  //UT_DEBUGMSG(("Doc %d\n", m_parseState));
		X_VerifyParseState(_PS_StyleSec);
		m_parseState = _PS_Doc;
		return;		

	case TT_DIV:
		{
			/* <div> is a block marker; <span> is an inline marker
			 */
			if (m_parseState == _PS_Block) m_parseState = _PS_Sec;

			/* stack class attr. values if recognized;
			 * NOTE: these are ptrs to static strings - don't alloc/free them
			 */
			const XML_Char * p_val = _getXMLPropValue (static_cast<const XML_Char *>("class"), atts);
			SectionClass sc = childOfSection () ? sc_other : s_class_query (p_val);
			if (sc == sc_other)
				m_divClasses.push_back (0);
			else
				m_divClasses.push_back (const_cast<char *>(s_section_classes[sc]));

			/* <div> elements can specify block-level styles; concatenate and stack...
			 */
			UT_UTF8String * prev = 0;
			if (m_divStyles.getItemCount ())
				{
					prev = reinterpret_cast<UT_UTF8String *>(m_divStyles.getLastItem ());
				}
			UT_UTF8String * style = 0;
			if (prev)
				style = new UT_UTF8String(*prev);
			else
				style = new UT_UTF8String;

			if (style)
				{
					p_val = _getXMLPropValue (static_cast<const XML_Char *>("align"), atts);
					if (p_val)
						{
							if (!UT_XML_strcmp (p_val, "right"))
								*style += "text-align: right; ";
							else if (!UT_XML_strcmp (p_val, "center"))
								*style += "text-align: center; ";
							else if (!UT_XML_strcmp (p_val, "left"))
								*style += "text-align: left; ";
							else if (!UT_XML_strcmp (p_val, "justify"))
								*style += "text-align: justify; ";
						}
				}

			p_val = _getXMLPropValue (static_cast<const XML_Char *>("style"), atts);
			if (style && p_val)
				{
					*style += p_val;
					*style += "; ";
				}

			m_divStyles.push_back (style);

			/* for top-level section <div> elements, create a new document section
			 */
			if (sc != sc_other)
				{
					m_parseState = _PS_Doc;
					X_CheckError(requireSection ()); // TODO: handle this intelligently
				}
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
		X_CheckError(pushInline ("font-style:italic"));
		return;

	case TT_DFN:
	case TT_STRONG:
	case TT_B:
		X_CheckError(pushInline ("font-weight:bold"));
		return;

	case TT_CODE:
	case TT_TT:
		X_CheckError(pushInline ("font-family:Courier"));
		return;

	case TT_U:
		X_CheckError(pushInline ("text-decoration:underline"));
		return;

	case TT_S://	case TT_STRIKE:
		X_CheckError(pushInline ("text-decoration:line-through"));
		return;

	case TT_SUP:
		X_CheckError(pushInline ("text-position:superscript"));
		return;

	case TT_SUB:
		X_CheckError(pushInline ("text-position:subscript"));
		return;
		
	case TT_FONT:
		UT_DEBUGMSG(("Font tag encountered\n"));
		{
			UT_UTF8String style;
			
			const XML_Char * p_val = 0;

			p_val = _getXMLPropValue (static_cast<const XML_Char *>("color"), atts);
			if (p_val) s_append_color (style, p_val, "color");

			p_val = _getXMLPropValue (static_cast<const XML_Char *>("background"), atts);
			if (p_val) s_append_color (style, p_val, "bgcolor");

			p_val = _getXMLPropValue (static_cast<const XML_Char *>("size"), atts);
			if (p_val) s_append_font_size (style, p_val);
			
			p_val = _getXMLPropValue (static_cast<const XML_Char *>("face"), atts);
			if (p_val) s_append_font_family (style, p_val);
			
			// UT_String_sprintf(output, "color:%s; bgcolor: %s; font-family:%s; size:%spt", color.c_str(), bgcolor.c_str(), face.c_str(), size.c_str());
			UT_DEBUGMSG(("Font properties: %s\n", style.utf8_str()));

			X_CheckError(pushInline (style.utf8_str ()));
		}
		return;

	case TT_PRE:
		if (m_parseState == _PS_Block) m_parseState = _PS_Sec;
		m_iPreCount++;
		m_bWhiteSignificant = true;
		return;

	case TT_H1:
	case TT_H2:
	case TT_H3:
	case TT_BLOCKQUOTE:
		// &...
	case TT_P:
	case TT_TR:
	case TT_H4:
	case TT_H5:
	case TT_H6:
		if (m_listType != L_NONE)
		{
			/* hmm, document/paragraph structure inside list
			 */
			if (_data_CharCount ())
				{
					UT_UCSChar ucs = UCS_LF;
					X_CheckError(appendSpan (&ucs, 1));
					_data_NewBlock ();
				}
		}
		else
		{
			const XML_Char * style = _getXMLPropValue (static_cast<const XML_Char *>("style"), atts);
			const XML_Char * align = _getXMLPropValue (static_cast<const XML_Char *>("align"), atts);

			const XML_Char * p_val = _getXMLPropValue (static_cast<const XML_Char *>("awml:style"), atts);

			if (p_val)
				{
					X_CheckError (newBlock (p_val, style, align));
				}
			else if (tokenIndex == TT_H1)
				{
					X_CheckError (newBlock ("Heading 1", style, align));
				}
			else if (tokenIndex == TT_H2)
				{
					X_CheckError (newBlock ("Heading 2", style, align));
				}
			else if (tokenIndex == TT_H3)
				{
					X_CheckError (newBlock ("Heading 3", style, align));
				}
			else if (tokenIndex == TT_BLOCKQUOTE)
				{
					X_CheckError (newBlock ("Block Text", style, align));
				}
			else if (m_bWhiteSignificant)
				{
					X_CheckError (newBlock ("Plain Text", style, align));
				}
			else
				{
					X_CheckError (newBlock ("Normal", style, align));
				}
		}
		return;

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
		X_CheckError (requireSection ());
		m_parseState = _PS_Block;

		XML_Char *sz;

		if (m_listType != L_NONE)
		{
			UT_uint16 thisID = m_iListID;
			m_utsParents.viewTop(reinterpret_cast<void**>(&parentID));

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

			const XML_Char* temp = static_cast<const XML_Char*>(listAtts[propsPos]);
			listAtts[propsPos] = props.c_str();

			X_CheckError(appendStrux(PTX_Block, listAtts));

			listAtts[propsPos] = temp;

			// append a field
			UT_XML_cloneString(sz, "type");
			new_atts[0] = sz;
			UT_XML_cloneString(sz, "list_label");
			new_atts[1] = sz;
			X_CheckError(appendObject(PTO_Field, new_atts));

			// append the character run
			UT_XML_cloneString(sz, "type");
			new_atts[0] = sz;
			UT_XML_cloneString(sz, "list_label");
			new_atts[1] = sz;
			X_CheckError(appendFmt(new_atts));

			/* warn XML charData() handler of new block, but insert a tab first
			 */
			UT_UCSChar ucs = UCS_TAB;
			X_CheckError(appendSpan (&ucs, 1));
			_data_NewBlock ();
		}
		return;
	}

	case TT_SPAN:
		{
			UT_UTF8String utf8val;

			const XML_Char * p_val = _getXMLPropValue (static_cast<const XML_Char *>("style"), atts);
			if (p_val)
				{
					utf8val = static_cast<const char *>(p_val);
					utf8val = s_parseCSStyle (utf8val, CSS_MASK_INLINE);
					UT_DEBUGMSG(("CSS->Props (utf8val): [%s]\n",utf8val.utf8_str()));
				}
			X_CheckError(pushInline (utf8val.utf8_str ()));
		}
		return;

	case TT_BR:
	  //UT_DEBUGMSG(("B %d\n", m_parseState));
		if(m_parseState == _PS_Block)
		{
			UT_UCSChar ucs = UCS_LF;
			X_CheckError(appendSpan(&ucs,1));
		}
		return;

	case TT_A:
	{
		const XML_Char * p_val = 0;
		p_val = _getXMLPropValue(static_cast<const XML_Char *>("xlink:href"), atts);
		if (p_val == 0) p_val = _getXMLPropValue(static_cast<const XML_Char *>("href"), atts);
		if( p_val )
		{
			X_CheckError(requireBlock ());
		    UT_XML_cloneString(sz, "xlink:href");
		    new_atts[0] = sz;
	    	sz = NULL;
		    UT_XML_cloneString(sz, p_val);
		    new_atts[1] = sz;
			X_CheckError(appendObject(PTO_Hyperlink,new_atts));
		}
		else
		{
			p_val = _getXMLPropValue(static_cast<const XML_Char *>("id"), atts);
			if (p_val == 0) p_val = _getXMLPropValue(static_cast<const XML_Char *>("name"), atts);
			if (p_val)
			{
				X_CheckError(requireBlock ());

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
					X_CheckError(appendObject(PTO_Bookmark,bm_new_atts));
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
					X_CheckError(appendObject(PTO_Bookmark,bm_new_atts));

					FREEP(m_szBookMarkName);
					m_szBookMarkName = NULL;
				}
			}
		}
		return;
	}

	case TT_IMG:
		{
		const XML_Char * szSrc    = _getXMLPropValue (static_cast<const XML_Char *>("src"),    atts);
		const XML_Char * szStyle  = _getXMLPropValue (static_cast<const XML_Char *>("style"),  atts);
		const XML_Char * szWidth  = _getXMLPropValue (static_cast<const XML_Char *>("width"),  atts);
		const XML_Char * szHeight = _getXMLPropValue (static_cast<const XML_Char *>("height"), atts);

		if ( szSrc == 0) break;
		if (*szSrc == 0) break;

		FG_Graphic * pfg = 0;

		if (strncmp (szSrc, "data:", 5) == 0) // data-URL - special case
				pfg = importDataURLImage (szSrc + 5);
		else if (!isClipboard ())
			pfg = importImage (szSrc);

		if (pfg == 0) break;

		UT_ByteBuf * pBB = static_cast<FG_GraphicRaster *>(pfg)->getRaster_PNG ();
		X_CheckError(pBB);

		char * mimetype = UT_strdup ("image/png");
		X_CheckError(mimetype);

		UT_UTF8String utf8val;
		if (szStyle)
			{
				utf8val = static_cast<const char *>(szStyle);
				utf8val = s_parseCSStyle (utf8val, CSS_MASK_IMAGE);
				UT_DEBUGMSG(("CSS->Props (utf8val): [%s]\n",utf8val.utf8_str()));
			}
		if (szWidth && (strstr (utf8val.utf8_str (), "width") == 0))
			{
				UT_Dimension units = UT_determineDimension (szWidth, DIM_PX);
				double d = UT_convertDimensionless (szWidth);
				float width = static_cast<float>(UT_convertDimensions (d, units, DIM_IN));
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
				float height = static_cast<float>(UT_convertDimensions (d, units, DIM_IN));
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
		UT_String_sprintf (dataid, "image%u", static_cast<unsigned int>(m_iNewImage++));

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
				X_CheckError(requireBlock ());
			}
		UT_DEBUGMSG(("inserting `%s' as `%s' [%s]\n",szSrc,dataid.c_str(),utf8val.utf8_str()));

		X_CheckError(appendObject (PTO_Image, api_atts));
		X_CheckError(getDoc()->createDataItem (dataid.c_str(), false, pBB, static_cast<void*>(mimetype), NULL));

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
		X_CheckError(requireBlock ());
		X_CheckError(appendFmt(new_atts));
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
		m_parseState = _PS_Init; // irrel. - shouldn't see anything after this
		return;

	case TT_BODY:
		/* add two empty blocks at the end...
		 */
		newBlock ("Normal", 0, 0);
		newBlock ("Normal", 0, 0);

		m_parseState = _PS_Init;
		return;

	case TT_DIV:
		{
			if (m_parseState == _PS_Block) m_parseState = _PS_Sec;

			m_divClasses.pop_back ();

			if (m_divStyles.getItemCount ())
				{
					UT_UTF8String * prev = 0;
					prev = reinterpret_cast<UT_UTF8String *>(m_divStyles.getLastItem ());
					DELETEP(prev);
				}
			m_divStyles.pop_back ();
		}
		return;

	case TT_OL:
	case TT_UL:
	case TT_DL:
		UT_uint16 *temp;

		if(m_utsParents.pop(reinterpret_cast<void**>(&temp)))
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
		UT_ASSERT(m_lenCharDataSeen==0);
		m_parseState = _PS_Sec;
		while (_getInlineDepth ()) _popInlineFmt ();
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
		if (m_listType == L_NONE)
			{
				m_parseState = _PS_Sec;
				while (_getInlineDepth ()) _popInlineFmt ();
			}
		return;

		/* text formatting
		 */

	case TT_S://	case TT_STRIKE:
	case TT_U:
	case TT_SUP:
	case TT_SUB:
	case TT_FONT:	
	case TT_CODE:
	case TT_TT:
	case TT_DFN:
	case TT_STRONG:
	case TT_B:
	case TT_Q:
	case TT_SAMP:
	case TT_VAR:
	case TT_KBD:
	case TT_ADDRESS:
	case TT_CITE:
	case TT_EM:
	case TT_I:
		UT_ASSERT(m_lenCharDataSeen==0);
		_popInlineFmt ();
		if (m_parseState == _PS_Block)
			{
				X_CheckError(appendFmt(&m_vecInlineFmt));
			}
		return;

	case TT_SPAN:
		UT_ASSERT(m_lenCharDataSeen==0);
		//UT_DEBUGMSG(("B %d\n", m_parseState));
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()>0);
		_popInlineFmt();
		appendFmt(&m_vecInlineFmt);
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
			X_CheckError(appendObject(PTO_Bookmark,bm_new_atts));
			for(i = 0; i < 5; i++) FREEP(bm_new_atts[i]);
			FREEP(m_szBookMarkName);
			m_szBookMarkName = NULL;
		}
		else if (m_parseState == _PS_Block)
		{
			/* if (m_parseState == _PS_Sec) then this is an anchor outside
			 * of a block, not a hyperlink (see TT_A in startElement)
			 */
 			X_CheckError(appendObject(PTO_Hyperlink,0));
		}
		return;

	case TT_PRE:
		UT_ASSERT(m_lenCharDataSeen==0);
		if (m_parseState == _PS_Block) m_parseState = _PS_Sec;
		m_iPreCount--;
		m_bWhiteSignificant = (m_iPreCount > 0);
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
		X_CheckError(appendFmt(&m_vecInlineFmt));
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
	if ((m_parseState == _PS_StyleSec) || (m_parseState == _PS_Init))
		{
			return; // outside body here
		}

	/* No need to insert new blocks if we're just looking at the spaces
	 * between XML elements - unless we're in a <pre> sequence
	 */
	if (!m_bWhiteSignificant && (m_parseState != _PS_Block))
	{
#ifdef XHTML_UCS4
		UT_UCS4String buf(buffer,static_cast<size_t>(length),!m_bWhiteSignificant);
#else
		UT_UCS2String buf(buffer,static_cast<size_t>(length),!m_bWhiteSignificant);
#endif
		if (buf.size () == 0) return; // probably shouldn't happen; not sure
		if ((buf.size () == 1) && (buf[0] == UCS_SPACE)) return;
	}

	int failLine = 0;

	// bool bResetState = (m_parseState != _PS_Block);

	X_CheckError(requireBlock ());

	IE_Imp_XML::charData (buffer, length);

	// if (bResetState) m_parseState = _PS_Sec;

	return;

X_Fail:
	UT_DEBUGMSG (("X_Fail at %d\n", failLine));
	return;
}

FG_Graphic * IE_Imp_XHTML::importDataURLImage (const XML_Char * szData)
{
	if (strncmp (szData, "image/", 6))
		{
			UT_DEBUGMSG(("importDataURLImage: URL-embedded data does not appear to be an image...\n"));
			return 0;
		}
	const char * b64bufptr = static_cast<const char *>(szData);

	while (*b64bufptr) if (*b64bufptr++ == ',') break;
	size_t b64length = static_cast<size_t>(strlen (b64bufptr));
	if (b64length == 0)
		{
			UT_DEBUGMSG(("importDataURLImage: URL-embedded data has no data?\n"));
			return 0;
		}

	size_t binmaxlen = ((b64length >> 2) + 1) * 3;
	size_t binlength = binmaxlen;
	char * binbuffer = static_cast<char *>(malloc (binmaxlen));
	if (binbuffer == 0)
		{
			UT_DEBUGMSG(("importDataURLImage: out of memory\n"));
			return 0;
		}
	char * binbufptr = binbuffer;

	if (!UT_UTF8_Base64Decode (binbufptr, binlength, b64bufptr, b64length))
		{
			UT_DEBUGMSG(("importDataURLImage: error decoding Base64 data - I assume that's what it is...\n"));
			FREEP(binbuffer);
			return 0;
		}
	binlength = binmaxlen - binlength;

	UT_ByteBuf * pBB = new UT_ByteBuf;
	if (pBB == 0)
		{
			UT_DEBUGMSG(("importDataURLImage: out of memory\n"));
			FREEP(binbuffer);
			return 0;
		}
	pBB->ins (0, reinterpret_cast<const UT_Byte *>(binbuffer), binlength);
	FREEP(binbuffer);

	IE_ImpGraphic * pieg = 0;
	if (IE_ImpGraphic::constructImporter (pBB, IEGFT_Unknown, &pieg) != UT_OK)
		{
			UT_DEBUGMSG(("unable to construct image importer!\n"));
			return 0;
		}
	if (pieg == 0) return 0;

	FG_Graphic * pfg = 0;
	UT_Error import_status = pieg->importGraphic (pBB, &pfg);
	delete pieg;
	if (import_status != UT_OK)
		{
			UT_DEBUGMSG(("unable to import image!\n"));
			return 0;
		}
	UT_DEBUGMSG(("image loaded successfully\n"));

	return pfg;
}

FG_Graphic * IE_Imp_XHTML::importImage (const XML_Char * szSrc)
{
	const char * szFile = static_cast<const char *>(szSrc);

	if (strncmp (szFile, "http://", 7) == 0)
		{
			UT_DEBUGMSG(("found web image reference (%s) - skipping...\n",szFile));
			return 0;
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
			return 0;
		}
	UT_DEBUGMSG(("found image reference (%s) - loading... \n",szFile));

	IE_ImpGraphic * pieg = 0;
	if (IE_ImpGraphic::constructImporter (szFile, IEGFT_Unknown, &pieg) != UT_OK)
		{
			UT_DEBUGMSG(("unable to construct image importer!\n"));
			return 0;
		}
	if (pieg == 0) return 0;

	FG_Graphic * pfg = 0;
	UT_Error import_status = pieg->importGraphic (szFile, &pfg);
	delete pieg;
	if (import_status != UT_OK)
		{
			UT_DEBUGMSG(("unable to import image!\n"));
			return 0;
		}
	UT_DEBUGMSG(("image loaded successfully\n"));

	return pfg;
}

bool IE_Imp_XHTML::pushInline (const char * props)
{
	if (!requireBlock ()) return false;

	const XML_Char * api_atts[3];

	XML_Char * sz = NULL;

	UT_XML_cloneString (sz, PT_PROPS_ATTRIBUTE_NAME);
	if (sz == NULL)
		return false;
	api_atts[0] = sz;

	sz = NULL;

	UT_XML_cloneString (sz, props);
	if (sz == NULL)
		return false;
	api_atts[1] = sz;

	api_atts[2] = NULL;

	_pushInlineFmt (api_atts);
	return appendFmt (&m_vecInlineFmt);
}

bool IE_Imp_XHTML::newBlock (const char * style_name, const char * css_style, const char * align)
{
	if (!requireSection ()) return false;

	UT_UTF8String * div_style = 0;
	if (m_divStyles.getItemCount ())
		div_style = reinterpret_cast<UT_UTF8String *>(m_divStyles.getLastItem ());

	UT_UTF8String style;
	if (div_style)
		style = *div_style;
	if (align)
		{
			if (!UT_XML_strcmp (align, "right"))
				style += "text-align: right; ";
			else if (!UT_XML_strcmp (align, "center"))
				style += "text-align: center; ";
			else if (!UT_XML_strcmp (align, "left"))
				style += "text-align: left; ";
			else if (!UT_XML_strcmp (align, "justify"))
				style += "text-align: justify; ";
		}
	if (css_style)
		style += css_style;

	UT_UTF8String utf8val = s_parseCSStyle (style, CSS_MASK_BLOCK);
	UT_DEBUGMSG(("CSS->Props (utf8val): [%s]\n",utf8val.utf8_str()));

	const XML_Char * api_atts[5];

	api_atts[2] = NULL;
	api_atts[4] = NULL;

	XML_Char * sz = NULL;

	UT_XML_cloneString (sz, PT_STYLE_ATTRIBUTE_NAME);
	if (sz == NULL)
		return false;
	api_atts[0] = sz;

	sz = NULL;

	UT_XML_cloneString (sz, style_name);
	if (sz == NULL)
		return false;
	api_atts[1] = sz;

	if (utf8val.byteLength ())
		{
			sz = NULL;

			UT_XML_cloneString (sz, PT_PROPS_ATTRIBUTE_NAME);
			if (sz == NULL)
				return false;
			api_atts[2] = sz;

			sz = NULL;

			UT_XML_cloneString (sz, utf8val.utf8_str ());
			if (sz == NULL)
				return false;
			api_atts[3] = sz;
		}
	if (!appendStrux (PTX_Block, api_atts))
		{
			return false;
		}
	m_parseState = _PS_Block;

	_data_NewBlock (); // warn XML charData() handler that a new block is beginning

	while (_getInlineDepth()) _popInlineFmt ();

	utf8val = s_parseCSStyle (style, CSS_MASK_INLINE);
	UT_DEBUGMSG(("CSS->Props (utf8val): [%s]\n",utf8val.utf8_str()));

	return pushInline (utf8val.utf8_str ());
}

/* forces document into block state; returns false on failure
 */
bool IE_Imp_XHTML::requireBlock ()
{
	if (m_parseState == _PS_Block) return true;

	return m_bWhiteSignificant ? newBlock ("Plain Text", 0, 0) : newBlock ("Normal", 0, 0);
}

/* forces document into section state; returns false on failure
 */
bool IE_Imp_XHTML::requireSection ()
{
	if (m_parseState == _PS_Sec) return true;

	if (!appendStrux (PTX_Section,NULL))
		{
			return false;
		}
	m_parseState = _PS_Sec;

	m_addedPTXSection = true;
	return true;
}

/* returns true if child of a <div> with a recognized "class" attribute
 */
bool IE_Imp_XHTML::childOfSection ()
{
	bool bChild = false;
	UT_uint32 count = m_divClasses.getItemCount ();

	for (UT_uint32 i = 0; i < count; i++)
		if (m_divClasses[i])
			{
				bChild = true;
				break;
			}
	return bChild;
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
		else if (isspace (static_cast<int>(u)))
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
		else if ((isspace (static_cast<int>(u))) || (*csstr == ':'))
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
		else if (!bQuoted && isspace (static_cast<int>(u))) bSpace = true;

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
	if (!isdigit (static_cast<int>(u))) return false;

	while (*ptr)
		{
			u = static_cast<unsigned char>(*ptr);
			if (!isdigit (static_cast<int>(u))) break;
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
									if ((isdigit (static_cast<int>(u))) || (*ptr == '%'))
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
					float dim = static_cast<float>(UT_convertDimensions (d, units, DIM_IN));
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
		char * name = static_cast<char *>(malloc (name_end - name_start + 1));
		if (name)
		{
			strncpy (name, name_start, name_end - name_start);
			name[name_end - name_start] = 0;
		}
		char * value = static_cast<char *>(malloc (value_end - value_start + 1));
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
