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

// Danish translations provided by Martin Willemoes Hansen <mwh@stampede.org>
//                             , Birger Langkjer <birger.langkjer@image.dk>
//                             and Rasmus Toftdahl Olesen <rto@post.tele.dk>

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

// We use the Win32 '&' character to denote a keyboard accelerator on a menu item.
// If your platform doesn't have a way to do accelerators or uses a different
// character, remove or change the '&' in your menu constructor code.

// If the third argument is UT_TRUE, then this is the fall-back for
// this language (named in the first argument).

BeginSet(da,DK,UT_TRUE)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&Filer",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Ny", 				"Nyt dokument")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"Å&bn",				"Åbn et eksisterende dokument")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Luk", 			"Luk dokumentet")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Gem", 			"Gem dokumentet")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Gem &som", 			"Gem dokumentet under et andet navn")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,		"&Sideopsætning",		"Ændr sideopsætning")
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
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"&Afslut", 			"Luk alle vinduer og afslut programmet")

	MenuLabel(AP_MENU_ID_EDIT,				"&Redigér",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"F&ortryd",			"Fortryd ændring")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Gentag",			"Gentag sidste fortrydte ændring")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"K&lip",			"Udklip det markerede og put det i udklipsholderen")
    MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Kopiér",			"Kopiér det markerede og put det i udklipsholderen")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"&Indsæt",			"Indsæt udklipsholderens indhold")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"&Ryd",				"Fjern det markerede")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,		"&Markér alt",		"Markér hele dokumentet")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Søg",				"Søg efter en angivet tekst")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"&Erstat",			"Erstat en angivet tekst med en anden tekst")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"Gå &til",			"Flyt markøren til en angivet placering")
	
	MenuLabel(AP_MENU_ID_VIEW,				"&Vis",				NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Værktøjslinjer",				NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standard",			"Vis eller skjul standard værktøjslinjen")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,		"&Formatering",			"Vis eller skjul formateringsværktøjslinjen")
	MenuLabel(AP_MENU_ID_VIEW_TB_EXTRA,		"&Ekstra",			"Vis eller skjul den ekstra værktøjslinje")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Lineal",			"Vis eller skjul linealerne")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,		"S&tatuslinje",			"Vis eller skjul statuslinjen")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"Vis &afsnit",		"Vis tegn der ikke skal udskrives")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"Side&hoved og sidefod",	"Redigér tekst i toppen eller i bunden af hver side")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Zoom",			"Formindsk eller forstør dokumentet på skærmen")

	MenuLabel(AP_MENU_ID_INSERT,				"&Indsæt",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Skift",			"Indsæt et side-, række- eller sektionsskift")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"Side&tal",			"Indsæt et automatisk opdateret sidetal")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,		"&Dato og klokkeslet",		"Indsæt datoen og/eller klokkeslæt")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Felt",			"Indsæt et kalkuleret felt")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"Sy&mbol",			"Indsæt et symbol eller andre specielle tegn")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,		"&Billede",			"Indsæt et billede fra en fil")

	MenuLabel(AP_MENU_ID_FORMAT,				"F&ormatér",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Skrifttype",			"Ændr skrifttype for den markerede tekst")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Afsnit",			"Ændr formatet for det markerede afsnit")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"&Punktopstilling",		"Tilføj eller ændr punktopstillingen i det markerede afsnit")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"&Rammer og skygge",		"Tilføj rammer og skygge til den markerede tekst")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"Ko&lonner",			"Ændr antallet af kolonner")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"St&il",			"Angiv eller tilføj stil for den markerede tekst")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabulatorer",			"Sæt tabulatorstop")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Fed",				"Gør den markerede tekst fed")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Kursiv",			"Gør den markerede tekst kursiv")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Understreg",		"Understreg den markerede tekst")
	MenuLabel(AP_MENU_ID_FMT_OVERLINE,		"&Overstreg",		"Overstreg den markerede tekst")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"&Gennemstreg",		"Gennemstreg den markerede tekst")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,		"&Hævet",		"Gør den markerede tekst højere end omgivelserne")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"Sæ&nket",		"Gør den markerede tekst lavere end omgivelserne")

	MenuLabel(AP_MENU_ID_TOOLS,				"Vær&ktøjer",			NULL)   
	MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"&Stavekontrol",	"Kontrollér dokumentet for forkert stavede ord")
	MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,	"O&rdoptælling",		"Optæl antallet af ord i dokumentet")
	MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"&Indstillinger",			"Opsæt programmet...")

	MenuLabel(AP_MENU_ID_ALIGN,				"&Justering",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Venstrestillet",		"Venstrestil afsnittet")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Centreret",			"Centrér afsnittet")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Højrestillet",		"Højrestil afsnittet")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Lige marginer",		"Lav lige marginer på afsnittet")

	MenuLabel(AP_MENU_ID_WINDOW,				"Vi&ndue",			NULL)
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
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Flere vinduer",		"Vis en komplet liste over vinduer")

	MenuLabel(AP_MENU_ID_HELP,				"&Hjælp",		NULL)
	MenuLabel(AP_MENU_ID_HELP_CONTENTS,		"&Indhold af hjælp",	"Vis indhold af hjælp")
	MenuLabel(AP_MENU_ID_HELP_INDEX,		"Hjælpe&indeks",		"Vis hjælpeindeks")
	MenuLabel(AP_MENU_ID_HELP_CHECKVER,		"Tjek &version",	"Vis programversionsnummer")
	MenuLabel(AP_MENU_ID_HELP_SEARCH,		"&Søg i hjælp",	"Vis programinformation, versionsnummer og ophavsret")
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Om %s",			"Vis programinformation, versionsnummer og ophavsret") 
	MenuLabel(AP_MENU_ID_HELP_ABOUTOS,		"Om &Open Source",	"Vis information om Open Source")


	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",				"Ændr til det foreslåede ord")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",				"Ændr til det foreslåede ord")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",				"Ændr til det foreslåede ord")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",				"Ændr til det foreslåede ord")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",				"Ændr til det foreslåede ord")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",				"Ændr til det foreslåede ord")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",				"Ændr til det foreslåede ord")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",				"Ændr til det foreslåede ord")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",				"Ændr til det foreslåede ord")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"&Ignorér alle", 		"Ignorér alle forekomster af dette ord i dokumentet")
	MenuLabel(AP_MENU_ID_SPELL_ADD,			"&Tilføj", 			"Tilføj dette ord til din personlige ordliste")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()










