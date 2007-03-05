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

#ifndef GR_QNXGRAPHICS_H
#define GR_QNXGRAPHICS_H

#include "xap_QNXApp.h"
#include "gr_Graphics.h"
#include <string.h>
#include <Pt.h>
#include <photon/PhRender.h>
#include "ut_vector.h"

#include <ut_string.h>

class UT_ByteBuf;

class QNXFont : public GR_Font {
  public:
		QNXFont(FontID *aFont); 
		~QNXFont();
		const char *getFont(void); 
		const int getSize(void);


		const int getDisplayFontSize(void);
		const char *getDisplayFont(void);
		void	createDisplayFont(UT_uint32 size); 
		void deleteDisplayFont();
	
		virtual UT_sint32 measureUnremappedCharForCache(UT_UCSChar cChar) const;

	private:
		FontID	*m_fontID;
		FontID	*m_displayFontID;
		FontID	*m_120ptFontID;
};

class GR_QNXAllocInfo : public GR_AllocInfo
{
  public:
	GR_QNXAllocInfo(PtWidget_t * win, PtWidget_t * draw, XAP_App * app) 
	: m_win(win),m_draw(draw) {};

	virtual GR_GraphicsId getType() const {return GRID_QNX;}
	virtual bool isPrinterGraphics() const { return false; }
	PtWidget_t *m_win,*m_draw;

};

class GR_QNXGraphics : public GR_Graphics
{
	// all constructors are protected; instances must be created via
	// GR_GraphicsFactory
 public:
	~GR_QNXGraphics();

	static UT_uint32 s_getClassId() {return GRID_QNX;}
	virtual UT_uint32 getClassId() {return s_getClassId();}

	virtual GR_Capability getCapability() {UT_ASSERT(UT_NOT_IMPLEMENTED); return GRCAP_UNKNOWN;}
	
	static const char *    graphicsDescriptor(void){return "QNX Default";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);
	
	virtual void		drawGlyph(UT_uint32 glyph_idx,UT_sint32 xoff,UT_sint32 yoff);
	virtual void 		drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff);
	virtual void 		drawChars(const UT_UCSChar* pChars, int iCharOffset,
						   		  int iLength, UT_sint32 xoff, UT_sint32 yoff,
								  int * pCharWidths = NULL);
	virtual void 		setFont(const GR_Font* pFont);
	virtual void        clearFont(void) {m_pFont = NULL; }
//	virtual UT_uint32		measureString(const UT_UCSChar *s,int iOffset,int num,UT_GrowBufElement *pWidths);
	virtual UT_sint32 	measureUnRemappedChar(const UT_UCSChar c);
	virtual void 		getColor(UT_RGBColor& clr);
	virtual void 		setColor(const UT_RGBColor& clr);

	virtual GR_Font* 	getGUIFont();
	virtual UT_uint32 	getFontAscent();
	virtual UT_uint32 	getFontDescent();
	virtual UT_uint32 	getFontHeight();
	virtual void getCoverage(UT_NumberVector &coverage);

	virtual void		_beginPaint();
	virtual void		_endPaint();
	virtual void 		drawLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void 		setLineWidth(UT_sint32);
	virtual void 		xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void		polygon(UT_RGBColor& c,UT_Point *pts,UT_uint32 nPoints);
	virtual void 		polyLine(UT_Point * pts, UT_uint32 nPoints);
	virtual void 		fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void 		fillRect(const UT_RGBColor& c, UT_Rect &r);
	virtual void 		invertRect(const UT_Rect* pRect);
	virtual void 		setClipRect(const UT_Rect* pRect);
	virtual void 		scroll(UT_sint32, UT_sint32);
	virtual void 		scroll(UT_sint32 x_dest, UT_sint32 y_dest,
							   UT_sint32 x_src, UT_sint32 y_src,
							   UT_sint32 width, UT_sint32 height);
	virtual void 		clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
  
	virtual void 		drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
   	virtual GR_Image* 	createNewImage(const char* pszName, const UT_ByteBuf* pBBPNG, 
									   UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, 
									   GR_Image::GRType iType = GR_Image::GRT_Raster);
  

	virtual bool 		queryProperties(GR_Graphics::Properties gp) const;
	virtual void 		flush(void);

	/* Printing */
	virtual bool 		startPrint(void);
	virtual bool 		startPage(const char * szPageLabel, UT_uint32 pageNumber,
							  		bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	virtual bool 		endPrint(void);

	PpPrintContext_t * 		getPrintContext();
	void					setPrintContext(PpPrintContext_t *c);

	/* 3D Stuff */
	virtual void 		setColorSpace(GR_Graphics::ColorSpace c);
	virtual GR_Graphics::ColorSpace getColorSpace(void) const;
	
	virtual void 		setCursor(GR_Graphics::Cursor c);
	virtual GR_Graphics::Cursor getCursor(void) const;

	virtual void		setColor3D(GR_Color3D c);
	void 				init3dColors();
	virtual void 		fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void 		fillRect(GR_Color3D c, UT_Rect &r);
	virtual void setLineProperties ( double inWidthPixels, 
					 GR_Graphics::JoinStyle inJoinStyle = JOIN_MITER,
					 GR_Graphics::CapStyle inCapStyle   = CAP_BUTT,
					 GR_Graphics::LineStyle inLineStyle = LINE_SOLID ) ;

	/* GR_Font versions of the above information */
	virtual UT_uint32 	getFontAscent(const GR_Font *);
	virtual UT_uint32 	getFontDescent(const GR_Font *);
	virtual UT_uint32 	getFontHeight(const GR_Font *);
	virtual void		setZoomPercentage(UT_uint32 iZoom);
	//Need this in the textdrawing cb.
	PgColor_t getCurrentPgColor() { return m_currentColor; };
	QNXFont *getCurrentQNXFont() { return m_pFont; };
	PhGC_t	*getCurrentGC() { return m_pGC; };

protected:
	// all instances have to be created via GR_GraphicsFactory; see gr_Graphics.h
	GR_QNXGraphics(PtWidget_t * win, PtWidget_t * draw, XAP_App * app);
	virtual GR_Font* 	_findFont(const char* pszFontFamily, 
								  const char* pszFontStyle, 
								  const char* pszFontVariant, 
								  const char* pszFontWeight, 
								  const char* pszFontStretch, 
								  const char* pszFontSize,
								  const char* pszLang);

	virtual UT_uint32 	getDeviceResolution(void) const;
	inline int 				DrawSetup();
	inline int 				DrawTeardown();

	PtWidget_t *  	m_pWin;
	PtWidget_t *  	m_pDraw;
//	PdOffscreenContext_t *m_pOSC;
//	PhDrawContext_t				*m_pOldDC;
	PhPoint_t		m_OffsetPoint;		
	PhTile_t		*m_pClipList;
	struct _Pf_ctrl * m_pFontCx;	

	// our currently requested font by handle
	QNXFont *			m_pFont;
	// our "OEM" system font, like a 10 point Helvetica for GUI items
	QNXFont * 			m_pFontGUI;
  
	void*  				m_pColormap;
	UT_sint32			m_iLineWidth;
	UT_sint32			m_iAscentCache, m_iDescentCache,m_iHeightCache;

	GR_Graphics::Cursor		m_cursor;
	GR_Graphics::ColorSpace	m_cs;
	
	PgColor_t			m_currentColor;
	PgColor_t			m_3dColors[COUNT_3D_COLORS];
	
	PhGC_t				*m_pGC;
	PhGC_t				*m_pGC_old;

	UT_uint32			m_iShadowZoomPercentage;
 	bool             m_bPrintNextPage;    
	PpPrintContext_t *  m_pPrintContext;

	virtual GR_Image * genImageFromRectangle(const UT_Rect & r);
private:

	virtual void saveRectangle(UT_Rect &r, UT_uint32 iIndx);
	virtual void restoreRectangle(UT_uint32 iIndx);
	bool	OSCIsValid();
	void	blitScreen();
	void	setDamage(int x,int y,int h,int w);
	
	UT_GenericVector<UT_Rect*>				m_vSaveRect;
	UT_GenericVector<PhImage_t *>			m_vSaveRectBuf;
	PdOffscreenContext_t *m_pOSC;
	PhArea_t				m_DamagedArea;
};

#endif /* GR_QNXGRAPHICS_H */
