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


// deleteStrux-related functions for class pt_PieceTable.

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"

/****************************************************************/
/****************************************************************/
bool pt_PieceTable::_unlinkStrux(pf_Frag_Strux * pfs,
									pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	switch (pfs->getStruxType())	
	{
	case PTX_Section:
		return _unlinkStrux_Section(pfs,ppfEnd,pfragOffsetEnd);

	case PTX_SectionHdrFtr:
		return _unlinkStrux_Section(pfs,ppfEnd,pfragOffsetEnd);
		
	case PTX_Block:
		return _unlinkStrux_Block(pfs,ppfEnd,pfragOffsetEnd);
		
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
}

bool pt_PieceTable::_unlinkStrux_Block(pf_Frag_Strux * pfs,
										  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	UT_ASSERT(pfs->getStruxType()==PTX_Block);
	
	// unlink this Block strux from the document.
	// the caller is responsible for deleting pfs.

	if (ppfEnd)
		*ppfEnd = pfs->getNext();
	if (pfragOffsetEnd)
		*pfragOffsetEnd = 0;
	
	// find the previous strux (either a paragraph or something else).

	pf_Frag_Strux * pfsPrev = NULL;
	pf_Frag * pf = pfs->getPrev();
	while (pf && !pfsPrev)
	{
		if (pf->getType() == pf_Frag::PFT_Strux)
			pfsPrev = static_cast<pf_Frag_Strux *> (pf);
		pf = pf->getPrev();
	}
	UT_ASSERT(pfsPrev);			// we have a block that's not in a section ??

	switch (pfsPrev->getStruxType())
	{
	case PTX_Block:
		// if there is a paragraph before us, we can delete this
		// paragraph knowing that our content will be assimilated
		// in to the previous one.

		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;

	case PTX_Section:
		// we are the first paragraph in this section.  if we have
		// content, we cannot be deleted, since there is no one to
		// inherit our content.

		if (_struxHasContent(pfs))
		{
			// TODO decide if this should assert or just fail...
			UT_DEBUGMSG(("Cannot delete first paragraph with content.\n"));
			UT_ASSERT(0);
			return false;
		}


	case PTX_SectionHdrFtr:
		// we are the first paragraph in this section.  if we have
		// content, we cannot be deleted, since there is no one to
		// inherit our content.

		if (_struxHasContent(pfs))
		{
			// TODO decide if this should assert or just fail...
			UT_DEBUGMSG(("Cannot delete first paragraph with content.\n"));
			UT_ASSERT(0);
			return false;
		}

		// no content in this paragraph.
		
		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;

	default:
		UT_ASSERT(0);
		return false;
	}
}

bool pt_PieceTable::_unlinkStrux_Section(pf_Frag_Strux * pfs,
											pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	UT_ASSERT(pfs->getStruxType()==PTX_Section || pfs->getStruxType()==PTX_SectionHdrFtr );
	
	// unlink this Section strux from the document.
	// the caller is responsible for deleting pfs.

	if (ppfEnd)
		*ppfEnd = pfs->getNext();
	if (pfragOffsetEnd)
		*pfragOffsetEnd = 0;
	
	// find the previous strux (either a paragraph or something else).

	pf_Frag_Strux * pfsPrev = NULL;
	pf_Frag * pf = pfs->getPrev();
	while (pf && !pfsPrev)
	{
		if (pf->getType() == pf_Frag::PFT_Strux)
			pfsPrev = static_cast<pf_Frag_Strux *> (pf);
		pf = pf->getPrev();
	}

	if (!pfsPrev)
	{
		// first section in the document cannot be deleted.
		// TODO decide if this should assesrt or just file...
		UT_DEBUGMSG(("Cannot delete first section in document.\n"));
		UT_ASSERT(0);
		return false;
	}
	
	switch (pfsPrev->getStruxType())
	{
	case PTX_Block:
		// if there is a paragraph before us, we can delete this
		// section knowing that our paragraphs will be assimilated
		// in to the previous section (that is, the container of
		// this block).

		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;

	case PTX_Section:
		// there are no blocks (paragraphs) between this section
		// and the previous section.  this is not possible.
		// TODO decide if this should assert or just fail...
		UT_DEBUGMSG(("No blocks between sections ??\n"));
		UT_ASSERT(0);
		return false;


	case PTX_SectionHdrFtr:
		// there are no blocks (paragraphs) between this section
		// and the previous section.  this is not possible.
		// TODO decide if this should assert or just fail...
		UT_DEBUGMSG(("No blocks between sections ??\n"));
		UT_ASSERT(0);
		return false;

	default:
		UT_ASSERT(0);
		return false;
	}
}
			
bool pt_PieceTable::_deleteStruxWithNotify(PT_DocPosition dpos,
											  pf_Frag_Strux * pfs,
											  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	PX_ChangeRecord_Strux * pcrs
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_DeleteStrux,
									dpos, pfs->getIndexAP(), pfs->getStruxType());
	UT_ASSERT(pcrs);

	if (!_unlinkStrux(pfs,ppfEnd,pfragOffsetEnd))
		return false;
	
	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcrs);
	m_pDocument->notifyListeners(pfs,pcrs);

	delete pfs;

	return true;
}

			
bool pt_PieceTable::_deleteStrux_norec(PT_DocPosition dpos,
											  pf_Frag_Strux * pfs,
											  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	PX_ChangeRecord_Strux * pcrs
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_DeleteStrux,
									dpos, pfs->getIndexAP(), pfs->getStruxType());
	UT_ASSERT(pcrs);

	if (!_unlinkStrux(pfs,ppfEnd,pfragOffsetEnd))
		return false;
	
	// No history for field updates..
	// m_history.addChangeRecord(pcrs);
	m_pDocument->notifyListeners(pfs,pcrs);

	delete pfs;

	return true;
}









