/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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


#include "gr_MacGraphics.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"



MacGraphics::MacGraphics(CGContextRef context, XAP_App * app)
      : GR_Graphics ()
{
      UT_ASSERT (context != NULL);

      m_CGContext = context;
}

MacGraphics::~MacGraphics ()
{

}


void MacGraphics::drawChars(const UT_UCSChar* pChars, 
		int iCharOffset, int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}

void MacGraphics::setFont(GR_Font* pFont)
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}


UT_uint32 MacGraphics::getFontAscent()
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return 0;
}


UT_uint32 MacGraphics::getFontDescent()
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return 0;
}


UT_uint32 MacGraphics::getFontHeight()
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return 0;
}


UT_uint32 MacGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return 0;
}


void MacGraphics::setColor(UT_RGBColor& clr)
{
    CGContextSetRGBStrokeColor (m_CGContext, clr.m_red, clr.m_grn, clr.m_blu, 1.0f);
}


GR_Font* MacGraphics::getGUIFont()
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return NULL;
}

GR_Font* MacGraphics::findFont(
		const char* pszFontFamily, 
		const char* pszFontStyle, 
		const char* pszFontVariant, 
		const char* pszFontWeight, 
		const char* pszFontStretch, 
		const char* pszFontSize)
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return NULL;
}

void MacGraphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
    CGContextBeginPath (m_CGContext);
    CGContextMoveToPoint (m_CGContext, x1, y1);
    CGContextAddLineToPoint (m_CGContext, x2, y2);
    CGContextClosePath (m_CGContext);
    CGContextStrokePath (m_CGContext);
}


void MacGraphics::xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32)
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}


void MacGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
    UT_uint32 i;
    CGContextBeginPath (m_CGContext);
    CGContextMoveToPoint (m_CGContext, pts[0].x, pts[0].y);
    for (i = 1; i < nPoints; i++) {
        CGContextAddLineToPoint (m_CGContext, pts[i].x, pts[i].y);
    }
    CGContextClosePath (m_CGContext);
    CGContextStrokePath (m_CGContext);
}


	/* For fillRect() and ??:
	**   begin fill at x0,y0,
	**   ?? should x0+w,y0+h or x0+w+1,y0+h+1 be the last pixel affected ??
	*/
void MacGraphics::fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
    CGRect myRect;

    myRect.origin.x = x;
    myRect.origin.y = y;
    myRect.size.width = w;
    myRect.size.height = h;
    CGContextSetRGBFillColor (m_CGContext, c.m_red, c.m_grn, c.m_blu, 1.0f);
    CGContextFillRect (m_CGContext, myRect);
}
        
void MacGraphics::invertRect(const UT_Rect* pRect)
{
    CGContextSaveGState (m_CGContext);
    /* do the rect inversion */
    CGContextRestoreGState (m_CGContext);
    
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}
        
void MacGraphics::setClipRect(const UT_Rect* pRect)
{
    CGRect myRect;
    
    myRect.origin.x = pRect->left;
    myRect.origin.y = pRect->top;
    myRect.size.width = pRect->width;
    myRect.size.height = pRect->height;
    
    CGContextBeginPath (m_CGContext);
    CGContextAddRect (m_CGContext, myRect);
    CGContextClosePath (m_CGContext);
    CGContextClip (m_CGContext);
}

void MacGraphics::scroll(UT_sint32, UT_sint32)
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}


void MacGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height)
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}


void MacGraphics::clearArea(UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
    CGRect myRect;

    myRect.origin.x = x;
    myRect.origin.y = y;
    myRect.size.width = w;
    myRect.size.height = h;
    CGContextSetRGBFillColor (m_CGContext, 0.f, 0.f, 0.f, 1.0f);
    CGContextFillRect (m_CGContext, myRect);
}

UT_Bool MacGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
		return UT_TRUE;
	case DGP_PAPER:
		return UT_FALSE;
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

	/* the following are only used for printing */

UT_Bool MacGraphics::startPrint(void)
{
    // FIXIT
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return UT_FALSE;
}

UT_Bool MacGraphics::startPage(const char * szPageLabel, UT_uint32 pageNumber,
							  UT_Bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight)
{
    // FIXIT
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return UT_FALSE;
}


UT_Bool MacGraphics::endPrint(void)
{
    // FIXIT
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return UT_FALSE;
}




// Code below borrowed to gr_BeOSGraphics.cpp... FIXIT
//////////////////////////////////////////////////////////////////
// This is a static method in the GR_Font base class implemented
// in platform code.
//////////////////////////////////////////////////////////////////
void GR_Font::s_getGenericFontProperties(const char * szFontName,
										 FontFamilyEnum * pff,
										 FontPitchEnum * pfp,
										 UT_Bool * pbTrueType)
{
	// describe in generic terms the named font.

	// Note: most of the unix font handling code is in abi/src/af/xap/unix
	// Note: rather than in the graphics class.  i'm not sure this matters,
	// Note: but it is just different....

	// TODO add code to map the given font name into one of the
	// TODO enums in GR_Font and set *pff and *pft.

	*pff = FF_Unknown;
	*pfp = FP_Unknown;
	*pbTrueType = UT_TRUE;
}

