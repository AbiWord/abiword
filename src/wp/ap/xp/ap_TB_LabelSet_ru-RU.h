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

BeginSetEnc(ru,RU,true,"koi8-r")

ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,			NoIcon,			NULL, NULL)

//	(id,					szLabel,		IconName,     		szToolTip, szStatusMsg)

ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Новый", 		tb_new_xpm,		NULL, "Создать новый документ")	
ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Открыть",		tb_open_xpm,		NULL, "Открыть существующий документ")
ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Сохранить", 		tb_save_xpm,		NULL, "Сохранить документ")
ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Сохранить как",	tb_save_as_xpm,		NULL, "Сохранить документ под другим имнем")
ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Печать",		tb_print_xpm,		NULL, "Печатать документ или его часть")

ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Отмена",		tb_undo_xpm,		NULL, "Отменить предыдущее действие")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Повтор",		tb_redo_xpm,		NULL, "Повторить предыдущее отмененное действие")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Вырезать",		tb_cut_xpm,		NULL, "Вырезать выделение и поместить его в буфер обмена")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Копировать",		tb_copy_xpm,		NULL, "Скопировать выделение и поместить его в буфер обмена")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Вставить",		tb_paste_xpm,		NULL, "Вставить содержимое буфера обмена в документ")

ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Стиль",		NoIcon,			NULL, "Определить или применить стили")
ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Шрифт",		NoIcon,			NULL, "Изменить шрифт текста")
ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Размер шрифта",	NoIcon,			NULL, "Изменить размер шрифта текста")
ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Жирный",		tb_text_bold_rus_xpm,	NULL, "Сделать выделение жирным")
ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Курсив",		tb_text_italic_rus_xpm,	NULL, "Сделать выделение курсивным")
ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Подчеркивание",	tb_text_underline_rus_xpm,	NULL, "Подчеркнуть выделение")
ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Надчеркивание",	tb_text_overline_rus_xpm,	NULL, "Надчеркнуть выделение")
ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Перечеркивание", 	tb_text_strikeout_rus_xpm,	NULL, "Перечеркнуть выделение")

ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Верний индекс", 	tb_text_superscript_xpm,	NULL, "Перевести в верхний индекс")
ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Нижний индекс",	tb_text_subscript_xpm,	NULL, "Перевести в нижний индекс")
ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Символ",		tb_symbol_xpm,		NULL, "Вставить символ или другой специальный знак")

ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"По левому краю",	tb_text_align_left_xpm,	NULL, "Выровнять абзац по левому краю")
ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"По центру",		tb_text_center_xpm,	NULL, "Выровнять абзац по центру")
ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"По правому краю",	tb_text_align_right_xpm,	NULL, "Выровнять абзац по правому краю")
ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"По ширине",		tb_text_justify_xpm,	NULL, "Выровнять абзац по ширине листа")

ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Отступ перед: 0",	tb_para_0before_xpm,	NULL, "Установить отступ перед абзацем в 0 пунктов")
ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"Отступ перед: 12",	tb_para_12before_xpm,	NULL, "Установить отступ перед абзацем в 12 пунктов")

ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Одиночный интервал",	tb_line_single_space_xpm,	NULL, "Установить интервал абзаца в \"Одиночный\"")
ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"Полуторный интервал",	tb_line_middle_space_xpm,	NULL, "Установить интервал абзаца в \"Полуторный\"")
ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Двойной интервал",	tb_line_double_space_xpm,	NULL, "Установить интервал абзаца в \"Двойной\"")

ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,		"1 колонка",		tb_1column_xpm,		NULL, "Разбить документ на 1 клонку")
ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,		"2 колонки",		tb_2column_xpm,		NULL, "Разбить документ на 2 клонки")
ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,		"3 колонки",		tb_3column_xpm,		NULL, "Разбить документ на 3 клонки")

ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"Масштаб",		NoIcon,			NULL, "Изменить масштаб отображения документа")
ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Ненумерованный спсок",	tb_lists_bullets_xpm,	NULL, "Начать/Завершить ненумерованный список")
ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Нумерованный список",	tb_lists_numbers_xpm,	NULL, "Начать/Завершить нумерованный список")
	
// ... add others here ...

ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,			NoIcon,			NULL, NULL)

EndSet()
