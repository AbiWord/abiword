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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
	static  GR_Win32USPFont * newFont(LOGFONTW & lf, double fPoints, HDC hdc, HDC printHDC);
	virtual ~GR_Win32USPFont();

	SCRIPT_CACHE * getScriptCache() {return &m_sc;}
	HDC            getPrintDC() const {return m_printHDC;}
	void           setPrintDC(HDC dc) {m_printHDC = dc;}

	UT_sint32      getScreenAscent() const {return m_iScreenAscent;}
	void           setScreenAscent(UT_sint32 iA) {m_iScreenAscent = iA;}

  protected:
	// all construction has to be done via the graphics class
	GR_Win32USPFont(LOGFONTW & lf, double fPoints, HDC hdc, HDC printHDC);

	virtual void _clearAnyCachedInfo();

  private:
	SCRIPT_CACHE m_sc;
	HDC          m_printHDC;
	UT_sint32    m_iScreenAscent;
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
	static const char *    graphicsDescriptor();
	static const char *    getUSPVersion();
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);

	virtual void		   setFont(const GR_Font* pFont);

	virtual void		   drawChars(const UT_UCSChar* pChars,
									  int iCharOffset, int iLength,
									  UT_sint32 xoff, UT_sint32 yoff,
									  int * pCharWidth);

	virtual void           drawCharsRelativeToBaseline(const UT_UCSChar* pChars,
													   int iCharOffset,
													   int iLength,
													   UT_sint32 xoff,
													   UT_sint32 yoff,
													   int* pCharWidths = NULL);

	virtual UT_uint32		getFontHeight();
	virtual UT_uint32		getFontAscent();
	virtual UT_uint32		getFontDescent();
	virtual UT_uint32		getFontAscent(GR_Font *);
	virtual UT_uint32		getFontDescent(GR_Font *);
	virtual UT_uint32		getFontHeight(GR_Font *);

	virtual void      setZoomPercentage(UT_uint32 iZoom);

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

	virtual bool      needsSpecialCaretPositioning(GR_RenderInfo & ri);
	virtual UT_uint32 adjustCaretPosition(GR_RenderInfo & ri, bool bForward);
	virtual void      adjustDeletePosition(GR_RenderInfo & ri);

  protected:
	inline bool       _needsSpecialBreaking(GR_Win32USPRenderInfo &ri);
	inline bool       _needsSpecialCaretPositioning(GR_Win32USPRenderInfo &ri);
	inline bool       _scriptBreak(GR_Win32USPRenderInfo &ri);

  public:
	virtual const UT_VersionInfo & getVersion() const {return s_Version;}

	virtual void          setPrintDC(HDC dc);


  protected:
	// all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
	GR_Win32USPGraphics(HDC, HWND);
	GR_Win32USPGraphics(HDC, const DOCINFOW *, HGLOBAL hDevMode = NULL);

  private:
	bool      _constructorCommonCode();
	virtual GR_Win32Font * _newFont(LOGFONTW & lf, double fPoints, HDC hdc, HDC printHDC);

	void   _setupFontOnDC(GR_Win32USPFont *pFont, bool bZoomMe);

	void   _drawChars(const UT_UCSChar* pChars,
					  int iCharOffset, int iLength,
					  UT_sint32 xoff, UT_sint32 yoff,
					  int * pCharWidth);

	bool   m_bConstructorSucceeded;

	static HINSTANCE s_hUniscribe;
	static UT_uint32 s_iInstanceCount;
	static UT_VersionInfo s_Version;
	static const SCRIPT_PROPERTIES **s_ppScriptProperties;
	static int s_iMaxScript;
	static UT_UTF8String s_sDescription;
	static UT_UTF8String s_sUSPVersion;

  protected:
	// These are the Uniscribe functions we load from the DLL.  We
	// prefix the names with 'f' to avoid clashes with the function
	// names defined in usp10.h.
	//
	// IMPORTANT: All uniscribe functions must be called via these
	// pointers since we do not want to link against usp10.dll !!!

	static tScriptItemize       fScriptItemize;
	static tScriptShape         fScriptShape;
	static tScriptStringOut     fScriptStringOut;
	static tScriptStringAnalyse fScriptStringAnalyse;
	static tScriptStringFree    fScriptStringFree;
	static tScriptTextOut       fScriptTextOut;
	static tScriptPlace         fScriptPlace;
	static tScriptJustify       fScriptJustify;
	static tScriptCPtoX         fScriptCPtoX;
	static tScriptXtoCP         fScriptXtoCP;
	static tScriptBreak         fScriptBreak;
	static tScriptIsComplex     fScriptIsComplex;
	static tScriptGetProperties fScriptGetProperties;
	static tScriptRecordDigitSubstitution fScriptRecordDigitSubstitution;
  public:
	// these need to be public so we can g_free various things ...
	static tScriptFreeCache   fScriptFreeCache;
};

#endif
