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

BeginSetEnc(uk,UA,true,"cp1251")

MenuLabel(AP_MENU_ID__BOGUS1__,		NULL,			NULL)

//       (id,				szLabel,           	szStatusMsg)

MenuLabel(AP_MENU_ID_FILE,		"&Файл",		NULL)
 MenuLabel(AP_MENU_ID_FILE_NEW,		"&Новий", 		"Створіти новий документ")	
 MenuLabel(AP_MENU_ID_FILE_OPEN,	"&Відкрити",		"Відкрити існуючий документ")
 MenuLabel(AP_MENU_ID_FILE_SAVE,	"&Зберегти", 		"Зберегти документ")
 MenuLabel(AP_MENU_ID_FILE_SAVEAS,	"Зберегти &як", 	"Зберегти документ під іншим им`ям")
 MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"П&араметри сторінки",	"Змінити параметри друку")
 MenuLabel(AP_MENU_ID_FILE_PRINT,	"&Друк",		"Друкувати документ або його частину")
 MenuLabel(AP_MENU_ID_FILE_RECENT_1,	"&1 %s",		"Відкрити цей документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_2,	"&2 %s",		"Відкрити цей документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_3,	"&3 %s",		"Відкрити цей документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_4,	"&4 %s",		"Відкрити цей документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_5,	"&5 %s",		"Відкрити цей документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_6,	"&6 %s",		"Відкрити цей документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_7,	"&7 %s",		"Відкрити цей документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_8,	"&8 %s",		"Відкрити цей документ")
 MenuLabel(AP_MENU_ID_FILE_RECENT_9,	"&9 %s",		"Відкрити цей документ")
 MenuLabel(AP_MENU_ID_FILE_CLOSE,	"&Закрити", 		"Закрити документ")
 MenuLabel(AP_MENU_ID_FILE_EXIT,	"&Вихід", 		"Закрити всі документи і вийти")
    
MenuLabel(AP_MENU_ID_EDIT,		"&Редагування",	NULL)
 MenuLabel(AP_MENU_ID_EDIT_UNDO,	"С&касування",		"Відмінити попередню дію")
 MenuLabel(AP_MENU_ID_EDIT_REDO,	"&Повтор",		"Повторити попередню відмінену дію")
 MenuLabel(AP_MENU_ID_EDIT_CUT,		"&Вирізати",		"Вирізати виділення і вмістити його в буфер обміну")
 MenuLabel(AP_MENU_ID_EDIT_COPY,	"&Копіювати",		"Скопіювати виділення і вмістити його в буфер обміну")
 MenuLabel(AP_MENU_ID_EDIT_PASTE,	"В&ставити",		"Вставити вміст буфера обміну в документ")
 MenuLabel(AP_MENU_ID_EDIT_CLEAR,	"Про&чистити",		"Очистити виділення")
 MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"В&иділити все",	"Виділити весь документ")
 MenuLabel(AP_MENU_ID_EDIT_FIND,	"&Знайти",		"Знайти заданий текст")
 MenuLabel(AP_MENU_ID_EDIT_REPLACE,	"З&амінити",		"Замінити заданий текст іншим")
 MenuLabel(AP_MENU_ID_EDIT_GOTO,	"Пе&рейти до",		"Перемістити курсор в задане місце")

MenuLabel(AP_MENU_ID_VIEW,		"&Вигляд",		NULL)
 MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,	"&Панелі",		NULL)
 MenuLabel(AP_MENU_ID_VIEW_TB_STD,	"&Стандартна",		"Показати/сховати стандартну панель")
 MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"&Форматування",	"Показати/сховати панель форматування")
 MenuLabel(AP_MENU_ID_VIEW_TB_EXTRA,	"&Додаткова",		"Показати/сховати додаткову панель")
 MenuLabel(AP_MENU_ID_VIEW_RULER,	"&Лінійка",		"Показати/сховати лінійку")
 MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"&Рядок статусу",	"Показати/сховати рядок статусу")
 MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,	"П&оказати спецсимволи",	"Показати/сховати символи, що недрукуются")
 MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,	"&Верхній и нижній колонтитули",	"Редагувати текст внизу і вгорі кожній сторінці")
 MenuLabel(AP_MENU_ID_VIEW_ZOOM,	"&Масштаб",		"Зменшити/збільшити масштаб документа")

MenuLabel(AP_MENU_ID_INSERT,		"В&ставка",		NULL)
 MenuLabel(AP_MENU_ID_INSERT_BREAK,	"&Розрив",		"Вставити разрив сторінки, колонки або секцій")
 MenuLabel(AP_MENU_ID_INSERT_PAGENO,	"&Номера сторінок",	"Вставити автопоновлення номерів сторінок")
 MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"&Дата і час",		"Вставити дату/время")
 MenuLabel(AP_MENU_ID_INSERT_FIELD,	"&Поле",		"Вставити автопоновлення поля")
 MenuLabel(AP_MENU_ID_INSERT_SYMBOL,	"С&имвол",		"Вставити символ або інший спеціальний знак")
 MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,	"М&алюнок",		"Вставити існуючий малюнок із зовнішнього файла")

MenuLabel(AP_MENU_ID_FORMAT,		"Ф&орматування",	NULL)
 MenuLabel(AP_MENU_ID_FMT_FONT,		"&Шрифт",		"Змінити шрифт виділеного тексту")
 MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,	"&Абзац",		"Змінити формат виділеного абзацу")
 MenuLabel(AP_MENU_ID_FMT_BULLETS,	"&Списки",		"Додати/прибрати маркери/нумерацію у абзацах")
 MenuLabel(AP_MENU_ID_FMT_BORDERS,	"&Кордони і залиття",	"Додати/прибрати рамку і заливку у виділенни")
 MenuLabel(AP_MENU_ID_FMT_COLUMNS,	"К&олонки",		"Змінити к-сть колонок")
 MenuLabel(AP_MENU_ID_FMT_TABS,		"&Табуляція",		"Встановити позиції табуляцї")
 MenuLabel(AP_MENU_ID_FMT_BOLD,		"&Жирний",		"Зробити виділення жирним")
 MenuLabel(AP_MENU_ID_FMT_ITALIC,	"&Курсив",		"Зробити виділення курсивним")
 MenuLabel(AP_MENU_ID_FMT_UNDERLINE,	"&Підкреслення",	"Підкреслити выділення")
 MenuLabel(AP_MENU_ID_FMT_OVERLINE,	"Надк&реслення",	"Надкреслити выділення")
 MenuLabel(AP_MENU_ID_FMT_STRIKE,	"П&ерекреслення",	"Перекреслити виділення")
 MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,	"&Верхій індекс",	"Перевести виділення у верхній індекс")
 MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,	"&Нижній індекс",	"Перевести виділення в нижній індекс")
 MenuLabel(AP_MENU_ID_ALIGN,		"Р&івняння",	NULL)
 MenuLabel(AP_MENU_ID_ALIGN_LEFT,	"По &лівому краю",	"Вирівняти абзац по лівому краю")
 MenuLabel(AP_MENU_ID_ALIGN_CENTER,	"По &центру",		"Вирівняти абзац по центру")
 MenuLabel(AP_MENU_ID_ALIGN_RIGHT,	"По &правому краю",	"Вирівняти абзац по правому краю")
 MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,	"По &ширині",		"Вирівняти абзац по ширине листа")
 MenuLabel(AP_MENU_ID_FMT_STYLE,	"Ст&иль",		"Визначити або застосувати стилі до виділення")

MenuLabel(AP_MENU_ID_TOOLS,		"&Сервіс",		NULL)   
 MenuLabel(AP_MENU_ID_TOOLS_SPELL,	"&Правопис",		"Перевірити документ на правопис")
 MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,	"&Статистика",		"Полічити к-сть слів, символів і т. д. в документі")
 MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,	"&Установки",		"Змінити установки")

MenuLabel(AP_MENU_ID_WINDOW,		"&Вікно",		NULL)
 MenuLabel(AP_MENU_ID_WINDOW_NEW,	"&Нове вікно",		"Відкритти нове вікно для документа")
 MenuLabel(AP_MENU_ID_WINDOW_1,		"&1 %s",		"Перемкнутися на це вікно")
 MenuLabel(AP_MENU_ID_WINDOW_2,		"&2 %s",		"Перемкнутися на це вікно")
 MenuLabel(AP_MENU_ID_WINDOW_3,		"&3 %s",		"Перемкнутися на це вікно")
 MenuLabel(AP_MENU_ID_WINDOW_4,		"&4 %s",		"Перемкнутися на це вікно")
 MenuLabel(AP_MENU_ID_WINDOW_5,		"&5 %s",		"Перемкнутися на це вікно")
 MenuLabel(AP_MENU_ID_WINDOW_6,		"&6 %s",		"Перемкнутися на це вікно")
 MenuLabel(AP_MENU_ID_WINDOW_7,		"&7 %s",		"Перемкнутися на це вікно")
 MenuLabel(AP_MENU_ID_WINDOW_8,		"&8 %s",		"Перемкнутися на це вікно")
 MenuLabel(AP_MENU_ID_WINDOW_9,		"&9 %s",		"Перемкнутися на це вікно")
 MenuLabel(AP_MENU_ID_WINDOW_MORE,	"&Іші вікна",		"Показати повний список вікон")

MenuLabel(AP_MENU_ID_HELP,		"&Допомога",		NULL)
 MenuLabel(AP_MENU_ID_HELP_CONTENTS,	"&Зміст",		"Покликати зміст допомоги")
 MenuLabel(AP_MENU_ID_HELP_INDEX,	"&Оглавление",		"Покликати оглавление допомоги")
 MenuLabel(AP_MENU_ID_HELP_CHECKVER,	"&Версія",		"Показати версію програми")
 MenuLabel(AP_MENU_ID_HELP_SEARCH,	"&Пошук",		"Шукати допомогу про...")
 MenuLabel(AP_MENU_ID_HELP_ABOUT,	"О &програме %s",	"Показати версію, статус і т. д.") 
 MenuLabel(AP_MENU_ID_HELP_ABOUTOS,	"О &Open Source",	"Показати інформацію про Open Source")

MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",			"Заминити цім варіантом написання")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",			"Заминити цім варіантом написання")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",			"Заминити цім варіантом написання")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",			"Заминити цім варіантом написання")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",			"Заминити цім варіантом написання")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",			"Заминити цім варіантом написання")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",			"Заминити цім варіантом написання")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",			"Заминити цім варіантом написання")
MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",			"Заминити цім варіантом написання")
MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"&Ігнорувати",	 	"Ігнорувати всі вхождения")
MenuLabel(AP_MENU_ID_SPELL_ADD,		"&Додати", 		"Додати слово у словник")

// ... add others here ...

MenuLabel(AP_MENU_ID__BOGUS2__,		NULL,			NULL)

EndSet()
