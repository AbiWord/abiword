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

BeginSet(FiFI)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Uusi", 		tb_new_xpm,		NULL, "Luo uusi asiakirja")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Avaa",		tb_open_xpm,	NULL, "Avaa asiakirja")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Tallenna", 	tb_save_xpm,	NULL, "Tallenna asiakirja")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Tallenna Nimella", 	tb_save_as_xpm,	NULL, "Tallenna käsiteltävä asiakirja eri nimelle")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Tulosta",	tb_print_xpm,	NULL, "Tulosta asiakirja")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Peru",		tb_undo_xpm,	NULL, "Peru muutokset")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Uudelleen",		tb_redo_xpm,	NULL, "Palauta peruttu muokkaus")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Leikkaa",		tb_cut_xpm,		NULL, "Leikkaa")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Kopioi",		tb_copy_xpm,	NULL, "Kopioi")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Liitä",	tb_paste_xpm,	NULL, "Liitä")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Tyyli",	NoIcon,			NULL, "Tyyli")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Fontti",		NoIcon,			NULL, "Fontti")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Fontin koko", NoIcon,		NULL, "Fontin koko")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Lihavoitu",		tb_text_bold_xpm,		NULL, "Lihavoitu")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Kursivoitu",	tb_text_italic_xpm,	NULL, "Kursivoitu")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Alleviivattu",tb_text_underline_xpm,	NULL, "Alleviivattu")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Yliviivattu",   tb_text_strikeout_xpm,	NULL, "Yliviivattu")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Vasen",		tb_text_align_left_xpm,		NULL, "Vasen tasaus")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Keskitetty",	tb_text_center_xpm,	NULL, "Keskitetty tasaus")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Oikea",	tb_text_align_right_xpm,	NULL, "Oikea tasaus")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Molemmat",	tb_text_justify_xpm,	NULL, "Molemmat reunat tasattu")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Ei yhtään ennen",		tb_para_0before_xpm,	NULL, "Riviväli ennen: ei väliä")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt ennen",		tb_para_12before_xpm,	NULL, "Riviväli ennen: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Ykkös riviväli",	tb_line_single_space_xpm,	NULL, "Ykkös riviväli")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 Spacing",		tb_line_middle_space_xpm,	NULL, "1.5 riviväli")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Kaksois riviväli",	tb_line_double_space_xpm,	NULL, "Kaksois riviväli")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 palsta",			tb_1column_xpm,			NULL, "1 palsta")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 palstaa",		tb_2column_xpm,			NULL, "2 palstaa")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 palstaa",		tb_3column_xpm,			NULL, "3 palstaa")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Zoomi",		NoIcon,			NULL, "Zoomi")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
