/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
 * Copyright (C) 2000 Hubert Figuiere
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

#include <Appearance.h>
#include <Fonts.h>

#include "gr_MacGraphics.h"
#include "gr_MacFont.h"
#include "xap_MacFontManager.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"


GR_MacGraphics::GR_MacGraphics(CGrafPtr port, XAP_MacFontManager * fontManager, XAP_App * app)
      : GR_Graphics ()
{
    Rect rect;
    OSStatus err;
    UT_ASSERT (port != NULL);
	UT_ASSERT (fontManager);

	m_pMacFontManager = fontManager;
	m_qdPort = port;
    err = CreateCGContextForPort (port, &m_CGContext );
    UT_ASSERT (err == noErr);
    ::GetPortBounds(port, &rect);
	SyncCGContextOriginWithPort (m_CGContext, port );

    CGContextTranslateCTM(m_CGContext, 0, (float)(rect.bottom - rect.top));
    // Be aware that by performing a negative scale in the following line of
    // code, your text will also be flipped
    CGContextScaleCTM(m_CGContext, 1, -1);

    
	m_CGFont   = NULL;
    m_pMacFont = NULL;

    UT_RGBColor c;
    c.m_red = c.m_grn = c.m_blu = 0;		//Black
    m_3Dcolors[CLR3D_Foreground] = c;
    c.m_red = c.m_grn = c.m_blu = 219;		//Light Grey
    m_3Dcolors[CLR3D_Background] = c;
    c.m_red = c.m_grn = c.m_blu = 150;		//Dark Grey
    m_3Dcolors[CLR3D_BevelDown] = c;
    c.m_red = c.m_grn = c.m_blu = 255;		//White
    m_3Dcolors[CLR3D_Highlight] = c;
    c.m_red = c.m_grn = c.m_blu = 255;		//White
    m_3Dcolors[CLR3D_BevelUp] = c;
}

GR_MacGraphics::~GR_MacGraphics ()
{
    CGContextRelease (m_CGContext);
    m_CGContext = NULL; 
    if (m_CGFont) {
        CGFontRelease (m_CGFont);
    }
    if (m_pMacFont) {
        delete m_pMacFont;
    }
}



void GR_MacGraphics::_syncQDOrigin (short y)
{
	OSStatus err;
	//	err = ::SyncCGContextOriginWithPort(m_CGContext, m_qdPort);
	//	UT_ASSERT (err == noErr);
	
	::CGContextTranslateCTM(m_CGContext, 0.f, y);
}

void GR_MacGraphics::drawChars(const UT_UCSChar* pChars, 
		int iCharOffset, int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_DEBUGMSG (("drawChars: %d, %d\n", iCharOffset, iLength));
	ATSUTextLayout theLayout = NULL;
    OSStatus err;
	ByteCount iSize = sizeof(CGContextRef);
	ATSUAttributeTag iTag = kATSUCGContextTag;
	ATSUAttributeValuePtr iValuePtr = &m_CGContext;
    ATSUStyle theStyle = m_pMacFont->getATSUStyle();
	/* set up our locals, verify parameters... */
    UT_ASSERT (pChars) ;
    theLayout = NULL;

	/* create the ATSUI layout */
    err = ATSUCreateTextLayoutWithTextPtr( pChars, iCharOffset,
										   iCharOffset + iLength, iCharOffset + iLength, 1, 
										   (unsigned long *) &iLength, &theStyle,
										   &theLayout);
	UT_ASSERT (err == noErr);
    if (err != noErr) 
		goto bail;

	/* Set the CG to force drawing using CGContext*/
	err = ATSUSetLayoutControls( theLayout, 1, &iTag, &iSize, &iValuePtr );
	UT_ASSERT (err == noErr);

	/* draw the text */
    err = ATSUDrawText(theLayout, 0, iLength,
					   FixRatio(xoff, 1), FixRatio(yoff, 1));
	UT_ASSERT (err == noErr);

	/* done */
 bail:
    if (theLayout != NULL) {
		ATSUDisposeTextLayout(theLayout);
	}
}

void GR_MacGraphics::setFont(GR_Font* pFont)
{
	// TODO: check the ownership of pFont. I think become GR_MacGraphics', but I'm
	// not sure
    // TODO: if remark above is true, delete.
	m_pMacFont = dynamic_cast<GR_MacFont *>(pFont);
    UT_ASSERT (m_pMacFont != NULL);
	ATSFontRef	fontRef;
    
#if 0
    if (m_CGFont) {
        CGFontRelease (m_CGFont);
    }
	fontRef = m_pMacFont->getFontRef();
    m_CGFont = CGFontCreateWithPlatformFont ((void*)&fontRef);
    ::CGContextSetFont(m_CGContext, m_CGFont);
	::CGContextSetFontSize (m_CGContext, m_pMacFont->getSize());
#endif
}


UT_uint32 GR_MacGraphics::getFontAscent()
{
    UT_ASSERT (m_pMacFont != NULL);
    return m_pMacFont->getAscent();
}


UT_uint32 GR_MacGraphics::getFontDescent()
{
    UT_ASSERT (m_pMacFont != NULL);
    return m_pMacFont->getDescent();
}


UT_uint32 GR_MacGraphics::getFontHeight()
{
    UT_ASSERT (m_pMacFont != NULL);
    return m_pMacFont->getHeight();
}

/*
  This one should be FAST !!
 */
UT_uint32 GR_MacGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	UT_UCSChar string[2];

	string [0] = c;
	string [1] = 0;
	return m_pMacFont->getTextWidth (string);
}


void GR_MacGraphics::setColor(UT_RGBColor& clr)
{
    ::CGContextSetRGBStrokeColor (m_CGContext, clr.m_red, clr.m_grn, clr.m_blu, 1.0f);
}


GR_Font* GR_MacGraphics::getGUIFont()
{
    // TODO: move this to GR_MacFont as it belongs to it.
    UT_DEBUGMSG (("HUB: GR_MacGraphics::getGUIFont() called\n"));
	ATSUStyle  theStyle;
    OSStatus err;
	GR_MacFont *theFont;

	err = m_pMacFontManager->_makeThemeATSUIStyle (kThemeApplicationFont, &theStyle);
	UT_ASSERT (err == noErr);
	if (err == noErr) 
	{
		theFont = new GR_MacFont (theStyle);
	}
	return theFont;
}

GR_Font* GR_MacGraphics::findFont(
		const char* pszFontFamily, 
		const char* pszFontStyle, 
		const char* pszFontVariant, 
		const char* pszFontWeight, 
		const char* pszFontStretch, 
		const char* pszFontSize)
{
	OSStatus err;
	UT_DEBUGMSG (("HUB: GR_MacGraphics::findFont() called\n"));
	ATSUStyle atsuiFont = m_pMacFontManager->findFont (pszFontFamily, pszFontStyle, 
														pszFontVariant, pszFontWeight, pszFontStretch, 
														convertDimension(pszFontSize));
	UT_ASSERT (atsuiFont);
    m_pMacFont = new GR_MacFont(atsuiFont);
	UT_ASSERT (m_pMacFont);
	err = ATSUDisposeStyle (atsuiFont);
    return(m_pMacFont);
}

void GR_MacGraphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
    CGContextBeginPath (m_CGContext);
    CGContextMoveToPoint (m_CGContext, x1, y1);
    CGContextAddLineToPoint (m_CGContext, x2, y2);
    CGContextClosePath (m_CGContext);
    CGContextStrokePath (m_CGContext);
}


void GR_MacGraphics::xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32)
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}

void GR_MacGraphics::setLineWidth(UT_sint32 w)
{
    CGContextSetLineWidth (m_CGContext, w);
}


void GR_MacGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
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
void GR_MacGraphics::fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
    CGRect myRect;

    myRect.origin.x = x;
    myRect.origin.y = y;
    myRect.size.width = w;
    myRect.size.height = h;
    CGContextSetRGBFillColor (m_CGContext, c.m_red, c.m_grn, c.m_blu, 1.0f);
    CGContextFillRect (m_CGContext, myRect);
}

void GR_MacGraphics::fillRect(UT_RGBColor& c, UT_Rect &r)
{
    fillRect (c, r.left, r.top, r.width, r.height);
}

        
void GR_MacGraphics::invertRect(const UT_Rect* pRect)
{
    CGContextSaveGState (m_CGContext);
    /* do the rect inversion */
    CGContextRestoreGState (m_CGContext);
    
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}
        
void GR_MacGraphics::setClipRect(const UT_Rect* pRect)
{
	/*
	  Assume that doing an empty path will do an emtpy clipping.
	  Check again later.
	 */
	m_pRect = pRect;
	if (pRect) 
	{
		CGRect myRect;
	
		myRect.origin.x = pRect->left;
		myRect.origin.y = pRect->top;
		myRect.size.width = pRect->width;
		myRect.size.height = pRect->height;
		
		CGContextClipToRect(m_CGContext, myRect);
	}
	else {
		CGContextBeginPath(m_CGContext);
		CGContextBeginPath(m_CGContext);
		CGContextClip (m_CGContext);
	}
}

void GR_MacGraphics::scroll(UT_sint32, UT_sint32, XAP_Frame * )
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}


void GR_MacGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height)
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}


void GR_MacGraphics::clearArea(UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
    CGRect myRect;

    myRect.origin.x = x;
    myRect.origin.y = y;
    myRect.size.width = w;
    myRect.size.height = h;
    CGContextSetRGBFillColor (m_CGContext, 0.f, 0.f, 0.f, 1.0f);
    CGContextFillRect (m_CGContext, myRect);
}

bool GR_MacGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
		return true;
	case DGP_PAPER:
		return false;
	default:
		UT_ASSERT(0);
		return false;
	}
}

	/* the following are only used for printing */

bool GR_MacGraphics::startPrint(void)
{
    // FIXIT
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return false;
}

bool GR_MacGraphics::startPage(const char * szPageLabel, UT_uint32 pageNumber,
							  bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight)
{
    // FIXIT
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return false;
}


bool GR_MacGraphics::endPrint(void)
{
    // FIXIT
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return false;
}

void GR_MacGraphics::setColorSpace(GR_Graphics::ColorSpace c)
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}


GR_Graphics::ColorSpace GR_MacGraphics::getColorSpace(void) const
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return GR_COLORSPACE_COLOR;
}

void GR_MacGraphics::setCursor(GR_Graphics::Cursor c)
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}

GR_Graphics::Cursor GR_MacGraphics::getCursor(void) const
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
    return GR_CURSOR_DEFAULT;
}

void GR_MacGraphics::setColor3D(GR_Color3D c)
{
    UT_RGBColor myC = m_3Dcolors [c];
    setColor (myC);
}


void GR_MacGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
    UT_RGBColor myC = m_3Dcolors [c];
    
    fillRect (myC, x, y, w, h);
}


void GR_MacGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
    UT_RGBColor myC = m_3Dcolors [c];
    
    fillRect (myC, r);    
}


UT_uint32 GR_MacGraphics::getFontAscent(GR_Font * font)
{
    // FIXME
    UT_ASSERT (font != NULL);
    return ((GR_MacFont *)font)->getAscent();
}


UT_uint32 GR_MacGraphics::getFontDescent(GR_Font *font)
{
    // FIXME
    UT_ASSERT (font != NULL);
    return ((GR_MacFont *)font)->getDescent();
}


UT_uint32 GR_MacGraphics::getFontHeight(GR_Font *font)
{
    // FIXME
    UT_ASSERT (font != NULL);
    return ((GR_MacFont *)font)->getHeight();
}



// Code below borrowed to gr_BeOSGraphics.cpp... FIXIT
//////////////////////////////////////////////////////////////////
// This is a static method in the GR_Font base class implemented
// in platform code.
//////////////////////////////////////////////////////////////////
void GR_Font::s_getGenericFontProperties(const char * szFontName,
										 FontFamilyEnum * pff,
										 FontPitchEnum * pfp,
										 bool * pbTrueType)
{
	// describe in generic terms the named font.

	// Note: most of the unix font handling code is in abi/src/af/xap/unix
	// Note: rather than in the graphics class.  i'm not sure this matters,
	// Note: but it is just different....

	// TODO add code to map the given font name into one of the
	// TODO enums in GR_Font and set *pff and *pft.

	*pff = FF_Unknown;
	*pfp = FP_Unknown;
	*pbTrueType = true;
}



