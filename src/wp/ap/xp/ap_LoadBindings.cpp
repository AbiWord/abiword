/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


// ********************************************************************************
// ********************************************************************************
// *** This file contains the table of binding sets (compatibility modes) that  ***
// *** this application provides.                                               ***
// ********************************************************************************
// ********************************************************************************

#include "ut_assert.h"
#include "ut_types.h"
#include "ut_string.h"
#include "ap_LoadBindings.h"
#include "ap_LoadBindings_Default.h"

/****************************************************************/
/****************************************************************/

typedef UT_Bool (*ap_LoadBindings_pFn)(EV_EditMethodContainer * pemc, EV_EditBindingMap **ppebm);

struct _lb
{
	const char *				m_name;
	ap_LoadBindings_pFn			m_fn;
};

static struct _lb s_lbTable[] =
{
	{	"default",		ap_LoadBindings_Default		}
};

#define NrElements(a)	((sizeof(a)/sizeof(a[0])))

/****************************************************************/
/****************************************************************/

UT_Bool AP_LoadBindings(const char * szName, EV_EditMethodContainer * pemc,
						EV_EditBindingMap **ppebm)
{
	// return an EditBindingMap for the requested compatibility mode
	// (given in szName) on the set of edit methods defined by the
	// application (in EditMethodContainer).
	
	for (UT_uint32 k=0; k<NrElements(s_lbTable); k++)
		if (UT_stricmp(szName,s_lbTable[k].m_name)==0)
			return (s_lbTable[k].m_fn)(pemc,ppebm);

	// unknown/unsupported binding set
	UT_ASSERT(0);						// TODO replace this with a better warning...
	return UT_FALSE;
}

	
