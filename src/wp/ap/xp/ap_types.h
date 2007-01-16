/* AbiWord
 * Copyright (C) 2003 Tomas Frydrych
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

#ifndef AP_TYPES_H
#define AP_TYPES_H

typedef enum
{
	hori_left,
	hori_mid,
	hori_right,
	vert_above,
	vert_mid,
	vert_below
} AP_CellSplitType;


typedef enum _AP_JumpTarget
{
	AP_JUMPTARGET_PAGE, 			// beginning of page
	AP_JUMPTARGET_LINE,
	AP_JUMPTARGET_BOOKMARK,
	AP_JUMPTARGET_PICTURE // TODO
} AP_JumpTarget;


#endif /* AP_TYPES_H */
