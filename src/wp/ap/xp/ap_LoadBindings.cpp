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
#include "ev_EditBinding.h"
#include "ap_LoadBindings.h"
#include "ap_LB_Default.h"
#include "ap_LB_Emacs.h"
#include "ap_LB_EmacsCtrlX.h"
#include "ap_LB_viEdit.h"
#include "ap_LB_viEdit_colon.h"
#include "ap_LB_viEdit_c.h"
#include "ap_LB_viEdit_d.h"
#include "ap_LB_viEdit_r.h"
#include "ap_LB_viEdit_y.h"
#include "ap_LB_viInput.h"
#include "ap_LB_DeadAbovedot.h"
#include "ap_LB_DeadAcute.h"
#include "ap_LB_DeadBreve.h"
#include "ap_LB_DeadCaron.h"
#include "ap_LB_DeadCedilla.h"
#include "ap_LB_DeadCircumflex.h"
#include "ap_LB_DeadDiaeresis.h"
#include "ap_LB_DeadDoubleacute.h"
#include "ap_LB_DeadGrave.h"
#include "ap_LB_DeadMacron.h"
#include "ap_LB_DeadOgonek.h"
#include "ap_LB_DeadTilde.h"

/****************************************************************/
/****************************************************************/

typedef UT_Bool (*ap_LoadBindings_pFn)(AP_BindingSet * pThis, EV_EditBindingMap * pebm);

struct _lb
{
	UT_Bool						m_bCanCycle;	// visible to CycleInputMode
	const char *				m_name;
	ap_LoadBindings_pFn			m_fn;
	EV_EditBindingMap *			m_pebm;			// must be deleted
};

static struct _lb s_lbTable[] =
{
	{	UT_TRUE,  "default",			ap_LoadBindings_Default,			NULL	}, // stock AbiWord bindings
	{   UT_TRUE,  "emacs",				ap_LoadBindings_Emacs,              NULL    }, // emacs key bindings
	{   UT_FALSE, "emacsctrlx",			ap_LoadBindings_EmacsCtrlX,         NULL    }, // emacs ctrl-x key bindings

	{	UT_TRUE,  "viEdit",				ap_LoadBindings_viEdit,				NULL	}, // vi Edit-Mode bindings
	{	UT_FALSE, "viEdit_colon",		ap_LoadBindings_viEdit_colon,		NULL	}, // vi Edit-Mode :-prefix key bindings
	{	UT_FALSE, "viEdit_c",			ap_LoadBindings_viEdit_c,			NULL	}, // vi Edit-Mode c-prefix key bindings
	{	UT_FALSE, "viEdit_d",			ap_LoadBindings_viEdit_d,			NULL	}, // vi Edit-Mode d-prefix key bindings
	{	UT_FALSE, "viEdit_y",			ap_LoadBindings_viEdit_y,			NULL	}, // vi Edit-Mode y-prefix key bindings
	{	UT_FALSE, "viEdit_r",			ap_LoadBindings_viEdit_r,			NULL	}, // vi Edit-Mode r-prefix key bindings
	{	UT_FALSE, "viInput",			ap_LoadBindings_viInput,			NULL	}, // vi Input-Mode bindings

	{	UT_FALSE, "deadabovedot",		ap_LoadBindings_DeadAbovedot,		NULL	}, // subordinate maps for 'dead'
	{	UT_FALSE, "deadacute",			ap_LoadBindings_DeadAcute,			NULL	}, // key prefixes.
	{	UT_FALSE, "deadbreve",			ap_LoadBindings_DeadBreve,			NULL	},
	{	UT_FALSE, "deadcaron",			ap_LoadBindings_DeadCaron,			NULL	},
	{	UT_FALSE, "deadcedilla",		ap_LoadBindings_DeadCedilla,		NULL	},
	{	UT_FALSE, "deadcircumflex",		ap_LoadBindings_DeadCircumflex,		NULL	},
	{	UT_FALSE, "deaddiaeresis",		ap_LoadBindings_DeadDiaeresis,		NULL	},
	{	UT_FALSE, "deaddoubleacute",	ap_LoadBindings_DeadDoubleacute,	NULL	},
	{	UT_FALSE, "deadgrave",			ap_LoadBindings_DeadGrave,			NULL	},
	{	UT_FALSE, "deadmacron",			ap_LoadBindings_DeadMacron,			NULL	},
	{	UT_FALSE, "deadogonek",			ap_LoadBindings_DeadOgonek,			NULL	},
	{	UT_FALSE, "deadtilde",			ap_LoadBindings_DeadTilde,			NULL	},
};

/****************************************************************/
/****************************************************************/

const char * AP_BindingSet::s_getNextInCycle(const char * szCurrent)
{
	// find the next one in the table with CanCycle set from the given
	// one.  wrap around if necessary.

	int kMatch = -1;
	int k;
	
	for (k=0; k<(int)NrElements(s_lbTable); k++)
		if (UT_stricmp(s_lbTable[k].m_name,szCurrent) == 0)
		{
			kMatch = k;
			break;
		}

	if (kMatch == -1)
		return NULL;

	for (k=kMatch+1; k<(int)NrElements(s_lbTable); k++)
		if (s_lbTable[k].m_bCanCycle)
			return s_lbTable[k].m_name;
	for (k=0; k<kMatch; k++)
		if (s_lbTable[k].m_bCanCycle)
			return s_lbTable[k].m_name;

	return NULL;
}

/****************************************************************/
/****************************************************************/

AP_BindingSet::AP_BindingSet(EV_EditMethodContainer * pemc)
	: XAP_BindingSet(pemc)
{
}

AP_BindingSet::~AP_BindingSet(void)
{
	for (UT_uint32 k=0; k<NrElements(s_lbTable); k++)
		if (s_lbTable[k].m_pebm)
			delete s_lbTable[k].m_pebm;
}

EV_EditBindingMap * AP_BindingSet::getMap(const char * szName)
{
	// return an EditBindingMap for the requested compatibility mode
	// (given in szName) on the set of edit methods defined by the
	// application (in EditMethodContainer).
	//
	// NOTE: the returned map should be treated as 'const' since
	// NOTE: it is shared by all windows.
	
	for (UT_uint32 k=0; k<NrElements(s_lbTable); k++)
		if (UT_stricmp(szName,s_lbTable[k].m_name)==0)
		{
			// we now share maps.  any given map is loaded exactly once.
			// if we haven't loaded this map before, load it and remember
			// it in our table.
			
			if (!s_lbTable[k].m_pebm)
			{
				s_lbTable[k].m_pebm = new EV_EditBindingMap(m_pemc);
				if (!s_lbTable[k].m_pebm)
					return NULL;
				(s_lbTable[k].m_fn)(this,s_lbTable[k].m_pebm);
			}
			return s_lbTable[k].m_pebm;
		}
	
	return NULL;
}

/*****************************************************************/

void AP_BindingSet::_loadMouse(EV_EditBindingMap* pebm,
							   const ap_bs_Mouse* pMouseTable,
							   UT_uint32 cMouseTable)
{
	UT_uint32 k, m;

	for (k=0; k<cMouseTable; k++)
		for (m=0; m<EV_COUNT_EMO; m++)
			if (pMouseTable[k].m_szMethod[m] && *pMouseTable[k].m_szMethod[m])
			{
				EV_EditMouseOp emo = EV_EMO_FromNumber(m+1);
				pebm->setBinding(pMouseTable[k].m_eb|emo,pMouseTable[k].m_szMethod[m]);
			}
}
 
void AP_BindingSet::_loadNVK(	EV_EditBindingMap*		pebm,
								const ap_bs_NVK*		pNVK,
								UT_uint32				cNVK,
								const ap_bs_NVK_Prefix*	pNVKPrefix,
								UT_uint32				cNVKPrefix)
{
	UT_uint32 k, m;

	// load terminal keys

	for (k=0; k<cNVK; k++)
		for (m=0; m<EV_COUNT_EMS; m++)
			if (pNVK[k].m_szMethod[m] && *pNVK[k].m_szMethod[m])
			{
				EV_EditModifierState ems = EV_EMS_FromNumber(m);
				pebm->setBinding(EV_EKP_PRESS|pNVK[k].m_eb|ems,pNVK[k].m_szMethod[m]);
			}

	// load prefix keys
	
	for (k=0; k<cNVKPrefix; k++)
		for (m=0; m<EV_COUNT_EMS; m++)
			if (pNVKPrefix[k].m_szMapName[m] && *pNVKPrefix[k].m_szMapName[m])
			{
				EV_EditModifierState ems = EV_EMS_FromNumber(m);
				EV_EditBindingMap * pebmSub = getMap(pNVKPrefix[k].m_szMapName[m]);
				if (pebmSub)
				{
					EV_EditBinding * pebSub = new EV_EditBinding(pebmSub);
					if (pebSub)
						pebm->setBinding(EV_EKP_PRESS|pNVKPrefix[k].m_eb|ems,pebSub);
				}
			}
}

void AP_BindingSet::_loadChar(	EV_EditBindingMap*			pebm,
								const ap_bs_Char*			pCharTable,
								UT_uint32					cCharTable,
								const ap_bs_Char_Prefix*	pCharPrefixTable,
								UT_uint32					cCharPrefixTable)
{
	UT_uint32 k, m;

	// load terminal keys

	for (k=0; k<cCharTable; k++)
		for (m=0; m<EV_COUNT_EMS_NoShift; m++)
			if (pCharTable[k].m_szMethod[m] && *pCharTable[k].m_szMethod[m])
			{
				EV_EditModifierState ems = EV_EMS_FromNumberNoShift(m);
				pebm->setBinding(EV_EKP_PRESS|pCharTable[k].m_eb|ems,pCharTable[k].m_szMethod[m]);
			}

	// load prefix keys
	
	for (k=0; k<cCharPrefixTable; k++)
		for (m=0; m<EV_COUNT_EMS_NoShift; m++)
			if (pCharPrefixTable[k].m_szMapName[m] && *pCharPrefixTable[k].m_szMapName[m])
			{
				EV_EditModifierState ems = EV_EMS_FromNumberNoShift(m);
				EV_EditBindingMap * pebmSub = getMap(pCharPrefixTable[k].m_szMapName[m]);
				if (pebmSub)
				{
					EV_EditBinding * pebSub = new EV_EditBinding(pebmSub);
					if (pebSub)
						pebm->setBinding(EV_EKP_PRESS|pCharPrefixTable[k].m_eb|ems,pebSub);
				}
			}

}
