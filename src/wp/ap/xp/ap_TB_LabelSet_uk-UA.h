/* AbiWord
 * Copyright (C) 1998-2001 AbiSource, Inc.
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

BeginSetEnc(uk,UA,true,"koi8-u")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,			NoIcon,					NULL,NULL)

	//          (id, 		                    szLabel,		IconName,     			szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Новий", 			tb_new_xpm,				NULL, "Створити новий документ")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"В╕дкрити",			tb_open_xpm,			NULL, "В╕дкрити ╕снуючий документ")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Зберегти", 		tb_save_xpm,			NULL, "Зберегти документ")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Зберегти як", 		tb_save_as_xpm,			NULL, "Зберегти документ п╕д ╕ншoю назвою")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Друк",		tb_print_xpm,			NULL, "Друкувати документ або його частину")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,	"Перегляд друку",	tb_print_preview_xpm, NULL, "Перегляд документу перед друком")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"В╕дм╕нити",			tb_undo_xpm,			NULL, "В╕дм╕нити редагування")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Повторити",			tb_redo_xpm,			NULL, "Повторити попередню в╕дм╕нену д╕ю")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Вир╕зати",			tb_cut_xpm,				NULL, "Вир╕зати вид╕лення ╕ пом╕стити в буфер обм╕ну")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Скоп╕ювати",			tb_copy_xpm,			NULL, "Скоп╕ювати вид╕лення")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Вставити",		tb_paste_xpm,			NULL, "Вставити з буферу обм╕ну")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_HEADER,	 "Редагувати верхн╕й колонтитул",		tb_edit_editheader_xpm,			NULL, "Редагувати верхн╕й колонтитул документу")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_FOOTER,	 "Редагувати нижн╕й колонтитул",		tb_edit_editfooter_xpm,			NULL, "Редагувати нижн╕й колонтитул документу")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEHEADER, "Стерти верхн╕й колонтитул",		tb_edit_removeheader_xpm,			NULL, "Стерти верхн╕й колонтитул документу")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEFOOTER, "Стерти нижн╕й колонтитул",		tb_edit_removefooter_xpm,			NULL, "Стерти нижн╕й колонтитул документу")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"Перев╕рка правопису",	tb_spellcheck_xpm,		NULL, "Перев╕рити правопис документу")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,				"Вставити зображення",	tb_insert_graphic_xpm,	NULL, "Вставити зображення в документ")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Стиль",		NoIcon,					NULL, "Визначити або застосувати стил╕")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Шрифт",			NoIcon,					NULL, "Зм╕нити шрифт тексту")
     ToolbarLabel(AP_TOOLBAR_ID_FMT_HYPERLINK, "Вставити посилання", tb_hyperlink, NULL, "Вставити посилання в документ")
     ToolbarLabel(AP_TOOLBAR_ID_FMT_BOOKMARK, "Вставити закладку", tb_anchor, NULL, "Вставити закладку в документ")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Розм╕р шрифту",	NoIcon,					NULL, "Розм╕р шрифтa тексту")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Жирний",			tb_text_bold_xpm,		NULL, "Зробити вид╕лення жирним")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Курсивний",		tb_text_italic_xpm,		NULL, "Зробити вид╕лення курсивним")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"П╕дкреслення",	tb_text_underline_xpm,	NULL, "П╕дкреслити вид╕лення")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Надкреслення",		tb_text_overline_xpm,	NULL, "Надкреслити вид╕лення")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Перекреслення",		tb_text_strikeout_xpm,	NULL, "Перекреслити вид╕лення")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_TOPLINE,		"Л╕н╕я вгор╕",		tb_text_topline_xpm,	NULL, "Провести л╕н╕ю над вид╕ленням")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOTTOMLINE,		"Л╕н╕я внизу",		tb_text_bottomline_xpm,	NULL, "Провести л╕н╕ю п╕д вид╕ленням")
	ToolbarLabel(AP_TOOLBAR_ID_HELP,			"Допомога",			tb_help_xpm,			NULL, "Показати допомогу")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Верхн╕й ╕ндекс",	tb_text_superscript_xpm,	NULL, "Перевести вид╕лення у верхн╕й ╕ндекс")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Нижн╕й ╕ндекс",	tb_text_subscript_xpm,		NULL, "Перевести вид╕лення в нижн╕й ╕ндекс")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Символ",		tb_symbol_xpm,				NULL, "Вставити символ або ╕нший спец╕альний знак")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"По л╕вому краю",			tb_text_align_left_xpm,		NULL, "Вир╕вняти абзац по л╕вому краю")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"По центру",		tb_text_center_xpm,			NULL, "Вир╕вняти абзац по центру")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"По правому краю",		tb_text_align_right_xpm,	NULL, "Вир╕вняти абзац по правому краю")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"По ширин╕",		tb_text_justify_xpm,		NULL, "Вир╕вняти абзац по ширин╕")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Без в╕дступу",		tb_para_0before_xpm,	NULL, "Абзац без попереднього в╕дступу")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"В╕дступ 12 пкт",		tb_para_12before_xpm,	NULL, "В╕дступити на 12 пункт╕в перед абзацом")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"╤нтервал 1",	tb_line_single_space_xpm,	NULL, "Одинарна в╕дстань м╕ж рядками абзацу")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"╤нтервал 1.5",		tb_line_middle_space_xpm,	NULL, "В╕дстань в п╕втора рядка м╕ж рядками абзацу")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"╤нтервал 2",	tb_line_double_space_xpm,	NULL, "Подв╕йна в╕дстань м╕ж рядками абзацу")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 стовпець",			tb_1column_xpm,			NULL, "Текст документу в одному стовпц╕")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 стовпц╕",		tb_2column_xpm,			NULL, "Текст документу в двох стовпцях")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 стовпц╕",		tb_3column_xpm,			NULL, "Текст документу в трьох стовпцях")

	ToolbarLabel(AP_TOOLBAR_ID_VIEW_SHOWPARA,	"Показати все",			tb_view_showpara_xpm,		NULL, "Показати/сховати спец. символи")
	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Масштаб",				NoIcon,					NULL, "Зм╕нити масштаб в╕дображення документу")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Маркований список",			tb_lists_bullets_xpm,	NULL, "Маркований список")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Нумерований список",		tb_lists_numbers_xpm,	NULL, "Нумерований список")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Кол╕р шрифту",		tb_text_fgcolor_xpm,	NULL, "Встановити кол╕р шрифту")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Кол╕р вид╕лення",		tb_text_bgcolor_xpm,	NULL, "Встановити кол╕р вид╕лення")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,			"Зб╕льшити в╕дступ",	tb_text_indent_xpm, 	NULL, "Зб╕льшити в╕дступ абзацу")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"Зменшити в╕дступ",	tb_text_unindent_xpm,	NULL, "Зменшити в╕дступ абзацу")

	ToolbarLabel(AP_TOOLBAR_ID_SCRIPT_PLAY,		"Виконати скрипт",	tb_script_play_xpm,		NULL, "Виконати скрипт")

     ToolbarLabel(AP_TOOLBAR_ID_FMTPAINTER, "Застосувати формат", tb_stock_paint_xpm, NULL, "Застосувати попередньо скоп╕йований формат до вид╕леного абзацу")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR,	"Примусити напрямок зл╕ва направо",		tb_text_direction_ltr_xpm,	NULL, "Примусити напрямок тексту зл╕ва направо")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL,	"Примусити напрямок справа нал╕во",		tb_text_direction_rtl_xpm,	NULL, "Примусити напрямок тексту справа нал╕во")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,		"Напрямок абзацу",	tb_text_dom_direction_rtl_xpm,	NULL, "Зм╕нити дом╕нантний напрямок абзацу")

     // ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,			NoIcon,			NULL,NULL)

EndSet()
