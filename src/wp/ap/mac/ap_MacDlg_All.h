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

#	include "ap_MacDlg_Replace.h"

	// ... add new dialogs here ...

#else

	DeclareDialog(XAP_DIALOG_ID_MESSAGE_BOX,	AP_MacDialog_MessageBox)
	DeclareDialog(XAP_DIALOG_ID_FILE_OPEN,		AP_MacDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FILE_SAVEAS,	AP_MacDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_PRINT,			AP_MacDialog_Print)
	DeclareDialog(XAP_DIALOG_ID_PRINTTOFILE,	AP_MacDialog_FileOpenSaveAs)
	DeclareDialog(XAP_DIALOG_ID_FONT,			AP_MacDialog_FontChooser)

//	DeclareDialog(AP_DIALOG_ID_REPLACE,			AP_MacDialog_Replace)
//	DeclareDialog(AP_DIALOG_ID_FIND,			AP_MacDialog_Replace)
	
	// ... also add new dialogs here ...

#endif
