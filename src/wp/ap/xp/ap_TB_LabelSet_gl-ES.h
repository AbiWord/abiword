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
 * Last Revision by Ramón Flores <fa2ramon@usc.es>
 * Last Revision: Jan 3, 2002
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

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Novo", 	tb_new_xpm,	NULL, "Criar un	documento novo")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Abrir",	tb_open_xpm,	NULL, "Abrir un documento existente")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Gardar", 	tb_save_xpm,	NULL, "Gardar o documento")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Gardar Como", 	tb_save_as_xpm,	NULL, "Gardar o documento con un nome diferente")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Imprimir",	tb_print_xpm,	NULL, "Imprimir o documento")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,	"Pre-visualización",	tb_print_preview_xpm,	NULL,	"Pré-visualización do documento antes de o imprimir")
	
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Desfacer",		tb_undo_xpm,	NULL, "Desfai edición")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Refacer",		tb_redo_xpm,	NULL, "Refai edición")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Cortar",		tb_cut_xpm,		NULL, "Cortar")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Copiar",		tb_copy_xpm,	NULL, "Copiar")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Colar",	tb_paste_xpm,	NULL, "Colar")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_HEADER,		"Editar Cabezallo", tb_edit_editheader_xpm, NULL, "Editar Cabezallo")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_FOOTER,		"Editar Rodapé", tb_edit_editfooter_xpm, NULL,	"Editar Rodapé")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEHEADER,	"Eliminar Cabezallo", tb_edit_removeheader_xpm, NULL, "Eliminar Cabezallo")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEFOOTER,	"Eliminar Rodapé", tb_edit_removefooter_xpm, NULL, "Eliminar Rodapé")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"Ortografia",	tb_spellcheck_xpm,	NULL,	"Verifición ortográfica do documento")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,			"Inserir Imaxe", tb_insert_graphic_xpm,NULL,	"Inserir imaxe")
	
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Estilo",	NoIcon,			NULL, "Estilo")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Fonte",		NoIcon,			NULL, "Tipo de letra")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_HYPERLINK,	"Inserir Ligazón", tb_hyperlink,	NULL,	"Inserir unha ligazón no documento")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOOKMARK,	"Inserir Áncora", tb_anchor,		NULL,	"Inserir unha áncora no documento")	
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Tamaño", NoIcon,		NULL, "Tamaño de letra")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Negrito",		tb_text_bold_xpm,		NULL, "Negrito")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Itálico",	tb_text_italic_xpm,	NULL, "Itálico")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Subliñado",tb_text_underline_xpm,	NULL, "Subliñado")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Sobreliñado",tb_text_overline_xpm,	NULL, "Sobreliñado")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Riscado",   tb_text_strikeout_xpm,	NULL, "Riscado")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_TOPLINE,		"Liña Superior",tb_text_topline_pt_PT_xpm,	NULL,	"Liña Superior")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOTTOMLINE,	"Liña Inferior",tb_text_bottomline_pt_PT_xpm,NULL,	"Liña Inferior")
	ToolbarLabel(AP_TOOLBAR_ID_HELP,		"Axuda",	tb_help_xpm,		NULL,	"Axuda")
	
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Superíndice",	tb_text_superscript_xpm,	NULL, "Superíndice")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Subíndice",	tb_text_subscript_xpm,		NULL, "Subíndice")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Símbolo",	tb_symbol_xpm,		NULL,	"Inserir símbolo")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Á Esquerda",		tb_text_align_left_xpm,		NULL, "Aliñar á esquerda")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Ao Centro",	tb_text_center_xpm,	NULL, "Aliñar ao centro")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Á Direita",	tb_text_align_right_xpm,	NULL, "Aliñar á direita")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Xustificar",	tb_text_justify_xpm,	NULL, "Xustificar o parágrafo")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Nada antes",		tb_para_0before_xpm,	NULL, "Espazamento anterior: nengun")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt antes",		tb_para_12before_xpm,	NULL, "Espazamento anterior: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Espazamento Simples",	tb_line_single_space_xpm,	NULL, "Espazamento simples")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"Espazamento 1.5",		tb_line_middle_space_xpm,	NULL, "Espazamento 1.5")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Espazamento Duplo",	tb_line_double_space_xpm,	NULL, "Espazamento duplo")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 Coluna",			tb_1column_xpm,			NULL, "1 Coluna")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 Colunas",		tb_2column_xpm,			NULL, "2 Colunas")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 Colunas",		tb_3column_xpm,			NULL, "3 Colunas")

	ToolbarLabel(AP_TOOLBAR_ID_VIEW_SHOWPARA,	"Ver Parágrafos", tb_view_showpara_xpm,	NULL,	"Mostrar/Ocultar ¶")
	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"Zoom",		NoIcon,			NULL,	"Zoom")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Lista con Marcas",tb_lists_bullets_xpm,	NULL,	"Listas con marcas")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Lista Numerada",tb_lists_numbers_xpm,	NULL,	"Listas numeradas")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Cor texto",		tb_text_fgcolor_xpm,			NULL,	"Cor do texto")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Cor fundo",	tb_text_bgcolor_xpm,			NULL,	"Cor de fundo")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,		"Indentar",	tb_text_indent_xpm,	NULL,	"Aumentar indentazón")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"Desindentar",	tb_text_unindent_xpm,	NULL,	"Diminuir indentazón")

	ToolbarLabel(AP_TOOLBAR_ID_SCRIPT_PLAY,		"Executar Script",tb_script_play_xpm,	NULL,	"Executar script")

#ifdef BIDI_ENABLED
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR,"Esquerda para Direita", tb_text_direction_ltr_xpm,	NULL, "Forzar orientación da Esquerda para a Direita")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL,"Direita para Esquerda", tb_text_direction_rtl_xpm,	NULL, "Forzar orientación da Direita para a Esquerda")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,	"Orientación do parágrafo",    tb_text_dom_direction_rtl_xpm,	NULL, "Forzar orientación normal do parágrafo")
#endif

	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
