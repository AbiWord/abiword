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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

// Note: if the tooltip is blank, the status message will be used as the
// Note: tooltip.  therefore, we probably don't need most tooltip strings
// Note: here -- unless the status message is too long to look good in
// Note: a tooltip.

// Note: the icon field should not be localized unless absolutely necessary.
// Note: the icon name here is to a specific icon (pixmap or bitmap or whatever)
// Note: that will always be in the application.  if, for example, a big fat 'B'
// Note: for BOLD doesn't make sense in another language, change the entry in
// Note: the localization and add the icon to whatever table.

// Note: if a tool item does not use an icon (like a combo box), use the
// Note: constant "NoIcon" in that column.

// If the third argument is UT_TRUE, then this is the fall-back for
// this language (named in the first argument).

BeginSet(ja,JP,UT_TRUE)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"新規", 	        tb_new_xpm,		NULL, "新規に文書を作成します")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"開く",		        tb_open_xpm,	NULL, "既存の文書を開きます")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"保存", 	        tb_save_xpm,	NULL, "文書を保存します")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"名前を付けて保存", tb_save_as_xpm,	NULL, "文書に別の名前をつけて保存します")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"印刷",	            tb_print_xpm,	NULL, "文書を印刷します")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"アンドゥ",	tb_undo_xpm,	NULL, "編集する前に戻します")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"リドゥ",	tb_redo_xpm,	NULL, "一つ前に戻します")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"切り取り",	tb_cut_xpm,		NULL, "切り取り")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"複写",		tb_copy_xpm,	NULL, "複写")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"貼り付け",	tb_paste_xpm,	NULL, "貼り付け")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"スタイル",	      NoIcon,			        NULL, "スタイル")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"フォント",		  NoIcon,			        NULL, "フォント")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"フォントサイズ", NoIcon,		            NULL, "フォントのサイズ")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"太字",		      tb_text_bold_xpm,		    NULL, "太字")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"イタリック",	  tb_text_italic_xpm,	    NULL, "イタリック")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"下線",           tb_text_underline_xpm,	NULL, "下線")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"上線",           tb_text_overline_xpm,	    NULL, "上線")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"取り消し線",     tb_text_strikeout_xpm,	NULL, "取り消し線")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"上付き",	      tb_text_superscript_xpm,	NULL, "上付き文字")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"下付き",	      tb_text_subscript_xpm,		NULL, "下付き文字")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"シンボル",	      tb_symbol_xpm,		NULL, "シンボルの挿入")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"左",	tb_text_align_left_xpm,		NULL, "左詰めに配置します")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"中央",	tb_text_center_xpm,	NULL, "中央詰めに配置します")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"右",	tb_text_align_right_xpm,	NULL, "右詰めに配置します")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"均等",	tb_text_justify_xpm,	NULL, "均等に配置します")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"None before",		tb_para_0before_xpm,	NULL, "Space before: None")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt before",		tb_para_12before_xpm,	NULL, "Space before: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Single Spacing",	tb_line_single_space_xpm,	NULL, "Single spacing")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 Spacing",		tb_line_middle_space_xpm,	NULL, "1.5 spacing")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Double Spacing",	tb_line_double_space_xpm,	NULL, "Double spacing")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 桁",		tb_1column_xpm,			NULL, "1 桁")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 桁",		tb_2column_xpm,			NULL, "2 桁")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 桁",		tb_3column_xpm,			NULL, "3 桁")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"ズーム",		    NoIcon,			            NULL, "ズーム")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Bullet 一覧",		tb_lists_bullets_xpm,		NULL, "開始/終端 Bullet 一覧")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"番号付け一覧",		tb_lists_numbers_xpm,		NULL, "開始/終端 番号付け一覧")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
