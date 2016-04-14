/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 2004-6 Tomas Frydrych <dr.tomas@yahoo.co.uk>
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

#ifndef GR_UNIX_PANGOGRAPHICS_H
#define GR_UNIX_PANGOGRAPHICS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>

#include "ut_types.h"
#include "ut_color.h"
#include "ut_string_class.h"
#include "gr_RenderInfo.h"

#include <cairo.h>
#include <pango/pango.h>

// we do not want this to be a plugin for now
#define GR_UNIXPANGO_BUILTIN

#ifndef GR_UNIXPANGO_BUILTIN
#define PLUGIN_NAME "Cairo Pango graphics"
#endif

/************************************************************************/
/************************************************************************/

class GR_PangoRenderInfo;
class GR_CairoGraphics;
class XAP_Frame;

class GR_CairoPatternImpl
	: public UT_ColorPatImpl
{
public:
	GR_CairoPatternImpl(const char * fileName);
	// don't take ownership
	GR_CairoPatternImpl(cairo_surface_t * surf);
	GR_CairoPatternImpl(const GR_CairoPatternImpl &);
	virtual ~GR_CairoPatternImpl();
    virtual UT_ColorPatImpl * clone() const;
	cairo_pattern_t *getPattern() const
		{
			return m_pattern;
		}
private:
	GR_CairoPatternImpl & operator=(const GR_CairoPatternImpl &);
	cairo_pattern_t *m_pattern;
};


class GR_CairoVectorImage
	: public GR_VectorImage
{
public:
	virtual void cairoSetSource(cairo_t *) = 0;
	virtual void renderToCairo(cairo_t*) = 0;
};

/** An abstract Cairo image */
class GR_CairoRasterImage
	: public GR_RasterImage
{
public:
	virtual GR_Image * createImageSegment(GR_Graphics * pG,const UT_Rect & rec);
	virtual void cairoSetSource(cairo_t *) = 0;
protected:
	// called by createImageSegment()
	virtual GR_CairoRasterImage *makeSubimage(const std::string & n,
											  UT_sint32 x, UT_sint32 y,
											  UT_sint32 w, UT_sint32 h) const = 0;
};

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
	virtual bool      doesGlyphExist(UT_UCS4Char g) const;
	virtual bool      glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG);
	PangoFont *       getPangoFont() const {return m_pf;}
	PangoFont *       getPangoLayoutFont() const {return m_pLayoutF;}

	void              reloadFont(GR_CairoGraphics * pG);
	double            getPointSize() const {return m_dPointSize;}
	UT_uint32         getZoom() const {return m_iZoom;}
	bool              isGuiFont () const {return m_bGuiFont;}
	const UT_String & getDescription() const {return m_sDesc;}

	virtual const char* getFamily() const;
	const PangoFontDescription * getPangoDescription() const {return m_pfdLay;}

	// ascent/descent in layout units
	UT_uint32         getAscent() const {return m_iAscent;}
	UT_uint32         getDescent() const {return m_iDescent;}

	PangoCoverage *   getPangoCoverage() const;
	PangoLanguage *   getPangoLanguage() const {return m_pPLang;}
	void              setLanguage(const char * pLang);
  private:
	UT_String              m_sDesc;
	UT_String              m_sLayoutDesc;
	double                 m_dPointSize;
	UT_uint32              m_iZoom;
	PangoFont *            m_pf;
	bool                   m_bGuiFont;
	mutable PangoCoverage *m_pCover;
	PangoFontDescription * m_pfdDev;
	PangoFontDescription * m_pfdLay;
	PangoLanguage *        m_pPLang;

	UT_uint32              m_iAscent;
	UT_uint32              m_iDescent;
	PangoFont *            m_pLayoutF;
};

class GR_PangoRenderInfo;


class ABI_EXPORT GR_CairoAllocInfo
	: public GR_AllocInfo
{
public:
	GR_CairoAllocInfo(bool bPreview, bool bPrinter, bool double_buffered)
		: m_bPreview(bPreview),
		  m_bPrinter(bPrinter),
		  m_double_buffered(double_buffered)
	{
	}
	virtual GR_GraphicsId getType() const {return GRID_UNIX;}
	virtual bool isPrinterGraphics() const {return m_bPrinter;}
	virtual cairo_t *createCairo() = 0;

	bool            m_bPreview;
	bool            m_bPrinter;
	bool m_double_buffered;
};


class ABI_EXPORT GR_CairoGraphics : public GR_Graphics
{
	friend class GR_UnixImage;

	// all constructors are protected; instances must be created via
	// GR_GraphicsFactory
public:
	virtual ~GR_CairoGraphics();

	virtual GR_Capability  getCapability() {return GRCAP_SCREEN_ONLY;}

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
	virtual void           clearFont(void) {m_pPFont = NULL;}

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

	PangoFontMap * getFontMap() const {return m_pFontMap;}
	PangoContext * getContext() const {return m_pContext;}
	PangoFontMap * getLayoutFontMap() const {return m_pLayoutFontMap;}
	PangoContext * getLayoutContext() const {return m_pLayoutContext;}

	virtual UT_uint32 getFontAscent();
	virtual UT_uint32 getFontDescent();
	virtual UT_uint32 getFontHeight();

	virtual UT_uint32 getFontAscent(const GR_Font *);
	virtual UT_uint32 getFontDescent(const GR_Font *);
	virtual UT_uint32 getFontHeight(const GR_Font *);

	virtual void		fillRect(GR_Color3D c,
								 UT_sint32 x, UT_sint32 y,
								 UT_sint32 w, UT_sint32 h);
	virtual void		fillRect(GR_Color3D c, UT_Rect &r);
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

	bool isDingbat(void) const {return m_bIsDingbat;}
	bool isSymbol(void) const {return m_bIsSymbol;};

	void resetFontMapResolution(void);

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
	virtual UT_uint32 getDeviceResolution(void) const;
	double    getResolutionRatio(void) const { return 1.0;}

	static  const std::vector<std::string> &       getAllFontNames(void);
	static  UT_uint32                         getAllFontCount();
	virtual GR_Font * getDefaultFont(GR_Font::FontFamilyEnum f = GR_Font::FF_Roman,
									 const char * pszLang = NULL);

	int dtpu(int d) const;
	int ptdu(int p) const;
	int ptlu(int p) const;
	int ptlunz(int p) const;
	int ltpu(int l) const;
	int ltpunz(int l) const;
	int pftlu(int pf) const;

	virtual bool		queryProperties(GR_Graphics::Properties gp) const;
//	virtual GR_Image*	createNewImage(const char* pszName,
//									   const UT_ByteBuf* pBB,
//									   UT_sint32 iDisplayWidth,
//									   UT_sint32 iDisplayHeight,
//									   GR_Image::GRType =GR_Image::GRT_Raster);

  	virtual bool		startPrint(void);
	virtual bool		endPrint(void);
	virtual bool		startPage(const char * szPageLabel,
								  UT_uint32 pageNumber,
								  bool bPortrait,
								  UT_uint32 iWidth, UT_uint32 iHeight);

	virtual void		setColorSpace(GR_Graphics::ColorSpace c);
	virtual GR_Graphics::ColorSpace getColorSpace(void) const;

	// virtual void		setCursor(GR_Graphics::Cursor c);
	virtual GR_Graphics::Cursor getCursor(void) const;

	virtual void		setColor3D(GR_Color3D c);
	virtual bool		getColor3D(GR_Color3D name, UT_RGBColor &color);

	// virtual void		scroll(UT_sint32, UT_sint32);
	// virtual void		scroll(UT_sint32 x_dest, UT_sint32 y_dest,
	//						   UT_sint32 x_src, UT_sint32 y_src,
	//						   UT_sint32 width, UT_sint32 height);

	virtual void	    	saveRectangle(UT_Rect & r, UT_uint32 iIndx);
	virtual void	    	restoreRectangle(UT_uint32 iIndx);
	// virtual GR_Image *  genImageFromRectangle(const UT_Rect & r);

	virtual void setLineProperties(double inWidth,
					 GR_Graphics::JoinStyle inJoinStyle = JOIN_MITER,
					 GR_Graphics::CapStyle inCapStyle   = CAP_BUTT,
					 GR_Graphics::LineStyle inLineStyle = LINE_SOLID);
	cairo_t* getCairo ();
	void setCairo(cairo_t *cr);

	static UT_uint32 getDefaultDeviceResolution();

  protected:
	// setup the graphics properties like color and clip if they have been set
	void _setProps();
	virtual void		_resetClip(void);

	// this is called to get the surface out of what is drawning.
	static cairo_surface_t * _getCairoSurfaceFromContext(cairo_t *cr,
												const cairo_rectangle_t & rect);
	static void _setSource(cairo_t *, const UT_RGBColor &);

	// all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
	GR_CairoGraphics(cairo_t *cr, UT_uint32 iDeviceResolution);
	GR_CairoGraphics();
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

	PangoFont *  _adjustedPangoFont (const GR_PangoFont * pFont, PangoFont * pf);
	PangoFont *  _adjustedLayoutPangoFont (const GR_PangoFont * pFont, PangoFont * pf);

	double            _tdudX(UT_sint32 layoutUnits) const;
	double            _tdudY(UT_sint32 layoutUnits) const;

	PangoFontMap *    m_pFontMap;
	PangoContext *    m_pContext;
	PangoFontMap *    m_pLayoutFontMap;
	PangoContext *    m_pLayoutContext;
	GR_PangoFont* m_pPFont;
	GR_PangoFont* m_pPFontGUI;

	// adjusted device front caching
	PangoFont *       m_pAdjustedPangoFont;
	PangoFontDescription* m_pAdjustedPangoFontDescription;
	UT_sint32         m_iAdjustedPangoFontSize;

	// adjusted layout font caching
	PangoFont *       m_pAdjustedLayoutPangoFont;
	PangoFontDescription* m_pAdjustedLayoutPangoFontDescription;
	UT_sint32         m_iAdjustedLayoutPangoFontSize;

	UT_uint32         m_iDeviceResolution;

	cairo_t	*         m_cr;

	GR_Graphics::Cursor	    m_cursor;
	GR_Graphics::ColorSpace	m_cs;

	UT_RGBColor		m_3dColors[COUNT_3D_COLORS];

	UT_RGBColor		m_curColor;
	bool            m_curColorDirty;
	bool            m_clipRectDirty;
	double                 m_lineWidth;
	GR_Graphics::JoinStyle m_joinStyle;
	GR_Graphics::CapStyle  m_capStyle;
	GR_Graphics::LineStyle m_lineStyle;
	bool            m_linePropsDirty;
	bool                    m_bIsSymbol;
	bool                    m_bIsDingbat;
	UT_sint32               m_iPrevX1;
	UT_sint32               m_iPrevX2;
	UT_sint32               m_iPrevY1;
	UT_sint32               m_iPrevY2;
	UT_uint32               m_iPrevRect;
	UT_sint32               m_iXORCount;

	/** init the cairo context once created */
	void _initCairo();

	// Double buffering implementation
	void _DeviceContext_SwitchToBuffer();
	void _DeviceContext_SwitchToScreen();

	// Suspend / resume drawing
	void _DeviceContext_SuspendDrawing();
	void _DeviceContext_ResumeDrawing();

	// save / restore rectangle vectors
	std::vector<UT_Rect*> m_vSaveRect;
	std::vector<cairo_surface_t*> m_vSaveRectBuf;

private:
	static UT_uint32 s_iInstanceCount;
	static UT_VersionInfo s_Version;
	static int s_iMaxScript;
	/** common private init for pango called from the constructor */
	void _initPango();
};


#endif
