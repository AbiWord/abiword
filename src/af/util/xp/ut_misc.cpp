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

/* These are the colors defined in the SVG standard (I haven't checked the final recommendation for changes)
 */
struct colorToRGBMapping
{
  char * m_name;

  unsigned char m_red;
  unsigned char m_green;
  unsigned char m_blue;
};

static struct colorToRGBMapping s_Colors[] =
{
	{ "aliceblue",			240, 248, 255 },
	{ "antiquewhite",		250, 235, 215 },
	{ "aqua",			  0, 255, 255 },
	{ "aquamarine",			127, 255, 212 },
	{ "azure",			240, 255, 255 },
	{ "beige",			245, 245, 220 },
	{ "bisque",			255, 228, 196 },
	{ "black",			  0,   0,   0 },
	{ "blanchedalmond",		255, 235, 205 },
	{ "blue",			  0,   0, 255 },
	{ "blueviolet",			138,  43, 226 },
	{ "brown",			165,  42,  42 },
	{ "burlywood",			222, 184, 135 },
	{ "cadetblue",			 95, 158, 160 },
	{ "chartreuse",			127, 255,   0 },
	{ "chocolate",			210, 105,  30 },
	{ "coral",			255, 127,  80 },
	{ "cornflowerblue",		100, 149, 237 },
	{ "cornsilk",			255, 248, 220 },
	{ "crimson",			220,  20,  60 },
	{ "cyan",			  0, 255, 255 },
	{ "darkblue",			  0,   0, 139 },
	{ "darkcyan",			  0, 139, 139 },
	{ "darkgoldenrod",		184, 134,  11 },
	{ "darkgray",			169, 169, 169 },
	{ "darkgreen",			  0, 100,   0 },
	{ "darkgrey",			169, 169, 169 },
	{ "darkkhaki",			189, 183, 107 },
	{ "darkmagenta",		139,   0, 139 },
	{ "darkolivegreen",		 85, 107,  47 },
	{ "darkorange",			255, 140,   0 },
	{ "darkorchid",			153,  50, 204 },
	{ "darkred",			139,   0,   0 },
	{ "darksalmon",			233, 150, 122 },
	{ "darkseagreen",		143, 188, 143 },
	{ "darkslateblue",		 72,  61, 139 },
	{ "darkslategray",		 47,  79,  79 },
	{ "darkslategrey",		 47,  79,  79 },
	{ "darkturquoise",		  0, 206, 209 },
	{ "darkviolet",			148,   0, 211 },
	{ "deeppink",			255,  20, 147 },
	{ "deepskyblue",		  0, 191, 255 },
	{ "dimgray",			105, 105, 105 },
	{ "dimgrey",			105, 105, 105 },
	{ "dodgerblue",			 30, 144, 255 },
	{ "ffffff",             255, 255, 255 },
	{ "firebrick",			178,  34,  34 },
	{ "floralwhite",		255, 250, 240 },
	{ "forestgreen",		 34, 139,  34 },
	{ "fuchsia",			255,   0, 255 },
	{ "gainsboro",			220, 220, 220 },
	{ "ghostwhite",			248, 248, 255 },
	{ "gold",			255, 215,   0 },
	{ "goldenrod",			218, 165,  32 },
	{ "gray",			128, 128, 128 },
	{ "grey",			128, 128, 128 },
	{ "green",			  0, 128,   0 },
	{ "greenyellow",		173, 255,  47 },
	{ "honeydew",			240, 255, 240 },
	{ "hotpink",			255, 105, 180 },
	{ "indianred",			205,  92,  92 },
	{ "indigo",			 75,   0, 130 },
	{ "ivory",			255, 255, 240 },
	{ "khaki",			240, 230, 140 },
	{ "lavender",			230, 230, 250 },
	{ "lavenderblush",		255, 240, 245 },
	{ "lawngreen",			124, 252,   0 },
	{ "lemonchiffon",		255, 250, 205 },
	{ "lightblue",			173, 216, 230 },
	{ "lightcoral",			240, 128, 128 },
	{ "lightcyan",			224, 255, 255 },
	{ "lightgoldenrodyellow",	250, 250, 210 },
	{ "lightgray",			211, 211, 211 },
	{ "lightgreen",			144, 238, 144 },
	{ "lightgrey",			211, 211, 211 },
	{ "lightpink",			255, 182, 193 },
	{ "lightsalmon",		255, 160, 122 },
	{ "lightseagreen",		 32, 178, 170 },
	{ "lightskyblue",		135, 206, 250 },
	{ "lightslategray",		119, 136, 153 },
	{ "lightslategrey",		119, 136, 153 },
	{ "lightsteelblue",		176, 196, 222 },
	{ "lightyellow",		255, 255, 224 },
	{ "lime",			  0, 255,   0 },
	{ "limegreen",			 50, 205,  50 },
	{ "linen",			250, 240, 230 },
	{ "magenta",			255,   0, 255 },
	{ "maroon",			128,   0,   0 },
	{ "mediumaquamarine",		102, 205, 170 },
	{ "mediumblue",			  0,   0, 205 },
	{ "mediumorchid",		186,  85, 211 },
	{ "mediumpurple",		147, 112, 219 },
	{ "mediumseagreen",		 60, 179, 113 },
	{ "mediumslateblue",		123, 104, 238 },
	{ "mediumspringgreen",		  0, 250, 154 },
	{ "mediumturquoise",		 72, 209, 204 },
	{ "mediumvioletred",		199,  21, 133 },
	{ "midnightblue",		 25,  25, 112 },
	{ "mintcream",			245, 255, 250 },
	{ "mistyrose",			255, 228, 225 },
	{ "moccasin",			255, 228, 181 },
	{ "navajowhite",		255, 222, 173 },
	{ "navy",			  0,   0, 128 },
	{ "oldlace",			253, 245, 230 },
	{ "olive",			128, 128,   0 },
	{ "olivedrab",			107, 142,  35 },
	{ "orange",			255, 165,   0 },
	{ "orangered",			255,  69,   0 },
	{ "orchid",			218, 112, 214 },
	{ "palegoldenrod",		238, 232, 170 },
	{ "palegreen",			152, 251, 152 },
	{ "paleturquoise",		175, 238, 238 },
	{ "palevioletred",		219, 112, 147 },
	{ "papayawhip",			255, 239, 213 },
	{ "peachpuff",			255, 218, 185 },
	{ "peru",			205, 133,  63 },
	{ "pink",			255, 192, 203 },
	{ "plum",			221, 160, 221 },
	{ "powderblue",			176, 224, 230 },
	{ "purple",			128,   0, 128 },
	{ "red",			255,   0,   0 },
	{ "rosybrown",			188, 143, 143 },
	{ "royalblue",			 65, 105, 225 },
	{ "saddlebrown",		139,  69,  19 },
	{ "salmon",			250, 128, 114 },
	{ "sandybrown",			244, 164,  96 },
	{ "seagreen",			 46, 139,  87 },
	{ "seashell",			255, 245, 238 },
	{ "sienna",			160,  82,  45 },
	{ "silver",			192, 192, 192 },
	{ "skyblue",			135, 206, 235 },
	{ "slateblue",			106,  90, 205 },
	{ "slategray",			112, 128, 144 },
	{ "slategrey",			112, 128, 144 },
	{ "snow",			255, 250, 250 },
	{ "springgreen",		  0, 255, 127 },
	{ "steelblue",			 70, 130, 180 },
	{ "tan",			210, 180, 140 },
	{ "teal",			  0, 128, 128 },
	{ "thistle",			216, 191, 216 },
	{ "tomato",			255,  99,  71 },
	{ "transparent",    255, 255, 255 },
	{ "turquoise",			 64, 224, 208 },
	{ "violet",			238, 130, 238 },
	{ "wheat",			245, 222, 179 },
	{ "white",			255, 255, 255 },
	{ "whitesmoke",			245, 245, 245 },
	{ "yellow",			255, 255,   0 },
	{ "yellowgreen",		154, 205,  50 }
};

#ifdef __MRC__
extern "C" {
#endif

static int color_compare (const void * a, const void * b)
{
  const char * name = static_cast<const char *>(a);
  const colorToRGBMapping * id = static_cast<const colorToRGBMapping *>(b);
		
  return UT_stricmp (name, id->m_name);
}

#ifdef __MRC__
};
#endif

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

UT_HashColor::UT_HashColor ()
{
	m_colorBuffer[0] = 0;
}

UT_HashColor::~UT_HashColor ()
{
	//
}

const char * UT_HashColor::setColor (const char * color)
{
	m_colorBuffer[0] = 0;
	if (color == 0) return 0;

	if (color[0] == '#') return setHashIfValid (color + 1);

	return lookupNamedColor (color);
}

const char * UT_HashColor::setColor (unsigned char r, unsigned char g, unsigned char b)
{
	static const char hexval[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };

	m_colorBuffer[0] = '#';
	m_colorBuffer[1] = hexval[(r >> 4) & 0x0f];
	m_colorBuffer[2] = hexval[ r       & 0x0f];
	m_colorBuffer[3] = hexval[(g >> 4) & 0x0f];
	m_colorBuffer[4] = hexval[ g       & 0x0f];
	m_colorBuffer[5] = hexval[(b >> 4) & 0x0f];
	m_colorBuffer[6] = hexval[ b       & 0x0f];
	m_colorBuffer[7] = 0;

	return static_cast<const char *>(m_colorBuffer);
}

const char * UT_HashColor::lookupNamedColor (const char * color_name)
{
	m_colorBuffer[0] = 0;
	if (color_name == 0) return 0;

	size_t length = sizeof (s_Colors) / sizeof (s_Colors[0]);

	colorToRGBMapping * id = 0;
	id = static_cast<colorToRGBMapping *>(bsearch (color_name, s_Colors, static_cast<int>(length), sizeof (colorToRGBMapping), color_compare));

	if (id == 0) return 0;

	return setColor (id->m_red, id->m_green, id->m_blue);
}

const char * UT_HashColor::setHashIfValid (const char * color_hash)
{
	m_colorBuffer[0] = 0;
	if (color_hash == 0) return 0;

	bool isValid = true;
	for (int i = 0; i < 6; i++)
	{
		switch (color_hash[i])
		{
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			m_colorBuffer[i+1] = color_hash[i];
		break;
		case 'A': m_colorBuffer[i+1] = 'a'; break;
		case 'B': m_colorBuffer[i+1] = 'b'; break;
		case 'C': m_colorBuffer[i+1] = 'c'; break;
		case 'D': m_colorBuffer[i+1] = 'd'; break;
		case 'E': m_colorBuffer[i+1] = 'e'; break;
		case 'F': m_colorBuffer[i+1] = 'f'; break;
		default:
			isValid = false;
		break;
		}
		if (!isValid) break;
	}
	if (!isValid) return 0;

	m_colorBuffer[0] = '#';
	m_colorBuffer[7] = 0;

	return static_cast<const char *>(m_colorBuffer);
}

const UT_RGBColor UT_HashColor::rgb ()
{
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;

	if (m_colorBuffer[0])
	{
		r = static_cast<unsigned char>(x_hexDigit (m_colorBuffer[1]) << 4) | static_cast<unsigned char>(x_hexDigit (m_colorBuffer[2]));
		g = static_cast<unsigned char>(x_hexDigit (m_colorBuffer[3]) << 4) | static_cast<unsigned char>(x_hexDigit (m_colorBuffer[4]));
		b = static_cast<unsigned char>(x_hexDigit (m_colorBuffer[5]) << 4) | static_cast<unsigned char>(x_hexDigit (m_colorBuffer[6]));
	}

	return UT_RGBColor (r, g, b);
}

static int parseColorToNextDelim ( const char * p, UT_uint32 & index )
{
  char buffer[7] = "" ;
  index = 0 ;

  while (isdigit(*p))
    {
      buffer[index++] = *p++;
    }
  buffer[index] = 0;
  return atoi(buffer);
}

static void UT_parseGrayColor(const char *p, UT_RGBColor& c, UT_uint32 len)
{
  UT_DEBUGMSG(("DOM: parsing gray value\n"));

  int grayVal = 0 ;

  p+=5; // go past gray(

  UT_uint32 index = 0;
  grayVal= parseColorToNextDelim ( p, index ) ;

  c.m_red = grayVal;
  c.m_grn = grayVal;
  c.m_blu = grayVal;
}

static void UT_parseCMYKColor(const char *p, UT_RGBColor& c, UT_uint32 len)
{
  // yes, i know that CMYK->RGB is lossy... DAL
  // WARNING: !!!!UNTESTED!!!!

  UT_DEBUGMSG(("DOM: parsing CMYK value!!\n"));

  int cyanVal    = 0;
  int magentaVal = 0;
  int yellowVal  = 0;
  int kVal       = 0;

  p += 5; // advance past "CMYK("

  UT_uint32 index = 0;

  cyanVal = parseColorToNextDelim ( p, index ) ;
  p+=(index+1); index = 0;

  magentaVal = parseColorToNextDelim ( p, index ) ;
  p+=(index+1); index = 0;

  yellowVal = parseColorToNextDelim ( p, index ) ;
  p+=(index+1); index = 0;

  kVal = parseColorToNextDelim ( p, index ) ;

  int  cPlusK = cyanVal + kVal;
  int  mPlusK  = magentaVal + kVal;
  int  yPlusK = yellowVal + kVal;
  
  if (cPlusK < 255)
    c.m_red = 255 - cPlusK;
  
  if (mPlusK < 255)
    c.m_grn = 255 - mPlusK;
  
  if (yPlusK < 255)
    c.m_blu = 255 - yPlusK;

  UT_DEBUGMSG(("DOM: CMYK (%d %d %d %d) -> RGB (%d %d %d)!!\n",
	       cyanVal, magentaVal, yellowVal, kVal,
	       c.m_red, c.m_grn, c.m_blu));
}

UT_RGBColor::UT_RGBColor()
{
	m_red = 0;
	m_grn = 0;
	m_blu = 0;
	m_bIsTransparent = false;
}

UT_RGBColor::UT_RGBColor(unsigned char red, unsigned char grn, unsigned char blu, bool bTransparent)
{
	m_red = red;
	m_grn = grn;
	m_blu = blu;
	m_bIsTransparent = bTransparent;
}

UT_RGBColor::UT_RGBColor(const UT_RGBColor &c)
{
	m_red = c.m_red;
	m_grn = c.m_grn;
	m_blu = c.m_blu;
	m_bIsTransparent = c.m_bIsTransparent;
}

bool UT_RGBColor::setColor(const char * pszColor)
{
	unsigned char r = m_red, g = m_grn, b = m_blu;
	
	if(!pszColor || !strcmp(pszColor,"transparent") || !strcmp(pszColor,"ffffff"))
	{
		m_red = m_grn = m_blu = 255;
		m_bIsTransparent = true;
	}
	else
	{
		UT_parseColor(pszColor, *this);
		m_bIsTransparent = false;
	}
	
	return (r != m_red || g != m_grn || b != m_blu);
}


void UT_setColor(UT_RGBColor & col, unsigned char r, unsigned char g, unsigned char b, bool bTransparent)
{
	col.m_red = r;
	col.m_grn = g;
	col.m_blu = b;
	col.m_bIsTransparent = bTransparent;
}

void UT_parseColor(const char *p, UT_RGBColor& c)
{
        UT_uint32 len = strlen (p);

	if ( len > 7 && strncmp ( p, "cmyk(", 5 ) == 0 )
	  {
	    // CMYK color. parse that out
	    UT_parseCMYKColor ( p, c, len ) ;
	    return;
	  }

	if ( len > 6 && strncmp ( p, "gray(", 5 ) == 0 )
	  {
	    // grayscale color. parse that out
	    UT_parseGrayColor ( p, c, len ) ;
	    return ;
	  }


	if(!strcmp(p,"transparent") || !strcmp(p,"ffffff"))
	{
		c.m_red = c.m_grn = c.m_blu = 255;
		c.m_bIsTransparent = true;
		return;
	}
	
	UT_HashColor hash;

	if (hash.setColor (p))
	  {
	    c = hash.rgb ();
	  }
	else if (hash.setHashIfValid (p))
	  {
	    c = hash.rgb ();
	  }
	else
	  {
	    UT_DEBUGMSG(("String = %s \n",p));
	    UT_ASSERT(UT_NOT_IMPLEMENTED);
	  }

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
  {0x00f7, 0x00f7},
  {0x060c, 0x060c}, //Arabic punctuation
  {0x061b, 0x061b},
  {0x061f, 0x061f},
  {0xfeff, 0xfeff}
};

static bool s_find_delim(UT_UCSChar c)
{
    // This function is called for every character in the document
    // during spell-checking. So make an effort to return in a hurry,
    // by checking for the common cases (a-z and the biggest area)
    // first. (not A-Z since there are more word seperators than
    // capitals in a sentence!)
    if ('a' <= c && c <= 'z') return false;
    if (0x00f8 <= c && c <= 0xfefe) return false;

    // Everything else via the table above...
    for(UT_uint32 i = 0; i < NrElements(s_word_delim); i++)
	{
		if(c < s_word_delim[i].low)
			return false;

		if(c <= s_word_delim[i].high)
			return true;
	}
	return false;
}

bool UT_isWordDelimiter(UT_UCSChar currentChar, UT_UCSChar followChar, UT_UCSChar prevChar)
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
			if (UT_UCS4_isalpha(followChar) && UT_UCS4_isalpha(prevChar))
			  {
				  return false;
			  }
			else
			  {
				  return true;
			  }
		case UCS_ABI_OBJECT:
			return true;

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
		if (0 == strcmp(static_cast<const char*>(p[0]), static_cast<const char*>(name)))
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

	unsigned short u16 = static_cast<unsigned short>(dw & 0xffff);
	signed short   s16 = *reinterpret_cast<signed short *>(&u16);
	UT_sint32      s32 = s16;

	return s32;
}

UT_sint32 signedHiWord(UT_uint32 dw)
{
	// return high word as a signed quantity

	unsigned short u16 = static_cast<unsigned short>((dw >> 16) & 0xffff);
	signed short   s16 = *reinterpret_cast<signed short *>(&u16);
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
	i = static_cast<UT_uint32>(atoi(sz.c_str()));
	return i;
}

void * UT_calloc ( UT_uint32 nmemb, UT_uint32 size )
{
#if 1
  // this is actually much faster than the OSes calloc usually
  UT_uint32 theSize = nmemb * size ;
  if ( !theSize )
    return NULL ;

  void * theBlock = ::malloc( theSize ) ;
  if ( !theBlock )
    return NULL ;

  ::memset( theBlock, 0, theSize ) ;
  return theBlock ;
#else
  return ::calloc ( nmemb, size ) ;
#endif
}
