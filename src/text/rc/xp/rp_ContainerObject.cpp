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

#include "rp_ContainerObject.h"
#include "rl_DocLayout.h"

rp_ContainerObject::rp_ContainerObject(rl_Layout* pLayout) :
	rp_Object(pLayout),
	m_iCloseTagWidth(0),
	m_iContentsWidth(0),
	m_pCloseTagString(NULL),
	m_iCloseTagTextWidth(0)
{
	GR_Graphics *pG = getLayout()->getDocLayout()->getGraphics();
	m_iArrowWidth = getHeight() / 2 - pG->tlu(1);
}

void rp_ContainerObject::addChild(rp_Object *pObject)
{
	m_vecChildren.addItem(pObject);
	m_iContentsWidth += pObject->getWidth();
	_recalcWidth();
}

void rp_ContainerObject::_setContentsWidth(UT_sint32 iWidth)
{
	m_iContentsWidth = iWidth;
	_recalcWidth();
}

void rp_ContainerObject::_recalcTagWidth()
{
	UT_sint32 iTagWidth = _getTextWidth() + m_iArrowWidth + 2 * getTagBorder();
	setTagWidth( iTagWidth );
	_recalcWidth();
}

void rp_ContainerObject::recalcHeight()
{
	UT_sint32 iHeight = getTagHeight();
	
	if (m_vecChildren.size() > 0)
	{
		rp_Object* pObject = m_vecChildren.getLastItem();
		
		if (getY() < pObject->getY())
		{
			iHeight +=  pObject->getY() + pObject->getHeight() - (getY() + getTagHeight());
		}
	}
	
	setHeight(iHeight);
}

void rp_ContainerObject::setHeight(UT_sint32 iHeight)
{
	m_iHeight = iHeight;
	
	/* FIXME: this seems to cause some kind of exponential behavior.... */
	rl_ContainerLayout* pCL = static_cast<rl_ContainerLayout*>(getLayout());
	UT_return_if_fail(pCL);
	rl_ContainerLayout* pParentLayout = static_cast<rl_ContainerLayout*>(pCL->getParent());
	if (pParentLayout)
	{
		rp_ContainerObject* pParentObject = static_cast<rp_ContainerObject*>(pParentLayout->getObject());
		UT_return_if_fail(pParentObject);
		
		if (true /*&& FIXME: do this only if I'm the last child - MARCM */)
		{
			pParentObject->recalcHeight();
		}
	}
}

void rp_ContainerObject::_recalcWidth()
{
	setWidth( getTagWidth() + m_iContentsWidth + m_iCloseTagWidth );
}

void rp_ContainerObject::_setCloseTagText(UT_UCS4String* pString)
{
	REPLACEP(m_pCloseTagString, pString);
	_recalcCloseTagTextWidth();
}

void rp_ContainerObject::_recalcCloseTagTextWidth()
{
	GR_Graphics *pG = getLayout()->getDocLayout()->getGraphics();
	
	UT_return_if_fail(pG);
	UT_return_if_fail(m_pCloseTagString);
	//UT_return_if_fail(m_pFont); // TODO: add getFont function, or use from pGr - MARCM
	
	m_iCloseTagTextWidth = pG->measureString(m_pCloseTagString->ucs4_str(), 0, m_pCloseTagString->size(), NULL);
	_recalcCloseTagWidth();
}

void rp_ContainerObject::_recalcCloseTagWidth()
{
	m_iCloseTagWidth = m_iCloseTagTextWidth + m_iArrowWidth + 2 * getTagBorder();
	_recalcWidth();
}

void rp_ContainerObject::layout()
{
	rl_ContainerLayout* pCL = static_cast<rl_ContainerLayout*>(getLayout());
	rp_ContainerObject* pObject = static_cast<rp_ContainerObject*>(pCL->getObject());
	GR_Graphics *pG = pCL->getDocLayout()->getGraphics();
	
	UT_ASSERT(pObject);
	
	rl_Layout* pPrev = pCL->getPrev();
	if (pPrev)
	{
		rp_Object* pPrevObject = pPrev->getObject();
		UT_ASSERT(pPrevObject);
		
		if (pPrev->breakAfter())
		{
			pObject->setX( 0 );
			pObject->setY( pPrevObject->getY() + pPrevObject->getHeight() + pG->tlu(1) /* add some extra spacing between the lines */ );
			
			rl_ContainerLayout* pParentLayout = static_cast<rl_ContainerLayout*>(pCL->getParent());
			if (pParentLayout)
			{
				// It seems we have a parent
				rp_ContainerObject* pParentObject = static_cast<rp_ContainerObject*>(pParentLayout->getObject());
				UT_return_if_fail(pParentObject);
				
				// update the parent's height with the height of this child
				pParentObject->setHeight(pParentObject->getHeight() + pObject->getHeight() + pG->tlu(1));
			}
		}
		else
		{
			rp_Object* pPrevObject = pPrev->getObject();
			UT_return_if_fail(pPrevObject)
				
			pObject->setX( pPrevObject->getX() + pPrevObject->getWidth() );
			pObject->setY( pPrevObject->getY() );
		}
	}
	else
	{
		// Our layout class is the first layout class in our parent's layout class,
		// or we don't have a parent at all, which makes our layout class a top level
		// structure.
	
		rl_ContainerLayout* pParentLayout = static_cast<rl_ContainerLayout*>(pCL->getParent());
		if (pParentLayout)
		{
			// It seems we have a parent
			rp_ContainerObject* pParentObject = static_cast<rp_ContainerObject*>(pParentLayout->getObject());
			UT_return_if_fail(pParentObject);
			
			pObject->setX( pParentObject->getX() + pParentObject->getTagWidth() );
			pObject->setY( pParentObject->getY() );
		}
		else
		{
			// We don't seem to have a parent, which makes our layout class
			// a top level one; ergo, our x position will be 0
			pObject->setX( 0 );
			pObject->setY( 0 ); // FIXME: this is wrong, and will fail when we have more than 1 section!!!!!! - MARCM
		}
	}

	// finally, layout all the children of this object
	UT_sint32 iChildWidth = 0;
	for (UT_uint32 i = 0; i < m_vecChildren.size(); i++)
	{
		rp_Object* pObject = m_vecChildren.getNthItem(i);
		pObject->layout();
		iChildWidth += pObject->getWidth();
	}
	
	// don't update the contents width when no childs are present
	// for example, in the rp_TextRun class, the contents width is determined
	// my measuring its string length, and not by the childs it contains (in fact,
	// a rp_TextRun class can't contain any children
	if (m_vecChildren.size() > 0)
		_setContentsWidth(iChildWidth);
}

void rp_ContainerObject::_drawOpenTag()
{
	GR_Graphics* pG = getLayout()->getDocLayout()->getGraphics();
	GR_Painter painter(pG);
	
	/* draw the box */
	pG->setColor3D(GR_Graphics::CLR3D_Foreground);
	painter.drawLine(getX(), getY(), getX() + getTagWidth() - pG->tlu(1) - _getArrowWidth(), getY());
	painter.drawLine(getX(), getY(), getX(), getY() + getTagHeight() - pG->tlu(1));
	painter.drawLine(getX(), getY() + getTagHeight() - pG->tlu(1), getX() + getTagWidth() - pG->tlu(1) - _getArrowWidth(), getY() + getTagHeight() - pG->tlu(1));
	
	/* draw the arrow */
	painter.drawLine(getX() + getTagWidth() - pG->tlu(1) - _getArrowWidth(), getY(), getX() + getTagWidth() - pG->tlu(1), getY() + getTagHeight()/2);
	painter.drawLine(getX() + getTagWidth() - pG->tlu(1), getY() + getTagHeight()/2 /*- pG->tlu(1)*/, getX() + getTagWidth() - pG->tlu(1) - _getArrowWidth(), getY() + getTagHeight() - pG->tlu(1));
	
	/* draw inner shaddow */
	pG->setColor3D(GR_Graphics::CLR3D_BevelUp);
	painter.drawLine(getX() + pG->tlu(1), getY() + pG->tlu(1), getX() + getTagWidth() - pG->tlu(1) - _getArrowWidth(), getY() + pG->tlu(1));
	painter.drawLine(getX() + pG->tlu(1), getY() + pG->tlu(1), getX() + pG->tlu(1), getY() + getTagHeight() - pG->tlu(2));
	
	/* draw the container description */
	pG->setColor3D(GR_Graphics::CLR3D_Foreground);
	UT_UCS4String* s = getText();
	if (s && s->size())
	{
		painter.drawChars(s->ucs4_str(), 0, s->size(), getX() + getTagBorder(), getY() + getTagBorder());
	}
	
	/* debug hack */
	/*UT_RGBColor r(255,0,0);
	pG->setColor(r);
	painter.drawLine(getX(), getY(), getX() + getWidth() - pG->tlu(1), getY());
	painter.drawLine(getX(), getY(), getX(), getY() + getHeight() - pG->tlu(1));
	painter.drawLine(getX(), getY() + getHeight() - pG->tlu(1), getX() + getWidth() - pG->tlu(1), getY() + getHeight() - pG->tlu(1));
	painter.drawLine(getX() + getWidth() - pG->tlu(1), getY(), getX() + getWidth() - pG->tlu(1), getY() + getHeight() - pG->tlu(1));*/
}

void rp_ContainerObject::_drawCloseTag()
{
	GR_Graphics* pG = getLayout()->getDocLayout()->getGraphics();
	GR_Painter painter(pG);
	
	rl_ContainerLayout* pLayout = static_cast<rl_ContainerLayout*>(getLayout());
	UT_return_if_fail(pLayout);

	UT_sint32 right = 0;
	UT_sint32 top = 0;
	UT_sint32 bottom = 0;
	UT_sint32 left = 0;
	
	rl_Layout* pChildLayout = pLayout->getLastChild();
	if (pChildLayout)
	{
		// we seem to have children in our layout class,
		// use the x, y, width and height of the last child in our container
		// layout class to position the close tag
		
		rp_Object* pChildObject = pChildLayout->getObject();
		UT_return_if_fail(pChildObject);

		left = pChildObject->getX() + pChildObject->getWidth();
		right = pChildObject->getX() + pChildObject->getWidth() + m_iCloseTagWidth - pG->tlu(1);
		top = pChildObject->getY();
		bottom = pChildObject->getY() + pChildObject->getHeight() - pG->tlu(1);
	}
	else
	{
		// we don't seem to have any children in our layout class,
		// use our own x, y, width and height to position the close tag
		left = getX() + getWidth() - m_iCloseTagWidth;	
		right = getX() + getWidth() - pG->tlu(1);
		top = getY();
		bottom = getY() + getHeight() - pG->tlu(1);
	}
	
	/* draw the box */
	pG->setColor3D(GR_Graphics::CLR3D_Foreground);
	painter.drawLine(right, top, right - m_iCloseTagWidth + pG->tlu(1) + _getArrowWidth(), top);
	painter.drawLine(right, top, right, bottom);
	painter.drawLine(right, bottom, right - m_iCloseTagWidth + pG->tlu(1) + _getArrowWidth(), bottom);
	
	/* draw the arrow */
	painter.drawLine(right - m_iCloseTagWidth + pG->tlu(1) + _getArrowWidth(), top, right - m_iCloseTagWidth + pG->tlu(1), top + getTagHeight()/2);
	painter.drawLine(right - m_iCloseTagWidth + pG->tlu(1), top + getTagHeight()/2, right - m_iCloseTagWidth + pG->tlu(1) + _getArrowWidth(), top + getTagHeight() - pG->tlu(1));
	
	/* draw inner shaddow */
	pG->setColor3D(GR_Graphics::CLR3D_BevelUp);
	// TODO: damn, too hard for my brain right now - MARCM
	// ...
	
	/* draw the container description */
	pG->setColor3D(GR_Graphics::CLR3D_Foreground);
	UT_UCS4String* s = _getCloseTagText();
	if (s && s->size())
	{
		painter.drawChars(s->ucs4_str(), 0, s->size(), right - m_iCloseTagWidth + pG->tlu(1) + _getArrowWidth() + getTagBorder(), top + getTagBorder());
	}
}

void rp_ContainerObject::draw()
{
	_drawOpenTag();
	
	for (UT_uint32 i = 0; i < m_vecChildren.size() ; i++)
	{
		rp_Object* pObject = m_vecChildren.getNthItem(i);
		pObject->draw();
	}
	
	_drawCloseTag();
}
