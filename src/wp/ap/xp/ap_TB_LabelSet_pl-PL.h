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

// Polish translated by Sercxemulo <explo@poczta.wp.pl>
// Last update: 07.14.2000A.D.
BeginSet(pl,PL,true)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Nowy", 		tb_new_xpm,		NULL, "Stwórz nowy dokument")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Otwórz",		tb_open_xpm,	NULL, "Otwórz istniej±cy dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Zapisz", 	tb_save_xpm,	NULL, "Zachowaj dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Zapisz jako", 	tb_save_as_xpm,	NULL, "Zachowaj dokument pod inn± nazw±")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Drukuj",	tb_print_xpm,	NULL, "Drukuj dokument")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Odwo³aj",		tb_undo_xpm,	NULL, "Cofnij  zmiany")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Cofnij zmiany",		tb_redo_xpm,	NULL, "Cofnij poprzednie poprawki")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Zachowaj i wytnij",		tb_cut_xpm,		NULL, "Wytnij i zachowaj w schowku")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Zachowaj",		tb_copy_xpm,	NULL, "Zapisz do schowka")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Wklej",	tb_paste_xpm,	NULL, "Wklej ze schowka")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Styl",	NoIcon,			NULL, "Styl")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Czcionka",		NoIcon,			NULL, "Rodzaj czcionki")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Wielko¶æ czcionki", NoIcon,		NULL, "Rozmiar czcionki")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Podgrubiona",		tb_text_bold_xpm,		NULL, "Pogrubiona")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Kursywa",	tb_text_italic_xpm,	NULL, "Pochylona")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Podkre¶lona",tb_text_underline_xpm,	NULL, "Podkre¶lona")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Przekre¶lona",   tb_text_strikeout_xpm,	NULL, "Przekre¶lona")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Indeks Górny",	tb_text_superscript_xpm,	NULL, "Indeks górny")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Indeks Dolny",	tb_text_subscript_xpm,		NULL, "Indeks dolny")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"W lewo",		tb_text_align_left_xpm,		NULL, "Wyrównanie w lewo")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Wycentruj",	tb_text_center_xpm,	NULL, "Wyrównanie na ¶rodek")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"W prawo",	tb_text_align_right_xpm,	NULL, "Wyrównanie w prawo")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Justowanie",	tb_text_justify_xpm,	NULL, "Wyrównanie do dwóch brzegów na raz")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Nic przed",		tb_para_0before_xpm,	NULL, "Bez odstêpu przed")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pkt przed",		tb_para_12before_xpm,	NULL, "wstaw 12 punktów przed")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Pojedyñczy odstêp",	tb_line_single_space_xpm,	NULL, "Pojedyñczy odstêp miêdzy liniami")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 odstêp",		tb_line_middle_space_xpm,	NULL, "Pó³torej odstêpu miêdzy liniami")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Podwójny odstêp",	tb_line_double_space_xpm,	NULL, "Podwójny odstêp miêdzy liniami")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 Kolumna",			tb_1column_xpm,			NULL, "1 Kolumna")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 Kolumna",		tb_2column_xpm,			NULL, "2 Kolumna")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 Kolumna",		tb_3column_xpm,			NULL, "3 Kolumna")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Powiêkszenie",		NoIcon,			NULL, "Powiêkszenie")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
