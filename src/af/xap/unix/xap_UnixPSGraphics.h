/* AbiSource Program Utilities
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
 


#ifndef PS_GRAPHICS_H
#define PS_GRAPHICS_H

#include "ut_vector.h"
#include "gr_Graphics.h"
#include "ut_misc.h"

#include "xap_UnixFont.h"
#include "xap_UnixFontManager.h"

#include "xap_UnixPSFont.h"
#include "xap_UnixPSGenerate.h"

/*****************************************************************/
/*****************************************************************/

class PS_Graphics : public DG_Graphics
{
public:
	/* TODO add other constructors to route to lp/lpr rather than a file */
	PS_Graphics(const char * szFilename,
				const char * szTitle,
				const char * szSoftwareNameAndVersion,
				AP_UnixFontManager * fontManager,
				UT_Bool		 bIsFile);
	virtual ~PS_Graphics();

	virtual void drawChars(const UT_UCSChar* pChars, 
						   int iCharOffset, int iLength,
						   UT_sint32 xoff, UT_sint32 yoff);
	virtual void setFont(DG_Font* pFont);

	virtual UT_uint32 getFontAscent();
	virtual UT_uint32 getFontDescent();
	virtual UT_uint32 getFontHeight();
	
	virtual UT_uint32 measureString(const UT_UCSChar*s, int iOffset, int num, unsigned short* pWidths);
	
	virtual UT_uint32 getResolution() const;
	virtual void setColor(UT_RGBColor& clr);
	virtual DG_Font* getGUIFont();
	virtual DG_Font* findFont(const char* pszFontFamily, 
							  const char* pszFontStyle, 
							  const char* pszFontVariant, 
							  const char* pszFontWeight, 
							  const char* pszFontStretch, 
							  const char* pszFontSize);
	virtual void drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2);
	virtual void xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	virtual void polyLine(UT_Point * pts, UT_uint32 nPoints);
	virtual void fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	virtual void invertRect(const UT_Rect*);
	virtual void setClipRect(const UT_Rect*);
	virtual void scroll(UT_sint32, UT_sint32);
	virtual void scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						UT_sint32 x_src, UT_sint32 y_src,
						UT_sint32 width, UT_sint32 height);
	virtual void clearArea(UT_sint32, UT_sint32, UT_sint32, UT_sint32);
	
	virtual UT_Bool queryProperties(DG_Graphics::Properties gp) const;
	
	virtual UT_Bool startPrint(void);
	virtual UT_Bool startPage(const char * szPagelabel, UT_uint32 pageNumber,
							  UT_Bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	virtual UT_Bool endPrint(void);

protected:
	UT_uint32		_scale(UT_uint32 units) const;
	UT_Bool			_startDocument(void);
	UT_Bool			_startPage(const char * szPageLabel, UT_uint32 pageNumber,
							   UT_Bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight);
	UT_Bool			_endPage(void);
	UT_Bool			_endDocument(void);
	void			_emit_DocumentNeededResources(void);
	void			_emit_IncludeResource(void);
	void			_emit_PrologMacros(void);
	void			_emit_FontMacros(void);
	void			_emit_SetFont(void);
	
	UT_Vector		m_vecFontList;
	PSFont *		m_pCurrentFont;
	ps_Generate *	m_ps;
	const char *	m_szFilename;
	const char *	m_szTitle;
	const char *	m_szSoftwareNameAndVersion;
	UT_Bool			m_bStartPrint;
	UT_Bool			m_bStartPage;
	UT_Bool			m_bNeedStroked;
	UT_Bool			m_bIsFile;

	AP_UnixFontManager *	m_fm;
	
};

#endif /* PS_GRAPHICS_H */
