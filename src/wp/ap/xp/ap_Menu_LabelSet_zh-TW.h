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

/*
 * Traditional Chinese Translation by
 *
 *  Chinese Linux Extensions Project  http://cle.linux.org.tw/
 *     Chih-Wei Huang <cwhuang@linux.org.tw>
 *     Armani         <armani@cle.linux.org.tw>
 *
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

BeginSetEnc(zh,TW,true,"BIG5")

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,			"檔案[&F]",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"開新檔案[&N]",			"建立一份新的文件")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"開啟舊檔[&O]",			"開啟舊檔")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"關閉[&C]", 			"關閉這份文件")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"儲存[&S]", 			"儲存這份文件")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"儲存為[&A]", 			"將檔案存為其它的檔名")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,		"頁面設定[&u]",			"改變輸出選項")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"列印[&P]",			"列印出所有或部份文件")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"[&1] %s",			"開啟這份文件")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"[&2] %s",			"開啟這份文件")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"[&3] %s",			"開啟這份文件")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"[&4] %s",			"開啟這份文件")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"[&5] %s",			"開啟這份文件")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"[&6] %s",			"開啟這份文件")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"[&7] %s",			"開啟這份文件")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"[&8] %s",			"開啟這份文件")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"[&9] %s",			"開啟這份文件")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"離開[&x]", 			"關閉這個應用程式所有視窗並離開")

	MenuLabel(AP_MENU_ID_EDIT,			"編輯[&E]",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"復原[&U]",			"復原")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"重做[&R]",			"重做")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"剪下[&t]",			"剪下選取區域的內容並貼到剪貼簿上")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"複製[&C]",			"複制選取區內的內容並貼到剪貼簿上")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"貼上[&P]",			"將剪貼簿上的內容貼到工作區")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"清除[&a]",			"刪除選取範圍內的內容")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,		"全選[&l]",			"選取全部的內容文件")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"尋找[&F]",			"搜尋特定的文字")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"置換[&e]",			"置換不同的字串或字元")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"到..[&G]",			"跳到新的插入點")
	
	MenuLabel(AP_MENU_ID_VIEW,			"檢視[&V]",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"工具[&T]",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"顯現/隱藏 標準工具列[&S]",	"顯現/隱藏標準工具列")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,		"顯現/隱藏 格式工具列[&F]",     "顯現/隱藏格式工具列")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"尺規[&R]",			"顯現/隱藏尺規工具列")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,		"狀態列[&S]",			"顯現/隱藏尺規工具列")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"顯示段落框線[&g]",		"顯示不會列印出來的段落框線標記")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"頁首/頁尾[&H]",		"編輯每一頁的頁首/頁尾")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"縮放視窗[&Z]",			"縮放視窗")

	MenuLabel(AP_MENU_ID_INSERT,			"插入[&I]",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"分格線[&B]",			"插入一個頁面或者欄位或者區段分隔線註記")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"頁碼[&u]",			"插入一個頁碼")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,		"日期/時間[&T]",		"插入時間或日期")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"區間[&F]",			"插入一個計算區間")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"符號[&m]",			"插入一個符號或特別字元")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,		"圖片[&P]",			"從現有的檔案開啟圖片")

	MenuLabel(AP_MENU_ID_FORMAT,			"格式[&o]",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"字型[&F]",			"改變字型")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"段落[&P]",			"改變段落")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"項目符號[&N]",			"增加或修改段落裡的項目符號")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"邊框及網底[&d]",		"邊框及網底設定")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"改變欄位數[&C]",		"改變欄位數")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"格式[&y]",			"段落風格設定")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabs",			"Set tab stops")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"粗體[&B]",			"使選取區字體變粗")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"斜體[&I]",			"使選取區字體變斜")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"底線[&U]",			"在選取區加入底線")
	MenuLabel(AP_MENU_ID_FMT_OVERLINE,		"外框[&O]",			"在選取區加入外框")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"中間線[&k]",			"將字串或字元從中槓掉")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,		"上標[&r]",			"使選取區文字成為上標字")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"下標[&S]",			"使選取區文字成為下標字")

	MenuLabel(AP_MENU_ID_TOOLS,			"工具[&T]",			NULL)   
	MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"拼字檢查[&S]",			"檢查拼字")
	MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,		"字數統計[&W]",			"計算本篇文章的字數")
	MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"選項[&O]",			"設定選項")

	MenuLabel(AP_MENU_ID_ALIGN,			"對齊[&A]",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"靠左對齊[&L]",			"靠左對齊")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"置中對齊[&C]",			"置中對齊")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"靠右對齊[&R]",			"靠右對齊")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"左右對齊[&J]",				"根據文章對齊")

	MenuLabel(AP_MENU_ID_WINDOW,			"視窗[&W]",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"在新視窗開啟..[&N]",		"將此份文章以另一視窗開啟")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"[&1] %s",			"喚起這個視窗")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"[&2] %s",			"喚起這個視窗")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"[&3] %s",			"喚起這個視窗")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"[&4] %s",			"喚起這個視窗")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"[&5] %s",			"喚起這個視窗")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"[&6] %s",			"喚起這個視窗")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"[&7] %s",			"喚起這個視窗")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"[&8] %s",			"喚起這個視窗")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"[&9] %s",			"喚起這個視窗")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"更多的視窗..[&M]",		"顯視所有視窗列表")

	MenuLabel(AP_MENU_ID_HELP,			"說明[&H]",			NULL)
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"關於[&A] %s",		"程式資訊, 版本號碼,版權歸屬 ")

	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",				"改變為這個建議拼字方式")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",				"改變為這個建議拼字方式")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",				"改變為這個建議拼字方式")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",				"改變為這個建議拼字方式")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",				"改變為這個建議拼字方式")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",				"改變為這個建議拼字方式")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",				"改變為這個建議拼字方式")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",				"改變為這個建議拼字方式")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",				"改變為這個建議拼字方式")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"忽略[&I]", 			"忽略所有現行的文件")
	MenuLabel(AP_MENU_ID_SPELL_ADD,		"增加[&A]", 			"增加這個字到自定的字典")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
