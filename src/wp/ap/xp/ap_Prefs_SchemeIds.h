/* AbiWord
 * Copyright (C) 1998-2001 AbiSource, Inc.
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
// set of default values for them.  Each item must have the AP_PREF_KEY_ prefix
// and each value must have the XAP_PREF_DEFAULT_ prefix.  Default values *must* obey
// XML encoding rules if they contain any double quote (&quot;), ampersand (&amp;), 
// or angle bracket (&lt; and &gt;) characters.
//
// ***FOR EACH PAIR DEFINED, ADD A 'dcl(basename)' TO THE BOTTOM HALF OF THIS FILE***
//
// Note: These are in addition to the application-independent ones defined in
// abi/src/xap/xp/xap_Prefs_SchemeIds.h.
//////////////////////////////////////////////////////////////////////////////

#define AP_PREF_KEY_CursorBlink						"CursorBlink"				/* enable cursor blink {0,1} */
#define AP_PREF_DEFAULT_CursorBlink					"1"

#define AP_PREF_KEY_AutoSpellCheck					"AutoSpellCheck"			/* enable automatic spell check {0,1} */
#define AP_PREF_DEFAULT_AutoSpellCheck				"1"

#define AP_PREF_KEY_SpellCheckCaps					"SpellCheckCaps"			/* enable checking CAPITALIZED words {0,1} */
#define AP_PREF_DEFAULT_SpellCheckCaps				"1"

#define AP_PREF_KEY_SpellCheckNumbers				"SpellCheckNumbers"			/* enable spell checking numbers {0,1} */
#define AP_PREF_DEFAULT_SpellCheckNumbers			"1"

#define AP_PREF_KEY_SpellCheckInternet				"SpellCheckInternet"		/* enable spell checking internet names {0,1} */
#define AP_PREF_DEFAULT_SpellCheckInternet			"1"

#define AP_PREF_KEY_OptionsTabNumber				"OptionsTabNumber"			/* the page number of the currently shown page in the */
#define AP_PREF_DEFAULT_OptionsTabNumber			"0"							/* options dialog */

#define AP_PREF_KEY_RulerUnits						"RulerUnits"				/* value in UT_dimensionName() */
#define AP_PREF_DEFAULT_RulerUnits					"in"

#define AP_PREF_KEY_RulerVisible					"RulerVisible"				/* are the rulers visible? {0,1} */
#define AP_PREF_DEFAULT_RulerVisible				"1"

#define AP_PREF_KEY_StandardBarVisible				"StandardBarVisible"		/* is the standard toolbar visible? {0,1} */
#define AP_PREF_DEFAULT_StandardBarVisible			"1"
#define AP_PREF_KEY_FormatBarVisible				"FormatBarVisible"			/* is the format toolbar visible? {0,1} */
#define AP_PREF_DEFAULT_FormatBarVisible			"1"
#define AP_PREF_KEY_ExtraBarVisible					"ExtraBarVisible"			/* is the extra toolbar visible? {0,1} */
#define AP_PREF_DEFAULT_ExtraBarVisible				"0"

#define AP_PREF_KEY_StatusBarVisible				"StatusBarVisible"			/* is the status bar visible? {0,1} */
#define AP_PREF_DEFAULT_StatusBarVisible			"1"

#define AP_PREF_KEY_ParaVisible                     "ParaVisible"               /* are the paragraphs/spaces/tats/etc. visible? {0,1} */
#define AP_PREF_DEFAULT_ParaVisible                 "0"


#define AP_PREF_KEY_SpellCheckWordList				"SpellCheckWordList"		/* name of ispell hash file */
#define AP_PREF_DEFAULT_SpellCheckWordList			"american.hash"				

#define AP_PREF_KEY_StringSet						"StringSet"					/* name of dialog/msgbox strings */
#define AP_PREF_DEFAULT_StringSet					"ja-JP"						

#define AP_PREF_KEY_KeyBindings						"KeyBindings"
#define AP_PREF_DEFAULT_KeyBindings					"default"					/* value in ap_LoadBindings.cpp */

#define AP_PREF_KEY_KeyBindingsCycle				"KeyBindingsCycle"			/* does F12 cycle keybindings? {0,1} */
#define AP_PREF_DEFAULT_KeyBindingsCycle			"0"

#define AP_PREF_KEY_InsertMode						"InsertMode"
#define AP_PREF_DEFAULT_InsertMode					"1"							/* overwrite or insert? {0,1} */

#define AP_PREF_KEY_InsertModeToggle				"InsertModeToggle"			/* does INS key toggle modes? {0,1} */
#define AP_PREF_DEFAULT_InsertModeToggle			"1"

#define AP_PREF_KEY_MenuLayout						"MenuLayouts"
#define AP_PREF_DEFAULT_MenuLayout					"Main"						/* value in BeginLayout() */

#define AP_PREF_KEY_MenuLabelSet					"MenuLabelSet"
#define AP_PREF_DEFAULT_MenuLabelSet				"en-US"						/* value in BeginSet() */

#define AP_PREF_KEY_ToolbarLabelSet					"ToolbarLabelSet"
#define AP_PREF_DEFAULT_ToolbarLabelSet				"en-US"						/* value in BeginSet() */

#define AP_PREF_KEY_ToolbarLayouts					"ToolbarLayouts"
#define AP_PREF_DEFAULT_ToolbarLayouts				"FileEditOps FormatOps ExtraOps"		/* values in BeginLayout() */

#define AP_PREF_KEY_SpellDirectory					"SpellCheckDirectory"		/* where we find hash files */
#define AP_PREF_DEFAULT_SpellDirectory				"dictionary" 				/* if relative, use prefix "getAbiSuiteLibDir()" */

#define AP_PREF_KEY_StringSetDirectory				"StringSetDirectory"		/* where we find StringSets */
#define AP_PREF_DEFAULT_StringSetDirectory			"strings"					/* if relative, use prefix "getAbiSuiteAppDir()" */

#ifdef BIDI_ENABLED
#define AP_PREF_KEY_DefaultDirectionRtl             "DefaultDirectionRtl"       /* the deafault direction of text is rtl */
#ifndef BIDI_RTL_DOMINANT
#define AP_PREF_DEFAULT_DefaultDirectionRtl         "0"
#else
#define AP_PREF_DEFAULT_DefaultDirectionRtl         "1"
#endif
#endif /*BIDI_ENABLED*/

#else /* AP_PREFS_SCHEMEIDS_H */
#ifdef dcl

dcl(CursorBlink)
dcl(AutoSpellCheck)
dcl(SpellCheckCaps)
dcl(SpellCheckNumbers)
dcl(SpellCheckInternet)
dcl(OptionsTabNumber)
dcl(RulerUnits)
dcl(RulerVisible)
dcl(StandardBarVisible)
dcl(FormatBarVisible)
dcl(ExtraBarVisible)
dcl(StatusBarVisible)
dcl(ParaVisible)
dcl(SpellCheckWordList)
dcl(StringSet)
dcl(KeyBindings)
dcl(KeyBindingsCycle)
dcl(InsertMode)
dcl(InsertModeToggle)
dcl(MenuLayout)
dcl(MenuLabelSet)
dcl(ToolbarLabelSet)
dcl(ToolbarLayouts)

dcl(SpellDirectory)
dcl(StringSetDirectory)

#ifdef BIDI_ENABLED
dcl(DefaultDirectionRtl)
#endif

#endif /* dcl */
#endif /* AP_PREFS_SCHEMEIDS_H */
