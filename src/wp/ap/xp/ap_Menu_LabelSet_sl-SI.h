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
 
// Slovenian translation provided by Andraz Tor <andraz.tori1@guest.arnes.si>


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

BeginSetEnc(sl,SI,true,"iso-8859-2")

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&Datoteka",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Nova", 			"Ustvari nov dokument")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"&Odpri",			"Odpri obstojeè dokument")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Zapri", 			"Zapri dokument")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Shrani", 			"Shrani dokument")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Shrani &kot", 		"Shrani dokument pod drugaènim imenom")
	MenuLabel(AP_MENU_ID_FILE_IMPORT,		"&Uvozi",			"Uvozi dokument")
	MenuLabel(AP_MENU_ID_FILE_EXPORT,       	"&Izvozi",			"Izvozi dokument")

	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,		"N&astavitev strani",		"Spremeni nastaviteve tiskanja")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"Na&tisni",			"Tiskaj celoten dokument ali njegov del")
	MenuLabel(AP_MENU_ID_FILE_PRINT_DIRECTLY,		"Tiskaj &neposredno",	"Tiskaj z uporabo internega gonilnika PostScript")
	MenuLabel(AP_MENU_ID_FILE_PRINT_PREVIEW, 		"P&redogled tiskanja", "Predogled dokumenta pred tiskanjem")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Odpri ta dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Odpri ta dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Odpri ta dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Odpri ta dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Odpri ta dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Odpri ta dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Odpri ta dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Odpri ta dokument")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Odpri ta dokument")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"Iz&hod", 			"Zapri vsa okna programa in konèaj")

	MenuLabel(AP_MENU_ID_EDIT,			"&Uredi",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Razveljavi",			"Razveljavi urejanje")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Obnovi",			"Obnovi prej razveljavljeno urejanje")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"&Izre¾i",				"Izre¾i izbiro in jo daj na odlo¾i¹èe")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Kopiraj",			"Kopiraj izbiro in jo daj na odlo¾i¹èe")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"&Prilepi",			"Vstavi vsebino odlo¾i¹èa")
	MenuLabel(AP_MENU_ID_EDIT_PASTE_SPECIAL, 	"Prilepi neo&blikovano",  "Vstavi neoblikovano vsebino odlo¾i¹èa")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"Poèi&sti",			"Zbri¹i izbiro")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,		"Izberi &vse",		"Izberi celoten dokument")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"I&¹èi",			"Poi¹èi doloèeno besedilo")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"&Zamenjaj",			"Zamenjaj doloèeno besedilo z drugim")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"Poj&di na",			"Prestavi kazalec na doloèeno mesto")
	MenuLabel(AP_MENU_ID_EDIT_EDITHEADER,			"Uredi glavo",			"Uredi glavo trenutne strani")
	MenuLabel(AP_MENU_ID_EDIT_EDITFOOTER,			"Uredi nogo",			"Uredi nogo trenutne strani")
	
	MenuLabel(AP_MENU_ID_VIEW,			"Po&gled",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_NORMAL, 		"O&bièajen", "Obièajen pogled")
	MenuLabel(AP_MENU_ID_VIEW_WEB, 			"&Spletni pogled", "Spletni pogled")
	MenuLabel(AP_MENU_ID_VIEW_PRINT, 		"Pogled &tiskanja", "Tiskalni¹ki pogled")
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Orodjarne",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,			"O&bièajne",		"Ka¾i/skrij obièajno orodjarno")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,			"&Oblikovanje",		"Ka¾i/skrij oblikovalsko orodjarno")
	MenuLabel(AP_MENU_ID_VIEW_TB_EXTRA,			"&Dodatne",		"Ka¾i/skrij dodatno orodjarno")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Ravnilo",			"Ka¾i/skrij ravnila")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,		"Vrstica s&tanja",		"Ka¾i/skrij vrstico stanja")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"Ka¾i o&dstavke",	"Ka¾i znake, ki oznaèujejo oblikovanje")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Glava in noga",	"Uredi besedilo na vrhu ali dnu vsake strani")
	MenuLabel(AP_MENU_ID_VIEW_FULLSCREEN, 		"Èez &cel zaslon", "Glej dokument èez cel zason")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Poveèava",		"Poveèaj ali zmanj¹aj prikaz dokumenta")

	MenuLabel(AP_MENU_ID_INSERT,			"&Vstavi",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Prelom",			"Vstavi prelom strani, stolpca ali odseka")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"©t&evilke strani",	"Vstavi ¹tevilke strani")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,		"Datum in &uro",	"Vstavi datum in/ali uro")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"Po&lje",		"Vstavi preraèunano polje")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"Si&mbol",		"Vstavi simbol ali kak drug posebni znak")
	MenuLabel(AP_MENU_ID_INSERT_PICTURE, 		"&Sliko", 	"Vstavi sliko")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,			"&Iz datoteke",	"Vstavi obstojeèo sliko iz druge datoteke")

	MenuLabel(AP_MENU_ID_FORMAT,			"&Oblikovanje",			NULL)
	MenuLabel(AP_MENU_ID_FMT_LANGUAGE,		"&Jezik",		"Spremeni jezik izbranega besedila")
	MenuLabel(AP_MENU_ID_FMT_FONT,			"P&isava",			"Spremeni pisavo izbranega besedila")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"Odst&avek",		"Spremeni obliko izbranega odstavka")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"O&znaèevanje in o¹tevilèenje",	"Dodaj ali spremeni oznaèevanje in o¹tevilèenje izbranega odstavka")
	MenuLabel(AP_MENU_ID_FMT_DOCUMENT,		"&Dokument",             "Nastavi lastnosti strani dokumenta kot so velikost strani in robovi")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"Okvirji in s&enèenja",		"Izbiri dodaj robove in senèenja")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Stolpci",			"Spremeni ¹tevilo stolpcev")
	MenuLabel(AP_MENU_ID_FMT_TOGGLECASE,   	"Spremeni &velikost", "Spremeni velikost izbranega besedila")
	MenuLabel(AP_MENU_ID_FMT_BACKGROUND,   	"O&zadnje", "Spremeni barvo ozadja va¹ega besedila")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"S&log",			"Doloèi ali uveljavi slog na izbiri")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabulatorji",			"Nastavi tabulatorska mesta")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Krepko",			"Naredi izbiro krepko (preklopi)")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"K&urzivno",			"Naredi izbiro kurzivno (preklopi)")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Podèrtano",		"Naredi izbiro podèrtano (preklopi)")
	MenuLabel(AP_MENU_ID_FMT_OVERLINE,		"Nad&èrtano",		"Naredi izbiro nadèrtano (preklopi)")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"P&reèrtano",			"Preèrtaj izbiro (preklopi)")
	MenuLabel(AP_MENU_ID_FMT_TOPLINE,		"Èrta z&goraj",			"Èrta nad izbiro (preklopi)")
	MenuLabel(AP_MENU_ID_FMT_BOTTOMLINE,	"Èrta spoda&j",		"Èrta pod izbiro (preklopi)")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,	"&Nadpisano",		"Naredi izbiro nadpisano (preklopi)")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"P&odpisano",		"Naredi izbiro podpisano (preklopi)")

	MenuLabel(AP_MENU_ID_TOOLS,			"&Orodja",			NULL)   
	MenuLabel(AP_MENU_ID_TOOLS_SPELLING, "È&rkovalnik", NULL)
	MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"Preveri è&rkovanje",		"Preveri dokument za napaènim èrkovanjem")
	MenuLabel(AP_MENU_ID_TOOLS_AUTOSPELL, "&Samodejno èrkovanje", "Samodejno preverjaj èrkovanje v dokumentu")
	MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,	"©tetje &besed",		"©tej ¹tevilo besed v dokumentu")
	MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"&Nastavitve",		"Nastavi nastavitve")
	MenuLabel(AP_MENU_ID_TOOLS_LANGUAGE, "&Jezik", 				"Spremeni jezik izbranega besedila")
	MenuLabel(AP_MENU_ID_TOOLS_PLUGINS, "&Vtièniki",			"Upravljaj z vtièniki")
	MenuLabel(AP_MENU_ID_TOOLS_SCRIPTS, "S&kripte",				"Izvedi pomo¾ne skripte")

	MenuLabel(AP_MENU_ID_ALIGN,				"&Poravnava",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Leva",			"Left-justify the paragraph")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Sredi¹èna",			"Center-align the paragraph")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Desna",			"Right-justify the paragraph")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Obojestranska",			"Justify the paragraph")

	MenuLabel(AP_MENU_ID_WEB, 			"&Splet", NULL)
	MenuLabel(AP_MENU_ID_WEB_SAVEASWEB,    "Shrani kot &splet",		"Save the document as a web page")
	MenuLabel(AP_MENU_ID_WEB_WEBPREVIEW,   "P&redogled sletne strani", "Preview the document as a web page")

	MenuLabel(AP_MENU_ID_WINDOW,			"&Okno",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&Novo okno",		"Odpri ¹e eno okno")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			"Dvigni to okno")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			"Dvigni to okno")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			"Dvigni to okno")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			"Dvigni to okno")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			"Dvigni to okno")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			"Dvigni to okno")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			"Dvigni to okno")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			"Dvigni to okno")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			"Dvigni to okno")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Veè oken",	"Ka¾i celoten seznam oken")

	MenuLabel(AP_MENU_ID_HELP,			"&Pomoè",		NULL)
	MenuLabel(AP_MENU_ID_HELP_CONTENTS,		"&Vsebina pomoèi",	"Ka¾i vsebino pomoèi")
	MenuLabel(AP_MENU_ID_HELP_INDEX,		"&Kazalo pomoèi",		"Ka¾i kazalo pomoèi")
	MenuLabel(AP_MENU_ID_HELP_CHECKVER,		"Ka¾i &razlièico",	"Poka¾i razlièico programa")
	MenuLabel(AP_MENU_ID_HELP_SEARCH,		"&I¹èi po pomoèi",	"I¹èi po pomoèi za...")
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&O %s",		"Poka¾i razlièico in licenco in podatke o programu") 
	MenuLabel(AP_MENU_ID_HELP_ABOUTOS,		"O O&dprtem programju",	"Poka¾i podatke o odprtem programju")

	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",				"Spremeni na to predlagano èrkovanje")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",				"Spremeni na to predlagano èrkovanje")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",				"Spremeni na to predlagano èrkovanje")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",				"Spremeni na to predlagano èrkovanje")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",				"Spremeni na to predlagano èrkovanje")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",				"Spremeni na to predlagano èrkovanje")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",				"Spremeni na to predlagano èrkovanje")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",				"Spremeni na to predlagano èrkovanje")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",				"Spremeni na to predlagano èrkovanje")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"&Prezri vse", 		"Prezri vse pojavitve te besede v dokumentu")
	MenuLabel(AP_MENU_ID_SPELL_ADD,			"&Dodaj", 			"To besedo dodaj v prikrojen slovar")

	/* autotext submenu labels */
	MenuLabel(AP_MENU_ID_INSERT_AUTOTEXT, "&Samodejno besedilo", "")
	MenuLabel(AP_MENU_ID_AUTOTEXT_ATTN, "Opozorilo:", "")
	MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING, "Zakljuèek:", "") 
	MenuLabel(AP_MENU_ID_AUTOTEXT_MAIL, "Po¹tna navodila:", "")
	MenuLabel(AP_MENU_ID_AUTOTEXT_REFERENCE, "Reference:", "")
	MenuLabel(AP_MENU_ID_AUTOTEXT_SALUTATION, "Pozdrav:", "")
	MenuLabel(AP_MENU_ID_AUTOTEXT_SUBJECT, "Zadeva:", "")
	MenuLabel(AP_MENU_ID_AUTOTEXT_EMAIL, "E-po¹ta:", "")

	MenuLabel(AP_MENU_ID_AUTOTEXT_ATTN_1, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_ATTN_2, "%s", " ")
	
	MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING_1, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING_2, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING_3, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING_4, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING_5, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING_6, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING_7, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING_8, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING_9, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING_10, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING_11, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_CLOSING_12, "%s", " ")
	
	MenuLabel(AP_MENU_ID_AUTOTEXT_MAIL_1, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_MAIL_2, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_MAIL_3, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_MAIL_4, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_MAIL_5, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_MAIL_6, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_MAIL_7, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_MAIL_8, "%s", " ")
	
	MenuLabel(AP_MENU_ID_AUTOTEXT_REFERENCE_1, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_REFERENCE_2, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_REFERENCE_3, "%s", " ")
	
	MenuLabel(AP_MENU_ID_AUTOTEXT_SALUTATION_1, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_SALUTATION_2, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_SALUTATION_3, "%s", " ")

	MenuLabel(AP_MENU_ID_AUTOTEXT_SALUTATION_4, "%s", " ")

	MenuLabel(AP_MENU_ID_AUTOTEXT_SUBJECT_1, "%s", " ")

	MenuLabel(AP_MENU_ID_AUTOTEXT_EMAIL_1, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_EMAIL_2, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_EMAIL_3, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_EMAIL_4, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_EMAIL_5, "%s", " ")
	MenuLabel(AP_MENU_ID_AUTOTEXT_EMAIL_6, "%s", " ")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
