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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "ut_vector.h"
#include "ut_string_class.h"
#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
/*****************************************************************/
/*****************************************************************/

UT_Rect::UT_Rect()
{
	left = top = height = width = 0;
}

UT_Rect::UT_Rect(UT_sint32 iLeft, UT_sint32 iTop, UT_sint32 iWidth, UT_sint32 iHeight)
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


UT_Rect::UT_Rect(const UT_Rect * r)
{
	left = r->left;
	top = r->top;
	width = r->width;
	height = r->height;
}

bool UT_Rect::containsPoint(UT_sint32 x, UT_sint32 y) const
{
	// return true iff the given (x,y) is inside the rectangle.

	if ((x < left) || (x >= left+width))
		return false;
	if ((y < top) || (y >= top+height))
		return false;

	return true;
}

void UT_Rect::set(int iLeft, int iTop, int iWidth, int iHeight)
{
	left = iLeft;
	top = iTop;
	width = iWidth;
	height = iHeight;
}

/*!
 * This method makes a union of the current rectangle with the one in the 
 * parameter list. This rectangle is the smallest one that covers both
 * rectangles.
 */
void UT_Rect::unionRect(const UT_Rect * pRect)
{
	UT_sint32 fx1,fx2,fy1,fy2;
	fx1 = UT_MIN(left,pRect->left);
	fx2 = UT_MAX(left+width,pRect->left + pRect->width);
	fy1 = UT_MIN(top,pRect->top);
	fy2 = UT_MAX(top+height,pRect->top + pRect->height);
	left = fx1;
	width = fx2 - fx1;
	top = fy1;
	height = fy2 - fy1;
}

bool UT_Rect::intersectsRect(const UT_Rect * pRect) const
{
	// return true if this rectangle and pRect intersect.

#define R_RIGHT(pr)		(((pr)->left)+((pr)->width))
#define R_BOTTOM(pr)	(((pr)->top)+((pr)->height))
	
	if (R_RIGHT(pRect) < left)
		return false;

	if (R_RIGHT(this) < pRect->left)
		return false;

	if (R_BOTTOM(pRect) < top)
		return false;

	if (R_BOTTOM(this) < pRect->top)
		return false;

	return true;

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

void UT_setColor(UT_RGBColor & col, unsigned char r, unsigned char g, unsigned char b)
{
	col.m_red = r;
	col.m_grn = g;
	col.m_blu = b;
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

	UT_DEBUGMSG(("String = %s \n",p));
	UT_ASSERT(UT_NOT_IMPLEMENTED);

	return;

 ParseHex:
	c.m_red = x_hexDigit(p[0]) * 16 + x_hexDigit(p[1]);
	c.m_grn = x_hexDigit(p[2]) * 16 + x_hexDigit(p[3]);
	c.m_blu = x_hexDigit(p[4]) * 16 + x_hexDigit(p[5]);

	return;
}

#ifdef WIN32
#define ut_PATHSEP	'\\'
#else
#define ut_PATHSEP '/'
#endif

const char * UT_pathSuffix(const char * path)
{
	// TODO This needs to be moved to platform code.
	
	if (!path)
		return NULL;

	// This algorithm is pretty simple.  We search backwards for
	// a dot, and if the dot happens AFTER the last slash (if there
	// is a slash), we consider the stuff beyond the dot (in
	// the forward direction) the extension.
	const char * dotpos = strrchr(path, '.');
	const char * slashpos = strrchr(path, ut_PATHSEP);

	if (slashpos)
	{
		// There is a slash, we have a file like "/foo/bar/mox.txt"
		// or "/foo/bar/mox" or "/foo.dir/bar/mox.txt" or
		// "/foo.dir/bar/mox".

		// If there is a dot, and it's after the slash, the extension
		// is to the right.  If the dot is before the slash, there is
		// no extension.  If there's no dot, there's no extension.
		if (dotpos)
			if (dotpos > slashpos)
				return dotpos;
			else
				return NULL;
		else
			return NULL;
	}
	else
	{
		// There is no slash, we have a path like "file" or "file.txt".

		// If there's a dot, return
		if (dotpos)
			return dotpos;
		else
			return NULL;
	}
}

typedef struct {
  UT_UCSChar low;
  UT_UCSChar high; 
} ucs_range;

static ucs_range s_word_delim[] = 
{
  // we will include all control chars from Latin1
  {0x0001, 0x0021},
  {0x0023, 0x0026},
  {0x0028, 0x002f},
  {0x003a, 0x0040},
  {0x005b, 0x0060},
  {0x007b, 0x007e},
  {0x00a1, 0x00a7},
  {0x00a9, 0x00b3},
  {0x00b5, 0x00b7},
  {0x00b9, 0x00bf},
  {0x00f7, 0x00f7}
};

static bool s_find_delim(UT_UCSChar c)
{
    for(UT_uint32 i = 0; i < NrElements(s_word_delim); i++)
	{
		if(c < s_word_delim[i].low)
			return false;

		if(c <= s_word_delim[i].high)
			return true;
	}
	return false;
}

bool UT_isWordDelimiter(UT_UCSChar currentChar, UT_UCSChar followChar)
{
#if 1
    switch(currentChar)
	{
		case '"': //in some languages this can be in the middle of a word (Hebrew)
		case '\'':	// we want quotes inside words for contractions
		case UCS_LDBLQUOTE:    // smart quote, open double /* wjc */
		case UCS_RDBLQUOTE:    // smart quote, close double /* wjc */
		case UCS_LQUOTE:    // smart quote, open single  /* wjc */
		case UCS_RQUOTE:	// we want quotes inside words for contractions
			if (UT_UCS_isalpha(followChar))
			  {
				  return false;
			  }
			else
			  {
				  return true;
			  }
		case UCS_ABI_OBJECT:
			return false;

		default:
			return s_find_delim(currentChar);
	}

#else
	/*
		TODO we need a more systematic way to handle this, instead 
		TODO of just randomly adding more whitespace & punctuation
		TODO on an as-discovered basis
	*/
	switch (currentChar)
	{
		case ' ':
		case ',':
		case '.':
		case '-':
		case '_':
		case '(':
		case ')':
		case '[':
		case ']':
		case '{':
		case '}':
		case '<':
		case '>':
		case '*':
		case '/':
		case '+':
		case '=':
		case '#':
		case '$':
		case ';':
		case ':':
		case '!':
		case '?':
		case UCS_TAB:	// tab
		case UCS_LF:	// line break
		case UCS_VTAB:	// column break
		case UCS_FF:	// page break
		case 0x00a1:    // upside-down exclamation mark

		/* various currency symbols */
		case 0x00a2:
		case 0x00a3:
		case 0x00a4:
		case 0x00a5:

		/* other symbols */
		case 0x00a6:
		case 0x00a7:
		case 0x00a9:
		case 0x00ab:
		case 0x00ae:
		case 0x00b0:
		case 0x00b1:

		return true;
		case '"': //in some languages this can be in the middle of a word (Hebrew)
		case '\'':	// we want quotes inside words for contractions
		case UCS_LDBLQUOTE:    // smart quote, open double /* wjc */
		case UCS_RDBLQUOTE:    // smart quote, close double /* wjc */
		case UCS_LQUOTE:    // smart quote, open single  /* wjc */
		case UCS_RQUOTE:	// we want quotes inside words for contractions
			if (UT_UCS_isalpha(followChar))
			  {
				  return false;
			  }
			else
			  {
				  return true;
			  }
		case UCS_ABI_OBJECT:
		default:
			return false;
	}
#endif
}

const XML_Char* UT_getAttribute(const XML_Char* name, const XML_Char** atts)
{
	UT_ASSERT(atts);

	const XML_Char** p = atts;

	while (*p)
	{
		if (0 == strcmp((const char*)p[0], (const char*)name))
			break;
		p += 2;
	}

	if (*p)
		return p[1];
	else
		return NULL;
}

//////////////////////////////////////////////////////////////////

UT_sint32 signedLoWord(UT_uint32 dw)
{
	// return low word as a signed quantity

	unsigned short u16 = (unsigned short)(dw & 0xffff);
	signed short   s16 = *(signed short *)&u16;
	UT_sint32      s32 = s16;

	return s32;
}

UT_sint32 signedHiWord(UT_uint32 dw)
{
	// return high word as a signed quantity

	unsigned short u16 = (unsigned short)((dw >> 16) & 0xffff);
	signed short   s16 = *(signed short *)&u16;
	UT_sint32      s32 = s16;

	return s32;
}

//////////////////////////////////////////////////////////////////

/*!
 * simplesplit splits the referring string along the character 'separator', 
 * removing the separator character, and placing the resulting strings in a 
 * UT_Vector. If 'max' is specified, this is done max times - the max + 1
 * string in the vector will contain the remainder of the original string 
 * (str).
 */
UT_Vector * simpleSplit (const UT_String & str, char separator,
						 size_t max)
{
	UT_Vector * utvResult = new UT_Vector();
	UT_String* utsEntry;
	UT_uint32 start = 0;

	for(size_t j = 0; (max == 0 || j < max) && start < str.size(); j++)
	{
		utsEntry = new UT_String;

		for (; (str[start] != separator || j == max - 1) && start < str.size(); start++)
			*utsEntry += str[start];

		start++;						// skipping over the separator character
										// itself
		if (utsEntry->empty())
			delete utsEntry;
		else
			utvResult->addItem(utsEntry);
	}

	return utvResult;
}

/**
 * It search the next space (blank space, tab, etc.) in the string
 * str, starting at offset.
 *
 * @param str is the string where we will look for spaces.
 * @param offset is the offset where we will start our search.
 * @returns offset of the next space.
 */
UT_uint32 find_next_space(const UT_String& str, UT_uint32 offset)
{
	size_t max = str.size();
	for (++offset; offset < max; ++offset)
		if (isspace(str[offset]))
			break;

	return offset;
}

/**
 * It warps the string str in various lines, taking care that no line
 * goes beyond the column col.
 *
 * @param str is the string to warp.
 * @param col_max is the column that no character in any line should
 *                pass (except if we can not cut this line anywhere).
 */
void warpString(UT_String& str, size_t col_max)
{
	size_t max = str.size();

	for (UT_uint32 i = 0; i < max;)
	{
		UT_uint32 j = i, old_j;

		do {
			old_j = j;
			j = find_next_space(str, j);

			if (j < max && str[j] == '\n')
				i = j;
		} while (j - i < col_max && j < max);

		if (j >= max)
			return;

		if (old_j == i) // no spaces in the whole line!
		{
			str[j] = '\n';
			i = j;
		}
		else
		{
			str[old_j] = '\n';
			i = old_j;
		}
	}
}

/*!
 * Strips off the first numeric part of string and returns it as a uint32.
 * ie. "Numbered Heading 5" would return 5.
 */
UT_uint32 UT_HeadingDepth(const char * szHeadingName)
{
	UT_String sz;
	UT_uint32 i = 0;
	bool bFound = false;
	bool bStop = false;
	for(i=0; i< strlen(szHeadingName) && !bStop ; i++)
	{
		if(szHeadingName[i] >= '0' && szHeadingName[i] <= '9')
		{
			sz += szHeadingName[i];
			bFound = true;
		}
		else if(bFound)
		{
			bStop = true;
			break;
		}
	}
	i = (UT_uint32) atoi(sz.c_str());
	return i;
}
