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
													const PP_PropertyVector & attributes,
													const PP_PropertyVector & properties)
{
	UT_return_val_if_fail (m_pts==PTS_Editing, false);

	pf_Frag * pf;
	PT_BlockOffset fo;

	// The return value of getFragFromPosition was never checked (sterwill)
	// bool bFound =
	getFragFromPosition(dpos,&pf,&fo);

	UT_return_val_if_fail (pf, false);
	
	if ((fo==0) && (pf->getPrev()))
	{
		if (pf->getPrev()->getType() == pf_Frag::PFT_FmtMark)
		{
			// we are adjacent to another FmtMark.  we can just hack on this
			// one rather than inserting two consecutive marks.
			
			pf_Frag_FmtMark * pffm = static_cast<pf_Frag_FmtMark *>(pf->getPrev());
			pf_Frag_Strux * pfsContainer = NULL;
			bool bFoundStrux;
			bFoundStrux = _getStruxOfTypeFromPosition(dpos,PTX_Block,&pfsContainer);
			UT_return_val_if_fail (bFoundStrux, false);

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
	UT_DebugOnly<bool> bMerged;
	bMerged = m_varset.mergeAP(ptc, indexOldAP, attributes, properties, &indexNewAP, getDocument());
	UT_ASSERT_HARMLESS(bMerged);

	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
		return true;

	pf_Frag_Strux * pfs = NULL;
	bool bFoundStrux = false;
	//
	// This code is to ensure that FmtMarks get inserted into
	// Embeded containers and not the enclosing block
	//
	if(pf->getType() == pf_Frag::PFT_Strux)
	{
	    pf_Frag_Strux * pfse = static_cast<pf_Frag_Strux *>(pf);
	    if(isEndFootnote(pfse))
	    {
		if(pf->getPrev())
		{
		    if (pf->getPrev()->getType() ==  pf_Frag::PFT_Strux)
		    {
			pfs = static_cast<pf_Frag_Strux *>(pf->getPrev());
			if(pfs->getStruxType() == PTX_Block)
			{
			    bFoundStrux = true;
			}
			else
			{
			    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		    }
		    else 
		    {
			bFoundStrux = _getStruxFromFragSkip(pf->getPrev(),&pfs);
		    }
		}
	    }
	}
	if(!bFoundStrux)
	    bFoundStrux = _getStruxFromFragSkip(pf,&pfs);
	UT_return_val_if_fail (bFoundStrux, false);
	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pf) + fo;

	if (!_insertFmtMark(pf,fo,indexNewAP))
		return false;

	// create a change record, add it to the history, and notify
	// anyone listening.

	PX_ChangeRecord_FmtMark * pcr
		= new PX_ChangeRecord_FmtMark(PX_ChangeRecord::PXT_InsertFmtMark,
									  dpos,indexNewAP,blockOffset);
	UT_return_val_if_fail (pcr,false);

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

		UT_return_val_if_fail (pf->getType() == pf_Frag::PFT_Text,false);
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
	UT_return_val_if_fail (m_pts==PTS_Editing, false);

	PT_BlockOffset blockOffset = 0;

#ifdef DEBUG
	{
		PT_DocPosition dposTest = getFragPosition(pfsBlock) + pfsBlock->getLength();
		UT_ASSERT_HARMLESS(dposTest == dpos);
	}
#endif

	if (!_insertFmtMark(pfsBlock,pfsBlock->getLength(),api))
		return false;

	// create a change record, add it to the history, and notify
	// anyone listening.

	PX_ChangeRecord_FmtMark * pcr
		= new PX_ChangeRecord_FmtMark(PX_ChangeRecord::PXT_InsertFmtMark,
									  dpos,api,blockOffset);
	UT_return_val_if_fail (pcr,false);

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
	UT_return_val_if_fail (m_pts==PTS_Editing,false);
	UT_return_val_if_fail (pfs,false);

	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pffm);

	PX_ChangeRecord_FmtMark * pcr
		= new PX_ChangeRecord_FmtMark(PX_ChangeRecord::PXT_DeleteFmtMark,
									  dpos, pffm->getIndexAP(), blockOffset);
	UT_return_val_if_fail (pcr,false);

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

/*
     purges fmt marks from document; leaves no records in the undo history
*/
bool pt_PieceTable::purgeFmtMarks()
{
	pf_Frag * pf_First = m_fragments.getFirst();
	pf_Frag * pfTemp   = pf_First;

#ifdef DEBUG
	UT_uint32 iCount  = 0;
#endif
	while (pfTemp)
	{
		if (pfTemp->getType() == pf_Frag::PFT_EndOfDoc)
			break;
		if (pfTemp->getType() == pf_Frag::PFT_FmtMark)
		{
			pf_Frag * pfNewTemp;
			PT_BlockOffset fragOffsetNewTemp;

			bool bResult = _deleteFmtMark(static_cast<pf_Frag_FmtMark *>(pfTemp), &pfNewTemp,&fragOffsetNewTemp);
			
			UT_return_val_if_fail (bResult,false);

			// FmtMarks have length zero, so we don't need to update dposTemp.
			pfTemp = pfNewTemp;
#ifdef DEBUG
			++iCount;
#endif
		}
		else
		{
			pfTemp = pfTemp->getNext();
		}
	}
	UT_DEBUGMSG(("pt_PieceTable::purgeFmtMarks: removed %d marks\n", iCount));
	return true;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool pt_PieceTable::_fmtChangeFmtMarkWithNotify(PTChangeFmt ptc, pf_Frag_FmtMark * pffm,
												   PT_DocPosition dpos, 
												   const PP_PropertyVector & attributes, const PP_PropertyVector & properties,
												   pf_Frag_Strux * pfs,
												   pf_Frag ** ppfNewEnd, UT_uint32 * pfragOffsetNewEnd)
{
	UT_return_val_if_fail (m_pts==PTS_Editing,false);

	// apply a span-level change to the given FmtMark.
	// create a change record for this change and put it in the history.

	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pffm->getIndexAP();
	UT_DebugOnly<bool> bMerged;
	bMerged = m_varset.mergeAP(ptc, indexOldAP, attributes, properties, &indexNewAP, getDocument());
	UT_ASSERT_HARMLESS(bMerged);

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
	UT_return_val_if_fail (pcr,false);

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
													PP_AttrProp *pAttrProp)
{
	UT_return_val_if_fail (pAttrProp,false);

#if 1
	// the old way is rather inefficient, inserting one property at a time ...
	_insertFmtMarkFragWithNotify(ptc, dpos, pAttrProp->getAttributes(), pAttrProp->getProperties());
#else
	const gchar * properties[] =	{ NULL, NULL, 0};

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

	const gchar * Attributes[] =	{ NULL, NULL, 0};

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
#endif
	return true;
}
