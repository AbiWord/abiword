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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "ut_color.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include <limits.h>

/*****************************************************************/
/*****************************************************************/

/* These are the colors defined in the SVG standard (I haven't checked the final recommendation for changes)
 */
struct colorToRGBMapping
{
  const char * m_name;

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
	{ "turquoise",			 64, 224, 208 },
	{ "violet",			238, 130, 238 },
	{ "wheat",			245, 222, 179 },
	{ "white",			255, 255, 255 },
	{ "whitesmoke",			245, 245, 245 },
	{ "yellow",			255, 255,   0 },
	{ "yellowgreen",		154, 205,  50 }
};


static int color_compare (const void * a, const void * b)
{
  const char * name = static_cast<const char *>(a);
  const colorToRGBMapping * id = static_cast<const colorToRGBMapping *>(b);
		
  return g_ascii_strcasecmp (name, id->m_name);
}


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

static void UT_parseGrayColor(const char *p, UT_RGBColor& c)
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

static void UT_parseCMYKColor(const char *p, UT_RGBColor& c)
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


UT_ColorPatImpl::~UT_ColorPatImpl()
{
}


UT_RGBColor::UT_RGBColor()
    : m_patImpl(NULL)
{
	m_red = 0;
	m_grn = 0;
	m_blu = 0;
	m_bIsTransparent = false;
}

UT_RGBColor::UT_RGBColor(unsigned char red, unsigned char grn, unsigned char blu, bool bTransparent)
    : m_patImpl(NULL)
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
    m_patImpl = ( c.m_patImpl ? c.m_patImpl->clone() : NULL );
}

UT_RGBColor::UT_RGBColor(const UT_ColorPatImpl * pat)
    : m_red(0)
    , m_grn(0)
    , m_blu(0)
    , m_bIsTransparent(false)
    , m_patImpl(pat)
{
}


UT_RGBColor::~UT_RGBColor()
{
    DELETEP(m_patImpl);
}


UT_RGBColor & UT_RGBColor::operator=(const  UT_RGBColor &c)
{
	m_red = c.m_red;
	m_grn = c.m_grn;
	m_blu = c.m_blu;
	m_bIsTransparent = c.m_bIsTransparent;
    if(m_patImpl) {
        delete m_patImpl;
    }
    m_patImpl = ( c.m_patImpl ? c.m_patImpl->clone() : NULL );

    return *this;
}


bool UT_RGBColor::setColor(const char * pszColor)
{
	unsigned char r = m_red, g = m_grn, b = m_blu;
	
	if(!pszColor || !strcmp(pszColor,"transparent") /* || !strcmp(pszColor,"ffffff") */)
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
	    UT_parseCMYKColor ( p, c ) ;
	    return;
	  }

	if ( len > 6 && strncmp ( p, "gray(", 5 ) == 0 )
	  {
	    // grayscale color. parse that out
	    UT_parseGrayColor ( p, c ) ;
	    return ;
	  }


	if(!strcmp(p,"transparent") /* || !strcmp(p,"ffffff") */)
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

/*! This function takes in a color string of any form (e.g. "red", "CMYK()",
	 "#000000", "000000", etc.) and returns an RGB hexadecimal string.
	 \param szColor The incoming string to parse
	 \param bPrefix The return string will be prefixed with a '#'
	 if bPrefix is true.  Defaults to false.
	 \return An RGB hexadecimal string or an empty string if szColor is empty

    WARNING: Will return 000000 or #000000 if an invalid color is passed in
*/

std::string UT_colorToHex(const char * szColor, bool bPrefix)
{
	std::string sColor;
	UT_return_val_if_fail(szColor && *szColor, sColor);

	// This initialization will cause black to be returned if an invalid
	// color is passed in. TODO: make UT_parseColor() return a bool to
	// make this unnecessary?
	UT_RGBColor color(0,0,0);
	UT_HashColor hashColor;

	UT_parseColor(szColor, color);
	sColor = hashColor.setColor(color.m_red, color.m_grn, color.m_blu);

	if(!bPrefix)
		sColor.erase(0, 1);

	return sColor;
}

