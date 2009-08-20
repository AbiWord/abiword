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
						  contextMatches(contextTag, NS_W_KEY, "tblCellMar");
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
						  contextMatches(contextTag, NS_W_KEY, "tblBorders");
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
						  contextMatches(contextTag, NS_W_KEY, "tblBorders");
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
						  contextMatches(contextTag, NS_W_KEY, "pgBorders");
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
						  contextMatches(contextTag, NS_W_KEY, "odso");
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
						  contextMatches(contextTag, NS_W_KEY, "fieldMapData");
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
						  contextMatches(contextTag, NS_W_KEY, "style");
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
	m_keywordMap.insert(std::make_pair("W:addressFieldName", KEYWORD_addressFieldName));
	m_keywordMap.insert(std::make_pair("W:adjustRightInd", KEYWORD_adjustRightInd));
	m_keywordMap.insert(std::make_pair("W:alias", KEYWORD_alias));
	m_keywordMap.insert(std::make_pair("W:aliases", KEYWORD_aliases));
	m_keywordMap.insert(std::make_pair("W:altName", KEYWORD_altName));
	m_keywordMap.insert(std::make_pair("W:annotationRef", KEYWORD_annotationRef));
	m_keywordMap.insert(std::make_pair("W:attr", KEYWORD_attr));
	m_keywordMap.insert(std::make_pair("W:autoRedefine", KEYWORD_autoRedefine));
	m_keywordMap.insert(std::make_pair("W:autoSpaceDE", KEYWORD_autoSpaceDE));
	m_keywordMap.insert(std::make_pair("W:autoSpaceDN", KEYWORD_autoSpaceDN));
	m_keywordMap.insert(std::make_pair("W:b", KEYWORD_b));
	m_keywordMap.insert(std::make_pair("W:background", KEYWORD_background));
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
	m_keywordMap.insert(std::make_pair("W:body", KEYWORD_body));
	m_keywordMap.insert(std::make_pair("W:bookmarkEnd", KEYWORD_bookmarkEnd));
	m_keywordMap.insert(std::make_pair("W:bookmarkStart", KEYWORD_bookmarkStart));
	m_keywordMap.insert(std::make_pair("W:bottom", KEYWORD_bottom));
	m_keywordMap.insert(std::make_pair("W:break", KEYWORD_break));
	m_keywordMap.insert(std::make_pair("W:calendar", KEYWORD_calendar));
	m_keywordMap.insert(std::make_pair("W:cantSplit", KEYWORD_cantSplit));
	m_keywordMap.insert(std::make_pair("W:caps", KEYWORD_caps));
	m_keywordMap.insert(std::make_pair("W:category", KEYWORD_category));
	m_keywordMap.insert(std::make_pair("W:cellDel", KEYWORD_cellDel));
	m_keywordMap.insert(std::make_pair("W:cellIns", KEYWORD_cellIns));
	m_keywordMap.insert(std::make_pair("W:cellMerge", KEYWORD_cellMerge));
	m_keywordMap.insert(std::make_pair("W:charset", KEYWORD_charset));
	m_keywordMap.insert(std::make_pair("W:checkErrors", KEYWORD_checkErrors));
	m_keywordMap.insert(std::make_pair("W:citation", KEYWORD_citation));
	m_keywordMap.insert(std::make_pair("W:cnfStyle", KEYWORD_cnfStyle));
	m_keywordMap.insert(std::make_pair("W:colDelim", KEYWORD_colDelim));
	m_keywordMap.insert(std::make_pair("W:column", KEYWORD_column));
	m_keywordMap.insert(std::make_pair("W:comboBox", KEYWORD_comboBox));
	m_keywordMap.insert(std::make_pair("W:comment", KEYWORD_comment));
	m_keywordMap.insert(std::make_pair("W:commentRangeEnd", KEYWORD_commentRangeEnd));
	m_keywordMap.insert(std::make_pair("W:commentRangeStart", KEYWORD_commentRangeStart));
	m_keywordMap.insert(std::make_pair("W:commentReference", KEYWORD_commentReference));
	m_keywordMap.insert(std::make_pair("W:comments", KEYWORD_comments));
	m_keywordMap.insert(std::make_pair("W:connectString", KEYWORD_connectString));
	m_keywordMap.insert(std::make_pair("W:contextualSpacing", KEYWORD_contextualSpacing));
	m_keywordMap.insert(std::make_pair("W:continuationSeparator", KEYWORD_continuationSeparator));
	m_keywordMap.insert(std::make_pair("W:col", KEYWORD_col));
	m_keywordMap.insert(std::make_pair("W:color", KEYWORD_color));
	m_keywordMap.insert(std::make_pair("W:cols", KEYWORD_cols));
	m_keywordMap.insert(std::make_pair("W:control", KEYWORD_control));
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
	m_keywordMap.insert(std::make_pair("W:del", KEYWORD_del));
	m_keywordMap.insert(std::make_pair("W:delText", KEYWORD_delText));
	m_keywordMap.insert(std::make_pair("W:description", KEYWORD_description));
	m_keywordMap.insert(std::make_pair("W:destination", KEYWORD_destination));
	m_keywordMap.insert(std::make_pair("W:dirty", KEYWORD_dirty));
	m_keywordMap.insert(std::make_pair("W:divId", KEYWORD_divId));
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
	m_keywordMap.insert(std::make_pair("W:doNotSuppressBlankLines", KEYWORD_doNotSuppressBlankLines));
	m_keywordMap.insert(std::make_pair("W:drawing", KEYWORD_drawing));
	m_keywordMap.insert(std::make_pair("W:dropDownList", KEYWORD_dropDownList));
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
	m_keywordMap.insert(std::make_pair("W:endnote", KEYWORD_endnote));
	m_keywordMap.insert(std::make_pair("W:endnotePr", KEYWORD_endnotePr));
	m_keywordMap.insert(std::make_pair("W:endnoteRef", KEYWORD_endnoteRef));
	m_keywordMap.insert(std::make_pair("W:endnoteReference", KEYWORD_endnoteReference));
	m_keywordMap.insert(std::make_pair("W:endnotes", KEYWORD_endnotes));
	m_keywordMap.insert(std::make_pair("W:evenAndOddHeaders", KEYWORD_evenAndOddHeaders));
	m_keywordMap.insert(std::make_pair("W:family", KEYWORD_family));
	m_keywordMap.insert(std::make_pair("W:fHdr", KEYWORD_fHdr));
	m_keywordMap.insert(std::make_pair("W:fieldMapData", KEYWORD_fieldMapData));
	m_keywordMap.insert(std::make_pair("W:fitText", KEYWORD_fitText));
	m_keywordMap.insert(std::make_pair("W:font", KEYWORD_font));
	m_keywordMap.insert(std::make_pair("W:fonts", KEYWORD_fonts));
	m_keywordMap.insert(std::make_pair("W:footerReference", KEYWORD_footerReference));
	m_keywordMap.insert(std::make_pair("W:footnote", KEYWORD_footnote));
	m_keywordMap.insert(std::make_pair("W:footnotePr", KEYWORD_footnotePr));
	m_keywordMap.insert(std::make_pair("W:footnoteRef", KEYWORD_footnoteRef));
	m_keywordMap.insert(std::make_pair("W:footnoteReference", KEYWORD_footnoteReference));
	m_keywordMap.insert(std::make_pair("W:footnotes", KEYWORD_footnotes));
	m_keywordMap.insert(std::make_pair("W:formProt", KEYWORD_formProt));
	m_keywordMap.insert(std::make_pair("W:framePr", KEYWORD_framePr));
	m_keywordMap.insert(std::make_pair("W:ftr", KEYWORD_ftr));
	m_keywordMap.insert(std::make_pair("W:gallery", KEYWORD_gallery));
	m_keywordMap.insert(std::make_pair("W:glossaryDocument", KEYWORD_glossaryDocument));
	m_keywordMap.insert(std::make_pair("W:gridAfter", KEYWORD_gridAfter));
	m_keywordMap.insert(std::make_pair("W:gridBefore", KEYWORD_gridBefore));
	m_keywordMap.insert(std::make_pair("W:gridCol", KEYWORD_gridCol));
	m_keywordMap.insert(std::make_pair("W:gridSpan", KEYWORD_gridSpan));
	m_keywordMap.insert(std::make_pair("W:group", KEYWORD_group));
	m_keywordMap.insert(std::make_pair("W:guid", KEYWORD_guid));
	m_keywordMap.insert(std::make_pair("W:hdr", KEYWORD_hdr));
	m_keywordMap.insert(std::make_pair("W:headerReference", KEYWORD_headerReference));
	m_keywordMap.insert(std::make_pair("W:headerSource", KEYWORD_headerSource));
	m_keywordMap.insert(std::make_pair("W:hidden", KEYWORD_hidden));
	m_keywordMap.insert(std::make_pair("W:hideMark", KEYWORD_hideMark));
	m_keywordMap.insert(std::make_pair("W:highlight", KEYWORD_highlight));
	m_keywordMap.insert(std::make_pair("W:hMerge", KEYWORD_hMerge));
	m_keywordMap.insert(std::make_pair("W:hps", KEYWORD_hps));
	m_keywordMap.insert(std::make_pair("W:hpsBaseText", KEYWORD_hpsBaseText));
	m_keywordMap.insert(std::make_pair("W:hpsRaise", KEYWORD_hpsRaise));
	m_keywordMap.insert(std::make_pair("W:i", KEYWORD_i));
	m_keywordMap.insert(std::make_pair("W:iCs", KEYWORD_iCs));
	m_keywordMap.insert(std::make_pair("W:id", KEYWORD_id));
	m_keywordMap.insert(std::make_pair("W:imprint", KEYWORD_imprint));
	m_keywordMap.insert(std::make_pair("W:ind", KEYWORD_ind));
	m_keywordMap.insert(std::make_pair("W:ins", KEYWORD_ins));
	m_keywordMap.insert(std::make_pair("W:insideH", KEYWORD_insideH));
	m_keywordMap.insert(std::make_pair("W:insideV", KEYWORD_insideV));
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
	m_keywordMap.insert(std::make_pair("W:left", KEYWORD_left));
    m_keywordMap.insert(std::make_pair("W:legacy", KEYWORD_legacy));
	m_keywordMap.insert(std::make_pair("W:lid", KEYWORD_lid));
	m_keywordMap.insert(std::make_pair("W:link", KEYWORD_link));
	m_keywordMap.insert(std::make_pair("W:linkToQuery", KEYWORD_linkToQuery));
	m_keywordMap.insert(std::make_pair("W:listItem", KEYWORD_listItem));
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
	m_keywordMap.insert(std::make_pair("W:mirrorIndents", KEYWORD_mirrorIndents));
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
	m_keywordMap.insert(std::make_pair("W:name", KEYWORD_name));
	m_keywordMap.insert(std::make_pair("W:next", KEYWORD_next));
	m_keywordMap.insert(std::make_pair("W:noBreakHyphen", KEYWORD_noBreakHyphen));
	m_keywordMap.insert(std::make_pair("W:noEndnote", KEYWORD_noEndnote));
	m_keywordMap.insert(std::make_pair("W:noProof", KEYWORD_noProof));
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
	m_keywordMap.insert(std::make_pair("W:object", KEYWORD_object));
	m_keywordMap.insert(std::make_pair("W:odso", KEYWORD_odso));
	m_keywordMap.insert(std::make_pair("W:oMath", KEYWORD_oMath));
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
	m_keywordMap.insert(std::make_pair("W:pict", KEYWORD_pict));
	m_keywordMap.insert(std::make_pair("W:picture", KEYWORD_picture));
	m_keywordMap.insert(std::make_pair("W:pitch", KEYWORD_pitch));
	m_keywordMap.insert(std::make_pair("W:placeholder", KEYWORD_placeholder));
	m_keywordMap.insert(std::make_pair("W:pos", KEYWORD_pos));
	m_keywordMap.insert(std::make_pair("W:position", KEYWORD_position));
	m_keywordMap.insert(std::make_pair("W:printerSettings", KEYWORD_printerSettings));
	m_keywordMap.insert(std::make_pair("W:pPr", KEYWORD_pPr));
	m_keywordMap.insert(std::make_pair("W:pPrChange", KEYWORD_pPrChange));
	m_keywordMap.insert(std::make_pair("W:pPrDefault", KEYWORD_pPrDefault));
	m_keywordMap.insert(std::make_pair("W:proofErr", KEYWORD_proofErr));
	m_keywordMap.insert(std::make_pair("W:pStyle", KEYWORD_pStyle));
	m_keywordMap.insert(std::make_pair("W:ptab", KEYWORD_ptab));
	m_keywordMap.insert(std::make_pair("W:qFormat", KEYWORD_qFormat));
	m_keywordMap.insert(std::make_pair("W:query", KEYWORD_query));
	m_keywordMap.insert(std::make_pair("W:r", KEYWORD_r));
	m_keywordMap.insert(std::make_pair("W:recipientData", KEYWORD_recipientData));
	m_keywordMap.insert(std::make_pair("W:recipients", KEYWORD_recipients));
	m_keywordMap.insert(std::make_pair("W:rFonts", KEYWORD_rFonts));
	m_keywordMap.insert(std::make_pair("W:richText", KEYWORD_richText));
	m_keywordMap.insert(std::make_pair("W:right", KEYWORD_right));
	m_keywordMap.insert(std::make_pair("W:rPr", KEYWORD_rPr));
	m_keywordMap.insert(std::make_pair("W:rPrChange", KEYWORD_rPrChange));
	m_keywordMap.insert(std::make_pair("W:rPrDefault", KEYWORD_rPrDefault));
	m_keywordMap.insert(std::make_pair("W:rsid", KEYWORD_rsid));
	m_keywordMap.insert(std::make_pair("W:rStyle", KEYWORD_rStyle));
	m_keywordMap.insert(std::make_pair("W:rt", KEYWORD_rt));
	m_keywordMap.insert(std::make_pair("W:rtl", KEYWORD_rtl));
	m_keywordMap.insert(std::make_pair("W:rtlGutter", KEYWORD_rtlGutter));
	m_keywordMap.insert(std::make_pair("W:ruby", KEYWORD_ruby));
	m_keywordMap.insert(std::make_pair("W:rubyAlign", KEYWORD_rubyAlign));
	m_keywordMap.insert(std::make_pair("W:rubyBase", KEYWORD_rubyBase));
	m_keywordMap.insert(std::make_pair("W:rubyPr", KEYWORD_rubyPr));
	m_keywordMap.insert(std::make_pair("W:saveSubsetFonts", KEYWORD_saveSubsetFonts));
	m_keywordMap.insert(std::make_pair("W:sdt", KEYWORD_sdt));
	m_keywordMap.insert(std::make_pair("W:sdtContent", KEYWORD_sdtContent));
	m_keywordMap.insert(std::make_pair("W:sdtEndPr", KEYWORD_sdtEndPr));
	m_keywordMap.insert(std::make_pair("W:sdtPr", KEYWORD_sdtPr));
	m_keywordMap.insert(std::make_pair("W:sectPr", KEYWORD_sectPr));
	m_keywordMap.insert(std::make_pair("W:sectPrChange", KEYWORD_sectPrChange));
	m_keywordMap.insert(std::make_pair("W:semiHidden", KEYWORD_semiHidden));
	m_keywordMap.insert(std::make_pair("W:separator", KEYWORD_separator));
	m_keywordMap.insert(std::make_pair("W:shadow", KEYWORD_shadow));
	m_keywordMap.insert(std::make_pair("W:shd", KEYWORD_shd));
	m_keywordMap.insert(std::make_pair("W:showingPlcHdr", KEYWORD_showingPlcHdr));
	m_keywordMap.insert(std::make_pair("W:sig", KEYWORD_sig));
	m_keywordMap.insert(std::make_pair("W:smallCaps", KEYWORD_smallCaps));
	m_keywordMap.insert(std::make_pair("W:smartTag", KEYWORD_smartTag));
	m_keywordMap.insert(std::make_pair("W:smartTagPr", KEYWORD_smartTagPr));
	m_keywordMap.insert(std::make_pair("W:snapToGrid", KEYWORD_snapToGrid));
	m_keywordMap.insert(std::make_pair("W:softHyphen", KEYWORD_softHyphen));
	m_keywordMap.insert(std::make_pair("W:spacing", KEYWORD_spacing));
	m_keywordMap.insert(std::make_pair("W:specVanish", KEYWORD_specVanish));
	m_keywordMap.insert(std::make_pair("W:src", KEYWORD_src));
	m_keywordMap.insert(std::make_pair("W:start", KEYWORD_start));
	m_keywordMap.insert(std::make_pair("W:startOverride", KEYWORD_startOverride));
	m_keywordMap.insert(std::make_pair("W:storeMappedDataAs", KEYWORD_storeMappedDataAs));
	m_keywordMap.insert(std::make_pair("W:strike", KEYWORD_strike));
	m_keywordMap.insert(std::make_pair("W:style", KEYWORD_style));
	m_keywordMap.insert(std::make_pair("W:styleLink", KEYWORD_styleLink));
	m_keywordMap.insert(std::make_pair("W:styles", KEYWORD_styles));
	m_keywordMap.insert(std::make_pair("W:suff", KEYWORD_suff));
	m_keywordMap.insert(std::make_pair("W:suppressAutoHypens", KEYWORD_suppressAutoHypens));
	m_keywordMap.insert(std::make_pair("W:suppressLineNumbers", KEYWORD_suppressLineNumbers));
	m_keywordMap.insert(std::make_pair("W:suppressOverlap", KEYWORD_suppressOverlap));
	m_keywordMap.insert(std::make_pair("W:sym", KEYWORD_sym));
	m_keywordMap.insert(std::make_pair("W:sz", KEYWORD_sz));
	m_keywordMap.insert(std::make_pair("W:szCs", KEYWORD_szCs));
	m_keywordMap.insert(std::make_pair("W:t", KEYWORD_t));
	m_keywordMap.insert(std::make_pair("W:tab", KEYWORD_tab));
	m_keywordMap.insert(std::make_pair("W:table", KEYWORD_table));
	m_keywordMap.insert(std::make_pair("W:tabs", KEYWORD_tabs));
	m_keywordMap.insert(std::make_pair("W:tag", KEYWORD_tag));
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
	m_keywordMap.insert(std::make_pair("W:titlePg", KEYWORD_titlePg));
	m_keywordMap.insert(std::make_pair("W:tl2br", KEYWORD_tl2br));
	m_keywordMap.insert(std::make_pair("W:tmpl", KEYWORD_tmpl));
	m_keywordMap.insert(std::make_pair("W:top", KEYWORD_top));
	m_keywordMap.insert(std::make_pair("W:topLinePunct", KEYWORD_topLinePunct));
	m_keywordMap.insert(std::make_pair("W:tr", KEYWORD_tr));
	m_keywordMap.insert(std::make_pair("W:tr2bl", KEYWORD_tr2bl));
	m_keywordMap.insert(std::make_pair("W:trHeight", KEYWORD_trHeight));
	m_keywordMap.insert(std::make_pair("W:trPr", KEYWORD_trPr));
	m_keywordMap.insert(std::make_pair("W:trPrChange", KEYWORD_trPrChange));
	m_keywordMap.insert(std::make_pair("W:type", KEYWORD_type));
	m_keywordMap.insert(std::make_pair("W:types", KEYWORD_types));
	m_keywordMap.insert(std::make_pair("W:u", KEYWORD_u));
	m_keywordMap.insert(std::make_pair("W:udl", KEYWORD_udl));
	m_keywordMap.insert(std::make_pair("W:uiPriority", KEYWORD_uiPriority));
	m_keywordMap.insert(std::make_pair("W:unhideWhenUsed", KEYWORD_unhideWhenUsed));
	m_keywordMap.insert(std::make_pair("W:uniqueTag", KEYWORD_uniqueTag));
	m_keywordMap.insert(std::make_pair("W:vAlign", KEYWORD_vAlign));
	m_keywordMap.insert(std::make_pair("W:vanish", KEYWORD_vanish));
	m_keywordMap.insert(std::make_pair("W:vertAlign", KEYWORD_vertAlign));
	m_keywordMap.insert(std::make_pair("W:viewMergedData", KEYWORD_viewMergedData));
	m_keywordMap.insert(std::make_pair("W:vMerge", KEYWORD_vMerge));
	m_keywordMap.insert(std::make_pair("W:yearLong", KEYWORD_yearLong));
	m_keywordMap.insert(std::make_pair("W:yearShort", KEYWORD_yearShort));
	m_keywordMap.insert(std::make_pair("W:w", KEYWORD_w));
	m_keywordMap.insert(std::make_pair("W:wAfter", KEYWORD_wAfter));
	m_keywordMap.insert(std::make_pair("W:wBefore", KEYWORD_wBefore));
	m_keywordMap.insert(std::make_pair("W:webHidden", KEYWORD_webHidden));
	m_keywordMap.insert(std::make_pair("W:widowControl", KEYWORD_widowControl));
	m_keywordMap.insert(std::make_pair("W:wordWrap", KEYWORD_wordWrap));
	//TODO: add more here
}
