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

/* Galician translation by Jesus Bravo Alvarez <jba@pobox.com>
 * Last Revision: May 16, 2000
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

BeginSetEnc(gl,ES,true,"iso-8859-1")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Novo", 	tb_new_xpm,	NULL, "Crear un novo documento")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Abrir",	tb_open_xpm,	NULL, "Abrir un documento existente")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Gardar", 	tb_save_xpm,	NULL, "Garda-lo documento")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Gardar Como", 	tb_save_as_xpm,	NULL, "Garda-lo documento cun nome diferente")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Imprimir",	tb_print_xpm,	NULL, "Imprimi-lo documento")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Desfacer",		tb_undo_xpm,	NULL, "Desface-la edición")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Refacer",		tb_redo_xpm,	NULL, "Reface-la edición")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Cortar",		tb_cut_xpm,		NULL, "Cortar")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Copiar",		tb_copy_xpm,	NULL, "Copiar")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Pegar",	tb_paste_xpm,	NULL, "Pegar")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Estilo",	NoIcon,			NULL, "Estilo")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Fonte",		NoIcon,			NULL, "Fonte")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Tamaño da Fonte", NoIcon,		NULL, "Tamaño da Fonte")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Negriña",		tb_text_bold_xpm,		NULL, "Negriña")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Cursiva",	tb_text_italic_xpm,	NULL, "Cursiva")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Subliñado",tb_text_underline_xpm,	NULL, "Subliñado")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Superliñado",tb_text_overline_xpm,	NULL, "Superliñado")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Riscado",   tb_text_strikeout_xpm,	NULL, "Riscado")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Superíndice",	tb_text_superscript_xpm,	NULL, "Superíndice")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Subíndice",	tb_text_subscript_xpm,		NULL, "Subíndice")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Esquerda",		tb_text_align_left_xpm,		NULL, "Aliñamento á esquerda")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Centro",	tb_text_center_xpm,	NULL, "Aliñamento ó centro")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Dereita",	tb_text_align_right_xpm,	NULL, "Aliñamento á dereita")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Xustificar",	tb_text_justify_xpm,	NULL, "Xustifica-lo parágrafo")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Nada antes",		tb_para_0before_xpm,	NULL, "Espacio antes: ningún")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt antes",		tb_para_12before_xpm,	NULL, "Espacio antes: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Espaciado Simple",	tb_line_single_space_xpm,	NULL, "Espaciado simple")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"Espaciado 1.5",		tb_line_middle_space_xpm,	NULL, "Espaciado 1.5")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Espaciado dobre",	tb_line_double_space_xpm,	NULL, "Espaciado dobre")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 Columna",			tb_1column_xpm,			NULL, "1 Columna")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 Columnas",		tb_2column_xpm,			NULL, "2 Columnas")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 Columnas",		tb_3column_xpm,			NULL, "3 Columnas")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Escala",		NoIcon,			NULL, "Escala")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
