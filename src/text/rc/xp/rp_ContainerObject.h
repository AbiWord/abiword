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

#ifndef RP_CONTAINEROBJECT_H
#define RP_CONTAINEROBJECT_H

#include "pt_Types.h"
#include "rp_Object.h"
#include "rl_Layout.h"
#include "ut_vector.h"

class ABI_EXPORT rp_ContainerObject : public rp_Object
{
public:
	rp_ContainerObject(rl_Layout* pLayout);
	virtual ~rp_ContainerObject() {}

	void addChild(rp_Object* pObject);
	virtual void draw();

	UT_sint32 getCloseTagWidth() { return m_iCloseTagWidth; }
	virtual void recalcHeight();
	virtual void setHeight(UT_sint32 iHeight);
	
	virtual void layout();
	
protected:
	virtual void _drawOpenTag();
	virtual void _drawCloseTag();

	UT_sint32 _getArrowWidth() { return m_iArrowWidth; }

	virtual void _setContentsWidth(UT_sint32 iWidth);
	
	virtual void _recalcTagWidth();
	virtual void _recalcWidth();
	
	virtual void _recalcCloseTagTextWidth();
	void _recalcCloseTagWidth();
	UT_uint32 _getCloseTagTextWidth() { return m_iCloseTagTextWidth; }
	void _setCloseTagText(UT_UCS4String* pString);
	UT_UCS4String* _getCloseTagText() { return m_pCloseTagString; }

private:
	UT_GenericVector<rp_Object*>	m_vecChildren;

	UT_sint32						m_iArrowWidth;
	UT_sint32						m_iCloseTagWidth;
	UT_sint32						m_iContentsWidth;
	UT_UCS4String*					m_pCloseTagString;
	UT_uint32						m_iCloseTagTextWidth;
};

#endif /* RP_CONTAINEROBJECT_H */
