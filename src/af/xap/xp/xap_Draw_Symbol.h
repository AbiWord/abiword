/* AbiSource Application Framework
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


#ifndef XAP_DRAW_SYMBOL_H
#define XAP_DRAW_SYMBOL_H

#include "ut_misc.h"
#include "ut_types.h"

#include "xap_Preview.h"


class XAP_Draw_Symbol : public XAP_Preview
{
public:

	XAP_Draw_Symbol(GR_Graphics * gc);
	virtual ~XAP_Draw_Symbol(void);
				
	// data twiddlers
	void			setSelectedFont(char *font);
	void			setFontString(void);
	void			setFontStringarea(void);
	void			setFontToGC(GR_Graphics *p_gc, UT_uint32 MaxWidthAllowable, UT_sint32 PointSize);
	void			setFontfont(GR_Font * font);
	void			setWindowSize(UT_uint32 width, UT_uint32 height);
	void			setAreaSize(UT_uint32 width, UT_uint32 height);
	void			setAreaGc(GR_Graphics *);
	char *	                getSelectedFont(void);

    // where all the Symbol-specific drawing happens
	
	void			draw(void);
	void			drawarea(UT_UCSChar c, UT_UCSChar p);
	UT_UCSChar		calcSymbol(UT_uint32 x, UT_uint32 y);

	void			setCurrent(UT_UCSChar c)
	{
		m_PreviousSymbol = m_CurrentSymbol;
		m_CurrentSymbol = c;
		drawarea(m_CurrentSymbol, m_PreviousSymbol);
	}
	UT_UCSChar		getCurrent(void)
	{
		return m_CurrentSymbol;
	}
	void			onLeftButtonDown(UT_sint32 x, UT_sint32 y);

protected:
	GR_Graphics *               m_areagc;
	GR_Font *			        m_pFont;	// so we can delete it

	GR_Font *                   m_fontarea;
	
	UT_uint32                   m_drawWidth;
	UT_uint32                   m_drawHeight;
	UT_uint32                   m_drawareaWidth;
	UT_uint32                   m_drawareaHeight;

	UT_UCSChar m_CurrentSymbol;
	UT_UCSChar m_PreviousSymbol;
};

#endif /* XAP_Draw_Symbol_H */













