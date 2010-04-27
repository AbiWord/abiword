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

static const char * NativeEncodingName;
static const char * NativeSystemEncodingName;
static const char * Native8BitEncodingName;
static const char * NativeNonUnicodeEncodingName;
static const char * NativeUnicodeEncodingName;
static const char * LanguageISOName;
static const char * LanguageISOTerritory;

const char* XAP_Win32EncodingManager::getNativeEncodingName() const
{     return NativeEncodingName; }

const char* XAP_Win32EncodingManager::getNativeSystemEncodingName() const
{     return NativeSystemEncodingName; }

const char* XAP_Win32EncodingManager::getNative8BitEncodingName() const
{     return Native8BitEncodingName; }

const char* XAP_Win32EncodingManager::getNativeNonUnicodeEncodingName() const
{     return NativeNonUnicodeEncodingName; }

const char* XAP_Win32EncodingManager::getNativeUnicodeEncodingName() const
{     return NativeUnicodeEncodingName; }

const char* XAP_Win32EncodingManager::getLanguageISOName() const
{ 	return LanguageISOName; }

const char* XAP_Win32EncodingManager::getLanguageISOTerritory() const
{ 	return LanguageISOTerritory; }


void  XAP_Win32EncodingManager::initialize()
{
	char szLocaleInfo[64];
	static char szCodepage[64];
	static char szSystemCodepage[64];
	static char szLanguage[64];
	static char szTerritory[64];

	NativeNonUnicodeEncodingName = Native8BitEncodingName = NativeSystemEncodingName = NativeEncodingName = "CP1252";
	LanguageISOName = "en";
	LanguageISOTerritory = NULL;

	XAP_EncodingManager::initialize();

	// Unicode Encoding Name
	// TODO Does NT use UCS-2BE internally on non-Intel CPUs?
	// TODO Windows 2000 and XP use UTF-16 but on 2000 it may involve a
	// TODO  registry setting
	NativeUnicodeEncodingName = getUCS2LEName();

	// Encodings
	// User Encoding (Set via Region/Locale; does not require reboot)
	if (GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_IDEFAULTANSICODEPAGE,szLocaleInfo,sizeof(szLocaleInfo)/sizeof(szLocaleInfo[0])))
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
			NativeNonUnicodeEncodingName = Native8BitEncodingName = NativeEncodingName = szCodepage;
			m_bIsUnicodeLocale = false;
		}
	}
	// System Encoding (Used by GUI,DOS; Set via Region/Default Language; requires reboot)
	if (GetLocaleInfoA(LOCALE_SYSTEM_DEFAULT,LOCALE_IDEFAULTANSICODEPAGE,szLocaleInfo,sizeof(szLocaleInfo)/sizeof(szLocaleInfo[0])))
	{
		// Windows Unicode locale?
		if (!strcmp(szLocaleInfo,"0"))
		{
			NativeSystemEncodingName = NativeUnicodeEncodingName;
			//m_bIsUnicodeLocale = true;
		}
		else
		{
			szSystemCodepage[0] = 'C';
			szSystemCodepage[1] = 'P';
			strcpy(szSystemCodepage+2,szLocaleInfo);
			NativeSystemEncodingName = szSystemCodepage;
			//m_bIsUnicodeLocale = false;
		}
	}

    m_bIsUnicodeLocale = true;
	NativeEncodingName = "UCS-2LE";	
	NativeSystemEncodingName = "UCS-2LE";

	if (UT_getISO639Language(szLanguage))
	{
		LanguageISOName = szLanguage;
	}
	else
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}
	if (UT_getISO3166Country(szTerritory))
	{
		LanguageISOTerritory = szTerritory;
	}

	describe();
}


