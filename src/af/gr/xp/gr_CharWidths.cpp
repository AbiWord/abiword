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

#include <string.h>
#include "gr_CharWidths.h"

GR_CharWidths::GR_CharWidths(void)
{
	memset(m_aLatin1.aCW,0,sizeof(m_aLatin1.aCW));
}

GR_CharWidths::~GR_CharWidths(void)
{
	UT_VECTOR_SPARSEPURGEALL(Array256*, m_vecHiByte);
}

void GR_CharWidths::zeroWidths(void)
{
	memset(m_aLatin1.aCW,0,sizeof(m_aLatin1.aCW));
	UT_VECTOR_SPARSEPURGEALL(Array256*, m_vecHiByte);
	m_vecHiByte.clear();
}

void GR_CharWidths::setWidth(UT_UCSChar cIndex, UT_sint32 width)
{
	// remember a width for the given character.
	
	UT_uint32 hi = ((cIndex >> 8) & 0xff);
	UT_uint32 lo = (cIndex & 0xff);

	if (!hi)							// char is in latin1
	{
		m_aLatin1.aCW[lo] = width;
		return;
	}

	Array256 * pA = NULL;
	if (m_vecHiByte.getItemCount() > hi)
		pA = (Array256 *)m_vecHiByte.getNthItem(hi);
	if (!pA)
	{
		pA = new Array256;
		if (!pA)						// silently fail on memory problems
			return;
		memset(pA,0,sizeof(Array256));
	}

	m_vecHiByte.setNthItem(hi,pA,NULL);
	pA->aCW[lo] = width;
	return;
}

UT_sint32 GR_CharWidths::getWidth(UT_UCSChar cIndex) const
{
	// we only know the widths that we have been told.
	// if we haven't been told anything for a char, we
	// return zero.
	
	UT_uint32 hi = ((cIndex >> 8) & 0xff);
	UT_uint32 lo = (cIndex & 0xff);

	if (!hi)
		return m_aLatin1.aCW[lo];

	if (m_vecHiByte.getItemCount() > hi)
	{
		Array256 * pA = (Array256 *)m_vecHiByte.getNthItem(hi);
		if (pA)
			return pA->aCW[lo];
	}

	return 0;
}
