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

/*
 * Traditional Chinese Translation by
 *
 *  Chinese Linux Extensions Project  http://cle.linux.org.tw/
 *     Chih-Wei Huang <cwhuang@linux.org.tw>
 *     Armani         <armani@cle.linux.org.tw>
 *  ThizLinux Laboratory Ltd.  http://www.thizlinux.com/
 *     Anthony Fok <anthony@thizlinux.com>
 *
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

// If the third argument is true, then this is the fall-back for
// this language (named in the first argument).

BeginSetEnc(zh,TW,true,"BIG5")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,		NULL,		NULL)

	//          (id,				szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"新增", 	tb_new_xpm,	NULL, "建立一份新的文件")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"開啟",		tb_open_xpm,	NULL, "開啟現有文件")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"儲存", 	tb_save_xpm,	NULL, "儲存文件")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"另存新檔", 	tb_save_as_xpm,	NULL, "將文件另存新檔")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"列印",		tb_print_xpm,	NULL, "印出這份文件")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,	"預覽列印",	tb_print_preview_xpm, NULL, "列印前先在螢幕上預覽列印效果")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"復原",		tb_undo_xpm,	NULL, "復原")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"重做",		tb_redo_xpm,	NULL, "重做")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"剪下",		tb_cut_xpm,	NULL, "剪下")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"複製",		tb_copy_xpm,	NULL, "複製")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"貼上",		tb_paste_xpm,	NULL, "貼上")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_HEADER,		"編輯頁首",	tb_edit_editheader_xpm,		NULL, "編輯頁首")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_FOOTER,		"編輯頁尾",	tb_edit_editfooter_xpm,		NULL, "編輯頁尾")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEHEADER,	"移除頁首",	tb_edit_removeheader_xpm,	NULL, "移除頁首")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEFOOTER,	"移除頁尾",	tb_edit_removefooter_xpm,	NULL, "移除頁尾")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"拼字檢查",	tb_spellcheck_xpm,		NULL, "為文件進行拼字檢查")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,			"插入影像",	tb_insert_graphic_xpm,	NULL, "把一張影像插入文件裏")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"樣式",		NoIcon,		NULL, "樣式")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"字型",		NoIcon,		NULL, "字型")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_HYPERLINK,	"插入超連結", tb_hyperlink, NULL, "把一個超連結插入文件裏")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOOKMARK,	"插入書籤", tb_anchor, NULL, "把一個書籤插入文件裏")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"字型大小", 	NoIcon,		NULL, "字型大小")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"粗體",		tb_text_bold_xpm,	NULL, "粗體")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"斜體",		tb_text_italic_xpm,	NULL, "斜體")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"底線",		tb_text_underline_xpm,	NULL, "底線")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"上標線",	tb_text_overline_xpm,	NULL, "上標線")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"刪除線", 	tb_text_strikeout_xpm,	NULL, "刪除線")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_TOPLINE,		"行頂加線",	tb_text_topline_xpm,	NULL, "行頂加線")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOTTOMLINE,	"行底加線",	tb_text_bottomline_xpm,	NULL, "行底加線")
	ToolbarLabel(AP_TOOLBAR_ID_HELP,		"說明",		tb_help_xpm,		NULL, "說明")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"上標字",	tb_text_superscript_xpm,	NULL, "上標字")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"下標字",	tb_text_subscript_xpm,		NULL, "下標字")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"符號",		tb_symbol_xpm,		NULL, "插入符號")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"左",		tb_text_align_left_xpm,		NULL, "靠左對齊")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"中",		tb_text_center_xpm,	NULL, "置中對齊")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"右",		tb_text_align_right_xpm,	NULL, "靠右對齊")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"左右",		tb_text_justify_xpm,	NULL, "左右對齊")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"與前段距離: 無",	tb_para_0before_xpm,	NULL, "與前段距離: 無")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"與前段距離: 12 點",	tb_para_12before_xpm,	NULL, "與前段距離: 12 點")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"單行間距",	tb_line_single_space_xpm,	NULL, "單行間距")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 倍行高",	tb_line_middle_space_xpm,	NULL, "1.5 倍行高")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"兩倍行高",	tb_line_double_space_xpm,	NULL, "兩倍行高")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,		"一欄",		tb_1column_xpm,			NULL, "一欄")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,		"兩欄",		tb_2column_xpm,			NULL, "兩欄")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,		"三欄",		tb_3column_xpm,			NULL, "三欄")

	ToolbarLabel(AP_TOOLBAR_ID_VIEW_SHOWPARA,	"全部顯示",	tb_view_showpara_xpm,	NULL, "顯示/隱藏 段落標記")
	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"縮放比例",	NoIcon,			NULL, "縮放比例")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"項目符號",	tb_lists_bullets_xpm,	NULL, "項目符號")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"編號",		tb_lists_numbers_xpm,	NULL, "編號")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"字型色彩",	tb_text_fgcolor_xpm,	NULL, "字型色彩")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"醒目提示",	tb_text_bgcolor_xpm,	NULL, "醒目提示")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,		"增加縮排",	tb_text_indent_xpm, 	NULL, "增加縮排")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"減小縮排",	tb_text_unindent_xpm,	NULL, "減小縮排")
	ToolbarLabel(AP_TOOLBAR_ID_SCRIPT_PLAY,		"執行 script",	tb_script_play_xpm, 	NULL, "執行 script")

#ifdef BIDI_ENABLED
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR,	"強制文字左至右排",	tb_text_direction_ltr_xpm,	NULL, "強制文字從左至右排列")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL,        "強制文字右至左排",	tb_text_direction_rtl_xpm,	NULL, "強制文字從右至左排列")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,	"段落方向",	tb_text_dom_direction_rtl_xpm,	NULL, "變更段落文字的主要方向")
#endif
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
