 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#include "ut_types.h"
#include "pf_Frag.h"
#include "pt_PieceTable.h"

pf_Frag::pf_Frag(pt_PieceTable * pPT, PFType type, UT_uint32 length)
{
	m_type = type;
	m_length = length;
	m_next = NULL;
	m_prev = NULL;
	m_pPieceTable = pPT;
}

pf_Frag::~pf_Frag()
{
}

pf_Frag::PFType pf_Frag::getType(void) const
{
	return m_type;
}

pf_Frag * pf_Frag::getNext(void) const
{
	return m_next;
}

pf_Frag * pf_Frag::getPrev(void) const
{
	return m_prev;
}

pf_Frag * pf_Frag::setNext(pf_Frag * pNext)
{
	pf_Frag * pOld = m_next;
	m_next = pNext;
	return pOld;
}

pf_Frag * pf_Frag::setPrev(pf_Frag * pPrev)
{
	pf_Frag * pOld = m_prev;
	m_prev = pPrev;
	return pOld;
}

UT_uint32 pf_Frag::getLength(void) const
{
	return m_length;
}

UT_Bool pf_Frag::createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
										   PT_DocPosition dpos) const
{
	// this function must be overloaded.
	UT_ASSERT(0);
	return UT_TRUE;
}
