/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copryight (C) 2003-2004 Hubert Figuiere
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xap_Features.h"
#include "ap_Features.h"
#include "ut_types.h"
#include "ut_string.h"

#include "xap_Toolbar_Icons.h"
#include "ut_assert.h"

/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to load all of the icons.
** It is important that all of the ..._Icon_*.{h,xpm} files
** allow themselves to be included more than one time.
******************************************************************
*****************************************************************/
#if XAP_DONT_INLINE_XPM
#else
# include "ap_Toolbar_Icons_All.h"
#endif
/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to construct a table of
** the icon names and pointer to the data.
******************************************************************
*****************************************************************/

struct _it
{
	const char *				m_name;
	const char **				m_staticVariable;
	UT_uint32					m_sizeofVariable;
};

#if XAP_DONT_INLINE_XPM
#else
#define DefineToolbarIcon(name)		{ #name, (const char **) name, sizeof(name)/sizeof(name[0]) },

static struct _it s_itTable[] =
{
#include "ap_Toolbar_Icons_All.h"
};

#undef DefineToolbarIcon
#endif

/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to construct a table of
** the icon IDs to iconnames.
******************************************************************
*****************************************************************/
struct _im
{
	const char *	m_id;
	const char *	m_iconname;
};

#define toolbariconmap(id,name)	{#id, #name},

static struct _im s_imTable[] =
{

#include "ap_Toolbar_Iconmap.h"

};

#undef toolbariconmap

/*****************************************************************
******************************************************************
** With the tables fully loaded, now define the class.
******************************************************************
*****************************************************************/

#ifdef DEBUG
XAP_Toolbar_Icons iconsdebug;

#if defined (TOOLKIT_WIN)
#include "xap_Win32Toolbar_Icons.h"
#endif

#endif

XAP_Toolbar_Icons::XAP_Toolbar_Icons(void)
{
#if defined(DEBUG) && !XAP_DONT_INLINE_XPM
	// Check that the lists are in alphabetically order
	UT_uint32 range = G_N_ELEMENTS(s_imTable);
	UT_sint32 cmp;
	UT_uint32 i;

	for (i = 1; i < range; i++)
	{
		cmp = g_ascii_strcasecmp( s_imTable[i].m_id, s_imTable[i-1].m_id);
		UT_ASSERT(cmp > 0);
	}	
	
	range = G_N_ELEMENTS(s_itTable);
	for (i = 1; i < range; i++)
	{
		cmp = g_ascii_strcasecmp( s_itTable[i].m_name, s_itTable[i-1].m_name);
		UT_ASSERT(cmp > 0);
	}
#endif

/*
	Define EXPORT_XPM_TO_BMP 1 and remark #define XAP_DONT_INLINE_XPM 1
	at xap_Win32Features.h when you want to convert the XPM files into BMP.
	
	* The resulting BMP files should be copied into	the src/wp/ap/win/ToolbarIcons
	* The bitmap definitions should go to ap_Win32Res_Icons.rc2
	* The bitmaps ID should go to ap_Win32Resources.rc2
	* The mapping name -> resource id structure should go to xap_Win32Toolbar_Icons.cpp
	
*/

#ifdef EXPORT_XPM_TO_BMP
	char szID[1024];
	char szIDlow[1024];

	for (i = 0; i < range; i++)	{
		strcpy (szID, s_imTable[i].m_id);
		strcpy (szIDlow, s_imTable[i].m_id);
		strlwr (szIDlow);
		
		if (XAP_Win32Toolbar_Icons::saveBitmap (s_imTable[i].m_id)) {
			UT_DEBUGMSG(("AP_RID_TI_%s BITMAP DISCARDABLE \"../../../wp/ap/win/ToolbarIcons/%s.bmp\"\n",
				szID, szIDlow));
		}		
	}
	
	UT_DEBUGMSG(("Identifiers ---\n"));
	for (i = 0; i < range; i++)
		UT_DEBUGMSG(("#define AP_RID_TI_%s %u\n",  s_imTable[i].m_id, 3000 + i));
	
	UT_DEBUGMSG(("Mapping structure ---\n"));
	for (i = 0; i < range; i++)
			UT_DEBUGMSG(("\"%s\", AP_RID_TI_%s,\n",   s_imTable[i].m_id, s_imTable[i].m_id));	

#endif

}

XAP_Toolbar_Icons::~XAP_Toolbar_Icons(void)
{
}


bool XAP_Toolbar_Icons::_findIconNameForID(const char * szID, const char ** pName)
{
	bool bIDFound = false;

	if (!szID || !*szID )
		return false;

	UT_uint32 range = G_N_ELEMENTS(s_imTable);
	UT_sint32 middle, right = range - 1, left = 0;
	UT_sint32 cmp;

	while (left <= right)
	{
		middle = (left + right) >> 1;
		cmp = g_ascii_strcasecmp(szID, s_imTable[middle].m_id);

		if (cmp == 0) {
			bIDFound = true;
			*pName = s_imTable[middle].m_iconname;
			break;
		}

		if (cmp >  0)
			left = middle + 1;
		else
			right = middle - 1;
	}

	// Search the toolbariconmap for ID to iconname
	if(!bIDFound)
	{
		//	Format: ICONNAME_LANGCODE where LANGCODE code can be _XX (_yi) or _XXXA (_caES)
		char szBaseID[300];
		strcpy(szBaseID,szID);
		char *pLast = strrchr(szBaseID, '_');

		if (pLast)
			*pLast = '\0';

		right = range - 1;
		left = 0;

		while (left <= right)
		{
			middle = (left + right) >> 1;
			cmp = g_ascii_strcasecmp(szBaseID, s_imTable[middle].m_id);

			if (cmp == 0) {
				bIDFound = true;
				*pName = s_imTable[middle].m_iconname;
				break;
			}

			if (cmp >  0)
				left = middle + 1;
			else
				right = middle - 1;
		}
	}
	return bIDFound;
}




#if XAP_DONT_INLINE_XPM
#else
bool XAP_Toolbar_Icons::_findIconDataByName(const char * szID,
											  const char *** pIconData,
											  UT_uint32 * pSizeofData)
{
	const char * szName;
	// This is a static function.
	if (!szID || !*szID )
		return false;

	bool bIDFound = _findIconNameForID(szID, &szName);

	if( !bIDFound )
		return false;

	if( g_ascii_strcasecmp(szName,"NoIcon") == 0)
		return false;

	UT_uint32 range = G_N_ELEMENTS(s_itTable);
	UT_sint32 middle, right = range - 1, left = 0;
	UT_sint32 cmp;

	while (left <= right)
	{
		middle = (left + right) >> 1;
		cmp = g_ascii_strcasecmp(szName,s_itTable[middle].m_name);

		if (cmp == 0) {
			*pIconData = s_itTable[middle].m_staticVariable;
			*pSizeofData = s_itTable[middle].m_sizeofVariable;
			return true;
		}

		if (cmp >  0)
			left = middle + 1;
		else
			right = middle - 1;
  	}

	// Not found - no icon available.
	return false;
}
#endif

