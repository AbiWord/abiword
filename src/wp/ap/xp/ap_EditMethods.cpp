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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "fl_AutoLists.h"
#include "fp_Page.h"
#include "fg_Graphic.h"
#include "pd_Document.h"
#include "gr_Graphics.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_EditMethods.h"
#include "xap_Menu_Layouts.h"
#include "xap_Prefs.h"
#include "ap_Strings.h"
#include "ap_LoadBindings.h"
#include "ap_FrameData.h"
#include "ap_LeftRuler.h"
#include "ap_Prefs.h"
#include "ut_string_class.h"

#include "ap_Dialog_Id.h"
#include "ap_Dialog_Replace.h"
#include "ap_Dialog_Goto.h"
#include "ap_Dialog_Break.h"
#include "ap_Dialog_Paragraph.h"
#include "ap_Dialog_PageNumbers.h"
#include "ap_Dialog_PageSetup.h"
#include "ap_Dialog_Lists.h"
#include "ap_Dialog_Options.h"
#include "ap_Dialog_Spell.h"
#include "ap_Dialog_Styles.h"
#include "ap_Dialog_Tab.h"
#include "ap_Dialog_Insert_DateTime.h"
#include "ap_Dialog_Field.h"
#include "ap_Dialog_WordCount.h"
#include "ap_Dialog_Columns.h"
#include "ap_Dialog_Tab.h"
#include "ap_Dialog_ToggleCase.h"
#include "ap_Dialog_Background.h"

#include "xap_App.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_About.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Dlg_FileOpenSaveAs.h"
#include "xap_Dlg_FontChooser.h"
#include "xap_Dlg_Print.h"
#include "xap_Dlg_PrintPreview.h"
#include "xap_Dlg_WindowMore.h"
#include "xap_Dlg_Zoom.h"
#include "xap_Dlg_Insert_Symbol.h"
#include "xap_Dlg_Language.h"

#include "ie_imp.h"
#include "ie_impGraphic.h"
#include "ie_exp.h"
#include "ie_types.h"

/*****************************************************************/
/*****************************************************************/

/* abbreviations:
**   BOL    beginning of line
**   EOL    end of line
**   BOW    beginning of word
**   EOW    end of word
**   BOW    beginning of sentence
**   EOS    end of sentence
**   BOB    beginning of block
**   EOB    end of block
**   BOD    beginning of document
**   EOD    end of document
**
**   warpInsPt    warp insertion point
**   extSel       extend selection
**   del          delete
**   bck          backwards
**   fwd          forwards
**/

class ap_EditMethods
{
public:
	static EV_EditMethod_Fn scrollPageDown;
	static EV_EditMethod_Fn scrollPageUp;
	static EV_EditMethod_Fn scrollPageLeft;
	static EV_EditMethod_Fn scrollPageRight;
	static EV_EditMethod_Fn scrollLineDown;
	static EV_EditMethod_Fn scrollLineUp;
	static EV_EditMethod_Fn scrollLineLeft;
	static EV_EditMethod_Fn scrollLineRight;
	static EV_EditMethod_Fn scrollToTop;
	static EV_EditMethod_Fn scrollToBottom;

	static EV_EditMethod_Fn warpInsPtToXY;
	static EV_EditMethod_Fn warpInsPtLeft;
	static EV_EditMethod_Fn warpInsPtRight;
	static EV_EditMethod_Fn warpInsPtBOP;
	static EV_EditMethod_Fn warpInsPtEOP;
	static EV_EditMethod_Fn warpInsPtBOL;
	static EV_EditMethod_Fn warpInsPtEOL;
	static EV_EditMethod_Fn warpInsPtBOW;
	static EV_EditMethod_Fn warpInsPtEOW;
	static EV_EditMethod_Fn warpInsPtBOS;
	static EV_EditMethod_Fn warpInsPtEOS;
	static EV_EditMethod_Fn warpInsPtBOB;
	static EV_EditMethod_Fn warpInsPtEOB;
	static EV_EditMethod_Fn warpInsPtBOD;
	static EV_EditMethod_Fn warpInsPtEOD;
	static EV_EditMethod_Fn warpInsPtPrevPage;
	static EV_EditMethod_Fn warpInsPtNextPage;
	static EV_EditMethod_Fn warpInsPtPrevLine;
	static EV_EditMethod_Fn warpInsPtNextLine;

	static EV_EditMethod_Fn cursorDefault;
	static EV_EditMethod_Fn cursorIBeam;
	static EV_EditMethod_Fn cursorRightArrow;
	static EV_EditMethod_Fn cursorImage;
	static EV_EditMethod_Fn cursorImageSize;

	static EV_EditMethod_Fn contextMenu;
	static EV_EditMethod_Fn contextText;
	static EV_EditMethod_Fn contextMisspellText;

	static EV_EditMethod_Fn spellSuggest_1;
	static EV_EditMethod_Fn spellSuggest_2;
	static EV_EditMethod_Fn spellSuggest_3;
	static EV_EditMethod_Fn spellSuggest_4;
	static EV_EditMethod_Fn spellSuggest_5;
	static EV_EditMethod_Fn spellSuggest_6;
	static EV_EditMethod_Fn spellSuggest_7;
	static EV_EditMethod_Fn spellSuggest_8;
	static EV_EditMethod_Fn spellSuggest_9;

	static EV_EditMethod_Fn spellIgnoreAll;
	static EV_EditMethod_Fn spellAdd;

	static EV_EditMethod_Fn dragToXY;
	static EV_EditMethod_Fn dragToXYword;
	static EV_EditMethod_Fn endDrag;
	static EV_EditMethod_Fn extSelToXY;
	static EV_EditMethod_Fn extSelLeft;
	static EV_EditMethod_Fn extSelRight;
	static EV_EditMethod_Fn extSelBOL;
	static EV_EditMethod_Fn extSelEOL;
	static EV_EditMethod_Fn extSelBOW;
	static EV_EditMethod_Fn extSelEOW;
	static EV_EditMethod_Fn extSelBOS;
	static EV_EditMethod_Fn extSelEOS;
	static EV_EditMethod_Fn extSelBOB;
	static EV_EditMethod_Fn extSelEOB;
	static EV_EditMethod_Fn extSelBOD;
	static EV_EditMethod_Fn extSelEOD;
	static EV_EditMethod_Fn extSelPrevLine;
	static EV_EditMethod_Fn extSelNextLine;
	static EV_EditMethod_Fn extSelPageDown;
	static EV_EditMethod_Fn extSelPageUp;
	static EV_EditMethod_Fn selectAll;
	static EV_EditMethod_Fn selectWord;
	static EV_EditMethod_Fn selectLine;
	static EV_EditMethod_Fn selectBlock;

	static EV_EditMethod_Fn delLeft;
	static EV_EditMethod_Fn delRight;
	static EV_EditMethod_Fn delBOL;
	static EV_EditMethod_Fn delEOL;
	static EV_EditMethod_Fn delBOW;
	static EV_EditMethod_Fn delEOW;
	static EV_EditMethod_Fn delBOS;
	static EV_EditMethod_Fn delEOS;
	static EV_EditMethod_Fn delBOB;
	static EV_EditMethod_Fn delEOB;
	static EV_EditMethod_Fn delBOD;
	static EV_EditMethod_Fn delEOD;

	static EV_EditMethod_Fn insertData;
	static EV_EditMethod_Fn insertTab;
	static EV_EditMethod_Fn insertSoftBreak;
	static EV_EditMethod_Fn insertParagraphBreak;
	static EV_EditMethod_Fn insertSectionBreak;
	static EV_EditMethod_Fn insertLineBreak;
	static EV_EditMethod_Fn insertPageBreak;
	static EV_EditMethod_Fn insertColumnBreak;

	static EV_EditMethod_Fn insertSpace;
	static EV_EditMethod_Fn insertNBSpace;

	static EV_EditMethod_Fn insertGraveData; // for certain european keys
	static EV_EditMethod_Fn insertAcuteData;
	static EV_EditMethod_Fn insertCircumflexData;
	static EV_EditMethod_Fn insertTildeData;
	static EV_EditMethod_Fn insertMacronData;
	static EV_EditMethod_Fn insertBreveData;
	static EV_EditMethod_Fn insertAbovedotData;
	static EV_EditMethod_Fn insertDiaeresisData;
	static EV_EditMethod_Fn insertDoubleacuteData;
	static EV_EditMethod_Fn insertCaronData;
	static EV_EditMethod_Fn insertCedillaData;
	static EV_EditMethod_Fn insertOgonekData;

	static EV_EditMethod_Fn replaceChar;

	// TODO add functions for all of the standard menu commands.
	// TODO here are a few that i started.

	static EV_EditMethod_Fn fileNew;
	static EV_EditMethod_Fn fileOpen;
	static EV_EditMethod_Fn fileSave;
	static EV_EditMethod_Fn fileSaveAs;
	static EV_EditMethod_Fn pageSetup;
	static EV_EditMethod_Fn print;
	static EV_EditMethod_Fn printTB;
	static EV_EditMethod_Fn printPreview;
	static EV_EditMethod_Fn printDirectly;
	static EV_EditMethod_Fn fileInsertGraphic;
        static EV_EditMethod_Fn fileSaveAsWeb;
        static EV_EditMethod_Fn filePreviewWeb;

	static EV_EditMethod_Fn undo;
	static EV_EditMethod_Fn redo;
	static EV_EditMethod_Fn cut;
	static EV_EditMethod_Fn copy;
	static EV_EditMethod_Fn paste;
	static EV_EditMethod_Fn pasteSelection;
	static EV_EditMethod_Fn find;
	static EV_EditMethod_Fn findAgain;
	static EV_EditMethod_Fn go;
	static EV_EditMethod_Fn replace;
	static EV_EditMethod_Fn editHeader;
	static EV_EditMethod_Fn editFooter;

	static EV_EditMethod_Fn viewStd;
	static EV_EditMethod_Fn viewFormat;
	static EV_EditMethod_Fn viewExtra;
	static EV_EditMethod_Fn viewRuler;
	static EV_EditMethod_Fn viewStatus;
	static EV_EditMethod_Fn viewPara;
	static EV_EditMethod_Fn viewHeadFoot;
	static EV_EditMethod_Fn zoom;
	static EV_EditMethod_Fn dlgZoom;
        static EV_EditMethod_Fn viewFullScreen;

	static EV_EditMethod_Fn insBreak;
	static EV_EditMethod_Fn insPageNo;
	static EV_EditMethod_Fn insDateTime;
	static EV_EditMethod_Fn insField;
	static EV_EditMethod_Fn insSymbol;

	static EV_EditMethod_Fn dlgSpell;
	static EV_EditMethod_Fn dlgWordCount;
	static EV_EditMethod_Fn dlgOptions;
  
   	static EV_EditMethod_Fn dlgFont;
	static EV_EditMethod_Fn dlgParagraph;
	static EV_EditMethod_Fn dlgBullets;
	static EV_EditMethod_Fn dlgBorders;
	static EV_EditMethod_Fn dlgColumns;
	static EV_EditMethod_Fn style;
        static EV_EditMethod_Fn dlgBackground;
	static EV_EditMethod_Fn dlgStyle;
	static EV_EditMethod_Fn dlgTabs;
        static EV_EditMethod_Fn dlgToggleCase;
	static EV_EditMethod_Fn dlgLanguage;
	static EV_EditMethod_Fn language;
	static EV_EditMethod_Fn fontFamily;
	static EV_EditMethod_Fn fontSize;
	static EV_EditMethod_Fn toggleBold;
	static EV_EditMethod_Fn toggleItalic;
	static EV_EditMethod_Fn toggleUline;
	static EV_EditMethod_Fn toggleOline;
	static EV_EditMethod_Fn toggleStrike;
	static EV_EditMethod_Fn toggleSuper;
	static EV_EditMethod_Fn toggleSub;
	static EV_EditMethod_Fn togglePlain;
#ifdef BIDI_ENABLED
	static EV_EditMethod_Fn toggleDirOverrideLTR;
	static EV_EditMethod_Fn toggleDirOverrideRTL;
	static EV_EditMethod_Fn toggleDomDirection;
#endif
	static EV_EditMethod_Fn doBullets;
	static EV_EditMethod_Fn doNumbers;

        static EV_EditMethod_Fn colorForeTB;
        static EV_EditMethod_Fn colorBackTB;

        static EV_EditMethod_Fn toggleIndent;
        static EV_EditMethod_Fn toggleUnIndent;

	static EV_EditMethod_Fn alignLeft;
	static EV_EditMethod_Fn alignCenter;
	static EV_EditMethod_Fn alignRight;
	static EV_EditMethod_Fn alignJustify;

	static EV_EditMethod_Fn setStyleHeading1;
	static EV_EditMethod_Fn setStyleHeading2;
	static EV_EditMethod_Fn setStyleHeading3;

	static EV_EditMethod_Fn paraBefore0;
	static EV_EditMethod_Fn paraBefore12;

	static EV_EditMethod_Fn	sectColumns1;
	static EV_EditMethod_Fn	sectColumns2;
	static EV_EditMethod_Fn	sectColumns3;

	static EV_EditMethod_Fn singleSpace;
	static EV_EditMethod_Fn middleSpace;
	static EV_EditMethod_Fn doubleSpace;

	static EV_EditMethod_Fn openRecent_1;
	static EV_EditMethod_Fn openRecent_2;
	static EV_EditMethod_Fn openRecent_3;
	static EV_EditMethod_Fn openRecent_4;
	static EV_EditMethod_Fn openRecent_5;
	static EV_EditMethod_Fn openRecent_6;
	static EV_EditMethod_Fn openRecent_7;
	static EV_EditMethod_Fn openRecent_8;
	static EV_EditMethod_Fn openRecent_9;

	static EV_EditMethod_Fn activateWindow_1;
	static EV_EditMethod_Fn activateWindow_2;
	static EV_EditMethod_Fn activateWindow_3;
	static EV_EditMethod_Fn activateWindow_4;
	static EV_EditMethod_Fn activateWindow_5;
	static EV_EditMethod_Fn activateWindow_6;
	static EV_EditMethod_Fn activateWindow_7;
	static EV_EditMethod_Fn activateWindow_8;
	static EV_EditMethod_Fn activateWindow_9;
	static EV_EditMethod_Fn dlgMoreWindows;

	static EV_EditMethod_Fn dlgAbout;
	static EV_EditMethod_Fn helpContents;
	static EV_EditMethod_Fn helpIndex;
	static EV_EditMethod_Fn helpSearch;
	static EV_EditMethod_Fn helpCheckVer;
	static EV_EditMethod_Fn helpAboutOS;

	static EV_EditMethod_Fn newWindow;
	static EV_EditMethod_Fn cycleWindows;
	static EV_EditMethod_Fn cycleWindowsBck;
	static EV_EditMethod_Fn closeWindow;
	static EV_EditMethod_Fn querySaveAndExit;

	static EV_EditMethod_Fn setEditVI;
	static EV_EditMethod_Fn setInputVI;
	static EV_EditMethod_Fn cycleInputMode;
	static EV_EditMethod_Fn toggleInsertMode;

	static EV_EditMethod_Fn viCmd_A;
	static EV_EditMethod_Fn viCmd_C;
	static EV_EditMethod_Fn viCmd_I;
	static EV_EditMethod_Fn viCmd_J;
	static EV_EditMethod_Fn viCmd_O;
	static EV_EditMethod_Fn viCmd_P;
	static EV_EditMethod_Fn viCmd_a;
	static EV_EditMethod_Fn viCmd_o;
	static EV_EditMethod_Fn viCmd_c24;
	static EV_EditMethod_Fn viCmd_c28;
	static EV_EditMethod_Fn viCmd_c29;
	static EV_EditMethod_Fn viCmd_c5b;
	static EV_EditMethod_Fn viCmd_c5d;
	static EV_EditMethod_Fn viCmd_c5e;
	static EV_EditMethod_Fn viCmd_cb;
	static EV_EditMethod_Fn viCmd_cw;
	static EV_EditMethod_Fn viCmd_d24;
	static EV_EditMethod_Fn viCmd_d28;
	static EV_EditMethod_Fn viCmd_d29;
	static EV_EditMethod_Fn viCmd_d5b;
	static EV_EditMethod_Fn viCmd_d5d;
	static EV_EditMethod_Fn viCmd_d5e;
	static EV_EditMethod_Fn viCmd_db;
	static EV_EditMethod_Fn viCmd_dd;
	static EV_EditMethod_Fn viCmd_dw;
	static EV_EditMethod_Fn viCmd_y24;
	static EV_EditMethod_Fn viCmd_y28;
	static EV_EditMethod_Fn viCmd_y29;
	static EV_EditMethod_Fn viCmd_y5b;
	static EV_EditMethod_Fn viCmd_y5d;
	static EV_EditMethod_Fn viCmd_y5e;
	static EV_EditMethod_Fn viCmd_yb;
	static EV_EditMethod_Fn viCmd_yw;
	static EV_EditMethod_Fn viCmd_yy;

        static EV_EditMethod_Fn viewNormalLayout;
        static EV_EditMethod_Fn viewPrintLayout;
        static EV_EditMethod_Fn viewWebLayout;

        static EV_EditMethod_Fn toggleAutoSpell;

        static EV_EditMethod_Fn insAutotext_attn_1;
        static EV_EditMethod_Fn insAutotext_attn_2;

        static EV_EditMethod_Fn insAutotext_closing_1;
        static EV_EditMethod_Fn insAutotext_closing_2;
        static EV_EditMethod_Fn insAutotext_closing_3;
        static EV_EditMethod_Fn insAutotext_closing_4;
        static EV_EditMethod_Fn insAutotext_closing_5;
        static EV_EditMethod_Fn insAutotext_closing_6;
        static EV_EditMethod_Fn insAutotext_closing_7;
        static EV_EditMethod_Fn insAutotext_closing_8;
        static EV_EditMethod_Fn insAutotext_closing_9;
        static EV_EditMethod_Fn insAutotext_closing_10;
        static EV_EditMethod_Fn insAutotext_closing_11;
        static EV_EditMethod_Fn insAutotext_closing_12;

        static EV_EditMethod_Fn insAutotext_mail_1;
        static EV_EditMethod_Fn insAutotext_mail_2;
        static EV_EditMethod_Fn insAutotext_mail_3;
        static EV_EditMethod_Fn insAutotext_mail_4;
        static EV_EditMethod_Fn insAutotext_mail_5;
        static EV_EditMethod_Fn insAutotext_mail_6;
        static EV_EditMethod_Fn insAutotext_mail_7;
        static EV_EditMethod_Fn insAutotext_mail_8;

        static EV_EditMethod_Fn insAutotext_reference_1;
        static EV_EditMethod_Fn insAutotext_reference_2;
        static EV_EditMethod_Fn insAutotext_reference_3;

        static EV_EditMethod_Fn insAutotext_salutation_1;
        static EV_EditMethod_Fn insAutotext_salutation_2;
        static EV_EditMethod_Fn insAutotext_salutation_3;
        static EV_EditMethod_Fn insAutotext_salutation_4;

        static EV_EditMethod_Fn insAutotext_subject_1;

        static EV_EditMethod_Fn insAutotext_email_1;
        static EV_EditMethod_Fn insAutotext_email_2;
        static EV_EditMethod_Fn insAutotext_email_3;
        static EV_EditMethod_Fn insAutotext_email_4;
        static EV_EditMethod_Fn insAutotext_email_5;
        static EV_EditMethod_Fn insAutotext_email_6;

	static EV_EditMethod_Fn noop;

	// Test routines

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
	static EV_EditMethod_Fn Test_Dump;
	static EV_EditMethod_Fn Test_Ftr;
#endif
};

/*****************************************************************/
/*****************************************************************/

#define _D_				EV_EMT_REQUIREDATA

#define F(fn)			ap_EditMethods::fn
#define N(fn)			#fn
#define NF(fn)			N(fn), F(fn)

// keep this array alphabetically (strcmp) ordered under
// penalty of being forced to port Abi to the PalmOS

static EV_EditMethod s_arrayEditMethods[] =
{
#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
	EV_EditMethod(NF(Test_Dump),			0,	""),
	EV_EditMethod(NF(Test_Ftr),			0,	""),
#endif

	// a
	EV_EditMethod(NF(activateWindow_1),		0,		""),
	EV_EditMethod(NF(activateWindow_2),		0,		""),
	EV_EditMethod(NF(activateWindow_3),		0,		""),
	EV_EditMethod(NF(activateWindow_4),		0,		""),
	EV_EditMethod(NF(activateWindow_5),		0,		""),
	EV_EditMethod(NF(activateWindow_6),		0,		""),
	EV_EditMethod(NF(activateWindow_7),		0,		""),
	EV_EditMethod(NF(activateWindow_8),		0,		""),
	EV_EditMethod(NF(activateWindow_9),		0,		""),
	EV_EditMethod(NF(alignCenter),			0,		""),
	EV_EditMethod(NF(alignJustify),			0,		""),
	EV_EditMethod(NF(alignLeft),			0,		""),
	EV_EditMethod(NF(alignRight),			0,		""),

	// b

	// c
	EV_EditMethod(NF(closeWindow),			0,	""),
	EV_EditMethod(NF(colorBackTB), _D_, ""),
	EV_EditMethod(NF(colorForeTB), _D_, ""),
	EV_EditMethod(NF(contextMenu),			0,	""),
	EV_EditMethod(NF(contextMisspellText),	0,	""),
	EV_EditMethod(NF(contextText),			0,	""),
	EV_EditMethod(NF(copy),					0,	""),
	EV_EditMethod(NF(cursorDefault),		0,	""),
	EV_EditMethod(NF(cursorIBeam),			0,	""),
	EV_EditMethod(NF(cursorImage),			0,	""),
	EV_EditMethod(NF(cursorImageSize),		0,	""),
	EV_EditMethod(NF(cursorRightArrow),		0,	""),
	EV_EditMethod(NF(cut),					0,	""),
	EV_EditMethod(NF(cycleInputMode),		0,	""),
	EV_EditMethod(NF(cycleWindows),			0,	""),
	EV_EditMethod(NF(cycleWindowsBck),		0,	""),

	// d
	EV_EditMethod(NF(delBOB),				0,	""),
	EV_EditMethod(NF(delBOD),				0,	""),
	EV_EditMethod(NF(delBOL),				0,	""),
	EV_EditMethod(NF(delBOS),				0,	""),
	EV_EditMethod(NF(delBOW),				0,	""),
	EV_EditMethod(NF(delEOB),				0,	""),
	EV_EditMethod(NF(delEOD),				0,	""),
	EV_EditMethod(NF(delEOL),				0,	""),
	EV_EditMethod(NF(delEOS),				0,	""),
	EV_EditMethod(NF(delEOW),				0,	""),
	EV_EditMethod(NF(delLeft),				0,	""),
	EV_EditMethod(NF(delRight),				0,	""),
	EV_EditMethod(NF(dlgAbout),				0,	""),
	EV_EditMethod(NF(dlgBackground),                0,              ""),
	EV_EditMethod(NF(dlgBorders),			0,		""),
	EV_EditMethod(NF(dlgBullets),			0,		""),
	EV_EditMethod(NF(dlgColumns),			0,		""),
	EV_EditMethod(NF(dlgFont),			0,		""),
	EV_EditMethod(NF(dlgLanguage),			0,		""),
	EV_EditMethod(NF(dlgMoreWindows),		0,		""),
	EV_EditMethod(NF(dlgOptions),			0,	        ""),
	EV_EditMethod(NF(dlgParagraph),			0,		""),
	EV_EditMethod(NF(dlgSpell),			0,		""),
	EV_EditMethod(NF(dlgStyle),		       	0,		""),
	EV_EditMethod(NF(dlgTabs),		       	0,		""),
	EV_EditMethod(NF(dlgToggleCase),                0,              ""),
	EV_EditMethod(NF(dlgWordCount),			0,		""),
	EV_EditMethod(NF(dlgZoom),		       	0,		""),
	EV_EditMethod(NF(doBullets),			0,		""),
	EV_EditMethod(NF(doNumbers),			0,		""),
	EV_EditMethod(NF(doubleSpace),			0,		""),
	EV_EditMethod(NF(dragToXY),				0,	""),
	EV_EditMethod(NF(dragToXYword),			0,	""),

	// e
	EV_EditMethod(NF(editFooter),			0,	""),
	EV_EditMethod(NF(editHeader),			0,	""),
	EV_EditMethod(NF(endDrag),			0,	""),
	EV_EditMethod(NF(extSelBOB),			0,	""),
	EV_EditMethod(NF(extSelBOD),			0,	""),
	EV_EditMethod(NF(extSelBOL),			0,	""),
	EV_EditMethod(NF(extSelBOS),			0,	""),
	EV_EditMethod(NF(extSelBOW),			0,	""),
	EV_EditMethod(NF(extSelEOB),			0,	""),
	EV_EditMethod(NF(extSelEOD),			0,	""),
	EV_EditMethod(NF(extSelEOL),			0,	""),
	EV_EditMethod(NF(extSelEOS),			0,	""),
	EV_EditMethod(NF(extSelEOW),			0,	""),
	EV_EditMethod(NF(extSelLeft),			0,	""),
	EV_EditMethod(NF(extSelNextLine),		0,	""),
	EV_EditMethod(NF(extSelPageDown),		0,	""),
	EV_EditMethod(NF(extSelPageUp),			0,	""),
	EV_EditMethod(NF(extSelPrevLine),		0,	""),
	EV_EditMethod(NF(extSelRight),			0,	""),
	EV_EditMethod(NF(extSelToXY),			0,	""),

	// f
	EV_EditMethod(NF(fileInsertGraphic),	0,	""),
	EV_EditMethod(NF(fileNew),				0,	""),
	EV_EditMethod(NF(fileOpen),				0,	""),
	EV_EditMethod(NF(filePreviewWeb), 0, ""),
	EV_EditMethod(NF(fileSave),				0,	""),
	EV_EditMethod(NF(fileSaveAs),			0,	""),
	EV_EditMethod(NF(fileSaveAsWeb),                0, ""),
	EV_EditMethod(NF(find),					0,	""),
	EV_EditMethod(NF(findAgain),			0,	""),	
	EV_EditMethod(NF(fontFamily),			_D_,	""),
	EV_EditMethod(NF(fontSize),				_D_,	""),

	// g
	EV_EditMethod(NF(go),					0,	""),

	// h
	EV_EditMethod(NF(helpAboutOS),			0,		""),
	EV_EditMethod(NF(helpCheckVer),			0,		""),
	EV_EditMethod(NF(helpContents),			0,		""),
	EV_EditMethod(NF(helpIndex),			0,		""),
	EV_EditMethod(NF(helpSearch),			0,		""),
	
	// i
	EV_EditMethod(NF(insAutotext_attn_1), 0, ""),
	EV_EditMethod(NF(insAutotext_attn_2), 0, ""),
	EV_EditMethod(NF(insAutotext_closing_1), 0, ""),
	EV_EditMethod(NF(insAutotext_closing_10), 0, ""),
	EV_EditMethod(NF(insAutotext_closing_11), 0, ""),
	EV_EditMethod(NF(insAutotext_closing_12), 0, ""),
	EV_EditMethod(NF(insAutotext_closing_2), 0, ""),
	EV_EditMethod(NF(insAutotext_closing_3), 0, ""),
	EV_EditMethod(NF(insAutotext_closing_4), 0, ""),
	EV_EditMethod(NF(insAutotext_closing_5), 0, ""),
	EV_EditMethod(NF(insAutotext_closing_6), 0, ""),
	EV_EditMethod(NF(insAutotext_closing_7), 0, ""),
	EV_EditMethod(NF(insAutotext_closing_8), 0, ""),
	EV_EditMethod(NF(insAutotext_closing_9), 0, ""),
	EV_EditMethod(NF(insAutotext_email_1), 0, ""),
	EV_EditMethod(NF(insAutotext_email_2), 0, ""),
	EV_EditMethod(NF(insAutotext_email_3), 0, ""),
	EV_EditMethod(NF(insAutotext_email_4), 0, ""),
	EV_EditMethod(NF(insAutotext_email_5), 0, ""),
	EV_EditMethod(NF(insAutotext_email_6), 0, ""),
	EV_EditMethod(NF(insAutotext_mail_1), 0, ""),
	EV_EditMethod(NF(insAutotext_mail_2), 0, ""),
	EV_EditMethod(NF(insAutotext_mail_3), 0, ""),
	EV_EditMethod(NF(insAutotext_mail_4), 0, ""),
	EV_EditMethod(NF(insAutotext_mail_5), 0, ""),
	EV_EditMethod(NF(insAutotext_mail_6), 0, ""),
	EV_EditMethod(NF(insAutotext_mail_7), 0, ""),
	EV_EditMethod(NF(insAutotext_mail_8), 0, ""),
	EV_EditMethod(NF(insAutotext_reference_1), 0, ""),
	EV_EditMethod(NF(insAutotext_reference_2), 0, ""),
	EV_EditMethod(NF(insAutotext_reference_3), 0, ""),
	EV_EditMethod(NF(insAutotext_salutation_1), 0, ""),
	EV_EditMethod(NF(insAutotext_salutation_2), 0, ""),
	EV_EditMethod(NF(insAutotext_salutation_3), 0, ""),
	EV_EditMethod(NF(insAutotext_salutation_4), 0, ""),
	EV_EditMethod(NF(insAutotext_subject_1), 0, ""),
	EV_EditMethod(NF(insBreak),			0,		""),
	EV_EditMethod(NF(insDateTime),			0,		""),
	EV_EditMethod(NF(insField),			0,		""),
	EV_EditMethod(NF(insPageNo),			0,		""),
	EV_EditMethod(NF(insSymbol),			0,		""),
	EV_EditMethod(NF(insertAbovedotData),	_D_,	""),
	EV_EditMethod(NF(insertAcuteData),		_D_,	""),
	EV_EditMethod(NF(insertBreveData),		_D_,	""),
	EV_EditMethod(NF(insertCaronData),		_D_,	""),
	EV_EditMethod(NF(insertCedillaData),	_D_,	""),
	EV_EditMethod(NF(insertCircumflexData),	_D_,	""),
	EV_EditMethod(NF(insertColumnBreak),	0,	""),
	EV_EditMethod(NF(insertData),			_D_,	""),
	EV_EditMethod(NF(insertDiaeresisData),	_D_,	""),
	EV_EditMethod(NF(insertDoubleacuteData),_D_,	""),
	EV_EditMethod(NF(insertGraveData),		_D_,	""),
	EV_EditMethod(NF(insertLineBreak),		0,	""),
	EV_EditMethod(NF(insertMacronData),		_D_,	""),
	EV_EditMethod(NF(insertNBSpace),		0,	""),
	EV_EditMethod(NF(insertOgonekData),		_D_,	""),
	EV_EditMethod(NF(insertPageBreak),		0,	""),
	EV_EditMethod(NF(insertParagraphBreak),	0,	""),
	EV_EditMethod(NF(insertSectionBreak),	0,	""),
	EV_EditMethod(NF(insertSoftBreak),		0,	""),
	EV_EditMethod(NF(insertSpace),			0,	""),
	EV_EditMethod(NF(insertTab),			0,	""),
	EV_EditMethod(NF(insertTildeData),		_D_,	""),

	// j

	// k

	// l
	EV_EditMethod(NF(language),			0,	""),

	// m
	EV_EditMethod(NF(middleSpace),			0,		""),

	// n
	EV_EditMethod(NF(newWindow),			0,	""),
	EV_EditMethod(NF(noop),					0,	""),

	// o
	EV_EditMethod(NF(openRecent_1),			0,		""),
	EV_EditMethod(NF(openRecent_2),			0,		""),
	EV_EditMethod(NF(openRecent_3),			0,		""),
	EV_EditMethod(NF(openRecent_4),			0,		""),
	EV_EditMethod(NF(openRecent_5),			0,		""),
	EV_EditMethod(NF(openRecent_6),			0,		""),
	EV_EditMethod(NF(openRecent_7),			0,		""),
	EV_EditMethod(NF(openRecent_8),			0,		""),
	EV_EditMethod(NF(openRecent_9),			0,		""),

	// p
	EV_EditMethod(NF(pageSetup),			0,	""),
	EV_EditMethod(NF(paraBefore0),			0,		""),
	EV_EditMethod(NF(paraBefore12),			0,		""),
		// intended for ^V and Menu[Edit/Paste]
	EV_EditMethod(NF(paste),				0,	""),
	        // intended for X11 middle mouse
	EV_EditMethod(NF(pasteSelection),		0,	""),
	EV_EditMethod(NF(print),				0,	""),
	EV_EditMethod(NF(printDirectly), 0, ""),
	EV_EditMethod(NF(printPreview), 0, ""),
	EV_EditMethod(NF(printTB),				0,	""),

	// q
	EV_EditMethod(NF(querySaveAndExit),		0,	""),

	// r
	EV_EditMethod(NF(redo),					0,	""),
	EV_EditMethod(NF(replace),				0,	""),
	EV_EditMethod(NF(replaceChar),			_D_,	""),

	// s
	EV_EditMethod(NF(scrollLineDown),		0,	""),
	EV_EditMethod(NF(scrollLineLeft),		0,	""),
	EV_EditMethod(NF(scrollLineRight),		0,	""),
	EV_EditMethod(NF(scrollLineUp),			0,	""),
	EV_EditMethod(NF(scrollPageDown),		0,	""),
	EV_EditMethod(NF(scrollPageLeft),		0,	""),
	EV_EditMethod(NF(scrollPageRight),		0,	""),
	EV_EditMethod(NF(scrollPageUp),			0,	""),
	EV_EditMethod(NF(scrollToBottom),		0,	""),
	EV_EditMethod(NF(scrollToTop),			0,	""),
	EV_EditMethod(NF(sectColumns1),			0,		""),
	EV_EditMethod(NF(sectColumns2),			0,		""),
	EV_EditMethod(NF(sectColumns3),			0,		""),
	EV_EditMethod(NF(selectAll),			0,	""),
	EV_EditMethod(NF(selectBlock),			0,	""),   
	EV_EditMethod(NF(selectLine),			0,	""),
	EV_EditMethod(NF(selectWord),			0,	""),
	EV_EditMethod(NF(setEditVI),			0,	""),
	EV_EditMethod(NF(setInputVI),			0,	""),
	EV_EditMethod(NF(setStyleHeading1),		0,		""),
	EV_EditMethod(NF(setStyleHeading2),		0,		""),
	EV_EditMethod(NF(setStyleHeading3),		0,		""),
	EV_EditMethod(NF(singleSpace),			0,		""),
	EV_EditMethod(NF(spellAdd),				0,	""),
	EV_EditMethod(NF(spellIgnoreAll),		0,	""),
	EV_EditMethod(NF(spellSuggest_1),		0,	""),
	EV_EditMethod(NF(spellSuggest_2),		0,	""),
	EV_EditMethod(NF(spellSuggest_3),		0,	""),
	EV_EditMethod(NF(spellSuggest_4),		0,	""),
	EV_EditMethod(NF(spellSuggest_5),		0,	""),
	EV_EditMethod(NF(spellSuggest_6),		0,	""),
	EV_EditMethod(NF(spellSuggest_7),		0,	""),
	EV_EditMethod(NF(spellSuggest_8),		0,	""),
	EV_EditMethod(NF(spellSuggest_9),		0,	""),
	EV_EditMethod(NF(style),				_D_,	""),

	// t
	EV_EditMethod(NF(toggleAutoSpell), 0, ""),
	EV_EditMethod(NF(toggleBold),			0,		""),
#ifdef BIDI_ENABLED
	EV_EditMethod(NF(toggleDirOverrideLTR),		0,		""),
	EV_EditMethod(NF(toggleDirOverrideRTL),		0,		""),
	EV_EditMethod(NF(toggleDomDirection),	0,		""),
#endif
	EV_EditMethod(NF(toggleIndent), 0, ""),
	EV_EditMethod(NF(toggleInsertMode),         0,  ""),
	EV_EditMethod(NF(toggleItalic),			0,		""),
	EV_EditMethod(NF(toggleOline),			0,		""),
	EV_EditMethod(NF(togglePlain),			0,		""),
	EV_EditMethod(NF(toggleStrike),			0,		""),
	EV_EditMethod(NF(toggleSub),			0,		""),
	EV_EditMethod(NF(toggleSuper),			0,		""),
	EV_EditMethod(NF(toggleUline),			0,		""),
	EV_EditMethod(NF(toggleUnIndent), 0, ""),

	// u
	EV_EditMethod(NF(undo),					0,	""),

	// v
	EV_EditMethod(NF(viCmd_A),		0,	""),
	EV_EditMethod(NF(viCmd_C),		0,	""),
	EV_EditMethod(NF(viCmd_I),		0,	""),
	EV_EditMethod(NF(viCmd_J),		0,	""),
	EV_EditMethod(NF(viCmd_O),		0,	""),
	EV_EditMethod(NF(viCmd_P),		0,	""),
	EV_EditMethod(NF(viCmd_a),		0,	""),
	EV_EditMethod(NF(viCmd_o),		0,	""),
	EV_EditMethod(NF(viCmd_c24),	0,	""),
	EV_EditMethod(NF(viCmd_c28),	0,	""),
	EV_EditMethod(NF(viCmd_c29),	0,	""),
	EV_EditMethod(NF(viCmd_c5b),	0,	""),
	EV_EditMethod(NF(viCmd_c5d),	0,	""),
	EV_EditMethod(NF(viCmd_c5e),	0,	""),
	EV_EditMethod(NF(viCmd_cb),		0,	""),
	EV_EditMethod(NF(viCmd_cw),		0,	""),
	EV_EditMethod(NF(viCmd_d24),		0,	""),
	EV_EditMethod(NF(viCmd_d28),		0,	""),
	EV_EditMethod(NF(viCmd_d29),		0,	""),
	EV_EditMethod(NF(viCmd_d5b),		0,	""),
	EV_EditMethod(NF(viCmd_d5d),		0,	""),
	EV_EditMethod(NF(viCmd_d5e),		0,	""),
	EV_EditMethod(NF(viCmd_db),		0,	""),
	EV_EditMethod(NF(viCmd_dd),		0,	""),
	EV_EditMethod(NF(viCmd_dw),		0,	""),
	EV_EditMethod(NF(viCmd_y24),	0,	""),
	EV_EditMethod(NF(viCmd_y28),	0,	""),
	EV_EditMethod(NF(viCmd_y29),	0,	""),
	EV_EditMethod(NF(viCmd_y5b),	0,	""),
	EV_EditMethod(NF(viCmd_y5d),	0,	""),
	EV_EditMethod(NF(viCmd_y5e),	0,	""),
	EV_EditMethod(NF(viCmd_yb),		0,	""),
	EV_EditMethod(NF(viCmd_yw),		0,	""),
	EV_EditMethod(NF(viCmd_yy),		0,	""),
	EV_EditMethod(NF(viewExtra),			0,		""),
	EV_EditMethod(NF(viewFormat),			0,		""),
	EV_EditMethod(NF(viewFullScreen), 0, ""),
	EV_EditMethod(NF(viewHeadFoot),			0,		""),
	EV_EditMethod(NF(viewNormalLayout), 0, ""),
	EV_EditMethod(NF(viewPara),			0,		""),
	EV_EditMethod(NF(viewPrintLayout), 0, ""),
	EV_EditMethod(NF(viewRuler),			0,		""),
	EV_EditMethod(NF(viewStatus),			0,		""),
	EV_EditMethod(NF(viewStd),			0,		""),
	EV_EditMethod(NF(viewWebLayout), 0, ""),

	// w
	EV_EditMethod(NF(warpInsPtBOB),			0,	""),
	EV_EditMethod(NF(warpInsPtBOD),			0,	""),
	EV_EditMethod(NF(warpInsPtBOL),			0,	""),
	EV_EditMethod(NF(warpInsPtBOP),			0,	""),
	EV_EditMethod(NF(warpInsPtBOS),			0,	""),
	EV_EditMethod(NF(warpInsPtBOW),			0,	""),
	EV_EditMethod(NF(warpInsPtEOB),			0,	""),
	EV_EditMethod(NF(warpInsPtEOD),			0,	""),
	EV_EditMethod(NF(warpInsPtEOL),			0,	""),
	EV_EditMethod(NF(warpInsPtEOP),			0,	""),
	EV_EditMethod(NF(warpInsPtEOS),			0,	""),
	EV_EditMethod(NF(warpInsPtEOW),			0,	""),
	EV_EditMethod(NF(warpInsPtLeft),		0,	""),
	EV_EditMethod(NF(warpInsPtNextLine),	0,	""),
	EV_EditMethod(NF(warpInsPtNextPage),	0,	""),
	EV_EditMethod(NF(warpInsPtPrevLine),	0,	""),
	EV_EditMethod(NF(warpInsPtPrevPage),	0,	""),
	EV_EditMethod(NF(warpInsPtRight),		0,	""),
	EV_EditMethod(NF(warpInsPtToXY),		0,	""),

	// x

	// y

	// z
	EV_EditMethod(NF(zoom),			        0,     	"")
};



EV_EditMethodContainer * AP_GetEditMethods(void)
{
       // Construct a container for all of the methods this application
       // knows about.

       return new EV_EditMethodContainer(NrElements(s_arrayEditMethods),s_arrayEditMethods);
}

#undef _D_
#undef F
#undef N
#undef NF

/*****************************************************************/
/*****************************************************************/

#define F(fn)		ap_EditMethods::fn
#define Defun(fn)	bool F(fn)(AV_View*   pAV_View,   EV_EditMethodCallData *   pCallData  )
#define Defun0(fn)	bool F(fn)(AV_View* /*pAV_View*/, EV_EditMethodCallData * /*pCallData*/)
#define Defun1(fn)	bool F(fn)(AV_View*   pAV_View,   EV_EditMethodCallData * /*pCallData*/)
#define EX(fn)		F(fn)(pAV_View, pCallData)

// forward declaration
bool _helpOpenURL(AV_View* pAV_View, const char* helpURL);

Defun1(toggleAutoSpell)
{
        XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);

	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	UT_ASSERT(pPrefsScheme);

	bool b = false;

	pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_AutoSpellCheck, &b);
	return pPrefsScheme->setValueBool((XML_Char*)AP_PREF_KEY_AutoSpellCheck, !b);
}

Defun1(scrollPageDown)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGEDOWN);

	return true;
}

Defun1(scrollPageUp)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGEUP);

	return true;
}

Defun1(scrollPageLeft)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGELEFT);

	return true;
}

Defun1(scrollPageRight)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGERIGHT);

	return true;
}

Defun1(scrollLineDown)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_LINEDOWN);

	return true;
}

Defun1(scrollLineUp)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_LINEUP);

	return true;
}

Defun1(scrollLineLeft)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_LINELEFT);

	return true;
}

Defun1(scrollLineRight)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_LINERIGHT);

	return true;
}

Defun1(scrollToTop)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_TOTOP);

	return true;
}

Defun1(scrollToBottom)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_TOBOTTOM);

	return true;
}

Defun1(fileNew)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	XAP_Frame * pNewFrame = pApp->newFrame();

	if (pNewFrame)
		pFrame = pNewFrame;

	// the IEFileType here doesn't really matter, since the name is NULL
	UT_Error error = pFrame->loadDocument(NULL, IEFT_Unknown);

	if (pNewFrame)
        {
		pNewFrame->show();
	}
	return E2B(error);
}

/*****************************************************************/
/*****************************************************************/

// TODO i've pulled the code to compose a question in a message
// TODO box into these little s_Ask*() functions.  part of this
// TODO is to isolate the question asking from the code which
// TODO decides what to do with the answer.  but also to see if
// TODO we want to abstract things further and make us think about
// TODO localization of the question strings....

static void s_TellSaveFailed(XAP_Frame * pFrame, const char * fileName, UT_Error errorCode)
{
	XAP_String_Id String_id;
	
	if (errorCode == -201) // We have a write error
	  String_id = AP_STRING_ID_MSG_SaveFailedWrite;

	else if (errorCode == -202) // We have a name error
	  String_id = AP_STRING_ID_MSG_SaveFailedName;

	else if (errorCode == -203) // We have an export error
	  String_id = AP_STRING_ID_MSG_SaveFailedExport;
	
	else // The generic case - should be eliminated eventually
	  String_id = AP_STRING_ID_MSG_SaveFailed;

	pFrame->showMessageBox(String_id,
								XAP_Dialog_MessageBox::b_O,
								XAP_Dialog_MessageBox::a_OK,
								fileName);
}

static void s_TellSpellDone(XAP_Frame * pFrame)
{
	pFrame->showMessageBox(AP_STRING_ID_MSG_SpellDone,
								XAP_Dialog_MessageBox::b_O,
								XAP_Dialog_MessageBox::a_OK);
}

static void s_TellNotImplemented(XAP_Frame * pFrame, const char * szWhat, int iLine)
{
	const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
	const char *p_message = pSS->getValue(AP_STRING_ID_MSG_DlgNotImp);

	UT_uint32 joinedSize = strlen(p_message) + strlen(szWhat) + strlen(__FILE__) + 4 + 10; // The 4 is for the line #
	char *szMessage = (char *)malloc(joinedSize * sizeof(char));
	if (!szMessage)
	{
		UT_DEBUGMSG(("Could not allocate string for [%s %s %s %d]\n", p_message, szWhat));
		return;
	}

	sprintf(szMessage, p_message, szWhat, __FILE__, iLine);

	pFrame->showMessageBox(szMessage,
							XAP_Dialog_MessageBox::b_O,
							XAP_Dialog_MessageBox::a_OK);

	FREEP(szMessage);
}

static bool s_AskRevertFile(XAP_Frame * pFrame)
{
	// return true if we should revert the file (back to the saved copy).

	return (pFrame->showMessageBox(AP_STRING_ID_MSG_RevertBuffer,
										XAP_Dialog_MessageBox::b_YN,
										XAP_Dialog_MessageBox::a_YES,
										pFrame->getFilename())
						== XAP_Dialog_MessageBox::a_YES);
}

static bool s_AskCloseAllAndExit(XAP_Frame * pFrame)
{
	// return true if we should quit.
	return (pFrame->showMessageBox(AP_STRING_ID_MSG_QueryExit,
										XAP_Dialog_MessageBox::b_YN,
										XAP_Dialog_MessageBox::a_NO)
						== XAP_Dialog_MessageBox::a_YES);

}

static XAP_Dialog_MessageBox::tAnswer s_AskSaveFile(XAP_Frame * pFrame)
{
	return pFrame->showMessageBox(AP_STRING_ID_MSG_ConfirmSave,
										XAP_Dialog_MessageBox::b_YNC,
										XAP_Dialog_MessageBox::a_YES,
										pFrame->getTitle(200));
}

#define BAD_FILETYPE ((UT_sint32)(XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO) - 1)

static bool s_AskForPathname(XAP_Frame * pFrame,
								bool bSaveAs,
								const char * pSuggestedName,
								char ** ppPathname,
								IEFileType * ieft)
{
	// raise the file-open or file-save-as dialog.
	// return a_OK or a_CANCEL depending on which button
	// the user hits.
	// return a pointer a UT_strdup()'d string containing the
	// pathname the user entered -- ownership of this goes
	// to the caller (so free it when you're done with it).

	UT_DEBUGMSG(("s_AskForPathname: frame %p, bSaveAs %ld, suggest=[%s]\n",
				 pFrame,bSaveAs,((pSuggestedName) ? pSuggestedName : "")));

	UT_ASSERT(ppPathname);
	*ppPathname = NULL;

	pFrame->raise();

	XAP_Dialog_Id id = ((bSaveAs) ? XAP_DIALOG_ID_FILE_SAVEAS : XAP_DIALOG_ID_FILE_OPEN);

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	if (pSuggestedName && *pSuggestedName)
	{
		// if caller wants to suggest a name, use it and seed the
		// dialog in that directory and set the filename.
		pDialog->setCurrentPathname(pSuggestedName);
		pDialog->setSuggestFilename(true);
	}
	else
	{
		// if caller does not want to suggest a name, seed the dialog
		// to the directory containing this document (if it has a
		// name), but don't put anything in the filename portion.
		pDialog->setCurrentPathname(pFrame->getFilename());
		pDialog->setSuggestFilename(false);
	}

	// to fill the file types popup list, we need to convert
	// AP-level Imp/Exp descriptions, suffixes, and types into
	// strings.

	UT_uint32 filterCount = 0;

	if (bSaveAs)
		filterCount = IE_Exp::getExporterCount();
	else
		filterCount = IE_Imp::getImporterCount();
	
	const char ** szDescList = (const char **) calloc(filterCount + 1,
													  sizeof(char *));
	const char ** szSuffixList = (const char **) calloc(filterCount + 1,
														sizeof(char *));
	IEFileType * nTypeList = (IEFileType *) calloc(filterCount + 1,
												   sizeof(IEFileType));
	UT_uint32 k = 0;

	if (bSaveAs)
		while (IE_Exp::enumerateDlgLabels(k, &szDescList[k], &szSuffixList[k], &nTypeList[k]))
			k++;
	else
		while (IE_Imp::enumerateDlgLabels(k, &szDescList[k], &szSuffixList[k], &nTypeList[k]))
			k++;

	pDialog->setFileTypeList(szDescList, szSuffixList, (const UT_sint32 *) nTypeList);

	// AbiWord uses IEFT_AbiWord_1 as the default

	// try to remember the previous file type
	static UT_sint32 dflFileType = BAD_FILETYPE;

	if (*ieft != BAD_FILETYPE)
	  dflFileType = (UT_sint32) *ieft;
	else
	  dflFileType = (UT_sint32) IEFT_AbiWord_1;

	pDialog->setDefaultFileType(dflFileType);
		
	pDialog->runModal(pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const char * szResultPathname = pDialog->getPathname();
		if (szResultPathname && *szResultPathname)
			UT_cloneString(*ppPathname,szResultPathname);

		UT_sint32 type = pDialog->getFileType();
		dflFileType = type;

		// If the number is negative, it's a special type.
		// Some operating systems which depend solely on filename
		// suffixes to indentify type (like Windows) will always
		// want auto-detection.
		if (type < 0)
			switch (type)
			{
			case XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO:
				// do some automagical detecting
				*ieft = IEFT_Unknown;
				break;
			default:
				// it returned a type we don't know how to handle
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		else
			*ieft = (IEFileType) pDialog->getFileType();
	}

	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);
	
	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

static bool s_AskForGraphicPathname(XAP_Frame * pFrame,
				       char ** ppPathname,
				       IEGraphicFileType * iegft)
{
	// raise the file-open dialog for inserting an image.
	// return a_OK or a_CANCEL depending on which button
	// the user hits.
	// return a pointer a UT_strdup()'d string containing the
	// pathname the user entered -- ownership of this goes
	// to the caller (so free it when you're done with it).

	UT_DEBUGMSG(("s_AskForGraphicPathname: frame %p\n",
				 pFrame));

	UT_ASSERT(ppPathname);
	*ppPathname = NULL;

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)
		      (pDialogFactory->requestDialog(XAP_DIALOG_ID_INSERT_PICTURE));
	UT_ASSERT(pDialog);

	pDialog->setCurrentPathname(NULL);
	pDialog->setSuggestFilename(false);

	// to fill the file types popup list, we need to convert AP-level
	// ImpGraphic descriptions, suffixes, and types into strings.

	UT_uint32 filterCount = IE_ImpGraphic::getImporterCount();
	
	const char ** szDescList = (const char **) calloc(filterCount + 1,
							  sizeof(char *));
 	const char ** szSuffixList = (const char **) calloc(filterCount + 1,
							    sizeof(char *));
	IEGraphicFileType * nTypeList = (IEGraphicFileType *)
		 calloc(filterCount + 1,	sizeof(IEGraphicFileType));
	UT_uint32 k = 0;

	while (IE_ImpGraphic::enumerateDlgLabels(k, &szDescList[k], &szSuffixList[k], &nTypeList[k]))
		k++;

	pDialog->setFileTypeList(szDescList, szSuffixList, (const UT_sint32 *) nTypeList);
	
	pDialog->runModal(pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const char * szResultPathname = pDialog->getPathname();
		if (szResultPathname && *szResultPathname)
			UT_cloneString(*ppPathname,szResultPathname);

		UT_sint32 type = pDialog->getFileType();

		// If the number is negative, it's a special type.
		// Some operating systems which depend solely on filename
		// suffixes to indentify type (like Windows) will always
		// want auto-detection.
		if (type < 0)
			switch (type)
			{
			case XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO:
				// do some automagical detecting
				*iegft = IEGFT_Unknown;
				break;
			default:
				// it returned a type we don't know how to handle
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		else
			*iegft = (IEGraphicFileType) pDialog->getFileType();
	}

	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);
	
	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

/*****************************************************************/
/*****************************************************************/

static XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode)
{
	XAP_String_Id String_id;

	switch (errorCode)
	  {
	  case -301:
	    String_id = AP_STRING_ID_MSG_IE_FileNotFound;
	    break;

	  case -302:
	    String_id = AP_STRING_ID_MSG_IE_NoMemory;
	    break;

	  case -303:
	    String_id = AP_STRING_ID_MSG_IE_UnsupportedType;
	    //AP_STRING_ID_MSG_IE_UnknownType;
	    break;

	  case -304:
	    String_id = AP_STRING_ID_MSG_IE_BogusDocument;
	    break;

	  case -305:
	    String_id = AP_STRING_ID_MSG_IE_CouldNotOpen;
	    break;

	  case -306:
	    String_id = AP_STRING_ID_MSG_IE_CouldNotWrite;
	    break;

	  case -307:
	    String_id = AP_STRING_ID_MSG_IE_FakeType;
	    break;

	  case -311:
	    String_id = AP_STRING_ID_MSG_IE_UnsupportedType;
	    break;

	  default:
	    String_id = AP_STRING_ID_MSG_ImportError;
	  }

	return pFrame->showMessageBox(String_id,
									XAP_Dialog_MessageBox::b_O,
									XAP_Dialog_MessageBox::a_OK,
									pNewFile);
}

UT_Error fileOpen(XAP_Frame * pFrame, const char * pNewFile, IEFileType ieft)
{
	UT_DEBUGMSG(("fileOpen: loading [%s]\n",pNewFile));
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);

	XAP_Frame * pNewFrame = NULL;
	// not needed bool bRes = false;
	UT_Error errorCode = UT_IE_IMPORTERROR;

	// see if requested file is already open in another frame
	UT_sint32 ndx = pApp->findFrame(pNewFile);
	if (ndx >= 0)
	{
		// yep, reuse it
		pNewFrame = pApp->getFrame(ndx);
		UT_ASSERT(pNewFrame);

		if (s_AskRevertFile(pNewFrame))
		{
			// re-load the document in pNewFrame

			errorCode = pNewFrame->loadDocument(pNewFile, ieft);
			if (!errorCode)
			{
				pNewFrame->show();
				pPrefs->addRecent(pNewFile);
			}
			else
			{
				s_CouldNotLoadFileMessage(pNewFrame,pNewFile, errorCode);
			}
		}
		else
		{
			// cancel the FileOpen.
			errorCode = UT_OK;		// don't remove from recent list
		}
		
		return errorCode;
	}

	// We generally open documents in a new frame, which keeps the
	// contents of the current frame available.
	// However, as a convenience we do replace the contents of the
	// current frame if it's the only top-level view on an empty,
	// untitled document.

	if (pFrame->isDirty() || pFrame->getFilename() || (pFrame->getViewNumber() > 0))
	{
		// open new document in a new frame.  if we fail,
		// put up an error dialog on current frame (our
		// new one is not completely instantiated) and
		// return.  we do not create a new untitled document
		// in this case.

		pNewFrame = pApp->newFrame();
		if (!pNewFrame)
		{
			return false;
		}
		
		errorCode = pNewFrame->loadDocument(pNewFile, ieft);
		if (!errorCode)
		{
			pNewFrame->show();
			pPrefs->addRecent(pNewFile);
		}
		else
		{
			// TODO there is a problem with the way we create a
			// TODO new frame and then load a documentent into
			// TODO it.  if we try to load pNewFile and fail,
			// TODO and then destroy the window, and raise a
			// TODO message box (on the original window) we get
			// TODO nasty race on UNIX.  raising the dialog and
			// TODO waiting for input flushes out the show-windows
			// TODO on the new (and not yet completely instantiated)
			// TODO window.  this causes a view-less top-level
			// TODO window to appear -- which causes lots of
			// TODO expose-related problems... and then other
			// TODO problems which appear to be related to having
			// TODO multiple gtk_main()'s on the stack....
			// TODO
			// TODO for now, we force a new untitled document into
			// TODO the new window and then raise the message on
			// TODO this new window.
			// TODO
			// TODO long term, we may want to modified pApp->newFrame()
			// TODO to take an 'bool bShowWindow' argument....

			// the IEFileType here doesn't really matter since the file name is NULL
			errorCode = pNewFrame->loadDocument(NULL, IEFT_Unknown);
			if (!errorCode)
				pNewFrame->show();
			s_CouldNotLoadFileMessage(pNewFrame,pNewFile, errorCode);
		}
		
		return errorCode;
	}

	// we are replacing the single-view, unmodified, untitled document.
	// if we fail, put up an error message on the current frame
	// and return -- we do not replace this untitled document with a
	// new untitled document.

	errorCode = pFrame->loadDocument(pNewFile, ieft);
	if (!errorCode)
	{
		pFrame->show();
		pPrefs->addRecent(pNewFile);
	}
	else
	{
		s_CouldNotLoadFileMessage(pFrame,pNewFile, errorCode);
	}

	return errorCode;
}

Defun1(fileOpen)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	char * pNewFile = NULL;
	IEFileType ieft = (IEFileType)BAD_FILETYPE;
	bool bOK = s_AskForPathname(pFrame,false,NULL,&pNewFile,&ieft);

	if (!bOK || !pNewFile)
	  return false;
	
	// we own storage for pNewFile and must free it.

	UT_Error error = ::fileOpen(pFrame, pNewFile, ieft);

	free(pNewFile);
	return E2B(error);
}

Defun(fileSave)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	// can only save without prompting if filename already known
	if (!pFrame->getFilename())
		return EX(fileSaveAs);

	UT_Error errSaved;
	errSaved = pAV_View->cmdSave();

	// if it has a problematic extension save as instead
	if (errSaved == UT_EXTENSIONERROR)
	  return EX(fileSaveAs);

	if (errSaved)
	{
		// throw up a dialog
		s_TellSaveFailed(pFrame, pFrame->getFilename(), errSaved);
		return false;
	}

	if (pFrame->getViewNumber() > 0)
	{
		XAP_App * pApp = pFrame->getApp();
		UT_ASSERT(pApp);

		pApp->updateClones(pFrame);
	}

	return true;
}

Defun1(fileSaveAs)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	IEFileType ieft = (IEFileType)BAD_FILETYPE;
	char * pNewFile = NULL;
	bool bOK = s_AskForPathname(pFrame,true,NULL,&pNewFile,&ieft);

	if (!bOK || !pNewFile)
		return false;

	UT_DEBUGMSG(("fileSaveAs: saving as [%s]\n",pNewFile));
	
	UT_Error errSaved;
	errSaved = pAV_View->cmdSaveAs(pNewFile, (int) ieft);
	if (errSaved)
	{
		// throw up a dialog
		s_TellSaveFailed(pFrame, pNewFile, errSaved);
		free(pNewFile);
		return false;
	}

	// update the MRU list
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	pPrefs->addRecent(pNewFile);
	free(pNewFile);

	if (pFrame->getViewNumber() > 0)
	{
		// renumber clones
		pApp->updateClones(pFrame);
	}

	return true;
}

Defun1(fileSaveAsWeb)
{
  XAP_Frame * pFrame = static_cast<XAP_Frame *>(pAV_View->getParentData());
  IEFileType ieft = IEFT_HTML;
  char * pNewFile = NULL;
  bool bOK = s_AskForPathname(pFrame,true,NULL,&pNewFile,&ieft);
  
  if (!bOK || !pNewFile)
    return false;

  UT_Error errSaved;
  errSaved = pAV_View->cmdSaveAs(pNewFile, (int) ieft);
  if (errSaved)
    {
      // throw up a dialog
      s_TellSaveFailed(pFrame, pNewFile, errSaved);
      free(pNewFile);
      return false;
    }

  return true;
}

Defun1(filePreviewWeb)
{
  XAP_Frame * pFrame = static_cast<XAP_Frame *>(pAV_View->getParentData());
  char szTempFileName[ 2048 ];

  UT_tmpnam(szTempFileName);

  // Make sure that our filename ends with .html
  // This is necessary for this feature to work on Win32!
  if( strlen( szTempFileName ) > 5 )
  {
	  char *pWhere = strrchr( szTempFileName, '.' );

	  if( pWhere )
	  {
		if( UT_stricmp( pWhere, ".html" ) )
			strcpy( pWhere, ".html" );
	  }
	  else
		  strcat( szTempFileName, ".html" );
  }

  UT_Error errSaved = UT_OK;

  // we do this because we don't want to change the default
  // document extension or rename what we're working on
  errSaved = pAV_View->cmdSaveAs(szTempFileName, (int)IEFT_HTML, false);

  if(errSaved != UT_OK)
    {
      // throw up a dialog
      s_TellSaveFailed(pFrame, szTempFileName, errSaved);
      return false;
    }

  char * tmpUrl = NULL;
  
  tmpUrl = UT_catPathname("file:///", szTempFileName);

  bool bOk = _helpOpenURL(pAV_View, tmpUrl);
  FREEP(tmpUrl);

  return bOk;
}

Defun1(undo)
{
	pAV_View->cmdUndo(1);
	return true;
}

Defun1(redo)
{
	pAV_View->cmdRedo(1);
	return true;
}

Defun1(newWindow)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	return (pFrame->cloneFrame() ? true : false);
}

static bool _openRecent(AV_View* pAV_View, UT_uint32 ndx)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);

	UT_ASSERT(ndx > 0);
	UT_ASSERT(ndx <= pPrefs->getRecentCount());

	const char * szRecent = pPrefs->getRecent(ndx);

	// TODO HACK BROKEN BUSTED BLAH WARNING NOTE ERROR
	// BROKEN: We should store some sort of file type with the MRU data
	// BROKEN: or we don't know what to open it as!  We can't assume
	// BROKEN: IEFT_Unknown will detect what the user saved it as,
	// BROKEN: since the user can explictly export as any type.
	// TODO HACK BROKEN BUSTED BLAH WARNING NOTE ERROR
	
	UT_Error error = fileOpen(pFrame, szRecent, IEFT_Unknown);

	if (error)
		pPrefs->removeRecent(ndx);

	return E2B(error);
}

Defun1(openRecent_1)
{
	return _openRecent(pAV_View, 1);
}
Defun1(openRecent_2)
{
	return _openRecent(pAV_View, 2);
}
Defun1(openRecent_3)
{
	return _openRecent(pAV_View, 3);
}
Defun1(openRecent_4)
{
	return _openRecent(pAV_View, 4);
}
Defun1(openRecent_5)
{
	return _openRecent(pAV_View, 5);
}
Defun1(openRecent_6)
{
	return _openRecent(pAV_View, 6);
}
Defun1(openRecent_7)
{
	return _openRecent(pAV_View, 7);
}
Defun1(openRecent_8)
{
	return _openRecent(pAV_View, 8);
}
Defun1(openRecent_9)
{
	return _openRecent(pAV_View, 9);
}

static bool _activateWindow(AV_View* pAV_View, UT_uint32 ndx)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	UT_ASSERT(ndx > 0);
	UT_ASSERT(ndx <= pApp->getFrameCount());

	XAP_Frame * pSelFrame = pApp->getFrame(ndx - 1);

	if (pSelFrame)
		pSelFrame->raise();

	return true;
}

Defun1(activateWindow_1)
{
	return _activateWindow(pAV_View, 1);
}
Defun1(activateWindow_2)
{
	return _activateWindow(pAV_View, 2);
}
Defun1(activateWindow_3)
{
	return _activateWindow(pAV_View, 3);
}
Defun1(activateWindow_4)
{
	return _activateWindow(pAV_View, 4);
}
Defun1(activateWindow_5)
{
	return _activateWindow(pAV_View, 5);
}
Defun1(activateWindow_6)
{
	return _activateWindow(pAV_View, 6);
}
Defun1(activateWindow_7)
{
	return _activateWindow(pAV_View, 7);
}
Defun1(activateWindow_8)
{
	return _activateWindow(pAV_View, 8);
}
Defun1(activateWindow_9)
{
	return _activateWindow(pAV_View, 9);
}

static bool s_doMoreWindowsDlg(XAP_Frame* pFrame, XAP_Dialog_Id id)
{
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_WindowMore * pDialog
		= (XAP_Dialog_WindowMore *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	// run the dialog
	pDialog->runModal(pFrame);

	XAP_Frame * pSelFrame = NULL;	
	bool bOK = (pDialog->getAnswer() == XAP_Dialog_WindowMore::a_OK);

	if (bOK)
		pSelFrame = pDialog->getSelFrame();

	pDialogFactory->releaseDialog(pDialog);

	// now do it
	if (pSelFrame)
		pSelFrame->raise();

	return bOK;
}

Defun1(dlgMoreWindows)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	s_doMoreWindowsDlg(pFrame, XAP_DIALOG_ID_WINDOWMORE);
	return true;
}

static bool s_doAboutDlg(XAP_Frame* pFrame, XAP_Dialog_Id id)
{
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_About * pDialog
		= (XAP_Dialog_About *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	// run the dialog (it should really be modeless if anyone
	// gets the urge to make it safe that way)
	pDialog->runModal(pFrame);
	
	bool bOK = true;

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

static bool s_doToggleCase(XAP_Frame * pFrame, FV_View * pView, XAP_Dialog_Id id)
{
	UT_ASSERT(pFrame);

	pFrame->raise();

	if (pView->isSelectionEmpty())
	  {
	    pFrame->showMessageBox(AP_STRING_ID_MSG_EmptySelection,
				   XAP_Dialog_MessageBox::b_O,
				   XAP_Dialog_MessageBox::a_OK); 
	    return false;
	  }

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_ToggleCase * pDialog
		= (AP_Dialog_ToggleCase *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	// run the dialog (it should really be modeless if anyone
	// gets the urge to make it safe that way)
	pDialog->runModal(pFrame);
	bool bOK = (pDialog->getAnswer() == AP_Dialog_ToggleCase::a_OK);

	if (bOK)
	  pView->toggleCase(pDialog->getCase());

	pDialogFactory->releaseDialog(pDialog);
  
        return bOK;
}

Defun1(dlgToggleCase)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	return s_doToggleCase(pFrame, static_cast<FV_View *>(pAV_View), AP_DIALOG_ID_TOGGLECASE);
}

Defun1(dlgAbout)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	s_doAboutDlg(pFrame, XAP_DIALOG_ID_ABOUT);

	return true;
}

bool _helpOpenURL(AV_View* pAV_View, const char* helpURL)
{
 	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	pFrame->openURL(helpURL);
	return true;
}

bool _helpLocalizeAndOpenURL(AV_View* pAV_View, bool bLocal, const char* pathBeforeLang, const char* pathAfterLang)
{
 	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);

	const char * abiSuiteLibDir = pApp->getAbiSuiteLibDir();
	const XML_Char * abiSuiteLocString = NULL;
	char *helpURL, *tmpURL;

	helpURL = tmpURL = NULL;
	pPrefs->getPrefsValue((XML_Char*)AP_PREF_KEY_StringSet, &abiSuiteLocString);

	if (bLocal)
	{
		tmpURL = helpURL = UT_catPathname("file://", abiSuiteLibDir);
		helpURL = UT_catPathname(helpURL, pathBeforeLang);
		FREEP(tmpURL);
		tmpURL = helpURL;
		helpURL = UT_catPathname(helpURL, abiSuiteLocString);
		FREEP(tmpURL);
		tmpURL = helpURL;
		helpURL = UT_catPathname(helpURL, pathAfterLang);
		FREEP(tmpURL);
	}
	else {
		//TODO: No one uses this, so what kind of prefix should it have?
		tmpURL = helpURL = UT_catPathname(pathBeforeLang, abiSuiteLocString);
		helpURL = UT_catPathname(helpURL, pathAfterLang);
		FREEP(tmpURL);
	}

	bool bRetBuf = _helpOpenURL(pAV_View, helpURL);
	FREEP(helpURL);
	return bRetBuf;
}

Defun1(helpContents)
{
 	return _helpLocalizeAndOpenURL(pAV_View, true, "AbiWord/help", "contents.html");
}

Defun1(helpIndex)
{
 	return _helpLocalizeAndOpenURL(pAV_View, true, "AbiWord/help", "index.html");
}

Defun1(helpCheckVer)
{
	UT_String versionURL = "http://www.abisource.com/users/check_version.phtml?version=";
	versionURL += XAP_App::s_szBuild_Version;
 	return _helpOpenURL(pAV_View, versionURL);
}

Defun1(helpSearch)
{
        return _helpLocalizeAndOpenURL(pAV_View, true, "AbiWord/help", "search.html");
}

Defun1(helpAboutOS)
{
 	return _helpLocalizeAndOpenURL(pAV_View, true, "AbiWord/help", "aboutos.html");
}

Defun1(cycleWindows)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	UT_sint32 ndx = pApp->findFrame(pFrame);
	UT_ASSERT(ndx >= 0);

	if (ndx < (UT_sint32) pApp->getFrameCount() - 1)
		ndx++;
	else
		ndx = 0;

	XAP_Frame * pSelFrame = pApp->getFrame(ndx);

	if (pSelFrame)
		pSelFrame->raise();

	return true;
}

Defun1(cycleWindowsBck)
{
    XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	UT_sint32 ndx = pApp->findFrame(pFrame);
	UT_ASSERT(ndx >= 0);

	if (ndx > 0)
		ndx--;
	else
		ndx = pApp->getFrameCount() - 1;

	XAP_Frame * pSelFrame = pApp->getFrame(ndx);

	if (pSelFrame)
		pSelFrame->raise();

	return true;
}

Defun(closeWindow)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);


        if(pFrame == pApp->getLastFocussedFrame())
	{

	  // This probabally not necessary given the code that's in xap_App
          // but I hate seg faults.

	       pApp->clearLastFocussedFrame();
	}
	

	// is this the last view on a dirty document?
	if ((pFrame->getViewNumber() == 0) &&
		(pFrame->isDirty()))
	{
		XAP_Dialog_MessageBox::tAnswer ans = s_AskSaveFile(pFrame);

		switch (ans)
		{
		case XAP_Dialog_MessageBox::a_YES:				// save it first
			{
				bool bRet = EX(fileSave);
				if (!bRet)								// didn't successfully save,
					return false;					//    so don't close
			}
			break;

		case XAP_Dialog_MessageBox::a_NO:				// just close it
			break;

		case XAP_Dialog_MessageBox::a_CANCEL:			// don't close it
			return false;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
	}

	// are we the last window?
	if (1 >= pApp->getFrameCount())
	{
	  // Delete all the open modeless dialogs

	       pApp->closeModelessDlgs();
	       pApp->reallyExit();
	}

	// nuke the window
	
	pApp->forgetFrame(pFrame);
	pFrame->close();
	delete pFrame;

	return true;
}

Defun(querySaveAndExit)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);


	if (1 < pApp->getFrameCount())
	{
		if (!s_AskCloseAllAndExit(pFrame))
		{
			// never mind
			return false;
		}
	}


	bool bRet = true;
	UT_uint32 ndx = pApp->getFrameCount();

	// loop over windows, but stop if one can't close
	while (bRet && ndx > 0)
	{
		XAP_Frame * f = pApp->getFrame(ndx - 1);
		UT_ASSERT(f);
		pAV_View = f->getCurrentView();
		UT_ASSERT(pAV_View);

		bRet = EX(closeWindow);

		ndx--;
	}

	if (bRet)
	{

 	        //  delete all open modeless dialogs
	        pApp->closeModelessDlgs();

		// TODO: this shouldn't be necessary, but just in case
		pApp->reallyExit();
	}

	return bRet;
}

/*****************************************************************/
/*****************************************************************/

/*
	NOTE: This file should really be split in two:
	
		1.  XAP methods (above)
		2.  AbiWord-specific methods (below)

	Until we do the necessary architectural work, we just segregate
	the methods within the same file.
*/
#define ABIWORD_VIEW  	FV_View * pView = static_cast<FV_View *>(pAV_View)

Defun1(fileInsertGraphic)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	char* pNewFile = NULL;


	IEGraphicFileType iegft;
	bool bOK = s_AskForGraphicPathname(pFrame,&pNewFile,&iegft);

	if (!bOK || !pNewFile)
		return false;

	// we own storage for pNewFile and must free it.
	UT_DEBUGMSG(("fileInsertGraphic: loading [%s]\n",pNewFile));

       	IE_ImpGraphic *pIEG;
	FG_Graphic* pFG;

	UT_Error errorCode;

	errorCode = IE_ImpGraphic::constructImporter(pNewFile, iegft, &pIEG);
	if(errorCode)
	  {
		s_CouldNotLoadFileMessage(pFrame, pNewFile, errorCode);
		FREEP(pNewFile);
		return false;
	  }
	
	errorCode = pIEG->importGraphic(pNewFile, &pFG);
	if(errorCode)
	  {
		s_CouldNotLoadFileMessage(pFrame, pNewFile, errorCode);
		FREEP(pNewFile);
		DELETEP(pIEG);
		return false;
	  }

	DELETEP(pIEG);
	
	ABIWORD_VIEW;

	errorCode = pView->cmdInsertGraphic(pFG, pNewFile);
	if (errorCode)
	{
		s_CouldNotLoadFileMessage(pFrame, pNewFile, errorCode);

		FREEP(pNewFile);
		DELETEP(pFG);
		return false;
	}

	FREEP(pNewFile);
	DELETEP(pFG);
	
	return true;
}

Defun(warpInsPtToXY)
{
	ABIWORD_VIEW;
	pView->warpInsPtToXY(pCallData->m_xPos, pCallData->m_yPos, true);

	return true;
}

Defun1(warpInsPtLeft)
{
	ABIWORD_VIEW;
	pView->cmdCharMotion(false,1);
	return true;
}

Defun1(warpInsPtRight)
{
	ABIWORD_VIEW;
	pView->cmdCharMotion(true,1);
	return true;
}

Defun1(warpInsPtBOP)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOP);
	return true;
}

Defun1(warpInsPtEOP)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_EOP);
	return true;
}

Defun1(warpInsPtBOL)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOL);
	return true;
}

Defun1(warpInsPtEOL)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_EOL);
	return true;
}

Defun1(warpInsPtBOW)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOW);
	return true;
}

Defun1(warpInsPtEOW)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_EOW_MOVE);
	return true;
}

Defun0(warpInsPtBOS)
{
	return true;
}

Defun0(warpInsPtEOS)
{
	return true;
}

Defun1(warpInsPtBOB)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOB);
	return true;
}

Defun1(warpInsPtEOB)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_EOB);
	return true;
}

Defun1(warpInsPtBOD)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOD);
	return true;
}

Defun1(warpInsPtEOD)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_EOD);
	return true;
}

Defun1(warpInsPtPrevPage)
{
	ABIWORD_VIEW;
	pView->warpInsPtNextPrevPage(false);
	return true;
}

Defun1(warpInsPtNextPage)
{
	ABIWORD_VIEW;
	pView->warpInsPtNextPrevPage(true);
	return true;
}

Defun1(warpInsPtPrevLine)
{
	ABIWORD_VIEW;
	pView->warpInsPtNextPrevLine(false);
	return true;
}

Defun1(warpInsPtNextLine)
{
	ABIWORD_VIEW;
	pView->warpInsPtNextPrevLine(true);
	return true;
}

/*****************************************************************/

Defun1(cursorDefault)
{
	ABIWORD_VIEW;
	GR_Graphics * pG = pView->getGraphics();
	if (pG)
		pG->setCursor(GR_Graphics::GR_CURSOR_DEFAULT);
	return true;
}

Defun1(cursorIBeam)
{
	ABIWORD_VIEW;
	GR_Graphics * pG = pView->getGraphics();
	if (pG)
		pG->setCursor(GR_Graphics::GR_CURSOR_IBEAM);
	return true;
}

Defun1(cursorRightArrow)
{
	ABIWORD_VIEW;
	GR_Graphics * pG = pView->getGraphics();
	if (pG)
		pG->setCursor(GR_Graphics::GR_CURSOR_RIGHTARROW);
	return true;
}

Defun1(cursorImage)
{
	ABIWORD_VIEW;
	GR_Graphics * pG = pView->getGraphics();
	if (pG)
		pG->setCursor(GR_Graphics::GR_CURSOR_IMAGE);
	return true;
}

Defun1(cursorImageSize)
{
	// TODO figure out which corner or side we are on and
	// TODO map cursor to one of the standard 8 resizers.
	ABIWORD_VIEW;
	GR_Graphics * pG = pView->getGraphics();
	if (pG)
		pG->setCursor(GR_Graphics::GR_CURSOR_IBEAM);
	return true;
}

/*****************************************************************/

Defun1(contextMenu)
{
	// raise context menu over whatever we are over.  this is
	// intended for use by the keyboard accelerator rather than
	// the other "targeted" context{...} methods which are bound
	// to the mouse.
	
	ABIWORD_VIEW;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_ASSERT(pFrame);

	UT_sint32 xPos, yPos;
	EV_EditMouseContext emc = pView->getInsertionPointContext(&xPos,&yPos);

	const char * szContextMenuName = AP_FindContextMenu(emc);
	if (!szContextMenuName)
		return false;
	
	return pFrame->runModalContextMenu(pView,szContextMenuName,xPos,yPos);
}

Defun(contextText)
{
	ABIWORD_VIEW;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_ASSERT(pFrame);

	// move the IP so actions have the right context
	if (!pView->isXYSelected(pCallData->m_xPos, pCallData->m_yPos))
		EX(warpInsPtToXY);

	const char * szContextMenuName = AP_FindContextMenu(EV_EMC_TEXT);
	if (!szContextMenuName)
		return false;
	
	return pFrame->runModalContextMenu(pView,szContextMenuName,
									   pCallData->m_xPos,pCallData->m_yPos);
}

Defun(contextMisspellText)
{
	ABIWORD_VIEW;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_ASSERT(pFrame);

	// move the IP so actions have the right context
	EX(warpInsPtToXY);

	const char * szContextMenuName = AP_FindContextMenu(EV_EMC_MISSPELLEDTEXT);
	if (!szContextMenuName)
		return false;
	
	return pFrame->runModalContextMenu(pView,szContextMenuName,
									   pCallData->m_xPos,pCallData->m_yPos);
}

static bool _spellSuggest(AV_View* pAV_View, UT_uint32 ndx)
{
	ABIWORD_VIEW;
	pView->cmdContextSuggest(ndx);
	return true;
}

Defun1(spellSuggest_1)
{
	return _spellSuggest(pAV_View, 1);
}
Defun1(spellSuggest_2)
{
	return _spellSuggest(pAV_View, 2);
}
Defun1(spellSuggest_3)
{
	return _spellSuggest(pAV_View, 3);
}
Defun1(spellSuggest_4)
{
	return _spellSuggest(pAV_View, 4);
}
Defun1(spellSuggest_5)
{
	return _spellSuggest(pAV_View, 5);
}
Defun1(spellSuggest_6)
{
	return _spellSuggest(pAV_View, 6);
}
Defun1(spellSuggest_7)
{
	return _spellSuggest(pAV_View, 7);
}
Defun1(spellSuggest_8)
{
	return _spellSuggest(pAV_View, 8);
}
Defun1(spellSuggest_9)
{
	return _spellSuggest(pAV_View, 9);
}

Defun1(spellIgnoreAll)
{
	ABIWORD_VIEW;

	pView->cmdContextIgnoreAll();
	return true;
}

Defun1(spellAdd)
{
	ABIWORD_VIEW;

	pView->cmdContextAdd();
	return true;
}


/*****************************************************************/

Defun(dragToXY)
{
	ABIWORD_VIEW;
	pView->extSelToXY(pCallData->m_xPos, pCallData->m_yPos, true);
	return true;
}

Defun(dragToXYword)
{
	ABIWORD_VIEW;
	pView->extSelToXYword(pCallData->m_xPos, pCallData->m_yPos, true);
	return true;
}

Defun(endDrag)
{
	ABIWORD_VIEW;
	pView->endDrag(pCallData->m_xPos, pCallData->m_yPos);
	return true;
}

Defun(extSelToXY)
{
	ABIWORD_VIEW;
	pView->extSelToXY(pCallData->m_xPos, pCallData->m_yPos, false);
	return true;
}

Defun1(extSelLeft)
{
	ABIWORD_VIEW;
	pView->extSelHorizontal(false,1);
	return true;
}

Defun1(extSelRight)
{
	ABIWORD_VIEW;
	pView->extSelHorizontal(true,1);
	return true;
}

Defun1(extSelBOL)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_BOL);
	return true;
}

Defun1(extSelEOL)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_EOL);
	return true;
}

Defun1(extSelBOW)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_BOW);
	return true;
}

Defun1(extSelEOW)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_EOW_MOVE);
	return true;
}

Defun0(extSelBOS)
{
	return true;
}

Defun0(extSelEOS)
{
	return true;
}

Defun1(extSelBOB)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_BOB);
	return true;
}

Defun1(extSelEOB)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_EOB);
	return true;
}

Defun1(extSelBOD)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_BOD);
	return true;
}

Defun1(extSelEOD)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_EOD);
	return true;
}

Defun1(extSelPrevLine)
{
	ABIWORD_VIEW;
	pView->extSelNextPrevLine(false);
	return true;
}

Defun1(extSelNextLine)
{
	ABIWORD_VIEW;
	pView->extSelNextPrevLine(true);
	return true;
}

Defun0(extSelPageDown)
{
	return true;
}

Defun0(extSelPageUp)
{
	return true;
}

Defun(selectAll)
{
	ABIWORD_VIEW;
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOD, FV_DOCPOS_EOD);
	return true;
}

Defun(selectWord)
{
	ABIWORD_VIEW;
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOW, FV_DOCPOS_EOW_SELECT);
	return true;
}

Defun(selectLine)
{
	ABIWORD_VIEW;
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOL, FV_DOCPOS_EOL);
	return true;
}

Defun(selectBlock)
{
	ABIWORD_VIEW;
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOB, FV_DOCPOS_EOB);
	return true;
}

Defun1(delLeft)
{
	ABIWORD_VIEW;
	pView->cmdCharDelete(false,1);
	return true;
}

Defun1(delRight)
{
	ABIWORD_VIEW;
	pView->cmdCharDelete(true,1);
	return true;
}

Defun1(delBOL)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_BOL);
	return true;
}

Defun1(delEOL)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_EOL);
	return true;
}

Defun1(delBOW)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_BOW);
	return true;
}

Defun1(delEOW)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_EOW_SELECT);
	return true;
}

Defun0(delBOS)
{
	return true;
}

Defun0(delEOS)
{
	return true;
}

Defun1(delBOB)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_BOB);
	return true;
}

Defun1(delEOB)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_EOB);
	return true;
}

Defun1(delBOD)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_BOD);
	return true;
}

Defun1(delEOD)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_EOD);
	return true;
}

Defun(insertData)
{
	ABIWORD_VIEW;
	pView->cmdCharInsert(pCallData->m_pData,pCallData->m_dataLength);
	return true;
}

Defun(replaceChar)
{
	//ABIWORD_VIEW;
	return ( EX(delRight) && EX(insertData) && EX(setEditVI) );
}

Defun0(insertSoftBreak)
{
	return true;
}

Defun1(insertParagraphBreak)
{
	ABIWORD_VIEW;
	pView->insertParagraphBreak();
	return true;
}

Defun1(insertSectionBreak)
{
	ABIWORD_VIEW;
	pView->insertSectionBreak();
	return true;
}

/*
  Note that within the piece table, we use the following
  representations:
    char code					meaning
	UCS_TAB  (tab)				tab
	UCS_LF   (line feed)		forced line break
	UCS_VTAB (vertical tab)		forced column break
	UCS_FF   (form feed)		forced page break
*/

Defun1(insertTab)
{
	ABIWORD_VIEW;
	UT_UCSChar c = UCS_TAB;
	pView->cmdCharInsert(&c,1);
	return true;
}

Defun1(insertLineBreak)
{
	ABIWORD_VIEW;
	UT_UCSChar c = UCS_LF;
	pView->cmdCharInsert(&c,1);
	return true;
}

Defun1(insertColumnBreak)
{
	ABIWORD_VIEW;
	UT_UCSChar c = UCS_VTAB;
	pView->cmdCharInsert(&c,1);
	return true;
}

Defun1(insertPageBreak)
{
	ABIWORD_VIEW;
	UT_UCSChar c = UCS_FF;
	pView->cmdCharInsert(&c,1);
	return true;
}

Defun1(insertSpace)
{
	ABIWORD_VIEW;
	UT_UCSChar c = UCS_SPACE;
	pView->cmdCharInsert(&c,1);
	return true;
}

Defun1(insertNBSpace)
{
	ABIWORD_VIEW;
	UT_UCSChar c = UCS_NBSP;			// decimal 160 is NBS
	pView->cmdCharInsert(&c,1);
	return true;
}

/*****************************************************************/

Defun(insertGraveData)
{
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Grave map are mapped here.  The desired
	// character is in the argument (just like in insertData()).
	// We do the character mapping here.
	//
	// This keeps us from having to define 10 EditMethod
	// functions (one for each grave character).
	//
	// It would be nice if the key-binding mechanism (in
	// ap_LoadBindings_*.cpp) were extended to allow a constant
	// to be specified along with the function binding, so that
	// we could have bound 'A' on the DeadGrave map to
	// "insertData(0x00c0)", for example.
	
	UT_ASSERT(pCallData->m_dataLength==1);
	UT_UCSChar graveChar = 0x0000;
	switch (pCallData->m_pData[0])
	{
	case 0x41:		graveChar=0x00c0;	break;	// Agrave
	case 0x45:		graveChar=0x00c8;	break;	// Egrave
	case 0x49:		graveChar=0x00cc;	break;	// Igrave
	case 0x4f:		graveChar=0x00d2;	break;	// Ograve
	case 0x55:		graveChar=0x00d9;	break;	// Ugrave

	case 0x61:		graveChar=0x00e0;	break;	// agrave
	case 0x65:		graveChar=0x00e8;	break;	// egrave
	case 0x69:		graveChar=0x00ec;	break;	// igrave
	case 0x6f:		graveChar=0x00f2;	break;	// ograve
	case 0x75:		graveChar=0x00f9;	break;	// ugrave
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	
	pView->cmdCharInsert(&graveChar, 1);
	return true;
}

Defun(insertAcuteData)
{
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Acute map are mapped here.  The desired
	// character is in the argument (just like in insertData()).
	// We do the character mapping here.
	//
	// See note in Defun(insertGraveData)
	
	UT_ASSERT(pCallData->m_dataLength==1);
	UT_UCSChar acuteChar = 0x0000;
	switch (pCallData->m_pData[0])
	{
	case 0x41:		acuteChar=0x00c1;	break;	// Aacute
	case 0x45:		acuteChar=0x00c9;	break;	// Eacute
	case 0x49:		acuteChar=0x00cd;	break;	// Iacute
	case 0x4f:		acuteChar=0x00d3;	break;	// Oacute
	case 0x55:		acuteChar=0x00da;	break;	// Uacute
	case 0x59:		acuteChar=0x00dd;	break;	// Yacute

	case 0x61:		acuteChar=0x00e1;	break;	// aacute
	case 0x65:		acuteChar=0x00e9;	break;	// eacute
	case 0x69:		acuteChar=0x00ed;	break;	// iacute
	case 0x6f:		acuteChar=0x00f3;	break;	// oacute
	case 0x75:		acuteChar=0x00fa;	break;	// uacute
	case 0x79:		acuteChar=0x00fd;	break;	// yacute

	// Latin-2 characters
	case 0x53:		acuteChar=0x01a6;	break;	// Sacute
	case 0x5a:		acuteChar=0x01ac;	break;	// Zacute
	case 0x52:		acuteChar=0x01c0;	break;	// Racute
	case 0x4c:		acuteChar=0x01c5;	break;	// Lacute
	case 0x43:		acuteChar=0x01c6;	break;	// Cacute
	case 0x4e:		acuteChar=0x01d1;	break;	// Nacute

	case 0x73:		acuteChar=0x01b6;	break;	// sacute
	case 0x7a:		acuteChar=0x01bc;	break;	// zacute
	case 0x72:		acuteChar=0x01e0;	break;	// racute
	case 0x6c:		acuteChar=0x01e5;	break;	// lacute
	case 0x63:		acuteChar=0x01e6;	break;	// cacute
	case 0x6e:		acuteChar=0x01f1;	break;	// nacute

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	
	pView->cmdCharInsert(&acuteChar, 1);
	return true;
}

Defun(insertCircumflexData)
{
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Circumflex map are mapped here.  The desired
	// character is in the argument (just like in insertData()).
	// We do the character mapping here.
	//
	// See note in Defun(insertGraveData)
	
	UT_ASSERT(pCallData->m_dataLength==1);
	UT_UCSChar circumflexChar = 0x0000;
	switch (pCallData->m_pData[0])
	{
	case 0x41:		circumflexChar=0x00c2;	break;	// Acircumflex
	case 0x45:		circumflexChar=0x00ca;	break;	// Ecircumflex
	case 0x49:		circumflexChar=0x00ce;	break;	// Icircumflex
	case 0x4f:		circumflexChar=0x00d4;	break;	// Ocircumflex
	case 0x55:		circumflexChar=0x00db;	break;	// Ucircumflex

	case 0x61:		circumflexChar=0x00e2;	break;	// acircumflex
	case 0x65:		circumflexChar=0x00ea;	break;	// ecircumflex
	case 0x69:		circumflexChar=0x00ee;	break;	// icircumflex
	case 0x6f:		circumflexChar=0x00f4;	break;	// ocircumflex
	case 0x75:		circumflexChar=0x00fb;	break;	// ucircumflex

	// Latin-3 characters
	case 0x48:		circumflexChar=0x02a6;	break;	// Hcircumflex
	case 0x4a:		circumflexChar=0x02ac;	break;	// Jcircumflex
	case 0x43:		circumflexChar=0x02c6;	break;	// Ccircumflex
	case 0x47:		circumflexChar=0x02d8;	break;	// Gcircumflex
	case 0x53:		circumflexChar=0x02de;	break;	// Scircumflex

	case 0x68:		circumflexChar=0x02b6;	break;	// hcircumflex
	case 0x6a:		circumflexChar=0x02bc;	break;	// jcircumflex
	case 0x63:		circumflexChar=0x02e6;	break;	// ccircumflex
	case 0x67:		circumflexChar=0x02f8;	break;	// gcircumflex
	case 0x73:		circumflexChar=0x02fe;	break;	// scircumflex

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	
	pView->cmdCharInsert(&circumflexChar, 1);
	return true;
}

Defun(insertTildeData)
{
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Tilde map are mapped here.  The desired
	// character is in the argument (just like in insertData()).
	// We do the character mapping here.
	//
	// See note in Defun(insertGraveData)
	
	UT_ASSERT(pCallData->m_dataLength==1);
	UT_UCSChar tildeChar = 0x0000;
	switch (pCallData->m_pData[0])
	{
	case 0x41:		tildeChar=0x00c3;	break;	// Atilde
	case 0x4e:		tildeChar=0x00d1;	break;	// Ntilde
	case 0x4f:		tildeChar=0x00d5;	break;	// Otilde

	case 0x61:		tildeChar=0x00e3;	break;	// atilde
	case 0x6e:		tildeChar=0x00f1;	break;	// ntilde
	case 0x6f:		tildeChar=0x00f5;	break;	// otilde

	// Latin-4 characters
	case 0x49:		tildeChar=0x03a5;	break;	// Itilde
	case 0x55:		tildeChar=0x03dd;	break;	// Utilde

	case 0x69:		tildeChar=0x03b5;	break;	// itilde
	case 0x75:		tildeChar=0x03fd;	break;	// utilde

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	
	pView->cmdCharInsert(&tildeChar, 1);
	return true;
}

Defun(insertMacronData)
{
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Macron map are mapped here.  The desired
	// character is in the argument (just like in insertData()).
	// We do the character mapping here.
	//
	// See note in Defun(insertGraveData)
	
	UT_ASSERT(pCallData->m_dataLength==1);
	UT_UCSChar macronChar = 0x0000;

	switch (pCallData->m_pData[0])
	{
	// Latin-4 characters
	case 0x45:		macronChar=0x03aa;	break;	// Emacron
	case 0x41:		macronChar=0x03c0;	break;	// Amacron
	case 0x49:		macronChar=0x03cf;	break;	// Imacron
	case 0x4f:		macronChar=0x03d2;	break;	// Omacron
	case 0x55:		macronChar=0x03de;	break;	// Umacron

	case 0x65:		macronChar=0x03ba;	break;	// emacron
	case 0x61:		macronChar=0x03e0;	break;	// amacron
	case 0x69:		macronChar=0x03ef;	break;	// imacron
	case 0x6f:		macronChar=0x03f2;	break;	// omacron
	case 0x75:		macronChar=0x03fe;	break;	// umacron

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	
	pView->cmdCharInsert(&macronChar, 1);
	return true;
}

Defun(insertBreveData)
{
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Breve map are mapped here.  The desired
	// character is in the argument (just like in insertData()).
	// We do the character mapping here.
	//
	// See note in Defun(insertGraveData)
	
	UT_ASSERT(pCallData->m_dataLength==1);
	UT_UCSChar breveChar = 0x0000;

	switch (pCallData->m_pData[0])
	{
	// Latin-[23] characters
	case 0x41:		breveChar=0x01c3;	break;	// Abreve
	case 0x47:		breveChar=0x02ab;	break;	// Gbreve
	case 0x55:		breveChar=0x02dd;	break;	// Ubreve

	case 0x61:		breveChar=0x01e3;	break;	// abreve
	case 0x67:		breveChar=0x02bb;	break;	// gbreve
	case 0x75:		breveChar=0x02fd;	break;	// ubreve

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	
	pView->cmdCharInsert(&breveChar, 1);
	return true;
}

Defun(insertAbovedotData)
{
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Abovedot map are mapped here.  The desired
	// character is in the argument (just like in insertData()).
	// We do the character mapping here.
	//
	// See note in Defun(insertGraveData)
	
	UT_ASSERT(pCallData->m_dataLength==1);
	UT_UCSChar abovedotChar = 0x0000;

	switch (pCallData->m_pData[0])
	{
	// Latin-[234] characters
	case 0x5a:		abovedotChar=0x01af;	break;	// Zabovedot
	case 0x49:		abovedotChar=0x02a9;	break;	// Iabovedot
	case 0x43:		abovedotChar=0x02c5;	break;	// Cabovedot
	case 0x47:		abovedotChar=0x02d5;	break;	// Gabovedot
	case 0x45:		abovedotChar=0x03cc;	break;	// Eabovedot

	case 0x7a:		abovedotChar=0x01bf;	break;	// zabovedot
	//case 0x69: TODO no corresponding 'iabovedot', is this supposed to be 'idotless' ??
	case 0x63:		abovedotChar=0x02e5;	break;	// cabovedot
	case 0x67:		abovedotChar=0x02f5;	break;	// gabovedot
	case 0x65:		abovedotChar=0x03ec;	break;	// eabovedot

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	
	pView->cmdCharInsert(&abovedotChar, 1);
	return true;
}

Defun(insertDiaeresisData)
{
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Diaeresis map are mapped here.  The desired
	// character is in the argument (just like in insertData()).
	// We do the character mapping here.
	//
	// See note in Defun(insertGraveData)
	
	UT_ASSERT(pCallData->m_dataLength==1);
	UT_UCSChar diaeresisChar = 0x0000;
	switch (pCallData->m_pData[0])
	{
	case 0x41:		diaeresisChar=0x00c4;	break;	// Adiaeresis
	case 0x45:		diaeresisChar=0x00cb;	break;	// Ediaeresis
	case 0x49:		diaeresisChar=0x00cf;	break;	// Idiaeresis
	case 0x4f:		diaeresisChar=0x00d6;	break;	// Odiaeresis
	case 0x55:		diaeresisChar=0x00dc;	break;	// Udiaeresis
	// TODO no Ydiaeresis ??

	case 0x61:		diaeresisChar=0x00e4;	break;	// adiaeresis
	case 0x65:		diaeresisChar=0x00eb;	break;	// ediaeresis
	case 0x69:		diaeresisChar=0x00ef;	break;	// idiaeresis
	case 0x6f:		diaeresisChar=0x00f6;	break;	// odiaeresis
	case 0x75:		diaeresisChar=0x00fc;	break;	// udiaeresis
	case 0x79:		diaeresisChar=0x00ff;	break;	// ydiaeresis
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	
	pView->cmdCharInsert(&diaeresisChar, 1);
	return true;
}

Defun(insertDoubleacuteData)
{
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Doubleacute map are mapped here.  The desired
	// character is in the argument (just like in insertData()).
	// We do the character mapping here.
	//
	// See note in Defun(insertGraveData)
	
	UT_ASSERT(pCallData->m_dataLength==1);
	UT_UCSChar doubleacuteChar = 0x0000;

	switch (pCallData->m_pData[0])
	{
	// Latin-2 characters
	case 0x4f:		doubleacuteChar=0x01d5;	break;	// Odoubleacute
	case 0x55:		doubleacuteChar=0x01db;	break;	// Udoubleacute

	case 0x6f:		doubleacuteChar=0x01f5;	break;	// odoubleacute
	case 0x75:		doubleacuteChar=0x01fb;	break;	// udoubleacute

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	
	pView->cmdCharInsert(&doubleacuteChar, 1);
	return true;
}

Defun(insertCaronData)
{
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Caron map are mapped here.  The desired
	// character is in the argument (just like in insertData()).
	// We do the character mapping here.
	//
	// See note in Defun(insertGraveData)
	
	UT_ASSERT(pCallData->m_dataLength==1);
	UT_UCSChar caronChar = 0x0000;

	switch (pCallData->m_pData[0])
	{
	// Latin-2 characters
	case 0x4c:		caronChar=0x01a5;	break;	// Lcaron
	case 0x53:		caronChar=0x01a9;	break;	// Scaron
	case 0x54:		caronChar=0x01ab;	break;	// Tcaron
	case 0x5a:		caronChar=0x01ae;	break;	// Zcaron
	case 0x43:		caronChar=0x01c8;	break;	// Ccaron
	case 0x45:		caronChar=0x01cc;	break;	// Ecaron
	case 0x44:		caronChar=0x01cf;	break;	// Dcaron
	case 0x4e:		caronChar=0x01d2;	break;	// Ncaron
	case 0x52:		caronChar=0x01d8;	break;	// Rcaron

	case 0x6c:		caronChar=0x01b5;	break;	// lcaron
	case 0x73:		caronChar=0x01b9;	break;	// scaron
	case 0x74:		caronChar=0x01bb;	break;	// tcaron
	case 0x7a:		caronChar=0x01be;	break;	// zcaron
	case 0x63:		caronChar=0x01e8;	break;	// ccaron
	case 0x65:		caronChar=0x01ec;	break;	// ecaron
	case 0x64:		caronChar=0x01ef;	break;	// dcaron
	case 0x6e:		caronChar=0x01f2;	break;	// ncaron
	case 0x72:		caronChar=0x01f8;	break;	// rcaron

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	
	pView->cmdCharInsert(&caronChar, 1);
	return true;
}

Defun(insertCedillaData)
{
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Cedilla map are mapped here.  The desired
	// character is in the argument (just like in insertData()).
	// We do the character mapping here.
	//
	// See note in Defun(insertGraveData)
	
	UT_ASSERT(pCallData->m_dataLength==1);
	UT_UCSChar cedillaChar = 0x0000;
	switch (pCallData->m_pData[0])
	{
	case 0x43:		cedillaChar=0x00c7;	break;	// Ccedilla
	case 0x63:		cedillaChar=0x00e7;	break;	// ccedilla

	// Latin-[24] characters
	case 0x53:		cedillaChar=0x01aa;	break;	// Scedilla
	case 0x54:		cedillaChar=0x01de;	break;	// Tcedilla
	case 0x52:		cedillaChar=0x03a3;	break;	// Rcedilla
	case 0x4c:		cedillaChar=0x03a6;	break;	// Lcedilla
	case 0x47:		cedillaChar=0x03ab;	break;	// Gcedilla
	case 0x4e:		cedillaChar=0x03d1;	break;	// Ncedilla
	case 0x4b:		cedillaChar=0x03d3;	break;	// Kcedilla

	case 0x73:		cedillaChar=0x01ba;	break;	// scedilla
	case 0x74:		cedillaChar=0x01fe;	break;	// tcedilla
	case 0x72:		cedillaChar=0x03b3;	break;	// rcedilla
	case 0x6c:		cedillaChar=0x03b6;	break;	// lcedilla
	case 0x67:		cedillaChar=0x03bb;	break;	// gcedilla
	case 0x6e:		cedillaChar=0x03f1;	break;	// ncedilla
	case 0x6b:		cedillaChar=0x03f3;	break;	// kcedilla

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	
	pView->cmdCharInsert(&cedillaChar, 1);
	return true;
}

Defun(insertOgonekData)
{
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Ogonek map are mapped here.  The desired
	// character is in the argument (just like in insertData()).
	// We do the character mapping here.
	//
	// See note in Defun(insertGraveData)
	
	UT_ASSERT(pCallData->m_dataLength==1);
	UT_UCSChar ogonekChar = 0x0000;

	switch (pCallData->m_pData[0])
	{
	// Latin-[24] characters
	case 0x41:		ogonekChar=0x01a1;	break;	// Aogonek
	case 0x45:		ogonekChar=0x01ca;	break;	// Eogonek
	case 0x49:		ogonekChar=0x03c7;	break;	// Iogonek
	case 0x55:		ogonekChar=0x03d9;	break;	// Uogonek

	case 0x65:		ogonekChar=0x01b1;	break;	// eogonek
	case 0x61:		ogonekChar=0x01ea;	break;	// aogonek
	case 0x69:		ogonekChar=0x03e7;	break;	// iogonek
	case 0x75:		ogonekChar=0x03f9;	break;	// uogonek

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	
	pView->cmdCharInsert(&ogonekChar, 1);
	return true;
}

/*****************************************************************/

Defun1(cut)
{
	ABIWORD_VIEW;
	pView->cmdCut();
	
	return true;
}

Defun1(copy)
{
	ABIWORD_VIEW;
	pView->cmdCopy();
	
	return true;
}

Defun1(paste)
{
	ABIWORD_VIEW;
	pView->cmdPaste();
	
	return true;
}

Defun(pasteSelection)
{
	// this is intended for the X11 middle mouse thing.
	ABIWORD_VIEW;
	pView->cmdPasteSelectionAt(pCallData->m_xPos, pCallData->m_yPos);
	
	return true;
}

Defun1(editFooter)
{
	ABIWORD_VIEW;
	pView->cmdEditFooter();
	
	return true;
}


Defun1(editHeader)
{
	ABIWORD_VIEW;
	pView->cmdEditHeader();
	
	return true;
}

/*****************************************************************/

static bool s_doGotoDlg(FV_View * pView, XAP_Dialog_Id id)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);
	
	pFrame->raise();
	
	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());
	
	AP_Dialog_Goto * pDialog
		= (AP_Dialog_Goto *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	pDialog->runModeless(pFrame);
	
	//bool bOK = true;

	// get result?
	
	pDialogFactory->releaseDialog(pDialog);

	if(pDialog->isRunning() == true)
	{
		pDialog->activate();
	}
	else
	{
		pDialog->setView(pView);
		pDialog->runModeless(pFrame);
	}
	return true;
}

Defun1(go)
{
	ABIWORD_VIEW;
	XAP_Dialog_Id id = AP_DIALOG_ID_GOTO;

	return s_doGotoDlg(pView, id);
}

/*****************************************************************/

static bool s_doSpellDlg(FV_View * pView, XAP_Dialog_Id id)
{
   XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
   UT_ASSERT(pFrame);
	
   pFrame->raise();
	
   XAP_DialogFactory * pDialogFactory
     = (XAP_DialogFactory *)(pFrame->getDialogFactory());
	
   AP_Dialog_Spell * pDialog
     = (AP_Dialog_Spell *)(pDialogFactory->requestDialog(id));
   UT_ASSERT(pDialog);
	
   // run the dialog (it probably should be modeless if anyone
   // gets the urge to make it safe that way)
   pDialog->runModal(pFrame);
   bool bOK = pDialog->isComplete();

   if (bOK)
	   s_TellSpellDone(pFrame);

   pDialogFactory->releaseDialog(pDialog);
	
   return bOK;
}


Defun1(dlgSpell)
{
   ABIWORD_VIEW;
   XAP_Dialog_Id id = AP_DIALOG_ID_SPELL;

   return s_doSpellDlg(pView,id);
}

/*****************************************************************/

static bool s_doFindOrFindReplaceDlg(FV_View * pView, XAP_Dialog_Id id)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Replace * pDialog
		= (AP_Dialog_Replace *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	// prime the dialog with a "find" string if there's a
	// current selection.
	if (!pView->isSelectionEmpty())
	{
		UT_UCSChar * buffer = pView->getSelectionText();
		
		pDialog->setFindString(buffer);

		FREEP(buffer);
	}
	
	// run the dialog (it should really be modeless if anyone
	// gets the urge to make it safe that way)
        // OK I Will
        if(pDialog->isRunning() == true)
	{
	       pDialog->activate();
	}
        else
	{
	       pDialog->runModeless(pFrame);
	}
	bool bOK = true;
	return bOK;
}


Defun1(find)
{
	ABIWORD_VIEW;
	XAP_Dialog_Id id = AP_DIALOG_ID_FIND;

	return s_doFindOrFindReplaceDlg(pView,id);
}

Defun1(findAgain)
{
	ABIWORD_VIEW;

	return pView->findAgain();
}

Defun1(replace)
{
	ABIWORD_VIEW;
	XAP_Dialog_Id id = AP_DIALOG_ID_REPLACE;

	return s_doFindOrFindReplaceDlg(pView,id);
}

/*****************************************************************/

static bool s_doLangDlg(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_Dialog_Id id = XAP_DIALOG_ID_LANGUAGE;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_Language * pDialog
		= (XAP_Dialog_Language *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	const XML_Char ** props_in = NULL;
	if (pView->getCharFormat(&props_in))
	{
		pDialog->setLanguageProperty(UT_getAttribute("lang", props_in));
		FREEP(props_in);
	}

	// run the dialog

	pDialog->runModal(pFrame);

	// extract what they did

	bool bOK = (pDialog->getAnswer() == XAP_Dialog_Language::a_OK);

	if (bOK)
	{
		//UT_DEBUGMSG(("pressed OK\n"));
		UT_uint16 k = 0;
		const XML_Char * props_out[3];
		const XML_Char * s;

		if (pDialog->getChangedLangProperty(&s))
		{
			//UT_DEBUGMSG(("some change\n"));
			
			props_out[k++] = "lang";
			props_out[k++] = s;
		}

		props_out[k] = 0;						// put null after last pair.
		
		if (k > 0)								// if something changed
			pView->setCharFormat(props_out);
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

/*****************************************************************/

static bool s_doFontDlg(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_Dialog_Id id = XAP_DIALOG_ID_FONT;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_FontChooser * pDialog
		= (XAP_Dialog_FontChooser *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	// stuff the GR_Graphics into the dialog so that it
	// can query the system for font info relative to our
	// context.

	pDialog->setGraphicsContext(pView->getLayout()->getGraphics());

	// get current font info from pView

	const XML_Char ** props_in = NULL;
	if (pView->getCharFormat(&props_in))
	{
		// stuff font properties into the dialog.
		// for a/p which are constant across the selection (always
		// present) we will set the field in the dialog.  for things
		// which change across the selection, we ask the dialog not
		// to set the field (by passing null).

		pDialog->setFontFamily(UT_getAttribute("font-family", props_in));
		pDialog->setFontSize(UT_getAttribute("font-size", props_in));
		pDialog->setFontWeight(UT_getAttribute("font-weight", props_in));
		pDialog->setFontStyle(UT_getAttribute("font-style", props_in));
		pDialog->setColor(UT_getAttribute("color", props_in));

		// these behave a little differently since they are
		// probably just check boxes and we don't have to
		// worry about initializing a combo box with a choice
		// (and because they are all stuck under one CSS attribute).

		bool bUnderline = false;
		bool bOverline = false;
		bool bStrikeOut = false;
		const XML_Char * s = UT_getAttribute("text-decoration", props_in);
		if (s)
		{
			bUnderline = (strstr(s, "underline") != NULL);
			bOverline = (strstr(s, "overline") != NULL);
			bStrikeOut = (strstr(s, "line-through") != NULL);
		}
		pDialog->setFontDecoration(bUnderline,bOverline,bStrikeOut);
/*
#ifdef BIDI_ENABLED
		bool bDirection;
		s = UT_getAttribute("dir", props_in);
		if (s)
		{
		    bDirection = (strstr(s, "rtl") != NULL);
		}
        	pDialog->setDirection(bDirection);
#endif
*/
		free(props_in);
	}

	// run the dialog

	pDialog->runModal(pFrame);

	// extract what they did

	bool bOK = (pDialog->getAnswer() == XAP_Dialog_FontChooser::a_OK);

	if (bOK)
	{
		UT_uint32  k = 0;
		const XML_Char * props_out[17];
		const XML_Char * s;

		if (pDialog->getChangedFontFamily(&s))
		{
			props_out[k++] = "font-family";
			props_out[k++] = s;
		}

		if (pDialog->getChangedFontSize(&s))
		{
			props_out[k++] = "font-size";
			props_out[k++] = s;
		}

		if (pDialog->getChangedFontWeight(&s))
		{
			props_out[k++] = "font-weight";
			props_out[k++] = s;
		}

		if (pDialog->getChangedFontStyle(&s))
		{
			props_out[k++] = "font-style";
			props_out[k++] = s;
		}

		if (pDialog->getChangedColor(&s))
		{
			props_out[k++] = "color";
			props_out[k++] = s;
		}

		bool bUnderline = false;
		bool bChangedUnderline = pDialog->getChangedUnderline(&bUnderline);
		bool bOverline = false;
		bool bChangedOverline = pDialog->getChangedOverline(&bOverline);
		bool bStrikeOut = false;
		bool bChangedStrikeOut = pDialog->getChangedStrikeOut(&bStrikeOut);
/*
#ifdef BIDI_ENABLED
		bool bDirection = false;
		bool bChangedDirection = pDialog->getChangedDirection(&bDirection);
#endif
*/

		if (bChangedUnderline || bChangedStrikeOut || bChangedOverline)
		{
			if (bUnderline && bStrikeOut && bOverline)
				s = "underline line-through overline";
			else if (bUnderline && bOverline)
				s = "underline overline";
			else if (bStrikeOut && bOverline)
				s = "line-through overline";
			else if (bStrikeOut && bUnderline)
				s = "line-through underline";
			else if (bStrikeOut)
				s = "line-through";
			else if (bUnderline)
				s = "underline";
			else if (bOverline)
				s = "overline";
			else
				s = "none";

			props_out[k++] = "text-decoration";
			props_out[k++] = s;
		}
/*
#ifdef BIDI_ENABLED
		if(bChangedDirection)
		{
		    if (bDirection == 1)
		        s = "rtl";
		    else
		        s = "ltr";
		
		    props_out[k++] = "dir";
		    props_out[k++] = s;
		}
#endif
*/
		props_out[k] = 0;						// put null after last pair.
		UT_ASSERT(k < NrElements(props_out));

		if (k > 0)								// if something changed
			pView->setCharFormat(props_out);
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

static bool s_doTabDlg(FV_View * pView)
{


	XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Tab * pDialog
		= (AP_Dialog_Tab *)(pDialogFactory->requestDialog(AP_DIALOG_ID_TAB));
	
	if(pDialog)
	{

		// run the dialog
		pDialog->runModal(pFrame);

		// get the dialog answer
		AP_Dialog_Tab::tAnswer answer = pDialog->getAnswer();

		switch (answer)
		{
		case AP_Dialog_Tab::a_OK:
			
			break;
			
		case AP_Dialog_Tab::a_CANCEL:
			// do nothing
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
				
		pDialogFactory->releaseDialog(pDialog);
	}
	else
	{
		s_TellNotImplemented(pFrame, "Tabs dialog", __LINE__);
	}
	return true;	
}


static bool s_doParagraphDlg(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Paragraph * pDialog
		= (AP_Dialog_Paragraph *)(pDialogFactory->requestDialog(AP_DIALOG_ID_PARAGRAPH));
	UT_ASSERT(pDialog);

	const XML_Char ** props = NULL;

	if (!pView->getBlockFormat(&props))
		return false;

	if (!pDialog->setDialogData(props))
		return false;

	FREEP(props);

	if (!pView->getSectionFormat(&props))
		return false;

	if (!pDialog->setDialogData(props))
		return false;

	FREEP(props);
	
	// run the dialog
	pDialog->runModal(pFrame);

	// get the dialog answer
	AP_Dialog_Paragraph::tAnswer answer = pDialog->getAnswer();

	const XML_Char ** propitem = NULL;

	switch (answer)
	{
	case AP_Dialog_Paragraph::a_OK:

		// getDialogData() returns us XML_Char ** data we have to free
		pDialog->getDialogData(props);
		UT_ASSERT(props);

		// set properties back to document
		if (props && props[0])
			pView->setBlockFormat(props);

		// we have to loop through the props pairs, freeing each string
		// referenced, then freeing the pointers themselves
		if (props)
		{
			propitem = props;

			while (propitem[0] && propitem[1])
			{
				FREEP(propitem[0]);
				FREEP(propitem[1]);
				propitem += 2;
			}
		}

		// now free props
		FREEP(props);

		break;
		
	case AP_Dialog_Paragraph::a_TABS:

		s_doTabDlg(pView);

		break;
		
	case AP_Dialog_Paragraph::a_CANCEL:
		// do nothing
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
			
	pDialogFactory->releaseDialog(pDialog);

	return true;	
}


static bool s_doOptionsDlg(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Options * pDialog
		= (AP_Dialog_Options *)(pDialogFactory->requestDialog(AP_DIALOG_ID_OPTIONS));
	UT_ASSERT(pDialog);

	// run the dialog
	pDialog->runModal(pFrame);

	// get the dialog answer
	AP_Dialog_Options::tAnswer answer = pDialog->getAnswer();

	switch (answer)
	{
	case AP_Dialog_Options::a_OK:
		
		break;
		
	case AP_Dialog_Options::a_CANCEL:
		// do nothing
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
			
	pDialogFactory->releaseDialog(pDialog);

	return true;	
}

/****************************************************************/

Defun1(dlgLanguage)
{
	ABIWORD_VIEW;

	return s_doLangDlg(pView);
}

Defun(language)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "lang", NULL, 0};
	properties[1] = (const XML_Char *) pCallData->m_pData;
	pView->setCharFormat(properties);
	return true;
}

/*****************************************************************/

Defun1(dlgFont)
{
	ABIWORD_VIEW;

	return s_doFontDlg(pView);
}

Defun(fontFamily)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "font-family", NULL, 0};
	properties[1] = (const XML_Char *) pCallData->m_pData;
	pView->setCharFormat(properties);
	return true;
}

Defun(fontSize)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "font-size", NULL, 0};

	// BUGBUG: stupid casting trick will eventually bite us
	const XML_Char * sz = (const XML_Char *) pCallData->m_pData;

	if (sz && *sz)
	{
		UT_String buf = sz;
		buf += "pt";

		properties[1] = (XML_Char *)buf.c_str();
		pView->setCharFormat(properties);
	}
	return true;
}

/*****************************************************************/

static bool _toggleSpanOrBlock(FV_View * pView,
				  const XML_Char * prop,
				  const XML_Char * vOn,
				  const XML_Char * vOff,
				  bool bMultiple,
				  bool isSpan)
{
	const XML_Char * props_out[] =	{ NULL, NULL, 0};

	// get current font info from pView
	const XML_Char ** props_in = NULL;
	const XML_Char * s;

	if (isSpan)
	{
		if (!pView->getCharFormat(&props_in))
		return false;
	}
	else // isBlock
	{
		if (!pView->getBlockFormat(&props_in))
		return false;
	}

	props_out[0] = prop;
	props_out[1] = vOn;		// be optimistic

	XML_Char * buf = NULL;

	s = UT_getAttribute(prop, props_in);
	if (s)
	{
		if (bMultiple)
		{
			// some properties have multiple values
			XML_Char*	p = strstr(s, vOn);

			if (p)
			{
				// yep...
				if (strstr(s, vOff))
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

				// ... take it out
				int len = strlen(s);
				buf = (XML_Char *) calloc(len, sizeof(XML_Char));

				strncpy(buf, s, p - s);
				strcat(buf, s + (p - s) + strlen(vOn));

				// now see if anything's left
				XML_Char * q;
				UT_cloneString((char *&)q, buf);

				if (q && strtok(q, " "))
					props_out[1] = buf;		// yep, use it
				else
					props_out[1] = vOff;	// nope, clear it

				free(q);
			}
			else
			{
				// nope...
				if (UT_stricmp(s, vOff))
				{
					// ...put it in by appending to current contents
					int len = strlen(s) + strlen(vOn) + 2;
					buf = (XML_Char *) calloc(len, sizeof(XML_Char));

					strcpy(buf, s);
					strcat(buf, " ");
					strcat(buf, vOn);
					props_out[1] = buf;
				}
			}
		}
		else
		{
			if (0 == UT_stricmp(s, vOn))
				props_out[1] = vOff;
		}
	}
	FREEP(props_in);


	// set it either way
	
	if (isSpan)
	  pView->setCharFormat(props_out);
	else // isBlock
	  pView->setBlockFormat(props_out);

	FREEP(buf);
	return true;
}

static bool _toggleSpan(FV_View * pView,
			   const XML_Char * prop,
			   const XML_Char * vOn,
			   const XML_Char * vOff,
			   bool bMultiple=false)
{
  return _toggleSpanOrBlock (pView, prop, vOn, vOff, bMultiple, true);
}


/*****************************************************************/
/*****************************************************************/

static bool s_actuallyPrint(PD_Document *doc,  GR_Graphics *pGraphics,
			       FV_View * pPrintView, const char *pDocName,
			       UT_uint32 nCopies, bool bCollate,
			       UT_sint32 iWidth,  UT_sint32 iHeight,
			       UT_uint32 nToPage, UT_uint32 nFromPage)
{
        UT_uint32 j,k;

        dg_DrawArgs da;
	memset(&da, 0, sizeof(da));
	da.pG = NULL;

	fp_PageSize ps = pPrintView->getPageSize();

	pGraphics->setPortrait (ps.isPortrait ());

	if(pGraphics->startPrint())
	  {
	    if (bCollate)
	      {
		for (j=1; (j <= nCopies); j++)
		  for (k=nFromPage; (k <= nToPage); k++)
		    {
		      // NB we will need a better way to calc
		      // pGraphics->m_iRasterPosition when
		      // iHeight is allowed to vary page to page
		      pGraphics->m_iRasterPosition = (k-1)*iHeight;
		      pGraphics->startPage(pDocName, k, true, iWidth, iHeight);
		      pPrintView->draw(k-1, &da);
		    }
	      }
	    else
	      {
		for (k=nFromPage; (k <= nToPage); k++)
		  for (j=1; (j <= nCopies); j++)
		    {
		      // NB we will need a better way to calc
		      // pGraphics->m_iRasterPosition when
		      // iHeight is allowed to vary page to page
		      pGraphics->m_iRasterPosition = (k-1)*iHeight;
		      pGraphics->startPage(pDocName, k, true, iWidth, iHeight);
		      pPrintView->draw(k-1, &da);
		    }
	      }
	    pGraphics->endPrint();
	  }
	
	return true;
}

static bool s_doPrint(FV_View * pView, bool bTryToSuppressDialog,bool bPrintDirectly)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_Print * pDialog
		= (XAP_Dialog_Print *)(pDialogFactory->requestDialog(bPrintDirectly? XAP_DIALOG_ID_PRINT_DIRECTLY: XAP_DIALOG_ID_PRINT));
	UT_ASSERT(pDialog);

	FL_DocLayout* pLayout = pView->getLayout();
	PD_Document * doc = pLayout->getDocument();

	pDialog->setPaperSize (pView->getPageSize().getPredefinedName());
	pDialog->setDocumentTitle(pFrame->getTempNameFromTitle());
	pDialog->setDocumentPathname((doc->getFilename())
								 ? doc->getFilename()
								 : pFrame->getTempNameFromTitle());
	pDialog->setEnablePageRangeButton(true,1,pLayout->countPages());
	pDialog->setEnablePrintSelection(false);	// TODO change this when we know how to do it.
	pDialog->setEnablePrintToFile(true);
	pDialog->setTryToBypassActualDialog(bTryToSuppressDialog);

	pDialog->runModal(pFrame);

	XAP_Dialog_Print::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_Print::a_OK);

	if (bOK)
	{
		GR_Graphics * pGraphics = pDialog->getPrinterGraphicsContext();
		UT_ASSERT(pGraphics->queryProperties(GR_Graphics::DGP_PAPER));
		
		FL_DocLayout * pDocLayout = new FL_DocLayout(doc,pGraphics);
		pDocLayout->formatAll();
		FV_View * pPrintView = new FV_View(pFrame->getApp(),pFrame,pDocLayout);
		UT_uint32 nFromPage, nToPage;
		(void)pDialog->getDoPrintRange(&nFromPage,&nToPage);

		if (nToPage > pDocLayout->countPages())
		{
			nToPage = pDocLayout->countPages();
		}
		
		// TODO add code to handle getDoPrintSelection()

		UT_uint32 nCopies = pDialog->getNrCopies();
		bool bCollate = pDialog->getCollate();

		// TODO these are here temporarily to make printing work.  We'll fix the hack later.
		// BUGBUG assumes all pages are same size and orientation
		UT_sint32 iWidth = pDocLayout->getWidth();
		UT_sint32 iHeight = pDocLayout->getHeight() / pDocLayout->countPages();

		const char *pDocName = ((doc->getFilename()) ? doc->getFilename() : pFrame->getTempNameFromTitle());

		s_actuallyPrint(doc, pGraphics, pPrintView, pDocName, nCopies, bCollate,
				iWidth,  iHeight, nToPage, nFromPage);

		delete pDocLayout;
		delete pPrintView;

		pDialog->releasePrinterGraphicsContext(pGraphics);
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

static bool s_doPrintPreview(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_PrintPreview * pDialog
		= (XAP_Dialog_PrintPreview *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_PRINTPREVIEW));
	UT_ASSERT(pDialog);

	FL_DocLayout* pLayout = pView->getLayout();
	PD_Document * doc = pLayout->getDocument();

	pDialog->setPaperSize (pView->getPageSize().getPredefinedName());
	pDialog->setDocumentTitle(pFrame->getTempNameFromTitle());
	pDialog->setDocumentPathname((doc->getFilename())
				     ? doc->getFilename()
				     : pFrame->getTempNameFromTitle());

	pDialog->runModal(pFrame);

	GR_Graphics * pGraphics = pDialog->getPrinterGraphicsContext();
	UT_ASSERT(pGraphics->queryProperties(GR_Graphics::DGP_PAPER));
	
	FL_DocLayout * pDocLayout = new FL_DocLayout(doc,pGraphics);
	pDocLayout->formatAll();
	FV_View * pPrintView = new FV_View(pFrame->getApp(),pFrame,pDocLayout);
	UT_uint32 nFromPage = 1, nToPage = pLayout->countPages();
	
	if (nToPage > pDocLayout->countPages())
	  {
	    nToPage = pDocLayout->countPages();
	  }
		
	UT_uint32 nCopies = 1;
	bool bCollate  = false;
	
	// TODO these are here temporarily to make printing work.  We'll fix the hack later.
	// BUGBUG assumes all pages are same size and orientation
	UT_sint32 iWidth = pDocLayout->getWidth();
	UT_sint32 iHeight = pDocLayout->getHeight() / pDocLayout->countPages();
	
	const char *pDocName = ((doc->getFilename()) ? doc->getFilename() : pFrame->getTempNameFromTitle());
	
	s_actuallyPrint(doc, pGraphics, pPrintView, pDocName, nCopies, bCollate,
			iWidth,  iHeight, nToPage, nFromPage);

	delete pDocLayout;
	delete pPrintView;
	
	pDialog->releasePrinterGraphicsContext(pGraphics);

	pDialogFactory->releaseDialog(pDialog);
				
        return true;
}

static bool s_doZoomDlg(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_Zoom * pDialog
		= (XAP_Dialog_Zoom *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_ZOOM));
	UT_ASSERT(pDialog);

	pDialog->setZoomPercent(pFrame->getZoomPercentage());
	pDialog->setZoomType(pFrame->getZoomType());
	
	pDialog->runModal(pFrame);

	XAP_Dialog_Zoom::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_Zoom::a_OK);

	if (bOK)
	{
		UT_uint32 newZoom = pFrame->getZoomPercentage();
		pFrame->setZoomType(pDialog->getZoomType());
		switch(pDialog->getZoomType())
		{
		// special cases
		case XAP_Frame::z_PAGEWIDTH:
			newZoom = pView->calculateZoomPercentForPageWidth();
			break;
		case XAP_Frame::z_WHOLEPAGE:
			newZoom = pView->calculateZoomPercentForWholePage();
			break;
		default:
			newZoom = pDialog->getZoomPercent();
		}

		pFrame->setZoomPercentage(newZoom);
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

static bool s_doBreakDlg(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Break * pDialog
		= (AP_Dialog_Break *)(pDialogFactory->requestDialog(AP_DIALOG_ID_BREAK));
	UT_ASSERT(pDialog);

	pDialog->runModal(pFrame);

	AP_Dialog_Break::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_Break::a_OK);

	if(pView->isHdrFtrEdit())
		return false;

	if (bOK)
	{
		UT_UCSChar c;
		switch(pDialog->getBreakType())
		{
		// special cases
		case AP_Dialog_Break::b_PAGE:
			c = UCS_FF;
			pView->cmdCharInsert(&c,1);
			break;
		case AP_Dialog_Break::b_COLUMN:
		        c = UCS_VTAB;
			pView->cmdCharInsert(&c,1);
			break;
		case AP_Dialog_Break::b_NEXTPAGE:
		        pView->insertSectionBreak(BreakSectionNextPage);
			break;
		case AP_Dialog_Break::b_CONTINUOUS:
		        pView->insertSectionBreak(BreakSectionContinuous);
			break;
		case AP_Dialog_Break::b_EVENPAGE:
		        pView->insertSectionBreak(BreakSectionEvenPage);
			break;
		case AP_Dialog_Break::b_ODDPAGE:
		        pView->insertSectionBreak(BreakSectionOddPage);
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

static UT_Dimension
fp_2_dim (fp_PageSize::Unit u)
{
  switch (u)
    {
    case fp_PageSize::cm   : return DIM_CM;
    case fp_PageSize::mm   : return DIM_MM;
    case fp_PageSize::inch :
    default :
      return DIM_IN;
    }
}

static bool s_doPageSetupDlg (FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	pFrame->raise();
	XAP_DialogFactory * pDialogFactory
	  = (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_PageSetup * pDialog =
	  (AP_Dialog_PageSetup *)(pDialogFactory->requestDialog(AP_DIALOG_ID_FILE_PAGESETUP));

	UT_ASSERT(pDialog);

	PD_Document * pDoc = pView->getLayout()->getDocument();
	//
	// Need this for the conversion methods
	//
	fp_PageSize::Predefined orig_def,final_def;
	fp_PageSize pSize(pDoc->m_docPageSize.getPredefinedName());
	orig_def = pSize.NameToPredefined(pSize.getPredefinedName());
	//
	// Set first page of the dialog properties.
	//
	pDialog->setPageSize(pSize);
	pDialog->setPageSize(orig_def);
	AP_Dialog_PageSetup::Orientation orig_ori,final_ori;
	orig_ori =  AP_Dialog_PageSetup::PORTRAIT;
	if(pDoc->m_docPageSize.isPortrait() == false)
	       orig_ori = AP_Dialog_PageSetup::LANDSCAPE;
	pDialog->setPageOrientation(orig_ori);
	fp_PageSize::Unit orig_unit,final_unit,orig_margu,final_margu;
	double orig_scale,final_scale;
	orig_unit = pDoc->m_docPageSize.getUnit();
	orig_scale = pDoc->m_docPageSize.getScale();

	pDialog->setPageUnits(orig_unit);
	pDialog->setPageScale((int)(100.0*orig_scale));

	//
	// Set the second page of info
	// All the page and header/footer margins
	//
	const XML_Char ** props_in = NULL;
	const char* pszLeftMargin = NULL;
	const char* pszTopMargin = NULL;
	const char* pszRightMargin = NULL;
	const char* pszBottomMargin = NULL;
	const char* pszFooterMargin = NULL;
	const char* pszHeaderMargin = NULL;
	double dLeftMargin = 1.0;
	double dRightMargin=1.0; 
	double dTopMargin = 1.0;
	double dBottomMargin = 1.0;
	double dFooterMargin= 0.0;
	double dHeaderMargin = 0.0;
	UT_Dimension docMargUnits = DIM_IN;

	bool bResult = pView->getSectionFormat(&props_in);
	if (!bResult)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	if(props_in && props_in[0])
	{
	        pszLeftMargin = UT_getAttribute("page-margin-left", props_in);
		if(pszLeftMargin)
		{
		       dLeftMargin = UT_convertToInches(pszLeftMargin);
		       docMargUnits = UT_determineDimension(pszLeftMargin);
		}

	        pszRightMargin = UT_getAttribute("page-margin-right", props_in);
		if(pszRightMargin)
		{
		       dRightMargin = UT_convertToInches(pszRightMargin);
		       docMargUnits = UT_determineDimension(pszRightMargin);
		}

	        pszTopMargin = UT_getAttribute("page-margin-top", props_in);
		if(pszTopMargin)
		{
		       dTopMargin = UT_convertToInches(pszTopMargin);
		       docMargUnits = UT_determineDimension(pszTopMargin);
		}

	        pszBottomMargin = UT_getAttribute("page-margin-bottom", props_in);
		if(pszBottomMargin)
		{
		       dBottomMargin = UT_convertToInches(pszBottomMargin);
		       docMargUnits = UT_determineDimension(pszBottomMargin);
		}

	        pszFooterMargin = UT_getAttribute("page-margin-footer", props_in);
		if(pszFooterMargin)
		       dFooterMargin = UT_convertToInches(pszFooterMargin);

	        pszHeaderMargin = UT_getAttribute("page-margin-header", props_in);
		if(pszHeaderMargin)
		       dHeaderMargin = UT_convertToInches(pszHeaderMargin);
	}
	FREEP(props_in);
	orig_margu = fp_PageSize::inch;
	if(docMargUnits == DIM_MM)
	{
	        dLeftMargin = dLeftMargin * 25.4;
	        dRightMargin = dRightMargin * 25.4;
	        dTopMargin = dTopMargin * 25.4;
	        dBottomMargin = dBottomMargin * 25.4;
	        dFooterMargin = dFooterMargin * 25.4;
	        dHeaderMargin = dHeaderMargin * 25.4;
		orig_margu = fp_PageSize::mm;
	}
	else if(docMargUnits == DIM_CM)
	{
	        dLeftMargin = dLeftMargin * 2.54;
	        dRightMargin = dRightMargin * 2.54;
	        dTopMargin = dTopMargin * 2.54;
	        dBottomMargin = dBottomMargin * 2.54;
	        dFooterMargin = dFooterMargin * 2.54;
	        dHeaderMargin = dHeaderMargin * 2.54;
		orig_margu = fp_PageSize::cm;
	}

	//
	// OK set all page two stuff
	//
	pDialog->setMarginUnits(orig_margu);
	pDialog->setMarginTop((float) dTopMargin);
	pDialog->setMarginBottom((float) dBottomMargin);
	pDialog->setMarginLeft((float) dLeftMargin);
	pDialog->setMarginRight((float) dRightMargin);
	pDialog->setMarginHeader((float) dHeaderMargin);
	pDialog->setMarginFooter((float) dFooterMargin);

	pDialog->runModal (pFrame);

	AP_Dialog_PageSetup::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_PageSetup::a_OK);

	if(bOK == false)
	       return true;

	final_def = pSize.NameToPredefined(pDialog->getPageSize().getPredefinedName());
	final_ori = pDialog->getPageOrientation();
	final_unit = pDialog->getPageUnits();
	final_scale = pDialog->getPageScale()/100.0;
	if((final_def != orig_def) || (final_ori != orig_ori) || (final_unit != orig_unit) || ((final_scale-orig_scale) > 0.001) || ((final_scale-orig_scale) < -0.001) )
	{
	  //
	  // Set the new Page Stuff
	  //
	       pDoc->m_docPageSize.Set(pSize.PredefinedToName(final_def));
	       pDoc->m_docPageSize.Set(final_unit);
	       bool p = (final_ori == AP_Dialog_PageSetup::PORTRAIT);
	       if( p == true)
	       { 
		      pDoc->m_docPageSize.setPortrait();
	       }
	       else
	       { 
		      pDoc->m_docPageSize.setLandscape();
	       }
	       pDoc->m_docPageSize.setScale(final_scale);

	       //
	       // Get all clones of this frame and set the new page dimensions
	       //
		  
	       UT_Vector vClones;
	       if(pFrame->getViewNumber() > 0)
	       {
		      pApp->getClones(&vClones,pFrame);
		      for (UT_uint32 i = 0; i < vClones.getItemCount(); i++)
		      {
			     XAP_Frame * f = (XAP_Frame *) vClones.getNthItem(i);
				 FV_View * pV =  (FV_View *) f->getCurrentView();
				 if(pV->isHdrFtrEdit())
				 {
					 pV->eraseInsertionPoint();
					 pV->clearHdrFtrEdit();
					 pV->warpInsPtToXY(0,0,false);
				 }
			     UT_uint32 izoom = f->getZoomPercentage();
			     f->setZoomPercentage(izoom);
		      }
	       }
	       else
	       {
			   FV_View * pV =  (FV_View *) pFrame->getCurrentView();
			   if(pV->isHdrFtrEdit())
			   {
				   pV->clearHdrFtrEdit();
				   pV->warpInsPtToXY(0,0,false);
			   }
		      UT_uint32 izoom = pFrame->getZoomPercentage();
		      pFrame->setZoomPercentage(izoom);
	       }
	}

	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);	
	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	UT_ASSERT(pPrefsScheme);

	pPrefsScheme->setValue((XML_Char*)AP_PREF_KEY_RulerUnits,
			       (XML_Char*)UT_dimensionName(fp_2_dim (final_unit)));

	//
	// Recover ppView
	//
	FV_View * ppView = (FV_View *) pFrame->getCurrentView();
	//
	// Now gather all the margin properties...
	//
	
	UT_String szLeftMargin;
	UT_String szTopMargin;
	UT_String szRightMargin;
	UT_String szBottomMargin;
	UT_String szFooterMargin;
	UT_String szHeaderMargin;

	final_margu = pDialog->getMarginUnits();
	dTopMargin = (double) pDialog->getMarginTop();
	dBottomMargin = (double) pDialog->getMarginBottom();
	dLeftMargin = (double) pDialog->getMarginLeft();
	dRightMargin = (double) pDialog->getMarginRight();
	dHeaderMargin = (double) pDialog->getMarginHeader();
	dFooterMargin = (double) pDialog->getMarginFooter();

	docMargUnits = DIM_IN;
	if(final_margu == fp_PageSize::cm)
	{
	       docMargUnits = DIM_CM;
	       dLeftMargin = dLeftMargin / 2.54;
	       dRightMargin = dRightMargin / 2.54;
	       dTopMargin = dTopMargin / 2.54;
	       dBottomMargin = dBottomMargin / 2.54;
	       dFooterMargin = dFooterMargin / 2.54;
	       dHeaderMargin = dHeaderMargin / 2.54;
	}
	else if (final_margu == fp_PageSize::mm)
	{
	       docMargUnits = DIM_MM;
	       dLeftMargin = dLeftMargin / 25.4;
	       dRightMargin = dRightMargin / 25.4;
	       dTopMargin = dTopMargin / 25.4;
	       dBottomMargin = dBottomMargin / 25.4;
	       dFooterMargin = dFooterMargin / 25.4;
	       dHeaderMargin = dHeaderMargin / 25.4;
	}
	//
	// Convert them into const char strings and change the section format
	//
	UT_Vector v;
	szLeftMargin = UT_convertInchesToDimensionString(docMargUnits,dLeftMargin);
	v.addItem((void *)"page-margin-left");
	v.addItem((void *) szLeftMargin.c_str());

	szRightMargin = UT_convertInchesToDimensionString(docMargUnits,dRightMargin);
	v.addItem((void *)"page-margin-right");
	v.addItem((void *) szRightMargin.c_str());

	szTopMargin = UT_convertInchesToDimensionString(docMargUnits,dTopMargin);
	v.addItem((void *)"page-margin-top");
	v.addItem((void *) szTopMargin.c_str());

	szBottomMargin = UT_convertInchesToDimensionString(docMargUnits,dBottomMargin);
	v.addItem((void *)"page-margin-bottom");
	v.addItem((void *) szBottomMargin.c_str());

	szFooterMargin = UT_convertInchesToDimensionString(docMargUnits,dFooterMargin);
	v.addItem((void *)"page-margin-footer");
	v.addItem((void *) szFooterMargin.c_str());

	szHeaderMargin = UT_convertInchesToDimensionString(docMargUnits,dHeaderMargin);
	v.addItem((void *)"page-margin-header");
	v.addItem((void *) szHeaderMargin.c_str());

	UT_uint32 countv = v.getItemCount() + 1;
	const XML_Char ** props = (const XML_Char **) calloc(countv, sizeof(XML_Char *));
	UT_uint32 i;
	for(i=0; i<v.getItemCount();i++)
	{
		props[i] = (XML_Char *) v.getNthItem(i);
	}
	props[i] = (XML_Char *) NULL;
	if(ppView->isHdrFtrEdit())
	{
		ppView->clearHdrFtrEdit();
		ppView->warpInsPtToXY(0,0,false);
	}

	//
	// Finally we've got it all in place, Make the change!
	//

	ppView->setSectionFormat(props);
	FREEP(props);
	return true;
}

class FV_View_Insert_symbol_listener : public XAP_Insert_symbol_listener
	{
	public:

		void setView( AV_View * pJustFocussedView)
	        {
			p_view = (FV_View *) pJustFocussedView ;
	        }
		bool insertSymbol(UT_UCSChar Char, char *p_font_name)
		{
			UT_ASSERT(p_view != NULL);

			p_view->insertSymbol(Char, (XML_Char*)p_font_name);

			return true;
		}

	private:
		FV_View *p_view;
	};


static  FV_View_Insert_symbol_listener symbol_Listener;

static bool s_InsertSymbolDlg(FV_View * pView, XAP_Dialog_Id id  )
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();
	XAP_DialogFactory * pDialogFactory
	  = (XAP_DialogFactory *)(pFrame->getDialogFactory());
	
	XAP_Dialog_Insert_Symbol * pDialog
		= (XAP_Dialog_Insert_Symbol *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);
	
	if(pDialog->isRunning() == true)
	{
	       pDialog->activate();
	}
        else
	{
	       pDialog->setListener(&symbol_Listener);
	       pDialog->runModeless(pFrame);
		
	}
	return true;
}

/*****************************************************************/
/*****************************************************************/

Defun1(print)
{
	ABIWORD_VIEW;
	return s_doPrint(pView,false,false);
}

Defun1(printDirectly)
{
	ABIWORD_VIEW;
	return s_doPrint(pView,false,true);
}

Defun1(printTB)
{
	// print (intended to be from the tool-bar (where we'd like to
	// suppress the dialog if possible))

	ABIWORD_VIEW;
	return s_doPrint(pView,true,false);
}

Defun1(printPreview)
{
        ABIWORD_VIEW;
	return s_doPrintPreview(pView);
}

Defun1(pageSetup)
{
	ABIWORD_VIEW;
	return s_doPageSetupDlg(pView);
}

Defun1(dlgOptions)
{
	ABIWORD_VIEW;
	
	return s_doOptionsDlg(pView);
}


/*****************************************************************/
/*****************************************************************/

Defun1(viewStd)
{
	// TODO: Share this function with viewFormat & viewExtra
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();
	UT_ASSERT(pFrameData);

	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
	  return false;

	// toggle the ruler bit
	pFrameData->m_bShowBar[0] = ! pFrameData->m_bShowBar[0];

	// actually do the dirty work
	pFrame->toggleBar( 0, pFrameData->m_bShowBar[0] );

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValueBool((XML_Char*)AP_PREF_KEY_StandardBarVisible, pFrameData->m_bShowBar[0]);

	return true;
}

Defun1(viewFormat)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();
	UT_ASSERT(pFrameData);

	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
	  return false;

	// toggle the ruler bit
	pFrameData->m_bShowBar[1] = ! pFrameData->m_bShowBar[1];

	// actually do the dirty work
	pFrame->toggleBar( 1, pFrameData->m_bShowBar[1] );

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValueBool((XML_Char*)AP_PREF_KEY_FormatBarVisible, pFrameData->m_bShowBar[1]);

	return true;
}

Defun1(viewExtra)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_ASSERT(pFrameData);

	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
	  return false;

	// toggle the ruler bit
	pFrameData->m_bShowBar[2] = ! pFrameData->m_bShowBar[2];

	// actually do the dirty work
	pFrame->toggleBar( 2, pFrameData->m_bShowBar[2] );

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValueBool((XML_Char*)AP_PREF_KEY_ExtraBarVisible, pFrameData->m_bShowBar[2]);

	return true;
}

Defun1(viewNormalLayout)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();
	UT_ASSERT(pFrameData);

	pFrameData->m_pViewMode = VIEW_NORMAL;
	pFrame->toggleLeftRuler (false);

	FV_View * pView = static_cast<FV_View *>(pAV_View);
	pView->setViewMode (VIEW_NORMAL);

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValue(AP_PREF_KEY_LayoutMode, "2");

	return true;
}

Defun1(viewWebLayout)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();
	UT_ASSERT(pFrameData);

	pFrameData->m_pViewMode = VIEW_WEB;
	pFrame->toggleLeftRuler (false);

	FV_View * pView = static_cast<FV_View *>(pAV_View);
	pView->setViewMode (VIEW_WEB);

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValue(AP_PREF_KEY_LayoutMode, "3");

	return true;
}

Defun1(viewPrintLayout)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();
	UT_ASSERT(pFrameData);

	pFrameData->m_pViewMode = VIEW_PRINT;
	pFrame->toggleLeftRuler (true && (pFrameData->m_bShowRuler) &&
				 (!pFrameData->m_bIsFullScreen));

	FV_View * pView = static_cast<FV_View *>(pAV_View);
	pView->setViewMode (VIEW_PRINT);

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValue(AP_PREF_KEY_LayoutMode, "1");

	return true;
}

Defun1(viewStatus)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_ASSERT(pFrameData);
	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
	  return false;


	// toggle the view status bit
	pFrameData->m_bShowStatusBar = ! pFrameData->m_bShowStatusBar;

	// actually do the dirty work
	pFrame->toggleStatusBar(pFrameData->m_bShowStatusBar);

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValueBool((XML_Char*) AP_PREF_KEY_StatusBarVisible, pFrameData->m_bShowStatusBar);
	return true;
}

Defun1(viewRuler)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();
	UT_ASSERT(pFrameData);

	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
	  return false;

	// toggle the ruler bit
	pFrameData->m_bShowRuler = ! pFrameData->m_bShowRuler;

	// actually do the dirty work
	pFrame->toggleRuler(pFrameData->m_bShowRuler);

#if 1
	// POLICY: make this the default for new frames, too
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);
	pScheme->setValueBool((XML_Char*)AP_PREF_KEY_RulerVisible, pFrameData->m_bShowRuler);
#endif

	return true;
}

Defun1(viewFullScreen)
{
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
  AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();

  if(!pFrameData->m_bIsFullScreen) // we're hiding stuff
    {
      pFrame->toggleBar(0, false);
      pFrame->toggleBar(1, false);
      pFrame->toggleBar(2, false);
      pFrame->toggleStatusBar(false);
      pFrame->toggleRuler(false);
      pFrameData->m_bIsFullScreen = true;
    }
  else // we're (possibly) unhiding stuff
    {
      pFrame->toggleBar(0, pFrameData->m_bShowBar[0]);
      pFrame->toggleBar(1, pFrameData->m_bShowBar[1]);
      pFrame->toggleBar(2, pFrameData->m_bShowBar[2]);
      pFrame->toggleStatusBar(pFrameData->m_bShowStatusBar);
      pFrame->toggleRuler(pFrameData->m_bShowRuler);
      pFrameData->m_bIsFullScreen = false;
    }

  return true;
}

Defun1(viewPara)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();
	UT_ASSERT(pFrameData);

	pFrameData->m_bShowPara = !pFrameData->m_bShowPara;

	ABIWORD_VIEW;
	pView->setShowPara(pFrameData->m_bShowPara);
	pView->notifyListeners(AV_CHG_FRAMEDATA);	// to update toolbar

#if 1
	// POLICY: make this the default for new frames, too
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValueBool(AP_PREF_KEY_ParaVisible, pFrameData->m_bShowPara);
#endif

	return true;
}

Defun1(viewHeadFoot)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	// TODO: synch this implementation with ap_GetState_View
	s_TellNotImplemented(pFrame, "View Headers and Footers", __LINE__);
	return true;
}

Defun(zoom)
{
	ABIWORD_VIEW;

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	// TODO the cast below is ugly

	UT_uint32 iZoom = 0;
	char *p_zoom = (char*) (pCallData->m_pData);

	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	if(strcmp(p_zoom, pSS->getValue(XAP_STRING_ID_TB_Zoom_PageWidth)) == 0)
	{
		pFrame->setZoomType(XAP_Frame::z_PAGEWIDTH);
		iZoom = pView->calculateZoomPercentForPageWidth();
	}
	else if(strcmp(p_zoom, pSS->getValue(XAP_STRING_ID_TB_Zoom_WholePage)) == 0)
	{
		pFrame->setZoomType(XAP_Frame::z_WHOLEPAGE);
		iZoom = pView->calculateZoomPercentForWholePage();
	}
	else
	{
		pFrame->setZoomType(XAP_Frame::z_PERCENT);
		iZoom = atoi(p_zoom);
	}

	UT_ASSERT(iZoom > 0);

	pFrame->setZoomPercentage(iZoom);
	
	return true;
}

Defun1(dlgZoom)
{
	ABIWORD_VIEW;
	return s_doZoomDlg(pView);
}

static bool s_doInsertDateTime(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Insert_DateTime * pDialog
		= (AP_Dialog_Insert_DateTime *)(pDialogFactory->requestDialog(AP_DIALOG_ID_INSERT_DATETIME));
	UT_ASSERT(pDialog);

	pDialog->runModal(pFrame);

	if (pDialog->getAnswer() == AP_Dialog_Insert_DateTime::a_OK)
	{
		time_t	tim = time(NULL);
		struct tm *pTime = localtime(&tim);
		UT_UCSChar *CurrentDateTime = NULL;
		char szCurrentDateTime[CURRENT_DATE_TIME_SIZE];

		strftime(szCurrentDateTime,CURRENT_DATE_TIME_SIZE,pDialog->GetDateTimeFormat(),pTime);
		UT_UCS_cloneString_char(&CurrentDateTime,szCurrentDateTime);
		pView->cmdCharInsert(CurrentDateTime,UT_UCS_strlen(CurrentDateTime), true);
		FREEP(CurrentDateTime);
	}

	pDialogFactory->releaseDialog(pDialog);

	return true;
}

Defun1(insDateTime)
{
	ABIWORD_VIEW;
	return s_doInsertDateTime(pView);
}

/*****************************************************************/
/*****************************************************************/

Defun1(insBreak)
{
	ABIWORD_VIEW;
	return s_doBreakDlg(pView);
}

static bool s_doInsertPageNumbers(FV_View * pView)
{
        const XML_Char * right_attributes [] = {
	  "text-align", "right", NULL, NULL
	};
	
	const XML_Char * left_attributes [] = {
	  "text-align", "left", NULL, NULL
	};
	
	const XML_Char * center_attributes [] = {
	  "text-align", "center", NULL, NULL
	};

	const XML_Char ** atts = NULL;

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_PageNumbers * pDialog
		= (AP_Dialog_PageNumbers *)(pDialogFactory->requestDialog(AP_DIALOG_ID_PAGE_NUMBERS));
	UT_ASSERT(pDialog);

	pDialog->runModal(pFrame);

	if (pDialog->getAnswer() != AP_Dialog_PageNumbers::a_OK)
	{
		pDialogFactory->releaseDialog(pDialog);
		return true;
	}
	switch (pDialog->getAlignment())
	{
	    case AP_Dialog_PageNumbers::id_RALIGN : atts = right_attributes; break;
	    case AP_Dialog_PageNumbers::id_LALIGN : atts = left_attributes; break;
	    case AP_Dialog_PageNumbers::id_CALIGN : atts = center_attributes; break;
	    default: UT_ASSERT(UT_SHOULD_NOT_HAPPEN); break;
	}
	pView->processPageNumber(pDialog->isFooter(),atts);
	pDialogFactory->releaseDialog(pDialog);
	return true;
}

Defun1(insPageNo)
{
        ABIWORD_VIEW;
	return s_doInsertPageNumbers(pView);
}

static bool s_doField(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Field * pDialog
		= (AP_Dialog_Field *)(pDialogFactory->requestDialog(AP_DIALOG_ID_FIELD));
	UT_ASSERT(pDialog);

	pDialog->runModal(pFrame);

	if (pDialog->getAnswer() == AP_Dialog_Field::a_OK)
	{
		// TODO - Insert field correctly
		pView->cmdInsertField(pDialog->GetFieldFormat());
	}

	pDialogFactory->releaseDialog(pDialog);

	return true;
}

Defun1(insField)
{
	ABIWORD_VIEW;
	return s_doField(pView);
}

Defun1(insSymbol)
{

	ABIWORD_VIEW;
	XAP_Dialog_Id id = XAP_DIALOG_ID_INSERT_SYMBOL;

	return s_InsertSymbolDlg(pView,id);
}


/*****************************************************************/
/*****************************************************************/

Defun1(dlgParagraph)
{
	ABIWORD_VIEW;
	
	return s_doParagraphDlg(pView);
}

static bool s_doBullets(FV_View *pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);
	
	pFrame->raise();
	
	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());
	AP_Dialog_Lists * pDialog
		= (AP_Dialog_Lists *)(pDialogFactory->requestDialog(AP_DIALOG_ID_LISTS));
	UT_ASSERT(pDialog);

	if(pDialog->isRunning() == true)
	{
		pDialog->activate();
	}
	else
	{
		pDialog->runModeless(pFrame);
	}
	return true;
}


Defun1(dlgBullets)
{
  //
  // Dialog for Bullets and Lists
  //
#if defined(WIN32) || defined(__QNXTO__) || defined(__BEOS__) || defined(TARGET_OS_MAC)
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	s_TellNotImplemented(pFrame, "Lists dialog", __LINE__);
	return true;
#else // enable for GTK+ & Gnome builds only
	ABIWORD_VIEW;
	return s_doBullets(pView);
#endif
}

Defun1(dlgBorders)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	s_TellNotImplemented(pFrame, "Border and shading dialog", __LINE__);
	return true;
}

static bool s_doColumnsDlg(FV_View * pView)
{
	XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Columns * pDialog
		= (AP_Dialog_Columns *)(pDialogFactory->requestDialog(AP_DIALOG_ID_COLUMNS));
	UT_ASSERT(pDialog);

	if(pView->isHdrFtrEdit())
		return false;

	UT_uint32 iColumns = 1;
	bool bLineBetween = false;

	const XML_Char ** props_in = NULL;
	const XML_Char * sz = NULL;

    bool bResult = pView->getSectionFormat(&props_in);

    if (!bResult)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	// NB: maybe *no* properties are consistent across the selection
	if (props_in && props_in[0])
		sz = UT_getAttribute("columns", props_in);

	if (sz)
	{
		iColumns = atoi(sz);
	}

	if (props_in && props_in[0])
		sz = UT_getAttribute("column-line", props_in);

	if (sz)
	{
		if(strcmp(sz, "on") == 0)
		{
			bLineBetween = true;
		}
	}
	
	pDialog->setColumns(iColumns);
	pDialog->setLineBetween(bLineBetween);
	pDialog->runModal(pFrame);

	AP_Dialog_Columns::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_Columns::a_OK);

	if (bOK)
	{
		// Set the columns property.

		char buf[4];
		sprintf(buf, "%i", pDialog->getColumns());
		char buf2[4];
		if(pDialog->getLineBetween())
		{
			strcpy(buf2, "on");
		}
		else
		{
			strcpy(buf2, "off");
		}

#ifndef __MRC__
		const XML_Char * properties[] =	{ "columns", buf, "column-line", buf2, 0};
#else
		const XML_Char * properties[] =	{ "columns", NULL, "column-line", NULL, 0};
		properties [1] = buf;
		properties [3] = buf2;
#endif
		pView->setSectionFormat(properties);
	}

	free(props_in);

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}


Defun1(dlgColumns)
{
	ABIWORD_VIEW;
	return s_doColumnsDlg(pView);
}

Defun(style)
{
	ABIWORD_VIEW;
	const XML_Char * style = (const XML_Char *) pCallData->m_pData;
	pView->setStyle(style);
	return true;
}


static bool s_doStylesDlg(FV_View * pView)
{
	XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Styles * pDialog
		= (AP_Dialog_Styles *)(pDialogFactory->requestDialog(AP_DIALOG_ID_STYLES));
	UT_ASSERT(pDialog);
	pDialog->runModal(pFrame);

	AP_Dialog_Styles::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_Styles::a_OK);
	return bOK;
}


Defun1(dlgStyle)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
#ifndef NDEBUG
	ABIWORD_VIEW;
	return s_doStylesDlg(pView);
#else
	s_TellNotImplemented(pFrame, "Styles dialog", __LINE__);
#endif
	return true;
}

Defun1(dlgTabs)
{
 	ABIWORD_VIEW;
 	
 	return s_doTabDlg(pView);
}

Defun0(noop)
{
	// this is a no-op, so unbound menus don't assert at trade shows
	return true;
}

static bool s_doWordCountDlg(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_WordCount * pDialog
		= (AP_Dialog_WordCount *)(pDialogFactory->requestDialog(AP_DIALOG_ID_WORDCOUNT));
	UT_ASSERT(pDialog);

	if(pDialog->isRunning())
	{
		pDialog->activate();
	}
	else
	{
		pDialog->setCount(pView->countWords());
		pDialog->runModeless(pFrame);
	}
	bool bOK = true;
	return bOK;
}


Defun1(dlgWordCount)
{
        ABIWORD_VIEW;
	
	return s_doWordCountDlg(pView);
}

/****************************************************************/
/****************************************************************/

Defun1(toggleBold)
{
	ABIWORD_VIEW;
	return _toggleSpan(pView, "font-weight", "bold", "normal");
}

Defun1(toggleItalic)
{
	ABIWORD_VIEW;
	return _toggleSpan(pView, "font-style", "italic", "normal");
}

Defun1(toggleUline)
{
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-decoration", "underline", "none", true);
}
Defun1(toggleOline)
{
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-decoration", "overline", "none", true);
}

Defun1(toggleStrike)
{
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-decoration", "line-through", "none", true);
}

// MSWord defines this to 1/2 an inch, so we do too
#define TOGGLE_INDENT_AMT 0.5

Defun1(toggleIndent)
{
  ABIWORD_VIEW;
  bool ret;
  bool doLists = true;
  double page_size = pView->getPageSize().Width (fp_PageSize::inch);
  if(!pView->getCurrentBlock()->isListItem() || !pView->isSelectionEmpty() )
  {
	 doLists = false;
  }
  ret = pView->setBlockIndents(doLists, (double) TOGGLE_INDENT_AMT ,page_size);
  return ret;
}

Defun1(toggleUnIndent)
{
  ABIWORD_VIEW;
  bool ret;
  double page_size = pView->getPageSize().Width (fp_PageSize::inch);
  bool doLists = true;
  if(!pView->getCurrentBlock()->isListItem() || !pView->isSelectionEmpty() )
  {
	 doLists = false;
  }
  ret = pView->setBlockIndents(doLists, (double) -TOGGLE_INDENT_AMT ,page_size);
  return ret;
}

#undef TOGGLE_INDENT_AMT

Defun1(toggleSuper)
{
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-position", "superscript", "normal");
}

Defun1(toggleSub)
{
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-position", "subscript", "normal");
}

#ifdef BIDI_ENABLED
Defun1(toggleDirOverrideLTR)
{
	ABIWORD_VIEW;
	return _toggleSpan(pView, "dir-override", "ltr", "off");
}

Defun1(toggleDirOverrideRTL)
{
	ABIWORD_VIEW;
	return _toggleSpan(pView, "dir-override", "rtl", "off");
}

Defun1(toggleDomDirection)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "dom-dir", NULL, 0};
	const XML_Char drtl[] = "rtl";
	const XML_Char dltr[] = "ltr";

	if(pView->getCurrentBlock()->getDominantDirection())
	{
		properties[1] = (XML_Char *) &dltr;
	}
	else
	{
		properties[1] = (XML_Char *) &drtl;
	}

	pView->setBlockFormat(properties);
	return true;
}
#endif

Defun1(doBullets)
{
	ABIWORD_VIEW;
	pView->processSelectedBlocks(BULLETED_LIST);
	return true;
}

Defun1(doNumbers)
{
	ABIWORD_VIEW;
	pView->processSelectedBlocks(NUMBERED_LIST);
	return true;
}

Defun(colorForeTB)
{
  ABIWORD_VIEW;

  const XML_Char * properties[] = { "color", NULL, 0};
  properties[1] = (const XML_Char *) pCallData->m_pData;
  pView->setCharFormat(properties);

  return true;
}

Defun(colorBackTB)
{
        ABIWORD_VIEW;

	const XML_Char * properties[] = { "bgcolor", NULL, 0};
	properties[1] = (const XML_Char *) pCallData->m_pData;
	pView->setCharFormat(properties);

	return true;
}

Defun0(togglePlain)
{
	// TODO: remove all character-level formatting
	// HYP: explicitly delete it, to get back to defaults, styles
	return true;
}

Defun1(alignLeft)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "text-align", "left", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(alignCenter)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "text-align", "center", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(alignRight)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "text-align", "right", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(alignJustify)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "text-align", "justify", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(setStyleHeading1)
{
	ABIWORD_VIEW;
	const XML_Char * style = "Heading 1";
	pView->setStyle(style);
	return true;
}


Defun1(setStyleHeading2)
{
	ABIWORD_VIEW;
	const XML_Char * style = "Heading 2";
	pView->setStyle(style);
	return true;
}

Defun1(setStyleHeading3)
{
	ABIWORD_VIEW;
	const XML_Char * style = "Heading 3";
	pView->setStyle(style);
	return true;
}

Defun1(sectColumns1)
{
	ABIWORD_VIEW;
	if(pView->isHdrFtrEdit())
		return false;

	const XML_Char * properties[] =	{ "columns", "1", 0};
	pView->setSectionFormat(properties);
	return true;
}

Defun1(sectColumns2)
{
	ABIWORD_VIEW;
	if(pView->isHdrFtrEdit())
		return false;
	const XML_Char * properties[] =	{ "columns", "2", 0};
	pView->setSectionFormat(properties);
	return true;
}

Defun1(sectColumns3)
{
	ABIWORD_VIEW;
	if(pView->isHdrFtrEdit())
		return false;
	const XML_Char * properties[] =	{ "columns", "3", 0};
	pView->setSectionFormat(properties);
	return true;
}

Defun1(paraBefore0)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "margin-top", "0pt", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(paraBefore12)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "margin-top", "12pt", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(singleSpace)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "line-height", "1.0", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(middleSpace)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "line-height", "1.5", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(doubleSpace)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "line-height", "2.0", 0};
	pView->setBlockFormat(properties);
	return true;
}

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
Defun1(Test_Dump)
{
	ABIWORD_VIEW;
	pView->Test_Dump();
	return true;
}

Defun1(Test_Ftr)
{
	ABIWORD_VIEW;
	pView->insertPageNum(NULL, false);
	return true;
}
#endif

Defun1(setEditVI)
{
	ABIWORD_VIEW;
	// enter "VI Edit Mode" (only valid when VI keys are loaded)
	
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	// When exiting input mode, vi goes to previous character
	pView->cmdCharMotion(false,1);

	bool bResult = (pFrame->setInputMode("viEdit") != 0);
	return bResult;
}

Defun1(setInputVI)
{
	// enter "VI Input Mode" (only valid when VI keys are loaded)
	
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	bool bResult = (pFrame->setInputMode("viInput") != 0);
	return bResult;
}

Defun1(cycleInputMode)
{
	// switch to the next input mode { default, emacs, vi, ... }

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);

	// this edit method may get ignored entirely
	bool b;
	if (pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_KeyBindingsCycle, &b) && !b)
		return false;

	const char * szCurrentInputMode = pFrame->getInputMode();
	UT_ASSERT(szCurrentInputMode);
	const char * szNextInputMode = AP_BindingSet::s_getNextInCycle(szCurrentInputMode);
	if (!szNextInputMode)				// probably an error....
		return false;
	
	bool bResult = (pFrame->setInputMode(szNextInputMode) != 0);

#if 1
	// POLICY: make this the default for new frames, too
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValue((XML_Char*)AP_PREF_KEY_KeyBindings,
			  (XML_Char*)szNextInputMode);
#endif

	return bResult;
}

Defun1(toggleInsertMode)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);

	// this edit method may get ignored entirely
	bool b;
	if (pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_InsertModeToggle, &b) && !b)
		return false;

	// toggle the insert mode
	AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();
	UT_ASSERT(pFrameData);

	pFrameData->m_bInsertMode = ! pFrameData->m_bInsertMode;

	// the view actually does the dirty work
	pAV_View->setInsertMode(pFrameData->m_bInsertMode);

#if 1
	// POLICY: make this the default for new frames, too
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValueBool((XML_Char*)AP_PREF_KEY_InsertMode, pFrameData->m_bInsertMode); 
#endif

	return true;
}


//////////////////////////////////////////////////////////////////
// The following commands are suggested for the various VI keybindings.
// It may be possible to use our exisiting methods for them, but I
// didn't know all of the little (subtle) side-effects that make VI
// special.
//////////////////////////////////////////////////////////////////

Defun(viCmd_A)
{
	// insert after the end of the current line
	return ( EX(warpInsPtEOL) && EX(setInputVI) );
}

Defun(viCmd_C)
{
	// Select to the end of the line for modification
	return ( EX(extSelEOL) && EX(setInputVI) );
}

Defun(viCmd_I)
{
	// insert before the beginning of current line
	return ( EX(warpInsPtBOL) && EX(setInputVI) );
}

Defun(viCmd_J)
{
	// Join current and next line.
	return ( EX(warpInsPtEOL) && EX(delRight) && EX(insertSpace) );
}

Defun(viCmd_O)
{
	// insert new line before current line, go into input mode
	return ( EX(warpInsPtBOL) && EX(insertLineBreak) && EX(warpInsPtLeft) \
		&& EX(setInputVI) );
}

Defun(viCmd_P)
{
	// paste text before cursor
	return ( EX(warpInsPtLeft) && EX(paste) );
}

Defun(viCmd_a)
{
	// insert after the current position
	return ( EX(warpInsPtRight) && EX(setInputVI) );
}

Defun(viCmd_o)
{
	// insert new line after current line, go into input mode
	return ( EX(warpInsPtEOL) && EX(insertLineBreak) && EX(setInputVI) );
}

/* c$ */
Defun(viCmd_c24)
{
	//change to end of current line
	return ( EX(delEOL) && EX(setInputVI) );
}

/* c( */
Defun(viCmd_c28)
{
	//change to start of current sentence
	return ( EX(delBOS) && EX(setInputVI) );
}

/* c) */
Defun(viCmd_c29)
{
	//change to end of current sentence
	return ( EX(delEOS) && EX(setInputVI) );
}

/* c[ */
Defun(viCmd_c5b)
{
	//change to beginning of current block
	return ( EX(delBOB) && EX(setInputVI) );
}

/* c] */
Defun(viCmd_c5d)
{
	//change to end of current block
	return ( EX(delEOB) && EX(setInputVI) );
}

/* c^ */
Defun(viCmd_c5e)
{
	//change to beginning of current line
	return ( EX(delBOL) && EX(setInputVI) );
}

Defun(viCmd_cb)
{
	//change to beginning of current word
	return ( EX(delBOW) && EX(setInputVI) );
}

Defun(viCmd_cw)
{
	// delete to the end of current word, start input mode
	return ( EX(delEOW) && EX(setInputVI) );
}

/* d$ */
Defun(viCmd_d24)
{
	//delete to end of line
	return ( EX(extSelEOL) && EX(cut) );
}

/* d( */
Defun(viCmd_d28)
{
	//delete to start of sentence
	return ( EX(extSelBOS) && EX(cut) );
}

/* d) */
Defun(viCmd_d29)
{
	//delete to end of sentence
	return ( EX(extSelEOS) && EX(cut) );
}

/* d[ */
Defun(viCmd_d5b)
{
	//delete to beginning of block
	return ( EX(extSelBOB) && EX(cut) );
}

/* d] */
Defun(viCmd_d5d)
{
	//delete to end of block
	return ( EX(extSelEOB) && EX(cut) );
}

/* d^ */
Defun(viCmd_d5e)
{
	//delete to beginning of line
	return ( EX(extSelBOL) && EX(cut) );
}

Defun(viCmd_db)
{
	//delete to beginning of word
	return ( EX(extSelBOW) && EX(cut) );
}

Defun(viCmd_dd)
{
	// delete the current line
	return ( EX(warpInsPtBOL) && EX(extSelEOL) && EX(extSelRight) && EX(cut) );
}

Defun(viCmd_dw)
{
	//delete to end of word
	return ( EX(extSelEOW) && EX(cut) );
}

/* y$ */
Defun(viCmd_y24)
{
	//copy to end of current line
	return ( EX(extSelEOL) && EX(copy) );
}

/* y( */
Defun(viCmd_y28)
{
	//copy to beginning of current sentence
	return ( EX(extSelBOS) && EX(copy) );
}

/* y) */
Defun(viCmd_y29)
{
	//copy to end of current sentence
	return ( EX(extSelEOS) && EX(copy) );
}

/* y[ */
Defun(viCmd_y5b)
{
	//copy to beginning of current block
	return ( EX(extSelBOB) && EX(copy) );
}

/* y] */
Defun(viCmd_y5d)
{
	//copy to end of current block
	return ( EX(extSelEOB) && EX(copy) );
}

/* y^ */
Defun(viCmd_y5e)
{
	//copy to beginning of current line
	return ( EX(extSelBOL) && EX(copy) );
}

Defun(viCmd_yb)
{
	//copy to beginning of current word
	return ( EX(extSelBOW) && EX(copy) );
}

Defun(viCmd_yw)
{
	//copy to end of current word
	return ( EX(extSelEOW) && EX(copy) );
}

Defun(viCmd_yy)
{
	//copy current line
	return ( EX(warpInsPtBOL) && EX(extSelEOL) && EX(copy) );
}

bool _insAutotext (FV_View *pView, int id)
{
        XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	if (!pFrame)
	  return false;

	XAP_App * pApp = pFrame->getApp();
	if (!pApp)
	  return false;

	const XAP_StringSet * pSS = pApp->getStringSet();
	if (!pSS)
	  return false;

	const char * text = (const char *)pSS->getValue(id);
	UT_uint32 len = strlen (text);

	UT_UCSChar * ucstext = new UT_UCSChar [len + 1]; 
	UT_UCS_strcpy_char (ucstext, text);

	pView->cmdCharInsert(ucstext, len);

	delete [] ucstext;

	return true;
}

Defun(insAutotext_attn_1)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_ATTN_1);
}

Defun(insAutotext_attn_2)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_ATTN_2);
}

Defun(insAutotext_closing_1)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_CLOSING_1);
}

Defun(insAutotext_closing_2)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_CLOSING_2);
}

Defun(insAutotext_closing_3)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_CLOSING_3);
}

Defun(insAutotext_closing_4)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_CLOSING_4);
}

Defun(insAutotext_closing_5)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_CLOSING_5);
}

Defun(insAutotext_closing_6)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_CLOSING_6);
}

Defun(insAutotext_closing_7)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_CLOSING_7);
}

Defun(insAutotext_closing_8)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_CLOSING_8);
}

Defun(insAutotext_closing_9)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_CLOSING_9);
}

Defun(insAutotext_closing_10)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_CLOSING_10);
}

Defun(insAutotext_closing_11)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_CLOSING_11);
}

Defun(insAutotext_closing_12)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_CLOSING_12);
}

Defun(insAutotext_email_1)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_EMAIL_1);
}

Defun(insAutotext_email_2)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_EMAIL_2);
}

Defun(insAutotext_email_3)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_EMAIL_3);
}

Defun(insAutotext_email_4)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_EMAIL_4);
}

Defun(insAutotext_email_5)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_EMAIL_5);
}

Defun(insAutotext_email_6)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_EMAIL_6);
}

Defun(insAutotext_mail_1)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_MAIL_1);
}


Defun(insAutotext_mail_2)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_MAIL_2);
}

Defun(insAutotext_mail_3)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_MAIL_3);
}

Defun(insAutotext_mail_4)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_MAIL_4);
}

Defun(insAutotext_mail_5)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_MAIL_5);
}

Defun(insAutotext_mail_6)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_MAIL_6);
}

Defun(insAutotext_mail_7)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_MAIL_7);
}

Defun(insAutotext_mail_8)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_MAIL_8);
}

Defun(insAutotext_reference_1)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_REFERENCE_1);
}

Defun(insAutotext_reference_2)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_REFERENCE_2);
}

Defun(insAutotext_reference_3)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_REFERENCE_3);
}

Defun(insAutotext_salutation_1)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_SALUTATION_1);
}

Defun(insAutotext_salutation_2)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_SALUTATION_2);
}

Defun(insAutotext_salutation_3)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_SALUTATION_3);
}

Defun(insAutotext_salutation_4)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_SALUTATION_4);
}

Defun(insAutotext_subject_1)
{
  return _insAutotext(static_cast<FV_View *>(pAV_View), 
		      AP_STRING_ID_AUTOTEXT_SUBJECT_1);
}

Defun(dlgBackground)
{
#ifndef WIN32
	FV_View * pView = static_cast<FV_View *>(pAV_View);

	XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Background * pDialog
		= (AP_Dialog_Background *)(pDialogFactory->requestDialog(AP_DIALOG_ID_BACKGROUND));
	UT_ASSERT(pDialog);

	UT_RGBColor * pClr = pView->getCurrentPage()->getOwningSection()->getPaperColor();
	pDialog->setColor(*pClr);

	pDialog->runModal (pFrame);

	AP_Dialog_Background::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_Background::a_OK);

	if (bOK)
	  {
	    // let the view set the proper value in the
	    // document and refresh/redraw itself
	    UT_RGBColor clr = pDialog->getColor();
	    pView->setPaperColor (clr);
	  }

	pDialogFactory->releaseDialog(pDialog);
	return bOK;
#else
	return false;
#endif
}
