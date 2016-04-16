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

#include "ut_std_string.h"

#include "ie_imp_OPML.h"
#include "ie_types.h"
#include "xap_Module.h"

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_opml_register
#define abi_plugin_unregister abipgn_opml_unregister
#define abi_plugin_supports_version abipgn_opml_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("OPML")
#endif

/****************************************************************************/

// completely generic code to allow this to be a plugin

#define PLUGIN_NAME "AbiOPML::OPML"

static IE_Imp_OPML_Sniffer * m_sniffer = 0;

ABI_BUILTIN_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
	if (!m_sniffer)
	{
		m_sniffer = new IE_Imp_OPML_Sniffer (PLUGIN_NAME);
	}

	UT_ASSERT (m_sniffer);

	mi->name = "OPML Importer";
	mi->desc = "Imports OPML documents.";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Imp::registerImporter (m_sniffer);
	return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name    = 0;
	mi->desc    = 0;
	mi->version = 0;
	mi->author  = 0;
	mi->usage   = 0;

	UT_return_val_if_fail(m_sniffer, 0);

	IE_Imp::unregisterImporter (m_sniffer);
	delete m_sniffer;
	m_sniffer = 0;

	return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, 
				 UT_uint32 /*release*/)
{
  return 1;
}

/****************************************************************************/

IE_Imp_OPML_Sniffer::IE_Imp_OPML_Sniffer (const char *_name) :
  IE_ImpSniffer(_name)
{
  // 
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_OPML_Sniffer__SuffixConfidence[] = {
	{ "opml", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_OPML_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_OPML_Sniffer__SuffixConfidence;
}

UT_Confidence_t IE_Imp_OPML_Sniffer::recognizeContents (const char * szBuf, 
						     UT_uint32 iNumbytes)
{
	UT_uint32 iLinesToRead = 6 ;  // Only examine the first few lines of the file
	UT_uint32 iBytesScanned = 0 ;
	const char *p ;
	const char *magic ;
	p = szBuf ;
	while(iLinesToRead--)
	{
		magic = "<opml" ;
		if ((iNumbytes - iBytesScanned) < strlen(magic))
			return UT_CONFIDENCE_ZILCH;

		if (strncmp(p, magic, strlen(magic)) == 0)
			return UT_CONFIDENCE_PERFECT;

		while (*p != '\n' && *p != '\r')
		{
			iBytesScanned++;
			p++;
			if(iBytesScanned+2 >= iNumbytes)
				return UT_CONFIDENCE_ZILCH;
		}
		/*  Seek past the next newline:  */
		if (*p == '\n' || *p == '\r')
		{
			iBytesScanned++;
			p++;

			if ( *p == '\n' || *p == '\r' )
			{
				iBytesScanned++;
				p++;
			}
		}

	}

	return UT_CONFIDENCE_ZILCH;
}

UT_Error IE_Imp_OPML_Sniffer::constructImporter (PD_Document * pDocument,
													  IE_Imp ** ppie)
{
	IE_Imp_OPML * p = new IE_Imp_OPML(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Imp_OPML_Sniffer::getDlgLabels (const char ** pszDesc,
											const char ** pszSuffixList,
												IEFileType * ft)
{
	*pszDesc = "OPML (.opml)";
	*pszSuffixList = "*.opml";
	*ft = getFileType();
	return true;
}

/****************************************************************************/

IE_Imp_OPML::IE_Imp_OPML(PD_Document * pDocument)
	: IE_Imp_XML (pDocument, false),
	m_bOpenedBlock(false),
	m_iCurListID(AUTO_LIST_RESERVED),
	m_iOutlineDepth(0),
	m_sMetaTag("")
{
	m_utvLists.addItem((fl_AutoNum *)NULL);
}

IE_Imp_OPML::~IE_Imp_OPML() 
{
}

/****************************************************************************/

#define TT_OTHER			0		// anything else
#define TT_DOCUMENT			1		// a document <opml>
#define TT_SECTION			2		// a section <body>
#define TT_OUTLINE			3		// <outline>
#define TT_HEAD				4		// <head>
#define TT_TITLE			5		// <title>
#define TT_DATECREATED		6		// <dateCreated>
#define TT_DATEMODIFIED		7		// <dateModified>
#define TT_OWNERNAME		8		// <ownerName>
#define TT_OWNEREMAIL		9		// <ownerEmail>
#define TT_EXPANSIONSTATE	10		// <expansionState>
#define TT_VERTSCROLLSTATE	11		// <vertScrollState>
#define TT_WINDOWTOP		12		// <windowTop>
#define TT_WINDOWLEFT		13		// <windowLeft>
#define TT_WINDOWBOTTOM		14		// <windowBottom>
#define TT_WINDOWRIGHT		15		// <windowRight>


static struct xmlToIdMapping s_Tokens[] =
{
	{	"body",					TT_SECTION			},
	{	"dateCreated",			TT_DATECREATED		},
	{	"dateModified",			TT_DATEMODIFIED		},
	{	"expansionState",		TT_EXPANSIONSTATE	},
	{	"head",					TT_HEAD				},
	{	"opml",					TT_DOCUMENT			},
	{	"outline",				TT_OUTLINE			},
	{	"ownerEmail",			TT_OWNEREMAIL		},
	{	"ownerName",			TT_OWNERNAME		},
	{	"title",				TT_TITLE			},
	{	"vertScrollState",		TT_VERTSCROLLSTATE	},
	{	"windowBottom",			TT_WINDOWBOTTOM		},
	{	"windowLeft",			TT_WINDOWLEFT		},
	{	"windowRight",			TT_WINDOWRIGHT		},
	{	"windowTop",			TT_WINDOWTOP		},
};

#define TokenTableSize	((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

/*****************************************************************/	
/*****************************************************************/	

#define X_TestParseState(ps)	((m_parseState==(ps)))

#define X_VerifyParseState(ps)	do {  if (!(X_TestParseState(ps)))			\
									  {  m_error = UT_IE_BOGUSDOCUMENT;	\
									     UT_DEBUGMSG(("OPML import: X_TestParseState() failed %s\n", #ps)); \
										 return; } } while (0)

#define X_CheckError(v)			do {  if (!(v))								\
									  {  m_error = UT_ERROR;			\
									     UT_DEBUGMSG(("OPML import: X_CheckError() failed %s\n", #v)); \
										 return; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_error) return; } while (0)


/*****************************************************************/	
/*****************************************************************/	

void IE_Imp_OPML::startElement(const gchar *name, const gchar **atts)
{
	UT_DEBUGMSG(("OPML import: startElement: %s\n", name));

	// xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();

	UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);

	switch (tokenIndex)
	{
		case TT_DOCUMENT:
		{
			X_VerifyParseState(_PS_Init);
			m_parseState = _PS_Doc;
			// append the section here (rather than the TT_SECTION case) in case
			// the file is lacking a body element
			X_CheckError(appendStrux(PTX_Section, PP_NOPROPS));
			return;
		}

		case TT_SECTION:
		{
			X_VerifyParseState(_PS_Doc);
			m_parseState = _PS_Sec;
		
			return;
		}

		case TT_HEAD:
		{
			X_VerifyParseState(_PS_Doc);
			m_parseState = _PS_MetaData;
			return;
		}

		case TT_DATECREATED:
		case TT_DATEMODIFIED:
		case TT_EXPANSIONSTATE:
		case TT_OWNEREMAIL:
		case TT_OWNERNAME:
		case TT_TITLE:
		case TT_VERTSCROLLSTATE:
		case TT_WINDOWBOTTOM:
		case TT_WINDOWLEFT:
		case TT_WINDOWTOP:
		case TT_WINDOWRIGHT:
		{
			X_VerifyParseState(_PS_MetaData);
			m_parseState = _PS_Meta;
			m_sMetaTag = name;
			return;
		}

		case TT_OUTLINE:
		{
			X_CheckError((m_parseState == _PS_Sec) || (m_parseState == _PS_List));

			m_parseState = _PS_List;
			m_iOutlineDepth++;

			const gchar *text = NULL, *url = NULL;

			text = static_cast<const gchar*>(_getXMLPropValue("text", atts));
			url = static_cast<const gchar*>(_getXMLPropValue("htmlUrl", atts));

			if(!url) //no htmlUrl attribute, try the url attribute
				url = static_cast<const gchar*>(_getXMLPropValue("url", atts));

			if(!url) //try one more - xmlURL
				url = static_cast<const gchar*>(_getXMLPropValue("xmlUrl", atts));

			if(text)
			{
				_createBullet();

				if(url) // insert a hyperlink
				{
					PP_PropertyVector attr = {
						"xlink:href", url
					};
					X_CheckError(appendObject(PTO_Hyperlink, attr));
				}

				UT_UCS4String span = text;
				X_CheckError(appendSpan(span.ucs4_str(),span.length()));

				if(url)
				{
					X_CheckError(appendObject(PTO_Hyperlink, PP_NOPROPS));
				}
			}
			return;
		}

		case TT_OTHER:
		default:
		{
			UT_DEBUGMSG(("OPML import: Unknown or knowingly unhandled tag [%s]\n",name));
		}
	}
}

void IE_Imp_OPML::endElement(const gchar *name)
{
	UT_DEBUGMSG(("OPML import: endElement: %s\n", name));

	// xml parser keeps running until buffer consumed
	X_EatIfAlreadyError();

	UT_uint32 tokenIndex = _mapNameToToken (name, s_Tokens, TokenTableSize);

	switch (tokenIndex)
	{
		case TT_DOCUMENT:
		{
			X_VerifyParseState(_PS_Doc);

			if(!m_bOpenedBlock)
				X_CheckError(appendStrux(PTX_Block, PP_NOPROPS));

			m_parseState = _PS_Init;
			return;
		}

		case TT_SECTION:
		{
			X_VerifyParseState(_PS_Sec);
			m_parseState = _PS_Doc;
			return;
		}

		case TT_HEAD:
		{
			X_VerifyParseState(_PS_MetaData);
			m_parseState = _PS_Doc;
			return;
		}

		case TT_DATECREATED:
		case TT_DATEMODIFIED:
		case TT_EXPANSIONSTATE:
		case TT_OWNEREMAIL:
		case TT_OWNERNAME:
		case TT_TITLE:
		case TT_VERTSCROLLSTATE:
		case TT_WINDOWBOTTOM:
		case TT_WINDOWLEFT:
		case TT_WINDOWTOP:
		case TT_WINDOWRIGHT:
		{
			X_VerifyParseState(_PS_Meta);
			m_parseState = _PS_MetaData;
			m_sMetaTag = "";
			return;
		}

		case TT_OUTLINE:
		{
			X_VerifyParseState(_PS_List);
			m_iOutlineDepth--;

			if(m_iOutlineDepth == 0)
				m_parseState = _PS_Sec;

			return;
		}

		case TT_OTHER:
		default:
		{
			UT_DEBUGMSG(("OPML import: Unknown or knowingly unhandled tag [%s]\n",name));
		}
	}
}

void IE_Imp_OPML::_createBullet(void)
{
	// Largely taken from the DocBook importer

	UT_return_if_fail(m_iOutlineDepth);

	if(m_iOutlineDepth > m_utvLists.getItemCount())
		m_utvLists.addItem((fl_AutoNum *)NULL);

	if(m_utvLists.getNthItem(m_iOutlineDepth - 1) == NULL)
	{
		_createList();
	}

	std::string val;

	if(m_utvLists[m_iOutlineDepth - 1])
		val = UT_std_string_sprintf ("%d", m_utvLists[m_iOutlineDepth - 1]->getLevel());
	else
		val = "1";

	PP_PropertyVector attr = {
		PT_STYLE_ATTRIBUTE_NAME, "Bullet List",
		PT_LEVEL_ATTRIBUTE_NAME, val,
		PT_LISTID_ATTRIBUTE_NAME, "",
		PT_PARENTID_ATTRIBUTE_NAME, "",
		PT_PROPS_ATTRIBUTE_NAME, ""
	};

	if(m_utvLists[m_iOutlineDepth - 1])
		val = UT_std_string_sprintf ("%d", m_utvLists[m_iOutlineDepth - 1]->getID());
	else
		val = UT_std_string_sprintf ("%d", ++m_iCurListID);

	attr[5] = val;

	if(m_utvLists[m_iOutlineDepth - 1])
		val = UT_std_string_sprintf ("%d", m_utvLists[m_iOutlineDepth - 1]->getParentID());
	else
		val = "0";

	attr[7] = val;

	val = "start-value:0; list-style:Bullet List;";
	val += UT_std_string_sprintf(" margin-left:%fin", (m_iOutlineDepth * 0.5)); //set the indent

	attr[9] = val;

	X_CheckError(appendStrux(PTX_Block, attr));
	m_bOpenedBlock = true;

	// add the list label
	PP_PropertyVector attr2 = {
		PT_TYPE_ATTRIBUTE_NAME, "list_label"
	};

	X_CheckError(appendObject (PTO_Field, attr2));
	X_CheckError(appendFmt (attr2));
	UT_UCSChar ucs = UCS_TAB;
	appendSpan(&ucs,1);
	_popInlineFmt();
	X_CheckError(appendFmt (PP_NOPROPS));
}

void IE_Imp_OPML::_createList(void)
{
	// Largely taken from the DocBook importer

	UT_return_if_fail(m_iOutlineDepth);

	int pid = 0;

	if (m_iOutlineDepth > 1)
	{
		for (int i = (m_iOutlineDepth - 2); i >= 0; i--)
		{
			/* retrieves parent id, if available */
			if (m_utvLists[i])
			{
				pid = m_utvLists[i]->getID();
				break;
			}
		}
	}

	/* creates the new list */
	fl_AutoNum *an = new fl_AutoNum (
			m_iCurListID,
			pid,
			BULLETED_LIST,
			0,
			(const gchar *)"%L.",
			(const gchar *)"",
			getDoc(),
			NULL
		);
	getDoc()->addList(an);
	an->setLevel(m_iOutlineDepth);

	/* register it in the vector */
	if(m_utvLists.setNthItem((m_iOutlineDepth - 1), an, NULL) == -1)
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

	/* increment the id counter, so that it is unique */
	m_iCurListID++;
}

void IE_Imp_OPML::charData(const gchar *s, int /*len*/)
{
	if((m_parseState == _PS_Meta) && m_sMetaTag.length())
	{
		if(!strcmp(m_sMetaTag.utf8_str(), "title"))
		{
			getDoc()->setMetaDataProp("dc.title", s);
		}
		else if(!strcmp(m_sMetaTag.utf8_str(), "ownerName"))
		{
			getDoc()->setMetaDataProp("dc.creator", s);
		}
		else
		{
			return;
		}
		// TODO: use <expansionState> for list folding
	}

	return; //all text is stored in XML attributes
}
