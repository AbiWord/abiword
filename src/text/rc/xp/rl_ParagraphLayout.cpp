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

#include "rl_ParagraphLayout.h"
#include "rl_SpanLayout.h"

rl_ParagraphLayout::rl_ParagraphLayout(PTStruxType type, PL_StruxDocHandle sdh, rl_ContainerLayout* pParent, rl_DocLayout* pDocLayout) :
	rl_ContainerLayout(type, sdh, pParent, pDocLayout)
{
}

bool rl_ParagraphLayout::doclistener_insertSpan(const PX_ChangeRecord_Span * pcrs)
{
	UT_ASSERT(pcrs->getType()==PX_ChangeRecord::PXT_InsertSpan);
	
	PT_BlockOffset blockOffset = pcrs->getBlockOffset();
	UT_uint32 len = pcrs->getLength();
	UT_ASSERT(len>0);

	PT_BufIndex bi = pcrs->getBufIndex();
	const UT_UCSChar* pChars = m_pDoc->getPointer(bi);	
	
	addChild(new rl_SpanLayout(PTX_StruxDummy, NULL, this, getDocLayout(), pChars, len));
	
	return true;
}

rp_Object* rl_ParagraphLayout::format()
{
	m_bBreakAfter = true;
	if (getParent()) getParent()->breakAfter(this);
		
	return rl_ContainerLayout::format();
}
