/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2004-6 Tomas Frydrych <dr.tomas@yahoo.co.uk>
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

#ifndef INSIDE_GR_GRAPHICS_H
#error "do not include gr_CairoGraphics.h directly, just use gr_Graphics.h"
#endif

#ifndef GR_CAIROGRAPHICS_H
#define GR_CAIROGRAPHICS_H

#include <vector>

#include <cairo.h>
#include <pango/pango.h>

class GR_CairoGraphics;
class GR_PangoRenderInfo;

class ABI_EXPORT GR_PangoFont : public GR_Font
{
  public:
	GR_PangoFont(const char * pDesc, double dSize,
					 GR_CairoGraphics * pG,
					 const char * pLang,
					 bool bGuiFont = false);
	
	virtual ~GR_PangoFont();

	/*!
		Measure the unremapped char to be put into the cache.
		That means measuring it for a font size of 120
	 */
	virtual UT_sint32 measureUnremappedCharForCache(UT_UCS4Char cChar) const;
	virtual bool      doesGlyphExist(UT_UCS4Char g);
	virtual bool      glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG);
	PangoFont *       getPangoFont() const {return m_pf;}

	void              reloadFont(GR_CairoGraphics * pG);
	double            getPointSize() const {return m_dPointSize;}
	UT_uint32         getZoom() const {return m_iZoom;}
	bool              isGuiFont () const {return m_bGuiFont;}
	const UT_String & getDescription() const {return m_sDesc;}

	virtual const char* getFamily() const;
	const PangoFontDescription * getPangoDescription() const {return m_pfd;}

	// ascent/descent in layout units
	UT_uint32         getAscent() const {return m_iAscent;}
	UT_uint32         getDescent() const {return m_iDescent;}

	PangoCoverage *   getPangoCoverage() const;
	PangoLanguage *   getPangoLanguage() const {return m_pPLang;}
	void              setLanguage(const char * pLang);
	
  private:
	UT_String              m_sDesc;
	double                 m_dPointSize;
	UT_uint32              m_iZoom;
	PangoFont *            m_pf;
	bool                   m_bGuiFont;
	mutable PangoCoverage *m_pCover;
	PangoFontDescription * m_pfd;
	PangoLanguage *        m_pPLang;

	UT_uint32              m_iAscent;
	UT_uint32              m_iDescent;
};

class ABI_EXPORT GR_CairoGraphics : public GR_Graphics
{
	friend class GR_UnixImage;

	// all constructors are protected; instances must be created via
	// GR_GraphicsFactory
public:
	virtual ~GR_CairoGraphics();
	
	virtual void _beginPaint();
	virtual void _endPaint();

	virtual UT_sint32      measureUnRemappedChar(const UT_UCSChar c, UT_uint32 * height = 0);
	
	virtual void		   drawChars(const UT_UCSChar* pChars,
									 int iCharOffset, int iLength,
									 UT_sint32 xoff, UT_sint32 yoff,
									 int * pCharWidth);
                    
	virtual void           drawGlyph(UT_uint32 glyph_idx,
									 UT_sint32 xoff, UT_sint32 yoff);

	virtual UT_uint32      measureString(const UT_UCSChar* s, int iOffset,
										 int num,  UT_GrowBufElement* pWidths, UT_uint32 * height = 0);
	
	virtual GR_Font*	   getDefaultFont(UT_String& fontFamily,
										  const char * pszLang);
	
	virtual void           setFont(const GR_Font *);
	virtual void           clearFont() {m_pPFont = NULL;} 

	virtual void           setZoomPercentage(UT_uint32 iZoom);
	
	///////////////////////////////////////////////////////////////////
	// complex script processing
	//
	virtual bool itemize(UT_TextIterator & text, GR_Itemization & I);
	virtual bool shape(GR_ShapingInfo & si, GR_RenderInfo *& ri);
	virtual void prepareToRenderChars(GR_RenderInfo & ri);
	virtual void renderChars(GR_RenderInfo & ri);
	virtual void measureRenderedCharWidths(GR_RenderInfo & ri);
	virtual void appendRenderedCharsToBuff(GR_RenderInfo & ri, UT_GrowBuf & buf) const;
	virtual bool canBreak(GR_RenderInfo & ri, UT_sint32 &iNext, bool bAfter);

	virtual bool needsSpecialCaretPositioning(GR_RenderInfo & ri);
	virtual UT_uint32 adjustCaretPosition(GR_RenderInfo & ri, bool bForward);
	virtual void adjustDeletePosition(GR_RenderInfo & ri);
	virtual bool nativeBreakInfoForRightEdge() {return false;}
	
	virtual UT_sint32 resetJustification(GR_RenderInfo & ri, bool bPermanent);
	virtual UT_sint32 countJustificationPoints(const GR_RenderInfo & ri) const;
	virtual void      justify(GR_RenderInfo & ri);

	virtual UT_uint32 XYToPosition(const GR_RenderInfo & ri, UT_sint32 x, UT_sint32 y) const;
	virtual void      positionToXY(const GR_RenderInfo & ri,
								   UT_sint32& x, UT_sint32& y,
								   UT_sint32& x2, UT_sint32& y2,
								   UT_sint32& height, bool& bDirection) const;
	virtual UT_sint32 getTextWidth(GR_RenderInfo & ri);

	virtual const UT_VersionInfo & getVersion() const {return s_Version;}

	virtual void setColor(const UT_RGBColor& clr);
	virtual void getColor(UT_RGBColor &clr);
	
	virtual GR_Font * getGUIFont();	
	PangoContext * getContext() const {return m_pContext;}

	virtual UT_uint32 getFontAscent();
	virtual UT_uint32 getFontDescent();
	virtual UT_uint32 getFontHeight();

	virtual UT_uint32 getFontAscent(const GR_Font *);
	virtual UT_uint32 getFontDescent(const GR_Font *);
	virtual UT_uint32 getFontHeight(const GR_Font *);

	virtual void		polygon(UT_RGBColor& c,UT_Point *pts,UT_uint32 nPoints);
	virtual void		clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32);  
	virtual void		drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
	virtual void		xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void		polyLine(UT_Point * pts, UT_uint32 nPoints);
	virtual void		fillRect(const UT_RGBColor& c,
								 UT_sint32 x, UT_sint32 y,
								 UT_sint32 w, UT_sint32 h);
	virtual void		invertRect(const UT_Rect* pRect);
	virtual void		drawLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);

	bool isDingbat() const {return m_bIsDingbat;}
	bool isSymbol() const {return m_bIsSymbol;}
	
	virtual GR_Font* _findFont(const char* pszFontFamily,
							   const char* pszFontStyle,
							   const char* pszFontVariant,
							   const char* pszFontWeight,
							   const char* pszFontStretch,
							   const char* pszFontSize,
							   const char* pszLang);
	
	virtual void getCoverage(UT_NumberVector& coverage);
	virtual void setLineWidth(UT_sint32);
	virtual void setClipRect(const UT_Rect* pRect);
	virtual UT_uint32 getDeviceResolution() const { return m_iDeviceResolution; }

	static  const std::vector<const char *> &       getAllFontNames();
	static  UT_uint32                         getAllFontCount();
	virtual GR_Font * getDefaultFont(GR_Font::FontFamilyEnum f = GR_Font::FF_Roman,
									 const char * pszLang = NULL);

	int dtpu(int d) const;
	int ptdu(int p) const;
	int ptlu(int p) const;
	int ltpu(int l) const;
	int pftlu(int pf) const;

	virtual bool		queryProperties(GR_Graphics::Properties gp) const;
	virtual GR_Image*	createNewImage(const char* pszName,
									   const UT_ByteBuf* pBB,
									   UT_sint32 iDisplayWidth,
									   UT_sint32 iDisplayHeight,
									   GR_Image::GRType =GR_Image::GRT_Raster);
 
  	virtual bool		startPrint();
	virtual bool		endPrint();
	virtual bool		startPage(const char * szPageLabel,
								  UT_uint32 pageNumber,
								  bool bPortrait,
								  UT_uint32 iWidth, UT_uint32 iHeight);

	virtual void	    saveRectangle(UT_Rect & r, UT_uint32 iIndx);
	virtual void	    restoreRectangle(UT_uint32 iIndx);
    virtual GR_Image *  genImageFromRectangle(const UT_Rect & r);

	virtual void setLineProperties(double inWidth, 
					 GR_Graphics::JoinStyle inJoinStyle = JOIN_MITER,
					 GR_Graphics::CapStyle inCapStyle   = CAP_BUTT,
					 GR_Graphics::LineStyle inLineStyle = LINE_SOLID);
	
  protected:
	GR_CairoGraphics(cairo_t *cr);
	inline bool _scriptBreak(GR_PangoRenderInfo &ri);
	void _scaleCharacterMetrics(GR_PangoRenderInfo & RI);
	void _scaleJustification(GR_PangoRenderInfo & RI);

	inline UT_uint32 _measureExtent (PangoGlyphString * pg,
									 PangoFont * pf,
									 UT_BidiCharType iDir,
									 const char * pUtf8,
									 int * & pLogOffsets,
									 UT_sint32 & iStart,
									 UT_sint32 & iEnd);
	
	inline int * _calculateLogicalOffsets (PangoGlyphString * pGlyphs,
										   UT_BidiCharType iVisDir,
										   const char * pUtf8);

	void         _setIsSymbol(bool b) {m_bIsSymbol = b;}
	void         _setIsDingbat(bool b) {m_bIsDingbat = b;}

	PangoFont *  _adjustedPangoFont (GR_PangoFont * pFont, PangoFont * pf);

	PangoFontMap *    m_pFontMap;
	PangoContext *    m_pContext;
	GR_PangoFont* m_pPFont;
	GR_PangoFont* m_pPFontGUI;

	PangoFont *       m_pAdjustedPangoFont;
	GR_PangoFont* m_pAdjustedPangoFontSource;
	UT_uint32         m_iAdjustedPangoFontZoom;
	
	UT_uint32         m_iDeviceResolution;

	cairo_t *         m_cr;
	
	UT_GenericVector<UT_Rect *> m_vSaveRect;
	UT_GenericVector<cairo_t *> m_vSaveRectBuf;

	UT_RGBColor				m_curColor;
	bool                    m_bIsSymbol;       
	bool                    m_bIsDingbat;
	
private:
	static UT_uint32 s_iInstanceCount;
	static UT_VersionInfo s_Version;
	static int s_iMaxScript;
};

class ABI_EXPORT GR_ScreenGraphics : public GR_CairoGraphics
{
protected:
	GR_ScreenGraphics(cairo_t *cr)
	  : GR_CairoGraphics(cr)
	{}

public:
	typedef enum { CLR3D_Foreground=0,				/* color of text/foreground on a 3d object */
				   CLR3D_Background=1,				/* color of face/background on a 3d object */
				   CLR3D_BevelUp=2,					/* color of bevel-up  */
				   CLR3D_BevelDown=3,				/* color of bevel-down */
				   CLR3D_Highlight=4				/* color half-way between up and down */
	} GR_Color3D;
#define COUNT_3D_COLORS 5

	virtual void      setColor3D(GR_Color3D c) = 0;
	virtual bool      getColor3D(GR_Color3D /*name*/, UT_RGBColor & /*color*/) 
	{ return false; }

	virtual void fillRect(GR_Color3D c, UT_Rect &r) = 0;
	virtual void fillRect(GR_Color3D c,
						  UT_sint32 x, UT_sint32 y,
						  UT_sint32 w, UT_sint32 h) = 0;

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
				   GR_CURSOR_CROSSHAIR,
		                   GR_CURSOR_DOWNARROW,
		                   GR_CURSOR_DRAGTEXT,
		                   GR_CURSOR_COPYTEXT
	} Cursor;

	virtual void      setCursor(GR_ScreenGraphics::Cursor c) = 0;
	virtual GR_ScreenGraphics::Cursor getCursor(void) const = 0;

	virtual void      scroll(UT_sint32, UT_sint32) = 0;
	virtual void      scroll(UT_sint32 x_dest,
							 UT_sint32 y_dest,
							 UT_sint32 x_src,
							 UT_sint32 y_src,
							 UT_sint32 width,
							 UT_sint32 height) = 0;
};

#endif /* GR_CAIROGRAPHICS_H */
