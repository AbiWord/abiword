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
#include "ut_assert.h"
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
	virtual GR_Font* findFont(const char* pszFontFamily, const char* pszFontStyle, 
							  const char* pszFontVariant, const char* pszFontWeight, 
							  const char* pszFontStretch, const char* pszFontSize);
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
  
	virtual bool queryProperties(GR_Graphics::Properties gp) const;
	virtual bool startPrint(void);
	virtual bool startPage(const char * szPageLabel, UT_uint32 pageNumber,
						   bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	virtual bool endPrint(void);

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
	BView					*m_pShadowView, *m_pFrontView;
	BBitmap					*m_pShadowBitmap;
	BMessage				*m_pPrintSettings;	
	BPrintJob				*m_pPrintJob;
	BeOSFont				*m_pBeOSFont, *m_pFontGUI;
	GR_Graphics::ColorSpace m_cs;
	GR_Graphics::Cursor		m_cursor;
	rgb_color				m_3dColors[COUNT_3D_COLORS];
 	bool					m_bPrint;  
 	
 	// Takes a line and modifies it to fit with the BeOS pixel coordinate system.
 	// Returns a BPoint containing the modified end-point (x2, y2).
 	inline BPoint beosiseLineEnding(UT_sint32 x1, UT_sint32 y1, 
 									UT_sint32 x2, UT_sint32 y2);
};

BPoint GR_BeOSGraphics::beosiseLineEnding(UT_sint32 x1, UT_sint32 y1, 
												 UT_sint32 x2, UT_sint32 y2) {
	// Work out the line's orientation
	int tell = abs(x1-x2) - abs(y1-y2);

	// If it's exactly diagonal...
	if(tell == 0) 
	{
		// Drawn top-left to bottom-right
		if(x1<x2 && y1<y2)
		{
			x2--;
			y2--;
		}
		// Drawn bottom-right to top-left
		else if(x1>x2 && y1>y2)
		{
			x2++;
			y2++;
		}
		// Top-right to bottom-left
		else if(x1>x2 && y1<y2)
		{
			x2++;
			y2--;
		}
		// Bottom-left to top-right
		else if(x1<x2 && y1>y2)
		{
			x2--;
			y2++;
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		
	} 
	// ...else if it's at an acute angle to the horizontal... 
	else if (tell > 0)
	{
		// If the line is being drawn left-to-right
		if(x1<x2)
			x2--;
		else
			x2++;
	}
	// ...else it's at an obtuse angle to the horizontal.
	else // if (tell < 0) 
	{
		// If the line is being drawn top-to-bottom
		if(y1<y2)
			y2--;
		else
			y2++;
	}
	
	return(BPoint(x2, y2));
}


#endif /* GR_BEOSGRAPHICS_H */
