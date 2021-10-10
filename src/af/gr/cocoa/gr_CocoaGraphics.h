/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:nil; -*- */
/* Abiword
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

#pragma once

#import <Cocoa/Cocoa.h>

#include <string>

#include "gr_Graphics.h"
#include "xap_CocoaApp.h"
#include "xap_Frame.h"

class GR_CocoaFont;

@class XAP_CocoaNSView;

class GR_CocoaAllocInfo
    : public GR_AllocInfo
{
public:
    GR_CocoaAllocInfo(XAP_CocoaNSView* v)
        : GR_AllocInfo()
        , m_view(v)
    {
    }

    virtual GR_GraphicsId getType() const override { return GRID_COCOA; }
    virtual bool isPrinterGraphics() const override { return false; }

    XAP_CocoaNSView* m_view;
};

class GR_CocoaGraphics
    : public GR_Graphics
{
    // all constructors are protected; instances must be created via
    // GR_GraphicsFactory
public:
    ~GR_CocoaGraphics();

    static UT_uint32 s_getClassId()
    {
        return GRID_COCOA;
    }
    virtual UT_uint32 getClassId() override
    {
        return s_getClassId();
    }

    virtual GR_Capability getCapability() override
    {
        UT_ASSERT(UT_NOT_IMPLEMENTED);
        return GRCAP_UNKNOWN;
    }

    static const char* graphicsDescriptor(void)
    {
        return "Cocoa Default";
    }
    static GR_Graphics* graphicsAllocator(GR_AllocInfo&);

    // HACK: I need more speed
    virtual void drawGlyph(UT_uint32 /*glyph_idx*/, UT_sint32 /*xoff*/, UT_sint32 /*yoff*/) override
    {
        UT_ASSERT(UT_NOT_IMPLEMENTED);
    };
    virtual void drawChars(const UT_UCS4Char* pChars, int iCharOffset,
        int iLength, UT_sint32 xoff, UT_sint32 yoff,
        int* pCharWidhths = nullptr) override;

    virtual void setFont(const GR_Font* pFont) override;
    virtual void clearFont(void) override
    {
        m_pFont = nullptr;
    }
    virtual UT_uint32 getFontHeight() override;

    virtual UT_sint32 measureUnRemappedChar(const UT_UCSChar c, UT_uint32* height = nullptr) override;
    virtual void getCoverage(UT_NumberVector& coverage) override;

    virtual void setColor(const UT_RGBColor& clr) override;
    virtual void getColor(UT_RGBColor& clr) override;

    // at least one instance of GR_CocoaGraphics must have been created...
    static NSColor* HBlue()
    {
        return m_colorBlue16x15;
    }
    static NSColor* VBlue()
    {
        return m_colorBlue11x16;
    }
    static NSColor* HGrey()
    {
        return m_colorGrey16x15;
    }
    static NSColor* VGrey()
    {
        return m_colorGrey11x16;
    }

    virtual GR_Font* getGUIFont() override;

    virtual UT_uint32 getFontAscent() override;
    virtual UT_uint32 getFontDescent() override;
    virtual void drawLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32) override;
#if XAP_DONTUSE_XOR
#else
    // CoreGraphics doesn't support xor mode.
    virtual void xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2) override;
#endif
    virtual void setLineWidth(UT_sint32) override;
    virtual void polyLine(const UT_Point* pts, UT_uint32 nPoints) override;
    void rawPolyAtOffset(NSPoint* point, int npoint, UT_sint32 offset_x, UT_sint32 offset_y, NSColor* color, bool bFill);
    void fillNSRect(NSRect& aRect, NSColor* color);
    virtual void fillRect(const UT_RGBColor& c,
        UT_sint32 x, UT_sint32 y,
        UT_sint32 w, UT_sint32 h) override;
    virtual void invertRect(const UT_Rect* pRect) override;
    virtual void queueDraw(const UT_Rect* pRect) override;
    virtual void setClipRect(const UT_Rect* pRect) override;
    virtual void scroll(UT_sint32, UT_sint32) override;
    virtual void scroll(UT_sint32 x_dest, UT_sint32 y_dest,
        UT_sint32 x_src, UT_sint32 y_src,
        UT_sint32 width, UT_sint32 height) override;
    virtual void clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32) override;

    virtual void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest) override;
    virtual GR_Image* createNewImage(const char* pszName, const UT_ConstByteBufPtr& pBB,
        const std::string& mimetype,
        UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight,
        GR_Image::GRType = GR_Image::GRT_Raster) override;

    virtual void setLineProperties(double inWidthPixels,
        JoinStyle inJoinStyle,
        CapStyle inCapStyle,
        LineStyle inLineStyle) override;

    virtual bool queryProperties(GR_Graphics::Properties gp) const override;
    virtual bool startPrint(void) override;
    virtual bool startPage(const char* szPageLabel, UT_uint32 pageNumber,
        bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight) override;
    virtual void setZoomPercentage(UT_uint32 iZoom) override;
    virtual bool endPrint(void) override;

    virtual void flush(void) override;

    virtual void setColorSpace(GR_Graphics::ColorSpace c) override;
    virtual GR_Graphics::ColorSpace getColorSpace(void) const override;

    virtual void setCursor(GR_Graphics::Cursor c) override;
    virtual GR_Graphics::Cursor getCursor(void) const override;

    void setGrabCursor(GR_Graphics::Cursor c)
    {
        m_GrabCursor = c;
    }

    virtual void setColor3D(GR_Color3D c) override;
    virtual bool getColor3D(GR_Color3D /*name*/, UT_RGBColor& /*color*/) override;
    void init3dColors();
    virtual void fillRect(GR_Color3D c,
        UT_sint32 x, UT_sint32 y,
        UT_sint32 w, UT_sint32 h) override;
    virtual void fillRect(GR_Color3D c, UT_Rect& r) override;

    virtual void polygon(UT_RGBColor& c, UT_Point* pts, UT_uint32 nPoints);

    /* GR_Font versions of the above -- TODO: should I add drawChar* methods too? */
    virtual UT_uint32 getFontAscent(const GR_Font*) override;
    virtual UT_uint32 getFontDescent(const GR_Font*) override;
    virtual UT_uint32 getFontHeight(const GR_Font*) override;

    virtual GR_Image* genImageFromRectangle(const UT_Rect& r) override;
    virtual void saveRectangle(UT_Rect& r, UT_uint32 iIndx) override;
    virtual void restoreRectangle(UT_uint32 iIndx) override;
    virtual UT_uint32 getDeviceResolution(void) const override;

    typedef bool (*gr_cocoa_graphics_update)(NSRect* rect, GR_CocoaGraphics* pGr, void* param);
    void _setUpdateCallback(gr_cocoa_graphics_update callback, void* param);
    bool _callUpdateCallback(NSRect* aRect);
    XAP_CocoaNSView* _getView() const
    {
        return m_view;
    };

    static bool _isFlipped();
    static NSColor* _utRGBColorToNSColor(const UT_RGBColor& clr);
    static void _utNSColorToRGBColor(NSColor* c, UT_RGBColor& clr);
    void setIsPrinting(bool _isPrinting)
    {
        m_bIsPrinting = _isPrinting;
    };
    bool isPrinting(void) const
    {
        return m_bIsPrinting;
    };
    /* Cocoa Specific */
    static CGFloat _getScreenResolution(void);

protected:
    // all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
    GR_CocoaGraphics(XAP_CocoaNSView* view);
    virtual GR_Font* _findFont(const char* pszFontFamily,
        const char* pszFontStyle,
        const char* pszFontVariant,
        const char* pszFontWeight,
        const char* pszFontStretch,
        const char* pszFontSize,
        const char* pszLang) override;

    UT_uint32 _getResolution(void) const;
    void _setColor(NSColor* c);
    virtual void _beginPaint(void) override;
    virtual void _endPaint(void) override;

private:
    void _setClipRectImpl(const UT_Rect* pRect);

    void _resetContext(CGContextRef context); // reset m_CGContext to default values

    gr_cocoa_graphics_update m_updateCallback;
    void* m_updateCBparam;
    XAP_CocoaNSView* m_view;
    CGContextRef m_CGContext;
    std::vector<id> m_cacheArray;
    std::vector<NSRect*> m_cacheRectArray;
    NSColor* m_currentColor;

    static void _initColorAndImage(void);
    static bool m_colorAndImageInited;
    static NSImage* m_imageBlue16x15;
    static NSImage* m_imageBlue11x16;
    static NSImage* m_imageGrey16x15;
    static NSImage* m_imageGrey11x16;
    static NSColor* m_colorBlue16x15;
    static NSColor* m_colorBlue11x16;
    static NSColor* m_colorGrey16x15;
    static NSColor* m_colorGrey11x16;

    static NSCursor* m_Cursor_E;
    static NSCursor* m_Cursor_N;
    static NSCursor* m_Cursor_NE;
    static NSCursor* m_Cursor_NW;
    static NSCursor* m_Cursor_S;
    static NSCursor* m_Cursor_SE;
    static NSCursor* m_Cursor_SW;
    static NSCursor* m_Cursor_W;

    static NSCursor* m_Cursor_Wait;
    static NSCursor* m_Cursor_LeftArrow;
    static NSCursor* m_Cursor_RightArrow;
    static NSCursor* m_Cursor_Compass;
    static NSCursor* m_Cursor_Exchange;
    static NSCursor* m_Cursor_LeftRight;
    static NSCursor* m_Cursor_UpDown;
    static NSCursor* m_Cursor_Crosshair;
    static NSCursor* m_Cursor_HandPointer;
    static NSCursor* m_Cursor_DownArrow;

    // our currently requested font by handle
    const GR_CocoaFont* m_pFont;
    NSFont* m_fontForGraphics;

    GR_CocoaFont* m_pFontGUI;
    static UT_uint32 s_iInstanceCount;

    CGFloat m_fLineWidth; // device unit
    // line properties
    JoinStyle m_joinStyle;
    CapStyle m_capStyle;
    LineStyle m_lineStyle;

    GR_Graphics::Cursor m_cursor;
    GR_Graphics::Cursor m_GrabCursor;

    GR_Graphics::ColorSpace m_cs;

    float m_screenResolution;
    bool m_bIsPrinting;
    bool m_bIsDrawing;

public: //HACK
    NSColor* m_3dColors[COUNT_3D_COLORS];

private:
    /* private implementations. Allow esasy selection accross various ways */
    CGFloat _measureUnRemappedCharCached(const UT_UCSChar c);
    void _setCapStyle(CapStyle inCapStyle, CGContextRef* context = 0);
    void _setJoinStyle(JoinStyle inJoinStyle, CGContextRef* context = 0);
    void _setLineStyle(LineStyle inLineStyle, CGContextRef* context = 0);
    //
    /*!
	  Wrapper to draw the char.

	  \param ctLine  the CTLine
	  \param x X position
	  \param y Y position
	  \param begin the start of the range to draw
	  \param rangelen the length of the range

	  \note the NSView must be focused prior this call
	 */
    void _realDrawChars(CTLineRef ctLine,
        CGFloat x, CGFloat y, int begin, int rangelen,
        CGFloat xOffset);
};

class GR_CocoaPatternImpl
    : public UT_ColorPatImpl
{
public:
    GR_CocoaPatternImpl(NSImage* img)
        : m_pattern([img copy])
    {
    }
    ~GR_CocoaPatternImpl()
    {
        [m_pattern release];
    }
    virtual UT_ColorPatImpl* clone() const override
    {
        return new GR_CocoaPatternImpl(this->m_pattern);
    }

private:
    NSImage* m_pattern;
};
