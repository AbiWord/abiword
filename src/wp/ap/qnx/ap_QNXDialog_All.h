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
** MORE THAN ONE TIME.  Each time you add an entry to the top-half
** of this file be sure to add a corresponding entry to the other
** half and be sure to add an entry to each of the other platforms.
******************************************************************
*****************************************************************/

#ifndef AP_QNXDIALOG_ALL_H

#	define AP_QNXDIALOG_ALL_H

#	include "xap_QNXDlg_MessageBox.h"
#	include "xap_QNXDlg_FileOpenSaveAs.h"
#	include "xap_QNXDlg_Print.h"
#	include "xap_QNXDlg_WindowMore.h"
#	include "xap_QNXDlg_FontChooser.h"
#	include "xap_QNXDlg_About.h"
#	include "xap_QNXDlg_Zoom.h"
#	include "xap_QNXDlg_Insert_Symbol.h"
#	include "xap_QNXDlg_Language.h"
# include "xap_QNXDlg_PluginManager.h"
# include  "xap_QNXDlg_Password.h"
# include  "xap_QNXDlg_ClipArt.h"
# include  "xap_QNXDlg_Encoding.h"
# include  "xap_QNXDlg_Image.h"
# include	"xap_QNXDlg_HTMLOptions.h"

#	include "ap_QNXDialog_Replace.h"
#	include "ap_QNXDialog_Break.h"
#	include "ap_QNXDialog_Goto.h"
#	include "ap_QNXDialog_PageNumbers.h"
#	include "ap_QNXDialog_PageSetup.h"
#	include "ap_QNXDialog_Paragraph.h"
#	include "ap_QNXDialog_Options.h"
#	include "ap_QNXDialog_Spell.h"
#	include "ap_QNXDialog_Styles.h"
#	include "ap_QNXDialog_Tab.h"
#	include "ap_QNXDialog_Insert_DateTime.h"
#	include "ap_QNXDialog_WordCount.h"
#	include "ap_QNXDialog_Field.h"
#	include "ap_QNXDialog_Lists.h"
#	include "ap_QNXDialog_Columns.h"
#   include "ap_QNXDialog_ToggleCase.h"
#   include "ap_QNXDialog_Background.h"
#   include "ap_QNXDialog_New.h"
#   include "ap_QNXDialog_HdrFtr.h"
#	include "ap_QNXDialog_InsertBookmark.h"
#	include "ap_QNXDialog_InsertHyperlink.h"
# include "ap_QNXDialog_InsertTable.h"
#	include "ap_QNXDialog_MarkRevisions.h"
#	include "ap_QNXDialog_ListRevisions.h"
#	include "ap_QNXDialog_MetaData.h"
#if 0
#	include "ap_QNXDialog_Download_File.h"
#endif
#include "ap_QNXDialog_SplitCells.h"
#include "ap_QNXDialog_MergeCells.h"
#include "ap_QNXDialog_FormatTable.h"
#include "ap_QNXDialog_FormatFrame.h"
#include "ap_QNXDialog_FormatFootnotes.h"
#include "ap_QNXDialog_FormatTOC.h"
	// ... add new dialogs here ...

#else

	DeclareDialog(XAP_DIALOG_ID_MESSAGE_BOX,	XAP_QNXDialog_MessageBox)
	DeclareDialog(XAP_DIALOG_ID_FILE_OPEN,		XAP_QNXDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVEAS,	XAP_QNXDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_PRINT,			XAP_QNXDialog_Print)
	DeclareDialog(XAP_DIALOG_ID_PRINTTOFILE,	XAP_QNXDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FONT,			XAP_QNXDialog_FontChooser)
	DeclareDialog(XAP_DIALOG_ID_LANGUAGE,			XAP_QNXDialog_Language)
	DeclareDialog(XAP_DIALOG_ID_WINDOWMORE,		XAP_QNXDialog_WindowMore)
	DeclareDialog(XAP_DIALOG_ID_ZOOM,			XAP_QNXDialog_Zoom)
	DeclareDialog(XAP_DIALOG_ID_INSERT_SYMBOL,	XAP_QNXDialog_Insert_Symbol)
	DeclareDialog(XAP_DIALOG_ID_INSERT_PICTURE,	XAP_QNXDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_INSERTMATHML,	XAP_QNXDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_PLUGIN_MANAGER, XAP_QNXDialog_PluginManager)
	DeclareDialog(XAP_DIALOG_ID_ABOUT,			XAP_QNXDialog_About)
	DeclareDialog(XAP_DIALOG_ID_PASSWORD,	XAP_QNXDialog_Password)
	DeclareDialog(XAP_DIALOG_ID_CLIPART,XAP_QNXDialog_ClipArt)
	DeclareDialog(XAP_DIALOG_ID_ENCODING,XAP_QNXDialog_Encoding)
	DeclareDialog(XAP_DIALOG_ID_IMAGE,XAP_QNXDialog_Image)
	DeclareDialog(XAP_DIALOG_ID_INSERT_FILE,XAP_QNXDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_HTMLOPTIONS,XAP_QNXDialog_HTMLOptions)

	DeclareDialog(AP_DIALOG_ID_REPLACE,			AP_QNXDialog_Replace)
	DeclareDialog(AP_DIALOG_ID_FIND,			AP_QNXDialog_Replace)
	DeclareDialog(AP_DIALOG_ID_HDRFTR,  		AP_QNXDialog_HdrFtr)
	DeclareDialog(AP_DIALOG_ID_BACKGROUND,		AP_QNXDialog_Background)
	DeclareDialog(AP_DIALOG_ID_GOTO,			AP_QNXDialog_Goto)
	DeclareDialog(AP_DIALOG_ID_BREAK,			AP_QNXDialog_Break)
	DeclareDialog(AP_DIALOG_ID_SPELL,			AP_QNXDialog_Spell)
	DeclareDialog(AP_DIALOG_ID_STYLES,			AP_QNXDialog_Styles)
	DeclareDialog(AP_DIALOG_ID_TAB,				AP_QNXDialog_Tab)
	DeclareDialog(AP_DIALOG_ID_PAGE_NUMBERS,	AP_QNXDialog_PageNumbers)
	DeclareDialog(AP_DIALOG_ID_FILE_PAGESETUP,	AP_QNXDialog_PageSetup)
	DeclareDialog(AP_DIALOG_ID_PARAGRAPH,		AP_QNXDialog_Paragraph)
	DeclareDialog(AP_DIALOG_ID_OPTIONS,			AP_QNXDialog_Options)
	DeclareDialog(AP_DIALOG_ID_INSERT_DATETIME, AP_QNXDialog_Insert_DateTime)
	DeclareDialog(AP_DIALOG_ID_INSERT_TABLE, AP_QNXDialog_InsertTable)
	DeclareDialog(AP_DIALOG_ID_WORDCOUNT,		AP_QNXDialog_WordCount)
	DeclareDialog(AP_DIALOG_ID_FIELD,			AP_QNXDialog_Field)
	DeclareDialog(AP_DIALOG_ID_LISTS,			AP_QNXDialog_Lists)
	DeclareDialog(AP_DIALOG_ID_COLUMNS,			AP_QNXDialog_Columns)
	DeclareDialog(AP_DIALOG_ID_TOGGLECASE,		AP_QNXDialog_ToggleCase)
	DeclareDialog(AP_DIALOG_ID_FILE_NEW,        AP_QNXDialog_New)
	DeclareDialog(XAP_DIALOG_ID_FILE_IMPORT, XAP_QNXDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_EXPORT, XAP_QNXDialog_FileOpenSaveAs)
	DeclareDialog(AP_DIALOG_ID_INSERTBOOKMARK,	AP_QNXDialog_InsertBookmark)
	DeclareDialog(AP_DIALOG_ID_INSERTHYPERLINK,	AP_QNXDialog_InsertHyperlink)
	DeclareDialog(AP_DIALOG_ID_MARK_REVISIONS,	AP_QNXDialog_MarkRevisions)
	DeclareDialog(AP_DIALOG_ID_LIST_REVISIONS,	AP_QNXDialog_ListRevisions)
	DeclareDialog(AP_DIALOG_ID_METADATA,		AP_QNXDialog_MetaData)
	DeclareDialog(AP_DIALOG_ID_MERGE_CELLS,	AP_QNXDialog_MergeCells)
	DeclareDialog(AP_DIALOG_ID_SPLIT_CELLS,	AP_QNXDialog_SplitCells)
	DeclareDialog(AP_DIALOG_ID_FORMAT_TABLE,	AP_QNXDialog_FormatTable)
	DeclareDialog(AP_DIALOG_ID_FORMAT_FRAME,	AP_QNXDialog_FormatFrame)
	DeclareDialog(AP_DIALOG_ID_FORMAT_FOOTNOTES,	AP_QNXDialog_FormatFootnotes)
	DeclareDialog(AP_DIALOG_ID_FORMAT_TOC,	AP_QNXDialog_FormatTOC)
#if 0
	DeclareDialog(AP_DIALOG_ID_DOWNLOAD_FILE,	AP_QNXDialog_Download_File)
#endif
	// ... also add new dialogs here ...

#endif



