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
// and each value must have the XAP_PREF_DEFAULT_ prefix.
//
// ***FOR EACH PAIR DEFINED, ADD A 'dcl(basename)' TO THE BOTTOM HALF OF THIS FILE***
//
// Note: Additional keys may be defined by the application.
//////////////////////////////////////////////////////////////////////////////////////

#define XAP_PREF_KEY_ToolbarAppearance		"ToolbarAppearance"
#define XAP_PREF_DEFAULT_ToolbarAppearance	"icon"						/* {icon,text,both} */

#define XAP_PREF_KEY_UnixFontPath			"UnixFontPath"
// TODO until we the the installation problems solved, try several paths probably valid during debugging only.
// TODO when the installion is resolved, use the latter form of this.
//#define XAP_PREF_DEFAULT_UnixFontPath		"/usr/local/AbiSuite/lib/fonts;./src/wp/lib/unix/fonts;./wp/lib/unix/fonts;../../lib/unix/fonts"
#define XAP_PREF_DEFAULT_UnixFontPath		"/usr/local/AbiSuite/lib/fonts"

#else /* XAP_PREFS_SCHEMEID_H */
#ifdef dcl

dcl(ToolbarAppearance)
dcl(UnixFontPath)

#endif /* dcl */
#endif /* XAP_PREFS_SCHEMEID_H */
