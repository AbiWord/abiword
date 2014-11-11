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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

dcl(MSG_ImportingDoc, "Importing Document...")
dcl(MSG_BuildingDoc,  "Building Document:")
dcl(MSG_ParagraphsImported,  "Imported Paragraph")
dcl(MSG_AutoRevision, "Autorevision")
dcl(MSG_HistoryPartRestore1, "AbiWord cannot fully restore version %d of the document because the version information is incomplete.")
dcl(MSG_HistoryPartRestore2, "The nearest version that can be restored fully is %d. Would you like to restore this version instead? To partially restore version %d press No.")
dcl(MSG_HistoryPartRestore3, "To continue anyway, press OK.")
dcl(MSG_HistoryPartRestore4, "To quit the restoration attempt, press Cancel.")
dcl(MSG_HistoryNoRestore, "AbiWord cannot restore version %d of the document because the version information is missing.")
dcl(MSG_HistoryConfirmSave, "You have to save changes to document %s before proceeding. Save now?")

dcl(MSG_NoUndo, "This operation cannot be undone. Are you sure you want to proceed?")

/* Default name for new, untitled document */
dcl(UntitledDocument,			"Untitled%d")
dcl(ReadOnly,					"Read-Only")

/*  Styles */
dcl(STYLE_NONE,			    "None")		//xgettext:msgctxt
dcl(STYLE_NUMBER_LIST,		"Numbered List")
dcl(STYLE_PLAIN_TEXT,		"Plain Text")
dcl(STYLE_NORMAL,			"Normal")
dcl(STYLE_HEADING1, 		"Heading 1")
dcl(STYLE_HEADING2, 		"Heading 2")
dcl(STYLE_HEADING3, 		"Heading 3")
dcl(STYLE_HEADING4, 		"Heading 4")
dcl(STYLE_TOCHEADING, 		"Contents Header")
dcl(STYLE_TOCHEADING1, 		"Contents 1")
dcl(STYLE_TOCHEADING2, 		"Contents 2")
dcl(STYLE_TOCHEADING3, 		"Contents 3")
dcl(STYLE_TOCHEADING4, 		"Contents 4")
dcl(STYLE_BLOCKTEXT,		"Block Text")
dcl(STYLE_LOWERCASELIST,	"Lower Case List")
dcl(STYLE_UPPERCASTELIST,	"Upper Case List")
dcl(STYLE_LOWERROMANLIST,	"Lower Roman List")
dcl(STYLE_UPPERROMANLIST,	"Upper Roman List")
dcl(STYLE_BULLETLIST,		"Bullet List")
dcl(STYLE_DASHEDLIST,		"Dashed List")
dcl(STYLE_SQUARELIST,		"Square List")
dcl(STYLE_TRIANGLELIST,		"Triangle List")
dcl(STYLE_DIAMONLIST,		"Diamond List")
dcl(STYLE_STARLIST,			"Star List")
dcl(STYLE_TICKLIST,			"Tick List")
dcl(STYLE_BOXLIST,			"Box List")
dcl(STYLE_HANDLIST,			"Hand List")
dcl(STYLE_HEARTLIST,		"Heart List")
dcl(STYLE_ARROWHEADLIST,	"Arrowhead List")
dcl(STYLE_CHAPHEADING,		"Chapter Heading")
dcl(STYLE_SECTHEADING,		"Section Heading")
dcl(STYLE_ENDREFERENCE,		"Endnote Reference")
dcl(STYLE_ENDTEXT,			"Endnote Text")
dcl(STYLE_ENDNOTE,			"Endnote")
dcl(STYLE_FOOTREFERENCE,	"Footnote Reference")
dcl(STYLE_FOOTTEXT,			"Footnote Text")
dcl(STYLE_FOOTNOTE,			"Footnote")
dcl(STYLE_NUMHEAD1,			"Numbered Heading 1")
dcl(STYLE_NUMHEAD2,			"Numbered Heading 2")
dcl(STYLE_NUMHEAD3,			"Numbered Heading 3")
dcl(STYLE_IMPLIES_LIST,		"Implies List")
dcl(STYLE_DELIM_CHAPTER,	"Chapter")
dcl(STYLE_DELIM_SECTION,	"Section")


/* Common to many dialogs */
dcl(DLG_OK, 					"OK")
dcl(DLG_Cancel, 				"Cancel")
dcl(DLG_Close,					"Close")
dcl(DLG_Insert, 				"&Insert")
dcl(DLG_Update, 				"Update")
dcl(DLG_Apply,					"Apply")
dcl(DLG_Delete, 				"Delete")
dcl(DLG_Compare, 				"Compare")
dcl(DLG_Select, 				"Select")
dcl(DLG_Merge, 				    "Merge")
dcl(DLG_Show, 				    "Show")
dcl(DLG_Restore,			    "Restore")
dcl(DLG_HelpButton,			    "&Help")
dcl(DLG_Save,					"Sa&ve")
dcl(DLG_Open,					"&Open")

/* Units */
dcl(DLG_Unit_inch,				"inch")
dcl(DLG_Unit_cm,				"cm")
dcl(DLG_Unit_mm,				"mm")
dcl(DLG_Unit_points,			"points")
dcl(DLG_Unit_pica,				"pica")

/* Message box */
/* These are tagged "UnixMB" because the underscores precede accelerator
   characters.	It should be an ampersand on Windows, but Windows doesn't
   need a hand-constructed message box (Win32 API provides one).
*/
dcl(DLG_UnixMB_Yes, 			"_Yes")
dcl(DLG_UnixMB_No,				"_No")

dcl(DLG_MB_Yes,					"Yes")
dcl(DLG_MB_No,					"No")

/* More Windows dialog */
dcl(DLG_MW_MoreWindows, 		"View Document")
dcl(DLG_MW_Activate,			"View:")
dcl(DLG_MW_AvailableDocuments,		"Available Documents:")
dcl(DLG_MW_ViewButton,			"&View")

/* Remove Toolbar Icon */
dcl(DLG_Remove_Icon,			"Do you want to remove this icon from the toolbar?")

/* Font Selector dialog */
dcl(DLG_UFS_FontTitle,			"Font")
dcl(DLG_UFS_FontLabel,			"Font:")
dcl(DLG_UFS_StyleLabel, 		"Style:")
dcl(DLG_UFS_SizeLabel,			"Size:")
dcl(DLG_UFS_EncodingLabel,		"Encoding:")
dcl(DLG_UFS_EffectsFrameLabel,	"Effects:")
dcl(DLG_UFS_StrikeoutCheck, 	"Strike")
dcl(DLG_UFS_UnderlineCheck, 	"Underline")
dcl(DLG_UFS_OverlineCheck,		"Overline")
dcl(DLG_UFS_HiddenCheck,        "Hidden")
dcl(DLG_UFS_TransparencyCheck,	"Set no Highlight Color")
dcl(DLG_UFS_FontTab,			"Font")
dcl(DLG_UFS_ColorTab,			"Text Color")
dcl(DLG_UFS_BGColorTab, 		"HighLight Color")
dcl(DLG_UFS_StyleRegular,		"Regular")
dcl(DLG_UFS_StyleItalic,		"Italic")
dcl(DLG_UFS_StyleBold,			"Bold")
dcl(DLG_UFS_StyleBoldItalic,	"Bold Italic")
dcl(DLG_UFS_ToplineCheck,		"Topline")
dcl(DLG_UFS_BottomlineCheck,	"Bottomline")
dcl(DLG_UFS_ColorLabel, 		"Color:")
dcl(DLG_UFS_ScriptLabel,		"Script:")
dcl(DLG_UFS_SampleFrameLabel,	"Sample")
dcl(DLG_UFS_SuperScript, 		"Superscript")
dcl(DLG_UFS_SubScript, 			"Subscript")
dcl(DLG_UFS_Effects_None, "(None)")
dcl(DLG_UFS_Effects_UpperCase, "Uppercase")
dcl(DLG_UFS_Effects_LowerCase, "Lowercase")
dcl(DLG_UFS_Effects_TitleCase, "Title Case")
dcl(DLG_UFS_Effects_SmallCaps, "Small Capitals")

/* Unix FileOpenSaveAs dialog */
dcl(DLG_FOSA_OpenTitle, 				"Open File")
dcl(DLG_FOSA_SaveAsTitle,				"Save File As")
dcl(DLG_FOSA_ExportTitle, "Export File")
dcl(DLG_FOSA_ImportTitle, "Import File")
dcl(DLG_FOSA_InsertTitle, "Insert File")
dcl(DLG_FOSA_InsertMath,  "Insert Math File")
dcl(DLG_FOSA_InsertObject,  "Insert Embeddable Object")
dcl(DLG_FOSA_PrintToFileTitle,			"Print To File")
dcl(DLG_FOSA_RecordToFileTitle,			"Record Editing to File")
dcl(DLG_FOSA_ReplayFromFileTitle,		"Replay Editing from File")
dcl(DLG_FOSA_FileOpenTypeLabel, 		"Open file as &type:")
dcl(DLG_FOSA_FileInsertMath, 		    "Insert MathML file:")
dcl(DLG_FOSA_FileInsertObject, 		    "Insert Embeddable Object file:")
dcl(DLG_FOSA_FileSaveTypeLabel, 		"Save file as &type:")
dcl(DLG_FOSA_FilePrintTypeLabel,		"Print file as &type:")
dcl(DLG_FOSA_RecordToFileLabel,		    "File to record editing:")
dcl(DLG_FOSA_ReplayFromFileLabel,		"File to replay editing:")
dcl(DLG_FOSA_FileTypeAutoDetect,		"Automatically Detected")
dcl(DLG_FOSA_ALLIMAGES, "All Image Files")
dcl(DLG_FOSA_ALLDOCS, "All Documents")
dcl(DLG_FOSA_ALL, "All (*.*)")
dcl(DLG_InvalidPathname,				"Invalid pathname.")
dcl(DLG_NoSaveFile_DirNotExist, 		"A directory in the given pathname does not exist.")
dcl(DLG_NoSaveFile_DirNotWriteable, 	"The directory '%s' is write-protected.")
dcl(DLG_OverwriteFile,					"File already exists.  Overwrite file '%s'?")
dcl(DLG_FOSA_ExtensionDoesNotMatch,		"The given file extension does not match the chosen file type. Do you want to use this name anyway?")

/* Password dialog */
dcl(DLG_Password_Title,					"Enter Password")
dcl(DLG_Password_Password,				"Password:")

/* Zoom dialog */
dcl(DLG_Zoom_ZoomTitle, 				"Zoom")
dcl(DLG_Zoom_RadioFrameCaption, 		"Zoom to")
dcl(DLG_Zoom_200,						"&200%")
dcl(DLG_Zoom_100,						"&100%")
dcl(DLG_Zoom_75,						"&75%")
dcl(DLG_Zoom_PageWidth, 				"&Page width")
dcl(DLG_Zoom_WholePage, 				"&Whole page")
dcl(DLG_Zoom_Percent,					"P&ercent:")
dcl(DLG_Zoom_PreviewFrame,				"Preview")

/* Zoom tool bar -- Truncated to fit small combobox size*/
dcl(TB_Zoom_PageWidth,					"Page Width")
dcl(TB_Zoom_WholePage,					"Whole Page")
dcl(TB_Zoom_Percent, "Other...")

/* Font tool bar*/
dcl(TB_Font_Symbol,						"Symbols")

/* Unix Print dialog */
dcl(DLG_UP_PrintTitle,					"Print")
dcl(DLG_UP_PrintPreviewTitle,					   "AbiWord: Print Preview")
dcl(DLG_UP_PrintTo, 					"Print to: ")
dcl(DLG_UP_Printer, 					"Printer")
dcl(DLG_UP_File,						"File")
dcl(DLG_UP_PrinterCommand,				"Printer command: ")
dcl(DLG_UP_PageRanges,					"Page ranges:")
dcl(DLG_UP_All, 						"All")
dcl(DLG_UP_From,						"From: ")
dcl(DLG_UP_To,							" to ")
dcl(DLG_UP_Selection,					"Selection")
dcl(DLG_UP_Collate, 					"Collate")
dcl(DLG_UP_EmbedFonts,					"Embed Fonts")
dcl(DLG_UP_Copies,						"Copies: ")
dcl(DLG_UP_PrintButton, 				"Print")
dcl(DLG_UP_InvalidPrintString,			"The print command string is not valid.")
dcl(DLG_UP_PrintIn, 					"Print in: ")
dcl(DLG_UP_BlackWhite,					"Black & White")
dcl(DLG_UP_Grayscale,					"Grayscale")
dcl(DLG_UP_Color,						"Color")

/* Insert Symbol dialog */
dcl(DLG_Insert_SymbolTitle, 			"Insert Symbol")

/* Insert Picture Preview Dialog (Win32) */
dcl(DLG_IP_Title,						"Insert Picture")
dcl(DLG_IP_Activate_Label,				"Preview Picture")
dcl(DLG_IP_No_Picture_Label,			"No Picture")
dcl(DLG_IP_Height_Label,				"Height: ")
dcl(DLG_IP_Width_Label, 				"Width:  ")
dcl(DLG_IP_Button_Label,				"Insert")

/* Plugin dialog */
dcl(DLG_PLUGIN_MANAGER_TITLE,			"AbiWord Plugin Manager")
dcl(DLG_PLUGIN_MANAGER_ACTIVE,			"Active Plugins")
dcl(DLG_PLUGIN_MANAGER_DEACTIVATE,		"Deactivate plugin")
dcl(DLG_PLUGIN_MANAGER_DEACTIVATE_ALL,		"Deactivate all plugins")
dcl(DLG_PLUGIN_MANAGER_INSTALL, 		"Install new plugin")
dcl(DLG_PLUGIN_MANAGER_LIST,			"Plugin List")
dcl(DLG_PLUGIN_MANAGER_NAME,			"Name:")
dcl(DLG_PLUGIN_MANAGER_DESC,			"Description:")
dcl(DLG_PLUGIN_MANAGER_AUTHOR,			"Author:")
dcl(DLG_PLUGIN_MANAGER_VERSION, 		"Version:")
dcl(DLG_PLUGIN_MANAGER_DETAILS, 		"Plugin Details")
dcl(DLG_PLUGIN_MANAGER_NOT_AVAILABLE, 		"Not available")

/* spellchecker */
dcl(SPELL_CANTLOAD_DICT,				"Could not load the dictionary for the %s language")
dcl(SPELL_CANTLOAD_DLL,					"AbiWord cannot find the spelling file %s.dll\nPlease download and install Aspell from http://aspell.net/win32/")

/* plugin error messages */
dcl(DLG_PLUGIN_MANAGER_COULDNT_LOAD,	"Could not activate/load plugin")
dcl(DLG_PLUGIN_MANAGER_COULDNT_UNLOAD,	"Could not deactivate plugin")
dcl(DLG_PLUGIN_MANAGER_NONE_SELECTED,	"No plugin selected")

/* Language Dialog */
dcl(DLG_ULANG_LangTitle,				"Set Language")
dcl(DLG_ULANG_LangLabel,				"Select Language:")
dcl(DLG_ULANG_AvailableLanguages,			"Available Languages:")
dcl(DLG_ULANG_SetLangButton,				"&Set Language")
dcl(DLG_ULANG_DefaultLangLabel,         "Default language: ")
dcl(DLG_ULANG_DefaultLangChkbox,        "Make default for document")

/* ClipArt Dialog */
dcl(DLG_CLIPART_Title, "Clip Art")
dcl(DLG_CLIPART_Loading, "Loading Clip Art...")
dcl(DLG_CLIPART_Error, "Clip Art could not be loaded.")

/* About Dialog */
dcl(DLG_ABOUT_Title,					"About %s")
dcl(DLG_ABOUT_Description,              "%s is an Open Source application licensed under the GNU GPL.\nYou are free to redistribute this application.")
dcl(DLG_ABOUT_Version,                  "Version: %s")
dcl(DLG_ABOUT_Build,                    "Build options: %s")
dcl(DLG_ABOUT_URL,                      "For more information: http://www.abisource.com/")

/* image size dialog */
dcl(DLG_Image_Title, "Image Properties")
dcl(DLG_Image_DescTabLabel, "General")
dcl(DLG_Image_WrapTabLabel, "&Wrapping")
dcl(DLG_Image_PlacementTabLabel, "&Placement")
dcl(DLG_Image_Width, "Width:")
dcl(DLG_Image_Height, "Height:")
dcl(DLG_Image_Aspect, "Preserve aspect ratio")
dcl(DLG_Image_LblTitle, "Title:")
dcl(DLG_Image_LblDescription, "Description:")
dcl(DLG_Image_ImageSize, "Image Size")
dcl(DLG_Image_ImageDesc, "Image Name")
dcl(DLG_Image_TextWrapping, "Text Wrapping")
dcl(DLG_Image_Placement, "Image Placement")
dcl(DLG_Image_InLine, "Image placed in-line (no text wrapping)")
dcl(DLG_Image_WrappedNone, "Image floats above text")
dcl(DLG_Image_WrappedRight, "Text wrapped to the Right of the Image")
dcl(DLG_Image_WrappedLeft, "Text wrapped to the Left of the Image")
dcl(DLG_Image_WrappedBoth, "Text wrapped on both sides of the Image")
dcl(DLG_Image_PlaceParagraph, "Position relative to nearest paragraph")
dcl(DLG_Image_PlaceColumn, "Position relative to its Column")
dcl(DLG_Image_PlacePage, "Position relative to its Page")
dcl(DLG_Image_WrapType, "Type of text wrapping")
dcl(DLG_Image_SquareWrap, "Square text wrapping")
dcl(DLG_Image_TightWrap, "Tight text wrapping")

/* ListDocuments Dialog */
dcl(DLG_LISTDOCS_Title,					"Opened Documents")
dcl(DLG_LISTDOCS_Heading1,				"Choose document from the list:")

/*
For insert Table widget
*/
dcl(TB_InsertNewTable,                   "Insert New Table")
dcl(TB_Rows_x_Cols_Table,                "Table")
dcl(TB_ClearBackground,"Clear Background")
dcl(TB_ClearForeground,"Clear Foreground")

/*
	  Language names, arranged alphabetically.
	
	  Please add a country (in parentheses) only if there is more than one 
	  entry for the language (or if it's not the main dialect).

	  A specification in parentheses shall refer to the country code, if   
	  possible, rather than to geographical or descriptive information.

	  Add other information such as variant information solely by using a 
	  comma.
	
	  Please when translating the first entry, put it also into parenthesis
	  or, surround it by some other characters, so that it will appear on the top
	  of the list when sorted alphabetically.
*/
dcl(LANG_0,     							"(no proofing)")
dcl(LANG_ACH,   							"Acholi")
dcl(LANG_AF_ZA, 							"Afrikaans")
dcl(LANG_AK_GH, 							"Akan")	//
dcl(LANG_SQ_AL, 							"Albanian")	// Jordi 19/10/2002
dcl(LANG_AM_ET, 							"Amharic")
dcl(LANG_AR, 								"Arabic")
dcl(LANG_AR_EG, 							"Arabic (Egypt)")
dcl(LANG_AR_SA, 							"Arabic (Saudi Arabia)")
dcl(LANG_HY_AM, 							"Armenian")
dcl(LANG_AS_IN, 							"Assamese")
dcl(LANG_AST_ES, 							"Asturian")
dcl(LANG_AYR, 								"Aymara, Central")
dcl(LANG_AYM_BO, 							"Aymara (Central/Northern Bolivia)")
dcl(LANG_AYC_BO, 							"Aymara (Southern Bolivia)")
dcl(LANG_BM,    							"Bamanankan")
dcl(LANG_EU_ES, 							"Basque")
dcl(LANG_BE_BY, 							"Belarusian")
dcl(LANG_BE_LATIN,							"Belarusian, Latin")
dcl(LANG_BN_IN,                             "Bengali")
dcl(LANG_NB_NO, 							"Bokmal")
dcl(LANG_BS_BA, 							"Bosnian")
dcl(LANG_BR_FR,								"Breton")		// Jordi 19/10/2002
dcl(LANG_BG_BG, 							"Bulgarian")
dcl(LANG_CA_ES, 							"Catalan")
dcl(LANG_CGG,   							"Chiga")
dcl(LANG_ZH_HK, 							"Chinese (Hong Kong)")
dcl(LANG_ZH_CN, 							"Chinese (PRC)")
dcl(LANG_ZH_SG, 							"Chinese (Singapore)")
dcl(LANG_ZH_TW, 							"Chinese (Taiwan)")
dcl(LANG_COP_EG, 							"Coptic")
dcl(LANG_KW_GB,								"Cornish")		// Jordi 19/10/2002
dcl(LANG_CO_FR,								"Corsican")		// Jordi 19/10/2002
dcl(LANG_HR_HR,								"Croatian")
dcl(LANG_CS_CZ, 							"Czech")
dcl(LANG_DA_DK, 							"Danish")
dcl(LANG_FA_AF, 							"Dari")
dcl(LANG_DV_MV, 							"Dhivehi")
dcl(LANG_NL_NL, 							"Dutch")
dcl(LANG_EN_AU, 							"English (Australia)")
dcl(LANG_EN_CA, 							"English (Canada)")
dcl(LANG_EN_IN, 							"English (India)")
dcl(LANG_EN_IE, 							"English (Ireland)")
dcl(LANG_EN_NZ, 							"English (New Zealand)")
dcl(LANG_EN_ZA, 							"English (South Africa)")
dcl(LANG_EN_GB, 							"English (UK)")
dcl(LANG_EN_US, 							"English (US)")
dcl(LANG_EO,								"Esperanto")
dcl(LANG_ET,								"Estonian")		// Hipi: Why not et-EE?
dcl(LANG_FA_IR, 							"Farsi")
dcl(LANG_FIL_PH, 							"Filipino")
dcl(LANG_FI_FI, 							"Finnish")
dcl(LANG_NL_BE, 							"Flemish")
dcl(LANG_FR_BE, 							"French (Belgium)")
dcl(LANG_FR_CA, 							"French (Canada)")
dcl(LANG_FR_FR, 							"French (France)")
dcl(LANG_FR_CH, 							"French (Switzerland)")
dcl(LANG_FY_NL,								"Frisian")	// Jordi 19/10/2002
dcl(LANG_FF,								"Fulah")
dcl(LANG_GL, 								"Galician")
dcl(LANG_LG, 								"Ganda")
dcl(LANG_KA_GE, 							"Georgian")
dcl(LANG_DE_AT, 							"German (Austria)")
dcl(LANG_DE_DE, 							"German (Germany)")
dcl(LANG_DE_CH, 							"German (Switzerland)")
dcl(LANG_EL_GR, 							"Greek")
dcl(LANG_HA_NE, 							"Hausa (Niger)")
dcl(LANG_HA_NG, 							"Hausa (Nigeria)")
dcl(LANG_HAW_US,                            "Hawaiian")
dcl(LANG_HE_IL, 							"Hebrew")
dcl(LANG_HI_IN, 							"Hindi")
dcl(LANG_HU_HU, 							"Hungarian")
dcl(LANG_IS_IS, 							"Icelandic")
dcl(LANG_ID_ID,	 							"Indonesian")
dcl(LANG_IA,	 							"Interlingua")
dcl(LANG_IU_CA,                             "Inuktitut")
dcl(LANG_GA_IE, 							"Irish")
dcl(LANG_IT_IT, 							"Italian")
dcl(LANG_JA_JP, 							"Japanese")
dcl(LANG_KN_IN,                             "Kannada")
dcl(LANG_KK_KZ,                             "Kazakh")
dcl(LANG_KM_KH, 							"Khmer")
dcl(LANG_KO, 								"Korean")
dcl(LANG_KO_KR, 							"Korean (South Korea)")
dcl(LANG_KU, 								"Kurdish")
dcl(LANG_LO_LA,                             "Lao")
dcl(LANG_LA_IT, 							"Latin")	// Is _IT the right thing here?
dcl(LANG_LV_LV, 							"Latvian")
dcl(LANG_LT_LT, 							"Lithuanian")
dcl(LANG_JBO,   							"Lojban")
dcl(LANG_MK,								"Macedonian")	// Jordi 19/10/2002 Hipi: Why not mk-MK?
dcl(LANG_MG_MG,                             "Malagasy")
dcl(LANG_MS_MY,                             "Malay")
dcl(LANG_MNK_SN,							"Mandinka")
dcl(LANG_MI_NZ,                             "Maori")
dcl(LANG_MR_IN,                             "Marathi")
dcl(LANG_MH_MH, 							"Marshallese (Marshall Islands)")
dcl(LANG_MH_NR, 							"Marshallese (Nauru)")
dcl(LANG_MN_MN,                             "Mongolian")
dcl(LANG_NE_NP, 							"Nepali")
dcl(LANG_NN_NO, 							"Nynorsk")
dcl(LANG_OC_FR,								"Occitan")		// Jordi 19/10/2002
dcl(LANG_PS,								"Pashto")
dcl(LANG_PL_PL, 							"Polish")
dcl(LANG_PT_BR, 							"Portuguese (Brazil)")
dcl(LANG_PT_PT, 							"Portuguese (Portugal)")
dcl(LANG_PA_IN,                             "Punjabi (India)")
dcl(LANG_PA_PK,                             "Punjabi (Pakistan)")
dcl(LANG_QU_BO,                             "Quechua")
dcl(LANG_QUZ,                               "Quechua (Cusco)")
dcl(LANG_QUL_BO,                            "Quechua (Northern Bolivia)")
dcl(LANG_QUH_BO,                            "Quechua (Southern Bolivia)")
dcl(LANG_RO_RO, 							"Romanian")
dcl(LANG_RU_RU, 							"Russian")
dcl(LANG_RU_PETR1708,						"Russian, pre-1918")
dcl(LANG_SC_IT,								"Sardinian")	// Jordi 19/10/2002
dcl(LANG_SR,								"Serbian")		// Jordi 19/10/2002 Hipi: Why not sr-YU?
dcl(LANG_SR_LATIN,							"Serbian, Latin")
dcl(LANG_SK_SK, 							"Slovak")				// or Slovakian?
dcl(LANG_SL_SI, 							"Slovenian")
dcl(LANG_SON,   							"Songhay")
dcl(LANG_ES_MX, 							"Spanish (Mexico)")
dcl(LANG_ES_ES, 							"Spanish (Spain)")
dcl(LANG_SW,                                "Swahili")
dcl(LANG_SV_SE, 							"Swedish")
dcl(LANG_SYR,                               "Syriac")
dcl(LANG_TL_PH,                             "Tagalog")
dcl(LANG_TA_IN,                             "Tamil")
dcl(LANG_TE_IN,                             "Telugu")
dcl(LANG_TH_TH, 							"Thai")
dcl(LANG_TR_TR, 							"Turkish")
dcl(LANG_UK_UA, 							"Ukrainian")
dcl(LANG_UR, 								"Urdu")
dcl(LANG_UR_PK, 							"Urdu (Pakistan)")
dcl(LANG_UZ_UZ, 							"Uzbek")
dcl(LANG_VI_VN, 							"Vietnamese")
dcl(LANG_CY_GB, 							"Welsh")
dcl(LANG_WO_SN, 							"Wolof")
dcl(LANG_YI,								"Yiddish")
dcl(LANG_ZU,								"Zulu")


/* Encoding Dialog */
dcl(DLG_UENC_EncLabel,				"Select Encoding:")
dcl(DLG_UENC_EncTitle,				"Encoding")

/* Encoding description in different encodings; keep sorted by the numerical id's */
/* Ordered by 1) number of bits, 2) region & language, 3) platform / standard.	  */
/* 1) 7 bit, 8 bit, multibyte, Unicode. 										  */
/* 2) Western, Eastern, Asian, multilingual; common before rare.				  */
/* 3) ISO, de facto standards, MS Windows, Macintosh							  */
/* It may be desirable to change this order for each platform.					  */

/* 7 bit */
dcl(ENC_WEST_ASCII, 					"US-ASCII")
/* 8 bit */
/* Western Europe */
dcl(ENC_WEST_ISO,						"Western European, ISO-8859-1")
dcl(ENC_WEST_WIN,						"Western European, Windows Code Page 1252")
dcl(ENC_US_DOS,						"English, DOS/Windows Code Page 437")
dcl(ENC_MLNG_DOS,						"Western European, DOS/Windows Code Page 850")
dcl(ENC_WEST_MAC,						"Western European, Macintosh")
dcl(ENC_WEST_HP,						"Western European, HP")
dcl(ENC_WEST_NXT,						"Western European, NeXT")
/* Central & Eastern Europe */
dcl(ENC_CENT_ISO,						"Central European, ISO-8859-2")
dcl(ENC_CENT_WIN,						"Central European, Windows Code Page 1250")
dcl(ENC_CENT_MAC,						"Central European, Macintosh")
/* Baltic */
dcl(ENC_BALT_ISO,						"Baltic, ISO-8859-4")
dcl(ENC_BALT_WIN,						"Baltic, Windows Code Page 1257")
/* Greek */
dcl(ENC_GREE_ISO,						"Greek, ISO-8859-7")
dcl(ENC_GREE_WIN,						"Greek, Windows Code Page 1253")
dcl(ENC_GREE_MAC,						"Greek, Macintosh")
/* Cyrillic */
dcl(ENC_CYRL_ISO,						"Cyrillic, ISO-8859-5")
dcl(ENC_CYRL_KOI,						"Cyrillic, KOI8-R")
dcl(ENC_CYRL_WIN,						"Cyrillic, Windows Code Page 1251")
dcl(ENC_CYRL_MAC,						"Cyrillic, Macintosh")
dcl(ENC_UKRA_KOI,						"Ukrainian, KOI8-U")
dcl(ENC_UKRA_MAC,						"Ukrainian, Macintosh")
/* Turkish */
dcl(ENC_TURK_ISO,						"Turkish, ISO-8859-9")
dcl(ENC_TURK_WIN,						"Turkish, Windows Code Page 1254")
dcl(ENC_TURK_MAC,						"Turkish, Macintosh")
/* Other Roman-based encodings */
dcl(ENC_CROA_MAC,						"Croatian, Macintosh")
dcl(ENC_ICEL_MAC,						"Icelandic, Macintosh")
dcl(ENC_ROMA_MAC,						"Romanian, Macintosh")
/* Thai */
dcl(ENC_THAI_TIS,						"Thai, TIS-620")
dcl(ENC_THAI_WIN,						"Thai, Windows Code Page 874")
dcl(ENC_THAI_MAC,						"Thai, Macintosh")
/* Vietnamese */
dcl(ENC_VIET_VISCII,					"Vietnamese, VISCII")
dcl(ENC_VIET_TCVN,						"Vietnamese, TCVN")
dcl(ENC_VIET_WIN,						"Vietnamese, Windows Code Page 1258")
/* Hebrew */
dcl(ENC_HEBR_ISO,						"Hebrew, ISO-8859-8")
dcl(ENC_HEBR_WIN,						"Hebrew, Windows Code Page 1255")
dcl(ENC_HEBR_MAC,						"Hebrew, Macintosh")
/* Arabic */
dcl(ENC_ARAB_ISO,						"Arabic, ISO-8859-6")
dcl(ENC_ARAB_WIN,						"Arabic, Windows Code Page 1256")
dcl(ENC_ARAB_MAC,						"Arabic, Macintosh")
/* Armenian */
dcl(ENC_ARME_ARMSCII,					"Armenian, ARMSCII-8")
/* Georgian */
dcl(ENC_GEOR_ACADEMY,					"Georgian, Academy")
dcl(ENC_GEOR_PS,						"Georgian, PS")
/* Multibyte CJK */
/* Chinese Simplified */
dcl(ENC_CHSI_EUC,						"Chinese Simplified, EUC-CN (GB2312)")
dcl(ENC_CHSI_GB,						"Chinese Simplified, GB_2312-80")	// Cf. EUC
dcl(ENC_CHSI_HZ,						"Chinese Simplified, HZ")
dcl(ENC_CHSI_WIN,						"Chinese Simplified, Windows Code Page 936")
/* Chinese Traditional */
dcl(ENC_CHTR_BIG5,						"Chinese Traditional, BIG5")
dcl(ENC_CHTR_BIG5HKSCS,						"Chinese Traditional, BIG5-HKSCS")
dcl(ENC_CHTR_EUC,						"Chinese Traditional, EUC-TW")
dcl(ENC_CHTR_WIN,						"Chinese Traditional, Windows Code Page 950")
/* Japanese */
dcl(ENC_JAPN_ISO,						"Japanese, ISO-2022-JP")
dcl(ENC_JAPN_EUC,						"Japanese, EUC-JP")
dcl(ENC_JAPN_SJIS,						"Japanese, Shift-JIS")
dcl(ENC_JAPN_WIN,						"Japanese, Windows Code Page 932")
/* Korean */
dcl(ENC_KORE_KSC,						"Korean, KSC_5601") // ISO
dcl(ENC_KORE_EUC,						"Korean, EUC-KR")
dcl(ENC_KORE_JOHAB, 					"Korean, JOHAB")
dcl(ENC_KORE_WIN,						"Korean, Windows Code Page 949")
/* Unicode */
dcl(ENC_UNIC_UTF_7, 					"Unicode UTF-7")
dcl(ENC_UNIC_UTF_8, 					"Unicode UTF-8")
//dcl(ENC_UNIC_UTF_16,					"Unicode UTF-16")
dcl(ENC_UNIC_UTF_16BE,					"Unicode UTF-16 Big Endian")
dcl(ENC_UNIC_UTF_16LE,					"Unicode UTF-16 Little Endian")
//dcl(ENC_UNIC_UTF_32,					"Unicode UTF-32")
dcl(ENC_UNIC_UTF_32BE,					"Unicode UTF-32 Big Endian")
dcl(ENC_UNIC_UTF_32LE,					"Unicode UTF-32 Little Endian")
//dcl(ENC_UNIC_UCS2,						"Unicode UCS-2")
dcl(ENC_UNIC_UCS_2BE,					"Unicode UCS-2 Big Endian")
dcl(ENC_UNIC_UCS_2LE,					"Unicode UCS-2 Little Endian")
//dcl(ENC_UNIC_UCS4,						"Unicode UCS-4")
dcl(ENC_UNIC_UCS_4BE,					"Unicode UCS-4 Big Endian")
dcl(ENC_UNIC_UCS_4LE,					"Unicode UCS-4 Little Endian")

/* HTML Options Dialog */
dcl(DLG_HTMLOPT_ExpTitle,				"HTML Export Options")
dcl(DLG_HTMLOPT_ExpLabel,				"Select HTML export options:")
dcl(DLG_HTMLOPT_ExpSave,				"Save Settings")
dcl(DLG_HTMLOPT_ExpRestore,				"Restore Settings")
dcl(DLG_HTMLOPT_ExpIs4,					"Export as HTML 4.01")
dcl(DLG_HTMLOPT_ExpAbiWebDoc,			"Export with PHP instructions")
dcl(DLG_HTMLOPT_ExpDeclareXML,			"Declare as XML (version 1.0)")
dcl(DLG_HTMLOPT_ExpAllowAWML,			"Allow extra markup in AWML namespace")
dcl(DLG_HTMLOPT_ExpEmbedCSS,			"Embed (CSS) style sheet")
dcl(DLG_HTMLOPT_ExpEmbedImages,			"Embed images in URLs (Base64-encoded)")
dcl(DLG_HTMLOPT_ExpMathMLRenderPNG,		"Render MathML to PNG images (JavaScript will be used if option is not checked)")
dcl(DLG_HTMLOPT_ExpSplitDocument,		"Split document")
dcl(DLG_Exit_CloseWithoutSaving,				"Close &Without Saving")

	/* X11 input methods */
dcl(XIM_Methods, "Input Methods")

//since this string goes with XAP preference, I put it here, rather
//than ap_String_Id.h
dcl(DLG_Options_Label_LangWithKeyboard,         "Change Language when changing keyboard")
dcl(DLG_Options_Label_DirMarkerAfterClosingParenthesis, "Auto-insert direction markers")

dcl(DLG_History_WindowLabel, "Document History")
dcl(DLG_History_DocumentDetails, "Document Details")
dcl(DLG_History_Path, "Document name:")
dcl(DLG_History_Created, "Created:")
dcl(DLG_History_Version, "Version:")
dcl(DLG_History_LastSaved, "Last saved:")
dcl(DLG_History_EditTime, "Editing time:")
dcl(DLG_History_Id, "Identifier:")

dcl(DLG_History_List_Title, "Version history")
dcl(DLG_History_Version_Version, "Version")
dcl(DLG_History_Version_Started, "Created")
dcl(DLG_History_Version_AutoRevisioned, "Auto-revision")
dcl(DLG_History_Restore, "Restore")

dcl(DLG_DocComparison_WindowLabel, "Document Comparison")
dcl(DLG_DocComparison_DocsCompared, "Documents compared")
dcl(DLG_DocComparison_Results, "Results")
dcl(DLG_DocComparison_Relationship, "Relationship:")
dcl(DLG_DocComparison_Content, "Content:")
dcl(DLG_DocComparison_Fmt, "Format:")
dcl(DLG_DocComparison_Styles, "Styles:")

dcl(DLG_DocComparison_Identical, "identical")
dcl(DLG_DocComparison_Unrelated, "unrelated")
dcl(DLG_DocComparison_Siblings, "siblings")
dcl(DLG_DocComparison_Diverging, "diverging after version %d of %s")
dcl(DLG_DocComparison_DivergingPos, "diverging after document position %d")
dcl(DLG_DocComparison_TestSkipped, "(test skipped)")
dcl(DLG_DocComparison_Different, "different")
