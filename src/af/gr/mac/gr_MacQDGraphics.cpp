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


#include <ATSUnicode.h>

#include "gr_MacFont.h"
#include "xap_MacFontManager.h"
#include "gr_MacQDGraphics.h"
#include <Appearance.h>


#include "ut_debugmsg.h"
#include "ut_assert.h"



GR_MacGraphics::GR_MacGraphics(GrafPtr grafPort, XAP_App * app)
      : GR_Graphics ()
{
      UT_ASSERT (grafPort != NULL);

      m_GrafPort = grafPort;
}

GR_MacGraphics::~GR_MacGraphics ()
{

}


/*
void GR_MacGraphics::drawChars(const UT_UCSChar* pChars, 
		int iCharOffset, int iLength, UT_sint32 xoff, UT_sint32 yoff)
void setFont(GR_Font* pFont);

UT_uint32 getFontAscent();
UT_uint32 getFontDescent();
UT_uint32 getFontHeight();
*/

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


UT_uint32 GR_MacGraphics::getFontAscent(GR_Font *pGRF)
{
    UT_ASSERT (pGRF != NULL);
    return pGRF->getAscent();
}


UT_uint32 GR_MacGraphics::getFontDescent(GR_Font *pGRF)
{
    UT_ASSERT (pGRF != NULL);
    return pGRF->getDescent();
}

// ---

UT_uint32 GR_MacGraphics::getFontHeight(GR_Font *pGRF)
{
    UT_ASSERT (pGRF != NULL);
    return pGRF->getHeight();
}


void GR_MacGraphics::scroll(UT_sint32, UT_sint32, XAP_Frame * )
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}

void GR_MacGraphics::scroll(UT_sint32, UT_sint32 )
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}

void GR_MacGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height)
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
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

void GR_MacGraphics::setColor3D(GR_Color3D c)
{
    UT_RGBColor myC = m_3Dcolors [c];
    setColor (myC);
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

void GR_MacGraphics::setClipRect(const UT_Rect* pRect)
{
	Rect r;
	/*
	  Assume that doing an empty path will do an empty clipping.
	  Check again later.
	 */
	m_pRect = pRect;
	if (pRect) 
	{
		r.top = pRect->top;
		r.left = pRect->left;
		r.bottom = pRect->top - pRect->height;
		r.right = pRect->left + pRect->width;
	}
	else {
		Rect r = { 0, 0, 0, 0 };
	}
	::ClipRect( &r );
}


void GR_MacGraphics::clearArea(UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	Rect r;
	r.top = y;
	r.left = x;
	r.bottom = y - h;
	r.right = x + w;

    ::EmptyRect ( &r );
}

bool GR_MacGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
	case DGP_OPAQUEOVERLAY:
		return true;
	case DGP_PAPER:
		return false;
	default:
		UT_ASSERT(0);
		return false;
	}
}

void GR_MacGraphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
/*
    CGContextBeginPath (m_CGContext);
    CGContextMoveToPoint (m_CGContext, x1, y1);
    CGContextAddLineToPoint (m_CGContext, x2, y2);
    CGContextClosePath (m_CGContext);
    CGContextStrokePath (m_CGContext);
*/
}


void GR_MacGraphics::xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32)
{
    UT_ASSERT (UT_NOT_IMPLEMENTED);
}

void GR_MacGraphics::setLineWidth(UT_sint32 w)
{
/*
    CGContextSetLineWidth (m_CGContext, w);
*/
}


void GR_MacGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
    UT_uint32 i;
/*
    CGContextBeginPath (m_CGContext);
    CGContextMoveToPoint (m_CGContext, pts[0].x, pts[0].y);
    for (i = 1; i < nPoints; i++) {
        CGContextAddLineToPoint (m_CGContext, pts[i].x, pts[i].y);
    }
    CGContextClosePath (m_CGContext);
    CGContextStrokePath (m_CGContext);
*/
}


void GR_MacGraphics::fillRect(UT_RGBColor& c, UT_Rect &r)
{
    fillRect (c, r.left, r.top, r.width, r.height);
}

        
void GR_MacGraphics::invertRect(const UT_Rect* pRect)
{
	Rect r;
	r.top = pRect->top;
	r.left = pRect->left;
	r.bottom = pRect->top - pRect->height;
	r.right = pRect->left + pRect->width;

    ::InvertRect ( &r );
}

void GR_MacGraphics::fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	Rect r;
	RGBColor rgb;
	
	r.top = y;
	r.left = x;
	r.bottom = y - h;
	r.right = x + w;

	rgb.red = c.m_red;
	rgb.green = c.m_grn;
	rgb.blue = c.m_blu;
	
    RGBForeColor ( &rgb );
    ::PaintRect ( &r );
}

void GR_MacGraphics::getColor(UT_RGBColor& clr)
{
	clr = m_curColor;
}

void GR_MacGraphics::setColor(const UT_RGBColor& clr)
{
	m_curColor = clr;
	RGBColor rgb;
	
	rgb.red = clr.m_red;
	rgb.green = clr.m_grn;
	rgb.blue = clr.m_blu;
	
    ::RGBForeColor ( &rgb );
}


GR_Font* GR_MacGraphics::getGUIFont()
{
    // TODO: move this to GR_MacFont as it belongs to it.
    UT_DEBUGMSG (("HUB: GR_MacGraphics::getGUIFont() called\n"));
	ATSUStyle  theStyle = NULL;
    OSStatus err;
	GR_MacFont *theFont = NULL;
#if UNIVERSAL_INTERFACE_VERSION <= 0x0332
	int iFont = kThemeSystemFont;
#else
	int iFont = kThemeApplicationFont;
#endif

#if 1
	err = m_pMacFontManager->_makeThemeATSUIStyle ( iFont, &theStyle);
	UT_ASSERT (err == noErr);
	if (err == noErr) 
	{
		theFont = new GR_MacFont (theStyle);
	}
#else
	// Default parameters ??	
	theFont = new GR_MacFont ( 0, 0, 12 );
#endif

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
#if 1
	ATSUStyle atsuiFont = m_pMacFontManager->findFont (pszFontFamily, pszFontStyle, 
														pszFontVariant, pszFontWeight, pszFontStretch, 
														convertDimension(pszFontSize));
	UT_ASSERT (atsuiFont);
    m_pMacFont = new GR_MacFont(atsuiFont);
	UT_ASSERT (m_pMacFont);
	err = ATSUDisposeStyle (atsuiFont);
#else
	m_pMacFont = new GR_MacFont ( 0, 0, 12 );
#endif
    return(m_pMacFont);
}

UT_uint32 GR_MacGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	UT_UCSChar string[2];

	string [0] = c;
	string [1] = 0;
	return m_pMacFont->getTextWidth (string);
}

void GR_MacGraphics::drawChars(const UT_UCSChar* pChars, 
		int iCharOffset, int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_DEBUGMSG (("drawChars: %d, %d\n", iCharOffset, iLength));
	ATSUTextLayout theLayout = NULL;
    OSStatus err;
//	ByteCount iSize = sizeof(CGContextRef);
//	ATSUAttributeTag iTag = kATSUCGContextTag;
//	ATSUAttributeValuePtr iValuePtr = &m_CGContext;
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
//	err = ATSUSetLayoutControls( theLayout, 1, &iTag, &iSize, &iValuePtr );
//	UT_ASSERT (err == noErr);

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
//	ATSFontRef	fontRef;
    
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

/* FIXME BPF for MacOS  Sunday, August 5, 2001 */
void GR_MacGraphics::_setOrigin (short x, short y)
{
	OSStatus err;
	//	err = ::SyncCGContextOriginWithPort(m_CGContext, m_qdPort);
	//	UT_ASSERT (err == noErr);
	
   //	::CGContextTranslateCTM(m_CGContext, x, y);
	UT_DEBUGMSG (("Syncing QD origin: (%d, %d)\n", x, y));
}


// ---

// Code below borrowed from gr_BeOSGraphics.cpp... FIXIT
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

