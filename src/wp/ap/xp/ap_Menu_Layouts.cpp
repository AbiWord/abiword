 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ev_Menu_Layouts.h"
#include "ap_Menu_Layouts.h"
#include "ap_Menu_Id.h"

#define NrElements(a)	((sizeof(a)/sizeof(a[0])))

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
	AP_Menu_Id					m_id;
};

#define BeginLayout(Name)		static struct _lt s_ltTable_##Name[] = {
#define MenuItem(id)			{ EV_MLF_Normal,		(id)				 },
#define BeginSubMenu(id)		{ EV_MLF_BeginSubMenu,	(id)				 },
#define Separator()				{ EV_MLF_Separator,		AP_MENU_ID__BOGUS1__ },
#define EndSubMenu()			{ EV_MLF_EndSubMenu,	AP_MENU_ID__BOGUS1__ },
#define EndLayout()				};

#include "ap_Menu_Layouts_All.h"

#undef BeginLayout
#undef MenuItem
#undef BeginSubMenu
#undef Separator
#undef EndSubMenu
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
};

#define BeginLayout(Name)		{ #Name, NrElements(s_ltTable_##Name), s_ltTable_##Name },
#define MenuItem(id)			/*nothing*/
#define BeginSubMenu(id)		/*nothing*/
#define Separator()				/*nothing*/
#define EndSubMenu()			/*nothing*/
#define EndLayout()				/*nothing*/

static struct _tt s_ttTable[] =
{

#include "ap_Menu_Layouts_All.h"
	
};

#undef BeginLayout
#undef MenuItem
#undef BeginSubMenu
#undef Separator
#undef EndSubMenu
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
		UT_Bool bResult = pLayout->setLayoutItem(k, plt[k].m_id, plt[k].m_flags);
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

