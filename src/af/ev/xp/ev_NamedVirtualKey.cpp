 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
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
	"f35"
};

#define NrElements(a)	((sizeof(a))/(sizeof(a[0])))

const char * EV_NamedVirtualKey::getName(EV_EditBits eb)
{
	UT_ASSERT((UT_stricmp(s_Table[EV_NVK_F35&~EV_EKP_NAMEDKEY],"f35")==0));
	EV_EditVirtualKey evk = eb & ~EV_EKP_NAMEDKEY;
	if (evk < NrElements(s_Table))
		return s_Table[evk];
	return 0;
}

EV_EditBits EV_NamedVirtualKey::getEB(const char * szName)
{
	UT_ASSERT((UT_stricmp(s_Table[EV_NVK_F35&~EV_EKP_NAMEDKEY],"f35")==0));
	for (UT_uint32 k=1; k<NrElements(s_Table); k++)
		if (UT_stricmp(s_Table[k],szName)==0)
			return EV_NamedKey(k);
	return 0;
}
