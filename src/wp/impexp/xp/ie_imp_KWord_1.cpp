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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA··
 * 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_KWord_1.h"
#include "ie_types.h"
#include "pd_Document.h"
#include "ut_growbuf.h"
#include "ut_units.h"

#include "ut_string_class.h"

/*
 * This file is meant to import KWord 1.x documents.
 * Kword is a component of KOffice for KDE.
 */

/*****************************************************************/
/*****************************************************************/

#define X_CheckError(v)			do {  if (!(v))								\
									  {  m_error = UT_ERROR;			\
										 return; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_error) return; } while (0)

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

// we use a reference-counted sniffer
static IE_Imp_KWord_1_Sniffer * m_sniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo *mi)
{
  if (!m_sniffer)
  {
    m_sniffer = new IE_Imp_KWord_1_Sniffer ();
  }
  else
  {
    m_sniffer->ref();
  }

  mi->name = "KWord 1.x Importer";
  mi->desc = "Import KWord 1.x Documents";
  mi->version = ABI_VERSION_STRING;
  mi->author = "Abi the Ant";
  mi->usage = "No Usage";

  IE_Imp::registerImporter(m_sniffer);
  return 1;
}

ABI_FAR_CALL
int abi_plugin_unregister(XAP_ModuleInfo *mi)
{
  mi->name = 0;
  mi->desc = 0;
  mi->version = 0;
  mi->author = 0;
  mi->usage = 0;

  UT_ASSERT(m_sniffer);

  IE_Imp::unregisterImporter(m_sniffer);
  if (!m_sniffer->unref())
  {
    m_sniffer = 0;
  }

  return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version(UT_uint32 major, UT_uint32 minor, UT_uint32 release)
{
  return 1;
}

#endif

/***********************************************************/
/***********************************************************/

UT_uint8 IE_Imp_KWord_1_Sniffer::recognizeContents(const char *szBuf, UT_uint32 iNumbytes)
{

  // first, look for the <?xml at the beginning of the document
  if(strncmp(szBuf, "<?xml", 4) != 0)
    return IMP_CONFIDENCE_ZILCH;

  // now look for the KWord beginning tag <DOC
  if(strstr(szBuf, "<DOC") == NULL)
    return IMP_CONFIDENCE_ZILCH;

  return IMP_CONFIDENCE_PERFECT;
}

UT_uint8 IE_Imp_KWord_1_Sniffer::recognizeSuffix(const char *szSuffix)
{
  if (UT_stricmp(szSuffix,".kwd") == 0)
    return IMP_CONFIDENCE_PERFECT;
  return IMP_CONFIDENCE_ZILCH;
}

UT_Error IE_Imp_KWord_1_Sniffer::constructImporter(PD_Document *pDocument, IE_Imp **ppie)
{
  IE_Imp_KWord_1 *p = new IE_Imp_KWord_1(pDocument);
  *ppie = p;
  return UT_OK;
}

bool IE_Imp_KWord_1_Sniffer::getDlgLabels(const char **pszDesc, const char **pszSuffixList,
                                          IEFileType *ft)
{
  *pszDesc = "KWord 1.x (.kwd)";
  *pszSuffixList = "*.kwd";
  *ft = getFileType();
  return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_KWord_1::~IE_Imp_KWord_1()
{
}

IE_Imp_KWord_1::IE_Imp_KWord_1(PD_Document *pDocument) : IE_Imp_XML(pDocument, true)
  , m_bInText(false)
{
}

void IE_Imp_KWord_1::charData(const XML_Char *s, int len)
{
  X_EatIfAlreadyError();	// xml parser keeps running until buffer consumed

  UT_ASSERT(sizeof(XML_Char) == sizeof(UT_Byte));
  UT_ASSERT(sizeof(XML_Char) != sizeof(UT_UCSChar));

  if(!m_bInText)
    return;
  
  // parse UTF-8 text and convert to Unicode.
  
  UT_Byte * ss = (UT_Byte *)s;
  UT_UCS2String buf;
  UT_Byte currentChar;
  
  for (int k=0; k<len; k++)
    {
      currentChar = ss[k];
      
      if ((ss[k] < 0x80) && (m_lenCharDataSeen > 0))
	{
	  // is it us-ascii and we are in a UTF-8
	  // multi-byte sequence.  puke.
	  X_CheckError(0);
	}
      
      if (currentChar < 0x80)			// plain us-ascii part of latin-1
	{
	  buf += ss[k];		// copy as is.
	}
      else if ((currentChar & 0xf0) == 0xf0)	// lead byte in 4-byte surrogate pair
	{
	  // surrogate pairs are defined in section 3.7 of the
	  // unicode standard version 2.0 as an extension
	  // mechanism for rare characters in future extensions
	  // of the unicode standard.
	  UT_ASSERT(m_lenCharDataSeen == 0);
	  UT_ASSERT(UT_NOT_IMPLEMENTED);
	}
      else if ((currentChar & 0xe0) == 0xe0)  // lead byte in 3-byte sequence
	{
	  UT_ASSERT(m_lenCharDataSeen == 0);
	  m_lenCharDataExpected = 3;
	  m_charDataSeen[m_lenCharDataSeen++] = currentChar;
	}
      else if ((currentChar & 0xc0) == 0xc0)	// lead byte in 2-byte sequence
	{
	  UT_ASSERT(m_lenCharDataSeen == 0);
	  m_lenCharDataExpected = 2;
	  m_charDataSeen[m_lenCharDataSeen++] = currentChar;
	}
      else if ((currentChar & 0x80) == 0x80)		// trailing byte in multi-byte sequence
	{
	  UT_ASSERT(m_lenCharDataSeen > 0);
	  m_charDataSeen[m_lenCharDataSeen++] = currentChar;
	  if (m_lenCharDataSeen == m_lenCharDataExpected)
	    {
	      buf += UT_decodeUTF8char(m_charDataSeen,m_lenCharDataSeen);
	      m_lenCharDataSeen = 0;
	    }
	}
    }

  m_szTextBuffer += buf;
}

/*****************************************************************/
/*****************************************************************/

#define TT_OTHER           0                    // anything else
#define TT_ATTRIBUTE       1                    // attributes of document
#define TT_BOTTOMBORDER    2                    // bottom border
#define TT_CHARSET         3                    // chatset
#define TT_CLIPARTS        4                    // cliparts
#define TT_COLOR           5                    // color stuff
#define TT_COUNTER         6                    // ??
#define TT_DOC             7                    // a document <kwd>
#define TT_FLOW            8                    // allignment
#define TT_FOLLOWING       9                    // ??
#define TT_FONT            10                   // a font
#define TT_FORMAT          11                   // ??
#define TT_FORMATS         12                   // ??
#define TT_FRAME           13                   // a frame
#define TT_FRAMESET        14                   // a frameset
#define TT_INDENTS         15                   // indent
#define TT_ITALIC          16                   // italic font
#define TT_LAYOUT          17                   // layout
#define TT_LEFTBORDER      18                   // left border
#define TT_LINESPACING     19                   // line spacing
#define TT_NAME            20                   // ??
#define TT_OFFSETS         21                   // offsets
#define TT_PAPER           22                   // ??
#define TT_PAPERBORDERS    23                   // ??
#define TT_PAGEBREAKING    24                   // page breaking
#define TT_PARAGRAPH       25                   // paragraphs
#define TT_RIGHTBORDER     26                   // right border
#define TT_SIZE            27                   // size of a font
#define TT_STRIKEOUT       28                   // strikeout font
#define TT_STYLE           29                   // style
#define TT_STYLES          30                   // styles
#define TT_TEXT            31                   // written text
#define TT_TOPBORDER       32                   // top border
#define TT_UNDERLINE       33                   // underline font
#define TT_VERTALIGN       34                   // vertical alignment
#define TT_WEIGHT          35                   // font weight

// KEEP IN ALPHABETICAL ORDER!!

static struct xmlToIdMapping s_Tokens[] =
{
  { "ATTRIBUTE",     TT_ATTRIBUTE    },
  { "BOTTOMBORDER",  TT_BOTTOMBORDER },
  { "CHARSET",       TT_CHARSET      },
  { "CLIPARTS",      TT_CLIPARTS     },
  { "COLOR",      TT_COLOR     },
  { "COUNTER",       TT_COUNTER      },
  { "DOC",           TT_DOC          },
  { "FLOW",          TT_FLOW         },
  { "FOLLOWING",     TT_FOLLOWING    },
  { "FONT",          TT_FONT         },
  { "FORMAT",        TT_FORMAT       },
  { "FORMATS",       TT_FORMATS      },
  { "FRAME",         TT_FRAME        },
  { "FRAMESET",      TT_FRAMESET     },
  { "INDENTS",       TT_INDENTS      },
  { "ITALIC",        TT_ITALIC       },
  { "LAYOUT",        TT_LAYOUT       },
  { "LEFTBORDER",    TT_LEFTBORDER   },
  { "LINESPACING",   TT_LINESPACING  },
  { "NAME",          TT_NAME         },
  { "OFFSETS",       TT_OFFSETS      },
  { "PAPER",         TT_PAPER        },
  { "PAPERBORDERS",  TT_PAPERBORDERS },
  { "PAGEBREAKING",  TT_PAGEBREAKING },
  { "PARAGRAPH",     TT_PARAGRAPH    },
  { "RIGHTBORDER",   TT_RIGHTBORDER  },
  { "SIZE",          TT_SIZE         },
  { "STRIKEOUT",     TT_STRIKEOUT    },
  { "STYLE",         TT_STYLE        },
  { "STYLES",        TT_STYLES       },
  { "TEXT",          TT_TEXT         },
  { "TOPBORDER",     TT_TOPBORDER    },
  { "UNDERLINE",     TT_UNDERLINE    },
  { "VERTALIGN",     TT_VERTALIGN    },
  { "WEIGHT",        TT_WEIGHT       }
};

#define TokenTableSize ((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

/*****************************************************************/
/*****************************************************************/

#define X_TestParseState(ps) ((m_parseState==(ps)))

#define X_VerifyParseState(ps) do { if (!(X_TestParseState(ps))) { m_error = UT_IE_BOGUSDOCUMENT; return; } } while(0)

#define X_CheckDocument(b) do {  if (!(b)) { m_error = UT_IE_BOGUSDOCUMENT; return; } } while (0)

#define X_CheckError(v) do {  if (!(v)) {  m_error = UT_ERROR; return; } } while (0)

#define X_EatIfAlreadyError() do {  if (m_error) return; } while (0)

/*****************************************************************/
/*****************************************************************/

// turns a KWord justification number into left/right/center/justify
static const char *
numberToJustification(const char * justification_name)
{
	if (! strcmp(justification_name,"0"))
		return "left";
	else if (! strcmp(justification_name,"1"))
		return "right";
	else if (! strcmp(justification_name,"2"))
		return "center";
	else if (! strcmp(justification_name,"3"))
		return "justify";
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return "";
	}
}

// turns as KWord page-size number into a fp_PageSize::Predefined
static fp_PageSize::Predefined
kPageToFpPageSize (const char * sz)
{
  // TODO: handle more of these

  if(!strcmp(sz, "0"))
    return fp_PageSize::A3;
  else if(!strcmp(sz, "1"))
    return fp_PageSize::A4;
  else if(!strcmp(sz, "2"))
    return fp_PageSize::A5;
  else if(!strcmp(sz, "3"))
    return fp_PageSize::Letter;
  else if(!strcmp(sz, "4"))
    return fp_PageSize::Legal;
  else if(!strcmp(sz, "7"))
    return fp_PageSize::B5;
  else
    return fp_PageSize::Custom;
}

// superscript/subscript/normal
static const char *
kVertAlignToTextPos ( const char * sz )
{
  if (!strcmp(sz, "1"))
    return "superscript";
  else if(!strcmp(sz, "2"))
    return "subscript";
  else
    return "normal";
}

/*****************************************************************/
/*****************************************************************/

void IE_Imp_KWord_1::startElement(const XML_Char *name, const XML_Char **atts)
{
  // xml parser keeps running until buffer consumed
  X_EatIfAlreadyError();

  UT_uint32 tokenIndex = _mapNameToToken(name, s_Tokens, TokenTableSize);

  XML_Char *pVal = 0;

  switch (tokenIndex)
  {
    case TT_ATTRIBUTE:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin ATTRIBUTE\n"));
        break;
      }

    case TT_BOTTOMBORDER:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin BOTTOMBORDER\n"));
        break;
      }

    case TT_CHARSET:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin CHARSET\n"));
        break;
      }

    case TT_CLIPARTS:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin CLIPARTS\n"));
        break;
      }

    case TT_COLOR:
      {

        const XML_Char *p = NULL;
	int red, green, blue;
	red = green = blue = 0;
	p = _getXMLPropValue("red", atts);
	if (p != NULL) {
	  red = atoi(p);
	}
	if (p != NULL) {
	  p = _getXMLPropValue("green", atts);
	  green = atoi(p);
	}
	if (p != NULL) {
	  p = _getXMLPropValue("blue", atts);
	  blue = atoi(p);
	}
	
	m_szCharProps += "color:";
	m_szCharProps += UT_String_sprintf("%02x%02x%02x", red, green, blue);
	m_szCharProps += "; ";
	break;

      }

    case TT_COUNTER:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin COUNTER\n"));
        break;
      }

    case TT_DOC:
      {
        //  X_VerifyParseState(_PS_Init);
        m_parseState = _PS_Doc;
        return;
      }
      
    case TT_FOLLOWING:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin FOLLOWING\n"));
        break;
      }
      
    case TT_FRAME:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin FRAME\n"));

	m_szSectProps[m_szSectProps.size() - 2] = 0; // nock off the final ';'

	const XML_Char *propsArray[3];
	propsArray[0] = (XML_Char *)"props";
	propsArray[1] = (XML_Char *)m_szSectProps.c_str();
	propsArray[2] = 0;

        X_CheckError(getDoc()->appendStrux(PTX_Section,(const XML_Char**)propsArray));
	m_szSectProps.clear(); //reset cached properties

	UT_DEBUGMSG(("DOM: CREATED SECTION\n"));

        break;
      }
      
    case TT_FRAMESET:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin FRAMESET\n"));
        break;
      }
      
    case TT_INDENTS:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin INDENTS\n"));
        break;
      }
      
    case TT_LAYOUT:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin LAYOUT\n"));
        break;
      }
      
    case TT_LEFTBORDER:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin LEFTBORDER\n"));
        break;
      }
      
    case TT_LINESPACING:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin LINESPACING\n"));
        break;
      }
    
    case TT_OFFSETS:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin OFFSETS\n"));
        break;
      }
      
    case TT_PAPER:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin PAPER\n"));
	
	const XML_Char * pVal = NULL;

	pVal = _getXMLPropValue("format", atts);
	if(pVal)
	  {
	    // set the page size - we still set width&height below
	    getDoc()->m_docPageSize.Set(kPageToFpPageSize(pVal));
	  }

	pVal = _getXMLPropValue("orientation", atts);
	if(pVal)
	  {
	    if(strcmp(pVal, "1") == 0)
	      {
		// set to landscape
		getDoc()->m_docPageSize.setLandscape();
	      }
	    else
	      {
		// set to portrait
		getDoc()->m_docPageSize.setPortrait();
	      }
	  }

	double page_width, page_height;
	page_width = page_height = 0.0;
	pVal = _getXMLPropValue("width", atts);
	if(pVal)
	  {
	    // get the page width
	    page_width = atof(pVal);
	  }
	
	pVal = _getXMLPropValue("height", atts);
	if(pVal)
	  {
	    // get the page height
	    page_height = atof(pVal);
	  }

	if ( page_height != 0. && page_width != 0. )
	  getDoc()->m_docPageSize.Set(page_width, page_height, DIM_MM);
	break;
      }
      
    case TT_PAPERBORDERS:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin PAPERBORDERS\n"));

	// margins

	const XML_Char * pVal = NULL;

	pVal = _getXMLPropValue("right", atts);
	if(pVal)
	  {
	    // page-margin-right from mm
	    m_szSectProps += "page-margin-right:";
	    m_szSectProps += pVal;
	    m_szSectProps += "mm; ";
	  }

	// todo: really get these
	m_szSectProps += "page-margin-footer:0.0mm; page-margin-header:0.0mm; ";

	pVal = _getXMLPropValue("left", atts);
	if(pVal)
	  {
	    // page-margin-left from mm
	    m_szSectProps += "page-margin-left:";	    
	    m_szSectProps += pVal;
	    m_szSectProps += "mm; ";
	  }

	pVal = _getXMLPropValue("top", atts);
	if(pVal)
	  {
	    // page-margin-top from mm
	    m_szSectProps += "page-margin-top:";
	    m_szSectProps += pVal;
	    m_szSectProps += "mm; ";
	  }

	pVal = _getXMLPropValue("bottom", atts);
	if(pVal)
	  {
	    // page-margin-bottom from mm
	    m_szSectProps += "page-margin-bottom:";
	    m_szSectProps += pVal;
	    m_szSectProps += "mm; ";
	  }

        break;
      }
      
    case TT_PAGEBREAKING:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin PAGEBREAKING\n"));
        break;
      }
      
    case TT_PARAGRAPH:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin PARAGRPAH\n"));

        const XML_Char *pVal = NULL;

        pVal = _getXMLPropValue("value", atts);
        if (pVal)
        {
          m_ParaProps += "text-align:";
          m_ParaProps += numberToJustification(pVal);
          m_ParaProps += "; ";
        }

	m_ParaProps[m_ParaProps.size()-2] = 0;
	
	const XML_Char * props[3];
	props[0] = "props";
	props[1] = m_ParaProps.c_str();
	props[2] = 0;

	// TODO: handle more properties
        X_CheckError(getDoc()->appendStrux(PTX_Block, props));

	UT_DEBUGMSG(("DOM: CREATED PARAGRAPH\n"));
        break;
      }
      
    case TT_RIGHTBORDER:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin RIGHTBORDER\n"));
        break;
      }
      
    case TT_STYLE:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin STYLE\n"));
        break;
      }
      
    case TT_STYLES:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin STYLES\n"));
        break;
      }
      
    case TT_TEXT:
      {
        m_bInText = true;
        return;
      }
      
    case TT_TOPBORDER:
      {
        xxx_UT_DEBUGMSG(("ABIDEBUG: begin TOPBORDER\n"));
        break;
      }
      
    case TT_ITALIC:
      {
        pVal = (XML_Char *)_getXMLPropValue("value", atts);
        if (pVal && UT_strcmp(pVal, "1") == 0 )
        m_szCharProps += "font-style:italic; ";
        break;
      }
      
    case TT_UNDERLINE:
      {
        pVal = (XML_Char *)_getXMLPropValue("value", atts);
        if (pVal && UT_strcmp(pVal, "1") == 0 )
        m_szCharProps += "text-decoration:underline; ";
        break;
      }
      
    case TT_WEIGHT:
      {
        pVal = (XML_Char *)_getXMLPropValue("value", atts);
        if ( pVal && UT_strcmp ( pVal, "75" ) == 0 )
        m_szCharProps += "font-weight:bold; ";
        break;
      }
      
    case TT_STRIKEOUT:
      {
        pVal = (XML_Char *)_getXMLPropValue("value", atts);
        if (pVal && UT_strcmp(pVal, "1") == 0 )
        m_szCharProps += "text-decoration:strike-through; ";
        break;
      }
      
    case TT_FONT:
      {
        pVal = (XML_Char *)_getXMLPropValue("name", atts);
        if (pVal)
        {
          m_szCharProps += "font-face:";
          m_szCharProps += pVal;
          m_szCharProps += "; ";
        }
        break;
      }
      
    case TT_SIZE:
      {
        pVal = (XML_Char *)_getXMLPropValue("value", atts);
        if (pVal)
        {
          m_szCharProps += "font-size:";
          m_szCharProps += pVal;
          m_szCharProps += "; ";
        }
        break;
      }
      
    case TT_FLOW:
    case TT_FORMAT:
    case TT_NAME:
    case TT_VERTALIGN:
    default:
      xxx_UT_DEBUGMSG(("ABIDEBUG: work in progress\n"));
      break;
    }
  
}

void IE_Imp_KWord_1::_appendText()
{
  if (m_szTextBuffer.size())
    {
      if (!getDoc()->appendSpan(m_szTextBuffer.ucs_str(), m_szTextBuffer.size()))
      {
        UT_DEBUGMSG(("Error appending text run\n"));
        return;
      }
      UT_DEBUGMSG(("DOM: APPENDED TEXT\n"));
      m_szTextBuffer.clear();
    }
}

void IE_Imp_KWord_1::endElement(const XML_Char *name)
{
  X_EatIfAlreadyError();

  UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);

  switch (tokenIndex)
  {
    case TT_ATTRIBUTE:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end ATTRIBUTE\n"));
      break;

    case TT_BOTTOMBORDER:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end BOTTOMBORDER\n"));
      break;

    case TT_CHARSET:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end CHARSET\n"));
      break;

    case TT_CLIPARTS:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end CLIPARTS\n"));
      break;

    case TT_COLOR:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end COLOR\n"));
      break;

    case TT_COUNTER:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end COUNTER\n"));
      break;

    case TT_DOC:
    {
      //  X_VerifyParseState(_PS_Init);
      m_parseState = _PS_Doc;
      return;
    }

    case TT_FLOW:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end FLOW\n"));
      break;

    case TT_FOLLOWING:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end FOLLOWING\n"));
      break;

    case TT_FONT:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end FONT\n"));
      break;

    case TT_FORMAT:
    {
      const XML_Char *propsArray[3];

      if (m_szCharProps.size() == 0)
        {
          xxx_UT_DEBUGMSG(("ABIDEBUG: no properties\n"));
          _appendText ();
          break;
        }

      m_szCharProps[m_szCharProps.size() - 2] = 0; // nock off the final ';'

      propsArray[0] = (XML_Char *)"props";
      propsArray[1] = (XML_Char *)m_szCharProps.c_str();
      propsArray[2] = 0;

      xxx_UT_DEBUGMSG(("ABIDEBUG: formatting properties are: %s\n",propsArray[1]));

      X_CheckError(_pushInlineFmt(propsArray));
      X_CheckError(getDoc()->appendFmt(&m_vecInlineFmt));

      UT_DEBUGMSG(("DOM: APPENDED FORMAT\n"));

      m_szCharProps.clear();
      _appendText();

      _popInlineFmt();
      X_CheckError(getDoc()->appendFmt(&m_vecInlineFmt));

      break;
    }

    case TT_FORMATS:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end FORMATS\n"));
      break;

    case TT_FRAME:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end FRAME\n"));
      break;

    case TT_FRAMESET:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end FRAMESET\n"));
      break;

    case TT_INDENTS:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end INDENTS\n"));
      break;

    case TT_ITALIC:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end ITALIC\n"));
      break;

    case TT_LAYOUT:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end LAYOUT\n"));
      break;

    case TT_LEFTBORDER:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end LEFTBORDER\n"));
      break;

    case TT_LINESPACING:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end LINESPACING\n"));
      break;

   case TT_NAME:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end NAME\n"));
      break;

    case TT_OFFSETS:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end OFFSETS\n"));
      break;

    case TT_PAPER:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end PAPER\n"));
      break;

    case TT_PAPERBORDERS:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end PAPERBORDERS\n"));
      break;

    case TT_PAGEBREAKING:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end PAGEBREAKING\n"));
      break;

    case TT_PARAGRAPH:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end PARAGRPAH\n"));
      break;

    case TT_RIGHTBORDER:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end RIGHTBORDER\n"));
      break;

    case TT_SIZE:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end SIZE\n"));
      break;

    case TT_STRIKEOUT:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end STRIKEOUT\n"));
      break;

    case TT_STYLE:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end STYLE\n"));
      break;

    case TT_STYLES:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end STYLES\n"));
      break;

    case TT_TEXT:
      m_bInText = false;
      return;

    case TT_TOPBORDER:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end TOPBORDER\n"));
      break;

    case TT_UNDERLINE:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end UNDERLINE\n"));
      break;

    case TT_VERTALIGN:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end VERTALIGN\n"));
      break;

    case TT_WEIGHT:
      xxx_UT_DEBUGMSG(("ABIDEBUG: end WEIGHT\n"));
      break;

  }

}
