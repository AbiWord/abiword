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

// Norwegian translations provided by Edouard Lafargue / Lars Ranheim
//                                    Edouard.Lafargue@bigfoot.com

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

// We use the Win32 '&' character to denote a keyboard accelerator on a menu item.
// If your platform doesn't have a way to do accelerators or uses a different
// character, remove or change the '&' in your menu constructor code.

// If the second argument is UT_TRUE, then this is the fall-back for
// this language (named in the first two letters).

BeginSet(NoNO,UT_TRUE)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&Fil",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Ny", 				"Nytt dokument")
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"&Åpne",       			"Åpne et eksisterende dokument")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Lukk", 			"Lukk dokument")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Lagre", 			"Lagre dokument")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"L&agre som", 			"Lagre dokument under et annet navn")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,		"S&ideopsett",		"Endre sideopsett")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"Skriv &ut",			"Skriv ut hele dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Åpne dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Åpne dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Åpne dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Åpne dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Åpne dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Åpne dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Åpne dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Åpne dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Åpne dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"&Avslutt", 			"Lukk alle vinduer i programmet og avslutt")

	MenuLabel(AP_MENU_ID_EDIT,				"R&ediger",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"Omgjø&re",			"Gjøre om redigeringen")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"Gjen&ta",			"Gjenta siste fortrydelse")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"&Klipp ut",			"Klipp markeringen og legg det i utklipsboken")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"K&opier",			"Kopier markeringen og legg det i utklipsboken")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"Lim i&nn",			"Lim inn utklipsbokens innhold")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"S&lett",			"Slett markeringen")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,		"&Marker alt",			"Marker hele dokumentet")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Søk",				"Søk etter tekst")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"&Erstatt",			"Erstatt med en annen tekst")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"Gå t&il",			"Flytt markøren til et spesifisert sted")
	MenuLabel(AP_MENU_ID_EDIT_SPELL,		"&Spelling",		"Check the document for incorrect spelling")
	MenuLabel(AP_MENU_ID_EDIT_OPTIONS,		"Egens&kaper",			"Sett egenskaper")
	
	MenuLabel(AP_MENU_ID_VIEW,				"&Vis",				NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Verktøylinjer",				NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standard",			"Vis eller fjern standard verktøylinje")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,		"&Formatering",			"Vis eller fjern formaterings verktøylinje")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Lineal",			"Vis eller fjern linealer")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,		"S&tatuslinje",			"Vis eller fjern statuslinjen")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"Vis Para&grafer",		"Vis tegn som ikke skal skrives ut")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Topptekst og bunntekst",	"Rediger tekst på toppen eller i bunnen av hver side")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Zoom",			"Forminsk eller forstørr dokumentet på skjermen")

	MenuLabel(AP_MENU_ID_INSERT,				"&Erstatt ",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Skift",			"Erstatt en side, rekke, eller sektions skift")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"Side t&al",			"Erstatt en automatisk oppdatert side nummer")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,		"&Dato og tid",		"Erstatt dato og/eller tid")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"F&elt",			"Erstatt et kalkuleret felt")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"Sy&mbol",			"Erstatt et symbol eller andre spesiele tegn")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,		"Bi&lde",			"Erstatt et bilde fra en fil")

	MenuLabel(AP_MENU_ID_FORMAT,				"Forma&ter",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Skrifttype",			"Endre skrifttypen på den markerte tekst")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"Pa&ragraf",			"Endre formatet på den markerte paragraf")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"Punktopst&illing",		"Tilføy eller endre punktopstillingen for den markerte paragraf")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"Ra&mmer og skygge",		"Tilføy rammer og skygge til den markerte tekst")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Kolonner",			"Endre antallet kolonner")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"&Font stil",			"Definer eller tilføy skrifttype på den markerte tekst")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabulatorer",			"Sett tabulator stop")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Fet",			"Gjør markert tekst fet (skift)")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Kursiv",			"Gjør markert tekst kursiv (skift)")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Understreket",		"Understrek markert tekst (skift)")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"&Gjennomstreket",		"Gjennomstrek markert tekst (skift)")

	MenuLabel(AP_MENU_ID_ALIGN,				"&Juster",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Venstrestillet",		"Venstrestil paragrafen")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Sentrert",			"Sentrerer paragrafen")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Høyrestillet",		"Høyrestil paragrafen")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Like margener",		"Sett like marger på paragrafen")

	MenuLabel(AP_MENU_ID_WINDOW,				"&Vindu",			NULL)
        MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&Nytt vindu",			"Åpne et nytt vindu for dokumentet")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			"Hent dette vindu")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			"Hent dette vindu")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			"Hent dette vindu")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			"Hent dette vindu")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			"Hent dette vindu")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			"Hent dette vindu")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			"Hent dette vindu")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			"Hent dette vindu")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			"Hent dette vindu")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Flere vinduer",		"Vis full liste av vinduer")

	MenuLabel(AP_MENU_ID_HELP,				"&Hjelp",			NULL)
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Om %s",			"Vis program information, versions nummer, og copyright")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
