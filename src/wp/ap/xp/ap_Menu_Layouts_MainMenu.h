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

BeginLayout(Main,0)

	BeginSubMenu(AP_MENU_ID_FILE)
		MenuItem(AP_MENU_ID_FILE_NEW)
		MenuItem(AP_MENU_ID_FILE_NEW_USING_TEMPLATE)
		MenuItem(AP_MENU_ID_FILE_OPEN)
		MenuItem(AP_MENU_ID_FILE_SAVE)
		MenuItem(AP_MENU_ID_FILE_SAVEAS)
//		MenuItem(AP_MENU_ID_FILE_SAVE_TEMPLATE)
		Separator()
		MenuItem(AP_MENU_ID_FILE_IMPORT)
		MenuItem(AP_MENU_ID_FILE_EXPORT)
		MenuItem(AP_MENU_ID_FILE_REVERT)

		BeginSubMenu(AP_MENU_ID_FILE_RECENT)
			MenuItem(AP_MENU_ID_FILE_RECENT_1)
			MenuItem(AP_MENU_ID_FILE_RECENT_2)
			MenuItem(AP_MENU_ID_FILE_RECENT_3)
			MenuItem(AP_MENU_ID_FILE_RECENT_4)
			MenuItem(AP_MENU_ID_FILE_RECENT_5)
			MenuItem(AP_MENU_ID_FILE_RECENT_6)
			MenuItem(AP_MENU_ID_FILE_RECENT_7)
			MenuItem(AP_MENU_ID_FILE_RECENT_8)
			MenuItem(AP_MENU_ID_FILE_RECENT_9)
		EndSubMenu()

		Separator()
		MenuItem(AP_MENU_ID_FILE_PROPERTIES)
		MenuItem(AP_MENU_ID_FILE_PAGESETUP)
#if defined(HAVE_GNOME)
		MenuItem(AP_MENU_ID_FILE_PRINT_PREVIEW)
#endif
		MenuItem(AP_MENU_ID_FILE_PRINT)
		Separator()
		MenuItem(AP_MENU_ID_FILE_CLOSE)
		MenuItem(AP_MENU_ID_FILE_EXIT)
	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_EDIT)
		MenuItem(AP_MENU_ID_EDIT_UNDO)
		MenuItem(AP_MENU_ID_EDIT_REDO)
		Separator()
		MenuItem(AP_MENU_ID_EDIT_CUT)
		MenuItem(AP_MENU_ID_EDIT_COPY)
		MenuItem(AP_MENU_ID_EDIT_PASTE)
		MenuItem(AP_MENU_ID_EDIT_PASTE_SPECIAL)
		Separator()
		MenuItem(AP_MENU_ID_EDIT_CLEAR)
		MenuItem(AP_MENU_ID_EDIT_SELECTALL)
		Separator()
		MenuItem(AP_MENU_ID_EDIT_FIND)
		MenuItem(AP_MENU_ID_EDIT_REPLACE)
		MenuItem(AP_MENU_ID_EDIT_GOTO)
#if !(XAP_PREFSMENU_UNDER_TOOLS)
		Separator()
		MenuItem(AP_MENU_ID_TOOLS_OPTIONS)
#endif
	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_VIEW)
		MenuItem(AP_MENU_ID_VIEW_NORMAL)
		MenuItem(AP_MENU_ID_VIEW_WEB)
		MenuItem(AP_MENU_ID_VIEW_PRINT)
		MenuItem(AP_MENU_ID_WEB_WEBPREVIEW)
		Separator()

		BeginSubMenu(AP_MENU_ID_VIEW_TOOLBARS)
			MenuItem(AP_MENU_ID_VIEW_TB_1)
			MenuItem(AP_MENU_ID_VIEW_TB_2)
			MenuItem(AP_MENU_ID_VIEW_TB_3)
			MenuItem(AP_MENU_ID_VIEW_TB_4)			
// Currently we only can change toolbars in UNIX builds
#ifdef XP_UNIX_TARGET_GTK
			Separator()
			MenuItem(AP_MENU_ID_VIEW_LOCK_TB_LAYOUT)
			MenuItem(AP_MENU_ID_VIEW_DEFAULT_TB_LAYOUT)
#endif
		EndSubMenu()

		MenuItem(AP_MENU_ID_VIEW_RULER)
		MenuItem(AP_MENU_ID_VIEW_STATUSBAR)
		Separator()
		MenuItem(AP_MENU_ID_VIEW_LOCKSTYLES)
		MenuItem(AP_MENU_ID_VIEW_SHOWPARA)
		Separator()
		MenuItem(AP_MENU_ID_VIEW_FULLSCREEN)

		BeginSubMenu(AP_MENU_ID_VIEW_ZOOM_MENU)
			MenuItem(AP_MENU_ID_VIEW_ZOOM)
			MenuItem(AP_MENU_ID_VIEW_ZOOM_WIDTH)
			MenuItem(AP_MENU_ID_VIEW_ZOOM_WHOLE)
			MenuItem(AP_MENU_ID_VIEW_ZOOM_200)
			MenuItem(AP_MENU_ID_VIEW_ZOOM_100)
			MenuItem(AP_MENU_ID_VIEW_ZOOM_75)
			MenuItem(AP_MENU_ID_VIEW_ZOOM_50)
		EndSubMenu()

	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_INSERT)
		MenuItem(AP_MENU_ID_INSERT_BREAK)
		MenuItem(AP_MENU_ID_INSERT_PAGENO)
		MenuItem(AP_MENU_ID_INSERT_DATETIME)
		MenuItem(AP_MENU_ID_INSERT_FIELD)
		MenuItem(AP_MENU_ID_INSERT_TEXTBOX)
		MenuItem(AP_MENU_ID_INSERT_MAILMERGE)
		MenuItem(AP_MENU_ID_INSERT_SYMBOL)

		/* the autotext submenus */

		BeginSubMenu(AP_MENU_ID_INSERT_AUTOTEXT)

			BeginSubMenu(AP_MENU_ID_AUTOTEXT_ATTN)
				MenuItem(AP_MENU_ID_AUTOTEXT_ATTN_1)
				MenuItem(AP_MENU_ID_AUTOTEXT_ATTN_2)
			EndSubMenu()

			BeginSubMenu(AP_MENU_ID_AUTOTEXT_CLOSING)
				MenuItem(AP_MENU_ID_AUTOTEXT_CLOSING_1)
				MenuItem(AP_MENU_ID_AUTOTEXT_CLOSING_2)
				MenuItem(AP_MENU_ID_AUTOTEXT_CLOSING_3)
				MenuItem(AP_MENU_ID_AUTOTEXT_CLOSING_4)
				MenuItem(AP_MENU_ID_AUTOTEXT_CLOSING_5)
				MenuItem(AP_MENU_ID_AUTOTEXT_CLOSING_6)
				MenuItem(AP_MENU_ID_AUTOTEXT_CLOSING_7)
				MenuItem(AP_MENU_ID_AUTOTEXT_CLOSING_8)
				MenuItem(AP_MENU_ID_AUTOTEXT_CLOSING_9)
				MenuItem(AP_MENU_ID_AUTOTEXT_CLOSING_10)
				MenuItem(AP_MENU_ID_AUTOTEXT_CLOSING_11)
				MenuItem(AP_MENU_ID_AUTOTEXT_CLOSING_12)
			EndSubMenu()

			BeginSubMenu(AP_MENU_ID_AUTOTEXT_MAIL)
				MenuItem(AP_MENU_ID_AUTOTEXT_MAIL_1)
				MenuItem(AP_MENU_ID_AUTOTEXT_MAIL_2)
				MenuItem(AP_MENU_ID_AUTOTEXT_MAIL_3)
				MenuItem(AP_MENU_ID_AUTOTEXT_MAIL_4)
				MenuItem(AP_MENU_ID_AUTOTEXT_MAIL_5)
				MenuItem(AP_MENU_ID_AUTOTEXT_MAIL_6)
				MenuItem(AP_MENU_ID_AUTOTEXT_MAIL_7)
				MenuItem(AP_MENU_ID_AUTOTEXT_MAIL_8)
			EndSubMenu()

			BeginSubMenu(AP_MENU_ID_AUTOTEXT_REFERENCE)
				MenuItem(AP_MENU_ID_AUTOTEXT_REFERENCE_1)
				MenuItem(AP_MENU_ID_AUTOTEXT_REFERENCE_2)
				MenuItem(AP_MENU_ID_AUTOTEXT_REFERENCE_3)
			EndSubMenu()

			BeginSubMenu(AP_MENU_ID_AUTOTEXT_SALUTATION)
				MenuItem(AP_MENU_ID_AUTOTEXT_SALUTATION_1)
				MenuItem(AP_MENU_ID_AUTOTEXT_SALUTATION_2)
				MenuItem(AP_MENU_ID_AUTOTEXT_SALUTATION_3)
				MenuItem(AP_MENU_ID_AUTOTEXT_SALUTATION_4)
			EndSubMenu()

			BeginSubMenu(AP_MENU_ID_AUTOTEXT_EMAIL)
				MenuItem(AP_MENU_ID_AUTOTEXT_EMAIL_1)
				MenuItem(AP_MENU_ID_AUTOTEXT_EMAIL_2)
				MenuItem(AP_MENU_ID_AUTOTEXT_EMAIL_3)
				MenuItem(AP_MENU_ID_AUTOTEXT_EMAIL_4)
				MenuItem(AP_MENU_ID_AUTOTEXT_EMAIL_5)
				MenuItem(AP_MENU_ID_AUTOTEXT_EMAIL_6)
			EndSubMenu()

			BeginSubMenu(AP_MENU_ID_AUTOTEXT_SUBJECT)
				MenuItem(AP_MENU_ID_AUTOTEXT_SUBJECT_1)
			EndSubMenu()

		EndSubMenu()

		Separator()
		MenuItem(AP_MENU_ID_INSERT_HEADER)
		MenuItem(AP_MENU_ID_INSERT_FOOTER)
		MenuItem(AP_MENU_ID_INSERT_FILE)
		MenuItem(AP_MENU_ID_INSERT_BOOKMARK)
		MenuItem(AP_MENU_ID_INSERT_HYPERLINK)
		MenuItem(AP_MENU_ID_INSERT_TABLEOFCONTENTS)
		MenuItem(AP_MENU_ID_INSERT_FOOTNOTE)
		MenuItem(AP_MENU_ID_INSERT_ENDNOTE)

#ifdef HAVE_GNOME
		Separator()

		BeginSubMenu(AP_MENU_ID_INSERT_PICTURE)
			MenuItem(AP_MENU_ID_INSERT_CLIPART)
			MenuItem(AP_MENU_ID_INSERT_GRAPHIC)
		EndSubMenu()
#else
		MenuItem(AP_MENU_ID_INSERT_PICTURE)
#endif
		BeginSubMenu(AP_MENU_ID_INSERT_DIRECTIONMARKER)
 	        MenuItem(AP_MENU_ID_INSERT_DIRECTIONMARKER_LRM)
	        MenuItem(AP_MENU_ID_INSERT_DIRECTIONMARKER_RLM)
	    EndSubMenu()

	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_FORMAT)
		MenuItem(AP_MENU_ID_FMT_FONT)
		MenuItem(AP_MENU_ID_FMT_PARAGRAPH)
		MenuItem(AP_MENU_ID_FMT_BULLETS)
		MenuItem(AP_MENU_ID_FMT_DOCUMENT)
		MenuItem(AP_MENU_ID_FMT_FRAME)
        MenuItem(AP_MENU_ID_FMT_IMAGE)
#if 0 // someone code and turn this back on
		MenuItem(AP_MENU_ID_FMT_BORDERS)
#endif
		Separator()
		MenuItem(AP_MENU_ID_FMT_COLUMNS)
		MenuItem(AP_MENU_ID_FMT_TABS)
		MenuItem(AP_MENU_ID_FMT_HDRFTR)
		MenuItem(AP_MENU_ID_FMT_FOOTNOTES)
		MenuItem(AP_MENU_ID_FMT_TABLEOFCONTENTS)
		Separator()
		MenuItem(AP_MENU_ID_FMT_TOGGLECASE)
		Separator()

		BeginSubMenu(AP_MENU_ID_ALIGN)
			MenuItem(AP_MENU_ID_ALIGN_LEFT)
			MenuItem(AP_MENU_ID_ALIGN_CENTER)
			MenuItem(AP_MENU_ID_ALIGN_RIGHT)
			MenuItem(AP_MENU_ID_ALIGN_JUSTIFY)
		EndSubMenu()

		BeginSubMenu(AP_MENU_ID_FMT)
			MenuItem(AP_MENU_ID_FMT_BOLD)
			MenuItem(AP_MENU_ID_FMT_ITALIC)
			MenuItem(AP_MENU_ID_FMT_UNDERLINE)
			MenuItem(AP_MENU_ID_FMT_OVERLINE)
			MenuItem(AP_MENU_ID_FMT_STRIKE)
			//MenuItem(AP_MENU_ID_FMT_TOPLINE)
			//MenuItem(AP_MENU_ID_FMT_BOTTOMLINE)
			MenuItem(AP_MENU_ID_FMT_SUPERSCRIPT)
			MenuItem(AP_MENU_ID_FMT_SUBSCRIPT)
		EndSubMenu()

		BeginSubMenu(AP_MENU_ID_FMT_STYLE)
			MenuItem(AP_MENU_ID_FMT_STYLE_DEFINE)
			MenuItem(AP_MENU_ID_FMT_IMPORTSTYLES)
	        MenuItem(AP_MENU_ID_FMT_STYLIST)
		EndSubMenu()
		Separator()
		MenuItem(AP_MENU_ID_EDIT_REMOVEHEADER)
		MenuItem(AP_MENU_ID_EDIT_REMOVEFOOTER)

		BeginSubMenu(AP_MENU_ID_FMT_BACKGROUND)
		    MenuItem(AP_MENU_ID_FMT_BACKGROUND_PAGE_IMAGE)
		    MenuItem(AP_MENU_ID_FMT_BACKGROUND_PAGE_COLOR)
	    EndSubMenu()

	    BeginSubMenu(AP_MENU_ID_FMT_DIRECTION)
	        MenuItem(AP_MENU_ID_FMT_DIRECTION_DD_RTL)
        	MenuItem(AP_MENU_ID_FMT_DIRECTION_DO_LTR)
        	MenuItem(AP_MENU_ID_FMT_DIRECTION_DO_RTL)
    	EndSubMenu()
	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_TOOLS)

		BeginSubMenu(AP_MENU_ID_TOOLS_SPELLING)
			MenuItem(AP_MENU_ID_TOOLS_SPELL)
			MenuItem(AP_MENU_ID_TOOLS_AUTOSPELL)
			MenuItem(AP_MENU_ID_TOOLS_SPELLPREFS)
		EndSubMenu()

		MenuItem(AP_MENU_ID_FMT_LANGUAGE)
		MenuItem(AP_MENU_ID_TOOLS_WORDCOUNT)

		Separator()

	    BeginSubMenu(AP_MENU_ID_TOOLS_HISTORY)
	        MenuItem(AP_MENU_ID_TOOLS_HISTORY_SHOW)
	        MenuItem(AP_MENU_ID_TOOLS_REVISIONS_COMPARE_DOCUMENTS)
		    MenuItem(AP_MENU_ID_TOOLS_REVISIONS_AUTO)
		EndSubMenu()
	
		BeginSubMenu(AP_MENU_ID_TOOLS_REVISIONS)
			MenuItem(AP_MENU_ID_TOOLS_REVISIONS_MARK)
		    Separator()
	        MenuItem(AP_MENU_ID_TOOLS_REVISIONS_SHOW)
			MenuItem(AP_MENU_ID_TOOLS_REVISIONS_SHOW_AFTER)
			MenuItem(AP_MENU_ID_TOOLS_REVISIONS_SHOW_AFTERPREV)
			MenuItem(AP_MENU_ID_TOOLS_REVISIONS_SHOW_BEFORE)
			MenuItem(AP_MENU_ID_TOOLS_REVISIONS_SET_VIEW_LEVEL)
	        Separator()
			MenuItem(AP_MENU_ID_TOOLS_REVISIONS_FIND_NEXT)
			MenuItem(AP_MENU_ID_TOOLS_REVISIONS_FIND_PREV)
	        Separator()
			MenuItem(AP_MENU_ID_TOOLS_REVISIONS_ACCEPT_REVISION)
			MenuItem(AP_MENU_ID_TOOLS_REVISIONS_REJECT_REVISION)
#ifdef DEBUG
	        Separator()
		    MenuItem(AP_MENU_ID_TOOLS_REVISIONS_MERGE_DOCUMENTS)
#endif
		EndSubMenu()
	
	    Separator()

		MenuItem(AP_MENU_ID_TOOLS_PLUGINS)
		MenuItem(AP_MENU_ID_TOOLS_SCRIPTS)
		MenuItem(AP_MENU_ID_TOOLS_MAILMERGE)
#ifndef XP_MAC_TARGET_MACOSX
		// On MacOS X don't put a separator as the "Option" menu item is moved away at run time
#endif
#if XAP_PREFSMENU_UNDER_TOOLS
		Separator()
		MenuItem(AP_MENU_ID_TOOLS_OPTIONS)
#endif
	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_TABLE)

    		BeginSubMenu(AP_MENU_ID_TABLE_INSERT)
			MenuItem(AP_MENU_ID_TABLE_INSERT_TABLE)
			MenuItem(AP_MENU_ID_TABLE_INSERT_COLUMNS_BEFORE)
			MenuItem(AP_MENU_ID_TABLE_INSERT_COLUMNS_AFTER)
			MenuItem(AP_MENU_ID_TABLE_INSERT_ROWS_BEFORE)
			MenuItem(AP_MENU_ID_TABLE_INSERT_ROWS_AFTER)
			MenuItem(AP_MENU_ID_TABLE_INSERT_SUMCOLS)
			MenuItem(AP_MENU_ID_TABLE_INSERT_SUMROWS)
#if 0
// Not for 2.2
			MenuItem(AP_MENU_ID_TABLE_INSERT_CELLS)
#endif
		EndSubMenu()

		BeginSubMenu(AP_MENU_ID_TABLE_DELETE)
			MenuItem(AP_MENU_ID_TABLE_DELETE_TABLE)
			MenuItem(AP_MENU_ID_TABLE_DELETE_COLUMNS)
			MenuItem(AP_MENU_ID_TABLE_DELETE_ROWS)
#if 0
// Not for 2.2
			MenuItem(AP_MENU_ID_TABLE_DELETE_CELLS)
#endif
		EndSubMenu()

   		BeginSubMenu(AP_MENU_ID_TABLE_SELECT)
			MenuItem(AP_MENU_ID_TABLE_SELECT_TABLE)
			MenuItem(AP_MENU_ID_TABLE_SELECT_COLUMN)
			MenuItem(AP_MENU_ID_TABLE_SELECT_ROW)
			MenuItem(AP_MENU_ID_TABLE_SELECT_CELL)
		EndSubMenu()

		Separator()
		MenuItem(AP_MENU_ID_TABLE_MERGE_CELLS)
		MenuItem(AP_MENU_ID_TABLE_SPLIT_CELLS)
#if 0
// Not for 2.2
		MenuItem(AP_MENU_ID_TABLE_SPLIT_TABLE)
#endif
		MenuItem(AP_MENU_ID_TABLE_FORMAT)
		MenuItem(AP_MENU_ID_TABLE_TEXTTOTABLE)
#if DEBUG
	    BeginSubMenu(AP_MENU_ID_TABLE_SORT)
	       MenuItem(AP_MENU_ID_TABLE_SORTROWSASCEND)
	       MenuItem(AP_MENU_ID_TABLE_SORTROWSDESCEND)
	       MenuItem(AP_MENU_ID_TABLE_SORTCOLSASCEND)
	       MenuItem(AP_MENU_ID_TABLE_SORTCOLSDESCEND)
	    EndSubMenu()
#endif
		MenuItem(AP_MENU_ID_TABLE_AUTOFIT)

#if 0
// Not for 2.2
		MenuItem(AP_MENU_ID_TABLE_HEADING_ROWS_REPEAT)
#endif
	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_WINDOW)
		MenuItem(AP_MENU_ID_WINDOW_NEW)
		Separator()
		MenuItem(AP_MENU_ID_WINDOW_1)
		MenuItem(AP_MENU_ID_WINDOW_2)
		MenuItem(AP_MENU_ID_WINDOW_3)
		MenuItem(AP_MENU_ID_WINDOW_4)
		MenuItem(AP_MENU_ID_WINDOW_5)
		MenuItem(AP_MENU_ID_WINDOW_6)
		MenuItem(AP_MENU_ID_WINDOW_7)
		MenuItem(AP_MENU_ID_WINDOW_8)
		MenuItem(AP_MENU_ID_WINDOW_9)
		MenuItem(AP_MENU_ID_WINDOW_MORE)
	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_HELP)
		MenuItem(AP_MENU_ID_HELP_CONTENTS)
		MenuItem(AP_MENU_ID_HELP_INDEX)
		MenuItem(AP_MENU_ID_HELP_SEARCH)
		MenuItem(AP_MENU_ID_HELP_CHECKVER)
		MenuItem(AP_MENU_ID_HELP_REPORT_BUG)
		Separator()
		MenuItem(AP_MENU_ID_HELP_ABOUTOS)
		MenuItem(AP_MENU_ID_HELP_ABOUT_GNU)
#ifdef HAVE_GNOME
		MenuItem(AP_MENU_ID_HELP_ABOUT_GNOMEOFFICE)
#endif
		Separator()
#ifndef HAVE_GNOME
		MenuItem(AP_MENU_ID_HELP_CREDITS)
#ifndef XP_MAC_TARGET_MACOSX
		// On MacOS X don't put a separator as the "About" menu item is moved away at run time
		Separator()
#endif
#endif
		MenuItem(AP_MENU_ID_HELP_ABOUT)
	EndSubMenu()

EndLayout()
