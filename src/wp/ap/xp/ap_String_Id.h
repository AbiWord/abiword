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
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

// Message Boxes used in AP_EditMethods
dcl(MSG_SaveFailed,		"Could not write to the file %s.")
dcl(MSG_SaveFailedWrite,        "Writing error when attempting to save %s")
dcl(MSG_SaveFailedExport,       "Error while attempting to save %s: could not construct exporter")
dcl(MSG_SaveFailedName,         "Error while attempting to save %s: invalid name")
dcl(MSG_DlgNotImp,              "%s not implemented yet.\n\nIf you are a programmer, feel free to add code in %s, line %d\nand mail patches to:\n\n\tabiword-dev@abisource.com\n\nOtherwise, please be patient.")
dcl(MSG_RevertBuffer,	        "Revert to saved copy of %s?")
dcl(MSG_QueryExit,				"Close all windows and exit?")
dcl(MSG_ConfirmSave,	        "Save changes to %s?")
dcl(MSG_ImportError,	        "Error importing file %s.")
dcl(MSG_IE_FileNotFound,        "File %s not found")
dcl(MSG_IE_NoMemory,            "Out of memory attempting to open %s")
dcl(MSG_IE_UnknownType,         "File %s is of unknown type")
dcl(MSG_IE_FakeType,            "File %s is not of the type it claims to be")
dcl(MSG_IE_UnsupportedType,     "File %s is not of a currently supported file type")
dcl(MSG_IE_BogusDocument,       "File %s is a bogus document")
dcl(MSG_IE_CouldNotOpen,        "Could not open file %s for writing")
dcl(MSG_IE_CouldNotWrite,       "Could not write to file %s")
dcl(MSG_SpellDone,				"The spelling check is complete.")

// Status Bar Messages
dcl(PageInfoField,				"Page: %d/%d")
dcl(LeftMarginStatus,			"Left Margin [%s]")
dcl(RightMarginStatus,			"Right Margin [%s]")
dcl(FirstLineIndentStatus,		"First Line Indent [%s]")
dcl(LeftIndentTextIndentStatus,	"Left Indent [%s] First Line Indent [%s]")
dcl(ColumnGapStatus,			"Column Gap [%s]")
dcl(LeftIndentStatus,			"Left Indent [%s]")
dcl(RightIndentStatus,			"Right Indent [%s]")
dcl(TabStopStatus,				"Tab Stop [%s]")
dcl(TopMarginStatus,			"Top Margin [%s]")
dcl(BottomMarginStatus,			"Bottom Margin [%s]")
dcl(HeaderStatus,				"Header [%s]")
dcl(FooterStatus,				"Footer [%s]")

dcl(InsertModeFieldINS,			"INS")
dcl(InsertModeFieldOVR,			"OVR")

/* */
dcl(DLG_NEW_Title,              "Choose a Template")
dcl(DLG_NEW_Create,             "Create new document from a Template")
dcl(DLG_NEW_Open,               "Open an existing document")
dcl(DLG_NEW_NoFile,             "No File")
dcl(DLG_NEW_Choose,             "Choose")
dcl(DLG_NEW_StartEmpty,         "Start with an empty document")

dcl(DLG_NEW_Tab1,               "Wordprocessing")
dcl(DLG_NEW_Tab1_WP1,           "Create a new blank document")
dcl(DLG_NEW_Tab1_FAX1,          "Create a fax")
	
/* Find and Replace strings */
dcl(DLG_FR_FindTitle, 			"Find")
dcl(DLG_FR_ReplaceTitle,		"Replace")
dcl(DLG_FR_FindLabel, 			"Fi&nd what:")
dcl(DLG_FR_ReplaceWithLabel,	"Re&place with:")
dcl(DLG_FR_MatchCase,			"&Match case")
dcl(DLG_FR_FindNextButton,		"&Find Next")
dcl(DLG_FR_ReplaceButton,		"&Replace")
dcl(DLG_FR_ReplaceAllButton,	"Replace &All")

/* Break dialog */
dcl(DLG_Break_BreakTitle,		"Break")
dcl(DLG_Break_Insert,			"Insert")
dcl(DLG_Break_SectionBreaks,	"Section breaks")
dcl(DLG_Break_PageBreak,		"&Page break")
dcl(DLG_Break_ColumnBreak,		"&Column break")
dcl(DLG_Break_NextPage,			"&Next page")
dcl(DLG_Break_Continuous,		"Con&tinuous")
dcl(DLG_Break_EvenPage,			"&Even page")
dcl(DLG_Break_OddPage,			"&Odd page")

/* Word Count dialog */
dcl(DLG_WordCount_WordCountTitle,	"Word Count")
dcl(DLG_WordCount_Statistics,		"Statistics:")
dcl(DLG_WordCount_Pages,			"Pages")
dcl(DLG_WordCount_Words,			"Words")
dcl(DLG_WordCount_Characters_No,	"Characters (no spaces)")
dcl(DLG_WordCount_Characters_Sp,	"Characters (with spaces)")
dcl(DLG_WordCount_Paragraphs,		"Paragraphs")
dcl(DLG_WordCount_Lines,			"Lines")
dcl(DLG_WordCount_Update_Rate,		"Seconds between updates")
dcl(DLG_WordCount_Auto_Update,		" Auto Update")
	
/* Spell dialog */
dcl(DLG_Spell_SpellTitle,		"Spelling")
dcl(DLG_Spell_UnknownWord,		"Not in dictionary&:")
dcl(DLG_Spell_ChangeTo,			"Change &to:")
dcl(DLG_Spell_Change,			"&Change")
dcl(DLG_Spell_ChangeAll,		"Change A&ll")
dcl(DLG_Spell_Ignore,			"&Ignore")
dcl(DLG_Spell_IgnoreAll,		"I&gnore All")
dcl(DLG_Spell_AddToDict,		"&Add")
dcl(DLG_Spell_Suggestions,		"Sugg&estions:")
dcl(DLG_Spell_NoSuggestions,	"(no spelling suggestions)")

/* Style Dialog */
dcl(DLG_Styles_StylesTitle,		"Styles")
dcl(DLG_Styles_Available,		"Available Styles")
dcl(DLG_Styles_List,			"List")
dcl(DLG_Styles_ParaPrev,		"Paragraph Preview")
dcl(DLG_Styles_CharPrev,		"Character Preview")
dcl(DLG_Styles_Description,		"Description")
dcl(DLG_Styles_New,				"New...")
dcl(DLG_Styles_Modify,			"Modify...")
dcl(DLG_Styles_Delete,			"Delete")
dcl(DLG_Styles_LBL_InUse,       "In Use")
dcl(DLG_Styles_LBL_All,         "All")
dcl(DLG_Styles_LBL_UserDefined, "User-defined styles")
dcl(DLG_Styles_LBL_TxtMsg,      "What Hath God Wrought")
dcl(DLG_Styles_ModifyTitle,		"Modify Styles")
dcl(DLG_Styles_ModifyName,		"Style Name:")
dcl(DLG_Styles_ModifyBasedOn,	"Based On:")
dcl(DLG_Styles_ModifyFollowing,	"Style for following paragraph")
dcl(DLG_Styles_ModifyPreview,	"Preview")
dcl(DLG_Styles_ModifyDescription,	"Description")
dcl(DLG_Styles_ModifyTemplate,	"Add to template")
dcl(DLG_Styles_ModifyAutomatic,	"Automatically update")
dcl(DLG_Styles_ModifyShortCut,	"Shortcut Key")
dcl(DLG_Styles_ModifyFormat,	"Format")
dcl(DLG_Styles_ModifyParagraph,	"Paragraph")
dcl(DLG_Styles_ModifyFont,	    "Font")
dcl(DLG_Styles_ModifyTabs,	    "Tabs")
dcl(DLG_Styles_ModifyNumbering,	"Numbering")
dcl(DLG_Styles_ModifyLanguage,  "Language")
dcl(DLG_Styles_ModifyCharacter,  "Character")
dcl(DLG_Styles_ModifyType,       "Style Type")
dcl(DLG_Styles_DefNone,          "None")
dcl(DLG_Styles_DefCurrent,       "Current Settings")
dcl(DLG_Styles_NewTitle,         "New Style")
dcl(DLG_Styles_RemoveLab,        "Remove Property from Style")
dcl(DLG_Styles_RemoveButton,     "Remove")
dcl(DLG_Styles_ErrNotTitle1,     "Style Name - ")
dcl(DLG_Styles_ErrNotTitle2,     " - Reserved. \n You cannot use this name. Choose Another \n")
dcl(DLG_Styles_ErrNoStyle,       "No Style selected \n so it cannot be modified")
dcl(DLG_Styles_ErrStyleNot,      "This style does not exist \n so it cannot be modified")
dcl(DLG_Styles_ErrStyleBuiltin, "Cannot modify a builtin style")
dcl(DLG_Styles_ErrStyleCantDelete, "Cannot delete this style")


/* Paragraph dialog */
dcl(DLG_Para_ParaTitle,			"Paragraph")

dcl(DLG_Para_AlignLeft,			"Left")
dcl(DLG_Para_AlignCentered,		"Centered")
dcl(DLG_Para_AlignRight,		"Right")
dcl(DLG_Para_AlignJustified,	"Justified")

dcl(DLG_Para_SpecialNone,		"(none)")
dcl(DLG_Para_SpecialFirstLine,	"First line")
dcl(DLG_Para_SpecialHanging,	"Hanging")

dcl(DLG_Para_SpacingSingle,		"Single")
dcl(DLG_Para_SpacingHalf,		"1.5 lines")
dcl(DLG_Para_SpacingDouble,		"Double")
dcl(DLG_Para_SpacingAtLeast,	"At least")
dcl(DLG_Para_SpacingExactly,	"Exactly")
dcl(DLG_Para_SpacingMultiple,	"Multiple")

dcl(DLG_Para_TabLabelIndentsAndSpacing,	"&Indents and Spacing")
dcl(DLG_Para_TabLabelLineAndPageBreaks,	"Line and &Page Breaks")

dcl(DLG_Para_LabelAlignment,	"Ali&gnment:")
dcl(DLG_Para_LabelBy,			"B&y:")
dcl(DLG_Para_LabelLeft,			"&Left:")
dcl(DLG_Para_LabelRight,		"&Right:")
dcl(DLG_Para_LabelSpecial,		"&Special:")
dcl(DLG_Para_LabelBefore,		"&Before:")
dcl(DLG_Para_LabelAfter,		"Aft&er:")
dcl(DLG_Para_LabelLineSpacing,	"Li&ne spacing:")
dcl(DLG_Para_LabelAt,			"&At:")

dcl(DLG_Para_LabelIndentation,	"Indentation")
dcl(DLG_Para_LabelSpacing,		"Spacing")
dcl(DLG_Para_LabelPreview,		"Preview")
dcl(DLG_Para_LabelPagination,	"Pagination")

dcl(DLG_Para_PushWidowOrphanControl,	"&Widow/Orphan control")
dcl(DLG_Para_PushKeepLinesTogether,		"&Keep lines together")
dcl(DLG_Para_PushPageBreakBefore,		"Page &break before")
dcl(DLG_Para_PushSuppressLineNumbers,	"&Suppress line numbers")
dcl(DLG_Para_PushNoHyphenate,			"&Don't hyphenate")
dcl(DLG_Para_PushKeepWithNext,			"Keep with ne&xt")

dcl(DLG_Para_ButtonTabs,		"&Tabs...")

#ifdef BIDI_ENABLED
dcl(DLG_Para_DomDirection,              "Right-to-left &dominant")
#endif

/* Columns dialog */
dcl(DLG_Column_ColumnTitle,		"Columns")
dcl(DLG_Column_Number,			"Number of columns")
dcl(DLG_Column_Preview,			"Preview")
dcl(DLG_Column_One,				"One")
dcl(DLG_Column_Two,				"Two")
dcl(DLG_Column_Three,			"Three")
dcl(DLG_Column_Line_Between,	"Line between")


/* when translating these, the important thing to remember is to get a
   similar amount of text in the translation.  Microsoft Word uses
   strings just like this to simulate text, and if your translation
   ends up very, very wordy, it will take up too much room in the preview.
*/
dcl(DLG_Para_PreviewSampleFallback, "This paragraph represents words as they might appear in your document.  To see text from your document used in this preview, position your cursor in a document paragraph with some text in it and open this dialog.")
dcl(DLG_Para_PreviewPrevParagraph,  "Previous Paragraph Previous Paragraph Previous Paragraph Previous Paragraph Previous Paragraph Previous Paragraph Previous Paragraph")
dcl(DLG_Para_PreviewFollowParagraph,"Following Paragraph Following Paragraph Following Paragraph Following Paragraph Following Paragraph Following Paragraph Following Paragraph")

/* Options dialog */
dcl(DLG_Options_OptionsTitle,			"Preferences")

dcl(DLG_Options_TabLabel_Spelling,		"Spelling")
dcl(DLG_Options_TabLabel_Other,			"Other")
dcl(DLG_Options_TabLabel_Preferences,	"Preference Schemes")
dcl(DLG_Options_TabLabel_View,			"View")

dcl(DLG_Options_Btn_Save,				"Sa&ve")
dcl(DLG_Options_Btn_Apply,				"Apply")
dcl(DLG_Options_Btn_Default,			"De&faults")

dcl(DLG_Options_Label_SpellCheckAsType,	"Check s&pelling as you type")
dcl(DLG_Options_Label_SpellHideErrors,	"Hide &spelling errors in the document")
dcl(DLG_Options_Label_SpellSuggest,		"A&lways suggest corrections")
dcl(DLG_Options_Label_SpellMainOnly,	"Suggest from &main dictionary only")
dcl(DLG_Options_Label_SpellUppercase,	"Ignore words in &UPPERCASE")
dcl(DLG_Options_Label_SpellNumbers,		"Ignore words with num&bers")
dcl(DLG_Options_Label_SpellInternet,	"Ignore Internet and &file addresses")
dcl(DLG_Options_Label_SpellCustomDict,	"Custom Dictionary:")
dcl(DLG_Options_Label_SpellIgnoredWord,	"Ignored words:")
dcl(DLG_Options_Btn_CustomDict,			"&Dictionary...")
dcl(DLG_Options_Btn_IgnoreReset,		"&Reset")
dcl(DLG_Options_Btn_IgnoreEdit,			"&Edit")
dcl(DLG_Options_Label_SpellAutoReplace, "Auto replace misspelled words")

dcl(DLG_Options_Label_SmartQuotesEnable,"&Enable smart quotes")
dcl(DLG_Options_Label_DefaultPageSize, "Default page size")

#ifdef BIDI_ENABLED
dcl(DLG_Options_Label_DirectionRtl, "Default to right-to-left direction of text")
dcl(DLG_Options_Label_BiDiOptions, "Bi-Directional Options")
#endif

dcl(DLG_Options_Label_AutoSave,			"Auto Save")
dcl(DLG_Options_Label_AutoSaveCurrent,	"Auto &save current file each")
dcl(DLG_Options_Label_Minutes,			"minutes")
dcl(DLG_Options_Label_WithExtension,	"With extension:")

dcl(DLG_Options_Label_PrefsAutoSave,	"&Automatically save this Scheme")
dcl(DLG_Options_Label_PrefsCurrentScheme,	"&Current Preferences Scheme")

dcl(DLG_Options_Label_ViewShowHide,		"Show/Hide...")
dcl(DLG_Options_Label_ViewRuler,		"&Ruler")
dcl(DLG_Options_Label_ViewUnits,		"&Units:")
dcl(DLG_Options_Label_ViewCursorBlink,	"Cursor &blink")
dcl(DLG_Options_Label_ViewStandardTB,		"Standard Toolbar")
dcl(DLG_Options_Label_ViewFormatTB,		"Format Toolbar")
dcl(DLG_Options_Label_ViewExtraTB,		"Extra Toolbar")
dcl(DLG_Options_Label_ViewStatusBar,		"&Status bar")
dcl(DLG_Options_Label_ViewViewFrame,	"View...")
dcl(DLG_Options_Label_ViewAll,			"&All")
dcl(DLG_Options_Label_ViewHiddenText,	"&Hidden Text")
dcl(DLG_Options_Label_ViewUnprintable,	"Invisible &Layout Marks")

dcl(DLG_Options_Label_CheckWhiteForTransparent,	"Allow screen colors other than white")
dcl(DLG_Options_Label_ChooseForTransparent,	"Choose Screen Color")
dcl(DLG_Options_Label_ColorChooserLabel,	"Choose screen color for AbiWord")


dcl(DLG_Options_Prompt_IgnoreResetCurrent,	"Do you want to reset ignored words in the current document?")
dcl(DLG_Options_Prompt_IgnoreResetAll,		"Do you want to reset ignored words in all the documents?")

dcl(DLG_Options_Label_Icons, "Icons")
dcl(DLG_Options_Label_Text, "Text")
dcl(DLG_Options_Label_Both, "Text and Icon")
dcl(DLG_Options_Label_Show, "Show")
dcl(DLG_Options_Label_Hide, "Hide")
dcl(DLG_Options_Label_Toolbars, "Toolbars")
dcl(DLG_Options_Label_Look, "Button Style")
dcl(DLG_Options_Label_Visible, "Visible")
dcl(DLG_Options_Label_ViewTooltips, "View tooltips")
dcl(DLG_Options_Label_General, "General")
dcl(DLG_Options_Label_Ignore, "Ignore")
dcl(DLG_Options_Label_CustomDict, "custom.dic")
dcl(DLG_Options_Label_Layout, "Layout")
dcl(DLG_Options_Label_Schemes, "Preference Schemes")

dcl(DLG_Tab_TabTitle,					"Tabs")
dcl(DLG_Tab_Label_TabPosition,			"Tab stop position:")
dcl(DLG_Tab_Label_TabToClear,			"Tab stops to be cleared:")
dcl(DLG_Tab_Label_DefaultTS,			"Default tab stops:")

dcl(DLG_Tab_Label_Alignment,			"Alignment")
dcl(DLG_Tab_Radio_Left,					"Left")
dcl(DLG_Tab_Radio_Center,				"Center")
dcl(DLG_Tab_Radio_Right,				"Right")
dcl(DLG_Tab_Radio_Decimal,				"Decimal")
dcl(DLG_Tab_Radio_Bar,					"Bar")

dcl(DLG_Tab_Label_Leader,				"Leader")
dcl(DLG_Tab_Radio_None,					"&1 None")
dcl(DLG_Tab_Radio_Dot,					"&2 ..........")
dcl(DLG_Tab_Radio_Dash,					"&3	----------")
dcl(DLG_Tab_Radio_Underline,			"&4 __________")

dcl(DLG_Tab_Button_Set,					"Set")
dcl(DLG_Tab_Button_Clear,				"Clear")
dcl(DLG_Tab_Button_ClearAll,			"Clear &All")

dcl(DLG_DateTime_DateTimeTitle,			"Date and Time")
dcl(DLG_DateTime_AvailableFormats,		"&Available formats:")

dcl(DLG_Field_FieldTitle,				"Field")
dcl(DLG_Field_Types,					"&Types:")
dcl(DLG_Field_Fields,					"&Fields:")

dcl(FIELD_Type_Datetime,		"Date and Time")
dcl(FIELD_Type_Numbers,			"Numbers")
dcl(FIELD_Type_PieceTable,		"Piece Table")
dcl(FIELD_Datetime_CurrentTime,		"Current Time")
dcl(FIELD_Numbers_PageNumber,		"Page Number")
dcl(FIELD_Numbers_PagesCount,		"Number of Pages")
dcl(FIELD_Numbers_ListLabel,		"List Label")
dcl(FIELD_Numbers_WordCount, "Word Count")
dcl(FIELD_Numbers_CharCount, "Character Count")
dcl(FIELD_Numbers_LineCount, "Line Count")
dcl(FIELD_Numbers_ParaCount, "Paragraph Count")
dcl(FIELD_Numbers_NbspCount, "Character Count (w/o spaces)")
dcl(FIELD_PieceTable_Test,		"Kevins Test")
dcl(FIELD_PieceTable_MartinTest,	"Martins Test")
dcl(FIELD_Datetime_CurrentDate,         "Current Date")

dcl(FIELD_DateTime_MMDDYY, "mm/dd/yy")
dcl(FIELD_DateTime_DDMMYY, "dd/mm/yy")
dcl(FIELD_DateTime_MonthDayYear, "Month Day, Year")
dcl(FIELD_DateTime_MthDayYear, "Mth. Day, Year")
dcl(FIELD_DateTime_DefaultDate, "Default date representation")
dcl(FIELD_DateTime_DefaultDateNoTime, "Default date (w/o time)")
dcl(FIELD_DateTime_Wkday, "The weekday")
dcl(FIELD_DateTime_DOY, "Day # in the year")

dcl(FIELD_DateTime_MilTime, "Military Time")
dcl(FIELD_DateTime_AMPM, "AM/PM")
dcl(FIELD_DateTime_TimeZone, "Time Zone")
dcl(FIELD_DateTime_Epoch, "Seconds since the epoch")

dcl(FIELD_Application, "Application")
dcl(FIELD_Application_Filename, "File Name")
dcl(FIELD_Application_Version, "Version")
dcl(FIELD_Application_BuildId, "Build Id.")
dcl(FIELD_Application_Options, "Build Options")
dcl(FIELD_Application_Target, "Build Target")
dcl(FIELD_Application_CompileDate, "Compile Date")
dcl(FIELD_Application_CompileTime, "Compile Time")

dcl(DLG_Goto_Title,						"Go to...")
dcl(DLG_Goto_Label_Help,			"Choose your target in the left side.\nIf you want to use the \"Go To\" button, just fill the Number Entry with the desired number.  You can use + and - to perform relative movement.  I.e., if you write \"+2\" and you select \"Line\", the \"Go To\" will go 2 lines below your current position.")
dcl(DLG_Goto_Btn_Prev,					"<< Prev")
dcl(DLG_Goto_Btn_Next,					"Next >>")
dcl(DLG_Goto_Label_What,				"Go To &What:")
dcl(DLG_Goto_Label_Number,				"&Number:")
dcl(DLG_Goto_Btn_Goto,					"Go To")
dcl(DLG_Goto_Target_Page,				"Page")
dcl(DLG_Goto_Target_Line,				"Line")
dcl(DLG_Goto_Target_Picture,				"Picture")

// Lists Dialog

dcl(DLG_Lists_Title,                     "Lists for ")
dcl(DLG_Lists_Start_New_List,             "Start New List")
dcl(DLG_Lists_Stop_Current_List,             "Stop Current List")
dcl(DLG_Lists_Resume_Previous_List,             "Resume Previous List")
dcl(DLG_Lists_New_List_Type,             "New List \nType")
dcl(DLG_Lists_Numbered_List,             "Numbered List")
dcl(DLG_Lists_Lower_Case_List,             "Lower Case List")
dcl(DLG_Lists_Upper_Case_List,             "Upper Case List")
dcl(DLG_Lists_Lower_Roman_List,             "Lower Roman List")
dcl(DLG_Lists_Upper_Roman_List,             "Upper Roman List")
dcl(DLG_Lists_Bullet_List,                 "Bullet List")
dcl(DLG_Lists_Dashed_List,                 "Dashed List")
dcl(DLG_Lists_Square_List,                 "Square List")
dcl(DLG_Lists_Triangle_List,                 "Triangle List")
dcl(DLG_Lists_Diamond_List,                 "Diamond List")
dcl(DLG_Lists_Star_List,                 "Star List")
dcl(DLG_Lists_Implies_List,                 "Implies List")
dcl(DLG_Lists_Tick_List,                 "Tick List")
dcl(DLG_Lists_Box_List,                 "Box List")
dcl(DLG_Lists_Hand_List,                 "Hand List")
dcl(DLG_Lists_Heart_List,                 "Heart List")
dcl(DLG_Lists_Starting_Value,             "New Starting \nValue")
dcl(DLG_Lists_New_List_Label,             "New List Label")
dcl(DLG_Lists_Current_List_Type,          "Current List Type")
dcl(DLG_Lists_Current_List_Label,          "Current List Label")
dcl(DLG_Lists_Cur_Change_Start,          "Change Current \nList")
dcl(DLG_Lists_Type,                     "Type:")
dcl(DLG_Lists_Type_none,                     "None")
dcl(DLG_Lists_Type_bullet,                     "Bullet")
dcl(DLG_Lists_Type_numbered,                     "Numbered")
dcl(DLG_Lists_Style,                          "Style:")
dcl(DLG_Lists_Customize,                          "Customized List")
dcl(DLG_Lists_Format,                        "Format:")
dcl(DLG_Lists_Font,                     "Font:")
dcl(DLG_Lists_Level,                     "Level Delimiter:")
dcl(DLG_Lists_Start,                     "Start At:")
dcl(DLG_Lists_Align,                     "Text Align:")
dcl(DLG_Lists_Indent,                     "Label Align:")
dcl(DLG_Lists_Current_Font,                     "Current Font")
dcl(DLG_Lists_Preview,                     "Preview")
dcl(DLG_Lists_Start_New,                     "Start New List")
dcl(DLG_Lists_Apply_Current,                "Apply to Current List")
dcl(DLG_Lists_Start_Sub,                "Start Sublist")
dcl(DLG_Lists_Resume,                "Attach to Previous List")
dcl(DLG_Lists_SetDefault,                "Set Default Values")

     /* page numbers dialog */
dcl(DLG_PageNumbers_Title,           "Page Numbers")
dcl(DLG_PageNumbers_Left,            "Left")
dcl(DLG_PageNumbers_Right,           "Right")
dcl(DLG_PageNumbers_Center,          "Center")
dcl(DLG_PageNumbers_Header,          "Header")
dcl(DLG_PageNumbers_Footer,          "Footer")
dcl(DLG_PageNumbers_Preview,         "Preview")
dcl(DLG_PageNumbers_Alignment,       "Alignment:")
dcl(DLG_PageNumbers_Position,        "Position:")

     /* page setup dialog */
dcl(DLG_PageSetup_Title,  "Page Setup")
dcl(DLG_PageSetup_Paper,  "Paper...")
dcl(DLG_PageSetup_Width,  "&Width:")
dcl(DLG_PageSetup_Height, "&Height:")
dcl(DLG_PageSetup_Paper_Size, "Paper Si&ze:")
dcl(DLG_PageSetup_Units, "&Units:")
dcl(DLG_PageSetup_Orient, "Orientation...")
dcl(DLG_PageSetup_Landscape, "&Landscape")
dcl(DLG_PageSetup_Portrait, "&Portrait")
dcl(DLG_PageSetup_Scale, "Scale...")
dcl(DLG_PageSetup_Adjust, "&Adjust to:")
dcl(DLG_PageSetup_Percent, "% of normal size")
dcl(DLG_PageSetup_Page, "Page")
dcl(DLG_PageSetup_Top, "&Top:")
dcl(DLG_PageSetup_Header, "&Header:")
dcl(DLG_PageSetup_Footer, "&Footer:")
dcl(DLG_PageSetup_Bottom, "&Bottom:")
dcl(DLG_PageSetup_Left, "&Left:")
dcl(DLG_PageSetup_Right, "&Right:")
dcl(DLG_PageSetup_Margin, "&Margin")

dcl(MSG_DirectionModeChg,	"You have changed the direction mode.")
dcl(MSG_DefaultDirectionChg,"You have changed the default direction.")
dcl(MSG_AfterRestartNew, "This change will only take effect when you restart \
AbiWord or create a new document.")

dcl(DLG_ToggleCase_Title, "Change Case")
dcl(DLG_ToggleCase_SentenceCase, "Sentence case")
dcl(DLG_ToggleCase_LowerCase, "lowercase")
dcl(DLG_ToggleCase_UpperCase, "UPPERCASE")
dcl(DLG_ToggleCase_TitleCase, "Title Case")
dcl(DLG_ToggleCase_ToggleCase, "tOGGLE cASE")

dcl(DLG_Background_Title, "Change Background Color")

dcl(MSG_EmptySelection, "Current Selection is Empty")

     /* below are autotext defaults */

     dcl(AUTOTEXT_ATTN_1, "Attention:")
     dcl(AUTOTEXT_ATTN_2, "ATTN:")

     dcl(AUTOTEXT_CLOSING_1, "Best regards,")
     dcl(AUTOTEXT_CLOSING_2, "Best wishes,")
     dcl(AUTOTEXT_CLOSING_3, "Cordially,")
     dcl(AUTOTEXT_CLOSING_4, "Love,")
     dcl(AUTOTEXT_CLOSING_5, "Regards,")
     dcl(AUTOTEXT_CLOSING_6, "Respectfully yours,")
     dcl(AUTOTEXT_CLOSING_7, "Respectfully,")
     dcl(AUTOTEXT_CLOSING_8, "Sincerely yours,")
     dcl(AUTOTEXT_CLOSING_9, "Take care,")
     dcl(AUTOTEXT_CLOSING_10,"Thank you,")
     dcl(AUTOTEXT_CLOSING_11,"Thanks,")
     dcl(AUTOTEXT_CLOSING_12,"Yours truly,")

     dcl(AUTOTEXT_MAIL_1,"CERTIFIED MAIL")
     dcl(AUTOTEXT_MAIL_2,"CONFIDENTIAL")
     dcl(AUTOTEXT_MAIL_3,"PERSONAL")
     dcl(AUTOTEXT_MAIL_4,"REGISTERED MAIL")
     dcl(AUTOTEXT_MAIL_5,"SPECIAL DELIVERY")
     dcl(AUTOTEXT_MAIL_6,"VIA AIRMAIL")
     dcl(AUTOTEXT_MAIL_7,"VIA FACSIMILE")
     dcl(AUTOTEXT_MAIL_8,"VIA OVERNIGHT MAIL")

     dcl(AUTOTEXT_REFERENCE_1,"In reply to:")
     dcl(AUTOTEXT_REFERENCE_2,"RE:")
     dcl(AUTOTEXT_REFERENCE_3,"Reference:")

     dcl(AUTOTEXT_SALUTATION_1,"Dear Mom and Dad,")
     dcl(AUTOTEXT_SALUTATION_2,"Dear Sir or Madam:")
     dcl(AUTOTEXT_SALUTATION_3,"Ladies and Gentlemen:")
     dcl(AUTOTEXT_SALUTATION_4,"To Whom It May Concern:")

     dcl(AUTOTEXT_SUBJECT_1,"Subject")

     dcl(AUTOTEXT_EMAIL_1, "To:")
     dcl(AUTOTEXT_EMAIL_2, "From:")
     dcl(AUTOTEXT_EMAIL_3, "Subject:")
     dcl(AUTOTEXT_EMAIL_4, "CC:")
     dcl(AUTOTEXT_EMAIL_5, "BCC:")
     dcl(AUTOTEXT_EMAIL_6, "Fwd:")
     
