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
 


// TODO change this file to not reference GR_Graphics.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_units.h"
#include "gr_Graphics.h"

const char * UT_convertToDimensionString(UT_Dimension dim, double value)
{
	// return pointer to static buffer -- use it quickly.

	// TODO what should the decimal precision of each different
	// TODO unit of measurement be ??
	
	static char buf[100];
	switch (dim)
	{
	case DIM_IN:
		// (1/16th (0.0625) is smallest unit the ui will
		// let them enter (via the TopRuler), so let's
		// set the precision so that we get nice roundoff.
		// TODO we may need to improve this later.
		sprintf(buf,"%.4fin",value);
		break;

	case DIM_CM:
		sprintf(buf,"%.1fcm",(value * 2.54));
		break;

	case DIM_PI:
		sprintf(buf,"%.0fpi",(value * 6));
		break;

	case DIM_PT:
		sprintf(buf,"%.0fpt",(value * 72));
		break;

	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		break;
	}
	
	return buf;
}


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
	while ((*p) && (isdigit(*p) || (*p == '-') || (*p == '.')))
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
		else if (0 == UT_stricmp(p, "pi"))
		{
			result = f / 6;
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

double UT_convertToPoints(const char* s)
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
		if (0 == UT_stricmp(p, "pt"))
		{
			result = f;
		}
		else if (0 == UT_stricmp(p, "pi"))
		{
			result = f * 12;	// ie, 72 / 6
		}
		else if (0 == UT_stricmp(p, "in"))
		{
			result = f * 72;
		}
		else if (0 == UT_stricmp(p, "cm"))
		{
			result = f * 72 / 2.54;
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

UT_sint32 UT_docUnitsFromPaperUnits(GR_Graphics * pG, UT_sint32 iPaperUnits)
{
	// convert number in paper units (see above) into
	// "document" units in the given graphics context.

	UT_ASSERT(pG);

	return (pG->getResolution() * iPaperUnits / UT_PAPER_UNITS_PER_INCH);
}
