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

#include "ap_Prefs.h"

#define NrElements(a)		(sizeof(a) / sizeof(a[0]))
#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)
#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)

/*****************************************************************/

AP_Prefs::AP_Prefs(XAP_App * pApp)
	: XAP_Prefs(pApp)
{
}

AP_Prefs::~AP_Prefs(void)
{
}

const XML_Char * AP_Prefs::getBuiltinSchemeName(void) const
{
	return "_builtin_";
}


UT_Bool AP_Prefs::loadBuiltinPrefs(void)
{
	const XML_Char * szBuiltinSchemeName = getBuiltinSchemeName();
	
	XAP_PrefsScheme * pScheme = new XAP_PrefsScheme(szBuiltinSchemeName);
	if (!pScheme)
		return UT_FALSE;

	struct _table
	{
		XML_Char *		m_szKey;
		XML_Char *		m_szValue;
	};

	/* the set of keys defined in ap_Prefs.h must match this list. */

	struct _table _t[] =
	{
		{	XAP_PREF_KEY_KeyBindings,		XAP_PREF_DEFAULT_KeyBindings		/* value in ap_LoadBindings.cpp */	},
		{	XAP_PREF_KEY_MenuLayout,		XAP_PREF_DEFAULT_MenuLayout			/* value in BeginLayout() */		},
		{	XAP_PREF_KEY_MenuLabelSet,		XAP_PREF_DEFAULT_MenuLabelSet		/* value in BeginSet() */			},
		{	XAP_PREF_KEY_ToolbarAppearance,	XAP_PREF_DEFAULT_ToolbarAppearance	/* {icon,text,both} */				},
		{	XAP_PREF_KEY_ToolbarLabelSet,	XAP_PREF_DEFAULT_ToolbarLabelSet	/* value in BeginSet() */			},
		{	XAP_PREF_KEY_ToolbarLayouts,	XAP_PREF_DEFAULT_ToolbarLayouts		/* values in BeginLayout() */		},

		{	AP_PREF_KEY_AutoSpellCheck,			AP_PREF_DEFAULT_AutoSpellCheck			/* {0,1} */								},
		{	AP_PREF_KEY_RulerUnits,				AP_PREF_DEFAULT_RulerUnits				/* value in UT_dimensionName() */		},
		{	AP_PREF_KEY_SpellCheckWordList,		AP_PREF_DEFAULT_SpellCheckWordList		/* name of ispell hash file */			},
		{	AP_PREF_KEY_UnixISpellDirectory,	AP_PREF_DEFAULT_UnixISpellDirectory		/* where we find spell,strings,etc */	},
		{	AP_PREF_KEY_WinISpellDirectory,		AP_PREF_DEFAULT_WinISpellDirectory		/* where we find spell,strings,etc */	},
	};

	for (UT_uint32 k=0; k<NrElements(_t); k++)
		if (!pScheme->setValue(_t[k].m_szKey, _t[k].m_szValue))
			goto Failed;

	addScheme(pScheme);
	return setCurrentScheme(szBuiltinSchemeName);
	
Failed:
	DELETEP(pScheme);
	return UT_FALSE;
}
