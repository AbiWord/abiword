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
// with help from several people in nl.comp.os.linux and casema.linux

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

BeginSet(DuNL,UT_TRUE)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,			"&Bestand",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Nieuw", 			"Maak een nieuw bestand aan")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"&Open",			"Open bestand")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Sluit", 			"Sluit bestand")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Bewaar", 			"Bewaar bestand")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"&Bewaar Als", 			"Bewaar bestand onder andere naam")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,		"&Pagina Opmaak",		"Verander de printer opties")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"&Print",			"Print alles of gedeeltes van het bestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Open dit bestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Open dit bestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Open dit bestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Open dit bestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Open dit bestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Open dit bestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Open dit bestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Open dit bestand")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Open dit bestand")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"&Afsluiten", 			"Sluit alle vensters en beeindig programma")

	MenuLabel(AP_MENU_ID_EDIT,			"&Bewerken",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Ongedaan maken",		"Ongedaan maken")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Ongedaan maken terugzetten",  "Zet zojuist ongedaan gemaakte weer terug")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"&Knippen",			"Knip het geselecteerde en plaats het op het prikbord")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Copieer",			"Copieer het geselecteerde en plaats het op het prikbord")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"&Plakken",			"Invoegen prikbord inhoud")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"&Uitgummen",			"Verwijder het geselecteerde gedeelte")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,		"&Selecteer Alles",		"Selecteer het gehele tekstbestand")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Zoeken",			"Zoek de opgegeven tekst")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"&Vervang",			"Vervang geselecteerde tekst met de andere tekst")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"&Ga Naar",			"Verplaats invoegplaats naar aangewezen lokatie")
	
	MenuLabel(AP_MENU_ID_VIEW,			"&Visueel",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Knoppenbalken",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standaard",			"Laat de standaard knoppenbalk zien of verberg deze")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,		"&Opmaak",			"Laat zien of verberg de opmaak knoppenbalk")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Lineaal",			"Verberg of laat lineaal instellingen zien ")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,		"&Status Balk",			"Laat status balk zien of verstop statusbalk")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"&Laat Paragraaf zien",		"Laat niet-afdrukbare karakters zien")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Koptekst en Staarttekst",	"Bewerk tekst aan het begin of eind van iedere pagina")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Overzicht",			"Verklein of vergroot het document overzicht")

	MenuLabel(AP_MENU_ID_INSERT,			"&Invoegen",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Onderbreking",		"Invoegen van pagina, kolom, of sectie onderbreking")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"&Pagina Nummers",		"Voeg een automatisch opgewaardeerd pagina nummer in")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,		"&Datum en Tijd",		"Invoegen van datum en/of tijd")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Veld",			"Voeg een berekend veld in")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"&Symbool",			"Voeg een symbool of ander speciaal karakter toe")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,	"&Plaatje",			"Voeg een bestaand plaatje uit een ander bestand toe")

	MenuLabel(AP_MENU_ID_FORMAT,			"&Formaat",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Lettertype",			"Verander het lettertype van de geselecteerde tekst")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Paragraaf",			"Verander het formaat van de geselecteerde paragraaf")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"&Opsommingstekens en Nummering",	"Voeg toe of verander opsommingstekens en nummering voor geselecteerde paragraven")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"&Kantlijnen en Shaduw",		"Voeg kantlijnen en shaduw aan de geselecteerde tekst")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Kolommen",			"Verander het aantal kolommen")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"&Stijl",			"Definieer stijl of voeg stijl toe voor het geselecteerde")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabs",			"Tabs instellen")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Vetgedrukt",			"Maak het geselecteerde vetgedrukt (omschakelen)")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Schuingedrukt",		"Maak het geselecteerde schuingedrukt (omschakelen)")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Onderstrepen",		"Onderstreep het geselecteerde (omschakelen)")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"&Doorhalen",			"Het geselecteerde doorhalen (omschakelen)")

	MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"&Spelling",		"Check the document for incorrect spelling")
	MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"&Opties",			"Opties instellen")

	MenuLabel(AP_MENU_ID_ALIGN,			"&Kantlijn",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Links",			"Deze paragraaf links uitlijnen")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Centreer",			"Centreer deze paragraaf")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Rechts",			"Deze paragraaf rechts uitlijnen")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Uitlijnen",			"Deze paragraaf uitlijnen")

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
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Meer Vensters",		"Laat complete lijst van vensters zien")

	MenuLabel(AP_MENU_ID_HELP,			"&Help",			NULL)
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Over %s",			"Laat programma informatie zien, versie nummer, en copyright")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
