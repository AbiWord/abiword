/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiSource
 * 
 * Copyright (C) 2009 Firat Kiyak <firatkiyak@gmail.com>
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

// Class definition include
#include <OXMLi_ListenerState_Valid.h>

// Internal includes
#include <OXML_Document.h>
#include <OXML_FontManager.h>
#include <OXMLi_PackageManager.h>

// AbiWord includes
#include <ut_assert.h>
#include <ut_misc.h>

// External includes
#include <string>

OXMLi_ListenerState_Valid::OXMLi_ListenerState_Valid():
	OXMLi_ListenerState()
{
	populateKeywordTable();
}

void OXMLi_ListenerState_Valid::startElement (OXMLi_StartElementRequest * rqst)
{
	std::string contextTag = "";
	if(!rqst->context->empty())
	{
		contextTag = rqst->context->back();
	}

	std::map<std::string, int>::iterator it;
	it = m_keywordMap.find(rqst->pName);
	if(it == m_keywordMap.end())
	{
		UT_DEBUGMSG(("FRT: OpenXML importer unknown keyword: [%s]\n", rqst->pName.c_str()));
		rqst->valid = true; //TODO: change this to false once we have all the keywords added here
		return;
	}

	switch(it->second) 
	{			
	
		/**
		 * Main Document Story, Section 2.2
		 */

		case KEYWORD_background:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "background") ||
						  contextMatches(contextTag, NS_W_KEY, "document") ||
						  contextMatches(contextTag, NS_W_KEY, "glossaryDocument");
			break;
		}
		case KEYWORD_body:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "body") || 
						  contextMatches(contextTag, NS_W_KEY, "document");
			break;
		}
		case KEYWORD_document: 
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "document");
			break;
		}

		/**
		 * Paragraphs and Rich Formatting, Section 2.3
		 */

		//Paragraphs, Section 2.3.1
		case KEYWORD_adjustRightInd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "adjustRightInd") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_autoSpaceDE:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "autoSpaceDE") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_autoSpaceDN:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "autoSpaceDN") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_bar:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bar") || 
						  contextMatches(contextTag, NS_W_KEY, "pBdr");
			break;
		}
		case KEYWORD_between:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "between") || 
						  contextMatches(contextTag, NS_W_KEY, "pBdr");
			break;
		}
		case KEYWORD_bidi:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bidi") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_bottom:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bottom") || 
						  contextMatches(contextTag, NS_W_KEY, "pBdr");
			break;
		}
		case KEYWORD_cnfStyle:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "cnfStyle") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_contextualSpacing:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "contextualSpacing") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_divId:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "divId") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_framePr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "framePr") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_ind:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "ind") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_jc:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "jc") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_keepLines:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "keepLines") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_keepNext:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "keepNext") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_kinsoku:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "kinsoku") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_left:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "left") || 
						  contextMatches(contextTag, NS_W_KEY, "pBdr");
			break;
		}
		case KEYWORD_mirrorIndents:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "mirrorIndents") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_numPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "numPr") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_outlineLvl:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "outlineLvl") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_overflowPunct:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "overflowPunct") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_p:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "p") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_pageBreakBefore:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pageBreakBefore") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_pBdr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pBdr") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_pPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pPr") || 
						  contextMatches(contextTag, NS_W_KEY, "pPrChange") ||
						  contextMatches(contextTag, NS_W_KEY, "p");
			break;
		}
		case KEYWORD_pStyle:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pStyle") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_right:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "right") || 
						  contextMatches(contextTag, NS_W_KEY, "pBdr");
			break;
		}
		case KEYWORD_rPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rPr") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr") ||
						  contextMatches(contextTag, NS_W_KEY, "rPrChange");
			break;
		}
		case KEYWORD_shd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "shd") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_snapToGrid:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "snapToGrid") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_spacing:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "spacing") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_suppressAutoHypens:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "suppressAutoHypens") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_suppressLineNumbers:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "suppressLineNumbers") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_suppressOverlap:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "suppressOverlap") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_tab:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tab") || 
						  contextMatches(contextTag, NS_W_KEY, "tabs");
			break;
		}
		case KEYWORD_tabs:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tabs") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_textAlignment:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "textAlignment") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_textboxTightWrap:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "textboxTightWrap") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_textDirection:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "textDirection") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_top:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "top") || 
						  contextMatches(contextTag, NS_W_KEY, "pBdr");
			break;
		}
		case KEYWORD_topLinePunct:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "topLinePunct") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_widowControl:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "widowControl") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_wordWrap:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "wordWrap") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}

		//TODO: add more here

	};
}

void OXMLi_ListenerState_Valid::endElement (OXMLi_EndElementRequest * rqst)
{
	//TODO
	rqst->valid = true;
}

void OXMLi_ListenerState_Valid::charData (OXMLi_CharDataRequest * rqst)
{
	//TODO
	rqst->valid = true;
}

void OXMLi_ListenerState_Valid::populateKeywordTable()
{
	m_keywordMap.insert(std::make_pair("W:adjustRightInd", KEYWORD_adjustRightInd));
	m_keywordMap.insert(std::make_pair("W:autoSpaceDE", KEYWORD_autoSpaceDE));
	m_keywordMap.insert(std::make_pair("W:autoSpaceDN", KEYWORD_autoSpaceDN));
	m_keywordMap.insert(std::make_pair("W:background", KEYWORD_background));
	m_keywordMap.insert(std::make_pair("W:bar", KEYWORD_bar));
	m_keywordMap.insert(std::make_pair("W:between", KEYWORD_between));
	m_keywordMap.insert(std::make_pair("W:bidi", KEYWORD_bidi));
	m_keywordMap.insert(std::make_pair("W:body", KEYWORD_body));
	m_keywordMap.insert(std::make_pair("W:bottom", KEYWORD_bottom));
	m_keywordMap.insert(std::make_pair("W:cnfStyle", KEYWORD_cnfStyle));
	m_keywordMap.insert(std::make_pair("W:contextualSpacing", KEYWORD_contextualSpacing));
	m_keywordMap.insert(std::make_pair("W:divId", KEYWORD_divId));
	m_keywordMap.insert(std::make_pair("W:document", KEYWORD_document));
	m_keywordMap.insert(std::make_pair("W:framePr", KEYWORD_framePr));
	m_keywordMap.insert(std::make_pair("W:glossaryDocument", KEYWORD_glossaryDocument));
	m_keywordMap.insert(std::make_pair("W:ind", KEYWORD_ind));
	m_keywordMap.insert(std::make_pair("W:jc", KEYWORD_jc));
	m_keywordMap.insert(std::make_pair("W:keepLines", KEYWORD_keepLines));
	m_keywordMap.insert(std::make_pair("W:keepNext", KEYWORD_keepNext));
	m_keywordMap.insert(std::make_pair("W:kinsoku", KEYWORD_kinsoku));
	m_keywordMap.insert(std::make_pair("W:left", KEYWORD_left));
	m_keywordMap.insert(std::make_pair("W:mirrorIndents", KEYWORD_mirrorIndents));
	m_keywordMap.insert(std::make_pair("W:numPr", KEYWORD_numPr));
	m_keywordMap.insert(std::make_pair("W:outlineLvl", KEYWORD_outlineLvl));
	m_keywordMap.insert(std::make_pair("W:overflowPunct", KEYWORD_overflowPunct));
	m_keywordMap.insert(std::make_pair("W:p", KEYWORD_p));
	m_keywordMap.insert(std::make_pair("W:pageBreakBefore", KEYWORD_pageBreakBefore));
	m_keywordMap.insert(std::make_pair("W:pBdr", KEYWORD_pBdr));
	m_keywordMap.insert(std::make_pair("W:pPr", KEYWORD_pPr));
	m_keywordMap.insert(std::make_pair("W:pStyle", KEYWORD_pStyle));
	m_keywordMap.insert(std::make_pair("W:right", KEYWORD_right));
	m_keywordMap.insert(std::make_pair("W:rPr", KEYWORD_rPr));
	m_keywordMap.insert(std::make_pair("W:shd", KEYWORD_shd));
	m_keywordMap.insert(std::make_pair("W:snapToGrid", KEYWORD_snapToGrid));
	m_keywordMap.insert(std::make_pair("W:spacing", KEYWORD_spacing));
	m_keywordMap.insert(std::make_pair("W:suppressAutoHypens", KEYWORD_suppressAutoHypens));
	m_keywordMap.insert(std::make_pair("W:suppressLineNumbers", KEYWORD_suppressLineNumbers));
	m_keywordMap.insert(std::make_pair("W:suppressOverlap", KEYWORD_suppressOverlap));
	m_keywordMap.insert(std::make_pair("W:tab", KEYWORD_tab));
	m_keywordMap.insert(std::make_pair("W:tabs", KEYWORD_tabs));
	m_keywordMap.insert(std::make_pair("W:textAlignment", KEYWORD_textAlignment));
	m_keywordMap.insert(std::make_pair("W:textboxTightWrap", KEYWORD_textboxTightWrap));
	m_keywordMap.insert(std::make_pair("W:textDirection", KEYWORD_textDirection));
	m_keywordMap.insert(std::make_pair("W:top", KEYWORD_top));
	m_keywordMap.insert(std::make_pair("W:topLinePunct", KEYWORD_topLinePunct));
	m_keywordMap.insert(std::make_pair("W:widowControl", KEYWORD_widowControl));
	m_keywordMap.insert(std::make_pair("W:wordWrap", KEYWORD_wordWrap));
	//TODO: add more here
}
