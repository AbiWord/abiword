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


// ************************************************************************
// ************************************************************************
// *** THIS FILE DEFINES THE BINDINGS TO HANG OFF THE d PREFIX KEY IN   ***
// *** THE viEdit BINDINGS TABLE.                                       ***
// ************************************************************************
// ************************************************************************

#include "ut_assert.h"
#include "ut_types.h"
#include "ev_EditBits.h"
#include "ev_EditBinding.h"
#include "ev_EditMethod.h"
#include "ev_NamedVirtualKey.h"
#include "ap_LoadBindings.h"
#include "ap_LB_viEdit_d.h"

#define _S		| EV_EMS_SHIFT
#define _C		| EV_EMS_CONTROL
#define _A		| EV_EMS_ALT

/*****************************************************************
******************************************************************
** load bindings for the non-nvk
** (character keys).  note that SHIFT-state is implicit in the
** character value and is not included in the table.  note that
** we do not include the ASCII control characters (\x00 -- \x1f
** and others) since we don't map keystrokes into them.
******************************************************************
*****************************************************************/

static struct ap_bs_Char s_CharTable[] =
{
//	{char, /* desc   */ { none,						_C,						_A,		   _A_C		}},
	{0x24, /* $      */ { "viCmd_d24",				"",						"",			""		}},
	{0x28, /* (      */ { "viCmd_d28",				"",						"",			""		}},
	{0x29, /* )      */ { "viCmd_d29",				"",						"",			""		}},
	{0x5b, /* [      */ { "viCmd_d5b",				"",						"",			""		}},
	{0x5d, /* ]      */ { "viCmd_d5d",				"",						"",			""		}},
	{0x5e, /* ^      */ { "viCmd_d5e",				"",						"",			""		}},
	{0x62, /* b      */ { "viCmd_db",				"",						"",			""		}},
	{0x64, /* d      */ { "viCmd_dd",				"",						"",			""		}},
	{0x77, /* w      */ { "viCmd_dw",				"",			    		"",			""		}},
};


/*****************************************************************
******************************************************************
** put it all together and load the default bindings.
******************************************************************
*****************************************************************/

UT_Bool ap_LoadBindings_viEdit_d(AP_BindingSet * pThis,
								 EV_EditBindingMap * pebm)
{
	pThis->_loadChar(pebm,s_CharTable,NrElements(s_CharTable),NULL,0);
	
	return UT_TRUE;
}
