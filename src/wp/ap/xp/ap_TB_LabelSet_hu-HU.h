/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

// Hungarian translations provided by Tamas Decsi <tamas.decsi@techie.com>

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

BeginSet(hu,HU,UT_TRUE)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Új", 		tb_new_xpm,		NULL, "Új dokumentum létrehozása")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Megnyit",		tb_open_xpm,	NULL, "Meglévõ dokumentum megnyitása")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Ment", 	tb_save_xpm,	NULL, "A dokumentum mentése")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Mentés másként", 	tb_save_as_xpm,	NULL, "A dokumentum mentése más néven")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Nyomtat",	tb_print_xpm,	NULL, "A dokumentum nyomtatása")

	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Visszavon",		tb_undo_xpm,	NULL, "A szerkesztés visszavonása")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Újra",		tb_redo_xpm,	NULL, "A szerkesztés ismételt végrehajtása")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Kivágás",		tb_cut_xpm,		NULL, "Kivágás")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Másolás",		tb_copy_xpm,	NULL, "Másolás")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Beillesztés",	tb_paste_xpm,	NULL, "Beillesztés")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Stílus",	NoIcon,			NULL, "Stílus")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Betûtípus",		NoIcon,			NULL, "Betûtípus")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Betû Méret", NoIcon,		NULL, "Betû Méret")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Félkövér",		tb_text_bold_F_xpm,		NULL, "Félkövér")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Dõlt",	tb_text_italic_D_xpm,	NULL, "Dõlt")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Aláhúzott",tb_text_underline_A_xpm,	NULL, "Aláhúzott")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Felülvonás",tb_text_overline_F_xpm,	NULL, "Felülvonás")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Kihúzott",   tb_text_strikeout_K_xpm,	NULL, "Kihúzott")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Felsõ index",	tb_text_superscript_xpm,	NULL, "Felsõ index")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Alsó index",	tb_text_subscript_xpm,		NULL, "Alsó index")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Szimbólum",	tb_symbol_xpm,		NULL, "Szimbólum beillesztése")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Balra",		tb_text_align_left_xpm,		NULL, "Balra igazítás")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Középre",	tb_text_center_xpm,	NULL, "Középre igazítás")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Jobbra",	tb_text_align_right_xpm,	NULL, "Jobbra igazítás")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Sorkiegyenlítés",	tb_text_justify_xpm,	NULL, "A bekezdés sorkiegyenlítése")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Nincs elõtte",		tb_para_0before_xpm,	NULL, "Nincs elõtte hely")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pont elõtte",		tb_para_12before_xpm,	NULL, "12 pont hely van elõtte")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Szimpla sorköz",	tb_line_single_space_xpm,	NULL, "Szimpla sorköz")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1,5 sor",		tb_line_middle_space_xpm,	NULL, "1,5 sor")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Dupla sorköz",	tb_line_double_space_xpm,	NULL, "Dupla sorköz")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 Hasáb",			tb_1column_xpm,			NULL, "1 Hasáb")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 Hasáb",		tb_2column_xpm,			NULL, "2 Hasáb")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 Hasáb",		tb_3column_xpm,			NULL, "3 Hasáb")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Nagyítás",		NoIcon,			NULL, "Nagyítás")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,		"Felsorolások",		tb_lists_bullets_xpm,		NULL,		"Felsorolások kezdése/lezárása")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,		"Sorszámozott Listák",		tb_lists_numbers_xpm,		NULL,		"Sorszámozott Listák kezdése/lezárása")
	
	// ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
