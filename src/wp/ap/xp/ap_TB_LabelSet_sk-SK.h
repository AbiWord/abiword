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

// If the third argument is true, then this is the fall-back for
// this language (named in the first argument).

BeginSetEnc(sk,SK,true,"UTF-8")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Nový",		tb_new_xpm,	NULL, "Vytvoriť nový dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Otvoriť",	tb_open_xpm,	NULL, "Otvoriť uložený dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Uložiť", 	tb_save_xpm,	NULL, "Uložiť dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Uložiť ako", 	tb_save_as_xpm,	NULL, "Uložiť dokument pod iným menom")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Tlač",		tb_print_xpm,	NULL, "Vytlačiť dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,	"Ukážka",	tb_print_preview_xpm, NULL, "Náhľad dokumentu")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Späť",		tb_undo_xpm,	NULL, "Vrátiť úrpavy")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Opakovať",	tb_redo_xpm,	NULL, "Opakovať úpravy")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Vystrihnúť",	tb_cut_xpm,	NULL, "Vystrihnúť text/objekt")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Kopírovať",	tb_copy_xpm,	NULL, "Kopírovať")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Vložiť",	tb_paste_xpm,	NULL, "Vložiť")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"Pravopis",	tb_spellcheck_xpm,	NULL, "Nájsť preklepy v dokumente")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,			"Vložiť obrázok", tb_insert_graphic_xpm,NULL, "Vložiť obrázok")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Štýl",		NoIcon,			NULL, "Štýl")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Písmo",	NoIcon,			NULL, "Písmo")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Veľkosť písma", NoIcon,		NULL, "Veľkosť písma")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Tučné",	tb_text_bold_xpm,	NULL, "Tučné")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Šikmé",	tb_text_italic_xpm,	NULL, "Šikmé")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Podčiarknuté",	tb_text_underline_xpm,	NULL, "Podčiarknuté")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Pod čiarou",	tb_text_overline_xpm,	NULL, "Pod čiarou")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Preškrtnutie",	tb_text_strikeout_xpm,	NULL, "Preškrtnutie")
	ToolbarLabel(AP_TOOLBAR_ID_HELP,		"Pomocník",	tb_help_xpm,		NULL, "Pomocník")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Horný index",	tb_text_superscript_xpm,NULL, "Horný index")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Dolný index",	tb_text_subscript_xpm,	NULL, "Dolný index")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,       "Symbol",       tb_symbol_xpm,          NULL, "Symbol")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Vľavo",	tb_text_align_left_xpm,	NULL, "Zarovnať vľavo")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Stred",	tb_text_center_xpm,	NULL, "Zarovnať do stredu")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Vpravo",	tb_text_align_right_xpm,	NULL, "Zarovnať vpravo")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,		"Do bloku",	tb_text_justify_xpm,	NULL, "Zarovnať do bloku")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Nič posledné",		tb_para_0before_xpm,	NULL, "Nič posledné")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 bodov posledných",	tb_para_12before_xpm,	NULL, "12 bodov posledných")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Obyčajné riadkovanie",	tb_line_single_space_xpm,	NULL, "Obyčajné riadkovanie")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 riadkovanie",	tb_line_middle_space_xpm,	NULL, "1.5 riadkovanie")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Dvojité riadkovanie",	tb_line_double_space_xpm,	NULL, "Dvojité riadkovanie")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,		"1 stĺpec",		tb_1column_xpm,			NULL, "1 stĺpec")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,		"2 stĺpce",		tb_2column_xpm,			NULL, "2 stĺpce")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,		"3 stĺpce",		tb_3column_xpm,			NULL, "3 stĺpce")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"Zväčšenie",		NoIcon,			NULL,	"Zväčšenie")
        ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Odrážky",		tb_lists_bullets_xpm,	NULL,	"Začať/Ukončiť odrážkové zoznamy")
        ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Číslovaný zoznam",	tb_lists_numbers_xpm,	NULL,	"Začať/Ukončiť číslovaný zoznam") 
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Farba písma",		tb_text_fgcolor_xpm,	NULL,	"Farba písma")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Zvýraznanie",		tb_text_bgcolor_xpm,	NULL,	"Zvýraznenie")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,		"Zväčšiť odsadenie",	tb_text_indent_xpm,	NULL,	"Zväčšiť odsadenie")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,            "Zmenšiť odsadenie",	tb_text_unindent_xpm,	NULL,	"Zmenšiť odsadenie")
	
	// ... add others here ...
#ifdef BIDI_ENABLED
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR, "Text z ľava do prava", tb_text_direction_ltr_xpm, NULL, "")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL, "Text z prava do ľava", tb_text_direction_rtl_xpm, NULL, "")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION, "Smer odstavca",  tb_text_dom_direction_rtl_xpm,  NULL, "")
#endif

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
