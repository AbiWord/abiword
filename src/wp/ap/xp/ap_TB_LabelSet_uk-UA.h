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

// Note: if the tooltip is blank, the status message will be used as the
// Note: tooltip.  therefore, we probably don't need most tooltip strings
// Note: here -- unless the status message is too long to look good in
// Note: a tooltip.

// Note: the icon field should not be localized unless absolutely necessary.
// Note: the icon name here is to a specific icon (pixmap or bitmap or whatever)
// Note: that will always be in the application.  if, for example, a big fat 'B'
// Note: for BOLD doesn't make sense in another language, change the entry in
// Note: the localization and add the icon to whatever table.

// Note: if a tool item does not use an icon (like a combo box), use the
// Note: constant "NoIcon" in that column.

// If the third argument is true, then this is the fall-back for
// this language (named in the first argument).

BeginSetEnc(uk,UA,true,"cp1251")

ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,			NoIcon,			NULL, NULL)

//	(id,					szLabel,		IconName,     		szToolTip, szStatusMsg)

ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Новий", 		tb_new_xpm,		NULL, "Створити новий документ")	
ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Відкрити",		tb_open_xpm,		NULL, "Відкрити існуючий документ")
ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Зберегти", 		tb_save_xpm,		NULL, "Зберегти документ")
ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Зберегти як",		tb_save_as_xpm,		NULL, "Зберегти документ під іншим им`ям")
ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Друк",			tb_print_xpm,		NULL, "Друкувати документ або його частину")

ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Скасування",		tb_undo_xpm,		NULL, "Відмінити попередню дію")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Повтор",		tb_redo_xpm,		NULL, "Повторити попередню відмінену дію")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Вирізати",		tb_cut_xpm,		NULL, "Вырізати виділення і вмістити його в буфер обміну")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Копиювати",		tb_copy_xpm,		NULL, "Скопіювати виділення і вмістити його в буфер обміну")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Вставити",		tb_paste_xpm,		NULL, "Вставити вміст буфера обміна в документ")

ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Стиль",		NoIcon,			NULL, "Визначити або застосувати стилі")
ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Шрифт",		NoIcon,			NULL, "Змінити шрифт тексту")
ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Розмір шрифта",	NoIcon,			NULL, "Змінити размір шрифту текста")
ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Жирный",		tb_text_bold_rus_xpm,	NULL, "Зробити виділення жирным")
ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Курсив",		tb_text_italic_rus_xpm,	NULL, "Зробити виділення курсивным")
ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Підкреслення",		tb_text_underline_rus_xpm,	NULL, "Підкреслити виділення")
ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Надкреслення",		tb_text_overline_rus_xpm,	NULL, "Надкреслити виділення")
ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Перекреслення", 	tb_text_strikeout_rus_xpm,	NULL, "Перекреслити виділення")

ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Верхній індекс", 	tb_text_superscript_xpm,	NULL, "Перевести у верхній індекс")
ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Нижній індекс",	tb_text_subscript_xpm,	NULL, "Перевести в нижній індекс")
ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Символ",		tb_symbol_xpm,		NULL, "Вставити символ або інший спеціальний знак")

ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"По лівому краю",	tb_text_align_left_xpm,	NULL, "Вирівняти абзац по лівому краю")
ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"По центру",		tb_text_center_xpm,	NULL, "Вирівняти абзац по центру")
ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"По правому краю",	tb_text_align_right_xpm,	NULL, "Вирівняти абзац по правому краю")
ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"По ширині",		tb_text_justify_xpm,	NULL, "Вирівняти абзац по ширині листа")

ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Відступ перед: 0",	tb_para_0before_xpm,	NULL, "Встановити відступ перед абзацем в 0 пунктів")
ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"Відступ перед: 12",	tb_para_12before_xpm,	NULL, "Встановити відступ перед абзацем в 12 пунктів")

ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Одиночный інтервал",	tb_line_single_space_xpm,	NULL, "Встановити інтервал абзацу в \"Одиночній\"")
ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"Полуторный інтервал",	tb_line_middle_space_xpm,	NULL, "Встановити інтервал абзацу в \"Полуторній\"")
ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Подвійний інтервал",	tb_line_double_space_xpm,	NULL, "Встановити інтервал абзацу в \"Подвійний\"")

ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,		"1 колонка",		tb_1column_xpm,		NULL, "Разбити документ на 1 клонку")
ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,		"2 колонки",		tb_2column_xpm,		NULL, "Разбити документ на 2 клонки")
ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,		"3 колонки",		tb_3column_xpm,		NULL, "Разбити документ на 3 клонки")

ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"Масштаб",		NoIcon,			NULL, "Змінити масштаб відображення документа")
ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Ненумерований список",	tb_lists_bullets_xpm,	NULL, "Почати/Завершити ненумерований список")
ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Нумерований список",	tb_lists_numbers_xpm,	NULL, "Почати/Завершити нумерований список")
	
// ... add others here ...

ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,			NoIcon,			NULL, NULL)

EndSet()
