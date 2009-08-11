/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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

#ifndef _OXMLI_TYPES_H_
#define _OXMLI_TYPES_H_

// Internal includes
#include <OXML_Element.h>
#include <OXML_Section.h>

// External includes
#include <stack>
#include <vector>

typedef std::stack<OXML_SharedElement> OXMLi_ElementStack;
typedef std::stack<OXML_SharedSection> OXMLi_SectionStack;
typedef std::vector<std::string> OXMLi_ContextVector;

struct OXMLi_StartElementRequest
{
	std::string pName;
	std::map<std::string, std::string>* ppAtts;
	OXMLi_ElementStack * stck;
	OXMLi_SectionStack * sect_stck;	
	OXMLi_ContextVector * context;
	bool handled;
	bool valid;
};

struct OXMLi_EndElementRequest
{
	std::string pName;
	OXMLi_ElementStack * stck;
	OXMLi_SectionStack * sect_stck;	
	OXMLi_ContextVector * context;
	bool handled;
	bool valid;
};

struct OXMLi_CharDataRequest
{
	const gchar * buffer;
	int length;
	OXMLi_ElementStack * stck;
	OXMLi_ContextVector * context;
	bool handled;
	bool valid;
};

enum OXMLi_Keyword
{
	KEYWORD_adjustRightInd,
	KEYWORD_autoSpaceDE,
	KEYWORD_autoSpaceDN,
	KEYWORD_b,
	KEYWORD_background, 
	KEYWORD_bar,
	KEYWORD_bCs,
	KEYWORD_bdr,
	KEYWORD_between,
	KEYWORD_bidi,
	KEYWORD_bidiVisual,
	KEYWORD_body,
	KEYWORD_bottom,
	KEYWORD_break,
	KEYWORD_cantSplit,
	KEYWORD_caps,
	KEYWORD_cnfStyle,
	KEYWORD_color,
	KEYWORD_contextualSpacing,
	KEYWORD_control,
	KEYWORD_cr,
	KEYWORD_cs,
	KEYWORD_dayLong,
	KEYWORD_dayShort,
	KEYWORD_delText,
	KEYWORD_dirty,
	KEYWORD_divId,
	KEYWORD_document, 
	KEYWORD_drawing,
	KEYWORD_dstrike,
	KEYWORD_eastAsianLayout,
	KEYWORD_effect,
	KEYWORD_em,
	KEYWORD_emboss,
	KEYWORD_fitText,
	KEYWORD_framePr,
	KEYWORD_glossaryDocument,
	KEYWORD_gridAfter,
	KEYWORD_gridBefore,
	KEYWORD_gridCol,
	KEYWORD_gridSpan,
	KEYWORD_hidden,
	KEYWORD_hideMark,
	KEYWORD_highlight,
	KEYWORD_hMerge,
	KEYWORD_hps,
	KEYWORD_hpsBaseText,
	KEYWORD_hpsRaise,
	KEYWORD_i,
	KEYWORD_iCs,
	KEYWORD_imprint,
	KEYWORD_ind,
	KEYWORD_insideH,
	KEYWORD_insideV,
	KEYWORD_jc,
	KEYWORD_keepLines,
	KEYWORD_keepNext,
	KEYWORD_kern,
	KEYWORD_kinsoku,
	KEYWORD_lang,
	KEYWORD_lastRenderedPageBreak,
	KEYWORD_left,
	KEYWORD_lid,
	KEYWORD_mirrorIndents,
	KEYWORD_monthLong,
	KEYWORD_monthShort,
	KEYWORD_movie,
	KEYWORD_noBreakHyphen,
	KEYWORD_noProof,
	KEYWORD_noWrap,
	KEYWORD_numPr,
	KEYWORD_object,
	KEYWORD_oMath,
	KEYWORD_outline,
	KEYWORD_outlineLvl,
	KEYWORD_overflowPunct,
	KEYWORD_p,
	KEYWORD_pageBreakBefore,
	KEYWORD_pBdr,
	KEYWORD_pgNum,
	KEYWORD_pict,
	KEYWORD_pPr,
	KEYWORD_position,
	KEYWORD_pStyle,
	KEYWORD_ptab,
	KEYWORD_right,
	KEYWORD_r,
	KEYWORD_rFonts,
	KEYWORD_rPr,
	KEYWORD_rStyle,
	KEYWORD_rt,
	KEYWORD_rtl,
	KEYWORD_ruby,
	KEYWORD_rubyAlign,
	KEYWORD_rubyBase,
	KEYWORD_rubyPr,
	KEYWORD_shadow,
	KEYWORD_shd,
	KEYWORD_smallCaps,
	KEYWORD_snapToGrid,
	KEYWORD_softHyphen,
	KEYWORD_spacing,
	KEYWORD_specVanish,
	KEYWORD_strike,
	KEYWORD_suppressAutoHypens,
	KEYWORD_suppressLineNumbers,
	KEYWORD_suppressOverlap,
	KEYWORD_sym,
	KEYWORD_sz,
	KEYWORD_szCs,
	KEYWORD_t,
	KEYWORD_tab,
	KEYWORD_tabs,
	KEYWORD_tbl,
	KEYWORD_tblBorders,
	KEYWORD_tblCellMar,
	KEYWORD_tblCellSpacing,
	KEYWORD_tblGrid,
	KEYWORD_tblHeader,
	KEYWORD_tblInd,
	KEYWORD_tblLayout,
	KEYWORD_tblLook,
	KEYWORD_tblOverlap,
	KEYWORD_tblpPr,
	KEYWORD_tblPr,
	KEYWORD_tblPrEx,
	KEYWORD_tblStyle,
	KEYWORD_tblW,
	KEYWORD_tc,
	KEYWORD_tcBorders,
	KEYWORD_tcFitText,
	KEYWORD_tcMar,
	KEYWORD_tcPr,
	KEYWORD_tcW,
	KEYWORD_textAlignment,
	KEYWORD_textboxTightWrap,
	KEYWORD_textDirection,
	KEYWORD_tl2br,
	KEYWORD_top,
	KEYWORD_topLinePunct,
	KEYWORD_tr,
	KEYWORD_tr2bl,
	KEYWORD_trHeight,
	KEYWORD_trPr,
	KEYWORD_u,
	KEYWORD_vAlign,
	KEYWORD_vanish,
	KEYWORD_vertAlign,
	KEYWORD_vMerge,
	KEYWORD_yearLong,
	KEYWORD_yearShort,
	KEYWORD_w,
	KEYWORD_wAfter,
	KEYWORD_wBefore,
	KEYWORD_webHidden,
	KEYWORD_widowControl,
	KEYWORD_wordWrap
};

#endif //_OXMLI_TYPES_H_

