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


/* Welsh translation provided by Rhoslyn Prys<rhoslyn.prys@gwelywiwr.org> */

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

BeginSet(cy,GB,true)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,			NoIcon,					NULL,NULL)

	//          (id, 		                    szLabel,		IconName,     			szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Newydd", 			tb_new_xpm,				NULL, "Creu dogfen newydd")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Agor",			tb_open_xpm,			NULL, "Agor dogfen sy'n bod eisoes")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Cadw", 		tb_save_xpm,			NULL, "Cadw'r ddogfen")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Cadw Fel", 		tb_save_as_xpm,			NULL, "Cadw'r ddogfen o dan enw gwahanol")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Argraffu",		tb_print_xpm,			NULL, "Argraffu'r ddogfen")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,	"Rhagolwg Argraffu",	tb_print_preview_xpm, NULL, "Rhagolwg o'r ddogfen cyn argraffu")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Dad-wneud",			tb_undo_xpm,			NULL, "Dad-wneud y golygu")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Ail-wneud",			tb_redo_xpm,			NULL, "Ail-wneud y golygu")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Torri",			tb_cut_xpm,				NULL, "Torri")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Copio",			tb_copy_xpm,			NULL, "Copio")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Gludo",		tb_paste_xpm,			NULL, "Gludo")
	
ToolbarLabel(AP_TOOLBAR_ID_EDIT_HEADER,	 "Golygu'r Pennawd",		tb_edit_editheader_xpm,			NULL, "Golygu'r Pennawd")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_FOOTER,	 "Golygu'r Troedyn",		tb_edit_editfooter_xpm,			NULL, "Golygu'r Troedyn")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEHEADER, "Tynnu'r Pennawd",		tb_edit_removeheader_xpm,			NULL, "Tynnu'r Pennawd")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEFOOTER, "Tynnu'r Troedyn",		tb_edit_removefooter_xpm,			NULL, "Tynnu'r Troedyn")
	T
ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"Gwirio'r sillafu",	tb_spellcheck_xpm,		NULL, "Gwirio sillafu'r ddogfen")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,				"Mewnosod delwedd",	tb_insert_graphic_xpm,	NULL, "Mewnosod delwedd i'r ddogfen")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Arddull",		NoIcon,					NULL, "Arddull")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Ffont",			NoIcon,					NULL, "Ffont")
	
ToolbarLabel(AP_TOOLBAR_ID_FMT_HYPERLINK, "Insert Hyperlink", tb_hyperlink, NULL, "Insert a hyperlink into the document")
     ToolbarLabel(AP_TOOLBAR_ID_FMT_BOOKMARK, "Insert Bookmark", tb_anchor, NULL, "Insert a bookmark into the document")
	
ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Maint Ffont",	NoIcon,					NULL, "Maint Ffont")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Trwm",			tb_text_bold_xpm,		NULL, "Trwm")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Italig",		tb_text_italic_xpm,		NULL, "Italig")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Tanlinellu",	tb_text_underline_xpm,	NULL, "Tanlinellu")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Gosod linell uwchben",		tb_text_overline_xpm,	NULL, "Gosod llinell uwchben")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Llinell drwy",		tb_text_strikeout_xpm,	NULL, "Llinell allan")
	
ToolbarLabel(AP_TOOLBAR_ID_FMT_TOPLINE,		"Llinell uchaf",		tb_text_topline_xpm,	NULL, "Topline")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOTTOMLINE,		"Llinell isaf",		tb_text_bottomline_xpm,	NULL, "Bottomline")
	
ToolbarLabel(AP_TOOLBAR_ID_HELP,			"Cymorth",			tb_help_xpm,			NULL, "Cymorth")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Uwchysgrif",	tb_text_superscript_xpm,	NULL, "Uwchysgrif")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Isysgrif",
tb_text_subscript_xpm,		NULL, "Isysgrif")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Symbol",		tb_symbol_xpm,				NULL, "Mewnosod symbol")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Chwith",			tb_text_align_left_xpm,		NULL, "Alinio i'r chwith")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Canoli",		tb_text_center_xpm,			NULL, "Alinio i'r canol")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"De",		tb_text_align_right_xpm,	NULL, "Alinio i'r dde")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Unioni",		tb_text_justify_xpm,		NULL, "Unioni'r paragraf")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Dim cyn",		tb_para_0before_xpm,	NULL, "Bwlch cyn: Dim")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt cyn",		tb_para_12before_xpm,	NULL, "Bwlch cyn: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Bylchu sengl",	tb_line_single_space_xpm,	NULL, "Bylchu sengl")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"Bylchu 1.5",		tb_line_middle_space_xpm,	NULL, "Bylchu 1.5")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Bylchu dwbl",	tb_line_double_space_xpm,	NULL, "Bylchu dwbl")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 Golofn",			tb_1column_xpm,			NULL, "1 Golofn")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 Golofn",		tb_2column_xpm,			NULL, "2 Golofn")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 Colofn",		tb_3column_xpm,			NULL, "3 Colofn")

	ToolbarLabel(AP_TOOLBAR_ID_VIEW_SHOWPARA,	"Dangos Popeth",			tb_view_showpara_xpm,		NULL, "Dangos/cuddio ¶")
	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Chwyddo",				NoIcon,					NULL, "Chwyddo")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Bwledi",			tb_lists_bullets_xpm,	NULL, "Bwledi")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Rhifo",		tb_lists_numbers_xpm,	NULL, "Rhifo")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Lliw ffont",		tb_text_fgcolor_xpm,	NULL, "Lliw ffont")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Amlygu",		tb_text_bgcolor_xpm,	NULL, "Amlygu")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,			"Cynyddu'r mewnoliad",	tb_text_indent_xpm, 	NULL, "Cynyddu'r mewnoliad")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"Lleihau'r mewnoliad",	tb_text_unindent_xpm,	NULL, "Lleihau'r mewnoliad")

 
ToolbarLabel(AP_TOOLBAR_ID_SCRIPT_PLAY,		"Gw. sgript",	tb_script_play_xpm,		NULL, "Gweithredu'r sgript")

     ToolbarLabel(AP_TOOLBAR_ID_FMTPAINTER, "Fformatio'r  Peintiwr", tb_stock_paint_xpm, NULL, "Gosodwch fformat y paragraff fformatiwyd ynghynt i'r darn yma o destun")

#ifdef BIDI_ENABLED
	
ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR,	"Gorfodi cyfeiriad chwith/de i'r testun",		tb_text_direction_ltr_xpm,	NULL, "Gorfodi cyfeiriad chwith/de i'r testun")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL,	"Gorfodi cyfeiriad de/chwith i'r testun",		tb_text_direction_rtl_xpm,	NULL, "Gorfodi cyfeiriad de/chwith i'r testun")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,		"Cyfeiriad y paragraff",	tb_text_dom_direction_rtl_xpm,	NULL, "Newid prif gyfeiriad y paragraff")
#endif
	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,			NoIcon,			NULL,NULL)

EndSet()

