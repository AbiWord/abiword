/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
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
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "fd_Field.h"

#define SETP(p,v)	do { if (p) (*(p)) = (v); } while (0)

/****************************************************************/
/****************************************************************/

bool pt_PieceTable::changeObjectFormatNoUpdate(PTChangeFmt ptc ,pf_Frag_Object * pfo, const PP_PropertyVector & attributes, const PP_PropertyVector & properties)
{
	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pfo->getIndexAP();
	bool bMerged = m_varset.mergeAP(ptc,indexOldAP, attributes, properties, &indexNewAP,getDocument());
	UT_UNUSED(bMerged);
	UT_ASSERT_HARMLESS(bMerged);

	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
	{
		return true;
	}
	// actually apply the format change.
	
	pfo->setIndexAP(indexNewAP);
	return true;
}

bool pt_PieceTable::_fmtChangeObjectWithNotify(PTChangeFmt ptc,
                                               pf_Frag_Object * pfo, UT_uint32 fragOffset,
                                               PT_DocPosition dpos,
                                               UT_uint32 length,
                                               const PP_PropertyVector & attributes,
                                               const PP_PropertyVector & properties,
                                               pf_Frag_Strux * pfs,
                                               pf_Frag ** ppfNewEnd,
                                               UT_uint32 * pfragOffsetNewEnd,
											   bool bRevisionDelete)
{
	// apply a span-level change to the given object.
	// create a change record for this change and put it in the history.

	UT_return_val_if_fail (length == pfo->getLength(), false);
	UT_return_val_if_fail (fragOffset == 0, false);

	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pfo->getIndexAP();
	bool bMerged = m_varset.mergeAP(ptc, indexOldAP, attributes, properties, &indexNewAP, getDocument());
	UT_UNUSED(bMerged);
	UT_ASSERT_HARMLESS(bMerged);

	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
	{
		SETP(ppfNewEnd, pfo->getNext());
		SETP(pfragOffsetNewEnd, 0);
		return true;
	}

	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pfo) + fragOffset;
	
	// we do this before the actual change because various fields that
	// we need may be blown away during the change.  we then notify all
	// listeners of the change.

	PX_ChangeRecord_ObjectChange * pcr
		= new PX_ChangeRecord_ObjectChange(PX_ChangeRecord::PXT_ChangeObject,
										   dpos, indexOldAP,indexNewAP,
										   pfo->getObjectType(),blockOffset,bRevisionDelete);
	UT_return_val_if_fail (pcr, false);

	// apply the change to this fragment

	_fmtChangeObject(pfo,indexNewAP,ppfNewEnd,pfragOffsetNewEnd);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return true;
}

bool pt_PieceTable::_fmtChangeObject(pf_Frag_Object * pfo,
										PT_AttrPropIndex indexNewAP,
										pf_Frag ** ppfNewEnd,
										UT_uint32 * pfragOffsetNewEnd)
{
	// actually apply the format change.
	
	pfo->setIndexAP(indexNewAP);
    //    if (pfo->getField()) pfo->getField()->update();
	SETP(ppfNewEnd, pfo->getNext());
	SETP(pfragOffsetNewEnd, 0);
	return true;
}


