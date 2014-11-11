/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* 
 * Copyright (C) 2006 Robert Staudinger <robert.staudinger@gmail.com>
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

#include "ap_UnixStockIcons.h"
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

static const gchar * abi_stock_get_gtk_stock_id 	(const gchar *abi_stock_id);

/*
 * Built-in stock icon definitions.
 * Menus are identified by IDs, so that's needed too.
 */
static struct AbiStockEntry {
	const gchar 		 *abi_stock_id;
	const guint32 		  string_id;
	const gchar 		**xpm_data;
} const stock_entries[] = {
  { ABIWORD_FMTPAINTER,
	AP_STRING_ID_TOOLBAR_LABEL_FMTPAINTER,			(const gchar **) tb_stock_paint_xpm },

  { ABIWORD_1COLUMN,
	AP_STRING_ID_TOOLBAR_LABEL_1COLUMN,				(const gchar **) tb_1column_xpm },
  { ABIWORD_2COLUMN,
	AP_STRING_ID_TOOLBAR_LABEL_2COLUMN,				(const gchar **) tb_2column_xpm },
  { ABIWORD_3COLUMN,
	AP_STRING_ID_TOOLBAR_LABEL_3COLUMN,				(const gchar **) tb_3column_xpm },

  { ABIWORD_IMG,
	AP_STRING_ID_TOOLBAR_LABEL_IMG,					(const gchar **) tb_insert_graphic_xpm },
  { ABIWORD_VIEW_SHOWPARA,
	AP_STRING_ID_TOOLBAR_LABEL_VIEW_SHOWPARA,		(const gchar **) tb_view_showpara_xpm },

  { ABIWORD_LISTS_NUMBERS,
	AP_STRING_ID_TOOLBAR_LABEL_LISTS_NUMBERS,		(const gchar **) tb_lists_numbers_xpm },
  { ABIWORD_LISTS_BULLETS,
	AP_STRING_ID_TOOLBAR_LABEL_LISTS_BULLETS,		(const gchar **) tb_lists_bullets_xpm },

  { ABIWORD_COLOR_BACK,
	AP_STRING_ID_TOOLBAR_LABEL_COLOR_BACK,			(const gchar **) tb_text_bgcolor_xpm },
  { ABIWORD_COLOR_FORE,
	AP_STRING_ID_TOOLBAR_LABEL_COLOR_FORE,			(const gchar **) tb_text_fgcolor_xpm },

  { ABIWORD_INSERT_TABLE,
	AP_STRING_ID_TOOLBAR_LABEL_INSERT_TABLE,		(const gchar **) tb_insert_table_xpm },
  { ABIWORD_INSERT_TABLE,
	AP_STRING_ID_MENU_LABEL_TABLE_INSERT_TABLE, 	(const gchar **) tb_insert_table_xpm },
  { ABIWORD_ADD_ROW,
	AP_STRING_ID_TOOLBAR_LABEL_ADD_ROW,				(const gchar **) tb_add_row_xpm },
  { ABIWORD_ADD_ROW,
	AP_STRING_ID_MENU_LABEL_TABLE_INSERTROW,		(const gchar **) tb_add_row_xpm },
  { ABIWORD_ADD_COLUMN,
	AP_STRING_ID_TOOLBAR_LABEL_ADD_COLUMN,			(const gchar **) tb_add_column_xpm },
  { ABIWORD_ADD_COLUMN,
	AP_STRING_ID_MENU_LABEL_TABLE_INSERTCOLUMN,		(const gchar **) tb_add_column_xpm },
  { ABIWORD_DELETE_ROW,
	AP_STRING_ID_TOOLBAR_LABEL_DELETE_ROW,			(const gchar **) tb_delete_row_xpm },
  { ABIWORD_DELETE_ROW,
	AP_STRING_ID_MENU_LABEL_TABLE_DELETEROW,		(const gchar **) tb_delete_row_xpm },
  { ABIWORD_DELETE_COLUMN,
	AP_STRING_ID_TOOLBAR_LABEL_DELETE_COLUMN,		(const gchar **) tb_delete_column_xpm },
  { ABIWORD_DELETE_COLUMN,
	AP_STRING_ID_MENU_LABEL_TABLE_DELETECOLUMN,		(const gchar **) tb_delete_column_xpm },
  { ABIWORD_MERGE_CELLS,
	AP_STRING_ID_TOOLBAR_LABEL_MERGE_CELLS,			(const gchar **) tb_merge_cells_xpm },
  { ABIWORD_SPLIT_CELLS,
	AP_STRING_ID_TOOLBAR_LABEL_SPLIT_CELLS,			(const gchar **) tb_split_cells_xpm },
  { ABIWORD_FMT_HYPERLINK,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_HYPERLINK,		(const gchar **) tb_hyperlink_xpm },
  { ABIWORD_FMT_BOOKMARK,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_BOOKMARK,		(const gchar **) tb_anchor },
  { ABIWORD_FMT_OVERLINE,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_OVERLINE,		(const gchar **) tb_text_overline_xpm },

  { ABIWORD_FMT_SUPERSCRIPT,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_SUPERSCRIPT,		(const gchar **) tb_text_superscript_xpm },
  { ABIWORD_FMT_SUBSCRIPT,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_SUBSCRIPT,		(const gchar **) tb_text_subscript_xpm },
  { ABIWORD_INSERT_SYMBOL,
	AP_STRING_ID_TOOLBAR_LABEL_INSERT_SYMBOL,		(const gchar **) tb_symbol_xpm },

  { ABIWORD_PARA_0BEFORE,
	AP_STRING_ID_TOOLBAR_LABEL_PARA_0BEFORE,		(const gchar **) tb_para_0before_xpm },
  { ABIWORD_PARA_12BEFORE,
	AP_STRING_ID_TOOLBAR_LABEL_PARA_12BEFORE,		(const gchar **) tb_para_12before_xpm },
  { ABIWORD_SINGLE_SPACE,
	AP_STRING_ID_TOOLBAR_LABEL_SINGLE_SPACE,		(const gchar **) tb_line_single_space_xpm },
  { ABIWORD_MIDDLE_SPACE,
	AP_STRING_ID_TOOLBAR_LABEL_MIDDLE_SPACE,		(const gchar **) tb_line_middle_space_xpm },
  { ABIWORD_DOUBLE_SPACE,
	AP_STRING_ID_TOOLBAR_LABEL_DOUBLE_SPACE,		(const gchar **) tb_line_double_space_xpm },
  { ABIWORD_FMT_DIR_OVERRIDE_LTR,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_DIR_OVERRIDE_LTR,(const gchar **) tb_text_direction_ltr_xpm },
  { ABIWORD_FMT_DIR_OVERRIDE_RTL,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_DIR_OVERRIDE_RTL,(const gchar **) tb_text_direction_rtl_xpm },
  { ABIWORD_FMT_DOM_DIRECTION,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_DOM_DIRECTION,	(const gchar **) tb_text_dom_direction_rtl_xpm },
  { ABIWORD_EDIT_HEADER,
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_HEADER,			(const gchar **) tb_edit_editheader_xpm },
  { ABIWORD_EDIT_FOOTER,
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_FOOTER,			(const gchar **) tb_edit_editfooter_xpm },
  { ABIWORD_EDIT_REMOVEHEADER,
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_REMOVEHEADER,	(const gchar **) tb_edit_removeheader_xpm },
  { ABIWORD_EDIT_REMOVEFOOTER,
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_REMOVEFOOTER,	(const gchar **) tb_edit_removefooter_xpm },

  { ABIWORD_REVISIONS_NEW,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_NEW,		(const gchar **) tb_revision_new_xpm },
  { ABIWORD_REVISIONS_SELECT,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_SELECT,	(const gchar **) tb_revision_select_xpm },
  { ABIWORD_REVISIONS_SHOW_FINAL,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_SHOW_FINAL,  (const gchar **) tb_revision_show_final_xpm },
  { ABIWORD_REVISIONS_FIND_PREV,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_FIND_PREV,  (const gchar **) tb_revision_find_prev_xpm },
  { ABIWORD_REVISIONS_FIND_NEXT,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_FIND_NEXT,  (const gchar **) tb_revision_find_next_xpm },

  { ABIWORD_SEMITEM_THIS,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_THIS,	(const gchar **) tb_semitem_this_xpm },
  { ABIWORD_SEMITEM_NEXT,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_NEXT,	(const gchar **) tb_semitem_next_xpm },
  { ABIWORD_SEMITEM_PREV,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_PREV,	(const gchar **) tb_semitem_prev_xpm },
  { ABIWORD_SEMITEM_EDIT,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_EDIT,	(const gchar **) tb_semitem_edit_xpm },
  { ABIWORD_SEMITEM_STYLESHEET_APPLY,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_STYLESHEET_APPLY,	(const gchar **) tb_semitem_stylesheet_apply_xpm },

  { NULL,
	0, 											NULL }
};

/*
 * Mapping of abiword icons to their gtk counterparts.
 */
static struct AbiStockMapping {
	const gchar 		*abi_stock_id;
	const XAP_Menu_Id        menu_id;
	const gchar 		*gtk_stock_id;
} const stock_mapping[] = {
  { ABIWORD_FILE_NEW,				AP_MENU_ID_FILE_NEW,			"document-new" },
  { ABIWORD_FILE_OPEN,				AP_MENU_ID_FILE_OPEN,			"document-open" },
  { ABIWORD_FILE_SAVE,				AP_MENU_ID_FILE_SAVE,			"document-save" },
  { ABIWORD_FILE_SAVEAS,			AP_MENU_ID_FILE_SAVEAS,			"document-save-as" },
  { ABIWORD_FILE_PRINT,				AP_MENU_ID_FILE_PRINT,			"document-print" },
  { ABIWORD_FILE_PRINT_PREVIEW,		AP_MENU_ID_FILE_PRINT_PREVIEW,	"document-print-preview" },
  { ABIWORD_FILE_CLOSE,				AP_MENU_ID_FILE_CLOSE,			"window-close" },
  { ABIWORD_FILE_REVERT,			AP_MENU_ID_FILE_REVERT,			"document-revert" },
  { ABIWORD_FILE_PROPERTIES,		AP_MENU_ID_FILE_PROPERTIES,		"document-properties" },
  { ABIWORD_FILE_EXIT,				AP_MENU_ID_FILE_EXIT,			"application-exit" },
  { ABIWORD_SPELLCHECK,				AP_MENU_ID_TOOLS_SPELL,			"tools-check-spelling" },

  { ABIWORD_EDIT_CUT,				AP_MENU_ID_EDIT_CUT,			"edit-cut" },
  { ABIWORD_EDIT_COPY,				AP_MENU_ID_EDIT_COPY,			"edit-copy" },
  { ABIWORD_EDIT_PASTE,				AP_MENU_ID_EDIT_PASTE,			"edit-paste" },
  { ABIWORD_EDIT_UNDO,				AP_MENU_ID_EDIT_UNDO,			"edit-undo" },
  { ABIWORD_EDIT_REDO,				AP_MENU_ID_EDIT_REDO,			"edit-redo" },
  { ABIWORD_EDIT_CLEAR,				AP_MENU_ID_EDIT_CLEAR,			"edit-clear" },
  { ABIWORD_EDIT_FIND,				AP_MENU_ID_EDIT_FIND,			"edit-find" },
  { ABIWORD_EDIT_REPLACE, 			AP_MENU_ID_EDIT_REPLACE,		"edit-find-replace" },
  { ABIWORD_EDIT_GOTO,				AP_MENU_ID_EDIT_GOTO,			"go-jump" },

  { ABIWORD_HELP,					AP_MENU_ID_HELP_CONTENTS,		"help-contents" },
  { ABIWORD_HELP_ABOUT,				AP_MENU_ID_HELP_ABOUT, 			"help-about" },

//  { ABIWORD_FMT_FONT,				AP_MENU_ID_FMT_FONT,			GTK_STOCK_SELECT_FONT },
  { ABIWORD_FMT_BOLD,				AP_MENU_ID_FMT_BOLD,			"format-text-bold" },
  { ABIWORD_FMT_ITALIC,				AP_MENU_ID_FMT_ITALIC,			"format-text-italic" },
  { ABIWORD_FMT_UNDERLINE,			AP_MENU_ID_FMT_UNDERLINE,		"format-text-underline" },
//  { ABIWORD_FMT_CHOOSE,             0,                              GTK_STOCK_SELECT_FONT },

  { ABIWORD_ALIGN_LEFT,				AP_MENU_ID_ALIGN_LEFT,			"format-justify-left" },
  { ABIWORD_ALIGN_CENTER,			AP_MENU_ID_ALIGN_CENTER,		"format-justify-center" },
  { ABIWORD_ALIGN_RIGHT,			AP_MENU_ID_ALIGN_RIGHT,			"format-justify-right" },
  { ABIWORD_ALIGN_JUSTIFY,			AP_MENU_ID_ALIGN_JUSTIFY,		"format-justify-fill" },

  { ABIWORD_UNINDENT,				0,								"format-indent-less" },
  { ABIWORD_INDENT,					0,								"format-indent-more" },

  { ABIWORD_SCRIPT_PLAY,			AP_MENU_ID_TOOLS_SCRIPTS,		"system-run" },
  { ABIWORD_FMT_STRIKE,				AP_MENU_ID_FMT_STRIKE,			"format-text-strikethrough" },
  { ABIWORD_VIEW_FULL_SCREEN,       0,                              "view-fullscreen" },
  { NULL, 					0,					NULL }
};

/*!
 * Register AbiWord's gtk stock icons.
 */
void
abi_stock_init (void)
{
	gint			 	 i;

	static gboolean is_initialized = FALSE;
	if (is_initialized) {
		return;
	}
	is_initialized = TRUE;

	i = 0;
	while (stock_entries[i].abi_stock_id) {
		GdkPixbuf			*pixbuf;
		gint w, h;
		pixbuf = gdk_pixbuf_new_from_xpm_data (stock_entries[i].xpm_data);
		w = gdk_pixbuf_get_width(pixbuf);
		h = gdk_pixbuf_get_height(pixbuf);
		// XXX fixme. Damn Gtk deprecation.
		gtk_icon_theme_add_builtin_icon(stock_entries[i].abi_stock_id,
										std::max(w,h), pixbuf);
		g_object_unref (pixbuf);
		i++;
	}
}

/*!
 * Get stock id from toolbar icon name.
 */
gchar *
abi_stock_from_toolbar_id (const gchar *toolbar_id)
{
	// the toolbar icons are named like that: "FILE_NEW_de-AT"
	gchar 		 *stock_id = g_strdup (ABIWORD_STOCK_PREFIX);
	gchar		**tokens;
	gchar		**iter;
	gchar 		 *tmp1;
	const gchar	 *tmp2;
	gint	  	  off;

	tmp1 = g_ascii_strdown (toolbar_id, -1);

	static size_t underscorelen = 0;
	size_t tmp1len = strlen(tmp1);

	if (underscorelen == 0) {

		gchar *lastunderscore = g_strrstr_len(tmp1, tmp1len, "_");

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

	tmp2 = abi_stock_get_gtk_stock_id (stock_id);
	if (tmp2) {
		g_free (stock_id);
		stock_id = g_strdup (tmp2);
	} else {
		xxx_UT_DEBUGMSG(("abi_stock_get_gtk_stock_id returned NULL for stock_id: %s\n", stock_id));
	}

	return stock_id;
}

/*!
 * Map AbiWord stock id to gtk counterpart.
 * Retval must be g_free'd.
 */
static const gchar *
abi_stock_get_gtk_stock_id (const gchar * abi_stock_id)
{
	gint i;

	i = 0;
	while (stock_mapping[i].abi_stock_id) {
		if (0 == strcmp (abi_stock_id, stock_mapping[i].abi_stock_id)) {
			return stock_mapping[i].gtk_stock_id;
		}
		i++;
	}

	return NULL;
}
