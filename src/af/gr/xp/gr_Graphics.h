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

// TODO change the name of all dg_ and DG_ classes to gr_ and GR_

#ifndef GR_GRAPHICS_H
#define GR_GRAPHICS_H

#include "ut_types.h"
#include "ut_misc.h"

class UT_RGBColor;

/*
	DG_Font is a reference to a font.  As it happens, everything about fonts
	is platform-specific, so the class contains nothing.  All of its behavior
	and functionality is contained within its subclasses, each of which provides
	the implementation for some specific font technology.
*/
class DG_Font
{
};

/*
	DG_Graphics is a portable interface to a simple 2-d graphics layer.  It is not
	an attempt at a general purpose portability layer.  Rather, it contains only
	functions which are needed.
*/
class DG_Graphics
{
public:
	virtual void drawChars(const UT_UCSChar* pChars, 
		int iCharOffset, int iLength, UT_sint32 xoff, UT_sint32 yoff) = 0;
	virtual void setFont(DG_Font* pFont) = 0;

	virtual UT_uint32 getFontAscent() = 0;
	virtual UT_uint32 getFontDescent() = 0;
	virtual UT_uint32 getFontHeight() = 0;
	
	virtual UT_uint32 measureString(const UT_UCSChar*s, int iOffset, int num, unsigned short* pWidths) = 0;
	
	virtual UT_uint32 getResolution() const = 0;
	virtual void setColor(UT_RGBColor& clr) = 0;
	virtual DG_Font* findFont(
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
	virtual void drawLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32) = 0;
	virtual void xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32) = 0;
	virtual void fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h) = 0;
	virtual void invertRect(const UT_Rect* pRect) = 0;
	virtual void setClipRect(const UT_Rect* pRect) = 0;
	virtual void scroll(UT_sint32, UT_sint32) = 0;
	virtual void clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32) = 0;
	
	typedef enum { DGP_SCREEN, DGP_PAPER } Properties;
	
	virtual UT_Bool queryProperties(DG_Graphics::Properties gp) const = 0;

	/* the following are only used for printing */
	
	virtual UT_Bool startPrint(void) = 0;
	virtual UT_Bool startPage(const char * szPageLabel, UT_uint32 pageNumber,
							  UT_Bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight) = 0;
	virtual UT_Bool endPrint(void) = 0;
};

#endif /* GR_GRAPHICS_H */
