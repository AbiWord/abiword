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
*****************************************************************/


/*****************************************************************/
/*****************************************************************/
/** This file defines the set of Id's used for all menu-related **/
/** things.  Each Id defines a conceptual unit which may be     **/
/** used on one or more menus or not at all.                    **/
/*****************************************************************/
/*****************************************************************/

/*****************************************************************/
/*  Each menuitem(XXX) will create an ID  AP_MENU_ID_XXX         */
/*  You will have to add MENU_LABEL_XXX                          */
/*                   and MENU_STATUS_LINE_XXX                    */
/*  to the ap_Strings_Id.h file                                  */
/*****************************************************************/

menuitem(FILE)
menuitem(FILE_NEW)
menuitem(FILE_OPEN)
menuitem(FILE_SAVE)
menuitem(FILE_SAVEAS)
menuitem(FILE_SAVE_TEMPLATE)
menuitem(FILE_SAVEIMAGE)
menuitem(FILE_EXPORT)
menuitem(FILE_IMPORT)
menuitem(FILE_CLOSE)
menuitem(FILE_PROPERTIES)
menuitem(FILE_PAGESETUP)
menuitem(FILE_PRINT)
menuitem(FILE_PRINT_PREVIEW)
menuitem(FILE_PRINT_DIRECTLY)
menuitem(FILE_RECENT)
menuitem(FILE_RECENT_1)		// _recent_1 thru _recent_9 must be contiguous
menuitem(FILE_RECENT_2)
menuitem(FILE_RECENT_3)
menuitem(FILE_RECENT_4)
menuitem(FILE_RECENT_5)
menuitem(FILE_RECENT_6)
menuitem(FILE_RECENT_7)
menuitem(FILE_RECENT_8)
menuitem(FILE_RECENT_9)
menuitem(FILE_REVERT)
menuitem(FILE_EXIT)

menuitem(OPEN_TEMPLATE)

menuitem(EDIT)
menuitem(EDIT_UNDO)
menuitem(EDIT_REDO)
menuitem(EDIT_CUT)
menuitem(EDIT_COPY)
menuitem(EDIT_PASTE)
menuitem(EDIT_PASTE_SPECIAL)
menuitem(EDIT_CLEAR)
menuitem(EDIT_SELECTALL)
menuitem(EDIT_FIND)
menuitem(EDIT_REPLACE)
menuitem(EDIT_GOTO)
menuitem(EDIT_EDITHEADER)
menuitem(EDIT_EDITFOOTER)
menuitem(EDIT_REMOVEHEADER)
menuitem(EDIT_REMOVEFOOTER)

menuitem(VIEW)
menuitem(VIEW_TOOLBARS)
menuitem(VIEW_TB_STD)
menuitem(VIEW_TB_FORMAT)
menuitem(VIEW_TB_EXTRA)
menuitem(VIEW_TB_TABLE)
menuitem(VIEW_RULER)
menuitem(VIEW_STATUSBAR)
menuitem(VIEW_SHOWPARA)
menuitem(VIEW_LOCKSTYLES)
menuitem(VIEW_HEADFOOT) // This has been removed.

menuitem(VIEW_ZOOM_MENU)
menuitem(VIEW_ZOOM)
menuitem(VIEW_ZOOM_200)
menuitem(VIEW_ZOOM_100)
menuitem(VIEW_ZOOM_75)
menuitem(VIEW_ZOOM_50)
menuitem(VIEW_ZOOM_WIDTH)
menuitem(VIEW_ZOOM_WHOLE)
menuitem(VIEW_FULLSCREEN)
menuitem(VIEW_NORMAL)
menuitem(VIEW_WEB)
menuitem(VIEW_PRINT)

menuitem(INSERT)
menuitem(INSERT_BREAK)
menuitem(INSERT_PAGENO)
menuitem(INSERT_DATETIME)
menuitem(INSERT_FIELD)
menuitem(INSERT_MAILMERGE)
menuitem(INSERT_FILE)
menuitem(INSERT_SYMBOL)
menuitem(INSERT_FOOTNOTE)
menuitem(INSERT_ENDNOTE)
menuitem(INSERT_PICTURE)
menuitem(INSERT_CLIPART)
menuitem(INSERT_GRAPHIC)
menuitem(INSERT_BOOKMARK)
menuitem(INSERT_HYPERLINK)
menuitem(INSERT_DELETE_HYPERLINK)

menuitem(FORMAT)
menuitem(FMT_FONT)
menuitem(FMT_PARAGRAPH)
menuitem(FMT_BULLETS)
menuitem(FMT_DOCUMENT)
menuitem(FMT_BORDERS)
menuitem(FMT_COLUMNS)
menuitem(FMT_TOGGLECASE)
menuitem(FMT_BACKGROUND)
menuitem(FMT_HDRFTR)
menuitem(FMT_FOOTNOTES)
menuitem(FMT_STYLE)
menuitem(FMT_TABS)
menuitem(FMT_LANGUAGE)
menuitem(FMT_IMAGE)

menuitem(FMT)
menuitem(FMT_BOLD)
menuitem(FMT_ITALIC)
menuitem(FMT_UNDERLINE)
menuitem(FMT_OVERLINE)
menuitem(FMT_STRIKE)
menuitem(FMT_TOPLINE)
menuitem(FMT_BOTTOMLINE)
menuitem(FMT_SUPERSCRIPT)
menuitem(FMT_SUBSCRIPT)

menuitem(TOOLS)
menuitem(TOOLS_SPELLING)
menuitem(TOOLS_SPELL)
menuitem(TOOLS_AUTOSPELL)
menuitem(TOOLS_SPELLPREFS)
menuitem(TOOLS_WORDCOUNT)
menuitem(TOOLS_MAILMERGE)
menuitem(TOOLS_PLUGINS)
menuitem(TOOLS_SCRIPTS)
menuitem(TOOLS_OPTIONS)
menuitem(TOOLS_LANGUAGE)
menuitem(TOOLS_REVISIONS)
menuitem(TOOLS_REVISIONS_MARK)
menuitem(TOOLS_REVISIONS_SET_VIEW_LEVEL)
menuitem(TOOLS_REVISIONS_ACCEPT_REVISION)
menuitem(TOOLS_REVISIONS_REJECT_REVISION)
menuitem(CONTEXT_REVISIONS_ACCEPT_REVISION)
menuitem(CONTEXT_REVISIONS_REJECT_REVISION)
menuitem(ALIGN)
menuitem(ALIGN_LEFT)
menuitem(ALIGN_CENTER)
menuitem(ALIGN_RIGHT)
menuitem(ALIGN_JUSTIFY)


menuitem(TABLE)
menuitem(TABLE_INSERT)
menuitem(TABLE_INSERT_TABLE)
menuitem(TABLE_INSERTTABLE)
menuitem(TABLE_INSERT_COLUMNS_BEFORE)
menuitem(TABLE_INSERT_COLUMNS_AFTER)
menuitem(TABLE_INSERTCOLUMN)
menuitem(TABLE_INSERT_ROWS_BEFORE)
menuitem(TABLE_INSERT_ROWS_AFTER)
menuitem(TABLE_INSERTROW)
menuitem(TABLE_INSERT_CELLS)

menuitem(TABLE_DELETE)
menuitem(TABLE_DELETE_TABLE)
menuitem(TABLE_DELETETABLE)
menuitem(TABLE_DELETE_COLUMNS)
menuitem(TABLE_DELETECOLUMN)
menuitem(TABLE_DELETE_ROWS)
menuitem(TABLE_DELETEROW)
menuitem(TABLE_DELETE_CELLS)

menuitem(TABLE_SELECT)
menuitem(TABLE_SELECT_TABLE)
menuitem(TABLE_SELECT_COLUMN)
menuitem(TABLE_SELECT_ROW)
menuitem(TABLE_SELECT_CELL)

menuitem(TABLE_MERGE_CELLS)
menuitem(TABLE_SPLIT_CELLS)
menuitem(TABLE_SPLIT_TABLE)
menuitem(TABLE_FORMAT)
menuitem(TABLE_AUTOFIT)
menuitem(TABLE_HEADING_ROWS_REPEAT)
menuitem(TABLE_SORT)

menuitem(WINDOW)
menuitem(WINDOW_NEW)
menuitem(WINDOW_1)		// _window_1 thru _window_9 must be contiguous
menuitem(WINDOW_2)
menuitem(WINDOW_3)
menuitem(WINDOW_4)
menuitem(WINDOW_5)
menuitem(WINDOW_6)
menuitem(WINDOW_7)
menuitem(WINDOW_8)
menuitem(WINDOW_9)
menuitem(WINDOW_MORE)

menuitem(WEB_WEBPREVIEW)
menuitem(WEB_SAVEASWEB)

menuitem(HELP)
menuitem(HELP_CREDITS)
menuitem(HELP_CONTENTS)
menuitem(HELP_INDEX)
menuitem(HELP_CHECKVER)
menuitem(HELP_SEARCH)
menuitem(HELP_ABOUT)
menuitem(HELP_ABOUTOS)
menuitem(HELP_ABOUT_GNU)
menuitem(HELP_ABOUT_GNOMEOFFICE)
menuitem(HELP_REPORT_BUG)

menuitem(SPELL_SUGGEST_1)		// _suggest_1 thru _suggest_9 must be contiguous
menuitem(SPELL_SUGGEST_2)
menuitem(SPELL_SUGGEST_3)
menuitem(SPELL_SUGGEST_4)
menuitem(SPELL_SUGGEST_5)
menuitem(SPELL_SUGGEST_6)
menuitem(SPELL_SUGGEST_7)
menuitem(SPELL_SUGGEST_8)
menuitem(SPELL_SUGGEST_9)
menuitem(SPELL_IGNOREALL)
menuitem(SPELL_ADD)

	/* the following entries are for autotext */
menuitem(INSERT_AUTOTEXT)
menuitem(AUTOTEXT_ATTN)
menuitem(AUTOTEXT_CLOSING)
menuitem(AUTOTEXT_MAIL)
menuitem(AUTOTEXT_REFERENCE)
menuitem(AUTOTEXT_SALUTATION)
menuitem(AUTOTEXT_SUBJECT)
menuitem(AUTOTEXT_EMAIL)

menuitem(AUTOTEXT_ATTN_1)
menuitem(AUTOTEXT_ATTN_2)

menuitem(AUTOTEXT_CLOSING_1)
menuitem(AUTOTEXT_CLOSING_2)
menuitem(AUTOTEXT_CLOSING_3)
menuitem(AUTOTEXT_CLOSING_4)
menuitem(AUTOTEXT_CLOSING_5)
menuitem(AUTOTEXT_CLOSING_6)
menuitem(AUTOTEXT_CLOSING_7)
menuitem(AUTOTEXT_CLOSING_8)
menuitem(AUTOTEXT_CLOSING_9)
menuitem(AUTOTEXT_CLOSING_10)
menuitem(AUTOTEXT_CLOSING_11)
menuitem(AUTOTEXT_CLOSING_12)

menuitem(AUTOTEXT_MAIL_1)
menuitem(AUTOTEXT_MAIL_2)
menuitem(AUTOTEXT_MAIL_3)
menuitem(AUTOTEXT_MAIL_4)
menuitem(AUTOTEXT_MAIL_5)
menuitem(AUTOTEXT_MAIL_6)
menuitem(AUTOTEXT_MAIL_7)
menuitem(AUTOTEXT_MAIL_8)

menuitem(AUTOTEXT_REFERENCE_1)
menuitem(AUTOTEXT_REFERENCE_2)
menuitem(AUTOTEXT_REFERENCE_3)

menuitem(AUTOTEXT_SALUTATION_1)
menuitem(AUTOTEXT_SALUTATION_2)
menuitem(AUTOTEXT_SALUTATION_3)
menuitem(AUTOTEXT_SALUTATION_4)

menuitem(AUTOTEXT_SUBJECT_1)

menuitem(AUTOTEXT_EMAIL_1)
menuitem(AUTOTEXT_EMAIL_2)
menuitem(AUTOTEXT_EMAIL_3)
menuitem(AUTOTEXT_EMAIL_4)
menuitem(AUTOTEXT_EMAIL_5)
menuitem(AUTOTEXT_EMAIL_6)

/**** Add Menu Items here ****/


