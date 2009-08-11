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
						  contextMatches(contextTag, NS_W_KEY, "pBdr") ||
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
						  contextMatches(contextTag, NS_W_KEY, "pBdr") || 
						  contextMatches(contextTag, NS_W_KEY, "tcBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "tcMar") ||
						  contextMatches(contextTag, NS_W_KEY, "tblCellMar") ||
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
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
			break;
		}
		case KEYWORD_top:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "top") || 
						  contextMatches(contextTag, NS_W_KEY, "pBdr") ||
						  contextMatches(contextTag, NS_W_KEY, "tblBorders") ||
						  contextMatches(contextTag, NS_W_KEY, "tblCellMar") ||
						  contextMatches(contextTag, NS_W_KEY, "tcMar") ||
						  contextMatches(contextTag, NS_W_KEY, "tcBorders");
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
						  contextMatches(contextTag, NS_W_KEY, "rubyPr");
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
						  contextMatches(contextTag, NS_W_KEY, "trPr");
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
						  contextMatches(contextTag, NS_W_KEY, "trPrChange");
			break;
		}
		case KEYWORD_vAlign:
		{
			rqst->valid = nameMatches(rqst->pName, NS_W_KEY, "vAlign") || 
						  contextMatches(contextTag, NS_W_KEY, "tcPr");
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
	m_keywordMap.insert(std::make_pair("W:bidiVisual", KEYWORD_bidiVisual));
	m_keywordMap.insert(std::make_pair("W:body", KEYWORD_body));
	m_keywordMap.insert(std::make_pair("W:bottom", KEYWORD_bottom));
	m_keywordMap.insert(std::make_pair("W:break", KEYWORD_break));
	m_keywordMap.insert(std::make_pair("W:cantSplit", KEYWORD_cantSplit));
	m_keywordMap.insert(std::make_pair("W:caps", KEYWORD_caps));
	m_keywordMap.insert(std::make_pair("W:cnfStyle", KEYWORD_cnfStyle));
	m_keywordMap.insert(std::make_pair("W:contextualSpacing", KEYWORD_contextualSpacing));
	m_keywordMap.insert(std::make_pair("W:color", KEYWORD_color));
	m_keywordMap.insert(std::make_pair("W:control", KEYWORD_control));
	m_keywordMap.insert(std::make_pair("W:cr", KEYWORD_cr));
	m_keywordMap.insert(std::make_pair("W:cs", KEYWORD_cs));
	m_keywordMap.insert(std::make_pair("W:dayLong", KEYWORD_dayLong));
	m_keywordMap.insert(std::make_pair("W:dayShort", KEYWORD_dayShort));
	m_keywordMap.insert(std::make_pair("W:delText", KEYWORD_delText));
	m_keywordMap.insert(std::make_pair("W:dirty", KEYWORD_dirty));
	m_keywordMap.insert(std::make_pair("W:divId", KEYWORD_divId));
	m_keywordMap.insert(std::make_pair("W:document", KEYWORD_document));
	m_keywordMap.insert(std::make_pair("W:drawing", KEYWORD_drawing));
	m_keywordMap.insert(std::make_pair("W:dstrike", KEYWORD_dstrike));
	m_keywordMap.insert(std::make_pair("W:eastAsianLayout", KEYWORD_eastAsianLayout));
	m_keywordMap.insert(std::make_pair("W:effect", KEYWORD_effect));
	m_keywordMap.insert(std::make_pair("W:em", KEYWORD_em));
	m_keywordMap.insert(std::make_pair("W:emboss", KEYWORD_emboss));
	m_keywordMap.insert(std::make_pair("W:fitText", KEYWORD_fitText));
	m_keywordMap.insert(std::make_pair("W:framePr", KEYWORD_framePr));
	m_keywordMap.insert(std::make_pair("W:glossaryDocument", KEYWORD_glossaryDocument));
	m_keywordMap.insert(std::make_pair("W:gridAfter", KEYWORD_gridAfter));
	m_keywordMap.insert(std::make_pair("W:gridBefore", KEYWORD_gridBefore));
	m_keywordMap.insert(std::make_pair("W:gridCol", KEYWORD_gridCol));
	m_keywordMap.insert(std::make_pair("W:gridSpan", KEYWORD_gridSpan));
	m_keywordMap.insert(std::make_pair("W:hidden", KEYWORD_hidden));
	m_keywordMap.insert(std::make_pair("W:hideMark", KEYWORD_hideMark));
	m_keywordMap.insert(std::make_pair("W:highlight", KEYWORD_highlight));
	m_keywordMap.insert(std::make_pair("W:hMerge", KEYWORD_hMerge));
	m_keywordMap.insert(std::make_pair("W:hps", KEYWORD_hps));
	m_keywordMap.insert(std::make_pair("W:hpsBaseText", KEYWORD_hpsBaseText));
	m_keywordMap.insert(std::make_pair("W:hpsRaise", KEYWORD_hpsRaise));
	m_keywordMap.insert(std::make_pair("W:i", KEYWORD_i));
	m_keywordMap.insert(std::make_pair("W:iCs", KEYWORD_iCs));
	m_keywordMap.insert(std::make_pair("W:imprint", KEYWORD_imprint));
	m_keywordMap.insert(std::make_pair("W:ind", KEYWORD_ind));
	m_keywordMap.insert(std::make_pair("W:insideH", KEYWORD_insideH));
	m_keywordMap.insert(std::make_pair("W:insideV", KEYWORD_insideV));
	m_keywordMap.insert(std::make_pair("W:jc", KEYWORD_jc));
	m_keywordMap.insert(std::make_pair("W:keepLines", KEYWORD_keepLines));
	m_keywordMap.insert(std::make_pair("W:keepNext", KEYWORD_keepNext));
	m_keywordMap.insert(std::make_pair("W:kern", KEYWORD_kern));
	m_keywordMap.insert(std::make_pair("W:kinsoku", KEYWORD_kinsoku));
	m_keywordMap.insert(std::make_pair("W:lang", KEYWORD_lang));
	m_keywordMap.insert(std::make_pair("W:lastRenderedPageBreak", KEYWORD_lastRenderedPageBreak));
	m_keywordMap.insert(std::make_pair("W:left", KEYWORD_left));
	m_keywordMap.insert(std::make_pair("W:lid", KEYWORD_lid));
	m_keywordMap.insert(std::make_pair("W:mirrorIndents", KEYWORD_mirrorIndents));
	m_keywordMap.insert(std::make_pair("W:monthLong", KEYWORD_monthLong));
	m_keywordMap.insert(std::make_pair("W:monthShort", KEYWORD_monthShort));
	m_keywordMap.insert(std::make_pair("W:movie", KEYWORD_movie));
	m_keywordMap.insert(std::make_pair("W:noBreakHyphen", KEYWORD_noBreakHyphen));
	m_keywordMap.insert(std::make_pair("W:noProof", KEYWORD_noProof));
	m_keywordMap.insert(std::make_pair("W:noWrap", KEYWORD_noWrap));
	m_keywordMap.insert(std::make_pair("W:numPr", KEYWORD_numPr));
	m_keywordMap.insert(std::make_pair("W:object", KEYWORD_object));
	m_keywordMap.insert(std::make_pair("W:oMath", KEYWORD_oMath));
	m_keywordMap.insert(std::make_pair("W:outline", KEYWORD_outline));
	m_keywordMap.insert(std::make_pair("W:outlineLvl", KEYWORD_outlineLvl));
	m_keywordMap.insert(std::make_pair("W:overflowPunct", KEYWORD_overflowPunct));
	m_keywordMap.insert(std::make_pair("W:p", KEYWORD_p));
	m_keywordMap.insert(std::make_pair("W:pageBreakBefore", KEYWORD_pageBreakBefore));
	m_keywordMap.insert(std::make_pair("W:pBdr", KEYWORD_pBdr));
	m_keywordMap.insert(std::make_pair("W:pgNum", KEYWORD_pgNum));
	m_keywordMap.insert(std::make_pair("W:pict", KEYWORD_pict));
	m_keywordMap.insert(std::make_pair("W:position", KEYWORD_position));
	m_keywordMap.insert(std::make_pair("W:pPr", KEYWORD_pPr));
	m_keywordMap.insert(std::make_pair("W:pStyle", KEYWORD_pStyle));
	m_keywordMap.insert(std::make_pair("W:ptab", KEYWORD_ptab));
	m_keywordMap.insert(std::make_pair("W:r", KEYWORD_r));
	m_keywordMap.insert(std::make_pair("W:rFonts", KEYWORD_rFonts));
	m_keywordMap.insert(std::make_pair("W:right", KEYWORD_right));
	m_keywordMap.insert(std::make_pair("W:rPr", KEYWORD_rPr));
	m_keywordMap.insert(std::make_pair("W:rStyle", KEYWORD_rStyle));
	m_keywordMap.insert(std::make_pair("W:rt", KEYWORD_rt));
	m_keywordMap.insert(std::make_pair("W:rtl", KEYWORD_rtl));
	m_keywordMap.insert(std::make_pair("W:ruby", KEYWORD_ruby));
	m_keywordMap.insert(std::make_pair("W:rubyAlign", KEYWORD_rubyAlign));
	m_keywordMap.insert(std::make_pair("W:rubyBase", KEYWORD_rubyBase));
	m_keywordMap.insert(std::make_pair("W:rubyPr", KEYWORD_rubyPr));
	m_keywordMap.insert(std::make_pair("W:shadow", KEYWORD_shadow));
	m_keywordMap.insert(std::make_pair("W:shd", KEYWORD_shd));
	m_keywordMap.insert(std::make_pair("W:smallCaps", KEYWORD_smallCaps));
	m_keywordMap.insert(std::make_pair("W:snapToGrid", KEYWORD_snapToGrid));
	m_keywordMap.insert(std::make_pair("W:softHyphen", KEYWORD_softHyphen));
	m_keywordMap.insert(std::make_pair("W:spacing", KEYWORD_spacing));
	m_keywordMap.insert(std::make_pair("W:specVanish", KEYWORD_specVanish));
	m_keywordMap.insert(std::make_pair("W:strike", KEYWORD_strike));
	m_keywordMap.insert(std::make_pair("W:suppressAutoHypens", KEYWORD_suppressAutoHypens));
	m_keywordMap.insert(std::make_pair("W:suppressLineNumbers", KEYWORD_suppressLineNumbers));
	m_keywordMap.insert(std::make_pair("W:suppressOverlap", KEYWORD_suppressOverlap));
	m_keywordMap.insert(std::make_pair("W:sym", KEYWORD_sym));
	m_keywordMap.insert(std::make_pair("W:sz", KEYWORD_sz));
	m_keywordMap.insert(std::make_pair("W:szCs", KEYWORD_szCs));
	m_keywordMap.insert(std::make_pair("W:t", KEYWORD_t));
	m_keywordMap.insert(std::make_pair("W:tab", KEYWORD_tab));
	m_keywordMap.insert(std::make_pair("W:tabs", KEYWORD_tabs));
	m_keywordMap.insert(std::make_pair("W:tbl", KEYWORD_tbl));
	m_keywordMap.insert(std::make_pair("W:tblBorders", KEYWORD_tblBorders));
	m_keywordMap.insert(std::make_pair("W:tblCellMar", KEYWORD_tblCellMar));
	m_keywordMap.insert(std::make_pair("W:tblCellSpacing", KEYWORD_tblCellSpacing));
	m_keywordMap.insert(std::make_pair("W:tblGrid", KEYWORD_tblGrid));
	m_keywordMap.insert(std::make_pair("W:tblHeader", KEYWORD_tblHeader));
	m_keywordMap.insert(std::make_pair("W:tblInd", KEYWORD_tblInd));
	m_keywordMap.insert(std::make_pair("W:tblLayout", KEYWORD_tblLayout));
	m_keywordMap.insert(std::make_pair("W:tblLook", KEYWORD_tblLook));
	m_keywordMap.insert(std::make_pair("W:tblOverlap", KEYWORD_tblOverlap));
	m_keywordMap.insert(std::make_pair("W:tblpPr", KEYWORD_tblpPr));
	m_keywordMap.insert(std::make_pair("W:tblPr", KEYWORD_tblPr));
	m_keywordMap.insert(std::make_pair("W:tblPrEx", KEYWORD_tblPrEx));
	m_keywordMap.insert(std::make_pair("W:tblStyle", KEYWORD_tblStyle));
	m_keywordMap.insert(std::make_pair("W:tblW", KEYWORD_tblW));
	m_keywordMap.insert(std::make_pair("W:tc", KEYWORD_tc));
	m_keywordMap.insert(std::make_pair("W:tcBorders", KEYWORD_tcBorders));
	m_keywordMap.insert(std::make_pair("W:tcFitText", KEYWORD_tcFitText));
	m_keywordMap.insert(std::make_pair("W:tcMar", KEYWORD_tcMar));
	m_keywordMap.insert(std::make_pair("W:tcPr", KEYWORD_tcPr));
	m_keywordMap.insert(std::make_pair("W:tcW", KEYWORD_tcW));
	m_keywordMap.insert(std::make_pair("W:textAlignment", KEYWORD_textAlignment));
	m_keywordMap.insert(std::make_pair("W:textboxTightWrap", KEYWORD_textboxTightWrap));
	m_keywordMap.insert(std::make_pair("W:textDirection", KEYWORD_textDirection));
	m_keywordMap.insert(std::make_pair("W:tl2br", KEYWORD_tl2br));
	m_keywordMap.insert(std::make_pair("W:top", KEYWORD_top));
	m_keywordMap.insert(std::make_pair("W:topLinePunct", KEYWORD_topLinePunct));
	m_keywordMap.insert(std::make_pair("W:tr", KEYWORD_tr));
	m_keywordMap.insert(std::make_pair("W:tr2bl", KEYWORD_tr2bl));
	m_keywordMap.insert(std::make_pair("W:trHeight", KEYWORD_trHeight));
	m_keywordMap.insert(std::make_pair("W:trPr", KEYWORD_trPr));
	m_keywordMap.insert(std::make_pair("W:u", KEYWORD_u));
	m_keywordMap.insert(std::make_pair("W:vAlign", KEYWORD_vAlign));
	m_keywordMap.insert(std::make_pair("W:vanish", KEYWORD_vanish));
	m_keywordMap.insert(std::make_pair("W:vertAlign", KEYWORD_vertAlign));
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
