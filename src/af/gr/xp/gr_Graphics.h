/* AbiWord
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

#ifndef GR_GRAPHICS_H
#define GR_GRAPHICS_H

#include "ut_types.h"
#include "ut_misc.h"
#include "gr_Image.h"

class UT_RGBColor;

/*
	GR_Font is a reference to a font.  As it happens, everything about fonts
	is platform-specific, so the class contains nothing.  All of its behavior
	and functionality is contained within its subclasses, each of which provides
	the implementation for some specific font technology.
*/
class GR_Font
{
};

/*
	GR_Graphics is a portable interface to a simple 2-d graphics layer.  It is not
	an attempt at a general purpose portability layer.  Rather, it contains only
	functions which are needed.
*/
class GR_Graphics
{
public:
	virtual ~GR_Graphics();

	virtual void drawChars(const UT_UCSChar* pChars, 
		int iCharOffset, int iLength, UT_sint32 xoff, UT_sint32 yoff) = 0;
	virtual void setFont(GR_Font* pFont) = 0;

	virtual UT_uint32 getFontAscent() = 0;
	virtual UT_uint32 getFontDescent() = 0;
	virtual UT_uint32 getFontHeight() = 0;
	
	virtual UT_uint32 measureString(const UT_UCSChar*s, int iOffset, int num, unsigned short* pWidths) = 0;
	
	virtual UT_uint32 getResolution() const = 0;
	virtual void setColor(UT_RGBColor& clr) = 0;
	virtual GR_Font* getGUIFont() = 0;
	virtual GR_Font* findFont(
		const char* pszFontFamily, 
		const char* pszFontStyle, 
		const char* pszFontVariant, 
		const char* pszFontWeight, 
		const char* pszFontStretch, 
		const char* pszFontSize) = 0;
	UT_sint32 convertDimension(const char*) const;
	UT_Bool scaleDimensions(const char * szLeftIn, const char * szWidthIn,
				UT_uint32 iWidthAvail,
				UT_sint32 * piLeft,
				UT_uint32 * piWidth) const;

	void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
	
	virtual void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest, UT_sint32 iDestWidth, UT_sint32 iDestHeight) = 0;
	
	/* For drawLine() and xorLine():
	**   x0,y0 give the starting pixel.
	**   x1,y1 give the first pixel ***not drawn***.
	*/
	virtual void drawLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32) = 0;
	virtual void xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32) = 0;
	virtual void setLineWidth(UT_sint32) = 0;

	virtual void polyLine(UT_Point * pts, UT_uint32 nPoints) = 0;

	/* For fillRect() and ??:
	**   begin fill at x0,y0,
	**   ?? should x0+w,y0+h or x0+w+1,y0+h+1 be the last pixel affected ??
	*/
	virtual void fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h) = 0;
	virtual void invertRect(const UT_Rect* pRect) = 0;
	virtual void setClipRect(const UT_Rect* pRect) = 0;
	virtual void scroll(UT_sint32, UT_sint32) = 0;
	virtual void scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height) = 0;
	virtual void clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32) = 0;
	
	typedef enum { DGP_SCREEN, DGP_PAPER } Properties;
	
	virtual UT_Bool queryProperties(GR_Graphics::Properties gp) const = 0;

	/* the following are only used for printing */
	
	virtual UT_Bool startPrint(void) = 0;
	virtual UT_Bool startPage(const char * szPageLabel, UT_uint32 pageNumber,
							  UT_Bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight) = 0;
	virtual UT_Bool endPrint(void) = 0;
};

#endif /* GR_GRAPHICS_H */
