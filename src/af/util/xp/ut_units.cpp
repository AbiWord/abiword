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
#include <locale.h>

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_debugmsg.h"
#include "gr_Graphics.h"

const char * UT_dimensionName(UT_Dimension dim)
{
	switch (dim)
	{
	case DIM_IN:
		return "in";

	case DIM_CM:
		return "cm";

	case DIM_PI:
		return "pi";

	case DIM_PT:
		return "pt";

	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		return "in";
	}
}

UT_Dimension UT_determineDimension(const char * sz, UT_Dimension fallback)
{
	if (UT_stricmp(sz,"in") == 0)
		return DIM_IN;

	if (UT_stricmp(sz,"cm") == 0)
		return DIM_CM;
	
	if (UT_stricmp(sz,"pi") == 0)
		return DIM_PI;

	if (UT_stricmp(sz,"pt") == 0)
		return DIM_PT;

	return fallback;
}

const char * UT_convertToDimensionString(UT_Dimension dim, double value, const char * szPrecision)
{
	// return pointer to static buffer -- use it quickly.
	//
	// We temporarily force the locale back to english so that
	// we get a period as the decimal point.

	// TODO what should the decimal precision of each different
	// TODO unit of measurement be ??
	
	static char buf[100];
	char bufFormat[100];
	double valueScaled;
	
	switch (dim)
	{
	case DIM_IN:
		// (1/16th (0.0625) is smallest unit the ui will
		// let them enter (via the TopRuler), so let's
		// set the precision so that we get nice roundoff.
		// TODO we may need to improve this later.
		valueScaled = value;
		sprintf(bufFormat,"%%%sfin",((szPrecision && *szPrecision) ? szPrecision : ".4"));
		break;

	case DIM_CM:
		valueScaled = (value * 2.54);
		sprintf(bufFormat,"%%%sfcm",((szPrecision && *szPrecision) ? szPrecision : ".1"));
		break;

	case DIM_PI:
		valueScaled = (value * 6);
		sprintf(bufFormat,"%%%sfpi",((szPrecision && *szPrecision) ? szPrecision : ".0"));
		break;

	case DIM_PT:
		valueScaled = (value * 72);
		sprintf(bufFormat,"%%%sfpt",((szPrecision && *szPrecision) ? szPrecision : ".0"));
		break;

	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		valueScaled = value;
		sprintf(bufFormat,"%%%sf",((szPrecision && *szPrecision) ? szPrecision : ""));
		break;
	}
	setlocale(LC_NUMERIC,"C");
	sprintf(buf,bufFormat,value);
	//UT_DEBUGMSG(("ConvertToDimensionString: [%g] --> [%s]\n",valueScaled,buf));
	setlocale(LC_NUMERIC,""); // restore original locale
	
	return buf;
}


double UT_convertToInches(const char* s)
{
	// NOTE: we explicitly use a period '.' as a decimal place
	// NOTE: and assume that the locale is set to english.
	// NOTE: all other places where we deal with these values
	// NOTE: are wrapped with locale code.
	
	double result = 0;

	if (!s || !*s)
		return 0;

	double f = UT_convertDimensionless(s);
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
			result = f;					// unknown unit -- pass thru as is ??
		}
	}

	return result;
}

double UT_convertToPoints(const char* s)
{
	double result = 0;

	if (!s || !*s)
		return 0;

	double f = UT_convertDimensionless(s);
	const char *p = s;
	while ((*p) && (isdigit(*p) || (*p == '-') || (*p == '.')))
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
			result = f;					// unknown unit -- pass thru as is ??
		}
	}

	return result;
}

double UT_convertDimensionless(const char * sz)
{
	// convert given string into a number -- without using any dimension
	// info that may be in the string.
	//
	// normally we would just use atof(), but that is locale-aware and
	// we want our file format to be locale-independent and thus portable.
	// this means that anything we do internally (eg top ruler gadgets),
	// needs to be in this convention.
	//
	// we can let the GUI do locale-specific conversions for presentation
	// in dialogs and etc.

	setlocale(LC_NUMERIC,"C");
	double f = atof(sz);
	setlocale(LC_NUMERIC,"");

	//UT_DEBUGMSG(("ConvertDimensionless: [%s] --> [%f]\n", sz, f));
	return f;
}

const char * UT_convertToDimensionlessString(double value, const char * szPrecision)
{
	// return pointer to static buffer -- use it quickly.
	//
	// We temporarily force the locale back to english so that
	// we get a period as the decimal point.

	static char buf[100];

	char bufFormat[100];
	sprintf(bufFormat,"%%%sf",((szPrecision && *szPrecision) ? szPrecision : ""));
	
	setlocale(LC_NUMERIC,"C");
	sprintf(buf,bufFormat,value);
	//UT_DEBUGMSG(("ConvertToDimensionlessString: [%g] --> [%s]\n",value,buf));
	setlocale(LC_NUMERIC,""); // restore original locale
	
	return buf;
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
