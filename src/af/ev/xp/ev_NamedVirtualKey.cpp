/* AbiSource Program Utilities
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
 



#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ev_NamedVirtualKey.h"

// NOTE: the order of the entries in the following table
// NOTE: must match the EV_NVK_ definitions in the header
// NOTE: file.

static const char * s_Table[] =
{	"",				// must be at index zero
	"backspace",
	"space",
	"tab",
	"return",
	"escape",
	"pageup",
	"pagedown",
	"end",
	"home",
	"left",
	"up",
	"right",
	"down",
	"insert",
	"delete",
	"help",
	"f1",
	"f2",
	"f3",
	"f4",
	"f5",
	"f6",
	"f7",
	"f8",
	"f9",
	"f10",
	"f11",
	"f12",
	"f13",
	"f14",
	"f15",
	"f16",
	"f17",
	"f18",
	"f19",
	"f20",
	"f21",
	"f22",
	"f23",
	"f24",
	"f25",
	"f26",
	"f27",
	"f28",
	"f29",
	"f30",
	"f31",
	"f32",
	"f33",
	"f34",
	"f35",
	"DeadGrave",
	"DeadAcute",
	"DeadCircumflex",
	"DeadTilde",
	"DeadMacron",
	"DeadBreve",
	"DeadAboveDot",
	"DeadDiaeresis",
	"DeadDoubleAcute",
	"DeadCaron",
	"DeadCedilla",
	"DeadOgonek",
	"DeadIota",
	"MenuShortCut"
	// TODO as other items are added to ev_NamedVirtualKey, add items here.
};

#define NrElements(a)	((sizeof(a))/(sizeof(a[0])))

const char * EV_NamedVirtualKey::getName(EV_EditBits eb)
{
	UT_ASSERT((UT_stricmp(s_Table[EV_NVK_F35&~EV_EKP_NAMEDKEY],"f35")==0));
	UT_ASSERT((NrElements(s_Table) == EV_COUNT_NVK));

	EV_EditVirtualKey evk = eb & ~EV_EKP_NAMEDKEY;
	if (evk < NrElements(s_Table))
		return s_Table[evk];
	return 0;
}

EV_EditBits EV_NamedVirtualKey::getEB(const char * szName)
{
	UT_ASSERT((UT_stricmp(s_Table[EV_NVK_F35&~EV_EKP_NAMEDKEY],"f35")==0));
	UT_ASSERT((NrElements(s_Table) == EV_COUNT_NVK));

	for (UT_uint32 k=1; k<NrElements(s_Table); k++)
		if (UT_stricmp(s_Table[k],szName)==0)
			return EV_NamedKey(k);
	return 0;
}
