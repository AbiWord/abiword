/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

BeginSet(EnUS)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"New", 		tb_new_xpm,		NULL, "Create a new document")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Open",		tb_open_xpm,	NULL, "Open an existing document")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Save", 	tb_save_xpm,	NULL, "Save the document")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Save As", 	tb_save_as_xpm,	NULL, "Save the document under a different name")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Print",	tb_print_xpm,	NULL, "Print the document")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Undo",		tb_undo_xpm,	NULL, "Undo editing")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Redo",		tb_redo_xpm,	NULL, "Redo editing")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Cut",		tb_cut_xpm,		NULL, "Cut")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Copy",		tb_copy_xpm,	NULL, "Copy")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Paste",	tb_paste_xpm,	NULL, "Paste")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Style",	NoIcon,			NULL, "Style")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Font",		NoIcon,			NULL, "Font")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Font Size", NoIcon,		NULL, "Font Size")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Bold",		tb_text_bold_xpm,		NULL, "Bold")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Italic",	tb_text_italic_xpm,	NULL, "Italic")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Underline",tb_text_underline_xpm,	NULL, "Underline")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Strike",   tb_text_strikeout_xpm,	NULL, "Strikeout")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Superscript",	tb_text_superscript_xpm,	NULL, "Superscript")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Subscript",	tb_text_subscript_xpm,		NULL, "Subscript")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Left",		tb_text_align_left_xpm,		NULL, "Left alignment")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Center",	tb_text_center_xpm,	NULL, "Center alignment")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Right",	tb_text_align_right_xpm,	NULL, "Right alignment")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Justify",	tb_text_justify_xpm,	NULL, "Justify paragraph")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"None before",		tb_para_0before_xpm,	NULL, "Space before: None")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt before",		tb_para_12before_xpm,	NULL, "Space before: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Single Spacing",	tb_line_single_space_xpm,	NULL, "Single spacing")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 Spacing",		tb_line_middle_space_xpm,	NULL, "1.5 spacing")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Double Spacing",	tb_line_double_space_xpm,	NULL, "Double spacing")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 Column",			tb_1column_xpm,			NULL, "1 Column")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 Columns",		tb_2column_xpm,			NULL, "2 Columns")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 Columns",		tb_3column_xpm,			NULL, "3 Columns")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Zoom",		NoIcon,			NULL, "Zoom")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
