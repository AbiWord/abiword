/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Tomas Frydrych, <tomas@frydrych.uklinux.net>
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
#include "ut_growbuf.h"
#include "ut_misc.h"
#include "gr_Image.h"
#include "gr_Caret.h"
#include "gr_Transform.h"
#include "gr_CharWidthsCache.h"
#include "ut_hash.h"

class UT_RGBColor;
class XAP_App;
class XAP_PrefsScheme;
class XAP_Frame;
class UT_String;

/*!
  GR_Font is a reference to a font.  As it happens, everything about fonts
  is platform-specific, so the class contains nothing.  All of its behavior
  and functionality is contained within its subclasses, each of which provides
  the implementation for some specific font technology.
  
  May 16 2003
  The assertion made above is mostly wrong. Font works almost the same
  on every platform. We have metrics, glyphs, measurement, etc. Sure implementation is utterly 
  platform dependent, but we can provide a mostly XP interface to that.
  That would certainly reduce platform code a increase XP code.
   -- Hub
*/

class GR_Graphics;
class ABI_EXPORT GR_Font
{
	friend class GR_Graphics;

 public:
	GR_Font();

	typedef enum { FF_Unknown = 0, FF_Roman, FF_Swiss, FF_Modern,
				   FF_Script, FF_Decorative, FF_Technical, FF_BiDi, FF_Last } FontFamilyEnum;
	typedef enum { FP_Unknown = 0, FP_Fixed, FP_Variable } FontPitchEnum;

	// The following is actually implemented in platform code.
	// It is primarily used to characterize fonts for RTF export.
	static void s_getGenericFontProperties(const char * szFontName,
										   FontFamilyEnum * pff,
										   FontPitchEnum * pfp,
										   bool * pbTrueType);

	virtual const char* getFamily() const { return NULL; }
	UT_uint32           getAllocNumber() const {return m_iAllocNo;}
	/*!
		Measure the unremapped char to be put into the cache.
		That means measuring it for a font size of 120
	 */
	virtual UT_sint32 measureUnremappedCharForCache(UT_UCSChar cChar) const = 0;
	virtual const UT_String & hashKey(void) const;
	UT_uint32 getCharWidthFromCache (UT_UCSChar c) const;
	virtual GR_CharWidths* newFontWidths(void) const; /*reimplement if you want to instanciate something else */
	/*
	   implemented using character widths; platforms might want to
	   provide different implementation
	   NB: it is essential that this function is fast
	*/
	virtual bool doesGlyphExist(UT_UCS4Char g);
	static  bool s_doesGlyphExist(UT_UCS4Char g, void *instance)
	{
		UT_return_val_if_fail(instance, false);
		GR_Font * pThis = static_cast<GR_Font*>(instance);
		return pThis->doesGlyphExist(g);
	}
	
	
	
  protected:

	virtual ~GR_Font();

	GR_CharWidths * _getCharWidths() const {return m_pCharWidths;}
	/*! 
	  hash key for font cache. Must be initialized in ctor
	 otherwise override hashKey() method 
	*/
	mutable UT_String		m_hashKey;
  private:

	static UT_uint32 s_iAllocCount;
	UT_uint32        m_iAllocNo;
	mutable GR_CharWidths*	m_pCharWidths;
};


/*
  GR_Graphics is a portable interface to a simple 2-d graphics layer.  It is not
  an attempt at a general purpose portability layer.  Rather, it contains only
  functions which are needed.
*/

#define GR_OC_LEFT_FLUSHED 0x40000000 // flip bit 31
#define GR_OC_MAX_WIDTH    0x3fffffff
class ABI_EXPORT GR_Graphics
{
	friend class GR_Caret;
 public:
	GR_Graphics();
	virtual ~GR_Graphics();

	UT_sint32	tdu(UT_sint32 layoutUnits) const;
	UT_sint32	tlu(UT_sint32 deviceUnits) const;
	double	    tduD(double layoutUnits) const;
	double  	tluD(double deviceUnits) const;
	/*!
		Font units to layout units. Returns the dimension in layout units since font
		are not Zoomed
	 */
	UT_sint32	ftlu(UT_sint32 fontUnits) const;
	double		ftluD(double fontUnits) const;

	virtual void      drawGlyph(UT_uint32 glyph_idx, UT_sint32 xoff, UT_sint32 yoff) = 0;
	virtual void      drawChars(const UT_UCSChar* pChars,
								int iCharOffset,
								int iLength,
								UT_sint32 xoff,
								UT_sint32 yoff,
								int* pCharWidths = NULL) = 0;

	virtual void      setFont(GR_Font* pFont) = 0;
    virtual void      clearFont(void) = 0;
	virtual UT_uint32 getFontAscent() = 0;
	virtual UT_uint32 getFontDescent() = 0;
	virtual UT_uint32 getFontHeight() = 0;

	virtual UT_uint32 measureString(const UT_UCSChar*s,
									int iOffset,
									int num,
									UT_GrowBufElement* pWidths);

	virtual UT_uint32 measureUnRemappedChar(const UT_UCSChar c) = 0;
	virtual void getCoverage(UT_Vector& coverage) = 0;
	
	/* GR_Font versions of the above -- TODO: should I add drawChar* methods too? */
	virtual UT_uint32 getFontAscent(GR_Font *)  = 0;
	virtual UT_uint32 getFontDescent(GR_Font *) = 0;
	virtual UT_uint32 getFontHeight(GR_Font *)  = 0;

	UT_uint32         getMaxCharacterWidth(const UT_UCSChar*s, UT_uint32 Length);

	virtual void      setColor(const UT_RGBColor& clr) = 0;
	virtual void      getColor(UT_RGBColor& clr) = 0;
	virtual GR_Font*  getGUIFont() = 0;

	GR_Font*  findFont(const char* pszFontFamily,
					   const char* pszFontStyle,
					   const char* pszFontVariant,
					   const char* pszFontWeight,
					   const char* pszFontStretch,
					   const char* pszFontSize);
	static const char* findNearestFont(const char* pszFontFamily,
									   const char* pszFontStyle,
									   const char* pszFontVariant,
									   const char* pszFontWeight,
									   const char* pszFontStretch,
									   const char* pszFontSize);

	const char *      invertDimension(UT_Dimension, double) const;

	bool              scaleDimensions(const char * szLeftIn,
									  const char * szWidthIn,
									  UT_uint32 iWidthAvail,
									  UT_sint32 * piLeft,
									  UT_uint32 * piWidth) const;

	virtual void      drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);

   	virtual GR_Image* createNewImage(const char* pszName,
									 const UT_ByteBuf* pBB,
									 UT_sint32 iWidth,
									 UT_sint32 iHeight,
									 GR_Image::GRType iType = GR_Image::GRT_Raster);

	/* For drawLine() and xorLine():
	**   x0,y0 give the starting pixel.
	**   x1,y1 give the first pixel ***not drawn***.
	*/
	virtual void      drawLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32) = 0;
	virtual void      xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32) = 0;
	virtual void      setLineWidth(UT_sint32) = 0;
	virtual void      polyLine(UT_Point * pts, UT_uint32 nPoints) = 0;

	virtual void      fillRect(const UT_RGBColor& c,
							   UT_sint32 x,
							   UT_sint32 y,
							   UT_sint32 w,
							   UT_sint32 h) = 0;
	virtual void      fillRect(GR_Image *pImg, const UT_Rect &src, const UT_Rect & dest);

	void      fillRect(const UT_RGBColor& c, const UT_Rect &r);
	virtual void      invertRect(const UT_Rect* pRect) = 0;
	virtual void      setClipRect(const UT_Rect* pRect) = 0;
	const UT_Rect *   getClipRect(void) const { return m_pRect;}
	virtual void      scroll(UT_sint32, UT_sint32) = 0;

	virtual void      scroll(UT_sint32 x_dest,
				 UT_sint32 y_dest,
				 UT_sint32 x_src,
				 UT_sint32 y_src,
				 UT_sint32 width,
				 UT_sint32 height) = 0;

	virtual void      clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32) = 0;

	typedef enum { DGP_SCREEN, DGP_PAPER, DGP_OPAQUEOVERLAY } Properties;

	typedef enum
	  {
	    JOIN_MITER,
	    JOIN_ROUND,
	    JOIN_BEVEL
	  } JoinStyle ;

	typedef enum
	  {
	    CAP_BUTT,
	    CAP_ROUND,
	    CAP_PROJECTING
	  } CapStyle ;

	typedef enum
	  {
	    LINE_SOLID,
	    LINE_ON_OFF_DASH,
	    LINE_DOUBLE_DASH,
		LINE_DOTTED
	  } LineStyle ;

	virtual void setLineProperties ( double inWidthPixels,
					 JoinStyle inJoinStyle = JOIN_MITER,
					 CapStyle inCapStyle   = CAP_BUTT,
					 LineStyle inLineStyle = LINE_SOLID ) ;

	virtual bool      queryProperties(GR_Graphics::Properties gp) const = 0;
	/* the following 3 are only used for printing */

	virtual bool      startPrint(void) = 0;

	virtual bool      startPage(const char * szPageLabel,
								UT_uint32 pageNumber,
								bool bPortrait,
								UT_uint32 iWidth,
								UT_uint32 iHeight) = 0;

	virtual bool      endPrint(void) = 0;

	virtual void      flush(void);

	/* specific color space support */

	typedef enum { GR_COLORSPACE_COLOR,
				   GR_COLORSPACE_GRAYSCALE,
				   GR_COLORSPACE_BW
	} ColorSpace;

	virtual void      setColorSpace(GR_Graphics::ColorSpace c) = 0;
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
				   GR_CURSOR_IMAGESIZE_W,
				   GR_CURSOR_LEFTRIGHT,
				   GR_CURSOR_UPDOWN,
				   GR_CURSOR_EXCHANGE,
				   GR_CURSOR_GRAB,
				   GR_CURSOR_LINK,
				   GR_CURSOR_WAIT,
				   GR_CURSOR_LEFTARROW,
				   GR_CURSOR_VLINE_DRAG,
				   GR_CURSOR_HLINE_DRAG,
				   GR_CURSOR_CROSSHAIR
	} Cursor;

	virtual void      setCursor(GR_Graphics::Cursor c) = 0;
	virtual GR_Graphics::Cursor getCursor(void) const = 0;

	virtual void      setZoomPercentage(UT_uint32 iZoom);
	UT_uint32         getZoomPercentage(void) const;
	static UT_uint32  getResolution(void) { return UT_LAYOUT_RESOLUTION; }
	inline void       setPortrait (bool b) {m_bIsPortrait = b;}
	inline bool       isPortrait (void) const {return m_bIsPortrait;}

	typedef enum { CLR3D_Foreground=0,				/* color of text/foreground on a 3d object */
				   CLR3D_Background=1,				/* color of face/background on a 3d object */
				   CLR3D_BevelUp=2,					/* color of bevel-up  */
				   CLR3D_BevelDown=3,				/* color of bevel-down */
				   CLR3D_Highlight=4				/* color half-way between up and down */
	} GR_Color3D;
#define COUNT_3D_COLORS 5

	virtual void      setColor3D(GR_Color3D c) = 0;
	virtual void      fillRect(GR_Color3D c,
							   UT_sint32 x,
							   UT_sint32 y,
							   UT_sint32 w,
							   UT_sint32 h) = 0;

	virtual void	  fillRect(GR_Color3D c, UT_Rect &r) = 0;

    virtual void      polygon(UT_RGBColor& c,UT_Point *pts,UT_uint32 nPoints);
	//
	// Methods to deal with background repainting as used in the Unix FE. These
	// make redraws really fast and fix bug 119
	//
	const bool        isSpawnedRedraw(void) const;
	void              setSpawnedRedraw(bool exposeState);

	void              setPendingRect(UT_sint32 x,
									 UT_sint32 y,
									 UT_sint32 width,
									 UT_sint32 height);

	void              unionPendingRect( UT_Rect * pRect);
	void              setRecentRect( UT_Rect * pRect);
	const UT_Rect *   getPendingRect(void) const;
	const bool        isExposePending(void) const;
	void              setExposePending(bool bExposePending);
	const bool        isExposedAreaAccessed(void) const;
	void              setExposedAreaAccessed(bool bAccessedState);
	void              setDontRedraw( bool bDontRedraw);
	bool              isDontRedraw(void);
	void              doRepaint(UT_Rect * rClip);
	void              setDoMerge( bool bMergeState);
	bool              doMerge(void) const;

	const GR_Transform & getTransform() const {return m_Transform;}

	/* returns true on success, false on failure */
	bool              setTransform(const GR_Transform & tr)
	                     {
							 bool ret = _setTransform(tr);
							 if(!ret)
								 return false;
							 m_Transform = tr;
							 return true;
						 }

	void              createCaret()
		{
			UT_ASSERT(!m_pCaret);
			m_pCaret = new GR_Caret(this);
		}
	GR_Caret *        getCaret() { return m_pCaret; }
	virtual GR_Image *	  genImageFromRectangle(const UT_Rect & r) = 0;
	virtual void	  saveRectangle(UT_Rect & r, UT_uint32 iIndx) = 0;
	virtual void	  restoreRectangle(UT_uint32 iIndx) = 0;
	virtual UT_uint32 getDeviceResolution(void) const = 0;
//
// Use these methods to fix off by 1 errors while scrolling. Add the
// the logical difference to these first, then calculate how much
// the screen needs to scroll in device units
//
	UT_sint32         getPrevYOffset(void) const { return m_iPrevYOffset;}
	UT_sint32         getPrevXOffset(void) const { return m_iPrevXOffset;}
	void              setPrevYOffset(UT_sint32 y) { m_iPrevYOffset = y;}
	void              setPrevXOffset(UT_sint32 x) { m_iPrevXOffset = x;}

 protected:
	UT_sint32         _tduX(UT_sint32 layoutUnits) const;
	UT_sint32         _tduY(UT_sint32 layoutUnits) const;
	UT_sint32         _tduR(UT_sint32 layoutUnits) const;

	void _destroyFonts ();

	virtual GR_Font*  _findFont(const char* pszFontFamily,
								const char* pszFontStyle,
								const char* pszFontVariant,
								const char* pszFontWeight,
								const char* pszFontStretch,
								const char* pszFontSize) = 0;

 private:
	virtual bool       _setTransform(const GR_Transform & tr)
		                  {
							  UT_ASSERT( UT_NOT_IMPLEMENTED );
							  return false;
						  }
	
 public:
	// TODO -- this should not be public, create access methods !!!
	//
	// Postscript context positions graphics wrt top of current PAGE, NOT
	// wrt top of document. The screen graphics engine, though positions
	// graphics wrt the top of the document, therefore if we are printing
	// page 5 we need to adjust the vertical position of the graphic in the
	// postscript image printing routine by (current_page_number-1) * page_height
	// I'm going to call this variable m_iRasterPosition, for want of a better name,
	// it's not acutally a rasterposition --- any better names would be a good idea,
	// I jusy can't think of one right now.
	UT_uint32         m_iRasterPosition;

 protected:
	XAP_App	*	      m_pApp;
	UT_uint32	      m_iZoomPercentage;
	UT_uint32         m_iFontAllocNo;

	static XAP_PrefsScheme *m_pPrefsScheme;
	static UT_uint32 m_uTick;

	const UT_Rect *  m_pRect;
 private:
	GR_Caret *		 m_pCaret;
    bool             _PtInPolygon(UT_Point * pts,UT_uint32 nPoints,UT_sint32 x,UT_sint32 y);
    bool             m_bIsPortrait;
	bool             m_bSpawnedRedraw;
	UT_Rect          m_PendingExposeArea;
	UT_Rect          m_RecentExposeArea;
	bool             m_bExposePending;
	bool             m_bIsExposedAreaAccessed;
	bool             m_bDontRedraw;
	bool             m_bDoMerge;
//
// These hold the previous x and Y offset calculated from the scrolling code
// in Logical units. We need them to avoid off by 1 errors in scrolling.
//
	UT_sint32        m_iPrevYOffset;
	UT_sint32        m_iPrevXOffset;
	GR_Transform     m_Transform;

	UT_StringPtrMap	 m_hashFontCache;
	bool             m_bDoNotZoomText;
};

void xorRect(GR_Graphics* pG, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
void xorRect(GR_Graphics* pG, const UT_Rect& r);
#ifdef DEBUG
void flash(GR_Graphics* pG, const UT_Rect& r, const UT_RGBColor& c);
#endif

#endif /* GR_GRAPHICS_H */
