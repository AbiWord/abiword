/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 
 


// TODO change this file to not reference DG_Graphics.

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_units.h"
#include "dg_Graphics.h"


double UT_convertToInches(const char* s)
{
	double result = 0;

	if (!s || !*s)
		return 0;

	/*
		TODO later, this routine needs to handle more units.
	*/

	double f = atof(s);
	const char *p = s;
	while ((*p) && (isdigit(*p) || (*p == '.')))
	{
		p++;
	}

	// p should now point to the unit
	UT_ASSERT(*p);
	if (*p)
	{
		if (0 == UT_stricmp(p, "in"))
		{
			result = f;
		}
		else if (0 == UT_stricmp(p, "pt"))
		{
			result = f / 72;
		}
		else if (0 == UT_stricmp(p, "cm"))
		{
			result = f / 2.54;
		}
		else
		{
			UT_ASSERT(0);
			// unknown unit
		}
	}

	return result;
}

UT_sint32 UT_paperUnits(const char * sz)
{
	// convert string in form "8.5in" into "paper" units.
	// paper units are a relatively low-resolution (say
	// 1/100 inch) but are suitable for specifying margins,
	// etc. -- stuff relative to the actual paper.

	if (!sz || !*sz)
		return 0;

	double dInches = UT_convertToInches(sz);
	double dResolution = UT_PAPER_UNITS_PER_INCH;

	return (UT_sint32)(dInches * dResolution);
}

UT_sint32 UT_docUnitsFromPaperUnits(DG_Graphics * pG, UT_sint32 iPaperUnits)
{
	// convert number in paper units (see above) into
	// "document" units in the given graphics context.

	UT_ASSERT(pG);

	return (pG->getResolution() * iPaperUnits / UT_PAPER_UNITS_PER_INCH);
}
