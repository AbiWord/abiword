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

// insert-object-related routines for piece table.

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_FmtMark.h"
#include "pf_Frag_Text.h"
#include "pf_Frag_Strux.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_FmtMark.h"
#include "px_CR_FmtMarkChange.h"

#define SETP(p,v)	do { if (p) (*(p)) = (v); } while (0)

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool pt_PieceTable::_insertFmtMarkFragWithNotify(PTChangeFmt ptc,
													PT_DocPosition dpos,
													const XML_Char ** attributes,
													const XML_Char ** properties)
{
	UT_ASSERT(m_pts==PTS_Editing);

	pf_Frag * pf;
	PT_BlockOffset fo;

	// The return value of getFragFromPosition was never checked (sterwill)
	// bool bFound =
	getFragFromPosition(dpos,&pf,&fo);

	UT_ASSERT(pf);
	
	if ((fo==0) && (pf->getPrev()))
	{
		if (pf->getPrev()->getType() == pf_Frag::PFT_FmtMark)
		{
			// we are adjacent to another FmtMark.  we can just hack on this
			// one rather than inserting two consecutive marks.
			
			pf_Frag_FmtMark * pffm = static_cast<pf_Frag_FmtMark *>(pf->getPrev());
			pf_Frag_Strux * pfsContainer = NULL;
			bool bFoundStrux;
			bFoundStrux = _getStruxFromPosition(dpos,&pfsContainer);
			UT_ASSERT(bFoundStrux);

			return _fmtChangeFmtMarkWithNotify(ptc,pffm,dpos,attributes,properties,pfsContainer,NULL,NULL);
		}

		if (pf->getPrev()->getType() == pf_Frag::PFT_Text)
		{
			// if we are on a boundary between two frags and the one to our left (before us)
			// is a text fragment, we pretend to be at the end of it.

			pf = pf->getPrev();
			fo = pf->getLength();
		}
	}
	
	PT_AttrPropIndex indexOldAP = _chooseIndexAP(pf,fo);
	PT_AttrPropIndex indexNewAP;
	bool bMerged;
	bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP);
	UT_ASSERT(bMerged);

	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
		return true;
	
	pf_Frag_Strux * pfs = NULL;
	bool bFoundStrux;
	bFoundStrux = _getStruxFromFrag(pf,&pfs);
	UT_ASSERT(bFoundStrux);
	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pf) + fo;

	if (!_insertFmtMark(pf,fo,indexNewAP))
		return false;

	// create a change record, add it to the history, and notify
	// anyone listening.

	PX_ChangeRecord_FmtMark * pcr
		= new PX_ChangeRecord_FmtMark(PX_ChangeRecord::PXT_InsertFmtMark,
									  dpos,indexNewAP,blockOffset);
	UT_ASSERT(pcr);

	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return true;
}

bool pt_PieceTable::_insertFmtMark(pf_Frag * pf, UT_uint32 fragOffset, PT_AttrPropIndex api)
{
	pf_Frag_FmtMark * pffm = new pf_Frag_FmtMark(this,api);
	if (!pffm)
		return false;

	if (fragOffset == 0)
	{
		// we are at the beginning of a fragment, insert the
		// new FmtMark immediately prior to it.
		m_fragments.insertFrag(pf->getPrev(),pffm);
	}
	else if (fragOffset == pf->getLength())
	{
		// we are at the end of a fragment, insert the new
		// FmtMark immediately after it.
		m_fragments.insertFrag(pf,pffm);
	}
	else
	{
		// if the insert is in the middle of the (text) fragment, we
		// split the current fragment and insert the FmtMark between
		// them.

		UT_ASSERT(pf->getType() == pf_Frag::PFT_Text);
		pf_Frag_Text * pft = static_cast<pf_Frag_Text *>(pf);
		UT_uint32 lenTail = pft->getLength() - fragOffset;
		PT_BufIndex biTail = m_varset.getBufIndex(pft->getBufIndex(),fragOffset);
		pf_Frag_Text * pftTail = new pf_Frag_Text(this,biTail,lenTail,pft->getIndexAP(),pft->getField());
		if (!pftTail)
			goto MemoryError;
			
		pft->changeLength(fragOffset);
		m_fragments.insertFrag(pft,pffm);
		m_fragments.insertFrag(pffm,pftTail);
	}

	return true;

MemoryError:
	DELETEP(pffm);
	return false;
}
	
bool pt_PieceTable::_insertFmtMarkAfterBlockWithNotify(pf_Frag_Strux * pfsBlock,
														  PT_DocPosition dpos,
														  PT_AttrPropIndex api)
{
	UT_ASSERT(m_pts==PTS_Editing);

	PT_BlockOffset blockOffset = 0;

#ifdef DEBUG
	{
		PT_DocPosition dposTest = getFragPosition(pfsBlock) + pfsBlock->getLength();
		UT_ASSERT(dposTest == dpos);
	}
#endif

	if (!_insertFmtMark(pfsBlock,pfsBlock->getLength(),api))
		return false;

	// create a change record, add it to the history, and notify
	// anyone listening.

	PX_ChangeRecord_FmtMark * pcr
		= new PX_ChangeRecord_FmtMark(PX_ChangeRecord::PXT_InsertFmtMark,
									  dpos,api,blockOffset);
	UT_ASSERT(pcr);

	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfsBlock,pcr);

	return true;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool pt_PieceTable::_deleteFmtMarkWithNotify(PT_DocPosition dpos, pf_Frag_FmtMark * pffm,
												pf_Frag_Strux * pfs,
												pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	UT_ASSERT(m_pts==PTS_Editing);
	UT_ASSERT(pfs);

	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pffm);

	PX_ChangeRecord_FmtMark * pcr
		= new PX_ChangeRecord_FmtMark(PX_ChangeRecord::PXT_DeleteFmtMark,
									  dpos, pffm->getIndexAP(), blockOffset);
	UT_ASSERT(pcr);

	// actually remove the fragment from the list and delete it.

	_deleteFmtMark(pffm,ppfEnd,pfragOffsetEnd);
	
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);
	
	return true;
}

bool pt_PieceTable::_deleteFmtMark(pf_Frag_FmtMark * pffm,
									  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	// unlink the FmtMark from the fragment list and try to
	// coalesce the neighboring fragments.
	
	_unlinkFrag(pffm,ppfEnd,pfragOffsetEnd);
	delete pffm;
	return true;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool pt_PieceTable::_fmtChangeFmtMarkWithNotify(PTChangeFmt ptc, pf_Frag_FmtMark * pffm,
												   PT_DocPosition dpos, 
												   const XML_Char ** attributes, const XML_Char ** properties,
												   pf_Frag_Strux * pfs,
												   pf_Frag ** ppfNewEnd, UT_uint32 * pfragOffsetNewEnd)
{
	UT_ASSERT(m_pts==PTS_Editing);

	// apply a span-level change to the given FmtMark.
	// create a change record for this change and put it in the history.

	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pffm->getIndexAP();
	bool bMerged;
	bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP);
	UT_ASSERT(bMerged);

	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
	{
		SETP(ppfNewEnd, pffm->getNext());
		SETP(pfragOffsetNewEnd, 0);
		return true;
	}

	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pffm);
	
	// we do this before the actual change because various fields that
	// we need may be blown away during the change.  we then notify all
	// listeners of the change.

	PX_ChangeRecord_FmtMarkChange * pcr
		= new PX_ChangeRecord_FmtMarkChange(PX_ChangeRecord::PXT_ChangeFmtMark,
											dpos, indexOldAP,indexNewAP, blockOffset);
	UT_ASSERT(pcr);

	// apply the change to this fragment

	_fmtChangeFmtMark(pffm,indexNewAP,ppfNewEnd,pfragOffsetNewEnd);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return true;
}

bool pt_PieceTable::_fmtChangeFmtMark(pf_Frag_FmtMark * pffm,
										 PT_AttrPropIndex indexNewAP,
										 pf_Frag ** ppfNewEnd,
										 UT_uint32 * pfragOffsetNewEnd)
{
	// actually apply the format change.
	
	pffm->setIndexAP(indexNewAP);
	SETP(ppfNewEnd, pffm->getNext());
	SETP(pfragOffsetNewEnd, 0);
	return true;
}

bool pt_PieceTable::_insertFmtMarkFragWithNotify(PTChangeFmt ptc,
													PT_DocPosition dpos,
													PP_AttrProp *p_AttrProp)
{
	UT_ASSERT(p_AttrProp);

	const XML_Char * properties[] =	{ NULL, NULL, 0};

	int Index = 0;
	
	do
	{
		if(p_AttrProp->getNthProperty(Index, properties[0], properties[1]))
		{
			_insertFmtMarkFragWithNotify(ptc, dpos, NULL,
												properties);
		}
		else
		{
			break;
		}

		Index++;
	}
	while(true);

	const XML_Char * Attributes[] =	{ NULL, NULL, 0};

	Index = 0;
	
	do
	{
		if(p_AttrProp->getNthAttribute(Index, Attributes[0], Attributes[1]))
		{
			_insertFmtMarkFragWithNotify(ptc, dpos, Attributes,
												NULL);
		}
		else
		{
			break;
		}

		Index++;
	}
	while(true);

	return true;
}
