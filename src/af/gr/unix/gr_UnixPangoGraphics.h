/* AbiWord
 * Copyright (C) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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
					 GR_UnixPangoGraphics * pG, bool bGuiFont = false);
	
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
	
  private:
	UT_String  m_sDesc;
	double     m_dPointSize;
	UT_uint32  m_iZoom;
	PangoFont *m_pf;
	bool       m_bGuiFont;
	PangoCoverage * m_pCover;
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
                    
	virtual GR_Font*	   getDefaultFont(UT_String& fontFamily);
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
										const char* pszFontSize);

	virtual GR_Font* _findFont(const char* pszFontFamily,
							   const char* pszFontStyle,
							   const char* pszFontVariant,
							   const char* pszFontWeight,
							   const char* pszFontStretch,
							   const char* pszFontSize);
	
	virtual void getCoverage(UT_NumberVector& coverage);
	
  protected:
	// all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
	GR_UnixPangoGraphics(GdkWindow * win, XAP_UnixFontManager * fontManager, XAP_App *app);
	GR_UnixPangoGraphics(GdkPixmap * win, XAP_UnixFontManager * fontManager, XAP_App *app, bool bUsePixmap);

	inline bool _scriptBreak(GR_UnixPangoRenderInfo &ri);

	void _scaleCharacterMetrics(GR_UnixPangoRenderInfo & RI);
	void _scaleJustification(GR_UnixPangoRenderInfo & RI);
	
	int _dtpu(int d) const;
	int _ptdu(int p) const;
	int _ptlu(int p) const;
	int _ltpu(int l) const;
	int _pftlu(int pf) const;
	
  private:
	static UT_uint32 s_iInstanceCount;
	static UT_VersionInfo s_Version;
	static int s_iMaxScript;

	PangoFontMap * m_pFontMap;
	PangoContext * m_pContext;

	GR_UnixPangoFont* m_pPFont;
	GR_UnixPangoFont* m_pPFontGUI;
};

#endif
