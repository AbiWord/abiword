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

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

#ifndef AP_PREFS_SCHEMEIDS_H
#define AP_PREFS_SCHEMEIDS_H

//////////////////////////////////////////////////////////////////////////////
// The following are the set of application-dependent preference keys and the
// set of default values for them.  Each item must have the AP_PREF_IEY_ prefix
// and each value must have the AP_PREF_DEFAULT_ prefix.
//
// ***FOR EACH PAIR DEFINED, ADD A 'dcl(basename)' TO THE BOTTOM HALF OF THIS FILE***
//
// Note: These are in addition to the application-independent ones defined in
// abi/src/xap/xp/xap_Prefs_SchemeIds.h.
//////////////////////////////////////////////////////////////////////////////

#define AP_PREF_KEY_AutoSpellCheck					"AutoSpellCheck"
#define AP_PREF_DEFAULT_AutoSpellCheck				"1"							/* {0,1} */

#define AP_PREF_KEY_RulerUnits						"RulerUnits"
#define AP_PREF_DEFAULT_RulerUnits					"in"						/* value in UT_dimensionName() */

#define AP_PREF_KEY_SpellCheckWordList				"SpellCheckWordList"
#define AP_PREF_DEFAULT_SpellCheckWordList			"american.hash"				/* name of ispell hash file */

#define AP_PREF_KEY_StringSet						"StringSet"
#define AP_PREF_DEFAULT_StringSet					"EnUS"						/* name of dialog/msgbox strings */

#define AP_PREF_KEY_UnixISpellDirectory				"UnixISpellDirectory"
#define AP_PREF_DEFAULT_UnixISpellDirectory			"/usr/lib/ispell" 			/* where we find hash files */ /* TODO DECIDE WHERE THIS SHOULD BE */

#define AP_PREF_KEY_UnixStringSetDirectory			"UnixStringSetDirectory"
#define AP_PREF_DEFAULT_UnixStringSetDirectory		"/usr/local/AbiSuite/AbiWord/lib"	/* where we find StringSets */ /* TODO DECIDE WHERE THIS SHOULD BE */

#define AP_PREF_KEY_WinISpellDirectory				"WinISpellDirectory"
#define AP_PREF_DEFAULT_WinISpellDirectory			"C:\\"						/* where we find hash files */ /* TODO DECIDE WHERE THIS SHOULD BE */

#define AP_PREF_KEY_WinStringSetDirectory			"WinStringSetDirectory"
#define AP_PREF_DEFAULT_WinStringSetDirectory		"C:\\"						/* where we find StringSets */ /* TODO DECIDE WHERE THIS SHOULD BE */

#define AP_PREF_KEY_KeyBindings						"KeyBindings"
#define AP_PREF_DEFAULT_KeyBindings					"default"					/* value in ap_LoadBindings.cpp */

#define AP_PREF_KEY_MenuLayout						"MenuLayouts"
#define AP_PREF_DEFAULT_MenuLabelSet				"EnUS"						/* value in BeginLayout() */

#define AP_PREF_KEY_MenuLabelSet					"MenuLabelSet"
#define AP_PREF_DEFAULT_MenuLayout					"Main"						/* value in BeginSet() */

#define AP_PREF_KEY_ToolbarLabelSet					"ToolbarLabelSet"
#define AP_PREF_DEFAULT_ToolbarLabelSet				"EnUS"						/* value in BeginSet() */

#define AP_PREF_KEY_ToolbarLayouts					"ToolbarLayouts"
#define AP_PREF_DEFAULT_ToolbarLayouts				"FileEditOps FormatOps"		/* values in BeginLayout() */

#else /* AP_PREFS_SCHEMEIDS_H */
#ifdef dcl

dcl(AutoSpellCheck)
dcl(RulerUnits)
dcl(SpellCheckWordList)
dcl(StringSet)
dcl(UnixISpellDirectory)
dcl(UnixStringSetDirectory)
dcl(WinISpellDirectory)
dcl(WinStringSetDirectory)
dcl(KeyBindings)
dcl(MenuLayout)
dcl(MenuLabelSet)
dcl(ToolbarLabelSet)
dcl(ToolbarLayouts)

#endif /* dcl */
#endif /* AP_PREFS_SCHEMEIDS_H */
