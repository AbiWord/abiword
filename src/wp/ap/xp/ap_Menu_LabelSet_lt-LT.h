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

// Lithuanian Translation provided by Gediminas Paulauskas <menesis@delfi.lt>

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

BeginSetEnc(lt,LT,true,"iso-8859-13")

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,			"&Byla",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Nauja",		"Sukurti naujà dokumentà")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"A&tidaryti",		"Atidaryti dokumentà")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Iðsaugoti",		"Iðsaugoti esamà dokumentà")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Iðsaugoti k&aip",	"Iðsaugoti esamà dokumentà kitu vardu")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Uþdaryti",		"Uþdaryti esamà dokumentà")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,		"P&uslapio nuostatos",	"Iðdëstymo nuostatos esamo dokumento spausdinimui")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"S&pausdinti",		"Spausdinti dokumentà ar jo dalá")
	MenuLabel(AP_MENU_ID_FILE_PRINT_PREVIEW,	"Spaudinio pe&rþiûra",	"Perþiûrëti, kaip atrodys atspausdintas dokumentas")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",		"Atidaryti ðá dokumentà")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",		"Atidaryti ðá dokumentà")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",		"Atidaryti ðá dokumentà")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",		"Atidaryti ðá dokumentà")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",		"Atidaryti ðá dokumentà")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",		"Atidaryti ðá dokumentà")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",		"Atidaryti ðá dokumentà")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",		"Atidaryti ðá dokumentà")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",		"Atidaryti ðá dokumentà")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"I&ðeiti",		"Uþdaryti visus langus ir iðeiti ið programos")

	MenuLabel(AP_MENU_ID_EDIT,			"&Keisti",		NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"Atða&ukti",		"Atðaukti paskutiná atliktà veiksmà")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"Paka&rtoti",		"Pakartoti atðauktà veiksmà")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"&Iðkirpti",		"Paþymëjimà iðkirpti ir padëti á krepðá")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Kopijuoti",		"Paþymëjimà kopijuoti á krepðá")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"Á&dëti",		"Ádëti krepðio turiná")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"Ið&trinti",		"Iðtrinti paþymëjimà")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,		"Paþymëti &viskà",	"Paþymëti visà tekstà")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"R&asti",		"Ieðkoti dokumente eilutës")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"Pak&eisti",		"Pakeisti rastà eilutæ kita")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"Eiti &á",		"Perkelti þymeklá á nurodytà vietà")
	
	MenuLabel(AP_MENU_ID_VIEW,			"&Vaizdas",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"Árankiø &juostos",	NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standartinë",		"Rodyti/slëpti standartiniø árankiø juostà")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,		"&Formatavimo",		"Rodyti/slëpti teksto formatavimo árankiø juostà")
	MenuLabel(AP_MENU_ID_VIEW_TB_EXTRA,		"&Papildoma",		"Rodyti/slëpti papildomà árankiø juostà")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Liniuotë",		"Rodyti/slëpti liniuotæ")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,		"&Bûsenos juosta",	"Rodyti/slëpti bûsenos juostà")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"Rodyti þenklus",	"Rodyti nespausdinamus þenklus")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"Antraðtë ir poraðtë",	"Keisti antraðtæ ir poraðtæ")
        MenuLabel(AP_MENU_ID_VIEW_FULLSCREEN,		"&Visame ekrane", "Rodyti dokumentà viso ekrano reþimu")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Mastelis",		"Sumaþinti ar padidinti rodomà vaizdà")

	MenuLabel(AP_MENU_ID_INSERT,			"Áterpt&i",		NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Skyriklá...",		"Iterpti puslapio, skyriaus ar skilties skyriklá")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"Puslapio &numerá...",	"Áterpti savaime atsinaujinantá puslapio numerá")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,		"&Datà ir laikà...",	"Áterpti datà ir/ar laikà")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Laukà...",		"Áterpti apskaièiuojamà laukà")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"Þ&enklà...",		"Áterpti simbolá ar ypatingà þenklà")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,		"&Paveikslëlá",		"Áterpti paveikslëlá ið bylos")

	MenuLabel(AP_MENU_ID_FORMAT,			"F&ormatas",		NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Ðriftas",		"Keisti paþymëto teksto ðriftà")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Pastraipa",		"Keisti esamos pastraipos formatà")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"&Sàraðas",	"Pridëti ar pakeisti sàraðà esamoje pastraipoje")
        MenuLabel(AP_MENU_ID_FMT_DOCUMENT,              "&Dokumentas",             "Nustatyti dokumento puslapio savybes, pvz. dydá ir paraðtes")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"&Rëmeliai ir Ðeðëliai","Pridëti ar pakeisti rëmelius ar ðeðëlius esamai pastraipai")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"Skilt&ys",		"Keisti skilèiø skaièiø puslapyje")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"&Stilius",		"Nurodyti ar pritaikyti stiliø paþymëjimui")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabai",		"Nurodyti tabø sustojimus")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"St&oras",		"Pastorinti paþymëtà tekstà (perjungti)")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"Pasv&iræs",		"Pakreipti paþymëtà tekstà (perjungti)")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"Pabra&uktas",		"Pabraukti paþymëtà tekstà (perjungti)")
	MenuLabel(AP_MENU_ID_FMT_OVERLINE,		"Brûkðnys &virðuje",	"Perjungti paþymëto teksto pabraukimà virðuje")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"P&erbraukti",		"Perbraukti paþymëtà tekstà (perjungti)")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,		"Pa&keltas",		"Padaryti paþymëtà tekstà pakeltà (perjungti)")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"Nu&leistas",		"Padaryti paþymëtà tekstà nuleistà (perjungti)")

	MenuLabel(AP_MENU_ID_TOOLS,			"Á&rankiai",		NULL)   
	MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"&Raðyba",		"Patikrinti dokumento raðybos teisingumà")
	MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,		"&Statistika",		"Suskaièiuoti, kiek dokumente þodþiø, puslapiø ir kt.")
	MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"&Nuostatos",		"Nustatyti programos savybes")

	MenuLabel(AP_MENU_ID_ALIGN,			"&Lygiuoti",		NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"Pagal &kairæ",		"Lygiuoti pastraipà pagal kairájá kraðtà")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Centruoti",		"Lygiuoti eilutes pagal vidurá")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"Pagal &deðinæ",	"Lygiuoti pastraipà pagal deðinájá kraðtà")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Justify",		"Iðlyginti pastraipà pagal abu kraðtus")

	MenuLabel(AP_MENU_ID_WINDOW,			"&Langas",		NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&Naujas langas",	"Atidaryti naujà langà dokumentui")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",		"Iðkelti ðá langà")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",		"Iðkelti ðá langà")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",		"Iðkelti ðá langà")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",		"Iðkelti ðá langà")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",		"Iðkelti ðá langà")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",		"Iðkelti ðá langà")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",		"Iðkelti ðá langà")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",		"Iðkelti ðá langà")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",		"Iðkelti ðá langà")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Visi langai",		"Rodyti visø langø sàraðà")

	MenuLabel(AP_MENU_ID_HELP,			"&Pagalba",		NULL)
	MenuLabel(AP_MENU_ID_HELP_CONTENTS,		"Pagalbos &turinys",	"Parodyti pagalbos turiná")
	MenuLabel(AP_MENU_ID_HELP_INDEX,		"Pagalbos &Indeksas",	"Parodyti pagalbos indeksà")
	MenuLabel(AP_MENU_ID_HELP_CHECKVER,		"&Versija",		"Parodyti versijà")
	MenuLabel(AP_MENU_ID_HELP_SEARCH,		"&Paieðka pagalboje...",	"Ieðkoti pagalboje")
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Apie %s",		"Parodyti informacijà apie programà, versijà bei licenzijà") 
	MenuLabel(AP_MENU_ID_HELP_ABOUTOS,		"Apie &Open Source",	"Parodyti informacijà apie Open Source")

	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,		"%s",			"Pakeisti ðiuo pasiûlytu þodþiu")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,		"%s",			"Pakeisti ðiuo pasiûlytu þodþiu")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,		"%s",			"Pakeisti ðiuo pasiûlytu þodþiu")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,		"%s",			"Pakeisti ðiuo pasiûlytu þodþiu")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,		"%s",			"Pakeisti ðiuo pasiûlytu þodþiu")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,		"%s",			"Pakeisti ðiuo pasiûlytu þodþiu")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,		"%s",			"Pakeisti ðiuo pasiûlytu þodþiu")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,		"%s",			"Pakeisti ðiuo pasiûlytu þodþiu")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,		"%s",			"Pakeisti ðiuo pasiûlytu þodþiu")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,		"&Nepaisyti visø",	"Nepaisyti visø ðio þodþio pasikartojimø ðiame dokumente")
	MenuLabel(AP_MENU_ID_SPELL_ADD,			"&Pridëti", 		"Pridëti ðá þodá á nuosavà þodynà")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
