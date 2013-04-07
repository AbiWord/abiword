/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

#ifndef AP_PREFS_SCHEMEIDS_H
#define AP_PREFS_SCHEMEIDS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ap_Features.h"

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

#define AP_PREF_KEY_AutoGrammarCheck		    "AutoGrammarCheck"			/* enable automatic grammar check {0,1} */
#define AP_PREF_DEFAULT_AutoGrammarCheck  		"0"

#define AP_PREF_KEY_DisplayAnnotations		    "DisplayAnnotations"			/* enable display of annotations (0,1) */
#define AP_PREF_DEFAULT_DisplayAnnotations  		"0"

#define AP_PREF_KEY_DisplayRDFAnchors		    "DisplayRDFAnchors"			/* enable display of RDF anchors (0,1) */
#define AP_PREF_DEFAULT_DisplayRDFAnchors  		"0"

#define AP_PREF_KEY_SpellCheckCaps					"SpellCheckCaps"			/* enable checking CAPITALIZED words {0,1} */
#define AP_PREF_DEFAULT_SpellCheckCaps				"1"

#define AP_PREF_KEY_SpellCheckNumbers				"SpellCheckNumbers"			/* enable spell checking numbers {0,1} */
#define AP_PREF_DEFAULT_SpellCheckNumbers			"1"

#define AP_PREF_KEY_SpellCheckInternet				"SpellCheckInternet"		/* enable spell checking internet names {0,1} */
#define AP_PREF_DEFAULT_SpellCheckInternet			"1"

#define AP_PREF_KEY_SpellAutoReplace                "SpellAutoReplace" /* automatically substitute "incorrect" words with replacements */
#define AP_PREF_DEFAULT_SpellAutoReplace            "0"

#define AP_PREF_KEY_OptionsTabNumber				"OptionsTabNumber"			/* the page number of the currently shown page in the */
#define AP_PREF_DEFAULT_OptionsTabNumber			"0"							/* options dialog */

#define AP_PREF_KEY_RulerUnits						"RulerUnits"				/* value in UT_dimensionName() */
#define AP_PREF_DEFAULT_RulerUnits					"in"


#define AP_PREF_KEY_RulerVisible					"RulerVisible"				/* are the rulers visible? {0,1} */
#ifdef EMBEDDED_TARGET
#define AP_PREF_DEFAULT_RulerVisible				"0"
#else
#define AP_PREF_DEFAULT_RulerVisible				"1"
#endif

#	define AP_PREF_KEY_SimpleBarVisible					"SimpleBarVisible"			/* is the extra toolbar visible? {0,1} */
#	define AP_PREF_KEY_StandardBarVisible				"StandardBarVisible"		/* is the standard toolbar visible? {0,1} */
#	define AP_PREF_KEY_FormatBarVisible				"FormatBarVisible"			/* is the format toolbar visible? {0,1} */
#	define AP_PREF_KEY_ExtraBarVisible					"ExtraBarVisible"			/* is the extra toolbar visible? {0,1} */
#	define AP_PREF_KEY_TableBarVisible					"TableBarVisible"			/* is the table toolbar visible? {0,1} */
#	define AP_PREF_DEFAULT_SimpleBarVisible				"0"
#if XAP_SIMPLE_TOOLBAR
#	define AP_PREF_DEFAULT_StandardBarVisible			"0"
#	define AP_PREF_DEFAULT_FormatBarVisible			"0"
#else
#	define AP_PREF_DEFAULT_StandardBarVisible			"1"
#	define AP_PREF_DEFAULT_FormatBarVisible			"1"
#endif
#	define AP_PREF_DEFAULT_ExtraBarVisible				"0"
#	define AP_PREF_DEFAULT_TableBarVisible				"0"


#define AP_PREF_KEY_StatusBarVisible				"StatusBarVisible"			/* is the status bar visible? {0,1} */
#ifdef EMBEDDED_TARGET
#define AP_PREF_DEFAULT_StatusBarVisible			"0"
#else
#define AP_PREF_DEFAULT_StatusBarVisible			"1"
#endif


#define AP_PREF_KEY_ParaVisible                     "ParaVisible"               /* are the paragraphs/spaces/tats/etc. visible? {0,1} */
#define AP_PREF_DEFAULT_ParaVisible                 "0"


#define AP_PREF_KEY_StringSet						"StringSet"					/* name of dialog/msgbox strings */
#define AP_PREF_DEFAULT_StringSet					"en-US"

#define AP_PREF_KEY_KeyBindings						"KeyBindings"
#define AP_PREF_DEFAULT_KeyBindings					"default"					/* value in ap_LoadBindings.cpp */

#define AP_PREF_KEY_KeyBindingsCycle				"KeyBindingsCycle"			/* does F12 cycle keybindings? {0,1} */
#define AP_PREF_DEFAULT_KeyBindingsCycle			"0"

#define AP_PREF_KEY_InsertMode						"InsertMode"
#define AP_PREF_DEFAULT_InsertMode					"1"							/* overwrite or insert? {0,1} */

#define AP_PREF_KEY_InsertModeToggle				"InsertModeToggle"			/* does INS key toggle modes? {0,1} */
#define AP_PREF_DEFAULT_InsertModeToggle			"0"

#define AP_PREF_KEY_MenuLayout						"MenuLayouts"
#define AP_PREF_DEFAULT_MenuLayout					"Main"						/* value in BeginLayout() */


#define AP_PREF_KEY_ToolbarLayouts					"ToolbarLayouts"
#if defined (EMBEDDED_TARGET) && EMBEDDED_TARGET != EMBEDDED_TARGET_HILDON
#	define AP_PREF_DEFAULT_ToolbarLayouts				"Embedded"		/* values in BeginLayout() */
#elif XAP_SIMPLE_TOOLBAR
#	define AP_PREF_DEFAULT_ToolbarLayouts				"SimpleOps"		/* values in BeginLayout() */
#else
#	define AP_PREF_DEFAULT_ToolbarLayouts				"FileEditOps FormatOps TableOps ExtraOps"		/* values in BeginLayout() */
#endif

#define AP_PREF_KEY_SpellDirectory					"SpellCheckDirectory"		/* where we find hash files */
#define AP_PREF_DEFAULT_SpellDirectory				"dictionary" 				/* if relative, use prefix "getAbiSuiteLibDir()" */

#define AP_PREF_KEY_StringSetDirectory				"StringSetDirectory"		/* where we find StringSets */
#define AP_PREF_DEFAULT_StringSetDirectory			"strings"					/* if relative, use prefix "getAbiSuiteAppDir()" */

#define AP_PREF_KEY_LayoutMode                     "layoutMode"
#ifdef EMBEDDED_TARGET
#define AP_PREF_DEFAULT_LayoutMode                 "2" /* 1=print, 2=normal, 3=web */
#else
#define AP_PREF_DEFAULT_LayoutMode                 "1" /* 1=print, 2=normal, 3=web */
#endif

#define AP_PREF_KEY_AlwaysPromptEncoding			"AlwaysPromptEncoding"		/* if true, always show encoding dialog when importing/exporting text */
#define AP_PREF_DEFAULT_AlwaysPromptEncoding		"0"

#define AP_PREF_KEY_DefaultDirectionRtl             "DefaultDirectionRtl"       /* the deafault direction of text is rtl */
#ifndef BIDI_RTL_DOMINANT
#define AP_PREF_DEFAULT_DefaultDirectionRtl         "0"
#else
#define AP_PREF_DEFAULT_DefaultDirectionRtl         "1"
#endif

#define AP_PREF_KEY_DefaultSaveFormat "DefaultSaveFormat"
#define AP_PREF_DEFAULT_DefaultSaveFormat ".abw"
#define AP_PREF_KEY_CloseOnLastDoc "CloseOnLastDoc"
#define AP_PREF_DEFAULT_CloseOnLastDoc "0"

#define AP_PREF_KEY_LockStyles "LockStyles"
#define AP_PREF_DEFAULT_LockStyles "0"

#define AP_PREF_KEY_ColorForAnnotation1				"ColorAnnotation1"
#define AP_PREF_DEFAULT_ColorForAnnotation1			"ab04fe"
#define AP_PREF_KEY_ColorForAnnotation2				"ColorAnnotation2"
#define AP_PREF_DEFAULT_ColorForAnnotation2			"ab1477"
#define AP_PREF_KEY_ColorForAnnotation3				"ColorAnnotation3"
#define AP_PREF_DEFAULT_ColorForAnnotation3			"ff9708"
#define AP_PREF_KEY_ColorForAnnotation4				"ColorAnnotation4"
#define AP_PREF_DEFAULT_ColorForAnnotation4			"9eb345"
#define AP_PREF_KEY_ColorForAnnotation5				"ColorAnnotation5"
#define AP_PREF_DEFAULT_ColorForAnnotation5			"0fb305"
#define AP_PREF_KEY_ColorForAnnotation6				"ColorAnnotation6"
#define AP_PREF_DEFAULT_ColorForAnnotation6			"08b3f8"
#define AP_PREF_KEY_ColorForAnnotation7				"ColorAnnotation7"
#define AP_PREF_DEFAULT_ColorForAnnotation7			"04cec3"
#define AP_PREF_KEY_ColorForAnnotation8				"ColorAnnotation8"
#define AP_PREF_DEFAULT_ColorForAnnotation8			"0485c3"
#define AP_PREF_KEY_ColorForAnnotation9				"ColorAnnotation9"
#define AP_PREF_DEFAULT_ColorForAnnotation9			"0712c3"
#define AP_PREF_KEY_ColorForAnnotation10				"ColorAnnotation10"
#define AP_PREF_DEFAULT_ColorForAnnotation10			"ff0000"

#define AP_PREF_KEY_ColorForRDFAnchor1				"ColorRDFAnchor1"
#define AP_PREF_DEFAULT_ColorForRDFAnchor1			"ab04fe"
#define AP_PREF_KEY_ColorForRDFAnchor2				"ColorRDFAnchor2"
#define AP_PREF_DEFAULT_ColorForRDFAnchor2			"ab1477"
#define AP_PREF_KEY_ColorForRDFAnchor3				"ColorRDFAnchor3"
#define AP_PREF_DEFAULT_ColorForRDFAnchor3			"ff9708"
#define AP_PREF_KEY_ColorForRDFAnchor4				"ColorRDFAnchor4"
#define AP_PREF_DEFAULT_ColorForRDFAnchor4			"9eb345"
#define AP_PREF_KEY_ColorForRDFAnchor5				"ColorRDFAnchor5"
#define AP_PREF_DEFAULT_ColorForRDFAnchor5			"0fb305"
#define AP_PREF_KEY_ColorForRDFAnchor6				"ColorRDFAnchor6"
#define AP_PREF_DEFAULT_ColorForRDFAnchor6			"08b3f8"
#define AP_PREF_KEY_ColorForRDFAnchor7				"ColorRDFAnchor7"
#define AP_PREF_DEFAULT_ColorForRDFAnchor7			"04cec3"
#define AP_PREF_KEY_ColorForRDFAnchor8				"ColorRDFAnchor8"
#define AP_PREF_DEFAULT_ColorForRDFAnchor8			"0485c3"
#define AP_PREF_KEY_ColorForRDFAnchor9				"ColorRDFAnchor9"
#define AP_PREF_DEFAULT_ColorForRDFAnchor9			"0712c3"
#define AP_PREF_KEY_ColorForRDFAnchor10				"ColorRDFAnchor10"
#define AP_PREF_DEFAULT_ColorForRDFAnchor10			"ff0000"

#else /* AP_PREFS_SCHEMEIDS_H */
#ifdef dcl

dcl(CloseOnLastDoc)
dcl(DefaultSaveFormat)
dcl(CursorBlink)
dcl(AutoSpellCheck)
dcl(AutoGrammarCheck)
dcl(SpellCheckCaps)
dcl(SpellCheckNumbers)
dcl(SpellCheckInternet)
dcl(SpellAutoReplace)
dcl(OptionsTabNumber)
dcl(RulerUnits)
dcl(RulerVisible)
dcl(SimpleBarVisible)
dcl(StandardBarVisible)
dcl(FormatBarVisible)
dcl(ExtraBarVisible)
dcl(TableBarVisible)
dcl(StatusBarVisible)
dcl(ParaVisible)
dcl(StringSet)
dcl(KeyBindings)
dcl(KeyBindingsCycle)
dcl(InsertMode)
dcl(InsertModeToggle)
dcl(MenuLayout)
dcl(ToolbarLayouts)
dcl(LayoutMode)
dcl(AlwaysPromptEncoding)

dcl(SpellDirectory)
dcl(StringSetDirectory)

dcl(DefaultDirectionRtl)
dcl(LockStyles)

#endif /* dcl */
#endif /* AP_PREFS_SCHEMEIDS_H */
