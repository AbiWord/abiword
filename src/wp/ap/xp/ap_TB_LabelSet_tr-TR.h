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

/* Translations are provided by "alper" <shullgum@yahoo.com> */

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

BeginSetEnc(tr,TR,true,"cp1254")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,			NoIcon,					NULL,NULL)

	//          (id, 		                    szLabel,		IconName,     			szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Yeni", 		tb_new_xpm,				NULL, "Yeni belge yarat")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Aç",			tb_open_xpm,			NULL, "Mevcut bir belgeyi aç")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Kaydet", 		tb_save_xpm,			NULL, "Belgeyi kaydet")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Farklý kaydet",tb_save_as_xpm,			NULL, "Belgeyi farklý adla kaydet")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Yazdýr",		tb_print_xpm,			NULL, "Belgeyi yazdýr")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,	"Basým Önizleme",	tb_print_preview_xpm, NULL, "Basýmdan önce belgeyi önizle")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Geri al",		tb_undo_xpm,			NULL, "Düzenlemeyi geri al")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Yinele",		tb_redo_xpm,			NULL, "Geri alýnaný tekrar yap")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Kes",			tb_cut_xpm,				NULL, "Kes")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Kopyala",		tb_copy_xpm,			NULL, "Kopyala")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Yapýþtýr",		tb_paste_xpm,			NULL, "Yapýþtýr")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_HEADER,	 "Üstbilgi düzenle",tb_edit_editheader_xpm,			NULL, "Üstbilgiyi düzenle")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_FOOTER,	 "Altbilgi düzenle",tb_edit_editfooter_xpm,			NULL, "Altbilgiyi düzenle")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEHEADER, "Üstbilgiyi kaldýr",	tb_edit_removeheader_xpm,			NULL, "Üstbilgiyi kaldýr")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEFOOTER, "Altbilgiyi kaldýr",	tb_edit_removefooter_xpm,			NULL, "Altbilgiyi kaldýr")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"Yazým Denetimi",tb_spellcheck_xpm,		NULL, "Belgede yazým denetimi yap")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,				"Ýmge ekle",	tb_insert_graphic_xpm,	NULL, "Belgeye bir imge ekle")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Biçem",		NoIcon,					NULL, "Biçem")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Yazýtipi",		NoIcon,					NULL, "Yazýtipi")
    ToolbarLabel(AP_TOOLBAR_ID_FMT_HYPERLINK,  "Baðlantý ekle", tb_hyperlink,           NULL, "Belgeye bir metin baðlantýsý ekle")
    ToolbarLabel(AP_TOOLBAR_ID_FMT_BOOKMARK, "Yer imi ekle",    tb_anchor,              NULL, "Belgeye bir yer imi ekle")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Yazýtipi boyutu",	NoIcon,				NULL, "Yazýtipi boyutu")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Kalýn",		tb_text_bold_xpm,		NULL, "Kalýn")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Ýtalik",		tb_text_italic_xpm,		NULL, "Ýtalik")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Altçizgi",	    tb_text_underline_xpm,	NULL, "Altçizgi")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Üstçizgi",		tb_text_overline_xpm,	NULL, "Üstçizgi")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Çýkart",		tb_text_strikeout_xpm,	NULL, "Çýkart")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_TOPLINE,		"Tepeçizgi",	tb_text_topline_xpm,	NULL, "Tepeçizgi")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOTTOMLINE,	"Dipçizgi",		tb_text_bottomline_xpm,	NULL, "Dipçizgi")
	ToolbarLabel(AP_TOOLBAR_ID_HELP,			"Yardým",		tb_help_xpm,			NULL, "Yardým")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Üstsimge",	    tb_text_superscript_xpm,	NULL, "Üstsimge")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Altsimge",     tb_text_subscript_xpm,		NULL, "Altsimge")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Sembol",		tb_symbol_xpm,				NULL, "Araya sembol ekle")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Sola",			tb_text_align_left_xpm,		NULL, "Sola yaslý")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Ortala",		tb_text_center_xpm,			NULL, "Ortala")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Saða",		    tb_text_align_right_xpm,	NULL, "Saða yaslý")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Tam",		    tb_text_justify_xpm,		NULL, "Tam yaslý")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Önce aralýk yok",	tb_para_0before_xpm,	NULL, "Öncesinde aralýk yok")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"Önce 12 pt var",	tb_para_12before_xpm,	NULL, "Öncesinde 12 pt aralýk var")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Tek aralýk",	tb_line_single_space_xpm,	NULL, "Tek satýr aralýðý")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 aralýk",	tb_line_middle_space_xpm,	NULL, "1.5 satýr aralýðý")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Çift aralýk",	tb_line_double_space_xpm,	NULL, "Çift satýr aralýðý")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 Sütun",		tb_1column_xpm,			NULL, "1 Sütun")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 Sütun",		tb_2column_xpm,			NULL, "2 Sütun")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 Sütun",		tb_3column_xpm,			NULL, "3 Sütun")

	ToolbarLabel(AP_TOOLBAR_ID_VIEW_SHOWPARA,	"Tümünü göster",    tb_view_showpara_xpm,		NULL, "¶ Gizle/Göster")
	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Büyült",			NoIcon,					NULL, "Büyült")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Mermi imi",		tb_lists_bullets_xpm,	NULL, "Mermi imleri")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Numaralama",	    tb_lists_numbers_xpm,	NULL, "Numaralama")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Yazýtipi rengi",	tb_text_fgcolor_xpm,	NULL, "Yazýtipi rengi")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Vurgula",		    tb_text_bgcolor_xpm,	NULL, "Vurgula")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,			"Girinti arttýr",	tb_text_indent_xpm, 	NULL, "Girinti arttýr")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"Girinti azalt",	tb_text_unindent_xpm,	NULL, "Girinti azalt")

	ToolbarLabel(AP_TOOLBAR_ID_SCRIPT_PLAY,		"Betik yürüt",	    tb_script_play_xpm,		NULL, "Betiði yürüt")

#ifdef BIDI_ENABLED
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR,	"Soldan-saða metin",		tb_text_direction_ltr_xpm,	NULL, "Soldan-saða metin zorunlu")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL,	"Saðdan-sola metin",		tb_text_direction_rtl_xpm,	NULL, "Saðdan-sola metin zorunlu")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,		"Paragraf Yönü",	        tb_text_dom_direction_rtl_xpm,	NULL, "Baskýn paragraf yönünü deðiþtir")
#endif

     // ... add others here ...

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,			NoIcon,			NULL,NULL)

EndSet()
