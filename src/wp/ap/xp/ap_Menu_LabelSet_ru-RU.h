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

// If the third argument is true, then this is the fall-back for
// this language (named in the first argument).

BeginSetEnc(ru,RU,true,"koi8-r")

MenuLabel(AP_MENU_ID__BOGUS1__,		NULL,			NULL)

//       (id,				szLabel,           	szStatusMsg)

MenuLabel(AP_MENU_ID_FILE,		"&Файл",		NULL)
 MenuLabel(AP_MENU_ID_FILE_NEW,		"&Новый", 		"Создать новый документ")	
 MenuLabel(AP_MENU_ID_FILE_OPEN,	"&Открыть",		"Открыть существующий документ")
 MenuLabel(AP_MENU_ID_FILE_SAVE,	"&Сохранить", 		"Сохранить документ")
 MenuLabel(AP_MENU_ID_FILE_SAVEAS,	"Сохранить &как", 	"Сохранить документ под другим именем")
 MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"П&араметры страницы",	"Изменить параметры печати")
 MenuLabel(AP_MENU_ID_FILE_PRINT,	"&Печать",		"Печатать документ или его часть")
 MenuLabel(AP_MENU_ID_FILE_RECENT_1,	"&1 %s",		"Открыть этот документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_2,	"&2 %s",		"Открыть этот документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_3,	"&3 %s",		"Открыть этот документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_4,	"&4 %s",		"Открыть этот документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_5,	"&5 %s",		"Открыть этот документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_6,	"&6 %s",		"Открыть этот документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_7,	"&7 %s",		"Открыть этот документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_8,	"&8 %s",		"Открыть этот документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_9,	"&9 %s",		"Открыть этот документ")
 MenuLabel(AP_MENU_ID_FILE_CLOSE,	"&Закрыть", 		"Закрыть документ")
 MenuLabel(AP_MENU_ID_FILE_EXIT,	"&Выход", 		"Закрыть все документы и выйти")
    
MenuLabel(AP_MENU_ID_EDIT,		"&Редактирование",	NULL)
 MenuLabel(AP_MENU_ID_EDIT_UNDO,	"&Отмена",		"Отменить предыдущее действие")
 MenuLabel(AP_MENU_ID_EDIT_REDO,	"&Повтор",		"Повторить предыдущее отмененное действие")
 MenuLabel(AP_MENU_ID_EDIT_CUT,		"&Вырезать",		"Вырезать выделение и поместить его в буфер обмена")
 MenuLabel(AP_MENU_ID_EDIT_COPY,	"&Копировать",		"Скопировать выделение и поместить его в буфер обмена")
 MenuLabel(AP_MENU_ID_EDIT_PASTE,	"В&ставить",		"Вставить содержимое буфера обмена в документ")
 MenuLabel(AP_MENU_ID_EDIT_CLEAR,	"О&чистить",		"Очистить выделение")
 MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"В&ыделить все",	"Выделить весь документ")
 MenuLabel(AP_MENU_ID_EDIT_FIND,	"&Найти",		"Найти заданный текст")
 MenuLabel(AP_MENU_ID_EDIT_REPLACE,	"&Заменить",		"Заменить заданный текст другим")
 MenuLabel(AP_MENU_ID_EDIT_GOTO,	"Пе&рейти к",		"Переместить курсор в заданное место")

MenuLabel(AP_MENU_ID_VIEW,		"&Вид",			NULL)
 MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,	"&Панели",		NULL)
  MenuLabel(AP_MENU_ID_VIEW_TB_STD,	"&Стандартная",		"Показать/спрятать стандартную панель")
  MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"&Форматирование",	"Показать/спрятать панель форматирования")
  MenuLabel(AP_MENU_ID_VIEW_TB_EXTRA,	"&Дополнительная",	"Показать/спрятать дополнительную панель")
 MenuLabel(AP_MENU_ID_VIEW_RULER,	"&Линейка",		"Показать/спрятать линейку")
 MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"&Строка статуса",	"Показать/спрятать строку статуса")
 MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,	"П&оказать спецсимволы",	"Показать/спрятать непечатаемые символы")
 MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,	"&Верхний и нижний колонтитулы",	"Редактировать текст внизу и вверху каждой странице")
 MenuLabel(AP_MENU_ID_VIEW_ZOOM,	"&Масштаб",		"Уменьшить/увеличить масштаб представления документа")

MenuLabel(AP_MENU_ID_INSERT,		"В&ставка",		NULL)
 MenuLabel(AP_MENU_ID_INSERT_BREAK,	"&Разрыв",		"Вставить разрыв страницы, колонки или секции")
 MenuLabel(AP_MENU_ID_INSERT_PAGENO,	"&Номера страниц",	"Вставить автообновляемые номера страниц")
 MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"&Дата и время",	"Вставить дату/время")
 MenuLabel(AP_MENU_ID_INSERT_FIELD,	"&Поле",		"Вставить автообновляемое поле")
 MenuLabel(AP_MENU_ID_INSERT_SYMBOL,	"&Символ",		"Вставить символ или другой специальный знак")
 MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,	"Р&исунок",		"Вставить существующий рисунок из внешнего файла")

MenuLabel(AP_MENU_ID_FORMAT,		"Ф&орматирование",	NULL)
 MenuLabel(AP_MENU_ID_FMT_FONT,		"&Шрифт",		"Изменить шрифт выделенного текста")
 MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,	"&Абзац",		"Изменить формат выделенного абзаца")
 MenuLabel(AP_MENU_ID_FMT_BULLETS,	"&Списки",		"Добавить/убрать маркеры/нумерацию у абзацев")
 MenuLabel(AP_MENU_ID_FMT_BORDERS,	"&Границы и заливка",	"Добавить/убрать рамку и заливку у выделения")
 MenuLabel(AP_MENU_ID_FMT_COLUMNS,	"К&олонки",		"Изменить кол-во колонок")
 MenuLabel(AP_MENU_ID_FMT_TABS,		"&Табуляция",		"Установить позиции табуляции")
 MenuLabel(AP_MENU_ID_FMT_BOLD,		"&Жирный",		"Сделать выделение жирным")
 MenuLabel(AP_MENU_ID_FMT_ITALIC,	"&Курсив",		"Сделать выделение курсивным")
 MenuLabel(AP_MENU_ID_FMT_UNDERLINE,	"&Подчеркивание",	"Подчеркнуть выделение")
 MenuLabel(AP_MENU_ID_FMT_OVERLINE,	"Над&черкивание",	"Надчеркнуть выделение")
 MenuLabel(AP_MENU_ID_FMT_STRIKE,	"П&еречеркивание",	"Перечеркнуть выделение")
 MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,	"&Верний индекс",	"Перевести выделение в верхний индекс")
 MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,	"&Нижний индекс",	"Перевести выделение в нижний индекс")
 MenuLabel(AP_MENU_ID_ALIGN,		"В&ыравнивание",	NULL)
  MenuLabel(AP_MENU_ID_ALIGN_LEFT,	"По &левому краю",	"Выровнять абзац по левому краю")
  MenuLabel(AP_MENU_ID_ALIGN_CENTER,	"По &центру",		"Выровнять абзац по центру")
  MenuLabel(AP_MENU_ID_ALIGN_RIGHT,	"По &правому краю",	"Выровнять абзац по правому краю")
  MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,	"По &ширине",		"Выровнять абзац по ширине листа")
 MenuLabel(AP_MENU_ID_FMT_STYLE,	"Ст&иль",		"Определить или применить стили к выделению")

MenuLabel(AP_MENU_ID_TOOLS,		"&Сервис",		NULL)   
 MenuLabel(AP_MENU_ID_TOOLS_SPELL,	"&Правописание",	"Проверить документ на правописание")
 MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,	"&Статистика",		"Посчитать кол-во слов, символов и т. д. в документе")
 MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,	"&Установки",		"Изменить установки")

MenuLabel(AP_MENU_ID_WINDOW,		"&Окно",		NULL)
 MenuLabel(AP_MENU_ID_WINDOW_NEW,	"&Новое окно",		"Открыть новое окно для документа")
 MenuLabel(AP_MENU_ID_WINDOW_1,		"&1 %s",		"Переключиться на это окно")
 MenuLabel(AP_MENU_ID_WINDOW_2,		"&2 %s",		"Переключиться на это окно")
 MenuLabel(AP_MENU_ID_WINDOW_3,		"&3 %s",		"Переключиться на это окно")
 MenuLabel(AP_MENU_ID_WINDOW_4,		"&4 %s",		"Переключиться на это окно")
 MenuLabel(AP_MENU_ID_WINDOW_5,		"&5 %s",		"Переключиться на это окно")
 MenuLabel(AP_MENU_ID_WINDOW_6,		"&6 %s",		"Переключиться на это окно")
 MenuLabel(AP_MENU_ID_WINDOW_7,		"&7 %s",		"Переключиться на это окно")
 MenuLabel(AP_MENU_ID_WINDOW_8,		"&8 %s",		"Переключиться на это окно")
 MenuLabel(AP_MENU_ID_WINDOW_9,		"&9 %s",		"Переключиться на это окно")
 MenuLabel(AP_MENU_ID_WINDOW_MORE,	"&Другие окна",		"Показать полный список окон")

MenuLabel(AP_MENU_ID_HELP,		"&Помощь",		NULL)
 MenuLabel(AP_MENU_ID_HELP_CONTENTS,	"&Содержание",		"Вызвать содержание помощи")
 MenuLabel(AP_MENU_ID_HELP_INDEX,	"&Оглавление",		"Вызвать оглавление помощи")
 MenuLabel(AP_MENU_ID_HELP_CHECKVER,	"&Версия",		"Показать версию программы")
 MenuLabel(AP_MENU_ID_HELP_SEARCH,	"&Поиск",		"Искать помощь о...")
 MenuLabel(AP_MENU_ID_HELP_ABOUT,	"О &программе %s",	"Показать версию, статус и т. д.") 
 MenuLabel(AP_MENU_ID_HELP_ABOUTOS,	"О &Open Source",	"Показать информацию об Open Source")

MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",			"Заменить этим вариантом написания")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",			"Заменить этим вариантом написания")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",			"Заменить этим вариантом написания")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",			"Заменить этим вариантом написания")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",			"Заменить этим вариантом написания")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",			"Заменить этим вариантом написания")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",			"Заменить этим вариантом написания")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",			"Заменить этим вариантом написания")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",			"Заменить этим вариантом написания")
MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"&Игнорировать", 	"Игнорировать все вхождения")
MenuLabel(AP_MENU_ID_SPELL_ADD,		"&Добавить", 		"Добавить слово в словарь")

// ... add others here ...

MenuLabel(AP_MENU_ID__BOGUS2__,		NULL,			NULL)

EndSet()
