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


#ifndef AP_MENU_IDSET_H
#define AP_MENU_IDSET_H

#include "xap_Types.h"

/*****************************************************************/
/*****************************************************************/
/** This file defines the set of Id's used for all menu-related **/
/** things.  Each Id defines a conceptual unit which may be     **/
/** used on one or more menus or not at all.                    **/
/*****************************************************************/
/*****************************************************************/

/* the following Id's must start at zero and be contiguous */

enum _Ap_Menu_Id
{
	AP_MENU_ID__BOGUS1__ = 0,			/* must be first */

	AP_MENU_ID_FILE,
	AP_MENU_ID_FILE_NEW,
	AP_MENU_ID_FILE_OPEN,
	AP_MENU_ID_FILE_SAVE,
	AP_MENU_ID_FILE_SAVEAS,
	AP_MENU_ID_FILE_CLOSE,
	AP_MENU_ID_FILE_PAGESETUP,
	AP_MENU_ID_FILE_PRINT,
	AP_MENU_ID_FILE_PRINT_PREVIEW,
	AP_MENU_ID_FILE_PRINT_DIRECTLY,	
	AP_MENU_ID_FILE_RECENT_1,		// _recent_1 thru _recent_9 must be contiguous
	AP_MENU_ID_FILE_RECENT_2,
	AP_MENU_ID_FILE_RECENT_3,
	AP_MENU_ID_FILE_RECENT_4,
	AP_MENU_ID_FILE_RECENT_5,
	AP_MENU_ID_FILE_RECENT_6,
	AP_MENU_ID_FILE_RECENT_7,
	AP_MENU_ID_FILE_RECENT_8,
	AP_MENU_ID_FILE_RECENT_9,
	AP_MENU_ID_FILE_EXIT,

	AP_MENU_ID_EDIT,
	AP_MENU_ID_EDIT_UNDO,
	AP_MENU_ID_EDIT_REDO,
	AP_MENU_ID_EDIT_CUT,
	AP_MENU_ID_EDIT_COPY,
	AP_MENU_ID_EDIT_PASTE,
	AP_MENU_ID_EDIT_CLEAR,
	AP_MENU_ID_EDIT_SELECTALL,
	AP_MENU_ID_EDIT_FIND,
	AP_MENU_ID_EDIT_REPLACE,
	AP_MENU_ID_EDIT_GOTO,



	AP_MENU_ID_VIEW,
	AP_MENU_ID_VIEW_TOOLBARS,
	AP_MENU_ID_VIEW_TB_STD,
	AP_MENU_ID_VIEW_TB_FORMAT,
	AP_MENU_ID_VIEW_TB_EXTRA,
	AP_MENU_ID_VIEW_RULER,
	AP_MENU_ID_VIEW_STATUSBAR,
	AP_MENU_ID_VIEW_SHOWPARA,
	AP_MENU_ID_VIEW_HEADFOOT,
	AP_MENU_ID_VIEW_ZOOM,
	AP_MENU_ID_VIEW_FULLSCREEN,

	AP_MENU_ID_INSERT,
	AP_MENU_ID_INSERT_BREAK,
	AP_MENU_ID_INSERT_PAGENO,
	AP_MENU_ID_INSERT_DATETIME,
	AP_MENU_ID_INSERT_FIELD,
	AP_MENU_ID_INSERT_SYMBOL,
	AP_MENU_ID_INSERT_GRAPHIC,

	AP_MENU_ID_FORMAT,
	AP_MENU_ID_FMT_FONT,
	AP_MENU_ID_FMT_PARAGRAPH,
	AP_MENU_ID_FMT_BULLETS,
	AP_MENU_ID_FMT_DOCUMENT,
	AP_MENU_ID_FMT_BORDERS,
	AP_MENU_ID_FMT_COLUMNS,
	AP_MENU_ID_FMT_TOGGLECASE,
	AP_MENU_ID_FMT_STYLE,
	AP_MENU_ID_FMT_TABS,
	AP_MENU_ID_FMT_BOLD,
	AP_MENU_ID_FMT_ITALIC,
	AP_MENU_ID_FMT_UNDERLINE,
	AP_MENU_ID_FMT_OVERLINE,
	AP_MENU_ID_FMT_STRIKE,
	AP_MENU_ID_FMT_SUPERSCRIPT,
	AP_MENU_ID_FMT_SUBSCRIPT,
	AP_MENU_ID_FMT_LANGUAGE,

	AP_MENU_ID_TOOLS,
	AP_MENU_ID_TOOLS_SPELLING,
	AP_MENU_ID_TOOLS_SPELL,
	AP_MENU_ID_TOOLS_AUTOSPELL,
	AP_MENU_ID_TOOLS_WORDCOUNT,
	AP_MENU_ID_TOOLS_OPTIONS,
	AP_MENU_ID_TOOLS_LANGUAGE,

	AP_MENU_ID_ALIGN,
	AP_MENU_ID_ALIGN_LEFT,
	AP_MENU_ID_ALIGN_CENTER,
	AP_MENU_ID_ALIGN_RIGHT,
	AP_MENU_ID_ALIGN_JUSTIFY,

	AP_MENU_ID_WINDOW,
	AP_MENU_ID_WINDOW_NEW,
	AP_MENU_ID_WINDOW_1,		// _window_1 thru _window_9 must be contiguous
	AP_MENU_ID_WINDOW_2,
	AP_MENU_ID_WINDOW_3,
	AP_MENU_ID_WINDOW_4,
	AP_MENU_ID_WINDOW_5,
	AP_MENU_ID_WINDOW_6,
	AP_MENU_ID_WINDOW_7,
	AP_MENU_ID_WINDOW_8,
	AP_MENU_ID_WINDOW_9,
	AP_MENU_ID_WINDOW_MORE,

	AP_MENU_ID_HELP,
	AP_MENU_ID_HELP_CONTENTS,
	AP_MENU_ID_HELP_INDEX,
	AP_MENU_ID_HELP_CHECKVER,
	AP_MENU_ID_HELP_SEARCH,
	AP_MENU_ID_HELP_ABOUT,
	AP_MENU_ID_HELP_ABOUTOS,

	AP_MENU_ID_SPELL_SUGGEST_1,		// _suggest_1 thru _suggest_9 must be contiguous
	AP_MENU_ID_SPELL_SUGGEST_2,
	AP_MENU_ID_SPELL_SUGGEST_3,
	AP_MENU_ID_SPELL_SUGGEST_4,
	AP_MENU_ID_SPELL_SUGGEST_5,
	AP_MENU_ID_SPELL_SUGGEST_6,
	AP_MENU_ID_SPELL_SUGGEST_7,
	AP_MENU_ID_SPELL_SUGGEST_8,
	AP_MENU_ID_SPELL_SUGGEST_9,
	AP_MENU_ID_SPELL_IGNOREALL,
	AP_MENU_ID_SPELL_ADD,

	/* ... add others here ... */

	AP_MENU_ID__BOGUS2__				/* must be last */

};

#endif /* AP_MENU_IDSET_H */
