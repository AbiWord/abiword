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

#include "rp_Object.h"
#include "rl_DocLayout.h"

rp_Object::rp_Object(rl_Layout* pLayout) :
	m_pLayout(pLayout),
	m_iX(0),
	m_iY(0),
	m_iTagWidth(0),
	m_iWidth(0),
	m_pString(NULL),
	m_iTextWidth(0)
{
	GR_Graphics *pG = getLayout()->getDocLayout()->getGraphics();

	m_iTagBorder = pG->tlu(1); // used for the top, left, right and bottom borders
	
	m_pFont = pG->findFont("Times New Roman", "normal", "normal", "normal", "normal", "12pt");
	m_iHeight = pG->getFontHeight(m_pFont) + 2 * m_iTagBorder; // TODO: do I need this?
	m_iTagHeight = pG->getFontHeight(m_pFont) + 2 * m_iTagBorder;
	
	pG->setFont(m_pFont);
}

void rp_Object::setText(UT_UCS4String* pString)
{
	REPLACEP(m_pString, pString);
	_recalcTextWidth();
	_recalcTagWidth();
	_recalcWidth();
}

void rp_Object::expandAttrProp()
{
	
}

void rp_Object::_recalcTextWidth()
{
	GR_Graphics *pG = getLayout()->getDocLayout()->getGraphics();
	
	UT_return_if_fail(pG);
	UT_return_if_fail(m_pString);
	UT_return_if_fail(m_pFont);
	
	m_iTextWidth = pG->measureString(m_pString->ucs4_str(), 0, m_pString->size(), NULL);
}

void rp_Object::_recalcTagWidth()
{
	GR_Graphics *pG = getLayout()->getDocLayout()->getGraphics();
	m_iTagWidth = m_iTextWidth + 2 * m_iTagBorder;
}

void rp_Object::_recalcWidth()
{
	m_iWidth = m_iTagWidth; // wow, that looks hard
}
