/* AbiWord
 * Copyright (C) 1998-2001 AbiSource, Inc.
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

/* Translation by  hj <huangj@citiz.net>
 *                 ThizLinux Laboratory Ltd.  http://www.thizlinux.com/
 *                 Anthony Fok <anthony@thizlinux.com>
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

BeginSetEnc(zh,CN,true,"GB2312")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"New", 		tb_new_xpm,	NULL, "生成新文档")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Open",		tb_open_xpm,	NULL, "打开文档")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Save", 	tb_save_xpm,	NULL, "保存文档")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Save As", 	tb_save_as_xpm,	NULL, "另存文档")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Print",	tb_print_xpm,	NULL, "打印文档")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,	"Print Preview",	tb_print_preview_xpm, NULL, "打印前先在屏幕上预览打印效果")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Undo",		tb_undo_xpm,	NULL, "恢复编辑")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Redo",		tb_redo_xpm,	NULL, "重做编辑")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Cut",		tb_cut_xpm,	NULL, "剪切")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Copy",		tb_copy_xpm,	NULL, "复制")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Paste",	tb_paste_xpm,	NULL, "粘贴")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_HEADER,		"编辑页眉",	tb_edit_editheader_xpm,		NULL, "编辑页眉")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_FOOTER,		"编辑页脚",	tb_edit_editfooter_xpm,		NULL, "编辑页脚")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEHEADER,	"移除页眉",	tb_edit_removeheader_xpm,	NULL, "移除页眉")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEFOOTER,	"移除页脚",	tb_edit_removefooter_xpm,	NULL, "移除页脚")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"拼写检查",	tb_spellcheck_xpm,		NULL, "为文档进行拼写检查")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,			"插入影像",	tb_insert_graphic_xpm,		NULL, "把一张影像插入文档里")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Style",	NoIcon,		NULL, "样式")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Font",		NoIcon,		NULL, "字体")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_HYPERLINK,	"插入超链接",	tb_hyperlink,	NULL, "把一个超链接插入文档里")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOOKMARK,	"插入书签",	tb_anchor,	NULL, "把一个书签插入文档里")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Font Size", NoIcon,		NULL, "字体大小")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Bold",		tb_text_bold_xpm,	NULL, "粗体")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Italic",	tb_text_italic_xpm,	NULL, "斜体")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Underline",tb_text_underline_xpm,	NULL, "下划线")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Overline",tb_text_overline_xpm,	NULL, "上划线")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Strike",   tb_text_strikeout_xpm,	NULL, "中划线")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_TOPLINE,		"行顶线",	tb_text_topline_xpm,	NULL, "行顶线")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOTTOMLINE,	"行底线",	tb_text_bottomline_xpm,	NULL, "行底线")
	ToolbarLabel(AP_TOOLBAR_ID_HELP,		"帮助",		tb_help_xpm,		NULL, "帮助")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Superscript",	tb_text_superscript_xpm,	NULL, "上标")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Subscript",	tb_text_subscript_xpm,		NULL, "下标")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"符号",		tb_symbol_xpm,		NULL, "插入符号")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Left",		tb_text_align_left_xpm,	NULL, "左对齐")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Center",	tb_text_center_xpm,	NULL, "中间对齐")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Right",	tb_text_align_right_xpm,	NULL, "右对齐")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Justify",	tb_text_justify_xpm,	NULL, "两端对齐")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"None before",		tb_para_0before_xpm,	NULL, "段前间距:无")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt before",		tb_para_12before_xpm,	NULL, "段前间距:12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Single Spacing",	tb_line_single_space_xpm,	NULL, "单倍行距")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 Spacing",		tb_line_middle_space_xpm,	NULL, "1.5倍行距")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Double Spacing",	tb_line_double_space_xpm,	NULL, "双倍行距")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,		"1 Column",		tb_1column_xpm,			NULL, "1栏")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,		"2 Columns",		tb_2column_xpm,			NULL, "2栏")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,		"3 Columns",		tb_3column_xpm,			NULL, "3栏")

	ToolbarLabel(AP_TOOLBAR_ID_VIEW_SHOWPARA,	"全部显示",	tb_view_showpara_xpm,	NULL, "显示/隐藏 段落标记")
	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"缩放比例",	NoIcon,			NULL, "缩放比例")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"项目符号",	tb_lists_bullets_xpm,	NULL, "项目符号")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"编号",		tb_lists_numbers_xpm,	NULL, "编号")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"字型颜色",	tb_text_fgcolor_xpm,	NULL, "字体颜色")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"高亮颜色",	tb_text_bgcolor_xpm,	NULL, "高亮颜色")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,		"增加缩进",	tb_text_indent_xpm, 	NULL, "增加缩进")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"减小缩进",	tb_text_unindent_xpm,	NULL, "减小缩进")
	ToolbarLabel(AP_TOOLBAR_ID_SCRIPT_PLAY,		"执行脚本",	tb_script_play_xpm, 	NULL, "执行脚本")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR,	"强制文字左至右排",	tb_text_direction_ltr_xpm,	NULL, "强制文字从左至右排列")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL,        "强制文字右至左排",	tb_text_direction_rtl_xpm,	NULL, "强制文字从右至左排列")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,	"段落方向",	tb_text_dom_direction_rtl_xpm,	NULL, "改变段落文字的主要方向")

	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
