/* AbiSource Program Utilities
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



#include <windows.h>
#include <ctype.h>

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_locale.h"
#include "ut_Win32Locale.h"

// Codes from ISO 3166

// Very compressed form. Two char code followed by three char code.

static char s_ISO3166_2_and_3[] =
"AFAFGALALBDZDZAASASMADANDAOAGOAIAIAAQATAAGATGARARGAMARMAWABWAUAUSATAUT"
"AZAZEBSBHSBHBHRBDBGDBBBRBBYBLRBEBELBZBLZBJBENBMBMUBTBTNBOBOLBABIHBWBWA"
"BVBVTBRBRAIOIOTBNBRNBGBGRBFBFABIBDIKHKHMCMCMRCACANCVCPVKYCYMCFCAFTDTCD"
"CLCHLCNCHNCXCXRCCCCKCOCOLKMCOMCGCOGCKCOKCRCRICICIVHRHRVCUCUBCYCYPCZCZE"
"DKDNKDJDJIDMDMADODOMTPTMPECECUEGEGYSVSLVGQGNQERERIEEESTETETHFKFLKFOFRO"
"FJFJIFIFINFRFRAFXFXXGFGUFPFPYFTFATFGAGABGMGMBGEGEODEDEUGHGHAGIGIBGRGRC"
"GLGRLGDGRDGPGLPGUGUMGTGTMGNGINGWGNBGYGUYHTHTIHMHMDHNHNDHKHKGHUHUNISISL"
"ININDIDIDNIRIRNIQIRQIEIRLILISRITITAJMJAMJPJPNJOJORKZKAZKEKENKIKIRKPPRK"
"KRKORKWKWTKGKGZLALAOLVLVALBLBNLSLSOLRLBRLYLBYLILIELTLTULULUXMOMACMKMKD"
"MGMDGMWMWIMYMYSMVMDVMLMLIMTMLTMHMHLMQMTQMRMRTMUMUSYTMYTMXMEXFMFSMMDMDA"
"MCMCOMNMNGMSMSRMAMARMZMOZMMMMRNANAMNRNRUNPNPLNLNLDANANTNCNCLNZNZLNINIC"
"NENERNGNGANUNIUNFNFKMPMNPNONOROMOMNPKPAKPWPLWPAPANPGPNGPYPRYPEPERPHPHL"
"PNPCNPLPOLPTPRTPRPRIQAQATREREUROROMRURUSRWRWAKNKNALCLCAVCVCTWSWSMSMSMR"
"STSTPSASAUSNSENSCSYCSLSLESGSGPSKSVKSISVNSBSLBSOSOMZAZAFESESPLKLKASHSHN"
"PMSPMSDSDNSRSURSJSJMSZSWZSESWECHCHESYSYRTWTWNTJTJKTZTZATHTHATGTGOTKTKL"
"TOTONTTTTOTNTUNTRTURTMTKMTCTCATVTUVUGUGAUAUKRAEAREGBGBRUSUSAUMUMIUYURY"
"UZUZBVUVUTVAVATVEVENVNVNMVGVGBVIVIRWFWLFEHESHYEYEMYUYUGZRZARZMZMBZWZWE";

bool UT_getISO639Language(char * szLanguage)
{
	bool bSuccess = false;
	char szWinLang[4] = "", szISOLang[3] = "";

	UT_ASSERT(szLanguage);

	GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_SABBREVLANGNAME,szWinLang,4); //!TODO Using ANSI function
	GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_SISO639LANGNAME,szISOLang,3); //!TODO Using ANSI function

	// TODO Even Windows 2000 returns ISO 639-1 instead of ISO 639-2
	// TODO Make sure we return correct ISO 639-2 for at least the locales Abi supports.
	if (*szISOLang)
	{
		// Fix Norwegian
		if (!strcmp(szISOLang,"no"))
		{
			if (*szWinLang && !g_ascii_strcasecmp(szWinLang,"NON"))
				strcpy(szISOLang,"nn");
			else if (*szWinLang && !g_ascii_strcasecmp(szWinLang,"NOR"))
				strcpy(szISOLang,"nb");
		}
		// Fix Hebrew
		else if (!g_ascii_strcasecmp(szISOLang, "iw"))
		{
			strcpy(szISOLang, "he");
		}

		strcpy(szLanguage,szISOLang);
		bSuccess = true;
	}
	else if (*szWinLang)
	{
		// Convert to ISO 639
		// TODO Handle any other special cases
		// Fix Norwegian
		if (!g_ascii_strcasecmp(szWinLang,"NON"))
			strcpy(szLanguage,"nn");
		else if (!g_ascii_strcasecmp(szWinLang,"NOR"))
			strcpy(szLanguage,"nb");
		// Fix Chinese codes
		else if (!g_ascii_strncasecmp(szWinLang,"CH",2))
			strcpy(szLanguage,"zh");
		// Fix Japanese code
		else if (!g_ascii_strcasecmp(szWinLang,"JPN"))
			strcpy(szLanguage,"ja");
		else
		{
			szLanguage[0] = tolower(szWinLang[0]);
			szLanguage[1] = tolower(szWinLang[1]);
			szLanguage[2] = '\0';
		}
		bSuccess = true;
	}

	return bSuccess;
}

bool UT_getISO3166Country(char *szCountry)
{
	bool bSuccess = false;
	char szTmp[4];

	UT_ASSERT(szCountry);

	if (GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_SISO3166CTRYNAME,szCountry,3)) //!TODO Using ANSI function
	{
		// Fix Serbia
		if (!g_ascii_strcasecmp(szCountry, "SP"))
		{
			strcpy(szCountry, "SR");
		}

		bSuccess = true;
	}
	else if (GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_SABBREVCTRYNAME,szTmp,4)) //!TODO Using ANSI function
	{
		// Convert to ISO 3166
		char *psz;

		for ( psz = s_ISO3166_2_and_3; *psz != '\0'; psz += 5)
			if (!strncmp(&psz[2],szTmp,3))
				break;

		if (psz)
		{
			strncpy(szCountry,psz,2);
			szCountry[2] = '\0';
			bSuccess = true;
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}

	return bSuccess;
}
