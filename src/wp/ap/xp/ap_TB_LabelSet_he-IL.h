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

BeginSetEnc(he,IL,false,"iso-8859-8")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,			NoIcon,					NULL,NULL)

	//          (id, 		                    szLabel,		IconName,     			szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"���", 			tb_new_xpm,				NULL, "����� ���� ���")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"�����",			tb_open_xpm,			NULL, "����� ���� ����")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"����", 		tb_save_xpm,			NULL, "����� ����� �����")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"����� ���", 		tb_save_as_xpm,			NULL, "����� ����� ����� �� �� ���")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"�����",		tb_print_xpm,			NULL, "����� ����� �����")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,	"��� ���� �����",	tb_print_preview_xpm, NULL, "Preview the document before printing")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"��� �����",			tb_undo_xpm,			NULL, "����� ������ �������")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"���� �� �����",			tb_redo_xpm,			NULL, "���� �� ������ �������")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"����",			tb_cut_xpm,				NULL, "����")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"����",			tb_copy_xpm,			NULL, "����")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"����",		tb_paste_xpm,			NULL, "����")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"���� ����",	tb_spellcheck_xpm,		NULL, "���� ���� �����")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,				"���� �����",	tb_insert_graphic_xpm,	NULL, "���� ����� �����")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"�����",		NoIcon,					NULL, "�����")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"����",			NoIcon,					NULL, "����")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Font Size",	NoIcon,					NULL, "Font Size")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"�����",			tb_text_bold_xpm,		NULL, "�����")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"����",		tb_text_italic_xpm,		NULL, "����")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"�� �����",	tb_text_underline_xpm,	NULL, "�� �����")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"�� �����",		tb_text_overline_xpm,	NULL, "�� �����")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"�� ����",		tb_text_strikeout_xpm,	NULL, "�� ����")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_TOPLINE,		"Topline",		tb_text_topline_xpm,	NULL, "Topline")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOTTOMLINE,		"Bottomline",		tb_text_bottomline_xpm,	NULL, "Bottomline")
	ToolbarLabel(AP_TOOLBAR_ID_HELP,			"����",			tb_help_xpm,			NULL, "����")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"��� ����",	tb_text_superscript_xpm,	NULL, "��� ����")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"��� ����",	tb_text_subscript_xpm,		NULL, "��� ����")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"����",		tb_symbol_xpm,				NULL, "���� ����")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"�����",			tb_text_align_left_xpm,		NULL, "����� �����")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"������",		tb_text_center_xpm,			NULL, "������")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"�����",		tb_text_align_right_xpm,	NULL, "����� �����")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"����� ��-����",		tb_text_justify_xpm,		NULL, "����� ��-����")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"None before",		tb_para_0before_xpm,	NULL, "Space before: None")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt before",		tb_para_12before_xpm,	NULL, "Space before: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"����",	tb_line_single_space_xpm,	NULL, "����")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"���� ����",		tb_line_middle_space_xpm,	NULL, "���� ����")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"����",	tb_line_double_space_xpm,	NULL, "����")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"��� ���",			tb_1column_xpm,			NULL, "��� ���")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"��� �����",		tb_2column_xpm,			NULL, "��� �����")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"����� �����",		tb_3column_xpm,			NULL, "����� �����")

	ToolbarLabel(AP_TOOLBAR_ID_VIEW_SHOWPARA,	"��� ���",			tb_view_showpara_xpm,		NULL, "Show/hide �")
	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"���� ������",				NoIcon,					NULL, "���� ������")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"�������",			tb_lists_bullets_xpm,	NULL, "�������")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"�����",		tb_lists_numbers_xpm,	NULL, "�����")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"��� ����",		tb_text_fgcolor_xpm,	NULL, "��� ����")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Highlight",		tb_text_bgcolor_xpm,	NULL, "Highlight")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,			"���� �����",	tb_text_indent_xpm, 	NULL, "���� �����")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"���� �����",	tb_text_unindent_xpm,	NULL, "���� �����")

	ToolbarLabel(AP_TOOLBAR_ID_SCRIPT_PLAY,		"���� script",	tb_script_play_xpm,		NULL, "���� script")

     // ... add others here ...
#ifdef BIDI_ENABLED
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR,	"��� ����� �����",		tb_text_direction_ltr_xpm,	NULL, "��� ����� ����� �� �����")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL,	"��� ����� ����",		tb_text_direction_rtl_xpm,	NULL, "��� ����� ���� �� �����")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,		"����� �����",	tb_text_dom_direction_rtl_xpm,	NULL, "��� ����� ����� ����")
#endif
	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,			NoIcon,			NULL,NULL)

EndSet()
