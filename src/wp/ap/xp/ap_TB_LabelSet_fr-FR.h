/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

/* traduction fcella@mahj.org du 24/05/1999  */

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

BeginSet(FrFR)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Nouveau", 	tb_new_xpm,	NULL, "Creer un nouveau document")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Ouvrir",	tb_open_xpm,	NULL, "Ouvrir un document existant")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Enregistrer", 	tb_save_xpm,	NULL, "Enregistrer le document")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Enregistrer sous", 	tb_save_as_xpm,	NULL, "Enregistrer le document sous un autre nom")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Imprimer",	tb_print_xpm,	NULL, "Imprimer le document")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Annuler",	tb_undo_xpm,	NULL, "Annuler la frappe")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Répéter",	tb_redo_xpm,	NULL, "Répéter la frappe")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Couper",	tb_cut_xpm,	NULL, "Couper")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Copier",	tb_copy_xpm,	NULL, "Copier")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Coller",	tb_paste_xpm,	NULL, "Coller")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Style",	NoIcon,		NULL, "Style")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Polices",	NoIcon,		NULL, "Polices")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Taille",	NoIcon,		NULL, "Taille")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Gras",		tb_text_bold_xpm,	NULL, "Gras")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Italique",	tb_text_italic_xpm,	NULL, "Italique")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Souligné",tb_text_underline_xpm,	NULL, "Souligné")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Strike",   tb_text_strikeout_xpm,	NULL, "Strikeout")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Gauche",		tb_text_align_left_xpm,		NULL, "Alignement Gauche")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Centrer",	tb_text_center_xpm,	NULL, "Alignement Centrer")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Droite",	tb_text_align_right_xpm,	NULL, "Alignement Droite")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Justifier",	tb_text_justify_xpm,	NULL, "Justify paragraph")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"None before",		tb_para_0before_xpm,	NULL, "Space before: None")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt before",		tb_para_12before_xpm,	NULL, "Space before: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Single Spacing",	tb_line_single_space_xpm,	NULL, "Single spacing")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 Spacing",		tb_line_middle_space_xpm,	NULL, "1.5 spacing")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Double Spacing",	tb_line_double_space_xpm,	NULL, "Double spacing")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 Column",			tb_1column_xpm,			NULL, "1 Column")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 Columns",		tb_2column_xpm,			NULL, "2 Columns")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 Columns",		tb_3column_xpm,			NULL, "3 Columns")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Zoom",		NoIcon,			NULL, "Zoom")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
