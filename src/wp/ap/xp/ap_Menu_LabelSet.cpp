/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#include <string.h>
#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ev_Menu_Labels.h"
#include "xap_Menu_ActionSet.h"
#include "ap_Menu_Id.h"
#include "xap_EncodingManager.h"

/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to load labels for each
** language.  It is important that all of the ...LabelSet_*.h files
** allow themselves to be included more than one time.
******************************************************************
*****************************************************************/

#define BeginSetEnc(Language,Locale,bIsDefaultSetForLanguage,Encoding)								\
	static EV_Menu_LabelSet * _ap_CreateLabelSet_##Language##Locale(void)				\
	{	EV_Menu_LabelSet * pLabelSet =													\
			new EV_Menu_LabelSet(#Language"-"#Locale,AP_MENU_ID__BOGUS1__,AP_MENU_ID__BOGUS2__);	\
		UT_ASSERT(pLabelSet);									\
		char* encoding = (Encoding);								\
		char namebuf[2000],statusmsgbuf[2000];

#define BeginSet(Language,Locale,bIsDefaultSetForLanguage) \
		BeginSetEnc(Language,Locale,bIsDefaultSetForLanguage,"")
	
#define MenuLabel(id,szName,szStatusMsg) \
		pLabelSet->setLabel((id),											\
			XAP_EncodingManager::instance->strToNative((szName),encoding,namebuf,sizeof(namebuf)),			\
			XAP_EncodingManager::instance->strToNative((szStatusMsg),encoding,statusmsgbuf,sizeof(statusmsgbuf))	\
		);

#define EndSet()									return pLabelSet; }


#include "ap_Menu_LabelSet_Languages.h"

#undef BeginSetEnc
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
	bool						m_bIsDefaultSetForLanguage;
};


#define BeginSetEnc(Language,Locale,bIsDefaultSetForLanguage,Encoding)	{ #Language"-"#Locale, _ap_CreateLabelSet_##Language##Locale, bIsDefaultSetForLanguage },
#define BeginSet(Language,Locale,bIsDefaultSetForLanguage) \
		BeginSetEnc(Language,Locale,bIsDefaultSetForLanguage,"")
#define MenuLabel(id,szName,szStatusMsg)			/*nothing*/
#define EndSet()									/*nothing*/

static struct _lt s_ltTable[] =
{

#include "ap_Menu_LabelSet_Languages.h"
	
};

#undef BeginSetEnc
#undef BeginSet
#undef MenuLabel
#undef EndSet

/*****************************************************************
******************************************************************
** Put it all together and have a "load LabelSet by Language"
******************************************************************
*****************************************************************/
EV_Menu_LabelSet * AP_CreateMenuLabelSet(const char * szLanguage_)
{
	char buf[300];
	strcpy(buf,szLanguage_ ? szLanguage_ : "");
	char* szLanguage = buf;

	char* dot = strrchr(szLanguage,'.');
	if (dot)
		*dot = '\0'; /* remove encoding part from locale name */

	if (szLanguage && *szLanguage)
	{
		UT_uint32 k;
		
		for (k=0; k<NrElements(s_ltTable); k++)
			if (UT_stricmp(szLanguage,s_ltTable[k].m_name)==0)
				return (s_ltTable[k].m_fn)();

		// if we didn't find an exact match (Language and Locale),
		// try finding the default set for this language.

		char * dash = strchr(szLanguage, '-');
		int len = (dash ? dash - szLanguage : 2);

		for (k=0; k<NrElements(s_ltTable); k++)
			if (   (UT_strnicmp(szLanguage,s_ltTable[k].m_name,len)==0)
				&& (s_ltTable[k].m_bIsDefaultSetForLanguage))
				return (s_ltTable[k].m_fn)();
	}
	
	// we fall back to en-US if they didn't give us a valid language name.
	
	return _ap_CreateLabelSet_enUS();
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

