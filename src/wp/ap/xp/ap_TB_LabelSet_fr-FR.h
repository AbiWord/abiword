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

/* traduction fcella@mahj.org du 24/05/1999
 * compl�t�e par Philippe Duperron <duperron@mail.dotcom.fr> 08/06/1999  */
/* modifications par Gilles Saint-Denis <stdenisg@cedep.net> et
   Christophe Caron <ChrisDigo@aol.com> Last update 03/03/2001 */

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

BeginSetEnc(fr,FR,true,"iso-8859-1")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,		NULL, NULL)

	//          (id, 		                szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Nouveau", 			tb_new_xpm,			NULL, "Cr�er un nouveau document")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Ouvrir",			tb_open_xpm,			NULL, "Ouvrir un document existant")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Enregistrer",			tb_save_xpm,			NULL, "Enregistrer le document")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Enregistrer sous",		tb_save_as_xpm,			NULL, "Enregistrer le document sous un nouveau nom")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Imprimer",			tb_print_xpm,			NULL, "Imprimer le document")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW, 	"Aper�u avant impression", 	tb_print_preview_xpm, 		NULL, "Donne un aper�u du document avant impression")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Annuler",			tb_undo_xpm,			NULL, "Annuler la frappe")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"R�p�ter",			tb_redo_xpm,			NULL, "R�p�ter la frappe")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Couper",			tb_cut_xpm,			NULL, "Couper")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Copier",			tb_copy_xpm,			NULL, "Copier")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Coller",			tb_paste_xpm,			NULL, "Coller")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_HEADER,		"En-t�te",			tb_edit_editheader_xpm,	NULL, "Editer l'en-t�te")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_FOOTER,		"Pied de page",		tb_edit_editfooter_xpm,	NULL, "Editer le pied de page")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEHEADER, "Enlever l'en-t�te",	tb_edit_removeheader_xpm,	NULL, "Enlever l'en-t�te de cette page du document")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEFOOTER, "Enlever le pied de page",tb_edit_removefooter_xpm,	NULL, "Enlever le pied de page de cette page du document")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK, 		"Orthographe", 			tb_spellcheck_xpm, 		NULL, "V�rifier l'orthographe du document")
	ToolbarLabel(AP_TOOLBAR_ID_IMG, 		"Ins�rer une image", 		tb_insert_graphic_xpm, 		NULL, "Ins�rer une image dans un document")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Style",			NoIcon,				NULL, "Style")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Police",			NoIcon,				NULL, "Police")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_HYPERLINK,	"Ins�rer un hyperlien", tb_hyperlink,	NULL, "Ins�rer un hyperlien dans un document")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOOKMARK,	"Ins�rer un signet",	tb_anchor,		NULL, "Ins�rer un signet dans un document")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Taille",			NoIcon,				NULL, "Taille")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Gras",				tb_text_bold_G_xpm,		NULL, "Gras")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Italique",			tb_text_italic_xpm,		NULL, "Italique")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Soulign�",			tb_text_underline_S_xpm,	NULL, "Soulign�")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Barr� haut",			tb_text_overline_xpm,		NULL, "Barr� haut")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Barr�",			tb_text_strikeout_B_xpm,	NULL, "Barr�")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_TOPLINE,		"Haut ligne",		tb_text_topline_xpm,	NULL, "Ligne au-dessus")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOTTOMLINE,	"Bas ligne",		tb_text_bottomline_xpm,	NULL, "Ligne au-dessous")
	ToolbarLabel(AP_TOOLBAR_ID_HELP, 		"Aide", 			tb_help_xpm, 			NULL, "Aide sur AbiWord")
	
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Exposant",			tb_text_superscript_xpm,	NULL, "Exposant")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Indice",			tb_text_subscript_xpm,		NULL, "Indice")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Symbole",			tb_symbol_xpm,			NULL, "Ins�rer un symbole")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Gauche",			tb_text_align_left_xpm,		NULL, "Aligner � gauche")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Centr�",			tb_text_center_xpm,		NULL, "Paragraphe centr�")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Droite",			tb_text_align_right_xpm,	NULL, "Aligner � droite")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Justifi�",			tb_text_justify_xpm,		NULL, "Paragraphe justifi�")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Pas d'espacement avant",	tb_para_0before_xpm,		NULL, "Espacement avant: aucun")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"Espacement de 12 pt avant",	tb_para_12before_xpm,		NULL, "Espacement avant: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Interligne simple",		tb_line_single_space_xpm,	NULL, "Interligne simple")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"Interligne : 1,5 lignes",	tb_line_middle_space_xpm,	NULL, "Interligne : 1,5 lignes")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Interligne double",		tb_line_double_space_xpm,	NULL, "Interligne double")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,		"Une colonne",			tb_1column_xpm,			NULL, "Une colonne")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,		"Deux colonnes",		tb_2column_xpm,			NULL, "Deux colonnes")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,		"Trois colonnes",		tb_3column_xpm,			NULL, "Trois colonnes")

	ToolbarLabel(AP_TOOLBAR_ID_VIEW_SHOWPARA,	"Afficher/Masquer �",		tb_view_showpara_xpm,		NULL, "Afficher/Masquer �")
	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"Zoom",				NoIcon,				NULL, "Zoom")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Liste � puce",			tb_lists_bullets_xpm,		NULL, "D�marrer/arr�ter une liste � puce")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Liste num�rot�e",		tb_lists_numbers_xpm,		NULL, "D�marrer/arr�ter une liste num�rot�e")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Premier plan",			tb_text_fgcolor_xpm,		NULL, "Changer la couleur du premier plan")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Arri�re plan",			tb_text_bgcolor_xpm,		NULL, "Changer la couleur de l'arri�re plan")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,		"Augmenter le retrait", 	tb_text_indent_xpm, 		NULL, "Augmenter le retrait")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"Diminuer le retrait", 		tb_text_unindent_xpm,		NULL, "Diminuer le retrait")

	ToolbarLabel(AP_TOOLBAR_ID_SCRIPT_PLAY,		"Script",				tb_script_play_xpm,			NULL, "Ex�cuter un script")

    ToolbarLabel(AP_TOOLBAR_ID_FMTPAINTER, 		"Peindre la mise en forme", tb_stock_paint_xpm, NULL, "Appliquer au texte s�lectionn�, la mise en forme du paragraphe copi�")

	// ... add others here ...

#ifdef BIDI_ENABLED
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR,	"Oriente le texte de gauche � droite",		tb_text_direction_ltr_xpm,	NULL, "Forcer la direction du texte de gauche � droite")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL,	"Oriente le texte de droite � gauche",		tb_text_direction_rtl_xpm,	NULL, "Forcer la direction du texte de droite � gauche")	
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,		"Orientation des paragraphes",			tb_text_dom_direction_rtl_xpm,	NULL, "Changer l'orientation dominante des paragraphes")
#endif
	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
