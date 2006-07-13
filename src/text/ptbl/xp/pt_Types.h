/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef PT_TYPES_H
#define PT_TYPES_H

#include "ut_types.h"
#include "ut_vector.h"

//! index to actual document data
typedef UT_uint32 PT_BufIndex;
//! index to Attribute/Property Tables
typedef UT_uint32 PT_AttrPropIndex;

//! absolute document position
typedef UT_uint32 PT_DocPosition;
//! block-relative document position
typedef UT_uint32 PT_BlockOffset;

class PD_Document;
/*!
 PD_DocumentRange identifies a piece of the document, using two
 PT_DocPosition absolute document positions as limits.
*/

class ABI_EXPORT PD_DocumentRange
{
public:
	//! Dummy constructor.
	//! Resulting range is invalid until it is initialized with set
	PD_DocumentRange(void)
		: m_pDoc(NULL), m_pos1(0), m_pos2(0)
		{
		}

	//! Initializing constructor.
	PD_DocumentRange(PD_Document * pDoc, PT_DocPosition k1, PT_DocPosition k2)
		: m_pDoc(pDoc), m_pos1(k1), m_pos2(k2)
		{
		}

	//! Set range limits
	void set(PD_Document * pDoc, PT_DocPosition k1, PT_DocPosition k2)
		{
			m_pDoc = pDoc;
			m_pos1 = k1;
			m_pos2 = k2;
		}

	//! Document this range is in
	PD_Document *		m_pDoc;
	//! Lower limit of range
	PT_DocPosition		m_pos1;
	//! Upper limit of range
	PT_DocPosition		m_pos2;
};

//! PTStruxType tells the sub-type of a FragStrux.
typedef enum _PTStruxType
{
	PTX_Section = 0,       	// 0 -- maker sure that we can cast into uint
	PTX_Block,           	// 1
	PTX_SectionHdrFtr,   	// 2
	PTX_SectionEndnote,  	// 3
	PTX_SectionTable,    	// 4
	PTX_SectionCell,     	// 5
	PTX_SectionFootnote, 	// 6
	PTX_SectionMarginnote, 	// 7
	PTX_SectionFrame,      	// 8
	PTX_SectionTOC,			// 9
	PTX_EndCell,          	// 10
	PTX_EndTable,
    PTX_EndFootnote,
    PTX_EndMarginnote,
    PTX_EndEndnote,
    PTX_EndFrame,
	PTX_EndTOC,
	PTX_StruxDummy
} PTStruxType;

//! PTObjectType tells the sub-type of an FragObject.

typedef enum _PTObjectType { PTO_Image = 0, PTO_Field, PTO_Bookmark, PTO_Hyperlink, PTO_Math, PTO_Embed } PTObjectType;

//! PieceTable states
typedef enum _PTState { PTS_Create=0, PTS_Loading=1, PTS_Editing=2 } PTState;
//! ChangeFormat types
typedef enum _PTChangeFmt 
  { PTC_AddFmt=0, 
    PTC_RemoveFmt=1, 
    PTC_AddStyle=2, 
    PTC_SetFmt=3,
    PTC_SetExactly=4} PTChangeFmt;

typedef enum _PLListenerType
{
	PTL_UNKNOWN,
	PTL_DocLayout,
	PTL_CollabExport
	/* add more types here ONLY as necessary */
} PLListenerType;

//! ID of a listener - this is its location in m_vecListeners
typedef UT_uint32 PL_ListenerId;

//! opaque document data
typedef const void * PL_StruxDocHandle;
//! opaque layout data
typedef const void * PL_StruxFmtHandle;


// MUST keep these in sync with the attribute map in pp_Attribute.cpp !!!
typedef enum {
    // NULL value for terminating attibute lists
    PT__ATTRIBUTE_NAME_NULL = 0,

    // These are document-level attributes only used for doc-wide settings
    PT_STYLES_ATTRIBUTE_NAME,
    PT_XIDMAX_ATTRIBUTE_NAME,
    PT_XMLNS_ATTRIBUTE_NAME,
    PT_XMLSPACE_ATTRIBUTE_NAME,
    PT_XMLNSAWML_ATTRIBUTE_NAME,
    PT_XMLNSXLINK_ATTRIBUTE_NAME,
    PT_XMLNSSVG_ATTRIBUTE_NAME,
    PT_XMLNSFO_ATTRIBUTE_NAME,
    PT_XMLNSMATH_ATTRIBUTE_NAME,
    PT_XMLNSDC_ATTRIBUTE_NAME,
    PT_FILEFORMAT_ATTRIBUTE_NAME,
    PT_VERSION_ATTRIBUTE_NAME,

    // This is the normal attributes set
    PT_PROPS_ATTRIBUTE_NAME,
    PT_STYLE_ATTRIBUTE_NAME,
    PT_LEVEL_ATTRIBUTE_NAME,
    PT_LISTID_ATTRIBUTE_NAME,
    PT_PARENTID_ATTRIBUTE_NAME,
    PT_NAME_ATTRIBUTE_NAME,
    PT_TYPE_ATTRIBUTE_NAME,
    PT_BASEDON_ATTRIBUTE_NAME,
    PT_FOLLOWEDBY_ATTRIBUTE_NAME,
    PT_HEADER_ATTRIBUTE_NAME,
    PT_HEADEREVEN_ATTRIBUTE_NAME,
    PT_HEADERFIRST_ATTRIBUTE_NAME,
    PT_HEADERLAST_ATTRIBUTE_NAME,
    PT_FOOTER_ATTRIBUTE_NAME,
    PT_FOOTEREVEN_ATTRIBUTE_NAME,
    PT_FOOTERFIRST_ATTRIBUTE_NAME,
    PT_FOOTERLAST_ATTRIBUTE_NAME,
    PT_REVISION_ATTRIBUTE_NAME,
    PT_ID_ATTRIBUTE_NAME,
    PT_STRUX_IMAGE_DATAID,
    PT_XID_ATTRIBUTE_NAME,
    PT_XLINKHREF_ATTRIBUTE_NAME,
    PT_HREF_ATTRIBUTE_NAME,
    PT_LISTDELIM_ATTRIBUTE_NAME,
    PT_LISTDECIMAL_ATTRIBUTE_NAME,
    PT_STARTVALUE_ATTRIBUTE_NAME,
    PT_PAGETYPE_ATTRIBUTE_NAME,
    PT_WIDTH_ATTRIBUTE_NAME,
    PT_HEIGHT_ATTRIBUTE_NAME,
    PT_UNITS_ATTRIBUTE_NAME,
    PT_PAGESCALE_ATTRIBUTE_NAME,
    PT_ORIENTATION_ATTRIBUTE_NAME,
    PT_PARAM_ATTRIBUTE_NAME,
    
    /*add new stuff here */

    PT_ATTRIBUTE_NAME_COUNT
} PT_Attribute;

// list of all properties used internally by AbiWord
// MUST keep this in sync with the property map in pp_Property.cpp !!!
typedef enum
{
    abi__NULL = 0,
    abi_background_color,
    abi_background_image,
    abi_bgcolor,
    abi_bot_attach,
    abi_bot_color,
    abi_bot_style,
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
    abi_frame_column_xpos,
    abi_frame_column_ypos,
    abi_frame_height,
    abi_frame_page_xpos,
    abi_frame_page_ypos,
    abi_frame_position_to,
    abi_frame_type,
    abi_frame_width,
    abi_frame_xpos,
    abi_frame_ypos,
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
    abi_right_attach,
    abi_right_color,
    abi_right_style,
    abi_right_thickness,
    abi_section_footnote_line_thickness,
    abi_section_footnote_yoff,
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
    abi_text_folded,
    abi_text_folded_id,
    abi_text_indent,
    abi_text_position,
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
    abi_widows,
    abi_width,
    abi_wrap_mode,

    /* must be last */
    abi__prop_count
} PT_Property;

typedef struct 
{
    PT_Attribute a;
    GQuark       v;
} PT_AttributePair;

typedef struct
{
    PT_Property p;
    GQuark      v;
} PT_PropertyPair;

typedef UT_GenericVector<PT_PropertyPair>  PT_PropertyVector;
typedef UT_GenericVector<PT_AttributePair> PT_AttributeVector;

#endif /* PT_TYPES_H */
