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
#include "ut_debugmsg.h"
#include "ut_assert.h"

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
	{"-none-",		NULL, XAP_STRING_ID_LANG_0,     UTLANG_LTR},
	{"am-ET",		NULL, XAP_STRING_ID_LANG_AM_ET, UTLANG_LTR},
	{"af-ZA",		NULL, XAP_STRING_ID_LANG_AF_ZA, UTLANG_LTR},
	{"ar-EG",		NULL, XAP_STRING_ID_LANG_AR_EG, UTLANG_RTL},
	{"ar-SA",		NULL, XAP_STRING_ID_LANG_AR_SA, UTLANG_RTL},
	{"as-IN",		NULL, XAP_STRING_ID_LANG_AS_IN, UTLANG_LTR},
	{"be-BY",		NULL, XAP_STRING_ID_LANG_BE_BY, UTLANG_LTR},
	{"bg-BG",		NULL, XAP_STRING_ID_LANG_BG_BG, UTLANG_LTR},
	{"br-FR",		NULL, XAP_STRING_ID_LANG_BR_FR, UTLANG_LTR},		
	{"ca-ES",		NULL, XAP_STRING_ID_LANG_CA_ES, UTLANG_LTR},	
	{"co-FR",		NULL, XAP_STRING_ID_LANG_CO_FR, UTLANG_LTR},	
	{"cs-CZ",		NULL, XAP_STRING_ID_LANG_CS_CZ, UTLANG_LTR},
	{"cy-GB",		NULL, XAP_STRING_ID_LANG_CY_GB, UTLANG_LTR},
	{"da-DK",		NULL, XAP_STRING_ID_LANG_DA_DK, UTLANG_LTR},
	{"de-AT",		NULL, XAP_STRING_ID_LANG_DE_AT, UTLANG_LTR},
	{"de-CH",		NULL, XAP_STRING_ID_LANG_DE_CH, UTLANG_LTR},
	{"de-DE",		NULL, XAP_STRING_ID_LANG_DE_DE, UTLANG_LTR},
	{"el-GR",		NULL, XAP_STRING_ID_LANG_EL_GR, UTLANG_LTR},
	{"en-AU",		NULL, XAP_STRING_ID_LANG_EN_AU, UTLANG_LTR},
	{"en-CA",		NULL, XAP_STRING_ID_LANG_EN_CA, UTLANG_LTR},
	{"en-GB",		NULL, XAP_STRING_ID_LANG_EN_GB, UTLANG_LTR},
	{"en-IE",		NULL, XAP_STRING_ID_LANG_EN_IE, UTLANG_LTR},
	{"en-NZ",		NULL, XAP_STRING_ID_LANG_EN_NZ, UTLANG_LTR},
	{"en-US",		NULL, XAP_STRING_ID_LANG_EN_US, UTLANG_LTR},
	{"en-ZA",		NULL, XAP_STRING_ID_LANG_EN_ZA, UTLANG_LTR},
	{"eo",			NULL, XAP_STRING_ID_LANG_EO,    UTLANG_LTR},
	{"es-ES",		NULL, XAP_STRING_ID_LANG_ES_ES, UTLANG_LTR},
	{"es-MX",		NULL, XAP_STRING_ID_LANG_ES_MX, UTLANG_LTR},
	{"et",			NULL, XAP_STRING_ID_LANG_ET,    UTLANG_LTR},		// Hipi: Why not et-EE?
	{"eu-ES",		NULL, XAP_STRING_ID_LANG_EU_ES, UTLANG_LTR},	// Hipi: What about eu-FR?
	{"fa-IR",		NULL, XAP_STRING_ID_LANG_FA_IR, UTLANG_RTL},
	{"fi-FI",		NULL, XAP_STRING_ID_LANG_FI_FI, UTLANG_LTR},
	{"fr-BE",		NULL, XAP_STRING_ID_LANG_FR_BE, UTLANG_LTR},
	{"fr-CA",		NULL, XAP_STRING_ID_LANG_FR_CA, UTLANG_LTR},
	{"fr-CH",		NULL, XAP_STRING_ID_LANG_FR_CH, UTLANG_LTR},
	{"fr-FR",		NULL, XAP_STRING_ID_LANG_FR_FR, UTLANG_LTR},
	{"fy-NL",		NULL, XAP_STRING_ID_LANG_FY_NL, UTLANG_LTR},	
	{"ga-IE",		NULL, XAP_STRING_ID_LANG_GA_IE, UTLANG_LTR},
	{"gl-ES",		NULL, XAP_STRING_ID_LANG_GL_ES, UTLANG_LTR},
	{"ha-NE",		NULL, XAP_STRING_ID_LANG_HA_NE, UTLANG_LTR},
	{"ha-NG",		NULL, XAP_STRING_ID_LANG_HA_NG, UTLANG_LTR},
	{"he-IL",		NULL, XAP_STRING_ID_LANG_HE_IL, UTLANG_RTL},	// was iw-IL - why?
	{"hi-IN",		NULL, XAP_STRING_ID_LANG_HI_IN, UTLANG_LTR},
	{"hr",			NULL, XAP_STRING_ID_LANG_HR,    UTLANG_LTR},		// Hipi: Why not hr-HR?
	{"hu-HU",		NULL, XAP_STRING_ID_LANG_HU_HU, UTLANG_LTR},
	{"hy-AM",		NULL, XAP_STRING_ID_LANG_HY_AM, UTLANG_LTR},	// Win2K shows "hy-am"
	{"is-IS",		NULL, XAP_STRING_ID_LANG_IS_IS, UTLANG_LTR},
	{"ia",			NULL, XAP_STRING_ID_LANG_IA,    UTLANG_LTR},
	{"id-ID",		NULL, XAP_STRING_ID_LANG_ID_ID, UTLANG_LTR},
	{"it-IT",		NULL, XAP_STRING_ID_LANG_IT_IT, UTLANG_LTR},
	{"ja-JP",		NULL, XAP_STRING_ID_LANG_JA_JP, UTLANG_VERTICAL},	// TODO also UTLANG_LTR
	{"ka-GE",		NULL, XAP_STRING_ID_LANG_KA_GE, UTLANG_LTR},
	{"ko-KR",		NULL, XAP_STRING_ID_LANG_KO_KR, UTLANG_VERTICAL},	// TODO also UTLANG_LTR, Hipi: What about ko-KP?
	{"kw-GB",		NULL, XAP_STRING_ID_LANG_KW_GB, UTLANG_LTR},	
	{"la-IT",		NULL, XAP_STRING_ID_LANG_LA_IT, UTLANG_LTR},	// Hipi: Should be just "la"
	{"lt-LT",		NULL, XAP_STRING_ID_LANG_LT_LT, UTLANG_LTR},
	{"lv-LV",		NULL, XAP_STRING_ID_LANG_LV_LV, UTLANG_LTR},
	{"mh-MH",		NULL, XAP_STRING_ID_LANG_MH_MH, UTLANG_LTR},
	{"mh-NR",		NULL, XAP_STRING_ID_LANG_MH_NR, UTLANG_LTR},
	{"mk",			NULL, XAP_STRING_ID_LANG_MK,    UTLANG_LTR},		// Hipi: Why not mk-MK?
	{"nb-NO",		NULL, XAP_STRING_ID_LANG_NB_NO, UTLANG_LTR},
	{"nl-BE",		NULL, XAP_STRING_ID_LANG_NL_BE, UTLANG_LTR},
	{"nl-NL",		NULL, XAP_STRING_ID_LANG_NL_NL, UTLANG_LTR},
	{"nn-NO",		NULL, XAP_STRING_ID_LANG_NN_NO, UTLANG_LTR},	
	{"oc-FR",		NULL, XAP_STRING_ID_LANG_OC_FR, UTLANG_LTR},	
	{"pl-PL",		NULL, XAP_STRING_ID_LANG_PL_PL, UTLANG_LTR},
	{"pt-BR",		NULL, XAP_STRING_ID_LANG_PT_BR, UTLANG_LTR},
	{"pt-PT",		NULL, XAP_STRING_ID_LANG_PT_PT, UTLANG_LTR},
	{"ro-RO",		NULL, XAP_STRING_ID_LANG_RO_RO, UTLANG_LTR},
	{"ru-RU",		NULL, XAP_STRING_ID_LANG_RU_RU, UTLANG_LTR},	
	{"sc-IT",		NULL, XAP_STRING_ID_LANG_SC_IT, UTLANG_LTR},	
	{"sk-SK",		NULL, XAP_STRING_ID_LANG_SK_SK, UTLANG_LTR},
	{"sl-SI",		NULL, XAP_STRING_ID_LANG_SL_SI, UTLANG_LTR},
	{"sq-AL",		NULL, XAP_STRING_ID_LANG_SQ_AL, UTLANG_LTR},
	{"sr",			NULL, XAP_STRING_ID_LANG_SR,    UTLANG_LTR},		// Why not sr-YU?
	{"sv-SE",		NULL, XAP_STRING_ID_LANG_SV_SE, UTLANG_LTR},
	{"th-TH",		NULL, XAP_STRING_ID_LANG_TH_TH, UTLANG_LTR},
	{"tr-TR",		NULL, XAP_STRING_ID_LANG_TR_TR, UTLANG_LTR},		// UTLANG_RTL for Ottoman Turkish
	{"uk-UA",		NULL, XAP_STRING_ID_LANG_UK_UA, UTLANG_LTR},
	{"ur-PK",		NULL, XAP_STRING_ID_LANG_UR_PK, UTLANG_RTL},
	{"vi-VN",		NULL, XAP_STRING_ID_LANG_VI_VN, UTLANG_LTR},
	{"yi",			NULL, XAP_STRING_ID_LANG_YI,    UTLANG_RTL},
	{"zh-CN",		NULL, XAP_STRING_ID_LANG_ZH_CN, UTLANG_VERTICAL},	// TODO also UTLANG_LTR
	{"zh-HK",		NULL, XAP_STRING_ID_LANG_ZH_HK, UTLANG_VERTICAL},	// TODO also UTLANG_LTR
	{"zh-SG",		NULL, XAP_STRING_ID_LANG_ZH_SG, UTLANG_VERTICAL},	// TODO also UTLANG_LTR
	{"zh-TW",		NULL, XAP_STRING_ID_LANG_ZH_TW, UTLANG_VERTICAL},	// TODO also UTLANG_LTR
};

static int s_compareQ(const void * a, const void *b)
{
	const lang_entry * A = static_cast<const lang_entry *>(a);
	const lang_entry * B = static_cast<const lang_entry *>(b);
	return UT_strcmp(A->prop, B->prop);
}

static int s_compareB(const void * l, const void *e)
{
	const XML_Char * L   = static_cast<const XML_Char *>(l);
	const lang_entry * E = static_cast<const lang_entry *>(e);
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
			s_Table[i].lang = const_cast<XML_Char *>(pSS->getValue(s_Table[i].id));
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

	UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
				 "language to the tables\n", lang));
	UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
	return NULL;
}

UT_uint32 UT_Language::getIndxFromProperty(const XML_Char * prop)
{
	for(UT_uint32 i = 0; i < NrElements(s_Table); i++)
	{
		if(!UT_strcmp(prop, s_Table[i].prop))
			return i;
	}

	UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
				 "language to the tables\n", prop));
	UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
	return 0;
}

UT_uint32 UT_Language::getIdFromProperty(const XML_Char * prop)
{
	lang_entry * e = static_cast<lang_entry *>(bsearch(prop, s_Table, NrElements(s_Table), sizeof(lang_entry), s_compareB));
	if(e)
		return e->id;
	else
	{
		UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
					 "language to the tables\n", prop));
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		return 0;
	}
	
}

// This function is not as useless as might seem; it takes a pointer
// to a property string, finds the same property in the static table
// and returns the pointer to it. This is used by fp_TextRun to set its
// m_pLanguage member; by always refering into the static table it is
// possible to compare the language property by simply comparing the
// pointers, rather than having to use strcmp()

const XML_Char *  UT_Language::getPropertyFromProperty(const XML_Char * prop)
{
	lang_entry * e = static_cast<lang_entry *>(bsearch(prop, s_Table, NrElements(s_Table), sizeof(lang_entry), s_compareB));
	if(e)
		return e->prop;
	else
	{
		UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
					 "language to the tables\n", prop));
		//UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		return 0;
	}
}

UT_LANGUAGE_ORDER UT_Language::getOrderFromProperty(const XML_Char * prop)
{
	lang_entry * e = static_cast<lang_entry *>(bsearch(prop, s_Table, NrElements(s_Table), sizeof(lang_entry), s_compareB));
	if(e)
		return e->order;
	else
	{
		UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
					 "language to the tables\n", prop));
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		return UTLANG_LTR;
	}
}
