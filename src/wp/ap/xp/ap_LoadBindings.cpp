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
// *** This file contains the table of binding sets (compatibility modes) that  ***
// *** this application provides.                                               ***
// ********************************************************************************
// ********************************************************************************

#include "ut_assert.h"
#include "ut_types.h"
#include "ut_string.h"
#include "xap_LoadBindings.h"
#include "ap_LoadBindings_Default.h"
#include "ap_LoadBindings_DeadAbovedot.h"
#include "ap_LoadBindings_DeadAcute.h"
#include "ap_LoadBindings_DeadBreve.h"
#include "ap_LoadBindings_DeadCaron.h"
#include "ap_LoadBindings_DeadCedilla.h"
#include "ap_LoadBindings_DeadCircumflex.h"
#include "ap_LoadBindings_DeadDiaeresis.h"
#include "ap_LoadBindings_DeadDoubleacute.h"
#include "ap_LoadBindings_DeadGrave.h"
#include "ap_LoadBindings_DeadMacron.h"
#include "ap_LoadBindings_DeadOgonek.h"
#include "ap_LoadBindings_DeadTilde.h"

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
	{	"default",			ap_LoadBindings_Default			}, // stock AbiWord bindings
	
	{	"deadabovedot",		ap_LoadBindings_DeadAbovedot	}, // subordinate maps for 'dead'
	{	"deadacute",		ap_LoadBindings_DeadAcute		}, // key prefixes.
	{	"deadbreve",		ap_LoadBindings_DeadBreve		},
	{	"deadcaron",		ap_LoadBindings_DeadCaron		},
	{	"deadcedilla",		ap_LoadBindings_DeadCedilla		},
	{	"deadcircumflex",	ap_LoadBindings_DeadCircumflex	},
	{	"deaddiaeresis",	ap_LoadBindings_DeadDiaeresis	},
	{	"deaddoubleacute",	ap_LoadBindings_DeadDoubleacute	},
	{	"deadgrave",		ap_LoadBindings_DeadGrave		},
	{	"deadmacron",		ap_LoadBindings_DeadMacron		},
	{	"deadogonek",		ap_LoadBindings_DeadOgonek		},
	{	"deadtilde",		ap_LoadBindings_DeadTilde		},
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

	
