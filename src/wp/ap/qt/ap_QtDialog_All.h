/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2012 Hubert Figuiere
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
** MORE THAN ONE TIME.  Each time you add an entry to the top-half
** of this file be sure to add a corresponding entry to the other
** half and be sure to add an entry to each of the other platforms.
******************************************************************
*****************************************************************/

#ifndef AP_QTDIALOG_ALL_H
#define AP_QTDIALOG_ALL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if 0 // TODO
#   include "xap_QtDlg_About.h"
#   include "xap_QtDlg_ClipArt.h"
#   include "xap_QtDlg_MessageBox.h"
#   include "xap_QtDlg_WindowMore.h"
#   include "xap_QtDlg_Zoom.h"
#   include "xap_QtDlg_Insert_Symbol.h"
#   include "xap_QtDlg_Language.h"
#   include "xap_QtDlg_PluginManager.h"
#   include "xap_QtDlg_FileOpenSaveAs.h"
#   include "xap_QtDlg_Encoding.h"
#   include "xap_QtDlg_HTMLOptions.h"
#   include "xap_QtDlg_Password.h"
#   include "xap_QtDlg_Image.h"
#   include "xap_QtDlg_ListDocuments.h"
#   include "xap_QtDlg_History.h"
#   include "xap_QtDlg_DocComparison.h"

#   include "ap_QtDialog_Replace.h"
#   include "ap_QtDialog_Break.h"
#   include "ap_QtDialog_InsertTable.h"
#   include "ap_QtDialog_Goto.h"
#   include "ap_QtDialog_PageNumbers.h"
#   include "ap_QtDialog_PageSetup.h"
#   include "ap_QtDialog_Paragraph.h"
#   include "ap_QtDialog_Options.h"
#ifdef ENABLE_SPELL
#   include "ap_QtDialog_Spell.h"
#endif
#   include "ap_QtDialog_Styles.h"
#   include "ap_QtDialog_Stylist.h"
#   include "ap_QtDialog_Tab.h"
#   include "ap_QtDialog_Insert_DateTime.h"
#   include "ap_QtDialog_WordCount.h"
#   include "ap_QtDialog_Field.h"
#   include "ap_QtDialog_Lists.h"
#   include "ap_QtDialog_Columns.h"
#   include "ap_QtDialog_Tab.h"
#   include "ap_QtDialog_ToggleCase.h"
#   include "ap_QtDialog_Background.h"
#   include "ap_QtDialog_New.h"
#   include "ap_QtDialog_HdrFtr.h"
#   include "ap_QtDialog_InsertBookmark.h"
#   include "ap_QtDialog_InsertHyperlink.h"
#   include "ap_QtDialog_InsertXMLID.h"
#   include "ap_QtDialog_MetaData.h"
#   include "ap_QtDialog_MarkRevisions.h"
#   include "ap_QtDialog_ListRevisions.h"
#   include "ap_QtDialog_Annotation.h"
#   include "ap_QtPreview_Annotation.h"
#   include "ap_QtDialog_MergeCells.h"
#   include "ap_QtDialog_SplitCells.h"
#   include "ap_QtDialog_FormatTable.h"
#   include "ap_QtDialog_FormatFrame.h"
#   include "ap_QtDialog_FormatFootnotes.h"
#   include "ap_QtDialog_FormatTOC.h"
#   include "ap_QtDialog_MailMerge.h"
#   include "ap_QtDialog_Latex.h"
#   include "ap_QtDialog_Border_Shading.h"
#ifdef ENABLE_PRINT
#   include "xap_QtDlg_Print.h"
#   include "xap_QtDlg_PrintPreview.h"
#endif

#include "ap_QtDialog_RDFQuery.h"
#include "ap_QtDialog_RDFEditor.h"


#include "xap_QtDlg_FontChooser.h"

#endif // TODO

#else

#if 0 // TODO implement
DeclareDialog(XAP_DIALOG_ID_FONT, XAP_QtDialog_FontChooser, FALSE)

DeclareDialog(XAP_DIALOG_ID_ABOUT, XAP_QtDialog_About, FALSE)
DeclareDialog(XAP_DIALOG_ID_CLIPART, XAP_QtDialog_ClipArt, FALSE)
DeclareDialog(XAP_DIALOG_ID_FILE_OPEN, XAP_QtDialog_FileOpenSaveAs, FALSE)
DeclareDialog(XAP_DIALOG_ID_FILE_SAVEAS, XAP_QtDialog_FileOpenSaveAs, FALSE)
DeclareDialog(XAP_DIALOG_ID_FILE_SAVE_IMAGE, XAP_QtDialog_FileOpenSaveAs, FALSE)
#ifdef ENABLE_PRINT
DeclareDialog(XAP_DIALOG_ID_PRINT, XAP_QtDialog_Print, FALSE)
DeclareDialog(XAP_DIALOG_ID_PRINTPREVIEW, XAP_QtDialog_PrintPreview, FALSE)
DeclareDialog(XAP_DIALOG_ID_PRINTTOFILE, XAP_QtDialog_FileOpenSaveAs, FALSE)
#endif
DeclareDialog(XAP_DIALOG_ID_MESSAGE_BOX, XAP_QtDialog_MessageBox, FALSE)
DeclareDialog(XAP_DIALOG_ID_RECORDTOFILE, XAP_QtDialog_FileOpenSaveAs, FALSE)
DeclareDialog(XAP_DIALOG_ID_REPLAYFROMFILE, XAP_QtDialog_FileOpenSaveAs, FALSE)
DeclareDialog(XAP_DIALOG_ID_LANGUAGE, XAP_QtDialog_Language, FALSE)
DeclareDialog(XAP_DIALOG_ID_WINDOWMORE, XAP_QtDialog_WindowMore, FALSE)
DeclareDialog(XAP_DIALOG_ID_ZOOM, XAP_QtDialog_Zoom, FALSE)
DeclareDialog(XAP_DIALOG_ID_INSERT_SYMBOL,  XAP_QtDialog_Insert_Symbol, 		FALSE)
DeclareDialog(XAP_DIALOG_ID_INSERT_PICTURE,	XAP_QtDialog_FileOpenSaveAs, 		FALSE)
DeclareDialog(XAP_DIALOG_ID_INSERTMATHML,	XAP_QtDialog_FileOpenSaveAs, 		FALSE)
DeclareDialog(XAP_DIALOG_ID_INSERTOBJECT,	XAP_QtDialog_FileOpenSaveAs, 		FALSE)
DeclareDialog(XAP_DIALOG_ID_PLUGIN_MANAGER, XAP_QtDialog_PluginManager, 		FALSE)
DeclareDialog(XAP_DIALOG_ID_ENCODING,       XAP_QtDialog_Encoding, 			FALSE)
DeclareDialog(XAP_DIALOG_ID_HTMLOPTIONS,    XAP_QtDialog_HTMLOptions, 		FALSE)
DeclareDialog(XAP_DIALOG_ID_FILE_IMPORT,    XAP_QtDialog_FileOpenSaveAs, 		FALSE)
DeclareDialog(XAP_DIALOG_ID_FILE_EXPORT,    XAP_QtDialog_FileOpenSaveAs, 		FALSE)
DeclareDialog(XAP_DIALOG_ID_INSERT_FILE,    XAP_QtDialog_FileOpenSaveAs, 		FALSE)
DeclareDialog(XAP_DIALOG_ID_PASSWORD,       XAP_QtDialog_Password, 			FALSE)
DeclareDialog(XAP_DIALOG_ID_IMAGE,          XAP_QtDialog_Image, 				FALSE)
DeclareDialog(XAP_DIALOG_ID_LISTDOCUMENTS,  XAP_QtDialog_ListDocuments, 		FALSE)
DeclareDialog(XAP_DIALOG_ID_COMPAREDOCUMENTS,XAP_QtDialog_ListDocuments, 		FALSE)
DeclareDialog(XAP_DIALOG_ID_MERGEDOCUMENTS, XAP_QtDialog_ListDocuments, 		FALSE)
DeclareDialog(XAP_DIALOG_ID_HISTORY,        XAP_QtDialog_History, 			FALSE)
DeclareDialog(XAP_DIALOG_ID_DOCCOMPARISON,  XAP_QtDialog_DocComparison, 		FALSE)

DeclareDialog(AP_DIALOG_ID_REPLACE,			AP_QtDialog_Replace, 				FALSE)
DeclareDialog(AP_DIALOG_ID_FIND,			AP_QtDialog_Replace, 				FALSE)
DeclareDialog(AP_DIALOG_ID_HDRFTR,          AP_QtDialog_HdrFtr, 				FALSE)
DeclareDialog(AP_DIALOG_ID_BACKGROUND,      AP_QtDialog_Background, 			FALSE)
DeclareDialog(AP_DIALOG_ID_GOTO,			AP_QtDialog_Goto, 				FALSE)
DeclareDialog(AP_DIALOG_ID_BREAK,			AP_QtDialog_Break, 				FALSE)
#ifdef ENABLE_SPELL
DeclareDialog(AP_DIALOG_ID_SPELL,			AP_QtDialog_Spell, 				FALSE)
#endif
DeclareDialog(AP_DIALOG_ID_STYLES,			AP_QtDialog_Styles, 				FALSE)
DeclareDialog(AP_DIALOG_ID_STYLIST,			AP_QtDialog_Stylist, 				FALSE)
DeclareDialog(AP_DIALOG_ID_PAGE_NUMBERS,    AP_QtDialog_PageNumbers, 			FALSE)
DeclareDialog(AP_DIALOG_ID_PARAGRAPH,		AP_QtDialog_Paragraph, 			FALSE)
DeclareDialog(AP_DIALOG_ID_OPTIONS,			AP_QtDialog_Options, 				TRUE)
DeclareDialog(AP_DIALOG_ID_TAB,				AP_QtDialog_Tab, 					FALSE)
DeclareDialog(AP_DIALOG_ID_INSERT_DATETIME,	AP_QtDialog_Insert_DateTime, 		FALSE)
DeclareDialog(AP_DIALOG_ID_WORDCOUNT,		AP_QtDialog_WordCount, 			FALSE)
DeclareDialog(AP_DIALOG_ID_FIELD,			AP_QtDialog_Field, 				FALSE)
DeclareDialog(AP_DIALOG_ID_LISTS,			AP_QtDialog_Lists, 				FALSE)
DeclareDialog(AP_DIALOG_ID_COLUMNS,			AP_QtDialog_Columns, 				FALSE)
DeclareDialog(AP_DIALOG_ID_TAB,				AP_QtDialog_Tab, 					FALSE)
DeclareDialog(AP_DIALOG_ID_FILE_PAGESETUP,  AP_QtDialog_PageSetup, 			FALSE)
DeclareDialog(AP_DIALOG_ID_TOGGLECASE,      AP_QtDialog_ToggleCase, 			FALSE)
DeclareDialog(AP_DIALOG_ID_FILE_NEW,        AP_QtDialog_New, 					FALSE)
DeclareDialog(AP_DIALOG_ID_INSERTBOOKMARK,	AP_QtDialog_InsertBookmark, 		FALSE)
DeclareDialog(AP_DIALOG_ID_INSERTHYPERLINK,	AP_QtDialog_InsertHyperlink, 		FALSE)
DeclareDialog(AP_DIALOG_ID_INSERTXMLID,     AP_QtDialog_InsertXMLID,   		FALSE)
DeclareDialog(XAP_DIALOG_ID_IMAGE, 			XAP_QtDialog_Image, 				FALSE)
DeclareDialog(AP_DIALOG_ID_METADATA,		AP_QtDialog_MetaData, 			FALSE)
DeclareDialog(AP_DIALOG_ID_MARK_REVISIONS,	AP_QtDialog_MarkRevisions, 		FALSE)
DeclareDialog(AP_DIALOG_ID_LIST_REVISIONS,	AP_QtDialog_ListRevisions, 		FALSE)
DeclareDialog(AP_DIALOG_ID_ANNOTATION,		AP_QtDialog_Annotation, 			FALSE)
DeclareDialog(AP_DIALOG_ID_INSERT_TABLE,	AP_QtDialog_InsertTable, 			FALSE)
DeclareDialog(AP_DIALOG_ID_MERGE_CELLS,		AP_QtDialog_MergeCells, 			FALSE)
DeclareDialog(AP_DIALOG_ID_SPLIT_CELLS,		AP_QtDialog_SplitCells, 			FALSE)
DeclareDialog(AP_DIALOG_ID_FORMAT_TABLE,	AP_QtDialog_FormatTable, 			FALSE)
DeclareDialog(AP_DIALOG_ID_FORMAT_FRAME,	AP_QtDialog_FormatFrame, 			FALSE)
DeclareDialog(AP_DIALOG_ID_FORMAT_FOOTNOTES,AP_QtDialog_FormatFootnotes, 		FALSE)
DeclareDialog(AP_DIALOG_ID_FORMAT_TOC,		AP_QtDialog_FormatTOC, 			FALSE)
DeclareDialog(AP_DIALOG_ID_MAILMERGE,		AP_QtDialog_MailMerge, 			FALSE)
DeclareDialog(AP_DIALOG_ID_LATEX,		    AP_QtDialog_Latex, 				FALSE)
DeclareDialog(AP_DIALOG_ID_ANNOTATION_PREVIEW,	AP_QtPreview_Annotation, 				FALSE)
DeclareDialog(AP_DIALOG_ID_BORDER_SHADING,	AP_QtDialog_Border_Shading, 				FALSE)

DeclareDialog(AP_DIALOG_ID_RDF_QUERY,			AP_QtDialog_RDFQuery, 				FALSE)
DeclareDialog(AP_DIALOG_ID_RDF_EDITOR,			AP_QtDialog_RDFEditor, 				FALSE)

#endif

#endif /* AP_QTDIALOG_ALL_H */
