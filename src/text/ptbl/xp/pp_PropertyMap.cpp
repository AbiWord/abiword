/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#include "stdlib.h"
#include "string.h"
#include "ctype.h"

#include "pp_PropertyMap.h"

/**
 * The property names.
 * Keep these sorted and watch the commas.
 */
static const char * AbiPropertyName[PP_PropertyMap::abi__count] = {
	"annotation-author",
	"annotation-date",
	"annotation-title",
	"background-color",
	"background-image",
	"bgcolor",
	"border-merge",
	"border-shadow-merge",
	"bot-attach",
	"bot-color",
	"bot-shadow",
	"bot-shadow-color",
	"bot-space",
	"bot-style",
	"bot-thickness",
	"bounding-space",
	"cell-margin-bottom",
	"cell-margin-left",
	"cell-margin-right",
	"cell-margin-top",
	"color",
	"column-gap",
	"column-line",
	"columns",
	"default-tab-interval",
	"dir-override",
	"display",
	"document-endnote-initial",
	"document-endnote-place_enddoc",
	"document-endnote-place_endsection",
	"document-endnote-restart_section",
	"document-endnote-type",
	"document-footnote-initial",
	"document-footnote-restart_page",
	"document-footnote-restart_section",
	"document-footnote-type",
	"dom-dir",
	"field-color",
	"field-font",
	"font-family",
	"font-size",
	"font-stretch",
	"font-style",
	"font-variant",
	"font-weight",
	"footer",
	"footer-even",
	"footer-first",
	"footer-last",
	"format",
	"frame-col-xpos",
	"frame-col-ypos",
	"frame-expand-height",
	"frame-height",
	"frame-horiz-align",
	"frame-min-height",
	"frame-page-xpos",
	"frame-page-ypos",
	"frame-pref-column",
	"frame-pref-page",
	"frame-rel-width",
	"frame-type",
	"frame-width",
	"header",
	"header-even",
	"header-first",
	"header-last",
	"height",
	"homogeneous",
	"keep-together",
	"keep-with-next",
	"lang",
	"left-attach",
	"left-color",
	"left-shadow",
	"left-shadow-color",
	"left-space",
	"left-style",
	"left-thickness",
	"line-height",
	"list-decimal",
	"list-delim",
	"list-style",
	"list-tag",
	"margin-bottom",
	"margin-left",
	"margin-right",
	"margin-top",
	"orphans",
	"page-margin-bottom",
	"page-margin-footer",
	"page-margin-header",
	"page-margin-left",
	"page-margin-right",
	"page-margin-top",
	"position-to",
	"right-attach",
	"right-color",
	"right-shadow",
	"right-shadow-color",
	"right-space",
	"right-style",
	"right-thickness",
	"section-footnote-line-thickness",
	"section-footnote-yoff",
	"section-max-column-height",
	"section-restart",
	"section-restart-value",
	"section-space-after",
	"shading-background-color",
	"shading-foreground-color",
	"shading-pattern",
	"start-value",
	"table-border",
	"table-col-spacing",
	"table-column-leftpos",
	"table-column-props",
	"table-line-thickness",
	"table-line-type",
	"table-margin-bottom",
	"table-margin-left",
	"table-margin-right",
	"table-margin-top",
	"table-max-extra-margin",
	"table-rel-column-props",
	"table-rel-width",
	"table-row-props",
	"table-row-spacing",
	"table-width",
	"tabstops",
	"text-align",
	"text-decoration",
	"text-folded",
	"text-folded-id",
	"text-indent",
	"text-position",
	"text-transform",
	"tight-wrap",
	"toc-dest-style1",
	"toc-dest-style2",
	"toc-dest-style3",
	"toc-dest-style4",
	"toc-has-heading",
	"toc-has-label1",
	"toc-has-label2",
	"toc-has-label3",
	"toc-has-label4",
	"toc-heading",
	"toc-heading-style",
    "toc-id",
    "toc-indent1",
    "toc-indent2",
    "toc-indent3",
    "toc-indent4",
	"toc-label-after1",
	"toc-label-after2",
	"toc-label-after3",
	"toc-label-after4",
	"toc-label-before1",
	"toc-label-before2",
	"toc-label-before3",
	"toc-label-before4",
	"toc-label-inherits1",
	"toc-label-inherits2",
	"toc-label-inherits3",
	"toc-label-inherits4",
	"toc-label-start1",
	"toc-label-start2",
	"toc-label-start3",
	"toc-label-start4",
	"toc-label-type1",
	"toc-label-type2",
	"toc-label-type3",
	"toc-label-type4",
	"toc-page-type1",
	"toc-page-type2",
	"toc-page-type3",
	"toc-page-type4",
	"toc-source-style1",
	"toc-source-style2",
	"toc-source-style3",
	"toc-source-style4",
	"toc-tab-leader1",
	"toc-tab-leader2",
	"toc-tab-leader3",
	"toc-tab-leader4",
	"top-attach",
	"top-color",
	"top-shadow",
	"top-shadow-color",
	"top-space",
	"top-style",
	"top-thickness",
	"vert-align",
	"widows",
	"width",
	"wrap-mode",
	"xpad",
	"xpos",
	"ypad",
	"ypos"
};


const char** PP_PropertyMap::_properties(int & num)
{
	num = sizeof(AbiPropertyName) / sizeof(*AbiPropertyName);
	return &AbiPropertyName[0];
}

extern "C" {
	static int s_str_compare (const void * a, const void * b)
	{
		const char *         pea = reinterpret_cast<const char *>(a);
		const char * const * pod = reinterpret_cast<const char * const *>(b);

		return strcmp (pea, *pod);
	}
}

const char * PP_PropertyMap::abi_property_name (AbiPropertyIndex index)
{
	return (index < abi__count) ? AbiPropertyName[index] : 0;
}

bool PP_PropertyMap::abi_property_lookup (const char * name, AbiPropertyIndex & index)
{
	if ( name == 0) return false;
	if (*name == 0) return false;

	void * result = bsearch (name, AbiPropertyName,
							 static_cast<size_t>(abi__count), sizeof (char *), s_str_compare);

	if (result == 0) return false;

	const char * const * css_name = reinterpret_cast<const char * const *>(result);

	index = static_cast<AbiPropertyIndex>(css_name - AbiPropertyName);

	return true;
}

PP_PropertyMap::TypeColor PP_PropertyMap::color_type (const char * property)
{
	TypeColor color = color__unset;

	if (property == 0) return color; // erk!

	if (strcmp (property, "inherit") == 0)
		color = color_inherit;
	else if (strcmp (property, "transparent") == 0)
		color = color_transparent;
	else
		color = color_color;

	return color;
}

PP_PropertyMap::TypeLineStyle PP_PropertyMap::linestyle_type (const char * property)
{
	TypeLineStyle linestyle = linestyle__unset;

	if ( property == 0) return linestyle; // erk!
	if (*property == 0) return linestyle; // erk!

	unsigned char u = static_cast<unsigned char>(*property);
	if (isdigit (static_cast<int>(u)))
		{
			int i = atoi (property);
			if ((i < 0) || (i + 1 >= static_cast<int>(linestyle_inherit)))
				{
					/* line-style out of range
					   */
					return linestyle_solid; // erk!
				}
			return static_cast<TypeLineStyle>(i + 1);
		}

	if (strcmp (property, "inherit") == 0)
		linestyle = linestyle_inherit;
	else if (strcmp (property, "none")   == 0)
		linestyle = linestyle_none;
	else if (strcmp (property, "solid")  == 0)
		linestyle = linestyle_solid;
	else if (strcmp (property, "dotted") == 0)
		linestyle = linestyle_dotted;
	else if (strcmp (property, "dashed") == 0)
		linestyle = linestyle_dashed;
	else
		linestyle = linestyle_solid; // erk!

	return linestyle;
}

PP_PropertyMap::TypeThickness PP_PropertyMap::thickness_type (const char * property)
{
	TypeThickness thickness = thickness__unset;

	if (property == 0) return thickness; // erk!

	if (strcmp (property, "inherit") == 0)
		thickness = thickness_inherit;
	else
		thickness = thickness_length;

	return thickness;
}

PP_PropertyMap::TypeBackground PP_PropertyMap::background_type (const char * property)
{
	TypeBackground background = background__unset;

	if ( property == 0) return background; // erk!
	if (*property == 0) return background; // erk!

	unsigned char u = static_cast<unsigned char>(*property);

 // colours can have digits as their first character ie "1f7399". Use string
 // length as an additional hueristic to detect if property is a color string
 //

	if (isdigit (static_cast<int>(u)) && (strlen(property)<3)) 
		{
			int i = atoi (property);
			if ((i < 0) || (i + 1 >= static_cast<int>(background_inherit)))
				{
					/* line-style out of range
					 */
					return background_none; // erk!
				}
			return static_cast<TypeBackground>(i + 1);
		}

	if (strcmp (property, "inherit") == 0)
		background = background_inherit;
	else if ((strcmp (property, "none")  == 0) || (strcmp (property, "transparent") == 0))
		background = background_none;
	else 
		background = background_solid;

	return background;
}

static const char * s_linestyle[4] = {
	"none",
	"solid",
	"dotted",
	"dashed"
};

const char * PP_PropertyMap::linestyle_for_CSS (const char * property)
{
	if (property == 0) return s_linestyle[0];

	unsigned char u = static_cast<unsigned char>(*property);
	if (!isdigit (static_cast<int>(u))) return property;

	if ((*property > '0') && (*property < '4')) return s_linestyle[*property - '0'];

	return s_linestyle[0];
}

PP_PropertyMap::Line::Line () :
	m_t_color(color__unset),
	m_t_linestyle(linestyle__unset),
	m_t_thickness(thickness__unset),
	m_color(0,0,0),	// in case of color_color
	m_thickness(0), // in case of thickness_length
    m_spacing(0)
{
	// 
}

void PP_PropertyMap::Line::reset ()
{
	m_t_color = color__unset;
	m_t_linestyle = linestyle__unset;
	m_t_thickness = thickness__unset;
}

bool operator==( const PP_PropertyMap::Line L1, const PP_PropertyMap::Line L2)
{
	return (L1.m_thickness == L2.m_thickness) &&
		(L1.m_color.m_red == L2.m_color.m_red) &&
		(L1.m_color.m_grn == L2.m_color.m_grn) &&
		(L1.m_color.m_blu == L2.m_color.m_blu) &&
		(L1.m_t_linestyle == L2.m_t_linestyle) &&
		(L1.m_spacing == L2.m_spacing);
}

PP_PropertyMap::Background::Background () :
	m_t_background(background__unset),
	m_color(255,255,255)
{
	// 
}

void PP_PropertyMap::Background::reset () // restore defaults
{
	m_t_background = background__unset;
}

