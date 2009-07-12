#include "ap_Strings_wrapper.h"
#include "xap_App.h"

XAP_StringSet * strings;

void xap_strings_create(const char * szDomain)
{
	if (!strings)
  	strings = (XAP_StringSet *) XAP_App::getApp()->getStringSet();
	
	strings->setDomain(szDomain);
}

void xap_strings_destroy()
{
}

const char * xap_strings_get_value(const char * szData)
{
  return strings->getValue(szData);
}
