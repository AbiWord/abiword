/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

BeginLayout(FileEditOps, AP_STRING_ID_TB_Standard, AP_PREF_KEY_StandardBarVisible)

	ToolbarItem(AP_TOOLBAR_ID_FILE_NEW)
	ToolbarItem(AP_TOOLBAR_ID_FILE_OPEN)
	ToolbarItem(AP_TOOLBAR_ID_FILE_SAVE)
#ifdef ENABLE_PRINT
	Spacer()
#if defined(TOOLKIT_GTK_ALL) || defined (TOOLKIT_WIN)
	ToolbarItem(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW)
#endif
	ToolbarItem(AP_TOOLBAR_ID_FILE_PRINT)
#endif

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_EDIT_UNDO)
	ToolbarItem(AP_TOOLBAR_ID_EDIT_REDO)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_EDIT_CUT)
	ToolbarItem(AP_TOOLBAR_ID_EDIT_COPY)
	ToolbarItem(AP_TOOLBAR_ID_EDIT_PASTE)
	ToolbarItem(AP_TOOLBAR_ID_FMTPAINTER)

	Spacer()
#ifdef ENABLE_SPELL
	ToolbarItem(AP_TOOLBAR_ID_SPELLCHECK)
	Spacer()
#endif
	ToolbarItem(AP_TOOLBAR_ID_ZOOM)

#ifndef TOOLKIT_WIN
	ToolbarItem(AP_TOOLBAR_ID_REVISIONS_NEW)
	ToolbarItem(AP_TOOLBAR_ID_REVISIONS_SELECT)
	ToolbarItem(AP_TOOLBAR_ID_REVISIONS_SHOW_FINAL)
	ToolbarItem(AP_TOOLBAR_ID_REVISIONS_FIND_PREV)
	ToolbarItem(AP_TOOLBAR_ID_REVISIONS_FIND_NEXT)
#endif

EndLayout()
