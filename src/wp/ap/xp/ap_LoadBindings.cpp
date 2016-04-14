/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */


// ********************************************************************************
// ********************************************************************************
// *** This file contains the table of binding sets (compatibility modes) that  ***
// *** this application provides.                                               ***
// ********************************************************************************
// ********************************************************************************

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ut_assert.h"
#include "ut_types.h"
#include "ut_string.h"
#include "ev_EditBinding.h"
#include "ap_LoadBindings.h"
#include "ap_LB_Default.h"
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

#ifdef ENABLE_EMACS_KEYBINDING
#include "ap_LB_Emacs.h"
#include "ap_LB_EmacsCtrlX.h"
#endif

#ifdef ENABLE_VI_KEYBINDING
#include "ap_LB_viEdit.h"
#include "ap_LB_viEdit_colon.h"
#include "ap_LB_viEdit_c.h"
#include "ap_LB_viEdit_d.h"
#include "ap_LB_viEdit_r.h"
#include "ap_LB_viEdit_y.h"
#include "ap_LB_viInput.h"
#endif

/****************************************************************/
/****************************************************************/
c_lb::c_lb(bool bCycle, const char * name,  ap_LoadBindings_pFn pFn,	EV_EditBindingMap * pebm  ) : 
  m_bCanCycle(bCycle),
  m_fn(pFn),
  m_pebm(pebm)
{
  m_name = g_strdup(name);
}

c_lb::c_lb(c_lb * pc_lb)
{
  m_bCanCycle = pc_lb->m_bCanCycle;
  m_name = g_strdup(pc_lb->m_name);
  m_fn = pc_lb->m_fn;
  m_pebm = pc_lb->m_pebm;
}

c_lb::~c_lb(void)
{

    g_free(m_name);
    DELETEP (m_pebm);
}


/****************************************************************/
/****************************************************************/

const char * AP_BindingSet::getNextInCycle(const char * szCurrent)
{
	// find the next one in the table with CanCycle set from the given
	// one.  wrap around if necessary.

	int kMatch = -1;
	UT_sint32 k;
	
	for (k=0; k<m_vecBindings.getItemCount(); k++)
	  if (g_ascii_strcasecmp(m_vecBindings.getNthItem(k)->m_name,szCurrent) == 0)
		{
			kMatch = k;
			break;
		}

	if (kMatch == -1)
		return NULL;

	for (k=kMatch+1; k<m_vecBindings.getItemCount(); k++)
	  if (m_vecBindings.getNthItem(k)->m_bCanCycle)
			return m_vecBindings.getNthItem(k)->m_name;
	for (k=0; k<kMatch; k++)
		if (m_vecBindings.getNthItem(k)->m_bCanCycle)
			return m_vecBindings.getNthItem(k)->m_name;

	return NULL;
}

/****************************************************************/
/****************************************************************/

AP_BindingSet::AP_BindingSet(EV_EditMethodContainer * pemc)
	: XAP_BindingSet(pemc)
{
  loadBuiltin();
  UT_DEBUGMSG(("Created binding set %p \n",this));
}

AP_BindingSet::~AP_BindingSet(void)
{
        UT_VECTOR_PURGEALL(c_lb *, m_vecBindings);
}

void AP_BindingSet::loadBuiltin(void)
{

  m_vecBindings.addItem(new c_lb(true,	"default",			ap_LoadBindings_Default,			NULL)); // stock AbiWord bindings
#ifdef ENABLE_EMACS_KEYBINDING
  m_vecBindings.addItem(new c_lb(true,  "emacs",			ap_LoadBindings_Emacs, 				NULL)); // emacs key bindings
  m_vecBindings.addItem(new c_lb(false, "emacsctrlx",		ap_LoadBindings_EmacsCtrlX,			NULL)); // emacs ctrl-x key bindings
#endif
#ifdef ENABLE_VI_KEYBINDING
  m_vecBindings.addItem(new c_lb(true,  "viEdit",			ap_LoadBindings_viEdit,				NULL)); // vi Edit-Mode bindings
  m_vecBindings.addItem(new c_lb(false, "viEdit_colon",		ap_LoadBindings_viEdit_colon,		NULL)); // vi Edit-Mode :-prefix key bindings
  m_vecBindings.addItem(new c_lb(false, "viEdit_c",			ap_LoadBindings_viEdit_c,			NULL)); // vi Edit-Mode c-prefix key bindings
  m_vecBindings.addItem(new c_lb(false, "viEdit_d",			ap_LoadBindings_viEdit_d,			NULL)); // vi Edit-Mode d-prefix key bindings
  m_vecBindings.addItem(new c_lb(false, "viEdit_y",			ap_LoadBindings_viEdit_y,			NULL)); // vi Edit-Mode y-prefix key bindings
  m_vecBindings.addItem(new c_lb(false, "viEdit_r",			ap_LoadBindings_viEdit_r,			NULL)); // vi Edit-Mode r-prefix key bindings
  m_vecBindings.addItem(new c_lb(false, "viInput",			ap_LoadBindings_viInput,			NULL)); // vi Input-Mode bindings
#endif  
  m_vecBindings.addItem(new c_lb(false, "deadabovedot",		ap_LoadBindings_DeadAbovedot,		NULL)); // subordinate maps for 'dead'
  m_vecBindings.addItem(new c_lb(false, "deadacute",		ap_LoadBindings_DeadAcute,			NULL)); // key prefixes.
  m_vecBindings.addItem(new c_lb(false, "deadbreve",		ap_LoadBindings_DeadBreve,			NULL));
  m_vecBindings.addItem(new c_lb(false, "deadcaron",		ap_LoadBindings_DeadCaron,			NULL));
  m_vecBindings.addItem(new c_lb(false, "deadcedilla",		ap_LoadBindings_DeadCedilla,		NULL));
  m_vecBindings.addItem(new c_lb(false, "deadcircumflex",	ap_LoadBindings_DeadCircumflex,		NULL));
  m_vecBindings.addItem(new c_lb(false, "deaddiaeresis",	ap_LoadBindings_DeadDiaeresis,		NULL));
  m_vecBindings.addItem(new c_lb(false, "deaddoubleacute",	ap_LoadBindings_DeadDoubleacute,	NULL));
  m_vecBindings.addItem(new c_lb(false, "deadgrave",		ap_LoadBindings_DeadGrave,			NULL));
  m_vecBindings.addItem(new c_lb(false, "deadmacron",		ap_LoadBindings_DeadMacron,			NULL));
  m_vecBindings.addItem(new c_lb(false, "deadogonek",		ap_LoadBindings_DeadOgonek,			NULL));
  m_vecBindings.addItem(new c_lb(false, "deadtilde",		ap_LoadBindings_DeadTilde,			NULL));
}

EV_EditBindingMap * AP_BindingSet::getMap(const char * szName)
{
	// return an EditBindingMap for the requested compatibility mode
	// (given in szName) on the set of edit methods defined by the
	// application (in EditMethodContainer).
	//
	// NOTE: the returned map should be treated as 'const' since
	// NOTE: it is shared by all windows.
	
  for (UT_sint32 k=0; k< m_vecBindings.getItemCount(); k++)
    if (g_ascii_strcasecmp(szName,m_vecBindings.getNthItem(k)->m_name)==0)
		{
			// we now share maps.  any given map is loaded exactly once.
			// if we haven't loaded this map before, load it and remember
			// it in our table.
			
		  if (!m_vecBindings.getNthItem(k)->m_pebm)
			{
				m_vecBindings.getNthItem(k)->m_pebm = new EV_EditBindingMap(m_pemc);
				if (!m_vecBindings.getNthItem(k)->m_pebm)
					return NULL;
				(m_vecBindings.getNthItem(k)->m_fn)(this,m_vecBindings.getNthItem(k)->m_pebm);
			}
			return m_vecBindings.getNthItem(k)->m_pebm;
		}
	
	return NULL;
}


EV_EditBindingMap * AP_BindingSet::createMap(const char * szName)
{
  c_lb * pc_lb = new c_lb(false,szName,NULL,NULL);
  m_vecBindings.addItem(pc_lb);
  pc_lb->m_pebm = new EV_EditBindingMap(m_pemc);
  return pc_lb->m_pebm;
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
				UT_uint32			cNVK,
				const ap_bs_NVK_Prefix*	pNVKPrefix,
				UT_uint32		         cNVKPrefix)
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

void AP_BindingSet::_loadChar(	EV_EditBindingMap*  pebm,
				const ap_bs_Char*   pCharTable,
				UT_uint32           cCharTable,
				const ap_bs_Char_Prefix*  pCharPrefixTable,
				UT_uint32		cCharPrefixTable)
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
