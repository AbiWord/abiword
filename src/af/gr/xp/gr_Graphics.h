/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef GR_GRAPHICS_H
#define GR_GRAPHICS_H

#include "ut_types.h"
#include "ut_units.h"
#include "ut_misc.h"
#include "gr_Image.h"

class UT_RGBColor;
class XAP_App;
class XAP_PrefsScheme;

/*
	GR_Font is a reference to a font.  As it happens, everything about fonts
	is platform-specific, so the class contains nothing.  All of its behavior
	and functionality is contained within its subclasses, each of which provides
	the implementation for some specific font technology.
*/
class GR_Font
{
public:
	GR_Font();
	virtual ~GR_Font();

	typedef enum { FF_Unknown=0, FF_Roman=1, FF_Swiss=2, FF_Modern=3,
				   FF_Script=4, FF_Decorative=5, FF_Technical=6, FF_BiDi=7 } FontFamilyEnum;
	typedef enum { FP_Unknown=0, FP_Fixed=1, FP_Variable=2 } FontPitchEnum;

	// The following is actually implemented in platform code.
	// It is primarily used to characterize fonts for RTF export.
	static void s_getGenericFontProperties(const char * szFontName,
										   FontFamilyEnum * pff,
										   FontPitchEnum * pfp,
										   UT_Bool * pbTrueType);
};

/*
	GR_Graphics is a portable interface to a simple 2-d graphics layer.  It is not
	an attempt at a general purpose portability layer.  Rather, it contains only
	functions which are needed.
*/

class GR_Graphics
{
public:
	GR_Graphics();
	virtual ~GR_Graphics();

    // HACK: I need more speed
	virtual void drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff);

	virtual void drawChars(const UT_UCSChar* pChars, 
		int iCharOffset, int iLength, UT_sint32 xoff, UT_sint32 yoff) = 0;
	virtual void setFont(GR_Font* pFont) = 0;

	virtual UT_uint32 getFontAscent() = 0;
	virtual UT_uint32 getFontDescent() = 0;
	virtual UT_uint32 getFontHeight() = 0;
	
	virtual UT_uint32 measureString(const UT_UCSChar*s, int iOffset, int num, unsigned short* pWidths);
	virtual UT_uint32 measureUnRemappedChar(const UT_UCSChar c) = 0;

	UT_UCSChar remapGlyph(const UT_UCSChar actual, UT_Bool noMatterWhat);

	UT_uint32 getMaxCharacterWidth(const UT_UCSChar*s, UT_uint32 Length);
	
	virtual void setColor(UT_RGBColor& clr) = 0;
	
	virtual GR_Font* getGUIFont() = 0;
	virtual GR_Font* findFont(
		const char* pszFontFamily, 
		const char* pszFontStyle, 
		const char* pszFontVariant, 
		const char* pszFontWeight, 
		const char* pszFontStretch, 
		const char* pszFontSize) = 0;
	UT_sint32 convertDimension(const char*) const;
	const char * invertDimension(UT_Dimension, double) const;
	UT_Bool scaleDimensions(const char * szLeftIn, const char * szWidthIn,
				UT_uint32 iWidthAvail,
				UT_sint32 * piLeft,
				UT_uint32 * piWidth) const;

	virtual void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
   	virtual GR_Image* createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType = GR_Image::GRT_Raster);
	
	/* For drawLine() and xorLine():
	**   x0,y0 give the starting pixel.
	**   x1,y1 give the first pixel ***not drawn***.
	*/
	virtual void drawLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32) = 0;
	virtual void xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32) = 0;
	virtual void setLineWidth(UT_sint32) = 0;
	virtual void polyLine(UT_Point * pts, UT_uint32 nPoints) = 0;
	virtual void fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h) = 0;
	virtual void fillRect(UT_RGBColor& c, UT_Rect &r) = 0;
	virtual void invertRect(const UT_Rect* pRect) = 0;
	virtual void setClipRect(const UT_Rect* pRect) = 0;
	virtual void scroll(UT_sint32, UT_sint32) = 0;
	virtual void scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height) = 0;
	virtual void clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32) = 0;
	
	typedef enum { DGP_SCREEN, DGP_PAPER } Properties;
	
	virtual UT_Bool queryProperties(GR_Graphics::Properties gp) const = 0;

	/* the following 3 are only used for printing */
	
	virtual UT_Bool startPrint(void) = 0;
	virtual UT_Bool startPage(const char * szPageLabel, UT_uint32 pageNumber,
							  UT_Bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight) = 0;
	virtual UT_Bool endPrint(void) = 0;

	virtual void flush(void);

	/* specific color space support */

	typedef enum { GR_COLORSPACE_COLOR,
				   GR_COLORSPACE_GRAYSCALE,
				   GR_COLORSPACE_BW
	} ColorSpace;

	virtual void setColorSpace(GR_Graphics::ColorSpace c) = 0;
	virtual GR_Graphics::ColorSpace getColorSpace(void) const = 0;
	
	/* multiple cursor support */
	
	typedef enum { GR_CURSOR_INVALID=0,
				   GR_CURSOR_DEFAULT,
				   GR_CURSOR_IBEAM,
				   GR_CURSOR_RIGHTARROW,
				   GR_CURSOR_IMAGE,
				   GR_CURSOR_IMAGESIZE_NW,
				   GR_CURSOR_IMAGESIZE_N,
				   GR_CURSOR_IMAGESIZE_NE,
				   GR_CURSOR_IMAGESIZE_E,
				   GR_CURSOR_IMAGESIZE_SE,
				   GR_CURSOR_IMAGESIZE_S,
				   GR_CURSOR_IMAGESIZE_SW,
				   GR_CURSOR_IMAGESIZE_W
	} Cursor;

	virtual void setCursor(GR_Graphics::Cursor c) = 0;
	virtual GR_Graphics::Cursor getCursor(void) const = 0;

	void setZoomPercentage(UT_uint32 iZoom);
	UT_uint32 getZoomPercentage(void) const;
	UT_uint32 getResolution(void) const;
	void setLayoutResolutionMode(UT_Bool bEnable) {m_bLayoutResolutionModeEnabled = bEnable;}

	typedef enum { CLR3D_Foreground=0,				/* color of text/foreground on a 3d object */
				   CLR3D_Background=1,				/* color of face/background on a 3d object */
				   CLR3D_BevelUp=2,					/* color of bevel-up  */
				   CLR3D_BevelDown=3,				/* color of bevel-down */
				   CLR3D_Highlight=4				/* color half-way between up and down */
	} GR_Color3D;
#define COUNT_3D_COLORS 5
	
	virtual void					setColor3D(GR_Color3D c) = 0;
	virtual void					fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h) = 0;
	virtual void					fillRect(GR_Color3D c, UT_Rect &r) = 0;

	// Line #172
	// Postscript context positions graphics wrt top of current PAGE, NOT
	// wrt top of document. The screen graphics engine, though positions
	// graphics wrt the top of the document, therefore if we are printing
	// page 5 we need to adjust the vertical position of the graphic in the 
	// postscript image printing routine by (current_page_number-1) * page_height
	// I'm going to call this variable m_iRasterPosition, for want of a better name,
	// it's not acutally a rasterposition --- any better names would be a good idea,
	// I jusy can't think of one right now.
	UT_uint32 m_iRasterPosition;

    virtual void polygon(UT_RGBColor& c,UT_Point *pts,UT_uint32 nPoints);
protected:
	virtual UT_uint32 _getResolution(void) const = 0;
	
	XAP_App	*	m_pApp;
	UT_uint32	m_iZoomPercentage;
	UT_Bool		m_bLayoutResolutionModeEnabled;
	
	static UT_Bool m_bRemapGlyphsMasterSwitch;
	static UT_Bool m_bRemapGlyphsNoMatterWhat;
	static UT_UCSChar m_ucRemapGlyphsDefault;
	static UT_UCSChar *m_pRemapGlyphsTableSrc;
	static UT_UCSChar *m_pRemapGlyphsTableDst;
	static UT_uint32 m_iRemapGlyphsTableLen;

	static XAP_PrefsScheme *m_pPrefsScheme;
	static UT_uint32 m_uTick;
private:
    UT_Bool _PtInPolygon(UT_Point * pts,UT_uint32 nPoints,UT_sint32 x,UT_sint32 y);
};

#endif /* GR_GRAPHICS_H */
