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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
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
						  contextMatches(contextTag, NS_W_KEY, "pPr") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_bottom:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bottom") || 
						  contextMatches(contextTag, NS_W_KEY, "pBdr") ||
						  contextMatches(contextTag, NS_W_KEY, "pgBorders") || 
						  contextMatches(contextTag, NS_W_KEY, "tcMar") ||
						  contextMatches(contextTag, NS_W_KEY, "tcBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "tblBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "tblCellMar") ||
						  contextMatches(contextTag, NS_W_KEY, "divBdr");
			break;
		}
		case KEYWORD_cnfStyle:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "cnfStyle") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr") ||
						  contextMatches(contextTag, NS_W_KEY, "tcPr") ||
						  contextMatches(contextTag, NS_W_KEY, "trPr");
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
						  contextMatches(contextTag, NS_W_KEY, "pPr") ||
						  contextMatches(contextTag, NS_W_KEY, "trPr");
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
						  contextMatches(contextTag, NS_W_KEY, "pPr") ||
						  contextMatches(contextTag, NS_W_KEY, "tblPrEx") ||
						  contextMatches(contextTag, NS_W_KEY, "trPr") ||
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
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
						  contextMatches(contextTag, NS_W_KEY, "pBdr") ||
						  contextMatches(contextTag, NS_W_KEY, "pgBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "tcBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "tcMar") ||
						  contextMatches(contextTag, NS_W_KEY, "tblCellMar") ||
						  contextMatches(contextTag, NS_W_KEY, "tblBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "divBdr");
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
						  contextMatches(contextTag, NS_W_KEY, "pPrDefault") ||
						  contextMatches(contextTag, NS_W_KEY, "tblStylePr") ||
						  contextMatches(contextTag, NS_W_KEY, "style") ||
						  contextMatches(contextTag, NS_W_KEY, "lvl") ||
						  contextMatches(contextTag, NS_W_KEY, "p");
			break;
		}
		case KEYWORD_pStyle:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pStyle") || 
						  contextMatches(contextTag, NS_W_KEY, "lvl") ||
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_right:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "right") || 
						  contextMatches(contextTag, NS_W_KEY, "pBdr") || 
						  contextMatches(contextTag, NS_W_KEY, "tcBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "tcMar") ||
						  contextMatches(contextTag, NS_W_KEY, "tblCellMar") ||
						  contextMatches(contextTag, NS_W_KEY, "pgBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "tblBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "divBdr");
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
						  contextMatches(contextTag, NS_W_KEY, "sdtPr") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtEndPr") ||
						  contextMatches(contextTag, NS_W_KEY, "rPrDefault") ||
						  contextMatches(contextTag, NS_W_KEY, "tblStylePr") ||
						  contextMatches(contextTag, NS_W_KEY, "style") ||
						  contextMatches(contextTag, NS_W_KEY, "lvl") ||
						  contextMatches(contextTag, NS_W_KEY, "rPrChange");
			break;
		}
		case KEYWORD_shd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "shd") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr") ||
						  contextMatches(contextTag, NS_W_KEY, "tcPr") ||
						  contextMatches(contextTag, NS_W_KEY, "tblPrEx") ||
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
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
						  contextMatches(contextTag, NS_W_KEY, "tabs") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
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
						  contextMatches(contextTag, NS_W_KEY, "pPr") ||
						  contextMatches(contextTag, NS_W_KEY, "tcPr") ||
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_top:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "top") || 
						  contextMatches(contextTag, NS_W_KEY, "pBdr") ||
						  contextMatches(contextTag, NS_W_KEY, "tblBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "tblCellMar") ||
						  contextMatches(contextTag, NS_W_KEY, "tcMar") ||
						  contextMatches(contextTag, NS_W_KEY, "tcBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "pgBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "divBdr");
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
						  contextMatches(contextTag, NS_W_KEY, "rPr") ||
						  contextMatches(contextTag, NS_W_KEY, "framesetSplitbar");
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
						  contextMatches(contextTag, NS_W_KEY, "rPr") ||
						  contextMatches(contextTag, NS_W_KEY, "frame") ||
						  contextMatches(contextTag, NS_W_KEY, "frameset");
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
						  contextMatches(contextTag, NS_W_KEY, "rPr") ||
						  contextMatches(contextTag, NS_W_KEY, "framesetSplitbar");
			break;
		}
		case KEYWORD_webHidden:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "webHidden") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}

		//Run Content, Section 2.3.3
		case KEYWORD_break:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "break") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_control:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "control") || 
						  contextMatches(contextTag, NS_W_KEY, "pict") || 
						  contextMatches(contextTag, NS_W_KEY, "object");
			break;
		}
		case KEYWORD_cr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "cr") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_dayLong:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "dayLong") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_dayShort:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "dayShort") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_delText:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "delText") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_dirty:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "dirty") || 
						  contextMatches(contextTag, NS_W_KEY, "rubyPr");
			break;
		}
		case KEYWORD_drawing:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "drawing") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_hps:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "hps") || 
						  contextMatches(contextTag, NS_W_KEY, "rubyPr");
			break;
		}
		case KEYWORD_hpsBaseText:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "hpsBaseText") || 
						  contextMatches(contextTag, NS_W_KEY, "rubyPr");
			break;
		}
		case KEYWORD_hpsRaise:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "hpsRaise") || 
						  contextMatches(contextTag, NS_W_KEY, "rubyPr");
			break;
		}
		case KEYWORD_lastRenderedPageBreak:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "lastRenderedPageBreak") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_lid:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "lid") || 
						  contextMatches(contextTag, NS_W_KEY, "rubyPr") ||
						  contextMatches(contextTag, NS_W_KEY, "date") ||
						  contextMatches(contextTag, NS_W_KEY, "fieldMapData");
			break;
		}
		case KEYWORD_monthLong:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "monthLong") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_monthShort:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "monthShort") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_movie:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "movie") || 
						  contextMatches(contextTag, NS_W_KEY, "pict");
			break;
		}
		case KEYWORD_noBreakHyphen:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noBreakHyphen") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_object:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "object") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_pgNum:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pgNum") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_pict:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pict") || 
						  contextMatches(contextTag, NS_W_KEY, "numPicBullet") ||
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_ptab:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "ptab") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_rt:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rt") || 
						  contextMatches(contextTag, NS_W_KEY, "ruby");
			break;
		}
		case KEYWORD_ruby:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "ruby") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_rubyAlign:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rubyAlign") || 
						  contextMatches(contextTag, NS_W_KEY, "rubyPr");
			break;
		}
		case KEYWORD_rubyBase:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rubyBase") || 
						  contextMatches(contextTag, NS_W_KEY, "ruby");
			break;
		}
		case KEYWORD_rubyPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rubyPr") || 
						  contextMatches(contextTag, NS_W_KEY, "ruby");
			break;
		}
		case KEYWORD_softHyphen:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "softHyphen") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_sym:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "sym") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_t:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "t") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_yearLong:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "yearLong") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_yearShort:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "yearShort") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}

		//Tables, Section 2.4
		case KEYWORD_bidiVisual:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bidiVisual") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
			break;
		}
		case KEYWORD_cantSplit:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "cantSplit") || 
						  contextMatches(contextTag, NS_W_KEY, "trPr");
			break;
		}
		case KEYWORD_gridAfter:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "gridAfter") || 
						  contextMatches(contextTag, NS_W_KEY, "trPr");
			break;
		}
		case KEYWORD_gridBefore:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "gridBefore") || 
						  contextMatches(contextTag, NS_W_KEY, "trPr");
			break;
		}
		case KEYWORD_gridCol:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "gridCol") || 
						  contextMatches(contextTag, NS_W_KEY, "tblGrid");
			break;
		}
		case KEYWORD_gridSpan:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "gridSpan") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_hidden:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "hidden") || 
						  contextMatches(contextTag, NS_W_KEY, "trPr") ||
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_hideMark:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "hideMark") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_hMerge:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "hMerge") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_insideH:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "insideH") || 
						  contextMatches(contextTag, NS_W_KEY, "tblBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "tcBorders");
			break;
		}
		case KEYWORD_insideV:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "insideV") || 
						  contextMatches(contextTag, NS_W_KEY, "tblBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "tcBorders");
			break;
		}
		case KEYWORD_noWrap:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noWrap") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_tbl:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tbl") || 
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
		case KEYWORD_tblBorders:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblBorders") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPrEx") ||
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
			break;
		}
		case KEYWORD_tblCellMar:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblCellMar") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPrEx") ||
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
			break;
		}
		case KEYWORD_tblCellSpacing:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblCellSpacing") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPrEx") ||
						  contextMatches(contextTag, NS_W_KEY, "trPr") ||
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
			break;
		}
		case KEYWORD_tblGrid:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblGrid") || 
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tblGridChange");
			break;
		}
		case KEYWORD_tblHeader:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblHeader") || 
						  contextMatches(contextTag, NS_W_KEY, "trPr");
			break;
		}
		case KEYWORD_tblInd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblInd") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPrEx") ||
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
			break;
		}
		case KEYWORD_tblLayout:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblLayout") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPr") ||
						  contextMatches(contextTag, NS_W_KEY, "tblPrEx");
			break;
		}
		case KEYWORD_tblLook:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblLook") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPr") ||
						  contextMatches(contextTag, NS_W_KEY, "tblPrEx");
			break;
		}
		case KEYWORD_tblOverlap:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblOverlap") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
			break;
		}
		case KEYWORD_tblpPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblpPr") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
			break;
		}
		case KEYWORD_tblPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblPr") || 
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tblStylePr") ||
						  contextMatches(contextTag, NS_W_KEY, "style") ||
						  contextMatches(contextTag, NS_W_KEY, "tblPrChange");
			break;
		}
		case KEYWORD_tblPrEx:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblPrEx") || 
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tblPrExChange");
			break;
		}
		case KEYWORD_tblStyle:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblStyle") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
			break;
		}
		case KEYWORD_tblW:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblW") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPrEx") ||
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
			break;
		}
		case KEYWORD_tc:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tc") || 
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "tr");
			break;
		}
		case KEYWORD_tcBorders:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tcBorders") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_tcFitText:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tcFitText") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_tcMar:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tcMar") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_tcPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tcPr") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPrChange") ||
						  contextMatches(contextTag, NS_W_KEY, "style") ||
						  contextMatches(contextTag, NS_W_KEY, "tblStylePr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc");
			break;
		}
		case KEYWORD_tcW:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tcW") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_tl2br:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tl2br") || 
						  contextMatches(contextTag, NS_W_KEY, "tcBorders");
			break;
		}
		case KEYWORD_tr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tr") || 
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "stdContent") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl");
			break;
		}
		case KEYWORD_tr2bl:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tr2bl") || 
						  contextMatches(contextTag, NS_W_KEY, "tcBorders");
			break;
		}
		case KEYWORD_trHeight:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "trHeight") || 
						  contextMatches(contextTag, NS_W_KEY, "trPr");
			break;
		}
		case KEYWORD_trPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "trPr") || 
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "style") ||
						  contextMatches(contextTag, NS_W_KEY, "tblStylePr") ||
						  contextMatches(contextTag, NS_W_KEY, "trPrChange");
			break;
		}
		case KEYWORD_vAlign:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "vAlign") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr") ||
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;

		}
		case KEYWORD_vMerge:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "vMerge") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_wAfter:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "wAfter") || 
						  contextMatches(contextTag, NS_W_KEY, "trPr");
			break;
		}
		case KEYWORD_wBefore:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "wBefore") || 
						  contextMatches(contextTag, NS_W_KEY, "trPr");
			break;
		}

		//Section 2.5.1, Custom XML and Smart Tags
		case KEYWORD_attr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "attr") || 
						  contextMatches(contextTag, NS_W_KEY, "customXmlPr") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTagPr");
			break;
		}
		case KEYWORD_customXml:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "customXml") || 
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl");
			break;
		}
		case KEYWORD_customXmlPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "customXmlPr") || 
						  contextMatches(contextTag, NS_W_KEY, "customXml");
			break;
		}
		case KEYWORD_smartTag:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "smartTag") || 
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag");
			break;
		}
		case KEYWORD_smartTagPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "smartTagPr") || 
						  contextMatches(contextTag, NS_W_KEY, "smartTag");
			break;
		}

		//Section 2.5.2, Structured Document Tags
		case KEYWORD_alias:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "alias") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_bibliography:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bibliography") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_calendar:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "calendar") || 
						  contextMatches(contextTag, NS_W_KEY, "date");
			break;
		}
		case KEYWORD_citation:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "citation") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_comboBox:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "comboBox") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_dataBinding:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "dataBinding") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_date:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "date") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_dateFormat:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "dateFormat") || 
						  contextMatches(contextTag, NS_W_KEY, "date");
			break;
		}
		case KEYWORD_docPart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "docPart") || 
						  contextMatches(contextTag, NS_W_KEY, "placeholder") || 
						  contextMatches(contextTag, NS_W_KEY, "docParts"); 
			break;
		}
		case KEYWORD_docPartCategory:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "docPartCategory") || 
						  contextMatches(contextTag, NS_W_KEY, "docPartList") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartObj");
			break;
		}
		case KEYWORD_docPartGallery:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "docPartGallery") || 
						  contextMatches(contextTag, NS_W_KEY, "docPartList") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartObj");
			break;
		}
		case KEYWORD_docPartList:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "docPartList") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_docPartObj:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "docPartObj") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_docPartUnique:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "docPartUnique") || 
						  contextMatches(contextTag, NS_W_KEY, "docPartList") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartObj");
			break;
		}
		case KEYWORD_dropDownList:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "dropDownList") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_equation:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "equation") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_group:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "group") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_id:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "id") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_listItem:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "listItem") || 
						  contextMatches(contextTag, NS_W_KEY, "comboBox") ||
						  contextMatches(contextTag, NS_W_KEY, "dropDownList");
			break;
		}
		case KEYWORD_lock:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "lock") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_picture:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "picture") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_placeholder:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "placeholder") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr") ||
						  contextMatches(contextTag, NS_W_KEY, "customXmlPr");
			break;
		}
		case KEYWORD_richText:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "richText") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_sdt:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "sdt") || 
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl");
			break;
		}
		case KEYWORD_sdtContent:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "sdtContent") || 
						  contextMatches(contextTag, NS_W_KEY, "sdt");
			break;
		}
		case KEYWORD_sdtEndPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "sdtEndPr") || 
						  contextMatches(contextTag, NS_W_KEY, "sdt");
			break;
		}
		case KEYWORD_sdtPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "sdtPr") || 
						  contextMatches(contextTag, NS_W_KEY, "sdt");
			break;
		}
		case KEYWORD_showingPlcHdr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "showingPlcHdr") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_storeMappedDataAs:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "storeMappedDataAs") || 
						  contextMatches(contextTag, NS_W_KEY, "date");
			break;
		}
		case KEYWORD_tag:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tag") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_temporary:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "temporary") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}
		case KEYWORD_text:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "text") || 
						  contextMatches(contextTag, NS_W_KEY, "sdtPr");
			break;
		}

		//Section 2.6, Sections
		case KEYWORD_col:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "col") || 
						  contextMatches(contextTag, NS_W_KEY, "cols");
			break;
		}
		case KEYWORD_cols:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "cols") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_docGrid:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "docGrid") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_formProt:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "formProt") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_lnNumType:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "lnNumType") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_paperSrc:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "paperSrc") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_pgBorders:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pgBorders") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_pgMar:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pgMar") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_pgNumType:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pgNumType") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_pgSz:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pgSz") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_printerSettings:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "printerSettings") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_rtlGutter:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rtlGutter") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_sectPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "sectPr") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPrChange") ||
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_type:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "type") || 
						  contextMatches(contextTag, NS_W_KEY, "types") ||
						  contextMatches(contextTag, NS_W_KEY, "sectPr") ||
						  contextMatches(contextTag, NS_W_KEY, "fieldMapData") ||
						  contextMatches(contextTag, NS_W_KEY, "odso") ||
						  contextMatches(contextTag, NS_W_KEY, "textInput");
			break;
		}

		//Section 2.7, Styles
		case KEYWORD_aliases:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "aliases") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_autoRedefine:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "autoRedefine") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_basedOn:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "basedOn") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_latentStyles:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "latentStyles") || 
						  contextMatches(contextTag, NS_W_KEY, "styles");
			break;
		}
		case KEYWORD_link:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "link") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_locked:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "locked") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_lsdException:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "lsdException") || 
						  contextMatches(contextTag, NS_W_KEY, "latentStyles");
			break;
		}
		case KEYWORD_name:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "name") || 
						  contextMatches(contextTag, NS_W_KEY, "style") ||
						  contextMatches(contextTag, NS_W_KEY, "category") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartPr") ||
						  contextMatches(contextTag, NS_W_KEY, "abstractNum") ||
						  contextMatches(contextTag, NS_W_KEY, "fieldMapData") ||
						  contextMatches(contextTag, NS_W_KEY, "ffData") ||
						  contextMatches(contextTag, NS_W_KEY, "frame");
			break;
		}
		case KEYWORD_next:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "next") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_personal:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "personal") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_personalCompose:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "personalCompose") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_personalReply:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "personalReply") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_qFormat:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "qFormat") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_rsid:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rsid") || 
						  contextMatches(contextTag, NS_W_KEY, "style") ||
						  contextMatches(contextTag, NS_W_KEY, "rsids");
			break;
		}
		case KEYWORD_semiHidden:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "semiHidden") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_style:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "style") || 
						  contextMatches(contextTag, NS_W_KEY, "docPartPr") ||
						  contextMatches(contextTag, NS_W_KEY, "styles");
			break;
		}
		case KEYWORD_styles:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "styles");
			break;
		}
		case KEYWORD_uiPriority:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "uiPriority") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_unhideWhenUsed:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "unhideWhenUsed") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_docDefaults:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "docDefaults") || 
						  contextMatches(contextTag, NS_W_KEY, "styles");
			break;
		}
		case KEYWORD_pPrDefault:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pPrDefault") || 
						  contextMatches(contextTag, NS_W_KEY, "docDefaults");
			break;
		}
		case KEYWORD_rPrDefault:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rPrDefault") || 
						  contextMatches(contextTag, NS_W_KEY, "docDefaults");
			break;
		}
		case KEYWORD_tblStyleColBandSize:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblStyleColBandSize") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
			break;
		}
		case KEYWORD_tblStylePr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblStylePr") || 
						  contextMatches(contextTag, NS_W_KEY, "style");
			break;
		}
		case KEYWORD_tblStyleRowBandSize:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblStyleRowBandSize") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
			break;
		}

		//Section 2.8, Fonts
		case KEYWORD_altName:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "altName") || 
						  contextMatches(contextTag, NS_W_KEY, "font");
			break;
		}
		case KEYWORD_charset:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "charset") || 
						  contextMatches(contextTag, NS_W_KEY, "font");
			break;
		}
		case KEYWORD_embedBold:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "embedBold") || 
						  contextMatches(contextTag, NS_W_KEY, "font");
			break;
		}
		case KEYWORD_embedBoldItalic:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "embedBoldItalic") || 
						  contextMatches(contextTag, NS_W_KEY, "font");
			break;
		}
		case KEYWORD_embedItalic:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "embedItalic") || 
						  contextMatches(contextTag, NS_W_KEY, "font");
			break;
		}
		case KEYWORD_embedRegular:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "embedRegular") || 
						  contextMatches(contextTag, NS_W_KEY, "font");
			break;
		}
		case KEYWORD_embedSystemFonts:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "embedSystemFonts") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_embedTrueTypeFonts:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "embedTrueTypeFonts") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_family:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "family") || 
						  contextMatches(contextTag, NS_W_KEY, "font");
			break;
		}
		case KEYWORD_font:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "font") || 
						  contextMatches(contextTag, NS_W_KEY, "fonts");
			break;
		}
		case KEYWORD_fonts:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "fonts");
			break;
		}
		case KEYWORD_notTrueType:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "notTrueType") || 
						  contextMatches(contextTag, NS_W_KEY, "font");
			break;
		}
		case KEYWORD_panose1:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "panose1") || 
						  contextMatches(contextTag, NS_W_KEY, "font");
			break;
		}
		case KEYWORD_pitch:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pitch") || 
						  contextMatches(contextTag, NS_W_KEY, "font");
			break;
		}
		case KEYWORD_saveSubsetFonts:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "saveSubsetFonts") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_sig:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "sig") || 
						  contextMatches(contextTag, NS_W_KEY, "font");
			break;
		}

		//Section 2.9, Numbering
		case KEYWORD_abstractNum:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "abstractNum") || 
						  contextMatches(contextTag, NS_W_KEY, "numbering");
			break;
		}
		case KEYWORD_abstractNumId:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "abstractNumId") || 
						  contextMatches(contextTag, NS_W_KEY, "num");
			break;
		}
		case KEYWORD_ilvl:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "ilvl") || 
						  contextMatches(contextTag, NS_W_KEY, "numPr");
			break;
		}
		case KEYWORD_isLgl:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "isLgl") || 
						  contextMatches(contextTag, NS_W_KEY, "lvl");
			break;
		}
		case KEYWORD_legacy:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "legacy") || 
						  contextMatches(contextTag, NS_W_KEY, "lvl");
			break;
		}
		case KEYWORD_lvl:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "lvl") || 
						  contextMatches(contextTag, NS_W_KEY, "lvlOverride") ||
						  contextMatches(contextTag, NS_W_KEY, "abstractNum");
			break;
		}
		case KEYWORD_lvlJc:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "lvlJc") || 
						  contextMatches(contextTag, NS_W_KEY, "lvl");
			break;
		}
		case KEYWORD_lvlOverride:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "lvlOverride") || 
						  contextMatches(contextTag, NS_W_KEY, "num");
			break;
		}
		case KEYWORD_lvlPicBulletId:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "lvlPicBulletId") || 
						  contextMatches(contextTag, NS_W_KEY, "lvl");
			break;
		}
		case KEYWORD_lvlRestart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "lvlRestart") || 
						  contextMatches(contextTag, NS_W_KEY, "lvl");
			break;
		}
		case KEYWORD_lvlText:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "lvlText") || 
						  contextMatches(contextTag, NS_W_KEY, "lvl");
			break;
		}
		case KEYWORD_multiLevelType:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "multiLevelType") || 
						  contextMatches(contextTag, NS_W_KEY, "abstractNum");
			break;
		}
		case KEYWORD_nsid:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "nsid") || 
						  contextMatches(contextTag, NS_W_KEY, "abstractNum");
			break;
		}
		case KEYWORD_num:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "num") || 
						  contextMatches(contextTag, NS_W_KEY, "numbering");
			break;
		}
		case KEYWORD_numbering:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "numbering");
			break;
		}
		case KEYWORD_numFmt:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "numFmt") || 
						  contextMatches(contextTag, NS_W_KEY, "lvl") ||
						  contextMatches(contextTag, NS_W_KEY, "footnotePr") ||
						  contextMatches(contextTag, NS_W_KEY, "endnotePr");
			break;
		}
		case KEYWORD_numId:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "numId") || 
						  contextMatches(contextTag, NS_W_KEY, "numPr");
			break;
		}
		case KEYWORD_numIdMacAtCleanup:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "numIdMacAtCleanup") || 
						  contextMatches(contextTag, NS_W_KEY, "numbering");
			break;
		}
		case KEYWORD_numPicBullet:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "numPicBullet") || 
						  contextMatches(contextTag, NS_W_KEY, "numbering");
			break;
		}
		case KEYWORD_numStyleLink:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "numStyleLink") || 
						  contextMatches(contextTag, NS_W_KEY, "abstractNum");
			break;
		}
		case KEYWORD_start:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "start") || 
						  contextMatches(contextTag, NS_W_KEY, "lvl");
			break;
		}
		case KEYWORD_startOverride:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "startOverride") || 
						  contextMatches(contextTag, NS_W_KEY, "lvlOverride");
			break;
		}
		case KEYWORD_styleLink:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "styleLink") || 
						  contextMatches(contextTag, NS_W_KEY, "abstractNum");
			break;
		}
		case KEYWORD_suff:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "suff") || 
						  contextMatches(contextTag, NS_W_KEY, "lvl");
			break;
		}
		case KEYWORD_tmpl:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tmpl") || 
						  contextMatches(contextTag, NS_W_KEY, "abstractNum");
			break;
		}

		//Section 2.10, Headers and Footers
		case KEYWORD_evenAndOddHeaders:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "evenAndOddHeaders") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_footerReference:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "footerReference") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_ftr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "ftr");
			break;
		}
		case KEYWORD_hdr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "hdr");
			break;
		}
		case KEYWORD_headerReference:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "headerReference") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_titlePg:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "titlePg") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}

		//Section 2.11, Footnotes and Endnotes
		case KEYWORD_continuationSeparator:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "continuationSeparator") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_endnote:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "endnote") || 
						  contextMatches(contextTag, NS_W_KEY, "endnotes") ||
						  contextMatches(contextTag, NS_W_KEY, "endnotePr");
			break;
		}
		case KEYWORD_endnotePr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "endnotePr") || 
						  contextMatches(contextTag, NS_W_KEY, "settings") ||
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_endnoteRef:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "endnoteRef") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_endnoteReference:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "endnoteReference") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_endnotes:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "endnotes");
			break;
		}
		case KEYWORD_footnote:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "footnote") || 
						  contextMatches(contextTag, NS_W_KEY, "footnotes") ||
						  contextMatches(contextTag, NS_W_KEY, "footnotePr");
			break;
		}
		case KEYWORD_footnotePr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "footnotePr") || 
						  contextMatches(contextTag, NS_W_KEY, "settings") ||
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_footnoteRef:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "footnoteRef") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_footnoteReference:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "footnoteReference") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_footnotes:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "footnotes");
			break;
		}
		case KEYWORD_noEndnote:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noEndnote") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_numRestart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "numRestart") || 
						  contextMatches(contextTag, NS_W_KEY, "footnotePr") ||
						  contextMatches(contextTag, NS_W_KEY, "endnotePr");
			break;
		}
		case KEYWORD_numStart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "numStart") || 
						  contextMatches(contextTag, NS_W_KEY, "footnotePr") ||
						  contextMatches(contextTag, NS_W_KEY, "endnotePr");
			break;
		}
		case KEYWORD_pos:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pos") || 
						  contextMatches(contextTag, NS_W_KEY, "footnotePr") ||
						  contextMatches(contextTag, NS_W_KEY, "endnotePr");
			break;
		}
		case KEYWORD_separator:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "separator") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}

		//Section 2.12, Glossary Document
		case KEYWORD_behavior:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "behavior") || 
						  contextMatches(contextTag, NS_W_KEY, "behaviors");
			break;
		}
		case KEYWORD_behaviors:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "behaviors") || 
						  contextMatches(contextTag, NS_W_KEY, "docPartPr");
			break;
		}
		case KEYWORD_category:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "category") || 
						  contextMatches(contextTag, NS_W_KEY, "docPartPr");
			break;
		}
		case KEYWORD_description:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "description") || 
						  contextMatches(contextTag, NS_W_KEY, "docPartPr");
			break;
		}
		case KEYWORD_docPartBody:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "docPartBody") || 
						  contextMatches(contextTag, NS_W_KEY, "docPart");
			break;
		}
		case KEYWORD_docPartPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "docPartPr") || 
						  contextMatches(contextTag, NS_W_KEY, "docPart");
			break;
		}
		case KEYWORD_docParts:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "docParts") || 
						  contextMatches(contextTag, NS_W_KEY, "glossaryDocument");
			break;
		}
		case KEYWORD_gallery:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "gallery") || 
						  contextMatches(contextTag, NS_W_KEY, "category");
			break;
		}
		case KEYWORD_glossaryDocument:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "glossaryDocument");
			break;
		}
		case KEYWORD_guid:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "guid") || 
						  contextMatches(contextTag, NS_W_KEY, "docPartPr");
			break;
		}
		case KEYWORD_types:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "types") || 
						  contextMatches(contextTag, NS_W_KEY, "docPartPr");
			break;
		}

		//Section 2.13, Annotations
		case KEYWORD_annotationRef:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "annotationRef") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_comment:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "comment") || 
						  contextMatches(contextTag, NS_W_KEY, "comments");
			break;
		}
		case KEYWORD_commentRangeEnd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "commentRangeEnd") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_commentRangeStart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "commentRangeStart") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_commentReference:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "commentReference") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_comments:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "comments");
			break;
		}
		case KEYWORD_cellDel:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "cellDel") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_cellIns:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "cellIns") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_cellMerge:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "cellMerge") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_customXmlDelRangeEnd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "customXmlDelRangeEnd") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_customXmlDelRangeStart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "customXmlDelRangeStart") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_customXmlInsRangeEnd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "customXmlInsRangeEnd") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_customXmlInsRangeStart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "customXmlInsRangeStart") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_customXmlMoveFromRangeEnd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "customXmlMoveFromRangeEnd") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_customXmlMoveFromRangeStart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "customXmlMoveFromRangeStart") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_customXmlMoveToRangeEnd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "customXmlMoveToRangeEnd") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_customXmlMoveToRangeStart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "customXmlMoveToRangeStart") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_del:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "del") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "rPr") ||
						  contextMatches(contextTag, NS_W_KEY, "trPr") ||
						  contextMatches(contextTag, NS_W_KEY, "ctrlPr") ||
						  contextMatches(contextTag, NS_W_KEY, "numPr") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_ins:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "ins") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "rPr") ||
						  contextMatches(contextTag, NS_W_KEY, "trPr") ||
						  contextMatches(contextTag, NS_W_KEY, "ctrlPr") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_moveFrom:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "moveFrom") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "rPr") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_moveFromRangeEnd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "moveFromRangeEnd") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_moveFromRangeStart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "moveFromRangeStart") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_moveTo:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "moveTo") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "rPr") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_moveToRangeEnd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "moveToRangeEnd") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_moveToRangeStart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "moveToRangeStart") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_numberingChange:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "numberingChange") || 
						  contextMatches(contextTag, NS_W_KEY, "fldChar") ||
						  contextMatches(contextTag, NS_W_KEY, "numPr");
			break;
		}
		case KEYWORD_pPrChange:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pPrChange") || 
						  contextMatches(contextTag, NS_W_KEY, "pPr");
			break;
		}
		case KEYWORD_rPrChange:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rPrChange") || 
						  contextMatches(contextTag, NS_W_KEY, "rPr");
			break;
		}
		case KEYWORD_sectPrChange:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "sectPrChange") || 
						  contextMatches(contextTag, NS_W_KEY, "sectPr");
			break;
		}
		case KEYWORD_tblGridChange:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblGridChange") || 
						  contextMatches(contextTag, NS_W_KEY, "tblGrid");
			break;
		}
		case KEYWORD_tblPrChange:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblPrChange") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPr");
			break;
		}
		case KEYWORD_tblPrExChange:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tblPrExChange") || 
						  contextMatches(contextTag, NS_W_KEY, "tblPrEx");
			break;
		}
		case KEYWORD_tcPrChange:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "tcPrChange") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_trPrChange:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "trPrChange") || 
						  contextMatches(contextTag, NS_W_KEY, "trPr");
			break;
		}
		case KEYWORD_bookmarkEnd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bookmarkEnd") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_bookmarkStart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bookmarkStart") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_permEnd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "permEnd") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_permStart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "permStart") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_proofErr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "proofErr") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "deg") ||
						  contextMatches(contextTag, NS_W_KEY, "del") ||
						  contextMatches(contextTag, NS_W_KEY, "den") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "e") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "fName") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "ins") ||
						  contextMatches(contextTag, NS_W_KEY, "lim") ||
						  contextMatches(contextTag, NS_W_KEY, "moveFrom") ||
						  contextMatches(contextTag, NS_W_KEY, "moveTo") ||
						  contextMatches(contextTag, NS_W_KEY, "num") ||
						  contextMatches(contextTag, NS_W_KEY, "oMath") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "rt") ||
						  contextMatches(contextTag, NS_W_KEY, "rubyBase") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag") ||
						  contextMatches(contextTag, NS_W_KEY, "sub") ||
						  contextMatches(contextTag, NS_W_KEY, "sup") ||
						  contextMatches(contextTag, NS_W_KEY, "tbl") ||
						  contextMatches(contextTag, NS_W_KEY, "tr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}

		//Section 2.14, Mail Merge
		case KEYWORD_active:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "active") || 
						  contextMatches(contextTag, NS_W_KEY, "recipientData");
			break;
		}
		case KEYWORD_activeRecord:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "activeRecord") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_addressFieldName:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "addressFieldName") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_checkErrors:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "checkErrors") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_colDelim:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "colDelim") || 
						  contextMatches(contextTag, NS_W_KEY, "odso");
			break;
		}
		case KEYWORD_column:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "column") || 
						  contextMatches(contextTag, NS_W_KEY, "recipientData") ||
						  contextMatches(contextTag, NS_W_KEY, "fieldMapData");
			break;
		}
		case KEYWORD_connectString:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "connectString") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_dataSource:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "dataSource") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_dataType:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "dataType") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_destination:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "destination") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_doNotSuppressBlankLines:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotSuppressBlankLines") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_dynamicAddress:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "dynamicAddress") || 
						  contextMatches(contextTag, NS_W_KEY, "fieldMapData");
			break;
		}
		case KEYWORD_fHdr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "fHdr") || 
						  contextMatches(contextTag, NS_W_KEY, "odso");
			break;
		}
		case KEYWORD_fieldMapData:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "fieldMapData") || 
						  contextMatches(contextTag, NS_W_KEY, "odso");
			break;
		}
		case KEYWORD_headerSource:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "headerSource") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_linkToQuery:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "linkToQuery") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_mailAsAttachment:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "mailAsAttachment") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_mailMerge:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "mailMerge") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_mailSubject:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "mailSubject") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_mainDocumentType:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "mainDocumentType") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_mappedName:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "mappedName") || 
						  contextMatches(contextTag, NS_W_KEY, "fieldMapData");
			break;
		}
		case KEYWORD_odso:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "odso") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_query:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "query") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}
		case KEYWORD_recipientData:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "recipientData") || 
						  contextMatches(contextTag, NS_W_KEY, "odso") ||
						  contextMatches(contextTag, NS_W_KEY, "recipients");
			break;
		}
		case KEYWORD_recipients:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "recipients");
			break;
		}
		case KEYWORD_src:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "src") || 
						  contextMatches(contextTag, NS_W_KEY, "odso");
			break;
		}
		case KEYWORD_table:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "table") || 
						  contextMatches(contextTag, NS_W_KEY, "odso");
			break;
		}
		case KEYWORD_udl:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "udl") || 
						  contextMatches(contextTag, NS_W_KEY, "odso");
			break;
		}
		case KEYWORD_uniqueTag:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "uniqueTag") || 
						  contextMatches(contextTag, NS_W_KEY, "recipientData");
			break;
		}
		case KEYWORD_viewMergedData:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "viewMergedData") || 
						  contextMatches(contextTag, NS_W_KEY, "mailMerge");
			break;
		}

		//Section 2.15, Settings
		case KEYWORD_activeWritingStyle:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "viewMergedData") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_alignBordersAndEdges:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "alignBordersAndEdges") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_alwaysMergeEmptyNamespace:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "alwaysMergeEmptyNamespace") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_alwaysShowPlaceholderText:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "alwaysShowPlaceholderText") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_attachedSchema:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "attachedSchema") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_attachedTemplate:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "attachedTemplate") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_autoCaption:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "autoCaption") || 
						  contextMatches(contextTag, NS_W_KEY, "autoCaptions");
			break;
		}
		case KEYWORD_autoCaptions:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "autoCaptions") || 
						  contextMatches(contextTag, NS_W_KEY, "captions");
			break;
		}
		case KEYWORD_autoFormatOverride:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "autoFormatOverride") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_autoHyphenation:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "autoHyphenation") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_bookFoldPrinting:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bookFoldPrinting") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_bookFoldPrintingSheets:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bookFoldPrintingSheets") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_bookFoldRevPrinting:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bookFoldRevPrinting") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_bordersDoNotSurroundFooter:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bordersDoNotSurroundFooter") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_bordersDoNotSurroundHeader:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bordersDoNotSurroundHeader") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_caption:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "caption") || 
						  contextMatches(contextTag, NS_W_KEY, "captions");
			break;
		}
		case KEYWORD_captions:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "captions") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_characterSpacingControl:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "characterSpacingControl") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_clickAndTypeStyle:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "clickAndTypeStyle") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_clrSchemeMapping:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "clrSchemeMapping") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_consecutiveHyphenLimit:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "consecutiveHyphenLimit") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_decimalSymbol:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "decimalSymbol") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_defaultTableStyle:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "defaultTableStyle") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_defaultTabStop:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "defaultTabStop") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_displayBackgroundShape:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "displayBackgroundShape") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_displayHorizontalDrawingGridEvery:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "displayHorizontalDrawingGridEvery") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_displayVerticalDrawingGridEvery:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "displayVerticalDrawingGridEvery") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_documentProtection:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "documentProtection") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_documentType:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "documentType") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_docVar:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "docVar") || 
						  contextMatches(contextTag, NS_W_KEY, "docVars");
			break;
		}
		case KEYWORD_docVars:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "docVars") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_doNotAutoCompressPictures:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotAutoCompressPictures") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_doNotDemarcateInvalidXml:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotDemarcateInvalidXml") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_doNotDisplayPageBoundries:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotDisplayPageBoundries") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_doNotEmbedSmartTags:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotEmbedSmartTags") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_doNotHyphenateCaps:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotHypenateCaps") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_doNotIncludeSubdocsInStats:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotIncludeSubdocsInStats") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_doNotShadeFormData:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotShadeFormData") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_doNotTrackFormatting:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotTrackFormatting") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_doNotTrackMoves:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotTrackMoves") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_doNotUseMarginsForDrawingGridOrigin:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotUseMarginsForDrawingGridOrigin") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_doNotValidateAgainstSchema:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotValidateAgainstSchema") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_drawingGridHorizontalOrigin:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "drawingGridHorizontalOrigin") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_drawingGridHorizontalSpacing:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "drawingGridHorizontalSpacing") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_drawingGridVerticalOrigin:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "drawingGridVerticalOrigin") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_drawingGridVerticalSpacing:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "drawingGridVerticalSpacing") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_forceUpgrade:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "forceUpgrade") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_formsDesign:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "formsDesign") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_gutterAtTop:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "gutterAtTop") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_hdrShapeDefaults:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "hdrShapeDefaults") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_hideGrammaticalErrors:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "hideGrammaticalErrors") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_hideSpellingErrors:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "hideSpellingErrors") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_hyphenationZone:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "hyphenationZone") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_ignoreMixedContent:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "ignoreMixedContent") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_linkStyles:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "linkStyles") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_listSeparator:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "listSeparator") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_mirrorMargins:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "mirrorMargins") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_noLineBreaksAfter:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noLineBreaksAfter") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_noLineBreaksBefore:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noLineBreaksBefore") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_noPunctuationKerning:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noPunctuationKerning") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_printFormsData:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "printFormsData") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_printFractionalCharacterWidth:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "printFractionalCharacterWidth") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_printPostScriptOverText:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "printPostScriptOverText") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_printTwoOnOne:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "printTwoOnOne") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_proofState:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "proofState") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_readModeInkLockDown:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "readModeInkLockDown") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_removeDateAndTime:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "removeDateAndTime") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_removePersonalInformation:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "removePersonalInformation") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_revisionView:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "revisionView") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_rsidRoot:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rsidRoot") || 
						  contextMatches(contextTag, NS_W_KEY, "rsids");
			break;
		}
		case KEYWORD_rsids:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "rsids") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_saveFormsData:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "saveFormsData") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_saveInvalidXml:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "saveInvalidXml") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_savePreviewPicture:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "savePreviewPicture") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_saveThroughXslt:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "saveThroughXslt") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_saveXmlDataOnly:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "saveXmlDataOnly") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_settings:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_shapeDefaults:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "shapeDefaults") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_showEnvelope:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "showEnvelope") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_showXMLTags:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "showXMLTags") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_smartTagType:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "smartTagType") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_strictFirstAndLastChars:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "strictFirstAndLastChars") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_styleLockQFset:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "styleLockQFset") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_styleLockTheme:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "styleLockTheme") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_stylePaneFormatFilter:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "stylePaneFormatFilter") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_stylePaneSortMethod:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "stylePaneSortMethod") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_summaryLength:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "summaryLength") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_themeFontLang:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "themeFontLang") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_trackRevisions:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "trackRevisions") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_updateFields:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "updateFields") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_useXSLTWhenSaving:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "useXSLTWhenSaving") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_view:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "view") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_writeProtection:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "writeProtection") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_zoom:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "zoom") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_allowPNG:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "allowPNG") || 
						  contextMatches(contextTag, NS_W_KEY, "webSettings");
			break;
		}
		case KEYWORD_blockQuote:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "blockQuote") || 
						  contextMatches(contextTag, NS_W_KEY, "div");
			break;
		}
		case KEYWORD_bodyDiv:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "bodyDiv") || 
						  contextMatches(contextTag, NS_W_KEY, "div");
			break;
		}
		case KEYWORD_div:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "div") || 
						  contextMatches(contextTag, NS_W_KEY, "divs") ||
						  contextMatches(contextTag, NS_W_KEY, "divsChild");
			break;
		}
		case KEYWORD_divBdr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "divBdr") || 
						  contextMatches(contextTag, NS_W_KEY, "div");
			break;
		}
		case KEYWORD_divs:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "divs") || 
						  contextMatches(contextTag, NS_W_KEY, "webSettings");
			break;
		}
		case KEYWORD_divsChild:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "divsChild") || 
						  contextMatches(contextTag, NS_W_KEY, "div");
			break;
		}
		case KEYWORD_doNotOrganizeInFolder:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotOrganizeInFolder") || 
						  contextMatches(contextTag, NS_W_KEY, "webSettings");
			break;
		}
		case KEYWORD_doNotRelyOnCSS:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotRelyOnCSS") || 
						  contextMatches(contextTag, NS_W_KEY, "webSettings");
			break;
		}
		case KEYWORD_doNotSaveAsSingleFile:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotSaveAsSingleFile") || 
						  contextMatches(contextTag, NS_W_KEY, "webSettings");
			break;
		}
		case KEYWORD_doNotUseLongFileNames:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotUseLongFileNames") || 
						  contextMatches(contextTag, NS_W_KEY, "webSettings");
			break;
		}
		case KEYWORD_encoding:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "encoding") || 
						  contextMatches(contextTag, NS_W_KEY, "webSettings");
			break;
		}
		case KEYWORD_flatBorders:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "flatBorders") || 
						  contextMatches(contextTag, NS_W_KEY, "framesetSplitbar");
			break;
		}
		case KEYWORD_frame:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "frame") || 
						  contextMatches(contextTag, NS_W_KEY, "frameset");
			break;
		}
		case KEYWORD_frameLayout:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "frameLayout") || 
						  contextMatches(contextTag, NS_W_KEY, "frameset");
			break;
		}
		case KEYWORD_frameset:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "frameset") || 
						  contextMatches(contextTag, NS_W_KEY, "webSettings") ||
						  contextMatches(contextTag, NS_W_KEY, "frameset");
			break;
		}
		case KEYWORD_framesetSplitbar:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "framesetSplitbar") || 
						  contextMatches(contextTag, NS_W_KEY, "frameset");
			break;
		}
		case KEYWORD_linkedToFile:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "linkedToFile") || 
						  contextMatches(contextTag, NS_W_KEY, "frame");
			break;
		}
		case KEYWORD_marBottom:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "marBottom") || 
						  contextMatches(contextTag, NS_W_KEY, "div");
			break;
		}
		case KEYWORD_marH:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "marH") || 
						  contextMatches(contextTag, NS_W_KEY, "frame");
			break;
		}
		case KEYWORD_marLeft:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "marLeft") || 
						  contextMatches(contextTag, NS_W_KEY, "div");
			break;
		}
		case KEYWORD_marRight:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "marRight") || 
						  contextMatches(contextTag, NS_W_KEY, "div");
			break;
		}
		case KEYWORD_marTop:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "marTop") || 
						  contextMatches(contextTag, NS_W_KEY, "div");
			break;
		}
		case KEYWORD_marW:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "marW") || 
						  contextMatches(contextTag, NS_W_KEY, "frame");
			break;
		}
		case KEYWORD_noBorder:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noBorder") || 
						  contextMatches(contextTag, NS_W_KEY, "framesetSplitbar");
			break;
		}
		case KEYWORD_noResizeAllowed:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noResizeAllowed") || 
						  contextMatches(contextTag, NS_W_KEY, "frame");
			break;
		}
		case KEYWORD_optimizeForBrowser:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "optimizeForBrowser") || 
						  contextMatches(contextTag, NS_W_KEY, "webSettings");
			break;
		}
		case KEYWORD_pixelsPerInch:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "pixelsPerInch") || 
						  contextMatches(contextTag, NS_W_KEY, "webSettings");
			break;
		}
		case KEYWORD_relyOnVML:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "relyOnVML") || 
						  contextMatches(contextTag, NS_W_KEY, "webSettings");
			break;
		}
		case KEYWORD_saveSmartTagsAsXml:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "saveSmartTagsAsXml") || 
						  contextMatches(contextTag, NS_W_KEY, "webSettings");
			break;
		}
		case KEYWORD_scrollbar:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "scrollbar") || 
						  contextMatches(contextTag, NS_W_KEY, "frame");
			break;
		}
		case KEYWORD_sourceFileName:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "sourceFileName") || 
						  contextMatches(contextTag, NS_W_KEY, "frame");
			break;
		}
		case KEYWORD_targetScreenSz:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "targetScreenSz") || 
						  contextMatches(contextTag, NS_W_KEY, "webSettings");
			break;
		}
		case KEYWORD_webSettings:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "webSettings");
			break;
		}
		case KEYWORD_adjustLineHeightInTable:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "adjustLineHeightInTable") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_alignTablesRowByRow:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "alignTablesRowByRow") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_allowSpaceOfSameStyleInTable:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "allowSpaceOfSameStyleInTable") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_applyBreakingRules:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "applyBreakingRules") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_autofitToFirstFixedWidthCell:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "autofitToFirstFixedWidthCell") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_autoSpaceLikeWord95:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "autoSpaceLikeWord95") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_balanceSingleByteDoubleByteWidth:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "balanceSingleByteDoubleByteWidth") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_cachedColBalance:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "cachedColBalance") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_compat:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "compat") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_convMailMergeEsc:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "convMailMergeEsc") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_displayHangulFixedWidth:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "displayHangulFixedWidth") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotAutofitConstrainedTables:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotAutofitConstrainedTables") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotBreakConstrainedForcedTable:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotBreakConstrainedForcedTable") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotBreakWrappedTables:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotBreakWrappedTables") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotExpandShiftReturn:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotExpandShiftReturn") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotLeaveBackslashAlone:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotLeaveBackslashAlone") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotSnapToGridInCell:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotSnapToGridInCell") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotSuppressIndentation:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotSuppressIndentation") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotSuppressParagraphBorders:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotSuppressParagraphBorders") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotUseEastAsianBreakRules:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotUseEastAsianBreakRules") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotUseHTMLParagraphAutoSpacing:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotUseHTMLParagraphAutoSpacing") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotUseIndentAsNumberingTabStop:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotUseIndentAsNumberingTabStop") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotVertAlignCellWithSp:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotVertAlignCellWithSp") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotVertAlignInTxbx:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotVertAlignInTxbx") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_doNotWrapTextWithPunct:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "doNotWrapTextWithPunct") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_footnoteLayoutLikeWW8:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "footnoteLayoutLikeWW8") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_forgetLastTabAlignment:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "forgetLastTabAlignment") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_growAutofit:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "growAutofit") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_layoutRawTableWidth:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "layoutRawTableWidth") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_layoutTableRowsApart:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "layoutTableRowsApart") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_lineWrapLikeWord6:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "lineWrapLikeWord6") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_mwSmallCaps:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "mwSmallCaps") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_noColumnBalance:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noColumnBalance") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_noExtraLineSpacing:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noExtraLineSpacing") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_noLeading:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noLeading") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_noSpaceRaiseLower:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noSpaceRaiseLower") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_noTabHangInd:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "noTabHangInd") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_printBodyTextBeforeHeader:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "printBodyTextBeforeHeader") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_printColBlack:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "printColBlack") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_selectFldWithFirstOrLastChar:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "selectFldWithFirstOrLastChar") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_shapeLayoutLikeWW8:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "shapeLayoutLikeWW8") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_showBreaksInFrames:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "showBreaksInFrames") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_spaceForUL:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "spaceForUL") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_spacingInWholePoints:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "spacingInWholePoints") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_splitPgBreakAndParaMark:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "splitPgBreakAndParaMark") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_subFontBySize:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "subFontBySize") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_suppressBottomSpacing:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "suppressBottomSpacing") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_suppressSpacingAtTopOfPage:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "suppressSpacingAtTopOfPage") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_suppressSpBfAfterPgBrk:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "suppressSpBfAfterPgBrk") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_suppressTopSpacing:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "suppressTopSpacing") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_suppressTopSpacingWP:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "suppressTopSpacingWP") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_swapBordersFacingPages:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "swapBordersFacingPages") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_truncateFontHeightsLikeWP6:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "truncateFontHeightsLikeWP6") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_uiCompat97To2003:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "uiCompat97To2003") || 
						  contextMatches(contextTag, NS_W_KEY, "settings");
			break;
		}
		case KEYWORD_ulTrailSpace:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "ulTrailSpace") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_underlineTabInNumList:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "underlineTabInNumList") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_useAltKinsokuLineBreakRules:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "useAltKinsokuLineBreakRules") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_useAnsiKerningPairs:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "useAnsiKerningPairs") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_useFELayout:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "useFELayout") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_useNormalStyleForList:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "useNormalStyleForList") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_usePrinterMetrics:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "usePrinterMetrics") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_useSingleBorderforContiguousCells:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "useSingleBorderforContiguousCells") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_useWord2002TableStyleRules:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "useWord2002TableStyleRules") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_useWord97LineBreakRules:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "useWord97LineBreakRules") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_wpJustification:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "wpJustification") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_wpSpaceWidth:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "wpSpaceWidth") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}
		case KEYWORD_wrapTrailSpaces:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "wrapTrailSpaces") || 
						  contextMatches(contextTag, NS_W_KEY, "compat");
			break;
		}

		//Section 2.16, Fields and Hyperlinks
		case KEYWORD_calcOnExit:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "calcOnExit") || 
						  contextMatches(contextTag, NS_W_KEY, "ffData");
			break;
		}
		case KEYWORD_checkBox:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "calcOnExit") || 
						  contextMatches(contextTag, NS_W_KEY, "ffData");
			break;
		}
		case KEYWORD_checked:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "checked") || 
						  contextMatches(contextTag, NS_W_KEY, "checkBox");
			break;
		}
		case KEYWORD_ddList:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "ddList") || 
						  contextMatches(contextTag, NS_W_KEY, "ffData");
			break;
		}
		case KEYWORD_default:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "default") || 
						  contextMatches(contextTag, NS_W_KEY, "textInput") ||
						  contextMatches(contextTag, NS_W_KEY, "ddList") ||
						  contextMatches(contextTag, NS_W_KEY, "checkBox");
			break;
		}
		case KEYWORD_delInstrText:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "delInstrText") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_enabled:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "enabled") || 
						  contextMatches(contextTag, NS_W_KEY, "ffData");
			break;
		}
		case KEYWORD_entryMacro:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "entryMacro") || 
						  contextMatches(contextTag, NS_W_KEY, "ffData");
			break;
		}
		case KEYWORD_exitMacro:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "exitMacro") || 
						  contextMatches(contextTag, NS_W_KEY, "ffData");
			break;
		}
		case KEYWORD_ffData:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "ffData") || 
						  contextMatches(contextTag, NS_W_KEY, "fldChar");
			break;
		}
		case KEYWORD_fldChar:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "fldChar") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_fldData:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "fldData") || 
						  contextMatches(contextTag, NS_W_KEY, "fldChar") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple");
			break;
		}
		case KEYWORD_fldSimple:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "fldSimple") || 
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag");
			break;
		}
		case KEYWORD_format:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "format") || 
						  contextMatches(contextTag, NS_W_KEY, "textInput");
			break;
		}
		case KEYWORD_helpText:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "helpText") || 
						  contextMatches(contextTag, NS_W_KEY, "ffData");
			break;
		}
		case KEYWORD_hyperlink:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "hyperlink") || 
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag");
			break;
		}
		case KEYWORD_instrText:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "instrText") || 
						  contextMatches(contextTag, NS_W_KEY, "r");
			break;
		}
		case KEYWORD_listEntry:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "listEntry") || 
						  contextMatches(contextTag, NS_W_KEY, "ddList");
			break;
		}
		case KEYWORD_maxLength:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "maxLength") || 
						  contextMatches(contextTag, NS_W_KEY, "textInput");
			break;
		}
		case KEYWORD_result:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "result") || 
						  contextMatches(contextTag, NS_W_KEY, "ddList");
			break;
		}
		case KEYWORD_size:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "size") || 
						  contextMatches(contextTag, NS_W_KEY, "checkBox");
			break;
		}
		case KEYWORD_sizeAuto:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "sizeAuto") || 
						  contextMatches(contextTag, NS_W_KEY, "checkBox");
			break;
		}
		case KEYWORD_statusText:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "statusText") || 
						  contextMatches(contextTag, NS_W_KEY, "ffData");
			break;
		}
		case KEYWORD_textInput:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "textInput") || 
						  contextMatches(contextTag, NS_W_KEY, "ffData");
			break;
		}

		//Section 2.17, Miscallenous Topics
		case KEYWORD_txbxContent:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "txbxContent") || 
						  contextMatches(contextTag, NS_V_KEY, "textbox");
			break;
		}
		case KEYWORD_subDoc:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "subDoc") || 
						  contextMatches(contextTag, NS_W_KEY, "customXml") ||
						  contextMatches(contextTag, NS_W_KEY, "fldSimple") ||
						  contextMatches(contextTag, NS_W_KEY, "hyperlink") ||
						  contextMatches(contextTag, NS_W_KEY, "p") ||
						  contextMatches(contextTag, NS_W_KEY, "sdtContent") ||
						  contextMatches(contextTag, NS_W_KEY, "smartTag");
			break;
		}
		case KEYWORD_altChunk:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "altChunk") || 
						  contextMatches(contextTag, NS_W_KEY, "body") ||
						  contextMatches(contextTag, NS_W_KEY, "comment") ||
						  contextMatches(contextTag, NS_W_KEY, "docPartBody") ||
						  contextMatches(contextTag, NS_W_KEY, "endnote") ||
						  contextMatches(contextTag, NS_W_KEY, "footnote") ||
						  contextMatches(contextTag, NS_W_KEY, "ftr") ||
						  contextMatches(contextTag, NS_W_KEY, "hdr") ||
						  contextMatches(contextTag, NS_W_KEY, "tc") ||
						  contextMatches(contextTag, NS_W_KEY, "txbxContent");
			break;
		}
		case KEYWORD_altChunkPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "altChunkPr") || 
						  contextMatches(contextTag, NS_W_KEY, "altChunk");
			break;
		}
		case KEYWORD_matchSrc:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "matchSrc") || 
						  contextMatches(contextTag, NS_W_KEY, "altChunkPr");
			break;
		}

		//Section 5.2, DrawingML Picture
		case KEYWORD_blipFill:
		{
			rqst->valid = nameMatches(rqst->pName, NS_PIC_KEY, "blipFill") || 
						  contextMatches(contextTag, NS_PIC_KEY, "pic");
			break;
		}
		case KEYWORD_cNvPicPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_PIC_KEY, "cNvPicPr") || 
						  contextMatches(contextTag, NS_PIC_KEY, "nvPicPr");
			break;
		}
		case KEYWORD_cNvPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_PIC_KEY, "cNvPr") || 
						  contextMatches(contextTag, NS_PIC_KEY, "nvPicPr");
			break;
		}
		case KEYWORD_nvPicPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_PIC_KEY, "nvPicPr") || 
						  contextMatches(contextTag, NS_PIC_KEY, "pic");
			break;
		}
		case KEYWORD_pic:
		{
			rqst->valid = nameMatches(rqst->pName, NS_PIC_KEY, "pic") || 
						  contextMatches(contextTag, NS_PIC_KEY, "blipFill") ||
						  contextMatches(contextTag, NS_PIC_KEY, "nvPicPr") ||
						  contextMatches(contextTag, NS_PIC_KEY, "spPr");
			break;
		}
		case KEYWORD_spPr:
		{
			rqst->valid = nameMatches(rqst->pName, NS_PIC_KEY, "spPr") || 
						  contextMatches(contextTag, NS_PIC_KEY, "pic");
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
	m_keywordMap.insert(std::make_pair("W:abstractNum", KEYWORD_abstractNum));
	m_keywordMap.insert(std::make_pair("W:abstractNumId", KEYWORD_abstractNumId));
	m_keywordMap.insert(std::make_pair("W:active", KEYWORD_active));
	m_keywordMap.insert(std::make_pair("W:activeRecord", KEYWORD_activeRecord));
	m_keywordMap.insert(std::make_pair("W:activeWritingStyle", KEYWORD_activeWritingStyle));
	m_keywordMap.insert(std::make_pair("W:addressFieldName", KEYWORD_addressFieldName));
	m_keywordMap.insert(std::make_pair("W:adjustLineHeightInTable", KEYWORD_adjustLineHeightInTable));
	m_keywordMap.insert(std::make_pair("W:adjustRightInd", KEYWORD_adjustRightInd));
	m_keywordMap.insert(std::make_pair("W:alias", KEYWORD_alias));
	m_keywordMap.insert(std::make_pair("W:aliases", KEYWORD_aliases));
	m_keywordMap.insert(std::make_pair("W:alignBordersAndEdges", KEYWORD_alignBordersAndEdges));
	m_keywordMap.insert(std::make_pair("W:alignTablesRowByRow", KEYWORD_alignTablesRowByRow));
	m_keywordMap.insert(std::make_pair("W:allowPNG", KEYWORD_allowPNG));
	m_keywordMap.insert(std::make_pair("W:allowSpaceOfSameStyleInTable", KEYWORD_allowSpaceOfSameStyleInTable));
	m_keywordMap.insert(std::make_pair("W:altChunk", KEYWORD_altChunk));
	m_keywordMap.insert(std::make_pair("W:altChunkPr", KEYWORD_altChunkPr));
	m_keywordMap.insert(std::make_pair("W:altName", KEYWORD_altName));
	m_keywordMap.insert(std::make_pair("W:alwaysMergeEmptyNamespace", KEYWORD_alwaysMergeEmptyNamespace));
	m_keywordMap.insert(std::make_pair("W:alwaysShowPlaceholderText", KEYWORD_alwaysShowPlaceholderText));
	m_keywordMap.insert(std::make_pair("W:applyBreakingRules", KEYWORD_applyBreakingRules));
	m_keywordMap.insert(std::make_pair("W:annotationRef", KEYWORD_annotationRef));
	m_keywordMap.insert(std::make_pair("W:attachedSchema", KEYWORD_attachedSchema));
	m_keywordMap.insert(std::make_pair("W:attachedTemplate", KEYWORD_attachedTemplate));
	m_keywordMap.insert(std::make_pair("W:attr", KEYWORD_attr));
	m_keywordMap.insert(std::make_pair("W:autoCaption", KEYWORD_autoCaption));
	m_keywordMap.insert(std::make_pair("W:autoCaptions", KEYWORD_autoCaptions));
	m_keywordMap.insert(std::make_pair("W:autofitToFirstFixedWidthCell", KEYWORD_autofitToFirstFixedWidthCell));
	m_keywordMap.insert(std::make_pair("W:autoFormatOverride", KEYWORD_autoFormatOverride));
	m_keywordMap.insert(std::make_pair("W:autoHyphenation", KEYWORD_autoHyphenation));
	m_keywordMap.insert(std::make_pair("W:autoRedefine", KEYWORD_autoRedefine));
	m_keywordMap.insert(std::make_pair("W:autoSpaceDE", KEYWORD_autoSpaceDE));
	m_keywordMap.insert(std::make_pair("W:autoSpaceDN", KEYWORD_autoSpaceDN));
	m_keywordMap.insert(std::make_pair("W:autoSpaceLikeWord95", KEYWORD_autoSpaceLikeWord95));
	m_keywordMap.insert(std::make_pair("W:b", KEYWORD_b));
	m_keywordMap.insert(std::make_pair("W:background", KEYWORD_background));
	m_keywordMap.insert(std::make_pair("W:balanceSingleByteDoubleByteWidth", KEYWORD_balanceSingleByteDoubleByteWidth));
	m_keywordMap.insert(std::make_pair("W:bar", KEYWORD_bar));
	m_keywordMap.insert(std::make_pair("W:basedOn", KEYWORD_basedOn));
	m_keywordMap.insert(std::make_pair("W:bCs", KEYWORD_bCs));
	m_keywordMap.insert(std::make_pair("W:bdr", KEYWORD_bdr));
	m_keywordMap.insert(std::make_pair("W:behavior", KEYWORD_behavior));
	m_keywordMap.insert(std::make_pair("W:behaviors", KEYWORD_behaviors));
	m_keywordMap.insert(std::make_pair("W:between", KEYWORD_between));
	m_keywordMap.insert(std::make_pair("W:bibliography", KEYWORD_bibliography));
	m_keywordMap.insert(std::make_pair("W:bidi", KEYWORD_bidi));
	m_keywordMap.insert(std::make_pair("W:bidiVisual", KEYWORD_bidiVisual));
	m_keywordMap.insert(std::make_pair("PIC:blipFill", KEYWORD_blipFill));
	m_keywordMap.insert(std::make_pair("W:blockQuote", KEYWORD_blockQuote));
	m_keywordMap.insert(std::make_pair("W:bookFoldPrinting", KEYWORD_bookFoldPrinting));
	m_keywordMap.insert(std::make_pair("W:bookFoldPrintingSheets", KEYWORD_bookFoldPrintingSheets));
	m_keywordMap.insert(std::make_pair("W:bookFoldRevPrinting", KEYWORD_bookFoldRevPrinting));
	m_keywordMap.insert(std::make_pair("W:body", KEYWORD_body));
	m_keywordMap.insert(std::make_pair("W:bodyDiv", KEYWORD_bodyDiv));
	m_keywordMap.insert(std::make_pair("W:bookmarkEnd", KEYWORD_bookmarkEnd));
	m_keywordMap.insert(std::make_pair("W:bookmarkStart", KEYWORD_bookmarkStart));
	m_keywordMap.insert(std::make_pair("W:bordersDoNotSurroundFooter", KEYWORD_bordersDoNotSurroundFooter));
	m_keywordMap.insert(std::make_pair("W:bordersDoNotSurroundHeader", KEYWORD_bordersDoNotSurroundHeader));
	m_keywordMap.insert(std::make_pair("W:bottom", KEYWORD_bottom));
	m_keywordMap.insert(std::make_pair("W:break", KEYWORD_break));
	m_keywordMap.insert(std::make_pair("W:cachedColBalance", KEYWORD_cachedColBalance));
	m_keywordMap.insert(std::make_pair("W:calcOnExit", KEYWORD_calcOnExit));
	m_keywordMap.insert(std::make_pair("W:calendar", KEYWORD_calendar));
	m_keywordMap.insert(std::make_pair("W:cantSplit", KEYWORD_cantSplit));
	m_keywordMap.insert(std::make_pair("W:caps", KEYWORD_caps));
	m_keywordMap.insert(std::make_pair("W:caption", KEYWORD_caption));
	m_keywordMap.insert(std::make_pair("W:captions", KEYWORD_captions));
	m_keywordMap.insert(std::make_pair("W:category", KEYWORD_category));
	m_keywordMap.insert(std::make_pair("W:cellDel", KEYWORD_cellDel));
	m_keywordMap.insert(std::make_pair("W:cellIns", KEYWORD_cellIns));
	m_keywordMap.insert(std::make_pair("W:cellMerge", KEYWORD_cellMerge));
	m_keywordMap.insert(std::make_pair("W:characterSpacingControl", KEYWORD_characterSpacingControl));
	m_keywordMap.insert(std::make_pair("W:charset", KEYWORD_charset));
	m_keywordMap.insert(std::make_pair("W:checkBox", KEYWORD_checkBox));
	m_keywordMap.insert(std::make_pair("W:checked", KEYWORD_checked));
	m_keywordMap.insert(std::make_pair("W:checkErrors", KEYWORD_checkErrors));
	m_keywordMap.insert(std::make_pair("W:citation", KEYWORD_citation));
	m_keywordMap.insert(std::make_pair("W:clickAndTypeStyle", KEYWORD_clickAndTypeStyle));
	m_keywordMap.insert(std::make_pair("W:clrSchemeMapping", KEYWORD_clrSchemeMapping));
	m_keywordMap.insert(std::make_pair("W:cnfStyle", KEYWORD_cnfStyle));
	m_keywordMap.insert(std::make_pair("PIC:cNvPicPr", KEYWORD_cNvPicPr));
	m_keywordMap.insert(std::make_pair("PIC:cNvPr", KEYWORD_cNvPr));
	m_keywordMap.insert(std::make_pair("W:col", KEYWORD_col));
	m_keywordMap.insert(std::make_pair("W:colDelim", KEYWORD_colDelim));
	m_keywordMap.insert(std::make_pair("W:color", KEYWORD_color));
	m_keywordMap.insert(std::make_pair("W:cols", KEYWORD_cols));
	m_keywordMap.insert(std::make_pair("W:column", KEYWORD_column));
	m_keywordMap.insert(std::make_pair("W:comboBox", KEYWORD_comboBox));
	m_keywordMap.insert(std::make_pair("W:comment", KEYWORD_comment));
	m_keywordMap.insert(std::make_pair("W:commentRangeEnd", KEYWORD_commentRangeEnd));
	m_keywordMap.insert(std::make_pair("W:commentRangeStart", KEYWORD_commentRangeStart));
	m_keywordMap.insert(std::make_pair("W:commentReference", KEYWORD_commentReference));
	m_keywordMap.insert(std::make_pair("W:comments", KEYWORD_comments));
	m_keywordMap.insert(std::make_pair("W:compat", KEYWORD_compat));
	m_keywordMap.insert(std::make_pair("W:connectString", KEYWORD_connectString));
	m_keywordMap.insert(std::make_pair("W:consecutiveHyphenLimit", KEYWORD_consecutiveHyphenLimit));
	m_keywordMap.insert(std::make_pair("W:contextualSpacing", KEYWORD_contextualSpacing));
	m_keywordMap.insert(std::make_pair("W:continuationSeparator", KEYWORD_continuationSeparator));
	m_keywordMap.insert(std::make_pair("W:control", KEYWORD_control));
	m_keywordMap.insert(std::make_pair("W:convMailMergeEsc", KEYWORD_convMailMergeEsc));
	m_keywordMap.insert(std::make_pair("W:cr", KEYWORD_cr));
	m_keywordMap.insert(std::make_pair("W:cs", KEYWORD_cs));
	m_keywordMap.insert(std::make_pair("W:customXml", KEYWORD_customXml));
	m_keywordMap.insert(std::make_pair("W:customXmlDelRangeEnd", KEYWORD_customXmlDelRangeEnd));
	m_keywordMap.insert(std::make_pair("W:customXmlDelRangeStart", KEYWORD_customXmlDelRangeStart));
	m_keywordMap.insert(std::make_pair("W:customXmlInsRangeEnd", KEYWORD_customXmlInsRangeEnd));
	m_keywordMap.insert(std::make_pair("W:customXmlInsRangeStart", KEYWORD_customXmlInsRangeStart));
	m_keywordMap.insert(std::make_pair("W:customXmlMoveFromRangeEnd", KEYWORD_customXmlMoveFromRangeEnd));
	m_keywordMap.insert(std::make_pair("W:customXmlMoveFromRangeStart", KEYWORD_customXmlMoveFromRangeStart));
	m_keywordMap.insert(std::make_pair("W:customXmlMoveToRangeEnd", KEYWORD_customXmlMoveToRangeEnd));
	m_keywordMap.insert(std::make_pair("W:customXmlMoveToRangeStart", KEYWORD_customXmlMoveToRangeStart));
	m_keywordMap.insert(std::make_pair("W:customXmlPr", KEYWORD_customXmlPr));
	m_keywordMap.insert(std::make_pair("W:dataBinding", KEYWORD_dataBinding));
	m_keywordMap.insert(std::make_pair("W:dataSource", KEYWORD_dataSource));
	m_keywordMap.insert(std::make_pair("W:dataType", KEYWORD_dataType));
	m_keywordMap.insert(std::make_pair("W:date", KEYWORD_date));
	m_keywordMap.insert(std::make_pair("W:dateFormat", KEYWORD_dateFormat));
	m_keywordMap.insert(std::make_pair("W:dayLong", KEYWORD_dayLong));
	m_keywordMap.insert(std::make_pair("W:dayShort", KEYWORD_dayShort));
	m_keywordMap.insert(std::make_pair("W:ddList", KEYWORD_ddList));
	m_keywordMap.insert(std::make_pair("W:decimalSymbol", KEYWORD_decimalSymbol));
	m_keywordMap.insert(std::make_pair("W:default", KEYWORD_default));
	m_keywordMap.insert(std::make_pair("W:defaultTableStyle", KEYWORD_defaultTableStyle));
	m_keywordMap.insert(std::make_pair("W:defaultTabStop", KEYWORD_defaultTabStop));
	m_keywordMap.insert(std::make_pair("W:del", KEYWORD_del));
	m_keywordMap.insert(std::make_pair("W:delInstrText", KEYWORD_delInstrText));
	m_keywordMap.insert(std::make_pair("W:delText", KEYWORD_delText));
	m_keywordMap.insert(std::make_pair("W:description", KEYWORD_description));
	m_keywordMap.insert(std::make_pair("W:destination", KEYWORD_destination));
	m_keywordMap.insert(std::make_pair("W:dirty", KEYWORD_dirty));
	m_keywordMap.insert(std::make_pair("W:displayBackgroundShape", KEYWORD_displayBackgroundShape));
	m_keywordMap.insert(std::make_pair("W:displayHangulFixedWidth", KEYWORD_displayHangulFixedWidth));
	m_keywordMap.insert(std::make_pair("W:displayHorizontalDrawingGridEvery", KEYWORD_displayHorizontalDrawingGridEvery));
	m_keywordMap.insert(std::make_pair("W:displayVerticalDrawingGridEvery", KEYWORD_displayVerticalDrawingGridEvery));
	m_keywordMap.insert(std::make_pair("W:div", KEYWORD_div));
	m_keywordMap.insert(std::make_pair("W:divBdr", KEYWORD_divBdr));
	m_keywordMap.insert(std::make_pair("W:divId", KEYWORD_divId));
	m_keywordMap.insert(std::make_pair("W:divs", KEYWORD_divs));
	m_keywordMap.insert(std::make_pair("W:divsChild", KEYWORD_divsChild));
	m_keywordMap.insert(std::make_pair("W:docDefaults", KEYWORD_docDefaults));
	m_keywordMap.insert(std::make_pair("W:docGrid", KEYWORD_docGrid));
	m_keywordMap.insert(std::make_pair("W:docPart", KEYWORD_docPart));
	m_keywordMap.insert(std::make_pair("W:docPartBody", KEYWORD_docPartBody));
	m_keywordMap.insert(std::make_pair("W:docPartCategory", KEYWORD_docPartCategory));
	m_keywordMap.insert(std::make_pair("W:docPartGallery", KEYWORD_docPartGallery));
	m_keywordMap.insert(std::make_pair("W:docPartList", KEYWORD_docPartList));
	m_keywordMap.insert(std::make_pair("W:docPartObj", KEYWORD_docPartObj));
	m_keywordMap.insert(std::make_pair("W:docPartPr", KEYWORD_docPartPr));
	m_keywordMap.insert(std::make_pair("W:docParts", KEYWORD_docParts));
	m_keywordMap.insert(std::make_pair("W:docPartUnique", KEYWORD_docPartUnique));
	m_keywordMap.insert(std::make_pair("W:document", KEYWORD_document));
	m_keywordMap.insert(std::make_pair("W:documentProtection", KEYWORD_documentProtection));
	m_keywordMap.insert(std::make_pair("W:documentType", KEYWORD_documentType));
	m_keywordMap.insert(std::make_pair("W:docVar", KEYWORD_docVar));
	m_keywordMap.insert(std::make_pair("W:docVars", KEYWORD_docVars));
	m_keywordMap.insert(std::make_pair("W:doNotAutoCompressPictures", KEYWORD_doNotAutoCompressPictures));
	m_keywordMap.insert(std::make_pair("W:doNotAutofitConstrainedTables", KEYWORD_doNotAutofitConstrainedTables));
	m_keywordMap.insert(std::make_pair("W:doNotBreakConstrainedForcedTable", KEYWORD_doNotBreakConstrainedForcedTable));
	m_keywordMap.insert(std::make_pair("W:doNotBreakWrappedTables", KEYWORD_doNotBreakWrappedTables));
	m_keywordMap.insert(std::make_pair("W:doNotDemarcateInvalidXml", KEYWORD_doNotDemarcateInvalidXml));
	m_keywordMap.insert(std::make_pair("W:doNotDisplayPageBoundries", KEYWORD_doNotDisplayPageBoundries));
	m_keywordMap.insert(std::make_pair("W:doNotEmbedSmartTags", KEYWORD_doNotEmbedSmartTags));
	m_keywordMap.insert(std::make_pair("W:doNotExpandShiftReturn", KEYWORD_doNotExpandShiftReturn));
	m_keywordMap.insert(std::make_pair("W:doNotHyphenateCaps", KEYWORD_doNotHyphenateCaps));
	m_keywordMap.insert(std::make_pair("W:doNotIncludeSubdocsInStats", KEYWORD_doNotIncludeSubdocsInStats));
	m_keywordMap.insert(std::make_pair("W:doNotLeaveBackslashAlone", KEYWORD_doNotLeaveBackslashAlone));
	m_keywordMap.insert(std::make_pair("W:doNotOrganizeInFolder", KEYWORD_doNotOrganizeInFolder));
	m_keywordMap.insert(std::make_pair("W:doNotRelyOnCSS", KEYWORD_doNotRelyOnCSS));
	m_keywordMap.insert(std::make_pair("W:doNotSaveAsSingleFile", KEYWORD_doNotSaveAsSingleFile));
	m_keywordMap.insert(std::make_pair("W:doNotShadeFormData", KEYWORD_doNotShadeFormData));
	m_keywordMap.insert(std::make_pair("W:doNotSnapToGridInCell", KEYWORD_doNotSnapToGridInCell));
	m_keywordMap.insert(std::make_pair("W:doNotSuppressBlankLines", KEYWORD_doNotSuppressBlankLines));
	m_keywordMap.insert(std::make_pair("W:doNotSuppressIndentation", KEYWORD_doNotSuppressIndentation));
	m_keywordMap.insert(std::make_pair("W:doNotSuppressParagraphBorders", KEYWORD_doNotSuppressParagraphBorders));
	m_keywordMap.insert(std::make_pair("W:doNotTrackFormatting", KEYWORD_doNotTrackFormatting));
	m_keywordMap.insert(std::make_pair("W:doNotTrackMoves", KEYWORD_doNotTrackMoves));
	m_keywordMap.insert(std::make_pair("W:doNotUseEastAsianBreakRules", KEYWORD_doNotUseEastAsianBreakRules));
	m_keywordMap.insert(std::make_pair("W:doNotUseHTMLParagraphAutoSpacing", KEYWORD_doNotUseHTMLParagraphAutoSpacing));
	m_keywordMap.insert(std::make_pair("W:doNotUseIndentAsNumberingTabStop", KEYWORD_doNotUseIndentAsNumberingTabStop));
	m_keywordMap.insert(std::make_pair("W:doNotUseLongFileNames", KEYWORD_doNotUseLongFileNames));
	m_keywordMap.insert(std::make_pair("W:doNotUseMarginsForDrawingGridOrigin", KEYWORD_doNotUseMarginsForDrawingGridOrigin));
	m_keywordMap.insert(std::make_pair("W:doNotValidateAgainstSchema", KEYWORD_doNotValidateAgainstSchema));
	m_keywordMap.insert(std::make_pair("W:doNotVertAlignCellWithSp", KEYWORD_doNotVertAlignCellWithSp));
	m_keywordMap.insert(std::make_pair("W:doNotVertAlignInTxbx", KEYWORD_doNotVertAlignInTxbx));
	m_keywordMap.insert(std::make_pair("W:doNotWrapTextWithPunct", KEYWORD_doNotWrapTextWithPunct));
	m_keywordMap.insert(std::make_pair("W:drawing", KEYWORD_drawing));
	m_keywordMap.insert(std::make_pair("W:drawingGridHorizontalOrigin", KEYWORD_drawingGridHorizontalOrigin));
	m_keywordMap.insert(std::make_pair("W:drawingGridHorizontalSpacing", KEYWORD_drawingGridHorizontalSpacing));
	m_keywordMap.insert(std::make_pair("W:dropDownList", KEYWORD_dropDownList));
	m_keywordMap.insert(std::make_pair("W:drawingGridVerticalOrigin", KEYWORD_drawingGridHorizontalOrigin));
	m_keywordMap.insert(std::make_pair("W:drawingGridVerticalSpacing", KEYWORD_drawingGridHorizontalSpacing));
	m_keywordMap.insert(std::make_pair("W:dstrike", KEYWORD_dstrike));
	m_keywordMap.insert(std::make_pair("W:dynamicAddress", KEYWORD_dynamicAddress));
	m_keywordMap.insert(std::make_pair("W:eastAsianLayout", KEYWORD_eastAsianLayout));
	m_keywordMap.insert(std::make_pair("W:effect", KEYWORD_effect));
	m_keywordMap.insert(std::make_pair("W:equation", KEYWORD_equation));
	m_keywordMap.insert(std::make_pair("W:em", KEYWORD_em));
	m_keywordMap.insert(std::make_pair("W:embedBold", KEYWORD_embedBold));
	m_keywordMap.insert(std::make_pair("W:embedBoldItalic", KEYWORD_embedBoldItalic));
	m_keywordMap.insert(std::make_pair("W:embedItalic", KEYWORD_embedItalic));
	m_keywordMap.insert(std::make_pair("W:embedRegular", KEYWORD_embedRegular));
	m_keywordMap.insert(std::make_pair("W:embedSystemFonts", KEYWORD_embedSystemFonts));
	m_keywordMap.insert(std::make_pair("W:embedTrueTypeFonts", KEYWORD_embedTrueTypeFonts));
	m_keywordMap.insert(std::make_pair("W:emboss", KEYWORD_emboss));
	m_keywordMap.insert(std::make_pair("W:enabled", KEYWORD_enabled));
	m_keywordMap.insert(std::make_pair("W:encoding", KEYWORD_encoding));
	m_keywordMap.insert(std::make_pair("W:endnote", KEYWORD_endnote));
	m_keywordMap.insert(std::make_pair("W:endnotePr", KEYWORD_endnotePr));
	m_keywordMap.insert(std::make_pair("W:endnoteRef", KEYWORD_endnoteRef));
	m_keywordMap.insert(std::make_pair("W:endnoteReference", KEYWORD_endnoteReference));
	m_keywordMap.insert(std::make_pair("W:endnotes", KEYWORD_endnotes));
	m_keywordMap.insert(std::make_pair("W:entryMacro", KEYWORD_entryMacro));
	m_keywordMap.insert(std::make_pair("W:evenAndOddHeaders", KEYWORD_evenAndOddHeaders));
	m_keywordMap.insert(std::make_pair("W:exitMacro", KEYWORD_exitMacro));
	m_keywordMap.insert(std::make_pair("W:family", KEYWORD_family));
	m_keywordMap.insert(std::make_pair("W:ffData", KEYWORD_ffData));
	m_keywordMap.insert(std::make_pair("W:fHdr", KEYWORD_fHdr));
	m_keywordMap.insert(std::make_pair("W:fieldMapData", KEYWORD_fieldMapData));
	m_keywordMap.insert(std::make_pair("W:fitText", KEYWORD_fitText));
	m_keywordMap.insert(std::make_pair("W:flatBorders", KEYWORD_flatBorders));
	m_keywordMap.insert(std::make_pair("W:fldChar", KEYWORD_fldChar));
	m_keywordMap.insert(std::make_pair("W:fldData", KEYWORD_fldData));
	m_keywordMap.insert(std::make_pair("W:fldSimple", KEYWORD_fldSimple));
	m_keywordMap.insert(std::make_pair("W:font", KEYWORD_font));
	m_keywordMap.insert(std::make_pair("W:fonts", KEYWORD_fonts));
	m_keywordMap.insert(std::make_pair("W:footerReference", KEYWORD_footerReference));
	m_keywordMap.insert(std::make_pair("W:footnote", KEYWORD_footnote));
	m_keywordMap.insert(std::make_pair("W:footnoteLayoutLikeWW8", KEYWORD_footnoteLayoutLikeWW8));
	m_keywordMap.insert(std::make_pair("W:footnotePr", KEYWORD_footnotePr));
	m_keywordMap.insert(std::make_pair("W:footnoteRef", KEYWORD_footnoteRef));
	m_keywordMap.insert(std::make_pair("W:footnoteReference", KEYWORD_footnoteReference));
	m_keywordMap.insert(std::make_pair("W:footnotes", KEYWORD_footnotes));
	m_keywordMap.insert(std::make_pair("W:forceUpgrade", KEYWORD_forceUpgrade));
	m_keywordMap.insert(std::make_pair("W:forgetLastTabAlignment", KEYWORD_forgetLastTabAlignment));
	m_keywordMap.insert(std::make_pair("W:format", KEYWORD_format));
	m_keywordMap.insert(std::make_pair("W:formProt", KEYWORD_formProt));
	m_keywordMap.insert(std::make_pair("W:formsDesign", KEYWORD_formsDesign));
	m_keywordMap.insert(std::make_pair("W:frame", KEYWORD_frame));
	m_keywordMap.insert(std::make_pair("W:frameLayout", KEYWORD_frameLayout));
	m_keywordMap.insert(std::make_pair("W:framePr", KEYWORD_framePr));
	m_keywordMap.insert(std::make_pair("W:frameset", KEYWORD_frameset));
	m_keywordMap.insert(std::make_pair("W:framesetSplitbar", KEYWORD_framesetSplitbar));
	m_keywordMap.insert(std::make_pair("W:ftr", KEYWORD_ftr));
	m_keywordMap.insert(std::make_pair("W:gallery", KEYWORD_gallery));
	m_keywordMap.insert(std::make_pair("W:glossaryDocument", KEYWORD_glossaryDocument));
	m_keywordMap.insert(std::make_pair("W:gridAfter", KEYWORD_gridAfter));
	m_keywordMap.insert(std::make_pair("W:gridBefore", KEYWORD_gridBefore));
	m_keywordMap.insert(std::make_pair("W:gridCol", KEYWORD_gridCol));
	m_keywordMap.insert(std::make_pair("W:gridSpan", KEYWORD_gridSpan));
	m_keywordMap.insert(std::make_pair("W:group", KEYWORD_group));
	m_keywordMap.insert(std::make_pair("W:growAutofit", KEYWORD_growAutofit));
	m_keywordMap.insert(std::make_pair("W:guid", KEYWORD_guid));
	m_keywordMap.insert(std::make_pair("W:gutterAtTop", KEYWORD_gutterAtTop));
	m_keywordMap.insert(std::make_pair("W:hdr", KEYWORD_hdr));
	m_keywordMap.insert(std::make_pair("W:hdrShapeDefaults", KEYWORD_hdrShapeDefaults));
	m_keywordMap.insert(std::make_pair("W:headerReference", KEYWORD_headerReference));
	m_keywordMap.insert(std::make_pair("W:headerSource", KEYWORD_headerSource));
	m_keywordMap.insert(std::make_pair("W:helpText", KEYWORD_helpText));
	m_keywordMap.insert(std::make_pair("W:hidden", KEYWORD_hidden));
	m_keywordMap.insert(std::make_pair("W:hideGrammaticalErrors", KEYWORD_hideGrammaticalErrors));
	m_keywordMap.insert(std::make_pair("W:hideMark", KEYWORD_hideMark));
	m_keywordMap.insert(std::make_pair("W:hideSpellingErrors", KEYWORD_hideSpellingErrors));
	m_keywordMap.insert(std::make_pair("W:highlight", KEYWORD_highlight));
	m_keywordMap.insert(std::make_pair("W:hMerge", KEYWORD_hMerge));
	m_keywordMap.insert(std::make_pair("W:hps", KEYWORD_hps));
	m_keywordMap.insert(std::make_pair("W:hpsBaseText", KEYWORD_hpsBaseText));
	m_keywordMap.insert(std::make_pair("W:hpsRaise", KEYWORD_hpsRaise));
	m_keywordMap.insert(std::make_pair("W:hyperlink", KEYWORD_hyperlink));
	m_keywordMap.insert(std::make_pair("W:hyphenationZone", KEYWORD_hyphenationZone));
	m_keywordMap.insert(std::make_pair("W:i", KEYWORD_i));
	m_keywordMap.insert(std::make_pair("W:iCs", KEYWORD_iCs));
	m_keywordMap.insert(std::make_pair("W:id", KEYWORD_id));
	m_keywordMap.insert(std::make_pair("W:ignoreMixedContent", KEYWORD_ignoreMixedContent));
	m_keywordMap.insert(std::make_pair("W:imprint", KEYWORD_imprint));
	m_keywordMap.insert(std::make_pair("W:ind", KEYWORD_ind));
	m_keywordMap.insert(std::make_pair("W:ins", KEYWORD_ins));
	m_keywordMap.insert(std::make_pair("W:insideH", KEYWORD_insideH));
	m_keywordMap.insert(std::make_pair("W:insideV", KEYWORD_insideV));
	m_keywordMap.insert(std::make_pair("W:instrText", KEYWORD_instrText));
	m_keywordMap.insert(std::make_pair("W:ilvl", KEYWORD_ilvl));
    m_keywordMap.insert(std::make_pair("W:isLgl", KEYWORD_isLgl));
    m_keywordMap.insert(std::make_pair("W:jc", KEYWORD_jc));
	m_keywordMap.insert(std::make_pair("W:keepLines", KEYWORD_keepLines));
	m_keywordMap.insert(std::make_pair("W:keepNext", KEYWORD_keepNext));
	m_keywordMap.insert(std::make_pair("W:kern", KEYWORD_kern));
	m_keywordMap.insert(std::make_pair("W:kinsoku", KEYWORD_kinsoku));
	m_keywordMap.insert(std::make_pair("W:lang", KEYWORD_lang));
	m_keywordMap.insert(std::make_pair("W:lastRenderedPageBreak", KEYWORD_lastRenderedPageBreak));
	m_keywordMap.insert(std::make_pair("W:latentStyles", KEYWORD_latentStyles));
	m_keywordMap.insert(std::make_pair("W:layoutRawTableWidth", KEYWORD_layoutRawTableWidth));
	m_keywordMap.insert(std::make_pair("W:layoutTableRowsApart", KEYWORD_layoutTableRowsApart));
	m_keywordMap.insert(std::make_pair("W:left", KEYWORD_left));
    m_keywordMap.insert(std::make_pair("W:legacy", KEYWORD_legacy));
	m_keywordMap.insert(std::make_pair("W:lid", KEYWORD_lid));
	m_keywordMap.insert(std::make_pair("W:lineWrapLikeWord6", KEYWORD_lineWrapLikeWord6));
	m_keywordMap.insert(std::make_pair("W:link", KEYWORD_link));
	m_keywordMap.insert(std::make_pair("W:linkedToFile", KEYWORD_linkedToFile));
	m_keywordMap.insert(std::make_pair("W:linkStyles", KEYWORD_linkStyles));
	m_keywordMap.insert(std::make_pair("W:linkToQuery", KEYWORD_linkToQuery));
	m_keywordMap.insert(std::make_pair("W:listEntry", KEYWORD_listEntry));
	m_keywordMap.insert(std::make_pair("W:listItem", KEYWORD_listItem));
	m_keywordMap.insert(std::make_pair("W:listSeparator", KEYWORD_listSeparator));
	m_keywordMap.insert(std::make_pair("W:lock", KEYWORD_lock));
	m_keywordMap.insert(std::make_pair("W:locked", KEYWORD_locked));
	m_keywordMap.insert(std::make_pair("W:lnNumType", KEYWORD_lnNumType));
	m_keywordMap.insert(std::make_pair("W:lsdException", KEYWORD_lsdException));
	m_keywordMap.insert(std::make_pair("W:lvl", KEYWORD_lvl));
	m_keywordMap.insert(std::make_pair("W:lvlJc", KEYWORD_lvlJc));
	m_keywordMap.insert(std::make_pair("W:lvlOverride", KEYWORD_lvlOverride));
	m_keywordMap.insert(std::make_pair("W:lvlPicBulletId", KEYWORD_lvlPicBulletId));
	m_keywordMap.insert(std::make_pair("W:lvlRestart", KEYWORD_lvlRestart));
	m_keywordMap.insert(std::make_pair("W:lvlText", KEYWORD_lvlText));
	m_keywordMap.insert(std::make_pair("W:mailAsAttachment", KEYWORD_mailAsAttachment));
	m_keywordMap.insert(std::make_pair("W:mailMerge", KEYWORD_mailMerge));
	m_keywordMap.insert(std::make_pair("W:mailSubject", KEYWORD_mailSubject));
	m_keywordMap.insert(std::make_pair("W:mainDocumentType", KEYWORD_mainDocumentType));
	m_keywordMap.insert(std::make_pair("W:mappedName", KEYWORD_mappedName));
	m_keywordMap.insert(std::make_pair("W:marBottom", KEYWORD_marBottom));
	m_keywordMap.insert(std::make_pair("W:marH", KEYWORD_marH));
	m_keywordMap.insert(std::make_pair("W:marLeft", KEYWORD_marLeft));
	m_keywordMap.insert(std::make_pair("W:marRight", KEYWORD_marRight));
	m_keywordMap.insert(std::make_pair("W:marTop", KEYWORD_marTop));
	m_keywordMap.insert(std::make_pair("W:marW", KEYWORD_marW));
	m_keywordMap.insert(std::make_pair("W:maxLength", KEYWORD_maxLength));
	m_keywordMap.insert(std::make_pair("W:mirrorIndents", KEYWORD_mirrorIndents));
	m_keywordMap.insert(std::make_pair("W:mirrorMargins", KEYWORD_mirrorMargins));
	m_keywordMap.insert(std::make_pair("W:monthLong", KEYWORD_monthLong));
	m_keywordMap.insert(std::make_pair("W:monthShort", KEYWORD_monthShort));
	m_keywordMap.insert(std::make_pair("W:moveFrom", KEYWORD_moveFrom));
	m_keywordMap.insert(std::make_pair("W:moveFromRangeEnd", KEYWORD_moveFromRangeEnd));
	m_keywordMap.insert(std::make_pair("W:moveFromRangeStart", KEYWORD_moveFromRangeStart));
	m_keywordMap.insert(std::make_pair("W:moveTo", KEYWORD_moveTo));
	m_keywordMap.insert(std::make_pair("W:moveToRangeEnd", KEYWORD_moveToRangeEnd));
	m_keywordMap.insert(std::make_pair("W:moveToRangeStart", KEYWORD_moveToRangeStart));
	m_keywordMap.insert(std::make_pair("W:movie", KEYWORD_movie));
	m_keywordMap.insert(std::make_pair("W:multiLevelType", KEYWORD_multiLevelType));
	m_keywordMap.insert(std::make_pair("W:mwSmallCaps", KEYWORD_mwSmallCaps));
	m_keywordMap.insert(std::make_pair("W:name", KEYWORD_name));
	m_keywordMap.insert(std::make_pair("W:next", KEYWORD_next));
	m_keywordMap.insert(std::make_pair("W:noBorder", KEYWORD_noBorder));
	m_keywordMap.insert(std::make_pair("W:noBreakHyphen", KEYWORD_noBreakHyphen));
	m_keywordMap.insert(std::make_pair("W:noColumnBalance", KEYWORD_noColumnBalance));
	m_keywordMap.insert(std::make_pair("W:noEndnote", KEYWORD_noEndnote));
	m_keywordMap.insert(std::make_pair("W:noExtraLineSpacing", KEYWORD_noExtraLineSpacing));
	m_keywordMap.insert(std::make_pair("W:noLeading", KEYWORD_noLeading));
	m_keywordMap.insert(std::make_pair("W:noLineBreaksAfter", KEYWORD_noLineBreaksAfter));
	m_keywordMap.insert(std::make_pair("W:noLineBreaksBefore", KEYWORD_noLineBreaksBefore));
	m_keywordMap.insert(std::make_pair("W:noProof", KEYWORD_noProof));
	m_keywordMap.insert(std::make_pair("W:noPunctuationKerning", KEYWORD_noPunctuationKerning));
	m_keywordMap.insert(std::make_pair("W:noResizeAllowed", KEYWORD_noResizeAllowed));
	m_keywordMap.insert(std::make_pair("W:noSpaceRaiseLower", KEYWORD_noSpaceRaiseLower));
	m_keywordMap.insert(std::make_pair("W:noTabHangInd", KEYWORD_noTabHangInd));
	m_keywordMap.insert(std::make_pair("W:notTrueType", KEYWORD_notTrueType));
	m_keywordMap.insert(std::make_pair("W:noWrap", KEYWORD_noWrap));
	m_keywordMap.insert(std::make_pair("W:nsid", KEYWORD_nsid));
	m_keywordMap.insert(std::make_pair("W:num", KEYWORD_num));
	m_keywordMap.insert(std::make_pair("W:numbering", KEYWORD_numbering));
	m_keywordMap.insert(std::make_pair("W:numberingChange", KEYWORD_numberingChange));
	m_keywordMap.insert(std::make_pair("W:numFmt", KEYWORD_numFmt));
	m_keywordMap.insert(std::make_pair("W:numId", KEYWORD_numId));
	m_keywordMap.insert(std::make_pair("W:numIdMacAtCleanup", KEYWORD_numIdMacAtCleanup));
	m_keywordMap.insert(std::make_pair("W:numPicBullet", KEYWORD_numPicBullet));
	m_keywordMap.insert(std::make_pair("W:numPr", KEYWORD_numPr));
	m_keywordMap.insert(std::make_pair("W:numRestart", KEYWORD_numRestart));
	m_keywordMap.insert(std::make_pair("W:numStart", KEYWORD_numStart));
	m_keywordMap.insert(std::make_pair("W:numStyleLink", KEYWORD_numStyleLink));
	m_keywordMap.insert(std::make_pair("PIC:nvPicPr", KEYWORD_nvPicPr));
	m_keywordMap.insert(std::make_pair("W:object", KEYWORD_object));
	m_keywordMap.insert(std::make_pair("W:odso", KEYWORD_odso));
	m_keywordMap.insert(std::make_pair("W:oMath", KEYWORD_oMath));
	m_keywordMap.insert(std::make_pair("W:optimizeForBrowser", KEYWORD_optimizeForBrowser));
	m_keywordMap.insert(std::make_pair("W:outline", KEYWORD_outline));
	m_keywordMap.insert(std::make_pair("W:outlineLvl", KEYWORD_outlineLvl));
	m_keywordMap.insert(std::make_pair("W:overflowPunct", KEYWORD_overflowPunct));
	m_keywordMap.insert(std::make_pair("W:p", KEYWORD_p));
	m_keywordMap.insert(std::make_pair("W:pageBreakBefore", KEYWORD_pageBreakBefore));
	m_keywordMap.insert(std::make_pair("W:panose1", KEYWORD_panose1));
	m_keywordMap.insert(std::make_pair("W:paperSrc", KEYWORD_paperSrc));
	m_keywordMap.insert(std::make_pair("W:pBdr", KEYWORD_pBdr));
	m_keywordMap.insert(std::make_pair("W:permEnd", KEYWORD_permEnd));
	m_keywordMap.insert(std::make_pair("W:permStart", KEYWORD_permStart));
	m_keywordMap.insert(std::make_pair("W:personal", KEYWORD_personal));
	m_keywordMap.insert(std::make_pair("W:personalCompose", KEYWORD_personalCompose));
	m_keywordMap.insert(std::make_pair("W:personalReply", KEYWORD_personalReply));
	m_keywordMap.insert(std::make_pair("W:pgBorders", KEYWORD_pgBorders));
	m_keywordMap.insert(std::make_pair("W:pgMar", KEYWORD_pgMar));
	m_keywordMap.insert(std::make_pair("W:pgNum", KEYWORD_pgNum));
	m_keywordMap.insert(std::make_pair("W:pgNumType", KEYWORD_pgNumType));
	m_keywordMap.insert(std::make_pair("W:pgSz", KEYWORD_pgSz));
	m_keywordMap.insert(std::make_pair("PIC:pic", KEYWORD_pic));
	m_keywordMap.insert(std::make_pair("W:pict", KEYWORD_pict));
	m_keywordMap.insert(std::make_pair("W:picture", KEYWORD_picture));
	m_keywordMap.insert(std::make_pair("W:pitch", KEYWORD_pitch));
	m_keywordMap.insert(std::make_pair("W:pixelsPerInch", KEYWORD_pixelsPerInch));
	m_keywordMap.insert(std::make_pair("W:placeholder", KEYWORD_placeholder));
	m_keywordMap.insert(std::make_pair("W:pos", KEYWORD_pos));
	m_keywordMap.insert(std::make_pair("W:position", KEYWORD_position));
	m_keywordMap.insert(std::make_pair("W:printBodyTextBeforeHeader", KEYWORD_printBodyTextBeforeHeader));
	m_keywordMap.insert(std::make_pair("W:printColBlack", KEYWORD_printColBlack));
	m_keywordMap.insert(std::make_pair("W:printerSettings", KEYWORD_printerSettings));
	m_keywordMap.insert(std::make_pair("W:printFormsData", KEYWORD_printFormsData));
	m_keywordMap.insert(std::make_pair("W:printFractionalCharacterWidth", KEYWORD_printFractionalCharacterWidth));
	m_keywordMap.insert(std::make_pair("W:printPostScriptOverText", KEYWORD_printPostScriptOverText));
	m_keywordMap.insert(std::make_pair("W:printTwoOnOne", KEYWORD_printTwoOnOne));
	m_keywordMap.insert(std::make_pair("W:pPr", KEYWORD_pPr));
	m_keywordMap.insert(std::make_pair("W:pPrChange", KEYWORD_pPrChange));
	m_keywordMap.insert(std::make_pair("W:pPrDefault", KEYWORD_pPrDefault));
	m_keywordMap.insert(std::make_pair("W:proofErr", KEYWORD_proofErr));
	m_keywordMap.insert(std::make_pair("W:proofState", KEYWORD_proofState));
	m_keywordMap.insert(std::make_pair("W:pStyle", KEYWORD_pStyle));
	m_keywordMap.insert(std::make_pair("W:ptab", KEYWORD_ptab));
	m_keywordMap.insert(std::make_pair("W:qFormat", KEYWORD_qFormat));
	m_keywordMap.insert(std::make_pair("W:query", KEYWORD_query));
	m_keywordMap.insert(std::make_pair("W:r", KEYWORD_r));
	m_keywordMap.insert(std::make_pair("W:readModeInkLockDown", KEYWORD_readModeInkLockDown));
	m_keywordMap.insert(std::make_pair("W:recipientData", KEYWORD_recipientData));
	m_keywordMap.insert(std::make_pair("W:recipients", KEYWORD_recipients));
	m_keywordMap.insert(std::make_pair("W:relyOnVML", KEYWORD_relyOnVML));
	m_keywordMap.insert(std::make_pair("W:removeDateAndTime", KEYWORD_removeDateAndTime));
	m_keywordMap.insert(std::make_pair("W:removePersonalInformation", KEYWORD_removePersonalInformation));
	m_keywordMap.insert(std::make_pair("W:result", KEYWORD_result));
	m_keywordMap.insert(std::make_pair("W:revisionView", KEYWORD_revisionView));
	m_keywordMap.insert(std::make_pair("W:rFonts", KEYWORD_rFonts));
	m_keywordMap.insert(std::make_pair("W:richText", KEYWORD_richText));
	m_keywordMap.insert(std::make_pair("W:right", KEYWORD_right));
	m_keywordMap.insert(std::make_pair("W:rPr", KEYWORD_rPr));
	m_keywordMap.insert(std::make_pair("W:rPrChange", KEYWORD_rPrChange));
	m_keywordMap.insert(std::make_pair("W:rPrDefault", KEYWORD_rPrDefault));
	m_keywordMap.insert(std::make_pair("W:rsid", KEYWORD_rsid));
	m_keywordMap.insert(std::make_pair("W:rsidRoot", KEYWORD_rsidRoot));
	m_keywordMap.insert(std::make_pair("W:rsids", KEYWORD_rsids));
	m_keywordMap.insert(std::make_pair("W:rStyle", KEYWORD_rStyle));
	m_keywordMap.insert(std::make_pair("W:rt", KEYWORD_rt));
	m_keywordMap.insert(std::make_pair("W:rtl", KEYWORD_rtl));
	m_keywordMap.insert(std::make_pair("W:rtlGutter", KEYWORD_rtlGutter));
	m_keywordMap.insert(std::make_pair("W:ruby", KEYWORD_ruby));
	m_keywordMap.insert(std::make_pair("W:rubyAlign", KEYWORD_rubyAlign));
	m_keywordMap.insert(std::make_pair("W:rubyBase", KEYWORD_rubyBase));
	m_keywordMap.insert(std::make_pair("W:rubyPr", KEYWORD_rubyPr));
	m_keywordMap.insert(std::make_pair("W:saveFormsData", KEYWORD_saveFormsData));
	m_keywordMap.insert(std::make_pair("W:saveInvalidXml", KEYWORD_saveInvalidXml));
	m_keywordMap.insert(std::make_pair("W:savePreviewPicture", KEYWORD_savePreviewPicture));
	m_keywordMap.insert(std::make_pair("W:saveSmartTagsAsXml", KEYWORD_saveSmartTagsAsXml));
	m_keywordMap.insert(std::make_pair("W:saveSubsetFonts", KEYWORD_saveSubsetFonts));
	m_keywordMap.insert(std::make_pair("W:saveThroughXslt", KEYWORD_saveThroughXslt));
	m_keywordMap.insert(std::make_pair("W:saveXmlDataOnly", KEYWORD_saveXmlDataOnly));
	m_keywordMap.insert(std::make_pair("W:scrollbar", KEYWORD_scrollbar));
	m_keywordMap.insert(std::make_pair("W:sdt", KEYWORD_sdt));
	m_keywordMap.insert(std::make_pair("W:sdtContent", KEYWORD_sdtContent));
	m_keywordMap.insert(std::make_pair("W:sdtEndPr", KEYWORD_sdtEndPr));
	m_keywordMap.insert(std::make_pair("W:sdtPr", KEYWORD_sdtPr));
	m_keywordMap.insert(std::make_pair("W:sectPr", KEYWORD_sectPr));
	m_keywordMap.insert(std::make_pair("W:sectPrChange", KEYWORD_sectPrChange));
	m_keywordMap.insert(std::make_pair("W:selectFldWithFirstOrLastChar", KEYWORD_selectFldWithFirstOrLastChar));
	m_keywordMap.insert(std::make_pair("W:semiHidden", KEYWORD_semiHidden));
	m_keywordMap.insert(std::make_pair("W:separator", KEYWORD_separator));
	m_keywordMap.insert(std::make_pair("W:settings", KEYWORD_settings));
	m_keywordMap.insert(std::make_pair("W:shadow", KEYWORD_shadow));
	m_keywordMap.insert(std::make_pair("W:shapeDefaults", KEYWORD_shapeDefaults));
	m_keywordMap.insert(std::make_pair("W:shapeLayoutLikeWW8", KEYWORD_shapeLayoutLikeWW8));
	m_keywordMap.insert(std::make_pair("W:shd", KEYWORD_shd));
	m_keywordMap.insert(std::make_pair("W:showBreaksInFrames", KEYWORD_showBreaksInFrames));
	m_keywordMap.insert(std::make_pair("W:showEnvelope", KEYWORD_showEnvelope));
	m_keywordMap.insert(std::make_pair("W:showingPlcHdr", KEYWORD_showingPlcHdr));
	m_keywordMap.insert(std::make_pair("W:showXMLTags", KEYWORD_showXMLTags));
	m_keywordMap.insert(std::make_pair("W:sig", KEYWORD_sig));
	m_keywordMap.insert(std::make_pair("W:size", KEYWORD_size));
	m_keywordMap.insert(std::make_pair("W:sizeAuto", KEYWORD_sizeAuto));
	m_keywordMap.insert(std::make_pair("W:smallCaps", KEYWORD_smallCaps));
	m_keywordMap.insert(std::make_pair("W:smartTag", KEYWORD_smartTag));
	m_keywordMap.insert(std::make_pair("W:smartTagPr", KEYWORD_smartTagPr));
	m_keywordMap.insert(std::make_pair("W:smartTagType", KEYWORD_smartTagType));
	m_keywordMap.insert(std::make_pair("W:snapToGrid", KEYWORD_snapToGrid));
	m_keywordMap.insert(std::make_pair("W:softHyphen", KEYWORD_softHyphen));
	m_keywordMap.insert(std::make_pair("W:sourceFileName", KEYWORD_sourceFileName));
	m_keywordMap.insert(std::make_pair("W:spaceForUL", KEYWORD_spaceForUL));
	m_keywordMap.insert(std::make_pair("W:spacing", KEYWORD_spacing));
	m_keywordMap.insert(std::make_pair("W:spacingInWholePoints", KEYWORD_spacingInWholePoints));
	m_keywordMap.insert(std::make_pair("W:specVanish", KEYWORD_specVanish));
	m_keywordMap.insert(std::make_pair("W:splitPgBreakAndParaMark", KEYWORD_splitPgBreakAndParaMark));
	m_keywordMap.insert(std::make_pair("PIC:spPr", KEYWORD_spPr));
	m_keywordMap.insert(std::make_pair("W:src", KEYWORD_src));
	m_keywordMap.insert(std::make_pair("W:start", KEYWORD_start));
	m_keywordMap.insert(std::make_pair("W:startOverride", KEYWORD_startOverride));
	m_keywordMap.insert(std::make_pair("W:statusText", KEYWORD_statusText));
	m_keywordMap.insert(std::make_pair("W:storeMappedDataAs", KEYWORD_storeMappedDataAs));
	m_keywordMap.insert(std::make_pair("W:strictFirstAndLastChars", KEYWORD_strictFirstAndLastChars));
	m_keywordMap.insert(std::make_pair("W:strike", KEYWORD_strike));
	m_keywordMap.insert(std::make_pair("W:style", KEYWORD_style));
	m_keywordMap.insert(std::make_pair("W:styleLink", KEYWORD_styleLink));
	m_keywordMap.insert(std::make_pair("W:styleLockQFset", KEYWORD_styleLockQFset));
	m_keywordMap.insert(std::make_pair("W:styleLockTheme", KEYWORD_styleLockTheme));
	m_keywordMap.insert(std::make_pair("W:stylePaneFormatFilter", KEYWORD_stylePaneFormatFilter));
	m_keywordMap.insert(std::make_pair("W:stylePaneSortMethod", KEYWORD_stylePaneSortMethod));
	m_keywordMap.insert(std::make_pair("W:styles", KEYWORD_styles));
	m_keywordMap.insert(std::make_pair("W:subDoc", KEYWORD_subDoc));
	m_keywordMap.insert(std::make_pair("W:subFontBySize", KEYWORD_subFontBySize));
	m_keywordMap.insert(std::make_pair("W:suff", KEYWORD_suff));
	m_keywordMap.insert(std::make_pair("W:summaryLength", KEYWORD_summaryLength));
	m_keywordMap.insert(std::make_pair("W:suppressAutoHypens", KEYWORD_suppressAutoHypens));
	m_keywordMap.insert(std::make_pair("W:suppressBottomSpacing", KEYWORD_suppressBottomSpacing));
	m_keywordMap.insert(std::make_pair("W:suppressLineNumbers", KEYWORD_suppressLineNumbers));
	m_keywordMap.insert(std::make_pair("W:suppressOverlap", KEYWORD_suppressOverlap));
	m_keywordMap.insert(std::make_pair("W:suppressSpacingAtTopOfPage", KEYWORD_suppressSpacingAtTopOfPage));
	m_keywordMap.insert(std::make_pair("W:suppressSpBfAfterPgBrk", KEYWORD_suppressSpBfAfterPgBrk));
	m_keywordMap.insert(std::make_pair("W:suppressTopSpacing", KEYWORD_suppressTopSpacing));
	m_keywordMap.insert(std::make_pair("W:suppressTopSpacingWP", KEYWORD_suppressTopSpacingWP));
	m_keywordMap.insert(std::make_pair("W:swapBordersFacingPages", KEYWORD_swapBordersFacingPages));
	m_keywordMap.insert(std::make_pair("W:sym", KEYWORD_sym));
	m_keywordMap.insert(std::make_pair("W:sz", KEYWORD_sz));
	m_keywordMap.insert(std::make_pair("W:szCs", KEYWORD_szCs));
	m_keywordMap.insert(std::make_pair("W:t", KEYWORD_t));
	m_keywordMap.insert(std::make_pair("W:tab", KEYWORD_tab));
	m_keywordMap.insert(std::make_pair("W:table", KEYWORD_table));
	m_keywordMap.insert(std::make_pair("W:tabs", KEYWORD_tabs));
	m_keywordMap.insert(std::make_pair("W:tag", KEYWORD_tag));
	m_keywordMap.insert(std::make_pair("W:targetScreenSz", KEYWORD_targetScreenSz));
	m_keywordMap.insert(std::make_pair("W:tbl", KEYWORD_tbl));
	m_keywordMap.insert(std::make_pair("W:tblBorders", KEYWORD_tblBorders));
	m_keywordMap.insert(std::make_pair("W:tblCellMar", KEYWORD_tblCellMar));
	m_keywordMap.insert(std::make_pair("W:tblCellSpacing", KEYWORD_tblCellSpacing));
	m_keywordMap.insert(std::make_pair("W:tblGrid", KEYWORD_tblGrid));
	m_keywordMap.insert(std::make_pair("W:tblGridChange", KEYWORD_tblGridChange));
	m_keywordMap.insert(std::make_pair("W:tblHeader", KEYWORD_tblHeader));
	m_keywordMap.insert(std::make_pair("W:tblInd", KEYWORD_tblInd));
	m_keywordMap.insert(std::make_pair("W:tblLayout", KEYWORD_tblLayout));
	m_keywordMap.insert(std::make_pair("W:tblLook", KEYWORD_tblLook));
	m_keywordMap.insert(std::make_pair("W:tblOverlap", KEYWORD_tblOverlap));
	m_keywordMap.insert(std::make_pair("W:tblpPr", KEYWORD_tblpPr));
	m_keywordMap.insert(std::make_pair("W:tblPr", KEYWORD_tblPr));
	m_keywordMap.insert(std::make_pair("W:tblPrChange", KEYWORD_tblPrChange));
	m_keywordMap.insert(std::make_pair("W:tblPrEx", KEYWORD_tblPrEx));
	m_keywordMap.insert(std::make_pair("W:tblPrExChange", KEYWORD_tblPrExChange));
	m_keywordMap.insert(std::make_pair("W:tblStyle", KEYWORD_tblStyle));
	m_keywordMap.insert(std::make_pair("W:tblStylePr", KEYWORD_tblStylePr));
	m_keywordMap.insert(std::make_pair("W:tblStyleColBandSize", KEYWORD_tblStyleColBandSize));
	m_keywordMap.insert(std::make_pair("W:tblStyleRowBandSize", KEYWORD_tblStyleRowBandSize));
	m_keywordMap.insert(std::make_pair("W:tblW", KEYWORD_tblW));
	m_keywordMap.insert(std::make_pair("W:tc", KEYWORD_tc));
	m_keywordMap.insert(std::make_pair("W:tcBorders", KEYWORD_tcBorders));
	m_keywordMap.insert(std::make_pair("W:tcFitText", KEYWORD_tcFitText));
	m_keywordMap.insert(std::make_pair("W:tcMar", KEYWORD_tcMar));
	m_keywordMap.insert(std::make_pair("W:tcPr", KEYWORD_tcPr));
	m_keywordMap.insert(std::make_pair("W:tcPrChange", KEYWORD_tcPrChange));
	m_keywordMap.insert(std::make_pair("W:tcW", KEYWORD_tcW));
	m_keywordMap.insert(std::make_pair("W:temporary", KEYWORD_temporary));
	m_keywordMap.insert(std::make_pair("W:text", KEYWORD_text));
	m_keywordMap.insert(std::make_pair("W:textAlignment", KEYWORD_textAlignment));
	m_keywordMap.insert(std::make_pair("W:textboxTightWrap", KEYWORD_textboxTightWrap));
	m_keywordMap.insert(std::make_pair("W:textDirection", KEYWORD_textDirection));
	m_keywordMap.insert(std::make_pair("W:textInput", KEYWORD_textInput));
	m_keywordMap.insert(std::make_pair("W:themeFontLang", KEYWORD_themeFontLang));
	m_keywordMap.insert(std::make_pair("W:titlePg", KEYWORD_titlePg));
	m_keywordMap.insert(std::make_pair("W:tl2br", KEYWORD_tl2br));
	m_keywordMap.insert(std::make_pair("W:tmpl", KEYWORD_tmpl));
	m_keywordMap.insert(std::make_pair("W:top", KEYWORD_top));
	m_keywordMap.insert(std::make_pair("W:topLinePunct", KEYWORD_topLinePunct));
	m_keywordMap.insert(std::make_pair("W:tr", KEYWORD_tr));
	m_keywordMap.insert(std::make_pair("W:trackRevisions", KEYWORD_trackRevisions));
	m_keywordMap.insert(std::make_pair("W:tr2bl", KEYWORD_tr2bl));
	m_keywordMap.insert(std::make_pair("W:trHeight", KEYWORD_trHeight));
	m_keywordMap.insert(std::make_pair("W:trPr", KEYWORD_trPr));
	m_keywordMap.insert(std::make_pair("W:trPrChange", KEYWORD_trPrChange));
	m_keywordMap.insert(std::make_pair("W:truncateFontHeightsLikeWP6", KEYWORD_truncateFontHeightsLikeWP6));
	m_keywordMap.insert(std::make_pair("W:txbxContent", KEYWORD_txbxContent));
	m_keywordMap.insert(std::make_pair("W:type", KEYWORD_type));
	m_keywordMap.insert(std::make_pair("W:types", KEYWORD_types));
	m_keywordMap.insert(std::make_pair("W:u", KEYWORD_u));
	m_keywordMap.insert(std::make_pair("W:udl", KEYWORD_udl));
	m_keywordMap.insert(std::make_pair("W:uiCompat97To2003", KEYWORD_uiCompat97To2003));
	m_keywordMap.insert(std::make_pair("W:uiPriority", KEYWORD_uiPriority));
	m_keywordMap.insert(std::make_pair("W:ulTrailSpace", KEYWORD_ulTrailSpace));
	m_keywordMap.insert(std::make_pair("W:underlineTabInNumList", KEYWORD_underlineTabInNumList));
	m_keywordMap.insert(std::make_pair("W:unhideWhenUsed", KEYWORD_unhideWhenUsed));
	m_keywordMap.insert(std::make_pair("W:uniqueTag", KEYWORD_uniqueTag));
	m_keywordMap.insert(std::make_pair("W:updateFields", KEYWORD_updateFields));
	m_keywordMap.insert(std::make_pair("W:useAltKinsokuLineBreakRules", KEYWORD_useAltKinsokuLineBreakRules));
	m_keywordMap.insert(std::make_pair("W:useAnsiKerningPairs", KEYWORD_useAnsiKerningPairs));
	m_keywordMap.insert(std::make_pair("W:useFELayout", KEYWORD_useFELayout));
	m_keywordMap.insert(std::make_pair("W:useNormalStyleForList", KEYWORD_useNormalStyleForList));
	m_keywordMap.insert(std::make_pair("W:usePrinterMetrics", KEYWORD_usePrinterMetrics));
	m_keywordMap.insert(std::make_pair("W:useSingleBorderforContiguousCells", KEYWORD_useSingleBorderforContiguousCells));
	m_keywordMap.insert(std::make_pair("W:useWord2002TableStyleRules", KEYWORD_useWord2002TableStyleRules));
	m_keywordMap.insert(std::make_pair("W:useWord97LineBreakRules", KEYWORD_useWord97LineBreakRules));
	m_keywordMap.insert(std::make_pair("W:useXSLTWhenSaving", KEYWORD_useXSLTWhenSaving));
	m_keywordMap.insert(std::make_pair("W:vAlign", KEYWORD_vAlign));
	m_keywordMap.insert(std::make_pair("W:vanish", KEYWORD_vanish));
	m_keywordMap.insert(std::make_pair("W:vertAlign", KEYWORD_vertAlign));
	m_keywordMap.insert(std::make_pair("W:view", KEYWORD_view));
	m_keywordMap.insert(std::make_pair("W:viewMergedData", KEYWORD_viewMergedData));
	m_keywordMap.insert(std::make_pair("W:vMerge", KEYWORD_vMerge));
	m_keywordMap.insert(std::make_pair("W:yearLong", KEYWORD_yearLong));
	m_keywordMap.insert(std::make_pair("W:yearShort", KEYWORD_yearShort));
	m_keywordMap.insert(std::make_pair("W:w", KEYWORD_w));
	m_keywordMap.insert(std::make_pair("W:wAfter", KEYWORD_wAfter));
	m_keywordMap.insert(std::make_pair("W:wBefore", KEYWORD_wBefore));
	m_keywordMap.insert(std::make_pair("W:webHidden", KEYWORD_webHidden));
	m_keywordMap.insert(std::make_pair("W:webSettings", KEYWORD_webSettings));
	m_keywordMap.insert(std::make_pair("W:widowControl", KEYWORD_widowControl));
	m_keywordMap.insert(std::make_pair("W:wordWrap", KEYWORD_wordWrap));
	m_keywordMap.insert(std::make_pair("W:wpJustification", KEYWORD_wpJustification));
	m_keywordMap.insert(std::make_pair("W:wpSpaceWidth", KEYWORD_wpSpaceWidth));
	m_keywordMap.insert(std::make_pair("W:wrapTrailSpaces", KEYWORD_wrapTrailSpaces));
	m_keywordMap.insert(std::make_pair("W:writeProtection", KEYWORD_writeProtection));
	m_keywordMap.insert(std::make_pair("W:zoom", KEYWORD_zoom));
	//TODO: add more here
}
