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

// Czech translation provided by Zbynek Miksa <xmiksa@informatics.muni.cz>
// and Ladislav Michl <xmichl03@stud.fee.vutbr.cz>

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

BeginSet(cs,CZ,true)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,			"&Soubor",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Nový", 			"Vytvoøit nový dokument")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"&Otevøít",			"Otevøít existující dokument")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Zavøít", 			"Zavøít dokument")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Ulo¾it", 			"Ulo¾it dokument")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Ulo¾it &jako", 		"Ulo¾it dokument pod jiným jménem")
	MenuLabel(AP_MENU_ID_FILE_SAVEASWEB,		"Ulo¾it jako &web",		"Ulo¾it dokument jako web stránku")
	MenuLabel(AP_MENU_ID_FILE_WEBPREVIEW,		"Pre&view web page",		"Preview the document as a web page")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,		"Nasta&vení tisku ",		"Zmìnit nastavení tisku")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"&Tisk",			"Vytisknout celý dokument")
	MenuLabel(AP_MENU_ID_FILE_PRINT_DIRECTLY,	"&Pøímý tisk",			"Vytisknout pøes vnitøní PS ovladaè")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Otevøít tento dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Otevøít tento dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Otevøít tento dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Otevøít tento dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Otevøít tento dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Otevøít tento dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Otevøít tento dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Otevøít tento dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Otevøít tento dokument")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"&Konec", 			"Zavøít v¹echny okna a ukonèit aplikaci")

	MenuLabel(AP_MENU_ID_EDIT,			"Úpr&avy",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Zpìt",			"Zpìt editace")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"V&pøed",			"Redo previously undone editing")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"&Vyjmout",			"Pøesunout oznaèené oblasti do schránky")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Kopírovat",			"Zkopírovat oznaèené oblasti do schránky")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"V&lo¾it",			"Vlo¾it obsah schránky")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"Od&stranit",			"Smazat oznaèenou oblast")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,		"Vybr&at v¹e",			"Vybrat celý dokument")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Najít",			"Nalézt urèitý text")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"Nah&radit",			"Nahradit urèitý text jiným")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"&Skok na",			"Pøesun kurzoru na po¾adovanou stranu")
	
	MenuLabel(AP_MENU_ID_VIEW,			"&Zobrazit",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Nástroje",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standartní",			"Zobrazit nebo skrýt standartní panel nástrojù")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,		"&Formát",			"Zobrazit nebo skrýt panel nástrojù pro formátování")
	MenuLabel(AP_MENU_ID_VIEW_TB_EXTRA,             "&Extra",                       "Zobrazit nebo skrýt roz¹íøený panel nástrojù")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Pravítko",			"Zobrazit nebo skrýt pravítko")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,		"&Stavový øádek",		"Zobrazit nebo skrýt stavový øádek")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"&Okraje",			"Zobrazovat okraje stránky")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Záhlaví a zápatí",		"Editování textu v záhlaví a zápatí")
	MenuLabel(AP_MENU_ID_VIEW_FULLSCREEN,		"&Celá obrazovka",		"Zobrazit dokument pøes celou obrazovku")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Lupa",			"Zmen¹ení nebo zvìt¹ení zobrazení dokumentu")

	MenuLabel(AP_MENU_ID_INSERT,			"&Vlo¾it",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Zlom",			"Vlo¾it zlom stránky nebo sloupce")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"Èís&la stránek",		"Vlo¾it automatické èíslování")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,		"&Datum a èas",			"Vlo¾it èas a/nebo datum")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Pole",			"Vlo¾it poèítané pole")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"&Symbol",			"Vlo¾it symbol nebo jiný speciální znak")
	MenuLabel(AP_MENU_ID_INSERT_PICTURE,		"&Obrázek",			"Vlo¾it obrázek")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,		"Ze soubo&ru",			"Vlo¾it obrázek ze souboru")

	MenuLabel(AP_MENU_ID_FORMAT,			"F&ormát",			NULL)
	MenuLabel(AP_MENU_ID_FMT_LANGUAGE,		"&Jazyk",			"Zmìnit jazyk vybraného textu")
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Font",			"Zmìnit font vybraného textu")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Odstavec",			"Zmìnit formát vybraného odstavce")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"Odrá¾ky a èíslování",		"Pøidat nebo zmìnit odrá¾ky a èíslování vybraného odstavce")
	MenuLabel(AP_MENU_ID_FMT_DOCUMENT,		"&Dokument",			"Nastavit vlastnosti dokumentu (rozmìr stránky a tak...)")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"Okraje a stínování",		"Pøidat okraje a stínování vybranému textu")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Sloupce",			"Zmìnit poèet sloupcù")
	MenuLabel(AP_MENU_ID_FMT_TOGGLECASE,		"Zmìnit malá/velká",		"Zmìnit velikost písmen vybraného textu")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"St&yl",			"Definovat nebo aplikovat styl na vybraný")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabelátory",			"Nastavení tabelátorù")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Tuèné",			"Nastavení tuèného písma pro vybranou oblast")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Kurzíva",			"Nastavení kurzivy pro vybranou oblast")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Potr¾ení",			"Podtr¾ení písma ve vybrané oblasti")
	MenuLabel(AP_MENU_ID_FMT_OVERLINE,		"P&od èarou",			"Nastavení linky nad textem (zmìna)")

	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"Pøe¹k&rtnutí",			"Pøe¹krtnutí písma ve vybrané oblasti")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,		"Ho&rní index",			"Pøevede oznaèenou èást na horní index (zmìna)")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"Spo&dní index",		"Pøevede oznaèenou èást na spodní index (zmìna)")
        MenuLabel(AP_MENU_ID_TOOLS,			"&Nástroje",			NULL)
	MenuLabel(AP_MENU_ID_TOOLS_SPELLING,		"&Pravopis",			NULL)
        MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"&Pravopis",			"Najít pøeklepy v dokumentu")
	MenuLabel(AP_MENU_ID_TOOLS_AUTOSPELL,		"&Pravopis automaticky",	"Automaticky hledat pøeklepy")
        MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,		"Poèet &slov",			"Zjistit poèet slov v dokumentu")
        MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"&Mo¾nosti",			"Nastavení mo¾ností")
	MenuLabel(AP_MENU_ID_TOOLS_LANGUAGE,		"&Jazyk",			"Zmìnit jazyk vybraného textu")

	MenuLabel(AP_MENU_ID_ALIGN,			"&Umístìní",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Vlevo",			"Zarovnání odstavce vlevo")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Uprostøed",			"Zarovnání odstavce uprostøed")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"V&pravo",			"Zarovnání odstavce vpravo")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Do bloku",			"Zarovnání odstavce do bloku")

	MenuLabel(AP_MENU_ID_WINDOW,			"&Okno",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&Nové Okno",			"Otevøít nové okno")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			"Zdvihnutí toho okna")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			"Zdvihnutí toho okna")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			"Zdvihnutí toho okna")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			"Zdvihnutí toho okna")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			"Zdvihnutí toho okna")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			"Zdvihnutí toho okna")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			"Zdvihnutí toho okna")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			"Zdvihnutí toho okna")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			"Zdvihnutí toho okna")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Více oken",			"Zobrazit seznam v¹ech oken")

	MenuLabel(AP_MENU_ID_HELP,			"Nápo&vìda",			NULL)
        MenuLabel(AP_MENU_ID_HELP_CONTENTS,             "&Obsah nápovìdy",		"Zobrazí obsah nápovìdy")
        MenuLabel(AP_MENU_ID_HELP_INDEX,                "&Rejstøík",			"Zobrazí rejstøík nápovìdy")
        MenuLabel(AP_MENU_ID_HELP_CHECKVER,             "Uka¾ &verzi",			"Zobrazí èíslo verze programu")
        MenuLabel(AP_MENU_ID_HELP_SEARCH,               "&Hledej v nápovìdì",		"Hledej nápovìdu o...")
	MenuLabel(AP_MENU_ID_HELP_ABOUT,                "O &programu %s",               "Zobrazit informace o programu, èíslo verze a copyright")
        MenuLabel(AP_MENU_ID_HELP_ABOUTOS,              "O Open &Source",		"Zobrazí informace o Open Source")

	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",				"Zmìnit na tento navrhovaný pravopis")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",				"Zmìnit na tento navrhovaný pravopis")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",				"Zmìnit na tento navrhovaný pravopis")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",				"Zmìnit na tento navrhovaný pravopis")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",				"Zmìnit na tento navrhovaný pravopis")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",				"Zmìnit na tento navrhovaný pravopis")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",				"Zmìnit na tento navrhovaný pravopis")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",				"Zmìnit na tento navrhovaný pravopis")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",				"Zmìnit na tento navrhovaný pravopis")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"&Ignorovat v¹e", 		"Ignorovat v¹echny výskyty tohoto slova v dokumentu")
	MenuLabel(AP_MENU_ID_SPELL_ADD,		"&Pøidat", 			"Pøidat toto slovo do u¾ivatelského slovníku")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
