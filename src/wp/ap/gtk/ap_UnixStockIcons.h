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

#ifndef AP_UNIXSTOCKICONS_H
#define AP_UNIXSTOCKICONS_H

#include <gtk/gtk.h>
#include "xap_Types.h"

#define ABIWORD_STOCK_PREFIX			"abiword"

#define ABIWORD_FILE_NEW				"abiword-file-new"
#define ABIWORD_FILE_OPEN				"abiword-file-open"
#define ABIWORD_FILE_SAVE				"abiword-file-save"
#define ABIWORD_FILE_SAVEAS				"abiword-file-saveas"
#define ABIWORD_FILE_REVERT				"abiword-file-revert"		// GTK_STOCK_REVERT_TO_SAVED
#define ABIWORD_FILE_PRINT				"abiword-file-print"
#define ABIWORD_FILE_PRINT_PREVIEW		"abiword-file-print-preview"
#define ABIWORD_FILE_PROPERTIES			"abiword-file-properties"	// GTK_STOCK_PROPERTIES
#define ABIWORD_FILE_CLOSE				"abiword-file-close"
#define ABIWORD_FILE_EXIT				"abiword-file-exit"			// GTK_STOCK_QUIT

#define ABIWORD_SPELLCHECK				"abiword-spellcheck"

#define ABIWORD_EDIT_CUT				"abiword-edit-cut"
#define ABIWORD_EDIT_COPY				"abiword-edit-copy"
#define ABIWORD_EDIT_PASTE				"abiword-edit-paste"
#define ABIWORD_FMTPAINTER				"abiword-fmtpainter"
#define ABIWORD_EDIT_UNDO				"abiword-edit-undo"
#define ABIWORD_EDIT_REDO				"abiword-edit-redo"
#define ABIWORD_EDIT_CLEAR				"abiword-edit-clear"		// GTK_STOCK_CLEAR
#define ABIWORD_EDIT_FIND				"abiword-edit-find"			// GTK_STOCK_FIND
#define ABIWORD_EDIT_REPLACE 			"abiword-edit-replace"		// GTK_STOCK_FIND_AND_REPLACE
#define ABIWORD_EDIT_GOTO				"abiword-edit-goto"			// GTK_STOCK_JUMP_TO
#define ABIWORD_TOOLS_OPTIONS			"abiword-tools-options"		// GTK_STOCK_PREFERENCES

#define ABIWORD_1COLUMN					"abiword-1column"
#define ABIWORD_2COLUMN					"abiword-2column"
#define ABIWORD_3COLUMN					"abiword-3column"

#define ABIWORD_IMG						"abiword-img"
#define ABIWORD_VIEW_SHOWPARA			"abiword-view-showpara"
#define ABIWORD_HELP					"abiword-help"
#define ABIWORD_HELP_ABOUT				"abiword-help-about"		// GTK_STOCK_ABOUT
#define ABIWORD_FMT_FONT				"abiword-fmt-font"
#define ABIWORD_FMT_BOLD				"abiword-fmt-bold"
#define ABIWORD_FMT_ITALIC				"abiword-fmt-italic"
#define ABIWORD_FMT_UNDERLINE			"abiword-fmt-underline"

#define ABIWORD_ALIGN_LEFT				"abiword-align-left"
#define ABIWORD_ALIGN_CENTER			"abiword-align-center"
#define ABIWORD_ALIGN_RIGHT				"abiword-align-right"
#define ABIWORD_ALIGN_JUSTIFY			"abiword-align-justify"

#define ABIWORD_LISTS_NUMBERS			"abiword-lists-numbers"
#define ABIWORD_LISTS_BULLETS			"abiword-lists-bullets"

#define ABIWORD_UNINDENT				"abiword-unindent"
#define ABIWORD_INDENT					"abiword-indent"

#define ABIWORD_COLOR_BACK				"abiword-color_back"
#define ABIWORD_COLOR_FORE				"abiword-color_fore"

#define ABIWORD_INSERT_TABLE			"abiword-insert-table"
#define ABIWORD_ADD_ROW					"abiword-add-row"
#define ABIWORD_ADD_COLUMN				"abiword-add-column"
#define ABIWORD_DELETE_ROW				"abiword-delete-row"
#define ABIWORD_DELETE_COLUMN			"abiword-delete-column"
#define ABIWORD_MERGE_CELLS				"abiword-merge-cells"
#define ABIWORD_SPLIT_CELLS				"abiword-split-cells"

#define ABIWORD_FMT_HYPERLINK			"abiword-fmt-hyperlink"
#define ABIWORD_FMT_BOOKMARK			"abiword-fmt-bookmark"
#define ABIWORD_FMT_OVERLINE			"abiword-fmt-overline"
#define ABIWORD_FMT_STRIKE				"abiword-fmt-strike"
#define ABIWORD_FMT_SUPERSCRIPT			"abiword-fmt-superscript"
#define ABIWORD_FMT_SUBSCRIPT			"abiword-fmt-subscript"

#define ABIWORD_INSERT_SYMBOL			"abiword-insert-symbol"
#define ABIWORD_SCRIPT_PLAY				"abiword-script-play"

#define ABIWORD_PARA_0BEFORE			"abiword-para-0before"
#define ABIWORD_PARA_12BEFORE			"abiword-para-12before"

#define ABIWORD_SINGLE_SPACE			"abiword-single-space"
#define ABIWORD_MIDDLE_SPACE			"abiword-middle-space"
#define ABIWORD_DOUBLE_SPACE			"abiword-double-space"

#define ABIWORD_FMT_DIR_OVERRIDE_LTR 	"abiword-fmt-dir-override-ltr"
#define ABIWORD_FMT_DIR_OVERRIDE_RTL 	"abiword-fmt-dir-override-rtl"
#define ABIWORD_FMT_DOM_DIRECTION		"abiword-fmt-dom-direction"

#define ABIWORD_EDIT_HEADER				"abiword-edit-header"
#define ABIWORD_EDIT_FOOTER				"abiword-edit-footer"
#define ABIWORD_EDIT_REMOVEHEADER		"abiword-edit-removeheader"
#define ABIWORD_EDIT_REMOVEFOOTER		"abiword-edit-removefooter"

#define ABIWORD_FMT_CHOOSE              "abiword-fmt-choose"
#define ABIWORD_VIEW_FULL_SCREEN        "abiword-view-full-screen"

#define ABIWORD_REVISIONS_NEW			"abiword-revisions-new"
#define ABIWORD_REVISIONS_SELECT		"abiword-revisions-select"
#define ABIWORD_REVISIONS_SHOW_FINAL	"abiword-revisions-show-final"
#define ABIWORD_REVISIONS_FIND_PREV  	"abiword-revisions-find-prev"
#define ABIWORD_REVISIONS_FIND_NEXT  	"abiword-revisions-find-next"

#define ABIWORD_SEMITEM_THIS            "abiword-semitem-this"
#define ABIWORD_SEMITEM_NEXT            "abiword-semitem-next"
#define ABIWORD_SEMITEM_PREV            "abiword-semitem-prev"
#define ABIWORD_SEMITEM_EDIT            "abiword-semitem-edit"
#define ABIWORD_SEMITEM_STYLESHEET_APPLY "abiword-semitem-stylesheet-apply"

void		  abi_stock_init 				(void);
gchar * 	  abi_stock_from_toolbar_id 	(const gchar *toolbar_id);

#endif /* AP_UNIXSTOCKICONS_H */
