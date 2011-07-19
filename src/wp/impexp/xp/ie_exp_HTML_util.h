#ifndef IE_EXP_HTML_UTIL_H
#define	IE_EXP_HTML_UTIL_H


#include <ut_string_class.h>

bool is_CSS(const char * prop_name, const char ** prop_default = 0);
char * s_removeWhiteSpace(const char * text, UT_UTF8String & utf8str,
        bool bLowerCase = true);
UT_UTF8String s_string_to_url(const UT_UTF8String & str);
UT_UTF8String s_string_to_url(const UT_String & str);

#endif	/* IE_EXP_HTML_UTIL_H */

