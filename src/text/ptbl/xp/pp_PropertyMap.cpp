/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2003 AbiSource, Inc.
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

#include "stdlib.h"
#include "string.h"
#include "ctype.h"

#include "pp_PropertyMap.h"

static const char * AbiPropertyName[PP_PropertyMap::abi__count] = {
	"background-color",
	"bgcolor",
	"bot-color",
	"bot-style",
	"bot-thickness",
	"color",
	"column-gap",
	"column-line",
	"columns",
	"default-tab-interval",
	"dir-override",
	"display",
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
	"header",
	"header-even",
	"header-first",
	"header-last",
	"height",
	"homogeneous",
	"keep-together",
	"keep-with-next",
	"lang",
	"left-color",
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
	"right-color",
	"right-style",
	"right-thickness",
	"section-max-column-height",
	"section-restart",
	"section-restart-value",
	"section-space-after",
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
	"table-row-props",
	"table-row-spacing",
	"tabstops",
	"text-align",
	"text-decoration",
	"text-indent",
	"text-position",
	"top-color",
	"top-style",
	"top-thickness",
	"widows",
	"width"
};

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
	if (isdigit ((int) u))
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
	if (isdigit ((int) u))
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
	else // assume a color-name??
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
	if (!isdigit ((int) u)) return property;

	if ((*property > '0') && (*property < '4')) return s_linestyle[*property - '0'];

	return s_linestyle[0];
}

PP_PropertyMap::Line::Line () :
	m_t_color(color__unset),
	m_t_linestyle(linestyle__unset),
	m_t_thickness(thickness__unset),
	m_color(0,0,0),	// in case of color_color
	m_thickness(1)	// in case of thickness_length
{
	// 
}

void PP_PropertyMap::Line::reset ()
{
	m_t_color = color__unset;
	m_t_linestyle = linestyle__unset;
	m_t_thickness = thickness__unset;
}

PP_PropertyMap::Background::Background () :
	m_t_background(background__unset),
	m_color(256,256,256)
{
	// 
}

void PP_PropertyMap::Background::reset () // restore defaults
{
	m_t_background = background__unset;
}
