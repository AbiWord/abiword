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

BeginSet(ga,IE,true)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"&Nua", 		tb_new_xpm,		NULL, "Create an comhad nua")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"&Oscail",		tb_open_xpm,	NULL, "Oscail comhad")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"&Sábhál", 	tb_save_xpm,	NULL, "Sábhál an comhad")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Sábhál &Mar", 	tb_save_as_xpm,	NULL, "Sábhál an comhad mar an t-ainm nua")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Cló",	tb_print_xpm,	NULL, "Cló an comhad")
#ifdef HAVE_GNOME
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW, "Réamhthaispeántas Cló", tb_print_preview_xpm, NULL, "Preview the document before cló")
#endif

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Fo-dhean",		tb_undo_xpm,	NULL, "Fo-dhea editing")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Athdhéan",		tb_redo_xpm,	NULL, "Athdhéan editing")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Gearr",		tb_cut_xpm,		NULL, "Gearr")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"&Cóip",		tb_copy_xpm,	NULL, "Cóip")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Greamaigh",	tb_paste_xpm,	NULL, "Greamaigh")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Stíl",	NoIcon,			NULL, "Stíl")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Font",		NoIcon,			NULL, "Font")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Font Size", NoIcon,		NULL, "Font Size")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Cló Dubh",		tb_text_bold_xpm,		NULL, "Cló dubh")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"&Iodáileach",	tb_text_italic_xpm,	NULL, "Cló iodáileach")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Lín Faoi",tb_text_underline_xpm,	NULL, "Lín faoi")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Lín Thar",tb_text_overline_xpm,	NULL, "Lín thar")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Lín Tríd",   tb_text_strikeout_xpm,	NULL, "Lín trid")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Superscript",	tb_text_superscript_xpm,	NULL, "Superscript")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Subscript",	tb_text_subscript_xpm,		NULL, "Subscript")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Siombal",	tb_symbol_xpm,		NULL, "Cur siombal")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,	"Clé",	tb_text_align_left_xpm,		NULL, "alignment clé")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Lár",	tb_text_center_xpm,	NULL, "alignment lár")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,	"Deis",	tb_text_align_right_xpm,	NULL, "alignment deis")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Comhfhadaigh",	tb_text_justify_xpm,	NULL, "Comhfhadaigh paragraf")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Dada roimh",		tb_para_0before_xpm,	NULL, "Space before: Dada")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt roimh",		tb_para_12before_xpm,	NULL, "Space before: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Spásáil Singil",	tb_line_single_space_xpm,	NULL, "Spásáil singil")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"Spásáil 1.5",	tb_line_middle_space_xpm,	NULL, "Spásáil 1.5")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Spásáil Dhúbailte",	tb_line_double_space_xpm,	NULL, "Spásáil dhúbailte")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 Colún",		tb_1column_xpm,			NULL, "1 Colún")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 Colún",		tb_2column_xpm,			NULL, "2 Colún")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 Colún",		tb_3column_xpm,			NULL, "3 Colún")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Feith",		NoIcon,			NULL, "Feith")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,		"Liosta Piléaraithe",		tb_lists_bullets_xpm,		NULL,		"Thosaigh/Stop Liosta Piléaraithe")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,		"Liosta Uimhrithe",		tb_lists_numbers_xpm,		NULL,		"Thosaigh/Stop Liosta Uimhrithe")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,          "Dath an Cúlra",     NoIcon,                 NULL, "Athraigh Dath an Cúlra")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,          "Dath an Réamhionad",     NoIcon,                 NULL, "Athraigh Dath an Réamhionad")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT, "Eangaigh Paragraf", tb_text_indent_xpm, NULL, "Eangaigh Paragraf")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT, "Bhain Eanganna as Paragraf", tb_text_unindent_xpm, NULL, "Bhain Eanganna as Paragraf")

	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
