#include "ap_Strings_wrapper.h"

AP_StringSet *strings;

void ap_strings_create(const char *szDomain)
{
  strings = new AP_StringSet(NULL, szDomain);
}

void ap_strings_destroy()
{
  delete strings;
}

const char * ap_strings_get_value(const char * szData)
{
  return strings->getValue(szData);
}
