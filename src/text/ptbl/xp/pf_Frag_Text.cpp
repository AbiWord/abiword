/* AbiWord
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


#include "pf_Frag_Text.h"
#include "pt_PieceTable.h"
#include "px_ChangeRecord.h"
#include "px_CR_Span.h"
#include "ut_debugmsg.h"

pf_Frag_Text::pf_Frag_Text(pt_PieceTable * pPT,
						   PT_BufIndex bufIndex,
						   UT_uint32 length,
						   PT_AttrPropIndex indexAP,
                           fd_Field * pField)
	: pf_Frag(pPT,pf_Frag::PFT_Text,length)
{
	m_bufIndex = bufIndex;
    m_indexAP = indexAP;
    m_pField = pField;
}

pf_Frag_Text::~pf_Frag_Text()
{
}

void pf_Frag_Text::setIndexAP(PT_AttrPropIndex indexNewAP)
{
	m_indexAP = indexNewAP;
}

bool pf_Frag_Text::createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
												PT_DocPosition dpos,
												PT_BlockOffset blockOffset) const
{
	UT_ASSERT(ppcr);
	
	PX_ChangeRecord * pcr
		= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_InsertSpan,
								   dpos, m_indexAP,
								   m_bufIndex,m_length,blockOffset,m_pField);
	if (!pcr)
		return false;

	*ppcr = pcr;
	return true;
}

bool pf_Frag_Text::createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
												PT_DocPosition dpos,
												PT_BlockOffset blockOffset,
												PT_BlockOffset startFragOffset,
												PT_BlockOffset endFragOffset) const
{
	UT_ASSERT(ppcr);
	UT_ASSERT(endFragOffset <= m_length);
	UT_ASSERT(startFragOffset < endFragOffset);
	
	PX_ChangeRecord * pcr
		= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_InsertSpan,
								   dpos+startFragOffset,
								   m_indexAP,
								   m_bufIndex+startFragOffset,
								   (endFragOffset-startFragOffset),
								   blockOffset+startFragOffset,m_pField);
	if (!pcr)
		return false;

	*ppcr = pcr;
	return true;
}

void pf_Frag_Text::changeLength(UT_uint32 newLength)
{
	UT_ASSERT(newLength > 0);
	m_length = newLength;
	m_pPieceTable->getFragments().setFragsDirty();
}

void pf_Frag_Text::adjustOffsetLength(PT_BufIndex bi, UT_uint32 newLength)
{
	m_bufIndex = bi;
	m_length = newLength;
	m_pPieceTable->getFragments().setFragsDirty();
}

void pf_Frag_Text::setField(fd_Field * pField)
{
    m_pField = pField;
}
