 
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
#include "ev_Menu_Labels.h"
#include "ap_Menu_ActionSet.h"
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
	
#define MenuLabel(id,szName,szToolTip,szStatusMsg)	pLabelSet->setLabel((id),(szName),(szToolTip),(szStatusMsg));
			
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

#define BeginSet(Language)		{ #Language, _ap_CreateLabelSet_##Language },
#define MenuLabel(id,szName,szToolTip,szStatusMsg)	/*nothing*/
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

