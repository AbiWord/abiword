/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 


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

UT_uint32 UT_XML_strlen(const XML_Char * sz)
{
	if (!sz || !*sz)
		return 0;

	UT_uint32 k = 0;
	while (sz[k])
		k++;

	return k;
}

UT_Bool UT_XML_cloneString(XML_Char *& rszDest, const XML_Char * szSource)
{
	UT_uint32 length = UT_XML_strlen(szSource) + 1;
	rszDest = (XML_Char *)calloc(length,sizeof(XML_Char));
	if (!rszDest)
		return UT_FALSE;
	memmove(rszDest,szSource,length*sizeof(XML_Char));
	return UT_TRUE;
}

UT_sint32 UT_XML_stricmp(const XML_Char * sz1, const XML_Char * sz2)
{
	UT_ASSERT(sizeof(char) == sizeof(XML_Char));
	return UT_stricmp(sz1,sz2);
}
