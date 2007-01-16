#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_QNXEncodingManager.h"

/************************************************************/

XAP_EncodingManager *XAP_EncodingManager::get_instance()
{
	if (_instance == 0)
	{
		UT_DEBUGMSG(("Building XAP_EncodingManager\n"));
		_instance = new XAP_QNXEncodingManager();
		_instance->initialize();
		UT_DEBUGMSG(("XAP_EncodingManager built\n"));
	}

	return _instance;
}

/************************************************************/

XAP_QNXEncodingManager::XAP_QNXEncodingManager() 
{
}

XAP_QNXEncodingManager::~XAP_QNXEncodingManager() {}

static const char * NativeEncodingName;
static const char * Native8BitEncodingName;
static const char * NativeUnicodeEncodingName;
static const char * LanguageISOName;
static const char * LanguageISOTerritory;

const char* XAP_QNXEncodingManager::getNativeEncodingName() const
{
     return NativeEncodingName; 
}

const char* XAP_QNXEncodingManager::getNative8BitEncodingName() const
{
     return Native8BitEncodingName; 
}

const char* XAP_QNXEncodingManager::getNativeUnicodeEncodingName() const
{
     return NativeUnicodeEncodingName; 
}

const char* XAP_QNXEncodingManager::getLanguageISOName() const
{
 	return LanguageISOName; 
}

const char* XAP_QNXEncodingManager::getLanguageISOTerritory() const
{
 	return LanguageISOTerritory; 
}


void  XAP_QNXEncodingManager::initialize()
{
	char *ABLANG;
	char *val;
	char *lang;
	Native8BitEncodingName = "ISO-8859-1";
  NativeEncodingName = "UTF-8";
	NativeUnicodeEncodingName = "UTF-8";
	LanguageISOName = "en";
	LanguageISOTerritory = "US";


	if((ABLANG=getenv("ABLANG")))
	{
	lang=g_strdup(ABLANG);
			if((val=strsep(&lang,"_")))
				{
					LanguageISOName=val;
					LanguageISOTerritory=lang;
				}
	}
	XAP_EncodingManager::initialize();
	describe();
};


