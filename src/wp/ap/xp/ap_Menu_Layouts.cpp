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


#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ev_Menu_Layouts.h"
#include "xap_Menu_Layouts.h"
#include "ap_Menu_Id.h"

/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to load the layout for each
** menu layout in the application.  It is important that all of
** the ...Layout_*.h files allow themselves to be included more
** than one time.
******************************************************************
*****************************************************************/

struct _lt
{
	EV_Menu_LayoutFlags			m_flags;
	XAP_Menu_Id					m_id;
};

#define BeginLayout(Name,Cxt)	static struct _lt s_ltTable_##Name[] = {
#define MenuItem(id)			{ EV_MLF_Normal,		(id)				 },
#define BeginSubMenu(id)		{ EV_MLF_BeginSubMenu,	(id)				 },
#define BeginPopupMenu()		{ EV_MLF_BeginPopupMenu,AP_MENU_ID__BOGUS1__ },
#define Separator()				{ EV_MLF_Separator,		AP_MENU_ID__BOGUS1__ },
#define EndSubMenu()			{ EV_MLF_EndSubMenu,	AP_MENU_ID__BOGUS1__ },
#define EndPopupMenu()			{ EV_MLF_EndPopupMenu,	AP_MENU_ID__BOGUS1__ },
#define EndLayout()				};

#include "ap_Menu_Layouts_All.h"

#undef BeginLayout
#undef MenuItem
#undef BeginSubMenu
#undef BeginPopupMenu
#undef Separator
#undef EndSubMenu
#undef EndPopupMenu
#undef EndLayout

/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to construct a table containing
** the names and addresses of all the tables we constructed in the
** previous section.
******************************************************************
*****************************************************************/

struct _tt
{
	const char *				m_name;
	UT_uint32					m_nrEntries;
	struct _lt *				m_lt;
	EV_EditMouseContext			m_emc;
};

#define BeginLayout(Name,Cxt)	{ #Name, NrElements(s_ltTable_##Name), s_ltTable_##Name, Cxt },
#define MenuItem(id)			/*nothing*/
#define BeginSubMenu(id)		/*nothing*/
#define BeginPopupMenu()		/*nothing*/
#define Separator()				/*nothing*/
#define EndSubMenu()			/*nothing*/
#define EndPopupMenu()			/*nothing*/
#define EndLayout()				/*nothing*/

static struct _tt s_ttTable[] =
{

#include "ap_Menu_Layouts_All.h"
	
};

#undef BeginLayout
#undef MenuItem
#undef BeginSubMenu
#undef BeginPopupMenu
#undef Separator
#undef EndSubMenu
#undef EndPopupMenu
#undef EndLayout

/*****************************************************************
******************************************************************
** Put it all together and have a "load Layout by Name"
******************************************************************
*****************************************************************/

static EV_Menu_Layout * _ap_CreateMenuLayout(struct _tt * ptt)
{
	EV_Menu_Layout * pLayout = new EV_Menu_Layout(ptt->m_name,ptt->m_nrEntries);
	UT_ASSERT(pLayout);

	struct _lt * plt = ptt->m_lt;
	
	for (UT_uint32 k=0; (k < ptt->m_nrEntries); k++)
	{
		bool bResult = pLayout->setLayoutItem(k, plt[k].m_id, plt[k].m_flags);
		UT_ASSERT(bResult);
	}

	return pLayout;
}

EV_Menu_Layout * AP_CreateMenuLayout(const char * szName)
{
	UT_ASSERT(szName && *szName);		// no defaults

	for (UT_uint32 k=0; k<NrElements(s_ttTable); k++)
		if (UT_stricmp(szName,s_ttTable[k].m_name)==0)
			return _ap_CreateMenuLayout(&s_ttTable[k]);

	UT_ASSERT(0);						// no defaults
	return NULL;
}

const char * AP_FindContextMenu(EV_EditMouseContext emc)
{
	for (UT_uint32 k=0; k<NrElements(s_ttTable); k++)
		if (emc==s_ttTable[k].m_emc)
			return s_ttTable[k].m_name;

	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return NULL;
}
