/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 2001-2002 Dom Lachowicz
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
#include "ut_string.h"
#include "ie_imp_WML.h"
#include "ie_impGraphic.h"
#include "ie_types.h"
#include "pd_Document.h"
#include "ut_growbuf.h"
#include "ut_path.h"
#include "ut_string_class.h"
#include "fg_GraphicRaster.h"

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

IE_Imp_WML_Sniffer::IE_Imp_WML_Sniffer (const char * _name) :
  IE_ImpSniffer(_name)
{
  // 
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_WML_Sniffer__SuffixConfidence[] = {
	{ "wml", 	UT_CONFIDENCE_PERFECT 	},
	{ "",   	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_WML_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_WML_Sniffer__SuffixConfidence;
}

const IE_MimeConfidence * IE_Imp_WML_Sniffer::getMimeConfidence () 
{
	// mimetypes once getMimeConfidence is implemented (need to check correctness)
	// "text/vnd.wap.wml"
	return NULL; 
}

UT_Confidence_t IE_Imp_WML_Sniffer::recognizeContents(const char * szBuf, 
										   UT_uint32 /*iNumbytes*/)
{
	// TODO: scan the first few lines

	if(strstr(szBuf, "!DOCTYPE wml PUBLIC") == NULL)
		return UT_CONFIDENCE_ZILCH;

	return UT_CONFIDENCE_PERFECT;
}

UT_Error IE_Imp_WML_Sniffer::constructImporter(PD_Document * pDocument,
											   IE_Imp ** ppie)
{
	IE_Imp_WML * p = new IE_Imp_WML(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Imp_WML_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "WML (.wml)";
	*pszSuffixList = "*.wml";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_WML::~IE_Imp_WML()
{
	DELETEP(m_TableHelperStack);
}

IE_Imp_WML::IE_Imp_WML (PD_Document * pDocument) :
	IE_Imp_XML(pDocument,false),
	m_bOpenedBlock(false),
	m_bOpenedSection(false),
	m_iColumns(0),
	m_iImages(0),
	m_iOpenedColumns(0),
	m_TableHelperStack(new IE_Imp_TableHelperStack())
{
}

/*****************************************************************/
/*****************************************************************/

#define TT_OTHER		0	// anything else
#define TT_DOCUMENT		1	// a document <wml>
#define TT_SECTION		2	// card or section <card>
#define TT_BLOCK		3	// a paragraph <p>
#define TT_IMAGE		4	// an image object <img>
#define TT_BREAK		5	// a forced line-break <br/>
#define TT_BOLD			6	// bold text <b>
#define TT_ITALIC		7	// italic text <i>
#define TT_UNDERLINE	8	// underlined text <u>
#define TT_STRONG		9	// strong(bold) text <strong>
#define TT_EMPHASIS		10	// emphasis(bold) text <em>
#define TT_BIG			11	// big(superscript) text <big>
#define TT_SMALL		12	// small(subscript) text <small>
#define TT_TABLE		13	// <table>
#define TT_TABLE_ROW	14	// <tr>
#define TT_TABLE_CELL	15	// <td>
#define TT_ACCESS		16	// <access>
#define TT_HEAD			17	// <head>
#define TT_META			18	// <meta>
#define TT_TEMPLATE		19	// <template>
#define TT_DO			20	// <do>
#define TT_ONEVENT		21	// <onevent>
#define TT_POSTFIELD	22	// <postfield>
#define TT_GO			23	// <go>
#define TT_NOOP			24	// <noop>
#define TT_PREV			25	// <prev>
#define TT_REFRESH		26	// <refresh>
#define TT_FIELDSET		27	// <fieldset>
#define TT_INPUT		28	// <input>
#define TT_OPTGROUP		29	// <optgroup>
#define TT_OPTION		30	// <option>
#define TT_SELECT		31	// <select>
#define TT_SETVAR		32	// <setvar>
#define TT_TIMER		33	// <timer>
#define TT_ANCHOR		34	// <anchor>
#define TT_LINK			35	// <a>

// KEEP IN ALPHABETICAL ORDER!!

static struct xmlToIdMapping s_Tokens[] =
{
	{	"a",			TT_LINK			},
	{	"access",		TT_ACCESS		},
	{	"anchor",		TT_ANCHOR		},
	{	"b",			TT_BOLD			},
	{	"big",			TT_BIG			},
	{	"br",			TT_BREAK		},
	{	"card",			TT_SECTION		},
	{	"do",			TT_DO			},
	{	"em",			TT_EMPHASIS		},
	{	"fieldset",		TT_FIELDSET		},
	{	"go",			TT_GO			},
	{	"head",			TT_HEAD			},
	{	"i",			TT_ITALIC		},
	{	"img",			TT_IMAGE		},
	{	"input",		TT_INPUT		},
	{	"meta",			TT_META			},
	{	"noop",			TT_NOOP			},
	{	"onevent",		TT_ONEVENT		},
	{	"optgroup",		TT_OPTGROUP		},
	{	"option",		TT_OPTION		},
	{	"p",			TT_BLOCK		},
	{	"postfield",	TT_POSTFIELD	},
	{	"prev",			TT_PREV			},
	{	"refresh",		TT_REFRESH		},
	{	"select",		TT_SELECT		},
	{	"setvar",		TT_SETVAR		},
	{	"small",		TT_SMALL		},
	{	"strong",		TT_STRONG		},
	{	"table",		TT_TABLE		},
	{	"td",			TT_TABLE_CELL	},
	{	"template",		TT_TEMPLATE		},
	{	"timer",		TT_TIMER		},
	{	"tr",			TT_TABLE_ROW	},
	{	"u",			TT_UNDERLINE	},
	{	"wml",			TT_DOCUMENT		}
};

#define TokenTableSize	((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

/*****************************************************************/	
/*****************************************************************/	

#define X_TestParseState(ps)	((m_parseState==(ps)))

#define X_VerifyParseState(ps)	do {  if (!(X_TestParseState(ps)))			\
									  {  m_error = UT_IE_BOGUSDOCUMENT;	\
									     UT_DEBUGMSG(("WML import: X_TestParseState() failed %s\n", #ps)); \
										 return; } } while (0)

#define X_CheckDocument(b)		do {  if (!(b))								\
									  {  m_error = UT_IE_BOGUSDOCUMENT;	\
									     UT_DEBUGMSG(("WML import: X_CheckDocument() failed %s\n", #b)); \
										 return; } } while (0)

#define X_CheckError(v)			do {  if (!(v))								\
									  {  m_error = UT_ERROR;			\
									     UT_DEBUGMSG(("WML import: X_CheckError() failed %s\n", #v)); \
										 return; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_error) return; } while (0)

/*****************************************************************/
/*****************************************************************/

void IE_Imp_WML::openTable(const gchar **atts)
{
	const gchar * wml_columns = 0;
	wml_columns = _getXMLPropValue ("columns", atts);

	if (wml_columns)
	{
		m_iColumns = atoi(wml_columns);
		if (m_iColumns < 1)
			m_iColumns = 1;
	}
	else
		X_CheckDocument(false); // columns is a required attribute, bail out

	X_CheckError(m_TableHelperStack->tableStart(getDoc(),NULL));
}

void IE_Imp_WML::closeTable(void)
{
	X_CheckError(m_TableHelperStack->tableEnd());
}

void IE_Imp_WML::openRow(const gchar ** /*atts*/)
{
	X_CheckError(m_TableHelperStack->trStart(NULL));
}

void IE_Imp_WML::closeRow(void)
{
	//corrective code for columns that use colspan

	while(m_iColumns > m_iOpenedColumns)
	{
		const gchar ** empty = NULL;
		openCell(empty);
		closeCell();
	}
}

void IE_Imp_WML::openCell(const gchar ** /*atts*/)
{
	// Note: there's no rowspan or colspan in WML 1.1

	m_iOpenedColumns++;
	X_CheckError(m_TableHelperStack->tdStart(1,1,NULL));
}

void IE_Imp_WML::closeCell(void)
{
	m_TableHelperStack->tdEnd();
}

/*****************************************************************/
/*****************************************************************/

void IE_Imp_WML::startElement(const gchar *name,
			       const gchar **atts)
{
	UT_DEBUGMSG(("WML import: startElement: %s\n", name));

	// xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();
	
	UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);

	switch (tokenIndex)
	{
	case TT_DOCUMENT:
	{
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;
		return;
	}

	case TT_SECTION:
	{
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Sec;
		
		// Keep this appendStrux() call here to support files with more
		// than one <card>
		X_CheckError(appendStrux(PTX_Section, PP_NOPROPS));
		m_bOpenedSection = true;
		return;
	}

	case TT_HEAD:
	{
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_MetaData;
		return;
	}

	case TT_META:
	{
		X_VerifyParseState(_PS_MetaData);
		m_parseState = _PS_Meta;

		const gchar *metaname = NULL, *content = NULL;

		metaname = static_cast<const gchar*>(_getXMLPropValue("name", atts));
		content = static_cast<const gchar*>(_getXMLPropValue("content", atts));

		if(!metaname || !content)
			return;

		if(!strcmp("title", metaname))
		{
			getDoc()->setMetaDataProp("dc.title",content);
		}
		else if(!strcmp("author", metaname))
		{
			getDoc()->setMetaDataProp("dc.creator",content);
		}
		else if(!strcmp("subject", metaname))
		{
			getDoc()->setMetaDataProp("dc.subject",content);
		}
		else if(!strcmp("description", metaname))
		{
			getDoc()->setMetaDataProp("dc.description",content);
		}
		else if(!strcmp("publisher", metaname))
		{
			getDoc()->setMetaDataProp("dc.publisher",content);
		}
		else if(!strcmp("contributor", metaname))
		{
			getDoc()->setMetaDataProp("dc.contributor",content);
		}
		else if(!strcmp("source", metaname))
		{
			getDoc()->setMetaDataProp("dc.source",content);
		}
		else if(!strcmp("relation", metaname))
		{
			getDoc()->setMetaDataProp("dc.relation",content);
		}
		else if(!strcmp("coverage", metaname))
		{
			getDoc()->setMetaDataProp("dc.coverage",content);
		}
		else if(!strcmp("rights", metaname))
		{
			getDoc()->setMetaDataProp("dc.rights",content);
		}
		else if(!strcmp("keywords", metaname))
		{
			getDoc()->setMetaDataProp("abiword.keywords",content);
		}

		return;
	}

	case TT_BLOCK:
	{
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;

		const gchar *p_val = NULL;
		bool left = false;

		PP_PropertyVector attr = {
			"props", ""
		};

		p_val = static_cast<const gchar*>(_getXMLPropValue("align", atts));
		if(!p_val || !atts)
		{
			UT_DEBUGMSG(("WML: got <p> with no props\n"));
			left = true;
		}
		else
		{
			if(!strcmp(p_val, "center"))
			{
				attr[1] = "text-align:center";
			}
			else if(!strcmp(p_val, "right"))
			{
				attr[1] = "text-align:right";
			}
			else
			{
				left = true;
			}
		}

		X_CheckError(appendStrux(PTX_Block, (left ? PP_NOPROPS : attr)));
		m_bOpenedBlock = true;
		return;
	}

	case TT_IMAGE:
	{
		X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_Cell) || (m_parseState == _PS_Sec));

		if(m_parseState == _PS_Sec)
		{
			X_CheckError(appendStrux(PTX_Block,PP_NOPROPS));
			m_bOpenedBlock = true;
		}

		const gchar *p_val = NULL;
		p_val = _getXMLPropValue(static_cast<const gchar *>("src"), atts);

		if(p_val)
			createImage (p_val, atts);

		return;
	}

	case TT_BREAK:
	{
		X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_Cell));

		if(m_parseState == _PS_Block) //AbiWord doesn't allow breaks in tables
		{
			UT_UCSChar ucs = UCS_LF;
			X_CheckError(appendSpan(&ucs,1));
		}
		return;
	}

	case TT_ITALIC:
	case TT_UNDERLINE:
	case TT_BOLD:
	case TT_STRONG:
	case TT_EMPHASIS:
	case TT_BIG:
	case TT_SMALL:
	{
		X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_Cell));

		PP_PropertyVector attr = {
			"props", ""
		};

	    switch(tokenIndex)
		{
			case TT_ITALIC:
			{
				attr[1] = "font-style:italic";
				break;
			}

			case TT_UNDERLINE:
			{
				attr[1] = "text-decoration:underline";
				break;
			}

			case TT_BOLD:
			case TT_STRONG:
			case TT_EMPHASIS:
			{
				attr[1] = "font-weight:bold";
				break;
			}

			case TT_BIG:
			{
				attr[1] = "text-position:superscript";
				break;
			}

			case TT_SMALL:
			{
				attr[1] = "text-position:subscript";
				break;
			}

			default:
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				UT_DEBUGMSG(("DOM: WML: %s\n", name));
				break;
			}
		}

		X_CheckError(_pushInlineFmt(attr));
		X_CheckError(appendFmt(m_vecInlineFmt));
		return;
	}

	case TT_TABLE:
	{
		m_iColumns = 0;
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Table;
		openTable(atts);
		return;
	}

	case TT_TABLE_ROW:
	{
		m_iOpenedColumns = 0;
		X_VerifyParseState(_PS_Table);
		openRow(atts);
		return;
	}

	case TT_TABLE_CELL:
	{
		X_VerifyParseState(_PS_Table);
		m_parseState = _PS_Cell;
		openCell(atts);
		return;
	}

	case TT_ANCHOR:
	{
		X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_Cell));

		const gchar *p_val = NULL;
		p_val = _getXMLPropValue("id", atts);

		if(p_val)
		{
			PP_PropertyVector attr = {
				PT_TYPE_ATTRIBUTE_NAME,	"start",
				PT_NAME_ATTRIBUTE_NAME,	p_val
			};
			X_CheckError(appendObject(PTO_Bookmark, attr));
			attr[1] = "end";
			X_CheckError(appendObject(PTO_Bookmark, attr));
		}
		return;
	}

	case TT_LINK:
	{
		X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_Cell));

		const gchar *p_val = NULL;
		p_val = _getXMLPropValue("href", atts);

		if(p_val)
		{
			PP_PropertyVector attr = {
				"xlink:href", p_val
			};
			X_CheckError(appendObject(PTO_Hyperlink, attr));
		}
		else //href is required, bail out
		{
			X_CheckDocument(false);
		}
		return;
	}

	case TT_ACCESS:
	case TT_DO:
	case TT_FIELDSET:
	case TT_GO:
	case TT_INPUT:
	case TT_NOOP:
	case TT_ONEVENT:
	case TT_OPTGROUP:
	case TT_OPTION:
	case TT_POSTFIELD:
	case TT_PREV:
	case TT_REFRESH:
	case TT_SELECT:
	case TT_SETVAR:
	case TT_TEMPLATE:
	case TT_TIMER:
		return;

	case TT_OTHER:
	default:
		UT_DEBUGMSG(("WML: Unknown or knowingly unhandled tag [%s]\n",name));
	}

}

void IE_Imp_WML::endElement(const gchar *name)
{
	UT_DEBUGMSG(("WML import: endElement: %s\n", name));

	// xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();
	
   	UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);

	switch (tokenIndex)
	{
	case TT_DOCUMENT:
	{
		X_VerifyParseState(_PS_Doc);

		if(!m_bOpenedSection)
		{
			X_CheckError(appendStrux(PTX_Section, PP_NOPROPS));
			X_CheckError(appendStrux(PTX_Block, PP_NOPROPS));
		}

		m_parseState = _PS_Init;
		return;
	}

	case TT_SECTION:
	{
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Doc;

		if(!m_bOpenedBlock)
			X_CheckError(appendStrux(PTX_Block, PP_NOPROPS));

		m_bOpenedBlock = false;

		return;
	}

	case TT_HEAD:
	{
		X_VerifyParseState(_PS_MetaData);
		m_parseState = _PS_Doc;
		return;
	}

	case TT_META:
	{
		X_VerifyParseState(_PS_Meta);
		m_parseState = _PS_MetaData;
		return;
	}

	case TT_BLOCK:
	{
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);

		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;

		X_CheckDocument(_getInlineDepth()==0);
		return;
	}
		
	case TT_IMAGE:
	{
		X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_Cell) || (m_parseState == _PS_Sec));
		return;
	}

	case TT_BREAK:
	{
		X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_Cell));
		return;
	}

	case TT_ITALIC:
	case TT_UNDERLINE:
	case TT_BOLD:
	case TT_STRONG:
	case TT_EMPHASIS:
	case TT_BIG:
	case TT_SMALL:
	{
		UT_ASSERT_HARMLESS(m_lenCharDataSeen==0);

		X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_Cell));
		X_CheckDocument(_getInlineDepth()>0);

		_popInlineFmt();
		X_CheckError(appendFmt(m_vecInlineFmt));

		return;
	}

	case TT_TABLE:
	{
		X_VerifyParseState(_PS_Table);
		m_parseState = _PS_Block;
		m_iColumns = 0;

		closeTable();
		return;
	}

	case TT_TABLE_ROW:
	{
		X_VerifyParseState(_PS_Table);
		closeRow();
		m_iOpenedColumns = 0;
		return;
	}

	case TT_TABLE_CELL:
	{
		X_VerifyParseState(_PS_Cell);
		m_parseState = _PS_Table;
		closeCell();
		break;
	}

	case TT_ANCHOR:
	{
		X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_Cell));
		return;
	}

	case TT_LINK:
	{
		X_CheckError((m_parseState == _PS_Block) || (m_parseState == _PS_Cell));
		X_CheckError(appendObject(PTO_Hyperlink, PP_NOPROPS));
		return;
	}

	case TT_ACCESS:
	case TT_DO:
	case TT_FIELDSET:
	case TT_GO:
	case TT_INPUT:
	case TT_NOOP:
	case TT_ONEVENT:
	case TT_OPTGROUP:
	case TT_OPTION:
	case TT_POSTFIELD:
	case TT_PREV:
	case TT_REFRESH:
	case TT_SELECT:
	case TT_SETVAR:
	case TT_TEMPLATE:
	case TT_TIMER:
		return;

	case TT_OTHER:
	default:
		UT_DEBUGMSG(("WML: Unknown or intentionally unhandled end tag [%s]\n",name));
	}
}

void IE_Imp_WML::createImage(const char *name, const gchar **atts)
{
	char * relative_file = UT_go_url_resolve_relative(m_szFileName, name);
	if(!relative_file)
		return;

	UT_UTF8String filename(relative_file);
	g_free(relative_file);

	FG_ConstGraphicPtr pfg;
	if (IE_ImpGraphic::loadGraphic (filename.utf8_str(), IEGFT_Unknown, pfg) != UT_OK)
		return;

	const UT_ByteBuf * pBB = pfg->getBuffer();
	X_CheckError(pBB);

	UT_UTF8String dataid;
	UT_UTF8String_sprintf (dataid, "image%u", static_cast<unsigned int>(m_iImages++));

	UT_UTF8String alt;
	const gchar *p_val = NULL;

	p_val = _getXMLPropValue(static_cast<const gchar *>("alt"), atts);
	if (p_val)
		alt += p_val;

	X_CheckError (getDoc()->createDataItem (dataid.utf8_str(), false, pBB, pfg->getMimeType(), NULL));

	UT_UTF8String props;

	p_val = _getXMLPropValue("height", atts);

	if(p_val)
	{
		props = "height:";
		props += UT_UTF8String_sprintf("%fin", UT_convertDimToInches(UT_convertDimensionless(p_val), DIM_PX));
	}

	p_val = _getXMLPropValue("width", atts);

	if(p_val)
	{
		if(props.length())
			props+= "; ";
		props+= "width:";

		props += UT_UTF8String_sprintf("%fin", UT_convertDimToInches(UT_convertDimensionless(p_val), DIM_PX));
	}

	p_val = _getXMLPropValue("xml:lang", atts);

	if(p_val && *p_val)
	{
		if(props.length())
			props+= "; ";
		props+= "lang:";
		props+= p_val;
	}

	PP_PropertyVector attr = {
		"dataid", dataid.utf8_str(),
		"alt", alt.utf8_str(),
	};
	if(props.length())
	{
		attr.push_back(PT_PROPS_ATTRIBUTE_NAME);
		attr.push_back(props.utf8_str());
	}

	X_CheckError(appendObject(PTO_Image, attr));
}

void IE_Imp_WML::charData(const gchar *s, int len)
{
	if(m_parseState == _PS_Cell)
	{
		UT_UCS4String span = s;
		m_TableHelperStack->Inline(span.ucs4_str(), span.length());
		return;
	}

	IE_Imp_XML :: charData (s, len);
}
