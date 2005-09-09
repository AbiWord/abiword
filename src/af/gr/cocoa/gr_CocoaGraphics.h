/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* Abiword
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

#ifndef GR_COCOAGRAPHICS_H
#define GR_COCOAGRAPHICS_H

#import <Cocoa/Cocoa.h>

#include "xap_CocoaApp.h"
#include "xap_Frame.h"
#include "gr_Graphics.h"

class UT_ByteBuf;

class GR_CocoaGraphics;

class XAP_CocoaFont;

class StNSViewLocker;

@class XAP_CocoaNSView, XAP_CocoaNSScrollView;

class GR_CocoaAllocInfo : public GR_AllocInfo
{
public:
	GR_CocoaAllocInfo(NSView * v, XAP_App * app)
		: m_view(v), m_app(app) {};

	virtual GR_GraphicsId getType() const {return GRID_COCOA;}
	virtual bool isPrinterGraphics() const { return false; }
	
	NSView *m_view;
	XAP_App *m_app;
};

class GR_CocoaGraphics : public GR_Graphics
{
	// all constructors are protected; instances must be created via
	// GR_GraphicsFactory
 public:
	~GR_CocoaGraphics();


	static UT_uint32 s_getClassId() {return GRID_COCOA;}
	virtual UT_uint32 getClassId() {return s_getClassId();}
	
	virtual GR_Capability getCapability(){UT_ASSERT(UT_NOT_IMPLEMENTED); return GRCAP_UNKNOWN;}
	
	static const char *    graphicsDescriptor(void){return "Cocoa Default";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);
	
    // HACK: I need more speed
	virtual void      drawGlyph(UT_uint32 glyph_idx, UT_sint32 xoff, UT_sint32 yoff) 
		{ UT_ASSERT (UT_NOT_IMPLEMENTED); };
	virtual void		drawChars(const UT_UCSChar* pChars, int iCharOffset,
								  int iLength, UT_sint32 xoff, UT_sint32 yoff,
								  int * pCharWidhths = NULL);

	virtual void		setFont(GR_Font* pFont);
	virtual void        clearFont(void) { m_pFont = NULL;}
	virtual UT_uint32	getFontHeight();

	virtual UT_sint32 measureUnRemappedChar(const UT_UCSChar c);
	virtual void getCoverage(UT_NumberVector& coverage);

	virtual void		setColor(const UT_RGBColor& clr);
	virtual void		getColor(UT_RGBColor& clr);

	NSColor *		HBlue() const { return m_colorBlue16x15; }
	NSColor *		VBlue() const { return m_colorBlue11x16; }
	NSColor *		HGrey() const { return m_colorGrey16x15; }
	NSColor *		VGrey() const { return m_colorGrey11x16; }

	virtual GR_Font*	getGUIFont();

	virtual UT_uint32	getFontAscent();
	virtual UT_uint32	getFontDescent();
	virtual void		drawLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void		setLineWidth(UT_sint32);
	virtual void		polyLine(UT_Point * pts, UT_uint32 nPoints);
	void			rawPolyAtOffset(NSPoint * point, int npoint, UT_sint32 offset_x, UT_sint32 offset_y, NSColor * color, bool bFill);
	void			fillNSRect (NSRect & aRect, NSColor * color);
	virtual void		fillRect(const UT_RGBColor& c,
								 UT_sint32 x, UT_sint32 y,
								 UT_sint32 w, UT_sint32 h);
	virtual void		invertRect(const UT_Rect* pRect);
	virtual void		setClipRect(const UT_Rect* pRect);
	virtual void		scroll(UT_sint32, UT_sint32);
	virtual void		scroll(UT_sint32 x_dest, UT_sint32 y_dest,
							   UT_sint32 x_src, UT_sint32 y_src,
							   UT_sint32 width, UT_sint32 height);
	virtual void		clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
  
	virtual void		drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
	virtual GR_Image*	createNewImage(const char* pszName, const UT_ByteBuf* pBB,
					       UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight,
					       GR_Image::GRType = GR_Image::GRT_Raster);
  
	virtual void setLineProperties ( double    inWidthPixels, 
				      JoinStyle inJoinStyle,
				      CapStyle  inCapStyle,
				      LineStyle inLineStyle);

	virtual bool		queryProperties(GR_Graphics::Properties gp) const;
	virtual bool		startPrint(void);
	virtual bool		startPage(const char * szPageLabel, UT_uint32 pageNumber,
								  bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	virtual void      setZoomPercentage(UT_uint32 iZoom);
	virtual bool		endPrint(void);

	virtual void		flush(void);

	virtual void		setColorSpace(GR_Graphics::ColorSpace c);
	virtual GR_Graphics::ColorSpace getColorSpace(void) const;
	
	virtual void		setCursor(GR_Graphics::Cursor c);
	virtual GR_Graphics::Cursor getCursor(void) const;

	void			setGrabCursor(GR_Graphics::Cursor c) { m_GrabCursor = c; }

	virtual void		setColor3D(GR_Color3D c);
	void				init3dColors();
	virtual void		fillRect(GR_Color3D c,
								 UT_sint32 x, UT_sint32 y,
								 UT_sint32 w, UT_sint32 h);
	virtual void		fillRect(GR_Color3D c, UT_Rect &r);
	
	virtual void		polygon(UT_RGBColor& c,UT_Point *pts,UT_uint32 nPoints);
	
	/* GR_Font versions of the above -- TODO: should I add drawChar* methods too? */
	virtual UT_uint32 getFontAscent(GR_Font *);
	virtual UT_uint32 getFontDescent(GR_Font *);
	virtual UT_uint32 getFontHeight(GR_Font *);

    virtual GR_Image * genImageFromRectangle(const UT_Rect & r);
	virtual void	  saveRectangle(UT_Rect & r, UT_uint32 iIndx);
	virtual void	  restoreRectangle(UT_uint32 iIndx);
	virtual UT_uint32 getDeviceResolution(void) const;


	typedef bool (*gr_cocoa_graphics_update) (NSRect * rect, GR_CocoaGraphics *pGr, void * param);
	void				_setUpdateCallback (gr_cocoa_graphics_update callback, void * param);
	bool				_callUpdateCallback(NSRect *aRect);
	XAP_CocoaNSView *	_getView () { return m_pWin; };

	static bool			_isFlipped();
	static NSColor				*_utRGBColorToNSColor (const UT_RGBColor& clr);
	static void 				_utNSColorToRGBColor (NSColor *c, UT_RGBColor &clr);
	void				setIsPrinting(bool isPrinting) { m_bIsPrinting = isPrinting; };
	bool				isPrinting(void) const { return m_bIsPrinting; };
	/* Cocoa Specific */
	static	float		_getScreenResolution(void);

protected:
	// all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
	GR_CocoaGraphics(NSView * view, /*XAP_CocoaFontManager * fontManager,*/ XAP_App *app);
	virtual GR_Font*	_findFont(const char* pszFontFamily, 
								  const char* pszFontStyle, 
								  const char* pszFontVariant, 
								  const char* pszFontWeight, 
								  const char* pszFontStretch, 
								  const char* pszFontSize);

	virtual UT_uint32 	_getResolution(void) const;
	void				_setColor(NSColor * c);
	virtual void _beginPaint (void);
	virtual void _endPaint (void);
private:
	void		_setClipRectImpl(const UT_Rect* pRect);

	
	NSImage*			_makeNewCacheImage() 
	{
			NSImage * cache = [[NSImage alloc] initWithSize:NSMakeSize(0,0)];
			[cache setFlipped:YES];
			return cache;
	}
	
	void				_resetContext(CGContextRef context);	// reset m_CGContext to default values

	gr_cocoa_graphics_update	m_updateCallback;
	void 						*m_updateCBparam;
	XAP_CocoaNSView *  			m_pWin;
	NSMutableDictionary*		m_fontProps;
	CGContextRef				m_CGContext;
	UT_GenericVector<id>		m_cacheArray;
	UT_GenericVector<NSRect*>	m_cacheRectArray;
	NSColor *					m_currentColor;

	static void _initColorAndImage(void);
	static bool                 m_colorAndImageInited;
	static NSImage *			m_imageBlue16x15;
	static NSImage *			m_imageBlue11x16;
	static NSImage *			m_imageGrey16x15;
	static NSImage *			m_imageGrey11x16;
	static NSColor *			m_colorBlue16x15;
	static NSColor *			m_colorBlue11x16;
	static NSColor *			m_colorGrey16x15;
	static NSColor *			m_colorGrey11x16;
	
	static NSCursor *	m_Cursor_E;
	static NSCursor *	m_Cursor_N;
	static NSCursor *	m_Cursor_NE;
	static NSCursor *	m_Cursor_NW;
	static NSCursor *	m_Cursor_S;
	static NSCursor *	m_Cursor_SE;
	static NSCursor *	m_Cursor_SW;
	static NSCursor *	m_Cursor_W;

	static NSCursor *	m_Cursor_Wait;
	static NSCursor *	m_Cursor_LeftArrow;
	static NSCursor *	m_Cursor_RightArrow;
	static NSCursor *	m_Cursor_Compass;
	static NSCursor *	m_Cursor_Exchange;
	static NSCursor *	m_Cursor_LeftRight;
	static NSCursor *	m_Cursor_UpDown;
	static NSCursor *	m_Cursor_Crosshair;
	static NSCursor *	m_Cursor_HandPointer;
	static NSCursor *	m_Cursor_DownArrow;
	
	// our currently requested font by handle
	XAP_CocoaFont *	m_pFont;
	NSFont*						m_fontForGraphics;

	XAP_CocoaFont*	m_pFontGUI;
	static UT_uint32		s_iInstanceCount;
  
	float					m_fLineWidth;			// device unit
	// line properties
	JoinStyle m_joinStyle;
	CapStyle m_capStyle;
	LineStyle m_lineStyle;

	GR_Graphics::Cursor		m_cursor;
	GR_Graphics::Cursor		m_GrabCursor;

	GR_Graphics::ColorSpace	m_cs;
	
	UT_uint32				m_screenResolution;
	bool					m_bIsPrinting;
	bool					m_bIsDrawing;

public:		//HACK	
	NSColor	*			m_3dColors[COUNT_3D_COLORS];
private:
	/* private implementations. Allow esasy selection accross various ways */
	void _initMetricsLayouts(void);
	float _measureUnRemappedCharCached(const UT_UCSChar c);
	void _setCapStyle(CapStyle inCapStyle, CGContextRef * context = 0);
	void _setJoinStyle(JoinStyle inJoinStyle, CGContextRef * context = 0);
	void _setLineStyle (LineStyle inLineStyle, CGContextRef * context = 0);
	void _restartPaint(void);
	//
	/*!
	  Wrapper to draw the char.

	  \param cBuf the unichar buffer for the string
	  \param len the length of the buffer
	  \param fontProps the properties for the NSAttributedString
	  \param x X position
	  \param y Y position
	  \param begin the start of the range to draw
	  \param rangelen the length of the range

	  \note the NSView must be focused prior this call
	 */
	void _realDrawChars(const unichar* cBuf, int len, NSDictionary *fontProps, 
						float x, float y, int begin, int rangelen);
	//
	StNSViewLocker* m_viewLocker;
	//private font metrics objects
	NSTextStorage *m_fontMetricsTextStorage;
    NSLayoutManager *m_fontMetricsLayoutManager;
    NSTextContainer *m_fontMetricsTextContainer;
};

#endif /* GR_COCOAGRAPHICS_H */
