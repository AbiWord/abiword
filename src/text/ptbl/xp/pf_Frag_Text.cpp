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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */


#include "pf_Frag_Text.h"
#include "pt_PieceTable.h"
#include "px_ChangeRecord.h"
#include "px_CR_Span.h"
#include "pd_Iterator.h"
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

bool pf_Frag_Text::_isContentEqual(const pf_Frag & f2) const
{
	if(!pf_Frag::_isContentEqual(f2))
		return false;

	// NB: this we are asked to strictly compare 2 frags
	if(getLength() != f2.getLength())
		return false;

	pf_Frag * pf2 = const_cast<pf_Frag *>(&f2);
	
	PD_DocIterator t1(* (m_pPieceTable->getDocument()), getPos());
	PD_DocIterator t2(* (pf2->getPieceTable()->getDocument()), f2.getPos());

	UT_uint32 iLen = UT_MIN(getLength(), f2.getLength());
	UT_uint32 i = 0;
	
	while(i < iLen && t1.getStatus() == UTIter_OK && t2.getStatus() == UTIter_OK)
	{
		if(t1.getChar() != t2.getChar())
			return false;

		++i;
		++t1;
		++t2;
	}

	return true;
}



bool pf_Frag_Text::createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
												PT_DocPosition dpos,
												PT_BlockOffset blockOffset) const
{
	UT_return_val_if_fail (ppcr,false);
	
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
	UT_return_val_if_fail (ppcr,false);
	UT_return_val_if_fail (endFragOffset <= m_length,false);
	UT_return_val_if_fail (startFragOffset < endFragOffset,false);
	
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
	UT_ASSERT_HARMLESS(newLength > 0);
	UT_sint32 delta = static_cast<UT_sint32>(newLength) -  static_cast<UT_sint32>(m_length);
	m_length = newLength;
	lengthChanged(delta);
}

void pf_Frag_Text::adjustOffsetLength(PT_BufIndex bi, UT_uint32 newLength)
{
	m_bufIndex = bi;
	UT_sint32 delta = static_cast<UT_sint32>(newLength) -  static_cast<UT_sint32>(m_length);
	m_length = newLength;
	lengthChanged(delta);
}

void pf_Frag_Text::setField(fd_Field * pField)
{
    m_pField = pField;
}

std::string pf_Frag_Text::toString() const
{
    PT_BufIndex startidx = getBufIndex();
    const UT_UCSChar* ucsstart = m_pPieceTable->getPointer( startidx );
	UT_UTF8String ustr( ucsstart, getLength() );
    return ustr.utf8_str();
}
