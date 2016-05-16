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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef GR_WIN32GRAPHICS_H
#define GR_WIN32GRAPHICS_H

#include <windows.h>
#include "ut_misc.h"
#include "gr_Graphics.h"
#include "gr_Win32CharWidths.h"
#include "ut_vector.h"
#include "ut_stack.h"
#include <wchar.h>
#include <winuser.h>

class UT_ByteBuf;

#define _MAX_CACHE_PENS 64

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ABI_EXPORT GR_Win32Font : public GR_Font
{
public:
	static GR_Win32Font * newFont(LOGFONTW & lf, double fPoints, HDC hdc, HDC printDC);
	virtual ~GR_Win32Font();

	// need these to allow for adjustements in response to changes of device
	void        	setAscent(UT_uint32 n)  { m_tm.tmAscent = n; }
	void 	    setDescent(UT_uint32 n) { m_tm.tmDescent = n; }
	void         setHeight(UT_uint32 n)  { m_tm.tmHeight = n; }

	UT_uint32	getAscent(HDC hdc, HDC printHDC);
	UT_uint32	getDescent(HDC hdc, HDC printHDC);
	UT_uint32	getHeight(HDC hdc, HDC printHDC);
	UT_uint32   getUnscaledHeight() const { return m_iHeight;}

	HFONT       getDisplayFont(GR_Graphics * pGr);

	virtual UT_sint32 measureUnremappedCharForCache(UT_UCSChar cChar) const;
	UT_sint32	measureUnRemappedChar(UT_UCSChar c, UT_uint32 * height = 0);
	virtual GR_CharWidths* newFontWidths(void) const;
//
// UT_Rect of glyph in Logical units.
// rec.left = bearing Left (distance from origin to start)
// rec.width = width of the glyph
// rec.top = distance from the origin to the top of the glyph
// rec.height = total height of the glyph

	virtual bool glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG);

	void        selectFontIntoDC(GR_Graphics * pGr, HDC hdc);

	void        markGUIFont() {m_bGUIFont = true;}
	bool        isFontGUI() const {return m_bGUIFont;}

	HDC   getPrimaryHDC() const {return m_hdc;}
	HDC   getXHDC() const {return m_xhdc;}
	HDC   getYHDC() const {return m_yhdc;}

	void        setPrimaryHDC(HDC hdc) {m_hdc = hdc;}
	void        setXHDC(HDC hdc) {m_xhdc = hdc;}
	void        setYHDC(HDC hdc) {m_yhdc = hdc;}


	// NB: the font handle is one which was associated with this font when it was
	// origianlly created; however, it is not necessarily one that is to be used for
	// drawing as that has to reflect zoom factor and has to be obtained using
	// GR_Win32Font::Acq::getDisplayFont()
	// (The handle returned by getFontHandle() can be used for things that are not
	// affected by zoom, such as retrieving face names, etc.)
	HFONT       getFontHandle() const {return m_layoutFont;}
	double      getPointSize() const {return m_fPointSize;}

protected:
	// all construction has to be done via the graphics class
	GR_Win32Font(LOGFONTW & lf, double fPoints, HDC hdc, HDC printHDC);

	GR_Win32CharWidths * _getCharWidths() const
	{
#ifndef ABI_GRAPHICS_PLUGIN_NO_WIDTHS
		return reinterpret_cast<GR_Win32CharWidths *>(GR_Font::_getCharWidths());
#else
		UT_return_val_if_fail(UT_NOT_IMPLEMENTED,NULL);
#endif
	}

	// this function should clear any cached information the font might cary
	// it is prinicipally intened to be used when we share fonts between screen and
	// printer
	virtual void _clearAnyCachedInfo() {};
	void         _updateFontYMetrics(HDC hdc, HDC printHDC);


public:
	HFONT        getFontFromCache(UT_uint32 pixelsize, bool bIsLayout,
								  UT_uint32 zoomPercentage) const;

	void         fetchFont(UT_uint32 pixelsize) const;

	const TEXTMETRICW & getTextMetric() const {return m_tm;}

private:

	struct allocFont
	{
		UT_uint32			pixelSize;
		HFONT			    hFont;
	};

	void					insertFontInCache(UT_uint32 pixelsize, HFONT pFont) const;

	// we will store three different HDC values
	// m_hdc is handle to the device on which we are meant to draw
	// m_xhdc is handle to the device which was used for obtaining x-axis metrics
	// m_yhdc is handle to the device which was used for obtaining y-axis metrics
	// we have no control over the lifetime of any of
	// these dc's -- we only use these to check that the metrics and other font info is
	// uptodate -- they should NEVER be passed to any win32 API

	HDC				m_hdc;
	HDC				m_xhdc;
	HDC				m_yhdc;

	UT_uint32				m_defaultCharWidth;
	HFONT                   m_layoutFont;
	TEXTMETRICW				m_tm;
	UT_uint32		        m_iHeight; // unscaled height

	// a cache of 'allocFont *' at a given size
	mutable UT_Vector		m_allocFonts;
	bool                    m_bGUIFont;
	double                  m_fPointSize;
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ABI_EXPORT GR_Win32AllocInfo : public GR_AllocInfo
{
  public:
	GR_Win32AllocInfo():
		m_hdc(0), m_hwnd(0), m_pDocInfo(NULL), m_hDevMode(NULL) {};

	GR_Win32AllocInfo(HDC hdc, HWND hwnd):
		m_hdc(hdc), m_hwnd(hwnd), m_pDocInfo(NULL), m_hDevMode(NULL) {};

	GR_Win32AllocInfo(HDC hdc, const DOCINFOW* pDoc, HGLOBAL devmode):
		m_hdc(hdc), m_hwnd(0), m_pDocInfo(pDoc), m_hDevMode(devmode) {};

	virtual GR_GraphicsId getType() const {return GRID_WIN32;}
	virtual bool isPrinterGraphics() const {return (m_pDocInfo != 0);}

	HDC               m_hdc;
	HWND              m_hwnd;
	const DOCINFOW *   m_pDocInfo;
	HGLOBAL           m_hDevMode;
};


class ABI_EXPORT GR_Win32Graphics : public GR_Graphics
{
	// all constructors are protected; instances must be created via
	// GR_GraphicsFactory
public:
	virtual ~GR_Win32Graphics();

	static UT_uint32 s_getClassId() {return GRID_WIN32;}
	virtual UT_uint32 getClassId() {return s_getClassId();}

	virtual GR_Capability getCapability() {return GRCAP_SCREEN_AND_PRINTER;}

	static const char *    graphicsDescriptor(){return "Win32 Default";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);

	static  GR_Graphics *   getPrinterGraphics(const wchar_t * pPrinterName,
											   const wchar_t * pDocName);

	virtual void			drawGlyph(UT_uint32 glyph_idx, UT_sint32 xoff, UT_sint32 yoff);
	virtual void			drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff);
	virtual void			drawChars(const UT_UCSChar* pChars,
									  int iCharOffset, int iLength,
									  UT_sint32 xoff, UT_sint32 yoff,
									  int * pCharWidth);
	virtual void			setFont(const GR_Font* pFont);
	virtual void            clearFont(void) { m_pFont = NULL;}
	virtual UT_uint32		getFontHeight();
	virtual UT_sint32		measureUnRemappedChar(const UT_UCSChar c, UT_uint32 * height = 0);
	virtual void			setColor(const UT_RGBColor& clr);
	virtual void            getColor(UT_RGBColor& clr);
	virtual GR_Font*		getGUIFont();

	virtual UT_uint32		getFontAscent();
	virtual UT_uint32		getFontDescent();
	virtual void			getCoverage(UT_NumberVector& coverage);
	virtual void			drawLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void			xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void			setLineWidth(UT_sint32);

	virtual void            setLineProperties ( double inWidthPixels,
												JoinStyle inJoinStyle = JOIN_MITER,
												CapStyle inCapStyle   = CAP_BUTT,
												LineStyle inLineStyle = LINE_SOLID );

	virtual void			polyLine(const UT_Point * pts, UT_uint32 nPoints);
	virtual void			fillRect(const UT_RGBColor& c,
									 UT_sint32 x, UT_sint32 y,
									 UT_sint32 w, UT_sint32 h);
	virtual void			invertRect(const UT_Rect* pRect);
	virtual void			setClipRect(const UT_Rect* pRect);
	virtual void			scroll(UT_sint32 dx, UT_sint32 dy);
	virtual void			scroll(UT_sint32 x_dest, UT_sint32 y_dest,
								   UT_sint32 x_src, UT_sint32 y_src,
								   UT_sint32 width, UT_sint32 height);
	virtual void			clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32);

	virtual void			drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
	virtual GR_Image*		createNewImage(const char* pszName, const UT_ByteBuf* pBB, const std::string& mimetype,
						       UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType = GR_Image::GRT_Raster);

	virtual bool			queryProperties(GR_Graphics::Properties gp) const;

	virtual bool			startPrint(void);
	virtual bool			startPage(const char * szPageLabel, UT_uint32 pageNumber,
									  bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	virtual bool			endPrint(void);

	virtual HWND			getHwnd(void) const;

	virtual void			setColorSpace(GR_Graphics::ColorSpace c);
	virtual GR_Graphics::ColorSpace		getColorSpace(void) const;

	virtual void			setCursor(GR_Graphics::Cursor c);
	virtual GR_Graphics::Cursor			getCursor(void) const;
	virtual void			handleSetCursorMessage(void);

	virtual void			setColor3D(GR_Color3D c);
	virtual bool			getColor3D(GR_Color3D, UT_RGBColor &)
	{ return false; }
	void					init3dColors(void);
	virtual void			fillRect(GR_Color3D c,
									 UT_sint32 x, UT_sint32 y,
									 UT_sint32 w, UT_sint32 h);
	virtual void			fillRect(GR_Color3D c, UT_Rect &r);
    virtual void            polygon(const UT_RGBColor& c, const UT_Point *pts, UT_uint32 nPoints);
	virtual UT_uint32		getFontAscent(const GR_Font *);
	virtual UT_uint32		getFontDescent(const GR_Font *);
	virtual UT_uint32		getFontHeight(const GR_Font *);

    virtual GR_Image * genImageFromRectangle(const UT_Rect & r);
	virtual void		  saveRectangle(UT_Rect & r, UT_uint32 iIndx);
	virtual void		  restoreRectangle(UT_uint32 iIndx);
	virtual void 		  flush(void);
	void setBrush(HBRUSH hBrush){ m_hClearBrush = hBrush;};


	virtual void          setPrintDC(HDC dc);
	HDC                   getPrintDC() const {return m_printHDC;}
	HDC                   getPrimaryDC() const {return m_hdc;}

	void                  setPrintDCFontAllocNo(UT_uint32 i){m_iPrintDCFontAllocNo = i;}
	void                  setDCFontAllocNo(UT_uint32 i){m_iDCFontAllocNo = i;}

	double                getXYRatio() const {return m_fXYRatio;}
	double                getXYRatioPrint() const {return m_fXYRatioPrint;}

	static bool fixDevMode(HGLOBAL hModDev);

	static DOCINFOW *getDocInfo();
	static HDC createbestmetafilehdc();

protected:
	// all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
	GR_Win32Graphics(HDC, HWND);					/* for screen */
	GR_Win32Graphics(HDC, const DOCINFOW *, HGLOBAL hDevMode = NULL);	/* for printing */

	BITMAPINFO * ConvertDDBToDIB(HBITMAP bitmap, HPALETTE hPal, DWORD dwCompression);

	virtual GR_Font*		_findFont(const char* pszFontFamily,
									  const char* pszFontStyle,
									  const char* pszFontVariant,
									  const char* pszFontWeight,
									  const char* pszFontStretch,
									  const char* pszFontSize,
									  const char* pszLang);

	virtual UT_uint32 	getDeviceResolution(void) const;
	void					_setColor(DWORD clrRef);

	HDC m_bufferHdc;
	HDC m_dummyHdc;

	void _DeviceContext_MeasureBitBltCopySpeed(HDC source, HDC dest, int width, int height);
	void getWidthAndHeightFromHWND(HWND h, int &width, int &height);
	void _DeviceContext_SwitchToBuffer();
	void _DeviceContext_SwitchToScreen();
	void _DeviceContext_DrawBufferToScreen();

	void _DeviceContext_SuspendDrawing();
	void _DeviceContext_ResumeDrawing();

	void _DoubleBuffering_SetUpDummyBuffer();
	void _DoubleBuffering_ReleaseDummyBuffer();

	HDC _DoubleBuffering_CreateBuffer(HDC, int, int);
	void _DoubleBuffering_ReleaseBuffer(HDC);

	struct _HDCSwitchRecord
	{
		HDC oldHdc;
		_HDCSwitchRecord(HDC h) : oldHdc(h) { }
	};

	UT_Stack _HDCSwitchStack;

	void _DeviceContext_RestorePrevHDCFromStack();

private:
	virtual GR_Win32Font * _newFont(LOGFONTW & lf, double fPointSize, HDC hdc, HDC printDC);

  protected:

	UT_uint32               m_iDCFontAllocNo;
	UT_uint32               m_iPrintDCFontAllocNo;
	HDC						m_hdc;
	HDC                     m_printHDC;
	static HDC				m_defPrintHDC;
	static UT_uint32		s_iInstanceCount;
	HWND 					m_hwnd;
	const DOCINFOW *			m_pDocInfo;
	bool					m_bPrint;
	bool					m_bStartPrint;
	bool					m_bStartPage;
	GR_Win32Font*			m_pFont;
	GR_Win32Font*			m_pFontGUI;
	UT_sint32				m_iLineWidth;
    JoinStyle               m_eJoinStyle;
	CapStyle                m_eCapStyle;
	LineStyle               m_eLineStyle;

	GR_Graphics::ColorSpace m_cs;
	GR_Graphics::Cursor		m_cursor;

	DWORD					m_clrCurrent;
	DWORD					m_3dColors[COUNT_3D_COLORS];
	int                     m_nPrintLogPixelsY;
	double                  m_fXYRatio;
	double                  m_fXYRatioPrint;


private:
	void 					_constructorCommonCode(HDC);
	UT_uint16*				_remapGlyphs(const UT_UCSChar* pChars, int iCharOffset, int &iLength);
	virtual bool            _setTransform(const GR_Transform & tr);

	DWORD					m_clrXorPen;
	HPEN					m_hXorPen;

	UT_UCS2Char*			m_remapBuffer;
	UT_uint32				m_remapBufferSize;
	UT_UCS2Char*			m_remapIndices;

	UT_RGBColor				m_curColor;
	UT_Vector				m_vSaveRect;
	UT_Vector 				m_vSaveRectBuf;
	HBRUSH					m_hClearBrush;
	int						m_nLogPixelsY;
	HGLOBAL					m_hDevMode;

	typedef struct
	{
		HPEN 	hPen;
		int	 	nStyle;
		int 	nWidth;
		DWORD	dwColour;

	} CACHE_PEN;

	CACHE_PEN*				   m_pArPens;
	int						   m_nArPenPos;
	bool m_bIsPreview;
};

#endif /* GR_WIN32GRAPHICS_H */
