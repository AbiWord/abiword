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
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

// Message Boxes used in AP_EditMethods
dcl(MSG_SaveFailed,		"Could not write to the file %s.")
dcl(MSG_RevertBuffer,	"Revert to saved copy of %s?")
dcl(MSG_QueryExit,		"Close all windows and exit?")
dcl(MSG_ConfirmSave,	"Save changes to %s?")
dcl(MSG_ImportError,	"Error importing file %s.")
dcl(MSG_SpellDone,		"The spelling check is complete.")

// Status Bar Messages
dcl(PageInfoField,		"Page: %d/%d")

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

dcl(DLG_Para_PreviewSampleFallback, "This paragraph represents words as they might appear in your document.  "
                                    "To see text from your document used in this preview, position your cursor "
                                    "into a document paragraph with some text in it and launch this dialog.")
