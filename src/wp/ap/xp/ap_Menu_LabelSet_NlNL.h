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

// Dutch translations provided by albi <albi@linuxfan.com>

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

// We use the Win32 '&' character to denote a keyboard accelerator on a menu item.
// If your platform doesn't have a way to do accelerators or uses a different
// character, remove or change the '&' in your menu constructor code.

BeginSet(NlNL)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&Bestand",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Nieuw", 			"Maak een nieuw tekstbestand aan")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"&Open",			"Open bestaand tekstbestand")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Sluit", 			"Sluit tekstbestand")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Bewaar", 			"Bewaar tekstbestand")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Bewaar &Als", 		"Bewaar tekstbestand onder andere naam")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"Page Set&up",		"Verander de printer opties")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"&Print",			"Print alles of gedeeltes van het tekstbestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Open dit tekstbestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Open dit tekstbestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Open dit tekstbestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Open dit tekstbestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Open dit tekstbestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Open dit tekstbestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Open dit tekstbestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Open dit tekstbestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Open dit tekstbestand")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"A&fsluiten", 			"Sluit alle vensters in het programma en beeindig programma")

	MenuLabel(AP_MENU_ID_EDIT,				"&Bewerken",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Ongedaan maken",			"Ongedaan maken")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Ongedaan maken terugzetten",			"Maak zojuist ongedaan gemaakte weer ongedaan")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"Knip&pen",				"Knip het geselecteerde en plaats het op het prikbord")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Kopieer",			"Copieer het geselecteerde en plaats het op het prikbord")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"&Plakken",			"Invoegen prikbord inhoud")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"Uit&gummen",			"Verwijder het geselecteerde gedeelte")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"Selecteer A&lles",		"Selecteer het gehele tekstbestand")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Zoeken",			"Zoek de opgegeven tekst")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"V&ervang",			"Vervang geselecteerde tekst met de andere tekst")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"&Ga Naar",			"Verplaats invoegplaats naar aangewezen lokatie")
	MenuLabel(AP_MENU_ID_EDIT_OPTIONS,		"&Opties",			"Opties instellen")
	
	MenuLabel(AP_MENU_ID_VIEW,				"&Visueel",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Knoppenbalken",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standaard",		"Laat de standaard knoppenbalk zien of verberg deze")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"&Formatting",		"Laat zien of verberg de formatting knoppenbalk")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Ruler",			"Verberg of laat zien the rulers")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"&Status Balk",		"Laat status balk zien of verstop statusbalk")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"Laat Para&graaf zien",	"Laat niet-afdrukbare karakters zien")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Koptekst en Staarttekst",	"Bewerk tekst aan het begin of eind van iedere pagina")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Zoom",			"Veklein of vergroot the document display")

	MenuLabel(AP_MENU_ID_INSERT,			"&Invoegen",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Breuk",			"Invoegen van pagina, kolom, of sectie breuk")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"Pagina N&ummers",	"Voeg een automatisch-opgewaardeerde pagina nummer in")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"Datum en &Tijd",	"Invoegen van datum en/of tijd")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Veld",			"Voeg een berekend veld in")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"&Symbool",			"Voeg een symbool of ander speciaal karakter in")
	MenuLabel(AP_MENU_ID_INSERT_IMAGE,		"&Plaatje",			"Voeg een bestaand plaatje uit een ander bestand in")

	MenuLabel(AP_MENU_ID_FORMAT,			"F&ormaat",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Lettertype",			"Verander het lettertype van de geselecteerde tekst")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Paragraaf",		"Verander het formaat van de geselecteerde paragraaf")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"Bullets en &Nummering",	"Voeg toe of verander bullets and nummering voor geselecteerde paragraven")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"&Kantlijnen en Shaduw",		"Voeg borders and shaduw aan de geselecteerde tekst")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Kolommen",			"Verander het aantal kolommen")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"St&ijl",			"Definieer of voeg stijl toe voor het geselecteerde")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabs",			"Tabs instellen")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Vetgedrukt",			"Maak het geselecteerde vetgedrukt (omschakelen)")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Schuingedrukt",			"Maak het geselecteerde schuingedrukt (omschakelen)")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Onderstrepen",		"Onderstreep het geselecteerde (omschakelen)")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"Do&orhalen",			"Het geselecteerde doorhalen (omschakelen)")

	MenuLabel(AP_MENU_ID_ALIGN,				"&Kantlijn",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Links",			"Links-justify deze paragraaf")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Centreer",			"Center-align deze paragraaf")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Rechts",			"Rechts-justify deze paragraaf")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Rechtvaardig",			"Rechtvaardig deze paragraaf")

	MenuLabel(AP_MENU_ID_WINDOW,			"&Venster",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&Nieuw Venster",		"Open een nieuw venster voor dit tekstbestand")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			"Open dit venster")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			"Open dit venster")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			"Open dit venster")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			"Open dit venster")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			"Open dit venster")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			"Open dit venster")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			"Open dit venster")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			"Open dit venster")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			"Open dit venster")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Meer Vensters",	"Laat complete lijst van vensters zien")

	MenuLabel(AP_MENU_ID_HELP,				"&Help",			NULL)
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Over %s",		"Laat programma informatie zien, versie nummer, en copyright")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
