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



#ifndef WIN32_TT_h
#define WIN32_TT_h

#include "ut_misc.h"
#include "gr_Graphics.h"

class GR_Win32Font : public GR_Font
{
public:
	GR_Win32Font(HFONT hFont);
	HFONT getHFONT();

protected:
	HFONT				m_hFont;
};

class GR_Win32Graphics : public GR_Graphics
{
public:
	GR_Win32Graphics(HDC, HWND);					/* for screen */
	GR_Win32Graphics(HDC, const DOCINFO *);		/* for printing */
	~GR_Win32Graphics();

	virtual void drawChars(const UT_UCSChar* pChars, int iCharOffset, int iLength, UT_sint32 xoff, UT_sint32 yoff);
	virtual void setFont(GR_Font* pFont);
	virtual UT_uint32 getFontHeight();
	virtual UT_uint32 measureString(const UT_UCSChar*s, int iOffset, int num, unsigned short* pWidths);
	virtual UT_uint32 getResolution() const;
	virtual void setColor(UT_RGBColor& clr);
	virtual GR_Font* getGUIFont();
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
	virtual void xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void setLineWidth(UT_sint32);
	virtual void polyLine(UT_Point * pts, UT_uint32 nPoints);
	virtual void fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void invertRect(const UT_Rect* pRect);
	virtual void setClipRect(const UT_Rect* pRect);
	virtual void scroll(UT_sint32 dx, UT_sint32 dy);
	virtual void scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height);
	virtual void clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32);

	virtual void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest, UT_sint32 iDestWidth, UT_sint32 iDestHeight);
	
	virtual UT_Bool queryProperties(GR_Graphics::Properties gp) const;

	virtual UT_Bool startPrint(void);
	virtual UT_Bool startPage(const char * szPageLabel, UT_uint32 pageNumber,
							  UT_Bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	virtual UT_Bool endPrint(void);

protected:
	UT_RGBColor			m_clr;
	HDC					m_hdc;
	HWND 				m_hwnd;
	const DOCINFO *		m_pDocInfo;
	UT_Bool				m_bPrint;
	UT_Bool				m_bStartPrint;
	UT_Bool				m_bStartPage;
	GR_Win32Font*		m_pFont;
	GR_Win32Font*		m_pFontGUI;
	ABC					m_aABCs[256];
	int					m_aCharWidths[256];
	UT_sint32			m_iLineWidth;

private:
	void 				_constructorCommonCode(HDC);
};

#endif /* WIN32_TT_h */
