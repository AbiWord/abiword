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

#include "rl_DocLayout.h"
#include "rl_DocListener.h"

rl_DocLayout::rl_DocLayout(PD_Document* doc, GR_Graphics* pG) :
	m_pDoc(doc),
	m_pG(pG),
	m_pView(NULL),
	m_pAvView(NULL),
	m_pDocListener(NULL),
	m_pCurrentCL(NULL),
	m_lid(0)
{
}

rl_DocLayout::~rl_DocLayout()
{
	if (m_pDoc)
	{
		m_pDoc->removeListener(m_lid);
	}

	DELETEP(m_pDocListener);
}
	
void rl_DocLayout::fillLayouts(void)
{
	m_pDocListener = new rl_DocListener(m_pDoc, this);
	UT_ASSERT(m_pDocListener);
	
	m_pDoc->addListener(m_pDocListener, &m_lid);
}

void rl_DocLayout::formatAll(void)
{
	UT_DEBUGMSG(("rl_DocLayout::formatAll()\n"));

	for (UT_uint32 i = 0; i < m_vecContainers.size(); i++)
	{
		rl_ContainerLayout* pCLayout = static_cast<rl_ContainerLayout*>(m_vecContainers.getNthItem(i));
		pCLayout->format();
	}
}

void rl_DocLayout::add(rl_Layout *pLayout)
{
	if ( m_pCurrentCL == NULL)
		m_vecContainers.addItem(pLayout);
	else
		m_pCurrentCL->addChild(pLayout);
	
	if (pLayout->isContainer())
		m_pCurrentCL = static_cast<rl_ContainerLayout*>(pLayout);
}

void rl_DocLayout::closeContainer(PTStruxType type, bool backtrack)
{
	if (backtrack)
	{
		while (m_pCurrentCL && m_pCurrentCL->getType() != type)
			m_pCurrentCL = m_pCurrentCL->getParent();
		
		if (m_pCurrentCL)
			m_pCurrentCL = m_pCurrentCL->getParent();
	}
	else
	{
		if (m_pCurrentCL->getType() == type)
			m_pCurrentCL = m_pCurrentCL->getParent();
	}
}

void rl_DocLayout::closeAll()
{
	m_pCurrentCL = NULL;
}
