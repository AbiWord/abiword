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

/*****************************************************************/
/*****************************************************************/

UT_Rect::UT_Rect()
{
	left = top = height = width = 0;
}

UT_Rect::UT_Rect(int iLeft, int iTop, int iWidth, int iHeight)
{
	left = iLeft;
	top = iTop;
	width = iWidth;
	height = iHeight;
}

UT_Rect::UT_Rect(const UT_Rect & r)
{
	left = r.left;
	top = r.top;
	width = r.width;
	height = r.height;
}

UT_Bool UT_Rect::containsPoint(UT_sint32 x, UT_sint32 y) const
{
	// return true iff the given (x,y) is inside the rectangle.

	if ((x < left) || (x >= left+width))
		return UT_FALSE;
	if ((y < top) || (y >= top+height))
		return UT_FALSE;

	return UT_TRUE;
}

void UT_Rect::set(int iLeft, int iTop, int iWidth, int iHeight)
{
	left = iLeft;
	top = iTop;
	width = iWidth;
	height = iHeight;
}

UT_Bool UT_Rect::intersectsRect(const UT_Rect * pRect) const
{
	// return true if this rectangle and pRect intersect.

#define R_RIGHT(pr)		(((pr)->left)+((pr)->width))
#define R_BOTTOM(pr)	(((pr)->top)+((pr)->height))
	
	if (R_RIGHT(pRect) < left)
		return UT_FALSE;

	if (R_RIGHT(this) < pRect->left)
		return UT_FALSE;

	if (R_BOTTOM(pRect) < top)
		return UT_FALSE;

	if (R_BOTTOM(this) < pRect->top)
		return UT_FALSE;

	return UT_TRUE;

#undef R_RIGHT
#undef R_BOTTOM
}

/*****************************************************************/
/*****************************************************************/

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

UT_RGBColor::UT_RGBColor()
{
	m_red = 0;
	m_grn = 0;
	m_blu = 0;
}

UT_RGBColor::UT_RGBColor(unsigned char red, unsigned char grn, unsigned char blu)
{
	m_red = red;
	m_grn = grn;
	m_blu = blu;
}

void UT_parseColor(const char *p, UT_RGBColor& c)
{
	if (strlen(p) == 6)
	{
		goto ParseHex;
	}
	if ((strlen(p) == 7) && (*p=='#'))
	{
		p++;
		goto ParseHex;
	}

	// TODO consider accepting stock names (vga) colors, too?

	UT_ASSERT(UT_NOT_IMPLEMENTED);

	return;

 ParseHex:
	c.m_red = x_hexDigit(p[0]) * 16 + x_hexDigit(p[1]);
	c.m_grn = x_hexDigit(p[2]) * 16 + x_hexDigit(p[3]);
	c.m_blu = x_hexDigit(p[4]) * 16 + x_hexDigit(p[5]);

	return;
}

UT_Bool UT_isWordDelimiter(UT_UCSChar ch)
{
	switch (ch)
	{
	case ' ':
	case ',':
	case '.':
	case '"':
	case '-':
//	case '\'':
	case '(':
	case ')':
	case '[':
	case ']':
	case '*':
	case ';':
	case ':':
	case '!':
	case '?':
	case UCS_TAB:	// tab
	case UCS_LF:	// line break
	case UCS_VTAB:	// column break
	case UCS_FF:	// page break
		return UT_TRUE;
	default:
		return UT_FALSE;
	}
}

const XML_Char* UT_getAttribute(const XML_Char* name, const XML_Char** atts)
{
	UT_ASSERT(atts && atts[0]);

	const XML_Char** p = atts;

	while (*p)
	{
		if (0 == UT_stricmp(p[0], name))
			break;
		p += 2;
	}

	if (*p)
		return p[1];
	else
		return NULL;
}
