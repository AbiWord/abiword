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


#include "pf_Frag_Strux.h"
#include "px_ChangeRecord.h"
#include "px_CR_Strux.h"


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

bool pf_Frag_Strux::setFmtHandle(PL_ListenerId lid, PL_StruxFmtHandle sfh)
{
	return (m_vecFmtHandle.setNthItem(lid,(void *)sfh,NULL) == 0);
}

PT_AttrPropIndex pf_Frag_Strux::getIndexAP(void) const
{
	return m_indexAP;
}

void pf_Frag_Strux::setIndexAP(PT_AttrPropIndex indexNewAP)
{
	m_indexAP = indexNewAP;
}

bool pf_Frag_Strux::createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
												 PT_DocPosition dpos) const
{
	UT_ASSERT(ppcr);
	
	PX_ChangeRecord_Strux * pcr
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_InsertStrux,
									dpos, m_indexAP, m_struxType);
	if (!pcr)
		return false;

	*ppcr = pcr;
	return true;
}
