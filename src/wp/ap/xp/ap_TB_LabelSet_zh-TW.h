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

BeginSet(zh,TW,true)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"新增", 	tb_new_xpm,		NULL, "建立一份新的文件")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"開啟",		tb_open_xpm,	NULL, "開啟現有文件")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"儲存", 	tb_save_xpm,	NULL, "儲存文件")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"另存新檔", 	tb_save_as_xpm,	NULL, "將文件另存新檔")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"列印",		tb_print_xpm,	NULL, "印出這份文件")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"復原",		tb_undo_xpm,	NULL, "復原")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"重做",		tb_redo_xpm,	NULL, "重做")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"剪下",		tb_cut_xpm,		NULL, "剪下")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"複製",		tb_copy_xpm,	NULL, "複製")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"貼上",	tb_paste_xpm,	NULL, "貼上")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"格式",		NoIcon,			NULL, "格式")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"字型",		NoIcon,			NULL, "字型")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"字型大小", 	NoIcon,		NULL, "字型大小")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"粗體",		tb_text_bold_xpm,	NULL, "粗體")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"斜體",	tb_text_italic_xpm,	NULL, "斜體")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"底線",tb_text_underline_xpm,	NULL, "底線")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"上標線",tb_text_overline_xpm,	NULL, "上標線")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"中間線",   tb_text_strikeout_xpm,	NULL, "中間線")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"上標字",	tb_text_superscript_xpm,	NULL, "上標字")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"下標字",	tb_text_subscript_xpm,		NULL, "下標字")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"左",		tb_text_align_left_xpm,		NULL, "靠左對齊")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"中",		tb_text_center_xpm,	NULL, "置中對齊")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"右",	tb_text_align_right_xpm,	NULL, "靠右對齊")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"調整",	tb_text_justify_xpm,	NULL, "調整對齊")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"與前段距離: 無",		tb_para_0before_xpm,	NULL, "與前段距離: 無")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"與前段距離: 12 點",		tb_para_12before_xpm,	NULL, "與前段距離: 12 點")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"單一行高",	tb_line_single_space_xpm,	NULL, "單一行高")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 倍行高",		tb_line_middle_space_xpm,	NULL, "1.5 倍行高")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"兩倍行高",	tb_line_double_space_xpm,	NULL, "兩倍行高")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 欄",			tb_1column_xpm,			NULL, "1 欄")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 欄",		tb_2column_xpm,			NULL, "2 欄")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 欄",		tb_3column_xpm,			NULL, "3 欄")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"縮放",		NoIcon,			NULL, "縮放")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
