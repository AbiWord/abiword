#ifndef IE_EXP_HTML_UTIL_H
#define IE_EXP_HTML_UTIL_H

#include <ut_string_class.h>
#include <ut_types.h>

#define MYEOL "\n"

extern const char * s_prop_list[];
extern const UT_uint32 s_PropListLen;
extern const char * s_DTD_XHTML_AWML;
extern const char * s_DTD_XHTML;
extern const char * s_DTD_HTML4;
extern const char * s_Delimiter;
extern const char * s_HeaderCompact;
extern bool m_bSecondPass;
extern bool m_bInAFENote;
extern bool m_bInAnnotation;
extern UT_UTF8String sMathSVGScript;

extern const char * s_Header[2];

UT_UTF8String s_string_to_url (const UT_String & str);
UT_UTF8String s_string_to_url (const UT_UTF8String & str);
bool is_CSS (const char * prop_name, const char ** prop_default = 0);
char * s_removeWhiteSpace (const char * text, UT_UTF8String & utf8str,
								  bool bLowerCase = true);



#endif