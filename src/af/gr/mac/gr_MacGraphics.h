/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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



#ifndef GR_MACGRAPHICS_h
#define GR_MACGRAPHICS_h

#include <QuickDraw.h>

#include <ApplicationServices/ApplicationServices.h>

#include "ut_misc.h"
#include "ut_string.h"
#include "gr_Graphics.h"

class MacFont : public GR_Font
{
public:
	MacFont(const char * font, float pointSize)
            { m_font = UT_strdup (font); m_pointSize = pointSize; };
        virtual ~MacFont ()
            { if (m_font) free (m_font); };
	char *m_font;
	float m_pointSize;
};

class GR_MacGraphics : public GR_Graphics
{
public:
	GR_MacGraphics(CGrafPtr port, XAP_App * app);	/* for screen */
	virtual ~GR_MacGraphics();

	virtual void drawChars(const UT_UCSChar* pChars, 
		int iCharOffset, int iLength, UT_sint32 xoff, UT_sint32 yoff);
	virtual void setFont(GR_Font* pFont);

	virtual UT_uint32 getFontAscent();
	virtual UT_uint32 getFontDescent();
	virtual UT_uint32 getFontHeight();
	
	//virtual UT_uint32 measureString(const UT_UCSChar*s, int iOffset, int num, unsigned short* pWidths);
	virtual UT_uint32 measureUnRemappedChar(const UT_UCSChar c);
	
	virtual void setColor(UT_RGBColor& clr);
	virtual GR_Font* getGUIFont();
	virtual GR_Font* findFont(
		const char* pszFontFamily, 
		const char* pszFontStyle, 
		const char* pszFontVariant, 
		const char* pszFontWeight, 
		const char* pszFontStretch, 
		const char* pszFontSize);

	/* For drawLine() and xorLine():
	**   x0,y0 give the starting pixel.
	**   x1,y1 give the first pixel ***not drawn***.
	*/
	virtual void drawLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void setLineWidth(UT_sint32);
	virtual void polyLine(UT_Point * pts, UT_uint32 nPoints);

	/* For fillRect() and ??:
	**   begin fill at x0,y0,
	**   ?? should x0+w,y0+h or x0+w+1,y0+h+1 be the last pixel affected ??
	*/
	virtual void fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
       	virtual void fillRect(UT_RGBColor& c, UT_Rect &r);
	virtual void invertRect(const UT_Rect* pRect);
	virtual void setClipRect(const UT_Rect* pRect);
	virtual void scroll(UT_sint32, UT_sint32);
	virtual void scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height);
	virtual void clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	
	virtual bool queryProperties(GR_Graphics::Properties gp) const;

	/* the following are only used for printing */
	
	virtual bool startPrint(void);
	virtual bool startPage(const char * szPageLabel, UT_uint32 pageNumber,
							  bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	virtual bool endPrint(void);

	virtual void setColorSpace(GR_Graphics::ColorSpace c);
	virtual GR_Graphics::ColorSpace getColorSpace(void) const;

	virtual void setCursor(GR_Graphics::Cursor c);
	virtual GR_Graphics::Cursor getCursor(void) const;

	virtual void		setColor3D(GR_Color3D c);
	virtual void		fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void		fillRect(GR_Color3D c, UT_Rect &r);

	/* GR_Font versions of the above -- TODO: should I add drawChar* methods too? */
	virtual UT_uint32 getFontAscent(GR_Font *);
	virtual UT_uint32 getFontDescent(GR_Font *);
	virtual UT_uint32 getFontHeight(GR_Font *);
protected:
	virtual UT_uint32 _getResolution(void) const { return 72; };

        CGContextRef m_CGContext;
        MacFont	*m_pMacFont;
        
        UT_RGBColor m_3Dcolors[COUNT_3D_COLORS];
	
};

#endif /* GR_MACGRAPHICS_h */
