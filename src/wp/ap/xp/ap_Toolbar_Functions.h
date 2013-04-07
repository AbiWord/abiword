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


#ifndef AP_TOOLBAR_FUNCTIONS_H
#define AP_TOOLBAR_FUNCTIONS_H

/*****************************************************************
******************************************************************
** This file defines the EV_GetToolbarItemState functions used by
** the set of toolbar actions.
******************************************************************
*****************************************************************/

#include "ev_Toolbar_Actions.h"

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_ScriptsActive);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Changes);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Selection);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Clipboard);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Style);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Bullets);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Numbers);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_CharFmt);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_BlockFmt);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_SectionFmt);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Zoom);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_View);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_HdrFtr);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_StylesLocked);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Indents);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Spelling);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Table);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_TableOK);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_BookmarkOK);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_HyperlinkOK);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_TableMerged);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_AlwaysDisabled);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_HasRevisions);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_CursorInSemItem);

#endif /* AP_TOOLBAR_FUNCTIONS_H */




