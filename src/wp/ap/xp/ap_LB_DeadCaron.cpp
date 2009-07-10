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


// ********************************************************************************
// ********************************************************************************
// *** THIS FILE DEFINES THE BINDINGS TO HANG OFF THE DeadCaron PREFIX KEY IN   ***
// *** THE DEFAULT BINDINGS TABLE.                                              ***
// ********************************************************************************
// ********************************************************************************

#include "ut_assert.h"
#include "ut_types.h"
#include "ev_EditBits.h"
#include "ev_EditBinding.h"
#include "ev_EditMethod.h"
#include "ev_NamedVirtualKey.h"
#include "ap_LoadBindings.h"
#include "ap_LB_DeadCaron.h"

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
//	{char, /* desc   */ { none,						_C,		_A,		_A_C	}},

	// Latin-2 characters
	{0x4c, /* L      */ { "insertCaronData",		"",		"",		""		}},
	{0x53, /* S      */ { "insertCaronData",		"",		"",		""		}},
	{0x54, /* T      */ { "insertCaronData",		"",		"",		""		}},
	{0x5a, /* Z      */ { "insertCaronData",		"",		"",		""		}},
	{0x43, /* C      */ { "insertCaronData",		"",		"",		""		}},
	{0x45, /* E      */ { "insertCaronData",		"",		"",		""		}},
	{0x44, /* D      */ { "insertCaronData",		"",		"",		""		}},
	{0x4e, /* N      */ { "insertCaronData",		"",		"",		""		}},
	{0x52, /* R      */ { "insertCaronData",		"",		"",		""		}},

	{0x6c, /* l      */ { "insertCaronData",		"",		"",		""		}},
	{0x73, /* s      */ { "insertCaronData",		"",		"",		""		}},
	{0x74, /* t      */ { "insertCaronData",		"",		"",		""		}},
	{0x7a, /* z      */ { "insertCaronData",		"",		"",		""		}},
	{0x63, /* c      */ { "insertCaronData",		"",		"",		""		}},
	{0x65, /* e      */ { "insertCaronData",		"",		"",		""		}},
	{0x64, /* d      */ { "insertCaronData",		"",		"",		""		}},
	{0x6e, /* n      */ { "insertCaronData",		"",		"",		""		}},
	{0x72, /* r      */ { "insertCaronData",		"",		"",		""		}},
};


/*****************************************************************
******************************************************************
** put it all together and load the default bindings.
******************************************************************
*****************************************************************/

bool ap_LoadBindings_DeadCaron(AP_BindingSet * pThis,
								  EV_EditBindingMap * pebm)
{
	pThis->_loadChar(pebm,s_CharTable,G_N_ELEMENTS(s_CharTable),NULL,0);
	
	return true;
}
