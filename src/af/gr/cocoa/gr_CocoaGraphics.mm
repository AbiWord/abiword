/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003 Hubert Figuiere
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

#define USE_OFFSCREEN 1
//#undef USE_OFFSCREEN

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
//#include "ut_dialogHelper.h"
#include "ut_wctomb.h"
#include "xap_EncodingManager.h"
#include "ut_OverstrikingChars.h"

#import <Cocoa/Cocoa.h>
#import <ApplicationServices/ApplicationServices.h>

#define FloatToFixed(a) ((Fixed)((float) (a) * fixed1))

#define DISABLE_VERBOSE 1

#ifdef DISABLE_VERBOSE
# if DISABLE_VERBOSE
#  undef UT_DEBUGMSG
#  define UT_DEBUGMSG(x)
# endif
#endif

#ifdef USE_OFFSCREEN
#define LOCK_CONTEXT__ 	StNSImageLocker locker(m_pWin, m_offscreen); \
							GR_CaretDisabler caretDisabler(getCaret());
#else
#define LOCK_CONTEXT__	StNSViewLocker locker(m_pWin); \
								GR_CaretDisabler caretDisabler(getCaret());
#endif

#define CG_CONTEXT__ (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort]


@interface NSFont(PrivateAPI)
- (NSGlyph)_defaultGlyphForChar:(unichar)theChar;
- (ATSUFontID)_atsFontID;
@end

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

	void * operator new (size_t size);	// private so that we never call new for that class. Never defined.
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


XAP_CocoaFont *			GR_CocoaGraphics::s_pFontGUI = NULL;
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
#ifdef USE_OFFSCREEN
	m_offscreen (nil),
#endif
	m_cacheArray (10),
	m_cacheRectArray (10),
	m_currentColor (nil),
	m_pFont (NULL),
	m_screenResolution(0),
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
	m_iLineWidth = 0;
	s_iInstanceCount++;
	init3dColors ();
	
	/* resolution does not change thru the life of the object */
	NSScreen* mainScreen = [NSScreen mainScreen];
	NSDictionary* desc = [mainScreen deviceDescription];
	UT_ASSERT(desc);
	NSValue* value = [desc objectForKey:NSDeviceResolution];
	UT_ASSERT(value);
	m_screenResolution = lrintf([value sizeValue].height);

	m_xorCache = [[NSImage alloc] initWithSize:NSMakeSize(0,0)] ;
	[m_xorCache setFlipped:YES];

#ifdef USE_OFFSCREEN
	m_offscreen = [[NSImage alloc] initWithSize:viewBounds.size];
	[m_offscreen setFlipped:YES];
#endif

	LOCK_CONTEXT__;
	m_currentColor = [[NSColor blackColor] copy];
	_resetContext();

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
	UT_VECTOR_RELEASE(m_cacheArray);
	UT_VECTOR_PURGEALL(NSRect*, m_cacheRectArray);
	[m_xorCache release];
	[m_fontProps release];
	[m_currentColor release];
#ifdef USE_OFFSCREEN
	[m_offscreen release];
#endif

	s_iInstanceCount--;
	if(!s_iInstanceCount) {
		DELETEP(s_pFontGUI);
	}
	for (int i = 0; i < COUNT_3D_COLORS; i++) {
		[m_3dColors[i] release];
	}
	
	
	[m_fontMetricsTextStorage release];
	[m_fontMetricsLayoutManager release];
	[m_fontMetricsTextContainer release];
}

bool GR_CocoaGraphics::queryProperties(GR_Graphics::Properties gp) const
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


void GR_CocoaGraphics::setLineProperties ( double    inWidthPixels, 
				      JoinStyle inJoinStyle,
				      CapStyle  inCapStyle,
				      LineStyle inLineStyle )
{
	LOCK_CONTEXT__;
	
	[NSBezierPath setDefaultLineWidth:tduD(inWidthPixels)];
	
	switch (inJoinStyle) {
	case JOIN_MITER:
		[NSBezierPath setDefaultLineJoinStyle:NSMiterLineJoinStyle];
		break;
	case JOIN_ROUND:
		[NSBezierPath setDefaultLineJoinStyle:NSRoundLineJoinStyle];
		break;
	case JOIN_BEVEL:
		[NSBezierPath setDefaultLineJoinStyle:NSBevelLineJoinStyle];
		break;
	default:
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	}
	switch (inCapStyle) {
	case CAP_BUTT:
		[NSBezierPath setDefaultLineCapStyle:NSButtLineCapStyle];
		break;
	case CAP_ROUND:
		[NSBezierPath setDefaultLineCapStyle:NSRoundLineCapStyle];
		break;
	case CAP_PROJECTING:
		[NSBezierPath setDefaultLineCapStyle:NSSquareLineCapStyle];
		break;
	default:
		UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	}
	switch (inLineStyle) {
	case LINE_SOLID:
	case LINE_ON_OFF_DASH:
	case LINE_DOUBLE_DASH:
	case LINE_DOTTED:
		UT_DEBUGMSG (("TODO: line dashes\n"));
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
	UT_sint32 yoff = tdu(yoffLU);
	UT_sint32 xoff = xoffLU;	// layout Unit !
	
	NSString * string = nil;

    if (!m_fontMetricsTextStorage) {
		_initMetricsLayouts();
    }

	if (!pCharWidths) {
		NSPoint point = NSMakePoint (tdu(xoff), yoff);
		unichar * uniString = (unichar*)malloc((iLength + 1) * sizeof (unichar));
		for (int i = 0; i < iLength; i++) {
			uniString[i] = pChars[i + iCharOffset];
		}
		string =  [[NSString alloc] initWithCharacters:uniString length:iLength];
		NSAttributedString* attributedString = [[NSAttributedString alloc] initWithString:string attributes:m_fontProps];
		[m_fontMetricsTextStorage setAttributedString:attributedString];
		[string release];
		[attributedString release];

		LOCK_CONTEXT__;
		[m_fontMetricsLayoutManager drawGlyphsForGlyphRange:NSMakeRange(0, iLength) atPoint:point];
	}
	else {
		LOCK_CONTEXT__;
		int i;
		bool bSetAttributes = false;
		for (i = 0; i < iLength; i++) {
			unichar c2 = *(pChars + iCharOffset + i);
			string =  [[NSString alloc] initWithCharacters:&c2 length:1];
			if (bSetAttributes) {
				[m_fontMetricsTextStorage replaceCharactersInRange:NSMakeRange(0, 1) withString:string];
			}
			else {
				NSAttributedString* attributedString = [[NSAttributedString alloc] initWithString:string attributes:m_fontProps];
				[m_fontMetricsTextStorage setAttributedString:attributedString];
				bSetAttributes = true;
				[attributedString release];
			}
			[string release];
	
			NSPoint point;
			point.x = tdu(xoff);
			point.y = yoff;
	
			[m_fontMetricsLayoutManager drawGlyphsForGlyphRange:NSMakeRange(0, 1) atPoint:point];
			if (i < iLength - 1) {
				xoff +=	pCharWidths[i];
			}
		}
	}
	
//	flush();
}

void GR_CocoaGraphics::setFont(GR_Font * pFont)
{
	XAP_CocoaFont * pUFont = static_cast<XAP_CocoaFont *> (pFont);
	UT_ASSERT (pUFont);
	m_pFont = pUFont;
	[m_fontProps setObject:pUFont->getNSFont() forKey:NSFontAttributeName];
}

UT_uint32 GR_CocoaGraphics::getFontHeight(GR_Font * fnt)
{
	UT_ASSERT(fnt);
	return tlu((UT_uint32)static_cast<XAP_CocoaFont*>(fnt)->getHeight());
}

UT_uint32 GR_CocoaGraphics::getFontHeight()
{
	return tlu((UT_uint32)m_pFont->getHeight());
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
UT_uint32 GR_CocoaGraphics::_measureUnRemappedCharCached(const UT_UCSChar c)
{
	UT_uint32 width;
	width = m_pFont->getCharWidthFromCache(c);
	width *= ((float)m_pFont->getSize() / (float)GR_CharWidthsCache::CACHE_FONT_SIZE);
	return width;
}

/*!
	Measure unremapped char
	
	\return width in Layout Unit
 */
UT_uint32 GR_CocoaGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	// measureString() could be defined in terms of measureUnRemappedChar()
	// but its not (for presumed performance reasons).  Also, a difference
	// is that measureString() uses remapping to get past zero-width
	// character cells.

	if(c == 0x200B || c == 0xFEFF) { // 0-with spaces
		return 0;
	}

	return tlu(_measureUnRemappedCharCached(c));
}


void GR_CocoaGraphics::getCoverage(UT_Vector& coverage)
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
	NSColor *c = [NSColor colorWithDeviceRed:r green:g blue:b alpha:1.0 /*(clr.m_bIsTransparent ? 0.0 : 1.0)*/];
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
	clr.m_bIsTransparent = (a == 0.0f ? true : false);
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
	LOCK_CONTEXT__;
	[m_currentColor set];
}

GR_Font * GR_CocoaGraphics::getGUIFont(void)
{
	if (!s_pFontGUI)
	{
		// get the font resource		
		UT_DEBUGMSG(("GR_CocoaGraphics::getGUIFont: getting default font\n"));
		// bury it in a new font handle
		s_pFontGUI = new XAP_CocoaFont([NSFont labelFontOfSize:[NSFont labelFontSize]]); // Hardcoded GUI font size
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
		// Oops!  We don't have that font here.
		// first try "Times New Roman", which should be sensible, and should
		// be there unless the user fidled with the installation
		NSLog (@"Unable to fint font \"%s\".", pszFontFamily);
		nsfont = [[NSFontManager sharedFontManager] fontWithFamily:@"Times New Roman" 
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
	return tlu((UT_uint32)static_cast<XAP_CocoaFont*>(fnt)->getAscent());
}

// returns in LU
UT_uint32 GR_CocoaGraphics::getFontAscent()
{
	return tlu((UT_uint32)m_pFont->getAscent());
}

// returns in LU
UT_uint32 GR_CocoaGraphics::getFontDescent(GR_Font * fnt)
{
	UT_ASSERT(fnt);
	return tlu((UT_uint32)static_cast<XAP_CocoaFont*>(fnt)->getDescent());
}

UT_uint32 GR_CocoaGraphics::getFontDescent()
{
	return tlu((UT_uint32)m_pFont->getDescent());
}

void GR_CocoaGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
							   UT_sint32 x2, UT_sint32 y2)
{
	LOCK_CONTEXT__;
	UT_DEBUGMSG (("GR_CocoaGraphics::drawLine(%ld, %ld, %ld, %ld) width=%f\n", x1, y1, x2, y2,
	              [NSBezierPath defaultLineWidth]));
	::CGContextTranslateCTM (m_CGContext, 0.5, 0.5);

	::CGContextBeginPath(m_CGContext);
	// TODO set the line width according to m_iLineWidth
	::CGContextMoveToPoint (m_CGContext, tdu(x1), tdu(y1));
	::CGContextAddLineToPoint (m_CGContext, tdu(x2), tdu(y2));
	::CGContextStrokePath (m_CGContext);
}

void GR_CocoaGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	LOCK_CONTEXT__;
	UT_DEBUGMSG (("GR_CocoaGraphics::setLineWidth(%ld) was %f\n", iLineWidth, [NSBezierPath defaultLineWidth]));
	m_iLineWidth = tdu(iLineWidth);
	::CGContextSetLineWidth (m_CGContext, m_iLineWidth);
}

void GR_CocoaGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
			    UT_sint32 y2)
{
	// TODO use XOR mode NSCompositeXOR
//	LOCK_CONTEXT__;
	float x = UT_MIN(x1,x2);		// still layout unit
	float y = UT_MIN(y1,y2);		// still layout unit
	NSRect newBounds = NSMakeRect (tduD(x), tduD(y), tduD(UT_MAX(UT_MAX(x1,x2) - x,1.0)), 
													tduD(UT_MAX(UT_MAX(y1,y2) - y,1.0)));
	[m_xorCache setSize:newBounds.size];
	{
		StNSImageLocker locker(m_pWin, m_xorCache);
		CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];

		::CGContextTranslateCTM (context, 0.5, 0.5);
		::CGContextBeginPath(context);
		::CGContextSetLineWidth (context, m_iLineWidth);
		/* since we are in the image coordinate space, we should offset it with the origin */
		::CGContextMoveToPoint (context, tduD(x1 - x), tduD(y1 - y));
		::CGContextAddLineToPoint (context, tduD(x2 - x), tduD(y2 - y));
		::CGContextStrokePath (context);
	}
	// Should make an NSImage and XOR it onto the real image.
	LOCK_CONTEXT__;
	[m_xorCache compositeToPoint:newBounds.origin operation:NSCompositeXOR];
}

void GR_CocoaGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::polyLine() width=%f\n", [NSBezierPath defaultLineWidth]));
	
	LOCK_CONTEXT__;
	::CGContextTranslateCTM (m_CGContext, 0.5, 0.5);
	::CGContextBeginPath(m_CGContext);
	
	for (UT_uint32 i = 0; i < nPoints; i++)
	{
		if (i == 0) {
			::CGContextMoveToPoint(m_CGContext, tdu(pts[i].x), tdu(pts[i].y));
		}
		else {
			::CGContextAddLineToPoint(m_CGContext, tdu(pts[i].x), tdu(pts[i].y));
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

	NSHighlightRect (NSMakeRect (tdu(pRect->left), tdu(pRect->top), tdu(pRect->width), tdu(pRect->height)));
}

void GR_CocoaGraphics::setClipRect(const UT_Rect* pRect)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::setClipRect()\n"));
	m_pRect = pRect;

	LOCK_CONTEXT__;
	/* discard the clipping */
	/* currently the only way is to restore the graphic state */
	::CGContextRestoreGState(m_CGContext);
	::CGContextSaveGState(m_CGContext);
	/* restore the graphics settings */
	[m_currentColor set];
	::CGContextSetLineWidth (m_CGContext, m_iLineWidth);

	if (pRect) {
		UT_DEBUGMSG (("ClipRect set\n"));
		::CGContextClipToRect (m_CGContext, 
				::CGRectMake (tdu(pRect->left), tdu(pRect->top), tdu(pRect->width), tdu(pRect->height)));
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
	[NSGraphicsContext saveGraphicsState];
	[c set];
	NSRectFill (NSMakeRect (tdu(x), tdu(y), tdu(w), tdu(h)));
	[NSGraphicsContext restoreGraphicsState];
}

void GR_CocoaGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	UT_DEBUGMSG(("GR_CocoaGraphics::fillRect(GR_Color3D %d, %ld, %ld, %ld, %ld)\n", c, x, y, w, h));

	LOCK_CONTEXT__;
	[NSGraphicsContext saveGraphicsState];
	[m_3dColors[c] set];
	NSRectFill (NSMakeRect (tdu(x), tdu(y), tdu(w), tdu(h)));
	[NSGraphicsContext restoreGraphicsState];
}

void GR_CocoaGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_DEBUGMSG(("GR_CocoaGraphics::fillRect(GR_Color3D, UT_Rect &)\n"));
	UT_ASSERT(c < COUNT_3D_COLORS);
	fillRect(c,r.left,r.top,r.width,r.height);
}


void GR_CocoaGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
#ifdef USE_OFFSCREEN
	NSImage* img = _getOffscreen();
	LOCK_CONTEXT__;
	[img compositeToPoint:NSMakePoint(-tduD(dx),-tduD(dy)) operation:NSCompositeCopy];
#else
	[m_pWin scrollRect:[m_pWin bounds] by:NSMakeSize(tduD(dx),tduD(dy))];
	[m_pWin setNeedsDisplay:YES];
#endif
}

void GR_CocoaGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  UT_sint32 width, UT_sint32 height)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
#ifdef USE_OFFSCREEN
	NSImage* img = _getOffscreen();
	LOCK_CONTEXT__;
	[img drawAtPoint:NSMakePoint (tduD(x_dest),tduD(y_dest)) 
			fromRect:NSMakeRect(tduD(x_src), tduD(y_src), tduD(width), tduD(height))
			operation:NSCompositeCopy fraction:1.0f];
#else
	float dx, dy;
	dx = tduD(x_src - x_dest);
	dy = tduD(y_src - y_dest);
	
	[m_pWin scrollRect:NSMakeRect(tduD(x_src), tduD(y_src), tduD(width), tduD(height)) 
				by:NSMakeSize(dx,dy)];
	[m_pWin setNeedsDisplay:YES];
#endif
}

void GR_CocoaGraphics::clearArea(UT_sint32 x, UT_sint32 y,
							 UT_sint32 width, UT_sint32 height)
{
  	UT_DEBUGMSG(("ClearArea: %d %d %d %d\n", x, y, width, height));
	if (width > 0)
	{
		LOCK_CONTEXT__;
		NSEraseRect (NSMakeRect(tdu(x), tdu(y), tdu(width), tdu(height)));
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

	pImg->convertFromBuffer(pBB, tdu(iDisplayWidth), tdu(iDisplayHeight));
   	return pImg;
}

void GR_CocoaGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);

   	if (pImg->getType() != GR_Image::GRT_Raster) {
	   	pImg->render(this, tdu(xDest), tdu(yDest));
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
	[image drawInRect:NSMakeRect(tdu(xDest), tdu(yDest), pCocoaImage->getDisplayWidth(), iImageHeight)
	           fromRect:NSMakeRect(0, 0, size.width, size.height) operation:NSCompositeCopy fraction:1.0f];
//	[image compositeToPoint:NSMakePoint(xDest, yDest + iImageHeight) operation:NSCompositeCopy fraction:1.0f];
}


void GR_CocoaGraphics::flush(void)
{
	[[NSGraphicsContext currentContext] flushGraphics];
	[[m_pWin window] flushWindowIfNeeded];
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
	if (m_cursor == c)
		return;

	m_cursor = c;

	NSCursor * cursor;

	switch (c)
	{
	default:
		NSLog (@"Using unimplemented cursor");
		/*FALLTHRU*/
	case GR_CURSOR_DEFAULT:
		cursor = [NSCursor arrowCursor];
		break;

	case GR_CURSOR_IBEAM:
		cursor = [NSCursor IBeamCursor];
		break;

	case GR_CURSOR_WAIT:
		// There is no wait cursor for Cocoa.  Or something.
		cursor = [NSCursor arrowCursor];
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
#endif
	}
	[m_pWin addCursorRect:[m_pWin bounds] cursor:cursor];
	[cursor setOnMouseEntered:YES];
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
	m_3dColors[CLR3D_Background] = [[NSColor windowBackgroundColor] copy];
//	m_3dColors[CLR3D_Background] = [[NSColor controlColor] copy];
//	m_3dColors[CLR3D_Background] = [[NSColor lightGrayColor] copy];
	m_3dColors[CLR3D_BevelUp] = [[NSColor lightGrayColor] copy];
	m_3dColors[CLR3D_BevelDown] = [[NSColor grayColor] copy];
	m_3dColors[CLR3D_Highlight] = [[NSColor controlHighlightColor] copy];
}

void GR_CocoaGraphics::polygon(UT_RGBColor& clr,UT_Point *pts,UT_uint32 nPoints)
{
	NSColor *c = _utRGBColorToNSColor (clr);
	LOCK_CONTEXT__;
	::CGContextTranslateCTM (m_CGContext, 0.5, 0.5);
	::CGContextBeginPath(m_CGContext);
	for (UT_uint32 i = 0; i < nPoints; i++)
	{
		if (i == 0) {
			::CGContextMoveToPoint(m_CGContext, tdu(pts[i].x), tdu(pts[i].y));
		}
		else {
			::CGContextAddLineToPoint(m_CGContext, tdu(pts[i].x), tdu(pts[i].y));
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
#if 0
	[NSGraphicsContext saveGraphicsState];
	static float r=1.0, g=1.0, b=0.0;
	if (g == 0.0) { r = 1.0; g = 1.0; b = 0.0; }
	else if (r == 0.0) { r = 1.0; g = 0.0; b = 1.0; }
	else if (b == 0.0) { r = 0.0; g = 1.0; b = 1.0; }
	NSColor *c = [NSColor colorWithDeviceRed:r green:g blue:b alpha:1.0];
	[c set];
	NSRectFill ([v bounds]);
	[NSGraphicsContext restoreGraphicsState];
#endif	
# ifdef USE_OFFSCREEN

		NSImage * img = _getOffscreen ();
		// TODO: only draw what is needed.
# endif
		if ([v inLiveResize]) {
			UT_DEBUGMSG (("Is resizing\n"));
# ifdef USE_OFFSCREEN
			NSRect myBounds = [v bounds];
			[img setSize:myBounds.size];
			
			// take care of erasing after resizing.
			{
				StNSImageLocker locker (m_pWin, img);
				// the NSImage has been resized. So the CGContextRef has changed.
				// context is locked. it is safe to reset the CG context.
				_resetContext();
				::CGContextSaveGState(m_CGContext);
				[[NSColor whiteColor] set];
				::CGContextFillRect (m_CGContext, ::CGRectMake(myBounds.origin.x, myBounds.origin.y, 
				                                                myBounds.size.width, myBounds.size.height));
				::CGContextRestoreGState(m_CGContext);
			}
# endif
			::CGContextSaveGState(m_CGContext);
			::CGContextClipToRect (m_CGContext, ::CGRectMake (aRect.origin.x, aRect.origin.y, 
			                                                  aRect.size.width, aRect.size.height));
			if (!_callUpdateCallback (&aRect)) {

			}
			::CGContextRestoreGState(m_CGContext);
		}
# ifdef USE_OFFSCREEN
		[img drawAtPoint:aRect.origin fromRect:aRect operation:NSCompositeCopy fraction:1.0f];
# endif
		UT_DEBUGMSG (("- (void)drawRect:(NSRect)aRect: calling callback !\n"));
}

bool GR_CocoaGraphics::_isFlipped()
{
# ifdef USE_OFFSCREEN
	return NO;
# else
	return YES;
#endif
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


void GR_CocoaGraphics::saveRectangle(UT_Rect & rect,  UT_uint32 iIndx)
{
	NSRect* cacheRect = new NSRect;
	*cacheRect = NSMakeRect(tdu(rect.left), tdu(rect.top), 
						  tdu(rect.width), tdu(rect.height));
	NSImage* cache = _makeNewCacheImage();
	[cache setSize:cacheRect->size];
	{
		StNSImageLocker locker(m_pWin, cache);
		NSRect r = NSMakeRect (0.0, 0.0, tdu(rect.width), tdu(rect.height));
		NSEraseRect(r);
#ifdef USE_OFFSCREEN
		[m_offscreen compositeToPoint:NSMakePoint(0.0, 0.0) fromRect:*cacheRect operation:NSCompositeCopy];
#else
		UT_DEBUGMSG(("UT_NOT_IMPLEMENTED"));
#endif
	}
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
	pt.y += cacheRect->size.height;
	LOCK_CONTEXT__;
	[cache compositeToPoint:pt operation:NSCompositeCopy];
	[[m_pWin window] flushWindowIfNeeded];
}

UT_uint32 GR_CocoaGraphics::getDeviceResolution(void) const
{
	return _getResolution ();
}


void GR_CocoaGraphics::_resetContext()
{
	// TODO check that we properly reset parameters according to what has been saved.
	m_CGContext = CG_CONTEXT__;
	::CGContextSetLineWidth (m_CGContext, m_iLineWidth);
	::CGContextSetLineCap (m_CGContext, kCGLineCapButt);
	::CGContextSetLineJoin (m_CGContext, kCGLineJoinMiter);
	::CGContextTranslateCTM (m_CGContext, 0.5, 0.5);
	::CGContextSetShouldAntialias (m_CGContext, false);
 	[m_currentColor set];
	/* save initial graphics state that has no clipping */
	::CGContextSaveGState(m_CGContext);
}