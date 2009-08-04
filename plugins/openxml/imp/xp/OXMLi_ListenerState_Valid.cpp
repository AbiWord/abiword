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
		rqst->valid = false;
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
						  contextMatches(contextTag, NS_W_KEY, "ctrlPr") ||
						  contextMatches(contextTag, NS_W_KEY, "r") || 
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "rPrChange");
			break;
		}
		case KEYWORD_shd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "shd") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_snapToGrid:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "snapToGrid") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_spacing:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "spacing") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr") ||
						  contextMatches(contextTag, NS_W_KEY, "rPr");
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

		//Run, Section 2.3.2
		case KEYWORD_b:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "b") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_bCs:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bCs") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_bdr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bdr") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_caps:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "caps") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_color:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "color") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_cs:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "cs") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_dstrike:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "dstrike") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_eastAsianLayout:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "eastAsianLayout") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_effect:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "effect") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_em:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "em") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_emboss:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "emboss") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_fitText:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "fitText") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_highlight:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "highlight") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_i:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "i") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_iCs:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "iCs") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_imprint:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "imprint") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_kern:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "kern") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_lang:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "lang") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_noProof:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noProof") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_oMath:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "oMath") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_outline:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "outline") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_position:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "position") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_r:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "r") || 
						  contextMatches(contextTag, NS_W_KEY, "customXml") || 
						  contextMatches(contextTag, NS_W_KEY, "del") || 
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") || 
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") || 
						  contextMatches(contextTag, NS_W_KEY, "ins") || 
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") || 
						  contextMatches(contextTag, NS_W_KEY, "moveTo") || 
						  contextMatches(contextTag, NS_W_KEY, "p") || 
						  contextMatches(contextTag, NS_W_KEY, "rt") || 
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") || 
						  contextMatches(contextTag, NS_W_KEY, "smartTag");
			break;
		}
		case KEYWORD_rFonts:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rFonts") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_rStyle:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rStyle") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_rtl:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rtl") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_shadow:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "shadow") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_smallCaps:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "smallCaps") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_specVanish:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "specVanish") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_strike:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "strike") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_sz:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "sz") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_szCs:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "szCs") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_u:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "u") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_vanish:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "vanish") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_vertAlign:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "vertAlign") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_w:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "w") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_webHidden:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "webHidden") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}

		//TODO: add more here
	};
}

void OXMLi_ListenerState_Valid::endElement (OXMLi_EndElementRequest * rqst)
{
	std::map<std::string, int>::iterator it;
	it = m_keywordMap.find(rqst->pName);
	if(it == m_keywordMap.end())
	{
		rqst->valid = false;
		return;
	}
	rqst->valid = true;
}

void OXMLi_ListenerState_Valid::charData (OXMLi_CharDataRequest * rqst)
{
	rqst->valid = (rqst->buffer != NULL);
}

void OXMLi_ListenerState_Valid::populateKeywordTable()
{
	m_keywordMap.insert(std::make_pair("W:adjustRightInd", KEYWORD_adjustRightInd));
	m_keywordMap.insert(std::make_pair("W:autoSpaceDE", KEYWORD_autoSpaceDE));
	m_keywordMap.insert(std::make_pair("W:autoSpaceDN", KEYWORD_autoSpaceDN));
	m_keywordMap.insert(std::make_pair("W:b", KEYWORD_b));
	m_keywordMap.insert(std::make_pair("W:background", KEYWORD_background));
	m_keywordMap.insert(std::make_pair("W:bar", KEYWORD_bar));
	m_keywordMap.insert(std::make_pair("W:bCs", KEYWORD_bCs));
	m_keywordMap.insert(std::make_pair("W:bdr", KEYWORD_bdr));
	m_keywordMap.insert(std::make_pair("W:between", KEYWORD_between));
	m_keywordMap.insert(std::make_pair("W:bidi", KEYWORD_bidi));
	m_keywordMap.insert(std::make_pair("W:body", KEYWORD_body));
	m_keywordMap.insert(std::make_pair("W:bottom", KEYWORD_bottom));
	m_keywordMap.insert(std::make_pair("W:caps", KEYWORD_caps));
	m_keywordMap.insert(std::make_pair("W:cnfStyle", KEYWORD_cnfStyle));
	m_keywordMap.insert(std::make_pair("W:contextualSpacing", KEYWORD_contextualSpacing));
	m_keywordMap.insert(std::make_pair("W:color", KEYWORD_color));
	m_keywordMap.insert(std::make_pair("W:cs", KEYWORD_cs));
	m_keywordMap.insert(std::make_pair("W:divId", KEYWORD_divId));
	m_keywordMap.insert(std::make_pair("W:document", KEYWORD_document));
	m_keywordMap.insert(std::make_pair("W:dstrike", KEYWORD_dstrike));
	m_keywordMap.insert(std::make_pair("W:eastAsianLayout", KEYWORD_eastAsianLayout));
	m_keywordMap.insert(std::make_pair("W:effect", KEYWORD_effect));
	m_keywordMap.insert(std::make_pair("W:em", KEYWORD_em));
	m_keywordMap.insert(std::make_pair("W:emboss", KEYWORD_emboss));
	m_keywordMap.insert(std::make_pair("W:fitText", KEYWORD_fitText));
	m_keywordMap.insert(std::make_pair("W:framePr", KEYWORD_framePr));
	m_keywordMap.insert(std::make_pair("W:glossaryDocument", KEYWORD_glossaryDocument));
	m_keywordMap.insert(std::make_pair("W:highlight", KEYWORD_highlight));
	m_keywordMap.insert(std::make_pair("W:i", KEYWORD_i));
	m_keywordMap.insert(std::make_pair("W:iCs", KEYWORD_iCs));
	m_keywordMap.insert(std::make_pair("W:imprint", KEYWORD_imprint));
	m_keywordMap.insert(std::make_pair("W:ind", KEYWORD_ind));
	m_keywordMap.insert(std::make_pair("W:jc", KEYWORD_jc));
	m_keywordMap.insert(std::make_pair("W:keepLines", KEYWORD_keepLines));
	m_keywordMap.insert(std::make_pair("W:keepNext", KEYWORD_keepNext));
	m_keywordMap.insert(std::make_pair("W:kern", KEYWORD_kern));
	m_keywordMap.insert(std::make_pair("W:kinsoku", KEYWORD_kinsoku));
	m_keywordMap.insert(std::make_pair("W:lang", KEYWORD_lang));
	m_keywordMap.insert(std::make_pair("W:left", KEYWORD_left));
	m_keywordMap.insert(std::make_pair("W:mirrorIndents", KEYWORD_mirrorIndents));
	m_keywordMap.insert(std::make_pair("W:noProof", KEYWORD_noProof));
	m_keywordMap.insert(std::make_pair("W:numPr", KEYWORD_numPr));
	m_keywordMap.insert(std::make_pair("W:oMath", KEYWORD_oMath));
	m_keywordMap.insert(std::make_pair("W:outline", KEYWORD_outline));
	m_keywordMap.insert(std::make_pair("W:outlineLvl", KEYWORD_outlineLvl));
	m_keywordMap.insert(std::make_pair("W:overflowPunct", KEYWORD_overflowPunct));
	m_keywordMap.insert(std::make_pair("W:p", KEYWORD_p));
	m_keywordMap.insert(std::make_pair("W:pageBreakBefore", KEYWORD_pageBreakBefore));
	m_keywordMap.insert(std::make_pair("W:pBdr", KEYWORD_pBdr));
	m_keywordMap.insert(std::make_pair("W:position", KEYWORD_position));
	m_keywordMap.insert(std::make_pair("W:pPr", KEYWORD_pPr));
	m_keywordMap.insert(std::make_pair("W:pStyle", KEYWORD_pStyle));
	m_keywordMap.insert(std::make_pair("W:r", KEYWORD_r));
	m_keywordMap.insert(std::make_pair("W:rFonts", KEYWORD_rFonts));
	m_keywordMap.insert(std::make_pair("W:right", KEYWORD_right));
	m_keywordMap.insert(std::make_pair("W:rPr", KEYWORD_rPr));
	m_keywordMap.insert(std::make_pair("W:rStyle", KEYWORD_rStyle));
	m_keywordMap.insert(std::make_pair("W:rtl", KEYWORD_rtl));
	m_keywordMap.insert(std::make_pair("W:shadow", KEYWORD_shadow));
	m_keywordMap.insert(std::make_pair("W:shd", KEYWORD_shd));
	m_keywordMap.insert(std::make_pair("W:smallCaps", KEYWORD_smallCaps));
	m_keywordMap.insert(std::make_pair("W:snapToGrid", KEYWORD_snapToGrid));
	m_keywordMap.insert(std::make_pair("W:spacing", KEYWORD_spacing));
	m_keywordMap.insert(std::make_pair("W:specVanish", KEYWORD_specVanish));
	m_keywordMap.insert(std::make_pair("W:strike", KEYWORD_strike));
	m_keywordMap.insert(std::make_pair("W:suppressAutoHypens", KEYWORD_suppressAutoHypens));
	m_keywordMap.insert(std::make_pair("W:suppressLineNumbers", KEYWORD_suppressLineNumbers));
	m_keywordMap.insert(std::make_pair("W:suppressOverlap", KEYWORD_suppressOverlap));
	m_keywordMap.insert(std::make_pair("W:sz", KEYWORD_sz));
	m_keywordMap.insert(std::make_pair("W:szCs", KEYWORD_szCs));
	m_keywordMap.insert(std::make_pair("W:tab", KEYWORD_tab));
	m_keywordMap.insert(std::make_pair("W:tabs", KEYWORD_tabs));
	m_keywordMap.insert(std::make_pair("W:textAlignment", KEYWORD_textAlignment));
	m_keywordMap.insert(std::make_pair("W:textboxTightWrap", KEYWORD_textboxTightWrap));
	m_keywordMap.insert(std::make_pair("W:textDirection", KEYWORD_textDirection));
	m_keywordMap.insert(std::make_pair("W:top", KEYWORD_top));
	m_keywordMap.insert(std::make_pair("W:topLinePunct", KEYWORD_topLinePunct));
	m_keywordMap.insert(std::make_pair("W:u", KEYWORD_u));
	m_keywordMap.insert(std::make_pair("W:vanish", KEYWORD_vanish));
	m_keywordMap.insert(std::make_pair("W:vertAlign", KEYWORD_vertAlign));
	m_keywordMap.insert(std::make_pair("W:w", KEYWORD_w));
	m_keywordMap.insert(std::make_pair("W:webHidden", KEYWORD_webHidden));
	m_keywordMap.insert(std::make_pair("W:widowControl", KEYWORD_widowControl));
	m_keywordMap.insert(std::make_pair("W:wordWrap", KEYWORD_wordWrap));
	//TODO: add more here
}
