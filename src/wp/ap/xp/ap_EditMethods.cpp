/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001 Tomas Frydrych
 * Copyright (C) 2004 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef TOOLKIT_WIN
#include <io.h>
#else
// this ansi header is not available on Windows.
// needed for close()
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "xap_Features.h"
#include "ap_Features.h"
#include "ap_EditMethods.h"

#include "ut_locale.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_bytebuf.h"
#include "ut_Language.h"
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
#include "gr_DrawArgs.h"
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
#include "ap_Dialog_RDFQuery.h"
#include "ap_Dialog_RDFEditor.h"

#ifdef ENABLE_SPELL
#include "ap_Dialog_Spell.h"
#endif

#include "ap_Dialog_Styles.h"
#include "ap_Dialog_Stylist.h"
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
#include "ap_Dialog_InsertXMLID.h"
#include "ap_Dialog_MetaData.h"
#include "ap_Dialog_MarkRevisions.h"
#include "ap_Dialog_ListRevisions.h"
#include "ap_Dialog_MergeCells.h"
#include "ap_Dialog_SplitCells.h"
#include "ap_Dialog_FormatTable.h"
//	Maleesh 6/8/2010 - 
#include "ap_Dialog_Border_Shading.h"
#include "ap_Dialog_FormatFrame.h"
#include "ap_Dialog_FormatFootnotes.h"
#include "ap_Dialog_FormatTOC.h"
#include "ap_Dialog_MailMerge.h"
#include "ap_Dialog_Latex.h"
#include "fv_FrameEdit.h"
#include "fl_FootnoteLayout.h"
#include "gr_EmbedManager.h"
#include "fp_MathRun.h"
#include "ut_mbtowc.h"
#include "fp_EmbedRun.h"
#include "fp_FrameContainer.h"
#include "ap_Frame.h"

#include "xad_Document.h"
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
#include "xap_Dlg_ListDocuments.h"
#include "xap_Dlg_History.h"
#include "xap_Dlg_DocComparison.h"

#include "ie_imp.h"
#include "ie_impGraphic.h"
#include "ie_exp.h"
#include "ie_types.h"

#include "ut_timer.h"
#include "ut_Script.h"
#include "ut_path.h"
#include "ie_mailmerge.h"
#include "gr_Painter.h"
#include "fp_FootnoteContainer.h"


#include "ap_Dialog_Annotation.h"
#include "ap_Preview_Annotation.h"

#include "pd_DocumentRDF.h"

#include <sstream>
#include <iterator>

/*****************************************************************/
/*****************************************************************/

/* abbreviations:
**	 BOL	beginning of line
**	 EOL	end of line
**	 BOW	beginning of word
**	 EOW	end of word
**	 BOS	beginning of sentence
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

class ABI_EXPORT ap_EditMethods
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

	static EV_EditMethod_Fn cairoPrint;
	static EV_EditMethod_Fn cairoPrintDirectly;
	static EV_EditMethod_Fn cairoPrintPreview;

	static EV_EditMethod_Fn cursorDefault;
	static EV_EditMethod_Fn cursorIBeam;
	static EV_EditMethod_Fn cursorRightArrow;
	static EV_EditMethod_Fn cursorTopCell;
	static EV_EditMethod_Fn cursorVline;
	static EV_EditMethod_Fn cursorHline;
	static EV_EditMethod_Fn cursorLeftArrow;
	static EV_EditMethod_Fn cursorImage;
	static EV_EditMethod_Fn cursorImageSize;
	static EV_EditMethod_Fn cursorTOC;

	static EV_EditMethod_Fn contextPosObject;
	static EV_EditMethod_Fn contextImage;
	static EV_EditMethod_Fn contextHyperlink;
	static EV_EditMethod_Fn contextMath;
	static EV_EditMethod_Fn contextMenu;
	static EV_EditMethod_Fn contextRevision;
	static EV_EditMethod_Fn contextTOC;
	static EV_EditMethod_Fn contextText;
#ifdef ENABLE_SPELL
	static EV_EditMethod_Fn contextMisspellText;
#endif
	static EV_EditMethod_Fn contextEmbedLayout;

#ifdef ENABLE_SPELL
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
#endif
	
	static EV_EditMethod_Fn dragToXY;
	static EV_EditMethod_Fn dragToXYword;
	static EV_EditMethod_Fn endDrag;

	static EV_EditMethod_Fn editLatexAtPos;
	static EV_EditMethod_Fn editLatexEquation;
	static EV_EditMethod_Fn editEmbed;

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
	static EV_EditMethod_Fn saveImmediate;
	static EV_EditMethod_Fn selectAll;
	static EV_EditMethod_Fn selectWord;
	static EV_EditMethod_Fn selectLine;
	static EV_EditMethod_Fn selectBlock;
	static EV_EditMethod_Fn selectObject;
	static EV_EditMethod_Fn selectTable;
	static EV_EditMethod_Fn selectRow;
	static EV_EditMethod_Fn selectCell;
	static EV_EditMethod_Fn selectColumn;
	static EV_EditMethod_Fn selectColumnClick;
	static EV_EditMethod_Fn selectMath;
	static EV_EditMethod_Fn selectTOC;

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
	static EV_EditMethod_Fn deleteXMLID;
	static EV_EditMethod_Fn deleteColumns;
	static EV_EditMethod_Fn deleteCell;
	static EV_EditMethod_Fn deleteHyperlink;
	static EV_EditMethod_Fn deleteRows;
	static EV_EditMethod_Fn deleteTable;
	static EV_EditMethod_Fn doEscape;


	static EV_EditMethod_Fn insertBookmark;
	static EV_EditMethod_Fn insertXMLID;
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
	static EV_EditMethod_Fn insertSumRows;
	static EV_EditMethod_Fn insertSumCols;
	static EV_EditMethod_Fn insertTab;
	static EV_EditMethod_Fn insertTabCTL;
	static EV_EditMethod_Fn insertTabShift;

	static EV_EditMethod_Fn insertSpace;
	static EV_EditMethod_Fn insertNBSpace;
	static EV_EditMethod_Fn insertNBZWSpace;
	static EV_EditMethod_Fn insertZWJoiner;
	static EV_EditMethod_Fn insertLRM;
	static EV_EditMethod_Fn insertRLM;
	static EV_EditMethod_Fn insertClosingParenthesis;
	static EV_EditMethod_Fn insertOpeningParenthesis;

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
	static EV_EditMethod_Fn splitCells;
	static EV_EditMethod_Fn formatTable;
	static EV_EditMethod_Fn autoFitTable;

        static EV_EditMethod_Fn repeatThisRow;
        static EV_EditMethod_Fn removeThisRowRepeat;
        static EV_EditMethod_Fn tableToTextCommas;
        static EV_EditMethod_Fn tableToTextTabs;
        static EV_EditMethod_Fn tableToTextCommasTabs;

	static EV_EditMethod_Fn replaceChar;

	static EV_EditMethod_Fn cutVisualText;
	static EV_EditMethod_Fn copyVisualText;
	static EV_EditMethod_Fn dragVisualText;
	static EV_EditMethod_Fn pasteVisualText;
	static EV_EditMethod_Fn btn0VisualText;

	static EV_EditMethod_Fn btn1InlineImage;
	static EV_EditMethod_Fn btn0InlineImage;
	static EV_EditMethod_Fn copyInlineImage;
	static EV_EditMethod_Fn dragInlineImage;
	static EV_EditMethod_Fn releaseInlineImage;

	static EV_EditMethod_Fn btn1Frame;
	static EV_EditMethod_Fn btn0Frame;
	static EV_EditMethod_Fn dragFrame;
	static EV_EditMethod_Fn releaseFrame;
	static EV_EditMethod_Fn contextFrame;
	static EV_EditMethod_Fn deleteFrame;
	static EV_EditMethod_Fn cutFrame;
	static EV_EditMethod_Fn copyFrame;
	static EV_EditMethod_Fn selectFrame;
	static EV_EditMethod_Fn dlgFormatFrame;

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
	static EV_EditMethod_Fn fileNewUsingTemplate;
    static EV_EditMethod_Fn fileRevert;
	static EV_EditMethod_Fn fileOpen;
	static EV_EditMethod_Fn fileSave;
	static EV_EditMethod_Fn fileSaveAs;
	static EV_EditMethod_Fn fileSaveImage;
	static EV_EditMethod_Fn fileSaveEmbed;
	static EV_EditMethod_Fn fileExport;
	static EV_EditMethod_Fn fileImport;
	static EV_EditMethod_Fn importStyles;
	static EV_EditMethod_Fn formatPainter;
	static EV_EditMethod_Fn pageSetup;
	static EV_EditMethod_Fn print;
	static EV_EditMethod_Fn printTB;
	static EV_EditMethod_Fn printPreview;
	static EV_EditMethod_Fn printDirectly;
	static EV_EditMethod_Fn fileInsertGraphic;
	static EV_EditMethod_Fn fileInsertPositionedGraphic;
	static EV_EditMethod_Fn fileInsertPageBackgroundGraphic;
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

	static EV_EditMethod_Fn revisionNew;
	static EV_EditMethod_Fn revisionSelect;

	static EV_EditMethod_Fn viewStd;
	static EV_EditMethod_Fn viewFormat;
	static EV_EditMethod_Fn viewExtra;
	static EV_EditMethod_Fn viewTable;
	static EV_EditMethod_Fn viewTB1;
	static EV_EditMethod_Fn viewTB2;
	static EV_EditMethod_Fn viewTB3;
	static EV_EditMethod_Fn viewTB4;
	static EV_EditMethod_Fn lockToolbarLayout;
	static EV_EditMethod_Fn defaultToolbarLayout;
	static EV_EditMethod_Fn viewRuler;
	static EV_EditMethod_Fn viewStatus;
	static EV_EditMethod_Fn viewPara;
	static EV_EditMethod_Fn viewLockStyles;
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
	static EV_EditMethod_Fn insTextBox;
	static EV_EditMethod_Fn insMailMerge;
	static EV_EditMethod_Fn insSymbol;
	static EV_EditMethod_Fn insFile;
	static EV_EditMethod_Fn insTOC;
	static EV_EditMethod_Fn insFootnote;
	static EV_EditMethod_Fn insEndnote;

#ifdef ENABLE_SPELL
	static EV_EditMethod_Fn dlgSpell;
	static EV_EditMethod_Fn dlgSpellPrefs;
#endif
	
	static EV_EditMethod_Fn dlgWordCount;
	static EV_EditMethod_Fn dlgOptions;
    static EV_EditMethod_Fn dlgMetaData;

	static EV_EditMethod_Fn dlgFont;
	static EV_EditMethod_Fn dlgParagraph;
	static EV_EditMethod_Fn dlgBullets;
	static EV_EditMethod_Fn dlgBorders;
	static EV_EditMethod_Fn dlgColumns;
	static EV_EditMethod_Fn dlgFmtImage;
	static EV_EditMethod_Fn dlgFmtImageCtxt;
	static EV_EditMethod_Fn dlgFmtPosImage;
	static EV_EditMethod_Fn setPosImage;
	static EV_EditMethod_Fn dlgHdrFtr;
	static EV_EditMethod_Fn style;
	static EV_EditMethod_Fn dlgBackground;
	static EV_EditMethod_Fn dlgStyle;
	static EV_EditMethod_Fn dlgStylist;
	static EV_EditMethod_Fn dlgTabs;
	static EV_EditMethod_Fn formatTOC;
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
	static EV_EditMethod_Fn fontSizeIncrease;
	static EV_EditMethod_Fn fontSizeDecrease;
	static EV_EditMethod_Fn toggleBold;
	static EV_EditMethod_Fn toggleDisplayAnnotations;
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
	static EV_EditMethod_Fn toggleDomDirectionSect;
	static EV_EditMethod_Fn toggleDomDirectionDoc;
	static EV_EditMethod_Fn toggleRDFAnchorHighlight;

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
	static EV_EditMethod_Fn helpIntro;
	static EV_EditMethod_Fn helpSearch;
	static EV_EditMethod_Fn helpCheckVer;
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

	static EV_EditMethod_Fn	viCmd_5e;
	static EV_EditMethod_Fn viCmd_A;
	static EV_EditMethod_Fn viCmd_C;
	static EV_EditMethod_Fn viCmd_I;
	static EV_EditMethod_Fn viCmd_J;
	static EV_EditMethod_Fn viCmd_O;
	static EV_EditMethod_Fn viCmd_P;
	static EV_EditMethod_Fn viCmd_a;
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
	static EV_EditMethod_Fn viCmd_o;
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

#ifdef ENABLE_SPELL
	static EV_EditMethod_Fn toggleAutoSpell;
#endif
	
	static EV_EditMethod_Fn scriptPlay;
	static EV_EditMethod_Fn executeScript;

        static EV_EditMethod_Fn mailMerge;

	static EV_EditMethod_Fn hyperlinkCopyLocation;
	static EV_EditMethod_Fn hyperlinkJump;
	static EV_EditMethod_Fn hyperlinkJumpPos;
	static EV_EditMethod_Fn hyperlinkStatusBar;
	static EV_EditMethod_Fn rdfAnchorEditTriples;
	static EV_EditMethod_Fn rdfAnchorQuery;
	static EV_EditMethod_Fn rdfAnchorEditSemanticItem;
	static EV_EditMethod_Fn rdfAnchorExportSemanticItem;
	static EV_EditMethod_Fn rdfAnchorSelectThisReferenceToSemanticItem;
	static EV_EditMethod_Fn rdfAnchorSelectNextReferenceToSemanticItem;
	static EV_EditMethod_Fn rdfAnchorSelectPrevReferenceToSemanticItem;
	static EV_EditMethod_Fn rdfApplyStylesheetContactName;
	static EV_EditMethod_Fn rdfApplyStylesheetContactNick;
	static EV_EditMethod_Fn rdfApplyStylesheetContactNamePhone;
	static EV_EditMethod_Fn rdfApplyStylesheetContactNickPhone;
	static EV_EditMethod_Fn rdfApplyStylesheetContactNameHomepagePhone;
	static EV_EditMethod_Fn rdfApplyStylesheetEventName;
	static EV_EditMethod_Fn rdfApplyStylesheetEventSummary;
	static EV_EditMethod_Fn rdfApplyStylesheetEventSummaryLocation;
	static EV_EditMethod_Fn rdfApplyStylesheetEventSummaryLocationTimes;
	static EV_EditMethod_Fn rdfApplyStylesheetEventSummaryTimes;
	static EV_EditMethod_Fn rdfApplyStylesheetLocationName;
	static EV_EditMethod_Fn rdfApplyStylesheetLocationLatLong;
	static EV_EditMethod_Fn rdfApplyCurrentStyleSheet;
	static EV_EditMethod_Fn rdfStylesheetSettings;
	static EV_EditMethod_Fn rdfDisassocateCurrentStyleSheet;
	static EV_EditMethod_Fn rdfSemitemSetAsSource;
	static EV_EditMethod_Fn rdfSemitemRelatedToSourceFoafKnows;
	static EV_EditMethod_Fn rdfSemitemFindRelatedFoafKnows;
	static EV_EditMethod_Fn textToTable;
	static EV_EditMethod_Fn textToTableCommas;
	static EV_EditMethod_Fn textToTableSpaces;
	static EV_EditMethod_Fn textToTableTabs;
	static EV_EditMethod_Fn toggleMarkRevisions;
	static EV_EditMethod_Fn toggleAutoRevision;
	static EV_EditMethod_Fn revisionAccept;
	static EV_EditMethod_Fn revisionReject;
	static EV_EditMethod_Fn revisionFindNext;
	static EV_EditMethod_Fn revisionFindPrev;
	static EV_EditMethod_Fn revisionSetViewLevel;
	static EV_EditMethod_Fn toggleShowRevisions;
	static EV_EditMethod_Fn toggleShowRevisionsBefore;
	static EV_EditMethod_Fn toggleShowRevisionsAfter;
	static EV_EditMethod_Fn toggleShowRevisionsAfterPrevious;
	static EV_EditMethod_Fn revisionCompareDocuments;
	static EV_EditMethod_Fn purgeAllRevisions;
	static EV_EditMethod_Fn startNewRevision;
	
    static EV_EditMethod_Fn insAnnotation;
    static EV_EditMethod_Fn insAnnotationFromSel;
    static EV_EditMethod_Fn editAnnotation;
	
	static EV_EditMethod_Fn sortColsAscend;
	static EV_EditMethod_Fn sortColsDescend;
	static EV_EditMethod_Fn sortRowsAscend;
	static EV_EditMethod_Fn sortRowsDescend;

	static EV_EditMethod_Fn history;

	
	static EV_EditMethod_Fn insertTable;

#ifdef DEBUG
	static EV_EditMethod_Fn dumpRDFForPoint;
	static EV_EditMethod_Fn dumpRDFObjects;
	static EV_EditMethod_Fn rdfTest;
	static EV_EditMethod_Fn rdfPlay;
#endif
	static EV_EditMethod_Fn rdfQuery;
	static EV_EditMethod_Fn rdfEditor;
	static EV_EditMethod_Fn rdfQueryXMLIDs;
 	static EV_EditMethod_Fn rdfInsertRef;
	static EV_EditMethod_Fn rdfInsertNewContact;
	static EV_EditMethod_Fn rdfInsertNewContactFromFile;
	
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
#define _A_				EV_EMT_APP_METHOD

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
	EV_EditMethod(NF(autoFitTable),         0,      ""),

	// b
	EV_EditMethod(NF(beginHDrag), 0, ""),
	EV_EditMethod(NF(beginVDrag), 0, ""),
	EV_EditMethod(NF(btn0Frame), 0, ""),
	EV_EditMethod(NF(btn0InlineImage), 0, ""),
	EV_EditMethod(NF(btn0VisualText), 0, ""),
	EV_EditMethod(NF(btn1Frame), 0, ""),
	EV_EditMethod(NF(btn1InlineImage), 0, ""),

	// c
	EV_EditMethod(NF(cairoPrint), 0, ""),
	EV_EditMethod(NF(cairoPrintDirectly), 0, ""),
	EV_EditMethod(NF(cairoPrintPreview), 0, ""),
	EV_EditMethod(NF(clearSetCols), 0, ""),
	EV_EditMethod(NF(clearSetRows), 0, ""),
	EV_EditMethod(NF(closeWindow),			0,	""),
	EV_EditMethod(NF(closeWindowX), 0, ""),
	EV_EditMethod(NF(colorBackTB), _D_, ""),
	EV_EditMethod(NF(colorForeTB), _D_, ""),
	EV_EditMethod(NF(contextEmbedLayout), 		0,	""),
	EV_EditMethod(NF(contextFrame), 		0,	""),
	EV_EditMethod(NF(contextHyperlink), 		0,	""),
	EV_EditMethod(NF(contextImage), 0, ""),
	EV_EditMethod(NF(contextMath),			0,	""),
	EV_EditMethod(NF(contextMenu),			0,	""),
#ifdef ENABLE_SPELL
	EV_EditMethod(NF(contextMisspellText),	0,	""),
#endif
	EV_EditMethod(NF(contextPosObject), 0, ""),
	EV_EditMethod(NF(contextRevision),	    0,	""),
	EV_EditMethod(NF(contextTOC),			0,	""),
	EV_EditMethod(NF(contextText),			0,	""),
	EV_EditMethod(NF(copy), 				0,	""),
	EV_EditMethod(NF(copyFrame), 				0,	""),
	EV_EditMethod(NF(copyInlineImage), 				0,	""),
	EV_EditMethod(NF(copyVisualText),		0,	""),
	EV_EditMethod(NF(cursorDefault),		0,	""),
	EV_EditMethod(NF(cursorHline),      	0,	""),
	EV_EditMethod(NF(cursorIBeam),			0,	""),
	EV_EditMethod(NF(cursorImage),			0,	""),
	EV_EditMethod(NF(cursorImageSize),		0,	""),
	EV_EditMethod(NF(cursorLeftArrow),		0,	""),
	EV_EditMethod(NF(cursorRightArrow), 	0,	""),
	EV_EditMethod(NF(cursorTOC), 	0,	""),
	EV_EditMethod(NF(cursorTopCell), 	0,	""),
	EV_EditMethod(NF(cursorVline), 	        0,	""),
	EV_EditMethod(NF(cut),					0,	""),
	EV_EditMethod(NF(cutFrame),					0,	""),
	EV_EditMethod(NF(cutVisualText),		0,	""),
	EV_EditMethod(NF(cycleInputMode),		0,	""),
	EV_EditMethod(NF(cycleWindows), 		0,	""),
	EV_EditMethod(NF(cycleWindowsBck),		0,	""),

	// d
	EV_EditMethod(NF(defaultToolbarLayout),			0,	""),
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
	EV_EditMethod(NF(deleteFrame),   		0,	""),
	EV_EditMethod(NF(deleteHyperlink),		0,	""),
	EV_EditMethod(NF(deleteRows),   		0,	""),
	EV_EditMethod(NF(deleteTable),   		0,	""),
	EV_EditMethod(NF(deleteXMLID),  		0,	""),
	EV_EditMethod(NF(dlgAbout), 			_A_, ""),
	EV_EditMethod(NF(dlgBackground),		0,	""),
	EV_EditMethod(NF(dlgBorders),			0,	""),
	EV_EditMethod(NF(dlgBullets),			0,	""),
	EV_EditMethod(NF(dlgColorPickerBack),	0,	""),
	EV_EditMethod(NF(dlgColorPickerFore),	0,	""),
	EV_EditMethod(NF(dlgColumns),			0,	""),
	EV_EditMethod(NF(dlgFmtImage), 			0, ""),
	EV_EditMethod(NF(dlgFmtImageCtxt), 	   	0, ""),
	EV_EditMethod(NF(dlgFmtPosImage), 		0, ""),
	EV_EditMethod(NF(dlgFont),				0,	""),
	EV_EditMethod(NF(dlgFormatFrame),		0,	""),
	EV_EditMethod(NF(dlgHdrFtr),			0,	""),
	EV_EditMethod(NF(dlgLanguage),			0,	""),
	EV_EditMethod(NF(dlgMetaData), 			0, ""),
	EV_EditMethod(NF(dlgMoreWindows),		0,	""),
	EV_EditMethod(NF(dlgOptions),			0,	""),
	EV_EditMethod(NF(dlgParagraph), 		0,	""),
	EV_EditMethod(NF(dlgPlugins), 			0,	""),
#ifdef ENABLE_SPELL
	EV_EditMethod(NF(dlgSpell), 			0,	""),
	EV_EditMethod(NF(dlgSpellPrefs), 		0,	""),
#endif
	EV_EditMethod(NF(dlgStyle), 			0,	""),
	EV_EditMethod(NF(dlgStylist),           0,  ""),
	EV_EditMethod(NF(dlgTabs),				0,	""),
	EV_EditMethod(NF(dlgToggleCase),		0,	""),
	EV_EditMethod(NF(dlgWordCount), 		0,	""),
	EV_EditMethod(NF(dlgZoom),				0,	""),
	EV_EditMethod(NF(doBullets),			0,	""),
	EV_EditMethod(NF(doEscape),				0,	""),
	EV_EditMethod(NF(doNumbers),			0,	""),
	EV_EditMethod(NF(doubleSpace),			0,	""),
	EV_EditMethod(NF(dragFrame), 			0,	""),
	EV_EditMethod(NF(dragHline), 			0,	""),
	EV_EditMethod(NF(dragInlineImage),		0,	""),
	EV_EditMethod(NF(dragToXY), 			0,	""),
	EV_EditMethod(NF(dragToXYword), 		0,	""),
	EV_EditMethod(NF(dragVisualText),       0, ""),
	EV_EditMethod(NF(dragVline), 			0,	""),
#ifdef DEBUG
	EV_EditMethod(NF(dumpRDFForPoint),		0,	""),
	EV_EditMethod(NF(dumpRDFObjects),		0,	""),
#endif

	
	// e

	EV_EditMethod(NF(editAnnotation),		0,	""),
	EV_EditMethod(NF(editEmbed),			0,	""),
	EV_EditMethod(NF(editFooter),			0,	""),
	EV_EditMethod(NF(editHeader),			0,	""),
	EV_EditMethod(NF(editLatexAtPos),		0,	""),
	EV_EditMethod(NF(editLatexEquation),	0,	""),
	EV_EditMethod(NF(endDrag),				0,	""),
	EV_EditMethod(NF(endDragHline),			0,	""),
	EV_EditMethod(NF(endDragVline),			0,	""),
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
	EV_EditMethod(NF(fileInsertPageBackgroundGraphic),	0,	""),
	EV_EditMethod(NF(fileInsertPositionedGraphic),	0,	""),
	EV_EditMethod(NF(fileNew),				_A_,	""),
	EV_EditMethod(NF(fileNewUsingTemplate),				_A_,	""),
	EV_EditMethod(NF(fileOpen), 			_A_,	""),
	EV_EditMethod(NF(filePreviewWeb), 0, ""),
	EV_EditMethod(NF(fileRevert), 0, ""),
	EV_EditMethod(NF(fileSave), 			0,	""),
	EV_EditMethod(NF(fileSaveAs),			0,	""),
	EV_EditMethod(NF(fileSaveAsWeb),				0, ""),
	EV_EditMethod(NF(fileSaveEmbed),		0,	""),
	EV_EditMethod(NF(fileSaveImage),		0,	""),
	EV_EditMethod(NF(fileSaveTemplate), 0, ""),
	EV_EditMethod(NF(find), 				0,	""),
	EV_EditMethod(NF(findAgain),			0,	""),
	EV_EditMethod(NF(fontFamily),			_D_,	""),
	EV_EditMethod(NF(fontSize), 			_D_,	""),
	EV_EditMethod(NF(fontSizeDecrease),		0,	""),
	EV_EditMethod(NF(fontSizeIncrease),		0,	""),
	EV_EditMethod(NF(formatFootnotes),        0,  ""),
	EV_EditMethod(NF(formatPainter),		0,	""),
	EV_EditMethod(NF(formatTOC),			0,		""),
	EV_EditMethod(NF(formatTable),			0,		""),

	// g
	EV_EditMethod(NF(go),					0,	""),

	// h
	EV_EditMethod(NF(helpAboutGnomeOffice), _A_, ""),
	EV_EditMethod(NF(helpCheckVer), 		_A_,		""),
	EV_EditMethod(NF(helpContents), 		_A_,		""),
	EV_EditMethod(NF(helpCredits), _A_, ""),
	EV_EditMethod(NF(helpIntro),			_A_,		""),
	EV_EditMethod(NF(helpReportBug), _A_, ""),
	EV_EditMethod(NF(helpSearch),			_A_,		""),
	EV_EditMethod(NF(history),	            0,      ""),
	EV_EditMethod(NF(hyperlinkCopyLocation), 0, ""),
	EV_EditMethod(NF(hyperlinkJump),		0,		""),
	EV_EditMethod(NF(hyperlinkJumpPos),     0,      ""),
	EV_EditMethod(NF(hyperlinkStatusBar),	0,		""),
	// i
	EV_EditMethod(NF(importStyles),			0,		""),
	EV_EditMethod(NF(insAnnotation),		0,		""),
	EV_EditMethod(NF(insAnnotationFromSel),	0,		""),
	EV_EditMethod(NF(insBreak),				0,		""),
	EV_EditMethod(NF(insDateTime),			0,		""),
	EV_EditMethod(NF(insEndnote),			0,		""),
	EV_EditMethod(NF(insField),				0,		""),
	EV_EditMethod(NF(insFile),				0,		""),
	EV_EditMethod(NF(insFootnote),			0,		""),
	EV_EditMethod(NF(insMailMerge), 		0,		""),
	EV_EditMethod(NF(insPageNo),			0,		""),
	EV_EditMethod(NF(insSymbol),			0,		""),
	EV_EditMethod(NF(insTOC),			0,		""),
	EV_EditMethod(NF(insTextBox),			0,		""),
	EV_EditMethod(NF(insertAbovedotData),	_D_,	""),
	EV_EditMethod(NF(insertAcuteData),		_D_,	""),
	EV_EditMethod(NF(insertBookmark),		0,	""),
	EV_EditMethod(NF(insertBreveData),		_D_,	""),
	EV_EditMethod(NF(insertCaronData),		_D_,	""),
	EV_EditMethod(NF(insertCedillaData),	_D_,	""),
	EV_EditMethod(NF(insertCircumflexData), _D_,	""),
	EV_EditMethod(NF(insertClipart), 0, ""),
	EV_EditMethod(NF(insertClosingParenthesis),	_D_,""),
	EV_EditMethod(NF(insertColsAfter),	0,	""),
	EV_EditMethod(NF(insertColsBefore),	0,	""),
	EV_EditMethod(NF(insertColumnBreak),	0,	""),
	EV_EditMethod(NF(insertData),			_D_,	""),
	EV_EditMethod(NF(insertDiaeresisData),	_D_,	""),
	EV_EditMethod(NF(insertDoubleacuteData),_D_,	""),
	EV_EditMethod(NF(insertGraveData),		_D_,	""),
	EV_EditMethod(NF(insertHyperlink),		0,	""),
	EV_EditMethod(NF(insertLRM),		0,	""),
	EV_EditMethod(NF(insertLineBreak),		0,	""),
	EV_EditMethod(NF(insertMacronData), 	_D_,	""),
	EV_EditMethod(NF(insertNBSpace),		0,	""),
	EV_EditMethod(NF(insertNBZWSpace),		0,	""),
	EV_EditMethod(NF(insertOgonekData), 	_D_,	""),
	EV_EditMethod(NF(insertOpeningParenthesis),	_D_,""),
	EV_EditMethod(NF(insertPageBreak),		0,	""),
	EV_EditMethod(NF(insertParagraphBreak), 0,	""),
	EV_EditMethod(NF(insertRLM),		0,	""),
	EV_EditMethod(NF(insertRowsAfter),	0,	""),
	EV_EditMethod(NF(insertRowsBefore),	0,	""),
	EV_EditMethod(NF(insertSectionBreak),	0,	""),
	EV_EditMethod(NF(insertSoftBreak),		0,	""),
	EV_EditMethod(NF(insertSpace),			0,	""),
	EV_EditMethod(NF(insertSumCols),			0,	""),
	EV_EditMethod(NF(insertSumRows),			0,	""),
	EV_EditMethod(NF(insertTab),			0,	""),
	EV_EditMethod(NF(insertTabCTL),			0,	""),
	EV_EditMethod(NF(insertTabShift),			0,	""),
	EV_EditMethod(NF(insertTable),          0,  ""),
	EV_EditMethod(NF(insertTildeData),		_D_,	""),
	EV_EditMethod(NF(insertXMLID),    		0,	""),
	EV_EditMethod(NF(insertZWJoiner),		0,	""),

	// j

	// k

	// l
	EV_EditMethod(NF(language), 		0,	""),
	EV_EditMethod(NF(lockToolbarLayout),	0,	""),

	// m
	EV_EditMethod(NF(mailMerge), 0, ""),
	EV_EditMethod(NF(mergeCells),			0,		""),
	EV_EditMethod(NF(middleSpace),			0,		""),

	// n
	EV_EditMethod(NF(newWindow),			0,	""),
	EV_EditMethod(NF(noop), 				0,	""),

	// o
	EV_EditMethod(NF(openRecent_1), 		_A_,		""),
	EV_EditMethod(NF(openRecent_2), 		_A_,		""),
	EV_EditMethod(NF(openRecent_3), 		_A_,		""),
	EV_EditMethod(NF(openRecent_4), 		_A_,		""),
	EV_EditMethod(NF(openRecent_5), 		_A_,		""),
	EV_EditMethod(NF(openRecent_6), 		_A_,		""),
	EV_EditMethod(NF(openRecent_7), 		_A_,		""),
	EV_EditMethod(NF(openRecent_8), 		_A_,		""),
	EV_EditMethod(NF(openRecent_9), 		_A_,		""),
	EV_EditMethod(NF(openTemplate), 0, ""),

	// p
#ifdef ENABLE_PRINT
	EV_EditMethod(NF(pageSetup),			0,	""),
#endif
	EV_EditMethod(NF(paraBefore0),			0,		""),
	EV_EditMethod(NF(paraBefore12), 		0,		""),
		// intended for ^V and Menu[Edit/Paste]
	EV_EditMethod(NF(paste),				0,	""),
			// intended for X11 middle mouse
	EV_EditMethod(NF(pasteSelection),		0,	""),
	EV_EditMethod(NF(pasteSpecial), 		0,	""),
	EV_EditMethod(NF(pasteVisualText), 		0,	""),
	EV_EditMethod(NF(print),				0,	""),
#ifdef ENABLE_PRINT
	EV_EditMethod(NF(printDirectly),		0,	""),
	EV_EditMethod(NF(printPreview),			0,	""),
	EV_EditMethod(NF(printTB),				0,	""),
#endif
	EV_EditMethod(NF(purgeAllRevisions),	0,	""),

	// q
	EV_EditMethod(NF(querySaveAndExit), 	_A_,	""),

	// r
	EV_EditMethod(NF(rdfAnchorEditSemanticItem) , 0,  ""),
	EV_EditMethod(NF(rdfAnchorEditTriples), 0,  ""),
	EV_EditMethod(NF(rdfAnchorExportSemanticItem) , 0,  ""),
	EV_EditMethod(NF(rdfAnchorQuery) ,     0,  ""),
	EV_EditMethod(NF(rdfAnchorSelectNextReferenceToSemanticItem) , 0,  ""),
	EV_EditMethod(NF(rdfAnchorSelectPrevReferenceToSemanticItem) , 0,  ""),
	EV_EditMethod(NF(rdfAnchorSelectThisReferenceToSemanticItem) , 0,  ""),
	EV_EditMethod(NF(rdfApplyCurrentStyleSheet),  0,  ""),
	EV_EditMethod(NF(rdfApplyStylesheetContactName) ,  0,  ""),
	EV_EditMethod(NF(rdfApplyStylesheetContactNameHomepagePhone) ,  0,  ""),
	EV_EditMethod(NF(rdfApplyStylesheetContactNamePhone) ,  0,  ""),
	EV_EditMethod(NF(rdfApplyStylesheetContactNick) ,  0,  ""),
	EV_EditMethod(NF(rdfApplyStylesheetContactNickPhone) ,  0,  ""),
	EV_EditMethod(NF(rdfApplyStylesheetEventName) ,  0,  ""),
	EV_EditMethod(NF(rdfApplyStylesheetEventSummary) ,  0,  ""),
	EV_EditMethod(NF(rdfApplyStylesheetEventSummaryLocation) ,  0,  ""),
	EV_EditMethod(NF(rdfApplyStylesheetEventSummaryLocationTimes) ,  0,  ""),
	EV_EditMethod(NF(rdfApplyStylesheetEventSummaryTimes) ,  0,  ""),
	EV_EditMethod(NF(rdfApplyStylesheetLocationLatLong) ,  0,  ""),
	EV_EditMethod(NF(rdfApplyStylesheetLocationName) ,  0,  ""),
	EV_EditMethod(NF(rdfDisassocateCurrentStyleSheet),  0,  ""),
	EV_EditMethod(NF(rdfEditor),            0,	""),
	EV_EditMethod(NF(rdfInsertNewContact),  0,	""),
	EV_EditMethod(NF(rdfInsertNewContactFromFile),  0,	""),
	EV_EditMethod(NF(rdfInsertRef),         0,	""),
#ifdef DEBUG
	EV_EditMethod(NF(rdfPlay), 				0,	""),
#endif
	EV_EditMethod(NF(rdfQuery),             0,	""),
	EV_EditMethod(NF(rdfQueryXMLIDs),       0,	""),
	EV_EditMethod(NF(rdfSemitemFindRelatedFoafKnows),  0,  ""),
	EV_EditMethod(NF(rdfSemitemRelatedToSourceFoafKnows),  0,  ""),
	EV_EditMethod(NF(rdfSemitemSetAsSource),  0,  ""),
	EV_EditMethod(NF(rdfStylesheetSettings),  0,  ""),
#ifdef DEBUG
	EV_EditMethod(NF(rdfTest), 				0,	""),
#endif
	EV_EditMethod(NF(redo), 				0,	""),
	EV_EditMethod(NF(releaseFrame), 		0,	""),
	EV_EditMethod(NF(releaseInlineImage), 		0,	""),
	EV_EditMethod(NF(removeFooter), 		0,	""),
	EV_EditMethod(NF(removeHeader), 		0,	""),
	EV_EditMethod(NF(removeThisRowRepeat), 		0,	""),
	EV_EditMethod(NF(repeatThisRow),		0,	""),
	EV_EditMethod(NF(replace),				0,	""),
	EV_EditMethod(NF(replaceChar),			_D_,""),
	EV_EditMethod(NF(revisionAccept),		0,  ""),
	EV_EditMethod(NF(revisionCompareDocuments),	0,  ""),
	EV_EditMethod(NF(revisionFindNext),		0,  ""),
	EV_EditMethod(NF(revisionFindPrev),		0,  ""),
	EV_EditMethod(NF(revisionNew),   		0,	""),
	EV_EditMethod(NF(revisionReject),		0,  ""),
	EV_EditMethod(NF(revisionSelect),       0,	""),
	EV_EditMethod(NF(revisionSetViewLevel),	0,  ""),
	EV_EditMethod(NF(rotateCase),			0,	""),

	// s

	EV_EditMethod(NF(saveImmediate),			0,	""),
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
	EV_EditMethod(NF(selectCell),			0,	""),
	EV_EditMethod(NF(selectColumn),			0,	""),
	EV_EditMethod(NF(selectColumnClick),			0,	""),
	EV_EditMethod(NF(selectFrame),			0,	""),
	EV_EditMethod(NF(selectLine),			0,	""),
	EV_EditMethod(NF(selectMath),			0,	""),
	EV_EditMethod(NF(selectObject), 		0,	""),
	EV_EditMethod(NF(selectRow),			0,	""),
	EV_EditMethod(NF(selectTOC),			0,	""),
	EV_EditMethod(NF(selectTable),			0,	""),
	EV_EditMethod(NF(selectWord),			0,	""),
	EV_EditMethod(NF(setEditVI),			0,	""),
	EV_EditMethod(NF(setInputVI),			0,	""),
	EV_EditMethod(NF(setPosImage), 	0,		""),
	EV_EditMethod(NF(setStyleHeading1), 	0,		""),
	EV_EditMethod(NF(setStyleHeading2), 	0,		""),
	EV_EditMethod(NF(setStyleHeading3), 	0,		""),
	EV_EditMethod(NF(singleSpace),			0,		""),
	EV_EditMethod(NF(sortColsAscend),       0,  ""),
	EV_EditMethod(NF(sortColsDescend),      0,  ""),
	EV_EditMethod(NF(sortRowsAscend),       0,  ""),
	EV_EditMethod(NF(sortRowsDescend),      0,  ""),
#ifdef ENABLE_SPELL
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
#endif
	EV_EditMethod(NF(splitCells),           0,  ""),
	EV_EditMethod(NF(startNewRevision),     0,  ""),
	EV_EditMethod(NF(style),				_D_,""),

	// t
	EV_EditMethod(NF(tableToTextCommas),	0,		""),
	EV_EditMethod(NF(tableToTextCommasTabs),    0,		""),
	EV_EditMethod(NF(tableToTextTabs),    0,		""),
	EV_EditMethod(NF(textToTable),			0,		""),
	EV_EditMethod(NF(textToTableCommas),		0,		""),
	EV_EditMethod(NF(textToTableSpaces),			0,		""),
	EV_EditMethod(NF(textToTableTabs),		0,		""),
	EV_EditMethod(NF(toggleAutoRevision),  0,  ""),
#ifdef ENABLE_SPELL
	EV_EditMethod(NF(toggleAutoSpell),      0,  ""),
#endif
	EV_EditMethod(NF(toggleBold),			0,	""),
	EV_EditMethod(NF(toggleBottomline), 	0,	""),
	EV_EditMethod(NF(toggleDirOverrideLTR), 0,	""),
	EV_EditMethod(NF(toggleDirOverrideRTL), 0,	""),
	EV_EditMethod(NF(toggleDisplayAnnotations), 0,	""),
	EV_EditMethod(NF(toggleDomDirection),	0,	""),
	EV_EditMethod(NF(toggleDomDirectionDoc),	0,	""),
	EV_EditMethod(NF(toggleDomDirectionSect),	0,	""),
	EV_EditMethod(NF(toggleHidden),			0,	""),
	EV_EditMethod(NF(toggleIndent),         0,  ""),
	EV_EditMethod(NF(toggleInsertMode), 	0,  ""),
	EV_EditMethod(NF(toggleItalic), 		0,	""),
	EV_EditMethod(NF(toggleMarkRevisions),  0,  ""),
	EV_EditMethod(NF(toggleOline),			0,  ""),
	EV_EditMethod(NF(togglePlain),			0,	""),
	EV_EditMethod(NF(toggleRDFAnchorHighlight), 0,	""),
	EV_EditMethod(NF(toggleShowRevisions),  0,  ""),
	EV_EditMethod(NF(toggleShowRevisionsAfter),  0,  ""),
	EV_EditMethod(NF(toggleShowRevisionsAfterPrevious),  0,  ""),
	EV_EditMethod(NF(toggleShowRevisionsBefore),  0,  ""),
	EV_EditMethod(NF(toggleStrike), 		0,	""),
	EV_EditMethod(NF(toggleSub),			0,	""),
	EV_EditMethod(NF(toggleSuper),			0,	""),
	EV_EditMethod(NF(toggleTopline),		0,	""),
	EV_EditMethod(NF(toggleUline),			0,	""),
	EV_EditMethod(NF(toggleUnIndent),       0,  ""),

	// u
	EV_EditMethod(NF(undo), 				0,	""),

	// v
	EV_EditMethod(NF(viCmd_5e),		0,	""), //^ 
	EV_EditMethod(NF(viCmd_A),		0,	""),
	EV_EditMethod(NF(viCmd_C),		0,	""),
	EV_EditMethod(NF(viCmd_I),		0,	""),
	EV_EditMethod(NF(viCmd_J),		0,	""),
	EV_EditMethod(NF(viCmd_O),		0,	""),
	EV_EditMethod(NF(viCmd_P),		0,	""),
	EV_EditMethod(NF(viCmd_a),		0,	""),
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
	EV_EditMethod(NF(viCmd_o),		0,	""),
	EV_EditMethod(NF(viCmd_y24),	0,	""),
	EV_EditMethod(NF(viCmd_y28),	0,	""),
	EV_EditMethod(NF(viCmd_y29),	0,	""),
	EV_EditMethod(NF(viCmd_y5b),	0,	""),
	EV_EditMethod(NF(viCmd_y5d),	0,	""),
	EV_EditMethod(NF(viCmd_y5e),	0,	""),
	EV_EditMethod(NF(viCmd_yb), 	0,	""),
	EV_EditMethod(NF(viCmd_yw), 	0,	""),
	EV_EditMethod(NF(viCmd_yy), 	0,	""),
#if !XAP_SIMPLE_TOOLBAR
	EV_EditMethod(NF(viewExtra),			0,		""),
	EV_EditMethod(NF(viewFormat),			0,		""),
#endif
	EV_EditMethod(NF(viewFullScreen), 0, ""),
	EV_EditMethod(NF(viewHeadFoot), 		0,		""),
	EV_EditMethod(NF(viewLockStyles),   0,		""),
	EV_EditMethod(NF(viewNormalLayout), 0, ""),
	EV_EditMethod(NF(viewPara), 		0,		""),
	EV_EditMethod(NF(viewPrintLayout), 0, ""),
	EV_EditMethod(NF(viewRuler),			0,		""),
	EV_EditMethod(NF(viewStatus),			0,		""),
#if !XAP_SIMPLE_TOOLBAR
	EV_EditMethod(NF(viewStd),			0,		""),
#endif
	// capitals before lowercase ...
	EV_EditMethod(NF(viewTB1),			0,		""),
	EV_EditMethod(NF(viewTB2),			0,		""),
	EV_EditMethod(NF(viewTB3),			0,		""),
	EV_EditMethod(NF(viewTB4),			0,		""),
#if !XAP_SIMPLE_TOOLBAR
	EV_EditMethod(NF(viewTable),			0,		""),	
#endif
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

	return new EV_EditMethodContainer(G_N_ELEMENTS(s_arrayEditMethods),s_arrayEditMethods);
}

#undef _D_
#undef _A_
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
static bool _openURL(const char* url);

static UT_Timer * s_pToUpdateCursor = NULL;
static UT_Worker * s_pFrequentRepeat = NULL;
static XAP_Frame * s_pLoadingFrame = NULL;
static AD_Document * s_pLoadingDoc = NULL;
static bool s_LockOutGUI = false;

class ABI_EXPORT _Freq
{
public:
	_Freq(AV_View * pView,EV_EditMethodCallData * pData, void(* exe)(AV_View * pView,EV_EditMethodCallData * pData)):
		m_pView (pView),
		m_pData(pData),
		m_pExe(exe)
	{ xxx_UT_DEBUGMSG(("_Freq created %x ",this));};
	AV_View * m_pView;
	EV_EditMethodCallData * m_pData;
	void(* m_pExe)(AV_View * ,EV_EditMethodCallData *) ;
};

/*!
This little macro locks out loading frames from any activity thus preventing
segfaults.
*
* Also used to lock out operations during a frequently repeated event 
* (like holding down an arrow key)
*
*/

static bool s_EditMethods_check_frame(void)
{
	bool result = false;
	if(s_LockOutGUI)
	{
		return true;
	}
	if(s_pFrequentRepeat != NULL)
	{
		xxx_UT_DEBUGMSG(("Dropping frequent event!!!! \n"));
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
static bool lockGUI(void)
{
	s_LockOutGUI = true;
	return true;
}


/*!
 * Call this to allow GUI operations on AbiWord.
 */
static bool unlockGUI(void)
{
	s_LockOutGUI = false;
	return true;
}

#define CHECK_FRAME if(s_EditMethods_check_frame()) return true;

/*!
 * use this code to execute a one-off operation in an idle loop.
 * This allows us to drop frequent events like those that come from arrow keys
 * so we never get ahead of ourselves.
 */
static void _sFrequentRepeat(UT_Worker * pWorker)
{
	// I have experienced a situation in which the the worker fired recursively while
	// inside of the m_pExe function, creating an endless loop; this prevents that from hapening
	// (the problem was caused by an endless loop elsewhere, but the recursive firing made
	// it hard to diagnose; in any case when we use a timer rather than idle, this could
	// happen if m_pExe is taking longer to execute than the timer interval)
	
	static bool bRunning = false;

	if(bRunning)
		return;
	
	bRunning = true;
//
// Once run then delete, stop and set to NULL
//
	
	_Freq * pFreq = static_cast<_Freq *>(pWorker->getInstanceData());
	xxx_UT_DEBUGMSG((" _sFrequentRepeat: pWorker %x pFeq %x \n",pWorker,pFreq));
	s_pFrequentRepeat->stop();
	UT_Worker * pTmp =  s_pFrequentRepeat;
	//
	// Set s_pFrequentRepeat to NULL before we execute the method
	// so that the call itself doesn't generate a new event to process
	//
	s_pFrequentRepeat = NULL;

	pFreq->m_pExe(pFreq->m_pView,pFreq->m_pData);
	DELETEP(pFreq->m_pData);
	delete pFreq;
	delete pTmp;

	
	bRunning = false;
}

#ifdef ENABLE_SPELL
Defun1(toggleAutoSpell)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail (pFrame, false);

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail (pPrefs, false);

	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	UT_return_val_if_fail (pPrefsScheme, false);

	bool b = false;

	pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_AutoSpellCheck), &b);
	return pPrefsScheme->setValueBool(static_cast<const gchar *>(AP_PREF_KEY_AutoSpellCheck), !b);
}
#endif

Defun1(scrollPageDown)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGEDOWN);

	return true;
}

Defun1(scrollPageUp)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGEUP);

	return true;
}

Defun1(scrollPageLeft)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGELEFT);

	return true;
}

Defun1(scrollPageRight)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGERIGHT);

	return true;
}

Defun1(scrollLineDown)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	pAV_View->cmdScroll(AV_SCROLLCMD_LINEDOWN);

	return true;
}

Defun1(scrollLineUp)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	pAV_View->cmdScroll(AV_SCROLLCMD_LINEUP);

	return true;
}

Defun1(scrollWheelMouseDown)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	xxx_UT_DEBUGMSG(("Wheel Mouse Down \n"));
	pAV_View->cmdScroll(AV_SCROLLCMD_LINEDOWN, pAV_View->getGraphics()->tlu(60));

	return true;
}

Defun1(scrollWheelMouseUp)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	xxx_UT_DEBUGMSG(("Wheel Mouse Up \n"));
	pAV_View->cmdScroll(AV_SCROLLCMD_LINEUP, pAV_View->getGraphics()->tlu (60));

	return true;
}

Defun1(scrollLineLeft)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	pAV_View->cmdScroll(AV_SCROLLCMD_LINELEFT);

	return true;
}

Defun1(scrollLineRight)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	pAV_View->cmdScroll(AV_SCROLLCMD_LINERIGHT);

	return true;
}

Defun1(scrollToTop)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	pAV_View->cmdScroll(AV_SCROLLCMD_TOTOP);

	return true;
}

Defun1(scrollToBottom)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	pAV_View->cmdScroll(AV_SCROLLCMD_TOBOTTOM);

	return true;
}

Defun0(fileNew)
{
	CHECK_FRAME;
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, false);
	
#if 0 //def HAVE_HILDON	
	
	XAP_Frame * pNewFrame;
	if (pApp->getFrameCount() == 0)
		pNewFrame = pApp->newFrame();
	else
	{		
		//fileSave(NULL, NULL);		
		pNewFrame = pApp->getFrame(0);
		if (pNewFrame->isDirty())
		{
			if(!fileSave(pAV_View, NULL))
			{
				// we cannot just close the dirty file when the user clicked cancel -- if
				// she really want to loose unsaved changes, let her close it manually
				return false;
			}
		}
	}
#else
	XAP_Frame * pNewFrame = pApp->newFrame();
#endif

	// the IEFileType here doesn't really matter, since the name is NULL
	UT_Error error = pNewFrame->loadDocument((const char *)NULL, IEFT_Unknown);

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
static UT_sint32 s_iLastYScrollOffset = -1;
static UT_sint32 s_iLastXScrollOffset = -1;
static bool      s_bFreshDraw = false;

static void s_LoadingCursorCallback(UT_Worker * pTimer )
{
	UT_return_if_fail (pTimer);
	xxx_UT_DEBUGMSG(("Update Screen on load Frame %x \n",s_pLoadingFrame));
	XAP_Frame * pFrame = s_pLoadingFrame;
	UT_uint32 iPageCount = 0;
	
	if(pFrame == NULL)
	{
		s_bFirstDrawDone = false;
		return;
	}
	pFrame->setCursor(GR_Graphics::GR_CURSOR_WAIT);
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
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
			iPageCount = pLayout->countPages();

			if(!s_bFirstDrawDone && (iPageCount > 1))
			{
				pView->draw();
				s_bFirstDrawDone = true;
			}
			else
			{
				// we only want to draw if we need to:
				//   (1) if the scroller position has changed
				//   (2) if the previous draw was due to a scroll change
				//
				// This way each change of scroller position will
				// result in two draws, the second of which will
				// ensure that anything from the current vieport that
				// was not yet laid out when the first draw was made
				// is drawn
				if(iPageCount > 1)
				{
					pView->notifyListeners(AV_CHG_PAGECOUNT | AV_CHG_WINDOWSIZE);
					if(pView->getYScrollOffset() != s_iLastYScrollOffset ||
					   pView->getXScrollOffset() != s_iLastXScrollOffset)
					{
						pView->updateScreen(true);
						s_iLastYScrollOffset = pView->getYScrollOffset();
						s_iLastXScrollOffset = pView->getXScrollOffset();
						s_bFreshDraw = true;
						xxx_UT_DEBUGMSG(("Incr. loader: primary draw\n"));
					}
					else if(s_bFreshDraw)
					{
					    pView->updateScreen(true);
						s_bFreshDraw = false;
						xxx_UT_DEBUGMSG(("Incr. loader: secondary draw\n"));
					}
					else
					{
						xxx_UT_DEBUGMSG(("Incr. loader: draw not needed\n"));
					}
				}
			}
		}
	}
	else
	{
		s_bFirstDrawDone = false;
	}
}

/*!
 * Control Method for the updating loader.
\param bool bStartStop true to start the updating loader, flase to stop it
             after the document has loaded.
\param XAP_Frame * pFrame Pointer to the new frame being loaded.
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
			s_pToUpdateCursor = UT_Timer::static_constructor(s_LoadingCursorCallback,NULL);
		}
		s_bFirstDrawDone = false;
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
		UT_String msg (pSS->getValue(XAP_STRING_ID_MSG_ImportingDoc));
		pFrame->setStatusMessage ( static_cast<const gchar *>(msg.c_str()) );
		s_pToUpdateCursor->set(1000);
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
				FV_View * pView = static_cast<FV_View *>(s_pLoadingFrame->getCurrentView());
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


static void s_TellSaveFailed(XAP_Frame * pFrame, const char * fileName, UT_Error errorCode)
{
	XAP_String_Id String_id;

    switch(errorCode) {
    case UT_SAVE_CANCELLED: // We actually don't have a write error
        return;

    case UT_SAVE_WRITEERROR: // We have a write error
        String_id = AP_STRING_ID_MSG_SaveFailedWrite;
        break;

	case UT_SAVE_NAMEERROR: // We have a name error
        String_id = AP_STRING_ID_MSG_SaveFailedName;
        break;

	case UT_SAVE_EXPORTERROR: // We have an export error
        String_id = AP_STRING_ID_MSG_SaveFailedExport;
        break;

	default: // The generic case - should be eliminated eventually
        String_id = AP_STRING_ID_MSG_SaveFailed;
        break;
    }
	pFrame->showMessageBox(String_id,
			       XAP_Dialog_MessageBox::b_O,
			       XAP_Dialog_MessageBox::a_OK,
			       fileName);
}

#ifdef ENABLE_SPELL
static void s_TellSpellDone(XAP_Frame * pFrame, bool bIsSelection)
{
	pFrame->showMessageBox(bIsSelection ? AP_STRING_ID_MSG_SpellSelectionDone : AP_STRING_ID_MSG_SpellDone,
			       XAP_Dialog_MessageBox::b_O,
			       XAP_Dialog_MessageBox::a_OK);
}
#endif

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

	char *pFilename = UT_go_filename_from_uri(pFrame->getFilename());

	bool b = (pFrame->showMessageBox(AP_STRING_ID_MSG_RevertBuffer,
										XAP_Dialog_MessageBox::b_YN,
										XAP_Dialog_MessageBox::a_YES,
										pFilename)
						== XAP_Dialog_MessageBox::a_YES);

	FREEP(pFilename);
	return b;
}

#if XAP_DONT_CONFIRM_QUIT
#else
static bool s_AskCloseAllAndExit(XAP_Frame * pFrame)
{
	// return true if we should quit.
	return (pFrame->showMessageBox(AP_STRING_ID_MSG_QueryExit,
				       XAP_Dialog_MessageBox::b_YN,
				       XAP_Dialog_MessageBox::a_NO)
		== XAP_Dialog_MessageBox::a_YES);

}
#endif

static XAP_Dialog_MessageBox::tAnswer s_AskSaveFile(XAP_Frame * pFrame)
{
	XAP_Dialog_MessageBox * message = 
		pFrame->createMessageBox(AP_STRING_ID_MSG_ConfirmSave,
					 XAP_Dialog_MessageBox::b_YNC,
					 XAP_Dialog_MessageBox::a_YES,
					 pFrame->getNonDecoratedTitle());
	message->setSecondaryMessage(AP_STRING_ID_MSG_ConfirmSaveSecondary);
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
	// return a pointer a g_strdup()'d string containing the
	// pathname the user entered -- ownership of this goes
	// to the caller (so g_free it when you're done with it).

	UT_DEBUGMSG(("s_AskForPathname: frame %p, bSaveAs %d, suggest=[%s]\n",
				 pFrame,bSaveAs,((pSuggestedName) ? pSuggestedName : "")));

	UT_return_val_if_fail (ppPathname, false);
	*ppPathname = NULL;

	if (pFrame) {
		pFrame->raise();
	}
	
	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog
		= static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(id));
	UT_return_val_if_fail (pDialog, false);

	if (pSuggestedName && *pSuggestedName)
	{
		// if caller wants to suggest a name, use it and seed the
		// dialog in that directory and set the filename.
		pDialog->setCurrentPathname(pSuggestedName);
		pDialog->setSuggestFilename(true);
	}
	else if (pFrame)
	{
		// if caller does not want to suggest a name, seed the dialog
		// to the directory containing this document (if it has a
		// name), but don't put anything in the filename portion.
		PD_Document * pDoc = static_cast<PD_Document*>(pFrame->getCurrentDoc());
		std::string title;

		if (pDoc->getMetaDataProp (PD_META_KEY_TITLE, title) && !title.empty())
		{
#if 0
			// the metadata is returned to us in utf8; we have to convert it to whatever
			// the c-lib library uses
			const char * encoding;
			bool bSet = false;
			
			if(g_ascii_strcasecmp(l.getEncoding(), "UTF-8") != 0)
			{
				UT_iconv_t  cd = UT_iconv_open(l.getEncoding(), "UTF-8");

				if(UT_iconv_isValid(cd));
				{
					const char * pTitle = title.c_str();
					int bytes = title.size();
					int left;
					char out[500];
					char *out_ptr = out;
					int res = UT_iconv(cd, &pTitle, &bytes, &out,&left);
					if (res != (size_t) -1 && bytes == 0)
					{
						out[500 - outbytes] = '\0';
						pDialog->setCurrentPathname(out);
						bSet = true;
					}
				}
			}

			if(!bSet)
				pDialog->setCurrentPathname(title.c_str());
#else
			UT_legalizeFileName(title);
			pDialog->setCurrentPathname(title.c_str());
#endif
			pDialog->setSuggestFilename(true);
		} else {
			pDialog->setCurrentPathname(pFrame->getFilename());
			pDialog->setSuggestFilename(false);
		}
	}
	else {
		// we don't have a frame. This is likely that we are going to open
		// so don't need to suggest a name.
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

	const char ** szDescList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	UT_return_val_if_fail(szDescList, false);

	const char ** szSuffixList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	if(!szSuffixList)
	{
		UT_ASSERT_HARMLESS(szSuffixList);
		FREEP(szDescList);
		return false;
	}

	IEFileType * nTypeList = static_cast<IEFileType *>(UT_calloc(filterCount + 1, sizeof(IEFileType)));
	if(!nTypeList)
	{
		UT_ASSERT_HARMLESS(nTypeList);
		FREEP(szDescList);
		FREEP(szSuffixList);
		return false;
	}

	UT_uint32 k = 0;

	if (bSaveAs)
		while (IE_Exp::enumerateDlgLabels(k, &szDescList[k], &szSuffixList[k], &nTypeList[k]))
			k++;
	else
		while (IE_Imp::enumerateDlgLabels(k, &szDescList[k], &szSuffixList[k], &nTypeList[k]))
			k++;

	pDialog->setFileTypeList(szDescList, szSuffixList, static_cast<const UT_sint32 *>(nTypeList));

	// AbiWord uses IEFT_AbiWord_1 as the default

	// try to remember the previous file type
	static IEFileType dflFileType = IEFT_Bogus;

	// if a file format was given to us, then use that
	if (ieft != NULL && *ieft != IEFT_Bogus)
	  {
		// have a pre-existing file format, try to default to that
		UT_DEBUGMSG(("DOM: using given filetype %d\n", *ieft));
		dflFileType = *ieft;
	  }
	else if (bSaveAs)
	  {
		XAP_App * pApp = XAP_App::getApp();
		if(!pApp)
		{
			UT_ASSERT_HARMLESS(pApp);
			FREEP(szDescList);
			FREEP(szSuffixList);
			FREEP(nTypeList);
			return false;
		}

		XAP_Prefs * pPrefs = pApp->getPrefs();
		if(!pPrefs)
		{
			UT_ASSERT_HARMLESS(pPrefs);
			FREEP(szDescList);
			FREEP(szSuffixList);
			FREEP(nTypeList);
			return false;
		}

		const gchar * ftype = 0;

		// fetch the default save format
		pPrefs->getPrefsValue (static_cast<const gchar *>(AP_PREF_KEY_DefaultSaveFormat), &ftype, true);
		if (ftype)
		{
			// load the default file format
			dflFileType = IE_Exp::fileTypeForSuffix (ftype);
			UT_DEBUGMSG(("DOM: reverting to default file type: %s (%d)\n", ftype, dflFileType));
		}
		else
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
	  }
	else
	  {
		// try to load ABW by default
		dflFileType = IE_Imp::fileTypeForSuffix (".abw");
	  }

	pDialog->setDefaultFileType(dflFileType);
	UT_DEBUGMSG(("About to runModal on FileOpen \n"));
	pDialog->runModal(pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const std::string & resultPathname = pDialog->getPathname();
		if (!resultPathname.empty()) {
			*ppPathname = g_strdup(resultPathname.c_str());
		}

		UT_sint32 type = pDialog->getFileType();
		dflFileType = type;

		// If the number is negative, it's a special type.
		// Some operating systems which depend solely on filename
		// suffixes to identify type (like Windows) will always
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
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}
		else
			*ieft = static_cast<IEFileType>(pDialog->getFileType());
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
	// return a pointer a g_strdup()'d string containing the
	// pathname the user entered -- ownership of this goes
	// to the caller (so g_free it when you're done with it).

	UT_DEBUGMSG(("s_AskForGraphicPathname: frame %p\n",
				 pFrame));

	UT_return_val_if_fail (ppPathname, false);
	*ppPathname = NULL;

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog
		= static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_INSERT_PICTURE));
	UT_return_val_if_fail (pDialog, false);

	pDialog->setCurrentPathname(NULL);
	pDialog->setSuggestFilename(false);

	// to fill the file types popup list, we need to convert AP-level
	// ImpGraphic descriptions, suffixes, and types into strings.

	UT_uint32 filterCount = IE_ImpGraphic::getImporterCount();

	const char ** szDescList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	UT_return_val_if_fail(szDescList, false);

	const char ** szSuffixList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	if(!szSuffixList)
	{
		UT_ASSERT_HARMLESS(szSuffixList);
		FREEP(szDescList);
		return false;
	}

	IEGraphicFileType * nTypeList = (IEGraphicFileType *)
		 UT_calloc(filterCount + 1,	sizeof(IEGraphicFileType));
	if(!nTypeList)
	{
		UT_ASSERT_HARMLESS(nTypeList);
		FREEP(szDescList);
		FREEP(szSuffixList);
		return false;
	}

	UT_uint32 k = 0;

	while (IE_ImpGraphic::enumerateDlgLabels(k, &szDescList[k], &szSuffixList[k], &nTypeList[k]))
		k++;

	pDialog->setFileTypeList(szDescList, szSuffixList, static_cast<const UT_sint32 *>(nTypeList));
	if (iegft != NULL)
	  pDialog->setDefaultFileType(*iegft);
	pDialog->runModal(pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const std::string & resultPathname = pDialog->getPathname();
		if (!resultPathname.empty()) {
			*ppPathname = g_strdup(resultPathname.c_str());
		}

		UT_sint32 type = pDialog->getFileType();

		// If the number is negative, it's a special type.
		// Some operating systems which depend solely on filename
		// suffixes to identify type (like Windows) will always
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
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}
		else
			*iegft = static_cast<IEGraphicFileType>(pDialog->getFileType());
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
	  case UT_IE_FILENOTFOUND:
		String_id = AP_STRING_ID_MSG_IE_FileNotFound;
		break;

	  case UT_IE_NOMEMORY:
		String_id = AP_STRING_ID_MSG_IE_NoMemory;
		break;

	  case UT_IE_UNKNOWNTYPE:
		String_id = AP_STRING_ID_MSG_IE_UnsupportedType;
		//AP_STRING_ID_MSG_IE_UnknownType;
		break;

	  case UT_IE_BOGUSDOCUMENT:
		String_id = AP_STRING_ID_MSG_IE_BogusDocument;
		break;

	  case UT_IE_COULDNOTOPEN:
		String_id = AP_STRING_ID_MSG_IE_CouldNotOpen;
		break;

	  case UT_IE_COULDNOTWRITE:
		String_id = AP_STRING_ID_MSG_IE_CouldNotWrite;
		break;

	  case UT_IE_FAKETYPE:
		String_id = AP_STRING_ID_MSG_IE_FakeType;
		break;

	  case UT_IE_UNSUPTYPE:
		String_id = AP_STRING_ID_MSG_IE_UnsupportedType;
		break;

      case UT_IE_TRY_RECOVER:
		String_id = AP_STRING_ID_MSG_OpenRecovered;
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
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, UT_ERROR);

	XAP_Frame * pNewFrame = NULL;
	// not needed bool bRes = false;
	UT_Error errorCode = UT_IE_IMPORTERROR;

	// see if requested file is already open in another frame
	UT_sint32 ndx = pApp->findFrame(pNewFile);
	if (ndx >= 0)
	{
		// yep, reuse it
		pNewFrame = pApp->getFrame(ndx);
		UT_return_val_if_fail (pNewFrame, UT_ERROR);

		if (s_AskRevertFile(pNewFrame))
		{
			// re-load the document in pNewFrame
			s_StartStopLoadingCursor( true,pNewFrame);
			errorCode = pNewFrame->loadDocument(pNewFile, ieft);
			if (UT_IS_IE_SUCCESS(errorCode))
			{
				pNewFrame->show();
			}
			// even UT_IE_TRY_RECORVER
			if (errorCode)
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

	// For widgetized AbiWord, if there is a prexisting document in the 
	// Frame, we save it then open a the new document in the same frame

	if(pFrame)
	{
		AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());		
		if(pFrameData && pFrameData->m_bIsWidget)
		 {
			 if(pFrame->isDirty())
			 {
				 AV_View * pAV_View = pFrame->getCurrentView();
				 EV_EditMethodCallData * pCallData = NULL;
				 EX(saveImmediate);
			 }


			 s_StartStopLoadingCursor( true,pFrame);
			 errorCode = pFrame->loadDocument(pNewFile, ieft);
			 if (UT_IS_IE_SUCCESS(errorCode))
			 {
				 pFrame->updateZoom();
				 pFrame->show();
			 }
			 if (errorCode)
			 {
				 s_CouldNotLoadFileMessage(pFrame,pNewFile, errorCode);
			 }
			 s_StartStopLoadingCursor( false,NULL);
			 return errorCode;
		 } 
	}

	// We generally open documents in a new frame, which keeps the
	// contents of the current frame available.
	// However, as a convenience we do replace the contents of the
	// current frame if it's the only top-level view on an empty,
	// untitled document.

	if ((pFrame == NULL) || pFrame->isDirty() || pFrame->getFilename() || (pFrame->getViewNumber() > 0))
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

		errorCode = pNewFrame->loadDocument((const char *)NULL, IEFT_Unknown);
		if (UT_IS_IE_SUCCESS(errorCode))
		{
			pNewFrame->show();
		}
	    else
		{
			return false;
		}


		s_StartStopLoadingCursor( true,pNewFrame);
		errorCode = pNewFrame->loadDocument(pNewFile, ieft);
		if (UT_IS_IE_SUCCESS(errorCode))
		{
			pNewFrame->show();
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
			errorCode = pNewFrame->loadDocument((const char *)NULL, IEFT_Unknown);
			if (UT_IS_IE_SUCCESS(errorCode)) {
				pNewFrame->updateZoom();
				pNewFrame->show();
			}
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
	if (UT_IS_IE_SUCCESS(errorCode))
	{
		pFrame->updateZoom();
		pFrame->show();
	}
	if (errorCode)
	{
		s_CouldNotLoadFileMessage(pFrame,pNewFile, errorCode);
	}
	s_StartStopLoadingCursor( false,NULL);
	return errorCode;
}

#define ABIWORD_VIEW	FV_View * pView = static_cast<FV_View *>(pAV_View);

Defun1(importStyles)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_return_val_if_fail(pFrame,false);

	UT_Error error = UT_IE_IMPORTERROR;
	char * pFile = NULL;
	IEFileType ieft = IEFT_Unknown;
	bool bOK = s_AskForPathname(pFrame,false, XAP_DIALOG_ID_FILE_OPEN, NULL,&pFile,&ieft);

	if (!bOK || !pFile)
	  return false;

	PD_Document * pDoc = static_cast<PD_Document *>(pFrame->getCurrentDoc());

	UT_return_val_if_fail(pDoc,false);

	error = pDoc->importStyles(pFile,ieft);

	return E2B(error);
}


Defun1(fileOpen)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = NULL;
	IEFileType ieft = IEFT_Unknown;
	if (pAV_View) {
		pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
		UT_return_val_if_fail (pFrame, false);
		ieft = static_cast<PD_Document *>(pFrame->getCurrentDoc())->getLastOpenedType();
	}
	char * pNewFile = NULL;
	bool bOK = s_AskForPathname(pFrame,false, XAP_DIALOG_ID_FILE_OPEN, NULL,&pNewFile,&ieft);

	if (!bOK || !pNewFile)
	  return false;

	// we own storage for pNewFile and must g_free it.

	UT_Error error = ::fileOpen(pFrame, pNewFile, ieft);

	g_free(pNewFile);
	return E2B(error);
}

static UT_Error
s_importFile (XAP_Frame * pFrame, const char * pNewFile, IEFileType ieft)
{
	UT_DEBUGMSG(("fileOpen: loading [%s]\n",pNewFile));
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, UT_ERROR);
	XAP_Frame * pNewFrame = NULL;
	// not needed bool bRes = false;
	UT_Error errorCode = UT_IE_IMPORTERROR;

	// We open documents in a new frame, which keeps the
	// contents of the current frame available.
	// However, as a convenience we do replace the contents of the
	// current frame if it's the only top-level view on an empty,
	// untitled document.

	if ((pFrame == NULL) || pFrame->isDirty() || pFrame->getFilename() || (pFrame->getViewNumber() > 0))
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
			errorCode = pNewFrame->loadDocument((const char *)NULL, IEFT_Unknown);
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
	if (UT_IS_IE_SUCCESS(errorCode))
	{
		pFrame->show(); // don't add to the MRU
	}
	if (errorCode)
	{
		s_CouldNotLoadFileMessage(pFrame,pNewFile, errorCode);
	}
	s_StartStopLoadingCursor( false,NULL);
	return errorCode;
}

Defun1(openTemplate)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail (pFrame, false);

	char * pNewFile = NULL;
	IEFileType ieft = static_cast<PD_Document *>(pFrame->getCurrentDoc())->getLastOpenedType();
	bool bOK = s_AskForPathname(pFrame,false, XAP_DIALOG_ID_FILE_IMPORT, NULL,&pNewFile,&ieft);

	if (!bOK || !pNewFile)
	  return false;

	// we own storage for pNewFile and must g_free it.

	UT_Error error = s_importFile(pFrame, pNewFile, ieft);

	g_free(pNewFile);
	return E2B(error);
}

Defun(saveImmediate)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_return_val_if_fail (pFrame, false);
	//
	// If we're connected let the remote document know.
	// We do this with the savedoc signal
	//
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	if(pView)
	{
		PD_Document * pDoc = pView->getDocument();
		if(pDoc && pDoc->isConnected())
		{
			pDoc->signalListeners(PD_SIGNAL_SAVEDOC);
			if (pFrame->getViewNumber() > 0)
			{
				XAP_App * pApp = XAP_App::getApp();
				UT_return_val_if_fail (pApp, false);

				pApp->updateClones(pFrame);
			}
			if(!pDoc->isDirty())
				return true;
		}
	}
	// can only save without prompting if filename already known

	if (!pFrame->getFilename())
   		return EX(fileSaveAs);

	UT_Error errSaved;
	errSaved = pAV_View->cmdSave();
	
	// if it has a problematic extension save as instead
	//	if (errSaved == UT_EXTENSIONERROR)
	//  return EX(fileSaveAs);

	if (errSaved)
	{
		// throw up a dialog
		s_TellSaveFailed(pFrame, pFrame->getFilename(), errSaved);
		return false;
	}

	if (pFrame->getViewNumber() > 0)
	{
		XAP_App * pApp = XAP_App::getApp();
		UT_return_val_if_fail (pApp, false);

		pApp->updateClones(pFrame);
	}

	return true;
}

Defun(fileSave)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_return_val_if_fail (pFrame, false);
	//
	// If we're connected let the remote document know
	// We do this with the savedoc signal
	//
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	if(pView)
	{
		PD_Document * pDoc = pView->getDocument();
		if(pDoc && pDoc->isConnected())
		{
			pDoc->signalListeners(PD_SIGNAL_SAVEDOC);
			if (pFrame->getViewNumber() > 0)
			{
				XAP_App * pApp = XAP_App::getApp();
				UT_return_val_if_fail (pApp, false);

				pApp->updateClones(pFrame);
			}
			if(!pDoc->isDirty())
				return true;
		}
	}
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
		XAP_App * pApp = XAP_App::getApp();
		UT_return_val_if_fail (pApp, false);

		pApp->updateClones(pFrame);
	}

	return true;
}

static bool
s_actuallySaveAs(AV_View * pAV_View, bool overwriteName)
{
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail (pFrame, false);

	IEFileType ieft = IEFT_Bogus; // IEFT_Bogus will let the file dialog fall back to the default format

	//ieft = static_cast<PD_Document *>(pFrame->getCurrentDoc())->getLastSavedAsType();

	char * pNewFile = NULL;
	XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_SAVEAS;

	if ( !overwriteName )
	  id = XAP_DIALOG_ID_FILE_EXPORT;

	bool bOK = s_AskForPathname(pFrame,true, id, pFrame->getFilename(),&pNewFile,&ieft);

	if (!bOK || !pNewFile)
		return false;

	UT_DEBUGMSG(("fileSaveAs: saving as [%s]\n",pNewFile));

	UT_Error errSaved;
	errSaved = pAV_View->cmdSaveAs(pNewFile, static_cast<int>(ieft), overwriteName);
	if (errSaved)
	{
		// throw up a dialog
		s_TellSaveFailed(pFrame, pNewFile, errSaved);
		g_free(pNewFile);
		return false;
	}

	g_free(pNewFile);

	// ignore all of this stuff
	if (!overwriteName)
		return bOK;

	// update the MRU list
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, false);

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
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail (pFrame, false);

	char * pNewFile = NULL;
	IEFileType ieft = static_cast<PD_Document *>(pFrame->getCurrentDoc())->getLastOpenedType();
	bool bOK = s_AskForPathname(pFrame,false, XAP_DIALOG_ID_FILE_IMPORT, NULL,&pNewFile,&ieft);

	if (!bOK || !pNewFile)
	  return false;

	// we own storage for pNewFile and must g_free it.

	UT_Error error = s_importFile(pFrame, pNewFile, ieft);

	g_free(pNewFile);
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

  UT_return_val_if_fail (pAV_View, false);
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
  UT_return_val_if_fail (pFrame, false);

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
  errSaved = pAV_View->cmdSaveAs(pNewFile, static_cast<int>(ieft), false);
  if (errSaved)
    {
      // throw up a dialog
      s_TellSaveFailed(pFrame, pNewFile, errSaved);
      g_free(pNewFile);
      return false;
    }

  return bOK;
}

Defun1(fileSaveAsWeb)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
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
	  g_free(pNewFile);
	  return false;
	}

  return true;
}

Defun1(fileSaveImage)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog
		= static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_FILE_SAVE_IMAGE));
	UT_return_val_if_fail (pDialog, false);

	UT_uint32 filterCount = 1;
	const char ** szDescList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	UT_return_val_if_fail(szDescList, false);

	const char ** szSuffixList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	if(!szSuffixList)
	{
		UT_ASSERT_HARMLESS(szSuffixList);
		FREEP(szDescList);
		return false;
	}

	IEFileType * nTypeList = static_cast<IEFileType *>(UT_calloc(filterCount + 1, sizeof(IEFileType)));
	if(!nTypeList)
	{
		UT_ASSERT_HARMLESS(nTypeList);
		FREEP(szDescList);
		FREEP(szSuffixList);
		return false;
	}

	// we only support saving images in png format for now
	szDescList[0] = "Portable Network Graphics (.png)";
	szSuffixList[0] = "*.png";
	nTypeList[0] = static_cast<IEFileType>(1);

	pDialog->setFileTypeList(szDescList, szSuffixList,
							 static_cast<const UT_sint32 *>(nTypeList));

	pDialog->setDefaultFileType(static_cast<IEFileType>(1));

	pDialog->runModal(pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const std::string resultPathname = pDialog->getPathname();
		if (!resultPathname.empty()) {
			FV_View * pView = static_cast<FV_View *>(pAV_View);
			pView->saveSelectedImage (resultPathname.c_str());
		}
	}

	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);

	pDialogFactory->releaseDialog(pDialog);

	return true;
}


Defun1(fileSaveEmbed)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	FV_View * pView = static_cast<FV_View *>(pAV_View);
	fp_EmbedRun *pRun = dynamic_cast <fp_EmbedRun*> (pView->getSelectedObject ());
	UT_return_val_if_fail(pRun, false);

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog
		= static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_FILE_SAVEAS));
	UT_return_val_if_fail (pDialog, false);

	UT_uint32 filterCount = 1;
	const char ** szDescList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	UT_return_val_if_fail(szDescList, false);

	const char ** szSuffixList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	if(!szSuffixList)
	{
		UT_ASSERT_HARMLESS(szSuffixList);
		FREEP(szDescList);
		return false;
	}

	IEFileType * nTypeList = static_cast<IEFileType *>(UT_calloc(filterCount + 1, sizeof(IEFileType)));
	if(!nTypeList)
	{
		UT_ASSERT_HARMLESS(nTypeList);
		FREEP(szDescList);
		FREEP(szSuffixList);
		return false;
	}

	// we only support saving objects to their default format
	szDescList[0] =  pRun->getEmbedManager()->getMimeTypeDescription();
	szSuffixList[0] = pRun->getEmbedManager()->getMimeTypeSuffix();
	nTypeList[0] = static_cast<IEFileType>(1);

	pDialog->setFileTypeList(szDescList, szSuffixList,
							 static_cast<const UT_sint32 *>(nTypeList));

	pDialog->setDefaultFileType(static_cast<IEFileType>(1));

	pDialog->runModal(pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const std::string & resultPathname = pDialog->getPathname();
		if (!resultPathname.empty()) {
			const UT_ByteBuf *Buf;
			pView->getDocument()->getDataItemDataByName(pRun->getDataID(), &Buf, NULL, NULL);
			if (Buf)
				Buf->writeToURI(resultPathname.c_str());
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
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pAV_View->getParentData());
	
	std::string file = UT_createTmpFile("web", ".html");

	UT_Error errSaved = UT_OK;

	// we do this because we don't want to change the default
	// document extension or rename what we're working on
	char *uri = UT_go_filename_to_uri(file.c_str());
	if(uri)
	{
		if(XAP_App::getApp()->getPrefs())
			XAP_App::getApp()->getPrefs()->setIgnoreNextRecent();

		errSaved = pAV_View->cmdSaveAs(uri, IE_Exp::fileTypeForSuffix(".xhtml"), false);
	}
	else
		errSaved = UT_IE_COULDNOTWRITE;

	if(errSaved != UT_OK)
	{
		// throw up a dialog
		s_TellSaveFailed(pFrame, file.c_str(), errSaved);
		return false;
	}

	bool bOk = _openURL(uri);
	g_free(uri);

	return bOk;
}

Defun1(undo)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	pAV_View->cmdUndo(1);
	return true;
}

Defun1(redo)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	pAV_View->cmdRedo(1);
	return true;
}

Defun1(newWindow)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
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

static bool _openRecent(AV_View* pAV_View, UT_sint32 ndx)
{
	XAP_Frame * pFrame = NULL;
	if (pAV_View) {
		pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
		UT_return_val_if_fail(pFrame, false);
	}

	XAP_Prefs * pPrefs = XAP_App::getApp()->getPrefs();
	UT_return_val_if_fail (pPrefs, false);

	UT_return_val_if_fail (ndx > 0 && ndx <= pPrefs->getRecentCount(), false);

	const char * szRecent = pPrefs->getRecent(ndx);

	// TODO HACK BROKEN BUSTED BLAH WARNING NOTE ERROR
	// BROKEN: We should store some sort of file type with the MRU data
	// BROKEN: or we don't know what to open it as!  We can't assume
	// BROKEN: IEFT_Unknown will detect what the user saved it as,
	// BROKEN: since the user can explictly export as any type.
	// TODO HACK BROKEN BUSTED BLAH WARNING NOTE ERROR

	UT_Error error = fileOpen(pFrame, szRecent, IEFT_Unknown);

    if (!UT_IS_IE_SUCCESS(error))
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

static bool _activateWindow(AV_View* pAV_View, UT_sint32 ndx)
{
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, false);

	UT_return_val_if_fail (ndx > 0 && ndx <= pApp->getFrameCount(), false);


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
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_WindowMore * pDialog
		= static_cast<XAP_Dialog_WindowMore *>(pDialogFactory->requestDialog(id));
	UT_return_val_if_fail (pDialog, false);

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
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	s_doMoreWindowsDlg(pFrame, XAP_DIALOG_ID_WINDOWMORE);
	return true;
}

static bool s_doAboutDlg(XAP_Frame* pFrame, XAP_Dialog_Id id)
{
	if (pFrame) {
		pFrame->raise();
	}
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, false);

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pApp->getDialogFactory());

	XAP_Dialog_About * pDialog
		= static_cast<XAP_Dialog_About *>(pDialogFactory->requestDialog(id));
	UT_return_val_if_fail (pDialog, false);

	// run the dialog (it should really be modeless if anyone
	// gets the urge to make it safe that way)
	pDialog->runModal(pFrame);

	bool bOK = true;

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

static bool s_doToggleCase(XAP_Frame * pFrame, FV_View * pView, XAP_Dialog_Id id)
{
	UT_return_val_if_fail(pFrame, false);

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
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_ToggleCase * pDialog
		= static_cast<AP_Dialog_ToggleCase *>(pDialogFactory->requestDialog(id));
	UT_return_val_if_fail (pDialog, false);

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
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	return s_doToggleCase(pFrame, static_cast<FV_View *>(pAV_View), AP_DIALOG_ID_TOGGLECASE);
}

Defun1(rotateCase)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	FV_View * pView = static_cast<FV_View *>(pAV_View);
	pView->toggleCase(CASE_ROTATE);

	return true;
}

Defun1(dlgAbout)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = NULL;
	
	if (pAV_View) {
		pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
		UT_return_val_if_fail(pFrame, false);
	}
	
	s_doAboutDlg(pFrame, XAP_DIALOG_ID_ABOUT);

	return true;
}

Defun1(dlgMetaData)
{
  CHECK_FRAME;
  UT_return_val_if_fail (pAV_View, false);
  FV_View * pView = static_cast<FV_View *>(pAV_View);

  XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
  UT_return_val_if_fail(pFrame, false);

  XAP_App * pApp = XAP_App::getApp();
  UT_return_val_if_fail (pApp, false);

  pFrame->raise();

  XAP_DialogFactory * pDialogFactory
    = static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

  AP_Dialog_MetaData * pDialog
    = static_cast<AP_Dialog_MetaData *>(pDialogFactory->requestDialog(AP_DIALOG_ID_METADATA));
  UT_return_val_if_fail (pDialog, false);

  // get the properties

  PD_Document * pDocument = pView->getDocument();

  std::string prop;

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

	  for(UT_sint32 i = 0;i < pApp->getFrameCount();++i)
	  {
		  pApp->getFrame(i)->updateTitle ();
	  }	  

      pDocument->forceDirty();
    }

  // release the dialog

  pDialogFactory->releaseDialog(pDialog);

  return true ;
}

Defun1(fileNewUsingTemplate)
{
	CHECK_FRAME;
	XAP_Frame * pFrame = NULL;
	if (pAV_View) {
		FV_View * pView = static_cast<FV_View *>(pAV_View);
	
		pFrame = static_cast<XAP_Frame *>(pView->getParentData());
		UT_return_val_if_fail(pFrame, false);
	
	
		pFrame->raise();
	}
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, false);

 	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pApp->getDialogFactory());

	AP_Dialog_New * pDialog
		= static_cast<AP_Dialog_New *>(pDialogFactory->requestDialog(AP_DIALOG_ID_FILE_NEW));
	UT_return_val_if_fail (pDialog, false);

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
		    bOK = s_importFile (pFrame, str.c_str(), IEFT_Unknown) == UT_OK;
		}
		else
		{
			// we want a new blank doc
			XAP_Frame * pNewFrame = pApp->newFrame();

			if (pNewFrame)
				pFrame = pNewFrame;

			bOK = pFrame->loadDocument((const char *)NULL, IEFT_Unknown) == UT_OK;

			if (pNewFrame)
			{
				pNewFrame->show();
			}
		}
	}

	pDialogFactory->releaseDialog(pDialog);
	return bOK;
}

static bool _helpOpenURL(const char* helpURL)
{
	return XAP_App::getApp()->openHelpURL(helpURL);
}

static bool _openURL(const char* url)
{
	return XAP_App::getApp()->openURL(url);
}
	
bool helpLocalizeAndOpenURL(const char* pathBeforeLang, const char* pathAfterLang, const char *remoteURLbase)
{
	UT_String url (XAP_App::getApp()->localizeHelpUrl (pathBeforeLang, pathAfterLang, remoteURLbase));
	return _helpOpenURL(url.c_str());
}

Defun0(helpContents)
{
	return helpLocalizeAndOpenURL("help", "index", "http://www.abisource.com/help/");
}

Defun0(helpIntro)
{
	return helpLocalizeAndOpenURL("help", "introduction", "http://www.abisource.com/help/");
}

Defun0(helpCheckVer)
{
	UT_String versionURL ("http://www.abisource.com/users/check_version.phtml?version=");
	versionURL += XAP_App::s_szBuild_Version;
	return _openURL(versionURL.c_str());
}

Defun0(helpReportBug)
{
	UT_String bugURL ("http://bugzilla.abisource.com/enter_bug.cgi?product=AbiWord");

  bugURL += "&version=";
  bugURL += XAP_App::s_szBuild_Version;
  bugURL += "&comment=(";
  bugURL += XAP_App::s_szBuild_Options;
  bugURL += ")%0d%0a%0d%0a";

  return _openURL(bugURL.c_str());
}

Defun0(helpSearch)
{
	return helpLocalizeAndOpenURL("help", "search", "http://www.abisource.com/help/");
}

Defun0(helpCredits)
{
	return helpLocalizeAndOpenURL("help", "credits", "http://www.abisource.com/help/");
}

Defun0(helpAboutGnomeOffice)
{
	return _openURL("http://live.gnome.org/GnomeOffice/");
}

Defun1(cycleWindows)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, false);

	UT_sint32 ndx = pApp->findFrame(pFrame);
	UT_return_val_if_fail (ndx >= 0, false);

	if (ndx < static_cast<UT_sint32>(pApp->getFrameCount()) - 1)
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
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, false);

	UT_sint32 ndx = pApp->findFrame(pFrame);
	UT_return_val_if_fail (ndx >= 0, false);

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
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);

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
	bool bRemoteSave = false;
	bool bRet = true;
	if ((pFrame->getViewNumber() == 0) &&
		(pFrame->isDirty()))
	{
		
		XAP_Dialog_MessageBox::tAnswer ans;

		ans = s_AskSaveFile(pFrame);

		switch (ans)
		{
		case XAP_Dialog_MessageBox::a_YES:				// save it first
		{
			//
			// If we're connected let the remote document know.
			// We do this with the savedoc signal
			//
			FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
			if(pView)
			{
				PD_Document * pDoc = pView->getDocument();
				if(pDoc && pDoc->isConnected())
				{
					pDoc->signalListeners(PD_SIGNAL_SAVEDOC);
				}
				bRemoteSave = pDoc->isDirty();
				UT_DEBUGMSG(("remote save %d\n", bRemoteSave));
			}
			if(bRemoteSave)
				bRet = EX(fileSave);
			if (!bRet)								// didn't successfully save,
				return false;					//	  so don't close
		}
		break;

		case XAP_Dialog_MessageBox::a_NO:				// just close it
			break;
			
		case XAP_Dialog_MessageBox::a_CANCEL:			// don't close it
			return false;
			
		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return false;
			
		}
	}

	// are we the last window?
	if (1 >= pApp->getFrameCount())
	{
		// Delete all the open modeless dialogs

		pApp->closeModelessDlgs();

		// in single XAPAPP mode we can't close the app when closing the last frame
		// or reopen a new one.
#if XAP_SINGLE_XAPAPP
        UT_UNUSED(bCanExit);
#else
		if (bCanExit)
		{
			pApp->reallyExit();
		}
		else
		{
			// keep the app open with an empty document (in this frame)
			pFrame->loadDocument((const char *)NULL, IEFT_Unknown);
			pFrame->updateZoom();
			pFrame->show();
			return true;
		}
#endif
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
#if !defined(TOOLKIT_GTK_ALL)
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);

	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);

	bool close = false;

	pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_CloseOnLastDoc), &close);
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
		
	XAP_Frame * pFrame = NULL;
	bool bRet = true;

	if (pAV_View) {
		pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
		UT_return_val_if_fail(pFrame, false);
	}
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);

	if (pFrame) {

#if XAP_DONT_CONFIRM_QUIT
#else
		if (1 < pApp->getFrameCount())
		{
			if (!s_AskCloseAllAndExit(pFrame))
			{
				// never mind
				return false;
			}
		}
#endif
	}
	if (pApp->getFrameCount()) {
		UT_uint32 ndx = pApp->getFrameCount();

		// loop over windows, but stop if one can't close
		while (bRet && ndx > 0)
		{
			XAP_Frame * f = pApp->getFrame(ndx - 1);
			UT_return_val_if_fail (f, false);
			pAV_View = f->getCurrentView();
			UT_return_val_if_fail (pAV_View, false);

			bRet = s_closeWindow (pAV_View, pCallData, true);

			ndx--;
		}
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

Defun1(fileRevert)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

  UT_return_val_if_fail (pAV_View, false);
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

	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_ClipArt * pDialog
		= static_cast<XAP_Dialog_ClipArt *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_CLIPART));
UT_return_val_if_fail(pDialog, false);
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
		FG_Graphic* pFG;

		UT_Error errorCode;

		errorCode = IE_ImpGraphic::loadGraphic(pNewFile, iegft, &pFG);
		if(errorCode)
		{
			s_CouldNotLoadFileMessage(pFrame, pNewFile, errorCode);
			goto Cleanup;
		}

		errorCode = pView->cmdInsertGraphic(pFG);
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
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	char* pNewFile = NULL;


	IEGraphicFileType iegft = IEGFT_Unknown;
	bool bOK = s_AskForGraphicPathname(pFrame,&pNewFile,&iegft);

	if (!bOK || !pNewFile)
		return false;

	// we own storage for pNewFile and must g_free it.
	UT_DEBUGMSG(("fileInsertGraphic: loading [%s]\n",pNewFile));

	FG_Graphic* pFG;

	UT_Error errorCode;

	errorCode = IE_ImpGraphic::loadGraphic(pNewFile, iegft, &pFG);
	if(errorCode != UT_OK || !pFG)
	  {
		s_CouldNotLoadFileMessage(pFrame, pNewFile, errorCode);
		FREEP(pNewFile);
		return false;
	  }

	ABIWORD_VIEW;

	errorCode = pView->cmdInsertGraphic(pFG);
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

Defun1(fileInsertPositionedGraphic)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	char* pNewFile = NULL;


	IEGraphicFileType iegft = IEGFT_Unknown;
	bool bOK = s_AskForGraphicPathname(pFrame,&pNewFile,&iegft);

	if (!bOK || !pNewFile)
		return false;

	// we own storage for pNewFile and must g_free it.
	UT_DEBUGMSG(("fileInsertGraphic: loading [%s]\n",pNewFile));

	FG_Graphic* pFG;

	UT_Error errorCode;

	errorCode = IE_ImpGraphic::loadGraphic(pNewFile, iegft, &pFG);
	if(errorCode != UT_OK || !pFG)
	  {
		s_CouldNotLoadFileMessage(pFrame, pNewFile, errorCode);
		FREEP(pNewFile);
		return false;
	  }

	ABIWORD_VIEW;
	errorCode = pView->cmdInsertPositionedGraphic(pFG);
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


Defun1(fileInsertPageBackgroundGraphic)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	char* pNewFile = NULL;


	IEGraphicFileType iegft = IEGFT_Unknown;
	bool bOK = s_AskForGraphicPathname(pFrame,&pNewFile,&iegft);

	if (!bOK || !pNewFile)
		return false;

	// we own storage for pNewFile and must g_free it.
	UT_DEBUGMSG(("fileInsertBackgroundGraphic: loading [%s]\n",pNewFile));

	FG_Graphic* pFG;

	UT_Error errorCode;

	errorCode = IE_ImpGraphic::loadGraphic(pNewFile, iegft, &pFG);
	if(errorCode != UT_OK || !pFG)
	  {
		s_CouldNotLoadFileMessage(pFrame, pNewFile, errorCode);
		FREEP(pNewFile);
		return false;
	  }

	ABIWORD_VIEW;
	fl_BlockLayout * pBlock = pView->getCurrentBlock();
	UT_return_val_if_fail( pBlock, false );
	fl_DocSectionLayout * pDSL = pBlock->getDocSectionLayout();
	UT_return_val_if_fail( pDSL, false );
	PT_DocPosition iPos = pDSL->getPosition();
	errorCode = pView->cmdInsertGraphicAtStrux(pFG, iPos, PTX_Section);

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
	
	UT_return_val_if_fail (pView, false);
	// check if the run to select is a fp_ImageRun. If, so, don't move the view
	PT_DocPosition pos = pView->getDocPositionFromXY(pCallData->m_xPos, pCallData->m_yPos);
	fl_BlockLayout * pBlock = pView->getBlockAtPosition(pos);
	if(pBlock)
	{
		UT_sint32 x1,x2,y1,y2,iHeight;
		bool bEOL = false;
		bool bDir = false;
		
		fp_Run * pRun = NULL;
		
		pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,iHeight,bDir);
		while(pRun && ((pRun->getType() != FPRUN_IMAGE) && (pRun->getType() != FPRUN_EMBED)))
		{
			pRun = pRun->getNextRun();
		}
		if(pRun && ((pRun->getType() == FPRUN_IMAGE) || ((pRun->getType() == FPRUN_EMBED))))
		{
			// we've found an image: do not move the view, just select the image and exit
			pView->cmdSelect(pos,pos+1);
			// Set the cursor context to image selected.
			pView->getMouseContext(pCallData->m_xPos, pCallData->m_yPos);
			return true;
		}
		else
		{
			// do nothing...
		}
	}
	
	// no, the run is something else (ie. not a fp_ImageRun)
	pView->warpInsPtToXY(pCallData->m_xPos, pCallData->m_yPos, true);
	pView->extSelHorizontal(true, 1); // move point forward one
	
	return true;
}

Defun(warpInsPtToXY)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->warpInsPtToXY(pCallData->m_xPos, pCallData->m_yPos, true);

	return true;
}

static void sActualMoveLeft(AV_View *  pAV_View, EV_EditMethodCallData * /*pCallData*/)
{
	ABIWORD_VIEW;
	UT_return_if_fail (pView);
	bool bRTL = false;
	fl_BlockLayout * pBL = pView->getCurrentBlock();
	if(pBL)
		bRTL = pBL->getDominantDirection() == UT_BIDI_RTL;
	
	pView->cmdCharMotion(bRTL,1);
	if(pView->getGraphics() && pView->getGraphics()->allCarets()->getBaseCaret())
	{
//
// Draw fsking caret for sure!!!
//
		pView->getGraphics()->allCarets()->getBaseCaret()->forceDraw();
	}
}

Defun1(warpInsPtLeft)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
//
// Do this operation in an idle loop so when can reject queued events
//
//
// This code sets things up to handle the warp right in an idle loop.
//
	UT_return_val_if_fail (pView, false);
	int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	_Freq * pFreq = new _Freq(pView,NULL,sActualMoveLeft);
	s_pFrequentRepeat = UT_WorkerFactory::static_constructor (_sFrequentRepeat,pFreq, inMode, outMode);

	UT_ASSERT(s_pFrequentRepeat);
	UT_ASSERT(outMode != UT_WorkerFactory::NONE);

	// If the worker is working on a timer instead of in the idle
	// time, set the frequency of the checks.
	if ( UT_WorkerFactory::TIMER == outMode )
	{
		// this is really a timer, so it's safe to static_cast it
		static_cast<UT_Timer*>(s_pFrequentRepeat)->set(50);
	}
	s_pFrequentRepeat->start();
	return true;
}

static void sActualMoveRight(AV_View *  pAV_View, EV_EditMethodCallData * /*pCallData*/)
{
	ABIWORD_VIEW;
	UT_return_if_fail (pView);
	bool bRTL = false;
	fl_BlockLayout * pBL = pView->getCurrentBlock();
	if(pBL)
		bRTL = pBL->getDominantDirection() == UT_BIDI_RTL;
	
	pView->cmdCharMotion(!bRTL,1);
	if(pView->getGraphics() && pView->getGraphics()->allCarets()->getBaseCaret())
	{
//
// Draw the fsking caret for sure!!!
//
		pView->getGraphics()->allCarets()->getBaseCaret()->forceDraw();
	}
	return;
}

Defun1(warpInsPtRight)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
//
// Do this operation in an idle loop so when can reject queued events
//
//
// This code sets things up to handle the warp right in an idle loop.
//
	UT_return_val_if_fail (pView, false);
	int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	_Freq * pFreq = new _Freq(pView,NULL,sActualMoveRight);
	s_pFrequentRepeat = UT_WorkerFactory::static_constructor (_sFrequentRepeat,pFreq, inMode, outMode);

	UT_ASSERT(s_pFrequentRepeat);
	UT_ASSERT(outMode != UT_WorkerFactory::NONE);

	// If the worker is working on a timer instead of in the idle
	// time, set the frequency of the checks.
	if ( UT_WorkerFactory::TIMER == outMode )
	{
		// this is really a timer, so it's safe to static_cast it
		static_cast<UT_Timer*>(s_pFrequentRepeat)->set(50);
	}
	s_pFrequentRepeat->start();
	return true;
}

Defun1(warpInsPtBOP)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->moveInsPtTo(FV_DOCPOS_BOP);
	return true;
}

Defun1(warpInsPtEOP)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->moveInsPtTo(FV_DOCPOS_EOP);
	return true;
}

Defun1(warpInsPtBOL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->moveInsPtTo(FV_DOCPOS_BOL);
	return true;
}

Defun1(warpInsPtEOL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->moveInsPtTo(FV_DOCPOS_EOL);
	return true;
}

Defun1(warpInsPtBOW)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	bool bRTL = false;
	fl_BlockLayout * pBL = pView->getCurrentBlock();
	if(pBL)
		bRTL = pBL->getDominantDirection() == UT_BIDI_RTL;
	

	if(bRTL)
		pView->moveInsPtTo(FV_DOCPOS_EOW_MOVE);
	else
		pView->moveInsPtTo(FV_DOCPOS_BOW);

	return true;
}

Defun1(warpInsPtEOW)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	bool bRTL = false;
	fl_BlockLayout * pBL = pView->getCurrentBlock();
	if(pBL)
		bRTL = pBL->getDominantDirection() == UT_BIDI_RTL;
	

	if(bRTL)
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
	UT_return_val_if_fail (pView, false);
	pView->moveInsPtTo(FV_DOCPOS_BOB);
	return true;
}

Defun1(warpInsPtEOB)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->moveInsPtTo(FV_DOCPOS_EOB);
	return true;
}

Defun1(warpInsPtBOD)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->moveInsPtTo(FV_DOCPOS_BOD);
	return true;
}

Defun1(warpInsPtEOD)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
//
// This is called on cntrl-End. If called from within a footnote/endnote
// jump back to just after the insertion point
//
	UT_return_val_if_fail (pView, false);
	if(pView->isInFootnote())
	{
		fl_FootnoteLayout * pFL = pView->getClosestFootnote(pView->getPoint());
		PT_DocPosition pos = pFL->getDocPosition() + pFL->getLength();
		pView->setPoint(pos);
		pView->ensureInsertionPointOnScreen();
		return true;
	}
	if(pView->isInEndnote())
	{
		fl_EndnoteLayout * pEL = pView->getClosestEndnote(pView->getPoint());
		PT_DocPosition pos = pEL->getDocPosition() + pEL->getLength();
		pView->setPoint(pos);
		pView->ensureInsertionPointOnScreen();
		return true;
	}

	pView->moveInsPtTo(FV_DOCPOS_EOD);
	return true;
}

Defun1(warpInsPtPrevPage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->warpInsPtNextPrevPage(false);
	return true;
}

Defun1(warpInsPtNextPage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->warpInsPtNextPrevPage(true);
	return true;
}

Defun1(warpInsPtPrevScreen)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->warpInsPtNextPrevScreen(false);
	return true;
}

Defun1(warpInsPtNextScreen)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->warpInsPtNextPrevScreen(true);
	return true;
}

Defun1(warpInsPtPrevLine)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
//
// Finish handling current expose before doing the next movement
//
	UT_return_val_if_fail (pView, false);
	pView->warpInsPtNextPrevLine(false);
	if(pView->getGraphics() && pView->getGraphics()->allCarets()->getBaseCaret())
	{
//
// Draw fsking caret for sure!!!
//
		pView->getGraphics()->allCarets()->getBaseCaret()->forceDraw();
	}
	return true;
}

Defun1(warpInsPtNextLine)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
//
// Finish handling current expose before doing the next movement
//
	UT_return_val_if_fail (pView, false);
	pView->warpInsPtNextPrevLine(true);
	if(pView->getGraphics() && pView->getGraphics()->allCarets()->getBaseCaret())
	{
//
// Draw fsking caret for sure!!!
//
		pView->getGraphics()->allCarets()->getBaseCaret()->forceDraw();
	}
	return true;
}

/*****************************************************************/

Defun1(cursorDefault)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	UT_return_val_if_fail (pView, false);
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
	xxx_UT_DEBUGMSG((" CursorIBeam \n"));
	// clear status bar of any lingering messages
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if (pG)
	{
		pG->setCursor(GR_Graphics::GR_CURSOR_IBEAM);
	}
	static_cast<AV_View *>(pView)->notifyListeners(AV_CHG_MOUSEPOS);
	return true;
}

Defun1(cursorTOC)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if (pG)
	{
		pG->setCursor(GR_Graphics::GR_CURSOR_LINK);
	}
	return true;
}

Defun1(cursorRightArrow)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	UT_return_val_if_fail (pView, false);
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
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if (pG)
	{
		pG->setCursor(GR_Graphics::GR_CURSOR_VLINE_DRAG);
	}
	return true;
}


Defun1(cursorTopCell)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	pFrame->setStatusMessage(NULL);

	GR_Graphics * pG = pView->getGraphics();
	if (pG)
	{
		pG->setCursor(GR_Graphics::GR_CURSOR_DOWNARROW);
	}
	return true;
}


Defun1(cursorHline)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	// clear status bar of any lingering messages
	UT_return_val_if_fail (pView, false);
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
	UT_return_val_if_fail (pView, false);
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
	UT_return_val_if_fail (pView, false);
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
	UT_return_val_if_fail (pView, false);
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

static bool dlgEditLatexEquation(AV_View *pAV_View, EV_EditMethodCallData * /*pCallData*/, bool bStartDlg, PT_DocPosition pos)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	FL_DocLayout * pLayout = pView->getLayout();
	GR_EmbedManager * pMath = pLayout->getEmbedManager("mathml");
	if(pMath->isDefault())
	{
	  //
	  // No MathML plugin. We can't edit this.
	  //
	  UT_DEBUGMSG(("No Math Plugin! \n"));
	     return false;
	}
	if(pos == 0)
	{
	    pos = pView->getPoint()-1;
	}
	fl_BlockLayout * pBlock = pView->getCurrentBlock();
	fp_Run * pRun = NULL;
	fp_MathRun * pMathRun = NULL;
	UT_sint32 x1,y1,x2,y2,height;
	bool bEOL = false;
	bool bDir = false;
	pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,height,bDir);
	while(pRun && pRun->getLength() == 0)
	{
		pRun = pRun->getNextRun();
	}
	if(pRun == NULL)
	{
		return false;
	}
	if(pRun->getType() != FPRUN_MATH)
    {
		return false;
	}
	pMathRun = static_cast<fp_MathRun *>(pRun);
	const PP_AttrProp * pSpanAP = pMathRun->getSpanAP();
	const gchar * pszLatexID = NULL, *pszDisplayMode = NULL;
	pSpanAP->getAttribute("latexid",pszLatexID);
	pSpanAP->getProperty("display",pszDisplayMode);
	if(pszLatexID == NULL || *pszLatexID == 0)
	{
		return false;
	}
	const UT_ByteBuf * pByteBuf = NULL;
	UT_UTF8String sLatex;
	PD_Document * pDoc= pView->getDocument();
	bool bFoundLatexID = pDoc->getDataItemDataByName(pszLatexID, 
													 &pByteBuf,
													 NULL, NULL);

	if(!bFoundLatexID)
	{
		return true;
	}
	UT_UCS4_mbtowc myWC;
	sLatex.appendBuf( *pByteBuf, myWC);
	UT_DEBUGMSG(("Loaded Latex %s from PT \n",sLatex.utf8_str()));
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_Latex * pDialog
		= static_cast<AP_Dialog_Latex *>(pDialogFactory->requestDialog(AP_DIALOG_ID_LATEX));
	UT_return_val_if_fail(pDialog, false);
	if(pDialog->isRunning())
	{
		pDialog->activate();
		pDialog->setDisplayMode((pszDisplayMode && !strcmp(pszDisplayMode, "inline"))?
		                        ABI_DISPLAY_INLINE: ABI_DISPLAY_BLOCK);
		pDialog->fillLatex(sLatex);
	}
	else if(bStartDlg)
	{
		pDialog->runModeless(pFrame);
		pDialog->setDisplayMode((pszDisplayMode && !strcmp(pszDisplayMode, "inline"))?
		                        ABI_DISPLAY_INLINE: ABI_DISPLAY_BLOCK);
		pDialog->fillLatex(sLatex);
	}
	else
	{
		pDialogFactory->releaseDialog(pDialog);
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
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

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
	UT_DEBUGMSG(("Context Menu Name is........ %s \n",szContextMenuName));
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
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	//
	// Look if we've right clicked on a mathrun
	//
	PT_DocPosition pos = 0;
	if(pView->isMathLoaded() && pView->isMathSelected(pCallData->m_xPos, pCallData->m_yPos,pos))
	{
	  return s_doContextMenu(EV_EMC_MATH,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
	}
	return s_doContextMenu(EV_EMC_TEXT,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
}


Defun(contextFrame)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	// no frame context menu in normal view ...
	if(pView->getViewMode() == VIEW_NORMAL)
		return true;
	
	return s_doContextMenu(EV_EMC_FRAME,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
}

Defun(contextRevision)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	return s_doContextMenu(EV_EMC_REVISION,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
}

Defun(contextTOC)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	return s_doContextMenu_no_move(EV_EMC_TOC,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
}


Defun(contextMath)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	bool b = false;
	if(pView->isMathLoaded())
	{
	    b = s_doContextMenu_no_move( EV_EMC_MATH,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
	}
	else
	{
	    b = s_doContextMenu_no_move( EV_EMC_TEXT,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);

	}
	return b;
}

#ifdef ENABLE_SPELL
Defun(contextMisspellText)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	UT_DEBUGMSG(("Doing Misspelt text \n"));
	return s_doContextMenu(EV_EMC_MISSPELLEDTEXT,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
}
#endif

Defun(contextImage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	fp_Run *pRun = NULL;

	if ( pView->isSelectionEmpty () )
	  {
		// select the object if it isn't already
		UT_DEBUGMSG(("Selecting objec: %d\n", pCallData->m_xPos));
		pView->warpInsPtToXY(pCallData->m_xPos, pCallData->m_yPos, true);
		pView->extSelHorizontal (true, 1);
	  }
	PT_DocPosition pos = pView->getDocPositionFromXY(pCallData->m_xPos, pCallData->m_yPos);
	fl_BlockLayout * pBlock = pView->getBlockAtPosition(pos);
	bool bDoEmbed = false;
	if(pBlock)
	{
		UT_sint32 x1,x2,y1,y2,iHeight;
		bool bEOL = false;
		bool bDir = false;
		
		pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,iHeight,bDir);
		while(pRun && ((pRun->getType() != FPRUN_IMAGE) && (pRun->getType() != FPRUN_EMBED)))
		{
			pRun = pRun->getNextRun();
		}
		if(pRun && ((pRun->getType() == FPRUN_IMAGE) || ((pRun->getType() == FPRUN_EMBED))))
		{
			// Set the cursor context to image selected.
			if(pRun->getType() == FPRUN_EMBED)
			{
			      bDoEmbed = true;
			}
		}
		else
		{
			// do nothing...
		}
	}
	if(!bDoEmbed)
	{
	     return s_doContextMenu(EV_EMC_IMAGE,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
	}
	// get the menu from pRun
	fp_EmbedRun *pEmbedRun = dynamic_cast<fp_EmbedRun*>(pRun);
	return s_doContextMenu((pRun)? pEmbedRun->getContextualMenu(): EV_EMC_EMBED, pCallData->m_xPos, pCallData->m_yPos, pView, pFrame);
}


Defun(contextPosObject)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	return s_doContextMenu_no_move(EV_EMC_POSOBJECT,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
}


Defun(contextEmbedLayout)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	fp_Run *pRun = NULL;

	if ( pView->isSelectionEmpty () )
	  {
		// select the image if it isn't already
		UT_DEBUGMSG(("Selecting image: %d\n", pCallData->m_xPos));
		pView->warpInsPtToXY(pCallData->m_xPos, pCallData->m_yPos, true);
		pView->extSelHorizontal (true, 1);
	  }
	PT_DocPosition pos = pView->getDocPositionFromXY(pCallData->m_xPos, pCallData->m_yPos);
	fl_BlockLayout * pBlock = pView->getBlockAtPosition(pos);
	if(pBlock)
	{
		UT_sint32 x1,x2,y1,y2,iHeight;
		bool bEOL = false;
		bool bDir = false;
		
		pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,iHeight,bDir);
		while(pRun && ((pRun->getType() != FPRUN_IMAGE) && (pRun->getType() != FPRUN_EMBED)))
		{
			pRun = pRun->getNextRun();
		}
	}
	// get the menu from pRun
	fp_EmbedRun *pEmbedRun = dynamic_cast<fp_EmbedRun*>(pRun);
	return s_doContextMenu((pRun)? pEmbedRun->getContextualMenu(): EV_EMC_EMBED, pCallData->m_xPos, pCallData->m_yPos, pView, pFrame);
}

Defun(contextHyperlink)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	// move the IP so actions have the right context
	if (!pView->isXYSelected(pCallData->m_xPos, pCallData->m_yPos))
		EX(warpInsPtToXY);
	
	fp_Run * pRun = pView->getHyperLinkRun(pView->getPoint());
	UT_return_val_if_fail(pRun, false);
	fp_HyperlinkRun * pHRun = pRun->getHyperlink();
	
	if(pHRun && pHRun->getHyperlinkType() == HYPERLINK_NORMAL) // normal hyperlinks
	{
#ifdef ENABLE_SPELL
		if(pView->isTextMisspelled())
			return s_doContextMenu_no_move(EV_EMC_HYPERLINKMISSPELLED,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
		else
#endif
			return s_doContextMenu_no_move(EV_EMC_HYPERLINKTEXT,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
	}

	if(pHRun && pHRun->getHyperlinkType() == HYPERLINK_ANNOTATION) // annotations
	{
#ifdef ENABLE_SPELL
		if(pView->isTextMisspelled())
			return s_doContextMenu_no_move(EV_EMC_ANNOTATIONMISSPELLED,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
		else
#endif
			return s_doContextMenu_no_move(EV_EMC_ANNOTATIONTEXT,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
	}


	if(pHRun && pHRun->getHyperlinkType() == HYPERLINK_RDFANCHOR)
	{
		UT_DEBUGMSG(("open rdf context menu\n"));
		return s_doContextMenu_no_move(EV_EMC_RDFANCHORTEXT,pCallData->m_xPos, pCallData->m_yPos,pView,pFrame);
	}
	return false; // to avoid compilation warnings (should never be reached)
}

#ifdef ENABLE_SPELL
static bool _spellSuggest(AV_View* pAV_View, UT_uint32 ndx)
{
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
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

	UT_return_val_if_fail (pView, false);
	pView->cmdContextIgnoreAll();
	return true;
}

Defun1(spellAdd)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail (pView, false);
	pView->cmdContextAdd();
	return true;
}
#endif

/*****************************************************************/


static void sActualDragToXY(AV_View *  pAV_View, EV_EditMethodCallData * pCallData)
{
	ABIWORD_VIEW;
	UT_return_if_fail (pView);
	AP_Frame *pFrame = static_cast<AP_Frame *>(pAV_View->getParentData());
	if(pFrame->getDoWordSelections())
	{
		pView->extSelToXYword(pCallData->m_xPos, pCallData->m_yPos, true);
		return;
	}
	pView->extSelToXY(pCallData->m_xPos, pCallData->m_yPos, true);
	return;
}

Defun(dragToXY)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
//
// Do this operation in an idle loop so when can reject queued events
//
//
// This code sets things up to handle the warp right in an idle loop.
//
	UT_return_val_if_fail (pView, false);
	int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	EV_EditMethodCallData * pNewData = new  EV_EditMethodCallData(pCallData->m_pData,pCallData->m_dataLength);
	pNewData->m_xPos = pCallData->m_xPos;
	pNewData->m_yPos = pCallData->m_yPos;
	_Freq * pFreq = new _Freq(pView,pNewData,sActualDragToXY);
	s_pFrequentRepeat = UT_WorkerFactory::static_constructor (_sFrequentRepeat,pFreq, inMode, outMode);

	UT_ASSERT(s_pFrequentRepeat);
	UT_ASSERT(outMode != UT_WorkerFactory::NONE);

	// If the worker is working on a timer instead of in the idle
	// time, set the frequency of the checks.
	if ( UT_WorkerFactory::TIMER == outMode )
	{
		// this is really a timer, so it's safe to static_cast it
		static_cast<UT_Timer*>(s_pFrequentRepeat)->set(50);
	}
	s_pFrequentRepeat->start();
	return true;
}

Defun(dragToXYword)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->extSelToXYword(pCallData->m_xPos, pCallData->m_yPos, true);
	return true;
}

Defun(endDrag)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->endDrag(pCallData->m_xPos, pCallData->m_yPos);
	return true;
}

Defun(extSelToXY)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->extSelToXY(pCallData->m_xPos, pCallData->m_yPos, false);
	return true;
}

Defun1(extSelLeft)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	bool bRTL = false;
	fl_BlockLayout * pBL = pView->getCurrentBlock();
	if(pBL)
		bRTL = pBL->getDominantDirection() == UT_BIDI_RTL;
	

	pView->extSelHorizontal(bRTL,1);

	return true;
}

Defun1(extSelRight)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	bool bRTL = false;
	fl_BlockLayout * pBL = pView->getCurrentBlock();
	if(pBL)
		bRTL = pBL->getDominantDirection() == UT_BIDI_RTL;
	

	pView->extSelHorizontal(!bRTL,1);

	return true;
}

Defun1(extSelBOL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->extSelTo(FV_DOCPOS_BOL);
	return true;
}

Defun1(extSelEOL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->extSelTo(FV_DOCPOS_EOL);
	return true;
}

Defun1(extSelBOW)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	bool bRTL = false;
	fl_BlockLayout * pBL = pView->getCurrentBlock();
	if(pBL)
		bRTL = pBL->getDominantDirection() == UT_BIDI_RTL;
	

	if(bRTL)
		pView->extSelTo(FV_DOCPOS_EOW_MOVE);
	else
		pView->extSelTo(FV_DOCPOS_BOW);

	return true;
}

Defun1(extSelEOW)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	bool bRTL = false;
	fl_BlockLayout * pBL = pView->getCurrentBlock();
	if(pBL)
		bRTL = pBL->getDominantDirection() == UT_BIDI_RTL;
	

	if(bRTL)
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
	UT_return_val_if_fail (pView, false);
	pView->extSelTo(FV_DOCPOS_BOB);
	return true;
}

Defun1(extSelEOB)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->extSelTo(FV_DOCPOS_EOB);
	return true;
}

Defun1(extSelBOD)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->extSelTo(FV_DOCPOS_BOD);
	return true;
}

Defun1(extSelEOD)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->extSelTo(FV_DOCPOS_EOD);
	return true;
}

Defun1(extSelPrevLine)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->extSelNextPrevLine(false);
	return true;
}

Defun1(extSelNextLine)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->extSelNextPrevLine(true);
	return true;
}

Defun1(extSelPageDown)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->extSelNextPrevPage(true);
	return true;
}

Defun1(extSelPageUp)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->extSelNextPrevPage(false);
	return true;
}

Defun1(extSelScreenDown)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->extSelNextPrevScreen(true);
	return true;
}

Defun1(extSelScreenUp)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->extSelNextPrevScreen(false);
	return true;
}

Defun(selectAll)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOD, FV_DOCPOS_EOD);
	return true;
}

Defun(selectWord)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOW, FV_DOCPOS_EOW_SELECT);
	return true;
}

Defun(selectLine)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	if(pView->getMouseContext(pCallData->m_xPos, pCallData->m_yPos) == EV_EMC_LEFTOFTEXT)
	{
		AP_Frame *pFrame = static_cast<AP_Frame *>(pAV_View->getParentData());
		if(pFrame->getDoWordSelections())
		{
			pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOB, FV_DOCPOS_EOB);
			return true;
		}
	}
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOL, FV_DOCPOS_EOL);
	return true;
}

Defun(selectBlock)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOB, FV_DOCPOS_EOB);
	return true;
}

Defun1(selectTable)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	PT_DocPosition posStartTab,posEndTab;
	pf_Frag_Strux* tableSDH,*endTableSDH;
	PD_Document * pDoc = pView->getDocument();
	bool bRes = pDoc->getStruxOfTypeFromPosition(pView->getPoint(),PTX_SectionTable,&tableSDH);
	if(!bRes)
	{
		UT_DEBUGMSG(("No Table Strux found!! \n"));
		return false;
	}
	posStartTab = pDoc->getStruxPosition(tableSDH); //was -1
	UT_DEBUGMSG(("PosStart %d TableSDH %p \n",posStartTab,tableSDH));
	bRes = pDoc->getNextStruxOfType(tableSDH,PTX_EndTable,&endTableSDH);
	if(!bRes)
	{
		UT_DEBUGMSG(("No End Table Strux found!! \n"));
		return false;
	}
	posEndTab = pDoc->getStruxPosition(endTableSDH)+1; //was +1
	UT_DEBUGMSG(("PosEndTab %d endTableSDH %p \n",posEndTab,endTableSDH));
	pView->cmdSelect(posStartTab,posEndTab);
	return true;
}


Defun(selectTOC)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Select TOC \n"));
	UT_return_val_if_fail (pView, false);
	pView->cmdSelectTOC(pCallData->m_xPos, pCallData->m_yPos);
	return true;
}


Defun(editLatexAtPos)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Edit Math at Pos\n"));
	UT_return_val_if_fail (pView, false);
        PT_DocPosition pos = pView->getDocPositionFromLastXY();
        return dlgEditLatexEquation(pAV_View, pCallData, true,pos);
}


Defun(editLatexEquation)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Select and Edit Math \n"));
	UT_return_val_if_fail (pView, false);
        PT_DocPosition posL = pView->getDocPositionFromXY(pCallData->m_xPos, pCallData->m_yPos);
	PT_DocPosition posH = posL+1;
	pView->cmdSelect(posL,posH);
        return dlgEditLatexEquation(pAV_View, pCallData, true,0);
}


Defun1(editEmbed)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Select and Edit an Embedded Object \n"));
	UT_return_val_if_fail (pView, false);
        PT_DocPosition posL = pView->getPoint();
	PT_DocPosition posH = pView->getSelectionAnchor();
	PT_DocPosition posTemp = 0;
	if(posH < posL)
	{
	     posTemp = posL;
	     posL = posH;
	     posH = posTemp;
	}
	if(posL == posH)
	{
	     posH = posL+1;
	     pView->cmdSelect(posL,posH);
	}
	fl_BlockLayout * pBlock = pView->getBlockAtPosition(posL);
	if(pBlock)
	{
		UT_sint32 x1,x2,y1,y2,iHeight;
		bool bEOL = false;
		bool bDir = false;
		
		fp_Run * pRun = NULL;
		
		pRun = pBlock->findPointCoords(posL,bEOL,x1,y1,x2,y2,iHeight,bDir);
		while(pRun && ((pRun->getType() != FPRUN_IMAGE) && (pRun->getType() != FPRUN_EMBED)))
		{
			pRun = pRun->getNextRun();
		}
		if(pRun && ((pRun->getType() == FPRUN_IMAGE) || ((pRun->getType() == FPRUN_EMBED))))
		{
			// Set the cursor context to image selected.
		  UT_DEBUGMSG(("Found and embedded Object \n"));
			if(pRun->getType() == FPRUN_EMBED)
			{
			      fp_EmbedRun * pEmbedRun = static_cast<fp_EmbedRun *>(pRun);
			      UT_DEBUGMSG(("About to edit the object \n"));
			      GR_EmbedManager * pEmbed = pEmbedRun->getEmbedManager();
			      UT_sint32 uid = pEmbedRun->getUID();
			      pEmbed->modify(uid);
			}
		}
	}

	return true;
}

Defun(selectMath)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Select Math \n"));
	UT_return_val_if_fail (pView, false);
        PT_DocPosition posL = pView->getDocPositionFromXY(pCallData->m_xPos, pCallData->m_yPos);
	PT_DocPosition posH = posL+1;
	pView->cmdSelect(posL,posH);
	dlgEditLatexEquation(pAV_View, pCallData, false,0);
	return true;
}


Defun1(selectRow)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	PT_DocPosition posStartRow,posEndRow;
	pf_Frag_Strux* rowSDH,*endRowSDH,*tableSDH;
	UT_sint32 iLeft,iRight,iTop,iBot;

	PD_Document * pDoc = pView->getDocument();
	pView->getCellParams(pView->getPoint(), &iLeft, &iRight,&iTop,&iBot);
	
	bool bRes = pDoc->getStruxOfTypeFromPosition(pView->getPoint(),PTX_SectionTable,&tableSDH);
	if(!bRes)
	{
		return false;
	}
  
	//
	// Now find the number of rows and columns inthis table.
    //
	UT_sint32 numRows,numCols;
	bRes = pDoc->getRowsColsFromTableSDH(tableSDH,pView->isShowRevisions(), pView->getRevisionLevel(),&numRows,&numCols);
	if(!bRes)
	{
		return false;
	}
	rowSDH = pDoc->getCellSDHFromRowCol(tableSDH,pView->isShowRevisions(), pView->getRevisionLevel(),iTop,0);
	posStartRow = pDoc->getStruxPosition(rowSDH) - 1;
	endRowSDH = pDoc->getCellSDHFromRowCol(tableSDH,pView->isShowRevisions(), pView->getRevisionLevel(),iTop,numCols -1);
	posEndRow = pDoc->getStruxPosition(endRowSDH);
	bRes = pDoc->getNextStruxOfType(endRowSDH,PTX_EndCell,&endRowSDH);
	if(!bRes)
	{
		return false;
	}
	posEndRow = pDoc->getStruxPosition(endRowSDH)+1;
	pView->cmdSelect(posStartRow,posEndRow);
	pView->setSelectionMode(FV_SelectionMode_TableRow);
	return true;
}


Defun1(selectCell)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	PT_DocPosition posStartCell,posEndCell;
	pf_Frag_Strux* cellSDH,*endCellSDH;
	PD_Document * pDoc = pView->getDocument();
	bool bRes = pDoc->getStruxOfTypeFromPosition(pView->getPoint(),PTX_SectionCell,&cellSDH);
	if(!bRes)
	{
		return false;
	}
	posStartCell = pDoc->getStruxPosition(cellSDH) - 1;
	bRes = pDoc->getNextStruxOfType(cellSDH,PTX_EndCell,&endCellSDH);
	if(!bRes)
	{
		return false;
	}
	posEndCell = pDoc->getStruxPosition(endCellSDH)+1;
	pView->cmdSelect(posStartCell,posEndCell);
	return true;
}


Defun1(selectColumn)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	if(!pView->isInTable())
	{
		return false;
	}
	pView->cmdSelectColumn(pView->getPoint());
	return true;
}


Defun(selectColumnClick)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
	PT_DocPosition pos = pView->getDocPositionFromXY(x,y);
	if(!pView->isInTable(pos))
	{
		return false;
	}
	pView->cmdSelectColumn(pos);
	return true;
}


static void sActualDelLeft(AV_View *  pAV_View, EV_EditMethodCallData * /*pCallData*/)
{
	ABIWORD_VIEW;
	UT_return_if_fail (pView);
	pView->cmdCharDelete(false,1);
}

Defun1(delLeft)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
//
// Do this operation in an idle loop so when can reject queued events
//
//
// This code sets things up to handle the warp right in an idle loop.
//
	UT_return_val_if_fail (pView, false);
	int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	_Freq * pFreq = new _Freq(pView,NULL,sActualDelLeft);
	s_pFrequentRepeat = UT_WorkerFactory::static_constructor (_sFrequentRepeat,pFreq, inMode, outMode);

	UT_ASSERT(s_pFrequentRepeat);
	UT_ASSERT(outMode != UT_WorkerFactory::NONE);

	// If the worker is working on a timer instead of in the idle
	// time, set the frequency of the checks.
	if ( UT_WorkerFactory::TIMER == outMode )
	{
		// this is really a timer, so it's safe to static_cast it
		static_cast<UT_Timer*>(s_pFrequentRepeat)->set(50);
	}
	s_pFrequentRepeat->start();
	return true;
}


static void sActualDelRight(AV_View *  pAV_View, EV_EditMethodCallData * /*pCallData*/)
{
	ABIWORD_VIEW;
	UT_return_if_fail (pView);
	pView->cmdCharDelete(true,1);
}

Defun1(delRight)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
//
// Do this operation in an idle loop so when can reject queued events
//
//
// This code sets things up to handle the warp right in an idle loop.
//
	UT_return_val_if_fail (pView, false);
	int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	_Freq * pFreq = new _Freq(pView,NULL,sActualDelRight);
	s_pFrequentRepeat = UT_WorkerFactory::static_constructor (_sFrequentRepeat,pFreq, inMode, outMode);

	UT_ASSERT(s_pFrequentRepeat);
	UT_ASSERT(outMode != UT_WorkerFactory::NONE);

	// If the worker is working on a timer instead of in the idle
	// time, set the frequency of the checks.
	if ( UT_WorkerFactory::TIMER == outMode )
	{
		// this is really a timer, so it's safe to static_cast it
		static_cast<UT_Timer*>(s_pFrequentRepeat)->set(50);
	}
	s_pFrequentRepeat->start();
	return true;
}

Defun1(delBOL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->delTo(FV_DOCPOS_BOL);
	return true;
}

Defun1(delEOL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->delTo(FV_DOCPOS_EOL);
	return true;
}

Defun1(delBOW)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->delTo(FV_DOCPOS_BOW);
	return true;
}

Defun1(delEOW)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
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
	UT_return_val_if_fail(pView, false);
	pView->delTo(FV_DOCPOS_BOB);
	return true;
}

Defun1(delEOB)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	pView->delTo(FV_DOCPOS_EOB);
	return true;
}

Defun1(delBOD)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->delTo(FV_DOCPOS_BOD);
	return true;
}

Defun1(delEOD)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->delTo(FV_DOCPOS_EOD);
	return true;
}

#if 0
static bool pView->cmdCharInsert(const UT_UCS4Char * pText, UT_uint32 iLen,
						XAP_Frame * pFrame, FV_View * pView,
						bool bForce = false)
{
	// handle automatic language formatting
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);

	bool b = false;

	pPrefs->getPrefsValueBool(static_cast<gchar*>(XAP_PREF_KEY_ChangeLanguageWithKeyboard),
							  &b);
	if(b)
	{
		const UT_LangRecord * pLR = pApp->getKbdLanguage();

		if (pLR)
		{
			const gchar * props_out[] = {"lang", NULL, NULL};
			props_out[1] = pLR->m_szLangCode;
			pView->setCharFormat(props_out);
		}
	}
	
	pView->cmdCharInsert(pText, iLen, bForce);
	return true;
}
#endif

#if 0 // disabled because of conditionnal below
static void sActualInsertData(AV_View *  pAV_View, EV_EditMethodCallData * pCallData)
{
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->cmdCharInsert(pCallData->m_pData, pCallData->m_dataLength);
	return;
}
#endif
Defun(insertData)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, false);
	pView->cmdCharInsert(pCallData->m_pData, pCallData->m_dataLength);
#if 0
//
// Do this operation in an idle loop so when can reject queued events
//
//
// This code sets things up to handle the warp right in an idle loop.
//
	int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	GR_Graphics * pG = pView->getGraphics();
	EV_EditMethodCallData * pNewData = new  EV_EditMethodCallData(pCallData->m_pData,pCallData->m_dataLength);
	_Freq * pFreq = new _Freq(pView,pNewData,sActualInsertData);
	s_pFrequentRepeat = UT_WorkerFactory::static_constructor (_sFrequentRepeat,pFreq, inMode, outMode, pG);

	UT_ASSERT(s_pFrequentRepeat);
	UT_ASSERT(outMode != UT_WorkerFactory::NONE);

	// If the worker is working on a timer instead of in the idle
	// time, set the frequency of the checks.
	if ( UT_WorkerFactory::TIMER == outMode )
	{
		// this is really a timer, so it's safe to static_cast it
		static_cast<UT_Timer*>(s_pFrequentRepeat)->set(50);
	}
	s_pFrequentRepeat->start();
#endif
	return true;
}


Defun(insertClosingParenthesis)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	ABIWORD_VIEW;

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);

	bool bLang = false, bMarker = false;

	pPrefs->getPrefsValueBool(static_cast<const gchar *>(XAP_PREF_KEY_ChangeLanguageWithKeyboard),
							  &bLang);

	const UT_LangRecord * pLR = NULL;
	
	if(bLang)
	{
		pLR = pApp->getKbdLanguage();
		
		pPrefs->getPrefsValueBool(static_cast<const gchar *>(XAP_PREF_KEY_DirMarkerAfterClosingParenthesis), &bMarker);
	}

	if(bMarker && pLR)
	{
		UT_return_val_if_fail(pCallData->m_dataLength == 1, false);
		UT_UCS4Char data[2];
		data[0] = (UT_UCS4Char) *(pCallData->m_pData);
		
		if(pLR->m_eDir == UTLANG_RTL)
		{
			data[1] = UCS_RLM;
		}
		else if(pLR->m_eDir == UTLANG_LTR)
		{
			data[1] = UCS_LRM;
		}
		else
		{
			goto normal_insert;
		}

		pView->cmdCharInsert(&data[0],2);
		return true;
	}

 normal_insert:	
	pView->cmdCharInsert(pCallData->m_pData, pCallData->m_dataLength);
	return true;
}

Defun(insertOpeningParenthesis)
{
	CHECK_FRAME;
	UT_return_val_if_fail (pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	ABIWORD_VIEW;

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);

	bool bLang = false, bMarker = false;

	pPrefs->getPrefsValueBool(static_cast<const gchar *>(XAP_PREF_KEY_ChangeLanguageWithKeyboard),
							  &bLang);

	const UT_LangRecord * pLR = NULL;
	
	if(bLang)
	{
		pLR = pApp->getKbdLanguage();
		
		pPrefs->getPrefsValueBool(static_cast<const gchar *>(XAP_PREF_KEY_DirMarkerAfterClosingParenthesis), &bMarker);
	}
	
	if(bMarker && pLR)
	{
		UT_return_val_if_fail(pCallData->m_dataLength == 1, false);
		UT_UCS4Char data[2];
		data[1] = (UT_UCS4Char) *(pCallData->m_pData);

		if(pLR->m_eDir == UTLANG_RTL)
		{
			data[0] = UCS_RLM;
		}
		else if(pLR->m_eDir == UTLANG_LTR)
		{
			data[0] = UCS_LRM;
		}
		else
		{
			goto normal_insert;
		}

		pView->cmdCharInsert(&data[0], 2);
		return true;
	}

 normal_insert:	
	pView->cmdCharInsert(pCallData->m_pData, pCallData->m_dataLength);
	return true;
}

Defun1(insertLRM)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	
	UT_return_val_if_fail (pView, false);
	UT_UCS4Char cM = UCS_LRM;
	pView->cmdCharInsert(&cM, 1);
	return true;
}

Defun1(insertRLM)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail (pView, false);
	UT_UCS4Char cM = UCS_RLM;
	pView->cmdCharInsert(&cM, 1);
	return true;
}

/*****************************************************************/
// TODO the bInsert parameter is currently not used; it would require
// changes to the dialogues so that if bInsert == false the dialogue
// would be labeled "Delete bookmark"

static bool s_doBookmarkDlg(FV_View * pView, bool /*bInsert*/)
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_InsertBookmark * pDialog
		= static_cast<AP_Dialog_InsertBookmark *>(pDialogFactory->requestDialog(AP_DIALOG_ID_INSERTBOOKMARK));
UT_return_val_if_fail(pDialog, false);
	if (!pView->isSelectionEmpty())
	{
		UT_UCSChar * buffer;
		pView->getSelectionText(buffer);
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


static bool s_xmlidDlg(FV_View * pView, bool /*bInsert*/)
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_InsertXMLID * pDialog
		= static_cast<AP_Dialog_InsertXMLID *>(pDialogFactory->requestDialog(AP_DIALOG_ID_INSERTXMLID));
	UT_return_val_if_fail(pDialog, false);

	pDialog->setDoc(pView);
	pDialog->runModal(pFrame);

	AP_Dialog_GetStringCommon::tAnswer ans = pDialog->getAnswer();

	if (ans == AP_Dialog_GetStringCommon::a_OK)
	{
			pView->cmdInsertXMLID(pDialog->getString());
	}
	else if(ans == AP_Dialog_GetStringCommon::a_DELETE)
	{
			pView->cmdDeleteXMLID(pDialog->getString());
	}
	pDialogFactory->releaseDialog(pDialog);

	return (ans == AP_Dialog_GetStringCommon::a_DELETE || ans == AP_Dialog_GetStringCommon::a_OK);
}

Defun1(insertXMLID)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("insertXMLID()\n"));
	s_xmlidDlg(pView, true);
	return true;
}

Defun1(deleteXMLID)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	s_xmlidDlg(pView, false);
	return true;
}

/*****************************************************************/
static bool s_doHyperlinkDlg(FV_View * pView)
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_InsertHyperlink * pDialog
		= static_cast<AP_Dialog_InsertHyperlink *>(pDialogFactory->requestDialog(AP_DIALOG_ID_INSERTHYPERLINK));
	UT_return_val_if_fail(pDialog, false);
	std::string sTarget;
	std::string sTitle;
	bool bEdit = false;
	PT_DocPosition pos1 = 0;
	PT_DocPosition pos2 = 0;
	PT_DocPosition posOrig = pView->getPoint();
	pDialog->setDoc(pView);
	if(pView->isSelectionEmpty())
	{
		const char *buf;
		fp_HyperlinkRun * pHRun = static_cast<fp_HyperlinkRun *>(pView->getHyperLinkRun(pView->getPoint()));
		if(pHRun == NULL)
		{
			pDialogFactory->releaseDialog(pDialog);
			return false;
		}
		bEdit = true;
		buf = pHRun->getTarget();
		if (buf != NULL)
			sTarget = buf;
		buf = pHRun->getTitle();
		if (buf != NULL)
			sTitle = buf;
		fl_BlockLayout * pBL = pHRun->getBlock();
		fp_Run * pRun = NULL;
		if(pHRun->isStartOfHyperlink())
		{
			pos1 = pBL->getPosition(true) + pHRun->getBlockOffset()+1;
			pRun = pHRun->getNextRun();
			pos2 = pBL->getPosition(true) + pHRun->getBlockOffset() + 1;
			while(pRun && (pRun->getType() != FPRUN_HYPERLINK))
			{
			        pos2 += pRun->getLength();
				pRun = pRun->getNextRun();
			}
		}
		else
		{
			pos2 = pBL->getPosition(true) + pHRun->getBlockOffset();
			pRun = pHRun->getPrevRun();
			pos1 = pBL->getPosition(true) + pHRun->getBlockOffset();
			while(pRun && pRun->getHyperlink())
			{
				pos1 = pBL->getPosition(true) + pRun->getBlockOffset();
				pRun = pRun->getPrevRun();
			}
		}
//
// Select our range
//
//		pView->cmdSelect(pos1,pos2);
//
// Set the target
//
		pDialog->setHyperlink(sTarget.c_str());
		pDialog->setHyperlinkTitle(sTitle.c_str());
	}
	pDialog->runModal(pFrame);

	AP_Dialog_InsertHyperlink::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_InsertHyperlink::a_OK);

	if (bOK)
	{
		if(bEdit)
		{
//
// Delete the old one.
//
			pView->cmdDeleteHyperlink();
			if(!pView->isSelectionEmpty())
			{
				pView->cmdUnselectSelection();
			}
//
// Select our range
//
			pView->cmdSelect(pos1,pos2);
			pView->cmdInsertHyperlink(pDialog->getHyperlink(),
				pDialog->getHyperlinkTitle());
			pView->cmdUnselectSelection();
			pView->setPoint(posOrig);
		}
		else
		{
			pView->cmdInsertHyperlink(pDialog->getHyperlink(),
				pDialog->getHyperlinkTitle());
		}
	}
	else
	{
		if(bEdit)
		{
			pView->cmdUnselectSelection();
			pView->setPoint(posOrig);
		}
	}
	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

Defun1(insertHyperlink)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	if(pView->isSelectionEmpty())
	{
		if(!pView->getHyperLinkRun(pView->getPoint()))
		{
		//No selection
			XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
			UT_return_val_if_fail (pFrame, false);


			pFrame->showMessageBox(AP_STRING_ID_MSG_HyperlinkNoSelection, 
								   XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
			return false;
		}
	}
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
	UT_return_val_if_fail(pView, false);
	pView->insertParagraphBreak();
	return true;
}

Defun1(insertSectionBreak)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView, false);
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
	if(pView->isInFrame(pView->getPoint()))
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
		UT_return_val_if_fail(pFrame, false);
		pFrame->showMessageBox(AP_STRING_ID_MSG_NoBreakInsideFrame,
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
	UT_return_val_if_fail(pView, false);
	UT_UCSChar c = UCS_TAB;
	if(!pView->isInTable())
	{
		pView->cmdCharInsert(&c,1);
	}
	else
	{
		pView->cmdAdvanceNextPrevCell(true);
	}
	return true;
}


Defun1(insertTabCTL)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	UT_UCSChar c = UCS_TAB;
	pView->cmdCharInsert(&c,1);
	return true;
}


Defun1(insertTabShift)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	if(!pView->isInTable())
	{
		return true;
	}
	else
	{
		pView->cmdAdvanceNextPrevCell(false);
	}
	return true;
}


Defun1(insertLineBreak)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	UT_UCSChar c = UCS_LF;
	pView->cmdCharInsert(&c,1);
	return true;
}

Defun1(insertColumnBreak)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
//
// No column breaks in header/Footers
//
	if(pView->isHdrFtrEdit())
		return true;
	if(pView->isInTable())
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
		UT_return_val_if_fail(pFrame, false);
		pFrame->showMessageBox(AP_STRING_ID_MSG_NoBreakInsideTable,
							   XAP_Dialog_MessageBox::b_O,
							   XAP_Dialog_MessageBox::a_OK);
		return true;
	}
	if(pView->isInFrame(pView->getPoint()))
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
		UT_return_val_if_fail(pFrame, false);
		pFrame->showMessageBox(AP_STRING_ID_MSG_NoBreakInsideFrame,
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
	UT_return_val_if_fail(pView, false);
	PT_DocPosition insPoint;
	PT_DocPosition insAnchor;
	if(pView->isSelectionEmpty())
	{
		insPoint = pView->getPoint();
	}
	else
	{
		insPoint = pView->getPoint();
		insAnchor = pView->getSelectionAnchor();
		if(insAnchor < insPoint)
		{
			insPoint = insAnchor;
		}
	}

	pView->cmdInsertCol(insPoint,true); // is before
	return true;
}


Defun1(insertColsAfter)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PT_DocPosition insPoint;
	PT_DocPosition insAnchor;
	if(pView->isSelectionEmpty())
	{
		insPoint = pView->getPoint();
	}
	else
	{
		insPoint = pView->getPoint();
		insAnchor = pView->getSelectionAnchor();
		if(insAnchor < insPoint)
		{
			insPoint = insAnchor;
		}
	}

	pView->cmdInsertCol(insPoint,false); // is After
	return true;
}


Defun1(insertRowsBefore)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PT_DocPosition insPoint;
	PT_DocPosition insAnchor;
	if(pView->isSelectionEmpty())
	{
		insPoint = pView->getPoint();
	}
	else
	{
		insPoint = pView->getPoint();
		insAnchor = pView->getSelectionAnchor();
		if(insAnchor < insPoint)
		{
			insPoint = insAnchor;
		}
	}
	pView->cmdInsertRow(insPoint,true); // is before
	return true;
}


Defun1(insertRowsAfter)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PT_DocPosition insPoint;
	PT_DocPosition insAnchor;
	if(pView->isSelectionEmpty())
	{
		insPoint = pView->getPoint();
	}
	else
	{
		insPoint = pView->getPoint();
		insAnchor = pView->getSelectionAnchor();
		if(insAnchor > insPoint)
		{
			insPoint = insAnchor;
		}
	}
	pView->cmdInsertRow(insPoint,false); // is After
	return true;
}

/*********************************************************************************/

static bool s_doMergeCellsDlg(FV_View * pView)
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_MergeCells * pDialog
		= static_cast<AP_Dialog_MergeCells *>(pDialogFactory->requestDialog(AP_DIALOG_ID_MERGE_CELLS));
UT_return_val_if_fail(pDialog, false);
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


static bool s_doSplitCellsDlg(FV_View * pView)
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_SplitCells * pDialog
		= static_cast<AP_Dialog_SplitCells *>(pDialogFactory->requestDialog(AP_DIALOG_ID_SPLIT_CELLS));
UT_return_val_if_fail(pDialog, false);
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


Defun1(splitCells)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	s_doSplitCellsDlg(pView);
	return true;
}

/***********************************************************************************/

static bool s_doFormatTableDlg(FV_View * pView)
{
	UT_return_val_if_fail(pView, false);
	if(!pView->isInTable(pView->getPoint()))
	{
		pView->swapSelectionOrientation();
		UT_ASSERT(pView->isInTable(pView->getPoint()));
	}

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();
	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	//	Maleesh 6/8/2010 - TEMP

	AP_Dialog_FormatTable * pDialog
		= static_cast<AP_Dialog_FormatTable *>(pDialogFactory->requestDialog(AP_DIALOG_ID_FORMAT_TABLE));
	UT_return_val_if_fail(pDialog, false);
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


Defun1(formatTOC)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_FormatTOC * pDialog
		= static_cast<AP_Dialog_FormatTOC *>(pDialogFactory->requestDialog(AP_DIALOG_ID_FORMAT_TOC));
UT_return_val_if_fail(pDialog, false);
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

/***********************************************************************************/

Defun1(deleteCell)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView, false);
	pView->cmdDeleteCell(pView->getPoint());
	return true;
}


Defun1(deleteColumns)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView, false);
	pView->cmdDeleteCol(pView->getPoint());
	return true;
}


Defun1(deleteRows)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PT_DocPosition pos = pView->getPoint();
	if(pos > pView->getSelectionAnchor())
	{
		pos = pView->getSelectionAnchor();
	}
	pView->cmdDeleteRow(pos);
	return true;
}

Defun1(deleteTable)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PT_DocPosition pos = pView->getPoint();
	if(!pView->isInTable(pos))
	{
	  if(pos > pView->getSelectionAnchor())
	  {
	    pos--;
	  }
	  else
	  {
	    pos++;
	  }
	}
	pView->cmdDeleteTable(pos);
	return true;
}

Defun1(insertPageBreak)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView, false);

	UT_UCSChar c = UCS_FF;
//
// No page breaks in header/Footers
//
	if(pView->isHdrFtrEdit())
		return true;
	if(pView->isInTable())
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
		UT_return_val_if_fail(pFrame, false);
		pFrame->showMessageBox(AP_STRING_ID_MSG_NoBreakInsideTable,
							   XAP_Dialog_MessageBox::b_O,
							   XAP_Dialog_MessageBox::a_OK);
		return true;
	}
	if(pView->isInFrame(pView->getPoint()))
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
		UT_return_val_if_fail(pFrame, false);
		pFrame->showMessageBox(AP_STRING_ID_MSG_NoBreakInsideFrame,
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
	UT_return_val_if_fail(pView, false);
	UT_UCSChar c = UCS_SPACE;
	pView->cmdCharInsert(&c,1);
	return true;
}

Defun1(insertNBSpace)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	UT_UCSChar c = UCS_NBSP;			// decimal 160 is NBS
	pView->cmdCharInsert(&c,1);
	return true;
}

// non-breaking, zerrow width 
Defun1(insertNBZWSpace)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	UT_UCSChar c = 0xFEFF;
	pView->cmdCharInsert(&c,1);
	return true;
}

// zero width joiner
Defun1(insertZWJoiner)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
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

	UT_return_val_if_fail (pCallData->m_dataLength==1, false);
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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	UT_return_val_if_fail (pCallData->m_dataLength==1, false);
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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	UT_return_val_if_fail (pCallData->m_dataLength==1, false);
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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	UT_return_val_if_fail (pCallData->m_dataLength==1, false);
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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	UT_return_val_if_fail (pCallData->m_dataLength==1, false);
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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	UT_return_val_if_fail(pCallData->m_dataLength==1, false);
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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	UT_return_val_if_fail(pCallData->m_dataLength==1, false);
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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	UT_return_val_if_fail(pCallData->m_dataLength==1, false);
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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	UT_return_val_if_fail(pCallData->m_dataLength==1, false);
	UT_UCSChar doubleacuteChar = 0x0000;

	switch (pCallData->m_pData[0])
	{
// Latin-2 characters
	case 0x4f:		doubleacuteChar=0x01d5; break;	// Odoubleacute
	case 0x55:		doubleacuteChar=0x01db; break;	// Udoubleacute

	case 0x6f:		doubleacuteChar=0x01f5; break;	// odoubleacute
	case 0x75:		doubleacuteChar=0x01fb; break;	// udoubleacute

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	UT_return_val_if_fail(pCallData->m_dataLength==1, false);
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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	UT_return_val_if_fail(pCallData->m_dataLength==1, false);
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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	UT_return_val_if_fail(pCallData->m_dataLength==1, false);
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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
	UT_return_val_if_fail(pView, false);
	if (pView->isFrameSelected())
	{
		pView->copyFrame(false);
	}
	else
	{
		pView->cmdCut();
	}

	return true;
}

Defun1(copy)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	if (pView->isFrameSelected())
	{
		pView->copyFrame(true);
	}
	else
	{
		pView->cmdCopy();
	}

	return true;
}

static void sActualPaste(AV_View *  pAV_View, EV_EditMethodCallData * /*pCallData*/)
{
	ABIWORD_VIEW;
	pView->cmdPaste();
	return;
}

Defun1(paste)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
//
// Do this operation in an idle loop so when can reject queued events
//
//
// This code sets things up to handle the warp right in an idle loop.
//
	UT_return_val_if_fail(pView, false);
	int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	_Freq * pFreq = new _Freq(pView,NULL,sActualPaste);
	s_pFrequentRepeat = UT_WorkerFactory::static_constructor (_sFrequentRepeat,pFreq, inMode, outMode);

	UT_ASSERT(s_pFrequentRepeat);
	UT_ASSERT(outMode != UT_WorkerFactory::NONE);

	// If the worker is working on a timer instead of in the idle
	// time, set the frequency of the checks.
	if ( UT_WorkerFactory::TIMER == outMode )
	{
		// this is really a timer, so it's safe to static_cast it
		static_cast<UT_Timer*>(s_pFrequentRepeat)->set(50);
	}
	s_pFrequentRepeat->start();

	return true;
}

Defun(pasteSelection)
{
	CHECK_FRAME;
// this is intended for the X11 middle mouse thing.
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	pView->cmdPasteSelectionAt(pCallData->m_xPos, pCallData->m_yPos);

	return true;
}

Defun1(pasteSpecial)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	pView->cmdPaste(false);

	return true;
}

static bool checkViewModeIsPrint(FV_View * pView)
{
	UT_return_val_if_fail(pView, false);
	if(pView->getViewMode() != VIEW_PRINT)
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
		UT_return_val_if_fail(pFrame, false);
		XAP_Dialog_MessageBox::tAnswer res = pFrame->showMessageBox(AP_STRING_ID_MSG_CHECK_PRINT_MODE,
				   XAP_Dialog_MessageBox::b_YN,
				   XAP_Dialog_MessageBox::a_NO);
		if(res == XAP_Dialog_MessageBox::a_NO)
		{
			return false;
		}
		else
		{

			AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
			UT_return_val_if_fail (pFrameData, false);

			pFrameData->m_pViewMode = VIEW_PRINT;
			pFrame->toggleLeftRuler (true && (pFrameData->m_bShowRuler) &&
									 (!pFrameData->m_bIsFullScreen));


			pView->setViewMode (VIEW_PRINT);

			// POLICY: make this the default for new frames, too
			XAP_App * pApp = XAP_App::getApp();
			UT_return_val_if_fail(pApp, false);
			XAP_Prefs * pPrefs = pApp->getPrefs();
			UT_return_val_if_fail(pPrefs, false);
			XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
			UT_return_val_if_fail (pScheme, false);

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
	UT_return_val_if_fail(pView, false);
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
	UT_return_val_if_fail(pView, false);
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
	UT_return_val_if_fail(pView, false);
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

	UT_return_val_if_fail(pView, false);
	if(checkViewModeIsPrint(pView))
	{
		pView->cmdEditHeader();
	}
	return true;
}

/*****************************************************************/

static bool s_doGotoDlg(FV_View * pView, XAP_Dialog_Id id)
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_Goto * pDialog
		= static_cast<AP_Dialog_Goto *>(pDialogFactory->requestDialog(id));
UT_return_val_if_fail(pDialog, false);
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

#ifdef ENABLE_SPELL
static bool s_doSpellDlg(FV_View * pView, XAP_Dialog_Id id)
{
   UT_return_val_if_fail(pView,false);
   XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
   UT_return_val_if_fail(pFrame, false);

   pFrame->raise();

   XAP_DialogFactory * pDialogFactory
	 = static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

   AP_Dialog_Spell * pDialog
	 = static_cast<AP_Dialog_Spell *>(pDialogFactory->requestDialog(id));
   UT_return_val_if_fail (pDialog, false);

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
#endif

/*****************************************************************/

static bool s_doFindOrFindReplaceDlg(FV_View * pView, XAP_Dialog_Id id)
{
	UT_return_val_if_fail(pView,false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_Replace * pDialog
		= static_cast<AP_Dialog_Replace *>(pDialogFactory->requestDialog(id));
UT_return_val_if_fail(pDialog, false);
	// don't match case by default
	pDialog->setMatchCase(false);

	// prime the dialog with a "find" string if there's a
	// current selection.
	if (!pView->isSelectionEmpty())
	{
		UT_UCSChar * buffer;
		pView->getSelectionText(buffer);
		if(buffer != NULL)
		{
			pDialog->setFindString(buffer);
			FREEP(buffer);
		}
		else
		{
			pView->setPoint(pView->getPoint());
		}
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

	UT_return_val_if_fail(pView, false);
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
	UT_return_val_if_fail(pView,false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_Dialog_Id id = XAP_DIALOG_ID_LANGUAGE;

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_Language * pDialog
		= static_cast<XAP_Dialog_Language *>(pDialogFactory->requestDialog(id));
	UT_return_val_if_fail(pDialog, false);
	
	const gchar ** props_in = NULL;
	if (pView->getCharFormat(&props_in))
	{
		pDialog->setLanguageProperty(UT_getAttribute("lang", props_in));
		FREEP(props_in);
	}

	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail( pDoc, false );

	const PP_AttrProp *  pAP = pDoc->getAttrProp();
	UT_return_val_if_fail( pAP, false );

	const gchar * pLang = NULL;
	bool bRet = pAP->getProperty("lang", pLang);

	if(bRet)
	{
		pDialog->setDocumentLanguage(pLang);
	}
	else
	{
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
	}
	
	// run the dialog

	pDialog->runModal(pFrame);

	// extract what they did

	bool bOK = (pDialog->getAnswer() == XAP_Dialog_Language::a_OK);

	if (bOK)
	{
		//UT_DEBUGMSG(("pressed OK\n"));
		UT_uint32 k = 0;
		const gchar * props_out[3];
		const gchar * s = NULL;

		bool bChange = pDialog->getChangedLangProperty(&s);
		
		if (s)
		{
			props_out[k++] = "lang";
			props_out[k++] = s;
		}

		props_out[k] = 0;						// put null after last pair.

		if(k > 0 && bChange)								// if something changed
			pView->setCharFormat(props_out);

		if(k > 0 && pDialog->isMakeDocumentDefault() && strcmp(pLang, s))
		{
#ifdef ENABLE_SPELL
			FL_DocLayout* pLayout = pView->getLayout();
			
			if(pLayout)
				pLayout->queueAll(FL_DocLayout::bgcrSpelling | FL_DocLayout::bgcrGrammar);
#endif
			pDoc->setProperties(props_out);
		}
		
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

/*****************************************************************/

static bool s_doFontDlg(FV_View * pView)
{
	UT_return_val_if_fail(pView,false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_Dialog_Id id = XAP_DIALOG_ID_FONT;

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_FontChooser * pDialog
		= static_cast<XAP_Dialog_FontChooser *>(pDialogFactory->requestDialog(id));
UT_return_val_if_fail(pDialog, false);
	// stuff the GR_Graphics into the dialog so that it
	// can query the system for font info relative to our
	// context.

	pDialog->setGraphicsContext(pView->getLayout()->getGraphics());

	// get current font info from pView

	const gchar ** props_in = NULL;
	if (pView->getCharFormat(&props_in))
	{
		// stuff font properties into the dialog.
		// for a/p which are constant across the selection (always
		// present) we will set the field in the dialog.  for things
		// which change across the selection, we ask the dialog not
		// to set the field (by passing "").

		const gchar *szFontFamily = UT_getAttribute("font-family", props_in);
		const gchar *szTextTransform = UT_getAttribute("text-transform", props_in);
		const gchar *szFontSize = UT_getAttribute("font-size", props_in);
		const gchar *szFontWeight = UT_getAttribute("font-weight", props_in);
		const gchar *szFontStyle = UT_getAttribute("font-style", props_in);
		const gchar *szColor = UT_getAttribute("color", props_in);
		const gchar *szBGColor = UT_getAttribute("bgcolor", props_in);

		const std::string sFontFamily = (szFontFamily ? szFontFamily : "");
		const std::string sTextTransform = (szTextTransform ? szTextTransform : "");
		const std::string sFontSize = (szFontSize ? szFontSize : "");
		const std::string sFontWeight = (szFontWeight ? szFontWeight : "");
		const std::string sFontStyle = (szFontStyle ? szFontStyle : "");
		const std::string sColor = (szColor ? szColor : "");
		const std::string sBGColor = (szBGColor ? szBGColor : "");

		pDialog->setFontFamily(sFontFamily);
		pDialog->setTextTransform(sTextTransform);
		pDialog->setFontSize(sFontSize);
		pDialog->setFontWeight(sFontWeight);
		pDialog->setFontStyle(sFontStyle);
		pDialog->setColor(sColor);
		pDialog->setBGColor(sBGColor);
	   
//
// Set the background color for the preview
//
		gchar  background[8];
		const UT_RGBColor * bgCol = pView->getCurrentPage()->getFillType().getColor();
		sprintf(background, "%02x%02x%02x",bgCol->m_red,
				bgCol->m_grn,bgCol->m_blu);
		pDialog->setBackGroundColor( static_cast<const gchar *>(background));

		// these behave a little differently since they are
		// probably just check boxes and we don't have to
		// worry about initializing a combo box with a choice
		// (and because they are all stuck under one CSS attribute).

		bool bUnderline = false;
		bool bOverline = false;
		bool bStrikeOut = false;
		bool bTopLine = false;
		bool bBottomLine = false;

		const gchar * s = UT_getAttribute("text-decoration", props_in);
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
		const gchar * h = UT_getAttribute("display", props_in);
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
		pDialog->setSubScript(bSubScript);

		FREEP(props_in);
	}

	if(!pView->isSelectionEmpty())
	{
	    // set the drawable string to the selection text
		// the pointer return by getSelectionText() must be freed
		UT_UCS4Char* text = NULL;
		pView->getSelectionText(text);
		if(text)
		{
			pDialog->setDrawString(text);
			FREEP(text);
		}
	}

	// run the dialog

	pDialog->runModal(pFrame);

	// extract what they did

	bool bOK = (pDialog->getAnswer() == XAP_Dialog_FontChooser::a_OK);

	if (bOK)
	{
		std::vector<std::string> props_out;
		std::string s;

		if (pDialog->getChangedFontFamily(s))
		{
			props_out.push_back("font-family");
			props_out.push_back(s);
		}

		if (pDialog->getChangedTextTransform(s))
		{
			props_out.push_back("text-transform");
			props_out.push_back(s);
		}

		if (pDialog->getChangedFontSize(s))
		{
			props_out.push_back("font-size");
			props_out.push_back(s);
		}

		if (pDialog->getChangedFontWeight(s))
		{
			props_out.push_back("font-weight");
			props_out.push_back(s);
		}

		if (pDialog->getChangedFontStyle(s))
		{
			props_out.push_back("font-style");
			props_out.push_back(s);
		}

		if (pDialog->getChangedColor(s))
		{
			props_out.push_back("color");
			props_out.push_back(s);
		}

		if (pDialog->getChangedBGColor(s))
		{
			props_out.push_back("bgcolor");
			props_out.push_back(s);
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
		std::string decors;
		if (bChangedUnderline || bChangedStrikeOut || bChangedOverline || bChangedTopline || bChangedBottomline)
		{
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
			props_out.push_back("text-decoration");
			props_out.push_back(decors);
		}

		bool bHidden = false;
		bool bChangedHidden = pDialog->getChangedHidden(&bHidden);

		if (bChangedHidden)
		{
			if(bHidden)
			{
				props_out.push_back("display");
				props_out.push_back("none");
			}
			else
			{
				props_out.push_back("display");
				props_out.push_back("inline");
			}
		}

		bool bSuperScript = false;
		bool bChangedSuperScript = pDialog->getChangedSuperScript(&bSuperScript);

		if (bChangedSuperScript)
		{
			if(bSuperScript)
			{
				props_out.push_back("text-position");
				props_out.push_back("superscript");
			}
			else
			{
				props_out.push_back("text-position");
				props_out.push_back("");
			}
		}

		bool bSubScript = false;
		bool bChangedSubScript = pDialog->getChangedSubScript(&bSubScript);

		if (!(bChangedSuperScript && bSuperScript)) /* skip setting subscript if we just enabled superscript */
		{
			if (bChangedSubScript)
			{
				if(bSubScript)
				{
					props_out.push_back("text-position");
					props_out.push_back("subscript");
				}
				else
				{
					props_out.push_back("text-position");
					props_out.push_back("");
				}
			}
		}

		if (!props_out.empty()) {				// if something changed
			pView->setCharFormat(props_out);
		}
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

static void
s_TabSaveCallBack (AP_Dialog_Tab * /*pDlg*/, FV_View * pView,
				   const char * szTabStops, const char * szDflTabStop,
				   void * /*closure*/)
{
  UT_return_if_fail(szTabStops && szDflTabStop);

	const gchar * properties[3];
	properties[0] = "tabstops";
	properties[1] = szTabStops;
	properties[2] = 0;
	UT_DEBUGMSG(("AP_Dialog_Tab: Tab Stop [%s]\n",properties[1]));
	if(szTabStops && *szTabStops)
	{
		pView->setBlockFormat(properties);
	}
	else
	{
		properties[1] = " ";
		pView->setBlockFormat(properties);
	}

	properties[0] = "default-tab-interval";
	properties[1] = szDflTabStop;
	properties[2] = 0;
	UT_return_if_fail (properties[1]);
	UT_DEBUGMSG(("AP_Dialog_Tab: Default Tab Stop [%s]\n",properties[1]));

	pView->setBlockFormat(properties);
}

// non static, so it can be called from the paragraph dialog
bool s_doTabDlg(FV_View * pView)
{

	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_Tab * pDialog
		= static_cast<AP_Dialog_Tab *>(pDialogFactory->requestDialog(AP_DIALOG_ID_TAB));

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
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_Paragraph * pDialog
		= static_cast<AP_Dialog_Paragraph *>(pDialogFactory->requestDialog(AP_DIALOG_ID_PARAGRAPH));
UT_return_val_if_fail(pDialog, false);
	const gchar ** props = NULL;

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

	const gchar ** propitem = NULL;

	switch (answer)
	{
	case AP_Dialog_Paragraph::a_OK:

		// getDialogData() returns us gchar ** data we have to g_free
		pDialog->getDialogData(props);
		UT_return_val_if_fail (props, false);

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

		// now g_free props
		FREEP(props);

		break;

	case AP_Dialog_Paragraph::a_TABS:

		s_doTabDlg(pView);

		break;

	case AP_Dialog_Paragraph::a_CANCEL:
		// do nothing
		break;
	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	pDialogFactory->releaseDialog(pDialog);

	return true;
}


static bool s_doOptionsDlg(FV_View * pView, int which = -1)
{
#ifndef TOOLKIT_COCOA
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());
#else
	UT_UNUSED(pView);

	XAP_Frame * pFrame = 0; // don't necessarily have a frame in Cocoa-FE

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());
#endif
	XAP_TabbedDialog_NonPersistent * pDialog
		= static_cast<XAP_TabbedDialog_NonPersistent *>(pDialogFactory->requestDialog(AP_DIALOG_ID_OPTIONS));
	UT_return_val_if_fail(pDialog, false);

	if ( which != -1 )
	  pDialog->setInitialPageNum(which);
	else
	  pDialog->setInitialPageNum(0);

	// run the dialog
	pDialog->runModal(pFrame);

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
	UT_return_val_if_fail(pView, false);
	const gchar * properties[] = { "lang", NULL, 0};

	char lang[10];
	UT_return_val_if_fail(pCallData->m_dataLength < sizeof(lang),false);

	UT_uint32 i = 0;
	for(i = 0; i < pCallData->m_dataLength; i++)
		lang[i] = static_cast<char>(pCallData->m_pData[i]);
	lang[i] = 0;
	
	properties[1] = static_cast<const gchar *>(&lang[0]);
	pView->setCharFormat(properties);
	return true;
}

/*****************************************************************/

Defun1(dlgFont)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView, false);
	if (pView->getDocument()->areStylesLocked())
		return true;

	return s_doFontDlg(pView);
}

Defun(fontFamily)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	
	UT_return_val_if_fail(pView, false);
	const gchar * properties[] = { "font-family", NULL, 0};
	UT_UTF8String utf8(pCallData->m_pData, pCallData->m_dataLength);
	properties[1] = reinterpret_cast<const gchar *>(utf8.utf8_str());
	pView->setCharFormat(properties);
	
	return true;
}

Defun(fontSize)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView, false);	
	const gchar * properties[] = { "font-size", NULL, 0};
	UT_UTF8String utf8(pCallData->m_pData, pCallData->m_dataLength);	
	const gchar * sz = reinterpret_cast<const gchar *>(utf8.utf8_str());

	if (sz && *sz)
	{
		UT_String buf (sz);
		buf += "pt";

		properties[1] = static_cast<const gchar *>(buf.c_str());
		pView->setCharFormat(properties);
	}
	return true;
}

static bool _fontSizeChange(FV_View * pView, bool bIncrease)
{
	UT_return_val_if_fail(pView, false);
	const gchar ** span_props = NULL;
	const gchar * properties[] = { "font-size", NULL, 0};
	
	pView->getCharFormat(&span_props);
	UT_return_val_if_fail(span_props, false);
	
	const gchar * s = UT_getAttribute("font-size", span_props);

	if(!s)
		return false;

	double dPoints = UT_convertToPoints(s);
	FREEP(span_props);
	
#define PT_INC_SMALL  1.0
#define PT_INC_MEDIUM 2.0
#define PT_INC_LARGE  4.0
	
	if(bIncrease)
	{
		if(dPoints >= 26.0)
			dPoints += PT_INC_LARGE;
		else if(dPoints >= 8.0)
			dPoints += PT_INC_MEDIUM;
		else
			dPoints += PT_INC_SMALL;
	}
	else
	{
		if(dPoints > 26.0)
			dPoints -= PT_INC_LARGE;
		else if(dPoints > 8.0)
			dPoints -= PT_INC_MEDIUM;
		else
			dPoints -= PT_INC_SMALL;
		
	}

#undef PT_INC_SMALL
#undef PT_INC_MEDIUM
#undef PT_INC_LARGE

	// make sure that we do not decrease fonts too far ...
	if(dPoints < 2.0)
		return false;
	
	const gchar * sz = UT_formatDimensionString(DIM_PT, dPoints);

	if(!sz || !*sz)
		return false;

	properties[1] = sz;
	pView->setCharFormat(properties);

	return true;
}

Defun1(fontSizeIncrease)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	
	return _fontSizeChange(pView, true);
}

Defun1(fontSizeDecrease)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	
	return _fontSizeChange(pView, false);
}

Defun1(cairoPrint)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Cairo Print \n"));
	UT_return_val_if_fail (pView, false);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_Print * pDialog
		= static_cast<XAP_Dialog_Print *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_PRINT));
	pView->setCursorWait();
	pDialog->setPreview(false);
	pDialog->runModal(pFrame);
	GR_Graphics * pGraphics = pDialog->getPrinterGraphicsContext();
	pDialog->releasePrinterGraphicsContext(pGraphics);
	pView->clearCursorWait();
	s_pLoadingFrame = NULL;
	pView->setPoint(pView->getPoint());
	pView->updateScreen(false);
	pDialogFactory->releaseDialog(pDialog);
	return true;
}


Defun1(cairoPrintPreview)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Cairo Print Preview\n"));
	UT_return_val_if_fail (pView, false);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_Print * pDialog
		= static_cast<XAP_Dialog_Print *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_PRINT));
	pView->setCursorWait();
	pDialog->setPreview(true);
	pDialog->runModal(pFrame);
	GR_Graphics * pGraphics = pDialog->getPrinterGraphicsContext();
	pDialog->releasePrinterGraphicsContext(pGraphics);
	pView->clearCursorWait();
	s_pLoadingFrame = NULL;
	pView->setPoint(pView->getPoint());
	pView->updateScreen(false);
	pDialogFactory->releaseDialog(pDialog);
	return true;
}


Defun1(cairoPrintDirectly)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Cairo Print Directly\n"));
	UT_return_val_if_fail (pView, false);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_Print * pDialog
		= static_cast<XAP_Dialog_Print *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_PRINT));
	pView->setCursorWait();
	pDialog->setPreview(false);
	//
	// DOM you can use this for your command line printing
	//
	pDialog->PrintDirectly(pFrame,/*filename*/ NULL, /*printer name */NULL);
	GR_Graphics * pGraphics = pDialog->getPrinterGraphicsContext();
	pDialog->releasePrinterGraphicsContext(pGraphics);
	pView->clearCursorWait();
	s_pLoadingFrame = NULL;
	pView->updateScreen(false);
	pDialogFactory->releaseDialog(pDialog);
	return true;
	return true;
}

Defun1(formatPainter)
{
  CHECK_FRAME;
  ABIWORD_VIEW;

  // prereqs: !pView->isSelectionEmpty() && XAP_App::getApp()->canPasteFromClipboard()
  // taken care of in ap_Toolbar_Functions.cpp::ap_ToolbarGetState_Clipboard

  UT_return_val_if_fail(pView, false);
  const gchar ** block_properties = 0;
  const gchar ** span_properties  = 0;

  // get the current document's selected range
  PD_DocumentRange range;
  pView->getDocumentRangeOfCurrentSelection (&range);

  // now create a new (invisible) view to paste our clipboard contents into
  PD_Document * pNewDoc = new PD_Document();
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
  FREEP(span_properties);
  DELETEP(pDocLayout);
  UNREFP(pNewDoc);

  return true;
}

/*****************************************************************/

static bool _toggleSpanOrBlock(FV_View * pView,
				  const gchar * prop,
				  const gchar * vOn,
				  const gchar * vOff,
				  bool bMultiple,
				  bool isSpan)
{
	UT_return_val_if_fail(pView, false);
	if (pView->getDocument()->areStylesLocked())
		return true;

	const gchar * props_out[] =	{ NULL, NULL, 0};

	// get current font info from pView
	const gchar ** props_in = NULL;
	const gchar * s;

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

	gchar * buf = NULL;

	s = UT_getAttribute(prop, props_in);
	if (s)
	{
		if (bMultiple)
		{
			// some properties have multiple values
			const gchar*	p = strstr(s, vOn);

			if (p)
			{
				// yep...
				if (strstr(s, vOff))
				{
					UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				}

				// ... take it out
				int len = strlen(s);
				buf = static_cast<gchar *>(UT_calloc(len, sizeof(gchar)));

				strncpy(buf, s, p - s);
				strcat(buf, s + (p - s) + strlen(vOn));

				// now see if anything's left
				gchar * q  = g_strdup(buf);

				if (q && strtok(q, " "))
					props_out[1] = buf; 	// yep, use it
				else
					props_out[1] = vOff;	// nope, clear it

				g_free(q);
			}
			else
			{
				// nope...
				if (g_ascii_strcasecmp(s, vOff))
				{
					// ...put it in by appending to current contents
					int len = strlen(s) + strlen(vOn) + 2;
					buf = static_cast<gchar *>(UT_calloc(len, sizeof(gchar)));

					strcpy(buf, s);
					strcat(buf, " ");
					strcat(buf, vOn);
					props_out[1] = buf;
				}
			}
		}
		else
		{
			if (0 == g_ascii_strcasecmp(s, vOn))
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
			   const gchar * prop,
			   const gchar * vOn,
			   const gchar * vOff,
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
		     UT_sint32 nToPage, UT_sint32 nFromPage)
{
	std::set<UT_sint32> pages;
	for (UT_sint32 i = nFromPage; i <= nToPage; i++)
		{
			pages.insert(i);
		}

	return s_actuallyPrint(doc, pGraphics, pPrintView, pDocName, 
						   nCopies, bCollate, iWidth, iHeight, pages);
}

bool s_actuallyPrint(PD_Document *doc,  GR_Graphics *pGraphics,
		     FV_View * pPrintView, const char *pDocName,
		     UT_uint32 nCopies, bool bCollate,
		     UT_sint32 iWidth,  UT_sint32 iHeight,
		     const std::set<UT_sint32>& pages)
{
	UT_uint32 i,j,k;

	//
	// Lock out operations on this document
	//
	s_pLoadingDoc = static_cast<AD_Document *>(doc);

	if(pGraphics->startPrint())
	{
	  fp_PageSize ps = pPrintView->getPageSize();	  
	  bool orient = ps.isPortrait ();
	  pGraphics->setPortrait (orient);

	  const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet ();
	  const gchar * msgTmpl = pSS->getValue (AP_STRING_ID_MSG_PrintStatus);

	  gchar msgBuf [1024];

	  dg_DrawArgs da;
	  memset(&da, 0, sizeof(da));
	  da.pG = pGraphics;
	  
	  XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame ();

		if (bCollate)
		{
			for (j=1; (j <= nCopies); j++)
				{
					i = 0;
					for (std::set<UT_sint32>::const_iterator page = pages.begin();
						 page != pages.end();
						 page++)
						{
							i++;
							k = *page;
							sprintf (msgBuf, msgTmpl, i, pages.size());
							
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
		}
		else
		{
			i = 0;
			for (std::set<UT_sint32>::const_iterator page = pages.begin();
				 page != pages.end();
				 page++)
				{
					k = *page;
					i++;

					for (j=1; (j <= nCopies); j++)
						{
							sprintf (msgBuf, msgTmpl, i, pages.size());
							
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
		}
		pGraphics->endPrint();
		
		if(pFrame)
		  pFrame->setStatusMessage (""); // reset/0 out status bar
	}
	s_pLoadingDoc = NULL;

	return true;
}

#ifdef ENABLE_PRINT
#if defined(TOOLKIT_COCOA)
/* declare but possibly not implment them */
bool s_doPrint(FV_View * pView, bool bTryToSuppressDialog, bool bPrintDirectly);
#else
static bool s_doPrint(FV_View * pView, bool bTryToSuppressDialog,bool bPrintDirectly)
{
	UT_return_val_if_fail (pView, false);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
	UT_return_val_if_fail(pFrameData, false);

	if (pView->getViewMode() != VIEW_PRINT)
	{
		/* if the current view is in normal mode, switch to print layout
		 * first, this ensure that the printed layout is as it should be.
		 * We force screen update here, otherwise the screen is messed up
		 * under the print dialog
		 */
		pFrameData->m_pViewMode = VIEW_PRINT;
		pView->setViewMode (VIEW_PRINT);
		pView->updateScreen (false);
	}
	
	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_Print * pDialog
		= static_cast<XAP_Dialog_Print *>(pDialogFactory->requestDialog(bPrintDirectly? XAP_DIALOG_ID_PRINT_DIRECTLY: XAP_DIALOG_ID_PRINT));
UT_return_val_if_fail(pDialog, false);
	FL_DocLayout* pLayout = pView->getLayout();
	PD_Document * doc = pLayout->getDocument();

	pDialog->setPaperSize (pView->getPageSize().getPredefinedName());
	pDialog->setDocumentTitle(pFrame->getNonDecoratedTitle());
	pDialog->setDocumentPathname((!doc->getFilename().empty())
								 ? doc->getFilename().c_str()
								 : pFrame->getNonDecoratedTitle());
	pDialog->setEnablePageRangeButton(true,1,pLayout->countPages());
	pDialog->setEnablePrintSelection(false);	// TODO change this when we know how to do it.
	pDialog->setEnablePrintToFile(true);
	pDialog->setTryToBypassActualDialog(bTryToSuppressDialog);

	pDialog->runModal(pFrame);

	XAP_Dialog_Print::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_Print::a_OK);
	bool bHideFmtMarks = false;

	if (bOK)
	{

//
// Turn on Wait cursor
//
		pView->setCursorWait();
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
		UT_String msg (pSS->getValue(AP_STRING_ID_MSG_PrintingDoc));

		pFrame->setStatusMessage ( static_cast<const gchar *>(msg.c_str()) );

		GR_Graphics * pGraphics = pDialog->getPrinterGraphicsContext();

		if (!pGraphics)
		{
			pFrame->showMessageBox(AP_STRING_ID_PRINT_CANNOTSTARTPRINTJOB,
				   XAP_Dialog_MessageBox::b_O,
				   XAP_Dialog_MessageBox::a_OK);

		   return false;
		}

		UT_return_val_if_fail (pGraphics->queryProperties(GR_Graphics::DGP_PAPER), false);

		/*
		We need to re-layout the document for now, so the UnixPSGraphics class will
		get it's font list filled. When we find a better way to fill the UnixPSGraphics
		font list, we can remove the 4 lines below. - MARCM
		*/
		//
		FL_DocLayout * pDocLayout = NULL;
		FV_View * pPrintView = NULL;
		bool canQuickPrint = pGraphics->canQuickPrint();
		if(!canQuickPrint)
		{
				pDocLayout = new FL_DocLayout(doc,pGraphics);
				pPrintView = new FV_View(XAP_App::getApp(),0,pDocLayout);
				pPrintView->getLayout()->fillLayouts();
				pPrintView->getLayout()->formatAll();
				pPrintView->getLayout()->recalculateTOCFields();
		}
		else
		{
				pDocLayout = pLayout;
				pPrintView = pView;
				pDocLayout->setQuickPrint(pGraphics);
				if(pFrameData->m_bShowPara)
				{
					pPrintView->setShowPara(false);
					bHideFmtMarks = true;
				}
		}

		UT_sint32 nFromPage, nToPage;
		static_cast<void>(pDialog->getDoPrintRange(&nFromPage,&nToPage));

		// must use the layout of the print view here !!!
		if (nToPage > pPrintView->getLayout()->countPages())
		  nToPage = pPrintView->getLayout()->countPages();

		// TODO add code to handle getDoPrintSelection()

		UT_uint32 nCopies = pDialog->getNrCopies();
		bool bCollate = pDialog->getCollate();

		// TODO these are here temporarily to make printing work.  We'll fix the hack later.
		// BUGBUG assumes all pages are same size and orientation
		// Must use the layout create with printer graphics here, because the screen
		// layout adds screen margins to the width and height
		UT_sint32 iWidth = pDocLayout->getWidth();
		UT_sint32 iHeight = pDocLayout->getHeight() / pDocLayout->countPages();

		const char *pDocName = ((!doc->getFilename().empty()) ? doc->getFilename().c_str() : pFrame->getNonDecoratedTitle());
		s_actuallyPrint(doc, pGraphics, pPrintView, pDocName, nCopies, bCollate,
				iWidth,  iHeight, nToPage, nFromPage);

		if(!canQuickPrint)
		{
			delete pDocLayout;
			delete pPrintView;
		}
		else
		{
			if(bHideFmtMarks)
				pPrintView->setShowPara(true);

			pDocLayout->setQuickPrint(NULL);
		}
		pDialog->releasePrinterGraphicsContext(pGraphics);

//
// Turn off wait cursor
//
		pView->clearCursorWait();
		s_pLoadingFrame = NULL;
		pView->updateScreen(false);
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}
#endif
#endif

#ifdef ENABLE_PRINT
static bool s_doPrintPreview(FV_View * pView)
{
	UT_return_val_if_fail(pView, false);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
	UT_return_val_if_fail(pFrameData, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_PrintPreview * pDialog
		= static_cast<XAP_Dialog_PrintPreview *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_PRINTPREVIEW));
	UT_return_val_if_fail(pDialog, false);
	FL_DocLayout* pLayout = pView->getLayout();
	PD_Document * doc = pLayout->getDocument();

    // Turn on Wait cursor
	pView->setCursorWait();

	pDialog->setPaperSize (pView->getPageSize().getPredefinedName());
	pDialog->setDocumentTitle(pFrame->getNonDecoratedTitle());
	pDialog->setDocumentPathname((!doc->getFilename().empty())
								 ? doc->getFilename().c_str()
								 : pFrame->getNonDecoratedTitle());

	pDialog->runModal(pFrame);

	GR_Graphics * pGraphics = pDialog->getPrinterGraphicsContext();
	if (!(pGraphics && pGraphics->queryProperties(GR_Graphics::DGP_PAPER)))
		{
			UT_ASSERT_HARMLESS(pGraphics);
			UT_ASSERT_HARMLESS(pGraphics->queryProperties(GR_Graphics::DGP_PAPER));
			
			pDialogFactory->releaseDialog(pDialog);
			
			// Turn off wait cursor
			pView->clearCursorWait();

			return false;
		}

	/*
	We need to re-layout the document for now, so the UnixPSGraphics class will
	get it's font list filled. When we find a better way to fill the UnixPSGraphics
	font list, we can remove the 4 lines below. - MARCM
	*/
	FL_DocLayout * pDocLayout = NULL;
	FV_View * pPrintView = NULL;
	bool bHideFmtMarks = false;
	bool bDidQuickPrint = false;
	if(!pGraphics->canQuickPrint() || (pView->getViewMode() != VIEW_PRINT))
	{
			pDocLayout = new FL_DocLayout(doc,pGraphics);
			pPrintView = new FV_View(XAP_App::getApp(),0,pDocLayout);
			pPrintView->setViewMode(VIEW_PRINT);
			pPrintView->getLayout()->fillLayouts();
			pPrintView->getLayout()->formatAll();
			pPrintView->getLayout()->recalculateTOCFields();
	}
	else
	{
			pDocLayout = pLayout;
			pPrintView = pView;
			pDocLayout->setQuickPrint(pGraphics);
			bDidQuickPrint = true;
			if(pFrameData->m_bShowPara)
			{
				pPrintView->setShowPara(false);
				bHideFmtMarks = true;
			}
	}
	
	UT_uint32 nFromPage = 1, nToPage = pLayout->countPages(), nCopies = 1;
	bool bCollate  = false;

	// TODO these are here temporarily to make printing work.  We'll fix the hack later.
	// BUGBUG assumes all pages are same size and orientation
	UT_sint32 iWidth = pDocLayout->getWidth();
	UT_sint32 iHeight = pDocLayout->getHeight() / pDocLayout->countPages();

	const char *pDocName = ((!doc->getFilename().empty()) ? doc->getFilename().c_str() : pFrame->getNonDecoratedTitle());

	s_actuallyPrint(doc, pGraphics, pPrintView, pDocName, nCopies, bCollate,
					iWidth,  iHeight, nToPage, nFromPage);

	if(!bDidQuickPrint)
	{
			delete pDocLayout;
			delete pPrintView;
	}
	else
	{
		if(bHideFmtMarks)
			pPrintView->setShowPara(true);

		pDocLayout->setQuickPrint(NULL);
	}
	pDialog->releasePrinterGraphicsContext(pGraphics);

	pDialogFactory->releaseDialog(pDialog);

    // Turn off wait cursor
	pView->clearCursorWait();
	
	return true;
}
#endif

static bool s_doZoomDlg(FV_View * pView)
{
	UT_return_val_if_fail(pView, false);
	UT_String tmp;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	UT_return_val_if_fail (pPrefsScheme, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_Zoom * pDialog
		= static_cast<XAP_Dialog_Zoom *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_ZOOM));
	UT_return_val_if_fail(pDialog, false);

	pDialog->setZoomPercent(pFrame->getZoomPercentage());
	pDialog->setZoomType(pFrame->getZoomType());

	pDialog->runModal(pFrame);

	switch (pDialog->getZoomType())
	{
	case XAP_Frame::z_PAGEWIDTH:
		pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
				       static_cast<const gchar*>("Width"));
		break;
	case XAP_Frame::z_WHOLEPAGE:
		pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
				       static_cast<const gchar*>("Page"));
		break;
	default:
		{
			UT_UTF8String percent = UT_UTF8String_sprintf("%lu", static_cast<unsigned long>(pDialog->getZoomPercent()));
			pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
					       static_cast<const gchar*>(percent.utf8_str()));
		}
		break;
	}
	pFrame->setZoomType(pDialog->getZoomType());
	pFrame->quickZoom(pDialog->getZoomPercent());

	// Zoom is instant-apply, no need to worry about processing the
	// OK/cancel state of the dialog.  Just release it.
	pDialogFactory->releaseDialog(pDialog);
	return true;
}

Defun1(zoom100)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	UT_return_val_if_fail (pPrefsScheme, false);
	pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
						   static_cast<const gchar*>("100"));

  pFrame->raise();

  UT_uint32 newZoom = 100;
  pFrame->setZoomType( XAP_Frame::z_100 );
  pFrame->quickZoom(newZoom);
  return true;
}

Defun1(zoom200)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
  UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	UT_return_val_if_fail (pPrefsScheme, false);
	pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
						   static_cast<const gchar*>("200"));

  pFrame->raise();

  UT_uint32 newZoom = 200;
  pFrame->setZoomType( XAP_Frame::z_200 );
  pFrame->quickZoom(newZoom);

  return true;
}

Defun1(zoom50)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
  UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
UT_return_val_if_fail(pPrefsScheme, false);	pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
						   static_cast<const gchar*>("50"));

  pFrame->raise();

  UT_uint32 newZoom = 50;
  pFrame->setZoomType( XAP_Frame::z_PERCENT );
  pFrame->quickZoom(newZoom);

  return true;
}

Defun1(zoom75)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
  UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
UT_return_val_if_fail(pPrefsScheme, false);	pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
						   static_cast<const gchar*>("75"));

  pFrame->raise();

  UT_uint32 newZoom = 75;
  pFrame->setZoomType(	XAP_Frame::z_75 );
  pFrame->quickZoom(newZoom);

  return true;
}

Defun1(zoomWidth)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
  XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
  UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
UT_return_val_if_fail(pPrefsScheme, false);	pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
						   static_cast<const gchar*>("Width"));

  pFrame->raise();

  pFrame->setZoomType( XAP_Frame::z_PAGEWIDTH );

  UT_uint32 newZoom = pView->calculateZoomPercentForPageWidth();
  pFrame->quickZoom(newZoom);


  return true;
}

Defun1(zoomWhole)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
UT_return_val_if_fail(pPrefsScheme, false);	pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
						   static_cast<const gchar*>("Page"));


  pFrame->raise();

  pFrame->setZoomType( XAP_Frame::z_WHOLEPAGE );

  UT_uint32 newZoom = pView->calculateZoomPercentForWholePage();
  pFrame->quickZoom(newZoom);

  return true;
}

Defun1(zoomIn)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	
	pFrame->raise();
	UT_uint32 newZoom = UT_MIN(pFrame->getZoomPercentage() + 10, XAP_DLG_ZOOM_MAXIMUM_ZOOM);
	UT_String tmp (UT_String_sprintf("%d",newZoom));
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
UT_return_val_if_fail(pPrefsScheme, false);	pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
						 static_cast<const gchar*>(tmp.c_str()));
	
	pFrame->setZoomType( XAP_Frame::z_PERCENT );
	pFrame->quickZoom(newZoom);

	return true;
}

Defun1(zoomOut)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	
	pFrame->raise();
	
	UT_uint32 newZoom = UT_MAX(pFrame->getZoomPercentage() - 10, XAP_DLG_ZOOM_MINIMUM_ZOOM);
	UT_String tmp (UT_String_sprintf("%d",newZoom));
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
UT_return_val_if_fail(pPrefsScheme, false);	pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
						 static_cast<const gchar*>(tmp.c_str()));
	pFrame->setZoomType( XAP_Frame::z_PERCENT );
	pFrame->quickZoom(newZoom);

	
	return true;
}

static bool s_doBreakDlg(FV_View * pView)
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	if(pView->isHdrFtrEdit())
		return false;

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_Break * pDialog
		= static_cast<AP_Dialog_Break *>(pDialogFactory->requestDialog(AP_DIALOG_ID_BREAK));
	UT_return_val_if_fail(pDialog, false);
	pDialog->runModal(pFrame);

	AP_Dialog_Break::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_Break::a_OK);

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
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

#ifdef ENABLE_PRINT
static bool s_doPageSetupDlg (FV_View * pView)
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);

	pFrame->raise();
	XAP_DialogFactory * pDialogFactory
	  = static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_PageSetup * pDialog =
	  static_cast<AP_Dialog_PageSetup *>(pDialogFactory->requestDialog(AP_DIALOG_ID_FILE_PAGESETUP));

	UT_return_val_if_fail(pDialog, false);
	PD_Document * pDoc = pView->getLayout()->getDocument();
	//
	// Need this for the conversion methods
	//
	fp_PageSize::Predefined orig_def,final_def;
	double orig_wid = -1, orig_ht = -1, final_wid = -1, final_ht = -1;
	UT_Dimension orig_ut = DIM_IN, final_ut = DIM_IN;
	fp_PageSize pSize(pDoc->getPageSize()->getPredefinedName());
	orig_def = pSize.NameToPredefined(pSize.getPredefinedName());
	//
	// Set first page of the dialog properties.
	//
	AP_Dialog_PageSetup::Orientation orig_ori,final_ori;
	orig_ori =	AP_Dialog_PageSetup::PORTRAIT;
	if(pDoc->getPageSize()->isPortrait() == false)
	{
		orig_ori = AP_Dialog_PageSetup::LANDSCAPE;
	}
	if (orig_def == fp_PageSize::psCustom)
	{
		orig_ut = pDoc->getPageSize()->getDims();
		orig_wid = pDoc->getPageSize()->Width(orig_ut);
		orig_ht = pDoc->getPageSize()->Height(orig_ut);
		if(orig_ori == AP_Dialog_PageSetup::LANDSCAPE)
		{
			pSize.Set(orig_ht, orig_wid, orig_ut);
		}
		else
		{
			pSize.Set(orig_wid, orig_ht, orig_ut);
		}
	}
	pDialog->setPageSize(pSize);
	pDialog->setPageOrientation(orig_ori);
	UT_Dimension orig_margu,final_margu;
	double orig_scale,final_scale;
	orig_scale = pDoc->getPageSize()->getScale();

	// respect units set in the dialogue constructer from prefs
	UT_Dimension orig_uprefs = DIM_IN;
	const gchar * szRulerUnits;
	if (pApp->getPrefsValue(AP_PREF_KEY_RulerUnits,&szRulerUnits))
	{
		// we only allow in, cm, mm in the dlg
		UT_Dimension units = UT_determineDimension(szRulerUnits);
		if(units == DIM_CM || units == DIM_MM || units == DIM_IN)
		{
			orig_uprefs = units;
		}
	}

	// make sure that the units in the dlg are the same as in the prefs
	pDialog->setPageUnits(orig_uprefs);
	pDialog->setMarginUnits(orig_uprefs);
	
	pDialog->setPageScale(static_cast<int>(100.0*orig_scale));

	//
	// Set the second page of info
	// All the page and header/footer margins
	//
	const gchar ** props_in = NULL;
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

	bool bResult = pView->getSectionFormat(&props_in);
	if (!bResult)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}
	if(props_in && props_in[0])
	{
		pszLeftMargin = UT_getAttribute("page-margin-left", props_in);
		if(pszLeftMargin)
		{
			dLeftMargin = UT_convertToInches(pszLeftMargin);
		}

		pszRightMargin = UT_getAttribute("page-margin-right", props_in);
		if(pszRightMargin)
		{
			dRightMargin = UT_convertToInches(pszRightMargin);
		}

		pszTopMargin = UT_getAttribute("page-margin-top", props_in);
		if(pszTopMargin)
		{
			dTopMargin = UT_convertToInches(pszTopMargin);
		}

		pszBottomMargin = UT_getAttribute("page-margin-bottom", props_in);
		if(pszBottomMargin)
		{
			dBottomMargin = UT_convertToInches(pszBottomMargin);
		}

		pszFooterMargin = UT_getAttribute("page-margin-footer", props_in);
		if(pszFooterMargin)
			dFooterMargin = UT_convertToInches(pszFooterMargin);

		pszHeaderMargin = UT_getAttribute("page-margin-header", props_in);
		if(pszHeaderMargin)
			dHeaderMargin = UT_convertToInches(pszHeaderMargin);
	}
	FREEP(props_in);
	orig_margu = pDialog->getMarginUnits();
	if(orig_margu == DIM_MM)
	{
		dLeftMargin = dLeftMargin * 25.4;
		dRightMargin = dRightMargin * 25.4;
		dTopMargin = dTopMargin * 25.4;
		dBottomMargin = dBottomMargin * 25.4;
		dFooterMargin = dFooterMargin * 25.4;
		dHeaderMargin = dHeaderMargin * 25.4;
	}
	else if(orig_margu == DIM_CM)
	{
		dLeftMargin = dLeftMargin * 2.54;
		dRightMargin = dRightMargin * 2.54;
		dTopMargin = dTopMargin * 2.54;
		dBottomMargin = dBottomMargin * 2.54;
		dFooterMargin = dFooterMargin * 2.54;
		dHeaderMargin = dHeaderMargin * 2.54;
	}

	//
	// OK set all page two stuff
	//
	// do not set units -- they have not changed
	// pDialog->setMarginUnits(orig_margu);
	pDialog->setMarginTop(static_cast<float>(dTopMargin));
	pDialog->setMarginBottom(static_cast<float>(dBottomMargin));
	pDialog->setMarginLeft(static_cast<float>(dLeftMargin));
	pDialog->setMarginRight(static_cast<float>(dRightMargin));
	pDialog->setMarginHeader(static_cast<float>(dHeaderMargin));
	pDialog->setMarginFooter(static_cast<float>(dFooterMargin));

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
	final_scale = pDialog->getPageScale()/100.0;
	pSize.Set(final_def);

	if (final_def == fp_PageSize::psCustom)
	{
		final_ut = pDialog->getPageSize().getDims();
		final_wid = pDialog->getPageSize().Width(final_ut);
		final_ht = pDialog->getPageSize().Height(final_ut);
	}

	if((final_def != orig_def) || (final_ori != orig_ori) || ((final_scale-orig_scale) > 0.001) || ((final_scale-orig_scale) < -0.001) || (orig_ht != final_ht) || (orig_wid != final_wid) || (orig_ut != final_ut) )
	{
		final_wid = pDialog->getPageSize().Width(final_ut);
		final_ht = pDialog->getPageSize().Height(final_ut);
		//
		// Set the new Page Stuff
		//
		const gchar * szAttr[14] = {"pagetype",NULL,
									"orientation",NULL,
									"width",NULL,
									"height",NULL,
									"units",NULL,
									"page-scale",NULL,
									NULL,NULL};
		UT_UTF8String sType,sOri,sWidth,sHeight,sUnits,sScale;
		sType = pSize.getPredefinedName();
		sUnits = UT_dimensionName(final_ut);
		sWidth = UT_formatDimensionString(final_ut,final_wid);
		sHeight = UT_formatDimensionString(final_ut,final_ht);
		sScale = UT_formatDimensionString(DIM_none,final_scale);
		bool p = (final_ori == AP_Dialog_PageSetup::PORTRAIT);
		if(p)
			sOri = "portrait";
		else
			sOri = "landscape";
		szAttr[1] = sType.utf8_str();
		szAttr[3] = sOri.utf8_str();
		szAttr[5] = sWidth.utf8_str();
		szAttr[7] = sHeight.utf8_str();
		szAttr[9] = sUnits.utf8_str();
		szAttr[11] = sScale.utf8_str();
#ifdef DEBUG
		for (const gchar ** a = szAttr; (*a); a++)
			{
				UT_DEBUGMSG(("apEditMethods attrib %s value %s \n",a[0],a[1]));
				a++;
			}
#endif
		pDoc->setPageSizeFromFile(szAttr);
	}

	// I am not entirely sure about this; perhaps the units should only be modifiable
	// through the prefs dialogue, not through the page setup
	XAP_Prefs * pPrefs = pApp->getPrefs();
	if(!pPrefs)
	{
		UT_ASSERT_HARMLESS(pPrefs);
		DELETEP(pDialog);
		return false;
	}

	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
	if(!pPrefsScheme)
	{
		UT_ASSERT_HARMLESS(pPrefsScheme);
		DELETEP(pDialog);
		return false;
	}

	//
	// Recover ppView
	//
	FV_View * ppView = static_cast<FV_View *>(pFrame->getCurrentView());
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

	pPrefsScheme->setValue(static_cast<const gchar *>(AP_PREF_KEY_RulerUnits),
						   static_cast<const gchar *>(UT_dimensionName(final_margu)));

	dTopMargin = static_cast<double>(pDialog->getMarginTop());
	dBottomMargin = static_cast<double>(pDialog->getMarginBottom());
	dLeftMargin = static_cast<double>(pDialog->getMarginLeft());
	dRightMargin = static_cast<double>(pDialog->getMarginRight());
	dHeaderMargin = static_cast<double>(pDialog->getMarginHeader());
	dFooterMargin = static_cast<double>(pDialog->getMarginFooter());

	//
	// Convert them into const char strings and change the section format
	//
	UT_GenericVector<const gchar*> v;
	//szLeftMargin = UT_convertInchesToDimensionString(docMargUnits,dLeftMargin);
	szLeftMargin = UT_formatDimensionString(final_margu,dLeftMargin);
	v.addItem("page-margin-left");
	v.addItem(szLeftMargin.c_str());

	//szRightMargin = UT_convertInchesToDimensionString(docMargUnits,dRightMargin);
	szRightMargin = UT_formatDimensionString(final_margu,dRightMargin);
	v.addItem("page-margin-right");
	v.addItem(szRightMargin.c_str());

	//szTopMargin = UT_convertInchesToDimensionString(docMargUnits,dTopMargin);
	szTopMargin = UT_formatDimensionString(final_margu,dTopMargin);
	v.addItem("page-margin-top");
	v.addItem(szTopMargin.c_str());

	//szBottomMargin = UT_convertInchesToDimensionString(docMargUnits,dBottomMargin);
	szBottomMargin = UT_formatDimensionString(final_margu,dBottomMargin);
	v.addItem("page-margin-bottom");
	v.addItem(szBottomMargin.c_str());

	//szFooterMargin = UT_convertInchesToDimensionString(docMargUnits,dFooterMargin);
	szFooterMargin = UT_formatDimensionString(final_margu,dFooterMargin);
	v.addItem("page-margin-footer");
	v.addItem(szFooterMargin.c_str());

	//szHeaderMargin = UT_convertInchesToDimensionString(docMargUnits,dHeaderMargin);
	szHeaderMargin = UT_formatDimensionString(final_margu,dHeaderMargin);
	v.addItem("page-margin-header");
	v.addItem(szHeaderMargin.c_str());

	UT_uint32 countv = v.getItemCount() + 1;
	const gchar ** props = static_cast<const gchar **>(UT_calloc(countv, sizeof(gchar *)));
	if(!props)
	{
		UT_ASSERT_HARMLESS(props);
		DELETEP(pDialog);
		return false;
	}

	UT_sint32 i;
	for(i=0; i<v.getItemCount();i++)
	{
		props[i] = v.getNthItem(i);
	}
	props[i] = static_cast<gchar *>(NULL);
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
#endif

class ABI_EXPORT FV_View_Insert_symbol_listener : public XAP_Insert_symbol_listener
	{
	public:

		void setView( AV_View * pJustFocussedView)
			{
			p_view = static_cast<FV_View *>(pJustFocussedView) ;
			}
		bool insertSymbol(UT_UCSChar Char, const char *p_font_name)
		{
			UT_return_val_if_fail (p_view != NULL, false);

			p_view->insertSymbol(Char, p_font_name);

			return true;
		}

	private:
		FV_View *p_view;
	};


static	FV_View_Insert_symbol_listener symbol_Listener;

static bool s_InsertSymbolDlg(FV_View * pView, XAP_Dialog_Id id  )
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();
	XAP_DialogFactory * pDialogFactory
	  = static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	XAP_Dialog_Insert_Symbol * pDialog
		= static_cast<XAP_Dialog_Insert_Symbol *>(pDialogFactory->requestDialog(id));
UT_return_val_if_fail(pDialog, false);
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
#ifdef ENABLE_PRINT
	return s_doPrint(pView,false,false);
#else
    UT_UNUSED(pView);
    return false;
#endif
}


#ifdef ENABLE_PRINT
Defun1(printDirectly)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	return s_doPrint(pView,false,true);
}
#endif


#ifdef ENABLE_PRINT
Defun1(printTB)
{
	CHECK_FRAME;
// print (intended to be from the tool-bar (where we'd like to
	// suppress the dialog if possible))

	ABIWORD_VIEW;
	return s_doPrint(pView,true,false);
}
#endif


#ifdef ENABLE_PRINT
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
#endif

Defun1(dlgPlugins)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();
	XAP_DialogFactory * pDialogFactory
	  = static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_PluginManager * pDialog
		= static_cast<XAP_Dialog_PluginManager *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_PLUGIN_MANAGER));
UT_return_val_if_fail(pDialog, false);
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

#ifdef ENABLE_SPELL
Defun1(dlgSpellPrefs)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	
#ifdef TOOLKIT_COCOA
    return s_doOptionsDlg(pView, 4); // spelling tab
#endif

#if !defined (TOOLKIT_WIN) && !defined (TOOLKIT_GTK_ALL)
    return s_doOptionsDlg(pView, 1); // spelling tab
#else
    // spelling tab in Windows in the tab num 2
    // becuase 1, is language selection. For UNIX, it's
    // tab 2 as well. We should use an enumerator instead
    // of fixed values. Jordi,
	return s_doOptionsDlg(pView, 2);
#endif
}
#endif

/*****************************************************************/
/*****************************************************************/

/* the array below is a HACK. FIXME */
static const gchar* s_TBPrefsKeys [] = {
#if XAP_SIMPLE_TOOLBAR
	AP_PREF_KEY_SimpleBarVisible,
#else	
	AP_PREF_KEY_StandardBarVisible,
	AP_PREF_KEY_FormatBarVisible,
	AP_PREF_KEY_TableBarVisible,
	AP_PREF_KEY_ExtraBarVisible
#endif		
};

static bool
_viewTBx(AV_View* pAV_View, int num) 
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_return_val_if_fail (pFrameData, false);

	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
	  return false;

	// toggle the ruler bit
	pFrameData->m_bShowBar[num] = ! pFrameData->m_bShowBar[num];

	// actually do the dirty work
	pFrame->toggleBar(num, pFrameData->m_bShowBar[num] );

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_return_val_if_fail (pScheme, false);

	pScheme->setValueBool(s_TBPrefsKeys[num], pFrameData->m_bShowBar[num]);

	//	FV_View * pView = static_cast<FV_View *>(pAV_View);
	//	pView->draw(NULL);
	return true;
}


Defun1(viewTB1)
{
	CHECK_FRAME;
	return _viewTBx(pAV_View, 0);
}

Defun1(viewTB2)
{
	CHECK_FRAME;
	return _viewTBx(pAV_View, 1);
}

Defun1(viewTB3)
{
	CHECK_FRAME;
	return _viewTBx(pAV_View, 2);
}

Defun1(viewTB4)
{
	CHECK_FRAME;
	return _viewTBx(pAV_View, 3);
}


#if !XAP_SIMPLE_TOOLBAR
Defun1(viewStd)
{
	CHECK_FRAME;
// TODO: Share this function with viewFormat & viewExtra
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
	UT_return_val_if_fail (pFrameData, false);

	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
		return false;

	// toggle the ruler bit
	pFrameData->m_bShowBar[0] = ! pFrameData->m_bShowBar[0];

	// actually do the dirty work
	pFrame->toggleBar( 0, pFrameData->m_bShowBar[0] );

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_return_val_if_fail (pScheme, false);

	pScheme->setValueBool(static_cast<const gchar *>(AP_PREF_KEY_StandardBarVisible), pFrameData->m_bShowBar[0]);
	return true;
}
#endif

#if !XAP_SIMPLE_TOOLBAR
Defun1(viewFormat)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
	UT_return_val_if_fail (pFrameData, false);

	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
	  return false;

	// toggle the ruler bit
	pFrameData->m_bShowBar[1] = ! pFrameData->m_bShowBar[1];

	// actually do the dirty work
	pFrame->toggleBar( 1, pFrameData->m_bShowBar[1] );

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_return_val_if_fail (pScheme, false);

	pScheme->setValueBool(static_cast<const gchar *>(AP_PREF_KEY_FormatBarVisible), pFrameData->m_bShowBar[1]);
	return true;
}
#endif


#if !XAP_SIMPLE_TOOLBAR
Defun1(viewTable)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
UT_return_val_if_fail(pFrameData, false);
	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
	  return false;

	// toggle the ruler bit
	pFrameData->m_bShowBar[2] = ! pFrameData->m_bShowBar[2];

	// actually do the dirty work
	pFrame->toggleBar( 2, pFrameData->m_bShowBar[2] );

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_return_val_if_fail (pScheme, false);

	pScheme->setValueBool(static_cast<const gchar *>(AP_PREF_KEY_TableBarVisible), pFrameData->m_bShowBar[2]);
	return true;
}
#endif


#if !XAP_SIMPLE_TOOLBAR
Defun1(viewExtra)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
UT_return_val_if_fail(pFrameData, false);
	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
	  return false;

	// toggle the ruler bit
	pFrameData->m_bShowBar[3] = ! pFrameData->m_bShowBar[3];

	// actually do the dirty work
	pFrame->toggleBar( 3, pFrameData->m_bShowBar[3] );

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
UT_return_val_if_fail(pScheme, false);

	pScheme->setValueBool(static_cast<const gchar *>(AP_PREF_KEY_ExtraBarVisible), pFrameData->m_bShowBar[3]);
	
	return true;
}
#endif

Defun1(lockToolbarLayout)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);

	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);

	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
UT_return_val_if_fail(pScheme, false);

	return true;
}

Defun1(defaultToolbarLayout)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
UT_return_val_if_fail(pFrameData, false);
	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
	  return false;

	// we don't want to change their visibility, just the layout
	pFrame->toggleBar(0, pFrameData->m_bShowBar[0]);
	pFrame->toggleBar(1, pFrameData->m_bShowBar[1]);
	pFrame->toggleBar(2, pFrameData->m_bShowBar[2]);
	pFrame->toggleBar(3, pFrameData->m_bShowBar[3]);

	return true;
}

Defun1(viewNormalLayout)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	FV_View * pView = static_cast<FV_View *>(pAV_View);
	if(pView->isHdrFtrEdit())
	{
		pView->clearHdrFtrEdit();
		pView->warpInsPtToXY(0,0,false);
	}

	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
UT_return_val_if_fail(pFrameData, false);
	pFrameData->m_pViewMode = VIEW_NORMAL;
	pFrame->toggleLeftRuler (false);
	if(!pFrameData->m_bIsFullScreen)
		pFrame->toggleTopRuler (true);

	pView->setViewMode (VIEW_NORMAL);

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
UT_return_val_if_fail(pScheme, false);
	pScheme->setValue(AP_PREF_KEY_LayoutMode, "2");

	pView->updateScreen(false);
	//pView->notifyListeners(AV_CHG_ALL);

	if (pFrame->getZoomType() == pFrame->z_PAGEWIDTH || pFrame->getZoomType() == pFrame->z_WHOLEPAGE)
		pFrame->updateZoom();

	return true;
}


Defun1(viewWebLayout)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
UT_return_val_if_fail(pFrameData, false);
	pFrameData->m_pViewMode = VIEW_WEB;
	pFrame->toggleLeftRuler (false);
	pFrame->toggleTopRuler (false);
	//
	// This about this. we need to work out a page width for 100% zoom
	//
	//pFrame->setZoomType(XAP_Frame::z_PAGEWIDTH);

	FV_View * pView = static_cast<FV_View *>(pAV_View);
	pView->setViewMode (VIEW_WEB);

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
UT_return_val_if_fail(pScheme, false);
	pScheme->setValue(AP_PREF_KEY_LayoutMode, "3");

	pView->updateScreen(false);
	//pView->notifyListeners(AV_CHG_ALL);

	if (pFrame->getZoomType() == pFrame->z_PAGEWIDTH || pFrame->getZoomType() == pFrame->z_WHOLEPAGE)
		pFrame->updateZoom();

	return true;
}

Defun1(viewPrintLayout)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
UT_return_val_if_fail(pFrameData, false);
	pFrameData->m_pViewMode = VIEW_PRINT;
	pFrame->toggleLeftRuler (true && (pFrameData->m_bShowRuler) &&
				 (!pFrameData->m_bIsFullScreen));
	if(!pFrameData->m_bIsFullScreen)
		pFrame->toggleTopRuler (true);

	FV_View * pView = static_cast<FV_View *>(pAV_View);
	UT_DEBUGMSG(("Set mode VIEW PRINT \n"));
	pView->setViewMode (VIEW_PRINT);

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
UT_return_val_if_fail(pScheme, false);
	pScheme->setValue(AP_PREF_KEY_LayoutMode, "1");

	//pView->notifyListeners(AV_CHG_ALL);

	if (pFrame->getZoomType() == pFrame->z_PAGEWIDTH || pFrame->getZoomType() == pFrame->z_WHOLEPAGE)
		pFrame->updateZoom();
	pView->updateScreen(false);

	return true;
}

Defun1(viewStatus)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
UT_return_val_if_fail(pFrameData, false);	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
	  return false;


	// toggle the view status bit
	pFrameData->m_bShowStatusBar = ! pFrameData->m_bShowStatusBar;

	// actually do the dirty work
	pFrame->toggleStatusBar(pFrameData->m_bShowStatusBar);

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
UT_return_val_if_fail(pScheme, false);
	pScheme->setValueBool(static_cast<const gchar *>(AP_PREF_KEY_StatusBarVisible), pFrameData->m_bShowStatusBar);
	return true;
}

Defun1(viewRuler)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
	UT_return_val_if_fail(pFrameData, false);
	// don't do anything if fullscreen
	if (pFrameData->m_bIsFullScreen)
	  return false;

	// toggle the ruler bit
	pFrameData->m_bShowRuler = ! pFrameData->m_bShowRuler;

	// actually do the dirty work
	pFrame->toggleRuler(pFrameData->m_bShowRuler);

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
UT_return_val_if_fail(pScheme, false);	pScheme->setValueBool(static_cast<const gchar *>(AP_PREF_KEY_RulerVisible), pFrameData->m_bShowRuler);

	return true;
}

Defun1(viewFullScreen)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
	UT_return_val_if_fail(pFrameData, false);


	if(!pFrameData->m_bIsFullScreen) // we're hiding stuff
	{
		pFrameData->m_bIsFullScreen = true;
		for (UT_uint32 i = 0; i < 20; i++) // should never have more than 20 toolbars
		{
			if (!pFrame->getToolbar(i))
				break;

			if (pFrameData->m_bShowBar[i])
				pFrame->toggleBar(i, false);
		}
		if (pFrameData->m_bShowStatusBar)
			pFrame->toggleStatusBar(false);
		if (pFrameData->m_bShowRuler)
			pFrame->toggleRuler(false);
		pFrame->setFullScreen(true);
	}
	else // we're (possibly) unhiding stuff
	{
		if (pFrameData->m_bShowRuler)
			pFrame->toggleRuler(pFrameData->m_bShowRuler);
		if (pFrameData->m_bShowStatusBar)
			pFrame->toggleStatusBar(pFrameData->m_bShowStatusBar);
		for (UT_uint32 i = 0; i < 4; i++)
		{
			if (!pFrame->getToolbar(i))
				break;

			if (pFrameData->m_bShowBar[i])
				pFrame->toggleBar(i, true);
		}
		pFrameData->m_bIsFullScreen = false;
		pFrame->setFullScreen(false);
	}

	// Recalculate the layout after entering/leaving fullscreen
	pFrame->queue_resize();
	return true;
}

Defun1(viewPara)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
UT_return_val_if_fail(pFrameData, false);
	pFrameData->m_bShowPara = !pFrameData->m_bShowPara;

	ABIWORD_VIEW;
	pView->setShowPara(pFrameData->m_bShowPara);

	// POLICY: make this the default for new frames, too
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
UT_return_val_if_fail(pScheme, false);
	pScheme->setValueBool(AP_PREF_KEY_ParaVisible, pFrameData->m_bShowPara);
	pView->notifyListeners(AV_CHG_ALL);

	return true;
}

Defun1(viewHeadFoot)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	// TODO: synch this implementation with ap_GetState_View
	s_TellNotImplemented(pFrame, "View Headers and Footers", __LINE__);
	return true;
}

Defun1(viewLockStyles)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	pView->getDocument()->lockStyles( !pView->getDocument()->areStylesLocked() );
	pView->notifyListeners(AV_CHG_ALL);
 	return true;
}

Defun(zoom)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme *pPrefsScheme = pPrefs->getCurrentScheme();
UT_return_val_if_fail(pPrefsScheme, false);
	UT_uint32 iZoom = 0;
	
	UT_UTF8String utf8(pCallData->m_pData, pCallData->m_dataLength);
	const gchar *p_zoom = reinterpret_cast<const gchar *>(utf8.utf8_str());

	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	std::string sPageWidth;
	pSS->getValueUTF8(XAP_STRING_ID_TB_Zoom_PageWidth,sPageWidth);
	
	std::string sWholePage;
	pSS->getValueUTF8(XAP_STRING_ID_TB_Zoom_WholePage,sWholePage);
	
	std::string sPercent;
	pSS->getValueUTF8(XAP_STRING_ID_TB_Zoom_Percent,sPercent);
	
	if(strcmp(p_zoom, sPageWidth.c_str()) == 0)
	{
		pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
						 static_cast<const gchar*>("Width"));
		pFrame->setZoomType(XAP_Frame::z_PAGEWIDTH);
		iZoom = pView->calculateZoomPercentForPageWidth();
	}
	else if(strcmp(p_zoom, sWholePage.c_str()) == 0)
	{
		pFrame->setZoomType(XAP_Frame::z_WHOLEPAGE);
		pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
						 static_cast<const gchar*>("Page"));
		iZoom = pView->calculateZoomPercentForWholePage();
	}
	else if(strcmp(p_zoom, sPercent.c_str()) == 0)
	{
		// invoke the zoom dialog instead for some custom value
		return EX(dlgZoom);
	}
	else
	{
		// we've gotten back a number - turn it into a zoom percentage
		//UT_UTF8String tmp (UT_UTF8String_sprintf("%d",p_zoom))
		pPrefsScheme->setValue(static_cast<const gchar*>(XAP_PREF_KEY_ZoomType),
						 static_cast<const gchar*>(utf8.utf8_str()));		
		pFrame->setZoomType(XAP_Frame::z_PERCENT);
		iZoom = atoi(p_zoom);
	}
	  
	UT_return_val_if_fail (iZoom > 0, false);
	pFrame->quickZoom(iZoom);

//
// Make damn sure the cursor is ON!!
//
	FV_View * pAbiView = static_cast<FV_View *>(pFrame->getCurrentView());
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
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_Insert_DateTime * pDialog
		= static_cast<AP_Dialog_Insert_DateTime *>(pDialogFactory->requestDialog(AP_DIALOG_ID_INSERT_DATETIME));
UT_return_val_if_fail(pDialog, false);
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
	UT_return_val_if_fail(pView, false);
	if(pView->isInTable(pView->getPoint()-1) && pView->isInTable())
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
	UT_return_val_if_fail(pView,false);

	const gchar * right_attributes [] = {
	  "text-align", "right", NULL, NULL
	};

	const gchar * left_attributes [] = {
	  "text-align", "left", NULL, NULL
	};

	const gchar * center_attributes [] = {
	  "text-align", "center", NULL, NULL
	};

	const gchar ** atts = NULL;

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_PageNumbers * pDialog
		= static_cast<AP_Dialog_PageNumbers *>(pDialogFactory->requestDialog(AP_DIALOG_ID_PAGE_NUMBERS));
UT_return_val_if_fail(pDialog, false);
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
		default: UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN); break;
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
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_Field * pDialog
		= static_cast<AP_Dialog_Field *>(pDialogFactory->requestDialog(AP_DIALOG_ID_FIELD));
UT_return_val_if_fail(pDialog, false);
	pDialog->runModal(pFrame);

	if (pDialog->getAnswer() == AP_Dialog_Field::a_OK)
	{
		const gchar * pParam = pDialog->getParameter();
		const gchar * pAttr[3];
		const gchar param_name[] = "param";
		pAttr[0] = static_cast<const gchar *>(&param_name[0]);
		pAttr[1] = pParam;
		pAttr[2] = 0;

		if(pParam)
			pView->cmdInsertField(pDialog->GetFieldFormat(),static_cast<const gchar **>(&pAttr[0]));
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

Defun1(insMailMerge)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_MailMerge * pDialog
		= static_cast<AP_Dialog_MailMerge *>(pDialogFactory->requestDialog(AP_DIALOG_ID_MAILMERGE));
UT_return_val_if_fail(pDialog, false);
	if(pDialog->isRunning())
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

Defun1(insFile)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	
	IEFileType fType = IEFT_Unknown;
	char *pathName = NULL;
	
	// we'll share the same graphics context, which won't matter because
	// we only use it to get font metrics and stuff and not actually draw
	GR_Graphics *pGraphics = pView->getGraphics();
	
	if (s_AskForPathname (pFrame, false, XAP_DIALOG_ID_INSERT_FILE,
			      NULL, &pathName, &fType))
	{
	    UT_DEBUGMSG(("DOM: insertFile %s\n", pathName));
	    
	    PD_Document * newDoc = new PD_Document();
	    UT_Error err = newDoc->readFromFile(pathName, IEFT_Unknown);
	    
		if (!UT_IS_IE_SUCCESS(err))
		{
			UNREFP(newDoc);
			s_CouldNotLoadFileMessage(pFrame, pathName, err);
			return false;
		}
        if ( err == UT_IE_TRY_RECOVER ) 
        {
            s_CouldNotLoadFileMessage(pFrame, pathName, err);
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

Defun1(insTextBox)
{
	CHECK_FRAME;

	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	static_cast<FV_View *>(pView)->getFrameEdit()->setMode(FV_FrameEdit_WAIT_FOR_FIRST_CLICK_INSERT);
	static_cast<FV_View *>(pView)->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_CROSSHAIR);
	return true;
}

Defun1(insFootnote)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	return pView->insertFootnote(true);
}


static 
void insertAnnotation(FV_View * pView, bool bDescr)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_return_if_fail(pFrame);
	
	pFrame->raise();
	
	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());
	
	AP_Dialog_Annotation * pDialog
		= static_cast<AP_Dialog_Annotation *>(pDialogFactory->requestDialog(AP_DIALOG_ID_ANNOTATION));
	UT_return_if_fail(pDialog);
	
	pDialog->setAuthor(pView->getDocument()->getUserName());
	
	if (bDescr)
	{
		UT_UCS4Char* text = NULL;
		pView->getSelectionText(text);
		UT_UCS4String sUCS4(static_cast<const UT_UCS4Char *>(text));
		pDialog->setDescription(sUCS4.utf8_str());
	}
	
	// run the dialog	
	
	UT_DEBUGMSG(("insertAnnotation: Drawing annotation dialog...\n"));
	pDialog->runModal(pFrame);
	
	bool bOK = (pDialog->getAnswer() == AP_Dialog_Annotation::a_OK);  
	bool bApply = (pDialog->getAnswer() == AP_Dialog_Annotation::a_APPLY);	
	
	if (bOK || bApply)
	{
		const std::string &sTitle = pDialog->getTitle();
		const std::string &sAuthor = pDialog->getAuthor();
		const std::string &sText = pDialog->getDescription();
		
		UT_sint32 iAnnotation = pView->getDocument()->getUID(UT_UniqueId::Annotation);
		
		fl_AnnotationLayout * pAL = NULL;

		pView->insertAnnotation(iAnnotation,  
								sText,  
								sAuthor,  
								sTitle,  
								bApply);  

		if (bApply)  
		{  
			pView->setAnnotationText(iAnnotation, pDialog->getDescription());  
			pAL = pView->insertAnnotationDescription(iAnnotation, pDialog);
			UT_return_if_fail(pAL);        
		}
		
		pAL = pView->getAnnotationLayout(iAnnotation);
		if (pAL) 
			pView->selectAnnotation(pAL);
	}      
	
	// release the dialog
	pDialogFactory->releaseDialog(pDialog);
	
	// TODO: set the document as dirty when something changed
}

Defun1(insAnnotation)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	
	UT_DEBUGMSG(("insAnnotation: inserting\n"));
	insertAnnotation(pView, false);
	return true;
}


Defun1(insAnnotationFromSel)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	
	UT_DEBUGMSG(("insAnnotationFromSel: inserting\n"));
	insertAnnotation(pView, true);
	return true;
}

Defun1(toggleDisplayAnnotations)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	
	//
	// Set the preference to enable annotations display
	//
	XAP_Prefs * pPrefs = XAP_App::getApp()->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_return_val_if_fail(pScheme, false);
	bool b = false;
	pScheme->getValueBool(static_cast<const gchar *>(AP_PREF_KEY_DisplayAnnotations), &b );
	b = !b;
	UT_DEBUGMSG(("toggleDisplayAnnotations: Changing annotation display to %s\n",(b ? "true" : "false")));
	gchar szBuffer[2] = {0,0};
	szBuffer[0] = ((b)==true ? '1' : '0');
	pScheme->setValue(static_cast<const gchar *>(AP_PREF_KEY_DisplayAnnotations),szBuffer);
	return true ;
}

Defun1(editAnnotation)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	UT_DEBUGMSG(("editAnnotation\n"));

	fp_AnnotationRun * pA = static_cast<fp_AnnotationRun *>(pView->getHyperLinkRun(pView->getPoint()));
	UT_ASSERT(pA);
	
	pView->cmdEditAnnotationWithDialog(pA->getPID());
	return true;
}

Defun1(insTOC)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	pView->cmdInsertTOC();
	return true;
}


Defun1(insEndnote)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	return pView->insertFootnote(false);
	return true;
}

Defun1(toggleRDFAnchorHighlight)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	
	//
	// Set the preference to enable annotations display
	//
	XAP_Prefs * pPrefs = XAP_App::getApp()->getPrefs();
	UT_return_val_if_fail(pPrefs, false);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_return_val_if_fail(pScheme, false);
	bool b = false;
	pScheme->getValueBool(static_cast<const gchar *>(AP_PREF_KEY_DisplayRDFAnchors), &b );
	b = !b;
	UT_DEBUGMSG(("toggleRDFAnchorHighlight: Changing annotation display to %s\n",(b ? "true" : "false")));
	gchar szBuffer[2] = {0,0};
	szBuffer[0] = ((b)==true ? '1' : '0');
	pScheme->setValue(static_cast<const gchar *>(AP_PREF_KEY_DisplayRDFAnchors),szBuffer);
	return true ;
}


static bool s_doRDFQueryDlg( FV_View * pView, XAP_Dialog_Id id, AP_Dialog_RDFQuery*& dialogret )
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	// kill the annotation preview popup if needed
	if(pView->isAnnotationPreviewActive())
		pView->killAnnotationPreview();
	
	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_RDFQuery * pDialog
		= static_cast<AP_Dialog_RDFQuery *>(pDialogFactory->requestDialog(id));
	UT_return_val_if_fail(pDialog, false);
	dialogret = pDialog;

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

Defun1(rdfQuery)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	XAP_Dialog_Id id = AP_DIALOG_ID_RDF_QUERY;
	AP_Dialog_RDFQuery* dialog = 0;
	return s_doRDFQueryDlg( pView, id, dialog );
}


static bool s_doRDFEditorDlg( FV_View * pView, XAP_Dialog_Id id, AP_Dialog_RDFEditor*& dialogret, bool rstrct )
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	// kill the annotation preview popup if needed
	if(pView->isAnnotationPreviewActive())
		pView->killAnnotationPreview();
	
	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_RDFEditor * pDialog
		= static_cast<AP_Dialog_RDFEditor *>(pDialogFactory->requestDialog(id));
	UT_return_val_if_fail(pDialog, false);
	dialogret = pDialog;

	pDialog->hideRestrictionXMLID( !rstrct );
	
	
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


Defun1(rdfEditor)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	XAP_Dialog_Id id = AP_DIALOG_ID_RDF_EDITOR;
	AP_Dialog_RDFEditor* dialog = 0;
	return s_doRDFEditorDlg( pView, id, dialog, false );
}

Defun1(rdfInsertNewContact)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	if( PD_Document * pDoc = pView->getDocument() )
	{
		if( PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF() )
		{
			std::string objname;
			const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
			pSS->getValueUTF8(AP_STRING_ID_DLG_RDF_Insert_NewContact, objname);
			PD_RDFSemanticItemHandle obj = PD_RDFSemanticItem::createSemanticItem( rdf, "Contact" );
			obj->setName( objname );
			/*std::pair< PT_DocPosition, PT_DocPosition > range =*/
			obj->insert( pView );
			obj->showEditorWindow( obj );
		}
	}
	return 0;
}

Defun1(rdfInsertNewContactFromFile)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	if( PD_Document * pDoc = pView->getDocument() )
	{
		if( PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF() )
		{
			std::string objname;
			const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();
			pSS->getValueUTF8(AP_STRING_ID_DLG_RDF_Insert_NewContact, objname);
			PD_RDFSemanticItemHandle obj = PD_RDFSemanticItem::createSemanticItem( rdf, "Contact" );
			obj->setName( objname );
			obj->importFromFile();
		}
	}
	return 0;
}

Defun1(rdfInsertRef)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	if( PD_Document * pDoc = pView->getDocument() )
	{
		if( PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF() )
		{
			runInsertReferenceDialog( pView );
		}
	}
	return 0;
}



Defun1(rdfQueryXMLIDs)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	AP_Dialog_RDFQuery* dialog = 0;
	XAP_Dialog_Id id = AP_DIALOG_ID_RDF_QUERY;

	bool rc = s_doRDFQueryDlg( pView, id, dialog );
	if( dialog )
	{
		std::string sparql;
		PT_DocPosition point = pView->getPoint();
		UT_DEBUGMSG(("point is at:%d\n", point ));

		if( PD_Document * pDoc = pView->getDocument() )
		{
			if( PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF() )
			{
				std::set< std::string > xmlids;
				rdf->addRelevantIDsForPosition( xmlids, point );
				UT_DEBUGMSG(("xmlids.sz:%lu\n", (long unsigned)xmlids.size() ));

				sparql = PD_DocumentRDF::getSPARQL_LimitedToXMLIDList( xmlids );
			}
		}
		
		dialog->executeQuery( sparql );
	}
	return rc;
}

/*****************************************************************/
/*****************************************************************/

Defun1(dlgParagraph)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView, false);
	if (pView->getDocument()->areStylesLocked())
		return true;

	return s_doParagraphDlg(pView);
}

static bool s_doBullets(FV_View *pView)
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());
	AP_Dialog_Lists * pDialog
		= static_cast<AP_Dialog_Lists *>(pDialogFactory->requestDialog(AP_DIALOG_ID_LISTS));
UT_return_val_if_fail(pDialog, false);
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
#if defined(TARGET_OS_MAC)
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	s_TellNotImplemented(pFrame, "Lists dialog", __LINE__);
	return true;
#else // enable for GTK+ & Gnome builds only
	ABIWORD_VIEW;
	return s_doBullets(pView);
#endif
}

/***********************************************************************************/

static bool s_doBorderShadingDlg(FV_View * pView)
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_Border_Shading * pDialog
		= static_cast<AP_Dialog_Border_Shading *>(pDialogFactory->requestDialog(AP_DIALOG_ID_BORDER_SHADING));
	UT_return_val_if_fail(pDialog, false);
	if(!pView->isInTable(pView->getPoint()))
	{
		pView->setPoint(pView->getSelectionAnchor());
	}
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

Defun1(dlgBorders)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	s_doBorderShadingDlg(pView);

	return true;
}

Defun1(setPosImage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView, false);
	PT_DocPosition pos = pView->getDocPositionFromLastXY();

	fl_BlockLayout * pBlock = pView->getBlockAtPosition(pos);
	fp_Run *  pRun = NULL;
	fp_Line * pLine = NULL;
	UT_sint32 x1,x2,y1,y2,iHeight;
	bool bEOL = false;
	bool bDir = false;
	if(pBlock)
	{
		pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,iHeight,bDir);
		while(pRun && pRun->getType() != FPRUN_IMAGE)
		{
			pRun = pRun->getNextRun();
		}
		if(pRun && pRun->getType() == FPRUN_IMAGE)
		{
			UT_DEBUGMSG(("SEVIOR: Image run on pos \n"));
		}
		else
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
	}
	pLine = pRun->getLine();
	if(pLine == NULL)
	{
	        return false;
	}
	pView->cmdSelect(pos,pos+1);
	fp_ImageRun * pImageRun = static_cast<fp_ImageRun *>(pRun);
	std::string sWidth;
	std::string sHeight;
	double d = static_cast<double>(pRun->getWidth())/static_cast<double>(UT_LAYOUT_RESOLUTION);
	sWidth =  UT_formatDimensionedValue(d,"in", NULL);
	d = static_cast<double>(pRun->getHeight())/static_cast<double>(UT_LAYOUT_RESOLUTION);
	sHeight =  UT_formatDimensionedValue(d,"in", NULL);
//
// Get the dataID of the image.

	const char * dataID = pImageRun->getDataId();
	const PP_AttrProp * pImageAP = pImageRun->getSpanAP();
	std::string sFrameProps;
	std::string sProp;
	std::string sVal;
	sProp = "frame-type";
	sVal = "image";
	UT_std_string_setProperty(sFrameProps, sProp, sVal);
//
// Turn off the borders.
//
	sProp = "top-style";
	sVal = "none";
	UT_std_string_setProperty(sFrameProps, sProp, sVal);
	sProp = "right-style";
	UT_std_string_setProperty(sFrameProps, sProp, sVal);
	sProp = "left-style";
	UT_std_string_setProperty(sFrameProps, sProp, sVal);
	sProp = "bot-style";
	UT_std_string_setProperty(sFrameProps, sProp, sVal);
//
// Set width/Height
//
	sProp = "frame-width";
	sVal = sWidth;
	UT_std_string_setProperty(sFrameProps, sProp, sVal);
	sProp = "frame-height";
	sVal = sHeight;
	UT_std_string_setProperty(sFrameProps, sProp, sVal);
	double xpos = 0.0;
	double ypos= 0.0;

	sProp = "position-to";
	sVal = "page-above-text";
	UT_std_string_setProperty(sFrameProps, sProp, sVal);
	if(pView->isHdrFtrEdit() || pView->isInHdrFtr(pos))
	{
		pView->clearHdrFtrEdit();
		pView->warpInsPtToXY(0,0,false);
		pos = pView->getPoint();
	}

//
// Now calculate the Y offset to the Column
//
	UT_sint32 yLine = pLine->getY() + pLine->getColumn()->getY();
	ypos = static_cast<double>(yLine)/static_cast<double>(UT_LAYOUT_RESOLUTION);
	sProp = "frame-page-ypos";
	sVal = UT_formatDimensionedValue(ypos,"in", NULL);
	UT_std_string_setProperty(sFrameProps, sProp, sVal);
	UT_sint32 ix = pRun->getX() + pLine->getColumn()->getX() + pLine->getX();
	xpos =  static_cast<double>(ix)/static_cast<double>(UT_LAYOUT_RESOLUTION);
	sProp = "frame-page-xpos";
	sVal = UT_formatDimensionedValue(xpos,"in", NULL);
	UT_std_string_setProperty(sFrameProps, sProp, sVal);
	sVal = UT_std_string_sprintf("%d", pLine->getPage()->getPageNumber());
	sProp = "frame-pref-page";
	UT_std_string_setProperty(sFrameProps, sProp, sVal);
//
// Wrapped Mode
//
	sProp = "wrap-mode";
	sVal = "wrapped-both";
	UT_std_string_setProperty(sFrameProps, sProp, sVal);
	//
	// Now the alt and title
	//
	const char * szTitle = NULL;
	const char * szDescription = NULL;
	bool bFound = pImageAP->getAttribute("title",szTitle);
	if(!bFound)
	{
			szTitle = "";
	}
	bFound = pImageAP->getAttribute("alt",szDescription);
	if(!bFound)
	{
			szDescription = "";
	}
//
// Now define the Frame attributes strux
//
	const gchar * attributes[] = {PT_STRUX_IMAGE_DATAID,NULL,
								  PT_PROPS_ATTRIBUTE_NAME, NULL,
								  PT_IMAGE_TITLE,NULL,
								  PT_IMAGE_DESCRIPTION,NULL,
								  NULL,NULL};
	attributes[1] = dataID;
	attributes[3] = sFrameProps.c_str();
	attributes[5] = szTitle;
	attributes[7] = szDescription;
//
// This deletes the inline image and places a positioned image in it's place
// It deals with the undo/general update issues.
//
	pView->convertInLineToPositioned(pos,attributes);
//
// Done! Now have a positioned image!
//
	return true;
}

Defun1(dlgFmtPosImage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_Image * pDialog
		= static_cast<XAP_Dialog_Image *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_IMAGE));
	UT_return_val_if_fail(pDialog, false);
	fl_FrameLayout * pPosObj = pView->getFrameLayout();
	if(pPosObj == NULL)
	{
		// try to select frame
		pView->activateFrame();
		pPosObj = pView->getFrameLayout();
		if (pPosObj == NULL)
		{
			return true;
		}
	}
	if(pPosObj-> getFrameType() < FL_FRAME_WRAPPER_IMAGE)
	{
	  return true;
	}

	const PP_AttrProp* pAP = NULL;
	pPosObj->getAP(pAP);
	const gchar* szTitle = 0;
	const gchar* szDescription = 0;
	pDialog->setInHdrFtr(false);
	const char * pszRulerUnits = NULL;
	UT_Dimension dim = DIM_IN;
	if (XAP_App::getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits, &pszRulerUnits))
	{
		dim = UT_determineDimension(pszRulerUnits);
	}
	pDialog->setPreferedUnits(dim);

	fl_BlockLayout * pBL = pView->getCurrentBlock();
	// an approximate... TODO: make me more accurate
	fl_DocSectionLayout * pDSL = pBL->getDocSectionLayout();
	UT_sint32 iColWidth = pDSL->getActualColumnWidth();
	UT_sint32 iColHeight = pDSL->getActualColumnHeight();
	double max_width  = iColWidth*72.0/UT_LAYOUT_RESOLUTION; // units are 1/72 of an inch
	double max_height = iColHeight*72.0/UT_LAYOUT_RESOLUTION;

	pDialog->setMaxWidth (max_width);
	pDialog->setMaxHeight (max_height);

	if (pAP) 
	{
	  pAP->getAttribute ("title", szTitle);
	  pAP->getAttribute ("alt", szDescription);
	}

	if (szTitle) 
	{
	  pDialog->setTitle (szTitle);
	}
	if (szDescription) 
	{
	  pDialog->setDescription (szDescription);
	}
	const gchar * pszWidth = NULL;
	const gchar * pszHeight = NULL;
	if(!pAP || !pAP->getProperty("frame-width",pszWidth))
	{
	  pszWidth = "1.0in";
	}
	if(!pAP || !pAP->getProperty("frame-height",pszHeight))
	{
	  pszHeight = "1.0in";
	}
	pDialog->setWidth( UT_reformatDimensionString(dim,pszWidth));
	pDialog->setHeight( UT_reformatDimensionString(dim,pszHeight));

	UT_DEBUGMSG(("Width %s Height %s \n",pszWidth,pszHeight));
	WRAPPING_TYPE iWrap = WRAP_NONE;
	if(pPosObj->getFrameWrapMode() == FL_FRAME_WRAPPED_TO_LEFT  )
	{
	  iWrap = WRAP_TEXTLEFT;
	}
	if(pPosObj->getFrameWrapMode() == FL_FRAME_WRAPPED_TO_RIGHT  )
	{
	  iWrap = WRAP_TEXTRIGHT;
	}
	else if(pPosObj->getFrameWrapMode() == FL_FRAME_WRAPPED_BOTH_SIDES)
	{
	  iWrap = WRAP_TEXTBOTH;
	} 
	else if(pPosObj->getFrameWrapMode() == FL_FRAME_ABOVE_TEXT)
	{
	  iWrap = WRAP_NONE;
	}
	else if(pPosObj->getFrameWrapMode() == FL_FRAME_BELOW_TEXT)
	{
	  iWrap = WRAP_NONE;
	}
	POSITION_TO iPos = POSITION_TO_PARAGRAPH;
	if(pPosObj->getFramePositionTo() == FL_FRAME_POSITIONED_TO_COLUMN)
	{
	  iPos = POSITION_TO_COLUMN;
	}
	else if(pPosObj->getFramePositionTo() == FL_FRAME_POSITIONED_TO_PAGE)
	{
	  iPos = POSITION_TO_PAGE;
	}
	pDialog->setWrapping( iWrap);
	pDialog->setPositionTo( iPos);
	if(pPosObj->isTightWrap())
	{
	  pDialog->setTightWrap(true);
	}
	else
	{
	  pDialog->setTightWrap(false);
	}
	pDialog->runModal(pFrame);
	XAP_Dialog_Image::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_Image::a_OK);
	if(!bOK)
	{
	  return true;
	}


	UT_String sWidth;
	UT_String sHeight;

	sWidth = pDialog->getWidthString();
	sHeight = pDialog->getHeightString();
	UT_DEBUGMSG(("Width %s Height %s \n",sWidth.c_str(),sHeight.c_str()));
	const gchar * attribs[] = {"title", NULL, "alt", NULL, 0};
	attribs[1] = pDialog->getTitle().utf8_str();
	attribs[3] = pDialog->getDescription().utf8_str();


	if(pDialog->getWrapping() == WRAP_INLINE)
	{
		const gchar * properties[] = {"width", NULL, "height", NULL, 0};
		properties[1] = sWidth.c_str();
		properties[3] = sHeight.c_str();
		
		pView->convertPositionedToInLine(pPosObj);
		pView->setCharFormat(properties, attribs);
		pView->updateScreen();
		return true;
	}
	else
	{
	  POSITION_TO newFormatMode = pDialog->getPositionTo(); 
	  WRAPPING_TYPE newWrapMode = pDialog->getWrapping();
	  const gchar * properties[] = {"frame-width", NULL, 
									"frame-height", NULL, 
									"wrap-mode",NULL,
									"position-to",NULL,
									"tight-wrap",NULL,
									NULL,NULL,NULL,NULL,
									NULL,NULL,NULL};

	  properties[1] = sWidth.c_str();
	  properties[3] = sHeight.c_str();
	  if(newWrapMode == WRAP_TEXTRIGHT)
	  {
	    properties[5] = "wrapped-to-right";
	  }
	  else if(newWrapMode == WRAP_TEXTLEFT)
	  {
	    properties[5] = "wrapped-to-left";
	  }
	  else if(newWrapMode == WRAP_TEXTBOTH)
	  {
	    properties[5] = "wrapped-both";
	  }
	  else if(newWrapMode == WRAP_NONE)
	  {
	    properties[5] = "above-text";
	  }

	  if(newFormatMode == POSITION_TO_PARAGRAPH)
	  {
	    properties[7] = "block-above-text";
	  }
	  else if(newFormatMode == POSITION_TO_COLUMN)
	  {
	    properties[7] = "column-above-text";
	  }
	  else if(newFormatMode == POSITION_TO_PAGE)
	  {
	    properties[7] = "page-above-text";
	  }
	  if(pDialog->isTightWrap())
	  {
        properties[9] = "1";
	  }
	  else
	  {
	    properties[9] = "0";
	  }

	  fp_FrameContainer * pFrameC = static_cast<fp_FrameContainer *>(pPosObj->getFirstContainer());
	  fv_FrameStrings FrameStrings;
	  fl_BlockLayout * pCloseBL = NULL;
	  fp_Page * pPage = NULL;

	  if (pFrameC && (newFormatMode != iPos))
	  {
		  UT_sint32 iXposPage = pFrameC->getX() - pFrameC->getXPad();
		  UT_sint32 iYposPage = pFrameC->getY() - pFrameC->getYPad();
		  UT_sint32 xp = 0;
		  UT_sint32 yp = 0;
		  pPage = pFrameC->getColumn()->getPage();
		  pView->getPageScreenOffsets(pPage,xp,yp);
		  pView->getFrameStrings_view(iXposPage+xp,iYposPage+yp,
									  FrameStrings,&pCloseBL,&pPage);

		  UT_DEBUGMSG(("Position of frame: X %d\t Y %d\n",iXposPage,iYposPage));
		  if (newFormatMode == POSITION_TO_PARAGRAPH)
		  {
			  properties[10] = "xpos";
			  properties[11] = FrameStrings.sXpos.c_str();
			  properties[12] = "ypos";
			  properties[13] = FrameStrings.sYpos.c_str();
		  }
		  else if (newFormatMode == POSITION_TO_COLUMN)
		  {
			  properties[10] = "frame-col-xpos";
			  properties[11] = FrameStrings.sColXpos.c_str();
			  properties[12] = "frame-col-ypos";
			  properties[13] = FrameStrings.sColYpos.c_str();
			  properties[14] = "frame-pref-column";
			  properties[15] = FrameStrings.sPrefColumn.c_str();
		  }
		  else if (newFormatMode == POSITION_TO_PAGE)
		  {
			  properties[10] = "frame-page-xpos";
			  properties[11] = FrameStrings.sPageXpos.c_str();
			  properties[12] = "frame-page-ypos";
			  properties[13] = FrameStrings.sPageYpos.c_str();
		  }
	  }

	  //
	  // Change the frame!
	  //
	  pView->setFrameFormat(attribs,properties,pCloseBL);
	}
	return true;
}


static bool s_doFormatImageDlg(FV_View * pView, EV_EditMethodCallData * pCallData, bool bCtxtMenu)
{
	UT_DEBUG_ONLY_ARG(pCallData);
	
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_Image * pDialog
		= static_cast<XAP_Dialog_Image *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_IMAGE));
	UT_return_val_if_fail(pDialog, false);
	double max_width = 0., max_height = 0.;
	UT_sint32 iHeight,iWidth;

	// set units in the dialog.
	const char * pszRulerUnits = NULL;
	UT_Dimension dim = DIM_IN;
	if (XAP_App::getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits, &pszRulerUnits))
	{
		dim = UT_determineDimension(pszRulerUnits);
	}
	pDialog->setPreferedUnits(dim);

	fl_BlockLayout * pBL = pView->getCurrentBlock();
	bool bInHdrFtr = pView->isInHdrFtr(pView->getPoint());

	// an approximate... TODO: make me more accurate
	fl_DocSectionLayout * pDSL = pBL->getDocSectionLayout();
	UT_sint32 iColWidth = pDSL->getActualColumnWidth();
	UT_sint32 iColHeight = pDSL->getActualColumnHeight();
	max_width  = iColWidth*72.0/UT_LAYOUT_RESOLUTION;
	max_height = iColHeight*72.0/UT_LAYOUT_RESOLUTION;

	pDialog->setMaxWidth (max_width);
	pDialog->setMaxHeight (max_height); // units are 1/72 of an inch
	UT_DEBUGMSG(("formatting  image: %d\n", pCallData->m_xPos));
	const fp_Run * pRun = NULL;
	const char * dataID = NULL;
	fl_BlockLayout * pBlock = NULL;
	PT_DocPosition pos = 0;

	if (bCtxtMenu)
	{
		pos = pView->getDocPositionFromLastXY();
		pBlock = pView->getBlockAtPosition(pos);
		if(pBlock)
        {
			pRun = pBlock->findRunAtOffset(pos - pBlock->getPosition());
			if(pRun && (pRun->getType() == FPRUN_IMAGE))
			{
				dataID = static_cast<const fp_ImageRun *>(pRun)->getDataId();
			}
			else
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				return false;
			}
		}
		else
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return false;
        }
	}
	else
	{
		pos = pView->getSelectedImage(&dataID,&pRun);
		if (!pRun)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
		pBlock = pRun->getBlock();
	}

    pView->cmdSelect(pos,pos+1);
	const gchar ** props_in = NULL;

	const PP_AttrProp * pAP = 0;
	pView->getAttributes (&pAP);
	pDialog->setInHdrFtr(bInHdrFtr);
	  
	if (pView->getCharFormat(&props_in))
	{
	  // stuff properties into the dialog.

	  const gchar* szWidth = UT_getAttribute("width", props_in);
	  const gchar* szHeight = UT_getAttribute("height", props_in);

	  const gchar* szTitle = 0;
	  const gchar* szDescription = 0;
	  pDialog->setInHdrFtr(bInHdrFtr);
	  if (pAP) {
		  pAP->getAttribute ("title", szTitle);
		  pAP->getAttribute ("alt", szDescription);
	  }

	  if (szTitle) {
		  pDialog->setTitle (szTitle);
	  }
	  if (szDescription) {
		  pDialog->setDescription (szDescription);
	  }

	  double width = 0., height = 0.;
	  if(szWidth)
		  width = UT_convertToInches(szWidth);
	  if (width < 0.0001)
	  {
		  iWidth = 0;
		  UT_return_val_if_fail (pRun, false);
		  if(pRun->getType() == FPRUN_IMAGE)
		  {
			  iWidth = pRun->getWidth();
		  }
		  else
		  {
			  UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			  FREEP(props_in);
			  return false;
		  }
		  width = iWidth*72.0/UT_LAYOUT_RESOLUTION;
		  }
	  if(szHeight)
	      height = UT_convertToInches(szHeight);
	  if (height < 0.0001)
	  {
		  iHeight = 0;
		  UT_return_val_if_fail (pRun, false);
		  if(pRun->getType() == FPRUN_IMAGE)
		  {
			  iHeight = pRun->getHeight();
		  }
		  else
		  {
			  UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			  FREEP(props_in);
			  return false;
		  }
		  height = iHeight*72.0/UT_LAYOUT_RESOLUTION;
	  }
	  if(width > max_width)
	  {
	    height *= max_width / width;
	    width = max_width;
	  }
	  if(height > max_height)
	  {
	    width *= max_height / height;
	    height = max_height;
	  }
	  pDialog->setWidth( UT_convertInchesToDimensionString(dim,width));
	  pDialog->setHeight( UT_convertInchesToDimensionString(dim,height));
	  FREEP(props_in);

	  WRAPPING_TYPE oldWrap = WRAP_INLINE;
	  pDialog->runModal(pFrame);

	  XAP_Dialog_Image::tAnswer ans = pDialog->getAnswer();
	  bool bOK = (ans == XAP_Dialog_Image::a_OK);
	  std::string sWidth;
	  std::string sHeight;
	  if (bOK)
	  {
		  WRAPPING_TYPE newWrap = pDialog->getWrapping();
		  // now get them back in inches
		  sWidth = pDialog->getWidthString();
		  sHeight = pDialog->getHeightString();
		  UT_DEBUGMSG(("Width %s Height %s \n",sWidth.c_str(),sHeight.c_str()));
		  const gchar * properties[] = {"width", NULL, "height", NULL, 0};
		  // TODO: set format

		  if((newWrap == WRAP_INLINE) && (oldWrap == WRAP_INLINE))
		  {
			  UT_DEBUGMSG(("DOM: nw:%s nh:%s\n", sWidth.c_str(), sHeight.c_str()));
			  
			  properties[1] = sWidth.c_str();
			  properties[3] = sHeight.c_str();

			  const gchar * attribs[] = {"title", NULL, "alt", NULL, 0};
			  attribs[1] = pDialog->getTitle().utf8_str();
			  attribs[3] = pDialog->getDescription().utf8_str();

			  pView->setCharFormat(properties, attribs);
			  pView->updateScreen();
		  }

//
// This code turns inline-images into frames this way. Later
// we changes frames to inline and frame types to frame types
//
		  else if( !bInHdrFtr && (oldWrap == WRAP_INLINE) && (newWrap != WRAP_INLINE))
		  {

// OK we gotta create a frame with the dimensions of the image and roughly the
// the location of the image.
//
// Get the line of the image. (We have the run and Block)
//
			  fp_Line * pLine = pRun->getLine();

			  std::string sFrameProps;
			  std::string sProp;
			  std::string sVal;
			  sProp = "frame-type";
			  sVal = "image";
			  UT_std_string_setProperty(sFrameProps, sProp, sVal);
//
// Turn off the borders.
//
			  sProp = "top-style";
			  sVal = "none";
			  UT_std_string_setProperty(sFrameProps, sProp, sVal);
			  sProp = "right-style";
			  UT_std_string_setProperty(sFrameProps, sProp, sVal);
			  sProp = "left-style";
			  UT_std_string_setProperty(sFrameProps, sProp, sVal);
			  sProp = "bot-style";
			  UT_std_string_setProperty(sFrameProps, sProp, sVal);
//
// Set width/Height
//
			  sProp = "frame-width";
			  sVal = sWidth;
			  UT_std_string_setProperty(sFrameProps, sProp, sVal);
			  sProp = "frame-height";
			  sVal = sHeight;
			  UT_std_string_setProperty(sFrameProps, sProp, sVal);
			  double xpos = 0.0;
			  double ypos= 0.0;

			  sProp = "position-to";
			  if(pDialog->getPositionTo() == POSITION_TO_PARAGRAPH)
			  {
				  sVal = "block-above-text";
				  UT_std_string_setProperty(sFrameProps, sProp, sVal);
//
// Now calculate the Y offset to the paragraph
//
				  UT_sint32 xBlockOff,yBlockOff = 0;
				  UT_DebugOnly<bool> bValid = false;
				  bValid = pBlock->getXYOffsetToLine(xBlockOff,yBlockOff,pLine);
				  UT_ASSERT(bValid);
				  ypos = static_cast<double>(yBlockOff)/static_cast<double>(UT_LAYOUT_RESOLUTION);
				  sProp = "ypos";
				  sVal = UT_formatDimensionedValue(ypos,"in", NULL);
				  UT_std_string_setProperty(sFrameProps, sProp, sVal);
			  }
			  else if(pDialog->getPositionTo() == POSITION_TO_COLUMN)
			  {
				  sVal = "column-above-text";
				  UT_std_string_setProperty(sFrameProps, sProp, sVal);
//
// Now calculate the Y offset to the Column
//
				  UT_sint32 yLine = pLine->getY();
				  ypos = static_cast<double>(yLine)/static_cast<double>(UT_LAYOUT_RESOLUTION);
				  sProp = "frame-col-ypos";
				  sVal = UT_formatDimensionedValue(ypos,"in", NULL);
				  UT_std_string_setProperty(sFrameProps, sProp, sVal);
			  }
			  else if(pDialog->getPositionTo() == POSITION_TO_PAGE)
			  {
				  sVal = "page-above-text";
				  UT_std_string_setProperty(sFrameProps, sProp, sVal);
//
// Now calculate the Y offset to the Page
//
//
// Need this for the X/Y calculations to follow.
//
				  fp_Container * pCol = pLine->getColumn();
				  UT_ASSERT(pCol->getContainerType() == FP_CONTAINER_COLUMN);
				  UT_sint32 yLine = pLine->getY() + pCol->getY();
				  ypos = static_cast<double>(yLine)/static_cast<double>(UT_LAYOUT_RESOLUTION);
				  sProp = "frame-page-ypos";
				  sVal = UT_formatDimensionedValue(ypos,"in", NULL);
				  UT_std_string_setProperty(sFrameProps, sProp, sVal);
			  }
//
// Now set the wrapping type and the x-offset
//
			  if(pDialog->getWrapping() == WRAP_TEXTLEFT)
			  {
				  sProp = "wrap-mode";
				  sVal = "wrapped-to-left";
				  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  UT_sint32 ix = 0;
				  iWidth = UT_convertToLogicalUnits(sWidth.c_str());
				  if(pDialog->getPositionTo() == POSITION_TO_PARAGRAPH)
				  {
					  fp_Container * pCol = pLine->getColumn();
					  ix = pCol->getWidth() - pBlock->getRightMargin() - iWidth;
					  xpos =  static_cast<double>(ix)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					  sProp = "xpos";
					  sVal = UT_formatDimensionedValue(xpos,"in", NULL);
					  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  }
				  else if(pDialog->getPositionTo() == POSITION_TO_COLUMN)
				  {
					  fp_Container * pCol = pLine->getColumn();
					  ix = pCol->getWidth() -iWidth;
					  xpos =  static_cast<double>(ix)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					  sProp = "frame-col-xpos";
					  sVal = UT_formatDimensionedValue(xpos,"in", NULL);
					  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  }
				  else if(pDialog->getPositionTo() == POSITION_TO_PAGE)
				  {
					  fp_Page * pPage = pLine->getPage();
					  ix = pPage->getWidth() - iWidth;
					  xpos =  static_cast<double>(ix)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					  sProp = "frame-page-xpos";
					  sVal = UT_formatDimensionedValue(xpos,"in", NULL);
					  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  }
			  }
			  else if(pDialog->getWrapping() == WRAP_NONE)
			  {
				  sProp = "wrap-mode";
				  sVal = "above-text";
				  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  UT_sint32 ix = 0;
				  if(pDialog->getPositionTo() == POSITION_TO_PARAGRAPH)
				  {
					  xpos =  static_cast<double>(ix)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					  sProp = "xpos";
					  sVal = UT_formatDimensionedValue(xpos,"in", NULL);
					  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  }
				  else if(pDialog->getPositionTo() == POSITION_TO_COLUMN)
				  {
					  xpos =  static_cast<double>(ix)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					  sProp = "frame-col-xpos";
					  sVal = UT_formatDimensionedValue(xpos,"in", NULL);
					  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  }
				  else if(pDialog->getPositionTo() == POSITION_TO_PAGE)
				  {
					  xpos =  static_cast<double>(ix)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					  sProp = "frame-page-xpos";
					  sVal = UT_formatDimensionedValue(xpos,"in", NULL);
					  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  }

			  }
			  else if(pDialog->getWrapping() == WRAP_TEXTRIGHT)
			  {
				  sProp = "wrap-mode";
				  sVal = "wrapped-to-right";
				  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  UT_sint32 ix = 0;
				  if(pDialog->getPositionTo() == POSITION_TO_PARAGRAPH)
				  {
					  xpos =  static_cast<double>(ix)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					  sProp = "xpos";
					  sVal = UT_formatDimensionedValue(xpos,"in", NULL);
					  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  }
				  else if(pDialog->getPositionTo() == POSITION_TO_COLUMN)
				  {
					  xpos =  static_cast<double>(ix)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					  sProp = "frame-col-xpos";
					  sVal = UT_formatDimensionedValue(xpos,"in", NULL);
					  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  }
				  else if(pDialog->getPositionTo() == POSITION_TO_PAGE)
				  {
					  xpos =  static_cast<double>(ix)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					  sProp = "frame-page-xpos";
					  sVal = UT_formatDimensionedValue(xpos,"in", NULL);
					  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  }
			  }
			  else if(pDialog->getWrapping() == WRAP_TEXTBOTH)
			  {
				  sProp = "wrap-mode";
				  sVal = "wrapped-both";
				  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  UT_sint32 ix = pRun->getX();
				  if(pDialog->getPositionTo() == POSITION_TO_PARAGRAPH)
				  {
					  ix += pLine->getX();
					  xpos =  static_cast<double>(ix)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					  sProp = "xpos";
					  sVal = UT_formatDimensionedValue(xpos,"in", NULL);
					  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  }
				  else if(pDialog->getPositionTo() == POSITION_TO_COLUMN)
				  {
					  ix += pLine->getX();
					  xpos =  static_cast<double>(ix)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					  sProp = "frame-col-xpos";
					  sVal = UT_formatDimensionedValue(xpos,"in", NULL);
					  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  }
				  else if(pDialog->getPositionTo() == POSITION_TO_PAGE)
				  {
					  fp_Column * pCol = static_cast<fp_Column *>(pLine->getColumn());
					  ix += pLine->getX() + pCol->getX();
					  xpos =  static_cast<double>(ix)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					  sProp = "frame-page-xpos";
					  sVal = UT_formatDimensionedValue(xpos,"in", NULL);
					  UT_std_string_setProperty(sFrameProps, sProp, sVal);
				  }
			  }
			  if(pDialog->isTightWrap())
			  {
			    sProp = "tight-wrap";
			    sVal = "1";
			    UT_std_string_setProperty(sFrameProps, sProp, sVal);
			  }
			  else
			  {
			    sProp = "tight-wrap";
			    sVal = "0";
			    UT_std_string_setProperty(sFrameProps, sProp, sVal);
			  }
//
// Now define the Frame attributes strux
//
			  const gchar * attributes[9] = {PT_STRUX_IMAGE_DATAID,
											 NULL,"props",NULL,"title",NULL,"alt",NULL,NULL};

			  attributes[1] = dataID;
			  attributes[3] = sFrameProps.c_str();
			  attributes[5] = pDialog->getTitle().utf8_str();
			  attributes[7] = pDialog->getDescription().utf8_str();

//
// This deletes the inline image and places a positioned image in it's place
// It deals with the undo/general update issues.
//
			  pView->convertInLineToPositioned(pos,attributes);
//
// Done! Now have a positioned image!
//
		  }
//
// Change properties of a positioned image
//
		  else if( (oldWrap != WRAP_INLINE) && (newWrap != WRAP_INLINE))
		  {

		  }
//
// Convert a positioned image to an inline image
//
		  else if((oldWrap != WRAP_INLINE) && (newWrap == WRAP_INLINE))
		  {
		  }
	  }
	  pDialogFactory->releaseDialog(pDialog);
	  return true;
	}
	else
	{
		return false;
	}
}


Defun(dlgFmtImageCtxt)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
   	return s_doFormatImageDlg(pView,pCallData,true);
}


Defun(dlgFmtImage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	if(pView->getFrameEdit()->isActive())
	{
	  fl_FrameLayout * pFL = pView->getFrameLayout();

	  if(pFL == NULL)
	  {
	    return false;
	  }
	  if(pFL->getFrameType() == FL_FRAME_TEXTBOX_TYPE)
	  {
	    return true;
	  }
	  return EX(dlgFmtPosImage);
	}

	return s_doFormatImageDlg(pView,pCallData,false);
}


Defun(dlgColumns)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_Columns * pDialog
		= static_cast<AP_Dialog_Columns *>(pDialogFactory->requestDialog(AP_DIALOG_ID_COLUMNS));
UT_return_val_if_fail(pDialog, false);
	UT_uint32 iColumns = 1;
	bool bLineBetween = false;
	bool bSpaceAfter = false;
	bool bMaxHeight = false;

	const gchar ** props_in = NULL;
	const gchar * sz = NULL;

	bool bResult = pView->getSectionFormat(&props_in);

	if (!bResult)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
		const gchar * properties[] = { "columns", buf, "column-line", buf2, "dom-dir", buf3, "text-align", buf4, 0};

		UT_sint32 num_in_props = sizeof(properties)/sizeof(gchar *);
		UT_sint32 num_out_props = num_in_props;
		if(bMaxHeight)
		{
			num_out_props += 2;
		}
		if(bSpaceAfter)
		{
			num_out_props += 2;
		}
		const gchar ** props = static_cast<const gchar **>(UT_calloc(num_out_props,sizeof(gchar *)));
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
	
	UT_return_val_if_fail(pView, false);
	UT_UTF8String utf8(pCallData->m_pData, pCallData->m_dataLength);
	const gchar * style = reinterpret_cast<const gchar *>(utf8.utf8_str());
	pView->setStyle(style,false);
	pView->notifyListeners(AV_CHG_MOTION  | AV_CHG_HDRFTR);
	
	return true;
}

static bool s_doStylesDlg(FV_View * pView)
{
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_Styles * pDialog
		= static_cast<AP_Dialog_Styles *>(pDialogFactory->requestDialog(AP_DIALOG_ID_STYLES));
UT_return_val_if_fail(pDialog, false);	if(pView->isHdrFtrEdit())
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
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	//
	// Get all clones of this frame and set styles combo box
	//
	UT_GenericVector<XAP_Frame*> vClones;
	if(pFrame->getViewNumber() > 0)
	{
		pApp->getClones(&vClones,pFrame);
		for (UT_sint32 i = 0; i < vClones.getItemCount(); i++)
		{
			XAP_Frame * f = vClones.getNthItem(i);
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
	ABIWORD_VIEW;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_FormatFootnotes * pDialog
		= static_cast<AP_Dialog_FormatFootnotes *>(pDialogFactory->requestDialog(AP_DIALOG_ID_FORMAT_FOOTNOTES));
	UT_return_val_if_fail(pDialog, false);	
	pDialog->runModal(pFrame);
	AP_Dialog_FormatFootnotes::tAnswer ans = pDialog->getAnswer();
	if(ans == AP_Dialog_FormatFootnotes::a_OK)
	{
//
// update all the layouts.
//
// Clear out pending redraws...
//
		lockGUI();
		pFrame->nullUpdate();
		pDialog->updateDocWithValues();
		pView->updateScreen(false);
		unlockGUI();
	}
	pDialogFactory->releaseDialog(pDialog);
	return true;
}

Defun1(dlgStyle)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	ABIWORD_VIEW;

	return s_doStylesDlg(pView);
}

Defun1(dlgStylist)
{
	CHECK_FRAME;

	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_Stylist * pDialog
		= static_cast<AP_Dialog_Stylist *>(pDialogFactory->requestDialog(AP_DIALOG_ID_STYLIST));
UT_return_val_if_fail(pDialog, false);
	if(pDialog->isRunning())
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
	UT_return_val_if_fail(pView,false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_WordCount * pDialog
		= static_cast<AP_Dialog_WordCount *>(pDialogFactory->requestDialog(AP_DIALOG_ID_WORDCOUNT));
UT_return_val_if_fail(pDialog, false);
	if(pDialog->isRunning())
	{
		pDialog->activate();
	}
	else
	{
		pDialog->setCount(pView->countWords(true));
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
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_InsertTable * pDialog
		= static_cast<AP_Dialog_InsertTable *>(pDialogFactory->requestDialog(AP_DIALOG_ID_INSERT_TABLE));
UT_return_val_if_fail(pDialog, false);
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
			UT_LocaleTransactor t(LC_NUMERIC, "C");
			for (UT_uint32 i = 0; i<pDialog->getNumCols(); i++)
			{
				UT_String_sprintf(tmp, "%fin/", pDialog->getColumnWidth());
				propBuffer += tmp;
			}
			const gchar * propsArray[3];
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

Defun1(sortColsAscend)
{
	UT_UNUSED(pAV_View);
	CHECK_FRAME;
	//ABIWORD_VIEW;
	return true;
}

Defun1(sortColsDescend)
{
	UT_UNUSED(pAV_View);
	CHECK_FRAME;
	//ABIWORD_VIEW;
	return true;
}

Defun1(sortRowsAscend)
{
	UT_UNUSED(pAV_View);
	CHECK_FRAME;
	//ABIWORD_VIEW;
	return true;
}

Defun1(sortRowsDescend)
{
	UT_UNUSED(pAV_View);
	CHECK_FRAME;
	//ABIWORD_VIEW;
	return true;
}

Defun1(textToTable)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	pView->cmdTextToTable(3);
	return true;
}

Defun1(textToTableTabs)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	pView->cmdTextToTable(0);
	return true;
}

Defun1(textToTableCommas)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	pView->cmdTextToTable(1);
	return true;
}

Defun1(textToTableSpaces)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	pView->cmdTextToTable(2);
	return true;
}

Defun1(insertSumRows)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	const gchar * atts[3]={"param","",NULL};
	pView->cmdInsertField("sum_rows",atts,NULL);
	return true;
}

Defun1(insertSumCols)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	const gchar * atts[3]={"param","",NULL};
	pView->cmdInsertField("sum_cols",atts,NULL);
	return true;
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
					  double &page_margin_right,
					  double &page_margin_top,
					  double &page_margin_bottom)
{
  UT_return_if_fail(pView);
  // get current char properties from pView
  const gchar * prop = NULL;
  const gchar ** props_in = NULL;
  const gchar * sz = NULL;

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

	{
		prop = "page-margin-top";
		pView->getSectionFormat(&props_in);
		sz = UT_getAttribute(prop, props_in);
		page_margin_top	 = UT_convertToInches(sz);
		FREEP(props_in);
	}

	{
		prop = "page-margin-bottom";
		pView->getSectionFormat(&props_in);
		sz = UT_getAttribute(prop, props_in);
		page_margin_bottom = UT_convertToInches(sz);
		FREEP(props_in);
	}
}

// MSWord defines this to 1/2 an inch, so we do too
#define TOGGLE_INDENT_AMT 0.5

Defun1(toggleIndent)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
  bool doLists = true;
  double page_size = pView->getPageSize().Width (DIM_IN);

  double margin_left = 0., margin_right = 0., allowed = 0.,
	  page_margin_left = 0., page_margin_right = 0.,
	  page_margin_top = 0., page_margin_bottom = 0.;

  s_getPageMargins (pView, margin_left, margin_right,
					page_margin_left, page_margin_right,
					page_margin_top, page_margin_bottom);

  allowed = page_size - page_margin_left - page_margin_right;
  if (margin_left >= allowed)
	  return true;

  fl_BlockLayout * pBL = pView->getCurrentBlock();
  if(pBL && (!pBL->isListItem() || !pView->isSelectionEmpty()) )
  {
	  doLists = false;
  }
  return  pView->setBlockIndents(doLists, static_cast<double>(TOGGLE_INDENT_AMT) ,page_size);
}

Defun1(toggleUnIndent)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
  bool ret;
  double page_size = pView->getPageSize().Width (DIM_IN);
  bool doLists = true;

  double margin_left = 0., margin_right = 0., allowed = 0.,
	  page_margin_left = 0., page_margin_right = 0.,
	  page_margin_top = 0., page_margin_bottom = 0.;

  s_getPageMargins (pView, margin_left, margin_right,
					page_margin_left, page_margin_right,
					page_margin_top, page_margin_bottom);

  fl_BlockLayout * pBL = pView->getCurrentBlock();
  UT_BidiCharType iBlockDir = UT_BIDI_LTR;

  if(pBL)
	  iBlockDir = pBL->getDominantDirection();
  
  allowed = iBlockDir == UT_BIDI_LTR ? margin_left : margin_right;
  if ( allowed <= 0. )
	  return true ;

  if(pBL && (!pBL->isListItem() || !pView->isSelectionEmpty()) )
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

	UT_return_val_if_fail(pView,false);

	const gchar * properties[] = { "dom-dir", NULL, "text-align", NULL, 0};
	const gchar drtl[]	= "rtl";
	const gchar dltr[]	= "ltr";
	const gchar aright[] = "right";
	const gchar aleft[]	= "left";
	gchar cur_alignment[10];

	fl_BlockLayout * pBl = pView->getCurrentBlock();
	UT_return_val_if_fail( pBl, false );

	strncpy(cur_alignment,pBl->getProperty("text-align"),sizeof(cur_alignment)-1);
	cur_alignment[sizeof(cur_alignment)-1] = '\0';

	properties[3] = static_cast<gchar *>(&cur_alignment[0]);


	if(pBl->getDominantDirection()== UT_BIDI_RTL)
	{
		properties[1] = static_cast<const gchar *>(&dltr[0]);
		/*
		//the last run in the block is the FmtMark, and we need
		//to reset its direction
		static_cast<fp_Line *>(static_cast<fl_BlockLayout *>(pBl)->getLastContainer())->getLastRun()->setDirection(UT_BIDI_LTR);
		*/
	}
	else
	{
		properties[1] = static_cast<const gchar *>(&drtl[0]);
		/*
		static_cast<fp_Line *>(static_cast<fl_BlockLayout *>(pBl)->getLastContainer())->getLastRun()->setDirection(UT_BIDI_RTL);
		*/
	}

	// if the paragraph is was aligned either left or right, then
	// toggle the alignment as well; if it was anything else
	// i.e., justfied or centered, then leave it
	if(!strcmp(properties[3],aleft))
	{
		properties[3] = static_cast<const gchar *>(&aright[0]);
	}
	else if(!strcmp(properties[3],aright))
	{
		properties[3] = static_cast<const gchar *>(&aleft[0]);

	}

	pView->setBlockFormat(properties);

	return true;
}


Defun1(toggleDomDirectionSect)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);

	const gchar * properties[] = { "dom-dir", NULL, 0};
	const gchar drtl[]	= "rtl";
	const gchar dltr[]	= "ltr";

	fl_BlockLayout * pBl = pView->getCurrentBlock();
	UT_return_val_if_fail( pBl, false );

	fl_DocSectionLayout * pSL = pBl->getDocSectionLayout();
	UT_return_val_if_fail( pSL, false );
	
	if(pSL->getColumnOrder())
	{
		properties[1] = static_cast<const gchar *>(&dltr[0]);
	}
	else
	{
		properties[1] = static_cast<const gchar *>(&drtl[0]);
	}

	pView->setSectionFormat(properties);

	return true;
}

Defun1(toggleDomDirectionDoc)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc,false);

	const PP_AttrProp * pAP = pDoc->getAttrProp();
	UT_return_val_if_fail( pAP, false );

	const gchar * properties[] = { "dom-dir", NULL, 0};
	const gchar drtl[]	= "rtl";
	const gchar dltr[]	= "ltr";
	const gchar * szValue;
	
	UT_return_val_if_fail(pAP->getProperty(properties[0], szValue), false);
	
	if(!strcmp(szValue, drtl))
	{
		properties[1] = static_cast<const gchar *>(&dltr[0]);
	}
	else
	{
		properties[1] = static_cast<const gchar *>(&drtl[0]);
	}

	UT_return_val_if_fail(pDoc->setProperties(properties), false);

	return true;
}

Defun1(doBullets)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	pView->processSelectedBlocks(BULLETED_LIST);
	return true;
}

Defun1(doNumbers)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	pView->processSelectedBlocks(NUMBERED_LIST);
	return true;
}

Defun(colorForeTB)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	
	UT_return_val_if_fail(pView,false);
	const gchar * properties[] = { "color", NULL, 0};
	UT_UTF8String utf8(pCallData->m_pData, pCallData->m_dataLength);
	properties[1] = reinterpret_cast<const gchar *>(utf8.utf8_str());
	pView->setCharFormat(properties);

	return true;
}

Defun(colorBackTB)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);	
	const gchar * properties[] = { "bgcolor", NULL, 0};
	UT_UTF8String utf8(pCallData->m_pData, pCallData->m_dataLength);
	properties[1] = reinterpret_cast<const gchar *>(utf8.utf8_str());
	pView->setCharFormat(properties);

	return true;
}

/*! removes the "props" attribute, i.e., all non-style based formatting
 */
Defun1(togglePlain)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	if (pView->getDocument()->areStylesLocked())
		return true;

	pView->resetCharFormat(false);
		
	return true;
}

Defun1(alignLeft)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	if (pView->getDocument()->areStylesLocked())
		return true;

	const gchar * properties[] = { "text-align", "left", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(alignCenter)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	if (pView->getDocument()->areStylesLocked())
		return true;

	const gchar * properties[] = { "text-align", "center", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(alignRight)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	if (pView->getDocument()->areStylesLocked())
		return true;

	const gchar * properties[] = { "text-align", "right", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(alignJustify)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	if (pView->getDocument()->areStylesLocked())
		return true;

	const gchar * properties[] = { "text-align", "justify", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(setStyleHeading1)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	const gchar * style = "Heading 1";
	pView->setStyle(style,false);
	pView->notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR);
	return true;
}


Defun1(setStyleHeading2)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	const gchar * style = "Heading 2";
	pView->setStyle(style,false);
	pView->notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR);
	return true;
}

Defun1(setStyleHeading3)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	const gchar * style = "Heading 3";
	pView->setStyle(style,false);
	pView->notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR);
	return true;
}

Defun1(sectColumns1)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	if(pView->isHdrFtrEdit())
		return false;

	const gchar * properties[] = { "columns", "1", 0};
	pView->setSectionFormat(properties);
	return true;
}

Defun1(sectColumns2)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	if(pView->isHdrFtrEdit())
		return false;

	const gchar * properties[] = { "columns", "2", 0};
	pView->setSectionFormat(properties);
	return true;
}

Defun1(sectColumns3)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	if(pView->isHdrFtrEdit())
		return false;
	const gchar * properties[] = { "columns", "3", 0};
	pView->setSectionFormat(properties);
	return true;
}

Defun1(paraBefore0)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	if (pView->getDocument()->areStylesLocked())
		return true;

	const gchar * properties[] = { "margin-top", "0pt", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(paraBefore12)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	if (pView->getDocument()->areStylesLocked())
		return true;

	const gchar * properties[] = { "margin-top", "12pt", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(singleSpace)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	if (pView->getDocument()->areStylesLocked())
		return true;

	const gchar * properties[] = { "line-height", "1.0", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(middleSpace)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	if (pView->getDocument()->areStylesLocked())
		return true;

	const gchar * properties[] = { "line-height", "1.5", 0};
	pView->setBlockFormat(properties);
	return true;
}

Defun1(doubleSpace)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	if (pView->getDocument()->areStylesLocked())
		return true;

	const gchar * properties[] = { "line-height", "2.0", 0};
	pView->setBlockFormat(properties);
	return true;
}

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
Defun1(Test_Dump)
{
	CHECK_FRAME;
//	ABIWORD_VIEW;
//	UT_return_val_if_fail(pView,false);
//	pView->Test_Dump();
	return true;
}

Defun1(Test_Ftr)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	pView->insertPageNum(NULL, FL_HDRFTR_FOOTER);
	return true;
}
#endif

Defun1(setEditVI)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	// enter "VI Edit Mode" (only valid when VI keys are loaded)
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	// When exiting input mode, vi goes to previous character
	pView->cmdCharMotion(false,1);

	bool bResult = (XAP_App::getApp()->setInputMode("viEdit") != 0);
	return bResult;
}

Defun1(setInputVI)
{
	CHECK_FRAME;
// enter "VI Input Mode" (only valid when VI keys are loaded)
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	bool bResult = (XAP_App::getApp()->setInputMode("viInput") != 0);
	return bResult;
}

Defun1(cycleInputMode)
{
	CHECK_FRAME;
// switch to the next input mode { default, emacs, vi, ... }
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);

	// this edit method may get ignored entirely
	bool b;
	if (pPrefs->getPrefsValueBool(static_cast<const gchar *>(AP_PREF_KEY_KeyBindingsCycle), &b) && !b)
		return false;

	const char * szCurrentInputMode = pApp->getInputMode();
	UT_return_val_if_fail (szCurrentInputMode, false);
	AP_BindingSet * pBSet = static_cast<AP_BindingSet *>(pApp->getBindingSet());
	const char * szNextInputMode = pBSet->getNextInCycle(szCurrentInputMode);
	if (!szNextInputMode)				// probably an error....
		return false;

	bool bResult = (pApp->setInputMode(szNextInputMode) != 0);

	// POLICY: make this the default for new frames, too
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
UT_return_val_if_fail(pScheme, false);
	pScheme->setValue(static_cast<const gchar *>(AP_PREF_KEY_KeyBindings),
					  szNextInputMode);

	return bResult;
}

Defun1(toggleInsertMode)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, false);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_val_if_fail(pPrefs, false);

	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
    UT_return_val_if_fail(pFrameData, false);

	// this edit method may get ignored entirely
	bool b;
	if (pPrefs->getPrefsValueBool(AP_PREF_KEY_InsertModeToggle, &b) && !b) {
        // if we are in insert mode, just return, otherwise give a chance
        // to toggle it, or one might get stick to overwrite.
        if(pFrameData->m_bInsertMode) {
            return false;
        }
    }

	// toggle the insert mode
	pFrameData->m_bInsertMode = ! pFrameData->m_bInsertMode;

	// the view actually does the dirty work
	pAV_View->setInsertMode(pFrameData->m_bInsertMode);

	if (pFrameData->m_pStatusBar)
	  pFrameData->m_pStatusBar->notify(pAV_View, AV_CHG_ALL);

	// POLICY: make this the default for new frames, too
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
UT_return_val_if_fail(pScheme, false);
	pScheme->setValueBool(AP_PREF_KEY_InsertMode, pFrameData->m_bInsertMode);

	return true;
}


//////////////////////////////////////////////////////////////////
// The following commands are suggested for the various VI keybindings.
// It may be possible to use our exisiting methods for them, but I
// didn't know all of the little (subtle) side-effects that make VI
// special.
//////////////////////////////////////////////////////////////////

Defun(viCmd_5e)
{
	CHECK_FRAME;
	//Move to first non space char on current line 
	//TODO: BOL seems to count as a BOW, how to move to first non space? 
	return ( EX(warpInsPtBOL));
}

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
	return ( EX(delEOL) );
}

/* d( */
Defun(viCmd_d28)
{
	CHECK_FRAME;
//delete to start of sentence
	return ( EX(delBOS) );
}

/* d) */
Defun(viCmd_d29)
{
	CHECK_FRAME;
//delete to end of sentence
	return ( EX(delEOS) );
}

/* d[ */
Defun(viCmd_d5b)
{
	CHECK_FRAME;
//delete to beginning of block
	return ( EX(delBOB) );
}

/* d] */
Defun(viCmd_d5d)
{
	CHECK_FRAME;
//delete to end of block
	return ( EX(delEOB) );
}

/* d^ */
Defun(viCmd_d5e)
{
	CHECK_FRAME;
//delete to beginning of line
	return ( EX(delBOL) );
}

Defun(viCmd_db)
{
	CHECK_FRAME;
//delete to beginning of word
	return ( EX(delBOW) );
}

Defun(viCmd_dd)
{
	CHECK_FRAME;
// delete the current line
	return ( EX(warpInsPtBOL) && EX(delEOL) && EX(delLeft) && EX(warpInsPtBOL) );
}

Defun(viCmd_dw)
{
	CHECK_FRAME;
//delete to end of word
	return ( EX(delEOW) );
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

static bool s_AskForScriptName(XAP_Frame * pFrame,
							   UT_String& stPathname,
							   UT_ScriptIdType * ieft)
{
	UT_return_val_if_fail (ieft, false);

	stPathname.clear();

	pFrame->raise();

	XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_OPEN;

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog
		= static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(id));
	UT_return_val_if_fail(pDialog, false);

	UT_ScriptLibrary * instance = UT_ScriptLibrary::instance ();

	UT_uint32 filterCount = instance->getNumScripts ();

	const char ** szDescList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	UT_return_val_if_fail(szDescList, false);

	const char ** szSuffixList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	if(!szSuffixList)
	{
		UT_ASSERT_HARMLESS(szSuffixList);
		FREEP(szDescList);
		return false;
	}

	UT_ScriptIdType * nTypeList = static_cast<UT_ScriptIdType *>(UT_calloc(filterCount + 1, sizeof(UT_ScriptIdType)));
	if(!nTypeList)
	{
		UT_ASSERT_HARMLESS(nTypeList);
		FREEP(szDescList);
		FREEP(szSuffixList);
		return false;
	}

	UT_uint32 k = 0;

	while (instance->enumerateDlgLabels(k, &szDescList[k],
					   &szSuffixList[k], &nTypeList[k]))
		k++;

	pDialog->setFileTypeList(szDescList, szSuffixList,
							 static_cast<const UT_sint32 *>(nTypeList));

	UT_ScriptIdType dflFileType = -1;
	pDialog->setDefaultFileType(dflFileType);

	pDialog->runModal(pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const std::string & resultPathname = pDialog->getPathname();

		if (!resultPathname.empty()) {
			stPathname += resultPathname;
		}

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
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}
		else
			*ieft = static_cast<UT_ScriptIdType>(pDialog->getFileType());
	}

	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

class ABI_EXPORT OneShot_MailMerge_Listener : public IE_MailMerge::IE_MailMerge_Listener
{
public:

	explicit OneShot_MailMerge_Listener (PD_Document * pd)
		: IE_MailMerge::IE_MailMerge_Listener (), m_doc (pd)
		{

		}

	virtual ~OneShot_MailMerge_Listener ()
		{
		}
		
	virtual PD_Document* getMergeDocument () const
		{
			return m_doc;
		}
	
	virtual bool fireUpdate () 
		{
			// don't process any more data
			return false;
		}
	
private:
	PD_Document *m_doc;
};

Defun1(mailMerge)
{
  CHECK_FRAME;
  UT_return_val_if_fail(pAV_View, false);
  XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
  UT_return_val_if_fail(pFrame, false);

  PD_Document * pDoc = static_cast<PD_Document *>(pFrame->getCurrentDoc());
  UT_return_val_if_fail(pDoc, false);

  pFrame->raise();
  XAP_Dialog_Id id = XAP_DIALOG_ID_FILE_OPEN;
  
  XAP_DialogFactory * pDialogFactory
    = static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());
  
  XAP_Dialog_FileOpenSaveAs * pDialog
    = static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(id));
  UT_return_val_if_fail (pDialog, false);

  UT_uint32 filterCount = 0;
  
  filterCount = IE_MailMerge::getMergerCount();

  const char ** szDescList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
  UT_return_val_if_fail(szDescList, false);

  const char ** szSuffixList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
  if(!szSuffixList)
  {
	  UT_ASSERT_HARMLESS(szSuffixList);
	  FREEP(szDescList);
	  return false;
  }

  IEMergeType * nTypeList = static_cast<IEMergeType *>(UT_calloc(filterCount + 1, sizeof(IEMergeType)));
  if(!nTypeList)
  {
	  UT_ASSERT_HARMLESS(nTypeList);
	  FREEP(szDescList);
	  FREEP(szSuffixList);
	  return false;
  }

  UT_uint32 k = 0;
  
  while (IE_MailMerge::enumerateDlgLabels(k, &szDescList[k], &szSuffixList[k], &nTypeList[k]))
	  k++;

  pDialog->setFileTypeList(szDescList, szSuffixList, static_cast<const UT_sint32 *>(nTypeList));

  pDialog->setDefaultFileType(IE_MailMerge::fileTypeForSuffix (".xml"));

  pDialog->runModal(pFrame);

  XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
  bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

  if (bOK)
    {
		UT_String filename (pDialog->getPathname());
		UT_sint32 type = pDialog->getFileType();
		
		IE_MailMerge * pie = NULL;
		UT_Error errorCode = IE_MailMerge::constructMerger(filename.c_str(), static_cast<IEMergeType>(type), &pie);
		if (!errorCode)
		{
			OneShot_MailMerge_Listener listener (pDoc);
			pie->setListener (&listener);
			pie->mergeFile (filename.c_str());
			DELETEP(pie);
		}
	}

  pDialogFactory->releaseDialog(pDialog);
  return true;
}

Defun1(scriptPlay)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);

	UT_String pNewFile;

	UT_ScriptLibrary * instance = UT_ScriptLibrary::instance ();

	if (0 == instance->getNumScripts())
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

	// we have no expectations of executing a remote program
	char * scriptName = UT_go_filename_from_uri(pNewFile.c_str());
	UT_return_val_if_fail (scriptName != NULL, false);

#ifdef _WIN32
	// we need to add quotes to the script name _after_ the UT_go_filename_from_uri() call above;
	// if not, it will return NULL and the script won't play.

	UT_UTF8String script = "\"";
	script += scriptName;
	script += "\"";
	g_free(scriptName);
	scriptName = g_strdup(script.utf8_str());
#endif

	UT_DEBUGMSG(("scriptPlay (trying to play [%s])\n", pNewFile.c_str()));

	if (UT_OK != instance->execute(scriptName, ieft))
	{
		if (instance->errmsg().size() > 0)
		{
			pFrame->showMessageBox(instance->errmsg().c_str(),
					       XAP_Dialog_MessageBox::b_O,
					       XAP_Dialog_MessageBox::a_OK);
		}
		else
			pFrame->showMessageBox(AP_STRING_ID_SCRIPT_CANTRUN,
					       XAP_Dialog_MessageBox::b_O,
					       XAP_Dialog_MessageBox::a_OK,
					       scriptName);
	}

	g_free (scriptName);

	return true;
}

Defun(executeScript)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	XAP_Frame* pFrame = static_cast<XAP_Frame *> (pAV_View->getParentData());
	UT_return_val_if_fail(pFrame, false);
	UT_DEBUGMSG(("executeScript (trying to execute [%s])\n", pCallData->getScriptName().c_str()));

	UT_ScriptLibrary * instance = UT_ScriptLibrary::instance ();

	// we have no expectations of executing a remote program
	char * scriptName = UT_go_filename_from_uri (pCallData->getScriptName().c_str());
	UT_return_val_if_fail (scriptName != NULL, false);

#ifdef _WIN32
	// we need to add quotes to the script name _after_ the UT_go_filename_from_uri() call above;
	// if not, it will return NULL and the script won't execute.

	UT_UTF8String script = "\"";
	script += scriptName;
	script += "\"";
	g_free(scriptName);
	scriptName = g_strdup(script.utf8_str());
#endif

	if (UT_OK != instance->execute(scriptName))
	{
		if (instance->errmsg().size() > 0)
			pFrame->showMessageBox(instance->errmsg().c_str(),
								   XAP_Dialog_MessageBox::b_O,
								   XAP_Dialog_MessageBox::a_OK);

		else
			pFrame->showMessageBox(AP_STRING_ID_SCRIPT_CANTRUN,
								   XAP_Dialog_MessageBox::b_O,
								   XAP_Dialog_MessageBox::a_OK,
								   scriptName);
	}

	g_free (scriptName);

	return true;
}


Defun1(dlgColorPickerFore)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	FV_View * pView = static_cast<FV_View *>(pAV_View);
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_Background * pDialog
		= static_cast<AP_Dialog_Background *>(pDialogFactory->requestDialog(AP_DIALOG_ID_BACKGROUND));
UT_return_val_if_fail(pDialog, false);//
// Set the color in the dialog to the current Color
//
	const gchar ** propsChar = NULL;
	pView->getCharFormat(&propsChar);
	const gchar * pszChar = UT_getAttribute("color",propsChar);
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
		const gchar * clr = pDialog->getColor();
		const gchar * properties[] = { "color", NULL, 0};
		properties[1] = clr;
		pView->setCharFormat(properties);
	}
	pDialogFactory->releaseDialog(pDialog);
	FREEP(propsChar);
	return bOK;
}


Defun1(dlgColorPickerBack)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	FV_View * pView = static_cast<FV_View *>(pAV_View);

	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_Background * pDialog
		= static_cast<AP_Dialog_Background *>(pDialogFactory->requestDialog(AP_DIALOG_ID_BACKGROUND));
UT_return_val_if_fail(pDialog, false);//
// Set the color in the dialog to the current Color
//
	const gchar ** propsChar = NULL;
	pView->getCharFormat(&propsChar);
	const gchar * pszChar = UT_getAttribute("bgcolor",propsChar);
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
		const gchar * clr = pDialog->getColor();
		const gchar * properties[] = { "bgcolor", NULL, 0};
		properties[1] = clr;
		pView->setCharFormat(properties);
	}
	FREEP(propsChar);
	pDialogFactory->releaseDialog(pDialog);
	return bOK;
}

Defun1(dlgBackground)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	FV_View * pView = static_cast<FV_View *>(pAV_View);

	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_Background * pDialog
		= static_cast<AP_Dialog_Background *>(pDialogFactory->requestDialog(AP_DIALOG_ID_BACKGROUND));
UT_return_val_if_fail(pDialog, false);
//
// Get Current background color
//
	const gchar ** propsSection = NULL;
	pView->getSectionFormat(&propsSection);
	const gchar * pszBackground = UT_getAttribute("background-color",propsSection);
	pDialog->setColor(pszBackground);

	pDialog->runModal (pFrame);

	AP_Dialog_Background::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == AP_Dialog_Background::a_OK);

	if (bOK)
	{
		// let the view set the proper value in the
		// document and refresh/redraw itself
		const gchar * clr = pDialog->getColor();
		pView->setPaperColor (clr);
	}

	FREEP(propsSection);
	pDialogFactory->releaseDialog(pDialog);
	return bOK;
}


Defun1(dlgHdrFtr)
{
	CHECK_FRAME;
	UT_return_val_if_fail(pAV_View, false);
	FV_View * pView = static_cast<FV_View *>(pAV_View);

	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_HdrFtr * pDialog = static_cast<AP_Dialog_HdrFtr *>(pDialogFactory->requestDialog(AP_DIALOG_ID_HDRFTR));
UT_return_val_if_fail(pDialog, false);//
// Get stuff we need from the view
//
	if(pView->isHdrFtrEdit())
	{	
		pView->clearHdrFtrEdit();
		pView->warpInsPtToXY(0,0,false);
	}

	fl_BlockLayout *pBL = pView->getCurrentBlock();
	UT_return_val_if_fail( pBL, false );
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pBL->getDocSectionLayout());
	UT_ASSERT(pDSL->getContainerType() == FL_CONTAINER_DOCSECTION);
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
	const gchar ** propsSectionIn = NULL;
	pView->getSectionFormat(&propsSectionIn);
	const char * szRestart = NULL;
	szRestart = UT_getAttribute("section-restart",propsSectionIn);
	const char * szRestartValue1 = NULL;
	szRestartValue1 = UT_getAttribute("section-restart-value",propsSectionIn);
	bool bRestart = false;
	if(szRestart && *szRestart && (strcmp(szRestart,"1") == 0))
	{
		bRestart = true;
	}
	UT_sint32 restartValue = 1;
	if(szRestartValue1 && *szRestartValue1)
	{
		restartValue = atoi(szRestartValue1);
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
				sprintf(static_cast<char *>(szRestartValue),"%i",pDialog->getRestartValue());
				props_out[3] = static_cast<const char *>(szRestartValue);
			}
			else
			{
				props_out[1] = "0";
				props_out[2] = NULL;
			}
			pView->setSectionFormat(static_cast<const char **>(&props_out[0]));
		}
		pView->notifyListeners(AV_CHG_ALL);
	}

	pDialogFactory->releaseDialog(pDialog);
	return bOK;
}

Defun1(hyperlinkCopyLocation)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	pView->cmdHyperlinkCopyLocation(pView->getPoint());
	return true;
}

Defun(hyperlinkJump)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	
	fp_Run * pRun = pView->getHyperLinkRun(pView->getPoint());
	fp_HyperlinkRun * pHRun = NULL;
	if(pRun)
		pHRun = pRun->getHyperlink();
	
	if(pHRun && pHRun->getHyperlinkType() == HYPERLINK_NORMAL)
	{
		UT_DEBUGMSG(("hyperlinkJump: Normal hyperlink jump\n"));
		pView->cmdHyperlinkJump(pCallData->m_xPos, pCallData->m_yPos);
	}
	
	if(pHRun && pHRun->getHyperlinkType() == HYPERLINK_ANNOTATION)
	{
		// This is the behaveour when double clicking an annotation hypermark
		UT_DEBUGMSG(("hyperlinkJump: Hyperlink annotation (no jump) edit dialog\n"));
		fp_AnnotationRun * pARun = static_cast<fp_AnnotationRun *>(pHRun);
		pView->cmdEditAnnotationWithDialog(pARun->getPID());
	}
	
	return true;
}


Defun1(hyperlinkJumpPos)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	UT_DEBUGMSG(("hyperlinkJumpPos\n"));
	pView->cmdHyperlinkJump(pView->getPoint());
	return true;
}

Defun1(rdfAnchorEditTriples)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	XAP_Dialog_Id id = AP_DIALOG_ID_RDF_EDITOR;
	AP_Dialog_RDFEditor* dialog = 0;
	return s_doRDFEditorDlg( pView, id, dialog, true );
}
Defun1(rdfAnchorQuery)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	return rdfQueryXMLIDs( pView, 0 );
}

Defun1(rdfAnchorEditSemanticItem)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	if( PD_Document * pDoc = pView->getDocument() )
	{
		if( PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF() )
		{
			std::set< std::string > xmlids;
			rdf->addRelevantIDsForPosition( xmlids, pView->getPoint() );

			PD_RDFSemanticItems sl = rdf->getSemanticObjects( xmlids );
			rdf->showEditorWindow( sl );
			
			
			// PD_RDFContacts contacts = rdf->getContacts();
			// for( PD_RDFContacts::iterator ci = contacts.begin();
			// 	 ci != contacts.end(); ++ci )
			// {
			// 	PD_RDFContactHandle c = *ci;
			// 	std::set< std::string > clist = c->getXMLIDs();
			// 	std::set< std::string > tmp;
			// 	std::set_intersection( xmlids.begin(), xmlids.end(),
			// 						   clist.begin(), clist.end(),
			// 						   inserter( tmp, tmp.end() ));
			// 	if( !tmp.empty() )
			// 		c->showEditorWindow(c);
			// }

			// PD_RDFEvents events = rdf->getEvents();
			// for( PD_RDFEvents::iterator ci = events.begin();
			// 	 ci != events.end(); ++ci )
			// {
			// 	PD_RDFEventHandle c = *ci;
			// 	std::set< std::string > clist = c->getXMLIDs();
			// 	std::set< std::string > tmp;
			// 	std::set_intersection( xmlids.begin(), xmlids.end(),
			// 						   clist.begin(), clist.end(),
			// 						   inserter( tmp, tmp.end() ));
			// 	if( !tmp.empty() )
			// 		c->showEditorWindow(c);
			// }


			// PD_RDFLocations locations = rdf->getLocations();
			// for( PD_RDFLocations::iterator ci = locations.begin();
			// 	 ci != locations.end(); ++ci )
			// {
			// 	PD_RDFLocationHandle c = *ci;
			// 	std::set< std::string > clist = c->getXMLIDs();
			// 	std::set< std::string > tmp;
			// 	std::set_intersection( xmlids.begin(), xmlids.end(),
			// 						   clist.begin(), clist.end(),
			// 						   inserter( tmp, tmp.end() ));
			// 	UT_DEBUGMSG(("location name:%s linksubj:%s c->xmlids.sz:%d tmp.sz:%d\n", c->name().c_str(), c->linkingSubject().toString().c_str(), c->getXMLIDs().size(), tmp.size() ));
			// 	if( !tmp.empty() )
			// 	{
			// 		c->showEditorWindow(c);
			// 	}
			// }
		}
	}
	return 0;
}

Defun1(rdfAnchorExportSemanticItem)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	if( PD_Document * pDoc = pView->getDocument() )
	{
		if( PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF() )
		{
			std::set< std::string > xmlids;
			rdf->addRelevantIDsForPosition( xmlids, pView->getPoint() );

			if( xmlids.empty() )
				return 0;

			std::string filename = "";
			PD_RDFSemanticItems sl = rdf->getSemanticObjects( xmlids );
			for( PD_RDFSemanticItems::iterator ci = sl.begin();
				 ci != sl.end(); ++ci )
			{
				PD_RDFSemanticItemHandle h = *ci;
				std::set< std::string > clist = h->getXMLIDs();
				std::set< std::string > tmp;
				std::set_intersection( xmlids.begin(), xmlids.end(),
									   clist.begin(), clist.end(),
									   std::inserter( tmp, tmp.end() ));
				if( !tmp.empty() )
				{
					h->exportToFile();
				}
				
			}
			
			
			// rdf->addRelevantIDsForPosition( xmlids, pView->getPoint() );
			// PD_RDFContacts contacts = rdf->getContacts();
			// for( PD_RDFContacts::iterator ci = contacts.begin();
			// 	 ci != contacts.end(); ++ci )
			// {
			// 	PD_RDFContactHandle c = *ci;
			// 	std::set< std::string > clist = c->getXMLIDs();
			// 	std::set< std::string > tmp;
			// 	std::set_intersection( xmlids.begin(), xmlids.end(),
			// 						   clist.begin(), clist.end(),
			// 						   inserter( tmp, tmp.end() ));
			// 	if( !tmp.empty() )
			// 		c->exportToFile();
			// }
			// PD_RDFEvents events = rdf->getEvents();
			// for( PD_RDFEvents::iterator ci = events.begin();
			// 	 ci != events.end(); ++ci )
			// {
			// 	PD_RDFEventHandle c = *ci;
			// 	std::set< std::string > clist = c->getXMLIDs();
			// 	std::set< std::string > tmp;
			// 	std::set_intersection( xmlids.begin(), xmlids.end(),
			// 						   clist.begin(), clist.end(),
			// 						   inserter( tmp, tmp.end() ));
			// 	if( !tmp.empty() )
			// 		c->exportToFile();
			// }
			// PD_RDFLocations locations = rdf->getLocations();
			// for( PD_RDFLocations::iterator ci = locations.begin();
			// 	 ci != locations.end(); ++ci )
			// {
			// 	PD_RDFLocationHandle c = *ci;
			// 	std::set< std::string > clist = c->getXMLIDs();
			// 	std::set< std::string > tmp;
			// 	std::set_intersection( xmlids.begin(), xmlids.end(),
			// 						   clist.begin(), clist.end(),
			// 						   inserter( tmp, tmp.end() ));
			// 	if( !tmp.empty() )
			// 		c->exportToFile();
			// }
		}
	}
	return 0;
}



    struct selectReferenceToSemanticItemRing
	{
		PD_RDFSemanticItemHandle h;
		std::set< std::string > xmlids;
		std::set< std::string >::iterator iter;
	};

	static selectReferenceToSemanticItemRing& getSelectReferenceToSemanticItemRing()
	{
		static selectReferenceToSemanticItemRing ring;
		return ring;
	}

static void setSemanticItemRing( PD_DocumentRDFHandle rdf,
								 PD_RDFSemanticItemHandle h,
								 const std::set< std::string >& xmlids,
								 const std::string& xmlid )
{
	selectReferenceToSemanticItemRing& ring = getSelectReferenceToSemanticItemRing();

	ring.h = h;
	ring.xmlids = xmlids;
	for( std::set< std::string >::iterator ri = ring.xmlids.begin();
		 ri != ring.xmlids.end(); )
	{
		std::set< std::string >::iterator t = ri;
		++ri;
		std::pair< PT_DocPosition, PT_DocPosition > range = rdf->getIDRange( *t );
		if( !range.first || range.second <= range.first )
			ring.xmlids.erase( t );
	}
	ring.iter   = ring.xmlids.find( xmlid );	
}


static void rdfAnchorSelectPos( FV_View* pView,
								PD_DocumentRDFHandle rdf,
								PT_DocPosition pos,
								bool selectit = true )
{
	UT_DEBUGMSG(("rdfAnchorSelectPos() pos:%ld\n", (long)pos ));
	selectReferenceToSemanticItemRing& ring = getSelectReferenceToSemanticItemRing();
	ring.h.reset();
	ring.xmlids.clear();
	ring.iter = ring.xmlids.end();

	std::set< std::string > xmlids;
	rdf->addRelevantIDsForPosition( xmlids, pos );
	PD_RDFSemanticItems semitems = rdf->getSemanticObjects( xmlids );
	for( PD_RDFSemanticItems::iterator si = semitems.begin(); si != semitems.end(); ++si )
	{
		PD_RDFSemanticItemHandle c = *si;
		std::set< std::string > clist = c->getXMLIDs();
		for( std::set< std::string >::iterator clistiter = clist.begin();
			 clistiter != clist.end(); ++clistiter )
		{
			std::string xmlid = *clistiter;
			UT_DEBUGMSG(("rdfAnchorSelectPos() xmlid:%s\n", xmlid.c_str() ));
						
			std::pair< PT_DocPosition, PT_DocPosition > range = rdf->getIDRange( xmlid );
			if( range.first && range.second > range.first )
			{
				UT_DEBUGMSG(("rdfAnchorSelectPos() has range...\n" ));
				if( range.first <= pos && pos <= range.second )
				{
					UT_DEBUGMSG(("rdfAnchorSelectPos() contains point...\n" ));
					setSemanticItemRing( rdf, c, clist, xmlid );
					
					// ring.h = c;
					// ring.xmlids = clist;
					// for( std::set< std::string >::iterator ri = ring.xmlids.begin();
					// 	 ri != ring.xmlids.end(); )
					// {
					// 	std::set< std::string >::iterator t = ri;
					// 	++ri;
					// 	std::pair< PT_DocPosition, PT_DocPosition > range = rdf->getIDRange( *t );
					// 	if( !range.first || range.second <= range.first )
					// 		ring.xmlids.erase( t );
					// }
					// ring.iter   = ring.xmlids.find( xmlid );

					if( selectit )
						pView->selectRange( range );
					return;
				}
			}
		}
	}
	UT_DEBUGMSG(("rdfAnchorSelectPos() FAILED...\n" ));
}

//
// If they have started at another sem item, adapt to their choice
// Note that this can alter the ring.iter
//
static bool rdfAnchorContainsPoint( FV_View* pView,
									PD_DocumentRDFHandle rdf,
									PT_DocPosition pos )
{
	selectReferenceToSemanticItemRing& ring = getSelectReferenceToSemanticItemRing();

	std::set< std::string > xmlids;
	rdf->addRelevantIDsForPosition( xmlids, pos );
	std::set< std::string > tmp;
	std::set_intersection( xmlids.begin(), xmlids.end(),
						   ring.xmlids.begin(), ring.xmlids.end(),
						   std::inserter( tmp, tmp.end() ));
	UT_DEBUGMSG(("rdfAnchorContainsPoint() pos:%ld xmlids.sz:%ld tmp.sz:%ld\n",
				 (long)pos, (long)xmlids.size(), (long)tmp.size() ));
	if( tmp.empty() )
	{
		//
		// nothing for the cursor position is in the cached ring state!
		//
		UT_DEBUGMSG(("rdfAnchorContainsPoint() resyncing\n"));
		rdfAnchorSelectPos( pView, rdf, pos, false );
		return false;
	}
	return true;
}

	
Defun1(rdfAnchorSelectThisReferenceToSemanticItem)
{
	selectReferenceToSemanticItemRing& ring = getSelectReferenceToSemanticItemRing();
	ring.h.reset();
	ring.xmlids.clear();
	ring.iter = ring.xmlids.end();
	
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	if( PD_Document * pDoc = pView->getDocument() )
	{
		if( PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF() )
		{
			rdfAnchorSelectPos( pView, rdf, pView->getPoint(), true );
		}
	}
	return 0;
}

Defun1(rdfAnchorSelectNextReferenceToSemanticItem)
{
	selectReferenceToSemanticItemRing& ring = getSelectReferenceToSemanticItemRing();
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	if( PD_Document * pDoc = pView->getDocument() )
	{
		if( PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF() )
		{
			// If they have started at another sem item, adapt to their choice
			bool wasContained = rdfAnchorContainsPoint( pView, rdf, pView->getPoint()-1 );

			if( ring.iter == ring.xmlids.end() )
			{
				UT_DEBUGMSG((" selectNext(1) iter == end\n" ));
				return 0;
			}
			ring.iter++;

			UT_DEBUGMSG((" selectNext(2) iter == end:%d wasc:%d\n", ring.iter == ring.xmlids.end(), wasContained));
			// the iter was resyned and there is no next.
			if( ring.iter == ring.xmlids.end() && !wasContained )
				ring.iter--;

			if( ring.iter != ring.xmlids.end() )
			{
				std::string xmlid = *ring.iter;
				UT_DEBUGMSG((" selectNext() xmlid:%s\n", xmlid.c_str() ));
				std::pair< PT_DocPosition, PT_DocPosition > range = rdf->getIDRange( xmlid );
				if( range.first && range.second > range.first )
					pView->selectRange( range );
			}
		}
	}
	
	return 0;
}

Defun1(rdfAnchorSelectPrevReferenceToSemanticItem)
{
	selectReferenceToSemanticItemRing& ring = getSelectReferenceToSemanticItemRing();
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	if( PD_Document * pDoc = pView->getDocument() )
	{
		if( PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF() )
		{
			bool wasContained = rdfAnchorContainsPoint( pView, rdf, pView->getPoint()-1 );
	
			if( ring.iter == ring.xmlids.begin() )
			{
				UT_DEBUGMSG((" selectPrev() resetting iter to end()\n" ));
				ring.iter = ring.xmlids.end();
			}
			if( ring.iter == ring.xmlids.end() )
			{
				UT_DEBUGMSG((" selectPrev() iter IS end()\n" ));
				if( wasContained )
					return 0;
				
				// if we resynced, and there is no prev, then select the first one.
				UT_DEBUGMSG((" selectPrev() set iter to the first item due to resync...\n" ));
				ring.iter = ring.xmlids.begin();
				ring.iter++;
			}
	
			ring.iter--;
	
			std::string xmlid = *ring.iter;
			UT_DEBUGMSG((" selectPrev() xmlid:%s\n", xmlid.c_str() ));
			
			std::pair< PT_DocPosition, PT_DocPosition > range = rdf->getIDRange( xmlid );
			if( range.first && range.second > range.first )
				pView->selectRange( range );
		}
	}
	
	return 0;
}


static PD_RDFSemanticItemHandle& getrdfSemitemSource()
{
	static PD_RDFSemanticItemHandle ret;
	return ret;
}

Defun1(rdfSemitemSetAsSource)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PD_Document * pDoc = pView->getDocument();
	PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();
	std::set< std::string > xmlids;
	rdf->addRelevantIDsForPosition( xmlids, pView->getPoint() );
    PD_RDFSemanticItems sl = rdf->getSemanticObjects( xmlids );
	if( sl.empty() )
		return false;
	
	PD_RDFSemanticItemHandle si = *(sl.begin());
	getrdfSemitemSource() = si;
	return true;
}

Defun1(rdfSemitemFindRelatedFoafKnows)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PD_Document * pDoc = pView->getDocument();
	PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();

	std::set< std::string > xmlids;
	rdf->addRelevantIDsForPosition( xmlids, pView->getPoint() );
	UT_DEBUGMSG(("rdfSemitemFindRelatedFoafKnows(a) point->xmlids.sz:%ld\n", (long)xmlids.size() ));
	if( xmlids.empty() )
		rdf->addRelevantIDsForPosition( xmlids, pView->getPoint()-1 );
		
    PD_RDFSemanticItems sl = rdf->getSemanticObjects( xmlids );
	if( sl.empty() )
		return false;
	PD_RDFSemanticItemHandle src = *(sl.begin());
	UT_DEBUGMSG(("rdfSemitemFindRelatedFoafKnows(b) point->xmlids.sz:%ld\n", (long)xmlids.size() ));
	UT_DEBUGMSG(("rdfSemitemFindRelatedFoafKnows() point->sl.sz:%ld\n", (long)sl.size() ));
	for( PD_RDFSemanticItems::iterator iter = sl.begin(); iter != sl.end(); ++iter )
	{
		PD_RDFSemanticItemHandle si = *iter;
		UT_DEBUGMSG(("rdfSemitemFindRelatedFoafKnows() point->si:%s\n", si->name().c_str() ));
	}
	
	PD_RDFSemanticItems related = src->relationFind( PD_RDFSemanticItem::RELATION_FOAF_KNOWS );
	for( PD_RDFSemanticItems::iterator iter = related.begin(); iter != related.end(); ++iter )
	{
		PD_RDFSemanticItemHandle si = *iter;
		xmlids = si->getXMLIDs();
		for( std::set< std::string >::iterator xi = xmlids.begin(); xi != xmlids.end(); ++xi )
		{
			std::string xmlid = *xi;
			std::pair< PT_DocPosition, PT_DocPosition > p = rdf->getIDRange( xmlid );
			if( p.first && p.first != p.second )
			{
				setSemanticItemRing( rdf, si, xmlids, xmlid );
				PD_RDFSemanticItemViewSite vs( si, xmlid );
				vs.select( pView );
				return true;
			}
		}
	}
	
	return true;
}

Defun1(rdfSemitemRelatedToSourceFoafKnows)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PD_Document * pDoc = pView->getDocument();
	PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();
	PD_RDFSemanticItemHandle src = getrdfSemitemSource();
	std::set< std::string > xmlids;
	rdf->addRelevantIDsForPosition( xmlids, pView->getPoint() );
    PD_RDFSemanticItems sl = rdf->getSemanticObjects( xmlids );
	if( sl.empty() )
		return false;

	for( PD_RDFSemanticItems::iterator iter = sl.begin(); iter != sl.end(); ++iter )
	{
		PD_RDFSemanticItemHandle si = *iter;
		src->relationAdd( si, PD_RDFSemanticItem::RELATION_FOAF_KNOWS );
	}
	
	return true;
}

Defun1(rdfApplyCurrentStyleSheet)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PD_Document * pDoc = pView->getDocument();
	PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();

	std::set< std::string > xmlids;
	rdf->addRelevantIDsForPosition( xmlids, pView->getPoint() );
    PD_RDFSemanticItems sl = rdf->getSemanticObjects( xmlids );
	for( PD_RDFSemanticItems::iterator iter = sl.begin(); iter != sl.end(); ++iter )
	{
		PD_RDFSemanticItemHandle si = *iter;
		PD_RDFSemanticItemViewSite vs( si, pView->getPoint() );
		vs.reflowUsingCurrentStylesheet( pView );
	}	
	return true;
}

Defun1(rdfStylesheetSettings)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PD_Document * pDoc = pView->getDocument();
	PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();

	runSemanticStylesheetsDialog( pView );
	
	return true;
}

Defun1(rdfDisassocateCurrentStyleSheet)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PD_Document * pDoc = pView->getDocument();
	PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();

	std::set< std::string > xmlids;
	rdf->addRelevantIDsForPosition( xmlids, pView->getPoint() );
    PD_RDFSemanticItems sl = rdf->getSemanticObjects( xmlids );
	for( PD_RDFSemanticItems::iterator iter = sl.begin(); iter != sl.end(); ++iter )
	{
		PD_RDFSemanticItemHandle si = *iter;
		PD_RDFSemanticItemViewSite vs( si, pView->getPoint() );
		vs.disassociateStylesheet();
		vs.reflowUsingCurrentStylesheet( pView );
	}	
	return true;
}

static void _rdfApplyStylesheet( FV_View* pView, std::string stylesheetName, PT_DocPosition pos )
{
	PD_Document * pDoc = pView->getDocument();
	PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();

	std::set< std::string > xmlids;
	rdf->addRelevantIDsForPosition( xmlids, pos );
    PD_RDFSemanticItems sl = rdf->getSemanticObjects( xmlids );
	if( sl.empty() )
		return;

	for( PD_RDFSemanticItems::iterator iter = sl.begin(); iter != sl.end(); ++iter )
	{
		PD_RDFSemanticItemHandle si = *iter;
		PD_RDFSemanticStylesheetHandle ss =
			si->findStylesheetByName( PD_RDFSemanticStylesheet::stylesheetTypeSystem(),
									  stylesheetName );
		if( !ss )
			continue;
		
		PD_RDFSemanticItemViewSite vs( si, pos );
		vs.applyStylesheet( pView, ss );
		return;
	}	
}


Defun1(rdfApplyStylesheetContactName)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	_rdfApplyStylesheet( pView, RDF_SEMANTIC_STYLESHEET_CONTACT_NAME, pView->getPoint() );
	return true;
}

Defun1(rdfApplyStylesheetContactNameHomepagePhone)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	_rdfApplyStylesheet( pView, RDF_SEMANTIC_STYLESHEET_CONTACT_NAME_HOMEPAGE_PHONE, pView->getPoint() );
	return true;
}

Defun1(rdfApplyStylesheetContactNamePhone)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	_rdfApplyStylesheet( pView, RDF_SEMANTIC_STYLESHEET_CONTACT_NAME_PHONE, pView->getPoint() );
	return true;
}

Defun1(rdfApplyStylesheetContactNick)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	_rdfApplyStylesheet( pView, RDF_SEMANTIC_STYLESHEET_CONTACT_NICK, pView->getPoint() );
	return true;
}

Defun1(rdfApplyStylesheetContactNickPhone)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	_rdfApplyStylesheet( pView, RDF_SEMANTIC_STYLESHEET_CONTACT_NICK_PHONE, pView->getPoint() );
	return true;
}

Defun1(rdfApplyStylesheetEventName)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	_rdfApplyStylesheet( pView, RDF_SEMANTIC_STYLESHEET_EVENT_NAME, pView->getPoint() );
	return true;
}

Defun1(rdfApplyStylesheetEventSummary)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	_rdfApplyStylesheet( pView, RDF_SEMANTIC_STYLESHEET_EVENT_SUMMARY, pView->getPoint() );
	return true;
}

Defun1(rdfApplyStylesheetEventSummaryLocation)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	_rdfApplyStylesheet( pView, RDF_SEMANTIC_STYLESHEET_EVENT_SUMMARY_LOCATION, pView->getPoint() );
	return true;
}

Defun1(rdfApplyStylesheetEventSummaryLocationTimes)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	_rdfApplyStylesheet( pView, RDF_SEMANTIC_STYLESHEET_EVENT_SUMMARY_LOCATION_TIMES, pView->getPoint() );
	return true;
}

Defun1(rdfApplyStylesheetEventSummaryTimes)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	_rdfApplyStylesheet( pView, RDF_SEMANTIC_STYLESHEET_EVENT_SUMMARY_TIMES, pView->getPoint() );
	return true;
}

Defun1(rdfApplyStylesheetLocationLatLong)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	_rdfApplyStylesheet( pView, RDF_SEMANTIC_STYLESHEET_LOCATION_NAME_LATLONG, pView->getPoint() );
	return true;
}

Defun1(rdfApplyStylesheetLocationName)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	_rdfApplyStylesheet( pView, RDF_SEMANTIC_STYLESHEET_LOCATION_NAME, pView->getPoint() );
	return true;
}

Defun1(deleteHyperlink)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	pView->cmdDeleteHyperlink();
	return true;
}

Defun(hyperlinkStatusBar)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);

	if( pView->bubblesAreBlocked() )
	{
		UT_DEBUGMSG(("hyperlinkStatusBar() bubbles are blocked, not opening one right now\n" ));
		return true;	
	}
	
	GR_Graphics * pG = pView->getGraphics();
	if (pG)
		pG->setCursor(GR_Graphics::GR_CURSOR_LINK);

	UT_sint32 xpos = pCallData->m_xPos;
	UT_sint32 ypos = pCallData->m_yPos;
	PT_DocPosition  pos = pView->getDocPositionFromXY(xpos,ypos);
	fp_HyperlinkRun * pHRun = static_cast<fp_HyperlinkRun *>(pView->getHyperLinkRun(pos));
	if(!pHRun)
		return false;
	UT_DEBUGMSG(("hyperlinkStatusBar() pHRun:%p\n", pHRun ));
	UT_DEBUGMSG(("hyperlinkStatusBar()  type:%d\n", (int)pHRun->getHyperlinkType() ));
	if(pHRun->getHyperlinkType() == HYPERLINK_NORMAL)
	{
			pView->cmdHyperlinkStatusBar(xpos, ypos);
			return true;
	}

	UT_uint32 pid = 0;
	std::string sText;
	if( fp_AnnotationRun * pAnn = dynamic_cast<fp_AnnotationRun *>(pHRun) )
	{
		UT_DEBUGMSG(("hyperlinkStatusBar() have annotation...\n" ));
		pid = pAnn->getPID();
		UT_DebugOnly<bool> b = pView->getAnnotationText( pid, sText );
		UT_ASSERT(b);
	}
	else if( fp_RDFAnchorRun * pAnchorRun = dynamic_cast<fp_RDFAnchorRun *>(pHRun) )
	{
		UT_DEBUGMSG(("hyperlinkStatusBar() have RDF anchor!\n" ));
		pid = pAnchorRun->getPID();
		std::string xmlid = pAnchorRun->getXMLID();
		std::stringstream ss;
		ss << "xmlid:" << xmlid;
		if( PD_Document * pDoc = pView->getDocument() )
		{
			if( PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF() )
			{
				PD_RDFModelHandle m = rdf->getRDFForID( xmlid );
				ss << " triple count:" << m->getTripleCount();
#if DEBUG
				std::pair< PT_DocPosition, PT_DocPosition > range = rdf->getIDRange( xmlid );
				ss << " start:" << range.first << " end:" << range.second;
#endif
			}
		}
		ss << " ";
		sText = ss.str();
	}
	
	// avoid unneeded redrawings
	// check BOTH if we are already previewing an annotation, and that it is indeed the annotation we want
	if((pView->isAnnotationPreviewActive()) &&
	   (pView->getActivePreviewAnnotationID() == pid ))
	{
		xxx_UT_DEBUGMSG(("hyperlinkStatusBar: nothing to draw, annotation already previewed\n"));
		return true; // should be false? think not
	}
	
	// kill previous preview if needed (it is not the same annotation as it would have been detected above)
	if (pView->isAnnotationPreviewActive())
	{
		UT_DEBUGMSG(("hyperlinkStatusBar: Deleting previous annotation preview...\n"));
		pView->killAnnotationPreview();
	}
	
	std::string sTitle;
	std::string sAuthor;
	if(pHRun->getHyperlinkType() == HYPERLINK_ANNOTATION && sText.empty() )
	{
		UT_DEBUGMSG(("hyperlinkStatusBar: exiting because we have no annotation text for pid:%d\n", pid));
		return false;
	}
	
	// Optional fields
	pView->getAnnotationTitle( pid, sTitle );
	pView->getAnnotationAuthor( pid, sAuthor );
	
	// preview annotation

	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
	UT_return_val_if_fail(pFrame, false);
	
	// PLEASE DOOOON'T UNCOMMENT THIS EVIL LINE (unexpectedly will hide the pop-up)
	//pFrame->raise();

	// Annotation Preview windows are per frame!

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Preview_Annotation * pAnnPview
		= static_cast<AP_Preview_Annotation *>(pDialogFactory->requestDialog(AP_DIALOG_ID_ANNOTATION_PREVIEW));

	if(!pAnnPview)
		return false;
		
	UT_DEBUGMSG(("hyperlinkStatusBar: Previewing annotation text %s \n",sText.c_str()));
	
	// flags
	pView->setAnnotationPreviewActive(true);
	// this call is also needed to decide when to redraw the preview
	pView->setActivePreviewAnnotationID( pid ); 
	
	// Fields
	pAnnPview->setDescription(sText);
	
	// Optional fields
	// if those fields are to be hidden it should be at the GUI level (inside AP_Preview_Annotation)
	pAnnPview->setTitle(sTitle);	
	pAnnPview->setAuthor(sAuthor);
	
	fp_Line * pLine = pHRun->getLine();
	if(pLine)
	{
		UT_Rect * pRect = pLine->getScreenRect();
		if(pRect)
		{
			UT_sint32 ioff = pRect->top;
		    pAnnPview->setOffset(pG->tdu(ypos - ioff));
		}
		delete pRect;
	}
	pAnnPview->setXY(pG->tdu(xpos),pG->tdu(ypos));
	pAnnPview->runModeless(pFrame);
	
	//UT_sint32 xoff = 0, yoff = 0;
	//fp_Run * pRun = pView->getHyperLinkRun(pos);
	//pHRun->getLine()->getOffsets(pHRun, xoff, yoff); //TODO try getting container's screen offset... ->getContainer()
	// Sevior's infamous + 1....
	//yoff += pHRun->getLine()->getAscent() - pHRun->getAscent() + 1;
	UT_DEBUGMSG(("hyperlinkStatusBar: xypos %d %d\n",xpos,ypos));
	UT_DEBUGMSG(("hyperlinkStatusBar: setXY %d %d\n",pG->tdu(xpos),pG->tdu(ypos)));
	//UT_DEBUGMSG(("hyperlinkStatusBar: pRungetxy %d %d\n",pHRun->getX(),pHRun->getY()));
	//UT_DEBUGMSG(("hyperlinkStatusBar: getScreenOffsets %d %d\n",xoff,yoff));
	
	pAnnPview->draw();
	
	return true;	
}

static bool s_doMarkRevisions(XAP_Frame * pFrame, PD_Document * pDoc, FV_View * pView,
							  bool bToggleMark, bool bForceNew)
{
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_MarkRevisions * pDialog
		= static_cast<AP_Dialog_MarkRevisions *>(pDialogFactory->requestDialog(AP_DIALOG_ID_MARK_REVISIONS));
UT_return_val_if_fail(pDialog, false);
	pDialog->setDocument(pDoc);

	if(bForceNew)
		pDialog->forceNew();
	
	pDialog->runModal(pFrame);
	bool bOK = (pDialog->getAnswer() == AP_Dialog_MarkRevisions::a_OK);

	if (!bOK && bToggleMark)
	{
		// we have already turned this on, so turn it off again
		pView->toggleMarkRevisions();
	}
	else if(bOK)
	{
		pDialog->addRevision();
#if 0
		// cannot remember at all why I thought this was needed and it has been marked as
		// bug 7700, so I am going to disable this. Tomas, May 10, 2005
		
		// we also want to have paragraph marks and etc visible
		AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
		UT_return_val_if_fail(pFrameData, false);
		if(!pFrameData->m_bShowPara)
		{
			pFrameData->m_bShowPara = true;
			pView->setShowPara(true);
			pView->notifyListeners(AV_CHG_FRAMEDATA);	// to update toolbar
		}
#endif
	}


	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

Defun1(purgeAllRevisions)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc,false);

	//  turn revisions off
	return pDoc->purgeAllRevisions(pView);
}

Defun1(toggleAutoRevision)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc,false);
	
	bool bAuto = !pDoc->isAutoRevisioning();
	bool bDoIT = true;
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame,false);
	if(!bAuto)
	{
		// the user asked to turn revisioning off; this would disrupt
		// the record of changes, making it impossible to revert
		// reliably to any earlier versions of document history
		// we issue worning
		
		bDoIT = (XAP_Dialog_MessageBox::a_YES ==
				        pFrame->showMessageBox(AP_STRING_ID_MSG_AutoRevisionOffWarning, 
											   XAP_Dialog_MessageBox::b_YN, 
											   XAP_Dialog_MessageBox::a_NO));
	
	}
	if(bDoIT)
	{
//
// Get rid of the warning box before the redraw
//
		UT_sint32 i =0;
		for(i=0; i< 5;i++)
		{
			pFrame->nullUpdate();
		}
		pDoc->setAutoRevisioning(bAuto);
		pView->focusChange(AV_FOCUS_HERE);
	}
	return true;
}

Defun1(toggleMarkRevisions)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);

	if(!pView->isMarkRevisions())
	{
		// set view level to all
		pView->setRevisionLevel(0);
	}
	
	if(!pView->isMarkRevisions())
	{
		PD_Document * pDoc = pView->getDocument();
		XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
		UT_return_val_if_fail( pFrame && pDoc, false );
		
		if(s_doMarkRevisions(pFrame, pDoc, pView, false, false))
			pView->toggleMarkRevisions();
	}
	else
	{
		pView->toggleMarkRevisions();
	}
	


	
	return true;
}

Defun1(startNewRevision)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);

	if(!pView->isMarkRevisions())
	{
		// only do this when marking revisions is on
		return false;
	}

	PD_Document * pDoc = pView->getDocument();
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail( pDoc && pFrame, false );
	
	s_doMarkRevisions(pFrame, pDoc, pView, false, true);
	return true;
}

Defun1(toggleShowRevisions)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	pView->toggleShowRevisions();
	return true;
}

Defun1(toggleShowRevisionsBefore)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	bool bShow = pView->isShowRevisions();
	UT_uint32 iLevel = pView->getRevisionLevel();
	
	if(bShow)
	{
		//we are asked to hide revisions, first set view level to 0
		pView->setRevisionLevel(0);
		pView->toggleShowRevisions();
	}
	else if(iLevel != 0)
	{
		// we are asked to change view level
		pView->cmdSetRevisionLevel(0);
	}
	
	return true;
}

Defun1(toggleShowRevisionsAfter)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	bool bShow = pView->isShowRevisions();
	bool bMark = pView->isMarkRevisions();
	UT_uint32 iLevel = pView->getRevisionLevel();

	if(bMark)
	{
		if(iLevel != PD_MAX_REVISION)
		{
			pView->cmdSetRevisionLevel(PD_MAX_REVISION);
		}
		else
		{
			pView->cmdSetRevisionLevel(0);
		}
	}
	else if(bShow)
	{
		//we are asked to hide revisions, first set view level to max
		pView->setRevisionLevel(PD_MAX_REVISION);
		pView->toggleShowRevisions();
	}
	else if(iLevel != PD_MAX_REVISION)
	{
		// we are asked to change view level
		pView->cmdSetRevisionLevel(PD_MAX_REVISION);
	}
	
	return true;
}

Defun1(toggleShowRevisionsAfterPrevious)
{
	CHECK_FRAME;
	ABIWORD_VIEW;

	UT_return_val_if_fail(pView,false);
	UT_uint32 iLevel = pView->getRevisionLevel();
	UT_uint32 iDocLevel = pView->getDocument()->getHighestRevisionId();

	if(iDocLevel == 0)
		return false;
	
	if(iLevel != iDocLevel - 1)
	{
		// we are in Mark mode and are asked to treat all revisions
		// but the present as accepted
		pView->cmdSetRevisionLevel(iDocLevel-1);
	}
	else
	{
		pView->cmdSetRevisionLevel(0);
	}
	return true;
}

Defun(revisionAccept)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	pView->cmdAcceptRejectRevision(false, pCallData->m_xPos, pCallData->m_yPos);
	return true;
}

Defun(revisionReject)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	pView->cmdAcceptRejectRevision(true, pCallData->m_xPos, pCallData->m_yPos);
	return true;
}

Defun(revisionFindNext)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	pView->cmdFindRevision(true, pCallData->m_xPos, pCallData->m_yPos);
	return true;
}

Defun(revisionFindPrev)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	pView->cmdFindRevision(false, pCallData->m_xPos, pCallData->m_yPos);
	return true;
}

static bool s_doListRevisions(XAP_Frame * pFrame, PD_Document * pDoc, FV_View * pView)
{
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	AP_Dialog_ListRevisions * pDialog
		= static_cast<AP_Dialog_ListRevisions *>(pDialogFactory->requestDialog(AP_DIALOG_ID_LIST_REVISIONS));
UT_return_val_if_fail(pDialog, false);
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
	UT_return_val_if_fail(pView,false);
	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc,false);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame,false);

	s_doListRevisions(pFrame, pDoc, pView);

	return true;
}

Defun(revisionNew)
{
	UT_UNUSED(pCallData);
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);

	PD_Document * pDoc = pView->getDocument();
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail( pDoc && pFrame, false );

	s_doMarkRevisions(pFrame, pDoc, pView, false, true);
	pDoc->setMarkRevisions( true );
	
	return true;
}

Defun(revisionSelect)
{
	UT_UNUSED(pCallData);
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);

	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc,false);

	pDoc->setMarkRevisions( false );
	pView->setShowRevisions( true );
	
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame,false);

	s_doListRevisions(pFrame, pDoc, pView);

	return true;
}

Defun1(history)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	AD_Document * pDoc = (AD_Document *) pView->getDocument();
	UT_return_val_if_fail(pDoc,false);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame,false);

	return pDoc->showHistory(pView);
}

/*!
    This function can be used to raise one of the ListDocuments dialogues
    \param pFrame: the active frame
    \param bExcludeCurrent: true if current document is to be excluded
                            from the list
    \param iId: the dialogue iId determining which of the
                ListDocuments variants to raise

    \return: returns pointer to the document user selected or NULL
*/
static PD_Document * s_doListDocuments(XAP_Frame * pFrame, bool bExcludeCurrent, UT_uint32 iId)
{
	UT_return_val_if_fail(pFrame, NULL);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_ListDocuments * pDialog
		= static_cast<XAP_Dialog_ListDocuments *>(pDialogFactory->requestDialog(iId));
	
	UT_return_val_if_fail(pDialog, NULL);

	// the dialgue excludes current document by default, if we are to
	// include it, we need to tell it ...
	if(!bExcludeCurrent)
		pDialog->setIncludeActiveDoc(true);
	
	pDialog->runModal(pFrame);
	bool bOK = (pDialog->getAnswer() == XAP_Dialog_ListDocuments::a_OK);

	PD_Document *pD = NULL;
	
	if (bOK)
	{
		pD = (PD_Document *)pDialog->getDocument();
#if DEBUG
		if(!pD)
			UT_DEBUGMSG(("DIALOG LIST DOCUMENTS: no document\n"));
		else
			UT_DEBUGMSG(("DIALOG LIST DOCUMENTS: %s\n",
						 pD->getFilename().c_str()));
#endif
	}

	pDialogFactory->releaseDialog(pDialog);

	return pD;
}

Defun1(revisionCompareDocuments)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView,false);
	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc,false);

	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pAV_View->getParentData());
	UT_return_val_if_fail(pFrame,false);

	PD_Document * pDoc2 = s_doListDocuments(pFrame, true, XAP_DIALOG_ID_COMPAREDOCUMENTS);

	if(pDoc2)
	{
		pFrame->raise();

		XAP_DialogFactory * pDialogFactory
			= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

		XAP_Dialog_DocComparison * pDialog
			= static_cast<XAP_Dialog_DocComparison *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_DOCCOMPARISON));
	
		UT_return_val_if_fail(pDialog, false);

		pDialog->calculate(pDoc, pDoc2);
		pDialog->runModal(pFrame);
		pDialogFactory->releaseDialog(pDialog);
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

	UT_return_val_if_fail(pView, false);
	AP_TopRuler * pTopRuler = pView->getTopRuler();

	if(pTopRuler == NULL)
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
		UT_return_val_if_fail( pFrame, true );
		
		pTopRuler = new AP_TopRuler(pFrame);
		AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
		pFrameData->m_pTopRuler = pTopRuler;
		pView->setTopRuler(pTopRuler);
		pTopRuler->setViewHidden(pView);
	}
	if(pTopRuler->getView() == NULL)
	{
		return true;
	}

	pView->setDragTableLine(true);
	UT_sint32 x = pCallData->m_xPos;
	UT_sint32 y = pCallData->m_yPos;
	PT_DocPosition pos = pView->getDocPositionFromXY(x, y);
	xxx_UT_DEBUGMSG(("ap_EditMethods.cpp:: VDrag begin \n"));
	
	sTopRulerHeight = pTopRuler ? pTopRuler->setTableLineDrag(pos,x,siFixed) : 0;
	pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_GRAB);
	return true;
}

Defun(beginHDrag)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	AP_LeftRuler * pLeftRuler = pView->getLeftRuler();

	if(pLeftRuler == NULL)
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());

		pLeftRuler = new AP_LeftRuler(pFrame);
		AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
		pFrameData->m_pLeftRuler = pLeftRuler;
		pView->setLeftRuler(pLeftRuler);
		pLeftRuler->setViewHidden(pView);
	}

	pView->setDragTableLine(true);
	UT_sint32 x = pCallData->m_xPos;
	UT_sint32 y = pCallData->m_yPos;
	PT_DocPosition pos = pView->getDocPositionFromXY(x, y);
	sLeftRulerPos = pLeftRuler->setTableLineDrag(pos,siFixed,y);
	pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_GRAB);

	return true;
}

Defun1(clearSetCols)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	bool bres = pView->cmdAutoSizeCols();
	pView->setDragTableLine(false);
	return bres;
}


Defun1(autoFitTable)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	bool bres = pView->cmdAutoFitTable();
	return bres;
}

Defun1(clearSetRows)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	bool bres = pView->cmdAutoSizeRows();
	pView->setDragTableLine(false);
	return bres;
}

Defun(dragVline)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Doing Vertical Line drag \n"));

	UT_return_val_if_fail(pView, false);
	AP_TopRuler * pTopRuler = pView->getTopRuler();
	if(pTopRuler == NULL)
	{
		return true;
	}
	if(pTopRuler->getView() == NULL)
	{
		pTopRuler->setViewHidden(pView);
	}
	UT_sint32 x = pCallData->m_xPos + siFixed;
	pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_GRAB);
	EV_EditModifierState ems = 0; 
	xxx_UT_DEBUGMSG(("ap_EditMethods.cpp:: DRagging VLine \n"));
	pTopRuler->mouseMotion(ems, x, sTopRulerHeight);
	return true;
}

Defun(dragHline)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Doing Hline Line drag \n"));
	UT_return_val_if_fail(pView, false);
	AP_LeftRuler * pLeftRuler = pView->getLeftRuler();
	if(!pLeftRuler)
	{
		return true;
	}
	if(pLeftRuler->getView() == NULL)
	{
		pLeftRuler->setViewHidden(pView);
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

	UT_return_val_if_fail(pView, false);
	AP_TopRuler * pTopRuler = pView->getTopRuler();
	if(!pTopRuler)
	{
		return true;
	}
	if(pTopRuler->getView() == NULL)
	{
		pTopRuler->setView(pView);
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
	UT_return_val_if_fail(pView, false);
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


Defun(btn0InlineImage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	xxx_UT_DEBUGMSG(("Hover on Inline Image \n"));
	UT_return_val_if_fail(pView, false);
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
	pView->btn0InlineImage(x,y);
	return true;
}


Defun(btn1InlineImage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	UT_DEBUGMSG(("Click on InlineImage \n"));
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
	pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_GRAB);
	if(pView->getMouseContext(x,y) == EV_EMC_IMAGESIZE)
	{
	     PT_DocPosition pos = pView->getDocPositionFromXY(pCallData->m_xPos, pCallData->m_yPos);
	     fl_BlockLayout * pBlock = pView->getBlockAtPosition(pos);
	     if(pBlock)
	     {
		  UT_sint32 x1,x2,y1,y2,iHeight;
		  bool bEOL = false;
		  bool bDir = false;
		
		  fp_Run * pRun = NULL;
		
		  pRun = pBlock->findPointCoords(pos,bEOL,x1,y1,x2,y2,iHeight,bDir);
		  while(pRun && ((pRun->getType() != FPRUN_IMAGE) && (pRun->getType() != FPRUN_EMBED)))
		  {
			pRun = pRun->getNextRun();
		  }
		  if(pRun && (pRun->getType() == FPRUN_EMBED))
		  {
			// we've found an embed object: do not move the view, just select the image and exit
		        pView->cmdSelect(pos,pos+1);
		  }
	     }
	}
	pView->btn1InlineImage(x,y);
	return true;
}


Defun(copyInlineImage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Copy InlineImage \n"));
	UT_return_val_if_fail(pView, false);
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
	pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_GRAB);
	pView->btn1CopyImage(x,y);
	return true;
}

static bool sReleaseInlineImage = false;

static void sActualDragInlineImage(AV_View *  pAV_View, EV_EditMethodCallData * pCallData)
{
	ABIWORD_VIEW;
	UT_return_if_fail(pView);
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
	//
	// This boolean is true if we had to drop aa release event because
	// a drag was pending so instead of doing a dragInlineImage we now
	// do the release inline image and return.
	//
	if(sReleaseInlineImage)
	{
		UT_DEBUGMSG(("Nested Release InlineImage call \n"));
		sReleaseInlineImage = false;
		pView->releaseInlineImage(x,y);
		return;
	}
	pView->dragInlineImage(x,y);

}

Defun(dragInlineImage)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	xxx_UT_DEBUGMSG(("Drag Inline Image \n"));
//
// Do this operation in an idle loop so when can reject queued events
//
//
// This code sets things up to handle the warp right in an idle loop.
//
	UT_return_val_if_fail(pView, false);
	int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	//int inMode = UT_WorkerFactory::TIMER;
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	EV_EditMethodCallData * pNewData = new  EV_EditMethodCallData(pCallData->m_pData,pCallData->m_dataLength);
	pNewData->m_xPos = pCallData->m_xPos;
	pNewData->m_yPos = pCallData->m_yPos;
	_Freq * pFreq = new _Freq(pView,pNewData,sActualDragInlineImage);
	s_pFrequentRepeat = UT_WorkerFactory::static_constructor (_sFrequentRepeat,pFreq, inMode, outMode);

	UT_ASSERT(s_pFrequentRepeat);
	UT_ASSERT(outMode != UT_WorkerFactory::NONE);

	// If the worker is working on a timer instead of in the idle
	// time, set the frequency of the checks.
	if ( UT_WorkerFactory::TIMER == outMode )
	{
		// this is really a timer, so it's safe to static_cast it
		static_cast<UT_Timer*>(s_pFrequentRepeat)->set(50);
	}
	s_pFrequentRepeat->start();
	return true;
}


Defun(releaseInlineImage)
{
	sReleaseInlineImage = true;
	//
	// If this release event occurs while the current image is had a drag
	// event pending process then we can up with a duplicated image.
	// The CHECK_FRAME below will return true if there is a pending
	// drag being processed. The flag above will be set true to handle
	// this case.
	//
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Release Inline Image \n"));
	UT_return_val_if_fail(pView, false);
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
	sReleaseInlineImage = false;
	pView->releaseInlineImage(x,y);
	return true;
}


Defun(btn0Frame)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	xxx_UT_DEBUGMSG(("Hover on Frame \n"));
	UT_return_val_if_fail(pView, false);
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
	pView->btn0Frame(x,y);
	return true;
}


Defun(btn1Frame)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Click on Frame \n"));
	UT_return_val_if_fail(pView, false);
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
	pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_GRAB);
	pView->btn1Frame(x,y);
	return true;
}

static bool sReleaseFrame = false;

static void sActualDragFrame(AV_View *  pAV_View, EV_EditMethodCallData * pCallData)
{
	ABIWORD_VIEW;
	UT_return_if_fail(pView);
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
	if(sReleaseFrame)
	{
		sReleaseFrame = false;
		pView->releaseFrame(x,y);
		return;
	}
	pView->dragFrame(x,y);

}

Defun(dragFrame)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Doing Drag Frame \n"));
//
// Do this operation in an idle loop so when can reject queued events
//
//
// This code sets things up to handle the warp right in an idle loop.
//
	UT_return_val_if_fail(pView, false);
	int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	EV_EditMethodCallData * pNewData = new  EV_EditMethodCallData(pCallData->m_pData,pCallData->m_dataLength);
	pNewData->m_xPos = pCallData->m_xPos;
	pNewData->m_yPos = pCallData->m_yPos;
	_Freq * pFreq = new _Freq(pView,pNewData,sActualDragFrame);
	s_pFrequentRepeat = UT_WorkerFactory::static_constructor (_sFrequentRepeat,pFreq, inMode, outMode);

	UT_ASSERT(s_pFrequentRepeat);
	UT_ASSERT(outMode != UT_WorkerFactory::NONE);

	// If the worker is working on a timer instead of in the idle
	// time, set the frequency of the checks.
	if ( UT_WorkerFactory::TIMER == outMode )
	{
		// this is really a timer, so it's safe to static_cast it
		UT_DEBUGMSG(("Set timer to 50 ms \n"));
		static_cast<UT_Timer*>(s_pFrequentRepeat)->set(50);
	}
	s_pFrequentRepeat->start();
	return true;
}


Defun(releaseFrame)
{
	sReleaseFrame = true;
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Release Frame \n"));
	UT_return_val_if_fail(pView, false);
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
	sReleaseFrame = false;
	pView->releaseFrame(x,y);
	return true;
}


Defun1(deleteFrame)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Delete Frame \n"));
	UT_return_val_if_fail(pView, false);
	pView->deleteFrame();
	return true;
}

Defun1(cutFrame)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	pView->copyFrame(false);
	return true;
}


Defun1(copyFrame)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Copy Frame \n"));
	UT_return_val_if_fail(pView, false);
	pView->copyFrame(true);
	return true;
}


Defun1(selectFrame)
{
	CHECK_FRAME;
	UT_DEBUGMSG(("Select Frame \n"));
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	pView->selectFrame();
	return true;
}

Defun1(dlgFormatFrame)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Format Frame \n"));
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();


	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_FormatFrame * pDialog
		= static_cast<AP_Dialog_FormatFrame *>(pDialogFactory->requestDialog(AP_DIALOG_ID_FORMAT_FRAME));
UT_return_val_if_fail(pDialog, false);
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

Defun(cutVisualText)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData()); 
	UT_DEBUGMSG(("Cut on Selection \n"));
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
	pView->cutVisualText(x,y);
	if(pView->getVisualText()->isNotdraggingImage())
	{
	  pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_DRAGTEXT);
	  pFrame->setCursor(GR_Graphics::GR_CURSOR_DRAGTEXT);
	  if(	pView->getVisualText()->isDoingCopy())
	  {
	    pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_COPYTEXT);
	    pFrame->setCursor(GR_Graphics::GR_CURSOR_COPYTEXT);
	  }
	}
	else
	{
	  pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGE);
	}
	return true;
}


Defun(copyVisualText)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData()); 
	xxx_UT_DEBUGMSG(("Copy on Selection \n"));
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
	pView->copyVisualText(x,y);
	if(pView->getVisualText()->isNotdraggingImage())
	{
	  pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_DRAGTEXT);
	  pFrame->setCursor(GR_Graphics::GR_CURSOR_DRAGTEXT);
	  if(	pView->getVisualText()->isDoingCopy())
	  {
	    pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_COPYTEXT);
	    pFrame->setCursor(GR_Graphics::GR_CURSOR_COPYTEXT);
	  }
	}
	else
	{
	  pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGE);
	}
	return true;
}

static bool sEndVisualDrag = false;

static void sActualVisualDrag(AV_View *  pAV_View, EV_EditMethodCallData * pCallData)
{
	ABIWORD_VIEW;
	UT_return_if_fail(pView);
	XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData()); 
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
	if(sEndVisualDrag)
	{
		sEndVisualDrag = false;
		pView->pasteVisualText(x,y);
		return;
	}
	if(pView->getVisualText()->isNotdraggingImage())
	{
	  pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_DRAGTEXT);
	  pFrame->setCursor(GR_Graphics::GR_CURSOR_DRAGTEXT);
	  if(	pView->getVisualText()->isDoingCopy())
	  {
	    pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_COPYTEXT);
	    pFrame->setCursor(GR_Graphics::GR_CURSOR_COPYTEXT);
	  }
	}
	else
	{
	  pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_IMAGE);
	}
	pView->dragVisualText(x,y);
}

Defun(dragVisualText)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	sEndVisualDrag = false;
	xxx_UT_DEBUGMSG(("Drag Visual Text \n"));
	UT_return_val_if_fail(pView, false);
	PT_DocPosition posLow = pView->getSelectionAnchor();
	PT_DocPosition posHigh = pView->getPoint();
	if(posLow > posHigh)
	{
	     PT_DocPosition pos = posLow;
	     posLow = posHigh;
	     posHigh = pos;
	}
	if((posLow + 1) == posHigh)
	{
	     fl_BlockLayout * pBL = pView->getCurrentBlock();
	     if((pBL->getPosition() >= posLow) && ((pBL->getPosition() + pBL->getLength()) > posHigh))
	     {
	       UT_sint32 x1,x2,y1,y2,height;
	       bool bEOL,bDir;
	       bEOL = false;
	       fp_Run * pRun = pBL->findPointCoords(posHigh,bEOL,x1,x2,y1,y2,height,bDir);
	       if(pRun->getType() == FPRUN_IMAGE)
	       {
			   FV_VisualDragText * pVis = pView->getVisualText();
			   pVis->abortDrag();
	       }
	     }
	}
//
// Do this operation in an idle loop so when can reject queued events
//
//
// This code sets things up to handle the warp right in an idle loop.
//
	int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	EV_EditMethodCallData * pNewData = new  EV_EditMethodCallData(pCallData->m_pData,pCallData->m_dataLength);
	pNewData->m_xPos = pCallData->m_xPos;
	pNewData->m_yPos = pCallData->m_yPos;
	_Freq * pFreq = new _Freq(pView,pNewData,sActualVisualDrag);
	s_pFrequentRepeat = UT_WorkerFactory::static_constructor (_sFrequentRepeat,pFreq, inMode, outMode);

	UT_ASSERT(s_pFrequentRepeat);
	UT_ASSERT(outMode != UT_WorkerFactory::NONE);

	// If the worker is working on a timer instead of in the idle
	// time, set the frequency of the checks.
	if ( UT_WorkerFactory::TIMER == outMode )
	{
		// this is really a timer, so it's safe to static_cast it
		static_cast<UT_Timer*>(s_pFrequentRepeat)->set(50);
	}
	s_pFrequentRepeat->start();
	return true;
}


Defun(pasteVisualText)
{
    sEndVisualDrag = true;
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Drop Visual Text \n"));
	UT_return_val_if_fail(pView, false);
	UT_sint32 y = pCallData->m_yPos;
	UT_sint32 x = pCallData->m_xPos;
    sEndVisualDrag = false;
	pView->pasteVisualText(x,y);
	return true;
}



Defun(btn0VisualText)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	xxx_UT_DEBUGMSG(("In Visual Text \n"));
	UT_return_val_if_fail(pView, false);
	pView->btn0VisualDrag(pCallData->m_xPos,pCallData->m_yPos);
	static_cast<AV_View *>(pView)->notifyListeners(AV_CHG_MOUSEPOS);
	return true;
}

Defun0(repeatThisRow)
{
	CHECK_FRAME;
//	ABIWORD_VIEW;
	return true;
}

Defun0(removeThisRowRepeat)
{
	CHECK_FRAME;
//	ABIWORD_VIEW;
	return true;
}

Defun1(tableToTextCommas)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	pView->cmdTableToText(pView->getPoint(),0);
	return true;
}


Defun1(tableToTextTabs)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	pView->cmdTableToText(pView->getPoint(),1);
	return true;
}

Defun1(tableToTextCommasTabs)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	pView->cmdTableToText(pView->getPoint(),2);
	return true;
}

Defun1(doEscape)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_DEBUGMSG(("Escape Pressed. \n"));
	UT_return_val_if_fail(pView, false);
	FV_VisualDragText * pVis = pView->getVisualText();
	if(pVis->isActive())
	{
	    pVis->abortDrag();
	    sEndVisualDrag = false;
	    return true;
	}
	return true;
}

#ifdef DEBUG
Defun1(dumpRDFForPoint)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc, false);

    UT_DEBUGMSG(("dumpRDFForPoint...\n"));
    if( pView )
    {
        PT_DocPosition curr = pView->getPoint();
        UT_DEBUGMSG(("dumpRDFForPoint...current position:%d\n", curr));
        PD_RDFModelHandle h = pDoc->getDocumentRDF()->getRDFAtPosition( curr );
		
    }
    
    return true;
}

Defun1(dumpRDFObjects)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc, false);

    UT_DEBUGMSG(("dumpRDFObjects...\n"));
    pDoc->getDocumentRDF()->dumpObjectMarkersFromDocument();
    return true;
}

Defun1(rdfTest)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc, false);

    UT_DEBUGMSG(("RDFTest... running ml2 test\n"));
    pDoc->getDocumentRDF()->runMilestone2Test();
    return true;
}

Defun1(rdfPlay)
{
	CHECK_FRAME;
	ABIWORD_VIEW;
	UT_return_val_if_fail(pView, false);
	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc, false);

    UT_DEBUGMSG(("RDFTest... running RDF play\n"));
    pDoc->getDocumentRDF()->runPlay();
    return true;
}
#endif
