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

// Swedish translations provided by Henrik Berg <henrik@lansen.se>

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

BeginSet(sv,SE,UT_TRUE)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,			NoIcon,					NULL, NULL)

	//          (id,						szLabel,		IconName,					szToolTip, szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Nytt", 		tb_new_xpm,					NULL, "Skapar ett nytt tomt dokument")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Öppna",		tb_open_xpm,				NULL, "Öppnar ett befintligt dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Spara", 		tb_save_xpm,				NULL, "Sparar dokumentet")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Spara...", 	tb_save_as_xpm, 				NULL, "Sparar dokumentet med ett nytt namn")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Skriv ut",		tb_print_xpm,				NULL, "Skriv ut dokumentet")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Ångra",		tb_undo_xpm,				NULL, "Ångra inmatningen")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Upprepa",		tb_redo_xpm,				NULL, "Upprepa inmatningen")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Klipp ut",		tb_cut_xpm,					NULL, "Klipp ut")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Kopiera",		tb_copy_xpm,				NULL, "Kopiera")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Klistra in", 	tb_paste_xpm,				NULL, "Klistra in")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Stil",		NoIcon,					NULL, "Stil")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Teckensnitt",	NoIcon,					NULL, "Teckensnitt")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Teckenstorlek",	NoIcon,					NULL, "Teckenstorlek")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Fet",		tb_text_bold_F_xpm,			NULL, "Fet")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Kursiv",		tb_text_italic_K_xpm,			NULL, "Kursiv")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,		"Under", 		tb_text_underline_xpm,			NULL, "Understruken")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,		"Över",		tb_text_overline_xpm,			NULL, "Överstruken")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Genom", 		tb_text_strikeout_G_xpm,		NULL, "Genomstruken")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Upphöjd",		tb_text_superscript_xpm,		NULL, "Upphöjd")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,		"Nedsänkt",		tb_text_subscript_xpm,			NULL, "Nedsänkt")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,		"Symbol",		tb_symbol_xpm,				NULL, "Infoga symbol")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Vänster",		tb_text_align_left_xpm,			NULL, "Vänsterjustera")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,		"Centrera", 	tb_text_center_xpm,			NULL, "Centrera")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Höger",		tb_text_align_right_xpm,		NULL, "Högerjustera")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,		"Marginal", 	tb_text_justify_xpm,			NULL, "Marginaljustera")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,		"Inget före", 	tb_para_0before_xpm,			NULL, "Mellanrum före: Inget")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,		"12pt före", 	tb_para_12before_xpm,			NULL, "Mellanrum före: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,		"Enkelt",		tb_line_single_space_xpm,		NULL, "Enkelt radavstånd")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,		"1,5 rad",		tb_line_middle_space_xpm,		NULL, "1,5 radavstånd")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,		"Dubbelt",		tb_line_double_space_xpm,		NULL, "Dubbelt radavstånd")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 spalt",		tb_1column_xpm,				NULL, "1 spalt")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 spalter",	tb_2column_xpm,				NULL, "2 spalter")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 spalter",	tb_3column_xpm,				NULL, "3 spalter")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Zoom",		NoIcon,					NULL, "Zoom")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,		"Punktlista",	tb_lists_bullets_xpm,			NULL,	"Start/Stopp punktlista")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,		"Numreradlista",	tb_lists_numbers_xpm,			NULL,	"Start/Stopp numreradlista")

	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,			NoIcon,						NULL, NULL)

EndSet()
