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

#ifndef AP_WIN32DIALOG_ALL_H

#	define AP_WIN32DIALOG_ALL_H

#	include "xap_Win32Dlg_MessageBox.h"
#	include "xap_Win32Dlg_FileOpenSaveAs.h"
#	include "xap_Win32Dlg_Print.h"
#	include "xap_Win32Dlg_FontChooser.h"

#	include "ap_Win32Dialog_Replace.h"
#	include "ap_Win32Dialog_About.h"

	// ... add new dialogs here ...

#else

	DeclareDialog(XAP_DIALOG_ID_MESSAGE_BOX,	AP_Win32Dialog_MessageBox)
	DeclareDialog(XAP_DIALOG_ID_FILE_OPEN,		AP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVEAS,	AP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_PRINT,			AP_Win32Dialog_Print)
	DeclareDialog(XAP_DIALOG_ID_PRINTTOFILE,	AP_Win32Dialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FONT,			AP_Win32Dialog_FontChooser)

	DeclareDialog(AP_DIALOG_ID_REPLACE,			AP_Win32Dialog_Replace)
	DeclareDialog(AP_DIALOG_ID_FIND,			AP_Win32Dialog_Replace)
	DeclareDialog(AP_DIALOG_ID_ABOUT,			AP_Win32Dialog_Replace)
	
	// ... also add new dialogs here ...

#endif
