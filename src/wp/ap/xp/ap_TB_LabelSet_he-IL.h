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

BeginSetEnc(he,IL,false,"iso-8859-8")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,			NoIcon,					NULL,NULL)

	//          (id, 		                    szLabel,		IconName,     			szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"חדש", 			tb_new_xpm,				NULL, "יצירת מסמך חדש")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"פתיחה",			tb_open_xpm,			NULL, "פתיחת מסמך קיים")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"שמור", 		tb_save_xpm,			NULL, "שמירת המסמך הפעיל")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"שמירה בשם", 		tb_save_as_xpm,			NULL, "שמירת המסמך הפעיל עם שם חדש")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"הדפסה",		tb_print_xpm,			NULL, "הדפסת המסמך הפעיל")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,	"הצג לפני הדפסה",	tb_print_preview_xpm, NULL, "Preview the document before printing")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"בטל הקלדה",			tb_undo_xpm,			NULL, "ביטול הפעולה האחרונה")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"חזור על הקלדה",			tb_redo_xpm,			NULL, "חזרה על הפעולה האחרונה")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"גזור",			tb_cut_xpm,				NULL, "גזור")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"העתק",			tb_copy_xpm,			NULL, "העתק")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"הדבק",		tb_paste_xpm,			NULL, "הדבק")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"בדוק איות",	tb_spellcheck_xpm,		NULL, "בדוק איות במסמך")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,				"הוסף תמונה",	tb_insert_graphic_xpm,	NULL, "הוסף תמונה למסמך")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"סגנון",		NoIcon,					NULL, "סגנון")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"גופן",			NoIcon,					NULL, "גופן")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Font Size",	NoIcon,					NULL, "Font Size")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"מודגש",			tb_text_bold_xpm,		NULL, "מודגש")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"נטוי",		tb_text_italic_xpm,		NULL, "נטוי")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"קו תחתון",	tb_text_underline_xpm,	NULL, "קו תחתון")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"קו עליון",		tb_text_overline_xpm,	NULL, "קו עליון")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"קו חוצה",		tb_text_strikeout_xpm,	NULL, "קו חוצה")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_TOPLINE,		"Topline",		tb_text_topline_xpm,	NULL, "Topline")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOTTOMLINE,		"Bottomline",		tb_text_bottomline_xpm,	NULL, "Bottomline")
	ToolbarLabel(AP_TOOLBAR_ID_HELP,			"עזרה",			tb_help_xpm,			NULL, "עזרה")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"כתב עילי",	tb_text_superscript_xpm,	NULL, "כתב עילי")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"כתב תחתי",	tb_text_subscript_xpm,		NULL, "כתב תחתי")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"סימן",		tb_symbol_xpm,				NULL, "הוסף סימן")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"לשמאל",			tb_text_align_left_xpm,		NULL, "יישור לשמאל")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"ממורכז",		tb_text_center_xpm,			NULL, "ממורכז")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"לימין",		tb_text_align_right_xpm,	NULL, "יישור לימין")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"יישור דו-צידי",		tb_text_justify_xpm,		NULL, "יישור דו-צידי")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"None before",		tb_para_0before_xpm,	NULL, "Space before: None")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt before",		tb_para_12before_xpm,	NULL, "Space before: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"בודד",	tb_line_single_space_xpm,	NULL, "בודד")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"שורה וחצי",		tb_line_middle_space_xpm,	NULL, "שורה וחצי")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"כפול",	tb_line_double_space_xpm,	NULL, "כפול")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"טור אחד",			tb_1column_xpm,			NULL, "טור אחד")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"שני טורים",		tb_2column_xpm,			NULL, "שני טורים")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"שלושה טורים",		tb_3column_xpm,			NULL, "שלושה טורים")

	ToolbarLabel(AP_TOOLBAR_ID_VIEW_SHOWPARA,	"מסך מלא",			tb_view_showpara_xpm,		NULL, "Show/hide ¶")
	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"מרחק מתצוגה",				NoIcon,					NULL, "מרחק מתצוגה")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"תבליטים",			tb_lists_bullets_xpm,	NULL, "תבליטים")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"מספור",		tb_lists_numbers_xpm,	NULL, "מספור")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"צבע גופן",		tb_text_fgcolor_xpm,	NULL, "צבע גופן")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Highlight",		tb_text_bgcolor_xpm,	NULL, "Highlight")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,			"הגדל כניסה",	tb_text_indent_xpm, 	NULL, "הגדל כניסה")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"הקטן כניסה",	tb_text_unindent_xpm,	NULL, "הקטן כניסה")

	ToolbarLabel(AP_TOOLBAR_ID_SCRIPT_PLAY,		"הפעל script",	tb_script_play_xpm,		NULL, "הפעל script")

     // ... add others here ...
#ifdef BIDI_ENABLED
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR,	"אלץ כיוון אנגלי",		tb_text_direction_ltr_xpm,	NULL, "אלץ כיוון אנגלי של הטקסט")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL,	"אלץ כיוון עברי",		tb_text_direction_rtl_xpm,	NULL, "אלץ כיוון עברי של הטקסט")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,		"כיוון פיסקה",	tb_text_dom_direction_rtl_xpm,	NULL, "שנה כיוון פיסקה ראשי")
#endif
	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,			NoIcon,			NULL,NULL)

EndSet()
