 
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

	
