#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Win32EncodingManager.h"

// TODO This data is defined in ap_Win32Prefs.cpp

extern char s_ISO3166_2_and_3[1190];

/************************************************************/

XAP_EncodingManager *XAP_EncodingManager::get_instance()
{
	if (_instance == 0)
	{
		UT_DEBUGMSG(("Building XAP_EncodingManager\n"));
		_instance = new XAP_Win32EncodingManager();
		_instance->initialize();
		UT_DEBUGMSG(("XAP_EncodingManager built\n"));
	}

	return _instance;
}

/************************************************************/

XAP_Win32EncodingManager::XAP_Win32EncodingManager() 
{
}

XAP_Win32EncodingManager::~XAP_Win32EncodingManager() {}

static const char* NativeEncodingName, *LanguageISOName, *LanguageISOTerritory;

const char* XAP_Win32EncodingManager::getNativeEncodingName() const
{     return NativeEncodingName; };

const char* XAP_Win32EncodingManager::getLanguageISOName() const
{ 	return LanguageISOName; };

const char* XAP_Win32EncodingManager::getLanguageISOTerritory() const
{ 	return LanguageISOTerritory; };


void  XAP_Win32EncodingManager::initialize()
{
	char szLocaleInfo[64];
	static char szLanguage[64];
	static char szTerritory[64];
	bool bNorwaySpecialCase = false;

	NativeEncodingName = "ISO-8859-1";
	LanguageISOName = "en";
	LanguageISOTerritory = NULL;

	// Encoding 
	if (GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IDEFAULTANSICODEPAGE,szLocaleInfo,sizeof(szLocaleInfo)/sizeof(szLocaleInfo[0])))
	{
		// Windows Unicode locale?
		if (!strcmp(szLocaleInfo,"0"))
		{
			NativeEncodingName = "UCS-2-INTERNAL";	// As in ev_Win32Keyboard.cpp
			m_bIsUnicodeLocale = true;
		}
		else
		{
			NativeEncodingName = charsetFromCodepage(atoi(szLocaleInfo));
			m_bIsUnicodeLocale = false;
		}
	}

	// Language
	if (GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SABBREVLANGNAME,szLanguage,sizeof(szLanguage)/sizeof(szLanguage[0])))
	{
		if (!strcmp(szLanguage,"Non"))		// Special case: Nynorsk in Norway
			bNorwaySpecialCase = true;		// As in ap_Win32Prefs.cpp

		szLanguage[0] = tolower(szLanguage[0]);
		szLanguage[1] = tolower(szLanguage[1]);
		szLanguage[2] = '\0';
		LanguageISOName = szLanguage;
	}

	// Territory
	if (bNorwaySpecialCase == true)			// Special case: Nynorsk in Norway
		LanguageISOTerritory = "NYNORSK";	// As in ap_Win32Prefs.cpp
	else if (GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SABBREVCTRYNAME,szLocaleInfo,sizeof(szLocaleInfo)/sizeof(szLocaleInfo[0])))
	{
		// TODO This code is copied from ap_Win32Prefs.cpp
		char *psz;

		for (psz = s_ISO3166_2_and_3; *psz != '\0'; psz += 5 )
			if (!strncmp(&psz[2],szLocaleInfo,3))
				break;

		strncpy(szTerritory, psz, 2 );
		szTerritory[2] = '\0';
		LanguageISOTerritory = szTerritory;
	}

	XAP_EncodingManager::initialize();
	describe();
};


