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

// change-object-related routines for piece table.

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Object.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Object.h"
#include "px_ChangeRecord_ObjectChange.h"

#define SETP(p,v)	do { if (p) (*(p)) = (v); } while (0)

/****************************************************************/
/****************************************************************/

UT_Bool pt_PieceTable::_fmtChangeObjectWithNotify(PTChangeFmt ptc,
												  pf_Frag_Object * pfo, UT_uint32 fragOffset,
												  PT_DocPosition dpos,
												  UT_uint32 length,
												  const XML_Char ** attributes,
												  const XML_Char ** properties,
												  pf_Frag_Strux * pfs,
												  pf_Frag ** ppfNewEnd,
												  UT_uint32 * pfragOffsetNewEnd)
{
	// apply a span-level change to the given object.
	// create a change record for this change and put it in the history.

	UT_ASSERT(length == pfo->getLength());
	UT_ASSERT(fragOffset == 0);
	
	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pfo->getIndexAP();
	UT_Bool bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP);
	UT_ASSERT(bMerged);

	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
	{
		SETP(ppfNewEnd, pfo->getNext());
		SETP(pfragOffsetNewEnd, 0);
		return UT_TRUE;
	}
	
	// we do this before the actual change because various fields that
	// we need may be blown away during the change.  we then notify all
	// listeners of the change.

	PX_ChangeRecord_ObjectChange * pcr
		= new PX_ChangeRecord_ObjectChange(PX_ChangeRecord::PXT_ChangeObject,
										   dpos, indexOldAP,indexNewAP,
										   pfo->getObjectType(),ptc);
	UT_ASSERT(pcr);

	// apply the change to this fragment

	_fmtChangeObject(pfo,indexNewAP,ppfNewEnd,pfragOffsetNewEnd);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return UT_TRUE;
}

UT_Bool pt_PieceTable::_fmtChangeObject(pf_Frag_Object * pfo,
										PT_AttrPropIndex indexNewAP,
										pf_Frag ** ppfNewEnd,
										UT_uint32 * pfragOffsetNewEnd)
{
	// actually apply the format change.
	
	pfo->setIndexAP(indexNewAP);
	SETP(ppfNewEnd, pfo->getNext());
	SETP(pfragOffsetNewEnd, 0);
	return UT_TRUE;
}


