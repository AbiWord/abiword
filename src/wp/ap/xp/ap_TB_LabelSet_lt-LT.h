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

// Lithuanian Translation provided by Gediminas Paulauskas <menesis@delfi.lt>

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
// Note: the icon name here is to a specific icon (pixmap or bitmap or whatever)AbiWord
// Note: that will always be in the application.  if, for example, a big fat 'B'
// Note: for BOLD doesn't make sense in another language, change the entry in
// Note: the localization and add the icon to whatever table.

// Note: if a tool item does not use an icon (like a combo box), use the
// Note: constant "NoIcon" in that column.

// If the third argument is true, then this is the fall-back for
// this language (named in the first argument).

BeginSetEnc(lt,LT,true,"iso-8859-13")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,		NULL)

	//          (id, 		                    szLabel,	IconName,	    szToolTip,	szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Naujas",	tb_new_xpm,	NULL,		"")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Atidaryti",	tb_open_xpm,	NULL,		"")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Saugoti", 	tb_save_xpm,	NULL,		"")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Saugoti kaip",	tb_save_as_xpm,	NULL,		"")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Spausdinti",	tb_print_xpm,	NULL,		"")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,  "Sp. perþiûra", tb_print_preview_xpm, NULL, 	"Perþiûrëti kaip atrodys iðspausdinus")	

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Atðaukti",	tb_undo_xpm,	NULL,		"")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Kartoti",	tb_redo_xpm,	NULL,		"")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Iðkirpti",	tb_cut_xpm,	NULL,		"")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Kopijuoti",	tb_copy_xpm,	NULL,		"")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Ádëti",	tb_paste_xpm,	NULL,		"")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Stilius",	NoIcon,		NULL,		"")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Ðriftas",	NoIcon,		NULL,		"")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Ðrifto dydis", NoIcon,		NULL,		"")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Storas",	tb_text_bold_xpm,	NULL,	"")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Kursyvas",	tb_text_italic_xpm,	NULL,	"")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Pabrauktas",	tb_text_underline_xpm,	NULL,	"")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Brûkðnys virð",tb_text_overline_xpm,	NULL,	"")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Perbrauktas",	tb_text_strikeout_xpm,	NULL,	"")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Pakeltas",	tb_text_superscript_xpm,	NULL, "")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Nuleistas",	tb_text_subscript_xpm,		NULL, "")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Kairën",	tb_text_align_left_xpm, NULL, "Lygiuoti pagal kairæ")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Centre",tb_text_center_xpm,		NULL, "Lygiuoti pagal centrà")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Deðinën",	tb_text_align_right_xpm,NULL, "Lygiuoti pagal deðinæ")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Iðlyginti",	tb_text_justify_xpm,	NULL, "Iðlyginti abu kraðtus")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Jokio",	tb_para_0before_xpm,	NULL,	"Tarpas prieð: jokio")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt",	tb_para_12before_xpm,	NULL,	"Tarpas prieð: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Viengubi",	tb_line_single_space_xpm,	NULL,	"Viengubi tarpai tarp eiluèiø")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1,5 eilutës",	tb_line_middle_space_xpm,	NULL, "1,5 eilutës tarpai")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Dvigubi",	tb_line_double_space_xpm,	NULL, "Dvigubi tarpai")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,		"1 stulpelis",	tb_1column_xpm,		NULL,	"1 stulpelis")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,		"2 stulpeliai",	tb_2column_xpm,		NULL,	"2 stulpeliai")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,		"3 stulpeliai",	tb_3column_xpm,		NULL,	"3 stulpeliai")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"Mastelis",	NoIcon,			NULL,	"Mastelis")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Punktø sàr.",	tb_lists_bullets_xpm,	NULL,	"Pradëti/Uþbaigti punktø sàraðus")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Numeruotas",	tb_lists_numbers_xpm,	NULL,	"Pradëti/Uþbaigti numeruotus sàraðus")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Fonas",	tb_text_fgcolor_xpm,			NULL,	"Fono spalva")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Spalva",	tb_text_bgcolor_xpm,			NULL,	"Teksto spalva")	
	
	// New
	
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Simbolis",	tb_symbol_xpm,		NULL,	"Áterpti simbolá")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,		"Átraukti",	tb_text_indent_xpm,	NULL,	"Átraukti nuo dokumento kraðto")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"Pritraukti",	tb_text_unindent_xpm,	NULL,	"Pritraukti arèiau dokumento kraðto")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"Raðyba",	tb_spellcheck_xpm,	NULL,	"Patikrinti dokumento raðybà")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,			"Paveikslëlis", tb_insert_graphic_xpm,	NULL,	"Áterpti paveikslëlá á dokumentà")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
