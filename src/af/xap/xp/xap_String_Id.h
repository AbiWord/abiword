/* AbiSource Application Framework
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
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

/* Default name for new, untitled document */
dcl(UntitledDocument,		"Untitled%d")

/* Common to many dialogs */
dcl(DLG_OK,					"OK")
dcl(DLG_Cancel,				"Cancel")

/* Message box */
/* These are tagged "UnixMB" because the underscores precede accelerator
   characters.  It should be an ampersand on Windows, but Windows doesn't
   need a hand-constructed message box (Win32 API provides one).
*/
dcl(DLG_UnixMB_Yes,				"_Yes")
dcl(DLG_UnixMB_No,				"_No")
	
/* More Windows dialog */
dcl(DLG_MW_MoreWindows,		"Activate Window")
dcl(DLG_MW_Activate,		"Activate:")

/* Unix Font Selector dialog */
dcl(DLG_UFS_FontTitle,			"Font")
dcl(DLG_UFS_FontLabel,			"Font:")
dcl(DLG_UFS_StyleLabel,			"Style:")
dcl(DLG_UFS_SizeLabel,			"Size:")
dcl(DLG_UFS_EncodingLabel,		"Encoding:")
dcl(DLG_UFS_EffectsFrameLabel,	"Effects")
dcl(DLG_UFS_StrikeoutCheck,		"Strikeout")
dcl(DLG_UFS_UnderlineCheck,		"Underline")
dcl(DLG_UFS_FontTab,			"   Font   ")
dcl(DLG_UFS_ColorTab,			"   Color   ")
dcl(DLG_UFS_StyleRegular,		"Regular")
dcl(DLG_UFS_StyleItalic,		"Italic")
dcl(DLG_UFS_StyleBold,			"Bold")
dcl(DLG_UFS_StyleBoldItalic,	"Bold Italic")

/* Unix FileOpenSaveAs dialog */
dcl(DLG_FOSA_OpenTitle,					"Open File")
dcl(DLG_FOSA_SaveAsTitle,				"Save File As")
dcl(DLG_FOSA_PrintToFileTitle,			"Print To File")
dcl(DLG_FOSA_FileOpenTypeLabel,			"Open file as type:")
dcl(DLG_FOSA_FileSaveTypeLabel,			"Save file as type:")
dcl(DLG_FOSA_FilePrintTypeLabel,		"Print file as type:")
dcl(DLG_FOSA_FileTypeAutoDetect,		"Automatically Detected")	
dcl(DLG_InvalidPathname,				"Invalid pathname.")
dcl(DLG_NoSaveFile_DirNotExist,			"A directory in the given pathname does not exist.")
dcl(DLG_NoSaveFile_DirNotWriteable,		"The directory '%s' is write-protected.")
dcl(DLG_OverwriteFile,					"File already exists.  Overwrite file '%s'?")

/* Zoom dialog */
dcl(DLG_Zoom_ZoomTitle,					"Zoom")
dcl(DLG_Zoom_RadioFrameCaption,			"Zoom to")
dcl(DLG_Zoom_200,						"&200%")
dcl(DLG_Zoom_100,						"&100%")
dcl(DLG_Zoom_75,						"&75%")
dcl(DLG_Zoom_PageWidth,					"&Page width")
dcl(DLG_Zoom_WholePage,					"&Whole page")
dcl(DLG_Zoom_Percent,					"P&ercent:")	
dcl(DLG_Zoom_PreviewFrame,				"Preview")

/* Unix Print dialog */
dcl(DLG_UP_PrintTitle,					"Print")
dcl(DLG_UP_PrintTo,						"Print to: ")
dcl(DLG_UP_Printer,						"Printer")
dcl(DLG_UP_File,						"File")
dcl(DLG_UP_PrinterCommand,				"Printer command: ")
dcl(DLG_UP_PageRanges,					"Page ranges:")
dcl(DLG_UP_All,							"All")
dcl(DLG_UP_From,						"From: ")
dcl(DLG_UP_To,							" to ")
dcl(DLG_UP_Selection,					"Selection")
dcl(DLG_UP_Collate,						"Collate")
dcl(DLG_UP_Copies,						"Copies: ")
dcl(DLG_UP_PrintButton,					"Print")
dcl(DLG_UP_InvalidPrintString,			"The print command string is not valid.")
dcl(DLG_UP_PrintIn,						"Print in: ")
dcl(DLG_UP_BlackWhite,					"Black & White")
dcl(DLG_UP_Grayscale,					"Grayscale")
dcl(DLG_UP_Color,						"Color")
