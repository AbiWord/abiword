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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#include "xap_Features.h"
#include "ap_Features.h"
#include "ut_types.h"
#include "ut_string.h"
#include "xap_Toolbar_Icons.h"

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

AP_Toolbar_Icons::AP_Toolbar_Icons(void)
{
}

AP_Toolbar_Icons::~AP_Toolbar_Icons(void)
{
}



bool AP_Toolbar_Icons::_findIconNameForID(const char * szID, const char ** pName)
{
	bool bIDFound = false;
	
	if (!szID || !*szID )
		return false;
	UT_uint32 m;
	UT_uint32 mLimit = NrElements(s_imTable);

	// Search the map for overloaded ID_LANG to iconname
	for (m=0; m < mLimit; m++)
	{
		if (UT_stricmp(szID, s_imTable[m].m_id ) == 0)
		{
			bIDFound = true;
			*pName = s_imTable[m].m_iconname;
			break;
		}
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
				
		for (m=0; m < mLimit; m++)
		{
			if (UT_stricmp(szBaseID, s_imTable[m].m_id ) == 0)
			{
				bIDFound = true;
				*pName = s_imTable[m].m_iconname;
				break;
			}
		}		
	}
	return bIDFound;
}



#if XAP_DONT_INLINE_XPM
#else
bool AP_Toolbar_Icons::_findIconDataByName(const char * szID,
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

	if( UT_stricmp(szName,"NoIcon") == 0)
		return false;
	
	UT_uint32 kLimit = NrElements(s_itTable);
	UT_uint32 k;

	// Search to match icon name with data
	for (k=0; k < kLimit; k++)
	{
		if (UT_stricmp(szName,s_itTable[k].m_name) == 0)
		{
			*pIconData = s_itTable[k].m_staticVariable;
			*pSizeofData = s_itTable[k].m_sizeofVariable;
			return true;
		}
	}

	// Not found - no icon available.
	return false;
}
#endif

