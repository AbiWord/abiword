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

#include <string.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ap_Menu_Id.h"
#include "ap_Menu_Functions.h"
#include "ev_Menu_Actions.h"
#include "fv_View.h"


Defun_EV_GetMenuItemState_Fn(ap_GetState_Changes)
{
	UT_ASSERT(pView);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	switch(id)
	{
	case AP_MENU_ID_EDIT_UNDO:
	case AP_MENU_ID_EDIT_REDO:
		// TODO if (no relevant change history)
		// TODO     s |= EV_MIS_Gray;
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_Selection)
{
	UT_ASSERT(pView);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	switch(id)
	{
	case AP_MENU_ID_EDIT_CUT:
	case AP_MENU_ID_EDIT_COPY:
		if (pView->isSelectionEmpty())
			s = EV_MIS_Gray;
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_CharFmt)
{
	UT_ASSERT(pView);
	UT_Bool bMultiple = UT_FALSE;

	EV_Menu_ItemState s = EV_MIS_ZERO;

	const XML_Char * prop = NULL;
	const XML_Char * val  = NULL;

	switch(id)
	{
	case AP_MENU_ID_FMT_BOLD:
		prop = "font-weight";
		val  = "bold";
		break;

	case AP_MENU_ID_FMT_ITALIC:
		prop = "font-style";
		val  = "italic";
		break;

	case AP_MENU_ID_FMT_UNDERLINE:
		prop = "text-decoration";
		val  = "underline";
		bMultiple = UT_TRUE;
		break;

	case AP_MENU_ID_FMT_STRIKE:
		prop = "text-decoration";
		val  = "line-through";
		bMultiple = UT_TRUE;
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	if (prop && val)
	{
		// get current font info from pView
		const XML_Char ** props_in = NULL;
		const XML_Char * sz;

		if (!pView->getCharFormat(&props_in))
			return s;

		sz = UT_getAttribute(prop, props_in);
		if (sz)
		{
			if (bMultiple)
			{	
				// some properties have multiple values
				if (strstr(sz, val))
					s = EV_MIS_Toggled;
			}
			else
			{
				if (0 == UT_stricmp(sz, val))
					s = EV_MIS_Toggled;
			}
		}
		
		free(props_in);
	}

	return s;
}

Defun_EV_GetMenuItemState_Fn(ap_GetState_BlockFmt)
{
	UT_ASSERT(pView);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	const XML_Char * prop = "text-align";
	const XML_Char * val  = NULL;

	switch(id)
	{
	case AP_MENU_ID_ALIGN_LEFT:
		val  = "left";
		break;

	case AP_MENU_ID_ALIGN_CENTER:
		val  = "center";
		break;

	case AP_MENU_ID_ALIGN_RIGHT:
		val  = "right";
		break;

	case AP_MENU_ID_ALIGN_JUSTIFY:
		val  = "justify";
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	if (prop && val)
	{
		// get current font info from pView
		const XML_Char ** props_in = NULL;
		const XML_Char * sz;

		if (!pView->getBlockFormat(&props_in))
			return s;

		sz = UT_getAttribute(prop, props_in);
		if (sz && (0 == UT_stricmp(sz, val)))
			s = EV_MIS_Toggled;
		
		free(props_in);
	}

	return s;
}

