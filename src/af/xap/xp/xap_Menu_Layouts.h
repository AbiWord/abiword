/* AbiSource Application Framework
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


#ifndef XAP_MENU_LAYOUTS_H
#define XAP_MENU_LAYOUTS_H

#include "ev_Menu_Layouts.h"
#include "ev_EditBits.h"

EV_Menu_Layout * AP_CreateMenuLayout(const char * szName);
const char * AP_FindContextMenu(EV_EditMouseContext emc);

#endif /* XAP_MENU_LAYOUTS_H */
