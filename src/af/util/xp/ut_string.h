#ifndef UT_STRING_H
#define UT_STRING_H

#include "ut_types.h"

NSPR_BEGIN_EXTERN_C

UT_sint32 UT_stricmp(const char *s1, const char *s2);
UT_Bool UT_cloneString(char *& rszDest, const char * szSource);
UT_Bool UT_replaceString(char *& rszDest, const char * szSource);

NSPR_END_EXTERN_C

#endif /* UT_STRING_H */
