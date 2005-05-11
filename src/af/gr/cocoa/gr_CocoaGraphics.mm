/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2004 Hubert Figuiere
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

//#define USE_OFFSCREEN 1
#undef USE_OFFSCREEN

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ut_endian.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaAppController.h"
#include "xap_CocoaFont.h"
#include "gr_CocoaGraphics.h"
#include "gr_CocoaImage.h"
#include "gr_CharWidths.h"
#include "ut_sleep.h"
#include "xap_CocoaFrame.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"

#include "xap_EncodingManager.h"
#include "ut_OverstrikingChars.h"

#import <Cocoa/Cocoa.h>
#import <ApplicationServices/ApplicationServices.h>

#define DISABLE_VERBOSE 1

#ifdef DISABLE_VERBOSE
# if DISABLE_VERBOSE
#  undef UT_DEBUGMSG
#  define UT_DEBUGMSG(x)
# endif
#endif

#define CONTEXT_LOCKED__ m_viewLocker
#define LOCK_CONTEXT__   UT_ASSERT(CONTEXT_LOCKED__)

/*#define LOCK_CONTEXT__	StNSViewLocker locker(m_pWin); \
								m_CGContext = CG_CONTEXT__; \
								_setClipRectImpl(NULL);
*/

#define CG_CONTEXT__ (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort]

#define TDUX(x) (_tduX(x))
// #define TDUX(x) (_tduX(x)+1.0)

// create a stack object like that to lock a NSView, then it will be unlocked on scope exit.
// never do a new
class StNSViewLocker {
public:
	StNSViewLocker (NSView * view) {
		m_view = view;
		m_hasLock = [view lockFocusIfCanDraw];
//		UT_ASSERT(m_hasLock);
	}
	~StNSViewLocker () {
		if (m_hasLock == YES) {
			[m_view unlockFocus];
		}
	}
private:
	NSView *m_view;
	BOOL m_hasLock;

	//void * operator new (size_t size);	// private so that we never call new for that class. Never defined.
};

class StNSImageLocker {
public:
	StNSImageLocker (NSView * pView, NSImage * img) {
		m_image = img; m_pView = pView;
		[img lockFocus];
	}
	~StNSImageLocker () {
		[m_image unlockFocus];
		[m_pView setNeedsDisplay:YES];
	}
private:
	NSImage *m_image; 
	NSView * m_pView;

	void * operator new (size_t size);	// private so that we never call new for that class. Never defined.
};


UT_uint32 				GR_CocoaGraphics::s_iInstanceCount = 0;

const char* GR_Graphics::findNearestFont(const char* pszFontFamily,
										 const char* pszFontStyle,
										 const char* pszFontVariant,
										 const char* pszFontWeight,
										 const char* pszFontStretch,
										 const char* pszFontSize)
{
	return pszFontFamily;
}

GR_CocoaGraphics::GR_CocoaGraphics(NSView * win, XAP_App * app) :
	m_updateCallback(NULL),
	m_updateCBparam (NULL),
	m_pWin(win),
	m_cacheArray (10),
	m_cacheRectArray (10),
	m_currentColor (nil),
	m_imageBlue16x15 (nil),
	m_imageBlue11x16 (nil),
	m_imageGrey16x15 (nil),
	m_imageGrey11x16 (nil),
	m_colorBlue16x15 (nil),
	m_colorBlue11x16 (nil),
	m_colorGrey16x15 (nil),
	m_colorGrey11x16 (nil),
	m_pFont (NULL),
	m_fontForGraphics (nil),
	m_pFontGUI(NULL),
	m_fLineWidth(1.0),
	m_joinStyle(JOIN_MITER),
	m_capStyle(CAP_BUTT),
	m_lineStyle(LINE_SOLID),
	m_Cursor_E (nil),
	m_Cursor_N (nil),
	m_Cursor_NE (nil),
	m_Cursor_NW (nil),
	m_Cursor_S (nil),
	m_Cursor_SE (nil),
	m_Cursor_SW (nil),
	m_Cursor_W (nil),
	m_Cursor_Wait (nil),
	m_Cursor_LeftArrow (nil),
	m_Cursor_RightArrow (nil),
	m_Cursor_Compass (nil),
	m_Cursor_Exchange (nil),
	m_Cursor_LeftRight (nil),
	m_Cursor_UpDown (nil),
	m_Cursor_Crosshair (nil),
	m_Cursor_HandPointer (nil),
	m_Cursor_DownArrow (nil),
	m_GrabCursor(GR_CURSOR_DEFAULT),
	m_screenResolution(0),
	m_bIsPrinting(false),
	m_bIsDrawing(false),
	m_viewLocker(NULL),
	m_fontMetricsTextStorage(nil),
	m_fontMetricsLayoutManager(nil),
	m_fontMetricsTextContainer(nil)
{
	NSBundle * bundle = [NSBundle mainBundle];
	NSString * path   = 0;
	NSImage  * image  = 0;

	if (path = [bundle pathForResource:@"Blue16x15" ofType:@"png"])
		if (m_imageBlue16x15 = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_colorBlue16x15 = [NSColor colorWithPatternImage:m_imageBlue16x15];
			[m_colorBlue16x15 retain];
		}
	if (!m_colorBlue16x15) {
		m_colorBlue16x15 = [NSColor blueColor];
		[m_colorBlue16x15 retain];
	}
	if (path = [bundle pathForResource:@"Blue11x16" ofType:@"png"])
		if (m_imageBlue11x16 = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_colorBlue11x16 = [NSColor colorWithPatternImage:m_imageBlue11x16];
			[m_colorBlue11x16 retain];
		}
	if (!m_colorBlue11x16) {
		m_colorBlue11x16 = [NSColor blueColor];
		[m_colorBlue11x16 retain];
	}
	if (path = [bundle pathForResource:@"Grey16x15" ofType:@"png"])
		if (m_imageGrey16x15 = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_colorGrey16x15 = [NSColor colorWithPatternImage:m_imageGrey16x15];
			[m_colorGrey16x15 retain];
		}
	if (!m_colorGrey16x15) {
		m_colorGrey16x15 = [NSColor grayColor];
		[m_colorGrey16x15 retain];
	}
	if (path = [bundle pathForResource:@"Grey11x16" ofType:@"png"])
		if (m_imageGrey11x16 = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_colorGrey11x16 = [NSColor colorWithPatternImage:m_imageGrey11x16];
			[m_colorGrey11x16 retain];
		}
	if (!m_colorGrey11x16) {
		m_colorGrey11x16 = [NSColor grayColor];
		[m_colorGrey11x16 retain];
	}

	if (path = [bundle pathForResource:@"Cursor_E" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_E = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,7)];
			[image release];
		}
	if (!m_Cursor_E) {
		m_Cursor_E = [NSCursor arrowCursor];
		[m_Cursor_E retain];
	}
	if (path = [bundle pathForResource:@"Cursor_N" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_N = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,8)];
			[image release];
		}
	if (!m_Cursor_N) {
		m_Cursor_N = [NSCursor arrowCursor];
		[m_Cursor_N retain];
	}
	if (path = [bundle pathForResource:@"Cursor_NE" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_NE = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,7)];
			[image release];
		}
	if (!m_Cursor_NE) {
		m_Cursor_NE = [NSCursor arrowCursor];
		[m_Cursor_NE retain];
	}
	if (path = [bundle pathForResource:@"Cursor_NW" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_NW = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,7)];
			[image release];
		}
	if (!m_Cursor_NW) {
		m_Cursor_NW = [NSCursor arrowCursor];
		[m_Cursor_NW retain];
	}
	if (path = [bundle pathForResource:@"Cursor_S" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_S = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,7)];
			[image release];
		}
	if (!m_Cursor_S) {
		m_Cursor_S = [NSCursor arrowCursor];
		[m_Cursor_S retain];
	}
	if (path = [bundle pathForResource:@"Cursor_SE" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_SE = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,8)];
			[image release];
		}
	if (!m_Cursor_SE) {
		m_Cursor_SE = [NSCursor arrowCursor];
		[m_Cursor_SE retain];
	}
	if (path = [bundle pathForResource:@"Cursor_SW" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_SW = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,8)];
			[image release];
		}
	if (!m_Cursor_SW) {
		m_Cursor_SW = [NSCursor arrowCursor];
		[m_Cursor_SW retain];
	}
	if (path = [bundle pathForResource:@"Cursor_W" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_W = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,7)];
			[image release];
		}
	if (!m_Cursor_W) {
		m_Cursor_W = [NSCursor arrowCursor];
		[m_Cursor_W retain];
	}

	if (path = [bundle pathForResource:@"Cursor_Wait" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_Wait = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,7)];
			[image release];
		}
	if (!m_Cursor_Wait) {
		m_Cursor_Wait = [NSCursor arrowCursor];
		[m_Cursor_Wait retain];
	}
	if (path = [bundle pathForResource:@"Cursor_LeftArrow" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_LeftArrow = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,8)];
			[image release];
		}
	if (!m_Cursor_LeftArrow) {
		m_Cursor_LeftArrow = [NSCursor arrowCursor];
		[m_Cursor_LeftArrow retain];
	}
	if (path = [bundle pathForResource:@"Cursor_RightArrow" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_RightArrow = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,7)];
			[image release];
		}
	if (!m_Cursor_RightArrow) {
		m_Cursor_RightArrow = [NSCursor arrowCursor];
		[m_Cursor_RightArrow retain];
	}
	if (path = [bundle pathForResource:@"Cursor_Compass" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_Compass = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,7)];
			[image release];
		}
	if (!m_Cursor_Compass) {
		m_Cursor_Compass = [NSCursor arrowCursor];
		[m_Cursor_Compass retain];
	}
	if (path = [bundle pathForResource:@"Cursor_Exchange" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_Exchange = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,7)];
			[image release];
		}
	if (!m_Cursor_Exchange) {
		m_Cursor_Exchange = [NSCursor arrowCursor];
		[m_Cursor_Exchange retain];
	}
	if (path = [bundle pathForResource:@"leftright_cursor" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_LeftRight = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,8)];
			[image release];
		}
	if (!m_Cursor_LeftRight) {
		m_Cursor_LeftRight = [NSCursor arrowCursor];
		[m_Cursor_LeftRight retain];
	}
	if (path = [bundle pathForResource:@"updown_cursor" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_UpDown = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8,8)];
			[image release];
		}
	if (!m_Cursor_UpDown) {
		m_Cursor_UpDown = [NSCursor arrowCursor];
		[m_Cursor_UpDown retain];
	}
	if (path = [bundle pathForResource:@"Cursor_Crosshair" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_Crosshair = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7,7)];
			[image release];
		}
	if (!m_Cursor_Crosshair) {
		m_Cursor_Crosshair = [NSCursor arrowCursor];
		[m_Cursor_Crosshair retain];
	}
	if (path = [bundle pathForResource:@"Cursor_HandPointer" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_HandPointer = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(6,0)];
			[image release];
		}
	if (!m_Cursor_HandPointer) {
		m_Cursor_HandPointer = [NSCursor arrowCursor];
		[m_Cursor_HandPointer retain];
	}
	if (path = [bundle pathForResource:@"Cursor_DownArrow" ofType:@"png"])
		if (image = [[NSImage alloc] initWithContentsOfFile:path]) {
			m_Cursor_DownArrow = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(6,0)];
			[image release];
		}
	if (!m_Cursor_DownArrow) {
		m_Cursor_DownArrow = [NSCursor arrowCursor];
		[m_Cursor_DownArrow retain];
	}

	m_pApp = app;
	UT_ASSERT (m_pWin);
	m_fontProps = [[NSMutableDictionary alloc] init];
	NSRect viewBounds = [m_pWin bounds];
	if (![m_pWin isKindOfClass:[XAP_CocoaNSView class]]) {
		NSLog(@"attaching a non-XAP_CocoaNSView to a GR_CocoaGraphics");
	}

	[m_pWin setGraphics:this];
	[m_pWin allocateGState];
	s_iInstanceCount++;
	init3dColors ();
	
	/* resolution does not change thru the life of the object */
	m_screenResolution = lrintf(_getScreenResolution());

	StNSViewLocker locker(m_pWin);
	m_currentColor = [[NSColor blackColor] copy];
	m_CGContext = CG_CONTEXT__;
	_resetContext(m_CGContext);

	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
	m_cursor = GR_CURSOR_INVALID;
	setCursor(GR_CURSOR_DEFAULT);
	NSRect aRect = [m_pWin bounds];

	::CGContextSaveGState(m_CGContext);
	[[NSColor whiteColor] set];
	::CGContextFillRect (m_CGContext, ::CGRectMake(aRect.origin.x, aRect.origin.y, 
	                                                aRect.size.width, aRect.size.height));
	::CGContextRestoreGState(m_CGContext);
}

#ifndef RELEASEP
#define RELEASEP(X) do { if (X) { [X release]; X = nil; } } while (0)
#endif

GR_CocoaGraphics::~GR_CocoaGraphics()
{
	DELETEP(m_pFontGUI);

	_destroyFonts ();

	UT_VECTOR_RELEASE(m_cacheArray);
	UT_VECTOR_PURGEALL(NSRect*, m_cacheRectArray);
	[m_pWin setGraphics:NULL];
	[m_fontProps release];
	[m_fontForGraphics release];
	[m_currentColor release];

	RELEASEP(m_imageBlue16x15);
	RELEASEP(m_imageBlue11x16);
	RELEASEP(m_imageGrey16x15);
	RELEASEP(m_imageGrey11x16);

	RELEASEP(m_colorBlue16x15);
	RELEASEP(m_colorBlue11x16);
	RELEASEP(m_colorGrey16x15);
	RELEASEP(m_colorGrey11x16);

	RELEASEP(m_Cursor_E);
	RELEASEP(m_Cursor_N);
	RELEASEP(m_Cursor_NE);
	RELEASEP(m_Cursor_NW);
	RELEASEP(m_Cursor_S);
	RELEASEP(m_Cursor_SE);
	RELEASEP(m_Cursor_SW);
	RELEASEP(m_Cursor_W);

	RELEASEP(m_Cursor_Wait);
	RELEASEP(m_Cursor_LeftArrow);
	RELEASEP(m_Cursor_RightArrow);
	RELEASEP(m_Cursor_Compass);
	RELEASEP(m_Cursor_Exchange);
	RELEASEP(m_Cursor_LeftRight);
	RELEASEP(m_Cursor_UpDown);
	RELEASEP(m_Cursor_Crosshair);
	RELEASEP(m_Cursor_HandPointer);
	RELEASEP(m_Cursor_DownArrow);

	s_iInstanceCount--;
	for (int i = 0; i < COUNT_3D_COLORS; i++) {
		[m_3dColors[i] release];
	}
	
	DELETEP(m_viewLocker);
	[m_fontMetricsTextStorage release];
	[m_fontMetricsLayoutManager release];
	[m_fontMetricsTextContainer release];
}

void GR_CocoaGraphics::fillNSRect (NSRect & aRect, NSColor * color)
{
	if (CONTEXT_LOCKED__) {
		::CGContextSaveGState(m_CGContext);
		[color set];
		::CGContextFillRect (m_CGContext, ::CGRectMake(aRect.origin.x, aRect.origin.y, 
													   aRect.size.width, aRect.size.height));
		::CGContextRestoreGState(m_CGContext);
	}
}

void GR_CocoaGraphics::_beginPaint (void)
{
	UT_ASSERT(m_viewLocker == NULL);
	m_viewLocker = new StNSViewLocker(m_pWin);
	m_CGContext = CG_CONTEXT__;
	_resetContext(m_CGContext);
	_setClipRectImpl(NULL);
}

/*!
	Restart the paiting by unlocking and relocking the whole stuff. One
	of the purpose of this operation is to reset the clipping view
 */
void GR_CocoaGraphics::_restartPaint(void)
{
	UT_ASSERT(m_viewLocker);
	_endPaint();
	_beginPaint();
}

void GR_CocoaGraphics::_endPaint (void)
{
	DELETEP(m_viewLocker);
}

bool GR_CocoaGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
	case DGP_OPAQUEOVERLAY:
		return !isPrinting();
	case DGP_PAPER:
		return isPrinting();
	default:
		UT_ASSERT(0);
		return false;
	}
}


void GR_CocoaGraphics::setZoomPercentage(UT_uint32 iZoom)
{
	DELETEP(m_pFontGUI);
	GR_Graphics::setZoomPercentage (iZoom); // chain up
}

void GR_CocoaGraphics::setLineProperties ( double    inWidth, 
				      JoinStyle inJoinStyle,
				      CapStyle  inCapStyle,
				      LineStyle inLineStyle )
{
	m_fLineWidth = tduD(inWidth);
	m_joinStyle = inJoinStyle;
	m_capStyle = inCapStyle;
	m_lineStyle = inLineStyle;
	if (m_viewLocker) {
		::CGContextSetLineWidth (m_CGContext, m_fLineWidth);
		_setCapStyle(m_capStyle);
		_setJoinStyle(m_joinStyle);
		_setLineStyle(m_lineStyle);	
	}
}

void GR_CocoaGraphics::_setCapStyle(CapStyle inCapStyle, CGContextRef * context)
{
	switch (inCapStyle) {
	case CAP_BUTT:
		::CGContextSetLineCap((context ? *context : m_CGContext), kCGLineCapButt);
		break;
	case CAP_ROUND:
		::CGContextSetLineCap((context ? *context : m_CGContext), kCGLineCapRound);
		break;
	case CAP_PROJECTING:
		::CGContextSetLineCap((context ? *context : m_CGContext), kCGLineCapSquare);
		break;
	default:
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	}
}

void GR_CocoaGraphics::_setJoinStyle(JoinStyle inJoinStyle, CGContextRef * context)
{
	switch (inJoinStyle) {
	case JOIN_MITER:
		::CGContextSetLineJoin((context ? *context : m_CGContext), kCGLineJoinMiter);
		break;
	case JOIN_ROUND:
		::CGContextSetLineJoin((context ? *context : m_CGContext), kCGLineJoinRound);
		break;
	case JOIN_BEVEL:
		::CGContextSetLineJoin((context ? *context : m_CGContext), kCGLineJoinBevel);
		break;
	default:
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	}
}

void GR_CocoaGraphics::_setLineStyle (LineStyle inLineStyle, CGContextRef * context)
{
	int lws = context ? 1 : static_cast<int>(m_fLineWidth);

	switch (inLineStyle) {
	case LINE_SOLID:
		{
			::CGContextSetLineDash((context ? *context : m_CGContext), 0, NULL, 0);
		}
		break;
	case LINE_ON_OFF_DASH:
		{
			float dash_list[2] = { 4*lws, 5*lws };
			::CGContextSetLineDash((context ? *context : m_CGContext), 0, dash_list, 2);
		}
		break;
	case LINE_DOUBLE_DASH:
		{
			float dash_list[4] = { 1*lws, 3*lws, 4*lws, 2*lws };
			::CGContextSetLineDash((context ? *context : m_CGContext), 0, dash_list, 4);
		}
		break;
	case LINE_DOTTED:
		{
			float dash_list[2] = { 1*lws, 4*lws };
			::CGContextSetLineDash((context ? *context : m_CGContext), 0, dash_list, 2);
		}
		break;
	default:
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);	
	}
}

void GR_CocoaGraphics::_realDrawChars(const unichar* cBuf, int len, NSDictionary *fontProps, 
									  float x, float y, int begin, int rangelen)
{
	NSString *string;
	string = [[NSString alloc] initWithCharacters:cBuf length:len];
	if (string) {
		NSLog (@"drawChar(%@) x = %f, y = %f (%d,%d)", string, x, y, begin, rangelen);
		NSAttributedString * attributedString = [[NSAttributedString alloc] initWithString:string 
												 attributes:fontProps];
		if (attributedString) {
			[m_fontMetricsTextStorage setAttributedString:attributedString];
			[attributedString release];
		}
		[string release];
		
		NSPoint point = NSMakePoint(x, y);
		
		[m_fontMetricsLayoutManager drawGlyphsForGlyphRange:NSMakeRange(begin, rangelen) atPoint:point];
	}
}


void GR_CocoaGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
								 int iLength, UT_sint32 xoffLU, UT_sint32 yoffLU,
								 int * pCharWidths)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::drawChars()\n"));
	UT_sint32 yoff = _tduY(yoffLU); // - m_pFont->getDescent();
	UT_sint32 xoff = xoffLU;	// layout Unit !

    if (!m_fontMetricsTextStorage) {
		_initMetricsLayouts();
    }

	::CGContextSetShouldAntialias (m_CGContext, true);

	if (!pCharWidths) {
		/*
		  We don't have char widths because we don't care. Just draw the text.
		  CAUTION BiDi is handled by Cocoa in that case.
		 */
		unichar * uniString = (unichar*)malloc((iLength + 1) * sizeof (unichar));
		for (int i = 0; i < iLength; i++) {
			uniString[i] = m_pFont ? m_pFont->remapChar(pChars[i + iCharOffset]) : pChars[i + iCharOffset];
		}
		LOCK_CONTEXT__;
		_realDrawChars(uniString, iLength, m_fontProps, TDUX(xoff), yoff, 0, iLength);
		FREEP(uniString);
	}
	else {
		LOCK_CONTEXT__;

		/*
		  Here we have charWidth. that means WE HAVE TO USE THEM
		  Changes in revision 1.78 BROKE THAT.

		  That said, we should be done is that we must split the text run into words 
		  and draw run by run, cumulating the xoff with the char width.

		  Given the implication, a word is defined by either a blank (see UT_UCS4_isspace())
		  or a change of direction. -- Hub
		 */

		const UT_UCSChar * begin = pChars + iCharOffset;
		const UT_UCSChar * end = begin + iLength;

		int * endWidth = pCharWidths + iCharOffset + iLength;

		UT_sint32 widthWhiteSpace = 0;

		while (end > begin) {
			if (!UT_UCS4_isspace(*(end - 1))) {
				break;
			}
			--end;
			--endWidth;
			widthWhiteSpace += *endWidth;
		}
		iLength = end - begin;

		UT_sint32 widthTrailingNeutral = 0;
		UT_sint32 countTrailingNeutral = 0;

		const UT_UCSChar * endTN = end;

		NSCharacterSet * punctuation = [NSCharacterSet punctuationCharacterSet];

		while (end > begin) {
			unichar uc = (unichar) (*(end - 1));
			if ([punctuation characterIsMember:uc] == NO) {
				break;
			}
			--end;
			--endWidth;
			widthTrailingNeutral += *endWidth;
			countTrailingNeutral++;
		}

		if (iLength) {
			unichar * cBuf = (unichar *) malloc((iLength + 1) * sizeof(unichar));
			if (cBuf) {
				bool knownDir = false;
				bool rtl = false;
				int i;
				for (i = 0; i < iLength; i++) {
					if (!knownDir) {
						UT_BidiCharType charType = UT_bidiGetCharType(begin[i]);
						if (UT_BIDI_IS_STRONG(charType)) {
							rtl = UT_BIDI_IS_RTL(charType);
							knownDir = true;
							NSLog(@"direction is %d (1 == RTL %x)) set at idx %d with chartype = %x, char %x", rtl, 
							                           UT_BIDI_RTL, i, charType, begin[i]); 
						}
					}
					cBuf[i] = (unichar) (m_pFont ? m_pFont->remapChar(begin[i]) : begin[i]);
				}
				cBuf[iLength] = 0;

				if(!knownDir) {
					NSLog(@"direction is UNKNOWN");
				}

				float x = xoff;
				int len = iLength;
				int rangeLength = 0;
				int rangeBegin = 0;
				int j;
				float currentRunLen = 0;
				NSLog (@"start at x = %d", TDUX(x));
				for (j = 0; j < len; j++) {
					if (UT_UCS4_isspace(cBuf[j])) {
						if (rangeLength > 0) {
							NSLog (@"x = %d, currentRunLen = %d", TDUX(x), TDUX(currentRunLen));
							_realDrawChars(cBuf + rangeBegin, iLength - rangeBegin, m_fontProps, TDUX(x),
										   yoff, 0, rangeLength);
							// from here currentRunLen is signed... so just add it
							if (!rtl) {
								x += currentRunLen;
							}
							NSLog(@"x is now %d", TDUX(x));
						}
						if (j < len - 1) {
							if (rtl) {
								x -= pCharWidths[iCharOffset+j];
							}
							else{
								x += pCharWidths[iCharOffset+j];
							}
							NSLog (@"moved to (space) x = %d charwidth was %d", TDUX(x), TDUX(pCharWidths[iCharOffset+j]));
						}
						rangeBegin = j + 1;
						rangeLength = 0;
						currentRunLen = 0;
					}
					else {
						rangeLength++;
						if (rtl) {
							currentRunLen -= pCharWidths[iCharOffset+j];
						}
						else {
							currentRunLen += pCharWidths[iCharOffset+j];
						}
					}
				}
				if (rangeLength > 0) {
					_realDrawChars(cBuf + rangeBegin, iLength - rangeBegin, m_fontProps, TDUX(x),
								   yoff, 0, rangeLength);
				}
			}
		}
	}
	::CGContextSetShouldAntialias (m_CGContext, false);
}

void GR_CocoaGraphics::setFont(GR_Font * pFont)
{
	XAP_CocoaFont * pUFont = static_cast<XAP_CocoaFont *> (pFont);
	UT_ASSERT (pUFont);
	m_pFont = pUFont;
	[m_fontForGraphics release];
	NSFont* font = pUFont->getNSFont();
	m_fontForGraphics = [[NSFontManager sharedFontManager] convertFont:font
							toSize:[font pointSize] * (getZoomPercentage() / 100.)];
	[m_fontProps setObject:m_fontForGraphics forKey:NSFontAttributeName];
}

UT_uint32 GR_CocoaGraphics::getFontHeight(GR_Font * fnt)
{
	UT_ASSERT(fnt);
	return static_cast<UT_uint32>(lrint(ftluD(static_cast<XAP_CocoaFont*>(fnt)->getHeight())));
}

UT_uint32 GR_CocoaGraphics::getFontHeight()
{
	return static_cast<UT_uint32>(lrint(ftluD(m_pFont->getHeight())));
}


void GR_CocoaGraphics::_initMetricsLayouts(void)
{
	m_fontMetricsTextStorage = [[NSTextStorage alloc] init];

	m_fontMetricsLayoutManager = [[NSLayoutManager alloc] init];
	[m_fontMetricsTextStorage addLayoutManager:m_fontMetricsLayoutManager];
	[m_fontMetricsLayoutManager setTypesetterBehavior:NSTypesetterBehavior_10_2];

	m_fontMetricsTextContainer = [[NSTextContainer alloc] initWithContainerSize:NSMakeSize(10000, 1000)];
	[m_fontMetricsTextContainer setLineFragmentPadding:0];
	[m_fontMetricsLayoutManager addTextContainer:m_fontMetricsTextContainer];
}


/*!
	Internal version to measure unremapped char
	
	\return width in Device Unit
 */
float GR_CocoaGraphics::_measureUnRemappedCharCached(const UT_UCSChar c)
{
	float width;
	width = m_pFont->getCharWidthFromCache(c);
	width *= ((float)m_pFont->getSize() / (float)GR_CharWidthsCache::CACHE_FONT_SIZE);
	return width;
}

/*!
	Measure unremapped char
	
	\return width in Layout Unit
 */
UT_sint32 GR_CocoaGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	// measureString() could be defined in terms of measureUnRemappedChar()
	// but its not (for presumed performance reasons).  Also, a difference
	// is that measureString() uses remapping to get past zero-width
	// character cells.

	if(c == 0x200B || c == 0xFEFF) { // 0-with spaces
		return 0;
	}

	return static_cast<UT_uint32>(lrint(ftluD(_measureUnRemappedCharCached(c))));
}


void GR_CocoaGraphics::getCoverage(UT_NumberVector& coverage)
{
	m_pFont->getCoverage(coverage);	
}


UT_uint32 GR_CocoaGraphics::_getResolution(void) const
{
	return m_screenResolution;
}

/*!
	Convert a UT_RGBColor to a NSColor
	\param clr the UT_RGBColor to convert
	\return an autoreleased NSColor

	Handle the transparency as well.
 */
NSColor	*GR_CocoaGraphics::_utRGBColorToNSColor (const UT_RGBColor& clr)
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
void GR_CocoaGraphics::_utNSColorToRGBColor (NSColor *c, UT_RGBColor &clr)
{
	float r, g, b, a;
	[c getRed:&r green:&g blue:&b alpha:&a];
	clr.m_red = static_cast<unsigned char>(r * 255.0f);
	clr.m_grn = static_cast<unsigned char>(g * 255.0f);
	clr.m_blu = static_cast<unsigned char>(b * 255.0f);
	clr.m_bIsTransparent = false /*(a == 0.0f)*/;
}


void GR_CocoaGraphics::setColor(const UT_RGBColor& clr)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::setColor(const UT_RGBColor&): setting color %d, %d, %d\n", clr.m_red, clr.m_grn, clr.m_blu));
	NSColor *c = _utRGBColorToNSColor (clr);
	_setColor(c);
}

void	GR_CocoaGraphics::getColor(UT_RGBColor& clr)
{
	_utNSColorToRGBColor (m_currentColor, clr);
}


/* c will be copied */
void GR_CocoaGraphics::_setColor(NSColor * c)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::_setColor(NSColor *): setting NSColor\n"));
	[m_currentColor release];
	m_currentColor = [c copy];
	[m_fontProps setObject:m_currentColor forKey:NSForegroundColorAttributeName];
	if (m_viewLocker) {
		LOCK_CONTEXT__;
		[m_currentColor set];
	}
}

GR_Font * GR_CocoaGraphics::getGUIFont(void)
{
	if (!m_pFontGUI)
	{
		// get the font resource		
		UT_DEBUGMSG(("GR_CocoaGraphics::getGUIFont: getting default font\n"));
		// bury it in a new font handle
		m_pFontGUI = new XAP_CocoaFont([NSFont labelFontOfSize:
			([NSFont labelFontSize] * 100.0 / getZoomPercentage())]); // Hardcoded GUI font size
		UT_ASSERT(m_pFontGUI);
	}

	return m_pFontGUI;
}

GR_Font * GR_CocoaGraphics::_findFont(const char* pszFontFamily,
										const char* pszFontStyle,
										const char* pszFontVariant,
										const char* pszFontWeight,
										const char* pszFontStretch,
										const char* pszFontSize)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::findFont(%s, %s, %s)\n", pszFontFamily, pszFontStyle, pszFontSize));

	UT_ASSERT(pszFontFamily);
	UT_ASSERT(pszFontStyle);
	UT_ASSERT(pszFontWeight);
	UT_ASSERT(pszFontSize);

	double size = ceil(UT_convertToPoints(pszFontSize));

	size = (size < 1.0) ? 1.0 : size;

	NSString * font_name = [NSString stringWithUTF8String:pszFontFamily];

	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];

	NSString * family_name = [pController familyNameForFont:font_name];

	NSFont * nsfont = nil;

	if (family_name)
		{
			// fprintf(stderr, "*font* name: \"%s\", size=%lgpt\n", [font_name UTF8String], size);
			/* this looks like a font name, not a font-family name...
			 */
			nsfont = [NSFont fontWithName:font_name size:((float) size)];
		}
	if (!nsfont)
		{
			/* this probably is a real font-family name, not a font name...
			 */
			family_name = font_name;
			// fprintf(stderr, "family name: \"%s\", size=%lgpt\n", [family_name UTF8String], size);
			NSFontTraitMask s = 0;

			// this is kind of sloppy
			if (UT_strcmp(pszFontStyle, "italic") == 0)
				{
					s |= NSItalicFontMask;
				}
			if (UT_strcmp(pszFontWeight, "bold") == 0)
				{
					s |= NSBoldFontMask;
				}

			nsfont = [[NSFontManager sharedFontManager] fontWithFamily:family_name traits:s weight:5 size:size];

			if (!nsfont) // this is bad; the wrong font name ends up in the font popups - FIXME please!
				{
					/* add a few hooks for a few predefined font names that MAY differ.
					 * for example "Dingbats" is called "Zapf Dingbats" on MacOS X. 
					 * Only fallback AFTER. WARNING: this is recursive call, watch out the
					 * font family you pass.
					 */
					if (UT_stricmp(pszFontFamily, "Dingbats") == 0)
						{
							GR_Font * pGRFont = findFont("Zapf Dingbats", pszFontStyle, pszFontVariant, pszFontWeight, pszFontStretch, pszFontSize);
							XAP_CocoaFont * pFont = static_cast<XAP_CocoaFont *>(pGRFont);
							nsfont = pFont->getNSFont();
						}
					if (UT_stricmp(pszFontFamily, "Helvetic") == 0)
						{
							GR_Font * pGRFont = findFont("Helvetica", pszFontStyle, pszFontVariant, pszFontWeight, pszFontStretch, pszFontSize);
							XAP_CocoaFont * pFont = static_cast<XAP_CocoaFont *>(pGRFont);
							nsfont = pFont->getNSFont();
						}
				}
			if (!nsfont) // this is bad; the wrong font name ends up in the font popups - FIXME please!
				{
					/* Oops!  We don't have that font here.
					 * first try "Times New Roman", which should be sensible, and should
					 * be there unless the user fidled with the installation
					 */
					NSLog (@"Unable to find font \"%s\".", pszFontFamily);

					nsfont = [[NSFontManager sharedFontManager] fontWithFamily:@"Times" traits:s weight:5 size:size];
				}
		}

	/* bury the pointer to our Cocoa font in a XAP_CocoaFontHandle 
	 */
	XAP_CocoaFont * pFont = new XAP_CocoaFont(nsfont);
	UT_ASSERT(pFont);

	return pFont;
}

// returns in LU
UT_uint32 GR_CocoaGraphics::getFontAscent(GR_Font * fnt)
{
	UT_ASSERT(fnt);
	return static_cast<UT_uint32>(lrint(ftluD(static_cast<XAP_CocoaFont*>(fnt)->getAscent())));
}

// returns in LU
UT_uint32 GR_CocoaGraphics::getFontAscent()
{
	return static_cast<UT_uint32>(lrint(ftluD(m_pFont->getAscent())));
}

// returns in LU
UT_uint32 GR_CocoaGraphics::getFontDescent(GR_Font * fnt)
{
	UT_ASSERT(fnt);
	return static_cast<UT_uint32>(lrint(ftluD(static_cast<XAP_CocoaFont*>(fnt)->getDescent())));
}

UT_uint32 GR_CocoaGraphics::getFontDescent()
{
	return static_cast<UT_uint32>(lrint(ftluD(m_pFont->getDescent())));
}

void GR_CocoaGraphics::rawPolyAtOffset(NSPoint * point, int npoint, UT_sint32 offset_x, UT_sint32 offset_y, NSColor * color, bool bFill)
{
	UT_ASSERT(point && (npoint > 2) && color);
	if (!(point && (npoint > 2) && color))
		return;

	LOCK_CONTEXT__;

	[color set];

	::CGContextBeginPath(m_CGContext);
	::CGContextMoveToPoint(m_CGContext, TDUX(offset_x) + point[0].x, _tduY(offset_y) + point[0].y);

	for (int i = 1; i < npoint; i++)
		::CGContextAddLineToPoint(m_CGContext, TDUX(offset_x) + point[i].x, _tduY(offset_y) + point[i].y);

	::CGContextClosePath(m_CGContext);

	if (bFill)
		::CGContextFillPath(m_CGContext);
	else
		::CGContextStrokePath(m_CGContext);
}

void GR_CocoaGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
							   UT_sint32 x2, UT_sint32 y2)
{
	LOCK_CONTEXT__;
	UT_DEBUGMSG (("GR_CocoaGraphics::drawLine(%ld, %ld, %ld, %ld) width=%f\n", x1, y1, x2, y2, m_fLineWidth));
	// if ((y1 == y2) && (x1 >= 500)) fprintf(stderr, "GR_CocoaGraphics::drawLine(%ld, %ld, %ld, %ld) width=%f\n", x1, y1, x2, y2, m_fLineWidth);
	::CGContextBeginPath(m_CGContext);
	::CGContextMoveToPoint (m_CGContext, TDUX(x1), _tduY(y1));
	::CGContextAddLineToPoint (m_CGContext, TDUX(x2), _tduY(y2));
	[m_currentColor set];
	::CGContextStrokePath (m_CGContext);
}

void GR_CocoaGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::setLineWidth(%ld) was %f\n", iLineWidth, m_fLineWidth));
	m_fLineWidth = static_cast<float>(ceil(tduD(iLineWidth) - 0.75));
	m_fLineWidth = (m_fLineWidth > 0) ? m_fLineWidth : 1.0f;
	if (m_viewLocker) {
		::CGContextSetLineWidth (m_CGContext, m_fLineWidth);
		_setLineStyle(m_lineStyle);	
	}
}

void GR_CocoaGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::polyLine() width=%f\n", m_fLineWidth));
	
	LOCK_CONTEXT__;
	::CGContextBeginPath(m_CGContext);
	
	for (UT_uint32 i = 0; i < nPoints; i++)
	{
		if (i == 0) {
			::CGContextMoveToPoint(m_CGContext, TDUX(pts[i].x), _tduY(pts[i].y));
		}
		else {
			::CGContextAddLineToPoint(m_CGContext, TDUX(pts[i].x), _tduY(pts[i].y));
		}
	}
	::CGContextStrokePath(m_CGContext);
}

void GR_CocoaGraphics::invertRect(const UT_Rect* pRect)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::invertRect()\n"));
	UT_ASSERT(pRect);

	LOCK_CONTEXT__;
	// TODO handle invert. this is highlight.

	NSHighlightRect (NSMakeRect (TDUX(pRect->left), _tduY(pRect->top), _tduR(pRect->width), _tduR(pRect->height)));
}

void GR_CocoaGraphics::setClipRect(const UT_Rect* pRect)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::setClipRect()\n"));
	m_pRect = pRect;
	if (m_viewLocker) {
		/* if we are painting, restart the painting to reset the clipping view */
		_restartPaint();
	}
}
void GR_CocoaGraphics::_setClipRectImpl(const UT_Rect*)
{
	if (m_pRect) {
		UT_DEBUGMSG (("ClipRect set\n"));
		::CGContextClipToRect (m_CGContext, 
				::CGRectMake (TDUX(m_pRect->left), _tduY(m_pRect->top), _tduR(m_pRect->width), _tduR(m_pRect->height)));
	}
	else {
		UT_DEBUGMSG (("ClipRect reset!!\n"));
		NSRect bounds = [m_pWin bounds];
		::CGContextClipToRect (m_CGContext,
				::CGRectMake (bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height)); // ??
	}
}

void GR_CocoaGraphics::fillRect(const UT_RGBColor& clr, UT_sint32 x, UT_sint32 y,
							   UT_sint32 w, UT_sint32 h)
{
	UT_DEBUGMSG(("GR_CocoaGraphics::fillRect(UT_RGBColor&, %ld, %ld, %ld, %ld)\n", x, y, w, h));
	// save away the current color, and restore it after we fill the rect
	NSColor *c = _utRGBColorToNSColor (clr);

	LOCK_CONTEXT__;
	::CGContextSaveGState(m_CGContext);
	[c set];	
	::CGContextFillRect(m_CGContext, ::CGRectMake (tdu(x), tdu(y), tdu(w), tdu(h)));
	::CGContextRestoreGState(m_CGContext);
}

void GR_CocoaGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	UT_DEBUGMSG(("GR_CocoaGraphics::fillRect(GR_Color3D %d, %ld, %ld, %ld, %ld)\n", c, x, y, w, h));

	LOCK_CONTEXT__;
	::CGContextSaveGState(m_CGContext);
	[m_3dColors[c] set];
	::CGContextFillRect (m_CGContext, ::CGRectMake (tdu(x), tdu(y), tdu(w), tdu(h)));
	::CGContextRestoreGState(m_CGContext);
}

void GR_CocoaGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_DEBUGMSG(("GR_CocoaGraphics::fillRect(GR_Color3D, UT_Rect &)\n"));
	UT_ASSERT(c < COUNT_3D_COLORS);
	fillRect(c, r.left, r.top, r.width, r.height);
}


void GR_CocoaGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
	if (!dx && !dy) return;

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

void GR_CocoaGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
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

void GR_CocoaGraphics::clearArea(UT_sint32 x, UT_sint32 y,
							 UT_sint32 width, UT_sint32 height)
{
  	UT_DEBUGMSG(("ClearArea: %d %d %d %d\n", x, y, width, height));
	if (width > 0)
	{
		LOCK_CONTEXT__;
		::CGContextSaveGState(m_CGContext);
		[[NSColor whiteColor] set];
		::CGContextFillRect (m_CGContext, ::CGRectMake(TDUX(x), _tduY(y), 
														_tduR(width), _tduR(height)));
		::CGContextRestoreGState(m_CGContext);
	}
}

bool GR_CocoaGraphics::startPrint(void)
{
	UT_ASSERT (m_bIsPrinting);
	return true;
}

bool GR_CocoaGraphics::startPage(const char * /*szPageLabel*/, UT_uint32 /*pageNumber*/,
								bool /*bPortrait*/, UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
{
	UT_ASSERT (m_bIsPrinting);
	return true;
}

bool GR_CocoaGraphics::endPrint(void)
{
	UT_ASSERT (m_bIsPrinting);
	return true;
}


GR_Image* GR_CocoaGraphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
{
   	GR_Image* pImg = NULL;
   	if (iType == GR_Image::GRT_Raster)
   		pImg = new GR_CocoaImage(pszName);
   	else
	   	pImg = new GR_VectorImage(pszName);

	pImg->convertFromBuffer(pBB, _tduR(iDisplayWidth), _tduR(iDisplayHeight));
   	return pImg;
}

void GR_CocoaGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);

   	if (pImg->getType() != GR_Image::GRT_Raster) {
	   	pImg->render(this, (UT_sint32)rint(TDUX(xDest)), (UT_sint32)rint(_tduY(yDest)));
	   	return;
	}

   	GR_CocoaImage * pCocoaImage = static_cast<GR_CocoaImage *>(pImg);

   	NSImage * image = pCocoaImage->getNSImage();

	if (image == 0)
	{
		UT_DEBUGMSG(("Found no image data. This is probably SVG masquerading as a raster!\n"));
		return;
	}

//   	UT_sint32 iImageWidth = pCocoaImage->getDisplayWidth();
   	UT_sint32 iImageHeight = pCocoaImage->getDisplayHeight();
	NSSize size = [image size];

	LOCK_CONTEXT__;
	::CGContextSaveGState(m_CGContext);
//	::CGContextTranslateCTM (m_CGContext, -0.5, -0.5);
	[image drawInRect:NSMakeRect(TDUX(xDest), _tduY(yDest), pCocoaImage->getDisplayWidth(), iImageHeight)
	           fromRect:NSMakeRect(0, 0, size.width, size.height) operation:NSCompositeCopy fraction:1.0f];
//	[image compositeToPoint:NSMakePoint(xDest, yDest + iImageHeight) operation:NSCompositeCopy fraction:1.0f];
	::CGContextRestoreGState(m_CGContext);
}


void GR_CocoaGraphics::flush(void)
{
	if (!m_bIsDrawing)
		[m_pWin displayIfNeeded];
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
	GR_Graphics::Cursor old_cursor = m_cursor;

	m_cursor = (c == GR_CURSOR_GRAB) ? m_GrabCursor : c;

	if (m_cursor == old_cursor)
		return;

	bool bImplemented = true;

	switch (m_cursor)
	{
	case GR_CURSOR_DEFAULT:
		// NSLog(@"Cursor default");
		[m_pWin setCursor:[NSCursor arrowCursor]];
		break;

	case GR_CURSOR_IBEAM:
		// NSLog(@"Cursor IBeam");
		[m_pWin setCursor:[NSCursor IBeamCursor]];
		break;

	case GR_CURSOR_VLINE_DRAG:
		// NSLog(@"Cursor VLine Drag (unimplemented)");
		m_cursor = GR_CURSOR_LEFTRIGHT;
		// fall through...

	case GR_CURSOR_LEFTRIGHT:
		// NSLog(@"Cursor LeftRight");
		if (m_cursor != old_cursor)
		{
			[m_pWin setCursor:m_Cursor_LeftRight];
		}
		break;

	case GR_CURSOR_HLINE_DRAG:
		// NSLog(@"Cursor HLine Drag (unimplemented)");
		m_cursor = GR_CURSOR_UPDOWN;
		// fall through...

	case GR_CURSOR_UPDOWN:
		// NSLog(@"Cursor UpDown");
		if (m_cursor != old_cursor)
		{
			[m_pWin setCursor:m_Cursor_UpDown];
		}
		break;

	case GR_CURSOR_IMAGE:
		// NSLog(@"Cursor Image");
		[m_pWin setCursor:m_Cursor_Compass];
		break;

	case GR_CURSOR_IMAGESIZE_E:
		// NSLog(@"Cursor ImageSize [ E]");
		[m_pWin setCursor:m_Cursor_E];
		break;

	case GR_CURSOR_IMAGESIZE_N:
		// NSLog(@"Cursor ImageSize [N ]");
		[m_pWin setCursor:m_Cursor_N];
		break;

	case GR_CURSOR_IMAGESIZE_NE:
		// NSLog(@"Cursor ImageSize [NE]");
		[m_pWin setCursor:m_Cursor_NE];
		break;

	case GR_CURSOR_IMAGESIZE_NW:
		// NSLog(@"Cursor ImageSize [NW]");
		[m_pWin setCursor:m_Cursor_NW];
		break;

	case GR_CURSOR_IMAGESIZE_S:
		// NSLog(@"Cursor ImageSize [S ]");
		[m_pWin setCursor:m_Cursor_S];
		break;

	case GR_CURSOR_IMAGESIZE_SE:
		// NSLog(@"Cursor ImageSize [SE]");
		[m_pWin setCursor:m_Cursor_SE];
		break;

	case GR_CURSOR_IMAGESIZE_SW:
		// NSLog(@"Cursor ImageSize [SW]");
		[m_pWin setCursor:m_Cursor_SW];
		break;

	case GR_CURSOR_IMAGESIZE_W:
		// NSLog(@"Cursor ImageSize [ W]");
		[m_pWin setCursor:m_Cursor_W];
		break;

	case GR_CURSOR_WAIT:
		// NSLog(@"Cursor Wait");
		[m_pWin setCursor:m_Cursor_Wait];
		break;

	case GR_CURSOR_RIGHTARROW:
		// NSLog(@"Cursor RightArrow");
		[m_pWin setCursor:m_Cursor_RightArrow];
		break;

	case GR_CURSOR_LEFTARROW:
		// NSLog(@"Cursor LeftArrow");
		[m_pWin setCursor:m_Cursor_LeftArrow];
		break;

	case GR_CURSOR_EXCHANGE:
		// NSLog(@"Cursor Exchange");
		[m_pWin setCursor:m_Cursor_Exchange];
		break;

	case GR_CURSOR_CROSSHAIR:
		// NSLog(@"Cursor Crosshair");
		[m_pWin setCursor:m_Cursor_Crosshair];
		break;

	case GR_CURSOR_LINK:
		// NSLog(@"Cursor Link");
		[m_pWin setCursor:m_Cursor_HandPointer];
		break;

	case GR_CURSOR_DOWNARROW:
		// NSLog(@"Cursor DownArrow");
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

GR_Graphics::Cursor GR_CocoaGraphics::getCursor(void) const
{
	return m_cursor;
}

void GR_CocoaGraphics::setColor3D(GR_Color3D c)
{
	UT_DEBUGMSG(("GR_CocoaGraphics::setColor3D(GR_Color3D %d)\n", c));
	UT_ASSERT(c < COUNT_3D_COLORS);
	_setColor(m_3dColors[c]);
}

void GR_CocoaGraphics::init3dColors()
{
	m_3dColors[CLR3D_Foreground] = [[NSColor blackColor] copy];
	m_3dColors[CLR3D_Background] = [[NSColor controlColor] copy];
	m_3dColors[CLR3D_BevelUp]    = [[NSColor whiteColor] copy];
	m_3dColors[CLR3D_BevelDown]  =  [NSColor colorWithCalibratedWhite:0.0f alpha:0.25]; // [[NSColor darkGrayColor] copy];
   [m_3dColors[CLR3D_BevelDown] retain];
	m_3dColors[CLR3D_Highlight]  = [[NSColor whiteColor] copy]; // [[NSColor controlHighlightColor] copy];
}

void GR_CocoaGraphics::polygon(UT_RGBColor& clr,UT_Point *pts,UT_uint32 nPoints)
{
	NSColor *c = _utRGBColorToNSColor (clr);
	LOCK_CONTEXT__;
	::CGContextBeginPath(m_CGContext);
	for (UT_uint32 i = 0; i < nPoints; i++)
	{
		if (i == 0) {
			::CGContextMoveToPoint(m_CGContext, TDUX(pts[i].x), _tduY(pts[i].y));
		}
		else {
			::CGContextAddLineToPoint(m_CGContext, TDUX(pts[i].x), _tduY(pts[i].y));
		}
	}
	::CGContextSaveGState(m_CGContext);
	[c set];
	::CGContextFillPath(m_CGContext);
	::CGContextRestoreGState(m_CGContext);
}


void GR_CocoaGraphics::_setUpdateCallback (gr_cocoa_graphics_update callback, void * param)
{
	m_updateCallback = callback;
	m_updateCBparam = param;
}

/*!
	Call the update Callback that has been set

	\param aRect: the rect that should be updated
	\return false if no callback. Otherwise returns what the callback returns.
 */
bool GR_CocoaGraphics::_callUpdateCallback(NSRect * aRect)
{
	if (m_updateCallback == NULL) {
		return false;
	}
	m_bIsDrawing = true;
	bool ret = (*m_updateCallback) (aRect, this, m_updateCBparam);
	m_bIsDrawing = false;
	return ret;
}

bool GR_CocoaGraphics::_isFlipped()
{
	return YES;
}

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
	
	*pff = FF_Unknown;

	NSFont * nsfont = [[NSFontManager sharedFontManager] fontWithFamily:[NSString stringWithUTF8String:szFontName] 
		traits:0 weight:5 size:(float)12.0];

	if ([[NSFontManager sharedFontManager] traitsOfFont:nsfont] == NSFixedPitchFontMask){
		*pfp = FP_Fixed;
	}
	else {
		*pfp = FP_Variable;
	}
	*pbTrueType = true;	// consider that they are all TT fonts. we don't care
}


GR_Image * GR_CocoaGraphics::genImageFromRectangle(const UT_Rect & r)
{
	GR_CocoaImage *img = new GR_CocoaImage("ScreenShot");
	NSRect rect = NSMakeRect(TDUX(r.left), _tduY(r.top), 
						  _tduR(r.width), _tduR(r.height));
    NSBitmapImageRep *imageRep;
	{
		LOCK_CONTEXT__;
		imageRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:rect];
	}
	img->setFromImageRep(imageRep);
	[imageRep release];
	return static_cast<GR_Image*>(img);
}


void GR_CocoaGraphics::saveRectangle(UT_Rect & rect,  UT_uint32 iIndx)
{
	NSRect* cacheRect = new NSRect;
	cacheRect->origin.x = static_cast<float>(_tduX(rect.left)) - 1.0f;
	cacheRect->origin.y = static_cast<float>(_tduY(rect.top )) - 1.0f;
	cacheRect->size.width  = static_cast<float>(_tduR(rect.width )) + 2.0f;
	cacheRect->size.height = static_cast<float>(_tduR(rect.height)) + 2.0f;

	NSRect bounds = [m_pWin bounds];
	if (cacheRect->size.height > bounds.size.height - cacheRect->origin.y)
		cacheRect->size.height = bounds.size.height - cacheRect->origin.y;

    NSBitmapImageRep *imageRep;
	{
		LOCK_CONTEXT__;
		imageRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:*cacheRect];
	}
	NSImage* cache = [[NSImage alloc] initWithSize:[imageRep size]];
	[cache addRepresentation:imageRep];
	[imageRep release];
	// update cache arrays
	id oldObj;
	m_cacheArray.setNthItem(iIndx, cache, &oldObj);
	[oldObj release];
	NSRect *old;
	m_cacheRectArray.setNthItem(iIndx, cacheRect, &old);
	if (old) {
		delete old;
	}
}


void GR_CocoaGraphics::restoreRectangle(UT_uint32 iIndx)
{
	NSRect* cacheRect = m_cacheRectArray.getNthItem(iIndx);
	NSImage* cache = m_cacheArray.getNthItem(iIndx);
	NSPoint pt = cacheRect->origin;
	pt.y += cacheRect->size.height;
	{
		LOCK_CONTEXT__;
		[cache compositeToPoint:pt operation:NSCompositeCopy];
	}
}

UT_uint32 GR_CocoaGraphics::getDeviceResolution(void) const
{
	return _getResolution();
}

void GR_CocoaGraphics::_resetContext(CGContextRef context)
{
	// TODO check that we properly reset parameters according to what has been saved.
	::CGContextSetLineWidth(context, m_fLineWidth);
	_setCapStyle(m_capStyle, &context);
	_setJoinStyle(m_joinStyle, &context);
	_setLineStyle(m_lineStyle, &context);
	::CGContextSetShouldAntialias(context, false);
 	[m_currentColor set];
}

float	GR_CocoaGraphics::_getScreenResolution(void)
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

GR_Graphics *  GR_CocoaGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	GR_CocoaAllocInfo & allocator = (GR_CocoaAllocInfo&)info;
	
	return new GR_CocoaGraphics(allocator.m_view, allocator.m_app);
}
