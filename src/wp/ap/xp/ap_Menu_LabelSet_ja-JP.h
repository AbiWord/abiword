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

// If the third argument is UT_TRUE, then this is the fall-back for
// this language (named in the first argument).

BeginSet(ja,JP,true)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"ファイル(_F)",		NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"新規(_N)",			"新規に文書を作成します")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"開く(_O)",			"既存の文書を開きます")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"閉じる(_C)", 		"文書を閉じます")
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"保存(_S)", 		"文書を保存します")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"名前を付けて保存", "文書に別の名前をつけて保存します")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"ページ設定(_U)",	"印刷設定を変更します")
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"印刷(_P)",			"全てまたは文書の一部を印刷します")
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"(_1) %s",			"この文書を開きます")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"(_2) %s",			"この文書を開きます")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"(_3) %s",			"この文書を開きます")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"(_4) %s",			"この文書を開きます")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"(_5) %s",			"この文書を開きます")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"(_6) %s",			"この文書を開きます")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"(_7) %s",			"この文書を開きます")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"(_8) %s",			"この文書を開きます")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"(_9) %s",			"この文書を開きます")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"終了(_X)", 		"全てのウィンドウを閉じて、終了します")

	MenuLabel(AP_MENU_ID_EDIT,				"編集(_E)",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"アンドゥ(_U)",		"編集内容を元に戻します")
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"リドゥ(_R)",		"編集内容を一つ前に戻します")
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"切り取り(_T)",		"選択範囲を切り取って、クリップボードに格納します")
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"複写(_C)",			"選択範囲をコピーして、クリップボードに格納します")
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"貼り付け(_P)",		"クリップボードから挿入します")
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"クリア(_A)",		"選択範囲を削除します")
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"全て選択(_L)",		"文書全体を選択します")
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"検索(_F)",			"指定した文を検索します")
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"置換(_E)",			"指定した文を別の文に置き換えます")
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"ジャンプ(_G)",		"指定した場所にジャンプします")
	
	MenuLabel(AP_MENU_ID_VIEW,				"表示(_V)",			        NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"ツールバー(_T)",	        NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"標準(_S)",		            "標準ツールバーの表示/非表示")
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"書式(_F)",                 "書式ツールバーの表示/非表示")
	MenuLabel(AP_MENU_ID_VIEW_TB_EXTRA,		"拡張(_E)",			        "拡張ツールバーの表示/非表示")
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"ルーラー(_R)",		        "ルーラーの表示/非表示")
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"ステータスバー(_S)",	    "ステータスバーの表示/非表示")
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"段落表示(_G)",	            "段落を表示します")
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"ヘッダ・フッタ(_H)",	    "各ページの上下にある文を編集します")
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"ズーム(_Z)",			    "文書の表示を縮小/拡大して表示します")

	MenuLabel(AP_MENU_ID_INSERT,			"挿入(_I)",			        NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"ページ区切り(_B)",	        "ページ、カラムまたはセクションの区切りを挿入します")
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"ページ番号(_U)",	        "ページ番号を自動的に更新して挿入します")
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"日時(_T)",	                "日付と時刻を挿入します")
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"フィールド(_F)",	        "フィールドを計算結果を挿入します")
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"シンボル(_M)",		        "記号または他の特殊な文字を挿入します")
	MenuLabel(AP_MENU_ID_INSERT_GRAPHIC,	"画像(_P)",			        "既存の画像ファイルを挿入します")

	MenuLabel(AP_MENU_ID_FORMAT,			"書式(_O)",			        NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"フォント(_F)",		        "選択した文のフォントを変更します")
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"段落(_P)",		            "選択した段落の書式を変更します")
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"Bulletsと番号付け(_N)",	"選択した段落の Bullets と番号付けの追加または変更")
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"境界とシェード(_D)",	    "選択範囲の境界とシェードを追加します")
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"桁(_C)",			        "桁数を変更します")
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"スタイル(_Y)",		        "選択範囲の書式を定義したり適用します")
	MenuLabel(AP_MENU_ID_FMT_TABS,			"タブ(_T)",			        "タブをセットします")
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"太字(_B)",			        "選択範囲を太字にします (切り替え式)")
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"イタリック(_I)",	        "選択範囲をイタリックにします (切り替え式)")
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"下線(_U)",		            "選択範囲に下線を付けます (切り替え式)")
	MenuLabel(AP_MENU_ID_FMT_OVERLINE,		"上線(_O)",		            "選択範囲に上線を付けます (切り替え式)")
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"取消し線(_K)",		        "選択範囲に取り消し線を付けます (切り替え式)")
	MenuLabel(AP_MENU_ID_FMT_SUPERSCRIPT,	"上付き(_R)",	            "選択範囲を上付き文字にします (切り替え式)")
	MenuLabel(AP_MENU_ID_FMT_SUBSCRIPT,		"下付き(_S)",		        "選択範囲を下付き文字にします (切り替え式)")

	MenuLabel(AP_MENU_ID_TOOLS,				"ツール(_T)",			    NULL)   
	MenuLabel(AP_MENU_ID_TOOLS_SPELL,		"スペルチェック(_S)",		"文書中の間違った綴りをチェックします")
	MenuLabel(AP_MENU_ID_TOOLS_WORDCOUNT,	"文字数カウント(_W)",		"文書中の単語の合計をカウントします")
	MenuLabel(AP_MENU_ID_TOOLS_OPTIONS,		"設定(_P)",		            "全般の設定")

	MenuLabel(AP_MENU_ID_ALIGN,				"配置(_A)",			    NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"左詰め(_L)",			"段落を左詰めにします")
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"中央詰め(_C)",			"段落を中央詰めにします")
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"右詰め(_R)",			"段落を右詰めにします")
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"均等揃え(_J)",			"段落を均等に揃えます")

	MenuLabel(AP_MENU_ID_WINDOW,			"ウィンドウ(_W)",		NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"新規ウィンドウ(_N)",	"もう一つウィンドウを開きます")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"(_1) %s",			    "このウィンドウを表示します")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"(_2) %s",			    "このウィンドウを表示します")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"(_3) %s",			    "このウィンドウを表示します")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"(_4) %s",			    "このウィンドウを表示します")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"(_5) %s",			    "このウィンドウを表示します")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"(_6) %s",			    "このウィンドウを表示します")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"(_7) %s",			    "このウィンドウを表示します")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"(_8) %s",			    "このウィンドウを表示します")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"(_9) %s",			    "このウィンドウを表示します")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"更にウィンドウ(_M)",	"Show full list of windows")

	MenuLabel(AP_MENU_ID_HELP,			    "ヘルプ(_H)",		NULL)
	MenuLabel(AP_MENU_ID_HELP_CONTENTS,		"ヘルプ(_C)",	    "ヘルプを表示します")
	MenuLabel(AP_MENU_ID_HELP_INDEX,		"目次(_I)",		    "ヘルプの目次を表示します")
	MenuLabel(AP_MENU_ID_HELP_CHECKVER,		"バージョン(_V)",	"プログラムのバージョンを表示します")
	MenuLabel(AP_MENU_ID_HELP_SEARCH,		"検索(_S)",	        "ヘルプから検索します...")
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"%s 情報",		    "プログラム情報(バージョンや著作権)を表示します") 
	MenuLabel(AP_MENU_ID_HELP_ABOUTOS,		"Open Source(_O)",	"Open Source について情報を表示します")

	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_1,	"%s",				"この綴りに変更")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_2,	"%s",				"この綴りに変更")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_3,	"%s",				"この綴りに変更")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_4,	"%s",				"この綴りに変更")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_5,	"%s",				"この綴りに変更")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_6,	"%s",				"この綴りに変更")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_7,	"%s",				"この綴りに変更")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_8,	"%s",				"この綴りに変更")
	MenuLabel(AP_MENU_ID_SPELL_SUGGEST_9,	"%s",				"この綴りに変更")
	MenuLabel(AP_MENU_ID_SPELL_IGNOREALL,	"全て無視(_I)", 	"文書中のこの単語については全て無視します")
	MenuLabel(AP_MENU_ID_SPELL_ADD,			"追加(_A)", 		"この単語をユーザー辞書に追加します")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
