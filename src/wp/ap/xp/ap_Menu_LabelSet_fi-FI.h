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

// Finnish translations provided by Jarmo Karvonen <jarmo@dawn.joensuu.fi>

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

BeginSet(FiFI,UT_TRUE)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&Tiedosto",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Uusi", 			"Luo uusi asiakirja")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"&Avaa",			"Avaa asiakirja")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Sulje", 			"Sulje asiakirja")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Tallenna", 			"Tallenna asiakirja")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Tallenna &Nimell‰", 		"Tallenna k‰sitelt‰v‰ asiakirja eri nimelle")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"S&ivun asetukset",		"Muuta sivun tulostus asetuksia")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"T&ulosta",			"Tulosta koko asiakirja")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Avaa t‰m‰ asiakirja")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Avaa t‰m‰ asiakirja")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Avaa t‰m‰ asiakirja")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Avaa t‰m‰ asiakirja")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Avaa t‰m‰ asiakirja")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Avaa t‰m‰ asiakirja")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Avaa t‰m‰ asiakirja")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Avaa t‰m‰ asiakirja")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Avaa t‰m‰ asiakirja")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"&Lopeta", 			"Sulje kaikki ikkunat ja sulje sovellus")

	MenuLabel(AP_MENU_ID_EDIT,				"&Muokkaa",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Peru",			"Peru muutokset")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Uudelleen",			"Palauta edellinen peruttu muokkaus")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"&Leikkaa",				"Leikkaa valinta ja sioita se leikepˆyd‰lle")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Kopioi",			"Kopioi valinta ja sioita se leikepˆyd‰lle")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"L&iit‰",			"Liit‰ leikepˆyd‰n sis‰ltˆ")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"&Tyhjenn‰",			"Tyhjenn‰ valinta")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"&Valitse kaikki",		"Valitse koko asiakirja")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Etsi",			"Etsi haluttu teksti")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"K&orvaa",			"Korvaa haluttu teksti toisella tekstill‰")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"&Mene",			"Siirr‰ osoitin haluttuun kohtaan")
	
	MenuLabel(AP_MENU_ID_VIEW,				"&N‰yt‰",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Tyˆkalurivit",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Perusvalikko",		"N‰yt‰ tai piilota perusvalikko")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"&Muotoilu",		"N‰yt‰ tai piilota muotoiluvalikot")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Viivain",			"N‰yt‰ tai piilota viivain")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"T&ilapalkki",		"N‰yt‰ tai piilota tilapalkki")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"&N‰yt‰ koodit",	"N‰yt‰ tulostumattomat merkit")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Yl‰tunniste ja alatunniste",	"Muokkaa yl‰- tai alatunnistetta")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Zoomaus",			"Pienenn‰ tai suurenna asiakirjaa n‰ytˆll‰")

	MenuLabel(AP_MENU_ID_INSERT,			"&Lis‰‰",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&V‰li",			"Lis‰‰ sivu, palsta tai kappale v‰li")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"Sivu &Numerot",	"Lis‰‰ p‰ivittyv‰ sivunumerointi")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"&P‰iv‰ys ja aika",	"Lis‰‰ p‰iv‰ys ja/tai aika")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Kentt‰",			"Lis‰‰ laskentakentt‰")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"&Symboli",			"Lis‰‰ symboli tai muu erikoismerkki")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,		"K&uva",			"Lis‰‰ kuva tiedostosta")

	MenuLabel(AP_MENU_ID_FORMAT,			"&Muotoile",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Fontti",			"Muuta Fonttia valitussa tekstiss‰")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"Ka&ppale",		"Muuta asetuksia valitussa kappaleessa")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"L&uettelomerkit ja numerointi",	"Lis‰‰ tai moukkaa luettelomerkkej‰ tai numerointia valitussa kappaleessa")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"&Reunat ja varjostus",		"Lis‰‰ reunat ja varjostus valittuun tekstiin")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Palstat",			"Muuta palstojen m‰‰r‰‰")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"T&yyli",			"M‰‰rit‰ tai valitse tyyli valittuun tekstiin")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Sisenn‰",			"M‰‰rit‰ sisennyskohdat")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Lihavoitu",			"Muuta valinta lihavoiduksi (p‰‰lle/pois)")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Kursivoitu",			"Muuta valinta kursivoiduksi (p‰‰lle/pois)")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Alleviivattu",		"Muuta valinta alleviivatuksi (p‰‰lle/pois)")
	MenuLabel(AP_MENU_ID_FMT_OVERLINE,              "Ylle&viivattu",            "Muuta valinta ylleviivaus (p‰‰lle/pois)")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"&Yliviivattu",			"Muuta valinta yliviivatuksi (p‰‰lle/pois)")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,   	"Yl‰&indeksi",         		"Muuta valinta yl‰indeksiksi (p‰‰lle/pois)")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,             "Alai&ndeksi",         		"Muuta valinta alaindeksiksi (p‰‰lle/pois)")

        MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"&Oikoluku",		        "Tarkista asiakirja virheellisit‰ sanoista")
	MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"&Asetukset",			"Muuta asetuksia")

	MenuLabel(AP_MENU_ID_ALIGN,				"&Tasaus",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Vasen",			"Vasemmalle tasattu kappale")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Keskit‰",			"Keskita kappale")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Oikea",			"Oikealle tasattu kappale")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Tasaa molemmat reunat",			"Tasaa molemmat reunat")

	MenuLabel(AP_MENU_ID_WINDOW,			"&Ikkuna",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&Uusi ikkuna",		"Avaa ikkuna uudelle asiakirjalle")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			"Nosta t‰m‰ ikkuna")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			"Nosta t‰m‰ ikkuna")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			"Nosta t‰m‰ ikkuna")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			"Nosta t‰m‰ ikkuna")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			"Nosta t‰m‰ ikkuna")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			"Nosta t‰m‰ ikkuna")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			"Nosta t‰m‰ ikkuna")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			"Nosta t‰m‰ ikkuna")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			"Nosta t‰m‰ ikkuna")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Lis‰‰ ikkunoita",	"N‰yt‰ koko lista ikkunoista")

	MenuLabel(AP_MENU_ID_HELP,				"&Ohje",			NULL)
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Tietoja %s",		"Kertoo lis‰tietoja ohjelmasta, sen versiosta ja kopiointi oikeuksista")

	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,   "%s",                           "Vaihda ehdotettu sana")
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,   "%s",                           "Vaihda ehdotettu sana")
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,   "%s",                           "Vaihda ehdotettu sana")
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,   "%s",                           "Vaihda ehdotettu sana")
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,   "%s",                           "Vaihda ehdotettu sana")
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,   "%s",                           "Vaihda ehdotettu sana")
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,   "%s",                           "Vaihda ehdotettu sana")
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,   "%s",                           "Vaihda ehdotettu sana")
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,   "%s",                           "Vaihda ehdotettu sana")
        MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,   "&Ohita kaikki",          "Ohita kaikki t‰m‰n sana esiintymiset t‰ss‰ asiakirjassa")
        MenuLabel(AP_MENU_ID_SPELL_ADD,                 "&Lis‰‰",                         "Lis‰‰ t‰m‰ sana omaan sanastoon")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
