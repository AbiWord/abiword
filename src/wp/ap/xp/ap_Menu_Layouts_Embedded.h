/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ap_Features.h"
#ifdef APF_MENU_LAYOUTS_EMBEDDED_MENU
#  include APF_MENU_LAYOUTS_EMBEDDED_MENU
#else

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

/* This is the main menu for embedded targets
 *
 * The basic 'design' principles:
 *
 * 1. What is not built in, should not be on the menu.
 * 2. What is on the embedded toolbar should not be on the menu.
 */

BeginLayout(Main,0)

	BeginSubMenu(AP_MENU_ID_FILE)
		MenuItem(AP_MENU_ID_FILE_NEW_USING_TEMPLATE)
		MenuItem(AP_MENU_ID_FILE_OPEN)

		Separator()
		MenuItem(AP_MENU_ID_FILE_SAVEAS)
		MenuItem(AP_MENU_ID_FILE_REVERT)

#ifdef ENABLE_PRINT
		Separator()
		MenuItem(AP_MENU_ID_FILE_PAGESETUP)
		MenuItem(AP_MENU_ID_FILE_PRINT)
#endif
		Separator()
		MenuItem(AP_MENU_ID_FILE_PROPERTIES)
		Separator()
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
		MenuItem(AP_MENU_ID_FILE_EXIT)
	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_EDIT)
#if 0
		MenuItem(AP_MENU_ID_EDIT_UNDO)
		MenuItem(AP_MENU_ID_EDIT_REDO)
#endif
		MenuItem(AP_MENU_ID_EDIT_PASTE_SPECIAL)
		MenuItem(AP_MENU_ID_EDIT_SELECTALL)
		Separator()
		MenuItem(AP_MENU_ID_EDIT_REMOVEHEADER)
		MenuItem(AP_MENU_ID_EDIT_REMOVEFOOTER)
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
	    MenuItem(AP_MENU_ID_VIEW_PRINT)
		MenuItem(AP_MENU_ID_VIEW_RULER)
#if 0
		MenuItem(AP_MENU_ID_VIEW_STATUSBAR)
#endif
		Separator()
		MenuItem(AP_MENU_ID_VIEW_LOCKSTYLES)
#endif

        MenuItem(AP_MENU_ID_VIEW_SHOWPARA)
	    Separator()

		MenuItem(AP_MENU_ID_VIEW_FULLSCREEN)
		BeginSubMenu(AP_MENU_ID_VIEW_ZOOM_MENU)
			MenuItem(AP_MENU_ID_VIEW_ZOOM)
			MenuItem(AP_MENU_ID_VIEW_ZOOM_WIDTH)
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
#if 0
		MenuItem(AP_MENU_ID_INSERT_FIELD)
#endif
		MenuItem(AP_MENU_ID_INSERT_TEXTBOX)
#if 0
		MenuItem(AP_MENU_ID_INSERT_MAILMERGE)
#endif
		MenuItem(AP_MENU_ID_INSERT_SYMBOL)

		Separator()

		MenuItem(AP_MENU_ID_INSERT_HEADER)
		MenuItem(AP_MENU_ID_INSERT_FOOTER)
#if 0
		MenuItem(AP_MENU_ID_INSERT_FILE)
#endif
		MenuItem(AP_MENU_ID_INSERT_BOOKMARK)
		MenuItem(AP_MENU_ID_INSERT_HYPERLINK)
		MenuItem(AP_MENU_ID_INSERT_TABLEOFCONTENTS)
		MenuItem(AP_MENU_ID_INSERT_FOOTNOTE)
		MenuItem(AP_MENU_ID_INSERT_ENDNOTE)

#if 0
		MenuItem(AP_MENU_ID_INSERT_CLIPART)
#endif
		MenuItem(AP_MENU_ID_INSERT_GRAPHIC)

		Separator()

		BeginSubMenu(AP_MENU_ID_INSERT_DIRECTIONMARKER)
 	        MenuItem(AP_MENU_ID_INSERT_DIRECTIONMARKER_LRM)
	        MenuItem(AP_MENU_ID_INSERT_DIRECTIONMARKER_RLM)
	    EndSubMenu()

	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_FORMAT)
		MenuItem(AP_MENU_ID_FMT_FONT)
		MenuItem(AP_MENU_ID_FMT_PARAGRAPH)
		MenuItem(AP_MENU_ID_FMT_BULLETS)
		MenuItem(AP_MENU_ID_FMT_TABLE)
		MenuItem(AP_MENU_ID_FMT_FRAME)
    MenuItem(AP_MENU_ID_FMT_IMAGE)
// #if 1 // someone code and turn this back on
	//	Maleesh 6/10/2010 -
		MenuItem(AP_MENU_ID_FMT_BORDERS)
// #endif
		Separator()
		MenuItem(AP_MENU_ID_FMT_COLUMNS)
#if 0
		MenuItem(AP_MENU_ID_FMT_TABS)
#endif
		MenuItem(AP_MENU_ID_FMT_HDRFTR)
		MenuItem(AP_MENU_ID_FMT_FOOTNOTES)
		MenuItem(AP_MENU_ID_FMT_TABLEOFCONTENTS)
#if 0
		Separator()
		MenuItem(AP_MENU_ID_FMT_TOGGLECASE)
#endif
		Separator()

		BeginSubMenu(AP_MENU_ID_ALIGN)
			MenuItem(AP_MENU_ID_ALIGN_LEFT)
			MenuItem(AP_MENU_ID_ALIGN_CENTER)
			MenuItem(AP_MENU_ID_ALIGN_RIGHT)
			MenuItem(AP_MENU_ID_ALIGN_JUSTIFY)
		EndSubMenu()

		BeginSubMenu(AP_MENU_ID_FMT)
#if 0
			MenuItem(AP_MENU_ID_FMT_BOLD)
			MenuItem(AP_MENU_ID_FMT_ITALIC)
#endif
			MenuItem(AP_MENU_ID_FMT_UNDERLINE)
			MenuItem(AP_MENU_ID_FMT_OVERLINE)
			MenuItem(AP_MENU_ID_FMT_STRIKE)
			//MenuItem(AP_MENU_ID_FMT_TOPLINE)
			//MenuItem(AP_MENU_ID_FMT_BOTTOMLINE)
			MenuItem(AP_MENU_ID_FMT_SUPERSCRIPT)
			MenuItem(AP_MENU_ID_FMT_SUBSCRIPT)
		EndSubMenu()

		BeginSubMenu(AP_MENU_ID_FMT_BACKGROUND)
			MenuItem(AP_MENU_ID_FMT_BACKGROUND_PAGE_IMAGE)
			MenuItem(AP_MENU_ID_FMT_BACKGROUND_PAGE_COLOR)
		EndSubMenu()

    BeginSubMenu(AP_MENU_ID_FMT_DIRECTION)
      MenuItem(AP_MENU_ID_FMT_DIRECTION_DOCD_RTL)
      MenuItem(AP_MENU_ID_FMT_DIRECTION_SD_RTL)
      MenuItem(AP_MENU_ID_FMT_DIRECTION_DD_RTL)
      MenuItem(AP_MENU_ID_FMT_DIRECTION_DO_LTR)
      MenuItem(AP_MENU_ID_FMT_DIRECTION_DO_RTL)
    EndSubMenu()

		Separator()
		MenuItem(AP_MENU_ID_FMT_STYLE_DEFINE)

	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_TOOLS)

#ifdef ENABLE_SPELL
		MenuItem(AP_MENU_ID_TOOLS_SPELL)
#endif
		MenuItem(AP_MENU_ID_FMT_LANGUAGE)
		MenuItem(AP_MENU_ID_TOOLS_WORDCOUNT)
#if !XAP_SIMPLE_MENU
        MenuItem(AP_MENU_ID_FMT_STYLIST)
#endif

		Separator()

	    BeginSubMenu(AP_MENU_ID_TOOLS_HISTORY)
	        MenuItem(AP_MENU_ID_TOOLS_HISTORY_SHOW)
	        MenuItem(AP_MENU_ID_TOOLS_REVISIONS_COMPARE_DOCUMENTS)
		    MenuItem(AP_MENU_ID_TOOLS_REVISIONS_AUTO)
#if !XAP_SIMPLE_MENU
		    MenuItem(AP_MENU_ID_TOOLS_HISTORY_PURGE)
#endif
		EndSubMenu()

		BeginSubMenu(AP_MENU_ID_TOOLS_REVISIONS)
			MenuItem(AP_MENU_ID_TOOLS_REVISIONS_MARK)
#if !XAP_SIMPLE_MENU
			MenuItem(AP_MENU_ID_TOOLS_REVISIONS_NEW_REVISION)
#endif
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
#if !XAP_SIMPLE_MENU
			MenuItem(AP_MENU_ID_TOOLS_REVISIONS_PURGE)
#endif
		EndSubMenu()

		// RIVERA
#ifdef DEBUG
		BeginSubMenu(AP_MENU_ID_TOOLS_ANNOTATIONS)
			MenuItem(AP_MENU_ID_TOOLS_ANNOTATIONS_INSERT)
			MenuItem(AP_MENU_ID_TOOLS_ANNOTATIONS_INSERT_FROMSEL)
			Separator()
			MenuItem(AP_MENU_ID_TOOLS_ANNOTATIONS_TOGGLE_DISPLAY)
		EndSubMenu()
#endif

	    Separator()

#if !XAP_SIMPLE_MENU
		MenuItem(AP_MENU_ID_TOOLS_PLUGINS)
		MenuItem(AP_MENU_ID_TOOLS_SCRIPTS)
#if 0
		MenuItem(AP_MENU_ID_TOOLS_MAILMERGE)
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
// Not for 2.4
			MenuItem(AP_MENU_ID_TABLE_INSERT_CELLS)
#endif
		EndSubMenu()

		BeginSubMenu(AP_MENU_ID_TABLE_DELETE)
			MenuItem(AP_MENU_ID_TABLE_DELETE_TABLE)
			MenuItem(AP_MENU_ID_TABLE_DELETE_COLUMNS)
			MenuItem(AP_MENU_ID_TABLE_DELETE_ROWS)
#if 0
// Not for 2.4
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
// Not for 2.4
		MenuItem(AP_MENU_ID_TABLE_SPLIT_TABLE)
#endif
		MenuItem(AP_MENU_ID_TABLE_FORMAT)
                BeginSubMenu(AP_MENU_ID_TABLE_TEXTTOTABLE)
		   MenuItem(AP_MENU_ID_TABLE_TEXTTOTABLE_TABS)
		   MenuItem(AP_MENU_ID_TABLE_TEXTTOTABLE_COMMAS)
		   MenuItem(AP_MENU_ID_TABLE_TEXTTOTABLE_SPACES)
		   MenuItem(AP_MENU_ID_TABLE_TEXTTOTABLE_ALL)
                EndSubMenu()
#if DEBUG
	    BeginSubMenu(AP_MENU_ID_TABLE_SORT)
	       MenuItem(AP_MENU_ID_TABLE_SORTROWSASCEND)
	       MenuItem(AP_MENU_ID_TABLE_SORTROWSDESCEND)
	       MenuItem(AP_MENU_ID_TABLE_SORTCOLSASCEND)
	       MenuItem(AP_MENU_ID_TABLE_SORTCOLSDESCEND)
	    EndSubMenu()
#endif
#if !XAP_SIMPLE_MENU
	    BeginSubMenu(AP_MENU_ID_TABLE_TABLETOTEXT)
	       MenuItem(AP_MENU_ID_TABLE_TABLETOTEXTCOMMAS)
	       MenuItem(AP_MENU_ID_TABLE_TABLETOTEXTTABS)
	       MenuItem(AP_MENU_ID_TABLE_TABLETOTEXTCOMMASTABS)
	    EndSubMenu()
#endif
		MenuItem(AP_MENU_ID_TABLE_AUTOFIT)
#if DEBUG
	    BeginSubMenu(AP_MENU_ID_TABLE_HEADING_ROWS_REPEAT)
                MenuItem(AP_MENU_ID_TABLE_HEADING_ROWS_REPEAT_THIS)
                MenuItem(AP_MENU_ID_TABLE_HEADING_ROWS_REPEAT_REMOVE)
	    EndSubMenu()
#endif
	EndSubMenu()

#if !XAP_SIMPLE_MENU
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
#if 0
		MenuItem(AP_MENU_ID_HELP_CONTENTS)
		MenuItem(AP_MENU_ID_HELP_INTRO)
		MenuItem(AP_MENU_ID_HELP_SEARCH)
		MenuItem(AP_MENU_ID_HELP_CHECKVER)
		MenuItem(AP_MENU_ID_HELP_REPORT_BUG)
		Separator()
#ifdef TOOLKIT_GTK_ALL
		MenuItem(AP_MENU_ID_HELP_ABOUT_GNOMEOFFICE)
#endif
#ifndef TOOLKIT_GTK_ALL
		MenuItem(AP_MENU_ID_HELP_CREDITS)
#endif
#endif
		MenuItem(AP_MENU_ID_HELP_ABOUT)
	EndSubMenu()
#endif

EndLayout()

#endif
