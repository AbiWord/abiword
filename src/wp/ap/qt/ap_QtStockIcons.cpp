/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* 
 * Copyright (C) 2013 Serhat Kiyak <serhatkiyak91@gmail.com>
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

#include "ap_QtStockIcons.h"
#include "ut_string.h"
#include "ap_Strings.h"
#include "ap_Menu_Id.h"

// built-in stock icons
#include "../xp/ToolbarIcons/tb_stock_paint.xpm" 
#include "../xp/ToolbarIcons/tb_1column.xpm"
#include "../xp/ToolbarIcons/tb_2column.xpm"
#include "../xp/ToolbarIcons/tb_3column.xpm"
#include "../xp/ToolbarIcons/tb_insert_graphic.xpm"
#include "../xp/ToolbarIcons/tb_view_showpara.xpm"
#include "../xp/ToolbarIcons/tb_lists_numbers.xpm" 
#include "../xp/ToolbarIcons/tb_lists_bullets.xpm" 
#include "../xp/ToolbarIcons/tb_text_fgcolor.xpm"
#include "../xp/ToolbarIcons/tb_text_bgcolor.xpm"
#include "../xp/ToolbarIcons/tb_insert_table.xpm" 
#include "../xp/ToolbarIcons/tb_add_row.xpm" 
#include "../xp/ToolbarIcons/tb_add_column.xpm"
#include "../xp/ToolbarIcons/tb_delete_row.xpm"
#include "../xp/ToolbarIcons/tb_delete_column.xpm"
#include "../xp/ToolbarIcons/tb_merge_cells.xpm"
#include "../xp/ToolbarIcons/tb_split_cells.xpm"
#include "../xp/ToolbarIcons/tb_hyperlink.xpm"
#include "../xp/ToolbarIcons/tb_anchor.xpm"
#include "../xp/ToolbarIcons/tb_text_overline.xpm"
#include "../xp/ToolbarIcons/tb_text_superscript.xpm"
#include "../xp/ToolbarIcons/tb_text_subscript.xpm"
#include "../xp/ToolbarIcons/tb_symbol.xpm"
#include "../xp/ToolbarIcons/tb_para_0before.xpm"
#include "../xp/ToolbarIcons/tb_para_12before.xpm"
#include "../xp/ToolbarIcons/tb_line_single_space.xpm"
#include "../xp/ToolbarIcons/tb_line_middle_space.xpm"
#include "../xp/ToolbarIcons/tb_line_double_space.xpm"
#include "../xp/ToolbarIcons/tb_text_direction_ltr.xpm"
#include "../xp/ToolbarIcons/tb_text_direction_rtl.xpm"
#include "../xp/ToolbarIcons/tb_text_dom_direction_rtl.xpm"
#include "../xp/ToolbarIcons/tb_edit_editheader.xpm"
#include "../xp/ToolbarIcons/tb_edit_editfooter.xpm"
#include "../xp/ToolbarIcons/tb_edit_removeheader.xpm"
#include "../xp/ToolbarIcons/tb_edit_removefooter.xpm"
#include "../xp/ToolbarIcons/tb_revision_new.xpm"
#include "../xp/ToolbarIcons/tb_revision_select.xpm"
#include "../xp/ToolbarIcons/tb_revision_show_final.xpm"
#include "../xp/ToolbarIcons/tb_revision_find_prev.xpm"
#include "../xp/ToolbarIcons/tb_revision_find_next.xpm"
#include "../xp/ToolbarIcons/tb_semitem_this.xpm"
#include "../xp/ToolbarIcons/tb_semitem_next.xpm"
#include "../xp/ToolbarIcons/tb_semitem_prev.xpm"
#include "../xp/ToolbarIcons/tb_semitem_edit.xpm"
#include "../xp/ToolbarIcons/tb_semitem_stylesheet_apply.xpm"
#include "../xp/ToolbarIcons/tb_new.xpm"
#include "../xp/ToolbarIcons/tb_open.xpm"
#include "../xp/ToolbarIcons/tb_save.xpm"
#include "../xp/ToolbarIcons/tb_save_as.xpm"
#include "../xp/ToolbarIcons/tb_print.xpm"
#include "../xp/ToolbarIcons/tb_print_preview.xpm"
#include "../xp/ToolbarIcons/menu_close.xpm"
#include "../xp/ToolbarIcons/menu_revert.xpm"
#include "../xp/ToolbarIcons/menu_file-properties.xpm"
#include "../xp/ToolbarIcons/menu_exit.xpm"
#include "../xp/ToolbarIcons/tb_spellcheck.xpm"
#include "../xp/ToolbarIcons/tb_cut.xpm"
#include "../xp/ToolbarIcons/tb_copy.xpm"
#include "../xp/ToolbarIcons/tb_paste.xpm"
#include "../xp/ToolbarIcons/tb_undo.xpm"
#include "../xp/ToolbarIcons/tb_redo.xpm"
#include "../xp/ToolbarIcons/tb_text_font.xpm"
#include "../xp/ToolbarIcons/tb_text_bold.xpm"
#include "../xp/ToolbarIcons/tb_text_italic.xpm"
#include "../xp/ToolbarIcons/tb_text_underline.xpm"
#include "../xp/ToolbarIcons/tb_text_align_left.xpm"
#include "../xp/ToolbarIcons/tb_text_center.xpm"
#include "../xp/ToolbarIcons/tb_text_align_right.xpm"
#include "../xp/ToolbarIcons/tb_text_justify.xpm"
#include "../xp/ToolbarIcons/tb_text_unindent.xpm"
#include "../xp/ToolbarIcons/tb_text_indent.xpm"
#include "../xp/ToolbarIcons/tb_script_play.xpm"
#include "../xp/ToolbarIcons/tb_text_strikeout.xpm"
#include "../xp/ToolbarIcons/tb_view_full_screen.xpm"

static struct AbiStockEntry {
	const char 		 *abi_stock_id;
	const XAP_Menu_Id	  menu_id; 		
	const guint32 		  string_id;
	const char 		**xpm_data;
} const stock_entries[] = { 
  { ABIWORD_FMTPAINTER,						AP_MENU_ID_FILE_NEW,
	AP_STRING_ID_TOOLBAR_LABEL_FMTPAINTER,				(const char **) tb_stock_paint_xpm },

  { ABIWORD_1COLUMN,							0,
	AP_STRING_ID_TOOLBAR_LABEL_1COLUMN,				(const char **) tb_1column_xpm }, 
  { ABIWORD_2COLUMN,							0,
	AP_STRING_ID_TOOLBAR_LABEL_2COLUMN,				(const char **) tb_2column_xpm },
  { ABIWORD_3COLUMN,							0,
	AP_STRING_ID_TOOLBAR_LABEL_3COLUMN,				(const char **) tb_3column_xpm },

  { ABIWORD_IMG,							AP_MENU_ID_INSERT_GRAPHIC, 		
	AP_STRING_ID_TOOLBAR_LABEL_IMG,					(const char **) tb_insert_graphic_xpm },
  { ABIWORD_VIEW_SHOWPARA,						AP_MENU_ID_VIEW_SHOWPARA, 			
	AP_STRING_ID_TOOLBAR_LABEL_VIEW_SHOWPARA,			(const char **) tb_view_showpara_xpm },

  { ABIWORD_LISTS_NUMBERS,						0, 								
	AP_STRING_ID_TOOLBAR_LABEL_LISTS_NUMBERS,			(const char **) tb_lists_numbers_xpm },
  { ABIWORD_LISTS_BULLETS,						0, 							
	AP_STRING_ID_TOOLBAR_LABEL_LISTS_BULLETS,			(const char **) tb_lists_bullets_xpm },

  { ABIWORD_COLOR_BACK,						0, 							
	AP_STRING_ID_TOOLBAR_LABEL_COLOR_BACK,				(const char **) tb_text_bgcolor_xpm },
  { ABIWORD_COLOR_FORE,						0, 							
	AP_STRING_ID_TOOLBAR_LABEL_COLOR_FORE,				(const char **) tb_text_fgcolor_xpm },

  { ABIWORD_INSERT_TABLE,						AP_MENU_ID_TABLE_INSERT_TABLE, 	
	AP_STRING_ID_TOOLBAR_LABEL_INSERT_TABLE,			(const char **) tb_insert_table_xpm },
  { ABIWORD_INSERT_TABLE,						AP_MENU_ID_TABLE_INSERTTABLE,
	AP_STRING_ID_MENU_LABEL_TABLE_INSERT_TABLE, 			(const char **) tb_insert_table_xpm },
  { ABIWORD_ADD_ROW,							AP_MENU_ID_TABLE_INSERT_ROWS_AFTER, 
	AP_STRING_ID_TOOLBAR_LABEL_ADD_ROW,				(const char **) tb_add_row_xpm }, 
  { ABIWORD_ADD_ROW,							AP_MENU_ID_TABLE_INSERTROW, 
	AP_STRING_ID_MENU_LABEL_TABLE_INSERTROW,			(const char **) tb_add_row_xpm }, 
  { ABIWORD_ADD_COLUMN,						AP_MENU_ID_TABLE_INSERT_COLUMNS_AFTER, 
	AP_STRING_ID_TOOLBAR_LABEL_ADD_COLUMN,				(const char **) tb_add_column_xpm },
  { ABIWORD_ADD_COLUMN,						AP_MENU_ID_TABLE_INSERTCOLUMN, 
	AP_STRING_ID_MENU_LABEL_TABLE_INSERTCOLUMN,			(const char **) tb_add_column_xpm },
  { ABIWORD_DELETE_ROW,						AP_MENU_ID_TABLE_DELETE_ROWS, 
	AP_STRING_ID_TOOLBAR_LABEL_DELETE_ROW,				(const char **) tb_delete_row_xpm },
  { ABIWORD_DELETE_ROW,						AP_MENU_ID_TABLE_DELETEROW, 
	AP_STRING_ID_MENU_LABEL_TABLE_DELETEROW,			(const char **) tb_delete_row_xpm },
  { ABIWORD_DELETE_COLUMN,						AP_MENU_ID_TABLE_DELETE_COLUMNS,
	AP_STRING_ID_TOOLBAR_LABEL_DELETE_COLUMN,			(const char **) tb_delete_column_xpm },
  { ABIWORD_DELETE_COLUMN,						AP_MENU_ID_TABLE_DELETECOLUMN,
	AP_STRING_ID_MENU_LABEL_TABLE_DELETECOLUMN,			(const char **) tb_delete_column_xpm },
  { ABIWORD_MERGE_CELLS,						AP_MENU_ID_TABLE_MERGE_CELLS, 
	AP_STRING_ID_TOOLBAR_LABEL_MERGE_CELLS,				(const char **) tb_merge_cells_xpm },
  { ABIWORD_SPLIT_CELLS,						AP_MENU_ID_TABLE_SPLIT_CELLS, 
	AP_STRING_ID_TOOLBAR_LABEL_SPLIT_CELLS,				(const char **) tb_split_cells_xpm },
  { ABIWORD_FMT_HYPERLINK,						AP_MENU_ID_INSERT_HYPERLINK, 
	AP_STRING_ID_TOOLBAR_LABEL_FMT_HYPERLINK,			(const char **) tb_hyperlink_xpm },
  { ABIWORD_FMT_BOOKMARK,						AP_MENU_ID_INSERT_BOOKMARK, 
	AP_STRING_ID_TOOLBAR_LABEL_FMT_BOOKMARK,			(const char **) tb_anchor },
  { ABIWORD_FMT_OVERLINE,						AP_MENU_ID_FMT_OVERLINE, 
	AP_STRING_ID_TOOLBAR_LABEL_FMT_OVERLINE,			(const char **) tb_text_overline_xpm },

  { ABIWORD_FMT_SUPERSCRIPT,						AP_MENU_ID_FMT_SUPERSCRIPT, 
	AP_STRING_ID_TOOLBAR_LABEL_FMT_SUPERSCRIPT,			(const char **) tb_text_superscript_xpm },
  { ABIWORD_FMT_SUBSCRIPT,						AP_MENU_ID_FMT_SUBSCRIPT, 
	AP_STRING_ID_TOOLBAR_LABEL_FMT_SUBSCRIPT,			(const char **) tb_text_subscript_xpm },
  { ABIWORD_INSERT_SYMBOL,						AP_MENU_ID_INSERT_SYMBOL, 
	AP_STRING_ID_TOOLBAR_LABEL_INSERT_SYMBOL,			(const char **) tb_symbol_xpm },

  { ABIWORD_PARA_0BEFORE,						0, 
	AP_STRING_ID_TOOLBAR_LABEL_PARA_0BEFORE,			(const char **) tb_para_0before_xpm },
  { ABIWORD_PARA_12BEFORE,						0, 
	AP_STRING_ID_TOOLBAR_LABEL_PARA_12BEFORE,			(const char **) tb_para_12before_xpm },
  { ABIWORD_SINGLE_SPACE,						0, 
	AP_STRING_ID_TOOLBAR_LABEL_SINGLE_SPACE,			(const char **) tb_line_single_space_xpm },
  { ABIWORD_MIDDLE_SPACE,						0, 
	AP_STRING_ID_TOOLBAR_LABEL_MIDDLE_SPACE,			(const char **) tb_line_middle_space_xpm },
  { ABIWORD_DOUBLE_SPACE,						0, 
	AP_STRING_ID_TOOLBAR_LABEL_DOUBLE_SPACE,			(const char **) tb_line_double_space_xpm },
  { ABIWORD_FMT_DIR_OVERRIDE_LTR,					AP_MENU_ID_FMT_DIRECTION_DO_LTR, 
	AP_STRING_ID_TOOLBAR_LABEL_FMT_DIR_OVERRIDE_LTR,		(const char **) tb_text_direction_ltr_xpm },
  { ABIWORD_FMT_DIR_OVERRIDE_RTL,					AP_MENU_ID_FMT_DIRECTION_DO_RTL, 
	AP_STRING_ID_TOOLBAR_LABEL_FMT_DIR_OVERRIDE_RTL,		(const char **) tb_text_direction_rtl_xpm },
  { ABIWORD_FMT_DOM_DIRECTION,						AP_MENU_ID_FMT_DIRECTION_DD_RTL, 
	AP_STRING_ID_TOOLBAR_LABEL_FMT_DOM_DIRECTION,			(const char **) tb_text_dom_direction_rtl_xpm },
  { ABIWORD_EDIT_HEADER,						AP_MENU_ID_EDIT_EDITHEADER, 
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_HEADER,				(const char **) tb_edit_editheader_xpm },
  { ABIWORD_EDIT_FOOTER,						AP_MENU_ID_EDIT_EDITFOOTER, 
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_FOOTER,				(const char **) tb_edit_editfooter_xpm },
  { ABIWORD_EDIT_REMOVEHEADER,						AP_MENU_ID_EDIT_REMOVEHEADER, 
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_REMOVEHEADER,			(const char **) tb_edit_removeheader_xpm },
  { ABIWORD_EDIT_REMOVEFOOTER,						AP_MENU_ID_EDIT_REMOVEFOOTER, 
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_REMOVEFOOTER,			(const char **) tb_edit_removefooter_xpm },

  { ABIWORD_REVISIONS_NEW,						0,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_NEW,			(const char **) tb_revision_new_xpm }, 
  { ABIWORD_REVISIONS_SELECT,						0,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_SELECT,			(const char **) tb_revision_select_xpm }, 
  { ABIWORD_REVISIONS_SHOW_FINAL,					0,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_SHOW_FINAL,  		(const char **) tb_revision_show_final_xpm }, 
  { ABIWORD_REVISIONS_FIND_PREV,					0,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_FIND_PREV,  		(const char **) tb_revision_find_prev_xpm }, 
  { ABIWORD_REVISIONS_FIND_NEXT,					0,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_FIND_NEXT,  		(const char **) tb_revision_find_next_xpm }, 

  { ABIWORD_SEMITEM_THIS,						0,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_THIS,			(const char **) tb_semitem_this_xpm }, 
  { ABIWORD_SEMITEM_NEXT,						0,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_NEXT,			(const char **) tb_semitem_next_xpm }, 
  { ABIWORD_SEMITEM_PREV,						0,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_PREV,			(const char **) tb_semitem_prev_xpm }, 
  { ABIWORD_SEMITEM_EDIT,						0,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_EDIT,			(const char **) tb_semitem_edit_xpm }, 
  { ABIWORD_SEMITEM_STYLESHEET_APPLY,					0,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_STYLESHEET_APPLY,		(const char **) tb_semitem_stylesheet_apply_xpm },
  { ABIWORD_FILE_NEW,							AP_MENU_ID_FILE_NEW,
	AP_STRING_ID_TOOLBAR_LABEL_FILE_NEW,				(const char **) tb_new_xpm },
  { ABIWORD_FILE_OPEN,							AP_MENU_ID_FILE_OPEN,
	AP_STRING_ID_TOOLBAR_LABEL_FILE_OPEN,				(const char **) tb_open_xpm },
  { ABIWORD_FILE_SAVE,							AP_MENU_ID_FILE_SAVE,
	AP_STRING_ID_TOOLBAR_LABEL_FILE_SAVE,				(const char **) tb_save_xpm },  
  { ABIWORD_FILE_SAVEAS,						AP_MENU_ID_FILE_SAVEAS,
	AP_STRING_ID_TOOLBAR_LABEL_FILE_SAVEAS,				(const char **) tb_save_as_xpm }, 
  { ABIWORD_FILE_PRINT,						AP_MENU_ID_FILE_PRINT,
	AP_STRING_ID_TOOLBAR_LABEL_FILE_PRINT,				(const char **) tb_print_xpm },  
  { ABIWORD_FILE_PRINT_PREVIEW,					AP_MENU_ID_FILE_PRINT_PREVIEW,
	AP_STRING_ID_TOOLBAR_LABEL_FILE_PRINT_PREVIEW,			(const char **) tb_print_preview_xpm },
  { ABIWORD_FILE_CLOSE,						AP_MENU_ID_FILE_CLOSE,
	AP_STRING_ID_MENU_LABEL_FILE_CLOSE,				(const char **) menu_close_xpm }, 
  { ABIWORD_FILE_REVERT,						AP_MENU_ID_FILE_REVERT,
	AP_STRING_ID_MENU_LABEL_FILE_REVERT,				(const char **) menu_revert_xpm }, 
  { ABIWORD_FILE_PROPERTIES,						AP_MENU_ID_FILE_PROPERTIES,
	AP_STRING_ID_MENU_LABEL_FILE_PROPERTIES,			(const char **) menu_file_properties_xpm }, 
  { ABIWORD_FILE_EXIT,							AP_MENU_ID_FILE_EXIT,
	AP_STRING_ID_MENU_LABEL_FILE_EXIT,				(const char **) menu_exit_xpm }, 
  { ABIWORD_SPELLCHECK,						AP_MENU_ID_TOOLS_SPELL,
	AP_STRING_ID_TOOLBAR_LABEL_SPELLCHECK,				(const char **) tb_spellcheck_xpm }, 
  { ABIWORD_EDIT_CUT,							AP_MENU_ID_EDIT_CUT,
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_CUT,				(const char **) tb_cut_xpm }, 
  { ABIWORD_EDIT_COPY,							AP_MENU_ID_EDIT_COPY,
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_COPY,				(const char **) tb_copy_xpm }, 
  { ABIWORD_EDIT_PASTE,						AP_MENU_ID_EDIT_PASTE,
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_PASTE,				(const char **) tb_paste_xpm }, 
  { ABIWORD_EDIT_UNDO,							AP_MENU_ID_EDIT_UNDO,
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_UNDO,				(const char **) tb_undo_xpm }, 
  { ABIWORD_EDIT_REDO,							AP_MENU_ID_EDIT_REDO,
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_REDO,				(const char **) tb_redo_xpm }, 
  { ABIWORD_FMT_FONT,							AP_MENU_ID_FMT_FONT,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_FONT,				(const char **) tb_text_font_xpm },
  { ABIWORD_FMT_BOLD,							AP_MENU_ID_FMT_BOLD,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_BOLD,				(const char **) tb_text_bold_xpm },
  { ABIWORD_FMT_ITALIC,						AP_MENU_ID_FMT_ITALIC,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_ITALIC,				(const char **) tb_text_italic_xpm },
  { ABIWORD_FMT_UNDERLINE,						AP_MENU_ID_FMT_UNDERLINE,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_UNDERLINE,			(const char **) tb_text_underline_xpm }, 
  { ABIWORD_ALIGN_LEFT,						AP_MENU_ID_ALIGN_LEFT,
	AP_STRING_ID_TOOLBAR_LABEL_ALIGN_LEFT,				(const char **) tb_text_align_left_xpm }, 
  { ABIWORD_ALIGN_CENTER,						AP_MENU_ID_ALIGN_CENTER,
	AP_STRING_ID_TOOLBAR_LABEL_ALIGN_CENTER,			(const char **) tb_text_center_xpm }, 
  { ABIWORD_ALIGN_RIGHT,						AP_MENU_ID_ALIGN_RIGHT,
	AP_STRING_ID_TOOLBAR_LABEL_ALIGN_RIGHT,				(const char **) tb_text_align_right_xpm }, 
  { ABIWORD_ALIGN_JUSTIFY,						AP_MENU_ID_ALIGN_JUSTIFY,
	AP_STRING_ID_TOOLBAR_LABEL_ALIGN_JUSTIFY,			(const char **) tb_text_justify_xpm }, 
  { ABIWORD_UNINDENT,							0,
	AP_STRING_ID_TOOLBAR_LABEL_UNINDENT,				(const char **) tb_text_unindent_xpm }, 
  { ABIWORD_INDENT,							0,
	AP_STRING_ID_TOOLBAR_LABEL_INDENT,				(const char **) tb_text_indent_xpm }, 
  { ABIWORD_SCRIPT_PLAY,						AP_MENU_ID_TOOLS_SCRIPTS,
	AP_STRING_ID_TOOLBAR_LABEL_SCRIPT_PLAY,				(const char **) tb_script_play_xpm }, 
  { ABIWORD_FMT_STRIKE,						AP_MENU_ID_FMT_STRIKE,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_STRIKE,				(const char **) tb_text_strikeout_xpm },
  { ABIWORD_VIEW_FULL_SCREEN,       					0,
	AP_STRING_ID_TOOLBAR_LABEL_VIEW_FULL_SCREEN,			(const char **) tb_view_full_screen_xpm },
  { NULL,								0,
	0, 								NULL }
};

QPixmap
abi_pixmap_from_toolbar_id (const char *toolbar_id)
{
	// the toolbar icons are named like that: "FILE_NEW_de-AT"
	char 		 *stock_id = g_strdup (ABIWORD_STOCK_PREFIX);
	char		**tokens;
	char		**iter;
	char 		 *tmp1;
	gint	  	  off;

	tmp1 = g_ascii_strdown (toolbar_id, -1);

	static size_t underscorelen = 0;
	size_t tmp1len = strlen(tmp1);

	if (underscorelen == 0) {

		char *lastunderscore = g_strrstr_len(tmp1, tmp1len, "_");

		if (lastunderscore && *lastunderscore) {
			underscorelen = strlen(lastunderscore);
		} else {
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			// We'll use six as the fail-safe, but some language codes like ast-ES
			// and be@latin need more of the string chopped off (Bug 11810)
			underscorelen = 6;
		}
	}

	off = tmp1len - underscorelen;
	tmp1[off] = '\0';
	tokens = g_strsplit (tmp1, "_", 0);
	g_free (tmp1);

	iter = tokens;
	while (*iter) {
		tmp1 = stock_id;
		stock_id = g_strdup_printf ("%s-%s", stock_id, *iter);
		g_free (tmp1);
		iter++;
	}
	g_strfreev (tokens);

	int i=0;
	while (stock_entries[i].abi_stock_id) 
	{
		if(!strcmp(stock_id, stock_entries[i].abi_stock_id))
		{
			QPixmap pixbuf(stock_entries[i].xpm_data);
			return pixbuf;
		}
		i++;
	}

	// Should never reach here
	UT_ASSERT(0);
	QPixmap pixbuf(0, 0);
	return pixbuf;
}
