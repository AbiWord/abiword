#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Win32Locale.h"
#include "xap_Win32EncodingManager.h"

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

static const char* NativeEncodingName, *NativeUnicodeEncodingName, *LanguageISOName, *LanguageISOTerritory;

const char* XAP_Win32EncodingManager::getNativeEncodingName() const
{     return NativeEncodingName; };

const char* XAP_Win32EncodingManager::getNativeUnicodeEncodingName() const
{     return NativeUnicodeEncodingName; };

const char* XAP_Win32EncodingManager::getLanguageISOName() const
{ 	return LanguageISOName; };

const char* XAP_Win32EncodingManager::getLanguageISOTerritory() const
{ 	return LanguageISOTerritory; };


void  XAP_Win32EncodingManager::initialize()
{
	char szLocaleInfo[64];
	static char szCodepage[64];
	static char szLanguage[64];
	static char szTerritory[64];
	bool bNorwaySpecialCase = false;

	NativeEncodingName = "CP1252";
	LanguageISOName = "en";
	LanguageISOTerritory = NULL;

	// Unicode Encoding Name
	// TODO Does NT use UCS-2BE internally on non-Intel CPUs?
	NativeUnicodeEncodingName = getUCS2LEName();

	// Encoding 
	if (GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IDEFAULTANSICODEPAGE,szLocaleInfo,sizeof(szLocaleInfo)/sizeof(szLocaleInfo[0])))
	{
		// Windows Unicode locale?
		if (!strcmp(szLocaleInfo,"0"))
		{
			NativeEncodingName = NativeUnicodeEncodingName;
			m_bIsUnicodeLocale = true;
		}
		else
		{
			szCodepage[0] = 'C';
			szCodepage[1] = 'P';
			strcpy(szCodepage+2,szLocaleInfo);
			NativeEncodingName = szCodepage;
			m_bIsUnicodeLocale = false;
		}
	}

	if (UT_getISO639Language(szLanguage))
	{
		LanguageISOName = szLanguage;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	if (UT_getISO3166Country(szTerritory))
	{
		LanguageISOTerritory = szTerritory;
	}

	XAP_EncodingManager::initialize();
	describe();
};


