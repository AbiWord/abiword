/* AbiHello
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
#include "ev_Menu_Labels.h"
#include "xap_Menu_ActionSet.h"
#include "ap_Menu_Id.h"

/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to load labels for each
** language.  It is important that all of the ...LabelSet_*.h files
** allow themselves to be included more than one time.
******************************************************************
*****************************************************************/

#define BeginSet(Language)																\
	static EV_Menu_LabelSet * _ap_CreateLabelSet_##Language(void)						\
	{	EV_Menu_LabelSet * pLabelSet =													\
			new EV_Menu_LabelSet(#Language,AP_MENU_ID__BOGUS1__,AP_MENU_ID__BOGUS2__);	\
		UT_ASSERT(pLabelSet);
	
#define MenuLabel(id,szName,szStatusMsg)	pLabelSet->setLabel((id),(szName),(szStatusMsg));
			
#define EndSet()									return pLabelSet; }


#include "ap_Menu_LabelSet_Languages.h"

#undef BeginSet
#undef MenuLabel
#undef EndSet

/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to construct a table of
** language names and function pointers to the constructor
** for that language.
******************************************************************
*****************************************************************/

typedef EV_Menu_LabelSet * (*ap_CreateLabelSet_pFn)(void);

struct _lt
{
	const char *				m_name;
	ap_CreateLabelSet_pFn		m_fn;
};

#define BeginSet(Language)							{ #Language, _ap_CreateLabelSet_##Language },
#define MenuLabel(id,szName,szStatusMsg)	/*nothing*/
#define EndSet()									/*nothing*/

static struct _lt s_ltTable[] =
{

#include "ap_Menu_LabelSet_Languages.h"
	
};

#define NrElements(a)	((sizeof(a)/sizeof(a[0])))

#undef BeginSet
#undef MenuLabel
#undef EndSet

/*****************************************************************
******************************************************************
** Put it all together and have a "load LabelSet by Language"
******************************************************************
*****************************************************************/

EV_Menu_LabelSet * AP_CreateMenuLabelSet(const char * szLanguage)
{
	if (szLanguage && *szLanguage)
		for (UT_uint32 k=0; k<NrElements(s_ltTable); k++)
			if (UT_stricmp(szLanguage,s_ltTable[k].m_name)==0)
				return (s_ltTable[k].m_fn)();

	// we fall back to EnUS if they didn't give us a valid language name.
	
	return _ap_CreateLabelSet_EnUS();
}

UT_uint32 AP_GetMenuLabelSetLanguageCount(void)
{
	return NrElements(s_ltTable);
}

const char * AP_GetNthMenuLabelLanguageName(UT_uint32 ndx)
{
	UT_ASSERT(ndx < NrElements(s_ltTable));

	return s_ltTable[ndx].m_name;
}

