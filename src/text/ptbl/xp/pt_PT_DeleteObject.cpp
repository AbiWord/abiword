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
#include "px_ChangeRecord_Object.h"

/****************************************************************/
/****************************************************************/

UT_Bool pt_PieceTable::_deleteObjectWithNotify(PT_DocPosition dpos,
											   pf_Frag_Object * pfo, UT_uint32 fragOffset,
											   UT_uint32 length,
											   pf_Frag_Strux * pfs,
											   pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	// create a change record for this change and put it in the history.

	UT_ASSERT(pfs);
	UT_ASSERT(length == pfo->getLength());
	UT_ASSERT(fragOffset == 0);

	PX_ChangeRecord_Object * pcr
		= new PX_ChangeRecord_Object(PX_ChangeRecord::PXT_DeleteObject,
									 dpos, pfo->getIndexAP(), pfo->getObjectType());
	UT_ASSERT(pcr);

	// actually remove the fragment from the list and delete it.
	
	_deleteObject(pfo,ppfEnd,pfragOffsetEnd);
#if 0
	// TODO I think the following delete has already been done.
	delete pfo;
#endif	

	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);
	return UT_TRUE;
}

UT_Bool pt_PieceTable::_deleteObject(pf_Frag_Object * pfo,
									 pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	// unlink the object from the fragment list and try to
	// coalesce the neighboring fragments.
	
	_unlinkFrag(pfo,ppfEnd,pfragOffsetEnd);
	delete pfo;
	return UT_TRUE;
}

