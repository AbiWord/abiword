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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef GR_BEOSGRAPHICS_H
#define GR_BEOSGRAPHICS_H

#include "gr_Graphics.h"
#include <Be.h>

//#define USE_BACKING_BITMAP 1

class BeOSFont : public GR_Font {
	public:
		BeOSFont(BFont *aFont) 	{ m_pBFont = aFont; };
		BFont *get_font(void) 	{ return(m_pBFont); };
	private:
		BFont	*m_pBFont;
};

class GR_BeOSGraphics : public GR_Graphics
{
public:
  GR_BeOSGraphics(BView *front, XAP_App *app);
  ~GR_BeOSGraphics();

  virtual void drawChars(const UT_UCSChar* pChars, int iCharOffset,
			 int iLength, UT_sint32 xoff, UT_sint32 yoff);
  virtual void setFont(GR_Font* pFont);
  virtual UT_uint32 getFontHeight();
  //virtual UT_uint32 measureString(const UT_UCSChar*s, int iOffset, int num, unsigned short* pWidths);
  virtual UT_uint32 measureUnRemappedChar(const UT_UCSChar c);
  virtual UT_uint32 _getResolution(void) const;
  virtual void setColor(UT_RGBColor& clr);
  virtual void setColor3D(GR_Color3D c);
  virtual GR_Font* getGUIFont(void);
  virtual GR_Font* findFont(
		const char* pszFontFamily, 
		const char* pszFontStyle, 
		const char* pszFontVariant, 
		const char* pszFontWeight, 
		const char* pszFontStretch, 
		const char* pszFontSize);
  virtual UT_uint32 getFontAscent();
  virtual UT_uint32 getFontDescent();
  virtual void drawLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
  virtual void polyLine(UT_Point * pts, UT_uint32 nPoints);
  virtual void xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
  virtual void fillRect(UT_RGBColor& c, UT_Rect &r);
  virtual void fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
  virtual void fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
  virtual void fillRect(GR_Color3D c, UT_Rect &r);

  virtual void invertRect(const UT_Rect* pRect);
  virtual void setClipRect(const UT_Rect* pRect);
  virtual void scroll(UT_sint32, UT_sint32);
  virtual void scroll(UT_sint32 x_dest, UT_sint32 y_dest,
					  UT_sint32 x_src, UT_sint32 y_src,
					  UT_sint32 width, UT_sint32 height);
  virtual void clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
  virtual void setLineWidth(UT_sint32 iLineWidth);

  virtual void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
   virtual GR_Image* createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType = GR_Image::GRT_Raster);
  
  virtual UT_Bool queryProperties(GR_Graphics::Properties gp) const;
  virtual UT_Bool startPrint(void);
  virtual UT_Bool startPage(const char * szPageLabel, UT_uint32 pageNumber,
			UT_Bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
  virtual UT_Bool endPrint(void);

  virtual void flush(void);
  virtual void setColorSpace(GR_Graphics::ColorSpace c);
  virtual GR_Graphics::ColorSpace getColorSpace(void) const;

  virtual void setCursor(GR_Graphics::Cursor c);
  virtual GR_Graphics::Cursor getCursor(void) const;


	//Added for local updating of the View
	void			ResizeBitmap(BRect r);
	BBitmap 		*ShadowBitmap()	
					{ return(m_pShadowBitmap); };
	BMessage		*GetPrintSettings() 	
					{ return(m_pPrintSettings); };
	void 			SetPrintSettings(BMessage *s) 
					{ m_pPrintSettings = s; };
	BPrintJob		*GetPrintJob()
					{ return(m_pPrintJob); };
	void			SetPrintJob(BPrintJob *j)
					{ m_pPrintJob = j; };
 
	//Added for obtain background Color
  	  rgb_color		Get3DColor(GR_Graphics::GR_Color3D c)
    				{ return m_3dColors[c];};
 
protected:
	BView				*m_pShadowView, *m_pFrontView;
	BBitmap				*m_pShadowBitmap;
	BMessage			*m_pPrintSettings;	
	BPrintJob			*m_pPrintJob;
	BeOSFont			*m_pBeOSFont, *m_pFontGUI;
	GR_Graphics::ColorSpace m_cs;
	GR_Graphics::Cursor		m_cursor;
	rgb_color			m_3dColors[COUNT_3D_COLORS];
 	UT_Bool                         m_bPrint;    
};

#endif /* GR_BEOSGRAPHICS_H */
