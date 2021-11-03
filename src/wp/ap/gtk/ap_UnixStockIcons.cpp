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

/*
 * Built-in stock icon definitions.
 * Menus are identified by IDs, so that's needed too.
 */
static struct AbiStockEntry {
	const gchar*		  abi_stock_id;
	const guint32		  string_id;
	const gchar*		  name;
} const stock_entries[] = {
  { ABIWORD_FMTPAINTER,
	AP_STRING_ID_TOOLBAR_LABEL_FMTPAINTER,			"tb_stock_paint" },

  { ABIWORD_1COLUMN,
	AP_STRING_ID_TOOLBAR_LABEL_1COLUMN,				"tb_1column" },
  { ABIWORD_2COLUMN,
	AP_STRING_ID_TOOLBAR_LABEL_2COLUMN,				"tb_2column" },
  { ABIWORD_3COLUMN,
	AP_STRING_ID_TOOLBAR_LABEL_3COLUMN,				"tb_3column" },

  { ABIWORD_IMG,
	AP_STRING_ID_TOOLBAR_LABEL_IMG,					"tb_insert_graphic" },
  { ABIWORD_VIEW_SHOWPARA,
	AP_STRING_ID_TOOLBAR_LABEL_VIEW_SHOWPARA,		"tb_view_showpara" },

  { ABIWORD_LISTS_NUMBERS,
	AP_STRING_ID_TOOLBAR_LABEL_LISTS_NUMBERS,		"tb_lists_numbers" },
  { ABIWORD_LISTS_BULLETS,
	AP_STRING_ID_TOOLBAR_LABEL_LISTS_BULLETS,		"tb_lists_bullets" },

  { ABIWORD_COLOR_BACK,
	AP_STRING_ID_TOOLBAR_LABEL_COLOR_BACK,			"tb_text_bgcolor" },
  { ABIWORD_COLOR_FORE,
	AP_STRING_ID_TOOLBAR_LABEL_COLOR_FORE,			"tb_text_fgcolor" },

  { ABIWORD_INSERT_TABLE,
	AP_STRING_ID_TOOLBAR_LABEL_INSERT_TABLE,		"tb_insert_table" },
  { ABIWORD_INSERT_TABLE,
	AP_STRING_ID_MENU_LABEL_TABLE_INSERT_TABLE, 	"tb_insert_table" },
  { ABIWORD_ADD_ROW,
	AP_STRING_ID_TOOLBAR_LABEL_ADD_ROW,				"tb_add_row" },
  { ABIWORD_ADD_ROW,
	AP_STRING_ID_MENU_LABEL_TABLE_INSERTROW,		"tb_add_row" },
  { ABIWORD_ADD_COLUMN,
	AP_STRING_ID_TOOLBAR_LABEL_ADD_COLUMN,			"tb_add_column" },
  { ABIWORD_ADD_COLUMN,
	AP_STRING_ID_MENU_LABEL_TABLE_INSERTCOLUMN,		"tb_add_column" },
  { ABIWORD_DELETE_ROW,
	AP_STRING_ID_TOOLBAR_LABEL_DELETE_ROW,			"tb_delete_row" },
  { ABIWORD_DELETE_ROW,
	AP_STRING_ID_MENU_LABEL_TABLE_DELETEROW,		"tb_delete_row" },
  { ABIWORD_DELETE_COLUMN,
	AP_STRING_ID_TOOLBAR_LABEL_DELETE_COLUMN,		"tb_delete_column" },
  { ABIWORD_DELETE_COLUMN,
	AP_STRING_ID_MENU_LABEL_TABLE_DELETECOLUMN,		"tb_delete_column" },
  { ABIWORD_MERGE_CELLS,
	AP_STRING_ID_TOOLBAR_LABEL_MERGE_CELLS,			"tb_merge_cells" },
  { ABIWORD_SPLIT_CELLS,
	AP_STRING_ID_TOOLBAR_LABEL_SPLIT_CELLS,			"tb_split_cells" },
  { ABIWORD_FMT_HYPERLINK,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_HYPERLINK,		"tb_hyperlink" },
  { ABIWORD_FMT_BOOKMARK,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_BOOKMARK,		"tb_anchor" },
  { ABIWORD_FMT_OVERLINE,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_OVERLINE,		"tb_text_overline" },

  { ABIWORD_FMT_SUPERSCRIPT,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_SUPERSCRIPT,		"tb_text_superscript" },
  { ABIWORD_FMT_SUBSCRIPT,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_SUBSCRIPT,		"tb_text_subscript" },
  { ABIWORD_INSERT_SYMBOL,
	AP_STRING_ID_TOOLBAR_LABEL_INSERT_SYMBOL,		"tb_symbol" },

  { ABIWORD_PARA_0BEFORE,
	AP_STRING_ID_TOOLBAR_LABEL_PARA_0BEFORE,		"tb_para_0before" },
  { ABIWORD_PARA_12BEFORE,
	AP_STRING_ID_TOOLBAR_LABEL_PARA_12BEFORE,		"tb_para_12before" },
  { ABIWORD_SINGLE_SPACE,
	AP_STRING_ID_TOOLBAR_LABEL_SINGLE_SPACE,		"tb_line_single_space" },
  { ABIWORD_MIDDLE_SPACE,
	AP_STRING_ID_TOOLBAR_LABEL_MIDDLE_SPACE,		"tb_line_middle_space" },
  { ABIWORD_DOUBLE_SPACE,
	AP_STRING_ID_TOOLBAR_LABEL_DOUBLE_SPACE,		"tb_line_double_space" },
  { ABIWORD_FMT_DIR_OVERRIDE_LTR,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_DIR_OVERRIDE_LTR,"tb_text_direction_ltr" },
  { ABIWORD_FMT_DIR_OVERRIDE_RTL,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_DIR_OVERRIDE_RTL,"tb_text_direction_rtl" },
  { ABIWORD_FMT_DOM_DIRECTION,
	AP_STRING_ID_TOOLBAR_LABEL_FMT_DOM_DIRECTION,	"tb_text_dom_direction_rtl" },
  { ABIWORD_EDIT_HEADER,
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_HEADER,			"tb_edit_editheader" },
  { ABIWORD_EDIT_FOOTER,
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_FOOTER,			"tb_edit_editfooter" },
  { ABIWORD_EDIT_REMOVEHEADER,
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_REMOVEHEADER,	"tb_edit_removeheader" },
  { ABIWORD_EDIT_REMOVEFOOTER,
	AP_STRING_ID_TOOLBAR_LABEL_EDIT_REMOVEFOOTER,	"tb_edit_removefooter" },

  { ABIWORD_REVISIONS_NEW,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_NEW,		"tb_revision_new" },
  { ABIWORD_REVISIONS_SELECT,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_SELECT,	"tb_revision_select" },
  { ABIWORD_REVISIONS_SHOW_FINAL,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_SHOW_FINAL,  "tb_revision_show_final" },
  { ABIWORD_REVISIONS_FIND_PREV,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_FIND_PREV,  "tb_revision_find_prev" },
  { ABIWORD_REVISIONS_FIND_NEXT,
	AP_STRING_ID_TOOLBAR_LABEL_REVISIONS_FIND_NEXT,  "tb_revision_find_next" },

  { ABIWORD_SEMITEM_THIS,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_THIS,	"tb_semitem_this" },
  { ABIWORD_SEMITEM_NEXT,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_NEXT,	"tb_semitem_next" },
  { ABIWORD_SEMITEM_PREV,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_PREV,	"tb_semitem_prev" },
  { ABIWORD_SEMITEM_EDIT,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_EDIT,	"tb_semitem_edit" },
  { ABIWORD_SEMITEM_STYLESHEET_APPLY,
	AP_STRING_ID_TOOLBAR_LABEL_SEMITEM_STYLESHEET_APPLY,	"tb_semitem_stylesheet_apply" },

  { nullptr,
	0, 											nullptr }
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
  { nullptr, 					0,					nullptr }
};

/*!
 * Register AbiWord's gtk stock icons.
 */
void
abi_stock_init (void)
{
	static gboolean is_initialized = FALSE;
	if (is_initialized) {
		return;
	}
	is_initialized = TRUE;

	gtk_icon_theme_add_resource_path(gtk_icon_theme_get_default(), "/com/abisource/AbiWord");
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
		xxx_UT_DEBUGMSG(("abi_stock_get_gtk_stock_id returned nullptr for stock_id: %s\n", stock_id));
	}

	return stock_id;
}

/*!
 * Map AbiWord stock id to gtk counterpart.
 * Returned string is static.
 */
const gchar *
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

	i = 0;
	while (stock_entries[i].abi_stock_id) {
		if (strcmp(abi_stock_id, stock_entries[i].abi_stock_id) == 0) {
			return stock_entries[i].name;
		}
		i++;
	}

	return nullptr;
}
