/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* Abiword
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

#ifndef GR_UNIXGRAPHICS_H
#define GR_UNIXGRAPHICS_H


#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "xap_UnixApp.h"
#include "xap_UnixFont.h"
#include "xap_Frame.h"
#include "gr_Graphics.h"
#include "ut_vector.h"

#include <X11/Xft/Xft.h>

class UT_ByteBuf;
class UT_String;
class UT_Wctomb;

class XAP_UnixFontManager;
class XAP_UnixGnomePrintGraphics;

UT_uint32 adobeToUnicode(UT_uint32 iAdobe);

UT_uint32 adobeDingbatsToUnicode(UT_uint32 iAdobe);

class GR_UnixAllocInfo : public GR_AllocInfo
{
public:
 	GR_UnixAllocInfo(GdkWindow * win, XAP_UnixFontManager * fontManager)
		: m_win(win),m_pixmap(NULL),m_fontManager(fontManager),
		  m_usePixmap(false), m_pGnomePrint(NULL) {};

	GR_UnixAllocInfo(GdkPixmap * win, XAP_UnixFontManager * fontManager,
					 bool bUsePixmap)
		: m_win(NULL), m_pixmap(win), m_fontManager(fontManager),
		  m_usePixmap(bUsePixmap), m_pGnomePrint(NULL) {};

	GR_UnixAllocInfo(XAP_UnixGnomePrintGraphics * pGPG)
		: m_win(NULL), m_pixmap(NULL), m_fontManager(NULL),
		  m_usePixmap(false), m_pGnomePrint(pGPG) {};

	virtual GR_GraphicsId getType() const {return GRID_UNIX;};
	virtual bool isPrinterGraphics() const {return false; };

	GdkWindow* m_win;
	GdkPixmap* m_pixmap;
	XAP_UnixFontManager * m_fontManager;
	bool m_usePixmap;
	XAP_UnixGnomePrintGraphics * m_pGnomePrint;
};

class GR_UnixGraphics : public GR_Graphics
{
	friend class GR_UnixImage;

	// all constructors are protected; instances must be created via
	// GR_GraphicsFactory
 public:
	virtual ~GR_UnixGraphics();

	static UT_uint32 s_getClassId() {return GRID_UNIX;}
	virtual UT_uint32 getClassId() {return s_getClassId();}
	
	virtual GR_Capability getCapability(){UT_ASSERT(UT_NOT_IMPLEMENTED); return GRCAP_UNKNOWN;}
	static const char *    graphicsDescriptor(void){return "Unix Default";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo& allocInfo);
	
	virtual void		setFont(GR_Font* pFont);
	virtual void        clearFont(void) {m_pFont = NULL;} 
	virtual UT_uint32	getFontHeight();

	virtual const char* findNearestFont(const char* pszFontFamily,
										const char* pszFontStyle,
										const char* pszFontVariant,
										const char* pszFontWeight,
										const char* pszFontStretch,
										const char* pszFontSize,
										const char* pszLang);

	// virtual UT_uint32	measureString(const UT_UCSChar*s, int iOffset, int num, unsigned short* pWidths);
	virtual UT_sint32 measureUnRemappedChar(const UT_UCSChar c);

	virtual void		setColor(const UT_RGBColor& clr);
	virtual void        getColor(UT_RGBColor &clr);

	virtual GR_Font*	getGUIFont();

	virtual GR_Font*	getDefaultFont(UT_String& fontFamily);
	virtual GR_Font *   getDefaultFont(GR_Font::FontFamilyEnum f = GR_Font::FF_Roman);

	virtual UT_uint32	getFontAscent();
	virtual UT_uint32	getFontDescent();

	virtual void		getCoverage(UT_NumberVector& coverage);

	virtual void		setLineWidth(UT_sint32);

	virtual void		setClipRect(const UT_Rect* pRect);
	virtual void		scroll(UT_sint32, UT_sint32);
	virtual void		scroll(UT_sint32 x_dest, UT_sint32 y_dest,
							   UT_sint32 x_src, UT_sint32 y_src,
							   UT_sint32 width, UT_sint32 height);

	virtual GR_Image*	createNewImage(const char* pszName, const UT_ByteBuf* pBB,
					       UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight,
					       GR_Image::GRType = GR_Image::GRT_Raster);
  
	virtual bool		queryProperties(GR_Graphics::Properties gp) const;
	virtual bool		startPrint(void);
	virtual bool		startPage(const char * szPageLabel, UT_uint32 pageNumber,
								  bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);

	virtual void      setZoomPercentage(UT_uint32 iZoom);
	virtual bool		endPrint(void);

	virtual void		flush(void);

	virtual void		setColorSpace(GR_Graphics::ColorSpace c);
	virtual GR_Graphics::ColorSpace getColorSpace(void) const;
	
	virtual void		setCursor(GR_Graphics::Cursor c);
	virtual GR_Graphics::Cursor getCursor(void) const;

	virtual void		setColor3D(GR_Color3D c);
	virtual bool		getColor3D(GR_Color3D name, UT_RGBColor &color);
	void				init3dColors(GtkStyle * pStyle);

	void                createPixmapFromXPM( char ** pXPM,GdkPixmap *source,
											 GdkBitmap * mask);
	GdkWindow*  	  	getWindow(void) const
	{ return m_pWin;}


	virtual void setLineProperties ( double inWidth, 
					 GR_Graphics::JoinStyle inJoinStyle = JOIN_MITER,
					 GR_Graphics::CapStyle inCapStyle   = CAP_BUTT,
					 GR_Graphics::LineStyle inLineStyle = LINE_SOLID ) ;

	/* GR_Font versions of the above -- TODO: should I add drawChar* methods too? */
	virtual UT_uint32 getFontAscent(GR_Font *);
	virtual UT_uint32 getFontDescent(GR_Font *);
	virtual UT_uint32 getFontHeight(GR_Font *);
    virtual GR_Image * genImageFromRectangle(const UT_Rect & r);
	virtual void	  saveRectangle(UT_Rect & r, UT_uint32 iIndx);
	virtual void	  restoreRectangle(UT_uint32 iIndx);
	
	virtual UT_uint32 	getDeviceResolution(void) const;

 protected:
	// all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
 	GR_UnixGraphics(GdkWindow * win, XAP_UnixFontManager * fontManager);
	GR_UnixGraphics(GdkPixmap * win, XAP_UnixFontManager * fontManager, bool bUsePixmap);


	virtual void _beginPaint ();
	virtual void _endPaint ();

	virtual void        drawGlyph(UT_uint32 glyph_idx, UT_sint32 xoff, UT_sint32 yoff);
	virtual void		drawChars(const UT_UCSChar* pChars, int iCharOffset,
								  int iLength, UT_sint32 xoff, UT_sint32 yoff,
								  int * pCharWidths = NULL);
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
	bool                isDingbat(void) const;
	bool                isSymbol(void) const;

	virtual GR_Font*	_findFont(const char* pszFontFamily, 
								  const char* pszFontStyle, 
								  const char* pszFontVariant, 
								  const char* pszFontWeight, 
								  const char* pszFontStretch, 
								  const char* pszFontSize,
								  const char* pszLang);

	void				_setColor(GdkColor & c);

	XAP_UnixFontManager * 	m_pFontManager;

	void                _setIsSymbol(bool b) {m_bIsSymbol = b;}
	void                _setIsDingbat(bool b) {m_bIsDingbat = b;}

	void                _setDeviceResolution(void);

	GdkGC*       			m_pGC;
	GdkGC*  	      		m_pXORGC;
	GdkWindow*  	  		m_pWin;

  private: // font stuff should be private so that derrived classes do not get easily
		   // mixed up if using different fonts
	XAP_UnixFontHandle *	m_pFont;

	// make this just a GR_Font pointer so that derrived classes can set it (derrived
	// classes might use font class not derrived from XAP_UnixFontHandle)
	GR_Font *	            m_pFontGUI;

	// Current GDK fonts corresponding to this. Calling m_pFont->explodeGdkFont
	// causes gdk_font_load to be called and memory to be allocated. This should
	// not happen on every draw
	GdkFont * m_pSingleByteFont, * m_pMultiByteFont;

	// our "OEM" system font, like a 10 point Helvetica for GUI items
	static UT_uint32		s_iInstanceCount;
  
	GdkColormap* 	 		m_pColormap;
	int          			m_iWindowHeight, m_iWindowWidth;
	UT_sint32				m_iLineWidth;
	GR_Graphics::Cursor		m_cursor;

	GR_Graphics::ColorSpace	m_cs;
	
	GdkColor				m_3dColors[COUNT_3D_COLORS];
	XAP_UnixFontHandle *	m_pFallBackFontHandle;

protected:	
	XftDraw*				m_pXftDraw;
	XftColor				m_XftColor;

	UT_uint32               m_iDeviceResolution;
	
private:	
	XftFont*				m_pXftFontL;
	XftFont*				m_pXftFontD;
	
	Drawable				m_Drawable;
	Visual*					m_pVisual;
	Colormap				m_Colormap;

	FcChar32				m_aMap[FC_CHARSET_MAP_SIZE];
	UT_sint32               m_iXoff;
	UT_sint32               m_iYoff;
	UT_RGBColor				m_curColor;

	UT_GenericVector<UT_Rect*>     m_vSaveRect;
	UT_GenericVector<GdkPixbuf *>  m_vSaveRectBuf;

	bool                    m_bIsSymbol;       
	bool                    m_bIsDingbat;
};

#endif /* GR_UNIXGRAPHICS_H */
