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

// Swedish translations provided by Henrik Berg <henrik@lansen.se>

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

// We use the Win32 '&' character to denote a keyboard accelerator on a menu item.
// If your platform doesn't have a way to do accelerators or uses a different
// character, remove or change the '&' in your menu constructor code.

// If the third argument is true, then this is the fall-back for
// this language (named in the first argument).

BeginSetEnc(sv,SE,true,"iso-8859-1")

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&Arkiv",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Nytt", 			"Skapar ett nytt tomt dokument")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"&Öppna",			"Öppnar ett befintligt dokument")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"St&äng", 			"Stänger dokumentet")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Spara", 			"Sparar dokumentet")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Spara so&m", 		"Sparar dokumentet med ett nytt namn")
        MenuLabel(AP_MENU_ID_WEB_SAVEASWEB,               "Spara som &webbsida",   "Spara dokumentet som webbsida")
        MenuLabel(AP_MENU_ID_WEB_WEBPREVIEW,              "&Visa som webbsida",    "Visa dokumentet som webbsida")
 	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"Utskriftsforma&t", "Ändra inställningar för dokumentet")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"Skriv &ut",		"Skriver ut allt eller delar av dokument")
        MenuLabel(AP_MENU_ID_FILE_PRINT_DIRECTLY,   "Skriv ut &direkt",                       "Skriv ut med den interna PSdrivrutinen")
        MenuLabel(AP_MENU_ID_FILE_PRINT_PREVIEW, "&Förhandsgranska", "Förhandsgranska dokumentet vid utskrift")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Öppnar detta dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Öppnar detta dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Öppnar detta dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Öppnar detta dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Öppnar detta dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Öppnar detta dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Öppnar detta dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Öppnar detta dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Öppnar detta dokumentet")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"&Avsluta", 		"Stänger programmets alla fönster, och avslutar")

	MenuLabel(AP_MENU_ID_EDIT,				"&Redigera",		NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Ångra",			"Ångra inmatningen")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Upprepa",			"Upprepa föregående ångrade inmatning")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"&Klipp ut",		"Klipper ut markeringen och placerar den i Urklippet")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"K&opiera",			"Kopierar markeringen och placerar den i Urklippet")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"K&listra in",		"Infogar innehållet i Urklipp")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"&Radera",			"Raderar markeringen")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"&Markera allt",	"Markerar hela dokumentet")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Sök",				"Söker efter angiven text")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"&Ersätt",			"Ersätter angiven text med annan text")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"&Gå till",			"Gå till en ny plats i dokumentet")
        MenuLabel(AP_MENU_ID_EDIT_EDITHEADER,                   "Redigera sid&huvud",                "Redigera text vid dokumenthuvud")
        MenuLabel(AP_MENU_ID_EDIT_EDITFOOTER,                   "Redigera sid&fot",                "Redigera text vid dokumentfot")
     
	MenuLabel(AP_MENU_ID_VIEW,				"Vi&sa",			NULL)
     MenuLabel(AP_MENU_ID_VIEW_NORMAL, "&Normalläge", "Normalläge")
     MenuLabel(AP_MENU_ID_VIEW_WEB, "&Weblayoutläge", "Weblayoutläge")
     MenuLabel(AP_MENU_ID_VIEW_PRINT, "&Utskriftslayoutläge", "Utskriftslayoutläge")
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Verktygsfält",	NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standard",		"Visar eller döljer verktygsfältet Standard")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"&Formatera",		"Visar eller döljer verktygsfältet Formatera")
	MenuLabel(AP_MENU_ID_VIEW_TB_EXTRA,		"&Extra",			"Visar eller döljer verktygsfältet Extra")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Linjal",			"Visar eller döljer linjalen")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"S&tatusrad",		"Visar eller döljer statusraden")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"&Visa alla",		"Visar alla icke utskrivbara tecken")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Sidhuvud och sidfot",	"Redigera text vid huvud eller fot på varje sida")
        MenuLabel(AP_MENU_ID_VIEW_FULLSCREEN, "&Helsida", "Visa dokumentet som helsida")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Zooma",			"Minska eller förstora visningen av dokumentet")

	MenuLabel(AP_MENU_ID_INSERT,			"&Infoga",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Brytning",		"Infogar sid-, spalt-, eller styckebrytning")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"Sidn&ummer",		"Infogar sidnumrering som uppdateras automatisk")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"Datum och &tid",	"Infogar dagens datum och/eller tid")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Fält",			"Infogar ett automatiskt fält")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"Sy&mbol",			"Infogar en symbol eller annat specialtecken")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,	"&Bildobjekt",		"Infogar en ny bild från en annan fil")

	MenuLabel(AP_MENU_ID_FORMAT,			"Forma&t",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"T&ecken",			"Väljer teckensnitt för aktuell markering")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Stycke",			"Formaterar aktuellt eller markerade stycken")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"Punkter och &numrering",	"Infogar eller ändrar punkter och numrering för markerade stycken")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"Kantlinjer och f&yllning",	"Infogar kantlinjer och fyllning till aktuell markering")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"S&palter",			"Ändrar antalet spalter")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"Format&mall", "Definiera och använda formatmallar")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabbar",			"Ställer in tabbar")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Fet",				"Gör aktuell markering fetstil (växlande)")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Kursiv",			"Gör aktuell markering kursiv (växlande)")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Understruken",	"Gör aktuell markering understruken (växlande)")
	MenuLabel(AP_MENU_ID_FMT_OVERLINE,		"&Överstruken",		"Gör aktuell markering överstruken (växlande)")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"&Genomstruken",	"Gör aktuell markering genomstruken (växlande)")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,	"Upp&höjd",			"Gör aktuell markering upphöjd (växlande)")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"Neds&änkt",		"Gör aktuell markering nedsänkt (växlande)")

	MenuLabel(AP_MENU_ID_TOOLS,				"&Verktyg",			NULL)
        MenuLabel(AP_MENU_ID_TOOLS_SPELLING, "&Stavning", NULL)
	MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"&Stavning",		"Kontroller stavningen i dokumentet")
        MenuLabel(AP_MENU_ID_TOOLS_AUTOSPELL, "&Automatisk Stavningskontroll","Automatisk stavningskontroll av dokumentet")
 	MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,	"&Ordstatistik",	"Räkna antal ord, rader, paragrafer, osv i dokumentet")
	MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"&Alternativ",		"Anger alternativ")
        MenuLabel(AP_MENU_ID_TOOLS_LANGUAGE, "S&pråk", "Ändra språk på den markerade texten")

	MenuLabel(AP_MENU_ID_ALIGN,				"&Justering",		NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Vänster",			"Vänsterjustera aktuellt eller markerade stycken")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Centrerat",		"Centrera aktuellt eller markerade stycken")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Höger",			"Högerjustera aktuellt eller markerade stycken")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Marginaler",		"Marginaljustera aktuellt eller markerade stycken")

	MenuLabel(AP_MENU_ID_WINDOW,			"F&önster",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&Nytt fönster",	"Öppnar ett annat fönster för dokumentet")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			"Aktiverar detta fönstret")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			"Aktiverar detta fönstret")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			"Aktiverar detta fönstret")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			"Aktiverar detta fönstret")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			"Aktiverar detta fönstret")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			"Aktiverar detta fönstret")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			"Aktiverar detta fönstret")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			"Aktiverar detta fönstret")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			"Aktiverar detta fönstret")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Fler fönster",	"Visa hela lista med fönster")

	MenuLabel(AP_MENU_ID_HELP,				"&Hjälp",			NULL)
	MenuLabel(AP_MENU_ID_HELP_CONTENTS,		"Hjälp &Contents",	"Visar hjälp Contents")
	MenuLabel(AP_MENU_ID_HELP_INDEX,		"Hjälp &index",		"Visar hjälp Index")
	MenuLabel(AP_MENU_ID_HELP_CHECKVER,		"Visa &version",	"Visar programmets versionsnummer")
	MenuLabel(AP_MENU_ID_HELP_SEARCH,		"&Search for Help",	"Söker i hjälpen")
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Om %s",			"Visar programinformation, versionsnummer, och copyright")
	MenuLabel(AP_MENU_ID_HELP_ABOUTOS,		"About &Open Source",	"Visar information om Open Source")

	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",				"Ändrar till föreslagen stavning")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",				"Ändrar till föreslagen stavning")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",				"Ändrar till föreslagen stavning")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",				"Ändrar till föreslagen stavning")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",				"Ändrar till föreslagen stavning")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",				"Ändrar till föreslagen stavning")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",				"Ändrar till föreslagen stavning")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",				"Ändrar till föreslagen stavning")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",				"Ändrar till föreslagen stavning")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"&Ignorera alla", 	"Ignorera alla förekomster av detta ord i dokumentet")
	MenuLabel(AP_MENU_ID_SPELL_ADD,			"&Lägg till", 		"Lägg till detta ord till den egna ordlistan")

     /* autotext submenu labels */
     /* Should probably be translated as well.
     MenuLabel(AP_MENU_ID_INSERT_AUTOTEXT, "&Autotext", "")
     MenuLabel(AP_MENU_ID_AUTOTEXT_ATTN, "Attention:", "")
     MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING, "Closing:", "") 
     MenuLabel(AP_MENU_ID_AUTOTEXT_MAIL, "Mail Instructions:", "")
     MenuLabel(AP_MENU_ID_AUTOTEXT_REFERENCE, "Reference:", "")
     MenuLabel(AP_MENU_ID_AUTOTEXT_SALUTATION, "Salutation:", "")
     MenuLabel(AP_MENU_ID_AUTOTEXT_SUBJECT, "Subject:", "")
     MenuLabel(AP_MENU_ID_AUTOTEXT_EMAIL, "Email:", "")
     */

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
