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

#ifndef AP_LOADBINDINGS_H
#define AP_LOADBINDINGS_H
#include "xap_LoadBindings.h"
#include "ev_EditBits.h"
#include "ut_vector.h"
#include "ut_string.h"

class EV_EditMethodContainer;
class EV_EditBindingMap;
class AP_BindingSet;
//////////////////////////////////////////////////////////////////
#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
#  define FN_TEST_DUMP		"Test_Dump"
#  define FN_TEST_FTR           "Test_Ftr"
#else
#  define FN_TEST_DUMP		""
#  define FN_TEST_FTR           ""
#endif

//////////////////////////////////////////////////////////////////

// binding set table to describe mouse bindings
struct ap_bs_Mouse
{
	EV_EditBits			m_eb;			// sans emo
	const char *		m_szMethod[EV_COUNT_EMO];
};

// binding set table to describe NVK bindings
struct ap_bs_NVK
{
	EV_EditBits			m_eb;			// sans ems
	const char *		m_szMethod[EV_COUNT_EMS];
};

// binding set table to describe NVK bindings which are
// prefixes to other maps
struct ap_bs_NVK_Prefix
{
	EV_EditBits			m_eb;			// sans ems
	const char *		m_szMapName[EV_COUNT_EMS];
};

// binding set table to describe non-nvk bindings
struct ap_bs_Char
{
	EV_EditBits			m_eb;			// sans ems & shift
	const char *		m_szMethod[EV_COUNT_EMS_NoShift];
};

// binding set table to describe non-nvk bindings which are
// prefixes to other maps
struct ap_bs_Char_Prefix
{
	EV_EditBits			m_eb;			// sans ems & shift
	const char *		m_szMapName[EV_COUNT_EMS_NoShift];
};


typedef bool (*ap_LoadBindings_pFn)(AP_BindingSet * pThis, EV_EditBindingMap * pebm);

class ABI_EXPORT c_lb
{
 public:
  c_lb(c_lb * pc_lb);
  c_lb(	bool bCycle,	const char * name, ap_LoadBindings_pFn fn, EV_EditBindingMap * pebm);
  ~c_lb(void);
	bool						m_bCanCycle;	// visible to CycleInputMode
	char *					m_name;
	ap_LoadBindings_pFn			m_fn;
	EV_EditBindingMap *			m_pebm;			// must be deleted
};

/*****************************************************************/

class ABI_EXPORT AP_BindingSet : public XAP_BindingSet
{
public:
	AP_BindingSet(EV_EditMethodContainer * pemc);
	virtual ~AP_BindingSet(void);

	virtual EV_EditBindingMap *	getMap(const char * szName);
	void                            loadBuiltin(void);
	EV_EditBindingMap *            createMap(const char * szName);
	void _loadChar(	EV_EditBindingMap*			pebm,
			const ap_bs_Char*			pCharTable,
			UT_uint32				cCharTable,
			const ap_bs_Char_Prefix*	pCharPrefixTable,
			UT_uint32			cCharPrefixTable);
	void _loadNVK(	EV_EditBindingMap*			pebm,
			const ap_bs_NVK*			pNVK,
			UT_uint32				cNVK,
			const ap_bs_NVK_Prefix*		pNVKPrefix,
			UT_uint32			cNVKPrefix);
	void _loadMouse(EV_EditBindingMap*		pebm,
			const ap_bs_Mouse*		pMouseTable,
			UT_uint32			cMouseTable);

	const char * getNextInCycle(const char * szCurrent);
 private:
	UT_GenericVector<c_lb *>         m_vecBindings;
};

#endif /* AP_LOADBINDINGS_H */

