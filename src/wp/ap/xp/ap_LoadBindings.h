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

#ifndef AP_LOADBINDINGS_H
#define AP_LOADBINDINGS_H
#include "xap_LoadBindings.h"
#include "ev_EditBits.h"
class EV_EditMethodContainer;
class EV_EditBindingMap;

struct ap_bs_Mouse						/* binding set table to describe mouse bindings */
{
	EV_EditBits			m_eb;			// sans emo
	const char *		m_szMethod[EV_COUNT_EMO];
};

struct ap_bs_NVK						/* binding set table to describe NVK bindings */
{
	EV_EditBits			m_eb;			// sans ems
	const char *		m_szMethod[EV_COUNT_EMS];
};

struct ap_bs_NVK_Prefix					/* binding set table to describe NVK bindings which are prefixes to other maps */
{
	EV_EditBits			m_eb;			// sans ems
	const char *		m_szMapName[EV_COUNT_EMS];
};

struct ap_bs_Char						/* binding set table to describe non-nvk bindings */
{
	EV_EditBits			m_eb;			// sans ems & shift
	const char *		m_szMethod[EV_COUNT_EMS_NoShift];
};

struct ap_bs_Char_Prefix				/* binding set table to describe non-nvk bindings which are prefixes to other maps */
{
	EV_EditBits			m_eb;			// sans ems & shift
	const char *		m_szMapName[EV_COUNT_EMS_NoShift];
};

/*****************************************************************/

class AP_BindingSet : public XAP_BindingSet
{
public:
	AP_BindingSet(EV_EditMethodContainer * pemc);
	virtual ~AP_BindingSet(void);

	virtual EV_EditBindingMap *	getMap(const char * szName);

	void _loadChar(EV_EditBindingMap * pebm,
				   ap_bs_Char * pCharTable, UT_uint32 cCharTable,
				   ap_bs_Char_Prefix * pCharPrefixTable, UT_uint32 cCharPrefixTable);
	void _loadNVK(EV_EditBindingMap * pebm,
				  ap_bs_NVK * pNVK, UT_uint32 cNVK,
				  ap_bs_NVK_Prefix * pNVKPrefix, UT_uint32 cNVKPrefix);
	void _loadMouse(EV_EditBindingMap * pebm,
					ap_bs_Mouse * pMouseTable, UT_uint32 cMouseTable);
};

#endif /* AP_LOADBINDINGS_H */

