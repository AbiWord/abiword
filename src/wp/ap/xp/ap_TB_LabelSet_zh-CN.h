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

// Translation by  hj <huangj@citiz.net>

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

BeginSet(zh,CN,true)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"New", 		tb_new_xpm,		NULL, "\xc9\xfa\xb3\xc9\xd0\xc2\xce\xc4\xb5\xb5")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Open",		tb_open_xpm,	NULL, "\xb4\xf2\xbf\xaa\xce\xc4\xb5\xb5")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Save", 	tb_save_xpm,	NULL, "\xb1\xa3\xb4\xe6\xce\xc4\xb5\xb5")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Save As", 	tb_save_as_xpm,	NULL, "\xc1\xed\xb4\xe6\xce\xc4\xb5\xb5")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Print",	tb_print_xpm,	NULL, "\xb4\xf2\xd3\xa1\xce\xc4\xb5\xb5")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Undo",		tb_undo_xpm,	NULL, "\xbb\xd6\xb8\xb4\xb1\xe0\xbc\xad")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Redo",		tb_redo_xpm,	NULL, "\xd6\xd8\xd7\xf6\xb1\xe0\xbc\xad")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Cut",		tb_cut_xpm,		NULL, "\xbc\xf4\xc7\xd0")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Copy",		tb_copy_xpm,	NULL, "\xb8\xb4\xd6\xc6")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Paste",	tb_paste_xpm,	NULL, "\xd5\xb3\xcc\xf9")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Style",	NoIcon,			NULL, "\xd7\xd6\xd0\xcd")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Font",		NoIcon,			NULL, "\xd7\xd6\xcc\xe5")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Font Size", NoIcon,		NULL, "\xd7\xd6\xcc\xe5\xb4\xf3\xd0\xa1")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Bold",		tb_text_bold_xpm,		NULL, "\xb4\xd6\xcc\xe5")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Italic",	tb_text_italic_xpm,	NULL, "\xd0\xb1\xcc\xe5")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Underline",tb_text_underline_xpm,	NULL, "\xcf\xc2\xbb\xae\xcf\xdf")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Overline",tb_text_overline_xpm,	NULL, "\xc9\xcf\xbb\xae\xcf\xdf")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Strike",   tb_text_strikeout_xpm,	NULL, "\xd6\xd0\xbb\xae\xcf\xdf")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Superscript",	tb_text_superscript_xpm,	NULL, "\xc9\xcf\xb1\xea")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Subscript",	tb_text_subscript_xpm,		NULL, "\xcf\xc2\xb1\xea")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Left",		tb_text_align_left_xpm,		NULL, "\xd7\xf3\xb6\xd4\xc6\xeb")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Center",	tb_text_center_xpm,	NULL, "\xd6\xd0\xbc\xe4\xb6\xd4\xc6\xeb")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Right",	tb_text_align_right_xpm,	NULL, "\xd3\xd2\xb6\xd4\xc6\xeb")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Justify",	tb_text_justify_xpm,	NULL, "\xc1\xbd\xb6\xcb\xb6\xd4\xc6\xeb")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"None before",		tb_para_0before_xpm,	NULL, "\xb6\xce\xc7\xb0\xbc\xe4\xbe\xe0:\xce\xde")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt before",		tb_para_12before_xpm,	NULL, "\xb6\xce\xc7\xb0\xbc\xe4\xbe\xe0:12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Single Spacing",	tb_line_single_space_xpm,	NULL, "\xb5\xa5\xb1\xb6\xd0\xd0\xbe\xe0")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 Spacing",		tb_line_middle_space_xpm,	NULL, "1.5\xb1\xb8\xd0\xd0\xbe\xe0")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Double Spacing",	tb_line_double_space_xpm,	NULL, "\xcb\xab\xb1\xb6\xd0\xd0\xbe\xe0")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 Column",			tb_1column_xpm,			NULL, "1\xc0\xb8")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 Columns",		tb_2column_xpm,			NULL, "2\xc0\xb8")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 Columns",		tb_3column_xpm,			NULL, "3\xc0\xb8")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Zoom",		NoIcon,			NULL, "\xcb\xf5\xb7\xc5")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()

