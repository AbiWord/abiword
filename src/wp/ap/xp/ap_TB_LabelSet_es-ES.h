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

BeginSet(es,ES,true)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Nuevo", 			tb_new_xpm,		NULL, "Crear un nuevo documento")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Abrir",			tb_open_xpm,	NULL, "Abrir un documento existente")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Guardar",			tb_save_xpm,	NULL, "Guardar el documento")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Guardar como",		tb_save_as_xpm,	NULL, "Guardar el documento con un nombre diferente")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Imprimir",			tb_print_xpm,	NULL, "Imprimir el documento")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW, "Presentación preliminar", tb_print_preview_xpm, NULL, "Presentación preliminar")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Deshacer",			tb_undo_xpm,	NULL, "Deshacer la edición")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Rehacer",			tb_redo_xpm,	NULL, "Rehacer la edición")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Cortar",			tb_cut_xpm,		NULL, "Cortar")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Copiar",			tb_copy_xpm,	NULL, "Copiar")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Pegar",			tb_paste_xpm,	NULL, "Pegar")

	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"Corrección ortográfica", tb_spellcheck_xpm, NULL, "Corrección ortográfica")
	ToolbarLabel(AP_TOOLBAR_ID_IMG, 			"Insertar imagen",	tb_insert_graphic_xpm, NULL, "Insertar una imagen en el documento")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Estilo",			NoIcon,			NULL, "Estilo")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Fuente",			NoIcon,			NULL, "Fuente")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Tamaño",			NoIcon,			NULL, "Tamaño de la fuente")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Negrita",			tb_text_bold_N_xpm,		NULL, "Negrita")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Italica",			tb_text_italic_K_xpm,	NULL, "Cursiva")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Subrayado",		tb_text_underline_S_xpm,NULL, "Subrayado")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Superrayado",		tb_text_overline_xpm,	NULL, "Superrayado")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Tachado",			tb_text_strikeout_xpm,	NULL, "Tachado")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Superíndice",		tb_text_superscript_xpm,NULL, "Superíndice")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Subíndice",		tb_text_subscript_xpm,	NULL, "Subíndice")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Símbolo",			tb_symbol_xpm,			NULL, "Insertar símbolo")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Izquierda",		tb_text_align_left_xpm,	NULL, "Alinear a la izquierda")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Centro",			tb_text_center_xpm,		NULL, "Alinear en el centro")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Derecha",			tb_text_align_right_xpm,NULL, "Alinear a la derecha")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Justificar",		tb_text_justify_xpm,	NULL, "Justificar párrafo")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Nada antes",		tb_para_0before_xpm,	NULL, "Espaciado anterior: ninguno")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt antes",		tb_para_12before_xpm,	NULL, "Espaciado anterior: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Espaciado simple",	tb_line_single_space_xpm,	NULL, "Espaciado simple")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"Espaciado 1,5",	tb_line_middle_space_xpm,	NULL, "Espaciado 1,5")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Espaciado doble",	tb_line_double_space_xpm,	NULL, "Espaciado doble")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 Columna",		tb_1column_xpm,			NULL, "1 Columna")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 Columnas",		tb_2column_xpm,			NULL, "2 Columnas")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 Columnas",		tb_3column_xpm,			NULL, "3 Columnas")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Zoom",				NoIcon,					NULL, "Zoom")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Viñetas",			tb_lists_bullets_xpm,	NULL, "Viñetas")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Listas",			tb_lists_numbers_xpm,	NULL, "Listas numeradas")
	
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Color de la tinta",	NoIcon,				NULL, "Cambiar el color de la tinta")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Color de fondo",	NoIcon,					NULL, "Cambiar el color del fondo")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,			"Indentar párrafo", tb_text_indent_xpm,		NULL, "Incrementar la indentación del párrafo")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"Desindentar párrafo", tb_text_unindent_xpm,	NULL, "Reducir la indentación del párrafo")

	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
