/* AbiWord
 * Copyright (C) 1998-2001 AbiSource, Inc.
 * Copyright (C) 2001 Tomas Frydrych
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
#include <locale.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "fl_AutoLists.h"
#include "fp_Page.h"
#include "fp_Line.h"
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
#include "ap_TopRuler.h"
#include "ap_Prefs.h"
#include "ut_string_class.h"

#include "ap_Dialog_Id.h"
#include "ap_Dialog_Replace.h"
#include "ap_Dialog_Goto.h"
#include "ap_Dialog_Break.h"
#include "ap_Dialog_InsertTable.h"
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
#include "ap_Dialog_New.h"
#include "ap_Dialog_HdrFtr.h"
#include "ap_Dialog_InsertBookmark.h"
#include "ap_Dialog_InsertHyperlink.h"
#include "ap_Dialog_MetaData.h"
#include "ap_Dialog_MarkRevisions.h"
#include "ap_Dialog_ListRevisions.h"
#include "ap_Dialog_MergeCells.h"
#include "ap_Dialog_FormatTable.h"
#include "ap_Dialog_FormatFootnotes.h"

#include "xap_App.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_About.h"
#include "xap_Dlg_ClipArt.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Dlg_FileOpenSaveAs.h"
#include "xap_Dlg_FontChooser.h"
#include "xap_Dlg_Print.h"
#include "xap_Dlg_PrintPreview.h"
#include "xap_Dlg_WindowMore.h"
#include "xap_Dlg_Zoom.h"
#include "xap_Dlg_Insert_Symbol.h"
#include "xap_Dlg_Language.h"
#include "xap_Dlg_PluginManager.h"
#include "xap_Dlg_Image.h"

#include "ie_imp.h"
#include "ie_impGraphic.h"
#include "ie_exp.h"
#include "ie_types.h"

#include "ut_timer.h"
#include "ut_Script.h"
#include "ut_path.h"

/*****************************************************************/
/*****************************************************************/

/* abbreviations:
**	 BOL	beginning of line
**	 EOL	end of line
**	 BOW	beginning of word
**	 EOW	end of word
**	 BOW	beginning of sentence
**	 EOS	end of sentence
**	 BOB	beginning of block
**	 EOB	end of block
**	 BOD	beginning of document
**	 EOD	end of document
**
**	 warpInsPt	  warp insertion point
**	 extSel 	  extend selection
**	 del		  delete
**	 bck		  backwards
**	 fwd		  forwards
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
	static EV_EditMethod_Fn scrollWheelMouseDown;
	static EV_EditMethod_Fn scrollWheelMouseUp;

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
	static EV_EditMethod_Fn warpInsPtPrevScreen;
	static EV_EditMethod_Fn warpInsPtNextScreen;
	static EV_EditMethod_Fn warpInsPtPrevLine;
	static EV_EditMethod_Fn warpInsPtNextLine;

	static EV_EditMethod_Fn cursorDefault;
	static EV_EditMethod_Fn cursorIBeam;
	static EV_EditMethod_Fn cursorRightArrow;
	static EV_EditMethod_Fn cursorVline;
	static EV_EditMethod_Fn cursorHline;
	static EV_EditMethod_Fn cursorLeftArrow;
	static EV_EditMethod_Fn cursorImage;
	static EV_EditMethod_Fn cursorImageSize;

	static EV_EditMethod_Fn contextImage;
	static EV_EditMethod_Fn contextHyperlink;
	static EV_EditMethod_Fn contextMenu;
	static EV_EditMethod_Fn contextRevision;
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

	static EV_EditMethod_Fn dragSelectionBegin;
	static EV_EditMethod_Fn dragSelectionEnd;

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
	static EV_EditMethod_Fn extSelScreenUp;
	static EV_EditMethod_Fn extSelScreenDown;
	static EV_EditMethod_Fn selectAll;
	static EV_EditMethod_Fn selectWord;
	static EV_EditMethod_Fn selectLine;
	static EV_EditMethod_Fn selectBlock;
	static EV_EditMethod_Fn selectObject;

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
	static EV_EditMethod_Fn deleteBookmark;
	static EV_EditMethod_Fn deleteColumns;
	static EV_EditMethod_Fn deleteCell;
	static EV_EditMethod_Fn deleteHyperlink;
	static EV_EditMethod_Fn deleteRows;
	static EV_EditMethod_Fn deleteTable;

	static EV_EditMethod_Fn insertBookmark;
	static EV_EditMethod_Fn insertHyperlink;
	static EV_EditMethod_Fn insertColsAfter;
	static EV_EditMethod_Fn insertColsBefore;
	static EV_EditMethod_Fn insertColumnBreak;
	static EV_EditMethod_Fn insertData;
	static EV_EditMethod_Fn insertLineBreak;
	static EV_EditMethod_Fn insertParagraphBreak;
	static EV_EditMethod_Fn insertPageBreak;
	static EV_EditMethod_Fn insertRowsAfter;
	static EV_EditMethod_Fn insertRowsBefore;
	static EV_EditMethod_Fn insertSectionBreak;
	static EV_EditMethod_Fn insertSoftBreak;
	static EV_EditMethod_Fn insertTab;

	static EV_EditMethod_Fn insertSpace;
	static EV_EditMethod_Fn insertNBSpace;
	static EV_EditMethod_Fn insertNBZWSpace;
	static EV_EditMethod_Fn insertZWJoiner;

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

	static EV_EditMethod_Fn mergeCells;
	static EV_EditMethod_Fn formatTable;

	static EV_EditMethod_Fn replaceChar;

	static EV_EditMethod_Fn resizeImage;
	static EV_EditMethod_Fn endResizeImage;
	static EV_EditMethod_Fn dragImage;
	static EV_EditMethod_Fn dropImage;

	static EV_EditMethod_Fn beginVDrag;
	static EV_EditMethod_Fn clearSetCols;
	static EV_EditMethod_Fn dragVline;
	static EV_EditMethod_Fn endDragVline;

	static EV_EditMethod_Fn beginHDrag;
	static EV_EditMethod_Fn clearSetRows;
	static EV_EditMethod_Fn dragHline;
	static EV_EditMethod_Fn endDragHline;


	// TODO add functions for all of the standard menu commands.
	// TODO here are a few that i started.

	static EV_EditMethod_Fn fileNew;
    static EV_EditMethod_Fn fileRevert;
	static EV_EditMethod_Fn toolbarNew;
	static EV_EditMethod_Fn fileOpen;
	static EV_EditMethod_Fn fileSave;
	static EV_EditMethod_Fn fileSaveAs;
	static EV_EditMethod_Fn fileSaveImage;
	static EV_EditMethod_Fn fileExport;
	static EV_EditMethod_Fn fileImport;
	static EV_EditMethod_Fn formatPainter;
	static EV_EditMethod_Fn pageSetup;
	static EV_EditMethod_Fn print;
	static EV_EditMethod_Fn printTB;
	static EV_EditMethod_Fn printPreview;
	static EV_EditMethod_Fn printDirectly;
	static EV_EditMethod_Fn fileInsertGraphic;
	static EV_EditMethod_Fn insertClipart;
	static EV_EditMethod_Fn fileSaveAsWeb;
    static EV_EditMethod_Fn fileSaveTemplate;
	static EV_EditMethod_Fn filePreviewWeb;
	static EV_EditMethod_Fn openTemplate;

	static EV_EditMethod_Fn undo;
	static EV_EditMethod_Fn redo;
	static EV_EditMethod_Fn cut;
	static EV_EditMethod_Fn copy;
	static EV_EditMethod_Fn paste;
	static EV_EditMethod_Fn pasteSelection;
	static EV_EditMethod_Fn pasteSpecial;
	static EV_EditMethod_Fn find;
	static EV_EditMethod_Fn findAgain;
	static EV_EditMethod_Fn go;
	static EV_EditMethod_Fn replace;
	static EV_EditMethod_Fn editHeader;
	static EV_EditMethod_Fn editFooter;
	static EV_EditMethod_Fn removeHeader;
	static EV_EditMethod_Fn removeFooter;

	static EV_EditMethod_Fn viewStd;
	static EV_EditMethod_Fn viewFormat;
	static EV_EditMethod_Fn viewExtra;
	static EV_EditMethod_Fn viewTable;
	static EV_EditMethod_Fn viewRuler;
	static EV_EditMethod_Fn viewStatus;
	static EV_EditMethod_Fn viewPara;
	static EV_EditMethod_Fn viewHeadFoot;
	static EV_EditMethod_Fn zoom;
	static EV_EditMethod_Fn dlgZoom;
	static EV_EditMethod_Fn viewFullScreen;

	static EV_EditMethod_Fn zoom100;
	static EV_EditMethod_Fn zoom200;
	static EV_EditMethod_Fn zoom75;
	static EV_EditMethod_Fn zoom50;
	static EV_EditMethod_Fn zoomWidth;
	static EV_EditMethod_Fn zoomWhole;
	static EV_EditMethod_Fn zoomIn;
	static EV_EditMethod_Fn zoomOut;

	static EV_EditMethod_Fn insBreak;
	static EV_EditMethod_Fn insPageNo;
	static EV_EditMethod_Fn insDateTime;
	static EV_EditMethod_Fn insField;
	static EV_EditMethod_Fn insSymbol;
	static EV_EditMethod_Fn insFile;
	static EV_EditMethod_Fn insFootnote;
	static EV_EditMethod_Fn insEndnote;

	static EV_EditMethod_Fn dlgSpell;
	static EV_EditMethod_Fn dlgSpellPrefs;
	static EV_EditMethod_Fn dlgWordCount;
	static EV_EditMethod_Fn dlgOptions;
    static EV_EditMethod_Fn dlgMetaData;

	static EV_EditMethod_Fn dlgFont;
	static EV_EditMethod_Fn dlgParagraph;
	static EV_EditMethod_Fn dlgBullets;
	static EV_EditMethod_Fn dlgBorders;
	static EV_EditMethod_Fn dlgColumns;
	static EV_EditMethod_Fn dlgFmtImage;
	static EV_EditMethod_Fn dlgHdrFtr;
	static EV_EditMethod_Fn style;
	static EV_EditMethod_Fn dlgBackground;
	static EV_EditMethod_Fn dlgStyle;
	static EV_EditMethod_Fn dlgTabs;
	static EV_EditMethod_Fn formatFootnotes;
	static EV_EditMethod_Fn dlgToggleCase;
	static EV_EditMethod_Fn rotateCase;
	static EV_EditMethod_Fn dlgLanguage;
	static EV_EditMethod_Fn dlgPlugins;
	static EV_EditMethod_Fn dlgColorPickerFore;
	static EV_EditMethod_Fn dlgColorPickerBack;
	static EV_EditMethod_Fn language;
	static EV_EditMethod_Fn fontFamily;
	static EV_EditMethod_Fn fontSize;
	static EV_EditMethod_Fn toggleBold;
	static EV_EditMethod_Fn toggleHidden;
	static EV_EditMethod_Fn toggleItalic;
	static EV_EditMethod_Fn toggleUline;
	static EV_EditMethod_Fn toggleOline;
	static EV_EditMethod_Fn toggleStrike;
	static EV_EditMethod_Fn toggleTopline;
	static EV_EditMethod_Fn toggleBottomline;
	static EV_EditMethod_Fn toggleSuper;
	static EV_EditMethod_Fn toggleSub;
	static EV_EditMethod_Fn togglePlain;
	static EV_EditMethod_Fn toggleDirOverrideLTR;
	static EV_EditMethod_Fn toggleDirOverrideRTL;
	static EV_EditMethod_Fn toggleDomDirection;

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

	static EV_EditMethod_Fn sectColumns1;
	static EV_EditMethod_Fn sectColumns2;
	static EV_EditMethod_Fn sectColumns3;

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
	static EV_EditMethod_Fn helpAboutGnu;
	static EV_EditMethod_Fn helpAboutGnomeOffice;
	static EV_EditMethod_Fn helpCredits;
	static EV_EditMethod_Fn helpReportBug;

	static EV_EditMethod_Fn newWindow;
	static EV_EditMethod_Fn cycleWindows;
	static EV_EditMethod_Fn cycleWindowsBck;
	static EV_EditMethod_Fn closeWindow;
	static EV_EditMethod_Fn closeWindowX;
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

	static EV_EditMethod_Fn scriptPlay;
	static EV_EditMethod_Fn executeScript;

        static EV_EditMethod_Fn mailMerge;

	static EV_EditMethod_Fn hyperlinkJump;
	static EV_EditMethod_Fn hyperlinkStatusBar;

	static EV_EditMethod_Fn lockGUI;
	static EV_EditMethod_Fn unlockGUI;

	static EV_EditMethod_Fn toggleMarkRevisions;
	static EV_EditMethod_Fn revisionAccept;
	static EV_EditMethod_Fn revisionReject;
	static EV_EditMethod_Fn revisionSetViewLevel;

	static EV_EditMethod_Fn insertTable;

	static EV_EditMethod_Fn noop;

	// Test routines

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
	static EV_EditMethod_Fn Test_Dump;
	static EV_EditMethod_Fn Test_Ftr;
#endif

};

/*****************************************************************/
/*****************************************************************/

#define _D_ 			EV_EMT_REQUIREDATA

#define F(fn)			ap_EditMethods::fn
#define N(fn)			#fn
#define NF(fn)			N(fn), F(fn)

// !!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// keep this array alphabetically (strcmp) ordered under
// penalty of being forced to port Abi to the PalmOS
//
// YOUR NEW METHOD WON'T BE FOUND AND YOU'LL SCREW UP ALL THE OTHER METHODS
// IF YOU DON'T DO THIS
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
static EV_EditMethod s_arrayEditMethods[] =
{
#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
	EV_EditMethod(NF(Test_Dump),			0,	""),
	EV_EditMethod(NF(Test_Ftr), 		0,	""),
#endif

	// a
	EV_EditMethod(NF(activateWindow_1), 	0,		""),
	EV_EditMethod(NF(activateWindow_2), 	0,		""),
	EV_EditMethod(NF(activateWindow_3), 	0,		""),
	EV_EditMethod(NF(activateWindow_4), 	0,		""),
	EV_EditMethod(NF(activateWindow_5), 	0,		""),
	EV_EditMethod(NF(activateWindow_6), 	0,		""),
	EV_EditMethod(NF(activateWindow_7), 	0,		""),
	EV_EditMethod(NF(activateWindow_8), 	0,		""),
	EV_EditMethod(NF(activateWindow_9), 	0,		""),
	EV_EditMethod(NF(alignCenter),			0,		""),
	EV_EditMethod(NF(alignJustify), 		0,		""),
	EV_EditMethod(NF(alignLeft),			0,		""),
	EV_EditMethod(NF(alignRight),			0,		""),

	// b
	EV_EditMethod(NF(beginHDrag), 0, ""),
	EV_EditMethod(NF(beginVDrag), 0, ""),

	// c
	EV_EditMethod(NF(clearSetCols), 0, ""),
	EV_EditMethod(NF(clearSetRows), 0, ""),
	EV_EditMethod(NF(closeWindow),			0,	""),
	EV_EditMethod(NF(closeWindowX), 0, ""),
	EV_EditMethod(NF(colorBackTB), _D_, ""),
	EV_EditMethod(NF(colorForeTB), _D_, ""),
	EV_EditMethod(NF(contextHyperlink), 		0,	""),
	EV_EditMethod(NF(contextImage), 0, ""),
	EV_EditMethod(NF(contextMenu),			0,	""),
	EV_EditMethod(NF(contextMisspellText),	0,	""),
	EV_EditMethod(NF(contextRevision),	    0,	""),
	EV_EditMethod(NF(contextText),			0,	""),
	EV_EditMethod(NF(copy), 				0,	""),
	EV_EditMethod(NF(cursorDefault),		0,	""),
	EV_EditMethod(NF(cursorHline),      	0,	""),
	EV_EditMethod(NF(cursorIBeam),			0,	""),
	EV_EditMethod(NF(cursorImage),			0,	""),
	EV_EditMethod(NF(cursorImageSize),		0,	""),
	EV_EditMethod(NF(cursorLeftArrow),		0,	""),
	EV_EditMethod(NF(cursorRightArrow), 	0,	""),
	EV_EditMethod(NF(cursorVline), 	        0,	""),
	EV_EditMethod(NF(cut),					0,	""),
	EV_EditMethod(NF(cycleInputMode),		0,	""),
	EV_EditMethod(NF(cycleWindows), 		0,	""),
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
	EV_EditMethod(NF(delRight), 			0,	""),
	EV_EditMethod(NF(deleteBookmark),		0,	""),
	EV_EditMethod(NF(deleteCell),   		0,	""),
	EV_EditMethod(NF(deleteColumns),   		0,	""),
	EV_EditMethod(NF(deleteHyperlink),		0,	""),
	EV_EditMethod(NF(deleteRows),   		0,	""),
	EV_EditMethod(NF(deleteTable),   		0,	""),
	EV_EditMethod(NF(dlgAbout), 			0,	""),
	EV_EditMethod(NF(dlgBackground),		0,	""),
	EV_EditMethod(NF(dlgBorders),			0,	""),
	EV_EditMethod(NF(dlgBullets),			0,	""),
	EV_EditMethod(NF(dlgColorPickerBack),	0,	""),
	EV_EditMethod(NF(dlgColorPickerFore),	0,	""),
	EV_EditMethod(NF(dlgColumns),			0,	""),
	EV_EditMethod(NF(dlgFmtImage), 			0, ""),
	EV_EditMethod(NF(dlgFont),				0,	""),
	EV_EditMethod(NF(dlgHdrFtr),			0,	""),
	EV_EditMethod(NF(dlgLanguage),			0,	""),
	EV_EditMethod(NF(dlgMetaData), 			0, ""),
	EV_EditMethod(NF(dlgMoreWindows),		0,	""),
	EV_EditMethod(NF(dlgOptions),			0,	""),
	EV_EditMethod(NF(dlgParagraph), 		0,	""),
	EV_EditMethod(NF(dlgPlugins), 			0,	""),
	EV_EditMethod(NF(dlgSpell), 			0,	""),
	EV_EditMethod(NF(dlgSpellPrefs), 		0,	""),
	EV_EditMethod(NF(dlgStyle), 			0,	""),
	EV_EditMethod(NF(dlgTabs),				0,	""),
	EV_EditMethod(NF(dlgToggleCase),		0,	""),
	EV_EditMethod(NF(dlgWordCount), 		0,	""),
	EV_EditMethod(NF(dlgZoom),				0,	""),
	EV_EditMethod(NF(doBullets),			0,	""),
	EV_EditMethod(NF(doNumbers),			0,	""),
	EV_EditMethod(NF(doubleSpace),			0,	""),
	EV_EditMethod(NF(dragHline), 			0,	""),
	EV_EditMethod(NF(dragImage),			0,	""),
	EV_EditMethod(NF(dragSelectionBegin), 0, ""),
	EV_EditMethod(NF(dragSelectionEnd), 0, ""),
	EV_EditMethod(NF(dragToXY), 			0,	""),
	EV_EditMethod(NF(dragToXYword), 		0,	""),
	EV_EditMethod(NF(dragVline), 			0,	""),
	EV_EditMethod(NF(dropImage),			0,	""),


	// e
	EV_EditMethod(NF(editFooter),			0,	""),
	EV_EditMethod(NF(editHeader),			0,	""),
	EV_EditMethod(NF(endDrag),				0,	""),
	EV_EditMethod(NF(endDragHline),			0,	""),
	EV_EditMethod(NF(endDragVline),			0,	""),
	EV_EditMethod(NF(endResizeImage),		0,	""),
	EV_EditMethod(NF(executeScript),		EV_EMT_REQUIRE_SCRIPT_NAME, ""),
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
	EV_EditMethod(NF(extSelPageUp), 		0,	""),
	EV_EditMethod(NF(extSelPrevLine),		0,	""),
	EV_EditMethod(NF(extSelRight),			0,	""),
	EV_EditMethod(NF(extSelScreenDown),		0,	""),
	EV_EditMethod(NF(extSelScreenUp),		0,	""),
	EV_EditMethod(NF(extSelToXY),			0,	""),

	// f
	EV_EditMethod(NF(fileExport), 0, ""),
	EV_EditMethod(NF(fileImport), 0, ""),
	EV_EditMethod(NF(fileInsertGraphic),	0,	""),
	EV_EditMethod(NF(fileNew),				0,	""),
	EV_EditMethod(NF(fileOpen), 			0,	""),
	EV_EditMethod(NF(filePreviewWeb), 0, ""),
	EV_EditMethod(NF(fileRevert), 0, ""),
	EV_EditMethod(NF(fileSave), 			0,	""),
	EV_EditMethod(NF(fileSaveAs),			0,	""),
	EV_EditMethod(NF(fileSaveAsWeb),				0, ""),
	EV_EditMethod(NF(fileSaveImage),		0,	""),
	EV_EditMethod(NF(fileSaveTemplate), 0, ""),
	EV_EditMethod(NF(find), 				0,	""),
	EV_EditMethod(NF(findAgain),			0,	""),
	EV_EditMethod(NF(fontFamily),			_D_,	""),
	EV_EditMethod(NF(fontSize), 			_D_,	""),
	EV_EditMethod(NF(formatFootnotes),        0,  ""),
	EV_EditMethod(NF(formatPainter),		0,	""),
	EV_EditMethod(NF(formatTable),			0,		""),

	// g
	EV_EditMethod(NF(go),					0,	""),

	// h
	EV_EditMethod(NF(helpAboutGnomeOffice), 0, ""),
	EV_EditMethod(NF(helpAboutGnu), 0, ""),
	EV_EditMethod(NF(helpAboutOS),			0,		""),
	EV_EditMethod(NF(helpCheckVer), 		0,		""),
	EV_EditMethod(NF(helpContents), 		0,		""),
	EV_EditMethod(NF(helpCredits), 0, ""),
	EV_EditMethod(NF(helpIndex),			0,		""),
	EV_EditMethod(NF(helpReportBug), 0, ""),
	EV_EditMethod(NF(helpSearch),			0,		""),
	EV_EditMethod(NF(hyperlinkJump),		0,		""),
	EV_EditMethod(NF(hyperlinkStatusBar),	0,		""),
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
	EV_EditMethod(NF(insBreak), 		0,		""),
	EV_EditMethod(NF(insDateTime),			0,		""),
	EV_EditMethod(NF(insEndnote),			0,		""),
	EV_EditMethod(NF(insField), 		0,		""),
	EV_EditMethod(NF(insFile), 0, ""),
	EV_EditMethod(NF(insFootnote),			0,		""),
	EV_EditMethod(NF(insPageNo),			0,		""),
	EV_EditMethod(NF(insSymbol),			0,		""),
	EV_EditMethod(NF(insertAbovedotData),	_D_,	""),
	EV_EditMethod(NF(insertAcuteData),		_D_,	""),
	EV_EditMethod(NF(insertBookmark),		0,	""),
	EV_EditMethod(NF(insertBreveData),		_D_,	""),
	EV_EditMethod(NF(insertCaronData),		_D_,	""),
	EV_EditMethod(NF(insertCedillaData),	_D_,	""),
	EV_EditMethod(NF(insertCircumflexData), _D_,	""),
	EV_EditMethod(NF(insertClipart), 0, ""),
	EV_EditMethod(NF(insertColsAfter),	0,	""),
	EV_EditMethod(NF(insertColsBefore),	0,	""),
	EV_EditMethod(NF(insertColumnBreak),	0,	""),
	EV_EditMethod(NF(insertData),			_D_,	""),
	EV_EditMethod(NF(insertDiaeresisData),	_D_,	""),
	EV_EditMethod(NF(insertDoubleacuteData),_D_,	""),
	EV_EditMethod(NF(insertGraveData),		_D_,	""),
	EV_EditMethod(NF(insertHyperlink),		0,	""),
	EV_EditMethod(NF(insertLineBreak),		0,	""),
	EV_EditMethod(NF(insertMacronData), 	_D_,	""),
	EV_EditMethod(NF(insertNBSpace),		0,	""),
	EV_EditMethod(NF(insertNBZWSpace),		0,	""),
	EV_EditMethod(NF(insertOgonekData), 	_D_,	""),
	EV_EditMethod(NF(insertPageBreak),		0,	""),
	EV_EditMethod(NF(insertParagraphBreak), 0,	""),
	EV_EditMethod(NF(insertRowsAfter),	0,	""),
	EV_EditMethod(NF(insertRowsBefore),	0,	""),
	EV_EditMethod(NF(insertSectionBreak),	0,	""),
	EV_EditMethod(NF(insertSoftBreak),		0,	""),
	EV_EditMethod(NF(insertSpace),			0,	""),
	EV_EditMethod(NF(insertTab),			0,	""),
	EV_EditMethod(NF(insertTable),          0,  ""),
	EV_EditMethod(NF(insertTildeData),		_D_,	""),
	EV_EditMethod(NF(insertZWJoiner),		0,	""),

	// j

	// k

	// l
	EV_EditMethod(NF(language), 		0,	""),
	EV_EditMethod(NF(lockGUI), 		0,	""),

	// m
	EV_EditMethod(NF(mailMerge), 0, ""),
	EV_EditMethod(NF(mergeCells),			0,		""),
	EV_EditMethod(NF(middleSpace),			0,		""),

	// n
	EV_EditMethod(NF(newWindow),			0,	""),
	EV_EditMethod(NF(noop), 				0,	""),

	// o
	EV_EditMethod(NF(openRecent_1), 		0,		""),
	EV_EditMethod(NF(openRecent_2), 		0,		""),
	EV_EditMethod(NF(openRecent_3), 		0,		""),
	EV_EditMethod(NF(openRecent_4), 		0,		""),
	EV_EditMethod(NF(openRecent_5), 		0,		""),
	EV_EditMethod(NF(openRecent_6), 		0,		""),
	EV_EditMethod(NF(openRecent_7), 		0,		""),
	EV_EditMethod(NF(openRecent_8), 		0,		""),
	EV_EditMethod(NF(openRecent_9), 		0,		""),
	EV_EditMethod(NF(openTemplate), 0, ""),

	// p
	EV_EditMethod(NF(pageSetup),			0,	""),
	EV_EditMethod(NF(paraBefore0),			0,		""),
	EV_EditMethod(NF(paraBefore12), 		0,		""),
		// intended for ^V and Menu[Edit/Paste]
	EV_EditMethod(NF(paste),				0,	""),
			// intended for X11 middle mouse
	EV_EditMethod(NF(pasteSelection),		0,	""),
	EV_EditMethod(NF(pasteSpecial), 		0,	""),
	EV_EditMethod(NF(print),				0,	""),
	EV_EditMethod(NF(printDirectly), 0, ""),
	EV_EditMethod(NF(printPreview), 0, ""),
	EV_EditMethod(NF(printTB),				0,	""),

	// q
	EV_EditMethod(NF(querySaveAndExit), 	0,	""),

	// r
	EV_EditMethod(NF(redo), 				0,	""),
	EV_EditMethod(NF(removeFooter), 		0,	""),
	EV_EditMethod(NF(removeHeader), 		0,	""),
	EV_EditMethod(NF(replace),				0,	""),
	EV_EditMethod(NF(replaceChar),			_D_,""),
	EV_EditMethod(NF(resizeImage),			0,  ""),
	EV_EditMethod(NF(revisionAccept),		0,  ""),
	EV_EditMethod(NF(revisionReject),		0,  ""),
	EV_EditMethod(NF(revisionSetViewLevel),	0,  ""),
	EV_EditMethod(NF(rotateCase),			0,	""),

	// s
	EV_EditMethod(NF(scriptPlay),			0,	""),
	EV_EditMethod(NF(scrollLineDown),		0,	""),
	EV_EditMethod(NF(scrollLineLeft),		0,	""),
	EV_EditMethod(NF(scrollLineRight),		0,	""),
	EV_EditMethod(NF(scrollLineUp), 		0,	""),
	EV_EditMethod(NF(scrollPageDown),		0,	""),
	EV_EditMethod(NF(scrollPageLeft),		0,	""),
	EV_EditMethod(NF(scrollPageRight),		0,	""),
	EV_EditMethod(NF(scrollPageUp), 		0,	""),
	EV_EditMethod(NF(scrollToBottom),		0,	""),
	EV_EditMethod(NF(scrollToTop),			0,	""),
	EV_EditMethod(NF(scrollWheelMouseDown), 		0,	""),
	EV_EditMethod(NF(scrollWheelMouseUp),			0,	""),
	EV_EditMethod(NF(sectColumns1), 		0,		""),
	EV_EditMethod(NF(sectColumns2), 		0,		""),
	EV_EditMethod(NF(sectColumns3), 		0,		""),
	EV_EditMethod(NF(selectAll),			0,	""),
	EV_EditMethod(NF(selectBlock),			0,	""),
	EV_EditMethod(NF(selectLine),			0,	""),
	EV_EditMethod(NF(selectObject), 		0,	""),
	EV_EditMethod(NF(selectWord),			0,	""),
	EV_EditMethod(NF(setEditVI),			0,	""),
	EV_EditMethod(NF(setInputVI),			0,	""),
	EV_EditMethod(NF(setStyleHeading1), 	0,		""),
	EV_EditMethod(NF(setStyleHeading2), 	0,		""),
	EV_EditMethod(NF(setStyleHeading3), 	0,		""),
	EV_EditMethod(NF(singleSpace),			0,		""),
	EV_EditMethod(NF(spellAdd), 			0,	""),
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
	EV_EditMethod(NF(style),				_D_,""),

	// t
	EV_EditMethod(NF(toggleAutoSpell),      0,  ""),
	EV_EditMethod(NF(toggleBold),			0,	""),
	EV_EditMethod(NF(toggleBottomline), 	0,	""),
	EV_EditMethod(NF(toggleDirOverrideLTR), 0,	""),
	EV_EditMethod(NF(toggleDirOverrideRTL), 0,	""),
	EV_EditMethod(NF(toggleDomDirection),	0,	""),
	EV_EditMethod(NF(toggleHidden),			0,	""),
	EV_EditMethod(NF(toggleIndent),         0,  ""),
	EV_EditMethod(NF(toggleInsertMode), 	0,  ""),
	EV_EditMethod(NF(toggleItalic), 		0,	""),
	EV_EditMethod(NF(toggleMarkRevisions),  0,  ""),
	EV_EditMethod(NF(toggleOline),			0,  ""),
	EV_EditMethod(NF(togglePlain),			0,	""),
	EV_EditMethod(NF(toggleStrike), 		0,	""),
	EV_EditMethod(NF(toggleSub),			0,	""),
	EV_EditMethod(NF(toggleSuper),			0,	""),
	EV_EditMethod(NF(toggleTopline),		0,	""),
	EV_EditMethod(NF(toggleUline),			0,	""),
	EV_EditMethod(NF(toggleUnIndent),       0,  ""),
	EV_EditMethod(NF(toolbarNew),           0,  ""),

	// u
	EV_EditMethod(NF(undo), 				0,	""),
	EV_EditMethod(NF(unlockGUI), 		0,	""),

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
	EV_EditMethod(NF(viCmd_cb), 	0,	""),
	EV_EditMethod(NF(viCmd_cw), 	0,	""),
	EV_EditMethod(NF(viCmd_d24),		0,	""),
	EV_EditMethod(NF(viCmd_d28),		0,	""),
	EV_EditMethod(NF(viCmd_d29),		0,	""),
	EV_EditMethod(NF(viCmd_d5b),		0,	""),
	EV_EditMethod(NF(viCmd_d5d),		0,	""),
	EV_EditMethod(NF(viCmd_d5e),		0,	""),
	EV_EditMethod(NF(viCmd_db), 	0,	""),
	EV_EditMethod(NF(viCmd_dd), 	0,	""),
	EV_EditMethod(NF(viCmd_dw), 	0,	""),
	EV_EditMethod(NF(viCmd_y24),	0,	""),
	EV_EditMethod(NF(viCmd_y28),	0,	""),
	EV_EditMethod(NF(viCmd_y29),	0,	""),
	EV_EditMethod(NF(viCmd_y5b),	0,	""),
	EV_EditMethod(NF(viCmd_y5d),	0,	""),
	EV_EditMethod(NF(viCmd_y5e),	0,	""),
	EV_EditMethod(NF(viCmd_yb), 	0,	""),
	EV_EditMethod(NF(viCmd_yw), 	0,	""),
	EV_EditMethod(NF(viCmd_yy), 	0,	""),
	EV_EditMethod(NF(viewExtra),			0,		""),
	EV_EditMethod(NF(viewFormat),			0,		""),
	EV_EditMethod(NF(viewFullScreen), 0, ""),
	EV_EditMethod(NF(viewHeadFoot), 		0,		""),
	EV_EditMethod(NF(viewNormalLayout), 0, ""),
	EV_EditMethod(NF(viewPara), 		0,		""),
	EV_EditMethod(NF(viewPrintLayout), 0, ""),
	EV_EditMethod(NF(viewRuler),			0,		""),
	EV_EditMethod(NF(viewStatus),			0,		""),
	EV_EditMethod(NF(viewStd),			0,		""),
	EV_EditMethod(NF(viewTable),			0,		""),
	EV_EditMethod(NF(viewWebLayout), 0, ""),

	// w
	EV_EditMethod(NF(warpInsPtBOB), 		0,	""),
	EV_EditMethod(NF(warpInsPtBOD), 		0,	""),
	EV_EditMethod(NF(warpInsPtBOL), 		0,	""),
	EV_EditMethod(NF(warpInsPtBOP), 		0,	""),
	EV_EditMethod(NF(warpInsPtBOS), 		0,	""),
	EV_EditMethod(NF(warpInsPtBOW), 		0,	""),
	EV_EditMethod(NF(warpInsPtEOB), 		0,	""),
	EV_EditMethod(NF(warpInsPtEOD), 		0,	""),
	EV_EditMethod(NF(warpInsPtEOL), 		0,	""),
	EV_EditMethod(NF(warpInsPtEOP), 		0,	""),
	EV_EditMethod(NF(warpInsPtEOS), 		0,	""),
	EV_EditMethod(NF(warpInsPtEOW), 		0,	""),
	EV_EditMethod(NF(warpInsPtLeft),		0,	""),
	EV_EditMethod(NF(warpInsPtNextLine),	0,	""),
	EV_EditMethod(NF(warpInsPtNextPage),	0,	""),
	EV_EditMethod(NF(warpInsPtNextScreen),	0,	""),
	EV_EditMethod(NF(warpInsPtPrevLine),	0,	""),
	EV_EditMethod(NF(warpInsPtPrevPage),	0,	""),
	EV_EditMethod(NF(warpInsPtPrevScreen),	0,	""),
	EV_EditMethod(NF(warpInsPtRight),		0,	""),
	EV_EditMethod(NF(warpInsPtToXY),		0,	""),

	// x

	// y

	// z
	EV_EditMethod(NF(zoom), 				0,		""),
	EV_EditMethod(NF(zoom100), 0, ""),
	EV_EditMethod(NF(zoom200), 0, ""),
	EV_EditMethod(NF(zoom50), 0, ""),
	EV_EditMethod(NF(zoom75), 0, ""),
	EV_EditMethod(NF(zoomIn), 0, ""),
	EV_EditMethod(NF(zoomOut), 0, ""),
	EV_EditMethod(NF(zoomWhole), 0, ""),
	EV_EditMethod(NF(zoomWidth), 0, "")
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
#define Defun(fn)	bool F(fn)(AV_View*   pAV_View,   EV_EditMethodCallData *	pCallData  )
#define Defun0(fn)	bool F(fn)(AV_View* /*pAV_View*/, EV_EditMethodCallData * /*pCallData*/)
#define Defun1(fn)	bool F(fn)(AV_View*   pAV_View,   EV_EditMethodCallData * /*pCallData*/)
#define EX(fn)		F(fn)(pAV_View, pCallData)

// forward declaration
static bool _helpOpenURL(AV_View* pAV_View, const char* helpURL);


static UT_Timer * s_pToUpdateCursor = NULL;
static XAP_Frame * s_pLoadingFrame = NULL;
static AD_Document * s_pLoadingDoc = NULL;
static bool s_LockOutGUI = false;

/*!
This little macro locks out loading frames from any activity thus preventing
segfaults.
*/

static bool s_EditMethods_check_frame(void)
{
	bool result = false;
	if(s_LockOutGUI)
	{
		return true;
	}
	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
	AV_View * pView = NULL;
	if(pFrame)
	{
		pView = pFrame->getCurrentView();
	}
	if(s_pLoadingFrame && (pFrame == s_pLoadingFrame))
	{
		result = true;
	}
	else if(pFrame && (s_pLoadingDoc != NULL) && (pFrame->getCurrentDoc() == s_pLoadingDoc))
	{
	        result = true;
	}
	else if(pView && ((pView->getPoint() == 0) || pView->isLayoutFilling()))
	{
		result = true;
	}
	return result;
}

/*!
 * Call this if you want to prevent GUI operations on AbiWord.
 */
Defun(lockGUI)
{
	s_LockOutGUI = true;
	return true;
}


/*!
 * Call this to allow GUI operations on AbiWord.
 */
Defun(unlockGUI)
{
	s_LockOutGUI = false;
	return true;
}

#define CHECK_FRAME if(s_EditMethods_check_frame()) return true;

Defun1(toggleAutoSpell)
{
	CHECK_FRAME;
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
	CHECK_FRAME;
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGEDOWN);

	return true;
}

Defun1(scrollPageUp)
{
	CHECK_FRAME;
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGEUP);

	return true;
}

Defun1(scrollPageLeft)
{
	CHECK_FRAME;
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGELEFT);

	return true;
}

Defun1(scrollPageRight)
{
	CHECK_FRAME;
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGERIGHT);

	return true;
}

Defun1(scrollLineDown)
{
	CHECK_FRAME;
	pAV_View->cmdScroll(AV_SCROLLCMD_LINEDOWN);

	return true;
}

Defun1(scrollLineUp)
{
	CHECK_FRAME;
	pAV_View->cmdScroll(AV_SCROLLCMD_LINEUP);

	return true;
}

Defun1(scrollWheelMouseDown)
{
	CHECK_FRAME;
	pAV_View->cmdScroll(AV_SCROLLCMD_LINEDOWN, 60);

	return true;
}

Defun1(scrollWheelMouseUp)
{
	CHECK_FRAME;
	pAV_View->cmdScroll(AV_SCROLLCMD_LINEUP, 60);

	return true;
}

Defun1(scrollLineLeft)
{
	CHECK_FRAME;
	pAV_View->cmdScroll(AV_SCROLLCMD_LINELEFT);

	return true;
}

Defun1(scrollLineRight)
{
	CHECK_FRAME;
	pAV_View->cmdScroll(AV_SCROLLCMD_LINERIGHT);

	return true;
}

Defun1(scrollToTop)
{
	CHECK_FRAME;
	pAV_View->cmdScroll(AV_SCROLLCMD_TOTOP);

	return true;
}

Defun1(scrollToBottom)
{
	CHECK_FRAME;
	pAV_View->cmdScroll(AV_SCROLLCMD_TOBOTTOM);

	return true;
}

Defun1(toolbarNew)
{
	CHECK_FRAME;
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

/*!
 * Callback function to implement the updating loader. This enables the user
 * to see the document as soon as possible and updates the size of the scroll
 * bars as the document is loaded.
 */
static bool s_bFirstDrawDone = false;
static void s_LoadingCursorCallback(UT_Worker * pTimer )
{
	UT_ASSERT(pTimer);
	XAP_Frame * pFrame = s_pLoadingFrame;
	if(pFrame == NULL)
	{
		return;
	}
	const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
	pFrame->setCursor(GR_Graphics::GR_CURSOR_WAIT);
	FV_View * pView = (FV_View *) pFrame->getCurrentView();
	if(pView)
	{
		GR_Graphics * pG = pView->getGraphics();
		if(pG)
		{
			pG->setCursor(GR_Graphics::GR_CURSOR_WAIT);
		}
		FL_DocLayout * pLayout = pView->getLayout();
		if(pView->getPoint() > 0)
		{
			pLayout->updateLayout();
			if(!s_bFirstDrawDone)
			{
				pView->draw();
				s_bFirstDrawDone = true;
			}
			else
			{
				pView->updateScreen(true);
			}
			if(pView->getLayout()->countPages() >1)
			{
				UT_String msg = pSS->getValue(XAP_STRING_ID_MSG_BuildingDoc);
				pFrame->setStatusMessage ( (XML_Char *) msg.c_str() );
			}
			else
			{
				UT_String msg =  pSS->getValue(XAP_STRING_ID_MSG_ImportingDoc);
				pFrame->setStatusMessage ( (XML_Char *) msg.c_str() );
			}
		}
		else
		{
			UT_String msg =  pSS->getValue(XAP_STRING_ID_MSG_ImportingDoc);
			pFrame->setStatusMessage ( (XML_Char *) msg.c_str() );
		}
	}
	else
	{
		UT_String msg =  pSS->getValue(XAP_STRING_ID_MSG_ImportingDoc);
		pFrame->setStatusMessage ( (XML_Char *) msg.c_str() );
	}
}

/*!
 * Control Method for the updating loader.
\params bool bStartStop true to start the updating loader, flase to stop it
             after the document has loaded.
\params XAP_Frame * pFrame Pointer to the new frame being loaded.
*/
static void s_StartStopLoadingCursor( bool bStartStop, XAP_Frame * pFrame)
{
	// Now construct the timer for auto-updating
	if(bStartStop)
	{
//
// Can't have multiple loading document yet. Need Vectors of loading frames
// and auto-updaters. Do this later.
//
		if(s_pLoadingFrame != NULL)
		{
			return;
		}
		s_pLoadingFrame = pFrame;
		s_pLoadingDoc = pFrame->getCurrentDoc();
		if(s_pToUpdateCursor == NULL)
		{
			GR_Graphics * pG = NULL;
			s_pToUpdateCursor = UT_Timer::static_constructor(s_LoadingCursorCallback,NULL,pG);
		}
		s_bFirstDrawDone = false;
		s_pToUpdateCursor->set(500);
		s_pToUpdateCursor->start();
//		s_pLoadingFrame = XAP_App::getApp()->getLastFocussedFrame();
	}
	else
	{
		if(s_pToUpdateCursor != NULL)
		{
			s_pToUpdateCursor->stop();
			DELETEP(s_pToUpdateCursor);
			s_pToUpdateCursor = NULL;
			if(s_pLoadingFrame != NULL)
			{
				s_pLoadingFrame->setCursor(GR_Graphics::GR_CURSOR_DEFAULT);
				FV_View * pView = (FV_View *) s_pLoadingFrame->getCurrentView();
				if(pView)
				{
					pView->setCursorToContext();
					pView->focusChange(AV_FOCUS_HERE);
				}
			}
			s_pLoadingFrame = NULL;
		}
		s_pLoadingDoc = NULL;
	}
}


static void s_TellOpenFailed(XAP_Frame * pFrame, const char * fileName, UT_Error errorCode)
{
  XAP_String_Id String_id = AP_STRING_ID_MSG_OpenFailed;

// TODO use switch statement for more error codes
#if 0  
  switch(errorCode)
	{
	default:
	  String_id = AP_STRING_ID_MSG_OpenFailed;
	  break;
	}
#endif
  
  pFrame->showMessageBox(String_id,
			 XAP_Dialog_MessageBox::b_O,
			 XAP_Dialog_MessageBox::a_OK,
			 fileName);
}

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

static void s_TellSpellDone(XAP_Frame * pFrame, bool bIsSelection)
{
	pFrame->showMessageBox(bIsSelection ? AP_STRING_ID_MSG_SpellSelectionDone : AP_STRING_ID_MSG_SpellDone,
			       XAP_Dialog_MessageBox::b_O,
			       XAP_Dialog_MessageBox::a_OK);
}

static void s_TellNotImplemented(XAP_Frame * pFrame, const char * szWhat, int iLine)
{
	XAP_Dialog_MessageBox * message = 
		pFrame->createMessageBox(AP_STRING_ID_MSG_DlgNotImp,
					 XAP_Dialog_MessageBox::b_O,
					 XAP_Dialog_MessageBox::a_OK,
					 szWhat, __FILE__, iLine);
	pFrame->showMessageBox(message);

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
	XAP_Dialog_MessageBox * message = 
		pFrame->createMessageBox(AP_STRING_ID_MSG_ConfirmSave,
					 XAP_Dialog_MessageBox::b_YNC,
					 XAP_Dialog_MessageBox::a_YES,
					 pFrame->getNonDecoratedTitle());
	message->setSecondaryMessage(AP_STRING_ID_MSG_ConfirmSaveSecondary, pFrame->getTimeSinceSave()/60);
	return pFrame->showMessageBox(message);
}

static bool s_AskForPathname(XAP_Frame * pFrame,
				 bool bSaveAs,
				 XAP_Dialog_Id id,
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

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

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

	const char ** szDescList = (const char **) UT_calloc(filterCount + 1,
													  sizeof(char *));
	const char ** szSuffixList = (const char **) UT_calloc(filterCount + 1,
														sizeof(char *));
	IEFileType * nTypeList = (IEFileType *) UT_calloc(filterCount + 1,
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
	static IEFileType dflFileType = IEFT_Bogus;

	if (ieft != NULL && *ieft != IEFT_Bogus)
	  {
		// have a pre-existing file format, try to default to that
		UT_DEBUGMSG(("DOM: using filetype %d\n", *ieft));
		dflFileType = *ieft;
	  }
	else if (bSaveAs)
	  {
		XAP_App * pApp = pFrame->getApp();
		UT_ASSERT(pApp);
		XAP_Prefs * pPrefs = pApp->getPrefs();
		UT_ASSERT(pPrefs);

		const XML_Char * ftype = 0;

		pPrefs->getPrefsValue ((XML_Char*)AP_PREF_KEY_DefaultSaveFormat, &ftype);
		if (!ftype)
		  ftype = ".abw";

		// load the default file format
		dflFileType = IE_Exp::fileTypeForSuffix (ftype);
		UT_DEBUGMSG(("DOM: reverting to default file type: %s (%d)\n", ftype, dflFileType));
	  }
	else
	  {
		// try to load ABW by default
		dflFileType = IE_Imp::fileTypeForSuffix (".abw");
	  }

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

	const char ** szDescList = (const char **) UT_calloc(filterCount + 1,
							  sizeof(char *));
	const char ** szSuffixList = (const char **) UT_calloc(filterCount + 1,
								sizeof(char *));
	IEGraphicFileType * nTypeList = (IEGraphicFileType *)
		 UT_calloc(filterCount + 1,	sizeof(IEGraphicFileType));
	UT_uint32 k = 0;

	while (IE_ImpGraphic::enumerateDlgLabels(k, &szDescList[k], &szSuffixList[k], &nTypeList[k]))
		k++;

	pDialog->setFileTypeList(szDescList, szSuffixList, (const UT_sint32 *) nTypeList);
	if (iegft != NULL)
	  pDialog->setDefaultFileType(*iegft);
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

XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode)
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
			s_StartStopLoadingCursor( true,pNewFrame);
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
		s_StartStopLoadingCursor( false,NULL);
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
		// return.	we do not create a new untitled document
		// in this case.
		pNewFrame = pApp->newFrame();
		if (!pNewFrame)
		{
			s_StartStopLoadingCursor( false,NULL);
			return false;
		}

// Open a complete but blank frame, then load the document into it

		errorCode = pNewFrame->loadDocument(NULL, IEFT_Unknown);
		if (!errorCode)
		{
			pNewFrame->show();
		}
	    else
		{
			return false;
		}


		s_StartStopLoadingCursor( true,pNewFrame);
		errorCode = pNewFrame->loadDocument(pNewFile, ieft);
		if (!errorCode)
		{
			pNewFrame->show();
			pPrefs->addRecent(pNewFile);
		}
#if 0
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
#endif
		s_StartStopLoadingCursor( false,NULL);
		return errorCode;
	}

	// we are replacing the single-view, unmodified, untitled document.
	// if we fail, put up an error message on the current frame
	// and return -- we do not replace this untitled document with a
	// new untitled document.
	s_StartStopLoadingCursor( true,pFrame);
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
	s_StartStopLoadingCursor( false,NULL);
	return errorCode;
}

#define ABIWORD_VIEW	FV_View * pView = static_cast<FV_View *>(pAV_View);

Defun1(fileOpen)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_ASSERT(pFrame);

	char * pNewFile = NULL;
	IEFileType ieft = static_cast<PD_Document *>(pFrame->getCurrentDoc())->getLastOpenedType();
	bool bOK = s_AskForPathname(pFrame,false, XAP_DIALOG_ID_FILE_OPEN, NULL,&pNewFile,&ieft);

	if (!bOK || !pNewFile)
	  return false;

	// we own storage for pNewFile and must free it.

	UT_Error error = ::fileOpen(pFrame, pNewFile, ieft);

	free(pNewFile);
	return E2B(error);
}

static UT_Error
s_importFile (XAP_Frame * pFrame, const char * pNewFile, IEFileType ieft)
{
	UT_DEBUGMSG(("fileOpen: loading [%s]\n",pNewFile));
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Frame * pNewFrame = NULL;
	// not needed bool bRes = false;
	UT_Error errorCode = UT_IE_IMPORTERROR;

	// We open documents in a new frame, which keeps the
	// contents of the current frame available.
	// However, as a convenience we do replace the contents of the
	// current frame if it's the only top-level view on an empty,
	// untitled document.

	if (pFrame->isDirty() || pFrame->getFilename() || (pFrame->getViewNumber() > 0))
	{
		// open new document in a new frame.  if we fail,
		// put up an error dialog on current frame (our
		// new one is not completely instantiated) and
		// return.	we do not create a new untitled document
		// in this case.

		pNewFrame = pApp->newFrame();
		if (!pNewFrame)
		{
			s_StartStopLoadingCursor( false,NULL);
			return false;
		}

		// treat import as creating a new, dirty document that
		// must be saved to be made 'clean'
		s_StartStopLoadingCursor( true,pNewFrame);
		errorCode = pNewFrame->importDocument(pNewFile, ieft, false);
		if (!errorCode)
		{
			pNewFrame->show(); // don't add to the MRU
		}
		else
		{
			// see problem documented in ::fileOpen()
			errorCode = pNewFrame->loadDocument(NULL, IEFT_Unknown);
			if (!errorCode)
				pNewFrame->show();
			s_CouldNotLoadFileMessage(pNewFrame,pNewFile, errorCode);
		}
		s_StartStopLoadingCursor( false,NULL);
		return errorCode;
	}

	// we are replacing the single-view, unmodified, untitled document.
	// if we fail, put up an error message on the current frame
	// and return -- we do not replace this untitled document with a
	// new untitled document.
	s_StartStopLoadingCursor( true,pFrame);
	errorCode = pFrame->importDocument(pNewFile, ieft);
	if (!errorCode)
	{
		pFrame->show(); // don't add to the MRU
	}
	else
	{
		s_CouldNotLoadFileMessage(pFrame,pNewFile, errorCode);
	}
	s_StartStopLoadingCursor( false,NULL);
	return errorCode;
}

Defun1(openTemplate)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	char * pNewFile = NULL;
	IEFileType ieft = static_cast<PD_Document *>(pFrame->getCurrentDoc())->getLastOpenedType();
	bool bOK = s_AskForPathname(pFrame,false, XAP_DIALOG_ID_FILE_IMPORT, NULL,&pNewFile,&ieft);

	if (!bOK || !pNewFile)
	  return false;

	// we own storage for pNewFile and must free it.

	UT_Error error = s_importFile(pFrame, pNewFile, ieft);

	free(pNewFile);
	return E2B(error);
}

Defun(fileSave)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
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

static bool
s_actuallySaveAs(AV_View * pAV_View, bool overwriteName)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	IEFileType ieft = static_cast<PD_Document *>(pFrame->getCurrentDoc())->getLastSavedAsType();
	char * pNewFile = NULL;
	XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_SAVEAS;

	if ( !overwriteName )
	  id = XAP_DIALOG_ID_FILE_EXPORT;

	bool bOK = s_AskForPathname(pFrame,true, id, pFrame->getFilename(),&pNewFile,&ieft);

	if (!bOK || !pNewFile)
		return false;

	UT_DEBUGMSG(("fileSaveAs: saving as [%s]\n",pNewFile));

	UT_Error errSaved;
	errSaved = pAV_View->cmdSaveAs(pNewFile, (int) ieft, overwriteName);
	if (errSaved)
	{
		// throw up a dialog
		s_TellSaveFailed(pFrame, pNewFile, errSaved);
		free(pNewFile);
		return false;
	}

	// ignore all of this stuff
	if (!overwriteName)
		return bOK;

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

Defun1(fileExport)
{
	CHECK_FRAME;
	return s_actuallySaveAs(pAV_View, false);
}

Defun1(fileImport)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	char * pNewFile = NULL;
	IEFileType ieft = static_cast<PD_Document *>(pFrame->getCurrentDoc())->getLastOpenedType();
	bool bOK = s_AskForPathname(pFrame,false, XAP_DIALOG_ID_FILE_IMPORT, NULL,&pNewFile,&ieft);

	if (!bOK || !pNewFile)
	  return false;

	// we own storage for pNewFile and must free it.

	UT_Error error = s_importFile(pFrame, pNewFile, ieft);

	free(pNewFile);
	return E2B(error);
}

Defun1(fileSaveAs)
{
	CHECK_FRAME;
	return s_actuallySaveAs(pAV_View, true);
}

Defun1(fileSaveTemplate)
{
  CHECK_FRAME;

  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
  UT_ASSERT(pFrame);

  IEFileType ieft = IE_Exp::fileTypeForSuffix ( ".awt" ) ;
  char * pNewFile = NULL;
  XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_SAVEAS;

  UT_String suggestedName (XAP_App::getApp()->getUserPrivateDirectory());
  suggestedName += "/templates/" ;

  bool bOK = s_AskForPathname(pFrame,true, id, suggestedName.c_str(),&pNewFile,&ieft);

  if (!bOK || !pNewFile)
    return false;

  UT_DEBUGMSG(("fileSaveTemplate: saving as [%s]\n",pNewFile));

  UT_Error errSaved;
  errSaved = pAV_View->cmdSaveAs(pNewFile, (int) ieft, false);
  if (errSaved)
    {
      // throw up a dialog
      s_TellSaveFailed(pFrame, pNewFile, errSaved);
      free(pNewFile);
      return false;
    }

  return bOK;
}

Defun1(fileSaveAsWeb)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pAV_View->getParentData());
  IEFileType ieft = IE_Exp::fileTypeForSuffix (".xhtml");
  char * pNewFile = NULL;
  bool bOK = s_AskForPathname(pFrame,true, XAP_DIALOG_ID_FILE_SAVEAS, pFrame->getFilename(),&pNewFile,&ieft);

  if (!bOK || !pNewFile)
	return false;

  UT_Error errSaved;
  errSaved = pAV_View->cmdSaveAs(pNewFile, ieft);
  if (errSaved)
	{
	  // throw up a dialog
	  s_TellSaveFailed(pFrame, pNewFile, errSaved);
	  free(pNewFile);
	  return false;
	}

  return true;
}

Defun1(fileSaveImage)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *) pFrame->getDialogFactory();

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_FILE_SAVEAS));
	UT_ASSERT(pDialog);

	UT_uint32 filterCount = 1;
	const char ** szDescList = (const char **) UT_calloc(filterCount + 1, sizeof(char *));
	const char ** szSuffixList = (const char **) UT_calloc(filterCount + 1, sizeof(char *));
	IEFileType * nTypeList = (IEFileType *) UT_calloc(filterCount + 1, sizeof(IEFileType));

	// we only support saving images in png format for now
	szDescList[0] = "Portable Network Graphics (.png)";
	szSuffixList[0] = "*.png";
	nTypeList[0] = (IEFileType)1;

	pDialog->setFileTypeList(szDescList, szSuffixList,
							 (const UT_sint32 *) nTypeList);

	pDialog->setDefaultFileType((IEFileType)1);

	pDialog->runModal(pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const char * szResultPathname = pDialog->getPathname();
		if (szResultPathname && *szResultPathname)
		{
			FV_View * pView = static_cast<FV_View *>(pAV_View);
			pView->saveSelectedImage (szResultPathname);
		}
	}

	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);

	pDialogFactory->releaseDialog(pDialog);

	return true;
}

Defun1(filePreviewWeb)
{
	CHECK_FRAME;
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
		if( UT_stricmp( pWhere, ".html" ) && UT_stricmp( pWhere, ".htm" ) )
			strcpy( pWhere, ".html" );
	  }
	  else
		  strcat( szTempFileName, ".html" );
  }

  UT_Error errSaved = UT_OK;

  // we do this because we don't want to change the default
  // document extension or rename what we're working on
  errSaved = pAV_View->cmdSaveAs(szTempFileName, IE_Exp::fileTypeForSuffix(".xhtml"), false);

  if(errSaved != UT_OK)
	{
	  // throw up a dialog
	  s_TellSaveFailed(pFrame, szTempFileName, errSaved);
	  return false;
	}

  char * tmpUrl = NULL;

  if (szTempFileName[0] == '/')
	  tmpUrl = UT_catPathname("file://", szTempFileName);
  else
	  tmpUrl = UT_catPathname("file:///", szTempFileName);

  bool bOk = _helpOpenURL(pAV_View, tmpUrl);
  FREEP(tmpUrl);

#if 0
  // ugly race condition
  UT_unlink (szTempFileName);
#endif

  return bOk;
}

Defun1(undo)
{
	CHECK_FRAME;
	pAV_View->cmdUndo(1);
	return true;
}

Defun1(redo)
{
	CHECK_FRAME;
	pAV_View->cmdRedo(1);
	return true;
}

Defun1(newWindow)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	XAP_Frame * pClone = pFrame->cloneFrame();
	if(!pClone)
	{
		return false;
	}
	s_StartStopLoadingCursor(true,pClone);
	pClone = pFrame->buildFrame(pClone);
	s_StartStopLoadingCursor(false,pClone);
	return (pClone ? true : false);
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
	CHECK_FRAME;
	return _openRecent(pAV_View, 1);
}
Defun1(openRecent_2)
{
	CHECK_FRAME;
	return _openRecent(pAV_View, 2);
}
Defun1(openRecent_3)
{
	CHECK_FRAME;
	return _openRecent(pAV_View, 3);
}
Defun1(openRecent_4)
{
	CHECK_FRAME;
	return _openRecent(pAV_View, 4);
}
Defun1(openRecent_5)
{
	CHECK_FRAME;
	return _openRecent(pAV_View, 5);
}
Defun1(openRecent_6)
{
	CHECK_FRAME;
	return _openRecent(pAV_View, 6);
}
Defun1(openRecent_7)
{
	CHECK_FRAME;
	return _openRecent(pAV_View, 7);
}
Defun1(openRecent_8)
{
	CHECK_FRAME;
	return _openRecent(pAV_View, 8);
}
Defun1(openRecent_9)
{
	CHECK_FRAME;
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
	CHECK_FRAME;
	return _activateWindow(pAV_View, 1);
}
Defun1(activateWindow_2)
{
	CHECK_FRAME;
	return _activateWindow(pAV_View, 2);
}
Defun1(activateWindow_3)
{
	CHECK_FRAME;
	return _activateWindow(pAV_View, 3);
}
Defun1(activateWindow_4)
{
	CHECK_FRAME;
	return _activateWindow(pAV_View, 4);
}
Defun1(activateWindow_5)
{
	CHECK_FRAME;
	return _activateWindow(pAV_View, 5);
}
Defun1(activateWindow_6)
{
	CHECK_FRAME;
	return _activateWindow(pAV_View, 6);
}
Defun1(activateWindow_7)
{
	CHECK_FRAME;
	return _activateWindow(pAV_View, 7);
}
Defun1(activateWindow_8)
{
	CHECK_FRAME;
	return _activateWindow(pAV_View, 8);
}
Defun1(activateWindow_9)
{
	CHECK_FRAME;
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
	if (!pDialog)
		return false;

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
	CHECK_FRAME;
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
	if (!pDialog)
		return false;

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

#if 0
	// we do not need selection (if there is none, we will try to apply
	// the case to the word at editing position)
	if (pView->isSelectionEmpty())
	  {
		pFrame->showMessageBox(AP_STRING_ID_MSG_EmptySelection,
				   XAP_Dialog_MessageBox::b_O,
				   XAP_Dialog_MessageBox::a_OK);
		return false;
	  }
#endif

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_ToggleCase * pDialog
		= (AP_Dialog_ToggleCase *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

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
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	return s_doToggleCase(pFrame, static_cast<FV_View *>(pAV_View), AP_DIALOG_ID_TOGGLECASE);
}

Defun1(rotateCase)
{
	CHECK_FRAME;
	FV_View * pView = static_cast<FV_View *>(pAV_View);
	pView->toggleCase(CASE_ROTATE);

	return true;
}

Defun1(dlgAbout)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	s_doAboutDlg(pFrame, XAP_DIALOG_ID_ABOUT);

	return true;
}

Defun(dlgMetaData)
{
  CHECK_FRAME;

  FV_View * pView = static_cast<FV_View *>(pAV_View);

  XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
  UT_ASSERT(pFrame);

  XAP_App * pApp = pFrame->getApp();
  UT_ASSERT(pApp);

  pFrame->raise();

  XAP_DialogFactory * pDialogFactory
    = (XAP_DialogFactory *)(pFrame->getDialogFactory());

  AP_Dialog_MetaData * pDialog
    = (AP_Dialog_MetaData *)(pDialogFactory->requestDialog(AP_DIALOG_ID_METADATA));
  UT_ASSERT(pDialog);
  if (!pDialog)
    return false;

  // get the properties

  PD_Document * pDocument = pView->getDocument();

  UT_String prop ( "" ) ;

  if ( pDocument->getMetaDataProp ( PD_META_KEY_TITLE, prop ) )
    pDialog->setTitle ( prop ) ;
  if ( pDocument->getMetaDataProp ( PD_META_KEY_SUBJECT, prop ) )
    pDialog->setSubject ( prop ) ;
  if ( pDocument->getMetaDataProp ( PD_META_KEY_CREATOR, prop ) )
    pDialog->setAuthor ( prop ) ;
  if ( pDocument->getMetaDataProp ( PD_META_KEY_PUBLISHER, prop ) )
    pDialog->setPublisher ( prop ) ;
  if ( pDocument->getMetaDataProp ( PD_META_KEY_CONTRIBUTOR, prop ) )
    pDialog->setCoAuthor ( prop ) ;
  if ( pDocument->getMetaDataProp ( PD_META_KEY_TYPE, prop ) )
    pDialog->setCategory ( prop ) ;
  if ( pDocument->getMetaDataProp ( PD_META_KEY_KEYWORDS, prop ) )
    pDialog->setKeywords ( prop ) ;
  if ( pDocument->getMetaDataProp ( PD_META_KEY_LANGUAGE, prop ) )
    pDialog->setLanguages ( prop ) ;
  if ( pDocument->getMetaDataProp ( PD_META_KEY_SOURCE, prop ) )
    pDialog->setSource ( prop ) ;
  if ( pDocument->getMetaDataProp ( PD_META_KEY_RELATION, prop ) )
    pDialog->setRelation ( prop ) ;
  if ( pDocument->getMetaDataProp ( PD_META_KEY_COVERAGE, prop ) )
    pDialog->setCoverage ( prop ) ;
  if ( pDocument->getMetaDataProp ( PD_META_KEY_RIGHTS, prop ) )
    pDialog->setRights ( prop ) ;
  if ( pDocument->getMetaDataProp ( PD_META_KEY_DESCRIPTION, prop ) )
    pDialog->setDescription ( prop ) ;

  // run the dialog

  pDialog->runModal(pFrame);
  bool bOK = (pDialog->getAnswer() == AP_Dialog_MetaData::a_OK);

  if (bOK)
    {
      // reset the props
      pDocument->setMetaDataProp ( PD_META_KEY_TITLE, pDialog->getTitle() ) ;
      pDocument->setMetaDataProp ( PD_META_KEY_SUBJECT, pDialog->getSubject() ) ;
      pDocument->setMetaDataProp ( PD_META_KEY_CREATOR, pDialog->getAuthor() ) ;
      pDocument->setMetaDataProp ( PD_META_KEY_PUBLISHER, pDialog->getPublisher() ) ;
      pDocument->setMetaDataProp ( PD_META_KEY_CONTRIBUTOR, pDialog->getCoAuthor() ) ;
      pDocument->setMetaDataProp ( PD_META_KEY_TYPE, pDialog->getCategory() ) ;
      pDocument->setMetaDataProp ( PD_META_KEY_KEYWORDS, pDialog->getKeywords() ) ;
      pDocument->setMetaDataProp ( PD_META_KEY_LANGUAGE, pDialog->getLanguages() ) ;
      pDocument->setMetaDataProp ( PD_META_KEY_SOURCE, pDialog->getSource() ) ;
      pDocument->setMetaDataProp ( PD_META_KEY_RELATION, pDialog->getRelation() ) ;
      pDocument->setMetaDataProp ( PD_META_KEY_COVERAGE, pDialog->getCoverage() ) ;
      pDocument->setMetaDataProp ( PD_META_KEY_RIGHTS, pDialog->getRights() ) ;
      pDocument->setMetaDataProp ( PD_META_KEY_DESCRIPTION, pDialog->getDescription() ) ;

      // TODO: set the document as dirty when something changed
    }

  // release the dialog

  pDialogFactory->releaseDialog(pDialog);
  return true ;
}

Defun(fileNew)
{
	CHECK_FRAME;
	FV_View * pView = static_cast<FV_View *>(pAV_View);

	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	UT_ASSERT(pFrame);

	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_New * pDialog
		= (AP_Dialog_New *)(pDialogFactory->requestDialog(AP_DIALOG_ID_FILE_NEW));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

	pDialog->runModal(pFrame);
	bool bOK = (pDialog->getAnswer() == AP_Dialog_New::a_OK);

	if (bOK)
	{
		UT_String str;
		const char * szStr;

		switch(pDialog->getOpenType())
		{
			// this will just open up a blank document
		case AP_Dialog_New::open_New :
			break;

			// these two will open things as templates
		case AP_Dialog_New::open_Existing :
		case AP_Dialog_New::open_Template :
			szStr = pDialog->getFileName();
			if (szStr)
				str += szStr;
			break;
		}

		if (str.size())
		{
			// we want to create from a template
		  if ( pDialog->getOpenType() == AP_Dialog_New::open_Existing )
		    bOK = ::fileOpen(pFrame, str.c_str(), IEFT_Unknown) == UT_OK;
		  else
		    bOK = s_importFile (pFrame, str.c_str(), IEFT_Unknown) == UT_OK;
		}
		else
		{
			// we want a new blank doc
			XAP_Frame * pNewFrame = pApp->newFrame();

			if (pNewFrame)
				pFrame = pNewFrame;

			bOK = pFrame->loadDocument(NULL, IEFT_Unknown) == UT_OK;

			if (pNewFrame)
			{
				pNewFrame->show();
			}
		}
	}

	pDialogFactory->releaseDialog(pDialog);
	return bOK;
}

static bool _helpOpenURL(XAP_Frame * pFrame, const char* helpURL)
{
	UT_return_val_if_fail(pFrame, false);
	pFrame->openHelpURL(helpURL);
	return true;
}

static bool _openURL(AV_View* pAV_View, const char* url)
{
	XAP_Frame* pFrame = static_cast<XAP_Frame*> (pAV_View->getParentData());
	pFrame->openURL (url);
	return true;
}

static bool _helpOpenURL(AV_View* pAV_View, const char* helpURL)
{
	XAP_Frame* pFrame = static_cast<XAP_Frame*> (pAV_View->getParentData());
	return _helpOpenURL ( pFrame, helpURL ) ;
}

inline static void _catPath(UT_String& st, const char* st2)
{
	if (st.size() > 0)
	{
		if (st[st.size() - 1] != '/')
			st += '/';
	}
	else
		st += '/';

	st += st2;
}

bool helpLocalizeAndOpenURL(XAP_Frame * pFrame, bool bLocal, const char* pathBeforeLang, const char* pathAfterLang)
{	
	UT_return_val_if_fail(pFrame, false);
	XAP_App* pApp = pFrame->getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs* pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs,false);

	const char* abiSuiteLibDir = pApp->getAbiSuiteLibDir();
	const XML_Char* abiSuiteLocString = NULL;
	UT_String url;

	pPrefs->getPrefsValue((XML_Char*)AP_PREF_KEY_StringSet, &abiSuiteLocString);

	if (bLocal)
	{
		UT_String path(abiSuiteLibDir);
		_catPath(path, pathBeforeLang);

		UT_String localized_path(path);
		_catPath(localized_path, abiSuiteLocString);

		if (UT_directoryExists(localized_path.c_str()))
		{
			// the localised help exists, so use it
			path = localized_path;
		}
		else
		{
			// the localised help directory does not exist, so fall back to the
			// en-US help localtion, which should always be available
			localized_path = path;
			_catPath(localized_path, "en-US");
			UT_DEBUGMSG(("help does not exist, using en-US instead\n"));
		}

		_catPath(localized_path, pathAfterLang);
		url = "file://";
		url += localized_path;
	}
	else {
		//TODO: No one uses this, so what kind of prefix should it have?
		url = pathBeforeLang;
		_catPath(url, abiSuiteLocString);
		_catPath(url, pathAfterLang);
	}

	return _helpOpenURL(pFrame, url.c_str());
}
	
static bool _helpLocalizeAndOpenURL(AV_View* pAV_View, bool bLocal, const char* pathBeforeLang, const char* pathAfterLang)
{
	XAP_Frame* pFrame = static_cast<XAP_Frame*> (pAV_View->getParentData());

	return helpLocalizeAndOpenURL(pFrame, bLocal, pathBeforeLang, pathAfterLang);
}

Defun1(helpContents)
{
	CHECK_FRAME;
	return _helpLocalizeAndOpenURL(pAV_View, true, "AbiWord/help", "index.html");
}

Defun1(helpIndex)
{
	CHECK_FRAME;
	return _helpLocalizeAndOpenURL(pAV_View, true, "AbiWord/help", "introduction.html");
}

Defun1(helpCheckVer)
{
	CHECK_FRAME;
	UT_String versionURL = "http://www.abisource.com/users/check_version.phtml?version=";
	versionURL += XAP_App::s_szBuild_Version;
	return _openURL(pAV_View, versionURL.c_str());
}

Defun1(helpReportBug)
{
	CHECK_FRAME;
	UT_String bugURL = "http://bugzilla.abisource.com/enter_bug.cgi?product=AbiWord";

  bugURL += "&version=";
  bugURL += XAP_App::s_szBuild_Version;
  bugURL += "&comment=(";
  bugURL += XAP_App::s_szBuild_Options;
  bugURL += ")%0d%0a%0d%0a";

  return _openURL(pAV_View, bugURL.c_str());
}

Defun1(helpSearch)
{
	CHECK_FRAME;
	return _helpLocalizeAndOpenURL(pAV_View, true, "AbiWord/help", "search.html");
}

Defun1(helpCredits)
{
	CHECK_FRAME;
	return _helpLocalizeAndOpenURL(pAV_View, true, "AbiWord/help", "credits.html");
}

Defun1(helpAboutGnu)
{
	CHECK_FRAME;
	return _openURL(pAV_View, "http://www.gnu.org/philosophy/");
}

Defun1(helpAboutGnomeOffice)
{
	CHECK_FRAME;
	return _openURL(pAV_View, "http://www.gnome.org/gnome-office/");
}

Defun1(helpAboutOS)
{
	CHECK_FRAME;
	return _helpLocalizeAndOpenURL(pAV_View, true, "AbiWord/help", "aboutos.html");
}

Defun1(cycleWindows)
{
	CHECK_FRAME;
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
	CHECK_FRAME;
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

static bool
s_closeWindow (AV_View * pAV_View, EV_EditMethodCallData * pCallData,
		   bool bCanExit)
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
	if (1 >= pApp->getFrameCount())
	{
		// Delete all the open modeless dialogs

		pApp->closeModelessDlgs();
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
				return false;					//	  so don't close
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

		if (bCanExit)
		{
			pApp->reallyExit();
		}
		else
		{
			// keep the app open with an empty document (in this frame)
			pFrame->loadDocument(NULL, IEFT_Unknown);
			pFrame->show();
			return true;
		}
	}

	// nuke the window

	pApp->forgetFrame(pFrame);
	pFrame->close();
	delete pFrame;

	return true;
}

Defun(closeWindow)
{
	CHECK_FRAME;
#if !defined(XP_UNIX_TARGET_GTK)
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);

	bool close = false;

	pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_CloseOnLastDoc, &close);
	return s_closeWindow (pAV_View, pCallData, close);
#else
	// must, to comply with the HIG
	return s_closeWindow (pAV_View, pCallData, true);
#endif
}

Defun(closeWindowX)
{
	CHECK_FRAME;
	return s_closeWindow (pAV_View, pCallData, true);
}

Defun(querySaveAndExit)
{
	CHECK_FRAME;
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

		bRet = s_closeWindow (pAV_View, pCallData, true);

		ndx--;
	}

	if (bRet)
	{

			//	delete all open modeless dialogs
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

		1.	XAP methods (above)
		2.	AbiWord-specific methods (below)

	Until we do the necessary architectural work, we just segregate
	the methods within the same file.
*/

Defun(fileRevert)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

  XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());

  if (XAP_Dialog_MessageBox::a_YES == pFrame->showMessageBox(AP_STRING_ID_MSG_RevertFile,
								 XAP_Dialog_MessageBox::b_YN,
								 XAP_Dialog_MessageBox::a_NO))
	  pView->cmdUndo ( pView->undoCount(true) - pView->undoCount(false) );

  return true;
}

Defun1(insertClipart)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_DEBUGMSG(("DOM: insert clipart\n"));

	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	UT_ASSERT(pFrame);

	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_ClipArt * pDialog
		= (XAP_Dialog_ClipArt *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_CLIPART));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

	// set the initial directory
	UT_String dir(pApp->getAbiSuiteLibDir());
	dir += "/clipart/";

	pDialog->setInitialDir (dir.c_str());

	pDialog->runModal(pFrame);
	bool bOK = (pDialog->getAnswer() == XAP_Dialog_ClipArt::a_OK);
	const char * pNewFile = pDialog->getGraphicName ();

	bool ret = false;

	if (bOK && pNewFile)
	{
		IEGraphicFileType iegft = IEGFT_Unknown;
		IE_ImpGraphic *pIEG;
		FG_Graphic* pFG;

		UT_Error errorCode;

		errorCode = IE_ImpGraphic::constructImporter(pNewFile, iegft, &pIEG);
		if(errorCode)
		{
			s_CouldNotLoadFileMessage(pFrame, pNewFile, errorCode);
			goto Cleanup;
		}

		errorCode = pIEG->importGraphic(pNewFile, &pFG);
		if(errorCode)
		{
			s_CouldNotLoadFileMessage(pFrame, pNewFile, errorCode);
			DELETEP(pIEG);
			goto Cleanup;
		}

		DELETEP(pIEG);

		errorCode = pView->cmdInsertGraphic(pFG, pNewFile);
		if (errorCode)
		{
			s_CouldNotLoadFileMessage(pFrame, pNewFile, errorCode);
			DELETEP(pFG);
			goto Cleanup;
		}

		DELETEP(pFG);
		ret = true; // goes to Cleanup
	}

 Cleanup:

	pDialogFactory->releaseDialog(pDialog);
	return ret;
}

Defun1(fileInsertGraphic)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	char* pNewFile = NULL;


	IEGraphicFileType iegft = IEGFT_Unknown;
	bool bOK = s_AskForGraphicPathname(pFrame,&pNewFile,&iegft);

	if (!bOK || !pNewFile)
		return false;

	// we own storage for pNewFile and must free it.
	UT_DEBUGMSG(("fileInsertGraphic: loading [%s]\n",pNewFile));

	IE_ImpGraphic *pIEG;
	FG_Graphic* pFG;

	UT_Error errorCode;

	errorCode = IE_ImpGraphic::constructImporter(pNewFile, iegft, &pIEG);
	if(errorCode != UT_OK)
	  {
		s_CouldNotLoadFileMessage(pFrame, pNewFile, errorCode);
		FREEP(pNewFile);
		return false;
	  }

	errorCode = pIEG->importGraphic(pNewFile, &pFG);
	if(errorCode != UT_OK || !pFG)
	  {
		s_CouldNotLoadFileMessage(pFrame, pNewFile, errorCode);
		FREEP(pNewFile);
		DELETEP(pIEG);
		return false;
	  }

	DELETEP(pIEG);

	ABIWORD_VIEW;

	errorCode = pView->cmdInsertGraphic(pFG, pNewFile);
	if (errorCode != UT_OK)
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

Defun(selectObject)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->warpInsPtToXY(pCallData->m_xPos, pCallData->m_yPos, true);
	pView->extSelHorizontal(true, 1); // move point forward one
	return true;
}

Defun(warpInsPtToXY)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->warpInsPtToXY(pCallData->m_xPos, pCallData->m_yPos, true);

	return true;
}

Defun1(warpInsPtLeft)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdCharMotion(pView->getCurrentBlock()->getDominantDirection() == FRIBIDI_TYPE_RTL,1);
	return true;
}

Defun1(warpInsPtRight)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdCharMotion(pView->getCurrentBlock()->getDominantDirection() != FRIBIDI_TYPE_RTL,1);
	return true;
}

Defun1(warpInsPtBOP)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOP);
	return true;
}

Defun1(warpInsPtEOP)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_EOP);
	return true;
}

Defun1(warpInsPtBOL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOL);
	return true;
}

Defun1(warpInsPtEOL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_EOL);
	return true;
}

Defun1(warpInsPtBOW)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	if(pView->getCurrentBlock()->getDominantDirection()==FRIBIDI_TYPE_RTL)
		pView->moveInsPtTo(FV_DOCPOS_EOW_MOVE);
	else
		pView->moveInsPtTo(FV_DOCPOS_BOW);

	return true;
}

Defun1(warpInsPtEOW)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	if(pView->getCurrentBlock()->getDominantDirection()==FRIBIDI_TYPE_RTL)
		pView->moveInsPtTo(FV_DOCPOS_BOW);
	else
		pView->moveInsPtTo(FV_DOCPOS_EOW_MOVE);

	return true;
}

Defun0(warpInsPtBOS)
{
	CHECK_FRAME;
	return true;
}

Defun0(warpInsPtEOS)
{
	CHECK_FRAME;
	return true;
}

Defun1(warpInsPtBOB)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOB);
	return true;
}

Defun1(warpInsPtEOB)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_EOB);
	return true;
}

Defun1(warpInsPtBOD)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOD);
	return true;
}

Defun1(warpInsPtEOD)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_EOD);
	return true;
}

Defun1(warpInsPtPrevPage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->warpInsPtNextPrevPage(false);
	return true;
}

Defun1(warpInsPtNextPage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->warpInsPtNextPrevPage(true);
	return true;
}

Defun1(warpInsPtPrevScreen)
{
	ABIWORD_VIEW;
	pView->warpInsPtNextPrevScreen(false);
	return true;
}

Defun1(warpInsPtNextScreen)
{
	ABIWORD_VIEW;
	pView->warpInsPtNextPrevScreen(true);
	return true;
}

Defun1(warpInsPtPrevLine)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->warpInsPtNextPrevLine(false);
	return true;
}

Defun1(warpInsPtNextLine)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->warpInsPtNextPrevLine(true);
	return true;
}

/*****************************************************************/

Defun1(cursorDefault)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if (pG)
	{
		pG->setCursor(GR_Graphics::GR_CURSOR_DEFAULT);
	}
	return true;
}

Defun1(cursorIBeam)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if (pG)
	{
		pG->setCursor(GR_Graphics::GR_CURSOR_IBEAM);
	}
	return true;
}

Defun1(cursorRightArrow)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if (pG)
	{
		pG->setCursor(GR_Graphics::GR_CURSOR_RIGHTARROW);
	}
	return true;
}


Defun1(cursorVline)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if (pG)
	{
		pG->setCursor(GR_Graphics::GR_CURSOR_VLINE_DRAG);
	}
	return true;
}


Defun1(cursorHline)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if (pG)
	{
		pG->setCursor(GR_Graphics::GR_CURSOR_HLINE_DRAG);
	}
	return true;
}

Defun1(cursorLeftArrow)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if (pG)
	{
		pG->setCursor(GR_Graphics::GR_CURSOR_LEFTARROW);
	}
	return true;
}

Defun1(cursorImage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if (pG)
	{
		pG->setCursor(GR_Graphics::GR_CURSOR_IMAGE);
	}
	return true;
}

Defun1(cursorImageSize)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if (pG)
	{
		// set the mouse cursor to the appropriate shape
		pG->setCursor( pView->getImageSelCursor() );
	}
	return true;
}

/*****************************************************************/

Defun1(contextMenu)
{
	CHECK_FRAME;
// raise context menu over whatever we are over.  this is
	// intended for use by the keyboard accelerator rather than
	// the other "targeted" context{...} methods which are bound
	// to the mouse.

	ABIWORD_VIEW;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_ASSERT(pFrame);

	UT_sint32 xPos, yPos;
	EV_EditMouseContext emc = pView->getInsertionPointContext(&xPos,&yPos);

	const char * szContextMenuName = XAP_App::getApp()->getMenuFactory()->FindContextMenu(emc);
	if (!szContextMenuName)
		return false;
	bool res =	pFrame->runModalContextMenu(pView,szContextMenuName,xPos,yPos);
	return res;
}

static bool s_doContextMenu_no_move( EV_EditMouseContext emc,
											UT_sint32 xPos,
											UT_sint32 yPos,
											FV_View * pView,
											XAP_Frame * pFrame)
{
	const char * szContextMenuName =  XAP_App::getApp()->getMenuFactory()->FindContextMenu(emc);
	xxx_UT_DEBUGMSG(("Context Menu Name is........ %s \N",szContextMenuName));
	if (!szContextMenuName)
		return false;
	bool res =	pFrame->runModalContextMenu(pView,szContextMenuName,
									   xPos,yPos);
	return res;
}

bool static s_doContextMenu(EV_EditMouseContext emc,
							UT_sint32 xPos,
							UT_sint32 yPos,
							FV_View * pView,
							XAP_Frame * pFrame)
{
	// move the IP so actions have the right context
	if (!pView->isXYSelected(xPos, yPos))
		pView->warpInsPtToXY(xPos,yPos,true);

	return s_doContextMenu_no_move(emc,xPos,yPos,pView,pFrame);
}

Defun(contextText)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_ASSERT(pFrame);
	return s_doContextMenu(EV_EMC_TEXT,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
}

Defun(contextRevision)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_ASSERT(pFrame);
	return s_doContextMenu(EV_EMC_REVISION,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
}

Defun(contextMisspellText)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_ASSERT(pFrame);
	return s_doContextMenu(EV_EMC_MISSPELLEDTEXT,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
}

Defun(contextImage)
{
CHECK_FRAME;
	ABIWORD_VIEW;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_ASSERT(pFrame);

	if ( pView->isSelectionEmpty () )
	  {
		// select the image if it isn't already
		UT_DEBUGMSG(("Selecting image: %d\n", pCallData->m_xPos));
		pView->warpInsPtToXY(pCallData->m_xPos, pCallData->m_yPos, true);
		pView->extSelHorizontal (true, 1);
	  }
	return s_doContextMenu(EV_EMC_IMAGE,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
}

Defun(contextHyperlink)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_ASSERT(pFrame);

	// move the IP so actions have the right context
	if (!pView->isXYSelected(pCallData->m_xPos, pCallData->m_yPos))
		EX(warpInsPtToXY);

	if(pView->isTextMisspelled())
		return s_doContextMenu_no_move(EV_EMC_HYPERLINKMISSPELLED,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
	else
		return s_doContextMenu_no_move(EV_EMC_HYPERLINKTEXT,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);

}

static bool _spellSuggest(AV_View* pAV_View, UT_uint32 ndx)
{
	ABIWORD_VIEW;
	pView->cmdContextSuggest(ndx);
	return true;
}

Defun1(spellSuggest_1)
{
	CHECK_FRAME;
	return _spellSuggest(pAV_View, 1);
}
Defun1(spellSuggest_2)
{
	CHECK_FRAME;
	return _spellSuggest(pAV_View, 2);
}
Defun1(spellSuggest_3)
{
	CHECK_FRAME;
	return _spellSuggest(pAV_View, 3);
}
Defun1(spellSuggest_4)
{
	CHECK_FRAME;
	return _spellSuggest(pAV_View, 4);
}
Defun1(spellSuggest_5)
{
	CHECK_FRAME;
	return _spellSuggest(pAV_View, 5);
}
Defun1(spellSuggest_6)
{
	CHECK_FRAME;
	return _spellSuggest(pAV_View, 6);
}
Defun1(spellSuggest_7)
{
	CHECK_FRAME;
	return _spellSuggest(pAV_View, 7);
}
Defun1(spellSuggest_8)
{
	CHECK_FRAME;
	return _spellSuggest(pAV_View, 8);
}
Defun1(spellSuggest_9)
{
	CHECK_FRAME;
	return _spellSuggest(pAV_View, 9);
}

Defun1(spellIgnoreAll)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	pView->cmdContextIgnoreAll();
	return true;
}

Defun1(spellAdd)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	pView->cmdContextAdd();
	return true;
}


/*****************************************************************/

Defun(dragSelectionBegin)
{
  // begin of drag+drop edit

  CHECK_FRAME;
  //ABIWORD_VIEW;

  return true;
}

Defun(dragSelectionEnd)
{
  // end of drag+drop edit

  CHECK_FRAME;
  ABIWORD_VIEW;

  if (pView->isSelectionEmpty())
    return true;

  pView->endDragSelection(pCallData->m_xPos, pCallData->m_yPos);
  return true;
}

Defun(dragToXY)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->extSelToXY(pCallData->m_xPos, pCallData->m_yPos, true);
	return true;
}

Defun(dragToXYword)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->extSelToXYword(pCallData->m_xPos, pCallData->m_yPos, true);
	return true;
}

Defun(endDrag)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

pView->endDrag(pCallData->m_xPos, pCallData->m_yPos);
	return true;
}

Defun(extSelToXY)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->extSelToXY(pCallData->m_xPos, pCallData->m_yPos, false);
	return true;
}

Defun1(extSelLeft)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	pView->extSelHorizontal(pView->getCurrentBlock()->getDominantDirection()==FRIBIDI_TYPE_RTL,1);

	return true;
}

Defun1(extSelRight)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	pView->extSelHorizontal(pView->getCurrentBlock()->getDominantDirection()!=FRIBIDI_TYPE_RTL,1);

	return true;
}

Defun1(extSelBOL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_BOL);
	return true;
}

Defun1(extSelEOL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_EOL);
	return true;
}

Defun1(extSelBOW)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	if(pView->getCurrentBlock()->getDominantDirection()==FRIBIDI_TYPE_RTL)
		pView->extSelTo(FV_DOCPOS_EOW_MOVE);
	else
		pView->extSelTo(FV_DOCPOS_BOW);

	return true;
}

Defun1(extSelEOW)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	if(pView->getCurrentBlock()->getDominantDirection()==FRIBIDI_TYPE_RTL)
		pView->extSelTo(FV_DOCPOS_BOW);
	else
		pView->extSelTo(FV_DOCPOS_EOW_MOVE);

	return true;
}

Defun0(extSelBOS)
{
	CHECK_FRAME;
	return true;
}

Defun0(extSelEOS)
{
	CHECK_FRAME;
	return true;
}

Defun1(extSelBOB)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_BOB);
	return true;
}

Defun1(extSelEOB)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_EOB);
	return true;
}

Defun1(extSelBOD)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_BOD);
	return true;
}

Defun1(extSelEOD)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_EOD);
	return true;
}

Defun1(extSelPrevLine)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->extSelNextPrevLine(false);
	return true;
}

Defun1(extSelNextLine)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->extSelNextPrevLine(true);
	return true;
}

Defun1(extSelPageDown)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->extSelNextPrevPage(true);
	return true;
}

Defun1(extSelPageUp)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->extSelNextPrevPage(false);
	return true;
}

Defun1(extSelScreenDown)
{
	ABIWORD_VIEW;
	pView->extSelNextPrevScreen(true);
	return true;
}

Defun1(extSelScreenUp)
{
	ABIWORD_VIEW;
	pView->extSelNextPrevScreen(false);
	return true;
}

Defun(selectAll)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOD, FV_DOCPOS_EOD);
	return true;
}

Defun(selectWord)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOW, FV_DOCPOS_EOW_SELECT);
	return true;
}

Defun(selectLine)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOL, FV_DOCPOS_EOL);
	return true;
}

Defun(selectBlock)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOB, FV_DOCPOS_EOB);
	return true;
}

Defun1(delLeft)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdCharDelete(false,1);
	return true;
}

Defun1(delRight)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdCharDelete(true,1);
	return true;
}

Defun1(delBOL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_BOL);
	return true;
}

Defun1(delEOL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_EOL);
	return true;
}

Defun1(delBOW)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_BOW);
	return true;
}

Defun1(delEOW)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_EOW_MOVE);
	return true;
}

Defun0(delBOS)
{
	CHECK_FRAME;
	return true;
}

Defun0(delEOS)
{
	CHECK_FRAME;
	return true;
}

Defun1(delBOB)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_BOB);
	return true;
}

Defun1(delEOB)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_EOB);
	return true;
}

Defun1(delBOD)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_BOD);
	return true;
}

Defun1(delEOD)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_EOD);
	return true;
}

Defun(insertData)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdCharInsert(pCallData->m_pData, pCallData->m_dataLength);
	return true;
}

/*****************************************************************/
// TODO the bInsert parameter is currently not used; it would require
// changes to the dialogues so that if bInsert == false the dialogue
// would be labeled "Delete bookmark"

static bool s_doBookmarkDlg(FV_View * pView, bool /*bInsert*/)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_InsertBookmark * pDialog
		= (AP_Dialog_InsertBookmark *)(pDialogFactory->requestDialog(AP_DIALOG_ID_INSERTBOOKMARK));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

	if (!pView->isSelectionEmpty())
	{
		UT_UCSChar * buffer = pView->getSelectionText();
		pDialog->setSuggestedBM(buffer);
		FREEP(buffer);
	}

	pDialog->setDoc(pView);
	pDialog->runModal(pFrame);

	AP_Dialog_InsertBookmark::tAnswer ans = pDialog->getAnswer();

	if (ans == AP_Dialog_InsertBookmark::a_OK)
	{
			pView->cmdInsertBookmark(pDialog->getBookmark());
	}
	else if(ans == AP_Dialog_InsertBookmark::a_DELETE)
	{
			pView->cmdDeleteBookmark(pDialog->getBookmark());
	}
	pDialogFactory->releaseDialog(pDialog);

	return (ans == AP_Dialog_InsertBookmark::a_DELETE || ans == AP_Dialog_InsertBookmark::a_OK);
}

Defun1(insertBookmark)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	s_doBookmarkDlg(pView, true);
	return true;
}

Defun1(deleteBookmark)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	s_doBookmarkDlg(pView, false);
	return true;
}

/*****************************************************************/
static bool s_doHyperlinkDlg(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_InsertHyperlink * pDialog
		= (AP_Dialog_InsertHyperlink *)(pDialogFactory->requestDialog(AP_DIALOG_ID_INSERTHYPERLINK));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

	pDialog->setDoc(pView);

	pDialog->runModal(pFrame);

	AP_Dialog_InsertHyperlink::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_InsertHyperlink::a_OK);

	if (bOK)
	{
		pView->cmdInsertHyperlink(pDialog->getHyperlink());
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

Defun1(insertHyperlink)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	if(pView->isSelectionEmpty())
	{
		//No selection
		XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
		UT_ASSERT((pFrame));


		pFrame->showMessageBox(AP_STRING_ID_MSG_HyperlinkNoSelection, 
				       XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		return false;
	}
	else
		s_doHyperlinkDlg(pView);
	return true;
}

Defun(replaceChar)
{
	CHECK_FRAME;
//ABIWORD_VIEW;
	return ( EX(delRight) && EX(insertData) && EX(setEditVI) );
}

Defun0(insertSoftBreak)
{
	CHECK_FRAME;
	return true;
}

Defun1(insertParagraphBreak)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->insertParagraphBreak();
	return true;
}

Defun1(insertSectionBreak)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
//
// No section breaks in header/Footers
//
	if(pView->isHdrFtrEdit())
		return true;
	if(pView->isInTable())
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
		pFrame->showMessageBox(AP_STRING_ID_MSG_NoBreakInsideTable,
							   XAP_Dialog_MessageBox::b_O,
							   XAP_Dialog_MessageBox::a_OK);
		return true;
	}

	pView->insertSectionBreak();
	return true;
}

/*
  Note that within the piece table, we use the following
  representations:
	char code					meaning
	UCS_TAB  (tab)				tab
	UCS_LF	 (line feed)		forced line break
	UCS_VTAB (vertical tab) 	forced column break
	UCS_FF	 (form feed)		forced page break
*/

Defun1(insertTab)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_UCSChar c = UCS_TAB;
	pView->cmdCharInsert(&c,1);
	return true;
}

Defun1(insertLineBreak)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_UCSChar c = UCS_LF;
	pView->cmdCharInsert(&c,1);
	return true;
}

Defun1(insertColumnBreak)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
//
// No column breaks in header/Footers
//
	if(pView->isHdrFtrEdit())
		return true;
	if(pView->isInTable())
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
		pFrame->showMessageBox(AP_STRING_ID_MSG_NoBreakInsideTable,
							   XAP_Dialog_MessageBox::b_O,
							   XAP_Dialog_MessageBox::a_OK);
		return true;
	}

	UT_UCSChar c = UCS_VTAB;
	pView->cmdCharInsert(&c,1);
	return true;
}

Defun1(insertColsBefore)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	pView->cmdInsertCol(pView->getPoint(),true); // is before
	return true;
}


Defun1(insertColsAfter)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	pView->cmdInsertCol(pView->getPoint(),false); // is After
	return true;
}


Defun1(insertRowsBefore)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	pView->cmdInsertRow(pView->getPoint(),true); // is before
	return true;
}


Defun1(insertRowsAfter)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	pView->cmdInsertRow(pView->getPoint(),false); // is before
	return true;
}

/*********************************************************************************/

static bool s_doMergeCellsDlg(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_MergeCells * pDialog
		= (AP_Dialog_MergeCells *)(pDialogFactory->requestDialog(AP_DIALOG_ID_MERGE_CELLS));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

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


Defun1(mergeCells)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	s_doMergeCellsDlg(pView);
	return true;
}

/***********************************************************************************/

static bool s_doFormatTableDlg(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_FormatTable * pDialog
		= (AP_Dialog_FormatTable *)(pDialogFactory->requestDialog(AP_DIALOG_ID_FORMAT_TABLE));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

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


Defun1(formatTable)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	s_doFormatTableDlg(pView);
	return true;
}

/***********************************************************************************/

Defun1(deleteCell)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	pView->cmdDeleteCell(pView->getPoint());
	return true;
}


Defun1(deleteColumns)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	pView->cmdDeleteCol(pView->getPoint());
	return true;
}


Defun1(deleteRows)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	pView->cmdDeleteRow(pView->getPoint());
	return true;
}

Defun1(deleteTable)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	pView->cmdDeleteTable(pView->getPoint());
	return true;
}

Defun1(insertPageBreak)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_UCSChar c = UCS_FF;
//
// No page breaks in header/Footers
//
	if(pView->isHdrFtrEdit())
		return true;
	if(pView->isInTable())
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
		pFrame->showMessageBox(AP_STRING_ID_MSG_NoBreakInsideTable,
							   XAP_Dialog_MessageBox::b_O,
							   XAP_Dialog_MessageBox::a_OK);
		return true;
	}
	pView->cmdCharInsert(&c,1);
	return true;
}

Defun1(insertSpace)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_UCSChar c = UCS_SPACE;
	pView->cmdCharInsert(&c,1);
	return true;
}

Defun1(insertNBSpace)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_UCSChar c = UCS_NBSP;			// decimal 160 is NBS
	pView->cmdCharInsert(&c,1);
	return true;
}

// non-breaking, zerrow width 
Defun1(insertNBZWSpace)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_UCSChar c = 0xFEFF;
	pView->cmdCharInsert(&c,1);
	return true;
}

// zero width joiner
Defun1(insertZWJoiner)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_UCSChar c = 0x200D;
	pView->cmdCharInsert(&c,1);
	return true;
}

/*****************************************************************/

Defun(insertGraveData)
{
	CHECK_FRAME;
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
	CHECK_FRAME;
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
	CHECK_FRAME;
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Circumflex map are mapped here.	The desired
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
	CHECK_FRAME;
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
	CHECK_FRAME;
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Macron map are mapped here.	The desired
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
	CHECK_FRAME;
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
	CHECK_FRAME;
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
	CHECK_FRAME;
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
	CHECK_FRAME;
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
	case 0x4f:		doubleacuteChar=0x01d5; break;	// Odoubleacute
	case 0x55:		doubleacuteChar=0x01db; break;	// Udoubleacute

	case 0x6f:		doubleacuteChar=0x01f5; break;	// odoubleacute
	case 0x75:		doubleacuteChar=0x01fb; break;	// udoubleacute

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

	pView->cmdCharInsert(&doubleacuteChar, 1);
	return true;
}

Defun(insertCaronData)
{
	CHECK_FRAME;
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
	case 0x4c:		caronChar=0x013d;	break;	// Lcaron
	case 0x53:		caronChar=0x0160;	break;	// Scaron
	case 0x54:		caronChar=0x0164;	break;	// Tcaron
	case 0x5a:		caronChar=0x017d;	break;	// Zcaron
	case 0x43:		caronChar=0x010c;	break;	// Ccaron
	case 0x45:		caronChar=0x011a;	break;	// Ecaron
	case 0x44:		caronChar=0x010e;	break;	// Dcaron
	case 0x4e:		caronChar=0x0147;	break;	// Ncaron
	case 0x52:		caronChar=0x0158;	break;	// Rcaron

	case 0x6c:		caronChar=0x013e;	break;	// lcaron
	case 0x73:		caronChar=0x0161;	break;	// scaron
	case 0x74:		caronChar=0x0165;	break;	// tcaron
	case 0x7a:		caronChar=0x017e;	break;	// zcaron
	case 0x63:		caronChar=0x010d;	break;	// ccaron
	case 0x65:		caronChar=0x011b;	break;	// ecaron
	case 0x64:		caronChar=0x010f;	break;	// dcaron
	case 0x6e:		caronChar=0x0148;	break;	// ncaron
	case 0x72:		caronChar=0x0159;	break;	// rcaron

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

	pView->cmdCharInsert(&caronChar, 1);
	return true;
}

Defun(insertCedillaData)
{
	CHECK_FRAME;
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
	case 0x43:		cedillaChar=0x00c7; break;	// Ccedilla
	case 0x63:		cedillaChar=0x00e7; break;	// ccedilla

	// Latin-[24] characters
	case 0x53:		cedillaChar=0x01aa; break;	// Scedilla
	case 0x54:		cedillaChar=0x01de; break;	// Tcedilla
	case 0x52:		cedillaChar=0x03a3; break;	// Rcedilla
	case 0x4c:		cedillaChar=0x03a6; break;	// Lcedilla
	case 0x47:		cedillaChar=0x03ab; break;	// Gcedilla
	case 0x4e:		cedillaChar=0x03d1; break;	// Ncedilla
	case 0x4b:		cedillaChar=0x03d3; break;	// Kcedilla

	case 0x73:		cedillaChar=0x01ba; break;	// scedilla
	case 0x74:		cedillaChar=0x01fe; break;	// tcedilla
	case 0x72:		cedillaChar=0x03b3; break;	// rcedilla
	case 0x6c:		cedillaChar=0x03b6; break;	// lcedilla
	case 0x67:		cedillaChar=0x03bb; break;	// gcedilla
	case 0x6e:		cedillaChar=0x03f1; break;	// ncedilla
	case 0x6b:		cedillaChar=0x03f3; break;	// kcedilla

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

	pView->cmdCharInsert(&cedillaChar, 1);
	return true;
}

Defun(insertOgonekData)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// This function provides an interlude.  All of the keys
	// on the Dead_Ogonek map are mapped here.	The desired
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
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdCut();

	return true;
}

Defun1(copy)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdCopy();

	return true;
}

Defun1(paste)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdPaste();

	return true;
}

Defun(pasteSelection)
{
	CHECK_FRAME;
// this is intended for the X11 middle mouse thing.
	ABIWORD_VIEW;
	pView->cmdPasteSelectionAt(pCallData->m_xPos, pCallData->m_yPos);

	return true;
}

Defun1(pasteSpecial)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdPaste(false);

	return true;
}

static bool checkViewModeIsPrint(FV_View * pView)
{
	if(pView->getViewMode() != VIEW_PRINT)
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
		UT_ASSERT(pFrame);
		XAP_Dialog_MessageBox::tAnswer res = pFrame->showMessageBox(AP_STRING_ID_MSG_CHECK_PRINT_MODE,
				   XAP_Dialog_MessageBox::b_YN,
				   XAP_Dialog_MessageBox::a_NO);
		if(res == XAP_Dialog_MessageBox::a_NO)
		{
			return false;
		}
		else
		{

			AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();
			UT_ASSERT(pFrameData);

			pFrameData->m_pViewMode = VIEW_PRINT;
			pFrame->toggleLeftRuler (true && (pFrameData->m_bShowRuler) &&
									 (!pFrameData->m_bIsFullScreen));


			pView->setViewMode (VIEW_PRINT);

			// POLICY: make this the default for new frames, too
			XAP_App * pApp = pFrame->getApp();
			UT_ASSERT(pApp);
			XAP_Prefs * pPrefs = pApp->getPrefs();
			UT_ASSERT(pPrefs);
			XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
			UT_ASSERT(pScheme);

			pScheme->setValue(AP_PREF_KEY_LayoutMode, "1");

			pView->updateScreen(false);
			pView->notifyListeners(AV_CHG_ALL);
		}
	}
	return true;
}

Defun1(editFooter)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	if(checkViewModeIsPrint(pView))
	{
		pView->cmdEditFooter();
	}
	return true;
}

Defun1(removeHeader)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	if(checkViewModeIsPrint(pView))
	{
		pView->cmdRemoveHdrFtr(true);
	}
	return true;
}

Defun1(removeFooter)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	if(checkViewModeIsPrint(pView))
	{
		pView->cmdRemoveHdrFtr(false);
	}
	return true;
}

Defun1(editHeader)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	if(checkViewModeIsPrint(pView))
	{
		pView->cmdEditHeader();
	}
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
	if (!pDialog)
		return false;

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
	CHECK_FRAME;
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
   if (!pDialog)
	   return false;

   // run the dialog (it probably should be modeless if anyone
   // gets the urge to make it safe that way)
   pDialog->runModal(pFrame);
   bool bOK = pDialog->isComplete();

   if (bOK)
	   s_TellSpellDone(pFrame, pDialog->isSelection());

   pDialogFactory->releaseDialog(pDialog);

   return bOK;
}


Defun1(dlgSpell)
{
	CHECK_FRAME;
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
	if (!pDialog)
		return false;

	// don't match case by default
	pDialog->setMatchCase(false);

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
	CHECK_FRAME;
	ABIWORD_VIEW;
	XAP_Dialog_Id id = AP_DIALOG_ID_FIND;

	return s_doFindOrFindReplaceDlg(pView,id);
}

Defun1(findAgain)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	return pView->findAgain();
}

Defun1(replace)
{
	CHECK_FRAME;
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
	if (!pDialog)
		return false;

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
		static XML_Char szLang[50];

		if (pDialog->getChangedLangProperty(&s))
		{
			//UT_DEBUGMSG(("some change\n"));
			sprintf(szLang,"%s",s);
			props_out[k++] = "lang";
			props_out[k++] = (const XML_Char *) szLang;
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
	if (!pDialog)
		return false;

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
		pDialog->setBGColor(UT_getAttribute("bgcolor", props_in));
		
//
// Set the background color for the preview
//
		static XML_Char  background[8];
		UT_RGBColor * bgCol = pView->getCurrentPage()->getOwningSection()->getPaperColor();
		sprintf(background, "%02x%02x%02x",bgCol->m_red,
				bgCol->m_grn,bgCol->m_blu);
		pDialog->setBackGroundColor( (const XML_Char *) background);

		// these behave a little differently since they are
		// probably just check boxes and we don't have to
		// worry about initializing a combo box with a choice
		// (and because they are all stuck under one CSS attribute).

		bool bUnderline = false;
		bool bOverline = false;
		bool bStrikeOut = false;
		bool bTopLine = false;
		bool bBottomLine = false;

		const XML_Char * s = UT_getAttribute("text-decoration", props_in);
		if (s)
		{
			bUnderline = (strstr(s, "underline") != NULL);
			bOverline = (strstr(s, "overline") != NULL);
			bStrikeOut = (strstr(s, "line-through") != NULL);
			bTopLine = (strstr(s, "topline") != NULL);
			bBottomLine = (strstr(s, "bottomline") != NULL);
		}
		pDialog->setFontDecoration(bUnderline,bOverline,bStrikeOut,bTopLine,bBottomLine);

		bool bHidden = false;
		const XML_Char * h = UT_getAttribute("display", props_in);
		if(h)
		{
			bHidden = (strstr(h, "none") != NULL);
		}
		pDialog->setHidden(bHidden);
		
		bool bSuperScript = false;
		h = UT_getAttribute("text-position", props_in);
		if(h)
		{
			bSuperScript = (strstr(h, "superscript") != NULL);
		}
		pDialog->setSuperScript(bSuperScript);
		
		bool bSubScript = false;
		h = UT_getAttribute("text-position", props_in);
		if(h)
		{
			bSubScript = (strstr(h, "subscript") != NULL);
		}
		pDialog->setSuperScript(bSubScript);

		FREEP(props_in);
	}

	if(!pView->isSelectionEmpty())
	  {
	    // set the drawable string to the selection text
	    pDialog->setDrawString(pView->getSelectionText());
	  }

	// run the dialog

	pDialog->runModal(pFrame);

	// extract what they did

	bool bOK = (pDialog->getAnswer() == XAP_Dialog_FontChooser::a_OK);

	if (bOK)
	{
		UT_uint32  k = 0;
		const XML_Char * props_out[19];
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

		if (pDialog->getChangedBGColor(&s))
		{
			props_out[k++] = "bgcolor";
			props_out[k++] = s;
		}

		bool bUnderline = false;
		bool bChangedUnderline = pDialog->getChangedUnderline(&bUnderline);
		bool bOverline = false;
		bool bChangedOverline = pDialog->getChangedOverline(&bOverline);
		bool bStrikeOut = false;
		bool bChangedStrikeOut = pDialog->getChangedStrikeOut(&bStrikeOut);
		bool bTopline = false;
		bool bChangedTopline = pDialog->getChangedTopline(&bTopline);
		bool bBottomline = false;
		bool bChangedBottomline = pDialog->getChangedBottomline(&bBottomline);
		UT_String decors;
		static XML_Char sstr[50];
		if (bChangedUnderline || bChangedStrikeOut || bChangedOverline || bChangedTopline || bChangedBottomline)
		{
			decors.clear();
			if(bUnderline)
				decors += "underline ";
			if(bStrikeOut)
				decors += "line-through ";
			if(bOverline)
				decors += "overline ";
			if(bTopline)
				decors += "topline ";
			if(bBottomline)
				decors += "bottomline ";
			if(!bUnderline && !bStrikeOut && !bOverline && !bTopline && !bBottomline)
				decors = "none";
			sprintf(sstr,"%s",decors.c_str());
			props_out[k++] = "text-decoration";
			props_out[k++] = (const XML_Char *) sstr;
		}

		bool bHidden = false;
		bool bChangedHidden = pDialog->getChangedHidden(&bHidden);

		if (bChangedHidden)
		{
			if(bHidden)
			{
				props_out[k++] = "display";
				props_out[k++] = "none";
				
			}
			else
			{
				props_out[k++] = "display";
				props_out[k++] ="";
			}
		}
		
		bool bSuperScript = false;
		bool bChangedSuperScript = pDialog->getChangedSuperScript(&bSuperScript);

		if (bChangedSuperScript)
		{
			if(bSuperScript)
			{
				props_out[k++] = "text-position";
				props_out[k++] = "superscript";
				
			}
			else
			{
				props_out[k++] = "text-position";
				props_out[k++] ="";
			}
		}
		
		bool bSubScript = false;
		bool bChangedSubScript = pDialog->getChangedSubScript(&bSubScript);

		if (bChangedSubScript)
		{
			if(bSubScript)
			{
				props_out[k++] = "text-position";
				props_out[k++] = "subscript";
				
			}
			else
			{
				props_out[k++] = "text-position";
				props_out[k++] ="";
			}
		}


		props_out[k] = 0;						// put null after last pair.
		UT_ASSERT(k < NrElements(props_out));
		for(UT_uint32 i = 0; i<k; i= i+2 )
			UT_DEBUGMSG(("SEVIOR: Props = %s: Values = %s \n",props_out[i],props_out[i+1]));
		if (k > 0)								// if something changed
			pView->setCharFormat(props_out);
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

static void
s_TabSaveCallBack (AP_Dialog_Tab * pDlg, FV_View * pView,
				   const char * szTabStops, const char * szDflTabStop,
				   void * closure)
{
  UT_return_if_fail(szTabStops && szDflTabStop);

	const XML_Char * properties[3];
	properties[0] = "tabstops";
	properties[1] = szTabStops;
	properties[2] = 0;
	UT_DEBUGMSG(("AP_Dialog_Tab: Tab Stop [%s]\n",properties[1]));

	pView->setBlockFormat(properties);

	properties[0] = "default-tab-interval";
	properties[1] = szDflTabStop;
	properties[2] = 0;
	UT_ASSERT(properties[1]);
	UT_DEBUGMSG(("AP_Dialog_Tab: Default Tab Stop [%s]\n",properties[1]));

	pView->setBlockFormat(properties);
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
		// setup the callback function, no closure
		pDialog->setSaveCallback(s_TabSaveCallBack, NULL);

		// run the dialog
		pDialog->runModal(pFrame);

		// get the dialog answer
		AP_Dialog_Tab::tAnswer answer = pDialog->getAnswer();

		switch (answer)
		{
		case AP_Dialog_Tab::a_OK:
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
	if (!pDialog)
		return false;

	const XML_Char ** props = NULL;

	if (!pView->getBlockFormat(&props))
		return false;

	if (!pDialog->setDialogData(props))
		return false;

	FREEP(props);

	// let's steal the width from getTopRulerInfo.
	AP_TopRulerInfo info;
	pView->getTopRulerInfo(&info);

	// TODO tables
	pDialog->setMaxWidth (info.u.c.m_xColumnWidth);

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


static bool s_doOptionsDlg(FV_View * pView, int which = -1)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Options * pDialog
		= (AP_Dialog_Options *)(pDialogFactory->requestDialog(AP_DIALOG_ID_OPTIONS));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

	if ( which != -1 )
	  pDialog->setInitialPageNum(which);
	else
	  pDialog->setInitialPageNum(0);

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
	CHECK_FRAME;
	ABIWORD_VIEW;

	return s_doLangDlg(pView);
}

Defun(language)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * properties[] = { "lang", NULL, 0};

	char lang[10];
	UT_return_val_if_fail(pCallData->m_dataLength < sizeof(lang),false);

	UT_uint32 i = 0;
	for(i = 0; i < pCallData->m_dataLength; i++)
		lang[i] = (char) pCallData->m_pData[i];
	lang[i] = 0;
	
	properties[1] = (const XML_Char *) &lang;
	pView->setCharFormat(properties);
	return true;
}

/*****************************************************************/

Defun1(dlgFont)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	return s_doFontDlg(pView);
}

Defun(fontFamily)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * properties[] = { "font-family", NULL, 0};
	properties[1] = (const XML_Char *) pCallData->m_pData;
	pView->setCharFormat(properties);
	return true;
}

Defun(fontSize)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * properties[] = { "font-size", NULL, 0};

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

Defun(formatPainter)
{
  CHECK_FRAME;
  ABIWORD_VIEW;

  // prereqs: !pView->isSelectionEmpty() && XAP_App::getApp()->canPasteFromClipboard()
  // taken care of in ap_Toolbar_Functions.cpp::ap_ToolbarGetState_Clipboard

  const XML_Char ** block_properties = 0;
  const XML_Char ** span_properties  = 0;

  // get the current document's selected range
  PD_DocumentRange range;
  pView->getDocumentRangeOfCurrentSelection (&range);

  // now create a new (invisible) view to paste our clipboard contents into
  PD_Document * pNewDoc = new PD_Document(XAP_App::getApp());
  pNewDoc->newDocument();

  FL_DocLayout *pDocLayout = new FL_DocLayout(pNewDoc, pView->getGraphics());
  FV_View pPasteView (XAP_App::getApp(), 0, pDocLayout);
  pDocLayout->setView (&pPasteView);
  pDocLayout->fillLayouts();
  pDocLayout->formatAll();

  // paste contents
  pPasteView.cmdPaste (true);

  // select all so that we can get the block & span properties
  pPasteView.cmdSelect(0, 0, FV_DOCPOS_BOD, FV_DOCPOS_EOD);

  // get the paragraph and span/character formatting properties of
  // the clipboard selection
  if (!pPasteView.getBlockFormat(&block_properties))
    {
      UT_DEBUGMSG(("DOM: No block attributes in the new paragraph!\n"));
    }

  if (!pPasteView.getCharFormat(&span_properties))
    {
      UT_DEBUGMSG(("DOM: No span attributes in the new paragraph!\n"));
    }

  // reset what was selected before setting the block and char formatting
  pView->cmdSelect (range.m_pos1, range.m_pos2) ;

  // set the current selection's properties to what's on the clipboard
  if ( block_properties )
    pView->setBlockFormat (block_properties);
  if ( span_properties )
    pView->setCharFormat (span_properties);

  FREEP(block_properties);
  FREEP(block_properties);
  DELETEP(pDocLayout);
  UNREFP(pNewDoc);

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
	props_out[1] = vOn; 	// be optimistic

	XML_Char * buf = NULL;

	s = UT_getAttribute(prop, props_in);
	if (s)
	{
		if (bMultiple)
		{
			// some properties have multiple values
			XML_Char*	p = (XML_Char *)strstr(s, vOn);

			if (p)
			{
				// yep...
				if (strstr(s, vOff))
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

				// ... take it out
				int len = strlen(s);
				buf = (XML_Char *) UT_calloc(len, sizeof(XML_Char));

				strncpy(buf, s, p - s);
				strcat(buf, s + (p - s) + strlen(vOn));

				// now see if anything's left
				XML_Char * q;
				UT_cloneString((char *&)q, buf);

				if (q && strtok(q, " "))
					props_out[1] = buf; 	// yep, use it
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
					buf = (XML_Char *) UT_calloc(len, sizeof(XML_Char));

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

bool s_actuallyPrint(PD_Document *doc,  GR_Graphics *pGraphics,
		     FV_View * pPrintView, const char *pDocName,
		     UT_uint32 nCopies, bool bCollate,
		     UT_sint32 iWidth,  UT_sint32 iHeight,
		     UT_uint32 nToPage, UT_uint32 nFromPage)
{
	UT_uint32 j,k;

	//
	// Lock out operations on this document
	//
	s_pLoadingDoc = (AD_Document *) doc;

	if(pGraphics->startPrint())
	{
	  fp_PageSize ps = pPrintView->getPageSize();	  
	  bool orient = ps.isPortrait ();
	  pGraphics->setPortrait (orient);

	  const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet ();
	  const XML_Char * msgTmpl = pSS->getValue (AP_STRING_ID_MSG_PrintStatus);

	  XML_Char msgBuf [1024];

	  dg_DrawArgs da;
	  memset(&da, 0, sizeof(da));
	  da.pG = NULL;
	  
	  XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame ();

		if (bCollate)
		{
			for (j=1; (j <= nCopies); j++)
				for (k=nFromPage; (k <= nToPage); k++)
				{
					sprintf (msgBuf, msgTmpl, k, nToPage);

					if(pFrame) {
					  pFrame->setStatusMessage ( msgBuf );
					  pFrame->nullUpdate();
					}

					// NB we will need a better way to calc
					// pGraphics->m_iRasterPosition when
					// iHeight is allowed to vary page to page
					pGraphics->m_iRasterPosition = (k-1)*iHeight;
					pGraphics->startPage(pDocName, k, orient, iWidth, iHeight);
					pPrintView->draw(k-1, &da);
				}
		}
		else
		{
			for (k=nFromPage; (k <= nToPage); k++)
				for (j=1; (j <= nCopies); j++)
				{
					sprintf (msgBuf, msgTmpl, k, nToPage);

					if(pFrame) {
					  pFrame->setStatusMessage ( msgBuf );
					  pFrame->nullUpdate();
					}

					// NB we will need a better way to calc
					// pGraphics->m_iRasterPosition when
					// iHeight is allowed to vary page to page
					pGraphics->m_iRasterPosition = (k-1)*iHeight;
					pGraphics->startPage(pDocName, k, orient, iWidth, iHeight);
					pPrintView->draw(k-1, &da);
				}
		}
		pGraphics->endPrint();
		
		if(pFrame)
		  pFrame->setStatusMessage (""); // reset/0 out status bar
	}
	s_pLoadingDoc = NULL;
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
	if (!pDialog)
		return false;

	FL_DocLayout* pLayout = pView->getLayout();
	PD_Document * doc = pLayout->getDocument();

	pDialog->setPaperSize (pView->getPageSize().getPredefinedName());
	pDialog->setDocumentTitle(pFrame->getNonDecoratedTitle());
	pDialog->setDocumentPathname((doc->getFilename())
				     ? doc->getFilename()
				     : pFrame->getNonDecoratedTitle());
	pDialog->setEnablePageRangeButton(true,1,pLayout->countPages());
	pDialog->setEnablePrintSelection(false);	// TODO change this when we know how to do it.
	pDialog->setEnablePrintToFile(true);
	pDialog->setTryToBypassActualDialog(bTryToSuppressDialog);

	pDialog->runModal(pFrame);

	XAP_Dialog_Print::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_Print::a_OK);

	if (bOK)
	{

//
// Turn on Wait cursor
//
	        if(pView)
		{
		     pView->setCursorWait();
		}
		s_pLoadingFrame = pFrame;
		s_pLoadingDoc = (AD_Document *) doc;

		const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
		UT_String msg =  pSS->getValue(AP_STRING_ID_MSG_PrintingDoc);

		pFrame->setStatusMessage ( (XML_Char *) msg.c_str() );

		GR_Graphics * pGraphics = pDialog->getPrinterGraphicsContext();
		UT_ASSERT(pGraphics->queryProperties(GR_Graphics::DGP_PAPER));

		FL_DocLayout * pDocLayout = new FL_DocLayout(doc,pGraphics);
		FV_View * pPrintView = new FV_View(pFrame->getApp(),0,pDocLayout);
		pPrintView->getLayout()->fillLayouts();
		pPrintView->getLayout()->formatAll();

		UT_uint32 nFromPage, nToPage;
		(void)pDialog->getDoPrintRange(&nFromPage,&nToPage);

		if (nToPage > pLayout->countPages())
		  nToPage = pLayout->countPages();

		// TODO add code to handle getDoPrintSelection()

		UT_uint32 nCopies = pDialog->getNrCopies();
		bool bCollate = pDialog->getCollate();

		// TODO these are here temporarily to make printing work.  We'll fix the hack later.
		// BUGBUG assumes all pages are same size and orientation
		UT_sint32 iWidth = pLayout->getWidth();
		UT_sint32 iHeight = pLayout->getHeight() / pLayout->countPages();

		const char *pDocName = ((doc->getFilename()) ? doc->getFilename() : pFrame->getNonDecoratedTitle());

		s_actuallyPrint(doc, pGraphics, pPrintView, pDocName, nCopies, bCollate,
				iWidth,  iHeight, nToPage, nFromPage);

		delete pDocLayout;
		delete pPrintView;

		pDialog->releasePrinterGraphicsContext(pGraphics);

//
// Turn off wait cursor
//
		pView->clearCursorWait();
		s_pLoadingFrame = NULL;
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
	if (!pDialog)
		return false;

	FL_DocLayout* pLayout = pView->getLayout();
	PD_Document * doc = pLayout->getDocument();

//
// Turn on Wait cursor
//
	pView->setCursorWait();
	s_pLoadingDoc = (AD_Document *) doc;

	pDialog->setPaperSize (pView->getPageSize().getPredefinedName());
	pDialog->setDocumentTitle(pFrame->getNonDecoratedTitle());
	pDialog->setDocumentPathname((doc->getFilename())
					 ? doc->getFilename()
					 : pFrame->getNonDecoratedTitle());

	pDialog->runModal(pFrame);

	GR_Graphics * pGraphics = pDialog->getPrinterGraphicsContext();
	UT_ASSERT(pGraphics->queryProperties(GR_Graphics::DGP_PAPER));

	FL_DocLayout * pDocLayout = new FL_DocLayout(doc,pGraphics);
	FV_View * pPrintView = new FV_View(pFrame->getApp(),pFrame,pDocLayout);
	// delay filling layouts...

	UT_uint32 nFromPage = 1, nToPage = pLayout->countPages();

	if (nToPage > pLayout->countPages())
	  {
		nToPage = pLayout->countPages();
	  }

	UT_uint32 nCopies = 1;
	bool bCollate  = false;

	// TODO these are here temporarily to make printing work.  We'll fix the hack later.
	// BUGBUG assumes all pages are same size and orientation
	UT_sint32 iWidth = pLayout->getWidth();
	UT_sint32 iHeight = pLayout->getHeight() / pLayout->countPages();

	const char *pDocName = ((doc->getFilename()) ? doc->getFilename() : pFrame->getNonDecoratedTitle());

	s_actuallyPrint(doc, pGraphics, pPrintView, pDocName, nCopies, bCollate,
			iWidth,  iHeight, nToPage, nFromPage);

	delete pDocLayout;
	delete pPrintView;

	pDialog->releasePrinterGraphicsContext(pGraphics);

	pDialogFactory->releaseDialog(pDialog);
//
// Turn off wait cursor
//
	pView->clearCursorWait();
	s_pLoadingDoc = NULL;

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
	if (!pDialog)
		return false;

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
		s_StartStopLoadingCursor(true, pFrame);
		pFrame->setZoomPercentage(newZoom);
		s_StartStopLoadingCursor(false, pFrame);
	}
	else
	  {
	    // force toolbar combo refresh
	    pFrame->setZoomPercentage(pFrame->getZoomPercentage());
	  }

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

Defun1(zoom100)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
  UT_ASSERT(pFrame);

  pFrame->raise();

  UT_uint32 newZoom = 100;
  pFrame->setZoomType( XAP_Frame::z_100 );

  s_StartStopLoadingCursor(true, pFrame);
  pFrame->setZoomPercentage(newZoom);
  s_StartStopLoadingCursor(false, pFrame);

  return true;
}

Defun1(zoom200)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
  UT_ASSERT(pFrame);

  pFrame->raise();

  UT_uint32 newZoom = 200;
  pFrame->setZoomType( XAP_Frame::z_200 );

  s_StartStopLoadingCursor(true, pFrame);
  pFrame->setZoomPercentage(newZoom);
  s_StartStopLoadingCursor(false, pFrame);

  return true;
}

Defun1(zoom50)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
  UT_ASSERT(pFrame);

  pFrame->raise();

  UT_uint32 newZoom = 50;
  pFrame->setZoomType( XAP_Frame::z_PERCENT );

  s_StartStopLoadingCursor(true, pFrame);
  pFrame->setZoomPercentage(newZoom);
  s_StartStopLoadingCursor(false, pFrame);

  return true;
}

Defun1(zoom75)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
  UT_ASSERT(pFrame);

  pFrame->raise();

  UT_uint32 newZoom = 75;
  pFrame->setZoomType(	XAP_Frame::z_75 );

  s_StartStopLoadingCursor(true, pFrame);
  pFrame->setZoomPercentage(newZoom);
  s_StartStopLoadingCursor(false, pFrame);

  return true;
}

Defun1(zoomWidth)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
  UT_ASSERT(pFrame);

  pFrame->raise();

  pFrame->setZoomType( XAP_Frame::z_PAGEWIDTH );

  UT_uint32 newZoom = pView->calculateZoomPercentForPageWidth();

  s_StartStopLoadingCursor(true, pFrame);
  pFrame->setZoomPercentage(newZoom);
  s_StartStopLoadingCursor(false, pFrame);


  return true;
}

Defun1(zoomWhole)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
  UT_ASSERT(pFrame);

  pFrame->raise();

  pFrame->setZoomType( XAP_Frame::z_WHOLEPAGE );

  UT_uint32 newZoom = pView->calculateZoomPercentForWholePage();

  s_StartStopLoadingCursor(true, pFrame);
  pFrame->setZoomPercentage(newZoom);
  s_StartStopLoadingCursor(false, pFrame);

  return true;
}

Defun1(zoomIn)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
  UT_ASSERT(pFrame);

  pFrame->raise();

  UT_uint32 newZoom = pFrame->getZoomPercentage() + 10;
  pFrame->setZoomType( XAP_Frame::z_PERCENT );

  s_StartStopLoadingCursor(true, pFrame);
  pFrame->setZoomPercentage(newZoom);
  s_StartStopLoadingCursor(false, pFrame);

  return true;
}

Defun1(zoomOut)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
  UT_ASSERT(pFrame);

  pFrame->raise();

  UT_uint32 newZoom = pFrame->getZoomPercentage() - 10;
  pFrame->setZoomType( XAP_Frame::z_PERCENT );

  s_StartStopLoadingCursor(true, pFrame);
  pFrame->setZoomPercentage(newZoom);
  s_StartStopLoadingCursor(false, pFrame);

  return true;
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
	if (!pDialog)
		return false;

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
	if (!pDialog)
		return false;

	PD_Document * pDoc = pView->getLayout()->getDocument();
	//
	// Need this for the conversion methods
	//
	fp_PageSize::Predefined orig_def,final_def;
	double orig_wid = -1, orig_ht = -1, final_wid = -1, final_ht = -1;
	UT_Dimension orig_ut = DIM_IN, final_ut = DIM_IN;
	fp_PageSize pSize(pDoc->m_docPageSize.getPredefinedName());
	orig_def = pSize.NameToPredefined(pSize.getPredefinedName());
	//
	// Set first page of the dialog properties.
	//
	if (orig_def == fp_PageSize::psCustom)
	{
		orig_ut = pDoc->m_docPageSize.getDims();
		orig_ht = pDoc->m_docPageSize.Width(orig_ut);
		orig_wid = pDoc->m_docPageSize.Height(orig_ut);
		pSize.Set(orig_ht, orig_wid, orig_ut);
	}
	pDialog->setPageSize(pSize);
	AP_Dialog_PageSetup::Orientation orig_ori,final_ori;
	orig_ori =	AP_Dialog_PageSetup::PORTRAIT;
	if(pDoc->m_docPageSize.isPortrait() == false)
		orig_ori = AP_Dialog_PageSetup::LANDSCAPE;
	pDialog->setPageOrientation(orig_ori);
	UT_Dimension orig_unit,final_unit,orig_margu,final_margu;
	double orig_scale,final_scale;
	orig_unit = pDoc->m_docPageSize.getDims();
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
	orig_margu = DIM_IN;
	if(docMargUnits == DIM_MM)
	{
		dLeftMargin = dLeftMargin * 25.4;
		dRightMargin = dRightMargin * 25.4;
		dTopMargin = dTopMargin * 25.4;
		dBottomMargin = dBottomMargin * 25.4;
		dFooterMargin = dFooterMargin * 25.4;
		dHeaderMargin = dHeaderMargin * 25.4;
		orig_margu = DIM_MM;
	}
	else if(docMargUnits == DIM_CM)
	{
		dLeftMargin = dLeftMargin * 2.54;
		dRightMargin = dRightMargin * 2.54;
		dTopMargin = dTopMargin * 2.54;
		dBottomMargin = dBottomMargin * 2.54;
		dFooterMargin = dFooterMargin * 2.54;
		dHeaderMargin = dHeaderMargin * 2.54;
		orig_margu = DIM_CM;
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
	{
		delete pDialog;
		return true;
	}

	final_def = pSize.NameToPredefined(pDialog->getPageSize().getPredefinedName());
	final_ori = pDialog->getPageOrientation();
	final_unit = pDialog->getPageUnits();
	final_scale = pDialog->getPageScale()/100.0;

	if (final_def == fp_PageSize::psCustom)
	{
		final_ut = pDialog->getPageSize().getDims();
		final_wid = pDialog->getPageSize().Width(final_ut);
		final_ht = pDialog->getPageSize().Height(final_ut);
	}

	if((final_def != orig_def) || (final_ori != orig_ori) || (final_unit != orig_unit) || ((final_scale-orig_scale) > 0.001) || ((final_scale-orig_scale) < -0.001) || (orig_ht != final_ht) || (orig_wid != final_wid) || (orig_ut != final_ut) )
	{
	  //
	  // Set the new Page Stuff
	  //
 		pDoc->m_docPageSize.Set(pSize.PredefinedToName(final_def));
 		pDoc->m_docPageSize.Set(final_unit);
		if (final_def == fp_PageSize::psCustom)
		{
			pDoc->m_docPageSize.Set(final_wid,
									final_ht,
									final_ut);
		}

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
				FV_View * pV =	(FV_View *) f->getCurrentView();
				if(pV->isHdrFtrEdit())
				{
					pV->clearHdrFtrEdit();
					pV->warpInsPtToXY(0,0,false);
				}
				UT_uint32 izoom = f->getZoomPercentage();
				f->setZoomPercentage(izoom);
			}
		}
		else
		{
			FV_View * pV =	(FV_View *) pFrame->getCurrentView();
			if(pV->isHdrFtrEdit())
			{
				pV->clearHdrFtrEdit();
				pV->warpInsPtToXY(0,0,false);
			}
			UT_uint32 izoom = pFrame->getZoomPercentage();

			s_StartStopLoadingCursor(true, pFrame);
			pFrame->setZoomPercentage(izoom);
			s_StartStopLoadingCursor(false, pFrame);
		}
	}

	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	UT_ASSERT(pPrefsScheme);

	pPrefsScheme->setValue((XML_Char*)AP_PREF_KEY_RulerUnits,
						   (XML_Char*)UT_dimensionName(final_unit));

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
	if(final_margu == DIM_CM)
	{
		docMargUnits = DIM_CM;
		dLeftMargin = dLeftMargin / 2.54;
		dRightMargin = dRightMargin / 2.54;
		dTopMargin = dTopMargin / 2.54;
		dBottomMargin = dBottomMargin / 2.54;
		dFooterMargin = dFooterMargin / 2.54;
		dHeaderMargin = dHeaderMargin / 2.54;
	}
	else if (final_margu == DIM_MM)
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
	const XML_Char ** props = (const XML_Char **) UT_calloc(countv, sizeof(XML_Char *));
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
	delete pDialog;
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


static	FV_View_Insert_symbol_listener symbol_Listener;

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
	if (!pDialog)
		return false;

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
	CHECK_FRAME;
	ABIWORD_VIEW;
	return s_doPrint(pView,false,false);
}

Defun1(printDirectly)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return s_doPrint(pView,false,true);
}

Defun1(printTB)
{
	CHECK_FRAME;
// print (intended to be from the tool-bar (where we'd like to
	// suppress the dialog if possible))

	ABIWORD_VIEW;
	return s_doPrint(pView,true,false);
}

Defun1(printPreview)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return s_doPrintPreview(pView);
}

Defun1(pageSetup)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return s_doPageSetupDlg(pView);
}

Defun1(dlgPlugins)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();
	XAP_DialogFactory * pDialogFactory
	  = (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_PluginManager * pDialog
		= (XAP_Dialog_PluginManager *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_PLUGIN_MANAGER));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

	if (pDialog)
	{
		pDialog->runModal (pFrame);
		// simple non-persisten dialogues have to be deleted after use!
		delete pDialog;
		return true;
	}

	return false;
}

Defun1(dlgOptions)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	return s_doOptionsDlg(pView);
}

Defun1(dlgSpellPrefs)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

  return s_doOptionsDlg(pView, 1); // spelling tab
}

/*****************************************************************/
/*****************************************************************/

Defun1(viewStd)
{
	CHECK_FRAME;
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
	CHECK_FRAME;
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

Defun1(viewTable)
{
	CHECK_FRAME;
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

	pScheme->setValueBool((XML_Char*)AP_PREF_KEY_TableBarVisible, pFrameData->m_bShowBar[2]);

	return true;
}

Defun1(viewExtra)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_ASSERT(pFrameData);

	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
	  return false;

	// toggle the ruler bit
	pFrameData->m_bShowBar[3] = ! pFrameData->m_bShowBar[3];

	// actually do the dirty work
	pFrame->toggleBar( 3, pFrameData->m_bShowBar[3] );

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValueBool((XML_Char*)AP_PREF_KEY_ExtraBarVisible, pFrameData->m_bShowBar[3]);

	return true;
}

Defun(viewNormalLayout)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	FV_View * pView = static_cast<FV_View *>(pAV_View);
	if(pView->isHdrFtrEdit())
	{
		pView->clearHdrFtrEdit();
		pView->warpInsPtToXY(0,0,false);
	}

	AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();
	UT_ASSERT(pFrameData);

	pFrameData->m_pViewMode = VIEW_NORMAL;
	pFrame->toggleLeftRuler (false);

	pView->setViewMode (VIEW_NORMAL);

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValue(AP_PREF_KEY_LayoutMode, "2");

	pView->updateScreen(false);
	//pView->notifyListeners(AV_CHG_ALL);

	return true;
}


Defun(viewWebLayout)
{
	CHECK_FRAME;
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

	pView->updateScreen(false);
	//pView->notifyListeners(AV_CHG_ALL);

	return true;
}

Defun(viewPrintLayout)
{
	CHECK_FRAME;
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

	pView->updateScreen(false);
	//pView->notifyListeners(AV_CHG_ALL);

	return true;
}

Defun1(viewStatus)
{
	CHECK_FRAME;
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
	CHECK_FRAME;
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

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);
	pScheme->setValueBool((XML_Char*)AP_PREF_KEY_RulerVisible, pFrameData->m_bShowRuler);

	return true;
}

Defun1(viewFullScreen)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
  AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();

  if(!pFrameData->m_bIsFullScreen) // we're hiding stuff
	{
	  pFrameData->m_bIsFullScreen = true;
	  pFrame->toggleBar(0, false);
	  pFrame->toggleBar(1, false);
	  pFrame->toggleBar(2, false);
	  pFrame->toggleBar(3, false);
	  pFrame->toggleStatusBar(false);
	  pFrame->toggleRuler(false);
	  pFrame->setFullScreen(true);
	}
  else // we're (possibly) unhiding stuff
	{
	  pFrame->toggleBar(0, pFrameData->m_bShowBar[0]);
	  pFrame->toggleBar(1, pFrameData->m_bShowBar[1]);
	  pFrame->toggleBar(2, pFrameData->m_bShowBar[2]);
	  pFrame->toggleBar(3, pFrameData->m_bShowBar[3]);
	  pFrame->toggleStatusBar(pFrameData->m_bShowStatusBar);
	  pFrame->toggleRuler(pFrameData->m_bShowRuler);
	  pFrameData->m_bIsFullScreen = false;
	  pFrame->setFullScreen(false);
	}

  return true;
}

Defun1(viewPara)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();
	UT_ASSERT(pFrameData);

	pFrameData->m_bShowPara = !pFrameData->m_bShowPara;

	ABIWORD_VIEW;
	pView->setShowPara(pFrameData->m_bShowPara);

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_ASSERT(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValueBool(AP_PREF_KEY_ParaVisible, pFrameData->m_bShowPara);
	pView->notifyListeners(AV_CHG_ALL);

	return true;
}

Defun1(viewHeadFoot)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	// TODO: synch this implementation with ap_GetState_View
	s_TellNotImplemented(pFrame, "View Headers and Footers", __LINE__);
	return true;
}

Defun(zoom)
{
	CHECK_FRAME;
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
	else if(strcmp(p_zoom, pSS->getValue(XAP_STRING_ID_TB_Zoom_Percent)) == 0)
	  {
	    // invoke the zoom dialog instead for some custom value
	    return EX(dlgZoom);
	  }
	else
	{
	  // we've gotten back a number - turn it into a zoom percentage
		pFrame->setZoomType(XAP_Frame::z_PERCENT);
		iZoom = atoi(p_zoom);
	}

	UT_ASSERT(iZoom > 0);

	s_StartStopLoadingCursor(true, pFrame);
	pFrame->setZoomPercentage(iZoom);
	s_StartStopLoadingCursor(false, pFrame);
//
// Make damn sure the cursor is ON!!
//
	FV_View * pAbiView = (FV_View *) pFrame->getCurrentView();
	pAbiView->focusChange(AV_FOCUS_HERE);

	return true;
}

Defun1(dlgZoom)
{
	CHECK_FRAME;
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
	if (!pDialog)
		return false;

	pDialog->runModal(pFrame);

	if (pDialog->getAnswer() == AP_Dialog_Insert_DateTime::a_OK)
	{
		time_t	tim = time(NULL);
		struct tm *pTime = localtime(&tim);
		UT_UCSChar *CurrentDateTime = NULL;
		char szCurrentDateTime[CURRENT_DATE_TIME_SIZE];

		strftime(szCurrentDateTime,CURRENT_DATE_TIME_SIZE,pDialog->GetDateTimeFormat(),pTime);
		UT_UCS4_cloneString_char(&CurrentDateTime,szCurrentDateTime);
		pView->cmdCharInsert(CurrentDateTime,UT_UCS4_strlen(CurrentDateTime), true);
		FREEP(CurrentDateTime);
	}

	pDialogFactory->releaseDialog(pDialog);

	return true;
}

Defun1(insDateTime)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return s_doInsertDateTime(pView);
}

/*****************************************************************/
/*****************************************************************/

Defun1(insBreak)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	if(pView->isInTable())
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
		pFrame->showMessageBox(AP_STRING_ID_MSG_NoBreakInsideTable,
							   XAP_Dialog_MessageBox::b_O,
							   XAP_Dialog_MessageBox::a_OK);
		return true;
	}
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
	if (!pDialog)
		return false;

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
	pView->processPageNumber(pDialog->isFooter() ?
								  FL_HDRFTR_FOOTER : FL_HDRFTR_HEADER,
							 atts);
	pDialogFactory->releaseDialog(pDialog);
	return true;
}

Defun1(insPageNo)
{
	CHECK_FRAME;
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
	if (!pDialog)
		return false;

	pDialog->runModal(pFrame);

	if (pDialog->getAnswer() == AP_Dialog_Field::a_OK)
	{
		// TODO - Insert field correctly
		const XML_Char * pParam = pDialog->getParameter();
		const XML_Char * pAttr[3];
		const XML_Char param_name[] = "param";
		pAttr[0] = (const XML_Char *)&param_name;
		pAttr[1] = pParam;
		pAttr[2] = 0;

		if(pParam)
			pView->cmdInsertField(pDialog->GetFieldFormat(),(const XML_Char **)&pAttr[0]);
		else
			pView->cmdInsertField(pDialog->GetFieldFormat());
	}

	pDialogFactory->releaseDialog(pDialog);

	return true;
}

Defun1(insField)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return s_doField(pView);
}

Defun1(insFile)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pAV_View->getParentData());
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	
	IEFileType fType = IEFT_Unknown;
	char *pathName = NULL;
	
	// we'll share the same graphics context, which won't matter because
	// we only use it to get font metrics and stuff and not actually draw
	GR_Graphics *pGraphics = pView->getGraphics();
	
	if (s_AskForPathname (pFrame, false, XAP_DIALOG_ID_INSERT_FILE,
			      NULL, &pathName, &fType))
	  {
	    UT_DEBUGMSG(("DOM: insertFile %s\n", pathName));
	    
	    PD_Document * newDoc = new PD_Document(pApp);
	    UT_Error err = newDoc->readFromFile(pathName, IEFT_Unknown);
	    
	    if ( err != UT_OK )
	      {
		UNREFP(newDoc);
		s_TellOpenFailed(pFrame, pathName, err);
		return false;
	      }
	    
	    // create a new layout and view object for the doc
	    FL_DocLayout *pDocLayout = new FL_DocLayout(newDoc,pGraphics);
	    FV_View copyView(pApp,0,pDocLayout);

	    pDocLayout->setView (&copyView);
	    pDocLayout->fillLayouts();
	    
	    copyView.cmdSelect(0, 0, FV_DOCPOS_BOD, FV_DOCPOS_EOD); // select all the contents of the new doc
	    copyView.cmdCopy(); // copy the contents of the new document
	    pView->cmdPaste ( true ); // paste the contents into the existing document honoring the formatting
	    
	    DELETEP(pDocLayout);
	    UNREFP(newDoc);
	    return true;
	  }
	
	return false;
}

Defun1(insSymbol)
{
	CHECK_FRAME;

	ABIWORD_VIEW;
	XAP_Dialog_Id id = XAP_DIALOG_ID_INSERT_SYMBOL;

	return s_InsertSymbolDlg(pView,id);
}

Defun1(insFootnote)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	return pView->insertFootnote();
}


Defun1(insEndnote)
{
	CHECK_FRAME;
	//ABIWORD_VIEW;

	return true;
}


/*****************************************************************/
/*****************************************************************/

Defun1(dlgParagraph)
{
	CHECK_FRAME;
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
	if (!pDialog)
		return false;

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
	CHECK_FRAME;
//
  // Dialog for Bullets and Lists
  //
#if defined(__QNXTO__) || defined(__BEOS__) || defined(TARGET_OS_MAC)
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
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	s_TellNotImplemented(pFrame, "Border and shading dialog", __LINE__);
	return true;
}

Defun(dlgFmtImage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_Image * pDialog
		= (XAP_Dialog_Image *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_IMAGE));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

	double width = 0., height = 0.;
	double max_width = 0., max_height = 0.;
	UT_sint32 x1,x2,y1,y2,iHeight,iWidth;
	bool bEOL = false;
	bool bDir = false;

	// set units in the dialog.
	const char * pszRulerUnits = NULL;
	UT_Dimension dim = DIM_IN;
	if (XAP_App::getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits, &pszRulerUnits))
	{
		dim = UT_determineDimension((char *)pszRulerUnits);
	};
	pDialog->setPreferedUnits(dim);

	const fp_PageSize & page = pView->getPageSize ();

	// an approximate... TODO: make me more accurate
	max_width  = page.Width (DIM_IN) * 72.0;
	max_height = page.Height (DIM_IN) * 72.0;

	pDialog->setMaxWidth (max_width);
	pDialog->setMaxHeight (max_height);
	UT_DEBUGMSG(("formatting  image: %d\n", pCallData->m_xPos));
	PT_DocPosition pos = pView->getDocPositionFromLastXY();

	fl_BlockLayout * pBlock = pView->getBlockAtPosition(pos);
	fp_Run *  pRun = NULL;
	if(pBlock)
	{
		pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,iHeight,bDir);
		while(pRun && pRun->getType() != FPRUN_IMAGE)
		{
			pRun = pRun->getNext();
		}
		if(pRun && pRun->getType() == FPRUN_IMAGE)
		{
			UT_DEBUGMSG(("SEVIOR: Image run on pos \n"));
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
	}
    pView->cmdSelect(pos,pos+1);
	const XML_Char ** props_in = NULL;
	if (pView->getCharFormat(&props_in))
	{
	  // stuff properties into the dialog.

	  const XML_Char* szWidth = UT_getAttribute("width", props_in);
	  const XML_Char* szHeight = UT_getAttribute("height", props_in);

	  // 72.0 is pixels/inch
	  bool bDoWidth = false;
	  if(szWidth)
	  {
		  double dum = UT_convertToInches(szWidth);
		  if(dum > 0.0001)
		  {
			  bDoWidth = true;
		  }
	  }
	  bool bDoHeight = false;
	  if(szHeight)
	  {
		  double dum = UT_convertToInches(szHeight);
		  if(dum > 0.0001)
		  {
			  bDoHeight = true;
		  }
	  }

	  if(bDoWidth)
	  {
		  double dum = UT_convertToInches(szWidth);
		  pDialog->setWidth( UT_convertInchesToDimensionString(dim,dum));
	  }
	  else
	  {
		  iWidth = 0;
		  UT_ASSERT(pRun);
		  if(pRun && pRun->getType() == FPRUN_IMAGE)
		  {
			  iWidth = pRun->getWidth();
		  }
		  else
		  {
			  UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			  FREEP(props_in);
			  return false;
		  }
		  pDialog->setWidth(iWidth);
	  }
	  if(bDoHeight)
	  {
		  double dum = UT_convertToInches(szHeight);
		  pDialog->setHeight( UT_convertInchesToDimensionString(dim,dum));
	  }
	  else
	  {
		  iHeight = 0;
		  UT_ASSERT(pRun);
		  if(pRun && pRun->getType() == FPRUN_IMAGE)
		  {
			  iHeight = pRun->getHeight();
		  }
		  else
		  {
			  UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			  FREEP(props_in);
			  return false;
		  }
		  pDialog->setHeight(iHeight);
	  }
	  FREEP(props_in);

	  pDialog->runModal(pFrame);

	  XAP_Dialog_Image::tAnswer ans = pDialog->getAnswer();
	  bool bOK = (ans == XAP_Dialog_Image::a_OK);

	  if (bOK)
	  {
		  // now get them back in inches
		  width  = pDialog->getWidth () / 72.0;
		  height = pDialog->getHeight () / 72.0;
		  char widthBuf[32];
		  char heightBuf[32];

		  // TODO: set format
		  const XML_Char * properties[] = {"width", NULL, "height", NULL, 0};
		  char * old_locale = setlocale(LC_NUMERIC, "C");
		  sprintf(widthBuf, "%fin", width);
		  sprintf(heightBuf, "%fin", height);
		  setlocale(LC_NUMERIC, old_locale);

		  UT_DEBUGMSG(("DOM: nw:%s nh:%s\n", widthBuf, heightBuf));

		  properties[1] = widthBuf;
		  properties[3] = heightBuf;
		  pView->setCharFormat(properties);
		  pView->updateScreen();
		}

	  pDialogFactory->releaseDialog(pDialog);
	  return true;
	}
	else
	{
		return false;
	}
}

Defun(dlgColumns)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Columns * pDialog
		= (AP_Dialog_Columns *)(pDialogFactory->requestDialog(AP_DIALOG_ID_COLUMNS));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

	UT_uint32 iColumns = 1;
	bool bLineBetween = false;
	bool bSpaceAfter = false;
	bool bMaxHeight = false;

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

	if ( iColumns > 1 )
	  {
		EX(viewPrintLayout);
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

	UT_uint32 iOrder = 0;
	if (props_in && props_in[0])
		sz = UT_getAttribute("dom-dir", props_in);
	if (sz)
		iOrder = strcmp(sz, "ltr") ? 1 : 0;

	pDialog->setColumnOrder(iOrder);

	if(props_in && props_in[0])
	{
		sz = UT_getAttribute("section-space-after",props_in);
		if(sz && *sz)
		{
			bSpaceAfter = true;
		}
		sz = UT_getAttribute("section-max-column-height",props_in);
		if(sz && *sz)
		{
			bMaxHeight = true;
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
		bMaxHeight = bMaxHeight || pDialog->isMaxHeightChanged();
		bSpaceAfter = bSpaceAfter || pDialog->isSpaceAfterChanged();

		char buf3[4];
		char buf4[6];
		if(pDialog->getColumnOrder())
		{
			strcpy(buf3, "rtl");
			strcpy(buf4, "right");
		}
		else
		{
			strcpy(buf3, "ltr");
			strcpy(buf4, "left");
		}
#ifndef __MRC__ 		/* column-order */
		const XML_Char * properties[] = { "columns", buf, "column-line", buf2, "dom-dir", buf3, "text-align", buf4, 0};
#else
		const XML_Char * properties[] = { "columns", NULL, "column-line", NULL, "dom-dir", NULL, "text-align", NULL, 0};
		properties [1] = buf;
		properties [3] = buf2;
		properties [5] = buf3;
		properties [7] = buf4;
#endif

		UT_sint32 num_in_props = sizeof(properties)/sizeof(XML_Char *);
		UT_sint32 num_out_props = num_in_props;
		if(bMaxHeight)
		{
			num_out_props += 2;
		}
		if(bSpaceAfter)
		{
			num_out_props += 2;
		}
		const XML_Char ** props = (const XML_Char **) UT_calloc(num_out_props,sizeof(XML_Char *));
		UT_sint32 i = 0;
		for(i = 0; i < num_in_props-1; i++)
		{
			props[i] = properties[i];
		}
		if(bSpaceAfter)
		{
			props[i++] = "section-space-after";
			props[i++] = pDialog->getSpaceAfterString();
		}
		if(bMaxHeight)
		{
			props[i++] = "section-max-column-height";
			props[i++] = pDialog->getHeightString();
		}
		props[i] = NULL;
		pView->setSectionFormat(props);
		FREEP(props);
	}

	FREEP(props_in);
	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

Defun(style)
{
	CHECK_FRAME;
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
	if (!pDialog)
		return false;
	if(pView->isHdrFtrEdit())
	{
		pView->clearHdrFtrEdit();
		pView->warpInsPtToXY(0,0,false);
	}

	pDialog->runModal(pFrame);

//	AP_Dialog_Styles::tAnswer ans = pDialog->getAnswer();
	bool bOK = true;
//
// update the combo box with the new styles.
//
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	//
	// Get all clones of this frame and set styles combo box
	//
	UT_Vector vClones;
	if(pFrame->getViewNumber() > 0)
	{
		pApp->getClones(&vClones,pFrame);
		for (UT_uint32 i = 0; i < vClones.getItemCount(); i++)
		{
			XAP_Frame * f = (XAP_Frame *) vClones.getNthItem(i);
			f->repopulateCombos();
		}
	}
	else
	{
		pFrame->repopulateCombos();
	}
//
// Now update all views on the document. Do this always to be safe.
//
	{
		PD_Document * pDoc = pView->getLayout()->getDocument();
		pDoc->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
	}

	pDialogFactory->releaseDialog(pDialog);
	return bOK;
}

Defun1(formatFootnotes)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_FormatFootnotes * pDialog
		= (AP_Dialog_FormatFootnotes *)(pDialogFactory->requestDialog(AP_DIALOG_ID_FORMAT_FOOTNOTES));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;
	pDialog->runModal(pFrame);
#if 0
	AP_Dialog_FormatFootnotes::tAnswer ans = pDialog->getAnswer();
	if(ans == b_OK)
	{
//
// update all the layouts.
//
		XAP_App * pApp = pFrame->getApp();
		UT_ASSERT(pApp);
	//
	// Get all clones of this frame and reset the layouts
	//
		UT_Vector vClones;
		if(pFrame->getViewNumber() > 0)
		{
			pApp->getClones(&vClones,pFrame);
			for (UT_uint32 i = 0; i < vClones.getItemCount(); i++)
			{
				XAP_Frame * f = (XAP_Frame *) vClones.getNthItem(i);
				FV_View * pV = f->getCurrentView();
				pV->getLayout()->updatePropsNoRebuild();
			}
		}
		else
		{
			FV_View * pV = pFrame->getCurrentView();
			pV->getLayout()->updatePropsNoRebuild();
			pFrame->repopulateCombos();
		}
	}
#endif
	return true;
}
Defun1(dlgStyle)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);
	ABIWORD_VIEW;

	return s_doStylesDlg(pView);
}

Defun1(dlgTabs)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	return s_doTabDlg(pView);
}

Defun0(noop)
{
	CHECK_FRAME;
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
	if (!pDialog)
		return false;

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
	CHECK_FRAME;
	ABIWORD_VIEW;

	return s_doWordCountDlg(pView);
}

/****************************************************************/
/****************************************************************/

static bool s_doInsertTableDlg(FV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_InsertTable * pDialog
		= (AP_Dialog_InsertTable *)(pDialogFactory->requestDialog(AP_DIALOG_ID_INSERT_TABLE));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

	pDialog->runModal(pFrame);

	AP_Dialog_InsertTable::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_InsertTable::a_OK);
//
// Should be able to put a table in Headers/footers eventually
//
//	if(pView->isHdrFtrEdit())
//		return false;
//
	if (bOK)
	{
		if (pDialog->getColumnType() == AP_Dialog_InsertTable::b_FIXEDSIZE)
		{
			UT_String tmp;
			UT_String propBuffer;
			for (UT_uint32 i = 0; i<pDialog->getNumCols(); i++)
			{
				UT_String_sprintf(tmp, "%fin/", pDialog->getColumnWidth());
				propBuffer += tmp;
			}
			const XML_Char * propsArray[3];
	        	propsArray[0] = "table-column-props";
			propsArray[1] = propBuffer.c_str();
			propsArray[2] = NULL;
			pView->cmdInsertTable(pDialog->getNumRows(),pDialog->getNumCols(),propsArray);
		} else
		{
			pView->cmdInsertTable(pDialog->getNumRows(),pDialog->getNumCols(),NULL);
		}
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

Defun1(insertTable)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return s_doInsertTableDlg(pView);
}


/****************************************************************/
/****************************************************************/
Defun1(toggleHidden)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return _toggleSpan(pView, "display", "none", "");
}

Defun1(toggleBold)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return _toggleSpan(pView, "font-weight", "bold", "normal");
}

Defun1(toggleItalic)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return _toggleSpan(pView, "font-style", "italic", "normal");
}

Defun1(toggleUline)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-decoration", "underline", "none", true);
}
Defun1(toggleOline)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-decoration", "overline", "none", true);
}

Defun1(toggleStrike)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-decoration", "line-through", "none", true);
}


Defun1(toggleTopline)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-decoration", "topline", "none", true);
}


Defun1(toggleBottomline)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-decoration", "bottomline", "none", true);
}

// non-static so that ap_Toolbar_Functions.cpp can use it
void s_getPageMargins(FV_View * pView,
					  double &margin_left,
					  double &margin_right,
					  double &page_margin_left,
					  double &page_margin_right)
{
  // get current char properties from pView
  const XML_Char * prop = NULL;
  const XML_Char ** props_in = NULL;
  const XML_Char * sz = NULL;

	{
		pView->getBlockFormat(&props_in);
		prop = "margin-left";
		sz = UT_getAttribute(prop, props_in);
		margin_left = UT_convertToInches(sz);
		FREEP(props_in);
	}

	{
		pView->getBlockFormat(&props_in);
		prop = "margin-right";
		sz = UT_getAttribute(prop, props_in);
		margin_right = UT_convertToInches(sz);
		FREEP(props_in);
	}

	{
		prop = "page-margin-left";
		pView->getSectionFormat(&props_in);
		sz = UT_getAttribute(prop, props_in);
		page_margin_left = UT_convertToInches(sz);
		FREEP(props_in);
	}

	{
		prop = "page-margin-right";
		pView->getSectionFormat(&props_in);
		sz = UT_getAttribute(prop, props_in);
		page_margin_right = UT_convertToInches(sz);
		FREEP(props_in);
	}
}

// MSWord defines this to 1/2 an inch, so we do too
#define TOGGLE_INDENT_AMT 0.5

Defun1(toggleIndent)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
  bool doLists = true;
  double page_size = pView->getPageSize().Width (DIM_IN);

  double margin_left = 0., margin_right = 0., allowed = 0.,
	  page_margin_left = 0., page_margin_right = 0.;

  s_getPageMargins (pView, margin_left, margin_right,
					page_margin_left, page_margin_right);

  allowed = page_size - page_margin_left - page_margin_right;
  if (margin_left >= allowed)
	  return true;
  
  if(!pView->getCurrentBlock()->isListItem() || !pView->isSelectionEmpty() )
  {
	  doLists = false;
  }
  return  pView->setBlockIndents(doLists, (double) TOGGLE_INDENT_AMT ,page_size);
}

Defun1(toggleUnIndent)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
  bool ret;
  double page_size = pView->getPageSize().Width (DIM_IN);
  bool doLists = true;

  double margin_left = 0., margin_right = 0., allowed = 0.,
	  page_margin_left = 0., page_margin_right = 0.;

  s_getPageMargins (pView, margin_left, margin_right,
					page_margin_left, page_margin_right);

  FriBidiCharType iBlockDir = pView->getCurrentBlock()->getDominantDirection();
  allowed = iBlockDir == FRIBIDI_TYPE_LTR ? margin_left : margin_right;
  if ( allowed <= 0. )
	  return true ;

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
	CHECK_FRAME;
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-position", "superscript", "normal");
}

Defun1(toggleSub)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-position", "subscript", "normal");
}

Defun1(toggleDirOverrideLTR)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return _toggleSpan(pView, "dir-override", "ltr", "");
}

Defun1(toggleDirOverrideRTL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return _toggleSpan(pView, "dir-override", "rtl", "");
}

Defun1(toggleDomDirection)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * properties[] = { "dom-dir", NULL, "text-align", NULL, 0};
	const XML_Char drtl[]	= "rtl";
	const XML_Char dltr[]	= "ltr";
	const XML_Char aright[] = "right";
	const XML_Char aleft[]	= "left";
	XML_Char cur_alignment[10];

	fl_BlockLayout * pBl = pView->getCurrentBlock();

	strcpy(cur_alignment,pBl->getProperty("text-align"));
	properties[3] = (XML_Char *) &cur_alignment;


	if(pBl->getDominantDirection()== FRIBIDI_TYPE_RTL)
	{
		properties[1] = (XML_Char *) &dltr;
		/*
		//the last run in the block is the FmtMark, and we need
		//to reset its direction
		static_cast<fp_Line *>(static_cast<fl_BlockLayout *>(pBl)->getLastContainer())->getLastRun()->setDirection(FRIBIDI_TYPE_LTR);
		*/
	}
	else
	{
		properties[1] = (XML_Char *) &drtl;
		/*
		static_cast<fp_Line *>(static_cast<fl_BlockLayout *>(pBl)->getLastContainer())->getLastRun()->setDirection(FRIBIDI_TYPE_RTL);
		*/
	}

	// if the paragraph is was aligned either left or right, then
	// toggle the alignment as well; if it was anything else
	// i.e., justfied or centered, then leave it
	if(!strcmp(properties[3],aleft))
	{
		properties[3] = (XML_Char *) &aright;
	}
	else if(!strcmp(properties[3],aright))
	{
		properties[3] = (XML_Char *) &aleft;

	}

	pView->setBlockFormat(properties);

	return true;
}


Defun1(doBullets)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->processSelectedBlocks(BULLETED_LIST);
	return true;
}

Defun1(doNumbers)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->processSelectedBlocks(NUMBERED_LIST);
	return true;
}

Defun(colorForeTB)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

  const XML_Char * properties[] = { "color", NULL, 0};
  properties[1] = (const XML_Char *) pCallData->m_pData;
  pView->setCharFormat(properties);

  return true;
}

Defun(colorBackTB)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	const XML_Char * properties[] = { "bgcolor", NULL, 0};
	properties[1] = (const XML_Char *) pCallData->m_pData;
	pView->setCharFormat(properties);

	return true;
}

/*! removes the "props" attribute, i.e., all non-style based formatting
 */
Defun1(togglePlain)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	const XML_Char p[] = "props";
	const XML_Char v[] = "";
	const XML_Char * props_out[3] = {p,v,NULL};
	pView->setCharFormat(NULL,props_out);
		
	return true;
}

Defun1(alignLeft)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * properties[] = { "text-align", "left", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(alignCenter)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * properties[] = { "text-align", "center", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(alignRight)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * properties[] = { "text-align", "right", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(alignJustify)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * properties[] = { "text-align", "justify", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(setStyleHeading1)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * style = "Heading 1";
	pView->setStyle(style);
	return true;
}


Defun1(setStyleHeading2)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * style = "Heading 2";
	pView->setStyle(style);
	return true;
}

Defun1(setStyleHeading3)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * style = "Heading 3";
	pView->setStyle(style);
	return true;
}

Defun1(sectColumns1)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	if(pView->isHdrFtrEdit())
		return false;

	const XML_Char * properties[] = { "columns", "1", 0};
	pView->setSectionFormat(properties);
	return true;
}

Defun1(sectColumns2)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	if(pView->isHdrFtrEdit())
		return false;

	const XML_Char * properties[] = { "columns", "2", 0};
	pView->setSectionFormat(properties);
	return true;
}

Defun(sectColumns3)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	if(pView->isHdrFtrEdit())
		return false;
	const XML_Char * properties[] = { "columns", "3", 0};
	pView->setSectionFormat(properties);
	EX(viewPrintLayout);
	return true;
}

Defun1(paraBefore0)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * properties[] = { "margin-top", "0pt", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(paraBefore12)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * properties[] = { "margin-top", "12pt", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(singleSpace)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * properties[] = { "line-height", "1.0", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(middleSpace)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * properties[] = { "line-height", "1.5", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(doubleSpace)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	const XML_Char * properties[] = { "line-height", "2.0", 0};
	pView->setBlockFormat(properties);
	return true;
}

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
Defun1(Test_Dump)
{
CHECK_FRAME;
//	ABIWORD_VIEW;
//	pView->Test_Dump();
	return true;
}

Defun1(Test_Ftr)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->insertPageNum(NULL, FL_HDRFTR_FOOTER);
	return true;
}
#endif

Defun1(setEditVI)
{
	CHECK_FRAME;
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
	CHECK_FRAME;
// enter "VI Input Mode" (only valid when VI keys are loaded)

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_ASSERT(pFrame);

	bool bResult = (pFrame->setInputMode("viInput") != 0);
	return bResult;
}

Defun1(cycleInputMode)
{
	CHECK_FRAME;
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

	// POLICY: make this the default for new frames, too
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValue((XML_Char*)AP_PREF_KEY_KeyBindings,
					  (XML_Char*)szNextInputMode);

	return bResult;
}

Defun1(toggleInsertMode)
{
	CHECK_FRAME;
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

	// POLICY: make this the default for new frames, too
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_ASSERT(pScheme);

	pScheme->setValueBool((XML_Char*)AP_PREF_KEY_InsertMode, pFrameData->m_bInsertMode);

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
	CHECK_FRAME;
// insert after the end of the current line
	return ( EX(warpInsPtEOL) && EX(setInputVI) );
}

Defun(viCmd_C)
{
	CHECK_FRAME;
// Select to the end of the line for modification
	return ( EX(extSelEOL) && EX(setInputVI) );
}

Defun(viCmd_I)
{
	CHECK_FRAME;
// insert before the beginning of current line
	return ( EX(warpInsPtBOL) && EX(setInputVI) );
}

Defun(viCmd_J)
{
	CHECK_FRAME;
// Join current and next line.
	return ( EX(warpInsPtEOL) && EX(delRight) && EX(insertSpace) );
}

Defun(viCmd_O)
{
	CHECK_FRAME;
// insert new line before current line, go into input mode
	return ( EX(warpInsPtBOL) && EX(insertLineBreak) && EX(warpInsPtLeft) \
		&& EX(setInputVI) );
}

Defun(viCmd_P)
{
	CHECK_FRAME;
// paste text before cursor
	return ( EX(warpInsPtLeft) && EX(paste) );
}

Defun(viCmd_a)
{
	CHECK_FRAME;
// insert after the current position
	return ( EX(warpInsPtRight) && EX(setInputVI) );
}

Defun(viCmd_o)
{
	CHECK_FRAME;
// insert new line after current line, go into input mode
	return ( EX(warpInsPtEOL) && EX(insertLineBreak) && EX(setInputVI) );
}

/* c$ */
Defun(viCmd_c24)
{
	CHECK_FRAME;
//change to end of current line
	return ( EX(delEOL) && EX(setInputVI) );
}

/* c( */
Defun(viCmd_c28)
{
	CHECK_FRAME;
//change to start of current sentence
	return ( EX(delBOS) && EX(setInputVI) );
}

/* c) */
Defun(viCmd_c29)
{
	CHECK_FRAME;
//change to end of current sentence
	return ( EX(delEOS) && EX(setInputVI) );
}

/* c[ */
Defun(viCmd_c5b)
{
	CHECK_FRAME;
//change to beginning of current block
	return ( EX(delBOB) && EX(setInputVI) );
}

/* c] */
Defun(viCmd_c5d)
{
	CHECK_FRAME;
//change to end of current block
	return ( EX(delEOB) && EX(setInputVI) );
}

/* c^ */
Defun(viCmd_c5e)
{
	CHECK_FRAME;
//change to beginning of current line
	return ( EX(delBOL) && EX(setInputVI) );
}

Defun(viCmd_cb)
{
	CHECK_FRAME;
//change to beginning of current word
	return ( EX(delBOW) && EX(setInputVI) );
}

Defun(viCmd_cw)
{
	CHECK_FRAME;
// delete to the end of current word, start input mode
	return ( EX(delEOW) && EX(setInputVI) );
}

/* d$ */
Defun(viCmd_d24)
{
	CHECK_FRAME;
//delete to end of line
	return ( EX(extSelEOL) && EX(cut) );
}

/* d( */
Defun(viCmd_d28)
{
	CHECK_FRAME;
//delete to start of sentence
	return ( EX(extSelBOS) && EX(cut) );
}

/* d) */
Defun(viCmd_d29)
{
	CHECK_FRAME;
//delete to end of sentence
	return ( EX(extSelEOS) && EX(cut) );
}

/* d[ */
Defun(viCmd_d5b)
{
	CHECK_FRAME;
//delete to beginning of block
	return ( EX(extSelBOB) && EX(cut) );
}

/* d] */
Defun(viCmd_d5d)
{
	CHECK_FRAME;
//delete to end of block
	return ( EX(extSelEOB) && EX(cut) );
}

/* d^ */
Defun(viCmd_d5e)
{
	CHECK_FRAME;
//delete to beginning of line
	return ( EX(extSelBOL) && EX(cut) );
}

Defun(viCmd_db)
{
	CHECK_FRAME;
//delete to beginning of word
	return ( EX(extSelBOW) && EX(cut) );
}

Defun(viCmd_dd)
{
	CHECK_FRAME;
// delete the current line
	return ( EX(warpInsPtBOL) && EX(extSelEOL) && EX(extSelRight) && EX(cut) );
}

Defun(viCmd_dw)
{
	CHECK_FRAME;
//delete to end of word
	return ( EX(extSelEOW) && EX(cut) );
}

/* y$ */
Defun(viCmd_y24)
{
	CHECK_FRAME;
//copy to end of current line
	return ( EX(extSelEOL) && EX(copy) );
}

/* y( */
Defun(viCmd_y28)
{
	CHECK_FRAME;
//copy to beginning of current sentence
	return ( EX(extSelBOS) && EX(copy) );
}

/* y) */
Defun(viCmd_y29)
{
	CHECK_FRAME;
//copy to end of current sentence
	return ( EX(extSelEOS) && EX(copy) );
}

/* y[ */
Defun(viCmd_y5b)
{
	CHECK_FRAME;
//copy to beginning of current block
	return ( EX(extSelBOB) && EX(copy) );
}

/* y] */
Defun(viCmd_y5d)
{
	CHECK_FRAME;
//copy to end of current block
	return ( EX(extSelEOB) && EX(copy) );
}

/* y^ */
Defun(viCmd_y5e)
{
	CHECK_FRAME;
//copy to beginning of current line
	return ( EX(extSelBOL) && EX(copy) );
}

Defun(viCmd_yb)
{
	CHECK_FRAME;
//copy to beginning of current word
	return ( EX(extSelBOW) && EX(copy) );
}

Defun(viCmd_yw)
{
	CHECK_FRAME;
//copy to end of current word
	return ( EX(extSelEOW) && EX(copy) );
}

Defun(viCmd_yy)
{
	CHECK_FRAME;
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
	UT_UCS4_strcpy_char (ucstext, text);

	pView->cmdCharInsert(ucstext, len);

	delete [] ucstext;

	return true;
}

Defun(insAutotext_attn_1)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_ATTN_1);
}

Defun(insAutotext_attn_2)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_ATTN_2);
}

Defun(insAutotext_closing_1)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_CLOSING_1);
}

Defun(insAutotext_closing_2)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_CLOSING_2);
}

Defun(insAutotext_closing_3)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_CLOSING_3);
}

Defun(insAutotext_closing_4)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_CLOSING_4);
}

Defun(insAutotext_closing_5)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_CLOSING_5);
}

Defun(insAutotext_closing_6)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_CLOSING_6);
}

Defun(insAutotext_closing_7)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_CLOSING_7);
}

Defun(insAutotext_closing_8)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_CLOSING_8);
}

Defun(insAutotext_closing_9)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_CLOSING_9);
}

Defun(insAutotext_closing_10)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_CLOSING_10);
}

Defun(insAutotext_closing_11)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_CLOSING_11);
}

Defun(insAutotext_closing_12)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_CLOSING_12);
}

Defun(insAutotext_email_1)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_EMAIL_1);
}

Defun(insAutotext_email_2)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_EMAIL_2);
}

Defun(insAutotext_email_3)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_EMAIL_3);
}

Defun(insAutotext_email_4)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_EMAIL_4);
}

Defun(insAutotext_email_5)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_EMAIL_5);
}

Defun(insAutotext_email_6)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_EMAIL_6);
}

Defun(insAutotext_mail_1)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_MAIL_1);
}


Defun(insAutotext_mail_2)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_MAIL_2);
}

Defun(insAutotext_mail_3)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_MAIL_3);
}

Defun(insAutotext_mail_4)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_MAIL_4);
}

Defun(insAutotext_mail_5)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_MAIL_5);
}

Defun(insAutotext_mail_6)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_MAIL_6);
}

Defun(insAutotext_mail_7)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_MAIL_7);
}

Defun(insAutotext_mail_8)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_MAIL_8);
}

Defun(insAutotext_reference_1)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_REFERENCE_1);
}

Defun(insAutotext_reference_2)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_REFERENCE_2);
}

Defun(insAutotext_reference_3)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_REFERENCE_3);
}

Defun(insAutotext_salutation_1)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_SALUTATION_1);
}

Defun(insAutotext_salutation_2)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_SALUTATION_2);
}

Defun(insAutotext_salutation_3)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_SALUTATION_3);
}

Defun(insAutotext_salutation_4)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_SALUTATION_4);
}

Defun(insAutotext_subject_1)
{
	CHECK_FRAME;
	return _insAutotext(static_cast<FV_View *>(pAV_View),
			  AP_STRING_ID_AUTOTEXT_SUBJECT_1);
}

static bool s_AskForScriptName(XAP_Frame * pFrame,
							   UT_String& stPathname,
							   UT_ScriptIdType * ieft)
{
	UT_ASSERT(ieft);

	stPathname.clear();

	pFrame->raise();

	XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_OPEN;

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

	UT_ScriptLibrary& instance = UT_ScriptLibrary::instance ();

	UT_uint32 filterCount = instance.getNumScripts ();

	const char ** szDescList = (const char **) UT_calloc(filterCount + 1,
							  sizeof(char *));
	const char ** szSuffixList = (const char **) UT_calloc(filterCount + 1,
								sizeof(char *));
	UT_ScriptIdType * nTypeList = (UT_ScriptIdType *) UT_calloc(filterCount + 1,
								 sizeof(UT_ScriptIdType));
	UT_uint32 k = 0;

	while (instance.enumerateDlgLabels(k, &szDescList[k],
					   &szSuffixList[k], &nTypeList[k]))
		k++;

	pDialog->setFileTypeList(szDescList, szSuffixList,
							 (const UT_sint32 *) nTypeList);

	UT_ScriptIdType dflFileType = -1;
	pDialog->setDefaultFileType(dflFileType);

	pDialog->runModal(pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
#ifdef WIN32
		const char* szResultPathname = pDialog->getPathname();

		if (szResultPathname && *szResultPathname)
		{
			stPathname = "\"";
			stPathname += szResultPathname;
			stPathname += "\"";
		}
#else
	const char* szResultPathname = pDialog->getPathname();

		if (szResultPathname && *szResultPathname)
		{
			stPathname += szResultPathname;
		}
#endif

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
				*ieft = -1;
				break;
			default:
				// it returned a type we don't know how to handle
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		else
			*ieft = (UT_ScriptIdType) pDialog->getFileType();
	}

	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

class MailMerge_Listener : public UT_XML::Listener
{
public:

  MailMerge_Listener ( PD_Document *pdoc )
    : UT_XML::Listener(), mDoc(pdoc), mAcceptingText(false)
  {
  }
  
  virtual ~MailMerge_Listener ()
  {
  }
  
  virtual void startElement (const XML_Char * name, const XML_Char ** atts) 
  {
    mCharData.clear ();
    mKey.clear ();

    if (!UT_strcmp (name, "merge"))
	{
	  const XML_Char * key = UT_getAttribute("key", atts);
	  if (key) {
	    mKey = key;
	    mAcceptingText = true;
	  }
	}
  }
  
  virtual void endElement (const XML_Char * name)
  {
    if (!UT_strcmp(name, "merge")) {
      if (mCharData.size()) {
	UT_DEBUGMSG(("DOM: creating mail merge: %s => %s\n", mKey.c_str(),
		     mCharData.utf8_str()));
	mDoc->setMailMergeField (mKey, mCharData);
      }
    }
    mCharData.clear ();
    mKey.clear ();
  }
  
  virtual void charData (const XML_Char * buffer, int length)
  {
    if (buffer && length && mAcceptingText) {
      UT_String buf(buffer, length);
      mCharData += buf.c_str();
    }
  }

private:

  UT_String mKey;
  UT_UTF8String mCharData;
  PD_Document * mDoc;
  bool mAcceptingText;
};

Defun1(mailMerge)
{
  CHECK_FRAME;
  XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
  UT_return_val_if_fail(pFrame, false);

  PD_Document * pDoc = static_cast<PD_Document *>(pFrame->getCurrentDoc());
  UT_return_val_if_fail(pDoc, false);

  pFrame->raise();
  XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_OPEN;
  
  XAP_DialogFactory * pDialogFactory
    = (XAP_DialogFactory *)(pFrame->getDialogFactory());
  
  XAP_Dialog_FileOpenSaveAs * pDialog
    = (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
  UT_ASSERT(pDialog);
  if (!pDialog)
    return false;

  pDialog->runModal(pFrame);

  XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
  bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

  if (bOK)
    {
      UT_String filename = pDialog->getPathname();
      
      UT_XML reader;
      MailMerge_Listener listener (pDoc);
      reader.setListener (&listener);
      reader.parse (filename.c_str());
    }
  
  pDialogFactory->releaseDialog(pDialog);
  return true;
}

Defun1(scriptPlay)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_ASSERT(pFrame);

	UT_String pNewFile;

	UT_ScriptLibrary &instance = UT_ScriptLibrary::instance ();

	if (0 == instance.getNumScripts())
	{
		pFrame->showMessageBox(AP_STRING_ID_SCRIPT_NOSCRIPTS,
				   XAP_Dialog_MessageBox::b_O,
				   XAP_Dialog_MessageBox::a_OK);
		return true;
	}

	UT_ScriptIdType ieft = -1;

	bool bOK = s_AskForScriptName(pFrame, pNewFile, &ieft);

	if (!bOK || pNewFile.empty())
		return false;

	UT_DEBUGMSG(("scriptPlay (trying to play [%s])\n", pNewFile.c_str()));

	if (UT_OK != instance.execute(pNewFile.c_str(), ieft))
	{
		if (instance.errmsg().size() > 0)
		{
			pFrame->showMessageBox(instance.errmsg().c_str(),
					       XAP_Dialog_MessageBox::b_O,
					       XAP_Dialog_MessageBox::a_OK);
		}
		else
			pFrame->showMessageBox(AP_STRING_ID_SCRIPT_CANTRUN,
					       XAP_Dialog_MessageBox::b_O,
					       XAP_Dialog_MessageBox::a_OK,
					       pNewFile.c_str());
	}

	return true;
}

Defun(executeScript)
{
	CHECK_FRAME;
	XAP_Frame* pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_ASSERT(pFrame);
	UT_DEBUGMSG(("executeScript (trying to execute [%s])\n", pCallData->getScriptName().c_str()));

	UT_ScriptLibrary &instance = UT_ScriptLibrary::instance ();

	if (UT_OK != instance.execute(pCallData->getScriptName().c_str()))
	{
		if (instance.errmsg().size() > 0)
			pFrame->showMessageBox(instance.errmsg().c_str(),
								   XAP_Dialog_MessageBox::b_O,
								   XAP_Dialog_MessageBox::a_OK);

		else
			pFrame->showMessageBox(AP_STRING_ID_SCRIPT_CANTRUN,
								   XAP_Dialog_MessageBox::b_O,
								   XAP_Dialog_MessageBox::a_OK,
								   pCallData->getScriptName().c_str());
	}

	return true;
}


Defun(dlgColorPickerFore)
{
	CHECK_FRAME;
	FV_View * pView = static_cast<FV_View *>(pAV_View);

	XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Background * pDialog
		= (AP_Dialog_Background *)(pDialogFactory->requestDialog(AP_DIALOG_ID_BACKGROUND));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;
//
// Set the color in the dialog to the current Color
//
	const XML_Char ** propsChar = NULL;
	pView->getCharFormat(&propsChar);
	const XML_Char * pszChar = UT_getAttribute("color",propsChar);
	pDialog->setColor(pszChar);
//
// Set the dialog to Foreground Color Mode.
//
	pDialog->setForeground();

	pDialog->runModal (pFrame);

	AP_Dialog_Background::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_Background::a_OK);

	if (bOK)
	{
		const XML_Char * clr = pDialog->getColor();
		const XML_Char * properties[] = { "color", NULL, 0};
		properties[1] = clr;
		pView->setCharFormat(properties);
	}
	pDialogFactory->releaseDialog(pDialog);
	FREEP(propsChar);
	return bOK;
}


Defun(dlgColorPickerBack)
{
	CHECK_FRAME;
	FV_View * pView = static_cast<FV_View *>(pAV_View);

	XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Background * pDialog
		= (AP_Dialog_Background *)(pDialogFactory->requestDialog(AP_DIALOG_ID_BACKGROUND));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;
//
// Set the color in the dialog to the current Color
//
	const XML_Char ** propsChar = NULL;
	pView->getCharFormat(&propsChar);
	const XML_Char * pszChar = UT_getAttribute("bgcolor",propsChar);
	pDialog->setColor(pszChar);
//
// Set the dialog to Highlight Color Mode.
//
	pDialog->setHighlight();

	pDialog->runModal (pFrame);

	AP_Dialog_Background::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_Background::a_OK);

	if (bOK)
	{
		const XML_Char * clr = pDialog->getColor();
		const XML_Char * properties[] = { "bgcolor", NULL, 0};
		properties[1] = clr;
		pView->setCharFormat(properties);
	}
	FREEP(propsChar);
	pDialogFactory->releaseDialog(pDialog);
	return bOK;
}

Defun(dlgBackground)
{
	CHECK_FRAME;
	FV_View * pView = static_cast<FV_View *>(pAV_View);

	XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Background * pDialog
		= (AP_Dialog_Background *)(pDialogFactory->requestDialog(AP_DIALOG_ID_BACKGROUND));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

//
// Get Current background color
//
	const XML_Char ** propsSection = NULL;
	pView->getSectionFormat(&propsSection);
	const XML_Char * pszBackground = UT_getAttribute("background-color",propsSection);
	pDialog->setColor(pszBackground);

	pDialog->runModal (pFrame);

	AP_Dialog_Background::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_Background::a_OK);

	if (bOK)
	{
		// let the view set the proper value in the
		// document and refresh/redraw itself
		const XML_Char * clr = pDialog->getColor();
		pView->setPaperColor (clr);
	}

	FREEP(propsSection);
	pDialogFactory->releaseDialog(pDialog);
	return bOK;
}


Defun(dlgHdrFtr)
{
	CHECK_FRAME;
	FV_View * pView = static_cast<FV_View *>(pAV_View);

	XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_HdrFtr * pDialog = (AP_Dialog_HdrFtr *) (pDialogFactory->requestDialog(AP_DIALOG_ID_HDRFTR));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;
//
// Get stuff we need from the view
//
	if(pView->isHdrFtrEdit())
	{
		pView->clearHdrFtrEdit();
		pView->warpInsPtToXY(0,0,false);
	}
	fl_DocSectionLayout * pDSL = (fl_DocSectionLayout *) pView->getCurrentBlock()->getSectionLayout();

	bool bOldHdr = false;
	bool bOldHdrEven = false;
	bool bOldHdrFirst = false;
	bool bOldHdrLast = false;
	bool bOldFtr = false;
	bool bOldFtrEven = false;
	bool bOldFtrFirst = false;
	bool bOldFtrLast = false;
	bool bOldBools[6];
	UT_sint32 i = 0;
	for(i=0; i<6; i++)
	{
		bOldBools[i] = false;
	}
	if(NULL != pDSL->getHeader())
	{
		bOldHdr = true;
	}
	if(NULL != pDSL->getHeaderEven())
	{
		bOldHdrEven = true;
		bOldBools[AP_Dialog_HdrFtr::HdrEven] = true;
	}
	if(NULL != pDSL->getHeaderFirst())
	{
		bOldHdrFirst = true;
		bOldBools[AP_Dialog_HdrFtr::HdrFirst] = true;
	}
	if(NULL != pDSL->getHeaderLast())
	{
		bOldHdrLast = true;
		bOldBools[AP_Dialog_HdrFtr::HdrLast] = true;
	}
	if(NULL != pDSL->getFooter())
	{
		bOldFtr = true;
	}
	if(NULL != pDSL->getFooterEven())
	{
		bOldFtrEven = true;
		bOldBools[AP_Dialog_HdrFtr::FtrEven] = true;
	}
	if(NULL != pDSL->getFooterFirst())
	{
		bOldFtrFirst = true;
		bOldBools[AP_Dialog_HdrFtr::FtrFirst] = true;
	}
	if(NULL != pDSL->getFooterLast())
	{
		bOldFtrLast = true;
		bOldBools[AP_Dialog_HdrFtr::FtrLast] = true;
	}
	for(i =0; i < 6; i++)
	{
		pDialog->setValue((AP_Dialog_HdrFtr::HdrFtr_Control) i,
						  bOldBools[i], false);
	}
	const XML_Char ** propsSectionIn = NULL;
	pView->getSectionFormat(&propsSectionIn);
	const char * szRestart = NULL;
	szRestart = UT_getAttribute("section-restart",propsSectionIn);
	const char * szRestartValue = NULL;
	szRestartValue = UT_getAttribute("section-restart-value",propsSectionIn);
	bool bRestart = false;
	if(szRestart && *szRestart && (UT_strcmp(szRestart,"1") == 0))
	{
		bRestart = true;
	}
	UT_sint32 restartValue = 1;
	if(szRestartValue && *szRestartValue)
	{
		restartValue = atoi(szRestartValue);
	}
	pDialog->setRestart(bRestart, restartValue, false);
	FREEP(propsSectionIn);

	pDialog->runModal (pFrame);

	AP_Dialog_HdrFtr::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_HdrFtr::a_OK);

	if (bOK)
	{
		// let the view set the proper value in the
		// document and refresh/redraw itself
//
// Read back hdr/ftr type changes
//
		bool bNewHdrEven = pDialog->getValue(AP_Dialog_HdrFtr::HdrEven);
		bool bNewHdrFirst = pDialog->getValue(AP_Dialog_HdrFtr::HdrFirst);
		bool bNewHdrLast = pDialog->getValue(AP_Dialog_HdrFtr::HdrLast);
		bool bNewFtrEven = pDialog->getValue(AP_Dialog_HdrFtr::FtrEven);
		bool bNewFtrFirst = pDialog->getValue(AP_Dialog_HdrFtr::FtrFirst);
		bool bNewFtrLast = pDialog->getValue(AP_Dialog_HdrFtr::FtrLast);
//
// Save everything from the PieceTable we need.
//
		pView->SetupSavePieceTableState();
//
// Now delete the header/footers that need to be deleted.
//

		if(bOldHdrEven && !bNewHdrEven)
		{
			pView->removeThisHdrFtr(FL_HDRFTR_HEADER_EVEN);
		}
		if(bOldHdrFirst && !bNewHdrFirst)
		{
			pView->removeThisHdrFtr(FL_HDRFTR_HEADER_FIRST);
		}
		if(bOldHdrLast && !bNewHdrLast)
		{
			pView->removeThisHdrFtr(FL_HDRFTR_HEADER_LAST);
		}
		if(bOldFtrEven && !bNewFtrEven)
		{
			pView->removeThisHdrFtr(FL_HDRFTR_FOOTER_EVEN);
		}
		if(bOldHdrFirst && !bNewHdrFirst)
		{
			pView->removeThisHdrFtr(FL_HDRFTR_FOOTER_FIRST);
		}
		if(bOldHdrLast && !bNewHdrLast)
		{
			pView->removeThisHdrFtr(FL_HDRFTR_FOOTER_LAST);
		}
//
// Now create odd header/footers if there are none and any other Header/Footer
// types are asked for
//
		if(!bOldHdr && (bNewHdrEven || bNewHdrFirst || bNewHdrLast))
		{
			pView->createThisHdrFtr(FL_HDRFTR_HEADER);
		}
		if(!bOldFtr && (bNewFtrEven || bNewFtrFirst || bNewFtrLast))
		{
			pView->createThisHdrFtr(FL_HDRFTR_FOOTER);
		}
//
// OK now create and populate the  requested header/footer types
//
		if(bNewHdrEven && !bOldHdrEven)
		{
			pView->createThisHdrFtr(FL_HDRFTR_HEADER_EVEN);
			pView->populateThisHdrFtr(FL_HDRFTR_HEADER_EVEN);
		}
		if(bNewHdrFirst && !bOldHdrFirst)
		{
			pView->createThisHdrFtr(FL_HDRFTR_HEADER_FIRST);
			pView->populateThisHdrFtr(FL_HDRFTR_HEADER_FIRST);
		}
		if(bNewHdrLast && !bOldHdrLast)
		{
			pView->createThisHdrFtr(FL_HDRFTR_HEADER_LAST);
			pView->populateThisHdrFtr(FL_HDRFTR_HEADER_LAST);
		}
		if(bNewFtrEven && !bOldFtrEven)
		{
			pView->createThisHdrFtr(FL_HDRFTR_FOOTER_EVEN);
			pView->populateThisHdrFtr(FL_HDRFTR_FOOTER_EVEN);
		}
		if(bNewFtrFirst && !bOldFtrFirst)
		{
			pView->createThisHdrFtr(FL_HDRFTR_FOOTER_FIRST);
			pView->populateThisHdrFtr(FL_HDRFTR_FOOTER_FIRST);
		}
		if(bNewFtrLast && !bOldFtrLast)
		{
			pView->createThisHdrFtr(FL_HDRFTR_FOOTER_LAST);
			pView->populateThisHdrFtr(FL_HDRFTR_FOOTER_LAST);
		}


		pView->RestoreSavedPieceTableState();
		if(pDialog->isRestartChanged())
		{
			const char * props_out[] = {"section-restart",NULL,"section-restart-value",NULL,NULL};
			static char szRestartValue[12];
			if(pDialog->isRestart())
			{
				props_out[1] = "1";
				sprintf((char *) szRestartValue,"%i",pDialog->getRestartValue());
				props_out[3] = (const char *) szRestartValue;
			}
			else
			{
				props_out[1] = "0";
				props_out[2] = NULL;
			}
			pView->setSectionFormat((const char **) props_out);
		}
	}

	pDialogFactory->releaseDialog(pDialog);
	return bOK;
}

Defun(hyperlinkJump)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdHyperlinkJump(pCallData->m_xPos, pCallData->m_yPos);
	return true;
}

Defun1(deleteHyperlink)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdDeleteHyperlink();
	return true;
}

Defun(hyperlinkStatusBar)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	GR_Graphics * pG = pView->getGraphics();
	if (pG)
		pG->setCursor(GR_Graphics::GR_CURSOR_LINK);

	pView->cmdHyperlinkStatusBar(pCallData->m_xPos, pCallData->m_yPos);
	return true;
}

static bool s_doMarkRevisions(XAP_Frame * pFrame, PD_Document * pDoc, FV_View * pView)
{
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_MarkRevisions * pDialog
		= (AP_Dialog_MarkRevisions *)(pDialogFactory->requestDialog(AP_DIALOG_ID_MARK_REVISIONS));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

	pDialog->setDocument(pDoc);
	pDialog->runModal(pFrame);
	bool bOK = (pDialog->getAnswer() == AP_Dialog_MarkRevisions::a_OK);

	if (!bOK)
	{
		// we have already turned this on, so turn it off again
		pView->toggleMarkRevisions();
	}
	else
	{
		pDialog->addRevision();
		// we also want to have paragraph marks and etc visible
		AP_FrameData *pFrameData = (AP_FrameData *)pFrame->getFrameData();
		UT_ASSERT(pFrameData);

		if(!pFrameData->m_bShowPara)
		{
			pFrameData->m_bShowPara = true;
			pView->setShowPara(true);
			pView->notifyListeners(AV_CHG_FRAMEDATA);	// to update toolbar
		}
	}


	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

Defun1(toggleMarkRevisions)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	pView->toggleMarkRevisions();

	if(pView->isMarkRevisions())
	{
		PD_Document * pDoc = pView->getDocument();
		XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
		s_doMarkRevisions(pFrame, pDoc, pView);
	}

	return true;
}

Defun(revisionAccept)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdAcceptRejectRevision(false, pCallData->m_xPos, pCallData->m_yPos);
	return true;
}

Defun(revisionReject)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	pView->cmdAcceptRejectRevision(true, pCallData->m_xPos, pCallData->m_yPos);
	return true;
}

static bool s_doListRevisions(XAP_Frame * pFrame, PD_Document * pDoc, FV_View * pView)
{
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_ListRevisions * pDialog
		= (AP_Dialog_ListRevisions *)(pDialogFactory->requestDialog(AP_DIALOG_ID_LIST_REVISIONS));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

	pDialog->setDocument(pDoc);
	pDialog->runModal(pFrame);
	bool bOK = (pDialog->getAnswer() == AP_Dialog_ListRevisions::a_OK);

	if (bOK)
	{
		pView->cmdSetRevisionLevel(pDialog->getSelectedId());
	}


	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

Defun1(revisionSetViewLevel)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc,false);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame,false);

	s_doListRevisions(pFrame, pDoc, pView);

	return true;
}

Defun(resizeImage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if(pG)
	{
		bool bIsResizing = pView->isResizingImage();
		if (!bIsResizing)
			pView->startImageResizing(pCallData->m_xPos, pCallData->m_yPos);
	
		UT_sint32 xOrigin;
		UT_sint32 yOrigin;
		UT_sint32 xDiff; 
		UT_sint32 yDiff;
		
		pView->getResizeOrigin(xOrigin, yOrigin);
		UT_Rect orgImgRect = pView->getImageSelRect();
		
		xDiff = pCallData->m_xPos - xOrigin;
		yDiff = pCallData->m_yPos - yOrigin;	
		
		UT_DEBUGMSG(("MARCM: ap_EditMethods::resizing image! Origin at pos: (x:%d,y:%d) - mouse at pos (x:%d,y:%d)\n", xOrigin, yOrigin, pCallData->m_xPos, pCallData->m_yPos));
	
		UT_sint32 x1,x2,y1,y2,iHeight;
		bool bEOL = false;
		bool bDir = false;
	
		PT_DocPosition pos = pView->getDocPositionFromXY(xOrigin, yOrigin);
	
		fl_BlockLayout * pBlock = pView->getBlockAtPosition(pos);
		fp_Run *  pRun = NULL;
		if(pBlock)
		{
			pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,iHeight,bDir);
			while(pRun && pRun->getType() != FPRUN_IMAGE)
			{
				pRun = pRun->getNext();
			}
			if(pRun && pRun->getType() == FPRUN_IMAGE)
			{
				UT_DEBUGMSG(("MARCM: Image run on pos \n"));
			}
			else
			{
				//UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				return false;
			}
		}

		UT_Rect r = orgImgRect;
		double aRatio = ((double )r.height)/((double )r.width);
		
		// we can savely use the cursor format to see what kind of dragging we are doing
		GR_Graphics::Cursor cur = pView->getImageSelCursor();
		switch (cur)
		{
			case GR_Graphics::GR_CURSOR_IMAGESIZE_NW:
				r.left += xDiff;
				r.top += (UT_sint32)(xDiff * aRatio);
				r.width -= xDiff;
				r.height -= (UT_sint32)(xDiff * aRatio);
				break;
			case GR_Graphics::GR_CURSOR_IMAGESIZE_N:
				r.top += yDiff;
				r.height -= yDiff;
				break;
			case GR_Graphics::GR_CURSOR_IMAGESIZE_NE:
				r.top -= (UT_sint32)(xDiff * aRatio);
				r.width += xDiff;
				r.height += (UT_sint32)(xDiff * aRatio);
				break;		
			case GR_Graphics::GR_CURSOR_IMAGESIZE_E:
				r.width += xDiff;
				break;
			case GR_Graphics::GR_CURSOR_IMAGESIZE_SE:
				r.width += xDiff;
				r.height += (UT_sint32)(xDiff * aRatio);
				break;
			case GR_Graphics::GR_CURSOR_IMAGESIZE_S:			
				r.height += yDiff;
				break;
			case GR_Graphics::GR_CURSOR_IMAGESIZE_SW:			
				r.left += xDiff;
				r.width -= xDiff;
				r.height -= (UT_sint32)(xDiff * aRatio);
				break;
			case GR_Graphics::GR_CURSOR_IMAGESIZE_W:
				r.left += xDiff;
				r.width -= xDiff;
				break;
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		
		pG->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_DOTTED); // MARCM: setting the line style to DOTTED doesn't seem to work with GTK2
		pG->setColor(UT_RGBColor(255,255,255));
		if (bIsResizing)
		{
			UT_DEBUGMSG(("MARCM: Clearing old line\n"));
			xorRect(pG, pView->getCurImageSel());
		}
		pView->setCurImageSel(r);
		xorRect(pG, r);
		pG->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_SOLID);
				
		UT_DEBUGMSG(("MARCM: image display size: (w:%d,h:%d) - total change (w:%d,h:%d)\n",r.width,r.height,xDiff, yDiff));
	}
		
	return true;
}

Defun(endResizeImage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if(pG)
	{
		pView->stopImageResizing();
	
		UT_sint32 xOrigin;
		UT_sint32 yOrigin;
		pView->getResizeOrigin(xOrigin, yOrigin);
		UT_Rect newImgBounds = pView->getCurImageSel();
		
		// an approximate... TODO: make me more accurate
		const fp_PageSize & page = pView->getPageSize ();		
		double max_width = 0., max_height = 0.;
		max_width  = page.Width (DIM_IN) * 72.0;
		max_height = page.Height (DIM_IN) * 72.0;
		
		// some range checking stuff
		newImgBounds.width = abs(newImgBounds.width);
		newImgBounds.height = abs(newImgBounds.height);
		
		if (newImgBounds.width > max_width)
			newImgBounds.width = (UT_sint32)max_width;
		
		if (newImgBounds.height > max_height)
			newImgBounds.height = (UT_sint32)max_height;
		
		if (newImgBounds.width == 0)
			newImgBounds.width = 1;

		if (newImgBounds.height == 0)
			newImgBounds.height = 1;
		
		// clear the resizing line
		pG->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_DOTTED); // MARCM: setting the line style to DOTTED doesn't seem to work with GTK2
		pG->setColor(UT_RGBColor(255,255,255));
		xorRect(pG, pView->getCurImageSel());
		pG->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_SOLID);
		
		UT_DEBUGMSG(("MARCM: ap_EditMethods::done resizing image! new size in px (h:%d,w:%d)\n", newImgBounds.width, newImgBounds.height));
	
		UT_sint32 x1,x2,y1,y2,iHeight;
		bool bEOL = false;
		bool bDir = false;
	
		PT_DocPosition pos = pView->getDocPositionFromXY(xOrigin, yOrigin);
	
		fl_BlockLayout * pBlock = pView->getBlockAtPosition(pos);
		fp_Run *  pRun = NULL;
		if(pBlock)
		{
			pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,iHeight,bDir);
			while(pRun && pRun->getType() != FPRUN_IMAGE)
			{
				pRun = pRun->getNext();
			}
			if(pRun && pRun->getType() == FPRUN_IMAGE)
			{
				UT_DEBUGMSG(("MARCM: Image run on pos \n"));
			}
			else
			{
				//UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				return false;
			}
		}
		pView->cmdSelect(pos,pos+1);
	
		char widthBuf[32];
		char heightBuf[32];
		
		// TODO: set format
		const XML_Char * properties[] = {"width", NULL, "height", NULL, 0};
		char * old_locale = setlocale(LC_NUMERIC, "C");
		double fzoom = (double) pView->getGraphics()->getZoomPercentage();
		double fwidth = (double) newImgBounds.width;
		double fheight = (double) newImgBounds.height;
		fwidth = fwidth*100./fzoom;
		fheight = fheight*100./fzoom;
		UT_sint32 iwidth = (UT_sint32) fwidth;
		UT_sint32 iheight = (UT_sint32) fheight;
		sprintf(widthBuf, "%fin", UT_convertDimToInches(iwidth, DIM_PX));
		sprintf(heightBuf, "%fin", UT_convertDimToInches(iheight, DIM_PX));
		setlocale(LC_NUMERIC, old_locale);
		
		UT_DEBUGMSG(("MARCM: nw:%s nh:%s\n", widthBuf, heightBuf));
		
		properties[1] = widthBuf;
		properties[3] = heightBuf;
		pView->setCharFormat(properties);
		pView->updateScreen();
	}
		
	return true;
}
static UT_sint32 sTopRulerHeight =0;
static UT_sint32 sLeftRulerPos =0;
static UT_sint32 siFixed =0;

Defun(beginVDrag)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	AP_TopRuler * pTopRuler = pView->getTopRuler();
	if(!pTopRuler)
	{
		return true;
	}
	pView->setDragTableLine(true);
	UT_sint32 x = pCallData->m_xPos;
	UT_sint32 y = pCallData->m_yPos;
	PT_DocPosition pos = pView->getDocPositionFromXY(x, y);
	sTopRulerHeight = pTopRuler->setTableLineDrag(pos,x,siFixed);
	pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_GRAB);
	return true;
}

Defun(beginHDrag)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	AP_LeftRuler * pLeftRuler = pView->getLeftRuler();
	if(!pLeftRuler)
	{
		return true;
	}
	pView->setDragTableLine(true);
	UT_sint32 x = pCallData->m_xPos;
	UT_sint32 y = pCallData->m_yPos;
	PT_DocPosition pos = pView->getDocPositionFromXY(x, y);
	sLeftRulerPos = pLeftRuler->setTableLineDrag(pos,siFixed,y);
	pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_GRAB);

	return true;
}

Defun(clearSetCols)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	bool bres = pView->cmdAutoSizeTable();
	pView->setDragTableLine(false);
	return bres;
}

Defun(clearSetRows)
{
	CHECK_FRAME;
	//ABIWORD_VIEW;
	return true;
}

Defun(dragVline)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Doing Line drag \n"));
	AP_TopRuler * pTopRuler = pView->getTopRuler();
	if(!pTopRuler)
	{
		return true;
	}
	UT_sint32 x = pCallData->m_xPos + siFixed;
	pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_GRAB);
	EV_EditModifierState ems = 0; 
	pTopRuler->mouseMotion(ems, x, sTopRulerHeight);
	return true;
}

Defun(dragHline)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	AP_LeftRuler * pLeftRuler = pView->getLeftRuler();
	if(!pLeftRuler)
	{
		return true;
	}
	UT_sint32 y = pCallData->m_yPos;
	pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_GRAB);
	EV_EditModifierState ems = 0; 
	pLeftRuler->mouseMotion(ems, sLeftRulerPos,y);
	return true;
}

Defun(endDragVline)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	AP_TopRuler * pTopRuler = pView->getTopRuler();
	if(!pTopRuler)
	{
		return true;
	}
	UT_sint32 x = pCallData->m_xPos;
	EV_EditModifierState ems = 0; 
	EV_EditMouseButton emb = EV_EMB_BUTTON1;
	pTopRuler->mouseRelease(ems,emb, x, sTopRulerHeight);
	pView->setDragTableLine(false);
	pView->setCursorToContext();
	return true;
}

Defun(endDragHline)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	AP_LeftRuler * pLeftRuler = pView->getLeftRuler();
	if(!pLeftRuler)
	{
		return true;
	}
	UT_sint32 y = pCallData->m_yPos;
	EV_EditModifierState ems = 0; 
	EV_EditMouseButton emb = EV_EMB_BUTTON1;
	pLeftRuler->mouseRelease(ems,emb,sLeftRulerPos,y);
	pView->setDragTableLine(false);
	pView->setCursorToContext();
	return true;
}

Defun(dragImage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if(pG)
	{
		fp_Run *  pRun = NULL;
		
		if (!pView->isDraggingImage())
		{
			UT_sint32 x1,x2,y1,y2,iHeight;
			bool bEOL = false;
			bool bDir = false;
			
			PT_DocPosition pos = pView->getDocPositionFromXY(pCallData->m_xPos, pCallData->m_yPos);
		
			fl_BlockLayout * pBlock = pView->getBlockAtPosition(pos);
			if(pBlock)
			{
				pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,iHeight,bDir);
				while(pRun && pRun->getType() != FPRUN_IMAGE)
				{
					pRun = pRun->getNext();
				}
				if(pRun && pRun->getType() == FPRUN_IMAGE)
				{
					UT_DEBUGMSG(("MARCM: Image run on pos \n"));
				}
				else
				{
					//UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					return false;
				}
			}
			pView->cmdSelect(pos,pos+1);
			pView->startImageDrag(pRun, pCallData->m_xPos, pCallData->m_yPos);
		}
		pView->drawDraggedImage(pCallData->m_xPos, pCallData->m_yPos);
	}
	
	return true;
}

Defun(dropImage)
{	
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if(pG)
	{
		pView->stopImageDrag(pCallData->m_xPos, pCallData->m_yPos);
	}
	
	return true;
}

