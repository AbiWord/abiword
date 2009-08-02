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
	KEYWORD_background, 
	KEYWORD_bar,
	KEYWORD_between,
	KEYWORD_bidi,
	KEYWORD_body,
	KEYWORD_bottom,
	KEYWORD_cnfStyle,
	KEYWORD_contextualSpacing,
	KEYWORD_divId,
	KEYWORD_document, 
	KEYWORD_framePr,
	KEYWORD_glossaryDocument,
	KEYWORD_ind,
	KEYWORD_jc,
	KEYWORD_keepLines,
	KEYWORD_keepNext,
	KEYWORD_kinsoku,
	KEYWORD_left,
	KEYWORD_mirrorIndents,
	KEYWORD_numPr,
	KEYWORD_outlineLvl,
	KEYWORD_overflowPunct,
	KEYWORD_p,
	KEYWORD_pageBreakBefore,
	KEYWORD_pBdr,
	KEYWORD_pPr,
	KEYWORD_pStyle,
	KEYWORD_right,
	KEYWORD_rPr,
	KEYWORD_shd,
	KEYWORD_snapToGrid,
	KEYWORD_spacing,
	KEYWORD_suppressAutoHypens,
	KEYWORD_suppressLineNumbers,
	KEYWORD_suppressOverlap,
	KEYWORD_tab,
	KEYWORD_tabs,
	KEYWORD_textAlignment,
	KEYWORD_textboxTightWrap,
	KEYWORD_textDirection,
	KEYWORD_top,
	KEYWORD_topLinePunct,
	KEYWORD_widowControl,
	KEYWORD_wordWrap
};

#endif //_OXMLI_TYPES_H_

