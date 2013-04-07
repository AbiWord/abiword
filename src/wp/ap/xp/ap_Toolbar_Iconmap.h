/* AbiWord
 * Copyright (C) 2002 AbiSource, Inc.
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

/*****************************************************************/
/*****************************************************************/
/** This file defines maps the set of Id's used for all toolbar **/
/** to icons that are loaded into abi.   Defaults are ID, icon  **/
/** string.  Overloads are ID_LANG, icon.                       **/
/*****************************************************************/
/*****************************************************************/

// Mapping sof ID, to default icons
// Mappings to ID_LANG to non-default icons
// We can do specific mappings for some languages. For example,
// in Spainsh, some applications use N for Negrita instead of B for Bold
// For now, we do not use this.
//
// SAMPLE: toolbariconmap(LISTS_BULLETS_fi-FI,tb_lists_xpm)

///////////////////////////////////////////
// !!!!
//
// NB: Keep this list alphabetically sorted
//
// !!!!!
///////////////////////////////////////////


toolbariconmap(1COLUMN,tb_1column_xpm)
toolbariconmap(2COLUMN,tb_2column_xpm)
toolbariconmap(3COLUMN,tb_3column_xpm)
toolbariconmap(ADD_COLUMN, tb_add_column_xpm)
toolbariconmap(ADD_ROW, tb_add_row_xpm)
toolbariconmap(ALIGN_CENTER,tb_text_center_xpm)
toolbariconmap(ALIGN_JUSTIFY,tb_text_justify_xpm)
toolbariconmap(ALIGN_LEFT,tb_text_align_left_xpm)
toolbariconmap(ALIGN_RIGHT,tb_text_align_right_xpm)
toolbariconmap(COLOR_BACK,tb_text_bgcolor_xpm)
toolbariconmap(COLOR_FORE,tb_text_fgcolor_xpm)
toolbariconmap(DELETE_COLUMN, tb_delete_column_xpm)
toolbariconmap(DELETE_ROW, tb_delete_row_xpm)
toolbariconmap(DOUBLE_SPACE,tb_line_double_space_xpm)
toolbariconmap(EDIT_COPY,tb_copy_xpm)
toolbariconmap(EDIT_CUT,tb_cut_xpm)
toolbariconmap(EDIT_FOOTER,tb_edit_editfooter_xpm)
toolbariconmap(EDIT_HEADER,tb_edit_editheader_xpm)
toolbariconmap(EDIT_PASTE,tb_paste_xpm)
toolbariconmap(EDIT_REDO,tb_redo_xpm)
toolbariconmap(EDIT_REMOVEFOOTER,tb_edit_removefooter_xpm)
toolbariconmap(EDIT_REMOVEHEADER,tb_edit_removeheader_xpm)
toolbariconmap(EDIT_UNDO,tb_undo_xpm)
toolbariconmap(FILE_NEW,tb_new_xpm)
toolbariconmap(FILE_OPEN,tb_open_xpm)
toolbariconmap(FILE_PRINT,tb_print_xpm)
toolbariconmap(FILE_PRINT_PREVIEW,tb_print_preview_xpm)
toolbariconmap(FILE_SAVE,tb_save_xpm)
toolbariconmap(FILE_SAVEAS,tb_save_as_xpm)
toolbariconmap(FMT_BOLD,tb_text_bold_xpm)
toolbariconmap(FMT_BOOKMARK,tb_anchor)
toolbariconmap(FMT_BOTTOMLINE,tb_text_bottomline_xpm)
#if XAP_SIMPLE_TOOLBAR
toolbariconmap(FMT_CHOOSE,tb_stock_font_xpm)
#endif
toolbariconmap(FMT_DIR_OVERRIDE_LTR,tb_text_direction_ltr_xpm)
toolbariconmap(FMT_DIR_OVERRIDE_RTL,tb_text_direction_rtl_xpm)
toolbariconmap(FMT_DOM_DIRECTION,tb_text_dom_direction_rtl_xpm)
toolbariconmap(FMT_FONT,NoIcon)
toolbariconmap(FMT_HYPERLINK,tb_hyperlink_xpm)
toolbariconmap(FMT_ITALIC,tb_text_italic_xpm)
toolbariconmap(FMT_OVERLINE,tb_text_overline_xpm)
toolbariconmap(FMT_SIZE,NoIcon)
toolbariconmap(FMT_STRIKE,tb_text_strikeout_xpm)
toolbariconmap(FMT_STYLE,NoIcon)
toolbariconmap(FMT_SUBSCRIPT,tb_text_subscript_xpm)
toolbariconmap(FMT_SUPERSCRIPT,tb_text_superscript_xpm)
toolbariconmap(FMT_TOPLINE,tb_text_topline_xpm)
toolbariconmap(FMT_UNDERLINE,tb_text_underline_xpm)
toolbariconmap(FMTPAINTER,tb_stock_paint_xpm)
toolbariconmap(FT_LINEBOTTOM, tb_LineBottom_xpm)
toolbariconmap(FT_LINELEFT, tb_LineLeft_xpm)
toolbariconmap(FT_LINERIGHT, tb_LineRight_xpm)
toolbariconmap(FT_LINETOP, tb_LineTop_xpm)
toolbariconmap(HELP,tb_help_xpm)
toolbariconmap(IMG,tb_insert_graphic_xpm)
toolbariconmap(INDENT,tb_text_indent_xpm)
toolbariconmap(INSERT_SYMBOL,tb_symbol_xpm)
toolbariconmap(INSERT_TABLE, tb_insert_table_xpm)
toolbariconmap(LISTS_BULLETS,tb_lists_bullets_xpm)
toolbariconmap(LISTS_NUMBERS,tb_lists_numbers_xpm)
toolbariconmap(Menu_AbiWord_About,menu_about_xpm)
toolbariconmap(Menu_AbiWord_Add_Column, menu_add_column_xpm)
toolbariconmap(Menu_AbiWord_Add_Row, menu_add_row_xpm)
toolbariconmap(Menu_AbiWord_Align_Center,menu_text_center_xpm)
toolbariconmap(Menu_AbiWord_Align_Justify,menu_text_justify_xpm)
toolbariconmap(Menu_AbiWord_Align_Left,menu_text_left_xpm)
toolbariconmap(Menu_AbiWord_Align_Right,menu_text_right_xpm)
toolbariconmap(Menu_AbiWord_Bold,menu_text_bold_xpm)
toolbariconmap(Menu_AbiWord_Book,menu_book_xpm)
toolbariconmap(Menu_AbiWord_Bookmark,menu_insert_bookmark_xpm)
toolbariconmap(Menu_AbiWord_Bottomline,menu_text_bottomline_xpm)
toolbariconmap(Menu_AbiWord_Clear,menu_delete_xpm)
toolbariconmap(Menu_AbiWord_Close, menu_close_xpm)
toolbariconmap(Menu_AbiWord_Copy,menu_copy_xpm)
toolbariconmap(Menu_AbiWord_Credits,menu_credits_xpm)
toolbariconmap(Menu_AbiWord_Cut,menu_cut_xpm)
toolbariconmap(Menu_AbiWord_Delete_Column, menu_delete_column_xpm)
toolbariconmap(Menu_AbiWord_Delete_Row, menu_delete_row_xpm)
toolbariconmap(Menu_AbiWord_Delete_Table,menu_delete_table_xpm)
toolbariconmap(Menu_AbiWord_Execute,menu_exec_xpm)
toolbariconmap(Menu_AbiWord_Exit,menu_exit_xpm)
toolbariconmap(Menu_AbiWord_Export, menu_export_xpm)
toolbariconmap(Menu_AbiWord_Font,menu_font_xpm)
toolbariconmap(Menu_AbiWord_Goto,menu_jump_to_xpm)
toolbariconmap(Menu_AbiWord_Help,menu_help_xpm)
toolbariconmap(Menu_AbiWord_Hyperlink,menu_insert_hyperlink_xpm)
toolbariconmap(Menu_AbiWord_Img,menu_insert_graphic_xpm)
toolbariconmap(Menu_AbiWord_Import, menu_import_xpm)
toolbariconmap(Menu_AbiWord_Insert_Symbol,menu_insert_symbol_xpm)
toolbariconmap(Menu_AbiWord_Insert_Table,menu_insert_table_xpm)
toolbariconmap(Menu_AbiWord_Italic,menu_text_italic_xpm)
toolbariconmap(Menu_AbiWord_Merge_Cells, menu_merge_cells_xpm)
toolbariconmap(Menu_AbiWord_New,menu_new_xpm)
toolbariconmap(Menu_AbiWord_Open,menu_open_xpm)
toolbariconmap(Menu_AbiWord_Overline,menu_text_overline_xpm)
toolbariconmap(Menu_AbiWord_Paste,menu_paste_xpm)
toolbariconmap(Menu_AbiWord_Preferences,menu_preferences_xpm)
toolbariconmap(Menu_AbiWord_Print,menu_print_xpm)
toolbariconmap(Menu_AbiWord_Print_Preview,menu_print_preview_xpm)
toolbariconmap(Menu_AbiWord_Print_Setup, menu_print_setup_xpm)
toolbariconmap(Menu_AbiWord_Properties, menu_file_properties_xpm)
toolbariconmap(Menu_AbiWord_Redo,menu_redo_xpm)
toolbariconmap(Menu_AbiWord_Replace,menu_search_replace_xpm)
toolbariconmap(Menu_AbiWord_Revert, menu_revert_xpm)
toolbariconmap(Menu_AbiWord_Save,menu_save_xpm)
toolbariconmap(Menu_AbiWord_SaveAs,menu_save_as_xpm)
toolbariconmap(Menu_AbiWord_Search,menu_search_xpm)
toolbariconmap(Menu_AbiWord_Spellcheck,menu_spellcheck_xpm)
toolbariconmap(Menu_AbiWord_Split_Cells, menu_split_cells_xpm)
toolbariconmap(Menu_AbiWord_Strike,menu_text_strikethrough_xpm)
toolbariconmap(Menu_AbiWord_Subscript,menu_subscript_xpm)
toolbariconmap(Menu_AbiWord_Superscript,menu_superscript_xpm)
toolbariconmap(Menu_AbiWord_Topline,menu_text_topline_xpm)
toolbariconmap(Menu_AbiWord_Underline,menu_text_underline_xpm)
toolbariconmap(Menu_AbiWord_Undo,menu_undo_xpm)
toolbariconmap(MERGE_CELLS, tb_merge_cells_xpm)
toolbariconmap(MERGEABOVE, tb_MergeAbove_xpm)
toolbariconmap(MERGEBELOW, tb_MergeBelow_xpm)
toolbariconmap(MERGELEFT, tb_MergeLeft_xpm)
toolbariconmap(MERGERIGHT, tb_MergeRight_xpm)
toolbariconmap(MIDDLE_SPACE,tb_line_middle_space_xpm)
toolbariconmap(OPTIONSDLG, tb_LineLeft_xpm)
toolbariconmap(PARA_0BEFORE,tb_para_0before_xpm)
toolbariconmap(PARA_12BEFORE,tb_para_12before_xpm)
toolbariconmap(REVISIONS_FIND_NEXT,tb_revision_find_next_xpm)
toolbariconmap(REVISIONS_FIND_PREV,tb_revision_find_prev_xpm)
toolbariconmap(REVISIONS_NEW,tb_revision_new_xpm)
toolbariconmap(REVISIONS_SELECT,tb_revision_select_xpm)
toolbariconmap(REVISIONS_SHOW_FINAL,tb_revision_show_final_xpm)
toolbariconmap(SCRIPT_PLAY,tb_script_play_xpm)
toolbariconmap(SEMITEM_EDIT, tb_semitem_edit_xpm)
toolbariconmap(SEMITEM_NEXT, tb_semitem_next_xpm)
toolbariconmap(SEMITEM_PREV, tb_semitem_prev_xpm)
toolbariconmap(SEMITEM_STYLESHEET_APPLY, tb_semitem_stylesheet_apply_xpm)
toolbariconmap(SEMITEM_THIS, tb_semitem_this_xpm)
toolbariconmap(SINGLE_SPACE,tb_line_single_space_xpm)
toolbariconmap(SPELLCHECK,tb_spellcheck_xpm)
toolbariconmap(SPLIT_CELLS, tb_split_cells_xpm)
toolbariconmap(SPLITABOVE, tb_SplitAbove_xpm)
toolbariconmap(SPLITBELOW, tb_SplitBelow_xpm)
toolbariconmap(SPLITHORIMID, tb_SplitHoriMid_xpm)
toolbariconmap(SPLITLEFT, tb_SplitLeft_xpm)
toolbariconmap(SPLITRIGHT, tb_SplitRight_xpm)
toolbariconmap(SPLITVERTMID, tb_SplitVertMid_xpm)
toolbariconmap(TB_ADD_COLUMN, tb_add_column_xpm)
toolbariconmap(TB_ADD_ROW, tb_add_row_xpm)
toolbariconmap(TB_DELETE_COLUMN, tb_delete_column_xpm)
toolbariconmap(TB_DELETE_ROW, tb_delete_row_xpm)
toolbariconmap(TB_MERGE_CELLS, tb_merge_cells_xpm)
toolbariconmap(TB_SPLIT_CELLS, tb_split_cells_xpm)
toolbariconmap(TRANSPARENTLANG,tb_transparent_xpm)
toolbariconmap(UNINDENT,tb_text_unindent_xpm)
#if XAP_SIMPLE_TOOLBAR
toolbariconmap(VIEW_FULL_SCREEN,tb_view_full_screen_xpm)
#endif
toolbariconmap(VIEW_SHOWPARA,tb_view_showpara_xpm)
toolbariconmap(ZOOM,NoIcon)


