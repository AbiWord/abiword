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
//#include "xap_QNXFont.h"
#include "gr_Graphics.h"
#include <string.h>
#include <Pt.h>

#include <ut_string.h>

class UT_ByteBuf;

class QNXFont : public GR_Font {
        public:
                QNXFont(const char *aFont) { 
					m_fontstr = (aFont) ? UT_strdup(aFont) : NULL; 
				};
                ~QNXFont() { 
					if (m_fontstr) { 
						free(m_fontstr); 
					} 
				};
                const char *getFont(void) { 
					return(m_fontstr); 
				};
        private:
                char   *m_fontstr;
};

class GR_QNXGraphics : public GR_Graphics
{
 public:
	GR_QNXGraphics(PtWidget_t * win, PtWidget_t * draw, XAP_App * app);
	~GR_QNXGraphics();

	virtual void 		drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff);
	virtual void 		drawChars(const UT_UCSChar* pChars, int iCharOffset,
						   		  int iLength, UT_sint32 xoff, UT_sint32 yoff);
	virtual void 		setFont(GR_Font* pFont);
	virtual UT_uint32 	measureUnRemappedChar(const UT_UCSChar c);
	virtual void 		setColor(UT_RGBColor& clr);

	virtual GR_Font* 	getGUIFont();
	virtual GR_Font* 	findFont(const char* pszFontFamily, 
								 const char* pszFontStyle, 
								 const char* pszFontVariant, 
								 const char* pszFontWeight, 
								 const char* pszFontStretch, 
								 const char* pszFontSize);
	virtual UT_uint32 	getFontAscent();
	virtual UT_uint32 	getFontDescent();
	virtual UT_uint32 	getFontHeight();
	virtual void 		drawLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void 		setLineWidth(UT_sint32);
	virtual void 		xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void 		polyLine(UT_Point * pts, UT_uint32 nPoints);
	virtual void 		fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void 		fillRect(UT_RGBColor& c, UT_Rect &r);
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

	/* GR_Font versions of the above information */
	virtual UT_uint32 	getFontAscent(GR_Font *);
	virtual UT_uint32 	getFontDescent(GR_Font *);
	virtual UT_uint32 	getFontHeight(GR_Font *);

 protected:
	virtual UT_uint32 	_getResolution(void) const;
	int 				DrawSetup();
	int 				DrawTeardown();

	PtWidget_t *  	m_pWin;
	PtWidget_t *  	m_pDraw;
	PhPoint_t		m_OffsetPoint;		
	PhTile_t *		m_pClipList;		

	// our currently requested font by handle
	QNXFont *			m_pFont;
	// our "OEM" system font, like a 10 point Helvetica for GUI items
	QNXFont * 			m_pFontGUI;
  
	void*  				m_pColormap;
	UT_sint32			m_iLineWidth;
	UT_sint32			m_iAscentCache, m_iDescentCache;

	GR_Graphics::Cursor		m_cursor;
	GR_Graphics::ColorSpace	m_cs;
	
	PgColor_t			m_currentColor;
	PgColor_t			m_3dColors[COUNT_3D_COLORS];

	UT_uint32			m_iShadowZoomPercentage;
 	bool             m_bPrintNextPage;    
	PpPrintContext_t *  m_pPrintContext;
};

#endif /* GR_QNXGRAPHICS_H */
