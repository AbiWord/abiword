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

#include "pd_Style.h"
#include "pt_PieceTable.h"

PD_Style::PD_Style(pt_PieceTable * pPT, PT_AttrPropIndex indexAP)
{
	m_pPT = pPT;
	m_indexAP = indexAP;
	m_pBasedOn = NULL;
	m_pFollowedBy = NULL;
}

PD_Style::~PD_Style()
{
}

UT_Bool PD_Style::setIndexAP(PT_AttrPropIndex indexAP)
{
	UT_ASSERT(indexAP != m_indexAP);

	// TODO: may need to rebind, handle undo, etc.

	m_indexAP = indexAP;

	return UT_TRUE;
}

UT_Bool PD_Style::isUsed(void) const
{
	// TODO: we need some way of refcounting
	// TODO: what if this document is a template

	// for now, we cheat
	return isUserDefined();
}

//////////////////////////////////////////////////////////////////
// a sub-class to wrap the compiled-in styles
//////////////////////////////////////////////////////////////////

PD_BuiltinStyle::PD_BuiltinStyle(pt_PieceTable * pPT, PT_AttrPropIndex indexAP)
	: PD_Style(pPT, indexAP)
{
	m_indexAPOrig = indexAP;
}

PD_BuiltinStyle::~PD_BuiltinStyle()
{
}

