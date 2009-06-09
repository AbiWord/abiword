/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
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
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

#ifndef XAP_STRING_ID_H
#define XAP_STRING_ID_H
#define XAP_STRING_ID_MSG_ImportingDoc  "Importing Document..."
#define XAP_STRING_ID_MSG_BuildingDoc  "Building Document:"
#define XAP_STRING_ID_MSG_AutoRevision  "Autorevision"
#define XAP_STRING_ID_MSG_HistoryPartRestore1  "AbiWord cannot fully restore version %d of the document because the version information is incomplete."
#define XAP_STRING_ID_MSG_HistoryPartRestore2  "The nearest version that can be restored fully is %d. Would you like to restore this version instead? To partially restore version %d press No."
#define XAP_STRING_ID_MSG_HistoryPartRestore3  "To continue anyway, press OK."
#define XAP_STRING_ID_MSG_HistoryPartRestore4  "To quit the restoration attempt, press Cancel."
#define XAP_STRING_ID_MSG_HistoryNoRestore  "AbiWord cannot restore version %d of the document because the version information is missing."
#define XAP_STRING_ID_MSG_HistoryConfirmSave  "You have to save changes to document %s before proceeding. Save now?"

#define XAP_STRING_ID_MSG_NoUndo  "This operation cannot be undone. Are you sure you want to proceed?"

/* Default name for new, untitled document */
#define XAP_STRING_ID_UntitledDocument  "Untitled%d"
#define XAP_STRING_ID_ReadOnly  "Read-Only"

/*  Styles */
#define XAP_STRING_ID_STYLE_NUMBER_LIST  "Numbered List"
#define XAP_STRING_ID_STYLE_PLAIN_TEXT  "Plain Text"
#define XAP_STRING_ID_STYLE_NORMAL  "Normal"
#define XAP_STRING_ID_STYLE_HEADING1  "Heading 1"
#define XAP_STRING_ID_STYLE_HEADING2  "Heading 2"
#define XAP_STRING_ID_STYLE_HEADING3  "Heading 3"
#define XAP_STRING_ID_STYLE_HEADING4  "Heading 4"
#define XAP_STRING_ID_STYLE_TOCHEADING  "Contents Header"
#define XAP_STRING_ID_STYLE_TOCHEADING1  "Contents 1"
#define XAP_STRING_ID_STYLE_TOCHEADING2  "Contents 2"
#define XAP_STRING_ID_STYLE_TOCHEADING3  "Contents 3"
#define XAP_STRING_ID_STYLE_TOCHEADING4  "Contents 4"
#define XAP_STRING_ID_STYLE_BLOCKTEXT  "Block Text"          
#define XAP_STRING_ID_STYLE_LOWERCASELIST  "Lower Case List"     
#define XAP_STRING_ID_STYLE_UPPERCASTELIST  "Upper Case List"     
#define XAP_STRING_ID_STYLE_LOWERROMANLIST  "Lower Roman List"    
#define XAP_STRING_ID_STYLE_UPPERROMANLIST  "Upper Roman List"    
#define XAP_STRING_ID_STYLE_BULLETLIST  "Bullet List"         
#define XAP_STRING_ID_STYLE_DASHEDLIST  "Dashed List"         
#define XAP_STRING_ID_STYLE_SQUARELIST  "Square List"         
#define XAP_STRING_ID_STYLE_TRIANGLELIST  "Triangle List"       
#define XAP_STRING_ID_STYLE_DIAMONLIST  "Diamond List"	      
#define XAP_STRING_ID_STYLE_STARLIST  "Star List"           
#define XAP_STRING_ID_STYLE_TICKLIST  "Tick List"           
#define XAP_STRING_ID_STYLE_BOXLIST  "Box List"            
#define XAP_STRING_ID_STYLE_HANDLIST  "Hand List"           
#define XAP_STRING_ID_STYLE_HEARTLIST  "Heart List"          
#define XAP_STRING_ID_STYLE_CHAPHEADING  "Chapter Heading"     
#define XAP_STRING_ID_STYLE_SECTHEADING  "Section Heading"     
#define XAP_STRING_ID_STYLE_ENDREFERENCE  "Endnote Reference"   
#define XAP_STRING_ID_STYLE_ENDTEXT  "Endnote Text"        
#define XAP_STRING_ID_STYLE_FOOTREFERENCE  "Footnote Reference"   
#define XAP_STRING_ID_STYLE_FOOTTEXT  "Footnote Text"        
#define XAP_STRING_ID_STYLE_NUMHEAD1  "Numbered Heading 1"  
#define XAP_STRING_ID_STYLE_NUMHEAD2  "Numbered Heading 2"  
#define XAP_STRING_ID_STYLE_NUMHEAD3  "Numbered Heading 3"  
#define XAP_STRING_ID_STYLE_IMPLIES_LIST  "Implies List"  


/* Common to many dialogs */
#define XAP_STRING_ID_DLG_OK  "OK"
#define XAP_STRING_ID_DLG_Cancel  "Cancel"
#define XAP_STRING_ID_DLG_Close  "Close"
#define XAP_STRING_ID_DLG_Insert  "&Insert"
#define XAP_STRING_ID_DLG_Update  "Update"
#define XAP_STRING_ID_DLG_Apply  "Apply"
#define XAP_STRING_ID_DLG_Delete  "Delete"
#define XAP_STRING_ID_DLG_Compare  "Compare"
#define XAP_STRING_ID_DLG_Select  "Select"
#define XAP_STRING_ID_DLG_Merge  "Merge"
#define XAP_STRING_ID_DLG_Show  "Show"
#define XAP_STRING_ID_DLG_Restore  "Restore"
	
/* Units */
#define XAP_STRING_ID_DLG_Unit_inch  "inch"
#define XAP_STRING_ID_DLG_Unit_cm  "cm"
#define XAP_STRING_ID_DLG_Unit_mm  "mm"
#define XAP_STRING_ID_DLG_Unit_points  "points"
#define XAP_STRING_ID_DLG_Unit_pica  "pica"

/* Message box */
/* These are tagged "UnixMB" because the underscores precede accelerator
   #define XAP_STRING_ID_acters.	It should be an ampersand on Windows, but Windows doesn't
   need a hand-constructed message box (Win32 API provides one).
*/
#define XAP_STRING_ID_DLG_UnixMB_Yes  "_Yes"
#define XAP_STRING_ID_DLG_UnixMB_No  "_No"

#define XAP_STRING_ID_DLG_MB_Yes  "Yes"
#define XAP_STRING_ID_DLG_MB_No  "No"

/* More Windows dialog */
#define XAP_STRING_ID_DLG_MW_MoreWindows  "View Document"
#define XAP_STRING_ID_DLG_MW_Activate  "View:"
#define XAP_STRING_ID_DLG_MW_AvailableDocuments  "Available Documents"
#define XAP_STRING_ID_DLG_MW_ViewButton  "&View"

/* Remove Toolbar Icon */
#define XAP_STRING_ID_DLG_Remove_Icon  "Do you want to remove this icon from the toolbar?"

/* Font Selector dialog */
#define XAP_STRING_ID_DLG_UFS_FontTitle  "Font"
#define XAP_STRING_ID_DLG_UFS_FontLabel  "Font:"
#define XAP_STRING_ID_DLG_UFS_StyleLabel  "Style:"
#define XAP_STRING_ID_DLG_UFS_SizeLabel  "Size:"
#define XAP_STRING_ID_DLG_UFS_EncodingLabel  "Encoding:"
#define XAP_STRING_ID_DLG_UFS_EffectsFrameLabel  "Effects"
#define XAP_STRING_ID_DLG_UFS_StrikeoutCheck  "Strike"
#define XAP_STRING_ID_DLG_UFS_UnderlineCheck  "Underline"
#define XAP_STRING_ID_DLG_UFS_OverlineCheck  "Overline"
#define XAP_STRING_ID_DLG_UFS_HiddenCheck  "Hidden"	
#define XAP_STRING_ID_DLG_UFS_TransparencyCheck  "Set no Highlight Color"
#define XAP_STRING_ID_DLG_UFS_FontTab  "Font"
#define XAP_STRING_ID_DLG_UFS_ColorTab  "Text Color"
#define XAP_STRING_ID_DLG_UFS_BGColorTab  "HighLight Color"
#define XAP_STRING_ID_DLG_UFS_StyleRegular  "Regular"
#define XAP_STRING_ID_DLG_UFS_StyleItalic  "Italic"
#define XAP_STRING_ID_DLG_UFS_StyleBold  "Bold"
#define XAP_STRING_ID_DLG_UFS_StyleBoldItalic  "Bold Italic"
#define XAP_STRING_ID_DLG_UFS_ToplineCheck  "Topline"
#define XAP_STRING_ID_DLG_UFS_BottomlineCheck  "Bottomline"
#define XAP_STRING_ID_DLG_UFS_ColorLabel  "Color:"
#define XAP_STRING_ID_DLG_UFS_ScriptLabel  "Script:"
#define XAP_STRING_ID_DLG_UFS_SampleFrameLabel  "Sample"
#define XAP_STRING_ID_DLG_UFS_SuperScript  "Superscript"
#define XAP_STRING_ID_DLG_UFS_SubScript  "Subscript"

/* Unix FileOpenSaveAs dialog */
#define XAP_STRING_ID_DLG_FOSA_OpenTitle  "Open File"
#define XAP_STRING_ID_DLG_FOSA_SaveAsTitle  "Save File As"
#define XAP_STRING_ID_DLG_FOSA_ExportTitle  "Export File"
#define XAP_STRING_ID_DLG_FOSA_ImportTitle  "Import File"
#define XAP_STRING_ID_DLG_FOSA_InsertTitle  "Insert File"
#define XAP_STRING_ID_DLG_FOSA_InsertMath  "Insert Math File"
#define XAP_STRING_ID_DLG_FOSA_InsertObject  "Insert Embeddable Object"
#define XAP_STRING_ID_DLG_FOSA_PrintToFileTitle  "Print To File"
#define XAP_STRING_ID_DLG_FOSA_RecordToFileTitle  "Record Editing to File"
#define XAP_STRING_ID_DLG_FOSA_ReplayFromFileTitle  "Replay Editing from File"
#define XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel  "Open file as type:"
#define XAP_STRING_ID_DLG_FOSA_FileInsertMath  "Insert MathML file:"
#define XAP_STRING_ID_DLG_FOSA_FileInsertObject  "Insert Embeddable Object file:"
#define XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel  "Save file as type:"
#define XAP_STRING_ID_DLG_FOSA_FilePrintTypeLabel  "Print file as type:"
#define XAP_STRING_ID_DLG_FOSA_RecordToFileLabel  "File to record editing:"
#define XAP_STRING_ID_DLG_FOSA_ReplayFromFileLabel  "File to replay editing:"
#define XAP_STRING_ID_DLG_FOSA_FileTypeAutoDetect  "Automatically Detected"
#define XAP_STRING_ID_DLG_FOSA_ALLIMAGES  "All Image Files"
#define XAP_STRING_ID_DLG_FOSA_ALLDOCS  "All Documents"
#define XAP_STRING_ID_DLG_FOSA_ALL  "All (*.*)"
#define XAP_STRING_ID_DLG_InvalidPathname  "Invalid pathname."
#define XAP_STRING_ID_DLG_NoSaveFile_DirNotExist  "A directory in the given pathname does not exist."
#define XAP_STRING_ID_DLG_NoSaveFile_DirNotWriteable  "The directory '%s' is write-protected."
#define XAP_STRING_ID_DLG_OverwriteFile  "File already exists.  Overwrite file '%s'?"
#define XAP_STRING_ID_DLG_FOSA_ExtensionDoesNotMatch  "The given file extension does not match the chosen file type. Do you want to use this name anyway?"

/* Password dialog */
#define XAP_STRING_ID_DLG_Password_Title  "Enter Password"
#define XAP_STRING_ID_DLG_Password_Password  "Password:"

/* Zoom dialog */
#define XAP_STRING_ID_DLG_Zoom_ZoomTitle  "Zoom"
#define XAP_STRING_ID_DLG_Zoom_RadioFrameCaption  "Zoom to"
#define XAP_STRING_ID_DLG_Zoom_200  "&200%"
#define XAP_STRING_ID_DLG_Zoom_100  "&100%"
#define XAP_STRING_ID_DLG_Zoom_75  "&75%"
#define XAP_STRING_ID_DLG_Zoom_PageWidth  "&Page width"
#define XAP_STRING_ID_DLG_Zoom_WholePage  "&Whole page"
#define XAP_STRING_ID_DLG_Zoom_Percent  "P&ercent:"
#define XAP_STRING_ID_DLG_Zoom_PreviewFrame  "Preview"

/* Zoom tool bar -- Truncated to fit small combobox size*/
#define XAP_STRING_ID_TB_Zoom_PageWidth  "Page Width"
#define XAP_STRING_ID_TB_Zoom_WholePage  "Whole Page"
#define XAP_STRING_ID_TB_Zoom_Percent  "Other..."

/* Font tool bar*/
#define XAP_STRING_ID_TB_Font_Symbol  "Symbols"

/* Unix Print dialog */
#define XAP_STRING_ID_DLG_UP_PrintTitle  "Print"
#define XAP_STRING_ID_DLG_UP_PrintPreviewTitle  "AbiWord: Print Preview"
#define XAP_STRING_ID_DLG_UP_PrintTo  "Print to: "
#define XAP_STRING_ID_DLG_UP_Printer  "Printer"
#define XAP_STRING_ID_DLG_UP_File  "File"
#define XAP_STRING_ID_DLG_UP_PrinterCommand  "Printer command: "
#define XAP_STRING_ID_DLG_UP_PageRanges  "Page ranges:"
#define XAP_STRING_ID_DLG_UP_All  "All"
#define XAP_STRING_ID_DLG_UP_From  "From: "
#define XAP_STRING_ID_DLG_UP_To  " to "
#define XAP_STRING_ID_DLG_UP_Selection  "Selection"
#define XAP_STRING_ID_DLG_UP_Collate  "Collate"
#define XAP_STRING_ID_DLG_UP_EmbedFonts  "Embed Fonts"
#define XAP_STRING_ID_DLG_UP_Copies  "Copies: "
#define XAP_STRING_ID_DLG_UP_PrintButton  "Print"
#define XAP_STRING_ID_DLG_UP_InvalidPrintString  "The print command string is not valid."
#define XAP_STRING_ID_DLG_UP_PrintIn  "Print in: "
#define XAP_STRING_ID_DLG_UP_BlackWhite  "Black & White"
#define XAP_STRING_ID_DLG_UP_Grayscale  "Grayscale"
#define XAP_STRING_ID_DLG_UP_Color  "Color"

/* Insert Symbol dialog */
#define XAP_STRING_ID_DLG_Insert_SymbolTitle  "Insert Symbol"

/* Insert Picture Preview Dialog (Win32) */
#define XAP_STRING_ID_DLG_IP_Title  "Insert Picture"
#define XAP_STRING_ID_DLG_IP_Activate_Label  "Preview Picture"
#define XAP_STRING_ID_DLG_IP_No_Picture_Label  "No Picture"
#define XAP_STRING_ID_DLG_IP_Height_Label  "Height: "
#define XAP_STRING_ID_DLG_IP_Width_Label  "Width:  "
#define XAP_STRING_ID_DLG_IP_Button_Label  "Insert"

/* Plugin dialog */
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_TITLE  "AbiWord Plugin Manager"
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_ACTIVE  "Active Plugins"
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE  "Deactivate plugin"
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE_ALL  "Deactivate all plugins"
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_INSTALL  "Install new plugin"
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_LIST  "Plugin List"
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_NAME  "Name:"
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_DESC  "Description:"
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_AUTHOR  "Author:"
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_VERSION  "Version:"
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_DETAILS  "Plugin Details:"
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_NOT_AVAILABLE  "Not available"

/* spellchecker */
#define XAP_STRING_ID_SPELL_CANTLOAD_DICT  "Could not load the dictionary for the %s language"
#define XAP_STRING_ID_SPELL_CANTLOAD_DLL  "AbiWord cannot find the spelling file %s.dll\nPlease download and install Aspell from http://aspell.net/win32/"

/* plugin error messages */
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_LOAD  "Could not activate/load plugin"
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_UNLOAD  "Could not deactivate plugin"
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_NONE_SELECTED  "No plugin selected"

/* Language Dialog */
#define XAP_STRING_ID_DLG_ULANG_LangTitle  "Set Language"
#define XAP_STRING_ID_DLG_ULANG_LangLabel  "Select Language:"
#define XAP_STRING_ID_DLG_ULANG_AvailableLanguages  "Available Languages"
#define XAP_STRING_ID_DLG_ULANG_SetLangButton  "&Set Language"
#define XAP_STRING_ID_DLG_ULANG_DefaultLangLabel  "Default language: "
#define XAP_STRING_ID_DLG_ULANG_DefaultLangChkbox  "Make default for document"
	
/* ClipArt Dialog */
#define XAP_STRING_ID_DLG_CLIPART_Title  "Clip Art"
#define XAP_STRING_ID_DLG_CLIPART_Loading  "Loading Clip Art"
#define XAP_STRING_ID_DLG_CLIPART_Error  "Clip Art could not be loaded"

/* About Dialog */
#define XAP_STRING_ID_DLG_ABOUT_Title  "About %s"

/* image size dialog */
#define XAP_STRING_ID_DLG_Image_Title  "Image Properties"
#define XAP_STRING_ID_DLG_Image_Width  "Width:"
#define XAP_STRING_ID_DLG_Image_Height  "Height:"
#define XAP_STRING_ID_DLG_Image_Aspect  "Preserve aspect ratio"
#define XAP_STRING_ID_DLG_Image_LblTitle  "Title:"
#define XAP_STRING_ID_DLG_Image_LblDescription  "Description:"
#define XAP_STRING_ID_DLG_Image_ImageSize  "Set Image Size"
#define XAP_STRING_ID_DLG_Image_ImageDesc  "Set Image Name"
#define XAP_STRING_ID_DLG_Image_TextWrapping  "Define Text Wrapping"
#define XAP_STRING_ID_DLG_Image_Placement  "Define Image Placement"
#define XAP_STRING_ID_DLG_Image_InLine  "Image placed in-line (no text wrapping)"
#define XAP_STRING_ID_DLG_Image_WrappedNone  "Image floats above text"
#define XAP_STRING_ID_DLG_Image_WrappedRight  "Text wrapped to the Right of the Image"
#define XAP_STRING_ID_DLG_Image_WrappedLeft  "Text wrapped to the Left of the Image"
#define XAP_STRING_ID_DLG_Image_WrappedBoth  "Text wrapped on both sides of the Image"
#define XAP_STRING_ID_DLG_Image_PlaceParagraph  "Position relative to nearest paragraph"
#define XAP_STRING_ID_DLG_Image_PlaceColumn  "Position relative to its Column"
#define XAP_STRING_ID_DLG_Image_PlacePage  "Position relative to its Page"
#define XAP_STRING_ID_DLG_Image_WrapType  "Type of text wrapping"
#define XAP_STRING_ID_DLG_Image_SquareWrap  "Square text wrapping"
#define XAP_STRING_ID_DLG_Image_TightWrap  "Tight text wrapping"

/* ListDocuments Dialog */
#define XAP_STRING_ID_DLG_LISTDOCS_Title  "Opened Documents"
#define XAP_STRING_ID_DLG_LISTDOCS_Heading1  "Choose document from the list:"
	
/*
For insert Table widget
*/
#define XAP_STRING_ID_TB_InsertNewTable  "Insert New Table"
#define XAP_STRING_ID_TB_Table  "Table"
#define XAP_STRING_ID_TB_ClearBackground  "Clear Background"

/*
	  Language property in different languages; alphabetical except English first.
	  Please when translating the first entry, put it also into parenthesis
	  or, surround it by some other #define XAP_STRING_ID_acters, so that it will appear on the top
	  of the list when sorted alphabetically.
*/
#define XAP_STRING_ID_LANG_0  "(no proofing)"
#define XAP_STRING_ID_LANG_EN_AU  "English (Australia)"
#define XAP_STRING_ID_LANG_EN_CA  "English (Canada)"
#define XAP_STRING_ID_LANG_EN_GB  "English (UK)"
#define XAP_STRING_ID_LANG_EN_IE  "English (Ireland)"
#define XAP_STRING_ID_LANG_EN_NZ  "English (New Zealand)"
#define XAP_STRING_ID_LANG_EN_ZA  "English (South Africa)"
#define XAP_STRING_ID_LANG_EN_US  "English (US)"
#define XAP_STRING_ID_LANG_AF_ZA  "Afrikaans"
#define XAP_STRING_ID_LANG_AK_GH  "Akan"	// 
#define XAP_STRING_ID_LANG_SQ_AL  "Albanian"	// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_AM_ET  "Amharic (Ethiopia)"
#define XAP_STRING_ID_LANG_AR  "Arabic"
#define XAP_STRING_ID_LANG_AR_EG  "Arabic (Egypt)"
#define XAP_STRING_ID_LANG_AR_SA  "Arabic (Saudi Arabia)"
#define XAP_STRING_ID_LANG_HY_AM  "Armenian"
#define XAP_STRING_ID_LANG_AS_IN  "Assamese"
#define XAP_STRING_ID_LANG_AST_ES  "Asturian (Spain)"
#define XAP_STRING_ID_LANG_AYM_BO  "Aymara (La Paz)"
#define XAP_STRING_ID_LANG_AYC_BO  "Aymara (Oruro)"
#define XAP_STRING_ID_LANG_AYR  "Central Aymara"
#define XAP_STRING_ID_LANG_EU_ES  "Basque"
#define XAP_STRING_ID_LANG_BE_BY  "Belarusian"
#define XAP_STRING_ID_LANG_BE_LATIN  "Belarusian, Latin"
#define XAP_STRING_ID_LANG_BN_IN  "Bengali"
#define XAP_STRING_ID_LANG_BR_FR  "Breton"		// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_BG_BG  "Bulgarian"
#define XAP_STRING_ID_LANG_CA_ES  "Catalan"
#define XAP_STRING_ID_LANG_KW_GB  "Cornish"		// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_CO_FR  "Corsican"		// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_HR_HR  "Croatian"
#define XAP_STRING_ID_LANG_ZH_HK  "Chinese (Hong Kong)"
#define XAP_STRING_ID_LANG_ZH_CN  "Chinese (PRC)"
#define XAP_STRING_ID_LANG_ZH_SG  "Chinese (Singapore)"
#define XAP_STRING_ID_LANG_ZH_TW  "Chinese (Taiwan)"
#define XAP_STRING_ID_LANG_COP_EG  "Coptic"
#define XAP_STRING_ID_LANG_CS_CZ  "Czech"
#define XAP_STRING_ID_LANG_DA_DK  "Danish"
#define XAP_STRING_ID_LANG_NL_NL  "Dutch (Netherlands)"
#define XAP_STRING_ID_LANG_EO  "Esperanto"
#define XAP_STRING_ID_LANG_ET  "Estonian"		// Hipi: Why not et-EE?
#define XAP_STRING_ID_LANG_FA_IR  "Farsi"
#define XAP_STRING_ID_LANG_FI_FI  "Finnish"
#define XAP_STRING_ID_LANG_NL_BE  "Flemish (Belgium)"
#define XAP_STRING_ID_LANG_FR_BE  "French (Belgium)"
#define XAP_STRING_ID_LANG_FR_CA  "French (Canada)"
#define XAP_STRING_ID_LANG_FR_FR  "French (France)"
#define XAP_STRING_ID_LANG_FR_CH  "French (Switzerland)"
#define XAP_STRING_ID_LANG_FY_NL  "Frisian"	// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_GL  "Galician (Galego)"
#define XAP_STRING_ID_LANG_KA_GE  "Georgian"
#define XAP_STRING_ID_LANG_DE_AT  "German (Austria)"
#define XAP_STRING_ID_LANG_DE_DE  "German (Germany)"
#define XAP_STRING_ID_LANG_DE_CH  "German (Switzerland)"
#define XAP_STRING_ID_LANG_EL_GR  "Greek"
#define XAP_STRING_ID_LANG_HA_NE  "Hausa (Niger)"
#define XAP_STRING_ID_LANG_HA_NG  "Hausa (Nigeria)"
#define XAP_STRING_ID_LANG_HAW_US  "Hawaiian"
#define XAP_STRING_ID_LANG_HE_IL  "Hebrew"
#define XAP_STRING_ID_LANG_HI_IN  "Hindi"
#define XAP_STRING_ID_LANG_HU_HU  "Hungarian"
#define XAP_STRING_ID_LANG_IS_IS  "Icelandic"
#define XAP_STRING_ID_LANG_ID_ID  "Indonesian"
#define XAP_STRING_ID_LANG_IU_CA  "Inuktitut"
#define XAP_STRING_ID_LANG_IA  "Interlingua"
#define XAP_STRING_ID_LANG_GA_IE  "Irish"
#define XAP_STRING_ID_LANG_IT_IT  "Italian (Italy)"
#define XAP_STRING_ID_LANG_JA_JP  "Japanese"
#define XAP_STRING_ID_LANG_KN_IN  "Kannada"
#define XAP_STRING_ID_LANG_KO_KR  "Korean"
#define XAP_STRING_ID_LANG_KO  "Korean"
#define XAP_STRING_ID_LANG_KU  "Kurdish"
#define XAP_STRING_ID_LANG_LO_LA  "Lao"
#define XAP_STRING_ID_LANG_LA_IT  "Latin (Renaissance)"	// Is _IT the right thing here?
#define XAP_STRING_ID_LANG_LV_LV  "Latvian"
#define XAP_STRING_ID_LANG_LT_LT  "Lithuanian"
#define XAP_STRING_ID_LANG_MK  "Macedonian"	// Jordi 19/10/2002 Hipi: Why not mk-MK?
#define XAP_STRING_ID_LANG_MS_MY  "Malay"
#define XAP_STRING_ID_LANG_MI_NZ  "Maori"
#define XAP_STRING_ID_LANG_MR_IN  "Marathi"
#define XAP_STRING_ID_LANG_MH_MH  "Marshallese (Marshall Islands)"
#define XAP_STRING_ID_LANG_MH_NR  "Marshallese (Nauru)"
#define XAP_STRING_ID_LANG_MN_MN  "Mongolian"
#define XAP_STRING_ID_LANG_NB_NO  "Norwegian Bokmal"
#define XAP_STRING_ID_LANG_NE_NP  "Nepali (Nepal)"
#define XAP_STRING_ID_LANG_NN_NO  "Norwegian Nynorsk"
#define XAP_STRING_ID_LANG_OC_FR  "Occitan"		// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_PL_PL  "Polish"
#define XAP_STRING_ID_LANG_PS  "Pashto"
#define XAP_STRING_ID_LANG_PT_BR  "Portuguese (Brazil)"
#define XAP_STRING_ID_LANG_PT_PT  "Portuguese (Portugal)"
#define XAP_STRING_ID_LANG_PA_IN  "Punjabi (Gurmukhi)"
#define XAP_STRING_ID_LANG_PA_PK  "Punjabi (Shahmukhi)"
#define XAP_STRING_ID_LANG_QU_BO  "Quechua"
#define XAP_STRING_ID_LANG_QUH_BO  "Quechua (3 vowels)"
#define XAP_STRING_ID_LANG_QUL_BO  "Quechua (5 vowels)"
#define XAP_STRING_ID_LANG_RO_RO  "Romanian"
#define XAP_STRING_ID_LANG_RU_RU  "Russian (Russia)"
#define XAP_STRING_ID_LANG_SC_IT  "Sardinian"	// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_SR  "Serbian"		// Jordi 19/10/2002 Hipi: Why not sr-YU?
#define XAP_STRING_ID_LANG_SK_SK  "Slovak"				// or Slovakian?
#define XAP_STRING_ID_LANG_SL_SI  "Slovenian"
#define XAP_STRING_ID_LANG_ES_MX  "Spanish (Mexico)"
#define XAP_STRING_ID_LANG_ES_ES  "Spanish (Spain)"
#define XAP_STRING_ID_LANG_SW  "Swahili"
#define XAP_STRING_ID_LANG_SV_SE  "Swedish"
#define XAP_STRING_ID_LANG_SYR  "Syriac"
#define XAP_STRING_ID_LANG_TL_PH  "Tagalog"
#define XAP_STRING_ID_LANG_TA_IN  "Tamil"
#define XAP_STRING_ID_LANG_TE_IN  "Telugu"
#define XAP_STRING_ID_LANG_TH_TH  "Thai"
#define XAP_STRING_ID_LANG_TR_TR  "Turkish"
#define XAP_STRING_ID_LANG_UK_UA  "Ukrainian"
#define XAP_STRING_ID_LANG_UR_PK  "Urdu (Pakistan)"
#define XAP_STRING_ID_LANG_UR  "Urdu"
#define XAP_STRING_ID_LANG_UZ_UZ  "Uzbek"
#define XAP_STRING_ID_LANG_VI_VN  "Vietnamese"
#define XAP_STRING_ID_LANG_CY_GB  "Welsh"
#define XAP_STRING_ID_LANG_WO_SN  "Wolof (Senegal)"
#define XAP_STRING_ID_LANG_YI  "Yiddish"


/* Encoding Dialog */
#define XAP_STRING_ID_DLG_UENC_EncLabel  "Select Encoding:"
#define XAP_STRING_ID_DLG_UENC_EncTitle  "Encoding"

/* Encoding description in different encodings; keep sorted by the numerical id's */
/* Ordered by 1) number of bits, 2) region & language, 3) platform / standard.	  */
/* 1) 7 bit, 8 bit, multibyte, Unicode. 										  */
/* 2) Western, Eastern, Asian, multilingual; common before rare.				  */
/* 3) ISO, de facto standards, MS Windows, Macintosh							  */
/* It may be desirable to change this order for each platform.					  */

/* 7 bit */
#define XAP_STRING_ID_ENC_WEST_ASCII  "US-ASCII"
/* 8 bit */
/* Western Europe */
#define XAP_STRING_ID_ENC_WEST_ISO  "Western European, ISO-8859-1"
#define XAP_STRING_ID_ENC_WEST_WIN  "Western European, Windows Code Page 1252"
#define XAP_STRING_ID_ENC_US_DOS  "Western European, DOS/Windows Code Page 437"
#define XAP_STRING_ID_ENC_MLNG_DOS  "Western European, DOS/Windows Code Page 850"
#define XAP_STRING_ID_ENC_WEST_MAC  "Western European, Macintosh"
#define XAP_STRING_ID_ENC_WEST_HP  "Western European, HP"
#define XAP_STRING_ID_ENC_WEST_NXT  "Western European, NeXT"
/* Central & Eastern Europe */
#define XAP_STRING_ID_ENC_CENT_ISO  "Central European, ISO-8859-2"
#define XAP_STRING_ID_ENC_CENT_WIN  "Central European, Windows Code Page 1250"
#define XAP_STRING_ID_ENC_CENT_MAC  "Central European, Macintosh"
/* Baltic */
#define XAP_STRING_ID_ENC_BALT_ISO  "Baltic, ISO-8859-4"
#define XAP_STRING_ID_ENC_BALT_WIN  "Baltic, Windows Code Page 1257"
/* Greek */
#define XAP_STRING_ID_ENC_GREE_ISO  "Greek, ISO-8859-7"
#define XAP_STRING_ID_ENC_GREE_WIN  "Greek, Windows Code Page 1253"
#define XAP_STRING_ID_ENC_GREE_MAC  "Greek, Macintosh"
/* Cyrillic */
#define XAP_STRING_ID_ENC_CYRL_ISO  "Cyrillic, ISO-8859-5"
#define XAP_STRING_ID_ENC_CYRL_KOI  "Cyrillic, KOI8-R"
#define XAP_STRING_ID_ENC_CYRL_WIN  "Cyrillic, Windows Code Page 1251"
#define XAP_STRING_ID_ENC_CYRL_MAC  "Cyrillic, Macintosh"
#define XAP_STRING_ID_ENC_UKRA_KOI  "Ukrainian, KOI8-U"
#define XAP_STRING_ID_ENC_UKRA_MAC  "Ukrainian, Macintosh"
/* Turkish */
#define XAP_STRING_ID_ENC_TURK_ISO  "Turkish, ISO-8859-9"
#define XAP_STRING_ID_ENC_TURK_WIN  "Turkish, Windows Code Page 1254"
#define XAP_STRING_ID_ENC_TURK_MAC  "Turkish, Macintosh"
/* Other Roman-based encodings */
#define XAP_STRING_ID_ENC_CROA_MAC  "Croatian, Macintosh"
#define XAP_STRING_ID_ENC_ICEL_MAC  "Icelandic, Macintosh"
#define XAP_STRING_ID_ENC_ROMA_MAC  "Romanian, Macintosh"
/* Thai */
#define XAP_STRING_ID_ENC_THAI_TIS  "Thai, TIS-620"
#define XAP_STRING_ID_ENC_THAI_WIN  "Thai, Windows Code Page 874"
#define XAP_STRING_ID_ENC_THAI_MAC  "Thai, Macintosh"
/* Vietnamese */
#define XAP_STRING_ID_ENC_VIET_VISCII  "Vietnamese, VISCII"
#define XAP_STRING_ID_ENC_VIET_TCVN  "Vietnamese, TCVN"
#define XAP_STRING_ID_ENC_VIET_WIN  "Vietnamese, Windows Code Page 1258"
/* Hebrew */
#define XAP_STRING_ID_ENC_HEBR_ISO  "Hebrew, ISO-8859-8"
#define XAP_STRING_ID_ENC_HEBR_WIN  "Hebrew, Windows Code Page 1255"
#define XAP_STRING_ID_ENC_HEBR_MAC  "Hebrew, Macintosh"
/* Arabic */
#define XAP_STRING_ID_ENC_ARAB_ISO  "Arabic, ISO-8859-6"
#define XAP_STRING_ID_ENC_ARAB_WIN  "Arabic, Windows Code Page 1256"
#define XAP_STRING_ID_ENC_ARAB_MAC  "Arabic, Macintosh"
/* Armenian */
#define XAP_STRING_ID_ENC_ARME_ARMSCII  "Armenian, ARMSCII-8"
/* Georgian */
#define XAP_STRING_ID_ENC_GEOR_ACADEMY  "Georgian, Academy"
#define XAP_STRING_ID_ENC_GEOR_PS  "Georgian, PS"
/* Multibyte CJK */
/* Chinese Simplified */
#define XAP_STRING_ID_ENC_CHSI_EUC  "Chinese Simplified, EUC-CN (GB2312)"
#define XAP_STRING_ID_ENC_CHSI_GB  "Chinese Simplified, GB_2312-80"	// Cf. EUC
#define XAP_STRING_ID_ENC_CHSI_HZ  "Chinese Simplified, HZ"
#define XAP_STRING_ID_ENC_CHSI_WIN  "Chinese Simplified, Windows Code Page 936"
/* Chinese Traditional */
#define XAP_STRING_ID_ENC_CHTR_BIG5  "Chinese Traditional, BIG5"
#define XAP_STRING_ID_ENC_CHTR_BIG5HKSCS  "Chinese Traditional, BIG5-HKSCS"
#define XAP_STRING_ID_ENC_CHTR_EUC  "Chinese Traditional, EUC-TW"
#define XAP_STRING_ID_ENC_CHTR_WIN  "Chinese Traditional, Windows Code Page 950"
/* Japanese */
#define XAP_STRING_ID_ENC_JAPN_ISO  "Japanese, ISO-2022-JP"
#define XAP_STRING_ID_ENC_JAPN_EUC  "Japanese, EUC-JP"
#define XAP_STRING_ID_ENC_JAPN_SJIS  "Japanese, Shift-JIS"
#define XAP_STRING_ID_ENC_JAPN_WIN  "Japanese, Windows Code Page 932"
/* Korean */
#define XAP_STRING_ID_ENC_KORE_KSC  "Korean, KSC_5601" // ISO
#define XAP_STRING_ID_ENC_KORE_EUC  "Korean, EUC-KR"
#define XAP_STRING_ID_ENC_KORE_JOHAB  "Korean, Johab"
#define XAP_STRING_ID_ENC_KORE_WIN  "Korean, Windows Code Page 949"
/* Unicode */
#define XAP_STRING_ID_ENC_UNIC_UTF_7  "Unicode UTF-7"
#define XAP_STRING_ID_ENC_UNIC_UTF_8  "Unicode UTF-8"
//#define XAP_STRING_ID_ENC_UNIC_UTF_16  "Unicode UTF-16"
#define XAP_STRING_ID_ENC_UNIC_UTF_16BE  "Unicode UTF-16 Big Endian"
#define XAP_STRING_ID_ENC_UNIC_UTF_16LE  "Unicode UTF-16 Little Endian"
//#define XAP_STRING_ID_ENC_UNIC_UTF_32  "Unicode UTF-32"
#define XAP_STRING_ID_ENC_UNIC_UTF_32BE  "Unicode UTF-32 Big Endian"
#define XAP_STRING_ID_ENC_UNIC_UTF_32LE  "Unicode UTF-32 Little Endian"
//#define XAP_STRING_ID_ENC_UNIC_UCS2  "Unicode UCS-2"
#define XAP_STRING_ID_ENC_UNIC_UCS_2BE  "Unicode UCS-2 Big Endian"
#define XAP_STRING_ID_ENC_UNIC_UCS_2LE  "Unicode UCS-2 Little Endian"
//#define XAP_STRING_ID_ENC_UNIC_UCS4  "Unicode UCS-4"
#define XAP_STRING_ID_ENC_UNIC_UCS_4BE  "Unicode UCS-4 Big Endian"
#define XAP_STRING_ID_ENC_UNIC_UCS_4LE  "Unicode UCS-4 Little Endian"

/* HTML Options Dialog */
#define XAP_STRING_ID_DLG_HTMLOPT_ExpTitle  "HTML Export Options"
#define XAP_STRING_ID_DLG_HTMLOPT_ExpLabel  "Select HTML export options:"
#define XAP_STRING_ID_DLG_HTMLOPT_ExpSave  "Save Settings"
#define XAP_STRING_ID_DLG_HTMLOPT_ExpRestore  "Restore Settings"
#define XAP_STRING_ID_DLG_HTMLOPT_ExpIs4  "Export as HTML 4.01"
#define XAP_STRING_ID_DLG_HTMLOPT_ExpAbiWebDoc  "Export with PHP instructions"
#define XAP_STRING_ID_DLG_HTMLOPT_ExpDeclareXML  "Declare as XML (version 1.0)"
#define XAP_STRING_ID_DLG_HTMLOPT_ExpAllowAWML  "Allow extra markup in AWML namespace"
#define XAP_STRING_ID_DLG_HTMLOPT_ExpEmbedCSS  "Embed (CSS) style sheet"
#define XAP_STRING_ID_DLG_HTMLOPT_ExpEmbedImages  "Embed images in URLs (Base64-encoded)"
#define XAP_STRING_ID_DLG_Exit_CloseWithoutSaving  "Close &Without Saving"

	/* X11 input methods */
#define XAP_STRING_ID_XIM_Methods  "Input Methods"

//since this string goes with XAP preference, I put it here, rather
//than ap_String_Id.h
#define XAP_STRING_ID_DLG_Options_Label_LangWithKeyboard  "Change Language when changing keyboard"
#define XAP_STRING_ID_DLG_Options_Label_DirMarkerAfterClosingParenthesis  "Auto-insert direction markers"

#define XAP_STRING_ID_DLG_History_WindowLabel  "Document History"
#define XAP_STRING_ID_DLG_History_DocumentDetails  "Document Details"
#define XAP_STRING_ID_DLG_History_Path  "Document name:"
#define XAP_STRING_ID_DLG_History_Created  "Created:"
#define XAP_STRING_ID_DLG_History_Version  "Version:"
#define XAP_STRING_ID_DLG_History_LastSaved  "Last saved:"
#define XAP_STRING_ID_DLG_History_EditTime  "Editing time:"
#define XAP_STRING_ID_DLG_History_Id  "Identifier:"

#define XAP_STRING_ID_DLG_History_List_Title  "Version history"
#define XAP_STRING_ID_DLG_History_Version_Version  "Version"
#define XAP_STRING_ID_DLG_History_Version_Started  "Created"
#define XAP_STRING_ID_DLG_History_Version_AutoRevisioned  "Auto-revision"
	
#define XAP_STRING_ID_DLG_DocComparison_WindowLabel  "Document Comparison"
#define XAP_STRING_ID_DLG_DocComparison_DocsCompared  "Documents compared"
#define XAP_STRING_ID_DLG_DocComparison_Results  "Results"
#define XAP_STRING_ID_DLG_DocComparison_Relationship  "Relationship:"
#define XAP_STRING_ID_DLG_DocComparison_Content  "Content:"
#define XAP_STRING_ID_DLG_DocComparison_Fmt  "Format:"
#define XAP_STRING_ID_DLG_DocComparison_Styles  "Styles:"

#define XAP_STRING_ID_DLG_DocComparison_Identical  "identical"
#define XAP_STRING_ID_DLG_DocComparison_Unrelated  "unrelated"
#define XAP_STRING_ID_DLG_DocComparison_Siblings  "siblings"
#define XAP_STRING_ID_DLG_DocComparison_Diverging  "diverging after version %d of %s"
#define XAP_STRING_ID_DLG_DocComparison_DivergingPos  "diverging after document position %d"
#define XAP_STRING_ID_DLG_DocComparison_TestSkipped  "(test skipped)"
#define XAP_STRING_ID_DLG_DocComparison_Different  "different"

#endif /* XAP_STRING_ID_H */
