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
 


#ifndef UTMISC_H
#define UTMISC_H

#include "ut_types.h"
#include "ut_xml.h"

class UT_RGBColor;
class UT_Rect;
class UT_Vector;
class UT_String;

// ----------------------------------------------------------------
#define UT_RGBCOLOR_PROXIMITY 45

class ABI_EXPORT UT_RGBColor
{
public:
	UT_RGBColor();
	UT_RGBColor(unsigned char, unsigned char, unsigned char, bool bTransparent = false);
	UT_RGBColor(const UT_RGBColor&);
	bool operator != (const UT_RGBColor &op1)
	{
		return (op1.m_red != m_red || op1.m_grn != m_grn || op1.m_blu != m_blu);
	}

	bool operator == (const UT_RGBColor &op1)
	{
		return (op1.m_red == m_red && op1.m_grn == m_grn && op1.m_blu == m_blu);
	}
	
	// returns true if the two colors are near each other in the RGB space
	bool operator %= (const UT_RGBColor &op1)
	{
		UT_uint32 iDiff = abs(m_red - op1.m_red) + abs(m_grn - op1.m_grn) + abs(m_blu - op1.m_blu);
		return (iDiff < UT_RGBCOLOR_PROXIMITY);
	}

	UT_RGBColor & operator ^= (const UT_RGBColor &op1)
	{
		m_red ^= op1.m_red;
		m_grn ^= op1.m_grn;
		m_blu ^= op1.m_blu;
		return *this;
	}

	UT_RGBColor & operator += (const unsigned char inc)
	{
		m_red += inc;
		m_grn += inc;
		m_blu += inc;
		return *this;
	}

	UT_RGBColor & operator += (const  UT_RGBColor &inc)
	{
		m_red += inc.m_red;
		m_grn += inc.m_grn;
		m_blu += inc.m_blu;
		return *this;
	}

	UT_RGBColor & operator -= (const  UT_RGBColor &inc)
	{
		m_red -= inc.m_red;
		m_grn -= inc.m_grn;
		m_blu -= inc.m_blu;
		return *this;
	}

	inline bool isTransparent() const {return m_bIsTransparent;}
	bool setColor(const char * pszColor);

	unsigned char m_red;
	unsigned char m_grn;
	unsigned char m_blu;
	bool m_bIsTransparent;
	
};

void UT_setColor(UT_RGBColor & col, unsigned char r, unsigned char g, unsigned char b, bool bTransparent = false);
ABI_EXPORT void UT_parseColor(const char*, UT_RGBColor&);

class ABI_EXPORT UT_HashColor
{
private:
	char m_colorBuffer[8]; // format: "" or "#abc123" (i.e., '#' + 6 lower-case hex digits)

public:
	UT_HashColor ();
	~UT_HashColor ();

	/* The following 5 functions return a pointer to m_colorBuffer on success,
	 * or 0 on failure (invalid or unknown color).
	 */
	const char * setColor (unsigned char r, unsigned char g, unsigned char b);
	const char * setColor (const UT_RGBColor & color) { return setColor (color.m_red, color.m_grn, color.m_blu); }
	const char * setColor (const char * color); // try match hash (e.g., "#C01bB7") or name (e.g., "turquoise")
	const char * lookupNamedColor (const char * color_name); // "Orange" or "blue" or "LightGoldenRodYellow" or...
	const char * setHashIfValid (const char * color_hash);   // "ff0013" or "AD5FE6" or... (NOTE: no '#')

	const UT_RGBColor rgb (); // Call this *if* setColor () succeeds; otherwise defaults to black.
};

// ----------------------------------------------------------------
class ABI_EXPORT UT_Rect
{
public:
	UT_Rect();
	UT_Rect(UT_sint32 iLeft, UT_sint32 iTop, UT_sint32 iWidth, UT_sint32 iHeight);
	UT_Rect(const UT_Rect &);
	UT_Rect(const UT_Rect * r);

	bool containsPoint(UT_sint32 x, UT_sint32 y) const;
	void set(UT_sint32 iLeft, UT_sint32 iTop, UT_sint32 iWidth, UT_sint32 iHeight);
	bool intersectsRect(const UT_Rect * pRect) const;
	void unionRect( const UT_Rect *pRect);
	UT_sint32	left;
	UT_sint32	top;
	UT_sint32	width;
	UT_sint32	height;
};

// ----------------------------------------------------------------
struct ABI_EXPORT UT_Point
{
	UT_sint32	x;
	UT_sint32	y;
};

#define UT_MAX(A,B)	(((A) > (B)) ? (A) : (B))
#define UT_MIN(A,B)	(((A) < (B)) ? (A) : (B))
#define UT_ABS(A)	( ((A) < 0) ? (-(A)) : (A) )

const char * UT_pathSuffix(const char * path);
bool UT_isWordDelimiter(UT_UCSChar currentChar, UT_UCSChar followChar, UT_UCSChar prevChar);
ABI_EXPORT const XML_Char* UT_getAttribute(const XML_Char* name, const XML_Char** atts);

UT_sint32 signedHiWord(UT_uint32 dw);
UT_sint32 signedLoWord(UT_uint32 dw);

UT_Vector * simpleSplit (const UT_String & str, char separator = ' ',
						 size_t max = 0 /* 0 == full split */);

void warpString(UT_String& str, size_t col_max = 75);
UT_uint32 UT_HeadingDepth(const char * szHeadName);
#endif /* UTMISC_H */
