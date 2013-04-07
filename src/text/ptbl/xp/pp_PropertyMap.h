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


#ifndef PP_PROPERTYMAP_H
#define PP_PROPERTYMAP_H

#if 0  // only used for code below. determine what to do with it
#if defined(__MINGW32__)
#undef snprintf
#define _GLIBCXX_USE_C99_DYNAMIC 1
#endif

#include <map>
#endif

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include "ut_string_class.h"
#include "ut_color.h"


class ABI_EXPORT PP_PropertyMap
{
public:

	/**
	 * For debugging and test purpose: get the properties array.
	 */
	static const char** _properties(int & num);

	/**
	 * List of all properties used internally by AbiWord
	 * Keep these sorted.
	 */
	enum AbiPropertyIndex
	{
		abi_annotation_author= 0,
		abi_annotation_date,
		abi_annotation_title,
		abi_background_color,
		abi_background_image,
		abi_bgcolor,
		abi_border_merge,
		abi_border_shadow_merge,
		abi_bot_attach,
		abi_bot_color,
		abi_bot_style,
		abi_bot_shadow,
		abi_bot_shadow_color,
		abi_bot_space,
		abi_bot_thickness,
		abi_bounding_space,
		abi_cell_margin_bottom,
		abi_cell_margin_left,
		abi_cell_margin_right,
		abi_cell_margin_top,
		abi_color,
		abi_column_gap,
		abi_column_line,
		abi_columns,
		abi_default_tab_interval,
		abi_dir_override,
		abi_display,
		abi_document_endnote_initial,
		abi_document_endnote_place_enddoc,
		abi_document_endnote_place_endsection,
		abi_document_endnote_restart_section,
		abi_document_endnote_type,
		abi_document_footnote_initial,
		abi_document_footnote_restart_page,
		abi_document_footnote_restart_section,
		abi_document_footnote_type,
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
		abi_frame_col_xpos,
		abi_frame_col_ypos,
		abi_frame_expand_height,
		abi_frame_height,
		abi_frame_horiz_align,
		abi_frame_min_height,
		abi_frame_page_xpos,
		abi_frame_page_ypos,
		abi_frame_pref_column,
		abi_frame_pref_page,
		abi_frame_rel_width,
		abi_frame_type,
		abi_frame_width,
		abi_header,
		abi_header_even,
		abi_header_first,
		abi_header_last,
		abi_height,
		abi_homogeneous,
		abi_keep_together,
		abi_keep_with_next,
		abi_lang,
		abi_left_attach,
		abi_left_color,
		abi_left_style,
		abi_left_thickness,
		abi_left_shadow,
		abi_left_shadow_color,
		abi_left_space,
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
		abi_position_to,
		abi_right_attach,
		abi_right_color,
		abi_right_shadow,
		abi_right_shadow_color,
		abi_right_space,
		abi_right_style,
		abi_right_thickness,
		abi_section_footnote_line_thickness,
		abi_section_footnote_yoff,
		abi_section_max_column_height,
		abi_section_restart,
		abi_section_restart_value,
		abi_section_space_after,
		abi_shading_background_color,
		abi_shading_foreground_color,
		abi_shading_pattern,
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
		abi_table_max_extra_margin,
		abi_table_rel_column_props,
		abi_table_rel_width,
		abi_table_row_props,
		abi_table_row_spacing,
		abi_table_width,
		abi_tabstops,
		abi_text_align,
		abi_text_decoration,
		abi_text_folded,
		abi_text_folded_id,
		abi_text_indent,
		abi_text_position,
		abi_text_transform,
		abi_tight_wrap,
	    abi_toc_dest_style1,
	    abi_toc_dest_style2,
	    abi_toc_dest_style3,
	    abi_toc_dest_style4,
        abi_toc_id,
		abi_toc_indent1,
		abi_toc_indent2,
		abi_toc_indent3,
		abi_toc_indent4,
	    abi_toc_has_heading,
		abi_toc_heading,
		abi_toc_has_label1,
		abi_toc_has_label2,
		abi_toc_has_label3,
		abi_toc_has_label4,
		abi_toc_heading_style,
		abi_toc_label_after1,
		abi_toc_label_after2,
		abi_toc_label_after3,
		abi_toc_label_after4,
		abi_toc_label_before1,
		abi_toc_label_before2,
		abi_toc_label_before3,
		abi_toc_label_before4,
		abi_toc_label_inherits1,
		abi_toc_label_inherits2,
		abi_toc_label_inherits3,
		abi_toc_label_inherits4,
		abi_toc_label_start1,
		abi_toc_label_start2,
		abi_toc_label_start3,
		abi_toc_label_start4,
		abi_toc_label_type1,
		abi_toc_label_type2,
		abi_toc_label_type3,
		abi_toc_label_type4,
		abi_toc_page_type1,
		abi_toc_page_type2,
		abi_toc_page_type3,
		abi_toc_page_type4,
	    abi_toc_source_style1,
	    abi_toc_source_style2,
	    abi_toc_source_style3,
	    abi_toc_source_style4,
		abi_toc_tab_leader1,
		abi_toc_tab_leader2,
		abi_toc_tab_leader3,
		abi_toc_tab_leader4,
		abi_top_attach,
		abi_top_color,
		abi_top_style,
		abi_top_thickness,
		abi_top_shadow,
		abi_top_shadow_color,
		abi_top_space,
		abi_vert_align,
		abi_widows,
		abi_width,
		abi_wrap_mode,
		abi_xpad,
		abi_xpos,
		abi_ypad,
		abi_ypos,
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

	class ABI_EXPORT Line
	{
	public:
		Line ();

		void reset (); // restore defaults
		TypeColor		m_t_color;
		TypeLineStyle	m_t_linestyle;
		TypeThickness	m_t_thickness;

		UT_RGBColor		m_color;		// in case of color_color
		UT_uint32		m_thickness;	// in case of thickness_length
		UT_sint32       m_spacing;      // gap in logical unit to content
	};


	class ABI_EXPORT Background
	{
	public:
		Background ();

		void reset (); // restore defaults

		TypeBackground	m_t_background;

		UT_RGBColor		m_color;		// in case of background_solid
	};

	static const char * abi_property_name (AbiPropertyIndex index);

	static bool abi_property_lookup (const char * name, AbiPropertyIndex & index);

#if 0
	typedef std::map<UT_sint32, UT_UTF8String *> map_type;
private:
	map_type m_map;
public:
	const map_type & map () const { return m_map; }

	inline void clear ()
	{
		m_map.clear ();
	}

	bool ins (AbiPropertyIndex key, UT_UTF8String * value) // responsibility for value passes here
	{
		if ((value == 0) || (key == abi__count))
			return false;
		std::pair<map_type::iterator, bool> p =
			m_map.insert(map_type::value_type(static_cast<UT_sint32>(key),
											  value));
		return p.second;
	}
	bool ins (AbiPropertyIndex key, const char * value)
	{
		if ((value == 0) || (key == abi__count))
			return false;
		std::pair<map_type::iterator, bool> p =
			m_map.insert(map_type::value_type(static_cast<UT_sint32>(key),
											  new UT_UTF8String(value)));
		return p.second;
	}

	/* returns false if no such key-value
	 */
	inline bool del (AbiPropertyIndex key) // value is deleted
	{
		if (key == abi__count) {
			return false;
		}
		map_type::iterator i = m_map.find(static_cast<UT_sint32>(key));
		if (i == m_map.end()) {
			return false;
		}
		delete (*i).second;
		m_map.erase(i);
		return true;
	}
	inline bool del (AbiPropertyIndex key, UT_UTF8String *& value) // value is passed back
	{
		if (key == abi__count) return false;
		map_type::iterator i = m_map.find(static_cast<UT_sint32>(key));
		if (i == m_map.end()) {
			return false;
		}
		value = (*i).second;
		m_map.erase(i);
		return true;
	}

	inline const UT_UTF8String * operator[] (AbiPropertyIndex key)
	{
		if (key == abi__count)
			return 0;
		map_type::iterator i = m_map.find(static_cast<UT_sint32>(key));
		if (i == m_map.end()) {
			return NULL;
		}
		return (*i).second;
	}
#endif
};
ABI_EXPORT bool operator==(const PP_PropertyMap::Line L1, const PP_PropertyMap::Line L2);

#endif /* ! PP_PROPERTYMAP_H */
