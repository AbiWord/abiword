/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2005, 2009 Hubert Figuiere
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#import <Cocoa/Cocoa.h>
#import <CoreServices/CoreServices.h>
#import <ApplicationServices/ApplicationServices.h>

#include <cairo/cairo-quartz.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_sleep.h"
#include "ut_endian.h"
#include "ut_OverstrikingChars.h"

#include "xap_CocoaApp.h"
#include "xap_CocoaAppController.h"
#include "gr_CocoaCairoGraphics.h"
#include "gr_CocoaImage.h"
#include "gr_CharWidths.h"
#include "xap_CocoaFrame.h"
#include "xap_CocoaCompat.h"

#include "xap_EncodingManager.h"

#define DISABLE_VERBOSE 0

#ifdef DISABLE_VERBOSE
# if DISABLE_VERBOSE
#  undef UT_DEBUGMSG
#  define UT_DEBUGMSG(x)
# endif
#endif


//#define LOCK_CONTEXT__	StNSViewLocker locker(m_pWin);
								//CGContextRef context = CG_CONTEXT__;
								//_setClipRectImpl(NULL);

//#define CG_CONTEXT__ (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort]

// create a stack object like that to lock a NSView, then it will be unlocked on scope exit.
// never do a new
class StNSViewLocker {
public:
	StNSViewLocker (NSView * view) {
		m_view = view;
		m_hasLock = [view lockFocusIfCanDraw];
	}
	~StNSViewLocker () {
		if (m_hasLock == YES) {
			[m_view unlockFocus];
		}
	}
private:
	NSView *m_view;
	BOOL m_hasLock;
};


#define TDUX(x) (_tduX(x))
// #define TDUX(x) (_tduX(x)+1.0)



bool      GR_CocoaCairoGraphics::m_colorAndImageInited = false;

UT_RGBColor * GR_CocoaCairoGraphics::m_colorBlue16x15 = NULL;
UT_RGBColor * GR_CocoaCairoGraphics::m_colorBlue11x16 = NULL;
UT_RGBColor * GR_CocoaCairoGraphics::m_colorGrey16x15 = NULL;
UT_RGBColor * GR_CocoaCairoGraphics::m_colorGrey11x16 = NULL;

NSCursor *	GR_CocoaCairoGraphics::m_Cursor_E = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_N = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_NE = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_NW = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_S = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_SE = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_SW = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_W = nil;

NSCursor *	GR_CocoaCairoGraphics::m_Cursor_Wait  = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_LeftArrow = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_RightArrow = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_Compass = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_Exchange = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_LeftRight = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_UpDown = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_Crosshair = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_HandPointer = nil;
NSCursor *	GR_CocoaCairoGraphics::m_Cursor_DownArrow = nil;

GR_Image* GR_CocoaCairoGraphicsBase::createNewImage(const char* pszName, const UT_ByteBuf* pBB,
	const std::string & mimeType, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight,
	GR_Image::GRType iType)
{
   	GR_Image* pImg = NULL;
   	if (iType == GR_Image::GRT_Raster)
   		pImg = new GR_CocoaImage(pszName);
   	else
	   	pImg = new GR_VectorImage(pszName);

	pImg->convertFromBuffer(pBB, mimeType, tdu(iDisplayWidth), tdu(iDisplayHeight));
   	return pImg;
}


GR_CocoaCairoGraphicsBase::GR_CocoaCairoGraphicsBase()
	: GR_CairoGraphics()
{
}

GR_CocoaCairoGraphicsBase::GR_CocoaCairoGraphicsBase(cairo_t *cr, UT_uint32 iDeviceResolution)
	: GR_CairoGraphics(cr, iDeviceResolution)
{
}


GR_CocoaCairoGraphicsBase::~GR_CocoaCairoGraphicsBase()
{
}


void GR_CocoaCairoGraphics::_initColorAndImage(void)
{
	NSBundle * bundle = [NSBundle mainBundle];
	NSString * path   = nil;
	NSImage  * image  = nil;

	if (m_colorAndImageInited) {
		return;
	}

	path = [bundle pathForResource:@"Blue16x15" ofType:@"png"];
	m_colorBlue16x15 = new UT_RGBColor(new GR_CairoPatternImpl([path UTF8String]));
	path = [bundle pathForResource:@"Blue11x16" ofType:@"png"];
	m_colorBlue11x16 = new UT_RGBColor(new GR_CairoPatternImpl([path UTF8String]));
	path = [bundle pathForResource:@"Grey16x15" ofType:@"png"];
	m_colorGrey16x15 = new UT_RGBColor(new GR_CairoPatternImpl([path UTF8String]));
	path = [bundle pathForResource:@"Grey11x16" ofType:@"png"];
	m_colorGrey11x16 = new UT_RGBColor(new GR_CairoPatternImpl([path UTF8String]));

	// Cursors
	path = [bundle pathForResource:@"Cursor_E" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_E = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,7)];
			[image release];
		}
	}
	if (!m_Cursor_E) {
		m_Cursor_E = [NSCursor arrowCursor];
		[m_Cursor_E retain];
	}
	path = [bundle pathForResource:@"Cursor_N" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_N = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,8)];
			[image release];
		}
	}
	if (!m_Cursor_N) {
		m_Cursor_N = [NSCursor arrowCursor];
		[m_Cursor_N retain];
	}
	path = [bundle pathForResource:@"Cursor_NE" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_NE = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,7)];
			[image release];
		}
	}
	if (!m_Cursor_NE) {
		m_Cursor_NE = [NSCursor arrowCursor];
		[m_Cursor_NE retain];
	}
	path = [bundle pathForResource:@"Cursor_NW" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_NW = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,7)];
			[image release];
		}
	}
	if (!m_Cursor_NW) {
		m_Cursor_NW = [NSCursor arrowCursor];
		[m_Cursor_NW retain];
	}
	path = [bundle pathForResource:@"Cursor_S" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_S = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,7)];
			[image release];
		}
	}
	if (!m_Cursor_S) {
		m_Cursor_S = [NSCursor arrowCursor];
		[m_Cursor_S retain];
	}
	path = [bundle pathForResource:@"Cursor_SE" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_SE = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,8)];
			[image release];
		}
	}
	if (!m_Cursor_SE) {
		m_Cursor_SE = [NSCursor arrowCursor];
		[m_Cursor_SE retain];
	}
	path = [bundle pathForResource:@"Cursor_SW" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_SW = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,8)];
			[image release];
		}
	}
	if (!m_Cursor_SW) {
		m_Cursor_SW = [NSCursor arrowCursor];
		[m_Cursor_SW retain];
	}
	path = [bundle pathForResource:@"Cursor_W" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_W = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,7)];
			[image release];
		}
	}
	if (!m_Cursor_W) {
		m_Cursor_W = [NSCursor arrowCursor];
		[m_Cursor_W retain];
	}

	path = [bundle pathForResource:@"Cursor_Wait" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_Wait = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,7)];
			[image release];
		}
	}
	if (!m_Cursor_Wait) {
		m_Cursor_Wait = [NSCursor arrowCursor];
		[m_Cursor_Wait retain];
	}
	path = [bundle pathForResource:@"Cursor_LeftArrow" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_LeftArrow = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,8)];
			[image release];
		}
	}
	if (!m_Cursor_LeftArrow) {
		m_Cursor_LeftArrow = [NSCursor arrowCursor];
		[m_Cursor_LeftArrow retain];
	}
	path = [bundle pathForResource:@"Cursor_RightArrow" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_RightArrow = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,7)];
			[image release];
		}
	}
	if (!m_Cursor_RightArrow) {
		m_Cursor_RightArrow = [NSCursor arrowCursor];
		[m_Cursor_RightArrow retain];
	}
	path = [bundle pathForResource:@"Cursor_Compass" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_Compass = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,7)];
			[image release];
		}
	}
	if (!m_Cursor_Compass) {
		m_Cursor_Compass = [NSCursor arrowCursor];
		[m_Cursor_Compass retain];
	}
	path = [bundle pathForResource:@"Cursor_Exchange" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_Exchange = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,7)];
			[image release];
		}
	}
	if (!m_Cursor_Exchange) {
		m_Cursor_Exchange = [NSCursor arrowCursor];
		[m_Cursor_Exchange retain];
	}
	path = [bundle pathForResource:@"leftright_cursor" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_LeftRight = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,8)];
			[image release];
		}
	}
	if (!m_Cursor_LeftRight) {
		m_Cursor_LeftRight = [NSCursor arrowCursor];
		[m_Cursor_LeftRight retain];
	}
	path = [bundle pathForResource:@"updown_cursor" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_UpDown = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,8)];
			[image release];
		}
	}
	if (!m_Cursor_UpDown) {
		m_Cursor_UpDown = [NSCursor arrowCursor];
		[m_Cursor_UpDown retain];
	}
	path = [bundle pathForResource:@"Cursor_Crosshair" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_Crosshair = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,7)];
			[image release];
		}
	}
	if (!m_Cursor_Crosshair) {
		m_Cursor_Crosshair = [NSCursor arrowCursor];
		[m_Cursor_Crosshair retain];
	}
	path = [bundle pathForResource:@"Cursor_HandPointer" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_HandPointer = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(6,0)];
			[image release];
		}
	}
	if (!m_Cursor_HandPointer) {
		m_Cursor_HandPointer = [NSCursor arrowCursor];
		[m_Cursor_HandPointer retain];
	}
	path = [bundle pathForResource:@"Cursor_DownArrow" ofType:@"png"];
	if (path) {
		image = [[NSImage alloc] initWithContentsOfFile:path];
		if (image) {
			m_Cursor_DownArrow = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(6,0)];
			[image release];
		}
	}
	if (!m_Cursor_DownArrow) {
		m_Cursor_DownArrow = [NSCursor arrowCursor];
		[m_Cursor_DownArrow retain];
	}
	
	m_colorAndImageInited = true;
}



GR_CocoaCairoGraphics::GR_CocoaCairoGraphics(XAP_CocoaNSView * win, bool /*double_buffered*/)
	: m_pWin(win),
	m_updateCallback(NULL),
	m_updateCBparam (NULL),
	m_cacheRectArray (10),
	m_cacheArray (10),
	m_GrabCursor(GR_CURSOR_DEFAULT),
	m_screenResolution(0),
	m_bIsPrinting(false),
	m_bIsDrawing(false),
	m_bDoShowPage(false)
{
	_initColorAndImage();
	init3dColors();

	UT_ASSERT (m_pWin);
	if (![m_pWin isKindOfClass:[XAP_CocoaNSView class]]) {
		NSLog(@"attaching a non-XAP_CocoaNSView to a GR_CocoaCairoGraphics");
	}

	[m_pWin setGraphics:this];
	[m_pWin allocateGState];

	m_cr = NULL;

	setCursor(GR_CURSOR_DEFAULT);	

	/* resolution does not change thru the life of the object */
	m_screenResolution = lrintf(_getScreenResolution());
	UT_DEBUGMSG(("screen resolution %d\n", m_screenResolution));
}


GR_CocoaCairoGraphics::~GR_CocoaCairoGraphics()
{
	_destroyFonts ();

	std::for_each(m_cacheArray.begin(),  m_cacheArray.end(), &cairo_surface_destroy);
	
	[m_pWin setGraphics:NULL];
}


UT_uint32 GR_CocoaCairoGraphics::_getResolution(void) const
{
	return m_screenResolution;
}

/*!
	Convert a UT_RGBColor to a NSColor
	\param clr the UT_RGBColor to convert
	\return an autoreleased NSColor

	Handle the transparency as well.
*/
NSColor *GR_CocoaCairoGraphics::_utRGBColorToNSColor (const UT_RGBColor& clr)
{
	float r,g,b;
	r = (float)clr.m_red / 255.0f;
	g = (float)clr.m_grn / 255.0f;
	b = (float)clr.m_blu / 255.0f;
	UT_DEBUGMSG (("converting color r=%f, g=%f, b=%f from %d, %d, %d\n", r, g, b, clr.m_red, clr.m_grn, clr.m_blu));
	NSColor *c = [NSColor colorWithDeviceRed:r green:g blue:b alpha:1.0/*(clr.m_bIsTransparent ? 0.0 : 1.0)*/];
	return c;
}


/*!
	Convert a NSColor to an UT_RGBColor
	\param c NSColor to convert
	\retval clr destination UT_RGBColor.
	
	Handle the transparency as well.
 */
void GR_CocoaCairoGraphics::_utNSColorToRGBColor (NSColor *c, UT_RGBColor &clr)
{
	UT_DEBUGMSG(("colorSpaceName %s\n", [[c colorSpaceName] UTF8String]));
	if([[c colorSpaceName] isEqualToString:NSPatternColorSpace]) {
		NSImage * img = [c patternImage];
		UT_ASSERT(img);
		NSSize size = [img size];
		UT_DEBUGMSG(("NSColor pattern size is %f, %f\n", size.width, size.height));
				
		cairo_surface_t * surface = cairo_quartz_surface_create(CAIRO_FORMAT_ARGB32, size.width, size.height);
		UT_DEBUGMSG(("status = %d\n", (int)cairo_surface_status(surface)));
		CGContextRef context = cairo_quartz_surface_get_cg_context(surface);

		NSGraphicsContext *gc = [NSGraphicsContext graphicsContextWithGraphicsPort:context flipped:NO]; 
		[NSGraphicsContext saveGraphicsState]; 
		[NSGraphicsContext setCurrentContext:gc];
		[img drawAtPoint:NSMakePoint(0,0) fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
		[NSGraphicsContext restoreGraphicsState];

		clr.setPattern(new GR_CairoPatternImpl(surface));
	}
	else {
		XAP_CGFloat r, g, b, a;
		[[c colorUsingColorSpaceName:NSDeviceRGBColorSpace] getRed:&r green:&g blue:&b alpha:&a];
		UT_DEBUGMSG(("color rgba is %f %f %f %f\n", r, g, b, a));
		clr.m_red = static_cast<unsigned char>(r * 255.0f);
		clr.m_grn = static_cast<unsigned char>(g * 255.0f);
		clr.m_blu = static_cast<unsigned char>(b * 255.0f);
		clr.m_bIsTransparent = false /*(a == 0.0f)*/;
	}
}


GR_Font * GR_CocoaCairoGraphics::getGUIFont(void)
{
	if (!m_pPFontGUI)
	{
		NSFont * font = [NSFont labelFontOfSize:[NSFont labelFontSize]];
		UT_UTF8String s = XAP_EncodingManager::get_instance()->getLanguageISOName();
		const char * pCountry
			= XAP_EncodingManager::get_instance()->getLanguageISOTerritory();
		if(pCountry)
		{
			s += "-";
			s += pCountry;
		}
		
		m_pPFontGUI = new GR_PangoFont([[font familyName] UTF8String], [NSFont labelFontSize], this, s.utf8_str(), true);
		UT_DEBUGMSG(("GR_CocoaCairoGraphics::getGUIFont: getting default font\n"));
		UT_ASSERT(m_pPFontGUI);
	}

	return m_pPFontGUI;
}



void GR_CocoaCairoGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
	if (!dx && !dy)
	{
		return;
	}

	UT_sint32 oldDY = tdu(getPrevYOffset());
	UT_sint32 oldDX = tdu(getPrevXOffset());
	UT_sint32 newY = getPrevYOffset() + dy;
	UT_sint32 newX = getPrevXOffset() + dx;
	UT_sint32 ddx = -(tdu(newX) - oldDX);
	UT_sint32 ddy = -(tdu(newY) - oldDY);
	setPrevYOffset(newY);
	setPrevXOffset(newX);

	[m_pWin displayIfNeeded];

	NSRect bounds = [m_pWin bounds];
	NSSize offset = NSMakeSize(ddx,ddy);
	[m_pWin scrollRect:bounds by:offset];

	if (offset.width > 0)
	{
		if (offset.width < bounds.size.width)
		{
			if (offset.height > 0)
			{
				if (offset.height < bounds.size.height)
				{
					NSRect tmp;
					tmp.origin.x    = bounds.origin.x + offset.width;
					tmp.origin.y    = bounds.origin.y;
					tmp.size.width  = bounds.size.width - offset.width;
					tmp.size.height = offset.height;
					[m_pWin setNeedsDisplayInRect:tmp];

					bounds.size.width = offset.width;
				}
			}
			else if (offset.height < 0)
			{
				if ((-offset.height) < bounds.size.height)
				{
					NSRect tmp;
					tmp.origin.x    = bounds.origin.x + offset.width;
					tmp.origin.y    = bounds.origin.y + bounds.size.height + offset.height;
					tmp.size.width  = bounds.size.width - offset.width;
					tmp.size.height = - offset.height;
					[m_pWin setNeedsDisplayInRect:tmp];

					bounds.size.width = offset.width;
				}
			}
			else
			{
				bounds.size.width = offset.width;
			}
		}
	}
	else if (offset.width < 0)
	{
		if ((-offset.width) < bounds.size.width)
		{
			if (offset.height > 0)
			{
				if (offset.height < bounds.size.height)
				{
					NSRect tmp;
					tmp.origin.x    = bounds.origin.x;
					tmp.origin.y    = bounds.origin.y;
					tmp.size.width  = bounds.size.width - offset.width;
					tmp.size.height = offset.height;
					[m_pWin setNeedsDisplayInRect:tmp];

					bounds.origin.x += bounds.size.width + offset.width;
					bounds.size.width = -offset.width;
				}
			}
			else if (offset.height < 0)
			{
				if ((-offset.height) < bounds.size.height)
				{
					NSRect tmp;
					tmp.origin.x    = bounds.origin.x;
					tmp.origin.y    = bounds.origin.y + bounds.size.height + offset.height;
					tmp.size.width  = bounds.size.width + offset.width;
					tmp.size.height = - offset.height;
					[m_pWin setNeedsDisplayInRect:tmp];

					bounds.origin.x += bounds.size.width + offset.width;
					bounds.size.width = -offset.width;
				}
			}
			else
			{
				bounds.origin.x += bounds.size.width + offset.width;
				bounds.size.width = -offset.width;
			}
		}
	}
	else
	{
		if (offset.height > 0)
		{
			if (offset.height < bounds.size.height)
			{
				bounds.size.height = offset.height;
			}
		}
		else if (offset.height < 0)
		{
			if ((-offset.height) < bounds.size.height)
			{
				bounds.origin.y += bounds.size.height + offset.height;
				bounds.size.height = -offset.height;
			}
		}
	}
	[m_pWin setNeedsDisplayInRect:bounds];
}

void GR_CocoaCairoGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  UT_sint32 width, UT_sint32 height)
{
	UT_sint32 dx = x_src - x_dest;
	UT_sint32 dy = y_src - y_dest;

	UT_sint32 oldDX = tdu(getPrevXOffset());
	UT_sint32 oldDY = tdu(getPrevYOffset());

	UT_sint32 newX = getPrevXOffset() + dx;
	UT_sint32 newY = getPrevYOffset() + dy;

	UT_sint32 ddx = oldDX - tdu(newX);
	UT_sint32 ddy = oldDY - tdu(newY);

	[m_pWin scrollRect:NSMakeRect(tdu(x_dest)-ddx, tdu(y_dest)-ddy, _tduR(width), _tduR(height)) by:NSMakeSize(ddx,ddy)];

	setPrevXOffset(newX);
	setPrevYOffset(newY);
}


void GR_CocoaCairoGraphics::setCursor(GR_Graphics::Cursor c)
{
	GR_Graphics::Cursor old_cursor = m_cursor;

	m_cursor = (c == GR_CURSOR_GRAB) ? m_GrabCursor : c;

	if (m_cursor == old_cursor)
		return;

	bool bImplemented = true;
	switch (m_cursor)
	{
	case GR_CURSOR_DEFAULT:
		[m_pWin setCursor:[NSCursor arrowCursor]];
		break;

	case GR_CURSOR_IBEAM:
		[m_pWin setCursor:[NSCursor IBeamCursor]];
		break;

	case GR_CURSOR_VLINE_DRAG:
		m_cursor = GR_CURSOR_LEFTRIGHT;
		// fall through...

	case GR_CURSOR_LEFTRIGHT:
		if (m_cursor != old_cursor)
		{
			[m_pWin setCursor:m_Cursor_LeftRight];
		}
		break;

	case GR_CURSOR_HLINE_DRAG:
		m_cursor = GR_CURSOR_UPDOWN;
		// fall through...

	case GR_CURSOR_UPDOWN:
		if (m_cursor != old_cursor)
		{
			[m_pWin setCursor:m_Cursor_UpDown];
		}
		break;

	case GR_CURSOR_IMAGE:
		[m_pWin setCursor:m_Cursor_Compass];
		break;

	case GR_CURSOR_IMAGESIZE_E:
		[m_pWin setCursor:m_Cursor_E];
		break;

	case GR_CURSOR_IMAGESIZE_N:
		[m_pWin setCursor:m_Cursor_N];
		break;

	case GR_CURSOR_IMAGESIZE_NE:
		[m_pWin setCursor:m_Cursor_NE];
		break;

	case GR_CURSOR_IMAGESIZE_NW:
		[m_pWin setCursor:m_Cursor_NW];
		break;

	case GR_CURSOR_IMAGESIZE_S:
		[m_pWin setCursor:m_Cursor_S];
		break;

	case GR_CURSOR_IMAGESIZE_SE:
		[m_pWin setCursor:m_Cursor_SE];
		break;

	case GR_CURSOR_IMAGESIZE_SW:
		[m_pWin setCursor:m_Cursor_SW];
		break;

	case GR_CURSOR_IMAGESIZE_W:
		[m_pWin setCursor:m_Cursor_W];
		break;

	case GR_CURSOR_WAIT:
		[m_pWin setCursor:m_Cursor_Wait];
		break;

	case GR_CURSOR_RIGHTARROW:
		[m_pWin setCursor:m_Cursor_RightArrow];
		break;

	case GR_CURSOR_LEFTARROW:
		[m_pWin setCursor:m_Cursor_LeftArrow];
		break;

	case GR_CURSOR_EXCHANGE:
		[m_pWin setCursor:m_Cursor_Exchange];
		break;

	case GR_CURSOR_CROSSHAIR:
		[m_pWin setCursor:m_Cursor_Crosshair];
		break;

	case GR_CURSOR_LINK:
		[m_pWin setCursor:m_Cursor_HandPointer];
		break;

	case GR_CURSOR_DOWNARROW:
		[m_pWin setCursor:m_Cursor_DownArrow];
		break;


	case GR_CURSOR_GRAB:
		NSLog(@"Cursor Grab (unimplemented)");
		bImplemented = false;
		// cursor_number = GDK_HAND1;
		break;

	case GR_CURSOR_INVALID:
		NSLog(@"Cursor Invalid (unimplemented)");
	default:
		NSLog(@"Unexpected cursor! (unimplemented)");
		bImplemented = false;
		break;
	}
	if (!bImplemented)
	{
		m_cursor = GR_CURSOR_DEFAULT;
		if (m_cursor != old_cursor)
		{
			[m_pWin setCursor:[NSCursor arrowCursor]];
		}
	}
	if (m_cursor != old_cursor)
	{
		[[m_pWin window] invalidateCursorRectsForView:m_pWin];
	}
}



void GR_CocoaCairoGraphics::init3dColors()
{
	UT_DEBUGMSG(("init3dColors()\n"));
	_utNSColorToRGBColor([NSColor blackColor], m_3dColors[CLR3D_Foreground]);
	_utNSColorToRGBColor([NSColor lightGrayColor], m_3dColors[CLR3D_Background]);
	_utNSColorToRGBColor([NSColor whiteColor], m_3dColors[CLR3D_BevelUp]);
	_utNSColorToRGBColor([NSColor darkGrayColor] /*[NSColor colorWithCalibratedWhite:0.0f alpha:0.25]*/, m_3dColors[CLR3D_BevelDown]);
	_utNSColorToRGBColor([NSColor whiteColor], m_3dColors[CLR3D_Highlight]); // [[NSColor controlHighlightColor] copy];

	m_bHave3DColors = true;
}


void GR_CocoaCairoGraphics::setIsPrinting(bool isprinting)
{
	m_bIsPrinting = isprinting;
}


void GR_CocoaCairoGraphics::_setUpdateCallback (gr_cocoa_graphics_update callback, void * param)
{
	m_updateCallback = callback;
	m_updateCBparam = param;
}

/*!
	Call the update Callback that has been set

	\param aRect: the rect that should be updated
	\return false if no callback. Otherwise returns what the callback returns.
 */
bool GR_CocoaCairoGraphics::_callUpdateCallback(NSRect * aRect)
{
	if (m_updateCallback == NULL) {
		return false;
	}
	m_bIsDrawing = true;
	bool ret = (*m_updateCallback) (aRect, this, m_updateCBparam);
	m_bIsDrawing = false;
	return ret;
}

bool GR_CocoaCairoGraphics::_isFlipped()
{
	return true;
}


GR_Image * GR_CocoaCairoGraphics::genImageFromRectangle(const UT_Rect & r)
{
	GR_CocoaImage *img = new GR_CocoaImage("ScreenShot");
	cairo_rectangle_t rect;
	
	rect.x = _tduX(r.left);
	rect.y = _tduY(r.top);
	rect.width = _tduR(r.width);
	rect.height = _tduR(r.height);
	
	cairo_surface_t * surface = NULL;
	surface = _getCairoSurfaceFromContext(m_cr, rect);
	img->setSurface(surface);
	cairo_surface_destroy(surface);
	img->setDisplaySize(rect.width, rect.height);
	return img;
}


void GR_CocoaCairoGraphics::saveRectangle(UT_Rect & rect,  UT_uint32 iIndx)
{
	cairo_rectangle_t cacheRect;
	cacheRect.x = static_cast<float>(_tduX(rect.left)) - 1.0f;
	cacheRect.y = static_cast<float>(_tduY(rect.top )) - 1.0f;
	cacheRect.width  = static_cast<float>(_tduR(rect.width )) + 2.0f;
	cacheRect.height = static_cast<float>(_tduR(rect.height)) + 2.0f;

	NSRect bounds = [m_pWin bounds];
	if (cacheRect.height > bounds.size.height - cacheRect.y) {
		cacheRect.height = bounds.size.height - cacheRect.y;
	}
	cairo_surface_t * cache;
	cache = _getCairoSurfaceFromContext(m_cr, cacheRect);
	cairo_surface_t * old = m_cacheArray[iIndx];
	m_cacheArray[iIndx] = cache;
	cairo_surface_destroy(old);
	m_cacheRectArray[iIndx] = cacheRect;
}


void GR_CocoaCairoGraphics::restoreRectangle(UT_uint32 iIndx)
{
	const cairo_rectangle_t & cacheRect = m_cacheRectArray[iIndx];
	cairo_surface_t * cache = m_cacheArray[iIndx];
	cairo_set_source_surface(m_cr, cache, cacheRect.x, cacheRect.y);
	cairo_paint(m_cr);
}


float	GR_CocoaCairoGraphics::_getScreenResolution(void)
{
	static float fResolution = 0.0;
	if (fResolution == 0.0) {
		NSScreen* mainScreen = [NSScreen mainScreen];
		NSDictionary* desc = [mainScreen deviceDescription];
		UT_ASSERT(desc);
		NSValue* value = [desc objectForKey:NSDeviceResolution];
		UT_ASSERT(value);
		fResolution = [value sizeValue].height;
	}
	return fResolution;
}

GR_Graphics *  GR_CocoaCairoGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	GR_CocoaCairoAllocInfo & allocator = static_cast<GR_CocoaCairoAllocInfo&>(info);
	
	return new GR_CocoaCairoGraphics(allocator.m_win, allocator.m_double_buffered);
}


void GR_CocoaCairoGraphics::_beginPaint ()
{
	m_cr = _createCairo(m_pWin);
	_initCairo();
}


void GR_CocoaCairoGraphics::_endPaint ()
{
	cairo_destroy(m_cr);
	m_cr = NULL;
}



bool GR_CocoaCairoGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
		case DGP_SCREEN:
		case DGP_OPAQUEOVERLAY:
			return !m_bIsPrinting;
		case DGP_PAPER:
			return m_bIsPrinting;
		default:
			UT_ASSERT_NOT_REACHED ();
			return false;
	}
}


bool GR_CocoaCairoGraphics::startPrint(void)
{
	if(!m_bIsPrinting) {
		return false;
	}
	m_bDoShowPage = false;
	return true;
}


bool GR_CocoaCairoGraphics::endPrint(void)
{
	if(!m_bIsPrinting) {
		return false;
	}
	if (m_bDoShowPage) {
		cairo_show_page(m_cr);
	}
	return true;
}


bool GR_CocoaCairoGraphics::startPage(const char * /*szPageLabel*/,
							  UT_uint32 /*pageNumber*/,
							  bool /*bPortrait*/,
							  UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
{
	if(!m_bIsPrinting) {
		return false;
	}
	if (m_bDoShowPage) {
		cairo_show_page(m_cr);
	}

	m_bDoShowPage = true;

	return true;
}


cairo_t *GR_CocoaCairoGraphics::_createCairo(NSView * view)
{
	// TODO: make sure we don't create unecessary context.
	// rule of thumb: if the CGContextRef is unchanged, use it.
	NSRect rect = [view bounds];
	cairo_t * cr = NULL;
	
	UT_ASSERT(view);
	
	BOOL locked = [view lockFocusIfCanDraw];
	// if it is not locked, we need to create the cairo_t anyway.
	CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
	cairo_surface_t * surface = cairo_quartz_surface_create_for_cg_context(context, 
							  rect.size.width, rect.size.height);
	cr = cairo_create(surface);
	cairo_surface_destroy(surface);
	
	if(locked) {
		[view unlockFocus];
	}
	return cr;
}

cairo_t *GR_CocoaCairoAllocInfo::createCairo()
{
	if(m_win) 
	{
		return GR_CocoaCairoGraphics::_createCairo(m_win);
	}
	return NULL;
}


