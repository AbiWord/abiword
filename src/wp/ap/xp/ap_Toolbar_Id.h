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


#ifndef AP_TOOLBAR_IDSET_H
#define AP_TOOLBAR_IDSET_H

#include "xap_Types.h"

/********************************************************************/
/********************************************************************/
/** This file defines the set of Id's used for all toolbar-related **/
/** things.  Each Id defines a conceptual unit which may be        **/
/** used on one or more toolbars or not at all.                    **/
/********************************************************************/
/********************************************************************/

/* the following Id's must start at zero and be contiguous */

enum _Ap_Toolbar_Id
{
	AP_TOOLBAR_ID__BOGUS1__ = 0,			/* must be first */

	AP_TOOLBAR_ID_FILE_NEW,
	AP_TOOLBAR_ID_FILE_OPEN,
	AP_TOOLBAR_ID_FILE_SAVE,
	AP_TOOLBAR_ID_FILE_SAVEAS,
	AP_TOOLBAR_ID_FILE_PRINT,

	AP_TOOLBAR_ID_EDIT_UNDO,
	AP_TOOLBAR_ID_EDIT_REDO,
	AP_TOOLBAR_ID_EDIT_CUT,
	AP_TOOLBAR_ID_EDIT_COPY,
	AP_TOOLBAR_ID_EDIT_PASTE,

	AP_TOOLBAR_ID_FMT_STYLE,
	AP_TOOLBAR_ID_FMT_FONT,
	AP_TOOLBAR_ID_FMT_SIZE,
	AP_TOOLBAR_ID_FMT_BOLD,
	AP_TOOLBAR_ID_FMT_ITALIC,
	AP_TOOLBAR_ID_FMT_UNDERLINE,
	AP_TOOLBAR_ID_FMT_OVERLINE,
	AP_TOOLBAR_ID_FMT_STRIKE,

	AP_TOOLBAR_ID_FMT_SUPERSCRIPT,
	AP_TOOLBAR_ID_FMT_SUBSCRIPT,

	AP_TOOLBAR_ID_ALIGN_LEFT,
	AP_TOOLBAR_ID_ALIGN_CENTER,
	AP_TOOLBAR_ID_ALIGN_RIGHT,
	AP_TOOLBAR_ID_ALIGN_JUSTIFY,

	AP_TOOLBAR_ID_PARA_0BEFORE,
	AP_TOOLBAR_ID_PARA_12BEFORE,

	AP_TOOLBAR_ID_SINGLE_SPACE,
	AP_TOOLBAR_ID_MIDDLE_SPACE,
	AP_TOOLBAR_ID_DOUBLE_SPACE,

	AP_TOOLBAR_ID_1COLUMN,
	AP_TOOLBAR_ID_2COLUMN,
	AP_TOOLBAR_ID_3COLUMN,

	AP_TOOLBAR_ID_ZOOM,
	
	/* ... add others here ... */

	AP_TOOLBAR_ID__BOGUS2__				/* must be last */

};

#endif /* AP_TOOLBAR_IDSET_H */
