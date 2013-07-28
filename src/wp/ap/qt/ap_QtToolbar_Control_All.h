/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2012 Hubert Figuiere
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

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.  Each time you add an entry to the top-half
** of this file be sure to add a corresponding entry to the other
** half and be sure to add an entry to each of the other platforms.
******************************************************************
*****************************************************************/

#ifndef AP_QTTOOLBAR_CONTROL_ALL_H

#define AP_QTTOOLBAR_CONTROL_ALL_H

#include "ap_QtToolbar_StyleCombo.h"
#include "ap_QtToolbar_FontCombo.h"
#include "ap_QtToolbar_SizeCombo.h"
#include "ap_QtToolbar_ZoomCombo.h"
	// ... add new controls here ...

#else

Declare_Control(AP_TOOLBAR_ID_FMT_STYLE, AP_QtToolbar_StyleCombo)
Declare_Control(AP_TOOLBAR_ID_FMT_FONT,  AP_QtToolbar_FontCombo)
Declare_Control(AP_TOOLBAR_ID_FMT_SIZE,  AP_QtToolbar_SizeCombo)
Declare_Control(AP_TOOLBAR_ID_ZOOM,      AP_QtToolbar_ZoomCombo)
// ... also add new controls here ...

#endif
