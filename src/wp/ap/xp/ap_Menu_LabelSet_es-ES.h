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

/* Spanish translations provided by Javier Yáñez <javier@cartero.com> */
/* Updated by Joaquín Cuenca Abela <cuenca@celium.net> */

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

BeginSetEnc(es,ES,true,"iso-8859-1")

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&Archivo",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Nuevo", 			"Crear un nuevo documento")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"&Abrir",			"Abrir un documento existente")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Cerrar", 			"Cerrar el documento")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Guardar",			"Guardar el documento")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Guardar &como", 	"Guardar el documento con un nombre diferente")
	MenuLabel(AP_MENU_ID_FILE_IMPORT,       "&Import",			"Importar el documento")
	MenuLabel(AP_MENU_ID_FILE_EXPORT,       "&Export",			"Guardar el documento sin cambiar el nombre actual")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"Configurar pág&ina","Cambiar las opciones de impresión")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"Im&primir",		"Imprimir todo o parte del documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Abrir este documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Abrir este documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Abrir este documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Abrir este documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Abrir este documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Abrir este documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Abrir este documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Abrir este documento")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Abrir este documento")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"S&alir", 			"Cerrar todas las ventanas y salir")

	MenuLabel(AP_MENU_ID_EDIT,				"&Edición",	NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Deshacer",		"Deshacer la edición")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Rehacer",			"Rehacer la edición deshecha previamente")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"Cor&tar",			"Cortar la selección y ponerla en el portapapeles")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Copiar",			"Copiar la selección y ponerla en el portapapeles")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"&Pegar",			"Insertar el contenido del portapapeles")
	MenuLabel(AP_MENU_ID_EDIT_PASTE_SPECIAL, "Pegar sin formato",  "Insertar el contenido del portapapeles sin formato")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"Borr&ar",			"Borrar la selección")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"Seleccionar  t&odo",	"Selecciona el documento entero")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Buscar",			"Buscar el texto indicado")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"R&eemplazar",		"Reemplazar el texto indicado con otro texto")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"&Ir a",			"Mover el punto de inserción a un lugar específico")
	
	MenuLabel(AP_MENU_ID_VIEW,				"&Ver",				NULL)
	MenuLabel(AP_MENU_ID_VIEW_NORMAL, "&Normal", "Vista Normal")
	MenuLabel(AP_MENU_ID_VIEW_WEB, "&Web Layout", "Composición de Web")
	MenuLabel(AP_MENU_ID_VIEW_PRINT, "&Print Layout", "Composición de Impresión")
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Barras de herramientas",	NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Estándar",		"Mostrar u ocultar la barra de herramientas estándar")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"&Formato",			"Mostrar u ocultar la barra de herramientas de formato")
	MenuLabel(AP_MENU_ID_VIEW_TB_EXTRA,		"&Extra",			"Mostrar u ocultar la barra de herramientas extra")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Regla",			"Mostrar u ocultar las reglas")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"&Barra de estado",	"Mostrar u ocultar la barra de estado")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"Mostrar párra&fos","Mostrar los carácteres no imprimibles")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Cabecera y pie de página",	"Edita el texto de la cabecera y el pie de cada página")
	MenuLabel(AP_MENU_ID_VIEW_FULLSCREEN, "Pantalla Completa", "Ver el documento en modo de pantalla completa")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Zoom",			"Reducir o aumentar el documento")

	MenuLabel(AP_MENU_ID_INSERT,			"&Insertar",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Salto",			"Insertar un salto de pagina, columna o sección")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"N&úmeros de página",	"Insertar un número de página secuencial")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"Fecha y &Hora",	"Insertar la fecha y/o la hora")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Campo",			"Insertar un campo calculado")
	MenuLabel(AP_MENU_ID_INSERT_PICTURE, 	"&Imagen",	 		"Insertar una imagen")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"&Símbolo",			"Insertar un símbolo u otro carácter especial")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,	"&Desde Fichero",	"Insertar una imagen desde un fichero")

	MenuLabel(AP_MENU_ID_FORMAT,			"F&ormato",			NULL)
	MenuLabel(AP_MENU_ID_FMT_LANGUAGE,		"&Lenguaje",		"Cambiar el lenguaje del texto seleccionado")
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Fuente",			"Cambiar el tipo del texto seleccionado")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Párrafo",			"Cambiar el formato del párrafo seleccionado")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"Puntos y &numeración",	"Añadir o modificar puntos y numeración en los párrafos seleccionados")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"Bor&des  y sombreado",	"Añadir bordes y sombreado a la seleccion")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Columnas",		"Cambiar el número de columnas")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"Est&ilo",			"Definir o aplicar un estilo para la selección")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabuladores",		"Definir las marcas de tabulación")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Negrita",			"Convertir a negrita la selección")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Cursiva",			"Convertir a itálica la selección")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Subrayado",		"Subrayar la selección")
	MenuLabel(AP_MENU_ID_FMT_OVERLINE,		"Su&perrayado",		"Overline the selection (toggle)")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"Tacha&do",			"Tachar la selección")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,	"Supe&ríndice",		"Convertir la selección en superíndice")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"S&ubíndice",		"Convertir la selección en subíndice")

	MenuLabel(AP_MENU_ID_TOOLS,				"&Herramientas",	NULL)   
	MenuLabel(AP_MENU_ID_TOOLS_SPELLING, 	"Corrección ortográfica", NULL)
	MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"&Corrección ortográfica",		"Verifica la ortografía del documento")
	MenuLabel(AP_MENU_ID_TOOLS_AUTOSPELL,	"Verificación automatica", "Automaticamente verifica el documento")
	MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,	"Contar &palabras", "Cuenta el número de palabras que hay en el documento")
	MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"&Opciones",		"Configurar las opciones")
	MenuLabel(AP_MENU_ID_TOOLS_LANGUAGE,	"&Lenguaje",		"Cambiar el lenguaje del texto seleccionado")

	MenuLabel(AP_MENU_ID_ALIGN,				"&Alineación",		NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Izquierda",		"Alinear el párrafo a la izquierda")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Centro",			"Alinear en el centro el párrafo")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Derecha",			"Alinear a la derecha el párrafo")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Justifica",		"Justificar el párrafo")

	MenuLabel(AP_MENU_ID_WINDOW,			"&Ventanas",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&Nueva ventana",	"Abrir una nueva ventana para el documento")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			"Ir a esta ventana")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			"Ir a esta ventana")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			"Ir a esta ventana")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			"Ir a esta ventana")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			"Ir a esta ventana")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			"Ir a esta ventana")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			"Ir a esta ventana")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			"Ir a esta ventana")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			"Ir a esta ventana")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&Más ventanas",	"Mostrar la lista completa de ventanas")

	MenuLabel(AP_MENU_ID_HELP,				"&Ayuda",			NULL)
	MenuLabel(AP_MENU_ID_HELP_CONTENTS,		"&Contenidos",		"Mostrar el contenido de la ayuda")
	MenuLabel(AP_MENU_ID_HELP_INDEX,		"&Índice",			"Mostrar el índice de la ayuda")
	MenuLabel(AP_MENU_ID_HELP_CHECKVER,		"&Versión",			"Mostrar el número de versión del programa")
	MenuLabel(AP_MENU_ID_HELP_SEARCH,		"&Buscar ayuda",	"Mostrar información acerca del programa, número de versión y copyright")
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Acerca de %s",	"Mostrar información del programa, versión y derechos de autor")
	MenuLabel(AP_MENU_ID_HELP_ABOUTOS,		"Acerca del Software &libre",	"Mostrar información acerca del software libre")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",				"Cambiar por la sugerencia")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",				"Cambiar por la sugerencia")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",				"Cambiar por la sugerencia")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",				"Cambiar por la sugerencia")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",				"Cambiar por la sugerencia")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",				"Cambiar por la sugerencia")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",				"Cambiar por la sugerencia")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",				"Cambiar por la sugerencia")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",				"Cambiar por la sugerencia")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"&Ignorar todas", 	"Ignorar todas las ocurrencias de esta palabra en el documento")
	MenuLabel(AP_MENU_ID_SPELL_ADD,			"&Añadir", 			"Añadir esta palabra al diccionario personal")

	/* autotext submenu labels */
	MenuLabel(AP_MENU_ID_AUTOTEXT_ATTN, "Atención:", "")
	MenuLabel(AP_MENU_ID_AUTOTEXT_REFERENCE, "Referencia:", "")
	MenuLabel(AP_MENU_ID_AUTOTEXT_SUBJECT, "Subjeto:", "")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
