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

// German Translation provided by Christian Terboven <cterboven@gmx.net>

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
// Note: the icon name here is to a specific icon (pixmap or bitmap or whatever)AbiWord
// Note: that will always be in the application.  if, for example, a big fat 'B'
// Note: for BOLD doesn't make sense in another language, change the entry in
// Note: the localization and add the icon to whatever table.

// Note: if a tool item does not use an icon (like a combo box), use the
// Note: constant "NoIcon" in that column.

// If the third argument is UT_TRUE, then this is the fall-back for
// this language (named in the first argument).

BeginSet(de,DE,UT_TRUE)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,		NULL)

	//          (id, 		                    szLabel,	IconName,	    szToolTip,	szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Neu",		tb_new_xpm,		NULL,		"Neues Dokument erstellen")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Öffnen",	tb_open_xpm,	NULL,		"Vorhandenes Dokument öffnen")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Speichern", 	tb_save_xpm,NULL,		"Aktuelles Dokument speichern")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Speichern unter", 	tb_save_as_xpm,	NULL,	"Aktuelles Dokument unter einem anderen Namen speichern")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Drucken",	tb_print_xpm,	NULL,		"Dokument oder Teile davon drucken")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Rückgängig",	tb_undo_xpm,NULL,		"Letzten Befehl rückgängig machen")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Wiederholen",	tb_redo_xpm,NULL,		"Letzten Befehl wiederholen")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Ausschneiden",	tb_cut_xpm,	NULL,		"Markierung löschen und in die Zwischenablage kopieren")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Kopieren",	tb_copy_xpm,	NULL,		"Markierung in die Zwischenablage kopieren")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Einfügen",	tb_paste_xpm,	NULL,		"Inhalt der Zwischenablage einfügen")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Stil",		NoIcon,			NULL,		"Stil")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Schriftart",   NoIcon,		NULL,		"Schriftart")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Schriftgröße", NoIcon,		NULL,		"Schriftgröße")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Fett",		tb_text_bold_xpm,	NULL,	"Fett")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Kursiv",	tb_text_italic_xpm,	NULL,	"Kursiv")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Unterstrichen",tb_text_underline_xpm,	NULL,	"Unterstrichen")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Überstrichen",tb_text_overline_xpm,	NULL, "Überstrichen")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Durchgestrichen", tb_text_strikeout_xpm,	NULL,	"Durchgestrichen")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Höhergestellt",	tb_text_superscript_xpm,	NULL, "Höhergestellt")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Tiefergestellt",	tb_text_subscript_xpm,		NULL, "Tiefergestellt")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Links",	tb_text_align_left_xpm, NULL, "Links ausrichten")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Zentriert",tb_text_center_xpm,		NULL, "Zentriert ausrichten")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Rechts",	tb_text_align_right_xpm,NULL, "Rechts ausrichten")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Ausrichten",	tb_text_justify_xpm,	NULL, "Ausrichten")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Keinen",	tb_para_0before_xpm,	NULL,	"Keinen Zeilenabstand")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt",	tb_para_12before_xpm,	NULL,	"Zeilenabstand: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Einfacher Zeilenabstand",	tb_line_single_space_xpm,	NULL,	"Einfacher Zeilenabstand")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 Zeilenabstand",	   tb_line_middle_space_xpm,	NULL, "1.5 Zeilenabstand")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Doppelter Zeilenabstand", tb_line_double_space_xpm,	NULL, "Doppelter Zeilenabstand")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,		"1 Spalte",		tb_1column_xpm,	NULL,	"1 Spalte")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,		"2 Spalten",	tb_2column_xpm,	NULL,	"2 Spalten")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,		"3 Spalten",	tb_3column_xpm,	NULL,	"3 Spalten")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"Zoom",			NoIcon,			NULL,	"Zoom")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
