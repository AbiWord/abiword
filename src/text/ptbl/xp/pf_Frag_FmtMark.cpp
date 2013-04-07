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


#include "pf_Frag_FmtMark.h"
#include "pt_PieceTable.h"
#include "px_ChangeRecord.h"
#include "px_CR_FmtMark.h"


pf_Frag_FmtMark::pf_Frag_FmtMark(pt_PieceTable * pPT,
								 PT_AttrPropIndex indexAP)
	: pf_Frag(pPT,pf_Frag::PFT_FmtMark,0)
{
	m_indexAP = indexAP;
}

pf_Frag_FmtMark::~pf_Frag_FmtMark()
{
}

bool pf_Frag_FmtMark::createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
												   PT_DocPosition dpos,
												   PT_BlockOffset blockOffset) const
{
	UT_return_val_if_fail (ppcr,false);
	
	PX_ChangeRecord * pcr
		= new PX_ChangeRecord_FmtMark(PX_ChangeRecord::PXT_InsertFmtMark,
									  dpos, m_indexAP, blockOffset);
	if (!pcr)
		return false;

	*ppcr = pcr;
	return true;
}
