/* AbiSource Program Utilities
 * Copyright (C) 2001 Tomas Frydrych
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

#include <stdlib.h>

#include "ut_Language.h"
#include "ut_string.h"

#include "xap_App.h"
#include "xap_Strings.h"

// Please keep the list below alphabetised by the lang; even though 
// this is not required for it to work, it will make it easier to maintain.

// to add a new language:
// (1) check on this list it is not already there;
// (2) add it to the list in xap_String_Id.h 
// (3) add it here, using the ID corresponding to the one
//     from xap_String_Id.h

static lang_entry s_Table[] = 
{
	//the property value, the localised translation, the numerical id
	{"-none-",		NULL, XAP_STRING_ID_LANG_0},
	{"af-ZA",		NULL, XAP_STRING_ID_LANG_AF_ZA},
	{"ar-EG",		NULL, XAP_STRING_ID_LANG_AR_EG},
	{"ar-SA",		NULL, XAP_STRING_ID_LANG_AR_SA},
	{"as-IN",		NULL, XAP_STRING_ID_LANG_AS_IN},
	{"be-BY",		NULL, XAP_STRING_ID_LANG_BE_BY},
	{"bg-BG",		NULL, XAP_STRING_ID_LANG_BG_BG},
	{"ca-ES",		NULL, XAP_STRING_ID_LANG_CA_ES},
	{"cs-CZ",		NULL, XAP_STRING_ID_LANG_CS_CZ},
	{"cy-GB",		NULL, XAP_STRING_ID_LANG_CY_GB},
	{"da-DK",		NULL, XAP_STRING_ID_LANG_DA_DK},
	{"de-AT",		NULL, XAP_STRING_ID_LANG_DE_AT},
	{"de-CH",		NULL, XAP_STRING_ID_LANG_DE_CH},
	{"de-DE",		NULL, XAP_STRING_ID_LANG_DE_DE},
	{"el-GR",		NULL, XAP_STRING_ID_LANG_EL_GR},
	{"en-AU",		NULL, XAP_STRING_ID_LANG_EN_AU},
	{"en-CA",		NULL, XAP_STRING_ID_LANG_EN_CA},
	{"en-GB",		NULL, XAP_STRING_ID_LANG_EN_GB},
	{"en-IE",		NULL, XAP_STRING_ID_LANG_EN_IE},
	{"en-NZ",		NULL, XAP_STRING_ID_LANG_EN_NZ},
	{"en-US",		NULL, XAP_STRING_ID_LANG_EN_US},
	{"en-ZA",		NULL, XAP_STRING_ID_LANG_EN_ZA},
	{"eo",			NULL, XAP_STRING_ID_LANG_EO},
	{"es-ES",		NULL, XAP_STRING_ID_LANG_ES_ES},
	{"es-MX",		NULL, XAP_STRING_ID_LANG_ES_MX},
	{"eu-ES",		NULL, XAP_STRING_ID_LANG_EU_ES},
	{"fa-IR",		NULL, XAP_STRING_ID_LANG_FA_IR},
	{"fi-FI",		NULL, XAP_STRING_ID_LANG_FI_FI},
	{"fr-BE",		NULL, XAP_STRING_ID_LANG_FR_BE},
	{"fr-CA",		NULL, XAP_STRING_ID_LANG_FR_CA},
	{"fr-CH",		NULL, XAP_STRING_ID_LANG_FR_CH},
	{"fr-FR",		NULL, XAP_STRING_ID_LANG_FR_FR},
	{"ga-IE",		NULL, XAP_STRING_ID_LANG_GA_IE},
	{"gl-ES",		NULL, XAP_STRING_ID_LANG_GL_ES},
	{"he-IL",		NULL, XAP_STRING_ID_LANG_HE_IL},	// was iw-IL - why?
	{"hi-IN",		NULL, XAP_STRING_ID_LANG_HI_IN},
	{"hu-HU",		NULL, XAP_STRING_ID_LANG_HU_HU},
	{"hy-AM",		NULL, XAP_STRING_ID_LANG_HY_AM},	// Win2K shows "hy-am"
	{"is-IS",		NULL, XAP_STRING_ID_LANG_IS_IS},
	{"ia",			NULL, XAP_STRING_ID_LANG_IA},
	{"id-ID",		NULL, XAP_STRING_ID_LANG_ID_ID},
	{"it-IT",		NULL, XAP_STRING_ID_LANG_IT_IT},
	{"ja-JP",		NULL, XAP_STRING_ID_LANG_JA_JP},
	{"ka-GE",		NULL, XAP_STRING_ID_LANG_KA_GE},
	{"ko-KR",		NULL, XAP_STRING_ID_LANG_KO_KR},
	{"la-IT",		NULL, XAP_STRING_ID_LANG_LA_IT},
	{"lt-LT",		NULL, XAP_STRING_ID_LANG_LT_LT},
	{"lv-LV",		NULL, XAP_STRING_ID_LANG_LV_LV},
	{"nb-NO",		NULL, XAP_STRING_ID_LANG_NB_NO},
	{"nl-BE",		NULL, XAP_STRING_ID_LANG_NL_BE},
	{"nl-NL",		NULL, XAP_STRING_ID_LANG_NL_NL},
	{"nn-NO",		NULL, XAP_STRING_ID_LANG_NN_NO},
	{"pl-PL",		NULL, XAP_STRING_ID_LANG_PL_PL},
	{"pt-BR",		NULL, XAP_STRING_ID_LANG_PT_BR},
	{"pt-PT",		NULL, XAP_STRING_ID_LANG_PT_PT},
	{"ro-RO",		NULL, XAP_STRING_ID_LANG_RO_RO},
	{"ru-RU",		NULL, XAP_STRING_ID_LANG_RU_RU},
	{"sk-SK",		NULL, XAP_STRING_ID_LANG_SK_SK},
	{"sl-SI",		NULL, XAP_STRING_ID_LANG_SL_SI},
	{"sv-SE",		NULL, XAP_STRING_ID_LANG_SV_SE},
	{"th-TH",		NULL, XAP_STRING_ID_LANG_TH_TH},
	{"tr-TR",		NULL, XAP_STRING_ID_LANG_TR_TR},
	{"uk-UA",		NULL, XAP_STRING_ID_LANG_UK_UA},
	{"vi-VN",		NULL, XAP_STRING_ID_LANG_VI_VN},
	{"yi",			NULL, XAP_STRING_ID_LANG_YI},
	{"zh-CN",		NULL, XAP_STRING_ID_LANG_ZH_CN},
	{"zh-HK",		NULL, XAP_STRING_ID_LANG_ZH_HK},
	{"zh-SG",		NULL, XAP_STRING_ID_LANG_ZH_SG},
	{"zh-TW",		NULL, XAP_STRING_ID_LANG_ZH_TW},
};

static int s_compareQ(const void * a, const void *b)
{
	const lang_entry * A = (const lang_entry *) a;
	const lang_entry * B = (const lang_entry *) b;
	return UT_strcmp(A->prop, B->prop);
}

static int s_compareB(const void * l, const void *e)
{
	const XML_Char * L   = (const XML_Char * ) l;
	const lang_entry * E = (const lang_entry *) e;
	return UT_strcmp(L, E->prop);
}

bool UT_Language::s_Init = true;


// the constructor looks up the translations for the property and sorts out the table
// alphabetically by the property

UT_Language::UT_Language()
{
	if(s_Init) //only do this once
	{
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

		for(UT_uint32 i = 0; i < NrElements(s_Table); i++)
		{
			s_Table[i].lang = (XML_Char *) pSS->getValue(s_Table[i].id);
		}

		qsort(s_Table, NrElements(s_Table), sizeof(lang_entry), s_compareQ);
		s_Init = false;
	}
}

UT_uint32 UT_Language::getCount()
{
	return (NrElements(s_Table));
}

const XML_Char * UT_Language::getNthProperty(UT_uint32 n)
{
	return (s_Table[n].prop);
}

const XML_Char * UT_Language::getNthLanguage(UT_uint32 n)
{
	return (s_Table[n].lang);
}


const XML_Char * UT_Language::getPropertyFromLanguage(const XML_Char * lang)
{
	for(UT_uint32 i = 0; i < NrElements(s_Table); i++)
	{
		if(!UT_strcmp(lang, s_Table[i].lang))
			return s_Table[i].prop;
	}
	return NULL;
}

UT_uint32 UT_Language::getIndxFromProperty(const XML_Char * prop)
{
	for(UT_uint32 i = 0; i < NrElements(s_Table); i++)
	{
		if(!UT_strcmp(prop, s_Table[i].prop))
			return i;
	}
	return 0;
}

UT_uint32 UT_Language::getIdFromProperty(const XML_Char * prop)
{
	lang_entry * e = (lang_entry *) bsearch(prop, s_Table, NrElements(s_Table), sizeof(lang_entry), s_compareB);
	if(e)
		return e->id;
	else
		return 0;
}

// this function is not as useless as might seem; it takes a pointer to a property string, finds the same
// property in the static table and returns the pointer to it
// this is used by fp_TextRun to set its m_pLanguage member; by always refering into the static table
// it is possible to compare the language property by simply comparing the pointers, rather than
// having to use strcmp

const XML_Char *  UT_Language::getPropertyFromProperty(const XML_Char * prop)
{
	lang_entry * e = (lang_entry *) bsearch(prop, s_Table, NrElements(s_Table), sizeof(lang_entry), s_compareB);
	if(e)
		return e->prop;
	else
		return 0;
}

