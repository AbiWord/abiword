/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

// Hungarian translations provided by Tamas Decsi <tamas.decsi@techie.com>

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

BeginSet(hu,HU,UT_TRUE)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&Fájl",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"Ú&j", 			"Új dokumentum létrehozása")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"Meg&nyitás",			"Dokumentum megnyitása")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Bezárás", 			"Dokumentum bezárása")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Mentés", 			"Dokumentum mentése")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Mentés &másként", 		"Dokumentum mentése más néven")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"&Oldalbeállítás",		"Nyomtatási beállítások")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"&Nyomtatás",			"A teljes dokumentum, vagy egy részének nyomtatása")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Ennek a dokumentumnak a megnyitása")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Ennek a dokumentumnak a megnyitása")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Ennek a dokumentumnak a megnyitása")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Ennek a dokumentumnak a megnyitása")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Ennek a dokumentumnak a megnyitása")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Ennek a dokumentumnak a megnyitása")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Ennek a dokumentumnak a megnyitása")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Ennek a dokumentumnak a megnyitása")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Ennek a dokumentumnak a megnyitása")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"&Kilépés", 			"Az alkalmazás összes ablakának bezárása és kilépés")

	MenuLabel(AP_MENU_ID_EDIT,				"&Szerkesztés",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Visszavon",			"Szerkesztés visszavonása")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"Új&ra",			"Az elõzõ visszavonás újra")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"&Kivágás",				"A kijelölt részt kivágja, és a vágólapra helyezi")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Másolás",			"A kijelölt részt a vágólapra másolja")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"&Beillesztés",			"A vágólap tartalmát beszúrja")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"&Törlés",			"A kijelölés törlése")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"Mindent kijelö&l",		"A teljes dokumentum kijelölése")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"Kere&s",			"A megadott szöveg keresése")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"Cs&erél",			"A megadott szöveg cseréje más szövegre")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"U&grás",			"A kurzor áthelyezése egy megadott helyre")
	
	MenuLabel(AP_MENU_ID_VIEW,				"&Nézet",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Eszköztárak",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Szabvány",		"A szabvány eszköztár megjelenítése vagy elrejtése")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"&Formázás",		"A formázás eszköztár megjelenítése vagy elrejtése")
	MenuLabel(AP_MENU_ID_VIEW_TB_EXTRA,		"E&xtra",			"Az extra eszköztár megjelenítése vagy elrejtése")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Vonalzó",			"A vonalzók megjelenítése vagy elrejtése")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"Állapot&sor",		"Az állapotsor megjelenítése vagy elrejtése")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"Bekezdésjelek",	"A nem nyomtatott karakterek megjelenítése")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Fejléc és lábléc",	"Az oldalak tetején és alján lévõ szöveg szerkesztése")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Nagyítás",			"A dokumentum nézet nagyítása vagy kicsinyítése")

	MenuLabel(AP_MENU_ID_INSERT,			"&Beszúrás",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"Töréspont",			"Oldal-, Hasáb-, vagy Szekciótörés beszúrása")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"&Oldalszám",	"Automatikusan frissített oldalszám beszúrása")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"&Dátum és idõ",	"Dátum és/vagy idõ beszúrása")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Mezõ",			"Számított mezõ beszúrása")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"&Szimbólum",			"Szimbólum vagy speciális karakter beszúrása")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,	"Ké&p",			"Meglévõ kép beszúrása fájlból")

	MenuLabel(AP_MENU_ID_FORMAT,			"F&ormátum",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Betû",			"A kijelölt szövegrész betûtípusának megváltoztatása")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"Be&kezdés",		"A kijelölt bekezdés formázása")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"Fel&sorolás",	"Felsorolás hozzárendelése vagy módosítása kijelölt bekezdésekhez")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"Sze&gély és árnyék",		"Szegély és árnyék hozzárendelése a kijelölt részhez")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Hasábok",			"A hasábok számának megváltoztatása")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"S&tílus",			"Stílus megadása vagy alkalmazása a kijelölt részhez")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabulátorok",			"Tabulátorok beállítása")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Félkövér",			"A kijelölt rész félkövérré tétele (megfordítás)")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Dõlt",			"A kijelölt rész dõltté tétele (megfordítás)")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Aláhúzás",		"A kijelölt rész aláhúzása (megfordítás)")
	MenuLabel(AP_MENU_ID_FMT_OVERLINE,		"Felülvonás",		"A kijelölt rész felülvonása (megfordítás)")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"Kihúzás",			"A kijelölt rész kihúzása (megfordítás)")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,	"Felsõ index",		"A kijelölt rész felsõ indexszé tétele (megfordítás)")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"Alsó index",		"A kijelölt rész alsó indexszé tétele (megfordítás)")

	MenuLabel(AP_MENU_ID_TOOLS,		"&Eszközök",		NULL)
	MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"&Helyesírás",		"A dokumentum helyesírásának ellenõrzése")
	MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,		"&Szavak száma",		"A dokumentum szavainak megszámlálása")
	MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"&Beállítások",			"Beállítások szerkesztése")

	MenuLabel(AP_MENU_ID_ALIGN,				"&Igazítás",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Balra",			"A bekezdés igazítása balra")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Középre",			"A bekezdés igazítása középre")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Jobbra",			"A bekezdés igazítása jobbra")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Sorkiegyenlítés",			"A bekezdés sorkiegyenlítése")

	MenuLabel(AP_MENU_ID_WINDOW,			"&Ablak",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"Ú&j ablak",		"Új ablak nyitása a dokumentumnak")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			"Ezt az ablakot mutassa")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			"Ezt az ablakot mutassa")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			"Ezt az ablakot mutassa")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			"Ezt az ablakot mutassa")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			"Ezt az ablakot mutassa")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			"Ezt az ablakot mutassa")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			"Ezt az ablakot mutassa")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			"Ezt az ablakot mutassa")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			"Ezt az ablakot mutassa")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&További ablakok",	"Az ablakok teljes listája")

	MenuLabel(AP_MENU_ID_HELP,				"Sú&gó",			NULL)
	MenuLabel(AP_MENU_ID_HELP_CONTENTS,		"Súgó &Tartalom",	"Súgó tartalmának megjelenítése")
	MenuLabel(AP_MENU_ID_HELP_INDEX,		"Súgó Tartalomjegyzék",		"Súgó tartalomjegyzékének megjelenítése")
	MenuLabel(AP_MENU_ID_HELP_CHECKVER,		"&Verziószám",	"A program verziószámának megjelenítése")
	MenuLabel(AP_MENU_ID_HELP_SEARCH,		"Súgó &Keresése",	"Súgó keresése...")
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"%s &Névjegy",		"Program információk, verziószám és copyright megjelenítése")
	MenuLabel(AP_MENU_ID_HELP_ABOUTOS,		"A &Szabad Forráskódról",	"Információ a szabad forráskódról")

	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",				"Cserélje erre a javaslatra")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",				"Cserélje erre a javaslatra")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",				"Cserélje erre a javaslatra")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",				"Cserélje erre a javaslatra")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",				"Cserélje erre a javaslatra")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",				"Cserélje erre a javaslatra")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",				"Cserélje erre a javaslatra")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",				"Cserélje erre a javaslatra")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",				"Cserélje erre a javaslatra")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"M&indet Kihagy", 		"Hagyja figyelmen kívül a szó dokumentumbeli összes elõfordulását")
	MenuLabel(AP_MENU_ID_SPELL_ADD,			"&Felvesz", 			"Vegye fel ezt a szót az egyedi szótárba")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
