/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>
#import <ApplicationServices/ApplicationServices.h>

#define DISABLE_VERBOSE 1
#define USE_OFFSCREEN 1
//#undef USE_OFFSCREEN

#ifdef DISABLE_VERBOSE
# if DISABLE_VERBOSE
#  undef UT_DEBUGMSG
#  define UT_DEBUGMSG(x)
# endif
#endif

#ifdef USE_OFFSCREEN
#define LOCK_CONTEXT__ 	StNSImageLocker locker(m_pWin, m_offscreen)
#else
#define LOCK_CONTEXT__	StNSViewLocker locker(m_pWin)
#endif

#define CG_CONTEXT__ (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort]

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
	NSImage *m_image; NSView * m_pView;

	void * operator new (size_t size);	// private so that we never call new for that class. Never defined.
};


XAP_CocoaFontHandle *	GR_CocoaGraphics::s_pFontGUI = NULL;
UT_uint32 				GR_CocoaGraphics::s_iInstanceCount = 0;

GR_CocoaGraphics::GR_CocoaGraphics(NSView * win, XAP_CocoaFontManager * fontManager, XAP_App * app)
	: m_updateCallback(NULL),
	m_updateCBparam (NULL),
	m_pFontManager (fontManager),
	m_offscreen (nil),
	m_currentColor (nil),
	m_pFont (NULL)
{
	m_pApp = app;
	UT_ASSERT (win);
	NSRect viewBounds = [win bounds];
//  	xxx_UT_DEBUGMSG (("frame is %f %f %f %f\n", theRect.origin.x, theRect.origin.y, theRect.size.width, theRect.size.height));
	if (![win isKindOfClass:[XAP_CocoaNSView class]]) {
		m_pWin = [[XAP_CocoaNSView alloc] initWithFrame:viewBounds];
		[win addSubview:m_pWin];
	}
	else {
		[win retain];
		m_pWin = win;
	}
	[m_pWin setXAPFrame:(XAP_CocoaFrame*)(app->getLastFocussedFrame())];
	[m_pWin setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[m_pWin setGraphics:this];
	m_iLineWidth = 0;
	s_iInstanceCount++;
	init3dColors ();

	m_offscreen = [[NSImage alloc] initWithSize:viewBounds.size];
	[m_offscreen setFlipped:YES];

	LOCK_CONTEXT__;
	m_CGContext = CG_CONTEXT__;
	::CGContextSetLineWidth (m_CGContext, m_iLineWidth);
	::CGContextSetLineCap (m_CGContext, kCGLineCapButt);
	::CGContextSetLineJoin (m_CGContext, kCGLineJoinMiter);
	::CGContextTranslateCTM (m_CGContext, 0.5, 0.5);
	m_currentColor = [[NSColor blackColor] copy];
 	[m_currentColor set];

	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
	m_cursor = GR_CURSOR_INVALID;
	setCursor(GR_CURSOR_DEFAULT);
	::CGContextSetShouldAntialias (m_CGContext, 0);
	/* save initial graphics state that has no clipping */
	::CGContextSaveGState(m_CGContext);
# ifndef USE_OFFSCREEN
	StNSImageLocker locker (m_offscreen);
# endif
	NSRect aRect = [m_pWin bounds];

	::CGContextSaveGState(m_CGContext);
	[[NSColor controlColor] set];
	::CGContextFillRect (m_CGContext, ::CGRectMake(aRect.origin.x, aRect.origin.y, 
	                                                aRect.size.width, aRect.size.height));
	::CGContextRestoreGState(m_CGContext);
}

GR_CocoaGraphics::~GR_CocoaGraphics()
{
	[m_pWin removeFromSuperview];

	s_iInstanceCount--;
	if(!s_iInstanceCount) {
		DELETEP(s_pFontGUI);
	}
	for (int i = 0; i < COUNT_3D_COLORS; i++) {
		[m_3dColors[i] release];
	}
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


void GR_CocoaGraphics::setLineProperties ( double    inWidthPixels, 
				      JoinStyle inJoinStyle,
				      CapStyle  inCapStyle,
				      LineStyle inLineStyle )
{
	LOCK_CONTEXT__;
	
	[NSBezierPath setDefaultLineWidth:inWidthPixels];
	
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


// HACK: I need more speed
void GR_CocoaGraphics::drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::drawChar()\n"));
	UT_UCSChar Wide_char = remapGlyph(Char, false);
	if(Wide_char == 0x200B || Wide_char == 0xFEFF) //zero width spaces
		return;

	NSFont *font = m_pFont->getNSFont();

	NSString * string = [[NSString alloc] initWithData:
							[NSData dataWithBytes:&Wide_char length:sizeof(Wide_char)]
							encoding:NSUnicodeStringEncoding];

	NSPoint point;
	point.x = xoff;
	point.y = yoff;

	LOCK_CONTEXT__;
	// TODO: set attributes
	[string drawAtPoint:point withAttributes:[NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName]];
	[string release];
}

void GR_CocoaGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
							 int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::drawChars()\n"));
	if (!m_pFontManager)
		return;
	UT_ASSERT(m_pFont);
	WCTOMB_DECLS;
	NSFont *font = m_pFont->getNSFont();
	UT_sint32 x;

	// to be able to handle overstriking characters, we have to remember the width
	// of the previous character printed
	// NB: overstriking characters are only supported under UTF-8, since on 8-bit locales
	// these are typically handled by combination glyphs

	static UT_sint32 prevWidth = 0;
	UT_sint32 curX;
	UT_sint32 curWidth;

	const UT_UCSChar *pC;

	// Lock the NSView
	LOCK_CONTEXT__;
	NSString * string = nil;

  	for(pC=pChars+iCharOffset, x=xoff; pC<pChars+iCharOffset+iLength; ++pC)
	{
		UT_UCSChar actual = remapGlyph(*pC,false);
		if(actual == 0x200B || actual == 0xFEFF) //zero width spaces
			continue;
		// TODO try to bufferize the string allocation
		string = [[NSString alloc] initWithData:[NSData dataWithBytes:&actual length:sizeof(UT_UCSChar)]
							encoding:NSUnicodeStringEncoding];

		/*	if the locale is unicode (i.e., utf-8) then we do not want
			to convert the UCS string to anything,
			gdk_draw_text can draw 16-bit string, if the font is
			a matrix; however the string is interpreted as big-endian
		*/
		//unicode font
		//UT_DEBUGMSG(("CocoaGraphics::drawChars: utf-8\n"));

		switch(isOverstrikingChar(*pC))
		{
		case UT_NOT_OVERSTRIKING:
			curWidth = (UT_sint32)[font widthOfString:string];
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

		[string drawAtPoint:point withAttributes:[NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName]];

		//gdk_draw_text(m_pWin,font,m_pGC,curX,yoff+font->ascent,(gchar*)&beucs,2);
		x+=curWidth;
		prevWidth = curWidth;
/*
		This was in the non-bidi branch, and as I am not sure the bidi brach is complete, I left it here
		for the time being

		NSPoint point;
		point.x = x;
		point.y = yoff;

		// TODO: set attributes
		[string drawAtPoint:point withAttributes:nil];
		//gdk_draw_text(m_pWin,font,m_pGC,x,yoff+font->ascent,(gchar*)&beucs,2);
		x += [font widthOfString:string];
		//x+=gdk_text_width(font, (gchar*)&beucs, 2);
*/
		[string release];
	}

	flush();
}

void GR_CocoaGraphics::setFont(GR_Font * pFont)
{
	XAP_CocoaFontHandle * pUFont = static_cast<XAP_CocoaFontHandle *> (pFont);
	UT_ASSERT (pUFont);
	m_pFont = pUFont;
}

UT_uint32 GR_CocoaGraphics::getFontHeight(GR_Font * fnt)
{
	UT_ASSERT(fnt);

	XAP_CocoaFontHandle * hndl = static_cast<XAP_CocoaFontHandle *>(fnt);

	NSFont* pFont = hndl->getNSFont();
//	printf ("line height %f\n", [pFont defaultLineHeightForFont]);
	return (UT_uint32)[pFont defaultLineHeightForFont];
}

UT_uint32 GR_CocoaGraphics::getFontHeight()
{
	return getFontHeight(m_pFont);
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
	[string release];

	return (UT_uint32)w;
}


UT_uint32 GR_CocoaGraphics::_getResolution(void) const
{
	return 75;
}

/*
	Allocate the color. Must be released by caller.
 */
NSColor	*GR_CocoaGraphics::_utRGBColorToNSColor (const UT_RGBColor& clr)
{
	float r,g,b;
	r = (float)clr.m_red / 255.0f;
	g = (float)clr.m_grn / 255.0f;
	b = (float)clr.m_blu / 255.0f;
	UT_DEBUGMSG (("converting color r=%f, g=%f, b=%f from %d, %d, %d\n", r, g, b, clr.m_red, clr.m_grn, clr.m_blu));
	NSColor *c = [[NSColor colorWithDeviceRed:r green:g blue:b alpha:1.0] retain];	// is autoreleased, so retain
	return c;
}


void GR_CocoaGraphics::setColor(const UT_RGBColor& clr)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::setColor(const UT_RGBColor&): setting color %d, %d, %d\n", clr.m_red, clr.m_grn, clr.m_blu));
	NSColor *c = _utRGBColorToNSColor (clr);
	_setColor(c);
	[c release];
}

/* c will be copied */
void GR_CocoaGraphics::_setColor(NSColor * c)
{
	UT_DEBUGMSG (("GR_CocoaGraphics::_setColor(NSColor *): setting NSColor\n"));
	[m_currentColor release];
	m_currentColor = [c copy];
	LOCK_CONTEXT__;
	[m_currentColor set];
}

GR_Font * GR_CocoaGraphics::getGUIFont(void)
{
	if (!m_pFontManager)
		return NULL;

	if (!s_pFontGUI)
	{
		// get the font resource		
		UT_DEBUGMSG(("GR_CocoaGraphics::getGUIFont: getting default font\n"));
		NSFont * myFont = [NSFont labelFontOfSize:[NSFont labelFontSize]];
		NSString * myFontName = [myFont familyName];
		char * name = (char *)malloc ([myFontName cStringLength] + 1);
		[myFontName getCString:name];
		UT_DEBUGMSG(("GUI font is %s\n", name));
		// below we only set style to style last, but we should probably do otherwise
		// -- Hub May 28th 2002.
		XAP_CocoaFont * font = (XAP_CocoaFont *)m_pFontManager->getFont (name, XAP_CocoaFont::STYLE_LAST);
		//(XAP_CocoaFont *) m_pFontManager->getDefaultFont();
		UT_ASSERT(font);

		// bury it in a new font handle
		s_pFontGUI = new XAP_CocoaFontHandle(font, [NSFont labelFontSize]); // Hardcoded GUI font size
		UT_ASSERT(s_pFontGUI);
		DELETEP (font);			// s_pFontGUI has duplicated the font object. Free it.
		FREEP (name);
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
	//
	// current forget about style.
	// FIXME
	// -- Hub May 28 2002.
	const XAP_CocoaFont * cocoafont = m_pFontManager->getFont(pszFontFamily, XAP_CocoaFont::STYLE_LAST);//s);
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
	return (UT_uint32)[pFont ascender];
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
	return (UT_uint32)[pFont descender];
}

UT_uint32 GR_CocoaGraphics::getFontDescent()
{
	return getFontDescent(m_pFont);
}

void GR_CocoaGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
							   UT_sint32 x2, UT_sint32 y2)
{
	CGContextRef context;

	LOCK_CONTEXT__;
	UT_DEBUGMSG (("GR_CocoaGraphics::drawLine(%ld, %ld, %ld, %ld) width=%f\n", x1, y1, x2, y2,
	              [NSBezierPath defaultLineWidth]));
	::CGContextTranslateCTM (m_CGContext, 0.5, 0.5);

	::CGContextBeginPath(m_CGContext);
	// TODO set the line width according to m_iLineWidth
	::CGContextMoveToPoint (m_CGContext, x1, y1);
	::CGContextAddLineToPoint (m_CGContext, x2, y2);
	::CGContextStrokePath (m_CGContext);
}

void GR_CocoaGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	LOCK_CONTEXT__;
	UT_DEBUGMSG (("GR_CocoaGraphics::setLineWidth(%ld) was %f\n", iLineWidth, [NSBezierPath defaultLineWidth]));
	m_iLineWidth = iLineWidth;
	::CGContextSetLineWidth (m_CGContext, m_iLineWidth);
}

void GR_CocoaGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
			    UT_sint32 y2)
{
	// TODO use XOR mode NSCompositeXOR
	LOCK_CONTEXT__;

	::CGContextTranslateCTM (m_CGContext, 0.5, 0.5);
	::CGContextBeginPath(m_CGContext);
	// TODO set the line width according to m_iLineWidth
	::CGContextMoveToPoint (m_CGContext, x1, y1);
	::CGContextAddLineToPoint (m_CGContext, x2, y2);
	::CGContextStrokePath (m_CGContext);
	// Should make an NSImage and XOR it onto the real image.
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
			::CGContextMoveToPoint(m_CGContext, pts[i].x, pts[i].y);
		}
		else {
			::CGContextAddLineToPoint(m_CGContext, pts[i].x, pts[i].y);
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

	NSHighlightRect (NSMakeRect (pRect->left, pRect->top, pRect->width, pRect->height));
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
		::CGContextClipToRect (m_CGContext, ::CGRectMake (pRect->left, pRect->top, pRect->width, pRect->height));
	}
	else {
		UT_DEBUGMSG (("ClipRect reset!!\n"));
	}
}

void GR_CocoaGraphics::fillRect(const UT_RGBColor& c, UT_Rect &r)
{
	UT_DEBUGMSG(("GR_CocoaGraphics::fillRect(UT_RGBColor&, UT_Rect &)\n"));
	fillRect(c,r.left,r.top,r.width,r.height);
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
	NSRectFill (NSMakeRect (x, y, w, h));
	[NSGraphicsContext restoreGraphicsState];

	[c release];
}

void GR_CocoaGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	UT_DEBUGMSG(("GR_CocoaGraphics::fillRect(GR_Color3D %d, %ld, %ld, %ld, %ld)\n", c, x, y, w, h));

	LOCK_CONTEXT__;
	[NSGraphicsContext saveGraphicsState];
	[m_3dColors[c] set];
	NSRectFill (NSMakeRect (x, y, w, h));
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
  	UT_DEBUGMSG(("ClearArea: %d %d %d %d\n", x, y, width, height));
	if (width > 0)
	{
		NSEraseRect (NSMakeRect(x, y, width, height));
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

   	NSImage * image = pCocoaImage->getNSImage();

	if (image == 0)
	{
		UT_DEBUGMSG(("Found no image data. This is probably SVG masquerading as a raster!\n"));
		return;
	}

   	UT_sint32 iImageWidth = pCocoaImage->getDisplayWidth();
   	UT_sint32 iImageHeight = pCocoaImage->getDisplayHeight();

	LOCK_CONTEXT__;
	[image compositeToPoint:NSMakePoint(xDest, yDest + iImageHeight) operation:NSCompositeCopy fraction:1.0f];
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
		UT_ASSERT(UT_NOT_IMPLEMENTED);
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
	m_3dColors[CLR3D_Foreground] = [[NSColor blackColor] retain];
	m_3dColors[CLR3D_Background] = [[NSColor controlColor] retain];
//	m_3dColors[CLR3D_Background] = [[NSColor grayColor] retain];
	m_3dColors[CLR3D_BevelUp] = [[NSColor lightGrayColor] retain];
	m_3dColors[CLR3D_BevelDown] = [[NSColor darkGrayColor] retain];
	m_3dColors[CLR3D_Highlight] = [[NSColor controlHighlightColor] retain];
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
			::CGContextMoveToPoint(m_CGContext, pts[i].x, pts[i].y);
		}
		else {
			::CGContextAddLineToPoint(m_CGContext, pts[i].x, pts[i].y);
		}
	}
	::CGContextSaveGState(m_CGContext);
	[c set];
	::CGContextFillPath(m_CGContext);
	::CGContextRestoreGState(m_CGContext);
	[c release];
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
				m_CGContext = CG_CONTEXT__;
				::CGContextSaveGState(m_CGContext);
				[[NSColor controlColor] set];
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

bool GR_CocoaGraphics::storeCachedImage(UT_Rect * pRect)
{
	NSRect r = NSMakeRect(pRect->left, pRect->top, 
						  pRect->width, pRect->height);
	LOCK_CONTEXT__;
	[[m_pWin window] cacheImageInRect:r];
	[[m_pWin window] flushWindowIfNeeded];
	return true; 
}

bool GR_CocoaGraphics::restoreCachedImage()
{
	LOCK_CONTEXT__;
	[[m_pWin window] restoreCachedImage];
	[[m_pWin window] flushWindowIfNeeded];
	return true; 
}
