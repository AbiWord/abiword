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

#ifndef GR_WIN32_GRAPHICS_H
#define GR_WIN32_GRAPHICS_H

#include "ut_types.h"
#include "gr_Win32Graphics.h"
#include "gr_RenderInfo.h"
#include "gr_Win32Uniscribe.h"

// we do not want this to be a plugin for now
#define GR_WIN32USP_BUILTIN

#ifndef GR_WIN32USP_BUILTIN
#define PLUGIN_NAME "Uniscribe graphics class for Win32"
#endif

/************************************************************************/
/************************************************************************/

class ABI_EXPORT GR_Win32USPFont : public GR_Win32Font
{
  public:
	GR_Win32USPFont(LOGFONT & lf): GR_Win32Font(lf), m_sc(NULL){};
	virtual ~GR_Win32USPFont();

	SCRIPT_CACHE * getScriptCache() {return &m_sc;}
  private:
	SCRIPT_CACHE m_sc;
};

class GR_Win32USPRenderInfo;

class ABI_EXPORT GR_Win32USPGraphics : public GR_Win32Graphics
{
	// all constructors are protected; instances must be created via
	// GR_GraphicsFactory
public:
	virtual ~GR_Win32USPGraphics();

	static UT_uint32 s_getClassId() {return GRID_WIN32_UNISCRIBE;}
	virtual UT_uint32 getClassId() {return s_getClassId();}
	
	virtual GR_Capability  getCapability() {return GRCAP_SCREEN_AND_PRINTER;}
	static const char *    graphicsDescriptor(){return "Win32 Uniscribe";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);

	///////////////////////////////////////////////////////////////////
	// complex script processing
	//
	virtual bool itemize(UT_TextIterator & text, GR_Itemization & I);
	virtual bool shape(GR_ShapingInfo & si, GR_RenderInfo *& ri);
	virtual void prepareToRenderChars(GR_RenderInfo & ri);
	virtual void renderChars(GR_RenderInfo & ri);
	virtual void measureRenderedCharWidths(GR_RenderInfo & ri);
	virtual void appendRenderedCharsToBuff(GR_RenderInfo & ri, UT_GrowBuf & buf) const;

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
	inline bool       _needsSpecialBreaking(GR_Win32USPRenderInfo &ri);
	inline bool       _needsSpecialCaretPositioning(GR_Win32USPRenderInfo &ri);
	inline bool       _scriptBreak(GR_Win32USPRenderInfo &ri);

  public:
	virtual const UT_VersionInfo & getVersion() const {return s_Version;}
	
  protected:
	// all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
	GR_Win32USPGraphics(HDC, HWND, XAP_App *);
	GR_Win32USPGraphics(HDC, const DOCINFO *, XAP_App *, HGLOBAL hDevMode = NULL);
	
  private:
	bool      _constructorCommonCode();
	virtual GR_Win32Font * _newFont(LOGFONT & lf){return new GR_Win32USPFont(lf);}
	
	static HINSTANCE s_hUniscribe;
	static UT_uint32 s_iInstanceCount;
	static UT_VersionInfo s_Version;
	static const SCRIPT_PROPERTIES **s_ppScriptProperties;
	static int s_iMaxScript;
	

  protected:
	// these are the Uniscribe functions we load from the DLL
	static tScriptItemize       ScriptItemize;
	static tScriptShape         ScriptShape;
	static tScriptStringOut     ScriptStringOut;
	static tScriptStringAnalyse ScriptStringAnalyse;
	static tScriptStringFree    ScriptStringFree;
	static tScriptTextOut       ScriptTextOut;
	static tScriptPlace         ScriptPlace;
	static tScriptJustify       ScriptJustify;
	static tScriptCPtoX         ScriptCPtoX;
	static tScriptXtoCP         ScriptXtoCP;
	static tScriptBreak         ScriptBreak;
	static tScriptIsComplex     ScriptIsComplex;
	static tScriptGetProperties ScriptGetProperties;
	static tScriptRecordDigitSubstitution ScriptRecordDigitSubstitution;
  public:
	// these need to be public so we can free various things ...
	static tScriptFreeCache   ScriptFreeCache;
};

#endif
