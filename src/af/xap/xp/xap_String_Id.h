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

/* Default name for new, untitled document */
dcl(UntitledDocument,			"Untitled%d")

/* Common to many dialogs */
dcl(DLG_OK,						"OK")
dcl(DLG_Cancel,					"Cancel")
dcl(DLG_Close,					"Close")
dcl(DLG_Insert,					"&Insert")
dcl(DLG_Update,					"Update")
dcl(DLG_Apply,					"Apply")
/* Units */
dcl(DLG_Unit_inch,				"inch")
dcl(DLG_Unit_cm,				"cm")
dcl(DLG_Unit_mm,                                "mm")
dcl(DLG_Unit_points,			"points")
dcl(DLG_Unit_pico,				"pica")

/* Message box */
/* These are tagged "UnixMB" because the underscores precede accelerator
   characters.  It should be an ampersand on Windows, but Windows doesn't
   need a hand-constructed message box (Win32 API provides one).
*/
dcl(DLG_UnixMB_Yes,				"_Yes")
dcl(DLG_UnixMB_No,				"_No")

dcl(DLG_QNXMB_Yes,				"Yes")
dcl(DLG_QNXMB_No,				"No")
	
/* More Windows dialog */
dcl(DLG_MW_MoreWindows,			"Activate Window")
dcl(DLG_MW_Activate,			"Activate:")

/* Unix Font Selector dialog */
dcl(DLG_UFS_FontTitle,			"Font")
dcl(DLG_UFS_FontLabel,			"Font:")
dcl(DLG_UFS_StyleLabel,			"Style:")
dcl(DLG_UFS_SizeLabel,			"Size:")
dcl(DLG_UFS_EncodingLabel,		"Encoding:")
dcl(DLG_UFS_EffectsFrameLabel,	"Effects")
dcl(DLG_UFS_StrikeoutCheck,		"Strikeout")
dcl(DLG_UFS_UnderlineCheck,		"Underline")
dcl(DLG_UFS_OverlineCheck,		"Overline")
dcl(DLG_UFS_TransparencyCheck,	"Set no Hightlight Color")
dcl(DLG_UFS_FontTab,			"   Font   ")
dcl(DLG_UFS_ColorTab,			"Text Color")
dcl(DLG_UFS_BGColorTab,			"HighLight Color")
dcl(DLG_UFS_StyleRegular,		"Regular")
dcl(DLG_UFS_StyleItalic,		"Italic")
dcl(DLG_UFS_StyleBold,			"Bold")
dcl(DLG_UFS_StyleBoldItalic,	"Bold Italic")
#ifdef BIDI_ENABLED
dcl(DLG_UFS_Direction,     "Right-to-left")
#endif

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

/* Zoom tool bar */
dcl(TB_Zoom_PageWidth,					"Width")
dcl(TB_Zoom_WholePage,					"Page")

/* Unix Print dialog */
dcl(DLG_UP_PrintTitle,					"Print")
dcl(DLG_UP_PrintPreviewTitle,                      "AbiWord: Print Preview")
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
dcl(DLG_UP_EmbedFonts,					"Embed Fonts")
dcl(DLG_UP_Copies,						"Copies: ")
dcl(DLG_UP_PrintButton,					"Print")
dcl(DLG_UP_InvalidPrintString,			"The print command string is not valid.")
dcl(DLG_UP_PrintIn,						"Print in: ")
dcl(DLG_UP_BlackWhite,					"Black & White")
dcl(DLG_UP_Grayscale,					"Grayscale")
dcl(DLG_UP_Color,						"Color")

/* Insert Symbol dialog */
dcl(DLG_Insert_SymbolTitle,				"Insert Symbol")

/* Insert Picture Preview Dialog (Win32) */
dcl(DLG_IP_Title,						"Insert Picture")
dcl(DLG_IP_Activate_Label,				"Preview Picture")
dcl(DLG_IP_No_Picture_Label,			"No Picture")
dcl(DLG_IP_Height_Label,				"Height: ")
dcl(DLG_IP_Width_Label,					"Width:  ")
dcl(DLG_IP_Button_Label,				"Insert")

/* Plugin dialog */
dcl(DLG_PLUGIN_MANAGER_TITLE,           "AbiWord Plugin Manager")
dcl(DLG_PLUGIN_MANAGER_ACTIVE,          "Active Plugins")
dcl(DLG_PLUGIN_MANAGER_DEACTIVATE,      "Deactivate plugin")
dcl(DLG_PLUGIN_MANAGER_DEACTIVATE_ALL,  "Deactivate all plugins")
dcl(DLG_PLUGIN_MANAGER_INSTALL,         "Install new plugin")
dcl(DLG_PLUGIN_MANAGER_LIST,            "Plugin List")
dcl(DLG_PLUGIN_MANAGER_NAME,            "Name")
dcl(DLG_PLUGIN_MANAGER_DESC,            "Description")
dcl(DLG_PLUGIN_MANAGER_AUTHOR,          "Author")
dcl(DLG_PLUGIN_MANAGER_VERSION,         "Version")
dcl(DLG_PLUGIN_MANAGER_DETAILS,         "Plugin Details")

/* plugin error messages */
dcl(DLG_PLUGIN_MANAGER_COULDNT_LOAD,    "Could not activate/load plugin")
dcl(DLG_PLUGIN_MANAGER_COULDNT_UNLOAD,  "Could not deactivate plugin") 
dcl(DLG_PLUGIN_MANAGER_NONE_SELECTED,   "No plugin selected")

/* Language Dialog */
dcl(DLG_ULANG_LangLabel,				"Select Language:")
dcl(DLG_ULANG_LangTitle,				"Language")

/* ClipArt Dialog */
dcl(DLG_CLIPART_Title, "Clip Art")

/* Language property in different languages; keep sorted by the numerical id's */
dcl(LANG_0,								"no proofing")
dcl(LANG_1,								"English (Australia)")
//dcl(LANG_1,								"English (Canada)")
dcl(LANG_2,								"English (UK)")
//dcl(LANG_2,								"English (Ireland)")
dcl(LANG_3,								"English (New Zealand)")
//dcl(LANG_4,								"English (South Africa)")
dcl(LANG_4,								"English (US)")
//dcl(LANG_4,								"Afrikaans")
dcl(LANG_5,								"Arabic (Egypt)")
dcl(LANG_6,								"Arabic (Saudi Arabia)")
dcl(LANG_7,								"Armenian")
dcl(LANG_8,								"Chinese (Hong Kong)")
dcl(LANG_9,								"Chinese (PRC)")
dcl(LANG_10,								"Chinese (Singapore)")
dcl(LANG_11,								"Chinese (Taiwan)")
dcl(LANG_12,								"Czech")
dcl(LANG_13,								"Danish")
dcl(LANG_14,								"Dutch (Netherlands)")
dcl(LANG_15,								"Farsi")
dcl(LANG_16,								"Finnish")
dcl(LANG_17,								"French (France)")
dcl(LANG_18,								"Georgian")
dcl(LANG_19,								"German (Austria)")
dcl(LANG_20,								"German (Germany)")
dcl(LANG_21,								"Greek")
dcl(LANG_22,								"Hebrew")
dcl(LANG_23,								"Hindi")
dcl(LANG_24,								"Italian (Italy)")
dcl(LANG_25,								"Japanese")
dcl(LANG_26,								"Korean")
dcl(LANG_27,								"Lithuanian")
dcl(LANG_28,								"Norwegian Bokmal")
dcl(LANG_29,								"Norwegian Nynorsk")
dcl(LANG_30,								"Portuguese (Brazil)")
dcl(LANG_31,								"Portuguese (Portugal)")
dcl(LANG_32,								"Russian (Russia)")
dcl(LANG_33,								"Spanish (Mexico)")
dcl(LANG_34,								"Spanish (Spain)")
dcl(LANG_35,								"Swedish")
dcl(LANG_36,								"Thai")
dcl(LANG_37,								"Vietnamese")

/* Encoding Dialog */
dcl(DLG_UENC_EncLabel,				"Select Encoding:")
dcl(DLG_UENC_EncTitle,				"Encoding")

/* Encoding description in different encodings; keep sorted by the numerical id's */
/* Ordered by 1) number of bits, 2) region & language, 3) platform / standard.    */
/* 1) 7 bit, 8 bit, multibyte, Unicode.                                           */
/* 2) Western, Eastern, Asian, multilingual; common before rare.                  */
/* 3) ISO, de facto standards, MS Windows, Macintosh                              */
/* It may be desirable to change this order for each platform.                    */

/* 7 bit */
dcl(ENC_0,								"ASCII")
/* 8 bit */
/* Western Europe */
dcl(ENC_1,								"Western European, ISO-8859-1")
dcl(ENC_2,								"Western European, Windows Code Page 1252")
dcl(ENC_3,								"Western European, Macintosh")
/* Central & Eastern Europe */
dcl(ENC_4,								"Central European, ISO-8859-2")
dcl(ENC_5,								"Central European, Windows Code Page 1250")
dcl(ENC_6,								"Central European, Macintosh")
/* Baltic */
dcl(ENC_7,								"Baltic, ISO-8859-4")
dcl(ENC_8,								"Baltic, Windows Code Page 1257")
/* Greek */
dcl(ENC_9,								"Greek, ISO-8859-7")
dcl(ENC_10,								"Greek, Windows Code Page 1253")
dcl(ENC_11,								"Greek, Macintosh")
/* Cyrillic */
dcl(ENC_12,								"Cyrillic, ISO-8859-5")
dcl(ENC_13,								"Cyrillic, KOI8-R")
dcl(ENC_14,								"Cyrillic, Windows Code Page 1251")
dcl(ENC_15,								"Cyrillic, Macintosh")
dcl(ENC_16,								"Ukrainian, KOI8-U")
dcl(ENC_17,								"Ukrainian, Macintosh")
/* Turkish */
dcl(ENC_18,								"Turkish, ISO-8859-9")
dcl(ENC_19,								"Turkish, Windows Code Page 1254")
dcl(ENC_20,								"Turkish, Macintosh")
/* Thai */
dcl(ENC_21,								"Thai, TIS-620")
dcl(ENC_22,								"Thai, Windows Code Page 874")
dcl(ENC_23,								"Thai, Macintosh")
/* Vietnamese */
dcl(ENC_24,								"Vietnamese, VISCII")
dcl(ENC_25,								"Vietnamese, TCVN")
dcl(ENC_26,								"Vietnamese, Windows Code Page 1258")
/* Hebrew */
dcl(ENC_27,								"Hebrew, ISO-8859-8")
dcl(ENC_28,								"Hebrew, Windows Code Page 1255")
dcl(ENC_29,								"Hebrew, Macintosh")
/* Arabic */
dcl(ENC_30,								"Arabic, ISO-8859-6")
dcl(ENC_31,								"Arabic, Windows Code Page 1256")
dcl(ENC_32,								"Arabic, Macintosh")
/* Armenian */
dcl(ENC_33,								"Armenian, ARMSCII-8")
/* Georgian */
dcl(ENC_34,								"Georgian, Academy")
dcl(ENC_35,								"Georgian, PS")
/* Multibyte CJK */
/* Chinese Simplified */
dcl(ENC_36,								"Chinese Simplified, EUC-CN")
dcl(ENC_37,								"Chinese Simplified, GB_2312-80")	// Cf. EUC
dcl(ENC_38,								"Chinese Simplified, HZ")
dcl(ENC_39,								"Chinese Simplified, Windows Code Page 936")
/* Chinese Traditional */
dcl(ENC_40,								"Chinese Traditional, BIG5")
dcl(ENC_41,								"Chinese Traditional, EUC-TW")
dcl(ENC_42,								"Chinese Traditional, Windows Code Page 950")
/* Japanese */
dcl(ENC_43,								"Japanese, ISO-2022-JP")
dcl(ENC_44,								"Japanese, EUC-JP")
dcl(ENC_45,								"Japanese, Shift-JIS")
dcl(ENC_46,								"Japanese, Windows Code Page 932")
/* Korean */
dcl(ENC_47,								"Korean, KSC_5601")	// ISO
dcl(ENC_48,								"Korean, EUC-KR")
dcl(ENC_49,								"Korean, Johab")
dcl(ENC_50,								"Korean, Windows Code Page 949")
/* Unicode */
dcl(ENC_51,								"Unicode UTF-7")
dcl(ENC_52,								"Unicode UTF-8")
dcl(ENC_53,								"Unicode UCS-2 Big Endian")
dcl(ENC_54,								"Unicode UCS-2 Little Endian")












