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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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

static UT_LangRecord s_Table[] = 
{
	//language code, localised language name, numerical id, text
	//direction
	{"-none-",		nullptr, XAP_STRING_ID_LANG_0,     UTLANG_LTR},
	{"ach",  		nullptr, XAP_STRING_ID_LANG_ACH,   UTLANG_LTR},
	{"af-ZA",		nullptr, XAP_STRING_ID_LANG_AF_ZA, UTLANG_LTR},
	{"ak-GH",		nullptr, XAP_STRING_ID_LANG_AK_GH, UTLANG_LTR},
	{"am-ET",		nullptr, XAP_STRING_ID_LANG_AM_ET, UTLANG_LTR},
	{"ar",			nullptr, XAP_STRING_ID_LANG_AR,    UTLANG_RTL},
	{"ar-EG",		nullptr, XAP_STRING_ID_LANG_AR_EG, UTLANG_RTL},
	{"ar-SA",		nullptr, XAP_STRING_ID_LANG_AR_SA, UTLANG_RTL},
	{"as-IN",		nullptr, XAP_STRING_ID_LANG_AS_IN, UTLANG_LTR},
	{"ast-ES",		nullptr, XAP_STRING_ID_LANG_AST_ES,UTLANG_LTR},
	{"aym-BO",		nullptr, XAP_STRING_ID_LANG_AYM_BO,UTLANG_LTR},
	{"ayc-BO",		nullptr, XAP_STRING_ID_LANG_AYC_BO,UTLANG_LTR},
	{"ayr",			nullptr, XAP_STRING_ID_LANG_AYR,   UTLANG_LTR},
	{"be-BY",		nullptr, XAP_STRING_ID_LANG_BE_BY, UTLANG_LTR},
	{"be@latin",		nullptr, XAP_STRING_ID_LANG_BE_LATIN, UTLANG_LTR},
	{"bg-BG",		nullptr, XAP_STRING_ID_LANG_BG_BG, UTLANG_LTR},
	{"bm",   		nullptr, XAP_STRING_ID_LANG_BM,    UTLANG_LTR},
	{"bn-IN",       	nullptr, XAP_STRING_ID_LANG_BN_IN, UTLANG_LTR},
	{"br-FR",		nullptr, XAP_STRING_ID_LANG_BR_FR, UTLANG_LTR},
	{"bs-BA",		nullptr, XAP_STRING_ID_LANG_BS_BA, UTLANG_LTR},
	{"ca-ES",		nullptr, XAP_STRING_ID_LANG_CA_ES, UTLANG_LTR},
	{"cgg",  		nullptr, XAP_STRING_ID_LANG_CGG,   UTLANG_LTR},
	{"co-FR",		nullptr, XAP_STRING_ID_LANG_CO_FR, UTLANG_LTR},
	{"cop-EG",		nullptr, XAP_STRING_ID_LANG_COP_EG,UTLANG_LTR},
	{"cs-CZ",		nullptr, XAP_STRING_ID_LANG_CS_CZ, UTLANG_LTR},
	{"cy-GB",		nullptr, XAP_STRING_ID_LANG_CY_GB, UTLANG_LTR},
	{"da-DK",		nullptr, XAP_STRING_ID_LANG_DA_DK, UTLANG_LTR},
	{"de-AT",		nullptr, XAP_STRING_ID_LANG_DE_AT, UTLANG_LTR},
	{"de-CH",		nullptr, XAP_STRING_ID_LANG_DE_CH, UTLANG_LTR},
	{"de-DE",		nullptr, XAP_STRING_ID_LANG_DE_DE, UTLANG_LTR},
	{"dv-MV",		nullptr, XAP_STRING_ID_LANG_DV_MV, UTLANG_LTR},
	{"el-GR",		nullptr, XAP_STRING_ID_LANG_EL_GR, UTLANG_LTR},
	{"en-AU",		nullptr, XAP_STRING_ID_LANG_EN_AU, UTLANG_LTR},
	{"en-CA",		nullptr, XAP_STRING_ID_LANG_EN_CA, UTLANG_LTR},
	{"en-GB",		nullptr, XAP_STRING_ID_LANG_EN_GB, UTLANG_LTR},
	{"en-IE",		nullptr, XAP_STRING_ID_LANG_EN_IE, UTLANG_LTR},
	{"en-IN",		nullptr, XAP_STRING_ID_LANG_EN_IN, UTLANG_LTR},
	{"en-NZ",		nullptr, XAP_STRING_ID_LANG_EN_NZ, UTLANG_LTR},
	{"en-US",		nullptr, XAP_STRING_ID_LANG_EN_US, UTLANG_LTR},
	{"en-ZA",		nullptr, XAP_STRING_ID_LANG_EN_ZA, UTLANG_LTR},
	{"eo",			nullptr, XAP_STRING_ID_LANG_EO,    UTLANG_LTR},
	{"es-ES",		nullptr, XAP_STRING_ID_LANG_ES_ES, UTLANG_LTR},
	{"es-MX",		nullptr, XAP_STRING_ID_LANG_ES_MX, UTLANG_LTR},
	{"et",			nullptr, XAP_STRING_ID_LANG_ET,    UTLANG_LTR},
	{"eu-ES",		nullptr, XAP_STRING_ID_LANG_EU_ES, UTLANG_LTR},
	{"fa-AF",		nullptr, XAP_STRING_ID_LANG_FA_AF, UTLANG_RTL},
	{"fa-IR",		nullptr, XAP_STRING_ID_LANG_FA_IR, UTLANG_RTL},
	{"fi-FI",		nullptr, XAP_STRING_ID_LANG_FI_FI, UTLANG_LTR},
	{"fil-PH",		nullptr, XAP_STRING_ID_LANG_FIL_PH, UTLANG_LTR},
	{"fr-BE",		nullptr, XAP_STRING_ID_LANG_FR_BE, UTLANG_LTR},
	{"fr-CA",		nullptr, XAP_STRING_ID_LANG_FR_CA, UTLANG_LTR},
	{"fr-CH",		nullptr, XAP_STRING_ID_LANG_FR_CH, UTLANG_LTR},
	{"fr-FR",		nullptr, XAP_STRING_ID_LANG_FR_FR, UTLANG_LTR},
	{"fy-NL",		nullptr, XAP_STRING_ID_LANG_FY_NL, UTLANG_LTR},
	{"ff",			nullptr, XAP_STRING_ID_LANG_FF,    UTLANG_LTR},
	{"ga-IE",		nullptr, XAP_STRING_ID_LANG_GA_IE, UTLANG_LTR},
	{"gl",			nullptr, XAP_STRING_ID_LANG_GL,    UTLANG_LTR},
	{"ha-NE",		nullptr, XAP_STRING_ID_LANG_HA_NE, UTLANG_LTR},
	{"ha-NG",		nullptr, XAP_STRING_ID_LANG_HA_NG, UTLANG_LTR},
	{"haw-US",      	nullptr, XAP_STRING_ID_LANG_HAW_US,UTLANG_LTR},
	{"he-IL",		nullptr, XAP_STRING_ID_LANG_HE_IL, UTLANG_RTL},
	{"hi-IN",		nullptr, XAP_STRING_ID_LANG_HI_IN, UTLANG_LTR},
	{"hr-HR",		nullptr, XAP_STRING_ID_LANG_HR_HR, UTLANG_LTR},
	{"hu-HU",		nullptr, XAP_STRING_ID_LANG_HU_HU, UTLANG_LTR},
	{"hy-AM",		nullptr, XAP_STRING_ID_LANG_HY_AM, UTLANG_LTR},
	{"ia",			nullptr, XAP_STRING_ID_LANG_IA,    UTLANG_LTR},
	{"id-ID",		nullptr, XAP_STRING_ID_LANG_ID_ID, UTLANG_LTR},
	{"is-IS",		nullptr, XAP_STRING_ID_LANG_IS_IS, UTLANG_LTR},
	{"it-IT",		nullptr, XAP_STRING_ID_LANG_IT_IT, UTLANG_LTR},
	{"iu-CA",		nullptr, XAP_STRING_ID_LANG_IU_CA, UTLANG_LTR},
	{"ja-JP",		nullptr, XAP_STRING_ID_LANG_JA_JP, UTLANG_VERTICAL},	// TODO also UTLANG_LTR
	{"jbo", 		nullptr, XAP_STRING_ID_LANG_JBO,   UTLANG_LTR},
	{"ka-GE",		nullptr, XAP_STRING_ID_LANG_KA_GE, UTLANG_LTR},
	{"kk-KZ",		nullptr, XAP_STRING_ID_LANG_KK_KZ, UTLANG_LTR},
	{"km-KH",		nullptr, XAP_STRING_ID_LANG_KM_KH, UTLANG_LTR},
	{"kn-IN",       	nullptr, XAP_STRING_ID_LANG_KN_IN, UTLANG_LTR},
	{"ko-KR",		nullptr, XAP_STRING_ID_LANG_KO_KR, UTLANG_VERTICAL},	// TODO also UTLANG_LTR
	{"ko",			nullptr, XAP_STRING_ID_LANG_KO,    UTLANG_LTR},
	{"ku",			nullptr, XAP_STRING_ID_LANG_KU,    UTLANG_LTR},
	{"kw-GB",		nullptr, XAP_STRING_ID_LANG_KW_GB, UTLANG_LTR},
	{"la-IT",		nullptr, XAP_STRING_ID_LANG_LA_IT, UTLANG_LTR},
	{"lg",  		nullptr, XAP_STRING_ID_LANG_LG,    UTLANG_LTR},
	{"lo-LA",		nullptr, XAP_STRING_ID_LANG_LO_LA, UTLANG_LTR},
	{"lt-LT",		nullptr, XAP_STRING_ID_LANG_LT_LT, UTLANG_LTR},
	{"lv-LV",		nullptr, XAP_STRING_ID_LANG_LV_LV, UTLANG_LTR},
	{"mg-MG",		nullptr, XAP_STRING_ID_LANG_MG_MG, UTLANG_LTR},
	{"mh-MH",		nullptr, XAP_STRING_ID_LANG_MH_MH, UTLANG_LTR},
	{"mh-NR",		nullptr, XAP_STRING_ID_LANG_MH_NR, UTLANG_LTR},
	{"mi-NZ",		nullptr, XAP_STRING_ID_LANG_MI_NZ, UTLANG_LTR},
	{"mk",			nullptr, XAP_STRING_ID_LANG_MK,    UTLANG_LTR},
	{"mn-MN",		nullptr, XAP_STRING_ID_LANG_MN_MN, UTLANG_LTR},
	{"mnk-SN",		nullptr, XAP_STRING_ID_LANG_MNK_SN,UTLANG_LTR},
	{"mr-IN",		nullptr, XAP_STRING_ID_LANG_MR_IN, UTLANG_LTR},
	{"ms-MY",		nullptr, XAP_STRING_ID_LANG_MS_MY, UTLANG_LTR},
	{"nb-NO",		nullptr, XAP_STRING_ID_LANG_NB_NO, UTLANG_LTR},
	{"ne-NP",		nullptr, XAP_STRING_ID_LANG_NE_NP, UTLANG_LTR},
	{"nl-BE",		nullptr, XAP_STRING_ID_LANG_NL_BE, UTLANG_LTR},
	{"nl-NL",		nullptr, XAP_STRING_ID_LANG_NL_NL, UTLANG_LTR},
	{"nn-NO",		nullptr, XAP_STRING_ID_LANG_NN_NO, UTLANG_LTR},
	{"oc-FR",		nullptr, XAP_STRING_ID_LANG_OC_FR, UTLANG_LTR},
	{"pa-IN",		nullptr, XAP_STRING_ID_LANG_PA_IN, UTLANG_LTR},
	{"pa-PK",		nullptr, XAP_STRING_ID_LANG_PA_PK, UTLANG_RTL},
	{"pl-PL",		nullptr, XAP_STRING_ID_LANG_PL_PL, UTLANG_LTR},
	{"ps",			nullptr, XAP_STRING_ID_LANG_PS,    UTLANG_RTL},
	{"pt-BR",		nullptr, XAP_STRING_ID_LANG_PT_BR, UTLANG_LTR},
	{"pt-PT",		nullptr, XAP_STRING_ID_LANG_PT_PT, UTLANG_LTR},
	{"qu-BO",		nullptr, XAP_STRING_ID_LANG_QU_BO, UTLANG_LTR},
	{"quh-BO",		nullptr, XAP_STRING_ID_LANG_QUH_BO,UTLANG_LTR},
	{"qul-BO",		nullptr, XAP_STRING_ID_LANG_QUL_BO,UTLANG_LTR},
	{"quz",  		nullptr, XAP_STRING_ID_LANG_QUZ,   UTLANG_LTR},
	{"ro-RO",		nullptr, XAP_STRING_ID_LANG_RO_RO, UTLANG_LTR},
	{"ru-RU",		nullptr, XAP_STRING_ID_LANG_RU_RU, UTLANG_LTR},
	{"ru@petr1708",		nullptr, XAP_STRING_ID_LANG_RU_PETR1708, UTLANG_LTR},
	{"sc-IT",		nullptr, XAP_STRING_ID_LANG_SC_IT, UTLANG_LTR},
	{"sk-SK",		nullptr, XAP_STRING_ID_LANG_SK_SK, UTLANG_LTR},
	{"sl-SI",		nullptr, XAP_STRING_ID_LANG_SL_SI, UTLANG_LTR},
	{"son", 		nullptr, XAP_STRING_ID_LANG_SON,   UTLANG_LTR},
	{"sq-AL",		nullptr, XAP_STRING_ID_LANG_SQ_AL, UTLANG_LTR},
	{"sr",			nullptr, XAP_STRING_ID_LANG_SR,    UTLANG_LTR},
	{"sr@latin",		nullptr, XAP_STRING_ID_LANG_SR_LATIN, UTLANG_LTR},
	{"sv-SE",		nullptr, XAP_STRING_ID_LANG_SV_SE, UTLANG_LTR},
	{"sw",  		nullptr, XAP_STRING_ID_LANG_SW,    UTLANG_LTR},
	{"syr",  		nullptr, XAP_STRING_ID_LANG_SYR,   UTLANG_RTL},
	{"ta-IN",		nullptr, XAP_STRING_ID_LANG_TA_IN, UTLANG_LTR},
	{"te-IN",		nullptr, XAP_STRING_ID_LANG_TE_IN, UTLANG_LTR},
	{"th-TH",		nullptr, XAP_STRING_ID_LANG_TH_TH, UTLANG_LTR},
	{"tl-PH",		nullptr, XAP_STRING_ID_LANG_TL_PH, UTLANG_LTR},
	{"tr-TR",		nullptr, XAP_STRING_ID_LANG_TR_TR, UTLANG_LTR},		// UTLANG_RTL for Ottoman Turkish
	{"uk-UA",		nullptr, XAP_STRING_ID_LANG_UK_UA, UTLANG_LTR},
	{"ur-PK",		nullptr, XAP_STRING_ID_LANG_UR_PK, UTLANG_RTL},
	{"ur",			nullptr, XAP_STRING_ID_LANG_UR,    UTLANG_RTL},
	{"uz-UZ",		nullptr, XAP_STRING_ID_LANG_UZ_UZ, UTLANG_RTL},
	{"vi-VN",		nullptr, XAP_STRING_ID_LANG_VI_VN, UTLANG_LTR},
	{"wo-SN",		nullptr, XAP_STRING_ID_LANG_WO_SN, UTLANG_LTR},
	{"yi",			nullptr, XAP_STRING_ID_LANG_YI,    UTLANG_RTL},
	{"zh-CN",		nullptr, XAP_STRING_ID_LANG_ZH_CN, UTLANG_VERTICAL},	// TODO also UTLANG_LTR
	{"zh-HK",		nullptr, XAP_STRING_ID_LANG_ZH_HK, UTLANG_VERTICAL},	// TODO also UTLANG_LTR
	{"zh-SG",		nullptr, XAP_STRING_ID_LANG_ZH_SG, UTLANG_VERTICAL},	// TODO also UTLANG_LTR
	{"zh-TW",		nullptr, XAP_STRING_ID_LANG_ZH_TW, UTLANG_VERTICAL},	// TODO also UTLANG_LTR
	{"zu",  		nullptr, XAP_STRING_ID_LANG_ZU,    UTLANG_LTR},
};

/*!
 Compare function used by qsort()

 \param a left side of comparison
 \param b right side of comparison
 \return negative, 0, or positive

 Special "no proofing" language will always be sorted to the
  top of the list
 */
static int s_compareQ(const void * a, const void *b)
{
	const UT_LangRecord * A = static_cast<const UT_LangRecord *>(a);
	const UT_LangRecord * B = static_cast<const UT_LangRecord *>(b);
#if 0
	// as long as bsearch is used searching for lang codes this is wrong
	if (B->m_nID == XAP_STRING_ID_LANG_0)
		return 1;
	else if (A->m_nID == XAP_STRING_ID_LANG_0)
		return -1;

	return g_utf8_collate(A->m_szLangName, B->m_szLangName);
#else

	return strcmp(A->m_szLangCode, B->m_szLangCode);
#endif
}

/*!
 Compare function used by bsearch()

 \param l langauge code
 \param e language table entry
 \return negative, 0, or positive
 */
static int s_compareB(const void * l, const void *e)
{
	const gchar * L   = static_cast<const gchar *>(l);
	const UT_LangRecord * E = static_cast<const UT_LangRecord *>(e);

#if 0
	// as long as bsearch is used searching for lang codes this is wrong
	if (E->m_nID == XAP_STRING_ID_LANG_0)
		return 1;
	else if (L == s_Table[0].m_szLangName)
		return -1;
	else if (strcmp(L, s_Table[0].m_szLangName) == 0)
		return -1;

	return g_utf8_collate(L, E->m_szLangName);
#else
	// make the comparison case insensitive to cope with buggy systems 
	return g_ascii_strcasecmp(L, E->m_szLangCode);
#endif
}

/*!
 Set the language name attribut in the language table. It takes the localized
 names from the available stringset. This function is supplied to set the names
 after the stringset is known to the application.
*/
void UT_Language_updateLanguageNames() 
{
    const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
    UT_return_if_fail(pSS);
    
    for(UT_uint32 i = 0; i < G_N_ELEMENTS(s_Table); i++)
    {
        s_Table[i].m_szLangName = pSS->getValue(s_Table[i].m_nID);
    }

    qsort(&s_Table[0], G_N_ELEMENTS(s_Table), sizeof(UT_LangRecord), s_compareQ);
}


bool UT_Language::s_Init = true;

/*!
 The constructor looks up the translations for the language code and sorts the table
  alphabetically by the language name
 */
UT_Language::UT_Language()
{
	if(s_Init) // do this once here
    {
        UT_Language_updateLanguageNames();
        s_Init = false;
    }
}

UT_uint32 UT_Language::getCount()
{
	return (G_N_ELEMENTS(s_Table));
}

const gchar * UT_Language::getNthLangCode(UT_uint32 n)
{
	return (s_Table[n].m_szLangCode);
}

const gchar * UT_Language::getNthLangName(UT_uint32 n)
{
	return (s_Table[n].m_szLangName);
}

UT_uint32 UT_Language::getNthId(UT_uint32 n)
{
	return (s_Table[n].m_nID);
}

const gchar * UT_Language::getCodeFromName(const gchar * szName)
{
	for(UT_uint32 i = 0; i < G_N_ELEMENTS(s_Table); i++)
	{
		// make the comparison case insensitive to cope with buggy systems 
		if(!g_ascii_strcasecmp(szName, s_Table[i].m_szLangName))
			return s_Table[i].m_szLangCode;
	}

	UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
	UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
				 "language to the tables\n", szName));
	return nullptr;
}

UT_uint32 UT_Language::getIndxFromCode(const gchar * szCode)
{
	for(UT_uint32 i = 0; i < G_N_ELEMENTS(s_Table); i++)
	{
		// make the comparison case insensitive to cope with buggy systems 
		if(!g_ascii_strcasecmp(szCode, s_Table[i].m_szLangCode))
			return i;
	}

	// see if our tables contain short version of this code, e.g., hr instead of hr-HR
	static gchar szShortCode[7];
	strncpy(szShortCode, szCode,6);
	szShortCode[6] = 0;

	char * dash = strchr(szShortCode, '-');
	if(dash)
	{
		*dash = 0;

		for(UT_uint32 i = 0; i < G_N_ELEMENTS(s_Table); i++)
		{
			// make the comparison case insensitive to cope with buggy systems 
			if(!g_ascii_strcasecmp(szShortCode, s_Table[i].m_szLangCode))
				return i;
		}
	}
		
	
	UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
	UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
				 "language to the tables\n", szCode));
	return 0;
}

UT_uint32 UT_Language::getIdFromCode(const gchar * szCode)
{
	const UT_LangRecord * e = getLangRecordFromCode(szCode);

	if(e)
		return e->m_nID;
	else
	{
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
					 "language to the tables\n", szCode));
		return 0;
	}
	
}

// This function is not as useless as might seem; it takes a pointer
// to a property string, finds the same property in the static table
// and returns the pointer to it. This is used by fp_TextRun to set its
// m_pLanguage member; by always refering into the static table it is
// possible to compare the language property by simply comparing the
// pointers, rather than having to use strcmp()

const gchar * UT_Language::getCodeFromCode(const gchar * szName)
{
	const UT_LangRecord * e = getLangRecordFromCode(szName);
	
	if(e)
		return e->m_szLangCode;
	else
	{
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
					 "language to the tables\n", szName));
		return nullptr;
	}
}

UT_LANGUAGE_DIR UT_Language::getDirFromCode(const gchar * szCode)
{
	const UT_LangRecord * e = getLangRecordFromCode(szCode);
	
	if(e)
		return e->m_eDir;
	else
	{
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
					 "language to the tables\n", szCode));
		return UTLANG_LTR;
	}
}

const UT_LangRecord * UT_Language::getLangRecordFromCode(const gchar * szCode)
{
	const UT_LangRecord * e = static_cast<UT_LangRecord *>(bsearch(szCode, s_Table,
																   G_N_ELEMENTS(s_Table),
																   sizeof(UT_LangRecord), s_compareB));
	if(!e)
	{
		// see if our tables contain short version of this code, e.g., hr instead of hr-HR
		static gchar szShortCode[7];
		strncpy(szShortCode, szCode,6);
		szShortCode[6] = 0;

		char * dash = strchr(szShortCode, '-');
		if(dash)
		{
			*dash = 0;
			e = static_cast<UT_LangRecord *>(bsearch(szShortCode, s_Table,
													 G_N_ELEMENTS(s_Table),
													 sizeof(UT_LangRecord), s_compareB));

			if(e)
				return e;
		}
		
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		UT_DEBUGMSG(("UT_Language: unknown language [%s]; if this message appears, add the "
					 "language to the tables\n", szCode));
		return nullptr;
	}

	return e;
}
	
