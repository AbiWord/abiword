/* AbiHello
 * Copyright (C) 1999 AbiSource, Inc.
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

#include "ut_types.h"
#include "ut_string.h"
#include "xap_Toolbar_Icons.h"

#define NrElements(a)	((sizeof(a)/sizeof(a[0])))

/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to load all of the icons.
** It is important that all of the ..._Icon_*.{h,xpm} files
** allow themselves to be included more than one time.
******************************************************************
*****************************************************************/

#include "ap_Toolbar_Icons_All.h"

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

#define DefineToolbarIcon(name)		{ #name, (const char **) ##name, sizeof(##name)/sizeof(##name[0]) },

static struct _it s_itTable[] =
{

#include "ap_Toolbar_Icons_All.h"
	
};

#undef DefineToolbarIcon

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

bool AP_Toolbar_Icons::_findIconDataByName(const char * szName,
											  const char *** pIconData,
											  UT_uint32 * pSizeofData)
{
	// This is a static function.

	if (!szName || !*szName || (UT_stricmp(szName,"NoIcon")==0))
		return false;
	
	UT_uint32 kLimit = NrElements(s_itTable);
	UT_uint32 k;

	for (k=0; k < kLimit; k++)
		if (UT_stricmp(szName,s_itTable[k].m_name) == 0)
		{
			*pIconData = s_itTable[k].m_staticVariable;
			*pSizeofData = s_itTable[k].m_sizeofVariable;
			return true;
		}

	return false;
}

