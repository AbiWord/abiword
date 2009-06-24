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

#define XAP_STRING_ID_MSG_ImportingDoc  _("Importing Document...")
#define XAP_STRING_ID_MSG_BuildingDoc  _("Building Document:")
#define XAP_STRING_ID_MSG_AutoRevision  _("Autorevision")
#define XAP_STRING_ID_MSG_HistoryPartRestore1  _("AbiWord cannot fully restore version %d of the document because the version information is incomplete.")
#define XAP_STRING_ID_MSG_HistoryPartRestore2  _("The nearest version that can be restored fully is %d. Would you like to restore this version instead? To partially restore version %d press No.")
#define XAP_STRING_ID_MSG_HistoryPartRestore3  _("To continue anyway, press OK.")
#define XAP_STRING_ID_MSG_HistoryPartRestore4  _("To quit the restoration attempt, press Cancel.")
#define XAP_STRING_ID_MSG_HistoryNoRestore  _("AbiWord cannot restore version %d of the document because the version information is missing.")
#define XAP_STRING_ID_MSG_HistoryConfirmSave  _("You have to save changes to document %s before proceeding. Save now?")

#define XAP_STRING_ID_MSG_NoUndo  _("This operation cannot be undone. Are you sure you want to proceed?")

/* Default name for new, untitled document */
#define XAP_STRING_ID_UntitledDocument  _("Untitled%d")
#define XAP_STRING_ID_ReadOnly  _("Read-Only")

/*  Styles */
#define XAP_STRING_ID_STYLE_NUMBER_LIST  _("Numbered List")
#define XAP_STRING_ID_STYLE_PLAIN_TEXT  _("Plain Text")
#define XAP_STRING_ID_STYLE_NORMAL  _("Normal")
#define XAP_STRING_ID_STYLE_HEADING1  _("Heading 1")
#define XAP_STRING_ID_STYLE_HEADING2  _("Heading 2")
#define XAP_STRING_ID_STYLE_HEADING3  _("Heading 3")
#define XAP_STRING_ID_STYLE_HEADING4  _("Heading 4")
#define XAP_STRING_ID_STYLE_TOCHEADING  _("Contents Header")
#define XAP_STRING_ID_STYLE_TOCHEADING1  _("Contents 1")
#define XAP_STRING_ID_STYLE_TOCHEADING2  _("Contents 2")
#define XAP_STRING_ID_STYLE_TOCHEADING3  _("Contents 3")
#define XAP_STRING_ID_STYLE_TOCHEADING4  _("Contents 4")
#define XAP_STRING_ID_STYLE_BLOCKTEXT  _("Block Text")          
#define XAP_STRING_ID_STYLE_LOWERCASELIST  _("Lower Case List")     
#define XAP_STRING_ID_STYLE_UPPERCASTELIST  _("Upper Case List")     
#define XAP_STRING_ID_STYLE_LOWERROMANLIST  _("Lower Roman List")    
#define XAP_STRING_ID_STYLE_UPPERROMANLIST  _("Upper Roman List")    
#define XAP_STRING_ID_STYLE_BULLETLIST  _("Bullet List")         
#define XAP_STRING_ID_STYLE_DASHEDLIST  _("Dashed List")         
#define XAP_STRING_ID_STYLE_SQUARELIST  _("Square List")         
#define XAP_STRING_ID_STYLE_TRIANGLELIST  _("Triangle List")       
#define XAP_STRING_ID_STYLE_DIAMONLIST  _("Diamond List")	      
#define XAP_STRING_ID_STYLE_STARLIST  _("Star List")           
#define XAP_STRING_ID_STYLE_TICKLIST  _("Tick List")           
#define XAP_STRING_ID_STYLE_BOXLIST  _("Box List")            
#define XAP_STRING_ID_STYLE_HANDLIST  _("Hand List")           
#define XAP_STRING_ID_STYLE_HEARTLIST  _("Heart List")          
#define XAP_STRING_ID_STYLE_CHAPHEADING  _("Chapter Heading")     
#define XAP_STRING_ID_STYLE_SECTHEADING  _("Section Heading")     
#define XAP_STRING_ID_STYLE_ENDREFERENCE  _("Endnote Reference")   
#define XAP_STRING_ID_STYLE_ENDTEXT  _("Endnote Text")        
#define XAP_STRING_ID_STYLE_FOOTREFERENCE  _("Footnote Reference")   
#define XAP_STRING_ID_STYLE_FOOTTEXT  _("Footnote Text")        
#define XAP_STRING_ID_STYLE_NUMHEAD1  _("Numbered Heading 1")  
#define XAP_STRING_ID_STYLE_NUMHEAD2  _("Numbered Heading 2")  
#define XAP_STRING_ID_STYLE_NUMHEAD3  _("Numbered Heading 3")  
#define XAP_STRING_ID_STYLE_IMPLIES_LIST  _("Implies List")  


/* Common to many dialogs */
#define XAP_STRING_ID_DLG_OK  _("OK")
#define XAP_STRING_ID_DLG_Cancel  _("Cancel")
#define XAP_STRING_ID_DLG_Close  _("Close")
#define XAP_STRING_ID_DLG_Insert  _("&Insert")
#define XAP_STRING_ID_DLG_Update  _("Update")
#define XAP_STRING_ID_DLG_Apply  _("Apply")
#define XAP_STRING_ID_DLG_Delete  _("Delete")
#define XAP_STRING_ID_DLG_Compare  _("Compare")
#define XAP_STRING_ID_DLG_Select  _("Select")
#define XAP_STRING_ID_DLG_Merge  _("Merge")
#define XAP_STRING_ID_DLG_Show  _("Show")
#define XAP_STRING_ID_DLG_Restore  _("Restore")
	
/* Units */
#define XAP_STRING_ID_DLG_Unit_inch  _("inch")
#define XAP_STRING_ID_DLG_Unit_cm  _("cm")
#define XAP_STRING_ID_DLG_Unit_mm  _("mm")
#define XAP_STRING_ID_DLG_Unit_points  _("points")
#define XAP_STRING_ID_DLG_Unit_pica  _("pica")

/* Message box */
/* These are tagged _("UnixMB") because the underscores precede accelerator
   #define XAP_STRING_ID_acters.	It should be an ampersand on Windows, but Windows doesn't
   need a hand-constructed message box (Win32 API provides one).
*/
#define XAP_STRING_ID_DLG_UnixMB_Yes  _("_Yes")
#define XAP_STRING_ID_DLG_UnixMB_No  _("_No")

#define XAP_STRING_ID_DLG_MB_Yes  _("Yes")
#define XAP_STRING_ID_DLG_MB_No  _("No")

/* More Windows dialog */
#define XAP_STRING_ID_DLG_MW_MoreWindows  _("View Document")
#define XAP_STRING_ID_DLG_MW_Activate  _("View:")
#define XAP_STRING_ID_DLG_MW_AvailableDocuments  _("Available Documents")
#define XAP_STRING_ID_DLG_MW_ViewButton  _("&View")

/* Remove Toolbar Icon */
#define XAP_STRING_ID_DLG_Remove_Icon  _("Do you want to remove this icon from the toolbar?")

/* Font Selector dialog */
#define XAP_STRING_ID_DLG_UFS_FontTitle  _("Font")
#define XAP_STRING_ID_DLG_UFS_FontLabel  _("Font:")
#define XAP_STRING_ID_DLG_UFS_StyleLabel  _("Style:")
#define XAP_STRING_ID_DLG_UFS_SizeLabel  _("Size:")
#define XAP_STRING_ID_DLG_UFS_EncodingLabel  _("Encoding:")
#define XAP_STRING_ID_DLG_UFS_EffectsFrameLabel  _("Effects")
#define XAP_STRING_ID_DLG_UFS_StrikeoutCheck  _("Strike")
#define XAP_STRING_ID_DLG_UFS_UnderlineCheck  _("Underline")
#define XAP_STRING_ID_DLG_UFS_OverlineCheck  _("Overline")
#define XAP_STRING_ID_DLG_UFS_HiddenCheck  _("Hidden")	
#define XAP_STRING_ID_DLG_UFS_TransparencyCheck  _("Set no Highlight Color")
#define XAP_STRING_ID_DLG_UFS_FontTab  _("Font")
#define XAP_STRING_ID_DLG_UFS_ColorTab  _("Text Color")
#define XAP_STRING_ID_DLG_UFS_BGColorTab  _("HighLight Color")
#define XAP_STRING_ID_DLG_UFS_StyleRegular  _("Regular")
#define XAP_STRING_ID_DLG_UFS_StyleItalic  _("Italic")
#define XAP_STRING_ID_DLG_UFS_StyleBold  _("Bold")
#define XAP_STRING_ID_DLG_UFS_StyleBoldItalic  _("Bold Italic")
#define XAP_STRING_ID_DLG_UFS_ToplineCheck  _("Topline")
#define XAP_STRING_ID_DLG_UFS_BottomlineCheck  _("Bottomline")
#define XAP_STRING_ID_DLG_UFS_ColorLabel  _("Color:")
#define XAP_STRING_ID_DLG_UFS_ScriptLabel  _("Script:")
#define XAP_STRING_ID_DLG_UFS_SampleFrameLabel  _("Sample")
#define XAP_STRING_ID_DLG_UFS_SuperScript  _("Superscript")
#define XAP_STRING_ID_DLG_UFS_SubScript  _("Subscript")

/* Unix FileOpenSaveAs dialog */
#define XAP_STRING_ID_DLG_FOSA_OpenTitle  _("Open File")
#define XAP_STRING_ID_DLG_FOSA_SaveAsTitle  _("Save File As")
#define XAP_STRING_ID_DLG_FOSA_ExportTitle  _("Export File")
#define XAP_STRING_ID_DLG_FOSA_ImportTitle  _("Import File")
#define XAP_STRING_ID_DLG_FOSA_InsertTitle  _("Insert File")
#define XAP_STRING_ID_DLG_FOSA_InsertMath  _("Insert Math File")
#define XAP_STRING_ID_DLG_FOSA_InsertObject  _("Insert Embeddable Object")
#define XAP_STRING_ID_DLG_FOSA_PrintToFileTitle  _("Print To File")
#define XAP_STRING_ID_DLG_FOSA_RecordToFileTitle  _("Record Editing to File")
#define XAP_STRING_ID_DLG_FOSA_ReplayFromFileTitle  _("Replay Editing from File")
#define XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel  _("Open file as type:")
#define XAP_STRING_ID_DLG_FOSA_FileInsertMath  _("Insert MathML file:")
#define XAP_STRING_ID_DLG_FOSA_FileInsertObject  _("Insert Embeddable Object file:")
#define XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel  _("Save file as type:")
#define XAP_STRING_ID_DLG_FOSA_FilePrintTypeLabel  _("Print file as type:")
#define XAP_STRING_ID_DLG_FOSA_RecordToFileLabel  _("File to record editing:")
#define XAP_STRING_ID_DLG_FOSA_ReplayFromFileLabel  _("File to replay editing:")
#define XAP_STRING_ID_DLG_FOSA_FileTypeAutoDetect  _("Automatically Detected")
#define XAP_STRING_ID_DLG_FOSA_ALLIMAGES  _("All Image Files")
#define XAP_STRING_ID_DLG_FOSA_ALLDOCS  _("All Documents")
#define XAP_STRING_ID_DLG_FOSA_ALL  _("All (*.*)")
#define XAP_STRING_ID_DLG_InvalidPathname  _("Invalid pathname.")
#define XAP_STRING_ID_DLG_NoSaveFile_DirNotExist  _("A directory in the given pathname does not exist.")
#define XAP_STRING_ID_DLG_NoSaveFile_DirNotWriteable  _("The directory '%s' is write-protected.")
#define XAP_STRING_ID_DLG_OverwriteFile  _("File already exists.  Overwrite file '%s'?")
#define XAP_STRING_ID_DLG_FOSA_ExtensionDoesNotMatch  _("The given file extension does not match the chosen file type. Do you want to use this name anyway?")

/* Password dialog */
#define XAP_STRING_ID_DLG_Password_Title  _("Enter Password")
#define XAP_STRING_ID_DLG_Password_Password  _("Password:")

/* Zoom dialog */
#define XAP_STRING_ID_DLG_Zoom_ZoomTitle  _("Zoom")
#define XAP_STRING_ID_DLG_Zoom_RadioFrameCaption  _("Zoom to")
#define XAP_STRING_ID_DLG_Zoom_200  _("&200%")
#define XAP_STRING_ID_DLG_Zoom_100  _("&100%")
#define XAP_STRING_ID_DLG_Zoom_75  _("&75%")
#define XAP_STRING_ID_DLG_Zoom_PageWidth  _("&Page width")
#define XAP_STRING_ID_DLG_Zoom_WholePage  _("&Whole page")
#define XAP_STRING_ID_DLG_Zoom_Percent  _("P&ercent:")
#define XAP_STRING_ID_DLG_Zoom_PreviewFrame  _("Preview")

/* Zoom tool bar -- Truncated to fit small combobox size*/
#define XAP_STRING_ID_TB_Zoom_PageWidth  _("Page Width")
#define XAP_STRING_ID_TB_Zoom_WholePage  _("Whole Page")
#define XAP_STRING_ID_TB_Zoom_Percent  _("Other...")

/* Font tool bar*/
#define XAP_STRING_ID_TB_Font_Symbol  _("Symbols")

/* Unix Print dialog */
#define XAP_STRING_ID_DLG_UP_PrintTitle  _("Print")
#define XAP_STRING_ID_DLG_UP_PrintPreviewTitle  _("AbiWord: Print Preview")
#define XAP_STRING_ID_DLG_UP_PrintTo  _("Print to: ")
#define XAP_STRING_ID_DLG_UP_Printer  _("Printer")
#define XAP_STRING_ID_DLG_UP_File  _("File")
#define XAP_STRING_ID_DLG_UP_PrinterCommand  _("Printer command: ")
#define XAP_STRING_ID_DLG_UP_PageRanges  _("Page ranges:")
#define XAP_STRING_ID_DLG_UP_All  _("All")
#define XAP_STRING_ID_DLG_UP_From  _("From: ")
#define XAP_STRING_ID_DLG_UP_To  _(" to ")
#define XAP_STRING_ID_DLG_UP_Selection  _("Selection")
#define XAP_STRING_ID_DLG_UP_Collate  _("Collate")
#define XAP_STRING_ID_DLG_UP_EmbedFonts  _("Embed Fonts")
#define XAP_STRING_ID_DLG_UP_Copies  _("Copies: ")
#define XAP_STRING_ID_DLG_UP_PrintButton  _("Print")
#define XAP_STRING_ID_DLG_UP_InvalidPrintString  _("The print command string is not valid.")
#define XAP_STRING_ID_DLG_UP_PrintIn  _("Print in: ")
#define XAP_STRING_ID_DLG_UP_BlackWhite  _("Black & White")
#define XAP_STRING_ID_DLG_UP_Grayscale  _("Grayscale")
#define XAP_STRING_ID_DLG_UP_Color  _("Color")

/* Insert Symbol dialog */
#define XAP_STRING_ID_DLG_Insert_SymbolTitle  _("Insert Symbol")

/* Insert Picture Preview Dialog (Win32) */
#define XAP_STRING_ID_DLG_IP_Title  _("Insert Picture")
#define XAP_STRING_ID_DLG_IP_Activate_Label  _("Preview Picture")
#define XAP_STRING_ID_DLG_IP_No_Picture_Label  _("No Picture")
#define XAP_STRING_ID_DLG_IP_Height_Label  _("Height: ")
#define XAP_STRING_ID_DLG_IP_Width_Label  _("Width:  ")
#define XAP_STRING_ID_DLG_IP_Button_Label  _("Insert")

/* Plugin dialog */
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_TITLE  _("AbiWord Plugin Manager")
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_ACTIVE  _("Active Plugins")
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE  _("Deactivate plugin")
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_DEACTIVATE_ALL  _("Deactivate all plugins")
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_INSTALL  _("Install new plugin")
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_LIST  _("Plugin List")
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_NAME  _("Name:")
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_DESC  _("Description:")
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_AUTHOR  _("Author:")
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_VERSION  _("Version:")
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_DETAILS  _("Plugin Details:")
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_NOT_AVAILABLE  _("Not available")

/* spellchecker */
#define XAP_STRING_ID_SPELL_CANTLOAD_DICT  _("Could not load the dictionary for the %s language")
#define XAP_STRING_ID_SPELL_CANTLOAD_DLL  _("AbiWord cannot find the spelling file %s.dll\nPlease download and install Aspell from http://aspell.net/win32/")

/* plugin error messages */
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_LOAD  _("Could not activate/load plugin")
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_COULDNT_UNLOAD  _("Could not deactivate plugin")
#define XAP_STRING_ID_DLG_PLUGIN_MANAGER_NONE_SELECTED  _("No plugin selected")

/* Language Dialog */
#define XAP_STRING_ID_DLG_ULANG_LangTitle  _("Set Language")
#define XAP_STRING_ID_DLG_ULANG_LangLabel  _("Select Language:")
#define XAP_STRING_ID_DLG_ULANG_AvailableLanguages  _("Available Languages")
#define XAP_STRING_ID_DLG_ULANG_SetLangButton  _("&Set Language")
#define XAP_STRING_ID_DLG_ULANG_DefaultLangLabel  _("Default language: ")
#define XAP_STRING_ID_DLG_ULANG_DefaultLangChkbox  _("Make default for document")
	
/* ClipArt Dialog */
#define XAP_STRING_ID_DLG_CLIPART_Title  _("Clip Art")
#define XAP_STRING_ID_DLG_CLIPART_Loading  _("Loading Clip Art")
#define XAP_STRING_ID_DLG_CLIPART_Error  _("Clip Art could not be loaded")

/* About Dialog */
#define XAP_STRING_ID_DLG_ABOUT_Title  _("About %s")

/* image size dialog */
#define XAP_STRING_ID_DLG_Image_Title  _("Image Properties")
#define XAP_STRING_ID_DLG_Image_Width  _("Width:")
#define XAP_STRING_ID_DLG_Image_Height  _("Height:")
#define XAP_STRING_ID_DLG_Image_Aspect  _("Preserve aspect ratio")
#define XAP_STRING_ID_DLG_Image_LblTitle  _("Title:")
#define XAP_STRING_ID_DLG_Image_LblDescription  _("Description:")
#define XAP_STRING_ID_DLG_Image_ImageSize  _("Set Image Size")
#define XAP_STRING_ID_DLG_Image_ImageDesc  _("Set Image Name")
#define XAP_STRING_ID_DLG_Image_TextWrapping  _("Define Text Wrapping")
#define XAP_STRING_ID_DLG_Image_Placement  _("Define Image Placement")
#define XAP_STRING_ID_DLG_Image_InLine  _("Image placed in-line (no text wrapping)")
#define XAP_STRING_ID_DLG_Image_WrappedNone  _("Image floats above text")
#define XAP_STRING_ID_DLG_Image_WrappedRight  _("Text wrapped to the Right of the Image")
#define XAP_STRING_ID_DLG_Image_WrappedLeft  _("Text wrapped to the Left of the Image")
#define XAP_STRING_ID_DLG_Image_WrappedBoth  _("Text wrapped on both sides of the Image")
#define XAP_STRING_ID_DLG_Image_PlaceParagraph  _("Position relative to nearest paragraph")
#define XAP_STRING_ID_DLG_Image_PlaceColumn  _("Position relative to its Column")
#define XAP_STRING_ID_DLG_Image_PlacePage  _("Position relative to its Page")
#define XAP_STRING_ID_DLG_Image_WrapType  _("Type of text wrapping")
#define XAP_STRING_ID_DLG_Image_SquareWrap  _("Square text wrapping")
#define XAP_STRING_ID_DLG_Image_TightWrap  _("Tight text wrapping")

/* ListDocuments Dialog */
#define XAP_STRING_ID_DLG_LISTDOCS_Title  _("Opened Documents")
#define XAP_STRING_ID_DLG_LISTDOCS_Heading1  _("Choose document from the list:")
	
/*
For insert Table widget
*/
#define XAP_STRING_ID_TB_InsertNewTable  _("Insert New Table")
#define XAP_STRING_ID_TB_Table  _("Table")
#define XAP_STRING_ID_TB_ClearBackground  _("Clear Background")

/*
	  Language property in different languages; alphabetical except English first.
	  Please when translating the first entry, put it also into parenthesis
	  or, surround it by some other #define XAP_STRING_ID_acters, so that it will appear on the top
	  of the list when sorted alphabetically.
*/
#define XAP_STRING_ID_LANG_0  _("(no proofing)")
#define XAP_STRING_ID_LANG_EN_AU  _("English (Australia)")
#define XAP_STRING_ID_LANG_EN_CA  _("English (Canada)")
#define XAP_STRING_ID_LANG_EN_GB  _("English (UK)")
#define XAP_STRING_ID_LANG_EN_IE  _("English (Ireland)")
#define XAP_STRING_ID_LANG_EN_NZ  _("English (New Zealand)")
#define XAP_STRING_ID_LANG_EN_ZA  _("English (South Africa)")
#define XAP_STRING_ID_LANG_EN_US  _("English (US)")
#define XAP_STRING_ID_LANG_AF_ZA  _("Afrikaans")
#define XAP_STRING_ID_LANG_AK_GH  _("Akan")	// 
#define XAP_STRING_ID_LANG_SQ_AL  _("Albanian")	// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_AM_ET  _("Amharic (Ethiopia)")
#define XAP_STRING_ID_LANG_AR  _("Arabic")
#define XAP_STRING_ID_LANG_AR_EG  _("Arabic (Egypt)")
#define XAP_STRING_ID_LANG_AR_SA  _("Arabic (Saudi Arabia)")
#define XAP_STRING_ID_LANG_HY_AM  _("Armenian")
#define XAP_STRING_ID_LANG_AS_IN  _("Assamese")
#define XAP_STRING_ID_LANG_AST_ES  _("Asturian (Spain)")
#define XAP_STRING_ID_LANG_AYM_BO  _("Aymara (La Paz)")
#define XAP_STRING_ID_LANG_AYC_BO  _("Aymara (Oruro)")
#define XAP_STRING_ID_LANG_AYR  _("Central Aymara")
#define XAP_STRING_ID_LANG_EU_ES  _("Basque")
#define XAP_STRING_ID_LANG_BE_BY  _("Belarusian")
#define XAP_STRING_ID_LANG_BE_LATIN  _("Belarusian, Latin")
#define XAP_STRING_ID_LANG_BN_IN  _("Bengali")
#define XAP_STRING_ID_LANG_BR_FR  _("Breton")		// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_BG_BG  _("Bulgarian")
#define XAP_STRING_ID_LANG_CA_ES  _("Catalan")
#define XAP_STRING_ID_LANG_KW_GB  _("Cornish")		// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_CO_FR  _("Corsican")		// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_HR_HR  _("Croatian")
#define XAP_STRING_ID_LANG_ZH_HK  _("Chinese (Hong Kong)")
#define XAP_STRING_ID_LANG_ZH_CN  _("Chinese (PRC)")
#define XAP_STRING_ID_LANG_ZH_SG  _("Chinese (Singapore)")
#define XAP_STRING_ID_LANG_ZH_TW  _("Chinese (Taiwan)")
#define XAP_STRING_ID_LANG_COP_EG  _("Coptic")
#define XAP_STRING_ID_LANG_CS_CZ  _("Czech")
#define XAP_STRING_ID_LANG_DA_DK  _("Danish")
#define XAP_STRING_ID_LANG_NL_NL  _("Dutch (Netherlands)")
#define XAP_STRING_ID_LANG_EO  _("Esperanto")
#define XAP_STRING_ID_LANG_ET  _("Estonian")		// Hipi: Why not et-EE?
#define XAP_STRING_ID_LANG_FA_IR  _("Farsi")
#define XAP_STRING_ID_LANG_FI_FI  _("Finnish")
#define XAP_STRING_ID_LANG_NL_BE  _("Flemish (Belgium)")
#define XAP_STRING_ID_LANG_FR_BE  _("French (Belgium)")
#define XAP_STRING_ID_LANG_FR_CA  _("French (Canada)")
#define XAP_STRING_ID_LANG_FR_FR  _("French (France)")
#define XAP_STRING_ID_LANG_FR_CH  _("French (Switzerland)")
#define XAP_STRING_ID_LANG_FY_NL  _("Frisian")	// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_GL  _("Galician (Galego)")
#define XAP_STRING_ID_LANG_KA_GE  _("Georgian")
#define XAP_STRING_ID_LANG_DE_AT  _("German (Austria)")
#define XAP_STRING_ID_LANG_DE_DE  _("German (Germany)")
#define XAP_STRING_ID_LANG_DE_CH  _("German (Switzerland)")
#define XAP_STRING_ID_LANG_EL_GR  _("Greek")
#define XAP_STRING_ID_LANG_HA_NE  _("Hausa (Niger)")
#define XAP_STRING_ID_LANG_HA_NG  _("Hausa (Nigeria)")
#define XAP_STRING_ID_LANG_HAW_US  _("Hawaiian")
#define XAP_STRING_ID_LANG_HE_IL  _("Hebrew")
#define XAP_STRING_ID_LANG_HI_IN  _("Hindi")
#define XAP_STRING_ID_LANG_HU_HU  _("Hungarian")
#define XAP_STRING_ID_LANG_IS_IS  _("Icelandic")
#define XAP_STRING_ID_LANG_ID_ID  _("Indonesian")
#define XAP_STRING_ID_LANG_IU_CA  _("Inuktitut")
#define XAP_STRING_ID_LANG_IA  _("Interlingua")
#define XAP_STRING_ID_LANG_GA_IE  _("Irish")
#define XAP_STRING_ID_LANG_IT_IT  _("Italian (Italy)")
#define XAP_STRING_ID_LANG_JA_JP  _("Japanese")
#define XAP_STRING_ID_LANG_KN_IN  _("Kannada")
#define XAP_STRING_ID_LANG_KO_KR  _("Korean")
#define XAP_STRING_ID_LANG_KO  _("Korean")
#define XAP_STRING_ID_LANG_KU  _("Kurdish")
#define XAP_STRING_ID_LANG_LO_LA  _("Lao")
#define XAP_STRING_ID_LANG_LA_IT  _("Latin (Renaissance)")	// Is _IT the right thing here?
#define XAP_STRING_ID_LANG_LV_LV  _("Latvian")
#define XAP_STRING_ID_LANG_LT_LT  _("Lithuanian")
#define XAP_STRING_ID_LANG_MK  _("Macedonian")	// Jordi 19/10/2002 Hipi: Why not mk-MK?
#define XAP_STRING_ID_LANG_MS_MY  _("Malay")
#define XAP_STRING_ID_LANG_MI_NZ  _("Maori")
#define XAP_STRING_ID_LANG_MR_IN  _("Marathi")
#define XAP_STRING_ID_LANG_MH_MH  _("Marshallese (Marshall Islands)")
#define XAP_STRING_ID_LANG_MH_NR  _("Marshallese (Nauru)")
#define XAP_STRING_ID_LANG_MN_MN  _("Mongolian")
#define XAP_STRING_ID_LANG_NB_NO  _("Norwegian Bokmal")
#define XAP_STRING_ID_LANG_NE_NP  _("Nepali (Nepal)")
#define XAP_STRING_ID_LANG_NN_NO  _("Norwegian Nynorsk")
#define XAP_STRING_ID_LANG_OC_FR  _("Occitan")		// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_PL_PL  _("Polish")
#define XAP_STRING_ID_LANG_PS  _("Pashto")
#define XAP_STRING_ID_LANG_PT_BR  _("Portuguese (Brazil)")
#define XAP_STRING_ID_LANG_PT_PT  _("Portuguese (Portugal)")
#define XAP_STRING_ID_LANG_PA_IN  _("Punjabi (Gurmukhi)")
#define XAP_STRING_ID_LANG_PA_PK  _("Punjabi (Shahmukhi)")
#define XAP_STRING_ID_LANG_QU_BO  _("Quechua")
#define XAP_STRING_ID_LANG_QUH_BO  _("Quechua (3 vowels)")
#define XAP_STRING_ID_LANG_QUL_BO  _("Quechua (5 vowels)")
#define XAP_STRING_ID_LANG_RO_RO  _("Romanian")
#define XAP_STRING_ID_LANG_RU_RU  _("Russian (Russia)")
#define XAP_STRING_ID_LANG_SC_IT  _("Sardinian")	// Jordi 19/10/2002
#define XAP_STRING_ID_LANG_SR  _("Serbian")		// Jordi 19/10/2002 Hipi: Why not sr-YU?
#define XAP_STRING_ID_LANG_SK_SK  _("Slovak")				// or Slovakian?
#define XAP_STRING_ID_LANG_SL_SI  _("Slovenian")
#define XAP_STRING_ID_LANG_ES_MX  _("Spanish (Mexico)")
#define XAP_STRING_ID_LANG_ES_ES  _("Spanish (Spain)")
#define XAP_STRING_ID_LANG_SW  _("Swahili")
#define XAP_STRING_ID_LANG_SV_SE  _("Swedish")
#define XAP_STRING_ID_LANG_SYR  _("Syriac")
#define XAP_STRING_ID_LANG_TL_PH  _("Tagalog")
#define XAP_STRING_ID_LANG_TA_IN  _("Tamil")
#define XAP_STRING_ID_LANG_TE_IN  _("Telugu")
#define XAP_STRING_ID_LANG_TH_TH  _("Thai")
#define XAP_STRING_ID_LANG_TR_TR  _("Turkish")
#define XAP_STRING_ID_LANG_UK_UA  _("Ukrainian")
#define XAP_STRING_ID_LANG_UR_PK  _("Urdu (Pakistan)")
#define XAP_STRING_ID_LANG_UR  _("Urdu")
#define XAP_STRING_ID_LANG_UZ_UZ  _("Uzbek")
#define XAP_STRING_ID_LANG_VI_VN  _("Vietnamese")
#define XAP_STRING_ID_LANG_CY_GB  _("Welsh")
#define XAP_STRING_ID_LANG_WO_SN  _("Wolof (Senegal)")
#define XAP_STRING_ID_LANG_YI  _("Yiddish")


/* Encoding Dialog */
#define XAP_STRING_ID_DLG_UENC_EncLabel  _("Select Encoding:")
#define XAP_STRING_ID_DLG_UENC_EncTitle  _("Encoding")

/* Encoding description in different encodings; keep sorted by the numerical id's */
/* Ordered by 1) number of bits, 2) region & language, 3) platform / standard.	  */
/* 1) 7 bit, 8 bit, multibyte, Unicode. 										  */
/* 2) Western, Eastern, Asian, multilingual; common before rare.				  */
/* 3) ISO, de facto standards, MS Windows, Macintosh							  */
/* It may be desirable to change this order for each platform.					  */

/* 7 bit */
#define XAP_STRING_ID_ENC_WEST_ASCII  _("US-ASCII")
/* 8 bit */
/* Western Europe */
#define XAP_STRING_ID_ENC_WEST_ISO  _("Western European, ISO-8859-1")
#define XAP_STRING_ID_ENC_WEST_WIN  _("Western European, Windows Code Page 1252")
#define XAP_STRING_ID_ENC_US_DOS  _("Western European, DOS/Windows Code Page 437")
#define XAP_STRING_ID_ENC_MLNG_DOS  _("Western European, DOS/Windows Code Page 850")
#define XAP_STRING_ID_ENC_WEST_MAC  _("Western European, Macintosh")
#define XAP_STRING_ID_ENC_WEST_HP  _("Western European, HP")
#define XAP_STRING_ID_ENC_WEST_NXT  _("Western European, NeXT")
/* Central & Eastern Europe */
#define XAP_STRING_ID_ENC_CENT_ISO  _("Central European, ISO-8859-2")
#define XAP_STRING_ID_ENC_CENT_WIN  _("Central European, Windows Code Page 1250")
#define XAP_STRING_ID_ENC_CENT_MAC  _("Central European, Macintosh")
/* Baltic */
#define XAP_STRING_ID_ENC_BALT_ISO  _("Baltic, ISO-8859-4")
#define XAP_STRING_ID_ENC_BALT_WIN  _("Baltic, Windows Code Page 1257")
/* Greek */
#define XAP_STRING_ID_ENC_GREE_ISO  _("Greek, ISO-8859-7")
#define XAP_STRING_ID_ENC_GREE_WIN  _("Greek, Windows Code Page 1253")
#define XAP_STRING_ID_ENC_GREE_MAC  _("Greek, Macintosh")
/* Cyrillic */
#define XAP_STRING_ID_ENC_CYRL_ISO  _("Cyrillic, ISO-8859-5")
#define XAP_STRING_ID_ENC_CYRL_KOI  _("Cyrillic, KOI8-R")
#define XAP_STRING_ID_ENC_CYRL_WIN  _("Cyrillic, Windows Code Page 1251")
#define XAP_STRING_ID_ENC_CYRL_MAC  _("Cyrillic, Macintosh")
#define XAP_STRING_ID_ENC_UKRA_KOI  _("Ukrainian, KOI8-U")
#define XAP_STRING_ID_ENC_UKRA_MAC  _("Ukrainian, Macintosh")
/* Turkish */
#define XAP_STRING_ID_ENC_TURK_ISO  _("Turkish, ISO-8859-9")
#define XAP_STRING_ID_ENC_TURK_WIN  _("Turkish, Windows Code Page 1254")
#define XAP_STRING_ID_ENC_TURK_MAC  _("Turkish, Macintosh")
/* Other Roman-based encodings */
#define XAP_STRING_ID_ENC_CROA_MAC  _("Croatian, Macintosh")
#define XAP_STRING_ID_ENC_ICEL_MAC  _("Icelandic, Macintosh")
#define XAP_STRING_ID_ENC_ROMA_MAC  _("Romanian, Macintosh")
/* Thai */
#define XAP_STRING_ID_ENC_THAI_TIS  _("Thai, TIS-620")
#define XAP_STRING_ID_ENC_THAI_WIN  _("Thai, Windows Code Page 874")
#define XAP_STRING_ID_ENC_THAI_MAC  _("Thai, Macintosh")
/* Vietnamese */
#define XAP_STRING_ID_ENC_VIET_VISCII  _("Vietnamese, VISCII")
#define XAP_STRING_ID_ENC_VIET_TCVN  _("Vietnamese, TCVN")
#define XAP_STRING_ID_ENC_VIET_WIN  _("Vietnamese, Windows Code Page 1258")
/* Hebrew */
#define XAP_STRING_ID_ENC_HEBR_ISO  _("Hebrew, ISO-8859-8")
#define XAP_STRING_ID_ENC_HEBR_WIN  _("Hebrew, Windows Code Page 1255")
#define XAP_STRING_ID_ENC_HEBR_MAC  _("Hebrew, Macintosh")
/* Arabic */
#define XAP_STRING_ID_ENC_ARAB_ISO  _("Arabic, ISO-8859-6")
#define XAP_STRING_ID_ENC_ARAB_WIN  _("Arabic, Windows Code Page 1256")
#define XAP_STRING_ID_ENC_ARAB_MAC  _("Arabic, Macintosh")
/* Armenian */
#define XAP_STRING_ID_ENC_ARME_ARMSCII  _("Armenian, ARMSCII-8")
/* Georgian */
#define XAP_STRING_ID_ENC_GEOR_ACADEMY  _("Georgian, Academy")
#define XAP_STRING_ID_ENC_GEOR_PS  _("Georgian, PS")
/* Multibyte CJK */
/* Chinese Simplified */
#define XAP_STRING_ID_ENC_CHSI_EUC  _("Chinese Simplified, EUC-CN (GB2312)")
#define XAP_STRING_ID_ENC_CHSI_GB  _("Chinese Simplified, GB_2312-80")	// Cf. EUC
#define XAP_STRING_ID_ENC_CHSI_HZ  _("Chinese Simplified, HZ")
#define XAP_STRING_ID_ENC_CHSI_WIN  _("Chinese Simplified, Windows Code Page 936")
/* Chinese Traditional */
#define XAP_STRING_ID_ENC_CHTR_BIG5  _("Chinese Traditional, BIG5")
#define XAP_STRING_ID_ENC_CHTR_BIG5HKSCS  _("Chinese Traditional, BIG5-HKSCS")
#define XAP_STRING_ID_ENC_CHTR_EUC  _("Chinese Traditional, EUC-TW")
#define XAP_STRING_ID_ENC_CHTR_WIN  _("Chinese Traditional, Windows Code Page 950")
/* Japanese */
#define XAP_STRING_ID_ENC_JAPN_ISO  _("Japanese, ISO-2022-JP")
#define XAP_STRING_ID_ENC_JAPN_EUC  _("Japanese, EUC-JP")
#define XAP_STRING_ID_ENC_JAPN_SJIS  _("Japanese, Shift-JIS")
#define XAP_STRING_ID_ENC_JAPN_WIN  _("Japanese, Windows Code Page 932")
/* Korean */
#define XAP_STRING_ID_ENC_KORE_KSC  _("Korean, KSC_5601") // ISO
#define XAP_STRING_ID_ENC_KORE_EUC  _("Korean, EUC-KR")
#define XAP_STRING_ID_ENC_KORE_JOHAB  _("Korean, Johab")
#define XAP_STRING_ID_ENC_KORE_WIN  _("Korean, Windows Code Page 949")
/* Unicode */
#define XAP_STRING_ID_ENC_UNIC_UTF_7  _("Unicode UTF-7")
#define XAP_STRING_ID_ENC_UNIC_UTF_8  _("Unicode UTF-8")
//#define XAP_STRING_ID_ENC_UNIC_UTF_16  _("Unicode UTF-16")
#define XAP_STRING_ID_ENC_UNIC_UTF_16BE  _("Unicode UTF-16 Big Endian")
#define XAP_STRING_ID_ENC_UNIC_UTF_16LE  _("Unicode UTF-16 Little Endian")
//#define XAP_STRING_ID_ENC_UNIC_UTF_32  _("Unicode UTF-32")
#define XAP_STRING_ID_ENC_UNIC_UTF_32BE  _("Unicode UTF-32 Big Endian")
#define XAP_STRING_ID_ENC_UNIC_UTF_32LE  _("Unicode UTF-32 Little Endian")
//#define XAP_STRING_ID_ENC_UNIC_UCS2  _("Unicode UCS-2")
#define XAP_STRING_ID_ENC_UNIC_UCS_2BE  _("Unicode UCS-2 Big Endian")
#define XAP_STRING_ID_ENC_UNIC_UCS_2LE  _("Unicode UCS-2 Little Endian")
//#define XAP_STRING_ID_ENC_UNIC_UCS4  _("Unicode UCS-4")
#define XAP_STRING_ID_ENC_UNIC_UCS_4BE  _("Unicode UCS-4 Big Endian")
#define XAP_STRING_ID_ENC_UNIC_UCS_4LE  _("Unicode UCS-4 Little Endian")

/* HTML Options Dialog */
#define XAP_STRING_ID_DLG_HTMLOPT_ExpTitle  _("HTML Export Options")
#define XAP_STRING_ID_DLG_HTMLOPT_ExpLabel  _("Select HTML export options:")
#define XAP_STRING_ID_DLG_HTMLOPT_ExpSave  _("Save Settings")
#define XAP_STRING_ID_DLG_HTMLOPT_ExpRestore  _("Restore Settings")
#define XAP_STRING_ID_DLG_HTMLOPT_ExpIs4  _("Export as HTML 4.01")
#define XAP_STRING_ID_DLG_HTMLOPT_ExpAbiWebDoc  _("Export with PHP instructions")
#define XAP_STRING_ID_DLG_HTMLOPT_ExpDeclareXML  _("Declare as XML (version 1.0)")
#define XAP_STRING_ID_DLG_HTMLOPT_ExpAllowAWML  _("Allow extra markup in AWML namespace")
#define XAP_STRING_ID_DLG_HTMLOPT_ExpEmbedCSS  _("Embed (CSS) style sheet")
#define XAP_STRING_ID_DLG_HTMLOPT_ExpEmbedImages  _("Embed images in URLs (Base64-encoded)")
#define XAP_STRING_ID_DLG_Exit_CloseWithoutSaving  _("Close &Without Saving")

	/* X11 input methods */
#define XAP_STRING_ID_XIM_Methods  _("Input Methods")

//since this string goes with XAP preference, I put it here, rather
//than ap_String_Id.h
#define XAP_STRING_ID_DLG_Options_Label_LangWithKeyboard  _("Change Language when changing keyboard")
#define XAP_STRING_ID_DLG_Options_Label_DirMarkerAfterClosingParenthesis  _("Auto-insert direction markers")

#define XAP_STRING_ID_DLG_History_WindowLabel  _("Document History")
#define XAP_STRING_ID_DLG_History_DocumentDetails  _("Document Details")
#define XAP_STRING_ID_DLG_History_Path  _("Document name:")
#define XAP_STRING_ID_DLG_History_Created  _("Created:")
#define XAP_STRING_ID_DLG_History_Version  _("Version:")
#define XAP_STRING_ID_DLG_History_LastSaved  _("Last saved:")
#define XAP_STRING_ID_DLG_History_EditTime  _("Editing time:")
#define XAP_STRING_ID_DLG_History_Id  _("Identifier:")

#define XAP_STRING_ID_DLG_History_List_Title  _("Version history")
#define XAP_STRING_ID_DLG_History_Version_Version  _("Version")
#define XAP_STRING_ID_DLG_History_Version_Started  _("Created")
#define XAP_STRING_ID_DLG_History_Version_AutoRevisioned  _("Auto-revision")
	
#define XAP_STRING_ID_DLG_DocComparison_WindowLabel  _("Document Comparison")
#define XAP_STRING_ID_DLG_DocComparison_DocsCompared  _("Documents compared")
#define XAP_STRING_ID_DLG_DocComparison_Results  _("Results")
#define XAP_STRING_ID_DLG_DocComparison_Relationship  _("Relationship:")
#define XAP_STRING_ID_DLG_DocComparison_Content  _("Content:")
#define XAP_STRING_ID_DLG_DocComparison_Fmt  _("Format:")
#define XAP_STRING_ID_DLG_DocComparison_Styles  _("Styles:")

#define XAP_STRING_ID_DLG_DocComparison_Identical  _("identical")
#define XAP_STRING_ID_DLG_DocComparison_Unrelated  _("unrelated")
#define XAP_STRING_ID_DLG_DocComparison_Siblings  _("siblings")
#define XAP_STRING_ID_DLG_DocComparison_Diverging  _("diverging after version %d of %s")
#define XAP_STRING_ID_DLG_DocComparison_DivergingPos  _("diverging after document position %d")
#define XAP_STRING_ID_DLG_DocComparison_TestSkipped  _("(test skipped)")
#define XAP_STRING_ID_DLG_DocComparison_Different  _("different")

#endif /* XAP_STRING_ID_H */
