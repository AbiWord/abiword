/* AbiWord
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


#ifndef UTMISC_H
#define UTMISC_H

#include "ut_types.h"
#include "xmlparse.h"

class UT_RGBColor;
class UT_Rect;

// ----------------------------------------------------------------
class UT_RGBColor
{
public:
	UT_RGBColor();
	UT_RGBColor(unsigned char, unsigned char, unsigned char);
	
	unsigned char m_red;
	unsigned char m_grn;
	unsigned char m_blu;
};

void UT_parseColor(const char*, UT_RGBColor&);

// ----------------------------------------------------------------
class UT_Rect
{
public:
	UT_Rect();
	void setValues(int, int, int, int);
	
	UT_sint32	left;
	UT_sint32	top;
	UT_sint32	width;
	UT_sint32	height;

protected:
};

#define UT_MAX(A,B)	(((A) > (B)) ? (A) : (B))
#define UT_MIN(A,B)	(((A) < (B)) ? (A) : (B))
#define UT_ABS(A)	( ((A) < 0) ? (-(A)) : (A) )

UT_Bool UT_isWordDelimiter(UT_UCSChar);
const XML_Char* UT_getAttribute(char* name, const XML_Char** atts);

#endif /* UTMISC_H */
