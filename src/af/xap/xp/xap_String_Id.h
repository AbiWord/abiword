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

dcl(MSG_ShowUnixFontWarning, "AbiWord was not able to add its fonts to the X font path.\nThis does not mean that there is anything wrong with your\nsystem, but you will need to modify your font path manually.\nPlease see \"Unix Font Path Problem\" in the FAQ section of\nthe Abiword help file for more detailed information, including\ninstructions on how to turn this warning off.")

dcl(MSG_UnixFontSizeWarning, "The operating system was unable to allocate a font of the\nrequested size; a smaller font will be used in its place on\nthe screen, which will look strange, but the overall layout\nwill be correct, and you should be able to print the document\nproperly. Using a smaller zoom factor may resolve this problem.")
dcl(MSG_ImportingDoc, "Importing Document..")
dcl(MSG_BuildingDoc, "Building Document..")

/* Default name for new, untitled document */
dcl(UntitledDocument,			"Untitled%d")


/*  Styles */
dcl(STYLE_NUMBER_LIST,		"Numbered List")
dcl(STYLE_PLAIN_TEXT,		"Plain Text")
dcl(STYLE_NORMAL,			"Normal")
dcl(STYLE_HEADING1, 		"Heading 1")
dcl(STYLE_HEADING2, 		"Heading 2")
dcl(STYLE_HEADING3, 		"Heading 3")
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
dcl(STYLE_CHAPHEADING,		"Chapter Heading")     
dcl(STYLE_SECTHEADING,		"Section Heading")     
dcl(STYLE_ENDREFERENCE,		"Endnote Reference")   
dcl(STYLE_ENDTEXT,			"Endnote Text")        
dcl(STYLE_NUMHEAD1,			"Numbered Heading 1")  
dcl(STYLE_NUMHEAD2,			"Numbered Heading 2")  
dcl(STYLE_NUMHEAD3,			"Numbered Heading 3")  
dcl(STYLE_IMPLIES_LIST,		"Implies List")  


/* Common to many dialogs */
dcl(DLG_OK, 					"OK")
dcl(DLG_Cancel, 				"Cancel")
dcl(DLG_Close,					"Close")
dcl(DLG_Insert, 				"&Insert")
dcl(DLG_Update, 				"Update")
dcl(DLG_Apply,					"Apply")
dcl(DLG_Delete, 				"Delete")
/* Units */
dcl(DLG_Unit_inch,				"inch")
dcl(DLG_Unit_cm,				"cm")
dcl(DLG_Unit_mm,				"mm")
dcl(DLG_Unit_points,			"points")
dcl(DLG_Unit_pico,				"pica")

/* Message box */
/* These are tagged "UnixMB" because the underscores precede accelerator
   characters.	It should be an ampersand on Windows, but Windows doesn't
   need a hand-constructed message box (Win32 API provides one).
*/
dcl(DLG_UnixMB_Yes, 			"_Yes")
dcl(DLG_UnixMB_No,				"_No")

dcl(DLG_QNXMB_Yes,				"Yes")
dcl(DLG_QNXMB_No,				"No")

/* More Windows dialog */
dcl(DLG_MW_MoreWindows, 		"Activate Window")
dcl(DLG_MW_Activate,			"Activate:")

/* Remove Toolbar Icon */
dcl(DLG_Remove_Icon,			"Do you want to remove this icon from the toolbar?")

/* Font Selector dialog */
dcl(DLG_UFS_FontTitle,			"Font")
dcl(DLG_UFS_FontLabel,			"Font:")
dcl(DLG_UFS_StyleLabel, 		"Style:")
dcl(DLG_UFS_SizeLabel,			"Size:")
dcl(DLG_UFS_EncodingLabel,		"Encoding:")
dcl(DLG_UFS_EffectsFrameLabel,	"Effects")
dcl(DLG_UFS_StrikeoutCheck, 	"Strikeout")
dcl(DLG_UFS_UnderlineCheck, 	"Underline")
dcl(DLG_UFS_OverlineCheck,		"Overline")
dcl(DLG_UFS_HiddenCheck,        "Hidden")	
dcl(DLG_UFS_TransparencyCheck,	"Set no Highlight Color")
dcl(DLG_UFS_FontTab,			"	Font   ")
dcl(DLG_UFS_ColorTab,			"Text Color")
dcl(DLG_UFS_BGColorTab, 		"HighLight Color")
dcl(DLG_UFS_StyleRegular,		"Regular")
dcl(DLG_UFS_StyleItalic,		"Italic")
dcl(DLG_UFS_StyleBold,			"Bold")
dcl(DLG_UFS_StyleBoldItalic,	"Bold Italic")
dcl(DLG_UFS_ToplineCheck,		"Topline")
dcl(DLG_UFS_BottomlineCheck,	"Bottomline")
dcl(DLG_UFS_SmallCapsCheck, 	"Small Caps")
dcl(DLG_UFS_ColorLabel, 		"Color:")
dcl(DLG_UFS_ScriptLabel,		"Script:")
dcl(DLG_UFS_SampleFrameLabel,	"Sample")


/* Unix FileOpenSaveAs dialog */
dcl(DLG_FOSA_OpenTitle, 				"Open File")
dcl(DLG_FOSA_SaveAsTitle,				"Save File As")
dcl(DLG_FOSA_ExportTitle, "Export File")
dcl(DLG_FOSA_ImportTitle, "Import File")
dcl(DLG_FOSA_InsertTitle, "Insert File")
dcl(DLG_FOSA_PrintToFileTitle,			"Print To File")
dcl(DLG_FOSA_FileOpenTypeLabel, 		"Open file as type:")
dcl(DLG_FOSA_FileSaveTypeLabel, 		"Save file as type:")
dcl(DLG_FOSA_FilePrintTypeLabel,		"Print file as type:")
dcl(DLG_FOSA_FileTypeAutoDetect,		"Automatically Detected")
dcl(DLG_FOSA_ALLIMAGES, "All Image Files")
dcl(DLG_FOSA_ALLDOCS, "All Documents")
dcl(DLG_FOSA_ALL, "All (*.*)")
dcl(DLG_InvalidPathname,				"Invalid pathname.")
dcl(DLG_NoSaveFile_DirNotExist, 		"A directory in the given pathname does not exist.")
dcl(DLG_NoSaveFile_DirNotWriteable, 	"The directory '%s' is write-protected.")
dcl(DLG_OverwriteFile,					"File already exists.  Overwrite file '%s'?")

dcl(DLG_Password_Title, "Enter Password")
dcl(DLG_Password_Password, "Password:")

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
dcl(TB_Zoom_PageWidth,					"Fit to Width")
dcl(TB_Zoom_WholePage,					"Fit to Height")
dcl(TB_Zoom_Percent, "Other...")

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
dcl(DLG_PLUGIN_MANAGER_DEACTIVATE_ALL,	"Deactivate all plugins")
dcl(DLG_PLUGIN_MANAGER_INSTALL, 		"Install new plugin")
dcl(DLG_PLUGIN_MANAGER_LIST,			"Plugin List")
dcl(DLG_PLUGIN_MANAGER_NAME,			"Name")
dcl(DLG_PLUGIN_MANAGER_DESC,			"Description")
dcl(DLG_PLUGIN_MANAGER_AUTHOR,			"Author")
dcl(DLG_PLUGIN_MANAGER_VERSION, 		"Version")
dcl(DLG_PLUGIN_MANAGER_DETAILS, 		"Plugin Details")

dcl(DICTIONARY_CANTLOAD, "Could not load the dictionary for the %s language")

/* plugin error messages */
dcl(DLG_PLUGIN_MANAGER_COULDNT_LOAD,	"Could not activate/load plugin")
dcl(DLG_PLUGIN_MANAGER_COULDNT_UNLOAD,	"Could not deactivate plugin")
dcl(DLG_PLUGIN_MANAGER_NONE_SELECTED,	"No plugin selected")

/* Language Dialog */
dcl(DLG_ULANG_LangLabel,				"Select Language:")
dcl(DLG_ULANG_LangTitle,				"Language")

/* ClipArt Dialog */
dcl(DLG_CLIPART_Title, "Clip Art")

/* About Dialog */
dcl(DLG_ABOUT_Title,					"About %s")

	 /* image size dialog */
	 dcl(DLG_Image_Title, "Image Properties")
	 dcl(DLG_Image_Width, "Width:")
	 dcl(DLG_Image_Height, "Height:")
	 dcl(DLG_Image_Aspect, "Preserve Aspect Ratio:")

dcl(DLG_Options_Label_UnixFontWarning, "Show font warning at start up")
dcl(DLG_Options_Label_ModifyUnixFontPath, "Modify Unix Font Path")

/* HashDownloader dialogs */
dcl(DLG_HashDownloader_AskDownload, 		"I could not find a dictionary for %s.\nWould you like me to try downloading it from the internet?")
dcl(DLG_HashDownloader_AskFirstTryFailed, 	"Download failed!\nWould you like me to try again?\nIf yes, please make sure you are connected to the internet.")
dcl(DLG_HashDownloader_AskInstallGlobal, 	"You seem to have permission to install the dictionary system-wide.\nWould you like to do that?\nIf yes, I will install the dictionary in %s")
dcl(DLG_HashDownloader_DictlistDLFail, 		"Sorry! I seem unable to download the dictionary-list.")
dcl(DLG_HashDownloader_DictDLFail, 			"Sorry! I seem unable to download the dictionary.")
dcl(DLG_HashDownloader_DictInstallFail, 	"Sorry! I seem unable to install the dictionary.")
dcl(DLG_HashDownloader_DictNotAvailable, 	"I'm sorry. The wanted dictionary is not available for download.")
dcl(DLG_HashDownloader_DictNotForThis, 		"I'm sorry. The wanted dictionary is not available for this release.\nPlease consider upgrading AbiWord.")
dcl(DLG_HashDownloader_FeatureDisabled, 	"I'm sorry. This feature is not available anymore.")
dcl(DLG_HashDownloader_FeatureDisabledForThis, 	"I'm sorry. This feature is disabled for this release.\nPlease consider upgrading AbiWord.")
dcl(DLG_HashDownloader_DictList,			"the dictionary-list")
dcl(DLG_HashDownloader_Dict, 				"the dictionary")
dcl(DLG_HashDownloader_Dlg_Title, 			"New dictionary installation")

/*
For insert Table widget
*/
dcl(TB_InsertNewTable,                   "Insert New Table")
dcl(TB_Table,                            "Table")

/*
	  Language property in different languages; alphabetical except English first.
	  Please when translating the first entry, put it also into parenthesis
	  or, surround it by some other characters, so that it will appear on the top
	  of the list when sorted alphabetically.
*/
dcl(LANG_0, 							"(no proofing)")
dcl(LANG_EN_AU, 							"English (Australia)")
dcl(LANG_EN_CA, 							"English (Canada)")
dcl(LANG_EN_GB, 							"English (UK)")
dcl(LANG_EN_IE, 							"English (Ireland)")
dcl(LANG_EN_NZ, 							"English (New Zealand)")
dcl(LANG_EN_ZA, 							"English (South Africa)")
dcl(LANG_EN_US, 							"English (US)")
dcl(LANG_AF_ZA, 							"Afrikaans")
dcl(LANG_SQ_AL, 							"Albanian")	// Jordi 19/10/2002
dcl(LANG_AR_EG, 							"Arabic (Egypt)")
dcl(LANG_AR_SA, 							"Arabic (Saudi Arabia)")
dcl(LANG_HY_AM, 							"Armenian")
dcl(LANG_AS_IN, 							"Assamese")
dcl(LANG_EU_ES, 							"Basque")
dcl(LANG_BE_BY, 							"Belarusian")
dcl(LANG_BR_FR,								"Breton")		// Jordi 19/10/2002
dcl(LANG_BG_BG, 							"Bulgarian")
dcl(LANG_CA_ES, 							"Catalan")
dcl(LANG_KW_GB,								"Cornish")		// Jordi 19/10/2002
dcl(LANG_CO_FR,								"Corsican")		// Jordi 19/10/2002
dcl(LANG_HR,								"Croatian")		// Jordi
dcl(LANG_ZH_HK, 							"Chinese (Hong Kong)")
dcl(LANG_ZH_CN, 							"Chinese (PRC)")
dcl(LANG_ZH_SG, 							"Chinese (Singapore)")
dcl(LANG_ZH_TW, 							"Chinese (Taiwan)")
dcl(LANG_CS_CZ, 							"Czech")
dcl(LANG_DA_DK, 							"Danish")
dcl(LANG_NL_NL, 							"Dutch (Netherlands)")
dcl(LANG_EO,								"Esperanto")
dcl(LANG_ET,								"Estonian")
dcl(LANG_FA_IR, 							"Farsi")
dcl(LANG_FI_FI, 							"Finnish")
dcl(LANG_NL_BE, 							"Flemish (Belgium)")
dcl(LANG_FR_BE, 							"French (Belgium)")
dcl(LANG_FR_CA, 							"French (Canada)")
dcl(LANG_FR_FR, 							"French (France)")
dcl(LANG_FR_CH, 							"French (Switzerland)")
dcl(LANG_FY_NL,								"Frisian")	// Jordi 19/10/2002
dcl(LANG_GL_ES, 							"Galician")
dcl(LANG_KA_GE, 							"Georgian")
dcl(LANG_DE_AT, 							"German (Austria)")
dcl(LANG_DE_DE, 							"German (Germany)")
dcl(LANG_DE_CH, 							"German (Switzerland)")
dcl(LANG_EL_GR, 							"Greek")
dcl(LANG_HAU_NE,							"Hausa (Niger)")
dcl(LANG_HAU_NG,							"Hausa (Nigeria)")
dcl(LANG_HE_IL, 							"Hebrew")
dcl(LANG_HI_IN, 							"Hindi")
dcl(LANG_HU_HU, 							"Hungarian")
dcl(LANG_IS_IS, 							"Icelandic")
dcl(LANG_ID_ID,	 							"Indonesian")
dcl(LANG_IA,	 							"Interlingua")
dcl(LANG_GA_IE, 							"Irish")
dcl(LANG_IT_IT, 							"Italian (Italy)")
dcl(LANG_JA_JP, 							"Japanese")
dcl(LANG_KO_KR, 							"Korean")
dcl(LANG_LA_IT, 							"Latin (Renaissance)")	// Is _IT the right thing here?
dcl(LANG_LV_LV, 							"Latvian")
dcl(LANG_LT_LT, 							"Lithuanian")
dcl(LANG_MK,								"Macedonian")	// Jordi 19/10/2002
dcl(LANG_MH_MH, 							"Marshallese (Marshall Islands)")
dcl(LANG_MH_NR, 							"Marshallese (Nauru)")
dcl(LANG_NB_NO, 							"Norwegian Bokmal")
dcl(LANG_NN_NO, 							"Norwegian Nynorsk")
dcl(LANG_OC_FR,								"Occitan")		// Jordi 19/10/2002
dcl(LANG_PL_PL, 							"Polish")
dcl(LANG_PT_BR, 							"Portuguese (Brazil)")
dcl(LANG_PT_PT, 							"Portuguese (Portugal)")
dcl(LANG_RO_RO, 							"Romanian")
dcl(LANG_RU_RU, 							"Russian (Russia)")
dcl(LANG_SC_IT,								"Sardinian")	// Jordi 19/10/2002
dcl(LANG_SR,								"Serbian")		// Jordi 19/10/2002
dcl(LANG_SK_SK, 							"Slovak")				// or Slovakian?
dcl(LANG_SL_SI, 							"Slovenian")
dcl(LANG_ES_MX, 							"Spanish (Mexico)")
dcl(LANG_ES_ES, 							"Spanish (Spain)")
dcl(LANG_SV_SE, 							"Swedish")
dcl(LANG_TH_TH, 							"Thai")
dcl(LANG_TR_TR, 							"Turkish")
dcl(LANG_UK_UA, 							"Ukrainian")
dcl(LANG_VI_VN, 							"Vietnamese")
dcl(LANG_CY_GB, 							"Welsh")
dcl(LANG_YI,								"Yiddish")


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
dcl(ENC_KORE_JOHAB, 					"Korean, Johab")
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
