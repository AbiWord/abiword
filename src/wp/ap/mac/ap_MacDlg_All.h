/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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

#ifndef AP_MACDIALOG_ALL_H

#	define AP_MACDIALOG_ALL_H

#	include "xap_MacDlg_MessageBox.h"
#	include "xap_MacDlg_FileOpenSaveAs.h"
#	include "xap_MacDlg_Print.h"
#	include "xap_MacDlg_FontChooser.h"
#	include "xap_MacDlg_WindowMore.h"
#	include "xap_MacDlg_About.h"
#   include "xap_MacDlg_Insert_Symbol.h"
#	include "xap_MacDlg_Language.h"

#	include "ap_MacDlg_Replace.h"
#	include "ap_MacDlg_Break.h"

	// ... add new dialogs here ...

#else

	DeclareDialog(XAP_DIALOG_ID_MESSAGE_BOX,	XAP_MacDialog_MessageBox)
	DeclareDialog(XAP_DIALOG_ID_FILE_OPEN,		XAP_MacDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVEAS,	XAP_MacDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_PRINT,			XAP_MacDialog_Print)
	DeclareDialog(XAP_DIALOG_ID_PRINTTOFILE,	XAP_MacDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FONT,			XAP_MacDialog_FontChooser)
	DeclareDialog(XAP_DIALOG_ID_LANGUAGE,			XAP_MacDialog_Language)
	DeclareDialog(XAP_DIALOG_ID_WINDOWMORE,		XAP_MacDialog_WindowMore)
	DeclareDialog(XAP_DIALOG_ID_INSERT_SYMBOL,	XAP_MacDialog_Insert_Symbol)
	DeclareDialog(XAP_DIALOG_ID_INSERT_PICTURE,	XAP_MacDialog_FileOpenSaveAs)
//	DeclareDialog(XAP_DIALOG_ID_ABOUT,			XAP_MacDialog_About)

//	DeclareDialog(AP_DIALOG_ID_REPLACE,			AP_MacDialog_Replace)
//	DeclareDialog(AP_DIALOG_ID_FIND,			AP_MacDialog_Replace)
	DeclareDialog(AP_DIALOG_ID_BREAK,			AP_MacDialog_Break)
	
	// ... also add new dialogs here ...

#endif
