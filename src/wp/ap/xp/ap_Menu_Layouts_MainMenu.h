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
		MenuItem(AP_MENU_ID_FILE_OPEN)
		MenuItem(AP_MENU_ID_FILE_SAVE)
		MenuItem(AP_MENU_ID_FILE_SAVEAS)
	//MenuItem(AP_MENU_ID_FILE_SAVE_TEMPLATE)
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
		Separator()
		MenuItem(AP_MENU_ID_EDIT_EDITHEADER)
		MenuItem(AP_MENU_ID_EDIT_EDITFOOTER)
		MenuItem(AP_MENU_ID_EDIT_REMOVEHEADER)
		MenuItem(AP_MENU_ID_EDIT_REMOVEFOOTER)
	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_VIEW)
		MenuItem(AP_MENU_ID_VIEW_NORMAL)
		MenuItem(AP_MENU_ID_VIEW_WEB)
		MenuItem(AP_MENU_ID_VIEW_PRINT)
        MenuItem(AP_MENU_ID_WEB_WEBPREVIEW)
		Separator()
		BeginSubMenu(AP_MENU_ID_VIEW_TOOLBARS)
			MenuItem(AP_MENU_ID_VIEW_TB_STD)
			MenuItem(AP_MENU_ID_VIEW_TB_FORMAT)
                        MenuItem(AP_MENU_ID_VIEW_TB_TABLE)
			MenuItem(AP_MENU_ID_VIEW_TB_EXTRA)
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

                Separator ()
                MenuItem(AP_MENU_ID_INSERT_FILE)
		MenuItem(AP_MENU_ID_INSERT_BOOKMARK)
		MenuItem(AP_MENU_ID_INSERT_HYPERLINK)
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
	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_FORMAT)
		MenuItem(AP_MENU_ID_FMT_FONT)
		MenuItem(AP_MENU_ID_FMT_PARAGRAPH)
		MenuItem(AP_MENU_ID_FMT_BULLETS)
     MenuItem(AP_MENU_ID_FMT_DOCUMENT)
#if 0 // someone code and turn this back on
		MenuItem(AP_MENU_ID_FMT_BORDERS)
#endif
		Separator()
		MenuItem(AP_MENU_ID_FMT_COLUMNS)
		MenuItem(AP_MENU_ID_FMT_TABS)
	    MenuItem(AP_MENU_ID_FMT_HDRFTR)
		MenuItem(AP_MENU_ID_FMT_FOOTNOTES)
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
		MenuItem(AP_MENU_ID_FMT_TOPLINE)
		MenuItem(AP_MENU_ID_FMT_BOTTOMLINE)
		MenuItem(AP_MENU_ID_FMT_SUPERSCRIPT)
		MenuItem(AP_MENU_ID_FMT_SUBSCRIPT)
     EndSubMenu()
	    MenuItem(AP_MENU_ID_FMT_BACKGROUND)
		MenuItem(AP_MENU_ID_FMT_STYLE)
	EndSubMenu()

	BeginSubMenu(AP_MENU_ID_TOOLS)
                BeginSubMenu(AP_MENU_ID_TOOLS_SPELLING)
		        MenuItem(AP_MENU_ID_TOOLS_SPELL)
                        MenuItem(AP_MENU_ID_TOOLS_AUTOSPELL)
                        MenuItem(AP_MENU_ID_TOOLS_SPELLPREFS)
                EndSubMenu()
                MenuItem(AP_MENU_ID_FMT_LANGUAGE)
		MenuItem(AP_MENU_ID_TOOLS_WORDCOUNT)

	    BeginSubMenu(AP_MENU_ID_TOOLS_REVISIONS)
 	    MenuItem(AP_MENU_ID_TOOLS_REVISIONS_MARK)
	    MenuItem(AP_MENU_ID_TOOLS_REVISIONS_ACCEPT_REVISION)
	    MenuItem(AP_MENU_ID_TOOLS_REVISIONS_REJECT_REVISION)
	    MenuItem(AP_MENU_ID_TOOLS_REVISIONS_SET_VIEW_LEVEL)
	    EndSubMenu()

	    MenuItem(AP_MENU_ID_TOOLS_PLUGINS)
	    MenuItem(AP_MENU_ID_TOOLS_SCRIPTS)
            MenuItem(AP_MENU_ID_TOOLS_MAILMERGE)
		Separator()
		MenuItem(AP_MENU_ID_TOOLS_OPTIONS)
    EndSubMenu()

	BeginSubMenu(AP_MENU_ID_TABLE)
    	BeginSubMenu(AP_MENU_ID_TABLE_INSERT)
	        MenuItem(AP_MENU_ID_TABLE_INSERT_TABLE)
	        MenuItem(AP_MENU_ID_TABLE_INSERT_COLUMNS_BEFORE)
	        MenuItem(AP_MENU_ID_TABLE_INSERT_COLUMNS_AFTER)
	        MenuItem(AP_MENU_ID_TABLE_INSERT_ROWS_BEFORE)
	        MenuItem(AP_MENU_ID_TABLE_INSERT_ROWS_AFTER)
#if 0
// Not for 2.0
		    MenuItem(AP_MENU_ID_TABLE_INSERT_CELLS)
#endif
		EndSubMenu()
   	    BeginSubMenu(AP_MENU_ID_TABLE_DELETE)
	        MenuItem(AP_MENU_ID_TABLE_DELETE_TABLE)
	        MenuItem(AP_MENU_ID_TABLE_DELETE_COLUMNS)
	        MenuItem(AP_MENU_ID_TABLE_DELETE_ROWS)
#if 0
// Not for 2.0
		    MenuItem(AP_MENU_ID_TABLE_DELETE_CELLS)
#endif
		EndSubMenu()
   	    BeginSubMenu(AP_MENU_ID_TABLE_SELECT)
	        MenuItem(AP_MENU_ID_TABLE_SELECT_TABLE)
#if 0
// Not for 2.0
	        MenuItem(AP_MENU_ID_TABLE_SELECT_COLUMN)
#endif
	        MenuItem(AP_MENU_ID_TABLE_SELECT_ROW)
		    MenuItem(AP_MENU_ID_TABLE_SELECT_CELL)
		EndSubMenu()
		Separator()
		MenuItem(AP_MENU_ID_TABLE_MERGE_CELLS)
#if 0
// Not for 2.0
		MenuItem(AP_MENU_ID_TABLE_SPLIT_CELLS)
		MenuItem(AP_MENU_ID_TABLE_SPLIT_TABLE)
#endif
		MenuItem(AP_MENU_ID_TABLE_FORMAT)
#if 0
// Not for 2.0
		MenuItem(AP_MENU_ID_TABLE_AUTOFIT)
		MenuItem(AP_MENU_ID_TABLE_HEADING_ROWS_REPEAT)
		MenuItem(AP_MENU_ID_TABLE_SORT)
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
	Separator()
#endif
		MenuItem(AP_MENU_ID_HELP_ABOUT)
EndSubMenu()

EndLayout()
