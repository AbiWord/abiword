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

#include "rl_ContainerLayout.h"

rl_ContainerLayout::rl_ContainerLayout(PTStruxType type, PL_StruxDocHandle sdh, rl_ContainerLayout* pParent, rl_DocLayout *pDocLayout) :
	rl_Layout(type, sdh, pParent, pDocLayout)
{
}

rp_Object* rl_ContainerLayout::_createObject()
{
	return new rp_ContainerObject(this);
}

void rl_ContainerLayout::breakAfter(rl_Layout* child)
{
	if (m_vecChildren.size() > 0 && 
		child == m_vecChildren.getLastItem())
	{
		m_bBreakAfter = true;
		if (getParent()) getParent()->breakAfter(this);
	}
}

rl_Layout* rl_ContainerLayout::getLastChild()
{
	if (m_vecChildren.size() > 0)
		return m_vecChildren.getLastItem();
	
	return NULL;
}

void rl_ContainerLayout::addChild(rl_Layout *pLayout)
{
	// TODO: ... document this
	m_bBreakAfter = false;

	if (m_vecChildren.size() > 0)
	{
		m_vecChildren.getLastItem()->setNext( pLayout );
		pLayout->setPrev( m_vecChildren.getLastItem() );
	}
	
	m_vecChildren.addItem(pLayout);
}

rp_Object* rl_ContainerLayout::format()
{
	rp_ContainerObject* pObject = static_cast<rp_ContainerObject *>(_createObject());
	setObject( pObject );

	pObject->layout();
	
	for (UT_uint32 i = 0; i < m_vecChildren.size(); i++)
	{
		rl_Layout* pLayout = m_vecChildren.getNthItem(i);
		rp_Object* child = pLayout->format();
		pObject->addChild( child );
	}

	pObject->recalcHeight();
	
	return pObject;
}
