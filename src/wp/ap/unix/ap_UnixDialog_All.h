/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef AP_UNIXDIALOG_ALL_H

#	define AP_UNIXDIALOG_ALL_H

// Temporary "standard" sizes for some Gtk widgets.
// We need better cross-platform dialog layout guidelines.
#	define DEFAULT_BUTTON_WIDTH	85

#	include "xap_UnixDlg_MessageBox.h"
#	include "xap_UnixDlg_FileOpenSaveAs.h"
#	include "xap_UnixDlg_Print.h"
#	include "xap_UnixDlg_WindowMore.h"
#	include "xap_UnixDlg_FontChooser.h"
#	include "xap_UnixDlg_About.h"
#	include "xap_UnixDlg_Zoom.h"
#	include "xap_UnixDlg_Insert_Symbol.h"

#	include "ap_UnixDialog_Replace.h"
#	include "ap_UnixDialog_Break.h"
#	include "ap_UnixDialog_Goto.h"
#	include "ap_UnixDialog_Paragraph.h"
#	include "ap_UnixDialog_Options.h"
#	include "ap_UnixDialog_Spell.h"
#	include "ap_UnixDialog_Insert_DateTime.h"
#	include "ap_UnixDialog_WordCount.h"
#	include "ap_UnixDialog_Field.h"
#	include "ap_UnixDialog_Lists.h"
#	include "ap_UnixDialog_Columns.h"

#ifdef HAVE_GNOME
#	include "xap_UnixGnomeDlg_About.h"
#	include "xap_UnixGnomeDlg_FontChooser.h"
#	include "xap_UnixGnomeDlg_Insert_Symbol.h"
#	include "xap_UnixGnomeDlg_MessageBox.h"
#	include "xap_UnixGnomeDlg_Zoom.h"

#	include "ap_UnixGnomeDialog_Replace.h"
#	include "ap_UnixGnomeDialog_Break.h"
#	include "ap_UnixGnomeDialog_Goto.h"
#	include "ap_UnixGnomeDialog_Paragraph.h"
#	include "ap_UnixGnomeDialog_Options.h"
#	include "ap_UnixGnomeDialog_WordCount.h"
#	include "ap_UnixGnomeDialog_Field.h"

#endif

	// ... add new dialogs here ...

#else

#   ifdef HAVE_GNOME
	DeclareDialog(XAP_DIALOG_ID_MESSAGE_BOX,	XAP_UnixGnomeDialog_MessageBox)
	DeclareDialog(XAP_DIALOG_ID_FILE_OPEN,		XAP_UnixDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVEAS,	XAP_UnixDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_PRINT,			XAP_UnixDialog_Print)
	DeclareDialog(XAP_DIALOG_ID_PRINTTOFILE,	XAP_UnixDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FONT,			XAP_UnixGnomeDialog_FontChooser)
	DeclareDialog(XAP_DIALOG_ID_WINDOWMORE,		XAP_UnixDialog_WindowMore)
	DeclareDialog(XAP_DIALOG_ID_ZOOM,			XAP_UnixGnomeDialog_Zoom)
	DeclareDialog(XAP_DIALOG_ID_ABOUT,			XAP_UnixGnomeDialog_About)
	DeclareDialog(XAP_DIALOG_ID_INSERT_SYMBOL,  XAP_UnixGnomeDialog_Insert_Symbol)
	DeclareDialog(XAP_DIALOG_ID_INSERT_PICTURE,	XAP_UnixDialog_FileOpenSaveAs)

	DeclareDialog(AP_DIALOG_ID_REPLACE,			AP_UnixGnomeDialog_Replace)
	DeclareDialog(AP_DIALOG_ID_FIND,			AP_UnixGnomeDialog_Replace)
	DeclareDialog(AP_DIALOG_ID_GOTO,			AP_UnixGnomeDialog_Goto)
	DeclareDialog(AP_DIALOG_ID_BREAK,			AP_UnixGnomeDialog_Break)
	DeclareDialog(AP_DIALOG_ID_SPELL,			AP_UnixDialog_Spell)
	DeclareDialog(AP_DIALOG_ID_PARAGRAPH,		AP_UnixGnomeDialog_Paragraph)	
	DeclareDialog(AP_DIALOG_ID_OPTIONS,			AP_UnixGnomeDialog_Options)
	DeclareDialog(AP_DIALOG_ID_INSERT_DATETIME,	AP_UnixDialog_Insert_DateTime)			
	DeclareDialog(AP_DIALOG_ID_WORDCOUNT,       AP_UnixGnomeDialog_WordCount)
	DeclareDialog(AP_DIALOG_ID_FIELD,			AP_UnixGnomeDialog_Field)
	DeclareDialog(AP_DIALOG_ID_LISTS,		     AP_UnixDialog_Lists)
	DeclareDialog(AP_DIALOG_ID_COLUMNS,			AP_UnixDialog_Columns)

	// ... also add new dialogs here ...
#   else
	DeclareDialog(XAP_DIALOG_ID_MESSAGE_BOX,	XAP_UnixDialog_MessageBox)
	DeclareDialog(XAP_DIALOG_ID_FILE_OPEN,		XAP_UnixDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVEAS,	XAP_UnixDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_PRINT,			XAP_UnixDialog_Print)
	DeclareDialog(XAP_DIALOG_ID_PRINTTOFILE,	XAP_UnixDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FONT,			XAP_UnixDialog_FontChooser)
	DeclareDialog(XAP_DIALOG_ID_WINDOWMORE,		XAP_UnixDialog_WindowMore)
	DeclareDialog(XAP_DIALOG_ID_ZOOM,			XAP_UnixDialog_Zoom)
	DeclareDialog(XAP_DIALOG_ID_INSERT_SYMBOL,  XAP_UnixDialog_Insert_Symbol)
	DeclareDialog(XAP_DIALOG_ID_INSERT_PICTURE,	XAP_UnixDialog_FileOpenSaveAs)

	DeclareDialog(XAP_DIALOG_ID_ABOUT,			XAP_UnixDialog_About)

	DeclareDialog(AP_DIALOG_ID_REPLACE,			AP_UnixDialog_Replace)
	DeclareDialog(AP_DIALOG_ID_FIND,			AP_UnixDialog_Replace)
	DeclareDialog(AP_DIALOG_ID_GOTO,			AP_UnixDialog_Goto)
	DeclareDialog(AP_DIALOG_ID_BREAK,			AP_UnixDialog_Break)
	DeclareDialog(AP_DIALOG_ID_SPELL,			AP_UnixDialog_Spell)
	DeclareDialog(AP_DIALOG_ID_PARAGRAPH,		AP_UnixDialog_Paragraph)	
	DeclareDialog(AP_DIALOG_ID_OPTIONS,			AP_UnixDialog_Options)
	DeclareDialog(AP_DIALOG_ID_INSERT_DATETIME,	AP_UnixDialog_Insert_DateTime)		
	DeclareDialog(AP_DIALOG_ID_WORDCOUNT,		AP_UnixDialog_WordCount)
	DeclareDialog(AP_DIALOG_ID_FIELD,			AP_UnixDialog_Field)
	DeclareDialog(AP_DIALOG_ID_LISTS,			AP_UnixDialog_Lists)
	DeclareDialog(AP_DIALOG_ID_COLUMNS,			AP_UnixDialog_Columns)

	// ... also add new dialogs here ...
#   endif

#endif



