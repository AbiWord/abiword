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

/* Original italian translations provided by Mauro Colorio <macolori@tin.it> */
/* Work continued by Marco Innocenti <dot0037@iperbole.bologna.it> */


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

BeginSet(ItIT,UT_TRUE)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&File",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Nuovo", 			"Crea un documento nuovo")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"&Apri",			"Apre un documento esistente")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Chiudi", 			"Chiude il documento")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Salva", 			"Salva il documento")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Sa&lva con nome", 	"Salva il documento sotto un altro nome")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"&Imposta pagina",	"Cambia le opzioni di stampa")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"Stam&pa",			"Stampa tutto o parte del documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Apre questo documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Apre questo documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Apre questo documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Apre questo documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Apre questo documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Apre questo documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Apre questo documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Apre questo documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Apre questo documento")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"&Esci", 			"Chiude tutte le finestre nell'applicazione ed esce")

	MenuLabel(AP_MENU_ID_EDIT,				"&Modifica",		NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Annulla",			"Annulla l'operazione")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Ripristina",		"Ripete l'operazione precedentemente annullata")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"Tag&lia",			"Taglia la selezione e la mette negli appunti")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Copia",			"Copia la selezione e la mette negli appunti")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"&Incolla",			"Inserisce il contenuto degli appunti")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"Ca&ncella",		"Cancella la selezione")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"S&eleziona tutto",	"Seleziona l'intero documento")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Trova",			"Cerca il testo specificato")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"Sostit&uisci",		"Sostituisce il testo specificato con un altro")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"&Vai a",			"Sposta il cursore nel punto specificato")
	MenuLabel(AP_MENU_ID_EDIT_SPELL,		"&Controllo ortografico",		"Controlla l'ortografia del documento")
	MenuLabel(AP_MENU_ID_EDIT_OPTIONS,		"&Opzioni",			"Setta le opzioni")
	
	MenuLabel(AP_MENU_ID_VIEW,				"&Visualizza",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Barre degli strumenti",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standard",			"Visualizza o nasconde la barra degli strumenti standard")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"&Formattazione",		"Visualizza o nasconde la barra degli strumenti di formattazione")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Righelli",				"Visualizza o nasconde i righelli")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"&Barra di stato",		"Visualizza o nasconde la barra di stato")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"Visualizza Para&grafi",	"Visualizza i caratteri che non vengono stampati")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Intestazione e piè di pagina",	"Edita il testo nell'intestazione e nel piè di ogni pagina")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Zoom",				"Riduce o ingrandisce il documento visualizzato")

	MenuLabel(AP_MENU_ID_INSERT,			"&Inserisci",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Interruzione",		"Inserisce un'interruzione di pagina , colonna o sessione")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"N&umeri di pagina",	"Inserisce un numero di pagina automatico")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"Da&ta e ora",			"Inserisce la data e/o l'ora")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Campo",				"Inserisce un campo di calcolo")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"&Simbolo",				"Inserisce un simobolo o un altro carattere speciale")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,	"&Figura",				"Inserisce un'immagine esistente, da un altro file")

	MenuLabel(AP_MENU_ID_FORMAT,			"F&ormato",				NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Carattere",			"Cambia il carattere del testo selezionato")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Paragrafo",			"Cambia il formato del paragrafo selezionato")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"Elenchi puntati e &Numerati",	"Aggiunge o modifica un elenco puntato o numerato alla selezione")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"Bor&di e sfondo",		"Aggiunge bordi e sfondo alla selezione")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Colonne",				"Cambia il numero delle colonne")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"St&ile",				"Definisce o applica uno stile alla selezione")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabulazione",			"Setta la tabulazione")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Grassetto",			"Mette in grassetto la selezione")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Inclinato",				"Inclina la selezione (reversibile)")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Sottolineato",		"Sottolinea la selezione (reversibile)")
	MenuLabel(AP_MENU_ID_FMT_OVERLINE,		"&Overline",		"Mette una linea sopra la selezione (reversibile)")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"&Barrato",				"Barra la selezione (reversibile)")
        MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,   "&Apice",         "Scrive in apice la selezione (reversibile)")                                                    
        MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,             "P&edice",           "Scrive in pedice la selezione (reversibile)")                                              
			
	MenuLabel(AP_MENU_ID_ALIGN,				"&Allinea",				NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Sinistra",			"Allinea a sinistra il paragrafo")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Centro",				"Allinea al centro il paragrafo")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Destra",				"Allinea a destra il paragrafo")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Giustificato",		"Giustifica il paragrafo")

	MenuLabel(AP_MENU_ID_WINDOW,			"&Finestra",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&Nuova finestra",		"Apre un'altra finestra per il documento")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",				"Attiva questa finestra")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",				"Attiva questa finestra")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",				"Attiva questa finestra")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",				"Attiva questa finestra")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",				"Attiva questa finestra")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",				"Attiva questa finestra")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",				"Attiva questa finestra")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",				"Attiva questa finestra")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",				"Attiva questa finestra")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Dispondi tutto",		"Visualizza tutta la lista delle finestre")

	MenuLabel(AP_MENU_ID_HELP,				"&?",					NULL)
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Informazioni su %s",	"Visualizza le informazioni sul programma, numero di versione e copyright")

        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,   "%s",                           "Cambia in questa parola suggerita")                                                
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,   "%s",                           "Cambia in questa parola suggerita")                                                
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,   "%s",                           "Cambia in questa parola suggerita")                                                
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,   "%s",                           "Cambia in questa parola suggerita")                                                
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,   "%s",                           "Cambia in questa parola suggerita")                                                
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,   "%s",                           "Cambia in questa parola suggerita")                                                
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,   "%s",                           "Cambia in questa parola suggerita")                                                
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,   "%s",                           "Cambia in questa parola suggerita")                                                
        MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,   "%s",                           "Cambia in questa parola suggerita")                                                
        MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,   "&Ignora Tutto",          "Ignora sempre questa parola nel documento")
        MenuLabel(AP_MENU_ID_SPELL_ADD,                 "&Aggiungi",                         "Aggiungi questa parola ad vocabolario personale")
										       

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
