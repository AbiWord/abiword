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

	case DIM_MM:
	   	return "mm";
	   
	case DIM_PI:
		return "pi";

	case DIM_PT:
		return "pt";

	case DIM_PX:
	   	return "px";
	   
	case DIM_PERCENT:
		return "%";
	   
	case DIM_none:
		return "";

	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		return "in";
	}
}

#define STR_COMPARE UT_strcmp

UT_Dimension UT_determineDimension(const char * sz, UT_Dimension fallback)
{
	const char *p = sz;
	while ((*p) && (isdigit(*p) || (*p == '-') || (*p == '.')))
	{
		p++;
	}

	// p should now point to the unit
	if (*p)
	{
		if (STR_COMPARE(p,"in") == 0)
			return DIM_IN;

		if (STR_COMPARE(p,"cm") == 0)
			return DIM_CM;

	   	if (STR_COMPARE(p,"mm") == 0)
			return DIM_MM;

		if (STR_COMPARE(p,"pi") == 0)
			return DIM_PI;

		if (STR_COMPARE(p,"pt") == 0)
			return DIM_PT;

	   	if (STR_COMPARE(p,"px") == 0)
			return DIM_PX;

	   	if (STR_COMPARE(p,"%") == 0)
			return DIM_PERCENT;
	   
		UT_ASSERT(UT_TODO);
	}

	return fallback;
}

double UT_convertInchesToDimension(double inches, UT_Dimension dim)
{
	double valueScaled = inches;
	
	switch (dim)
	{
	case DIM_IN:	valueScaled = inches;			break;
	case DIM_CM:	valueScaled = (inches * 2.54);	break;
	case DIM_MM:    valueScaled = (inches * 25.4);  break;
	case DIM_PI:	valueScaled = (inches * 6);		break;
	case DIM_PT:	valueScaled = (inches * 72);	break;
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		break;
	}

	return valueScaled;
}

const char * UT_convertInchesToDimensionString(UT_Dimension dim, double valueInInches, const char * szPrecision)
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
		valueScaled = valueInInches;
		sprintf(bufFormat,"%%%sfin",((szPrecision && *szPrecision) ? szPrecision : ".4"));
		break;

	case DIM_CM:
		valueScaled = (valueInInches * 2.54);
		sprintf(bufFormat,"%%%sfcm",((szPrecision && *szPrecision) ? szPrecision : ".2"));
		break;

	case DIM_MM:
		valueScaled = (valueInInches * 25.4);
		sprintf(bufFormat,"%%%sfmm",((szPrecision && *szPrecision) ? szPrecision : ".1"));
		break;

	case DIM_PI:
		valueScaled = (valueInInches * 6);
		sprintf(bufFormat,"%%%sfpi",((szPrecision && *szPrecision) ? szPrecision : ".0"));
		break;

	case DIM_PT:
		valueScaled = (valueInInches * 72);
		sprintf(bufFormat,"%%%sfpt",((szPrecision && *szPrecision) ? szPrecision : ".0"));
		break;

	case DIM_PX:
 	case DIM_none:
		valueScaled = valueInInches;
		sprintf(bufFormat,"%%%sf",((szPrecision && *szPrecision) ? szPrecision : ""));
		break;

	case DIM_PERCENT:
		valueScaled = valueInInches;
		sprintf(bufFormat,"%%%sf%%",((szPrecision && *szPrecision) ? szPrecision : ""));
		break;

	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		valueScaled = valueInInches;
		sprintf(bufFormat,"%%%sf",((szPrecision && *szPrecision) ? szPrecision : ""));
		break;
	}
	setlocale(LC_NUMERIC,"C");
	sprintf(buf,bufFormat,valueScaled);
	//UT_DEBUGMSG(("ConvertToDimensionString: [%g] --> [%s]\n",valueScaled,buf));
	setlocale(LC_NUMERIC,""); // restore original locale
	
	return buf;
}

const char * UT_formatDimensionString(UT_Dimension dim, double value, const char * szPrecision)
{
	// return pointer to static buffer -- use it quickly.
	//
	// We temporarily force the locale back to english so that
	// we get a period as the decimal point.

	// TODO what should the decimal precision of each different
	// TODO unit of measurement be ??
	
	static char buf[100];
	char bufFormat[100];
	
	switch (dim)
	{
	case DIM_IN:
		// (1/16th (0.0625) is smallest unit the ui will
		// let them enter (via the TopRuler), so let's
		// set the precision so that we get nice roundoff.
		// TODO we may need to improve this later.
		sprintf(bufFormat,"%%%sfin",((szPrecision && *szPrecision) ? szPrecision : ".4"));
		break;

	case DIM_CM:
		sprintf(bufFormat,"%%%sfcm",((szPrecision && *szPrecision) ? szPrecision : ".2"));
		break;

	case DIM_MM:
		sprintf(bufFormat,"%%%sfmm",((szPrecision && *szPrecision) ? szPrecision : ".1"));
		break;

	case DIM_PI:
		sprintf(bufFormat,"%%%sfpi",((szPrecision && *szPrecision) ? szPrecision : ".0"));
		break;

	case DIM_PT:
		sprintf(bufFormat,"%%%sfpt",((szPrecision && *szPrecision) ? szPrecision : ".0"));
		break;

	case DIM_PX:
 	case DIM_none:
		sprintf(bufFormat,"%%%sf",((szPrecision && *szPrecision) ? szPrecision : ""));
		break;

	case DIM_PERCENT:
		sprintf(bufFormat,"%%%sf%%",((szPrecision && *szPrecision) ? szPrecision : ""));
		break;

	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		sprintf(bufFormat,"%%%sf",((szPrecision && *szPrecision) ? szPrecision : ""));
		break;
	}
	setlocale(LC_NUMERIC,"C");
	sprintf(buf,bufFormat,value);
	//UT_DEBUGMSG(("ConvertToDimensionString: [%g] --> [%s]\n",valueScaled,buf));
	setlocale(LC_NUMERIC,""); // restore original locale
	
	return buf;
}

const char * UT_reformatDimensionString(UT_Dimension dim, const char *sz, const char * szPrecision)
{
	double d = UT_convertDimensionless(sz);

	// if needed, switch unit systems and round off
	UT_Dimension dimOld = UT_determineDimension(sz, dim);

	if (dimOld != dim)
	{
		double dInches = UT_convertToInches(sz);
		d = UT_convertInchesToDimension(dInches, dim); 
	}

	return UT_formatDimensionString(dim, d);
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
	
	if (f == 0)
	    return 0;
	
	const char *p = s;
	while ((*p) && (isdigit(*p) || (*p == '-') || (*p == '.') || isspace(*p)))
	{
		p++;
	}

	// p should now point to the unit
	UT_ASSERT(*p);
	if(*p)
	  {
	    UT_Dimension dim = UT_determineDimension(p, (UT_Dimension)-1);
	    result = UT_convertDimToInches (f, dim);
	  }

	return result;
}

double UT_convertDimToInches (double f, UT_Dimension dim)
{
  double result = 0.0;
  switch(dim)
    {
    case DIM_IN: result = f;        break;
    case DIM_PI: result = f / 6;    break;
    case DIM_PT: result = f / 72;   break;
    case DIM_CM: result = f / 2.54; break;
    case DIM_MM: result = f / 25.4; break;
    default:
      UT_DEBUGMSG(("Unknown dimension type: %d", dim));
      UT_ASSERT(0);
      result = f;
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
	while ((*p) && (isdigit(*p) || (*p == '-') || (*p == '.') || isspace(*p)))
	{
		p++;
	}

	// p should now point to the unit
	UT_ASSERT(*p);
	if (*p)
	{
	    UT_Dimension dim = UT_determineDimension(p, (UT_Dimension)-1);

	    switch(dim)
	      {
	      case DIM_PT: result = f;             break;
	      case DIM_PI: result = f * 12;        break; // ie, 72 / 6
	      case DIM_IN: result = f * 72;        break;
	      case DIM_CM: result = f * 72 / 2.54; break;
	      case DIM_MM: result = f * 72 / 25.4; break;
	      default:
		UT_DEBUGMSG(("Unknown dimension type: %s", p));
		UT_ASSERT(0);
		result = f;
	      }
	}

	return result;
}

UT_sint32 UT_convertToLayoutUnits(const char* s)
{
	return (UT_sint32)(UT_convertToInches(s) * UT_LAYOUT_UNITS);
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

UT_Bool UT_hasDimensionComponent(const char * sz)
{
	// TODO : check against known units instead of taking any
	// TODO : ASCII chars after a number as a sign of units.
	
	if (!sz)
		return UT_FALSE;
	
	const char *p = sz;
	while ((*p) && (isdigit(*p) || (*p == '-') || (*p == '.')))
	{
		p++;
	}

	// if we landed on non-NULL, unit component
	if(*p)
		return UT_TRUE;
	else
		return UT_FALSE;
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

UT_sint32 UT_layoutUnitsFromPaperUnits(UT_sint32 iPaperUnits)
{
	// convert number in paper units (see above) into
	// "layout" units.

	return (UT_LAYOUT_UNITS * iPaperUnits / UT_PAPER_UNITS_PER_INCH);
}

UT_sint32 UT_paperUnitsFromLayoutUnits(UT_sint32 iLayoutUnits)
{
	// convert number in layout units into paper units (loss of precision)    
	
   	return (UT_PAPER_UNITS_PER_INCH * iLayoutUnits / UT_LAYOUT_UNITS);
}

const char * UT_formatDimensionedValue(double value,
									   const char * szUnits,
									   const char * szPrecision)
{
	// format the given value into a static buffer with
	// the optional format precision and using the given
	// physical units.

	static char buf[100];

	const char * szValue = UT_convertToDimensionlessString(value,szPrecision);

	sprintf(buf,"%s%s",szValue,szUnits);

	return buf;
}

double UT_convertToDimension(const char* s, UT_Dimension dim)
{
	double d;

	// if needed, switch unit systems and round off

	if (UT_determineDimension(s, dim) != dim)
	{
		double dInches = UT_convertToInches(s);
		d = UT_convertInchesToDimension(dInches, dim); 
	}
	else
	{
		d = UT_convertDimensionless(s);
	}

	return d;
}
