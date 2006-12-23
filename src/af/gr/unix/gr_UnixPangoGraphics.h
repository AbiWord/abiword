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

#ifndef GR_UNIX_PANGOGRAPHICS_H
#define GR_UNIX_PANGOGRAPHICS_H

#include "ut_types.h"
#include "gr_UnixGraphics.h"
#include "gr_RenderInfo.h"

#include <pango/pango-font.h>

#ifndef WITHOUT_PRINTING
#include "xap_UnixGnomePrintGraphics.h"
#endif

// we do not want this to be a plugin for now
#define GR_UNIXPANGO_BUILTIN

#ifndef GR_UNIXPANGO_BUILTIN
#define PLUGIN_NAME "Pango graphics class for Unix"
#endif

/************************************************************************/
/************************************************************************/

class GR_UnixPangoRenderInfo;
class GR_UnixPangoGraphics;

class ABI_EXPORT GR_UnixPangoFont : public GR_Font
{
  public:
	GR_UnixPangoFont(const char * pDesc, double dSize,
					 GR_UnixPangoGraphics * pG,
					 const char * pLang,
					 bool bGuiFont = false);
	
	virtual ~GR_UnixPangoFont();

	/*!
		Measure the unremapped char to be put into the cache.
		That means measuring it for a font size of 120
	 */
	virtual UT_sint32 measureUnremappedCharForCache(UT_UCS4Char cChar) const;
	
	virtual bool      doesGlyphExist(UT_UCS4Char g);

	virtual bool      glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG);
	
	PangoFont *       getPangoFont() const {return m_pf;}

	void              reloadFont(GR_UnixPangoGraphics * pG);
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

class GR_UnixPangoRenderInfo;

class ABI_EXPORT GR_UnixPangoGraphics : public GR_UnixGraphics
{
	// all constructors are protected; instances must be created via
	// GR_GraphicsFactory
public:
	virtual ~GR_UnixPangoGraphics();

	static UT_uint32 s_getClassId() {return GRID_UNIX_PANGO;}
	virtual UT_uint32 getClassId() {return s_getClassId();}
	
	virtual GR_Capability  getCapability() {return GRCAP_SCREEN_ONLY;}
	static const char *    graphicsDescriptor(){return "Unix Pango";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);

	virtual UT_sint32      measureUnRemappedChar(const UT_UCSChar c);
	
	virtual void		   drawChars(const UT_UCSChar* pChars,
									  int iCharOffset, int iLength,
									  UT_sint32 xoff, UT_sint32 yoff,
									  int * pCharWidth);
                    
	virtual UT_uint32      measureString(const UT_UCSChar* s, int iOffset,
										 int num,  UT_GrowBufElement* pWidths);
	
	virtual GR_Font*	   getDefaultFont(UT_String& fontFamily,
										  const char * pszLang);
	virtual void           setFont(GR_Font *);

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

	virtual GR_Font * getGUIFont(void);
	
	PangoFontMap * getFontMap() {return m_pFontMap;}
	PangoContext * getContext() {return m_pContext;}

	virtual UT_uint32 getFontAscent();
	virtual UT_uint32 getFontDescent();
	virtual UT_uint32 getFontHeight();

	virtual UT_uint32 getFontAscent(GR_Font *);
	virtual UT_uint32 getFontDescent(GR_Font *);
	virtual UT_uint32 getFontHeight(GR_Font *);

	virtual const char* findNearestFont(const char* pszFontFamily,
										const char* pszFontStyle,
										const char* pszFontVariant,
										const char* pszFontWeight,
										const char* pszFontStretch,
										const char* pszFontSize,
										const char* pszLang);

	virtual GR_Font* _findFont(const char* pszFontFamily,
							   const char* pszFontStyle,
							   const char* pszFontVariant,
							   const char* pszFontWeight,
							   const char* pszFontStretch,
							   const char* pszFontSize,
							   const char* pszLang);
	
	virtual void getCoverage(UT_NumberVector& coverage);

	virtual UT_uint32 getDeviceResolution(void) const;

	static  UT_GenericVector<const char*> *   getAllFontNames(void);
	static  UT_uint32                         getAllFontCount();
	virtual GR_Font * getDefaultFont(GR_Font::FontFamilyEnum f = GR_Font::FF_Roman,
									 const char * pszLang = NULL);

	int dtpu(int d) const;
	int ptdu(int p) const;
	int ptlu(int p) const;
	int ltpu(int l) const;
	int pftlu(int pf) const;
	
  protected:
	// all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
	GR_UnixPangoGraphics(GdkWindow * win);
	GR_UnixPangoGraphics();

	inline bool _scriptBreak(GR_UnixPangoRenderInfo &ri);

	void _scaleCharacterMetrics(GR_UnixPangoRenderInfo & RI);
	void _scaleJustification(GR_UnixPangoRenderInfo & RI);

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
	
  protected:
	PangoFontMap *    m_pFontMap;
	PangoContext *    m_pContext;
	bool              m_bOwnsFontMap;
	GR_UnixPangoFont* m_pPFont;
	GR_UnixPangoFont* m_pPFontGUI;

	UT_uint32         m_iDeviceResolution;

	void init();

private:
	static UT_uint32 s_iInstanceCount;
	static UT_VersionInfo s_Version;
	static int s_iMaxScript;
};


#ifndef WITHOUT_PRINTING
class XAP_UnixGnomePrintGraphics;

/*!
    When printing, we need to combine pango with GnomePrint; we could do that in a single
    graphics class, but that would mean if(print) test inside each function. In order to
    avoid slowing the screen operations, we will use a derrived class.
*/
class ABI_EXPORT GR_UnixPangoPrintGraphics : public GR_UnixPangoGraphics
{
  public:
	
	GR_UnixPangoPrintGraphics(XAP_UnixGnomePrintGraphics * pGPG);

	virtual ~GR_UnixPangoPrintGraphics();

	static UT_uint32 s_getClassId() {return GRID_UNIX_PANGO_PRINT;}
	virtual UT_uint32 getClassId() {return s_getClassId();}
	
	virtual GR_Capability  getCapability() {return GRCAP_PRINTER_ONLY;}
	static const char *    graphicsDescriptor(){return "Unix Pango Print";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);

	GnomePrintContext *    getGnomePrintContext() const;
	UT_sint32              scale_ydir (UT_sint32 in) const;
	UT_sint32              scale_xdir (UT_sint32 in) const;
	virtual void setColor(const UT_RGBColor& clr);
	virtual void getColor(UT_RGBColor& clr);
	
	virtual void drawChars(const UT_UCSChar* pChars, 
						   int iCharOffset, int iLength,
						   UT_sint32 xoff, UT_sint32 yoff,
						   int * pCharWidths = NULL);
	
	virtual bool shape(GR_ShapingInfo & si, GR_RenderInfo *& ri);
	virtual void renderChars(GR_RenderInfo & ri);

	virtual void drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2);
	virtual void setLineWidth(UT_sint32);
	virtual void setLineProperties ( double inWidthPixels,
									 JoinStyle inJoinStyle,
									 CapStyle inCapStyle,
									 LineStyle inLineStyle);

	virtual GR_Font* getGUIFont();
	virtual void xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void polyLine(UT_Point * pts, UT_uint32 nPoints);
	virtual void fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void invertRect(const UT_Rect*);
	virtual void setClipRect(const UT_Rect*);
	virtual void scroll(UT_sint32, UT_sint32);
	virtual void scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height);
	virtual void clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32);

	virtual void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
   	virtual GR_Image* createNewImage(const char* pszName, const UT_ByteBuf* pBBPNG, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType);
	
	virtual bool queryProperties(GR_Graphics::Properties gp) const;
	
	virtual bool startPrint(void);
	virtual bool startPage(const char * szPagelabel, UT_uint32 pageNumber,
							  bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	virtual bool endPrint(void);

	virtual void setColorSpace(GR_Graphics::ColorSpace c);
	virtual GR_Graphics::ColorSpace getColorSpace(void) const;
	
	virtual void setCursor(GR_Graphics::Cursor c);
	virtual GR_Graphics::Cursor getCursor(void) const;

	virtual void					setColor3D(GR_Color3D c);
	virtual UT_RGBColor *			getColor3D(GR_Color3D c);
	virtual void fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void fillRect(GR_Color3D c, UT_Rect &r);
	virtual void setPageSize(char* pageSizeName, UT_uint32 iwidth = 0, UT_uint32 iheight=0);

    virtual GR_Image * genImageFromRectangle(const UT_Rect & r) { return NULL;}
	virtual void	  saveRectangle(UT_Rect & r, UT_uint32 iIndx) {}
	virtual void	  restoreRectangle(UT_uint32 iIndx) {}

	virtual UT_uint32 getDeviceResolution(void) const;
	virtual bool      canQuickPrint(void)
	{ return true;}
  protected:

	
	XAP_UnixGnomePrintGraphics * m_pGnomePrint;
	
	PangoFontMap *    m_pGPFontMap;
	PangoContext *    m_pGPContext;
	UT_uint32         m_iScreenResolution;
};
#endif // ifndef WHITOUT_PRINTING

#endif
