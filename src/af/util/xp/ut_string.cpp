
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_string.h"

UT_sint32 UT_stricmp(const char *s1, const char *s2)
{
#ifdef WINNT
	return stricmp(s1,s2);
#else
	return strcasecmp(s1,s2);
#endif
}

UT_Bool UT_cloneString(char *& rszDest, const char * szSource)
{
	if (szSource && *szSource)
		rszDest = strdup(szSource);
	else
		rszDest = NULL;
	return UT_TRUE;
}

UT_Bool UT_replaceString(char *& rszDest, const char * szSource)
{
	if (rszDest)
		free(rszDest);
	rszDest = NULL;

	return UT_cloneString(rszDest,szSource);
}
