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

// Norwegian Nynorsk translations provided by Karl Ove Hufthammer <huftis@bigfoot.com>

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

BeginSet(no,NYN,UT_TRUE)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&Fil",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Ny", 			"Opprettar eit nytt dokument")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"&Opn",			"Opnar eit eksisterande dokument")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Close", 			"Lukkar dokumentet")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Lagr", 			"Lagrar dokumentet")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Lagr &som", 		"Lagrar dokumentet med eit anna namn")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"Utskr&iftsformat",		"Endrar utskriftsalternativ")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"Skriv &ut",			"Skriv ut heile eller delar av dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Opnar dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Opnar dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Opnar dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Opnar dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Opnar dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Opnar dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Opnar dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Opnar dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Opnar dette dokumentet")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"&Avslutt", 			"Lukkar alle vindauge og avsluttar programmet")

	MenuLabel(AP_MENU_ID_EDIT,				"&Rediger",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Angr",			"Angrar siste handling")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Gjer om",			"Gjer om siste angra handling")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"Klipp &ut",				"Klipper ut det merkte området og plasserer det på utklippstavla")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Kopier",			"Kopierer det merkte området til utklippstavla")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"&Lim inn",			"Sett inn innhaldet på utklippstavla")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"&Fjern",			"Slettar det merkte området")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"&Merk alt",		"Merker heile dokumentet")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Søk etter",			"Søker etter valt tekst")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"E&rstatt",			"Erstattar valt tekst med ein annan tekst")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"&Gå til",			"Flyttar innsettingspunktet til ein ny plass")
	
	MenuLabel(AP_MENU_ID_VIEW,				"&Vis",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Verktøylinjer",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standard",		"Viser eller skjuler standardverktøylinja")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"&Formatering",		"Viser eller skjuler formateringsverktøylinja")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Linjal",			"Viser eller skjuler linjalar")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"&Statuslinje",		"Viser eller skjuler statuslinja")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"Vis &avsnittsmerke",	"Viser eller skjuler teikn som ikkje blir skrivne ut")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Topptekst og botntekst",	"Viser tekst på toppen eller botnen av kvar side")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Zoom",			"Endrar visingsstorleiken på skjermen")

	MenuLabel(AP_MENU_ID_INSERT,			"Sett &inn",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Skift",			"Sett inn side-, spalte- eller inndelingsskift")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"Side&tal",	"Sett inn eit sidetal som blir automatisk oppdatert")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"Dat&o og klokkeslett",	"Sett inn gjeldande dato og/eller klokkeslett")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"F&elt",			"Sett inn eit felt")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"S&ymbol",			"Sett inn eit symbol eller anna spesialteikn")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,	"Bi&lde",			"Sett inn eit bilde frå ei grafikkfil")

	MenuLabel(AP_MENU_ID_FORMAT,			"F&ormat",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Skrift",			"Endrar skrift på merkt tekst")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Avsnitt",		"Endrar utsjånaden til merkt avsnitt")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"&Punktmerking og nummerering",	"Legg til eller endrar punktmerking eller nummerering for merkt(e) avsnitt")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"&Kantlinjer og skyggelegging",		"Endrar kantlinjene og skyggelegginga for det merkte området")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"Spalte&r",			"Endrar talet på spalter")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"Sti&l",			"Brukar eller endrer stil på det merkte området")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"Ta&bulatorar",			"Angjev tabularar")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Halvfeit",			"Aktiverer/deaktiverer bruk av halvfeit skrift i det merkte området")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"Ku&rsiv",			"Aktiverer/deaktiverer bruk av kursivert skrift i det merkte området")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Understreking",		"Aktiverer/deaktiverer bruk av understreking i det merkte området")
	MenuLabel(AP_MENU_ID_FMT_OVERLINE,		"&Overstreking",		"Aktiverer/deaktiverer bruk av overstreking i det merkte området")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"&Gjennomstreking",			"Aktiverer/deaktiverer bruk av gjennomstreking i det merkte området")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,	"H&eva skrift",		"Aktiverer/deaktiverer bruk av heva skrift i det merkte området")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"Se&nka skrift",		"Aktiverer/deaktiverer bruk av senka skrift i det merkte området")

	MenuLabel(AP_MENU_ID_TOOLS,				"V&erktøy",			NULL)   
	MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"&Stavekontroll",		"Stavekontrollerer dokumentet")
	MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,	"Ordt&elling",		"Tel talet på ord i dokumentet")
	MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"&Alternativ",			"Endrar alternativ")

	MenuLabel(AP_MENU_ID_ALIGN,				"&Justering",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Venstrejuster",			"Venstrejusterer avsnittet")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Midtstill",			"Midtstiller avsnittet")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Høgrejuster",			"Høgrejusterer avsnittet")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Blokkjuster",			"Blokkjusterer avsnittet")

	MenuLabel(AP_MENU_ID_WINDOW,			"Vi&ndauge",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&Nytt vindauge",		"Opnar eit nytt vindauge for dette dokumentet")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			"Vis dette vindauget")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			"Vis dette vindauget")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			"Vis dette vindauget")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			"Vis dette vindauget")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			"Vis dette vindauget")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			"Vis dette vindauget")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			"Vis dette vindauget")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			"Vis dette vindauget")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			"Vis dette vindauget")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"Fleire &vindauge",	"Viser eit liste over vindauge")

	MenuLabel(AP_MENU_ID_HELP,				"&Hjelp",			NULL)
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Om %s",		"Viser informasjon om programmet, versjonsnummer og opphavsrett")

	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",				"Erstatt med denne stavemåten")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",				"Erstatt med denne stavemåten")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",				"Erstatt med denne stavemåten")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",				"Erstatt med denne stavemåten")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",				"Erstatt med denne stavemåten")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",				"Erstatt med denne stavemåten")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",				"Erstatt med denne stavemåten")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",				"Erstatt med denne stavemåten")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",				"Erstatt med denne stavemåten")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"Ign&orer alle", 		"Ignorerer alltid dette ordet i gjeldande dokument")
	MenuLabel(AP_MENU_ID_SPELL_ADD,			"&Legg til", 			"Legg til ordet i eigendefinert ordliste")

	// ... add others here ...
	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
