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

// Dutch translations provided by albi <albi@linuxfan.com>
// with help from several people in nl.comp.os.linux and casema.linux

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

// If the third argument is UT_TRUE, then this is the fall-back for
// this language (named in the first argument).

BeginSet(du,NL,UT_TRUE)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Nieuw", 		tb_new_xpm,			NULL, "Maak nieuw tekstbestand aan")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Open",			tb_open_xpm,			NULL, "Open bestaand tekstbestand")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Bewaar", 		tb_save_xpm,			NULL, "Bewaar tekstbestand")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Bewaar Als", 		tb_save_as_xpm,			NULL, "Bewaar tekstbestand onder een andere naam")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Print",		tb_print_xpm,			NULL, "Print tekstbestand")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Ongedaan maken",	tb_undo_xpm,			NULL, "Ongedaan maken")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Terugzetten",		tb_redo_xpm,			NULL, "Terugzetten bewerking")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Knip",			tb_cut_xpm,			NULL, "Knippen")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Copieeren",		tb_copy_xpm,			NULL, "Copieeren")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Plakken",		tb_paste_xpm,			NULL, "Plakken")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Stijl",		NoIcon,				NULL, "Stijl")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Lettertype",		NoIcon,				NULL, "Lettertype")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Letter Grootte",	NoIcon,				NULL, "Lettertype Grootte")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Vetgedrukt",		tb_text_bold_xpm,		NULL, "Vetgedrukt")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Schuingedrukt",	tb_text_italic_xpm,		NULL, "Schuingedrukt")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Onderstreep",		tb_text_underline_xpm,		NULL, "Onderstrepen")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Doorhalen",  		tb_text_strikeout_xpm,		NULL, "Doorhalen")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Links",		tb_text_align_left_xpm,		NULL, "Linker kantlijn")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Centreer",		tb_text_center_xpm,		NULL, "Centreer ")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Rechts",		tb_text_align_right_xpm,	NULL, "Rechter kantlijn")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Uitlijnen",		tb_text_justify_xpm,		NULL, "Paragraaf uitlijnen")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Niets voor",		tb_para_0before_xpm,		NULL, "Ruimte voor: Geen")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt voor",		tb_para_12before_xpm,		NULL, "Ruimte voor: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Enkele lege regel tussenvoegen",	tb_line_single_space_xpm,	NULL, "Enkele lege regel tussenvoegen")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 lege regel tussenvoegen",		tb_line_middle_space_xpm,	NULL, "1.5 lege regel tussenvoegen")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Twee lege regels tussenvoegen",	tb_line_double_space_xpm,	NULL, "Twee lege regels tussenvoegen")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,		"1 Kolom",		tb_1column_xpm,			NULL, "1 Kolom")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,		"2 Kolommen",		tb_2column_xpm,			NULL, "2 Kolommen")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,		"3 Kolommen",		tb_3column_xpm,			NULL, "3 Kolommen")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"Zoom",		NoIcon,			NULL, "Zoom")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
