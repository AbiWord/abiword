/* AbiWord
 * Copyright (C) 2004 Marc Maurer
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

#ifndef RP_OBJECT_H
#define RP_OBJECT_H

#include "pt_Types.h"
#include "pp_AttrProp.h"
#include "pp_Revision.h"
#include "gr_Painter.h"
#include "gr_Graphics.h"

class rl_Layout;

class ABI_EXPORT rp_Object
{
public:
	rp_Object(rl_Layout* pLayout);
	virtual ~rp_Object() {}

	rl_Layout* getLayout() { return m_pLayout; }
		
	virtual void draw() = 0;
		
	UT_sint32 getX() { return m_iX; }
	UT_sint32 getY() { return m_iY; }
	void setX(UT_sint32 iX) { m_iX = iX; }
	void setY(UT_sint32 iY) { m_iY = iY; }
	UT_sint32 getWidth() { return m_iWidth; }
	UT_sint32 getHeight() { return m_iHeight; }
	virtual void setWidth(UT_sint32 iWidth) { m_iWidth = iWidth; }
	virtual void setHeight(UT_sint32 iHeight) { m_iHeight = iHeight; }
	
	UT_sint32 getTagWidth() { return m_iTagWidth; }	
	void setTagWidth(UT_sint32 iTagWidth) { m_iTagWidth = iTagWidth; }
	
	UT_sint32 getTagHeight() { return m_iTagHeight; }
	void setTagHeight(UT_sint32 iTagHeight) { m_iTagHeight = iTagHeight; }
	
	UT_sint32 getTagBorder() { return m_iTagBorder; }	
	void setTagBorder(UT_sint32 iTagBorder) { m_iTagBorder = iTagBorder; }
	
	void setText(UT_UCS4String* pString);
	UT_UCS4String* getText() { return m_pString; }

	void expandAttrProp();
	
	void setFont(GR_Font* pFont) { m_pFont = pFont; }
	GR_Font* getFont() { return m_pFont; }
	
	virtual void layout() {}

	virtual void recalcHeight() { /* FIXME: implement me? */ }

protected:
	virtual void _recalcTextWidth();
	UT_uint32 _getTextWidth() { return m_iTextWidth; }
	virtual void _recalcTagWidth();
	virtual void _recalcWidth();
	
	UT_sint32 m_iWidth;
	UT_sint32 m_iHeight;
	
private:
	rl_Layout* m_pLayout;

	UT_sint32 m_iX;
	UT_sint32 m_iY;
	
	UT_sint32 m_iTagWidth;
	UT_sint32 m_iTagHeight;
	UT_sint32 m_iTagBorder;

	UT_UCS4String* m_pString;
	UT_uint32 m_iTextWidth;

	GR_Font* m_pFont; // TODO: do we really need to store this? - MARCM
};

#endif /* RP_OBJECT_H */
