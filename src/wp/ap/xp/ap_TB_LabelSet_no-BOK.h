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

// Norwegian translations provided by Edouard Lafargue <Edouard.Lafargue@bigfoot.com>
//                                and Gro Elin Hansen  <Gro.Hansen@student.uib.no>

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

// If the second argument is UT_TRUE, then this is the fall-back for
// this language (named in the first two letters).

BeginSet(NoNO,UT_TRUE)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Ny", 		tb_new_xpm,	NULL, "Nytt dokument")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Åpne",		tb_open_xpm,	NULL, "Åpne et eksisterende dokument")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Lagre",	tb_save_xpm,	NULL, "Lagre dokumentet")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Lagre som", 	tb_save_as_xpm,	NULL, "Lagre dokumentet med et nytt navn")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Skriv ut",	tb_print_xpm,	NULL, "Skriv ut dokumentet")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Angre",	tb_undo_xpm,	NULL, "Angre")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Gjenta",	tb_redo_xpm,	NULL, "Gjenta")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Klipp ut",	tb_cut_xpm,	NULL, "Klipp ut")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Kopier",	tb_copy_xpm,	NULL, "Kopier")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Lim inn",	tb_paste_xpm,	NULL, "Lim inn")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Stil",		NoIcon,			NULL, "Stil")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Skrift",	NoIcon,			NULL, "Skrift")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Skriftstørrelse", 	NoIcon,		NULL, "Skriftstørrelse")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Uthev",	tb_text_bold_F_xpm,	NULL, "Uthev")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Kursiv",	tb_text_italic_K_xpm,	NULL, "Kursiv")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Understreket",	tb_text_underline_xpm,	NULL, "Understreket")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Gjennomstreket",tb_text_strikeout_G_xpm,	NULL, "Gjennomstreket")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Venstre",	tb_text_align_left_xpm,	NULL, "Venstrestilt")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Midtstilt",	tb_text_center_xpm,	NULL, "Midtstilt")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Høyre",	tb_text_align_right_xpm,NULL, "Høyrestilt")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Like marger",	tb_text_justify_xpm,	NULL, "Like marger")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Ingen før",	tb_para_0before_xpm,	NULL, "Mellomrom før: Ingen")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt før",	tb_para_12before_xpm,	NULL, "Mellomrom før: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Enkel linjeavstand",	tb_line_single_space_xpm,	NULL, "Enkel linjeavstand")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 linjeavstand",	tb_line_middle_space_xpm,	NULL, "1.5 linjeavstand")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Dobbel linjeavstand",	tb_line_double_space_xpm,	NULL, "Dobbel linjeavstand")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,		"1 Kolonne",	tb_1column_xpm,		NULL, "1 Kolonne")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,		"2 Kolonner",	tb_2column_xpm,		NULL, "2 Kolonner")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,		"3 Kolonner",	tb_3column_xpm,		NULL, "3 Kolonner")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"Zoom",		NoIcon,			NULL, "Zoom")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
