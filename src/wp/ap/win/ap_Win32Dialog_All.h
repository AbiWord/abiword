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

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.  Each time you add an entry to the top-half
** of this file be sure to add a corresponding entry to the other
** half and be sure to add an entry to each of the other platforms.
******************************************************************
*****************************************************************/

#ifndef AP_WIN32DIALOG_ALL_H

#	define AP_WIN32DIALOG_ALL_H

#	include "xap_Win32Dlg_MessageBox.h"
#	include "xap_Win32Dlg_FileOpenSaveAs.h"
#	include "xap_Win32Dlg_Print.h"
#	include "xap_Win32Dlg_PrintPreview.h"
#	include "xap_Win32Dlg_FontChooser.h"
#	include "xap_Win32Dlg_WindowMore.h"
#	include "xap_Win32Dlg_About.h"
#	include "xap_Win32Dlg_Zoom.h"
#	include "xap_Win32Dlg_Insert_Symbol.h"
#	include "xap_Win32Dlg_Language.h"
#	include "xap_Win32Dlg_Encoding.h"
#	include "xap_Win32Dlg_PluginManager.h"
#	include "xap_Win32Dlg_Password.h"
#	include "xap_Win32Dlg_Image.h"
#	include "xap_Win32Dlg_HTMLOptions.h"
#	include "xap_Win32Dlg_ListDocuments.h"
#	include "xap_Win32Dlg_History.h"
#	include "xap_Win32Dlg_DocComparison.h"

#	include "ap_Win32Dialog_Replace.h"
#	include "ap_Win32Dialog_Break.h"
#	include "ap_Win32Dialog_Options.h"
#	include "ap_Win32Dialog_Paragraph.h"
#ifdef ENABLE_SPELL
#	include "ap_Win32Dialog_Spell.h"
#endif
#	include "ap_Win32Dialog_Styles.h"
#	include "ap_Win32Dialog_Insert_DateTime.h"
#	include "ap_Win32Dialog_WordCount.h"
#	include "ap_Win32Dialog_Field.h"
#	include "ap_Win32Dialog_Goto.h"
#	include "ap_Win32Dialog_Columns.h"
#	include "ap_Win32Dialog_Lists.h"
#	include "ap_Win32Dialog_Tab.h"
#	include "ap_Win32Dialog_PageNumbers.h"
#	include "ap_Win32Dialog_PageSetup.h"
#	include "ap_Win32Dialog_ToggleCase.h"
#	include "ap_Win32Dialog_Background.h"
#	include "ap_Win32Dialog_HdrFtr.h"
#	include "ap_Win32Dialog_InsertBookmark.h"
#	include "ap_Win32Dialog_InsertHyperlink.h"
#	include "ap_Win32Dialog_New.h"
#	include "ap_Win32Dialog_MarkRevisions.h"
#	include "ap_Win32Dialog_ListRevisions.h"
#	include "ap_Win32Dialog_InsertTable.h"
#	include "ap_Win32Dialog_MetaData.h"
#	include "ap_Win32Dialog_MergeCells.h"
#	include "ap_Win32Dialog_SplitCells.h"
#	include "ap_Win32Dialog_FormatTable.h"
#	include "ap_Win32Dialog_FormatFootnotes.h"
#	include "ap_Win32Dialog_MailMerge.h"
#	include "ap_Win32Dialog_FormatFrame.h"
#	include "ap_Win32Dialog_FormatTOC.h"
#	include "ap_Win32Dialog_Latex.h"
#	include "ap_Win32Dialog_Stylist.h"
#	include "ap_Win32Dialog_Annotation.h"
#   include "ap_Win32Preview_Annotation.h"
	// ... add new dialogs here ...

//	Maleesh 6/8/2010 -
#	include "ap_Win32Dialog_Border_Shading.h"
#else

	DeclareDialog(XAP_DIALOG_ID_MESSAGE_BOX,	XAP_Win32Dialog_MessageBox)
	DeclareDialog(XAP_DIALOG_ID_FILE_OPEN,		XAP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVEAS,	XAP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVE_IMAGE,	XAP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_IMPORT,	XAP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_EXPORT,	XAP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_INSERT_FILE,	XAP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_PRINT,			XAP_Win32Dialog_Print)
	DeclareDialog(XAP_DIALOG_ID_PRINTPREVIEW,		XAP_Win32Dialog_PrintPreview)
	DeclareDialog(XAP_DIALOG_ID_PRINTTOFILE,	XAP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_RECORDTOFILE,	XAP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_REPLAYFROMFILE,	XAP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FONT,			XAP_Win32Dialog_FontChooser)
	DeclareDialog(XAP_DIALOG_ID_LANGUAGE,		XAP_Win32Dialog_Language)
	DeclareDialog(XAP_DIALOG_ID_WINDOWMORE,		XAP_Win32Dialog_WindowMore)
	DeclareDialog(XAP_DIALOG_ID_ZOOM,			XAP_Win32Dialog_Zoom)
	DeclareDialog(XAP_DIALOG_ID_ABOUT,			XAP_Win32Dialog_About)
	DeclareDialog(XAP_DIALOG_ID_INSERT_SYMBOL,	XAP_Win32Dialog_Insert_Symbol)
	DeclareDialog(XAP_DIALOG_ID_INSERT_PICTURE, XAP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_INSERTMATHML,	XAP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_INSERTOBJECT,	XAP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_ENCODING,		XAP_Win32Dialog_Encoding)
	DeclareDialog(XAP_DIALOG_ID_PLUGIN_MANAGER, XAP_Win32Dialog_PluginManager)
	DeclareDialog(XAP_DIALOG_ID_PASSWORD,		XAP_Win32Dialog_Password)
	DeclareDialog(XAP_DIALOG_ID_IMAGE,			XAP_Win32Dialog_Image)
	DeclareDialog(XAP_DIALOG_ID_HTMLOPTIONS,    XAP_Win32Dialog_HTMLOptions)
	DeclareDialog(XAP_DIALOG_ID_LISTDOCUMENTS,  XAP_Win32Dialog_ListDocuments)
	DeclareDialog(XAP_DIALOG_ID_MERGEDOCUMENTS, XAP_Win32Dialog_ListDocuments)
	DeclareDialog(XAP_DIALOG_ID_COMPAREDOCUMENTS,XAP_Win32Dialog_ListDocuments)
	DeclareDialog(XAP_DIALOG_ID_HISTORY,        XAP_Win32Dialog_History)
	DeclareDialog(XAP_DIALOG_ID_DOCCOMPARISON,  XAP_Win32Dialog_DocComparison)

	DeclareDialog(AP_DIALOG_ID_REPLACE,			AP_Win32Dialog_Replace)
	DeclareDialog(AP_DIALOG_ID_FIND,			AP_Win32Dialog_Replace)
	DeclareDialog(AP_DIALOG_ID_BREAK,			AP_Win32Dialog_Break)
#ifdef ENABLE_SPELL
	DeclareDialog(AP_DIALOG_ID_SPELL,			AP_Win32Dialog_Spell)
#endif
	DeclareDialog(AP_DIALOG_ID_STYLES,			AP_Win32Dialog_Styles)
	DeclareDialog(AP_DIALOG_ID_OPTIONS,			AP_Win32Dialog_Options)
	DeclareDialog(AP_DIALOG_ID_PARAGRAPH,	   	AP_Win32Dialog_Paragraph)
	DeclareDialog(AP_DIALOG_ID_INSERT_DATETIME,	AP_Win32Dialog_Insert_DateTime)
	DeclareDialog(AP_DIALOG_ID_WORDCOUNT,		AP_Win32Dialog_WordCount)
	DeclareDialog(AP_DIALOG_ID_FIELD,			AP_Win32Dialog_Field)
	DeclareDialog(AP_DIALOG_ID_GOTO,			AP_Win32Dialog_Goto)
	DeclareDialog(AP_DIALOG_ID_COLUMNS,			AP_Win32Dialog_Columns)
	DeclareDialog(AP_DIALOG_ID_LISTS,			AP_Win32Dialog_Lists)
	DeclareDialog(AP_DIALOG_ID_TAB,				AP_Win32Dialog_Tab)
	DeclareDialog(AP_DIALOG_ID_PAGE_NUMBERS,	AP_Win32Dialog_PageNumbers)
	DeclareDialog(AP_DIALOG_ID_FILE_PAGESETUP,	AP_Win32Dialog_PageSetup)
	DeclareDialog(AP_DIALOG_ID_TOGGLECASE,		AP_Win32Dialog_ToggleCase)
	DeclareDialog(AP_DIALOG_ID_BACKGROUND,		AP_Win32Dialog_Background)
	DeclareDialog(AP_DIALOG_ID_HDRFTR,  		AP_Win32Dialog_HdrFtr)
 	DeclareDialog(AP_DIALOG_ID_INSERTBOOKMARK,	AP_Win32Dialog_InsertBookmark)
 	DeclareDialog(AP_DIALOG_ID_INSERTHYPERLINK,	AP_Win32Dialog_InsertHyperlink)
	DeclareDialog(AP_DIALOG_ID_FILE_NEW,		AP_Win32Dialog_New)
	DeclareDialog(AP_DIALOG_ID_MARK_REVISIONS,	AP_Win32Dialog_MarkRevisions)
	DeclareDialog(AP_DIALOG_ID_LIST_REVISIONS,	AP_Win32Dialog_ListRevisions)
	DeclareDialog(AP_DIALOG_ID_INSERT_TABLE,	AP_Win32Dialog_InsertTable)
	DeclareDialog(AP_DIALOG_ID_METADATA,		AP_Win32Dialog_MetaData)
	DeclareDialog(AP_DIALOG_ID_MERGE_CELLS,		AP_Win32Dialog_MergeCells)
	DeclareDialog(AP_DIALOG_ID_SPLIT_CELLS,		AP_Win32Dialog_SplitCells)
 	DeclareDialog(AP_DIALOG_ID_FORMAT_TABLE,	AP_Win32Dialog_FormatTable)
	DeclareDialog(AP_DIALOG_ID_FORMAT_FRAME,	AP_Win32Dialog_FormatFrame)
	DeclareDialog(AP_DIALOG_ID_FORMAT_FOOTNOTES,AP_Win32Dialog_FormatFootnotes)
	DeclareDialog(AP_DIALOG_ID_FORMAT_TOC,      AP_Win32Dialog_FormatTOC)
	DeclareDialog(AP_DIALOG_ID_MAILMERGE,		AP_Win32Dialog_MailMerge)
 	DeclareDialog(AP_DIALOG_ID_ANNOTATION, 		AP_Win32Dialog_Annotation)
	DeclareDialog(AP_DIALOG_ID_STYLIST,			AP_Win32Dialog_Stylist)
	DeclareDialog(AP_DIALOG_ID_LATEX,			AP_Win32Dialog_Latex)
	DeclareDialog(AP_DIALOG_ID_ANNOTATION_PREVIEW,	AP_Win32Preview_Annotation)
    // DeclareDialog(AP_DIALOG_ID_RDF_QUERY,			AP_Win32Dialog_RDFQuery)
	// DeclareDialog(AP_DIALOG_ID_RDF_EDITOR,			AP_Win32Dialog_RDFEditor)
 	// ... also add new dialogs here ...

	//	Maleesh 6/8/2010 -
	DeclareDialog(AP_DIALOG_ID_BORDER_SHADING,	AP_Win32Dialog_Border_Shading)

	// Remember to place the dialog box with the higher ID
	// as the latest member. See XAP_DialogFactory::getNextId
#endif
