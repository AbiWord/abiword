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
 
#include "rl_SpanLayout.h"
#include "rp_TextRun.h"

rl_SpanLayout::rl_SpanLayout(PTStruxType type, PL_StruxDocHandle sdh, rl_ContainerLayout* pParent, rl_DocLayout *pDocLayout,
	const UT_UCSChar* pChars, UT_uint32 len) :
	rl_ContainerLayout(type, sdh, pParent, pDocLayout),
	m_pChars(pChars),
	m_iLength(len)
{
}
