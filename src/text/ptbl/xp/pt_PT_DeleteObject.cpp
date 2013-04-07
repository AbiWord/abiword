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


// deleteSpan-related routines for class pt_PieceTable

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"

/****************************************************************/
/****************************************************************/

bool pt_PieceTable::_deleteObjectWithNotify(PT_DocPosition dpos,
											   pf_Frag_Object * pfo, UT_uint32 fragOffset,
											   UT_uint32 length,
											   pf_Frag_Strux * pfs,
											   pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd,
											   bool bAddChangeRec)
{
	// create a change record for this change and put it in the history.

	UT_return_val_if_fail (pfs,false);
	UT_return_val_if_fail (length == pfo->getLength(), false);
	UT_return_val_if_fail (fragOffset == 0,false);

	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pfo) + fragOffset;

	PX_ChangeRecord_Object * pcr
		= new PX_ChangeRecord_Object(PX_ChangeRecord::PXT_DeleteObject,
									 dpos, pfo->getIndexAP(), pfo->getXID(), pfo->getObjectType(),
									 blockOffset, pfo->getField(),pfo);
	UT_return_val_if_fail (pcr, false);

	// actually remove the fragment from the list and delete it.

	_deleteObject(pfo,ppfEnd,pfragOffsetEnd);

	if (bAddChangeRec)
		m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);
	if (!bAddChangeRec)
		delete pcr;
	
	return true;
}

bool pt_PieceTable::_deleteObject(pf_Frag_Object * pfo,
									 pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	// unlink the object from the fragment list and try to
	// coalesce the neighboring fragments.
        xxx_UT_DEBUGMSG(("Deleting Frag object %p  \n",pfo));
	_unlinkFrag(pfo,ppfEnd,pfragOffsetEnd);
	delete pfo;
	return true;
}

