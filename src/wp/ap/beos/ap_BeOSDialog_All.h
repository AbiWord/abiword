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

#ifndef AP_BEOSDIALOG_ALL_H

#	define AP_BEOSDIALOG_ALL_H

#include "xap_BeOSDlg_MessageBox.h"
#include "xap_BeOSDlg_FileOpenSaveAs.h"
#include "xap_BeOSDlg_Print.h"
#include "xap_BeOSDlg_FontChooser.h"
#include "xap_BeOSDlg_WindowMore.h"
#include "xap_BeOSDlg_Zoom.h"
#include "xap_BeOSDlg_About.h"
#include "xap_BeOSDlg_Insert_Symbol.h"

#include "ap_BeOSDialog_Replace.h"
#include "ap_BeOSDialog_Break.h"
#include "ap_BeOSDialog_Goto.h"
#include "ap_BeOSDialog_Paragraph.h"
#include "ap_BeOSDialog_Spell.h"
#include "ap_BeOSDialog_Options.h"
#include "ap_BeOSDialog_WordCount.h"
#include "ap_BeOSDialog_Insert_DateTime.h"
#include "ap_BeOSDialog_Field.h"
#include "ap_BeOSDialog_Tab.h"
#include "ap_BeOSDialog_Columns.h" 
#include "ap_BeOSDialog_PageNumbers.h"

	// ... add new dialogs here ...

#else

	DeclareDialog(XAP_DIALOG_ID_MESSAGE_BOX,	XAP_BeOSDialog_MessageBox)
	DeclareDialog(XAP_DIALOG_ID_FILE_OPEN,		XAP_BeOSDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVEAS,	XAP_BeOSDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_PRINT,			XAP_BeOSDialog_Print)
	DeclareDialog(XAP_DIALOG_ID_PRINTTOFILE,	XAP_BeOSDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FONT,			XAP_BeOSDialog_FontChooser)
	DeclareDialog(XAP_DIALOG_ID_WINDOWMORE,		XAP_BeOSDialog_WindowMore) 
	DeclareDialog(XAP_DIALOG_ID_ZOOM,			XAP_BeOSDialog_Zoom) 
	DeclareDialog(XAP_DIALOG_ID_ABOUT,			XAP_BeOSDialog_About) 
	DeclareDialog(XAP_DIALOG_ID_INSERT_SYMBOL,		XAP_BeOSDialog_Insert_Symbol) 
	DeclareDialog(XAP_DIALOG_ID_INSERT_PICTURE,	XAP_BeOSDialog_FileOpenSaveAs)
	DeclareDialog(AP_DIALOG_ID_REPLACE,			AP_BeOSDialog_Replace)
	DeclareDialog(AP_DIALOG_ID_FIND,			AP_BeOSDialog_Replace)
	DeclareDialog(AP_DIALOG_ID_BREAK,			AP_BeOSDialog_Break)
    DeclareDialog(AP_DIALOG_ID_GOTO,            AP_BeOSDialog_Goto)
	DeclareDialog(AP_DIALOG_ID_SPELL,			AP_BeOSDialog_Spell)
	DeclareDialog(AP_DIALOG_ID_PARAGRAPH,		AP_BeOSDialog_Paragraph)
	DeclareDialog(AP_DIALOG_ID_OPTIONS,			AP_BeOSDialog_Options)
	DeclareDialog(AP_DIALOG_ID_WORDCOUNT,		AP_BeOSDialog_WordCount)
	DeclareDialog(AP_DIALOG_ID_INSERT_DATETIME,	AP_BeOSDialog_Insert_DateTime)
	DeclareDialog(AP_DIALOG_ID_FIELD, 			AP_BeOSDialog_Field)
	DeclareDialog(AP_DIALOG_ID_TAB,				AP_BeOSDialog_Tab)
	DeclareDialog(AP_DIALOG_ID_COLUMNS,			AP_BeOSDialog_Columns)
	DeclareDialog(AP_DIALOG_ID_PAGE_NUMBERS,		AP_BeOSDialog_PageNumbers)
	// ... also add new dialogs here ...

#endif
