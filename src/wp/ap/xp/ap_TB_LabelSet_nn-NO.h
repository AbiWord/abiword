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

// Norwegian Nynorsk translations provided by Karl Ove Hufthammer <huftis@bigfoot.com>

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

BeginSetEnc(nn,NO,true,"iso-8859-1")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,			NoIcon,					NULL,NULL)

	//          (id, 		                    szLabel,		IconName,		szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Ny", 			tb_new_xpm,		NULL, "Opprett eit nytt dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Opna",			tb_open_xpm,		NULL, "Opna eit eksisterande dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Lagra", 		tb_save_xpm,		NULL, "Lagra dokumentet")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Lagra som", 		tb_save_as_xpm,		NULL, "Lagra dokumentet med eit anna namn")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Skriv ut",		tb_print_xpm,		NULL, "Skriv ut heile eller delar av dokumentet")
        ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,  "F�rehandsvising",	tb_print_preview_xpm,	NULL, "Vis dokumentet slik det blir skrive ut")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Angra",			tb_undo_xpm,		NULL, "Angra siste handling")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Gjer om",		tb_redo_xpm,		NULL, "Gjer om siste angra handling")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Klipp ut",		tb_cut_xpm,		NULL, "Klipp ut")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Kopier",		tb_copy_xpm,		NULL, "Kopier")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Lim inn",		tb_paste_xpm,		NULL, "Lim inn")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_HEADER,	 "Rediger topptekst",		tb_edit_editheader_xpm,			NULL, "Rediger topptekst")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_FOOTER,	 "Rediger botntekst",		tb_edit_editfooter_xpm,			NULL, "Rediger botntekst")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEHEADER, "Fjern topptekst",		tb_edit_removeheader_xpm,			NULL, "Fjern topptekst")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEFOOTER, "Fjern botntekst",		tb_edit_removefooter_xpm,			NULL, "Fjern botntekst")
        ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"Stavekontroll",	tb_spellcheck_xpm, 	NULL, "Stavekontrollerer dokumentet")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,			"Sett inn bilete",	tb_insert_graphic_xpm, 	NULL, "Sett inn eit bilete i dokumentet")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Stil",			NoIcon,			NULL, "Stil")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Skrift",		NoIcon,			NULL, "Skrift")
     ToolbarLabel(AP_TOOLBAR_ID_FMT_HYPERLINK, "Sett inn lenkje", tb_hyperlink, NULL, "Sett inn ei lenkje i dokumentet")
     ToolbarLabel(AP_TOOLBAR_ID_FMT_BOOKMARK, "Sett inn bokmerke", tb_anchor, NULL, "Sett inn eit bokmerke i dokumentet")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Skriftstorleik",	NoIcon,			NULL, "Skriftstorleik")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Halvfeit",		tb_text_bold_xpm,	NULL, "Halvfeit")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Kursiv",		tb_text_italic_xpm,	NULL, "Kursiv")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Understreking",	tb_text_underline_xpm,	NULL, "Understreking")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Overstreking",		tb_text_overline_xpm,	NULL, "Overstreking")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Gjennomstreking",  	tb_text_strikeout_xpm,	NULL, "Gjennomstreking")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_TOPLINE,		"Topplinje",		tb_text_topline_xpm,	NULL, "Topplinje")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOTTOMLINE,		"Botnlinje",		tb_text_bottomline_xpm,	NULL, "Botnlinje")
        ToolbarLabel(AP_TOOLBAR_ID_HELP,		"Hjelp",		tb_help_xpm, 		NULL, "Hjelp")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Heva skrift",	tb_text_superscript_xpm,	NULL, "Heva skrift")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Senka skrift",	tb_text_subscript_xpm,		NULL, "Senka skrift")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Symbol",	tb_symbol_xpm,		NULL, "Sett inn symbol")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Venstrejuster",		tb_text_align_left_xpm,		NULL, "Venstrejuster")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Midtstill",	tb_text_center_xpm,	NULL, "Midtstill")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"H�grejuster",	tb_text_align_right_xpm,	NULL, "H�grejuster")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Blokkjuster",	tb_text_justify_xpm,	NULL, "Blokkjuster")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Ingen mellomrom f�r",		tb_para_0before_xpm,	NULL, "Mellomrom f�r: Ingen")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12-punkts mellomrom f�r",		tb_para_12before_xpm,	NULL, "Mellomrom f�r: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Enkel linjeavstand",	tb_line_single_space_xpm,	NULL, "Enkel linjeavstand")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"Halvannan linjeavstand",		tb_line_middle_space_xpm,	NULL, "Halvannan linjeavstand")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Dobbel linjeavstand",	tb_line_double_space_xpm,	NULL, "Dobbel linjeavstand")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"Ei spalte",			tb_1column_xpm,			NULL, "Ei spalte")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"To spalter",		tb_2column_xpm,			NULL, "To spalter")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"Tre spalter",		tb_3column_xpm,			NULL, "Tre spalter")

	ToolbarLabel(AP_TOOLBAR_ID_VIEW_SHOWPARA,	"Vis alt",			tb_view_showpara_xpm,		NULL, "Vis/skjul �")
	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Forst�rring",		NoIcon,			NULL, "Forst�rring")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Punktmerkt liste",		tb_lists_bullets_xpm,		NULL,		"Punktmerkt liste")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Nummerert liste",		tb_lists_numbers_xpm,		NULL,		"Nummerert liste")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Tekstfarge",		tb_text_fgcolor_xpm,					NULL, "Tekstfarge")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Uthevingsfarge",		tb_text_bgcolor_xpm,					NULL, "Uthevingsfarge")
        ToolbarLabel(AP_TOOLBAR_ID_INDENT,			"Auk innrykk", tb_text_indent_xpm, NULL, "Aukar avsnittsinnrykk")
        ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"Reduser innrykk", tb_text_unindent_xpm, NULL, "Reduserer avsnittsinnrykk")

	ToolbarLabel(AP_TOOLBAR_ID_SCRIPT_PLAY,		"Kj�r skript",	tb_script_play_xpm,		NULL, "Kj�r skript")

     ToolbarLabel(AP_TOOLBAR_ID_FMTPAINTER, "Bruk format", tb_stock_paint_xpm, NULL, "Brukar tidlegare kopiert formatering p� merkt tekst")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR,	"Tving VTH-retning",		tb_text_direction_ltr_xpm,	NULL, "Tvingar venstre til h�gre-retning for tekst")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL,	"Tving HTV-retning",		tb_text_direction_rtl_xpm,	NULL, "Tvingar h�gre til venstre-retning for tekst")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,		"Avsnittsretning",	tb_text_dom_direction_rtl_xpm,	NULL, "Endrar hovudtekstretning for avsnitt")

     // ... add others here ...

EndSet()
