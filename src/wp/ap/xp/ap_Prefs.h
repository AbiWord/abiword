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

#ifndef AP_PREFS_H
#define AP_PREFS_H

#include "xap_Prefs.h"

class AP_Prefs : public XAP_Prefs
{
public:
	AP_Prefs(XAP_App * pApp);
	virtual ~AP_Prefs(void);

	virtual UT_Bool				loadBuiltinPrefs(void);
	virtual const XML_Char *	getBuiltinSchemeName(void) const;
	virtual const char *		getPrefsPathname(void) const = 0;
};

//////////////////////////////////////////////////////////////////////////////
// The following are the set of application-dependent preference keys.
// These are in addition to the application-independent ones defined in
// abi/src/xap/xp/xap_Prefs.h.
//
// The list in ap_Prefs::loadBuildinPrefs() in _t[] should match these sets.

#define AP_PREF_KEY_AutoSpellCheck					"AutoSpellCheck"
#define AP_PREF_KEY_RulerUnits						"RulerUnits"
#define AP_PREF_KEY_SpellCheckWordList				"SpellCheckWordList"
#define AP_PREF_KEY_UnixISpellDirectory				"UnixISpellDirectory"
#define AP_PREF_KEY_WinISpellDirectory				"WinISpellDirectory"

#define AP_PREF_DEFAULT_AutoSpellCheck				"1"
#define AP_PREF_DEFAULT_RulerUnits					"in"
#define AP_PREF_DEFAULT_SpellCheckWordList			"american.hash"
#define AP_PREF_DEFAULT_UnixISpellDirectory			"/usr/lib/ispell" /* TODO decide where this should be */
#define AP_PREF_DEFAULT_WinISpellDirectory			"C:\\" /* TODO DECIDE WHERE THIS SHOULD BE */

#endif /* AP_PREFS_H */
