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

// Indonesian translation by Tim & Rita Allen <tim@proximity.com.au>
// Amendments suggested by I Made Wiryana <made@nakula.rvs.uni-bielefeld.de>

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

BeginSet(id,ID,true)

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,		NoIcon,			NULL,NULL)

	//          (id, 		                    szLabel,	IconName,     	szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Baru", 	tb_new_xpm,	NULL, "Buat dokumen baru")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Buka",		tb_open_xpm,	NULL, "Buka dokumen lama")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Simpan", 	tb_save_xpm,	NULL, "Simpan dokumen")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Simpan Sbg", 	tb_save_as_xpm,	NULL, "Simpan dokumen dengan nama lain")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Cetak",	tb_print_xpm,	NULL, "Cetak dokumen")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,  "Tunjukkan cetak", tb_print_preview_xpm, NULL, "Tunjukkan bentuk cetak")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Kembali",	tb_undo_xpm,	NULL, "Batalkan perintah terakhir")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Berlaku lagi",		tb_redo_xpm,	NULL, "Yang dibatalkan tadi berlaku lagi")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Potong",	tb_cut_xpm,	NULL, "Potong")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Salin",	tb_copy_xpm,	NULL, "Salin")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Letakkan",	tb_paste_xpm,	NULL, "Letakkan")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,          "Cek ejaan", tb_spellcheck_xpm, NULL, "Periksa ejaan dokumen")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,                 "Selipkan gambar", tb_insert_graphic_xpm, NULL, "Selipkan gambar")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Model",	NoIcon,			NULL, "Model")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Huruf",		NoIcon,			NULL, "Huruf")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Ukuran huruf",	NoIcon,			NULL, "Ukuran huruf")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Tebal",		tb_text_bold_xpm,		NULL, "Tebal")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Miring",	tb_text_italic_xpm,	NULL, "Miring")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Garis bawah",tb_text_underline_xpm,	NULL, "Garis bawah")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Garis atas",tb_text_overline_xpm,	NULL, "Garis atas")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Coret",   tb_text_strikeout_xpm,	NULL, "Dicoret")
        ToolbarLabel(AP_TOOLBAR_ID_HELP,                "Bantuan", tb_help_xpm, NULL, "Bantuan")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Huruf dinaikkan",	tb_text_superscript_xpm,	NULL, "Huruf dinaikkan")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Huruf diturunkan",	tb_text_subscript_xpm,		NULL, "Huruf diturunkan")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Simbol",	tb_symbol_xpm,		NULL, "Sisipkan simbol")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Kiri",		tb_text_align_left_xpm,		NULL, "Rata kiri")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Tengah",	tb_text_center_xpm,	NULL, "Pusatkan di tengah")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Kanan",	tb_text_align_right_xpm,	NULL, "Rata kanan")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Rata dua sisi",	tb_text_justify_xpm,	NULL, "Rata dua sisi")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Tidak ada sebelum",		tb_para_0before_xpm,	NULL, "Ruang sebelum: Tidak ada")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt sebelum",		tb_para_12before_xpm,	NULL, "Ruang sebelum: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Satu Spasi",	tb_line_single_space_xpm,	NULL, "Satu spasi")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 Spasi",	tb_line_middle_space_xpm,	NULL, "1,5 spasi")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Dua Spasi",	tb_line_double_space_xpm,	NULL, "Dua spasi")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,		"1 Kolom",		tb_1column_xpm,	NULL, "1 Kolom")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,		"2 Kolom",		tb_2column_xpm,	NULL, "2 Kolom")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,		"3 Kolom",		tb_3column_xpm,	NULL, "3 Kolom")

	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,		"Perbesar",		NoIcon,	                NULL, "Perbesar")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Daftar bundar kitam",	tb_lists_xpm,		NULL, "Daftar bundar hitam")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Daftar bernomor",	tb_lists_numbers_xpm,	NULL, "Daftar bernomor")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Warna Depan",	        NoIcon,			NULL, "Warna depan")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Warna Belakang",	NoIcon,		        NULL, "Warna belakang")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,		"Inden Paragraf", tb_text_indent_xpm, 	        NULL, "Inden ke kanan paragraf")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"Batal Inden", tb_text_unindent_xpm,	        NULL, "Inden balik ke kiri")
	
	// ... add others here ...
#ifdef BIDI_ENABLED
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIRECTION,		"Arah Teks",	tb_text_direction_rtl_xpm,	NULL, "Mengganti arah teks")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,		"Arah Teks Paragraf",	tb_text_dom_direction_rtl_xpm,	NULL, "Mengganti arah teks paragraf")
#endif
	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,		NoIcon,			NULL,NULL)

EndSet()
