/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:nil; -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2021 Hubert Figuière
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
#include <stdlib.h>
#include <string.h>

#include <memory>

#include "gr_CharWidths.h"
#include "gr_CocoaFont.h"
#include "gr_CocoaGraphics.h"
#include "gr_CocoaImage.h"
#include "gr_CoreGraphicsUtils.h"
#include "ut_endian.h"
#include "ut_sleep.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaAppController.h"
#include "xap_CocoaFrame.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_misc.h"
#include "ut_std_vector.h"

#include "ut_OverstrikingChars.h"
#include "xap_EncodingManager.h"

#include <ApplicationServices/ApplicationServices.h>
#import <Cocoa/Cocoa.h>
#include <CoreServices/CoreServices.h>

#define DISABLE_VERBOSE 1

#if defined(DISABLE_VERBOSE) && DISABLE_VERBOSE
#undef UT_DEBUGMSG
#define UT_DEBUGMSG(x)
#endif

#define TDUX(x) (_tduXD(x))
// #define TDUX(x) (_tduX(x)+1.0)

void GR_CGContextIsNull(NSString* format, ...)
{
    va_list vl;
    va_start(vl, format);
    va_end(vl);
    NSLogv(format, vl);
}
#define _ASSERT_CG_CONTEXT                                                               \
    if (m_CGContext == nullptr) {                                                        \
        GR_CGContextIsNull(@"%s:%d: %s no CGContext", __FILE__, __LINE__, __FUNCTION__); \
    }

bool GR_CocoaGraphics::m_colorAndImageInited = false;

NSImage* GR_CocoaGraphics::m_imageBlue16x15 = nil;
NSImage* GR_CocoaGraphics::m_imageBlue11x16 = nil;
NSImage* GR_CocoaGraphics::m_imageGrey16x15 = nil;
NSImage* GR_CocoaGraphics::m_imageGrey11x16 = nil;

NSColor* GR_CocoaGraphics::m_colorBlue16x15 = nil;
NSColor* GR_CocoaGraphics::m_colorBlue11x16 = nil;
NSColor* GR_CocoaGraphics::m_colorGrey16x15 = nil;
NSColor* GR_CocoaGraphics::m_colorGrey11x16 = nil;

NSCursor* GR_CocoaGraphics::m_Cursor_E = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_N = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_NE = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_NW = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_S = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_SE = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_SW = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_W = nil;

NSCursor* GR_CocoaGraphics::m_Cursor_Wait = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_LeftArrow = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_RightArrow = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_Compass = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_Exchange = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_LeftRight = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_UpDown = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_Crosshair = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_HandPointer = nil;
NSCursor* GR_CocoaGraphics::m_Cursor_DownArrow = nil;

void GR_CocoaGraphics::_initColorAndImage(void)
{
    NSBundle* bundle = [NSBundle mainBundle];
    NSString* path = nil;
    NSImage* image = nil;

    if (m_colorAndImageInited) {
        return;
    }

    path = [bundle pathForResource:@"Blue16x15" ofType:@"png"];
    if (path) {
        m_imageBlue16x15 = [[NSImage alloc] initWithContentsOfFile:path];
        if (m_imageBlue16x15) {
            m_colorBlue16x15 = [[NSColor colorWithPatternImage:m_imageBlue16x15] retain];
        }
    }
    if (!m_colorBlue16x15) {
        m_colorBlue16x15 = [NSColor.blueColor retain];
    }
    path = [bundle pathForResource:@"Blue11x16" ofType:@"png"];
    if (path) {
        m_imageBlue11x16 = [[NSImage alloc] initWithContentsOfFile:path];
        if (m_imageBlue11x16) {
            m_colorBlue11x16 = [[NSColor colorWithPatternImage:m_imageBlue11x16] retain];
        }
    }
    if (!m_colorBlue11x16) {
        m_colorBlue11x16 = [NSColor.blueColor retain];
    }
    path = [bundle pathForResource:@"Grey16x15" ofType:@"png"];
    if (path) {
        m_imageGrey16x15 = [[NSImage alloc] initWithContentsOfFile:path];
        if (m_imageGrey16x15) {
            m_colorGrey16x15 = [[NSColor colorWithPatternImage:m_imageGrey16x15] retain];
        }
    }
    if (!m_colorGrey16x15) {
        m_colorGrey16x15 = [NSColor.grayColor retain];
    }
    path = [bundle pathForResource:@"Grey11x16" ofType:@"png"];
    if (path) {
        m_imageGrey11x16 = [[NSImage alloc] initWithContentsOfFile:path];
        if (m_imageGrey11x16) {
            m_colorGrey11x16 = [[NSColor colorWithPatternImage:m_imageGrey11x16] retain];
        }
    }
    if (!m_colorGrey11x16) {
        m_colorGrey11x16 = [NSColor.grayColor retain];
    }

    // Cursors
    path = [bundle pathForResource:@"Cursor_E" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_E = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7, 7)];
            [image release];
        }
    }
    if (!m_Cursor_E) {
        m_Cursor_E = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"Cursor_N" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_N = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7, 8)];
            [image release];
        }
    }
    if (!m_Cursor_N) {
        m_Cursor_N = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"Cursor_NE" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_NE = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8, 7)];
            [image release];
        }
    }
    if (!m_Cursor_NE) {
        m_Cursor_NE = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"Cursor_NW" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_NW = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7, 7)];
            [image release];
        }
    }
    if (!m_Cursor_NW) {
        m_Cursor_NW = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"Cursor_S" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_S = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7, 7)];
            [image release];
        }
    }
    if (!m_Cursor_S) {
        m_Cursor_S = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"Cursor_SE" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_SE = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8, 8)];
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
            m_Cursor_SW = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7, 8)];
            [image release];
        }
    }
    if (!m_Cursor_SW) {
        m_Cursor_SW = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"Cursor_W" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_W = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8, 7)];
            [image release];
        }
    }
    if (!m_Cursor_W) {
        m_Cursor_W = [NSCursor.arrowCursor retain];
    }

    path = [bundle pathForResource:@"Cursor_Wait" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_Wait = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8, 7)];
            [image release];
        }
    }
    if (!m_Cursor_Wait) {
        m_Cursor_Wait = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"Cursor_LeftArrow" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_LeftArrow = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7, 8)];
            [image release];
        }
    }
    if (!m_Cursor_LeftArrow) {
        m_Cursor_LeftArrow = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"Cursor_RightArrow" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_RightArrow = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8, 7)];
            [image release];
        }
    }
    if (!m_Cursor_RightArrow) {
        m_Cursor_RightArrow = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"Cursor_Compass" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_Compass = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7, 7)];
            [image release];
        }
    }
    if (!m_Cursor_Compass) {
        m_Cursor_Compass = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"Cursor_Exchange" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_Exchange = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7, 7)];
            [image release];
        }
    }
    if (!m_Cursor_Exchange) {
        m_Cursor_Exchange = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"leftright_cursor" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_LeftRight = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8, 8)];
            [image release];
        }
    }
    if (!m_Cursor_LeftRight) {
        m_Cursor_LeftRight = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"updown_cursor" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_UpDown = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(8, 8)];
            [image release];
        }
    }
    if (!m_Cursor_UpDown) {
        m_Cursor_UpDown = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"Cursor_Crosshair" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_Crosshair = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(7, 7)];
            [image release];
        }
    }
    if (!m_Cursor_Crosshair) {
        m_Cursor_Crosshair = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"Cursor_HandPointer" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_HandPointer = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(6, 0)];
            [image release];
        }
    }
    if (!m_Cursor_HandPointer) {
        m_Cursor_HandPointer = [NSCursor.arrowCursor retain];
    }
    path = [bundle pathForResource:@"Cursor_DownArrow" ofType:@"png"];
    if (path) {
        image = [[NSImage alloc] initWithContentsOfFile:path];
        if (image) {
            m_Cursor_DownArrow = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(6, 0)];
            [image release];
        }
    }
    if (!m_Cursor_DownArrow) {
        m_Cursor_DownArrow = [NSCursor.arrowCursor retain];
    }

    m_colorAndImageInited = true;
}

GR_CocoaGraphics::GR_CocoaGraphics(XAP_CocoaNSView* view)
    : m_updateCallback(nullptr)
    , m_updateCBparam(nullptr)
    , m_view(view)
    , m_CGContext(nullptr)
    , m_cacheArray(10)
    , m_cacheRectArray(10)
    , m_currentColor(nil)
    , m_pFont(nullptr)
    , m_fontForGraphics(nil)
    , m_pFontGUI(nullptr)
    , m_fLineWidth(1.0)
    , m_joinStyle(JOIN_MITER)
    , m_capStyle(CAP_BUTT)
    , m_lineStyle(LINE_SOLID)
    , m_GrabCursor(GR_CURSOR_DEFAULT)
    , m_screenResolution(0)
    , m_bIsPrinting(false)
    , m_bIsDrawing(false)
{
    _initColorAndImage();

    UT_ASSERT(m_view);
    if (![m_view isKindOfClass:[XAP_CocoaNSView class]]) {
        NSLog(@"attaching a non-XAP_CocoaNSView to a GR_CocoaGraphics");
    }

    [m_view setGraphics:this];
    init3dColors();

    /* resolution does not change thru the life of the object */
    m_screenResolution = lrintf(_getScreenResolution());

    m_currentColor = [[NSColor blackColor] retain];

    m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
    m_cursor = GR_CURSOR_INVALID;
    setCursor(GR_CURSOR_DEFAULT);
}

#ifndef RELEASEP
#define RELEASEP(X)  \
    do {             \
        [X release]; \
        X = nil;     \
    } while (0)
#endif

GR_CocoaGraphics::~GR_CocoaGraphics()
{
    DELETEP(m_pFontGUI);

    _destroyFonts();

    UT_std_vector_releaseall(m_cacheArray);
    UT_std_vector_sparsepurgeall(m_cacheRectArray);

    [m_view setGraphics:nullptr];
    [m_fontForGraphics release];
    [m_currentColor release];

    for (int i = 0; i < COUNT_3D_COLORS; i++) {
        [m_3dColors[i] release];
    }
}

void GR_CocoaGraphics::fillNSRect(NSRect& aRect, NSColor* color)
{
    _ASSERT_CG_CONTEXT;

    GR_CGStateSave state(m_CGContext);
    ::CGContextSetFillColorWithColor(m_CGContext, color.CGColor);
    ::CGContextFillRect(m_CGContext, aRect);
}

void GR_CocoaGraphics::_beginPaint(void)
{
    if (!m_view.in_draw_rect) {
        NSLog(@"Not in drawRect. BREAK HERE if you need.");
    }
    NSGraphicsContext* context = context = NSGraphicsContext.currentContext;
    m_CGContext = context.CGContext;
    if (!m_CGContext) {
        // no context, let's draw in the CGLayer
        m_CGContext = ::CGLayerGetContext(getCGLayer());
    }
    _ASSERT_CG_CONTEXT;

    _resetContext();

#if 0
    NSRect aRect = m_view.bounds;
    {
        GR_CGStateSave state(m_CGContext);
        ::CGContextSetFillColorWithColor(m_CGContext, ::CGColorGetConstantColor(kCGColorWhite));
        ::CGContextFillRect(m_CGContext, aRect);
    }
#endif
    _setClipRectImpl(nullptr);
}

void GR_CocoaGraphics::_endPaint(void)
{
    m_CGContext = nullptr;
}

bool GR_CocoaGraphics::queryProperties(GR_Graphics::Properties gp) const
{
    switch (gp) {
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
    GR_Graphics::setZoomPercentage(iZoom); // chain up
}

void GR_CocoaGraphics::setLineProperties(double inWidth,
    JoinStyle inJoinStyle,
    CapStyle inCapStyle,
    LineStyle inLineStyle)
{
    m_fLineWidth = tduD(inWidth);
    m_joinStyle = inJoinStyle;
    m_capStyle = inCapStyle;
    m_lineStyle = inLineStyle;

    if (m_view.in_draw_rect) {
        _ASSERT_CG_CONTEXT;
        ::CGContextSetLineWidth(m_CGContext, m_fLineWidth);
        _setCapStyle(m_capStyle);
        _setJoinStyle(m_joinStyle);
        _setLineStyle(m_lineStyle);
    }
}

void GR_CocoaGraphics::_setCapStyle(CapStyle inCapStyle)
{
    _ASSERT_CG_CONTEXT;
    CGLineCap cap;

    switch (inCapStyle) {
    case CAP_BUTT:
        cap = kCGLineCapButt;
        break;
    case CAP_ROUND:
        cap = kCGLineCapRound;
        break;
    case CAP_PROJECTING:
        cap = kCGLineCapSquare;
        break;
    default:
        UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        return;
    }
    ::CGContextSetLineCap(m_CGContext, cap);
}

void GR_CocoaGraphics::_setJoinStyle(JoinStyle inJoinStyle)
{
    _ASSERT_CG_CONTEXT;
    CGLineJoin join;

    switch (inJoinStyle) {
    case JOIN_MITER:
        join = kCGLineJoinMiter;
        break;
    case JOIN_ROUND:
        join = kCGLineJoinRound;
        break;
    case JOIN_BEVEL:
        join = kCGLineJoinBevel;
        break;
    default:
        UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        return;
    }
    ::CGContextSetLineJoin(m_CGContext, join);
}

void GR_CocoaGraphics::_setLineStyle(LineStyle inLineStyle)
{
    _ASSERT_CG_CONTEXT;
    CGFloat lws = m_fLineWidth;

    switch (inLineStyle) {
    case LINE_SOLID:
        ::CGContextSetLineDash(m_CGContext, 0, nullptr, 0);
        break;
    case LINE_ON_OFF_DASH:
    {
        CGFloat dash_list[] = { 4. * lws, 5. * lws };
        ::CGContextSetLineDash(m_CGContext, 0, dash_list, 2);
    } break;
    case LINE_DOUBLE_DASH:
    {
        CGFloat dash_list[] = { 1. * lws, 3. * lws, 4. * lws, 2. * lws };
        ::CGContextSetLineDash(m_CGContext, 0, dash_list, 4);
    } break;
    case LINE_DOTTED:
    {
        CGFloat dash_list[] = { 1. * lws, 4. * lws };
        ::CGContextSetLineDash(m_CGContext, 0, dash_list, 2);
    } break;
    default:
        UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    }
}

void GR_CocoaGraphics::_drawTextLine(CTLineRef ctLine,
    CGFloat x, CGFloat y)
{
    // NSLog(@"real draw char x=%lf, y=%lf", x, y);

    ::CGContextSetTextPosition(m_CGContext, x, y);
    ::CTLineDraw(ctLine, m_CGContext);
}

void GR_CocoaGraphics::_drawSpacedTextLine(CTLineRef ctLine,
    CGFloat x, CGFloat y, int length,
    int* charWidths)
{
    CFArrayRef glyphRuns = ::CTLineGetGlyphRuns(ctLine);
    // NSLog(@"drawSpacedLine: glyphCount %ld, run count %lu, {%lf, %lf}", (long)::CTLineGetGlyphCount(ctLine), [(id)glyphRuns count], x, y);
    CFIndex runIndex = 0;
    for (int i = 0; i < length;) {
        CTRunRef currentRun = (CTRunRef)::CFArrayGetValueAtIndex(glyphRuns, runIndex);
        UT_ASSERT(currentRun != nullptr);
        CFIndex runGlyphCount = ::CTRunGetGlyphCount(currentRun);
        runIndex++;
        CFIndex indexInRun = 0;
        const CGGlyph* glyphs = ::CTRunGetGlyphsPtr(currentRun);

        std::unique_ptr<CGPoint[]> points(new CGPoint[runGlyphCount]);
        for (int inRun = 0; inRun < runGlyphCount; inRun++) {
            points[inRun] = ::NSMakePoint(x, y);
            x += TDUX(charWidths[i + inRun]);
        }
        ::CTFontDrawGlyphs((CTFontRef)m_fontForGraphics, glyphs + indexInRun, points.get(), runGlyphCount, m_CGContext);
        i += runGlyphCount;
    }
}

void GR_CocoaGraphics::drawChars(const UT_UCS4Char* pChars, int iCharOffset,
    int iLength, UT_sint32 xoffLU, UT_sint32 yoffLU,
    int* pCharWidths)
{
    UT_DEBUGMSG(("GR_CocoaGraphics::drawChars()\n"));
    _ASSERT_CG_CONTEXT;

    CGFloat xoff = xoffLU; // layout Unit !

    const UT_UCS4Char* begin = pChars + iCharOffset;

    NSFont* font = m_fontForGraphics;
    NSDictionary* attributes = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];
    NSString* string = [[[NSString alloc]
        initWithBytes:(void*)begin
               length:(NSUInteger)iLength * sizeof(UT_UCS4Char)
             encoding:NSUTF32LittleEndianStringEncoding]
        autorelease];
    // NSLog(@"drawChars string=%@ font=%@", string, font);
    NSAttributedString* attrString = [[[NSAttributedString alloc]
        initWithString:string
            attributes:attributes]
        autorelease];

    CTLineRef ctLine = ::CTLineCreateWithAttributedString((CFAttributedStringRef)attrString);

    GR_CGStateSave state(m_CGContext);
    ::CGContextSetShouldAntialias(m_CGContext, true);

    // This is needed to set the text transformation matrix to sensible
    // value
    ::CGContextSetTextMatrix(m_CGContext, CGAffineTransformIdentity);

    ::CGContextTranslateCTM(m_CGContext, 0, m_view.bounds.size.height);
    ::CGContextScaleCTM(m_CGContext, 1.0, -1.0);
    // The position is on the baseline, while yoffLU is the top of the bounding box.
    CGFloat y = m_view.bounds.size.height - (_tduYD(yoffLU) + [font ascender]);

    if (!pCharWidths) {
        /*
		  We don't have char widths because we don't care. Just draw the text.
		 */
        _drawTextLine(ctLine, TDUX(xoff), y);
    } else {
        _drawSpacedTextLine(ctLine, TDUX(xoff), y, iLength, pCharWidths + iCharOffset);
    }
    ::CGContextSetShouldAntialias(m_CGContext, false);
    ::CFRelease(ctLine);
}

void GR_CocoaGraphics::setFont(const GR_Font* pFont)
{
    const GR_CocoaFont* pUFont = static_cast<const GR_CocoaFont*>(pFont);
    UT_ASSERT(pUFont);
    m_pFont = pUFont;
    [m_fontForGraphics release];
    NSFont* font = pUFont->getNSFont();
    m_fontForGraphics = [[[NSFontManager sharedFontManager] convertFont:font
                                                                 toSize:[font pointSize] * (getZoomPercentage() / 100.)] retain];
}

UT_uint32 GR_CocoaGraphics::getFontHeight(const GR_Font* fnt)
{
    UT_ASSERT(fnt);
    return static_cast<UT_uint32>(lrint(ftluD(static_cast<const GR_CocoaFont*>(fnt)->getHeight())));
}

UT_uint32 GR_CocoaGraphics::getFontHeight()
{
    return static_cast<UT_uint32>(lrint(ftluD(m_pFont->getHeight())));
}

/*!
	Internal version to measure unremapped char
	
	\return width in Device Unit
 */
CGFloat GR_CocoaGraphics::_measureUnRemappedCharCached(const UT_UCSChar c)
{
    CGFloat width;
    width = m_pFont->getCharWidthFromCache(c);
    width *= (m_pFont->getSize() / GR_CharWidthsCache::CACHE_FONT_SIZE);
    return width;
}

/*!
	Measure unremapped char
	
	\return width in Layout Unit
 */
UT_sint32 GR_CocoaGraphics::measureUnRemappedChar(const UT_UCSChar c, UT_uint32* height)
{
    // measureString() could be defined in terms of measureUnRemappedChar()
    // but its not (for presumed performance reasons).  Also, a difference
    // is that measureString() uses remapping to get past zero-width
    // character cells.
    if (height != nullptr) {
        *height = 0;
    }
    if (c == 0x200B || c == 0xFEFF) { // 0-with spaces
        return 0;
    }

    NSFont* font = m_pFont->getNSFont();
    CGGlyph glyph = 0;
    UniChar chars = c; // XXX BAD BAD we truncate here.
    ::CTFontGetGlyphsForCharacters((CTFontRef)font, &chars, &glyph, 1);
    CGSize size;
    double advance = ::CTFontGetAdvancesForGlyphs((CTFontRef)font, kCTFontOrientationDefault, &glyph, &size, 1);

    return static_cast<UT_uint32>(lrint(ftluD(advance)));
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
NSColor* GR_CocoaGraphics::_utRGBColorToNSColor(const UT_RGBColor& clr)
{
    CGFloat r, g, b;
    r = (CGFloat)clr.m_red / 255.0f;
    g = (CGFloat)clr.m_grn / 255.0f;
    b = (CGFloat)clr.m_blu / 255.0f;
    UT_DEBUGMSG(("converting color r=%lf, g=%lf, b=%lf from %d, %d, %d\n", r, g, b, clr.m_red, clr.m_grn, clr.m_blu));
    NSColor* c = [NSColor colorWithDeviceRed:r green:g blue:b alpha:1.0 /*(clr.m_bIsTransparent ? 0.0 : 1.0)*/];
    return c;
}

void GR_CocoaGraphics::setColor(const UT_RGBColor& clr)
{
    UT_DEBUGMSG(("GR_CocoaGraphics::setColor(const UT_RGBColor&): setting color %d, %d, %d\n", clr.m_red, clr.m_grn, clr.m_blu));
    NSColor* c = _utRGBColorToNSColor(clr);
    _setColor(c);
}

void GR_CocoaGraphics::getColor(UT_RGBColor& clr)
{
    _utNSColorToRGBColor(m_currentColor, clr);
}

/* c will be copied */
void GR_CocoaGraphics::_setColor(NSColor* c)
{
    UT_DEBUGMSG(("GR_CocoaGraphics::_setColor(NSColor*): setting NSColor\n"));
    [m_currentColor release];
    m_currentColor = [c retain];

    if (m_view.in_draw_rect) {
        _ASSERT_CG_CONTEXT;
        ::CGContextSetStrokeColorWithColor(m_CGContext, m_currentColor.CGColor);
        ::CGContextSetFillColorWithColor(m_CGContext, m_currentColor.CGColor);
    }
}

GR_Font* GR_CocoaGraphics::getGUIFont(void)
{
    if (!m_pFontGUI) {
        // get the font resource
        UT_DEBUGMSG(("GR_CocoaGraphics::getGUIFont: getting default font\n"));
        // bury it in a new font handle
        m_pFontGUI = new GR_CocoaFont([NSFont labelFontOfSize:
                                                  ([NSFont labelFontSize] * 100.0 / getZoomPercentage())]); // Hardcoded GUI font size
        UT_ASSERT(m_pFontGUI);
    }

    return m_pFontGUI;
}

GR_Font* GR_CocoaGraphics::_findFont(const char* pszFontFamily,
    const char* pszFontStyle,
    const char* pszFontVariant,
    const char* pszFontWeight,
    const char* pszFontStretch,
    const char* pszFontSize,
    const char* pszLang)
{
    UT_DEBUGMSG(("GR_CocoaGraphics::findFont(%s, %s, %s)\n", pszFontFamily, pszFontStyle, pszFontSize));

    UT_ASSERT(pszFontFamily);
    UT_ASSERT(pszFontStyle);
    UT_ASSERT(pszFontWeight);
    UT_ASSERT(pszFontSize);

    CGFloat size = ceil(UT_convertToPoints(pszFontSize));

    size = (size < 1.0) ? 1.0 : size;

    NSString* font_name = [NSString stringWithUTF8String:pszFontFamily];

    NSString* family_name = [(XAP_CocoaAppController*)[NSApp delegate] familyNameForFont:font_name];

    NSFont* nsfont = nil;

    if (family_name) {
        // fprintf(stderr, "*font* name: \"%s\", size=%lgpt\n", [font_name UTF8String], size);
        /* this looks like a font name, not a font-family name...
		 */
        nsfont = [NSFont fontWithName:font_name size:((CGFloat)size)];
    }
    if (!nsfont) {
        /* this probably is a real font-family name, not a font name...
		 */
        family_name = font_name;
        // fprintf(stderr, "family name: \"%s\", size=%lgpt\n", [family_name UTF8String], size);
        NSFontTraitMask s = 0;

        // this is kind of sloppy
        if (strcmp(pszFontStyle, "italic") == 0) {
            s |= NSItalicFontMask;
        }
        if (strcmp(pszFontWeight, "bold") == 0) {
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
            if (g_ascii_strcasecmp(pszFontFamily, "Dingbats") == 0) {
                GR_Font* pGRFont = findFont("Zapf Dingbats", pszFontStyle, pszFontVariant, pszFontWeight, pszFontStretch, pszFontSize, pszLang);
                GR_CocoaFont* pFont = static_cast<GR_CocoaFont*>(pGRFont);
                nsfont = pFont->getNSFont();
            }
            if (g_ascii_strcasecmp(pszFontFamily, "Helvetic") == 0) {
                GR_Font* pGRFont = findFont("Helvetica", pszFontStyle, pszFontVariant, pszFontWeight, pszFontStretch, pszFontSize, pszLang);
                GR_CocoaFont* pFont = static_cast<GR_CocoaFont*>(pGRFont);
                nsfont = pFont->getNSFont();
            }
        }
        if (!nsfont) // this is bad; the wrong font name ends up in the font popups - FIXME please!
        {
            /* Oops!  We don't have that font here.
			 * first try "Times New Roman", which should be sensible, and should
			 * be there unless the user fidled with the installation
			 */
            NSLog(@"Unable to find font \"%s\".", pszFontFamily);

            nsfont = [[NSFontManager sharedFontManager] fontWithFamily:@"Times" traits:s weight:5 size:size];
        }
    }

    /* bury the pointer to our Cocoa font in a GR_CocoaFont */
    return new GR_CocoaFont(nsfont);
}

// returns in LU
UT_uint32 GR_CocoaGraphics::getFontAscent(const GR_Font* fnt)
{
    UT_ASSERT(fnt);
    return static_cast<UT_uint32>(lrint(ftluD(static_cast<const GR_CocoaFont*>(fnt)->getAscent())));
}

// returns in LU
UT_uint32 GR_CocoaGraphics::getFontAscent()
{
    return static_cast<UT_uint32>(lrint(ftluD(m_pFont->getAscent())));
}

// returns in LU
UT_uint32 GR_CocoaGraphics::getFontDescent(const GR_Font* fnt)
{
    UT_ASSERT(fnt);
    return static_cast<UT_uint32>(lrint(ftluD(static_cast<const GR_CocoaFont*>(fnt)->getDescent())));
}

UT_uint32 GR_CocoaGraphics::getFontDescent()
{
    return static_cast<UT_uint32>(lrint(ftluD(m_pFont->getDescent())));
}

void GR_CocoaGraphics::rawPolyAtOffset(NSPoint* point, int npoint, UT_sint32 offset_x, UT_sint32 offset_y, NSColor* color, bool bFill)
{
    UT_ASSERT(point && (npoint > 2) && color);
    if (!(point && (npoint > 2) && color)) {
        return;
    }

    _ASSERT_CG_CONTEXT;

    GR_CGStateSave state(m_CGContext);
    ::CGContextBeginPath(m_CGContext);
    ::CGContextMoveToPoint(m_CGContext, TDUX(offset_x) + point[0].x, _tduY(offset_y) + point[0].y);

    for (int i = 1; i < npoint; i++) {
        ::CGContextAddLineToPoint(m_CGContext, TDUX(offset_x) + point[i].x, _tduY(offset_y) + point[i].y);
    }

    ::CGContextClosePath(m_CGContext);

    if (bFill) {
        ::CGContextSetFillColorWithColor(m_CGContext, color.CGColor);
        ::CGContextFillPath(m_CGContext);
    } else {
        ::CGContextSetStrokeColorWithColor(m_CGContext, color.CGColor);
        ::CGContextStrokePath(m_CGContext);
    }
}

void GR_CocoaGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
    UT_sint32 x2, UT_sint32 y2)
{
    UT_DEBUGMSG(("GR_CocoaGraphics::drawLine(%d, %d, %d, %d) width=%lf\n", x1, y1, x2, y2, m_fLineWidth));
    _ASSERT_CG_CONTEXT;

    // if ((y1 == y2) && (x1 >= 500)) fprintf(stderr, "GR_CocoaGraphics::drawLine(%ld, %ld, %ld, %ld) width=%f\n", x1, y1, x2, y2, m_fLineWidth);
    GR_CGStateSave state(m_CGContext);
    ::CGContextBeginPath(m_CGContext);
    ::CGContextMoveToPoint(m_CGContext, TDUX(x1), _tduY(y1));
    ::CGContextAddLineToPoint(m_CGContext, TDUX(x2), _tduY(y2));
    ::CGContextSetStrokeColorWithColor(m_CGContext, m_currentColor.CGColor);
    ::CGContextStrokePath(m_CGContext);
}

void GR_CocoaGraphics::setLineWidth(UT_sint32 iLineWidth)
{
    UT_DEBUGMSG(("GR_CocoaGraphics::setLineWidth(%d) was %lf\n", iLineWidth, m_fLineWidth));
    m_fLineWidth = ceil(tduD(iLineWidth) - 0.75);
    m_fLineWidth = (m_fLineWidth > 0) ? m_fLineWidth : 1.0f;

    if (m_view.in_draw_rect) {
        _ASSERT_CG_CONTEXT;

        ::CGContextSetLineWidth(m_CGContext, m_fLineWidth);
        _setLineStyle(m_lineStyle);
    }
}

void GR_CocoaGraphics::polyLine(const UT_Point* pts, UT_uint32 nPoints)
{
    UT_DEBUGMSG(("GR_CocoaGraphics::polyLine() width=%f\n", m_fLineWidth));
    _ASSERT_CG_CONTEXT;

    GR_CGStateSave state(m_CGContext);
    ::CGContextBeginPath(m_CGContext);

    ::CGContextMoveToPoint(m_CGContext, TDUX(pts[0].x), _tduY(pts[0].y));
    for (UT_uint32 i = 1; i < nPoints; i++) {
        ::CGContextAddLineToPoint(m_CGContext, TDUX(pts[i].x), _tduY(pts[i].y));
    }
    ::CGContextStrokePath(m_CGContext);
}

void GR_CocoaGraphics::invertRect(const UT_Rect* pRect)
{
    UT_DEBUGMSG(("GR_CocoaGraphics::invertRect()\n"));
    UT_ASSERT(pRect);

    // TODO handle invert. this is highlight.

    NSHighlightRect(NSMakeRect(TDUX(pRect->left), _tduY(pRect->top), _tduR(pRect->width), _tduR(pRect->height)));
}

void GR_CocoaGraphics::queueDraw(const UT_Rect* pRect)
{
    CGRect clip;
    if (pRect) {
        clip = ::CGRectMake(TDUX(pRect->left), _tduY(pRect->top), _tduR(pRect->width), _tduR(pRect->height));
    } else {
        clip = m_view.bounds;
    }
    [m_view setNeedsDisplayInRect:clip];
}

void GR_CocoaGraphics::setClipRect(const UT_Rect* pRect)
{
    UT_DEBUGMSG(("GR_CocoaGraphics::setClipRect()\n"));
    if (pRect) {
        m_pRect.reset(new UT_Rect(*pRect));
    } else {
        m_pRect.reset();
    }
}

void GR_CocoaGraphics::_setClipRectImpl(const UT_Rect*)
{
    _ASSERT_CG_CONTEXT;
    if (m_pRect) {
        UT_DEBUGMSG(("ClipRect set\n"));
        ::CGContextClipToRect(m_CGContext,
            ::CGRectMake(TDUX(m_pRect->left), _tduY(m_pRect->top), _tduR(m_pRect->width), _tduR(m_pRect->height)));
    } else {
        UT_DEBUGMSG(("ClipRect reset!!\n"));
        ::CGContextClipToRect(m_CGContext, m_view.bounds); // ??
    }
}

void GR_CocoaGraphics::fillRect(const UT_RGBColor& clr, UT_sint32 x, UT_sint32 y,
    UT_sint32 w, UT_sint32 h)
{
    UT_DEBUGMSG(("GR_CocoaGraphics::fillRect(UT_RGBColor&, %d, %d, %d, %d)\n", x, y, w, h));

    /* make this as accurate as possible, though it's still not perfect :-(
	 */
    double _x1 = tduD(x);
    double _y1 = tduD(y);

    double x1 = floor(_x1);
    double y1 = floor(_y1);

    double _x2 = tduD(x + w);
    double _y2 = tduD(y + h);

    double x2 = ceil(_x2);
    double y2 = ceil(_y2);

    double dw = ceil(tduD(w));
    double dh = ceil(tduD(h));

    CGFloat f_x = x1;
    CGFloat f_y = y1;
    CGFloat f_w = dw;
    CGFloat f_h = dh;

    if (((_x1 - x1) > 0.75) || ((x2 - _x2) < 0.25) || ((_x1 - x1) > (x2 - _x2))) {
        f_x = x2 - dw;
    }
    if (((_y1 - y1) > 0.75) || ((y2 - _y2) < 0.25) || ((_y1 - y1) > (y2 - _y2))) {
        f_y = y2 - dh;
    }

    // save away the current color, and restore it after we fill the rect
    NSColor* c = _utRGBColorToNSColor(clr);

    _ASSERT_CG_CONTEXT;

    GR_CGStateSave state(m_CGContext);
    ::CGContextSetFillColorWithColor(m_CGContext, c.CGColor);
    ::CGContextFillRect(m_CGContext, ::CGRectMake(f_x, f_y, f_w, f_h));
}

void GR_CocoaGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
    UT_ASSERT(c < COUNT_3D_COLORS);
    UT_DEBUGMSG(("GR_CocoaGraphics::fillRect(GR_Color3D %d, %d, %d, %d, %d)\n", c, x, y, w, h));

    _ASSERT_CG_CONTEXT;

    GR_CGStateSave state(m_CGContext);
    ::CGContextSetFillColorWithColor(m_CGContext, m_3dColors[c].CGColor);
    ::CGContextFillRect(m_CGContext, ::CGRectMake(tdu(x), tdu(y), tdu(w), tdu(h)));
}

void GR_CocoaGraphics::fillRect(GR_Color3D c, UT_Rect& r)
{
    UT_DEBUGMSG(("GR_CocoaGraphics::fillRect(GR_Color3D, UT_Rect &)\n"));
    UT_ASSERT(c < COUNT_3D_COLORS);
    fillRect(c, r.left, r.top, r.width, r.height);
}

void GR_CocoaGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
    if (!dx && !dy)
        return;

    UT_sint32 oldDY = tdu(getPrevYOffset());
    UT_sint32 oldDX = tdu(getPrevXOffset());
    UT_sint32 newY = getPrevYOffset() + dy;
    UT_sint32 newX = getPrevXOffset() + dx;
    UT_sint32 ddx = -(tdu(newX) - oldDX);
    UT_sint32 ddy = -(tdu(newY) - oldDY);
    setPrevYOffset(newY);
    setPrevXOffset(newX);

    [m_view displayIfNeeded];

    NSRect bounds = m_view.bounds;
    NSSize offset = NSMakeSize(ddx, ddy);
    [m_view scrollRect:bounds by:offset];

    if (offset.width > 0) {
        if (offset.width < bounds.size.width) {
            if (offset.height > 0) {
                if (offset.height < bounds.size.height) {
                    NSRect tmp;
                    tmp.origin.x = bounds.origin.x + offset.width;
                    tmp.origin.y = bounds.origin.y;
                    tmp.size.width = bounds.size.width - offset.width;
                    tmp.size.height = offset.height;
                    [m_view setNeedsDisplayInRect:tmp];

                    bounds.size.width = offset.width;
                }
            } else if (offset.height < 0) {
                if ((-offset.height) < bounds.size.height) {
                    NSRect tmp;
                    tmp.origin.x = bounds.origin.x + offset.width;
                    tmp.origin.y = bounds.origin.y + bounds.size.height + offset.height;
                    tmp.size.width = bounds.size.width - offset.width;
                    tmp.size.height = -offset.height;
                    [m_view setNeedsDisplayInRect:tmp];

                    bounds.size.width = offset.width;
                }
            } else {
                bounds.size.width = offset.width;
            }
        }
    } else if (offset.width < 0) {
        if ((-offset.width) < bounds.size.width) {
            if (offset.height > 0) {
                if (offset.height < bounds.size.height) {
                    NSRect tmp;
                    tmp.origin = bounds.origin;
                    tmp.size.width = bounds.size.width - offset.width;
                    tmp.size.height = offset.height;
                    [m_view setNeedsDisplayInRect:tmp];

                    bounds.origin.x += bounds.size.width + offset.width;
                    bounds.size.width = -offset.width;
                }
            } else if (offset.height < 0) {
                if ((-offset.height) < bounds.size.height) {
                    NSRect tmp;
                    tmp.origin.x = bounds.origin.x;
                    tmp.origin.y = bounds.origin.y + bounds.size.height + offset.height;
                    tmp.size.width = bounds.size.width + offset.width;
                    tmp.size.height = -offset.height;
                    [m_view setNeedsDisplayInRect:tmp];

                    bounds.origin.x += bounds.size.width + offset.width;
                    bounds.size.width = -offset.width;
                }
            } else {
                bounds.origin.x += bounds.size.width + offset.width;
                bounds.size.width = -offset.width;
            }
        }
    } else {
        if (offset.height > 0) {
            if (offset.height < bounds.size.height) {
                bounds.size.height = offset.height;
            }
        } else if (offset.height < 0) {
            if ((-offset.height) < bounds.size.height) {
                bounds.origin.y += bounds.size.height + offset.height;
                bounds.size.height = -offset.height;
            }
        }
    }
    [m_view setNeedsDisplayInRect:bounds];
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

    [m_view scrollRect:NSMakeRect(tdu(x_dest) - ddx, tdu(y_dest) - ddy, _tduR(width), _tduR(height)) by:NSMakeSize(ddx, ddy)];

    setPrevXOffset(newX);
    setPrevYOffset(newY);
}

void GR_CocoaGraphics::clearArea(UT_sint32 x, UT_sint32 y,
    UT_sint32 width, UT_sint32 height)
{
    UT_DEBUGMSG(("ClearArea: %d %d %d %d\n", x, y, width, height));
    if (width > 0) {
        _ASSERT_CG_CONTEXT;
        GR_CGStateSave state(m_CGContext);
        ::CGContextSetFillColorWithColor(m_CGContext, ::CGColorGetConstantColor(kCGColorWhite));
        ::CGContextFillRect(m_CGContext, ::CGRectMake(TDUX(x), _tduY(y), _tduR(width), _tduR(height)));
    }
}

bool GR_CocoaGraphics::startPrint(void)
{
    UT_ASSERT(m_bIsPrinting);
    return true;
}

bool GR_CocoaGraphics::startPage(const char* /*szPageLabel*/, UT_uint32 /*pageNumber*/,
    bool /*bPortrait*/, UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
{
    UT_ASSERT(m_bIsPrinting);
    return true;
}

bool GR_CocoaGraphics::endPrint(void)
{
    UT_ASSERT(m_bIsPrinting);
    return true;
}

GR_Image* GR_CocoaGraphics::createNewImage(const char* pszName, const UT_ConstByteBufPtr& pBB, const std::string& mimetype, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
{
    GR_Image* pImg = nullptr;
    if (iType == GR_Image::GRT_Raster) {
        pImg = new GR_CocoaImage(pszName);
    } else {
        pImg = new GR_VectorImage(pszName);
    }

    pImg->convertFromBuffer(pBB, mimetype, _tduR(iDisplayWidth), _tduR(iDisplayHeight));
    return pImg;
}

void GR_CocoaGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
    UT_ASSERT(pImg);

    if (pImg->getType() != GR_Image::GRT_Raster) {
        pImg->render(this, (UT_sint32)rint(TDUX(xDest)), (UT_sint32)rint(_tduY(yDest)));
        return;
    }

    GR_CocoaImage* pCocoaImage = static_cast<GR_CocoaImage*>(pImg);

    NSImage* image = pCocoaImage->getNSImage();

    if (image == nil) {
        UT_DEBUGMSG(("Found no image data. This is probably SVG masquerading as a raster!\n"));
        return;
    }

    //   	UT_sint32 iImageWidth = pCocoaImage->getDisplayWidth();
    UT_sint32 iImageHeight = pCocoaImage->getDisplayHeight();
    NSSize size = [image size];

    _ASSERT_CG_CONTEXT;

    GR_CGStateSave state(m_CGContext);
    //	::CGContextTranslateCTM (m_CGContext, -0.5, -0.5);
    [image drawInRect:NSMakeRect(TDUX(xDest), _tduY(yDest), pCocoaImage->getDisplayWidth(), iImageHeight)
             fromRect:NSMakeRect(0, 0, size.width, size.height)
            operation:NSCompositingOperationCopy
             fraction:1.0f];
}

void GR_CocoaGraphics::flush(void)
{
    if (!m_bIsDrawing) {
        [m_view displayIfNeeded];
    }
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

    switch (m_cursor) {
    case GR_CURSOR_DEFAULT:
        // NSLog(@"Cursor default");
        [m_view setCursor:NSCursor.arrowCursor];
        break;

    case GR_CURSOR_IBEAM:
        // NSLog(@"Cursor IBeam");
        [m_view setCursor:NSCursor.IBeamCursor];
        break;

    case GR_CURSOR_VLINE_DRAG:
        // NSLog(@"Cursor VLine Drag (unimplemented)");
        m_cursor = GR_CURSOR_LEFTRIGHT;
        // fall through...

    case GR_CURSOR_LEFTRIGHT:
        // NSLog(@"Cursor LeftRight");
        if (m_cursor != old_cursor) {
            [m_view setCursor:m_Cursor_LeftRight];
        }
        break;

    case GR_CURSOR_HLINE_DRAG:
        // NSLog(@"Cursor HLine Drag (unimplemented)");
        m_cursor = GR_CURSOR_UPDOWN;
        // fall through...

    case GR_CURSOR_UPDOWN:
        // NSLog(@"Cursor UpDown");
        if (m_cursor != old_cursor) {
            [m_view setCursor:m_Cursor_UpDown];
        }
        break;

    case GR_CURSOR_IMAGE:
        // NSLog(@"Cursor Image");
        [m_view setCursor:m_Cursor_Compass];
        break;

    case GR_CURSOR_IMAGESIZE_E:
        // NSLog(@"Cursor ImageSize [ E]");
        [m_view setCursor:m_Cursor_E];
        break;

    case GR_CURSOR_IMAGESIZE_N:
        // NSLog(@"Cursor ImageSize [N ]");
        [m_view setCursor:m_Cursor_N];
        break;

    case GR_CURSOR_IMAGESIZE_NE:
        // NSLog(@"Cursor ImageSize [NE]");
        [m_view setCursor:m_Cursor_NE];
        break;

    case GR_CURSOR_IMAGESIZE_NW:
        // NSLog(@"Cursor ImageSize [NW]");
        [m_view setCursor:m_Cursor_NW];
        break;

    case GR_CURSOR_IMAGESIZE_S:
        // NSLog(@"Cursor ImageSize [S ]");
        [m_view setCursor:m_Cursor_S];
        break;

    case GR_CURSOR_IMAGESIZE_SE:
        // NSLog(@"Cursor ImageSize [SE]");
        [m_view setCursor:m_Cursor_SE];
        break;

    case GR_CURSOR_IMAGESIZE_SW:
        // NSLog(@"Cursor ImageSize [SW]");
        [m_view setCursor:m_Cursor_SW];
        break;

    case GR_CURSOR_IMAGESIZE_W:
        // NSLog(@"Cursor ImageSize [ W]");
        [m_view setCursor:m_Cursor_W];
        break;

    case GR_CURSOR_WAIT:
        // NSLog(@"Cursor Wait");
        [m_view setCursor:m_Cursor_Wait];
        break;

    case GR_CURSOR_RIGHTARROW:
        // NSLog(@"Cursor RightArrow");
        [m_view setCursor:m_Cursor_RightArrow];
        break;

    case GR_CURSOR_LEFTARROW:
        // NSLog(@"Cursor LeftArrow");
        [m_view setCursor:m_Cursor_LeftArrow];
        break;

    case GR_CURSOR_EXCHANGE:
        // NSLog(@"Cursor Exchange");
        [m_view setCursor:m_Cursor_Exchange];
        break;

    case GR_CURSOR_CROSSHAIR:
        // NSLog(@"Cursor Crosshair");
        [m_view setCursor:m_Cursor_Crosshair];
        break;

    case GR_CURSOR_LINK:
        // NSLog(@"Cursor Link");
        [m_view setCursor:m_Cursor_HandPointer];
        break;

    case GR_CURSOR_DOWNARROW:
        // NSLog(@"Cursor DownArrow");
        [m_view setCursor:m_Cursor_DownArrow];
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
    if (!bImplemented) {
        m_cursor = GR_CURSOR_DEFAULT;
        if (m_cursor != old_cursor) {
            [m_view setCursor:NSCursor.arrowCursor];
        }
    }
    if (m_cursor != old_cursor) {
        [m_view.window invalidateCursorRectsForView:m_view];
    }
}

GR_Graphics::Cursor GR_CocoaGraphics::getCursor(void) const
{
    return m_cursor;
}
/*!
    Convert a NSColor to an UT_RGBColor
    \param c NSColor to convert
    \retval clr destination UT_RGBColor.

    Handle the transparency as well.
 */
void GR_CocoaGraphics::_utNSColorToRGBColor(NSColor* c, UT_RGBColor& clr)
{
    UT_DEBUGMSG(("colorSpaceName %s\n", c.colorSpaceName.UTF8String));
    if ([c.colorSpaceName isEqualToString:NSPatternColorSpace]) {
        NSImage* img = c.patternImage;
        UT_ASSERT(img);
        UT_DEBUGMSG(("NSColor pattern size is %f, %f\n", img.size.width, img.size.height));

        clr.setPattern(new GR_CocoaPatternImpl(img));
    } else {
        CGFloat r, g, b, a;
        [[c colorUsingColorSpaceName:NSDeviceRGBColorSpace] getRed:&r green:&g blue:&b alpha:&a];
        UT_DEBUGMSG(("color rgba is %f %f %f %f\n", r, g, b, a));
        clr.m_red = static_cast<uint8_t>(r * 255.0f);
        clr.m_grn = static_cast<uint8_t>(g * 255.0f);
        clr.m_blu = static_cast<uint8_t>(b * 255.0f);
        clr.m_bIsTransparent = false /*(a == 0.0f)*/;
    }
}

void GR_CocoaGraphics::setColor3D(GR_Color3D c)
{
    UT_DEBUGMSG(("GR_CocoaGraphics::setColor3D(GR_Color3D %d)\n", c));
    UT_ASSERT(c < COUNT_3D_COLORS);
    _setColor(m_3dColors[c]);
}

bool GR_CocoaGraphics::getColor3D(GR_Color3D c, UT_RGBColor& /*colour*/)
{
    UT_ASSERT(c < COUNT_3D_COLORS);

    //UT_ASSERT(UT_NOT_IMPLEMENTED);
    return false;
}

void GR_CocoaGraphics::init3dColors()
{
    UT_DEBUGMSG(("init3dColors()\n"));
    m_3dColors[CLR3D_Foreground] = NSColor.blackColor;
    m_3dColors[CLR3D_Background] = NSColor.lightGrayColor;
    m_3dColors[CLR3D_BevelUp] = NSColor.whiteColor;
    m_3dColors[CLR3D_BevelDown] = NSColor.darkGrayColor;
    m_3dColors[CLR3D_Highlight] = NSColor.whiteColor;
}

void GR_CocoaGraphics::polygon(const UT_RGBColor& clr, const UT_Point* pts, UT_uint32 nPoints)
{
    _ASSERT_CG_CONTEXT;

    GR_CGStateSave state(m_CGContext);
    NSColor* c = _utRGBColorToNSColor(clr);
    ::CGContextBeginPath(m_CGContext);
    for (UT_uint32 i = 0; i < nPoints; i++) {
        if (i == 0) {
            ::CGContextMoveToPoint(m_CGContext, TDUX(pts[i].x), _tduY(pts[i].y));
        } else {
            ::CGContextAddLineToPoint(m_CGContext, TDUX(pts[i].x), _tduY(pts[i].y));
        }
    }
    ::CGContextSetFillColorWithColor(m_CGContext, c.CGColor);
    ::CGContextFillPath(m_CGContext);
}

void GR_CocoaGraphics::_setUpdateCallback(gr_cocoa_graphics_update callback, void* param)
{
    m_updateCallback = callback;
    m_updateCBparam = param;
}

/*!
	Call the update Callback that has been set

	\param aRect: the rect that should be updated
	\return false if no callback. Otherwise returns what the callback returns.
 */
bool GR_CocoaGraphics::_callUpdateCallback(NSRect* aRect)
{
    if (m_updateCallback == nullptr) {
        return false;
    }
    m_bIsDrawing = true;
    bool ret = (*m_updateCallback)(aRect, this, m_updateCBparam);
    m_bIsDrawing = false;
    return ret;
}

bool GR_CocoaGraphics::_isFlipped()
{
    return true;
}

//////////////////////////////////////////////////////////////////
// This is a static method in the GR_Font base class implemented
// in platform code.
//////////////////////////////////////////////////////////////////

void GR_Font::s_getGenericFontProperties(const char* szFontName,
    FontFamilyEnum* pff,
    FontPitchEnum* pfp,
    bool* pbTrueType)
{
    // describe in generic terms the named font.

    // Note: most of the unix font handling code is in abi/src/af/xap/unix
    // Note: rather than in the graphics class.  i'm not sure this matters,
    // Note: but it is just different....

    // TODO add code to map the given font name into one of the

    *pff = FF_Unknown;

    NSFont* nsfont = [[NSFontManager sharedFontManager] fontWithFamily:[NSString stringWithUTF8String:szFontName]
                                                                traits:0
                                                                weight:5
                                                                  size:(CGFloat)12.0];

    if ([[NSFontManager sharedFontManager] traitsOfFont:nsfont] == NSFixedPitchFontMask) {
        *pfp = FP_Fixed;
    } else {
        *pfp = FP_Variable;
    }
    *pbTrueType = true; // consider that they are all TT fonts. we don't care
}

GR_Image* GR_CocoaGraphics::genImageFromRectangle(const UT_Rect& r)
{
    GR_CocoaImage* img = new GR_CocoaImage("ScreenShot");
    NSRect rect = NSMakeRect(TDUX(r.left), _tduY(r.top),
        _tduR(r.width), _tduR(r.height));

    NSBitmapImageRep* imageRep = [m_view bitmapImageRepForCachingDisplayInRect:rect];
    [m_view cacheDisplayInRect:rect toBitmapImageRep:imageRep];
    img->setFromImageRep(imageRep);

    return img;
}

void GR_CocoaGraphics::saveRectangle(UT_Rect& rect, UT_uint32 iIndx)
{
    NSRect* cacheRect = new NSRect;
    cacheRect->origin.x = _tduX(rect.left) - 1.0f;
    cacheRect->origin.y = _tduY(rect.top) - 1.0f;
    cacheRect->size.width = _tduR(rect.width) + 2.0f;
    cacheRect->size.height = _tduR(rect.height) + 2.0f;

    NSRect bounds = m_view.bounds;
    if (cacheRect->size.height > bounds.size.height - cacheRect->origin.y) {
        cacheRect->size.height = bounds.size.height - cacheRect->origin.y;
    }

    NSBitmapImageRep* imageRep = [m_view bitmapImageRepForCachingDisplayInRect:*cacheRect];
    [m_view cacheDisplayInRect:*cacheRect toBitmapImageRep:imageRep];
    NSImage* cache = [[NSImage alloc] initWithSize:[imageRep size]];
    [cache addRepresentation:imageRep];

    // update cache arrays
    id oldObj = m_cacheArray[iIndx];
    [oldObj release];
    m_cacheArray[iIndx] = cache;

    NSRect* old = m_cacheRectArray[iIndx];
    if (old) {
        delete old;
    }
    m_cacheRectArray[iIndx] = cacheRect;
}

void GR_CocoaGraphics::restoreRectangle(UT_uint32 iIndx)
{
    NSRect* cacheRect = m_cacheRectArray[iIndx];
    NSImage* cache = m_cacheArray[iIndx];
    NSPoint pt = cacheRect->origin;
    pt.y += cacheRect->size.height;
    [cache drawAtPoint:pt fromRect:NSZeroRect operation:NSCompositingOperationCopy fraction:1.0];
}

UT_uint32 GR_CocoaGraphics::getDeviceResolution(void) const
{
    return _getResolution();
}

void GR_CocoaGraphics::_resetContext()
{
    if (m_CGContext == nullptr) {
        GR_CGContextIsNull(@"Attempted to reset nullptr context");
    }
    ::CGContextScaleCTM(m_CGContext, 1.0, -1.0);
    ::CGContextTranslateCTM(m_CGContext, 0, -m_view.bounds.size.height);
    // TODO check that we properly reset parameters according to what has been saved.
    ::CGContextSetLineWidth(m_CGContext, m_fLineWidth);
    _setCapStyle(m_capStyle);
    _setJoinStyle(m_joinStyle);
    _setLineStyle(m_lineStyle);
    ::CGContextSetShouldAntialias(m_CGContext, false);
    CGColorRef color = m_currentColor.CGColor;
    ::CGContextSetFillColorWithColor(m_CGContext, color);
    ::CGContextSetStrokeColorWithColor(m_CGContext, color);
}

CGFloat GR_CocoaGraphics::_getScreenResolution(void)
{
    static CGFloat fResolution = 0.0;
    if (fResolution == 0.0) {
        NSDictionary* desc = NSScreen.mainScreen.deviceDescription;
        UT_ASSERT(desc);
        NSValue* value = [desc objectForKey:NSDeviceResolution];
        UT_ASSERT(value);
        fResolution = value.sizeValue.height / NSScreen.mainScreen.backingScaleFactor;
    }
    return fResolution;
}

/// Convenience to get the CGLayer, to abstract who owns / creates it.
CGLayerRef GR_CocoaGraphics::getCGLayer() const
{
    return m_view.drawingLayer;
}

GR_Graphics* GR_CocoaGraphics::graphicsAllocator(GR_AllocInfo& info)
{
    GR_CocoaAllocInfo& allocator = static_cast<GR_CocoaAllocInfo&>(info);

    return new GR_CocoaGraphics(allocator.m_view);
}
