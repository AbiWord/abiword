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


#include "pf_Frag_Strux_Block.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Strux.h"

#define pf_FRAG_STRUX_BLOCK_LENGTH 1

/*****************************************************************/
/*****************************************************************/

pf_Frag_Strux_Block::pf_Frag_Strux_Block(pt_PieceTable * pPT,
										 PT_AttrPropIndex indexAP)
	: pf_Frag_Strux(pPT,PTX_Block,pf_FRAG_STRUX_BLOCK_LENGTH,indexAP)
{
	m_preferredSpanAPI = 0;
}

pf_Frag_Strux_Block::~pf_Frag_Strux_Block()
{
}

void pf_Frag_Strux_Block::setPreferredSpanFmt(PT_AttrPropIndex api)
{
	m_preferredSpanAPI = api;
}

PT_AttrPropIndex pf_Frag_Strux_Block::getPreferredSpanFmt(void) const
{
	return m_preferredSpanAPI;
}

