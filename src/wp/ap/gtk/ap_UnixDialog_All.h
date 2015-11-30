/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

/*
 * Port to Maemo Development Platform
 * Author: INdT - Renato Araujo <renato.filho@indt.org.br>
 */

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.  Each time you add an entry to the top-half
** of this file be sure to add a corresponding entry to the other
** half and be sure to add an entry to each of the other platforms.
******************************************************************
*****************************************************************/

#ifndef AP_UNIXDIALOG_ALL_H
#define AP_UNIXDIALOG_ALL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#   include "xap_UnixDlg_About.h"
#   include "xap_UnixDlg_ClipArt.h"
#	include "xap_UnixDlg_MessageBox.h"
#	include "xap_UnixDlg_WindowMore.h"
#	include "xap_UnixDlg_Zoom.h"
#	include "xap_UnixDlg_Insert_Symbol.h"
#	include "xap_UnixDlg_Language.h"
#   include "xap_UnixDlg_PluginManager.h"
#   include "xap_UnixDlg_FileOpenSaveAs.h"
#   include "xap_UnixDlg_Encoding.h"
#   include "xap_UnixDlg_HTMLOptions.h"
#   include "xap_UnixDlg_Password.h"
#   include "xap_UnixDlg_Image.h"
#   include "xap_UnixDlg_ListDocuments.h"
#	include "xap_UnixDlg_History.h"
#   include "xap_UnixDlg_DocComparison.h"

#	include "ap_UnixDialog_Replace.h"
#	include "ap_UnixDialog_Break.h"
#	include "ap_UnixDialog_InsertTable.h"
#	include "ap_UnixDialog_Goto.h"
#   include "ap_UnixDialog_PageNumbers.h"
#   include "ap_UnixDialog_PageSetup.h"
#	include "ap_UnixDialog_Paragraph.h"
#	include "ap_UnixDialog_Options.h"
#ifdef ENABLE_SPELL
#	include "ap_UnixDialog_Spell.h"
#endif
#	include "ap_UnixDialog_Styles.h"
#   include "ap_UnixDialog_Stylist.h"
#	include "ap_UnixDialog_Tab.h"
#	include "ap_UnixDialog_Insert_DateTime.h"
#	include "ap_UnixDialog_WordCount.h"
#	include "ap_UnixDialog_Field.h"
#	include "ap_UnixDialog_Lists.h"
#	include "ap_UnixDialog_Columns.h"
#	include "ap_UnixDialog_Tab.h"
#   include "ap_UnixDialog_ToggleCase.h"
#   include "ap_UnixDialog_Background.h"
#   include "ap_UnixDialog_New.h"
#   include "ap_UnixDialog_HdrFtr.h"
#	include "ap_UnixDialog_InsertBookmark.h"
#	include "ap_UnixDialog_InsertHyperlink.h"
#	include "ap_UnixDialog_InsertXMLID.h"
#   include "ap_UnixDialog_MetaData.h"
#   include "ap_UnixDialog_MarkRevisions.h"
#   include "ap_UnixDialog_ListRevisions.h"
#   include "ap_UnixDialog_Annotation.h"
#   include "ap_UnixPreview_Annotation.h"
#   include "ap_UnixDialog_MergeCells.h"
#   include "ap_UnixDialog_SplitCells.h"
#   include "ap_UnixDialog_FormatTable.h"
#   include "ap_UnixDialog_FormatFrame.h"
#   include "ap_UnixDialog_FormatFootnotes.h"
#   include "ap_UnixDialog_FormatTOC.h"
#   include "ap_UnixDialog_MailMerge.h"
#   include "ap_UnixDialog_Latex.h"
#	include "ap_UnixDialog_Border_Shading.h"
#   ifdef ENABLE_PRINT
#       include "xap_UnixDlg_Print.h"
#       include "xap_UnixDlg_PrintPreview.h"
#   endif
#if 0
#	include "ap_UnixDialog_Download_File.h"
#endif

#include "ap_UnixDialog_RDFQuery.h"
#include "ap_UnixDialog_RDFEditor.h"


#   include "xap_UnixDlg_FontChooser.h"

#else

	DeclareDialog(XAP_DIALOG_ID_FONT,			XAP_UnixDialog_FontChooser, 		FALSE)

	DeclareDialog(XAP_DIALOG_ID_ABOUT,			XAP_UnixDialog_About, 				FALSE)
	DeclareDialog(XAP_DIALOG_ID_CLIPART,		XAP_UnixDialog_ClipArt,				FALSE)
	DeclareDialog(XAP_DIALOG_ID_FILE_OPEN,		XAP_UnixDialog_FileOpenSaveAs, 		FALSE)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVEAS,	XAP_UnixDialog_FileOpenSaveAs, 		FALSE)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVE_IMAGE,	XAP_UnixDialog_FileOpenSaveAs, 		FALSE)
#ifdef ENABLE_PRINT
	DeclareDialog(XAP_DIALOG_ID_PRINT,			XAP_UnixDialog_Print, 				FALSE)
	DeclareDialog(XAP_DIALOG_ID_PRINTPREVIEW,	XAP_UnixDialog_PrintPreview, 		FALSE)
	DeclareDialog(XAP_DIALOG_ID_PRINTTOFILE,	XAP_UnixDialog_FileOpenSaveAs, 		FALSE)
#endif
	DeclareDialog(XAP_DIALOG_ID_MESSAGE_BOX,	XAP_UnixDialog_MessageBox, 			FALSE)
	DeclareDialog(XAP_DIALOG_ID_RECORDTOFILE,	XAP_UnixDialog_FileOpenSaveAs, 		FALSE)
	DeclareDialog(XAP_DIALOG_ID_REPLAYFROMFILE,	XAP_UnixDialog_FileOpenSaveAs, 		FALSE)
	DeclareDialog(XAP_DIALOG_ID_LANGUAGE,		XAP_UnixDialog_Language, 			FALSE)
	DeclareDialog(XAP_DIALOG_ID_WINDOWMORE,		XAP_UnixDialog_WindowMore, 			FALSE)
	DeclareDialog(XAP_DIALOG_ID_ZOOM,			XAP_UnixDialog_Zoom, 				FALSE)
	DeclareDialog(XAP_DIALOG_ID_INSERT_SYMBOL,  XAP_UnixDialog_Insert_Symbol, 		FALSE)
	DeclareDialog(XAP_DIALOG_ID_INSERT_PICTURE,	XAP_UnixDialog_FileOpenSaveAs, 		FALSE)
	DeclareDialog(XAP_DIALOG_ID_INSERTMATHML,	XAP_UnixDialog_FileOpenSaveAs, 		FALSE)
	DeclareDialog(XAP_DIALOG_ID_INSERTOBJECT,	XAP_UnixDialog_FileOpenSaveAs, 		FALSE)
	DeclareDialog(XAP_DIALOG_ID_PLUGIN_MANAGER, XAP_UnixDialog_PluginManager, 		FALSE)
	DeclareDialog(XAP_DIALOG_ID_ENCODING,       XAP_UnixDialog_Encoding, 			FALSE)
	DeclareDialog(XAP_DIALOG_ID_HTMLOPTIONS,    XAP_UnixDialog_HTMLOptions, 		FALSE)
    DeclareDialog(XAP_DIALOG_ID_FILE_IMPORT,    XAP_UnixDialog_FileOpenSaveAs, 		FALSE)
    DeclareDialog(XAP_DIALOG_ID_FILE_EXPORT,    XAP_UnixDialog_FileOpenSaveAs, 		FALSE)
    DeclareDialog(XAP_DIALOG_ID_INSERT_FILE,    XAP_UnixDialog_FileOpenSaveAs, 		FALSE)
    DeclareDialog(XAP_DIALOG_ID_PASSWORD,       XAP_UnixDialog_Password, 			FALSE)
    DeclareDialog(XAP_DIALOG_ID_IMAGE,          XAP_UnixDialog_Image, 				FALSE)
	DeclareDialog(XAP_DIALOG_ID_LISTDOCUMENTS,  XAP_UnixDialog_ListDocuments, 		FALSE)
	DeclareDialog(XAP_DIALOG_ID_COMPAREDOCUMENTS,XAP_UnixDialog_ListDocuments, 		FALSE)
	DeclareDialog(XAP_DIALOG_ID_MERGEDOCUMENTS, XAP_UnixDialog_ListDocuments, 		FALSE)
    DeclareDialog(XAP_DIALOG_ID_HISTORY,        XAP_UnixDialog_History, 			FALSE)
    DeclareDialog(XAP_DIALOG_ID_DOCCOMPARISON,  XAP_UnixDialog_DocComparison, 		FALSE)

	DeclareDialog(AP_DIALOG_ID_REPLACE,			AP_UnixDialog_Replace, 				FALSE)
	DeclareDialog(AP_DIALOG_ID_FIND,			AP_UnixDialog_Replace, 				FALSE)
    DeclareDialog(AP_DIALOG_ID_HDRFTR,          AP_UnixDialog_HdrFtr, 				FALSE)
    DeclareDialog(AP_DIALOG_ID_BACKGROUND,      AP_UnixDialog_Background, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_GOTO,			AP_UnixDialog_Goto, 				FALSE)
	DeclareDialog(AP_DIALOG_ID_BREAK,			AP_UnixDialog_Break, 				FALSE)
#ifdef ENABLE_SPELL
	DeclareDialog(AP_DIALOG_ID_SPELL,			AP_UnixDialog_Spell, 				FALSE)
#endif
	DeclareDialog(AP_DIALOG_ID_STYLES,			AP_UnixDialog_Styles, 				FALSE)
	DeclareDialog(AP_DIALOG_ID_STYLIST,			AP_UnixDialog_Stylist, 				FALSE)
	DeclareDialog(AP_DIALOG_ID_PAGE_NUMBERS,    AP_UnixDialog_PageNumbers, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_PARAGRAPH,		AP_UnixDialog_Paragraph, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_OPTIONS,			AP_UnixDialog_Options, 				TRUE)
	DeclareDialog(AP_DIALOG_ID_TAB,				AP_UnixDialog_Tab, 					FALSE)
	DeclareDialog(AP_DIALOG_ID_INSERT_DATETIME,	AP_UnixDialog_Insert_DateTime, 		FALSE)
	DeclareDialog(AP_DIALOG_ID_WORDCOUNT,		AP_UnixDialog_WordCount, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_FIELD,			AP_UnixDialog_Field, 				FALSE)
	DeclareDialog(AP_DIALOG_ID_LISTS,			AP_UnixDialog_Lists, 				FALSE)
	DeclareDialog(AP_DIALOG_ID_COLUMNS,			AP_UnixDialog_Columns, 				FALSE)
	DeclareDialog(AP_DIALOG_ID_TAB,				AP_UnixDialog_Tab, 					FALSE)
	DeclareDialog(AP_DIALOG_ID_FILE_PAGESETUP,  AP_UnixDialog_PageSetup, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_TOGGLECASE,      AP_UnixDialog_ToggleCase, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_FILE_NEW,        AP_UnixDialog_New, 					FALSE)
	DeclareDialog(AP_DIALOG_ID_INSERTBOOKMARK,	AP_UnixDialog_InsertBookmark, 		FALSE)
	DeclareDialog(AP_DIALOG_ID_INSERTHYPERLINK,	AP_UnixDialog_InsertHyperlink, 		FALSE)
	DeclareDialog(AP_DIALOG_ID_INSERTXMLID,     AP_UnixDialog_InsertXMLID,   		FALSE)
	DeclareDialog(XAP_DIALOG_ID_IMAGE, 			XAP_UnixDialog_Image, 				FALSE)
	DeclareDialog(AP_DIALOG_ID_METADATA,		AP_UnixDialog_MetaData, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_MARK_REVISIONS,	AP_UnixDialog_MarkRevisions, 		FALSE)
	DeclareDialog(AP_DIALOG_ID_LIST_REVISIONS,	AP_UnixDialog_ListRevisions, 		FALSE)
	DeclareDialog(AP_DIALOG_ID_ANNOTATION,		AP_UnixDialog_Annotation, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_INSERT_TABLE,	AP_UnixDialog_InsertTable, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_MERGE_CELLS,		AP_UnixDialog_MergeCells, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_SPLIT_CELLS,		AP_UnixDialog_SplitCells, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_FORMAT_TABLE,	AP_UnixDialog_FormatTable, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_FORMAT_FRAME,	AP_UnixDialog_FormatFrame, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_FORMAT_FOOTNOTES,AP_UnixDialog_FormatFootnotes, 		FALSE)
	DeclareDialog(AP_DIALOG_ID_FORMAT_TOC,		AP_UnixDialog_FormatTOC, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_MAILMERGE,		AP_UnixDialog_MailMerge, 			FALSE)
	DeclareDialog(AP_DIALOG_ID_LATEX,		    AP_UnixDialog_Latex, 				FALSE)
	DeclareDialog(AP_DIALOG_ID_ANNOTATION_PREVIEW,	AP_UnixPreview_Annotation, 				FALSE)
	DeclareDialog(AP_DIALOG_ID_BORDER_SHADING,	AP_UnixDialog_Border_Shading, 				FALSE)

    DeclareDialog(AP_DIALOG_ID_RDF_QUERY,			AP_UnixDialog_RDFQuery, 				FALSE)
    DeclareDialog(AP_DIALOG_ID_RDF_EDITOR,			AP_UnixDialog_RDFEditor, 				FALSE)
#endif /* AP_UNIXDIALOG_ALL_H */
