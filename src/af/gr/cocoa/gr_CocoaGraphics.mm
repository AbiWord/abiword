/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ut_endian.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFontManager.h"
#include "xap_CocoaFont.h"
#include "gr_CocoaGraphics.h"
#include "gr_CocoaImage.h"
#include "ut_sleep.h"
#include "xap_CocoaFrame.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"
//#include "ut_dialogHelper.h"
#include "ut_wctomb.h"
#include "xap_EncodingManager.h"
#include "ut_OverstrikingChars.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

// create a stack object like that to lock a NSView, then it will be unlocked on scope exit.
// never do a new
class StNSViewLocker {
public:
	StNSViewLocker (NSView * view) { 
		m_view = view; 
		[view lockFocusIfCanDraw]; 
	}
	~StNSViewLocker () {
		[m_view unlockFocus];
	}
private:
	NSView *m_view;
	
	void * operator new (size_t size);	// private so that we never call new for that class. Never defined.
};

//
// Below this size we use GDK fonts. Above it we use metric info.
//
#define MAX_ABI_GDK_FONT_SIZE 200

XAP_CocoaFontHandle *	GR_CocoaGraphics::s_pFontGUI = NULL;
UT_uint32 				GR_CocoaGraphics::s_iInstanceCount = 0;

GR_CocoaGraphics::GR_CocoaGraphics(NSView * win, XAP_CocoaFontManager * fontManager, XAP_App * app)
{
	m_pApp = app;
	m_pWin = win;
	m_pFontManager = fontManager;
	m_pFont = NULL;
	s_iInstanceCount++;

	StNSViewLocker locker(m_pWin);
	
 	[[NSColor blackColor] set];
		
	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
	m_cursor = GR_CURSOR_INVALID;
	setCursor(GR_CURSOR_DEFAULT);
}

GR_CocoaGraphics::~GR_CocoaGraphics()
{
	s_iInstanceCount--;
	if(!s_iInstanceCount) {
		DELETEP(s_pFontGUI);
	}
	for (int i = 0; i < COUNT_3D_COLORS; i++) {
		[m_3dColors[i] dealloc];
	}
}

bool GR_CocoaGraphics::queryProperties(GR_Graphics::Properties gp) const
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

/* let's cache this, since construction of UT_Wctomb is rather slow */
static UT_Wctomb* w = NULL;
static char text[MB_LEN_MAX];
static int text_length;
static bool fallback_used;

#define WCTOMB_DECLS \
	if (!w) {	\
	    w = new UT_Wctomb;	\
	} else	\
	    w->initialize();	
	    
#define CONVERT_TO_MBS(c)	\
    	if (c<=0xff) {	\
		/* this branch is to allow Lists to function */	\
		text[0] = (unsigned char)c;			\
		text_length = 1;				\
		fallback_used = 0;				\
	} else	{\
		fallback_used = 0;	\
		if (!w->wctomb(text,text_length,(wchar_t)c)) {	\
		    w->wctomb_or_fallback(text,text_length,(wchar_t)c);	\
		    fallback_used = 1;	\
		}	\
	}	


// HACK: I need more speed
void GR_CocoaGraphics::drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_UCSChar Wide_char = remapGlyph(Char, false);
	if(Wide_char == 0x200B || Wide_char == 0xFEFF) //zero width spaces
		return;
		
	NSFont *font = m_pFont->getNSFont();

	NSString * string = [[NSString alloc] initWithData:
							[NSData dataWithBytes:&Wide_char length:sizeof(Wide_char)] 
							encoding:NSUnicodeStringEncoding];

	StNSViewLocker locker(m_pWin);
	NSPoint point;
	point.x = xoff;
	point.y = yoff;
	
	// TODO: set attributes
	[string drawAtPoint:point withAttributes:nil];	
	[string dealloc];
}

void GR_CocoaGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
							 int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	if (!m_pFontManager)
		return;
	UT_ASSERT(m_pFont);
	WCTOMB_DECLS;
	NSFont *font = m_pFont->getNSFont();
	UT_sint32 x;

#ifdef BIDI_ENABLED	
	// to be able to handle overstriking characters, we have to remember the width
	// of the previous character printed
	// NB: overstriking characters are only supported under UTF-8, since on 8-bit locales
	// these are typically handled by combination glyphs

	static UT_sint32 prevWidth = 0;
	UT_sint32 curX;
	UT_sint32 curWidth;
#endif
	const UT_UCSChar *pC;
	
	// Lock the NSView
	StNSViewLocker locker(m_pWin);
	NSString * string = [NSString alloc];

  	for(pC=pChars+iCharOffset, x=xoff; pC<pChars+iCharOffset+iLength; ++pC)
	{
		UT_UCSChar actual = remapGlyph(*pC,false);
		if(actual == 0x200B || actual == 0xFEFF) //zero width spaces
			continue;
		[string initWithData:[NSData dataWithBytes:&actual length:sizeof(UT_UCSChar)] 
							encoding:NSUnicodeStringEncoding];
			
		/*	if the locale is unicode (i.e., utf-8) then we do not want
			to convert the UCS string to anything,
			gdk_draw_text can draw 16-bit string, if the font is
			a matrix; however the string is interpreted as big-endian
		*/
		//unicode font
		//UT_DEBUGMSG(("CocoaGraphics::drawChars: utf-8\n"));
#ifdef BIDI_ENABLED
		switch(isOverstrikingChar(*pC))
		{
		case UT_NOT_OVERSTRIKING:
			curWidth = [font widthOfString:string];
			curX = x;
			break;
		case UT_OVERSTRIKING_RTL:
			curWidth = 0;
			curX = x;
			break;
		case UT_OVERSTRIKING_LTR:
			curWidth =  prevWidth;
			curX = x - prevWidth;
			break;
		}
	
		NSPoint point;
		point.x = curX;
		point.y = yoff;

		// TODO: set attributes
		[string drawAtPoint:point withAttributes:nil];	
		
		//gdk_draw_text(m_pWin,font,m_pGC,curX,yoff+font->ascent,(gchar*)&beucs,2);
		x+=curWidth;
		prevWidth = curWidth;
#else
		NSPoint point;
		point.x = x;
		point.y = yoff;

		// TODO: set attributes
		[string drawAtPoint:point withAttributes:nil];	
		//gdk_draw_text(m_pWin,font,m_pGC,x,yoff+font->ascent,(gchar*)&beucs,2);
		x += [font widthOfString:string];
		//x+=gdk_text_width(font, (gchar*)&beucs, 2);
#endif
	}
	
	[string dealloc];
	flush();
}

void GR_CocoaGraphics::setFont(GR_Font * pFont)
{
	XAP_CocoaFontHandle * pUFont = static_cast<XAP_CocoaFontHandle *> (pFont);
#if 1	
	// this is probably caching done on the wrong level
	// but it's currently faster to shortcut
	// than to call explodeGdkFonts
	// TODO: turn this off when our text runs get a bit smarter
	if(m_pFont && (pUFont->getCocoaFont() == m_pFont->getCocoaFont()) && 
	   (pUFont->getSize() == m_pFont->getSize()))
	  return;
#endif

	m_pFont = pUFont;
//
// Only use gdk fonts for Low resolution
//
//	if(pUFont->getSize()< MAX_ABI_GDK_FONT_SIZE)
//	{
//		m_pFont->explodeGdkFonts(m_pSingleByteFont,m_pMultiByteFont);
//	}
}

UT_uint32 GR_CocoaGraphics::getFontHeight(GR_Font * fnt)
{
	return getFontAscent(fnt)+getFontDescent(fnt);
}

UT_uint32 GR_CocoaGraphics::getFontHeight()
{
	if (!m_pFontManager)
		return 0;

	return getFontAscent()+getFontDescent();
}

UT_uint32 GR_CocoaGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	// measureString() could be defined in terms of measureUnRemappedChar()
	// but its not (for presumed performance reasons).  Also, a difference
	// is that measureString() uses remapping to get past zero-width
	// character cells.
	
	if(c == 0x200B || c == 0xFEFF) // 0-with spaces
		return 0;

	UT_ASSERT(m_pFont);

	NSFont * font = m_pFont->getNSFont();
	NSString * string = [[NSString alloc] initWithData:[NSData dataWithBytes:&c length:sizeof(UT_UCSChar)] 
							encoding:NSUnicodeStringEncoding];
	float w = [font widthOfString:string];
	[string dealloc];
	
	return w;
}


UT_uint32 GR_CocoaGraphics::_getResolution(void) const
{
	// this is hard-coded at 100 for X now, since 75 (which
	// most X servers return when queried for a resolution)
	// makes for tiny fonts on modern resolutions.

	return 100;
}


void GR_CocoaGraphics::setColor(const UT_RGBColor& clr)
{
	NSColor *c = [NSColor colorWithDeviceRed:clr.m_red green:clr.m_grn blue:clr.m_blu alpha:1.0];
	_setColor(c);
	[c dealloc];
}

void GR_CocoaGraphics::_setColor(NSColor * c)
{
	StNSViewLocker locker(m_pWin);
	[c set];
}

GR_Font * GR_CocoaGraphics::getGUIFont(void)
{
	if (!m_pFontManager)
		return NULL;
	
	if (!s_pFontGUI)
	{
		// get the font resource
		//UT_DEBUGMSG(("GR_CocoaGraphics::getGUIFont: getting default font\n"));
		XAP_CocoaFont * font = (XAP_CocoaFont *) m_pFontManager->getDefaultFont();
		UT_ASSERT(font);

		// bury it in a new font handle
		s_pFontGUI = new XAP_CocoaFontHandle(font, [NSFont labelFontSize]); // Hardcoded GUI font size
		UT_ASSERT(s_pFontGUI);
	}

	return s_pFontGUI;
}

GR_Font * GR_CocoaGraphics::findFont(const char* pszFontFamily, 
									const char* pszFontStyle, 
									const char* /*pszFontVariant*/, 
									const char* pszFontWeight, 
									const char* /*pszFontStretch*/, 
									const char* pszFontSize)
{
	if (!m_pFontManager)
		return NULL;

	UT_ASSERT(pszFontFamily);
	UT_ASSERT(pszFontStyle);
	UT_ASSERT(pszFontWeight);
	UT_ASSERT(pszFontSize);
	
	// convert styles to XAP_CocoaFont:: formats
	XAP_CocoaFont::style s = XAP_CocoaFont::STYLE_NORMAL;

	// this is kind of sloppy
	if (!UT_strcmp(pszFontStyle, "normal") &&
		!UT_strcmp(pszFontWeight, "normal"))
	{
		s = XAP_CocoaFont::STYLE_NORMAL;
	}
	else if (!UT_strcmp(pszFontStyle, "normal") &&
			 !UT_strcmp(pszFontWeight, "bold"))
	{
		s = XAP_CocoaFont::STYLE_BOLD;
	}
	else if (!UT_strcmp(pszFontStyle, "italic") &&
			 !UT_strcmp(pszFontWeight, "normal"))
	{
		s = XAP_CocoaFont::STYLE_ITALIC;
	}
	else if (!UT_strcmp(pszFontStyle, "italic") &&
			 !UT_strcmp(pszFontWeight, "bold"))
	{
		s = XAP_CocoaFont::STYLE_BOLD_ITALIC;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	// Request the appropriate XAP_CocoaFont
	XAP_CocoaFont * cocoafont = m_pFontManager->getFont(pszFontFamily, s);	
	if (!cocoafont)
	{
		// Oops!  We don't have that font here.
		// first try "Times New Roman", which should be sensible, and should
		// be there unless the user fidled with the installation
		cocoafont = m_pFontManager->getFont("Times New Roman", s);
		
		// Oh well, see if there are any fonts at all, and if so
		// just take the first one ...
		if(!cocoafont)
		{
				UT_Vector *	pVec = m_pFontManager->getAllFonts();
				if(pVec && pVec->getItemCount() > 0)
				{
					// get the first font we have
					cocoafont = static_cast<XAP_CocoaFont *>(pVec->getNthItem(0));
				}
		
		}
	}

	// bury the pointer to our Cocoa font in a XAP_CocoaFontHandle with the correct size.

//
// This piece of code scales the FONT chosen at low resolution to that at high
// resolution. This fixes bug 1632 and other non-WYSIWYG behaviour.
//
	UT_uint32 iSize = getAppropriateFontSizeFromString(pszFontSize);
	XAP_CocoaFontHandle * pFont = new XAP_CocoaFontHandle(cocoafont, iSize);
	UT_ASSERT(pFont);

	return pFont;
}

UT_uint32 GR_CocoaGraphics::getFontAscent(GR_Font * fnt)
{
	UT_ASSERT(fnt);

	XAP_CocoaFontHandle * hndl = static_cast<XAP_CocoaFontHandle *>(fnt);

	NSFont* pFont = hndl->getNSFont();
	return [pFont ascender];
}

UT_uint32 GR_CocoaGraphics::getFontAscent()
{
	return getFontAscent(m_pFont);
}

UT_uint32 GR_CocoaGraphics::getFontDescent(GR_Font * fnt)
{
	UT_ASSERT(fnt);

	XAP_CocoaFontHandle * hndl = static_cast<XAP_CocoaFontHandle *>(fnt);

	NSFont* pFont = hndl->getNSFont();
	return [pFont descender];
}

UT_uint32 GR_CocoaGraphics::getFontDescent()
{
	return getFontDescent(m_pFont);
}

void GR_CocoaGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
							   UT_sint32 x2, UT_sint32 y2)
{
	// TODO set the line width according to m_iLineWidth	
	StNSViewLocker locker(m_pWin);
	[NSBezierPath strokeLineFromPoint:NSMakePoint(x1, y1) toPoint:NSMakePoint(x2, y2)];
}

void GR_CocoaGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	m_iLineWidth = iLineWidth;

	StNSViewLocker locker(m_pWin);
	[NSBezierPath setDefaultLineWidth:iLineWidth];
}

void GR_CocoaGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
			    UT_sint32 y2)
{
	// TODO use XOR mode
	StNSViewLocker locker(m_pWin);
	[NSBezierPath strokeLineFromPoint:NSMakePoint(x1, y1) toPoint:NSMakePoint(x2, y2)];
}

void GR_CocoaGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
	NSBezierPath *path = [NSBezierPath bezierPath];
	
	for (UT_uint32 i = 0; i < nPoints; i++) 
	{
		if (i == 0) {
			[path moveToPoint:NSMakePoint (pts[i].x, pts[i].y)];
		}
		else {
			[path lineToPoint:NSMakePoint (pts[i].x, pts[i].y)];
		}
	}
	StNSViewLocker locker(m_pWin);
	[path stroke];
	[path dealloc];
}

void GR_CocoaGraphics::invertRect(const UT_Rect* pRect)
{
	UT_ASSERT(pRect);

	StNSViewLocker locker(m_pWin);
	// TODO handle invert
	[NSBezierPath fillRect:NSMakeRect (pRect->left, pRect->top, 
	                                   pRect->left + pRect->width, pRect->top + pRect->height)];
}

void GR_CocoaGraphics::setClipRect(const UT_Rect* pRect)
{
	m_pRect = pRect;

	StNSViewLocker locker(m_pWin);
	NSRect aRect = NSMakeRect (pRect->left, pRect->top, 
	                           pRect->left + pRect->width, pRect->top + pRect->height);
	
	if (pRect)
	{
		[NSBezierPath clipRect:aRect];
	}
}

void GR_CocoaGraphics::fillRect(UT_RGBColor& c, UT_Rect &r)
{
	fillRect(c,r.left,r.top,r.width,r.height);
}

void GR_CocoaGraphics::fillRect(UT_RGBColor& clr, UT_sint32 x, UT_sint32 y,
							   UT_sint32 w, UT_sint32 h)
{
	// save away the current color, and restore it after we fill the rect
	NSColor *c = [NSColor colorWithDeviceRed:clr.m_red green:clr.m_grn blue:clr.m_blu alpha:1.0];
	NSBezierPath *path = [NSBezierPath bezierPathWithRect:NSMakeRect (x, y, x + w, y + h)];
	
	StNSViewLocker locker(m_pWin);
	[NSGraphicsContext saveGraphicsState];
	[c set];
	[path stroke];	
	[NSGraphicsContext restoreGraphicsState];

	[c dealloc];
	[path dealloc];
}

void GR_CocoaGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void GR_CocoaGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  UT_sint32 width, UT_sint32 height)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void GR_CocoaGraphics::clearArea(UT_sint32 x, UT_sint32 y,
							 UT_sint32 width, UT_sint32 height)
{
  //	UT_DEBUGMSG(("ClearArea: %d %d %d %d\n", x, y, width, height));
	if (width > 0)
	{
		UT_RGBColor clrWhite(255,255,255);
		fillRect(clrWhite, x, y, width, height);
	}
}

bool GR_CocoaGraphics::startPrint(void)
{
	UT_ASSERT(0);
	return false;
}

bool GR_CocoaGraphics::startPage(const char * /*szPageLabel*/, UT_uint32 /*pageNumber*/,
								bool /*bPortrait*/, UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
{
	UT_ASSERT(0);
	return false;
}

bool GR_CocoaGraphics::endPrint(void)
{
	UT_ASSERT(0);
	return false;
}


GR_Image* GR_CocoaGraphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
{
   	GR_Image* pImg = NULL;
   	if (iType == GR_Image::GRT_Raster)
   		pImg = new GR_CocoaImage(pszName);
   	else
	   	pImg = new GR_VectorImage(pszName);
	     
	pImg->convertFromBuffer(pBB, iDisplayWidth, iDisplayHeight);
   	return pImg;
}

void GR_CocoaGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);

   	if (pImg->getType() != GR_Image::GRT_Raster) {
	   	pImg->render(this, xDest, yDest);
	   	return;
	}
	   
   	GR_CocoaImage * pCocoaImage = static_cast<GR_CocoaImage *>(pImg);

   	Fatmap * image = pCocoaImage->getData();

	if (image == 0)
	{
		UT_DEBUGMSG(("Found no image data. This is probably SVG masquerading as a raster!\n"));
		return;
	}

   	UT_sint32 iImageWidth = pCocoaImage->getDisplayWidth();
   	UT_sint32 iImageHeight = pCocoaImage->getDisplayHeight();

	UT_ASSERT(UT_NOT_IMPLEMENTED);
	/*
   	gdk_draw_rgb_image(m_pWin,
			   m_pGC,
			   xDest,
			   yDest,
			   iImageWidth,
			   iImageHeight,
			   GDK_RGB_DITHER_NORMAL,
			   image->data,
			   image->width * 3); // This parameter is the total bytes across one row,
                                              // which is pixels * 3 (we use 3 bytes per pixel).

	*/
}


void GR_CocoaGraphics::flush(void)
{
	[[NSGraphicsContext currentContext] flushGraphics];
}

void GR_CocoaGraphics::setColorSpace(GR_Graphics::ColorSpace /* c */)
{
	// we only use ONE color space here now (GdkRGB's space)
	// and we don't let people change that on us.
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

GR_Graphics::ColorSpace GR_CocoaGraphics::getColorSpace(void) const
{
	return m_cs;
}

void GR_CocoaGraphics::setCursor(GR_Graphics::Cursor c)
{
	UT_DEBUGMSG(("Don't support setting cursor\n"));
#if 0
	if (m_cursor == c)
		return;
	
	m_cursor = c;
	
	GdkCursorType cursor_number;
	
	switch (c)
	{
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		/*FALLTHRU*/
	case GR_CURSOR_DEFAULT:
		cursor_number = GDK_TOP_LEFT_ARROW;
		break;
		
	case GR_CURSOR_IBEAM:
		cursor_number = GDK_XTERM;
		break;

	//I have changed the shape of the arrow so get a consistent
	//behaviour in the bidi build; I think the new arrow is better
	//for the purpose anyway
	
	case GR_CURSOR_RIGHTARROW:
		cursor_number = GDK_SB_RIGHT_ARROW; //GDK_ARROW;
		break;

#ifdef BIDI_ENABLED
//#error choose a suitable cursor; this is just a placeholder !!!		
	case GR_CURSOR_LEFTARROW:
		cursor_number = GDK_SB_LEFT_ARROW; //GDK_LEFT_PTR;
		break;
#endif		

	case GR_CURSOR_IMAGE:
		cursor_number = GDK_FLEUR;
		break;
		
	case GR_CURSOR_IMAGESIZE_NW:
		cursor_number = GDK_TOP_LEFT_CORNER;
		break;
		
	case GR_CURSOR_IMAGESIZE_N:
		cursor_number = GDK_TOP_SIDE;
		break;
		
	case GR_CURSOR_IMAGESIZE_NE:
		cursor_number = GDK_TOP_RIGHT_CORNER;
		break;
		
	case GR_CURSOR_IMAGESIZE_E:
		cursor_number = GDK_RIGHT_SIDE;
		break;
		
	case GR_CURSOR_IMAGESIZE_SE:
		cursor_number = GDK_BOTTOM_RIGHT_CORNER;
		break;
		
	case GR_CURSOR_IMAGESIZE_S:
		cursor_number = GDK_BOTTOM_SIDE;
		break;
		
	case GR_CURSOR_IMAGESIZE_SW:
		cursor_number = GDK_BOTTOM_LEFT_CORNER;
		break;
		
	case GR_CURSOR_IMAGESIZE_W:
		cursor_number = GDK_LEFT_SIDE;
		break;
		
	case GR_CURSOR_LEFTRIGHT:
		cursor_number = GDK_SB_H_DOUBLE_ARROW;
		break;

	case GR_CURSOR_UPDOWN:
		cursor_number = GDK_SB_V_DOUBLE_ARROW;
		break;

	case GR_CURSOR_EXCHANGE:
		cursor_number = GDK_EXCHANGE;
		break;

	case GR_CURSOR_GRAB:
		cursor_number = GDK_HAND1;
		break;
	}

	GdkCursor * cursor = gdk_cursor_new(cursor_number);
	gdk_window_set_cursor(m_pWin, cursor);
	gdk_cursor_destroy(cursor);
#endif
}

GR_Graphics::Cursor GR_CocoaGraphics::getCursor(void) const
{
	return m_cursor;
}

void GR_CocoaGraphics::setColor3D(GR_Color3D c)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	_setColor(m_3dColors[c]);
}

void GR_CocoaGraphics::init3dColors()
{
	m_3dColors[CLR3D_Foreground] = [NSColor controlColor];
	m_3dColors[CLR3D_Background] = [NSColor controlBackgroundColor];
	m_3dColors[CLR3D_BevelUp] = [NSColor controlShadowColor];
	m_3dColors[CLR3D_BevelDown] = [NSColor controlDarkShadowColor];
	m_3dColors[CLR3D_Highlight] = [NSColor controlHighlightColor];
}

void GR_CocoaGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	NSBezierPath *path = [NSBezierPath bezierPathWithRect:NSMakeRect (x, y, x + w, y + h)];
	
	StNSViewLocker locker(m_pWin);
	[NSGraphicsContext saveGraphicsState];
	[m_3dColors[c] set];
	[path stroke];	
	[NSGraphicsContext restoreGraphicsState];

	[path dealloc];
}

void GR_CocoaGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	fillRect(c,r.left,r.top,r.width,r.height);
}

void GR_CocoaGraphics::polygon(UT_RGBColor& clr,UT_Point *pts,UT_uint32 nPoints)
{
	NSBezierPath *path = [NSBezierPath bezierPath];
	
	for (UT_uint32 i = 0; i < nPoints; i++) 
	{
		if (i == 0) {
			[path moveToPoint:NSMakePoint (pts[i].x, pts[i].y)];
		}
		else {
			[path lineToPoint:NSMakePoint (pts[i].x, pts[i].y)];
		}
	}
	[path closePath];
	NSColor *c = [NSColor colorWithDeviceRed:clr.m_red green:clr.m_grn blue:clr.m_blu alpha:1.0];
	StNSViewLocker locker(m_pWin);
	[NSGraphicsContext saveGraphicsState];
	[c set];
	[path stroke];
	[NSGraphicsContext restoreGraphicsState];
	[path dealloc];
	[c dealloc];
}

//////////////////////////////////////////////////////////////////
// This is a static method in the GR_Font base class implemented
// in platform code.
//////////////////////////////////////////////////////////////////

void GR_Font::s_getGenericFontProperties(const char * /*szFontName*/,
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

