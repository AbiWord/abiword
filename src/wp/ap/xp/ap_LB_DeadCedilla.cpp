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
// *** THIS FILE DEFINES THE BINDINGS TO HANG OFF THE DeadCedilla PREFIX KEY IN   ***
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
#include "ap_LB_DeadCedilla.h"

#define NrElements(a)	((sizeof(a)/sizeof(a[0])))

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
	{0x43, /* C      */ { "insertCedillaData",		"",		"",		""		}},
	{0x63, /* c      */ { "insertCedillaData",		"",		"",		""		}},

#if 0
	// TODO add these Latin-[24] characters when we
	// TODO fix the char widths calculations.
	{0x53, /* S      */ { "insertCedillaData",		"",		"",		""		}},
	{0x54, /* T      */ { "insertCedillaData",		"",		"",		""		}},
	{0x52, /* R      */ { "insertCedillaData",		"",		"",		""		}},
	{0x4c, /* L      */ { "insertCedillaData",		"",		"",		""		}},
	{0x47, /* G      */ { "insertCedillaData",		"",		"",		""		}},
	{0x4e, /* N      */ { "insertCedillaData",		"",		"",		""		}},
	{0x4b, /* K      */ { "insertCedillaData",		"",		"",		""		}},

	{0x73, /* s      */ { "insertCedillaData",		"",		"",		""		}},
	{0x74, /* t      */ { "insertCedillaData",		"",		"",		""		}},
	{0x72, /* r      */ { "insertCedillaData",		"",		"",		""		}},
	{0x6c, /* l      */ { "insertCedillaData",		"",		"",		""		}},
	{0x67, /* g      */ { "insertCedillaData",		"",		"",		""		}},
	{0x6e, /* n      */ { "insertCedillaData",		"",		"",		""		}},
	{0x6b, /* k      */ { "insertCedillaData",		"",		"",		""		}},
#endif
};


/*****************************************************************
******************************************************************
** put it all together and load the default bindings.
******************************************************************
*****************************************************************/

UT_Bool ap_LoadBindings_DeadCedilla(AP_BindingSet * pThis,
									EV_EditBindingMap * pebm)
{
	pThis->_loadChar(pebm,s_CharTable,NrElements(s_CharTable),NULL,0);
	
	return UT_TRUE;
}
