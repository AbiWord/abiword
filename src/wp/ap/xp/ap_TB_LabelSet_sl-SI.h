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


// Slovenian translation provided by Andraz Tor <andraz.tori1@guest.arnes.si>

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

BeginSetEnc(sl,SI,true,"iso-8859-2")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,			NoIcon,					NULL,NULL)

	//          (id, 		                    szLabel,		IconName,     			szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Nova", 			tb_new_xpm,				NULL, "Ustvari nov dokument")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Odpri",			tb_open_xpm,			NULL, "Odpri obstojeè dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Shrani", 		tb_save_xpm,			NULL, "Shrani dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Shrani kot", 		tb_save_as_xpm,			NULL, "Shrani dokument pod drugaènim imenom")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Tiskaj",		tb_print_xpm,			NULL, "Natisni dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,	"Predogled tiskanja",	tb_print_preview_xpm, NULL, "Predogled dokumenta pred tiskanjem")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Razveljavi",			tb_undo_xpm,			NULL, "Razveljavi urejanje")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Obnovi",			tb_redo_xpm,			NULL, "Obnovi urejanje")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Izre¾i",			tb_cut_xpm,				NULL, "Izre¾i izbiro")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Kopiraj",			tb_copy_xpm,			NULL, "Kopiraj izbiro")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Prilepi",		tb_paste_xpm,			NULL, "Prilepi z odlo¾i¹èa")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"Preveri èrkovanje",	tb_spellcheck_xpm,		NULL, "Preveri èrkovanje dokumenta")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,			"Vstavi sliko",	tb_insert_graphic_xpm,	NULL, "V dokument vstavi sliko")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Slog",		NoIcon,					NULL, "Slog")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Pisava",			NoIcon,					NULL, "Pisava")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Velikost pisave",	NoIcon,					NULL, "Velikost pisave")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Krepko",			tb_text_bold_xpm,		NULL, "Krepko")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Kurzivno",		tb_text_italic_xpm,		NULL, "Kurzivno")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Podèrtano",	tb_text_underline_xpm,	NULL, "Podèrtano")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Nadèrtano",		tb_text_overline_xpm,	NULL, "Nadèrtano")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Preèrtano",		tb_text_strikeout_xpm,	NULL, "Preèrtano")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_TOPLINE,		"Èrta zgoraj",		tb_text_topline_xpm,	NULL, "Èrta zgoraj")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOTTOMLINE,	"Èrta spodaj",		tb_text_bottomline_xpm,	NULL, "Èrta spodaj")
	ToolbarLabel(AP_TOOLBAR_ID_HELP,		"Pomoè",			tb_help_xpm,			NULL, "Pomoè")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Nadpisano",	tb_text_superscript_xpm,	NULL, "Nadpisano")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Podpisano",	tb_text_subscript_xpm,		NULL, "Podpisano")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Simbol",		tb_symbol_xpm,				NULL, "Vstavi simbol")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Leva",			tb_text_align_left_xpm,		NULL, "Leva poravnava")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Sredinska",		tb_text_center_xpm,			NULL, "Sredinska poravnava")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Desna",		tb_text_align_right_xpm,	NULL, "Desna poravnava")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Obojestranska",		tb_text_justify_xpm,		NULL, "Obojestranska poravnava")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Brez presledka pred",		tb_para_0before_xpm,	NULL, "Brez presledka pred odstavkom")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pik presledka pred",		tb_para_12before_xpm,	NULL, "12 pik presledka pred odstavkom")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Enojni medvrstièni prostor",	tb_line_single_space_xpm,	NULL, "Enojni medvrstièni prostor")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1,5 medvrstiènega prostora",		tb_line_middle_space_xpm,	NULL, "1,5 medvrstièni prostor")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Dvojni medvrstièni prostor",	tb_line_double_space_xpm,	NULL, "Dvojni medvrstièni prostor")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 stolpec",			tb_1column_xpm,			NULL, "1 stolpec")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 stolpca",		tb_2column_xpm,			NULL, "2 stolpca")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 stolpci",		tb_3column_xpm,			NULL, "3 stolpci")

	ToolbarLabel(AP_TOOLBAR_ID_VIEW_SHOWPARA,	"Ka¾i vse",			tb_view_showpara_xpm,		NULL, "Ka¾i/skrij")
	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"Poveèava",				NoIcon,					NULL, "Poveèaj")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Vrstiène oznake",			tb_lists_bullets_xpm,	NULL, "Vrstiène oznake")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"O¹tevilèenje",		tb_lists_numbers_xpm,	NULL, "O¹tevilèenjestrani")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Barva pisave",		tb_text_fgcolor_xpm,	NULL, "Barva pisave")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Osvetlitev",		tb_text_bgcolor_xpm,	NULL, "Osvetlitev")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,		"Poveèaj zamik",	tb_text_indent_xpm, 	NULL, "Poveèaj zamik")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"Zmanj¹aj zamik",	tb_text_unindent_xpm,	NULL, "Zmanj¹aj zamik")

#ifdef ABI_OPT_JS
	ToolbarLabel(AP_TOOLBAR_ID_SCRIPT_PLAY,		"Izvedi skripto",	tb_script_play_xpm,		NULL, "Izvedi skripto")
#endif

     // ... add others here ...
#ifdef BIDI_ENABLED
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR,	"Besedilo od leve proti desni",		tb_text_direction_ltr_xpm,	NULL, "Besedilo mora biti od leve proti desni")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL,	"Besedilo od desne proti levi",		tb_text_direction_rtl_xpm,	NULL, "Besedilo mora biti od desne proti levi")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,		"Usmerjenost odstavka",	tb_text_dom_direction_rtl_xpm,	NULL, "Spremeni prevladujoèo usmerjenost odstavka")
#endif
	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,			NoIcon,			NULL,NULL)

EndSet()
