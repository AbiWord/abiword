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


#ifndef AP_TOOLBAR_FUNCTIONS_H
#define AP_TOOLBAR_FUNCTIONS_H

/*****************************************************************
******************************************************************
** This file defines the EV_GetToolbarItemState functions used by
** the set of toolbar actions.
******************************************************************
*****************************************************************/

#include "ev_Toolbar_Actions.h"

Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Changes);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Selection);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Clipboard);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_CharFmt);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_BlockFmt);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_SectionFmt);
Defun_EV_GetToolbarItemState_Fn(ap_ToolbarGetState_Zoom);

#endif /* AP_TOOLBAR_FUNCTIONS_H */
