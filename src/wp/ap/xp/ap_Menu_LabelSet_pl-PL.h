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

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

// We use the Win32 '&' character to denote a keyboard accelerator on a menu item.
// If your platform doesn't have a way to do accelerators or uses a different
// character, remove or change the '&' in your menu constructor code.
// translated by: Sercxemulo <explo@poczta.wp.pl>
// last update: 07.14.2000A.D.
// If the third argument is UT_TRUE, then this is the fall-back for
// this language (named in the first argument).

BeginSet(pl,PL,UT_TRUE)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&Plik",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Nowy", 			"Stwórz nowy dokument")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"&Otwórz",			"Otwórz istniej±cy dokument")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Zamknij", 			"Zamknij dokument")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"Z&apisz", 			"Zapisz dokument")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Zapisz &Jako", 		"Zachowaj dokument pod inn± nazw±")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"Opcje D&ruku",		"Zmieñ ustawienia wydruku")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"&Drukuj",			"Drukuj ca³o¶æ lub czê¶æ dokumentu")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Otwórz ten dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Otwórz ten dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Otwórz ten dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Otwórz ten dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Otwórz ten dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Otwórz ten dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Otwórz ten dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Otwórz ten dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Otwórz ten dokument")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"&Koniec", 			"Zamknij wszystkie okna robocze i zakoñcz")

	MenuLabel(AP_MENU_ID_EDIT,				"&Edit",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Cofnij",			"Cofnij zmiany")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Cofnij zmiany",			"Cofnij poprawki, czyli poprzednie poprawki")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"Zachowaj i &wytnij",				"Wytnij zaznaczone i zapamiêtaj w Schowku")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Zachowaj",			"Zapamiêtaj w Schowku")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"&Wklej",			"Wklej zawarto¶æ Schowka")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"Wyc&zy¶æ",			"Skasuj zaznaczenie")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"Zaznacz W&szystko",		"Select the entire document")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Znajd¼",			"Znajd¼ podany tekst")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"Z&amieñ",			"Zmieñ tekst w innych tekstach")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"&Id¼ do",			"Przenie¶ punkt w którym wpisujesz tekst do")
	
	MenuLabel(AP_MENU_ID_VIEW,				"&Widok",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Pasek narzêdziowy",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standard",		"Poka¿ lub schowaj standardowy pasek narzêdziowy")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"&Formatowanie",		"Poka¿ lub schowaj pasek z narzêdziami do formatowania")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Przymiar",			"Poka¿ lub schowaj przymiar")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"&Pasek informacyjny",		"Poka¿ lub schowaj pasek informacjyjny")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"Poka¿ Para&grafy",	"Poka¿ znaczki niedrukowalne")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Nag³ówek i Stopka",	"Popraw tekst który siê uka¿e na górze i na dole ka¿dej stron")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Powiêkszenie",			"Zmniejsz i powiêksz obszar roboczy")

	MenuLabel(AP_MENU_ID_INSERT,			"&Wstaw",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Przerwij",			"Wstaw stronê, kolumnê lub przerwij zaznaczanie")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"N&umerowanie stron",	"Wstaw automatyczne numerowanie stron")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"Data i &Czas",	"Wstaw datê i/lub czas")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Pole",			"Wstaw wyliczone pole")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"&Symbol",			"Wstaw symbol lub inny specyficzny znak")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,	"&Obrazek",			"Wstaw obrazek z innego istniej±cego pliku")

	MenuLabel(AP_MENU_ID_FORMAT,			"F&ormatuj",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Czcionka",			"Change the font of the selected text")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Paragraf",		"Zmieñ format zaznaczonego paragrafu")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"podpunkty i &Numerowanie",	"Dodaj lub zmieñ wyliczenia stosowane w paragrafie")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"&Ramki& i Cienie",		"Dodaj ramkê i cieñ")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Kolumny",			"Zmieñ ilo¶æ kolumn")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"St&yle",			"Zdefinuj lub uaktualnij styl paragrafu")	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabulatory",			"Set tab stops")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Pogrubienie",			"Ustaw pogrubienie (prze³±cznik)")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Kursywa",			"Ustaw kursywê (prze³±cznik)")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Podkre¶lenie",		"Ustaw podkre¶lenie (prze³±cznik)")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"Przek&re¶lenie",			"Ustaw przekre¶lenie (prze³±cznik)")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,	"Indeks &górny",		"Indeks górny (prze³±cznik)")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"Indeks d&olny",		"Indeks dolny (prze³±cznik)")

	MenuLabel(AP_MENU_ID_TOOLS,				"&Narzêdzia",			NULL)   
	MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"&Ortografia",		"Sprawd¼ ortografiê tekstu")
	MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,	"&Zliczanie s³ów",		"Policz ile jest s³ów w dokumencie")
	MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"&Opcje",			"Ustaw opcje")

	MenuLabel(AP_MENU_ID_ALIGN,				"&Wyrównywanie",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"W &lewo",			"Wyrównaj w lewo")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"Na ¶&rodku",			"Centrowanie akapitu")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"W &prawo",			"Wyrównaj w prawo akapit")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Justowanie",			"Justify the paragraph")

	MenuLabel(AP_MENU_ID_WINDOW,			"&Okno",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&Nowe Okno",		"Otwórz nowe okno dla dokumentu")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			"Poka¿ to okno")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			"Poka¿ to okno")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			"Poka¿ to okno")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			"Poka¿ to okno")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			"Poka¿ to okno")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			"Poka¿ to okno")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			"Poka¿ to okno")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			"Poka¿ to okno")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			"Poka¿ to okno")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Wiêcej okien",	"Poka¿ ca³± listê okien")

	MenuLabel(AP_MENU_ID_HELP,				"&Pomoc",			NULL)
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&O ... %s",		"Poka¿ informacje o programie, wersji i prawach autorskich")

	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",				"Zmieñ na t± sugerowan± frazê")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",				"Zmieñ na t± sugerowan± frazê")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",				"Zmieñ na t± sugerowan± frazê")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",				"Zmieñ na t± sugerowan± frazê")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",				"Zmieñ na t± sugerowan± frazê")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",				"Zmieñ na t± sugerowan± frazê")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",				"Zmieñ na t± sugerowan± frazê")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",				"Zmieñ na t± sugerowan± frazê")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",				"Zmieñ na t± sugerowan± frazê")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"&Zignoruj wszystko", 		"Zignoruj wszystkie wyst±pienia tego wyrazu")
	MenuLabel(AP_MENU_ID_SPELL_ADD,			"&Dodaj", 			"Dodaj to s³owo do dadatkowego s³ownika")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
