/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2004 Hubert Figuiere
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

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.  Each time you add an entry to the top-half
** of this file be sure to add a corresponding entry to the other
** half and be sure to add an entry to each of the other platforms.
******************************************************************
*****************************************************************/

#ifndef AP_COCOADIALOG_ALL_H
#define AP_COCOADIALOG_ALL_H

#	define DEFAULT_BUTTON_WIDTH	85

#	include "xap_CocoaDlg_MessageBox.h"
#	include "xap_CocoaDlg_FileOpenSaveAs.h"
#	include "xap_CocoaDlg_Print.h"
#   include "xap_CocoaDlg_PrintPreview.h"
#	include "xap_CocoaDlg_WindowMore.h"
#	include "xap_CocoaDlg_FontChooser.h"
#	include "xap_CocoaDlg_About.h"
#	include "xap_CocoaDlg_Zoom.h"
#	include "xap_CocoaDlg_Insert_Symbol.h"
#	include "xap_CocoaDlg_Language.h"
#   include "xap_CocoaDlg_PluginManager.h"
#   include "xap_CocoaDlg_ClipArt.h"
#   include "xap_CocoaDlg_Encoding.h"
#	include "xap_CocoaDlg_Password.h"
#	include "xap_CocoaDlg_Image.h"
#   include "xap_CocoaDlg_HTMLOptions.h"
#   include "xap_CocoaDlg_ListDocuments.h"
#   include "xap_CocoaDlg_DocComparison.h"
#   include "xap_CocoaDlg_History.h"

#	include "ap_CocoaDialog_Replace.h"
#	include "ap_CocoaDialog_Break.h"
#	include "ap_CocoaDialog_FormatFootnotes.h"
#	include "ap_CocoaDialog_Goto.h"
#   include "ap_CocoaDialog_PageNumbers.h"
#   include "ap_CocoaDialog_PageSetup.h"
#	include "ap_CocoaDialog_Paragraph.h"
#	include "ap_CocoaDialog_Options.h"
#	include "ap_CocoaDialog_Spell.h"
#	include "ap_CocoaDialog_Styles.h"
#	include "ap_CocoaDialog_Tab.h"
#	include "ap_CocoaDialog_Insert_DateTime.h"
#	include "ap_CocoaDialog_WordCount.h"
#	include "ap_CocoaDialog_Field.h"
#	include "ap_CocoaDialog_Lists.h"
#	include "ap_CocoaDialog_Columns.h"
#	include "ap_CocoaDialog_Tab.h"
#   include "ap_CocoaDialog_ToggleCase.h"
#   include "ap_CocoaDialog_Background.h"
#   include "ap_CocoaDialog_New.h"
#   include "ap_CocoaDialog_HdrFtr.h"
#	include "ap_CocoaDialog_InsertBookmark.h"
#	include "ap_CocoaDialog_InsertHyperlink.h"
#	include "ap_CocoaDialog_MetaData.h"
#	include "ap_CocoaDialog_MarkRevisions.h"
#	include "ap_CocoaDialog_ListRevisions.h"
#	include "ap_CocoaDialog_MergeCells.h"
#	include "ap_CocoaDialog_SplitCells.h"
#	include "ap_CocoaDialog_InsertTable.h"
#	include "ap_CocoaDialog_FormatTable.h"
#	include "ap_CocoaDialog_FormatFrame.h"
#	include "ap_CocoaDialog_FormatTOC.h"
#	include "ap_CocoaDialog_Stylist.h"
#	include "ap_CocoaDialog_MailMerge.h"
#	include "ap_CocoaDialog_Latex.h"
#   include "ap_CocoaDialog_Annotation.h"
#   include "ap_CocoaPreview_Annotation.h"
#if 0
#	include "ap_CocoaDialog_Download_File.h"
#endif
	// ... add new dialogs here ...

#else

	DeclareDialog(XAP_DIALOG_ID_MESSAGE_BOX,	XAP_CocoaDialog_MessageBox)
	DeclareDialog(XAP_DIALOG_ID_FILE_OPEN,		XAP_CocoaDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVEAS,	XAP_CocoaDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVE_IMAGE,	XAP_CocoaDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_PRINT,			XAP_CocoaDialog_Print)
	DeclareDialog(XAP_DIALOG_ID_PRINTTOFILE,	XAP_CocoaDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_RECORDTOFILE,	XAP_CocoaDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_REPLAYFROMFILE,	XAP_CocoaDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FONT,			XAP_CocoaDialog_FontChooser)
	DeclareDialog(XAP_DIALOG_ID_LANGUAGE,		XAP_CocoaDialog_Language)
	DeclareDialog(XAP_DIALOG_ID_WINDOWMORE,		XAP_CocoaDialog_WindowMore)
	DeclareDialog(XAP_DIALOG_ID_ZOOM,			XAP_CocoaDialog_Zoom)
	DeclareDialog(XAP_DIALOG_ID_INSERT_SYMBOL,  XAP_CocoaDialog_Insert_Symbol)
	DeclareDialog(XAP_DIALOG_ID_INSERT_PICTURE,	XAP_CocoaDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_INSERTMATHML,	XAP_CocoaDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_INSERTOBJECT,	XAP_CocoaDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_HTMLOPTIONS,	XAP_CocoaDialog_HTMLOptions)
	DeclareDialog(XAP_DIALOG_ID_ABOUT,			XAP_CocoaDialog_About)
	DeclareDialog(XAP_DIALOG_ID_PLUGIN_MANAGER, XAP_CocoaDialog_PluginManager)
//PORT	DeclareDialog(XAP_DIALOG_ID_CLIPART,        XAP_CocoaDialog_ClipArt)
	DeclareDialog(XAP_DIALOG_ID_ENCODING,       XAP_CocoaDialog_Encoding)
	DeclareDialog(XAP_DIALOG_ID_FILE_IMPORT, XAP_CocoaDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_EXPORT, XAP_CocoaDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_INSERT_FILE, XAP_CocoaDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_PASSWORD, XAP_CocoaDialog_Password)
	DeclareDialog(XAP_DIALOG_ID_IMAGE, XAP_CocoaDialog_Image)
	DeclareDialog(XAP_DIALOG_ID_LISTDOCUMENTS,  XAP_CocoaDialog_ListDocuments)
	DeclareDialog(XAP_DIALOG_ID_COMPAREDOCUMENTS,XAP_CocoaDialog_ListDocuments)
	DeclareDialog(XAP_DIALOG_ID_MERGEDOCUMENTS, XAP_CocoaDialog_ListDocuments)
	DeclareDialog(XAP_DIALOG_ID_HISTORY,		XAP_CocoaDialog_History)
	DeclareDialog(XAP_DIALOG_ID_DOCCOMPARISON,		XAP_CocoaDialog_DocComparison)

	DeclareDialog(AP_DIALOG_ID_REPLACE,			AP_CocoaDialog_Replace)
	DeclareDialog(AP_DIALOG_ID_FIND,			AP_CocoaDialog_Replace)
	DeclareDialog(AP_DIALOG_ID_HDRFTR,          AP_CocoaDialog_HdrFtr)
	DeclareDialog(AP_DIALOG_ID_BACKGROUND,      AP_CocoaDialog_Background)
	DeclareDialog(AP_DIALOG_ID_GOTO,			AP_CocoaDialog_Goto)
	DeclareDialog(AP_DIALOG_ID_BREAK,			AP_CocoaDialog_Break)
#ifdef ENABLE_SPELL
	DeclareDialog(AP_DIALOG_ID_SPELL,			AP_CocoaDialog_Spell)
#endif
	DeclareDialog(AP_DIALOG_ID_STYLES,			AP_CocoaDialog_Styles)
	DeclareDialog(AP_DIALOG_ID_PAGE_NUMBERS,    AP_CocoaDialog_PageNumbers)
	DeclareDialog(AP_DIALOG_ID_PARAGRAPH,		AP_CocoaDialog_Paragraph)
	DeclareDialog(AP_DIALOG_ID_OPTIONS,			AP_CocoaDialog_Options)
	DeclareDialog(AP_DIALOG_ID_INSERT_DATETIME,	AP_CocoaDialog_Insert_DateTime)
	DeclareDialog(AP_DIALOG_ID_WORDCOUNT,		AP_CocoaDialog_WordCount)
	DeclareDialog(AP_DIALOG_ID_FIELD,			AP_CocoaDialog_Field)
	DeclareDialog(AP_DIALOG_ID_LISTS,			AP_CocoaDialog_Lists)
	DeclareDialog(AP_DIALOG_ID_COLUMNS,			AP_CocoaDialog_Columns)
	DeclareDialog(AP_DIALOG_ID_TAB,				AP_CocoaDialog_Tab)
	DeclareDialog(AP_DIALOG_ID_FILE_PAGESETUP,  AP_CocoaDialog_PageSetup)
	DeclareDialog(AP_DIALOG_ID_TOGGLECASE,      AP_CocoaDialog_ToggleCase)
	DeclareDialog(AP_DIALOG_ID_FILE_NEW,        AP_CocoaDialog_New)

	DeclareDialog(AP_DIALOG_ID_INSERTBOOKMARK,	AP_CocoaDialog_InsertBookmark)
	DeclareDialog(AP_DIALOG_ID_INSERTHYPERLINK,	AP_CocoaDialog_InsertHyperlink)
	DeclareDialog(AP_DIALOG_ID_METADATA,		AP_CocoaDialog_MetaData)
 	DeclareDialog(AP_DIALOG_ID_MARK_REVISIONS,	AP_CocoaDialog_MarkRevisions)
	DeclareDialog(AP_DIALOG_ID_LIST_REVISIONS,	AP_CocoaDialog_ListRevisions)
	DeclareDialog(AP_DIALOG_ID_ANNOTATION,  AP_CocoaDialog_Annotation)
	DeclareDialog(AP_DIALOG_ID_INSERT_TABLE,		AP_CocoaDialog_InsertTable)
	DeclareDialog(AP_DIALOG_ID_MERGE_CELLS,		AP_CocoaDialog_MergeCells)
	DeclareDialog(AP_DIALOG_ID_SPLIT_CELLS,		AP_CocoaDialog_SplitCells)
	DeclareDialog(AP_DIALOG_ID_FORMAT_TABLE,		AP_CocoaDialog_FormatTable)
	DeclareDialog(AP_DIALOG_ID_FORMAT_FRAME,		AP_CocoaDialog_FormatFrame)
	DeclareDialog(AP_DIALOG_ID_FORMAT_FOOTNOTES,		AP_CocoaDialog_FormatFootnotes)
	DeclareDialog(AP_DIALOG_ID_FORMAT_TOC,		AP_CocoaDialog_FormatTOC)
	DeclareDialog(AP_DIALOG_ID_STYLIST,		AP_CocoaDialog_Stylist)
	DeclareDialog(AP_DIALOG_ID_MAILMERGE,		AP_CocoaDialog_MailMerge)
	DeclareDialog(AP_DIALOG_ID_LATEX,		AP_CocoaDialog_Latex)
	DeclareDialog(AP_DIALOG_ID_ANNOTATION_PREVIEW,	AP_CocoaPreview_Annotation)
#if 0
	DeclareDialog(AP_DIALOG_ID_DOWNLOAD_FILE,	AP_CocoaDialog_Download_File)
#endif

//FIXME: TO BE IMPLEMENTED
//	DeclareDialog(AP_DIALOG_ID_RDF_QUERY,			AP_CocoaDialog_RDFQuery)
//	DeclareDialog(AP_DIALOG_ID_RDF_EDITOR,			AP_CocoaDialog_RDFEditor)

    // ... also add new dialogs here ...

#endif
