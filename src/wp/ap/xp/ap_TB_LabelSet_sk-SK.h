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

BeginSetEnc(sk,SK,true,"iso-8859-2")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Nový",		tb_new_xpm,	NULL, "Vytvori5 nový dokument")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Otvori»",	tb_open_xpm,	NULL, "Otvori» ulo¾ený dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Ulo¾i»", 	tb_save_xpm,	NULL, "Ulo¾i» dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Ulo¾i» ako", 	tb_save_as_xpm,	NULL, "Ulo¾i» dokument pod iným menom")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Tlaè",		tb_print_xpm,	NULL, "Vytlaèi» dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,	"Uká¾ka",	tb_print_preview_xpm, NULL, "Náhµad dokumentu")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Spä»",		tb_undo_xpm,	NULL, "Vráti» úrpavy")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Opakova»",	tb_redo_xpm,	NULL, "Opakova» úpravy")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Vystrihnú»",	tb_cut_xpm,	NULL, "Vystrihnú» text/objekt")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Kopírova»",	tb_copy_xpm,	NULL, "Kopírova»")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Vlo¾i»",	tb_paste_xpm,	NULL, "Vlo¾i»")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"Pravopis",	tb_spellcheck_xpm,	NULL, "Nájs» preklepy v dokumente")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,			"Vlo¾i» obrázok", tb_insert_graphic_xpm,NULL, "Vlo¾i» obrázok")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"©týl",		NoIcon,			NULL, "©týl")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Písmo",	NoIcon,			NULL, "Písmo")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Veµkos» písma", NoIcon,		NULL, "Veµkos» písma")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Tuèné",	tb_text_bold_xpm,	NULL, "Tuèné")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"©ikmé",	tb_text_italic_xpm,	NULL, "©ikmé")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Podèiarknuté",	tb_text_underline_xpm,	NULL, "Podèiarknuté")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Pod èiarou",	tb_text_overline_xpm,	NULL, "Pod èiarou")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Pre¹krtnutie",	tb_text_strikeout_xpm,	NULL, "Pre¹krtnutie")
	ToolbarLabel(AP_TOOLBAR_ID_HELP,		"Pomocník",	tb_help_xpm,		NULL, "Pomocník")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Horný index",	tb_text_superscript_xpm,NULL, "Horný index")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Dolný index",	tb_text_subscript_xpm,	NULL, "Dolný index")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,       "Symbol",       tb_symbol_xpm,          NULL, "Symbol")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Vµavo",	tb_text_align_left_xpm,	NULL, "Zarovna» vµavo")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Stred",	tb_text_center_xpm,	NULL, "Zarovna» do stredu")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Vpravo",	tb_text_align_right_xpm,	NULL, "Zarovna» vpravo")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Do bloku",	tb_text_justify_xpm,	NULL, "Zarovna» do bloku")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Niè posledné",		tb_para_0before_xpm,	NULL, "Niè posledné")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 bodov posledných",	tb_para_12before_xpm,	NULL, "12 bodov posledných")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Obyèajné riadkovanie",	tb_line_single_space_xpm,	NULL, "Obyèajné riadkovanie")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 riadkovanie",	tb_line_middle_space_xpm,	NULL, "1.5 riadkovanie")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Dvojité riadkovanie",	tb_line_double_space_xpm,	NULL, "Dvojité riadkovanie")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,		"1 ståpec",		tb_1column_xpm,			NULL, "1 ståpec")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,		"2 ståpce",		tb_2column_xpm,			NULL, "2 ståpce")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,		"3 ståpce",		tb_3column_xpm,			NULL, "3 ståpce")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"Zväè¹enie",		NoIcon,			NULL,	"Zväè¹enie")
        ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Odrá¾ky",		tb_lists_bullets_xpm,	NULL,	"Zaèa»/Ukonèi» odrá¾kové zoznamy")
        ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Èíslovaný zoznam",	tb_lists_numbers_xpm,	NULL,	"Zaèa»/Ukonèi» èíslovaný zoznam") 
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Farba písma",		tb_text_fgcolor_xpm,	NULL,	"Farba písma")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Zvýraznanie",		tb_text_bgcolor_xpm,	NULL,	"Zvýraznenie")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,		"Zväè¹i» odsadenie",	tb_text_indent_xpm,	NULL,	"Zväè¹i» odsadenie")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,            "Zmen¹i» odsadenie",	tb_text_unindent_xpm,	NULL,	"Zmen¹i» odsadenie")
	
	// ... add others here ...
#ifdef BIDI_ENABLED
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR, "Text z µava do prava", tb_text_direction_ltr_xpm, NULL, "")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL, "Text z prava do µava", tb_text_direction_rtl_xpm, NULL, "")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION, "Smer odstavca",  tb_text_dom_direction_rtl_xpm,  NULL, "")
#endif

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
