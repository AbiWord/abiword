/* AbiSource Application Framework
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

#ifndef XAP_PREFS_SCHEMEIDS_H
#define XAP_PREFS_SCHEMEIDS_H

//////////////////////////////////////////////////////////////////////////////////////
// The following are the set of scheme-based application-independent preference keys
// and the set of default values for them.  Each item must have the XAP_PREF_KEY_ prefix
// and each value must have the XAP_PREF_DEFAULT_ prefix.  Default values *must* obey
// XML encoding rules if they contain any double quote (&quot;), ampersand (&amp;), 
// or angle bracket (&lt; and &gt;) characters.
//
// ***FOR EACH PAIR DEFINED, ADD A 'dcl(basename)' TO THE BOTTOM HALF OF THIS FILE***
//
// Note: Additional keys may be defined by the application.
//////////////////////////////////////////////////////////////////////////////////////

#define XAP_PREF_KEY_ToolbarAppearance		"ToolbarAppearance"
#define XAP_PREF_DEFAULT_ToolbarAppearance	"icon"						/* {icon,text,both} */

#define XAP_PREF_KEY_UnixFontPath			"UnixFontPath"
#define XAP_PREF_DEFAULT_UnixFontPath		"fonts"						/* if relative path, prepend "getAbiSuiteLibDir()" */

#define XAP_PREF_KEY_RemapGlyphsMasterSwitch			"RemapGlyphsMasterSwitch"
 /* completely ignore glyph remapping if false */
#define XAP_PREF_DEFAULT_RemapGlyphsMasterSwitch		"1"

#define XAP_PREF_KEY_RemapGlyphsNoMatterWhat			"RemapGlyphsNoMatterWhat"
 /* if true, do remap even if originals aren't zero-width */
#define XAP_PREF_DEFAULT_RemapGlyphsNoMatterWhat	"0"

#define XAP_PREF_KEY_RemapGlyphsDefault			    "RemapGlyphsDefault"
 /* default replacement glyph for originals not mentioned in the table */
#define XAP_PREF_DEFAULT_RemapGlyphsDefault			"&#x00B0;"
/* degree symbol */

#define XAP_PREF_KEY_RemapGlyphsTable			    "RemapGlyphsTable"
/* pairwise table of originals and replacements, arbitrarily many pairs */
#define XAP_PREF_DEFAULT_RemapGlyphsTable			"&#x2018;`&#x2019;'&#x201c;&quot;&#x201d;&quot;"
/* smart quotes */

#define XAP_PREF_KEY_SmartQuotesEnable				"SmartQuotesEnable"
/* substitute curly smart quotes on the fly */
#define XAP_PREF_DEFAULT_SmartQuotesEnable			"1"

#define XAP_PREF_KEY_UseSuffix                      "UseSuffix"
/* append suffixes to saved files */
#define XAP_PREF_DEFAULT_UseSuffix                  "1"

#else /* XAP_PREFS_SCHEMEID_H */
#ifdef dcl

dcl(ToolbarAppearance)
dcl(UnixFontPath)

dcl(RemapGlyphsMasterSwitch)
dcl(RemapGlyphsNoMatterWhat)
dcl(RemapGlyphsDefault)
dcl(RemapGlyphsTable)

dcl(SmartQuotesEnable)
dcl(UseSuffix)
#endif /* dcl */
#endif /* XAP_PREFS_SCHEMEID_H */
