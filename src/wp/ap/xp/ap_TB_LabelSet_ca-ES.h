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
 *
 * Translation by SoftCatalà (jesus@softcatala.org)
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

BeginSet(ca,ES,true)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Nou", 		tb_new_xpm,		NULL, "Crea un nou document")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Obre",		tb_open_xpm,	NULL, "Obre un dicument existent")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Desa", 	tb_save_xpm,	NULL, "Desa el document")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Anomena i desa", 	tb_save_as_xpm,	NULL, "Desa el document amb un altre nom")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Imprimeix",	tb_print_xpm,	NULL, "Imprimeix el document")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,  "Previsualitza", tb_print_preview_xpm, NULL, "Previsualitza el document abans d'imprimir-lo")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Desfés",		tb_undo_xpm,	NULL, "Desfés l'edició")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Refés",		tb_redo_xpm,	NULL, "Refés l'edició")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Retalla",		tb_cut_xpm,		NULL, "Retalla")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Copia",		tb_copy_xpm,	NULL, "Copia")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Enganxa",	tb_paste_xpm,	NULL, "Enganxa")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK, 		"Ortografia", tb_spellcheck_xpm, NULL, "Comprova l'ortografia")
	ToolbarLabel(AP_TOOLBAR_ID_IMG, 		"Insereix una imatge", tb_insert_graphic_xpm, NULL, "Insereix una imatge en el document")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Estil",	NoIcon,			NULL, "Estil")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Tipus de lletra",		NoIcon,			NULL, "Tipus de lletra")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Mida de la lletra", NoIcon,		NULL, "Mida de la lletra")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Negreta",		tb_text_bold_N_xpm,		NULL, "Negreta")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Cursiva",	tb_text_italic_K_xpm,	NULL, "Cursiva")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Subratllat",tb_text_underline_S_xpm,	NULL, "Subratllat")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Supraratllat",tb_text_overline_xpm,	NULL, "Supraratllat")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Barrat",   tb_text_strikeout_xpm,	NULL, "Barrat")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Superíndex",	tb_text_superscript_xpm,	NULL, "Superíndex")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Subíndex",	tb_text_subscript_xpm,		NULL, "Subíndex")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Símbol",			tb_symbol_xpm,			NULL, "Inserta un símbol")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Esquerra",		tb_text_align_left_xpm,		NULL, "Alineació esquerra")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Centre",	tb_text_center_xpm,	NULL, "Alineació central")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Dreta",	tb_text_align_right_xpm,	NULL, "Alineació dreta")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Justificat",	tb_text_justify_xpm,	NULL, "Justifica el paràgraf")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Cap abans",		tb_para_0before_xpm,	NULL, "Espai previ: cap")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt abans",		tb_para_12before_xpm,	NULL, "Espai previ: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Espaiat senzill",	tb_line_single_space_xpm,	NULL, "Espaiat senzill")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"Espaiat 1,5",		tb_line_middle_space_xpm,	NULL, "Espaiat 1,5")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Espaiat doble",	tb_line_double_space_xpm,	NULL, "Espaiat doble")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,		"1 columna",			tb_1column_xpm,			NULL, "1 columna")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,		"2 columnes",		tb_2column_xpm,			NULL, "2 columnes")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,		"3 columnes",		tb_3column_xpm,			NULL, "3 columnes")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"Zoom",			NoIcon,			NULL, "Zoom")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Vinyetes",			tb_lists_bullets_xpm,	NULL, "Vinyetes")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Llistes",			tb_lists_numbers_xpm,	NULL, "Llistes numerades")
	
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Color",			tb_text_fgcolor_xpm,			NULL,	"Canvia el color")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Color de fons",	tb_text_bgcolor_xpm,			NULL,	"Canvia el color de fons")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,		"Identació",	tb_text_indent_xpm,	NULL,	"Augmenta la identació del paràgraf")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"Desidentació",	tb_text_unindent_xpm,	NULL,	"Disminueix la identació del paràgraf")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
