
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_string.h"

static int x_hexDigit(char c)
{
	if ((c>='0') && (c<='9'))
	{
		return c-'0';
	}

	if ((c>='a') && (c<='f'))
	{
		return c - 'a' + 10;
	}

	if ((c>='A') && (c<='F'))
	{
		return c - 'A' + 10;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return 0;
}

// TODO shouldn't we have a #000000 syntax like CSS?
void UT_parseColor(const char *p, UT_RGBColor& c)
{
	UT_ASSERT(strlen(p) == 6);

	c.m_red = x_hexDigit(p[0]) * 16 + x_hexDigit(p[1]);
	c.m_grn = x_hexDigit(p[2]) * 16 + x_hexDigit(p[3]);
	c.m_blu = x_hexDigit(p[4]) * 16 + x_hexDigit(p[5]);
}

double UT_convertToInches(const char* s)
{
	UT_ASSERT(s);

	/*
		TODO later, this routine needs to handle more units.
	*/

	double result = 0;
	double f = atof(s);
	const char *p = s;
	while ((*p) && (isdigit(*p) || (*p == '.') || (*p == '.')))
	{
		p++;
	}

	// p should now point to the unit
	if (*p)
	{
		if (0 == UT_stricmp(p, "in"))
		{
			result = f;
		}
		else if (0 == UT_stricmp(p, "pt"))
		{
			result = f * 72;
		}
		else
		{
			// unknown unit
		}
	}
	else
	{
		// no units are assumed to be inches  
		result = f;
	}

	return result;
}

UT_Bool UT_scaleGeometry(const char * szLeftIn,
						 const char * szWidthIn,
						 UT_uint32 iWidthAvail,
						 UT_uint32 * piLeft,
						 UT_uint32 * piWidth)
{
	/* Scale the given left-offset and width using the width available.
	** Compute the actual left-offset and actual width used.
	** We allow the given left-offset to be a number.
	** We allow the given width to be a number or "*"; where "*" indicates
	** we take all remaining space available.
	*/

	UT_uint32 iLeft = (UT_uint32) (UT_convertToInches(szLeftIn) * 100);
	UT_uint32 iWidth;

	if (szWidthIn[0] == '*')
		iWidth = iWidthAvail - iLeft;
	else
		iWidth = (UT_uint32) (UT_convertToInches(szWidthIn) * 100);

	*piLeft = iLeft;
	*piWidth = iWidth;

	return UT_TRUE;
}
