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

// Temporary "standard" sizes for some Gtk widgets.
// We need better cross-platform dialog layout guidelines.
#	define DEFAULT_BUTTON_WIDTH	85

#	include "xap_QNXDlg_MessageBox.h"
#	include "xap_QNXDlg_FileOpenSaveAs.h"
#	include "xap_QNXDlg_Print.h"
#	include "xap_QNXDlg_WindowMore.h"
#	include "xap_QNXDlg_FontChooser.h"
#	include "xap_QNXDlg_About.h"
#	include "xap_QNXDlg_Zoom.h"
#	include "xap_QNXDlg_Insert_Symbol.h"

#	include "ap_QNXDialog_Replace.h"
#	include "ap_QNXDialog_Break.h"
#	include "ap_QNXDialog_Goto.h"
#	include "ap_QNXDialog_Paragraph.h"
#	include "ap_QNXDialog_Options.h"
#	include "ap_QNXDialog_Spell.h"
#	include "ap_QNXDialog_Insert_DateTime.h"
#	include "ap_QNXDialog_WordCount.h"
#	include "ap_QNXDialog_Field.h"
#	include "ap_QNXDialog_Lists.h"
#	include "ap_QNXDialog_Columns.h"

	// ... add new dialogs here ...

#else

	DeclareDialog(XAP_DIALOG_ID_MESSAGE_BOX,	XAP_QNXDialog_MessageBox)
	DeclareDialog(XAP_DIALOG_ID_FILE_OPEN,		XAP_QNXDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVEAS,	XAP_QNXDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_PRINT,			XAP_QNXDialog_Print)
	DeclareDialog(XAP_DIALOG_ID_PRINTTOFILE,	XAP_QNXDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FONT,			XAP_QNXDialog_FontChooser)
	DeclareDialog(XAP_DIALOG_ID_WINDOWMORE,		XAP_QNXDialog_WindowMore)
	DeclareDialog(XAP_DIALOG_ID_ZOOM,			XAP_QNXDialog_Zoom)
	DeclareDialog(XAP_DIALOG_ID_INSERT_SYMBOL,	XAP_QNXDialog_Insert_Symbol)
	DeclareDialog(XAP_DIALOG_ID_INSERT_PICTURE,	XAP_QNXDialog_FileOpenSaveAs)

	DeclareDialog(XAP_DIALOG_ID_ABOUT,			XAP_QNXDialog_About)

	DeclareDialog(AP_DIALOG_ID_REPLACE,			AP_QNXDialog_Replace)
	DeclareDialog(AP_DIALOG_ID_FIND,			AP_QNXDialog_Replace)
	DeclareDialog(AP_DIALOG_ID_GOTO,			AP_QNXDialog_Goto)
	DeclareDialog(AP_DIALOG_ID_BREAK,			AP_QNXDialog_Break)
	DeclareDialog(AP_DIALOG_ID_SPELL,			AP_QNXDialog_Spell)
	DeclareDialog(AP_DIALOG_ID_PARAGRAPH,		AP_QNXDialog_Paragraph)	
	DeclareDialog(AP_DIALOG_ID_OPTIONS,			AP_QNXDialog_Options)	
	DeclareDialog(AP_DIALOG_ID_INSERT_DATETIME, AP_QNXDialog_Insert_DateTime)
	DeclareDialog(AP_DIALOG_ID_WORDCOUNT,		AP_QNXDialog_WordCount)
	DeclareDialog(AP_DIALOG_ID_FIELD,			AP_QNXDialog_Field)
	DeclareDialog(AP_DIALOG_ID_LISTS,			AP_QNXDialog_Lists)
	DeclareDialog(AP_DIALOG_ID_COLUMNS,			AP_QNXDialog_Columns)

	// ... also add new dialogs here ...

#endif



