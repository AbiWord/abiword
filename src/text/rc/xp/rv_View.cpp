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
 
 #include "rv_View.h"
 
 rv_View::rv_View(XAP_App * pApp, void* pParentData, rl_DocLayout* pLayout) :
	AV_View(pApp, pParentData),
	m_pLayout(pLayout),
	//m_pDoc(pLayout->getDocument()),
	m_pG(m_pLayout->getGraphics())
{
	UT_ASSERT(pLayout);
	pLayout->setView(this);
}

rv_View::~rv_View()
{
}

void rv_View::focusChange(AV_Focus focus)
{
	UT_DEBUGMSG(("rv_View::focusChange\n"));
}

void rv_View::setXScrollOffset(UT_sint32)
{
	UT_DEBUGMSG(("rv_View::setXScrollOffset\n"));
}

void rv_View::setYScrollOffset(UT_sint32)
{
	UT_DEBUGMSG(("rv_View::setYScrollOffset\n"));
}

void rv_View::setCursorToContext(void)
{
	UT_DEBUGMSG(("rv_View::setCursorToContext\n"));
}

void rv_View::draw(const UT_Rect* pClipRect)
{
	/*if (getPoint() == 0)
	{
		return; FIXME: enable this when we have points going here...
	}*/ 
	
	if (pClipRect)
	{
		_draw(pClipRect->left, pClipRect->top, pClipRect->width, pClipRect->height, true);
	}
	else
	{
		_draw(0, 0, getWindowWidth(), getWindowHeight(), false);
	}
}

void rv_View::_draw(UT_sint32 x, UT_sint32 y,
			UT_sint32 width, UT_sint32 height,
			bool bClip)
{
	GR_Painter painter(m_pG);

	if ((getWindowWidth() <= 0) || (getWindowHeight() <= 0))
	{
		UT_DEBUGMSG(("rv_View::draw() called with zero drawing area.\n"));
		return;
	}

	if ((width <= 0) || (height <= 0))
	{
		UT_DEBUGMSG(("rv_View::draw() called with zero width or height expose.\n"));
		return;
	}
	
	UT_Rect rClip;
	if (bClip)
	{
		rClip.left = x;
		rClip.top = y;
		rClip.width = width;
		rClip.height = height;
		m_pG->setClipRect(&rClip);
	}
	
	/* clear the background */	
	painter.fillRect(GR_Graphics::CLR3D_Background, x, y, width, height);

	/* draw the containers */
	for (UT_uint32 i = 0; i < m_pLayout->getNumContainers(); i++)
	{
		rl_ContainerLayout* pCLayout = m_pLayout->getNthContainer(i);
		if (pCLayout->getObject()) // FIXME: should not be needed!!!
			pCLayout->getObject()->draw();
	}	
}

void rv_View::updateScreen(bool bDirtyRunsOnly)
{
	UT_DEBUGMSG(("rv_View::updateScreen\n"));
}

void rv_View::updateLayout(void)
{
	UT_DEBUGMSG(("rv_View::updateLayout\n"));
}

void rv_View::cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos)
{
	UT_DEBUGMSG(("rv_View::cmdScroll\n"));
}

bool rv_View::notifyListeners(const AV_ChangeMask hint)
{
	/* FIXME: HANDLE ALL CASES HERE - MARCM */
	return AV_View::notifyListeners(hint);
}
