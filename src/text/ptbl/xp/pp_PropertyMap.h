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


#ifndef PP_PROPERTYMAP_H
#define PP_PROPERTYMAP_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include "ut_misc.h"
#include "ut_IntStrMap.h"

class ABI_EXPORT PP_PropertyMap
{
public:
	enum AbiPropertyIndex // list of all properties used internally by AbiWord
	{
		abi_background_color = 0,
		abi_bgcolor,
		abi_bot_color,
		abi_bot_style,
		abi_bot_thickness,
		abi_color,
		abi_column_gap,
		abi_column_line,
		abi_columns,
		abi_default_tab_interval,
		abi_dir_override,
		abi_display,
		abi_dom_dir,
		abi_field_color,
		abi_field_font,
		abi_font_family,
		abi_font_size,
		abi_font_stretch,
		abi_font_style,
		abi_font_variant,
		abi_font_weight,
		abi_footer,
		abi_footer_even,
		abi_footer_first,
		abi_footer_last,
		abi_format,
		abi_header,
		abi_header_even,
		abi_header_first,
		abi_header_last,
		abi_height,
		abi_homogeneous,
		abi_keep_together,
		abi_keep_with_next,
		abi_lang,
		abi_left_color,
		abi_left_style,
		abi_left_thickness,
		abi_line_height,
		abi_list_decimal,
		abi_list_delim,
		abi_list_style,
		abi_list_tag,
		abi_margin_bottom,
		abi_margin_left,
		abi_margin_right,
		abi_margin_top,
		abi_orphans,
		abi_page_margin_bottom,
		abi_page_margin_footer,
		abi_page_margin_header,
		abi_page_margin_left,
		abi_page_margin_right,
		abi_page_margin_top,
		abi_right_color,
		abi_right_style,
		abi_right_thickness,
		abi_section_max_column_height,
		abi_section_restart,
		abi_section_restart_value,
		abi_section_space_after,
		abi_start_value,
		abi_table_border,
		abi_table_col_spacing,
		abi_table_column_leftpos,
		abi_table_column_props,
		abi_table_line_thickness,
		abi_table_line_type,
		abi_table_margin_bottom,
		abi_table_margin_left,
		abi_table_margin_right,
		abi_table_margin_top,
		abi_table_row_props,
		abi_table_row_spacing,
		abi_tabstops,
		abi_text_align,
		abi_text_decoration,
		abi_text_indent,
		abi_text_position,
		abi_top_color,
		abi_top_style,
		abi_top_thickness,
		abi_widows,
		abi_width,
		abi__count
	};
	enum TypeColor
	{
		color__unset = 0,
		color_inherit,
		color_transparent,
		color_color
	};
	enum TypeLineStyle
	{
		linestyle__unset = 0,
		linestyle_none,   // "0" or "none"
		linestyle_solid,  // "1" or "solid"
		linestyle_dotted, // "2" or "dotted"
		linestyle_dashed, // "3" or "dashed"

		// insert new styles here; do not change order of existing styles

		linestyle_inherit
	};
	enum TypeThickness
	{
		thickness__unset = 0,
		thickness_inherit,
		thickness_length
	};

	/* currently support only for solid fill styles, but should be able to
	 * come up with something to support images etc. - what else?
	 */
	enum TypeBackground
	{
		background__unset = 0,
		background_none,  // background-color: transparent
		background_solid, // background-color: <color>

		// insert new styles here; do not change order of existing styles

		background_inherit
	};

	static TypeColor color_type (const char * property);
	static TypeLineStyle linestyle_type (const char * property);
	static TypeThickness thickness_type (const char * property);
	static TypeBackground background_type (const char * property);

	static const char * linestyle_for_CSS (const char * property);

	class Line
	{
	public:
		Line ();

		void reset (); // restore defaults

		TypeColor		m_t_color;
		TypeLineStyle	m_t_linestyle;
		TypeThickness	m_t_thickness;

		UT_RGBColor		m_color;		// in case of color_color
		UT_uint32		m_thickness;	// in case of thickness_length
	};
	class Background
	{
	public:
		Background ();

		void reset (); // restore defaults

		TypeBackground	m_t_background;

		UT_RGBColor		m_color;		// in case of background_solid
	};

	static const char * abi_property_name (AbiPropertyIndex index);

	static bool abi_property_lookup (const char * name, AbiPropertyIndex & index);

private:
	UT_IntStrMap	m_map;
public:
	const UT_IntStrMap & map () const { return m_map; }

	inline void clear ()
	{
		m_map.clear ();
	}

	bool ins (AbiPropertyIndex key, UT_UTF8String * value) // responsibility for value passes here
	{
		if ((value == 0) || (key == abi__count)) return false;
		return m_map.ins (static_cast<UT_sint32>(key), value);
	}
	bool ins (AbiPropertyIndex key, const char * value)
	{
		if ((value == 0) || (key == abi__count)) return false;
		return m_map.ins (static_cast<UT_sint32>(key), value);
	}

	/* returns false if no such key-value
	 */
	inline bool del (AbiPropertyIndex key) // value is deleted
	{
		if (key == abi__count) return false;
		return m_map.del (static_cast<UT_sint32>(key));
	}
	inline bool del (AbiPropertyIndex key, UT_UTF8String *& value) // value is passed back
	{
		if (key == abi__count) return false;
		return m_map.del (static_cast<UT_sint32>(key), value);
	}

	inline const UT_UTF8String * operator[] (AbiPropertyIndex key)
	{
		if (key == abi__count) return 0;
		return m_map[static_cast<UT_sint32>(key)];
	}
};

#endif /* ! PP_PROPERTYMAP_H */
