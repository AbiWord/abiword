 
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

#include "pf_Frag_Strux.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Strux.h"


pf_Frag_Strux::pf_Frag_Strux(pt_PieceTable * pPT,
							 PTStruxType struxType,
							 UT_uint32 length,
							 PT_AttrPropIndex indexAP)
	: pf_Frag(pPT, pf_Frag::PFT_Strux, length)
{
	m_struxType = struxType;
	m_indexAP = indexAP;
}

pf_Frag_Strux::~pf_Frag_Strux()
{
	// we do not purge the items in m_vecFmtHandle
	// since we did not allocate them.
}

PTStruxType pf_Frag_Strux::getStruxType(void) const
{
	return m_struxType;
}

PL_StruxFmtHandle pf_Frag_Strux::getFmtHandle(PL_ListenerId lid) const
{
	return (PL_StruxFmtHandle)m_vecFmtHandle.getNthItem(lid);
}

UT_Bool pf_Frag_Strux::setFmtHandle(PL_ListenerId lid, PL_StruxFmtHandle sfh)
{
	UT_uint32 kLimit = m_vecFmtHandle.getItemCount();
	if (lid < kLimit)
		return (m_vecFmtHandle.setNthItem(lid,(void *)sfh,NULL) == 0);
	else if (lid == kLimit)
		return (m_vecFmtHandle.addItem((void *)sfh) == 0);

	// TODO we need to fix the vector class so that i can do a setNthItem
	// TODO and have it automatically grow the vector -- or we need to do
	// TODO an addItem(0) until we get to lid-1 and then addItem(sfh).
	UT_ASSERT((0));
	return UT_FALSE;
}

PT_AttrPropIndex pf_Frag_Strux::getIndexAP(void) const
{
	return m_indexAP;
}

void pf_Frag_Strux::setIndexAP(PT_AttrPropIndex indexNewAP)
{
	m_indexAP = indexNewAP;
}

UT_Bool pf_Frag_Strux::createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
												 PT_DocPosition dpos) const
{
	UT_ASSERT(ppcr);
	
	PX_ChangeRecord_Strux * pcr
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_InsertStrux,
									dpos,
									m_indexAP,m_indexAP,
									UT_FALSE,UT_FALSE,
									m_struxType);
	if (!pcr)
		return UT_FALSE;

	*ppcr = pcr;
	return UT_TRUE;
}
