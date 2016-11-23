/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_locale.h"
#include "ut_std_string.h"
#include "ut_string.h"
#include "ie_impexp_XSL-FO.h"
#include "ie_imp_XSL-FO.h"
#include "ie_types.h"
#include "ie_impGraphic.h"
#include "pd_Document.h"
#include "fg_GraphicRaster.h"
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

IE_Imp_XSL_FO_Sniffer::IE_Imp_XSL_FO_Sniffer (const char * _name) :
  IE_ImpSniffer(_name)
{
  // 
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_XSL_FO_Sniffer__SuffixConfidence[] = {
	{ "fo", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_XSL_FO_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_XSL_FO_Sniffer__SuffixConfidence;
}

UT_Confidence_t IE_Imp_XSL_FO_Sniffer::recognizeContents(const char * szBuf, 
											  UT_uint32 iNumbytes)
{
	UT_uint32 iLinesToRead = 6;
	UT_uint32 iBytesScanned = 0;

	const char *p;
	const char * magic;

	p = szBuf;

	while ( iLinesToRead-- )
	{
		magic = "<fo:root ";
		if ( (iNumbytes - iBytesScanned) < strlen(magic) ) return(UT_CONFIDENCE_ZILCH);
		if ( strncmp(p, magic, strlen(magic)) == 0 ) return(UT_CONFIDENCE_PERFECT);
		/*  Seek to the next newline:  */
		while ( *p != '\n' && *p != '\r' )
		{
			iBytesScanned++; p++;
			if( iBytesScanned+2 >= iNumbytes ) return(UT_CONFIDENCE_ZILCH);
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

  return UT_CONFIDENCE_ZILCH;
}

UT_Error IE_Imp_XSL_FO_Sniffer::constructImporter(PD_Document * pDocument,
												  IE_Imp ** ppie)
{
	IE_Imp_XSL_FO * p = new IE_Imp_XSL_FO(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_XSL_FO_Sniffer::getDlgLabels(const char ** pszDesc,
											const char ** pszSuffixList,
											IEFileType * ft)
{
	*pszDesc = "XSL-FO (.fo)";
	*pszSuffixList = "*.fo";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_XSL_FO::~IE_Imp_XSL_FO()
{
	DELETEP(m_TableHelperStack);
}

IE_Imp_XSL_FO::IE_Imp_XSL_FO(PD_Document * pDocument)
	: IE_Imp_XML(pDocument, false),
	  m_iBlockDepth(0),
	  m_iListDepth(0),
	  m_iListBlockDepth(0),
	  m_iTableDepth(0),
	  m_iFootnotes(0),
	  m_iImages(0),
	  m_bOpenedLink(false),
	  m_bPendingFootnote(false),
	  m_bInFootnote(false),
	  m_bIgnoreFootnoteBlock(false),
	  m_TableHelperStack(new IE_Imp_TableHelperStack())
{
}

/*****************************************************************/
/*****************************************************************/

static struct xmlToIdMapping s_Tokens[] =
{
	{	"fo:basic-link",			TT_BASICLINK			},
	{	"fo:block",					TT_BLOCK				},
	{	"fo:character",				TT_CHAR					},
	{	"fo:external-graphic",		TT_IMAGE				},
	{	"fo:flow",					TT_SECTION				},
	{	"fo:footnote",				TT_FOOTNOTE				},
	{	"fo:footnote-body",			TT_FOOTNOTEBODY			},
	{	"fo:inline",				TT_INLINE				},
	{	"fo:layout-master-set",		TT_LAYOUT_MASTER_SET	},
	{	"fo:list",					TT_LIST					},
	{	"fo:list-block",			TT_LISTBLOCK			},
	{	"fo:list-item",				TT_LISTITEM				},
	{	"fo:list-item-body",		TT_LISTITEMBODY			},
	{	"fo:list-item-label",		TT_LISTITEMLABEL		},
	{	"fo:page-sequence",			TT_PAGE_SEQUENCE		},
	{	"fo:region-body",			TT_REGION_BODY			},
	{	"fo:root",					TT_DOCUMENT				},
	{	"fo:simple-page-master",	TT_SIMPLE_PAGE_MASTER	},
	{	"fo:static-content",		TT_STATIC				},
	{	"fo:table",					TT_TABLE				},
	{	"fo:table-body",			TT_TABLEBODY			},
	{	"fo:table-cell",			TT_TABLECELL			},
	{	"fo:table-column",			TT_TABLECOLUMN			},
	{	"fo:table-row",				TT_TABLEROW				},
};

#define TokenTableSize	((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

/*****************************************************************/	
/*****************************************************************/	

#define X_TestParseState(ps)	((m_parseState==(ps)))

#define X_VerifyParseState(ps)	do {  if (!(X_TestParseState(ps)))			\
									  {  m_error = UT_IE_BOGUSDOCUMENT;	\
										 UT_DEBUGMSG(("XSL-FO: X_VerifyParseState failed: %s\n", #ps)); \
										 return; } } while (0)

#define X_CheckDocument(b)		do {  if (!(b))								\
									  {  m_error = UT_IE_BOGUSDOCUMENT;	\
										 UT_DEBUGMSG(("XSL-FO: X_CheckDocument failed: %s\n", #b)); \
										 return; } } while (0)

#define X_CheckError(v)			do {  if (!(v))								\
									  {  m_error = UT_ERROR;			\
										 UT_DEBUGMSG(("XSL-FO: X_CheckError failed: %s\n", #v)); \
										 return; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_error) return; } while (0)

/*****************************************************************/
/*****************************************************************/

#define USED() do {if(used) sBuf+="; "; else used = true;} while (0)

void IE_Imp_XSL_FO::startElement(const gchar *name,
								  const gchar **atts)
{
	UT_DEBUGMSG(("XSL-FO import: startElement: %s\n", name));

	// xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();
	
	UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);
	m_utnsTagStack.push(tokenIndex);

    const gchar * buf[3];
    const gchar ** p_atts;
	buf[0] = static_cast<const gchar *>("props");
	buf[2] = NULL;
	
	UT_UTF8String sBuf;
	const gchar * pVal = NULL;
	
	bool used = false;

	switch (tokenIndex)
	{
		case TT_DOCUMENT:
		{
			X_VerifyParseState(_PS_Init);
			m_parseState = _PS_Doc;
			break;
		}

		case TT_SECTION:
		{
			X_VerifyParseState(_PS_Doc);
			m_parseState = _PS_Sec;
			X_CheckError(appendStrux(PTX_Section, PP_NOPROPS));
			break;
		}

		case TT_STATIC: // TODO: turn these into headers and footers, not generic sections
		{
			X_VerifyParseState(_PS_Doc);
			m_parseState = _PS_Sec;
			X_CheckError(appendStrux(PTX_Section, PP_NOPROPS));
			break;
		}

		case TT_BLOCK:
		{
			X_CheckError((m_parseState == _PS_Sec) || (m_parseState == _PS_Block) || (m_parseState == _PS_List)); //blocks can be nested

			m_iBlockDepth++; // leave this above the footnote check to prevent parse errors

			if(m_bIgnoreFootnoteBlock)
				break; // we want to ignore the first <fo:block> in <fo:footnote-body> because it appends an unwanted "break" before the footnote text

			m_parseState = _PS_Block;

			pVal = static_cast<const gchar*>(_getXMLPropValue("background-color", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "bgcolor:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("color", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "color:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("language", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "lang:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("font-size", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "font-size:";
				sBuf += static_cast<const char *>(pVal);
			}
			
			pVal = static_cast<const gchar*>(_getXMLPropValue("font-family", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "font-family:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("font-weight", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "font-weight:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("font-style", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "font-style:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("font-stretch", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "font-stretch:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("keep-together", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "keep-together:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("keep-with-next", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "keep-with-next:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("line-height", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "line-height:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("margin-bottom", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "margin-bottom:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("margin-top", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "margin-top:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("margin-left", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "margin-left:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("margin-right", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "margin-right:";
				sBuf += static_cast<const char *>(pVal);
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("text-align", atts));
			if (pVal && *pVal)
			{
				if (!strcmp("left", pVal) || !strcmp("right", pVal) || !strcmp("center", pVal) || !strcmp("justify", pVal))
				{
					USED();
					sBuf += "text-align:";
					sBuf += static_cast<const char *>(pVal);
				}
				else if (!strcmp("start", pVal))
				{
					USED();
					sBuf += "text-align:left";
				}
				else if (!strcmp("end", pVal))
				{
					USED();
					sBuf += "text-align:right";
				}
				// "outside" and "inside" are not handled
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("widows", atts));
			if (pVal && *pVal)
			{
				USED();
				sBuf += "widows:";
				sBuf += static_cast<const char *>(pVal);
			}

			if(sBuf.length())
				buf[1] = sBuf.utf8_str();
			else
				buf[0] = NULL;

			xxx_UT_DEBUGMSG(("FO import: block props='%s'\n", sBuf.utf8_str()));

			// append the atts/block to the document; if we're in a table, let
			// charData() handle it instead (see Bug 10566)
			if(m_iTableDepth == 0)
				X_CheckError(appendStrux(PTX_Block, PP_std_copyProps(buf)));

			break;
		}

		case TT_FOOTNOTE:
		{
			X_CheckError((m_parseState == _PS_Sec) || (m_parseState == _PS_Block) || (m_parseState == _PS_List));
			UT_return_if_fail(m_bPendingFootnote == false);

			std::string id = UT_std_string_sprintf("%d", ++m_iFootnotes);

			const PP_PropertyVector fnbuf = {
				PT_TYPE_ATTRIBUTE_NAME, "footnote_ref",
				"footnote-id", id,
				PT_PROPS_ATTRIBUTE_NAME, "text-position:superscript"
			};
			X_CheckError(appendObject(PTO_Field,fnbuf));

			m_bPendingFootnote = true;

			break;
		}

		case TT_FOOTNOTEBODY:
		{
			X_CheckError((m_parseState == _PS_Sec) || (m_parseState == _PS_Block) || (m_parseState == _PS_List));
			X_CheckError(m_bPendingFootnote);
			m_bPendingFootnote = false;

			std::string id = UT_std_string_sprintf("%d", m_iFootnotes);

			const PP_PropertyVector notebuf = {
				"footnote-id", id
			};
			X_CheckError(appendStrux(PTX_SectionFootnote, notebuf));

			const PP_PropertyVector anchorbuf = {
				PT_TYPE_ATTRIBUTE_NAME, "footnote_anchor",
				"footnote-id", id,
				PT_PROPS_ATTRIBUTE_NAME, "text-position:superscript"
			};
			X_CheckError(appendObject(PTO_Field,anchorbuf));

			m_bInFootnote = true;
			m_bIgnoreFootnoteBlock = true;

			break;
		}

			// we treat both of these as if they were the same
			// they represent character-level formatting
		case TT_CHAR:
		case TT_INLINE:
		{
			X_VerifyParseState(_PS_Block);
			if(m_bPendingFootnote)
				break;

			{
				pVal = static_cast<const gchar*>(_getXMLPropValue("background-color", atts));
				if (pVal && *pVal)
				{
					USED();
					sBuf += "bgcolor:";
					sBuf += static_cast<const char *>(pVal);
				}

				pVal = static_cast<const gchar*>(_getXMLPropValue("color", atts));
				if (pVal && *pVal)
				{
					USED();
					sBuf += "color:";
					sBuf += static_cast<const char *>(pVal);
				}

				pVal = static_cast<const gchar*>(_getXMLPropValue("language", atts));
				if (pVal && *pVal)
				{
					USED();
					sBuf += "lang:";
					sBuf += static_cast<const char *>(pVal);
				}

				pVal = static_cast<const gchar*>(_getXMLPropValue("font-size", atts));
				if (pVal && *pVal)
				{
					USED();
					sBuf += "font-size:";
					sBuf += static_cast<const char *>(pVal);
				}
				
				pVal = static_cast<const gchar*>(_getXMLPropValue("font-family", atts));
				if (pVal && *pVal)
				{
					USED();
					sBuf += "font-family:";
					sBuf += static_cast<const char *>(pVal);
				}

				pVal = static_cast<const gchar*>(_getXMLPropValue("font-weight", atts));
				if (pVal && *pVal)
				{
					USED();
					sBuf += "font-weight:";
					sBuf += static_cast<const char *>(pVal);
				}

				pVal = static_cast<const gchar*>(_getXMLPropValue("font-style", atts));
				if (pVal && *pVal)
				{
					USED();
					sBuf += "font-style:";
					sBuf += static_cast<const char *>(pVal);
				}

				pVal = static_cast<const gchar*>(_getXMLPropValue("font-stretch", atts));
				if (pVal && *pVal)
				{
					USED();
					sBuf += "font-stretch:";
					sBuf += static_cast<const char *>(pVal);
				}

				pVal = static_cast<const gchar*>(_getXMLPropValue("keep-together", atts));
				if (pVal && *pVal)
				{
					USED();
					sBuf += "keep-together:";
					sBuf += static_cast<const char *>(pVal);
				}

				pVal = static_cast<const gchar*>(_getXMLPropValue("keep-with-next", atts));
				if (pVal && *pVal)
				{
					USED();
					sBuf += "keep-with-next:";
					sBuf += static_cast<const char *>(pVal);
				}

				pVal = static_cast<const gchar*>(_getXMLPropValue("text-decoration", atts));
				if (pVal && *pVal)
				{
					USED();
					sBuf += "text-decoration:";
					sBuf += static_cast<const char *>(pVal);
				}

				pVal = _getXMLPropValue("text-transform", atts);
				if (pVal && *pVal &&
				   ((strcmp(pVal, "none") == 0) ||
				    (strcmp(pVal, "capitalize") == 0) ||
				    (strcmp(pVal, "uppercase") == 0) ||
				    (strcmp(pVal, "lowercase") == 0)))
				{
					USED();
					sBuf += "text-transform:";
					sBuf += pVal;
				}


				buf[1] = sBuf.utf8_str();

				xxx_UT_DEBUGMSG(("FO import: inline props='%s'\n", sBuf.utf8_str()));

				p_atts = &buf[0];
				X_CheckError(_pushInlineFmt(PP_std_copyProps(p_atts)));
				X_CheckError(appendFmt(m_vecInlineFmt));

				pVal = static_cast<const gchar*>(_getXMLPropValue("id", atts));
				if(pVal)
				{
					PP_PropertyVector buf2 = {
						PT_TYPE_ATTRIBUTE_NAME, "start",
						PT_NAME_ATTRIBUTE_NAME, pVal
					};
					X_CheckError(appendObject(PTO_Bookmark, buf2));
					buf2[1] = "end";
					X_CheckError(appendObject(PTO_Bookmark, buf2));
				}
			}
			break;
		}

		case TT_BASICLINK:
		{
			if(m_bOpenedLink)
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				break;
			}

			gchar *p_val = NULL;
			p_val = (gchar *)_getXMLPropValue("internal-destination", atts);

			if(p_val) //internal
			{
				std::string link = "#";
				// It looks like the spec allows for an 'empty string', so we will too
				if(*p_val)
					link += p_val;

				const PP_PropertyVector linkbuf =  {
					"xlink:href", link
				};
				X_CheckError(appendObject(PTO_Hyperlink, linkbuf));
				m_bOpenedLink = true;
				break;
			}

			p_val = (gchar *)_getXMLPropValue("external-destination", atts);

			if(p_val) //external
			{
				UT_UTF8String link = p_val;
				int len = strlen(p_val);
				if((len > 7) && (link.substr(0,5) == "url('") && (link.substr(len-2, 2) == "')"))
				{
					p_val[len - 2] = '\0';
					p_val = p_val + 5;

					const PP_PropertyVector linkbuf =  {
						"xlink:href", p_val
					};
					X_CheckError(appendObject(PTO_Hyperlink, linkbuf));
					m_bOpenedLink = true;
				}
				else
				{
					UT_DEBUGMSG(("XSL-FO import: invalid external-destination attribute: [%s]\n", p_val));
				}
			}

			break;
		}

		//Lists:

		case TT_LIST:
		{
			X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_List) || (m_parseState == _PS_Sec));

			m_parseState = _PS_ListSec;
			m_iListDepth++;

			break;
		}

		case TT_LISTBLOCK:
		{
			X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_List) || (m_parseState == _PS_ListSec) || (m_parseState == _PS_Sec));

			m_parseState = _PS_List;
			m_iListBlockDepth++;

			break;
		}

		case TT_LISTITEM:
		{
			X_VerifyParseState(_PS_List);
			break;
		}

		case TT_LISTITEMLABEL:
		{
			X_VerifyParseState(_PS_List);
			break;
		}

		case TT_LISTITEMBODY:
		{
			X_VerifyParseState(_PS_List);
			break;
		}


		//Tables:

		case TT_TABLE:
		{
			X_CheckError((m_parseState == _PS_Sec) || (m_parseState == _PS_Block) || (m_parseState == _PS_List));
			X_CheckError(m_TableHelperStack->tableStart(getDoc(),NULL));
			m_iTableDepth++;

			m_parseState = _PS_Table;
			break;
		}

		case TT_TABLEROW:
		{
			X_VerifyParseState(_PS_Table);
			X_CheckError(m_TableHelperStack->trStart(NULL));
			break;
		}

		case TT_TABLEBODY:
		case TT_TABLECOLUMN:
		{
			X_VerifyParseState(_PS_Table);
			break;
		}

		case TT_TABLECELL:
		{
			X_VerifyParseState(_PS_Table);
			m_parseState = _PS_Block;

			UT_sint32 rowspan = 1, colspan = 1;

			pVal = static_cast<const gchar*>(_getXMLPropValue("number-columns-spanned", atts));
			if(pVal)
			{
				colspan = atoi(pVal);
				if(colspan < 1)
					colspan = 1;
			}

			pVal = static_cast<const gchar*>(_getXMLPropValue("number-rows-spanned", atts));
			if(pVal)
			{
				rowspan = atoi(pVal);
				if(rowspan < 1)
					rowspan = 1;
			}


			X_CheckError(m_TableHelperStack->tdStart(rowspan, colspan, NULL));
			break;
		}

			// here we set the page size
		case TT_SIMPLE_PAGE_MASTER:
		{
			X_VerifyParseState(_PS_Doc);
			{
				// TODO: we should do some cool stuff based on these prop=val keys:
				// margin-top, margin-bottom, margin-left, margin-right,
				// page-width, page-height
			}
			break;
		}

		case TT_IMAGE:
		{
			X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_List) || (m_parseState == _PS_Sec));

			gchar *p_val = NULL;
			p_val = (gchar *)_getXMLPropValue(static_cast<const gchar *>("src"), atts);

			if(p_val)
			{
				UT_UTF8String src = p_val;
				int len = strlen(p_val);
				if((len > 7) && (src.substr(0,5) == "url('") && (src.substr(len-2, 2) == "')"))
				{
					p_val[len - 2] = '\0';
					p_val = p_val + 5;
					createImage(p_val, atts);
				}
				else
				{
					UT_DEBUGMSG(("XSL-FO import: invalid image src attribute: [%s]\n", p_val));
				}
			}

			break;
		}

			// these we just plain ignore
		case TT_LAYOUT_MASTER_SET:
		case TT_REGION_BODY:
		case TT_PAGE_SEQUENCE:
		{
			break;
		}

		default:
		{
			UT_DEBUGMSG(("Unknown or knowingly unhandled tag [%s]\n",name));
			break;
		}
	}

}

#undef USED

void IE_Imp_XSL_FO::endElement(const gchar *name)
{
	UT_DEBUGMSG(("XSL-FO import: endElement: %s\n", name));

	// xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();
	
   	UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize), i = 0;
	m_utnsTagStack.pop((UT_sint32*)&i);

	if(i != tokenIndex)
	{
		UT_DEBUGMSG(("XSL-FO: Parse error!\n"));
	}

	switch (tokenIndex)
	{
		case TT_DOCUMENT:
		{
			X_VerifyParseState(_PS_Doc);
			m_parseState = _PS_Init;
			break;
		}

		case TT_SECTION:
		{
			X_VerifyParseState(_PS_Sec);
			m_parseState = _PS_Doc;
			break;
		}

		case TT_STATIC:
		{
			X_VerifyParseState(_PS_Sec);
			m_parseState = _PS_Doc;
			break;
		}

		case TT_BLOCK:
		{
			UT_ASSERT_HARMLESS(m_lenCharDataSeen == 0);
			m_iBlockDepth--; // keep this above the footnote block check

			if(m_bIgnoreFootnoteBlock) //we only want to ignore the first block, not all
			{
				m_bIgnoreFootnoteBlock = false;
				break;
			}

			X_VerifyParseState( _PS_Block);

			if(_isInListTag())
				m_parseState = _PS_List;
			else if(m_iTableDepth)
				m_parseState = _PS_Block;
			else if(m_iBlockDepth == 0)
				m_parseState = _PS_Sec;

			X_CheckDocument(_getInlineDepth() == 0);

			break;
		}

		case TT_FOOTNOTE:
		{
			X_CheckError((m_parseState == _PS_Sec) || (m_parseState == _PS_Block) || (m_parseState == _PS_List));
			X_CheckError(!m_bInFootnote);
			break;
		}

		case TT_FOOTNOTEBODY:
		{
			X_CheckError((m_parseState == _PS_Sec) || (m_parseState == _PS_Block) || (m_parseState == _PS_List));
			X_CheckError(m_bInFootnote);
			X_CheckError(appendStrux(PTX_EndFootnote, PP_NOPROPS));

			if(m_bIgnoreFootnoteBlock) //there was no block in <fo:footnote-body> (invalid file?)
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				m_bIgnoreFootnoteBlock = false;
			}

			if(_isInListTag())
				m_parseState = _PS_List;
			else if(m_iBlockDepth)
				m_parseState = _PS_Block;
			else if(m_iBlockDepth == 0)
				m_parseState = _PS_Sec;

			m_bInFootnote = false;

			break;
		}

		case TT_INLINE:
		case TT_CHAR:
		{
			UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);
			X_VerifyParseState(_PS_Block);

			if(m_bPendingFootnote)
				break;

			X_CheckDocument(_getInlineDepth() > 0);
			_popInlineFmt();
			X_CheckError(appendFmt(m_vecInlineFmt));
			break;
		}

		case TT_BASICLINK:
		{
			X_VerifyParseState(_PS_Block);

			if(m_bOpenedLink)
				X_CheckError(appendObject(PTO_Hyperlink, PP_NOPROPS));

			m_bOpenedLink = false;
			break;
		}

		//Lists:

		case TT_LIST:
		{
			X_VerifyParseState(_PS_ListSec);

			m_iListDepth--;

			if(m_iBlockDepth)
				m_parseState = _PS_Block;
			else if(_isInListTag())
				m_parseState = _PS_List;

			break;
		}

		case TT_LISTBLOCK:
		{
			X_VerifyParseState(_PS_List);
			m_iListBlockDepth--;

			if(_isInListTag())
				m_parseState = _PS_List;
			else if((m_iListBlockDepth == 0) && (m_iListDepth > 0))
				m_parseState = _PS_ListSec;
			else if((m_iBlockDepth > 0) || m_iTableDepth)
				m_parseState = _PS_Block;
			else if((m_iBlockDepth == 0) && (m_iListDepth == 0))
				m_parseState = _PS_Sec;

			break;
		}

		case TT_LISTITEM:
		{
			X_VerifyParseState(_PS_List);
			break;
		}

		case TT_LISTITEMLABEL:
		{
			X_VerifyParseState(_PS_List);
			break;
		}

		case TT_LISTITEMBODY:
		{
			X_VerifyParseState(_PS_List);
			break;
		}


		//Tables:

		case TT_TABLE:
		{
			X_VerifyParseState(_PS_Table);
			m_iTableDepth--;

			if(_isInListTag())
				m_parseState = _PS_List;
			else if(m_iBlockDepth > 0)
				m_parseState = _PS_Block;
			else
				m_parseState = _PS_Sec;

			X_CheckError(m_TableHelperStack->tableEnd());
			break;
		}

		case TT_TABLEBODY:
		case TT_TABLEROW:
		{
			X_VerifyParseState(_PS_Table);
			break;
		}

		case TT_TABLECOLUMN:
		{
			X_VerifyParseState(_PS_Table);
			break;
		}

		case TT_TABLECELL:
		{
			X_VerifyParseState(_PS_Block);
			m_parseState = _PS_Table;

			X_CheckError(m_TableHelperStack->tdEnd());
			break;
		}

		case TT_IMAGE:
		{
			X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_List) || (m_parseState == _PS_Sec));
			break;
		}

		case TT_OTHER:
		case TT_LAYOUT_MASTER_SET:
		case TT_SIMPLE_PAGE_MASTER:
		case TT_REGION_BODY:
		case TT_PAGE_SEQUENCE:
		{
			break;
		}

		default:
		{
			UT_DEBUGMSG(("XSL-FO: Unknown or intentionally unhandled end tag [%s]\n",name));
			break;
		}
	}

}

bool IE_Imp_XSL_FO::_isInListTag(void)
{
	return ((_tagTop() == TT_LISTBLOCK) || (_tagTop() == TT_LISTITEM) || (_tagTop() == TT_LISTITEMLABEL) || (_tagTop() == TT_LISTITEMBODY));
}

UT_uint32 IE_Imp_XSL_FO::_tagTop(void)
{
	UT_sint32 i = 0;

	if (m_utnsTagStack.viewTop (i))
		return (UT_uint32)i;
	return 0;
}

void IE_Imp_XSL_FO::createImage(const char *name, const gchar **atts)
{
	UT_return_if_fail(name && *name && m_szFileName && *m_szFileName);

	char * relative_file = UT_go_url_resolve_relative(m_szFileName, name);
	if(!relative_file)
		return;

	UT_UTF8String filename(relative_file);
	g_free(relative_file);

	FG_ConstGraphicPtr pfg;
	if (IE_ImpGraphic::loadGraphic (filename.utf8_str(), IEGFT_Unknown, pfg) != UT_OK)
		return;

	const UT_ConstByteBufPtr & pBB = pfg->getBuffer();
	X_CheckError(pBB);

	UT_UTF8String dataid;
	UT_UTF8String_sprintf (dataid, "image%u", static_cast<unsigned int>(m_iImages++));

	X_CheckError (getDoc()->createDataItem (dataid.utf8_str(), false, pBB,
                                            pfg->getMimeType(), NULL));

	UT_UTF8String props, dim;
	const gchar *p_val = NULL;

	UT_LocaleTransactor t(LC_NUMERIC, "C");

	p_val = _getXMLPropValue(static_cast<const gchar *>("content-height"), atts);

	if(p_val)
	{
		props = "height:";
		dim = UT_UTF8String_sprintf("%fin", UT_convertDimToInches(UT_convertDimensionless(p_val), UT_determineDimension(p_val,DIM_PX)));
		props+= dim.utf8_str();
		dim.clear();
	}

	p_val = _getXMLPropValue(static_cast<const gchar *>("content-width"), atts);

	if(p_val)
	{
		if(props.length()) //the image might not have a content-height attribute
			props+= "; ";

		props+= "width:";
		dim = UT_UTF8String_sprintf("%fin", UT_convertDimToInches(UT_convertDimensionless(p_val), UT_determineDimension(p_val,DIM_PX)));
		props+= dim.utf8_str();
	}

	PP_PropertyVector attr = {
		"dataid", dataid.utf8_str()
	};
	if(props.length())
	{
		attr.push_back(PT_PROPS_ATTRIBUTE_NAME);
		attr.push_back(props.utf8_str());
	}

	X_CheckError(appendObject(PTO_Image, attr));
}

void IE_Imp_XSL_FO::charData(const gchar *s, int len)
{
	if(m_bPendingFootnote) //ignore the character data between <fo:footnote> and <fo:footnote-body>
		return;

	if(m_iTableDepth && (m_parseState != _PS_Table))
	{
		UT_UCS4String span = s;

		if(strcmp(span.utf8_str(), "\n") != 0) //don't insert linefeeds
			m_TableHelperStack->Inline(span.ucs4_str(), span.length());

		return;
	}

	IE_Imp_XML :: charData (s, len);
}
