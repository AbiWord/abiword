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

// We use the Win32 '&' character to denote a keyboard accelerator on a menu item.
// If your platform doesn't have a way to do accelerators or uses a different
// character, remove or change the '&' in your menu constructor code.

// If the third argument is true, then this is the fall-back for
// this language (named in the first argument).

BeginSet(id,ID,true)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,			"&Berkas",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&Baru", 			"Membuat dokumen baru")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"B&uka",				"Membuka dokumen lama")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Tutup", 			"Menutup dokumenn")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Simpan", 			"Menyimpan dokumen")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"S&impan Sbg", 			"Menyimpan dokumen dengan nama lain")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,		"Ganti &pilihan cetak",		"Mengganti pilihan cetak")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"&Cetak",			"Mencetak seluruh atau sebagian dokumen")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Membuka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Membuka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Membuka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Membuka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Membuka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Membuka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Membuka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Membuka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Membuka dokumen ini")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"&Akhiri", 			"Menutup seluruh window dan mengakhiri AbiWord")

	MenuLabel(AP_MENU_ID_EDIT,				"&Ubah",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Kembali",			"Membatalkan perintah terakhir")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Berlaku lagi",		"Yang dibatalkan tadi berlaku lagi")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"&Potong",			"Memotong seleksi dan simpan di Clipboard")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Salin",			"Menyalinkan seleksi dan simpan di Clipboard")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"&Letakkan",			"Meletakkan isi Clipboard")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"&Hapus",			"Menghapuskan bagian yang dipilih")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,		"Pilih Se&mua",			"Memilih seluruh dokumen")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Cari",			"Menari suatu kata")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"G&anti",			"Menganti kata tersebut dengan kata lainnya")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"Pin&dah",			"Memindah ke bagian lain")
	
	MenuLabel(AP_MENU_ID_VIEW,				"&Tampilan",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Papan simbol perintah",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standar",			"Tampilkan atau sembunyikan papan simbol perintah standar")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,		"&Format",			"Tampilkan atau sembunyikan papan simbol perintah format")
	MenuLabel(AP_MENU_ID_VIEW_TB_EXTRA,		"&Ekstra",			"Tampilkan atau sembunyikan papan simbol perintah ekstra")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Mistar",			"Tampilkan atau sembunyikan mistar")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,		"Papan &keterangan",		"Tampilkan atau sembunyikan papan keterangan")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"&Tampilkan paragraf",		"Tampilkan huruf-huruf yg tidak dicetak")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Kepala surat dan catatan kaki",		"Ganti kata-kata yang di atas atau bawah di seluruh halaman")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"Per&besar",			"Memperbesar atau perkecil tampilan dokumen")

	MenuLabel(AP_MENU_ID_INSERT,			"&Sisipkan",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Pemutus",			"Sisipkan pemutus halaman, kolom, atau bagian")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"&Nomor Halaman",		"Sisipkan nomor halaman yang diganti secara otomatis")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,		"&Tanggal dan Jam",		"Sisipkan tanggal dan/atau jam")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Field",			"Sisipkan a calculated field")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"Sim&bol",			"Sisipkan simbol atau huruf istimewa lainnya")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,		"&Gambar",			"Sisipkan gambar dari berkas lain")

	MenuLabel(AP_MENU_ID_FORMAT,			"F&ormat",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Huruf",			"Mengganti genis huruf kata-kata dipilih")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Paragraf",			"Mengganti format paragraf yang diseleksi")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"Tanda &Bulat dan Penomoron",	"Menambahkan atau mengganti tanda bulat atau penomoron untuk paragraf diseleksi")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"Garis Batas dan &Corak",	"Menggunakan garis batas atau corak di yang telah diseleksi")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Kolom",			"Mengubah jumlah kolom")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"&Model",			"Memilih model untuk yg telah diseleksi")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tab",				"Memilih posisi tabulasi")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"T&ebal",			"Tebalkan tulisan yg telah diseleksi atau sebaliknya")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Miring",			"Memiringkan tulisan yg telah diseleksi atau sebaliknya")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Garis bawah",			"Garis bawahi tulisan yg telah diseleksi atau sebaliknya")
	MenuLabel(AP_MENU_ID_FMT_OVERLINE,		"Garis ata&s",			"Garis atasi tulisan yg telah diseleksi atau sebaliknya")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"C&oret",			"Mencoret tulisan yang telah diseleksi atau sebaliknya")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,		"Huruf &atas",			"Menaikkan posisi tulisan yang telah diseleksi")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"Huruf ba&wah",			"Menurunkan posisi tulisan yang telah diseleksi")

	MenuLabel(AP_MENU_ID_TOOLS,				"&Alat-alat",			NULL)
	MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"&Ejaan",			"Memeriksa ejaan")
	MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,		"&Hitung Kata",			"Menghitung berapa banyak kata di dalam dokumen")
	MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"P&ilihan",			"Menentukan opsi")

	MenuLabel(AP_MENU_ID_ALIGN,				"&Aturan",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Kiri",			"Meratakan kiri paragraf")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Tengah",			"Pusatkan paragraf di tengah")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"Ka&nan",			"Meratakan kanan paragraf")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Justifikasi",			"Meratakan paragraf dua sisi")

	MenuLabel(AP_MENU_ID_WINDOW,			"&Window",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"Window &Baru",			"Membuka window baru untuk dokumen")
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

	MenuLabel(AP_MENU_ID_HELP,				"&Bantuan",			NULL)
	MenuLabel(AP_MENU_ID_HELP_CONTENTS,		"&Isi Bantuan",		"Tampilkan isi bantuan")
	MenuLabel(AP_MENU_ID_HELP_INDEX,		"In&deks Bantuan",		"Tampilkan indeks bantuan")
	MenuLabel(AP_MENU_ID_HELP_CHECKVER,		"Nomor &Versi",		"Tampilkan nomor versi program")
	MenuLabel(AP_MENU_ID_HELP_SEARCH,		"&Cari Bantuan",	"Mencari bantuan mengenai...")
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&Mengenai %s",			"Tampilkan keterangan program, nomor versi, dan hak cipta")
	MenuLabel(AP_MENU_ID_HELP_ABOUTOS,		"Mengenai &Open Source",	"Tampilkan keterangan mengenai Open Source")

	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",					"Mengganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",					"Mengganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",					"Mengganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",					"Mengganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",					"Mengganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",					"Mengganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",					"Mengganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",					"Mengganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",					"Mengganti dengan ejaan disarankan")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"&Abaikan Semua", 			"Selalu abaikan kata ini")
	MenuLabel(AP_MENU_ID_SPELL_ADD,			"&Tambah", 			"Tambahkan kata ini ke kamus khusus")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
