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

// Danish translations provided by Martin Willemoes Hansen <mwh@stampede.org>

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

// We use the Win32 '&' character to denote a keyboard accelerator on a menu item.
// If your platform doesn't have a way to do accelerators or uses a different
// character, remove or change the '&' in your menu constructor code.

BeginSet(DaDK)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&Filer",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Ny", 				"Nyt dokument")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"Å&bn",				"Åbn et eksisterende dokument")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Luk", 			"Luk dokumentet")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Gem", 			"Gem dokumentet")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Ge&m som", 			"Gem dokumentet under et andet navn")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,		"S&ideopsætning",		"Ændre sideopsætningen")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"&Udskriv",			"Udskriv hele dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Åbn dette dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Åbn dette dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Åbn dette dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Åbn dette dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Åbn dette dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Åbn dette dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Åbn dette dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Åbn dette dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Åbn dette dokument")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"A&fslut", 			"Luk alle vinduer i programet og afslut")

	MenuLabel(AP_MENU_ID_EDIT,				"&Rediger",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"Fo&rtryd",			"Fortryd redigeringen")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"Gen&tag",			"Gentag sidste fortrydelse")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"&Klip",			"Klip markeringen og put det i udklipsholderen")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"K&opier",			"Kopier markeringen og put det i udklipsholderen")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"Sæt i&nd",			"Indsæt udklipsholderens indhold")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"Ry&d",				"Ryd markeringen")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,		"&Marker alt",			"Marker hele dokumentet")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Søg",				"Søg efter en specificeret tekst")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"&Erstat",			"Erstat en specificeret tekst med en anden tekst")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"Gå t&il",			"Flyt kurseren til en specificeret lokalisation")
	MenuLabel(AP_MENU_ID_EDIT_OPTIONS,		"O&psætning",			"Sæt opsætningen")
	
	MenuLabel(AP_MENU_ID_VIEW,				"&Vis",				NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Værktøjslinjer",				NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standard",			"Vis eller skjul standard værktøjslinjen")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,		"&Formatering",			"Vis eller skjul formatering værktøjslinjen")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Lineal",			"Vis eller skjul linealerne")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,		"S&tatuslinje",			"Vis eller skjul statuslinjen")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"Vis Para&grafer",		"Vis tegn der ikke skal udskrives")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"S&idehoved og sidefod",	"Rediger tekst på toppen eller i bunden af hver side")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Zoom",			"Formindsk eller forstør dokumentet på skærmen")

	MenuLabel(AP_MENU_ID_INSERT,				"&Indsæt",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Skift",			"Indsæt et side, række, eller sektions skift")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"Side t&al",			"Indsæt et automatiskopdateret side tal")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,		"&Dato og klokkeslæt",		"Indsæt datoen og/eller klokkeslæt")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"F&elt",			"Indsæt et kalkuleret felt")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"Sy&mbol",			"Indsæt et symbol eller andre speciele tegn")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,		"Bi&llede",			"Indsæt et billede fra en fil")

	MenuLabel(AP_MENU_ID_FORMAT,				"Forma&ter",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Skrifttype",			"Ændre skrifttypen på den markerede tekst")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"Pa&ragraf",			"Ændre formatet på den markerede paragraf")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"Punktopst&illing",		"Tilføj eller ændre punktopstillingen for den markerede paragraf")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"Ra&mmer og skyge",		"Tilføj rammer og skygge til den markerede tekst")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Kolonner",			"Ændre antallet af kolonner")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"Sti&l",			"Definer eller tilføj stil på den markerede tekst")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabulatorer",			"Sæt tabulator stop")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Fed",				"Lav det makerede fed (skift)")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Kursiv",			"Lav det markerede kursiv (skift)")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Understreget",		"Understreg det markerede (skift)")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"&Gennemstreget",		"Gennemstreg det markerede (skift)")

	MenuLabel(AP_MENU_ID_ALIGN,				"&Juster",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Venstrestillet",		"Venstrestil paragrafen")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Centreret",			"Centrerer paragrafen")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Højrestillet",		"Højrestil paragrafen")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Lige margener",		"Sæt lige margener på paragrafen")

	MenuLabel(AP_MENU_ID_WINDOW,				"&Vindue",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&Nyt vindue",			"Åbn et nyt vindue for dokumentet")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			"Hent dette vindue")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			"Hent dette vindue")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			"Hent dette vindue")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			"Hent dette vindue")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			"Hent dette vindue")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			"Hent dette vindue")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			"Hent dette vindue")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			"Hent dette vindue")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			"Hent dette vindue")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Flere vinduer",		"Vis fuld liste af vinduer")

	MenuLabel(AP_MENU_ID_HELP,				"&Hjælp",			NULL)
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Om %s",			"Vis program information, versions nummer, og copyright")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
