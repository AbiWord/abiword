/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

// We use the Win32 '&' character to denote a keyboard accelerator on a menu item.
// If your platform doesn't have a way to do accelerators or uses a different
// character, remove or change the '&' in your menu constructor code.

// If the second argument is UT_TRUE, then this is the fall-back for
// this language (named in the first two letters).

BeginSet(IdID,UT_TRUE)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,			"&Fail",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Baru", 			"Buat dokumen baru")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"B&uka",				"Buka dokumen lama")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Tutup", 			"Tutup dokumenn")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Simpan", 			"Simpan dokumen")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"S&impan Sbg", 			"Simpan dokumen dengan nama lain")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,		"Ganti &pilihan cetak",		"Ganti pilihan cetak")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"&Cetak",			"Cetak seluruh atau sebagian dokumen")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Buka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Buka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Buka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Buka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Buka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Buka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Buka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Buka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Buka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"&Keluar", 			"Tutup seluruh window dan keluar dari AbiWord")

	MenuLabel(AP_MENU_ID_EDIT,				"&Ubah",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Kembali",			"Batalkan perintah terakhir")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Berlaku lagi",			"Yang dibatalkan tadi berlaku lagi")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"&Gunting",			"Guntingkan seleksi dan simpan di Clipboard")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Salin",			"Salinkan seleksi dan simpan di Clipboard")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"&Letakkan",			"Letakkan isi Clipboard")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"&Hapus",			"Hapuskan seleksi")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,		"Seleksi &Seluruh",		"Seleksi seluruh dokumen")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Cari",				"Cari kata-kata tersebut")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"G&anti",			"Ganti kata tersebut dengan kata lainnya")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"&Pindah",			"Pindah ke sesuatu tempat")
	MenuLabel(AP_MENU_ID_EDIT_SPELL,		"&Ejaan",			"Cek dokumen, cari ejaan yg salah")
	MenuLabel(AP_MENU_ID_EDIT_OPTIONS,		"P&ilihan",			"Memilih")
	
	MenuLabel(AP_MENU_ID_VIEW,				"&Tampilan",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Papan simbol perintah",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standar",			"Tampilkan atau sembunyikan papan simbol perintah standar")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,		"&Format",			"Tampilkan atau sembunyikan papan simbol perintah format")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Mistar",			"Tampilkan atau sembunyikan mistar")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,		"Papan &keterangan",		"Tampilkan atau sembunyikan papan keterangan")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"&Tampilkan paragraf",		"Tampilkan huruf-huruf yg tidak dicetak")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Kepala surat dan catatan kaki",		"Ganti kata-kata yang di atas atau bawah di seluruh halaman")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"Per&besar",			"Perbesar atau perkecil tampilan dokumen")

	MenuLabel(AP_MENU_ID_INSERT,			"&Selip",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Akhir",			"Akhiri halaman, kolom, atau bagian")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"&Nomor Halaman",		"Selipkan nomor halaman yang diganti secara automatis")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,		"&Tanggal dan Jam",		"Selipkan tanggal dan/atau jam")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Field",			"Selipkan a calculated field")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"Sim&bol",			"Selipkan simbol atau huruf istimewa lainnya")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,		"&Gambar",			"Selipkan gambar dari fail lain")

	MenuLabel(AP_MENU_ID_FORMAT,			"F&ormat",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Fon",				"Ganti fonnya kata-kata diseleksi")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Paragraf",			"Ganti formatnya paragraf diseleksi")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"Tanda &Bulat dan Penomoron",	"Tambahkan atau ganti tanda bulat atau penomoron untuk paragraf diseleksi")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"Batas dan &Corak",		"Pasangkan batas atau corak di yg telah diseleksi")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Kolom",			"Ganti berapa banyak kolom")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"&Model",			"Pilih model untuk yg telah diseleksi")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tab",				"Pilih posisi tab")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"T&ebal",			"Tebalkan yg telah diseleksi")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Miring",			"Miringkan yg telah diseleksi")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Garis bawah",			"Yg telah diseleksi jadi garis bawah")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"C&oret",			"Coretkan yg telah diseleksi (toggle)")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,		"Huruf &atas",			"Naikkan yg telah diseleksi")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"Huruf ba&wah",			"Turunkan yg telah diseleksi")

	MenuLabel(AP_MENU_ID_ALIGN,				"&Aturan",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Kiri",			"Rata kirikan paragraf")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Tengah",			"Pusatkan paragraf di tengah")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"Ka&nan",			"Rata kanankan paragraf")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Justifikasi",			"Justifikasikan paragraf")

	MenuLabel(AP_MENU_ID_WINDOW,			"&Window",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"Window Baru",			"Buka window baru untuk dokumen")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			"Naikkan window")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			"Naikkan window")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			"Naikkan window")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			"Naikkan window")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			"Naikkan window")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			"Naikkan window")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			"Naikkan window")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			"Naikkan window")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			"Naikkan window")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"Window &lain",			"Tampilkan daftar lengkap window")

	MenuLabel(AP_MENU_ID_HELP,				"&Tolong",			NULL)
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Mengenai %s",			"Tampilkan keterangan program, nomor versi, dan hak cipta")

	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",					"Ganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",					"Ganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",					"Ganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",					"Ganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",					"Ganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",					"Ganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",					"Ganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",					"Ganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",					"Ganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"&Abaikan Semua", 			"Selalu abaikan kata ini")
	MenuLabel(AP_MENU_ID_SPELL_ADD,			"&Tambah", 			"Tambahkan kata ini ke kamus khusus")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
