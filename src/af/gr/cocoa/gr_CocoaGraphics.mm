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

#define LOCK_CONTEXT__ UT_ASSERT(m_viewLocker)

/*#define LOCK_CONTEXT__	StNSViewLocker locker(m_pWin); \
								m_CGContext = CG_CONTEXT__; \
								_setClipRectImpl(NULL);
*/

#define CG_CONTEXT__ (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort]

#define TDUX(x) (_tduX(x)+1.0)

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

GR_CocoaGraphics::GR_CocoaGraphics(NSView * win, XAP_App * app)
	: m_updateCallback(NULL),
	m_updateCBparam (NULL),
	m_pWin(win),
	m_cacheArray (10),
	m_cacheRectArray (10),
	m_currentColor (nil),
	m_pFont (NULL),
	m_fontForGraphics (nil),
	m_pFontGUI(NULL),
	m_fLineWidth(1.0),
	m_joinStyle(JOIN_MITER),
	m_capStyle(CAP_BUTT),
	m_lineStyle(LINE_SOLID),
	m_screenResolution(0),
	m_bIsPrinting(false),
	m_viewLocker(NULL),
	m_fontMetricsTextStorage(nil),
	m_fontMetricsLayoutManager(nil),
	m_fontMetricsTextContainer(nil)
{
	m_pApp = app;
	UT_ASSERT (m_pWin);
	m_fontProps = [[NSMutableDictionary alloc] init];
	NSRect viewBounds = [m_pWin bounds];
	if (![m_pWin isKindOfClass:[XAP_CocoaNSView class]]) {
		NSLog(@"attaching a non-XAP_CocoaNSView to a GR_CocoaGraphics");
	}

	[m_pWin setXAPFrame:app->getLastFocussedFrame()];
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

GR_CocoaGraphics::~GR_CocoaGraphics()
{
	_destroyFonts ();

	UT_VECTOR_RELEASE(m_cacheArray);
	UT_VECTOR_PURGEALL(NSRect*, m_cacheRectArray);
	[m_pWin setGraphics:NULL];
	[m_fontProps release];
	[m_fontForGraphics release];
	[m_currentColor release];

	s_iInstanceCount--;
	for (int i = 0; i < COUNT_3D_COLORS; i++) {
		[m_3dColors[i] release];
	}
	
	DELETEP(m_viewLocker);
	[m_fontMetricsTextStorage release];
	[m_fontMetricsLayoutManager release];
	[m_fontMetricsTextContainer release];
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

void GR_CocoaGraphics::_setCapStyle(CapStyle inCapStyle)
{
	switch (inCapStyle) {
	case CAP_BUTT:
		::CGContextSetLineCap (m_CGContext, kCGLineCapButt);
		break;
	case CAP_ROUND:
		::CGContextSetLineCap (m_CGContext, kCGLineCapRound);
		break;
	case CAP_PROJECTING:
		::CGContextSetLineCap (m_CGContext, kCGLineCapSquare);
		break;
	default:
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	}
}

void GR_CocoaGraphics::_setJoinStyle(JoinStyle inJoinStyle)
{
	switch (inJoinStyle) {
	case JOIN_MITER:
		::CGContextSetLineJoin (m_CGContext, kCGLineJoinMiter);
		break;
	case JOIN_ROUND:
		::CGContextSetLineJoin (m_CGContext, kCGLineJoinRound);
		break;
	case JOIN_BEVEL:
		::CGContextSetLineJoin (m_CGContext, kCGLineJoinBevel);
		break;
	default:
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	}
}


void GR_CocoaGraphics::_setLineStyle (LineStyle inLineStyle)
{
	switch (inLineStyle) {
	case LINE_SOLID:
		{
			::CGContextSetLineDash (m_CGContext, 0, NULL, 0);
		}
		break;
	case LINE_ON_OFF_DASH:
		{
			float dash_list[2] = { 4, 5 };
			::CGContextSetLineDash (m_CGContext, 0, dash_list, 2);
		}
		break;
	case LINE_DOUBLE_DASH:
		{
			float dash_list[4] = { 1, 3, 4, 2 };
			::CGContextSetLineDash (m_CGContext, 0, dash_list, 4);
		}
		break;
	case LINE_DOTTED:
		{
			float dash_list[2] = { 1, 4 };
			::CGContextSetLineDash (m_CGContext, 0, dash_list, 2);
		}
		break;
	default:
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);	
	}
}

void GR_CocoaGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
								 int iLength, UT_sint32 xoffLU, UT_sint32 yoffLU,
								 int * pCharWidths)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::drawChars()\n"));
	UT_sint32 yoff = _tduY(yoffLU); // - m_pFont->getDescent();
	UT_sint32 xoff = xoffLU;	// layout Unit !

	NSString * string = nil;

    if (!m_fontMetricsTextStorage) {
		_initMetricsLayouts();
    }

	::CGContextSetShouldAntialias (m_CGContext, true);

	if (!pCharWidths) {
		NSPoint point = NSMakePoint (TDUX(xoff), yoff);
		unichar * uniString = (unichar*)malloc((iLength + 1) * sizeof (unichar));
		for (int i = 0; i < iLength; i++) {
			uniString[i] = pChars[i + iCharOffset];
		}
		string =  [[NSString alloc] initWithCharacters:uniString length:iLength];
		NSAttributedString* attributedString = [[NSAttributedString alloc] initWithString:string attributes:m_fontProps];
		[m_fontMetricsTextStorage setAttributedString:attributedString];
		[string release];
		[attributedString release];
		FREEP(uniString);

		LOCK_CONTEXT__;
		[m_fontMetricsLayoutManager drawGlyphsForGlyphRange:NSMakeRange(0, iLength) atPoint:point];
	}
	else {
		LOCK_CONTEXT__;
		int i;
		bool bSetAttributes = false;
		unichar *cBuf = (unichar*)malloc((iLength + 1) * sizeof(unichar));
		const UT_UCSChar* begin = pChars + iCharOffset;
		const UT_UCSChar* end;
		UT_sint32 currentRunLen = 0;
		for (i = 0; i <= iLength; i++) {
			end =  pChars + iCharOffset + i;
			unichar c2 = *end;
			if ((c2 == ' ') || (c2 == '\t') || (i == iLength)) {
				const UT_UCSChar* current = begin;
				unichar* stuff = cBuf;
				int len = 0;
				while (current != end) {
					*stuff =  *current;
					current++;
					stuff++;
					len++;
				}
				*stuff = 0;
				string =  [[NSString alloc] initWithCharacters:cBuf length:len];
				NSAttributedString* attributedString = [[NSAttributedString alloc] initWithString:string attributes:m_fontProps];
				[m_fontMetricsTextStorage setAttributedString:attributedString];
				bSetAttributes = true;
				[attributedString release];
				[string release];
		
				NSPoint point = NSMakePoint (TDUX(xoff), yoff);
		
				[m_fontMetricsLayoutManager drawGlyphsForGlyphRange:NSMakeRange(0, len) atPoint:point];
				xoff += currentRunLen;	
				if (i < iLength - 1) {
					xoff += pCharWidths[iCharOffset+i];
				}
				currentRunLen = 0;	
				begin =  end + 1;
			}
			else if (i < iLength - 1) {
				currentRunLen += pCharWidths[iCharOffset+i];
			}
		}
		FREEP(cBuf);
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

	NSFont*		nsfont;
	double size = UT_convertToPoints(pszFontSize);
	nsfont = [[NSFontManager sharedFontManager] fontWithFamily:[NSString stringWithUTF8String:pszFontFamily] 
		traits:s weight:5 size:size];
	if (!nsfont)
	{
		/* 
		add a few hooks for a few predefined font names that MAY differ.
		for example "Dingbats" is called "Zapf Dingbats" on MacOS X. 
		Only fallback AFTER. WARNING: this is recursive call, watch out the
		font family you pass.
		*/
		if (UT_stricmp(pszFontFamily, "Dingbats") == 0) {
			return findFont("Zapf Dingbats", pszFontStyle, pszFontVariant, 
			                    pszFontWeight, pszFontStretch, pszFontSize);
		}
		// Oops!  We don't have that font here.
		// first try "Times New Roman", which should be sensible, and should
		// be there unless the user fidled with the installation
		NSLog (@"Unable to find font \"%s\".", pszFontFamily);
		nsfont = [[NSFontManager sharedFontManager] fontWithFamily:@"Times" 
		                traits:s weight:5 size:size];
	}

	// bury the pointer to our Cocoa font in a XAP_CocoaFontHandle 
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

void GR_CocoaGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
							   UT_sint32 x2, UT_sint32 y2)
{
	LOCK_CONTEXT__;
	UT_DEBUGMSG (("GR_CocoaGraphics::drawLine(%ld, %ld, %ld, %ld) width=%f\n", x1, y1, x2, y2,
	              m_fLineWidth));
	::CGContextBeginPath(m_CGContext);
	::CGContextMoveToPoint (m_CGContext, TDUX(x1), _tduY(y1));
	::CGContextAddLineToPoint (m_CGContext, TDUX(x2), _tduY(y2));
	[m_currentColor set];
	::CGContextStrokePath (m_CGContext);
}

void GR_CocoaGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::setLineWidth(%ld) was %f\n", iLineWidth, m_fLineWidth));
	m_fLineWidth = tduD(iLineWidth);
	if (m_viewLocker) {
		::CGContextSetLineWidth (m_CGContext, m_fLineWidth);
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
	DELETEP(m_pRect);
	if (pRect) {
		m_pRect = new UT_Rect(pRect);
	}
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
	::CGContextFillRect(m_CGContext, ::CGRectMake (TDUX(x), _tduY(y), _tduR(w), _tduR(h)));
	::CGContextRestoreGState(m_CGContext);
}

void GR_CocoaGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	UT_DEBUGMSG(("GR_CocoaGraphics::fillRect(GR_Color3D %d, %ld, %ld, %ld, %ld)\n", c, x, y, w, h));

	LOCK_CONTEXT__;
	::CGContextSaveGState(m_CGContext);
	[m_3dColors[c] set];
	::CGContextFillRect (m_CGContext, ::CGRectMake (TDUX(x), _tduY(y), _tduR(w), _tduR(h)));
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
	UT_sint32 oldDY = tdu(getPrevYOffset());
	UT_sint32 oldDX = tdu(getPrevXOffset());
	UT_sint32 newY = getPrevYOffset() + dy;
	UT_sint32 newX = getPrevXOffset() + dx;
	UT_sint32 ddx = -(tdu(newX) - oldDX);
	UT_sint32 ddy = -(tdu(newY) - oldDY);
	setPrevYOffset(newY);
	setPrevXOffset(newX);
	[m_pWin scrollRect:[m_pWin bounds] by:NSMakeSize(ddx,ddy)];
}

void GR_CocoaGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  UT_sint32 width, UT_sint32 height)
{
	UT_sint32 dx, dy;
	dx = tdu(x_src - x_dest);
	dy = tdu(y_src - y_dest);
	printf ("scrollrect: %f %f %f %f -> %d %d\n", (float)tdu(x_src), (float)tdu(y_src), 
			(float)tdu(width), (float)tdu(height), -dx, dy);
	[m_pWin scrollRect:NSMakeRect(tduD(x_src), tduD(y_src), tduD(width), tduD(height)) 
				by:NSMakeSize(-dx,dy)];
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
	::CGContextTranslateCTM (m_CGContext, -0.5, -0.5);
	[image drawInRect:NSMakeRect(TDUX(xDest), _tduY(yDest), pCocoaImage->getDisplayWidth(), iImageHeight)
	           fromRect:NSMakeRect(0, 0, size.width, size.height) operation:NSCompositeCopy fraction:1.0f];
//	[image compositeToPoint:NSMakePoint(xDest, yDest + iImageHeight) operation:NSCompositeCopy fraction:1.0f];
	::CGContextRestoreGState(m_CGContext);
}


void GR_CocoaGraphics::flush(void)
{
	[m_pWin setNeedsDisplay:YES];
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

static NSCursor *s_leftRightCursor = nil;
static NSCursor *s_upDownCursor = nil;

void GR_CocoaGraphics::setCursor(GR_Graphics::Cursor c)
{
	if (m_cursor == c)
		return;

	m_cursor = c;

	switch (c)
	{
	default:
		NSLog (@"Using unimplemented cursor");
		/*FALLTHRU*/
	case GR_CURSOR_DEFAULT:
		NSLog(@"Cursor default");
		[m_pWin setCursor:[NSCursor arrowCursor]];
		break;

	case GR_CURSOR_IBEAM:
		NSLog(@"Cursor IBeam");
		[m_pWin setCursor:[NSCursor IBeamCursor]];
		break;

	case GR_CURSOR_WAIT:
		// There is no wait cursor for Cocoa.  Or something.
		NSLog(@"Cursor wait");
		[m_pWin setCursor:[NSCursor arrowCursor]];
		break;

	case GR_CURSOR_LEFTRIGHT:
		if (s_leftRightCursor == nil) {
			s_leftRightCursor = [[NSCursor alloc] initWithImage:[NSImage imageNamed:@"leftright_cursor"] hotSpot:NSMakePoint(8,8)];
			UT_ASSERT(s_leftRightCursor);
		}
		[m_pWin setCursor:s_leftRightCursor];
		break;

	case GR_CURSOR_UPDOWN:
		if (s_upDownCursor == nil) {
			s_upDownCursor = [[NSCursor alloc] initWithImage:[NSImage imageNamed:@"updown_cursor"] hotSpot:NSMakePoint(8,8)];
			UT_ASSERT(s_upDownCursor);
		}
		[m_pWin setCursor:s_upDownCursor];
		break;

#if 0

	//I have changed the shape of the arrow so get a consistent
	//behaviour in the bidi build; I think the new arrow is better
	//for the purpose anyway

	case GR_CURSOR_RIGHTARROW:
		cursor_number = GDK_SB_RIGHT_ARROW; //GDK_ARROW;
		break;

	case GR_CURSOR_LEFTARROW:
		cursor_number = GDK_SB_LEFT_ARROW; //GDK_LEFT_PTR;
		break;

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

	case GR_CURSOR_EXCHANGE:
		cursor_number = GDK_EXCHANGE;
		break;

	case GR_CURSOR_GRAB:
		cursor_number = GDK_HAND1;
		break;
#endif
	}
	[[m_pWin window] invalidateCursorRectsForView:m_pWin];
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
	m_3dColors[CLR3D_BevelUp] = [[NSColor whiteColor] copy];
	m_3dColors[CLR3D_BevelDown] = [[NSColor darkGrayColor] copy];
	m_3dColors[CLR3D_Highlight] = [[NSColor controlHighlightColor] copy];
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
	return (*m_updateCallback) (aRect, this, m_updateCBparam);
}

void GR_CocoaGraphics::_updateRect(NSView * v, NSRect aRect)
{
		if ([v inLiveResize]) {
			xxx_UT_DEBUGMSG (("Is resizing\n"));

			::CGContextSaveGState(m_CGContext);
			::CGContextClipToRect (m_CGContext, ::CGRectMake (aRect.origin.x, aRect.origin.y, 
			                                                  aRect.size.width, aRect.size.height));
//			if (!_callUpdateCallback (&aRect)) {

//			}
			::CGContextRestoreGState(m_CGContext);
		}
/*		else {
			_callUpdateCallback(&aRect);
		}
		else {
			m_CGContext = CG_CONTEXT__;
		}*/
		UT_DEBUGMSG (("- (void)drawRect:(NSRect)aRect: calling callback !\n"));
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
	*cacheRect = NSMakeRect(TDUX(rect.left), _tduY(rect.top), 
						  _tduR(rect.width), _tduR(rect.height));
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
	m_cacheArray.setNthItem(iIndx, cache, (void **)&oldObj);
	[oldObj release];
	NSRect *old;
	m_cacheRectArray.setNthItem(iIndx, cacheRect, (void **)&old);
	if (old) {
		delete old;
	}
}


void GR_CocoaGraphics::restoreRectangle(UT_uint32 iIndx)
{
	NSRect* cacheRect = static_cast<NSRect*>(m_cacheRectArray.getNthItem(iIndx));
	NSImage* cache = static_cast<NSImage*>(m_cacheArray.getNthItem(iIndx));
	NSPoint pt = cacheRect->origin;
	pt.x -= 1;		/* I don't know why this offset, but it is nicer no more pixeldirt */
	pt.y += cacheRect->size.height;
	{
		LOCK_CONTEXT__;
		[cache compositeToPoint:pt operation:NSCompositeCopy];
	}
}

UT_uint32 GR_CocoaGraphics::getDeviceResolution(void) const
{
	return _getResolution ();
}


void GR_CocoaGraphics::_resetContext(CGContextRef context)
{
	// TODO check that we properly reset parameters according to what has been saved.
	::CGContextSetLineWidth (context, m_fLineWidth);
	_setCapStyle(m_capStyle);
	_setJoinStyle(m_joinStyle);
	_setLineStyle(m_lineStyle);
	::CGContextTranslateCTM (context, 0.5, 0.5);
	::CGContextSetShouldAntialias (context, false);
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


