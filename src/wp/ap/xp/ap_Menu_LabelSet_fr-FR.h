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

/* French translations provided by Philippe Duperron <duperron@mail.dotcom.fr> */
/* modifications by Gilles Saint-Denis <stdenisg@cedep.net> and 
   Christopher Caron <ChrisDigo@aol.com> 23/10/2000 */

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

BeginSet(fr,FR,UT_TRUE)

    MenuLabel(AP_MENU_ID__BOGUS1__,         NULL,               NULL)

    //       (id,                           szLabel,            szStatusMsg)

    MenuLabel(AP_MENU_ID_FILE,              "&Fichier",         NULL)
    MenuLabel(AP_MENU_ID_FILE_NEW,          "&Nouveau",         "Crée un nouveau document")    
    MenuLabel(AP_MENU_ID_FILE_OPEN,         "&Ouvrir",          "Ouvre un document existant")
    MenuLabel(AP_MENU_ID_FILE_CLOSE,        "&Fermer",          "Ferme le document actif")
    MenuLabel(AP_MENU_ID_FILE_SAVE,         "&Enregistrer",     "Enregistre le document actif")
    MenuLabel(AP_MENU_ID_FILE_SAVEAS,       "En&registrer sous",    "Enregistre le document actif sous un nouveau nom")
    MenuLabel(AP_MENU_ID_FILE_PAGESETUP,    "Mise en &page",    "Modifie la mise en page du document")
    MenuLabel(AP_MENU_ID_FILE_PRINT,        "&Imprimer",        "Imprime tout ou partie du document")
    MenuLabel(AP_MENU_ID_FILE_RECENT_1,     "&1 %s",            "Ouvre ce document")
    MenuLabel(AP_MENU_ID_FILE_RECENT_2,     "&2 %s",            "Ouvre ce document")
    MenuLabel(AP_MENU_ID_FILE_RECENT_3,     "&3 %s",            "Ouvre ce document")
    MenuLabel(AP_MENU_ID_FILE_RECENT_4,     "&4 %s",            "Ouvre ce document")
    MenuLabel(AP_MENU_ID_FILE_RECENT_5,     "&5 %s",            "Ouvre ce document")
    MenuLabel(AP_MENU_ID_FILE_RECENT_6,     "&6 %s",            "Ouvre ce document")
    MenuLabel(AP_MENU_ID_FILE_RECENT_7,     "&7 %s",            "Ouvre ce document")
    MenuLabel(AP_MENU_ID_FILE_RECENT_8,     "&8 %s",            "Ouvre ce document")
    MenuLabel(AP_MENU_ID_FILE_RECENT_9,     "&9 %s",            "Ouvre ce document")
    MenuLabel(AP_MENU_ID_FILE_EXIT,         "&Quitter",         "Ferme toutes les fenêtres et quitte l'application")

    MenuLabel(AP_MENU_ID_EDIT,              "&Edition",         NULL)
    MenuLabel(AP_MENU_ID_EDIT_UNDO,         "&Annuler",         "Annule la dernière action")
    MenuLabel(AP_MENU_ID_EDIT_REDO,         "&Répéter",         "Refait l'action précédemment annulée")
    MenuLabel(AP_MENU_ID_EDIT_CUT,          "&Couper",          "Efface la sélection et la place dans le presse-papiers")
    MenuLabel(AP_MENU_ID_EDIT_COPY,         "Co&pier",          "Copie la sélection et la place dans le presse-papiers")
    MenuLabel(AP_MENU_ID_EDIT_PASTE,        "C&oller",          "Insère le contenu du presse-papiers au point d'insertion")
    MenuLabel(AP_MENU_ID_EDIT_CLEAR,        "E&ffacer",         "Efface la selection")
    MenuLabel(AP_MENU_ID_EDIT_SELECTALL,    "Selectio&nner tout",    "Selectionne le document entier")
    MenuLabel(AP_MENU_ID_EDIT_FIND,         "Rec&hercher",      "Recherche le texte spécifié")
    MenuLabel(AP_MENU_ID_EDIT_REPLACE,      "R&emplacer",       "Recherche le teste spécifié et le remplace par un texte différent")
    MenuLabel(AP_MENU_ID_EDIT_GOTO,         "A&tteindre",       "Déplace le point d'insertion à l'endroit spécifié")
    
    MenuLabel(AP_MENU_ID_VIEW,              "&Affichage",       NULL)
    MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,     "&Barre d'outils",  NULL)
    MenuLabel(AP_MENU_ID_VIEW_TB_STD,       "&Standard",        "Affiche ou masque la barre d'outils standard")
    MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,    "&Mise en forme",   "Affiche ou masque la barre d'outils de mise en forme")
	MenuLabel(AP_MENU_ID_VIEW_TB_EXTRA,		"&Extra",			"Affiche ou masque la barre d'outils extra")
    MenuLabel(AP_MENU_ID_VIEW_RULER,        "&Règles",          "Affiche ou masque les règles")
    MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,    "Barre d'é&tat",    "Affiche ou masque la barre d'état")
    MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,     "&Afficher/Masquer ¶",          "Affiche ou masque les caractères non-imprimables")
    MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,     "&Entêtes et pieds de page",    "Edite le texte de l'entête et du pied de page")
    MenuLabel(AP_MENU_ID_VIEW_ZOOM,         "&Zoom",            "Règle le facteur d'agrandissement de l'affichage")

    MenuLabel(AP_MENU_ID_INSERT,            "&Insertion",      NULL)
    MenuLabel(AP_MENU_ID_INSERT_BREAK,      "&Saut",           "Insère un saut de page, de colonne ou de section")
    MenuLabel(AP_MENU_ID_INSERT_PAGENO,     "N&uméros de page",         "Insère un numéro de page mis à jour automatiquement")
    MenuLabel(AP_MENU_ID_INSERT_DATETIME,   "Date et &heure",  "Insère la date et/ou l'heure")
    MenuLabel(AP_MENU_ID_INSERT_FIELD,      "&Champ",          "Insère un champ calculé")
    MenuLabel(AP_MENU_ID_INSERT_SYMBOL,     "Caractères s&péciaux",    "Insère un symbole ou un caractère spécial")
    MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,    "Ima&ge",          "Insère une image existante à partir d'un autre fichier")

    MenuLabel(AP_MENU_ID_FORMAT,            "F&ormat",         NULL)
    MenuLabel(AP_MENU_ID_FMT_FONT,          "&Police",         "Change la police du texte sélectionné")
    MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,     "Pa&ragraphe",     "Change le format du paragraphe sélectionné")
    MenuLabel(AP_MENU_ID_FMT_BULLETS,       "P&uces et numéros",     "Ajoute ou modifie les puces et numéros du paragraphe sélectionné")
    MenuLabel(AP_MENU_ID_FMT_BORDERS,       "Bor&dures et trames",   "Ajoute des bordures et une trame à la sélection")
    MenuLabel(AP_MENU_ID_FMT_COLUMNS,       "C&olonnes",       "Change le nombre de colonnes")
    MenuLabel(AP_MENU_ID_FMT_STYLE,         "&Style",          "Definit ou applique un style à la sélection")
    MenuLabel(AP_MENU_ID_FMT_TABS,          "&Tabulations",    "Definit les tabulations")
    MenuLabel(AP_MENU_ID_FMT_BOLD,          "&Gras",           "Met la sélection en gras (bascule)")
    MenuLabel(AP_MENU_ID_FMT_ITALIC,        "&Italique",       "Met la sélection en italique (bascule)")
    MenuLabel(AP_MENU_ID_FMT_UNDERLINE,     "Sou&ligné",       "Souligne la sélection (bascule)")
    MenuLabel(AP_MENU_ID_FMT_OVERLINE,      "Barré &haut",     "Barre la sélection en haut (bascule)")
    MenuLabel(AP_MENU_ID_FMT_STRIKE,        "&Barré",          "Barre la sélection (bascule)")
    MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,   "&Exposant",       "Met la sélection en exposant (bascule)")
    MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,     "I&ndice",         "Met la sélection en indice (bascule)")

	MenuLabel(AP_MENU_ID_TOOLS,				"O&utils",			NULL)   
    MenuLabel(AP_MENU_ID_TOOLS_SPELL,       "O&rthographe",		"Vérifie l'orthographe du document")
	MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,	"&Statistiques",	"Statistiques du document")
    MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,     "O&ptions",         "Configure les différentes options")
    
    MenuLabel(AP_MENU_ID_ALIGN,             "&Alignement",     NULL)
    MenuLabel(AP_MENU_ID_ALIGN_LEFT,        "à &Gauche",       "Aligne le paragraphe à gauche")
    MenuLabel(AP_MENU_ID_ALIGN_CENTER,      "&Centré",         "Centre le paragraphe")
    MenuLabel(AP_MENU_ID_ALIGN_RIGHT,       "à &Droite",       "Aligne le paragraphe à droite")
    MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,     "&Justifié",       "Justifie le paragraphe à droite et à gauche")

    MenuLabel(AP_MENU_ID_WINDOW,            "Fe&nêtre",        NULL)
    MenuLabel(AP_MENU_ID_WINDOW_NEW,        "&Nouvelle fenêtre",    "Ouvre une nouvelle fenêtre sur le document actif")
    MenuLabel(AP_MENU_ID_WINDOW_1,          "&1 %s",           "Passe à la fenêtre contenant ce document")
    MenuLabel(AP_MENU_ID_WINDOW_2,          "&2 %s",           "Passe à la fenêtre contenant ce document")
    MenuLabel(AP_MENU_ID_WINDOW_3,          "&3 %s",           "Passe à la fenêtre contenant ce document")
    MenuLabel(AP_MENU_ID_WINDOW_4,          "&4 %s",           "Passe à la fenêtre contenant ce document")
    MenuLabel(AP_MENU_ID_WINDOW_5,          "&5 %s",           "Passe à la fenêtre contenant ce document")
    MenuLabel(AP_MENU_ID_WINDOW_6,          "&6 %s",           "Passe à la fenêtre contenant ce document")
    MenuLabel(AP_MENU_ID_WINDOW_7,          "&7 %s",           "Passe à la fenêtre contenant ce document")
    MenuLabel(AP_MENU_ID_WINDOW_8,          "&8 %s",           "Passe à la fenêtre contenant ce document")
    MenuLabel(AP_MENU_ID_WINDOW_9,          "&9 %s",           "Passe à la fenêtre contenant ce document")
    MenuLabel(AP_MENU_ID_WINDOW_MORE,       "&Plus de fenêtres",    "Montre la liste complète des fenêtres")

    MenuLabel(AP_MENU_ID_HELP,              "&Aide",            NULL)
	MenuLabel(AP_MENU_ID_HELP_CONTENTS,		"&Sommaire",		"Affiche le sommaire de l'aide")
	MenuLabel(AP_MENU_ID_HELP_INDEX,		"&Index de l'aide",		"Affiche l'index de l'aide")
	MenuLabel(AP_MENU_ID_HELP_CHECKVER,		"&Vérifier la version",	"Affiche le numéro de version du programme")
	MenuLabel(AP_MENU_ID_HELP_SEARCH,		"&Rechercher dans l'aide",	"Recherche de l'aide à propos de...")
    MenuLabel(AP_MENU_ID_HELP_ABOUT,        "&A propos de %s",  "Affiche des informations sur le programme, le numéro de version et le copyright")
	MenuLabel(AP_MENU_ID_HELP_ABOUTOS,		"A propos de l'&Open Source",	"Affiche des informations sur le concept Open Source")

	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",				"Change pour le texte suggéré")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",				"Change pour le texte suggéré")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",				"Change pour le texte suggéré")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",				"Change pour le texte suggéré")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",				"Change pour le texte suggéré")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",				"Change pour le texte suggéré")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",				"Change pour le texte suggéré")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",				"Change pour le texte suggéré")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",				"Change pour le texte suggéré")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"&Ignorer tout", 	"Ignore le mot chaque fois qu'on le rencontre dans le document")
	MenuLabel(AP_MENU_ID_SPELL_ADD,			"&Ajouter", 		"Ajouter ce mot au dictionnaire personnel")

    // ... add others here ...

    MenuLabel(AP_MENU_ID__BOGUS2__,         NULL,               NULL)

EndSet()
