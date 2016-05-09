/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004-2006 Tomas Frydrych <dr.tomas@yahoo.co.uk>
 * Copyright (C) 2009 Hubert Figuiere
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


#ifndef __GR_COCOACAIROGRAPHICS_H__
#define __GR_COCOACAIROGRAPHICS_H__

#import <Cocoa/Cocoa.h>

#include "gr_CairoGraphics.h"

class GR_PangoFont;
@class XAP_CocoaNSView;

class ABI_EXPORT GR_CocoaCairoAllocInfo : public GR_CairoAllocInfo
{
public:
 	explicit GR_CocoaCairoAllocInfo(XAP_CocoaNSView * win, bool double_buffered = false)
		: GR_CairoAllocInfo(false, false, double_buffered),
		m_win(win)
		{}

	explicit GR_CocoaCairoAllocInfo(bool bPreview)
		: GR_CairoAllocInfo(bPreview, true, false),
		  m_win(NULL){}
	virtual cairo_t *createCairo();

	XAP_CocoaNSView     * m_win;
};

class ABI_EXPORT GR_CocoaCairoGraphicsBase
	: public GR_CairoGraphics
{
 public:
	~GR_CocoaCairoGraphicsBase();

	virtual GR_Image*	createNewImage(const char* pszName,
									   const UT_ByteBuf* pBB,
	                                   const std::string & mimeType,
									   UT_sint32 iDisplayWidth,
									   UT_sint32 iDisplayHeight,
									   GR_Image::GRType =GR_Image::GRT_Raster);
 protected:
	GR_CocoaCairoGraphicsBase();
	GR_CocoaCairoGraphicsBase(cairo_t *cr, UT_uint32 iDeviceResolution);

};


class ABI_EXPORT GR_CocoaCairoGraphics
	: public GR_CocoaCairoGraphicsBase
{
public:
	~GR_CocoaCairoGraphics();
	static UT_uint32       s_getClassId() {return GRID_COCOA_PANGO;}
	virtual UT_uint32      getClassId() {return s_getClassId();}

	static const char *    graphicsDescriptor(){return "Cocoa Cairo Pango";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);
	XAP_CocoaNSView *  getView () {return m_pWin;}

	virtual GR_Font * getGUIFont(void);

	virtual void		setCursor(GR_Graphics::Cursor c);

        static NSColor                          *_utRGBColorToNSColor (const UT_RGBColor& clr);
        static void                             _utNSColorToRGBColor (NSColor *c, UT_RGBColor &clr);

//	void                createPixmapFromXPM(char ** pXPM,GdkPixmap *source,
//											GdkBitmap * mask);
	virtual void		scroll(UT_sint32, UT_sint32);
	virtual void		scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						   UT_sint32 x_src, UT_sint32 y_src,
						   UT_sint32 width, UT_sint32 height);
	virtual void	    saveRectangle(UT_Rect & r, UT_uint32 iIndx);
	virtual void	    restoreRectangle(UT_uint32 iIndx);
	virtual GR_Image *  genImageFromRectangle(const UT_Rect & r);

	virtual void _beginPaint ();
	virtual void _endPaint ();

	virtual bool		queryProperties(GR_Graphics::Properties gp) const;
  	virtual bool		startPrint(void);
	virtual bool		endPrint(void);
	virtual bool		startPage(const char * szPageLabel,
								  UT_uint32 pageNumber,
								  bool bPortrait,
								  UT_uint32 iWidth, UT_uint32 iHeight);

// Cocoa specific
	static cairo_t *	_createCairo(NSView * view);
	void			init3dColors();
	void setIsPrinting(bool);
	void			setGrabCursor(GR_Graphics::Cursor c) { m_GrabCursor = c; }

	// at least one instance of GR_CocoaGraphics must have been created...
	static const UT_RGBColor &		HBlue()
		{
			return *m_colorBlue16x15;
		}
	static const UT_RGBColor &		VBlue()
		{
			return *m_colorBlue11x16;
		}
	static const UT_RGBColor &		HGrey()
		{
			return *m_colorGrey16x15;
		}
	static const UT_RGBColor &		VGrey()
		{
			return *m_colorGrey11x16;
		}

	static bool _isFlipped();
	typedef bool (*gr_cocoa_graphics_update) (NSRect * rect, GR_CocoaCairoGraphics *pGr, void * param);
	void				_setUpdateCallback (gr_cocoa_graphics_update callback, void * param);
	bool				_callUpdateCallback(NSRect *aRect);
	static	float		_getScreenResolution(void);
protected:
	GR_CocoaCairoGraphics(XAP_CocoaNSView * win = nil, bool double_buffered = false);

	virtual UT_uint32 	_getResolution(void) const;

private:
	static UT_uint32                s_iInstanceCount;
	XAP_CocoaNSView *               m_pWin;
	gr_cocoa_graphics_update	m_updateCallback;
	void 				*m_updateCBparam;
	std::vector<cairo_rectangle_t>	m_cacheRectArray;
	std::vector<cairo_surface_t*>		m_cacheArray;
	GR_Graphics::Cursor		m_GrabCursor;
	UT_uint32			m_screenResolution;
	bool				m_bIsPrinting;
	bool				m_bIsDrawing;
	bool                m_bDoShowPage;
	static void _initColorAndImage(void);
	static bool			m_colorAndImageInited;
	static UT_RGBColor *	m_colorBlue16x15;
	static UT_RGBColor *	m_colorBlue11x16;
	static UT_RGBColor *	m_colorGrey16x15;
	static UT_RGBColor *	m_colorGrey11x16;

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
};


#endif

