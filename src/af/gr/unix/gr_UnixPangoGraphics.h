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

//class ABI_EXPORT GR_UnixPangoFont : public GR_UnixFont
class ABI_EXPORT GR_UnixPangoFont : public GR_Font
{
  public:
	//GR_UnixPangoFont(LOGFONT & lf): GR_UnixFont(lf), m_sc(NULL){};
	GR_UnixPangoFont(PangoFont *pf) : m_pf(pf) {};
	virtual ~GR_UnixPangoFont();
	PangoFont *m_pf;

	//SCRIPT_CACHE * getScriptCache() {return &m_sc;}
  //private:
	//SCRIPT_CACHE m_sc;
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
	
	//virtual GR_Capability  getCapability() {return GRCAP_SCREEN_AND_PRINTER;}
	virtual GR_Capability  getCapability() {return GRCAP_SCREEN_ONLY;}
	static const char *    graphicsDescriptor(){return "Unix Pango";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);

	virtual void			drawChars(const UT_UCSChar* pChars,
									  int iCharOffset, int iLength,
									  UT_sint32 xoff, UT_sint32 yoff,
									  int * pCharWidth);

	///////////////////////////////////////////////////////////////////
	// complex script processing
	//
	virtual bool itemize(UT_TextIterator & text, GR_Itemization & I);
	virtual bool shape(GR_ShapingInfo & si, GR_RenderInfo *& ri);
	virtual void prepareToRenderChars(GR_RenderInfo & ri);
	virtual void renderChars(GR_RenderInfo & ri);
	virtual void measureRenderedCharWidths(GR_RenderInfo & ri);
	virtual void appendRenderedCharsToBuff(GR_RenderInfo & ri, UT_GrowBuf & buf) const;
#if 0
	virtual bool canBreak(GR_RenderInfo & ri, UT_sint32 &iNext);

	virtual UT_sint32 resetJustification(GR_RenderInfo & ri, bool bPermanent);
	virtual UT_sint32 countJustificationPoints(const GR_RenderInfo & ri) const;
	virtual void      justify(GR_RenderInfo & ri);

    	virtual UT_uint32 XYToPosition(const GR_RenderInfo & ri, UT_sint32 x, UT_sint32 y) const;
    	virtual void      positionToXY(const GR_RenderInfo & ri,
								   UT_sint32& x, UT_sint32& y,
								   UT_sint32& x2, UT_sint32& y2,
								   UT_sint32& height, bool& bDirection) const;
	
	virtual UT_sint32 getTextWidth(const GR_RenderInfo & ri) const;

  protected:
	inline bool       _needsSpecialBreaking(GR_UnixPangoRenderInfo &ri);
	inline bool       _needsSpecialCaretPositioning(GR_UnixPangoRenderInfo &ri);
	inline bool       _scriptBreak(GR_UnixPangoRenderInfo &ri);
#endif
  public:
	virtual const UT_VersionInfo & getVersion() const {return s_Version;}
	
  protected:
	// all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
	GR_UnixPangoGraphics(GdkWindow * win, XAP_UnixFontManager * fontManager, XAP_App *app);
	GR_UnixPangoGraphics(GdkPixmap * win, XAP_UnixFontManager * fontManager, XAP_App *app, bool bUsePixmap);

  private:
	//bool      _constructorCommonCode();
	//virtual GR_UnixFont * _newFont(LOGFONT & lf){return new GR_UnixPangoFont(lf);}
	
	//static HINSTANCE s_hUniscribe;
	static UT_uint32 s_iInstanceCount;
	static UT_VersionInfo s_Version;
	//static const SCRIPT_PROPERTIES **s_ppScriptProperties;
	static int s_iMaxScript;
	
};

#endif
