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

bool pf_Frag_Strux::_isContentEqual(const pf_Frag &f2) const
{
	if(!pf_Frag::_isContentEqual(f2))
		return false;

	if(m_struxType != ((const pf_Frag_Strux &)(f2)).getStruxType())
		return false;

	return true;
}

PTStruxType pf_Frag_Strux::getStruxType(void) const
{
	return m_struxType;
}

fl_ContainerLayout* pf_Frag_Strux::getFmtHandle(PL_ListenerId lid) const
{
	if (m_vecFmtHandle.size() <= static_cast<int>(lid)) return 0;
	return m_vecFmtHandle.getNthItem(lid);
}

bool pf_Frag_Strux::setFmtHandle(PL_ListenerId lid, fl_ContainerLayout* sfh)
{
	return (m_vecFmtHandle.setNthItem(lid,sfh,NULL) == 0);
}

bool pf_Frag_Strux::createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
												 PT_DocPosition dpos) const
{
	UT_return_val_if_fail (ppcr,false);
	
	PX_ChangeRecord_Strux * pcr
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_InsertStrux,
									dpos, m_indexAP, getXID(), m_struxType);
	if (!pcr)
		return false;

	*ppcr = pcr;
	return true;
}

bool pf_Frag_Strux::usesXID() const
{
	switch (m_struxType)
	{
		case PTX_Section:
		case PTX_Block:
		case PTX_SectionHdrFtr:
		case PTX_SectionEndnote:
		case PTX_SectionTable:
		case PTX_SectionCell:
		case PTX_SectionFootnote:
		case PTX_SectionMarginnote:
		case PTX_SectionFrame:
		case PTX_SectionTOC:
			return true;

		default:
			return false;
	}

	return false;
}

bool pf_Frag_Strux::isMatchingType(const pf_Frag * pf) const
{
	UT_return_val_if_fail( pf, false );

	if(pf->getType() != pf_Frag::PFT_Strux)
		return false;

	const pf_Frag_Strux * pfs = (const pf_Frag_Strux*)pf;

	return isMatchingType(pfs->getStruxType());
}


bool pf_Frag_Strux::isMatchingType(PTStruxType eType) const
{
	switch(getStruxType())
	{
		case PTX_Section:
		case PTX_Block:
		case PTX_SectionHdrFtr:
		case PTX_StruxDummy:        return false;
					
		case PTX_SectionEndnote:    return eType == PTX_EndEndnote;
		case PTX_SectionTable:      return eType == PTX_EndTable;
		case PTX_SectionCell:       return eType == PTX_EndCell;
		case PTX_SectionFootnote:   return eType == PTX_EndFootnote;
		case PTX_SectionMarginnote: return eType == PTX_EndMarginnote;
		case PTX_SectionFrame:      return eType == PTX_EndFrame;
		case PTX_SectionTOC:        return eType == PTX_EndTOC;
		case PTX_EndCell:           return eType == PTX_SectionCell;
		case PTX_EndTable:          return eType == PTX_SectionTable;
		case PTX_EndFootnote:       return eType == PTX_SectionFootnote;
		case PTX_EndMarginnote:     return eType == PTX_SectionMarginnote;
		case PTX_EndEndnote:        return eType == PTX_SectionEndnote;
		case PTX_EndFrame:          return eType == PTX_SectionFrame;
		case PTX_EndTOC:            return eType == PTX_SectionTOC;

		default:
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
	}

	return false;
}

